//=============================================================================
/// Copyright (c) 2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for raytracing code objects binary analysis helper functions.
//=============================================================================

// C++.
#include <cassert>

// Shared.
#include "common/rga_entry_type.h"

// Backend.
#include "radeon_gpu_analyzer_backend/be_program_builder_binary.h"

// Local.
#include "radeon_gpu_analyzer_cli/kc_utils_binary_raytracing.h"
#include "radeon_gpu_analyzer_cli/kc_utils_dxr.h"
#include "radeon_gpu_analyzer_cli/kc_utils_vulkan.h"
#include "radeon_gpu_analyzer_cli/kc_utils.h"
#include "radeon_gpu_analyzer_cli/kc_xml_writer.h"

beWaveSize ExtractWaveSizeForShaderSubtype(const BeAmdPalMetaData::PipelineMetaData& pipeline, BeAmdPalMetaData::ShaderSubtype shader_subtype)
{
    beWaveSize wave_size = beWaveSize::kWave64;
    if (shader_subtype != BeAmdPalMetaData::ShaderSubtype::kUnknown)
    {
        for (const auto& stage : pipeline.hardware_stages)
        {
            if (BeAmdPalMetaData::StageType::kCS == stage.stage_type)
            {
                wave_size = BeAmdPalMetaData::GetWaveSize(stage.stats.wavefront_size);
            }
        }
    }
    return wave_size;
}

bool ExtractShaderSubtype(const BeAmdPalMetaData::PipelineMetaData& pipeline, const std::string& kernel, std::string& shader_subtype, beWaveSize& wave_size)
{
    bool ret = false;
    for (const auto& shader_function : pipeline.shader_functions)
    {
        if (kernel == shader_function.name)
        {
            shader_subtype = BeAmdPalMetaData::GetShaderSubtypeName(shader_function.shader_subtype);
            wave_size      = ExtractWaveSizeForShaderSubtype(pipeline, shader_function.shader_subtype);
            ret            = true;
        }
    }
    for (const auto& shader : pipeline.shaders)
    {
        if (shader.shader_subtype != BeAmdPalMetaData::ShaderSubtype::kUnknown)
        {
            shader_subtype = BeAmdPalMetaData::GetShaderSubtypeName(shader.shader_subtype);
            wave_size      = ExtractWaveSizeForShaderSubtype(pipeline, shader.shader_subtype);
            ret            = true;
        }
    }

    return ret;
}

beKA::beStatus RayTracingBinaryWorkflowStrategy::WriteOutputFiles(const Config&                             config,
                                                                  const std::string&                        asic,
                                                                  const std::map<std::string, std::string>& kernel_to_disassembly,
                                                                  const BeAmdPalMetaData::PipelineMetaData& amdpal_pipeline_md,
                                                                  std::string&                              error_msg)
{
    beKA::beStatus status = beKA::beStatus::kBeStatusGeneralFailed;
    assert(!kernel_to_disassembly.empty());
    if (!kernel_to_disassembly.empty())
    {
        std::string base_isa_filename = config.isa_file;
        for (const auto& kernel : kernel_to_disassembly)
        {
            const auto& amdgpu_kernel_name    = kernel.first;
            const auto& shader_kernel_content = kernel.second;
            std::string shader_kernel_subtype;
            beWaveSize  wave_size;
            if (ExtractShaderSubtype(amdpal_pipeline_md, amdgpu_kernel_name, shader_kernel_subtype, wave_size))
            {
                std::string concat_kernel_name = KcUtilsDxr::CombineKernelAndKernelSubtype(amdgpu_kernel_name, shader_kernel_subtype);
                std::string isa_filename;
                KcUtilsDxr::ConstructOutputFileName(
                    base_isa_filename, kStrDefaultFilenameIsa, kStrDefaultExtensionText, concat_kernel_name, asic, isa_filename);
                if (!isa_filename.empty())
                {
                    KcUtils::DeleteFile(isa_filename);

                    [[maybe_unused]] bool is_file_written = KcUtils::WriteTextFile(isa_filename, shader_kernel_content, nullptr);
                    assert(is_file_written);
                    if (!KcUtils::FileNotEmpty(isa_filename))
                    {
                        status = beKA::beStatus::kBeStatusWriteToFileFailed;
                        std::stringstream error_stream;
                        error_stream << concat_kernel_name << ", output file name " << isa_filename;
                        error_msg = error_stream.str();
                    }
                    else
                    {
                        // Store output metadata.
                        StoreOutputFilesToOutputMD(config, asic, amdgpu_kernel_name, shader_kernel_subtype, isa_filename, wave_size);
                        status = beKA::beStatus::kBeStatusSuccess;
                    }
                }
            }
        }
    }
    return status;
}

void RayTracingBinaryWorkflowStrategy::RunPostProcessingSteps(const Config& config, const BeAmdPalMetaData::PipelineMetaData& amdpal_pipeline_md)
{
    KcUtilsDxr     util(output_metadata_, config.print_process_cmd_line, log_callback_);
    beKA::beStatus status = beKA::beStatus::kBeStatusSuccess;

    // Generate CSV files with parsed ISA if required.
    if (config.is_parsed_isa_required)
    {
        status = util.ParseIsaFilesToCSV(config.is_line_numbers_required) ? beKA::beStatus::kBeStatusSuccess : beKA::beStatus::kBeStatusParseIsaToCsvFailed;
    }

    // Extract Statistics if required.
    if (status == beKA::beStatus::kBeStatusSuccess)
    {
        util.ExtractStatistics(config, amdpal_pipeline_md);
    }

    // Analyze live registers if requested.
    bool is_livereg_required = !config.livereg_analysis_file.empty();
    if (is_livereg_required && (status == beKA::beStatus::kBeStatusSuccess))
    {
        // Perform Live Registers analysis if required.
        status = util.PerformLiveVgprAnalysis(config) ? beKA::beStatus::kBeStatusSuccess : beKA::beStatus::kBeStatusParseIsaToCsvFailed;
    }
    bool is_live_sgpr_required = !config.sgpr_livereg_analysis_file.empty();
    if (is_live_sgpr_required && (status == beKA::beStatus::kBeStatusSuccess))
    {
        // Perform Live Registers analysis if required.
        status = util.PerformLiveSgprAnalysis(config) ? beKA::beStatus::kBeStatusSuccess : beKA::beStatus::kBeStatusParseIsaToCsvFailed;
    }

    bool is_cfg_required = (!config.block_cfg_file.empty() || !config.inst_cfg_file.empty());
    if (is_cfg_required && (status == beKA::beStatus::kBeStatusSuccess))
    {
        // Extract Control Flow Graph.
        util.ExtractCFG(config);
    }
}

bool RayTracingBinaryWorkflowStrategy::GenerateSessionMetadataFile(const Config& config)
{
    RgFileEntryData file_kernel_data;
    bool            ret = !config.session_metadata_file.empty();
    assert(ret);
    if (ret && !output_metadata_.empty())
    {
        ret = KcXmlWriter::GenerateClSessionMetadataFile(config.session_metadata_file, file_kernel_data, output_metadata_);
        if (!ret)
        {
            std::stringstream msg;
            msg << kStrErrorFailedToGenerateSessionMetdata << std::endl;
            log_callback_(msg.str());
        }
    }

    KcUtilsDxr::DeleteTempFiles(output_metadata_);

    return ret;
}

uint32_t GetRaytracingStage(const std::string& curr_kernel_subtype)
{
    uint32_t stage = BeRtxPipelineStage::kRayGeneration;
    for (const auto& kernel_subtype : kStrRtxStageNames)
    {
        if (kernel_subtype == curr_kernel_subtype)
        {
            break;
        }
        stage++;
    }
    assert(BeRtxPipelineStage::kRayGeneration <= stage && stage < BeRtxPipelineStage::kCountRtx);
    return stage;
}

void RayTracingBinaryWorkflowStrategy::StoreOutputFilesToOutputMD(const Config&      config,
                                                                  const std::string& asic,
                                                                  const std::string& kernel,
                                                                  const std::string& kernel_subtype,
                                                                  const std::string& isa_filename,
                                                                  beWaveSize         wave_size)
{
    std::string   concat_kernel_name             = KcUtilsDxr::CombineKernelAndKernelSubtype(kernel, kernel_subtype);
    uint32_t      stage                          = GetRaytracingStage(kernel_subtype);
    RgaEntryType  entry                          = beProgramBuilderBinary::GetEntryType(api_, stage);
    RgOutputFiles outFiles                       = RgOutputFiles(entry, isa_filename, binary_codeobj_file_);
    outFiles.input_file                          = concat_kernel_name;
    outFiles.is_isa_file_temp                    = config.isa_file.empty();
    outFiles.wave_size                           = wave_size;
    output_metadata_[{asic, concat_kernel_name}] = outFiles;
}

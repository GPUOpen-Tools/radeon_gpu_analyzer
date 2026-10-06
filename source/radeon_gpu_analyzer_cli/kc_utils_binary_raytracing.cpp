//=============================================================================
/// Copyright (c) 2026 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for raytracing code objects binary analysis helper functions.
//=============================================================================

// C++.
#include <cassert>

// Local.
#include "radeon_gpu_analyzer_cli/kc_utils_binary_raytracing.h"
#include "radeon_gpu_analyzer_cli/kc_utils_dxr.h"
#include "radeon_gpu_analyzer_cli/kc_utils_lightning.h"
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

bool ExtractShaderSubtype(const BeAmdPalMetaData::PipelineMetaData& pipeline,
                          const std::string&                        kernel,
                          std::string&                              shader_subtype,
                          beWaveSize&                               wave_size,
                          std::string&                              api_shader_hash)
{
    bool ret = false;
    for (const auto& shader_function : pipeline.shader_functions)
    {
        if (kernel == shader_function.raw_name)
        {
            shader_subtype  = BeAmdPalMetaData::GetShaderSubtypeName(shader_function.shader_subtype);
            wave_size       = ExtractWaveSizeForShaderSubtype(pipeline, shader_function.shader_subtype);
            api_shader_hash = BeAmdPalMetaData::GetShaderHashString(shader_function.hash);
            ret             = true;
        }
    }
    for (const auto& shader : pipeline.shaders)
    {
        if (shader.shader_subtype != BeAmdPalMetaData::ShaderSubtype::kUnknown)
        {
            shader_subtype  = BeAmdPalMetaData::GetShaderSubtypeName(shader.shader_subtype);
            wave_size       = ExtractWaveSizeForShaderSubtype(pipeline, shader.shader_subtype);
            api_shader_hash = BeAmdPalMetaData::GetShaderHashString(shader.hash);
            ret             = true;
        }
    }

    return ret;
}

beKA::beStatus KcCliRaytracingBinaryAnalysisStrategy::WriteOutputFiles(const Config&                             config,
                                                                       const std::string&                        asic,
                                                                       const std::map<std::string, std::string>& kernel_to_disassembly,
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
            std::string api_shader_hash;
            if (ExtractShaderSubtype(amdpal_pipeline_md_, amdgpu_kernel_name, shader_kernel_subtype, wave_size, api_shader_hash))
            {
                std::string demangled_name     = BeMangledKernelUtils::DemangleShaderName(amdgpu_kernel_name);
                std::string concat_kernel_name = KcUtilsDxr::CombineKernelAndKernelSubtype(demangled_name, shader_kernel_subtype);
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
                        StoreOutputFilesToOutputMD(config, asic, demangled_name, shader_kernel_subtype, isa_filename, wave_size, api_shader_hash);
                        status = beKA::beStatus::kBeStatusSuccess;
                    }
                }
            }
        }
    }
    return status;
}

void KcCliRaytracingBinaryAnalysisStrategy::RunPostProcessingSteps(const Config& config)
{
    KcUtilsDxr util(binary_codeobj_file_, output_metadata_, config.print_process_cmd_line, log_callback_);

    // Extract Statistics if required.
    util.ExtractStatistics(config, amdpal_pipeline_md_);

    // csv, livereg, cfg, etc.
    util.RunPostProcessingSteps(config);
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

void KcCliRaytracingBinaryAnalysisStrategy::StoreOutputFilesToOutputMD(const Config&      config,
                                                                       const std::string& asic,
                                                                       const std::string& kernel,
                                                                       const std::string& kernel_subtype,
                                                                       const std::string& isa_filename,
                                                                       beWaveSize         wave_size,
                                                                       const std::string& api_shader_hash)
{
    std::string   concat_kernel_name             = KcUtilsDxr::CombineKernelAndKernelSubtype(kernel, kernel_subtype);
    uint32_t      stage                          = GetRaytracingStage(kernel_subtype);
    auto          api                            = beProgramBuilderBinary::GetApiFromPipelineMetadata(amdpal_pipeline_md_);
    RgaEntryType  entry                          = beProgramBuilderBinary::GetEntryType(api, stage);
    RgOutputFiles outFiles                       = RgOutputFiles(entry, isa_filename, binary_codeobj_file_);
    outFiles.input_file                          = concat_kernel_name;
    outFiles.is_isa_file_temp                    = config.isa_file.empty();
    outFiles.wave_size                           = wave_size;
    outFiles.api_shader_hash                     = api_shader_hash;
    output_metadata_[{asic, concat_kernel_name}] = outFiles;
}

bool KcCliRaytracingBinaryAnalysisStrategy::GenerateSessionMetadataFile(const Config& config)
{
    bool ret = !config.session_metadata_file.empty();
    assert(ret);
    if (ret && !output_metadata_.empty())
    {
        ret = KcXmlWriter::GenerateBinaryAnalysisSessionMetadataFile(config.session_metadata_file, binary_codeobj_file_, output_metadata_);
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

bool KcCliRaytracingBinaryAnalysisStrategy::GeneratCompilationSummary(const Config& config, const std::string& asic, RgaAnalysisSummary::AnalysisResult& result)
{
    bool ret = !config.session_summary_file.empty();
    if (ret && !output_metadata_.empty())
    {
        result.target_architecture_ = asic;
        if (config.include_target_metadata)
        {
            beKA::BeIsaSpecExplorer::PopulateFromSpec(asic, result.target_architecture_metadata_);
        }

        result.inputs_.inputs_.push_back(binary_codeobj_file_);
        result.output_.api_ = kStrRgaModeDxr;

        for (const auto& out_file_data : output_metadata_)
        {
            RgaAnalysisSummary::Kernel kernel;
            ret = ret && KcUtils::GenerateKernelSummary(
                             out_file_data.first.first, out_file_data.first.second, out_file_data.second, kernel, log_callback_, config.print_process_cmd_line);
            if (ret)
            {
                result.output_.kernels_.emplace_back(kernel);
            }
        }
    }
    return ret;
}

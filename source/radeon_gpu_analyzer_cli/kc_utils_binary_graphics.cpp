//=============================================================================
/// Copyright (c) 2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for binary analysis graphics strategy.
//=============================================================================
// C++.
#include <cassert>

// Shared.
#include "common/rga_entry_type.h"

// Backend.
#include "radeon_gpu_analyzer_backend/be_program_builder_binary.h"

// Local.
#include "radeon_gpu_analyzer_cli/kc_utils_binary_graphics.h"
#include "radeon_gpu_analyzer_cli/kc_utils_dxr.h"
#include "radeon_gpu_analyzer_cli/kc_utils_vulkan.h"
#include "radeon_gpu_analyzer_cli/kc_utils.h"
#include "radeon_gpu_analyzer_cli/kc_xml_writer.h"

beKA::beStatus GraphicsBinaryWorkflowStrategy::WriteOutputFiles(const Config&                             config,
                                                                const std::string&                        asic,
                                                                const std::map<std::string, std::string>& kernel_to_disassembly,
                                                                const BeAmdPalMetaData::PipelineMetaData& amdpal_pipeline_md,
                                                                std::string&                              )
{
    beKA::beStatus status = beKA::beStatus::kBeStatusGeneralFailed;
    assert(!kernel_to_disassembly.empty());
    if (!kernel_to_disassembly.empty())
    {
        // Retrive graphics api for current binary.
        graphics_api_ = beProgramBuilderBinary::GetApiFromPipelineMetadata(amdpal_pipeline_md);
        
        // Write the ISA disassembly files.
        for (uint32_t stage = BePipelineStage::kVertex; stage < BePipelineStage::kCount; stage++)
        {
            std::string isa_file;
            if (KcUtils::ConstructOutFileName(
                    config.isa_file, beProgramBuilderBinary::GetStageFileSuffixesFromApi(graphics_api_)[stage], asic, kStrVulkanIsaFileExtension, isa_file))
            {
                if (!isa_file.empty())
                {
                    KcUtils::DeleteFile(isa_file);
                }
                
                beWaveSize wave_size;
                bool is_file_written = beProgramBuilderVulkan::WriteIsaFileWithHwMapping(stage, amdpal_pipeline_md, kernel_to_disassembly, isa_file, wave_size);
                if (is_file_written)
                {
                    // For graphics workflows, also extract stats.
                    bool is_stats_file_required = !config.analysis_file.empty();
                    if (is_stats_file_required)
                    {
                        std::string stats_file;
                        if (KcUtils::ConstructOutFileName(config.analysis_file,
                                                          beProgramBuilderBinary::GetStageFileSuffixesFromApi(graphics_api_)[stage],
                                                          asic,
                                                          kStrVulkanStatsFileExtension,
                                                          stats_file))
                        {
                            StoreOutputFilesToOutputMD(config, asic, stage, isa_file, stats_file, wave_size);

                            if (!stats_file.empty())
                            {
                                KcUtils::DeleteFile(stats_file);
                            }
                        }
                    }
                    else
                    {
                        StoreOutputFilesToOutputMD(config, asic, stage, isa_file, "", wave_size);
                    }

                    status = beKA::beStatus::kBeStatusSuccess;
                }
            }
        }

        KcUtilsVulkan vk_util(output_metadata_, "", log_callback_, beProgramBuilderBinary::GetStageFileSuffixesFromApi(graphics_api_));
        vk_util.ExtractStatistics(config, asic, amdpal_pipeline_md, kernel_to_disassembly);

    }

    return status;
}

void GraphicsBinaryWorkflowStrategy::StoreOutputFilesToOutputMD(const Config&      config,
                                                                const std::string& asic,
                                                                uint32_t           stage,
                                                                const std::string& isa_filename,
                                                                const std::string& stats_filename,
                                                                beWaveSize         wave_size)
{
    bool device_md_exists = (output_metadata_.find(asic) != output_metadata_.end());
    if (!device_md_exists)
    {
        RgVkOutputMetadata md;
        RgOutputFiles      out_files(asic);
        md.fill(out_files);
        output_metadata_[asic] = md;
    }

    RgVkOutputMetadata& device_md = output_metadata_[asic];
    if (stage < BePipelineStage::kCount)
    {
        RgOutputFiles& stage_md = device_md[stage];
        stage_md.entry_type     = beProgramBuilderBinary::GetEntryType(graphics_api_, stage);
        stage_md.device         = asic;
        stage_md.input_file     = binary_codeobj_file_;
        stage_md.bin_file       = binary_codeobj_file_;
        stage_md.isa_file       = isa_filename;
        if (!config.analysis_file.empty())
        {
            stage_md.stats_file = stats_filename;
        }
        stage_md.wave_size = wave_size;
    }

    // If user does not provide an isa filename explicitly.
    if (config.isa_file.empty())
    {
        // ISA file generated is a temporary one.
        temp_files_.push_back(isa_filename);
    }
}

void GraphicsBinaryWorkflowStrategy::RunPostProcessingSteps(const Config& config, const BeAmdPalMetaData::PipelineMetaData&)
{
    const auto&   suffixes = beProgramBuilderBinary::GetStageFileSuffixesFromApi(graphics_api_);
    KcUtilsVulkan vk_util(output_metadata_, "", log_callback_, suffixes);
    vk_util.RunPostProcessingSteps(config);
}

bool GraphicsBinaryWorkflowStrategy::GenerateSessionMetadataFile(const Config& config)
{
    RgFileEntryData    file_kernel_data;
    RgClOutputMetadata metadata;
    bool               ret = !config.session_metadata_file.empty();
    assert(ret);
    if (ret)
    {
        // For each per-device data.
        for (const auto& output_metadata_for_device : output_metadata_)
        {
            const RgVkOutputMetadata& out_files_for_device = output_metadata_for_device.second;
            if (out_files_for_device.empty())
            {
                continue;
            }

            for (const auto& output_files : out_files_for_device)
            {
                if (!output_files.input_file.empty())
                {
                    std::string stage_str;
                    ret                                                     = RgaEntryTypeUtils::GetEntryTypeStr(output_files.entry_type, stage_str);
                    metadata[{output_metadata_for_device.first, stage_str}] = output_files;
                }
            }
        }

        ret = ret && KcXmlWriter::GenerateClSessionMetadataFile(config.session_metadata_file, file_kernel_data, metadata);
        if (!ret)
        {
            RgLog::stdOut << kStrErrorFailedToGenerateSessionMetdata << std::endl;
        }
    }

    if (!config.should_retain_temp_files)
    {
        KcUtilsVulkan::DeleteTempFiles(temp_files_);
    }

    return ret;
}

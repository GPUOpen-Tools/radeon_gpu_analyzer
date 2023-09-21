//======================================================================
// Copyright 2023 Advanced Micro Devices, Inc. All rights reserved.
//======================================================================

#pragma once

// Shared.
#include "common/rg_log.h"

// Local.
#include "source/radeon_gpu_analyzer_cli/kc_data_types.h"
#include "source/radeon_gpu_analyzer_cli/kc_cli_string_constants.h"
#include "radeon_gpu_analyzer_cli/kc_config.h"

// Class for Vulkan mode utility functions for ISA Dissassembly and post-processing.
class KcCLICommanderVulkanUtil
{
public:
    KcCLICommanderVulkanUtil(std::map<std::string, RgVkOutputMetadata>& output_metadata,
                             std::string physical_adapter_name, 
                             LoggingCallbackFunction log_callback,
                             const std::array<std::string, BePipelineStage::kCount>& vulkan_stage_file_suffix)
        : output_metadata_(output_metadata)
        , physical_adapter_name_(physical_adapter_name)
        , log_callback_(log_callback)
        , vulkan_stage_file_suffix_(vulkan_stage_file_suffix)
    {}

    // Post-process for all devices.
    void RunPostProcessingSteps(const Config& config) const;

    // Generate RGA CLI session metadata file.
    bool GenerateSessionMetadata(const Config& config) const;

    // Delete temporary files.
    static void DeleteTempFiles(std::vector<std::string>& temp_files);

    // Convert statistics file from Vulkan mode into normal RGA stats format.
    static beKA::beStatus ConvertStats(const BeVkPipelineFiles& isaFiles, 
                                       const BeVkPipelineFiles& stats_files, 
                                       const Config&            config, 
                                       const std::string&       device);

    // Convert ISA text to CSV form with additional data.
    static bool GetParsedIsaCsvText(const std::string& isaText, const std::string& device, bool add_line_numbers, std::string& csvText);

    // Store ISA text in the file.
    static beKA::beStatus WriteIsaToFile(const std::string& file_name, const std::string& isa_text, LoggingCallbackFunction log_callback);

    
    // Extract Resource Usage (statistics) data.
    void ExtractStatistics(const Config&                             config,
                           const std::string&                        device,
                           const std::string&                        amdgpu_dis_output,
                           const std::map<std::string, std::string>& shader_to_disassembly);

    // Utility for extracting statistics.
    static std::string BuildStatisticsStr(const beKA::AnalysisData& stats, std::size_t stage, bool is_compute_bit_set);

    // Utility for populating statistics Analysis data from amdgpu metadata.
    static beKA::AnalysisData PopulateAnalysisData(size_t pos,
                                                   const std::string&  amdgpu_dis_md_str,
                                                   const std::string&  current_device);

 private:

    KcCLICommanderVulkanUtil() = default;

    // Parse ISA files and generate separate files that contain parsed ISA in CSV format.
    bool ParseIsaFilesToCSV(bool add_line_numbers, const std::string& device_string, RgVkOutputMetadata& metadata) const;

    // Perform the live registers analysis.
    bool PerformLiveRegAnalysis(const Config& config, const std::string& device_string, RgVkOutputMetadata& metadata) const;

    // Generate the per-block or per-instruction Control Flow Graph.
    bool ExtractCFG(const Config& config, const std::string& device_string, const RgVkOutputMetadata& metadata) const;

    // Log result to stdout based on passed bool.
    void LogResult(bool result) const
    {
        RgLog::stdOut << (result ? kStrInfoSuccess : kStrInfoFailed) << std::endl;
    }

    // Per-device output metadata.
    std::map<std::string, RgVkOutputMetadata>& output_metadata_;

    // Name of the first physical adapter installed on the system.
    std::string physical_adapter_name_;

    // Log callback function.
    LoggingCallbackFunction log_callback_;

    // Suffixes for stage-specific output files.
    const std::array<std::string, BePipelineStage::kCount>& vulkan_stage_file_suffix_;

};

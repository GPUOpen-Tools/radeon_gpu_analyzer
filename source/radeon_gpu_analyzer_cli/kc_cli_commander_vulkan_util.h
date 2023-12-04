//======================================================================
// Copyright 2023 Advanced Micro Devices, Inc. All rights reserved.
//======================================================================

#pragma once

// Shared.
#include "common/rg_log.h"

// Backend.
#include "radeon_gpu_analyzer_backend/be_program_builder_vulkan.h"

// Local.
#include "source/radeon_gpu_analyzer_cli/kc_data_types.h"
#include "source/radeon_gpu_analyzer_cli/kc_cli_string_constants.h"
#include "radeon_gpu_analyzer_cli/kc_config.h"

// Extensions of different input/output file types.
static const std::string kStrVulkanSpirvFileExtension          = "spv";
static const std::string kStrVulkanSpirvTextFileExtension      = "spvasm";
static const std::string kStrVulkanBinaryFileExtension         = "bin";
static const std::string kStrVulkanIsaFileExtension            = "isa";
static const std::string kStrVulkanStatsFileExtension          = "csv";
static const std::string kStrVulkanValidationInfoFileExtension = "txt";
static const std::string kStrVulkanHlslFileExtension           = "hlsl";

// An array containing per-stage RgEntryType(s).
typedef std::array<RgEntryType, BePipelineStage::kCount> BeVkPipelineEntries;

// Output metadata entry types for Vulkan pipeline stages.
static const BeVkPipelineEntries 
kVulkanStageEntryTypes = 
{
    RgEntryType::kVkVertex,
    RgEntryType::kVkTessControl,
    RgEntryType::kVkTessEval,
    RgEntryType::kVkGeometry,
    RgEntryType::kVkFragment,
    RgEntryType::kVkCompute
};

// Output metadata entry types for Vulkan pipeline stages.
static const BeVkPipelineEntries 
kOpenGLStageEntryTypes = 
{
    RgEntryType::kGlVertex,
    RgEntryType::kGlTessControl,
    RgEntryType::kGlTessEval,
    RgEntryType::kGlGeometry,
    RgEntryType::kGlFragment,
    RgEntryType::kGlCompute
};

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

    // Util to convert stats.
    static beKA::beStatus ConvertStats(const std::string& is_file,
                                       const std::string& stats_file,
                                       const Config&      config,
                                       const std::string& device);

    // Convert ISA text to CSV form with additional data.
    static bool GetParsedIsaCsvText(const std::string& isaText, const std::string& device, bool add_line_numbers, std::string& csvText);

    // Store ISA text in the file.
    static beKA::beStatus WriteIsaToFile(const std::string& file_name, const std::string& isa_text, LoggingCallbackFunction log_callback);

    
    // Extract Resource Usage (statistics) data.
    void ExtractStatistics(const Config&                             config,
                           const std::string&                        device,
                           const BeAmdPalMetaData::PipelineMetaData& amdpal_pipeline_md,
                           const std::map<std::string, std::string>& shader_to_disassembly);

    // Utility for extracting statistics.
    static std::string BuildStatisticsStr(const beKA::AnalysisData& stats, std::size_t stage, bool is_compute_bit_set);

    // Utility for populating statistics Analysis data from amdgpu metadata.
    static beKA::AnalysisData PopulateAnalysisData(const beKA::AnalysisData& stats,
                                                   const std::string&        current_device);

 private:

    KcCLICommanderVulkanUtil() = default;

    // Parse ISA files and generate separate files that contain parsed ISA in CSV format.
    bool ParseIsaFilesToCSV(bool add_line_numbers, const std::string& device_string, RgVkOutputMetadata& metadata) const;

    // Perform the live registers analysis.
    bool PerformLiveVgprAnalysis(const Config& config, const std::string& device_string, RgVkOutputMetadata& metadata) const;

    // Perform the live registers analysis.
    bool PerformLiveSgprAnalysis(const Config& config, const std::string& device_string, RgVkOutputMetadata& metadata) const;

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

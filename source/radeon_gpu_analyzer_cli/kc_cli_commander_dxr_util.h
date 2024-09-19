//======================================================================
// Copyright 2023 Advanced Micro Devices, Inc. All rights reserved.
//======================================================================

#ifndef RGA_RADEONGPUANALYZERCLI_SRC_KC_CLI_COMMANDER_DXR_UTIL_H_
#define RGA_RADEONGPUANALYZERCLI_SRC_KC_CLI_COMMANDER_DXR_UTIL_H_

// C++.
#include <string>

// Backend.
#include "source/radeon_gpu_analyzer_backend/be_include.h"
#include "radeon_gpu_analyzer_backend/be_program_builder_vulkan.h"

// Local.
#include "source/radeon_gpu_analyzer_cli/kc_data_types.h"
#include "radeon_gpu_analyzer_cli/kc_config.h"


// Class for DXR mode utility functions for ISA post-processing.
class KcCLICommanderDxrUtil
{
public:

    KcCLICommanderDxrUtil(const std::string&      binary_codeobj_file,
                          RgClOutputMetadata&     output_metadata,
                          bool                    should_print_cmd,
                          LoggingCallbackFunction log_callback)
        : binary_codeobj_file_(binary_codeobj_file)
        , output_metadata_(output_metadata)
        , should_print_cmd_(should_print_cmd)
        , log_callback_(log_callback)
    {}

    // Parse ISA files and generate separate files that contain parsed ISA in CSV format.
    bool ParseIsaFilesToCSV(bool add_line_numbers) const;

    // Perform live VGPR analysis.
    bool PerformLiveVgprAnalysis(const Config& config) const;

    // Perform live SGPR analysis.
    bool PerformLiveSgprAnalysis(const Config& config) const;

    // Extract program Control Flow Graph.
    bool ExtractCFG(const Config& config) const;

    // Extract Resource Usage (statistics) data.
    beKA::beStatus ExtractStatistics(const Config& config, const BeAmdPalMetaData::PipelineMetaData& amdpal_pipeline_md) const;

    // Perform post-compile actions.
    bool RunPostCompileSteps(const Config& config);

    // Helper function that combines the kernel and shader_subtype to kernel_shader_subtype.
    static std::string CombineKernelAndKernelSubtype(const std::string& kernel, const std::string& shader_subtype);

    // Helper function that sepeartes the kernel and shader_subtype from kernel_shader_subtype.
    static std::pair<std::string, std::string> SeparateKernelAndKernelSubtype(const std::string& combined_name);

    // Convert statistics file from Graphics mode into normal RGA stats format.
    static beKA::beStatus ConvertStats(const BeRtxPipelineFiles& isaFiles,
                                       const BeRtxPipelineFiles& stats_files,
                                       const Config&            config,
                                       const std::string&       device);

    // Wrapper around KcUtils::ConstructOutputFileName() to keep track of session output files.
    static void ConstructOutputFileName(const std::string& base_output_filename,
                                        const std::string& default_suffix,
                                        const std::string& default_extension,
                                        const std::string& entry_point_name,
                                        const std::string& device_name,
                                        std::string&       generated_filename); 

private:

    // Generate RGA CLI session metadata file.
    bool GenerateSessionMetadata(const Config& config) const;

    // Delete all temporary files created by RGA.
    void DeleteTempFiles() const;

    // ---- DATA ----

    // CodeObject Binary filename.
    std::string binary_codeobj_file_;

    // Output Metadata.
    RgClOutputMetadata& output_metadata_;

    // Specifies whether the "-#" option (print commands) is enabled.
    bool should_print_cmd_ = false;

    // Log callback function.
    LoggingCallbackFunction log_callback_;

    // Ouput files generated for the CLI session.
    static std::set<std::string> session_output_files_;

};

#endif  // RGA_RADEONGPUANALYZERCLI_SRC_KC_CLI_COMMANDER_DXR_UTIL_H_
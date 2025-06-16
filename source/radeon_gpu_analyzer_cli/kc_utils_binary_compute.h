//=============================================================================
/// Copyright (c) 2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for binary analysis compute strategy.
//=============================================================================
#ifndef RGA_RADEONGPUANALYZERCLI_SRC_KC_UTILS_BINARY_COMPUTE_H_
#define RGA_RADEONGPUANALYZERCLI_SRC_KC_UTILS_BINARY_COMPUTE_H_

// Local.
#include "radeon_gpu_analyzer_cli/kc_utils_binary_default.h"

// Post-processing workflow strategy functions for compute workflows.
class ComputeBinaryWorkflowStrategy : public BinaryWorkflowStrategy
{
public:
    ComputeBinaryWorkflowStrategy(std::string binary_codeobj_file, LoggingCallbackFunction log_callback)
        : binary_codeobj_file_(binary_codeobj_file)
        , log_callback_(log_callback)
    {
    }

    // Write Isa file(s) to disk for compute workflows.
    beKA::beStatus WriteOutputFiles(const Config&                             config,
                                    const std::string&                        asic,
                                    const std::map<std::string, std::string>& kernel_to_disassembly,
                                    const BeAmdPalMetaData::PipelineMetaData& amdpal_pipeline_md,
                                    std::string&                              error_msg) override;

    // Perform post-processing actions for compute workflows.
    void RunPostProcessingSteps(const Config& config, const BeAmdPalMetaData::PipelineMetaData& amdpal_pipeline_md) override;

    // Generates the metadata for the binary.
    bool GenerateSessionMetadataFile(const Config& config) override;

private:
    // Store output file names to the output metadata for compute workflows.
    void StoreOutputFilesToOutputMD(const Config&      config,
                                    const std::string& asic,
                                    const std::string& kernel_name,
                                    const std::string& kernel_abbreviation,
                                    const std::string& isa_filename);

    
    // Path to binary code object on disk.
    std::string binary_codeobj_file_;

    // Output Metadata for compute workflows.
    RgClOutputMetadata output_metadata_;

    // Log callback function.
    LoggingCallbackFunction log_callback_;
};

#endif  // RGA_RADEONGPUANALYZERCLI_SRC_KC_UTILS_BINARY_COMPUTE_H_

//=============================================================================
/// Copyright (c) 2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for binary analysis graphics strategy.
//=============================================================================

#ifndef RGA_RADEONGPUANALYZERCLI_SRC_KC_UTILS_BINARY_GRAPHICS_H_
#define RGA_RADEONGPUANALYZERCLI_SRC_KC_UTILS_BINARY_GRAPHICS_H_

// Backend.
#include "radeon_gpu_analyzer_backend/be_program_builder_binary.h"

// Local.
#include "radeon_gpu_analyzer_cli/kc_utils_binary_default.h"

// Post-processing workflow strategy functions for graphics workflows.
class GraphicsBinaryWorkflowStrategy : public BinaryWorkflowStrategy
{
public:
    GraphicsBinaryWorkflowStrategy(std::string binary_codeobj_file, LoggingCallbackFunction log_callback)
        : binary_codeobj_file_(binary_codeobj_file)
        , log_callback_(log_callback)
    {
    }

    // Write Isa file(s) to disk for graphics workflows.
    beKA::beStatus WriteOutputFiles(const Config&                             config,
                                    const std::string&                        asic,
                                    const std::map<std::string, std::string>& kernel_to_disassembly,
                                    const BeAmdPalMetaData::PipelineMetaData& amdpal_pipeline_md,
                                    std::string&                              error_msg) override;

    // Perform post-processing actions for graphics workflows.
    void RunPostProcessingSteps(const Config& config, const BeAmdPalMetaData::PipelineMetaData& amdpal_pipeline_md) override;

    // Generates the metadata for the binary.
    bool GenerateSessionMetadataFile(const Config& config) override;

    // The type of Graphics Api.
    beProgramBuilderBinary::ApiEnum graphics_api_ = beProgramBuilderBinary::ApiEnum::kUnknown;

private:
    // Store output file names to the output metadata for graphics workflows.
    void StoreOutputFilesToOutputMD(const Config&      config,
                                    const std::string& asic,
                                    uint32_t           stage,
                                    const std::string& isa_filename,
                                    const std::string& stats_filename,
                                    beWaveSize         wave_size);
    
    // Path to binary code object on disk.
    std::string binary_codeobj_file_;

    // Per-device output metadata.
    std::map<std::string, RgVkOutputMetadata> output_metadata_;

    // Temporary files.
    std::vector<std::string> temp_files_;

    // Log callback function.
    LoggingCallbackFunction log_callback_;
};

#endif  // RGA_RADEONGPUANALYZERCLI_SRC_KC_UTILS_BINARY_GRAPHICS_H_

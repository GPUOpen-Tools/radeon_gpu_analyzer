//=============================================================================
/// Copyright (c) 2026 Advanced Micro Devices, Inc. All rights reserved.
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
class KcCliGraphicsBinaryAnalysisStrategy : public KcCliBinaryAnalysisStrategy
{
public:
    KcCliGraphicsBinaryAnalysisStrategy(std::string                        binary_codeobj_file,
                                        beProgramBuilderBinary::ApiEnum    graphics_api,
                                        BeAmdPalMetaData::PipelineMetaData amdpal_pipeline_md,
                                        LoggingCallbackFunction            log_callback)
        : binary_codeobj_file_(binary_codeobj_file)
        , graphics_api_(graphics_api)
        , amdpal_pipeline_md_(std::move(amdpal_pipeline_md))
        , log_callback_(log_callback)
    {
    }

    // Write Isa file(s) to disk for graphics workflows.
    beKA::beStatus WriteOutputFiles(const Config&                             config,
                                    const std::string&                        asic,
                                    const std::map<std::string, std::string>& kernel_to_disassembly,
                                    std::string&                              error_msg) override;

    // Perform post-processing actions for graphics workflows.
    void RunPostProcessingSteps(const Config& config) override;

    // Generate session metadata for the code object.
    bool GenerateSessionMetadataFile(const Config& config) override;

    // Generates the analysis output for the binary.
    bool GeneratCompilationSummary(const Config& config, const std::string& asic, RgaAnalysisSummary::AnalysisResult& result) override;

    // The type of Graphics Api.
    beProgramBuilderBinary::ApiEnum graphics_api_ = beProgramBuilderBinary::ApiEnum::kUnknown;

private:
    // Store output file names to the output metadata for graphics workflows.
    void StoreOutputFilesToOutputMD(const Config&      config,
                                    const std::string& asic,
                                    uint32_t           stage,
                                    const std::string& isa_filename,
                                    const std::string& stats_filename,
                                    beWaveSize         wave_size,
                                    const std::string& api_shader_hash);

    // Path to binary code object on disk.
    std::string binary_codeobj_file_;

    // Parsed pipeline metadata for the binary code object.
    BeAmdPalMetaData::PipelineMetaData amdpal_pipeline_md_;

    // Per-device output metadata.
    std::map<std::string, RgVkOutputMetadata> output_metadata_;

    // Temporary files.
    std::vector<std::string> temp_files_;

    // Log callback function.
    LoggingCallbackFunction log_callback_;
};

#endif  // RGA_RADEONGPUANALYZERCLI_SRC_KC_UTILS_BINARY_GRAPHICS_H_

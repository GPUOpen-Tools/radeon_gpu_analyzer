//======================================================================
// Copyright 2023 Advanced Micro Devices, Inc. All rights reserved.
//======================================================================
#ifndef RGA_RADEONGPUANALYZERCLI_SRC_KC_CLI_COMMANDER_BIN_UTIL_H_
#define RGA_RADEONGPUANALYZERCLI_SRC_KC_CLI_COMMANDER_BIN_UTIL_H_

// C++.
#include <string>

// Backend.
#include "radeon_gpu_analyzer_backend/be_utils.h"
#include "radeon_gpu_analyzer_backend/be_program_builder_binary.h"

// Local.
#include "radeon_gpu_analyzer_cli/kc_config.h"

// AmdgpudisOutput Parser interface.
class ParseAmdgpudisOutputStrategy
{
public:

    // Defaulted Virtual Destructor.
    virtual ~ParseAmdgpudisOutputStrategy() = default;

    // Parses amdgpu-dis output and extracts a table with
    // the amdgpu kernel name being the key and that shader stage's disassembly the value.
    virtual beKA::beStatus ParseAmdgpudisKernels(const std::string&                  amdgpu_dis_output,
                                                 std::map<std::string, std::string>& kernel_to_disassembly,
                                                 std::string&                        error_msg) const = 0;
};

// AmdgpudisOutput Parser for graphics workflows.
class ParseAmdgpudisOutputGraphicStrategy : public ParseAmdgpudisOutputStrategy
{
public:
    // Parses amdgpu-dis output and extracts a table with
    // the amdgpu shader name being the key and that shader stage's disassembly the value.
    beKA::beStatus ParseAmdgpudisKernels(const std::string&                  amdgpu_dis_output,
                                         std::map<std::string, std::string>& kernel_to_disassembly,
                                         std::string&                        error_msg) const override;
};

// AmdgpudisOutput Parser for compute workflows.
class ParseAmdgpudisOutputComputeStrategy : public ParseAmdgpudisOutputStrategy
{
public:
    // Parses amdgpu-dis output and extracts a table with
    // the amdgpu shader name being the key and that shader stage's disassembly the value.
    beKA::beStatus ParseAmdgpudisKernels(const std::string&                  amdgpu_dis_output,
                                         std::map<std::string, std::string>& kernel_to_disassembly,
                                         std::string&                        error_msg) const override;
};

// Post-processing workflow functions interface for binary mode.
class BinaryWorkflowStrategy
{
public:

    // Defaulted Virtual Destructor.
    virtual ~BinaryWorkflowStrategy() = default;

    // Write Isa file(s) to disk.
    virtual beKA::beStatus WriteOutputFiles(const Config&                             config,
                                            const std::string&                        asic,
                                            const std::map<std::string, std::string>& kernel_to_disassembly,
                                            const BeAmdPalMetaData::PipelineMetaData& amdpal_pipeline_md,
                                            std::string&                              error_msg) = 0;

    // Perform post-processing actions.
    virtual void RunPostProcessingSteps(const Config& config, const BeAmdPalMetaData::PipelineMetaData& amdpal_pipeline_md) = 0;

    // Perform post-compile actions.
    virtual bool RunPostCompileSteps(const Config& config) = 0;
};

// Post-processing workflow strategy functions for graphics workflows.
class GraphicsBinaryWorkflowStrategy : public BinaryWorkflowStrategy
{
public:
    GraphicsBinaryWorkflowStrategy(LoggingCallbackFunction log_callback)
        : log_callback_(log_callback)
    {}

    // Write Isa file(s) to disk for graphics workflows.
    beKA::beStatus WriteOutputFiles(const Config&                             config,
                                    const std::string&                        asic,
                                    const std::map<std::string, std::string>& kernel_to_disassembly,
                                    const BeAmdPalMetaData::PipelineMetaData& amdpal_pipeline_md,
                                    std::string&                              error_msg) override;

    // Perform post-processing actions for graphics workflows.
    void RunPostProcessingSteps(const Config& config, const BeAmdPalMetaData::PipelineMetaData& amdpal_pipeline_md) override;

    // Perform post-compile actions for graphics workflows.
    bool RunPostCompileSteps(const Config& config) override;

    // The type of Graphics Api.
    beProgramBuilderBinary::ApiEnum graphics_api_;

private:

    // Store output file names to the output metadata for graphics workflows.
    void StoreOutputFilesToOutputMD(const Config&      config,
                                    const std::string& asic,
                                    uint32_t           stage,
                                    const std::string& isa_filename,
                                    const std::string& stats_filename);

    // Per-device output metadata.
    std::map<std::string, RgVkOutputMetadata> output_metadata_;

    // Temporary files.
    std::vector<std::string> temp_files_;

    // Log callback function.
    LoggingCallbackFunction log_callback_;
};

// Post-processing workflow strategy functions for dxr workflows.
class RayTracingBinaryWorkflowStrategy : public BinaryWorkflowStrategy
{
public:
    RayTracingBinaryWorkflowStrategy(LoggingCallbackFunction log_callback)
        : log_callback_(log_callback)
    {
    }

    // Write Isa file(s) to disk for raytracing workflows.
    beKA::beStatus WriteOutputFiles(const Config&                             config,
                                    const std::string&                        asic,
                                    const std::map<std::string, std::string>& kernel_to_disassembly,
                                    const BeAmdPalMetaData::PipelineMetaData& amdpal_pipeline_md,
                                    std::string&                              error_msg) override;

    // Perform post-processing actions for raytracing workflows.
    void RunPostProcessingSteps(const Config& config, const BeAmdPalMetaData::PipelineMetaData& amdpal_pipeline_md) override;

    // Perform post-compile actions for raytracing workflows.
    bool RunPostCompileSteps(const Config& config) override;


private:
    // Store output file names to the output metadata for raytracing workflows.
    void StoreOutputFilesToOutputMD(const Config&      config,
                                    const std::string& asic,
                                    const std::string& kernel,
                                    const std::string& kernel_subtype,
                                    const std::string& isa_filename);

    // Output Metadata for raytracing workflows.
    RgClOutputMetadata output_metadata_;

    // Log callback function.
    LoggingCallbackFunction log_callback_;
};

// Post-processing workflow strategy functions for compute workflows.
class ComputeBinaryWorkflowStrategy : public BinaryWorkflowStrategy
{
public:
    ComputeBinaryWorkflowStrategy(LoggingCallbackFunction log_callback)
        : log_callback_(log_callback)
    {}

    // Write Isa file(s) to disk for compute workflows.
    beKA::beStatus WriteOutputFiles(const Config&                             config,
                                    const std::string&                        asic,
                                    const std::map<std::string, std::string>& kernel_to_disassembly,
                                    const BeAmdPalMetaData::PipelineMetaData& amdpal_pipeline_md,
                                    std::string&                              error_msg) override;

    // Perform post-processing actions for compute workflows.
    void RunPostProcessingSteps(const Config& config, const BeAmdPalMetaData::PipelineMetaData& amdpal_pipeline_md) override;

        // Perform post-compile actions for compute workflows.
    bool RunPostCompileSteps(const Config& config) override;

private:

    // Store output file names to the output metadata for compute workflows.
    void StoreOutputFilesToOutputMD(const Config&      config,
                                    const std::string& asic,
                                    const std::string& kernel_name,
                                    const std::string& kernel_abbreviation,
                                    const std::string& isa_filename);

    // Output Metadata for compute workflows.
    RgClOutputMetadata output_metadata_;

    // Log callback function.
    LoggingCallbackFunction log_callback_;
};
#endif  // RGA_RADEONGPUANALYZERCLI_SRC_KC_CLI_COMMANDER_BIN_UTIL_H_
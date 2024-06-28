//=================================================================
// Copyright 2020 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

#ifndef RGA_RADEONGPUANALYZERBACKEND_SRC_BE_PROGRAM_BUILDER_DX12_H_
#define RGA_RADEONGPUANALYZERBACKEND_SRC_BE_PROGRAM_BUILDER_DX12_H_

#ifdef _WIN32

// C++.
#include <vector>
#include <map>
#include <string>

// CLI.
#include "source/radeon_gpu_analyzer_cli/kc_config.h"

// Backend.
#include "utils/dx12/backend/rg_dx12_data_types.h"

// Local.
#include "radeon_gpu_analyzer_backend/be_program_builder.h"
#include "radeon_gpu_analyzer_backend/be_data_types.h"
#include "radeon_gpu_analyzer_backend/autogen/be_include_dx12.h"

using namespace beKA;
using namespace rga;

class BeProgramBuilderDx12
{
public:
    BeProgramBuilderDx12() = default;
    ~BeProgramBuilderDx12(void);

    // Get the list of supported GPUs from the driver.
    beStatus GetSupportGpus(const Config& config, std::vector<std::string>& gpus, std::map<std::string, int>& driver_ids);

    // Compiles the Dx12 pipeline. This method is a wrapper around the older Compile method.
    // In addition to previous api, this method also has additional support for dx12 single shader autogeneration.
    beStatus CompileDX12Pipeline(const Config&      config,
                                 const std::string& target_device,
                                 std::string&       out_text,
                                 std::string&       error_msg,
                                 BeVkPipelineFiles& generated_isa_files,
                                 BeVkPipelineFiles& generated_amdil_files,
                                 BeVkPipelineFiles& generated_stat_files,
                                 std::string&       generated_binary_file);

    // Compile for DXR based on the user-provided options.
    beStatus CompileDXRPipeline(const Config&                      config,
                                const std::string&                 target_device,
                                std::string&                       out_text,
                                std::vector<RgDxrPipelineResults>& output_mapping,
                                std::string&                       error_msg);

    // Allows RGA to check and validate inputs for a Dx12 pipeline.
    // This function return true if all inputs are valid.
    // This functions also tries autogenerating other required files to complete the pipeline.
    bool ValidateAndGeneratePipeline(const Config& config, bool is_dxr_pipeline);

    // Generate livereg VGPR analysis file.
    void PerformLiveVgprAnalysis(const std::string& isa_file,
                                 const std::string& stage_name,
                                 const std::string& target,
                                 const Config&      config,
                                 bool&              is_ok) const;

    // Generate Livereg SGPR Analysis file.
    void PerformLiveSgprAnalysis(const std::string& isa_file,
                                 const std::string& stage_name,
                                 const std::string& target,
                                 const Config&      config,
                                 bool&              is_ok) const;

    // Generate per Block Cfg file.
    void GeneratePerBlockCfg(const std::string& isa_file,
                             const std::string& pipeline_name_dxr,
                             const std::string& stage_name,
                             const std::string& target,
                             const Config&      config_updated,
                             bool               is_dxr,
                             bool&              is_ok) const;

    // Generate per Instruction Cfg file.
    void GeneratePerInstructionCfg(const std::string& isa_file,
                                   const std::string& pipeline_name_dxr,
                                   const std::string& stage_name,
                                   const std::string& target,
                                   const Config&      config_updated,
                                   bool               is_dxr,
                                   bool&              is_ok) const;

    // Accepts a DXR pipeline name as reported in the DXR output metadata file and
    // returns true if this is a "NULL" pipeline (empty pipeline generated as a container in Shader mode).
    static bool IsDxrNullPipeline(const std::string& pipeline_name);

private:
    // Compile the pipeline based on the user-provided options.
    beStatus Compile(const Config&      config,
                     const std::string& target_device,
                     std::string&       out_text,
                     std::string&       error_msg,
                     BeVkPipelineFiles& generated_isa_files,
                     BeVkPipelineFiles& generated_amdil_files,
                     BeVkPipelineFiles& generated_stat_files,
                     std::string&       generated_binary_file);

    // Enable RGA's driver stack for a specific target. We do this before launching DX12Backend
    // which inherits the environment from the current process.
    bool EnableNullBackendForDevice(const Config& config, const std::string& device_name);

    // Print Autogeneration status message.
    void PrintAutoGenerationStatus();

    // Mapping between the codename and the driver ID.
    std::map<std::string, int> code_name_to_driver_id_;

    // Dxc autogenerated missing files for dx12 compilation.
    BeDx12AutoGenPipelineInfo dxc_autogen_pipeline_info_;
};

#endif
#endif // RGA_RADEONGPUANALYZERBACKEND_SRC_BE_PROGRAM_BUILDER_DX12_H_

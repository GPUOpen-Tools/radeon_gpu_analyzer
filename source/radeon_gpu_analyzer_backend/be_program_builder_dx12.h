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

using namespace beKA;
using namespace rga;

static const std::array<std::string, BePipelineStage::kCount> kStrDx12StageNames =
{
    "vertex",
    "hull",
    "domain",
    "geometry",
    "pixel",
    "compute"
};

class BeProgramBuilderDx12
{
public:
    BeProgramBuilderDx12() = default;
    ~BeProgramBuilderDx12(void) = default;

    // Get the list of supported GPUs from the driver.
    beStatus GetSupportGpus(const Config& config, std::vector<std::string>& gpus, std::map<std::string, int>& driver_ids);

    // Compile the pipeline based on the user-provided options.
    beStatus Compile(const Config& config, const std::string& target_device,
        std::string& out_text, std::string& error_msg, BeVkPipelineFiles& generated_isa_files,
        BeVkPipelineFiles& generated_stat_files, std::string& generated_binary_file);

    // Compile for DXR based on the user-provided options.
    beStatus CompileDXRPipeline(const Config& config, const std::string& target_device,
        std::string& out_text, std::vector<RgDxrPipelineResults>& output_mapping, std::string& error_msg);

private:
    // Enable RGA's driver stack for a specific target. We do this before launching DX12Backend
    // which inherits the environment from the current process.
    bool EnableNullBackendForDevice(const Config& config, const std::string& device_name);

    // Mapping between the codename and the driver ID.
    std::map<std::string, int> code_name_to_driver_id_;
};

#endif
#endif // RGA_RADEONGPUANALYZERBACKEND_SRC_BE_PROGRAM_BUILDER_DX12_H_

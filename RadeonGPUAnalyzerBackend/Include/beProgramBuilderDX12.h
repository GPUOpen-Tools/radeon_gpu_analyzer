//=================================================================
// Copyright 2019 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

#ifdef _WIN32

// C++.
#include <vector>
#include <map>
#include <string>

// CLI.
#include <RadeonGPUAnalyzerCLI/Src/kcConfig.h>

// Local.
#include <RadeonGPUAnalyzerBackend/Include/beProgramBuilder.h>
#include <RadeonGPUAnalyzerBackend/Include/beDataTypes.h>
using namespace beKA;

#pragma once

static const std::array<std::string, bePipelineStage::Count>
STR_DX12_STAGE_NAMES =
{
    "vertex",
    "hull",
    "domain",
    "geometry",
    "pixel",
    "compute"
};

class beProgramBuilderDX12
{
public:
    beProgramBuilderDX12() = default;
    ~beProgramBuilderDX12(void) = default;

    // Get the list of supported GPUs from the driver.
    beStatus GetSupportGpus(const Config& config, std::vector<std::string>& gpus, std::map<std::string, int>& driverIds);

    // Compile the pipeline based on the user-provided options.
    beStatus Compile(const Config& config, const std::string& targetDevice,
        std::string& outText, std::string& errorMsg, beVkPipelineFiles& generatedIsaFiles,
        beVkPipelineFiles& generatedStatFiles);

private:
    // Mapping between the codename and the driver ID.
    std::map<std::string, int> m_codeNameToDriverId;
};

#endif

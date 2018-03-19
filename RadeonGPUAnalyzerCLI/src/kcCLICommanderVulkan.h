//=================================================================
// Copyright 2017 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================
#pragma once

// C++.
#include <string>
#include <sstream>
#include <vector>
#include <set>

// Local.
#include <RadeonGPUAnalyzerCLI/src/kcCLICommander.h>
#include <RadeonGPUAnalyzerCLI/src/kcDataTypes.h>

// Backend.
#include <RadeonGPUAnalyzerBackend/include/beProgramBuilderVulkan.h>

class kcCLICommanderVulkan :
    public kcCLICommander
{
public:
    kcCLICommanderVulkan();
    virtual ~kcCLICommanderVulkan();

    // Print the Vulkan version.
    virtual void Version(Config& config, LoggingCallBackFunc_t callback) override;

    // Execute the build.
    virtual void RunCompileCommands(const Config& config, LoggingCallBackFunc_t callback) override;

private:

    // Caches the supported devices.
    bool GetSupportedDevices();

    // The builder.
    beProgramBuilderVulkan* m_pVulkanBuilder;

    // Unique collections of the device names.
    std::set<std::string> m_supportedDevicesCache;
};

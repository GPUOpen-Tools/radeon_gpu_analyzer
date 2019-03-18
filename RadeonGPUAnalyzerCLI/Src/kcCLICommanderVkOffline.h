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
#include <RadeonGPUAnalyzerCLI/Src/kcCLICommander.h>
#include <RadeonGPUAnalyzerCLI/Src/kcDataTypes.h>

// Backend.
#include <RadeonGPUAnalyzerBackend/Include/beProgramBuilderVkOffline.h>

class kcCLICommanderVkOffline : public kcCLICommander
{
public:
    kcCLICommanderVkOffline();
    virtual ~kcCLICommanderVkOffline();

    // Print the Vulkan version.
    virtual void Version(Config& config, LoggingCallBackFunc_t callback) override;

    // Execute the build.
    virtual void RunCompileCommands(const Config& config, LoggingCallBackFunc_t callback) override;

private:

    // Caches the supported devices.
    bool GetSupportedDevices();

    // The builder.
    beProgramBuilderVkOffline* m_pVulkanBuilder;

    // Unique collections of the device names.
    std::set<std::string> m_supportedDevicesCache;
};

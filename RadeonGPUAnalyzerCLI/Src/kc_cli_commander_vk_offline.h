//=================================================================
// Copyright 2020 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================
#ifndef RGA_RADEONGPUANALYZERCLI_SRC_KC_CLI_COMMANDER_VK_OFFLINE_H_
#define RGA_RADEONGPUANALYZERCLI_SRC_KC_CLI_COMMANDER_VK_OFFLINE_H_

// C++.
#include <string>
#include <sstream>
#include <vector>
#include <set>

// Local.
#include "RadeonGPUAnalyzerCLI/Src/kc_cli_commander.h"

// Backend.
#include "RadeonGPUAnalyzerBackend/Src/be_program_builder_vk_offline.h"

class KcCLICommanderVkOffline : public KcCliCommander
{
public:
    KcCLICommanderVkOffline();
    virtual ~KcCLICommanderVkOffline();

    // Print the Vulkan version.
    virtual void Version(Config& config, LoggingCallbackFunction callback) override;

    // Execute the build.
    virtual void RunCompileCommands(const Config& config, LoggingCallbackFunction callback) override;

private:

    // Caches the supported devices.
    bool GetSupportedDevices();

    // The builder.
    BeProgramBuilderVkOffline* vulkan_builder_;

    // Unique collections of the device names.
    std::set<std::string> supported_devices_cache_;
};
#endif // RGA_RADEONGPUANALYZERCLI_SRC_KC_CLI_COMMANDER_VK_OFFLINE_H_

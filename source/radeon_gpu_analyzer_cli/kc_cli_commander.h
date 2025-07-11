//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for CLI Commander interface.
//=============================================================================
#ifndef RGA_RADEONGPUANALYZERCLI_SRC_KC_CLI_COMMANDER_H_
#define RGA_RADEONGPUANALYZERCLI_SRC_KC_CLI_COMMANDER_H_

// C++.
#include <string>
#include <set>
#include <functional>

// Local.
#include "radeon_gpu_analyzer_cli/kc_config.h"
#include "radeon_gpu_analyzer_cli/kc_utils.h"

class KcCliCommander
{
public:

    virtual ~KcCliCommander() {};

    // List the adapters installed on the system.
    virtual void ListAdapters(Config& config, LoggingCallbackFunction callback);

    // Print the driver version.
    virtual void Version(Config& config, LoggingCallbackFunction callback);

    // Print list of supported devices.
    virtual bool PrintAsicList(const Config&)
    {
        return KcUtils::PrintAsicList();
    }

    // Perform compilation steps.
    virtual void RunCompileCommands(const Config& config, LoggingCallbackFunction callback) = 0;

    // Perform post-compile actions.
    virtual bool RunPostCompileSteps(const Config& config);

    // Generates the RGA CLI version info file with the provided name.
    static bool GenerateVersionInfoFile(const Config& config);

    // Parse the source file and extract list of entry points (for example, kernels for OpenCL).
    // Dump the extracted entry points to stdout.
    virtual bool ListEntries(const Config& config, LoggingCallbackFunction callback);

protected:
    // -- Functions --

    // Initialize the list of GPU targets based on device(s) specified in "devices".
    // "supportedDevices" is the list of supported devices for the given mode. Only targets from this list will be considered.
    // The matched devices are returned in "matchedDevices" set.
    // If "allowUnknownDevice" is true, no error message will be printed if no matched devices are found.
    static bool InitRequestedAsicList(const std::vector<std::string>& devices,
                                      beKA::RgaMode                   mode,
                                      const std::set<std::string>&    supported_devices,
                                      std::set<std::string>&          matched_devices, 
                                      bool                            allow_unknown_device);

    // Logging callback type.
    bool LogCallback(const std::string& str) const;

    // -- Data --

    // Log callback function
    LoggingCallbackFunction   log_callback_;
};

#endif // RGA_RADEONGPUANALYZERCLI_SRC_KC_CLI_COMMANDER_H_

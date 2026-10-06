//=============================================================================
/// Copyright (c) 2024-2026 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for CLI Commander interface for binary code objects.
//=============================================================================
#ifndef RGA_RADEONGPUANALYZERCLI_SRC_KC_CLI_COMMANDER_BINARY_H_
#define RGA_RADEONGPUANALYZERCLI_SRC_KC_CLI_COMMANDER_BINARY_H_

// C++.
#include <map>

// Local.
#include "radeon_gpu_analyzer_cli/kc_cli_commander.h"
#include "radeon_gpu_analyzer_cli/kc_cli_binary_analysis.h"

// Commander interface for binary code objects.
class KcCliCommanderBinary : public KcCliCommander
{
public:
    // Default constructor.
    KcCliCommanderBinary() = default;

    // RunCompileCommands function is called as such mostly for legacy reasons.
    // This function has no compiling step. It generates disassembly, and post-processing analyses.
    virtual void RunCompileCommands(const Config& config, LoggingCallbackFunction callback) override;

    // Perform post-compile actions.
    virtual bool RunPostCompileSteps(const Config& config) override;

    // List source file paths from DWARF debug info.
    virtual bool ListSourcePaths(const Config& config, LoggingCallbackFunction callback) override;

    // Generates Binary Analysis "version info" data and writes it to the file specified by "filename".
    // The data will be appended to the existing content of the file.
    static bool GenerateBinaryAnalysisVersionInfo(const std::string& filename);

private:
    // Get the list of names of supported targets in DeviceInfo format.
    static beKA::beStatus GetSupportedTargets(std::set<std::string>& targets);

    // Identify the devices requested by user.
    beKA::beStatus InitRequestedAsicBinary(const Config&                config,
                                           bool                         verbose,
                                           const std::set<std::string>& supported_devices,
                                           const std::string&           binary_codeobj_file,
                                           const std::string&           target_device,
                                           std::set<std::string>&       matched_targets);

    // Generate metadata file for the current cli session.
    bool GenerateSessionMetadataFile(const Config& config);

    // Generate metadata file for the current cli session.
    bool GenerateSessionSummary(const Config& config);

    // Maps input binary file to its binary analysis.
    std::map<std::string, KcCliBinaryAnalysis> binary_file_to_binary_analysis_map_;
};

#endif  // RGA_RADEONGPUANALYZERCLI_SRC_KC_CLI_COMMANDER_BINARY_H_

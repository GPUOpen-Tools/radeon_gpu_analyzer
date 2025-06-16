//=============================================================================
/// Copyright (c) 2024-2025 Advanced Micro Devices, Inc. All rights reserved.
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

    // Generates Binary Analysis "version info" data and writes it to the file specified by "filename".
    // The data will be appended to the existing content of the file.
    static bool GenerateBinaryAnalysisVersionInfo(const std::string& filename);

    // Get the list of names of supported targets in DeviceInfo format.
    static beKA::beStatus GetSupportedTargets(std::set<std::string>& targets);

private:
    // Validate .bin input file.
    beKA::beStatus IsBinaryInputValid(const Config& config, bool verbose, const std::string& binary_codeobj_file) const;

    // Invoke the amdgpu-dis executable and write out-text to a file on disk.
    beKA::beStatus DisassembleBinary(const std::string& bin_file, bool verbose, std::string& out_text, std::string& error_txt) const;

    // Extract target device from ISA disassembly.
    static bool ExtractDeviceFromAmdgpudisOutput(const std::string& amdgpu_dis_output, std::string& device);

    // Identify the devices requested by user.
    beKA::beStatus InitRequestedAsicBinary(const Config&                config,
                                           bool                         verbose,
                                           const std::set<std::string>& supported_devices,
                                           const std::string&           binary_codeobj_file,
                                           const std::string&           amdgpu_dis_output,
                                           std::set<std::string>&       matched_targets);

    // Maps input binary file to its binary analysis.
    KcCliBinaryAnalysis::Map binary_file_to_binary_analysis_map_;
};

#endif  // RGA_RADEONGPUANALYZERCLI_SRC_KC_CLI_COMMANDER_BINARY_H_

//=================================================================
// Copyright 2020 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================
#ifndef RGA_RADEONGPUANALYZERCLI_SRC_KC_CLI_COMMANDER_CL_H_
#define RGA_RADEONGPUANALYZERCLI_SRC_KC_CLI_COMMANDER_CL_H_

// C++.
#include <string>
#include <set>

// Local.
#include "radeon_gpu_analyzer_cli/kc_cli_commander.h"
#include "radeon_gpu_analyzer_cli/kc_data_types.h"

using namespace std;

class KcCliCommanderCL: public KcCliCommander
{
public:

    KcCliCommanderCL();
    virtual ~KcCliCommanderCL();

    // Parse the source file and extract list of entry points (for example, kernels for OpenCL).
    // Dump the extracted entry points to stdout.
    virtual bool ListEntries(const Config& config, LoggingCallbackFunction callback) override;

    // Output multiple commands for all commands that requires compilation: GetBinary, GetIL, GetISA, GetAnlysis, GetMetadata, GetDebugIL,ListKernels
    void RunCompileCommands(const Config& config, LoggingCallbackFunction callback);

    // Print the list of target devices.
    virtual bool PrintAsicList(const Config&) override;

private:
    bool Init(const Config& config, LoggingCallbackFunction callback);
    bool Compile(const Config& config);

    // output for all commands that requires compilation
    void Analysis(const Config& config);
    void GetILText(const Config& config);
    void GetISAText(const Config& config);
    void GetBinary(const Config& config);
    void GetMetadata(const Config& config);

    // Returns the list of required kernels according to the user's configurations.
    // \param[in]  config - the configuration as given by the user.
    // \param[out] requiredKernels - a container of the required kernels.
    void InitRequiredKernels(const Config& config, const std::vector<std::string>& required_devices, std::vector<std::string>& required_kernels);

    // Writes a statistics CSV file.
    void WriteAnalysisFile(const Config& config, const std::string& kernel_name,
        const std::string& device_name, const beKA::AnalysisData& analysis);

private:
    std::set<string>             external_devices_;
    std::vector<GDT_GfxCardInfo> table_;
    std::set<std::string>        asics_;
    std::vector<std::string>     asics_sorted_;
    Backend* be_;

    // Holds the name of the kernels to be built.
    std::vector<std::string> required_kernels_;

    // True if the "--kernel all" option was specified by the user.
    bool is_all_kernels_;
};

#endif // RGA_RADEONGPUANALYZERCLI_SRC_KC_CLI_COMMANDER_CL_H_

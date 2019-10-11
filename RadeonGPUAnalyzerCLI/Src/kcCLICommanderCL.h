//=================================================================
// Copyright 2017 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

#pragma once

// C++.
#include <string>
#include <set>

// Local.
#include <RadeonGPUAnalyzerCLI/Src/kcCLICommander.h>
#include <RadeonGPUAnalyzerCLI/Src/kcDataTypes.h>

using namespace std;

/// This is the Commander interface
class kcCLICommanderCL: public kcCLICommander
{
public:

    kcCLICommanderCL();
    virtual ~kcCLICommanderCL();

    // Parse the source file and extract list of entry points (for example, kernels for OpenCL).
    // Dump the extracted entry points to stdout.
    virtual bool ListEntries(const Config& config, LoggingCallBackFunc_t callback) override;

    // Output multiple commands for all commands that requires compilation: GetBinary, GetIL, GetISA, GetAnlysis, GetMetadata, GetDebugIL,ListKernels
    void RunCompileCommands(const Config& config, LoggingCallBackFunc_t callback);

    // Print the list of target devices.
    virtual bool PrintAsicList(const Config&) override;

private:
    bool Init(const Config& config, LoggingCallBackFunc_t callback);
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
    void InitRequiredKernels(const Config& config, const std::vector<std::string>& requiredDevices, std::vector<std::string>& requiredKernels);

    // Writes a statistics CSV file.
    void WriteAnalysisFile(const Config& config, const std::string& kernelName,
        const std::string& deviceName, const beKA::AnalysisData& analysis);

private:
    std::set<string>             m_externalDevices;
    std::vector<GDT_GfxCardInfo> m_table;
    std::set<std::string>        m_asics;
    std::vector<std::string>     m_asicsSorted;

    // Holds the name of the kernels to be built.
    std::vector<std::string> m_requiredKernels;

    // True if the "--kernel all" option was specified by the user.
    bool m_isAllKernels;

    Backend* be;
};

//=================================================================
// Copyright 2019 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

#pragma once

#ifdef _WIN32

// Local.
#include <RadeonGPUAnalyzerCLI/Src/kcCLICommander.h>
#include <RadeonGPUAnalyzerCLI/Src/kcConfig.h>
#include <RadeonGPUAnalyzerCLI/Src/kcDataTypes.h>

// Backend.
#include <RadeonGPUAnalyzerBackend/Include/beProgramBuilderDX12.h>

class kcCLICommanderDX12 : public kcCLICommander
{
public:
    kcCLICommanderDX12(void) = default;
    virtual ~kcCLICommanderDX12(void) = default;

    // List the adapters installed on the system.
    void ListAdapters(Config& config, LoggingCallBackFunc_t callback) override;

    // Perform the compilation.
    void RunCompileCommands(const Config& config, LoggingCallBackFunc_t callback);

    // Print the list of supported targets.
    virtual bool PrintAsicList(const Config&) override;

private:

    std::vector<GDT_GfxCardInfo> m_dxDefaultAsicsList;
    beProgramBuilderDX12 m_dx12Backend;;

};

#endif

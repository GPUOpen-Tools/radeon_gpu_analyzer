//=================================================================
// Copyright 2019 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

#ifndef RGA_RADEONGPUANALYZERCLI_SRC_KC_CLI_COMMANDER_DX12_H_
#define RGA_RADEONGPUANALYZERCLI_SRC_KC_CLI_COMMANDER_DX12_H_

#pragma once

#ifdef _WIN32

// Local.
#include "RadeonGPUAnalyzerCLI/Src/kc_cli_commander.h"
#include "RadeonGPUAnalyzerCLI/Src/kc_config.h"
#include "RadeonGPUAnalyzerCLI/Src/kc_data_types.h"

// Backend.
#include "RadeonGPUAnalyzerBackend/Src/be_program_builder_dx12.h"

class KcCliCommanderDX12 : public KcCliCommander
{
public:
    KcCliCommanderDX12(void) = default;
    virtual ~KcCliCommanderDX12(void) = default;

    // List the adapters installed on the system.
    void ListAdapters(Config& config, LoggingCallbackFunction callback) override;

    // Perform the compilation.
    void RunCompileCommands(const Config& config, LoggingCallbackFunction callback);

    // Print the list of supported targets.
    virtual bool PrintAsicList(const Config&) override;

private:
    std::vector<GDT_GfxCardInfo> dx_default_asic_list_;
    BeProgramBuilderDx12 dx12_backend_;;
};

#endif
#endif // RGA_RADEONGPUANALYZERCLI_SRC_KC_CLI_COMMANDER_DX12_H_

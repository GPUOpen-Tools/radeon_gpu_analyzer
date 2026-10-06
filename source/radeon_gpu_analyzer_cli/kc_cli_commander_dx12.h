//=============================================================================
/// Copyright (c) 2019-2026 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for CLI Commander interface for compiling for DX12.
//=============================================================================

#ifndef RGA_RADEONGPUANALYZERCLI_SRC_KC_CLI_COMMANDER_DX12_H_
#define RGA_RADEONGPUANALYZERCLI_SRC_KC_CLI_COMMANDER_DX12_H_

#pragma once

#ifdef _WIN32

// Common.
#include "common/rga_analysis_summary.h"

// Local.
#include "radeon_gpu_analyzer_cli/kc_cli_commander.h"
#include "radeon_gpu_analyzer_cli/kc_config.h"
#include "radeon_gpu_analyzer_cli/kc_data_types.h"

// Backend.
#include "radeon_gpu_analyzer_backend/be_program_builder_dx12.h"

class KcCliCommanderDX12 : public KcCliCommander
{
public:
    KcCliCommanderDX12(void)          = default;
    virtual ~KcCliCommanderDX12(void) = default;

    // List the adapters installed on the system.
    void ListAdapters(Config& config, LoggingCallbackFunction callback) override;

    // Perform the compilation.
    void RunCompileCommands(const Config& config, LoggingCallbackFunction callback);

    // Print the list of supported targets.
    virtual bool PrintAsicList(const Config& config) override;

private:
    bool GetDX12DriverAsicList(const Config& config, std::set<std::string>& target_gpus, bool print = false);

    // DXR: extract metadata and disassembly from each pipeline binary via llvm-readobj + llvm-objdump,
    // then feed into the raytracing analysis strategy for ISA output, stats, and post-processing.
    bool PostProcessDxrPipelineBinary(const Config& config, const std::string& target, const std::string& pipeline_elf, std::string& error_msg);

    RgaAnalysisSummary           summary_;
    std::vector<GDT_GfxCardInfo> dx_default_asic_list_;
    BeProgramBuilderDx12         dx12_backend_;
};

#endif
#endif  // RGA_RADEONGPUANALYZERCLI_SRC_KC_CLI_COMMANDER_DX12_H_

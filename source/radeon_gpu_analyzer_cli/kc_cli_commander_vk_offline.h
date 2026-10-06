//=============================================================================
/// Copyright (c) 2020-2026 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for CLI Commander interface for compiling with the Vk-offline Compiler (amdllpc).
//=============================================================================
#ifndef RGA_RADEONGPUANALYZERCLI_SRC_KC_CLI_COMMANDER_VK_OFFLINE_H_
#define RGA_RADEONGPUANALYZERCLI_SRC_KC_CLI_COMMANDER_VK_OFFLINE_H_

// C++.
#include <string>
#include <sstream>
#include <vector>
#include <set>

// Local.
#include "radeon_gpu_analyzer_cli/kc_cli_commander.h"

// Backend.
#include "radeon_gpu_analyzer_backend/be_program_builder_vk_offline.h"

class KcCLICommanderVkOffline : public KcCliCommander
{
public:
    KcCLICommanderVkOffline();
    virtual ~KcCLICommanderVkOffline();

    // Print the list of supported devices.
    bool PrintAsicList(const Config&) override;

    // Print the Vulkan version.
    virtual void Version(Config& config, LoggingCallbackFunction callback) override;

    // Execute the build.
    virtual void RunCompileCommands(const Config& config, LoggingCallbackFunction callback) override;

    // Perform post-compile actions (generate session metadata).
    virtual bool RunPostCompileSteps(const Config& config) override;

private:

    // Caches the supported devices.
    bool GetSupportedDevices();

    // Store output file names to the output metadata.
    void StoreOutputFilesToOutputMD(const std::string&               device,
                                    const BePipelineFiles&         spv_files,
                                    const BePipelineFiles&         isa_files,
                                    const BePipelineFiles&         stats_files,
                                    const BeVkPipelineWaveSizes&     wave_sizes,
                                    const BeVkPipelineShaderHashes&  shader_hashes);

    // Generate the XML session summary file.
    bool GenerateSessionSummary(const Config& config);

    // The builder.
    BeProgramBuilderVkOffline* vulkan_builder_;

    // Unique collections of the device names.
    std::set<std::string> supported_devices_cache_;

    // Per-device output metadata.
    std::map<std::string, RgVkOutputMetadata> output_metadata_;
};
#endif // RGA_RADEONGPUANALYZERCLI_SRC_KC_CLI_COMMANDER_VK_OFFLINE_H_

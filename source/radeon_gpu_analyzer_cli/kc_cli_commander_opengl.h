//=============================================================================
/// Copyright (c) 2020-2026 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for CLI Commander interface for compiling with the OpenGL Compiler (glc).
//=============================================================================
#ifndef RGA_RADEONGPUANALYZERCLI_SRC_KC_CLI_COMMANDER_OPENGL_H_
#define RGA_RADEONGPUANALYZERCLI_SRC_KC_CLI_COMMANDER_OPENGL_H_

// C++.
#include <map>

// Local.
#include "radeon_gpu_analyzer_cli/kc_cli_commander.h"
#include "radeon_gpu_analyzer_cli/kc_data_types.h"

// Backend.
#include "radeon_gpu_analyzer_backend/be_program_builder_opengl.h"

class KcCliCommanderOpenGL :
    public KcCliCommander
{
public:
    KcCliCommanderOpenGL();
    virtual ~KcCliCommanderOpenGL();

    virtual void Version(Config& config, LoggingCallbackFunction callback) override;

    virtual bool  PrintAsicList(const Config&) override;

    virtual void RunCompileCommands(const Config& config, LoggingCallbackFunction callback) override;

private:
    // Create glc stats file.
    void CreateStatisticsFile(const gtString&         statistics_file,
                                     const Config&           config,
                                     const std::string&      device,
                                     IStatisticsParser&      stats_parser,
                                     LoggingCallbackFunction log_cb);

    // Delete the specified file.
    bool DeleteFile(const gtString& file_full_path);

    // Store output file names to the output metadata for a device.
    void StoreOutputFilesToOutputMD(const std::string& device, const OpenglOptions& gl_options, const Config& config);

    // Generate the XML session summary file.
    bool GenerateSessionSummary(const Config& config);

    // The builder.
    BeProgramBuilderOpengl* ogl_builder_;

    // Unique collections of the device names.
    std::set<std::string> supported_devices_cache_;

    // Per-device output metadata.
    std::map<std::string, RgVkOutputMetadata> output_metadata_;

    // Forward declaration: an internal structure representing a target device's info.
    struct OpenglDeviceInfo;
};

#endif // RGA_RADEONGPUANALYZERCLI_SRC_KC_CLI_COMMANDER_OPENGL_H_

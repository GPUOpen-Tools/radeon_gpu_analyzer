//=================================================================
// Copyright 2020 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================
#ifndef RGA_RADEONGPUANALYZERCLI_SRC_KC_CLI_COMMANDER_OPENGL_H_
#define RGA_RADEONGPUANALYZERCLI_SRC_KC_CLI_COMMANDER_OPENGL_H_

// C++.
#include <map>

// Local.
#include "RadeonGPUAnalyzerCLI/Src/kc_cli_commander.h"

// Backend.
#include "RadeonGPUAnalyzerBackend/Src/be_program_builder_opengl.h"

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
    // The builder.
    BeProgramBuilderOpengl* ogl_builder_;

    // Unique collections of the device names.
    std::set<std::string> supported_devices_cache_;

    // Forward declaration: an internal structure representing a target device's info.
    struct OpenglDeviceInfo;
};

#endif // RGA_RADEONGPUANALYZERCLI_SRC_KC_CLI_COMMANDER_OPENGL_H_

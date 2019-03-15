//=================================================================
// Copyright 2017 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

#pragma once

// C++.
#include <map>

// Local.
#include <RadeonGPUAnalyzerCLI/Src/kcCLICommander.h>
#include <RadeonGPUAnalyzerCLI/Src/kcDataTypes.h>

// Backend.
#include <RadeonGPUAnalyzerBackend/Include/beProgramBuilderOpenGL.h>

class kcCLICommanderOpenGL :
    public kcCLICommander
{
public:
    kcCLICommanderOpenGL();
    virtual ~kcCLICommanderOpenGL();

    virtual void Version(Config& config, LoggingCallBackFunc_t callback) override;

    virtual bool  PrintAsicList(const Config&) override;

    virtual void RunCompileCommands(const Config& config, LoggingCallBackFunc_t callback) override;

private:
    // The builder.
    beProgramBuilderOpenGL* m_pOglBuilder;

    // Unique collections of the device names.
    std::set<std::string> m_supportedDevicesCache;

    // An internal structure representing a target device's info.
    struct OpenGLDeviceInfo;
};

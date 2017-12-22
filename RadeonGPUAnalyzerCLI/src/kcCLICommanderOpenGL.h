//=================================================================
// Copyright 2017 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

#ifndef kcCLICommanderOpenGL_h__
#define kcCLICommanderOpenGL_h__

// C++.
#include <map>

// Local.
#include <RadeonGPUAnalyzerCLI/src/kcCLICommander.h>
#include <RadeonGPUAnalyzerCLI/src/kcDataTypes.h>

// Backend.
#include <RadeonGPUAnalyzerBackend/include/beProgramBuilderOpenGL.h>

class kcCLICommanderOpenGL :
    public kcCLICommander
{
public:
    kcCLICommanderOpenGL();
    virtual ~kcCLICommanderOpenGL();

    virtual void Version(Config& config, LoggingCallBackFunc_t callback) override;

    virtual bool  PrintAsicList(std::ostream& log) override;

    virtual void RunCompileCommands(const Config& config, LoggingCallBackFunc_t callback) override;

private:
    // The builder.
    beProgramBuilderOpenGL* m_pOglBuilder;

    // Unique collections of the device names.
    std::set<std::string> m_supportedDevicesCache;

    // An internal structure representing a target device's info.
    struct OpenGLDeviceInfo;
};

#endif // kcCLICommanderOpenGL_h__

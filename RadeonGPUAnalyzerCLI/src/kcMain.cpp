//=================================================================
// Copyright 2017 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

#include <memory>
#include <map>
#include <utility>
#include <sstream>
#include <boost/algorithm/string.hpp>

// Infra.
#include <AMDTOSWrappers/Include/osEnvironmentVariable.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <Utils/include/rgLog.h>

// Backend.
#include <RadeonGPUAnalyzerBackend/include/beProgramBuilderOpenCL.h>

// Local.
#include <RadeonGPUAnalyzerCLI/src/kcConfig.h>
#include <RadeonGPUAnalyzerCLI/src/kcParseCmdLine.h>
#include <RadeonGPUAnalyzerCLI/src/kcCLICommanderCL.h>
#include <RadeonGPUAnalyzerCLI/src/kcCliStringConstants.h>
#include <RadeonGPUAnalyzerCLI/src/kcCLICommanderOpenGL.h>
#include <RadeonGPUAnalyzerCLI/src/kcCLICommanderVulkan.h>
#include <RadeonGPUAnalyzerCLI/src/kcUtils.h>
#include <RadeonGPUAnalyzerCLI/src/kcCLICommanderLightning.h>

#ifdef _WIN32
    #include <RadeonGPUAnalyzerCLI/src/kcCLICommanderDX.h>
#endif

using namespace beKA;
static std::ostream& s_Log = cout;

static void loggingCallback(const string& s)
{
    s_Log << s.c_str();
}

int main(int argc, char* argv[])
{
    bool   status = true;
    Config config;
    std::stringstream  msg;
#ifdef _WIN64
    // Enable 64-bit build for OpenCL.
    osEnvironmentVariable envVar64Bit(STR_OCL_ENV_VAR_GPU_FORCE_64BIT_PTR_NAME, STR_OCL_ENV_VAR_GPU_FORCE_64BIT_PTR_VALUE);
    osSetCurrentProcessEnvVariable(envVar64Bit);
#endif // _WIN64

    status = status && ParseCmdLine(argc, argv, config);
    status = status && kcUtils::InitCLILogFile(config);

    // Create corresponding Commander object.
    std::shared_ptr<kcCLICommander> pCommander = nullptr;

    if (status)
    {
        switch (config.m_SourceLanguage)
        {
        case SourceLanguage_None:
            if (config.m_RequestedCommand == Config::ccVersion)
            {
                kcUtils::PrintRgaVersion();
            }
            break;

        case SourceLanguage_OpenCL:
            pCommander = std::make_shared<kcCLICommanderCL>();
            break;

#ifdef _WIN32
        case SourceLanguage_HLSL:
        case SourceLanguage_AMDIL:
        case SourceLanguage_DXasm:
        case SourceLanguage_DXasmT:
            pCommander = std::make_shared<kcCLICommanderDX>();
            break;
#endif

        case SourceLanguage_GLSL:
            rgLog::stdOut << STR_ERR_GLSL_MODE_DEPRECATED << std::endl;
            status = false;
            break;

        case SourceLanguage_GLSL_OpenGL:
            pCommander = std::make_shared<kcCLICommanderOpenGL>();
            break;

        case SourceLanguage_GLSL_Vulkan:
        case SourceLanguage_SPIRV_Vulkan:
        case SourceLanguage_SPIRVTXT_Vulkan:
            pCommander = std::make_shared<kcCLICommanderVulkan>();
            break;

        case SourceLanguage_Rocm_OpenCL:
        {
            pCommander = std::make_shared<kcCLICommanderLightning>();
            if (static_cast<kcCLICommanderLightning&>(*pCommander).Init(config, loggingCallback) != beKA::beStatus_SUCCESS)
            {
                rgLog::stdOut << STR_ERR_INITIALIZATION_FAILURE << std::endl;
                status = false;
            }
            break;
        }
        }
    }

    // Perform requested actions.
    if (status && pCommander != nullptr)
    {
        switch (config.m_RequestedCommand)
        {
        case Config::ccCompile:
            pCommander->RunCompileCommands(config, loggingCallback);
            // Perform post-compile steps
            pCommander->RunPostCompileSteps(config);
            break;

        case Config::ccListKernels:
            kcCLICommanderLightning::ListEntries(config, loggingCallback);
            break;

        case Config::ccListAsics:
            if (!pCommander->PrintAsicList(s_Log))
            {
                rgLog::stdOut << STR_ERR_CANNOT_EXTRACT_SUPPORTED_DEVICE_LIST << std::endl;
                status = false;
            }
            break;

        case Config::ccListAdapters:
            pCommander->ListAdapters(config, loggingCallback);
            break;

        case Config::ccVersion:
            pCommander->Version(config, loggingCallback);
            break;

        case Config::ccGenVersionInfoFile:
            kcUtils::GenerateVersionInfoFile(config.m_versionInfoFile);
            break;

        case Config::ccInvalid:
            rgLog::stdOut << STR_ERR_NO_VALID_CMD_DETECTED << std::endl;
            status = false;
            break;
        }
    }

    rgLog::Close();

    return 0;
}

//=================================================================
// Copyright 2017 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

#include <memory>
#include <map>
#include <utility>
#include <sstream>

// Infra.
#ifdef _WIN32
    #pragma warning(push)
    #pragma warning(disable:4309)
#endif
#include <AMDTOSWrappers/Include/osEnvironmentVariable.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <Utils/Include/rgLog.h>
#ifdef _WIN32
    #pragma warning(pop)
#endif

// Backend.
#include <RadeonGPUAnalyzerBackend/Include/beProgramBuilderOpenCL.h>

// Local.
#include <RadeonGPUAnalyzerCLI/Src/kcConfig.h>
#include <RadeonGPUAnalyzerCLI/Src/kcParseCmdLine.h>
#include <RadeonGPUAnalyzerCLI/Src/kcCLICommanderCL.h>
#include <RadeonGPUAnalyzerCLI/Src/kcCliStringConstants.h>
#include <RadeonGPUAnalyzerCLI/Src/kcCLICommanderOpenGL.h>
#include <RadeonGPUAnalyzerCLI/Src/kcCLICommanderVkOffline.h>
#include <RadeonGPUAnalyzerCLI/Src/kcCLICommanderVulkan.h>
#include <RadeonGPUAnalyzerCLI/Src/kcUtils.h>
#include <RadeonGPUAnalyzerCLI/Src/kcCLICommanderLightning.h>

#ifdef _WIN32
    #include <RadeonGPUAnalyzerCLI/Src/kcCLICommanderDX.h>
    #include <RadeonGPUAnalyzerCLI/Src/kcCLICommanderDX12.h>
#endif

using namespace beKA;

static void loggingCallback(const string& s)
{
    rgLog::stdOut << s.c_str() << std::flush;
}

// Perform finalizing actions before exiting.
static void Shutdown()
{
    rgLog::file << KC_STR_CLI_LOG_CLOSE << std::endl;
    rgLog::Close();
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
        switch (config.m_mode)
        {
        case Mode_None:
            if (config.m_RequestedCommand == Config::ccVersion)
            {
                kcUtils::PrintRgaVersion();
            }
            break;

        case Mode_OpenCL:
            pCommander = std::make_shared<kcCLICommanderCL>();
            break;

#ifdef _WIN32
        case Mode_DX11:
        case Mode_AMDIL:
            pCommander = std::make_shared<kcCLICommanderDX>();
            break;
        case Mode_DX12:
            pCommander = std::make_shared<kcCLICommanderDX12>();
            break;
#endif

        case Mode_OpenGL:
            pCommander = std::make_shared<kcCLICommanderOpenGL>();
            break;

        case Mode_Vk_Offline:
        case Mode_Vk_Offline_Spv:
        case Mode_Vk_Offline_SpvTxt:
            pCommander = std::make_shared<kcCLICommanderVkOffline>();
            break;

        case Mode_Vulkan:
            pCommander = std::make_shared<kcCLICommanderVulkan>();
            break;

        case Mode_Rocm_OpenCL:
        {
            pCommander = std::make_shared<kcCLICommanderLightning>();
            if (static_cast<kcCLICommanderLightning&>(*pCommander).Init(config, loggingCallback) != beKA::beStatus_SUCCESS)
            {
                rgLog::stdErr << STR_ERR_INITIALIZATION_FAILURE << std::endl;
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

        case Config::ccListEntries:
            pCommander->ListEntries(config, loggingCallback);
            break;

        case Config::ccListAsics:
            if (!pCommander->PrintAsicList(config))
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

        case Config::ccInvalid:
            rgLog::stdOut << STR_ERR_NO_VALID_CMD_DETECTED << std::endl;
            status = false;
            break;
        }
    }
    else if (status && config.m_RequestedCommand == Config::ccGenVersionInfoFile)
    {
        kcCLICommander::GenerateVersionInfoFile(config);
    }

    Shutdown();

    return 0;
}

//=================================================================
// Copyright 2020 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

// C++.
#include <memory>
#include <map>
#include <utility>
#include <sstream>

// Infra.
#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4309)
#endif
#include "AMDTOSWrappers/Include/osEnvironmentVariable.h"
#include "AMDTOSWrappers/Include/osProcess.h"
#include "Utils/Include/rgLog.h"
#ifdef _WIN32
#pragma warning(pop)
#endif
// Backend.
#include "RadeonGPUAnalyzerBackend/Src/be_program_builder_opencl.h"
// Local.
#include "RadeonGPUAnalyzerCLI/Src/kc_config.h"
#include "RadeonGPUAnalyzerCLI/Src/kc_parse_cmd_line.h"
#include "RadeonGPUAnalyzerCLI/Src/kc_cli_commander_cl.h"
#include "RadeonGPUAnalyzerCLI/Src/kc_cli_string_constants.h"
#include "RadeonGPUAnalyzerCLI/Src/kc_cli_commander_opengl.h"
#include "RadeonGPUAnalyzerCLI/Src/kc_cli_commander_vk_offline.h"
#include "RadeonGPUAnalyzerCLI/Src/kc_cli_commander_vulkan.h"
#include "RadeonGPUAnalyzerCLI/Src/kc_utils.h"
#include "RadeonGPUAnalyzerCLI/Src/kc_cli_commander_lightning.h"
#ifdef _WIN32
#include "RadeonGPUAnalyzerCLI/Src/kc_cli_commander_dx11.h"
#include "RadeonGPUAnalyzerCLI/Src/kc_cli_commander_dx12.h"
#endif

using namespace beKA;

// Constants: error messages.
static const char* kStrErrInitializationFailure = "Error: failed to initialize.";

// Constants: log messages.
static const char* kStrRgaCliLogsEnd = "RGA CLI process finished.";

// Constants: environment variables.
static const wchar_t* kStrOpenclEnvVarGpuForce64BitPtrName = L"GPU_FORCE_64BIT_PTR";
static const wchar_t* kStrOpenclEnvVarGpuForce64BitPtrValue = L"1";

static void LoggingCallback(const string& s)
{
    rgLog::stdOut << s.c_str() << std::flush;
}

// Perform finalizing actions before exiting.
static void Shutdown()
{
    rgLog::file << kStrRgaCliLogsEnd << std::endl;
    rgLog::Close();
}

int main(int argc, char* argv[])
{
    bool status = true;
    Config config;
    std::stringstream msg;

#ifdef _WIN64
    // Enable 64-bit build for OpenCL.
    osEnvironmentVariable envVar64Bit(kStrOpenclEnvVarGpuForce64BitPtrName, kStrOpenclEnvVarGpuForce64BitPtrValue);
    osSetCurrentProcessEnvVariable(envVar64Bit);
#endif // _WIN64

    status = status && ParseCmdLine(argc, argv, config);
    status = status && KcUtils::InitCLILogFile(config);

    // Create corresponding Commander object.
    std::shared_ptr<KcCliCommander> commander = nullptr;
    if (status)
    {
        switch (config.mode)
        {
        case kModeNone:
            if (config.requested_command == Config::kVersion)
            {
                KcUtils::PrintRgaVersion();
            }
            break;
        case kModeOpencl:
            commander = std::make_shared<KcCliCommanderCL>();
            break;
#ifdef _WIN32
        case RgaMode::kModeDx11:
        case RgaMode::kModeAmdil:
            commander = std::make_shared<KcCliCommanderDX>();
            break;
        case kModeDx12:
        case kModeDxr:
            commander = std::make_shared<KcCliCommanderDX12>();
            break;
#endif
        case RgaMode::kModeOpengl:
            commander = std::make_shared<KcCliCommanderOpenGL>();
            break;
        case RgaMode::kModeVkOffline:
        case RgaMode::kModeVkOfflineSpv:
        case RgaMode::kModeVkOfflineSpvTxt:
            commander = std::make_shared<KcCLICommanderVkOffline>();
            break;
        case kModeVulkan:
            commander = std::make_shared<KcCliCommanderVulkan>();
            break;
        case RgaMode::kModeRocmOpencl:
        {
            commander = std::make_shared<KcCLICommanderLightning>();
            if (static_cast<KcCLICommanderLightning&>(*commander).Init(config, LoggingCallback) != beKA::kBeStatusSuccess)
            {
                rgLog::stdErr << kStrErrInitializationFailure << std::endl;
                status = false;
            }
            break;
        }
        }
    }
    // Perform requested actions.
    if (status && commander != nullptr)
    {
        switch (config.requested_command)
        {
        case Config::kCompile:
        case Config::kGenTemplateFile:
            commander->RunCompileCommands(config, LoggingCallback);
            // Perform post-compile steps
            commander->RunPostCompileSteps(config);
            break;
        case Config::kListEntries:
            commander->ListEntries(config, LoggingCallback);
            break;
        case Config::kListAsics:
            if (!commander->PrintAsicList(config))
            {
                rgLog::stdOut << kStrErrorCannotExtractSupportedDeviceList << std::endl;
                status = false;
            }
            break;
        case Config::kListAdapters:
            commander->ListAdapters(config, LoggingCallback);
            break;
        case Config::kVersion:
            commander->Version(config, LoggingCallback);
            break;
        case Config::kInvalid:
            rgLog::stdOut << kStrErrorNoValidCommandDetected << std::endl;
            status = false;
            break;
        }
    }
    else if (status && config.requested_command == Config::kGenVersionInfoFile)
    {
        KcCliCommander::GenerateVersionInfoFile(config);
    }

    Shutdown();
    return 0;
}
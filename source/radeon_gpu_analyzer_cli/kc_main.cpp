//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief CLI entry point.
//=============================================================================

// C++.
#include <memory>
#include <map>
#include <utility>
#include <sstream>

// Infra.
#include "external/amdt_os_wrappers/Include/osEnvironmentVariable.h"
#include "external/amdt_os_wrappers/Include/osProcess.h"
#include "common/rg_log.h"

// Backend.
#include "radeon_gpu_analyzer_backend/be_program_builder_opencl.h"
// Local.
#include "radeon_gpu_analyzer_cli/kc_config.h"
#include "radeon_gpu_analyzer_cli/kc_parse_cmd_line.h"
#include "radeon_gpu_analyzer_cli/kc_cli_commander_cl.h"
#include "radeon_gpu_analyzer_cli/kc_cli_string_constants.h"
#include "radeon_gpu_analyzer_cli/kc_cli_commander_opengl.h"
#include "radeon_gpu_analyzer_cli/kc_cli_commander_vk_offline.h"
#include "radeon_gpu_analyzer_cli/kc_cli_commander_vulkan.h"
#include "radeon_gpu_analyzer_cli/kc_cli_commander_binary.h"
#include "radeon_gpu_analyzer_cli/kc_utils.h"
#include "radeon_gpu_analyzer_cli/kc_cli_commander_lightning.h"
#ifdef _WIN32
#include "radeon_gpu_analyzer_cli/kc_cli_commander_dx11.h"
#include "radeon_gpu_analyzer_cli/kc_cli_commander_dx12.h"
#endif

using namespace beKA;

// Constants: error messages.
static const char* kStrErrInitializationFailure = "Error: failed to initialize.";

// Constants: log messages.
static const char* kStrRgaCliLogsEnd = "RGA CLI process finished.";

// Constants: environment variables.
static const wchar_t* kStrOpenclEnvVarGpuForce64BitPtrName = L"GPU_FORCE_64BIT_PTR";
static const wchar_t* kStrOpenclEnvVarGpuForce64BitPtrValue = L"1";

static void LoggingCallback(const std::string& s)
{
    RgLog::stdOut << s.c_str() << std::flush;
}

// Perform finalizing actions before exiting.
static void Shutdown()
{
    RgLog::file << kStrRgaCliLogsEnd << std::endl;
    RgLog::Close();
}

int main(int argc, char* argv[])
{
    bool status = true;
    Config config;
    std::stringstream msg;

#ifdef _WIN64

    // Update the PATH environment variable so that spawned processes inherit the VC++ runtime libraries path.
    KcUtils::UpdatePathEnvVar();

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
        case kModeBinary:
            commander = std::make_shared<KcCliCommanderBinary>();
            break;
        case RgaMode::kModeOpenclOffline:
        {
            commander = std::make_shared<KcCLICommanderLightning>();
            if (static_cast<KcCLICommanderLightning&>(*commander).Init(config, LoggingCallback) != beKA::kBeStatusSuccess)
            {
                RgLog::stdErr << kStrErrInitializationFailure << std::endl;
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
                RgLog::stdOut << kStrErrorCannotExtractSupportedDeviceList << std::endl;
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
            RgLog::stdOut << kStrErrorNoValidCommandDetected << std::endl;
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

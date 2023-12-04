//=================================================================
// Copyright 2020-2021 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

// C++
#include <cassert>

// Backend.
#include "radeon_gpu_analyzer_backend/be_utils.h"

// Local.
#include "radeon_gpu_analyzer_cli/kc_utils.h"
#include "radeon_gpu_analyzer_cli/kc_xml_writer.h"
#include "radeon_gpu_analyzer_cli/kc_cli_commander.h"
#include "radeon_gpu_analyzer_cli/kc_cli_commander_lightning.h"
#include "radeon_gpu_analyzer_cli/kc_cli_commander_vulkan.h"
#include "radeon_gpu_analyzer_cli/kc_cli_commander_bin.h"
#include "radeon_gpu_analyzer_cli/kc_cli_string_constants.h"

// Shared.
#include "common/rg_log.h"

// Constants.
static const char* kStrErrorTargetNotSupported                          = " compilation for the detected target GPU is not supported: ";
static const char* kStrErrorFailedToGenerateVersionInfoFile             = "Error: failed to generate the version info file.";
static const char* kStrErrorFailedToGenerateVersionInfoHeader           = "Error: failed to generate version info header.";
static const char* kStrErrorFailedToGenerateVersionInfoRocmcl           = "Error: failed to generate version info for Offline OpenCL mode.";
static const char* kStrErrorFailedToGenerateVersionInfoFileVulkanSystem = "Error: failed to generate system version info for Vulkan live-driver mode.";
static const char* kStrErrorFailedToGenerateVersionInfoFileVulkan       = "Error: failed to generate version info for Vulkan live-driver mode.";
static const char* kStrErrorFailedToGenerateVersionInfoBinaryAnalysis   = "Error: failed to generate version info for Binary Analysis mode.";

void KcCliCommander::Version(Config& config, LoggingCallbackFunction callback)
{
    KcUtils::PrintRgaVersion();
}

std::string  GetInputlLanguageString(beKA::RgaMode mode)
{
    switch (mode)
    {
    case beKA::RgaMode::kModeOpenclOffline:
        return kStrRgaModeOpenclOffline;
    case beKA::RgaMode::kModeOpengl:
        return kStrRgaModeOpengl;
    case beKA::RgaMode::kModeVulkan:
        return kStrRgaModeVulkan;
    case beKA::RgaMode::kModeVkOffline:
    case beKA::RgaMode::kModeVkOfflineSpv:
    case beKA::RgaMode::kModeVkOfflineSpvTxt:
        return kStrRgaModeVkOffline;
    case beKA::RgaMode::kModeDx11:
        return kStrRgaModeDx11;
    case beKA::RgaMode::kModeDx12:
        return kStrRgaModeDx12;
    case beKA::RgaMode::kModeDxr:
        return kStrRgaModeDxr;
    default:
        assert(false && kStrRgaModeErrorUnknownMode);
        return "";
    }
}

void KcCliCommander::ListAdapters(Config & config, LoggingCallbackFunction callback)
{
    GT_UNREFERENCED_PARAMETER(config);
    std::stringstream  msg;
    msg << kStrErrorCommandNotSupported << std::endl;
    callback(msg.str());
}

bool KcCliCommander::RunPostCompileSteps(const Config & config)
{
    return true;
}

bool KcCliCommander::GenerateVersionInfoFile(const Config& config)
{
    bool status = true;
    std::string filename = config.version_info_file;

    // If the file requires wrapping with double-quotes, do it.
    if (filename.find(' ') != std::string::npos &&
        filename.find("\"") != std::string::npos)
    {
        std::stringstream wrapped_filename;
        wrapped_filename << "\"" << filename << "\"";
        filename = wrapped_filename.str();
    }

    // Delete the version info file if it already exists.
    if (!filename.empty() && KcUtils::FileNotEmpty(filename))
    {
        BeUtils::DeleteFileFromDisk(filename);
    }

    // Generate the Version Info header.
    RgLog::stdErr << kStrInfoGeneratingVersionInfoFile << filename << std::endl;
    status = status && KcXmlWriter::AddVersionInfoHeader(filename);
    assert(status);

    if (status)
    {
        // Try generating the version info file for Binary Analysis mode.
        bool is_binary_analysis_version_info_generated = KcCliCommanderBin::GenerateBinaryAnalysisVersionInfo(filename);
        assert(is_binary_analysis_version_info_generated);
        if (!is_binary_analysis_version_info_generated)
        {
            RgLog::stdErr << kStrErrorFailedToGenerateVersionInfoBinaryAnalysis << std::endl;
        }

        // Try generating the version info file for OpenCL-Offline mode.
        bool is_opencl_offline_version_info_generated =
            KcCLICommanderLightning::GenerateOpenclOfflineVersionInfo(filename);
        assert(is_opencl_offline_version_info_generated);
        if (!is_opencl_offline_version_info_generated)
        {
            RgLog::stdErr << kStrErrorFailedToGenerateVersionInfoRocmcl << std::endl;
        }

        // Try generating the version info file for Vulkan live-driver mode.
        bool is_vulkan_version_info_generated =
            KcCliCommanderVulkan::GenerateVulkanVersionInfo(config,
                filename, config.print_process_cmd_line);

        if (is_vulkan_version_info_generated)
        {
            // Try generating system version info for Vulkan live-driver mode.
            is_vulkan_version_info_generated = KcCliCommanderVulkan::GenerateSystemVersionInfo(config,
                filename, config.print_process_cmd_line);
            assert(is_vulkan_version_info_generated);
            if (!is_vulkan_version_info_generated)
            {
                RgLog::stdErr << kStrErrorFailedToGenerateVersionInfoFileVulkanSystem << std::endl;
            }
        }
        else
        {
            RgLog::stdErr << kStrErrorFailedToGenerateVersionInfoFileVulkan << std::endl;
        }

        // We are good if at least one of the modes managed to generate the version info.
        status = is_binary_analysis_version_info_generated || is_opencl_offline_version_info_generated || is_vulkan_version_info_generated;
        assert(status);
    }
    else
    {
        RgLog::stdErr << kStrErrorFailedToGenerateVersionInfoHeader << std::endl;
    }

    if (!status)
    {
        RgLog::stdErr << kStrErrorFailedToGenerateVersionInfoFile << std::endl;
    }

    return status;
}

bool KcCliCommander::ListEntries(const Config & config, LoggingCallbackFunction callback)
{
    RgLog::stdErr << kStrErrorCommandNotSupported << std::endl;
    return false;
}

bool KcCliCommander::InitRequestedAsicList(const std::vector<std::string>& devices, 
                                           beKA::RgaMode                   mode,
                                           const std::set<std::string>&    supported_devices,
                                           std::set<std::string>&          matched_devices, 
                                           bool                            allow_unknown_devices)
{
    if (!devices.empty())
    {
        // Take the devices which the user selected.
        for (const std::string& device : devices)
        {
            std::string  matched_arch_name;
            bool found_single_target = KcUtils::FindGPUArchName(device, matched_arch_name, true, allow_unknown_devices);

            if (found_single_target)
            {
                // Check if the matched architecture is in the list of known ASICs.
                // The architecture returned by FindGPUArchName() is an extended name, for example: "gfx804 (Graphics IP v8)",
                // while the items in the list of known ASICs are "short" names: "gfx804".
                bool is_supported = false;
                matched_arch_name = KcUtils::ToLower(matched_arch_name);
                for (const std::string& asic : supported_devices)
                {
                    if (matched_arch_name.find(KcUtils::ToLower(asic)) != std::string::npos)
                    {
                        matched_devices.insert(asic);
                        is_supported = true;
                        break;
                    }
                }
                if (!is_supported && !allow_unknown_devices)
                {
                    RgLog::stdErr << kStrErrorError << GetInputlLanguageString(mode)
                        << kStrErrorTargetNotSupported << matched_arch_name << std::endl;
                }
            }
        }
    }
    else
    {
        // Take all known devices and do not print anything.
        for (const std::string& device : supported_devices)
        {
            std::string  arch_name = "";
            std::string  tmp_msg;
            bool found = KcUtils::FindGPUArchName(device, arch_name, false, allow_unknown_devices);
            if (found)
            {
                matched_devices.insert(device);
            }
        }
    }

    bool ret = (matched_devices.size() != 0);
    return ret;
}

bool KcCliCommander::LogCallback(const std::string& theString) const
{
    bool ret = false;
    if (log_callback_)
    {
        log_callback_(theString);
        ret = true;
    }
    return ret;
}

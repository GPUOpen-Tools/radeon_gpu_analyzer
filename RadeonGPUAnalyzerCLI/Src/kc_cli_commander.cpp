//=================================================================
// Copyright 2020 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

// C++
#include <cassert>

// Backend.
#include "RadeonGPUAnalyzerBackend/Src/be_utils.h"

// Local.
#include "RadeonGPUAnalyzerCLI/Src/kc_utils.h"
#include "RadeonGPUAnalyzerCLI/Src/kc_xml_writer.h"
#include "RadeonGPUAnalyzerCLI/Src/kc_cli_commander.h"
#include "RadeonGPUAnalyzerCLI/Src/kc_cli_commander_lightning.h"
#include "RadeonGPUAnalyzerCLI/Src/kc_cli_commander_vulkan.h"
#include "RadeonGPUAnalyzerCLI/Src/kc_cli_string_constants.h"

// Shared.
#include "Utils/Include/rgLog.h"

// Constants.
static const char* kStrErrorTargetNotSupported = " compilation for the detected target GPU is not supported: ";
static const char* kStrErrorFailedToGenerateVersionInfoFile = "Error: failed to generate the Version Info file.";
static const char* kStrErrorFailedToGenerateVersionInfoHeader = "Error: failed to generate version info header.";
static const char* kStrErrorFailedToGenerateVersionInfoRocmcl = "Error: failed to generate version info for ROCm.";
static const char* kStrErrorFailedToGenerateVersionInfoFileVulkanSystem = "Error: failed to generate system version info for Vulkan live-driver mode.";
static const char* kStrErrorFailedToGenerateVersionInfoFileVulkan = "Error: failed to generate version info for Vulkan live-driver mode.";

void KcCliCommander::Version(Config& config, LoggingCallbackFunction callback)
{
    KcUtils::PrintRgaVersion();
}

std::string  GetInputlLanguageString(beKA::RgaMode mode)
{
    switch (mode)
    {
    case beKA::RgaMode::kModeOpencl:
        return "OpenCL";
    case beKA::RgaMode::kModeOpengl:
        return "OpenGL";
    case beKA::RgaMode::kModeVulkan:
        return "Vulkan";
    case beKA::RgaMode::kModeVkOffline:
    case beKA::RgaMode::kModeVkOfflineSpv:
    case beKA::RgaMode::kModeVkOfflineSpvTxt:
        return "Vulkan Offline";
    case beKA::RgaMode::kModeDx11:
        return "DX11";
    case beKA::RgaMode::kModeDx12:
        return "DX12";
    case beKA::RgaMode::kModeDxr:
        return "DXR";
    default:
        const char* const kStrErrorUnknownMode = "Unknown mode.";
        assert(false && kStrErrorUnknownMode);
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
        status = KcUtils::DeleteFile(filename);
    }

    // Generate the Version Info header.
    rgLog::stdErr << kStrInfoGeneratingVersionInfoFile << filename << std::endl;
    status = status && KcXmlWriter::AddVersionInfoHeader(filename);
    assert(status);

    if (status)
    {
        // Try generating the version info file for ROCm OpenCL mode.
        bool is_rocm_version_info_generated =
            KcCLICommanderLightning::GenerateRocmVersionInfo(filename);
        assert(is_rocm_version_info_generated);
        if (!is_rocm_version_info_generated)
        {
            rgLog::stdErr << kStrErrorFailedToGenerateVersionInfoRocmcl << std::endl;
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
                rgLog::stdErr << kStrErrorFailedToGenerateVersionInfoFileVulkanSystem << std::endl;
            }
        }
        else
        {
            rgLog::stdErr << kStrErrorFailedToGenerateVersionInfoFileVulkan << std::endl;
        }

        // We are good if at least one of the modes managed to generate the version info.
        status = is_rocm_version_info_generated || is_vulkan_version_info_generated;
        assert(status);
    }
    else
    {
        rgLog::stdErr << kStrErrorFailedToGenerateVersionInfoHeader << std::endl;
    }

    if (!status)
    {
        rgLog::stdErr << kStrErrorFailedToGenerateVersionInfoFile << std::endl;
    }

    return status;
}

bool KcCliCommander::ListEntries(const Config & config, LoggingCallbackFunction callback)
{
    rgLog::stdErr << kStrErrorCommandNotSupported << std::endl;
    return false;
}

bool KcCliCommander::GetParsedIsaCsvText(const std::string& isaText, const std::string& device, bool add_line_numbers, std::string& csv_text)
{
    static const char* kStrCsvParsedIsaHeader = "Address, Opcode, Operands, Functional Unit, Cycles, Binary Encoding\n";
    static const char* kStrCsvParsedIsaHeaderLineNumbers = "Address, Source Line Number, Opcode, Operands, Functional Unit, Cycles, Binary Encoding\n";

    bool ret = false;
    std::string  parsed_isa;
    if (BeProgramBuilder::ParseIsaToCsv(isaText, device, parsed_isa, add_line_numbers, true) == beKA::kBeStatusSuccess)
    {
        csv_text = (add_line_numbers ? kStrCsvParsedIsaHeaderLineNumbers : kStrCsvParsedIsaHeader) + parsed_isa;
        ret = true;
    }
    return ret;
}

bool KcCliCommander::InitRequestedAsicList(const std::vector<std::string>& devices, RgaMode mode,
    const std::set<std::string>& supported_devices,
    std::set<std::string>& matched_devices, bool allow_unknown_devices)
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
                    rgLog::stdErr << kStrErrorError << GetInputlLanguageString(mode)
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

beKA::beStatus KcCliCommander::WriteIsaToFile(const std::string& fileName, const std::string& isaText)
{
    beStatus ret = beStatus::kBeStatusInvalid;
    ret = KcUtils::WriteTextFile(fileName, isaText, log_callback_) ?
        beKA::kBeStatusSuccess : beKA::kBeStatusWriteToFileFailed;
    if (ret != beKA::kBeStatusSuccess)
    {
        rgLog::stdErr << kStrErrorFailedToWriteIsaFile << fileName << std::endl;
    }
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

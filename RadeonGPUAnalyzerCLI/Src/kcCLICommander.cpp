//=================================================================
// Copyright 2017 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

// C++
#include <cassert>

// Backend.
#include <RadeonGPUAnalyzerBackend/Include/beUtils.h>

// Local.
#include <RadeonGPUAnalyzerCLI/Src/kcUtils.h>
#include <RadeonGPUAnalyzerCLI/Src/kcXmlWriter.h>
#include <RadeonGPUAnalyzerCLI/Src/kcCLICommander.h>
#include <RadeonGPUAnalyzerCLI/Src/kcCLICommanderLightning.h>
#include <RadeonGPUAnalyzerCLI/Src/kcCLICommanderVulkan.h>

// Shared.
#include <Utils/Include/rgLog.h>


void kcCLICommander::Version(Config& config, LoggingCallBackFunc_t callback)
{
    kcUtils::PrintRgaVersion();
}

std::string  GetInputlLanguageString(beKA::RgaMode mode)
{
    switch (mode)
    {
    case beKA::RgaMode::Mode_OpenCL:
        return "OpenCL";
    case beKA::RgaMode::Mode_OpenGL:
        return "OpenGL";
    case beKA::RgaMode::Mode_Vulkan:
        return "Vulkan";
    case beKA::RgaMode::Mode_Vk_Offline:
    case beKA::RgaMode::Mode_Vk_Offline_Spv:
    case beKA::RgaMode::Mode_Vk_Offline_SpvTxt:
        return "Vulkan Offline";
    case beKA::RgaMode::Mode_DX11:
        return "DX";
    default:
        assert(false && STR_ERR_UNKNOWN_MODE);
        return "";
    }
}

void kcCLICommander::ListAdapters(Config & config, LoggingCallBackFunc_t callback)
{
    GT_UNREFERENCED_PARAMETER(config);
    std::stringstream  msg;
    msg << STR_ERR_COMMAND_NOT_SUPPORTED << std::endl;
    callback(msg.str());
}

bool kcCLICommander::RunPostCompileSteps(const Config & config)
{
    return true;
}

bool kcCLICommander::GenerateVersionInfoFile(const Config& config)
{
    bool status = true;
    std::string fileName = config.m_versionInfoFile;

    // If the file requires wrapping with double-quotes, do it.
    if (fileName.find(' ') != std::string::npos &&
        fileName.find("\"") != std::string::npos)
    {
        std::stringstream wrappedFileName;
        wrappedFileName << "\"" << fileName << "\"";
        fileName = wrappedFileName.str();
    }

    // Delete the version info file if it already exists.
    if (!fileName.empty() && kcUtils::FileNotEmpty(fileName))
    {
        status = kcUtils::DeleteFile(fileName);
    }

    // Generate the Version Info header.
    rgLog::stdErr << STR_INFO_GENERATING_VERSION_INFO_FILE << fileName << std::endl;
    status = status && kcXmlWriter::AddVersionInfoHeader(fileName);
    assert(status);

    if (status)
    {
        // Try generating the version info file for ROCm OpenCL mode.
        bool isRocmVersionInfoGenerated =
            kcCLICommanderLightning::GenerateRocmVersionInfo(fileName);
        assert(isRocmVersionInfoGenerated);
        if (!isRocmVersionInfoGenerated)
        {
            rgLog::stdErr << STR_ERR_FAILED_GENERATE_VERSION_INFO_FILE_ROCM_CL << std::endl;
        }

        // Try generating the version info file for Vulkan live-driver mode.
        bool isVulkanVersionInfoGenerated =
            kcCLICommanderVulkan::GenerateVulkanVersionInfo(config,
                fileName, config.m_printProcessCmdLines);

        if (isVulkanVersionInfoGenerated)
        {
            // Try generating system version info for Vulkan live-driver mode.
            isVulkanVersionInfoGenerated = kcCLICommanderVulkan::GenerateSystemVersionInfo(config,
                fileName, config.m_printProcessCmdLines);
            assert(isVulkanVersionInfoGenerated);
            if (!isVulkanVersionInfoGenerated)
            {
                rgLog::stdErr << STR_ERR_FAILED_GENERATE_VERSION_INFO_FILE_VULKAN_SYSTEM << std::endl;
            }
        }
        else
        {
            rgLog::stdErr << STR_ERR_FAILED_GENERATE_VERSION_INFO_FILE_VULKAN << std::endl;
        }

        // We are good if at least one of the modes managed to generate the version info.
        status = isRocmVersionInfoGenerated || isVulkanVersionInfoGenerated;
        assert(status);
    }
    else
    {
        rgLog::stdErr << STR_ERR_FAILED_GENERATE_VERSION_INFO_HEADER << std::endl;
    }

    if (!status)
    {
        rgLog::stdErr << STR_ERR_FAILED_GENERATE_VERSION_INFO_FILE << std::endl;
    }

    return status;
}

bool kcCLICommander::ListEntries(const Config & config, LoggingCallBackFunc_t callback)
{
    rgLog::stdErr << STR_ERR_COMMAND_NOT_SUPPORTED << std::endl;

    return false;
}

bool kcCLICommander::GetParsedIsaCSVText(const std::string& isaText, const std::string& device, bool addLineNumbers, std::string& csvText)
{
    bool  ret = false;

    std::string  parsedIsa;
    if (beProgramBuilder::ParseISAToCSV(isaText, device, parsedIsa, addLineNumbers, true) == beKA::beStatus_SUCCESS)
    {
        csvText = (addLineNumbers ? STR_CSV_PARSED_ISA_HEADER_LINE_NUMS : STR_CSV_PARSED_ISA_HEADER) + parsedIsa;
        ret = true;
    }

    return ret;
}

bool kcCLICommander::InitRequestedAsicList(const std::vector<std::string>& devices, RgaMode mode,
    const std::set<std::string>& supportedDevices,
    std::set<std::string>& matchedDevices, bool allowUnknownDevices)
{
    if (!devices.empty())
    {
        // Take the devices which the user selected.
        for (const std::string& device : devices)
        {
            std::string  matchedArchName;
            bool foundSingleTarget = kcUtils::FindGPUArchName(device, matchedArchName, true, allowUnknownDevices);

            if (foundSingleTarget)
            {
                // Check if the matched architecture is in the list of known ASICs.
                // The architecture returned by FindGPUArchName() is an extended name, for example: "gfx804 (Graphics IP v8)",
                // while the items in the list of known ASICs are "short" names: "gfx804".
                bool  isSupported = false;
                matchedArchName = kcUtils::ToLower(matchedArchName);
                for (const std::string& asic : supportedDevices)
                {
                    if (matchedArchName.find(kcUtils::ToLower(asic)) != std::string::npos)
                    {
                        matchedDevices.insert(asic);
                        isSupported = true;
                        break;
                    }
                }
                if (!isSupported && !allowUnknownDevices)
                {
                    rgLog::stdErr << STR_ERR_ERROR << GetInputlLanguageString(mode)
                        << STR_ERR_TARGET_IS_NOT_SUPPORTED << matchedArchName << std::endl;
                }
            }
        }
    }
    else
    {
        // Take all known devices and do not print anything.
        for (const std::string& device : supportedDevices)
        {
            std::string  archName = "";
            std::string  tmpMsg;

            bool  found = kcUtils::FindGPUArchName(device, archName, false, allowUnknownDevices);
            if (found)
            {
                matchedDevices.insert(device);
            }
        }
    }

    bool ret = (matchedDevices.size() != 0);

    return ret;
}

beKA::beStatus kcCLICommander::WriteISAToFile(const std::string& fileName, const std::string& isaText)
{
    beStatus res = beStatus::beStatus_Invalid;

    res = kcUtils::WriteTextFile(fileName, isaText, m_LogCallback) ?
        beKA::beStatus_SUCCESS : beKA::beStatus_WriteToFile_FAILED;

    if (res != beKA::beStatus_SUCCESS)
    {
        rgLog::stdErr << STR_ERR_FAILED_ISA_FILE_WRITE << fileName << std::endl;
    }

    return res;
}
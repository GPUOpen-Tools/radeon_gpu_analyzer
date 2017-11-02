//=================================================================
// Copyright 2017 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

// Local.
#include <RadeonGPUAnalyzerCLI/src/kcUtils.h>
#include <RadeonGPUAnalyzerCLI/src/kcCLICommander.h>


void kcCLICommander::Version(Config& config, LoggingCallBackFunc_t callback)
{
    std::stringstream s_Log;
    std::cout << STR_RGA_PRODUCT_NAME << " " << STR_RGA_VERSION_PREFIX << STR_RGA_VERSION << "." << STR_RGA_BUILD_NUM << std::endl;
}

std::string  GetInpulLanguageString(beKA::SourceLanguage lang)
{
    switch (lang)
    {
        case beKA::SourceLanguage_OpenCL:
            return "OpenCL";
        case beKA::SourceLanguage_GLSL_OpenGL:
            return "OpenGL";
        case beKA::SourceLanguage_GLSL_Vulkan:
        case beKA::SourceLanguage_SPIRV_Vulkan:
            return "Vulkan";
        case beKA::SourceLanguage_HLSL:
            return "DX";
        default:
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

bool kcCLICommander::InitRequestedAsicList(const Config& config, const std::set<std::string>& supportedDevices, std::set<std::string>& targets)
{
    if (!config.m_ASICs.empty())
    {
        // Take the devices which the user selected.
        std::set<std::string>  matchedArchs;
        for (const std::string& asicName : config.m_ASICs)
        {
            std::string  matchedArchName;
            std::string  msg;
            bool foundSingleTarget = kcUtils::FindGPUArchName(asicName, matchedArchName, msg);
            LogCallBack(msg);

            if (foundSingleTarget)
            {
                // Check if the matched architecture is in the list of known ASICs.
                // The architecture returned by FindGPUArchName() is an extended name, for example: "gfx804 (Graphics IP v8)",
                // while the items in the list of known ASICs are "short" names: "gfx804".
                bool  isSupported = false;
                for (const std::string& asic : supportedDevices)
                {
                    if (matchedArchName.find(asic) != std::string::npos)
                    {
                        targets.insert(asic);
                        isSupported = true;
                        break;
                    }
                }
                if (!isSupported)
                {
                    std::stringstream  log;
                    log << STR_ERR_ERROR << GetInpulLanguageString(config.m_SourceLanguage)
                        << STR_ERR_TARGET_IS_NOT_SUPPORTED << matchedArchName << std::endl;
                    LogCallBack(log.str());
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

            bool  found = kcUtils::FindGPUArchName(device, archName, tmpMsg);
            if (found)
            {
                targets.insert(device);
            }
        }
    }

    bool ret = (targets.size() != 0);

    return ret;
}

//=================================================================
// Copyright 2017 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

// Backend.
#include <RadeonGPUAnalyzerBackend/include/beUtils.h>

// Local.
#include <RadeonGPUAnalyzerCLI/src/kcUtils.h>
#include <RadeonGPUAnalyzerCLI/src/kcCLICommander.h>
#include <RadeonGPUAnalyzerCLI/src/kcCLICommanderLightning.h>


void kcCLICommander::Version(Config& config, LoggingCallBackFunc_t callback)
{
    kcUtils::PrintRgaVersion();
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

bool kcCLICommander::GenerateSessionMetadata(const Config& config, const rgOutputMetadata& outMetadata) const
{
    rgFileEntryData  fileKernelData;

    bool  ret = !config.m_sessionMetadataFile.empty();

    if (ret)
    {
        for (const std::string&  inputFile : config.m_InputFiles)
        {
            rgEntryData  entryData;
            ret = ret && kcCLICommanderLightning::ExtractEntries(inputFile, config, entryData);
            if (ret)
            {
                fileKernelData[inputFile] = entryData;
            }
        }
    }

    ret = ret && kcUtils::GenerateCliMetadataFile(config.m_sessionMetadataFile, fileKernelData, outMetadata);

    return ret;
}

bool kcCLICommander::RunPostCompileSteps(const Config& config) const
{
    bool ret = false;
    if (!config.m_sessionMetadataFile.empty())
    {
        ret = GenerateSessionMetadata(config, m_outputMetadata);
        if (!ret)
        {
            std::stringstream  msg;
            msg << STR_ERR_FAILED_GENERATE_SESSION_METADATA << std::endl;
            m_LogCallback(msg.str());
        }
    }

    DeleteTempFiles();

    return ret;
}

void kcCLICommander::DeleteTempFiles() const
{
    for (const auto& outFileData : m_outputMetadata)
    {
        const rgOutputFiles  outFiles = outFileData.second;
        gtString  fileName;
        if (outFiles.m_isBinFileTemp && kcUtils::FileNotEmpty(outFiles.m_binFile))
        {
            fileName.fromASCIIString(outFiles.m_binFile.c_str());
            kcUtils::DeleteFile(fileName);
        }
        if (outFiles.m_isIsaFileTemp && kcUtils::FileNotEmpty(outFiles.m_isaFile))
        {
            fileName.fromASCIIString(outFiles.m_isaFile.c_str());
            kcUtils::DeleteFile(fileName);
        }
    }
}

bool kcCLICommander::InitRequestedAsicList(const Config& config, const std::set<std::string>& supportedDevices,
                                           std::set<std::string>& targets, std::function<void(const std::string&)> logCallback)
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
            logCallback(msg);

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
                    logCallback(log.str());
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

bool kcCLICommander::ListEntries(const Config& config, LoggingCallBackFunc_t callback)
{
    bool  ret = true;
    std::string  fileName, prepSrc;
    rgEntryData  entryData;
    std::stringstream  msg;

    if (config.m_SourceLanguage != SourceLanguage_OpenCL && config.m_SourceLanguage != SourceLanguage_Rocm_OpenCL)
    {
        msg << STR_ERR_COMMAND_NOT_SUPPORTED << std::endl;
        ret = false;
    }
    else
    {
        if (config.m_InputFiles.size() == 1)
        {
            fileName = config.m_InputFiles[0];
        }
        else
        {
            msg << STR_ERR_ONE_INPUT_FILE_EXPECTED << std::endl;
            ret = false;
        }
    }

    if (ret && (ret = kcCLICommanderLightning::ExtractEntries(fileName, config, entryData)) == true)
    {
        // Sort the entry names in alphabetical order.
        std::sort(entryData.begin(), entryData.end(),
                  [](const std::pair<std::string, int>& a, const std::pair<std::string, int>& b) {return (a.first < b.first); });

        // Dump the entry points.
        for (const std::pair<std::string, int>& dataItem : entryData)
        {
            msg << dataItem.first << ": " << dataItem.second << std::endl;
        }
        msg << std::endl;
    }

    callback(msg.str());

    return ret;
}

bool kcCLICommander::GetSupportedTargets(SourceLanguage lang, std::set<std::string>& targets)
{
    if (lang == SourceLanguage_Rocm_OpenCL)
    {
        targets = kcCLICommanderLightning::GetSupportedTargets();
    }
    else
    {
        std::vector<GDT_GfxCardInfo> dxDeviceTable;
        std::set<std::string> supportedDevices;
        std::set<std::string> matchedTargets;

        if (beUtils::GetAllGraphicsCards(dxDeviceTable, supportedDevices))
        {
            if (InitRequestedAsicList(Config(), supportedDevices, matchedTargets, [](const std::string& s) {}))
            {
                targets = matchedTargets;
            }
        }

    }
    return (!targets.empty());
}

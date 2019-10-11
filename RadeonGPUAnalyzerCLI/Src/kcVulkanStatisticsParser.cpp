//=================================================================
// Copyright 2017 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

// C++.
#include <iosfwd>
#include <streambuf>
#include <vector>

// Infra.
#ifdef _WIN32
    #pragma warning(push)
    #pragma warning(disable:4309)
#endif
#include <AMDTOSWrappers/Include/osFilePath.h>
#ifdef _WIN32
    #pragma warning(pop)
#endif

// Local.
#include <RadeonGPUAnalyzerCLI/Src/kcVulkanStatisticsParser.h>
#include <RadeonGPUAnalyzerCLI/Src/kcUtils.h>

// Constants.
static const char* ISA_SIZE_TOKEN = "codeLenInByte";
static const char* USED_VGPRS_TOKEN = "NumVgprs";
static const char* USED_SGPRS_TOKEN = "NumSgprs";
static const char* END_OF_LINE_DELIMITER = ";";
static const char* VK_OFFLINE_DEVICE_NAME_TAHITI = "tahiti";
static const char* VK_OFFLINE_DEVICE_NAME_TONGA = "tonga";
static const char* VK_OFFLINE_DEVICE_NAME_CAPEVERDE = "capeverde";
static const char* VK_OFFLINE_DEVICE_NAME_HAINAN = "hainan";
static const char* VK_OFFLINE_DEVICE_NAME_OLAND = "oland";
static const char* VK_OFFLINE_DEVICE_NAME_PITCAIRN = "pitcairn";
static const char* VK_OFFLINE_DEVICE_NAME_ICELAND = "iceland";

kcVulkanStatisticsParser::kcVulkanStatisticsParser()
{
}

kcVulkanStatisticsParser::~kcVulkanStatisticsParser()
{
}

/// Extracts a numeric value from the SC's textual statistics for Vulkan.
/// Params:
///     fileContent: the content of the SC statistics file.
///     attributeToken: the attribute whose value is to be extracted.
//      numericValue: the extracted value.
// Returns: true for success, false otherwise.
static bool ExtractNumericStatistic(const std::string& fileContent, const char* attributeToken, size_t& extractedValue)
{
    bool ret = false;
    size_t valueBeginIndex = fileContent.find(attributeToken);

    if (valueBeginIndex != std::string::npos)
    {
        valueBeginIndex += strlen(attributeToken) + 1;

        if (valueBeginIndex < fileContent.size())
        {
            size_t valueEndIndex = fileContent.find(END_OF_LINE_DELIMITER, valueBeginIndex) - 1;

            if (valueEndIndex != std::string::npos)
            {
                size_t valueLength = valueEndIndex - valueBeginIndex + 1;

                if (valueLength > 0)
                {
                    // Extract the value.
                    std::string value = fileContent.substr(valueBeginIndex, valueLength);
                    std::string::iterator end_pos = std::remove_if(value.begin(),
                        value.end(), [&value](char c) { return (c == ' ' || !std::isdigit(c)); });
                    value.erase(end_pos, value.end());
                    try
                    {
                        extractedValue = std::stoi(value);
                        ret = true;
                    }
                    catch (...){}
                }
            }
        }
    }

    return ret;
}

// Extracts the ISA size in bytes.
static bool ExtractIsaSize(const std::string& fileContent, size_t& isaSizeInBytes)
{
    return ExtractNumericStatistic(fileContent, ISA_SIZE_TOKEN, isaSizeInBytes);
}

// Extracts the number of used SGPRs.
static bool ExtractUsedSgprs(const std::string& fileContent, size_t& isaSizeInBytes)
{
    return ExtractNumericStatistic(fileContent, USED_SGPRS_TOKEN, isaSizeInBytes);
}

// Extracts the number of used VGPRs.
static bool ExtractUsedVgprs(const std::string& fileContent, size_t& isaSizeInBytes)
{
    return ExtractNumericStatistic(fileContent, USED_VGPRS_TOKEN, isaSizeInBytes);
}

static bool IsGfx6Device(const std::string& device)
{
    std::string deviceLower = device;
    std::transform(deviceLower.begin(), deviceLower.end(), deviceLower.begin(), ::tolower);
    bool ret = deviceLower.find(VK_OFFLINE_DEVICE_NAME_TAHITI) != std::string::npos ||
        deviceLower.find(VK_OFFLINE_DEVICE_NAME_CAPEVERDE) != std::string::npos ||
        deviceLower.find(VK_OFFLINE_DEVICE_NAME_HAINAN) != std::string::npos ||
        deviceLower.find(VK_OFFLINE_DEVICE_NAME_OLAND) != std::string::npos ||
        deviceLower.find(VK_OFFLINE_DEVICE_NAME_PITCAIRN) != std::string::npos;
    return ret;
}

bool kcVulkanStatisticsParser::ParseStatistics(const std::string& device, const gtString& statisticsFile, beKA::AnalysisData& statistics)
{
    bool ret = false;
    statistics.ISASize = 0;
    statistics.numSGPRsUsed = 0;
    statistics.numVGPRsUsed = 0;
    statistics.numVGPRsAvailable = 256;
    statistics.numSGPRsAvailable = 104;
    statistics.LDSSizeAvailable = 65536;

    // Special cases.
    std::string deviceLower = device;
    std::transform(deviceLower.begin(), deviceLower.end(), deviceLower.begin(), ::tolower);
    if (deviceLower.find(VK_OFFLINE_DEVICE_NAME_TONGA) != std::string::npos ||
        deviceLower.find(VK_OFFLINE_DEVICE_NAME_ICELAND) != std::string::npos)
    {
        statistics.numSGPRsAvailable = 96;
    }
    if (IsGfx6Device(device))
    {
        statistics.LDSSizeAvailable = 32768;
    }

    // Check if the file exists.
    if (!statisticsFile.isEmpty())
    {
        osFilePath filePath(statisticsFile);

        if (filePath.exists())
        {
            std::ifstream file(statisticsFile.asASCIICharArray());
            std::string fileContent((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

            if (!fileContent.empty())
            {
                // Extract the ISA size in bytes.
                size_t isaSizeInBytes = 0;
                bool isIsaSizeExtracted = ExtractIsaSize(fileContent, isaSizeInBytes);

                if (isIsaSizeExtracted)
                {
                    statistics.ISASize = isaSizeInBytes;
                }

                // Extract the number of used SGPRs.
                size_t usedSgprs = 0;
                bool isSgprsExtracted = ExtractUsedSgprs(fileContent, usedSgprs);

                if (isSgprsExtracted)
                {
                    statistics.numSGPRsUsed = usedSgprs;
                }

                // Extract the number of used VGPRs.
                size_t usedVgprs = 0;
                bool isVgprsExtracted = ExtractUsedVgprs(fileContent, usedVgprs);

                if (isVgprsExtracted)
                {
                    statistics.numVGPRsUsed = usedVgprs;
                }

                // We succeeded if all data was extracted successfully.
                ret = (isIsaSizeExtracted && isSgprsExtracted && isVgprsExtracted);
            }
        }
    }

    return ret;
}


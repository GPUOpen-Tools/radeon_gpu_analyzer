//=================================================================
// Copyright 2017 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

// C++.
#include <algorithm>
#include <sstream>
#include <fstream>
#include <iostream>
#include <cctype>

// Infra.
#ifdef _WIN32
    #pragma warning(push)
    #pragma warning(disable:4309)
#endif
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osFile.h>
#ifdef _WIN32
    #pragma warning(pop)
#endif

// Local.
#include <RadeonGPUAnalyzerBackend/Include/beUtils.h>
#include <RadeonGPUAnalyzerBackend/Include/beStringConstants.h>
#include <DeviceInfoUtils.h>
#include <DeviceInfo.h>

// *** INTERNALLY-LINKED AUXILIARY FUNCTIONS - BEGIN ***

// Converts string to its lower-case version.
std::string ToLower(const std::string& str)
{
    std::string lstr = str;
    std::transform(lstr.begin(), lstr.end(), lstr.begin(), [](const char& c) {return std::tolower(c); });
    return lstr;
}

// Retrieves the list of devices according to the given HW generation.
static void AddGenerationDevices(GDT_HW_GENERATION hwGen, std::vector<GDT_GfxCardInfo>& cardList,
                                 std::set<std::string> &uniqueNamesOfPublishedDevices, bool convertToLower = false)
{
    std::vector<GDT_GfxCardInfo> cardListBuffer;
    if (AMDTDeviceInfoUtils::Instance()->GetAllCardsInHardwareGeneration(hwGen, cardListBuffer))
    {
        cardList.insert(cardList.end(), cardListBuffer.begin(), cardListBuffer.end());

        for (const GDT_GfxCardInfo& cardInfo : cardList)
        {
            uniqueNamesOfPublishedDevices.insert(convertToLower ? ToLower(cardInfo.m_szCALName) : cardInfo.m_szCALName);
        }
    }
}

// *** INTERNALLY-LINKED AUXILIARY FUNCTIONS - END ***

bool beUtils::GdtHwGenToNumericValue(GDT_HW_GENERATION hwGen, size_t& gfxIp)
{
    const size_t BE_GFX_IP_6 = 6;
    const size_t BE_GFX_IP_7 = 7;
    const size_t BE_GFX_IP_8 = 8;
    const size_t BE_GFX_IP_9 = 9;

    bool ret = true;

    switch (hwGen)
    {
        case GDT_HW_GENERATION_SOUTHERNISLAND:
            gfxIp = BE_GFX_IP_6;
            break;

        case GDT_HW_GENERATION_SEAISLAND:
            gfxIp = BE_GFX_IP_7;
            break;

        case GDT_HW_GENERATION_VOLCANICISLAND:
            gfxIp = BE_GFX_IP_8;
            break;

        case GDT_HW_GENERATION_GFX9:
            gfxIp = BE_GFX_IP_9;
            break;

        case GDT_HW_GENERATION_NONE:
        case GDT_HW_GENERATION_NVIDIA:
        case GDT_HW_GENERATION_LAST:
        default:
            // We should not get here.
            GT_ASSERT_EX(false, L"Unsupported HW Generation.");
            ret = false;
            break;
    }

    return ret;
}

bool beUtils::GdtHwGenToString(GDT_HW_GENERATION hwGen, std::string& hwGenAsStr)
{
    const char* BE_GFX_IP_6 = "SI";
    const char* BE_GFX_IP_7 = "CI";
    const char* BE_GFX_IP_8 = "VI";

    bool ret = true;

    switch (hwGen)
    {
        case GDT_HW_GENERATION_SOUTHERNISLAND:
            hwGenAsStr = BE_GFX_IP_6;
            break;

        case GDT_HW_GENERATION_SEAISLAND:
            hwGenAsStr = BE_GFX_IP_7;
            break;

        case GDT_HW_GENERATION_VOLCANICISLAND:
            hwGenAsStr = BE_GFX_IP_8;
            break;

        case GDT_HW_GENERATION_NONE:
        case GDT_HW_GENERATION_NVIDIA:
        case GDT_HW_GENERATION_LAST:
        default:
            // We should not get here.
            GT_ASSERT_EX(false, L"Unsupported HW Generation.");
            ret = false;
            break;
    }

    return ret;
}

bool beUtils::GfxCardInfoSortPredicate(const GDT_GfxCardInfo& a, const GDT_GfxCardInfo& b)
{
    // Generation is the primary key.
    if (a.m_generation < b.m_generation) { return true; }

    if (a.m_generation > b.m_generation) { return false; }

    // CAL name is next.
    int ret = ::strcmp(a.m_szCALName, b.m_szCALName);

    if (ret < 0) { return true; }

    if (ret > 0) { return false; }

    // Marketing name next.
    ret = ::strcmp(a.m_szMarketingName, b.m_szMarketingName);

    if (ret < 0) { return true; }

    if (ret > 0) { return false; }

    // DeviceID last.
    return a.m_deviceID < b.m_deviceID;
}

bool beUtils::GetAllGraphicsCards(std::vector<GDT_GfxCardInfo>& cardList,
                                  std::set<std::string>& uniqueNamesOfPublishedDevices,
                                  bool convertToLower)
{
     // Retrieve the list of devices for every relevant hardware generations.
    AddGenerationDevices(GDT_HW_GENERATION_SOUTHERNISLAND, cardList, uniqueNamesOfPublishedDevices, convertToLower);
    AddGenerationDevices(GDT_HW_GENERATION_SEAISLAND, cardList, uniqueNamesOfPublishedDevices, convertToLower);
    AddGenerationDevices(GDT_HW_GENERATION_VOLCANICISLAND, cardList, uniqueNamesOfPublishedDevices, convertToLower);
    AddGenerationDevices(GDT_HW_GENERATION_GFX9, cardList, uniqueNamesOfPublishedDevices, convertToLower);

    // Sort the data.
    std::sort(cardList.begin(), cardList.end(), beUtils::GfxCardInfoSortPredicate);

    return (!cardList.empty() && !uniqueNamesOfPublishedDevices.empty());
}

bool beUtils::GetMarketingNameToCodenameMapping(std::map<std::string, std::set<std::string>>& cardsMap)
{
    std::vector<GDT_GfxCardInfo> cardList;
    std::set<std::string> uniqueNames;

    // Retrieve the list of all supported cards.
    bool ret = GetAllGraphicsCards(cardList, uniqueNames);

    if (ret)
    {
        for (const GDT_GfxCardInfo& card : cardList)
        {
            if (card.m_szMarketingName != nullptr &&
                card.m_szCALName != nullptr &&
                (strlen(card.m_szMarketingName) > 1) &&
                (strlen(card.m_szCALName) > 1))
            {
                // Create the key string.
                std::string displayName;
                ret = AMDTDeviceInfoUtils::Instance()->GetHardwareGenerationDisplayName(card.m_generation, displayName);
                if (ret)
                {
                    std::stringstream nameBuilder;
                    nameBuilder << card.m_szCALName << " (" << displayName << ")";

                    // Add this item to the relevant container in the map.
                    cardsMap[nameBuilder.str()].insert(card.m_szMarketingName);
                }
            }
        }
    }

    return ret;
}

void beUtils::DeleteOutputFiles(const beProgramPipeline& outputFilePaths)
{
    DeleteFile(outputFilePaths.m_vertexShader);
    DeleteFile(outputFilePaths.m_tessControlShader);
    DeleteFile(outputFilePaths.m_tessEvaluationShader);
    DeleteFile(outputFilePaths.m_geometryShader);
    DeleteFile(outputFilePaths.m_fragmentShader);
    DeleteFile(outputFilePaths.m_computeShader);
}

void beUtils::DeleteFile(const gtString& filePath)
{
    osFilePath osPath(filePath);

    if (osPath.exists())
    {
        osFile osFile(osPath);
        osFile.deleteFile();
    }
}

bool  beUtils::IsFilePresent(const std::string& fileName)
{
    bool  ret = true;
    if (!fileName.empty())
    {
        std::ifstream file(fileName);
        ret = (file.good() && file.peek() != std::ifstream::traits_type::eof());
    }
    return ret;
}

std::string beUtils::GetFileExtension(const std::string& fileName)
{
    size_t offset = fileName.rfind('.');
    const std::string& ext = (offset != std::string::npos && ++offset < fileName.size()) ? fileName.substr(offset) : "";
    return ext;
}

void beUtils::PrintCmdLine(const std::string & cmdLine, bool doPrint)
{
    if (doPrint)
    {
        std::cout << std::endl << BE_STR_LAUNCH_EXTERNAL_PROCESS << cmdLine << std::endl << std::endl;
    }
}

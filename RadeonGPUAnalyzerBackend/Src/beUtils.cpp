//=================================================================
// Copyright 2017 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

// C++.
#include <algorithm>
#include <sstream>
#include <fstream>
#include <iostream>
#include <cctype>
#include <cwctype>

// Infra.
#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4309)
#endif
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osFile.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#ifdef _WIN32
#pragma warning(pop)
#endif

// Local.
#include <RadeonGPUAnalyzerBackend/Include/beUtils.h>
#include <RadeonGPUAnalyzerBackend/Include/beStringConstants.h>
#include <DeviceInfoUtils.h>
#include <DeviceInfo.h>

// CLI.
#include <RadeonGPUAnalyzerCLI/Src/kcUtils.h>

// Constants.
static const char* STR_ERROR_CODE_OBJECT_PARSE_FAILURE = "Error: failed to parse Code Object .text section.";
static const char* STR_ERROR_LC_DISASSEMBLER_LAUNCH_FAILURE = "Error: failed to launch the LC disassembler.";

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

        for (const GDT_GfxCardInfo& cardInfo : cardListBuffer)
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
    const size_t BE_GFX_IP_10 = 10;

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

    case GDT_HW_GENERATION_GFX10:
        gfxIp = BE_GFX_IP_10;
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

struct lex_compare {
    bool operator() (const int64_t& lhs, const int64_t& rhs) const
    {

    }
};

bool beUtils::GetAllGraphicsCards(std::vector<GDT_GfxCardInfo>& cardList,
    std::set<std::string>& uniqueNamesOfPublishedDevices,
    bool convertToLower /*= false*/)
{
    // Retrieve the list of devices for every relevant hardware generations.
    AddGenerationDevices(GDT_HW_GENERATION_SOUTHERNISLAND, cardList, uniqueNamesOfPublishedDevices, convertToLower);
    AddGenerationDevices(GDT_HW_GENERATION_SEAISLAND, cardList, uniqueNamesOfPublishedDevices, convertToLower);
    AddGenerationDevices(GDT_HW_GENERATION_VOLCANICISLAND, cardList, uniqueNamesOfPublishedDevices, convertToLower);
    AddGenerationDevices(GDT_HW_GENERATION_GFX9, cardList, uniqueNamesOfPublishedDevices, convertToLower);
    AddGenerationDevices(GDT_HW_GENERATION_GFX10, cardList, uniqueNamesOfPublishedDevices, convertToLower);

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
    DeleteFileFromDisk(outputFilePaths.m_vertexShader);
    DeleteFileFromDisk(outputFilePaths.m_tessControlShader);
    DeleteFileFromDisk(outputFilePaths.m_tessEvaluationShader);
    DeleteFileFromDisk(outputFilePaths.m_geometryShader);
    DeleteFileFromDisk(outputFilePaths.m_fragmentShader);
    DeleteFileFromDisk(outputFilePaths.m_computeShader);
}

void beUtils::DeleteFileFromDisk(const gtString& filePath)
{
    osFilePath osPath(filePath);

    if (osPath.exists())
    {
        osFile osFile(osPath);
        osFile.deleteFile();
    }
}

void beUtils::DeleteFileFromDisk(const std::string& filePath)
{
    gtString gPath;
    gPath << filePath.c_str();
    return DeleteFileFromDisk(gPath);
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

bool beUtils::ReadBinaryFile(const std::string& fileName, std::vector<char>& content)
{
    bool ret = false;
    std::ifstream input;
    input.open(fileName.c_str(), std::ios::binary);

    if (input.is_open())
    {
        content = std::vector<char>(std::istreambuf_iterator<char>(input), {});
        ret = !content.empty();
    }
    return ret;
}

bool beUtils::IsFilesIdentical(const std::string& fileName1, const std::string& fileName2)
{
    std::vector<char> content1;
    std::vector<char> content2;
    bool isFileRead1 = beUtils::ReadBinaryFile(fileName1, content1);
    bool isFileRead2 = beUtils::ReadBinaryFile(fileName2, content2);
    return (isFileRead1 && isFileRead2 && std::equal(content1.begin(), content1.end(), content2.begin()));
}

void beUtils::PrintCmdLine(const std::string & cmdLine, bool doPrint)
{
    if (doPrint)
    {
        std::cout << std::endl << BE_STR_LAUNCH_EXTERNAL_PROCESS << cmdLine << std::endl << std::endl;
    }
}

void beUtils::splitString(const std::string& str, char delim, std::vector<std::string>& dst)
{
    std::stringstream ss;
    ss.str(str);
    std::string substr;
    while (std::getline(ss, substr, delim))
    {
        dst.push_back(substr);
    }
}

bool beUtils::DeviceNameLessThan(const std::string& a, const std::string& b)
{
    const char* GFX_NOTATION_TOKEN = "gfx";
    bool ret = true;
    size_t szA = a.find(GFX_NOTATION_TOKEN);
    size_t szB = b.find(GFX_NOTATION_TOKEN);
    if (szA == std::string::npos && szB == std::string::npos)
    {
        // Neither name is in gfx-notation, compare using standard string logic.
        ret = a.compare(b) < 0;
    }
    else if (!(szA != std::string::npos && szB != std::string::npos))
    {
        // Only one name has the gfx notation, assume that it is a newer generation.
        ret = (szB != std::string::npos);
    }
    else
    {
        // Both names are in gfx notation, compare according to the number.
        std::vector<std::string> splitA;
        std::vector<std::string> splitB;
        beUtils::splitString(a, 'x', splitA);
        beUtils::splitString(b, 'x', splitB);
        assert(splitA.size() > 1);
        assert(splitB.size() > 1);
        if (splitA.size() > 1 && splitB.size() > 1)
        {
            try
            {
                int numA = std::stoi(splitA[1], nullptr);
                int numB = std::stoi(splitB[1], nullptr);
                ret = ((numB - numA) > 0);
            }
            catch (...)
            {
                ret = false;
            }
        }
    }
    return ret;
}

bool beUtils::DisassembleCodeObject(const std::string& coFileName, bool shouldPrintCmd,
    std::string& disassemblyWhole, std::string& disassemblyText, std::string& errorMsg)
{
    // Build the command.
    std::stringstream cmd;
    cmd << coFileName;

    osFilePath  lcDisassemblerExe;
    long        exitCode = 0;

    osGetCurrentApplicationPath(lcDisassemblerExe, false);
    lcDisassemblerExe.appendSubDirectory(LC_DISASSEMBLER_DIR);

    lcDisassemblerExe.setFileName(LC_DISASSEMBLER_EXE);

    // Clear the error message buffer.
    errorMsg.clear();
    std::string outText;

    kcUtils::ProcessStatus status = kcUtils::LaunchProcess(lcDisassemblerExe.asString().asASCIICharArray(),
        cmd.str(),
        "",
        PROCESS_WAIT_INFINITE,
        shouldPrintCmd,
        outText,
        errorMsg,
        exitCode);

    // Extract the .text disassembly.
    assert(!outText.empty());
    disassemblyWhole = outText;

    if (!disassemblyWhole.empty())
    {
        // Find where the .text section starts.
        size_t textOffsetStart = disassemblyWhole.find(".text");
        assert(textOffsetStart != std::string::npos);
        assert(textOffsetStart != std::string::npos &&
            textOffsetStart < disassemblyWhole.size() + 5);
        if (textOffsetStart < disassemblyWhole.size() + 5)
        {
            // Skip .text identifier.
            textOffsetStart += 5;

            // Find where the relevant portion of the disassembly ends.
            size_t textOffsetEnd = disassemblyWhole.find("s_code_end");
            assert(textOffsetEnd != std::string::npos);
            if (textOffsetEnd != std::string::npos)
            {
                // Extract the relevant section.
                size_t numCharacters = textOffsetEnd - textOffsetStart;
                assert(numCharacters > 0);
                assert(numCharacters < disassemblyWhole.size() - textOffsetStart);
                if (numCharacters > 0 && numCharacters < disassemblyWhole.size() - textOffsetStart)
                {
                    disassemblyText = disassemblyWhole.substr(textOffsetStart, numCharacters);
                }
                else if (errorMsg.empty())
                {
                    errorMsg = STR_ERROR_CODE_OBJECT_PARSE_FAILURE;
                }
            }
            else if (errorMsg.empty())
            {
                errorMsg = STR_ERROR_CODE_OBJECT_PARSE_FAILURE;
            }
        }
    }

    if (disassemblyText.empty())
    {
        if (status == kcUtils::ProcessStatus::Success && errorMsg.empty())
        {
            errorMsg = STR_ERROR_LC_DISASSEMBLER_LAUNCH_FAILURE;
        }
    }

    return (status == kcUtils::ProcessStatus::Success ? beStatus_SUCCESS : beStatus_dx12BackendLaunchFailure);
}


static bool ExtractAttributeValue(const std::string &disassemblyWhole, size_t kdPos, const std::string& attributeName, uint32_t& value)
{
    bool ret = false;
    bool shouldAbort = false;
    bool isBefore = false;
    try
    {
        // Offset where our attribute is within the string.
        size_t startPosTemp = 0;

        // The reference symbol.
        const std::string KD_SYMBOL_TOKEN = ".symbol:";
        if (attributeName < KD_SYMBOL_TOKEN)
        {
            // Look before the reference symbol.
           startPosTemp = disassemblyWhole.rfind(attributeName, kdPos);
           isBefore = true;
        }
        else if (attributeName > KD_SYMBOL_TOKEN)
        {
            // Look after the reference symbol.
            startPosTemp = disassemblyWhole.find(attributeName, kdPos);
        }
        else
        {
            // We shouldn't get here.
            assert(false);
            shouldAbort = true;
        }

        if (!shouldAbort)
        {
            startPosTemp += attributeName.size();
            assert((isBefore && startPosTemp < kdPos) || (!isBefore && startPosTemp > kdPos));
            if ((isBefore && startPosTemp < kdPos) || (!isBefore && startPosTemp > kdPos))
            {
                while (std::iswspace(disassemblyWhole[++startPosTemp]));
                assert((isBefore && startPosTemp < kdPos) || (!isBefore && startPosTemp > kdPos));
                if ((isBefore && startPosTemp < kdPos) || (!isBefore && startPosTemp > kdPos))
                {
                    size_t endPos = startPosTemp;
                    while (!std::iswspace(disassemblyWhole[++endPos]));
                    assert(startPosTemp < endPos);
                    if (startPosTemp < endPos)
                    {
                        // Extract the string representing the value and convert to non-negative decimal number.
                        std::string valueAsStr = disassemblyWhole.substr(startPosTemp, endPos - startPosTemp);
                        std::stringstream conversionStream;
                        conversionStream << std::hex << valueAsStr;
                        conversionStream >> value;
                        ret = true;
                    }
                }
            }
        }
    }
    catch (...)
    {
        // Failure occurred.
        ret = false;
    }
    return ret;
}

bool beUtils::ExtractCodeObjectStatistics(const std::string& disassemblyWhole,
    std::map<std::string, beKA::AnalysisData>& dataMap)
{
    bool ret = false;
    dataMap.clear();
    const char* KERNEL_SYMBOL_TOKEN = ".kd";
    size_t startPos = disassemblyWhole.find(KERNEL_SYMBOL_TOKEN);
    while (startPos != std::string::npos)
    {
        // Extract the kernel name.
        std::string kernelName;
        size_t startPosTemp = startPos;
        std::stringstream kernelNameStream;
        while (--startPosTemp > 0 && !std::iswspace(disassemblyWhole[startPosTemp]));
        assert(startPosTemp + 1 < startPos - 1);
        if (startPosTemp + 1 < startPos - 1)
        {
            kernelName = disassemblyWhole.substr(startPosTemp + 1, startPos - startPosTemp - 1);
            auto iter = dataMap.find(kernelName);
            assert(iter == dataMap.end());
            if (iter == dataMap.end())
            {
                // LDS.
                const std::string LDS_USAGE_TOKEN = ".group_segment_fixed_size:";
                uint32_t ldsUsage = 0;
                bool isOk = ExtractAttributeValue(disassemblyWhole, startPos, LDS_USAGE_TOKEN, ldsUsage);
                assert(isOk);

                // SGPR count.
                const std::string SGPR_COUNT_TOKEN = ".sgpr_count:";
                uint32_t sgprCount = 0;
                isOk = ExtractAttributeValue(disassemblyWhole, startPos, SGPR_COUNT_TOKEN, sgprCount);
                assert(isOk);

                // SGPR spill count.
                const std::string SGPR_SPILL_COUNT_TOKEN = ".sgpr_spill_count:";
                uint32_t sgprSpillCount = 0;
                isOk = ExtractAttributeValue(disassemblyWhole, startPos, SGPR_SPILL_COUNT_TOKEN, sgprSpillCount);
                assert(isOk);

                // VGPR count.
                const std::string VGPR_COUNT_TOKEN = ".vgpr_count:";
                uint32_t vgprCount = 0;
                isOk = ExtractAttributeValue(disassemblyWhole, startPos, VGPR_COUNT_TOKEN, vgprCount);
                assert(isOk);

                // VGPR spill count.
                const std::string VGPR_SPILL_COUNT_TOKEN = ".vgpr_spill_count";
                uint32_t vgprSpillCount = 0;
                isOk = ExtractAttributeValue(disassemblyWhole, startPos, VGPR_SPILL_COUNT_TOKEN, vgprSpillCount);
                assert(isOk);

                // Wavefront size.
                const std::string WAVEFRONT_SIZE_TOKEN = ".wavefront_size:";
                uint32_t wavefrontSize = 0;
                isOk = ExtractAttributeValue(disassemblyWhole, startPos, WAVEFRONT_SIZE_TOKEN, wavefrontSize);
                assert(isOk);

                // Add values which were extracted from the Code Object meta data.
                AnalysisData data;
                data.LDSSizeUsed = ldsUsage;
                data.numSGPRsUsed = sgprCount;
                data.numSGPRSpills = sgprSpillCount;
                data.numVGPRsUsed = vgprCount;
                data.numVGPRSpills = vgprSpillCount;
                data.wavefrontSize = wavefrontSize;

                // Add fixed values.
                data.LDSSizeAvailable = 65536;
                data.numVGPRsAvailable = 256;
                data.numSGPRsAvailable = 104;

                // Add the kernel's stats to the map.
                dataMap[kernelName] = data;

                // Move to the next kernel.
                startPos = disassemblyWhole.find(KERNEL_SYMBOL_TOKEN, startPos + 1);
            }
        }
    }

    ret = !dataMap.empty();
    return ret;
}

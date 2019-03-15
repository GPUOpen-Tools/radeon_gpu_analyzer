//=================================================================
// Copyright 2017 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

#ifndef beUtils_h__
#define beUtils_h__

// Device info.
#include <vector>
#include <set>
#include <string>
#include <map>
#include <DeviceInfo.h>
#include <RadeonGPUAnalyzerBackend/Include/beDataTypes.h>
#ifdef _WIN32
    #pragma warning(push)
    #pragma warning(disable:4309)
#endif
#include <AMDTBaseTools/Include/gtString.h>
#ifdef _WIN32
    #pragma warning(pop)
#endif

class beUtils
{
public:

    // Convert HW generation to its corresponding numerical value.
    static bool GdtHwGenToNumericValue(GDT_HW_GENERATION hwGen, size_t& gfxIp);

    // Convert HW generation to its corresponding string representation.
    static bool GdtHwGenToString(GDT_HW_GENERATION hwGen, std::string& hwGenAsStr);

    // Predicate to be used for sorting HW devices.
    static bool GfxCardInfoSortPredicate(const GDT_GfxCardInfo& a, const GDT_GfxCardInfo& b);

    // Gets all of the supported graphics cards.
    // If "convertToLower" is true, the device names returned in "uniqueNamesOfPublishedDevices" will be
    // converted to lower case.
    static bool GetAllGraphicsCards(std::vector<GDT_GfxCardInfo>& cardList,
                                    std::set<std::string>& uniqueNamesOfPublishedDevices,
                                    bool convertToLower = false);

    // Gets a mapping of the marketing names to the internal code names.
    static bool GetMarketingNameToCodenameMapping(std::map<std::string, std::set<std::string>>& cardsMap);

    // Deletes the physical files from the file system.
    static void DeleteOutputFiles(const beProgramPipeline& outputFilePaths);

    // Deletes a physical file from the file system.
    static void DeleteFile(const gtString& filePath);

    // Checks if file exists and is not empty.
    static bool IsFilePresent(const std::string& fileName);

    // Retrieves the file extension.
    static std::string GetFileExtension(const std::string& fileName);

    // Print "cmdLine" to stdout if "doPrint" is true.
    static void PrintCmdLine(const std::string& cmdLine, bool doPrint);

private:
    // No instances for this class, as this is a static utility class.
    beUtils() = default;
    beUtils(const beUtils& other) = default;
    ~beUtils() = default;
};

#endif // beUtils_h__

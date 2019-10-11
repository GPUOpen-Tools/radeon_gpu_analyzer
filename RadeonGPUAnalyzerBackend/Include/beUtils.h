//=================================================================
// Copyright 2017 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

#ifndef beUtils_h__
#define beUtils_h__

// C++.
#include <vector>
#include <set>
#include <string>
#include <map>

// Device info.
#include <DeviceInfo.h>

// Local.
#include <RadeonGPUAnalyzerBackend/Include/beDataTypes.h>
#include <RadeonGPUAnalyzerBackend/Include/beInclude.h>

// Infra.
#ifdef _WIN32
    #pragma warning(push)
    #pragma warning(disable:4309)
#endif
#include <AMDTBaseTools/Include/gtString.h>
#ifdef _WIN32
    #pragma warning(pop)
#endif

// A container that keeps the mapping between some "non-standard" PAL NULL device names
// returned by driver and the "standard" names that can be found in the Device Info lib.
static const std::vector<std::pair<std::string, std::string>>
PAL_DEVICE_NAME_MAPPING =
{
    {"bristol",        "bristol ridge"},
    {"polaris10",      "ellesmere"},
    {"polaris11",      "baffin"}
};

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
    static void DeleteFileFromDisk(const gtString& filePath);

    // Deletes a physical file from the file system.
    static void DeleteFileFromDisk(const std::string& filePath);

    // Checks if file exists and is not empty.
    static bool IsFilePresent(const std::string& fileName);

    // Returns true if the contents of the two given files is identical, and false otherwise.
    static bool IsFilesIdentical(const std::string& fileName1, const std::string& fileName2);

    // Retrieves the file extension.
    static std::string GetFileExtension(const std::string& fileName);

    // Reads a binary file into the vector.
    static bool ReadBinaryFile(const std::string& fileName, std::vector<char>& content);

    // Print "cmdLine" to stdout if "doPrint" is true.
    static void PrintCmdLine(const std::string& cmdLine, bool doPrint);

    // Split the given string according to the given delimiter,
    // and store the results in the destination vector.
    static void splitString(const std::string& str, char delim, std::vector<std::string>& dst);

    // Returns true if the given string represents a numeric value, and false otherwise.
    static bool IsNumericValue(const std::string& str);

    // Returns true if device name a is "less than" b. This is used when sorting containers.
    // A gfx-notation device name, which indicates a Vega+ target, will never be "less than"
    // a non-gfx notation name. Two non-gfx notation names would be compared using the usual
    // string comparison logic, and two gfx-notation names would be compared according to the
    // number (e.g. gfx900 is less than gfx902 and gfx902 is less than gfx906).
    static bool DeviceNameLessThan(const std::string& a, const std::string& b);

    // Disassembles the given Code Object, sets the output variables with the entire disassembly
    // and with the .text section disassembly. If shouldPrintCmd is set to true, the invocation
    // command of the external process would be printed to the console.
    static bool DisassembleCodeObject(const std::string& coFileName, bool shouldPrintCmd,
        std::string& disassemblyAll, std::string& disassemblyText, std::string& errorMsg);

    // Extracts statistics from a given Code Object's disassembly.
    // Returns true on success and false otherwise.
    static bool ExtractCodeObjectStatistics(const std::string& disassemblyWhole ,
        std::map<std::string, beKA::AnalysisData>& data);

private:
    // No instances for this class, as this is a static utility class.
    beUtils() = default;
    beUtils(const beUtils& other) = default;
    ~beUtils() = default;
};

#endif // beUtils_h__

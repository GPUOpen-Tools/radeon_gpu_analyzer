//=================================================================
// Copyright 2020 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

#ifndef RGA_RADEONGPUANALYZERBACKEND_SRC_BE_UTILS_H_
#define RGA_RADEONGPUANALYZERBACKEND_SRC_BE_UTILS_H_

// C++.
#include <vector>
#include <set>
#include <string>
#include <map>

// Device info.
#include "DeviceInfo.h"

// Local.
#include "RadeonGPUAnalyzerBackend/Src/be_data_types.h"
#include "RadeonGPUAnalyzerBackend/Src/be_include.h"

// Infra.
#ifdef _WIN32
    #pragma warning(push)
    #pragma warning(disable:4309)
#endif
#include "AMDTBaseTools/Include/gtString.h"
#ifdef _WIN32
    #pragma warning(pop)
#endif

// A container that keeps the mapping between some "non-standard" PAL NULL device names
// returned by driver and the "standard" names that can be found in the Device Info lib.
static const std::vector<std::pair<std::string, std::string>>
kPalDeviceNameMapping =
{
    {"bristol",        "bristol ridge"},
    {"polaris10",      "ellesmere"},
    {"polaris11",      "baffin"},
    {"polaris12",      "gfx804"}
};

class BeUtils
{
public:

    // Convert HW generation to its corresponding numerical value.
    static bool GdtHwGenToNumericValue(GDT_HW_GENERATION hw_generation, size_t& gfx_ip);

    // Gets all of the supported graphics cards.
    // If "should_convert_to_lower" is true, the device names returned in "uniqueNamesOfPublishedDevices" will be
    // converted to lower case.
    static bool GetAllGraphicsCards(std::vector<GDT_GfxCardInfo>& card_list,
                                    std::set<std::string>& public_device_unique_names,
                                    bool should_convert_to_lower = false);

    // Gets a mapping of the marketing names to the internal code names.
    static bool GetMarketingNameToCodenameMapping(std::map<std::string, std::set<std::string>>& cards_map);

    // Deletes the physical files from the file system.
    static void DeleteOutputFiles(const BeProgramPipeline& output_file_paths);

    // Deletes a physical file from the file system.
    static void DeleteFileFromDisk(const gtString& file_path);

    // Deletes a physical file from the file system.
    static void DeleteFileFromDisk(const std::string& file_path);

    // Checks if file exists and is not empty.
    static bool IsFilePresent(const std::string& filename);

    // Returns true if the contents of the two given files is identical, and false otherwise.
    static bool IsFilesIdentical(const std::string& filename1, const std::string& filename2);

    // Retrieves the file extension.
    static std::string GetFileExtension(const std::string& filename);

    // Reads a binary file into the vector.
    static bool ReadBinaryFile(const std::string& filename, std::vector<char>& content);

    // Print given string to stdout if "doPrint" is true.
    static void PrintCmdLine(const std::string& cmd_line, bool should_print_cmd);

    // Split the given string according to the given delimiter,
    // and store the results in the destination vector.
    static void SplitString(const std::string& str, char delim, std::vector<std::string>& dst);

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
    static bool DisassembleCodeObject(const std::string& code_object_filename, bool should_print_cmd,
        std::string& disassembly_whole, std::string& disassembly_text, std::string& error_msg);

    // Extracts statistics from a given Code Object's disassembly.
    // Returns true on success and false otherwise.
    static bool ExtractCodeObjectStatistics(const std::string& disassembly_whole,
        std::map<std::string, beKA::AnalysisData>& data);

private:
    // No instances for this class, as this is a static utility class.
    BeUtils() = default;
    BeUtils(const BeUtils& other) = default;
    ~BeUtils() = default;
};

#endif // RGA_RADEONGPUANALYZERBACKEND_SRC_BE_UTILS_H_

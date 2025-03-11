//=================================================================
// Copyright 2020-2021 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

#ifndef RGA_RADEONGPUANALYZERBACKEND_SRC_BE_UTILS_H_
#define RGA_RADEONGPUANALYZERBACKEND_SRC_BE_UTILS_H_

// C++.
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

// Device info.
#include "DeviceInfo.h"

// Local.
#include "radeon_gpu_analyzer_backend/be_data_types.h"
#include "radeon_gpu_analyzer_backend/be_include.h"

// Infra.
#include "external/amdt_base_tools/Include/gtString.h"

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

    // Gets all of the supported pre-RDNA3 graphics cards (gfx9 and gfx10).
    // If "should_convert_to_lower" is true, the device names returned in "uniqueNamesOfPublishedDevices" will be
    // converted to lower case.
    static bool GetPreRdna3GraphicsCards(std::vector<GDT_GfxCardInfo>& card_list,
                                    std::set<std::string>&        public_device_unique_names,
                                    bool                          should_convert_to_lower = false);

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

    // Trim leading and trailing whitespace characters.
    static void TrimLeadingAndTrailingWhitespace(const std::string& text, std::string& trimmed_text);

    // Returns true if the given string represents a numeric value, and false otherwise.
    static bool IsNumericValue(const std::string& str);

    // Returns true if device name a is "less than" b. This is used when sorting containers.
    // A gfx-notation device name, which indicates a Vega+ target, will never be "less than"
    // a non-gfx notation name. Two non-gfx notation names would be compared using the usual
    // string comparison logic, and two gfx-notation names would be compared according to the
    // number (e.g. gfx900 is less than gfx902 and gfx902 is less than gfx906).
    static bool DeviceNameLessThan(const std::string& a, const std::string& b);

    // Extracts statistics from a given Code Object's disassembly.
    // Returns true on success and false otherwise.
    static bool ExtractCodeObjectStatistics(const std::string& disassembly_whole,
        std::map<std::string, beKA::AnalysisData>& data);

    // Returns true if the given shader stage name is a valid amdgpu-dis shader stage name and false otherwise.
    static bool IsValidAmdgpuShaderStage(const std::string& shader_stage);

    // Sets the output string to the amdgpu-dis shader stage that corresponds to the given pipeline stage.
    // Returns true upon success, false otherwise.
    static bool BePipelineStageToAmdgpudisStageName(BePipelineStage pipeline_stage, std::string& amdgpu_dis_stage);

    // Sets the output stage to the ray tracing shader stage that corresponds to the given amdgpu-dis shader stage name.
    // Returns true upon success, false otherwise.
    static bool BeAmdgpudisStageNameToBeRayTracingStage(const std::string& amdgpu_dis_stage, std::size_t& ray_tracing_stage);

    // String helper StartsWith.
    static inline auto StartsWith(std::string_view str, std::string_view prefix) -> bool
    {
        // Optimization: 'pos' == 0 limits the search for the prefix.
        // The function does not search backwards through the entire string!
        return str.rfind(prefix, 0) == 0;
    }

    // String helper EndsWith.
    static inline auto EndsWith(std::string_view str, std::string_view suffix) -> bool
    {
        if (str.length() < suffix.length())
        {
            return false;
        }
        return std::equal(suffix.rbegin(), suffix.rend(), str.rbegin());
    }

    // String helper MatchWith for equality.
    static inline auto MatchWith(std::string_view str, std::string_view other) -> bool
    {
        return str.compare(other) == 0;
    }

    // String helper ContainsAny of part_strings.
    static inline auto ContainsAny(std::string_view str, std::initializer_list<std::string_view> part_strings) -> bool
    {
        return std::any_of(part_strings.begin(), part_strings.end(), [&str = std::as_const(str)](std::string_view partString) {
            return str.find(partString) != std::string_view::npos;
        });
    }

    // String helper StartsWithAny of part_strings.
    static inline auto StartsWithAny(std::string_view str, std::initializer_list<std::string_view> part_strings) -> bool
    {
        return std::any_of(
            part_strings.begin(), part_strings.end(), [&str = std::as_const(str)](std::string_view part_string) { return StartsWith(str, part_string); });
    }

    // String helper StartsWithAny of part_strings.
    static inline auto EndsWithAny(std::string_view str, std::initializer_list<std::string_view> part_strings) -> bool
    {
        return std::any_of(
            part_strings.begin(), part_strings.end(), [&str = std::as_const(str)](std::string_view part_string) { return EndsWith(str, part_string); });
    }

    // String helper MatchesWithAny of part_strings.
    static inline auto MatchesWithAny(std::string_view str, std::initializer_list<std::string_view> part_strings) -> bool
    {
        return std::any_of(
            part_strings.begin(), part_strings.end(), [&str = std::as_const(str)](std::string_view part_string) { return MatchWith(str, part_string); });
    }

private:
    // No instances for this class, as this is a static utility class.
    BeUtils() = default;
    BeUtils(const BeUtils& other) = default;
    ~BeUtils() = default;
};

#endif // RGA_RADEONGPUANALYZERBACKEND_SRC_BE_UTILS_H_

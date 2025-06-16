//=============================================================================
/// Copyright (c) 2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for OpenCL helper functions.
//=============================================================================

#ifndef RGA_RADEONGPUANALYZERCLI_SRC_KC_UTILS_LIGHTNING_H_
#define RGA_RADEONGPUANALYZERCLI_SRC_KC_UTILS_LIGHTNING_H_
// C++.
#include <string>

// Backend.
#include "source/radeon_gpu_analyzer_backend/be_include.h"
#include "source/radeon_gpu_analyzer_backend/be_opencl_definitions.h"

// Local.
#include "source/radeon_gpu_analyzer_cli/kc_data_types.h"
#include "radeon_gpu_analyzer_cli/kc_config.h"

// Kernel Header Strings.
static const std::string kLcKernelIsaHeader1 = "AMD Kernel Code for ";
static const std::string kLcKernelIsaHeader2 = "Disassembly for ";
static const std::string kLcKernelIsaHeader3 = "@kernel ";

// Class for OpenCl mode utility functions for ISA post-processing.
class KcUtilsLightning
{
public:
    KcUtilsLightning(RgClOutputMetadata& output_metadata, bool should_print_cmd, LoggingCallbackFunction log_callback)
        : output_metadata_(output_metadata)
        , should_print_cmd_(should_print_cmd)
        , log_callback_(log_callback)
    {
    }

    // Log Error Status.
    static void LogErrorStatus(beKA::beStatus status, const std::string& error_msg);

    // Parse ISA files and generate separate files that contain parsed ISA in CSV format.
    bool ParseIsaFilesToCSV(bool add_line_numbers) const;

    // Perform live VGPR analysis.
    bool PerformLiveVgprAnalysis(const Config& config) const;

    // Perform live VGPR analysis.
    bool PerformLiveSgprAnalysis(const Config& config) const;

    // Extract program Control Flow Graph.
    bool ExtractCFG(const Config& config) const;

    // Get the AMD GPU metadata from the binary.
    beKA::beStatus ExtractMetadata(const CmpilerPaths& compiler_paths, const std::string& metadata_filename) const;

    // Extract Resource Usage (statistics) data.
    beKA::beStatus ExtractStatistics(const Config& config) const;

    // Convert ISA text to CSV form with additional data.
    static bool GetParsedIsaCsvText(const std::string& isaText, const std::string& device, bool add_line_numbers, std::string& csvText);

    // Store ISA text in the file.
    static beKA::beStatus WriteIsaToFile(const std::string& file_name, const std::string& isa_text, LoggingCallbackFunction log_callback);

    // Extract the list of entry points from the source file specified by "fileName".
    static bool ExtractEntries(const std::string& filename, const Config& config, const CmpilerPaths& compiler_paths, RgEntryData& entry_data);

    // Pre-fix ISA Text with Header.
    static std::string PrefixWithISAHeader(const std::string& kernel_name, const std::string& kernel_isa_text);

    // Delete all temporary files created by RGA.
    static void DeleteTempFiles(const RgClOutputMetadata& output_metadata);

private:
    // ---- DATA ----

    // Output Metadata.
    RgClOutputMetadata& output_metadata_;

    // Specifies whether the "-#" option (print commands) is enabled.
    bool should_print_cmd_ = false;

    // Log callback function.
    LoggingCallbackFunction log_callback_;

};
#endif  // RGA_RADEONGPUANALYZERCLI_SRC_KC_UTILS_LIGHTNING_H_

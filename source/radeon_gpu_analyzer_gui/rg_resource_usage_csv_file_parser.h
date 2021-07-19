#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_RESOURCE_USAGE_CSV_FILE_PARSER_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_RESOURCE_USAGE_CSV_FILE_PARSER_H_

// C++.
#include <vector>

// Local.
#include "radeon_gpu_analyzer_gui/rg_csv_file_parser.h"
#include "radeon_gpu_analyzer_gui/rg_data_types.h"

// A file parser used to process Resource Usage CSV files.
class RgResourceUsageCsvFileParser : public RgCsvFileParser
{
public:
    // A constructor that accepts a full path to the CSV file to be parsed.
    RgResourceUsageCsvFileParser(const std::string& csv_file_path) : RgCsvFileParser(csv_file_path) {}

    // Get the resource usage data parsed from the CSV file.
    const RgResourceUsageData& GetData() const { return resource_usage_data_; }

protected:
    // Parse the given line from the CSV file
    virtual bool ProcessLineTokens(const std::vector<std::string>& tokens) override;

private:
    // The variable used to store resource usage parsed from the CSV file.
    RgResourceUsageData resource_usage_data_;

    // An enumeration specifying each column we expect to find in the Resource Usage CSV file.
    enum rgResourceUsageCsvFileColumns
    {
        kDevice,
        kScratchMemory,
        kThreadsPerWorkgroup,
        kWavefrontSize,
        kAvailableLdsBytes,
        kUsedLdsBytes,
        kAvailableSgprs,
        kUsedSgprs,
        kSgprSpills,
        kAvailableVgprs,
        kUsedVgprs,
        kVgprSpills,
        kClWorkgroupXDimension,
        kClWorkgroupYDimension,
        kClWorkgroupZDimension,
        kIsaSize,
        kCount
    };
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_RESOURCE_USAGE_CSV_FILE_PARSER_H_

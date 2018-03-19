#pragma once

// C++.
#include <vector>

// Local.
#include <RadeonGPUAnalyzerGUI/include/rgCsvFileParser.h>
#include <RadeonGPUAnalyzerGUI/include/rgDataTypes.h>

// A file parser used to process Resource Usage CSV files.
class rgResourceUsageCsvFileParser : public rgCsvFileParser
{
public:
    // A constructor that accepts a full path to the CSV file to be parsed.
    rgResourceUsageCsvFileParser(const std::string& csvFilePath) : rgCsvFileParser(csvFilePath) {}

    // Get the resource usage data parsed from the CSV file.
    const rgResourceUsageData& GetData() const { return m_resourceUsageData; }

protected:
    // Parse the given line from the CSV file
    virtual bool ProcessLineTokens(const std::vector<std::string>& tokens) override;

private:
    // The variable used to store resource usage parsed from the CSV file.
    rgResourceUsageData m_resourceUsageData;

    // An enumeration specifying each column we expect to find in the Resource Usage CSV file.
    enum rgResourceUsageCsvFileColumns
    {
        Device,
        ScratchMemory,
        ThreadsPerWorkgroup,
        WavefrontSize,
        AvailableLdsBytes,
        UsedLdsBytes,
        AvailableSgprs,
        UsedSgprs,
        SgprSpills,
        AvailableVgprs,
        UsedVgprs,
        VgprSpills,
        ClWorkgroupXDimension,
        ClWorkgroupYDimension,
        ClWorkgroupZDimension,
        IsaSize,
        Count
    };
};
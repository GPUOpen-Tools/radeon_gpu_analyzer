#pragma once

// C++.
#include <string>
#include <AMDTBaseTools/Include/gtString.h>

// Backend.
#include <RadeonGPUAnalyzerBackend/include/beInclude.h>

class IStatisticsParser
{
public:
    IStatisticsParser() {}
    virtual ~IStatisticsParser() {}

    /// Parse the given raw statistics file and extract CLI statistics.
    /// Params:
    ///     Input: the full path to the raw statistics file.
    ///     Output: the statistics in a parsed statistics.
    /// Return value: true for success, false otherwise.
    virtual bool ParseStatistics(const gtString& statisticsFile, beKA::AnalysisData& statistics) = 0;
};

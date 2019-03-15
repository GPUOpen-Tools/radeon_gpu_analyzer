#pragma once

// C++.
#include <string>

// Infra.
#ifdef _WIN32
    #pragma warning(push)
    #pragma warning(disable:4309)
#endif
#include <AMDTBaseTools/Include/gtString.h>
#ifdef _WIN32
    #pragma warning(pop)
#endif

// Backend.
#include <RadeonGPUAnalyzerBackend/Include/beInclude.h>

class IStatisticsParser
{
public:
    IStatisticsParser() {}
    virtual ~IStatisticsParser() {}

    // Parse the given raw statistics file and extract CLI statistics.
    // Params:
    //     device: the name of the device for which the statistics are generated.
    //     statisticsFile: the full path to the raw statistics file.
    //     statistics: the statistics in a parsed statistics.
    // Return value: true for success, false otherwise.
    virtual bool ParseStatistics(const std::string& device, const gtString& statisticsFile, beKA::AnalysisData& statistics) = 0;
};

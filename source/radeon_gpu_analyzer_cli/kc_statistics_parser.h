//=================================================================
// Copyright 2020-2021 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================
#ifndef RGA_RADEONGPUANALYZERCLI_SRC_KC_STATISTICS_PARSER_H_
#define RGA_RADEONGPUANALYZERCLI_SRC_KC_STATISTICS_PARSER_H_

// C++.
#include <string>

// Infra.
#include "external/amdt_base_tools/Include/gtString.h"

// Backend.
#include "source/radeon_gpu_analyzer_backend/be_include.h"

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

#endif // RGA_RADEONGPUANALYZERCLI_SRC_KC_STATISTICS_PARSER_H_

//=================================================================
// Copyright 2017 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

#pragma once

// Local.
#include <RadeonGPUAnalyzerCLI/src/kcIStatisticsParser.h>

class kcVulkanStatisticsParser :
    public IStatisticsParser
{
public:
    kcVulkanStatisticsParser();
    virtual ~kcVulkanStatisticsParser();

    virtual bool ParseStatistics(const gtString& statisticsFilePath, beKA::AnalysisData& parsedStatistics) override;

};

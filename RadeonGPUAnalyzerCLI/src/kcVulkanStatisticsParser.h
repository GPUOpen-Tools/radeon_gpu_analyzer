//=================================================================
// Copyright 2017 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

#pragma once

// Local.
#include <RadeonGPUAnalyzerCLI/Src/kcIStatisticsParser.h>

class kcVulkanStatisticsParser :
    public IStatisticsParser
{
public:
    kcVulkanStatisticsParser();
    virtual ~kcVulkanStatisticsParser();

    virtual bool ParseStatistics(const std::string& device, const gtString& statisticsFile, beKA::AnalysisData& statistics) override;

};

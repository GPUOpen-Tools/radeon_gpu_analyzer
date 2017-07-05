//=================================================================
// Copyright 2017 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

#ifndef kcVulkanStatisticsParser_h__
#define kcVulkanStatisticsParser_h__

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

#endif // kcVulkanStatisticsParser_h__

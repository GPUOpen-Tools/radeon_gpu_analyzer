//=================================================================
// Copyright 2017 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

#pragma once

// Local.
#include <RadeonGPUAnalyzerCLI/src/kcIStatisticsParser.h>

class kcOpenGLStatisticsParser :
    public IStatisticsParser
{
public:
    kcOpenGLStatisticsParser();
    virtual ~kcOpenGLStatisticsParser();

    virtual bool ParseStatistics(const gtString& scStatistics, beKA::AnalysisData& statisticsAsCsv) override;

};

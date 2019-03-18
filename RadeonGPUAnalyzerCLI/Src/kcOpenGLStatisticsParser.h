//=================================================================
// Copyright 2017 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

#pragma once

// Local.
#include <RadeonGPUAnalyzerCLI/Src/kcIStatisticsParser.h>

class kcOpenGLStatisticsParser :
    public IStatisticsParser
{
public:
    kcOpenGLStatisticsParser();
    virtual ~kcOpenGLStatisticsParser();

    virtual bool ParseStatistics(const std::string& device, const gtString& statisticsFile, beKA::AnalysisData& statistics) override;

};

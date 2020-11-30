//=================================================================
// Copyright 2020 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================
#ifndef RGA_RADEONGPUANALYZERCLI_SRC_KC_OPENGL_STATISTICS_PARSER_H_
#define RGA_RADEONGPUANALYZERCLI_SRC_KC_OPENGL_STATISTICS_PARSER_H_

// Local.
#include "RadeonGPUAnalyzerCLI/Src/kc_statistics_parser.h"

class KcOpenGLStatisticsParser :
    public IStatisticsParser
{
public:
    KcOpenGLStatisticsParser() = default;
    virtual ~KcOpenGLStatisticsParser() = default;
    virtual bool ParseStatistics(const std::string& device, const gtString& statistics_file, beKA::AnalysisData& statistics) override;
};

#endif // RGA_RADEONGPUANALYZERCLI_SRC_KC_OPENGL_STATISTICS_PARSER_H_

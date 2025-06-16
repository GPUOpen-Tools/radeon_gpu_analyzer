//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for class parsing statistics for vulkan.
//=============================================================================
#ifndef RGA_RADEONGPUANALYZERCLI_SRC_KC_VULKAN_STATISTICS_PARSER_H_
#define RGA_RADEONGPUANALYZERCLI_SRC_KC_VULKAN_STATISTICS_PARSER_H_

// Local.
#include "radeon_gpu_analyzer_cli/kc_statistics_parser.h"

class KcVulkanStatisticsParser :
    public IStatisticsParser
{
public:
    KcVulkanStatisticsParser();
    virtual ~KcVulkanStatisticsParser();

    virtual bool ParseStatistics(const std::string& device, const gtString& statistics_file, beKA::AnalysisData& statistics) override;
};

#endif // RGA_RADEONGPUANALYZERCLI_SRC_KC_VULKAN_STATISTICS_PARSER_H_

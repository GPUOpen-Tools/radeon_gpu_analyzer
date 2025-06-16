//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for the rga gui utilities for Graphics APIs.
//=============================================================================
#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_UTILS_GRAPHICS_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_UTILS_GRAPHICS_H_

// C++.
#include <cassert>
#include <memory>
#include <string>

// Local.
#include "radeon_gpu_analyzer_gui/rg_data_types.h"

// A utilities class that can be implemented per graphics API.
class RgUtilsGraphics
{
public:
    virtual ~RgUtilsGraphics() = default;

    // Get a utility factory instance based on the given API.
    static std::shared_ptr<RgUtilsGraphics> CreateUtility(RgProjectAPI api);

    // Get the abbreviated stage name string for the given pipeline stage.
    virtual std::string PipelineStageToAbbreviation(RgPipelineStage pipeline_stage) = 0;

    // Get the API-specific name for the given pipeline stage.
    virtual std::string PipelineStageToString(RgPipelineStage pipeline_stage) = 0;

    // Get the default source code for the given stage.
    virtual std::string GetDefaultShaderCode(RgPipelineStage pipeline_stage) = 0;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_UTILS_GRAPHICS_H_

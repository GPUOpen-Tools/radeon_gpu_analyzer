#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_UTILS_VULKAN_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_UTILS_VULKAN_H_

// Local.
#include "radeon_gpu_analyzer_gui/rg_utils_graphics.h"

class RgUtilsVulkan : public RgUtilsGraphics
{
public:
    virtual ~RgUtilsVulkan() = default;

    // Get the Vulkan glsl abbreviation string for the given pipeline stage.
    virtual std::string PipelineStageToAbbreviation(RgPipelineStage pipeline_stage) override;

    // Get the API-specific name for the given pipeline stage.
    virtual std::string PipelineStageToString(RgPipelineStage pipeline_stage) override;

    // Get the default source code for the given stage.
    virtual std::string GetDefaultShaderCode(RgPipelineStage pipeline_stage) override;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_UTILS_VULKAN_H_

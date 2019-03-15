#pragma once

// Local.
#include <RadeonGPUAnalyzerGUI/Include/rgUtilsGraphics.h>

class rgUtilsVulkan : public rgUtilsGraphics
{
public:
    virtual ~rgUtilsVulkan() = default;

    // Get the Vulkan glsl abbreviation string for the given pipeline stage.
    virtual std::string PipelineStageToAbbreviation(rgPipelineStage pipelineStage) override;

    // Get the API-specific name for the given pipeline stage.
    virtual std::string PipelineStageToString(rgPipelineStage pipelineStage) override;

    // Get the default source code for the given stage.
    virtual std::string GetDefaultShaderCode(rgPipelineStage pipelineStage) override;
};
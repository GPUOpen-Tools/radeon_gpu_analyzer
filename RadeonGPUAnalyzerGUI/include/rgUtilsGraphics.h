#pragma once

// C++.
#include <cassert>
#include <memory>
#include <string>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/rgDataTypes.h>

// A utilities class that can be implemented per graphics API.
class rgUtilsGraphics
{
public:
    virtual ~rgUtilsGraphics() = default;

    // Get a utility factory instance based on the given API.
    static std::shared_ptr<rgUtilsGraphics> CreateUtility(rgProjectAPI api);

    // Get the abbreviated stage name string for the given pipeline stage.
    virtual std::string PipelineStageToAbbreviation(rgPipelineStage pipelineStage) = 0;

    // Get the API-specific name for the given pipeline stage.
    virtual std::string PipelineStageToString(rgPipelineStage pipelineStage) = 0;

    // Get the default source code for the given stage.
    virtual std::string GetDefaultShaderCode(rgPipelineStage pipelineStage) = 0;
};
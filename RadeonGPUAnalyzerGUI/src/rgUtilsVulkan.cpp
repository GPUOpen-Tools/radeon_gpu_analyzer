// C++.
#include <cassert>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/rgDataTypesVulkan.h>
#include <RadeonGPUAnalyzerGUI/Include/rgUtilsVulkan.h>

std::string rgUtilsVulkan::PipelineStageToAbbreviation(rgPipelineStage pipelineStage)
{
    std::string stageName;
    switch (pipelineStage)
    {
    case rgPipelineStage::Vertex:
        stageName = STR_VULKAN_STAGE_NAME_VERTEX_ABBREVIATION;
        break;
    case rgPipelineStage::TessellationControl:
        stageName = STR_VULKAN_STAGE_NAME_TESSELLATION_CONTROL_ABBREVIATION;
        break;
    case rgPipelineStage::TessellationEvaluation:
        stageName = STR_VULKAN_STAGE_NAME_TESSELLATION_EVALUATION_ABBREVIATION;
        break;
    case rgPipelineStage::Geometry:
        stageName = STR_VULKAN_STAGE_NAME_GEOMETRY_ABBREVIATION;
        break;
    case rgPipelineStage::Fragment:
        stageName = STR_VULKAN_STAGE_NAME_FRAGMENT_ABBREVIATION;
        break;
    case rgPipelineStage::Compute:
        stageName = STR_VULKAN_STAGE_NAME_COMPUTE_ABBREVIATION;
        break;
    default:
        // If this assert fires, the given stage isn't recognized.
        assert(false);
        break;
    }

    return stageName;
}

std::string rgUtilsVulkan::PipelineStageToString(rgPipelineStage pipelineStage)
{
    std::string stageName;
    switch (pipelineStage)
    {
    case rgPipelineStage::Vertex:
        stageName = STR_VULKAN_STAGE_NAME_VERTEX;
        break;
    case rgPipelineStage::TessellationControl:
        stageName = STR_VULKAN_STAGE_NAME_TESSELLATION_CONTROL;
        break;
    case rgPipelineStage::TessellationEvaluation:
        stageName = STR_VULKAN_STAGE_NAME_TESSELLATION_EVALUATION;
        break;
    case rgPipelineStage::Geometry:
        stageName = STR_VULKAN_STAGE_NAME_GEOMETRY;
        break;
    case rgPipelineStage::Fragment:
        stageName = STR_VULKAN_STAGE_NAME_FRAGMENT;
        break;
    case rgPipelineStage::Compute:
        stageName = STR_VULKAN_STAGE_NAME_COMPUTE;
        break;
    default:
        // If this assert fires, the given stage isn't recognized.
        assert(false);
        break;
    }

    return stageName;
}

std::string rgUtilsVulkan::GetDefaultShaderCode(rgPipelineStage pipelineStage)
{
    std::string sourcecode;
    switch (pipelineStage)
    {
    case rgPipelineStage::Vertex:
        sourcecode = STR_NEW_FILE_TEMPLATE_CODE_VULKAN_GLSL_VERTEX_SHADER;
        break;
    case rgPipelineStage::TessellationControl:
        sourcecode = STR_NEW_FILE_TEMPLATE_CODE_VULKAN_GLSL_TESSELLATION_CONTROL_SHADER;
        break;
    case rgPipelineStage::TessellationEvaluation:
        sourcecode = STR_NEW_FILE_TEMPLATE_CODE_VULKAN_GLSL_TESSELLATION_EVALUATION_SHADER;
        break;
    case rgPipelineStage::Geometry:
        sourcecode = STR_NEW_FILE_TEMPLATE_CODE_VULKAN_GLSL_GEOMETRY_SHADER;
        break;
    case rgPipelineStage::Fragment:
        sourcecode = STR_NEW_FILE_TEMPLATE_CODE_VULKAN_GLSL_FRAGMENT_SHADER;
        break;
    case rgPipelineStage::Compute:
        sourcecode = STR_NEW_FILE_TEMPLATE_CODE_VULKAN_GLSL_COMPUTE_SHADER;
        break;
    default:
        // If this assert fires, the given stage isn't recognized.
        assert(false);
        break;
    }

    return sourcecode;
}
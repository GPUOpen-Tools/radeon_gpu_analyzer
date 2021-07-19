// C++.
#include <cassert>

// Local.
#include "radeon_gpu_analyzer_gui/rg_data_types_vulkan.h"
#include "radeon_gpu_analyzer_gui/rg_utils_vulkan.h"

std::string RgUtilsVulkan::PipelineStageToAbbreviation(RgPipelineStage pipeline_stage)
{
    std::string stage_name;
    switch (pipeline_stage)
    {
    case RgPipelineStage::kVertex:
        stage_name = kStrVulkanStageNameVertexAbbreviation;
        break;
    case RgPipelineStage::kTessellationControl:
        stage_name = kStrVulkanStageNameTessellationControlAbbreviation;
        break;
    case RgPipelineStage::kTessellationEvaluation:
        stage_name = kStrVulkanStageNameTessellationEvaluationAbbreviation;
        break;
    case RgPipelineStage::kGeometry:
        stage_name = kStrVulkanStageNameGeometryAbbreviation;
        break;
    case RgPipelineStage::kFragment:
        stage_name = kStrVulkanStageNameFragmentAbbreviation;
        break;
    case RgPipelineStage::kCompute:
        stage_name = kStrVulkanStageNameComputeAbbreviation;
        break;
    default:
        // If this assert fires, the given stage isn't recognized.
        assert(false);
        break;
    }

    return stage_name;
}

std::string RgUtilsVulkan::PipelineStageToString(RgPipelineStage pipeline_stage)
{
    std::string stage_name;
    switch (pipeline_stage)
    {
    case RgPipelineStage::kVertex:
        stage_name = kStrVulkanStageNameVertex;
        break;
    case RgPipelineStage::kTessellationControl:
        stage_name = kStrVulkanStageNameTessellationControl;
        break;
    case RgPipelineStage::kTessellationEvaluation:
        stage_name = kStrVulkanStageNameTessellationEvaluation;
        break;
    case RgPipelineStage::kGeometry:
        stage_name = kStrVulkanStageNameGeometry;
        break;
    case RgPipelineStage::kFragment:
        stage_name = kStrVulkanStageNameFragment;
        break;
    case RgPipelineStage::kCompute:
        stage_name = kStrVulkanStageNameCompute;
        break;
    default:
        // If this assert fires, the given stage isn't recognized.
        assert(false);
        break;
    }

    return stage_name;
}

std::string RgUtilsVulkan::GetDefaultShaderCode(RgPipelineStage pipeline_stage)
{
    std::string source_code;
    switch (pipeline_stage)
    {
    case RgPipelineStage::kVertex:
        source_code = kStrNewFileTemplateCodeVulkanGlslVertexShader;
        break;
    case RgPipelineStage::kTessellationControl:
        source_code = kStrNewFileTemplateCodeVulkanGlslTessellationControlShader;
        break;
    case RgPipelineStage::kTessellationEvaluation:
        source_code = kStrNewFileTemplateCodeVulkanGlslTessellationEvaluationShader;
        break;
    case RgPipelineStage::kGeometry:
        source_code = kStrNewFileTemplateCodeVulkanGlslGeometryShader;
        break;
    case RgPipelineStage::kFragment:
        source_code = kStrNewFileTemplateCodeVulkanGlslFragmentShader;
        break;
    case RgPipelineStage::kCompute:
        source_code = kStrNewFileTemplateCodeVulkanGlslComputeShader;
        break;
    default:
        // If this assert fires, the given stage isn't recognized.
        assert(false);
        break;
    }

    return source_code;
}

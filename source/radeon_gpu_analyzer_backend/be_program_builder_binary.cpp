//=============================================================================
/// Copyright (c) 2023-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for rga backend progam builder binary analysis class.
//=============================================================================

// C++
#include <cassert>

// Local
#include "radeon_gpu_analyzer_backend/be_program_builder_binary.h"
#include "radeon_gpu_analyzer_backend/be_utils.h"

// Cli
#include "source/radeon_gpu_analyzer_cli/kc_cli_string_constants.h"

bool beProgramBuilderBinary::IsGraphicsApi(ApiEnum api)
{
    bool ret = false;
    switch (api)
    {
    case ApiEnum::kDXR:
    case ApiEnum::kDX12:
    case ApiEnum::kVulkan:
    case ApiEnum::kVulkanRT:
    case ApiEnum::kOpenGL:
        ret = true;
        break;
    default:
        ret = false;
        break;
    }
    return ret;
}

beProgramBuilderBinary::ApiEnum beProgramBuilderBinary::GetApiFromStr(const std::string& api_str)
{
    ApiEnum api = ApiEnum::kUnknown;
    if (api_str == kStrRgaModeDxr)
    {
        api = ApiEnum::kDXR;
    }
    else if (api_str == kStrRgaModeDx12)
    {
        api = ApiEnum::kDX12;
    }
    else if (api_str == kStrRgaModeVulkan)
    {
        api = ApiEnum::kVulkan;
    }
    else if (api_str == kStrRgaModeVulkanRT)
    {
        api = ApiEnum::kVulkanRT;
    }
    else if (api_str == kStrRgaModeOpengl)
    {
        api = ApiEnum::kOpenGL;
    }
    return api;
}

std::string beProgramBuilderBinary::GetStrFromApi(ApiEnum api)
{
    std::string api_string = "Invalid";

    switch (api)
    {
    case ApiEnum::kDXR:
        api_string = kStrRgaModeDxr;
        break;
    case ApiEnum::kDX12:
        api_string = kStrRgaModeDx12;
        break;
    case ApiEnum::kVulkan:
        api_string = kStrRgaModeVulkan;
        break;
    case ApiEnum::kVulkanRT:
        api_string = kStrRgaModeVulkanRT;
        break;
    case ApiEnum::kOpenGL:
        api_string = kStrRgaModeOpengl;
        break;
    default:
        assert(false);
        break;
    }

    return api_string;
}

BeVkPipelineFiles beProgramBuilderBinary::GetStageFileSuffixesFromApi(ApiEnum api)
{
    BeVkPipelineFiles suffixes;

    switch (api)
    {
    case ApiEnum::kDX12:
        suffixes = kStrDx12StageSuffix;
        break;
    case ApiEnum::kVulkan:
        suffixes = kVulkanStageFileSuffix;
        break;
    case ApiEnum::kOpenGL:
        suffixes = kVulkanStageFileSuffix;
        break;
    default:
        assert(false);
        break;
    }

    return suffixes;
}

beProgramBuilderBinary::ApiEnum beProgramBuilderBinary::GetApiFromPipelineMetadata(const BeAmdPalMetaData::PipelineMetaData& pipeline)
{
    ApiEnum api = GetApiFromStr(pipeline.api);
    if (IsGraphicsApi(api))
    {
        bool is_raytracing_binary = false;
        if (!pipeline.shader_functions.empty())
        {
            for (const auto& shader_function : pipeline.shader_functions)
            {
                if (shader_function.shader_subtype != BeAmdPalMetaData::ShaderSubtype::kUnknown)
                {
                    is_raytracing_binary = true;
                    break;
                }
            }
        }
        for (const auto& shader : pipeline.shaders)
        {
            if (shader.shader_subtype != BeAmdPalMetaData::ShaderSubtype::kUnknown)
            {
                // Metadata contains a .shader section where one of the shader has a .shader_subtype which is a raytracing stage.
                is_raytracing_binary = true;
                break;
            }
        }

        if (is_raytracing_binary)
        {
            if (api == ApiEnum::kDX12)
            {
                api = ApiEnum::kDXR;
            }
            else if (api == ApiEnum::kVulkan)
            {
                api = ApiEnum::kVulkanRT;
            }
        }
    }
    return api;
}

RgaEntryType beProgramBuilderBinary::GetEntryType(ApiEnum api, uint32_t stage)
{
    RgaEntryType entry = RgaEntryType::kUnknown;
    if (api == ApiEnum::kDXR || api == ApiEnum::kVulkanRT)
    {
        assert(BeRtxPipelineStage::kRayGeneration <= stage && stage < BeRtxPipelineStage::kCountRtx);
        if (BeRtxPipelineStage::kRayGeneration <= stage && stage < BeRtxPipelineStage::kCountRtx)
        {
            if (api == ApiEnum::kDXR)
            {
                // An array containing per-stage RgEntryType(s).
                typedef std::array<RgaEntryType, BeRtxPipelineStage::kCountRtx> BeDxrPipelineEntries;

                // Output metadata entry types for Dxr rt pipeline stages.
                static const BeDxrPipelineEntries kDxrStageEntryTypes = {
                    RgaEntryType::kDxrRayGeneration,
                    RgaEntryType::kDxrIntersection,
                    RgaEntryType::kDxrAnyHit,
                    RgaEntryType::kDxrClosestHit,
                    RgaEntryType::kDxrMiss,
                    RgaEntryType::kDxrCallable,
                    RgaEntryType::kDxrTraversal,
                    RgaEntryType::kDxrUnknown,
                };
                entry = kDxrStageEntryTypes[stage];
            }
            else if (api == ApiEnum::kVulkanRT)
            {
                // An array containing per-stage RgEntryType(s).
                typedef std::array<RgaEntryType, BeRtxPipelineStage::kCountRtx> BeVkRtPipelineEntries;

                // Output metadata entry types for Vulkan raytracing pipeline stages.
                static const BeVkRtPipelineEntries kVkRayTracingStageEntryTypes = {RgaEntryType::kVkRayGeneration,
                                                                                   RgaEntryType::kVkIntersection,
                                                                                   RgaEntryType::kVkAnyHit,
                                                                                   RgaEntryType::kVkClosestHit,
                                                                                   RgaEntryType::kVkMiss,
                                                                                   RgaEntryType::kVkCallable,
                                                                                   RgaEntryType::kVkTraversal,
                                                                                   RgaEntryType::kVkUnknown};
                entry                                                           = kVkRayTracingStageEntryTypes[stage];
            }
        }
    }
    else
    {
        assert(BePipelineStage::kVertex <= stage && stage < BePipelineStage::kCount);
        if (BePipelineStage::kVertex <= stage && stage < BePipelineStage::kCount)
        {
            // An array containing per-stage RgEntryType(s).
            typedef std::array<RgaEntryType, BePipelineStage::kCount> BeVkPipelineEntries;

            switch (api)
            {
            case ApiEnum::kDX12:
            {
                // Output metadata entry types for Vulkan pipeline stages.
                static const BeVkPipelineEntries kDx12StageEntryTypes = {RgaEntryType::kDxVertex,
                                                                         RgaEntryType::kDxHull,
                                                                         RgaEntryType::kDxDomain,
                                                                         RgaEntryType::kDxGeometry,
                                                                         RgaEntryType::kDxPixel,
                                                                         RgaEntryType::kDxCompute,
                                                                         RgaEntryType::kDxMesh,
                                                                         RgaEntryType::kDxAmplification};
                entry                                                 = kDx12StageEntryTypes[stage];
                break;
            }
            case ApiEnum::kVulkan:
            {
                // Output metadata entry types for Vulkan pipeline stages.
                static const BeVkPipelineEntries kVulkanStageEntryTypes = {
                    RgaEntryType::kVkVertex,
                    RgaEntryType::kVkTessControl,
                    RgaEntryType::kVkTessEval,
                    RgaEntryType::kVkGeometry,
                    RgaEntryType::kVkFragment,
                    RgaEntryType::kVkCompute,
                    RgaEntryType::kVkMesh,
                    RgaEntryType::kVkTask,
                };
                entry = kVulkanStageEntryTypes[stage];
                break;
            }

            case ApiEnum::kOpenGL:
            {
                // Output metadata entry types for Vulkan pipeline stages.
                static const BeVkPipelineEntries kOpenGLStageEntryTypes = {RgaEntryType::kGlVertex,
                                                                           RgaEntryType::kGlTessControl,
                                                                           RgaEntryType::kGlTessEval,
                                                                           RgaEntryType::kGlGeometry,
                                                                           RgaEntryType::kGlFragment,
                                                                           RgaEntryType::kGlCompute};
                entry                                                   = kOpenGLStageEntryTypes[stage];
                break;
            }
            default:
                assert(false);
                break;
            }
        }
    }
    return entry;
}

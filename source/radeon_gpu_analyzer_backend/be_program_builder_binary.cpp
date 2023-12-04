//======================================================================
// Copyright 2020-2023 Advanced Micro Devices, Inc. All rights reserved.
//======================================================================

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
        if (!pipeline.shader_functions.empty())
        {
            api = ApiEnum::kDXR;
        }
        for (const auto& shader : pipeline.shaders)
        {
            if (shader.shader_subtype != BeAmdPalMetaData::ShaderSubtype::kUnknown)
            {
                api = ApiEnum::kDXR;
                break;
            }
        }
    }
    return api;
}

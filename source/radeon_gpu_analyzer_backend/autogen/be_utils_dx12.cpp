//=================================================================
// Copyright 2024 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

// Infra.
#include "external/amdt_os_wrappers/Include/osFilePath.h"

// Shared.
#include "common/rga_shared_utils.h"

// Local.
#include "radeon_gpu_analyzer_backend/autogen/be_utils_dx12.h"
#include "radeon_gpu_analyzer_backend/be_utils.h"

// C++
#include <cassert>

std::string BeDx12Utils::GetShaderModelPrefix(BePipelineStage stage)
{
    std::string ret;
    switch (stage)
    {
    case kVertex:
        ret = "vs";
        break;
    case kTessellationControl:
        ret = "hs";
        break;
    case kTessellationEvaluation:
        ret = "ds";
        break;
    case kGeometry:
        ret = "gs";
        break;
    case kFragment:
        ret = "ps";
        break;
    case kCompute:
        ret = "cs";
        break;
    case kCount:
    default:
        // We shouldn't get here.
        assert(false);
        break;
    }
    return ret;
}

std::string BeDx12Utils::GenerateShaderModel(const std::string& shader_model, const Config& config, BePipelineStage stage)
{
    std::string ret;
    if (!shader_model.empty())
    {
        ret = shader_model;
    }
    else
    {
        // Auto-generate the shader model.
        bool is_shader_model_generated = false;
        if (!config.all_model.empty())
        {
            std::stringstream model;
            std::string       shader_model_prefix = GetShaderModelPrefix(stage);
            if (!shader_model_prefix.empty())
            {
                model << shader_model_prefix << "_" << config.all_model;
                ret                       = model.str();
                is_shader_model_generated = true;
            }
        }
    }
    return ret;
}


bool BeDx12Utils::GetShaderStageSourceFileName(const Config& config, const BePipelineStage& stage, std::string& source_filename)
{
    bool ret = true;
    switch (stage)
    {
    case BePipelineStage::kVertex:
        if (!config.vs_hlsl.empty())
        {
            source_filename = BeDx12Utils::GetAbsoluteFileName(config.vs_hlsl);
            break;
        }
        [[fallthrough]];
    case BePipelineStage::kFragment:
        if (!config.ps_hlsl.empty())
        {
            source_filename = BeDx12Utils::GetAbsoluteFileName(config.ps_hlsl);
            break;
        }
        [[fallthrough]];
    case BePipelineStage::kCompute:
        if (!config.cs_hlsl.empty())
        {
            source_filename = BeDx12Utils::GetAbsoluteFileName(config.cs_hlsl);
            break;
        }
        // for each of the stages, check if --all-hlsl option was specified.
        if (!config.all_hlsl.empty())
        {
            source_filename = BeDx12Utils::GetAbsoluteFileName(config.all_hlsl);
            break;
        }
        [[fallthrough]];
    default:
        ret = false;
    }
    return ret;
}

std::wstring BeDx12Utils::asCharArray(const std::string& ss)
{
    gtString gt_str;
    gt_str << ss.c_str();
    return gt_str.asCharArray();
}

std::string BeDx12Utils::asASCIICharArray(const std::wstring& ws)
{
    gtString gt_str;
    gt_str << ws.c_str();
    return gt_str.asASCIICharArray();
}

const wchar_t* BeDx12Utils::asCharArray(const void* ptr)
{
    return static_cast<const wchar_t*>(ptr);
}

const char* BeDx12Utils::asASCIICharArray(const void* ptr)
{
    return static_cast<const char*>(ptr);
}

bool BeDx12Utils::MaskToComponentCount(BYTE mask, BYTE& component_count)
{
    bool ret = true;
    switch (mask)
    {
    case 0x1:
    case 0x2:
    case 0x4:
    case 0x8:
        component_count = 1;
        break;

    case 0x3:
    case 0x6:
    case 0xC:
        component_count = 2;
        break;

    case 0x7:
    case 0xE:
        component_count = 3;
        break;

    case 0xF:
        component_count = 4;
        break;

    default:
        ret = false;
    }
    return ret;
}

beKA::beStatus BeDx12Utils::CheckHr(HRESULT hr)
{
    if (FAILED(hr))
    {
        return beKA::beStatus::kBeStatusDxcCheckHrFailed;
    }

    return beKA::beStatus::kBeStatusSuccess;
}

std::string BeDx12Utils::GetAbsoluteFileName(const std::string& file_path)
{
    if (!file_path.empty())
    {
        gtString gtstr_file_path;
        gtstr_file_path << file_path.c_str();
        osFilePath os_file_path(gtstr_file_path);
        os_file_path.resolveToAbsolutePath();
        return os_file_path.asString().asASCIICharArray();
    }
    return file_path;
}

bool BeDx12Utils::ParseShaderModel(const std::string& shader_model_str, ShaderModelVersion& shader_model)
{
    const char* kStrErrorFailedParsingShaderModel = "Error: failed parsing shader model string.";
    bool        ret                               = true;
    try
    {
        std::vector<std::string> model_components;
        BeUtils::SplitString(shader_model_str, '_', model_components);
        assert(model_components.size() > 2);
        if (model_components.size() > 2)
        {
            bool is_numeric_value = BeUtils::IsNumericValue(model_components[1]);
            assert(is_numeric_value);
            if (is_numeric_value)
            {
                shader_model.major = std::stoi(model_components[1]);
                is_numeric_value   = BeUtils::IsNumericValue(model_components[2]);
                assert(is_numeric_value);
                if (is_numeric_value)
                {
                    shader_model.minor = std::stoi(model_components[2]);
                }
                else
                {
                    // Invalid shader model.
                    ret = false;
                }
            }
            else
            {
                // Invalid shader model.
                ret = false;
            }
        }
    }
    catch (...)
    {
        std::cout << kStrErrorFailedParsingShaderModel << std::endl;
        ret = false;
    }
    return ret;
}

// Returns true if this is DXR Shader mode and false otherwise.
bool BeDx12Utils::IsDxrShaderMode(const Config& config)
{
    std::string dxr_mode = RgaSharedUtils::ToLower(config.dxr_mode);
    return (dxr_mode.compare(kStrDxrModeShader) == 0);
}

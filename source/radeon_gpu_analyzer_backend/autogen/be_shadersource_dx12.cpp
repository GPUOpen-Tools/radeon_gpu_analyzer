//=================================================================
// Copyright 2024 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

// Local.
#include "radeon_gpu_analyzer_backend/autogen/be_reflection_dx12.h"
#include "radeon_gpu_analyzer_backend/autogen/be_utils_dx12.h"

// CLI.
#include "radeon_gpu_analyzer_cli/kc_utils.h"

static bool GetTypeName(D3D_REGISTER_COMPONENT_TYPE component_type, D3D_MIN_PRECISION min_precision, std::uint8_t component_count, std::string& type_name)
{
    bool ret = true;
    switch (component_type)
    {
    case D3D_REGISTER_COMPONENT_FLOAT32:
        if (min_precision == D3D_MIN_PRECISION_FLOAT_16)
        {
            switch (component_count)
            {
            case 1:
                type_name = "min16float";
                break;

            case 2:
                type_name = "min16float2";
                break;

            case 3:
                type_name = "min16float3";
                break;

            case 4:
                type_name = "min16float4";
                break;

            default:
                ret = false;
            }
        }
        else if (min_precision == D3D_MIN_PRECISION_FLOAT_2_8)
        {
            switch (component_count)
            {
            case 1:
                type_name = "min10float";
                break;

            case 2:
                type_name = "min10float2";
                break;

            case 3:
                type_name = "min10float3";
                break;

            case 4:
                type_name = "min10float4";
                break;

            default:
                ret = false;
            }
        }
        else
        {
            switch (component_count)
            {
            case 1:
                type_name = "float";
                break;

            case 2:
                type_name = "float2";
                break;

            case 3:
                type_name = "float3";
                break;

            case 4:
                type_name = "float4";
                break;

            default:
                ret = false;
            }
        }
        break;

    case D3D_REGISTER_COMPONENT_SINT32:
        if (min_precision == D3D_MIN_PRECISION_SINT_16)
        {
            switch (component_count)
            {
            case 1:
                type_name = "min16int";
                break;

            case 2:
                type_name = "min16int2";
                break;

            case 3:
                type_name = "min16int3";
                break;

            case 4:
                type_name = "min16int4";
                break;

            default:
                ret = false;
            }
        }
        else if ((min_precision & D3D_MIN_PRECISION_ANY_10) != 0)  // Is it valid?!
        {
            switch (component_count)
            {
            case 1:
                type_name = "min12int";
                break;

            case 2:
                type_name = "min12int2";
                break;

            case 3:
                type_name = "min12int3";
                break;

            case 4:
                type_name = "min12int4";
                break;

            default:
                ret = false;
            }
        }
        else
        {
            switch (component_count)
            {
            case 1:
                type_name = "int";
                break;

            case 2:
                type_name = "int2";
                break;

            case 3:
                type_name = "int3";
                break;

            case 4:
                type_name = "int4";
                break;

            default:
                ret = false;
            }
        }
        break;

    case D3D_REGISTER_COMPONENT_UINT32:
        if (min_precision == D3D_MIN_PRECISION_UINT_16)
        {
            switch (component_count)
            {
            case 1:
                type_name = "min16uint";
                break;

            case 2:
                type_name = "min16uint2";
                break;

            case 3:
                type_name = "min16uint3";
                break;

            case 4:
                type_name = "min16uint4";
                break;

            default:
                ret = false;
            }
        }
        else if ((min_precision & D3D_MIN_PRECISION_ANY_10) != 0)  // Is it valid?!
        {
            switch (component_count)
            {
            case 1:
                type_name = "min12uint";
                break;

            case 2:
                type_name = "min12uint2";
                break;

            case 3:
                type_name = "min12uint3";
                break;

            case 4:
                type_name = "min12uint4";
                break;

            default:
                ret = false;
            }
        }
        else
        {
            switch (component_count)
            {
            case 1:
                type_name = "uint";
                break;

            case 2:
                type_name = "uint2";
                break;

            case 3:
                type_name = "uint3";
                break;

            case 4:
                type_name = "uint4";
                break;

            default:
                ret = false;
            }
        }
        break;

    default:
        ret = false;
    }
    return ret;
}

// If not supported, returns 0.
static const uint8_t DxgiFormatToComponentCount(DXGI_FORMAT dxgi_format)
{
    switch (dxgi_format)
    {
        case DXGI_FORMAT_UNKNOWN: return 0;
        case DXGI_FORMAT_R32G32B32A32_TYPELESS: return 4;
        case DXGI_FORMAT_R32G32B32A32_FLOAT: return 4;
        case DXGI_FORMAT_R32G32B32A32_UINT: return 4;
        case DXGI_FORMAT_R32G32B32A32_SINT: return 4;
        case DXGI_FORMAT_R32G32B32_TYPELESS: return 3;
        case DXGI_FORMAT_R32G32B32_FLOAT: return 3;
        case DXGI_FORMAT_R32G32B32_UINT: return 3;
        case DXGI_FORMAT_R32G32B32_SINT: return 3;
        case DXGI_FORMAT_R16G16B16A16_TYPELESS: return 4;
        case DXGI_FORMAT_R16G16B16A16_FLOAT: return 4;
        case DXGI_FORMAT_R16G16B16A16_UNORM: return 4;
        case DXGI_FORMAT_R16G16B16A16_UINT: return 4;
        case DXGI_FORMAT_R16G16B16A16_SNORM: return 4;
        case DXGI_FORMAT_R16G16B16A16_SINT: return 4;
        case DXGI_FORMAT_R32G32_TYPELESS: return 2;
        case DXGI_FORMAT_R32G32_FLOAT: return 2;
        case DXGI_FORMAT_R32G32_UINT: return 2;
        case DXGI_FORMAT_R32G32_SINT: return 2;
        case DXGI_FORMAT_R32G8X24_TYPELESS: return 2;
        case DXGI_FORMAT_R10G10B10A2_TYPELESS: return 4;
        case DXGI_FORMAT_R10G10B10A2_UNORM: return 4;
        case DXGI_FORMAT_R10G10B10A2_UINT: return 4;
        case DXGI_FORMAT_R11G11B10_FLOAT: return 3;
        case DXGI_FORMAT_R8G8B8A8_TYPELESS: return 4;
        case DXGI_FORMAT_R8G8B8A8_UNORM: return 4;
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB: return 4;
        case DXGI_FORMAT_R8G8B8A8_UINT: return 4;
        case DXGI_FORMAT_R8G8B8A8_SNORM: return 4;
        case DXGI_FORMAT_R8G8B8A8_SINT: return 4;
        case DXGI_FORMAT_R16G16_TYPELESS: return 2;
        case DXGI_FORMAT_R16G16_FLOAT: return 2;
        case DXGI_FORMAT_R16G16_UNORM: return 2;
        case DXGI_FORMAT_R16G16_UINT: return 2;
        case DXGI_FORMAT_R16G16_SNORM: return 2;
        case DXGI_FORMAT_R16G16_SINT: return 2;
        case DXGI_FORMAT_R32_TYPELESS: return 1;
        case DXGI_FORMAT_D32_FLOAT: return 1;
        case DXGI_FORMAT_R32_FLOAT: return 1;
        case DXGI_FORMAT_R32_UINT: return 1;
        case DXGI_FORMAT_R32_SINT: return 1;
        case DXGI_FORMAT_R24G8_TYPELESS: return 2;
        case DXGI_FORMAT_R8G8_TYPELESS: return 2;
        case DXGI_FORMAT_R8G8_UNORM: return 2;
        case DXGI_FORMAT_R8G8_UINT: return 2;
        case DXGI_FORMAT_R8G8_SNORM: return 2;
        case DXGI_FORMAT_R8G8_SINT: return 2;
        case DXGI_FORMAT_R16_TYPELESS: return 1;
        case DXGI_FORMAT_R16_FLOAT: return 1;
        case DXGI_FORMAT_D16_UNORM: return 1;
        case DXGI_FORMAT_R16_UNORM: return 1;
        case DXGI_FORMAT_R16_UINT: return 1;
        case DXGI_FORMAT_R16_SNORM: return 1;
        case DXGI_FORMAT_R16_SINT: return 1;
        case DXGI_FORMAT_R8_TYPELESS: return 1;
        case DXGI_FORMAT_R8_UNORM: return 1;
        case DXGI_FORMAT_R8_UINT: return 1;
        case DXGI_FORMAT_R8_SNORM: return 1;
        case DXGI_FORMAT_R8_SINT: return 1;
        case DXGI_FORMAT_A8_UNORM: return 1;
        case DXGI_FORMAT_R1_UNORM: return 1;
        case DXGI_FORMAT_R9G9B9E5_SHAREDEXP: return 3;
        case DXGI_FORMAT_B5G6R5_UNORM: return 3;
        case DXGI_FORMAT_B5G5R5A1_UNORM: return 4;
        case DXGI_FORMAT_B8G8R8A8_UNORM: return 4;
        case DXGI_FORMAT_B8G8R8X8_UNORM: return 4;
        case DXGI_FORMAT_B8G8R8A8_TYPELESS: return 4;
        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB: return 4;
        case DXGI_FORMAT_B8G8R8X8_TYPELESS: return 4;
        case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB: return 4;
        case DXGI_FORMAT_B4G4R4A4_UNORM: return 4;
        default: return 0;
    }
}

// For given format of render target or vertex attribute, returns intended HLSL type, e.g.:
// DXGI_FORMAT_R8G8B8A8_UNORM -> float4
// DXGI_FORMAT_R32G32_UINT    -> uint2
// If not supported, returns null.
static const char* DxgiFormatToHlslType(DXGI_FORMAT dxgi_format)
{
    switch (dxgi_format)
    {
        case DXGI_FORMAT_R32G32B32A32_FLOAT: return "float4";
        case DXGI_FORMAT_R32G32B32A32_UINT: return "uint4";
        case DXGI_FORMAT_R32G32B32A32_SINT: return "int4";
        case DXGI_FORMAT_R32G32B32_FLOAT: return "float3";
        case DXGI_FORMAT_R32G32B32_UINT: return "uint3";
        case DXGI_FORMAT_R32G32B32_SINT: return "int3";
        case DXGI_FORMAT_R16G16B16A16_FLOAT: return "float4";
        case DXGI_FORMAT_R16G16B16A16_UNORM: return "float4";
        case DXGI_FORMAT_R16G16B16A16_UINT: return "uint4";
        case DXGI_FORMAT_R16G16B16A16_SNORM: return "float4";
        case DXGI_FORMAT_R16G16B16A16_SINT: return "int4";
        case DXGI_FORMAT_R32G32_FLOAT: return "float2";
        case DXGI_FORMAT_R32G32_UINT: return "uint2";
        case DXGI_FORMAT_R32G32_SINT: return "int2";
        case DXGI_FORMAT_R10G10B10A2_UNORM: return "float4";
        case DXGI_FORMAT_R10G10B10A2_UINT: return "uint4";
        case DXGI_FORMAT_R11G11B10_FLOAT: return "float3";
        case DXGI_FORMAT_R8G8B8A8_UNORM: return "float4";
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB: return "float4";
        case DXGI_FORMAT_R8G8B8A8_UINT: return "uint4";
        case DXGI_FORMAT_R8G8B8A8_SNORM: return "float4";
        case DXGI_FORMAT_R8G8B8A8_SINT: return "int4";
        case DXGI_FORMAT_R16G16_FLOAT: return "float2";
        case DXGI_FORMAT_R16G16_UNORM: return "float2";
        case DXGI_FORMAT_R16G16_UINT: return "uint2";
        case DXGI_FORMAT_R16G16_SNORM: return "float2";
        case DXGI_FORMAT_R16G16_SINT: return "int2";
        case DXGI_FORMAT_R32_UINT: return "uint";
        case DXGI_FORMAT_R32_SINT: return "int";
        case DXGI_FORMAT_R8G8_UNORM: return "float2";
        case DXGI_FORMAT_R8G8_UINT: return "uint2";
        case DXGI_FORMAT_R8G8_SNORM: return "float2";
        case DXGI_FORMAT_R8G8_SINT: return "int2";
        case DXGI_FORMAT_R16_FLOAT: return "float";
        case DXGI_FORMAT_D16_UNORM: return "float";
        case DXGI_FORMAT_R16_UNORM: return "float";
        case DXGI_FORMAT_R16_UINT: return "uint";
        case DXGI_FORMAT_R16_SNORM: return "float";
        case DXGI_FORMAT_R16_SINT: return "int";
        case DXGI_FORMAT_R8_UNORM: return "float";
        case DXGI_FORMAT_R8_UINT: return "uint";
        case DXGI_FORMAT_R8_SNORM: return "float";
        case DXGI_FORMAT_R8_SINT: return "int";
        case DXGI_FORMAT_A8_UNORM: return "float";
        case DXGI_FORMAT_R1_UNORM: return "float";
        case DXGI_FORMAT_R9G9B9E5_SHAREDEXP: return "float3";
        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB: return "float4";
        default: return nullptr;
    }
}


class ShaderSourceGeneratorVertex
{
public:
    ShaderSourceGeneratorVertex();

    beKA::beStatus SetPixelShaderInputs(const D3D12_SIGNATURE_PARAMETER_DESC* params, size_t param_count);

    // This call is optional.
    // If not used, default input layout is assumed as in auto-generated GPSO.
    void SetInputLayoutElements(const D3D12_INPUT_ELEMENT_DESC* elements, size_t count)
    {
        input_elements_.assign(elements, elements + count);
    };

    beKA::beStatus FinalizeVertexShaderSetup(std::string& vertex_shader_string, std::stringstream& err);

private:
    std::vector<D3D12_SIGNATURE_PARAMETER_DESC> vs_outputs_;
    std::vector<D3D12_INPUT_ELEMENT_DESC>       input_elements_;
    std::stringstream                           stream_;
};

ShaderSourceGeneratorVertex::ShaderSourceGeneratorVertex()
{
    // Set default as in auto-generated GPSO.
    D3D12_INPUT_ELEMENT_DESC element = {};
    element.SemanticName             = "POSITION";
    element.Format                   = DXGI_FORMAT_R32G32B32A32_FLOAT;
    element.InputSlotClass           = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
    input_elements_.push_back(element);
}

beKA::beStatus ShaderSourceGeneratorVertex::SetPixelShaderInputs(const D3D12_SIGNATURE_PARAMETER_DESC* params, size_t param_count)
{
    beKA::beStatus rc = beKA::beStatus::kBeStatusSuccess;
    if (!vs_outputs_.empty())
    {
        rc = beKA::beStatus::kBeStatusDxcCannotAutoGenerateVertexShader;
    }
    if (rc == beKA::beStatus::kBeStatusSuccess)
    {
        bool position_found = false;
        for (size_t i = 0; i < param_count; ++i)
        {
            const auto& param = params[i];
            if (param.SystemValueType == D3D_NAME_UNDEFINED || param.SystemValueType == D3D_NAME_POSITION)
            {
                if (param.SystemValueType != D3D_NAME_UNDEFINED && param.SemanticIndex != 0)
                {
                    rc = beKA::beStatus::kBeStatusDxcCannotAutoGenerateVertexShader;
                }

                if (rc == beKA::beStatus::kBeStatusSuccess)
                {
                    // We don't expect PS input to use streams.
                    if (param.Stream != 0)
                    {
                        rc = beKA::beStatus::kBeStatusDxcCannotAutoGenerateVertexShader;
                    }

                    if (rc == beKA::beStatus::kBeStatusSuccess)
                    {
                        vs_outputs_.push_back(param);

                        if (param.SystemValueType == D3D_NAME_POSITION)
                        {
                            // SV_Position must be float4.
                            if (param.ComponentType != D3D_REGISTER_COMPONENT_FLOAT32)
                            {
                                rc = beKA::beStatus::kBeStatusDxcCannotAutoGenerateVertexShader;
                            }
                            position_found = true;
                        }
                    }
                }
            }
            else if (param.SystemValueType == D3D_NAME_VIEWPORT_ARRAY_INDEX || param.SystemValueType == D3D_NAME_SHADINGRATE ||
                     param.SystemValueType == D3D_NAME_CLIP_DISTANCE || param.SystemValueType == D3D_NAME_CULL_DISTANCE)
            {
                vs_outputs_.push_back(param);
            }
            // else:
            // {
            //     Ignore, system value is auto-generated.
            // }
        }

        if (!position_found && rc == beKA::beStatus::kBeStatusSuccess)
        {
            D3D12_SIGNATURE_PARAMETER_DESC param_desc = {};
            param_desc.SemanticName                   = "SV_Position";
            param_desc.SystemValueType                = D3D_NAME_POSITION;
            param_desc.ComponentType                  = D3D_REGISTER_COMPONENT_FLOAT32;
            param_desc.Mask                           = 0xF;
            vs_outputs_.push_back(param_desc);
        }
    }
    return rc;
}

beKA::beStatus ShaderSourceGeneratorVertex::FinalizeVertexShaderSetup(std::string& vertex_shader_string, std::stringstream& err)
{
    beKA::beStatus rc = beKA::beStatus::kBeStatusSuccess;
    stream_ << "// " << kAutoGeneratedShaderHeader << "\n\n";
    stream_ << "struct VsInput\n"
               "{\n";
    for (size_t i = 0; i < input_elements_.size(); ++i)
    {
        const D3D12_INPUT_ELEMENT_DESC& element = input_elements_[i];

        const char* const type = DxgiFormatToHlslType(element.Format);
        if (!type)
        {
            rc = beKA::beStatus::kBeStatusDxcCannotAutoGenerateVertexShader;
            err << "Error: unsupported input layout element type.\n";
            break;
        }

        stream_ << "    " << type << " attribute" << i << ": " << element.SemanticName << element.SemanticIndex << ";\n";
    }

    if (rc == beKA::beStatus::kBeStatusSuccess)
    {
        stream_ << "};\n\n";

        stream_ << "struct VsOutput\n"
                   "{\n";
        for (size_t i = 0; i < vs_outputs_.size(); ++i)
        {
            BYTE component_count;
            if (BeDx12Utils::MaskToComponentCount(vs_outputs_[i].Mask, component_count))
            {
                std::string type;
                if (GetTypeName(vs_outputs_[i].ComponentType, vs_outputs_[i].MinPrecision, component_count, type))
                {
                    stream_ << "    " << type << " attribute" << i << ": " << vs_outputs_[i].SemanticName << vs_outputs_[i].SemanticIndex << ";\n";
                }
                else
                {
                    rc = beKA::beStatus::kBeStatusDxcCannotAutoGenerateVertexShader;
                    break;
                }
            }
            else
            {
                rc = beKA::beStatus::kBeStatusDxcCannotAutoGenerateVertexShader;
                break;
            }
        }

        if (rc == beKA::beStatus::kBeStatusSuccess)
        {
            stream_ << "};\n\n";

            stream_ << "void ";
            stream_ << kVSEntryPoint;
            stream_ << "(VsInput input, out VsOutput output)\n"
                       "{\n"
                       "    float4 result = float4(0.0, 0.0, 0.0, 0.0);\n";

            for (size_t i = 0; i < input_elements_.size(); ++i)
            {
                if (rc == beKA::beStatus::kBeStatusSuccess)
                {
                    switch (DxgiFormatToComponentCount(input_elements_[i].Format))
                    {
                    case 1:
                        stream_ << "    result += float4(input.attribute" << i << ".xxxx);\n";
                        break;
                    case 2:
                        stream_ << "    result += float4(input.attribute" << i << ".xyyy);\n";
                        break;
                    case 3:
                        stream_ << "    result += float4(input.attribute" << i << ".xyzz);\n";
                        break;
                    case 4:
                        stream_ << "    result += float4(input.attribute" << i << ".xyzw);\n";
                        break;
                    default:
                        rc = beKA::beStatus::kBeStatusDxcCannotAutoGenerateVertexShader;
                        err << "Error: unsupported input layout element format.\n";
                        break;
                    }
                }
            }

            for (size_t i = 0; i < vs_outputs_.size(); ++i)
            {
                if (rc == beKA::beStatus::kBeStatusSuccess)
                {
                    BYTE component_count;
                    if (BeDx12Utils::MaskToComponentCount(vs_outputs_[i].Mask, component_count))
                    {
                        std::string type;
                        if (GetTypeName(vs_outputs_[i].ComponentType, vs_outputs_[i].MinPrecision, component_count, type))
                        {
                            switch (component_count)
                            {
                            case 1:
                                stream_ << "    output.attribute" << i << " = " << type << "(result.x);\n";
                                break;
                            case 2:
                                stream_ << "    output.attribute" << i << " = " << type << "(result.xy);\n";
                                break;
                            case 3:
                                stream_ << "    output.attribute" << i << " = " << type << "(result.xyz);\n";
                                break;
                            case 4:
                                stream_ << "    output.attribute" << i << " = " << type << "(result.xyzw);\n";
                                break;
                            default:
                                rc = beKA::beStatus::kBeStatusDxcCannotAutoGenerateVertexShader;
                                break;
                            }
                        }
                    }
                    else
                    {
                        rc = beKA::beStatus::kBeStatusDxcCannotAutoGenerateVertexShader;
                    }
                }                
            }

            if (rc == beKA::beStatus::kBeStatusSuccess)
            {                
                stream_ << "}\n";
                vertex_shader_string = stream_.str();
            }
        }
    }

    return rc;
    
}

class ShaderSourceGeneratorPixel
{
public:
    ShaderSourceGeneratorPixel();

    beKA::beStatus SetVertexShaderOutputs(const D3D12_SIGNATURE_PARAMETER_DESC* params, size_t param_count);
    // This call is optional.
    // If not used, default render targets are assumed as in auto-generated GPSO.
    void SetRenderTargetFormats(const DXGI_FORMAT* formats, size_t count)
    {
        render_target_formats_.assign(formats, formats + count);
    }

    beKA::beStatus FinalizePixelShaderSetup(std::string& pixel_shader_string, std::stringstream& err);

private:
    std::stringstream                           stream_;
    std::vector<D3D12_SIGNATURE_PARAMETER_DESC> ps_outputs_;
    std::vector<DXGI_FORMAT>                    render_target_formats_;
};

ShaderSourceGeneratorPixel::ShaderSourceGeneratorPixel()
{
    // Set default as in auto-generated GPSO.
    render_target_formats_.push_back(DXGI_FORMAT_R8G8B8A8_UNORM);
}

beKA::beStatus ShaderSourceGeneratorPixel::SetVertexShaderOutputs(const D3D12_SIGNATURE_PARAMETER_DESC* params, size_t param_count)
{
    beKA::beStatus rc = beKA::beStatus::kBeStatusSuccess;
    for (size_t i = 0; i < param_count; ++i)
    {
        if (params[i].Stream != 0)
        {
            // We don't expect VS output to use streams.
            rc = beKA::beStatus::kBeStatusDxcCannotAutoGeneratePixelShader;
            break;
        }
        ps_outputs_.push_back(params[i]);
    }
    return rc;
}

beKA::beStatus ShaderSourceGeneratorPixel::FinalizePixelShaderSetup(std::string& pixel_shader_string, std::stringstream& err)
{
    beKA::beStatus rc = beKA::beStatus::kBeStatusSuccess;
    stream_ << "// " << kAutoGeneratedShaderHeader << "\n\n";
    stream_ << "struct PsInput\n"
               "{\n";
    for (size_t i = 0; i < ps_outputs_.size(); ++i)
    {
        const auto& param           = ps_outputs_[i];
        BYTE        component_count;
        if (BeDx12Utils::MaskToComponentCount(param.Mask, component_count))
        {
            std::string type;
            if (GetTypeName(param.ComponentType, param.MinPrecision, component_count, type))
            {
                stream_ << "    " << type << " attribute" << i << ": " << param.SemanticName
                        << param.SemanticIndex << ";\n";
            }
            else
            {
                rc = beKA::beStatus::kBeStatusDxcCannotAutoGenerateVertexShader;
                break;
            }
        }
    }

    if (rc == beKA::beStatus::kBeStatusSuccess)
    {
        stream_ << "};\n\n";

        stream_ << "struct PsOutput\n"
                   "{\n";
        for (size_t i = 0; i < render_target_formats_.size(); ++i)
        {
            if (render_target_formats_[i] != DXGI_FORMAT_UNKNOWN)
            {
                const char* const type = DxgiFormatToHlslType(render_target_formats_[i]);
                if (!type)
                {
                    rc = beKA::beStatus::kBeStatusDxcCannotAutoGenerateVertexShader;
                    err << "Error: unsupported render target type.\n";
                    break;
                }

                stream_ << "    " << type << " rt" << i << ": SV_Target" << i << ";\n";
            }
        }

        if (rc == beKA::beStatus::kBeStatusSuccess)
        {
            stream_ << "};\n\n";

            stream_ << "void ";
            stream_ << kPSEntryPoint;
            stream_ << "(PsInput input, out PsOutput output)\n"
                       "{\n"
                       "    float4 result = float4(0.0, 0.0, 0.0, 0.0);\n";
            for (size_t i = 0; i < ps_outputs_.size(); ++i)
            {
                const auto& param = ps_outputs_[i];
                BYTE        component_count;
                if (BeDx12Utils::MaskToComponentCount(param.Mask, component_count))
                {
                    switch (component_count)
                    {
                    case 1:
                        stream_ << "    result += float4(input.attribute" << i << ".xxxx);\n";
                        break;
                    case 2:
                        stream_ << "    result += float4(input.attribute" << i << ".xyyy);\n";
                        break;
                    case 3:
                        stream_ << "    result += float4(input.attribute" << i << ".xyzz);\n";
                        break;
                    case 4:
                        stream_ << "    result += float4(input.attribute" << i << ".xyzw);\n";
                        break;
                    default:
                        rc = beKA::beStatus::kBeStatusDxcCannotAutoGeneratePixelShader;
                        break;
                    }
                }
                else
                {
                     rc = beKA::beStatus::kBeStatusDxcCannotAutoGeneratePixelShader;
                    break;
                }
            }

            for (size_t i = 0; i < render_target_formats_.size(); ++i)
            {
                if (rc == beKA::beStatus::kBeStatusSuccess)
                {
                    const DXGI_FORMAT format = render_target_formats_[i];
                    if (format != DXGI_FORMAT_UNKNOWN)
                    {
                        const char* const type = DxgiFormatToHlslType(format);
                        if (!type)
                        {
                            rc = beKA::beStatus::kBeStatusDxcCannotAutoGeneratePixelShader;
                            err << "Error: unsupported input layout element type.\n";
                            break;
                        }

                        switch (DxgiFormatToComponentCount(format))
                        {
                        case 1:
                            stream_ << "    output.rt" << i << " = " << type << "(result.x);\n";
                            break;
                        case 2:
                            stream_ << "    output.rt" << i << " = " << type << "(result.xy);\n";
                            break;
                        case 3:
                            stream_ << "    output.rt" << i << " = " << type << "(result.xyz);\n";
                            break;
                        case 4:
                            stream_ << "    output.rt" << i << " = " << type << "(result.xyzw);\n";
                            break;
                        default:
                            rc = beKA::beStatus::kBeStatusDxcCannotAutoGeneratePixelShader;
                            err << "Error: unsupported input layout element format.\n";
                            break;
                        }
                    }
                    else
                    {
                        rc = beKA::beStatus::kBeStatusDxcCannotAutoGeneratePixelShader;
                        err << "Error: unsupported input layout element format.\n";
                        break;
                    }
                }
                
            }

            if (rc == beKA::beStatus::kBeStatusSuccess)
            {
                stream_ << "}\n";

                pixel_shader_string = stream_.str();
            }

        }

    }

    return rc;
}

beKA::beStatus BeDx12Reflection::GenerateVertexShader(const D3D12_GRAPHICS_PIPELINE_STATE_DESC* gpso,
                                                      const DxcReflectionOutput&                dxc_output,
                                                      std::string&                              vertex_shader,
                                                      std::stringstream&                        err) const
{

    ShaderSourceGeneratorVertex vs_gen;
    beKA::beStatus              rc =  vs_gen.SetPixelShaderInputs(dxc_output.param_desc_inputs.data(), dxc_output.param_desc_inputs.size());
    if (rc == beKA::beStatus::kBeStatusSuccess)
    {
        if (gpso)
        {
            vs_gen.SetInputLayoutElements(gpso->InputLayout.pInputElementDescs, gpso->InputLayout.NumElements);
        }
        rc = vs_gen.FinalizeVertexShaderSetup(vertex_shader, err);
    }
    return rc;
}


beKA::beStatus BeDx12Reflection::GeneratePixelShader(const D3D12_GRAPHICS_PIPELINE_STATE_DESC* gpso,
                                                     const DxcReflectionOutput&                dxc_output,
                                                     std::string&                              pixel_shader,
                                                     std::stringstream&                        err) const
{
    // Default render target count in generated GPSO is 1.
    const uint32_t render_target_count = gpso ? gpso->NumRenderTargets : 1;
    const bool     is_required_ps      = render_target_count > 0;
    beKA::beStatus rc                  = beKA::beStatus::kBeStatusSuccess;
    if (is_required_ps)
    {
        // Generate pixel shader.
        ShaderSourceGeneratorPixel ps_gen;
        rc = ps_gen.SetVertexShaderOutputs(dxc_output.param_desc_outputs.data(), dxc_output.param_desc_outputs.size());
        if (rc == beKA::beStatus::kBeStatusSuccess)
        {
            if (gpso)
            {
                ps_gen.SetRenderTargetFormats(gpso->RTVFormats, gpso->NumRenderTargets);
            }
            rc = ps_gen.FinalizePixelShaderSetup(pixel_shader, err);
        }
    }
    return rc;
}

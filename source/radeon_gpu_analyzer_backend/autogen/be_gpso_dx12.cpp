//=============================================================================
/// Copyright (c) 2024-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for dx12 gpso auto generation utilities.
//=============================================================================

// Dx12 Backend.
#include "utils/dx12/backend/rg_dx12_utils.h"

// Local.
#include "radeon_gpu_analyzer_backend/autogen/be_reflection_dx12.h"
#include "radeon_gpu_analyzer_backend/autogen/be_utils_dx12.h"

// CLI.
#include "radeon_gpu_analyzer_cli/kc_utils.h"

bool ParamDescToRenderTargetFormat(const D3D12_SIGNATURE_PARAMETER_DESC& param_desc, DXGI_FORMAT& dxgi_format)
{
    assert(param_desc.SystemValueType == D3D_NAME_TARGET);
    bool ret = false;

    switch (param_desc.ComponentType)
    {
    case D3D_REGISTER_COMPONENT_FLOAT32:
    {
        BYTE component_count;
        if (BeDx12Utils::MaskToComponentCount(param_desc.Mask, component_count))
        {
            switch (component_count)
            {
            case 1:
                dxgi_format = DXGI_FORMAT_R8_UNORM;
                ret         = true;

            case 2:
                dxgi_format = DXGI_FORMAT_R8G8_UNORM;
                ret         = true;

            case 3:
            case 4:
                dxgi_format = DXGI_FORMAT_R8G8B8A8_UNORM;
                ret         = true;
            }
        }
        break;
    }

    case D3D_REGISTER_COMPONENT_UINT32:
    {
        BYTE component_count;
        if (BeDx12Utils::MaskToComponentCount(param_desc.Mask, component_count))
        {
            switch (component_count)
            {
            case 1:
                dxgi_format = DXGI_FORMAT_R16_UINT;
                ret         = true;

            case 2:
                dxgi_format = DXGI_FORMAT_R16G16_UINT;
                ret         = true;

            case 3:
            case 4:
                dxgi_format = DXGI_FORMAT_R16G16B16A16_UINT;
                ret         = true;
            }
        }
        break;
    }

    case D3D_REGISTER_COMPONENT_SINT32:
    {
        BYTE component_count;
        if (BeDx12Utils::MaskToComponentCount(param_desc.Mask, component_count))
        {
            switch (component_count)
            {
            case 1:
                dxgi_format = DXGI_FORMAT_R16_SINT;
                ret         = true;

            case 2:
                dxgi_format = DXGI_FORMAT_R16G16_SINT;
                ret         = true;

            case 3:
            case 4:
                dxgi_format = DXGI_FORMAT_R16G16B16A16_SINT;
                ret         = true;
            }
        }
        break;
    }

    }

    if (ret == false)
    {
        dxgi_format = DXGI_FORMAT_UNKNOWN;
    }
    return ret;
}

const char* DxgiFormatToString(DXGI_FORMAT dxgi_format)
{
    switch (dxgi_format)
    {
        case DXGI_FORMAT_UNKNOWN : return "DXGI_FORMAT_UNKNOWN";
        case DXGI_FORMAT_R32G32B32A32_TYPELESS : return "DXGI_FORMAT_R32G32B32A32_TYPELESS";
        case DXGI_FORMAT_R32G32B32A32_FLOAT : return "DXGI_FORMAT_R32G32B32A32_FLOAT";
        case DXGI_FORMAT_R32G32B32A32_UINT : return "DXGI_FORMAT_R32G32B32A32_UINT";
        case DXGI_FORMAT_R32G32B32A32_SINT : return "DXGI_FORMAT_R32G32B32A32_SINT";
        case DXGI_FORMAT_R32G32B32_TYPELESS : return "DXGI_FORMAT_R32G32B32_TYPELESS";
        case DXGI_FORMAT_R32G32B32_FLOAT : return "DXGI_FORMAT_R32G32B32_FLOAT";
        case DXGI_FORMAT_R32G32B32_UINT : return "DXGI_FORMAT_R32G32B32_UINT";
        case DXGI_FORMAT_R32G32B32_SINT : return "DXGI_FORMAT_R32G32B32_SINT";
        case DXGI_FORMAT_R16G16B16A16_TYPELESS : return "DXGI_FORMAT_R16G16B16A16_TYPELESS";
        case DXGI_FORMAT_R16G16B16A16_FLOAT : return "DXGI_FORMAT_R16G16B16A16_FLOAT";
        case DXGI_FORMAT_R16G16B16A16_UNORM : return "DXGI_FORMAT_R16G16B16A16_UNORM";
        case DXGI_FORMAT_R16G16B16A16_UINT : return "DXGI_FORMAT_R16G16B16A16_UINT";
        case DXGI_FORMAT_R16G16B16A16_SNORM : return "DXGI_FORMAT_R16G16B16A16_SNORM";
        case DXGI_FORMAT_R16G16B16A16_SINT : return "DXGI_FORMAT_R16G16B16A16_SINT";
        case DXGI_FORMAT_R32G32_TYPELESS : return "DXGI_FORMAT_R32G32_TYPELESS";
        case DXGI_FORMAT_R32G32_FLOAT : return "DXGI_FORMAT_R32G32_FLOAT";
        case DXGI_FORMAT_R32G32_UINT : return "DXGI_FORMAT_R32G32_UINT";
        case DXGI_FORMAT_R32G32_SINT : return "DXGI_FORMAT_R32G32_SINT";
        case DXGI_FORMAT_R32G8X24_TYPELESS : return "DXGI_FORMAT_R32G8X24_TYPELESS";
        case DXGI_FORMAT_D32_FLOAT_S8X24_UINT : return "DXGI_FORMAT_D32_FLOAT_S8X24_UINT";
        case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS : return "DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS";
        case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT : return "DXGI_FORMAT_X32_TYPELESS_G8X24_UINT";
        case DXGI_FORMAT_R10G10B10A2_TYPELESS : return "DXGI_FORMAT_R10G10B10A2_TYPELESS";
        case DXGI_FORMAT_R10G10B10A2_UNORM : return "DXGI_FORMAT_R10G10B10A2_UNORM";
        case DXGI_FORMAT_R10G10B10A2_UINT : return "DXGI_FORMAT_R10G10B10A2_UINT";
        case DXGI_FORMAT_R11G11B10_FLOAT : return "DXGI_FORMAT_R11G11B10_FLOAT";
        case DXGI_FORMAT_R8G8B8A8_TYPELESS : return "DXGI_FORMAT_R8G8B8A8_TYPELESS";
        case DXGI_FORMAT_R8G8B8A8_UNORM : return "DXGI_FORMAT_R8G8B8A8_UNORM";
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : return "DXGI_FORMAT_R8G8B8A8_UNORM_SRGB";
        case DXGI_FORMAT_R8G8B8A8_UINT : return "DXGI_FORMAT_R8G8B8A8_UINT";
        case DXGI_FORMAT_R8G8B8A8_SNORM : return "DXGI_FORMAT_R8G8B8A8_SNORM";
        case DXGI_FORMAT_R8G8B8A8_SINT : return "DXGI_FORMAT_R8G8B8A8_SINT";
        case DXGI_FORMAT_R16G16_TYPELESS : return "DXGI_FORMAT_R16G16_TYPELESS";
        case DXGI_FORMAT_R16G16_FLOAT : return "DXGI_FORMAT_R16G16_FLOAT";
        case DXGI_FORMAT_R16G16_UNORM : return "DXGI_FORMAT_R16G16_UNORM";
        case DXGI_FORMAT_R16G16_UINT : return "DXGI_FORMAT_R16G16_UINT";
        case DXGI_FORMAT_R16G16_SNORM : return "DXGI_FORMAT_R16G16_SNORM";
        case DXGI_FORMAT_R16G16_SINT : return "DXGI_FORMAT_R16G16_SINT";
        case DXGI_FORMAT_R32_TYPELESS : return "DXGI_FORMAT_R32_TYPELESS";
        case DXGI_FORMAT_D32_FLOAT : return "DXGI_FORMAT_D32_FLOAT";
        case DXGI_FORMAT_R32_FLOAT : return "DXGI_FORMAT_R32_FLOAT";
        case DXGI_FORMAT_R32_UINT : return "DXGI_FORMAT_R32_UINT";
        case DXGI_FORMAT_R32_SINT : return "DXGI_FORMAT_R32_SINT";
        case DXGI_FORMAT_R24G8_TYPELESS : return "DXGI_FORMAT_R24G8_TYPELESS";
        case DXGI_FORMAT_D24_UNORM_S8_UINT : return "DXGI_FORMAT_D24_UNORM_S8_UINT";
        case DXGI_FORMAT_R24_UNORM_X8_TYPELESS : return "DXGI_FORMAT_R24_UNORM_X8_TYPELESS";
        case DXGI_FORMAT_X24_TYPELESS_G8_UINT : return "DXGI_FORMAT_X24_TYPELESS_G8_UINT";
        case DXGI_FORMAT_R8G8_TYPELESS : return "DXGI_FORMAT_R8G8_TYPELESS";
        case DXGI_FORMAT_R8G8_UNORM : return "DXGI_FORMAT_R8G8_UNORM";
        case DXGI_FORMAT_R8G8_UINT : return "DXGI_FORMAT_R8G8_UINT";
        case DXGI_FORMAT_R8G8_SNORM : return "DXGI_FORMAT_R8G8_SNORM";
        case DXGI_FORMAT_R8G8_SINT : return "DXGI_FORMAT_R8G8_SINT";
        case DXGI_FORMAT_R16_TYPELESS : return "DXGI_FORMAT_R16_TYPELESS";
        case DXGI_FORMAT_R16_FLOAT : return "DXGI_FORMAT_R16_FLOAT";
        case DXGI_FORMAT_D16_UNORM : return "DXGI_FORMAT_D16_UNORM";
        case DXGI_FORMAT_R16_UNORM : return "DXGI_FORMAT_R16_UNORM";
        case DXGI_FORMAT_R16_UINT : return "DXGI_FORMAT_R16_UINT";
        case DXGI_FORMAT_R16_SNORM : return "DXGI_FORMAT_R16_SNORM";
        case DXGI_FORMAT_R16_SINT : return "DXGI_FORMAT_R16_SINT";
        case DXGI_FORMAT_R8_TYPELESS : return "DXGI_FORMAT_R8_TYPELESS";
        case DXGI_FORMAT_R8_UNORM : return "DXGI_FORMAT_R8_UNORM";
        case DXGI_FORMAT_R8_UINT : return "DXGI_FORMAT_R8_UINT";
        case DXGI_FORMAT_R8_SNORM : return "DXGI_FORMAT_R8_SNORM";
        case DXGI_FORMAT_R8_SINT : return "DXGI_FORMAT_R8_SINT";
        case DXGI_FORMAT_A8_UNORM : return "DXGI_FORMAT_A8_UNORM";
        case DXGI_FORMAT_R1_UNORM : return "DXGI_FORMAT_R1_UNORM";
        case DXGI_FORMAT_R9G9B9E5_SHAREDEXP : return "DXGI_FORMAT_R9G9B9E5_SHAREDEXP";
        case DXGI_FORMAT_R8G8_B8G8_UNORM : return "DXGI_FORMAT_R8G8_B8G8_UNORM";
        case DXGI_FORMAT_G8R8_G8B8_UNORM : return "DXGI_FORMAT_G8R8_G8B8_UNORM";
        case DXGI_FORMAT_BC1_TYPELESS : return "DXGI_FORMAT_BC1_TYPELESS";
        case DXGI_FORMAT_BC1_UNORM : return "DXGI_FORMAT_BC1_UNORM";
        case DXGI_FORMAT_BC1_UNORM_SRGB : return "DXGI_FORMAT_BC1_UNORM_SRGB";
        case DXGI_FORMAT_BC2_TYPELESS : return "DXGI_FORMAT_BC2_TYPELESS";
        case DXGI_FORMAT_BC2_UNORM : return "DXGI_FORMAT_BC2_UNORM";
        case DXGI_FORMAT_BC2_UNORM_SRGB : return "DXGI_FORMAT_BC2_UNORM_SRGB";
        case DXGI_FORMAT_BC3_TYPELESS : return "DXGI_FORMAT_BC3_TYPELESS";
        case DXGI_FORMAT_BC3_UNORM : return "DXGI_FORMAT_BC3_UNORM";
        case DXGI_FORMAT_BC3_UNORM_SRGB : return "DXGI_FORMAT_BC3_UNORM_SRGB";
        case DXGI_FORMAT_BC4_TYPELESS : return "DXGI_FORMAT_BC4_TYPELESS";
        case DXGI_FORMAT_BC4_UNORM : return "DXGI_FORMAT_BC4_UNORM";
        case DXGI_FORMAT_BC4_SNORM : return "DXGI_FORMAT_BC4_SNORM";
        case DXGI_FORMAT_BC5_TYPELESS : return "DXGI_FORMAT_BC5_TYPELESS";
        case DXGI_FORMAT_BC5_UNORM : return "DXGI_FORMAT_BC5_UNORM";
        case DXGI_FORMAT_BC5_SNORM : return "DXGI_FORMAT_BC5_SNORM";
        case DXGI_FORMAT_B5G6R5_UNORM : return "DXGI_FORMAT_B5G6R5_UNORM";
        case DXGI_FORMAT_B5G5R5A1_UNORM : return "DXGI_FORMAT_B5G5R5A1_UNORM";
        case DXGI_FORMAT_B8G8R8A8_UNORM : return "DXGI_FORMAT_B8G8R8A8_UNORM";
        case DXGI_FORMAT_B8G8R8X8_UNORM : return "DXGI_FORMAT_B8G8R8X8_UNORM";
        case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM : return "DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM";
        case DXGI_FORMAT_B8G8R8A8_TYPELESS : return "DXGI_FORMAT_B8G8R8A8_TYPELESS";
        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB : return "DXGI_FORMAT_B8G8R8A8_UNORM_SRGB";
        case DXGI_FORMAT_B8G8R8X8_TYPELESS : return "DXGI_FORMAT_B8G8R8X8_TYPELESS";
        case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB : return "DXGI_FORMAT_B8G8R8X8_UNORM_SRGB";
        case DXGI_FORMAT_BC6H_TYPELESS : return "DXGI_FORMAT_BC6H_TYPELESS";
        case DXGI_FORMAT_BC6H_UF16 : return "DXGI_FORMAT_BC6H_UF16";
        case DXGI_FORMAT_BC6H_SF16 : return "DXGI_FORMAT_BC6H_SF16";
        case DXGI_FORMAT_BC7_TYPELESS : return "DXGI_FORMAT_BC7_TYPELESS";
        case DXGI_FORMAT_BC7_UNORM : return "DXGI_FORMAT_BC7_UNORM";
        case DXGI_FORMAT_BC7_UNORM_SRGB : return "DXGI_FORMAT_BC7_UNORM_SRGB";
        case DXGI_FORMAT_AYUV : return "DXGI_FORMAT_AYUV";
        case DXGI_FORMAT_Y410 : return "DXGI_FORMAT_Y410";
        case DXGI_FORMAT_Y416 : return "DXGI_FORMAT_Y416";
        case DXGI_FORMAT_NV12 : return "DXGI_FORMAT_NV12";
        case DXGI_FORMAT_P010 : return "DXGI_FORMAT_P010";
        case DXGI_FORMAT_P016 : return "DXGI_FORMAT_P016";
        case DXGI_FORMAT_420_OPAQUE : return "DXGI_FORMAT_420_OPAQUE";
        case DXGI_FORMAT_YUY2 : return "DXGI_FORMAT_YUY2";
        case DXGI_FORMAT_Y210 : return "DXGI_FORMAT_Y210";
        case DXGI_FORMAT_Y216 : return "DXGI_FORMAT_Y216";
        case DXGI_FORMAT_NV11 : return "DXGI_FORMAT_NV11";
        case DXGI_FORMAT_AI44 : return "DXGI_FORMAT_AI44";
        case DXGI_FORMAT_IA44 : return "DXGI_FORMAT_IA44";
        case DXGI_FORMAT_P8 : return "DXGI_FORMAT_P8";
        case DXGI_FORMAT_A8P8 : return "DXGI_FORMAT_A8P8";
        case DXGI_FORMAT_B4G4R4A4_UNORM : return "DXGI_FORMAT_B4G4R4A4_UNORM";
        case DXGI_FORMAT_P208 : return "DXGI_FORMAT_P208";
        case DXGI_FORMAT_V208 : return "DXGI_FORMAT_V208";
        case DXGI_FORMAT_V408 : return "DXGI_FORMAT_V408";
        case DXGI_FORMAT_SAMPLER_FEEDBACK_MIN_MIP_OPAQUE : return "DXGI_FORMAT_SAMPLER_FEEDBACK_MIN_MIP_OPAQUE";
        case DXGI_FORMAT_SAMPLER_FEEDBACK_MIP_REGION_USED_OPAQUE : return "DXGI_FORMAT_SAMPLER_FEEDBACK_MIP_REGION_USED_OPAQUE";
        default: return nullptr;
    }
}

struct GpsoGeneratorUtil
{
    std::vector<D3D12_INPUT_ELEMENT_DESC> input_layout_elements;
    D3D12_PRIMITIVE_TOPOLOGY_TYPE         primitive_topology_type = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    std::vector<DXGI_FORMAT>              render_target_formats;
    // Unused. If we ever add support for conservative rasterization in .gpso file format, this parameter is ready.
    bool conservative_rasterization = false;

    const char* AllocateStringMemory(const char* str)
    {
        strings_memory_.push_back(std::make_unique<std::string>(str));
        return strings_memory_.back()->c_str();
    }

private:
    std::vector<std::unique_ptr<std::string>> strings_memory_;
};

static const char* InputSlotClassToStr(D3D12_INPUT_CLASSIFICATION classification)
{
    switch (classification)
    {
    case D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA:
        return "D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA";
    case D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA:
        return "D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA";
    default:
        return nullptr;
    }
}

static bool WriteGpsoInputLayoutElement(std::stringstream& stream, const D3D12_INPUT_ELEMENT_DESC& input_element)
{
    bool        ret                = false;
    const char* dxgi_format_string = DxgiFormatToString(input_element.Format);
    const char* input_slot_string  = InputSlotClassToStr(input_element.InputSlotClass);
    if (dxgi_format_string != nullptr && input_slot_string != nullptr)
    {
        stream << "{ \"" << input_element.SemanticName << "\", " 
               << input_element.SemanticIndex << ", " 
               << dxgi_format_string << ", " 
               << input_element.InputSlot << ", " 
               << input_element.AlignedByteOffset << ", " 
               << input_slot_string << ", "
               << input_element.InstanceDataStepRate << " }\n";
        ret = true;
    }
    return ret;
}

static const char* PrimitiveTopologyToStr(D3D12_PRIMITIVE_TOPOLOGY_TYPE topology_type)
{
    switch (topology_type)
    {
        case D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT: return "D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT";
        case D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE: return "D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE";
        case D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE: return "D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE";
        case D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH: return "D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH";
        default: return nullptr;
    }
}

static beKA::beStatus WriteGpso(const GpsoGeneratorUtil& gpso_util, std::string& gpso_string, std::stringstream&)
{
    beKA::beStatus rc = beKA::beStatus::kBeStatusSuccess;

    std::stringstream stream;
    stream << "# " << kAutoGeneratedShaderHeader << "\n\n";

    stream << "# " << rga::RgDx12Utils::kStrElemSchemaVersion << "\n";
    stream << rga::RgDx12Utils::kStrElemSchemaVersion10 << "\n\n";

    stream << "# " << rga::RgDx12Utils::kStrElemSchemaInputLayoutNum
           << " (the number of D3D12_INPUT_ELEMENT_DESC elements in the D3D12_INPUT_LAYOUT_DESC structure - must match the following \""
           << rga::RgDx12Utils::kStrElemSchemaInputLayout << "\" section)\n";
    stream << gpso_util.input_layout_elements.size() << "\n\n";

    stream << "# " << rga::RgDx12Utils::kStrElemSchemaInputLayout
           << " ( {SemanticName, SemanticIndex, Format, InputSlot, AlignedByteOffset, InputSlotClass, InstanceDataStepRate } )\n ";


    for (const auto& e : gpso_util.input_layout_elements)
    {
        if (!WriteGpsoInputLayoutElement(stream, e))
        {
            rc = beKA::beStatus::kBeStatusDxcCannotAutoGenerateGpso;
            break;
        }
    }

    if (rc == beKA::beStatus::kBeStatusSuccess)
    {
        stream << "\n";

        stream << "# " << rga::RgDx12Utils::kStrElemSchemaPrimitiveTopologyType
               << " (the D3D12_PRIMITIVE_TOPOLOGY_TYPE value to be used when creating the PSO)\n";
        stream << PrimitiveTopologyToStr(gpso_util.primitive_topology_type) << "\n\n";

        stream << "# " << rga::RgDx12Utils::kStrElemSchemaNumRenderTargets << " (the number of formats in the upcoming RTVFormats section)\n";
        stream << gpso_util.render_target_formats.size() << "\n\n";

        stream << "# " << rga::RgDx12Utils::kStrElemSchemaRtvFormats
               << " (an array of DXGI_FORMAT-typed values for the render target formats - the number of items in the array should match the above "
               << rga::RgDx12Utils::kStrElemSchemaNumRenderTargets << " section)\n";

        stream << '{';

        for (size_t i = 0; i < gpso_util.render_target_formats.size(); ++i)
        {
            if (i > 0)
            {
                stream << ", ";
            }
            else
            {
                stream << " ";
            }
            const char* dxgi_format_string = DxgiFormatToString(gpso_util.render_target_formats[i]);
            if (dxgi_format_string != nullptr)
            {
                stream << dxgi_format_string;
            }
            else
            {
                rc = beKA::beStatus::kBeStatusDxcCannotAutoGenerateGpso;
                break;
            }
        }

        if (rc == beKA::beStatus::kBeStatusSuccess)
        {
            stream << " }\n";

            gpso_string = stream.str();
        }

    }

    return rc;
}

class GpsoGenerator
{
public:
    void SetConservativeRasterization(bool enabled)
    {
        gpso_util_.conservative_rasterization = enabled;
    }
    beKA::beStatus           SetVertexShaderInputs(const D3D12_SIGNATURE_PARAMETER_DESC* params, size_t count);
    void                     SetRenderTargets(const DXGI_FORMAT* formats, size_t count);
    const GpsoGeneratorUtil& GetGpsoUtil() const
    {
        return gpso_util_;
    }

private:
    static beKA::beStatus SignatureParameterToFormatAndSize(const D3D12_SIGNATURE_PARAMETER_DESC& param, std::pair<DXGI_FORMAT, size_t>& result);

    GpsoGeneratorUtil gpso_util_;
};

beKA::beStatus GpsoGenerator::SetVertexShaderInputs(const D3D12_SIGNATURE_PARAMETER_DESC* params, size_t count)
{
    beKA::beStatus rc             = beKA::beStatus::kBeStatusSuccess;

    uint32_t vertex_byte_offset   = 0;
    uint32_t instance_byte_offset = 0;

    for (size_t i = 0; i < count; ++i)
    {
        const auto& param = params[i];
        if (param.SystemValueType == D3D_NAME_UNDEFINED)
        {
            if (param.Stream != 0)
            {
                // We don't expect this to use streams.
                rc = beKA::beStatus::kBeStatusDxcCannotAutoGenerateGpso;
                break;
            }

            std::pair<DXGI_FORMAT, size_t> format_and_size;
            rc = SignatureParameterToFormatAndSize(param, format_and_size);
            if (rc != beKA::beStatus::kBeStatusSuccess)
            {
                rc = beKA::beStatus::kBeStatusDxcCannotAutoGenerateGpso;
                break;
            }

            D3D12_INPUT_ELEMENT_DESC element = {};
            element.SemanticName             = gpso_util_.AllocateStringMemory(param.SemanticName);
            element.SemanticIndex            = param.SemanticIndex;
            element.Format                   = format_and_size.first;
            element.InputSlot                = 0;
            element.InputSlotClass           = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
            element.InstanceDataStepRate     = 0;

            if (strstr(param.SemanticName, "INST") || strstr(param.SemanticName, "Inst") || strstr(param.SemanticName, "inst"))
            {
                element.InputSlot            = 1;
                element.InputSlotClass       = D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
                element.InstanceDataStepRate = 1;
                element.AlignedByteOffset    = instance_byte_offset;

                instance_byte_offset += static_cast<uint32_t>(format_and_size.second);
            }
            else
            {
                element.AlignedByteOffset = vertex_byte_offset;

                vertex_byte_offset += static_cast<uint32_t>(format_and_size.second);
            }

            gpso_util_.input_layout_elements.push_back(element);
        }
    }

    return rc;
}

void GpsoGenerator::SetRenderTargets(const DXGI_FORMAT* formats, size_t count)
{
    gpso_util_.render_target_formats.assign(formats, formats + count);
}

beKA::beStatus GpsoGenerator::SignatureParameterToFormatAndSize(const D3D12_SIGNATURE_PARAMETER_DESC& param, std::pair<DXGI_FORMAT, size_t>& result)
{
    beKA::beStatus rc = beKA::beStatus::kBeStatusSuccess;
    
    switch (param.ComponentType)
    {
    case D3D_REGISTER_COMPONENT_FLOAT32:
        if (param.MinPrecision == D3D_MIN_PRECISION_DEFAULT)
        {
            BYTE component_count;
            if (BeDx12Utils::MaskToComponentCount(param.Mask, component_count))
            {
                switch (component_count)
                {
                case 1:
                    result = std::make_pair(DXGI_FORMAT_R32_FLOAT, sizeof(float));
                    break;
                case 2:
                    result = std::make_pair(DXGI_FORMAT_R32G32_FLOAT, sizeof(float) * 2);
                    break;
                case 3:
                    result = std::make_pair(DXGI_FORMAT_R32G32B32_FLOAT, sizeof(float) * 3);
                    break;
                case 4:
                    result = std::make_pair(DXGI_FORMAT_R32G32B32A32_FLOAT, sizeof(float) * 4);
                    break;
                default:
                    rc = beKA::beStatus::kBeStatusDxcCannotAutoGenerateGpso;
                    break;
                }
            }
            else
            {
                rc = beKA::beStatus::kBeStatusDxcCannotAutoGenerateGpso;
                break;
            }
        }
        else  // Precision is 16 or 10 bit.
        {
            BYTE component_count;
            if (BeDx12Utils::MaskToComponentCount(param.Mask, component_count))
            {
                switch (component_count)
                {
                case 1:
                    result = std::make_pair(DXGI_FORMAT_R16_FLOAT, sizeof(float));
                    break;
                case 2:
                    result = std::make_pair(DXGI_FORMAT_R16G16_FLOAT, sizeof(float) * 2);
                    break;
                case 3:
                    // There are no 3-component 16-bit formats!
                    result = std::make_pair(DXGI_FORMAT_R32G32B32_FLOAT, sizeof(float) * 3);
                    break;
                case 4:
                    result = std::make_pair(DXGI_FORMAT_R16G16B16A16_FLOAT, sizeof(float) * 4);
                    break;
                default:
                    rc = beKA::beStatus::kBeStatusDxcCannotAutoGenerateGpso;
                    break;
                }
            }
            else
            {
                rc = beKA::beStatus::kBeStatusDxcCannotAutoGenerateGpso;
                break;
            }
        }
        break;

    case D3D_REGISTER_COMPONENT_UINT32:
        if (param.MinPrecision == D3D_MIN_PRECISION_DEFAULT)
        {
            BYTE component_count;
            if (BeDx12Utils::MaskToComponentCount(param.Mask, component_count))
            {
                switch (component_count)
                {
                case 1:
                    result = std::make_pair(DXGI_FORMAT_R32_UINT, sizeof(float));
                    break;
                case 2:
                    result = std::make_pair(DXGI_FORMAT_R32G32_UINT, sizeof(float) * 2);
                    break;
                case 3:
                    result = std::make_pair(DXGI_FORMAT_R32G32B32_UINT, sizeof(float) * 3);
                    break;
                case 4:
                    result = std::make_pair(DXGI_FORMAT_R32G32B32A32_UINT, sizeof(float) * 4);
                    break;
                default:
                    rc = beKA::beStatus::kBeStatusDxcCannotAutoGenerateGpso;
                    break;
                }
            }
            else
            {
                rc = beKA::beStatus::kBeStatusDxcCannotAutoGenerateGpso;
                break;
            }

            
        }
        else  // Precision is 16 or 10 bit.
        {
            BYTE component_count;
            if (BeDx12Utils::MaskToComponentCount(param.Mask, component_count))
            {
                switch (component_count)
                {                
                case 1:
                    result = std::make_pair(DXGI_FORMAT_R16_UINT, sizeof(float));
                    break;
                case 2:
                    result = std::make_pair(DXGI_FORMAT_R16G16_UINT, sizeof(float) * 2);
                    break;
                case 3:
                    // There are no 3-component 16-bit formats!
                    result = std::make_pair(DXGI_FORMAT_R32G32B32_UINT, sizeof(float) * 3);
                    break;
                case 4:
                    result = std::make_pair(DXGI_FORMAT_R16G16B16A16_UINT, sizeof(float) * 4);
                    break;
                default:
                    rc = beKA::beStatus::kBeStatusDxcCannotAutoGenerateGpso;
                    break;
                }
            }
            else
            {
                rc = beKA::beStatus::kBeStatusDxcCannotAutoGenerateGpso;
                break;
            }
        }
        break;

    case D3D_REGISTER_COMPONENT_SINT32:
        if (param.MinPrecision == D3D_MIN_PRECISION_DEFAULT)
        {
            BYTE component_count;
            if (BeDx12Utils::MaskToComponentCount(param.Mask, component_count))
            {
                switch (component_count)
                {
                case 1:
                    result = std::make_pair(DXGI_FORMAT_R32_SINT, sizeof(float));
                    break;
                case 2:
                    result = std::make_pair(DXGI_FORMAT_R32G32_SINT, sizeof(float) * 2);
                    break;
                case 3:
                    result = std::make_pair(DXGI_FORMAT_R32G32B32_SINT, sizeof(float) * 3);
                    break;
                case 4:
                    result = std::make_pair(DXGI_FORMAT_R32G32B32A32_SINT, sizeof(float) * 4);
                    break;
                default:
                    rc = beKA::beStatus::kBeStatusDxcCannotAutoGenerateGpso;
                    break;
                }
            }
            else
            {
                rc = beKA::beStatus::kBeStatusDxcCannotAutoGenerateGpso;
                break;
            }
        }
        else  // Precision is 16 or 10 bit.
        {
            BYTE component_count;
            if (BeDx12Utils::MaskToComponentCount(param.Mask, component_count))
            {
                switch (component_count)
                {
                case 1:
                    result = std::make_pair(DXGI_FORMAT_R16_SINT, sizeof(float));
                    break;
                case 2:
                    result = std::make_pair(DXGI_FORMAT_R16G16_SINT, sizeof(float) * 2);
                    break;
                case 3:
                    // There are no 3-component 16-bit formats!
                    result = std::make_pair(DXGI_FORMAT_R32G32B32_SINT, sizeof(float) * 3);
                    break;
                case 4:
                    result = std::make_pair(DXGI_FORMAT_R16G16B16A16_SINT, sizeof(float) * 4);
                    break;
                default:
                    rc = beKA::beStatus::kBeStatusDxcCannotAutoGenerateGpso;
                    break;
                }
            }
            else
            {
                rc = beKA::beStatus::kBeStatusDxcCannotAutoGenerateGpso;
                break;
            }
        }
        break;

    default:
        rc = beKA::beStatus::kBeStatusDxcCannotAutoGenerateGpso;
        break;
    }

    if (rc != beKA::beStatus::kBeStatusSuccess)
    {
        result = std::make_pair(DXGI_FORMAT_UNKNOWN, 0);

    }
    return rc;
}

beKA::beStatus BeDx12Reflection::GenerateGpso(UINT64                     shader_requires_flags,
                                              bool                       has_vs,
                                              const DxcReflectionOutput& vs_output,
                                              bool                       has_ps,
                                              const DxcReflectionOutput& ps_output,
                                              std::string&               gpso_text,
                                              std::stringstream&         err) const
{
    beKA::beStatus rc = beKA::beStatus::kBeStatusSuccess;
    GpsoGenerator gpso_genenerator;

    if ((shader_requires_flags & D3D_SHADER_REQUIRES_INNER_COVERAGE) != 0)
    {
        gpso_genenerator.SetConservativeRasterization(true);
    }

    if (has_vs)
    {
        rc = gpso_genenerator.SetVertexShaderInputs(vs_output.param_desc_inputs.data(), vs_output.param_desc_inputs.size());
    }
    else
    {
        // Hardcoded for autogenerated VS.
        D3D12_SIGNATURE_PARAMETER_DESC vs_input = {};
        vs_input.SemanticName                   = "POSITION";
        vs_input.SystemValueType                = D3D_NAME_UNDEFINED;
        vs_input.ComponentType                  = D3D_REGISTER_COMPONENT_FLOAT32;
        vs_input.Mask                           = 0xF;
        rc = gpso_genenerator.SetVertexShaderInputs(&vs_input, 1);
    }

    if (has_ps)
    {
        std::vector<DXGI_FORMAT> render_target_formats;
        for (const auto& param_desc : ps_output.param_desc_outputs)
        {
            if (param_desc.SystemValueType == D3D_NAME_TARGET)
            {
                // Render targets can be sparse, e.g. only RT1, RT3, with no RT0 and RT2 - support this.
                assert(param_desc.SemanticIndex >= render_target_formats.size());
                const size_t unused_render_targets_to_add = param_desc.SemanticIndex - render_target_formats.size();
                for (size_t i = 0; i < unused_render_targets_to_add; ++i)
                {
                    render_target_formats.push_back(DXGI_FORMAT_UNKNOWN);
                }

                DXGI_FORMAT dxgi_format;
                if (ParamDescToRenderTargetFormat(param_desc, dxgi_format))
                {
                    render_target_formats.push_back(dxgi_format);
                }
                else
                {
                    rc = beKA::beStatus::kBeStatusDxcCannotAutoGenerateGpso;
                }
            }
        }
        gpso_genenerator.SetRenderTargets(render_target_formats.data(), render_target_formats.size());
    }
    else
    {
        // Hardcoded for autogenerated PS.
        const DXGI_FORMAT rt_formats[] = {DXGI_FORMAT_R8G8B8A8_UNORM};
        gpso_genenerator.SetRenderTargets(rt_formats, 1);
    }

    if (rc == beKA::beStatus::kBeStatusSuccess)
    {
        rc = WriteGpso(gpso_genenerator.GetGpsoUtil(), gpso_text, err);
    }

    return rc;
}

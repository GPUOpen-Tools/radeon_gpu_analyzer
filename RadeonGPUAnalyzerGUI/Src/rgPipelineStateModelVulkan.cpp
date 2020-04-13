// C++.
#include <cassert>
#include <sstream>

// Infra.
#include <QtCommon/Scaling/ScalingManager.h>

// Utils.
#include <Utils/Vulkan/Include/rgPsoFactoryVulkan.h>
#include <Utils/Vulkan/Include/rgPsoSerializerVulkan.h>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgPipelineStateModelVulkan.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgEditorElement.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgEditorElementArray.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgEditorElementBool.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgEditorElementEnum.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgEditorElementNumeric.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDataTypes.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDataTypesVulkan.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDefinitions.h>
#include <RadeonGPUAnalyzerGUI/Include/rgStringConstants.h>
#include <RadeonGPUAnalyzerGUI/Include/rgUtils.h>

static const rgEnumValuesVector& GetFormatEnumerators()
{
    static rgEnumValuesVector formatValues = {
        ENUM_VALUE(VK_FORMAT_UNDEFINED),
        ENUM_VALUE(VK_FORMAT_R4G4_UNORM_PACK8),
        ENUM_VALUE(VK_FORMAT_R4G4B4A4_UNORM_PACK16),
        ENUM_VALUE(VK_FORMAT_B4G4R4A4_UNORM_PACK16),
        ENUM_VALUE(VK_FORMAT_R5G6B5_UNORM_PACK16),
        ENUM_VALUE(VK_FORMAT_B5G6R5_UNORM_PACK16),
        ENUM_VALUE(VK_FORMAT_R5G5B5A1_UNORM_PACK16),
        ENUM_VALUE(VK_FORMAT_B5G5R5A1_UNORM_PACK16),
        ENUM_VALUE(VK_FORMAT_A1R5G5B5_UNORM_PACK16),
        ENUM_VALUE(VK_FORMAT_R8_UNORM),
        ENUM_VALUE(VK_FORMAT_R8_SNORM),
        ENUM_VALUE(VK_FORMAT_R8_USCALED),
        ENUM_VALUE(VK_FORMAT_R8_SSCALED),
        ENUM_VALUE(VK_FORMAT_R8_UINT),
        ENUM_VALUE(VK_FORMAT_R8_SINT),
        ENUM_VALUE(VK_FORMAT_R8_SRGB),
        ENUM_VALUE(VK_FORMAT_R8G8_UNORM),
        ENUM_VALUE(VK_FORMAT_R8G8_SNORM),
        ENUM_VALUE(VK_FORMAT_R8G8_USCALED),
        ENUM_VALUE(VK_FORMAT_R8G8_SSCALED),
        ENUM_VALUE(VK_FORMAT_R8G8_UINT),
        ENUM_VALUE(VK_FORMAT_R8G8_SINT),
        ENUM_VALUE(VK_FORMAT_R8G8_SRGB),
        ENUM_VALUE(VK_FORMAT_R8G8B8_UNORM),
        ENUM_VALUE(VK_FORMAT_R8G8B8_SNORM),
        ENUM_VALUE(VK_FORMAT_R8G8B8_USCALED),
        ENUM_VALUE(VK_FORMAT_R8G8B8_SSCALED),
        ENUM_VALUE(VK_FORMAT_R8G8B8_UINT),
        ENUM_VALUE(VK_FORMAT_R8G8B8_SINT),
        ENUM_VALUE(VK_FORMAT_R8G8B8_SRGB),
        ENUM_VALUE(VK_FORMAT_B8G8R8_UNORM),
        ENUM_VALUE(VK_FORMAT_B8G8R8_SNORM),
        ENUM_VALUE(VK_FORMAT_B8G8R8_USCALED),
        ENUM_VALUE(VK_FORMAT_B8G8R8_SSCALED),
        ENUM_VALUE(VK_FORMAT_B8G8R8_UINT),
        ENUM_VALUE(VK_FORMAT_B8G8R8_SINT),
        ENUM_VALUE(VK_FORMAT_B8G8R8_SRGB),
        ENUM_VALUE(VK_FORMAT_R8G8B8A8_UNORM),
        ENUM_VALUE(VK_FORMAT_R8G8B8A8_SNORM),
        ENUM_VALUE(VK_FORMAT_R8G8B8A8_USCALED),
        ENUM_VALUE(VK_FORMAT_R8G8B8A8_SSCALED),
        ENUM_VALUE(VK_FORMAT_R8G8B8A8_UINT),
        ENUM_VALUE(VK_FORMAT_R8G8B8A8_SINT),
        ENUM_VALUE(VK_FORMAT_R8G8B8A8_SRGB),
        ENUM_VALUE(VK_FORMAT_B8G8R8A8_UNORM),
        ENUM_VALUE(VK_FORMAT_B8G8R8A8_SNORM),
        ENUM_VALUE(VK_FORMAT_B8G8R8A8_USCALED),
        ENUM_VALUE(VK_FORMAT_B8G8R8A8_SSCALED),
        ENUM_VALUE(VK_FORMAT_B8G8R8A8_UINT),
        ENUM_VALUE(VK_FORMAT_B8G8R8A8_SINT),
        ENUM_VALUE(VK_FORMAT_B8G8R8A8_SRGB),
        ENUM_VALUE(VK_FORMAT_A8B8G8R8_UNORM_PACK32),
        ENUM_VALUE(VK_FORMAT_A8B8G8R8_SNORM_PACK32),
        ENUM_VALUE(VK_FORMAT_A8B8G8R8_USCALED_PACK32),
        ENUM_VALUE(VK_FORMAT_A8B8G8R8_SSCALED_PACK32),
        ENUM_VALUE(VK_FORMAT_A8B8G8R8_UINT_PACK32),
        ENUM_VALUE(VK_FORMAT_A8B8G8R8_SINT_PACK32),
        ENUM_VALUE(VK_FORMAT_A8B8G8R8_SRGB_PACK32),
        ENUM_VALUE(VK_FORMAT_A2R10G10B10_UNORM_PACK32),
        ENUM_VALUE(VK_FORMAT_A2R10G10B10_SNORM_PACK32),
        ENUM_VALUE(VK_FORMAT_A2R10G10B10_USCALED_PACK32),
        ENUM_VALUE(VK_FORMAT_A2R10G10B10_SSCALED_PACK32),
        ENUM_VALUE(VK_FORMAT_A2R10G10B10_UINT_PACK32),
        ENUM_VALUE(VK_FORMAT_A2R10G10B10_SINT_PACK32),
        ENUM_VALUE(VK_FORMAT_A2B10G10R10_UNORM_PACK32),
        ENUM_VALUE(VK_FORMAT_A2B10G10R10_SNORM_PACK32),
        ENUM_VALUE(VK_FORMAT_A2B10G10R10_USCALED_PACK32),
        ENUM_VALUE(VK_FORMAT_A2B10G10R10_SSCALED_PACK32),
        ENUM_VALUE(VK_FORMAT_A2B10G10R10_UINT_PACK32),
        ENUM_VALUE(VK_FORMAT_A2B10G10R10_SINT_PACK32),
        ENUM_VALUE(VK_FORMAT_R16_UNORM),
        ENUM_VALUE(VK_FORMAT_R16_SNORM),
        ENUM_VALUE(VK_FORMAT_R16_USCALED),
        ENUM_VALUE(VK_FORMAT_R16_SSCALED),
        ENUM_VALUE(VK_FORMAT_R16_UINT),
        ENUM_VALUE(VK_FORMAT_R16_SINT),
        ENUM_VALUE(VK_FORMAT_R16_SFLOAT),
        ENUM_VALUE(VK_FORMAT_R16G16_UNORM),
        ENUM_VALUE(VK_FORMAT_R16G16_SNORM),
        ENUM_VALUE(VK_FORMAT_R16G16_USCALED),
        ENUM_VALUE(VK_FORMAT_R16G16_SSCALED),
        ENUM_VALUE(VK_FORMAT_R16G16_UINT),
        ENUM_VALUE(VK_FORMAT_R16G16_SINT),
        ENUM_VALUE(VK_FORMAT_R16G16_SFLOAT),
        ENUM_VALUE(VK_FORMAT_R16G16B16_UNORM),
        ENUM_VALUE(VK_FORMAT_R16G16B16_SNORM),
        ENUM_VALUE(VK_FORMAT_R16G16B16_USCALED),
        ENUM_VALUE(VK_FORMAT_R16G16B16_SSCALED),
        ENUM_VALUE(VK_FORMAT_R16G16B16_UINT),
        ENUM_VALUE(VK_FORMAT_R16G16B16_SINT),
        ENUM_VALUE(VK_FORMAT_R16G16B16_SFLOAT),
        ENUM_VALUE(VK_FORMAT_R16G16B16A16_UNORM),
        ENUM_VALUE(VK_FORMAT_R16G16B16A16_SNORM),
        ENUM_VALUE(VK_FORMAT_R16G16B16A16_USCALED),
        ENUM_VALUE(VK_FORMAT_R16G16B16A16_SSCALED),
        ENUM_VALUE(VK_FORMAT_R16G16B16A16_UINT),
        ENUM_VALUE(VK_FORMAT_R16G16B16A16_SINT),
        ENUM_VALUE(VK_FORMAT_R16G16B16A16_SFLOAT),
        ENUM_VALUE(VK_FORMAT_R32_UINT),
        ENUM_VALUE(VK_FORMAT_R32_SINT),
        ENUM_VALUE(VK_FORMAT_R32_SFLOAT),
        ENUM_VALUE(VK_FORMAT_R32G32_UINT),
        ENUM_VALUE(VK_FORMAT_R32G32_SINT),
        ENUM_VALUE(VK_FORMAT_R32G32_SFLOAT),
        ENUM_VALUE(VK_FORMAT_R32G32B32_UINT),
        ENUM_VALUE(VK_FORMAT_R32G32B32_SINT),
        ENUM_VALUE(VK_FORMAT_R32G32B32_SFLOAT),
        ENUM_VALUE(VK_FORMAT_R32G32B32A32_UINT),
        ENUM_VALUE(VK_FORMAT_R32G32B32A32_SINT),
        ENUM_VALUE(VK_FORMAT_R32G32B32A32_SFLOAT),
        ENUM_VALUE(VK_FORMAT_R64_UINT),
        ENUM_VALUE(VK_FORMAT_R64_SINT),
        ENUM_VALUE(VK_FORMAT_R64_SFLOAT),
        ENUM_VALUE(VK_FORMAT_R64G64_UINT),
        ENUM_VALUE(VK_FORMAT_R64G64_SINT),
        ENUM_VALUE(VK_FORMAT_R64G64_SFLOAT),
        ENUM_VALUE(VK_FORMAT_R64G64B64_UINT),
        ENUM_VALUE(VK_FORMAT_R64G64B64_SINT),
        ENUM_VALUE(VK_FORMAT_R64G64B64_SFLOAT),
        ENUM_VALUE(VK_FORMAT_R64G64B64A64_UINT),
        ENUM_VALUE(VK_FORMAT_R64G64B64A64_SINT),
        ENUM_VALUE(VK_FORMAT_R64G64B64A64_SFLOAT),
        ENUM_VALUE(VK_FORMAT_B10G11R11_UFLOAT_PACK32),
        ENUM_VALUE(VK_FORMAT_E5B9G9R9_UFLOAT_PACK32),
        ENUM_VALUE(VK_FORMAT_D16_UNORM),
        ENUM_VALUE(VK_FORMAT_X8_D24_UNORM_PACK32),
        ENUM_VALUE(VK_FORMAT_D32_SFLOAT),
        ENUM_VALUE(VK_FORMAT_S8_UINT),
        ENUM_VALUE(VK_FORMAT_D16_UNORM_S8_UINT),
        ENUM_VALUE(VK_FORMAT_D24_UNORM_S8_UINT),
        ENUM_VALUE(VK_FORMAT_D32_SFLOAT_S8_UINT),
        ENUM_VALUE(VK_FORMAT_BC1_RGB_UNORM_BLOCK),
        ENUM_VALUE(VK_FORMAT_BC1_RGB_SRGB_BLOCK),
        ENUM_VALUE(VK_FORMAT_BC1_RGBA_UNORM_BLOCK),
        ENUM_VALUE(VK_FORMAT_BC1_RGBA_SRGB_BLOCK),
        ENUM_VALUE(VK_FORMAT_BC2_UNORM_BLOCK),
        ENUM_VALUE(VK_FORMAT_BC2_SRGB_BLOCK),
        ENUM_VALUE(VK_FORMAT_BC3_UNORM_BLOCK),
        ENUM_VALUE(VK_FORMAT_BC3_SRGB_BLOCK),
        ENUM_VALUE(VK_FORMAT_BC4_UNORM_BLOCK),
        ENUM_VALUE(VK_FORMAT_BC4_SNORM_BLOCK),
        ENUM_VALUE(VK_FORMAT_BC5_UNORM_BLOCK),
        ENUM_VALUE(VK_FORMAT_BC5_SNORM_BLOCK),
        ENUM_VALUE(VK_FORMAT_BC6H_UFLOAT_BLOCK),
        ENUM_VALUE(VK_FORMAT_BC6H_SFLOAT_BLOCK),
        ENUM_VALUE(VK_FORMAT_BC7_UNORM_BLOCK),
        ENUM_VALUE(VK_FORMAT_BC7_SRGB_BLOCK),
        ENUM_VALUE(VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK),
        ENUM_VALUE(VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK),
        ENUM_VALUE(VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK),
        ENUM_VALUE(VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK),
        ENUM_VALUE(VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK),
        ENUM_VALUE(VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK),
        ENUM_VALUE(VK_FORMAT_EAC_R11_UNORM_BLOCK),
        ENUM_VALUE(VK_FORMAT_EAC_R11_SNORM_BLOCK),
        ENUM_VALUE(VK_FORMAT_EAC_R11G11_UNORM_BLOCK),
        ENUM_VALUE(VK_FORMAT_EAC_R11G11_SNORM_BLOCK),
        ENUM_VALUE(VK_FORMAT_ASTC_4x4_UNORM_BLOCK),
        ENUM_VALUE(VK_FORMAT_ASTC_4x4_SRGB_BLOCK),
        ENUM_VALUE(VK_FORMAT_ASTC_5x4_UNORM_BLOCK),
        ENUM_VALUE(VK_FORMAT_ASTC_5x4_SRGB_BLOCK),
        ENUM_VALUE(VK_FORMAT_ASTC_5x5_UNORM_BLOCK),
        ENUM_VALUE(VK_FORMAT_ASTC_5x5_SRGB_BLOCK),
        ENUM_VALUE(VK_FORMAT_ASTC_6x5_UNORM_BLOCK),
        ENUM_VALUE(VK_FORMAT_ASTC_6x5_SRGB_BLOCK),
        ENUM_VALUE(VK_FORMAT_ASTC_6x6_UNORM_BLOCK),
        ENUM_VALUE(VK_FORMAT_ASTC_6x6_SRGB_BLOCK),
        ENUM_VALUE(VK_FORMAT_ASTC_8x5_UNORM_BLOCK),
        ENUM_VALUE(VK_FORMAT_ASTC_8x5_SRGB_BLOCK),
        ENUM_VALUE(VK_FORMAT_ASTC_8x6_UNORM_BLOCK),
        ENUM_VALUE(VK_FORMAT_ASTC_8x6_SRGB_BLOCK),
        ENUM_VALUE(VK_FORMAT_ASTC_8x8_UNORM_BLOCK),
        ENUM_VALUE(VK_FORMAT_ASTC_8x8_SRGB_BLOCK),
        ENUM_VALUE(VK_FORMAT_ASTC_10x5_UNORM_BLOCK),
        ENUM_VALUE(VK_FORMAT_ASTC_10x5_SRGB_BLOCK),
        ENUM_VALUE(VK_FORMAT_ASTC_10x6_UNORM_BLOCK),
        ENUM_VALUE(VK_FORMAT_ASTC_10x6_SRGB_BLOCK),
        ENUM_VALUE(VK_FORMAT_ASTC_10x8_UNORM_BLOCK),
        ENUM_VALUE(VK_FORMAT_ASTC_10x8_SRGB_BLOCK),
        ENUM_VALUE(VK_FORMAT_ASTC_10x10_UNORM_BLOCK),
        ENUM_VALUE(VK_FORMAT_ASTC_10x10_SRGB_BLOCK),
        ENUM_VALUE(VK_FORMAT_ASTC_12x10_UNORM_BLOCK),
        ENUM_VALUE(VK_FORMAT_ASTC_12x10_SRGB_BLOCK),
        ENUM_VALUE(VK_FORMAT_ASTC_12x12_UNORM_BLOCK),
        ENUM_VALUE(VK_FORMAT_ASTC_12x12_SRGB_BLOCK),
        ENUM_VALUE(VK_FORMAT_G8B8G8R8_422_UNORM),
        ENUM_VALUE(VK_FORMAT_B8G8R8G8_422_UNORM),
        ENUM_VALUE(VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM),
        ENUM_VALUE(VK_FORMAT_G8_B8R8_2PLANE_420_UNORM),
        ENUM_VALUE(VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM),
        ENUM_VALUE(VK_FORMAT_G8_B8R8_2PLANE_422_UNORM),
        ENUM_VALUE(VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM),
        ENUM_VALUE(VK_FORMAT_R10X6_UNORM_PACK16),
        ENUM_VALUE(VK_FORMAT_R10X6G10X6_UNORM_2PACK16),
        ENUM_VALUE(VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16),
        ENUM_VALUE(VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16),
        ENUM_VALUE(VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16),
        ENUM_VALUE(VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16),
        ENUM_VALUE(VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16),
        ENUM_VALUE(VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16),
        ENUM_VALUE(VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16),
        ENUM_VALUE(VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16),
        ENUM_VALUE(VK_FORMAT_R12X4_UNORM_PACK16),
        ENUM_VALUE(VK_FORMAT_R12X4G12X4_UNORM_2PACK16),
        ENUM_VALUE(VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16),
        ENUM_VALUE(VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16),
        ENUM_VALUE(VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16),
        ENUM_VALUE(VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16),
        ENUM_VALUE(VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16),
        ENUM_VALUE(VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16),
        ENUM_VALUE(VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16),
        ENUM_VALUE(VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16),
        ENUM_VALUE(VK_FORMAT_G16B16G16R16_422_UNORM),
        ENUM_VALUE(VK_FORMAT_B16G16R16G16_422_UNORM),
        ENUM_VALUE(VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM),
        ENUM_VALUE(VK_FORMAT_G16_B16R16_2PLANE_420_UNORM),
        ENUM_VALUE(VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM),
        ENUM_VALUE(VK_FORMAT_G16_B16R16_2PLANE_422_UNORM),
        ENUM_VALUE(VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM),
        ENUM_VALUE(VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG),
        ENUM_VALUE(VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG),
        ENUM_VALUE(VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG),
        ENUM_VALUE(VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG),
        ENUM_VALUE(VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG),
        ENUM_VALUE(VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG),
        ENUM_VALUE(VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG),
        ENUM_VALUE(VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG),

        // NOTE: All declarations below use values repeated from the declarations above.
        ENUM_VALUE(VK_FORMAT_G8B8G8R8_422_UNORM_KHR),
        ENUM_VALUE(VK_FORMAT_B8G8R8G8_422_UNORM_KHR),
        ENUM_VALUE(VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM_KHR),
        ENUM_VALUE(VK_FORMAT_G8_B8R8_2PLANE_420_UNORM_KHR),
        ENUM_VALUE(VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM_KHR),
        ENUM_VALUE(VK_FORMAT_G8_B8R8_2PLANE_422_UNORM_KHR),
        ENUM_VALUE(VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM_KHR),
        ENUM_VALUE(VK_FORMAT_R10X6_UNORM_PACK16_KHR),
        ENUM_VALUE(VK_FORMAT_R10X6G10X6_UNORM_2PACK16_KHR),
        ENUM_VALUE(VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16_KHR),
        ENUM_VALUE(VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16_KHR),
        ENUM_VALUE(VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16_KHR),
        ENUM_VALUE(VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16_KHR),
        ENUM_VALUE(VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16_KHR),
        ENUM_VALUE(VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16_KHR),
        ENUM_VALUE(VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16_KHR),
        ENUM_VALUE(VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16_KHR),
        ENUM_VALUE(VK_FORMAT_R12X4_UNORM_PACK16_KHR),
        ENUM_VALUE(VK_FORMAT_R12X4G12X4_UNORM_2PACK16_KHR),
        ENUM_VALUE(VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16_KHR),
        ENUM_VALUE(VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16_KHR),
        ENUM_VALUE(VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16_KHR),
        ENUM_VALUE(VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16_KHR),
        ENUM_VALUE(VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16_KHR),
        ENUM_VALUE(VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16_KHR),
        ENUM_VALUE(VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16_KHR),
        ENUM_VALUE(VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16_KHR),
        ENUM_VALUE(VK_FORMAT_G16B16G16R16_422_UNORM_KHR),
        ENUM_VALUE(VK_FORMAT_B16G16R16G16_422_UNORM_KHR),
        ENUM_VALUE(VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM_KHR),
        ENUM_VALUE(VK_FORMAT_G16_B16R16_2PLANE_420_UNORM_KHR),
        ENUM_VALUE(VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM_KHR),
        ENUM_VALUE(VK_FORMAT_G16_B16R16_2PLANE_422_UNORM_KHR),
        ENUM_VALUE(VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM_KHR),
    };

    return formatValues;
}

static const rgEnumValuesVector& GetPipelineCreateFlagEnumerators()
{
    static rgEnumValuesVector pipelineCreateFlagEnumerators = {
        ENUM_VALUE(VK_PIPELINE_CREATE_DISABLE_OPTIMIZATION_BIT),
        ENUM_VALUE(VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT),
        ENUM_VALUE(VK_PIPELINE_CREATE_DERIVATIVE_BIT),
        ENUM_VALUE(VK_PIPELINE_CREATE_VIEW_INDEX_FROM_DEVICE_INDEX_BIT),
        ENUM_VALUE(VK_PIPELINE_CREATE_DISPATCH_BASE),
        ENUM_VALUE(VK_PIPELINE_CREATE_DEFER_COMPILE_BIT_NV),
        ENUM_VALUE(VK_PIPELINE_CREATE_VIEW_INDEX_FROM_DEVICE_INDEX_BIT_KHR),
        ENUM_VALUE(VK_PIPELINE_CREATE_DISPATCH_BASE_KHR),
    };

    return pipelineCreateFlagEnumerators;
}

static const rgEnumValuesVector& GetDescriptorSetLayoutCreateFlagEnumerators()
{
    static rgEnumValuesVector descriptorSetLayoutCreateFlagEnumerators = {
        ENUM_VALUE(VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR),
        ENUM_VALUE(VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT)
    };

    return descriptorSetLayoutCreateFlagEnumerators;
}

static const rgEnumValuesVector& GetAttachmentDescriptionFlagEnumerators()
{
    static rgEnumValuesVector attachmentDescriptionFlagEnumerators = {
        ENUM_VALUE(VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT),
    };

    return attachmentDescriptionFlagEnumerators;
}

static const rgEnumValuesVector& GetPrimitiveTopologyEnumerators()
{
    static rgEnumValuesVector primitiveTopologyEnumerators = {
        ENUM_VALUE(VK_PRIMITIVE_TOPOLOGY_POINT_LIST),
        ENUM_VALUE(VK_PRIMITIVE_TOPOLOGY_LINE_LIST),
        ENUM_VALUE(VK_PRIMITIVE_TOPOLOGY_LINE_STRIP),
        ENUM_VALUE(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST),
        ENUM_VALUE(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP),
        ENUM_VALUE(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN),
        ENUM_VALUE(VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY),
        ENUM_VALUE(VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY),
        ENUM_VALUE(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY),
        ENUM_VALUE(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY),
        ENUM_VALUE(VK_PRIMITIVE_TOPOLOGY_PATCH_LIST)
    };

    return primitiveTopologyEnumerators;
}

static const rgEnumValuesVector& GetAttachmentLoadOpEnumerators()
{
    static rgEnumValuesVector loadOpEnumerators = {
        ENUM_VALUE(VK_ATTACHMENT_LOAD_OP_LOAD),
        ENUM_VALUE(VK_ATTACHMENT_LOAD_OP_CLEAR),
        ENUM_VALUE(VK_ATTACHMENT_LOAD_OP_DONT_CARE)
    };

    return loadOpEnumerators;
}

static const rgEnumValuesVector& GetAttachmentStoreOpEnumerators()
{
    static rgEnumValuesVector storeOpEnumerators = {
        ENUM_VALUE(VK_ATTACHMENT_STORE_OP_STORE),
        ENUM_VALUE(VK_ATTACHMENT_STORE_OP_DONT_CARE),
    };

    return storeOpEnumerators;
}

static const rgEnumValuesVector& GetImageLayoutEnumerators()
{
    static rgEnumValuesVector imageLayoutEnumerators = {
        ENUM_VALUE(VK_IMAGE_LAYOUT_UNDEFINED),
        ENUM_VALUE(VK_IMAGE_LAYOUT_GENERAL),
        ENUM_VALUE(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL),
        ENUM_VALUE(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL),
        ENUM_VALUE(VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL),
        ENUM_VALUE(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL),
        ENUM_VALUE(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL),
        ENUM_VALUE(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL),
        ENUM_VALUE(VK_IMAGE_LAYOUT_PREINITIALIZED),
        ENUM_VALUE(VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL),
        ENUM_VALUE(VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL),
        ENUM_VALUE(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR),
        ENUM_VALUE(VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR),
        ENUM_VALUE(VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL_KHR),
        ENUM_VALUE(VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL_KHR),
    };

    return imageLayoutEnumerators;
}

static const rgEnumValuesVector& GetPolygonModeEnumerators()
{
    static rgEnumValuesVector polygonModeEnumerators = {
        ENUM_VALUE(VK_POLYGON_MODE_FILL),
        ENUM_VALUE(VK_POLYGON_MODE_LINE),
        ENUM_VALUE(VK_POLYGON_MODE_POINT),
        ENUM_VALUE(VK_POLYGON_MODE_FILL_RECTANGLE_NV),
    };

    return polygonModeEnumerators;
}

static const rgEnumValuesVector& GetCullModeFlagEnumerators()
{
    static rgEnumValuesVector cullModeFlagEnumerators = {
        ENUM_VALUE(VK_CULL_MODE_NONE),
        ENUM_VALUE(VK_CULL_MODE_FRONT_BIT),
        ENUM_VALUE(VK_CULL_MODE_BACK_BIT),
        ENUM_VALUE(VK_CULL_MODE_FRONT_AND_BACK),
    };

    return cullModeFlagEnumerators;
}

static const rgEnumValuesVector& GetFrontFaceEnumerators()
{
    static rgEnumValuesVector frontFaceEnumerators = {
        ENUM_VALUE(VK_FRONT_FACE_COUNTER_CLOCKWISE),
        ENUM_VALUE(VK_FRONT_FACE_CLOCKWISE),
    };

    return frontFaceEnumerators;
}

static const rgEnumValuesVector& GetRasterizationSamplesEnumerators()
{
    static rgEnumValuesVector rasterizationSamplesFlags = {
        ENUM_VALUE(VK_SAMPLE_COUNT_1_BIT),
        ENUM_VALUE(VK_SAMPLE_COUNT_2_BIT),
        ENUM_VALUE(VK_SAMPLE_COUNT_4_BIT),
        ENUM_VALUE(VK_SAMPLE_COUNT_8_BIT),
        ENUM_VALUE(VK_SAMPLE_COUNT_16_BIT),
        ENUM_VALUE(VK_SAMPLE_COUNT_32_BIT),
        ENUM_VALUE(VK_SAMPLE_COUNT_64_BIT),
    };

    return rasterizationSamplesFlags;
}

static const rgEnumValuesVector& GetCompareOpEnumerators()
{
    static rgEnumValuesVector compareOpEnumerators = {
        ENUM_VALUE(VK_COMPARE_OP_NEVER),
        ENUM_VALUE(VK_COMPARE_OP_LESS),
        ENUM_VALUE(VK_COMPARE_OP_EQUAL),
        ENUM_VALUE(VK_COMPARE_OP_LESS_OR_EQUAL),
        ENUM_VALUE(VK_COMPARE_OP_GREATER),
        ENUM_VALUE(VK_COMPARE_OP_NOT_EQUAL),
        ENUM_VALUE(VK_COMPARE_OP_GREATER_OR_EQUAL),
        ENUM_VALUE(VK_COMPARE_OP_ALWAYS),
    };

    return compareOpEnumerators;
}

static const rgEnumValuesVector& GetStencilOpEnumerators()
{
    static rgEnumValuesVector stencilOpEnumerators = {
        ENUM_VALUE(VK_STENCIL_OP_KEEP),
        ENUM_VALUE(VK_STENCIL_OP_ZERO),
        ENUM_VALUE(VK_STENCIL_OP_REPLACE),
        ENUM_VALUE(VK_STENCIL_OP_INCREMENT_AND_CLAMP),
        ENUM_VALUE(VK_STENCIL_OP_DECREMENT_AND_CLAMP),
        ENUM_VALUE(VK_STENCIL_OP_INVERT),
        ENUM_VALUE(VK_STENCIL_OP_INCREMENT_AND_WRAP),
        ENUM_VALUE(VK_STENCIL_OP_DECREMENT_AND_WRAP),
    };

    return stencilOpEnumerators;
}

static const rgEnumValuesVector& GetLogicOpEnumerators()
{
    static rgEnumValuesVector logicOpEnumerators = {
        ENUM_VALUE(VK_LOGIC_OP_CLEAR),
        ENUM_VALUE(VK_LOGIC_OP_AND),
        ENUM_VALUE(VK_LOGIC_OP_AND_REVERSE),
        ENUM_VALUE(VK_LOGIC_OP_COPY),
        ENUM_VALUE(VK_LOGIC_OP_AND_INVERTED),
        ENUM_VALUE(VK_LOGIC_OP_NO_OP),
        ENUM_VALUE(VK_LOGIC_OP_XOR),
        ENUM_VALUE(VK_LOGIC_OP_OR),
        ENUM_VALUE(VK_LOGIC_OP_NOR),
        ENUM_VALUE(VK_LOGIC_OP_EQUIVALENT),
        ENUM_VALUE(VK_LOGIC_OP_INVERT),
        ENUM_VALUE(VK_LOGIC_OP_OR_REVERSE),
        ENUM_VALUE(VK_LOGIC_OP_COPY_INVERTED),
        ENUM_VALUE(VK_LOGIC_OP_OR_INVERTED),
        ENUM_VALUE(VK_LOGIC_OP_NAND),
        ENUM_VALUE(VK_LOGIC_OP_SET),
    };

    return logicOpEnumerators;
}

static const rgEnumValuesVector& GetBlendFactorEnumerators()
{
    static rgEnumValuesVector blendFactorEnumerators = {
        ENUM_VALUE(VK_BLEND_FACTOR_ZERO),
        ENUM_VALUE(VK_BLEND_FACTOR_ONE),
        ENUM_VALUE(VK_BLEND_FACTOR_SRC_COLOR),
        ENUM_VALUE(VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR),
        ENUM_VALUE(VK_BLEND_FACTOR_DST_COLOR),
        ENUM_VALUE(VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR),
        ENUM_VALUE(VK_BLEND_FACTOR_SRC_ALPHA),
        ENUM_VALUE(VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA),
        ENUM_VALUE(VK_BLEND_FACTOR_DST_ALPHA),
        ENUM_VALUE(VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA),
        ENUM_VALUE(VK_BLEND_FACTOR_CONSTANT_COLOR),
        ENUM_VALUE(VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR),
        ENUM_VALUE(VK_BLEND_FACTOR_CONSTANT_ALPHA),
        ENUM_VALUE(VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA),
        ENUM_VALUE(VK_BLEND_FACTOR_SRC_ALPHA_SATURATE),
        ENUM_VALUE(VK_BLEND_FACTOR_SRC1_COLOR),
        ENUM_VALUE(VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR),
        ENUM_VALUE(VK_BLEND_FACTOR_SRC1_ALPHA),
        ENUM_VALUE(VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA),
    };

    return blendFactorEnumerators;
}

static const rgEnumValuesVector& GetBlendOpEnumerators()
{
    static rgEnumValuesVector blendOpEnumerators = {
        ENUM_VALUE(VK_BLEND_OP_ADD),
        ENUM_VALUE(VK_BLEND_OP_SUBTRACT),
        ENUM_VALUE(VK_BLEND_OP_REVERSE_SUBTRACT),
        ENUM_VALUE(VK_BLEND_OP_MIN),
        ENUM_VALUE(VK_BLEND_OP_MAX),
        ENUM_VALUE(VK_BLEND_OP_ZERO_EXT),
        ENUM_VALUE(VK_BLEND_OP_SRC_EXT),
        ENUM_VALUE(VK_BLEND_OP_DST_EXT),
        ENUM_VALUE(VK_BLEND_OP_SRC_OVER_EXT),
        ENUM_VALUE(VK_BLEND_OP_DST_OVER_EXT),
        ENUM_VALUE(VK_BLEND_OP_SRC_IN_EXT),
        ENUM_VALUE(VK_BLEND_OP_DST_IN_EXT),
        ENUM_VALUE(VK_BLEND_OP_SRC_OUT_EXT),
        ENUM_VALUE(VK_BLEND_OP_DST_OUT_EXT),
        ENUM_VALUE(VK_BLEND_OP_SRC_ATOP_EXT),
        ENUM_VALUE(VK_BLEND_OP_DST_ATOP_EXT),
        ENUM_VALUE(VK_BLEND_OP_XOR_EXT),
        ENUM_VALUE(VK_BLEND_OP_MULTIPLY_EXT),
        ENUM_VALUE(VK_BLEND_OP_SCREEN_EXT),
        ENUM_VALUE(VK_BLEND_OP_OVERLAY_EXT),
        ENUM_VALUE(VK_BLEND_OP_DARKEN_EXT),
        ENUM_VALUE(VK_BLEND_OP_LIGHTEN_EXT),
        ENUM_VALUE(VK_BLEND_OP_COLORDODGE_EXT),
        ENUM_VALUE(VK_BLEND_OP_COLORBURN_EXT),
        ENUM_VALUE(VK_BLEND_OP_HARDLIGHT_EXT),
        ENUM_VALUE(VK_BLEND_OP_SOFTLIGHT_EXT),
        ENUM_VALUE(VK_BLEND_OP_DIFFERENCE_EXT),
        ENUM_VALUE(VK_BLEND_OP_EXCLUSION_EXT),
        ENUM_VALUE(VK_BLEND_OP_INVERT_EXT),
        ENUM_VALUE(VK_BLEND_OP_INVERT_RGB_EXT),
        ENUM_VALUE(VK_BLEND_OP_LINEARDODGE_EXT),
        ENUM_VALUE(VK_BLEND_OP_LINEARBURN_EXT),
        ENUM_VALUE(VK_BLEND_OP_VIVIDLIGHT_EXT),
        ENUM_VALUE(VK_BLEND_OP_LINEARLIGHT_EXT),
        ENUM_VALUE(VK_BLEND_OP_PINLIGHT_EXT),
        ENUM_VALUE(VK_BLEND_OP_HARDMIX_EXT),
        ENUM_VALUE(VK_BLEND_OP_HSL_HUE_EXT),
        ENUM_VALUE(VK_BLEND_OP_HSL_SATURATION_EXT),
        ENUM_VALUE(VK_BLEND_OP_HSL_COLOR_EXT),
        ENUM_VALUE(VK_BLEND_OP_HSL_LUMINOSITY_EXT),
        ENUM_VALUE(VK_BLEND_OP_PLUS_EXT),
        ENUM_VALUE(VK_BLEND_OP_PLUS_CLAMPED_EXT),
        ENUM_VALUE(VK_BLEND_OP_PLUS_CLAMPED_ALPHA_EXT),
        ENUM_VALUE(VK_BLEND_OP_PLUS_DARKER_EXT),
        ENUM_VALUE(VK_BLEND_OP_MINUS_EXT),
        ENUM_VALUE(VK_BLEND_OP_MINUS_CLAMPED_EXT),
        ENUM_VALUE(VK_BLEND_OP_CONTRAST_EXT),
        ENUM_VALUE(VK_BLEND_OP_INVERT_OVG_EXT),
        ENUM_VALUE(VK_BLEND_OP_RED_EXT),
        ENUM_VALUE(VK_BLEND_OP_GREEN_EXT),
        ENUM_VALUE(VK_BLEND_OP_BLUE_EXT),
    };

    return blendOpEnumerators;
}

static const rgEnumValuesVector& GetColorComponentFlagEnumerators()
{
    static rgEnumValuesVector colorComponentFlagEnumerators = {
        ENUM_VALUE(VK_COLOR_COMPONENT_R_BIT),
        ENUM_VALUE(VK_COLOR_COMPONENT_G_BIT),
        ENUM_VALUE(VK_COLOR_COMPONENT_B_BIT),
        ENUM_VALUE(VK_COLOR_COMPONENT_A_BIT),
    };

    return colorComponentFlagEnumerators;
}

static const rgEnumValuesVector& GetShaderStageFlagEnumerators()
{
    static rgEnumValuesVector stageFlagEnumerators = {
        ENUM_VALUE(VK_SHADER_STAGE_VERTEX_BIT),
        ENUM_VALUE(VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT),
        ENUM_VALUE(VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT),
        ENUM_VALUE(VK_SHADER_STAGE_GEOMETRY_BIT),
        ENUM_VALUE(VK_SHADER_STAGE_FRAGMENT_BIT),
        ENUM_VALUE(VK_SHADER_STAGE_COMPUTE_BIT),
        ENUM_VALUE(VK_SHADER_STAGE_ALL_GRAPHICS),
    };

    return stageFlagEnumerators;
}

static const rgEnumValuesVector& GetDescriptorTypeEnumerators()
{
    static rgEnumValuesVector descriptorTypeEnumerators = {
        ENUM_VALUE(VK_DESCRIPTOR_TYPE_SAMPLER),
        ENUM_VALUE(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER),
        ENUM_VALUE(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE),
        ENUM_VALUE(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE),
        ENUM_VALUE(VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER),
        ENUM_VALUE(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER),
        ENUM_VALUE(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER),
        ENUM_VALUE(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER),
        ENUM_VALUE(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC),
        ENUM_VALUE(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC),
        ENUM_VALUE(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT),
    };

    return descriptorTypeEnumerators;
}

static const rgEnumValuesVector& GetPipelineStageFlagEnumerators()
{
    static rgEnumValuesVector pipelineStageFlagEnumerators = {
        ENUM_VALUE(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT),
        ENUM_VALUE(VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT),
        ENUM_VALUE(VK_PIPELINE_STAGE_VERTEX_INPUT_BIT),
        ENUM_VALUE(VK_PIPELINE_STAGE_VERTEX_SHADER_BIT),
        ENUM_VALUE(VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT),
        ENUM_VALUE(VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT),
        ENUM_VALUE(VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT),
        ENUM_VALUE(VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT),
        ENUM_VALUE(VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT),
        ENUM_VALUE(VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT),
        ENUM_VALUE(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT),
        ENUM_VALUE(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT),
        ENUM_VALUE(VK_PIPELINE_STAGE_TRANSFER_BIT),
        ENUM_VALUE(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT),
        ENUM_VALUE(VK_PIPELINE_STAGE_HOST_BIT),
        ENUM_VALUE(VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT),
        ENUM_VALUE(VK_PIPELINE_STAGE_ALL_COMMANDS_BIT),
        ENUM_VALUE(VK_PIPELINE_STAGE_COMMAND_PROCESS_BIT_NVX),
    };

    return pipelineStageFlagEnumerators;
}

static const rgEnumValuesVector& GetAccessFlagEnumerators()
{
    static rgEnumValuesVector accessFlagEnumerators = {
        ENUM_VALUE(VK_ACCESS_INDIRECT_COMMAND_READ_BIT),
        ENUM_VALUE(VK_ACCESS_INDEX_READ_BIT),
        ENUM_VALUE(VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT),
        ENUM_VALUE(VK_ACCESS_UNIFORM_READ_BIT),
        ENUM_VALUE(VK_ACCESS_INPUT_ATTACHMENT_READ_BIT),
        ENUM_VALUE(VK_ACCESS_SHADER_READ_BIT),
        ENUM_VALUE(VK_ACCESS_SHADER_WRITE_BIT),
        ENUM_VALUE(VK_ACCESS_COLOR_ATTACHMENT_READ_BIT),
        ENUM_VALUE(VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT),
        ENUM_VALUE(VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT),
        ENUM_VALUE(VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT),
        ENUM_VALUE(VK_ACCESS_TRANSFER_READ_BIT),
        ENUM_VALUE(VK_ACCESS_TRANSFER_WRITE_BIT),
        ENUM_VALUE(VK_ACCESS_HOST_READ_BIT),
        ENUM_VALUE(VK_ACCESS_HOST_WRITE_BIT),
        ENUM_VALUE(VK_ACCESS_MEMORY_READ_BIT),
        ENUM_VALUE(VK_ACCESS_MEMORY_WRITE_BIT),
        ENUM_VALUE(VK_ACCESS_COMMAND_PROCESS_READ_BIT_NVX),
        ENUM_VALUE(VK_ACCESS_COMMAND_PROCESS_WRITE_BIT_NVX),
        ENUM_VALUE(VK_ACCESS_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT_EXT),
    };

    return accessFlagEnumerators;
}

static const rgEnumValuesVector& GetDependencyFlagEnumerators()
{
    static rgEnumValuesVector dependencyFlagEnumerators = {
        ENUM_VALUE(VK_DEPENDENCY_BY_REGION_BIT),
        ENUM_VALUE(VK_DEPENDENCY_DEVICE_GROUP_BIT),
        ENUM_VALUE(VK_DEPENDENCY_VIEW_LOCAL_BIT),
        ENUM_VALUE(VK_DEPENDENCY_VIEW_LOCAL_BIT_KHR),
        ENUM_VALUE(VK_DEPENDENCY_DEVICE_GROUP_BIT_KHR),
    };

    return dependencyFlagEnumerators;
}

static const rgEnumValuesVector& GetSubpassDescriptionFlagEnumerators()
{
    static rgEnumValuesVector subpassDescriptionFlagEnumerators = {
        ENUM_VALUE(VK_SUBPASS_DESCRIPTION_PER_VIEW_ATTRIBUTES_BIT_NVX),
        ENUM_VALUE(VK_SUBPASS_DESCRIPTION_PER_VIEW_POSITION_X_ONLY_BIT_NVX),
    };

    return subpassDescriptionFlagEnumerators;
}

static const rgEnumValuesVector& GetPipelineBindPointEnumerators()
{
    static rgEnumValuesVector pipelineBindPointEnumerators = {
        ENUM_VALUE(VK_PIPELINE_BIND_POINT_GRAPHICS),
        ENUM_VALUE(VK_PIPELINE_BIND_POINT_COMPUTE),
    };

    return pipelineBindPointEnumerators;
}

rgPipelineStateModelVulkan::rgPipelineStateModelVulkan(QWidget* pParent)
    : rgPipelineStateModel(pParent)
    , m_descriptorSetLayoutCount(0)
{
}

uint32_t GetSampleMaskDimension(VkSampleCountFlagBits sampleCountBits)
{
    uint32_t enumDimension = 0;
    switch (sampleCountBits)
    {
    case VK_SAMPLE_COUNT_1_BIT:
    case VK_SAMPLE_COUNT_2_BIT:
    case VK_SAMPLE_COUNT_4_BIT:
    case VK_SAMPLE_COUNT_8_BIT:
    case VK_SAMPLE_COUNT_16_BIT:
    case VK_SAMPLE_COUNT_32_BIT:
        // One 32-bit uint_32 is used to hold 1 to 32 bit flags.
        enumDimension = 1;
        break;
    case VK_SAMPLE_COUNT_64_BIT:
        // Two 32-bit uint_32's are used to hold the 64 bit flags.
        enumDimension = 2;
        break;
    default:
        assert(false);
    }

    return enumDimension;
}

bool rgPipelineStateModelVulkan::CheckValidPipelineState(std::string& errorString) const
{
    bool ret = false;

    std::stringstream errorStream;

    // Choose how to validate the PSO state based on the pipeline type.
    if (m_pipelineType == rgPipelineType::Graphics)
    {
        // Verify that if the number of resolve attachments is non-null, the dimension of
        // pResolveAttachments matches that of pColorAttachments.
        VkRenderPassCreateInfo* pRenderPassCreateInfo = m_pGraphicsPipelineState->GetRenderPassCreateInfo();
        assert(pRenderPassCreateInfo != nullptr);
        if (pRenderPassCreateInfo != nullptr)
        {
            bool isResolveAttachmentsCompatible = true;

            // Step through each subpass.
            for (uint32_t subpassIndex = 0; subpassIndex < pRenderPassCreateInfo->subpassCount; ++subpassIndex)
            {
                assert(pRenderPassCreateInfo->pSubpasses != nullptr);
                if (pRenderPassCreateInfo->pSubpasses != nullptr)
                {
                    // Step to the nth item in the subpass array.
                    const VkSubpassDescription* pSubpass = (pRenderPassCreateInfo->pSubpasses + subpassIndex);
                    assert(pSubpass != nullptr);
                    if (pSubpass != nullptr)
                    {
                        // Is the pResolveAttachments array used? If so, verify the dimension matches the
                        // number of color attachments.
                        if (pSubpass->pResolveAttachments != nullptr)
                        {
                            uint32_t* pSubpassResolveAttachmentCount = m_resolveAttachmentCountPerSubpass.at(subpassIndex);
                            if (pSubpassResolveAttachmentCount != nullptr)
                            {
                                // Does the number of resolve attachments match the number of color attachments?
                                assert(pSubpass->colorAttachmentCount == *pSubpassResolveAttachmentCount);
                                if (pSubpass->colorAttachmentCount != *pSubpassResolveAttachmentCount)
                                {
                                    // Build an error string that mentions the index of the problematic subpass.
                                    errorStream << STR_ERROR_RESOLVE_ATTACHMENTS_INVALID_A;
                                    errorStream << subpassIndex;
                                    errorStream << STR_ERROR_RESOLVE_ATTACHMENTS_INVALID_B;
                                    errorStream << " ";
                                    errorStream << STR_ERROR_RESOLVE_ATTACHMENTS_INVALID_C;

                                    // Return false, and exit out of the loop. Validation failed.
                                    isResolveAttachmentsCompatible = false;
                                    break;
                                }
                            }
                        }
                    }
                }
            }

            ret = isResolveAttachmentsCompatible;
        }

        VkPipelineMultisampleStateCreateInfo* pMultisamplingStateCreateInfo =
            m_pGraphicsPipelineState->GetPipelineMultisampleStateCreateInfo();
        assert(pMultisamplingStateCreateInfo != nullptr);
        if (pMultisamplingStateCreateInfo != nullptr)
        {
            bool isMultisamplingSampleMaskValid = false;

            // If the multisampling state's pSampleMask is enabled, verify that the dimension
            // of the array is compatible with the rasterizationSamples state.
            if (pMultisamplingStateCreateInfo->pSampleMask != nullptr)
            {
                uint32_t enumDimension = GetSampleMaskDimension(pMultisamplingStateCreateInfo->rasterizationSamples);

                // If the pSampleMask is used, the dimension should be non-zero.
                assert(enumDimension != 0);
                if (enumDimension != 0)
                {
                    // The pSampleMask array is valid if the rasterizationSamples enum
                    // matches the dimension that the user has set.
                    isMultisamplingSampleMaskValid = enumDimension == m_sampleMaskDimension;

                    if (!isMultisamplingSampleMaskValid)
                    {
                        // Construct an error message indicating why the pipeline is valid.
                        errorStream << STR_ERROR_MULTISAMPLING_SAMPLE_MASK_INVALID_A;
                        errorStream << std::endl;
                        errorStream << STR_ERROR_MULTISAMPLING_SAMPLE_MASK_INVALID_B;
                    }
                }
            }
            else
            {
                // Multisampling state doesn't need to be verified further if pSampleMask is null.
                isMultisamplingSampleMaskValid = true;
            }

            // Update the return value based on the mask validity.
            ret = ret && isMultisamplingSampleMaskValid;
        }
    }
    else if (m_pipelineType == rgPipelineType::Compute)
    {
        // Nothing needs to be validated for a compute pipeline. Just return true each time.
        ret = true;
    }
    else
    {
        // The pipeline type couldn't be determined. This shouldn't happen.
        assert(false && "Failed to validate pipeline because the pipeline type could not be determined.");
    }

    errorString = errorStream.str();

    return ret;
}

void rgPipelineStateModelVulkan::InitializeDefaultGraphicsPipeline()
{
    std::shared_ptr<rgPsoFactoryVulkan> pPipelineFactory = std::make_shared<rgPsoFactoryVulkan>();

    rgPsoGraphicsVulkan* pGraphicsPsoState = pPipelineFactory->GetDefaultGraphicsPsoCreateInfo();
    assert(pGraphicsPsoState != nullptr);
    if (pGraphicsPsoState != nullptr)
    {
        // Configure the pipeline's default blend state to be compatible with the default shader code.
        VkPipelineColorBlendAttachmentState* pColorBlendAttachment = new VkPipelineColorBlendAttachmentState{};
        pColorBlendAttachment->colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        pColorBlendAttachment->blendEnable = VK_FALSE;
        pColorBlendAttachment->srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        pColorBlendAttachment->dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        pColorBlendAttachment->colorBlendOp = VK_BLEND_OP_ADD;
        pColorBlendAttachment->srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        pColorBlendAttachment->dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        pColorBlendAttachment->alphaBlendOp = VK_BLEND_OP_ADD;

        // Add a default color blend create info state.
        VkPipelineColorBlendStateCreateInfo* pColorBlendStateInfo = pGraphicsPsoState->GetPipelineColorBlendStateCreateInfo();
        pColorBlendStateInfo->pAttachments = pColorBlendAttachment;
        pColorBlendStateInfo->attachmentCount = 1;

        // Assign the default state in the model.
        m_pGraphicsPipelineState = pGraphicsPsoState;
    }
}

void rgPipelineStateModelVulkan::InitializeDefaultComputePipeline()
{
    std::shared_ptr<rgPsoFactoryVulkan> pPipelineFactory = std::make_shared<rgPsoFactoryVulkan>();

    // Configure a compute pipeline state with default configuration.
    rgPsoComputeVulkan* pComputePsoState = pPipelineFactory->GetDefaultComputePsoCreateInfo();
    assert(pComputePsoState != nullptr);
    if (pComputePsoState != nullptr)
    {
        // Assign the default state in the model.
        m_pComputePipelineState = pComputePsoState;
    }
}

void rgPipelineStateModelVulkan::InitializeDescriptorSetLayoutCreateInfoArray(rgEditorElement* pRootElement, rgPsoCreateInfoVulkan* pCreateInfo)
{
    // Create the root element for the Descriptor Set Layout array configuration.
    rgEditorElement* pDescriptorSetLayoutsRoot = new rgEditorElement(pRootElement, STR_VULKAN_DESCRIPTOR_SET_LAYOUTS_HEADER);

    // Display the type name with brackets to indicate that it's an array.
    std::stringstream descriptorSetLayoutsArrayStream;
    descriptorSetLayoutsArrayStream << STR_VULKAN_DESCRIPTOR_SET_LAYOUT_CREATE_INFO << "[]";

    std::vector<VkDescriptorSetLayoutCreateInfo*>& descriptorSetLayouts = pCreateInfo->GetDescriptorSetLayoutCreateInfo();

    // Create the Descriptor Set Layout array root item.
    rgEditorElementArray* pDescriptorSetLayoutsItem = new rgEditorElementArray(pDescriptorSetLayoutsRoot, descriptorSetLayoutsArrayStream.str().c_str(),
        [=](int elementIndex)
    {
        std::vector<VkDescriptorSetLayoutCreateInfo*>& currentDescriptorSetLayouts = pCreateInfo->GetDescriptorSetLayoutCreateInfo();
        int elementCount = static_cast<int>(currentDescriptorSetLayouts.size());

        // If we remove the given element, is it necessary to shift other subsequent elements?
        if (elementCount > 1 && elementIndex < elementCount - 1)
        {
            // Shift all elements after the removed element.
            for (int copyIndex = elementIndex; copyIndex < (elementCount - 1); ++copyIndex)
            {
                memcpy(*(currentDescriptorSetLayouts.data() + copyIndex), *(currentDescriptorSetLayouts.data() + (copyIndex + 1)), sizeof(VkDescriptorSetLayoutCreateInfo));
            }
        }
    });

    // Initialize the Descriptor Set Layout count.
    m_descriptorSetLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());

    // Create the Descriptor Set Layout count item.
    rgEditorElement* pDescriptorSetLayoutCountItem = MakeNumericElement(nullptr, STR_VULKAN_DESCRIPTOR_SET_LAYOUT_COUNT,
        &m_descriptorSetLayoutCount, [=] { HandleDescriptorSetLayoutCountChanged(pDescriptorSetLayoutsItem, pCreateInfo); });

    // Provide the element used to track the dimension of the Descriptor Set Layouts array.
    pDescriptorSetLayoutsItem->SetArraySizeElement(static_cast<rgEditorElementNumeric<uint32_t>*>(pDescriptorSetLayoutCountItem));

    // Initialize the Descriptor Set Layouts array rows.
    HandleDescriptorSetLayoutCountChanged(pDescriptorSetLayoutsItem, pCreateInfo, true);

    // Add the Descriptor Set Layouts create info array item.
    pDescriptorSetLayoutsRoot->AppendChildItem(pDescriptorSetLayoutsItem);

    // Add the Descriptor Set Layout section.
    pRootElement->AppendChildItem(pDescriptorSetLayoutsRoot);
}

rgEditorElement* rgPipelineStateModelVulkan::InitializeGraphicsPipelineCreateInfo(QWidget* pParent)
{
    rgEditorElement* pCreateInfoRootNode = nullptr;

    assert(m_pGraphicsPipelineState != nullptr);
    if (m_pGraphicsPipelineState != nullptr)
    {
        // Create the root node that all create info elements are attached to.
        pCreateInfoRootNode = new rgEditorElement(pParent, STR_VULKAN_GRAPHICS_PIPELINE_STATE);

        // Add the Graphics Pipeline create info root node.
        InitializeVkGraphicsPipelineCreateInfo(pCreateInfoRootNode, m_pGraphicsPipelineState);

        // Add the Pipeline Layout create info root node.
        VkPipelineLayoutCreateInfo* pPipelineLayoutCreateInfo = m_pGraphicsPipelineState->GetPipelineLayoutCreateInfo();
        assert(pPipelineLayoutCreateInfo != nullptr);
        if (pPipelineLayoutCreateInfo != nullptr)
        {
            InitializePipelineLayoutCreateInfo(pCreateInfoRootNode, pPipelineLayoutCreateInfo);
        }

        // Initialize rows related to Descriptor Set Layout configuration.
        InitializeDescriptorSetLayoutCreateInfoArray(pCreateInfoRootNode, m_pGraphicsPipelineState);

        // Add the Render Pass create info root node.
        VkRenderPassCreateInfo* pRenderPassCreateInfo = m_pGraphicsPipelineState->GetRenderPassCreateInfo();
        assert(pRenderPassCreateInfo != nullptr);
        if (pRenderPassCreateInfo != nullptr)
        {
            InitializeRenderPassCreateInfo(pCreateInfoRootNode, pRenderPassCreateInfo);
        }
    }

    return pCreateInfoRootNode;
}

void rgPipelineStateModelVulkan::HandleDescriptorSetLayoutCountChanged(rgEditorElement* pRootElement, rgPsoCreateInfoVulkan* pCreateInfo, bool firstInit)
{
    int numExistingElements = pRootElement->ChildCount();
    int newElementCount = static_cast<int32_t>(m_descriptorSetLayoutCount);

    if (newElementCount != numExistingElements)
    {
        if (firstInit)
        {
            numExistingElements = newElementCount;
        }

        std::vector<VkDescriptorSetLayoutCreateInfo*>& descriptorSetLayouts = pCreateInfo->GetDescriptorSetLayoutCreateInfo();

        rgEditorElementArray* pArrayRoot = static_cast<rgEditorElementArray*>(pRootElement);
        if (pArrayRoot != nullptr)
        {
            if (newElementCount == 0)
            {
                for (size_t elementIndex = 0; elementIndex < descriptorSetLayouts.size(); ++elementIndex)
                {
                    RG_SAFE_DELETE(descriptorSetLayouts[elementIndex]);
                }

                pRootElement->ClearChildren();
                descriptorSetLayouts.clear();

                // Let the array root element know that the child elements were resized.
                pArrayRoot->InvokeElementResizedCallback();
            }
            else
            {
                std::vector<VkDescriptorSetLayoutCreateInfo*> newDescriptorSetLayouts;
                for (size_t elementIndex = 0; elementIndex < newElementCount; ++elementIndex)
                {
                    // Create an array of object with the new dimension.
                    VkDescriptorSetLayoutCreateInfo* pNewElement = new VkDescriptorSetLayoutCreateInfo{};
                    newDescriptorSetLayouts.push_back(pNewElement);
                }

                // If the element count was increased, copy all old contents.
                // If it was reduced, copy as many as will fit in the new array.
                uint32_t elementCount = newElementCount > numExistingElements ? numExistingElements : newElementCount;

                // Copy existing element data into the resized elements.
                for (uint32_t index = 0; index < elementCount; ++index)
                {
                    // Copy all member data into the new array elements.
                    memcpy(newDescriptorSetLayouts[index], descriptorSetLayouts[index], sizeof(VkDescriptorSetLayoutCreateInfo));
                }

                // Destroy the original array element data.
                for (size_t elementIndex = 0; elementIndex < descriptorSetLayouts.size(); ++elementIndex)
                {
                    RG_SAFE_DELETE(descriptorSetLayouts[elementIndex]);
                }

                // Resize the number of Descriptor Set Layouts.
                descriptorSetLayouts.clear();

                // Remove all existing child element items from the array root node.
                pRootElement->ClearChildren();

                // Create a new element row for each item in the resized array.
                for (int newChildIndex = 0; newChildIndex < newElementCount; ++newChildIndex)
                {
                    pCreateInfo->AddDescriptorSetLayoutCreateInfo(newDescriptorSetLayouts[newChildIndex]);

                    // Show the child index and the API's name for the structure.
                    std::stringstream elementNameStream;
                    elementNameStream << newChildIndex;
                    elementNameStream << " ";
                    elementNameStream << STR_VULKAN_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;

                    // Create an element node for each item in the array.
                    rgEditorElementArrayElement* pArrayElement = new rgEditorElementArrayElement(pRootElement, elementNameStream.str());

                    // Each element in the array needs to know its index and the array root element.
                    pArrayElement->SetElementIndex(pArrayRoot, newChildIndex);

                    // Invoke the callback used to initialize each array element.
                    InitializeDescriptorSetLayoutCreateInfo(pArrayElement, descriptorSetLayouts[newChildIndex]);

                    // Add the element node to the array root node.
                    pRootElement->AppendChildItem(pArrayElement);

                    // Initialize the newly added rows.
                    pRootElement->InitializeRows();
                }

                if (!firstInit)
                {
                    // Let the array root element know that the child elements were resized.
                    pArrayRoot->InvokeElementResizedCallback();

                    // Expand the array root node that was resized.
                    emit ExpandNode(pArrayRoot);
                }
            }
        }
    }
}

void rgPipelineStateModelVulkan::InitializeVkGraphicsPipelineCreateInfo(rgEditorElement* pRootElement, rgPsoGraphicsVulkan* pGraphicsPipelineCreateInfo)
{
    assert(pGraphicsPipelineCreateInfo != nullptr);
    if (pGraphicsPipelineCreateInfo != nullptr)
    {
        // Append the new graphics pipeline create info root item to the parent.
        rgEditorElement* pGraphicsPipelineCreateInfoRoot = new rgEditorElement(pRootElement, STR_VULKAN_GRAPHICS_PIPELINE_CREATE_INFO);
        pRootElement->AppendChildItem(pGraphicsPipelineCreateInfoRoot);

        VkGraphicsPipelineCreateInfo* pVkGraphicsPipelineCreateInfo = pGraphicsPipelineCreateInfo->GetGraphicsPipelineCreateInfo();

        // Add the "flags" member.
        const rgEnumValuesVector& pipelineFlagsEnumerators = GetPipelineCreateFlagEnumerators();
        rgEditorElement* pFlagsItem = new rgEditorElementEnum(m_pParent, STR_VULKAN_PIPELINE_MEMBER_FLAGS, pipelineFlagsEnumerators, reinterpret_cast<uint32_t*>(&pVkGraphicsPipelineCreateInfo->flags), true);
        pGraphicsPipelineCreateInfoRoot->AppendChildItem(pFlagsItem);

        // Connect to the flags enum list widget status signal.
        bool isConnected = connect(pFlagsItem, &rgEditorElement::EnumListWidgetStatusSignal, this, &rgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
        assert(isConnected);

        // Add the "pVertexInputState" node.
        VkPipelineVertexInputStateCreateInfo* pVertexInputStateCreateInfo = pGraphicsPipelineCreateInfo->GetPipelineVertexInputStateCreateInfo();
        assert(pVertexInputStateCreateInfo != nullptr);
        if (pVertexInputStateCreateInfo != nullptr)
        {
            InitializeVertexInputStateCreateInfo(pGraphicsPipelineCreateInfoRoot, pVertexInputStateCreateInfo);
        }

        // Add the "pInputAssemblyState" node.
        VkPipelineInputAssemblyStateCreateInfo* pInputAssemblyStateCreateInfo = pGraphicsPipelineCreateInfo->GetPipelineInputAssemblyStateCreateInfo();
        assert(pInputAssemblyStateCreateInfo != nullptr);
        if (pInputAssemblyStateCreateInfo != nullptr)
        {
            InitializeInputAssemblyStateCreateInfo(pGraphicsPipelineCreateInfoRoot, pInputAssemblyStateCreateInfo);
        }

        // Add the "pTessellationState" node.
        VkPipelineTessellationStateCreateInfo* pTessellationStateCreateInfo = pGraphicsPipelineCreateInfo->GetipelineTessellationStateCreateInfo();
        assert(pTessellationStateCreateInfo != nullptr);
        if (pTessellationStateCreateInfo != nullptr)
        {
            InitializeTessellationStateCreateInfo(pGraphicsPipelineCreateInfoRoot, pTessellationStateCreateInfo);
        }

        // Add the "pViewportState" node.
        VkPipelineViewportStateCreateInfo* pPipelineViewportStateCreateInfo = pGraphicsPipelineCreateInfo->GetPipelineViewportStateCreateInfo();
        assert(pPipelineViewportStateCreateInfo != nullptr);
        if (pPipelineViewportStateCreateInfo != nullptr)
        {
            InitializeViewportStateCreateInfo(pGraphicsPipelineCreateInfoRoot, pPipelineViewportStateCreateInfo);
        }

        // Add the "pRasterizationState" node.
        VkPipelineRasterizationStateCreateInfo* pPipelineRasterizationStateCreateInfo = pGraphicsPipelineCreateInfo->GetPipelineRasterizationStateCreateInfo();
        assert(pPipelineRasterizationStateCreateInfo != nullptr);
        if (pPipelineRasterizationStateCreateInfo != nullptr)
        {
            InitializeRasterizationStateCreateInfo(pGraphicsPipelineCreateInfoRoot, pPipelineRasterizationStateCreateInfo);
        }

        // Add the "pMultisampleState" node.
        VkPipelineMultisampleStateCreateInfo* pPipelineMultisampleStateCreateInfo = pGraphicsPipelineCreateInfo->GetPipelineMultisampleStateCreateInfo();
        assert(pPipelineMultisampleStateCreateInfo != nullptr);
        if (pPipelineMultisampleStateCreateInfo != nullptr)
        {
            InitializeMultisampleStateCreateInfo(pGraphicsPipelineCreateInfoRoot, pPipelineMultisampleStateCreateInfo);
        }

        // Add the "pDepthStencilState" node.
        VkPipelineDepthStencilStateCreateInfo* pPipelineDepthStencilStateCreateInfo = pGraphicsPipelineCreateInfo->GetPipelineDepthStencilStateCreateInfo();
        assert(pPipelineDepthStencilStateCreateInfo != nullptr);
        if (pPipelineDepthStencilStateCreateInfo != nullptr)
        {
            InitializeDepthStencilStateCreateInfo(pGraphicsPipelineCreateInfoRoot, pPipelineDepthStencilStateCreateInfo);
        }

        // Add the "pColorBlendState" node.
        VkPipelineColorBlendStateCreateInfo* pPipelineColorBlendStateCreateInfo = pGraphicsPipelineCreateInfo->GetPipelineColorBlendStateCreateInfo();
        assert(pPipelineColorBlendStateCreateInfo != nullptr);
        if (pPipelineColorBlendStateCreateInfo != nullptr)
        {
            InitializeColorBlendStateCreateInfo(pGraphicsPipelineCreateInfoRoot, pPipelineColorBlendStateCreateInfo);
        }

        // Add the "subpass" member.
        rgEditorElement* pSubpassCreateInfo = MakeNumericElement(pGraphicsPipelineCreateInfoRoot, STR_VULKAN_PIPELINE_MEMBER_SUBPASS, &pVkGraphicsPipelineCreateInfo->subpass);
        pGraphicsPipelineCreateInfoRoot->AppendChildItem(pSubpassCreateInfo);

        // Add the "basePipelineIndex" member.
        rgEditorElement* pBasePipelineIndexCreateInfo = MakeNumericElement(pGraphicsPipelineCreateInfoRoot, STR_VULKAN_PIPELINE_MEMBER_BASE_INDEX, &pVkGraphicsPipelineCreateInfo->basePipelineIndex);
        pGraphicsPipelineCreateInfoRoot->AppendChildItem(pBasePipelineIndexCreateInfo);
    }
}

void rgPipelineStateModelVulkan::InitializeVertexInputStateCreateInfo(rgEditorElement* pRootElement, VkPipelineVertexInputStateCreateInfo* pVertexInputStateCreateInfo)
{
    assert(pVertexInputStateCreateInfo != nullptr);
    if (pVertexInputStateCreateInfo != nullptr)
    {
        rgEditorElement* pVertexInputStateRoot = new rgEditorElement(pRootElement, STR_VULKAN_PIPELINE_MEMBER_PVERTEX_INPUT_STATE);
        pRootElement->AppendChildItem(pVertexInputStateRoot);

        // Add the "flags" member.
        rgEditorElement* pFlagsItem = MakeNumericElement(pVertexInputStateRoot, STR_VULKAN_PIPELINE_MEMBER_FLAGS, &pVertexInputStateCreateInfo->flags);
        pVertexInputStateRoot->AppendChildItem(pFlagsItem);

        // Create the vertex binding descriptions array root item.
        rgEditorElementArray* pVertexBindingDescriptionsItem = new rgEditorElementArray(pVertexInputStateRoot, STR_VULKAN_PIPELINE_MEMBER_PVERTEX_BINDING_DESCRIPTIONS,
            [=](int elementIndex) { RemoveElement(pVertexInputStateCreateInfo->pVertexBindingDescriptions, pVertexInputStateCreateInfo->vertexBindingDescriptionCount, elementIndex); });

        // Add the "vertexBindingDescriptionCount" member.
        rgEditorElement* pVertexBindingDescriptionCountItem = MakeNumericElement(nullptr, STR_VULKAN_PIPELINE_MEMBER_VERTEX_BINDING_DESCRIPTION_COUNT,
            &pVertexInputStateCreateInfo->vertexBindingDescriptionCount, [=] { HandleVertexBindingDescriptionCountChanged(pVertexBindingDescriptionsItem, pVertexInputStateCreateInfo); });

        // Provide the element used to track the dimension of the pVertexBindingDescriptions array.
        pVertexBindingDescriptionsItem->SetArraySizeElement(static_cast<rgEditorElementNumeric<uint32_t>*>(pVertexBindingDescriptionCountItem));

        // Initialize the pVertexBindingDescriptions array rows.
        HandleVertexBindingDescriptionCountChanged(pVertexBindingDescriptionsItem, pVertexInputStateCreateInfo, true);

        // Add the "pVertexBindingDescriptions" item.
        pVertexInputStateRoot->AppendChildItem(pVertexBindingDescriptionsItem);

        // Create the vertex attribute descriptions array root item.
        rgEditorElementArray* pVertexAttributeDescriptionsItem = new rgEditorElementArray(pVertexInputStateRoot, STR_VULKAN_PIPELINE_MEMBER_PVERTEX_ATTRIBUTE_DESCRIPTIONS,
            [=](int elementIndex) { RemoveElement(pVertexInputStateCreateInfo->pVertexAttributeDescriptions, pVertexInputStateCreateInfo->vertexAttributeDescriptionCount, elementIndex); });

        // Add the "vertexAttributeDescriptionCount" member.
        rgEditorElement* pVertexAttributeDescriptionCountItem = MakeNumericElement(nullptr, STR_VULKAN_PIPELINE_MEMBER_VERTEX_ATTRIBUTE_DESCRIPTION_COUNT,
            &pVertexInputStateCreateInfo->vertexAttributeDescriptionCount, [=] { HandleVertexAttributeDescriptionCountChanged(pVertexAttributeDescriptionsItem, pVertexInputStateCreateInfo); });

        // Provide the element used to track the dimension of the pVertexAttributeDescriptions array.
        pVertexAttributeDescriptionsItem->SetArraySizeElement(static_cast<rgEditorElementNumeric<uint32_t>*>(pVertexAttributeDescriptionCountItem));

        // Initialize the pVertexAttributeDescriptions array rows.
        HandleVertexAttributeDescriptionCountChanged(pVertexAttributeDescriptionsItem, pVertexInputStateCreateInfo, true);

        // Add the "pVertexAttributeDescriptions" member.
        pVertexInputStateRoot->AppendChildItem(pVertexAttributeDescriptionsItem);
    }
}

void rgPipelineStateModelVulkan::HandleVertexBindingDescriptionCountChanged(rgEditorElement* pRootElement, VkPipelineVertexInputStateCreateInfo* pInputStateCreateInfo, bool firstInit)
{
    ResizeHandler(pRootElement,
        pInputStateCreateInfo->vertexBindingDescriptionCount,
        pInputStateCreateInfo->pVertexBindingDescriptions,
        STR_VULKAN_VERTEX_INPUT_BINDING_DESCRIPTION,
        std::bind(&rgPipelineStateModelVulkan::InitializeVertexInputBindingDescriptionCreateInfo, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
        firstInit);
}

void rgPipelineStateModelVulkan::HandleVertexAttributeDescriptionCountChanged(rgEditorElement* pRootElement, VkPipelineVertexInputStateCreateInfo* pInputStateCreateInfo, bool firstInit)
{
    ResizeHandler(pRootElement,
        pInputStateCreateInfo->vertexAttributeDescriptionCount,
        pInputStateCreateInfo->pVertexAttributeDescriptions,
        STR_VULKAN_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION,
        std::bind(&rgPipelineStateModelVulkan::InitializeVertexInputAttributeDescriptionCreateInfo, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
        firstInit);
}

void rgPipelineStateModelVulkan::InitializeVertexInputBindingDescriptionCreateInfo(rgEditorElement* pRootElement, VkVertexInputBindingDescription* pBaseInputBindingDescriptionItem, int itemIndex)
{
    assert(pRootElement != nullptr);
    assert(pBaseInputBindingDescriptionItem != nullptr);
    if (pRootElement != nullptr && pBaseInputBindingDescriptionItem != nullptr)
    {
        VkVertexInputBindingDescription* pOffsetInputBindingDescriptionItem = pBaseInputBindingDescriptionItem + itemIndex;
        assert(pOffsetInputBindingDescriptionItem != nullptr);
        if (pOffsetInputBindingDescriptionItem != nullptr)
        {
            // Add the "binding" member.
            rgEditorElement* pBindingElement = MakeNumericElement(pRootElement, STR_VULKAN_PIPELINE_MEMBER_VERTEX_BINDING, &pOffsetInputBindingDescriptionItem->binding);
            pRootElement->AppendChildItem(pBindingElement);

            // Add the "stride" member.
            rgEditorElement* pStrideElement = MakeNumericElement(pRootElement, STR_VULKAN_PIPELINE_MEMBER_VERTEX_STRIDE, &pOffsetInputBindingDescriptionItem->stride);
            pRootElement->AppendChildItem(pStrideElement);

            // Add the "inputRate" member values.
            const rgEnumValuesVector inputRateValues = {
                ENUM_VALUE(VK_VERTEX_INPUT_RATE_VERTEX),
                ENUM_VALUE(VK_VERTEX_INPUT_RATE_INSTANCE)
            };
            rgEditorElement* pInputRateElement = new rgEditorElementEnum(m_pParent, STR_VULKAN_PIPELINE_MEMBER_VERTEX_INPUT_RATE, inputRateValues, reinterpret_cast<uint32_t*>(&pOffsetInputBindingDescriptionItem->inputRate));
            pRootElement->AppendChildItem(pInputRateElement);

            // Connect to the enum list widget status signal.
            bool isConnected = connect(pInputRateElement, &rgEditorElement::EnumListWidgetStatusSignal, this, &rgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
            assert(isConnected);
        }
    }
}

void rgPipelineStateModelVulkan::InitializeVertexInputAttributeDescriptionCreateInfo(rgEditorElement* pRootElement, VkVertexInputAttributeDescription* pBaseInputAttributeDescription, int itemIndex)
{
    assert(pRootElement != nullptr);
    assert(pBaseInputAttributeDescription != nullptr);
    if (pRootElement != nullptr && pBaseInputAttributeDescription != nullptr)
    {
        VkVertexInputAttributeDescription* pOffsetInputAttributeDescription = pBaseInputAttributeDescription + itemIndex;
        assert(pOffsetInputAttributeDescription != nullptr);
        if (pOffsetInputAttributeDescription != nullptr)
        {
            // Add the "location" member.
            rgEditorElement* pLocationElement = MakeNumericElement(pRootElement, STR_VULKAN_PIPELINE_MEMBER_VERTEX_LOCATION, &pOffsetInputAttributeDescription->location);
            pRootElement->AppendChildItem(pLocationElement);

            // Add the "binding" member.
            rgEditorElement* pBindingElement = MakeNumericElement(pRootElement, STR_VULKAN_PIPELINE_MEMBER_VERTEX_BINDING, &pOffsetInputAttributeDescription->binding);
            pRootElement->AppendChildItem(pBindingElement);

            // Add the "format" member values.
            const rgEnumValuesVector& formatEnumerators = GetFormatEnumerators();
            rgEditorElement* pFormatElement = new rgEditorElementEnum(m_pParent, STR_VULKAN_PIPELINE_MEMBER_VERTEX_FORMAT, formatEnumerators, reinterpret_cast<uint32_t*>(&pOffsetInputAttributeDescription->format));
            pRootElement->AppendChildItem(pFormatElement);

            // Connect to the enum list widget status signal.
            bool isConnected = connect(pFormatElement, &rgEditorElement::EnumListWidgetStatusSignal, this, &rgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
            assert(isConnected);

            // Add the "offset" member.
            rgEditorElement* pOffsetElement = MakeNumericElement(pRootElement, STR_VULKAN_PIPELINE_MEMBER_OFFSET, &pOffsetInputAttributeDescription->offset);
            pRootElement->AppendChildItem(pOffsetElement);
        }
    }
}

void rgPipelineStateModelVulkan::InitializeInputAssemblyStateCreateInfo(rgEditorElement* pRootElement, VkPipelineInputAssemblyStateCreateInfo* pInputAssemblyStateCreateInfo)
{
    assert(pInputAssemblyStateCreateInfo != nullptr);
    if (pInputAssemblyStateCreateInfo != nullptr)
    {
        rgEditorElement* pInputAssemblyStateRoot = new rgEditorElement(pRootElement, STR_VULKAN_PIPELINE_MEMBER_PINPUT_ASSEMBLY_STATE);
        pRootElement->AppendChildItem(pInputAssemblyStateRoot);

        // Add the "flags" member.
        rgEditorElement* pFlagsItem = MakeNumericElement(pInputAssemblyStateRoot, STR_VULKAN_PIPELINE_MEMBER_FLAGS, &pInputAssemblyStateCreateInfo->flags);
        pInputAssemblyStateRoot->AppendChildItem(pFlagsItem);

        // Add the primitive "topology" node.
        const rgEnumValuesVector& primitiveTopologyEnumerators = GetPrimitiveTopologyEnumerators();
        rgEditorElement* pTopologyItem = new rgEditorElementEnum(m_pParent, STR_VULKAN_PIPELINE_MEMBER_TOPOLOGY, primitiveTopologyEnumerators, reinterpret_cast<uint32_t*>(&pInputAssemblyStateCreateInfo->topology));
        pInputAssemblyStateRoot->AppendChildItem(pTopologyItem);

        // Connect to the enum list widget status signal.
        bool isConnected = connect(pTopologyItem, &rgEditorElement::EnumListWidgetStatusSignal, this, &rgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
        assert(isConnected);

        // Add the "primitiveRestartEnable" member.
        rgEditorElementBool* pPrimitiveRestartEnableItem = new rgEditorElementBool(pInputAssemblyStateRoot, STR_VULKAN_PIPELINE_MEMBER_PRIMITIVE_RESTART_ENABLE, &pInputAssemblyStateCreateInfo->primitiveRestartEnable);
        pInputAssemblyStateRoot->AppendChildItem(pPrimitiveRestartEnableItem);
    }
}

void rgPipelineStateModelVulkan::InitializeTessellationStateCreateInfo(rgEditorElement* pRootElement, VkPipelineTessellationStateCreateInfo* pTessellationStateCreateInfo)
{
    assert(pTessellationStateCreateInfo != nullptr);
    if (pTessellationStateCreateInfo != nullptr)
    {
        rgEditorElement* pTessellationStateRoot = new rgEditorElement(pRootElement, STR_VULKAN_PIPELINE_MEMBER_PTESSELLATION_STATE);
        pRootElement->AppendChildItem(pTessellationStateRoot);

        // Add the "flags" member.
        rgEditorElement* pFlagsItem = MakeNumericElement(pTessellationStateRoot, STR_VULKAN_PIPELINE_MEMBER_FLAGS, &pTessellationStateCreateInfo->flags);
        pTessellationStateRoot->AppendChildItem(pFlagsItem);

        // Add the "patchControlPoints" member.
        rgEditorElement* pPatchControlPointsItem = MakeNumericElement(pTessellationStateRoot, STR_VULKAN_PIPELINE_MEMBER_PATCH_CONTROL_POINTS, &pTessellationStateCreateInfo->patchControlPoints);
        pTessellationStateRoot->AppendChildItem(pPatchControlPointsItem);
    }
}

void rgPipelineStateModelVulkan::InitializeViewportStateCreateInfo(rgEditorElement* pRootElement, VkPipelineViewportStateCreateInfo* pPipelineViewportStateCreateInfo)
{
    assert(pPipelineViewportStateCreateInfo != nullptr);
    if (pPipelineViewportStateCreateInfo != nullptr)
    {
        rgEditorElement* pViewportStateRoot = new rgEditorElement(pRootElement, STR_VULKAN_PIPELINE_MEMBER_PVIEWPORT_STATE);
        pRootElement->AppendChildItem(pViewportStateRoot);

        // Add the "flags" member.
        rgEditorElement* pFlagsItem = MakeNumericElement(pViewportStateRoot, STR_VULKAN_PIPELINE_MEMBER_FLAGS, &pPipelineViewportStateCreateInfo->flags);
        pViewportStateRoot->AppendChildItem(pFlagsItem);

        // Create the "pViewports" root node.
        rgEditorElementArray* pViewportsRootItem = new rgEditorElementArray(pViewportStateRoot, STR_VULKAN_PIPELINE_MEMBER_PVIEWPORTS,
            [=](int elementIndex) { RemoveElement(pPipelineViewportStateCreateInfo->pViewports, pPipelineViewportStateCreateInfo->viewportCount, elementIndex); });

        assert(pViewportsRootItem != nullptr);
        if (pViewportsRootItem != nullptr)
        {
            // Create the "viewportCountItem" node.
            rgEditorElement* pViewportCountItem = MakeNumericElement(nullptr, STR_VULKAN_PIPELINE_MEMBER_VIEWPORT_COUNT,
                &pPipelineViewportStateCreateInfo->viewportCount, [=] { HandlePipelineViewportCountChanged(pViewportsRootItem, pPipelineViewportStateCreateInfo); });

            // Add the viewport array node.
            pViewportStateRoot->AppendChildItem(pViewportsRootItem);

            // Provide the element used to track the dimension of the pViewports array.
            pViewportsRootItem->SetArraySizeElement(static_cast<rgEditorElementNumeric<uint32_t>*>(pViewportCountItem));

            // Initialize the pViewports array rows.
            HandlePipelineViewportCountChanged(pViewportsRootItem, pPipelineViewportStateCreateInfo, true);
        }

        // Create the "pScissors" root node.
        rgEditorElementArray* pScissorsRootItem = new rgEditorElementArray(pViewportStateRoot, STR_VULKAN_PIPELINE_MEMBER_PSCISSORS,
            [=](int elementIndex) { RemoveElement(pPipelineViewportStateCreateInfo->pScissors, pPipelineViewportStateCreateInfo->scissorCount, elementIndex); });

        // Create the "scissorCountItem" member.
        rgEditorElement* pScissorCountItem = MakeNumericElement(nullptr, STR_VULKAN_PIPELINE_MEMBER_SCISSOR_COUNT,
            &pPipelineViewportStateCreateInfo->scissorCount, [=] { HandlePipelineScissorCountChanged(pScissorsRootItem, pPipelineViewportStateCreateInfo); });

        // Add the "pScissors" member.
        pViewportStateRoot->AppendChildItem(pScissorsRootItem);

        // Provide the element used to track the dimension of the pScissors array.
        pScissorsRootItem->SetArraySizeElement(static_cast<rgEditorElementNumeric<uint32_t>*>(pScissorCountItem));

        // Initialize the pScissors array rows.
        HandlePipelineScissorCountChanged(pScissorsRootItem, pPipelineViewportStateCreateInfo, true);
    }
}

void rgPipelineStateModelVulkan::HandlePipelineViewportCountChanged(rgEditorElement* pRootElement, VkPipelineViewportStateCreateInfo* pViewportStateCreateInfo, bool firstInit)
{
    ResizeHandler(pRootElement,
        pViewportStateCreateInfo->viewportCount,
        pViewportStateCreateInfo->pViewports,
        STR_VULKAN_PIPELINE_MEMBER_VK_VIEWPORT,
        std::bind(&rgPipelineStateModelVulkan::InitializePipelineViewportDescriptionCreateInfo, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
        firstInit);
}

void rgPipelineStateModelVulkan::HandlePipelineScissorCountChanged(rgEditorElement* pRootElement, VkPipelineViewportStateCreateInfo* pViewportStateCreateInfo, bool firstInit)
{
    ResizeHandler(pRootElement,
        pViewportStateCreateInfo->scissorCount,
        pViewportStateCreateInfo->pScissors,
        STR_VULKAN_PIPELINE_MEMBER_SCISSOR_RECT,
        std::bind(&rgPipelineStateModelVulkan::InitializePipelineScissorDescriptionCreateInfo, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
        firstInit);
}

void rgPipelineStateModelVulkan::InitializePipelineViewportDescriptionCreateInfo(rgEditorElement* pRootElement, VkViewport* pBaseViewportDescription, int itemIndex)
{
    assert(pRootElement != nullptr);
    assert(pBaseViewportDescription != nullptr);
    if (pRootElement != nullptr && pBaseViewportDescription != nullptr)
    {
        VkViewport* pOffsetViewportDescription = pBaseViewportDescription + itemIndex;
        assert(pOffsetViewportDescription != nullptr);
        if (pOffsetViewportDescription != nullptr)
        {
            // Add the "x" node.
            rgEditorElement* pXItem = MakeNumericElement(pRootElement, STR_VULKAN_MEMBER_X, &pOffsetViewportDescription->x);
            pRootElement->AppendChildItem(pXItem);

            // Add the "y" node.
            rgEditorElement* pYItem = MakeNumericElement(pRootElement, STR_VULKAN_MEMBER_Y, &pOffsetViewportDescription->y);
            pRootElement->AppendChildItem(pYItem);

            // Add the "width" node.
            rgEditorElement* pWidthItem = MakeNumericElement(pRootElement, STR_VULKAN_MEMBER_WIDTH, &pOffsetViewportDescription->width);
            pRootElement->AppendChildItem(pWidthItem);

            // Add the "height" node.
            rgEditorElement* pHeightItem = MakeNumericElement(pRootElement, STR_VULKAN_MEMBER_HEIGHT, &pOffsetViewportDescription->height);
            pRootElement->AppendChildItem(pHeightItem);

            // Add the "minDepth" node.
            rgEditorElement* pMinDepthItem = MakeNumericElement(pRootElement, STR_VULKAN_PIPELINE_MEMBER_VIEWPORT_MIN_DEPTH, &pOffsetViewportDescription->minDepth);
            pRootElement->AppendChildItem(pMinDepthItem);

            // Add the "maxDepth" node.
            rgEditorElement* pMaxDepthItem = MakeNumericElement(pRootElement, STR_VULKAN_PIPELINE_MEMBER_VIEWPORT_MAX_DEPTH, &pOffsetViewportDescription->maxDepth);
            pRootElement->AppendChildItem(pMaxDepthItem);
        }
    }
}

void rgPipelineStateModelVulkan::InitializePipelineScissorDescriptionCreateInfo(rgEditorElement* pRootElement, VkRect2D* pBaseScissorDescription, int itemIndex)
{
    assert(pRootElement != nullptr);
    assert(pBaseScissorDescription != nullptr);
    if (pRootElement != nullptr && pBaseScissorDescription != nullptr)
    {
        VkRect2D* pOffsetScissorDescription = pBaseScissorDescription + itemIndex;
        assert(pOffsetScissorDescription != nullptr);
        if (pOffsetScissorDescription != nullptr)
        {
            // Get the stencil operation enumerators.
            rgEditorElement* pOffsetRoot = new rgEditorElement(pRootElement, STR_VULKAN_PIPELINE_MEMBER_OFFSET);

            // Add the "x" node.
            rgEditorElement* pXItem = MakeNumericElement(pOffsetRoot, STR_VULKAN_MEMBER_X, &pOffsetScissorDescription->offset.x);
            pOffsetRoot->AppendChildItem(pXItem);

            // Add the "y" node.
            rgEditorElement* pYItem = MakeNumericElement(pOffsetRoot, STR_VULKAN_MEMBER_Y, &pOffsetScissorDescription->offset.y);
            pOffsetRoot->AppendChildItem(pYItem);

            // Add the offset root element.
            pRootElement->AppendChildItem(pOffsetRoot);

            rgEditorElement* pExtentRoot = new rgEditorElement(pRootElement, STR_VULKAN_PIPELINE_MEMBER_EXTENT);

            // Add the "width" node.
            rgEditorElement* pWidthItem = MakeNumericElement(pExtentRoot, STR_VULKAN_MEMBER_WIDTH, &pOffsetScissorDescription->extent.width);
            pExtentRoot->AppendChildItem(pWidthItem);

            // Add the "height" node.
            rgEditorElement* pHeightItem = MakeNumericElement(pExtentRoot, STR_VULKAN_MEMBER_HEIGHT, &pOffsetScissorDescription->extent.height);
            pExtentRoot->AppendChildItem(pHeightItem);

            // Add the extent root element.
            pRootElement->AppendChildItem(pExtentRoot);
        }
    }
}

void rgPipelineStateModelVulkan::InitializeRasterizationStateCreateInfo(rgEditorElement* pRootElement, VkPipelineRasterizationStateCreateInfo* pRasterizationStateCreateInfo)
{
    assert(pRasterizationStateCreateInfo != nullptr);
    if (pRasterizationStateCreateInfo != nullptr)
    {
        rgEditorElement* pRasterizationStateRoot = new rgEditorElement(pRootElement, STR_VULKAN_PIPELINE_MEMBER_PRASTERIZATION_STATE);
        pRootElement->AppendChildItem(pRasterizationStateRoot);

        // Add the "flags" member.
        rgEditorElement* pFlagsItem = MakeNumericElement(pRasterizationStateRoot, STR_VULKAN_PIPELINE_MEMBER_FLAGS, &pRasterizationStateCreateInfo->flags);
        pRasterizationStateRoot->AppendChildItem(pFlagsItem);

        // Add the "depthClampEnable" node.
        rgEditorElementBool* pDepthClampEnableItem = new rgEditorElementBool(pRasterizationStateRoot, STR_VULKAN_PIPELINE_MEMBER_DEPTH_CLAMP_ENABLE, &pRasterizationStateCreateInfo->depthClampEnable);
        pRasterizationStateRoot->AppendChildItem(pDepthClampEnableItem);

        // Add the "rasterizerDiscardEnable" node.
        rgEditorElementBool* pRasterizerDiscardEnable = new rgEditorElementBool(pRasterizationStateRoot, STR_VULKAN_PIPELINE_MEMBER_RASTERIZER_DISCARD_ENABLE, &pRasterizationStateCreateInfo->rasterizerDiscardEnable);
        pRasterizationStateRoot->AppendChildItem(pRasterizerDiscardEnable);

        // Add the "polygonMode" member values.
        const rgEnumValuesVector& polygonModeEnumerators = GetPolygonModeEnumerators();
        rgEditorElement* pPolygonModeNode = new rgEditorElementEnum(m_pParent, STR_VULKAN_PIPELINE_MEMBER_RASTERIZER_POLYGON_MODE, polygonModeEnumerators, reinterpret_cast<uint32_t*>(&pRasterizationStateCreateInfo->polygonMode));
        pRasterizationStateRoot->AppendChildItem(pPolygonModeNode);

        // Connect to the enum list widget status signal.
        bool isConnected = connect(pPolygonModeNode, &rgEditorElement::EnumListWidgetStatusSignal, this, &rgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
        assert(isConnected);

        // Add the "cullMode" member values.
        const rgEnumValuesVector& cullModeEnumerators = GetCullModeFlagEnumerators();
        rgEditorElement* pCullModeNode = new rgEditorElementEnum(m_pParent, STR_VULKAN_PIPELINE_MEMBER_RASTERIZER_CULL_MODE, cullModeEnumerators, reinterpret_cast<uint32_t*>(&pRasterizationStateCreateInfo->cullMode), true);
        pRasterizationStateRoot->AppendChildItem(pCullModeNode);

        // Connect to the enum list widget status signal.
        isConnected = connect(pCullModeNode, &rgEditorElement::EnumListWidgetStatusSignal, this, &rgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
        assert(isConnected);

        // Add the "frontFace" member values.
        const rgEnumValuesVector& frontFaceEnumerators = GetFrontFaceEnumerators();
        rgEditorElement* pFrontFaceNode = new rgEditorElementEnum(m_pParent, STR_VULKAN_PIPELINE_MEMBER_RASTERIZER_FRONT_FACE, frontFaceEnumerators, reinterpret_cast<uint32_t*>(&pRasterizationStateCreateInfo->frontFace));
        pRasterizationStateRoot->AppendChildItem(pFrontFaceNode);

        // Connect to the enum list widget status signal.
        isConnected = connect(pFrontFaceNode, &rgEditorElement::EnumListWidgetStatusSignal, this, &rgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
        assert(isConnected);

        // Add the "depthBiasEnable" node.
        rgEditorElementBool* pDepthBiasEnableNode = new rgEditorElementBool(pRasterizationStateRoot, STR_VULKAN_PIPELINE_MEMBER_RASTERIZER_DEPTH_BIAS_ENABLE, &pRasterizationStateCreateInfo->depthBiasEnable);
        pRasterizationStateRoot->AppendChildItem(pDepthBiasEnableNode);

        // Add the "depthBiasConstantFactor" node.
        rgEditorElement* pDepthBiasConstantFactorNode = MakeNumericElement(pRasterizationStateRoot, STR_VULKAN_PIPELINE_MEMBER_RASTERIZER_DEPTH_BIAS_CONSTANT_FACTOR, &pRasterizationStateCreateInfo->depthBiasConstantFactor);
        pRasterizationStateRoot->AppendChildItem(pDepthBiasConstantFactorNode);

        // Add the "depthBiasClamp" node.
        rgEditorElement* pDepthBiasClampNode = MakeNumericElement(pRasterizationStateRoot, STR_VULKAN_PIPELINE_MEMBER_RASTERIZER_DEPTH_BIAS_CLAMP, &pRasterizationStateCreateInfo->depthBiasClamp);
        pRasterizationStateRoot->AppendChildItem(pDepthBiasClampNode);

        // Add the "depthBiasSlopeFactor" node.
        rgEditorElement* pDepthBiasSlopeFactorNode = MakeNumericElement(pRasterizationStateRoot, STR_VULKAN_PIPELINE_MEMBER_RASTERIZER_DEPTH_BIAS_SLOPE_FACTOR, &pRasterizationStateCreateInfo->depthBiasSlopeFactor);
        pRasterizationStateRoot->AppendChildItem(pDepthBiasSlopeFactorNode);

        // Add the "lineWidth" node.
        rgEditorElement* pLineWidthNode = MakeNumericElement(pRasterizationStateRoot, STR_VULKAN_PIPELINE_MEMBER_RASTERIZER_LINE_WIDTH, &pRasterizationStateCreateInfo->lineWidth);
        pRasterizationStateRoot->AppendChildItem(pLineWidthNode);
    }
}

void rgPipelineStateModelVulkan::HandleMultisamplingSampleMaskDimensionChanged(rgEditorElement* pRootElement, VkPipelineMultisampleStateCreateInfo* pMultisampleStateCreateInfo, bool firstInit)
{
    ResizeHandler(pRootElement,
        m_sampleMaskDimension,
        pMultisampleStateCreateInfo->pSampleMask,
        STR_VULKAN_MULTISAMPLE_RASTERIZATION_SAMPLE_FLAGS_TYPE,
        std::bind(&rgPipelineStateModelVulkan::InitializeSampleMask, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
        firstInit);
}

void rgPipelineStateModelVulkan::InitializeSampleMask(rgEditorElement* pRootElement, uint32_t* pSampleMaskBase, int itemIndex)
{
    assert(pRootElement != nullptr);
    assert(pSampleMaskBase != nullptr);
    if (pRootElement != nullptr && pSampleMaskBase != nullptr)
    {
        uint32_t* pSampleMaskItem = pSampleMaskBase + itemIndex;
        assert(pSampleMaskItem != nullptr);
        if (pSampleMaskItem != nullptr)
        {
            // Add the array element.
            rgEditorElement* pSampleMaskElementNode = MakeNumericElement(pRootElement, STR_VULKAN_MULTISAMPLE_RASTERIZATION_SAMPLE_FLAGS_ELEMENT_TYPE, reinterpret_cast<uint32_t*>(pSampleMaskItem));
            pRootElement->AppendChildItem(pSampleMaskElementNode);
        }
    }
}

void rgPipelineStateModelVulkan::InitializeMultisampleStateCreateInfo(rgEditorElement* pRootElement, VkPipelineMultisampleStateCreateInfo* pPipelineMultisampleStateCreateInfo)
{
    // Initialize the sample mask dimension.
    m_sampleMaskDimension = 0;

    assert(pPipelineMultisampleStateCreateInfo != nullptr);
    if (pPipelineMultisampleStateCreateInfo != nullptr)
    {
        rgEditorElement* pMultisampleStateRoot = new rgEditorElement(pRootElement, STR_VULKAN_PIPELINE_MEMBER_PMULTISAMPLE_STATE);
        pRootElement->AppendChildItem(pMultisampleStateRoot);

        // Add the "flags" node.
        rgEditorElement* pFlagsItem = MakeNumericElement(pMultisampleStateRoot, STR_VULKAN_PIPELINE_MEMBER_FLAGS, &pPipelineMultisampleStateCreateInfo->flags);
        pMultisampleStateRoot->AppendChildItem(pFlagsItem);

    // pSampleMask array:
        if (pPipelineMultisampleStateCreateInfo->pSampleMask != nullptr)
        {
            // Initialize the mask dimension if the pSampleMask is used.
            uint32_t enumDimension = GetSampleMaskDimension(pPipelineMultisampleStateCreateInfo->rasterizationSamples);
            m_sampleMaskDimension = enumDimension;
        }

        // Create the pSampleMask array root item.
        rgEditorElementArray* pSampleMaskRootItem = new rgEditorElementArray(pMultisampleStateRoot, STR_VULKAN_MULTISAMPLE_P_SAMPLE_MASK,
            [=](int elementIndex) { RemoveElement(pPipelineMultisampleStateCreateInfo->pSampleMask, m_sampleMaskDimension, elementIndex); });

        // The user can only have up to 2 elements within the pSampleMask array.
        pSampleMaskRootItem->SetMaximumArraySize(2);

        // Create the pSampleMask dimension node.
        rgEditorElement* pSampleMaskCountNode = MakeNumericElement(nullptr, STR_VULKAN_RENDER_PASS_SUBPASS_INPUT_ATTACHMENT_COUNT,
            &m_sampleMaskDimension,
            [=] { HandleMultisamplingSampleMaskDimensionChanged(pSampleMaskRootItem, pPipelineMultisampleStateCreateInfo); });

        // Set the array size element for the pSampleMask root item.
        // When the array size count value changes, the array of child elements will get resized.
        pSampleMaskRootItem->SetArraySizeElement(static_cast<rgEditorElementNumeric<uint32_t>*>(pSampleMaskCountNode));

        // Initialize the existing array of pSampleMask data.
        HandleMultisamplingSampleMaskDimensionChanged(pSampleMaskRootItem, pPipelineMultisampleStateCreateInfo, true);

        // Add the "rasterizationSamples" flags node.
        const rgEnumValuesVector& rasterizationSamplesEnumerators = GetRasterizationSamplesEnumerators();
        rgEditorElement* pRasterizationSamples = new rgEditorElementEnum(m_pParent, STR_VULKAN_MULTISAMPLE_RASTERIZATION_SAMPLES, rasterizationSamplesEnumerators, reinterpret_cast<uint32_t*>(&pPipelineMultisampleStateCreateInfo->rasterizationSamples));
        pMultisampleStateRoot->AppendChildItem(pRasterizationSamples);

        // Connect to the enum list widget status signal.
        bool isConnected = connect(pRasterizationSamples, &rgEditorElement::EnumListWidgetStatusSignal, this, &rgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
        assert(isConnected);

        // Add the "sampleShadingEnable" node.
        rgEditorElementBool* pSampleShadingEnable = new rgEditorElementBool(pMultisampleStateRoot, STR_VULKAN_MULTISAMPLE_SAMPLE_SHADING_ENABLE, &pPipelineMultisampleStateCreateInfo->sampleShadingEnable);
        pMultisampleStateRoot->AppendChildItem(pSampleShadingEnable);

        // Add the "minSampleShading" node.
        rgEditorElement* pMinSampleShadingNode = MakeNumericElement(pMultisampleStateRoot, STR_VULKAN_MULTISAMPLE_MIN_SAMPLE_SHADING, &pPipelineMultisampleStateCreateInfo->minSampleShading);
        pMultisampleStateRoot->AppendChildItem(pMinSampleShadingNode);

        // Add the sample mask array node.
        pMultisampleStateRoot->AppendChildItem(pSampleMaskRootItem);

        // Add the "alphaToCoverageEnable" node.
        rgEditorElementBool* pAlphaToCoverageNode = new rgEditorElementBool(pMultisampleStateRoot, STR_VULKAN_MULTISAMPLE_ALPHA_TO_COVERAGE_ENABLE, &pPipelineMultisampleStateCreateInfo->alphaToCoverageEnable);
        pMultisampleStateRoot->AppendChildItem(pAlphaToCoverageNode);

        // Add the "alphaToOneEnable" node.
        rgEditorElementBool* pAlphaToOneNode = new rgEditorElementBool(pMultisampleStateRoot, STR_VULKAN_MULTISAMPLE_ALPHA_TO_ONE_ENABLE, &pPipelineMultisampleStateCreateInfo->alphaToOneEnable);
        pMultisampleStateRoot->AppendChildItem(pAlphaToOneNode);
    }
}

void rgPipelineStateModelVulkan::InitializeStencilOpState(rgEditorElement* pDepthStencilStateRoot, VkStencilOpState* pStencilOpState)
{
    assert(pDepthStencilStateRoot != nullptr);
    assert(pStencilOpState != nullptr);
    if (pDepthStencilStateRoot != nullptr && pStencilOpState != nullptr)
    {
        // Get the stencil operation enumerators.
        const rgEnumValuesVector& stencilOpEnumerators = GetStencilOpEnumerators();

        // Add the "failOp" node.
        rgEditorElement* pFailOpNode = new rgEditorElementEnum(m_pParent, STR_VULKAN_DEPTH_STENCIL_STATE_FAIL_OP, stencilOpEnumerators, reinterpret_cast<uint32_t*>(&pStencilOpState->failOp));
        pDepthStencilStateRoot->AppendChildItem(pFailOpNode);

        // Connect to the enum list widget status signal.
        bool isConnected = connect(pFailOpNode, &rgEditorElement::EnumListWidgetStatusSignal, this, &rgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
        assert(isConnected);

        // Add the "passOp" node.
        rgEditorElement* pPassOpNode = new rgEditorElementEnum(m_pParent, STR_VULKAN_DEPTH_STENCIL_STATE_PASS_OP, stencilOpEnumerators, reinterpret_cast<uint32_t*>(&pStencilOpState->passOp));
        pDepthStencilStateRoot->AppendChildItem(pPassOpNode);

        // Connect to the enum list widget status signal.
        isConnected = connect(pPassOpNode, &rgEditorElement::EnumListWidgetStatusSignal, this, &rgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
        assert(isConnected);

        // Add the "depthFailOp" node.
        rgEditorElement* pDepthFailOp = new rgEditorElementEnum(m_pParent, STR_VULKAN_DEPTH_STENCIL_STATE_DEPTH_FAIL_OP, stencilOpEnumerators, reinterpret_cast<uint32_t*>(&pStencilOpState->depthFailOp));
        pDepthStencilStateRoot->AppendChildItem(pDepthFailOp);

        // Connect to the enum list widget status signal.
        isConnected = connect(pDepthFailOp, &rgEditorElement::EnumListWidgetStatusSignal, this, &rgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
        assert(isConnected);

        // Add the "compareOp" node.
        const rgEnumValuesVector& compareOpEnumerators = GetCompareOpEnumerators();
        rgEditorElement* pCompareOpNode = new rgEditorElementEnum(m_pParent, STR_VULKAN_DEPTH_STENCIL_STATE_COMPARE_OP, compareOpEnumerators, reinterpret_cast<uint32_t*>(&pStencilOpState->compareOp));
        pDepthStencilStateRoot->AppendChildItem(pCompareOpNode);

        // Connect to the enum list widget status signal.
        isConnected = connect(pCompareOpNode, &rgEditorElement::EnumListWidgetStatusSignal, this, &rgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
        assert(isConnected);

        // Add the "compareMask" node.
        rgEditorElement* pCompareMaskNode = MakeNumericElement(pDepthStencilStateRoot, STR_VULKAN_DEPTH_STENCIL_STATE_COMPARE_MASK, &pStencilOpState->compareMask);
        pDepthStencilStateRoot->AppendChildItem(pCompareMaskNode);

        // Add the "writeMask" node.
        rgEditorElement* pWriteMaskNode = MakeNumericElement(pDepthStencilStateRoot, STR_VULKAN_DEPTH_STENCIL_STATE_WRITE_MASK, &pStencilOpState->writeMask);
        pDepthStencilStateRoot->AppendChildItem(pWriteMaskNode);

        // Add the "reference" node.
        rgEditorElement* pReferenceNode = MakeNumericElement(pDepthStencilStateRoot, STR_VULKAN_DEPTH_STENCIL_STATE_REFERENCE, &pStencilOpState->reference);
        pDepthStencilStateRoot->AppendChildItem(pReferenceNode);
    }
}

void rgPipelineStateModelVulkan::InitializeDepthStencilStateCreateInfo(rgEditorElement* pRootElement, VkPipelineDepthStencilStateCreateInfo* pPipelineDepthStencilStateCreateInfo)
{
    assert(pPipelineDepthStencilStateCreateInfo != nullptr);
    if (pPipelineDepthStencilStateCreateInfo != nullptr)
    {
        rgEditorElement* pDepthStencilStateRoot = new rgEditorElement(pRootElement, STR_VULKAN_PIPELINE_MEMBER_PDEPTH_STENCIL_STATE);
        pRootElement->AppendChildItem(pDepthStencilStateRoot);

        // Add the "flags" node.
        rgEditorElement* pFlagsItem = MakeNumericElement(pDepthStencilStateRoot, STR_VULKAN_PIPELINE_MEMBER_FLAGS, &pPipelineDepthStencilStateCreateInfo->flags);
        pDepthStencilStateRoot->AppendChildItem(pFlagsItem);

        // Add the "depthTestEnable" node.
        rgEditorElementBool* pDepthTestEnableNode = new rgEditorElementBool(pDepthStencilStateRoot, STR_VULKAN_DEPTH_STENCIL_DEPTH_TEST_ENABLE, &pPipelineDepthStencilStateCreateInfo->depthTestEnable);
        pDepthStencilStateRoot->AppendChildItem(pDepthTestEnableNode);

        // Add the "depthWriteEnable" node.
        rgEditorElementBool* pDepthWriteEnableNode = new rgEditorElementBool(pDepthStencilStateRoot, STR_VULKAN_DEPTH_STENCIL_DEPTH_WRITE_ENABLE, &pPipelineDepthStencilStateCreateInfo->depthWriteEnable);
        pDepthStencilStateRoot->AppendChildItem(pDepthWriteEnableNode);

        // Add the "depthCompareOp" node.
        const rgEnumValuesVector& compareOpEnumerators = GetCompareOpEnumerators();
        rgEditorElement* pDepthCompareOpNode = new rgEditorElementEnum(m_pParent, STR_VULKAN_DEPTH_STENCIL_DEPTH_COMPARE_OP, compareOpEnumerators, reinterpret_cast<uint32_t*>(&pPipelineDepthStencilStateCreateInfo->depthCompareOp), m_pParent);
        pDepthStencilStateRoot->AppendChildItem(pDepthCompareOpNode);

        // Connect to the enum list widget status signal.
        bool isConnected = connect(pDepthCompareOpNode, &rgEditorElement::EnumListWidgetStatusSignal, this, &rgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
        assert(isConnected);

        // Add the "depthBoundsTestEnable" node.
        rgEditorElementBool* pDepthBoundsTestEnableNode = new rgEditorElementBool(pDepthStencilStateRoot, STR_VULKAN_DEPTH_STENCIL_DEPTH_BOUNDS_TEST_ENABLE, &pPipelineDepthStencilStateCreateInfo->depthBoundsTestEnable);
        pDepthStencilStateRoot->AppendChildItem(pDepthBoundsTestEnableNode);

        // Add the "stencilTestEnable" node.
        rgEditorElementBool* pStencilTestEnableNode = new rgEditorElementBool(pDepthStencilStateRoot, STR_VULKAN_DEPTH_STENCIL_STENCIL_TEST_ENABLE, &pPipelineDepthStencilStateCreateInfo->stencilTestEnable);
        pDepthStencilStateRoot->AppendChildItem(pStencilTestEnableNode);

        // Create the root node for the front stencil op state.
        rgEditorElement* pFrontStencilOpStateNode = new rgEditorElement(pDepthStencilStateRoot, STR_VULKAN_DEPTH_STENCIL_FRONT);
        InitializeStencilOpState(pFrontStencilOpStateNode, &pPipelineDepthStencilStateCreateInfo->front);
        pDepthStencilStateRoot->AppendChildItem(pFrontStencilOpStateNode);

        // Create the root node for the back stencil op state.
        rgEditorElement* pBackStencilOpStateNode = new rgEditorElement(pDepthStencilStateRoot, STR_VULKAN_DEPTH_STENCIL_BACK);
        InitializeStencilOpState(pBackStencilOpStateNode, &pPipelineDepthStencilStateCreateInfo->back);
        pDepthStencilStateRoot->AppendChildItem(pBackStencilOpStateNode);

        // Add the "minDepthBounds" node.
        rgEditorElement* pMinDepthBoundsNode = MakeNumericElement(pDepthStencilStateRoot, STR_VULKAN_DEPTH_STENCIL_MIN_DEPTH_BOUNDS, &pPipelineDepthStencilStateCreateInfo->minDepthBounds);
        pDepthStencilStateRoot->AppendChildItem(pMinDepthBoundsNode);

        // Add the "maxDepthBounds" node.
        rgEditorElement* pMaxDepthBoundsNode = MakeNumericElement(pDepthStencilStateRoot, STR_VULKAN_DEPTH_STENCIL_MAX_DEPTH_BOUNDS, &pPipelineDepthStencilStateCreateInfo->maxDepthBounds);
        pDepthStencilStateRoot->AppendChildItem(pMaxDepthBoundsNode);
    }
}

void rgPipelineStateModelVulkan::InitializeColorBlendStateCreateInfo(rgEditorElement* pRootElement, VkPipelineColorBlendStateCreateInfo* pPipelineColorBlendStateCreateInfo)
{
    assert(pPipelineColorBlendStateCreateInfo != nullptr);
    if (pPipelineColorBlendStateCreateInfo != nullptr)
    {
        rgEditorElement* pColorBlendStateRoot = new rgEditorElement(pRootElement, STR_VULKAN_PIPELINE_MEMBER_PCOLOR_BLEND_STATE);
        pRootElement->AppendChildItem(pColorBlendStateRoot);

        // Add the "flags" node.
        rgEditorElement* pFlagsItem = MakeNumericElement(pColorBlendStateRoot, STR_VULKAN_PIPELINE_MEMBER_FLAGS, &pPipelineColorBlendStateCreateInfo->flags);
        pColorBlendStateRoot->AppendChildItem(pFlagsItem);

        // Add the "logicOpEnable" node.
        rgEditorElementBool* pLogicOpEnableNode = new rgEditorElementBool(pColorBlendStateRoot, STR_VULKAN_COLOR_BLEND_STATE_LOGIC_OP_ENABLE, &pPipelineColorBlendStateCreateInfo->logicOpEnable);
        pColorBlendStateRoot->AppendChildItem(pLogicOpEnableNode);

        // Add the "logicOp" node.
        const rgEnumValuesVector& logicOpEnumerators = GetLogicOpEnumerators();
        rgEditorElement* pLogicOpNode = new rgEditorElementEnum(m_pParent, STR_VULKAN_COLOR_BLEND_STATE_LOGIC_OP, logicOpEnumerators, reinterpret_cast<uint32_t*>(&pPipelineColorBlendStateCreateInfo->logicOp));
        pColorBlendStateRoot->AppendChildItem(pLogicOpNode);

        // Connect to the enum list widget status signal.
        bool isConnected = connect(pLogicOpNode, &rgEditorElement::EnumListWidgetStatusSignal, this, &rgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
        assert(isConnected);

        // Attachment array:
            // Create the attachments array root item.
        rgEditorElementArray* pAttachmentsRootItem = new rgEditorElementArray(pColorBlendStateRoot, STR_VULKAN_COLOR_BLEND_STATE_P_ATTACHMENTS,
            [=](int elementIndex) { RemoveElement(pPipelineColorBlendStateCreateInfo->pAttachments, pPipelineColorBlendStateCreateInfo->attachmentCount, elementIndex); });

        // Create the attachmentCount member.
        rgEditorElement* pColorBlendAttachmentStateCountItem = MakeNumericElement(nullptr, STR_VULKAN_COLOR_BLEND_STATE_ATTACHMENT_COUNT,
            &pPipelineColorBlendStateCreateInfo->attachmentCount, [=] { HandlePipelineColorBlendAttachmentCountChanged(pAttachmentsRootItem, pPipelineColorBlendStateCreateInfo); });

        // Add the attachment array node.
        pColorBlendStateRoot->AppendChildItem(pAttachmentsRootItem);

        // Provide the element used to track the dimension of the pAttachments array.
        pAttachmentsRootItem->SetArraySizeElement(static_cast<rgEditorElementNumeric<uint32_t>*>(pColorBlendAttachmentStateCountItem));

        // Initialize the pAttachments array rows.
        HandlePipelineColorBlendAttachmentCountChanged(pAttachmentsRootItem, pPipelineColorBlendStateCreateInfo, true);

        // Add the blend constants array root node.
        rgEditorElement* pBlendConstantsRootNode = new rgEditorElement(pColorBlendStateRoot, STR_VULKAN_COLOR_BLEND_STATE_BLEND_CONSTANTS);

        // Add the blendConstants array item nodes.
        for (uint32_t blendConstantIndex = 0; blendConstantIndex < 4; ++blendConstantIndex)
        {
            std::stringstream elementNameStream;
            elementNameStream << STR_VULKAN_COLOR_BLEND_STATE_BLEND_CONSTANTS;
            elementNameStream << "[";
            elementNameStream << blendConstantIndex;
            elementNameStream << "]";

            rgEditorElement* pBlendConstantNode = MakeNumericElement(pBlendConstantsRootNode, elementNameStream.str().c_str(), &pPipelineColorBlendStateCreateInfo->blendConstants[blendConstantIndex]);
            pBlendConstantsRootNode->AppendChildItem(pBlendConstantNode);
        }

        pColorBlendStateRoot->AppendChildItem(pBlendConstantsRootNode);
    }
}

void rgPipelineStateModelVulkan::HandlePipelineColorBlendAttachmentCountChanged(rgEditorElement* pRootElement, VkPipelineColorBlendStateCreateInfo* pPipelineColorBlendStateCreateInfo, bool firstInit)
{
    ResizeHandler(pRootElement,
        pPipelineColorBlendStateCreateInfo->attachmentCount,
        pPipelineColorBlendStateCreateInfo->pAttachments,
        STR_VULKAN_COLOR_BLEND_ATTACHMENT_STATE,
        std::bind(&rgPipelineStateModelVulkan::InitializePipelineBlendAttachmentStateCreateInfo, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
        firstInit);
}

void rgPipelineStateModelVulkan::InitializePipelineBlendAttachmentStateCreateInfo(rgEditorElement* pRootElement, VkPipelineColorBlendAttachmentState* pBaseColorBlendAttachmentState, int itemIndex)
{
    assert(pRootElement != nullptr);
    assert(pBaseColorBlendAttachmentState != nullptr);
    if (pRootElement != nullptr && pBaseColorBlendAttachmentState != nullptr)
    {
        VkPipelineColorBlendAttachmentState* pOffsetColorBlendAttachmentState = pBaseColorBlendAttachmentState + itemIndex;
        assert(pOffsetColorBlendAttachmentState != nullptr);
        if (pOffsetColorBlendAttachmentState != nullptr)
        {
            // Add the "blendEnable" node.
            rgEditorElementBool* pBlendEnableNode = new rgEditorElementBool(pRootElement, STR_VULKAN_COLOR_BLEND_ATTACHMENT_STATE_BLEND_ENABLE, &pOffsetColorBlendAttachmentState->blendEnable);
            pRootElement->AppendChildItem(pBlendEnableNode);

            // Get the blend factor enumerators.
            const rgEnumValuesVector& blendFactorEnumerators = GetBlendFactorEnumerators();

            // Get the blend op enumerators.
            const rgEnumValuesVector& blendOpEnumerators = GetBlendOpEnumerators();

            // Add the "srcColorBlendFactor" node.
            rgEditorElement* pSrcColorBlendFactorNode = new rgEditorElementEnum(m_pParent, STR_VULKAN_COLOR_BLEND_ATTACHMENT_STATE_SRC_COLOR_BLEND_FACTOR, blendFactorEnumerators, reinterpret_cast<uint32_t*>(&pOffsetColorBlendAttachmentState->srcColorBlendFactor));
            pRootElement->AppendChildItem(pSrcColorBlendFactorNode);

            // Connect to the enum list widget status signal.
            bool isConnected = connect(pSrcColorBlendFactorNode, &rgEditorElement::EnumListWidgetStatusSignal, this, &rgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
            assert(isConnected);

            // Add the "dstColorBlendFactor" node.
            rgEditorElement* pDstColorBlendFactorNode = new rgEditorElementEnum(m_pParent, STR_VULKAN_COLOR_BLEND_ATTACHMENT_STATE_DST_COLOR_BLEND_FACTOR, blendFactorEnumerators, reinterpret_cast<uint32_t*>(&pOffsetColorBlendAttachmentState->dstColorBlendFactor));
            pRootElement->AppendChildItem(pDstColorBlendFactorNode);

            // Connect to the enum list widget status signal.
            isConnected = connect(pDstColorBlendFactorNode, &rgEditorElement::EnumListWidgetStatusSignal, this, &rgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
            assert(isConnected);

            // Add the "colorBlendOp" node.
            rgEditorElement* pColorBlendOpNode = new rgEditorElementEnum(m_pParent, STR_VULKAN_COLOR_BLEND_ATTACHMENT_STATE_COLOR_BLEND_OP, blendOpEnumerators, reinterpret_cast<uint32_t*>(&pOffsetColorBlendAttachmentState->colorBlendOp));
            pRootElement->AppendChildItem(pColorBlendOpNode);

            // Connect to the enum list widget status signal.
            isConnected = connect(pColorBlendOpNode, &rgEditorElement::EnumListWidgetStatusSignal, this, &rgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
            assert(isConnected);

            // Add the "srcAlphaBlendFactor" node.
            rgEditorElement* pSrcAlphaBlendFactorNode = new rgEditorElementEnum(m_pParent, STR_VULKAN_COLOR_BLEND_ATTACHMENT_STATE_SRC_ALPHA_BLEND_FACTOR, blendFactorEnumerators, reinterpret_cast<uint32_t*>(&pOffsetColorBlendAttachmentState->srcAlphaBlendFactor));
            pRootElement->AppendChildItem(pSrcAlphaBlendFactorNode);

            // Connect to the enum list widget status signal.
            isConnected = connect(pSrcAlphaBlendFactorNode, &rgEditorElement::EnumListWidgetStatusSignal, this, &rgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
            assert(isConnected);

            // Add the "dstAlphaBlendFactor" node.
            rgEditorElement* pDstAlphaBlendFactorNode = new rgEditorElementEnum(m_pParent, STR_VULKAN_COLOR_BLEND_ATTACHMENT_STATE_DST_ALPHA_BLEND_FACTOR, blendFactorEnumerators, reinterpret_cast<uint32_t*>(&pOffsetColorBlendAttachmentState->dstAlphaBlendFactor));
            pRootElement->AppendChildItem(pDstAlphaBlendFactorNode);

            // Connect to the enum list widget status signal.
            isConnected = connect(pDstAlphaBlendFactorNode, &rgEditorElement::EnumListWidgetStatusSignal, this, &rgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
            assert(isConnected);

            // Add the "alphaBlendOp" node.
            rgEditorElement* pAlphaBlendOpNode = new rgEditorElementEnum(m_pParent, STR_VULKAN_COLOR_BLEND_ATTACHMENT_STATE_ALPHA_BLEND_OP, blendOpEnumerators, reinterpret_cast<uint32_t*>(&pOffsetColorBlendAttachmentState->alphaBlendOp));
            pRootElement->AppendChildItem(pAlphaBlendOpNode);

            // Connect to the enum list widget status signal.
            isConnected = connect(pAlphaBlendOpNode, &rgEditorElement::EnumListWidgetStatusSignal, this, &rgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
            assert(isConnected);

            // Add the "colorWriteMask" member values.
            const rgEnumValuesVector& colorComponentFlagEnumerators = GetColorComponentFlagEnumerators();
            rgEditorElement* pColorComponentWriteMask = new rgEditorElementEnum(m_pParent, STR_VULKAN_COLOR_BLEND_ATTACHMENT_STATE_COLOR_WRITE_MASK, colorComponentFlagEnumerators, reinterpret_cast<uint32_t*>(&pOffsetColorBlendAttachmentState->colorWriteMask), true);
            pRootElement->AppendChildItem(pColorComponentWriteMask);

            // Connect to the enum list widget status signal.
            isConnected = connect(pColorComponentWriteMask, &rgEditorElement::EnumListWidgetStatusSignal, this, &rgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
            assert(isConnected);
        }
    }
}

rgEditorElement* rgPipelineStateModelVulkan::InitializeComputePipelineCreateInfo(QWidget* pParent)
{
    rgEditorElement* pCreateInfoRootNode = nullptr;

    assert(m_pComputePipelineState != nullptr);
    if (m_pComputePipelineState != nullptr)
    {
        // The create info root node that all other create info elements are attached to.
        pCreateInfoRootNode = new rgEditorElement(pParent, STR_VULKAN_COMPUTE_PIPELINE_STATE);

        // Add the Compute Pipeline create info root node.
        InitializeVkComputePipelineCreateInfo(pCreateInfoRootNode, m_pComputePipelineState);

        // Add the Pipeline Layout create info root node.
        VkPipelineLayoutCreateInfo* pPipelineLayoutCreateInfo = m_pComputePipelineState->GetPipelineLayoutCreateInfo();
        assert(pPipelineLayoutCreateInfo != nullptr);
        if (pPipelineLayoutCreateInfo != nullptr)
        {
            InitializePipelineLayoutCreateInfo(pCreateInfoRootNode, pPipelineLayoutCreateInfo);
        }

        // Initialize rows related to Descriptor Set Layout configuration.
        InitializeDescriptorSetLayoutCreateInfoArray(pCreateInfoRootNode, m_pComputePipelineState);
    }

    return pCreateInfoRootNode;
}

bool rgPipelineStateModelVulkan::LoadPipelineStateFile(QWidget* pParent, const std::string& psoFilePath, rgPipelineType pipelineType, std::string& errorString)
{
    bool isOk = false;

    try
    {
        // Assign the type of pipeline being loaded from file.
        m_pipelineType = pipelineType;

        if (m_pipelineType == rgPipelineType::Graphics)
        {
            rgPsoGraphicsVulkan* pGraphicsPipelineState = nullptr;

            // Load the graphics pipeline state file.
            isOk = rgPsoSerializerVulkan::ReadStructureFromFile(psoFilePath, &pGraphicsPipelineState, errorString);

            assert(isOk);
            if (isOk)
            {
                // Assign the new graphics pipeline state in the model.
                m_pGraphicsPipelineState = pGraphicsPipelineState;

                // Initialize the create info structure to bind it to the model.
                m_pRootItem = InitializeGraphicsPipelineCreateInfo(pParent);
            }
            else
            {
                rgUtils::ShowErrorMessageBox(errorString.c_str());
            }
        }
        else if (m_pipelineType == rgPipelineType::Compute)
        {
            rgPsoComputeVulkan* pComputePipelineState = nullptr;

            // Load the compute pipeline state file.
            isOk = rgPsoSerializerVulkan::ReadStructureFromFile(psoFilePath, &pComputePipelineState, errorString);

            assert(isOk);
            if (isOk)
            {
                // Assign the new compute pipeline state in the model.
                m_pComputePipelineState = pComputePipelineState;

                // Initialize the create info structure to bind it to the model.
                m_pRootItem = InitializeComputePipelineCreateInfo(pParent);
            }
            else
            {
                rgUtils::ShowErrorMessageBox(errorString.c_str());
            }
        }
        else
        {
            assert(false);
            errorString = STR_ERR_CANNOT_DETERMINE_PIPELINE_TYPE;
        }
    }
    catch (...)
    {
        errorString = STR_ERR_FAILED_TO_READ_PIPELINE_STATE_FILE;
        rgUtils::ShowErrorMessageBox(errorString.c_str());
    }

    return isOk;
}

bool rgPipelineStateModelVulkan::SavePipelineStateFile(const std::string& psoFilePath, std::string& errorString)
{
    bool isOk = false;

    try
    {
        std::string validationErrorString;
        bool isValid = CheckValidPipelineState(validationErrorString);

        assert(isValid);
        if (isValid)
        {
            if (m_pipelineType == rgPipelineType::Graphics)
            {
                // Save the graphics pipeline state file.
                isOk = rgPsoSerializerVulkan::WriteStructureToFile(m_pGraphicsPipelineState, psoFilePath, errorString);
            }
            else if (m_pipelineType == rgPipelineType::Compute)
            {
                // Save the compute pipeline state file.
                isOk = rgPsoSerializerVulkan::WriteStructureToFile(m_pComputePipelineState, psoFilePath, errorString);
            }
            else
            {
                assert(false);
                errorString = STR_ERR_CANNOT_DETERMINE_PIPELINE_TYPE;
            }
        }
        else
        {
            std::stringstream errorStream;
            errorStream << STR_ERR_FAILED_TO_VALIDATE << std::endl;
            errorStream << validationErrorString;
            errorString = errorStream.str();
        }
    }
    catch (...)
    {
        errorString = STR_ERR_FAILED_TO_SAVE_PIPELINE_STATE_FILE;
    }

    return isOk;
}

void rgPipelineStateModelVulkan::InitializeVkComputePipelineCreateInfo(rgEditorElement* pRootElement, rgPsoComputeVulkan* pComputePipelineCreateInfo)
{
    assert(pComputePipelineCreateInfo != nullptr);
    if (pComputePipelineCreateInfo != nullptr)
    {
        VkComputePipelineCreateInfo* pVkComputePipelineCreateInfo = static_cast<VkComputePipelineCreateInfo*>(pComputePipelineCreateInfo->GetComputePipelineCreateInfo());
        assert(pVkComputePipelineCreateInfo != nullptr);
        if (pVkComputePipelineCreateInfo != nullptr)
        {
            rgEditorElement* pVkComputePipelineCreateInfoRoot = new rgEditorElement(pRootElement, STR_VULKAN_COMPUTE_PIPELINE_CREATE_INFO);
            pRootElement->AppendChildItem(pVkComputePipelineCreateInfoRoot);

            // Add the "flags" member.
            const rgEnumValuesVector& pipelineFlagsEnumerators = GetPipelineCreateFlagEnumerators();
            rgEditorElement* pFlagsItem = new rgEditorElementEnum(m_pParent, STR_VULKAN_PIPELINE_MEMBER_FLAGS, pipelineFlagsEnumerators, reinterpret_cast<uint32_t*>(&pVkComputePipelineCreateInfo->flags), true);
            pVkComputePipelineCreateInfoRoot->AppendChildItem(pFlagsItem);

            // Connect to the flags enum list widget status signal.
            bool isConnected = connect(pFlagsItem, &rgEditorElement::EnumListWidgetStatusSignal, this, &rgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
            assert(isConnected);

            // Add the "basePipelineIndex" member.
            rgEditorElement* pBasePipelineIndexCreateInfo = MakeNumericElement(pVkComputePipelineCreateInfoRoot, STR_VULKAN_PIPELINE_MEMBER_BASE_INDEX, &pVkComputePipelineCreateInfo->basePipelineIndex);
            pVkComputePipelineCreateInfoRoot->AppendChildItem(pBasePipelineIndexCreateInfo);
        }
    }
}

void rgPipelineStateModelVulkan::InitializePipelineLayoutCreateInfo(rgEditorElement* pRootElement, VkPipelineLayoutCreateInfo* pPipelineLayoutCreateInfo)
{
    assert(pPipelineLayoutCreateInfo != nullptr);
    if (pPipelineLayoutCreateInfo != nullptr)
    {
        rgEditorElement* pPipelineLayoutCreateInfoRoot = new rgEditorElement(pRootElement, STR_VULKAN_PIPELINE_LAYOUT_CREATE_INFO);
        pRootElement->AppendChildItem(pPipelineLayoutCreateInfoRoot);

        // Add the "flags" member.
        rgEditorElement* pFlagsItem = MakeNumericElement(pPipelineLayoutCreateInfoRoot, STR_VULKAN_PIPELINE_MEMBER_FLAGS, &pPipelineLayoutCreateInfo->flags);
        pPipelineLayoutCreateInfoRoot->AppendChildItem(pFlagsItem);

    // Descriptor Set Layout array:
        // Create the layout array root item.
        rgEditorElementArray* pDescriptorSetLayoutsRootItem = new rgEditorElementArray(pPipelineLayoutCreateInfoRoot, STR_VULKAN_PIPELINE_LAYOUT_P_SET_LAYOUTS,
            [=](int elementIndex) { RemoveElement(pPipelineLayoutCreateInfo->pSetLayouts, pPipelineLayoutCreateInfo->setLayoutCount, elementIndex); });

        // Create the setLayoutCount member.
        rgEditorElement* pDescriptorSetLayoutCountItem = MakeNumericElement(nullptr, STR_VULKAN_PIPELINE_LAYOUT_DESCRIPTOR_SET_LAYOUT_COUNT,
            &pPipelineLayoutCreateInfo->setLayoutCount, [=] { HandlePipelineLayoutDescriptorSetLayoutCountChanged(pDescriptorSetLayoutsRootItem, pPipelineLayoutCreateInfo); });

        // Provide the element used to track the dimension of the Descriptor Set Layouts array.
        pDescriptorSetLayoutsRootItem->SetArraySizeElement(static_cast<rgEditorElementNumeric<uint32_t>*>(pDescriptorSetLayoutCountItem));

        // Initialize the Descriptor Set Layout array rows.
        HandlePipelineLayoutDescriptorSetLayoutCountChanged(pDescriptorSetLayoutsRootItem, pPipelineLayoutCreateInfo, true);

        // Add the descriptor set layouts array node.
        pPipelineLayoutCreateInfoRoot->AppendChildItem(pDescriptorSetLayoutsRootItem);

    // Push Constants array:
        // Create the push constants array root item.
        rgEditorElementArray* pPushConstantsArrayRootItem = new rgEditorElementArray(pPipelineLayoutCreateInfoRoot, STR_VULKAN_PIPELINE_LAYOUT_P_PUSH_CONSTANT_RANGES,
            [=](int elementIndex) { RemoveElement(pPipelineLayoutCreateInfo->pPushConstantRanges, pPipelineLayoutCreateInfo->pushConstantRangeCount, elementIndex); });

        // Create the pushConstantRangeCount member.
        rgEditorElement* pPushConstantsCountItem = MakeNumericElement(nullptr, STR_VULKAN_PIPELINE_LAYOUT_PUSH_CONSTANT_RANGE_COUNT,
            &pPipelineLayoutCreateInfo->pushConstantRangeCount, [=] { HandlePushConstantsCountChanged(pPushConstantsArrayRootItem, pPipelineLayoutCreateInfo); });

        // Provide the element used to track the dimension of the Push Constants array.
        pPushConstantsArrayRootItem->SetArraySizeElement(static_cast<rgEditorElementNumeric<uint32_t>*>(pPushConstantsCountItem));

        // Initialize the Push Constants array rows.
        HandlePushConstantsCountChanged(pPushConstantsArrayRootItem, pPipelineLayoutCreateInfo, true);

        // Add the descriptor set layouts array node.
        pPipelineLayoutCreateInfoRoot->AppendChildItem(pPushConstantsArrayRootItem);
    }
}

void rgPipelineStateModelVulkan::InitializeDescriptorSetLayoutCreateInfo(rgEditorElement* pRootElement, VkDescriptorSetLayoutCreateInfo* pDescriptorSetLayoutCreateInfo)
{
    assert(pRootElement != nullptr);
    assert(pDescriptorSetLayoutCreateInfo != nullptr);
    if (pRootElement != nullptr && pDescriptorSetLayoutCreateInfo != nullptr)
    {
        // Add the "flags" node.
        const rgEnumValuesVector& descriptorSetLayoutCreateFlags = GetDescriptorSetLayoutCreateFlagEnumerators();
        rgEditorElement* pDescriptorSetLayoutFlagsNode = new rgEditorElementEnum(m_pParent, STR_VULKAN_PIPELINE_MEMBER_FLAGS, descriptorSetLayoutCreateFlags, reinterpret_cast<uint32_t*>(&pDescriptorSetLayoutCreateInfo->flags), true);
        pRootElement->AppendChildItem(pDescriptorSetLayoutFlagsNode);

        // Connect to the enum list widget status signal.
        bool isConnected = connect(pDescriptorSetLayoutFlagsNode, &rgEditorElement::EnumListWidgetStatusSignal, this, &rgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
        assert(isConnected);

        // Binding array:
            // Create the binding array root item.
        rgEditorElementArray* pDescriptorSetLayoutBindingsRootItem = new rgEditorElementArray(pRootElement, STR_VULKAN_PIPELINE_LAYOUT_DESCRIPTOR_SET_LAYOUT_P_BINDINGS,
            [=](int elementIndex) { RemoveElement(pDescriptorSetLayoutCreateInfo->pBindings, pDescriptorSetLayoutCreateInfo->bindingCount, elementIndex); });

        // Create the bindingCount member.
        rgEditorElement* pDescriptorSetBindingCountItem = MakeNumericElement(nullptr, STR_VULKAN_PIPELINE_LAYOUT_DESCRIPTOR_SET_LAYOUT_BINDING_COUNT,
            &pDescriptorSetLayoutCreateInfo->bindingCount, [=] { HandleDescriptorSetLayoutBindingCountChanged(pDescriptorSetLayoutBindingsRootItem, pDescriptorSetLayoutCreateInfo); });

        // Provide the element used to track the dimension of the pDescriptorSetBindings array.
        pDescriptorSetLayoutBindingsRootItem->SetArraySizeElement(static_cast<rgEditorElementNumeric<uint32_t>*>(pDescriptorSetBindingCountItem));

        // Initialize the pDescriptorSetBindings array rows.
        HandleDescriptorSetLayoutBindingCountChanged(pDescriptorSetLayoutBindingsRootItem, pDescriptorSetLayoutCreateInfo, true);

        // Add the descriptor set layout bindings array node.
        pRootElement->AppendChildItem(pDescriptorSetLayoutBindingsRootItem);
    }
}

void rgPipelineStateModelVulkan::HandlePipelineLayoutDescriptorSetLayoutCountChanged(rgEditorElement* pRootElement, VkPipelineLayoutCreateInfo* pPipelineLayoutCreateInfo, bool firstInit)
{
    ResizeHandler(pRootElement,
        pPipelineLayoutCreateInfo->setLayoutCount,
        pPipelineLayoutCreateInfo->pSetLayouts,
        STR_VULKAN_DESCRIPTOR_SET_LAYOUT_HANDLE,
        std::bind(&rgPipelineStateModelVulkan::InitializeDescriptorSetLayout, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
        firstInit);
}

void rgPipelineStateModelVulkan::HandlePushConstantsCountChanged(rgEditorElement* pRootElement, VkPipelineLayoutCreateInfo* pPipelineLayoutCreateInfo, bool firstInit)
{
    ResizeHandler(pRootElement,
        pPipelineLayoutCreateInfo->pushConstantRangeCount,
        pPipelineLayoutCreateInfo->pPushConstantRanges,
        STR_VULKAN_PUSH_CONSTANT_RANGE_TYPE,
        std::bind(&rgPipelineStateModelVulkan::InitializePushConstantRange, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
        firstInit);
}

void rgPipelineStateModelVulkan::HandleDescriptorSetLayoutBindingCountChanged(rgEditorElement* pRootElement, VkDescriptorSetLayoutCreateInfo* pDescriptorSetLayoutCreateInfo, bool firstInit)
{
    ResizeHandler(pRootElement,
        pDescriptorSetLayoutCreateInfo->bindingCount,
        pDescriptorSetLayoutCreateInfo->pBindings,
        STR_VULKAN_DESCRIPTOR_SET_LAYOUT_BINDING_TYPE,
        std::bind(&rgPipelineStateModelVulkan::InitializeDescriptorSetLayoutBinding, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
        firstInit);
}

void rgPipelineStateModelVulkan::InitializeDescriptorSetLayoutBinding(rgEditorElement* pRootElement, VkDescriptorSetLayoutBinding* pBaseDescriptorSetLayout, int itemIndex)
{
    assert(pRootElement != nullptr);
    assert(pBaseDescriptorSetLayout != nullptr);
    if (pRootElement != nullptr && pBaseDescriptorSetLayout != nullptr)
    {
        VkDescriptorSetLayoutBinding* pOffsetDescriptorSetLayout = pBaseDescriptorSetLayout + itemIndex;
        assert(pOffsetDescriptorSetLayout != nullptr);
        if (pOffsetDescriptorSetLayout != nullptr)
        {
            // Add the "binding" node.
            rgEditorElement* pDescriptorSetLayoutBindingIndexNode = MakeNumericElement(pRootElement, STR_VULKAN_DESCRIPTOR_SET_LAYOUT_BINDING, &pOffsetDescriptorSetLayout->binding);
            pRootElement->AppendChildItem(pDescriptorSetLayoutBindingIndexNode);

            // Add the "descriptorType" node.
            const rgEnumValuesVector& descriptorTypeEnumerators = GetDescriptorTypeEnumerators();
            rgEditorElement* pDescriptorTypeNode = new rgEditorElementEnum(m_pParent, STR_VULKAN_DESCRIPTOR_SET_LAYOUT_BINDING_DESCRIPTOR_TYPE, descriptorTypeEnumerators, reinterpret_cast<uint32_t*>(&pOffsetDescriptorSetLayout->descriptorType));
            pRootElement->AppendChildItem(pDescriptorTypeNode);

            // Connect to the enum list widget status signal.
            bool isConnected = connect(pDescriptorTypeNode, &rgEditorElement::EnumListWidgetStatusSignal, this, &rgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
            assert(isConnected);

            // Add the "descriptorCount" node.
            rgEditorElement* pDescriptorSetLayoutBindingCountNode = MakeNumericElement(pRootElement, STR_VULKAN_DESCRIPTOR_SET_LAYOUT_BINDING_DESCRIPTOR_COUNT, &pOffsetDescriptorSetLayout->descriptorCount);
            pRootElement->AppendChildItem(pDescriptorSetLayoutBindingCountNode);

            // Add the "stageFlags" member node.
            const rgEnumValuesVector& stageFlagEnumerators = GetShaderStageFlagEnumerators();
            rgEditorElement* pStageFlagsNode = new rgEditorElementEnum(m_pParent, STR_VULKAN_PIPELINE_LAYOUT_STAGE_FLAGS, stageFlagEnumerators, reinterpret_cast<uint32_t*>(&pOffsetDescriptorSetLayout->stageFlags), true);
            pRootElement->AppendChildItem(pStageFlagsNode);

            // Connect to the enum list widget status signal.
            isConnected = connect(pStageFlagsNode, &rgEditorElement::EnumListWidgetStatusSignal, this, &rgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
            assert(isConnected);
        }
    }
}

void rgPipelineStateModelVulkan::InitializeDescriptorSetLayout(rgEditorElement* pRootElement, VkDescriptorSetLayout* pBaseDescriptorSetLayout, int itemIndex)
{
    assert(pRootElement != nullptr);
    assert(pBaseDescriptorSetLayout != nullptr);
    if (pRootElement != nullptr && pBaseDescriptorSetLayout != nullptr)
    {
        VkDescriptorSetLayout* pOffsetDescriptorSetLayout = pBaseDescriptorSetLayout + itemIndex;
        assert(pOffsetDescriptorSetLayout != nullptr);
        if (pOffsetDescriptorSetLayout != nullptr)
        {
            // Add the "pDescriptorSetLayout" handle member.
            rgEditorElement* pDescriptorSetLayoutHandleNode = MakeNumericElement(pRootElement, STR_VULKAN_DESCRIPTOR_SET_LAYOUT_HANDLE, reinterpret_cast<uint32_t*>(pOffsetDescriptorSetLayout));
            pRootElement->AppendChildItem(pDescriptorSetLayoutHandleNode);
        }
    }
}

void rgPipelineStateModelVulkan::InitializePushConstantRange(rgEditorElement* pRootElement, VkPushConstantRange* pBasePushConstant, int itemIndex)
{
    assert(pRootElement != nullptr);
    assert(pBasePushConstant != nullptr);
    if (pRootElement != nullptr && pBasePushConstant != nullptr)
    {
        VkPushConstantRange* pOffsetPushConstant = pBasePushConstant + itemIndex;
        assert(pOffsetPushConstant != nullptr);
        if (pOffsetPushConstant != nullptr)
        {
            // Add the "stageFlags" member node.
            const rgEnumValuesVector& stageFlagEnumerators = GetShaderStageFlagEnumerators();
            rgEditorElement* pStageFlagsNode = new rgEditorElementEnum(m_pParent, STR_VULKAN_PIPELINE_LAYOUT_STAGE_FLAGS, stageFlagEnumerators, reinterpret_cast<uint32_t*>(&pOffsetPushConstant->stageFlags), true);
            pRootElement->AppendChildItem(pStageFlagsNode);

            // Connect to the enum list widget status signal.
            bool isConnected = connect(pStageFlagsNode, &rgEditorElement::EnumListWidgetStatusSignal, this, &rgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
            assert(isConnected);

            // Add the "offset" member.
            rgEditorElement* pOffsetNode = MakeNumericElement(pRootElement, STR_VULKAN_PIPELINE_LAYOUT_PUSH_CONSTANT_OFFSET, &pOffsetPushConstant->offset);
            pRootElement->AppendChildItem(pOffsetNode);

            // Add the "size" member.
            rgEditorElement* pSizeNode = MakeNumericElement(pRootElement, STR_VULKAN_PIPELINE_LAYOUT_PUSH_CONSTANT_SIZE, &pOffsetPushConstant->size);
            pRootElement->AppendChildItem(pSizeNode);
        }
    }
}

void rgPipelineStateModelVulkan::InitializeRenderPassCreateInfo(rgEditorElement* pRootElement, VkRenderPassCreateInfo* pRenderPassCreateInfo)
{
    assert(pRenderPassCreateInfo != nullptr);
    if (pRenderPassCreateInfo != nullptr)
    {
        rgEditorElement* pRenderPassCreateInfoRoot = new rgEditorElement(pRootElement, STR_VULKAN_RENDER_PASS_CREATE_INFO);
        pRootElement->AppendChildItem(pRenderPassCreateInfoRoot);

        // Add the "flags" member.
        rgEditorElement* pFlagsItem = MakeNumericElement(pRenderPassCreateInfoRoot, STR_VULKAN_PIPELINE_MEMBER_FLAGS, &pRenderPassCreateInfo->flags);
        pRenderPassCreateInfoRoot->AppendChildItem(pFlagsItem);

    // Attachment array:
        // Create the attachments array root item.
        rgEditorElementArray* pAttachmentsRootItem = new rgEditorElementArray(pRenderPassCreateInfoRoot, STR_VULKAN_RENDER_PASS_P_ATTACHMENTS,
            [=](int elementIndex) { RemoveElement(pRenderPassCreateInfo->pAttachments, pRenderPassCreateInfo->attachmentCount, elementIndex); });

        // Create the attachmentCount member.
        rgEditorElement* pAttachmentCountItem = MakeNumericElement(nullptr, STR_VULKAN_RENDER_PASS_ATTACHMENT_COUNT,
            &pRenderPassCreateInfo->attachmentCount, [=] { HandleRenderPassAttachmentCountChanged(pAttachmentsRootItem, pRenderPassCreateInfo); });

        // Add the attachment array node.
        pRenderPassCreateInfoRoot->AppendChildItem(pAttachmentsRootItem);

        // Provide the element used to track the dimension of the pAttachments array.
        pAttachmentsRootItem->SetArraySizeElement(static_cast<rgEditorElementNumeric<uint32_t>*>(pAttachmentCountItem));

        // Initialize the pAttachments rows.
        HandleRenderPassAttachmentCountChanged(pAttachmentsRootItem, pRenderPassCreateInfo, true);

    // Subpass array:
        // Create the subpasses array root item.
        rgEditorElementArray* pSubpassRootItem = new rgEditorElementArray(pRenderPassCreateInfoRoot, STR_VULKAN_RENDER_PASS_P_SUBPASSES,
            [=](int elementIndex)
        {
            // Remove the artificial "resolveAttachmentCount" that was configured per-subpass.
            auto resolveAttachmentCountIter = m_resolveAttachmentCountPerSubpass.find(elementIndex);
            if (resolveAttachmentCountIter != m_resolveAttachmentCountPerSubpass.end())
            {
                // Is the resolveAttachmentCount variable valid?
                uint32_t* pResolveAttachmentCount = resolveAttachmentCountIter->second;
                assert(pResolveAttachmentCount != nullptr);
                if (pResolveAttachmentCount != nullptr)
                {
                    // Destroy the resolveAttachmentCount variable associated with the subpass.
                    RG_SAFE_DELETE(pResolveAttachmentCount);

                    // Erase the variable since the subpass is being destroyed.
                    m_resolveAttachmentCountPerSubpass.erase(resolveAttachmentCountIter);
                }
            }

            // Remove the subpass at the given index.
            RemoveElement(pRenderPassCreateInfo->pSubpasses, pRenderPassCreateInfo->subpassCount, elementIndex);
        });

        // Create the subpassCount member.
        rgEditorElement* pSubpassDescriptionCountItem = MakeNumericElement(nullptr, STR_VULKAN_RENDER_PASS_SUBPASS_COUNT,
            &pRenderPassCreateInfo->subpassCount, [=] { HandleRenderPassSubpassCountChanged(pSubpassRootItem, pRenderPassCreateInfo); });

        // Add the pSubpasses array node.
        pRenderPassCreateInfoRoot->AppendChildItem(pSubpassRootItem);

        // Provide the element used to track the dimension of the pSubpassDescription array.
        pSubpassRootItem->SetArraySizeElement(static_cast<rgEditorElementNumeric<uint32_t>*>(pSubpassDescriptionCountItem));

        // Initialize the pSubpassDescription rows.
        HandleRenderPassSubpassCountChanged(pSubpassRootItem, pRenderPassCreateInfo, true);

    // Dependency array:
        // Create the dependency array root item.
        rgEditorElementArray* pDependenciesRootItem = new rgEditorElementArray(pRenderPassCreateInfoRoot, STR_VULKAN_RENDER_PASS_P_DEPENDENCIES,
            [=](int elementIndex) { RemoveElement(pRenderPassCreateInfo->pDependencies, pRenderPassCreateInfo->dependencyCount, elementIndex); });

        // Create the dependencyCount member.
        rgEditorElement* pDependencyDescriptionCountItem = MakeNumericElement(nullptr, STR_VULKAN_RENDER_PASS_DEPENDENCY_COUNT,
            &pRenderPassCreateInfo->dependencyCount, [=] { HandleRenderPassDependencyCountChanged(pDependenciesRootItem, pRenderPassCreateInfo); });

        // Add the pDependencies array node.
        pRenderPassCreateInfoRoot->AppendChildItem(pDependenciesRootItem);

        // Provide the element used to track the dimension of the pDependencies array.
        pDependenciesRootItem->SetArraySizeElement(static_cast<rgEditorElementNumeric<uint32_t>*>(pDependencyDescriptionCountItem));

        // Initialize the pDependencies rows.
        HandleRenderPassDependencyCountChanged(pDependenciesRootItem, pRenderPassCreateInfo, true);
    }
}

void rgPipelineStateModelVulkan::HandleRenderPassAttachmentCountChanged(rgEditorElement* pRootElement, VkRenderPassCreateInfo* pRenderPassCreateInfo, bool firstInit)
{
    ResizeHandler(pRootElement,
        pRenderPassCreateInfo->attachmentCount,
        pRenderPassCreateInfo->pAttachments,
        STR_VULKAN_RENDER_PASS_ATTACHMENT_DESCRIPTION,
        std::bind(&rgPipelineStateModelVulkan::InitializeRenderPassAttachmentDescriptionCreateInfo, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
        firstInit);
}

void rgPipelineStateModelVulkan::HandleRenderPassSubpassCountChanged(rgEditorElement* pRootElement, VkRenderPassCreateInfo* pRenderPassCreateInfo, bool firstInit)
{
    ResizeHandler(pRootElement,
        pRenderPassCreateInfo->subpassCount,
        pRenderPassCreateInfo->pSubpasses,
        STR_VULKAN_RENDER_PASS_SUBPASS_DESCRIPTION,
        std::bind(&rgPipelineStateModelVulkan::InitializeRenderPassSubpassDescriptionCreateInfo, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
        firstInit);
}

void rgPipelineStateModelVulkan::HandleRenderPassDependencyCountChanged(rgEditorElement* pRootElement, VkRenderPassCreateInfo* pRenderPassCreateInfo, bool firstInit)
{
    ResizeHandler(pRootElement,
        pRenderPassCreateInfo->dependencyCount,
        pRenderPassCreateInfo->pDependencies,
        STR_VULKAN_RENDER_PASS_DEPENDENCY_DESCRIPTION,
        std::bind(&rgPipelineStateModelVulkan::InitializeRenderPassDependencyDescriptionCreateInfo, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
        firstInit);
}

void rgPipelineStateModelVulkan::InitializeRenderPassAttachmentDescriptionCreateInfo(rgEditorElement* pRootElement, VkAttachmentDescription* pBaseAttachmentDescription, int itemIndex)
{
    assert(pRootElement != nullptr);
    assert(pBaseAttachmentDescription != nullptr);
    if (pRootElement != nullptr && pBaseAttachmentDescription != nullptr)
    {
        VkAttachmentDescription* pOffsetAttachmentDescription = pBaseAttachmentDescription + itemIndex;
        assert(pOffsetAttachmentDescription != nullptr);
        if (pOffsetAttachmentDescription != nullptr)
        {
            // Add the "flags" member.
            const rgEnumValuesVector& attachmentDescriptionFlagEnumerators = GetAttachmentDescriptionFlagEnumerators();
            rgEditorElement* pFlagsElement = new rgEditorElementEnum(m_pParent, STR_VULKAN_PIPELINE_MEMBER_FLAGS, attachmentDescriptionFlagEnumerators, reinterpret_cast<uint32_t*>(&pOffsetAttachmentDescription->flags), true);
            pRootElement->AppendChildItem(pFlagsElement);

            // Connect to the enum list widget status signal.
            bool isConnected = connect(pFlagsElement, &rgEditorElement::EnumListWidgetStatusSignal, this, &rgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
            assert(isConnected);

            // Add the "format" member values.
            const rgEnumValuesVector& formatEnumerators = GetFormatEnumerators();
            rgEditorElement* pFormatElement = new rgEditorElementEnum(m_pParent, STR_VULKAN_PIPELINE_MEMBER_VERTEX_FORMAT, formatEnumerators, reinterpret_cast<uint32_t*>(&pOffsetAttachmentDescription->format));
            pRootElement->AppendChildItem(pFormatElement);

            // Connect to the enum list widget status signal.
            isConnected = connect(pFormatElement, &rgEditorElement::EnumListWidgetStatusSignal, this, &rgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
            assert(isConnected);

            // Add the "samples" member.
            rgEditorElement* pSamplesItem = MakeNumericElement(pRootElement, STR_VULKAN_RENDER_PASS_ATTACHMENT_SAMPLES, reinterpret_cast<uint32_t*>(&pOffsetAttachmentDescription->samples));
            pRootElement->AppendChildItem(pSamplesItem);

            // Add the "loadOp" member values.
            const rgEnumValuesVector& loadOpEnumerators = GetAttachmentLoadOpEnumerators();
            rgEditorElement* pLoadOpItem = new rgEditorElementEnum(m_pParent, STR_VULKAN_RENDER_PASS_LOAD_OP, loadOpEnumerators, reinterpret_cast<uint32_t*>(&pOffsetAttachmentDescription->loadOp));
            pRootElement->AppendChildItem(pLoadOpItem);

            // Connect to the enum list widget status signal.
            isConnected = connect(pLoadOpItem, &rgEditorElement::EnumListWidgetStatusSignal, this, &rgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
            assert(isConnected);

            // Add the "storeOp" member values.
            const rgEnumValuesVector& storeOpEnumerators = GetAttachmentStoreOpEnumerators();
            rgEditorElement* pStoreOpItem = new rgEditorElementEnum(m_pParent, STR_VULKAN_RENDER_PASS_STORE_OP, storeOpEnumerators, reinterpret_cast<uint32_t*>(&pOffsetAttachmentDescription->storeOp));
            pRootElement->AppendChildItem(pStoreOpItem);

            // Connect to the enum list widget status signal.
            isConnected = connect(pStoreOpItem, &rgEditorElement::EnumListWidgetStatusSignal, this, &rgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
            assert(isConnected);

            // Add the "stencilLoadOp" member values.
            rgEditorElement* pStencilLoadOpItem = new rgEditorElementEnum(m_pParent, STR_VULKAN_RENDER_PASS_STENCIL_LOAD_OP, loadOpEnumerators, reinterpret_cast<uint32_t*>(&pOffsetAttachmentDescription->stencilLoadOp));
            pRootElement->AppendChildItem(pStencilLoadOpItem);

            // Connect to the enum list widget status signal.
            isConnected = connect(pStencilLoadOpItem, &rgEditorElement::EnumListWidgetStatusSignal, this, &rgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
            assert(isConnected);

            // Add the "stencilStoreOp" member values.
            rgEditorElement* pStencilStoreOpItem = new rgEditorElementEnum(m_pParent, STR_VULKAN_RENDER_PASS_STENCIL_STORE_OP, storeOpEnumerators, reinterpret_cast<uint32_t*>(&pOffsetAttachmentDescription->stencilStoreOp));
            pRootElement->AppendChildItem(pStencilStoreOpItem);

            // Connect to the enum list widget status signal.
            isConnected = connect(pStencilStoreOpItem, &rgEditorElement::EnumListWidgetStatusSignal, this, &rgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
            assert(isConnected);

            // Add the "initialLayout" member values.
            const rgEnumValuesVector& imageLayoutEnumerators = GetImageLayoutEnumerators();
            rgEditorElement* pIinitialLayoutItem = new rgEditorElementEnum(m_pParent, STR_VULKAN_RENDER_PASS_INITIAL_LAYOUT, imageLayoutEnumerators, reinterpret_cast<uint32_t*>(&pOffsetAttachmentDescription->initialLayout));
            pRootElement->AppendChildItem(pIinitialLayoutItem);

            // Connect to the enum list widget status signal.
            isConnected = connect(pIinitialLayoutItem, &rgEditorElement::EnumListWidgetStatusSignal, this, &rgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
            assert(isConnected);

            // Add the "finalLayout" member values.
            rgEditorElement* pFinalLayoutItem = new rgEditorElementEnum(m_pParent, STR_VULKAN_RENDER_PASS_FINAL_LAYOUT, imageLayoutEnumerators, reinterpret_cast<uint32_t*>(&pOffsetAttachmentDescription->finalLayout));
            pRootElement->AppendChildItem(pFinalLayoutItem);

            // Connect to the enum list widget status signal.
            isConnected = connect(pFinalLayoutItem, &rgEditorElement::EnumListWidgetStatusSignal, this, &rgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
            assert(isConnected);
        }
    }
}

void rgPipelineStateModelVulkan::InitializeRenderPassSubpassDescriptionCreateInfo(rgEditorElement* pRootElement, VkSubpassDescription* pBaseSubpassDescription, int itemIndex)
{
    assert(pRootElement != nullptr);
    assert(pBaseSubpassDescription != nullptr);
    if (pRootElement != nullptr && pBaseSubpassDescription != nullptr)
    {
        VkSubpassDescription* pOffsetSubpassDescription = pBaseSubpassDescription + itemIndex;
        assert(pOffsetSubpassDescription != nullptr);
        if (pOffsetSubpassDescription != nullptr)
        {
            // Add the "flags" member.
            const rgEnumValuesVector& subpassDescriptionFlagEnumerators = GetSubpassDescriptionFlagEnumerators();
            rgEditorElement* pFlags = new rgEditorElementEnum(m_pParent, STR_VULKAN_PIPELINE_MEMBER_FLAGS, subpassDescriptionFlagEnumerators, reinterpret_cast<uint32_t*>(&pOffsetSubpassDescription->flags), true);
            pRootElement->AppendChildItem(pFlags);

            // Connect to the enum list widget status signal.
            bool isConnected = connect(pFlags, &rgEditorElement::EnumListWidgetStatusSignal, this, &rgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
            assert(isConnected);

            // Add the "pipelineBindPoint" member.
            const rgEnumValuesVector& pipelineBindPointEnumerators = GetPipelineBindPointEnumerators();
            rgEditorElement* pPipelineBindPointNode = new rgEditorElementEnum(m_pParent, STR_VULKAN_RENDER_PASS_DEPENDENCY_PIPELINE_BIND_POINT, pipelineBindPointEnumerators, reinterpret_cast<uint32_t*>(&pOffsetSubpassDescription->pipelineBindPoint));
            pRootElement->AppendChildItem(pPipelineBindPointNode);

            // Connect to the enum list widget status signal.
            isConnected = connect(pPipelineBindPointNode, &rgEditorElement::EnumListWidgetStatusSignal, this, &rgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
            assert(isConnected);


        // Input attachment array:
            // Create the input attachment array root item.
            rgEditorElementArray* pInputAttachmentsRootItem = new rgEditorElementArray(pRootElement, STR_VULKAN_RENDER_PASS_SUBPASS_P_INPUT_ATTACHMENTS,
                [=](int elementIndex) { RemoveElement(pOffsetSubpassDescription->pInputAttachments, pOffsetSubpassDescription->inputAttachmentCount, elementIndex); });

            // Create the input attachment count member.
            rgEditorElement* pInputAttachmentCountNode = MakeNumericElement(nullptr, STR_VULKAN_RENDER_PASS_SUBPASS_INPUT_ATTACHMENT_COUNT,
                &pOffsetSubpassDescription->inputAttachmentCount, [=] { HandleRenderPassSubpassInputAttachmentCountChanged(pInputAttachmentsRootItem, pOffsetSubpassDescription); });

            // Add the pInputAttachments array node.
            pRootElement->AppendChildItem(pInputAttachmentsRootItem);

            // Provide the element used to track the dimension of the pInputAttachments array.
            pInputAttachmentsRootItem->SetArraySizeElement(static_cast<rgEditorElementNumeric<uint32_t>*>(pInputAttachmentCountNode));

            // Initialize the pInputAttachments rows.
            HandleRenderPassSubpassInputAttachmentCountChanged(pInputAttachmentsRootItem, pOffsetSubpassDescription, true);

        // Color attachment and resolve attachment array:
            // Create the color attachment array root item.
            rgEditorElementArray* pColorAttachmentsRootItem = new rgEditorElementArray(pRootElement, STR_VULKAN_RENDER_PASS_SUBPASS_P_COLOR_ATTACHMENTS,
                [=](int elementIndex) { RemoveElement(pOffsetSubpassDescription->pColorAttachments, pOffsetSubpassDescription->colorAttachmentCount, elementIndex); });

            // Create the color attachment count member.
            rgEditorElement* pColorAttachmentCountNode = MakeNumericElement(nullptr, STR_VULKAN_RENDER_PASS_SUBPASS_COLOR_ATTACHMENT_COUNT,
                &pOffsetSubpassDescription->colorAttachmentCount, [=] { HandleRenderPassSubpassColorAttachmentCountChanged(pColorAttachmentsRootItem, pOffsetSubpassDescription); });

            // Add the pColorAttachments array node.
            pRootElement->AppendChildItem(pColorAttachmentsRootItem);

            // Provide the element used to track the dimension of the pColorAttachments array.
            pColorAttachmentsRootItem->SetArraySizeElement(static_cast<rgEditorElementNumeric<uint32_t>*>(pColorAttachmentCountNode));

            // Initialize the pColorAttachments rows.
            HandleRenderPassSubpassColorAttachmentCountChanged(pColorAttachmentsRootItem, pOffsetSubpassDescription, true);

            int subpassIndex = itemIndex;

            // Create a new resolve attachment count. Each subpass will get its own.
            uint32_t* pResolveAttachmentCount = new uint32_t{};
            m_resolveAttachmentCountPerSubpass[subpassIndex] = pResolveAttachmentCount;

            // If pResolveAttachments is non-null, we can assume that the dimension of the array matches
            // that of colorAttachmentCount. Initialize the count to match the colorAttachmentCount.
            if (pOffsetSubpassDescription->pResolveAttachments != nullptr)
            {
                *pResolveAttachmentCount = pOffsetSubpassDescription->colorAttachmentCount;
            }

        // Resolve attachment array:
            // Create the resolve attachment array root item.
            rgEditorElementArray* pResolveAttachmentsRootItem = new rgEditorElementArray(pRootElement, STR_VULKAN_RENDER_PASS_SUBPASS_P_RESOLVE_ATTACHMENTS,
                [=](int elementIndex)
            {
                // Ensure that the element index
                assert(pResolveAttachmentCount != nullptr);
                if (pResolveAttachmentCount != nullptr)
                {
                    // An array item has been trashed. Decrease the corresponding count item.
                    RemoveElement(pOffsetSubpassDescription->pResolveAttachments, *pResolveAttachmentCount, elementIndex);
                }
            });

            // Create an artificial resolve attachment count member.
            // This member is not part of the VkSubpassDescription structure, but is required since pResolveAttachments array can be NULL, or equal to colorAttachmentCount.
            rgEditorElement* pResolveAttachmentCountNode = MakeNumericElement(nullptr, STR_VULKAN_RENDER_PASS_SUBPASS_RESOLVE_ATTACHMENT_COUNT,
                pResolveAttachmentCount, [=]
            {
                HandleRenderPassSubpassResolveAttachmentCountChanged(subpassIndex, pResolveAttachmentsRootItem, pOffsetSubpassDescription);
            });

            // Add the pResolveAttachments array node.
            pRootElement->AppendChildItem(pResolveAttachmentsRootItem);

            // Provide the element used to track the dimension of the pResolveAttachments array.
            pResolveAttachmentsRootItem->SetArraySizeElement(static_cast<rgEditorElementNumeric<uint32_t>*>(pResolveAttachmentCountNode));

            // Initialize the pResolveAttachments rows.
            HandleRenderPassSubpassResolveAttachmentCountChanged(subpassIndex, pResolveAttachmentsRootItem, pOffsetSubpassDescription, true);

            // Create the depth stencil attachment item.
            rgEditorElement* pDepthStencilAttachmentsRootItem = new rgEditorElement(pRootElement, STR_VULKAN_RENDER_PASS_SUBPASS_P_DEPTH_STENCIL_ATTACHMENT);
            if (pOffsetSubpassDescription->pDepthStencilAttachment != nullptr)
            {
                // Create a copy of the depth stencil attachment that can be modified.
                m_pDepthStencilAttachment = new VkAttachmentReference{};
                memcpy(m_pDepthStencilAttachment, pOffsetSubpassDescription->pDepthStencilAttachment, sizeof(VkAttachmentReference));
                pOffsetSubpassDescription->pDepthStencilAttachment = m_pDepthStencilAttachment;

                // Initialize the tree items with the modifiable attachment pointer.
                InitializeAttachmentReference(pDepthStencilAttachmentsRootItem, m_pDepthStencilAttachment, 0);
            }
            pRootElement->AppendChildItem(pDepthStencilAttachmentsRootItem);

        // Input attachment array:
            // Create the preserveAttachments array root item.
            rgEditorElementArray* pPreserveInputAttachmentsRootItem = new rgEditorElementArray(pRootElement, STR_VULKAN_RENDER_PASS_SUBPASS_P_PRESERVE_ATTACHMENTS,
                [=](int elementIndex) { RemoveElement(pOffsetSubpassDescription->pPreserveAttachments, pOffsetSubpassDescription->preserveAttachmentCount, elementIndex); });

            // Create the preserveAttachment count member.
            rgEditorElement* pPreserveAttachmentCountNode = MakeNumericElement(nullptr, STR_VULKAN_RENDER_PASS_SUBPASS_PRESERVE_ATTACHMENT_COUNT,
                &pOffsetSubpassDescription->preserveAttachmentCount, [=] { HandleRenderPassSubpassPreserveAttachmentCountChanged(pPreserveInputAttachmentsRootItem, pOffsetSubpassDescription); });

            // Add the pPreserveAttachments array node.
            pRootElement->AppendChildItem(pPreserveInputAttachmentsRootItem);

            // Provide the element used to track the dimension of the pPreserveInputAttachments array.
            pPreserveInputAttachmentsRootItem->SetArraySizeElement(static_cast<rgEditorElementNumeric<uint32_t>*>(pPreserveAttachmentCountNode));

            // Initialize the pPreserveInputAttachments rows.
            HandleRenderPassSubpassPreserveAttachmentCountChanged(pPreserveInputAttachmentsRootItem, pOffsetSubpassDescription, true);
        }
    }
}

void rgPipelineStateModelVulkan::HandleRenderPassSubpassInputAttachmentCountChanged(rgEditorElement* pRootElement, VkSubpassDescription* pSubpassCreateInfo, bool firstInit)
{
    ResizeHandler(pRootElement,
        pSubpassCreateInfo->inputAttachmentCount,
        pSubpassCreateInfo->pInputAttachments,
        STR_VULKAN_RENDER_SUBPASS_ATTACHMENT_REFERENCE,
        std::bind(&rgPipelineStateModelVulkan::InitializeAttachmentReference, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
        firstInit);
}

void rgPipelineStateModelVulkan::HandleRenderPassSubpassColorAttachmentCountChanged(rgEditorElement* pRootElement, VkSubpassDescription* pSubpassCreateInfo, bool firstInit)
{
    // Resize the color attachments array.
    ResizeHandler(pRootElement,
        pSubpassCreateInfo->colorAttachmentCount,
        pSubpassCreateInfo->pColorAttachments,
        STR_VULKAN_RENDER_SUBPASS_ATTACHMENT_REFERENCE,
        std::bind(&rgPipelineStateModelVulkan::InitializeAttachmentReference, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
        firstInit);
}

void rgPipelineStateModelVulkan::HandleRenderPassSubpassResolveAttachmentCountChanged(int subpassIndex, rgEditorElement* pRootElement, VkSubpassDescription* pSubpassCreateInfo, bool firstInit)
{
    uint32_t* pSubpassResolveAttachmentCount = m_resolveAttachmentCountPerSubpass.at(subpassIndex);

    assert(pSubpassResolveAttachmentCount != nullptr);
    if (pSubpassResolveAttachmentCount != nullptr)
    {
        // Resize the resolve attachments array.
        ResizeHandler(pRootElement,
            *pSubpassResolveAttachmentCount,
            pSubpassCreateInfo->pResolveAttachments,
            STR_VULKAN_RENDER_SUBPASS_ATTACHMENT_REFERENCE,
            std::bind(&rgPipelineStateModelVulkan::InitializeAttachmentReference, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
            firstInit);
    }
}

void rgPipelineStateModelVulkan::HandleRenderPassSubpassPreserveAttachmentCountChanged(rgEditorElement* pRootElement, VkSubpassDescription* pSubpassCreateInfo, bool firstInit)
{
    // Resize the preserve attachments array.
    ResizeHandler(pRootElement,
        pSubpassCreateInfo->preserveAttachmentCount,
        pSubpassCreateInfo->pPreserveAttachments,
        STR_VULKAN_RENDER_SUBPASS_PRESERVE_ATTACHMENT_ELEMENT_TYPE,
        std::bind(&rgPipelineStateModelVulkan::InitializePreserveAttachment, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
        firstInit);
}

void rgPipelineStateModelVulkan::InitializeAttachmentReference(rgEditorElement* pRootElement, VkAttachmentReference* pBaseAttachmentReference, int itemIndex)
{
    assert(pRootElement != nullptr);
    assert(pBaseAttachmentReference != nullptr);
    if (pRootElement != nullptr && pBaseAttachmentReference != nullptr)
    {
        VkAttachmentReference* pOffsetAttachmentReference = pBaseAttachmentReference + itemIndex;
        assert(pOffsetAttachmentReference != nullptr);
        if (pOffsetAttachmentReference != nullptr)
        {
            // Add the "attachment" member.
            rgEditorElement* pAttachmentNode = MakeNumericElement(pRootElement, STR_VULKAN_RENDER_PASS_SUBPASS_ATTACHMENT_INDEX, &pOffsetAttachmentReference->attachment);
            pRootElement->AppendChildItem(pAttachmentNode);

            // Add the "layout" member node.
            const rgEnumValuesVector& imageLayoutEnumerators = GetImageLayoutEnumerators();
            rgEditorElement* pImageLayoutNode = new rgEditorElementEnum(m_pParent, STR_VULKAN_RENDER_PASS_SUBPASS_ATTACHMENT_LAYOUT, imageLayoutEnumerators, reinterpret_cast<uint32_t*>(&pOffsetAttachmentReference->layout));
            pRootElement->AppendChildItem(pImageLayoutNode);

            // Connect to the enum list widget status signal.
            bool isConnected = connect(pImageLayoutNode, &rgEditorElement::EnumListWidgetStatusSignal, this, &rgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
            assert(isConnected);
        }
    }
}

void rgPipelineStateModelVulkan::InitializePreserveAttachment(rgEditorElement* pRootElement, uint32_t* pBaseReserveAttachment, int itemIndex)
{
    assert(pRootElement != nullptr);
    assert(pBaseReserveAttachment != nullptr);
    if (pRootElement != nullptr && pBaseReserveAttachment != nullptr)
    {
        uint32_t* pOffsetReserveAttachment = pBaseReserveAttachment + itemIndex;
        assert(pOffsetReserveAttachment != nullptr);
        if (pOffsetReserveAttachment != nullptr)
        {
            // Add the "pReserveAttachment" member.
            rgEditorElement* pPreserveAttachmentNode = MakeNumericElement(pRootElement, STR_VULKAN_RENDER_PASS_PRESERVE_ATTACHMENT, reinterpret_cast<uint32_t*>(pOffsetReserveAttachment));
            pRootElement->AppendChildItem(pPreserveAttachmentNode);
        }
    }
}

void rgPipelineStateModelVulkan::InitializeRenderPassDependencyDescriptionCreateInfo(rgEditorElement* pRootElement, VkSubpassDependency* pBaseDependencyDescription, int itemIndex)
{
    assert(pRootElement != nullptr);
    assert(pBaseDependencyDescription != nullptr);
    if (pRootElement != nullptr && pBaseDependencyDescription != nullptr)
    {
        VkSubpassDependency* pOffsetDependencyDescription = pBaseDependencyDescription + itemIndex;
        assert(pOffsetDependencyDescription != nullptr);
        if (pOffsetDependencyDescription != nullptr)
        {
            // Add the "srcSubpass" member.
            rgEditorElement* pSrcSubpass = MakeNumericElement(pRootElement, STR_VULKAN_RENDER_PASS_DEPENDENCY_SRC_SUBPASS, reinterpret_cast<uint32_t*>(&pOffsetDependencyDescription->srcSubpass));
            pRootElement->AppendChildItem(pSrcSubpass);

            // Add the "dstSubpass" member.
            rgEditorElement* pDstSubpass = MakeNumericElement(pRootElement, STR_VULKAN_RENDER_PASS_DEPENDENCY_DST_SUBPASS, reinterpret_cast<uint32_t*>(&pOffsetDependencyDescription->dstSubpass));
            pRootElement->AppendChildItem(pDstSubpass);

            // Get the stage flag enumerators.
            const rgEnumValuesVector& stageFlagEnumerators = GetPipelineStageFlagEnumerators();

            // Add the "srcStageMask" member.
            rgEditorElement* pSrcStageMask = new rgEditorElementEnum(m_pParent, STR_VULKAN_RENDER_PASS_DEPENDENCY_SRC_STAGE_MASK, stageFlagEnumerators, reinterpret_cast<uint32_t*>(&pOffsetDependencyDescription->srcStageMask), true);
            pRootElement->AppendChildItem(pSrcStageMask);

            // Connect to the enum list widget status signal.
            bool isConnected = connect(pSrcStageMask, &rgEditorElement::EnumListWidgetStatusSignal, this, &rgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
            assert(isConnected);

            // Add the "dstStageMask" member.
            rgEditorElement* pDstStageMask = new rgEditorElementEnum(m_pParent, STR_VULKAN_RENDER_PASS_DEPENDENCY_DST_STAGE_MASK, stageFlagEnumerators, reinterpret_cast<uint32_t*>(&pOffsetDependencyDescription->dstStageMask), true);
            pRootElement->AppendChildItem(pDstStageMask);

            // Connect to the enum list widget status signal.
            isConnected = connect(pDstStageMask, &rgEditorElement::EnumListWidgetStatusSignal, this, &rgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
            assert(isConnected);

            // Get the access flag enumerators.
            const rgEnumValuesVector& accessFlagEnumerators = GetAccessFlagEnumerators();

            // Add the "srcAccessMask" member.
            rgEditorElement* pSrcAccessMask = new rgEditorElementEnum(m_pParent, STR_VULKAN_RENDER_PASS_DEPENDENCY_SRC_ACCESS_MASK, accessFlagEnumerators, reinterpret_cast<uint32_t*>(&pOffsetDependencyDescription->srcAccessMask), true);
            pRootElement->AppendChildItem(pSrcAccessMask);

            // Connect to the enum list widget status signal.
            isConnected = connect(pSrcAccessMask, &rgEditorElement::EnumListWidgetStatusSignal, this, &rgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
            assert(isConnected);

            // Add the "dstAccessMask" member.
            rgEditorElement* pDstAccessMask = new rgEditorElementEnum(m_pParent, STR_VULKAN_RENDER_PASS_DEPENDENCY_DST_ACCESS_MASK, accessFlagEnumerators, reinterpret_cast<uint32_t*>(&pOffsetDependencyDescription->dstAccessMask), true);
            pRootElement->AppendChildItem(pDstAccessMask);

            // Connect to the enum list widget status signal.
            isConnected = connect(pDstAccessMask, &rgEditorElement::EnumListWidgetStatusSignal, this, &rgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
            assert(isConnected);

            // Add the "dependencyFlags" member.
            const rgEnumValuesVector& dependencyFlagEnumerators = GetDependencyFlagEnumerators();
            rgEditorElement* pDependencyFlags = new rgEditorElementEnum(m_pParent, STR_VULKAN_RENDER_PASS_DEPENDENCY_DEPENDENCY_FLAGS, dependencyFlagEnumerators, reinterpret_cast<uint32_t*>(&pOffsetDependencyDescription->dependencyFlags), true);
            pRootElement->AppendChildItem(pDependencyFlags);

            // Connect to the enum list widget status signal.
            isConnected = connect(pDependencyFlags, &rgEditorElement::EnumListWidgetStatusSignal, this, &rgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
            assert(isConnected);
        }
    }
}
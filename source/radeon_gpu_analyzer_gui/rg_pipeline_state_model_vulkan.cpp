//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for Pso model for vulkan.
//=============================================================================

// C++.
#include <cassert>
#include <sstream>

// Infra.
#include "qt_common/utils/scaling_manager.h"

// Utils.
#include "source/common/vulkan/rg_pso_factory_vulkan.h"
#include "source/common/vulkan/rg_pso_serializer_vulkan.h"

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_build_view_vulkan.h"
#include "radeon_gpu_analyzer_gui/qt/rg_pipeline_state_model_vulkan.h"
#include "radeon_gpu_analyzer_gui/qt/rg_editor_element.h"
#include "radeon_gpu_analyzer_gui/qt/rg_editor_element_array_element_add.h"
#include "radeon_gpu_analyzer_gui/qt/rg_editor_element_bool.h"
#include "radeon_gpu_analyzer_gui/qt/rg_editor_element_enum.h"
#include "radeon_gpu_analyzer_gui/qt/rg_editor_element_numeric.h"
#include "radeon_gpu_analyzer_gui/rg_data_types.h"
#include "radeon_gpu_analyzer_gui/rg_data_types_vulkan.h"
#include "radeon_gpu_analyzer_gui/rg_definitions.h"
#include "radeon_gpu_analyzer_gui/rg_object_names.h"
#include "radeon_gpu_analyzer_gui/rg_string_constants.h"
#include "radeon_gpu_analyzer_gui/rg_utils.h"

static const RgEnumValuesVector& GetFormatEnumerators()
{
    static RgEnumValuesVector format_values = {
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

    return format_values;
}

static const RgEnumValuesVector& GetPipelineCreateFlagEnumerators()
{
    static RgEnumValuesVector pipeline_create_flag_enumerators = {
        ENUM_VALUE(VK_PIPELINE_CREATE_DISABLE_OPTIMIZATION_BIT),
        ENUM_VALUE(VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT),
        ENUM_VALUE(VK_PIPELINE_CREATE_DERIVATIVE_BIT),
        ENUM_VALUE(VK_PIPELINE_CREATE_VIEW_INDEX_FROM_DEVICE_INDEX_BIT),
        ENUM_VALUE(VK_PIPELINE_CREATE_DISPATCH_BASE),
        ENUM_VALUE(VK_PIPELINE_CREATE_DEFER_COMPILE_BIT_NV),
        ENUM_VALUE(VK_PIPELINE_CREATE_VIEW_INDEX_FROM_DEVICE_INDEX_BIT_KHR),
        ENUM_VALUE(VK_PIPELINE_CREATE_DISPATCH_BASE_KHR),
    };

    return pipeline_create_flag_enumerators;
}

static const RgEnumValuesVector& GetDescriptorSetLayoutCreateFlagEnumerators()
{
    static RgEnumValuesVector descriptor_set_layout_create_flag_enumerators = {
        ENUM_VALUE(VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR),
        ENUM_VALUE(VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT)
    };

    return descriptor_set_layout_create_flag_enumerators;
}

static const RgEnumValuesVector& GetAttachmentDescriptionFlagEnumerators()
{
    static RgEnumValuesVector attachment_description_flag_enumerators = {
        ENUM_VALUE(VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT),
    };

    return attachment_description_flag_enumerators;
}

static const RgEnumValuesVector& GetPrimitiveTopologyEnumerators()
{
    static RgEnumValuesVector primitive_topology_enumerators = {
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

    return primitive_topology_enumerators;
}

static const RgEnumValuesVector& GetAttachmentLoadOpEnumerators()
{
    static RgEnumValuesVector load_op_enumerators = {
        ENUM_VALUE(VK_ATTACHMENT_LOAD_OP_LOAD),
        ENUM_VALUE(VK_ATTACHMENT_LOAD_OP_CLEAR),
        ENUM_VALUE(VK_ATTACHMENT_LOAD_OP_DONT_CARE)
    };

    return load_op_enumerators;
}

static const RgEnumValuesVector& GetAttachmentStoreOpEnumerators()
{
    static RgEnumValuesVector store_op_enumerators = {
        ENUM_VALUE(VK_ATTACHMENT_STORE_OP_STORE),
        ENUM_VALUE(VK_ATTACHMENT_STORE_OP_DONT_CARE),
    };

    return store_op_enumerators;
}

static const RgEnumValuesVector& GetImageLayoutEnumerators()
{
    static RgEnumValuesVector image_layout_enumerators = {
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

    return image_layout_enumerators;
}

static const RgEnumValuesVector& GetPolygonModeEnumerators()
{
    static RgEnumValuesVector polygon_mode_enumerators = {
        ENUM_VALUE(VK_POLYGON_MODE_FILL),
        ENUM_VALUE(VK_POLYGON_MODE_LINE),
        ENUM_VALUE(VK_POLYGON_MODE_POINT),
        ENUM_VALUE(VK_POLYGON_MODE_FILL_RECTANGLE_NV),
    };

    return polygon_mode_enumerators;
}

static const RgEnumValuesVector& GetCullModeFlagEnumerators()
{
    static RgEnumValuesVector cull_mode_flag_enumerators = {
        ENUM_VALUE(VK_CULL_MODE_NONE),
        ENUM_VALUE(VK_CULL_MODE_FRONT_BIT),
        ENUM_VALUE(VK_CULL_MODE_BACK_BIT),
        ENUM_VALUE(VK_CULL_MODE_FRONT_AND_BACK),
    };

    return cull_mode_flag_enumerators;
}

static const RgEnumValuesVector& GetFrontFaceEnumerators()
{
    static RgEnumValuesVector front_face_enumerators = {
        ENUM_VALUE(VK_FRONT_FACE_COUNTER_CLOCKWISE),
        ENUM_VALUE(VK_FRONT_FACE_CLOCKWISE),
    };

    return front_face_enumerators;
}

static const RgEnumValuesVector& GetRasterizationSamplesEnumerators()
{
    static RgEnumValuesVector rasterization_samples_flags = {
        ENUM_VALUE(VK_SAMPLE_COUNT_1_BIT),
        ENUM_VALUE(VK_SAMPLE_COUNT_2_BIT),
        ENUM_VALUE(VK_SAMPLE_COUNT_4_BIT),
        ENUM_VALUE(VK_SAMPLE_COUNT_8_BIT),
        ENUM_VALUE(VK_SAMPLE_COUNT_16_BIT),
        ENUM_VALUE(VK_SAMPLE_COUNT_32_BIT),
        ENUM_VALUE(VK_SAMPLE_COUNT_64_BIT),
    };

    return rasterization_samples_flags;
}

static const RgEnumValuesVector& GetCompareOpEnumerators()
{
    static RgEnumValuesVector compare_op_enumerators = {
        ENUM_VALUE(VK_COMPARE_OP_NEVER),
        ENUM_VALUE(VK_COMPARE_OP_LESS),
        ENUM_VALUE(VK_COMPARE_OP_EQUAL),
        ENUM_VALUE(VK_COMPARE_OP_LESS_OR_EQUAL),
        ENUM_VALUE(VK_COMPARE_OP_GREATER),
        ENUM_VALUE(VK_COMPARE_OP_NOT_EQUAL),
        ENUM_VALUE(VK_COMPARE_OP_GREATER_OR_EQUAL),
        ENUM_VALUE(VK_COMPARE_OP_ALWAYS),
    };

    return compare_op_enumerators;
}

static const RgEnumValuesVector& GetStencilOpEnumerators()
{
    static RgEnumValuesVector stencil_op_enumerators = {
        ENUM_VALUE(VK_STENCIL_OP_KEEP),
        ENUM_VALUE(VK_STENCIL_OP_ZERO),
        ENUM_VALUE(VK_STENCIL_OP_REPLACE),
        ENUM_VALUE(VK_STENCIL_OP_INCREMENT_AND_CLAMP),
        ENUM_VALUE(VK_STENCIL_OP_DECREMENT_AND_CLAMP),
        ENUM_VALUE(VK_STENCIL_OP_INVERT),
        ENUM_VALUE(VK_STENCIL_OP_INCREMENT_AND_WRAP),
        ENUM_VALUE(VK_STENCIL_OP_DECREMENT_AND_WRAP),
    };

    return stencil_op_enumerators;
}

static const RgEnumValuesVector& GetLogicOpEnumerators()
{
    static RgEnumValuesVector logic_op_enumerators = {
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

    return logic_op_enumerators;
}

static const RgEnumValuesVector& GetBlendFactorEnumerators()
{
    static RgEnumValuesVector blend_factor_enumerators = {
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

    return blend_factor_enumerators;
}

static const RgEnumValuesVector& GetBlendOpEnumerators()
{
    static RgEnumValuesVector blend_op_enumerators = {
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

    return blend_op_enumerators;
}

static const RgEnumValuesVector& GetColorComponentFlagEnumerators()
{
    static RgEnumValuesVector color_component_flag_enumerators = {
        ENUM_VALUE(VK_COLOR_COMPONENT_R_BIT),
        ENUM_VALUE(VK_COLOR_COMPONENT_G_BIT),
        ENUM_VALUE(VK_COLOR_COMPONENT_B_BIT),
        ENUM_VALUE(VK_COLOR_COMPONENT_A_BIT),
    };

    return color_component_flag_enumerators;
}

static const RgEnumValuesVector& GetShaderStageFlagEnumerators()
{
    static RgEnumValuesVector stage_flag_enumerators = {
        ENUM_VALUE(VK_SHADER_STAGE_VERTEX_BIT),
        ENUM_VALUE(VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT),
        ENUM_VALUE(VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT),
        ENUM_VALUE(VK_SHADER_STAGE_GEOMETRY_BIT),
        ENUM_VALUE(VK_SHADER_STAGE_FRAGMENT_BIT),
        ENUM_VALUE(VK_SHADER_STAGE_COMPUTE_BIT),
        ENUM_VALUE(VK_SHADER_STAGE_ALL_GRAPHICS),
    };

    return stage_flag_enumerators;
}

static const RgEnumValuesVector& GetDescriptorTypeEnumerators()
{
    static RgEnumValuesVector descriptor_type_enumerators = {
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

    return descriptor_type_enumerators;
}

static const RgEnumValuesVector& GetPipelineStageFlagEnumerators()
{
    static RgEnumValuesVector pipeline_stage_flag_enumerators = {
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
    };

    return pipeline_stage_flag_enumerators;
}

static const RgEnumValuesVector& GetAccessFlagEnumerators()
{
    static RgEnumValuesVector access_flag_enumerators = {
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
        ENUM_VALUE(VK_ACCESS_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT_EXT),
    };

    return access_flag_enumerators;
}

static const RgEnumValuesVector& GetDependencyFlagEnumerators()
{
    static RgEnumValuesVector dependency_flag_enumerators = {
        ENUM_VALUE(VK_DEPENDENCY_BY_REGION_BIT),
        ENUM_VALUE(VK_DEPENDENCY_DEVICE_GROUP_BIT),
        ENUM_VALUE(VK_DEPENDENCY_VIEW_LOCAL_BIT),
        ENUM_VALUE(VK_DEPENDENCY_VIEW_LOCAL_BIT_KHR),
        ENUM_VALUE(VK_DEPENDENCY_DEVICE_GROUP_BIT_KHR),
    };

    return dependency_flag_enumerators;
}

static const RgEnumValuesVector& GetSubpassDescriptionFlagEnumerators()
{
    static RgEnumValuesVector subpass_description_flag_enumerators = {
        ENUM_VALUE(VK_SUBPASS_DESCRIPTION_PER_VIEW_ATTRIBUTES_BIT_NVX),
        ENUM_VALUE(VK_SUBPASS_DESCRIPTION_PER_VIEW_POSITION_X_ONLY_BIT_NVX),
    };

    return subpass_description_flag_enumerators;
}

static const RgEnumValuesVector& GetPipelineBindPointEnumerators()
{
    static RgEnumValuesVector pipeline_bind_point_enumerators = {
        ENUM_VALUE(VK_PIPELINE_BIND_POINT_GRAPHICS),
        ENUM_VALUE(VK_PIPELINE_BIND_POINT_COMPUTE),
    };

    return pipeline_bind_point_enumerators;
}

RgPipelineStateModelVulkan::RgPipelineStateModelVulkan(QWidget* parent)
    : RgPipelineStateModel(parent)
    , descriptor_set_layout_count_(0)
{
}

RgPipelineStateModelVulkan::~RgPipelineStateModelVulkan()
{
    RG_SAFE_DELETE(flags_);
    RG_SAFE_DELETE(alpha_blend_op_node_);
    RG_SAFE_DELETE(color_component_write_mask_);
    RG_SAFE_DELETE(color_blend_op_node_);
    RG_SAFE_DELETE(compare_op_node_);
    RG_SAFE_DELETE(cull_mode_node_);
    RG_SAFE_DELETE(dependency_flags_);
    RG_SAFE_DELETE(depth_compare_op_node_);
    RG_SAFE_DELETE(depth_fail_op_);
    RG_SAFE_DELETE(descriptor_set_layout_flags_node_);
    RG_SAFE_DELETE(descriptor_type_node_);
    RG_SAFE_DELETE(dst_access_mask_);
    RG_SAFE_DELETE(dst_alpha_blend_factor_node_);
    RG_SAFE_DELETE(dst_color_blend_factor_node_);
    RG_SAFE_DELETE(dst_stage_mask_);
    RG_SAFE_DELETE(fail_op_node_);
    RG_SAFE_DELETE(final_layout_item_);
    RG_SAFE_DELETE(flags_element_);
    RG_SAFE_DELETE(flags_item_);
    RG_SAFE_DELETE(flags_item1_);
    RG_SAFE_DELETE(format_element_);
    RG_SAFE_DELETE(format_element1_);
    RG_SAFE_DELETE(front_face_node_);
    RG_SAFE_DELETE(image_layout_node_);
    RG_SAFE_DELETE(initial_layout_item_);
    RG_SAFE_DELETE(input_rate_element_);
    RG_SAFE_DELETE(load_op_item_);
    RG_SAFE_DELETE(logic_op_node_);
    RG_SAFE_DELETE(pass_op_node_);
    RG_SAFE_DELETE(pipeline_bind_point_node);
    RG_SAFE_DELETE(polygon_mode_node_);
    RG_SAFE_DELETE(rasterization_samples_);
    RG_SAFE_DELETE(src_access_mask_);
    RG_SAFE_DELETE(src_alpha_blend_factor_node_);
    RG_SAFE_DELETE(src_color_blend_factor_node_);
    RG_SAFE_DELETE(src_stage_mask_);
    RG_SAFE_DELETE(stage_flags_node_);
    RG_SAFE_DELETE(stage_flags_node1_);
    RG_SAFE_DELETE(pipeline_bind_point_node_);
    RG_SAFE_DELETE(stencil_load_op_item_);
    RG_SAFE_DELETE(stencil_store_op_item_);
    RG_SAFE_DELETE(store_op_item_);
    RG_SAFE_DELETE(topology_item_);
    for (RgEditorElement* element : front_stencil_op_state_node_)
    {
        if (element != nullptr)
        {
            RG_SAFE_DELETE(element);
        }
    }
    front_stencil_op_state_node_.clear();

    for (RgEditorElement* element : back_stencil_op_state_node_)
    {
        if (element != nullptr)
        {
            RG_SAFE_DELETE(element);
        }
    }
    back_stencil_op_state_node_.clear();
}

uint32_t GetSampleMaskDimension(VkSampleCountFlagBits sample_count_bits)
{
    uint32_t enum_dimension = 0;
    switch (sample_count_bits)
    {
    case VK_SAMPLE_COUNT_1_BIT:
    case VK_SAMPLE_COUNT_2_BIT:
    case VK_SAMPLE_COUNT_4_BIT:
    case VK_SAMPLE_COUNT_8_BIT:
    case VK_SAMPLE_COUNT_16_BIT:
    case VK_SAMPLE_COUNT_32_BIT:
        // One 32-bit uint_32 is used to hold 1 to 32 bit flags.
        enum_dimension = 1;
        break;
    case VK_SAMPLE_COUNT_64_BIT:
        // Two 32-bit uint_32's are used to hold the 64 bit flags.
        enum_dimension = 2;
        break;
    default:
        assert(false);
    }

    return enum_dimension;
}

bool RgPipelineStateModelVulkan::CheckValidPipelineState(std::string& error_string) const
{
    bool ret = false;

    std::stringstream error_stream;

    // Choose how to validate the PSO state based on the pipeline type.
    if (pipeline_type_ == RgPipelineType::kGraphics)
    {
        // Verify that if the number of resolve attachments is non-null, the dimension of
        // pResolveAttachments matches that of pColorAttachments.
        VkRenderPassCreateInfo* render_pass_create_info = graphics_pipeline_state_->GetRenderPassCreateInfo();
        assert(render_pass_create_info != nullptr);
        if (render_pass_create_info != nullptr)
        {
            bool resolve_attachments_compatible = true;

            // Step through each subpass.
            for (uint32_t subpass_index = 0; subpass_index < render_pass_create_info->subpassCount; ++subpass_index)
            {
                assert(render_pass_create_info->pSubpasses != nullptr);
                if (render_pass_create_info->pSubpasses != nullptr)
                {
                    // Step to the nth item in the subpass array.
                    const VkSubpassDescription* subpass = (render_pass_create_info->pSubpasses + subpass_index);
                    assert(subpass != nullptr);
                    if (subpass != nullptr)
                    {
                        // Is the pResolveAttachments array used? If so, verify the dimension matches the
                        // number of color attachments.
                        if (subpass->pResolveAttachments != nullptr)
                        {
                            uint32_t* subpass_resolve_attachment_count = resolve_attachment_count_per_subpass_.at(subpass_index);
                            if (subpass_resolve_attachment_count != nullptr)
                            {
                                // Does the number of resolve attachments match the number of color attachments?
                                assert(subpass->colorAttachmentCount == *subpass_resolve_attachment_count);
                                if (subpass->colorAttachmentCount != *subpass_resolve_attachment_count)
                                {
                                    // Build an error string that mentions the index of the problematic subpass.
                                    error_stream << kStrErrorResolveAttachmentsInvalidA;
                                    error_stream << subpass_index;
                                    error_stream << kStrErrorResolveAttachmentsInvalidB;
                                    error_stream << " ";
                                    error_stream << kStrErrorResolveAttachmentsInvalidC;

                                    // Return false, and exit out of the loop. Validation failed.
                                    resolve_attachments_compatible = false;
                                    break;
                                }
                            }
                        }
                    }
                }
            }

            ret = resolve_attachments_compatible;
        }

        VkPipelineMultisampleStateCreateInfo* multisampling_state_create_info =
            graphics_pipeline_state_->GetPipelineMultisampleStateCreateInfo();
        assert(multisampling_state_create_info != nullptr);
        if (multisampling_state_create_info != nullptr)
        {
            bool is_multisampling_sample_mask_valid = false;

            // If the multisampling state's pSampleMask is enabled, verify that the dimension
            // of the array is compatible with the rasterizationSamples state.
            if (multisampling_state_create_info->pSampleMask != nullptr)
            {
                uint32_t enum_dimension = GetSampleMaskDimension(multisampling_state_create_info->rasterizationSamples);

                // If the pSampleMask is used, the dimension should be non-zero.
                assert(enum_dimension != 0);
                if (enum_dimension != 0)
                {
                    // The pSampleMask array is valid if the rasterizationSamples enum
                    // matches the dimension that the user has set.
                    is_multisampling_sample_mask_valid = enum_dimension == sample_mask_dimension_;

                    if (!is_multisampling_sample_mask_valid)
                    {
                        // Construct an error message indicating why the pipeline is valid.
                        error_stream << kStrErrorMultisamplingSampleMaskInvalidA;
                        error_stream << std::endl;
                        error_stream << kStrErrorMultisamplingSampleMaskInvalidB;
                    }
                }
            }
            else
            {
                // Multisampling state doesn't need to be verified further if pSampleMask is null.
                is_multisampling_sample_mask_valid = true;
            }

            // Update the return value based on the mask validity.
            ret = ret && is_multisampling_sample_mask_valid;
        }
    }
    else if (pipeline_type_ == RgPipelineType::kCompute)
    {
        // Nothing needs to be validated for a compute pipeline. Just return true each time.
        ret = true;
    }
    else
    {
        // The pipeline type couldn't be determined. This shouldn't happen.
        assert(false && "Failed to validate pipeline because the pipeline type could not be determined.");
    }

    error_string = error_stream.str();

    return ret;
}

void RgPipelineStateModelVulkan::InitializeDefaultGraphicsPipeline()
{
    std::shared_ptr<RgPsoFactoryVulkan> pipeline_factory = std::make_shared<RgPsoFactoryVulkan>();

    RgPsoGraphicsVulkan* graphics_pso_state = pipeline_factory->GetDefaultGraphicsPsoCreateInfo();
    assert(graphics_pso_state != nullptr);
    if (graphics_pso_state != nullptr)
    {
        // Configure the pipeline's default blend state to be compatible with the default shader code.
        VkPipelineColorBlendAttachmentState* color_blend_attachment = new VkPipelineColorBlendAttachmentState{};
        color_blend_attachment->colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        color_blend_attachment->blendEnable = VK_FALSE;
        color_blend_attachment->srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        color_blend_attachment->dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        color_blend_attachment->colorBlendOp = VK_BLEND_OP_ADD;
        color_blend_attachment->srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        color_blend_attachment->dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        color_blend_attachment->alphaBlendOp = VK_BLEND_OP_ADD;

        // Add a default color blend create info state.
        VkPipelineColorBlendStateCreateInfo* color_blend_state_info = graphics_pso_state->GetPipelineColorBlendStateCreateInfo();
        color_blend_state_info->pAttachments = color_blend_attachment;
        color_blend_state_info->attachmentCount = 1;

        // Assign the default state in the model.
        graphics_pipeline_state_ = graphics_pso_state;
    }
}

void RgPipelineStateModelVulkan::InitializeDefaultComputePipeline()
{
    std::shared_ptr<RgPsoFactoryVulkan> pipeline_factory = std::make_shared<RgPsoFactoryVulkan>();

    // Configure a compute pipeline state with default configuration.
    RgPsoComputeVulkan* compute_pso_state = pipeline_factory->GetDefaultComputePsoCreateInfo();
    assert(compute_pso_state != nullptr);
    if (compute_pso_state != nullptr)
    {
        // Assign the default state in the model.
        compute_pipeline_state_ = compute_pso_state;
    }
}

void RgPipelineStateModelVulkan::InitializeDescriptorSetLayoutCreateInfoArray(RgEditorElement* root_element, RgPsoCreateInfoVulkan* create_info)
{
    // Create the root element for the Descriptor Set Layout array configuration.
    RgEditorElement* descriptor_set_layouts_root = new RgEditorElement(root_element, kStrVulkanDescriptorSetLayoutsHeader);

    // Set the object name.
    descriptor_set_layouts_root->setObjectName(kStrDescriptorSetLayoutsRoot);

    // Display the type name with brackets to indicate that it's an array.
    std::stringstream descriptor_set_layouts_array_stream;
    descriptor_set_layouts_array_stream << kStrVulkanDescriptorSetLayoutCreateInfo << "[]";

    std::vector<VkDescriptorSetLayoutCreateInfo*>& descriptor_set_layouts = create_info->GetDescriptorSetLayoutCreateInfo();

    // Create the Descriptor Set Layout array root item.
    RgEditorElementArrayElementAdd* descriptor_set_layouts_item = new RgEditorElementArrayElementAdd(descriptor_set_layouts_root, descriptor_set_layouts_array_stream.str().c_str(),
        [=](int element_index)
    {
        std::vector<VkDescriptorSetLayoutCreateInfo*>& current_descriptor_set_layouts = create_info->GetDescriptorSetLayoutCreateInfo();
        int element_count = static_cast<int>(current_descriptor_set_layouts.size());

        // If we remove the given element, is it necessary to shift other subsequent elements?
        if (element_count > 1 && element_index < element_count - 1)
        {
            // Shift all elements after the removed element.
            for (int copy_index = element_index; copy_index < (element_count - 1); ++copy_index)
            {
                memcpy(*(current_descriptor_set_layouts.data() + copy_index), *(current_descriptor_set_layouts.data() + (copy_index + 1)), sizeof(VkDescriptorSetLayoutCreateInfo));
            }
        }
    });

    // Set the object name.
    descriptor_set_layouts_item->setObjectName(kStrDescriptorSetLayoutsItem);

    // Initialize the Descriptor Set Layout count.
    descriptor_set_layout_count_ = static_cast<uint32_t>(descriptor_set_layouts.size());

    // Create the Descriptor Set Layout count item.
    RgEditorElement* descriptor_set_layout_count_item = MakeNumericElement(nullptr, kStrVulkanDescriptorSetLayoutCount,
        &descriptor_set_layout_count_, [=] { HandleDescriptorSetLayoutCountChanged(descriptor_set_layouts_item, create_info); });

    // Set the object name.
    descriptor_set_layout_count_item->setObjectName(kStrDescriptorSetLayoutCountItem);

    // Provide the element used to track the dimension of the Descriptor Set Layouts array.
    descriptor_set_layouts_item->SetArraySizeElement(static_cast<RgEditorElementNumeric<uint32_t>*>(descriptor_set_layout_count_item));

    // Initialize the Descriptor Set Layouts array rows.
    HandleDescriptorSetLayoutCountChanged(descriptor_set_layouts_item, create_info, true);

    // Add the Descriptor Set Layouts create info array item.
    descriptor_set_layouts_root->AppendChildItem(descriptor_set_layouts_item);

    // Add the Descriptor Set Layout section.
    root_element->AppendChildItem(descriptor_set_layouts_root);
}

RgEditorElement* RgPipelineStateModelVulkan::InitializeGraphicsPipelineCreateInfo(QWidget* parent)
{
    RgEditorElement* create_info_root_node = nullptr;

    assert(graphics_pipeline_state_ != nullptr);
    if (graphics_pipeline_state_ != nullptr)
    {
        // Create the root node that all create info elements are attached to.
        create_info_root_node = new RgEditorElement(parent, kStrVulkanGraphicsPipelineState);

        // Set object name.
        create_info_root_node->setObjectName(kStrCreateInfoRootNode);

        // Add the Graphics Pipeline create info root node.
        InitializeVkGraphicsPipelineCreateInfo(create_info_root_node, graphics_pipeline_state_);

        // Add the Pipeline Layout create info root node.
        VkPipelineLayoutCreateInfo* pipeline_layout_create_info = graphics_pipeline_state_->GetPipelineLayoutCreateInfo();
        assert(pipeline_layout_create_info != nullptr);
        if (pipeline_layout_create_info != nullptr)
        {
            InitializePipelineLayoutCreateInfo(create_info_root_node, pipeline_layout_create_info);
        }

        // Initialize rows related to Descriptor Set Layout configuration.
        InitializeDescriptorSetLayoutCreateInfoArray(create_info_root_node, graphics_pipeline_state_);

        // Add the Render Pass create info root node.
        VkRenderPassCreateInfo* render_pass_create_info = graphics_pipeline_state_->GetRenderPassCreateInfo();
        assert(render_pass_create_info != nullptr);
        if (render_pass_create_info != nullptr)
        {
            InitializeRenderPassCreateInfo(create_info_root_node, render_pass_create_info);
        }
    }

    return create_info_root_node;
}

void RgPipelineStateModelVulkan::HandleDescriptorSetLayoutCountChanged(RgEditorElement* root_element, RgPsoCreateInfoVulkan* create_info, bool first_init)
{
    int num_existing_elements = root_element->ChildCount();
    int new_element_count = static_cast<int32_t>(descriptor_set_layout_count_);

    if (new_element_count != num_existing_elements)
    {
        if (first_init)
        {
            num_existing_elements = new_element_count;
        }

        std::vector<VkDescriptorSetLayoutCreateInfo*>& descriptor_set_layouts = create_info->GetDescriptorSetLayoutCreateInfo();

        RgEditorElementArrayElementAdd* array_root = static_cast<RgEditorElementArrayElementAdd*>(root_element);
        if (array_root != nullptr)
        {
            if (new_element_count == 0)
            {
                for (size_t element_index = 0; element_index < descriptor_set_layouts.size(); ++element_index)
                {
                    RG_SAFE_DELETE(descriptor_set_layouts[element_index]);
                }

                root_element->ClearChildren();
                descriptor_set_layouts.clear();

                // Let the array root element know that the child elements were resized.
                array_root->InvokeElementResizedCallback();
            }
            else
            {
                std::vector<VkDescriptorSetLayoutCreateInfo*> new_descriptor_set_layouts;
                for (size_t element_index = 0; element_index < new_element_count; ++element_index)
                {
                    // Create an array of object with the new dimension.
                    VkDescriptorSetLayoutCreateInfo* new_element = new VkDescriptorSetLayoutCreateInfo{};
                    new_descriptor_set_layouts.push_back(new_element);
                }

                // If the element count was increased, copy all old contents.
                // If it was reduced, copy as many as will fit in the new array.
                uint32_t element_count = new_element_count > num_existing_elements ? num_existing_elements : new_element_count;

                // Copy existing element data into the resized elements.
                for (uint32_t index = 0; index < element_count; ++index)
                {
                    // Copy all member data into the new array elements.
                    memcpy(new_descriptor_set_layouts[index], descriptor_set_layouts[index], sizeof(VkDescriptorSetLayoutCreateInfo));
                }

                // Destroy the original array element data.
                for (size_t element_index = 0; element_index < descriptor_set_layouts.size(); ++element_index)
                {
                    RG_SAFE_DELETE(descriptor_set_layouts[element_index]);
                }

                // Resize the number of Descriptor Set Layouts.
                descriptor_set_layouts.clear();

                // Remove all existing child element items from the array root node.
                root_element->ClearChildren();

                // Create a new element row for each item in the resized array.
                for (int new_child_index = 0; new_child_index < new_element_count; ++new_child_index)
                {
                    create_info->AddDescriptorSetLayoutCreateInfo(new_descriptor_set_layouts[new_child_index]);

                    // Show the child index and the API's name for the structure.
                    std::stringstream element_name_stream;
                    element_name_stream << new_child_index;
                    element_name_stream << " ";
                    element_name_stream << kStrVulkanDescriptorSetLayoutCreateInfo;

                    // Create an element node for each item in the array.
                    RgEditorElementArrayElementRemove* array_element = new RgEditorElementArrayElementRemove(root_element, element_name_stream.str());

                    // Set object name.
                    QString name = kStrArrayElement + QString::number(new_child_index);
                    array_element->setObjectName(name);

                    // Each element in the array needs to know its index and the array root element.
                    array_element->SetElementIndex(array_root, new_child_index);

                    // Invoke the callback used to initialize each array element.
                    InitializeDescriptorSetLayoutCreateInfo(array_element, descriptor_set_layouts[new_child_index]);

                    // Add the element node to the array root node.
                    root_element->AppendChildItem(array_element);

                    // Initialize the newly added rows.
                    root_element->InitializeRows();
                }

                if (!first_init)
                {
                    // Let the array root element know that the child elements were resized.
                    array_root->InvokeElementResizedCallback();

                    // Expand the array root node that was resized.
                    emit ExpandNode(array_root);
                }
            }
        }
    }
}

void RgPipelineStateModelVulkan::InitializeVkGraphicsPipelineCreateInfo(RgEditorElement* root_element, RgPsoGraphicsVulkan* graphics_pipeline_create_info)
{
    assert(graphics_pipeline_create_info != nullptr);
    if (graphics_pipeline_create_info != nullptr)
    {
        // Append the new graphics pipeline create info root item to the parent.
        RgEditorElement* graphics_pipeline_create_info_root = new RgEditorElement(root_element, kStrVulkanGraphicsPipelineCreateInfo);
        root_element->AppendChildItem(graphics_pipeline_create_info_root);

        // Set object name.
        graphics_pipeline_create_info_root->setObjectName(kStrGraphicsPipelineCreateInfoRoot);

        VkGraphicsPipelineCreateInfo* vk_graphics_pipeline_create_info = graphics_pipeline_create_info->GetGraphicsPipelineCreateInfo();

        // Add the "flags" member.
        const RgEnumValuesVector& pipeline_flags_enumerators = GetPipelineCreateFlagEnumerators();
        RgEditorElement* flags_item = new RgEditorElementEnum(parent_, kStrVulkanPipelineMemberFlags, pipeline_flags_enumerators, reinterpret_cast<uint32_t*>(&vk_graphics_pipeline_create_info->flags), true);
        graphics_pipeline_create_info_root->AppendChildItem(flags_item);

        // Connect to the splitter moved signal to close the drop down.
        RgBuildViewVulkan* build_view_vulkan = static_cast<RgBuildViewVulkan*>(parent_);
        bool is_connected = connect(build_view_vulkan, &RgBuildViewVulkan::SplitterMoved, static_cast<RgEditorElementEnum*>(flags_item), &RgEditorElementEnum::HotKeyPressedSignal);
        assert(is_connected);

        // Set object name.
        flags_item->setObjectName(kStrFlagsItem);

        // Connect to the flags enum list widget status signal.
        is_connected = connect(flags_item, &RgEditorElement::EnumListWidgetStatusSignal, this, &RgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
        assert(is_connected);

        // Connect the shortcut hot key signal.
        is_connected = connect(this, &RgPipelineStateModelVulkan::HotKeyPressedSignal, static_cast<RgEditorElementEnum*>(flags_item), &RgEditorElementEnum::HotKeyPressedSignal);
        assert(is_connected);

        // Add the "pVertexInputState" node.
        VkPipelineVertexInputStateCreateInfo* vertex_input_state_create_info = graphics_pipeline_create_info->GetPipelineVertexInputStateCreateInfo();
        assert(vertex_input_state_create_info != nullptr);
        if (vertex_input_state_create_info != nullptr)
        {
            InitializeVertexInputStateCreateInfo(graphics_pipeline_create_info_root, vertex_input_state_create_info);
        }

        // Add the "pInputAssemblyState" node.
        VkPipelineInputAssemblyStateCreateInfo* input_assembly_state_create_info = graphics_pipeline_create_info->GetPipelineInputAssemblyStateCreateInfo();
        assert(input_assembly_state_create_info != nullptr);
        if (input_assembly_state_create_info != nullptr)
        {
            InitializeInputAssemblyStateCreateInfo(graphics_pipeline_create_info_root, input_assembly_state_create_info);
        }

        // Add the "pTessellationState" node.
        VkPipelineTessellationStateCreateInfo* tessellation_state_create_info = graphics_pipeline_create_info->GetPipelineTessellationStateCreateInfo();
        assert(tessellation_state_create_info != nullptr);
        if (tessellation_state_create_info != nullptr)
        {
            InitializeTessellationStateCreateInfo(graphics_pipeline_create_info_root, tessellation_state_create_info);
        }

        // Add the "pViewportState" node.
        VkPipelineViewportStateCreateInfo* pipeline_viewport_state_create_info = graphics_pipeline_create_info->GetPipelineViewportStateCreateInfo();
        assert(pipeline_viewport_state_create_info != nullptr);
        if (pipeline_viewport_state_create_info != nullptr)
        {
            InitializeViewportStateCreateInfo(graphics_pipeline_create_info_root, pipeline_viewport_state_create_info);
        }

        // Add the "pRasterizationState" node.
        VkPipelineRasterizationStateCreateInfo* pipeline_rasterization_state_create_info = graphics_pipeline_create_info->GetPipelineRasterizationStateCreateInfo();
        assert(pipeline_rasterization_state_create_info != nullptr);
        if (pipeline_rasterization_state_create_info != nullptr)
        {
            InitializeRasterizationStateCreateInfo(graphics_pipeline_create_info_root, pipeline_rasterization_state_create_info);
        }

        // Add the "pMultisampleState" node.
        VkPipelineMultisampleStateCreateInfo* pipeline_multisample_state_create_info = graphics_pipeline_create_info->GetPipelineMultisampleStateCreateInfo();
        assert(pipeline_multisample_state_create_info != nullptr);
        if (pipeline_multisample_state_create_info != nullptr)
        {
            InitializeMultisampleStateCreateInfo(graphics_pipeline_create_info_root, pipeline_multisample_state_create_info);
        }

        // Add the "pDepthStencilState" node.
        VkPipelineDepthStencilStateCreateInfo* pipeline_depth_stencil_state_create_info = graphics_pipeline_create_info->GetPipelineDepthStencilStateCreateInfo();
        assert(pipeline_depth_stencil_state_create_info != nullptr);
        if (pipeline_depth_stencil_state_create_info != nullptr)
        {
            InitializeDepthStencilStateCreateInfo(graphics_pipeline_create_info_root, pipeline_depth_stencil_state_create_info);
        }

        // Add the "pColorBlendState" node.
        VkPipelineColorBlendStateCreateInfo* pipeline_color_blend_state_create_info = graphics_pipeline_create_info->GetPipelineColorBlendStateCreateInfo();
        assert(pipeline_color_blend_state_create_info != nullptr);
        if (pipeline_color_blend_state_create_info != nullptr)
        {
            InitializeColorBlendStateCreateInfo(graphics_pipeline_create_info_root, pipeline_color_blend_state_create_info);
        }

        // Add the "subpass" member.
        RgEditorElement* subpass_create_info = MakeNumericElement(graphics_pipeline_create_info_root, kStrVulkanPipelineMemberSubpass, &vk_graphics_pipeline_create_info->subpass);
        graphics_pipeline_create_info_root->AppendChildItem(subpass_create_info);

        // Add the "basePipelineIndex" member.
        RgEditorElement* base_pipeline_index_create_info = MakeNumericElement(graphics_pipeline_create_info_root, kStrVulkanPipelineMemberBaseIndex, &vk_graphics_pipeline_create_info->basePipelineIndex);
        graphics_pipeline_create_info_root->AppendChildItem(base_pipeline_index_create_info);
    }
}

void RgPipelineStateModelVulkan::InitializeVertexInputStateCreateInfo(RgEditorElement* root_element, VkPipelineVertexInputStateCreateInfo* vertex_input_state_create_info)
{
    assert(vertex_input_state_create_info != nullptr);
    if (vertex_input_state_create_info != nullptr)
    {
        RgEditorElement* vertex_info_state_root = new RgEditorElement(root_element, kStrVulkanPipelineMemberPvertexInputState);
        root_element->AppendChildItem(vertex_info_state_root);

        // Set object name.
        vertex_info_state_root->setObjectName(kStrVertexInputStateRoot);

        // Add the "flags" member.
        RgEditorElement* flags_item = MakeNumericElement(vertex_info_state_root, kStrVulkanPipelineMemberFlags, &vertex_input_state_create_info->flags);
        vertex_info_state_root->AppendChildItem(flags_item);

        // Set object name.
        flags_item->setObjectName(kStrFlagsItem);

        // Create the vertex binding descriptions array root item.
        RgEditorElementArrayElementAdd* vertex_binding_descriptions_item = new RgEditorElementArrayElementAdd(vertex_info_state_root, kStrVulkanPipelineMemberPvertexBindingDescriptions,
            [=](int element_index) { RemoveElement(vertex_input_state_create_info->pVertexBindingDescriptions, vertex_input_state_create_info->vertexBindingDescriptionCount, element_index); });

        // Set object name.
        vertex_binding_descriptions_item->setObjectName(kStrVertexBindingDescriptionsItem);

        // Add the "vertexBindingDescriptionCount" member.
        RgEditorElement* vertex_binding_description_count_item = MakeNumericElement(nullptr, kStrVulkanPipelineMemberVertexBindingDescriptionCount,
            &vertex_input_state_create_info->vertexBindingDescriptionCount, [=] { HandleVertexBindingDescriptionCountChanged(vertex_binding_descriptions_item, vertex_input_state_create_info); });

        // Set object name.
        vertex_binding_description_count_item->setObjectName(kStrVertexBindingDescriptionCountItem);

        // Provide the element used to track the dimension of the pVertexBindingDescriptions array.
        vertex_binding_descriptions_item->SetArraySizeElement(static_cast<RgEditorElementNumeric<uint32_t>*>(vertex_binding_description_count_item));

        // Initialize the pVertexBindingDescriptions array rows.
        HandleVertexBindingDescriptionCountChanged(vertex_binding_descriptions_item, vertex_input_state_create_info, true);

        // Add the "pVertexBindingDescriptions" item.
        vertex_info_state_root->AppendChildItem(vertex_binding_descriptions_item);

        // Create the vertex attribute descriptions array root item.
        RgEditorElementArrayElementAdd* vertex_attribute_descriptions_item = new RgEditorElementArrayElementAdd(vertex_info_state_root, kStrVulkanPipelineMemberPvertexAttributeDescriptions,
            [=](int element_index) { RemoveElement(vertex_input_state_create_info->pVertexAttributeDescriptions, vertex_input_state_create_info->vertexAttributeDescriptionCount, element_index); });

        // Set object name.
        vertex_attribute_descriptions_item->setObjectName(kStrVertexAttributeDescriptionsItem);

        // Add the "vertexAttributeDescriptionCount" member.
        RgEditorElement* vertex_attribute_description_count_item = MakeNumericElement(nullptr, kStrVulkanPipelineMemberVertexAttributeDescriptionCount,
            &vertex_input_state_create_info->vertexAttributeDescriptionCount, [=] { HandleVertexAttributeDescriptionCountChanged(vertex_attribute_descriptions_item, vertex_input_state_create_info); });

        // Set object name.
        vertex_attribute_description_count_item->setObjectName(kStrVertexAttributeDescriptionCountItem);

        // Provide the element used to track the dimension of the pVertexAttributeDescriptions array.
        vertex_attribute_descriptions_item->SetArraySizeElement(static_cast<RgEditorElementNumeric<uint32_t>*>(vertex_attribute_description_count_item));

        // Initialize the pVertexAttributeDescriptions array rows.
        HandleVertexAttributeDescriptionCountChanged(vertex_attribute_descriptions_item, vertex_input_state_create_info, true);

        // Add the "pVertexAttributeDescriptions" member.
        vertex_info_state_root->AppendChildItem(vertex_attribute_descriptions_item);
    }
}

void RgPipelineStateModelVulkan::HandleVertexBindingDescriptionCountChanged(RgEditorElement* root_element, VkPipelineVertexInputStateCreateInfo* input_state_create_info, bool first_init)
{
    ResizeHandler(root_element,
        input_state_create_info->vertexBindingDescriptionCount,
        input_state_create_info->pVertexBindingDescriptions,
        kStrVulkanVertexInputBindingDescription,
        std::bind(&RgPipelineStateModelVulkan::InitializeVertexInputBindingDescriptionCreateInfo, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
        first_init);
}

void RgPipelineStateModelVulkan::HandleVertexAttributeDescriptionCountChanged(RgEditorElement* root_element, VkPipelineVertexInputStateCreateInfo* input_state_create_info, bool first_init)
{
    ResizeHandler(root_element,
        input_state_create_info->vertexAttributeDescriptionCount,
        input_state_create_info->pVertexAttributeDescriptions,
        kStrVulkanVertexInputAttributeDescription,
        std::bind(&RgPipelineStateModelVulkan::InitializeVertexInputAttributeDescriptionCreateInfo, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
        first_init);
}

void RgPipelineStateModelVulkan::InitializeVertexInputBindingDescriptionCreateInfo(RgEditorElement* root_element, VkVertexInputBindingDescription* base_input_binding_description_item, int item_index)
{
    assert(root_element != nullptr);
    assert(base_input_binding_description_item != nullptr);
    if (root_element != nullptr && base_input_binding_description_item != nullptr)
    {
        VkVertexInputBindingDescription* offset_input_binding_description_item = base_input_binding_description_item + item_index;
        assert(offset_input_binding_description_item != nullptr);
        if (offset_input_binding_description_item != nullptr)
        {
            // Add the "binding" member.
            RgEditorElement* binding_element = MakeNumericElement(root_element, kStrVulkanPipelineMemberVertexBinding, &offset_input_binding_description_item->binding);
            root_element->AppendChildItem(binding_element);

            // Set object name.
            binding_element->setObjectName(kStrBindingElement);

            // Add the "stride" member.
            RgEditorElement* stride_element = MakeNumericElement(root_element, kStrVulkanPipelineMemberVertexStride, &offset_input_binding_description_item->stride);
            root_element->AppendChildItem(stride_element);

            // Set object name.
            stride_element->setObjectName(kStrStrideElement);

            // Add the "inputRate" member values.
            const RgEnumValuesVector input_rate_values = {
                ENUM_VALUE(VK_VERTEX_INPUT_RATE_VERTEX),
                ENUM_VALUE(VK_VERTEX_INPUT_RATE_INSTANCE)
            };
            RgEditorElement* input_rate_element = new RgEditorElementEnum(parent_, kStrVulkanPipelineMemberVertexInputRate, input_rate_values, reinterpret_cast<uint32_t*>(&offset_input_binding_description_item->inputRate));
            root_element->AppendChildItem(input_rate_element);

            // Connect to the splitter moved signal to close the drop down.
            RgBuildViewVulkan* build_view_vulkan = static_cast<RgBuildViewVulkan*>(parent_);
            bool is_connected = connect(build_view_vulkan, &RgBuildViewVulkan::SplitterMoved, static_cast<RgEditorElementEnum*>(input_rate_element), &RgEditorElementEnum::HotKeyPressedSignal);

            // Set object name.
            input_rate_element->setObjectName(kStrInputRateElement);

            // Connect to the enum list widget status signal.
            is_connected = connect(input_rate_element, &RgEditorElement::EnumListWidgetStatusSignal, this, &RgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
            assert(is_connected);

            // Connect the shortcut hot key signal.
            is_connected = connect(this, &RgPipelineStateModelVulkan::HotKeyPressedSignal, static_cast<RgEditorElementEnum*>(input_rate_element), &RgEditorElementEnum::HotKeyPressedSignal);
            assert(is_connected);
        }
    }
}

void RgPipelineStateModelVulkan::InitializeVertexInputAttributeDescriptionCreateInfo(RgEditorElement* root_element, VkVertexInputAttributeDescription* base_input_attribute_description, int item_index)
{
    assert(root_element != nullptr);
    assert(base_input_attribute_description != nullptr);
    if (root_element != nullptr && base_input_attribute_description != nullptr)
    {
        VkVertexInputAttributeDescription* offset_input_attribute_description = base_input_attribute_description + item_index;
        assert(offset_input_attribute_description != nullptr);
        if (offset_input_attribute_description != nullptr)
        {
            // Add the "location" member.
            RgEditorElement* location_element = MakeNumericElement(root_element, kStrVulkanPipelineMemberVertexLocation, &offset_input_attribute_description->location);
            root_element->AppendChildItem(location_element);

            // Set object name.
            location_element->setObjectName(kStrLocationElement);

            // Add the "binding" member.
            RgEditorElement* binding_element = MakeNumericElement(root_element, kStrVulkanPipelineMemberVertexBinding, &offset_input_attribute_description->binding);
            root_element->AppendChildItem(binding_element);

            // Set object name.
            binding_element->setObjectName(kStrBindingElement);

            // Add the "format" member values.
            const RgEnumValuesVector& format_enumerators = GetFormatEnumerators();
            RgEditorElement* format_element = new RgEditorElementEnum(parent_, kStrVulkanPipelineMemberVertexFormat, format_enumerators, reinterpret_cast<uint32_t*>(&offset_input_attribute_description->format));
            root_element->AppendChildItem(format_element);

            // Connect to the splitter moved signal to close the drop down.
            RgBuildViewVulkan* build_view_vulkan = static_cast<RgBuildViewVulkan*>(parent_);
            bool is_connected = connect(build_view_vulkan, &RgBuildViewVulkan::SplitterMoved, static_cast<RgEditorElementEnum*>(format_element), &RgEditorElementEnum::HotKeyPressedSignal);

            // Set object name.
            format_element->setObjectName(kStrFormatElement);

            // Connect to the enum list widget status signal.
            is_connected = connect(format_element, &RgEditorElement::EnumListWidgetStatusSignal, this, &RgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
            assert(is_connected);

            // Connect the shortcut hot key signal.
            is_connected = connect(this, &RgPipelineStateModelVulkan::HotKeyPressedSignal, static_cast<RgEditorElementEnum*>(format_element), &RgEditorElementEnum::HotKeyPressedSignal);
            assert(is_connected);

            // Add the "offset" member.
            RgEditorElement* offset_element = MakeNumericElement(root_element, kStrVulkanPipelineMemberOffset, &offset_input_attribute_description->offset);
            root_element->AppendChildItem(offset_element);

            // Set object name.
            offset_element->setObjectName(kStrOffsetElement);
        }
    }
}

void RgPipelineStateModelVulkan::InitializeInputAssemblyStateCreateInfo(RgEditorElement* root_element, VkPipelineInputAssemblyStateCreateInfo* input_assembly_state_create_info)
{
    assert(input_assembly_state_create_info != nullptr);
    if (input_assembly_state_create_info != nullptr)
    {
        RgEditorElement* input_assembly_state_root = new RgEditorElement(root_element, kStrVulkanPipelineMemberPinputAssemblyState);
        root_element->AppendChildItem(input_assembly_state_root);

        // Set object name.
        input_assembly_state_root->setObjectName(kStrInputAssemblyStateRoot);

        // Add the "flags" member.
        RgEditorElement* flags_item = MakeNumericElement(input_assembly_state_root, kStrVulkanPipelineMemberFlags, &input_assembly_state_create_info->flags);
        input_assembly_state_root->AppendChildItem(flags_item);

        // Set object name.
        flags_item->setObjectName(kStrFlagsItem);

        // Add the primitive "topology" node.
        const RgEnumValuesVector& primitive_topology_enumerators = GetPrimitiveTopologyEnumerators();
        RgEditorElement* topology_item = new RgEditorElementEnum(parent_, kStrVulkanPipelineMemberTopology, primitive_topology_enumerators, reinterpret_cast<uint32_t*>(&input_assembly_state_create_info->topology));
        input_assembly_state_root->AppendChildItem(topology_item);

        // Connect to the splitter moved signal to close the drop down.
        RgBuildViewVulkan* build_view_vulkan = static_cast<RgBuildViewVulkan*>(parent_);
        bool is_connected = connect(build_view_vulkan, &RgBuildViewVulkan::SplitterMoved, static_cast<RgEditorElementEnum*>(topology_item), &RgEditorElementEnum::HotKeyPressedSignal);

        // Set object name.
        topology_item->setObjectName(kStrTopologyItem);

        // Connect to the enum list widget status signal.
        is_connected = connect(topology_item, &RgEditorElement::EnumListWidgetStatusSignal, this, &RgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
        assert(is_connected);

        // Connect the shortcut hot key signal.
        is_connected = connect(this, &RgPipelineStateModelVulkan::HotKeyPressedSignal, static_cast<RgEditorElementEnum*>(topology_item), &RgEditorElementEnum::HotKeyPressedSignal);
        assert(is_connected);

        // Add the "primitiveRestartEnable" member.
        RgEditorElementBool* primitive_restart_enable_item = new RgEditorElementBool(input_assembly_state_root, kStrVulkanPipelineMemberPrimitiveRestartEnable, &input_assembly_state_create_info->primitiveRestartEnable);
        input_assembly_state_root->AppendChildItem(primitive_restart_enable_item);

        // Set object name.
        primitive_restart_enable_item->setObjectName(kStrPrimitiveRestartEnableItem);
    }
}

void RgPipelineStateModelVulkan::InitializeTessellationStateCreateInfo(RgEditorElement* root_element, VkPipelineTessellationStateCreateInfo* tessellation_state_create_info)
{
    assert(tessellation_state_create_info != nullptr);
    if (tessellation_state_create_info != nullptr)
    {
        RgEditorElement* tessellation_state_root = new RgEditorElement(root_element, kStrVulkanPipelineMemberPtessellationState);
        root_element->AppendChildItem(tessellation_state_root);

        // Set object name.
        tessellation_state_root->setObjectName(kStrTessellationStateRoot);

        // Add the "flags" member.
        RgEditorElement* flags_item = MakeNumericElement(tessellation_state_root, kStrVulkanPipelineMemberFlags, &tessellation_state_create_info->flags);
        tessellation_state_root->AppendChildItem(flags_item);

        // Set object name.
        flags_item->setObjectName(kStrFlagsItem);

        // Add the "patchControlPoints" member.
        RgEditorElement* patch_control_points_item = MakeNumericElement(tessellation_state_root, kStrVulkanPipelineMemberPatchControlPoints, &tessellation_state_create_info->patchControlPoints);
        tessellation_state_root->AppendChildItem(patch_control_points_item);

        // Set object name.
        patch_control_points_item->setObjectName(kStrPatchControlPointsItem);
    }
}

void RgPipelineStateModelVulkan::InitializeViewportStateCreateInfo(RgEditorElement* root_element, VkPipelineViewportStateCreateInfo* pipeline_viewport_state_create_info)
{
    assert(pipeline_viewport_state_create_info != nullptr);
    if (pipeline_viewport_state_create_info != nullptr)
    {
        RgEditorElement* viewport_state_root = new RgEditorElement(root_element, kStrVulkanPipelineMemberPviewportState);
        root_element->AppendChildItem(viewport_state_root);

        // Set object name.
        viewport_state_root->setObjectName(kStrViewportStateRootItem);

        // Add the "flags" member.
        RgEditorElement* flags_item = MakeNumericElement(viewport_state_root, kStrVulkanPipelineMemberFlags, &pipeline_viewport_state_create_info->flags);
        viewport_state_root->AppendChildItem(flags_item);

        // Set object name.
        flags_item->setObjectName(kStrFlagsItem);

        // Create the "pViewports" root node.
        RgEditorElementArrayElementAdd* viewports_root_item = new RgEditorElementArrayElementAdd(viewport_state_root, kStrVulkanPipelineMemberPviewports,
            [=](int element_index) { RemoveElement(pipeline_viewport_state_create_info->pViewports, pipeline_viewport_state_create_info->viewportCount, element_index); });

        // Set object name.
        viewports_root_item->setObjectName(kStrViewportsRootItem);

        assert(viewports_root_item != nullptr);
        if (viewports_root_item != nullptr)
        {
            // Create the "viewportCountItem" node.
            RgEditorElement* viewport_count_item = MakeNumericElement(nullptr, kStrVulkanPipelineMemberViewportCount,
                &pipeline_viewport_state_create_info->viewportCount, [=] { HandlePipelineViewportCountChanged(viewports_root_item, pipeline_viewport_state_create_info); });

            // Set object name.
            viewport_count_item->setObjectName(kStrViewportCountItem);

            // Add the viewport array node.
            viewport_state_root->AppendChildItem(viewports_root_item);

            // Provide the element used to track the dimension of the pViewports array.
            viewports_root_item->SetArraySizeElement(static_cast<RgEditorElementNumeric<uint32_t>*>(viewport_count_item));

            // Initialize the pViewports array rows.
            HandlePipelineViewportCountChanged(viewports_root_item, pipeline_viewport_state_create_info, true);
        }

        // Create the "pScissors" root node.
        RgEditorElementArrayElementAdd* scissors_root_item = new RgEditorElementArrayElementAdd(viewport_state_root, kStrVulkanPipelineMemberPscissors,
            [=](int element_index) { RemoveElement(pipeline_viewport_state_create_info->pScissors, pipeline_viewport_state_create_info->scissorCount, element_index); });

        // Set object name.
        scissors_root_item->setObjectName(kStrScissorsRootItem);

        // Create the "scissorCountItem" member.
        RgEditorElement* scissor_count_item = MakeNumericElement(nullptr, kStrVulkanPipelineMemberScissorCount,
            &pipeline_viewport_state_create_info->scissorCount, [=] { HandlePipelineScissorCountChanged(scissors_root_item, pipeline_viewport_state_create_info); });

        // Set object name.
        scissor_count_item->setObjectName(kStrScissorCountItem);

        // Add the "pScissors" member.
        viewport_state_root->AppendChildItem(scissors_root_item);

        // Provide the element used to track the dimension of the pScissors array.
        scissors_root_item->SetArraySizeElement(static_cast<RgEditorElementNumeric<uint32_t>*>(scissor_count_item));

        // Initialize the pScissors array rows.
        HandlePipelineScissorCountChanged(scissors_root_item, pipeline_viewport_state_create_info, true);
    }
}

void RgPipelineStateModelVulkan::HandlePipelineViewportCountChanged(RgEditorElement* root_element, VkPipelineViewportStateCreateInfo* viewport_state_create_info, bool first_init)
{
    ResizeHandler(root_element,
        viewport_state_create_info->viewportCount,
        viewport_state_create_info->pViewports,
        kStrVulkanPipelineMemberVkViewport,
        std::bind(&RgPipelineStateModelVulkan::InitializePipelineViewportDescriptionCreateInfo, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
        first_init);
}

void RgPipelineStateModelVulkan::HandlePipelineScissorCountChanged(RgEditorElement* root_element, VkPipelineViewportStateCreateInfo* viewport_state_create_info, bool first_init)
{
    ResizeHandler(root_element,
        viewport_state_create_info->scissorCount,
        viewport_state_create_info->pScissors,
        kStrVulkanPipelineMemberScissorRect,
        std::bind(&RgPipelineStateModelVulkan::InitializePipelineScissorDescriptionCreateInfo, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
        first_init);
}

void RgPipelineStateModelVulkan::InitializePipelineViewportDescriptionCreateInfo(RgEditorElement* root_element, VkViewport* base_viewport_description, int item_index)
{
    assert(root_element != nullptr);
    assert(base_viewport_description != nullptr);
    if (root_element != nullptr && base_viewport_description != nullptr)
    {
        VkViewport* offset_viewport_description = base_viewport_description + item_index;
        assert(offset_viewport_description != nullptr);
        if (offset_viewport_description != nullptr)
        {
            // Add the "x" node.
            RgEditorElement* x_item = MakeNumericElement(root_element, kStrVulkanMemberX, &offset_viewport_description->x);
            root_element->AppendChildItem(x_item);

            // Set object name.
            x_item->setObjectName(kStrXItem);

            // Add the "y" node.
            RgEditorElement* y_item = MakeNumericElement(root_element, kStrVulkanMemberY, &offset_viewport_description->y);
            root_element->AppendChildItem(y_item);

            // Set object name.
            y_item->setObjectName(kStrYItem);

            // Add the "width" node.
            RgEditorElement* width_item = MakeNumericElement(root_element, kStrVulkanMemberWidth, &offset_viewport_description->width);
            root_element->AppendChildItem(width_item);

            // Set object name.
            width_item->setObjectName(kStrWidthItem);

            // Add the "height" node.
            RgEditorElement* height_item = MakeNumericElement(root_element, kStrVulkanMemberHeight, &offset_viewport_description->height);
            root_element->AppendChildItem(height_item);

            // Set object name.
            height_item->setObjectName(kStrHeightItem);

            // Add the "minDepth" node.
            RgEditorElement* min_depth_item = MakeNumericElement(root_element, kStrVulkanPipelineMemberViewportMinDepth, &offset_viewport_description->minDepth);
            root_element->AppendChildItem(min_depth_item);

            // Set object name.
            min_depth_item->setObjectName(kStrMinDepthItem);

            // Add the "maxDepth" node.
            RgEditorElement* max_depth_item = MakeNumericElement(root_element, kStrVulkanPipelineMemberViewportMaxDepth, &offset_viewport_description->maxDepth);
            root_element->AppendChildItem(max_depth_item);

            // Set object name.
            max_depth_item->setObjectName(kStrMaxDepthItem);
        }
    }
}

void RgPipelineStateModelVulkan::InitializePipelineScissorDescriptionCreateInfo(RgEditorElement* root_element, VkRect2D* base_scissor_description, int item_index)
{
    assert(root_element != nullptr);
    assert(base_scissor_description != nullptr);
    if (root_element != nullptr && base_scissor_description != nullptr)
    {
        VkRect2D* offset_scissor_description = base_scissor_description + item_index;
        assert(offset_scissor_description != nullptr);
        if (offset_scissor_description != nullptr)
        {
            // Get the stencil operation enumerators.
            RgEditorElement* offset_root = new RgEditorElement(root_element, kStrVulkanPipelineMemberOffset);

            // Add the "x" node.
            RgEditorElement* x_item = MakeNumericElement(offset_root, kStrVulkanMemberX, &offset_scissor_description->offset.x);
            offset_root->AppendChildItem(x_item);

            // Set object name.
            x_item->setObjectName(kStrXItem);

            // Add the "y" node.
            RgEditorElement* y_item = MakeNumericElement(offset_root, kStrVulkanMemberY, &offset_scissor_description->offset.y);
            offset_root->AppendChildItem(y_item);

            // Set object name.
            y_item->setObjectName(kStrYItem);

            // Add the offset root element.
            root_element->AppendChildItem(offset_root);

            RgEditorElement* extent_root = new RgEditorElement(root_element, kStrVulkanPipelineMemberExtent);

            // Set object name.
            extent_root->setObjectName(kStrExtentRoot);

            // Add the "width" node.
            RgEditorElement* width_item = MakeNumericElement(extent_root, kStrVulkanMemberWidth, &offset_scissor_description->extent.width);
            extent_root->AppendChildItem(width_item);

            // Set object name.
            width_item->setObjectName(kStrWidthItem);

            // Add the "height" node.
            RgEditorElement* height_item = MakeNumericElement(extent_root, kStrVulkanMemberHeight, &offset_scissor_description->extent.height);
            extent_root->AppendChildItem(height_item);

            // Set object name.
            height_item->setObjectName(kStrHeightItem);

            // Add the extent root element.
            root_element->AppendChildItem(extent_root);
        }
    }
}

void RgPipelineStateModelVulkan::InitializeRasterizationStateCreateInfo(RgEditorElement* root_element, VkPipelineRasterizationStateCreateInfo* rasterization_state_create_info)
{
    assert(rasterization_state_create_info != nullptr);
    if (rasterization_state_create_info != nullptr)
    {
        RgEditorElement* rasterization_state_root = new RgEditorElement(root_element, kStrVulkanPipelineMemberPrasterizationState);
        root_element->AppendChildItem(rasterization_state_root);

        // Set object name.
        rasterization_state_root->setObjectName(kStrRasterizationStateRoot);

        // Add the "flags" member.
        RgEditorElement* flags_item = MakeNumericElement(rasterization_state_root, kStrVulkanPipelineMemberFlags, &rasterization_state_create_info->flags);
        rasterization_state_root->AppendChildItem(flags_item);

        // Set object name.
        flags_item->setObjectName(kStrFlagsItem);

        // Add the "depthClampEnable" node.
        RgEditorElementBool* depth_clamp_enable_item = new RgEditorElementBool(rasterization_state_root, kStrVulkanPipelineMemberDepthClampEnable, &rasterization_state_create_info->depthClampEnable);
        rasterization_state_root->AppendChildItem(depth_clamp_enable_item);

        // Set object name.
        depth_clamp_enable_item->setObjectName(kStrDepthClampEnableItem);

        // Add the "rasterizerDiscardEnable" node.
        RgEditorElementBool* rasterization_discard_enable = new RgEditorElementBool(rasterization_state_root, kStrVulkanPipelineMemberRasterizerDiscardEnable, &rasterization_state_create_info->rasterizerDiscardEnable);
        rasterization_state_root->AppendChildItem(rasterization_discard_enable);

        // Set object name.
        rasterization_discard_enable->setObjectName(kStrRasterizerDiscardEnable);

        // Add the "polygonMode" member values.
        const RgEnumValuesVector& polygon_mode_enumerators = GetPolygonModeEnumerators();
        RgEditorElement* polygon_mode_node = new RgEditorElementEnum(parent_, kStrVulkanPipelineMemberRasterizerPolygonMode, polygon_mode_enumerators, reinterpret_cast<uint32_t*>(&rasterization_state_create_info->polygonMode));
        rasterization_state_root->AppendChildItem(polygon_mode_node);

        // Connect to the splitter moved signal to close the drop down.
        RgBuildViewVulkan* build_view_vulkan = static_cast<RgBuildViewVulkan*>(parent_);
        bool is_connected = connect(build_view_vulkan, &RgBuildViewVulkan::SplitterMoved, static_cast<RgEditorElementEnum*>(polygon_mode_node), &RgEditorElementEnum::HotKeyPressedSignal);

        // Set object name.
        polygon_mode_node->setObjectName(kStrPolygonModeNode);

        // Connect to the enum list widget status signal.
        is_connected = connect(polygon_mode_node, &RgEditorElement::EnumListWidgetStatusSignal, this, &RgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
        assert(is_connected);

        // Connect the shortcut hot key signal.
        is_connected = connect(this, &RgPipelineStateModelVulkan::HotKeyPressedSignal, static_cast<RgEditorElementEnum*>(polygon_mode_node), &RgEditorElementEnum::HotKeyPressedSignal);
        assert(is_connected);

        // Add the "cullMode" member values.
        const RgEnumValuesVector& cull_mode_enumerators = GetCullModeFlagEnumerators();
        RgEditorElement* cull_mode_node = new RgEditorElementEnum(parent_, kStrVulkanPipelineMemberRasterizerCullMode, cull_mode_enumerators, reinterpret_cast<uint32_t*>(&rasterization_state_create_info->cullMode), true);
        rasterization_state_root->AppendChildItem(cull_mode_node);

        // Connect to the splitter moved signal to close the drop down.
        build_view_vulkan = static_cast<RgBuildViewVulkan*>(parent_);
        is_connected = connect(build_view_vulkan, &RgBuildViewVulkan::SplitterMoved, static_cast<RgEditorElementEnum*>(cull_mode_node), &RgEditorElementEnum::HotKeyPressedSignal);

        // Set object name.
        cull_mode_node->setObjectName(kStrCullModeNode);

        // Connect to the enum list widget status signal.
        is_connected = connect(cull_mode_node, &RgEditorElement::EnumListWidgetStatusSignal, this, &RgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
        assert(is_connected);

        // Connect the shortcut hot key signal.
        is_connected = connect(this, &RgPipelineStateModelVulkan::HotKeyPressedSignal, static_cast<RgEditorElementEnum*>(cull_mode_node), &RgEditorElementEnum::HotKeyPressedSignal);
        assert(is_connected);

        // Add the "frontFace" member values.
        const RgEnumValuesVector& front_face_enumerators = GetFrontFaceEnumerators();
        RgEditorElement* front_face_node = new RgEditorElementEnum(parent_, kStrVulkanPipelineMemberRasterizerFrontFace, front_face_enumerators, reinterpret_cast<uint32_t*>(&rasterization_state_create_info->frontFace));
        rasterization_state_root->AppendChildItem(front_face_node);

        // Connect to the splitter moved signal to close the drop down.
        build_view_vulkan = static_cast<RgBuildViewVulkan*>(parent_);
        is_connected = connect(build_view_vulkan, &RgBuildViewVulkan::SplitterMoved, static_cast<RgEditorElementEnum*>(front_face_node), &RgEditorElementEnum::HotKeyPressedSignal);

        // Set object name.
        front_face_node->setObjectName(kStrFrontFaceNode);

        // Connect to the enum list widget status signal.
        is_connected = connect(front_face_node, &RgEditorElement::EnumListWidgetStatusSignal, this, &RgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
        assert(is_connected);

        // Connect the shortcut hot key signal.
        is_connected = connect(this, &RgPipelineStateModelVulkan::HotKeyPressedSignal, static_cast<RgEditorElementEnum*>(front_face_node), &RgEditorElementEnum::HotKeyPressedSignal);
        assert(is_connected);

        // Add the "depthBiasEnable" node.
        RgEditorElementBool* depth_bias_enable_node = new RgEditorElementBool(rasterization_state_root, kStrVulkanPipelineMemberRasterizerDepthBiasEnable, &rasterization_state_create_info->depthBiasEnable);
        rasterization_state_root->AppendChildItem(depth_bias_enable_node);

        // Set object name.
        depth_bias_enable_node->setObjectName(kStrDepthBiasEnableNode);

        // Add the "depthBiasConstantFactor" node.
        RgEditorElement* depth_bias_constant_factor_node = MakeNumericElement(rasterization_state_root, kStrVulkanPipelineMemberRasterizerDepthBiasConstantFactor, &rasterization_state_create_info->depthBiasConstantFactor);
        rasterization_state_root->AppendChildItem(depth_bias_constant_factor_node);

        // Set object name.
        depth_bias_constant_factor_node->setObjectName(kStrDepthBiasConstantFactorNode);

        // Add the "depthBiasClamp" node.
        RgEditorElement* depth_bias_clamp_node = MakeNumericElement(rasterization_state_root, kStrVulkanPipelineMemberRasterizerDepthBiasClamp, &rasterization_state_create_info->depthBiasClamp);
        rasterization_state_root->AppendChildItem(depth_bias_clamp_node);

        // Set object name.
        depth_bias_clamp_node->setObjectName(kStrDepthBiasClampNode);

        // Add the "depthBiasSlopeFactor" node.
        RgEditorElement* depth_bias_slope_factor_node = MakeNumericElement(rasterization_state_root, kStrVulkanPipelineMemberRasterizerDepthBiasSlopeFactor, &rasterization_state_create_info->depthBiasSlopeFactor);
        rasterization_state_root->AppendChildItem(depth_bias_slope_factor_node);

        // Set object name.
        depth_bias_slope_factor_node->setObjectName(kStrDepthBiasSlopeFactorNode);

        // Add the "lineWidth" node.
        RgEditorElement* line_width_node = MakeNumericElement(rasterization_state_root, kStrVulkanPipelineMemberRasterizerLineWidth, &rasterization_state_create_info->lineWidth);
        rasterization_state_root->AppendChildItem(line_width_node);

        // Set object name.
        line_width_node->setObjectName(kStrLineWidthNode);
    }
}

void RgPipelineStateModelVulkan::HandleMultisamplingSampleMaskDimensionChanged(RgEditorElement* root_element, VkPipelineMultisampleStateCreateInfo* multisample_state_create_info, bool first_init)
{
    ResizeHandler(root_element,
        sample_mask_dimension_,
        multisample_state_create_info->pSampleMask,
        kStrVulkanMultisampleRasterizationSampleFlagsType,
        std::bind(&RgPipelineStateModelVulkan::InitializeSampleMask, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
        first_init);
}

void RgPipelineStateModelVulkan::InitializeSampleMask(RgEditorElement* root_element, uint32_t* sample_mask_base, int item_index)
{
    assert(root_element != nullptr);
    assert(sample_mask_base != nullptr);
    if (root_element != nullptr && sample_mask_base != nullptr)
    {
        uint32_t* sample_mask_item = sample_mask_base + item_index;
        assert(sample_mask_item != nullptr);
        if (sample_mask_item != nullptr)
        {
            // Add the array element.
            RgEditorElement* sample_mask_elemet_node = MakeNumericElement(root_element, kStrVulkanMultisampleRasterizationSampleFlagsElementType, reinterpret_cast<uint32_t*>(sample_mask_item));
            root_element->AppendChildItem(sample_mask_elemet_node);

            // Set object name.
            sample_mask_elemet_node->setObjectName(kStrSampleMaskElementNode);
        }
    }
}

void RgPipelineStateModelVulkan::InitializeMultisampleStateCreateInfo(RgEditorElement* root_element, VkPipelineMultisampleStateCreateInfo* pipeline_multisample_state_create_info)
{
    // Initialize the sample mask dimension.
    sample_mask_dimension_ = 0;

    assert(pipeline_multisample_state_create_info != nullptr);
    if (pipeline_multisample_state_create_info != nullptr)
    {
        RgEditorElement* multisample_state_root = new RgEditorElement(root_element, kStrVulkanPipelineMemberPmultisampleState);
        root_element->AppendChildItem(multisample_state_root);

        // Set object name.
        multisample_state_root->setObjectName(kStrMultisampleStateRoot);

        // Add the "flags" node.
        RgEditorElement* flags_item = MakeNumericElement(multisample_state_root, kStrVulkanPipelineMemberFlags, &pipeline_multisample_state_create_info->flags);
        multisample_state_root->AppendChildItem(flags_item);

        // Set object name.
        flags_item->setObjectName(kStrFlagsItem);

        // pSampleMask array:
        if (pipeline_multisample_state_create_info->pSampleMask != nullptr)
        {
            // Initialize the mask dimension if the pSampleMask is used.
            uint32_t enum_dimension = GetSampleMaskDimension(pipeline_multisample_state_create_info->rasterizationSamples);
            sample_mask_dimension_ = enum_dimension;
        }

        // Create the pSampleMask array root item.
        RgEditorElementArrayElementAdd* sample_mask_root_item = new RgEditorElementArrayElementAdd(multisample_state_root, kStrVulkanMultisampleSampleMask,
            [=](int element_index) { RemoveElement(pipeline_multisample_state_create_info->pSampleMask, sample_mask_dimension_, element_index); });

        // Set object name.
        sample_mask_root_item->setObjectName(kStrSampleMaskRootItem);

        // The user can only have up to 2 elements within the pSampleMask array.
        sample_mask_root_item->SetMaximumArraySize(2);

        // Create the pSampleMask dimension node.
        RgEditorElement* sample_mask_count_node = MakeNumericElement(nullptr, kStrVulkanRenderPassSubpassInputAttachmentCount,
            &sample_mask_dimension_,
            [=] { HandleMultisamplingSampleMaskDimensionChanged(sample_mask_root_item, pipeline_multisample_state_create_info); });

        // Set object name.
        sample_mask_count_node->setObjectName(kStrSampleMaskCountNode);

        // Set the array size element for the pSampleMask root item.
        // When the array size count value changes, the array of child elements will get resized.
        sample_mask_root_item->SetArraySizeElement(static_cast<RgEditorElementNumeric<uint32_t>*>(sample_mask_count_node));

        // Initialize the existing array of pSampleMask data.
        HandleMultisamplingSampleMaskDimensionChanged(sample_mask_root_item, pipeline_multisample_state_create_info, true);

        // Add the "rasterizationSamples" flags node.
        const RgEnumValuesVector& rasterization_samples_enumerators = GetRasterizationSamplesEnumerators();
        RgEditorElement* rasterization_samples = new RgEditorElementEnum(parent_, kStrVulkanMultisampleRasterizationSamples, rasterization_samples_enumerators, reinterpret_cast<uint32_t*>(&pipeline_multisample_state_create_info->rasterizationSamples));
        multisample_state_root->AppendChildItem(rasterization_samples);

        // Connect to the splitter moved signal to close the drop down.
        RgBuildViewVulkan* build_view_vulkan = static_cast<RgBuildViewVulkan*>(parent_);
        bool is_connected = connect(build_view_vulkan, &RgBuildViewVulkan::SplitterMoved, static_cast<RgEditorElementEnum*>(rasterization_samples), &RgEditorElementEnum::HotKeyPressedSignal);

        // Set object name.
        rasterization_samples->setObjectName(kStrRasterizationSamples);

        // Connect to the enum list widget status signal.
        is_connected = connect(rasterization_samples, &RgEditorElement::EnumListWidgetStatusSignal, this, &RgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
        assert(is_connected);

        // Connect the shortcut hot key signal.
        is_connected = connect(this, &RgPipelineStateModelVulkan::HotKeyPressedSignal, static_cast<RgEditorElementEnum*>(rasterization_samples), &RgEditorElementEnum::HotKeyPressedSignal);
        assert(is_connected);

        // Add the "sampleShadingEnable" node.
        RgEditorElementBool* sample_shading_enable = new RgEditorElementBool(multisample_state_root, kStrVulkanMultisampleSampleShadingEnable, &pipeline_multisample_state_create_info->sampleShadingEnable);
        multisample_state_root->AppendChildItem(sample_shading_enable);

        // Set object name.
        sample_shading_enable->setObjectName(kStrSampleShadingEnable);

        // Add the "minSampleShading" node.
        RgEditorElement* min_sample_shading_node = MakeNumericElement(multisample_state_root, kStrVulkanMultisampleMinSampleShading, &pipeline_multisample_state_create_info->minSampleShading);
        multisample_state_root->AppendChildItem(min_sample_shading_node);

        // Set object name.
        min_sample_shading_node->setObjectName(kStrMinSampleShadingNode);

        // Add the sample mask array node.
        multisample_state_root->AppendChildItem(sample_mask_root_item);

        // Add the "alphaToCoverageEnable" node.
        RgEditorElementBool* alpha_to_coverage_node = new RgEditorElementBool(multisample_state_root, kStrVulkanMultisampleAlphaToCoverageEnable, &pipeline_multisample_state_create_info->alphaToCoverageEnable);
        multisample_state_root->AppendChildItem(alpha_to_coverage_node);

        // Set object name.
        alpha_to_coverage_node->setObjectName(kStrAlphaBlendToCoverageNode);

        // Add the "alphaToOneEnable" node.
        RgEditorElementBool* alpha_to_one_node = new RgEditorElementBool(multisample_state_root, kStrVulkanMultisampleAlphaToOneEnable, &pipeline_multisample_state_create_info->alphaToOneEnable);
        multisample_state_root->AppendChildItem(alpha_to_one_node);

        // Set object name.
        alpha_to_one_node->setObjectName(kStrAlphaToOneNode);
    }
}

void RgPipelineStateModelVulkan::InitializeStencilOpState(RgEditorElement* depth_stencil_state_root, VkStencilOpState* stencil_op_state)
{
    assert(depth_stencil_state_root != nullptr);
    assert(stencil_op_state != nullptr);
    if (depth_stencil_state_root != nullptr && stencil_op_state != nullptr)
    {
        // Get the stencil operation enumerators.
        const RgEnumValuesVector& stencil_op_enumerators = GetStencilOpEnumerators();

        // Add the "failOp" node.
        RgEditorElement* fail_op_node = new RgEditorElementEnum(parent_, kStrVulkanDepthStencilStateFailOp, stencil_op_enumerators, reinterpret_cast<uint32_t*>(&stencil_op_state->failOp));
        depth_stencil_state_root->AppendChildItem(fail_op_node);

        // Connect to the splitter moved signal to close the drop down.
        RgBuildViewVulkan* build_view_vulkan = static_cast<RgBuildViewVulkan*>(parent_);
        bool is_connected = connect(build_view_vulkan, &RgBuildViewVulkan::SplitterMoved, static_cast<RgEditorElementEnum*>(fail_op_node), &RgEditorElementEnum::HotKeyPressedSignal);

        // Set object name.
        fail_op_node->setObjectName(kStrFailOpNode);

        // Connect to the enum list widget status signal.
        is_connected = connect(fail_op_node, &RgEditorElement::EnumListWidgetStatusSignal, this, &RgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
        assert(is_connected);

        // Connect the shortcut hot key signal.
        is_connected = connect(this, &RgPipelineStateModelVulkan::HotKeyPressedSignal, static_cast<RgEditorElementEnum*>(fail_op_node), &RgEditorElementEnum::HotKeyPressedSignal);
        assert(is_connected);

        // Add the "passOp" node.
        RgEditorElement* pass_op_node = new RgEditorElementEnum(parent_, kStrVulkanDepthStencilStatePassOp, stencil_op_enumerators, reinterpret_cast<uint32_t*>(&stencil_op_state->passOp));
        depth_stencil_state_root->AppendChildItem(pass_op_node);

        // Connect to the splitter moved signal to close the drop down.
        build_view_vulkan = static_cast<RgBuildViewVulkan*>(parent_);
        is_connected = connect(build_view_vulkan, &RgBuildViewVulkan::SplitterMoved, static_cast<RgEditorElementEnum*>(pass_op_node), &RgEditorElementEnum::HotKeyPressedSignal);

        // Set object name.
        pass_op_node->setObjectName(kStrPassOpNode);

        // Connect to the enum list widget status signal.
        is_connected = connect(pass_op_node, &RgEditorElement::EnumListWidgetStatusSignal, this, &RgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
        assert(is_connected);

        // Connect the shortcut hot key signal.
        is_connected = connect(this, &RgPipelineStateModelVulkan::HotKeyPressedSignal, static_cast<RgEditorElementEnum*>(pass_op_node), &RgEditorElementEnum::HotKeyPressedSignal);
        assert(is_connected);

        // Add the "depthFailOp" node.
        RgEditorElement* depth_fail_op = new RgEditorElementEnum(parent_, kStrVulkanDepthStencilStateDepthFailOp, stencil_op_enumerators, reinterpret_cast<uint32_t*>(&stencil_op_state->depthFailOp));
        depth_stencil_state_root->AppendChildItem(depth_fail_op);

        // Connect to the splitter moved signal to close the drop down.
        build_view_vulkan = static_cast<RgBuildViewVulkan*>(parent_);
        is_connected = connect(build_view_vulkan, &RgBuildViewVulkan::SplitterMoved, static_cast<RgEditorElementEnum*>(depth_fail_op), &RgEditorElementEnum::HotKeyPressedSignal);

        // Set object name.
        depth_fail_op->setObjectName(kStrDepthFailOp);

        // Connect to the enum list widget status signal.
        is_connected = connect(depth_fail_op, &RgEditorElement::EnumListWidgetStatusSignal, this, &RgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
        assert(is_connected);

        // Connect the shortcut hot key signal.
        is_connected = connect(this, &RgPipelineStateModelVulkan::HotKeyPressedSignal, static_cast<RgEditorElementEnum*>(depth_fail_op), &RgEditorElementEnum::HotKeyPressedSignal);
        assert(is_connected);

        // Add the "compareOp" node.
        const RgEnumValuesVector& compare_op_enumerators = GetCompareOpEnumerators();
        RgEditorElement* compare_op_node = new RgEditorElementEnum(parent_, kStrVulkanDepthStencilStateCompareOp, compare_op_enumerators, reinterpret_cast<uint32_t*>(&stencil_op_state->compareOp));
        depth_stencil_state_root->AppendChildItem(compare_op_node);

        // Connect to the splitter moved signal to close the drop down.
        build_view_vulkan = static_cast<RgBuildViewVulkan*>(parent_);
        is_connected = connect(build_view_vulkan, &RgBuildViewVulkan::SplitterMoved, static_cast<RgEditorElementEnum*>(compare_op_node), &RgEditorElementEnum::HotKeyPressedSignal);

        // Set object name.
        compare_op_node->setObjectName(kStrCompareOpNode);

        // Connect to the enum list widget status signal.
        is_connected = connect(compare_op_node, &RgEditorElement::EnumListWidgetStatusSignal, this, &RgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
        assert(is_connected);

        // Connect the shortcut hot key signal.
        is_connected = connect(this, &RgPipelineStateModelVulkan::HotKeyPressedSignal, static_cast<RgEditorElementEnum*>(compare_op_node), &RgEditorElementEnum::HotKeyPressedSignal);
        assert(is_connected);

        // Add the "compareMask" node.
        RgEditorElement* compare_mask_node = MakeNumericElement(depth_stencil_state_root, kStrVulkanDepthStencilStateCompareMask, &stencil_op_state->compareMask);
        depth_stencil_state_root->AppendChildItem(compare_mask_node);

        // Set object name.
        compare_mask_node->setObjectName(kStrCompareMaskNode);

        // Add the "writeMask" node.
        RgEditorElement* write_mask_node = MakeNumericElement(depth_stencil_state_root, kStrVulkanDepthStencilStateWriteMask, &stencil_op_state->writeMask);
        depth_stencil_state_root->AppendChildItem(write_mask_node);

        // Set object name.
        write_mask_node->setObjectName(kStrWriteMaskNode);

        // Add the "reference" node.
        RgEditorElement* reference_node = MakeNumericElement(depth_stencil_state_root, kStrVulkanDepthStencilStateReference, &stencil_op_state->reference);
        depth_stencil_state_root->AppendChildItem(reference_node);

        // Set object name.
        reference_node->setObjectName(kStrReferenceNode);
    }
}

void RgPipelineStateModelVulkan::InitializeDepthStencilStateCreateInfo(RgEditorElement* root_element, VkPipelineDepthStencilStateCreateInfo* pipeline_depth_stencil_state_create_info)
{
    assert(pipeline_depth_stencil_state_create_info != nullptr);
    if (pipeline_depth_stencil_state_create_info != nullptr)
    {
        RgEditorElement* depth_stencil_state_root = new RgEditorElement(root_element, kStrVulkanPipelineMemberPdepthStencilState);
        root_element->AppendChildItem(depth_stencil_state_root);

        // Set object name.
        depth_stencil_state_root->setObjectName(kStrDepthStencilStateRoot);

        // Add the "flags" node.
        RgEditorElement* flags_item = MakeNumericElement(depth_stencil_state_root, kStrVulkanPipelineMemberFlags, &pipeline_depth_stencil_state_create_info->flags);
        depth_stencil_state_root->AppendChildItem(flags_item);

        // Set object name.
        flags_item->setObjectName(kStrFlagsItem);

        // Add the "depthTestEnable" node.
        RgEditorElementBool* depth_test_enable_node = new RgEditorElementBool(depth_stencil_state_root, kStrVulkanDepthStencilDepthTestEnable, &pipeline_depth_stencil_state_create_info->depthTestEnable);
        depth_stencil_state_root->AppendChildItem(depth_test_enable_node);

        // Set object name.
        depth_test_enable_node->setObjectName(kStrDepthTestEnableNode);

        // Add the "depthWriteEnable" node.
        RgEditorElementBool* depth_write_enable_node = new RgEditorElementBool(depth_stencil_state_root, kStrVulkanDepthStencilDepthWriteEnable, &pipeline_depth_stencil_state_create_info->depthWriteEnable);
        depth_stencil_state_root->AppendChildItem(depth_write_enable_node);

        // Set object name.
        depth_write_enable_node->setObjectName(kStrDepthWriteEnableNode);

        // Add the "depthCompareOp" node.
        const RgEnumValuesVector& compare_op_enumerators = GetCompareOpEnumerators();
        RgEditorElement* depth_compare_op_node = new RgEditorElementEnum(parent_, kStrVulkanDepthStencilDepthCompareOp, compare_op_enumerators, reinterpret_cast<uint32_t*>(&pipeline_depth_stencil_state_create_info->depthCompareOp), parent_);
        depth_stencil_state_root->AppendChildItem(depth_compare_op_node);

        // Connect to the splitter moved signal to close the drop down.
        RgBuildViewVulkan* build_view_vulkan = static_cast<RgBuildViewVulkan*>(parent_);
        bool is_connected = connect(build_view_vulkan, &RgBuildViewVulkan::SplitterMoved, static_cast<RgEditorElementEnum*>(depth_compare_op_node), &RgEditorElementEnum::HotKeyPressedSignal);

        // Set object name.
        depth_compare_op_node->setObjectName(kStrDepthCompareOpNode);

        // Connect to the enum list widget status signal.
        is_connected = connect(depth_compare_op_node, &RgEditorElement::EnumListWidgetStatusSignal, this, &RgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
        assert(is_connected);

        // Connect the shortcut hot key signal.
        is_connected = connect(this, &RgPipelineStateModelVulkan::HotKeyPressedSignal, static_cast<RgEditorElementEnum*>(depth_compare_op_node), &RgEditorElementEnum::HotKeyPressedSignal);
        assert(is_connected);

        // Add the "depthBoundsTestEnable" node.
        RgEditorElementBool* depth_bounds_test_enable_node = new RgEditorElementBool(depth_stencil_state_root, kStrVulkanDepthStencilDepthBoundsTestEnable, &pipeline_depth_stencil_state_create_info->depthBoundsTestEnable);
        depth_stencil_state_root->AppendChildItem(depth_bounds_test_enable_node);

        // Set object name.
        depth_bounds_test_enable_node->setObjectName(kStrDepthBoundsTestEnableNode);

        // Add the "stencilTestEnable" node.
        RgEditorElementBool* stencil_test_enable_node = new RgEditorElementBool(depth_stencil_state_root, kStrVulkanDepthStencilStencilTestEnable, &pipeline_depth_stencil_state_create_info->stencilTestEnable);
        depth_stencil_state_root->AppendChildItem(stencil_test_enable_node);

        // Set object name.
        stencil_test_enable_node->setObjectName(kStrStencilTestEnableNode);

        // Create the root node for the front stencil op state.
        RgEditorElement* front_stencil_op_state_node = new RgEditorElement(depth_stencil_state_root, kStrVulkanDepthStencilFront);
        InitializeStencilOpState(front_stencil_op_state_node, &pipeline_depth_stencil_state_create_info->front);
        depth_stencil_state_root->AppendChildItem(front_stencil_op_state_node);

        // Set object name.
        front_stencil_op_state_node->setObjectName(kStrFrontStencilOpStateNode);

        // Create the root node for the back stencil op state.
        RgEditorElement* back_stencil_op_state_node = new RgEditorElement(depth_stencil_state_root, kStrVulkanDepthStencilBack);
        InitializeStencilOpState(back_stencil_op_state_node, &pipeline_depth_stencil_state_create_info->back);
        depth_stencil_state_root->AppendChildItem(back_stencil_op_state_node);

        // Set object name.
        back_stencil_op_state_node->setObjectName(kStrBackStencilOpStateNode);

        // Add the "minDepthBounds" node.
        RgEditorElement* min_depth_bounds_node = MakeNumericElement(depth_stencil_state_root, kStrVulkanDepthStencilMinDepthBounds, &pipeline_depth_stencil_state_create_info->minDepthBounds);
        depth_stencil_state_root->AppendChildItem(min_depth_bounds_node);

        // Set object name.
        min_depth_bounds_node->setObjectName(kStrMinDepthBoundsNode);

        // Add the "maxDepthBounds" node.
        RgEditorElement* max_depth_bounds_node = MakeNumericElement(depth_stencil_state_root, kStrVulkanDepthStencilMaxDepthBounds, &pipeline_depth_stencil_state_create_info->maxDepthBounds);
        depth_stencil_state_root->AppendChildItem(max_depth_bounds_node);

        // Set object name.
        max_depth_bounds_node->setObjectName(kStrMaxDepthBoundsNode);
    }
}

void RgPipelineStateModelVulkan::InitializeColorBlendStateCreateInfo(RgEditorElement* root_element, VkPipelineColorBlendStateCreateInfo* pipeline_color_blend_state_create_info)
{
    assert(pipeline_color_blend_state_create_info != nullptr);
    if (pipeline_color_blend_state_create_info != nullptr)
    {
        RgEditorElement* color_blend_state_root = new RgEditorElement(root_element, kStrVulkanPipelineMemberPcolorBlendState);
        root_element->AppendChildItem(color_blend_state_root);

        // Set object name.
        color_blend_state_root->setObjectName(kStrColorBlendStateRoot);

        // Add the "flags" node.
        RgEditorElement* flags_item = MakeNumericElement(color_blend_state_root, kStrVulkanPipelineMemberFlags, &pipeline_color_blend_state_create_info->flags);
        color_blend_state_root->AppendChildItem(flags_item);

        // Set object name.
        flags_item->setObjectName(kStrFlagsItem);

        // Add the "logicOpEnable" node.
        RgEditorElementBool* logic_op_enable_node = new RgEditorElementBool(color_blend_state_root, kStrVulkanColorBlendStateLogicOpEnable, &pipeline_color_blend_state_create_info->logicOpEnable);
        color_blend_state_root->AppendChildItem(logic_op_enable_node);

        // Set object name.
        logic_op_enable_node->setObjectName(kStrLogicOpEnableNode);

        // Add the "logicOp" node.
        const RgEnumValuesVector& log_op_enumerators = GetLogicOpEnumerators();
        RgEditorElement* logic_op_node = new RgEditorElementEnum(parent_, kStrVulkanColorBlendStateLogicOp, log_op_enumerators, reinterpret_cast<uint32_t*>(&pipeline_color_blend_state_create_info->logicOp));
        color_blend_state_root->AppendChildItem(logic_op_node);

        // Connect to the splitter moved signal to close the drop down.
        RgBuildViewVulkan* build_view_vulkan = static_cast<RgBuildViewVulkan*>(parent_);
        bool is_connected = connect(build_view_vulkan, &RgBuildViewVulkan::SplitterMoved, static_cast<RgEditorElementEnum*>(logic_op_node), &RgEditorElementEnum::HotKeyPressedSignal);

        // Set object name.
        logic_op_node->setObjectName(kStrLogicOpNode);

        // Connect to the enum list widget status signal.
        is_connected = connect(logic_op_node, &RgEditorElement::EnumListWidgetStatusSignal, this, &RgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
        assert(is_connected);

        // Connect the shortcut hot key signal.
        is_connected = connect(this, &RgPipelineStateModelVulkan::HotKeyPressedSignal, static_cast<RgEditorElementEnum*>(logic_op_node), &RgEditorElementEnum::HotKeyPressedSignal);
        assert(is_connected);

        // Attachment array:
        // Create the attachments array root item.
        RgEditorElementArrayElementAdd* attachments_root_item = new RgEditorElementArrayElementAdd(color_blend_state_root, kStrVulkanColorBlendStateAttachments,
            [=](int element_index) { RemoveElement(pipeline_color_blend_state_create_info->pAttachments, pipeline_color_blend_state_create_info->attachmentCount, element_index); });

        // Set object name.
        attachments_root_item->setObjectName(kStrAttachmentsRootItem);

        // Create the attachmentCount member.
        RgEditorElement* color_blend_attachments_state_count_item = MakeNumericElement(nullptr, kStrVulkanColorBlendStateAttachmentCount,
            &pipeline_color_blend_state_create_info->attachmentCount, [=] { HandlePipelineColorBlendAttachmentCountChanged(attachments_root_item, pipeline_color_blend_state_create_info); });

        // Set object name.
        color_blend_attachments_state_count_item->setObjectName(kStrColorBlendAttachementStateCountItem);

        // Add the attachment array node.
        color_blend_state_root->AppendChildItem(attachments_root_item);

        // Provide the element used to track the dimension of the pAttachments array.
        attachments_root_item->SetArraySizeElement(static_cast<RgEditorElementNumeric<uint32_t>*>(color_blend_attachments_state_count_item));

        // Initialize the pAttachments array rows.
        HandlePipelineColorBlendAttachmentCountChanged(attachments_root_item, pipeline_color_blend_state_create_info, true);

        // Add the blend constants array root node.
        RgEditorElement* blend_constants_root_node = new RgEditorElement(color_blend_state_root, kStrVulkanColorBlendStateBlendConstants);

        // Set object name.
        blend_constants_root_node->setObjectName(kStrBlendConstantsRootNode);

        // Add the blendConstants array item nodes.
        for (uint32_t blend_constant_index = 0; blend_constant_index < 4; ++blend_constant_index)
        {
            std::stringstream element_name_stream;
            element_name_stream << kStrVulkanColorBlendStateBlendConstants;
            element_name_stream << "[";
            element_name_stream << blend_constant_index;
            element_name_stream << "]";

            RgEditorElement* blend_constant_node = MakeNumericElement(blend_constants_root_node, element_name_stream.str().c_str(), &pipeline_color_blend_state_create_info->blendConstants[blend_constant_index]);
            blend_constants_root_node->AppendChildItem(blend_constant_node);

            // Set object name.
            QString name = kStrBlendConstantNode + QString::number(blend_constant_index);
            blend_constant_node->setObjectName(name);
        }

        color_blend_state_root->AppendChildItem(blend_constants_root_node);
    }
}

void RgPipelineStateModelVulkan::HandlePipelineColorBlendAttachmentCountChanged(RgEditorElement* root_element, VkPipelineColorBlendStateCreateInfo* pipeline_color_blend_state_create_info, bool first_init)
{
    ResizeHandler(root_element,
        pipeline_color_blend_state_create_info->attachmentCount,
        pipeline_color_blend_state_create_info->pAttachments,
        kStrVulkanColorBlendAttachmentState,
        std::bind(&RgPipelineStateModelVulkan::InitializePipelineBlendAttachmentStateCreateInfo, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
        first_init);
}

void RgPipelineStateModelVulkan::InitializePipelineBlendAttachmentStateCreateInfo(RgEditorElement* root_element, VkPipelineColorBlendAttachmentState* base_color_blend_attachment_state, int item_index)
{
    assert(root_element != nullptr);
    assert(base_color_blend_attachment_state != nullptr);
    if (root_element != nullptr && base_color_blend_attachment_state != nullptr)
    {
        VkPipelineColorBlendAttachmentState* offset_color_blend_attachment_state = base_color_blend_attachment_state + item_index;
        assert(offset_color_blend_attachment_state != nullptr);
        if (offset_color_blend_attachment_state != nullptr)
        {
            // Add the "blendEnable" node.
            RgEditorElementBool* blend_enable_node = new RgEditorElementBool(root_element, kStrVulkanColorBlendAttachmentStateBlendEnable, &offset_color_blend_attachment_state->blendEnable);
            root_element->AppendChildItem(blend_enable_node);

            // Set object name.
            blend_enable_node->setObjectName(kStrBlendEnableNode);

            // Get the blend factor enumerators.
            const RgEnumValuesVector& blend_factor_enumerators = GetBlendFactorEnumerators();

            // Get the blend op enumerators.
            const RgEnumValuesVector& blend_op_enumerators = GetBlendOpEnumerators();

            // Add the "srcColorBlendFactor" node.
            RgEditorElement* src_color_blend_factor_node = new RgEditorElementEnum(parent_, kStrVulkanColorBlendAttachmentStateSrcColorBlendFactor, blend_factor_enumerators, reinterpret_cast<uint32_t*>(&offset_color_blend_attachment_state->srcColorBlendFactor));
            root_element->AppendChildItem(src_color_blend_factor_node);

            // Connect to the splitter moved signal to close the drop down.
            RgBuildViewVulkan* build_view_vulkan = static_cast<RgBuildViewVulkan*>(parent_);
            bool is_connected = connect(build_view_vulkan, &RgBuildViewVulkan::SplitterMoved, static_cast<RgEditorElementEnum*>(src_color_blend_factor_node), &RgEditorElementEnum::HotKeyPressedSignal);

            // Set object name.
            src_color_blend_factor_node->setObjectName(kStrSrcColorBlendFactorNode);

            // Connect to the enum list widget status signal.
            is_connected = connect(src_color_blend_factor_node, &RgEditorElement::EnumListWidgetStatusSignal, this, &RgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
            assert(is_connected);

            // Connect the shortcut hot key signal.
            is_connected = connect(this, &RgPipelineStateModelVulkan::HotKeyPressedSignal, static_cast<RgEditorElementEnum*>(src_color_blend_factor_node), &RgEditorElementEnum::HotKeyPressedSignal);
            assert(is_connected);

            // Add the "dstColorBlendFactor" node.
            RgEditorElement* dst_color_blend_factor_node = new RgEditorElementEnum(parent_, kStrVulkanColorBlendAttachmentStateDstColorBlendFactor, blend_factor_enumerators, reinterpret_cast<uint32_t*>(&offset_color_blend_attachment_state->dstColorBlendFactor));
            root_element->AppendChildItem(dst_color_blend_factor_node);

            // Connect to the splitter moved signal to close the drop down.
            build_view_vulkan = static_cast<RgBuildViewVulkan*>(parent_);
            is_connected = connect(build_view_vulkan, &RgBuildViewVulkan::SplitterMoved, static_cast<RgEditorElementEnum*>(dst_color_blend_factor_node), &RgEditorElementEnum::HotKeyPressedSignal);

            // Set object name.
            dst_color_blend_factor_node->setObjectName(kStrDstColorBlendFactorNode);

            // Connect to the enum list widget status signal.
            is_connected = connect(dst_color_blend_factor_node, &RgEditorElement::EnumListWidgetStatusSignal, this, &RgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
            assert(is_connected);

            // Connect the shortcut hot key signal.
            is_connected = connect(this, &RgPipelineStateModelVulkan::HotKeyPressedSignal, static_cast<RgEditorElementEnum*>(dst_color_blend_factor_node), &RgEditorElementEnum::HotKeyPressedSignal);
            assert(is_connected);

            // Add the "colorBlendOp" node.
            RgEditorElement* color_blend_op_node = new RgEditorElementEnum(parent_, kStrVulkanColorBlendAttachmentStateColorBlendOp, blend_op_enumerators, reinterpret_cast<uint32_t*>(&offset_color_blend_attachment_state->colorBlendOp));
            root_element->AppendChildItem(color_blend_op_node);

            // Connect to the splitter moved signal to close the drop down.
            build_view_vulkan = static_cast<RgBuildViewVulkan*>(parent_);
            is_connected = connect(build_view_vulkan, &RgBuildViewVulkan::SplitterMoved, static_cast<RgEditorElementEnum*>(color_blend_op_node), &RgEditorElementEnum::HotKeyPressedSignal);

            // Set object name.
            color_blend_op_node->setObjectName(kStrColorBlendOpNode);

            // Connect to the enum list widget status signal.
            is_connected = connect(color_blend_op_node, &RgEditorElement::EnumListWidgetStatusSignal, this, &RgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
            assert(is_connected);

            // Connect the shortcut hot key signal.
            is_connected = connect(this, &RgPipelineStateModelVulkan::HotKeyPressedSignal, static_cast<RgEditorElementEnum*>(color_blend_op_node), &RgEditorElementEnum::HotKeyPressedSignal);
            assert(is_connected);

            // Add the "srcAlphaBlendFactor" node.
            RgEditorElement* src_alpha_blend_factor_node = new RgEditorElementEnum(parent_, kStrVulkanColorBlendAttachmentStateSrcAlphaBlendFactor, blend_factor_enumerators, reinterpret_cast<uint32_t*>(&offset_color_blend_attachment_state->srcAlphaBlendFactor));
            root_element->AppendChildItem(src_alpha_blend_factor_node);

            // Connect to the splitter moved signal to close the drop down.
            build_view_vulkan = static_cast<RgBuildViewVulkan*>(parent_);
            is_connected = connect(build_view_vulkan, &RgBuildViewVulkan::SplitterMoved, static_cast<RgEditorElementEnum*>(src_alpha_blend_factor_node), &RgEditorElementEnum::HotKeyPressedSignal);

            // Set object name.
            src_alpha_blend_factor_node->setObjectName(kStrSrcAlphaBlendFactorNode);

            // Connect to the enum list widget status signal.
            is_connected = connect(src_alpha_blend_factor_node, &RgEditorElement::EnumListWidgetStatusSignal, this, &RgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
            assert(is_connected);

            // Connect the shortcut hot key signal.
            is_connected = connect(this, &RgPipelineStateModelVulkan::HotKeyPressedSignal, static_cast<RgEditorElementEnum*>(src_alpha_blend_factor_node), &RgEditorElementEnum::HotKeyPressedSignal);
            assert(is_connected);

            // Add the "dstAlphaBlendFactor" node.
            RgEditorElement* dst_alpha_blend_factor_node = new RgEditorElementEnum(parent_, kStrVulkanColorBlendAttachmentStateDstAlphaBlendFactor, blend_factor_enumerators, reinterpret_cast<uint32_t*>(&offset_color_blend_attachment_state->dstAlphaBlendFactor));
            root_element->AppendChildItem(dst_alpha_blend_factor_node);

            // Connect to the splitter moved signal to close the drop down.
            build_view_vulkan = static_cast<RgBuildViewVulkan*>(parent_);
            is_connected = connect(build_view_vulkan, &RgBuildViewVulkan::SplitterMoved, static_cast<RgEditorElementEnum*>(dst_alpha_blend_factor_node), &RgEditorElementEnum::HotKeyPressedSignal);

            // Set object name.dst_alpha_blend
            dst_alpha_blend_factor_node->setObjectName(kStrDstAlphaBlendFactorNode);

            // Connect to the enum list widget status signal.
            is_connected = connect(dst_alpha_blend_factor_node, &RgEditorElement::EnumListWidgetStatusSignal, this, &RgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
            assert(is_connected);

            // Connect the shortcut hot key signal.
            is_connected = connect(this, &RgPipelineStateModelVulkan::HotKeyPressedSignal, static_cast<RgEditorElementEnum*>(dst_alpha_blend_factor_node), &RgEditorElementEnum::HotKeyPressedSignal);
            assert(is_connected);

            // Add the "alphaBlendOp" node.
            RgEditorElement* alpha_blend_op_node = new RgEditorElementEnum(parent_, kStrVulkanColorBlendAttachmentStateAlphaBlendOp, blend_op_enumerators, reinterpret_cast<uint32_t*>(&offset_color_blend_attachment_state->alphaBlendOp));
            root_element->AppendChildItem(alpha_blend_op_node);

            // Connect to the splitter moved signal to close the drop down.
            build_view_vulkan = static_cast<RgBuildViewVulkan*>(parent_);
            is_connected = connect(build_view_vulkan, &RgBuildViewVulkan::SplitterMoved, static_cast<RgEditorElementEnum*>(alpha_blend_op_node), &RgEditorElementEnum::HotKeyPressedSignal);

            // Set object name.
            alpha_blend_op_node->setObjectName(kStrAlphaBlendOpNode);

            // Connect to the enum list widget status signal.
            is_connected = connect(alpha_blend_op_node, &RgEditorElement::EnumListWidgetStatusSignal, this, &RgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
            assert(is_connected);

            // Connect the shortcut hot key signal.
            is_connected = connect(this, &RgPipelineStateModelVulkan::HotKeyPressedSignal, static_cast<RgEditorElementEnum*>(alpha_blend_op_node), &RgEditorElementEnum::HotKeyPressedSignal);
            assert(is_connected);

            // Add the "colorWriteMask" member values.
            const RgEnumValuesVector& color_component_flag_enumerators = GetColorComponentFlagEnumerators();
            RgEditorElement* color_component_write_mask = new RgEditorElementEnum(parent_, kStrVulkanColorBlendAttachmentStateColorWriteMask, color_component_flag_enumerators, reinterpret_cast<uint32_t*>(&offset_color_blend_attachment_state->colorWriteMask), true);
            root_element->AppendChildItem(color_component_write_mask);

            // Connect to the splitter moved signal to close the drop down.
            build_view_vulkan = static_cast<RgBuildViewVulkan*>(parent_);
            is_connected = connect(build_view_vulkan, &RgBuildViewVulkan::SplitterMoved, static_cast<RgEditorElementEnum*>(color_component_write_mask), &RgEditorElementEnum::HotKeyPressedSignal);

            // Set object name.
            color_component_write_mask->setObjectName(kStrColorComponentWriteMask);

            // Connect to the enum list widget status signal.
            is_connected = connect(color_component_write_mask, &RgEditorElement::EnumListWidgetStatusSignal, this, &RgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
            assert(is_connected);

            // Connect the shortcut hot key signal.
            is_connected = connect(this, &RgPipelineStateModelVulkan::HotKeyPressedSignal, static_cast<RgEditorElementEnum*>(color_component_write_mask), &RgEditorElementEnum::HotKeyPressedSignal);
            assert(is_connected);
        }
    }
}

RgEditorElement* RgPipelineStateModelVulkan::InitializeComputePipelineCreateInfo(QWidget* parent)
{
    RgEditorElement* create_info_root_node = nullptr;

    assert(compute_pipeline_state_ != nullptr);
    if (compute_pipeline_state_ != nullptr)
    {
        // The create info root node that all other create info elements are attached to.
        create_info_root_node = new RgEditorElement(parent, kStrVulkanComputePipelineState);

        // Set object name.
        create_info_root_node->setObjectName(kStrCreateInfoRootNode);

        // Add the Compute Pipeline create info root node.
        InitializeVkComputePipelineCreateInfo(create_info_root_node, compute_pipeline_state_);

        // Add the Pipeline Layout create info root node.
        VkPipelineLayoutCreateInfo* pipeline_layout_create_info = compute_pipeline_state_->GetPipelineLayoutCreateInfo();
        assert(pipeline_layout_create_info != nullptr);
        if (pipeline_layout_create_info != nullptr)
        {
            InitializePipelineLayoutCreateInfo(create_info_root_node, pipeline_layout_create_info);
        }

        // Initialize rows related to Descriptor Set Layout configuration.
        InitializeDescriptorSetLayoutCreateInfoArray(create_info_root_node, compute_pipeline_state_);
    }

    return create_info_root_node;
}

bool RgPipelineStateModelVulkan::LoadPipelineStateFile(QWidget* parent, const std::string& pso_file_path, RgPipelineType pipeline_type, std::string& error_string)
{
    bool is_ok = false;

    try
    {
        // Assign the type of pipeline being loaded from file.
        pipeline_type_ = pipeline_type;

        if (pipeline_type_ == RgPipelineType::kGraphics)
        {
            RgPsoGraphicsVulkan* graphics_pipeline_state = nullptr;

            // Load the graphics pipeline state file.
            is_ok = RgPsoSerializerVulkan::ReadStructureFromFile(pso_file_path, &graphics_pipeline_state, error_string);

            assert(is_ok);
            if (is_ok)
            {
                // Assign the new graphics pipeline state in the model.
                graphics_pipeline_state_ = graphics_pipeline_state;

                // Initialize the create info structure to bind it to the model.
                root_item_ = InitializeGraphicsPipelineCreateInfo(parent);
            }
            else
            {
                RgUtils::ShowErrorMessageBox(error_string.c_str());
            }
        }
        else if (pipeline_type_ == RgPipelineType::kCompute)
        {
            RgPsoComputeVulkan* compute_pipeline_state = nullptr;

            // Load the compute pipeline state file.
            is_ok = RgPsoSerializerVulkan::ReadStructureFromFile(pso_file_path, &compute_pipeline_state, error_string);

            assert(is_ok);
            if (is_ok)
            {
                // Assign the new compute pipeline state in the model.
                compute_pipeline_state_ = compute_pipeline_state;

                // Initialize the create info structure to bind it to the model.
                root_item_ = InitializeComputePipelineCreateInfo(parent);
            }
            else
            {
                RgUtils::ShowErrorMessageBox(error_string.c_str());
            }
        }
        else
        {
            assert(false);
            error_string = kStrErrCannotDeterminePipelineType;
        }
    }
    catch (...)
    {
        error_string = kStrErrCannotFailedToLoadPipelineStateFile;
    }

    return is_ok;
}

bool RgPipelineStateModelVulkan::SavePipelineStateFile(const std::string& pso_file_path, std::string& error_string)
{
    bool is_ok = false;

    try
    {
        std::string validation_error_string;
        bool is_valid = CheckValidPipelineState(validation_error_string);

        assert(is_valid);
        if (is_valid)
        {
            if (pipeline_type_ == RgPipelineType::kGraphics)
            {
                // Save the graphics pipeline state file.
                is_ok = RgPsoSerializerVulkan::WriteStructureToFile(graphics_pipeline_state_, pso_file_path, error_string);
            }
            else if (pipeline_type_ == RgPipelineType::kCompute)
            {
                // Save the compute pipeline state file.
                is_ok = RgPsoSerializerVulkan::WriteStructureToFile(compute_pipeline_state_, pso_file_path, error_string);
            }
            else
            {
                assert(false);
                error_string = kStrErrCannotDeterminePipelineType;
            }
        }
        else
        {
            std::stringstream error_stream;
            error_stream << kStrErrFailedToValidate << std::endl;
            error_stream << validation_error_string;
            error_string = error_stream.str();
        }
    }
    catch (...)
    {
        error_string = kStrErrFailedToSavePipelineStateFile;
    }

    return is_ok;
}

void RgPipelineStateModelVulkan::InitializeVkComputePipelineCreateInfo(RgEditorElement* root_element, RgPsoComputeVulkan* compute_pipeline_create_info)
{
    assert(compute_pipeline_create_info != nullptr);
    if (compute_pipeline_create_info != nullptr)
    {
        VkComputePipelineCreateInfo* vk_compute_pipeline_create_info = static_cast<VkComputePipelineCreateInfo*>(compute_pipeline_create_info->GetComputePipelineCreateInfo());
        assert(vk_compute_pipeline_create_info != nullptr);
        if (vk_compute_pipeline_create_info != nullptr)
        {
            RgEditorElement* vk_compute_pipeline_create_info_root = new RgEditorElement(root_element, kStrVulkanComputePipelineCreateInfo);
            root_element->AppendChildItem(vk_compute_pipeline_create_info_root);

            // Set object name.
            vk_compute_pipeline_create_info_root->setObjectName(kStrVkComputePipelineCreateInfoRoot);

            // Add the "flags" member.
            const RgEnumValuesVector& pipeline_flags_enumerators = GetPipelineCreateFlagEnumerators();
            RgEditorElement* flags_item = new RgEditorElementEnum(parent_, kStrVulkanPipelineMemberFlags, pipeline_flags_enumerators, reinterpret_cast<uint32_t*>(&vk_compute_pipeline_create_info->flags), true);
            vk_compute_pipeline_create_info_root->AppendChildItem(flags_item);

            // Connect to the splitter moved signal to close the drop down.
            RgBuildViewVulkan* build_view_vulkan = static_cast<RgBuildViewVulkan*>(parent_);
            bool is_connected = connect(build_view_vulkan, &RgBuildViewVulkan::SplitterMoved, static_cast<RgEditorElementEnum*>(flags_item), &RgEditorElementEnum::HotKeyPressedSignal);

            // Set object name.
            flags_item->setObjectName(kStrFlagsItem);

            // Connect to the flags enum list widget status signal.
            is_connected = connect(flags_item, &RgEditorElement::EnumListWidgetStatusSignal, this, &RgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
            assert(is_connected);

            // Connect the shortcut hot key signal.
            is_connected = connect(this, &RgPipelineStateModelVulkan::HotKeyPressedSignal, static_cast<RgEditorElementEnum*>(flags_item), &RgEditorElementEnum::HotKeyPressedSignal);
            assert(is_connected);

            // Add the "basePipelineIndex" member.
            RgEditorElement* base_pipeline_index_create_info = MakeNumericElement(vk_compute_pipeline_create_info_root, kStrVulkanPipelineMemberBaseIndex, &vk_compute_pipeline_create_info->basePipelineIndex);
            vk_compute_pipeline_create_info_root->AppendChildItem(base_pipeline_index_create_info);

            // Set object name.
            base_pipeline_index_create_info->setObjectName(kStrBasePipelineIndexCreateInfo);
        }
    }
}

void RgPipelineStateModelVulkan::InitializePipelineLayoutCreateInfo(RgEditorElement* root_element, VkPipelineLayoutCreateInfo* pipeline_layout_create_info)
{
    assert(pipeline_layout_create_info != nullptr);
    if (pipeline_layout_create_info != nullptr)
    {
        RgEditorElement* pipeline_layout_create_info_root = new RgEditorElement(root_element, kStrVulkanPipelineLayoutCreateInfo);
        root_element->AppendChildItem(pipeline_layout_create_info_root);

        // Set object name.
        pipeline_layout_create_info_root->setObjectName(kStrPipelineLayoutCreateInfoRoot);

        // Add the "flags" member.
        RgEditorElement* flags_item = MakeNumericElement(pipeline_layout_create_info_root, kStrVulkanPipelineMemberFlags, &pipeline_layout_create_info->flags);
        pipeline_layout_create_info_root->AppendChildItem(flags_item);

        // Set object name.
        flags_item->setObjectName(kStrFlagsItem);

        // Descriptor Set Layout array:
        // Create the layout array root item.
        RgEditorElementArrayElementAdd* descriptor_set_layouts_root_item = new RgEditorElementArrayElementAdd(pipeline_layout_create_info_root, kStrVulkanPipelineLayoutSetLayouts,
            [=](int element_index) { RemoveElement(pipeline_layout_create_info->pSetLayouts, pipeline_layout_create_info->setLayoutCount, element_index); });

        // Set object name.
        descriptor_set_layouts_root_item->setObjectName(kStrDescriptorSetLayoutsRootItem);

        // Create the setLayoutCount member.
        RgEditorElement* descriptor_set_layout_count_item = MakeNumericElement(nullptr, kStrVulkanPipelineLayoutDescriptorSetLayoutCount,
            &pipeline_layout_create_info->setLayoutCount, [=] { HandlePipelineLayoutDescriptorSetLayoutCountChanged(descriptor_set_layouts_root_item, pipeline_layout_create_info); });

        // Set object name.
        descriptor_set_layout_count_item->setObjectName(kStrDescriptorSetLayoutCountItem);

        // Provide the element used to track the dimension of the Descriptor Set Layouts array.
        descriptor_set_layouts_root_item->SetArraySizeElement(static_cast<RgEditorElementNumeric<uint32_t>*>(descriptor_set_layout_count_item));

        // Initialize the Descriptor Set Layout array rows.
        HandlePipelineLayoutDescriptorSetLayoutCountChanged(descriptor_set_layouts_root_item, pipeline_layout_create_info, true);

        // Add the descriptor set layouts array node.
        pipeline_layout_create_info_root->AppendChildItem(descriptor_set_layouts_root_item);

        // Push Constants array:
        // Create the push constants array root item.
        RgEditorElementArrayElementAdd* push_constants_array_root_item = new RgEditorElementArrayElementAdd(pipeline_layout_create_info_root, kStrVulkanPipelineLayoutPushConstantRanges,
            [=](int element_index) { RemoveElement(pipeline_layout_create_info->pPushConstantRanges, pipeline_layout_create_info->pushConstantRangeCount, element_index); });

        // Set object name.
        push_constants_array_root_item->setObjectName(kStrPushConstantArrayRootItem);

        // Create the pushConstantRangeCount member.
        RgEditorElement* push_constants_count_item = MakeNumericElement(nullptr, kStrVulkanPipelineLayoutPushConstantRangeCount,
            &pipeline_layout_create_info->pushConstantRangeCount, [=] { HandlePushConstantsCountChanged(push_constants_array_root_item, pipeline_layout_create_info); });

        // Set object name.
        push_constants_count_item->setObjectName(kStrPushConstantsCountItem);

        // Provide the element used to track the dimension of the Push Constants array.
        push_constants_array_root_item->SetArraySizeElement(static_cast<RgEditorElementNumeric<uint32_t>*>(push_constants_count_item));

        // Initialize the Push Constants array rows.
        HandlePushConstantsCountChanged(push_constants_array_root_item, pipeline_layout_create_info, true);

        // Add the descriptor set layouts array node.
        pipeline_layout_create_info_root->AppendChildItem(push_constants_array_root_item);
    }
}

void RgPipelineStateModelVulkan::InitializeDescriptorSetLayoutCreateInfo(RgEditorElement* root_element, VkDescriptorSetLayoutCreateInfo* descriptor_set_layout_create_info)
{
    assert(root_element != nullptr);
    assert(descriptor_set_layout_create_info != nullptr);
    if (root_element != nullptr && descriptor_set_layout_create_info != nullptr)
    {
        // Add the "flags" node.
        const RgEnumValuesVector& descriptor_set_layout_create_flags = GetDescriptorSetLayoutCreateFlagEnumerators();
        RgEditorElement* descriptor_set_layout_flags_node = new RgEditorElementEnum(parent_, kStrVulkanPipelineMemberFlags, descriptor_set_layout_create_flags, reinterpret_cast<uint32_t*>(&descriptor_set_layout_create_info->flags), true);
        root_element->AppendChildItem(descriptor_set_layout_flags_node);

        // Connect to the splitter moved signal to close the drop down.
        RgBuildViewVulkan* build_view_vulkan = static_cast<RgBuildViewVulkan*>(parent_);
        bool is_connected = connect(build_view_vulkan, &RgBuildViewVulkan::SplitterMoved, static_cast<RgEditorElementEnum*>(descriptor_set_layout_flags_node), &RgEditorElementEnum::HotKeyPressedSignal);

        // Set object name.
        descriptor_set_layout_flags_node->setObjectName(kStrDescriptorSetLayoutFlagsNode);

        // Connect to the enum list widget status signal.
        is_connected = connect(descriptor_set_layout_flags_node, &RgEditorElement::EnumListWidgetStatusSignal, this, &RgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
        assert(is_connected);

        // Connect the shortcut hot key signal.
        is_connected = connect(this, &RgPipelineStateModelVulkan::HotKeyPressedSignal, static_cast<RgEditorElementEnum*>(descriptor_set_layout_flags_node), &RgEditorElementEnum::HotKeyPressedSignal);
        assert(is_connected);

        // Binding array:
        // Create the binding array root item.
        RgEditorElementArrayElementAdd* descriptor_set_layout_bindings_root_item = new RgEditorElementArrayElementAdd(root_element, kStrVulkanPipelineLayoutDescriptorSetLayoutBindings,
            [=](int element_index) { RemoveElement(descriptor_set_layout_create_info->pBindings, descriptor_set_layout_create_info->bindingCount, element_index); });

        // Set object name.
        descriptor_set_layout_bindings_root_item->setObjectName(kStrDescriptorSetLayoutBindingsRootItem);

        // Create the bindingCount member.
        RgEditorElement* descriptor_set_binding_count_item = MakeNumericElement(nullptr, kStrVulkanPipelineLayoutDescriptorSetLayoutBindingCount,
            &descriptor_set_layout_create_info->bindingCount, [=] { HandleDescriptorSetLayoutBindingCountChanged(descriptor_set_layout_bindings_root_item, descriptor_set_layout_create_info); });

        // Set object name.
        descriptor_set_binding_count_item->setObjectName(kStrDescriptorSetBindingCountItem);

        // Provide the element used to track the dimension of the pDescriptorSetBindings array.
        descriptor_set_layout_bindings_root_item->SetArraySizeElement(static_cast<RgEditorElementNumeric<uint32_t>*>(descriptor_set_binding_count_item));

        // Initialize the pDescriptorSetBindings array rows.
        HandleDescriptorSetLayoutBindingCountChanged(descriptor_set_layout_bindings_root_item, descriptor_set_layout_create_info, true);

        // Add the descriptor set layout bindings array node.
        root_element->AppendChildItem(descriptor_set_layout_bindings_root_item);
    }
}

void RgPipelineStateModelVulkan::HandlePipelineLayoutDescriptorSetLayoutCountChanged(RgEditorElement* root_element, VkPipelineLayoutCreateInfo* pipeline_layout_create_info, bool first_init)
{
    ResizeHandler(root_element,
        pipeline_layout_create_info->setLayoutCount,
        pipeline_layout_create_info->pSetLayouts,
        kStrVulkanDescriptorSetLayoutHandle,
        std::bind(&RgPipelineStateModelVulkan::InitializeDescriptorSetLayout, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
        first_init);
}

void RgPipelineStateModelVulkan::HandlePushConstantsCountChanged(RgEditorElement* root_element, VkPipelineLayoutCreateInfo* pipeline_layout_create_info, bool first_init)
{
    ResizeHandler(root_element,
        pipeline_layout_create_info->pushConstantRangeCount,
        pipeline_layout_create_info->pPushConstantRanges,
        kStrVulkanPushConstantRangeType,
        std::bind(&RgPipelineStateModelVulkan::InitializePushConstantRange, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
        first_init);
}

void RgPipelineStateModelVulkan::HandleDescriptorSetLayoutBindingCountChanged(RgEditorElement* root_element, VkDescriptorSetLayoutCreateInfo* descriptor_set_layout_create_info, bool first_init)
{
    ResizeHandler(root_element,
        descriptor_set_layout_create_info->bindingCount,
        descriptor_set_layout_create_info->pBindings,
        kStrVulkanDescriptorSetLayoutBindingType,
        std::bind(&RgPipelineStateModelVulkan::InitializeDescriptorSetLayoutBinding, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
        first_init);
}

void RgPipelineStateModelVulkan::InitializeDescriptorSetLayoutBinding(RgEditorElement* root_element, VkDescriptorSetLayoutBinding* base_descriptor_set_layout, int item_index)
{
    assert(root_element != nullptr);
    assert(base_descriptor_set_layout != nullptr);
    if (root_element != nullptr && base_descriptor_set_layout != nullptr)
    {
        VkDescriptorSetLayoutBinding* offset_descriptor_set_layout = base_descriptor_set_layout + item_index;
        assert(offset_descriptor_set_layout != nullptr);
        if (offset_descriptor_set_layout != nullptr)
        {
            // Add the "binding" node.
            RgEditorElement* descriptor_set_layout_binding_index_node = MakeNumericElement(root_element, kStrVulkanDescriptorSetLayoutBinding, &offset_descriptor_set_layout->binding);
            root_element->AppendChildItem(descriptor_set_layout_binding_index_node);

            // Set object name.
            descriptor_set_layout_binding_index_node->setObjectName(kStrDescriptorSetLayoutBindingIndexNode);

            // Add the "descriptorType" node.
            const RgEnumValuesVector& descriptor_type_enumerators = GetDescriptorTypeEnumerators();
            RgEditorElement* descriptor_type_node = new RgEditorElementEnum(parent_, kStrVulkanDescriptorSetLayoutBindingDescriptorType, descriptor_type_enumerators, reinterpret_cast<uint32_t*>(&offset_descriptor_set_layout->descriptorType));
            root_element->AppendChildItem(descriptor_type_node);

            // Connect to the splitter moved signal to close the drop down.
            RgBuildViewVulkan* build_view_vulkan = static_cast<RgBuildViewVulkan*>(parent_);
            bool is_connected = connect(build_view_vulkan, &RgBuildViewVulkan::SplitterMoved, static_cast<RgEditorElementEnum*>(descriptor_type_node), &RgEditorElementEnum::HotKeyPressedSignal);

            // Set object name.
            descriptor_type_node->setObjectName(kStrDescriptorTypeNode);

            // Connect to the enum list widget status signal.
            is_connected = connect(descriptor_type_node, &RgEditorElement::EnumListWidgetStatusSignal, this, &RgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
            assert(is_connected);

            // Connect the shortcut hot key signal.
            is_connected = connect(this, &RgPipelineStateModelVulkan::HotKeyPressedSignal, static_cast<RgEditorElementEnum*>(descriptor_type_node), &RgEditorElementEnum::HotKeyPressedSignal);
            assert(is_connected);

            // Add the "descriptorCount" node.
            RgEditorElement* descriptor_set_layout_binding_count_node = MakeNumericElement(root_element, kStrVulkanDescriptorSetLayoutBindingDescriptorCount, &offset_descriptor_set_layout->descriptorCount);
            root_element->AppendChildItem(descriptor_set_layout_binding_count_node);

            // Set object name.
            descriptor_set_layout_binding_count_node->setObjectName(kStrDescriptionSetLayoutBindingCountNode);

            // Add the "stageFlags" member node.
            const RgEnumValuesVector& stage_flag_enumerators = GetShaderStageFlagEnumerators();
            RgEditorElement* stage_flags_node = new RgEditorElementEnum(parent_, kStrVulkanPipelineLayoutStageFlags, stage_flag_enumerators, reinterpret_cast<uint32_t*>(&offset_descriptor_set_layout->stageFlags), true);
            root_element->AppendChildItem(stage_flags_node);

            // Connect to the splitter moved signal to close the drop down.
            build_view_vulkan = static_cast<RgBuildViewVulkan*>(parent_);
            is_connected = connect(build_view_vulkan, &RgBuildViewVulkan::SplitterMoved, static_cast<RgEditorElementEnum*>(stage_flags_node), &RgEditorElementEnum::HotKeyPressedSignal);

            // Set object name.
            stage_flags_node->setObjectName(kStrStageFlagsNode);

            // Connect to the enum list widget status signal.
            is_connected = connect(stage_flags_node, &RgEditorElement::EnumListWidgetStatusSignal, this, &RgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
            assert(is_connected);

            // Connect the shortcut hot key signal.
            is_connected = connect(this, &RgPipelineStateModelVulkan::HotKeyPressedSignal, static_cast<RgEditorElementEnum*>(stage_flags_node), &RgEditorElementEnum::HotKeyPressedSignal);
            assert(is_connected);
        }
    }
}

void RgPipelineStateModelVulkan::InitializeDescriptorSetLayout(RgEditorElement* root_element, VkDescriptorSetLayout* base_descriptor_set_layout, int item_index)
{
    assert(root_element != nullptr);
    assert(base_descriptor_set_layout != nullptr);
    if (root_element != nullptr && base_descriptor_set_layout != nullptr)
    {
        VkDescriptorSetLayout* offset_descriptor_set_layout = base_descriptor_set_layout + item_index;
        assert(offset_descriptor_set_layout != nullptr);
        if (offset_descriptor_set_layout != nullptr)
        {
            // Add the "pDescriptorSetLayout" handle member.
            RgEditorElement* descriptor_set_layout_handle_node = MakeNumericElement(root_element, kStrVulkanDescriptorSetLayoutHandle, reinterpret_cast<uint32_t*>(offset_descriptor_set_layout));
            root_element->AppendChildItem(descriptor_set_layout_handle_node);

            // Set object name.
            descriptor_set_layout_handle_node->setObjectName(kStrDescriptorSetLayoutHandleNode);
        }
    }
}

void RgPipelineStateModelVulkan::InitializePushConstantRange(RgEditorElement* root_element, VkPushConstantRange* base_push_constant, int item_index)
{
    assert(root_element != nullptr);
    assert(base_push_constant != nullptr);
    if (root_element != nullptr && base_push_constant != nullptr)
    {
        VkPushConstantRange* offset_push_constant = base_push_constant + item_index;
        assert(offset_push_constant != nullptr);
        if (offset_push_constant != nullptr)
        {
            // Add the "stageFlags" member node.
            const RgEnumValuesVector& stage_flag_enumerators = GetShaderStageFlagEnumerators();
            RgEditorElement* stage_flags_node = new RgEditorElementEnum(parent_, kStrVulkanPipelineLayoutStageFlags, stage_flag_enumerators, reinterpret_cast<uint32_t*>(&offset_push_constant->stageFlags), true);
            root_element->AppendChildItem(stage_flags_node);

            // Connect to the splitter moved signal to close the drop down.
            RgBuildViewVulkan* build_view_vulkan = static_cast<RgBuildViewVulkan*>(parent_);
            bool is_connected = connect(build_view_vulkan, &RgBuildViewVulkan::SplitterMoved, static_cast<RgEditorElementEnum*>(stage_flags_node), &RgEditorElementEnum::HotKeyPressedSignal);

            // Set object name.
            stage_flags_node->setObjectName(kStrStageFlagsNode);

            // Connect to the enum list widget status signal.
            is_connected = connect(stage_flags_node, &RgEditorElement::EnumListWidgetStatusSignal, this, &RgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
            assert(is_connected);

            // Connect the shortcut hot key signal.
            is_connected = connect(this, &RgPipelineStateModelVulkan::HotKeyPressedSignal, static_cast<RgEditorElementEnum*>(stage_flags_node), &RgEditorElementEnum::HotKeyPressedSignal);
            assert(is_connected);

            // Add the "offset" member.
            RgEditorElement* offset_node = MakeNumericElement(root_element, kStrVulkanPipelineLayoutPushConstantOffset, &offset_push_constant->offset);
            root_element->AppendChildItem(offset_node);

            // Set object name.
            offset_node->setObjectName(kStrOffsetNode);

            // Add the "size" member.
            RgEditorElement* size_node = MakeNumericElement(root_element, kStrVulkanPipelineLayoutPushConstantSize, &offset_push_constant->size);
            root_element->AppendChildItem(size_node);

            // Set object name.
            size_node->setObjectName(kStrSizeNode);
        }
    }
}

void RgPipelineStateModelVulkan::InitializeRenderPassCreateInfo(RgEditorElement* root_element, VkRenderPassCreateInfo* render_pass_create_info)
{
    assert(render_pass_create_info != nullptr);
    if (render_pass_create_info != nullptr)
    {
        RgEditorElement* render_pass_create_info_root = new RgEditorElement(root_element, kStrVulkanRenderPassCreateInfo);
        root_element->AppendChildItem(render_pass_create_info_root);

        // Set object name.
        render_pass_create_info_root->setObjectName(kStrRenderPassCreateInfoRoot);

        // Add the "flags" member.
        RgEditorElement* flags_item = MakeNumericElement(render_pass_create_info_root, kStrVulkanPipelineMemberFlags, &render_pass_create_info->flags);
        render_pass_create_info_root->AppendChildItem(flags_item);

        // Set object name.
        flags_item->setObjectName(kStrFlagsItem);

        // Attachment array:
        // Create the attachments array root item.
        RgEditorElementArrayElementAdd* attachments_root_item = new RgEditorElementArrayElementAdd(render_pass_create_info_root, kStrVulkanRenderPassAttachments,
            [=](int element_index) { RemoveElement(render_pass_create_info->pAttachments, render_pass_create_info->attachmentCount, element_index); });

        // Set object name.
        attachments_root_item->setObjectName(kStrColorAttachmentsRootItem);

        // Create the attachmentCount member.
        RgEditorElement* attachment_count_item = MakeNumericElement(nullptr, kStrVulkanRenderPassAttachmentCount,
            &render_pass_create_info->attachmentCount, [=] { HandleRenderPassAttachmentCountChanged(attachments_root_item, render_pass_create_info); });

        // Set object name.
        attachment_count_item->setObjectName(kStrAttachmentCountItem);

        // Add the attachment array node.
        render_pass_create_info_root->AppendChildItem(attachments_root_item);

        // Provide the element used to track the dimension of the pAttachments array.
        attachments_root_item->SetArraySizeElement(static_cast<RgEditorElementNumeric<uint32_t>*>(attachment_count_item));

        // Initialize the pAttachments rows.
        HandleRenderPassAttachmentCountChanged(attachments_root_item, render_pass_create_info, true);

        // Subpass array:
        // Create the subpasses array root item.
        RgEditorElementArrayElementAdd* subpass_root_item = new RgEditorElementArrayElementAdd(render_pass_create_info_root, kStrVulkanRenderPassSubpasses,
            [=](int element_index)
        {
            // Remove the artificial "resolveAttachmentCount" that was configured per-subpass.
            auto resolve_attachment_count_iter = resolve_attachment_count_per_subpass_.find(element_index);
            if (resolve_attachment_count_iter != resolve_attachment_count_per_subpass_.end())
            {
                // Is the resolveAttachmentCount variable valid?
                uint32_t* resolve_attachment_count = resolve_attachment_count_iter->second;
                assert(resolve_attachment_count != nullptr);
                if (resolve_attachment_count != nullptr)
                {
                    // Destroy the resolveAttachmentCount variable associated with the subpass.
                    RG_SAFE_DELETE(resolve_attachment_count);

                    // Erase the variable since the subpass is being destroyed.
                    resolve_attachment_count_per_subpass_.erase(resolve_attachment_count_iter);
                }
            }

            // Remove the subpass at the given index.
            RemoveElement(render_pass_create_info->pSubpasses, render_pass_create_info->subpassCount, element_index);
        });

        // Set object name.
        subpass_root_item->setObjectName(kStrSubpassRootItem);

        // Create the subpassCount member.
        RgEditorElement* subpass_description_count_item = MakeNumericElement(nullptr, kStrVulkanRenderPassSubpassCount,
            &render_pass_create_info->subpassCount, [=] { HandleRenderPassSubpassCountChanged(subpass_root_item, render_pass_create_info); });

        // Set object name.
        subpass_description_count_item->setObjectName(kStrSubpassDescriptionCountItem);

        // Add the pSubpasses array node.
        render_pass_create_info_root->AppendChildItem(subpass_root_item);

        // Provide the element used to track the dimension of the pSubpassDescription array.
        subpass_root_item->SetArraySizeElement(static_cast<RgEditorElementNumeric<uint32_t>*>(subpass_description_count_item));

        // Initialize the pSubpassDescription rows.
        HandleRenderPassSubpassCountChanged(subpass_root_item, render_pass_create_info, true);

        // Dependency array:
        // Create the dependency array root item.
        RgEditorElementArrayElementAdd* dependencies_root_item = new RgEditorElementArrayElementAdd(render_pass_create_info_root, kStrVulkanRenderPassDependencies,
            [=](int element_index) { RemoveElement(render_pass_create_info->pDependencies, render_pass_create_info->dependencyCount, element_index); });

        // Set object name.
        dependencies_root_item->setObjectName(kStrDependenciesRootItem);

        // Create the dependencyCount member.
        RgEditorElement* dependency_description_count_item = MakeNumericElement(nullptr, kStrVulkanRenderPassDependencyCount,
            &render_pass_create_info->dependencyCount, [=] { HandleRenderPassDependencyCountChanged(dependencies_root_item, render_pass_create_info); });

        // Set object name.
        dependency_description_count_item->setObjectName(kStrDependencyDescriptionCountItem);

        // Add the pDependencies array node.
        render_pass_create_info_root->AppendChildItem(dependencies_root_item);

        // Provide the element used to track the dimension of the pDependencies array.
        dependencies_root_item->SetArraySizeElement(static_cast<RgEditorElementNumeric<uint32_t>*>(dependency_description_count_item));

        // Initialize the pDependencies rows.
        HandleRenderPassDependencyCountChanged(dependencies_root_item, render_pass_create_info, true);
    }
}

void RgPipelineStateModelVulkan::HandleRenderPassAttachmentCountChanged(RgEditorElement* root_element, VkRenderPassCreateInfo* render_pass_create_info, bool first_init)
{
    ResizeHandler(root_element,
        render_pass_create_info->attachmentCount,
        render_pass_create_info->pAttachments,
        kStrVulkanRenderPassAttachmentDescription,
        std::bind(&RgPipelineStateModelVulkan::InitializeRenderPassAttachmentDescriptionCreateInfo, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
        first_init);
}

void RgPipelineStateModelVulkan::HandleRenderPassSubpassCountChanged(RgEditorElement* root_element, VkRenderPassCreateInfo* render_pass_create_info, bool first_init)
{
    ResizeHandler(root_element,
        render_pass_create_info->subpassCount,
        render_pass_create_info->pSubpasses,
        kStrVulkanRenderPassSubpassDescription,
        std::bind(&RgPipelineStateModelVulkan::InitializeRenderPassSubpassDescriptionCreateInfo, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
        first_init);
}

void RgPipelineStateModelVulkan::HandleRenderPassDependencyCountChanged(RgEditorElement* root_element, VkRenderPassCreateInfo* render_pass_create_info, bool first_init)
{
    ResizeHandler(root_element,
        render_pass_create_info->dependencyCount,
        render_pass_create_info->pDependencies,
        kStrVulkanRenderPassDependencyDescription,
        std::bind(&RgPipelineStateModelVulkan::InitializeRenderPassDependencyDescriptionCreateInfo, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
        first_init);
}

void RgPipelineStateModelVulkan::InitializeRenderPassAttachmentDescriptionCreateInfo(RgEditorElement* root_element, VkAttachmentDescription* base_attachment_description, int item_index)
{
    assert(root_element != nullptr);
    assert(base_attachment_description != nullptr);
    if (root_element != nullptr && base_attachment_description != nullptr)
    {
        VkAttachmentDescription* offset_attachment_description = base_attachment_description + item_index;
        assert(offset_attachment_description != nullptr);
        if (offset_attachment_description != nullptr)
        {
            // Add the "flags" member.
            const RgEnumValuesVector& attachment_description_flag_enumerators = GetAttachmentDescriptionFlagEnumerators();
            RgEditorElement* flags_element = new RgEditorElementEnum(parent_, kStrVulkanPipelineMemberFlags, attachment_description_flag_enumerators, reinterpret_cast<uint32_t*>(&offset_attachment_description->flags), true);
            root_element->AppendChildItem(flags_element);

            // Connect to the splitter moved signal to close the drop down.
            RgBuildViewVulkan* build_view_vulkan = static_cast<RgBuildViewVulkan*>(parent_);
            bool is_connected = connect(build_view_vulkan, &RgBuildViewVulkan::SplitterMoved, static_cast<RgEditorElementEnum*>(flags_element), &RgEditorElementEnum::HotKeyPressedSignal);

            // Set object name.
            flags_element->setObjectName(kStrFlagsElement);

            // Connect to the enum list widget status signal.
            is_connected = connect(flags_element, &RgEditorElement::EnumListWidgetStatusSignal, this, &RgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
            assert(is_connected);

            // Connect the shortcut hot key signal.
            is_connected = connect(this, &RgPipelineStateModelVulkan::HotKeyPressedSignal, static_cast<RgEditorElementEnum*>(flags_element), &RgEditorElementEnum::HotKeyPressedSignal);
            assert(is_connected);

            // Add the "format" member values.
            const RgEnumValuesVector& format_enumerators = GetFormatEnumerators();
            RgEditorElement* format_element = new RgEditorElementEnum(parent_, kStrVulkanPipelineMemberVertexFormat, format_enumerators, reinterpret_cast<uint32_t*>(&offset_attachment_description->format));
            root_element->AppendChildItem(format_element);

            // Connect to the splitter moved signal to close the drop down.
            build_view_vulkan = static_cast<RgBuildViewVulkan*>(parent_);
            is_connected = connect(build_view_vulkan, &RgBuildViewVulkan::SplitterMoved, static_cast<RgEditorElementEnum*>(format_element), &RgEditorElementEnum::HotKeyPressedSignal);

            // Set object name.
            format_element->setObjectName(kStrFormatElement);

            // Connect to the enum list widget status signal.
            is_connected = connect(format_element, &RgEditorElement::EnumListWidgetStatusSignal, this, &RgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
            assert(is_connected);

            // Connect the shortcut hot key signal.
            is_connected = connect(this, &RgPipelineStateModelVulkan::HotKeyPressedSignal, static_cast<RgEditorElementEnum*>(format_element), &RgEditorElementEnum::HotKeyPressedSignal);
            assert(is_connected);

            // Add the "samples" member.
            RgEditorElement* samples_item = MakeNumericElement(root_element, kStrVulkanRenderPassAttachmentSamples, reinterpret_cast<uint32_t*>(&offset_attachment_description->samples));
            root_element->AppendChildItem(samples_item);

            // Set object name.
            samples_item->setObjectName(kStrSamplesItem);

            // Add the "loadOp" member values.
            const RgEnumValuesVector& load_op_enumerators = GetAttachmentLoadOpEnumerators();
            RgEditorElement* load_op_item = new RgEditorElementEnum(parent_, kStrVulkanRenderPassLoadOp, load_op_enumerators, reinterpret_cast<uint32_t*>(&offset_attachment_description->loadOp));
            root_element->AppendChildItem(load_op_item);

            // Connect to the splitter moved signal to close the drop down.
            build_view_vulkan = static_cast<RgBuildViewVulkan*>(parent_);
            is_connected = connect(build_view_vulkan, &RgBuildViewVulkan::SplitterMoved, static_cast<RgEditorElementEnum*>(load_op_item), &RgEditorElementEnum::HotKeyPressedSignal);

            // Set object name.
            load_op_item->setObjectName(kStrLoadOpItem);

            // Connect to the enum list widget status signal.
            is_connected = connect(load_op_item, &RgEditorElement::EnumListWidgetStatusSignal, this, &RgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
            assert(is_connected);

            // Connect the shortcut hot key signal.
            is_connected = connect(this, &RgPipelineStateModelVulkan::HotKeyPressedSignal, static_cast<RgEditorElementEnum*>(load_op_item), &RgEditorElementEnum::HotKeyPressedSignal);
            assert(is_connected);

            // Add the "storeOp" member values.
            const RgEnumValuesVector& store_op_enumerators = GetAttachmentStoreOpEnumerators();
            RgEditorElement* store_op_item = new RgEditorElementEnum(parent_, kStrVulkanRenderPassStoreOp, store_op_enumerators, reinterpret_cast<uint32_t*>(&offset_attachment_description->storeOp));
            root_element->AppendChildItem(store_op_item);

            // Connect to the splitter moved signal to close the drop down.
            build_view_vulkan = static_cast<RgBuildViewVulkan*>(parent_);
            is_connected = connect(build_view_vulkan, &RgBuildViewVulkan::SplitterMoved, static_cast<RgEditorElementEnum*>(store_op_item), &RgEditorElementEnum::HotKeyPressedSignal);

            // Set object name.
            store_op_item->setObjectName(kStrStoreOpItem);

            // Connect to the enum list widget status signal.
            is_connected = connect(store_op_item, &RgEditorElement::EnumListWidgetStatusSignal, this, &RgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
            assert(is_connected);

            // Connect the shortcut hot key signal.
            is_connected = connect(this, &RgPipelineStateModelVulkan::HotKeyPressedSignal, static_cast<RgEditorElementEnum*>(store_op_item), &RgEditorElementEnum::HotKeyPressedSignal);
            assert(is_connected);

            // Add the "stencilLoadOp" member values.
            RgEditorElement* stencil_load_op_item = new RgEditorElementEnum(parent_, kStrVulkanRenderPassStencilLoadOp, load_op_enumerators, reinterpret_cast<uint32_t*>(&offset_attachment_description->stencilLoadOp));
            root_element->AppendChildItem(stencil_load_op_item);

            // Connect to the splitter moved signal to close the drop down.
            build_view_vulkan = static_cast<RgBuildViewVulkan*>(parent_);
            is_connected = connect(build_view_vulkan, &RgBuildViewVulkan::SplitterMoved, static_cast<RgEditorElementEnum*>(stencil_load_op_item), &RgEditorElementEnum::HotKeyPressedSignal);

            // Set object name.
            stencil_load_op_item->setObjectName(kStrStencilLoadOpItem);

            // Connect to the enum list widget status signal.
            is_connected = connect(stencil_load_op_item, &RgEditorElement::EnumListWidgetStatusSignal, this, &RgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
            assert(is_connected);

            // Connect the shortcut hot key signal.
            is_connected = connect(this, &RgPipelineStateModelVulkan::HotKeyPressedSignal, static_cast<RgEditorElementEnum*>(stencil_load_op_item), &RgEditorElementEnum::HotKeyPressedSignal);
            assert(is_connected);

            // Add the "stencilStoreOp" member values.
            RgEditorElement* stencil_store_op_item = new RgEditorElementEnum(parent_, kStrVulkanRenderPassStencilStoreOp, store_op_enumerators, reinterpret_cast<uint32_t*>(&offset_attachment_description->stencilStoreOp));
            root_element->AppendChildItem(stencil_store_op_item);

            // Connect to the splitter moved signal to close the drop down.
            build_view_vulkan = static_cast<RgBuildViewVulkan*>(parent_);
            is_connected = connect(build_view_vulkan, &RgBuildViewVulkan::SplitterMoved, static_cast<RgEditorElementEnum*>(stencil_store_op_item), &RgEditorElementEnum::HotKeyPressedSignal);

            // Set object name.
            stencil_store_op_item->setObjectName(kStrStencilStoreOpItem);

            // Connect to the enum list widget status signal.
            is_connected = connect(stencil_store_op_item, &RgEditorElement::EnumListWidgetStatusSignal, this, &RgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
            assert(is_connected);

            // Connect the shortcut hot key signal.
            is_connected = connect(this, &RgPipelineStateModelVulkan::HotKeyPressedSignal, static_cast<RgEditorElementEnum*>(stencil_store_op_item), &RgEditorElementEnum::HotKeyPressedSignal);
            assert(is_connected);

            // Add the "initialLayout" member values.
            const RgEnumValuesVector& image_layout_enumerators = GetImageLayoutEnumerators();
            RgEditorElement* initial_layout_item = new RgEditorElementEnum(parent_, kStrVulkanRenderPassInitialLayout, image_layout_enumerators, reinterpret_cast<uint32_t*>(&offset_attachment_description->initialLayout));
            root_element->AppendChildItem(initial_layout_item);

            // Connect to the splitter moved signal to close the drop down.
            build_view_vulkan = static_cast<RgBuildViewVulkan*>(parent_);
            is_connected = connect(build_view_vulkan, &RgBuildViewVulkan::SplitterMoved, static_cast<RgEditorElementEnum*>(initial_layout_item), &RgEditorElementEnum::HotKeyPressedSignal);

            // Set object name.
            initial_layout_item->setObjectName(kStrInitialLayoutItem);

            // Connect to the enum list widget status signal.
            is_connected = connect(initial_layout_item, &RgEditorElement::EnumListWidgetStatusSignal, this, &RgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
            assert(is_connected);

            // Connect the shortcut hot key signal.
            is_connected = connect(this, &RgPipelineStateModelVulkan::HotKeyPressedSignal, static_cast<RgEditorElementEnum*>(initial_layout_item), &RgEditorElementEnum::HotKeyPressedSignal);
            assert(is_connected);

            // Add the "finalLayout" member values.
            RgEditorElement* final_layout_item = new RgEditorElementEnum(parent_, kStrVulkanRenderPassFinalLayout, image_layout_enumerators, reinterpret_cast<uint32_t*>(&offset_attachment_description->finalLayout));
            root_element->AppendChildItem(final_layout_item);

            // Connect to the splitter moved signal to close the drop down.
            build_view_vulkan = static_cast<RgBuildViewVulkan*>(parent_);
            is_connected = connect(build_view_vulkan, &RgBuildViewVulkan::SplitterMoved, static_cast<RgEditorElementEnum*>(final_layout_item), &RgEditorElementEnum::HotKeyPressedSignal);

            // Set object name.
            final_layout_item->setObjectName(kStrFinalLayoutItem);

            // Connect to the enum list widget status signal.
            is_connected = connect(final_layout_item, &RgEditorElement::EnumListWidgetStatusSignal, this, &RgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
            assert(is_connected);

            // Connect the shortcut hot key signal.
            is_connected = connect(this, &RgPipelineStateModelVulkan::HotKeyPressedSignal, static_cast<RgEditorElementEnum*>(final_layout_item), &RgEditorElementEnum::HotKeyPressedSignal);
            assert(is_connected);
        }
    }
}

void RgPipelineStateModelVulkan::InitializeRenderPassSubpassDescriptionCreateInfo(RgEditorElement* root_element, VkSubpassDescription* base_subpass_description, int item_index)
{
    assert(root_element != nullptr);
    assert(base_subpass_description != nullptr);
    if (root_element != nullptr && base_subpass_description != nullptr)
    {
        VkSubpassDescription* offset_subpass_description = base_subpass_description + item_index;
        assert(offset_subpass_description != nullptr);
        if (offset_subpass_description != nullptr)
        {
            // Add the "flags" member.
            const RgEnumValuesVector& subpass_description_flag_enumerators = GetSubpassDescriptionFlagEnumerators();
            RgEditorElement* flags = new RgEditorElementEnum(parent_, kStrVulkanPipelineMemberFlags, subpass_description_flag_enumerators, reinterpret_cast<uint32_t*>(&offset_subpass_description->flags), true);
            root_element->AppendChildItem(flags);

            // Connect to the splitter moved signal to close the drop down.
            RgBuildViewVulkan* build_view_vulkan = static_cast<RgBuildViewVulkan*>(parent_);
            bool is_connected = connect(build_view_vulkan, &RgBuildViewVulkan::SplitterMoved, static_cast<RgEditorElementEnum*>(flags), &RgEditorElementEnum::HotKeyPressedSignal);

            // Set object name.
            flags->setObjectName(kStrFlags);

            // Connect to the enum list widget status signal.
            is_connected = connect(flags, &RgEditorElement::EnumListWidgetStatusSignal, this, &RgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
            assert(is_connected);

            // Connect the shortcut hot key signal.
            is_connected = connect(this, &RgPipelineStateModelVulkan::HotKeyPressedSignal, static_cast<RgEditorElementEnum*>(flags), &RgEditorElementEnum::HotKeyPressedSignal);
            assert(is_connected);

            // Add the "pipelineBindPoint" member.
            const RgEnumValuesVector& pipeline_bind_point_enumerators = GetPipelineBindPointEnumerators();
            RgEditorElement* new_pipeline_bind_point_node = new RgEditorElementEnum(parent_, kStrVulkanRenderPassDependencyPipelineBindPoint, pipeline_bind_point_enumerators, reinterpret_cast<uint32_t*>(&offset_subpass_description->pipelineBindPoint));
            root_element->AppendChildItem(new_pipeline_bind_point_node);

            // Connect to the splitter moved signal to close the drop down.
            build_view_vulkan = static_cast<RgBuildViewVulkan*>(parent_);
            is_connected = connect(build_view_vulkan, &RgBuildViewVulkan::SplitterMoved, static_cast<RgEditorElementEnum*>(new_pipeline_bind_point_node), &RgEditorElementEnum::HotKeyPressedSignal);

            // Set object name.
            new_pipeline_bind_point_node->setObjectName(kStrPipelineBindPointNode);

            // Connect to the enum list widget status signal.
            is_connected = connect(new_pipeline_bind_point_node, &RgEditorElement::EnumListWidgetStatusSignal, this, &RgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
            assert(is_connected);

            // Connect the shortcut hot key signal.
            is_connected = connect(this, &RgPipelineStateModelVulkan::HotKeyPressedSignal, static_cast<RgEditorElementEnum*>(new_pipeline_bind_point_node), &RgEditorElementEnum::HotKeyPressedSignal);
            assert(is_connected);

        // Input attachment array:
            // Create the input attachment array root item.
            RgEditorElementArrayElementAdd* input_attachments_root_item = new RgEditorElementArrayElementAdd(root_element, kStrVulkanRenderPassSubpassInputAttachments,
                [=](int element_index) { RemoveElement(offset_subpass_description->pInputAttachments, offset_subpass_description->inputAttachmentCount, element_index); });

            // Set object name.
            input_attachments_root_item->setObjectName(kStrInputAttachmentsRootItem);

            // Create the input attachment count member.
            RgEditorElement* input_attachment_count_node = MakeNumericElement(nullptr, kStrVulkanRenderPassSubpassInputAttachmentCount,
                &offset_subpass_description->inputAttachmentCount, [=] { HandleRenderPassSubpassInputAttachmentCountChanged(input_attachments_root_item, offset_subpass_description); });

            // Set object name.
            input_attachment_count_node->setObjectName(kStrInputAttachmentCountNode);

            // Add the pInputAttachments array node.
            root_element->AppendChildItem(input_attachments_root_item);

            // Provide the element used to track the dimension of the pInputAttachments array.
            input_attachments_root_item->SetArraySizeElement(static_cast<RgEditorElementNumeric<uint32_t>*>(input_attachment_count_node));

            // Initialize the pInputAttachments rows.
            HandleRenderPassSubpassInputAttachmentCountChanged(input_attachments_root_item, offset_subpass_description, true);

        // Color attachment and resolve attachment array:
            // Create the color attachment array root item.
            RgEditorElementArrayElementAdd* color_attachments_root_item = new RgEditorElementArrayElementAdd(root_element, kStrVulkanRenderPassSubpassColorAttachments,
                [=](int element_index) { RemoveElement(offset_subpass_description->pColorAttachments, offset_subpass_description->colorAttachmentCount, element_index); });

            // Set object name.
            color_attachments_root_item->setObjectName(kStrColorAttachmentsRootItem);

            // Create the color attachment count member.
            RgEditorElement* color_attachment_count_node = MakeNumericElement(nullptr, kStrVulkanRenderPassSubpassColorAttachmentCount,
                &offset_subpass_description->colorAttachmentCount, [=] { HandleRenderPassSubpassColorAttachmentCountChanged(color_attachments_root_item, offset_subpass_description); });

            // Set object name.
            color_attachment_count_node->setObjectName(kStrColorAttachmentCountNode);

            // Add the pColorAttachments array node.
            root_element->AppendChildItem(color_attachments_root_item);

            // Provide the element used to track the dimension of the pColorAttachments array.
            color_attachments_root_item->SetArraySizeElement(static_cast<RgEditorElementNumeric<uint32_t>*>(color_attachment_count_node));

            // Initialize the pColorAttachments rows.
            HandleRenderPassSubpassColorAttachmentCountChanged(color_attachments_root_item, offset_subpass_description, true);

            int subpass_index = item_index;

            // Create a new resolve attachment count. Each subpass will get its own.
            uint32_t* resolve_attachment_count = new uint32_t{};
            resolve_attachment_count_per_subpass_[subpass_index] = resolve_attachment_count;

            // If pResolveAttachments is non-null, we can assume that the dimension of the array matches
            // that of colorAttachmentCount. Initialize the count to match the colorAttachmentCount.
            if (offset_subpass_description->pResolveAttachments != nullptr)
            {
                *resolve_attachment_count = offset_subpass_description->colorAttachmentCount;
            }

        // Resolve attachment array:
            // Create the resolve attachment array root item.
            RgEditorElementArrayElementAdd* resolve_attachments_root_item = new RgEditorElementArrayElementAdd(root_element, kStrVulkanRenderPassSubpassResolveAttachments,
                [=](int element_index)
            {
                // Ensure that the element index
                assert(resolve_attachment_count != nullptr);
                if (resolve_attachment_count != nullptr)
                {
                    // An array item has been trashed. Decrease the corresponding count item.
                    RemoveElement(offset_subpass_description->pResolveAttachments, *resolve_attachment_count, element_index);
                }
            });

            // Set object name.
            resolve_attachments_root_item->setObjectName(kStrResolveAttachmentsRootItem);

            // Create an artificial resolve attachment count member.
            // This member is not part of the VkSubpassDescription structure, but is required since pResolveAttachments array can be NULL, or equal to colorAttachmentCount.
            RgEditorElement* resolve_attachment_count_node = MakeNumericElement(nullptr, kStrVulkanRenderPassSubpassResolveAttachmentCount,
                resolve_attachment_count, [=]
                {
                    HandleRenderPassSubpassResolveAttachmentCountChanged(subpass_index, resolve_attachments_root_item, offset_subpass_description);
                });

            // Set object name.
            resolve_attachment_count_node->setObjectName(kStrResolveAttachmentCountNode);

            // Add the pResolveAttachments array node.
            root_element->AppendChildItem(resolve_attachments_root_item);

            // Provide the element used to track the dimension of the pResolveAttachments array.
            resolve_attachments_root_item->SetArraySizeElement(static_cast<RgEditorElementNumeric<uint32_t>*>(resolve_attachment_count_node));

            // Initialize the pResolveAttachments rows.
            HandleRenderPassSubpassResolveAttachmentCountChanged(subpass_index, resolve_attachments_root_item, offset_subpass_description, true);

            // Create the depth stencil attachment item.
            RgEditorElement* depth_stencil_attachments_root_item = new RgEditorElement(root_element, kStrVulkanRenderPassSubpassDepthStencilAttachment);
            if (offset_subpass_description->pDepthStencilAttachment != nullptr)
            {
                // Create a copy of the depth stencil attachment that can be modified.
                depth_stencil_attachment_ = new VkAttachmentReference{};
                memcpy(depth_stencil_attachment_, offset_subpass_description->pDepthStencilAttachment, sizeof(VkAttachmentReference));
                offset_subpass_description->pDepthStencilAttachment = depth_stencil_attachment_;

                // Initialize the tree items with the modifiable attachment pointer.
                InitializeAttachmentReference(depth_stencil_attachments_root_item, depth_stencil_attachment_, 0);
            }
            root_element->AppendChildItem(depth_stencil_attachments_root_item);

            // Set object name.
            depth_stencil_attachments_root_item->setObjectName(kStrDepthStencilAttachmentsRootItem);

            // Input attachment array:
            // Create the preserveAttachments array root item.
            RgEditorElementArrayElementAdd* preserveInput_attachments_root_item = new RgEditorElementArrayElementAdd(root_element, kStrVulkanRenderPassSubpassPreserveAttachments,
                [=](int element_index) { RemoveElement(offset_subpass_description->pPreserveAttachments, offset_subpass_description->preserveAttachmentCount, element_index); });

            // Set object name.
            preserveInput_attachments_root_item->setObjectName(kStrPreserveInputAttachmentRootItem);

            // Create the preserveAttachment count member.
            RgEditorElement* preserve_attachment_count_node = MakeNumericElement(nullptr, kStrVulkanRenderPassSubpassPreserveAttachmentCount,
                &offset_subpass_description->preserveAttachmentCount, [=] { HandleRenderPassSubpassPreserveAttachmentCountChanged(preserveInput_attachments_root_item, offset_subpass_description); });

            // Set object name.
            preserve_attachment_count_node->setObjectName(kStrPreserveAttachmentCountNode);

            // Add the pPreserveAttachments array node.
            root_element->AppendChildItem(preserveInput_attachments_root_item);

            // Provide the element used to track the dimension of the pPreserveInputAttachments array.
            preserveInput_attachments_root_item->SetArraySizeElement(static_cast<RgEditorElementNumeric<uint32_t>*>(preserve_attachment_count_node));

            // Initialize the pPreserveInputAttachments rows.
            HandleRenderPassSubpassPreserveAttachmentCountChanged(preserveInput_attachments_root_item, offset_subpass_description, true);
        }
    }
}

void RgPipelineStateModelVulkan::HandleRenderPassSubpassInputAttachmentCountChanged(RgEditorElement* root_element, VkSubpassDescription* subpass_create_info, bool first_init)
{
    ResizeHandler(root_element,
        subpass_create_info->inputAttachmentCount,
        subpass_create_info->pInputAttachments,
        kStrVulkanRenderSubpassAttachmentReference,
        std::bind(&RgPipelineStateModelVulkan::InitializeAttachmentReference, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
        first_init);
}

void RgPipelineStateModelVulkan::HandleRenderPassSubpassColorAttachmentCountChanged(RgEditorElement* root_element, VkSubpassDescription* subpass_create_info, bool first_init)
{
    // Resize the color attachments array.
    ResizeHandler(root_element,
        subpass_create_info->colorAttachmentCount,
        subpass_create_info->pColorAttachments,
        kStrVulkanRenderSubpassAttachmentReference,
        std::bind(&RgPipelineStateModelVulkan::InitializeAttachmentReference, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
        first_init);
}

void RgPipelineStateModelVulkan::HandleRenderPassSubpassResolveAttachmentCountChanged(int subpass_index, RgEditorElement* root_element, VkSubpassDescription* subpass_create_info, bool first_init)
{
    uint32_t* subpass_resolve_attachment_count = resolve_attachment_count_per_subpass_.at(subpass_index);

    assert(subpass_resolve_attachment_count != nullptr);
    if (subpass_resolve_attachment_count != nullptr)
    {
        // Resize the resolve attachments array.
        ResizeHandler(root_element,
            *subpass_resolve_attachment_count,
            subpass_create_info->pResolveAttachments,
            kStrVulkanRenderSubpassAttachmentReference,
            std::bind(&RgPipelineStateModelVulkan::InitializeAttachmentReference, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
            first_init);
    }
}

void RgPipelineStateModelVulkan::HandleRenderPassSubpassPreserveAttachmentCountChanged(RgEditorElement* root_element, VkSubpassDescription* subpass_create_info, bool first_init)
{
    // Resize the preserve attachments array.
    ResizeHandler(root_element,
        subpass_create_info->preserveAttachmentCount,
        subpass_create_info->pPreserveAttachments,
        kStrVulkanRenderSubpassPreserveAttachmentElementType,
        std::bind(&RgPipelineStateModelVulkan::InitializePreserveAttachment, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
        first_init);
}

void RgPipelineStateModelVulkan::InitializeAttachmentReference(RgEditorElement* root_element, VkAttachmentReference* base_attachment_reference, int item_index)
{
    assert(root_element != nullptr);
    assert(base_attachment_reference != nullptr);
    if (root_element != nullptr && base_attachment_reference != nullptr)
    {
        VkAttachmentReference* offset_attachment_reference = base_attachment_reference + item_index;
        assert(offset_attachment_reference != nullptr);
        if (offset_attachment_reference != nullptr)
        {
            // Add the "attachment" member.
            RgEditorElement* attachment_node = MakeNumericElement(root_element, kStrVulkanRenderPassSubpassAttachmentIndex, &offset_attachment_reference->attachment);
            root_element->AppendChildItem(attachment_node);

            // Set object name.
            attachment_node->setObjectName(kStrAttachmentNode);

            // Add the "layout" member node.
            const RgEnumValuesVector& image_layout_enumerators = GetImageLayoutEnumerators();
            RgEditorElement* image_layout_node = new RgEditorElementEnum(parent_, kStrVulkanRenderPassSubpassAttachmentLayout, image_layout_enumerators, reinterpret_cast<uint32_t*>(&offset_attachment_reference->layout));
            root_element->AppendChildItem(image_layout_node);

            // Connect to the splitter moved signal to close the drop down.
            RgBuildViewVulkan* build_view_vulkan = static_cast<RgBuildViewVulkan*>(parent_);
            bool is_connected = connect(build_view_vulkan, &RgBuildViewVulkan::SplitterMoved, static_cast<RgEditorElementEnum*>(image_layout_node), &RgEditorElementEnum::HotKeyPressedSignal);

            // Set object name.
            image_layout_node->setObjectName(kStrImageLayoutNode);

            // Connect to the enum list widget status signal.
            is_connected = connect(image_layout_node, &RgEditorElement::EnumListWidgetStatusSignal, this, &RgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
            assert(is_connected);

            // Connect the shortcut hot key signal.
            is_connected = connect(this, &RgPipelineStateModelVulkan::HotKeyPressedSignal, static_cast<RgEditorElementEnum*>(image_layout_node), &RgEditorElementEnum::HotKeyPressedSignal);
            assert(is_connected);
        }
    }
}

void RgPipelineStateModelVulkan::InitializePreserveAttachment(RgEditorElement* root_element, uint32_t* base_reserve_attachment, int item_index)
{
    assert(root_element != nullptr);
    assert(base_reserve_attachment != nullptr);
    if (root_element != nullptr && base_reserve_attachment != nullptr)
    {
        uint32_t* offset_reserve_attachment = base_reserve_attachment + item_index;
        assert(offset_reserve_attachment != nullptr);
        if (offset_reserve_attachment != nullptr)
        {
            // Add the "pReserveAttachment" member.
            RgEditorElement* preserve_attachment_node = MakeNumericElement(root_element, kStrVulkanRenderPassPreserveAttachment, reinterpret_cast<uint32_t*>(offset_reserve_attachment));
            root_element->AppendChildItem(preserve_attachment_node);

            // Set object name.
            preserve_attachment_node->setObjectName(kStrPreserveAttachmentNode);
        }
    }
}

void RgPipelineStateModelVulkan::InitializeRenderPassDependencyDescriptionCreateInfo(RgEditorElement* root_element, VkSubpassDependency* base_dependency_description, int item_index)
{
    assert(root_element != nullptr);
    assert(base_dependency_description != nullptr);
    if (root_element != nullptr && base_dependency_description != nullptr)
    {
        VkSubpassDependency* offset_dependency_description = base_dependency_description + item_index;
        assert(offset_dependency_description != nullptr);
        if (offset_dependency_description != nullptr)
        {
            // Add the "srcSubpass" member.
            RgEditorElement* src_subpass = MakeNumericElement(root_element, kStrVulkanRenderPassDependencySrcSubpass, reinterpret_cast<uint32_t*>(&offset_dependency_description->srcSubpass));
            root_element->AppendChildItem(src_subpass);

            // Set object name.
            src_subpass->setObjectName(kStrSrcSubpass);

            // Add the "dstSubpass" member.
            RgEditorElement* dst_subpass = MakeNumericElement(root_element, kStrVulkanRenderPassDependencyDstSubpass, reinterpret_cast<uint32_t*>(&offset_dependency_description->dstSubpass));
            root_element->AppendChildItem(dst_subpass);

            // Set object name.
            dst_subpass->setObjectName(kStrDstSubpass);

            // Get the stage flag enumerators.
            const RgEnumValuesVector& stage_flag_enumerators = GetPipelineStageFlagEnumerators();

            // Add the "srcStageMask" member.
            RgEditorElement* src_stage_mask = new RgEditorElementEnum(parent_, kStrVulkanRenderPassDependencySrcStageMask, stage_flag_enumerators, reinterpret_cast<uint32_t*>(&offset_dependency_description->srcStageMask), true);
            root_element->AppendChildItem(src_stage_mask);

            // Connect to the splitter moved signal to close the drop down.
            RgBuildViewVulkan* build_view_vulkan = static_cast<RgBuildViewVulkan*>(parent_);
            bool is_connected = connect(build_view_vulkan, &RgBuildViewVulkan::SplitterMoved, static_cast<RgEditorElementEnum*>(src_stage_mask), &RgEditorElementEnum::HotKeyPressedSignal);

            // Set object name.
            src_stage_mask->setObjectName(kStrSrcStageMask);

            // Connect to the enum list widget status signal.
            is_connected = connect(src_stage_mask, &RgEditorElement::EnumListWidgetStatusSignal, this, &RgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
            assert(is_connected);

            // Connect the shortcut hot key signal.
            is_connected = connect(this, &RgPipelineStateModelVulkan::HotKeyPressedSignal, static_cast<RgEditorElementEnum*>(src_stage_mask), &RgEditorElementEnum::HotKeyPressedSignal);
            assert(is_connected);

            // Add the "dstStageMask" member.
            RgEditorElement* dst_stage_mask = new RgEditorElementEnum(parent_, kStrVulkanRenderPassDependencyDstStageMask, stage_flag_enumerators, reinterpret_cast<uint32_t*>(&offset_dependency_description->dstStageMask), true);
            root_element->AppendChildItem(dst_stage_mask);

            // Connect to the splitter moved signal to close the drop down.
            build_view_vulkan = static_cast<RgBuildViewVulkan*>(parent_);
            is_connected = connect(build_view_vulkan, &RgBuildViewVulkan::SplitterMoved, static_cast<RgEditorElementEnum*>(dst_stage_mask), &RgEditorElementEnum::HotKeyPressedSignal);

            // Set object name.
            dst_stage_mask->setObjectName(kStrDstStageMask);

            // Connect to the enum list widget status signal.
            is_connected = connect(dst_stage_mask, &RgEditorElement::EnumListWidgetStatusSignal, this, &RgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
            assert(is_connected);

            // Connect the shortcut hot key signal.
            is_connected = connect(this, &RgPipelineStateModelVulkan::HotKeyPressedSignal, static_cast<RgEditorElementEnum*>(dst_stage_mask), &RgEditorElementEnum::HotKeyPressedSignal);
            assert(is_connected);

            // Get the access flag enumerators.
            const RgEnumValuesVector& access_flag_enumerators = GetAccessFlagEnumerators();

            // Add the "src_access_mask" member.
            RgEditorElement* src_access_mask = new RgEditorElementEnum(parent_, kStrVulkanRenderPassDependencySrcAccessMask, access_flag_enumerators, reinterpret_cast<uint32_t*>(&offset_dependency_description->srcAccessMask), true);
            root_element->AppendChildItem(src_access_mask);

            // Connect to the splitter moved signal to close the drop down.
            build_view_vulkan = static_cast<RgBuildViewVulkan*>(parent_);
            is_connected = connect(build_view_vulkan, &RgBuildViewVulkan::SplitterMoved, static_cast<RgEditorElementEnum*>(src_access_mask), &RgEditorElementEnum::HotKeyPressedSignal);

            // Set object name.
            src_access_mask->setObjectName(kStrSrcAccessMask);

            // Connect to the enum list widget status signal.
            is_connected = connect(src_access_mask, &RgEditorElement::EnumListWidgetStatusSignal, this, &RgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
            assert(is_connected);

            // Connect the shortcut hot key signal.
            is_connected = connect(this, &RgPipelineStateModelVulkan::HotKeyPressedSignal, static_cast<RgEditorElementEnum*>(src_access_mask), &RgEditorElementEnum::HotKeyPressedSignal);
            assert(is_connected);

            // Add the "dstAccessMask" member.
            RgEditorElement* dst_access_mask = new RgEditorElementEnum(parent_, kStrVulkanRenderPassDependencyDstAccessMask, access_flag_enumerators, reinterpret_cast<uint32_t*>(&offset_dependency_description->dstAccessMask), true);
            root_element->AppendChildItem(dst_access_mask);

            // Connect to the splitter moved signal to close the drop down.
            build_view_vulkan = static_cast<RgBuildViewVulkan*>(parent_);
            is_connected = connect(build_view_vulkan, &RgBuildViewVulkan::SplitterMoved, static_cast<RgEditorElementEnum*>(dst_access_mask), &RgEditorElementEnum::HotKeyPressedSignal);

            // Set object name.
            dst_access_mask->setObjectName(kStrDstAccessMask);

            // Connect to the enum list widget status signal.
            is_connected = connect(dst_access_mask, &RgEditorElement::EnumListWidgetStatusSignal, this, &RgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
            assert(is_connected);

            // Connect the shortcut hot key signal.
            is_connected = connect(this, &RgPipelineStateModelVulkan::HotKeyPressedSignal, static_cast<RgEditorElementEnum*>(dst_access_mask), &RgEditorElementEnum::HotKeyPressedSignal);
            assert(is_connected);

            // Add the "dependencyFlags" member.
            const RgEnumValuesVector& dependency_flag_enumerators = GetDependencyFlagEnumerators();
            RgEditorElement* dependency_flags = new RgEditorElementEnum(parent_, kStrVulkanRenderPassDependencyDependencyFlags, dependency_flag_enumerators, reinterpret_cast<uint32_t*>(&offset_dependency_description->dependencyFlags), true);
            root_element->AppendChildItem(dependency_flags);

            // Connect to the splitter moved signal to close the drop down.
            build_view_vulkan = static_cast<RgBuildViewVulkan*>(parent_);
            is_connected = connect(build_view_vulkan, &RgBuildViewVulkan::SplitterMoved, static_cast<RgEditorElementEnum*>(dependency_flags), &RgEditorElementEnum::HotKeyPressedSignal);

            // Set object name.
            dependency_flags->setObjectName(kStrDependencyFlags);

            // Connect to the enum list widget status signal.
            is_connected = connect(dependency_flags, &RgEditorElement::EnumListWidgetStatusSignal, this, &RgPipelineStateModelVulkan::EnumListWidgetStatusSignal);
            assert(is_connected);

            // Connect the shortcut hot key signal.
            is_connected = connect(this, &RgPipelineStateModelVulkan::HotKeyPressedSignal, static_cast<RgEditorElementEnum*>(dependency_flags), &RgEditorElementEnum::HotKeyPressedSignal);
            assert(is_connected);
        }
    }
}

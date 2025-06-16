//=============================================================================
/// Copyright (c) 2021-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for an vulkan pso factory.
//=============================================================================

#pragma once

// Local.
#include "source/common/vulkan/rg_pipeline_types.h"

class RgPsoFactoryVulkan
{
public:
    RgPsoFactoryVulkan() = default;
    virtual ~RgPsoFactoryVulkan() = default;

    // Retrieve a new graphics pipeline recipe structure with default configuration.
    RgPsoGraphicsVulkan* GetDefaultGraphicsPsoCreateInfo();

    // Retrieve a new compute pipeline recipe structure with default configuration.
    RgPsoComputeVulkan* GetDefaultComputePsoCreateInfo();
};

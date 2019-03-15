#pragma once

// Local.
#include <Utils/Vulkan/Include/rgPipelineTypes.h>

class rgPsoFactoryVulkan
{
public:
    rgPsoFactoryVulkan() = default;
    virtual ~rgPsoFactoryVulkan() = default;

    // Retrieve a new graphics pipeline recipe structure with default configuration.
    rgPsoGraphicsVulkan* GetDefaultGraphicsPsoCreateInfo();

    // Retrieve a new compute pipeline recipe structure with default configuration.
    rgPsoComputeVulkan* GetDefaultComputePsoCreateInfo();
};
// C++.
#include <cassert>

// Local.
#include "source/common/vulkan/rg_pso_factory_vulkan.h"

RgPsoGraphicsVulkan* RgPsoFactoryVulkan::GetDefaultGraphicsPsoCreateInfo()
{
    RgPsoGraphicsVulkan* result = nullptr;

    // Create the default graphics pipeline create info recipe structure.
    result = new RgPsoGraphicsVulkan{};

    assert(result != nullptr);
    if (result != nullptr)
    {
        result->Initialize();
    }

    return result;
}

RgPsoComputeVulkan* RgPsoFactoryVulkan::GetDefaultComputePsoCreateInfo()
{
    RgPsoComputeVulkan* result = nullptr;

    // Create the default compute pipeline create info recipe structure.
    result = new RgPsoComputeVulkan{};

    assert(result != nullptr);
    if (result != nullptr)
    {
        result->Initialize();
    }

    return result;
}

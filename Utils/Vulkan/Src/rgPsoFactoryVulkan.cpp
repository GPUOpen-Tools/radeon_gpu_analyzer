// C++.
#include <cassert>

// Local.
#include <Utils/Vulkan/Include/rgPsoFactoryVulkan.h>

rgPsoGraphicsVulkan* rgPsoFactoryVulkan::GetDefaultGraphicsPsoCreateInfo()
{
    rgPsoGraphicsVulkan* pResult = nullptr;

    // Create the default graphics pipeline create info recipe structure.
    pResult = new rgPsoGraphicsVulkan{};

    assert(pResult != nullptr);
    if (pResult != nullptr)
    {
        pResult->Initialize();
    }

    return pResult;
}

rgPsoComputeVulkan* rgPsoFactoryVulkan::GetDefaultComputePsoCreateInfo()
{
    rgPsoComputeVulkan* pResult = nullptr;

    // Create the default compute pipeline create info recipe structure.
    pResult = new rgPsoComputeVulkan{};

    assert(pResult != nullptr);
    if (pResult != nullptr)
    {
        pResult->Initialize();
    }

    return pResult;
}
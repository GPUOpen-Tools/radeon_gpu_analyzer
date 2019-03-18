// C++.
#include <cassert>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/rgFactory.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDataTypesOpenCL.h>
#include <RadeonGPUAnalyzerGUI/Include/rgFactoryOpenCL.h>
#include <RadeonGPUAnalyzerGUI/Include/rgFactoryVulkan.h>

std::shared_ptr<rgFactory> rgFactory::CreateFactory(rgProjectAPI api)
{
    std::shared_ptr<rgFactory> pFactory = nullptr;

    switch (api)
    {
    case rgProjectAPI::OpenCL:
        {
            pFactory = std::make_shared<rgFactoryOpenCL>();
        }
        break;
    case rgProjectAPI::Vulkan:
        {
            pFactory = std::make_shared<rgFactoryVulkan>();
        }
        break;
    default:
        // If this assert fires, a new API factory must be implemented.
        assert(true);
        break;
    }

    // If this assert fires, the factory failed to be created properly.
    assert(pFactory != nullptr);

    return pFactory;
}
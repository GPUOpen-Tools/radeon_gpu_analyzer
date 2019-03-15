// Local.
#include <RadeonGPUAnalyzerGUI/Include/rgUtilsGraphics.h>
#include <RadeonGPUAnalyzerGUI/Include/rgUtilsVulkan.h>

std::shared_ptr<rgUtilsGraphics> rgUtilsGraphics::CreateUtility(rgProjectAPI api)
{
    std::shared_ptr<rgUtilsGraphics> pUtilityInstance = nullptr;

    switch (api)
    {
    case rgProjectAPI::Vulkan:
        {
            pUtilityInstance = std::make_shared<rgUtilsVulkan>();
        }
        break;
    default:
        // If this assert fires, a new graphics API utility instance must be implemented.
        assert(false && "Unknown API used to create graphics utility class.");
        break;
    }

    // If this assert fires, the factory failed to be created properly.
    assert(pUtilityInstance != nullptr);

    return pUtilityInstance;
}
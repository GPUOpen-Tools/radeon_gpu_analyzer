// Local.
#include "radeon_gpu_analyzer_gui/rg_utils_graphics.h"
#include "radeon_gpu_analyzer_gui/rg_utils_vulkan.h"

std::shared_ptr<RgUtilsGraphics> RgUtilsGraphics::CreateUtility(RgProjectAPI api)
{
    std::shared_ptr<RgUtilsGraphics> utility_instance = nullptr;

    switch (api)
    {
    case RgProjectAPI::kVulkan:
        {
            utility_instance = std::make_shared<RgUtilsVulkan>();
        }
        break;
    default:
        // If this assert fires, a new graphics API utility instance must be implemented.
        assert(false && "Unknown API used to create graphics utility class.");
        break;
    }

    // If this assert fires, the factory failed to be created properly.
    assert(utility_instance != nullptr);

    return utility_instance;
}

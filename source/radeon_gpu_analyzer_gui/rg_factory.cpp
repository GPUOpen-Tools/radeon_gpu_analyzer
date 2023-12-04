// C++.
#include <cassert>

// Local.
#include "radeon_gpu_analyzer_gui/rg_factory.h"
#include "radeon_gpu_analyzer_gui/rg_data_types_opencl.h"
#include "radeon_gpu_analyzer_gui/rg_factory_binary.h"
#include "radeon_gpu_analyzer_gui/rg_factory_opencl.h"
#include "radeon_gpu_analyzer_gui/rg_factory_vulkan.h"

std::shared_ptr<RgFactory> RgFactory::CreateFactory(RgProjectAPI api)
{
    std::shared_ptr<RgFactory> factory = nullptr;

    switch (api)
    {
    case RgProjectAPI::kOpenCL:
        {
            factory = std::make_shared<RgFactoryOpencl>();
        }
        break;
    case RgProjectAPI::kVulkan:
        {
            factory = std::make_shared<RgFactoryVulkan>();
        }
        break;
    case RgProjectAPI::kBinary:
        {
            factory = std::make_shared<RgFactoryBinary>();
        }
        break;
    default:
        // If this assert fires, a new API factory must be implemented.
        assert(true);
        break;
    }

    // If this assert fires, the factory failed to be created properly.
    assert(factory != nullptr);

    return factory;
}

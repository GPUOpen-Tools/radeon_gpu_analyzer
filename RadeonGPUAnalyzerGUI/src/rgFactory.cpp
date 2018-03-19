// C++.
#include <cassert>

// Local.
#include <RadeonGPUAnalyzerGUI/include/rgFactory.h>
#include <RadeonGPUAnalyzerGUI/include/rgOpenCLFactory.h>

std::shared_ptr<rgFactory> rgFactory::CreateFactory(rgProjectAPI api)
{
    std::shared_ptr<rgFactory> pFactory = nullptr;

    switch (api)
    {
    case OpenCL:
        {
            pFactory = std::make_shared<rgOpenCLFactory>();
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
#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_FACTORY_GRAPHICS_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_FACTORY_GRAPHICS_H_

// Local.
#include "radeon_gpu_analyzer_gui/rg_factory.h"

// Forward declarations.
class RgPipelineStateModel;
class RgPipelineStateView;

class RgFactoryGraphics : public RgFactory
{
public:
    RgFactoryGraphics() = default;
    virtual ~RgFactoryGraphics() = default;

    // Create an API-specific Pipeline State model instance.
    virtual RgPipelineStateModel* CreatePipelineStateModel(QWidget* parent) = 0;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_FACTORY_GRAPHICS_H_

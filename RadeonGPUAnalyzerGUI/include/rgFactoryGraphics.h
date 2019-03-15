#pragma once

// Local.
#include <RadeonGPUAnalyzerGUI/Include/rgFactory.h>

// Forward declarations.
class rgPipelineStateModel;
class rgPipelineStateView;

class rgFactoryGraphics : public rgFactory
{
public:
    rgFactoryGraphics() = default;
    virtual ~rgFactoryGraphics() = default;

    // Create an API-specific Pipeline State model instance.
    virtual rgPipelineStateModel* CreatePipelineStateModel(QWidget* pParent) = 0;
};
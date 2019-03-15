#pragma once

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgSettingsTab.h>

// A Vulkan-specific implementation of the settings tab.
class rgSettingsTabVulkan : public rgSettingsTab
{
    Q_OBJECT

public:
    rgSettingsTabVulkan(QWidget* pParent);
    virtual ~rgSettingsTabVulkan() = default;

protected:
    virtual rgProjectAPI GetApiType() override;
};

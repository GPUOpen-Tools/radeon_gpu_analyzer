#pragma once

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgSettingsTab.h>

// An OpenCL-specific implementation of the settings tab.
class rgSettingsTabOpenCL : public rgSettingsTab
{
    Q_OBJECT

public:
    rgSettingsTabOpenCL(QWidget* pParent);
    virtual ~rgSettingsTabOpenCL() = default;

protected:
    virtual rgProjectAPI GetApiType() override;
};

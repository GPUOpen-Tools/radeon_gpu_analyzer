#pragma once

// Local.
#include "source/radeon_gpu_analyzer_gui/qt/rg_settings_tab.h"

// A Binary-specific implementation of the settings tab.
class RgSettingsTabBinary : public RgSettingsTab
{
    Q_OBJECT

public:
    RgSettingsTabBinary(QWidget* parent);
    virtual ~RgSettingsTabBinary() = default;

protected:
    virtual RgProjectAPI GetApiType() override;
};

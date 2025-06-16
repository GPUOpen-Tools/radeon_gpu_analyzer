//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for a Binary-specific implementation of the settings tab.
//=============================================================================
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

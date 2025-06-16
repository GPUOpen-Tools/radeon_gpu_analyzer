//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for a Binary-specific implementation of the settings tab.
//=============================================================================
// C++.
#include <cassert>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_settings_tab_binary.h"

RgSettingsTabBinary::RgSettingsTabBinary(QWidget* parent)
    : RgSettingsTab(parent)
{
}

RgProjectAPI RgSettingsTabBinary::GetApiType()
{
    return RgProjectAPI::kBinary;
}

//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for a OpenCL-specific implementation of the settings tab.
//=============================================================================
// C++.
#include <cassert>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_settings_tab_opencl.h"

RgSettingsTabOpencl::RgSettingsTabOpencl(QWidget* parent)
    : RgSettingsTab(parent)
{
}

RgProjectAPI RgSettingsTabOpencl::GetApiType()
{
    return RgProjectAPI::kOpenCL;
}

//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for a OpenCL-specific implementation of the settings tab.
//=============================================================================
#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_SETTINGS_TAB_OPENCL_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_SETTINGS_TAB_OPENCL_H_

// Local.
#include "source/radeon_gpu_analyzer_gui/qt/rg_settings_tab.h"

// An OpenCL-specific implementation of the settings tab.
class RgSettingsTabOpencl : public RgSettingsTab
{
    Q_OBJECT

public:
    RgSettingsTabOpencl(QWidget* parent);
    virtual ~RgSettingsTabOpencl() = default;

protected:
    virtual RgProjectAPI GetApiType() override;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_SETTINGS_TAB_OPENCL_H_

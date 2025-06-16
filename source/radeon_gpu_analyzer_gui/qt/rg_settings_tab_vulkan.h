//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for a Vulkan-specific implementation of the settings tab.
//=============================================================================
#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_SETTINGS_TAB_VULKAN_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_SETTINGS_TAB_VULKAN_H_

// Local.
#include "source/radeon_gpu_analyzer_gui/qt/rg_settings_tab.h"

// A Vulkan-specific implementation of the settings tab.
class RgSettingsTabVulkan : public RgSettingsTab
{
    Q_OBJECT

public:
    RgSettingsTabVulkan(QWidget* parent);
    virtual ~RgSettingsTabVulkan() = default;

protected:
    virtual RgProjectAPI GetApiType() override;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_SETTINGS_TAB_VULKAN_H_

//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for a Vulkan-specific implementation of the status bar.
//=============================================================================
#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_STATUS_BAR_VULKAN_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_STATUS_BAR_VULKAN_H_

// C++.
#include <memory>

// Qt.
#include <QtWidgets/QWidget>
#include <QPushButton>

// Local.
#include "source/radeon_gpu_analyzer_gui/qt/rg_status_bar.h"

class RgStatusBarVulkan : public RgStatusBar
{
    Q_OBJECT

public:
    RgStatusBarVulkan(QStatusBar* status_bar, QWidget* parent = nullptr);
    virtual ~RgStatusBarVulkan() = default;

private:
    // Set style sheets for mode and API push buttons.
    virtual void SetStylesheets(QStatusBar* status_bar) override;

    // The parent widget.
    QWidget* parent_ = nullptr;
};
#endif  // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_STATUS_BAR_VULKAN_H_

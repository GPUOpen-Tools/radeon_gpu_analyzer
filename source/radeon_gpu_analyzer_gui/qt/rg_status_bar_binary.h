//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for a Binary-specific implementation of the status bar.
//=============================================================================
#pragma once

// Local.
#include "source/radeon_gpu_analyzer_gui/qt/rg_status_bar.h"

class RgStatusBarBinary : public RgStatusBar
{
    Q_OBJECT

public:
    RgStatusBarBinary(QStatusBar* status_bar, QWidget* parent = nullptr);
    virtual ~RgStatusBarBinary() = default;

    // Returns status message string.
    bool ConstructStatusMessageString(StatusType type, std::string& status_msg_str) const override;

private:
    // Set style sheets for mode and API push buttons.
    virtual void SetStylesheets(QStatusBar* status_bar) override;

    // The parent widget.
    QWidget* parent_ = nullptr;
};

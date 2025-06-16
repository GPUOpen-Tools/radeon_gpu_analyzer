//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for RGA's Browse Push Button.
//=============================================================================

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_browse_button.h"

RgBrowseButton::RgBrowseButton(QWidget* parent) : QPushButton(parent)
{
}

void RgBrowseButton::focusInEvent(QFocusEvent* event)
{
    emit BrowseButtonFocusInEvent();

    // Pass the event onto the base class.
    QPushButton::focusInEvent(event);
}

void RgBrowseButton::focusOutEvent(QFocusEvent* event)
{
    emit BrowseButtonFocusOutEvent();

    // Pass the event onto the base class.
    QPushButton::focusOutEvent(event);
}

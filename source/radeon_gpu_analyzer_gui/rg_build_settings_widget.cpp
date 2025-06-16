//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for Build settings widget.
//=============================================================================

// C++.
#include <sstream>
#include <cassert>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_build_settings_widget.h"

RgBuildSettingsWidget::RgBuildSettingsWidget(QWidget* parent) : QFrame(parent)
{
    setObjectName("buildSettingsWidget");
}

void RgBuildSettingsWidget::focusInEvent(QFocusEvent* event)
{
    emit FrameFocusInEventSignal();

    // Pass the event onto the base class.
    QFrame::focusInEvent(event);
}

void RgBuildSettingsWidget::focusOutEvent(QFocusEvent* event)
{
    emit FrameFocusOutEventSignal();

    // Pass the event onto the base class.
    QFrame::focusOutEvent(event);
}

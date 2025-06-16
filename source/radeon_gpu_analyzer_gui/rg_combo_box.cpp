//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for RGA specific implementation of QComboBox.
//=============================================================================

// C++.
#include <sstream>
#include <cassert>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_combo_box.h"

RgComboBox::RgComboBox(QWidget* parent) : QComboBox(parent)
{
}

void RgComboBox::mousePressEvent(QMouseEvent* event)
{
    emit ComboBoxFocusInEvent();

    // Pass the event onto the base class.
    QComboBox::mousePressEvent(event);
}

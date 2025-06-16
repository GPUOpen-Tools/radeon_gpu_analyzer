//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for rga specific scroll area.
//=============================================================================

// C++.
#include <sstream>
#include <cassert>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_scroll_area.h"

RgScrollArea::RgScrollArea(QWidget* parent) : QScrollArea(parent)
{
}

void RgScrollArea::mousePressEvent(QMouseEvent* event)
{
    emit ScrollAreaClickedEvent();

    // Pass the event onto the base class.
    QScrollArea::mousePressEvent(event);
}

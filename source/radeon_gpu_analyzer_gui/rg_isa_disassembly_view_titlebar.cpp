//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for shader ISA Disassembly view titlebar widget.
//=============================================================================

// C++.
#include <sstream>

// Qt.
#include <QStyleOption>
#include <QPainter>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_isa_disassembly_view_titlebar.h"

RgIsaDisassemblyViewTitlebar::RgIsaDisassemblyViewTitlebar(QWidget* parent) :
    QWidget(parent)
{
}

void RgIsaDisassemblyViewTitlebar::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QStyleOption option;
    option.initFrom(this);
    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget, &option, &painter, this);
};

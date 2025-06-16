//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for RGA specific implementation of QLineEdit.
//=============================================================================

// C++.
#include <sstream>
#include <cassert>

// Qt.
#include <QPainter>
#include <QTimer>

// Infra.
#include "qt_common/utils/qt_util.h"

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_line_edit.h"

// The horizontal location of where to show the edit value.
static const int kPaintLocationX = 5;

// The height of the line edit.
static const int kLineEditSize = 18;

// The vertical margin of the highlight.
static const int kHighlightVerticalMargin = 5;

// The margin to leave from the top of the line edit
// before drawing the text.
static const int kVerticalMargin = 3;

RgLineEdit::RgLineEdit(QWidget* parent) : QLineEdit(parent)
{
}

void RgLineEdit::focusInEvent(QFocusEvent* event)
{
    emit LineEditFocusInEvent();

    // Pass the event onto the base class.
    QLineEdit::focusInEvent(event);

    // Select the text using single shot timer.
    QTimer::singleShot(0, this, &QLineEdit::selectAll);
}

void RgLineEdit::focusOutEvent(QFocusEvent* event)
{
    emit LineEditFocusOutEvent();

    // Pass the event onto the base class.
    QLineEdit::focusOutEvent(event);
}

void RgLineEdit::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    // Let the base class draw everything, i.e. text, cursor, border, etc.
    QLineEdit::paintEvent(event);

    // Set up the painter.
    QPainter painter;
    painter.begin(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Set the font size.
    QFont font = this->font();
    font.setPointSize(9);
    painter.setFont(font);

    // Highlight substring if it is requested.
    if (highlight_sub_string_)
    {
        // Go through all highlight locations.
        for (const auto& string_highlight_data : string_highlight_data_)
        {
            QString current = text().mid(0, string_highlight_data.start_location);
            int initial_text_width = QtCommon::QtUtils::GetTextWidth(font, current);

            current = text().mid(string_highlight_data.start_location,
                string_highlight_data.end_location - string_highlight_data.start_location);
            int width = QtCommon::QtUtils::GetTextWidth(font, current);
            QRect rect = this->rect();
            rect.setX(rect.x() + kPaintLocationX + initial_text_width);
            rect.setHeight(rect.height() - kHighlightVerticalMargin);
            rect.setY(rect.y() + kHighlightVerticalMargin);
            rect.setWidth(width);
            painter.fillRect(rect, string_highlight_data.highlight_color);
        }
    }

    painter.end();
}

void RgLineEdit::SetHighlightSubStringData(QVector<StringHighlightData> string_highlight_data)
{
    string_highlight_data_ = string_highlight_data;
}

void RgLineEdit::SetHighlightSubString(bool value)
{
    highlight_sub_string_ = value;
}

// C++.
#include <sstream>
#include <cassert>

// Qt.
#include <QPainter>

// Infra.
#include "QtCommon/Scaling/ScalingManager.h"
#include "QtCommon/Util/QtUtil.h"

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_label.h"

// The horizontal location of where to show the edit value.
static const int kPaintLocationX = 5;

// The height of the label.
static const int kLabelSize = 18;

// The vertical margin of the highlight.
static const int kHighlightVerticalMargin = 3;

// The margin to leave from the top of the label
// before drawing the text.
static const int kVerticalMargin = 3;

RgLabel::RgLabel(QWidget* parent) : QLabel(parent)
{
}

void RgLabel::focusInEvent(QFocusEvent* event)
{
    // Emit an event indicating that this widget got focus.
    // This is necessary so that external objects can be signaled when this occurs.
    emit LabelFocusInEventSignal();

    // Pass the event onto the base class.
    QLabel::focusInEvent(event);
}

void RgLabel::focusOutEvent(QFocusEvent* event)
{
    // Emit an event indicating that this widget lost focus.
    // This is necessary so that external objects can be signaled when this occurs.
    emit LabelFocusOutEventSignal();

    // Pass the event onto the base class.
    QLabel::focusOutEvent(event);
}

void RgLabel::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    // Set up the painter.
    QPainter painter;
    painter.begin(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Set properties for the text.
    QPen pen;
    pen.setColor(Qt::black);
    pen.setWidth(1);
    painter.setPen(pen);

    // Draw the text.
    QFont font = this->font();
    font.setPointSize(8);
    painter.setFont(font);

    // Highlight substring if it is requested.
    if (should_highlight_sub_string_)
    {
        // Go through all highlight locations.
        for (const auto& string_highlight_data : string_highlight_data_)
        {
            QString current = text().mid(0, string_highlight_data.m_startLocation);
            int initial_text_width = QtCommon::QtUtil::GetTextWidth(font, current);

            assert(string_highlight_data.m_endLocation - string_highlight_data.m_startLocation > 0);
            current = text().mid(string_highlight_data.m_startLocation,
                string_highlight_data.m_endLocation - string_highlight_data.m_startLocation);

            assert(!current.isNull());
            if (!current.isNull())
            {
                int width = QtCommon::QtUtil::GetTextWidth(font, current);
                QRect rect = this->rect();
                rect.setX(rect.x() + kPaintLocationX + initial_text_width);
                rect.setHeight(rect.height() - kHighlightVerticalMargin);
                rect.setY(rect.y() + kHighlightVerticalMargin);
                rect.setWidth(width);
                painter.fillRect(rect, string_highlight_data.m_highlightColor);
            }
        }
    }

    // Get the scaling factor.
    const double scaling_factor = ScalingManager::Get().GetScaleFactor();

    // Draw the text.
    painter.drawText(kPaintLocationX, (kLabelSize / 2 + kVerticalMargin) * scaling_factor, text());

    painter.end();
}

void RgLabel::SetHighlightSubStringData(QVector<StringHighlightData> string_highlight_data)
{
    string_highlight_data_ = string_highlight_data;
}

void RgLabel::SetHighlightSubString(bool value)
{
    should_highlight_sub_string_ = value;
}

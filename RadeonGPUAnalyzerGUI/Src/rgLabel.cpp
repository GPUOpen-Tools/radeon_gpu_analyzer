// C++.
#include <sstream>
#include <cassert>

// Qt.
#include <QPainter>

// Infra.
#include <QtCommon/Scaling/ScalingManager.h>
#include <QtCommon/Util/QtUtil.h>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgLabel.h>

// The horizontal location of where to show the edit value.
static const int s_PAINT_LOCATION_X = 5;

// The height of the label.
static const int s_LABEL_SIZE = 18;

// The vertical margin of the highlight.
static const int s_HIGHLIGHT_VERTICAL_MARGIN = 3;

// The margin to leave from the top of the label
// before drawing the text.
static const int s_VERTICAL_MARGIN = 3;

rgLabel::rgLabel(QWidget* pParent) : QLabel(pParent)
{
}

void rgLabel::focusInEvent(QFocusEvent* pEvent)
{
    // Emit an event indicating that this widget got focus.
    // This is necessary so that external objects can be signaled when this occurs.
    emit LabelFocusInEventSignal();

    // Pass the event onto the base class.
    QLabel::focusInEvent(pEvent);
}

void rgLabel::focusOutEvent(QFocusEvent* pEvent)
{
    // Emit an event indicating that this widget lost focus.
    // This is necessary so that external objects can be signaled when this occurs.
    emit LabelFocusOutEventSignal();

    // Pass the event onto the base class.
    QLabel::focusOutEvent(pEvent);
}

void rgLabel::paintEvent(QPaintEvent* pEvent)
{
    Q_UNUSED(pEvent);

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
    if (m_highlightSubString)
    {
        // Go through all highlight locations.
        for (const auto& stringHighlightData : m_stringHighlightData)
        {
            QString current = text().mid(0, stringHighlightData.m_startLocation);
            int initialTextWidth = QtCommon::QtUtil::GetTextWidth(font, current);

            assert(stringHighlightData.m_endLocation - stringHighlightData.m_startLocation > 0);
            current = text().mid(stringHighlightData.m_startLocation,
                stringHighlightData.m_endLocation - stringHighlightData.m_startLocation);

            assert(!current.isNull());
            if (!current.isNull())
            {
                int width = QtCommon::QtUtil::GetTextWidth(font, current);
                QRect rect = this->rect();
                rect.setX(rect.x() + s_PAINT_LOCATION_X + initialTextWidth);
                rect.setHeight(rect.height() - s_HIGHLIGHT_VERTICAL_MARGIN);
                rect.setY(rect.y() + s_HIGHLIGHT_VERTICAL_MARGIN);
                rect.setWidth(width);
                painter.fillRect(rect, stringHighlightData.m_highlightColor);
            }
        }
    }

    // Get the scaling factor.
    const double scalingFactor = ScalingManager::Get().GetScaleFactor();

    // Draw the text.
    painter.drawText(s_PAINT_LOCATION_X, (s_LABEL_SIZE / 2 + s_VERTICAL_MARGIN) * scalingFactor, text());

    painter.end();
}

void rgLabel::SetHighlightSubStringData(QVector<StringHighlightData> stringHighlightData)
{
    m_stringHighlightData = stringHighlightData;
}

void rgLabel::SetHighlightSubString(bool value)
{
    m_highlightSubString = value;
}
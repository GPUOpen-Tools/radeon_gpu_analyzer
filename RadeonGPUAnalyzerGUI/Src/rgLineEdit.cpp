// C++.
#include <sstream>
#include <cassert>

// Qt.
#include <QPainter>

// Infra.
#include <QtCommon/Scaling/ScalingManager.h>
#include <QtCommon/Util/QtUtil.h>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgLineEdit.h>

// The horizontal location of where to show the edit value.
static const int s_PAINT_LOCATION_X = 5;

// The height of the line edit.
static const int s_LINE_EDIT_SIZE = 18;

// The vertical margin of the highlight.
static const int s_HIGHLIGHT_VERTICAL_MARGIN = 5;

// The margin to leave from the top of the line edit
// before drawing the text.
static const int s_VERTICAL_MARGIN = 3;

rgLineEdit::rgLineEdit(QWidget* pParent) : QLineEdit(pParent)
{
}

void rgLineEdit::focusInEvent(QFocusEvent* pEvent)
{
    emit LineEditFocusInEvent();

    // Pass the event onto the base class.
    QLineEdit::focusInEvent(pEvent);

    // Select the text using single shot timer.
    QTimer::singleShot(0, this, &QLineEdit::selectAll);
}

void rgLineEdit::focusOutEvent(QFocusEvent* pEvent)
{
    emit LineEditFocusOutEvent();

    // Pass the event onto the base class.
    QLineEdit::focusOutEvent(pEvent);
}

void rgLineEdit::paintEvent(QPaintEvent* pEvent)
{
    Q_UNUSED(pEvent);

    // Let the base class draw everything, i.e. text, cursor, border, etc.
    QLineEdit::paintEvent(pEvent);

    // Set up the painter.
    QPainter painter;
    painter.begin(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Set the font size.
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

            current = text().mid(stringHighlightData.m_startLocation,
                stringHighlightData.m_endLocation - stringHighlightData.m_startLocation);
            int width = QtCommon::QtUtil::GetTextWidth(font, current);
            QRect rect = this->rect();
            rect.setX(rect.x() + s_PAINT_LOCATION_X + initialTextWidth);
            rect.setHeight(rect.height() - s_HIGHLIGHT_VERTICAL_MARGIN);
            rect.setY(rect.y() + s_HIGHLIGHT_VERTICAL_MARGIN);
            rect.setWidth(width);
            painter.fillRect(rect, stringHighlightData.m_highlightColor);
        }
    }

    painter.end();
}

void rgLineEdit::SetHighlightSubStringData(QVector<StringHighlightData> stringHighlightData)
{
    m_stringHighlightData = stringHighlightData;
}

void rgLineEdit::SetHighlightSubString(bool value)
{
    m_highlightSubString = value;
}
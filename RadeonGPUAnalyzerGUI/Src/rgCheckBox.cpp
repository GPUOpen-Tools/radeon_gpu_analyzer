// C++.
#include <sstream>
#include <cassert>

// Qt.
#include <QMouseEvent>
#include <QToolTip>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgCheckBox.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDataTypesOpenCL.h>

rgCheckBox::rgCheckBox(QWidget* pParent) : QCheckBox(pParent)
{
}

void rgCheckBox::focusInEvent(QFocusEvent* pEvent)
{
    emit CheckBoxFocusInEvent();

    // Pass the event onto the base class.
    QCheckBox::focusInEvent(pEvent);
}

void rgCheckBox::focusOutEvent(QFocusEvent* pEvent)
{
    emit CheckBoxFocusOutEvent();

    // Pass the event onto the base class.
    QCheckBox::focusOutEvent(pEvent);
}

void rgCheckBox::mouseMoveEvent(QMouseEvent* pEvent)
{
    // Get the widget name.
    QString widgetName = this->objectName();

    // Set tooltip for the widget hovered over.
    QString tooltip = TOOLTIPS[widgetName];

    // Display the tooltip.
    QToolTip::showText(pEvent->globalPos(), tooltip);

    // Pass the event onto the base class.
    QCheckBox::mouseMoveEvent(pEvent);
}
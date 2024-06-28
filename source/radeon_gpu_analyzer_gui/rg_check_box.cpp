// C++.
#include <sstream>
#include <cassert>

// Qt.
#include <QMouseEvent>
#include <QToolTip>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_check_box.h"
#include "radeon_gpu_analyzer_gui/rg_data_types_opencl.h"

RgCheckBox::RgCheckBox(QWidget* parent) : ScaledCheckBox(parent)
{
}

void RgCheckBox::focusInEvent(QFocusEvent* event)
{
    emit CheckBoxFocusInEvent();

    // Pass the event onto the base class.
    QCheckBox::focusInEvent(event);
}

void RgCheckBox::focusOutEvent(QFocusEvent* event)
{
    emit CheckBoxFocusOutEvent();

    // Pass the event onto the base class.
    QCheckBox::focusOutEvent(event);
}

void RgCheckBox::mouseMoveEvent(QMouseEvent* event)
{
    // Get the widget name.
    QString widgetName = this->objectName();

    // Set tooltip for the widget hovered over.
    QString tooltip = TOOLTIPS[widgetName];

    // Display the tooltip.
    QToolTip::showText(event->globalPosition().toPoint(), tooltip);

    // Pass the event onto the base class.
    QCheckBox::mouseMoveEvent(event);
}

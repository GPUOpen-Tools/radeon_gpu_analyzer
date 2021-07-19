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

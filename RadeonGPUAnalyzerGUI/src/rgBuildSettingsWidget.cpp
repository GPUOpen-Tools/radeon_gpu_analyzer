// C++.
#include <sstream>
#include <cassert>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgBuildSettingsWidget.h>

rgBuildSettingsWidget::rgBuildSettingsWidget(QWidget* pParent) : QFrame(pParent)
{
    setObjectName("buildSettingsWidget");
}

void rgBuildSettingsWidget::focusInEvent(QFocusEvent* pEvent)
{
    emit FrameFocusInEventSignal();

    // Pass the event onto the base class.
    QFrame::focusInEvent(pEvent);
}

void rgBuildSettingsWidget::focusOutEvent(QFocusEvent* pEvent)
{
    emit FrameFocusOutEventSignal();

    // Pass the event onto the base class.
    QFrame::focusOutEvent(pEvent);
}
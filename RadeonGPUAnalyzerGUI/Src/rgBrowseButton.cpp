// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgBrowseButton.h>

rgBrowseButton::rgBrowseButton(QWidget* pParent) : QPushButton(pParent)
{
}

void rgBrowseButton::focusInEvent(QFocusEvent* pEvent)
{
    emit BrowseButtonFocusInEvent();

    // Pass the event onto the base class.
    QPushButton::focusInEvent(pEvent);
}

void rgBrowseButton::focusOutEvent(QFocusEvent* pEvent)
{
    emit BrowseButtonFocusOutEvent();

    // Pass the event onto the base class.
    QPushButton::focusOutEvent(pEvent);
}
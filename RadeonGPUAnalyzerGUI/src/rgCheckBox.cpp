// C++.
#include <sstream>
#include <cassert>

// Local.
#include <RadeonGPUAnalyzerGUI/include/qt/rgCheckBox.h>

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
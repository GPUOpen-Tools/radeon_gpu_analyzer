// C++.
#include <sstream>
#include <cassert>

// Local.
#include <RadeonGPUAnalyzerGUI/include/qt/rgLineEdit.h>

rgLineEdit::rgLineEdit(QWidget* pParent) : QLineEdit(pParent)
{
}

void rgLineEdit::focusInEvent(QFocusEvent* pEvent)
{
    emit LineEditFocusInEvent();

    // Pass the event onto the base class.
    QLineEdit::focusInEvent(pEvent);
}

void rgLineEdit::focusOutEvent(QFocusEvent* pEvent)
{
    emit LineEditFocusOutEvent();

    // Pass the event onto the base class.
    QLineEdit::focusOutEvent(pEvent);
}
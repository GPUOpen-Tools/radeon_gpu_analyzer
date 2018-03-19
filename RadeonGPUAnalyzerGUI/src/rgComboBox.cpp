// C++.
#include <sstream>
#include <cassert>

// Local.
#include <RadeonGPUAnalyzerGUI/include/qt/rgComboBox.h>

rgComboBox::rgComboBox(QWidget* pParent) : QComboBox(pParent)
{
}

void rgComboBox::mousePressEvent(QMouseEvent* pEvent)
{
    emit ComboBoxFocusInEvent();

    // Pass the event onto the base class.
    QComboBox::mousePressEvent(pEvent);
}
// C++.
#include <sstream>
#include <cassert>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_combo_box.h"

RgComboBox::RgComboBox(QWidget* parent) : QComboBox(parent)
{
}

void RgComboBox::mousePressEvent(QMouseEvent* event)
{
    emit ComboBoxFocusInEvent();

    // Pass the event onto the base class.
    QComboBox::mousePressEvent(event);
}

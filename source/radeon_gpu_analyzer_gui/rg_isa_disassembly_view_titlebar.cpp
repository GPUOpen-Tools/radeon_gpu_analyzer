// C++.
#include <sstream>

// Qt.
#include <QStyleOption>
#include <QPainter>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_isa_disassembly_view_titlebar.h"

RgIsaDisassemblyViewTitlebar::RgIsaDisassemblyViewTitlebar(QWidget* parent) :
    QWidget(parent)
{
}

void RgIsaDisassemblyViewTitlebar::paintEvent(QPaintEvent *event)
{
    QStyleOption option;
    option.initFrom(this);
    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget, &option, &painter, this);
};

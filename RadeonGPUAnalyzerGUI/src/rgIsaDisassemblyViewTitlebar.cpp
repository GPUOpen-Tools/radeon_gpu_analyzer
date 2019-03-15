// C++.
#include <sstream>

// Qt.
#include <QStyleOption>
#include <QPainter>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgIsaDisassemblyViewTitlebar.h>

rgIsaDisassemblyViewTitlebar::rgIsaDisassemblyViewTitlebar(QWidget* pParent) :
    QWidget(pParent)
{
}

void rgIsaDisassemblyViewTitlebar::mousePressEvent(QMouseEvent* pEvent)
{
    emit FrameFocusInSignal();
}

void rgIsaDisassemblyViewTitlebar::mouseDoubleClickEvent(QMouseEvent* pEvent)
{
    emit ViewTitleBarDoubleClickedSignal();
}

void rgIsaDisassemblyViewTitlebar::paintEvent(QPaintEvent *pEvent)
{
    QStyleOption option;
    option.initFrom(this);
    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget, &option, &painter, this);
};

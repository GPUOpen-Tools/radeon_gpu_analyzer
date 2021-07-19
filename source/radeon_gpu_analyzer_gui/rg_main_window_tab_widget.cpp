// Qt.
#include <QResizeEvent>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_main_window_tab_widget.h"

RgMainWindowTabWidget::RgMainWindowTabWidget(QWidget* parent) :
    QTabWidget(parent)
{
    tab_bar_ = new RgMainWindowTabBar();

    tab_bar_->setParent(this);

    // Replace the RgMainWindowTabWidget's QTabBar with a custom one.
    setTabBar(tab_bar_);

    // Set the focus policy.
    setFocusPolicy(Qt::FocusPolicy::NoFocus);
}

QTabBar* RgMainWindowTabWidget::tabBar() const
{
    return QTabWidget::tabBar();
}

RgMainWindowTabBar* RgMainWindowTabWidget::GetTabBar()
{
    return tab_bar_;
}

int RgMainWindowTabWidget::TabHeight() const
{
    return tabBar()->sizeHint().height();
}

void RgMainWindowTabWidget::resizeEvent(QResizeEvent* pResizeEvent)
{
    if (tab_bar_ != nullptr)
    {
        tab_bar_->resize(pResizeEvent->size());
        QTabWidget::resizeEvent(pResizeEvent);
    }
}

void RgMainWindowTabWidget::SetTabEnabled(int index, bool enable)
{
    if (tab_bar_ != nullptr)
    {
        tab_bar_->setTabEnabled(index, enable);
    }
}

void RgMainWindowTabWidget::SetSpacerIndex(const int index)
{
    if (tab_bar_ != nullptr)
    {
        tab_bar_->SetSpacerIndex(index);
    }
}

void RgMainWindowTabWidget::SetTabTool(int index, QWidget* tool_widget)
{
    if (tab_bar_ != nullptr)
    {
        tab_bar_->SetTabTool(index, tool_widget);
    }
}

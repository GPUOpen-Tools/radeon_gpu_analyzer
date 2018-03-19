// Qt.
#include <QResizeEvent>

// Local.
#include <RadeonGPUAnalyzerGUI/include/qt/rgMainWindowTabWidget.h>

rgMainWindowTabWidget::rgMainWindowTabWidget(QWidget* pParent) :
    QTabWidget(pParent)
{
    m_pTabBar = new rgMainWindowTabBar();

    m_pTabBar->setParent(this);

    // Replace the rgMainWindowTabWidget's QTabBar with a custom one.
    setTabBar(m_pTabBar);
}

QTabBar* rgMainWindowTabWidget::tabBar() const
{
    return QTabWidget::tabBar();
}

rgMainWindowTabBar* rgMainWindowTabWidget::GetTabBar()
{
    return m_pTabBar;
}

int rgMainWindowTabWidget::TabHeight() const
{
    return tabBar()->sizeHint().height();
}

void rgMainWindowTabWidget::resizeEvent(QResizeEvent* pResizeEvent)
{
    if (m_pTabBar != nullptr)
    {
        m_pTabBar->resize(pResizeEvent->size());
        QTabWidget::resizeEvent(pResizeEvent);
    }
}

void rgMainWindowTabWidget::SetTabEnabled(int index, bool enable)
{
    if (m_pTabBar != nullptr)
    {
        m_pTabBar->setTabEnabled(index, enable);
    }
}

void rgMainWindowTabWidget::SetSpacerIndex(const int index)
{
    if (m_pTabBar != nullptr)
    {
        m_pTabBar->SetSpacerIndex(index);
    }
}

void rgMainWindowTabWidget::SetTabTool(int index, QWidget* pToolWidget)
{
    if (m_pTabBar != nullptr)
    {
        m_pTabBar->SetTabTool(index, pToolWidget);
    }
}

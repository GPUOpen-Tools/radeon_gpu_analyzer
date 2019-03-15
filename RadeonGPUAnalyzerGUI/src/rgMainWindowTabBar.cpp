// C++.
#include <cassert>

// Qt.
#include <QTabBar>
#include <QMouseEvent>
#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QSizePolicy>
#include <QPainter>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMainWindowTabBar.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgSettingsTab.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgUnsavedItemsDialog.h>
#include <RadeonGPUAnalyzerGUI/Include/rgStringConstants.h>
#include <RadeonGPUAnalyzerGUI/Include/rgUtils.h>

// Infra.
#include <QtCommon/Scaling/ScalingManager.h>

static const int s_TAB_BAR_HEIGHT = 47;
static const int s_TAB_BAR_WIDTH = 100;

rgMainWindowTabBar::rgMainWindowTabBar(QWidget* pParent) :
    QTabBar(pParent),
    m_spacerIndex(-1),
    m_mouseHoverLastTabIndex(-1),
    m_pParent(pParent),
    m_pSettingsTab(nullptr)
{
    setMouseTracking(true);
    setObjectName("rgMainWindowTabBar");

    // Install an app-level event filter so that this widget can process
    // [Shift+]Ctrl+Tab key press events before other widgets steal them.
    // In particular this is needed for the Tab key, since the tab key
    // on its own causes focus to step between widgets on other parts of the UI.
    qApp->installEventFilter(this);

    // Set the font stylesheet.
    setStyleSheet("font: bold 14px;");
}

void rgMainWindowTabBar::mouseMoveEvent(QMouseEvent* pEvent)
{
    int tabIndex = QTabBar::tabAt(pEvent->pos());

    // Only change mouse cursor if the mouse is hovering over a different tab.
    if (m_mouseHoverLastTabIndex != tabIndex)
    {
        if (isTabEnabled(tabIndex))
        {
            setCursor(Qt::PointingHandCursor);
        }
        else
        {
            setCursor(Qt::ArrowCursor);
        }
        m_mouseHoverLastTabIndex = tabIndex;
    }
}

void rgMainWindowTabBar::mousePressEvent(QMouseEvent* pEvent)
{
    int tabIndex = QTabBar::tabAt(pEvent->pos());

    // Check if the user is switching to the "START" tab.
    switch (tabIndex)
    {
    case (TabItem::Start):
    {
        bool userMadeSelection = true;

        assert(m_pSettingsTab != nullptr);
        if (m_pSettingsTab != nullptr)
        {
            userMadeSelection = m_pSettingsTab->PromptToSavePendingChanges();
        }

        if (userMadeSelection)
        {
            // User did not cancel the prompt, so let the tab switch occur.
            // Pass the event onto the base class.
            QTabBar::mousePressEvent(pEvent);
        }
        break;
    }
    case (TabItem::Settings):
    {
        // The user is switching to settings tab, allow it.
        QTabBar::mousePressEvent(pEvent);
        break;
    }
    default:
    {
        // Should not get here.
        assert(false);
    }
    }
}

bool rgMainWindowTabBar::eventFilter(QObject* pObject, QEvent* pEvent)
{
    bool filtered = false;

    if (pEvent != nullptr)
    {
        if (pEvent->type() == QEvent::KeyPress)
        {
            // Only process the events if the SettingsTab is the current tab.
            if (currentIndex() == TabItem::Settings)
            {
                if (m_pSettingsTab != nullptr)
                {
                    QKeyEvent* pKeyEvent = static_cast<QKeyEvent*>(pEvent);
                    if (pKeyEvent != nullptr)
                    {
                        // If Ctrl+Shift+Tab or Ctrl+Tab is switching tabs, prompt to save pending
                        // settings changes first.
                        // Note: The [Shift] is just a way to choose prev or next direction, but
                        // we only have two tabs, so they both behave the same way.
                        const int keyPressed = pKeyEvent->key();
                        if ((keyPressed == Qt::Key_Tab || keyPressed == Qt::Key_Backtab) &&
                            (pKeyEvent->modifiers() & Qt::ControlModifier))
                        {
                            // Prompt user to save pending changes.
                            bool userMadeSelection = m_pSettingsTab->PromptToSavePendingChanges();
                            if (userMadeSelection == false)
                            {
                                // User cancelled the prompt, so the event should be filtered out.
                                filtered = true;
                            }
                        }
                    }
                }
            }
        }
    }

    // Allow base class to filter the event if needed.
    if (!filtered)
    {
        filtered = QTabBar::eventFilter(pObject, pEvent);
    }

    return filtered;
}

void rgMainWindowTabBar::SetTabEnabled(int index, bool enable)
{
    // Force next mouseMoveEvent to set mouse cursor even if mouse is hovering over
    // the same tab as the last event call.
    m_mouseHoverLastTabIndex = -1;

    QTabBar::setTabEnabled(index, enable);
}

QSize rgMainWindowTabBar::minimumTabSizeHint(int index) const
{
    if (index == SpacerIndex() || tabText(index).isEmpty())
    {
        return QSize(0, QTabBar::tabSizeHint(index).height());
    }
    else
    {
        return QTabBar::minimumTabSizeHint(index);
    }
}

QSize rgMainWindowTabBar::tabSizeHint(int index) const
{
    const int height = s_TAB_BAR_HEIGHT * ScalingManager::Get().GetScaleFactor();
    const int width = s_TAB_BAR_WIDTH * ScalingManager::Get().GetScaleFactor();

    if (index == SpacerIndex())
    {
        return QSize(CalcSpacerWidth(), height);
    }
    else if (tabText(index).isEmpty())
    {
        int width = 0;
        QWidget* pWidget = tabButton(index, QTabBar::ButtonPosition::LeftSide);
        if (pWidget)
        {
            width += pWidget->width();
        }

        pWidget = tabButton(index, QTabBar::ButtonPosition::RightSide);
        if (pWidget)
        {
            width += pWidget->width();
        }
        return QSize(width, height);
    }
    else
    {
        return QSize(width, height);
    }
}

void rgMainWindowTabBar::paintEvent(QPaintEvent* pEvent)
{
    QTabBar::paintEvent(pEvent);
    if (count() > 0)
    {
        QPainter painter;
        painter.begin(this);
        painter.setPen(tabTextColor(0));
        painter.drawLine(pEvent->rect().topLeft(), pEvent->rect().topRight());
        painter.end();
    }
}

void rgMainWindowTabBar::SetSpacerIndex(int index)
{
    if (index != -1)
    {
        setTabEnabled(index, false);
        setTabText(index, "");
    }
    m_spacerIndex = index;
}

void rgMainWindowTabBar::SetTabTool(int index, QWidget* pWidget)
{
    setTabText(index, "");
    setTabEnabled(index, false);
    setTabButton(index, QTabBar::LeftSide, pWidget);
}

int rgMainWindowTabBar::SpacerIndex() const
{
    return m_spacerIndex;
}

int rgMainWindowTabBar::CalcSpacerWidth() const
{
    int spacerWidth = -1;

    if ( (count() == 0) || (m_spacerIndex < 0) )
    {
        spacerWidth = 0;
    }
    else
    {
        // Figure out the length of the spacer between the last left tab
        // and the right tab.
        QRect leftTabRect;
        int tabMargin = contentsMargins().left();
        if (count() > 1)
        {
            leftTabRect = tabRect(m_spacerIndex - 1);
            tabMargin = ScalingManager::Get().Scaled(4);
        }

        int rightTabsWidth = leftTabRect.width() * (count() - (m_spacerIndex + 1));
        spacerWidth = parentWidget()->width() - (leftTabRect.right() + rightTabsWidth);
        spacerWidth -= (tabMargin * 2);
    }

    return spacerWidth;
}

void rgMainWindowTabBar::SetParentWidget(QWidget* pParent)
{
    m_pParent = pParent;
}

void rgMainWindowTabBar::SetSettingsTab(rgSettingsTab* pSettingsTab)
{
    m_pSettingsTab = pSettingsTab;
}

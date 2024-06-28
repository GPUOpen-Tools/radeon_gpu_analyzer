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
#include "radeon_gpu_analyzer_gui/qt/rg_main_window_tab_bar.h"
#include "radeon_gpu_analyzer_gui/qt/rg_settings_tab.h"
#include "radeon_gpu_analyzer_gui/qt/rg_unsaved_items_dialog.h"
#include "radeon_gpu_analyzer_gui/rg_string_constants.h"
#include "radeon_gpu_analyzer_gui/rg_utils.h"

static const int kTabBarHeight = 47;
static const int kTabBarWidth = 100;

RgMainWindowTabBar::RgMainWindowTabBar(QWidget* parent) :
    QTabBar(parent),
    spacer_index_(-1),
    mouse_hover_last_tab_index_(-1),
    parent_(parent),
    settings_tab_(nullptr)
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

void RgMainWindowTabBar::mouseMoveEvent(QMouseEvent* event)
{
    int tab_index = QTabBar::tabAt(event->pos());

    // Only change mouse cursor if the mouse is hovering over a different tab.
    if (mouse_hover_last_tab_index_ != tab_index)
    {
        if (isTabEnabled(tab_index))
        {
            setCursor(Qt::PointingHandCursor);
        }
        else
        {
            setCursor(Qt::ArrowCursor);
        }
        mouse_hover_last_tab_index_ = tab_index;
    }
}

void RgMainWindowTabBar::mousePressEvent(QMouseEvent* event)
{
    int tab_index = QTabBar::tabAt(event->pos());

    // Check if the user is switching to the "START" tab.
    switch (tab_index)
    {
    case (TabItem::kStart):
    {
        bool user_mode_selection = true;

        assert(settings_tab_ != nullptr);
        if (settings_tab_ != nullptr)
        {
            user_mode_selection = settings_tab_->PromptToSavePendingChanges();
        }

        if (user_mode_selection)
        {
            // User did not cancel the prompt, so let the tab switch occur.
            // Pass the event onto the base class.
            QTabBar::mousePressEvent(event);
        }
        break;
    }
    case (TabItem::kSettings):
    {
        // The user is switching to settings tab, allow it.
        QTabBar::mousePressEvent(event);
        break;
    }
    default:
    {
        // Should not get here.
        assert(false);
    }
    }
}

bool RgMainWindowTabBar::eventFilter(QObject* object, QEvent* event)
{
    bool filtered = false;

    if (event != nullptr)
    {
        if (event->type() == QEvent::KeyPress)
        {
            // Only process the events if the SettingsTab is the current tab.
            if (currentIndex() == TabItem::kSettings)
            {
                if (settings_tab_ != nullptr)
                {
                    QKeyEvent* key_event = static_cast<QKeyEvent*>(event);
                    if (key_event != nullptr)
                    {
                        // If Ctrl+Shift+Tab or Ctrl+Tab is switching tabs, prompt to save pending
                        // settings changes first.
                        // Note: The [Shift] is just a way to choose prev or next direction, but
                        // we only have two tabs, so they both behave the same way.
                        const int key_pressed = key_event->key();
                        if ((key_pressed == Qt::Key_Tab || key_pressed == Qt::Key_Backtab) &&
                            (key_event->modifiers() & Qt::ControlModifier))
                        {
                            // Prompt user to save pending changes.
                            bool user_made_selection = settings_tab_->PromptToSavePendingChanges();
                            if (user_made_selection == false)
                            {
                                // User cancelled the prompt, so the event should be filtered out.
                                filtered = true;
                            }
                        }
                        else if ((key_pressed == Qt::Key_Up || key_pressed == Qt::Key_Down) &&
                            (key_event->modifiers() & Qt::ControlModifier))
                        {
                            settings_tab_->SelectNextListWidgetItem(key_pressed);
                            filtered = true;
                        }
                    }
                }
            }
        }
    }

    // Allow base class to filter the event if needed.
    if (!filtered)
    {
        filtered = QTabBar::eventFilter(object, event);
    }

    return filtered;
}

void RgMainWindowTabBar::SetTabEnabled(int index, bool enable)
{
    // Force next mouseMoveEvent to set mouse cursor even if mouse is hovering over
    // the same tab as the last event call.
    mouse_hover_last_tab_index_ = -1;

    QTabBar::setTabEnabled(index, enable);
}

QSize RgMainWindowTabBar::minimumTabSizeHint(int index) const
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

QSize RgMainWindowTabBar::tabSizeHint(int index) const
{
    const int height       = kTabBarHeight;
    const int width        = kTabBarWidth;

    if (index == SpacerIndex())
    {
        return QSize(CalcSpacerWidth(), height);
    }
    else if (tabText(index).isEmpty())
    {
        int additional_tab_width = 0;
        QWidget* widget = tabButton(index, QTabBar::ButtonPosition::LeftSide);
        if (widget)
        {
            additional_tab_width += widget->width();
        }

        widget = tabButton(index, QTabBar::ButtonPosition::RightSide);
        if (widget)
        {
            additional_tab_width += widget->width();
        }
        return QSize(additional_tab_width, height);
    }
    else
    {
        return QSize(width, height);
    }
}

void RgMainWindowTabBar::paintEvent(QPaintEvent* event)
{
    QTabBar::paintEvent(event);
    if (count() > 0)
    {
        QPainter painter;
        painter.begin(this);
        painter.setPen(tabTextColor(0));
        painter.drawLine(event->rect().topLeft(), event->rect().topRight());
        painter.end();
    }
}

void RgMainWindowTabBar::SetSpacerIndex(int index)
{
    if (index != -1)
    {
        setTabEnabled(index, false);
        setTabText(index, "");
    }
    spacer_index_ = index;
}

void RgMainWindowTabBar::SetTabTool(int index, QWidget* widget)
{
    setTabText(index, "");
    setTabEnabled(index, false);
    setTabButton(index, QTabBar::LeftSide, widget);
}

int RgMainWindowTabBar::SpacerIndex() const
{
    return spacer_index_;
}

int RgMainWindowTabBar::CalcSpacerWidth() const
{
    int spacer_width = -1;

    if ( (count() == 0) || (spacer_index_ < 0) )
    {
        spacer_width = 0;
    }
    else
    {
        // Figure out the length of the spacer between the last left tab
        // and the right tab.
        QRect left_tab_rect;
        int tab_margin = contentsMargins().left();
        if (count() > 1)
        {
            left_tab_rect = tabRect(spacer_index_ - 1);
            tab_margin = 4;
        }

        int right_tabs_width = left_tab_rect.width() * (count() - (spacer_index_ + 1));
        spacer_width = parentWidget()->width() - (left_tab_rect.right() + right_tabs_width);
        spacer_width -= (tab_margin * 2);
    }

    return spacer_width;
}

void RgMainWindowTabBar::SetParentWidget(QWidget* parent)
{
    parent_ = parent;
}

void RgMainWindowTabBar::SetSettingsTab(RgSettingsTab* settings_tab)
{
    settings_tab_ = settings_tab;
}

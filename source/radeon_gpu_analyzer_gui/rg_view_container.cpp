// C++.
#include <cassert>

// Qt.
#include <QAction>
#include <QHBoxLayout>
#include <QPushButton>
#include <QResizeEvent>
#include <QSpacerItem>
#include <QStyle>
#include <QWidget>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_view_container.h"
#include "radeon_gpu_analyzer_gui/rg_definitions.h"
#include "radeon_gpu_analyzer_gui/rg_utils.h"

// Identifying widget names.
static const char* kStrMaximizeButtonName = "viewMaximizeButton";
static const char* kStrTitlebarWidgetName = "viewTitlebar";

// Identifying property names.
static const char* kStrMaximizedStatePropertyName = "isMaximized";
static const char* kStrHoveredStatePropertyName = "viewHovered";
static const char* kStrFocusedStatePropertyName = "viewFocused";

RgViewContainer::RgViewContainer(QWidget* parent) :
    QWidget(parent)
{
    // Default initialize state properties.
    setProperty(kStrMaximizedStatePropertyName, false);
    setProperty(kStrHoveredStatePropertyName, false);
    setProperty(kStrFocusedStatePropertyName, false);

    setFocusPolicy(Qt::StrongFocus);
}

void RgViewContainer::SwitchContainerSize()
{
    SetMaximizedState(!is_in_maximized_state_);
    emit MaximizeButtonClicked();
}

void RgViewContainer::SetIsMaximizable(bool is_enabled)
{
    is_maximizable_ = is_enabled;

    if (maximize_button_ != nullptr)
    {
        // Toggle the visibility of the maximize/restore button in the container's titlebar.
        maximize_button_->setVisible(is_enabled);
    }
}

void RgViewContainer::SetMainWidget(QWidget* widget)
{
    if (widget != nullptr)
    {
        // Reparent the widget.
        widget->setParent(this);

        // Force the same max size as the main widget.
        setMaximumSize(widget->maximumSize());

        // Store pointer to main widget.
        main_widget_ = widget;

        // Use the main widget as the focus proxy for this container.
        setFocusProxy(widget);

        // Extract the title bar if it isn't set.
        if (title_bar_widget_ == nullptr)
        {
            ExtractTitlebar();
        }

        // Extract the maximize button.
        ExtractMaximizeButton();

        // Refresh widget sizes.
        RefreshGeometry();
    }
}

void RgViewContainer::SetTitlebarWidget(QWidget* widget)
{
    if (widget != nullptr)
    {
        // Reparent the widget.
        widget->setParent(this);
    }

    // Store pointer to title bar widget.
    title_bar_widget_ = widget;

    // Indicate the title bar is not embedded.
    is_embedded_titlebar_ = false;

    // Extract the maximize button.
    ExtractMaximizeButton();

    // Refresh widget sizes.
    RefreshGeometry();
}

QWidget* RgViewContainer::GetTitleBar()
{
    return title_bar_widget_;
}

void RgViewContainer::SetMaximizedState(bool is_maximized)
{
    setProperty(kStrMaximizedStatePropertyName, is_maximized);
    is_in_maximized_state_ = is_maximized;
    RgUtils::StyleRepolish(this, true);
}

void RgViewContainer::SetFocusedState(bool is_focused)
{
    setProperty(kStrFocusedStatePropertyName, is_focused);
    RgUtils::StyleRepolish(this, true);
}

bool RgViewContainer::IsInMaximizedState() const
{
    return is_in_maximized_state_;
}

void RgViewContainer::SetHiddenState(bool is_hidden)
{
    is_in_hidden_state_ = is_hidden;
}

bool RgViewContainer::IsMaximizable() const
{
    return is_maximizable_;
}

bool RgViewContainer::IsInHiddenState() const
{
    return is_in_hidden_state_;
}

QWidget* RgViewContainer::GetMainWidget() const
{
    return main_widget_;
}

void RgViewContainer::resizeEvent(QResizeEvent* event)
{
    // Refresh widget sizes.
    RefreshGeometry();

    // Normal event processing.
    QWidget::resizeEvent(event);
}

QSize RgViewContainer::sizeHint() const
{
    QSize ret(0, 0);

    // Use the main widget size hint, if it exists.
    if (main_widget_ != nullptr)
    {
        ret = main_widget_->sizeHint();
    }
    else
    {
        ret = QWidget::sizeHint();
    }

    return ret;
}

QSize RgViewContainer::minimumSizeHint() const
{
    QSize ret(0, 0);

    // Use the main widget minimum size hint, if it exists.
    if (main_widget_ != nullptr)
    {
        ret = main_widget_->minimumSizeHint();
    }
    else
    {
        ret = QWidget::minimumSizeHint();
    }

    return ret;
}

void RgViewContainer::enterEvent(QEnterEvent* event)
{
    Q_UNUSED(event);

    setProperty(kStrHoveredStatePropertyName, true);

    if (title_bar_widget_ != nullptr)
    {
        RgUtils::StyleRepolish(title_bar_widget_, true);
    }
}

void RgViewContainer::leaveEvent(QEvent* event)
{
    Q_UNUSED(event);

    setProperty(kStrHoveredStatePropertyName, false);

    if (title_bar_widget_ != nullptr)
    {
        RgUtils::StyleRepolish(title_bar_widget_, true);
    }
}

void RgViewContainer::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (event != nullptr && event->button() == Qt::LeftButton)
    {
        // A double-click on the top bar should behave like a click on the Resize button.
        MaximizeButtonClicked();
    }
}

void RgViewContainer::mousePressEvent(QMouseEvent* event)
{
    // A click in this view should unfocus the build settings view.
    emit ViewContainerMouseClickEventSignal();

    // Pass the event onto the base class.
    QWidget::mousePressEvent(event);
}

void RgViewContainer::RefreshGeometry()
{
    bool do_overlay_titlebar = false;
    int titlebar_height = 0;

    // If there is a titlebar, position it at the top and always on top of the main widget.
    if (title_bar_widget_ != nullptr && !is_embedded_titlebar_)
    {
        // Get height of titlebar area.
        titlebar_height = title_bar_widget_->minimumSize().height();

        title_bar_widget_->raise();
        title_bar_widget_->setGeometry(0, 0, size().width(), titlebar_height);
    }

    if (main_widget_ != nullptr)
    {
        // If there is no titlebar or if the titlebar is being overlayed, fill the entire
        // container with the main widget. Otherwise leave room for a titlebar.
        if (title_bar_widget_ == nullptr || do_overlay_titlebar || is_embedded_titlebar_)
        {
            main_widget_->setGeometry(0, 0, size().width(), size().height());
        }
        else
        {
            main_widget_->setGeometry(0, titlebar_height, size().width(), size().height() - titlebar_height);
        }
    }
}

void RgViewContainer::ExtractTitlebar()
{
    title_bar_widget_ = nullptr;

    if (main_widget_ != nullptr)
    {
        // Get the titlebar by name.
        title_bar_widget_ = main_widget_->findChild<QWidget*>(kStrTitlebarWidgetName);

        if (title_bar_widget_ != nullptr)
        {
            // Indicate the titlebar is embedded.
            is_embedded_titlebar_ = true;
        }
    }
}

void RgViewContainer::ExtractMaximizeButton()
{
    maximize_button_ = nullptr;

    // Try to get the button from the titlebar.
    if (title_bar_widget_ != nullptr)
    {
        // Get the maximize button by name.
        maximize_button_ = title_bar_widget_->findChild<QAbstractButton*>(kStrMaximizeButtonName);
    }

    // Try to get the button from the main widget.
    if (maximize_button_ == nullptr && main_widget_ != nullptr)
    {
        // Get the maximize button by name.
        maximize_button_ = main_widget_->findChild<QAbstractButton*>(kStrMaximizeButtonName);
    }

    // Connect the button's signals/slots if a valid one is found.
    if (maximize_button_ != nullptr)
    {
        [[maybe_unused]] bool is_connected = connect(maximize_button_, &QAbstractButton::clicked, this, &RgViewContainer::MaximizeButtonClicked);
        assert(is_connected);
    }
}

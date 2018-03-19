// Qt.
#include <QHBoxLayout>
#include <QPushButton>
#include <QResizeEvent>
#include <QSpacerItem>
#include <QStyle>
#include <QWidget>

// Local.
#include <RadeonGPUAnalyzerGUI/include/qt/rgViewContainer.h>
#include <RadeonGPUAnalyzerGUI/include/rgUtils.h>

// Identifying widget names.
static const char* s_MAXIMIZE_BUTTON_NAME = "viewMaximizeButton";
static const char* s_TITLEBAR_WIDGET_NAME = "viewTitlebar";

// Identifying property names.
static const char* s_MAXIMIZED_STATE_PROPERTY_NAME = "isMaximized";
static const char* s_HOVERED_STATE_PROPERTY_NAME = "viewHovered";
static const char* s_FOCUSED_STATE_PROPERTY_NAME = "viewFocused";

rgViewContainer::rgViewContainer(QWidget* pParent) :
    QWidget(pParent)
{
    // Default initialize state properties.
    setProperty(s_MAXIMIZED_STATE_PROPERTY_NAME, false);
    setProperty(s_HOVERED_STATE_PROPERTY_NAME, false);
    setProperty(s_FOCUSED_STATE_PROPERTY_NAME, false);

    setFocusPolicy(Qt::StrongFocus);
}

void rgViewContainer::SetIsMaximizable(bool isEnabled)
{
    m_isMaximizable = isEnabled;

    if (m_pMaximizeButton != nullptr)
    {
        // Toggle the visibility of the maximize/restore button in the container's titlebar.
        m_pMaximizeButton->setVisible(isEnabled);
    }
}

void rgViewContainer::SetMainWidget(QWidget* pWidget)
{
    if (pWidget != nullptr)
    {
        // Reparent the widget.
        pWidget->setParent(this);

        // Force the same max size as the main widget.
        setMaximumSize(pWidget->maximumSize());

        // Store pointer to main widget.
        m_pMainWidget = pWidget;

        // Use the main widget as the focus proxy for this container.
        setFocusProxy(pWidget);

        // Extract the title bar if it isn't set.
        if (m_pTitleBarWidget == nullptr)
        {
            ExtractTitlebar();
        }

        // Extract the maximize button.
        ExtractMaximizeButton();

        // Refresh widget sizes.
        RefreshGeometry();
    }
}

void rgViewContainer::SetTitlebarWidget(QWidget* pWidget)
{
    if (pWidget != nullptr)
    {
        // Reparent the widget.
        pWidget->setParent(this);
    }

    // Store pointer to title bar widget.
    m_pTitleBarWidget = pWidget;

    // Indicate the title bar is not embedded.
    m_isEmbeddedTitlebar = false;

    // Extract the maximize button.
    ExtractMaximizeButton();

    // Refresh widget sizes.
    RefreshGeometry();
}

QWidget* rgViewContainer::GetTitleBar()
{
    return m_pTitleBarWidget;
}

void rgViewContainer::SetMaximizedState(bool isMaximized)
{
    setProperty(s_MAXIMIZED_STATE_PROPERTY_NAME, isMaximized);
    m_isInMaximizedState = isMaximized;
    rgUtils::StyleRepolish(this, true);
}

void rgViewContainer::SetFocusedState(bool isFocused)
{
    setProperty(s_FOCUSED_STATE_PROPERTY_NAME, isFocused);
    rgUtils::StyleRepolish(this, true);
}

bool rgViewContainer::IsInMaximizedState() const
{
    return m_isInMaximizedState;
}

bool rgViewContainer::IsMaximizable() const
{
    return m_isMaximizable;
}

QWidget* rgViewContainer::GetMainWidget() const
{
    return m_pMainWidget;
}

void rgViewContainer::resizeEvent(QResizeEvent* pEvent)
{
    // Refresh widget sizes.
    RefreshGeometry();

    // Normal event processing.
    QWidget::resizeEvent(pEvent);
}

QSize rgViewContainer::sizeHint() const
{
    QSize ret(0, 0);

    // Use the main widget size hint, if it exists.
    if (m_pMainWidget != nullptr)
    {
        ret = m_pMainWidget->sizeHint();
    }
    else
    {
        ret = QWidget::sizeHint();
    }

    return ret;
}

QSize rgViewContainer::minimumSizeHint() const
{
    QSize ret(0, 0);

    // Use the main widget minimum size hint, if it exists.
    if (m_pMainWidget != nullptr)
    {
        ret = m_pMainWidget->minimumSizeHint();
    }
    else
    {
        ret = QWidget::minimumSizeHint();
    }

    return ret;
}

void rgViewContainer::enterEvent(QEvent* pEvent)
{
    setProperty(s_HOVERED_STATE_PROPERTY_NAME, true);

    if (m_pTitleBarWidget != nullptr)
    {
        rgUtils::StyleRepolish(m_pTitleBarWidget, true);
    }
}

void rgViewContainer::leaveEvent(QEvent* pEvent)
{
    setProperty(s_HOVERED_STATE_PROPERTY_NAME, false);

    if (m_pTitleBarWidget != nullptr)
    {
        rgUtils::StyleRepolish(m_pTitleBarWidget, true);
    }
}

void rgViewContainer::mouseDoubleClickEvent(QMouseEvent* pEvent)
{
    if (pEvent != nullptr && pEvent->button() == Qt::LeftButton)
    {
        // A double-click on the top bar should behave like a click on the Resize button.
        MaximizeButtonClicked();
    }
}

void rgViewContainer::mousePressEvent(QMouseEvent* pEvent)
{
    // A click in this view should unfocus the build settings view.
    emit ViewContainerMouseClickEventSignal();

    // Pass the event onto the base class.
    QWidget::mousePressEvent(pEvent);
}

void rgViewContainer::RefreshGeometry()
{
    bool doOverlayTitlebar = false;
    int titlebarHeight = 0;

    // If there is a titlebar, position it at the top and always on top of the main widget.
    if (m_pTitleBarWidget != nullptr && !m_isEmbeddedTitlebar)
    {
        // Get height of titlebar area.
        titlebarHeight = m_pTitleBarWidget->minimumSize().height();

        m_pTitleBarWidget->raise();
        m_pTitleBarWidget->setGeometry(0, 0, size().width(), titlebarHeight);
    }

    if (m_pMainWidget != nullptr)
    {
        // If there is no titlebar or if the titlebar is being overlayed, fill the entire
        // container with the main widget. Otherwise leave room for a titlebar.
        if (m_pTitleBarWidget == nullptr || doOverlayTitlebar || m_isEmbeddedTitlebar)
        {
            m_pMainWidget->setGeometry(0, 0, size().width(), size().height());
        }
        else
        {
            m_pMainWidget->setGeometry(0, titlebarHeight, size().width(), size().height() - titlebarHeight);
        }
    }
}

void rgViewContainer::ExtractTitlebar()
{
    m_pTitleBarWidget = nullptr;

    if (m_pMainWidget != nullptr)
    {
        // Get the titlebar by name.
        m_pTitleBarWidget = m_pMainWidget->findChild<QWidget*>(s_TITLEBAR_WIDGET_NAME);

        if (m_pTitleBarWidget != nullptr)
        {
            // Indicate the titlebar is embedded.
            m_isEmbeddedTitlebar = true;
        }
    }
}

void rgViewContainer::ExtractMaximizeButton()
{
    m_pMaximizeButton = nullptr;

    // Try to get the button from the titlebar.
    if (m_pTitleBarWidget != nullptr)
    {
        // Get the maximize button by name.
        m_pMaximizeButton = m_pTitleBarWidget->findChild<QAbstractButton*>(s_MAXIMIZE_BUTTON_NAME);
    }

    // Try to get the button from the main widget.
    if (m_pMaximizeButton == nullptr && m_pMainWidget != nullptr)
    {
        // Get the maximize button by name.
        m_pMaximizeButton = m_pMainWidget->findChild<QAbstractButton*>(s_MAXIMIZE_BUTTON_NAME);
    }

    // Connect the button's signals/slots if a valid one is found.
    if (m_pMaximizeButton != nullptr)
    {
        connect(m_pMaximizeButton, &QAbstractButton::clicked, this, &rgViewContainer::MaximizeButtonClicked);
    }
}
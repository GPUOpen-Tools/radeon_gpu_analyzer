#pragma once

// Qt.
#include <QWidget>
#include <QSize>

// Forward declarations.
class QVBoxLayout;
class QAbstractButton;

// Multi purpose view container to help with maximization, focus management, and titlebar support.
class rgViewContainer : public QWidget
{
    Q_OBJECT

public:
    rgViewContainer(QWidget* pParent = nullptr);
    virtual ~rgViewContainer() = default;

    // Set if the container is able to be maximized within the parent splitter.
    void SetIsMaximizable(bool isEnabled);

    // Set main widget.
    void SetMainWidget(QWidget* pWidget);

    // Set title bar widget.
    void SetTitlebarWidget(QWidget* pWidget);

    // Get the title bar widget.
    QWidget* GetTitleBar();

    // Set maximization state of the view.
    void SetMaximizedState(bool isMaximized);

    // Set hidden state of the view.
    void SetHiddenState(bool isHidden);

    // Returns true if this container is now in hidden state.
    bool IsInHiddenState() const;

    // Set focused state of the view.
    void SetFocusedState(bool isFocused);

    // Returns true if this container is now in maximized state.
    bool IsInMaximizedState() const;

    // Returns true if the container can be maximized within the parent splitter.
    bool IsMaximizable() const;

    // Returns the main widget inside the container.
    QWidget* GetMainWidget() const;

    // A handler to switch the size of the container.
    void SwitchContainerSize();

protected:
    void resizeEvent(QResizeEvent* pEvent) override;
    virtual QSize sizeHint() const override;
    virtual QSize minimumSizeHint() const override;
    virtual void enterEvent(QEvent* pEvent) override;
    virtual void leaveEvent(QEvent* pEvent) override;
    virtual void mouseDoubleClickEvent(QMouseEvent* pEvent) override;
    virtual void mousePressEvent(QMouseEvent* pEvent) override;

private:
    // Refresh geometry of child widgets.
    void RefreshGeometry();

    // Extract the titlebar from the main widget.
    void ExtractTitlebar();

    // Extract the maximize button, either from the main widget or the titlebar.
    void ExtractMaximizeButton();

    // Main widget.
    QWidget* m_pMainWidget = nullptr;

    // Titlebar widget.
    QWidget* m_pTitleBarWidget = nullptr;

    // Maximization button.
    QAbstractButton* m_pMaximizeButton = nullptr;

    // Whether the titlebar is embedded in the main widget.
    bool m_isEmbeddedTitlebar;

    // A flag to track whether this container is in maximized state or not.
    bool m_isInMaximizedState = false;

    // A flag used to determine if the container can be maximized.
    bool m_isMaximizable = true;

    // A flag used to determine if the container is hidden.
    bool m_isInHiddenState = false;

    // Action to handle Ctrl+R to switch the container size.
    QAction* m_pSwitchContainerSize = nullptr;

signals:
    // Signal emitted when the maximize/minimize button is clicked.
    void MaximizeButtonClicked();

    // Signal emitted when the user clicks on this view container.
    void ViewContainerMouseClickEventSignal();
};

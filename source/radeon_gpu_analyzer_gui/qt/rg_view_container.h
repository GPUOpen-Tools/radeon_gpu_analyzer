#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_VIEW_CONTAINER_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_VIEW_CONTAINER_H_

// Qt.
#include <QWidget>
#include <QSize>

// Forward declarations.
class QVBoxLayout;
class QAbstractButton;

// Multi purpose view container to help with maximization, focus management, and titlebar support.
class RgViewContainer : public QWidget
{
    Q_OBJECT

public:
    RgViewContainer(QWidget* parent = nullptr);
    virtual ~RgViewContainer() = default;

    // Set if the container is able to be maximized within the parent splitter.
    void SetIsMaximizable(bool is_enabled);

    // Set main widget.
    void SetMainWidget(QWidget* widget);

    // Set title bar widget.
    void SetTitlebarWidget(QWidget* widget);

    // Get the title bar widget.
    QWidget* GetTitleBar();

    // Set maximization state of the view.
    void SetMaximizedState(bool is_maximized);

    // Set hidden state of the view.
    void SetHiddenState(bool is_hidden);

    // Returns true if this container is now in hidden state.
    bool IsInHiddenState() const;

    // Set focused state of the view.
    void SetFocusedState(bool is_focused);

    // Returns true if this container is now in maximized state.
    bool IsInMaximizedState() const;

    // Returns true if the container can be maximized within the parent splitter.
    bool IsMaximizable() const;

    // Returns the main widget inside the container.
    QWidget* GetMainWidget() const;

    // A handler to switch the size of the container.
    void SwitchContainerSize();

protected:
    void resizeEvent(QResizeEvent* event) override;
    virtual QSize sizeHint() const override;
    virtual QSize minimumSizeHint() const override;
    virtual void  enterEvent(QEnterEvent* event) override;
    virtual void  leaveEvent(QEvent* event) override;
    virtual void  mouseDoubleClickEvent(QMouseEvent* event) override;
    virtual void  mousePressEvent(QMouseEvent* event) override;

private:
    // Refresh geometry of child widgets.
    void RefreshGeometry();

    // Extract the titlebar from the main widget.
    void ExtractTitlebar();

    // Extract the maximize button, either from the main widget or the titlebar.
    void ExtractMaximizeButton();

    // Main widget.
    QWidget* main_widget_ = nullptr;

    // Titlebar widget.
    QWidget* title_bar_widget_ = nullptr;

    // Maximization button.
    QAbstractButton* maximize_button_ = nullptr;

    // Whether the titlebar is embedded in the main widget.
    bool is_embedded_titlebar_;

    // A flag to track whether this container is in maximized state or not.
    bool is_in_maximized_state_ = false;

    // A flag used to determine if the container can be maximized.
    bool is_maximizable_ = true;

    // A flag used to determine if the container is hidden.
    bool is_in_hidden_state_ = false;

    // Action to handle Ctrl+R to switch the container size.
    QAction* switch_container_size_ = nullptr;

signals:
    // Signal emitted when the maximize/minimize button is clicked.
    void MaximizeButtonClicked();

    // Signal emitted when the user clicks on this view container.
    void ViewContainerMouseClickEventSignal();
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_VIEW_CONTAINER_H_

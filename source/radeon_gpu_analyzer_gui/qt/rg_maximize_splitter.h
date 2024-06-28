#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_MAXIMIZE_SPLITTER_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_MAXIMIZE_SPLITTER_H_

// C++.
#include <vector>

// Qt.
#include <QSplitter>

// Forward declarations.
class QWidget;
class RgViewContainer;

// Splitter class which allows maximization of contained views.
class RgMaximizeSplitter : public QSplitter
{
    Q_OBJECT

public:
    RgMaximizeSplitter(QWidget* parent = nullptr);
    virtual ~RgMaximizeSplitter() = default;

    // Add a maximizable view container to the splitter.
    void AddMaximizableWidget(RgViewContainer* view_container);

    // Get the currently-maximized widget in the splitter.
    QWidget* GetMaximizedWidget() const;

    // Maximize the given widget inside the splitter.
    void MaximizeWidget(QWidget* widget);

    // Restore the splitter to it's default non-maximized state.
    void Restore();

    // Return vector of splitter values.
    std::vector<int> ToStdVector();

private:
    // Vector of all view containers in this splitter.
    std::vector<RgViewContainer*> maximize_containers_;

    // Currently maximized widget. Will be nullptr if no widget is maximized.
    QWidget* maximized_widget_ = nullptr;

signals:
    // A signal emitted when a container in the splitter has been maximized.
    void ViewMaximized();

    // A signal emitted when the maximized container has been restored to normal size.
    void ViewRestored();

    // A signal emitted when the container frame should get the focus.
    void FrameInFocusSignal();

private slots:
    // Handler invoked when the corner button is pressed for a view container in this splitter.
    void HandleCornerButtonClicked();

    // Handler invoked when a view container in this splitter is destroyed.
    void HandleChildDestroyed(QObject* child);
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_MAXIMIZE_SPLITTER_H_

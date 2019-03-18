#pragma once

// C++.
#include <vector>

// Qt.
#include <QSplitter>

// Forward declarations.
class QWidget;
class rgViewContainer;

// Splitter class which allows maximization of contained views.
class rgMaximizeSplitter : public QSplitter
{
    Q_OBJECT

public:
    rgMaximizeSplitter(QWidget* pParent = nullptr);
    virtual ~rgMaximizeSplitter() = default;

    // Add a maximizable view container to the splitter.
    void AddMaximizableWidget(rgViewContainer* pViewContainer);

    // Get the currently-maximized widget in the splitter.
    QWidget* GetMaximizedWidget() const;

    // Maximize the given widget inside the splitter.
    void MaximizeWidget(QWidget* pWidget);

    // Restore the splitter to it's default non-maximized state.
    void Restore();

private:
    // Vector of all view containers in this splitter.
    std::vector<rgViewContainer*> m_maximizeContainers;

    // Currently maximized widget. Will be nullptr if no widget is maximized.
    QWidget* m_pMaximizedWidget = nullptr;

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
    void HandleChildDestroyed(QObject* pChild);
};
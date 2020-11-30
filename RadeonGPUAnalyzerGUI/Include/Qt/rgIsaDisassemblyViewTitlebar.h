#pragma once

// Qt.
#include <QWidget>

// A title bar inserted above the disassembly view.
class rgIsaDisassemblyViewTitlebar : public QWidget
{
    Q_OBJECT

public:
    rgIsaDisassemblyViewTitlebar(QWidget* pParent = nullptr);
    virtual ~rgIsaDisassemblyViewTitlebar() = default;

signals:
    // A signal to indicate that the frame has gained focus.
    void FrameFocusInSignal();

    // A signal to indicate that the title bar was double clicked.
    void ViewTitleBarDoubleClickedSignal();

protected:
    // Re-implement paintEvent.
    virtual void paintEvent(QPaintEvent* pEvent) override;
};
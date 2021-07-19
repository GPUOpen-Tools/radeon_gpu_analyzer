#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_ISA_DISASSEMBLY_VIEW_TITLEBAR_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_ISA_DISASSEMBLY_VIEW_TITLEBAR_H_

// Qt.
#include <QWidget>

// A title bar inserted above the disassembly view.
class RgIsaDisassemblyViewTitlebar : public QWidget
{
    Q_OBJECT

public:
    RgIsaDisassemblyViewTitlebar(QWidget* parent = nullptr);
    virtual ~RgIsaDisassemblyViewTitlebar() = default;

signals:
    // A signal to indicate that the frame has gained focus.
    void FrameFocusInSignal();

    // A signal to indicate that the title bar was double clicked.
    void ViewTitleBarDoubleClickedSignal();

protected:
    // Re-implement paintEvent.
    virtual void paintEvent(QPaintEvent* event) override;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_ISA_DISASSEMBLY_VIEW_TITLEBAR_H_

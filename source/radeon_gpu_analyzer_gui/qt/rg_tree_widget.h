#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_TREE_WIDGET_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_TREE_WIDGET_H_

// Qt.
#include <QTreeWidget>

// Local.
#include "source/radeon_gpu_analyzer_gui/qt/rg_tree_widget.h"

class RgTreeWidget : public QTreeWidget
{
    Q_OBJECT

public:
    RgTreeWidget(QWidget* parent = nullptr);
    virtual ~RgTreeWidget() = default;

protected:
    // Re-implement the focusOutEvent so we can hide this widget.
    virtual void focusOutEvent(QFocusEvent *event) override;

    // Re-implement the mouseMoveEvent method.
    virtual void mouseMoveEvent(QMouseEvent* event) override;

    // Re-implement the leaveEvent method.
    virtual void leaveEvent(QEvent* event) override;
};
#endif //RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_TREE_WIDGET_H_

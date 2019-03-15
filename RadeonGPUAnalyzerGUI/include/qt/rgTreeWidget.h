#pragma once

// Qt.
#include <QTreeWidget>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgTreeWidget.h>

class rgTreeWidget : public QTreeWidget
{
    Q_OBJECT

public:
    rgTreeWidget(QWidget* pParent = nullptr);
    virtual ~rgTreeWidget() = default;

protected:
    // Re-implement the focusOutEvent so we can hide this widget.
    virtual void focusOutEvent(QFocusEvent *event) override;

    // Re-implement the mouseMoveEvent method.
    virtual void mouseMoveEvent(QMouseEvent* pEvent) override;

    // Re-implement the leaveEvent method.
    virtual void leaveEvent(QEvent* pEvent) override;
};
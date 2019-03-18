#pragma once

// Qt.
#include <QListWidget>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgTreeWidget.h>

class rgListWidget : public QListWidget
{
    Q_OBJECT

public:
    rgListWidget(QWidget* pParent = nullptr);
    virtual ~rgListWidget() = default;

protected:
    // Re-implement the mouseMoveEvent method.
    virtual void mouseMoveEvent(QMouseEvent* pEvent) override;
};
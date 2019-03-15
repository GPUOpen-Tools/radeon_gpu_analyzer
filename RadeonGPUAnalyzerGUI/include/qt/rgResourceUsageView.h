#pragma once

// Qt.
#include <QWidget>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/rgDataTypes.h>
#include "ui_rgResourceUsageView.h"

// A view used to show GPU resource usage with respect to the target ASIC and kernel code.
class rgResourceUsageView : public QWidget
{
    Q_OBJECT

protected:
    // Re-implement mouse press event.
    virtual void mousePressEvent(QMouseEvent* pEvent) override;

    // Re-implement focus out event.
    virtual void focusOutEvent(QFocusEvent* pEvent) override;

signals:
    // A signal to indicate that the resource usage view has been clicked.
    void ResourceUsageViewClickedSignal();

    // A signal to indicate that the resource usage view has lost focus.
    void ResourceUsageViewFocusOutEventSignal();

public:
    rgResourceUsageView(QWidget* pParent = nullptr);
    virtual ~rgResourceUsageView() = default;

    // Populate the resource usage view with data.
    void PopulateView(const rgResourceUsageData& resourceUsage);

    // Getter for the resource usage string.
    std::string GetResourceUsageText();

    // Getter for the resource usage string font.
    QFont GetResourceUsageFont();

private:
    Ui::rgResourceUsageView ui;
};
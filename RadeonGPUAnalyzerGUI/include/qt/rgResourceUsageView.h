#pragma once

// Qt.
#include <QWidget>

// Local.
#include <RadeonGPUAnalyzerGUI/include/rgDataTypes.h>
#include "ui_rgResourceUsageView.h"

// A view used to show GPU resource usage with respect to the target ASIC and kernel code.
class rgResourceUsageView : public QWidget
{
    Q_OBJECT

public:
    rgResourceUsageView(QWidget* pParent = nullptr);
    virtual ~rgResourceUsageView() = default;

    // Populate the resource usage view with data.
    void PopulateView(const rgResourceUsageData& resourceUsage);

    // Getter for the resource usage string.
    std::string GetResourceUsageText();

    // Get for the resource usage string font.
    QFont GetResourceUsageFont();

private:
    Ui::rgResourceUsageView ui;
};
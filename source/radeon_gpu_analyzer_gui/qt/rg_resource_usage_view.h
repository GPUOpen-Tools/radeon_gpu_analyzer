//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for the resource usage view.
//=============================================================================
#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_RESOURCE_USAGE_VIEW_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_RESOURCE_USAGE_VIEW_H_

// Qt.
#include <QWidget>

// Local.
#include "source/radeon_gpu_analyzer_gui/rg_data_types.h"
#include "ui_rg_resource_usage_view.h"

// A view used to show GPU resource usage with respect to the target ASIC and kernel code.
class RgResourceUsageView : public QWidget
{
    Q_OBJECT

protected:
    // Re-implement mouse press event.
    virtual void mousePressEvent(QMouseEvent* event) override;

    // Re-implement focus out event.
    virtual void focusOutEvent(QFocusEvent* event) override;

signals:
    // A signal to indicate that the resource usage view has been clicked.
    void ResourceUsageViewClickedSignal();

    // A signal to indicate that the resource usage view has lost focus.
    void ResourceUsageViewFocusOutEventSignal();

public:
    RgResourceUsageView(QWidget* parent = nullptr);
    virtual ~RgResourceUsageView() = default;

    // Populate the resource usage view with data.
    void PopulateView(const RgResourceUsageData& resource_usage);

    // Getter for the resource usage string.
    std::string GetResourceUsageText();

    // Getter for the resource usage string font.
    QFont GetResourceUsageFont();

private:
    Ui::RgResourceUsageView ui_;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_RESOURCE_USAGE_VIEW_H_

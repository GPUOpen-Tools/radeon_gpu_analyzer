#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_STATUS_BAR_OPENCL_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_STATUS_BAR_OPENCL_H_

// Local.
#include "source/radeon_gpu_analyzer_gui/qt/rg_status_bar.h"

class RgStatusBarOpencl : public RgStatusBar
{
    Q_OBJECT

public:
    RgStatusBarOpencl(QStatusBar* status_bar, QWidget* parent = nullptr);
    virtual ~RgStatusBarOpencl() = default;

private:
    // Set style sheets for mode and API push buttons.
    virtual void SetStylesheets(QStatusBar* status_bar) override;

    // The parent widget.
    QWidget* parent_ = nullptr;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_STATUS_BAR_OPENCL_H_

//=============================================================================
/// Copyright (c) 2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for Reveal Kernel Name dialog.
//=============================================================================

#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_REVEAL_KERNEL_NAME_DIALOG_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_REVEAL_KERNEL_NAME_DIALOG_H_

// Qt.
#include <QDialog>

// Local.
#include "ui_rg_reveal_kernel_name_dialog.h"

class RgRevealKernelNameDialog : public QDialog
{
    Q_OBJECT

public:
    RgRevealKernelNameDialog(const QString& kernel_name, QWidget* parent = nullptr);
    virtual ~RgRevealKernelNameDialog() = default;

private:
    // Connect the signals.
    void ConnectSignals();

    // The generated interface view object.
    Ui::RgRevealKernelNameDialog ui_;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_REVEAL_KERNEL_NAME_DIALOG_H_

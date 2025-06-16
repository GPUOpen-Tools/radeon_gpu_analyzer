//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for Go To Line dialog.
//=============================================================================

#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_GO_TO_LINE_DIALOG_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_GO_TO_LINE_DIALOG_H_

// Qt.
#include <QDialog>

// Local.
#include "ui_rg_go_to_line_dialog.h"

class RgGoToLineDialog : public QDialog
{
    Q_OBJECT

public:
    // Enum for dialog result (indicates which button was pressed).
    enum RgGoToLineDialogResult
    {
        // Ensure cancel is same behavior as rejection (clicking X on window).
        kCancel = QDialog::Rejected,
        kOk,
    };

    RgGoToLineDialog(int max_line_number, QWidget* parent = nullptr);
    virtual ~RgGoToLineDialog();
    int GetLineNumber() const;

private slots:
    // Handler for when the user enters a line number.
    void HandleLineNumberEntered(const QString& text);

private:
    // Connect the signals.
    void ConnectSignals();

    // The line number entered by the user
    int line_number_;

    // The max line number that the user can enter
    int max_line_number_;

    // The generated interface view object.
    Ui::RgGoToLineDialog ui_;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_GO_TO_LINE_DIALOG_H_

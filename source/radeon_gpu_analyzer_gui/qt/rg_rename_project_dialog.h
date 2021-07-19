#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_RENAME_PROJECT_DIALOG_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_RENAME_PROJECT_DIALOG_H_

#include "ui_rg_rename_project_dialog.h"

// Forward declarations.
class QDialog;

class RgRenameProjectDialog :
    public QDialog
{
    Q_OBJECT

public:
    RgRenameProjectDialog(std::string& program_name, QWidget* parent);
    ~RgRenameProjectDialog() = default;

    // Override the QDialog's accept function to capture the program name.
    virtual void accept() override;

private slots:
    // Handler for when the user clicks the OK button.
    void HandleOKButtonClicked(bool /* checked */);

    // Handler for when the user clicks the Cancel button.
    void HandleCancelButtonClicked(bool /* checked */);

protected:
    // A reference to the string that would hold the program name.
    std::string& project_name_;

    // Connect signals.
    void ConnectSignals();

    // Set use default project name check box tooltip.
    void SetCheckboxToolTip(const std::string& text);

    // The rename dialog interface.
    Ui::RgRenameProjectDialog ui_;

private:
    // Set the cursor to the specified type.
    void SetCursor(const QCursor& cursor);
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_RENAME_PROJECT_DIALOG_H_


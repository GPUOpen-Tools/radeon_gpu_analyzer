#pragma once
#include "ui_rgRenameProjectDialog.h"

// Forward declarations.
class QDialog;

class rgRenameProjectDialog :
    public QDialog
{
    Q_OBJECT

public:
    rgRenameProjectDialog(std::string& programName, QWidget* pParent);
    ~rgRenameProjectDialog() = default;

    // Override the QDialog's accept function to capture the program name.
    virtual void accept() override;

private slots:
    // Handler for when the user clicks the OK button.
    void HandleOKButtonClicked(bool /* checked */);

    // Handler for when the user clicks the Cancel button.
    void HandleCancelButtonClicked(bool /* checked */);

protected:
    // A reference to the string that would hold the program name.
    std::string& m_projectName;

    // Connect signals.
    void ConnectSignals();

    // Set use default project name check box tooltip.
    void SetCheckboxToolTip(const std::string& text);

    // The rename dialog interface.
    Ui::rgRenameProjectDialog ui;

private:
    // Set the cursor to the specified type.
    void SetCursor(const QCursor& cursor);
};


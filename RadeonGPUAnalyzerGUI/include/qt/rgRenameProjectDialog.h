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
private:
    // A reference to the string that would hold the program name.
    std::string& m_projectName;

    // Set use default project name check box tooltip.
    void SetCheckboxToolTip(const std::string& text);

    Ui::rgRenameProjectDialog ui;
};


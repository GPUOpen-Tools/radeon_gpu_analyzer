#pragma once

// Qt.
#include <QDialog>

// Local.
#include "ui_rgGoToLineDialog.h"

class rgGoToLineDialog : public QDialog
{
    Q_OBJECT

public:
    // Enum for dialog result (indicates which button was pressed).
    enum rgGoToLineDialogResult
    {
        // Ensure cancel is same behavior as rejection (clicking X on window).
        Cancel = QDialog::Rejected,
        Ok,
    };

    rgGoToLineDialog(int maxLineNumber, QWidget* pParent = nullptr);
    virtual ~rgGoToLineDialog();
    int GetLineNumber() const;

private slots:
    // Handler for when the user enters a line number.
    void HandleLineNumberEntered(const QString& text);

private:
    // Connect the signals.
    void ConnectSignals();

    // The line number entered by the user
    int m_lineNumber;

    // The max line number that the user can enter
    int m_maxLineNumber;

    // The generated interface view object.
    Ui::rgGoToLineDialog ui;
};
#pragma once

// Qt.
#include <QDialog>

// Local.
#include "ui_rgAboutDialog.h"

// Forward Declarations for UpdateCheckApi.
namespace UpdateCheck
{
    class ThreadController;
    struct Results;
};

class rgAboutDialog :
    public QDialog
{
    Q_OBJECT

public:
    rgAboutDialog(QWidget* pParent);
    virtual ~rgAboutDialog() = default;

public slots:

    // Handler when the Check For Updates button is clicked.
    void HandleCheckForUpdatesClicked();

    // Handler for when the check for updates is cancelled by the user.
    void HandleCheckForUpdatesCancelled(UpdateCheck::ThreadController* pThread);

    // Handler for when the check for updates returns and the user
    // needs to be notified of the results.
    void HandleCheckForUpdatesCompleted(UpdateCheck::ThreadController* pThread, const UpdateCheck::Results& updateCheckResults);

protected:
    // The generated interface view object.
    Ui::rgAboutDialog ui;

    // A dialog that is displayed while the check for updates is in-progress.
    // Closing this dialog will signal the check for updates to be cancelled.
    // It will close automatically after the check for updates completes.
    QDialog* m_pCheckForUpdatesDialog;

    // The button box that is part of the check for updates dialog.
    // Storing this pointer allows the buttons to be changed as needed.
    QDialogButtonBox* m_pCheckForUpdatesDialogButtonBox;

    // The label on the check for updates pending dialog.
    QLabel* m_pCheckForUpdatesDialogLabel;

    // This class creates and interacts with the background thread that
    // performs the check for updates. We need to store a member variable
    // so that we can cancel the thread if needed. The thread will emit a
    // signal when the check for updates has either been cancelled or after
    // it has completed.
    UpdateCheck::ThreadController* m_pCheckForUpdatesThread;
};


#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_ABOUT_DIALOG_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_ABOUT_DIALOG_H_

// Qt.
#include <QDialog>

// Local.
#include "ui_rg_about_dialog.h"

// Forward Declarations for UpdateCheckApi.
namespace UpdateCheck
{
    class ThreadController;
    struct Results;
};

class RgAboutDialog :
    public QDialog
{
    Q_OBJECT

public:
    RgAboutDialog(QWidget* parent);
    virtual ~RgAboutDialog() = default;

public slots:

    // Handler when the Check For Updates button is clicked.
    void HandleCheckForUpdatesClicked();

    // Handler for when the check for updates is cancelled by the user.
    void HandleCheckForUpdatesCancelled(UpdateCheck::ThreadController* thread);

    // Handler for when the check for updates returns and the user
    // needs to be notified of the results.
    void HandleCheckForUpdatesCompleted(UpdateCheck::ThreadController* thread, const UpdateCheck::Results& update_check_results);

protected:
    // The generated interface view object.
    Ui::RgAboutDialog ui_;

    // A dialog that is displayed while the check for updates is in-progress.
    // Closing this dialog will signal the check for updates to be cancelled.
    // It will close automatically after the check for updates completes.
    QDialog* check_for_updates_dialog_;

    // The button box that is part of the check for updates dialog.
    // Storing this pointer allows the buttons to be changed as needed.
    QDialogButtonBox* check_for_updates_dialog_button_box_;

    // The label on the check for updates pending dialog.
    QLabel* check_for_updates_dialog_label_;

    // This class creates and interacts with the background thread that
    // performs the check for updates. We need to store a member variable
    // so that we can cancel the thread if needed. The thread will emit a
    // signal when the check for updates has either been cancelled or after
    // it has completed.
    UpdateCheck::ThreadController* check_for_updates_thread_;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_ABOUT_DIALOG_H_


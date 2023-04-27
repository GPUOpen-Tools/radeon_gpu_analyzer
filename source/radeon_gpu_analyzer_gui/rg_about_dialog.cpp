// C++.
#include <sstream>
#include <cassert>

// Qt.
#include <QPushButton>

// Infra.
#include "QtCommon/Scaling/ScalingManager.h"
#include "update_check_api/source/update_check_thread.h"
#include "update_check_api/source/update_check_results_dialog.h"

// Local.
#include "radeon_gpu_analyzer_gui/rg_data_types.h"
#include "radeon_gpu_analyzer_gui/qt/rg_about_dialog.h"
#include "radeon_gpu_analyzer_gui/rg_config_manager.h"
#include "radeon_gpu_analyzer_gui/rg_utils.h"

// Shared between CLI and GUI.
#include "source/common/rga_version_info.h"
#include "source/common/rga_shared_utils.h"

RgAboutDialog::RgAboutDialog(QWidget* parent) :
    QDialog(parent),
    check_for_updates_dialog_(nullptr),
    check_for_updates_dialog_button_box_(nullptr),
    check_for_updates_dialog_label_(nullptr),
    check_for_updates_thread_(nullptr)
{
    ui_.setupUi(this);

    // Set the size to fixed.
    QSize size;
    size.setWidth(300 * ScalingManager::Get().GetScaleFactor());
    size.setHeight(125 * ScalingManager::Get().GetScaleFactor());
    setFixedSize(size);

    // Set the background to white.
    RgUtils::SetBackgroundColor(this, Qt::white);

    // Disable the help button in the title bar, and disable resizing of this dialog.
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint | Qt::MSWindowsFixedSizeDialogHint);

    // Get the CLI version info.
    std::shared_ptr<RgCliVersionInfo> cli_version = RgConfigManager::Instance().GetVersionInfo();

    // Build the version strings for the CLI.
    if(cli_version != nullptr)
    {
        std::stringstream cli_version_string;
        cli_version_string << ui_.CliVersionLabel->text().toStdString() << " " <<
            cli_version->version << " (" << cli_version->build_date << ")";
        ui_.CliVersionLabel->setText(cli_version_string.str().c_str());
    }

    // Build the version string for the GUI.
    std::stringstream gui_version_string;

    // Convert the date string.
    std::string gui_data_string(kStrRgaBuildDate);
    bool is_converted = RgaSharedUtils::ConvertDateString(gui_data_string);
    assert(is_converted);

    gui_version_string << ui_.ApplicationVersionLabel->text().toStdString() << " " <<
        kStrRgaVersion << "." << kStrRgaBuildNum << " (" << gui_data_string << ")";
    ui_.ApplicationVersionLabel->setText(gui_version_string.str().c_str());

    // Set the about dialog OK button to have a pointing hand cursor.
    QPushButton* button = ui_.buttonBox->button(QDialogButtonBox::Ok);
    assert(button != nullptr);
    if (button != nullptr)
    {
        button->setCursor(Qt::PointingHandCursor);
    }

    ui_.checkForUpdatesButton->setCursor(Qt::PointingHandCursor);

    // Connect the check for updates push button to the handler.
    bool is_button_connected = connect(ui_.checkForUpdatesButton, &QPushButton::clicked, this, &RgAboutDialog::HandleCheckForUpdatesClicked);
    assert(is_button_connected);
    if (!is_button_connected)
    {
        // Disable the button if the slot cannot be connected.
        ui_.checkForUpdatesButton->setEnabled(false);
    }
}

void RgAboutDialog::HandleCheckForUpdatesClicked()
{
    // Don't allow checking for updates if there is already one in progress.
    if (check_for_updates_thread_ == nullptr)
    {
        std::string build_date_string(kStrRgaBuildDate);
        if (build_date_string == kStrRgaBuildDateDev)
        {
            // Pretend a dev build has no version so that
            // all public versions are reported as being newer.
            check_for_updates_thread_ = new UpdateCheck::ThreadController(this, 0, 0, 0, 0);
        }
        else
        {
            // Create the check for updates thread
            check_for_updates_thread_ = new UpdateCheck::ThreadController(this, RGA_VERSION_MAJOR, RGA_VERSION_MINOR, RGA_BUILD_NUMBER, RGA_VERSION_UPDATE);
        }
        
        // Build dialog to display and allow user to cancel the check if desired.
        if (check_for_updates_dialog_ == nullptr)
        {
            check_for_updates_dialog_ = new QDialog(this);
            check_for_updates_dialog_->setWindowTitle(kStrAppName);
            check_for_updates_dialog_->setWindowFlags(check_for_updates_dialog_->windowFlags() & ~Qt::WindowContextHelpButtonHint | Qt::MSWindowsFixedSizeDialogHint);
            check_for_updates_dialog_->setFixedWidth(250 * ScalingManager::Get().GetScaleFactor());
            check_for_updates_dialog_->setFixedHeight(100 * ScalingManager::Get().GetScaleFactor());

            // Set background to white.
            RgUtils::SetBackgroundColor(check_for_updates_dialog_, Qt::white);

            QVBoxLayout* layout = new QVBoxLayout();
            check_for_updates_dialog_->setLayout(layout);
            check_for_updates_dialog_label_ = new QLabel(kStrUpdatesCheckingForUpdates);
            check_for_updates_dialog_->layout()->addWidget(check_for_updates_dialog_label_);
            check_for_updates_dialog_->layout()->addItem(new QSpacerItem(5, 10, QSizePolicy::Minimum, QSizePolicy::Expanding));

            // Add Cancel button to cancel the check for updates.
            check_for_updates_dialog_button_box_ = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Ok, check_for_updates_dialog_);
            check_for_updates_dialog_button_box_->button(QDialogButtonBox::Cancel)->setCursor(Qt::PointingHandCursor);
            check_for_updates_dialog_button_box_->button(QDialogButtonBox::Ok)->setCursor(Qt::PointingHandCursor);
            check_for_updates_dialog_button_box_->button(QDialogButtonBox::Ok)->setVisible(false);
            check_for_updates_dialog_->layout()->addWidget(check_for_updates_dialog_button_box_);

            // If the cancel button is pressed, signal the dialog to reject, which is similar to closing it.
            bool is_rejected_connected = connect(check_for_updates_dialog_button_box_, &QDialogButtonBox::rejected, check_for_updates_dialog_, &QDialog::reject);
            assert(is_rejected_connected);

            bool is_ok_connected = connect(check_for_updates_dialog_button_box_, &QDialogButtonBox::accepted, check_for_updates_dialog_, &QDialog::accept);
            assert(is_rejected_connected);
        }
        else
        {
            // The dialog already exists, but may have the wrong text and button on it. Fix it up before displaying again.
            check_for_updates_dialog_label_->setText(kStrUpdatesCheckingForUpdates);
            check_for_updates_dialog_button_box_->button(QDialogButtonBox::Cancel)->setVisible(true);
            check_for_updates_dialog_button_box_->button(QDialogButtonBox::Ok)->setVisible(false);
        }

        // Cancel the check for updates if the dialog is closed.
        bool is_cancel_dialog_connected = connect(check_for_updates_dialog_, &QDialog::rejected, check_for_updates_thread_, &UpdateCheck::ThreadController::CancelCheckForUpdates);
        assert(is_cancel_dialog_connected);

        // Get notified when the check for updates has completed or was cancelled.
        bool is_completed_connected = connect(check_for_updates_thread_, &UpdateCheck::ThreadController::CheckForUpdatesComplete, this, &RgAboutDialog::HandleCheckForUpdatesCompleted);
        assert(is_completed_connected);
        bool is_cancelled_connected = connect(check_for_updates_thread_, &UpdateCheck::ThreadController::CheckForUpdatesCancelled, this, &RgAboutDialog::HandleCheckForUpdatesCancelled);
        assert(is_cancelled_connected);

        check_for_updates_thread_->StartCheckForUpdates(kStrRgaUpdatecheckUrl, kStrRgaUpdatecheckAssetName);

        ui_.checkForUpdatesButton->setCursor(Qt::WaitCursor);

        check_for_updates_dialog_->show();
    }
}

void RgAboutDialog::HandleCheckForUpdatesCancelled(UpdateCheck::ThreadController* thread)
{
    // Restore pointing hand cursor.
    ui_.checkForUpdatesButton->setCursor(Qt::PointingHandCursor);

    // Delete the previous thread since it is no longer useful.
    if (check_for_updates_thread_ == thread)
    {
        if (check_for_updates_thread_ != nullptr)
        {
            delete check_for_updates_thread_;
            check_for_updates_thread_ = nullptr;
        }
    }
}

void RgAboutDialog::HandleCheckForUpdatesCompleted(UpdateCheck::ThreadController* thread, const UpdateCheck::Results& update_check_results)
{
    if (update_check_results.was_check_successful && !update_check_results.update_info.is_update_available)
    {
        // Update the existing dialog to report that there are no updates available, and switch to the Ok button.
        check_for_updates_dialog_label_->setText(kStrUpdatesNoUpdatesAvailable);
        check_for_updates_dialog_button_box_->button(QDialogButtonBox::Cancel)->setVisible(false);
        check_for_updates_dialog_button_box_->button(QDialogButtonBox::Ok)->setVisible(true);
        check_for_updates_dialog_->update();
    }
    else
    {
        // Close the pending dialog.
        check_for_updates_dialog_->close();

        UpdateCheckResultsDialog* results_dialog = new UpdateCheckResultsDialog(this);
        if (results_dialog != nullptr)
        {
            // Hide the tag label.
            results_dialog->SetShowTags(false);

            // Change from default title.
            results_dialog->setWindowTitle(kStrUpdatesResultsWindowTitle);

            // Set background to white.
            RgUtils::SetBackgroundColor(results_dialog, Qt::white);

            // Set this dialog to get deleted when it is closed.
            results_dialog->setAttribute(Qt::WA_DeleteOnClose, true);
            results_dialog->setWindowFlags(results_dialog->windowFlags() & ~Qt::WindowContextHelpButtonHint);
            results_dialog->setFixedSize(400 * ScalingManager::Get().GetScaleFactor(),
                300 * ScalingManager::Get().GetScaleFactor());
            QDialogButtonBox* button_box = results_dialog->findChild<QDialogButtonBox*>("button_box_");
            if (button_box != nullptr)
            {
                QPushButton* close_button = button_box->button(QDialogButtonBox::Close);
                if (close_button != nullptr)
                {
                    close_button->setCursor(Qt::PointingHandCursor);
                    QObject::connect(button_box, SIGNAL(rejected()), results_dialog, SLOT(reject()));
                }
            }

            // Set the results.
            results_dialog->SetResults(update_check_results);

            // Display it as a modal dialog.
            results_dialog->exec();
        }
    }

    // Delete the thread so that it no longer exists in the background.
    if (check_for_updates_thread_ == thread)
    {
        if (check_for_updates_thread_ != nullptr)
        {
            delete check_for_updates_thread_;
            check_for_updates_thread_ = nullptr;
        }
    }

    // Restore pointing hand cursor.
    ui_.checkForUpdatesButton->setCursor(Qt::PointingHandCursor);
}

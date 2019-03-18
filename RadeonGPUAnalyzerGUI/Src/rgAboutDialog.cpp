// C++.
#include <sstream>
#include <cassert>

// Qt.
#include <QPushButton>

// Infra.
#include <QtCommon/Scaling/ScalingManager.h>
#include <UpdateCheckAPI/Include/UpdateCheckThread.h>
#include <UpdateCheckAPI/Include/UpdateCheckResultsDialog.h>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/rgDataTypes.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgAboutDialog.h>
#include <RadeonGPUAnalyzerGUI/Include/rgConfigManager.h>
#include <RadeonGPUAnalyzerGUI/Include/rgUtils.h>

// Shared between CLI and GUI.
#include <RadeonGPUAnalyzerGUI/../Utils/Include/rgaVersionInfo.h>
#include <RadeonGPUAnalyzerGUI/../Utils/Include/rgaSharedUtils.h>

rgAboutDialog::rgAboutDialog(QWidget* pParent) :
    QDialog(pParent),
    m_pCheckForUpdatesDialog(nullptr),
    m_pCheckForUpdatesDialogButtonBox(nullptr),
    m_pCheckForUpdatesDialogLabel(nullptr),
    m_pCheckForUpdatesThread(nullptr)
{
    ui.setupUi(this);

    // Set the size to fixed.
    QSize size;
    size.setWidth(300 * ScalingManager::Get().GetScaleFactor());
    size.setHeight(125 * ScalingManager::Get().GetScaleFactor());
    setFixedSize(size);

    // Set the background to white.
    rgUtils::SetBackgroundColor(this, Qt::white);

    // Disable the help button in the title bar, and disable resizing of this dialog.
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint | Qt::MSWindowsFixedSizeDialogHint);

    // Get the CLI version info.
    std::shared_ptr<rgCliVersionInfo> pCliVersion = rgConfigManager::Instance().GetVersionInfo();

    // Build the version strings for the CLI.
    if(pCliVersion != nullptr)
    {
        std::stringstream cliVersionString;
        cliVersionString << ui.CliVersionLabel->text().toStdString() << " " <<
            pCliVersion->m_version << " (" << pCliVersion->m_buildDate << ")";
        ui.CliVersionLabel->setText(cliVersionString.str().c_str());
    }

    // Build the version string for the GUI.
    std::stringstream guiVersionString;

    // Convert the date string.
    std::string guiDateString(STR_RGA_BUILD_DATE);
    bool isConverted = rgaSharedUtils::ConvertDateString(guiDateString);
    assert(isConverted);

    guiVersionString << ui.ApplicationVersionLabel->text().toStdString() << " " <<
        STR_RGA_VERSION << "." << STR_RGA_BUILD_NUM << " (" << guiDateString << ")";
    ui.ApplicationVersionLabel->setText(guiVersionString.str().c_str());

    // Set the about dialog OK button to have a pointing hand cursor.
    QPushButton* pButton = ui.buttonBox->button(QDialogButtonBox::Ok);
    assert(pButton != nullptr);
    if (pButton != nullptr)
    {
        pButton->setCursor(Qt::PointingHandCursor);
    }

    ui.checkForUpdatesButton->setCursor(Qt::PointingHandCursor);

    // Connect the check for updates push button to the handler.
    bool isButtonConnected = connect(ui.checkForUpdatesButton, &QPushButton::clicked, this, &rgAboutDialog::HandleCheckForUpdatesClicked);
    assert(isButtonConnected);
    if (!isButtonConnected)
    {
        // Disable the button if the slot cannot be connected.
        ui.checkForUpdatesButton->setEnabled(false);
    }
}

void rgAboutDialog::HandleCheckForUpdatesClicked()
{
    // Don't allow checking for updates if there is already one in progress.
    if (m_pCheckForUpdatesThread == nullptr)
    {
        // Create the check for updates thread
        m_pCheckForUpdatesThread = new UpdateCheck::ThreadController(this, RGA_VERSION_MAJOR, RGA_VERSION_MINOR, RGA_VERSION_UPDATE, RGA_BUILD_NUMBER);

        // Build dialog to display and allow user to cancel the check if desired.
        if (m_pCheckForUpdatesDialog == nullptr)
        {
            m_pCheckForUpdatesDialog = new QDialog(this);
            m_pCheckForUpdatesDialog->setWindowTitle(STR_APP_NAME);
            m_pCheckForUpdatesDialog->setWindowFlags(m_pCheckForUpdatesDialog->windowFlags() & ~Qt::WindowContextHelpButtonHint | Qt::MSWindowsFixedSizeDialogHint);
            m_pCheckForUpdatesDialog->setFixedWidth(250 * ScalingManager::Get().GetScaleFactor());
            m_pCheckForUpdatesDialog->setFixedHeight(100 * ScalingManager::Get().GetScaleFactor());

            // Set background to white.
            rgUtils::SetBackgroundColor(m_pCheckForUpdatesDialog, Qt::white);

            QVBoxLayout* pLayout = new QVBoxLayout();
            m_pCheckForUpdatesDialog->setLayout(pLayout);
            m_pCheckForUpdatesDialogLabel = new QLabel(STR_UPDATES_CHECKING_FOR_UPDATES);
            m_pCheckForUpdatesDialog->layout()->addWidget(m_pCheckForUpdatesDialogLabel);
            m_pCheckForUpdatesDialog->layout()->addItem(new QSpacerItem(5, 10, QSizePolicy::Minimum, QSizePolicy::Expanding));

            // Add Cancel button to cancel the check for updates.
            m_pCheckForUpdatesDialogButtonBox = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Ok, m_pCheckForUpdatesDialog);
            m_pCheckForUpdatesDialogButtonBox->button(QDialogButtonBox::Cancel)->setCursor(Qt::PointingHandCursor);
            m_pCheckForUpdatesDialogButtonBox->button(QDialogButtonBox::Ok)->setCursor(Qt::PointingHandCursor);
            m_pCheckForUpdatesDialogButtonBox->button(QDialogButtonBox::Ok)->setVisible(false);
            m_pCheckForUpdatesDialog->layout()->addWidget(m_pCheckForUpdatesDialogButtonBox);

            // If the cancel button is pressed, signal the dialog to reject, which is similar to closing it.
            bool isRejectedConnected = connect(m_pCheckForUpdatesDialogButtonBox, &QDialogButtonBox::rejected, m_pCheckForUpdatesDialog, &QDialog::reject);
            assert(isRejectedConnected);

            bool isOkConnected = connect(m_pCheckForUpdatesDialogButtonBox, &QDialogButtonBox::accepted, m_pCheckForUpdatesDialog, &QDialog::accept);
            assert(isRejectedConnected);
        }
        else
        {
            // The dialog already exists, but may have the wrong text and button on it. Fix it up before displaying again.
            m_pCheckForUpdatesDialogLabel->setText(STR_UPDATES_CHECKING_FOR_UPDATES);
            m_pCheckForUpdatesDialogButtonBox->button(QDialogButtonBox::Cancel)->setVisible(true);
            m_pCheckForUpdatesDialogButtonBox->button(QDialogButtonBox::Ok)->setVisible(false);
        }

        // Cancel the check for updates if the dialog is closed.
        bool isCancelDialogConnected = connect(m_pCheckForUpdatesDialog, &QDialog::rejected, m_pCheckForUpdatesThread, &UpdateCheck::ThreadController::CancelCheckForUpdates);
        assert(isCancelDialogConnected);

        // Get notified when the check for updates has completed or was cancelled.
        bool isCompletedConnected = connect(m_pCheckForUpdatesThread, &UpdateCheck::ThreadController::CheckForUpdatesComplete, this, &rgAboutDialog::HandleCheckForUpdatesCompleted);
        assert(isCompletedConnected);
        bool isCancelledConnected = connect(m_pCheckForUpdatesThread, &UpdateCheck::ThreadController::CheckForUpdatesCancelled, this, &rgAboutDialog::HandleCheckForUpdatesCancelled);
        assert(isCancelledConnected);

        m_pCheckForUpdatesThread->StartCheckForUpdates(STR_RGA_UPDATECHECK_URL, STR_RGA_UPDATECHECK_ASSET_NAME);

        ui.checkForUpdatesButton->setCursor(Qt::WaitCursor);

        m_pCheckForUpdatesDialog->show();
    }
}

void rgAboutDialog::HandleCheckForUpdatesCancelled(UpdateCheck::ThreadController* pThread)
{
    // Restore pointing hand cursor.
    ui.checkForUpdatesButton->setCursor(Qt::PointingHandCursor);

    // Delete the previous thread since it is no longer useful.
    if (m_pCheckForUpdatesThread == pThread)
    {
        if (m_pCheckForUpdatesThread != nullptr)
        {
            delete m_pCheckForUpdatesThread;
            m_pCheckForUpdatesThread = nullptr;
        }
    }
}

void rgAboutDialog::HandleCheckForUpdatesCompleted(UpdateCheck::ThreadController* pThread, const UpdateCheck::Results& updateCheckResults)
{
    if (updateCheckResults.wasCheckSuccessful && !updateCheckResults.updateInfo.m_isUpdateAvailable)
    {
        // Update the existing dialog to report that there are no updates available, and switch to the Ok button.
        m_pCheckForUpdatesDialogLabel->setText(STR_UPDATES_NO_UPDATES_AVAILABLE);
        m_pCheckForUpdatesDialogButtonBox->button(QDialogButtonBox::Cancel)->setVisible(false);
        m_pCheckForUpdatesDialogButtonBox->button(QDialogButtonBox::Ok)->setVisible(true);
        m_pCheckForUpdatesDialog->update();
    }
    else
    {
        // Close the pending dialog.
        m_pCheckForUpdatesDialog->close();

        UpdateCheckResultsDialog* pResultsDialog = new UpdateCheckResultsDialog(this);
        if (pResultsDialog != nullptr)
        {
            // Change from default title.
            pResultsDialog->setWindowTitle(STR_UPDATES_RESULTS_WINDOW_TITLE);

            // Set background to white.
            rgUtils::SetBackgroundColor(pResultsDialog, Qt::white);

            // Set this dialog to get deleted when it is closed.
            pResultsDialog->setAttribute(Qt::WA_DeleteOnClose, true);
            pResultsDialog->setWindowFlags(pResultsDialog->windowFlags() & ~Qt::WindowContextHelpButtonHint);
            pResultsDialog->setFixedSize(400 * ScalingManager::Get().GetScaleFactor(),
                300 * ScalingManager::Get().GetScaleFactor());
            QDialogButtonBox* pButtonBox = pResultsDialog->findChild<QDialogButtonBox*>("buttonBox");
            if (pButtonBox != nullptr)
            {
                QPushButton* pCloseButton = pButtonBox->button(QDialogButtonBox::Close);
                if (pCloseButton != nullptr)
                {
                    pCloseButton->setCursor(Qt::PointingHandCursor);
                }
            }

            // Set the results.
            pResultsDialog->SetResults(updateCheckResults);

            // Display it as a modal dialog.
            pResultsDialog->exec();
        }
    }

    // Delete the thread so that it no longer exists in the background.
    if (m_pCheckForUpdatesThread == pThread)
    {
        if (m_pCheckForUpdatesThread != nullptr)
        {
            delete m_pCheckForUpdatesThread;
            m_pCheckForUpdatesThread = nullptr;
        }
    }

    // Restore pointing hand cursor.
    ui.checkForUpdatesButton->setCursor(Qt::PointingHandCursor);
}

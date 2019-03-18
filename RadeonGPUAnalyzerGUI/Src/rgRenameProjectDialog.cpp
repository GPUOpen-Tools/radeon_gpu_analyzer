// C++.
#include <cassert>
#include <string>
#include <sstream>

// Qt.
#include <QDialog>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgRenameProjectDialog.h>
#include <RadeonGPUAnalyzerGUI/Include/rgUtils.h>
#include <RadeonGPUAnalyzerGUI/Include/rgStringConstants.h>

rgRenameProjectDialog::rgRenameProjectDialog(std::string& projectName, QWidget* pParent) :
    m_projectName(projectName), QDialog(pParent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint)
{
    // Setup the UI.
    ui.setupUi(this);

    // Set the background to white.
    rgUtils::SetBackgroundColor(this, Qt::white);

    // Generate a unique project name based on the incoming base name string.
    m_projectName = rgUtils::GenerateDefaultProjectName();
    ui.lineEditProjectName->setText(m_projectName.c_str());
    ui.lineEditProjectName->setFocus();

    // Read the use default project name value and update the check box.
    std::shared_ptr<rgGlobalSettings> pGlobalConfig = rgConfigManager::Instance().GetGlobalConfig();
    if (pGlobalConfig != nullptr)
    {
        ui.projectNameCheckBox->setChecked(pGlobalConfig->m_useDefaultProjectName);
    }

    // Set the tool tip for default project name check box.
    SetCheckboxToolTip(STR_GLOBAL_SETTINGS_CHECKBOX_TOOLTIP);

    // Disable resizing of this dialog.
    setFixedSize(size());

    // Connect signals.
    ConnectSignals();

    // Set the cursor type.
    SetCursor(Qt::PointingHandCursor);
}

void rgRenameProjectDialog::ConnectSignals()
{
    // Connect the OK button.
    bool isConnected = connect(this->ui.okPushButton, &QPushButton::clicked, this, &rgRenameProjectDialog::HandleOKButtonClicked);
    assert(isConnected);

    // Connect the Cancel button.
    isConnected = connect(this->ui.cancelPushButton, &QPushButton::clicked, this, &rgRenameProjectDialog::HandleCancelButtonClicked);
    assert(isConnected);
}

void rgRenameProjectDialog::accept()
{
    // Trim the leading and trailing whitespace characters from the project name.
    std::string projectName = ui.lineEditProjectName->text().toStdString();
    rgUtils::TrimLeadingAndTrailingWhitespace(projectName, projectName);

    // Input validation.
    if (rgUtils::IsValidFileName(projectName))
    {
        // Save the project name.
        m_projectName = projectName;

        // Save the default project name check box value.
        bool checked = ui.projectNameCheckBox->checkState() == Qt::Checked;
        std::shared_ptr<rgGlobalSettings> pGlobalConfig = rgConfigManager::Instance().GetGlobalConfig();
        if (pGlobalConfig != nullptr)
        {
            pGlobalConfig->m_useDefaultProjectName = checked;
            rgConfigManager::Instance().SaveGlobalConfigFile();
        }

        QDialog::accept();
    }
    else
    {
        // Notify the user that the project name is illegal.
        std::stringstream msg;
        msg << STR_ERR_ILLEGAL_PROJECT_NAME <<" \"";
        msg << projectName << "\".";
        rgUtils::ShowErrorMessageBox(msg.str().c_str(), this);
    }
}

void rgRenameProjectDialog::SetCheckboxToolTip(const std::string& text)
{
    ui.projectNameCheckBox->setToolTip(text.c_str());
}

void rgRenameProjectDialog::HandleOKButtonClicked(bool /* checked */)
{
    this->accept();
}

void rgRenameProjectDialog::HandleCancelButtonClicked(bool /* checked */)
{
    this->reject();
}

void rgRenameProjectDialog::SetCursor(const QCursor& cursor)
{
    this->ui.okPushButton->setCursor(cursor);
    this->ui.cancelPushButton->setCursor(cursor);
}

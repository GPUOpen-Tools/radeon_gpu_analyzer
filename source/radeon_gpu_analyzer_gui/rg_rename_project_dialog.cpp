//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for the rename project dialog.
//=============================================================================
// C++.
#include <cassert>
#include <string>
#include <sstream>

// Qt.
#include <QDialog>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_rename_project_dialog.h"
#include "radeon_gpu_analyzer_gui/rg_utils.h"
#include "radeon_gpu_analyzer_gui/rg_string_constants.h"

RgRenameProjectDialog::RgRenameProjectDialog(std::string& project_name, QWidget* parent) :
    project_name_(project_name), QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint)
{
    // Setup the UI.
    ui_.setupUi(this);

    // Generate a unique project name based on the incoming base name string.
    project_name_ = RgUtils::GenerateDefaultProjectName();
    ui_.lineEditProjectName->setText(project_name_.c_str());
    ui_.lineEditProjectName->setFocus();

    // Read the use default project name value and update the check box.
    std::shared_ptr<RgGlobalSettings> global_config = RgConfigManager::Instance().GetGlobalConfig();
    if (global_config != nullptr)
    {
        ui_.projectNameCheckBox->setChecked(global_config->use_default_project_name);
    }

    // Set the tool tip for default project name check box.
    SetCheckboxToolTip(kStrGlobalSettingsCheckboxTooltip);

    // Disable resizing of this dialog.
    setFixedSize(size());

    // Connect signals.
    ConnectSignals();

    // Set the cursor type.
    SetCursor(Qt::PointingHandCursor);
}

void RgRenameProjectDialog::ConnectSignals()
{
    // Connect the OK button.
    bool is_connected = connect(this->ui_.okPushButton, &QPushButton::clicked, this, &RgRenameProjectDialog::HandleOKButtonClicked);
    assert(is_connected);

    // Connect the Cancel button.
    is_connected = connect(this->ui_.cancelPushButton, &QPushButton::clicked, this, &RgRenameProjectDialog::HandleCancelButtonClicked);
    assert(is_connected);
}

void RgRenameProjectDialog::accept()
{
    // Trim the leading and trailing whitespace characters from the project name.
    std::string project_name = ui_.lineEditProjectName->text().toStdString();
    RgUtils::TrimLeadingAndTrailingWhitespace(project_name, project_name);

    // Input validation.
    std::string error_message;
    if (RgUtils::IsValidProjectName(project_name, error_message))
    {
        // Save the project name.
        project_name_ = project_name;

        // Save the default project name check box value.
        bool checked = ui_.projectNameCheckBox->checkState() == Qt::Checked;
        std::shared_ptr<RgGlobalSettings> global_config = RgConfigManager::Instance().GetGlobalConfig();
        if (global_config != nullptr)
        {
            global_config->use_default_project_name = checked;
            RgConfigManager::Instance().SaveGlobalConfigFile();
        }

        QDialog::accept();
    }
    else
    {
        // Notify the user that the project name is illegal.
        std::stringstream msg;
        msg << error_message <<" \"";
        msg << project_name << "\".";
        RgUtils::ShowErrorMessageBox(msg.str().c_str(), this);
    }
}

void RgRenameProjectDialog::SetCheckboxToolTip(const std::string& text)
{
    ui_.projectNameCheckBox->setToolTip(text.c_str());
}

void RgRenameProjectDialog::HandleOKButtonClicked(bool /* checked */)
{
    this->accept();
}

void RgRenameProjectDialog::HandleCancelButtonClicked(bool /* checked */)
{
    this->reject();
}

void RgRenameProjectDialog::SetCursor(const QCursor& cursor)
{
    this->ui_.okPushButton->setCursor(cursor);
    this->ui_.cancelPushButton->setCursor(cursor);
}

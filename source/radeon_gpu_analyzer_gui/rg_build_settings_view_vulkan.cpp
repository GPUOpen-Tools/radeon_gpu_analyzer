// C++.
#include <cassert>
#include <sstream>
#include <algorithm>

// Qt.
#include <QWidget>
#include <QMessageBox>
#include <QFileDialog>

// Infra.
#include "qt_common/utils/restore_cursor_position.h"

#include "common/rga_cli_defs.h"

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_build_settings_view.h"
#include "radeon_gpu_analyzer_gui/qt/rg_build_settings_view_vulkan.h"
#include "radeon_gpu_analyzer_gui/qt/rg_check_box.h"
#include "radeon_gpu_analyzer_gui/qt/rg_include_directories_view.h"
#include "radeon_gpu_analyzer_gui/qt/rg_line_edit.h"
#include "radeon_gpu_analyzer_gui/qt/rg_preprocessor_directives_dialog.h"
#include "radeon_gpu_analyzer_gui/rg_cli_utils.h"
#include "radeon_gpu_analyzer_gui/rg_config_manager.h"
#include "radeon_gpu_analyzer_gui/rg_string_constants.h"
#include "radeon_gpu_analyzer_gui/rg_utils.h"

// Checkbox tool tip stylesheets.
static const char* kStrFilemenuTitleBarTooltipWidth = "min-width: %1px; width: %2px;";
static const char* kStrFilemenuTitleBarTooltipHeight = "min-height: %1px; height: %2px; max-height: %3px;";

// The delimiter to use to join and split a string of option items.
static const char* kOptionsListDelimiter = ";";

RgBuildSettingsViewVulkan::RgBuildSettingsViewVulkan(QWidget* parent, const RgBuildSettingsVulkan& build_settings, bool is_global_settings) :
    RgBuildSettingsView(parent, is_global_settings),
    initial_settings_(build_settings)
{
    // Setup the UI.
    ui_.setupUi(this);

    // Create the include directories editor view.
    include_directories_view_ = new RgIncludeDirectoriesView(kOptionsListDelimiter, this);

    // Create the preprocessor directives editor dialog.
    preprocessor_directives_dialog_ = new RgPreprocessorDirectivesDialog(kOptionsListDelimiter, this);

    // Connect the signals.
    ConnectSignals();

    // Connect focus in/out events for all of the line edits.
    ConnectLineEditFocusEvents();

    // Connect clicked events for check boxes.
    ConnectCheckBoxClickedEvents();

    // Initialize the UI based on the incoming build settings.
    PushToWidgets(build_settings);

    // Initialize the command line preview text.
    UpdateCommandLineText();

    // Set tooltips for general items.
    ui_.targetGPUsLabel->setToolTip(kStrBuildSettingsTargetGpusTooltip);
    ui_.predefinedMacrosLabel->setToolTip(kStrBuildSettingsPredefinedMacrosTooltip);
    ui_.includeDirectoriesLabel->setToolTip(kStrBuildSettingsAdditionalIncDirectoryTooltip);

    // Set tooltip for Vulkan runtime section.
    ui_.vulkanOptionsHeaderLabel->setToolTip(kStrBuildSettingsVulkanRuntimeTooltip);

    // Set tooltip for ICD location item.
    ui_.ICDLocationLabel->setToolTip(kStrCliOptIcdDescription);

    // Set tooltips for alternative compiler (glslang) component.
    // Use the same tooltip for both the title and the item purposely.
    ui_.alternativeCompilerHeaderLabel->setToolTip(kStrBuildSettingsAlternativeCompilerGlslangTooltip);
    ui_.glslangOptionsLabel->setToolTip(kStrCliOptGlslangOptDescriptionA);
    ui_.compilerBinariesLabel->setToolTip(kStrCliDescAlternativeVkBinFolder);

    // Set tooltip for the General section.
    ui_.generalHeaderLabel->setToolTip(kStrBuildSettingsGeneralTooltip);

    // Set the tooltip for the command line section of the build settings.
    ui_.settingsCommandLineHeaderLabel->setToolTip(kStrBuildSettingsCmdLineTooltip);

    // Set the mouse cursor to the pointing hand cursor for various widgets.
    SetCursor();

    // Set the event filter for "All Options" text edit.
    ui_.allOptionsTextEdit->installEventFilter(this);

    // Hide HLSL options for now.
    HideHLSLOptions();
}

void RgBuildSettingsViewVulkan::HideHLSLOptions()
{
    // Hide HLSL options for now.
    ui_.vulkanOptionsHeaderLabel->hide();
    ui_.generateDebugInfoCheckBox->hide();
    ui_.generateDebugInfoLabel->hide();
    ui_.noExplicitBindingsCheckBox->hide();
    ui_.noExplicitBindingsLabel->hide();
    ui_.useHLSLBlockOffsetsCheckBox->hide();
    ui_.useHLSLBlockOffsetsLabel->hide();
    ui_.useHLSLIOMappingCheckBox->hide();
    ui_.useHLSLIOMappingLabel->hide();
}

void RgBuildSettingsViewVulkan::ConnectCheckBoxClickedEvents()
{
   bool is_connected = false;

    is_connected = connect(ui_.generateDebugInfoCheckBox, &RgCheckBox::clicked, this, &RgBuildSettingsViewVulkan::HandleCheckBoxClickedEvent);
    assert(is_connected);

    is_connected = connect(ui_.noExplicitBindingsCheckBox, &RgCheckBox::clicked, this, &RgBuildSettingsViewVulkan::HandleCheckBoxClickedEvent);
    assert(is_connected);

    is_connected = connect(ui_.useHLSLBlockOffsetsCheckBox, &RgCheckBox::clicked, this, &RgBuildSettingsViewVulkan::HandleCheckBoxClickedEvent);
    assert(is_connected);

    is_connected = connect(ui_.useHLSLIOMappingCheckBox, &RgCheckBox::clicked, this, &RgBuildSettingsViewVulkan::HandleCheckBoxClickedEvent);
    assert(is_connected);

    is_connected = connect(ui_.enableValidationLayersCheckBox, &RgCheckBox::clicked, this, &RgBuildSettingsViewVulkan::HandleCheckBoxClickedEvent);
    assert(is_connected);
}

// Make the UI reflect the values in the supplied settings struct.
void RgBuildSettingsViewVulkan::PushToWidgets(const RgBuildSettingsVulkan& settings)
{
    // Disable any signals from the widgets while they're being populated.
    QSignalBlocker signal_blocker_target_gpus_line_edit(ui_.targetGPUsLineEdit);
    QSignalBlocker signal_blocker_predefined_macros_line_edit(ui_.predefinedMacrosLineEdit);
    QSignalBlocker signal_blocker_include_directories_line_edit(ui_.includeDirectoriesLineEdit);
    QSignalBlocker signal_blocker_generate_debug_info_check_box(ui_.generateDebugInfoCheckBox);
    QSignalBlocker signal_blocker_no_explicit_bindings_check_box(ui_.noExplicitBindingsCheckBox);
    QSignalBlocker signal_blocker_use_hlsl_block_offsets_check_box(ui_.useHLSLBlockOffsetsCheckBox);
    QSignalBlocker signal_blocker_use_hlsl_io_mapping_check_box(ui_.useHLSLIOMappingCheckBox);
    QSignalBlocker signal_blocker_enable_validation_layers_check_box(ui_.enableValidationLayersCheckBox);
    QSignalBlocker signal_blocker_icd_location_line_edit(ui_.ICDLocationLineEdit);
    QSignalBlocker signal_blocker_glslang_options_line_edit(ui_.glslangOptionsLineEdit);
    QSignalBlocker signal_blocker_compiler_binaries_line_edit(ui_.compilerBinariesLineEdit);
    QSignalBlocker signal_blocker_output_file_binary_name_line_edit(ui_.outputFileBinaryNameLineEdit);

    // The items below are common build settings for all API types.
    QString target_gpus_list(RgUtils::BuildSemicolonSeparatedStringList(settings.target_gpus).c_str());
    ui_.targetGPUsLineEdit->setText(target_gpus_list);

    QString predefined_macros(RgUtils::BuildSemicolonSeparatedStringList(settings.predefined_macros).c_str());
    ui_.predefinedMacrosLineEdit->setText(predefined_macros);

    QString additional_include_dirs(RgUtils::BuildSemicolonSeparatedStringList(settings.additional_include_directories).c_str());
    ui_.includeDirectoriesLineEdit->setText(additional_include_dirs);

    // Items below are Vulkan-specific build settings only.
    ui_.generateDebugInfoCheckBox->setChecked(settings.is_generate_debug_info_checked);
    ui_.noExplicitBindingsCheckBox->setChecked(settings.is_no_explicit_bindings_checked);
    ui_.useHLSLBlockOffsetsCheckBox->setChecked(settings.is_use_hlsl_block_offsets_checked);
    ui_.useHLSLIOMappingCheckBox->setChecked(settings.is_use_hlsl_io_mapping_checked);
    ui_.enableValidationLayersCheckBox->setChecked(settings.is_enable_validation_layers_checked);
    ui_.ICDLocationLineEdit->setText(QString::fromStdString(settings.icd_location));
    ui_.glslangOptionsLineEdit->setText(QString::fromStdString(settings.glslang_options));
    ui_.compilerBinariesLineEdit->setText(QString::fromStdString(std::get<CompilerFolderType::kBin>(settings.compiler_paths)));

    // Output binary file name.
    ui_.outputFileBinaryNameLineEdit->setText(settings.binary_file_name.c_str());
}

RgBuildSettingsVulkan RgBuildSettingsViewVulkan::PullFromWidgets() const
{
    RgBuildSettingsVulkan settings;

    // Target GPUs.
    std::vector<std::string> target_gpus_vector;
    const std::string& comma_separated_target_gpus = ui_.targetGPUsLineEdit->text().toStdString();
    RgUtils::splitString(comma_separated_target_gpus, RgConfigManager::kRgaListDelimiter, target_gpus_vector);
    settings.target_gpus = target_gpus_vector;

    // Predefined Macros.
    std::vector<std::string> predefined_macros_vector;
    const std::string& comma_separated_predefined_macros = ui_.predefinedMacrosLineEdit->text().toStdString();
    RgUtils::splitString(comma_separated_predefined_macros, RgConfigManager::kRgaListDelimiter, predefined_macros_vector);
    settings.predefined_macros = predefined_macros_vector;

    // Additional Include Directories.
    std::vector<std::string> additional_include_directories_vector;
    const std::string& comma_separated_additional_include_directories = ui_.includeDirectoriesLineEdit->text().toStdString();
    RgUtils::splitString(comma_separated_additional_include_directories, RgConfigManager::kRgaListDelimiter, additional_include_directories_vector);
    settings.additional_include_directories = additional_include_directories_vector;

    // Vulkan-specific settings.
    settings.is_generate_debug_info_checked = ui_.generateDebugInfoCheckBox->isChecked();
    settings.is_no_explicit_bindings_checked = ui_.noExplicitBindingsCheckBox->isChecked();
    settings.is_use_hlsl_block_offsets_checked = ui_.useHLSLBlockOffsetsCheckBox->isChecked();
    settings.is_use_hlsl_io_mapping_checked = ui_.useHLSLIOMappingCheckBox->isChecked();
    settings.is_enable_validation_layers_checked = ui_.enableValidationLayersCheckBox->isChecked();
    settings.icd_location = ui_.ICDLocationLineEdit->text().toStdString();
    settings.glslang_options = ui_.glslangOptionsLineEdit->text().toStdString();
    std::get<CompilerFolderType::kBin>(settings.compiler_paths) = ui_.compilerBinariesLineEdit->text().toStdString();

    // Binary output file name.
    settings.binary_file_name = ui_.outputFileBinaryNameLineEdit->text().toStdString();

    return settings;
}

void RgBuildSettingsViewVulkan::ConnectSignals()
{
    // Add target GPU button.
    bool is_connected = connect(this->ui_.addTargetGPUsButton, &QPushButton::clicked, this, &RgBuildSettingsViewVulkan::HandleAddTargetGpusButtonClick);
    assert(is_connected);

    // Add include directories editor dialog button.
    is_connected = connect(this->ui_.includeDirsBrowseButton, &QPushButton::clicked, this, &RgBuildSettingsViewVulkan::HandleIncludeDirsBrowseButtonClick);
    assert(is_connected);

    // Add preprocessor directives editor dialog button.
    is_connected = connect(this->ui_.predefinedMacrosBrowseButton, &QPushButton::clicked, this, &RgBuildSettingsViewVulkan::HandlePreprocessorDirectivesBrowseButtonClick);
    assert(is_connected);

    // Connect all textboxes within the view.
    is_connected = connect(this->ui_.targetGPUsLineEdit, &QLineEdit::textChanged, this, &RgBuildSettingsViewVulkan::HandleTextEditChanged);
    assert(is_connected);

    // Handle changes to the Predefined Macros setting.
    is_connected = connect(this->ui_.predefinedMacrosLineEdit, &QLineEdit::textChanged, this, &RgBuildSettingsViewVulkan::HandleTextEditChanged);
    assert(is_connected);

    // Handle changes to the Include Directories setting.
    is_connected = connect(this->ui_.includeDirectoriesLineEdit, &QLineEdit::textChanged, this, &RgBuildSettingsViewVulkan::HandleTextEditChanged);
    assert(is_connected);

    // Connect the include directory editor dialog's "OK" button click.
    is_connected = connect(include_directories_view_, &RgIncludeDirectoriesView::OKButtonClicked, this, &RgBuildSettingsViewVulkan::HandleIncludeDirsUpdated);
    assert(is_connected);

    // Connect the preprocessor directives editor dialog's "OK" button click.
    is_connected = connect(preprocessor_directives_dialog_, &RgPreprocessorDirectivesDialog::OKButtonClicked, this, &RgBuildSettingsViewVulkan::HandlePreprocessorDirectivesUpdated);
    assert(is_connected);

    // Handle changes to the Generate Debug Info checkbox.
    is_connected = connect(this->ui_.generateDebugInfoCheckBox, &RgCheckBox::stateChanged, this, &RgBuildSettingsViewVulkan::HandleCheckboxStateChanged);
    assert(is_connected);

    // Handle changes to the No Explicit Bindings checkbox.
    is_connected = connect(this->ui_.noExplicitBindingsCheckBox, &RgCheckBox::stateChanged, this, &RgBuildSettingsViewVulkan::HandleCheckboxStateChanged);
    assert(is_connected);

    // Handle changes to the Use HLSL Block Offsets checkbox.
    is_connected = connect(this->ui_.useHLSLBlockOffsetsCheckBox, &RgCheckBox::stateChanged, this, &RgBuildSettingsViewVulkan::HandleCheckboxStateChanged);
    assert(is_connected);

    // Handle changes to the Use HLSL IO Mapping checkbox.
    is_connected = connect(this->ui_.useHLSLIOMappingCheckBox, &RgCheckBox::stateChanged, this, &RgBuildSettingsViewVulkan::HandleCheckboxStateChanged);
    assert(is_connected);

    // Handle changes to the Enable Validation Layers checkbox.
    is_connected = connect(this->ui_.enableValidationLayersCheckBox, &RgCheckBox::stateChanged, this, &RgBuildSettingsViewVulkan::HandleCheckboxStateChanged);
    assert(is_connected);

    // Handle clicking of ICD location browse button.
    is_connected = connect(this->ui_.ICDLocationBrowseButton, &QPushButton::clicked, this, &RgBuildSettingsViewVulkan::HandleICDLocationBrowseButtonClick);
    assert(is_connected);

    // Handle changes to the ICD location line edit.
    is_connected = connect(this->ui_.ICDLocationLineEdit, &QLineEdit::textChanged, this, &RgBuildSettingsViewVulkan::HandleICDLocationLineEditChanged);
    assert(is_connected);

    // Handle changes to the glslang options line edit.
    is_connected = connect(this->ui_.glslangOptionsLineEdit, &QLineEdit::textChanged, this, &RgBuildSettingsViewVulkan::HandleGlslangOptionsLineEditChanged);
    assert(is_connected);

    // Handle clicking of the Alternative Compiler path browse button.
    is_connected = connect(this->ui_.compilerBrowseButton, &QPushButton::clicked, this, &RgBuildSettingsViewVulkan::HandleAlternativeCompilerBrowseButtonClicked);
    assert(is_connected);

    // Handle changes to the Alternative Compiler path line edit.
    is_connected = connect(this->ui_.compilerBinariesLineEdit, &QLineEdit::textChanged, this, &RgBuildSettingsViewVulkan::HandleAlternativeCompilerLineEditChanged);
    assert(is_connected);

    // Binary Output File name textChanged signal.
    is_connected = connect(this->ui_.outputFileBinaryNameLineEdit, &QLineEdit::textChanged, this, &RgBuildSettingsViewVulkan::HandleOutputBinaryEditBoxChanged);
    assert(is_connected);

    // Binary Output File name editingFinished signal.
    is_connected = connect(this->ui_.outputFileBinaryNameLineEdit, &QLineEdit::editingFinished, this, &RgBuildSettingsViewVulkan::HandleOutputBinaryFileEditingFinished);
    assert(is_connected);
}

void RgBuildSettingsViewVulkan::HandleOutputBinaryFileEditingFinished()
{
    // Verify that the output binary file text is not empty before losing the focus.
    if (this->ui_.outputFileBinaryNameLineEdit->text().trimmed().isEmpty() || !RgUtils::IsValidFileName(ui_.outputFileBinaryNameLineEdit->text().toStdString()))
    {
        // Initialize the binary output file name edit line.
        std::shared_ptr<RgBuildSettings> default_settings = RgConfigManager::Instance().GetUserGlobalBuildSettings(RgProjectAPI::kVulkan);
        auto vk_default_settings = std::dynamic_pointer_cast<RgBuildSettingsVulkan>(default_settings);

        assert(vk_default_settings != nullptr);
        if (vk_default_settings != nullptr)
        {
            this->ui_.outputFileBinaryNameLineEdit->setText(vk_default_settings->binary_file_name.c_str());
        }
        this->ui_.outputFileBinaryNameLineEdit->setFocus();
    }

    // Signal to any listeners that the values in the UI have changed.
    HandlePendingChangesStateChanged(GetHasPendingChanges());

    // Update the command line preview text.
    UpdateCommandLineText();
}

void RgBuildSettingsViewVulkan::HandleOutputBinaryEditBoxChanged(const QString& text)
{
    // Update the tooltip.
    ui_.outputFileBinaryNameLineEdit->setToolTip(text);

    // Restore the cursor to the original position when the text has changed.
    QtCommon::QtUtils::RestoreCursorPosition cursor_position(ui_.outputFileBinaryNameLineEdit);

    // Signal to any listeners that the values in the UI have changed.
    HandlePendingChangesStateChanged(GetHasPendingChanges());

    // Update the command line preview text.
    UpdateCommandLineText();
}

void RgBuildSettingsViewVulkan::HandleICDLocationBrowseButtonClick(bool /* checked */)
{
    QString selected_file = QFileDialog::getOpenFileName(this, tr(kStrIcdLocationDialogSelectFileTitle),
                                                        ui_.ICDLocationLineEdit->text(),
                                                        tr(kStrDialogFilterIcd));
    if (!selected_file.isEmpty())
    {
        ui_.ICDLocationLineEdit->setText(selected_file);

        // Inform the UI of a possible change to the pending state.
        HandlePendingChangesStateChanged(GetHasPendingChanges());

        // Update the command line preview text.
        UpdateCommandLineText();
    }
}

void RgBuildSettingsViewVulkan::HandleICDLocationLineEditChanged(const QString& text)
{
    // Restore the cursor to the original position when the text has changed.
    QtCommon::QtUtils::RestoreCursorPosition cursor_position(ui_.ICDLocationLineEdit);

    // Set the text value.
    ui_.ICDLocationLineEdit->setText(text);

    // Inform the UI of a possible change to the pending state.
    HandlePendingChangesStateChanged(GetHasPendingChanges());

    // Update the command line preview text.
    UpdateCommandLineText();
}

void RgBuildSettingsViewVulkan::HandleGlslangOptionsLineEditChanged(const QString& text)
{
    // Restore the cursor to the original position when the text has changed.
    QtCommon::QtUtils::RestoreCursorPosition cursor_position(ui_.glslangOptionsLineEdit);

    // Set the text value.
    ui_.glslangOptionsLineEdit->setText(text);

    // Inform the UI of a possible change to the pending state.
    HandlePendingChangesStateChanged(GetHasPendingChanges());

    // Update the command line preview text.
    UpdateCommandLineText();
}

void RgBuildSettingsViewVulkan::HandleAlternativeCompilerBrowseButtonClicked()
{
    QFileDialog file_dialog;
    QLineEdit* line_edit = ui_.compilerBinariesLineEdit;
    assert(line_edit != nullptr);
    if (line_edit != nullptr)
    {
        QString currentDir = (line_edit->text().isEmpty() ?
            QString::fromStdString(RgConfigManager::Instance().GetLastSelectedFolder()) : line_edit->text());

        QString selected_directory = QFileDialog::getExistingDirectory(this, tr(kStrIncludeDirDialogSelectDirTitle),
            currentDir, QFileDialog::ShowDirsOnly);
        if (!selected_directory.isEmpty())
        {
            ui_.compilerBinariesLineEdit->setText(selected_directory);

            // Inform the UI of a possible change to the pending state.
            HandlePendingChangesStateChanged(GetHasPendingChanges());

            // Update the command line preview text.
            UpdateCommandLineText();
        }
    }
}

void RgBuildSettingsViewVulkan::HandleAlternativeCompilerLineEditChanged(const QString& text)
{
    // Restore the cursor to the original position when the text has changed.
    QtCommon::QtUtils::RestoreCursorPosition cursor_position(ui_.compilerBinariesLineEdit);

    // Set the text value.
    ui_.compilerBinariesLineEdit->setText(text);

    // Inform the UI of a possible change to the pending state.
    HandlePendingChangesStateChanged(GetHasPendingChanges());

    // Update the command line preview text.
    UpdateCommandLineText();
}

void RgBuildSettingsViewVulkan::ConnectLineEditFocusEvents()
{
    bool is_connected = false;

    is_connected = connect(this->ui_.includeDirectoriesLineEdit, &RgLineEdit::LineEditFocusInEvent, this, &RgBuildSettingsViewVulkan::HandleLineEditFocusInEvent);
    assert(is_connected);

    is_connected = connect(this->ui_.includeDirectoriesLineEdit, &RgLineEdit::LineEditFocusOutEvent, this, &RgBuildSettingsViewVulkan::HandleLineEditFocusOutEvent);
    assert(is_connected);

    is_connected = connect(this->ui_.predefinedMacrosLineEdit, &RgLineEdit::LineEditFocusInEvent, this, &RgBuildSettingsViewVulkan::HandleLineEditFocusInEvent);
    assert(is_connected);

    is_connected = connect(this->ui_.predefinedMacrosLineEdit, &RgLineEdit::LineEditFocusOutEvent, this, &RgBuildSettingsViewVulkan::HandleLineEditFocusOutEvent);
    assert(is_connected);

    is_connected = connect(this->ui_.targetGPUsLineEdit, &RgLineEdit::LineEditFocusInEvent, this, &RgBuildSettingsViewVulkan::HandleLineEditFocusInEvent);
    assert(is_connected);

    is_connected = connect(this->ui_.targetGPUsLineEdit, &RgLineEdit::LineEditFocusOutEvent, this, &RgBuildSettingsViewVulkan::HandleLineEditFocusOutEvent);
    assert(is_connected);

    is_connected = connect(this->ui_.ICDLocationLineEdit, &RgLineEdit::LineEditFocusInEvent, this, &RgBuildSettingsViewVulkan::HandleLineEditFocusInEvent);
    assert(is_connected);

    is_connected = connect(this->ui_.ICDLocationLineEdit, &RgLineEdit::LineEditFocusOutEvent, this, &RgBuildSettingsViewVulkan::HandleLineEditFocusOutEvent);
    assert(is_connected);

    is_connected = connect(this->ui_.glslangOptionsLineEdit, &RgLineEdit::LineEditFocusInEvent, this, &RgBuildSettingsViewVulkan::HandleLineEditFocusInEvent);
    assert(is_connected);

    is_connected = connect(this->ui_.glslangOptionsLineEdit, &RgLineEdit::LineEditFocusOutEvent, this, &RgBuildSettingsViewVulkan::HandleLineEditFocusOutEvent);
    assert(is_connected);

    is_connected = connect(this->ui_.compilerBinariesLineEdit, &RgLineEdit::LineEditFocusInEvent, this, &RgBuildSettingsViewVulkan::HandleLineEditFocusInEvent);
    assert(is_connected);

    is_connected = connect(this->ui_.compilerBinariesLineEdit, &RgLineEdit::LineEditFocusOutEvent, this, &RgBuildSettingsViewVulkan::HandleLineEditFocusOutEvent);
    assert(is_connected);

    is_connected = connect(this->ui_.addTargetGPUsButton, &RgBrowseButton::BrowseButtonFocusInEvent, this, &RgBuildSettingsViewVulkan::HandleBrowseButtonFocusInEvent);
    assert(is_connected);

    is_connected = connect(this->ui_.addTargetGPUsButton, &RgBrowseButton::BrowseButtonFocusOutEvent, this, &RgBuildSettingsViewVulkan::HandleBrowseButtonFocusOutEvent);
    assert(is_connected);

    is_connected = connect(this->ui_.predefinedMacrosBrowseButton, &RgBrowseButton::BrowseButtonFocusInEvent, this, &RgBuildSettingsViewVulkan::HandleBrowseButtonFocusInEvent);
    assert(is_connected);

    is_connected = connect(this->ui_.predefinedMacrosBrowseButton, &RgBrowseButton::BrowseButtonFocusOutEvent, this, &RgBuildSettingsViewVulkan::HandleBrowseButtonFocusOutEvent);
    assert(is_connected);

    is_connected = connect(this->ui_.includeDirsBrowseButton, &RgBrowseButton::BrowseButtonFocusInEvent, this, &RgBuildSettingsViewVulkan::HandleBrowseButtonFocusInEvent);
    assert(is_connected);

    is_connected = connect(this->ui_.includeDirsBrowseButton, &RgBrowseButton::BrowseButtonFocusOutEvent, this, &RgBuildSettingsViewVulkan::HandleBrowseButtonFocusOutEvent);
    assert(is_connected);

    is_connected = connect(this->ui_.ICDLocationBrowseButton, &RgBrowseButton::BrowseButtonFocusInEvent, this, &RgBuildSettingsViewVulkan::HandleBrowseButtonFocusInEvent);
    assert(is_connected);

    is_connected = connect(this->ui_.ICDLocationBrowseButton, &RgBrowseButton::BrowseButtonFocusOutEvent, this, &RgBuildSettingsViewVulkan::HandleBrowseButtonFocusOutEvent);
    assert(is_connected);

    is_connected = connect(this->ui_.compilerBrowseButton, &RgBrowseButton::BrowseButtonFocusInEvent, this, &RgBuildSettingsViewVulkan::HandleBrowseButtonFocusInEvent);
    assert(is_connected);

    is_connected = connect(this->ui_.compilerBrowseButton, &RgBrowseButton::BrowseButtonFocusOutEvent, this, &RgBuildSettingsViewVulkan::HandleBrowseButtonFocusOutEvent);
    assert(is_connected);

    is_connected = connect(this->ui_.enableValidationLayersCheckBox, &RgCheckBox::CheckBoxFocusInEvent, this, &RgBuildSettingsViewVulkan::HandleCheckBoxFocusInEvent);
    assert(is_connected);

    is_connected = connect(this->ui_.enableValidationLayersCheckBox, &RgCheckBox::CheckBoxFocusOutEvent, this, &RgBuildSettingsViewVulkan::HandleCheckBoxFocusOutEvent);
    assert(is_connected);
}

void RgBuildSettingsViewVulkan::HandleAddTargetGpusButtonClick()
{
    // Set the frame border color to red.
    emit SetFrameBorderRedSignal();

    // Trim out any spaces within the target GPUs string.
    QString selected_gpus = this->ui_.targetGPUsLineEdit->text();

    // Create a new Target GPU Selection dialog instance.
    target_gpus_dialog_ = new RgTargetGpusDialog(selected_gpus, this);

    // Center the dialog on the view (registering with the scaling manager
    // shifts it out of the center so we need to manually center it).
    RgUtils::CenterOnWidget(target_gpus_dialog_, this);

    // Present the dialog to the user.
    target_gpus_dialog_->setModal(true);
    int dialog_result = target_gpus_dialog_->exec();

    // If the user clicked "OK", extract the Checked items into a comma-separated string,
    // and put the string in the Target GPUs textbox.
    if (dialog_result == 1)
    {
        // After the dialog is hidden, extract the list of families selected by the user.
        std::vector<std::string> selected_families = target_gpus_dialog_->GetSelectedCapabilityGroups();

        // Remove gfx notation if needed.
        std::transform(selected_families.begin(), selected_families.end(), selected_families.begin(),
            [&](std::string& family)
        {
            return RgUtils::RemoveGfxNotation(family);
        });

        // Create a comma-separated list of GPU families that the user selected.
        std::stringstream family_list_string;
        size_t num_families = selected_families.size();
        for (size_t family_index = 0; family_index < num_families; ++family_index)
        {
            // Append the family name to the string.
            family_list_string << selected_families[family_index];

            // Append a comma between each family name, until the last one.
            if ((family_index + 1) < num_families)
            {
                family_list_string << RgConfigManager::kRgaListDelimiter;
            }
        }

        // Set the target GPUs text.
        ui_.targetGPUsLineEdit->setText(family_list_string.str().c_str());

        // Inform the UI of a possible change to the pending state.
        HandlePendingChangesStateChanged(GetHasPendingChanges());
    }
}

void RgBuildSettingsViewVulkan::HandleTextEditChanged()
{
    // Determine which control's text has been updated.
    QLineEdit* line_edit = static_cast<QLineEdit*>(QObject::sender());
    assert(line_edit != nullptr);
    if (line_edit != nullptr)
    {
        // Inform the UI of a possible change to the pending state.
        HandlePendingChangesStateChanged(GetHasPendingChanges());

        // Update the command line preview text.
        UpdateCommandLineText();
    }
}

std::string RgBuildSettingsViewVulkan::GetTitleString()
{
    std::stringstream title_string;

    // Build the title string.
    if (is_global_settings_)
    {
        // For the global settings.
        title_string << kStrBuildSettingsDefaultTitle;
        title_string << " ";
        title_string << kStrApiNameVulkan;
        title_string << " ";
    }
    else
    {
        // For project-specific settings.
        title_string << kStrBuildSettingsProjectTitle;
        title_string << " ";
    }
    title_string << kStrMenuBuildSettingsLower;

    return title_string.str();
}

const std::string RgBuildSettingsViewVulkan::GetTitleTooltipString() const
{
    std::stringstream tootltip_string;

    if (is_global_settings_)
    {
        tootltip_string << kStrBuildSettingsGlobalTooltipA;
        tootltip_string << kStrApiNameVulkan;
        tootltip_string << kStrBuildSettingsGlobalTooltipB;
    }
    else
    {
        tootltip_string << kStrBuildSettingsProjectTooltipA;
        tootltip_string << kStrApiNameVulkan;
        tootltip_string << kStrBuildSettingsProjectTooltipB;
    }

    return tootltip_string.str();
}

void RgBuildSettingsViewVulkan::UpdateCommandLineText()
{
    RgBuildSettingsVulkan api_build_setting = PullFromWidgets();

    // Generate a command line string from the build settings structure.
    std::string build_settings;
    bool ret = RgCliUtils::GenerateVulkanBuildSettingsString(api_build_setting, build_settings);
    assert(ret);
    if (ret)
    {
        ui_.allOptionsTextEdit->setPlainText(build_settings.c_str());
    }
}

void RgBuildSettingsViewVulkan::HandlePendingChangesStateChanged(bool has_pending_changes)
{
    // Let the base class determine if there is a need to signal listeners
    // about the pending changes state.
    RgBuildSettingsView::SetHasPendingChanges(has_pending_changes);
}

bool RgBuildSettingsViewVulkan::IsTargetGpusStringValid(std::vector<std::string>& errors) const
{
    bool is_valid = true;

    // The target GPUs string must be non-empty.
    std::string targetGpusString = ui_.targetGPUsLineEdit->text().toStdString();
    if (targetGpusString.empty())
    {
        // The Target GPUs field is invalid since it is empty.
        errors.push_back(kStrErrTargetGpusCannotBeEmpty);
        is_valid = false;
    }
    else
    {
        // Split the comma-separated GPUs string.
        std::vector<std::string> target_gpus_vector;
        RgUtils::splitString(targetGpusString, RgConfigManager::kRgaListDelimiter, target_gpus_vector);

        // Use the Config Manager to verify that the specified GPUs are valid.
        std::vector<std::string> invalid_gpus;
        RgConfigManager& config_manager = RgConfigManager::Instance();
        for (const std::string& target_gpu_family_name : target_gpus_vector)
        {
            std::string trimmed_name;
            RgUtils::TrimLeadingAndTrailingWhitespace(target_gpu_family_name, trimmed_name);

            // Is the given target GPU family name supported?
            if (!config_manager.IsGpuFamilySupported(trimmed_name))
            {
                // Add the GPU to a list of invalid names if it's not supported.
                invalid_gpus.push_back(trimmed_name);
            }
        }

        if (!invalid_gpus.empty())
        {
            // Build an error string indicating the invalid GPUs.
            std::stringstream error_stream;
            error_stream << kStrErrInvalidGpusSpecified;

            int num_invalid = static_cast<int>(invalid_gpus.size());
            for (int gpu_index = 0; gpu_index < num_invalid; ++gpu_index)
            {
                error_stream << invalid_gpus[gpu_index];

                if (gpu_index != (num_invalid - 1))
                {
                    error_stream << ", ";
                }
            }

            // Add an error indicating that the GPU name is invalid.
            errors.push_back(error_stream.str());
            is_valid = false;
        }
    }

    return is_valid;
}

bool RgBuildSettingsViewVulkan::ValidatePendingSettings()
{
    std::vector<std::string> error_fields;
    bool is_valid = IsTargetGpusStringValid(error_fields);
    if (!is_valid)
    {
        std::stringstream error_stream;
        error_stream << kStrErrInvalidPendingSetting;
        error_stream << std::endl;

        // Loop through all errors and append them to the stream.
        for (const std::string error : error_fields)
        {
            error_stream << error;
            error_stream << std::endl;
        }

        // Display an error message box to the user telling them what to fix.
        RgUtils::ShowErrorMessageBox(error_stream.str().c_str(), this);
    }

    return is_valid;
}

void RgBuildSettingsViewVulkan::SetCursor()
{
    // Set the cursor for buttons to pointing hand cursor.
    ui_.addTargetGPUsButton->setCursor(Qt::PointingHandCursor);
    ui_.includeDirsBrowseButton->setCursor(Qt::PointingHandCursor);
    ui_.predefinedMacrosBrowseButton->setCursor(Qt::PointingHandCursor);
    ui_.ICDLocationBrowseButton->setCursor(Qt::PointingHandCursor);
    ui_.compilerBrowseButton->setCursor(Qt::PointingHandCursor);

    // Set the cursor for check boxes to pointing hand cursor.
    ui_.generateDebugInfoCheckBox->setCursor(Qt::PointingHandCursor);
    ui_.noExplicitBindingsCheckBox->setCursor(Qt::PointingHandCursor);
    ui_.useHLSLBlockOffsetsCheckBox->setCursor(Qt::PointingHandCursor);
    ui_.useHLSLIOMappingCheckBox->setCursor(Qt::PointingHandCursor);
    ui_.enableValidationLayersCheckBox->setCursor(Qt::PointingHandCursor);
}

void RgBuildSettingsViewVulkan::HandleIncludeDirsBrowseButtonClick()
{
    // Set the frame border color to red.
    emit SetFrameBorderRedSignal();

    // Position the window in the middle of the screen.
    include_directories_view_->setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, include_directories_view_->size(), QGuiApplication::primaryScreen()->availableGeometry()));

    // Set the current include dirs.
    include_directories_view_->SetListItems(ui_.includeDirectoriesLineEdit->text());

    // Show the window.
    include_directories_view_->exec();
}

void RgBuildSettingsViewVulkan::HandleIncludeDirsUpdated(QStringList includeDirs)
{
    QString include_dirs_text;

    // Create a delimiter-separated string.
    if (!includeDirs.isEmpty())
    {
        include_dirs_text = includeDirs.join(kOptionsListDelimiter);
    }

    // Update the text box.
    ui_.includeDirectoriesLineEdit->setText(include_dirs_text);

    // Inform the rest of the UI that the settings have been changed.
    HandlePendingChangesStateChanged(GetHasPendingChanges());
}

void RgBuildSettingsViewVulkan::HandlePreprocessorDirectivesBrowseButtonClick()
{
    // Set the frame border color to red.
    emit SetFrameBorderRedSignal();

    // Position the window in the middle of the screen.
    preprocessor_directives_dialog_->setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, preprocessor_directives_dialog_->size(), QGuiApplication::primaryScreen()->availableGeometry()));

    // Set the current preprocessor directives in the dialog.
    preprocessor_directives_dialog_->SetListItems(ui_.predefinedMacrosLineEdit->text());

    // Show the dialog.
    preprocessor_directives_dialog_->exec();
}

void RgBuildSettingsViewVulkan::HandlePreprocessorDirectivesUpdated(QStringList preprocessor_directives)
{
    QString preprocessor_directives_text;

    // Create a delimiter-separated string.
    if (!preprocessor_directives.isEmpty())
    {
        preprocessor_directives_text = preprocessor_directives.join(kOptionsListDelimiter);
    }

    // Update the text box.
    ui_.predefinedMacrosLineEdit->setText(preprocessor_directives_text);

    // Inform the rest of the UI that the settings have been changed.
    HandlePendingChangesStateChanged(GetHasPendingChanges());
}

void RgBuildSettingsViewVulkan::HandleLineEditFocusInEvent()
{
    emit SetFrameBorderRedSignal();
}

void RgBuildSettingsViewVulkan::HandleLineEditFocusOutEvent()
{
    emit SetFrameBorderBlackSignal();
}

void RgBuildSettingsViewVulkan::HandleBrowseButtonFocusInEvent()
{
    emit SetFrameBorderRedSignal();
}

void RgBuildSettingsViewVulkan::HandleBrowseButtonFocusOutEvent()
{
    emit SetFrameBorderBlackSignal();
}

void RgBuildSettingsViewVulkan::HandleCheckBoxFocusInEvent()
{
    emit SetFrameBorderRedSignal();
}

void RgBuildSettingsViewVulkan::HandleCheckBoxFocusOutEvent()
{
    emit SetFrameBorderBlackSignal();
}

void RgBuildSettingsViewVulkan::HandleCheckBoxClickedEvent()
{
    emit SetFrameBorderRedSignal();
}

void RgBuildSettingsViewVulkan::HandleCheckboxStateChanged()
{
    // Make sure it was a checkbox that caused this state change (just to be sure).
    QCheckBox* check_box = static_cast<QCheckBox*>(QObject::sender());
    assert(check_box != nullptr);
    if (check_box != nullptr)
    {
        // Inform the UI of a possible change to the pending state.
        HandlePendingChangesStateChanged(GetHasPendingChanges());

        // Update the command line preview text.
        UpdateCommandLineText();
    }
}

bool RgBuildSettingsViewVulkan::GetHasPendingChanges() const
{
    RgBuildSettingsVulkan current_settings = PullFromWidgets();

    bool has_changes = !initial_settings_.HasSameSettings(current_settings);

    return has_changes;
}

bool RgBuildSettingsViewVulkan::RevertPendingChanges()
{
    PushToWidgets(initial_settings_);

    // Make sure the rest of the UI knows that the settings don't need to be saved.
    HandlePendingChangesStateChanged(false);

    return false;
}

void RgBuildSettingsViewVulkan::RestoreDefaultSettings()
{
    bool is_restored = false;

    if (is_global_settings_)
    {
        // If this is for the global settings, then restore to the hard-coded defaults.

        // Get the hardcoded default build settings.
        std::shared_ptr<RgBuildSettings> default_build_settings = RgConfigManager::GetDefaultBuildSettings(RgProjectAPI::kVulkan);

        std::shared_ptr<RgBuildSettingsVulkan> api_build_settings = std::dynamic_pointer_cast<RgBuildSettingsVulkan>(default_build_settings);
        assert(api_build_settings != nullptr);
        if (api_build_settings != nullptr)
        {
            // Reset our initial settings back to the defaults.
            initial_settings_ = *api_build_settings;

            // Update the UI to reflect the new initial settings.
            PushToWidgets(initial_settings_);

            // Update the ConfigManager to use the new settings.
            RgConfigManager::Instance().SetApiBuildSettings(kStrApiNameVulkan, &initial_settings_);

            // Save the settings file.
            is_restored = RgConfigManager::Instance().SaveGlobalConfigFile();

            // Inform the rest of the UI that the settings have been changed.
            HandlePendingChangesStateChanged(GetHasPendingChanges());
        }
    }
    else
    {
        // This view is showing project-specific settings, so restore back to the stored settings in the project.
        std::shared_ptr<RgBuildSettings> default_settings = RgConfigManager::Instance().GetUserGlobalBuildSettings(RgProjectAPI::kVulkan);
        auto vk_default_settings = std::dynamic_pointer_cast<RgBuildSettingsVulkan>(default_settings);

        initial_settings_ = *vk_default_settings;

        PushToWidgets(initial_settings_);

        // Inform the rest of the UI that the settings have been changed.
        HandlePendingChangesStateChanged(GetHasPendingChanges());

        // Let the RgBuildView know that the build settings have been updated.
        emit ProjectBuildSettingsSaved(vk_default_settings);
        is_restored = true;
    }

    // Show an error dialog if the settings failed to be reset.
    if (!is_restored)
    {
        RgUtils::ShowErrorMessageBox(kStrErrCannotRestoreDefaultSettings, this);
    }
}

bool RgBuildSettingsViewVulkan::SaveSettings()
{
    bool can_be_saved = ValidatePendingSettings();
    if (can_be_saved)
    {
        // Reset the initial settings to match what the UI shows.
        initial_settings_ = PullFromWidgets();

        if (is_global_settings_)
        {
            // Update the config manager to use these new settings.
            RgConfigManager& config_manager = RgConfigManager::Instance();
            config_manager.SetApiBuildSettings(kStrApiNameVulkan, &initial_settings_);

            // Save the global config settings.
            can_be_saved = config_manager.SaveGlobalConfigFile();
        }
        else
        {
            // Save the project settings.
            std::shared_ptr<RgBuildSettingsVulkan> tmp_ptr = std::make_shared<RgBuildSettingsVulkan>(initial_settings_);
            emit ProjectBuildSettingsSaved(tmp_ptr);
        }

        if (can_be_saved)
        {
            // Make sure the rest of the UI knows that the settings have been saved.
            HandlePendingChangesStateChanged(false);
        }
    }

    // Set focus to target GPUs browse button.
    ui_.addTargetGPUsButton->setFocus();

    return can_be_saved;
}

void RgBuildSettingsViewVulkan::mousePressEvent(QMouseEvent* event)
{
    Q_UNUSED(event);

    emit SetFrameBorderRedSignal();
}

bool RgBuildSettingsViewVulkan::eventFilter(QObject* object, QEvent* event)
{
    // Intercept events for "All Options" widget.
    if (event != nullptr)
    {
        if (event->type() == QEvent::FocusIn)
        {
            HandleLineEditFocusInEvent();
        }
        else if (event->type() == QEvent::FocusOut)
        {
            HandleLineEditFocusOutEvent();
        }
    }

    // Continue default processing.
    return QObject::eventFilter(object, event);
}

void RgBuildSettingsViewVulkan::SetInitialWidgetFocus()
{
    ui_.addTargetGPUsButton->setFocus();
}

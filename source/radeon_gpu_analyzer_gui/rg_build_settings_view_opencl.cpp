// C++.
#include <cassert>
#include <sstream>

// Qt.
#include <QWidget>
#include <QMessageBox>
#include <QFileDialog>

// Infra.
#include "qt_common/utils/restore_cursor_position.h"
#include "common/rga_cli_defs.h"

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_build_settings_view.h"
#include "radeon_gpu_analyzer_gui/qt/rg_build_settings_view_opencl.h"
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

RgBuildSettingsViewOpencl::RgBuildSettingsViewOpencl(QWidget* parent, const RgBuildSettingsOpencl& build_settings, bool is_global_settings) :
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

    // Connect focus in/out events for all of the checkboxes.
    ConnectCheckBoxClickedEvents();

    // Connect focus in event for the combobox.
    ConnectComboboxClickEvent();

    // Initialize the UI based on the incoming build settings.
    PushToWidgets(build_settings);

    // Initialize the command line preview text.
    UpdateCommandLineText();

    // Set tooltips for general items.
    ui_.targetGPUsLabel->setToolTip(kStrBuildSettingsTargetGpusTooltip);
    ui_.predefinedMacrosLabel->setToolTip(kStrBuildSettingsPredefinedMacrosTooltip);
    ui_.includeDirectoriesLabel->setToolTip(kStrBuildSettingsAdditionalIncDirectoryTooltip);

    // Set tooltips for alternative compiler components.
    ui_.alternativeCompilerLabel->setToolTip(kStrBuildSettingsAlternativeCompilerTooltipGeneric);
    ui_.compilerBinariesLabel->setToolTip(kStrCliDescAlternativeLightningCompilerBinFolder);
    ui_.compilerIncludesLabel->setToolTip(kStrCliDescAlternativeLightningCompilerIncFolder);
    ui_.compilerLibrariesLabel->setToolTip(kStrCliDescAlternativeLightningCompilerLibFolder);

    // Set tooltip for the optimization level item.
    ui_.optimizationLevelLabel->setToolTip(kStrBuildSettingsOptimizationLevel);

    // Set tooltips for the Settings command line section.
    ui_.settingsCommandLineHeaderLabel->setToolTip(kStrBuildSettingsSettingsCmdlineTooltip);

    // Set tooltips for the Additional options line section.
    ui_.additionalOptionsHeaderLabel->setToolTip(kStrBuildSettingsClangOptionsTooltip);

    // Set the tooltip for the General section of the build settings.
    ui_.generalHeaderLabel->setToolTip(kStrBuildSettingsGeneralTooltip);

    // Set the tooltip for the command line section of the build settings.
    ui_.settingsCommandLineHeaderLabel->setToolTip(kStrBuildSettingsCmdLineTooltip);

    // Set the mouse cursor to the pointing hand cursor for various widgets.
    SetCursor();

    // Set the geometry of tool tip boxes.
    SetToolTipGeometry();

    // Set the event filter for "Additional Options" and "All Options" text edits.
    ui_.additionalOptionsTextEdit->installEventFilter(this);
    ui_.allOptionsTextEdit->installEventFilter(this);

    // Disable tabbing in additional options text edit, so tabbing moves focus to the next widget.
    this->ui_.additionalOptionsTextEdit->setTabChangesFocus(true);
}

void RgBuildSettingsViewOpencl::PushToWidgets(const RgBuildSettingsOpencl& build_settings)
{
    // The items below are common build settings for all API types.
    QString target_gpus_list(RgUtils::BuildSemicolonSeparatedStringList(build_settings.target_gpus).c_str());
    ui_.targetGPUsLineEdit->setText(target_gpus_list);

    QString predefinedMacros(RgUtils::BuildSemicolonSeparatedStringList(build_settings.predefined_macros).c_str());
    ui_.predefinedMacrosLineEdit->setText(predefinedMacros);

    QString additionalIncludeDirs(RgUtils::BuildSemicolonSeparatedStringList(build_settings.additional_include_directories).c_str());
    ui_.includeDirectoriesLineEdit->setText(additionalIncludeDirs);

    // Items below are OpenCL-specific build settings only.
    ui_.optimizationLevelComboBox->setCurrentText(build_settings.optimization_level_.c_str());
    ui_.doubleAsSingleCheckBox->setChecked(build_settings.is_treat_double_as_single_);
    ui_.flushDenormalizedCheckBox->setChecked(build_settings.is_denorms_as_zeros_);
    ui_.strictAliasingCheckBox->setChecked(build_settings.is_strict_aliasing_);
    ui_.enableMADCheckBox->setChecked(build_settings.is_enable_mad_);
    ui_.ignoreZeroSignednessCheckBox->setChecked(build_settings.is_ignore_zero_signedness_);
    ui_.allowUnsafeOptimizationsCheckBox->setChecked(build_settings.is_unsafe_optimizations_);
    ui_.assumeNoNanNorInfiniteCheckBox->setChecked(build_settings.is_no_nan_nor_infinite_);
    ui_.aggressiveMathOptimizationsCheckBox->setChecked(build_settings.is_aggressive_math_optimizations_);
    ui_.correctlyRoundSinglePrecisionCheckBox->setChecked(build_settings.is_correctly_round_div_sqrt_);
    ui_.additionalOptionsTextEdit->document()->setPlainText(build_settings.additional_options.c_str());
    ui_.compilerBinariesLineEdit->setText(std::get<CompilerFolderType::kBin>(build_settings.compiler_paths).c_str());
    ui_.compilerIncludesLineEdit->setText(std::get<CompilerFolderType::kInclude>(build_settings.compiler_paths).c_str());
    ui_.compilerLibrariesLineEdit->setText(std::get<CompilerFolderType::kLib>(build_settings.compiler_paths).c_str());
}

RgBuildSettingsOpencl RgBuildSettingsViewOpencl::PullFromWidgets() const
{
    RgBuildSettingsOpencl settings;

    // Target GPUs
    std::vector<std::string> target_gpus_vector;
    const std::string& comma_separated_target_gpus = ui_.targetGPUsLineEdit->text().toStdString();
    RgUtils::splitString(comma_separated_target_gpus, RgConfigManager::kRgaListDelimiter, target_gpus_vector);
    settings.target_gpus = target_gpus_vector;

    // Predefined Macros
    std::vector<std::string> predefined_macros_vector;
    const std::string& commaSeparatedPredefinedMacros = ui_.predefinedMacrosLineEdit->text().toStdString();
    RgUtils::splitString(commaSeparatedPredefinedMacros, RgConfigManager::kRgaListDelimiter, predefined_macros_vector);
    settings.predefined_macros = predefined_macros_vector;

    // Additional Include Directories
    std::vector<std::string> additional_include_directories_vector;
    const std::string& comma_separated_additional_include_directories = ui_.includeDirectoriesLineEdit->text().toStdString();
    RgUtils::splitString(comma_separated_additional_include_directories, RgConfigManager::kRgaListDelimiter, additional_include_directories_vector);
    settings.additional_include_directories = additional_include_directories_vector;

    // AdditionalOptions:
    settings.additional_options = ui_.additionalOptionsTextEdit->document()->toPlainText().toStdString();

    // AlternativeCompilerBinDir:
    std::get<CompilerFolderType::kBin>(settings.compiler_paths) = ui_.compilerBinariesLineEdit->text().toStdString();

    // AlternativeCompilerIncDir:
    std::get<CompilerFolderType::kInclude>(settings.compiler_paths) = ui_.compilerIncludesLineEdit->text().toStdString();

    // AlternativeCompilerLibDir:
    std::get<CompilerFolderType::kLib>(settings.compiler_paths) = ui_.compilerLibrariesLineEdit->text().toStdString();

    // OpenCL Specific Settings

    // OptimizationLevel:
    settings.optimization_level_ = ui_.optimizationLevelComboBox->currentText().toStdString();

    // TreatDoubleFloatingPointAsSingle:
    settings.is_treat_double_as_single_ = ui_.doubleAsSingleCheckBox->isChecked();

    // FlushDenormalizedFloatsAsZero:
    settings.is_denorms_as_zeros_ = ui_.flushDenormalizedCheckBox->isChecked();

    // AssumeStrictAliasingRules:
    settings.is_strict_aliasing_ = ui_.strictAliasingCheckBox->isChecked();

    // EnableMADInstructions:
    settings.is_enable_mad_ = ui_.enableMADCheckBox->isChecked();

    //IgnoreSignednessOfZeros:
    settings.is_ignore_zero_signedness_ = ui_.ignoreZeroSignednessCheckBox->isChecked();

    // AllowUnsafeOptimizations:
    settings.is_unsafe_optimizations_ = ui_.allowUnsafeOptimizationsCheckBox->isChecked();

    // AssumeNoNaNNorInfinite:
    settings.is_no_nan_nor_infinite_ = ui_.assumeNoNanNorInfiniteCheckBox->isChecked();

    // AgressiveMathOptimizations:
    settings.is_aggressive_math_optimizations_ = ui_.aggressiveMathOptimizationsCheckBox->isChecked();

    // CorrectlyRoundSingleDivideAndSqrt:
    settings.is_correctly_round_div_sqrt_ = ui_.correctlyRoundSinglePrecisionCheckBox->isChecked();

    return settings;
}

void RgBuildSettingsViewOpencl::ConnectSignals()
{
    // Add target GPU button.
    bool is_connected = connect(this->ui_.addTargetGPUsButton, &QPushButton::clicked, this, &RgBuildSettingsViewOpencl::HandleAddTargetGpusButtonClick);
    assert(is_connected);

    // Add include directories editor dialog button.
    is_connected = connect(this->ui_.includeDirsBrowseButton, &QPushButton::clicked, this, &RgBuildSettingsViewOpencl::HandleIncludeDirsBrowseButtonClick);
    assert(is_connected);

    // Add preprocessor directives editor dialog button.
    is_connected = connect(this->ui_.predefinedMacrosBrowseButton, &QPushButton::clicked, this, &RgBuildSettingsViewOpencl::HandlePreprocessorDirectivesBrowseButtonClick);
    assert(is_connected);

    // Connect all textboxes within the view.
    is_connected = connect(this->ui_.targetGPUsLineEdit, &QLineEdit::textChanged, this, &RgBuildSettingsViewOpencl::HandleTextEditChanged);
    assert(is_connected);

    is_connected = connect(this->ui_.predefinedMacrosLineEdit, &QLineEdit::textChanged, this, &RgBuildSettingsViewOpencl::HandleTextEditChanged);
    assert(is_connected);

    is_connected = connect(this->ui_.includeDirectoriesLineEdit, &QLineEdit::textChanged, this, &RgBuildSettingsViewOpencl::HandleTextEditChanged);
    assert(is_connected);

    is_connected = connect(this->ui_.optimizationLevelComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &RgBuildSettingsViewOpencl::HandleComboboxIndexChanged);
    assert(is_connected);

    is_connected = connect(this->ui_.doubleAsSingleCheckBox, &RgCheckBox::stateChanged, this, &RgBuildSettingsViewOpencl::HandleCheckboxStateChanged);
    assert(is_connected);

    is_connected = connect(this->ui_.flushDenormalizedCheckBox, &RgCheckBox::stateChanged, this, &RgBuildSettingsViewOpencl::HandleCheckboxStateChanged);
    assert(is_connected);

    is_connected = connect(this->ui_.strictAliasingCheckBox, &RgCheckBox::stateChanged, this, &RgBuildSettingsViewOpencl::HandleCheckboxStateChanged);
    assert(is_connected);

    is_connected = connect(this->ui_.enableMADCheckBox, &RgCheckBox::stateChanged, this, &RgBuildSettingsViewOpencl::HandleCheckboxStateChanged);
    assert(is_connected);

    is_connected = connect(this->ui_.ignoreZeroSignednessCheckBox, &RgCheckBox::stateChanged, this, &RgBuildSettingsViewOpencl::HandleCheckboxStateChanged);
    assert(is_connected);

    is_connected = connect(this->ui_.allowUnsafeOptimizationsCheckBox, &RgCheckBox::stateChanged, this, &RgBuildSettingsViewOpencl::HandleCheckboxStateChanged);
    assert(is_connected);

    is_connected = connect(this->ui_.assumeNoNanNorInfiniteCheckBox, &RgCheckBox::stateChanged, this, &RgBuildSettingsViewOpencl::HandleCheckboxStateChanged);
    assert(is_connected);

    is_connected = connect(this->ui_.aggressiveMathOptimizationsCheckBox, &RgCheckBox::stateChanged, this, &RgBuildSettingsViewOpencl::HandleCheckboxStateChanged);
    assert(is_connected);

    is_connected = connect(this->ui_.correctlyRoundSinglePrecisionCheckBox, &RgCheckBox::stateChanged, this, &RgBuildSettingsViewOpencl::HandleCheckboxStateChanged);
    assert(is_connected);

    // Connect "Additional Options" text box.
    is_connected = connect(this->ui_.additionalOptionsTextEdit, &QPlainTextEdit::textChanged, this, &RgBuildSettingsViewOpencl::HandleAdditionalOptionsTextChanged);
    assert(is_connected);

    // Connect Alternative Compiler folders browse buttons.
    is_connected = connect(this->ui_.compilerBinariesBrowseButton, &QPushButton::clicked, this, [&]() { HandleCompilerFolderBrowseButtonClick(CompilerFolderType::kBin); });
    assert(is_connected);
    is_connected = connect(this->ui_.compilerIncludesBrowseButton, &QPushButton::clicked, this, [&]() { HandleCompilerFolderBrowseButtonClick(CompilerFolderType::kInclude); });
    assert(is_connected);
    is_connected = connect(this->ui_.compilerLibrariesBrowseButton, &QPushButton::clicked, this, [&]() { HandleCompilerFolderBrowseButtonClick(CompilerFolderType::kLib); });
    assert(is_connected);

    // Connect Alternative Compiler folders text edits.
    is_connected = connect(this->ui_.compilerBinariesLineEdit, &QLineEdit::textChanged, this, [&]() { HandleCompilerFolderEditChanged(CompilerFolderType::kBin); });
    assert(is_connected);
    is_connected = connect(this->ui_.compilerIncludesLineEdit, &QLineEdit::textChanged, this, [&]() { HandleCompilerFolderEditChanged(CompilerFolderType::kInclude); });
    assert(is_connected);
    is_connected = connect(this->ui_.compilerLibrariesLineEdit, &QLineEdit::textChanged, this, [&]() { HandleCompilerFolderEditChanged(CompilerFolderType::kLib); });
    assert(is_connected);

    // Connect the include directory editor dialog's "OK" button click.
    is_connected = connect(include_directories_view_, &RgIncludeDirectoriesView::OKButtonClicked, this, &RgBuildSettingsViewOpencl::HandleIncludeDirsUpdated);
    assert(is_connected);

    // Connect the preprocessor directives editor dialog's "OK" button click.
    is_connected = connect(preprocessor_directives_dialog_, &RgPreprocessorDirectivesDialog::OKButtonClicked, this, &RgBuildSettingsViewOpencl::HandlePreprocessorDirectivesUpdated);
    assert(is_connected);
}

void RgBuildSettingsViewOpencl::ConnectLineEditFocusEvents()
{
    bool is_connected = false;

    is_connected = connect(this->ui_.includeDirectoriesLineEdit, &RgLineEdit::LineEditFocusInEvent, this, &RgBuildSettingsViewOpencl::HandleLineEditFocusInEvent);
    assert(is_connected);

    is_connected = connect(this->ui_.includeDirectoriesLineEdit, &RgLineEdit::LineEditFocusOutEvent, this, &RgBuildSettingsViewOpencl::HandleLineEditFocusOutEvent);
    assert(is_connected);

    is_connected = connect(this->ui_.predefinedMacrosLineEdit, &RgLineEdit::LineEditFocusInEvent, this, &RgBuildSettingsViewOpencl::HandleLineEditFocusInEvent);
    assert(is_connected);

    is_connected = connect(this->ui_.predefinedMacrosLineEdit, &RgLineEdit::LineEditFocusOutEvent, this, &RgBuildSettingsViewOpencl::HandleLineEditFocusOutEvent);
    assert(is_connected);

    is_connected = connect(this->ui_.targetGPUsLineEdit, &RgLineEdit::LineEditFocusInEvent, this, &RgBuildSettingsViewOpencl::HandleLineEditFocusInEvent);
    assert(is_connected);

    is_connected = connect(this->ui_.targetGPUsLineEdit, &RgLineEdit::LineEditFocusOutEvent, this, &RgBuildSettingsViewOpencl::HandleLineEditFocusOutEvent);
    assert(is_connected);

    is_connected = connect(this->ui_.compilerBinariesLineEdit, &RgLineEdit::LineEditFocusInEvent, this, &RgBuildSettingsViewOpencl::HandleLineEditFocusInEvent);
    assert(is_connected);

    is_connected = connect(this->ui_.compilerBinariesLineEdit, &RgLineEdit::LineEditFocusOutEvent, this, &RgBuildSettingsViewOpencl::HandleLineEditFocusOutEvent);
    assert(is_connected);

    is_connected = connect(this->ui_.compilerIncludesLineEdit, &RgLineEdit::LineEditFocusInEvent, this, &RgBuildSettingsViewOpencl::HandleLineEditFocusInEvent);
    assert(is_connected);

    is_connected = connect(this->ui_.compilerIncludesLineEdit, &RgLineEdit::LineEditFocusOutEvent, this, &RgBuildSettingsViewOpencl::HandleLineEditFocusOutEvent);
    assert(is_connected);

    is_connected = connect(this->ui_.compilerLibrariesLineEdit, &RgLineEdit::LineEditFocusInEvent, this, &RgBuildSettingsViewOpencl::HandleLineEditFocusInEvent);
    assert(is_connected);

    is_connected = connect(this->ui_.compilerLibrariesLineEdit, &RgLineEdit::LineEditFocusOutEvent, this, &RgBuildSettingsViewOpencl::HandleLineEditFocusOutEvent);
    assert(is_connected);
}

void RgBuildSettingsViewOpencl::ConnectCheckBoxClickedEvents()
{
    bool is_connected = false;

    is_connected = connect(this->ui_.aggressiveMathOptimizationsCheckBox, &RgCheckBox::clicked, this, &RgBuildSettingsViewOpencl::HandleCheckBoxClickedEvent);
    assert(is_connected);

    is_connected = connect(this->ui_.allowUnsafeOptimizationsCheckBox, &RgCheckBox::clicked, this, &RgBuildSettingsViewOpencl::HandleCheckBoxClickedEvent);
    assert(is_connected);

    is_connected = connect(this->ui_.assumeNoNanNorInfiniteCheckBox, &RgCheckBox::clicked, this, &RgBuildSettingsViewOpencl::HandleCheckBoxClickedEvent);
    assert(is_connected);

    is_connected = connect(this->ui_.correctlyRoundSinglePrecisionCheckBox, &RgCheckBox::clicked, this, &RgBuildSettingsViewOpencl::HandleCheckBoxClickedEvent);
    assert(is_connected);

    is_connected = connect(this->ui_.doubleAsSingleCheckBox, &RgCheckBox::clicked, this, &RgBuildSettingsViewOpencl::HandleCheckBoxClickedEvent);
    assert(is_connected);

    is_connected = connect(this->ui_.enableMADCheckBox, &RgCheckBox::clicked, this, &RgBuildSettingsViewOpencl::HandleCheckBoxClickedEvent);
    assert(is_connected);

    is_connected = connect(this->ui_.flushDenormalizedCheckBox, &RgCheckBox::clicked, this, &RgBuildSettingsViewOpencl::HandleCheckBoxClickedEvent);
    assert(is_connected);

    is_connected = connect(this->ui_.ignoreZeroSignednessCheckBox, &RgCheckBox::clicked, this, &RgBuildSettingsViewOpencl::HandleCheckBoxClickedEvent);
    assert(is_connected);

    is_connected = connect(this->ui_.strictAliasingCheckBox, &RgCheckBox::clicked, this, &RgBuildSettingsViewOpencl::HandleCheckBoxClickedEvent);
    assert(is_connected);
}

void RgBuildSettingsViewOpencl::ConnectComboboxClickEvent()
{
    [[maybe_unused]] bool is_connected =
        connect(this->ui_.optimizationLevelComboBox, &RgComboBox::ComboBoxFocusInEvent, this, &RgBuildSettingsViewOpencl::HandleComboBoxFocusInEvent);
    assert(is_connected);
}

bool RgBuildSettingsViewOpencl::eventFilter(QObject* object, QEvent* event)
{
    // Intercept events for "Additional "Options" widget.
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

void RgBuildSettingsViewOpencl::HandleAddTargetGpusButtonClick()
{
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

void RgBuildSettingsViewOpencl::HandleTextEditChanged()
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

void RgBuildSettingsViewOpencl::HandleComboboxIndexChanged(int index)
{
    Q_UNUSED(index);

    // Determine which control's text has been updated.
    QComboBox *combo_box = static_cast<QComboBox*>(QObject::sender());
    assert(combo_box != nullptr);
    if (combo_box != nullptr)
    {
        // Inform the UI of a possible change to the pending state.
        HandlePendingChangesStateChanged(GetHasPendingChanges());

        // Update the command line preview text.
        UpdateCommandLineText();
    }
}

void RgBuildSettingsViewOpencl::HandleCheckboxStateChanged()
{
    // Determine which control's text has been updated.
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

std::string RgBuildSettingsViewOpencl::GetTitleString()
{
    std::stringstream title_string;

    // Build the title string.
    if (is_global_settings_)
    {
        // For the global settings.
        title_string << kStrBuildSettingsDefaultTitle;
        title_string << " ";
        title_string << kStrApiNameOpencl;
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

const std::string RgBuildSettingsViewOpencl::GetTitleTooltipString() const
{
    std::stringstream tooltip_string;

    if (is_global_settings_)
    {
        tooltip_string << kStrBuildSettingsGlobalTooltipA;
        tooltip_string << kStrApiNameOpencl;
        tooltip_string << kStrBuildSettingsGlobalTooltipB;
    }
    else
    {
        tooltip_string << kStrBuildSettingsProjectTooltipA;
        tooltip_string << kStrApiNameOpencl;
        tooltip_string << kStrBuildSettingsProjectTooltipB;
    }

    return tooltip_string.str();
}

void RgBuildSettingsViewOpencl::HandlePendingChangesStateChanged(bool has_pending_changes)
{
    // Let the base class determine if there is a need to signal listeners
    // about the pending changes state.
    RgBuildSettingsView::SetHasPendingChanges(has_pending_changes);
}

void RgBuildSettingsViewOpencl::UpdateCommandLineText()
{
    RgBuildSettingsOpencl api_build_setting = PullFromWidgets();

    // Generate a command line string from the build settings structure.
    std::string build_settings;
    bool ret = RgCliUtils::GenerateOpenclBuildSettingsString(api_build_setting, build_settings, false);
    assert(ret);
    if (ret)
    {
        ui_.allOptionsTextEdit->setPlainText(build_settings.c_str());
    }
}

bool RgBuildSettingsViewOpencl::IsTargetGpusStringValid(std::vector<std::string>& errors) const
{
    bool is_valid = true;

    // The target GPUs string must be non-empty.
    std::string target_gpus_string = ui_.targetGPUsLineEdit->text().toStdString();
    if (target_gpus_string.empty())
    {
        // The Target GPUs field is invalid since it is empty.
        errors.push_back(kStrErrTargetGpusCannotBeEmpty);
        is_valid = false;
    }
    else
    {
        // Split the comma-separated GPUs string.
        std::vector<std::string> target_gpus_vector;
        RgUtils::splitString(target_gpus_string, RgConfigManager::kRgaListDelimiter, target_gpus_vector);

        // Use the Config Manager to verify that the specified GPUs are valid.
        std::vector<std::string> invalid_gpus;
        RgConfigManager& config_manager = RgConfigManager::Instance();
        for (const std::string& targetGpuFamilyName : target_gpus_vector)
        {
            std::string trimmedName;
            RgUtils::TrimLeadingAndTrailingWhitespace(targetGpuFamilyName, trimmedName);

            // Is the given target GPU family name supported?
            if (!config_manager.IsGpuFamilySupported(trimmedName))
            {
                // Add the GPU to a list of invalid names if it's not supported.
                invalid_gpus.push_back(trimmedName);
            }
        }

        if (!invalid_gpus.empty())
        {
            // Build an error string indicating the invalid GPUs.
            std::stringstream error_stream;
            error_stream << kStrErrInvalidGpusSpecified;

            int numInvalid = static_cast<int>(invalid_gpus.size());
            for (int gpuIndex = 0; gpuIndex < numInvalid; ++gpuIndex)
            {
                error_stream << invalid_gpus[gpuIndex];

                if (gpuIndex != (numInvalid - 1))
                {
                    error_stream << ", ";
                }
            }

            // Add an error field indicating that the GPU name is invalid.
            errors.push_back(error_stream.str());
            is_valid = false;
        }
    }

    return is_valid;
}

bool RgBuildSettingsViewOpencl::ValidatePendingSettings()
{
    std::vector<std::string> errors;
    bool is_valid = IsTargetGpusStringValid(errors);
    if (!is_valid)
    {
        std::stringstream error_stream;
        error_stream << kStrErrInvalidPendingSetting;
        error_stream << std::endl;

        // Loop through all errors and append them to the stream.
        for (const std::string error_string : errors)
        {
            error_stream << error_string;
            error_stream << std::endl;
        }

        // Display an error message box to the user telling them what to fix.
        RgUtils::ShowErrorMessageBox(error_stream.str().c_str(), this);
    }

    return is_valid;
}

void RgBuildSettingsViewOpencl::SetCursor()
{
    // Set the cursor to pointing hand cursor.
    ui_.addTargetGPUsButton->setCursor(Qt::PointingHandCursor);
    ui_.aggressiveMathOptimizationsCheckBox->setCursor(Qt::PointingHandCursor);
    ui_.allowUnsafeOptimizationsCheckBox->setCursor(Qt::PointingHandCursor);
    ui_.assumeNoNanNorInfiniteCheckBox->setCursor(Qt::PointingHandCursor);
    ui_.correctlyRoundSinglePrecisionCheckBox->setCursor(Qt::PointingHandCursor);
    ui_.doubleAsSingleCheckBox->setCursor(Qt::PointingHandCursor);
    ui_.enableMADCheckBox->setCursor(Qt::PointingHandCursor);
    ui_.flushDenormalizedCheckBox->setCursor(Qt::PointingHandCursor);
    ui_.ignoreZeroSignednessCheckBox->setCursor(Qt::PointingHandCursor);
    ui_.strictAliasingCheckBox->setCursor(Qt::PointingHandCursor);
    ui_.optimizationLevelComboBox->setCursor(Qt::PointingHandCursor);
    ui_.includeDirsBrowseButton->setCursor(Qt::PointingHandCursor);
    ui_.predefinedMacrosBrowseButton->setCursor(Qt::PointingHandCursor);

    // Set the cursor for alternative compiler buttons.
    ui_.compilerBinariesBrowseButton->setCursor(Qt::PointingHandCursor);
    ui_.compilerIncludesBrowseButton->setCursor(Qt::PointingHandCursor);
    ui_.compilerLibrariesBrowseButton->setCursor(Qt::PointingHandCursor);
}

void RgBuildSettingsViewOpencl::HandleIncludeDirsBrowseButtonClick()
{
    // Position the window in the middle of the screen.
    include_directories_view_->setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, include_directories_view_->size(), QGuiApplication::primaryScreen()->availableGeometry()));

    // Set the current include dirs.
    include_directories_view_->SetListItems(ui_.includeDirectoriesLineEdit->text());

    // Show the window.
    include_directories_view_->exec();
}

void RgBuildSettingsViewOpencl::HandleIncludeDirsUpdated(QStringList includeDirs)
{
    QString includeDirsText;

    // Create a delimiter-separated string.
    if (!includeDirs.isEmpty())
    {
        includeDirsText = includeDirs.join(kOptionsListDelimiter);
    }

    // Update the text box.
    ui_.includeDirectoriesLineEdit->setText(includeDirsText);
}

void RgBuildSettingsViewOpencl::HandlePreprocessorDirectivesBrowseButtonClick()
{
    // Position the window in the middle of the screen.
    preprocessor_directives_dialog_->setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, preprocessor_directives_dialog_->size(), QGuiApplication::primaryScreen()->availableGeometry())); 

    // Set the current preprocessor directives in the dialog.
    preprocessor_directives_dialog_->SetListItems(ui_.predefinedMacrosLineEdit->text());

    // Show the dialog.
    preprocessor_directives_dialog_->exec();
}

void RgBuildSettingsViewOpencl::HandlePreprocessorDirectivesUpdated(QStringList preprocessor_directives)
{
    QString preprocessorDirectivesText;

    // Create a delimiter-separated string.
    if (!preprocessor_directives.isEmpty())
    {
        preprocessorDirectivesText = preprocessor_directives.join(kOptionsListDelimiter);
    }

    // Update the text box.
    ui_.predefinedMacrosLineEdit->setText(preprocessorDirectivesText);
}

void RgBuildSettingsViewOpencl::HandleAdditionalOptionsTextChanged()
{
    // Inform the UI of a possible change to the pending state.
    HandlePendingChangesStateChanged(GetHasPendingChanges());

    // Update the command line preview text.
    UpdateCommandLineText();
}

void RgBuildSettingsViewOpencl::SetToolTipGeometry()
{
    // Get the font metrics.
    QFontMetrics fontMetrics(ui_.doubleAsSingleCheckBox->font());

    // Use the width of an edit box as width of the tooltip string.
    const int width = ui_.includeDirectoriesLineEdit->width();

    // Calculate the height of the tooltip string.
    const int height = fontMetrics.height();

    // Create a width and a height string.
    const QString widthString = QString(kStrFilemenuTitleBarTooltipWidth).arg(width).arg(width);
    const QString heightString = QString(kStrFilemenuTitleBarTooltipHeight).arg(height).arg(height).arg(height);

    // Set the stylesheet.
    ui_.doubleAsSingleCheckBox->setStyleSheet("QToolTip {" + widthString + heightString + "}");
    ui_.flushDenormalizedCheckBox->setStyleSheet("QToolTip {" + widthString + "}");
    ui_.aggressiveMathOptimizationsCheckBox->setStyleSheet("QToolTip {" + widthString + "}");
    ui_.allowUnsafeOptimizationsCheckBox->setStyleSheet("QToolTip {" + widthString + "}");
    ui_.assumeNoNanNorInfiniteCheckBox->setStyleSheet("QToolTip {" + widthString + "}");
    ui_.correctlyRoundSinglePrecisionCheckBox->setStyleSheet("QToolTip {" + widthString + "}");
    ui_.doubleAsSingleCheckBox->setStyleSheet("QToolTip {" + widthString + "}");
    ui_.enableMADCheckBox->setStyleSheet("QToolTip {" + widthString + "}");
    ui_.ignoreZeroSignednessCheckBox->setStyleSheet("QToolTip {" + widthString + "}");
    ui_.strictAliasingCheckBox->setStyleSheet("QToolTip {" + widthString + "}");
    ui_.additionalOptionsHeaderLabel->setStyleSheet("QToolTip {" + widthString + "}");
}

void RgBuildSettingsViewOpencl::HandleLineEditFocusInEvent()
{
    emit SetFrameBorderGreenSignal();
}

void RgBuildSettingsViewOpencl::HandleLineEditFocusOutEvent()
{
    emit SetFrameBorderBlackSignal();
}

void RgBuildSettingsViewOpencl::HandleCheckBoxClickedEvent()
{
    emit SetFrameBorderGreenSignal();
}

void RgBuildSettingsViewOpencl::HandleComboBoxFocusInEvent()
{
    emit SetFrameBorderGreenSignal();
}

void RgBuildSettingsViewOpencl::HandleCompilerFolderBrowseButtonClick(CompilerFolderType folder_type)
{
    QFileDialog fileDialog;

    // If existing text in the line edit is not empty, pass it to the File Dialog.
    // Otherwise, get the latest entry from the directories list and open the dialog there.
    QLineEdit* line_edit = (folder_type == CompilerFolderType::kBin ? ui_.compilerBinariesLineEdit :
                           (folder_type == CompilerFolderType::kInclude ? ui_.compilerIncludesLineEdit : ui_.compilerLibrariesLineEdit));
    assert(line_edit != nullptr);
    if (line_edit != nullptr)
    {
        QString currentDir = (line_edit->text().isEmpty() ? QString::fromStdString(RgConfigManager::Instance().GetLastSelectedFolder()) : line_edit->text());

        QString selectedDirectory = QFileDialog::getExistingDirectory(this, tr(kStrIncludeDirDialogSelectDirTitle),
                                                                      currentDir, QFileDialog::ShowDirsOnly);
        if (!selectedDirectory.isEmpty())
        {
            switch (folder_type)
            {
            case CompilerFolderType::kBin:      ui_.compilerBinariesLineEdit->setText(selectedDirectory);  break;
            case CompilerFolderType::kInclude:  ui_.compilerIncludesLineEdit->setText(selectedDirectory);  break;
            case CompilerFolderType::kLib:      ui_.compilerLibrariesLineEdit->setText(selectedDirectory); break;
            default:                           assert(false);
            }
        }
    }
}

void RgBuildSettingsViewOpencl::HandleCompilerFolderEditChanged(CompilerFolderType folder_type)
{
    Q_UNUSED(folder_type);

    auto pTextEdit = qobject_cast<QLineEdit*>(QObject::sender());
    assert(pTextEdit != nullptr);
    if (pTextEdit != nullptr)
    {
        // Inform the UI of a possible change to the pending state.
        HandlePendingChangesStateChanged(GetHasPendingChanges());
    }

    // Update the command line preview text.
    UpdateCommandLineText();
}

bool RgBuildSettingsViewOpencl::GetHasPendingChanges() const
{
    bool ret = false;

    RgBuildSettingsOpencl apiBuildSettings = PullFromWidgets();

    ret = !initial_settings_.HasSameSettings(apiBuildSettings);

    return ret;
}

bool RgBuildSettingsViewOpencl::RevertPendingChanges()
{
    PushToWidgets(initial_settings_);

    // Inform the UI of a possible change to the pending state.
    HandlePendingChangesStateChanged(GetHasPendingChanges());

    return false;
}

void RgBuildSettingsViewOpencl::RestoreDefaultSettings()
{
    bool isRestored = false;

    if (is_global_settings_)
    {
        // If this is for the global settings, then restore to the hard-coded defaults.

        // Get the hard-coded default build settings.
        std::shared_ptr<RgBuildSettings> pDefaultBuildSettings = RgConfigManager::GetDefaultBuildSettings(RgProjectAPI::kOpenCL);

        std::shared_ptr<RgBuildSettingsOpencl> api_build_settings = std::dynamic_pointer_cast<RgBuildSettingsOpencl>(pDefaultBuildSettings);
        assert(api_build_settings != nullptr);
        if (api_build_settings != nullptr)
        {
            // Reset our initial settings back to the defaults.
            initial_settings_ = *api_build_settings;

            // Update the UI to reflect the new initial settings.
            PushToWidgets(initial_settings_);

            // Update the ConfigManager to use the new settings.
            RgConfigManager::Instance().SetApiBuildSettings(kStrApiNameOpencl, &initial_settings_);

            // Save the settings file
            isRestored = RgConfigManager::Instance().SaveGlobalConfigFile();

            // Inform the rest of the UI that the settings have been changed.
            HandlePendingChangesStateChanged(GetHasPendingChanges());
        }
    }
    else
    {
        // This view is showing project-specific settings, so restore back to the stored settings in the project.
        std::shared_ptr<RgBuildSettings> pDefaultSettings = RgConfigManager::Instance().GetUserGlobalBuildSettings(RgProjectAPI::kOpenCL);
        auto pCLDefaultSettings = std::dynamic_pointer_cast<RgBuildSettingsOpencl>(pDefaultSettings);

        initial_settings_ = *pCLDefaultSettings;

        PushToWidgets(initial_settings_);

        // Inform the rest of the UI that the settings have been changed.
        HandlePendingChangesStateChanged(GetHasPendingChanges());

        // Let the RgBuildView know that the build settings have been updated.
        emit ProjectBuildSettingsSaved(pCLDefaultSettings);
        isRestored = true;
    }

    // Show an error dialog if the settings failed to be reset.
    if (!isRestored)
    {
        RgUtils::ShowErrorMessageBox(kStrErrCannotRestoreDefaultSettings, this);
    }
}

bool RgBuildSettingsViewOpencl::SaveSettings()
{
    bool is_valid = ValidatePendingSettings();
    if (is_valid)
    {
        // Reset the initial settings to match what the UI shows.
        initial_settings_ = PullFromWidgets();

        if (is_global_settings_)
        {
            // Update the config manager to use these new settings.
            RgConfigManager& config_manager = RgConfigManager::Instance();
            config_manager.SetApiBuildSettings(kStrApiNameOpencl, &initial_settings_);

            // Save the global config settings.
            is_valid = config_manager.SaveGlobalConfigFile();
        }
        else
        {
            // Save the project settings.
            std::shared_ptr<RgBuildSettingsOpencl> pTmpPtr = std::make_shared<RgBuildSettingsOpencl>(initial_settings_);
            emit ProjectBuildSettingsSaved(pTmpPtr);
        }

        if (is_valid)
        {
            // Make sure the rest of the UI knows that the settings have been saved.
            HandlePendingChangesStateChanged(false);
        }
    }

    // Set focus to target GPUs browse button.
    ui_.addTargetGPUsButton->setFocus();

    return is_valid;
}

void RgBuildSettingsViewOpencl::mousePressEvent(QMouseEvent* event)
{
    Q_UNUSED(event);

    emit SetFrameBorderGreenSignal();
}

void RgBuildSettingsViewOpencl::SetInitialWidgetFocus()
{
    ui_.addTargetGPUsButton->setFocus();
}

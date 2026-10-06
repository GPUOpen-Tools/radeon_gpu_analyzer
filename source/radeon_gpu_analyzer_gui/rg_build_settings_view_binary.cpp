// C++.
#include <cassert>
#include <sstream>

// Qt.
#include <QWidget>
#include <QMessageBox>
#include <QFileDialog>
#include <QComboBox>

// Infra.
#include "qt_common/utils/restore_cursor_position.h"
#include "common/rga_cli_defs.h"

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_build_settings_view.h"
#include "radeon_gpu_analyzer_gui/qt/rg_build_settings_view_binary.h"
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

RgBuildSettingsViewBinary::RgBuildSettingsViewBinary(QWidget* parent, const RgBuildSettingsBinary& build_settings, bool is_global_settings) :
    RgBuildSettingsView(parent, is_global_settings),
    initial_settings_(build_settings)
{
    // Setup the UI.
    ui_.setupUi(this);

    // Create the include directories editor view.
    include_directories_view_ = new RgIncludeDirectoriesView(kOptionsListDelimiter, this);
    include_directories_view_->setWindowTitle(kStrSourceSearchDirsDialogTitle);

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
    ui_.includeDirectoriesLabel->setToolTip(kStrBuildSettingsAdditionalSourceDirectoryTooltip);

    // Set the tooltip for the General section of the build settings.
    ui_.generalHeaderLabel->setToolTip(kStrBuildSettingsGeneralTooltip);

    // Set the tooltip for the command line section of the build settings.
    ui_.settingsCommandLineHeaderLabel->setToolTip(kStrBuildSettingsCmdLineTooltip);

    // Set the mouse cursor to the pointing hand cursor for various widgets.
    SetCursor();

    // Set the geometry of tool tip boxes.
    SetToolTipGeometry();

    // Set the event filter for "All Options" text edits.
    ui_.allOptionsTextEdit->installEventFilter(this);
}

void RgBuildSettingsViewBinary::PushToWidgets(const RgBuildSettingsBinary& build_settings)
{
    // The items below are common build settings for all API types.
    QString additionalIncludeDirs(RgUtils::BuildSemicolonSeparatedStringList(build_settings.additional_include_directories).c_str());
    ui_.includeDirectoriesLineEdit->setText(additionalIncludeDirs);

    // Items below are Binary-specific build settings only.
    ui_.promptToAttachSourceCheckBox->setChecked(build_settings.prompt_to_attach_source_dirs);
}

RgBuildSettingsBinary RgBuildSettingsViewBinary::PullFromWidgets() const
{
    // Seed from initial_settings_ rather than default-constructing: this view has no widgets for
    // target_gpus/predefined_macros/additional_options/compiler_paths, so a default-constructed
    // struct would always differ from a legacy project's initial_settings_ in those fields,
    // producing a false "pending changes" positive and wiping the fields out on save.
    RgBuildSettingsBinary settings = initial_settings_;

    // Additional Include Directories
    std::vector<std::string> additional_include_directories_vector;
    const std::string& comma_separated_additional_include_directories = ui_.includeDirectoriesLineEdit->text().toStdString();
    RgUtils::splitString(comma_separated_additional_include_directories, RgConfigManager::kRgaListDelimiter, additional_include_directories_vector);
    settings.additional_include_directories = additional_include_directories_vector;

    // PromptToAttachSourceDirs:
    settings.prompt_to_attach_source_dirs = ui_.promptToAttachSourceCheckBox->isChecked();

    return settings;
}

void RgBuildSettingsViewBinary::ConnectSignals()
{
    // Add include directories editor dialog button.
    bool is_connected = connect(this->ui_.includeDirsBrowseButton, &QPushButton::clicked, this, &RgBuildSettingsViewBinary::HandleIncludeDirsBrowseButtonClick);
    assert(is_connected);

    is_connected = connect(this->ui_.includeDirectoriesLineEdit, &QLineEdit::textChanged, this, &RgBuildSettingsViewBinary::HandleTextEditChanged);
    assert(is_connected);

    // Connect the include directory editor dialog's "OK" button click.
    is_connected = connect(include_directories_view_, &RgIncludeDirectoriesView::OKButtonClicked, this, &RgBuildSettingsViewBinary::HandleIncludeDirsUpdated);
    assert(is_connected);

    is_connected = connect(this->ui_.promptToAttachSourceCheckBox, &RgCheckBox::stateChanged, this, &RgBuildSettingsViewBinary::HandleCheckboxStateChanged);
    assert(is_connected);
}

void RgBuildSettingsViewBinary::ConnectLineEditFocusEvents()
{
    bool is_connected = false;

    is_connected = connect(this->ui_.includeDirectoriesLineEdit, &RgLineEdit::LineEditFocusInEvent, this, &RgBuildSettingsViewBinary::HandleLineEditFocusInEvent);
    assert(is_connected);

    is_connected = connect(this->ui_.includeDirectoriesLineEdit, &RgLineEdit::LineEditFocusOutEvent, this, &RgBuildSettingsViewBinary::HandleLineEditFocusOutEvent);
    assert(is_connected);
}

void RgBuildSettingsViewBinary::ConnectCheckBoxClickedEvents()
{
}

void RgBuildSettingsViewBinary::ConnectComboboxClickEvent()
{
    [[maybe_unused]] bool is_connected = connect(this->ui_.promptToAttachSourceCheckBox, &RgCheckBox::clicked, this, &RgBuildSettingsViewBinary::HandleCheckBoxClickedEvent);
    assert(is_connected);
}

bool RgBuildSettingsViewBinary::eventFilter(QObject* object, QEvent* event)
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

void RgBuildSettingsViewBinary::HandleTextEditChanged()
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

void RgBuildSettingsViewBinary::HandleComboboxIndexChanged(int index)
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

void RgBuildSettingsViewBinary::HandleCheckboxStateChanged()
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

std::string RgBuildSettingsViewBinary::GetTitleString()
{
    std::stringstream title_string;

    // Build the title string.
    if (is_global_settings_)
    {
        // For the global settings.
        title_string << kStrBuildSettingsDefaultTitle;
        title_string << " ";
        title_string << kStrApiNameBinary;
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

const std::string RgBuildSettingsViewBinary::GetTitleTooltipString() const
{
    std::stringstream tooltip_string;

    if (is_global_settings_)
    {
        tooltip_string << kStrBuildSettingsGlobalTooltipA;
        tooltip_string << kStrApiNameBinary;
        tooltip_string << kStrBuildSettingsGlobalTooltipB;
    }
    else
    {
        tooltip_string << kStrBuildSettingsProjectTooltipA;
        tooltip_string << kStrApiNameBinary;
        tooltip_string << kStrBuildSettingsProjectTooltipB;
    }

    return tooltip_string.str();
}

void RgBuildSettingsViewBinary::HandlePendingChangesStateChanged(bool has_pending_changes)
{
    // Let the base class determine if there is a need to signal listeners
    // about the pending changes state.
    RgBuildSettingsView::SetHasPendingChanges(has_pending_changes);
}

void RgBuildSettingsViewBinary::UpdateCommandLineText()
{
    RgBuildSettingsBinary api_build_setting = PullFromWidgets();

    // Generate a command line string from the build settings structure.
    std::string build_settings;
    bool ret = RgCliUtils::GenerateBinaryBuildSettingsString(api_build_setting, build_settings, false);
    assert(ret);
    if (ret)
    {
        ui_.allOptionsTextEdit->setPlainText(build_settings.c_str());
    }
}

void RgBuildSettingsViewBinary::SetCursor()
{
    // Set the cursor to pointing hand cursor.
    ui_.includeDirsBrowseButton->setCursor(Qt::PointingHandCursor);
    ui_.promptToAttachSourceCheckBox->setCursor(Qt::PointingHandCursor);
}

void RgBuildSettingsViewBinary::SetToolTipGeometry()
{
    // Get the font metrics.
    QFontMetrics fontMetrics(ui_.promptToAttachSourceCheckBox->font());

    // Use the width of an edit box as width of the tooltip string.
    const int width = ui_.includeDirectoriesLineEdit->width();

    // Calculate the height of the tooltip string.
    const int height = fontMetrics.height();

    // Create a width and a height string.
    const QString widthString  = QString(kStrFilemenuTitleBarTooltipWidth).arg(width).arg(width);
    const QString heightString = QString(kStrFilemenuTitleBarTooltipHeight).arg(height).arg(height).arg(height);

    // Set the stylesheet.
    ui_.promptToAttachSourceCheckBox->setStyleSheet("QToolTip {" + widthString + heightString + "}");
    ui_.promptToAttachSourceCheckBox->setStyleSheet("QToolTip {" + widthString + "}");
}

void RgBuildSettingsViewBinary::HandleIncludeDirsBrowseButtonClick()
{
    // Position the window in the middle of the screen.
    include_directories_view_->setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, include_directories_view_->size(), QGuiApplication::primaryScreen()->availableGeometry()));

    // Set the current include dirs.
    include_directories_view_->SetListItems(ui_.includeDirectoriesLineEdit->text());

    // Show the window.
    include_directories_view_->exec();
}

void RgBuildSettingsViewBinary::HandleIncludeDirsUpdated(QStringList includeDirs)
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

void RgBuildSettingsViewBinary::HandleLineEditFocusInEvent()
{
    emit SetFrameBorderPurpleSignal();
}

void RgBuildSettingsViewBinary::HandleLineEditFocusOutEvent()
{
    emit SetFrameBorderBlackSignal();
}

void RgBuildSettingsViewBinary::HandleCheckBoxClickedEvent()
{
    emit SetFrameBorderPurpleSignal();
}

void RgBuildSettingsViewBinary::HandleComboBoxFocusInEvent()
{
    emit SetFrameBorderPurpleSignal();
}

bool RgBuildSettingsViewBinary::GetHasPendingChanges() const
{
    bool ret = false;

    RgBuildSettingsBinary apiBuildSettings = PullFromWidgets();

    ret = !initial_settings_.HasSameSettings(apiBuildSettings);

    return ret;
}

bool RgBuildSettingsViewBinary::RevertPendingChanges()
{
    PushToWidgets(initial_settings_);

    // Inform the UI of a possible change to the pending state.
    HandlePendingChangesStateChanged(GetHasPendingChanges());

    return false;
}

void RgBuildSettingsViewBinary::RestoreDefaultSettings()
{
    bool isRestored = false;

    if (is_global_settings_)
    {
        // If this is for the global settings, then restore to the hard-coded defaults.

        // Get the hard-coded default build settings.
        std::shared_ptr<RgBuildSettings> pDefaultBuildSettings = RgConfigManager::GetDefaultBuildSettings(RgProjectAPI::kBinary);

        std::shared_ptr<RgBuildSettingsBinary> api_build_settings = std::dynamic_pointer_cast<RgBuildSettingsBinary>(pDefaultBuildSettings);
        assert(api_build_settings != nullptr);
        if (api_build_settings != nullptr)
        {
            // Reset our initial settings back to the defaults.
            initial_settings_ = *api_build_settings;

            // Update the UI to reflect the new initial settings.
            PushToWidgets(initial_settings_);

            // Update the ConfigManager to use the new settings.
            RgConfigManager::Instance().SetApiBuildSettings(kStrApiAbbreviationBinary, &initial_settings_);

            // Save the settings file
            isRestored = RgConfigManager::Instance().SaveGlobalConfigFile();

            // Inform the rest of the UI that the settings have been changed.
            HandlePendingChangesStateChanged(GetHasPendingChanges());
        }
    }
    else
    {
        // This view is showing project-specific settings, so restore back to the stored settings in the project.
        std::shared_ptr<RgBuildSettings> pDefaultSettings = RgConfigManager::Instance().GetUserGlobalBuildSettings(RgProjectAPI::kBinary);
        auto pCLDefaultSettings = std::dynamic_pointer_cast<RgBuildSettingsBinary>(pDefaultSettings);

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

bool RgBuildSettingsViewBinary::SaveSettings()
{
    bool is_valid = true;

    // Reset the initial settings to match what the UI shows.
    initial_settings_ = PullFromWidgets();

    if (is_global_settings_)
    {
        // Update the config manager to use these new settings.
        RgConfigManager& config_manager = RgConfigManager::Instance();
        config_manager.SetApiBuildSettings(kStrApiAbbreviationBinary, &initial_settings_);

        // Save the global config settings.
        is_valid = config_manager.SaveGlobalConfigFile();
    }
    else
    {
        // Save the project settings.
        std::shared_ptr<RgBuildSettingsBinary> pTmpPtr = std::make_shared<RgBuildSettingsBinary>(initial_settings_);
        emit                                   ProjectBuildSettingsSaved(pTmpPtr);
    }

    if (is_valid)
    {
        // Make sure the rest of the UI knows that the settings have been saved.
        HandlePendingChangesStateChanged(false);
    }

    // Set focus to include directories browse button.
    ui_.includeDirsBrowseButton->setFocus();

    return true;
}

void RgBuildSettingsViewBinary::mousePressEvent(QMouseEvent* event)
{
    Q_UNUSED(event);

    emit SetFrameBorderPurpleSignal();
}

void RgBuildSettingsViewBinary::SetInitialWidgetFocus()
{
    ui_.includeDirsBrowseButton->setFocus();
}

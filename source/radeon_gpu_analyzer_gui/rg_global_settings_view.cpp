// C++.
#include <cassert>
#include <sstream>

// Qt.
#include <QApplication>
#include <QCheckBox>
#include <QDir>
#include <QFileDialog>
#include <QFocusEvent>
#include <QMessageBox>
#include <QProcess>
#include <QStyleHints>
#include <QWidget>

// Infra.
#include "qt_common/custom_widgets/arrow_icon_combo_box.h"
#include "qt_common/utils/common_definitions.h"
#include "qt_common/utils/qt_util.h"
#include "qt_common/utils/restore_cursor_position.h"

// Local.
#include "radeon_gpu_analyzer_gui/rg_config_manager.h"
#include "radeon_gpu_analyzer_gui/rg_definitions.h"
#include "radeon_gpu_analyzer_gui/qt/rg_global_settings_view.h"
#include "radeon_gpu_analyzer_gui/qt/rg_hide_list_widget_event_filter.h"
#include "radeon_gpu_analyzer_gui/qt/rg_line_edit.h"
#include "radeon_gpu_analyzer_gui/rg_string_constants.h"
#include "radeon_gpu_analyzer_gui/rg_utils.h"

// Names of special widgets within the build settings view.
static const char* kStrGlobalSettingsColumnVisibilityList      = "DisassemblyColumnVisibilityList";
static const char* kStrGlobalSettingsColumnListItemCheckbox    = "ListWidgetCheckBox";
static const char* kStrGlobalSettingsColumnListItemAllCheckbox = "ListWidgetAllCheckBox";

// API-specific colors.
static const QColor kApiColorBinary = QColor(128, 0, 128);
static const QColor kApiColorOpencl = QColor(18, 152, 0);
static const QColor kApiColorVulkan = QColor(224, 30, 55);

const static QString kLightThemeOption = "Light";
const static QString kDarkThemeOption  = "Dark";
const static QString kDetectOsOption   = "Detect OS";

// Columns push button font size.
const int kPushButtonFontSize = 11;

RgGlobalSettingsView::RgGlobalSettingsView(QWidget* parent, const RgGlobalSettings& global_settings)
    : RgBuildSettingsView(parent, true)
    , initial_settings_(global_settings)
    , parent_(parent)
{
    // Setup the UI.
    ui_.setupUi(this);

    // Create the widget used to control column visibility.
    CreateColumnVisibilityControls();

    // Fill the column visibility dropdown with all of the possible column names.
    PopulateColumnVisibilityList();

    // Initialize the combo box for font size.
    PopulateFontSizeDropdown();

    ui_.colorThemeComboBox->InitSingleSelect(this, kLightThemeOption, false);
    ui_.colorThemeComboBox->AddItem(kLightThemeOption, kColorThemeTypeLight);
    ui_.colorThemeComboBox->AddItem(kDarkThemeOption, kColorThemeTypeDark);
    ui_.colorThemeComboBox->AddItem(kDetectOsOption, kColorThemeTypeCount);

    ui_.colorThemeComboBox->SetSelectedRow(initial_settings_.color_theme);

    connect(ui_.colorThemeComboBox, &ArrowIconComboBox::SelectedItem, this, &RgGlobalSettingsView::HandleColorThemeComboBoxChanged);
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged, this, &RgGlobalSettingsView::HandleOsColorSchemeChanged);
#endif

    // Push values to various widgets.
    PushToWidgets(global_settings);

    // Connect the signals.
    ConnectSignals();

    // Set the mouse cursor to the pointing hand cursor for various widgets.
    SetCursor();

    // Set the tool tip for default project name check box.
    SetCheckboxToolTip(kStrGlobalSettingsCheckboxTooltip);

    // Set the tool tip for the two edit boxes.
    SetEditBoxToolTips();

    // Set the tooltip for the log file location.
    ui_.logFileLocationLabel->setToolTip(kStrGlobalSettingsLogFileLocation);

    // Set the tooltip for the project file location.
    ui_.projectFileLocationLabel->setToolTip(kStrGlobalSettingsProjectFileLocation);

    // Set the tooltips for the disassembly section.
    ui_.disassemblyViewLabel->setToolTip(kStrGlobalSettingsDisassemblySectionTooltip);

    // Set the tooltips for the disassembly columns.
    ui_.disassemblyViewColunnsLabel->setToolTip(kStrGlobalSettingsDisassemblyColumnsTooltip);

    // Set the tooltips for the source code editor.
    ui_.sourceCodeEditorLabel->setToolTip(kStrGlobalSettingsSrcViewSectionTooltip);

    // Set the tooltips for the font type in the source code editor.
    ui_.fontFamilyLabel->setToolTip(kStrGlobalSettingsSrcViewFontTypeTooltip);

    // Set the tooltips for the font size in the source code editor.
    ui_.fontSizeLabel->setToolTip(kStrGlobalSettingsSrcViewFontSizeTooltip);

    // Set the tooltips for the include files viewer.
    ui_.includeFilesViewerLabel->setToolTip(kStrGlobalSettingsSrcViewIncludeFilesViewerTooltip);

    // Set the tooltips for input files associations.
    ui_.assocExtGlslLabel->setToolTip(kStrGlobalSettnigsFileExtGlslTooltip);
    ui_.assocExtHlslLabel->setToolTip(kStrGlobalSettnigsFileExtHlslTooltip);
    ui_.assocExtSpvasLabel->setToolTip(kStrGlobalSettnigsFileExtSpvTxtTooltip);

    // Set the tooltip for the default source language.
    ui_.defaultLangLabel->setToolTip(kStrGlobalSettingsDefaultSrcLangTooltip);

    // Set the tooltip for the general section.
    ui_.generalLabel->setToolTip(kStrGlobalSettingsGeneralSectionTooltip);

    // Set the tooltip for the default API section.
    ui_.defaultApiOnStartupLabel->setToolTip(kStrGlobalSettingsDefaultStartupApiTooltip);

    // Set the tooltip for the Input Files section.
    ui_.inputFilesLabel->setToolTip(kStrGlobalSettingsInputFilesSectionTooltip);

    // Set the tooltip for the SPIR-V binary.
    ui_.assocExtSpvBinaryLabel->setToolTip(kStrGlobalSettingsSpvExtensionsTooltip);

    // Hide HLSL components until support for HLSL is added.
    ui_.assocExtHlslLabel->hide();
    ui_.assocExtHlslLineEdit->hide();
    ui_.defaultLangLabel->hide();
    ui_.defaultLangComboBox->hide();

    // Set column visibility arrow push button properties.
    RgProjectAPI current_api = RgConfigManager::Instance().GetCurrentAPI();
    if (current_api == RgProjectAPI::kOpenCL)
    {
        ui_.columnVisibilityArrowPushButton->SetBorderColor(kApiColorOpencl);
        ui_.columnVisibilityArrowPushButton->SetShowBorder(true);
        ui_.colorThemeComboBox->SetBorderColor(kApiColorOpencl);
        ui_.colorThemeComboBox->SetShowBorder(true);
    }
    else if (current_api == RgProjectAPI::kVulkan)
    {
        ui_.columnVisibilityArrowPushButton->SetBorderColor(kApiColorVulkan);
        ui_.columnVisibilityArrowPushButton->SetShowBorder(true);
        ui_.colorThemeComboBox->SetBorderColor(kApiColorVulkan);
        ui_.colorThemeComboBox->SetShowBorder(true);
    }
    else if(current_api == RgProjectAPI::kBinary)
    {
        ui_.columnVisibilityArrowPushButton->SetBorderColor(kApiColorBinary);
        ui_.columnVisibilityArrowPushButton->SetShowBorder(true);
        ui_.colorThemeComboBox->SetBorderColor(kApiColorBinary);
        ui_.colorThemeComboBox->SetShowBorder(true);
    }    
    else
    {
        // Should not get here.
        assert(false);
    }

    // Unsupported options under Settings tab needs to be disabled for binary mode.
    if (current_api == RgProjectAPI::kBinary)
    {
        ui_.sourceCodeEditorLabel->hide();
        ui_.fontFamilyLabel->hide();
        ui_.fontFamilyComboBox->hide();
        ui_.fontSizeLabel->hide();
        ui_.fontSizeComboBox->hide();
        ui_.includeFilesViewerLabel->hide();
        ui_.includeFilesViewerLineEdit->hide();
        ui_.includeFilesViewerBrowseButton->hide();

        ui_.inputFilesLabel->hide();     
        ui_.assocExtGlslLabel->hide();        
        ui_.assocExtGlslLineEdit->hide();        
        ui_.assocExtHlslLabel->hide();        
        ui_.assocExtHlslLineEdit->hide();   
        ui_.assocExtSpvasLabel->hide();        
        ui_.assocExtSpvasLineEdit->hide();        
        ui_.assocExtSpvBinaryLabel->hide();        
        ui_.assocExtSpvBinaryLineEdit->hide();        
    }
}

RgGlobalSettingsView::~RgGlobalSettingsView()
{
}

void RgGlobalSettingsView::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);

    // The global settings can be updated by other widgets in the application,
    // so if this view does not think it has any pending changes, then reset
    // the UI based on the config manager's global config, then update this
    // view's intitialSettings to match and signal that there are no pending changes.
    if (GetHasPendingChanges() == false)
    {
        std::shared_ptr<RgGlobalSettings> pSettings = RgConfigManager::Instance().GetGlobalConfig();
        assert(pSettings != nullptr);
        if (pSettings != nullptr)
        {
            PushToWidgets(*pSettings);
        }

        initial_settings_ = PullFromWidgets();

        HandlePendingChangesStateChanged(false);
    }
}

void RgGlobalSettingsView::PushToWidgets(const RgGlobalSettings& global_settings)
{
    // Initialize the log file location.
    QString log_file_location(global_settings.log_file_location.c_str());
    ui_.logFileLocationLineEdit->setText(log_file_location);

    // Initialize the project file location.
    QString project_file_location(global_settings.project_file_location.c_str());
    ui_.projectFileLocationLineEdit->setText(project_file_location);

     RgUtils::SetColumnVisibilityCheckboxes(ui_.columnVisibilityArrowPushButton, global_settings.visible_disassembly_view_columns);

    // Initialize the use-generated project names checkbox.
    ui_.projectNameCheckBox->setChecked(global_settings.use_default_project_name);

    // Initialize the combo box for setting the default API.
    ui_.defaultApiOnStartupComboBox->setCurrentIndex(global_settings.should_prompt_for_api ? static_cast<int>(RgProjectAPI::kUnknown)
                                                                                           : static_cast<int>(global_settings.default_api));

    // Font family.
    QFont font;
    font.setFamily(QString::fromStdString(global_settings.font_family));
    ui_.fontFamilyComboBox->setCurrentFont(font);

    // Font size.
    const int index = ui_.fontSizeComboBox->findData(global_settings.font_size);
    if (index != -1)
    {
        ui_.fontSizeComboBox->setCurrentIndex(index);
    }

    ui_.colorThemeComboBox->SetSelectedRow(global_settings.color_theme);

    // Include files viewer.
    ui_.includeFilesViewerLineEdit->setText(global_settings.include_files_viewer.c_str());

    // Input file associations.
    ui_.assocExtGlslLineEdit->setText(global_settings.input_file_ext_glsl.c_str());
    ui_.assocExtHlslLineEdit->setText(global_settings.input_file_ext_hlsl.c_str());
    ui_.assocExtSpvasLineEdit->setText(global_settings.input_file_ext_spv_txt.c_str());
    ui_.assocExtSpvBinaryLineEdit->setText(global_settings.input_file_ext_spv_bin.c_str());

    // Default high level language.
    ui_.defaultLangComboBox->setCurrentIndex(global_settings.default_lang == RgSrcLanguage::kGLSL ? 0 : 1);
}

RgGlobalSettings RgGlobalSettingsView::PullFromWidgets() const
{
    std::shared_ptr<RgGlobalSettings> settings_ptr = RgConfigManager::Instance().GetGlobalConfig();
    assert(settings_ptr != nullptr);

    // Make a local copy of this object so that it can be edited.
    RgGlobalSettings settings(*settings_ptr);

    // Log file location.
    settings.log_file_location = ui_.logFileLocationLineEdit->text().toStdString();

    // Project file location.
    settings.project_file_location = ui_.projectFileLocationLineEdit->text().toStdString();

    // Column visibility.
    settings.visible_disassembly_view_columns = RgUtils::GetColumnVisibilityCheckboxes(ui_.columnVisibilityArrowPushButton);

    // Use default project names.
    settings.use_default_project_name = ui_.projectNameCheckBox->isChecked();

    // Default API.
    int default_api_combo_box_index = ui_.defaultApiOnStartupComboBox->currentIndex();
    if (default_api_combo_box_index >= static_cast<int>(RgProjectAPI::kUnknown) && default_api_combo_box_index < static_cast<int>(RgProjectAPI::kApiCount))
    {
        settings.default_api = static_cast<RgProjectAPI>(default_api_combo_box_index);

        settings.should_prompt_for_api = (default_api_combo_box_index == static_cast<int>(RgProjectAPI::kUnknown));
    }

    // Font family.
    QFont font           = ui_.fontFamilyComboBox->currentFont();
    settings.font_family = font.family().toStdString();

    // Font size.
    settings.font_size = ui_.fontSizeComboBox->itemData(ui_.fontSizeComboBox->currentIndex()).toInt();

    settings.color_theme = ui_.colorThemeComboBox->CurrentRow();

    // Include files viewer.
    settings.include_files_viewer = ui_.includeFilesViewerLineEdit->text().toStdString();

    // Input file associations.
    settings.input_file_ext_glsl    = ui_.assocExtGlslLineEdit->text().toStdString();
    settings.input_file_ext_hlsl    = ui_.assocExtHlslLineEdit->text().toStdString();
    settings.input_file_ext_spv_txt = ui_.assocExtSpvasLineEdit->text().toStdString();
    settings.input_file_ext_spv_bin = ui_.assocExtSpvBinaryLineEdit->text().toStdString();

    // Default high level language.
    settings.default_lang = (ui_.defaultLangComboBox->currentIndex() == 0 ? RgSrcLanguage::kGLSL : RgSrcLanguage::kHLSL);

    return settings;
}

void RgGlobalSettingsView::ConnectSignals()
{
    // Browse log file location button.
    bool is_connected =
        connect(this->ui_.logFileLocationFolderOpenButton, &QPushButton::clicked, this, &RgGlobalSettingsView::HandleLogFileLocationBrowseButtonClick);
    assert(is_connected);

    // Browse project file location button.
    is_connected =
        connect(this->ui_.projectFileLocationFolderOpenButton, &QPushButton::clicked, this, &RgGlobalSettingsView::HandleProjectFileLocationBrowseButtonClick);
    assert(is_connected);

    // Log file location line edit.
    is_connected = connect(this->ui_.logFileLocationLineEdit, &QLineEdit::editingFinished, this, &RgGlobalSettingsView::HandleLogFileEditingFinished);
    assert(is_connected);

    // Log file location textChanged signal.
    is_connected = connect(this->ui_.logFileLocationLineEdit, &QLineEdit::textChanged, this, &RgGlobalSettingsView::HandleLogFileEditBoxChanged);
    assert(is_connected);

    // Project file location line edit.
    is_connected = connect(this->ui_.projectFileLocationLineEdit, &QLineEdit::editingFinished, this, &RgGlobalSettingsView::HandleProjectFileEditingFinished);
    assert(is_connected);

    // Project file location textChanged signal.
    is_connected = connect(this->ui_.projectFileLocationLineEdit, &QLineEdit::textChanged, this, &RgGlobalSettingsView::HandleProjectFileEditBoxChanged);
    assert(is_connected);

    // Connect the use default program name check box.
    is_connected = connect(this->ui_.projectNameCheckBox, &QCheckBox::stateChanged, this, &RgGlobalSettingsView::HandleProjectNameCheckboxStateChanged);
    assert(is_connected);

    // Connect the use default API combo box.
    is_connected = connect(ui_.defaultApiOnStartupComboBox,
                           static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                           this,
                           &RgGlobalSettingsView::HandleComboBoxChanged);
    assert(is_connected);

    // Connect the font family combo box.
    is_connected = connect(ui_.fontFamilyComboBox, &QFontComboBox::currentFontChanged, this, &RgGlobalSettingsView::HandleFontFamilyChanged);
    assert(is_connected);

    // Connect the font size combo box.
    is_connected = connect(
        ui_.fontSizeComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &RgGlobalSettingsView::HandleComboBoxChanged);
    assert(is_connected);

    // GLSL File extension associations textChanged signal.
    is_connected = connect(this->ui_.assocExtGlslLineEdit, &QLineEdit::textChanged, this, &RgGlobalSettingsView::HandleTextBoxChanged);
    assert(is_connected);

    // HLSL File extension associations textChanged signal.
    is_connected = connect(this->ui_.assocExtHlslLineEdit, &QLineEdit::textChanged, this, &RgGlobalSettingsView::HandleTextBoxChanged);
    assert(is_connected);

    // SPV Text File extension associations textChanged signal.
    is_connected = connect(this->ui_.assocExtSpvasLineEdit, &QLineEdit::textChanged, this, &RgGlobalSettingsView::HandleTextBoxChanged);
    assert(is_connected);

    // SPV Binary File extension associations textChanged signal.
    is_connected = connect(this->ui_.assocExtSpvBinaryLineEdit, &QLineEdit::textChanged, this, &RgGlobalSettingsView::HandleTextBoxChanged);
    assert(is_connected);

    // GLSL File extension associations textChanged signal.
    is_connected = connect(this->ui_.assocExtGlslLineEdit, &RgLineEdit::LineEditFocusOutEvent, this, &RgGlobalSettingsView::HandleFocusOutEvent);
    assert(is_connected);

    // HLSL File extension associations textChanged signal.
    is_connected = connect(this->ui_.assocExtHlslLineEdit, &RgLineEdit::LineEditFocusOutEvent, this, &RgGlobalSettingsView::HandleFocusOutEvent);
    assert(is_connected);

    // SPV Text File extension associations textChanged signal.
    is_connected = connect(this->ui_.assocExtSpvasLineEdit, &RgLineEdit::LineEditFocusOutEvent, this, &RgGlobalSettingsView::HandleFocusOutEvent);
    assert(is_connected);

    // SPV Binary File extension associations textChanged signal.
    is_connected = connect(this->ui_.assocExtSpvBinaryLineEdit, &RgLineEdit::LineEditFocusOutEvent, this, &RgGlobalSettingsView::HandleFocusOutEvent);
    assert(is_connected);

    // Default shader language combobox currentIndexChanged signal.
    is_connected = connect(this->ui_.defaultLangComboBox,
                           static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                           this,
                           &RgGlobalSettingsView::HandleComboBoxChanged);
    assert(is_connected);

    // Include files viewer browse button.
    is_connected =
        connect(this->ui_.includeFilesViewerBrowseButton, &QPushButton::clicked, this, &RgGlobalSettingsView::HandleIncludeFilesViewerBrowseButtonClick);
    assert(is_connected);

    // Include files viewer line edit.
    is_connected =
        connect(this->ui_.includeFilesViewerLineEdit, &QLineEdit::editingFinished, this, &RgGlobalSettingsView::HandleIncludeFilesViewerEditingFinished);
    assert(is_connected);

    // Include files viewer text changed.
    is_connected = connect(this->ui_.includeFilesViewerLineEdit, &QLineEdit::textChanged, this, &RgGlobalSettingsView::HandleTextBoxChanged);
    assert(is_connected);
}

void RgGlobalSettingsView::ProcessInputFileBlank() const
{
    if (ui_.assocExtGlslLineEdit->text().isEmpty())
    {
        emit ui_.assocExtGlslLineEdit->LineEditFocusOutEvent();
    }
    else if (ui_.assocExtHlslLineEdit->text().isEmpty())
    {
        emit ui_.assocExtHlslLineEdit->LineEditFocusOutEvent();
    }
    else if (ui_.assocExtSpvasLineEdit->text().isEmpty())
    {
        emit ui_.assocExtSpvasLineEdit->LineEditFocusOutEvent();
    }
    else if (ui_.assocExtSpvBinaryLineEdit->text().isEmpty())
    {
        emit ui_.assocExtSpvBinaryLineEdit->LineEditFocusOutEvent();
    }
}

void RgGlobalSettingsView::HandleFocusOutEvent()
{
    static const char* kStrGlslLineEdit      = "assocExtGlslLineEdit";
    static const char* kStrHlslLineEdit      = "assocExtHlslLineEdit";
    static const char* kStrSpvBinLineEdit    = "assocExtSpvBinaryLineEdit";
    static const char* kStrSpvTextLineEdit   = "assocExtSpvasLineEdit";
    static const char* kStrValueBlankMessage = "Input file type fields cannot be blank";

    // Figure out the sender and process appropriately.
    QObject* sender = QObject::sender();
    assert(sender != nullptr);

    RgLineEdit* line_edit = qobject_cast<RgLineEdit*>(sender);
    assert(line_edit != nullptr);

    // Process the click.
    if (line_edit != nullptr)
    {
        QString text = line_edit->text();
        if (text.isEmpty())
        {
            QWidget* widget = static_cast<QWidget*>(sender);
            assert(widget != nullptr);

            // Set the focus back to this widget.
            widget->setFocus();

            // Set the value to the previous one.
            QString name = line_edit->objectName();
            if (name.compare(kStrGlslLineEdit) == 0)
            {
                line_edit->setText(QString::fromStdString(initial_settings_.input_file_ext_glsl));
            }
            else if (name.compare(kStrHlslLineEdit) == 0)
            {
                line_edit->setText(QString::fromStdString(initial_settings_.input_file_ext_hlsl));
            }
            else if (name.compare(kStrSpvBinLineEdit) == 0)
            {
                line_edit->setText(QString::fromStdString(initial_settings_.input_file_ext_spv_bin));
            }
            else if (name.compare(kStrSpvTextLineEdit) == 0)
            {
                line_edit->setText(QString::fromStdString(initial_settings_.input_file_ext_spv_txt));
            }

            // Display an error message.
            RgUtils::ShowErrorMessageBox(kStrValueBlankMessage, this);
        }
    }
}

void RgGlobalSettingsView::HandlePendingChangesStateChanged(bool has_pending_changes)
{
    // Let the base class determine if there is a need to signal listeners
    // about the pending changes state.
    RgBuildSettingsView::SetHasPendingChanges(has_pending_changes);
}

void RgGlobalSettingsView::HandleLogFileLocationBrowseButtonClick(bool /* checked */)
{
    // Get the current log file location.
    QString current_log_file_location = QString::fromStdString(initial_settings_.log_file_location);

    // Open up a directory chooser.
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setDirectory(current_log_file_location);
    if (dialog.exec())
    {
        QStringList selected_dir = dialog.selectedFiles();

        // Update the text box.
        QString selectedDirectory = selected_dir.at(0);
        if (!selectedDirectory.isEmpty())
        {
            ui_.logFileLocationLineEdit->setText(selectedDirectory);

            // Signal to any listeners that the values in the UI have changed.
            HandlePendingChangesStateChanged(GetHasPendingChanges());
        }
    }
}

void RgGlobalSettingsView::HandleProjectFileLocationBrowseButtonClick(bool /* checked */)
{
    // Get the current project file location.
    QString current_project_file_location = QString::fromStdString(initial_settings_.project_file_location);

    // Open up a directory chooser.
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setDirectory(current_project_file_location);
    if (dialog.exec())
    {
        QStringList selected_dir = dialog.selectedFiles();

        // Update the text box.
        QString selected_directory = selected_dir.at(0);
        if (!selected_directory.isEmpty())
        {
            ui_.projectFileLocationLineEdit->setText(selected_directory);

            // Signal to any listeners that the values in the UI have changed.
            HandlePendingChangesStateChanged(GetHasPendingChanges());
        }
    }
}

void RgGlobalSettingsView::HandleIncludeFilesViewerBrowseButtonClick(bool checked)
{
    Q_UNUSED(checked);

    // Get the current viewer.
    QString current_viewer = QString::fromStdString(initial_settings_.log_file_location);

#ifdef _WIN32
    const char* kFileFilter = "Executable files (*.exe)|*.exe";
#else
    const char* kFileFilter = "";
#endif

    // Open up a file dialog.
    std::string selected_viewer_path;
    bool        is_selected =
        RgUtils::OpenFileDialog(this, selected_viewer_path, kStrFileDialogFilterIncludeViewer, kFileFilter) && RgUtils::IsFileExists(selected_viewer_path);

    // If the user selected a path, and it is a valid one, process this request.
    if (is_selected)
    {
        // Set the selected file path as the text in the text box.
        ui_.includeFilesViewerLineEdit->setText(selected_viewer_path.c_str());

        // Signal to any listeners that the values in the UI have changed.
        HandlePendingChangesStateChanged(GetHasPendingChanges());
    }
}

void RgGlobalSettingsView::SetCursor()
{
    // Set the cursor to pointing hand cursor.
    ui_.columnVisibilityArrowPushButton->setCursor(Qt::PointingHandCursor);
    ui_.logFileLocationFolderOpenButton->setCursor(Qt::PointingHandCursor);
    ui_.projectNameCheckBox->setCursor(Qt::PointingHandCursor);
    ui_.defaultApiOnStartupComboBox->setCursor(Qt::PointingHandCursor);
    ui_.fontFamilyComboBox->setCursor(Qt::PointingHandCursor);
    ui_.fontSizeComboBox->setCursor(Qt::PointingHandCursor);
    ui_.includeFilesViewerBrowseButton->setCursor(Qt::PointingHandCursor);
    ui_.colorThemeComboBox->setCursor(Qt::PointingHandCursor);
}

void RgGlobalSettingsView::HandleColumnVisibilityComboBoxItemClicked(QCheckBox* check_box)
{
    const QString& text = check_box->text();
    bool checked = check_box->isChecked();

    // Since the combo box must have at least one column visible (aka one column selected),
    // that suggests that deselecting "All" should be impossible, since that would imply that none
    // of the columns are visible. To handle this, the "All" checkbox will be disabled when/if all the options are selected.
    // Note that the ArrowIconComboBox will automatically handle selecting all the items when the "All" option is selected.
    // The "All" option can be re-enabled whenever one of the other options has been unchecked.

    // Get the column visibility state.
    int               first_column      = static_cast<int>(IsaItemModel::Columns::kPcAddress);
    int               last_column       = static_cast<int>(RgIsaItemModel::Columns::kColumnCount);
    std::vector<bool> column_visibility = RgUtils::GetColumnVisibilityCheckboxes(ui_.columnVisibilityArrowPushButton);
    if (checked == true)
    {
        // Disable the "All" option if all of the items are checked.
        bool all_checked = std::all_of(column_visibility.begin(),
                                       column_visibility.begin() + (last_column - first_column),
                                       [](bool b) { return b == true; });
        if (all_checked)
        {
            QListWidgetItem* item           = ui_.columnVisibilityArrowPushButton->FindItem(0);
            QCheckBox*       check_box_item = static_cast<QCheckBox*>(item->listWidget()->itemWidget(item));
            check_box_item->setDisabled(true);
        }
    }
    else
    {
        {
            // Re-enable "All" since an item was unchecked.
            QListWidgetItem* item      = ui_.columnVisibilityArrowPushButton->FindItem(0);
            QCheckBox*       check_box_item = static_cast<QCheckBox*>(item->listWidget()->itemWidget(item));
            check_box_item->setDisabled(false);
        }

        // Make sure that at least one item is still enabled.
        bool is_at_least_one_checked = std::any_of(column_visibility.begin(),
                                                   column_visibility.begin() + (last_column - first_column),
                                                   [](bool b) { return b == true; });
        if (!is_at_least_one_checked)
        {
            // The user tried to uncheck the last check box, but at least one box
            // MUST be checked, so find that item in the ListWidget, and set it back to checked.
            for (int row = 0; row < ui_.columnVisibilityArrowPushButton->RowCount(); row++)
            {
                QListWidgetItem* item           = ui_.columnVisibilityArrowPushButton->FindItem(row);
                QCheckBox*       check_box_item = static_cast<QCheckBox*>(item->listWidget()->itemWidget(item));
                QString          check_box_text = check_box_item->text();
                if (check_box_text.compare(text) == 0)
                {
                    check_box_item->setChecked(true);
                }
            }
        }
    }

    // Signal to any listeners that the values in the UI have changed.
    HandlePendingChangesStateChanged(GetHasPendingChanges());
}

void RgGlobalSettingsView::PopulateColumnVisibilityList()
{
    // Remove the existing items first.
    ui_.columnVisibilityArrowPushButton->ClearItems();

    // Add the "All" entry.
    QCheckBox* all_check_box = ui_.columnVisibilityArrowPushButton->AddCheckboxItem(kStrDisassemblyColumnAll, QVariant(), false, true);
    all_check_box->setObjectName(kStrGlobalSettingsColumnListItemAllCheckbox);

    ui_.columnVisibilityArrowPushButton->AddCheckboxItem(IsaItemModel::kColumnNames[IsaItemModel::Columns::kPcAddress].c_str(), QVariant(), false, false);
    ui_.columnVisibilityArrowPushButton->AddCheckboxItem(IsaItemModel::kColumnNames[IsaItemModel::Columns::kOpCode].c_str(), QVariant(), false, false);
    ui_.columnVisibilityArrowPushButton->AddCheckboxItem(IsaItemModel::kColumnNames[IsaItemModel::Columns::kOperands].c_str(), QVariant(), false, false);
    ui_.columnVisibilityArrowPushButton->AddCheckboxItem(
        IsaItemModel::kColumnNames[IsaItemModel::Columns::kBinaryRepresentation].c_str(), QVariant(), false, false);
    ui_.columnVisibilityArrowPushButton->AddCheckboxItem(
        RgIsaItemModel::kColumnNames[RgIsaItemModel::Columns::kIsaColumnVgprPressure - IsaItemModel::Columns::kColumnCount].c_str(), QVariant(), false, false);
    ui_.columnVisibilityArrowPushButton->AddCheckboxItem(
        RgIsaItemModel::kColumnNames[RgIsaItemModel::Columns::kIsaColumnFunctionalUnit - IsaItemModel::Columns::kColumnCount].c_str(),
        QVariant(),
        false,
        false);
    
    // Get notified when checkboxes are changed.
    bool is_connected = connect(
        ui_.columnVisibilityArrowPushButton, &ArrowIconComboBox::CheckboxChanged, this, &RgGlobalSettingsView::HandleColumnVisibilityComboBoxItemClicked);
    Q_ASSERT(is_connected);

    // If the initial settings are different than the global settings, then we know the checkbox values were
    // changed in another view.
    std::shared_ptr<RgGlobalSettings> settings = RgConfigManager::Instance().GetGlobalConfig();
    if (initial_settings_.visible_disassembly_view_columns != settings->visible_disassembly_view_columns)
    {
        if (settings != nullptr)
        {
            RgUtils::SetColumnVisibilityCheckboxes(ui_.columnVisibilityArrowPushButton, settings->visible_disassembly_view_columns);
        }
    }
}

void RgGlobalSettingsView::PopulateFontSizeDropdown()
{
    const std::vector<int> kFontSizes = {8, 9, 10, 11, 12, 14, 16, 18, 20, 22, 24, 26, 28, 36, 48, 72};
    for (auto font_size : kFontSizes)
    {
        ui_.fontSizeComboBox->addItem(QString::number(font_size), QString::number(font_size));
    }
}

void RgGlobalSettingsView::CreateColumnVisibilityControls()
{
    // Setup the list widget that opens when the user clicks the column visibility arrow.
    ui_.columnVisibilityArrowPushButton->InitMultiSelect(parent_, "Columns");
}

void RgGlobalSettingsView::HandleProjectNameCheckboxStateChanged(int checked)
{
    Q_UNUSED(checked);

    // Signal to any listeners that the values in the UI have changed.
    HandlePendingChangesStateChanged(GetHasPendingChanges());
}

void RgGlobalSettingsView::SetCheckboxToolTip(const std::string& text)
{
    ui_.projectNameCheckBox->setToolTip(text.c_str());
}

void RgGlobalSettingsView::HandleLogFileEditingFinished()
{
    // Verify that the line edit text is not empty or the location is not invalid before losing the focus.
    if (this->ui_.logFileLocationLineEdit->text().isEmpty() || !RgUtils::IsDirExists(this->ui_.logFileLocationLineEdit->text().toStdString()))
    {
        // If empty or invalid, use the saved location.
        std::shared_ptr<RgGlobalSettings> global_config = RgConfigManager::Instance().GetGlobalConfig();
        assert(global_config != nullptr);
        if (global_config != nullptr)
        {
            this->ui_.logFileLocationLineEdit->setText(global_config->log_file_location.c_str());
        }
        this->ui_.logFileLocationLineEdit->setFocus();
    }

    // Signal to any listeners that the values in the UI have changed.
    HandlePendingChangesStateChanged(GetHasPendingChanges());
}

void RgGlobalSettingsView::HandleProjectFileEditingFinished()
{
    // Verify that the line edit text is not empty or the location is not invalid before losing the focus.
    if (this->ui_.projectFileLocationLineEdit->text().isEmpty() || !RgUtils::IsDirExists(this->ui_.projectFileLocationLineEdit->text().toStdString()))
    {
        // If empty or invalid, use the saved location.
        std::shared_ptr<RgGlobalSettings> global_config = RgConfigManager::Instance().GetGlobalConfig();
        assert(global_config != nullptr);
        if (global_config != nullptr)
        {
            this->ui_.projectFileLocationLineEdit->setText(global_config->project_file_location.c_str());
        }
        this->ui_.projectFileLocationLineEdit->setFocus();
    }

    // Signal to any listeners that the values in the UI have changed.
    HandlePendingChangesStateChanged(GetHasPendingChanges());
}

void RgGlobalSettingsView::SetEditBoxToolTips()
{
    ui_.logFileLocationLineEdit->setToolTip(ui_.logFileLocationLineEdit->displayText());
    ui_.projectFileLocationLineEdit->setToolTip(ui_.projectFileLocationLineEdit->displayText());
}

void RgGlobalSettingsView::HandleLogFileEditBoxChanged(const QString& text)
{
    // Update the tooltip.
    ui_.logFileLocationLineEdit->setToolTip(text);

    // Restore the cursor to the original position when the text has changed.
    QtCommon::QtUtils::RestoreCursorPosition cursor_position(ui_.logFileLocationLineEdit);

    // Signal to any listeners that the values in the UI have changed.
    HandlePendingChangesStateChanged(GetHasPendingChanges());
}

void RgGlobalSettingsView::HandleProjectFileEditBoxChanged(const QString& text)
{
    // Update the tooltip.
    ui_.projectFileLocationLineEdit->setToolTip(text);

    // Restore the cursor to the original position when the text has changed.
    QtCommon::QtUtils::RestoreCursorPosition cursor_position(ui_.projectFileLocationLineEdit);

    // Signal to any listeners that the values in the UI have changed.
    HandlePendingChangesStateChanged(GetHasPendingChanges());
}

void RgGlobalSettingsView::HandleTextBoxChanged(const QString& text)
{
    Q_UNUSED(text);

    // Figure out the sender and process appropriately.
    QObject* sender = QObject::sender();
    assert(sender != nullptr);

    RgLineEdit* line_edit = static_cast<RgLineEdit*>(sender);
    assert(line_edit != nullptr);

    // Signal whether the text box is empty or not.
    if (line_edit != nullptr)
    {
        QString text_value = line_edit->text();
        if (text_value.isEmpty())
        {
            emit InputFileNameBlankSignal(true);
        }
        else
        {
            emit InputFileNameBlankSignal(false);
        }
    }

    // Signal to any listeners that the values in the UI have changed.
    HandlePendingChangesStateChanged(GetHasPendingChanges());
}

void RgGlobalSettingsView::HandleComboBoxChanged(int index)
{
    Q_UNUSED(index);

    // Signal to any listeners that the values in the UI have changed.
    HandlePendingChangesStateChanged(GetHasPendingChanges());
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
void RgGlobalSettingsView::HandleOsColorSchemeChanged(Qt::ColorScheme color_scheme)
{
    if (RgConfigManager::Instance().GetGlobalConfig()->color_theme != kColorThemeTypeCount)
    {
        return;
    }

    if (color_scheme == Qt::ColorScheme::Unknown)
    {
        return;
    }

    ColorThemeType color_mode = kColorThemeTypeLight;
    if (color_scheme == Qt::ColorScheme::Light)
    {
        color_mode = kColorThemeTypeLight;
    }
    else if (color_scheme == Qt::ColorScheme::Dark)
    {
        color_mode = kColorThemeTypeDark;
    }

    if (color_mode == QtCommon::QtUtils::ColorTheme::Get().GetColorTheme())
    {
        return;
    }

    QtCommon::QtUtils::ColorTheme::Get().SetColorTheme(color_mode);

    qApp->setPalette(QtCommon::QtUtils::ColorTheme::Get().GetCurrentPalette());

    emit QtCommon::QtUtils::ColorTheme::Get().ColorThemeUpdated();
}
#endif

void RgGlobalSettingsView::HandleColorThemeComboBoxChanged(QListWidgetItem* color_theme_option)
{
    ColorThemeType selected_color_mode = static_cast<ColorThemeType>(color_theme_option->data(Qt::UserRole).toInt());

    // If the setting was not changed, return early.
    if (selected_color_mode == RgConfigManager::Instance().GetGlobalConfig()->color_theme)
    {
        return;
    }

    HandlePendingChangesStateChanged(GetHasPendingChanges());
}

bool RgGlobalSettingsView::SetColorTheme()
{
    ColorThemeType selected_color_mode = static_cast<ColorThemeType>(initial_settings_.color_theme);

    // If the setting was not changed return early.
    if (selected_color_mode == QtCommon::QtUtils::ColorTheme::Get().GetColorTheme())
    {
        return false;
    }

    ColorThemeType color_mode = selected_color_mode;

    if (selected_color_mode == kColorThemeTypeCount)
    {
        color_mode = QtCommon::QtUtils::DetectOsSetting();
    }

    // If the setting was changed, but won't result in a color theme change return early.
    if (color_mode == QtCommon::QtUtils::ColorTheme::Get().GetColorTheme())
    {
        return false;
    }

    
    QString color_theme_changed_title = "Color Theme Changed. Restart Application?";
    QString color_theme_changed_text =
        "Not all UI elements will update to reflect the change in color theme until the application has restarted. Restart Application?";

    int ret = QtCommon::QtUtils::ShowMessageBox(
        this, QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Question, color_theme_changed_title, color_theme_changed_text);

    bool restart = false;

    if (ret == QMessageBox::Cancel)
    {
        int old_color_theme = RgConfigManager::Instance().GetGlobalConfig()->color_theme;

        for (int i = 0; i <= kColorThemeTypeCount; i++)
        {
            if (i == old_color_theme)
            {
                ui_.colorThemeComboBox->SetSelectedRow(i);
            }
        }
        initial_settings_.color_theme = old_color_theme;
    }
    else
    {
        QtCommon::QtUtils::ColorTheme::Get().SetColorTheme(color_mode);
    
        if (ret == QMessageBox::Yes)
        {
            restart = true;
        }
        else if (ret == QMessageBox::No)
        {
            qApp->setPalette(QtCommon::QtUtils::ColorTheme::Get().GetCurrentPalette());

            emit QtCommon::QtUtils::ColorTheme::Get().ColorThemeUpdated();
        }
    }
    return restart;
}

bool RgGlobalSettingsView::GetHasPendingChanges() const
{
    RgGlobalSettings current_settings = PullFromWidgets();

    bool has_changes = !initial_settings_.HasSameSettings(current_settings);

    return has_changes;
}

bool RgGlobalSettingsView::RevertPendingChanges()
{
    std::shared_ptr<RgGlobalSettings> settings = RgConfigManager::Instance().GetGlobalConfig();
    assert(settings != nullptr);
    if (settings != nullptr)
    {
        PushToWidgets(*settings);
    }

    // Make sure the rest of the UI knows that the settings don't need to be saved.
    HandlePendingChangesStateChanged(false);

    return false;
}

void RgGlobalSettingsView::RestoreDefaultSettings()
{
    // Reset our initial settings back to the defaults
    RgConfigManager::Instance().ResetToFactoryDefaults(initial_settings_);

    // Update the UI to reflect the new initial settings
    PushToWidgets(initial_settings_);

    // Update the ConfigManager to use the new settings.
    RgConfigManager::Instance().SetGlobalConfig(initial_settings_);

    // Save the settings file.
    RgConfigManager::Instance().SaveGlobalConfigFile();

    // Signal to any listeners that the values in the UI have changed.
    HandlePendingChangesStateChanged(GetHasPendingChanges());
}

bool RgGlobalSettingsView::SaveSettings()
{
    // Reset the initial settings to match what the UI shows.
    initial_settings_ = PullFromWidgets();

    bool restart = SetColorTheme();

    // Update the config manager to use these new settings.
    RgConfigManager& config_manager = RgConfigManager::Instance();
    config_manager.SetGlobalConfig(initial_settings_);

    // Save the settings.
    bool is_saved = config_manager.SaveGlobalConfigFile();

    if (is_saved)
    {
        // Make sure the rest of the UI knows that the settings have been saved.
        HandlePendingChangesStateChanged(false);
    }

    if (restart)
    {
        QString default_exe_name;
        default_exe_name.append(QDir::separator());
        default_exe_name.append(kStrAppFolderName);

#ifdef WIN32
        // Append an extension only in Windows.
        default_exe_name.append(".exe");
#endif

        const QString rga_executable = QDir::toNativeSeparators(qApp->applicationDirPath() + default_exe_name);

        QFileInfo file(rga_executable);
        if (file.exists())
        {
            QProcess* process = new QProcess(this);
            if (process != nullptr)
            {
                bool process_result = process->startDetached(rga_executable);
                if (process_result)
                {
                    qApp->quit();
                }
            }
        }
    }

    return is_saved;
}

std::string RgGlobalSettingsView::GetTitleString()
{
    std::string title_string = kStrSettingsConfirmationApplicationSettings;

    return title_string;
}

void RgGlobalSettingsView::HandleFontFamilyChanged(const QFont& font)
{
    Q_UNUSED(font);

    // Signal to any listeners that the values in the UI have changed.
    HandlePendingChangesStateChanged(GetHasPendingChanges());
}

void RgGlobalSettingsView::HandleIncludeFilesViewerEditingFinished()
{
    // Verify that the line edit text is not empty or the location is not invalid before losing the focus.
    if (this->ui_.includeFilesViewerLineEdit->text().isEmpty())
    {
        this->ui_.includeFilesViewerLineEdit->setText(kStrGlobalSettingsSrcViewIncludeViewerDefault);
    }
    else if (this->ui_.includeFilesViewerLineEdit->text().compare(kStrGlobalSettingsSrcViewIncludeViewerDefault) != 0 &&
             !RgUtils::IsFileExists(this->ui_.includeFilesViewerLineEdit->text().toStdString()))
    {
        // If empty or invalid, use the saved location.
        std::shared_ptr<RgGlobalSettings> global_config = RgConfigManager::Instance().GetGlobalConfig();
        assert(global_config != nullptr);
        if (global_config != nullptr)
        {
            this->ui_.includeFilesViewerLineEdit->setText(global_config->include_files_viewer.c_str());
        }
        this->ui_.includeFilesViewerLineEdit->setFocus();
    }

    // Signal to any listeners that the values in the UI have changed.
    HandlePendingChangesStateChanged(GetHasPendingChanges());
}

void RgGlobalSettingsView::SetInitialWidgetFocus()
{
    ui_.logFileLocationLineEdit->setFocus();
}

bool RgGlobalSettingsView::IsInputFileBlank() const
{
    return ui_.assocExtGlslLineEdit->text().isEmpty() || ui_.assocExtHlslLineEdit->text().isEmpty() || ui_.assocExtSpvasLineEdit->text().isEmpty() ||
           ui_.assocExtSpvBinaryLineEdit->text().isEmpty();
}

ArrowIconComboBox* RgGlobalSettingsView::GetColumnVisibilityComboBox()
{
    return ui_.columnVisibilityArrowPushButton;
}


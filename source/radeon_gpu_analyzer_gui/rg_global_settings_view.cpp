// C++.
#include <cassert>
#include <sstream>

// Qt.
#include <QApplication>
#include <QCheckBox>
#include <QFileDialog>
#include <QFocusEvent>
#include <QMessageBox>
#include <QWidget>

// Infra.
#include "QtCommon/CustomWidgets/ArrowIconWidget.h"
#include "QtCommon/Util/CommonDefinitions.h"
#include "QtCommon/CustomWidgets/ListWidget.h"
#include "QtCommon/Util/QtUtil.h"
#include "QtCommon/Util/RestoreCursorPosition.h"

// Local.
#include "radeon_gpu_analyzer_gui/rg_config_manager.h"
#include "radeon_gpu_analyzer_gui/rg_definitions.h"
#include "radeon_gpu_analyzer_gui/qt/rg_global_settings_view.h"
#include "radeon_gpu_analyzer_gui/qt/rg_hide_list_widget_event_filter.h"
#include "radeon_gpu_analyzer_gui/qt/rg_isa_disassembly_table_model.h"
#include "radeon_gpu_analyzer_gui/qt/rg_line_edit.h"
#include "radeon_gpu_analyzer_gui/rg_string_constants.h"
#include "radeon_gpu_analyzer_gui/rg_utils.h"

// Names of special widgets within the build settings view.
static const char* kStrGlobalSettingsColumnVisibilityList      = "DisassemblyColumnVisibilityList";
static const char* kStrGlobalSettingsColumnListItemCheckbox    = "ListWidgetCheckBox";
static const char* kStrGlobalSettingsColumnListItemAllCheckbox = "ListWidgetAllCheckBox";

// API-specific colors.
static const QColor kApiColorOpencl = QColor(18, 152, 0);
static const QColor kApiColorVulkan = QColor(224, 30, 55);

// Columns push button font size.
const int kPushButtonFontSize = 11;

// The maximum font size allowed in the font size combo box.
static const int kMaxFontSize = 50;

RgGlobalSettingsView::RgGlobalSettingsView(QWidget* parent, const RgGlobalSettings& global_settings)
    : RgBuildSettingsView(parent, true)
    , initial_settings_(global_settings)
    , parent_(parent)
{
    // Setup the UI.
    ui_.setupUi(this);

    // Set the background to white.
    QPalette pal = palette();
    pal.setColor(QPalette::Background, Qt::white);
    this->setAutoFillBackground(true);
    this->setPalette(pal);

    // Create the widget used to control column visibility.
    CreateColumnVisibilityControls();

    // Fill the column visibility dropdown with all of the possible column names.
    PopulateColumnVisibilityList();

    // Initialize the combo box for font size.
    for (int i = 1; i < kMaxFontSize; i++)
    {
        ui_.fontSizeComboBox->addItem(QString::number(i), QString::number(i));
    }

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

    // Set the fonts for the list widget push button.
    QFont  font         = ui_.columnVisibilityArrowPushButton->font();
    double scale_factor = ScalingManager::Get().GetScaleFactor();
    font.setPointSize(kButtonPointFontSize * scale_factor);
    ui_.columnVisibilityArrowPushButton->setFont(font);

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
    }
    else if (current_api == RgProjectAPI::kVulkan)
    {
        ui_.columnVisibilityArrowPushButton->SetBorderColor(kApiColorVulkan);
        ui_.columnVisibilityArrowPushButton->SetShowBorder(true);
    }
    else
    {
        // Should not get here.
        assert(false);
    }
}

RgGlobalSettingsView::~RgGlobalSettingsView()
{
    // Remove the column dropdown focus event filters if they exist.
    if (disassembly_columns_list_widget_ != nullptr && disassembly_columns_list_event_filter_ != nullptr)
    {
        disassembly_columns_list_widget_->removeEventFilter(disassembly_columns_list_event_filter_);
        qApp->removeEventFilter(disassembly_columns_list_event_filter_);
    }
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

    ListWidget::SetColumnVisibilityCheckboxes(disassembly_columns_list_widget_, global_settings.visible_disassembly_view_columns);

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
    settings.visible_disassembly_view_columns = ListWidget::GetColumnVisibilityCheckboxes(disassembly_columns_list_widget_);

    // Use default project names.
    settings.use_default_project_name = ui_.projectNameCheckBox->isChecked();

    // Default API.
    int default_api_combo_box_index = ui_.defaultApiOnStartupComboBox->currentIndex();
    if (default_api_combo_box_index >= 0 && default_api_combo_box_index < static_cast<int>(RgProjectAPI::kApiCount))
    {
        settings.default_api = static_cast<RgProjectAPI>(default_api_combo_box_index);

        settings.should_prompt_for_api = (default_api_combo_box_index == static_cast<int>(RgProjectAPI::kUnknown));
    }

    // Font family.
    QFont font           = ui_.fontFamilyComboBox->currentFont();
    settings.font_family = font.family().toStdString();

    // Font size.
    settings.font_size = ui_.fontSizeComboBox->itemData(ui_.fontSizeComboBox->currentIndex()).toInt();

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

    // Columns check box list widget button.
    is_connected = connect(this->ui_.columnVisibilityArrowPushButton, &QPushButton::clicked, this, &RgGlobalSettingsView::HandleViewColumnsButtonClick);
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

void RgGlobalSettingsView::HandleViewColumnsButtonClick(bool /* checked */)
{
    // Make the list widget appear and process user selection from the list widget.
    bool visible = disassembly_columns_list_widget_->isVisible();
    if (visible)
    {
        disassembly_columns_list_widget_->hide();

        // Change the up arrow to a down arrow.
        ui_.columnVisibilityArrowPushButton->SetDirection(ArrowIconWidget::Direction::DownArrow);
    }
    else
    {
        // If the initial settings are different than the global settings, then we know the checkbox values were
        // changed in another view.
        std::shared_ptr<RgGlobalSettings> settings = RgConfigManager::Instance().GetGlobalConfig();
        if (initial_settings_.visible_disassembly_view_columns != settings->visible_disassembly_view_columns)
        {
            if (settings != nullptr)
            {
                ListWidget::SetColumnVisibilityCheckboxes(disassembly_columns_list_widget_, settings->visible_disassembly_view_columns);
            }
        }

        // Compute where to place the combo box relative to where the arrow button is.
        QWidget* widget = ui_.columnVisibilityArrowPushButton;
        disassembly_columns_list_widget_->show();
        QRect  rect = widget->geometry();
        QPoint pos(0, 0);
        pos = widget->mapTo(this, pos);
        pos.setY(pos.y() + rect.height());
        int height = QtCommon::QtUtil::GetListWidgetHeight(disassembly_columns_list_widget_);
        int width  = QtCommon::QtUtil::GetListWidgetWidth(disassembly_columns_list_widget_);
        disassembly_columns_list_widget_->setGeometry(pos.x(), pos.y(), width + s_CHECK_BOX_WIDTH, height);

        // Change the down arrow to an up arrow.
        ui_.columnVisibilityArrowPushButton->SetDirection(ArrowIconWidget::Direction::UpArrow);
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
}

void RgGlobalSettingsView::HandleColumnVisibilityComboBoxItemClicked(const QString& text, bool checked)
{
    int first_column = RgIsaDisassemblyTableModel::GetTableColumnIndex(RgIsaDisassemblyTableColumns::kAddress);
    int last_column  = RgIsaDisassemblyTableModel::GetTableColumnIndex(RgIsaDisassemblyTableColumns::kCount);

    std::vector<bool> column_visibility = ListWidget::GetColumnVisibilityCheckboxes(disassembly_columns_list_widget_);

    // Make sure at least one check box is still checked.
    bool is_at_least_one_checked =
        std::any_of(column_visibility.begin() + first_column, column_visibility.begin() + last_column, [](bool b) { return b == true; });

    if (checked || is_at_least_one_checked)
    {
        // If the user checked the "All" option, Step through each column and set to visible.
        if (text.compare(kStrDisassemblyTableColumnAll) == 0 && (checked == true))
        {
            for (int column_index = first_column; column_index < last_column; ++column_index)
            {
                column_visibility[column_index] = checked;
            }

            // Update the state of the dropdown check boxes to reflect that all options are checked.
            ListWidget::SetColumnVisibilityCheckboxes(disassembly_columns_list_widget_, column_visibility);
        }
    }
    else
    {
        // The user tried to uncheck the last check box, but at least one box
        // MUST be checked, so find that item in the ListWidget, and set it back to checked.
        for (int row = 0; row < disassembly_columns_list_widget_->count(); row++)
        {
            QListWidgetItem* item           = disassembly_columns_list_widget_->item(row);
            QCheckBox*       check_box      = (QCheckBox*)disassembly_columns_list_widget_->itemWidget(item);
            QString          check_box_text = check_box->text();
            if (check_box_text.compare(text) == 0)
            {
                QCheckBox* check_box = (QCheckBox*)disassembly_columns_list_widget_->itemWidget(item);
                check_box->setChecked(true);
            }
        }
    }

    // See if the "All" box needs checking/un-checking.
    ListWidget::UpdateAllCheckbox(disassembly_columns_list_widget_);

    // Update the "All" checkbox text color to grey or black.
    UpdateAllCheckBoxText();

    // Signal to any listeners that the values in the UI have changed.
    HandlePendingChangesStateChanged(GetHasPendingChanges());
}

void RgGlobalSettingsView::HandleColumnVisibilityFilterStateChanged(bool checked)
{
    // Figure out the sender and process appropriately.
    QObject* sender = QObject::sender();
    assert(sender != nullptr);

    // Find out which entry caused the signal.
    QWidget* item = qobject_cast<QWidget*>(sender);
    assert(item != nullptr);

    QCheckBox* check_box = qobject_cast<QCheckBox*>(sender);
    assert(check_box != nullptr);

    // Process the click.
    if (check_box != nullptr)
    {
        HandleColumnVisibilityComboBoxItemClicked(check_box->text(), checked);
    }
}

std::string RgGlobalSettingsView::GetDisassemblyColumnName(RgIsaDisassemblyTableColumns column) const
{
    std::string result;

    static std::map<RgIsaDisassemblyTableColumns, std::string> column_name_map = {
        {RgIsaDisassemblyTableColumns::kAddress, kStrDisassemblyTableColumnAddress},
        {RgIsaDisassemblyTableColumns::kOpcode, kStrDisassemblyTableColumnOpcode},
        {RgIsaDisassemblyTableColumns::kOperands, kStrDisassemblyTableColumnOperands},
        {RgIsaDisassemblyTableColumns::kFunctionalUnit, kStrDisassemblyTableColumnFunctionalUnit},
        {RgIsaDisassemblyTableColumns::kCycles, kStrDisassemblyTableColumnCycles},
        {RgIsaDisassemblyTableColumns::kBinaryEncoding, kStrDisassemblyTableColumnBinaryEncoding},
        {RgIsaDisassemblyTableColumns::kLiveVgprs, kStrDisassemblyTableLiveVgprHeaderPart},
    };

    auto column_name_iter = column_name_map.find(column);
    if (column_name_iter != column_name_map.end())
    {
        result = column_name_iter->second;
    }
    else
    {
        // The incoming column doesn't have a name string mapped to it.
        assert(false);
    }

    return result;
}

void RgGlobalSettingsView::PopulateColumnVisibilityList()
{
    // Set up the function pointer responsible for handling column visibility filter state change.
    using std::placeholders::_1;
    std::function<void(bool)> slot_function_pointer = std::bind(&RgGlobalSettingsView::HandleColumnVisibilityFilterStateChanged, this, _1);

    // Remove the existing items first.
    ClearListWidget(disassembly_columns_list_widget_);

    // Add the "All" entry.
    ListWidget::AddListWidgetCheckboxItem(kStrDisassemblyTableColumnAll,
                                          disassembly_columns_list_widget_,
                                          slot_function_pointer,
                                          this,
                                          kStrGlobalSettingsColumnVisibilityList,
                                          kStrGlobalSettingsColumnListItemAllCheckbox);

    // Loop through each column enum member.
    int start_column = RgIsaDisassemblyTableModel::GetTableColumnIndex(RgIsaDisassemblyTableColumns::kAddress);
    int end_column   = RgIsaDisassemblyTableModel::GetTableColumnIndex(RgIsaDisassemblyTableColumns::kCount);

    // Add an item for each column in the table.
    for (int column_index = start_column; column_index < end_column; ++column_index)
    {
        // Add an item for each possible column in the table.
        std::string column_name = GetDisassemblyColumnName(static_cast<RgIsaDisassemblyTableColumns>(column_index));
        ListWidget::AddListWidgetCheckboxItem(column_name.c_str(),
                                              disassembly_columns_list_widget_,
                                              slot_function_pointer,
                                              this,
                                              kStrGlobalSettingsColumnVisibilityList,
                                              kStrGlobalSettingsColumnListItemCheckbox);
    }
}

void RgGlobalSettingsView::UpdateAllCheckBoxText()
{
    bool are_all_items_checked = true;

    // Check to see if all of the boxes are checked.
    for (int i = 1; i < disassembly_columns_list_widget_->count(); i++)
    {
        QListWidgetItem* item = disassembly_columns_list_widget_->item(i);
        assert(item != nullptr);

        QCheckBox* check_box = qobject_cast<QCheckBox*>(disassembly_columns_list_widget_->itemWidget(item));
        assert(check_box != nullptr);

        if (check_box->checkState() == Qt::CheckState::Unchecked)
        {
            are_all_items_checked = false;
            break;
        }
    }

    // If all boxes are checked, update the text color of the All check box.
    QListWidgetItem* item = disassembly_columns_list_widget_->item(0);
    if (item != nullptr)
    {
        QCheckBox* check_box = qobject_cast<QCheckBox*>(disassembly_columns_list_widget_->itemWidget(item));
        if (check_box != nullptr)
        {
            if (are_all_items_checked)
            {
                check_box->setStyleSheet("QCheckBox#ListWidgetAllCheckBox {color: grey;}");
            }
            else
            {
                check_box->setStyleSheet("QCheckBox#ListWidgetAllCheckBox {color: black;}");
            }
        }
    }
}

void RgGlobalSettingsView::CreateColumnVisibilityControls()
{
    // Setup the list widget that opens when the user clicks the column visibility arrow.
    RgUtils::SetupComboList(parent_, disassembly_columns_list_widget_, ui_.columnVisibilityArrowPushButton, disassembly_columns_list_event_filter_, false);
    disassembly_columns_list_widget_->setObjectName(kStrGlobalSettingsColumnVisibilityList);

    // Update scale factor for widgets.
    QFont  font         = ui_.columnVisibilityArrowPushButton->font();
    double scale_factor = ScalingManager::Get().GetScaleFactor();
    font.setPointSize(kPushButtonFontSize * scale_factor);
    disassembly_columns_list_widget_->setStyleSheet(s_LIST_WIDGET_STYLE.arg(font.pointSize()));

    // Reset the current selection in the column visibility list.
    disassembly_columns_list_widget_->setCurrentRow(0);
}

void RgGlobalSettingsView::HandleProjectNameCheckboxStateChanged(int checked)
{
    Q_UNUSED(checked);

    // Signal to any listeners that the values in the UI have changed.
    HandlePendingChangesStateChanged(GetHasPendingChanges());
}

void RgGlobalSettingsView::ClearListWidget(ListWidget*& list_widget)
{
    assert(list_widget != nullptr);

    // Disconnect slot/signal connection for each check box
    for (int row = 0; row < list_widget->count(); row++)
    {
        QListWidgetItem* item      = list_widget->item(row);
        QCheckBox*       check_box = (QCheckBox*)list_widget->itemWidget(item);

        if (list_widget->objectName().compare(kStrGlobalSettingsColumnVisibilityList) == 0)
        {
            bool is_disconnected = disconnect(check_box, &QCheckBox::clicked, this, &RgGlobalSettingsView::HandleColumnVisibilityFilterStateChanged);
            assert(is_disconnected);
        }
        else
        {
            assert(false);
        }
    }

    // Clear the list widget. This also deletes each item.
    list_widget->clear();
}

void RgGlobalSettingsView::CloseListWidget()
{
    disassembly_columns_list_widget_->hide();
    ui_.columnVisibilityArrowPushButton->SetDirection(ArrowIconWidget::Direction::DownArrow);
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
    QtCommon::QtUtil::RestoreCursorPosition cursor_position(ui_.logFileLocationLineEdit);

    // Signal to any listeners that the values in the UI have changed.
    HandlePendingChangesStateChanged(GetHasPendingChanges());
}

void RgGlobalSettingsView::HandleProjectFileEditBoxChanged(const QString& text)
{
    // Update the tooltip.
    ui_.projectFileLocationLineEdit->setToolTip(text);

    // Restore the cursor to the original position when the text has changed.
    QtCommon::QtUtil::RestoreCursorPosition cursor_position(ui_.projectFileLocationLineEdit);

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


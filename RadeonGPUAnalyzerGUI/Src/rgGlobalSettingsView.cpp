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
#include <QtCommon/CustomWidgets/ArrowIconWidget.h>
#include <QtCommon/Util/CommonDefinitions.h>
#include <QtCommon/CustomWidgets/ListWidget.h>
#include <QtCommon/Util/QtUtil.h>
#include <QtCommon/Util/RestoreCursorPosition.h>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/rgConfigManager.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDefinitions.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgGlobalSettingsView.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgHideListWidgetEventFilter.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgIsaDisassemblyTableModel.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgLineEdit.h>
#include <RadeonGPUAnalyzerGUI/Include/rgStringConstants.h>
#include <RadeonGPUAnalyzerGUI/Include/rgUtils.h>

// Names of special widgets within the build settings view.
static const char* STR_GLOBAL_SETTINGS_COLUMN_VISIBILITY_LIST = "DisassemblyColumnVisibilityList";
static const char* STR_GLOBAL_SETTINGS_COLUMN_LIST_ITEM_CHECKBOX = "ListWidgetCheckBox";
static const char* STR_GLOBAL_SETTINGS_COLUMN_LIST_ITEM_ALL_CHECKBOX = "ListWidgetAllCheckBox";

// API-specific colors.
static const QColor s_API_COLOR_OPENCL = QColor(18, 152, 0);
static const QColor s_API_COLOR_VULKAN = QColor(135, 20, 16);

// Columns push button font size.
const int s_PUSH_BUTTON_FONT_SIZE = 11;

// The maximum font size allowed in the font size combo box.
static const int s_MAX_FONT_SIZE = 50;

rgGlobalSettingsView::rgGlobalSettingsView(QWidget* pParent, const rgGlobalSettings& globalSettings) :
    rgBuildSettingsView(pParent, true),
    m_initialSettings(globalSettings),
    m_pParent(pParent)
{
    // Setup the UI.
    ui.setupUi(this);

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
    for (int i = 1; i < s_MAX_FONT_SIZE; i++)
    {
        ui.fontSizeComboBox->addItem(QString::number(i), QString::number(i));
    }

    // Push values to various widgets.
    PushToWidgets(globalSettings);

    // Connect the signals.
    ConnectSignals();

    // Set the mouse cursor to the pointing hand cursor for various widgets.
    SetCursor();

    // Set the tool tip for default project name check box.
    SetCheckboxToolTip(STR_GLOBAL_SETTINGS_CHECKBOX_TOOLTIP);

    // Set the tool tip for the two edit boxes.
    SetEditBoxToolTips();

    // Set the tooltip for the log file location.
    ui.logFileLocationLabel->setToolTip(STR_GLOBAL_SETTINGS_LOG_FILE_LOCATION);

    // Set the tooltips for the disassembly section.
    ui.disassemblyViewLabel->setToolTip(STR_GLOBAL_SETTINGS_DISASSEMBLY_SECTION_TOOLTIP);

    // Set the tooltips for the disassembly columns.
    ui.disassemblyViewColunnsLabel->setToolTip(STR_GLOBAL_SETTINGS_DISASSEMBLY_COLUMNS_TOOLTIP);

    // Set the tooltips for the source code editor.
    ui.sourceCodeEditorLabel->setToolTip(STR_GLOBAL_SETTINGS_SRC_VIEW_SECTION_TOOLTIP);

    // Set the tooltips for the font type in the source code editor.
    ui.fontFamilyLabel->setToolTip(STR_GLOBAL_SETTINGS_SRC_VIEW_FONT_TYPE_TOOLTIP);

    // Set the tooltips for the font size in the source code editor.
    ui.fontSizeLabel->setToolTip(STR_GLOBAL_SETTINGS_SRC_VIEW_FONT_SIZE_TOOLTIP);

    // Set the tooltips for the include files viewer.
    ui.includeFilesViewerLabel->setToolTip(STR_GLOBAL_SETTINGS_SRC_VIEW_INCLUDE_FILES_VIEWER_TOOLTIP);

    // Set the tooltips for input files associations.
    ui.assocExtGlslLabel->setToolTip(STR_GLOBAL_SETTNIGS_FILE_EXT_GLSL_TOOLTIP);
    ui.assocExtHlslLabel->setToolTip(STR_GLOBAL_SETTNIGS_FILE_EXT_HLSL_TOOLTIP);
    ui.assocExtSpvasLabel->setToolTip(STR_GLOBAL_SETTNIGS_FILE_EXT_SPV_TXT_TOOLTIP);

    // Set the tooltip for the default source language.
    ui.defaultLangLabel->setToolTip(STR_GLOBAL_SETTINGS_DEFAULT_SRC_LANG_TOOLTIP);

    // Set the tooltip for the general section.
    ui.generalLabel->setToolTip(STR_GLOBAL_SETTINGS_GENERAL_SECTION_TOOLTIP);

    // Set the tooltip for the default API section.
    ui.defaultApiOnStartupLabel->setToolTip(STR_GLOBAL_SETTINGS_DEFAULT_STARTUP_API_TOOLTIP);

    // Set the tooltip for the Input Files section.
    ui.inputFilesLabel->setToolTip(STR_GLOBAL_SETTINGS_INPUT_FILES_SECTION_TOOLTIP);

    // Set the tooltip for the SPIR-V binary.
    ui.assocExtSpvBinaryLabel->setToolTip(STR_GLOBAL_SETTINGS_SPV_EXTENSIONS_TOOLTIP);

    // Set the fonts for the list widget push button.
    QFont font = ui.columnVisibilityArrowPushButton->font();
    double scaleFactor = ScalingManager::Get().GetScaleFactor();
    font.setPointSize(gs_BUTTON_POINT_FONT_SIZE * scaleFactor);
    ui.columnVisibilityArrowPushButton->setFont(font);

    // Hide HLSL components until support for HLSL is added.
    ui.assocExtHlslLabel->hide();
    ui.assocExtHlslLineEdit->hide();
    ui.defaultLangLabel->hide();
    ui.defaultLangComboBox->hide();

    // Set column visibility arrow push button properties.
    rgProjectAPI currentApi = rgConfigManager::Instance().GetCurrentAPI();
    if (currentApi == rgProjectAPI::OpenCL)
    {
        ui.columnVisibilityArrowPushButton->SetBorderColor(s_API_COLOR_OPENCL);
        ui.columnVisibilityArrowPushButton->SetShowBorder(true);
    }
    else if (currentApi == rgProjectAPI::Vulkan)
    {
        ui.columnVisibilityArrowPushButton->SetBorderColor(s_API_COLOR_VULKAN);
        ui.columnVisibilityArrowPushButton->SetShowBorder(true);
    }
    else
    {
        // Should not get here.
        assert(false);
    }
}

rgGlobalSettingsView::~rgGlobalSettingsView()
{
    // Remove the column dropdown focus event filters if they exist.
    if (m_pDisassemblyColumnsListWidget != nullptr && m_pDisassemblyColumnsListEventFilter != nullptr)
    {
        m_pDisassemblyColumnsListWidget->removeEventFilter(m_pDisassemblyColumnsListEventFilter);
        qApp->removeEventFilter(m_pDisassemblyColumnsListEventFilter);
    }
}

void rgGlobalSettingsView::showEvent(QShowEvent* pEvent)
{
    QWidget::showEvent(pEvent);

    // The global settings can be updated by other widgets in the application,
    // so if this view does not think it has any pending changes, then reset
    // the UI based on the config manager's global config, then update this
    // view's intitialSettings to match and signal that there are no pending changes.
    if (GetHasPendingChanges() == false)
    {
        std::shared_ptr<rgGlobalSettings> pSettings = rgConfigManager::Instance().GetGlobalConfig();
        assert(pSettings != nullptr);
        if (pSettings != nullptr)
        {
            PushToWidgets(*pSettings);
        }

        m_initialSettings = PullFromWidgets();

        HandlePendingChangesStateChanged(false);
    }
}

void rgGlobalSettingsView::PushToWidgets(const rgGlobalSettings& globalSettings)
{
    // Initialize the log file location.
    QString logFileLocation(globalSettings.m_logFileLocation.c_str());
    ui.logFileLocationLineEdit->setText(logFileLocation);

    ListWidget::SetColumnVisibilityCheckboxes(m_pDisassemblyColumnsListWidget, globalSettings.m_visibleDisassemblyViewColumns);

    // Initialize the use-generated project names checkbox.
    ui.projectNameCheckBox->setChecked(globalSettings.m_useDefaultProjectName);

    // Initialize the combo box for setting the default API.
    ui.defaultApiOnStartupComboBox->setCurrentIndex(globalSettings.m_shouldPromptForAPI ? static_cast<int>(rgProjectAPI::Unknown) : static_cast<int>(globalSettings.m_defaultAPI));

    // Font family.
    QFont font;
    font.setFamily(QString::fromStdString(globalSettings.m_fontFamily));
    ui.fontFamilyComboBox->setCurrentFont(font);

    // Font size.
    const int index = ui.fontSizeComboBox->findData(globalSettings.m_fontSize);
    if (index != -1)
    {
        ui.fontSizeComboBox->setCurrentIndex(index);
    }

    // Include files viewer.
    ui.includeFilesViewerLineEdit->setText(globalSettings.m_includeFilesViewer.c_str());

    // Input file associations.
    ui.assocExtGlslLineEdit->setText(globalSettings.m_inputFileExtGlsl.c_str());
    ui.assocExtHlslLineEdit->setText(globalSettings.m_inputFileExtHlsl.c_str());
    ui.assocExtSpvasLineEdit->setText(globalSettings.m_inputFileExtSpvTxt.c_str());
    ui.assocExtSpvBinaryLineEdit->setText(globalSettings.m_inputFileExtSpvBin.c_str());

    // Default high level language.
    ui.defaultLangComboBox->setCurrentIndex(globalSettings.m_defaultLang == rgSrcLanguage::GLSL ? 0 : 1);
}

rgGlobalSettings rgGlobalSettingsView::PullFromWidgets() const
{
    std::shared_ptr<rgGlobalSettings> pSettings = rgConfigManager::Instance().GetGlobalConfig();
    assert(pSettings != nullptr);

    // Make a local copy of this object so that it can be edited.
    rgGlobalSettings settings(*pSettings);

    // Log file location.
    settings.m_logFileLocation = ui.logFileLocationLineEdit->text().toStdString();

    // Column visibility.
    settings.m_visibleDisassemblyViewColumns = ListWidget::GetColumnVisibilityCheckboxes(m_pDisassemblyColumnsListWidget);

    // Use default project names.
    settings.m_useDefaultProjectName = ui.projectNameCheckBox->isChecked();

    // Default API.
    int defaultApiComboBoxIndex = ui.defaultApiOnStartupComboBox->currentIndex();
    if (defaultApiComboBoxIndex >= 0 && defaultApiComboBoxIndex < static_cast<int>(rgProjectAPI::ApiCount))
    {
        settings.m_defaultAPI = static_cast<rgProjectAPI>(defaultApiComboBoxIndex);

        settings.m_shouldPromptForAPI = (defaultApiComboBoxIndex == static_cast<int>(rgProjectAPI::Unknown));
    }

    // Font family.
    QFont font = ui.fontFamilyComboBox->currentFont();
    settings.m_fontFamily = font.family().toStdString();

    // Font size.
    settings.m_fontSize = ui.fontSizeComboBox->itemData(ui.fontSizeComboBox->currentIndex()).toInt();

    // Include files viewer.
    settings.m_includeFilesViewer = ui.includeFilesViewerLineEdit->text().toStdString();

    // Input file associations.
    settings.m_inputFileExtGlsl   = ui.assocExtGlslLineEdit->text().toStdString();
    settings.m_inputFileExtHlsl   = ui.assocExtHlslLineEdit->text().toStdString();
    settings.m_inputFileExtSpvTxt = ui.assocExtSpvasLineEdit->text().toStdString();
    settings.m_inputFileExtSpvBin = ui.assocExtSpvBinaryLineEdit->text().toStdString();

    // Default high level language.
    settings.m_defaultLang = (ui.defaultLangComboBox->currentIndex() == 0 ? rgSrcLanguage::GLSL : rgSrcLanguage::HLSL);

    return settings;
}

void rgGlobalSettingsView::ConnectSignals()
{
    // Browse log file location button.
    bool isConnected = connect(this->ui.logFileLocationFolderOpenButton, &QPushButton::clicked, this, &rgGlobalSettingsView::HandleLogFileLocationBrowseButtonClick);
    assert(isConnected);

    // Log file location line edit.
    isConnected = connect(this->ui.logFileLocationLineEdit, &QLineEdit::editingFinished, this, &rgGlobalSettingsView::HandleLogFileEditingFinished);
    assert(isConnected);

    // Log file location textChanged signal.
    isConnected = connect(this->ui.logFileLocationLineEdit, &QLineEdit::textChanged, this, &rgGlobalSettingsView::HandleLogFileEditBoxChanged);
    assert(isConnected);

    // Columns check box list widget button.
    isConnected = connect(this->ui.columnVisibilityArrowPushButton, &QPushButton::clicked, this, &rgGlobalSettingsView::HandleViewColumnsButtonClick);
    assert(isConnected);

    // Connect the use default program name check box.
    isConnected = connect(this->ui.projectNameCheckBox, &QCheckBox::stateChanged, this, &rgGlobalSettingsView::HandleProjectNameCheckboxStateChanged);
    assert(isConnected);

    // Connect the use default API combo box.
    isConnected = connect(ui.defaultApiOnStartupComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &rgGlobalSettingsView::HandleComboBoxChanged);
    assert(isConnected);

    // Connect the font family combo box.
    isConnected = connect(ui.fontFamilyComboBox, &QFontComboBox::currentFontChanged, this, &rgGlobalSettingsView::HandleFontFamilyChanged);
    assert(isConnected);

    // Connect the font size combo box.
    isConnected = connect(ui.fontSizeComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &rgGlobalSettingsView::HandleComboBoxChanged);
    assert(isConnected);

    // GLSL File extension associations textChanged signal.
    isConnected = connect(this->ui.assocExtGlslLineEdit, &QLineEdit::textChanged, this, &rgGlobalSettingsView::HandleTextBoxChanged);
    assert(isConnected);

    // HLSL File extension associations textChanged signal.
    isConnected = connect(this->ui.assocExtHlslLineEdit, &QLineEdit::textChanged, this, &rgGlobalSettingsView::HandleTextBoxChanged);
    assert(isConnected);

    // SPV Text File extension associations textChanged signal.
    isConnected = connect(this->ui.assocExtSpvasLineEdit, &QLineEdit::textChanged, this, &rgGlobalSettingsView::HandleTextBoxChanged);
    assert(isConnected);

    // SPV Binary File extension associations textChanged signal.
    isConnected = connect(this->ui.assocExtSpvBinaryLineEdit, &QLineEdit::textChanged, this, &rgGlobalSettingsView::HandleTextBoxChanged);
    assert(isConnected);

    // GLSL File extension associations textChanged signal.
    isConnected = connect(this->ui.assocExtGlslLineEdit, &rgLineEdit::LineEditFocusOutEvent, this, &rgGlobalSettingsView::HandleFocusOutEvent);
    assert(isConnected);

    // HLSL File extension associations textChanged signal.
    isConnected = connect(this->ui.assocExtHlslLineEdit, &rgLineEdit::LineEditFocusOutEvent, this, &rgGlobalSettingsView::HandleFocusOutEvent);
    assert(isConnected);

    // SPV Text File extension associations textChanged signal.
    isConnected = connect(this->ui.assocExtSpvasLineEdit, &rgLineEdit::LineEditFocusOutEvent, this, &rgGlobalSettingsView::HandleFocusOutEvent);
    assert(isConnected);

    // SPV Binary File extension associations textChanged signal.
    isConnected = connect(this->ui.assocExtSpvBinaryLineEdit, &rgLineEdit::LineEditFocusOutEvent, this, &rgGlobalSettingsView::HandleFocusOutEvent);
    assert(isConnected);

    // Default shader language combobox currentIndexChanged signal.
    isConnected = connect(this->ui.defaultLangComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &rgGlobalSettingsView::HandleComboBoxChanged);
    assert(isConnected);

    // Include files viewer browse button.
    isConnected = connect(this->ui.includeFilesViewerBrowseButton, &QPushButton::clicked, this, &rgGlobalSettingsView::HandleIncludeFilesViewerBrowseButtonClick);
    assert(isConnected);

    // Include files viewer line edit.
    isConnected = connect(this->ui.includeFilesViewerLineEdit, &QLineEdit::editingFinished, this, &rgGlobalSettingsView::HandleIncludeFilesViewerEditingFinished);
    assert(isConnected);

    // Include files viewer text changed.
    isConnected = connect(this->ui.includeFilesViewerLineEdit, &QLineEdit::textChanged, this, &rgGlobalSettingsView::HandleTextBoxChanged);
    assert(isConnected);
}

void rgGlobalSettingsView::ProcessInputFileBlank() const
{
    if (ui.assocExtGlslLineEdit->text().isEmpty())
    {
        emit ui.assocExtGlslLineEdit->LineEditFocusOutEvent();
    }
    else if (ui.assocExtHlslLineEdit->text().isEmpty())
    {
        emit ui.assocExtHlslLineEdit->LineEditFocusOutEvent();
    }
    else if (ui.assocExtSpvasLineEdit->text().isEmpty())
    {
        emit ui.assocExtSpvasLineEdit->LineEditFocusOutEvent();
    }
    else if (ui.assocExtSpvBinaryLineEdit->text().isEmpty())
    {
        emit ui.assocExtSpvBinaryLineEdit->LineEditFocusOutEvent();
    }
}

void rgGlobalSettingsView::HandleFocusOutEvent()
{
    static const char* STR_GLSL_LINE_EDIT = "assocExtGlslLineEdit";
    static const char* STR_HLSL_LINE_EDIT = "assocExtHlslLineEdit";
    static const char* STR_SPV_BIN_LINE_EDIT = "assocExtSpvBinaryLineEdit";
    static const char* STR_SPV_TEXT_LINE_EDIT = "assocExtSpvasLineEdit";
    static const char* STR_VALUE_BLANK_MESSAGE = "Input file type fields cannot be blank";

    // Figure out the sender and process appropriately.
    QObject* pSender = QObject::sender();
    assert(pSender != nullptr);

    rgLineEdit* pLineEdit = qobject_cast<rgLineEdit*>(pSender);
    assert(pLineEdit != nullptr);

    // Process the click.
    if (pLineEdit != nullptr)
    {
        QString text = pLineEdit->text();
        if (text.isEmpty())
        {
            QWidget* pWidget = static_cast<QWidget*>(pSender);
            assert(pWidget != nullptr);

            // Set the focus back to this widget.
            pWidget->setFocus();

            // Set the value to the previous one.
            QString name = pLineEdit->objectName();
            if (name.compare(STR_GLSL_LINE_EDIT) == 0)
            {
                pLineEdit->setText(QString::fromStdString(m_initialSettings.m_inputFileExtGlsl));
            }
            else if (name.compare(STR_HLSL_LINE_EDIT) == 0)
            {
                pLineEdit->setText(QString::fromStdString(m_initialSettings.m_inputFileExtHlsl));
            }
            else if (name.compare(STR_SPV_BIN_LINE_EDIT) == 0)
            {
                pLineEdit->setText(QString::fromStdString(m_initialSettings.m_inputFileExtSpvBin));
            }
            else if (name.compare(STR_SPV_TEXT_LINE_EDIT) == 0)
            {
                pLineEdit->setText(QString::fromStdString(m_initialSettings.m_inputFileExtSpvTxt));
            }

            // Display an error message.
            rgUtils::ShowErrorMessageBox(STR_VALUE_BLANK_MESSAGE, this);
        }
    }
}

void rgGlobalSettingsView::HandlePendingChangesStateChanged(bool hasPendingChanges)
{
    // Let the base class determine if there is a need to signal listeners
    // about the pending changes state.
    rgBuildSettingsView::SetHasPendingChanges(hasPendingChanges);
}

void rgGlobalSettingsView::HandleLogFileLocationBrowseButtonClick(bool /* checked */)
{
    // Get the current log file location.
    QString currentLogFileLocation = QString::fromStdString(m_initialSettings.m_logFileLocation);

    // Open up a directory chooser.
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setDirectory(currentLogFileLocation);
    if (dialog.exec())
    {
        QStringList selectedDir = dialog.selectedFiles();

        // Update the text box.
        QString selectedDirectory = selectedDir.at(0);
        if (!selectedDirectory.isEmpty())
        {
            ui.logFileLocationLineEdit->setText(selectedDirectory);

            // Signal to any listeners that the values in the UI have changed.
            HandlePendingChangesStateChanged(GetHasPendingChanges());
        }
    }
}

void rgGlobalSettingsView::HandleIncludeFilesViewerBrowseButtonClick(bool checked)
{
    // Get the current viewer.
    QString currentViewer = QString::fromStdString(m_initialSettings.m_logFileLocation);

#ifdef _WIN32
    const char* fileFilter = "Executable files (*.exe)|*.exe";
#else
    const char* fileFilter = "";
#endif

    // Open up a file dialog.
    std::string selectedViewerPath;
    bool isSelected = rgUtils::OpenFileDialog(this, selectedViewerPath,
        STR_FILE_DIALOG_FILTER_INCLUDE_VIEWER, fileFilter) &&
        rgUtils::IsFileExists(selectedViewerPath);

    // If the user selected a path, and it is a valid one, process this request.
    if (isSelected)
    {
        // Set the selected file path as the text in the text box.
        ui.includeFilesViewerLineEdit->setText(selectedViewerPath.c_str());

        // Signal to any listeners that the values in the UI have changed.
        HandlePendingChangesStateChanged(GetHasPendingChanges());
    }
}

void rgGlobalSettingsView::HandleViewColumnsButtonClick(bool /* checked */)
{
    // Make the list widget appear and process user selection from the list widget.
    bool visible = m_pDisassemblyColumnsListWidget->isVisible();
    if (visible)
    {
        m_pDisassemblyColumnsListWidget->hide();

        // Change the up arrow to a down arrow.
        ui.columnVisibilityArrowPushButton->SetDirection(ArrowIconWidget::Direction::DownArrow);
    }
    else
    {
        // If the initial settings are different than the global settings, then we know the checkbox values were
        // changed in another view.
        std::shared_ptr<rgGlobalSettings> pSettings = rgConfigManager::Instance().GetGlobalConfig();
        if (m_initialSettings.m_visibleDisassemblyViewColumns != pSettings->m_visibleDisassemblyViewColumns)
        {
            if (pSettings != nullptr)
            {
                ListWidget::SetColumnVisibilityCheckboxes(m_pDisassemblyColumnsListWidget, pSettings->m_visibleDisassemblyViewColumns);
            }
        }

        // Compute where to place the combo box relative to where the arrow button is.
        QWidget* pWidget = ui.columnVisibilityArrowPushButton;
        m_pDisassemblyColumnsListWidget->show();
        QRect rect = pWidget->geometry();
        QPoint pos(0, 0);
        pos = pWidget->mapTo(this, pos);
        pos.setY(pos.y() + rect.height());
        int height = QtCommon::QtUtil::GetListWidgetHeight(m_pDisassemblyColumnsListWidget);
        int width = QtCommon::QtUtil::GetListWidgetWidth(m_pDisassemblyColumnsListWidget);
        m_pDisassemblyColumnsListWidget->setGeometry(pos.x(), pos.y(), width + s_CHECK_BOX_WIDTH, height);

        // Change the down arrow to an up arrow.
        ui.columnVisibilityArrowPushButton->SetDirection(ArrowIconWidget::Direction::UpArrow);
    }
}

void rgGlobalSettingsView::SetCursor()
{
    // Set the cursor to pointing hand cursor.
    ui.columnVisibilityArrowPushButton->setCursor(Qt::PointingHandCursor);
    ui.logFileLocationFolderOpenButton->setCursor(Qt::PointingHandCursor);
    ui.projectNameCheckBox->setCursor(Qt::PointingHandCursor);
    ui.defaultApiOnStartupComboBox->setCursor(Qt::PointingHandCursor);
    ui.fontFamilyComboBox->setCursor(Qt::PointingHandCursor);
    ui.fontSizeComboBox->setCursor(Qt::PointingHandCursor);
    ui.includeFilesViewerBrowseButton->setCursor(Qt::PointingHandCursor);
}

void rgGlobalSettingsView::HandleColumnVisibilityComboBoxItemClicked(const QString& text, bool checked)
{
    int firstColumn = rgIsaDisassemblyTableModel::GetTableColumnIndex(rgIsaDisassemblyTableColumns::Address);
    int lastColumn = rgIsaDisassemblyTableModel::GetTableColumnIndex(rgIsaDisassemblyTableColumns::Count);

    std::vector<bool> columnVisibility = ListWidget::GetColumnVisibilityCheckboxes(m_pDisassemblyColumnsListWidget);

    // Make sure at least one check box is still checked.
    bool isAtLeastOneChecked = std::any_of(columnVisibility.begin() + firstColumn, columnVisibility.begin() + lastColumn, [](bool b) { return b == true; });

    if (checked || isAtLeastOneChecked)
    {
        // If the user checked the "All" option, Step through each column and set to visible.
        if (text.compare(STR_DISASSEMBLY_TABLE_COLUMN_ALL) == 0 && (checked == true))
        {
            for (int columnIndex = firstColumn; columnIndex < lastColumn; ++columnIndex)
            {
                columnVisibility[columnIndex] = checked;
            }

            // Update the state of the dropdown check boxes to reflect that all options are checked.
            ListWidget::SetColumnVisibilityCheckboxes(m_pDisassemblyColumnsListWidget, columnVisibility);
        }
    }
    else
    {
        // The user tried to uncheck the last check box, but at least one box
        // MUST be checked, so find that item in the ListWidget, and set it back to checked.
        for (int row = 0; row < m_pDisassemblyColumnsListWidget->count(); row++)
        {
            QListWidgetItem* pItem = m_pDisassemblyColumnsListWidget->item(row);
            QCheckBox* pCheckBox = (QCheckBox*)m_pDisassemblyColumnsListWidget->itemWidget(pItem);
            QString checkBoxText = pCheckBox->text();
            if (checkBoxText.compare(text) == 0)
            {
                QCheckBox* pCheckBox = (QCheckBox*)m_pDisassemblyColumnsListWidget->itemWidget(pItem);
                pCheckBox->setChecked(true);
            }
        }
    }

    // See if the "All" box needs checking/un-checking.
    ListWidget::UpdateAllCheckbox(m_pDisassemblyColumnsListWidget);

    // Update the "All" checkbox text color to grey or black.
    UpdateAllCheckBoxText();

    // Signal to any listeners that the values in the UI have changed.
    HandlePendingChangesStateChanged(GetHasPendingChanges());
}

void rgGlobalSettingsView::HandleColumnVisibilityFilterStateChanged(bool checked)
{
    // Figure out the sender and process appropriately.
    QObject* pSender = QObject::sender();
    assert(pSender != nullptr);

    // Find out which entry caused the signal.
    QWidget* pItem = qobject_cast<QWidget*>(pSender);
    assert(pItem != nullptr);

    QCheckBox* pCheckBox = qobject_cast<QCheckBox*>(pSender);
    assert(pCheckBox != nullptr);

    // Process the click.
    if (pCheckBox != nullptr)
    {
        HandleColumnVisibilityComboBoxItemClicked(pCheckBox->text(), checked);
    }
}

std::string rgGlobalSettingsView::GetDisassemblyColumnName(rgIsaDisassemblyTableColumns column) const
{
    std::string result;

    static std::map<rgIsaDisassemblyTableColumns, std::string> ColumnNameMap =
    {
        { rgIsaDisassemblyTableColumns::Address,                   STR_DISASSEMBLY_TABLE_COLUMN_ADDRESS },
        { rgIsaDisassemblyTableColumns::Opcode,                    STR_DISASSEMBLY_TABLE_COLUMN_OPCODE },
        { rgIsaDisassemblyTableColumns::Operands,                  STR_DISASSEMBLY_TABLE_COLUMN_OPERANDS },
        { rgIsaDisassemblyTableColumns::FunctionalUnit,            STR_DISASSEMBLY_TABLE_COLUMN_FUNCTIONAL_UNIT },
        { rgIsaDisassemblyTableColumns::Cycles,                    STR_DISASSEMBLY_TABLE_COLUMN_CYCLES },
        { rgIsaDisassemblyTableColumns::BinaryEncoding,            STR_DISASSEMBLY_TABLE_COLUMN_BINARY_ENCODING },
    };

    auto columnNameIter = ColumnNameMap.find(column);
    if (columnNameIter != ColumnNameMap.end())
    {
        result = columnNameIter->second;
    }
    else
    {
        // The incoming column doesn't have a name string mapped to it.
        assert(false);
    }

    return result;
}

void rgGlobalSettingsView::PopulateColumnVisibilityList()
{
    // Set up the function pointer responsible for handling column visibility filter state change.
    using std::placeholders::_1;
    std::function<void(bool)> slotFunctionPointer = std::bind(&rgGlobalSettingsView::HandleColumnVisibilityFilterStateChanged, this, _1);

    // Remove the existing items first.
    ClearListWidget(m_pDisassemblyColumnsListWidget);

    // Add the "All" entry.
    ListWidget::AddListWidgetCheckboxItem(STR_DISASSEMBLY_TABLE_COLUMN_ALL, m_pDisassemblyColumnsListWidget, slotFunctionPointer, this, STR_GLOBAL_SETTINGS_COLUMN_VISIBILITY_LIST, STR_GLOBAL_SETTINGS_COLUMN_LIST_ITEM_ALL_CHECKBOX);

    // Loop through each column enum member.
    int startColumn = rgIsaDisassemblyTableModel::GetTableColumnIndex(rgIsaDisassemblyTableColumns::Address);
    int endColumn = rgIsaDisassemblyTableModel::GetTableColumnIndex(rgIsaDisassemblyTableColumns::Count);

    // Add an item for each column in the table.
    for (int columnIndex = startColumn; columnIndex < endColumn; ++columnIndex)
    {
        // Add an item for each possible column in the table.
        std::string columnName = GetDisassemblyColumnName(static_cast<rgIsaDisassemblyTableColumns>(columnIndex));
        ListWidget::AddListWidgetCheckboxItem(columnName.c_str(), m_pDisassemblyColumnsListWidget, slotFunctionPointer, this, STR_GLOBAL_SETTINGS_COLUMN_VISIBILITY_LIST, STR_GLOBAL_SETTINGS_COLUMN_LIST_ITEM_CHECKBOX);
    }
}

void rgGlobalSettingsView::UpdateAllCheckBoxText()
{
    bool areAllItemsChecked = true;

    // Check to see if all of the boxes are checked.
    for (int i = 1; i < m_pDisassemblyColumnsListWidget->count(); i++)
    {
        QListWidgetItem* pItem = m_pDisassemblyColumnsListWidget->item(i);
        assert(pItem != nullptr);

        QCheckBox* pCheckBox = qobject_cast<QCheckBox*>(m_pDisassemblyColumnsListWidget->itemWidget(pItem));
        assert(pCheckBox != nullptr);

        if (pCheckBox->checkState() == Qt::CheckState::Unchecked)
        {
            areAllItemsChecked = false;
            break;
        }
    }

    // If all boxes are checked, update the text color of the All check box.
    QListWidgetItem* pItem = m_pDisassemblyColumnsListWidget->item(0);
    if (pItem != nullptr)
    {
        QCheckBox* pCheckBox = qobject_cast<QCheckBox*>(m_pDisassemblyColumnsListWidget->itemWidget(pItem));
        if (pCheckBox != nullptr)
        {
            if (areAllItemsChecked)
            {
                pCheckBox->setStyleSheet("QCheckBox#ListWidgetAllCheckBox {color: grey;}");
            }
            else
            {
                pCheckBox->setStyleSheet("QCheckBox#ListWidgetAllCheckBox {color: black;}");
            }
        }
    }
}

void rgGlobalSettingsView::CreateColumnVisibilityControls()
{
    // Setup the list widget that opens when the user clicks the column visibility arrow.
    rgUtils::SetupComboList(m_pParent, m_pDisassemblyColumnsListWidget, ui.columnVisibilityArrowPushButton, m_pDisassemblyColumnsListEventFilter, false);
    m_pDisassemblyColumnsListWidget->setObjectName(STR_GLOBAL_SETTINGS_COLUMN_VISIBILITY_LIST);

    // Update scale factor for widgets.
    QFont font = ui.columnVisibilityArrowPushButton->font();
    double scaleFactor = ScalingManager::Get().GetScaleFactor();
    font.setPointSize(s_PUSH_BUTTON_FONT_SIZE * scaleFactor);
    m_pDisassemblyColumnsListWidget->setStyleSheet(s_LIST_WIDGET_STYLE.arg(font.pointSize()));

    // Reset the current selection in the column visibility list.
    m_pDisassemblyColumnsListWidget->setCurrentRow(0);
}

void rgGlobalSettingsView::HandleProjectNameCheckboxStateChanged(int checked)
{
    // Signal to any listeners that the values in the UI have changed.
    HandlePendingChangesStateChanged(GetHasPendingChanges());
}

void rgGlobalSettingsView::ClearListWidget(ListWidget* &pListWidget)
{
    assert(pListWidget != nullptr);

    // Disconnect slot/signal connection for each check box
    for (int row = 0; row < pListWidget->count(); row++)
    {
        QListWidgetItem* pItem = pListWidget->item(row);
        QCheckBox* pCheckBox = (QCheckBox*)pListWidget->itemWidget(pItem);

        if (pListWidget->objectName().compare(STR_GLOBAL_SETTINGS_COLUMN_VISIBILITY_LIST) == 0)
        {
            bool isDisconnected = disconnect(pCheckBox, &QCheckBox::clicked, this, &rgGlobalSettingsView::HandleColumnVisibilityFilterStateChanged);
            assert(isDisconnected);
        }
        else
        {
            assert(false);
        }
    }

    // Clear the list widget. This also deletes each item.
    pListWidget->clear();
}

void rgGlobalSettingsView::CloseListWidget()
{
    m_pDisassemblyColumnsListWidget->hide();
    ui.columnVisibilityArrowPushButton->SetDirection(ArrowIconWidget::Direction::DownArrow);
}

void rgGlobalSettingsView::SetCheckboxToolTip(const std::string& text)
{
    ui.projectNameCheckBox->setToolTip(text.c_str());
}

void rgGlobalSettingsView::HandleLogFileEditingFinished()
{
    // Verify that the line edit text is not empty or the location is not invalid before losing the focus.
    if (this->ui.logFileLocationLineEdit->text().isEmpty() || !rgUtils::IsDirExists(this->ui.logFileLocationLineEdit->text().toStdString()))
    {
        // If empty or invalid, use the saved location.
        std::shared_ptr<rgGlobalSettings> pGlobalConfig = rgConfigManager::Instance().GetGlobalConfig();
        assert(pGlobalConfig != nullptr);
        if (pGlobalConfig != nullptr)
        {
            this->ui.logFileLocationLineEdit->setText(pGlobalConfig->m_logFileLocation.c_str());
        }
        this->ui.logFileLocationLineEdit->setFocus();
    }

    // Signal to any listeners that the values in the UI have changed.
    HandlePendingChangesStateChanged(GetHasPendingChanges());
}

void rgGlobalSettingsView::SetEditBoxToolTips()
{
    ui.logFileLocationLineEdit->setToolTip(ui.logFileLocationLineEdit->displayText());
}

void rgGlobalSettingsView::HandleLogFileEditBoxChanged(const QString& text)
{
    // Update the tooltip.
    ui.logFileLocationLineEdit->setToolTip(text);

    // Restore the cursor to the original position when the text has changed.
    QtCommon::QtUtil::RestoreCursorPosition cursorPosition(ui.logFileLocationLineEdit);

    // Signal to any listeners that the values in the UI have changed.
    HandlePendingChangesStateChanged(GetHasPendingChanges());
}

void rgGlobalSettingsView::HandleTextBoxChanged(const QString& text)
{
    // Figure out the sender and process appropriately.
    QObject* pSender = QObject::sender();
    assert(pSender != nullptr);

    rgLineEdit* pLineEdit = qobject_cast<rgLineEdit*>(pSender);
    assert(pLineEdit != nullptr);

    // Signal whether the text box is empty or not.
    if (pLineEdit != nullptr)
    {
        QString text = pLineEdit->text();
        if (text.isEmpty())
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

void rgGlobalSettingsView::HandleComboBoxChanged(int index)
{
    // Signal to any listeners that the values in the UI have changed.
    HandlePendingChangesStateChanged(GetHasPendingChanges());
}

bool rgGlobalSettingsView::GetHasPendingChanges() const
{
    rgGlobalSettings currentSettings = PullFromWidgets();

    bool hasChanges = !m_initialSettings.HasSameSettings(currentSettings);

    return hasChanges;
}

bool rgGlobalSettingsView::RevertPendingChanges()
{
    std::shared_ptr<rgGlobalSettings> pSettings = rgConfigManager::Instance().GetGlobalConfig();
    assert(pSettings != nullptr);
    if (pSettings != nullptr)
    {
        PushToWidgets(*pSettings);
    }

    // Make sure the rest of the UI knows that the settings don't need to be saved.
    HandlePendingChangesStateChanged(false);

    return false;
}

void rgGlobalSettingsView::RestoreDefaultSettings()
{
    // Reset our initial settings back to the defaults
    rgConfigManager::Instance().ResetToFactoryDefaults(m_initialSettings);

    // Update the UI to reflect the new initial settings
    PushToWidgets(m_initialSettings);

    // Update the ConfigManager to use the new settings.
    rgConfigManager::Instance().SetGlobalConfig(m_initialSettings);

    // Save the settings file.
    rgConfigManager::Instance().SaveGlobalConfigFile();

    // Signal to any listeners that the values in the UI have changed.
    HandlePendingChangesStateChanged(GetHasPendingChanges());
}

bool rgGlobalSettingsView::SaveSettings()
{
    // Reset the initial settings to match what the UI shows.
    m_initialSettings = PullFromWidgets();

    // Update the config manager to use these new settings.
    rgConfigManager& configManager = rgConfigManager::Instance();
    configManager.SetGlobalConfig(m_initialSettings);

    // Save the settings.
    bool isSaved = configManager.SaveGlobalConfigFile();

    if (isSaved)
    {
        // Make sure the rest of the UI knows that the settings have been saved.
        HandlePendingChangesStateChanged(false);
    }

    return isSaved;
}

std::string rgGlobalSettingsView::GetTitleString()
{
    std::string titleString = STR_SETTINGS_CONFIRMATION_APPLICATION_SETTINGS;

    return titleString;
}

void rgGlobalSettingsView::HandleFontFamilyChanged(const QFont& font)
{
    // Signal to any listeners that the values in the UI have changed.
    HandlePendingChangesStateChanged(GetHasPendingChanges());
}

void rgGlobalSettingsView::HandleIncludeFilesViewerEditingFinished()
{
    // Verify that the line edit text is not empty or the location is not invalid before losing the focus.
    if (this->ui.includeFilesViewerLineEdit->text().isEmpty())
    {
        this->ui.includeFilesViewerLineEdit->setText(STR_GLOBAL_SETTINGS_SRC_VIEW_INCLUDE_VIEWER_DEFAULT);
    }
    else if (this->ui.includeFilesViewerLineEdit->text().compare(STR_GLOBAL_SETTINGS_SRC_VIEW_INCLUDE_VIEWER_DEFAULT) != 0 &&
        !rgUtils::IsFileExists(this->ui.includeFilesViewerLineEdit->text().toStdString()))
    {
        // If empty or invalid, use the saved location.
        std::shared_ptr<rgGlobalSettings> pGlobalConfig = rgConfigManager::Instance().GetGlobalConfig();
        assert(pGlobalConfig != nullptr);
        if (pGlobalConfig != nullptr)
        {
            this->ui.includeFilesViewerLineEdit->setText(pGlobalConfig->m_includeFilesViewer.c_str());
        }
        this->ui.includeFilesViewerLineEdit->setFocus();
    }

    // Signal to any listeners that the values in the UI have changed.
    HandlePendingChangesStateChanged(GetHasPendingChanges());
}

void rgGlobalSettingsView::SetInitialWidgetFocus()
{
    ui.logFileLocationLineEdit->setFocus();
}

bool rgGlobalSettingsView::IsInputFileBlank() const
{
    return ui.assocExtGlslLineEdit->text().isEmpty() ||
        ui.assocExtHlslLineEdit->text().isEmpty() ||
        ui.assocExtSpvasLineEdit->text().isEmpty() ||
        ui.assocExtSpvBinaryLineEdit->text().isEmpty();
}

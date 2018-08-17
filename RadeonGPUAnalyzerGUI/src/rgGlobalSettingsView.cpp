// C++.
#include <cassert>
#include <sstream>

// Qt.
#include <QApplication>
#include <QCheckBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QWidget>

// Infra.
#include <QtCommon/CustomWidgets/ArrowIconWidget.h>
#include <QtCommon/Util/CommonDefinitions.h>
#include <QtCommon/CustomWidgets/ListWidget.h>
#include <QtCommon/Util/QtUtil.h>
#include <QtCommon/Util/RestoreCursorPosition.h>

// Local.
#include <RadeonGPUAnalyzerGUI/include/rgConfigManager.h>
#include <RadeonGPUAnalyzerGUI/include/rgDefinitions.h>
#include <RadeonGPUAnalyzerGUI/include/qt/rgGlobalSettingsModel.h>
#include <RadeonGPUAnalyzerGUI/include/qt/rgGlobalSettingsView.h>
#include <RadeonGPUAnalyzerGUI/include/qt/rgHideListWidgetEventFilter.h>
#include <RadeonGPUAnalyzerGUI/include/qt/rgIsaDisassemblyTableModel.h>
#include <RadeonGPUAnalyzerGUI/include/rgStringConstants.h>
#include <RadeonGPUAnalyzerGUI/include/rgUtils.h>

// Names of special widgets within the build settings view.
static const char* STR_GLOBAL_SETTINGS_COLUMN_VISIBILITY_LIST = "DisassemblyColumnVisibilityList";
static const char* STR_GLOBAL_SETTINGS_COLUMN_LIST_ITEM_CHECKBOX = "ListWidgetCheckBox";
static const char* STR_GLOBAL_SETTINGS_COLUMN_LIST_ITEM_ALL_CHECKBOX = "ListWidgetAllCheckBox";

// Columns push button font size.
const int s_PUSH_BUTTON_FONT_SIZE = 11;

rgGlobalSettingsView::rgGlobalSettingsView(QWidget* pParent, std::shared_ptr<rgGlobalSettings> pGlobalSettings) :
    rgBuildSettingsView(pParent, true),
    m_pParent(pParent)
{
    // Setup the UI.
    ui.setupUi(this);

    // Set the background to white.
    QPalette pal = palette();
    pal.setColor(QPalette::Background, Qt::white);
    this->setAutoFillBackground(true);
    this->setPalette(pal);

    // Make sure that the build settings that we received are valid.
    assert(pGlobalSettings != nullptr);
    if (pGlobalSettings != nullptr)
    {
        // Create the widget used to control column visibility.
        CreateColumnVisibilityControls();

        m_pSettingsModel = new rgGlobalSettingsModel(pGlobalSettings, rgGlobalSettingsControls::Count);

        // Initialize the model by binding UI widgets to it.
        InitializeModel();

        // Initialize the values in the model.
        assert(m_pSettingsModel != nullptr);
        if (m_pSettingsModel != nullptr)
        {
            m_pSettingsModel->InitializeModelValues();
        }

        // Fill the column visibility dropdown with all of the possible column names.
        PopulateColumnVisibilityList();

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

        // Set the tooltips for the disassembly columns.
        ui.disassemblyViewLabel->setToolTip(STR_GLOBAL_SETTINGS_DISASSEMBLY_COLUMNS);

        // Set the fonts for the list widget push button.
        QFont font = ui.columnVisibilityArrowPushButton->font();
        double scaleFactor = ScalingManager::Get().GetScaleFactor();
        font.setPointSize(gs_BUTTON_POINT_FONT_SIZE * scaleFactor);
        ui.columnVisibilityArrowPushButton->setFont(font);
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

    assert(m_pSettingsModel != nullptr);
    if (m_pSettingsModel != nullptr)
    {
        isConnected = connect(m_pSettingsModel, &rgGlobalSettingsModel::PendingChangesStateChanged, this, &rgGlobalSettingsView::HandlePendingChangesStateChanged);
        assert(isConnected);
    }
}

void rgGlobalSettingsView::HandlePendingChangesStateChanged(bool hasPendingChanges)
{
    emit PendingChangesStateChanged(hasPendingChanges);
}

void rgGlobalSettingsView::HandleLogFileLocationBrowseButtonClick(bool /* checked */)
{
    // Get the current log file location.
    assert(m_pSettingsModel != nullptr);
    if (m_pSettingsModel != nullptr)
    {
        std::shared_ptr<rgGlobalSettings> pPendingGlobalSettings = m_pSettingsModel->GetPendingGlobalSettings();
        assert(pPendingGlobalSettings != nullptr);
        if (pPendingGlobalSettings != nullptr)
        {
            QString currentLogFileLocation = QString::fromStdString(pPendingGlobalSettings->m_logFileLocation);

            // Open up a directory chooser.
            QFileDialog dialog(this);
            dialog.setFileMode(QFileDialog::Directory);
            dialog.setDirectory(currentLogFileLocation);
            if (dialog.exec())
            {
                QStringList selectedDir = dialog.selectedFiles();

                // Update the text box
                QString selectedDirectory = selectedDir.at(0);
                if (!selectedDirectory.isEmpty())
                {
                    m_pSettingsModel->UpdateModelValue(rgGlobalSettingsControls::LogFileLocation, selectedDirectory, false);
                }
            }
        }
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
        // Update the check box values in case they were changed in disassembly view.
        std::shared_ptr<rgGlobalSettings> pPendingGlobalSettings = m_pSettingsModel->GetPendingGlobalSettings();
        ListWidget::SetColumnVisibilityCheckboxes(m_pDisassemblyColumnsListWidget, pPendingGlobalSettings->m_visibleDisassemblyViewColumns);

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
}

void rgGlobalSettingsView::HandleColumnVisibilityComboBoxItemClicked(const QString& text, const bool checked)
{
    int firstColumn = rgIsaDisassemblyTableModel::GetTableColumnIndex(rgIsaDisassemblyTableColumns::Address);
    int lastColumn = rgIsaDisassemblyTableModel::GetTableColumnIndex(rgIsaDisassemblyTableColumns::Count);

    assert(m_pSettingsModel != nullptr);
    if (m_pSettingsModel != nullptr)
    {
        std::shared_ptr<rgGlobalSettings> pPendingGlobalSettings = m_pSettingsModel->GetPendingGlobalSettings();
        assert(pPendingGlobalSettings != nullptr);
        if (pPendingGlobalSettings != nullptr)
        {
            // Make sure that the user didn't uncheck the only checked box, nor did they uncheck the "All" button, since this will uncheck
            // every single check box, which is something we do not want.
            if (checked || QtCommon::QtUtil::VerifyOneCheckboxChecked(pPendingGlobalSettings->m_visibleDisassemblyViewColumns, firstColumn, lastColumn))
            {
                // Process the selected text accordingly.
                if (text.compare(STR_DISASSEMBLY_TABLE_COLUMN_ALL) == 0 && (checked == true))
                {
                    // Step through each column and set to visible.
                    for (int columnIndex = firstColumn; columnIndex < lastColumn; ++columnIndex)
                    {
                        pPendingGlobalSettings->m_visibleDisassemblyViewColumns[columnIndex] = checked;
                    }

                    // Update the state of the dropdown check boxes based on the visibility in the global settings.
                    ListWidget::SetColumnVisibilityCheckboxes(m_pDisassemblyColumnsListWidget, pPendingGlobalSettings->m_visibleDisassemblyViewColumns);
                }
                else
                {
                    // Step through each column and set the visibility if the user changed the check state.
                    for (int columnIndex = firstColumn; columnIndex < lastColumn; ++columnIndex)
                    {
                        rgIsaDisassemblyTableColumns column = static_cast<rgIsaDisassemblyTableColumns>(columnIndex);
                        std::string columnName = GetDisassemblyColumnName(column);
                        if (text.compare(columnName.c_str()) == 0)
                        {
                            pPendingGlobalSettings->m_visibleDisassemblyViewColumns[columnIndex] = checked;
                            break;
                        }
                    }
                }

                // Update the model value with each column's check state.
                std::string columnVisibilityString = rgUtils::BuildSemicolonSeparatedBoolList(pPendingGlobalSettings->m_visibleDisassemblyViewColumns);
                m_pSettingsModel->UpdateModelValue(rgGlobalSettingsControls::DisassemblyViewColumns, columnVisibilityString.c_str(), false);
            }
            else
            {
                // Uncheck the check box since this is the only checked check box.
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
        }
    }
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

void rgGlobalSettingsView::InitializeModel()
{
    assert(m_pSettingsModel != nullptr);
    if (m_pSettingsModel != nullptr)
    {
        m_pSettingsModel->InitializeModel(ui.logFileLocationLineEdit, rgGlobalSettingsControls::LogFileLocation, "text");

        m_pSettingsModel->InitializeModel(m_pDisassemblyColumnsListWidget, rgGlobalSettingsControls::DisassemblyViewColumns, "checkStates");

        m_pSettingsModel->InitializeModel(ui.projectNameCheckBox, rgGlobalSettingsControls::AlwaysUseGeneratedProjectNames, "checked");
    }
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

    // Populate the check box items with the model's pending settings.
    std::shared_ptr<rgGlobalSettings> pPendingGlobalChanges = m_pSettingsModel->GetPendingGlobalSettings();
    ListWidget::SetColumnVisibilityCheckboxes(m_pDisassemblyColumnsListWidget, pPendingGlobalChanges->m_visibleDisassemblyViewColumns);
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
    assert(m_pSettingsModel != nullptr);
    if (m_pSettingsModel != nullptr)
    {
        bool value = checked == Qt::Checked ? true : false;
        m_pSettingsModel->UpdateModelValue(rgGlobalSettingsControls::AlwaysUseGeneratedProjectNames, value, false);
    }
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

    assert(m_pSettingsModel != nullptr);
    if (m_pSettingsModel != nullptr)
    {
        // Update the model value.
        m_pSettingsModel->UpdateModelValue(rgGlobalSettingsControls::LogFileLocation, text, false);
    }
}

void rgGlobalSettingsView::HandleDefaultProjectNameCheckboxUpdate()
{
    // Set the value for the project name check box.
    std::shared_ptr<rgGlobalSettings> pGlobalConfig = rgConfigManager::Instance().GetGlobalConfig();
    if (pGlobalConfig != nullptr)
    {
        ui.projectNameCheckBox->setChecked(pGlobalConfig->m_useDefaultProjectName);
    }
}

bool rgGlobalSettingsView::GetHasPendingChanges() const
{
    bool ret = false;
    assert(m_pSettingsModel != nullptr);
    if (m_pSettingsModel != nullptr)
    {
        ret = m_pSettingsModel->HasPendingChanges();
    }

    return ret;
}

bool rgGlobalSettingsView::RevertPendingChanges()
{
    assert(m_pSettingsModel != nullptr);
    if (m_pSettingsModel != nullptr)
    {
        m_pSettingsModel->RevertPendingChanges();

        // Populate the check box items with the model's pending settings.
        std::shared_ptr<rgGlobalSettings> pGlobalChanges = m_pSettingsModel->GetGlobalSettings();
        assert(pGlobalChanges != nullptr);
        if (pGlobalChanges != nullptr)
        {
            ListWidget::SetColumnVisibilityCheckboxes(m_pDisassemblyColumnsListWidget, pGlobalChanges->m_visibleDisassemblyViewColumns);
            auto pSettings = m_pSettingsModel->GetPendingGlobalSettings();
            assert(pSettings != nullptr);
            if (pSettings != nullptr)
            {
                *pSettings = *pGlobalChanges;
            }
        }
    }

    return false;
}

void rgGlobalSettingsView::RestoreDefaultSettings()
{
    rgConfigManager& configManager = rgConfigManager::Instance();

    assert(m_pSettingsModel != nullptr);
    if (m_pSettingsModel != nullptr)
    {
        std::shared_ptr<rgGlobalSettings> pPendingGlobalConfig = m_pSettingsModel->GetPendingGlobalSettings();
        std::shared_ptr<rgGlobalSettings> pGlobalConfig = m_pSettingsModel->GetGlobalSettings();
        assert(pPendingGlobalConfig != nullptr && pGlobalConfig != nullptr);
        if (pPendingGlobalConfig != nullptr && pGlobalConfig != nullptr)
        {
            // Reset the pending and actual global settings to default values.
            configManager.InitializeDefaultGlobalConfig(pPendingGlobalConfig);
            configManager.InitializeDefaultGlobalConfig(pGlobalConfig);

            // Save the config file.
            configManager.SetGlobalConfig(pGlobalConfig);
            configManager.SaveGlobalConfigFile();

            // Set the default settings in the model.
            m_pSettingsModel->SetBuildSettings(pGlobalConfig);

            // Revert any pending changes in the settings model.
            m_pSettingsModel->RevertPendingChanges();

            // Populate the check box items by reading the global settings.
            ListWidget::SetColumnVisibilityCheckboxes(m_pDisassemblyColumnsListWidget, pGlobalConfig->m_visibleDisassemblyViewColumns);
        }
    }
}

void rgGlobalSettingsView::SaveSettings()
{
    assert(m_pSettingsModel != nullptr);
    if (m_pSettingsModel != nullptr)
    {
        m_pSettingsModel->SubmitPendingChanges();

        rgConfigManager& configManager = rgConfigManager::Instance();

        std::shared_ptr<rgGlobalSettings> pGlobalSettings = m_pSettingsModel->GetGlobalSettings();
        configManager.SetGlobalConfig(pGlobalSettings);

        configManager.SaveGlobalConfigFile();
    }
}
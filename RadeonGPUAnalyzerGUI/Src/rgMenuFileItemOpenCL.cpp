// C++.
#include <cassert>
#include <sstream>

// Qt.
#include <QMenu>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QUrl>
#include <QKeyEvent>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMenuOpenCL.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMenuFileItemOpenCL.h>
#include <RadeonGPUAnalyzerGUI/Include/rgUtils.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDefinitions.h>

static const char* s_FILE_MENU_ITEM_COLOR = "#itemBackground[current = true] {background-color: rgb(253, 255, 215); border-style: solid; border-width: 1px; border-color: rgb(18, 152, 0);}";

// A delegate used to style a file item's entry point list.
class rgEntrypointItemStyleDelegate : public QStyledItemDelegate
{
public:
    rgEntrypointItemStyleDelegate(QObject* pParent = nullptr)
        : QStyledItemDelegate(pParent) {}

    // A custom row painter for the entry point list.
    void paint(QPainter* pPainter, const QStyleOptionViewItem& option, const QModelIndex& index) const
    {
        QStyleOptionViewItem itemOption(option);

        if (index.isValid())
        {
            initStyleOption(&itemOption, index);
        }

        // Invoke the item paint implementation with the updated item options.
        QStyledItemDelegate::paint(pPainter, itemOption, index);
    }
};

rgMenuItemEntryListModel::rgMenuItemEntryListModel(QWidget* pParent)
    : QObject(pParent)
{
    // Initialize the item model.
    m_pEntrypointItemModel = new QStandardItemModel(pParent);

    // Figure out which API is being used. This will determine which text is used for the item's entrypoints list.
    rgProjectAPI currentAPI = rgConfigManager::Instance().GetCurrentAPI();
    const std::string entrypointLabelText = rgUtils::GetEntrypointsNameString(currentAPI);

    // Add the column header text.
    m_pEntrypointItemModel->setHorizontalHeaderItem(0, new QStandardItem(entrypointLabelText.c_str()));
}

QStandardItemModel* rgMenuItemEntryListModel::GetEntryItemModel() const
{
    return m_pEntrypointItemModel;
}

void rgMenuItemEntryListModel::AddEntry(const std::string& entrypointName)
{
    std::string displayText;

    int currentRowCount = m_pEntrypointItemModel->rowCount();
    int newRowCount = currentRowCount + 1;

    // Truncate long entry point names.
    rgUtils::GetDisplayText(entrypointName, displayText, m_entryPointWidgetWidth, m_pEntryPointTree, gs_TEXT_TRUNCATE_LENGTH_BACK_OPENCL);

    // Save the entry point name and the possibly truncated display name.
    m_entryPointNames.push_back(entrypointName);
    m_displayNames.push_back(displayText);

    // Update the number of row items in the model.
    m_pEntrypointItemModel->setRowCount(newRowCount);

    // Set the data for the new item row, and left-align the entry point name string.
    QModelIndex modelIndex = m_pEntrypointItemModel->index(currentRowCount, 0);
    m_pEntrypointItemModel->setData(modelIndex, QString(displayText.c_str()));
    m_pEntrypointItemModel->setData(modelIndex, Qt::AlignLeft, Qt::TextAlignmentRole);
    m_pEntrypointItemModel->dataChanged(modelIndex, modelIndex);

    // Set the tooltip for the new entry point item.
    std::stringstream tooltipText;
    tooltipText << STR_MENU_ITEM_ENTRYPOINT_TOOLTIP_TEXT;
    tooltipText << entrypointName;
    m_pEntrypointItemModel->setData(modelIndex, tooltipText.str().c_str(), Qt::ToolTipRole);
}

void rgMenuItemEntryListModel::ClearEntries()
{
    // Clear the model data by removing all existing rows.
    int numRows = m_pEntrypointItemModel->rowCount();
    m_pEntrypointItemModel->removeRows(0, numRows);
}

void rgMenuItemEntryListModel::SetEntryPointWidgetWidth(const int width)
{
    m_entryPointWidgetWidth = width;
}

void rgMenuItemEntryListModel::SetEntryPointTreeWidget(rgMenuEntryPointTree* pTree)
{
    m_pEntryPointTree = pTree;
}

void rgMenuItemEntryListModel::GetEntryPointNames(std::vector<std::string>& entrypointNames)
{
    entrypointNames = m_entryPointNames;
}

std::string rgMenuItemEntryListModel::GetEntryPointName(const int index) const
{
    return m_entryPointNames[index];
}

std::string rgMenuItemEntryListModel::GetEntryPointName(const std::string& displayEntrypointName) const
{
    int index = 0;

    std::string value;
    if (!displayEntrypointName.empty())
    {
        auto iter = std::find(m_displayNames.begin(), m_displayNames.end(), displayEntrypointName);
        if (iter != m_displayNames.end())
        {
            value = *iter;
        }
    }
    return value;
}

rgMenuFileItemOpenCL::rgMenuFileItemOpenCL(const std::string& fileFullPath, rgMenu* pParent) :
    rgMenuFileItem(fileFullPath, pParent)
{
    ui.setupUi(this);

    UpdateFilepath(m_fullFilepath);

    // Start as a saved item.
    SetIsSaved(true);

    // Don't show QLineEdit item renaming control when an item is first created.
    ShowRenameControls(false);

    // The close button is hidden by default, and is made visible on item mouseover.
    ui.closeButton->hide();

    // Set tool and status tip for close button.
    std::string tooltip = STR_FILE_MENU_REMOVE_FILE_TOOLTIP_PREFIX + m_fullFilepath + STR_FILE_MENU_REMOVE_FILE_TOOLTIP_SUFFIX;
    rgUtils::SetToolAndStatusTip(tooltip, ui.closeButton);

    // Initialize the entry point list.
    InitializeEntrypointsList();

    // Connect signals for the file item.
    ConnectSignals();

    // Enable mouse tracking for the entry point list view.
    ui.entrypointListView->setMouseTracking(true);

    // Set mouse pointer to pointing hand cursor.
    SetCursor();
}

void rgMenuFileItemOpenCL::enterEvent(QEvent* pEvent)
{
    ui.closeButton->show();

    // Change the item color.
    SetHovered(true);
}

void rgMenuFileItemOpenCL::leaveEvent(QEvent* pEvent)
{
    ui.closeButton->hide();

    // Change the item color.
    SetHovered(false);
}

void rgMenuFileItemOpenCL::mouseDoubleClickEvent(QMouseEvent* pEvent)
{
    // On double-click, allow the user to re-name the item's filename.
    ShowRenameControls(true);
}

void rgMenuFileItemOpenCL::mousePressEvent(QMouseEvent* pEvent)
{
    emit MenuItemSelected(this);
}

void rgMenuFileItemOpenCL::resizeEvent(QResizeEvent *pEvent)
{
    UpdateFilenameLabelText();
}

void rgMenuFileItemOpenCL::showEvent(QShowEvent *pEvent)
{
    UpdateFilenameLabelText();
}

void rgMenuFileItemOpenCL::keyPressEvent(QKeyEvent* pEvent)
{
    if (pEvent->key() == Qt::Key_Escape)
    {
        m_isEscapePressed = true;
        // Hide the rename box, and display the labels
        ShowRenameControls(false);
    }
    else
    {
        // Pass the event onto the base class
        QWidget::keyPressEvent(pEvent);
    }
}

void rgMenuFileItemOpenCL::ClearEntrypointsList()
{
    assert(m_pEntryListModel != nullptr);
    if (m_pEntryListModel != nullptr)
    {
        m_pEntryListModel->ClearEntries();
    }
}

void rgMenuFileItemOpenCL::GetEntrypointNames(std::vector<std::string>& entrypointNames) const
{
    m_pEntryListModel->GetEntryPointNames(entrypointNames);
}

bool rgMenuFileItemOpenCL::GetSelectedEntrypointName(std::string& entrypointName) const
{
    bool gotEntrypointName = false;

    // Find the selected entry point in the list's selection model.
    QItemSelectionModel* pSelectionModel = ui.entrypointListView->selectionModel();
    assert(pSelectionModel != nullptr);
    if (pSelectionModel != nullptr)
    {
        // 1. Look for currently selected entry in the Selection Model.
        QModelIndex selectedEntrypointIndex = pSelectionModel->currentIndex();
        if (selectedEntrypointIndex.isValid())
        {
            // Extract and return the entry point name from the list.
            QVariant entrypointNameData = m_pEntryListModel->GetEntryItemModel()->data(selectedEntrypointIndex);
            entrypointName = entrypointNameData.toString().toStdString();
            gotEntrypointName = true;
        }
        else
        {
            // 2. If the Selection Model is empty (it may have been cleared after the build), check the last selected entry name.
            if (!m_lastSelectedEntryName.empty())
            {
                entrypointName = m_lastSelectedEntryName;
                gotEntrypointName = true;
            }
        }
    }

    return gotEntrypointName;
}

void rgMenuFileItemOpenCL::SetHovered(bool isHovered)
{
    // Set "hovered" property to be utilized by this widget's stylesheet.
    ui.itemBackground->setProperty(STR_FILE_MENU_PROPERTY_HOVERED, isHovered);

    // Repolish the widget to ensure the style gets updated.
    ui.itemBackground->style()->unpolish(ui.itemBackground);
    ui.itemBackground->style()->polish(ui.itemBackground);
}

void rgMenuFileItemOpenCL::SetCurrent(bool isCurrent)
{
    // Set "current" property to be utilized by this widget's stylesheet.
    ui.itemBackground->setProperty(STR_FILE_MENU_PROPERTY_CURRENT, isCurrent);

    // Repolish the widget to ensure the style gets updated.
    ui.itemBackground->style()->unpolish(ui.itemBackground);
    ui.itemBackground->style()->polish(ui.itemBackground);

    // Toggle the visibility of the entrypoints list based on if the item is current or not.
    ShowEntrypointsList(isCurrent);
}

void rgMenuFileItemOpenCL::UpdateFilenameLabelText()
{
    std::string text = m_filename;
    std::string displayText;

    // Determine suffix based on whether or not the file is saved.
    if (!m_isSaved)
    {
        text += STR_UNSAVED_FILE_SUFFIX;
    }

    // Get available space.
    const int availableSpace = ui.filenameDisplayLayout->contentsRect().width();

    // Get and set display text.
    rgUtils::GetDisplayText(text, displayText, availableSpace, ui.filenameLabel, gs_TEXT_TRUNCATE_LENGTH_BACK_OPENCL);
    ui.filenameLabel->setText(displayText.c_str());

    // Set the full path as a tooltip.
    this->setToolTip(m_fullFilepath.c_str());
}

QLineEdit* rgMenuFileItemOpenCL::GetRenameLineEdit()
{
    return ui.filenameLineEdit;
}

QLabel* rgMenuFileItemOpenCL::GetItemLabel()
{
    return ui.filenameLabel;
}

void rgMenuFileItemOpenCL::UpdateFilepath(const std::string& newFilepath)
{
    m_fullFilepath = newFilepath;

    // Only display the filename in the interface- not the full path to the file.
    bool isOk = rgUtils::ExtractFileName(newFilepath, m_filename);
    if (isOk == false)
    {
        // Attempt to display as much of the full file path as possible.
        m_filename = newFilepath;
    }

    // Update the view to display the latest filename.
    UpdateFilenameLabelText();
}

void rgMenuFileItemOpenCL::ConnectSignals()
{
    // Connect the item's close button.
    bool isConnected = connect(ui.closeButton, &QPushButton::clicked, this, &rgMenuFileItemOpenCL::HandleRemoveItemRequest);
    assert(isConnected);

    // Connect the filename QLineEdit signals, so the user can confirm a rename by pressing Return.
    isConnected = connect(ui.filenameLineEdit, &QLineEdit::returnPressed, this, &rgMenuFileItemOpenCL::HandleEnterPressed);
    assert(isConnected);

    // Connect the entry point list selected item changed signal.
    isConnected = connect(ui.entrypointListView, &QTreeView::clicked, this, &rgMenuFileItemOpenCL::HandleEntrypointClicked);
    assert(isConnected);

    // Slot/signal for the tree view.
    isConnected = connect(ui.entrypointListView, &QTreeView::entered, this, &rgMenuFileItemOpenCL::HandleTableItemEntered);
    assert(isConnected);

    // Connect the remove file context menu with the remove item request signal.
    isConnected = connect(m_contextMenuActions.pRemoveFile, &QAction::triggered, this, &rgMenuFileItemOpenCL::HandleRemoveItemRequest);
    assert(isConnected);
}

void rgMenuFileItemOpenCL::InitializeEntrypointsList()
{
    // Initialize the Expand/Contract icons. Use the expand icon by default.
    m_pEntryListModel = new rgMenuItemEntryListModel(this);
    ui.entrypointListView->setModel(m_pEntryListModel->GetEntryItemModel());
    ui.entrypointListView->hide();

    // Set the widget width so the entry point names get truncated correctly.
    m_pEntryListModel->SetEntryPointWidgetWidth(ui.entrypointListView->contentsRect().width());

    // Set the widget.
    m_pEntryListModel->SetEntryPointTreeWidget(ui.entrypointListView);

    m_pEntrypointStyleDelegate = new rgEntrypointItemStyleDelegate(ui.entrypointListView);
    ui.entrypointListView->setItemDelegate(m_pEntrypointStyleDelegate);
}

void rgMenuFileItemOpenCL::ShowEntrypointsList(bool showList)
{
    // Toggle the visibility of the entry point list.
    if (showList)
    {
        QStandardItemModel* pEntrypointListModel = m_pEntryListModel->GetEntryItemModel();
        if (pEntrypointListModel != nullptr)
        {
            bool isEntrypointListEnabled = false;

            // Will the parent menu allow expanding a file item's entry point list?
            rgMenuOpenCL* pParentFileMenu = static_cast<rgMenuOpenCL*>(GetParentMenu());
            assert(pParentFileMenu != nullptr);
            if (pParentFileMenu != nullptr)
            {
                isEntrypointListEnabled = pParentFileMenu->GetIsShowEntrypointListEnabled();
            }

            // Show the entry point list if it's allowed.
            if (isEntrypointListEnabled)
            {
                // Show the entrypoints list only when it's non-empty.
                if (pEntrypointListModel->rowCount() > 0)
                {
                    ui.entrypointListView->show();
                }
            }
        }
    }
    else
    {
        ui.entrypointListView->hide();
    }
}

void rgMenuFileItemOpenCL::SwitchToEntrypointByName(const std::string& displayName)
{
    // Get the list of entry point names for this file item.
    std::vector<std::string> entrypointNames;
    GetEntrypointNames(entrypointNames);

    int selectedEntrypointIndex = 0;
    bool foundEntryPoint = false;
    for (const std::string& currentEntry : entrypointNames)
    {
        if (displayName.compare(currentEntry) == 0)
        {
            foundEntryPoint = true;
            break;
        }
        selectedEntrypointIndex++;
    }

    // Compute the model index for the selected entrypoint's row, and set the selection in the selection model.
    if (foundEntryPoint)
    {
        QItemSelectionModel* pSelectionModel = ui.entrypointListView->selectionModel();
        assert(pSelectionModel != nullptr);
        if (pSelectionModel != nullptr)
        {
            QModelIndex selectedRowIndex = m_pEntryListModel->GetEntryItemModel()->index(selectedEntrypointIndex, 0);
            pSelectionModel->setCurrentIndex(selectedRowIndex, QItemSelectionModel::SelectCurrent);
            m_lastSelectedEntryName = entrypointNames[selectedEntrypointIndex];
        }
    }
}

void rgMenuFileItemOpenCL::UpdateBuildOutputs(const std::vector<rgEntryOutput>& entryOutputs)
{
    std::string currentDisplayEntrypointName;
    bool isEntrypointSelected = GetSelectedEntrypointName(currentDisplayEntrypointName);

    assert(m_pEntryListModel != nullptr);
    if (m_pEntryListModel != nullptr)
    {
        // Clear the existing list of outputs.
        m_pEntryListModel->ClearEntries();

        if (!entryOutputs.empty())
        {
            // Add a new item in the list for each build output entry.
            for (const rgEntryOutput& entryOutput : entryOutputs)
            {
                m_pEntryListModel->AddEntry(entryOutput.m_entrypointName);
            }

            // Update the entry point table view height.
            ui.entrypointListView->AdjustTreeSize(m_pEntryListModel->GetEntryItemModel());

            std::vector<std::string> entrypointNames;
            GetEntrypointNames(entrypointNames);
            if (!entrypointNames.empty())
            {
                bool selectFirstEntrypoint = false;
                if (isEntrypointSelected)
                {
                    // Attempt to re-select the same entry point from the new build outputs.
                    auto entryIter = std::find(entrypointNames.begin(), entrypointNames.end(), currentDisplayEntrypointName);
                    if (entryIter != entrypointNames.end())
                    {
                        // Re-select the previously selected entrypoint.
                        SwitchToEntrypointByName(m_pEntryListModel->GetEntryPointName(currentDisplayEntrypointName));
                    }
                    else
                    {
                        // Select the first entry point by default since the previous selection no longer exists.
                        selectFirstEntrypoint = true;
                    }
                }
                else
                {
                    // Select the first entry point by default since there was no selection previously.
                    selectFirstEntrypoint = true;
                }

                // Fall back to selecting the first entrypoint.
                if (selectFirstEntrypoint)
                {
                    // If the user didn't have an entry point selected previously, automatically select the first entrypoint.
                    currentDisplayEntrypointName = entrypointNames[0];

                    const std::string& inputFilePath = GetFilename();
                    emit SelectedEntrypointChanged(inputFilePath, currentDisplayEntrypointName);
                }
            }
        }
    }
}

void rgMenuFileItemOpenCL::HandleRemoveItemRequest()
{
    emit MenuItemCloseButtonClicked(this->GetFilename());
}

void rgMenuFileItemOpenCL::HandleEntrypointClicked()
{
    // Signal to the menu that the user has clicked within this item. This will switch the current file.
    emit MenuItemSelected(this);

    // Is the selected item valid?
    QItemSelectionModel* pSelectionModel = ui.entrypointListView->selectionModel();
    if (pSelectionModel->currentIndex().isValid())
    {
        // Pull the filename out of the selected item.
        const std::string& filePath = GetFilename();
        int selectedRow = pSelectionModel->currentIndex().row();

        // Get a list of all the entry point names for the input file.
        std::vector<std::string> entrypointNames;
        GetEntrypointNames(entrypointNames);

        bool isValidRow = selectedRow < entrypointNames.size();
        assert(isValidRow);
        if (isValidRow)
        {
            // Emit a signal indicating that the selected entry point has changed.
            emit SelectedEntrypointChanged(filePath, entrypointNames[selectedRow]);
        }
    }
}

void rgMenuFileItemOpenCL::HandleTableItemEntered(const QModelIndex& modelIndex)
{
    // Set the cursor if modelIndex valid.
    if (modelIndex.isValid())
    {
        ui.entrypointListView->setCursor(Qt::PointingHandCursor);
    }
    else
    {
        ui.entrypointListView->setCursor(Qt::ArrowCursor);
    }
}

void rgMenuFileItemOpenCL::SetCursor()
{
    // Set the cursor to pointing hand cursor.
    ui.closeButton->setCursor(Qt::PointingHandCursor);
}

void rgMenuFileItemOpenCL::HandleProjectBuildSuccess()
{
    // Change the style sheet when the project built successfully.
    setStyleSheet(s_FILE_MENU_ITEM_COLOR);
}
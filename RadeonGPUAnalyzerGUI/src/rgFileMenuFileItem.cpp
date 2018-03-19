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
#include <RadeonGPUAnalyzerGUI/include/qt/rgFileMenu.h>
#include <RadeonGPUAnalyzerGUI/include/qt/rgFileMenuFileItem.h>
#include <RadeonGPUAnalyzerGUI/include/rgUtils.h>
#include <RadeonGPUAnalyzerGUI/include/rgDefinitions.h>

// A delegate used to style a file item's entrypoint list.
class rgEntrypointItemStyleDelegate : public QStyledItemDelegate
{
public:
    rgEntrypointItemStyleDelegate(QObject* pParent = nullptr)
        : QStyledItemDelegate(pParent) {}

    // A custom row painter for the entrypoint list.
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

rgFileMenuItemEntryListModel::rgFileMenuItemEntryListModel(QWidget* pParent)
    : QWidget(pParent)
{
    // Initialize the item model.
    m_pEntrypointItemModel = new QStandardItemModel(pParent);

    // Figure out which API is being used. This will determine which text is used for the item's entrypoints list.
    rgProjectAPI currentAPI = rgConfigManager::Instance().GetCurrentAPI();
    const std::string entrypointLabelText = rgUtils::GetEntrypointsNameString(currentAPI);

    // Add the column header text.
    m_pEntrypointItemModel->setHorizontalHeaderItem(0, new QStandardItem(entrypointLabelText.c_str()));
}

QStandardItemModel* rgFileMenuItemEntryListModel::GetEntryItemModel() const
{
    return m_pEntrypointItemModel;
}

void rgFileMenuItemEntryListModel::AddEntry(const std::string& entrypointName)
{
    int currentRowCount = m_pEntrypointItemModel->rowCount();
    int newRowCount = currentRowCount + 1;

    // Update the number of row items in the model.
    m_pEntrypointItemModel->setRowCount(newRowCount);

    // Set the data for the new item row, and left-align the entrypoint name string.
    QModelIndex modelIndex = m_pEntrypointItemModel->index(currentRowCount, 0);
    m_pEntrypointItemModel->setData(modelIndex, QString(entrypointName.c_str()));
    m_pEntrypointItemModel->setData(modelIndex, Qt::AlignLeft, Qt::TextAlignmentRole);
    m_pEntrypointItemModel->dataChanged(modelIndex, modelIndex);

    // Set the tooltip for the new entrypoint item.
    std::stringstream tooltipText;
    tooltipText << STR_MENU_ITEM_ENTRYPOINT_TOOLTIP_TEXT;
    tooltipText << entrypointName;
    m_pEntrypointItemModel->setData(modelIndex, tooltipText.str().c_str(), Qt::ToolTipRole);
}

void rgFileMenuItemEntryListModel::ClearEntries()
{
    // Clear the model data by removing all existing rows.
    int numRows = m_pEntrypointItemModel->rowCount();
    m_pEntrypointItemModel->removeRows(0, numRows);
}

rgFileMenuFileItem::rgFileMenuFileItem(const std::string& fileFullPath, rgFileMenu* pParent) :
    rgFileMenuItem(pParent),
    m_fullFilepath(fileFullPath)
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

    // Initialize the context menu for right-clicks on the item.
    InitializeContextMenu();

    // Initialize the entrypoint list.
    InitializeEntrypointsList();

    // Connect signals for the file item.
    ConnectSignals();

    // Enable mouse tracking for the entry point list view.
    ui.entrypointListView->setMouseTracking(true);

    // Set mouse pointer to pointing hand cursor.
    SetCursor();
}

void rgFileMenuFileItem::enterEvent(QEvent* pEvent)
{
    ui.closeButton->show();
}

void rgFileMenuFileItem::leaveEvent(QEvent* pEvent)
{
    ui.closeButton->hide();
}

void rgFileMenuFileItem::mouseDoubleClickEvent(QMouseEvent* pEvent)
{
    // On double-click, allow the user to re-name the item's filename.
    ShowRenameControls(true);
}

void rgFileMenuFileItem::mousePressEvent(QMouseEvent* pEvent)
{
    emit MenuItemSelected(this);
}

void rgFileMenuFileItem::resizeEvent(QResizeEvent *pEvent)
{
    RefreshLabelText();
}

void rgFileMenuFileItem::showEvent(QShowEvent *pEvent)
{
    RefreshLabelText();
}

void rgFileMenuFileItem::keyPressEvent(QKeyEvent* pEvent)
{
    if (pEvent->key() == Qt::Key_Escape)
    {
        m_escapePressed = true;
        // Hide the rename box, and display the labels
        ShowRenameControls(false);
    }
    else
    {
        // Pass the event onto the base class
        QWidget::keyPressEvent(pEvent);
    }
}

void rgFileMenuFileItem::ClearEntrypointsList()
{
    assert(m_pEntryListModel != nullptr);
    if (m_pEntryListModel != nullptr)
    {
        m_pEntryListModel->ClearEntries();
    }
}

void rgFileMenuFileItem::GetEntrypointNames(std::vector<std::string>& entrypointNames)
{
    // Step through each row in the entrypoint item model.
    QStandardItemModel* pItemModel = m_pEntryListModel->GetEntryItemModel();
    for (int entrypointIndex = 0; entrypointIndex < pItemModel->rowCount(); ++entrypointIndex)
    {
        // Find the index of the selected row.
        QModelIndex itemIndex = pItemModel->index(entrypointIndex, 0);
        if (itemIndex.isValid())
        {
            // Extract the entrypoint name string and add it to the output list.
            QString entryString = pItemModel->data(itemIndex).toString();
            entrypointNames.push_back(entryString.toStdString());
        }
    }
}

bool rgFileMenuFileItem::GetSelectedEntrypointName(std::string& entrypointName) const
{
    bool gotEntrypointName = false;

    // Find the selected entrypoint in the list's selection model.
    QItemSelectionModel* pSelectionModel = ui.entrypointListView->selectionModel();
    assert(pSelectionModel != nullptr);
    if (pSelectionModel != nullptr)
    {
        // 1. Look for currently selected entry in the Selection Model.
        QModelIndex selectedEntrypointIndex = pSelectionModel->currentIndex();
        if (selectedEntrypointIndex.isValid())
        {
            // Extract and return the entrypoint name from the list.
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

const std::string& rgFileMenuFileItem::GetFilename() const
{
    return m_fullFilepath;
}

void rgFileMenuFileItem::SetIsSelected(bool isSelected)
{
    // Set "selected" property to be utilized by this widget's stylesheet.
    ui.itemBackground->setProperty("selected", isSelected);

    // Repolish the widget to ensure the style gets updated.
    ui.itemBackground->style()->unpolish(ui.itemBackground);
    ui.itemBackground->style()->polish(ui.itemBackground);

    // Toggle the visibility of the entrypoints list based on if the item is selected or not.
    ShowEntrypointsList(isSelected);
}

void rgFileMenuFileItem::SetIsSaved(bool isSaved)
{
    // Only refresh if there is a change.
    if (m_isSaved != isSaved)
    {
        m_isSaved = isSaved;
        RefreshLabelText();
    }
    else
    {
        m_isSaved = isSaved;
    }
}

void rgFileMenuFileItem::ShowRenameControls(bool isRenaming)
{
    // Swap the visibility of the filename label and line edit.
    if (isRenaming)
    {
        ui.filenameLabel->setVisible(false);
        ui.filenameLineEdit->setText(m_filename.c_str());
        ui.filenameLineEdit->setVisible(true);

        std::string filenameOnly;
        bool gotFilename = rgUtils::ExtractFileName(m_filename, filenameOnly, false);
        assert(gotFilename);

        // Focus on the widget with the filename selected so the user can start typing immediately.
        ui.filenameLineEdit->setFocus();
        ui.filenameLineEdit->setSelection(0, static_cast<int>(filenameOnly.length()));

        // Set cursor to IBeam cursor.
        setCursor(Qt::IBeamCursor);
    }
    else
    {
        ui.filenameLabel->setVisible(true);
        ui.filenameLineEdit->setVisible(false);

        // Set cursor to Arrow cursor.
        setCursor(Qt::ArrowCursor);
    }
}

void rgFileMenuFileItem::RefreshLabelText()
{
    std::string text = m_filename;

    // Determine suffix based on whether or not the file is saved.
    if (!m_isSaved)
    {
        text += STR_UNSAVED_FILE_SUFFIX;
    }

    // Get available space.
    const int AVAILABLE_SPACE = ui.filenameDisplayLayout->contentsRect().width();

    // Get file extension (we already added the isSaved suffix so it will be included in the extension).
    std::string extension;
    rgUtils::ExtractFileExtension(text, extension);

    // Always include the file extension (the +1 is to include the '.' from the file extension too).
    const int NUM_BACK_CHARS = gs_TEXT_TRUNCATE_LENGTH_BACK + static_cast<unsigned>(extension.length() + 1);
    const int NUM_FRONT_CHARS = gs_TEXT_TRUNCATE_LENGTH_FRONT;

    // Truncate filename within available space to get display text.
    std::string displayText = rgUtils::TruncateString(text, NUM_FRONT_CHARS, NUM_BACK_CHARS, AVAILABLE_SPACE, ui.filenameLabel->font(), rgUtils::EXPAND_NONE);

    // Set label text.
    ui.filenameLabel->setText(displayText.c_str());

    // Set the full path as a tooltip.
    this->setToolTip(m_fullFilepath.c_str());
}

bool rgFileMenuFileItem::RenameFile()
{
    bool isFileRenamed = false;

    if (!m_escapePressed)
    {
        // The new filename is whatever the user left in the renaming QLineEdit.
        const std::string newFilename = ui.filenameLineEdit->text().toStdString();

        // Only attempt a rename if the user has altered the filename string.
        bool filenameChanged = m_filename.compare(newFilename) != 0;

        // If the current filename differs from what the user left in the QLineEdit, update the filename.
        if (filenameChanged)
        {
            // The renamed file will live in the same location as the old one.
            std::string fileFolderPath;
            bool gotFolder = rgUtils::ExtractFileDirectory(m_fullFilepath, fileFolderPath);
            assert(gotFolder);

            // Generate the full path to where the new file lives.
            std::string newFilepath;
            rgUtils::AppendFileNameToPath(fileFolderPath, newFilename, newFilepath);

            if (rgUtils::IsValidFileName(newFilename))
            {
                if (!rgUtils::IsFileExists(newFilepath))
                {
                    // Rename the file on disk.
                    rgUtils::RenameFile(m_fullFilepath, newFilepath);

                    // Signal to the file menu that the file path has changed.
                    emit FileRenamed(m_fullFilepath, newFilepath);

                    // Update the file path so the item will display the correct filename in the menu.
                    UpdateFilepath(newFilepath);

                    // The file on disk was successfully renamed.
                    isFileRenamed = true;
                }
                else
                {
                    // Show an error message stating that the rename failed because a
                    // file with the same name already exists in the same location.
                    std::stringstream msg;
                    msg << STR_ERR_CANNOT_RENAME_FILE_A;
                    msg << newFilename;
                    msg << STR_ERR_CANNOT_RENAME_FILE_B_ALREADY_EXISTS;

                    rgUtils::ShowErrorMessageBox(msg.str().c_str(), this);
                }
            }
            else
            {
                std::stringstream msg;
                // Show an error message stating that the rename failed because
                // the given name is invalid.
                if (newFilename.empty())
                {
                    msg << STR_ERR_CANNOT_RENAME_FILE_BLANK_FILENAME;
                }
                else
                {
                    msg << STR_ERR_CANNOT_RENAME_FILE_A;
                    msg << newFilename;
                    msg << STR_ERR_CANNOT_RENAME_FILE_B_ILLEGAL_FILENAME;
                }

                rgUtils::ShowErrorMessageBox(msg.str().c_str(), this);
            }
        }

        // Re-enable the filename editing controls, since the user gets another chance to attempt a rename.
        if (!isFileRenamed && filenameChanged)
        {
            // Toggle the item back to being read-only, showing the updated filename.
            ShowRenameControls(true);
        }
        else
        {
            // Toggle the item back to being read-only, showing the updated filename.
            ShowRenameControls(false);
        }
    }
    else
    {
        m_escapePressed = false;
    }
    return isFileRenamed;
}

void rgFileMenuFileItem::UpdateFilepath(const std::string& newFilepath)
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
    RefreshLabelText();
}

void rgFileMenuFileItem::ConnectSignals()
{
    // Connect the item's close button.
    bool isConnected = connect(ui.closeButton, &QPushButton::clicked, this, &rgFileMenuFileItem::HandleCloseButtonClicked);
    assert(isConnected);

    // Connect the filename QLineEdit signals, so the user can confirm a rename by pressing Return.
    isConnected = connect(ui.filenameLineEdit, &QLineEdit::returnPressed, this, &rgFileMenuFileItem::HandleRenameFinished);
    assert(isConnected);

    // Connect the entrypoint list selected item changed signal.
    isConnected = connect(ui.entrypointListView, &QTreeView::clicked, this, &rgFileMenuFileItem::HandleEntrypointClicked);
    assert(isConnected);

    // Connect a FocusChanged handler so we know when a filename change is completed.
    isConnected = connect(qApp, &QApplication::focusChanged, this, &rgFileMenuFileItem::HandleFocusChanged);
    assert(isConnected);

    // Connect the item's "Open in file browser" menu item.
    isConnected = connect(m_pOpenContainingFolderAction, &QAction::triggered, this, &rgFileMenuFileItem::HandleOpenInFileBrowserClicked);
    assert(isConnected);

    // Connect the item's "Rename" menu item.
    isConnected = connect(m_pRenameFileAction, &QAction::triggered, this, &rgFileMenuFileItem::HandleRenameClicked);
    assert(isConnected);

    // Connect the item's "Rename" menu item.
    isConnected = connect(m_pRemoveFileAction, &QAction::triggered, this, &rgFileMenuFileItem::HandleCloseButtonClicked);
    assert(isConnected);

    // Connect the handler responsible for showing the item's context menu.
    isConnected = connect(this, &QWidget::customContextMenuRequested, this, &rgFileMenuFileItem::HandleOpenContextMenu);
    assert(isConnected);

    // Slot/signal for the tree view.
    isConnected = connect(ui.entrypointListView, &QTreeView::entered, this, &rgFileMenuFileItem::HandleTableItemEntered);
    assert(isConnected);

    this->setContextMenuPolicy(Qt::CustomContextMenu);
}

void rgFileMenuFileItem::InitializeEntrypointsList()
{
    // Initialize the Expand/Contract icons. Use the expand icon by default.
    m_pEntryListModel = new rgFileMenuItemEntryListModel(this);
    ui.entrypointListView->setModel(m_pEntryListModel->GetEntryItemModel());
    ui.entrypointListView->hide();

    m_pEntrypointStyleDelegate = new rgEntrypointItemStyleDelegate(ui.entrypointListView);
    ui.entrypointListView->setItemDelegate(m_pEntrypointStyleDelegate);
}

void rgFileMenuFileItem::InitializeContextMenu()
{
    // Create the context menu instance.
    m_pContextMenu = new QMenu(this);

    // Create the menu items to insert into the context menu.
    m_pOpenContainingFolderAction = new QAction(STR_FILE_CONTEXT_MENU_OPEN_CONTAINING_FOLDER, this);
    m_pContextMenu->addAction(m_pOpenContainingFolderAction);

    // Add a separator between the current menu items.
    m_pContextMenu->addSeparator();

    // Create the rename action and add it to the menu.
    m_pRenameFileAction = new QAction(STR_FILE_CONTEXT_MENU_RENAME_FILE, this);
    m_pContextMenu->addAction(m_pRenameFileAction);

    // Create the remove action and add it to the menu.
    m_pRemoveFileAction = new QAction(STR_FILE_CONTEXT_MENU_REMOVE_FILE, this);
    m_pContextMenu->addAction(m_pRemoveFileAction);
}

void rgFileMenuFileItem::OpenContextMenu()
{
    QPoint centerPoint(width() / 2, height() / 2);

    // Open the context menu on a default centered point.
    HandleOpenContextMenu(centerPoint);
}

void rgFileMenuFileItem::ShowEntrypointsList(bool showList)
{
    // Toggle the visibility of the entrypoint list.
    if (showList)
    {
        QStandardItemModel* pEntrypointListModel = m_pEntryListModel->GetEntryItemModel();
        if (pEntrypointListModel != nullptr)
        {
            bool isEntrypointListEnabled = false;

            // Will the parent menu allow expanding a file item's entrypoint list?
            rgFileMenu* pParentFileMenu = GetParentMenu();
            assert(pParentFileMenu != nullptr);
            if (pParentFileMenu != nullptr)
            {
                isEntrypointListEnabled = pParentFileMenu->GetIsShowEntrypointListEnabled();
            }

            // Show the entrypoint list if it's allowed.
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

void rgFileMenuFileItem::SwitchToEntrypointByName(const std::string& entrypointName)
{
    // Get the list of entrypoint names for this file item.
    std::vector<std::string> entrypointNames;
    GetEntrypointNames(entrypointNames);

    int selectedEntrypointIndex = 0;
    for (const std::string& currentName : entrypointNames)
    {
        if (entrypointName.compare(currentName) == 0)
        {
            break;
        }
        selectedEntrypointIndex++;
    }

    // Compute the model index for the selected entrypoint's row, and set the selection in the selection model.
    QItemSelectionModel* pSelectionModel = ui.entrypointListView->selectionModel();
    assert(pSelectionModel != nullptr);
    if (pSelectionModel != nullptr)
    {
        QModelIndex selectedRowIndex = m_pEntryListModel->GetEntryItemModel()->index(selectedEntrypointIndex, 0);
        pSelectionModel->setCurrentIndex(selectedRowIndex, QItemSelectionModel::SelectCurrent);
        m_lastSelectedEntryName = entrypointName;
    }
}

void rgFileMenuFileItem::UpdateBuildOutputs(const std::vector<rgEntryOutput>& entryOutputs)
{
    std::string currentEntrypointName;
    bool isEntrypointSelected = GetSelectedEntrypointName(currentEntrypointName);

    // Clear the existing list of outputs.
    m_pEntryListModel->ClearEntries();

    if (!entryOutputs.empty())
    {
        // Add a new item in the list for each build output entry.
        for (const rgEntryOutput& entryOutput : entryOutputs)
        {
            m_pEntryListModel->AddEntry(entryOutput.m_kernelName);
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
                // Attempt to re-select the same entrypoint from the new build outputs.
                auto entryIter = std::find(entrypointNames.begin(), entrypointNames.end(), currentEntrypointName);
                if (entryIter != entrypointNames.end())
                {
                    // Re-select the previously selected entrypoint.
                    SwitchToEntrypointByName(currentEntrypointName);
                }
                else
                {
                    // Select the first entrypoint by default since the previous selection no longer exists.
                    selectFirstEntrypoint = true;
                }
            }
            else
            {
                // Select the first entrypoint by default since there was no selection previously.
                selectFirstEntrypoint = true;
            }

            // Fall back to selecting the first entrypoint.
            if (selectFirstEntrypoint)
            {
                // If the user didn't have an entrypoint selected previously, automatically select the first entrypoint.
                currentEntrypointName = entrypointNames[0];

                const std::string& inputFilePath = GetFilename();
                emit SelectedEntrypointChanged(inputFilePath, currentEntrypointName);
            }
        }
    }
}

void rgFileMenuFileItem::HandleRenameFinished()
{
    // Remove focus from the filename editor, which will trigger a rename.
    rgUtils::FocusOnFirstValidAncestor(ui.filenameLineEdit);

    // Emit a signal indicating that the rename is finished.
    emit FileRenamed(m_fullFilepath, m_fullFilepath);
}

void rgFileMenuFileItem::HandleFocusChanged(QWidget* pOld, QWidget* pNow)
{
    Q_UNUSED(pNow);

    if (pOld != nullptr)
    {
        // If the control that lost focus was the renaming QLineEdit, finish the item rename.
        if (pOld == ui.filenameLineEdit)
        {
            RenameFile();
        }
    }
}

void rgFileMenuFileItem::HandleCloseButtonClicked()
{
    emit MenuItemCloseButtonClicked(this->GetFilename());
}

void rgFileMenuFileItem::HandleOpenInFileBrowserClicked()
{
    std::string fileDirectory;
    bool gotDirectory = rgUtils::ExtractFileDirectory(GetFilename(), fileDirectory);
    assert(gotDirectory);

    if (gotDirectory)
    {
        // Open a system file browser window pointing to the given directory.
        rgUtils::OpenFolderInFileBrowser(fileDirectory);
    }
}

void rgFileMenuFileItem::HandleRenameClicked()
{
    // Show the file item renaming controls.
    ShowRenameControls(true);
}

void rgFileMenuFileItem::HandleOpenContextMenu(const QPoint& widgetClickPosition)
{
    // Convert the widget's local click position to the global screen position.
    const QPoint clickPoint = mapToGlobal(widgetClickPosition);

    // Open the context menu at the user's click position.
    m_pContextMenu->exec(clickPoint);
}

void rgFileMenuFileItem::HandleEntrypointClicked()
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

        // Get a list of all the entrypoint names for the input file.
        std::vector<std::string> entrypointNames;
        GetEntrypointNames(entrypointNames);

        bool isValidRow = selectedRow < entrypointNames.size();
        assert(isValidRow);
        if (isValidRow)
        {
            // Emit a signal indicating that the selected entrypoint has changed.
            emit SelectedEntrypointChanged(filePath, entrypointNames[selectedRow]);
        }
    }
}

void rgFileMenuFileItem::HandleTableItemEntered(const QModelIndex& modelIndex)
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

void rgFileMenuFileItem::SetCursor()
{
    // Set the cursor to pointing hand cursor.
    ui.closeButton->setCursor(Qt::PointingHandCursor);
}
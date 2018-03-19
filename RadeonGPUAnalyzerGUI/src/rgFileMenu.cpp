// C++.
#include <cassert>
#include <sstream>

// Qt.
#include <QWidget>
#include <QBoxLayout>

// Local.
#include <RadeonGPUAnalyzerGUI/include/qt/rgAddCreateMenuItem.h>
#include <RadeonGPUAnalyzerGUI/include/qt/rgBuildView.h>
#include <RadeonGPUAnalyzerGUI/include/qt/rgFileMenu.h>
#include <RadeonGPUAnalyzerGUI/include/qt/rgFileMenuBuildSettingsItem.h>
#include <RadeonGPUAnalyzerGUI/include/qt/rgFileMenuFileItem.h>
#include <RadeonGPUAnalyzerGUI/include/qt/rgFileMenuItem.h>
#include <RadeonGPUAnalyzerGUI/include/qt/rgFileMenuTitlebar.h>
#include <RadeonGPUAnalyzerGUI/include/qt/rgIgnoreTabFocusEventFilter.h>
#include <RadeonGPUAnalyzerGUI/include/rgDefinitions.h>
#include <RadeonGPUAnalyzerGUI/include/rgStringConstants.h>
#include <RadeonGPUAnalyzerGUI/include/rgUtils.h>
#include <QtCommon/Scaling/ScalingManager.h>

// Indices for special case file items.
enum FileMenuFocusItems
{
    FOCUS_ADD_BUTTON,
    FOCUS_CREATE_BUTTON,
    FOCUS_BUILD_SETTINGS_BUTTON
};

// Stylesheet for add/create buttons when in focus.
static const char* s_BUTTON_FOCUS_STYLESHEET = "QPushButton { border: 1px solid #6666FF; margin: 1px; background: lightGray;}";

rgFileMenu::rgFileMenu(QWidget* pParent) :
    QFrame(pParent),
    m_focusIndex(0)
{
    ui.setupUi(this);
    m_pLayout = ui.fileMenuVerticalLayout;

    // Create the default items in the file menu.
    InitializeDefaultMenuItems();

    // Create file menu actions.
    CreateActions();

    // Make the menu as wide as the items.
    const int height = m_pDefaultMenuItem->height();
    this->resize(m_pDefaultMenuItem->width(), 2 * (height));

    // Apply style from stylesheet.
    rgUtils::LoadAndApplyStyle(STR_FILE_MENU_STYLESHEET_FILE, this);

    // Install filter to ignore tab input for switching focus.
    installEventFilter(&rgIgnoreTabFocusEventFilter::Get());
}

void rgFileMenu::CreateActions()
{
    // File menu next item.
    m_pNextItemAction = new QAction(this);
    m_pNextItemAction->setShortcut(QKeySequence(gs_ACTION_HOTKEY_FILE_MENU_NEXT_ITEM));
    m_pNextItemAction->setShortcutContext(Qt::WidgetShortcut);

    addAction(m_pNextItemAction);
    bool isConnected = connect(m_pNextItemAction, &QAction::triggered, this, &rgFileMenu::HandleNextItemAction);
    assert(isConnected);

    // File menu previous item.
    m_pPrevItemAction = new QAction(this);
    m_pPrevItemAction->setShortcut(QKeySequence(gs_ACTION_HOTKEY_FILE_MENU_PREV_ITEM));
    m_pPrevItemAction->setShortcutContext(Qt::WidgetShortcut);

    addAction(m_pPrevItemAction);
    isConnected = connect(m_pPrevItemAction, &QAction::triggered, this, &rgFileMenu::HandlePreviousItemAction);
    assert(isConnected);

    // File menu context menu shortcut.
    m_pOpenContextMenuAction = new QAction(this);
    m_pOpenContextMenuAction->setShortcut(QKeySequence(gs_ACTION_HOTKEY_FILE_MENU_CONTEXT_MENU));
    m_pOpenContextMenuAction->setShortcutContext(Qt::WidgetShortcut);

    addAction(m_pOpenContextMenuAction);
    isConnected = connect(m_pOpenContextMenuAction, &QAction::triggered, this, &rgFileMenu::HandleOpenContextMenuAction);
    assert(isConnected);

    // File menu activate item (used to trigger add/create buttons when in focus).
    m_activateItemAction = new QAction(this);
    QList<QKeySequence> shortcutActions;
    shortcutActions.push_back(QKeySequence(gs_ACTION_HOTKEY_FILE_MENU_ACTIVATE_RETURN));
    shortcutActions.push_back(QKeySequence(gs_ACTION_HOTKEY_FILE_MENU_ACTIVATE_ENTER));
    m_activateItemAction->setShortcuts(shortcutActions);
    m_activateItemAction->setShortcutContext(Qt::WidgetShortcut);

    addAction(m_activateItemAction);
    isConnected = connect(m_activateItemAction, &QAction::triggered, this, &rgFileMenu::HandleActivateItemAction);
    assert(isConnected);

    // File menu rename selected file.
    m_renameSelectedFileAction = new QAction(this);
    m_renameSelectedFileAction->setShortcut(QKeySequence(gs_ACTION_HOTKEY_FILE_MENU_RENAME));
    m_renameSelectedFileAction->setShortcutContext(Qt::WidgetShortcut);

    addAction(m_renameSelectedFileAction);
    isConnected = connect(m_renameSelectedFileAction, &QAction::triggered, this, &rgFileMenu::HandleRenameSelectedFileAction);
    assert(isConnected);

    // Connect the handler responsible for signaling when the tab key has been pressed.
    isConnected = connect(&rgIgnoreTabFocusEventFilter::Get(), &rgIgnoreTabFocusEventFilter::TabPressed, this, &rgFileMenu::HandleTabFocusPressed);
    assert(isConnected);
}

void rgFileMenu::AddItem(const std::string& fullPath, bool isNewFileItem)
{
    // Extract the file name from the full file path.
    std::string fileName;
    bool isOk = rgUtils::ExtractFileName(fullPath, fileName);
    assert(isOk);
    if (isOk)
    {
        // If a file with that name hasn't been added.
        if (m_fullFilepathToMenuItem.find(fullPath) == m_fullFilepathToMenuItem.end())
        {
            // Create the menu item widget and add it to the UI and to the list.
            rgFileMenuFileItem* pNewMenuItem = new rgFileMenuFileItem(fullPath, this);
            m_menuItems.push_back(pNewMenuItem);

            // Emit a signal that indicates that the number of items in the menu change,
            // and specify that the menu is not empty.
            emit FileMenuItemCountChanged(false);

            // Add the file name and its full path to the map.
            m_fullFilepathToMenuItem[fullPath] = pNewMenuItem;

            // Insert the item just above the default "CL" menu item.
            const size_t index = (!m_menuItems.empty()) ? (m_menuItems.size() - 1) : 0;
            m_pLayout->insertWidget(static_cast<const int>(index), pNewMenuItem, Qt::AlignTop);

            // Register the object with the scaling manager.
            ScalingManager::Get().RegisterObject(pNewMenuItem);

            // Connect signals for the new file menu item.
            ConnectMenuFileItemSignals(pNewMenuItem);

            // Select the file that was just opened.
            HandleSelectedFileChanged(pNewMenuItem);

            if (isNewFileItem)
            {
                // If a file was newly-created (as opposed to being opened), display the
                // item's renaming control immediately so the user can choose a new filename.
                pNewMenuItem->ShowRenameControls(true);
            }
        }
        else
        {
            // Report the error.
            std::stringstream msg;
            msg << STR_ERR_CANNOT_ADD_FILE_A << fileName << STR_ERR_CANNOT_ADD_FILE_B;
            rgUtils::ShowErrorMessageBox(msg.str().c_str(), this);
        }
    }
}

bool rgFileMenu::OffsetCurrentFileEntrypoint(int offset)
{
    bool ret = false;

    // The offset should be a positive or negative integer. If it's not, something is wrong.
    assert(offset != 0);

    // If focus index is in the range of the menu file items, select the appropriate file item.
    bool isFocusInFileItemRange = m_focusIndex < m_menuItems.size();
    assert(isFocusInFileItemRange);
    if (isFocusInFileItemRange)
    {
        rgFileMenuFileItem* pFileItem = m_menuItems[m_focusIndex];

        // Get the list of entrypoint names in the current file.
        std::vector<std::string> entrypointNames;
        pFileItem->GetEntrypointNames(entrypointNames);

        if (!entrypointNames.empty())
        {
            // Get the name of the currently selected entrypoint.
            std::string selectedEntrypointName;
            bool isEntrypointSelected = pFileItem->GetSelectedEntrypointName(selectedEntrypointName);
            if (isEntrypointSelected && !selectedEntrypointName.empty())
            {
                // Compute the index of the currently selected entrypoint.
                auto currentIndexIter = std::find(entrypointNames.begin(), entrypointNames.end(), selectedEntrypointName);
                int selectedIndex = currentIndexIter - entrypointNames.begin();

                // Offset the index, and check that it's still a valid index in the name list.
                selectedIndex += offset;
                if (selectedIndex >= 0 && selectedIndex < entrypointNames.size())
                {
                    // Emit a signal indicating that the selected entrypoint has changed.
                    auto nextEntrypointIter = entrypointNames.begin() + selectedIndex;
                    emit SelectedEntrypointChanged(pFileItem->GetFilename(), *nextEntrypointIter);
                    ret = true;
                }
            }
        }
    }

    return ret;
}

void rgFileMenu::ClearBuildOutputs()
{
    // Step through each file item in menu and clear the entrypoint list.
    for (rgFileMenuFileItem* pFileItem : m_menuItems)
    {
        assert(pFileItem != nullptr);
        if (pFileItem != nullptr)
        {
            pFileItem->ClearEntrypointsList();
        }
    }
}

void rgFileMenu::ClearFiles()
{
    // Deselect any currently selected file, because it's about to be removed.
    DeselectFiles();

    // Collect a list of keys for each file path with an item in the file menu.
    std::vector<std::string> openFiles;
    for (auto currentItem = m_fullFilepathToMenuItem.begin(); currentItem != m_fullFilepathToMenuItem.end(); ++currentItem)
    {
        openFiles.push_back(currentItem->first);
    }

    // Step through each path and remove the item from the file menu.
    for (const std::string& currentFilePath : openFiles)
    {
        RemoveItem(currentFilePath);
    }

    // Reset the build settings item to reflect no pending changes.
    if (m_pBuildSettingsMenuItem != nullptr)
    {
        m_pBuildSettingsMenuItem->SetHasPendingChanges(false);
    }
}

void rgFileMenu::GenerateUniqueFilename(const std::string& initialFilename, std::string& uniqueFilename) const
{
    // Append a suffix number onto the end of a filename to make it unique.
    int suffix = 1;

    // Loop to generate a new filename with suffix until it's a unique item in the file menu.
    do
    {
        std::stringstream filenameStream;
        filenameStream << initialFilename;
        filenameStream << suffix;

        uniqueFilename = filenameStream.str();
        suffix++;
    } while (IsFileInMenu(uniqueFilename));
}

rgFileMenuBuildSettingsItem* rgFileMenu::GetBuildSettingsItem() const
{
    return m_pBuildSettingsMenuItem;
}

rgFileMenuFileItem* rgFileMenu::GetFileItemFromPath(const std::string& sourceFilePath) const
{
    rgFileMenuFileItem* pResultItem = nullptr;

    // Step through each file item and find the one that matches the given path.
    for (size_t fileItemIndex = 0; fileItemIndex < m_menuItems.size(); fileItemIndex++)
    {
        if (m_menuItems[fileItemIndex]->GetFilename() == sourceFilePath)
        {
            pResultItem = m_menuItems[fileItemIndex];
            break;
        }
    }

    return pResultItem;
}

bool rgFileMenu::GetIsShowEntrypointListEnabled() const
{
    return m_isShowEntrypointListEnabled;
}

rgFileMenuFileItem* rgFileMenu::GetSelectedFileItem() const
{
    return m_pSelectedFileItem;
}

std::string rgFileMenu::GetSelectedFilePath() const
{
    std::string selectedFilePath;
    if (m_pSelectedFileItem != nullptr)
    {
        selectedFilePath = m_pSelectedFileItem->GetFilename();
    }
    return selectedFilePath;
}

struct MenuItemFilenameSearcher
{
    MenuItemFilenameSearcher(const std::string& filename) : m_targetFilename(filename) {}

    // A predicate that will compare each rgFileMenuFileItem's filename to the target filename.
    bool operator()(const rgFileMenuItem* pMenuItem) const
    {
        bool isMatchFound = false;
        const rgFileMenuFileItem* pFileItem = static_cast<const rgFileMenuFileItem*>(pMenuItem);

        std::string filenameOnly;
        bool isFilenameValid = rgUtils::ExtractFileName(pFileItem->GetFilename(), filenameOnly, false);
        assert(isFilenameValid);

        if (filenameOnly.compare(m_targetFilename) == 0)
        {
            isMatchFound = true;
        }

        return isMatchFound;
    }

    // The target filename to search the list of rgFileMenuFileItems for.
    std::string m_targetFilename;
};

bool rgFileMenu::IsFileInMenu(const std::string& filename) const
{
    // Only compare the filename string, excluding the file extension.
    std::string filenameOnly;
    bool isFilenameValid = rgUtils::ExtractFileName(filename, filenameOnly, false);
    assert(isFilenameValid);

    // Search through the list of items to find if any have a matching filename.
    MenuItemFilenameSearcher filenameSearch(filename);
    auto itemIter = std::find_if(m_menuItems.begin(), m_menuItems.end(), filenameSearch);

    // Return true if there's already a menu item with a matching filename.
    return (itemIter != m_menuItems.end());
}

void rgFileMenu::RemoveItem(const std::string& fullFilename)
{
    if (!fullFilename.empty())
    {
        // Remove the item from the map.
        auto filepathIter = m_fullFilepathToMenuItem.find(fullFilename);
        if (filepathIter != m_fullFilepathToMenuItem.end())
        {
            rgFileMenuFileItem* pMenuItem = filepathIter->second;

            m_fullFilepathToMenuItem.erase(filepathIter);

            // Remove the item from the list.
            for (auto it = m_menuItems.begin(); it != m_menuItems.end(); it++)
            {
                if (*it == pMenuItem)
                {
                    m_menuItems.erase(it);
                    break;
                }
            }

            if (m_menuItems.size() == 0)
            {
                // The file menu is empty.
                emit FileMenuItemCountChanged(true);
            }

            // Remove the item from the GUI.
            m_pLayout->removeWidget(pMenuItem);

            // The widget was removed from the layout, but still exists and will be rendered.
            // It is therefore also necessary to hide the widget to make it disappear entirely.
            pMenuItem->hide();
        }
    }
}

void rgFileMenu::DeselectFiles()
{
    // If a file was already selected, deselect it.
    if (m_pSelectedFileItem != nullptr)
    {
        m_pSelectedFileItem->SetIsSelected(false);
        m_pSelectedFileItem = nullptr;
    }
}

void rgFileMenu::SetItemIsSaved(const std::string& fullFilename, bool isSaved)
{
    // Find the item in the map.
    auto filepathIter = m_fullFilepathToMenuItem.find(fullFilename);
    if (filepathIter != m_fullFilepathToMenuItem.end())
    {
        rgFileMenuFileItem* pMenuItem = filepathIter->second;
        bool isItemValid = (pMenuItem != nullptr);
        assert(isItemValid);

        if (isItemValid)
        {
            // Set item saved state.
            pMenuItem->SetIsSaved(isSaved);
        }
    }
}

void rgFileMenu::SetIsShowEntrypointListEnabled(bool isEnabled)
{
    m_isShowEntrypointListEnabled = isEnabled;

    // Disable the ability to expand file items' entrypoint list.
    if (!isEnabled && m_pSelectedFileItem != nullptr)
    {
        m_pSelectedFileItem->ShowEntrypointsList(false);
    }
}

void rgFileMenu::SelectLastRemainingItem()
{
    auto firstItemIter = m_menuItems.rbegin();
    if (firstItemIter != m_menuItems.rend())
    {
        rgFileMenuItem* pItem = *firstItemIter;
        auto pFileItem = static_cast<rgFileMenuFileItem*>(pItem);

        HandleSelectedFileChanged(pFileItem);
    }
}

void rgFileMenu::UpdateBuildOutput(const rgBuildOutputsMap& buildOutputs)
{
    std::string gpuWithOutputs;
    std::shared_ptr<rgCliBuildOutput> pGpuOutputs = nullptr;
    bool isOutputValid = rgUtils::GetFirstValidOutputGpu(buildOutputs, gpuWithOutputs, pGpuOutputs);
    if (isOutputValid && (pGpuOutputs != nullptr))
    {
        // Step through each file item in the menu.
        for (rgFileMenuFileItem* pFileItem : m_menuItems)
        {
            // Find the build output with the current item's filename.
            const std::string& itemFilename = pFileItem->GetFilename();
            auto outputIter = pGpuOutputs->m_perFileOutput.find(itemFilename);
            if (outputIter != pGpuOutputs->m_perFileOutput.end())
            {
                // Update the menu item with the outputs for the matching input file.
                rgFileOutputs& fileOutputs = outputIter->second;
                const std::vector<rgEntryOutput>& entryOutputs = fileOutputs.m_outputs;
                pFileItem->UpdateBuildOutputs(entryOutputs);
            }
        }

        // Does the user have a file selected in the menu?
        if (m_pSelectedFileItem != nullptr)
        {
            // Auto-expand the list of entrypoints in the selected file item.
            const std::string& selectedFilename = m_pSelectedFileItem->GetFilename();
            m_pSelectedFileItem->ShowEntrypointsList(true);
        }
    }
}

void rgFileMenu::InitializeDefaultMenuItems()
{
    // Insert the "Build Settings" item into the top of the menu.
    m_pBuildSettingsMenuItem = new rgFileMenuBuildSettingsItem();
    m_pLayout->insertWidget(0, m_pBuildSettingsMenuItem);

    // Insert the default menu item to the top of the menu.
    m_pDefaultMenuItem = new rgAddCreateMenuItem(this);
    m_pLayout->insertWidget(0, m_pDefaultMenuItem);

    ConnectDefaultItemSignals();
}

void rgFileMenu::ConnectDefaultItemSignals()
{
    // Handler invoked when the "Add File" button is clicked within an API item.
    bool isConnected = connect(m_pDefaultMenuItem->GetAddButton(), &QPushButton::clicked, this, &rgFileMenu::OpenFileButtonClicked);
    assert(isConnected);

    // Handler invoked when the "Create File" button is clicked within an API item.
    isConnected = connect(m_pDefaultMenuItem->GetCreateButton(), &QPushButton::clicked, this, &rgFileMenu::CreateFileButtonClicked);
    assert(isConnected);

    // Handler invoked when the "Build settings" button is clicked.
    isConnected = connect(m_pBuildSettingsMenuItem->GetBuildSettingsButton(), &QPushButton::clicked, this, &rgFileMenu::BuildSettingsButtonClicked);
    assert(isConnected);
}

void rgFileMenu::ConnectMenuFileItemSignals(rgFileMenuFileItem* pMenuItem)
{
    // Connect the close button for the file's menu item.
    bool isConnected = connect(pMenuItem, &rgFileMenuFileItem::MenuItemCloseButtonClicked, this, &rgFileMenu::MenuItemCloseButtonClicked);
    assert(isConnected);

    // Connect the file menu item selection handler for each new item.
    isConnected = connect(pMenuItem, &rgFileMenuFileItem::MenuItemSelected, this, &rgFileMenu::HandleSelectedFileChanged);
    assert(isConnected);

    // Connect the file menu item rename handler for each new item.
    isConnected = connect(pMenuItem, &rgFileMenuFileItem::FileRenamed, this, &rgFileMenu::HandleRenamedFile);
    assert(isConnected);

    // Connect the file menu item's entrypoint changed handler.
    isConnected = connect(pMenuItem, &rgFileMenuFileItem::SelectedEntrypointChanged, this, &rgFileMenu::SelectedEntrypointChanged);
    assert(isConnected);
}

void rgFileMenu::HandleSelectedEntrypointChanged(const std::string& inputFilePath, const std::string& selectedEntrypointName)
{
    // Find the given input file and select the incoming entrypoint name.
    for (rgFileMenuFileItem* pFileItem : m_menuItems)
    {
        if (pFileItem->GetFilename().compare(inputFilePath) == 0)
        {
            // Switch to the given entrypoint in the file item's entrypoint list.
            pFileItem->SwitchToEntrypointByName(selectedEntrypointName);
        }
    }
}

void rgFileMenu::HandleBuildStarted()
{
    // While a build is in progress, disable adding/creating files or changing the build settings.
    m_pDefaultMenuItem->GetAddButton()->setEnabled(false);
    m_pDefaultMenuItem->GetCreateButton()->setEnabled(false);
    m_pBuildSettingsMenuItem->setEnabled(false);
}

void rgFileMenu::HandleBuildEnded()
{
    // After the build is over, re-enable the menu items.
    m_pDefaultMenuItem->GetAddButton()->setEnabled(true);
    m_pDefaultMenuItem->GetCreateButton()->setEnabled(true);
    m_pBuildSettingsMenuItem->setEnabled(true);
}

void rgFileMenu::HandleSwitchToFile(const std::string & filePath, int lineNum)
{
    // Set focus index to newly selected item.
    bool  found = false;
    for (size_t i = 0; i < m_menuItems.size(); i++)
    {
        if (m_menuItems[i]->GetFilename() == filePath)
        {
            found = true;
            m_focusIndex = i;
            break;
        }
    }

    if (found)
    {
        // Refresh the focus selection so it selects the current focus index.
        SelectFocusItem();

        // Scroll the source code editor to required line.
        emit ScrollCodeEditorToLine(lineNum);
    }
}

void rgFileMenu::HandleRenamedFile(const std::string& oldFilepath, const std::string& newFilepath)
{
    auto menuItemIter = m_fullFilepathToMenuItem.find(oldFilepath);
    bool foundOldFilepath = menuItemIter != m_fullFilepathToMenuItem.end();

    // If the old path was found, remove it.
    assert(foundOldFilepath);
    if (foundOldFilepath)
    {
        rgFileMenuFileItem* pMenuItem = menuItemIter->second;

        // Erase the existing file path, and make the new path point to the existing menu item.
        m_fullFilepathToMenuItem.erase(menuItemIter);
        m_fullFilepathToMenuItem[newFilepath] = pMenuItem;
    }

    // Signal to the BuildView that a file has been renamed.
    emit FileRenamed(oldFilepath, newFilepath);
}

void rgFileMenu::HandleSelectedFileChanged(rgFileMenuFileItem* pSelected)
{
    // Set focus index to newly selected item.
    for (size_t i = 0; i < m_menuItems.size(); i++)
    {
        if (m_menuItems[i] == pSelected)
        {
            m_focusIndex = i;
        }
    }

    // Refresh the focus selection so it selects the current focus index.
    SelectFocusItem();
}

void rgFileMenu::SelectFile(rgFileMenuFileItem* pSelected)
{
    if (pSelected != nullptr)
    {
        std::string oldFilename;
        if (m_pSelectedFileItem != nullptr)
        {
            m_pSelectedFileItem->SetIsSelected(false);
            oldFilename = m_pSelectedFileItem->GetFilename();
        }

        // Assign the new selection as the currently-selected file item.
        m_pSelectedFileItem = pSelected;

        // Set the file item as the new selection, and expand the entrypoint list.
        m_pSelectedFileItem->SetIsSelected(true);

        const std::string& newFilename = pSelected->GetFilename();
        StringToFileItemMap::iterator fullFilenameIter = m_fullFilepathToMenuItem.find(newFilename);
        if (fullFilenameIter != m_fullFilepathToMenuItem.end())
        {
            emit SelectedFileChanged(oldFilename, newFilename);
        }
    }
}

void rgFileMenu::SelectFocusItem()
{
    QPushButton* pAddButton = m_pDefaultMenuItem->GetAddButton();
    QPushButton* pCreateButton = m_pDefaultMenuItem->GetCreateButton();

    // Clear button stylesheets.
    pAddButton->setStyleSheet("");
    pCreateButton->setStyleSheet("");
    m_pBuildSettingsMenuItem->GetBuildSettingsButton()->setStyleSheet("");

    // If focus index is in the range of the menu file items, select the appropriate file item.
    if (m_focusIndex < m_menuItems.size())
    {
        rgFileMenuFileItem* pItem = m_menuItems[m_focusIndex];
        SelectFile(pItem);

        // Update the cursor.
        UpdateCursor();
    }
    else
    {
        // If out of range, special case handle the last focus items.
        // Get index excluding file items.
        size_t index = m_focusIndex - m_menuItems.size();

        // Handle special cases for add/create buttons and build settings item.
        switch (index)
        {
        case FOCUS_ADD_BUTTON:
            pAddButton->setStyleSheet(s_BUTTON_FOCUS_STYLESHEET);
            break;
        case FOCUS_CREATE_BUTTON:
            pCreateButton->setStyleSheet(s_BUTTON_FOCUS_STYLESHEET);
            break;
        case FOCUS_BUILD_SETTINGS_BUTTON:
            m_pBuildSettingsMenuItem->GetBuildSettingsButton()->setStyleSheet(s_BUTTON_FOCUS_STYLESHEET);
            m_pBuildSettingsMenuItem->GetBuildSettingsButton()->click();
            break;
        default:
            // Should never get here.
            assert(false);
        }
    }
}

void rgFileMenu::HandleNextItemAction()
{
    size_t endIndex = m_menuItems.size() + 2;

    // If a file item is currently selected, switch to the next available entrypoint.
    bool nextEntrypointSelected = false;
    if (m_focusIndex < m_menuItems.size())
    {
        nextEntrypointSelected = OffsetCurrentFileEntrypoint(1);
    }

    if (!nextEntrypointSelected)
    {
        // Increment focus index and wrap around.
        m_focusIndex++;
        if (m_focusIndex > endIndex)
        {
            m_focusIndex = 0;
        }

        // Refresh focus.
        SelectFocusItem();
    }
}

void rgFileMenu::HandlePreviousItemAction()
{
    size_t endIndex = m_menuItems.size() + 2;

    // If a file item is currently selected, switch to the next available entrypoint.
    bool previousEntrypointSelected = false;
    if (m_focusIndex < m_menuItems.size())
    {
        previousEntrypointSelected = OffsetCurrentFileEntrypoint(-1);
    }

    if (!previousEntrypointSelected)
    {
        // Decrement focus index and wrap around.
        if (m_focusIndex != 0)
        {
            m_focusIndex--;
        }
        else
        {
            m_focusIndex = endIndex;
        }

        // Refresh focus.
        SelectFocusItem();
    }
}

void rgFileMenu::HandleOpenContextMenuAction()
{
    if (m_pSelectedFileItem != nullptr)
    {
        m_pSelectedFileItem->OpenContextMenu();
    }
}

void rgFileMenu::HandleActivateItemAction()
{
    QPushButton* pAddButton = m_pDefaultMenuItem->GetAddButton();
    QPushButton* pCreateButton = m_pDefaultMenuItem->GetCreateButton();

    // Get index excluding all file items.
    size_t index = m_focusIndex - m_menuItems.size();

    // Click button matching the index.
    switch (index)
    {
    case FOCUS_ADD_BUTTON:
        pAddButton->click();
        break;
    case FOCUS_CREATE_BUTTON:
        pCreateButton->click();
        break;
    }
}

void rgFileMenu::HandleRenameSelectedFileAction()
{
    if (m_pSelectedFileItem != nullptr)
    {
        m_pSelectedFileItem->ShowRenameControls(true);
    }
}

void rgFileMenu::HandleTabFocusPressed()
{
    // Determine if the focus is on a file item or one of the menu buttons.
    if (m_focusIndex >= m_menuItems.size())
    {
        // Get index excluding file items.
        size_t index = m_focusIndex - m_menuItems.size();

        // If the user presses tab while the Build Settings button is selected, tab into the view (the build settings view).
        if (index == FOCUS_BUILD_SETTINGS_BUTTON)
        {
            emit FocusNextView();
        }
    }
}

void rgFileMenu::UpdateCursor()
{
    // Reset cursor to pointing hand cursor for all items.
    foreach(auto pItem, m_menuItems)
    {
        pItem->setCursor(Qt::PointingHandCursor);
    }

    // Set the cursor to pointing hand cursor for the selected item.
    m_menuItems[m_focusIndex]->setCursor(Qt::ArrowCursor);
}
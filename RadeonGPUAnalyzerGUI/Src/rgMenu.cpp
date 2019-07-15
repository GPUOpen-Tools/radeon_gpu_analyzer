// C++.
#include <cassert>
#include <sstream>

// Qt.
#include <QBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QKeyEvent>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMenu.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgAddCreateMenuItem.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgBuildView.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMenuBuildSettingsItem.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMenuFileItemGraphics.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMenuFileItemOpenCL.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMenuItem.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMenuTitlebar.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgHandleTabFocusEventFilter.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDefinitions.h>
#include <RadeonGPUAnalyzerGUI/Include/rgStringConstants.h>
#include <RadeonGPUAnalyzerGUI/Include/rgUtils.h>
#include <QtCommon/Scaling/ScalingManager.h>

rgMenu::rgMenu(QWidget* pParent) :
    QFrame(pParent),
    m_focusIndex(0),
    m_tabFocusIndex(0)
{
    // Initialize the view object.
    ui.setupUi(this);

    // Initialize the menu layout, which all file items will be added to.
    m_pLayout = ui.fileMenuVerticalLayout;

    // Create file menu actions.
    CreateActions();

    // Install filter to ignore tab input for switching focus.
    installEventFilter(&rgHandleTabFocusEventFilter::Get());
}

void rgMenu::DeselectItems()
{
    DeselectCurrentFile();
    auto pBuildSettingsItem = GetBuildSettingsItem();
    assert(pBuildSettingsItem != nullptr);
    if (pBuildSettingsItem != nullptr)
    {
        pBuildSettingsItem->SetCurrent(false);
        if (pBuildSettingsItem->GetBuildSettingsButton() != nullptr)
        {
            pBuildSettingsItem->GetBuildSettingsButton()->setStyleSheet(s_BUTTON_FOCUS_OUT_STYLESHEET);
        }
    }
}

void rgMenu::CreateActions()
{
    // File menu next item.
    m_pNextItemAction = new QAction(this);
    m_pNextItemAction->setShortcut(QKeySequence(gs_ACTION_HOTKEY_FILE_MENU_NEXT_ITEM));
    m_pNextItemAction->setShortcutContext(Qt::WidgetShortcut);

    addAction(m_pNextItemAction);
    bool isConnected = connect(m_pNextItemAction, &QAction::triggered, this, &rgMenu::HandleTabFocusPressed);
    assert(isConnected);

    // File menu previous item.
    m_pPrevItemAction = new QAction(this);
    m_pPrevItemAction->setShortcut(QKeySequence(gs_ACTION_HOTKEY_FILE_MENU_PREV_ITEM));
    m_pPrevItemAction->setShortcutContext(Qt::WidgetShortcut);

    addAction(m_pPrevItemAction);
    isConnected = connect(m_pPrevItemAction, &QAction::triggered, this, &rgMenu::HandleShiftTabFocusPressed);
    assert(isConnected);

    // File menu context menu shortcut.
    m_pOpenContextMenuAction = new QAction(this);
    m_pOpenContextMenuAction->setShortcut(QKeySequence(gs_ACTION_HOTKEY_FILE_MENU_CONTEXT_MENU));
    m_pOpenContextMenuAction->setShortcutContext(Qt::WidgetShortcut);

    addAction(m_pOpenContextMenuAction);
    isConnected = connect(m_pOpenContextMenuAction, &QAction::triggered, this, &rgMenu::HandleOpenContextMenuAction);
    assert(isConnected);

    // File menu activate item (used to trigger add/create buttons when in focus).
    m_pActivateItemAction = new QAction(this);
    QList<QKeySequence> shortcutActions;
    shortcutActions.push_back(QKeySequence(gs_ACTION_HOTKEY_FILE_MENU_ACTIVATE_RETURN));
    shortcutActions.push_back(QKeySequence(gs_ACTION_HOTKEY_FILE_MENU_ACTIVATE_ENTER));
    shortcutActions.push_back(QKeySequence(gs_ACTION_HOTKEY_FILE_MENU_ACTIVATE_SPACE));
    m_pActivateItemAction->setShortcuts(shortcutActions);
    m_pActivateItemAction->setShortcutContext(Qt::WidgetShortcut);

    addAction(m_pActivateItemAction);
    isConnected = connect(m_pActivateItemAction, &QAction::triggered, this, &rgMenu::HandleActivateItemAction);
    assert(isConnected);

    // File menu rename selected file.
    m_pRenameSelectedFileAction = new QAction(this);
    m_pRenameSelectedFileAction->setShortcut(QKeySequence(gs_ACTION_HOTKEY_FILE_MENU_RENAME));
    m_pRenameSelectedFileAction->setShortcutContext(Qt::WidgetShortcut);

    addAction(m_pRenameSelectedFileAction);
    isConnected = connect(m_pRenameSelectedFileAction, &QAction::triggered, this, &rgMenu::HandleRenameSelectedFileAction);
    assert(isConnected);

    // File menu tab key navigation.
    m_pTabKeyAction = new QAction(this);
    m_pTabKeyAction->setShortcut(QKeySequence(gs_ACTION_HOTKEY_FILE_MENU_ACTIVATE_TAB));
    m_pTabKeyAction->setShortcutContext(Qt::WidgetShortcut);

    addAction(m_pTabKeyAction);
    isConnected = connect(m_pTabKeyAction, &QAction::triggered, this, &rgMenu::HandleTabFocusPressed);
    assert(isConnected);

    // File menu shift+tab key navigation.
    m_pShiftTabKeyAction = new QAction(this);
    m_pShiftTabKeyAction->setShortcut(QKeySequence(gs_ACTION_HOTKEY_FILE_MENU_ACTIVATE_SHIFT_TAB));
    m_pShiftTabKeyAction->setShortcutContext(Qt::WidgetShortcut);

    addAction(m_pShiftTabKeyAction);
    isConnected = connect(m_pShiftTabKeyAction, &QAction::triggered, this, &rgMenu::HandleShiftTabFocusPressed);
    assert(isConnected);
}

void rgMenu::ClearFiles()
{
    // Deselect any currently selected file, because it's about to be removed.
    DeselectCurrentFile();

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

rgMenuBuildSettingsItem* rgMenu::GetBuildSettingsItem() const
{
    return m_pBuildSettingsMenuItem;
}

rgMenuFileItem* rgMenu::GetFileItemFromPath(const std::string& sourceFilePath) const
{
    rgMenuFileItem* pResultItem = nullptr;

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

rgMenuFileItem* rgMenu::GetSelectedFileItem() const
{
    return m_pSelectedFileItem;
}

std::string rgMenu::GetSelectedFilePath() const
{
    std::string selectedFilePath;
    if (m_pSelectedFileItem != nullptr)
    {
        selectedFilePath = m_pSelectedFileItem->GetFilename();
    }
    return selectedFilePath;
}

std::vector<rgMenuFileItem*> rgMenu::GetAllFileItems() const
{
    return m_menuItems;
}

bool rgMenu::IsCurrentlySelectedFileItem(rgMenuFileItem* pFileItem) const
{
    return (pFileItem == m_pSelectedFileItem);
}

bool rgMenu::IsEmpty() const
{
    return m_fullFilepathToMenuItem.empty();
}

bool rgMenu::IsFileInMenu(const std::string& fullPath) const
{
    bool ret = false;

    // Standardize the path of the given full path.
    std::string fullPathStandardized = fullPath;
    rgUtils::StandardizePathSeparator(fullPathStandardized);

    // Check if the full path already has an item in the menu.
    for (const rgMenuFileItem* pItem : m_menuItems)
    {
        assert(pItem != nullptr);
        if (pItem != nullptr)
        {
            // Retrieve and standardize the file name for this item.
            std::string itemFullPath = pItem->GetFilename();
            rgUtils::StandardizePathSeparator(itemFullPath);

            // If the paths match - we found it.
            if (itemFullPath.compare(fullPathStandardized) == 0)
            {
                ret = true;
                break;
            }
        }
    }
    return ret;
}

void rgMenu::RemoveItem(const std::string& fullFilename)
{
    if (!fullFilename.empty())
    {
        // Remove the item from the map.
        auto filepathIter = m_fullFilepathToMenuItem.find(fullFilename);
        if (filepathIter != m_fullFilepathToMenuItem.end())
        {
            rgMenuFileItem* pMenuItem = filepathIter->second;

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

            // Clear the "last selected item" variable if it's the item we are removing.
            if (m_pLastSelectedItem == pMenuItem)
            {
                m_pLastSelectedItem = nullptr;
            }
        }
    }
}

void rgMenu::DeselectCurrentFile()
{
    // If a file was already selected, deselect it.
    if (m_pSelectedFileItem != nullptr)
    {
        m_pSelectedFileItem->SetHovered(false);
        m_pSelectedFileItem->SetCurrent(false);
        m_pSelectedFileItem->setCursor(Qt::PointingHandCursor);
        m_pSelectedFileItem = nullptr;
    }
}

void rgMenu::SetItemIsSaved(const std::string& fullFilename, bool isSaved)
{
    // Find the item in the map.
    auto filepathIter = m_fullFilepathToMenuItem.find(fullFilename);
    if (filepathIter != m_fullFilepathToMenuItem.end())
    {
        rgMenuFileItem* pMenuItem = filepathIter->second;
        bool isItemValid = (pMenuItem != nullptr);
        assert(isItemValid);

        if (isItemValid)
        {
            // Set item saved state.
            pMenuItem->SetIsSaved(isSaved);
        }
    }
}

void rgMenu::SelectFirstItem()
{
    auto firstItemIter = m_menuItems.cbegin();
    if (firstItemIter != m_menuItems.cend() && *firstItemIter != nullptr)
    {
        HandleSelectedFileChanged(static_cast<rgMenuFileItem*>(*firstItemIter));
    }
}

void rgMenu::SelectLastRemainingItem()
{
    auto lastItemIter = m_menuItems.crbegin();
    if (lastItemIter != m_menuItems.crend() && *lastItemIter != nullptr)
    {
        HandleSelectedFileChanged(static_cast<rgMenuFileItem*>(*lastItemIter));
    }
}

void rgMenu::SwitchToLastSelectedItem()
{
    if (m_pLastSelectedItem != nullptr)
    {
        HandleSelectedFileChanged(m_pLastSelectedItem);
    }
}

void rgMenu::HandleBuildStarted()
{
    m_pBuildSettingsMenuItem->setEnabled(false);
}

void rgMenu::HandleBuildEnded()
{
    m_pBuildSettingsMenuItem->setEnabled(true);
}

void rgMenu::HandleSwitchToFile(const std::string & filePath, int lineNum)
{
    // Set focus index to newly selected item.
    bool  found = false;
    for (size_t i = 0; i < m_menuItems.size(); i++)
    {
        if (m_menuItems[i]->GetFilename() == filePath)
        {
            found = true;
            m_focusIndex = i;
            m_tabFocusIndex = i;
            break;
        }
    }

    if (found)
    {
        // Refresh the focus selection so it selects the current focus index.
        SelectFocusItem(FileMenuActionType::TabAction);

        // Scroll the source code editor to required line.
        emit ScrollCodeEditorToLine(lineNum);
    }
}

void rgMenu::HandleRenamedFile(const std::string& oldFilepath, const std::string& newFilepath)
{
    auto menuItemIter = m_fullFilepathToMenuItem.find(oldFilepath);
    bool foundOldFilepath = menuItemIter != m_fullFilepathToMenuItem.end();

    // If the old path was found, remove it.
    assert(foundOldFilepath);
    if (foundOldFilepath)
    {
        rgMenuFileItem* pMenuItem = menuItemIter->second;

        // Erase the existing file path, and make the new path point to the existing menu item.
        m_fullFilepathToMenuItem.erase(menuItemIter);
        m_fullFilepathToMenuItem[newFilepath] = pMenuItem;
    }

    // Signal to the BuildView that a file has been renamed.
    emit FileRenamed(oldFilepath, newFilepath);
}

void rgMenu::SelectFile(rgMenuFileItem* pSelected)
{
    if (pSelected != nullptr)
    {
        std::string oldFilename;
        if (m_pSelectedFileItem != nullptr)
        {
            m_pSelectedFileItem->SetHovered(false);
            oldFilename = m_pSelectedFileItem->GetFilename();
        }

        // Assign the new selection as the currently-selected file item.
        m_pSelectedFileItem = pSelected;
        m_pLastSelectedItem = pSelected;

        // Set the file item as the new selection, and expand the entry point list.
        m_pSelectedFileItem->SetHovered(true);

        // Update the cursor type.
        UpdateCursor(m_pSelectedFileItem);

        // Set the file item as the new highlighted item.
        UpdateHighlight(pSelected);

        // Set the file item as the current item.
        UpdateCurrentItem(pSelected);

        const std::string& newFilename = pSelected->GetFilename();
        StringToFileItemMap::iterator fullFilenameIter = m_fullFilepathToMenuItem.find(newFilename);
        if (fullFilenameIter != m_fullFilepathToMenuItem.end())
        {
            emit SelectedFileChanged(oldFilename, newFilename);
        }
    }
}

void rgMenu::DisplayFileInEditor(rgMenuFileItem* pSelected)
{
    std::string oldFilename;
    if (m_pSelectedFileItem != nullptr)
    {
        m_pSelectedFileItem->SetHovered(false);
        oldFilename = m_pSelectedFileItem->GetFilename();
    }

    const std::string& newFilename = pSelected->GetFilename();
    StringToFileItemMap::iterator fullFilenameIter = m_fullFilepathToMenuItem.find(newFilename);
    if (fullFilenameIter != m_fullFilepathToMenuItem.end())
    {
        // Set the file item as the new highlighted item.
        UpdateHighlight(pSelected);

        // Set this item as the current one.
        UpdateCurrentItem(pSelected);

        // Update the cursors.
        UpdateCursor(pSelected);

        // Assign the new selection as the currently-selected file item.
        m_pSelectedFileItem = pSelected;
        m_pLastSelectedItem = pSelected;

        emit SelectedFileChanged(oldFilename, newFilename);
    }
}

void rgMenu::HandleNextItemAction()
{
    // Compute the index of the next button to focus on. Take the extra non-file items into account.
    int offset = GetButtonCount() - 1;
    size_t endIndex = m_menuItems.size() + offset;

    // Increment focus index and wrap around.
    m_focusIndex++;
    if (m_focusIndex > endIndex)
    {
        m_focusIndex = 0;
    }

    // Update the tab focus index.
    m_tabFocusIndex = m_focusIndex;

    // Refresh focus.
    SelectFocusItem(FileMenuActionType::ArrowAction);
}

void rgMenu::HandlePreviousItemAction()
{
    // Compute the index of the next button to focus on. Take the extra non-file items into account.
    int offset = GetButtonCount() - 1;
    size_t endIndex = m_menuItems.size() + offset;

    // Decrement focus index and wrap around.
    if (m_focusIndex != 0)
    {
        m_focusIndex--;
    }
    else
    {
        m_focusIndex = endIndex;
    }

    // Update the tab focus index.
    m_tabFocusIndex = m_focusIndex;

    // Refresh focus.
    SelectFocusItem(FileMenuActionType::ArrowAction);
}

void rgMenu::HandleOpenContextMenuAction()
{
    if (m_pSelectedFileItem != nullptr)
    {
        m_pSelectedFileItem->OpenContextMenu();
    }
}

void rgMenu::HandleRenameSelectedFileAction()
{
    if (m_pSelectedFileItem != nullptr)
    {
        m_pSelectedFileItem->ShowRenameControls(true);
    }
}

void rgMenu::UpdateCursor(rgMenuFileItem* pCurrentStageItem)
{
    // Reset cursor to pointing hand cursor for all items.
    foreach(auto pItem, m_menuItems)
    {
        pItem->setCursor(Qt::PointingHandCursor);
    }

    // Set the cursor to arrow cursor for the selected item.
    pCurrentStageItem->setCursor(Qt::ArrowCursor);
}

void rgMenu::UpdateHighlight(rgMenuFileItem* pSelected)
{
    // Reset the highlight for all items.
    foreach(auto pItem, m_menuItems)
    {
        pItem->SetHovered(false);
    }

    pSelected->SetHovered(true);
}

void rgMenu::mousePressEvent(QMouseEvent* pEvent)
{
    emit FileMenuFocusInEvent();

    QFrame::mousePressEvent(pEvent);
}

void rgMenu::UpdateCurrentItem(rgMenuFileItem* pSelected)
{
    for (auto pItem : m_menuItems)
    {
        pItem->SetCurrent(false);
    }

    pSelected->SetCurrent(true);
}

void rgMenu::ClearFileMenuHighlight()
{
    for (auto pItem : m_menuItems)
    {
        pItem->SetHovered(false);
    }
}

void rgMenu::ClearFileMenuItemSelection()
{
    // Set the stylesheet for each file menu item to be not selected (grayed out),
    // along with the mouse cursor to pointing hand cursor.
    for (rgMenuFileItem* pItem : m_menuItems)
    {
        pItem->SetHovered(false);
        pItem->SetCurrent(false);
        pItem->setCursor(Qt::PointingHandCursor);
    }
}
// C++.
#include <cassert>
#include <sstream>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMenuBuildSettingsItem.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMenuVulkan.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMenuFileItemGraphics.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMenuPipelineStateItem.h>
#include <RadeonGPUAnalyzerGUI/Include/rgStringConstants.h>
#include <RadeonGPUAnalyzerGUI/Include/rgUtils.h>

rgMenuVulkan::rgMenuVulkan(QWidget* pParent)
    : rgMenuGraphics(pParent)
{
}

void rgMenuVulkan::ConnectDefaultItemSignals()
{
    // Handler invoked when the "Build settings" button is clicked.
    assert(m_pBuildSettingsMenuItem != nullptr);
    if (m_pBuildSettingsMenuItem != nullptr)
    {
        bool isConnected = connect(m_pBuildSettingsMenuItem->GetBuildSettingsButton(), &QPushButton::clicked, this, &rgMenu::HandleBuildSettingsButtonClicked);
        assert(isConnected);
    }
}

void rgMenuVulkan::ConnectMenuFileItemSignals(rgMenuFileItem* pMenuItem)
{
    // Connect the file menu item selection handler for each new item.
    bool isConnected = connect(pMenuItem, &rgMenuFileItemGraphics::MenuItemSelected, this, &rgMenu::MenuItemClicked);
    assert(isConnected);

    // Connect the file menu item rename handler for each new item.
    isConnected = connect(pMenuItem, &rgMenuFileItem::FileRenamed, this, &rgMenu::HandleRenamedFile);
    assert(isConnected);
}

void rgMenuVulkan::DeselectItems()
{
    rgMenu::DeselectItems();

    assert(m_pPipelineStateItem);
    if (m_pPipelineStateItem != nullptr)
    {
        m_pPipelineStateItem->SetCurrent(false);
    }
}

void rgMenuVulkan::ClearStageSourceFile(rgPipelineStage stage)
{
    // Find the target stage item and update the attached shader file.
    rgMenuFileItemGraphics* pStageItem = GetStageItem(stage);

    assert(pStageItem != nullptr);
    if (pStageItem != nullptr)
    {
        // Erase the file path from the path to menu item map.
        const std::string& pathBeingRemoved = pStageItem->GetFilename();
        auto filePathToItemMapIter = m_fullFilepathToMenuItem.find(pathBeingRemoved);
        if (filePathToItemMapIter != m_fullFilepathToMenuItem.end())
        {
            m_fullFilepathToMenuItem.erase(filePathToItemMapIter);
        }

        // Clear out the filepath for the stage item.
        pStageItem->SetShaderFile("", rgVulkanInputType::Unknown);

        // Set the "occupied" property for this file item.
        pStageItem->SetStageIsOccupied(false);

        // Set the "current" property for this file item.
        pStageItem->SetCurrent(false);

        // Set the cursor type to arrow cursor.
        pStageItem->SetCursor(Qt::ArrowCursor);

        // Set the "+" sub button as selected.
        RemoveSubButtonFocus();
        HideRemoveFilePushButton();
        pStageItem->SetButtonHighlighted(GraphicsMenuTabFocusItems::AddExistingFileButton);
        m_hasTabFocus = GraphicsMenuTabFocusItems::AddExistingFileButton;

        // Also set the tab focus index and focus index.
        // Find out if one of the buttons is currently selected.
        // If so, do not update the focus index.
        if (!IsButtonPressed())
        {
            if (stage == rgPipelineStage::Compute)
            {
                m_focusIndex = 0;
            }
            else
            {
                m_focusIndex = static_cast<int>(stage);
            }
            m_tabFocusIndex = m_focusIndex;
        }

        // Remove the item from the list.
        for (auto it = m_menuItems.begin(); it != m_menuItems.end(); it++)
        {
            if (*it == pStageItem)
            {
                m_menuItems.erase(it);
                break;
            }
        }
    }

    // Emit a signal that indicates that the number of items in the menu change,
    // and specify that the menu is not empty.
    emit FileMenuItemCountChanged(false);
}

bool rgMenuVulkan::IsButtonPressed() const
{
    bool buttonPressed = false;

    assert(m_pBuildSettingsMenuItem != nullptr);
    assert(m_pPipelineStateItem != nullptr);
    if (m_pBuildSettingsMenuItem != nullptr && m_pPipelineStateItem != nullptr)
    {
        buttonPressed = m_pBuildSettingsMenuItem->IsCurrent() || m_pPipelineStateItem->IsCurrent();
    }

    return buttonPressed;
}

bool rgMenuVulkan::SetStageSourceFile(rgPipelineStage stage, const std::string& fullPath, rgVulkanInputType fileType, bool isNewFileItem)
{
    bool ret = false;

    // If a file with that name hasn't been added.
    if (!IsFileInMenu(fullPath))
    {
        // Find the target stage item and update the attached shader file.
        rgMenuFileItemGraphics* pStageItem = GetStageItem(stage);

        assert(pStageItem != nullptr);
        if (pStageItem != nullptr)
        {
            pStageItem->SetShaderFile(fullPath, fileType);

            // Since users may add shader files to the pipeline stages in any order they choose, in
            // order to use the Up/Down arrow keys to navigate through shader stage items, a linear
            // list of menu items must be reconstructed.
            RebuildMenuItemsList();

            // Set the "occupied" property for this file item.
            pStageItem->SetStageIsOccupied(true);

            // Set the cursor for this item.
            UpdateCursor(pStageItem);

            // Hide the "Remove" button.
            pStageItem->RemoveSubButtonFocus();
            pStageItem->HideRemoveFilePushButton();

            ret = true;
        }

        // Emit a signal that indicates that the number of items in the menu change,
        // and specify that the menu is not empty.
        emit FileMenuItemCountChanged(false);

        // Add the file name and its full path to the map.
        m_fullFilepathToMenuItem[fullPath] = pStageItem;

        // Connect signals for the new file menu item.
        ConnectMenuFileItemSignals(pStageItem);

        // Select the file that was just opened.
        HandleSelectedFileChanged(pStageItem);

        if (isNewFileItem)
        {
            // If a file was newly-created (as opposed to being opened), display the
            // item's renaming control immediately so the user can choose a new filename.
            pStageItem->ShowRenameControls(true);
        }

    }
    else
    {
        // Report the error.
        std::stringstream msg;
        msg << STR_ERR_CANNOT_ADD_FILE_A << fullPath << STR_ERR_CANNOT_ADD_FILE_B;
        rgUtils::ShowErrorMessageBox(msg.str().c_str(), this);

        // Remove the highlight.
        rgMenuFileItemGraphics* pStageItem = GetStageItem(stage);

        assert(pStageItem != nullptr);
        if (pStageItem != nullptr)
        {
            pStageItem->SetCurrent(false);
            pStageItem->SetHovered(false);
        }

        // Set the highlight for the current stage.
        pStageItem = GetCurrentFileMenuItem();
        assert(pStageItem != nullptr);
        if (pStageItem != nullptr)
        {
            pStageItem->SetCurrent(true);
            pStageItem->SetHovered(true);
        }
    }

    return ret;
}

bool rgMenuVulkan::ReplaceStageFile(rgPipelineStage stage, const std::string& newFilePath, rgVulkanInputType fileType)
{
    bool ret = false;
    rgMenuFileItemGraphics* pFileItem = GetStageItem(stage);
    assert(pFileItem != nullptr);
    if (pFileItem != nullptr)
    {
        // Replace the file path in the file to item map.
        const std::string& oldFilePath = pFileItem->GetFilename();
        auto it = m_fullFilepathToMenuItem.find(oldFilePath);
        if (it != m_fullFilepathToMenuItem.end())
        {
            assert(it->second == pFileItem);
            if (it->second == pFileItem)
            {
                m_fullFilepathToMenuItem.erase(it);
                m_fullFilepathToMenuItem[newFilePath] = pFileItem;
                ret = true;
            }
        }

        // Replace the file path in the item.
        pFileItem->SetShaderFile(newFilePath, fileType);
    }

    return ret;
}

void rgMenuVulkan::HandleActivateItemAction()
{
    // If focus index is in the range of the menu file items, select the appropriate file item.
    const size_t totalStages = GetTotalPipelineStages();
    if (m_pipelineType == rgPipelineType::Graphics && m_tabFocusIndex < totalStages ||
        m_pipelineType == rgPipelineType::Compute && m_tabFocusIndex <= totalStages)
    {
        // See if a sub button is selected,
        // and then execute accordingly.
        if (m_hasTabFocus != GraphicsMenuTabFocusItems::NoButton)
        {
            // If the user hit enter when one of the sub-buttons were highlighted,
            // process accordingly.
            ProcessSubButtonAction();
        }
        else if (m_focusIndex < m_menuItems.size())
        {
            SelectFile(m_menuItems[m_focusIndex]);
            m_pBuildSettingsMenuItem->SetCurrent(false);
            m_pPipelineStateItem->SetCurrent(false);
        }
        else
        {
            // Local index: this is the index of the relevant
            // item within the menu items after filtering out
            // the file items (leaving only the other buttons:
            // pipeline state and build settings).
            const int PIPELINE_STATE_ITEM_LOCAL_INDEX = 0;
            const int BUILD_SETTINGS_ITEM_LOCAL_INDEX = 1;

            // Calculate the index of the relevant item.
            const int buttonCount = GetButtonCount();
            const size_t numOfFileItems = m_menuItems.size();
            size_t targetItemIndex = static_cast<size_t>(m_tabFocusIndex) - numOfFileItems;
            assert(targetItemIndex == 0 || targetItemIndex == 1);
            if (targetItemIndex == PIPELINE_STATE_ITEM_LOCAL_INDEX)
            {
                // This is the pipeline state button - simulate a click.
                m_pPipelineStateItem->ClickMenuItem();
            }
            else if(targetItemIndex == BUILD_SETTINGS_ITEM_LOCAL_INDEX)
            {
                // This is the build settings button - simulate a click.
                m_pBuildSettingsMenuItem->ClickMenuItem();
            }
            else
            {
                // We shouldn't get here.
                assert(false);
            }
        }

        RemoveFileMenuButtonFocus();
    }
    else
    {
        // Deselect graphics menu file items.
        for (rgMenuFileItem* pItem : m_menuItems)
        {
            rgMenuFileItemGraphics* pItemGraphics = static_cast<rgMenuFileItemGraphics*>(pItem);
            assert(pItemGraphics != nullptr);
            if (pItemGraphics != nullptr)
            {
                pItemGraphics->SetHovered(false);
                pItemGraphics->SetCurrent(false);
            }
        }

        // If out of range, special case handle the last focus items.
        // Get index excluding file items.
        size_t index = m_focusIndex - totalStages;

        // Handle special cases for pipeline state and build settings item.
        switch (index)
        {
        case static_cast<size_t>(GraphicsMenuFocusItems::BuildSettingsButton) :
            m_pBuildSettingsMenuItem->GetBuildSettingsButton()->setStyleSheet(s_BUTTON_FOCUS_IN_STYLESHEET);
            m_pBuildSettingsMenuItem->GetBuildSettingsButton()->click();
            break;
        case static_cast<size_t>(GraphicsMenuFocusItems::PipelineStateButton) :
            GetPipelineStateItem()->GetPipelineStateButton()->setStyleSheet(s_BUTTON_FOCUS_IN_STYLESHEET);
            GetPipelineStateItem()->GetPipelineStateButton()->click();
            break;
        default:
            // Should never get here.
            assert(false);
            break;
        }
    }
}

void rgMenuVulkan::ProcessSubButtonAction()
{
    const size_t totalStages = GetTotalPipelineStages();
    assert(m_tabFocusIndex < totalStages);
    if (m_tabFocusIndex < totalStages)
    {
        rgMenuFileItemGraphics* pItem = GetCurrentFileMenuItem();
        assert(pItem != nullptr);
        if (pItem != nullptr)
        {
            switch (m_hasTabFocus)
            {
            case (GraphicsMenuTabFocusItems::RemoveButton):
                pItem->ProcessRemoveButtonClick();
                break;
            case (GraphicsMenuTabFocusItems::AddExistingFileButton):
                pItem->ProcessAddExistingFileButtonClick();
                break;
            default:
                // Do not assert here.
                // Doing so will cause asserts any time the user
                // clicks on a file menu item, or hits enter when
                // a file menu item has the focus.
                break;
            }
        }
    }
}

void rgMenuVulkan::HandleSelectedFileChanged(rgMenuFileItem* pSelected)
{
    rgMenuFileItemGraphics* pGraphicsItem = qobject_cast<rgMenuFileItemGraphics*>(pSelected);
    assert(pGraphicsItem != nullptr);

    // Set focus index to newly selected item.
    for (size_t i = 0; i < m_menuItems.size() && pGraphicsItem != nullptr; i++)
    {
        if (m_menuItems[i] == pSelected)
        {
            // Update the focus and tab indices. Compute indices are always
            // 0 because there's only 1 item in the compute pipeline menu.
            rgPipelineStage stageType = pGraphicsItem->GetStage();
            if (stageType != rgPipelineStage::Compute)
            {
                m_focusIndex = static_cast<size_t>(stageType);
                m_tabFocusIndex = static_cast<size_t>(stageType);
            }
            else
            {
                m_focusIndex = 0;
                m_tabFocusIndex = 0;
            }
            break;
        }
    }

    // Update the current stage value.
    if (pGraphicsItem != nullptr)
    {
        m_currentStage = pGraphicsItem->GetStage();
    }

    // Display the currently selected file in the source editor.
    DisplayFileInEditor(pSelected);

    // Update button selection values.
    m_pPipelineStateItem->SetCurrent(false);
    m_pBuildSettingsMenuItem->SetCurrent(false);

    // Set the pipeline state and build settings buttons to have focus out style sheets.
    assert(m_pBuildSettingsMenuItem);
    assert(m_pPipelineStateItem);
    if (m_pBuildSettingsMenuItem != nullptr && m_pPipelineStateItem != nullptr)
    {
        m_pBuildSettingsMenuItem->GetBuildSettingsButton()->setStyleSheet(s_BUTTON_FOCUS_OUT_STYLESHEET);
        m_pBuildSettingsMenuItem->GetBuildSettingsButton()->setCursor(Qt::PointingHandCursor);

        m_pPipelineStateItem->GetPipelineStateButton()->setStyleSheet(s_BUTTON_FOCUS_OUT_STYLESHEET);
        m_pPipelineStateItem->GetPipelineStateButton()->setCursor(Qt::PointingHandCursor);
    }
}

void rgMenuVulkan::UpdateFocusIndex()
{
    m_focusIndex = static_cast<size_t>(m_currentStage);
    m_tabFocusIndex = static_cast<size_t>(m_currentStage);
}
// C++.
#include <cassert>

// Qt.
#include <QKeyEvent>

// Infra.
#include <QtCommon/Scaling/ScalingManager.h>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMenuBuildSettingsItem.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMenuGraphics.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMenuPipelineStateItem.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMenuFileItemGraphics.h>

static const char* s_BUTTON_FOCUS_IN_STYLESHEET_GRAPHICS = "QPushButton { background: rgb(253,255,174); border-style: solid; border-width: 2px; border-color: rgb(135, 20, 16);}";
static const char* s_BUTTON_TABBED_STYLESHEET = "QPushButton { border: 2px solid rgb(135,20,16); margin: 1px; background: rgb(214,214,214);}";
static const char* s_FILE_MENU_NAME_GRAPHICS = "fileMenuGraphics";

static const int s_PIPELINE_STATE_BUTTON_INDEX = 1;
static const int s_BUILD_SETTINGS_BUTTON_INDEX = 2;

static const int s_NUMBER_OF_GRAPHICS_PIPELINE_STAGES = 5;
static const int s_NUMBER_OF_COMPUTE_PIPELINE_STAGES = 1;

rgMenuGraphics::rgMenuGraphics(QWidget* pParent)
    : rgMenu(pParent)
{
    // Set the object name.
    setObjectName(s_FILE_MENU_NAME_GRAPHICS);
}

void rgMenuGraphics::SelectFocusItem(FileMenuActionType actionType)
{
    assert(m_pPipelineStateItem != nullptr);
    assert(m_pBuildSettingsMenuItem != nullptr);
    if (m_pPipelineStateItem != nullptr && m_pBuildSettingsMenuItem != nullptr)
    {
        ClearButtonStylesheets();

        // Find out total stages and which vector to use.
        const size_t totalPipelineStages = GetNumberPipelineStagesToCycleThrough(actionType);
        const std::vector<rgMenuFileItem*> menuFileItems = GetFileMenuItemsToProcess(actionType);

        // If focus index is in the range of the menu file items, select the appropriate file item.
        if (m_focusIndex < totalPipelineStages)
        {
            rgMenuFileItemGraphics* pItem = static_cast<rgMenuFileItemGraphics*>(menuFileItems[m_focusIndex]);
            assert(pItem != nullptr);
            if (pItem != nullptr)
            {
                UpdateHighlight(pItem);
            }
        }
        else
        {
            // Deselect graphics menu items.
            DeselectFileItems();

            // Out of range, so special case handle the last focus items.
            // Get index excluding file items.
            size_t index = m_focusIndex - totalPipelineStages;

            // Handle special cases for pipeline state and build settings buttons.
            SelectButton(index);
        }
    }
}

void rgMenuGraphics::SelectButton(size_t index)
{
    // Handle special cases for pipeline state and build settings buttons.
    switch (index)
    {
    case static_cast<size_t>(GraphicsMenuFocusItems::BuildSettingsButton) :
        if (!m_pBuildSettingsMenuItem->IsCurrent())
        {
            m_pBuildSettingsMenuItem->GetBuildSettingsButton()->setStyleSheet(s_BUTTON_TABBED_STYLESHEET);
            m_hasTabFocus = GraphicsMenuTabFocusItems::NoButton;
        }
        break;
    case static_cast<size_t>(GraphicsMenuFocusItems::PipelineStateButton) :
        if (!m_pPipelineStateItem->IsCurrent())
        {
            m_pPipelineStateItem->GetPipelineStateButton()->setStyleSheet(s_BUTTON_TABBED_STYLESHEET);
            m_hasTabFocus = GraphicsMenuTabFocusItems::NoButton;
        }
        break;
    default:
        // Should never get here.
        assert(false);
        break;
    }
}

void rgMenuGraphics::SelectTabFocusItem(bool shiftTabFocus)
{
    assert(m_pPipelineStateItem != nullptr);
    assert(m_pBuildSettingsMenuItem != nullptr);
    if (m_pPipelineStateItem != nullptr && m_pBuildSettingsMenuItem != nullptr)
    {
        ClearButtonStylesheets();

        // If focus index is in the range of the menu file items, select the appropriate file item.
        if (m_tabFocusIndex < m_totalPipelineStages)
        {
            rgMenuFileItemGraphics* pItem = GetCurrentFileMenuItem();
            assert(pItem != nullptr);
            if (pItem != nullptr)
            {
                if (!pItem->GetFilename().empty())
                {
                    UpdateHighlight(pItem);
                }
                else
                {

                    pItem->SetButtonHighlighted(GraphicsMenuTabFocusItems::AddExistingFileButton);
                    m_hasTabFocus = GraphicsMenuTabFocusItems::AddExistingFileButton;
                }
            }
        }
        else
        {
            // Deselect graphics menu items.
            DeselectFileItems();

            // Out of range, so special case handle the last focus items.
            // Get index excluding file items.
            size_t index = m_focusIndex - m_totalPipelineStages;

            // Handle special cases for pipeline state and build settings buttons.
            SelectButton(index);
        }
    }
}

void rgMenuGraphics::DeselectFileItems()
{
    // Deselect graphics menu file items.
    for (rgMenuFileItem* pItem : m_menuItems)
    {
        rgMenuFileItemGraphics* pItemGraphics = static_cast<rgMenuFileItemGraphics*>(pItem);
        assert(pItem != nullptr);
        if (pItem != nullptr)
        {
            pItem->SetHovered(false);
        }
    }
}

void rgMenuGraphics::ClearButtonStylesheets()
{
    // Clear button style sheets.
    if (!m_pBuildSettingsMenuItem->IsCurrent())
    {
        m_pBuildSettingsMenuItem->GetBuildSettingsButton()->setStyleSheet("");
    }

    if (!m_pPipelineStateItem->IsCurrent())
    {
        m_pPipelineStateItem->GetPipelineStateButton()->setStyleSheet("");
    }
}

void rgMenuGraphics::InitializeDefaultMenuItems(const std::shared_ptr<rgProjectClone> pProjectClone)
{
    // Initialize the default shader stage items.
    InitializeDefaultShaderStageItems(pProjectClone);

    // Get the index for the last widget added.
    int lastStage;
    std::shared_ptr<rgGraphicsProjectClone> pGraphicsClone = std::dynamic_pointer_cast<rgGraphicsProjectClone>(pProjectClone);
    assert(pGraphicsClone != nullptr);
    if (pGraphicsClone != nullptr)
    {
        if (pGraphicsClone->m_pipeline.m_type == rgPipelineType::Graphics)
        {
            lastStage = static_cast<char>(rgPipelineStage::Fragment);
        }
        else if (pGraphicsClone->m_pipeline.m_type == rgPipelineType::Compute)
        {
            lastStage = 0;
        }
        else
        {
            // If the pipeline type isn't Graphics or Compute, something is very wrong.
            assert(false);
        }

        // Insert a horizontal line before adding buttons.
        QHBoxLayout* pHorizontalLineLayout = CreateHorizontalLine();
        m_pLayout->insertLayout(lastStage + 1, pHorizontalLineLayout);

        // Insert the pipeline state menu item to the top of the menu.
        m_pPipelineStateItem = new rgMenuPipelineStateItem(pGraphicsClone->m_pipeline.m_type, this);
        m_pLayout->insertWidget(lastStage + 2, m_pPipelineStateItem);

        // Insert a horizontal line.
        pHorizontalLineLayout = CreateHorizontalLine();
        m_pLayout->insertLayout(lastStage + 3, pHorizontalLineLayout);

        // Insert the "Build Settings" item into the top of the menu.
        m_pBuildSettingsMenuItem = new rgMenuBuildSettingsItem();
        m_pLayout->insertWidget(lastStage + 4, m_pBuildSettingsMenuItem);

        // Connect signals for each individual shader stage item in the menu.
        ConnectStageItemSignals();

        // Connect menu item signals.
        ConnectDefaultItemSignals();

        // Connect the signals for Build settings and Pipeline state buttons.
        ConnectButtonSignals();

        // Make the menu as wide as the items.
        const int width = m_pBuildSettingsMenuItem->width();
        const int height = m_pBuildSettingsMenuItem->height();
        this->resize(width, 2 * height);

        // Set cursor.
        SetCursor(Qt::ArrowCursor);

        // Register the buttons with the scaling manager.
        ScalingManager::Get().RegisterObject(m_pBuildSettingsMenuItem);
        ScalingManager::Get().RegisterObject(m_pPipelineStateItem);
    }
}

QHBoxLayout* rgMenuGraphics::CreateHorizontalLine() const
{
    // Margin constants.
    static const int s_LEFT_MARGIN = 10;
    static const int s_TOP_MARGIN = 8;
    static const int s_RIGHT_MARGIN = 10;
    static const int s_BOTTOM_MARGIN = 8;

    QHBoxLayout* pHLayout = new QHBoxLayout();
    QFrame* pFrame = new QFrame;
    pFrame->setFrameShape(QFrame::HLine);
    pFrame->setFrameShadow(QFrame::Plain);
    pFrame->setStyleSheet("color: gray");
    pHLayout->addWidget(pFrame);
    pHLayout->setContentsMargins(s_LEFT_MARGIN, s_TOP_MARGIN, s_RIGHT_MARGIN, s_BOTTOM_MARGIN);
    pHLayout->setSizeConstraint(QLayout::SetDefaultConstraint);

    return pHLayout;
}

void rgMenuGraphics::InitializeDefaultShaderStageItems(const std::shared_ptr<rgProjectClone> pProjectClone)
{
    // Zero out the stage items array.
    m_shaderStageItems = {};

    // Create an item for each shader stage.
    std::shared_ptr<rgGraphicsProjectClone> pGraphicsClone = std::dynamic_pointer_cast<rgGraphicsProjectClone>(pProjectClone);
    assert(pGraphicsClone != nullptr);
    if (pGraphicsClone != nullptr)
    {
        m_pipelineType = pGraphicsClone->m_pipeline.m_type;
        m_totalPipelineStages = GetTotalPipelineStages();
        if (pGraphicsClone->m_pipeline.m_type == rgPipelineType::Graphics)
        {
            // Step through each stage of the graphics pipeline and create a new menu item used to add files to the stage.
            char firstStage = static_cast<char>(rgPipelineStage::Vertex);
            char lastStage = static_cast<char>(rgPipelineStage::Fragment);
            for (int stageIndex = firstStage; stageIndex <= lastStage; ++stageIndex)
            {
                rgPipelineStage stage = static_cast<rgPipelineStage>(stageIndex);

                // Create a new stage item and add it to the menu.
                rgMenuFileItemGraphics* pGraphicsPipelineStageItem = new rgMenuFileItemGraphics(this, stage);
                m_pLayout->insertWidget(stageIndex, pGraphicsPipelineStageItem);
                m_shaderStageItems[stageIndex] = pGraphicsPipelineStageItem;

                // Register the stage with the scaling manager.
                ScalingManager::Get().RegisterObject(pGraphicsPipelineStageItem);

                // Connect to file item's drag and drop signal.
                bool isConnected = connect(pGraphicsPipelineStageItem, &rgMenuFileItemGraphics::DragAndDropExistingFile,
                    this, &rgMenuGraphics::HandleDragAndDropExistingFile);
                assert(isConnected);
            }
        }
        else if (pGraphicsClone->m_pipeline.m_type == rgPipelineType::Compute)
        {
            // Create the compute shader stage item.
            rgPipelineStage computeStage = rgPipelineStage::Compute;
            rgMenuFileItemGraphics* pComputePipelineStageItem = new rgMenuFileItemGraphics(this, computeStage);
            m_pLayout->insertWidget(0, pComputePipelineStageItem);
            m_shaderStageItems[static_cast<size_t>(computeStage)] = pComputePipelineStageItem;

            // Register the stage with the scaling manager.
            ScalingManager::Get().RegisterObject(pComputePipelineStageItem);

            // Connect to file item's drag and drop signal.
            bool isConnected = connect(pComputePipelineStageItem, &rgMenuFileItemGraphics::DragAndDropExistingFile,
                this, &rgMenuGraphics::HandleDragAndDropExistingFile);
            assert(isConnected);
        }
        else
        {
            // If the pipeline type isn't Graphics or Compute, something is very wrong.
            assert(false);
        }
    }
}

void rgMenuGraphics::SetStageButtonsEnabled(bool isEnabled)
{
    // Loop through each available stage in the pipeline
    // and update the enabledness of the buttons.
    for (auto stageItemIter = m_shaderStageItems.begin();
        stageItemIter != m_shaderStageItems.end(); ++stageItemIter)
    {
        // If a stage item is nullptr, it's not a problem. It just means that the
        // stage is not actively being used within the project.
        rgMenuFileItemGraphics* pStageItem = *stageItemIter;
        if (pStageItem != nullptr)
        {
            pStageItem->SetButtonsEnabled(isEnabled);
        }
    }
}

void rgMenuGraphics::SetCursor(Qt::CursorShape shape)
{
    for (rgMenuFileItemGraphics* pItem : m_shaderStageItems)
    {
        // The stage items array is fixed-size, and slots may be set to nullptr if the stage is not
        // used. Only update the cursor for valid pipeline stage slots.
        if (pItem != nullptr)
        {
            pItem->setCursor(shape);
        }
    }
}

void rgMenuGraphics::HandleBuildStarted()
{
    // Call the base class implementation.
    rgMenu::HandleBuildStarted();

    // Disable the Add/Create buttons for each stage item.
    SetStageButtonsEnabled(false);
}

void rgMenuGraphics::HandleBuildEnded()
{
    // Call the base class implementation.
    rgMenu::HandleBuildEnded();

    // Re-enable the Add/Create buttons for each stage item.
    SetStageButtonsEnabled(true);
}

void rgMenuGraphics::ConnectStageItemSignals()
{
    for (size_t stageIndex = 0; stageIndex < m_shaderStageItems.size(); ++stageIndex)
    {
        rgMenuFileItemGraphics* pStageItem = m_shaderStageItems[stageIndex];
        if (pStageItem != nullptr)
        {
            // Connect the stage item's "Add existing file" button to remove sub button focus.
            bool isConnected = connect(pStageItem, &rgMenuFileItemGraphics::AddExistingFileButtonClicked, this, &rgMenuGraphics::HandleAddExistingFileButtonClicked);
            assert(isConnected);

            // Connect the stage item's "Add existing file" button.
            isConnected = connect(pStageItem, &rgMenuFileItemGraphics::AddExistingFileButtonClicked, this, &rgMenuGraphics::AddExistingFileButtonClicked);
            assert(isConnected);

            // Connect the stage item's drag and drop event.
            isConnected = connect(pStageItem, &rgMenuFileItemGraphics::DragAndDropExistingFile, this, &rgMenuGraphics::DragAndDropExistingFile);
            assert(isConnected);

            // Connect the stage item's "Create new file" button.
            isConnected = connect(pStageItem, &rgMenuFileItemGraphics::CreateSourceFileButtonClicked, this, &rgMenuGraphics::CreateNewFileButtonClicked);
            assert(isConnected);

            // Connect the stage item's "Remove source file" button.
            isConnected = connect(pStageItem, &rgMenuFileItemGraphics::RemoveSourceFileButtonClicked, this, &rgMenuGraphics::RemoveFileButtonClicked);
            assert(isConnected);

            // Connect the stage item's "Restore original SPIR-V binary" button.
            isConnected = connect(pStageItem, &rgMenuFileItemGraphics::RestoreOriginalSpvButtonClicked, this, &rgMenuGraphics::RestoreOriginalSpirvClicked);
            assert(isConnected);
        }
    }
}

void rgMenuGraphics::ConnectButtonSignals()
{
    // Connect the Build settings button's clicked signal.
    bool isConnected = connect(m_pBuildSettingsMenuItem, &rgMenuBuildSettingsItem::BuildSettingsButtonClicked, this, &rgMenuGraphics::HandleBuildSettingsButtonClicked);
    assert(isConnected);

    // Connect the pipeline state button's drag and drop file signal.
    isConnected = connect(m_pPipelineStateItem, &rgMenuPipelineStateItem::DragAndDropExistingFile, this, &rgMenuGraphics::DragAndDropExistingPsoFile);
    assert(isConnected);
}

int rgMenuGraphics::GetButtonCount() const
{
    // There are 2 extra buttons besides the file items:
    // Build settings
    // Pipeline state
    static const int s_EXTRA_GRAPHICS_BUTTON_COUNT = 2;
    return s_EXTRA_GRAPHICS_BUTTON_COUNT;
}

void rgMenuGraphics::HandleDragAndDropExistingFile()
{
    SetButtonsNoFocus();
}

void rgMenuGraphics::SetButtonsNoFocus()
{
    assert(m_pBuildSettingsMenuItem != nullptr);
    assert(m_pPipelineStateItem != nullptr);

    // Set the pipeline state and build settings buttons to have focus out style sheets.
    if (m_pBuildSettingsMenuItem != nullptr && m_pPipelineStateItem != nullptr)
    {
        m_pBuildSettingsMenuItem->SetCurrent(false);
        m_pPipelineStateItem->SetCurrent(false);
    }
}

rgMenuFileItemGraphics* rgMenuGraphics::GetStageItem(rgPipelineStage stage) const
{
    rgMenuFileItemGraphics* pResultWidget = nullptr;
    switch (stage)
    {
    case rgPipelineStage::Vertex:
    case rgPipelineStage::TessellationControl:
    case rgPipelineStage::TessellationEvaluation:
    case rgPipelineStage::Geometry:
    case rgPipelineStage::Fragment:
        pResultWidget = m_shaderStageItems[static_cast<size_t>(stage)];
        break;
    case rgPipelineStage::Compute:
        {
            pResultWidget = m_shaderStageItems[static_cast<size_t>(rgPipelineStage::Compute)];
        }
        break;
    default:
        assert(false);
        break;
    }

    return pResultWidget;
}

void rgMenuGraphics::RebuildMenuItemsList()
{
    // Erase, and then reconstruct the menu item list in order by stage.
    m_menuItems.clear();

    for (auto pStageItem : m_shaderStageItems)
    {
        if (pStageItem != nullptr)
        {
            // If a stage item has a filename, add the item to the ordered list.
            if (!pStageItem->GetFilename().empty())
            {
                m_menuItems.push_back(pStageItem);
            }
        }
    }
}

rgMenuPipelineStateItem* rgMenuGraphics::GetPipelineStateItem() const
{
    return m_pPipelineStateItem;
}

void rgMenuGraphics::HandleTabFocusPressed()
{
    // Remove focus from the sub buttons.
    RemoveSubButtonFocus();

    // Hide the remove file push button.
    HideRemoveFilePushButton();

    // Clear style sheets for all buttons.
    ClearButtonStylesheets();

    // Refresh focus.
    if (m_tabFocusIndex < m_totalPipelineStages)
    {
        if ((m_hasTabFocus == GraphicsMenuTabFocusItems::RemoveButton) ||
           (m_hasTabFocus == GraphicsMenuTabFocusItems::AddExistingFileButton))
        {
            IncrementTabFocusIndex();

            // Reset the sub-button tab focus.
            m_hasTabFocus = GraphicsMenuTabFocusItems::NoButton;
            if (m_tabFocusIndex < m_totalPipelineStages)
            {
                rgMenuFileItemGraphics* pItem = GetCurrentFileMenuItem();
                if (pItem != nullptr)
                {
                    if (!pItem->GetFilename().empty())
                    {
                        SelectTabFocusItem(false);
                    }
                    else
                    {
                        pItem->SetButtonHighlighted(GraphicsMenuTabFocusItems::AddExistingFileButton);
                        m_hasTabFocus = GraphicsMenuTabFocusItems::AddExistingFileButton;
                    }
                }
            }
            else
            {
                // Figure out the correct button index.
                m_focusIndex = m_tabFocusIndex;
                SelectTabFocusItem(false);
            }
        }
        else if (m_hasTabFocus == GraphicsMenuTabFocusItems::NoButton)
        {
            // Select the next item in the file menu.
            SelectNextItem(FileMenuActionType::TabAction);
        }
    }
    else
    {
        IncrementTabFocusIndex();

        // Select the next item in the file menu.
        SelectTabFocusItem(false);
    }
}

void rgMenuGraphics::SelectNextItem(FileMenuActionType actionType)
{
    // Figure out the correct button index.
    if (m_tabFocusIndex < m_totalPipelineStages)
    {
        rgMenuFileItemGraphics* pItem = GetCurrentFileMenuItem();

        assert(pItem != nullptr);
        if (pItem != nullptr && !pItem->GetFilename().empty())
        {
            if (m_hasTabFocus == GraphicsMenuTabFocusItems::RemoveButton ||
                m_hasTabFocus == GraphicsMenuTabFocusItems::AddExistingFileButton)
            {
                SelectTabFocusItem(false);
                m_hasTabFocus = GraphicsMenuTabFocusItems::NoButton;
            }
            else if (m_hasTabFocus == GraphicsMenuTabFocusItems::NoButton)
            {
                pItem->SetButtonHighlighted(GraphicsMenuTabFocusItems::RemoveButton);
                m_hasTabFocus = GraphicsMenuTabFocusItems::RemoveButton;
            }
        }
        else
        {
            pItem->SetButtonHighlighted(GraphicsMenuTabFocusItems::AddExistingFileButton);
            m_hasTabFocus = GraphicsMenuTabFocusItems::AddExistingFileButton;
        }
    }
    else
    {
        SelectFocusItem(actionType);
    }
}

void rgMenuGraphics::SelectPreviousItem(FileMenuActionType actionType)
{
    // Figure out the correct button index.
    if (m_tabFocusIndex < m_totalPipelineStages)
    {
        rgMenuFileItemGraphics* pItem = GetCurrentFileMenuItem();
        assert(pItem != nullptr);
        if (pItem != nullptr)
        {
            if (!pItem->GetFilename().empty())
            {
                if (m_hasTabFocus == GraphicsMenuTabFocusItems::RemoveButton)
                {
                    SelectTabFocusItem(true);
                    m_hasTabFocus = GraphicsMenuTabFocusItems::NoButton;
                }
                else
                {
                    pItem->SetButtonHighlighted(GraphicsMenuTabFocusItems::RemoveButton);
                    m_hasTabFocus = GraphicsMenuTabFocusItems::RemoveButton;
                }
            }
            else
            {
                pItem->SetButtonHighlighted(GraphicsMenuTabFocusItems::AddExistingFileButton);
                m_hasTabFocus = GraphicsMenuTabFocusItems::AddExistingFileButton;
            }
        }
    }
    else
    {
        SelectFocusItem(actionType);
    }
}

void rgMenuGraphics::IncrementTabFocusIndex()
{
    // Clear highlight for all file menu items.
    ClearFileMenuHighlight();

    // Advance the tab focus index.
    int offset = GetButtonCount() - 1;
    size_t endIndex = m_totalPipelineStages + offset + 1;

    // Increment focus index and wrap around.
    m_tabFocusIndex++;
    if (m_tabFocusIndex >= endIndex)
    {
        m_tabFocusIndex = 0;
        m_focusIndex = 0;

        // We've reached the end of file menu, so now
        // give focus to the next view.
        emit FocusNextView();
    }

    // Update the focus index.
    UpdateFocusIndex();

}

void rgMenuGraphics::DecrementTabFocusIndex()
{
    // Clear highlight for all file menu items.
    ClearFileMenuHighlight();

    // Decrement the tab focus index.
    int offset = GetButtonCount() - 1;
    size_t endIndex = m_totalPipelineStages + offset;

    // Decrement focus index and wrap around.
    if (m_tabFocusIndex > 0)
    {
        m_tabFocusIndex--;
    }
    else
    {
        m_tabFocusIndex = 0;

        // We've reached the beginning of file menu, so now
        // give focus to the previous view.
        emit FocusPrevView();
    }

    // Update the focus index.
    UpdateFocusIndex();
}

void rgMenuGraphics::UpdateFocusIndex()
{
    if (m_tabFocusIndex < m_totalPipelineStages)
    {
        // Update m_focusIndex only if m_tabFocusIndex points to an occupied stage.
        rgMenuFileItemGraphics* pItem = GetCurrentFileMenuItem();
        assert(pItem != nullptr);
        if (pItem != nullptr)
        {
            int count = 0;
            for (const auto& pGraphicsItem : m_menuItems)
            {
                if (pGraphicsItem == pItem)
                {
                    m_focusIndex = count;
                    break;
                }
                count++;
            }
        }
    }
    else
    {
        // Update the focus index.
        m_focusIndex = m_tabFocusIndex;
    }
}

void rgMenuGraphics::SelectFocusSubButton(rgMenuFileItemGraphics* pItem)
{
    assert(pItem != nullptr);

    if ((pItem != nullptr) && (!pItem->GetFilename().empty()))
    {
        if (m_hasTabFocus == GraphicsMenuTabFocusItems::NoButton)
        {
            if (pItem != nullptr)
            {
                // Give focus to the remove file button.
                pItem->SetButtonHighlighted(GraphicsMenuTabFocusItems::RemoveButton);
                m_hasTabFocus = GraphicsMenuTabFocusItems::RemoveButton;
            }
        }
    }
    else
    {
        if (pItem != nullptr)
        {
            pItem->SetButtonHighlighted(GraphicsMenuTabFocusItems::AddExistingFileButton);
            m_hasTabFocus = GraphicsMenuTabFocusItems::AddExistingFileButton;
        }
    }
}

void rgMenuGraphics::HandleShiftTabFocusPressed()
{
    // Remove focus from the sub buttons.
    RemoveSubButtonFocus();

    // Clear style sheets for all buttons.
    ClearButtonStylesheets();

    // Hide the remove file push button.
    HideRemoveFilePushButton();

    // Refresh focus.
    if (m_tabFocusIndex < m_totalPipelineStages)
    {
        if (m_hasTabFocus == GraphicsMenuTabFocusItems::AddExistingFileButton)
        {
            DecrementTabFocusIndex();

            // Reset the sub-button tab focus.
            m_hasTabFocus = GraphicsMenuTabFocusItems::NoButton;
            if (m_tabFocusIndex < m_totalPipelineStages)
            {
                rgMenuFileItemGraphics* pItem = GetCurrentFileMenuItem();
                assert(pItem != nullptr);
                if (pItem != nullptr)
                {
                    if (!pItem->GetFilename().empty())
                    {
                        if (m_hasTabFocus == GraphicsMenuTabFocusItems::NoButton)
                        {
                            UpdateHighlight(pItem);
                            pItem->SetButtonHighlighted(GraphicsMenuTabFocusItems::RemoveButton);
                            m_hasTabFocus = GraphicsMenuTabFocusItems::RemoveButton;
                        }
                        else if (m_hasTabFocus == GraphicsMenuTabFocusItems::RemoveButton)
                        {
                            SelectTabFocusItem(true);
                            m_hasTabFocus = GraphicsMenuTabFocusItems::NoButton;
                        }
                    }
                    else if (pItem->GetFilename().empty())
                    {
                        if (m_hasTabFocus == GraphicsMenuTabFocusItems::NoButton)
                        {
                            pItem->SetButtonHighlighted(GraphicsMenuTabFocusItems::AddExistingFileButton);
                            m_hasTabFocus = GraphicsMenuTabFocusItems::AddExistingFileButton;
                        }
                        else
                        {
                            DecrementTabFocusIndex();
                            m_hasTabFocus = GraphicsMenuTabFocusItems::NoButton;
                        }
                    }
                }
            }
            else
            {
                // Update the focus index.
                m_focusIndex = m_tabFocusIndex;
                SelectTabFocusItem(true);
            }
        }
        else if (m_hasTabFocus == GraphicsMenuTabFocusItems::RemoveButton)
        {
            SelectTabFocusItem(true);
            m_hasTabFocus = GraphicsMenuTabFocusItems::NoButton;
        }
        else if (m_hasTabFocus == GraphicsMenuTabFocusItems::NoButton)
        {
            DecrementTabFocusIndex();

            // Select the previous item in the file menu.
            SelectPreviousItem(FileMenuActionType::TabAction);
        }
    }
    else
    {
        DecrementTabFocusIndex();

        // Select the previous item in the file menu.
        SelectTabFocusItem(true);
    }
}

void rgMenuGraphics::HandleBuildSettingsButtonClicked(bool /* checked */)
{
    // Clear the file menu item selection.
    ClearFileMenuItemSelection();

    // Clear the sub button highlights.
    RemoveSubButtonFocus();

    assert(m_pBuildSettingsMenuItem != nullptr);
    assert(m_pPipelineStateItem != nullptr);
    if (m_pBuildSettingsMenuItem != nullptr && m_pPipelineStateItem != nullptr)
    {
        // Set the focus index.
        m_focusIndex = m_menuItems.size() + GetButtonCount() - s_PIPELINE_STATE_BUTTON_INDEX;

        // Update button selection values.
        m_pPipelineStateItem->SetCurrent(false);
        m_pBuildSettingsMenuItem->SetCurrent(true);
    }
}

void rgMenuGraphics::HandlePipelineStateButtonClicked(bool /* checked */)
{
    // Clear the file menu item selection.
    ClearFileMenuItemSelection();

    // Clear the sub button highlights.
    RemoveSubButtonFocus();

    assert(m_pBuildSettingsMenuItem != nullptr);
    assert(m_pPipelineStateItem != nullptr);
    if (m_pBuildSettingsMenuItem != nullptr && m_pPipelineStateItem != nullptr)
    {
        // Set the focus index.
        m_focusIndex = m_menuItems.size() + GetButtonCount() - s_BUILD_SETTINGS_BUTTON_INDEX;

        // Update button selection values.
        m_pPipelineStateItem->SetCurrent(true);
        m_pBuildSettingsMenuItem->SetCurrent(false);
    }
}

void rgMenuGraphics::RemoveSubButtonFocus()
{
    // Remove sub button focus from all file items.
    for (rgMenuFileItemGraphics* pItem : m_shaderStageItems)
    {
        // Do not assert here since pItem could be null
        // if this is compute pipeline item while in graphics pipeline,
        // and vice versa.
        if (pItem != nullptr)
        {
            pItem->RemoveSubButtonFocus();
        }
    }
}

void rgMenuGraphics::HideRemoveFilePushButton()
{
    // Hide the remove file push button for all file items.
    for (rgMenuFileItemGraphics* pItem : m_shaderStageItems)
    {
        // Do not assert here since pItem could be null
        // if this is compute pipeline item while in graphics pipeline,
        // and vice versa.
        if (pItem != nullptr)
        {
            pItem->HideRemoveFilePushButton();
        }
    }
}

rgMenuFileItemGraphics* rgMenuGraphics::GetCurrentFileMenuItem() const
{
    rgMenuFileItemGraphics* pItem = nullptr;
    switch (m_pipelineType)
    {
    case (rgPipelineType::Graphics):
    {
        if (m_tabFocusIndex < s_NUMBER_OF_GRAPHICS_PIPELINE_STAGES)
        {
            pItem = static_cast<rgMenuFileItemGraphics*>(m_shaderStageItems[m_tabFocusIndex]);
        }
    }
    break;
    case (rgPipelineType::Compute):
    {
        if (m_tabFocusIndex < s_NUMBER_OF_COMPUTE_PIPELINE_STAGES)
        {
            pItem = static_cast<rgMenuFileItemGraphics*>(m_shaderStageItems[static_cast<int>(rgPipelineStage::Compute)]);
        }
    }
    break;
    default:
        assert(false);
        break;
    }

    return pItem;
}

rgPipelineStage rgMenuGraphics::GetCurrentStage() const
{
    return m_currentStage;
}

void rgMenuGraphics::HandleAddExistingFileButtonClicked(rgPipelineStage stage)
{
    // Remove focus from all sub buttons.
    RemoveSubButtonFocus();

    // Now add back the focus for the current stage's "Add existing file" button.
    rgMenuFileItemGraphics* pStageItem = GetStageItem(stage);
    assert(pStageItem != nullptr);
    if (pStageItem != nullptr)
    {
        pStageItem->SetButtonHighlighted(GraphicsMenuTabFocusItems::AddExistingFileButton);
    }

    // Also update focus indexes.
    UpdateFocusIndex(stage);
}

void rgMenuGraphics::UpdateFocusIndex(rgPipelineStage stage)
{
    if (m_pipelineType == rgPipelineType::Compute)
    {
        m_focusIndex = 0;
        m_tabFocusIndex = 0;
    }
    else if (m_pipelineType == rgPipelineType::Graphics)
    {
        m_focusIndex = static_cast<int>(stage);
        m_tabFocusIndex = static_cast<int>(stage);
    }
    else
    {
        // Should not be here.
        assert(false);
    }
}

size_t rgMenuGraphics::GetTotalPipelineStages() const
{
    size_t totalPipelineStages = 0;

    if (m_pipelineType == rgPipelineType::Compute)
    {
        totalPipelineStages = s_NUMBER_OF_COMPUTE_PIPELINE_STAGES;
    }
    else if (m_pipelineType == rgPipelineType::Graphics)
    {
        totalPipelineStages = s_NUMBER_OF_GRAPHICS_PIPELINE_STAGES;
    }
    else
    {
        // Should not be here.
        assert(false);
    }

    return totalPipelineStages;
}

size_t rgMenuGraphics::GetNumberPipelineStagesToCycleThrough(FileMenuActionType actionType) const
{
    size_t totalOccupiedPipelineStages = 0;

    switch (actionType)
    {
    case FileMenuActionType::ArrowAction:
    case FileMenuActionType::TabAction:
    {
        if (m_pipelineType == rgPipelineType::Compute)
        {
            totalOccupiedPipelineStages = s_NUMBER_OF_COMPUTE_PIPELINE_STAGES;
        }
        else if (m_pipelineType == rgPipelineType::Graphics)
        {
            totalOccupiedPipelineStages = s_NUMBER_OF_GRAPHICS_PIPELINE_STAGES;
        }
        else
        {
            // Should not be here.
            assert(false);
        }
        break;
    }
    default:
        // We shouldn't get here.
        assert(false);
        break;
    }

    return totalOccupiedPipelineStages;
}

std::vector<rgMenuFileItem*> rgMenuGraphics::GetFileMenuItemsToProcess(FileMenuActionType actionType) const
{
    std::vector<rgMenuFileItem*> vectorToProcess;

    switch (actionType)
    {
    case FileMenuActionType::ArrowAction:
    case FileMenuActionType::TabAction:
    {
        std::vector<rgMenuFileItem*> vectorToProcess(std::begin(m_shaderStageItems), std::end(m_shaderStageItems));
        break;
    }
    default:
        // We shouldn't get here.
        assert(false);
        break;
    }

    return vectorToProcess;
}

void rgMenuGraphics::HandleRemoveFileMenuButtonFocus()
{
    RemoveSubButtonFocus();
    RemoveFileMenuButtonFocus();
}

void rgMenuGraphics::SetCurrent(rgPipelineStage stage, bool isCurrent)
{
    rgMenuFileItemGraphics* pStage = GetStageItem(stage);
    if (pStage != nullptr)
    {
        pStage->SetCurrent(isCurrent);
        pStage->SetHovered(isCurrent);
    }
}

void rgMenuGraphics::RemoveFileMenuButtonFocus()
{
    assert(m_pBuildSettingsMenuItem != nullptr);
    assert(m_pPipelineStateItem != nullptr);
    if (m_pBuildSettingsMenuItem != nullptr && m_pPipelineStateItem != nullptr)
    {
        // Set the pipeline state and build settings buttons to have focus out style sheets.
        // Verify that the button isn't currently selected before removing the focus.
        if (!m_pBuildSettingsMenuItem->IsCurrent())
        {
            m_pBuildSettingsMenuItem->GetBuildSettingsButton()->setStyleSheet(s_BUTTON_FOCUS_OUT_STYLESHEET);
            m_pBuildSettingsMenuItem->GetBuildSettingsButton()->setCursor(Qt::PointingHandCursor);
        }

        if (!m_pPipelineStateItem->IsCurrent())
        {
            m_pPipelineStateItem->GetPipelineStateButton()->setStyleSheet(s_BUTTON_FOCUS_OUT_STYLESHEET);
            m_pPipelineStateItem->GetPipelineStateButton()->setCursor(Qt::PointingHandCursor);
        }
    }
}

void rgMenuGraphics::mousePressEvent(QMouseEvent* pEvent)
{
    emit MenuClicked();
    emit FileMenuFocusInEvent();
}

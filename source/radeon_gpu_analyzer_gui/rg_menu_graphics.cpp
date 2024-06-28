// C++.
#include <cassert>

// Qt.
#include <QKeyEvent>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_menu_build_settings_item.h"
#include "radeon_gpu_analyzer_gui/qt/rg_menu_graphics.h"
#include "radeon_gpu_analyzer_gui/qt/rg_menu_pipeline_state_item.h"
#include "radeon_gpu_analyzer_gui/qt/rg_menu_file_item_graphics.h"

static const char* kStrButtonFocusInStylesheetGraphics = "QPushButton { background: rgb(253,255,174); border-style: solid; border-width: 2px; border-color: rgb(135, 20, 16);}";
static const char* kButtonTabbedStylesheet = "QPushButton { border: 2px solid rgb(135,20,16); margin: 1px; background: rgb(214,214,214);}";
static const char* kFileMenuNameGraphics = "fileMenuGraphics";

static const int kPipelineStateButtonIndex = 1;
static const int kBuildSettingsButtonIndex = 2;

static const int kNumberOfGraphicsPipelineStages = 5;
static const int kNumberOfComputePipelineStages = 1;

RgMenuGraphics::RgMenuGraphics(QWidget* parent)
    : RgMenu(parent)
{
    // Set the object name.
    setObjectName(kFileMenuNameGraphics);
}

void RgMenuGraphics::SelectFocusItem(FileMenuActionType action_type)
{
    assert(pipeline_state_item_ != nullptr);
    assert(build_settings_menu_item_ != nullptr);
    if (pipeline_state_item_ != nullptr && build_settings_menu_item_ != nullptr)
    {
        ClearButtonStylesheets();

        // Find out total stages and which vector to use.
        const size_t total_pipeline_stages = GetNumberPipelineStagesToCycleThrough(action_type);
        const std::vector<RgMenuFileItem*> menu_file_items = GetFileMenuItemsToProcess(action_type);

        // If focus index is in the range of the menu file items, select the appropriate file item.
        if (focus_index_ < total_pipeline_stages)
        {
            RgMenuFileItemGraphics* item = static_cast<RgMenuFileItemGraphics*>(menu_file_items[focus_index_]);
            assert(item != nullptr);
            if (item != nullptr)
            {
                UpdateHighlight(item);
            }
        }
        else
        {
            // Deselect graphics menu items.
            DeselectFileItems();

            // Out of range, so special case handle the last focus items.
            // Get index excluding file items.
            size_t index = focus_index_ - total_pipeline_stages;

            // Handle special cases for pipeline state and build settings buttons.
            SelectButton(index);
        }
    }
}

void RgMenuGraphics::SelectButton(size_t index)
{
    // Handle special cases for pipeline state and build settings buttons.
    switch (index)
    {
    case static_cast<size_t>(GraphicsMenuFocusItems::kBuildSettingsButton) :
        if (!build_settings_menu_item_->IsCurrent())
        {
            build_settings_menu_item_->GetBuildSettingsButton()->setStyleSheet(kButtonTabbedStylesheet);
            has_tab_focus_ = GraphicsMenuTabFocusItems::kNoButton;

            // Disable the build settings Build menu item.
            emit EnableBuildSettingsMenuItem(true);
        }
        break;
    case static_cast<size_t>(GraphicsMenuFocusItems::kPipelineStateButton) :
        if (!pipeline_state_item_->IsCurrent())
        {
            pipeline_state_item_->GetPipelineStateButton()->setStyleSheet(kButtonTabbedStylesheet);
            has_tab_focus_ = GraphicsMenuTabFocusItems::kNoButton;

            // Disable the Pipeline state Build menu item.
            emit EnablePipelineMenuItem(true);
        }
        break;
    default:
        // Should never get here.
        assert(false);
        break;
    }
}

void RgMenuGraphics::SelectTabFocusItem(bool shift_tab_focus)
{
    Q_UNUSED(shift_tab_focus);

    assert(pipeline_state_item_ != nullptr);
    assert(build_settings_menu_item_ != nullptr);
    if (pipeline_state_item_ != nullptr && build_settings_menu_item_ != nullptr)
    {
        ClearButtonStylesheets();

        // If focus index is in the range of the menu file items, select the appropriate file item.
        if (tab_focus_index_ < total_pipeline_stages_)
        {
            RgMenuFileItemGraphics* item = GetCurrentFileMenuItem();
            assert(item != nullptr);
            if (item != nullptr)
            {
                if (!item->GetFilename().empty())
                {
                    UpdateHighlight(item);
                }
                else
                {

                    item->SetButtonHighlighted(GraphicsMenuTabFocusItems::kAddExistingFileButton);
                    has_tab_focus_ = GraphicsMenuTabFocusItems::kAddExistingFileButton;
                }
            }
        }
        else
        {
            // Deselect graphics menu items.
            DeselectFileItems();

            // Out of range, so special case handle the last focus items.
            // Get index excluding file items.
            size_t index = focus_index_ - total_pipeline_stages_;

            // Handle special cases for pipeline state and build settings buttons.
            SelectButton(index);
        }
    }
}

void RgMenuGraphics::DeselectFileItems()
{
    // Deselect graphics menu file items.
    for (RgMenuFileItem* item : menu_items_)
    {
        assert(item != nullptr);
        if (item != nullptr)
        {
            item->SetHovered(false);
        }
    }
}

void RgMenuGraphics::ClearButtonStylesheets()
{
    // Clear button style sheets.
    if (!build_settings_menu_item_->IsCurrent())
    {
        build_settings_menu_item_->GetBuildSettingsButton()->setStyleSheet("");
    }

    if (!pipeline_state_item_->IsCurrent())
    {
        pipeline_state_item_->GetPipelineStateButton()->setStyleSheet("");
    }
}

void RgMenuGraphics::InitializeDefaultMenuItems(const std::shared_ptr<RgProjectClone> project_clone)
{
    // Initialize the default shader stage items.
    InitializeDefaultShaderStageItems(project_clone);

    // Get the index for the last widget added.
    int                                     last_stage{};
    std::shared_ptr<RgGraphicsProjectClone> graphics_clone = std::dynamic_pointer_cast<RgGraphicsProjectClone>(project_clone);
    assert(graphics_clone != nullptr);
    if (graphics_clone != nullptr)
    {
        if (graphics_clone->pipeline.type == RgPipelineType::kGraphics)
        {
            last_stage = static_cast<char>(RgPipelineStage::kFragment);
        }
        else if (graphics_clone->pipeline.type == RgPipelineType::kCompute)
        {
            last_stage = 0;
        }
        else
        {
            // If the pipeline type isn't Graphics or Compute, something is very wrong.
            assert(false);
        }

        // Insert a horizontal line before adding buttons.
        QHBoxLayout* horizontal_line_layout = CreateHorizontalLine();
        layout_->insertLayout(last_stage + 1, horizontal_line_layout);

        // Insert the pipeline state menu item to the top of the menu.
        pipeline_state_item_ = new RgMenuPipelineStateItem(graphics_clone->pipeline.type, this);
        layout_->insertWidget(last_stage + 2, pipeline_state_item_);

        // Insert a horizontal line.
        horizontal_line_layout = CreateHorizontalLine();
        layout_->insertLayout(last_stage + 3, horizontal_line_layout);

        // Insert the "Build Settings" item into the top of the menu.
        build_settings_menu_item_ = new RgMenuBuildSettingsItem(nullptr, kStrMenuBarBuildSettingsTooltip);
        layout_->insertWidget(last_stage + 4, build_settings_menu_item_);

        // Connect signals for each individual shader stage item in the menu.
        ConnectStageItemSignals();

        // Connect menu item signals.
        ConnectDefaultItemSignals();

        // Connect the signals for Build settings and Pipeline state buttons.
        ConnectButtonSignals();

        // Make the menu as wide as the items.
        const int width = build_settings_menu_item_->width();
        const int height = build_settings_menu_item_->height();
        this->resize(width, 2 * height);

        // Set cursor.
        SetCursor(Qt::ArrowCursor);
    }
}

QHBoxLayout* RgMenuGraphics::CreateHorizontalLine() const
{
    // Margin constants.
    static const int kLeftMargin = 10;
    static const int kTopMargin = 8;
    static const int kRightMargin = 10;
    static const int kBottomMargin = 8;

    QHBoxLayout* h_layout = new QHBoxLayout();
    QFrame* frame = new QFrame;
    frame->setFrameShape(QFrame::HLine);
    frame->setFrameShadow(QFrame::Plain);
    frame->setStyleSheet("color: gray");
    h_layout->addWidget(frame);
    h_layout->setContentsMargins(kLeftMargin, kTopMargin, kRightMargin, kBottomMargin);
    h_layout->setSizeConstraint(QLayout::SetDefaultConstraint);

    return h_layout;
}

void RgMenuGraphics::InitializeDefaultShaderStageItems(const std::shared_ptr<RgProjectClone> project_clone)
{
    // Zero out the stage items array.
    shader_stage_items_ = {};

    // Create an item for each shader stage.
    std::shared_ptr<RgGraphicsProjectClone> graphics_clone = std::dynamic_pointer_cast<RgGraphicsProjectClone>(project_clone);
    assert(graphics_clone != nullptr);
    if (graphics_clone != nullptr)
    {
        pipeline_type_ = graphics_clone->pipeline.type;
        total_pipeline_stages_ = GetTotalPipelineStages();
        if (graphics_clone->pipeline.type == RgPipelineType::kGraphics)
        {
            // Step through each stage of the graphics pipeline and create a new menu item used to add files to the stage.
            char first_stage = static_cast<char>(RgPipelineStage::kVertex);
            char last_stage = static_cast<char>(RgPipelineStage::kFragment);
            for (int stage_index = first_stage; stage_index <= last_stage; ++stage_index)
            {
                RgPipelineStage stage = static_cast<RgPipelineStage>(stage_index);

                // Create a new stage item and add it to the menu.
                RgMenuFileItemGraphics* graphics_pipeline_stage_item = new RgMenuFileItemGraphics(this, stage);
                layout_->insertWidget(stage_index, graphics_pipeline_stage_item);
                shader_stage_items_[stage_index] = graphics_pipeline_stage_item;

                // Connect to file item's drag and drop signal.
                bool is_connected = connect(graphics_pipeline_stage_item, &RgMenuFileItemGraphics::DragAndDropExistingFile,
                    this, &RgMenuGraphics::HandleDragAndDropExistingFile);
                assert(is_connected);

                // Connect the enable pipeline state menu item signal.
                is_connected = connect(graphics_pipeline_stage_item, &RgMenuFileItemGraphics::EnablePipelineMenuItem, this, &RgMenuGraphics::EnablePipelineMenuItem);
                assert(is_connected);

                // Connect the enable build settings menu item signal.
                is_connected = connect(graphics_pipeline_stage_item, &RgMenuFileItemGraphics::EnableBuildSettingsMenuItem, this, &RgMenuGraphics::EnableBuildSettingsMenuItem);
                assert(is_connected);
            }
        }
        else if (graphics_clone->pipeline.type == RgPipelineType::kCompute)
        {
            // Create the compute shader stage item.
            RgPipelineStage compute_stage = RgPipelineStage::kCompute;
            RgMenuFileItemGraphics* compute_pipeline_stage_item = new RgMenuFileItemGraphics(this, compute_stage);
            layout_->insertWidget(0, compute_pipeline_stage_item);
            shader_stage_items_[static_cast<size_t>(compute_stage)] = compute_pipeline_stage_item;

            // Connect to file item's drag and drop signal.
            bool is_connected = connect(compute_pipeline_stage_item, &RgMenuFileItemGraphics::DragAndDropExistingFile,
                this, &RgMenuGraphics::HandleDragAndDropExistingFile);
            assert(is_connected);

            // Connect the enable pipeline state menu item signal.
            is_connected = connect(compute_pipeline_stage_item, &RgMenuFileItemGraphics::EnablePipelineMenuItem, this, &RgMenuGraphics::EnablePipelineMenuItem);
            assert(is_connected);

            // Connect the enable build settings menu item signal.
            is_connected = connect(compute_pipeline_stage_item, &RgMenuFileItemGraphics::EnableBuildSettingsMenuItem, this, &RgMenuGraphics::EnableBuildSettingsMenuItem);
            assert(is_connected);
        }
        else
        {
            // If the pipeline type isn't Graphics or Compute, something is very wrong.
            assert(false);
        }
    }
}

void RgMenuGraphics::SetStageButtonsEnabled(bool is_enabled)
{
    // Loop through each available stage in the pipeline
    // and update the enabledness of the buttons.
    for (auto stage_item_iter = shader_stage_items_.begin();
        stage_item_iter != shader_stage_items_.end(); ++stage_item_iter)
    {
        // If a stage item is nullptr, it's not a problem. It just means that the
        // stage is not actively being used within the project.
        RgMenuFileItemGraphics* stage_item = *stage_item_iter;
        if (stage_item != nullptr)
        {
            stage_item->SetButtonsEnabled(is_enabled);
        }
    }
}

void RgMenuGraphics::SetCursor(Qt::CursorShape shape)
{
    for (RgMenuFileItemGraphics* item : shader_stage_items_)
    {
        // The stage items array is fixed-size, and slots may be set to nullptr if the stage is not
        // used. Only update the cursor for valid pipeline stage slots.
        if (item != nullptr)
        {
            item->setCursor(shape);
        }
    }
}

void RgMenuGraphics::HandleBuildStarted()
{
    // Call the base class implementation.
    RgMenu::HandleBuildStarted();

    // Disable the Add/Create buttons for each stage item.
    SetStageButtonsEnabled(false);
}

void RgMenuGraphics::HandleBuildEnded()
{
    // Call the base class implementation.
    RgMenu::HandleBuildEnded();

    // Re-enable the Add/Create buttons for each stage item.
    SetStageButtonsEnabled(true);
}

void RgMenuGraphics::ConnectStageItemSignals()
{
    for (size_t stage_index = 0; stage_index < shader_stage_items_.size(); ++stage_index)
    {
        RgMenuFileItemGraphics* stage_item = shader_stage_items_[stage_index];
        if (stage_item != nullptr)
        {
            // Connect the stage item's "Add existing file" button to remove sub button focus.
            bool is_connected = connect(stage_item, &RgMenuFileItemGraphics::AddExistingFileButtonClicked, this, &RgMenuGraphics::HandleAddExistingFileButtonClicked);
            assert(is_connected);

            // Connect the stage item's "Add existing file" button.
            is_connected = connect(stage_item, &RgMenuFileItemGraphics::AddExistingFileButtonClicked, this, &RgMenuGraphics::AddExistingFileButtonClicked);
            assert(is_connected);

            // Connect the stage item's drag and drop event.
            is_connected = connect(stage_item, &RgMenuFileItemGraphics::DragAndDropExistingFile, this, &RgMenuGraphics::DragAndDropExistingFile);
            assert(is_connected);

            // Connect the stage item's "Create new file" button.
            is_connected = connect(stage_item, &RgMenuFileItemGraphics::CreateSourceFileButtonClicked, this, &RgMenuGraphics::CreateNewFileButtonClicked);
            assert(is_connected);

            // Connect the stage item's "Remove source file" button.
            is_connected = connect(stage_item, &RgMenuFileItemGraphics::RemoveSourceFileButtonClicked, this, &RgMenuGraphics::RemoveFileButtonClicked);
            assert(is_connected);

            // Connect the stage item's "Restore original SPIR-V binary" button.
            is_connected = connect(stage_item, &RgMenuFileItemGraphics::RestoreOriginalSpvButtonClicked, this, &RgMenuGraphics::RestoreOriginalSpirvClicked);
            assert(is_connected);
        }
    }
}

void RgMenuGraphics::ConnectButtonSignals()
{
    // Connect the Build settings button's clicked signal.
    bool is_connected = connect(build_settings_menu_item_, &RgMenuBuildSettingsItem::BuildSettingsButtonClicked, this, &RgMenuGraphics::HandleBuildSettingsButtonClicked);
    assert(is_connected);

    // Connect the pipeline state button's drag and drop file signal.
    is_connected = connect(pipeline_state_item_, &RgMenuPipelineStateItem::DragAndDropExistingFile, this, &RgMenuGraphics::DragAndDropExistingPsoFile);
    assert(is_connected);

    // Connect the enable pipeline state menu item signal.
    is_connected = connect(pipeline_state_item_, &RgMenuPipelineStateItem::EnablePipelineMenuItem, this, &RgMenuGraphics::HandleEnablePipelineMenuItem);
    assert(is_connected);
    is_connected = connect(pipeline_state_item_, &RgMenuPipelineStateItem::EnableBuildSettingsMenuItem, this, &RgMenuGraphics::HandleEnableBuildSettingsMenuItem);
    assert(is_connected);

    // Connect the enable build settings menu item signal.
    is_connected = connect(build_settings_menu_item_, &RgMenuBuildSettingsItem::EnablePipelineMenuItem, this, &RgMenuGraphics::HandleEnablePipelineMenuItem);
    assert(is_connected);
    is_connected = connect(build_settings_menu_item_, &RgMenuBuildSettingsItem::EnableBuildSettingsMenuItem, this, &RgMenuGraphics::HandleEnableBuildSettingsMenuItem);
    assert(is_connected);
}

void RgMenuGraphics::HandleEnablePipelineMenuItem(bool is_enabled)
{
    emit EnablePipelineMenuItem(is_enabled);
}

void RgMenuGraphics::HandleEnableBuildSettingsMenuItem(bool is_enabled)
{
    emit EnableBuildSettingsMenuItem(is_enabled);
}

int RgMenuGraphics::GetButtonCount() const
{
    // There are 2 extra buttons besides the file items:
    // Build settings
    // Pipeline state
    static const int kExtraGraphicsButtonCount = 2;
    return kExtraGraphicsButtonCount;
}

void RgMenuGraphics::HandleDragAndDropExistingFile()
{
    SetButtonsNoFocus();
}

void RgMenuGraphics::SetButtonsNoFocus()
{
    assert(build_settings_menu_item_ != nullptr);
    assert(pipeline_state_item_ != nullptr);

    // Set the pipeline state and build settings buttons to have focus out style sheets.
    if (build_settings_menu_item_ != nullptr && pipeline_state_item_ != nullptr)
    {
        build_settings_menu_item_->SetCurrent(false);
        pipeline_state_item_->SetCurrent(false);
    }
}

RgMenuFileItemGraphics* RgMenuGraphics::GetStageItem(RgPipelineStage stage) const
{
    RgMenuFileItemGraphics* result_widget = nullptr;
    switch (stage)
    {
    case RgPipelineStage::kVertex:
    case RgPipelineStage::kTessellationControl:
    case RgPipelineStage::kTessellationEvaluation:
    case RgPipelineStage::kGeometry:
    case RgPipelineStage::kFragment:
        result_widget = shader_stage_items_[static_cast<size_t>(stage)];
        break;
    case RgPipelineStage::kCompute:
        {
            result_widget = shader_stage_items_[static_cast<size_t>(RgPipelineStage::kCompute)];
        }
        break;
    default:
        assert(false);
        break;
    }

    return result_widget;
}

void RgMenuGraphics::RebuildMenuItemsList()
{
    // Erase, and then reconstruct the menu item list in order by stage.
    menu_items_.clear();

    for (auto stage_item : shader_stage_items_)
    {
        if (stage_item != nullptr)
        {
            // If a stage item has a filename, add the item to the ordered list.
            if (!stage_item->GetFilename().empty())
            {
                menu_items_.push_back(stage_item);
            }
        }
    }
}

RgMenuPipelineStateItem* RgMenuGraphics::GetPipelineStateItem() const
{
    return pipeline_state_item_;
}

void RgMenuGraphics::HandleTabFocusPressed()
{
    // Remove focus from the sub buttons.
    RemoveSubButtonFocus();

    // Hide the remove file push button.
    HideRemoveFilePushButton();

    // Clear style sheets for all buttons.
    ClearButtonStylesheets();

    // Refresh focus.
    if (tab_focus_index_ < total_pipeline_stages_)
    {
        if ((has_tab_focus_ == GraphicsMenuTabFocusItems::kRemoveButton) ||
           (has_tab_focus_ == GraphicsMenuTabFocusItems::kAddExistingFileButton))
        {
            IncrementTabFocusIndex();

            // Reset the sub-button tab focus.
            has_tab_focus_ = GraphicsMenuTabFocusItems::kNoButton;
            if (tab_focus_index_ < total_pipeline_stages_)
            {
                RgMenuFileItemGraphics* item = GetCurrentFileMenuItem();
                if (item != nullptr)
                {
                    if (!item->GetFilename().empty())
                    {
                        SelectTabFocusItem(false);
                    }
                    else
                    {
                        item->SetButtonHighlighted(GraphicsMenuTabFocusItems::kAddExistingFileButton);
                        has_tab_focus_ = GraphicsMenuTabFocusItems::kAddExistingFileButton;
                    }
                }
            }
            else
            {
                // Figure out the correct button index.
                focus_index_ = tab_focus_index_;
                SelectTabFocusItem(false);
            }
        }
        else if (has_tab_focus_ == GraphicsMenuTabFocusItems::kNoButton)
        {
            // Select the next item in the file menu.
            SelectNextItem(FileMenuActionType::kTabAction);
        }
    }
    else
    {
        IncrementTabFocusIndex();

        // Select the next item in the file menu.
        SelectTabFocusItem(false);
    }
}

void RgMenuGraphics::SelectNextItem(FileMenuActionType action_type)
{
    // Figure out the correct button index.
    if (tab_focus_index_ < total_pipeline_stages_)
    {
        RgMenuFileItemGraphics* item = GetCurrentFileMenuItem();

        assert(item != nullptr);
        if (item != nullptr && !item->GetFilename().empty())
        {
            if (has_tab_focus_ == GraphicsMenuTabFocusItems::kRemoveButton ||
                has_tab_focus_ == GraphicsMenuTabFocusItems::kAddExistingFileButton)
            {
                SelectTabFocusItem(false);
                has_tab_focus_ = GraphicsMenuTabFocusItems::kNoButton;
            }
            else if (has_tab_focus_ == GraphicsMenuTabFocusItems::kNoButton)
            {
                item->SetButtonHighlighted(GraphicsMenuTabFocusItems::kRemoveButton);
                has_tab_focus_ = GraphicsMenuTabFocusItems::kRemoveButton;
            }
        }
        else
        {
            item->SetButtonHighlighted(GraphicsMenuTabFocusItems::kAddExistingFileButton);
            has_tab_focus_ = GraphicsMenuTabFocusItems::kAddExistingFileButton;
        }
    }
    else
    {
        SelectFocusItem(action_type);
    }
}

void RgMenuGraphics::SelectPreviousItem(FileMenuActionType action_type)
{
    // Figure out the correct button index.
    if (tab_focus_index_ < total_pipeline_stages_)
    {
        RgMenuFileItemGraphics* item = GetCurrentFileMenuItem();
        assert(item != nullptr);
        if (item != nullptr)
        {
            if (!item->GetFilename().empty())
            {
                if (has_tab_focus_ == GraphicsMenuTabFocusItems::kRemoveButton)
                {
                    SelectTabFocusItem(true);
                    has_tab_focus_ = GraphicsMenuTabFocusItems::kNoButton;
                }
                else
                {
                    item->SetButtonHighlighted(GraphicsMenuTabFocusItems::kRemoveButton);
                    has_tab_focus_ = GraphicsMenuTabFocusItems::kRemoveButton;
                }
            }
            else
            {
                item->SetButtonHighlighted(GraphicsMenuTabFocusItems::kAddExistingFileButton);
                has_tab_focus_ = GraphicsMenuTabFocusItems::kAddExistingFileButton;
            }
        }
    }
    else
    {
        SelectFocusItem(action_type);
    }
}

void RgMenuGraphics::IncrementTabFocusIndex()
{
    // Clear highlight for all file menu items.
    ClearFileMenuHighlight();

    // Advance the tab focus index.
    int offset = GetButtonCount() - 1;
    size_t end_index = total_pipeline_stages_ + offset + 1;

    // Increment focus index and wrap around.
    tab_focus_index_++;
    if (tab_focus_index_ >= end_index)
    {
        tab_focus_index_ = 0;
        focus_index_ = 0;

        // We've reached the end of file menu, so now
        // give focus to the next view.
        emit FocusNextView();
    }

    // Update the focus index.
    UpdateFocusIndex();

}

void RgMenuGraphics::DecrementTabFocusIndex()
{
    // Clear highlight for all file menu items.
    ClearFileMenuHighlight();

    // Decrement focus index and wrap around.
    if (tab_focus_index_ > 0)
    {
        tab_focus_index_--;
    }
    else
    {
        tab_focus_index_ = 0;

        // We've reached the beginning of file menu, so now
        // give focus to the previous view.
        emit FocusPrevView();
    }

    // Update the focus index.
    UpdateFocusIndex();
}

void RgMenuGraphics::UpdateFocusIndex()
{
    if (tab_focus_index_ < total_pipeline_stages_)
    {
        // Update focus_index_ only if tab_focus_index_ points to an occupied stage.
        RgMenuFileItemGraphics* item = GetCurrentFileMenuItem();
        assert(item != nullptr);
        if (item != nullptr)
        {
            int count = 0;
            for (const auto& graphics_item : menu_items_)
            {
                if (graphics_item == item)
                {
                    focus_index_ = count;
                    break;
                }
                count++;
            }
        }
    }
    else
    {
        // Update the focus index.
        focus_index_ = tab_focus_index_;
    }
}

void RgMenuGraphics::SelectFocusSubButton(RgMenuFileItemGraphics* item)
{
    assert(item != nullptr);

    if ((item != nullptr) && (!item->GetFilename().empty()))
    {
        if (has_tab_focus_ == GraphicsMenuTabFocusItems::kNoButton)
        {
            if (item != nullptr)
            {
                // Give focus to the remove file button.
                item->SetButtonHighlighted(GraphicsMenuTabFocusItems::kRemoveButton);
                has_tab_focus_ = GraphicsMenuTabFocusItems::kRemoveButton;
            }
        }
    }
    else
    {
        if (item != nullptr)
        {
            item->SetButtonHighlighted(GraphicsMenuTabFocusItems::kAddExistingFileButton);
            has_tab_focus_ = GraphicsMenuTabFocusItems::kAddExistingFileButton;
        }
    }
}

void RgMenuGraphics::HandleShiftTabFocusPressed()
{
    // Remove focus from the sub buttons.
    RemoveSubButtonFocus();

    // Clear style sheets for all buttons.
    ClearButtonStylesheets();

    // Hide the remove file push button.
    HideRemoveFilePushButton();

    // Refresh focus.
    if (tab_focus_index_ < total_pipeline_stages_)
    {
        if (has_tab_focus_ == GraphicsMenuTabFocusItems::kAddExistingFileButton)
        {
            DecrementTabFocusIndex();

            // Reset the sub-button tab focus.
            has_tab_focus_ = GraphicsMenuTabFocusItems::kNoButton;
            if (tab_focus_index_ < total_pipeline_stages_)
            {
                RgMenuFileItemGraphics* item = GetCurrentFileMenuItem();
                assert(item != nullptr);
                if (item != nullptr)
                {
                    if (!item->GetFilename().empty())
                    {
                        if (has_tab_focus_ == GraphicsMenuTabFocusItems::kNoButton)
                        {
                            UpdateHighlight(item);
                            item->SetButtonHighlighted(GraphicsMenuTabFocusItems::kRemoveButton);
                            has_tab_focus_ = GraphicsMenuTabFocusItems::kRemoveButton;
                        }
                        else if (has_tab_focus_ == GraphicsMenuTabFocusItems::kRemoveButton)
                        {
                            SelectTabFocusItem(true);
                            has_tab_focus_ = GraphicsMenuTabFocusItems::kNoButton;
                        }
                    }
                    else if (item->GetFilename().empty())
                    {
                        if (has_tab_focus_ == GraphicsMenuTabFocusItems::kNoButton)
                        {
                            item->SetButtonHighlighted(GraphicsMenuTabFocusItems::kAddExistingFileButton);
                            has_tab_focus_ = GraphicsMenuTabFocusItems::kAddExistingFileButton;
                        }
                        else
                        {
                            DecrementTabFocusIndex();
                            has_tab_focus_ = GraphicsMenuTabFocusItems::kNoButton;
                        }
                    }
                }
            }
            else
            {
                // Update the focus index.
                focus_index_ = tab_focus_index_;
                SelectTabFocusItem(true);
            }
        }
        else if (has_tab_focus_ == GraphicsMenuTabFocusItems::kRemoveButton)
        {
            SelectTabFocusItem(true);
            has_tab_focus_ = GraphicsMenuTabFocusItems::kNoButton;
        }
        else if (has_tab_focus_ == GraphicsMenuTabFocusItems::kNoButton)
        {
            DecrementTabFocusIndex();

            // Select the previous item in the file menu.
            SelectPreviousItem(FileMenuActionType::kTabAction);
        }
    }
    else
    {
        DecrementTabFocusIndex();

        // Select the previous item in the file menu.
        SelectTabFocusItem(true);
    }
}

void RgMenuGraphics::HandleBuildSettingsButtonClicked(bool /* checked */)
{
    // Clear the file menu item selection.
    ClearFileMenuItemSelection();

    // Clear the sub button highlights.
    RemoveSubButtonFocus();

    assert(build_settings_menu_item_ != nullptr);
    assert(pipeline_state_item_ != nullptr);
    if (build_settings_menu_item_ != nullptr && pipeline_state_item_ != nullptr)
    {
        // Set the focus index.
        focus_index_ = menu_items_.size() + GetButtonCount() - kPipelineStateButtonIndex;

        // Update button selection values.
        pipeline_state_item_->SetCurrent(false);
        build_settings_menu_item_->SetCurrent(true);
    }
}

void RgMenuGraphics::HandlePipelineStateButtonClicked(bool /* checked */)
{
    // Clear the file menu item selection.
    ClearFileMenuItemSelection();

    // Clear the sub button highlights.
    RemoveSubButtonFocus();

    assert(build_settings_menu_item_ != nullptr);
    assert(pipeline_state_item_ != nullptr);
    if (build_settings_menu_item_ != nullptr && pipeline_state_item_ != nullptr)
    {
        // Set the focus index.
        focus_index_ = menu_items_.size() + GetButtonCount() - kBuildSettingsButtonIndex;

        // Update button selection values.
        pipeline_state_item_->SetCurrent(true);
        build_settings_menu_item_->SetCurrent(false);
    }
}

void RgMenuGraphics::RemoveSubButtonFocus()
{
    // Remove sub button focus from all file items.
    for (RgMenuFileItemGraphics* item : shader_stage_items_)
    {
        // Do not assert here since item could be null
        // if this is compute pipeline item while in graphics pipeline,
        // and vice versa.
        if (item != nullptr)
        {
            item->RemoveSubButtonFocus();
        }
    }
}

void RgMenuGraphics::HideRemoveFilePushButton()
{
    // Hide the remove file push button for all file items.
    for (RgMenuFileItemGraphics* item : shader_stage_items_)
    {
        // Do not assert here since item could be null
        // if this is compute pipeline item while in graphics pipeline,
        // and vice versa.
        if (item != nullptr)
        {
            item->HideRemoveFilePushButton();
        }
    }
}

RgMenuFileItemGraphics* RgMenuGraphics::GetCurrentFileMenuItem() const
{
    RgMenuFileItemGraphics* item = nullptr;
    switch (pipeline_type_)
    {
    case (RgPipelineType::kGraphics):
    {
        if (tab_focus_index_ < kNumberOfGraphicsPipelineStages)
        {
            item = static_cast<RgMenuFileItemGraphics*>(shader_stage_items_[tab_focus_index_]);
        }
    }
    break;
    case (RgPipelineType::kCompute):
    {
        if (tab_focus_index_ < kNumberOfComputePipelineStages)
        {
            item = static_cast<RgMenuFileItemGraphics*>(shader_stage_items_[static_cast<int>(RgPipelineStage::kCompute)]);
        }
    }
    break;
    default:
        assert(false);
        break;
    }

    return item;
}

RgPipelineStage RgMenuGraphics::GetCurrentStage() const
{
    return current_stage_;
}

void RgMenuGraphics::HandleAddExistingFileButtonClicked(RgPipelineStage stage)
{
    // Remove focus from all sub buttons.
    RemoveSubButtonFocus();

    // Now add back the focus for the current stage's "Add existing file" button.
    RgMenuFileItemGraphics* stage_item = GetStageItem(stage);
    assert(stage_item != nullptr);
    if (stage_item != nullptr)
    {
        stage_item->SetButtonHighlighted(GraphicsMenuTabFocusItems::kAddExistingFileButton);
    }

    // Also update focus indexes.
    UpdateFocusIndex(stage);
}

void RgMenuGraphics::UpdateFocusIndex(RgPipelineStage stage)
{
    if (pipeline_type_ == RgPipelineType::kCompute)
    {
        focus_index_ = 0;
        tab_focus_index_ = 0;
    }
    else if (pipeline_type_ == RgPipelineType::kGraphics)
    {
        focus_index_ = static_cast<int>(stage);
        tab_focus_index_ = static_cast<int>(stage);
    }
    else
    {
        // Should not be here.
        assert(false);
    }
}

size_t RgMenuGraphics::GetTotalPipelineStages() const
{
    size_t total_pipeline_stages = 0;

    if (pipeline_type_ == RgPipelineType::kCompute)
    {
        total_pipeline_stages = kNumberOfComputePipelineStages;
    }
    else if (pipeline_type_ == RgPipelineType::kGraphics)
    {
        total_pipeline_stages = kNumberOfGraphicsPipelineStages;
    }
    else
    {
        // Should not be here.
        assert(false);
    }

    return total_pipeline_stages;
}

size_t RgMenuGraphics::GetNumberPipelineStagesToCycleThrough(FileMenuActionType action_type) const
{
    size_t total_occupied_pipeline_stages = 0;

    switch (action_type)
    {
    case FileMenuActionType::kArrowAction:
    case FileMenuActionType::kTabAction:
    {
        if (pipeline_type_ == RgPipelineType::kCompute)
        {
            total_occupied_pipeline_stages = kNumberOfComputePipelineStages;
        }
        else if (pipeline_type_ == RgPipelineType::kGraphics)
        {
            total_occupied_pipeline_stages = kNumberOfGraphicsPipelineStages;
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

    return total_occupied_pipeline_stages;
}

std::vector<RgMenuFileItem*> RgMenuGraphics::GetFileMenuItemsToProcess(FileMenuActionType action_type) const
{
    std::vector<RgMenuFileItem*> vector_to_process;

    switch (action_type)
    {
    case FileMenuActionType::kArrowAction:
    case FileMenuActionType::kTabAction:
    {
        std::vector<RgMenuFileItem*> vectorToProcess(std::begin(shader_stage_items_), std::end(shader_stage_items_));
        break;
    }
    default:
        // We shouldn't get here.
        assert(false);
        break;
    }

    return vector_to_process;
}

void RgMenuGraphics::HandleRemoveFileMenuButtonFocus()
{
    RemoveSubButtonFocus();
    RemoveFileMenuButtonFocus();
}

void RgMenuGraphics::SetCurrent(RgPipelineStage stage, bool is_current)
{
    RgMenuFileItemGraphics* stage_ptr = GetStageItem(stage);
    if (stage_ptr != nullptr)
    {
        stage_ptr->SetCurrent(is_current);
        stage_ptr->SetHovered(is_current);
    }
}

void RgMenuGraphics::RemoveFileMenuButtonFocus()
{
    assert(build_settings_menu_item_ != nullptr);
    assert(pipeline_state_item_ != nullptr);
    if (build_settings_menu_item_ != nullptr && pipeline_state_item_ != nullptr)
    {
        // Set the pipeline state and build settings buttons to have focus out style sheets.
        // Verify that the button isn't currently selected before removing the focus.
        if (!build_settings_menu_item_->IsCurrent())
        {
            build_settings_menu_item_->GetBuildSettingsButton()->setStyleSheet(kStrButtonFocusOutStylesheet);
            build_settings_menu_item_->GetBuildSettingsButton()->setCursor(Qt::PointingHandCursor);
        }

        if (!pipeline_state_item_->IsCurrent())
        {
            pipeline_state_item_->GetPipelineStateButton()->setStyleSheet(kStrButtonFocusOutStylesheet);
            pipeline_state_item_->GetPipelineStateButton()->setCursor(Qt::PointingHandCursor);
        }
    }
}

void RgMenuGraphics::mousePressEvent(QMouseEvent* event)
{
    Q_UNUSED(event);

    emit MenuClicked();
    emit FileMenuFocusInEvent();
}

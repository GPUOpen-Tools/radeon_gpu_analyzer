// C++.
#include <cassert>
#include <sstream>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_menu_build_settings_item.h"
#include "radeon_gpu_analyzer_gui/qt/rg_menu_vulkan.h"
#include "radeon_gpu_analyzer_gui/qt/rg_menu_file_item_graphics.h"
#include "radeon_gpu_analyzer_gui/qt/rg_menu_pipeline_state_item.h"
#include "radeon_gpu_analyzer_gui/rg_string_constants.h"
#include "radeon_gpu_analyzer_gui/rg_utils.h"

RgMenuVulkan::RgMenuVulkan(QWidget* parent)
    : RgMenuGraphics(parent)
{
}

void RgMenuVulkan::ConnectDefaultItemSignals()
{
    // Handler invoked when the "Build settings" button is clicked.
    assert(build_settings_menu_item_ != nullptr);
    if (build_settings_menu_item_ != nullptr)
    {
        bool is_connected = connect(build_settings_menu_item_->GetBuildSettingsButton(), &QPushButton::clicked, this, &RgMenu::HandleBuildSettingsButtonClicked);
        assert(is_connected);
    }
}

void RgMenuVulkan::ConnectMenuFileItemSignals(RgMenuFileItem* menu_item)
{
    // Connect the file menu item selection handler for each new item.
    bool is_connected = connect(menu_item, &RgMenuFileItemGraphics::MenuItemSelected, this, &RgMenu::MenuItemClicked);
    assert(is_connected);

    // Connect the file menu item rename handler for each new item.
    is_connected = connect(menu_item, &RgMenuFileItem::FileRenamed, this, &RgMenu::HandleRenamedFile);
    assert(is_connected);
}

void RgMenuVulkan::DeselectItems()
{
    RgMenu::DeselectItems();

    assert(pipeline_state_item_);
    if (pipeline_state_item_ != nullptr)
    {
        pipeline_state_item_->SetCurrent(false);
    }
}

void RgMenuVulkan::ClearStageSourceFile(RgPipelineStage stage)
{
    // Find the target stage item and update the attached shader file.
    RgMenuFileItemGraphics* stage_item = GetStageItem(stage);

    assert(stage_item != nullptr);
    if (stage_item != nullptr)
    {
        // Erase the file path from the path to menu item map.
        const std::string& path_being_removed = stage_item->GetFilename();
        auto file_path_to_item_map_iter = full_file_path_to_menu_item_.find(path_being_removed);
        if (file_path_to_item_map_iter != full_file_path_to_menu_item_.end())
        {
            full_file_path_to_menu_item_.erase(file_path_to_item_map_iter);
        }

        // Clear out the filepath for the stage item.
        stage_item->SetShaderFile("", RgVulkanInputType::kUnknown);

        // Set the "occupied" property for this file item.
        stage_item->SetStageIsOccupied(false);

        // Set the "current" property for this file item.
        stage_item->SetCurrent(false);

        // Set the cursor type to arrow cursor.
        stage_item->SetCursor(Qt::ArrowCursor);

        // Set the "+" sub button as selected.
        RemoveSubButtonFocus();
        HideRemoveFilePushButton();
        stage_item->SetButtonHighlighted(GraphicsMenuTabFocusItems::kAddExistingFileButton);
        has_tab_focus_ = GraphicsMenuTabFocusItems::kAddExistingFileButton;

        // Also set the tab focus index and focus index.
        // Find out if one of the buttons is currently selected.
        // If so, do not update the focus index.
        if (!IsButtonPressed())
        {
            if (stage == RgPipelineStage::kCompute)
            {
                focus_index_ = 0;
            }
            else
            {
                focus_index_ = static_cast<int>(stage);
            }
            tab_focus_index_ = focus_index_;
        }

        // Remove the item from the list.
        for (auto it = menu_items_.begin(); it != menu_items_.end(); it++)
        {
            if (*it == stage_item)
            {
                menu_items_.erase(it);
                break;
            }
        }
    }

    // Emit a signal that indicates that the number of items in the menu change,
    // and specify that the menu is not empty.
    emit FileMenuItemCountChanged(false);
}

bool RgMenuVulkan::IsButtonPressed() const
{
    bool button_pressed = false;

    assert(build_settings_menu_item_ != nullptr);
    assert(pipeline_state_item_ != nullptr);
    if (build_settings_menu_item_ != nullptr && pipeline_state_item_ != nullptr)
    {
        button_pressed = build_settings_menu_item_->IsCurrent() || pipeline_state_item_->IsCurrent();
    }

    return button_pressed;
}

bool RgMenuVulkan::SetStageSourceFile(RgPipelineStage stage, const std::string& full_path, RgVulkanInputType file_type, bool is_new_file_item)
{
    bool ret = false;

    // If a file with that name hasn't been added.
    if (!IsFileInMenu(full_path))
    {
        // Find the target stage item and update the attached shader file.
        RgMenuFileItemGraphics* stage_item = GetStageItem(stage);

        assert(stage_item != nullptr);
        if (stage_item != nullptr)
        {
            stage_item->SetShaderFile(full_path, file_type);

            // Since users may add shader files to the pipeline stages in any order they choose, in
            // order to use the Up/Down arrow keys to navigate through shader stage items, a linear
            // list of menu items must be reconstructed.
            RebuildMenuItemsList();

            // Set the "occupied" property for this file item.
            stage_item->SetStageIsOccupied(true);

            // Set the cursor for this item.
            UpdateCursor(stage_item);

            // Hide the "Remove" button.
            stage_item->RemoveSubButtonFocus();
            stage_item->HideRemoveFilePushButton();

            ret = true;
        }

        // Emit a signal that indicates that the number of items in the menu change,
        // and specify that the menu is not empty.
        emit FileMenuItemCountChanged(false);

        // Add the file name and its full path to the map.
        full_file_path_to_menu_item_[full_path] = stage_item;

        // Connect signals for the new file menu item.
        ConnectMenuFileItemSignals(stage_item);

        // Select the file that was just opened.
        HandleSelectedFileChanged(stage_item);

        if (is_new_file_item)
        {
            // If a file was newly-created (as opposed to being opened), display the
            // item's renaming control immediately so the user can choose a new filename.
            stage_item->ShowRenameControls(true);
        }

    }
    else
    {
        // Report the error.
        std::stringstream msg;
        msg << kStrErrCannotAddFileA << full_path << kStrErrCannotAddFileB;
        RgUtils::ShowErrorMessageBox(msg.str().c_str(), this);

        // Remove the highlight.
        RgMenuFileItemGraphics* stage_item = GetStageItem(stage);

        assert(stage_item != nullptr);
        if (stage_item != nullptr)
        {
            stage_item->SetCurrent(false);
            stage_item->SetHovered(false);
        }

        // Set the highlight for the current stage.
        stage_item = GetCurrentFileMenuItem();
        assert(stage_item != nullptr);
        if (stage_item != nullptr)
        {
            stage_item->SetCurrent(true);
            stage_item->SetHovered(true);
        }
    }

    return ret;
}

bool RgMenuVulkan::ReplaceStageFile(RgPipelineStage stage, const std::string& new_file_path, RgVulkanInputType file_type)
{
    bool ret = false;
    RgMenuFileItemGraphics* file_item = GetStageItem(stage);
    assert(file_item != nullptr);
    if (file_item != nullptr)
    {
        // Replace the file path in the file to item map.
        const std::string& old_file_path = file_item->GetFilename();
        auto it = full_file_path_to_menu_item_.find(old_file_path);
        if (it != full_file_path_to_menu_item_.end())
        {
            assert(it->second == file_item);
            if (it->second == file_item)
            {
                full_file_path_to_menu_item_.erase(it);
                full_file_path_to_menu_item_[new_file_path] = file_item;
                ret = true;
            }
        }

        // Replace the file path in the item.
        file_item->SetShaderFile(new_file_path, file_type);
    }

    return ret;
}

void RgMenuVulkan::HandleActivateItemAction()
{
    // If focus index is in the range of the menu file items, select the appropriate file item.
    const size_t total_stages = GetTotalPipelineStages();
    if (pipeline_type_ == RgPipelineType::kGraphics && tab_focus_index_ < total_stages ||
        pipeline_type_ == RgPipelineType::kCompute && tab_focus_index_ <= total_stages)
    {
        // See if a sub button is selected,
        // and then execute accordingly.
        if (has_tab_focus_ != GraphicsMenuTabFocusItems::kNoButton)
        {
            // If the user hit enter when one of the sub-buttons were highlighted,
            // process accordingly.
            ProcessSubButtonAction();
        }
        else if (focus_index_ < menu_items_.size())
        {
            SelectFile(menu_items_[focus_index_]);
            build_settings_menu_item_->SetCurrent(false);
            pipeline_state_item_->SetCurrent(false);
        }
        else
        {
            // Local index: this is the index of the relevant
            // item within the menu items after filtering out
            // the file items (leaving only the other buttons:
            // pipeline state and build settings).
            const int kPipelineStateItemLocalIndex = 0;
            const int kBuildSettingsItemLocalIndex = 1;

            // Calculate the index of the relevant item.
            const int button_count = GetButtonCount();
            const size_t num_of_file_items = menu_items_.size();
            size_t target_item_index = static_cast<size_t>(tab_focus_index_) - num_of_file_items;
            assert(target_item_index == 0 || target_item_index == 1);
            if (target_item_index == kPipelineStateItemLocalIndex)
            {
                // This is the pipeline state button - simulate a click.
                pipeline_state_item_->ClickMenuItem();
            }
            else if(target_item_index == kBuildSettingsItemLocalIndex)
            {
                // This is the build settings button - simulate a click.
                build_settings_menu_item_->ClickMenuItem();
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
        for (RgMenuFileItem* item : menu_items_)
        {
            RgMenuFileItemGraphics* item_graphics = static_cast<RgMenuFileItemGraphics*>(item);
            assert(item_graphics != nullptr);
            if (item_graphics != nullptr)
            {
                item_graphics->SetHovered(false);
                item_graphics->SetCurrent(false);
            }
        }

        // If out of range, special case handle the last focus items.
        // Get index excluding file items.
        size_t index = focus_index_ - total_stages;

        // Handle special cases for pipeline state and build settings item.
        switch (index)
        {
        case static_cast<size_t>(GraphicsMenuFocusItems::kBuildSettingsButton) :
            build_settings_menu_item_->GetBuildSettingsButton()->setStyleSheet(kStrButtonFocusInStylesheet);
            build_settings_menu_item_->GetBuildSettingsButton()->click();
            break;
        case static_cast<size_t>(GraphicsMenuFocusItems::kPipelineStateButton) :
            GetPipelineStateItem()->GetPipelineStateButton()->setStyleSheet(kStrButtonFocusInStylesheet);
            GetPipelineStateItem()->GetPipelineStateButton()->click();
            break;
        default:
            // Should never get here.
            assert(false);
            break;
        }
    }
}

void RgMenuVulkan::ProcessSubButtonAction()
{
    const size_t total_stages = GetTotalPipelineStages();
    assert(tab_focus_index_ < total_stages);
    if (tab_focus_index_ < total_stages)
    {
        RgMenuFileItemGraphics* item = GetCurrentFileMenuItem();
        assert(item != nullptr);
        if (item != nullptr)
        {
            switch (has_tab_focus_)
            {
            case (GraphicsMenuTabFocusItems::kRemoveButton):
                item->ProcessRemoveButtonClick();
                break;
            case (GraphicsMenuTabFocusItems::kAddExistingFileButton):
                item->ProcessAddExistingFileButtonClick();
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

void RgMenuVulkan::HandleSelectedFileChanged(RgMenuFileItem* selected)
{
    RgMenuFileItemGraphics* graphics_item = qobject_cast<RgMenuFileItemGraphics*>(selected);
    assert(graphics_item != nullptr);

    // Set focus index to newly selected item.
    for (size_t i = 0; i < menu_items_.size() && graphics_item != nullptr; i++)
    {
        if (menu_items_[i] == selected)
        {
            // Update the focus and tab indices. Compute indices are always
            // 0 because there's only 1 item in the compute pipeline menu.
            RgPipelineStage stage_type = graphics_item->GetStage();
            if (stage_type != RgPipelineStage::kCompute)
            {
                focus_index_ = static_cast<size_t>(stage_type);
                tab_focus_index_ = static_cast<size_t>(stage_type);
            }
            else
            {
                focus_index_ = 0;
                tab_focus_index_ = 0;
            }
            break;
        }
    }

    // Update the current stage value.
    if (graphics_item != nullptr)
    {
        current_stage_ = graphics_item->GetStage();
    }

    // Display the currently selected file in the source editor.
    DisplayFileInEditor(selected);

    // Update button selection values.
    pipeline_state_item_->SetCurrent(false);
    build_settings_menu_item_->SetCurrent(false);

    // Set the pipeline state and build settings buttons to have focus out style sheets.
    assert(build_settings_menu_item_);
    assert(pipeline_state_item_);
    if (build_settings_menu_item_ != nullptr && pipeline_state_item_ != nullptr)
    {
        build_settings_menu_item_->GetBuildSettingsButton()->setStyleSheet(kStrButtonFocusOutStylesheet);
        build_settings_menu_item_->GetBuildSettingsButton()->setCursor(Qt::PointingHandCursor);

        pipeline_state_item_->GetPipelineStateButton()->setStyleSheet(kStrButtonFocusOutStylesheet);
        pipeline_state_item_->GetPipelineStateButton()->setCursor(Qt::PointingHandCursor);
    }
}

void RgMenuVulkan::UpdateFocusIndex()
{
    focus_index_ = static_cast<size_t>(current_stage_);
    tab_focus_index_ = static_cast<size_t>(current_stage_);
}

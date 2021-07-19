// C++.
#include <cassert>
#include <sstream>

// Qt.
#include <QBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QKeyEvent>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_menu.h"
#include "radeon_gpu_analyzer_gui/qt/rg_add_create_menu_item.h"
#include "radeon_gpu_analyzer_gui/qt/rg_build_view.h"
#include "radeon_gpu_analyzer_gui/qt/rg_menu_build_settings_item.h"
#include "radeon_gpu_analyzer_gui/qt/rg_menu_file_item_graphics.h"
#include "radeon_gpu_analyzer_gui/qt/rg_menu_file_item_opencl.h"
#include "radeon_gpu_analyzer_gui/qt/rg_menu_item.h"
#include "radeon_gpu_analyzer_gui/qt/rg_menu_titlebar.h"
#include "radeon_gpu_analyzer_gui/qt/rg_handle_tab_focus_event_filter.h"
#include "radeon_gpu_analyzer_gui/rg_definitions.h"
#include "radeon_gpu_analyzer_gui/rg_string_constants.h"
#include "radeon_gpu_analyzer_gui/rg_utils.h"
#include "QtCommon/Scaling/ScalingManager.h"

RgMenu::RgMenu(QWidget* parent) :
    QFrame(parent),
    focus_index_(0),
    tab_focus_index_(0)
{
    // Initialize the view object.
    ui_.setupUi(this);

    // Initialize the menu layout, which all file items will be added to.
    layout_ = ui_.fileMenuVerticalLayout;

    // Create file menu actions.
    CreateActions();

    // Install filter to ignore tab input for switching focus.
    installEventFilter(&RgHandleTabFocusEventFilter::Get());
}

void RgMenu::DeselectItems()
{
    DeselectCurrentFile();
    auto build_settings_item = GetBuildSettingsItem();
    assert(build_settings_item != nullptr);
    if (build_settings_item != nullptr)
    {
        build_settings_item->SetCurrent(false);
        if (build_settings_item->GetBuildSettingsButton() != nullptr)
        {
            build_settings_item->GetBuildSettingsButton()->setStyleSheet(kStrButtonFocusOutStylesheet);
        }
    }
}

void RgMenu::CreateActions()
{
    // File menu next item.
    next_item_action_ = new QAction(this);
    next_item_action_->setShortcut(QKeySequence(kActionHotkeyFileMenuNextItem));
    next_item_action_->setShortcutContext(Qt::WidgetShortcut);

    addAction(next_item_action_);
    bool is_connected = connect(next_item_action_, &QAction::triggered, this, &RgMenu::HandleTabFocusPressed);
    assert(is_connected);

    // File menu previous item.
    prev_item_action_ = new QAction(this);
    prev_item_action_->setShortcut(QKeySequence(kActionHotkeyFileMenuPrevItem));
    prev_item_action_->setShortcutContext(Qt::WidgetShortcut);

    addAction(prev_item_action_);
    is_connected = connect(prev_item_action_, &QAction::triggered, this, &RgMenu::HandleShiftTabFocusPressed);
    assert(is_connected);

    // File menu context menu shortcut.
    open_context_menu_action_ = new QAction(this);
    open_context_menu_action_->setShortcut(QKeySequence(kActionHotkeyFileMenuContextMenu));
    open_context_menu_action_->setShortcutContext(Qt::WidgetShortcut);

    addAction(open_context_menu_action_);
    is_connected = connect(open_context_menu_action_, &QAction::triggered, this, &RgMenu::HandleOpenContextMenuAction);
    assert(is_connected);

    // File menu activate item (used to trigger add/create buttons when in focus).
    activate_item_action_ = new QAction(this);
    QList<QKeySequence> shortcut_actions;
    shortcut_actions.push_back(QKeySequence(kActionHotkeyFileMenuActivateReturn));
    shortcut_actions.push_back(QKeySequence(kActionHotkeyFileMenuActivateEnter));
    shortcut_actions.push_back(QKeySequence(kActionHotkeyFileMenuActivateSpace));
    activate_item_action_->setShortcuts(shortcut_actions);
    activate_item_action_->setShortcutContext(Qt::WidgetShortcut);

    addAction(activate_item_action_);
    is_connected = connect(activate_item_action_, &QAction::triggered, this, &RgMenu::HandleActivateItemAction);
    assert(is_connected);

    // File menu rename selected file.
    rename_selected_file_action_ = new QAction(this);
    rename_selected_file_action_->setShortcut(QKeySequence(kActionHotkeyFileMenuRename));
    rename_selected_file_action_->setShortcutContext(Qt::WidgetShortcut);

    addAction(rename_selected_file_action_);
    is_connected = connect(rename_selected_file_action_, &QAction::triggered, this, &RgMenu::HandleRenameSelectedFileAction);
    assert(is_connected);

    // File menu tab key navigation.
    tab_key_action_ = new QAction(this);
    tab_key_action_->setShortcut(QKeySequence(kActionHotkeyFileMenuActivateTab));
    tab_key_action_->setShortcutContext(Qt::WidgetShortcut);

    addAction(tab_key_action_);
    is_connected = connect(tab_key_action_, &QAction::triggered, this, &RgMenu::HandleTabFocusPressed);
    assert(is_connected);

    // File menu shift+tab key navigation.
    shift_tab_key_action_ = new QAction(this);
    shift_tab_key_action_->setShortcut(QKeySequence(kActionHotkeyFileMenuActivateShiftTab));
    shift_tab_key_action_->setShortcutContext(Qt::WidgetShortcut);

    addAction(shift_tab_key_action_);
    is_connected = connect(shift_tab_key_action_, &QAction::triggered, this, &RgMenu::HandleShiftTabFocusPressed);
    assert(is_connected);
}

void RgMenu::ClearFiles()
{
    // Deselect any currently selected file, because it's about to be removed.
    DeselectCurrentFile();

    // Collect a list of keys for each file path with an item in the file menu.
    std::vector<std::string> open_files;
    for (auto current_item = full_file_path_to_menu_item_.begin(); current_item != full_file_path_to_menu_item_.end(); ++current_item)
    {
        open_files.push_back(current_item->first);
    }

    // Step through each path and remove the item from the file menu.
    for (const std::string& current_file_path : open_files)
    {
        RemoveItem(current_file_path);
    }

    // Reset the build settings item to reflect no pending changes.
    if (build_settings_menu_item_ != nullptr)
    {
        build_settings_menu_item_->SetHasPendingChanges(false);
    }
}

RgMenuBuildSettingsItem* RgMenu::GetBuildSettingsItem() const
{
    return build_settings_menu_item_;
}

RgMenuFileItem* RgMenu::GetFileItemFromPath(const std::string& source_file_path) const
{
    RgMenuFileItem* result_item = nullptr;

    // Step through each file item and find the one that matches the given path.
    for (size_t file_item_index = 0; file_item_index < menu_items_.size(); file_item_index++)
    {
        if (menu_items_[file_item_index]->GetFilename() == source_file_path)
        {
            result_item = menu_items_[file_item_index];
            break;
        }
    }

    return result_item;
}

RgMenuFileItem* RgMenu::GetSelectedFileItem() const
{
    return selected_file_item_;
}

std::string RgMenu::GetSelectedFilePath() const
{
    std::string selected_file_path;
    if (selected_file_item_ != nullptr)
    {
        selected_file_path = selected_file_item_->GetFilename();
    }
    return selected_file_path;
}

std::vector<RgMenuFileItem*> RgMenu::GetAllFileItems() const
{
    return menu_items_;
}

bool RgMenu::IsCurrentlySelectedFileItem(RgMenuFileItem* file_item) const
{
    return (file_item == selected_file_item_);
}

bool RgMenu::IsEmpty() const
{
    return full_file_path_to_menu_item_.empty();
}

bool RgMenu::IsFileInMenu(const std::string& full_path) const
{
    bool ret = false;

    // Standardize the path of the given full path.
    std::string full_path_standardized = full_path;
    RgUtils::StandardizePathSeparator(full_path_standardized);

    // Check if the full path already has an item in the menu.
    for (const RgMenuFileItem* item : menu_items_)
    {
        assert(item != nullptr);
        if (item != nullptr)
        {
            // Retrieve and standardize the file name for this item.
            std::string item_full_path = item->GetFilename();
            RgUtils::StandardizePathSeparator(item_full_path);

            // If the paths match - we found it.
            if (item_full_path.compare(full_path_standardized) == 0)
            {
                ret = true;
                break;
            }
        }
    }
    return ret;
}

void RgMenu::RemoveItem(const std::string& full_filename)
{
    if (!full_filename.empty())
    {
        // Remove the item from the map.
        auto filepath_iter = full_file_path_to_menu_item_.find(full_filename);
        if (filepath_iter != full_file_path_to_menu_item_.end())
        {
            RgMenuFileItem* men_item = filepath_iter->second;

            full_file_path_to_menu_item_.erase(filepath_iter);

            // Remove the item from the list.
            for (auto it = menu_items_.begin(); it != menu_items_.end(); it++)
            {
                if (*it == men_item)
                {
                    menu_items_.erase(it);
                    break;
                }
            }

            if (menu_items_.size() == 0)
            {
                // The file menu is empty.
                emit FileMenuItemCountChanged(true);
            }

            // Remove the item from the GUI.
            layout_->removeWidget(men_item);

            // The widget was removed from the layout, but still exists and will be rendered.
            // It is therefore also necessary to hide the widget to make it disappear entirely.
            men_item->hide();

            // Clear the "last selected item" variable if it's the item we are removing.
            if (last_selected_item_ == men_item)
            {
                last_selected_item_ = nullptr;
            }
        }
    }
}

void RgMenu::DeselectCurrentFile()
{
    // If a file was already selected, deselect it.
    if (selected_file_item_ != nullptr)
    {
        selected_file_item_->SetHovered(false);
        selected_file_item_->SetCurrent(false);
        selected_file_item_->setCursor(Qt::PointingHandCursor);
        selected_file_item_ = nullptr;
    }
}

void RgMenu::SetItemIsSaved(const std::string& fullFilename, bool is_saved)
{
    // Find the item in the map.
    auto filepath_iter = full_file_path_to_menu_item_.find(fullFilename);
    if (filepath_iter != full_file_path_to_menu_item_.end())
    {
        RgMenuFileItem* menu_item = filepath_iter->second;
        bool is_item_valid = (menu_item != nullptr);
        assert(is_item_valid);

        if (is_item_valid)
        {
            // Set item saved state.
            menu_item->SetIsSaved(is_saved);
        }
    }
}

void RgMenu::SelectFirstItem()
{
    auto first_item_iter = menu_items_.cbegin();
    if (first_item_iter != menu_items_.cend() && *first_item_iter != nullptr)
    {
        HandleSelectedFileChanged(static_cast<RgMenuFileItem*>(*first_item_iter));
    }
}

void RgMenu::SelectLastRemainingItem()
{
    auto last_item_iter = menu_items_.crbegin();
    if (last_item_iter != menu_items_.crend() && *last_item_iter != nullptr)
    {
        HandleSelectedFileChanged(static_cast<RgMenuFileItem*>(*last_item_iter));
    }
}

void RgMenu::SwitchToLastSelectedItem()
{
    if (last_selected_item_ != nullptr)
    {
        HandleSelectedFileChanged(last_selected_item_);
    }
}

void RgMenu::HandleBuildStarted()
{
    build_settings_menu_item_->setEnabled(false);
}

void RgMenu::HandleBuildEnded()
{
    build_settings_menu_item_->setEnabled(true);
}

void RgMenu::HandleSwitchToFile(const std::string & file_path, int line_num)
{
    // Set focus index to newly selected item.
    bool  found = false;
    for (size_t i = 0; i < menu_items_.size(); i++)
    {
        if (menu_items_[i]->GetFilename() == file_path)
        {
            found = true;
            focus_index_ = i;
            tab_focus_index_ = i;
            break;
        }
    }

    if (found)
    {
        // Refresh the focus selection so it selects the current focus index.
        SelectFocusItem(FileMenuActionType::kTabAction);

        // Scroll the source code editor to required line.
        emit ScrollCodeEditorToLine(line_num);
    }
}

void RgMenu::HandleRenamedFile(const std::string& old_file_path, const std::string& new_file_path)
{
    auto menu_item_iter = full_file_path_to_menu_item_.find(old_file_path);
    bool found_old_filepath = menu_item_iter != full_file_path_to_menu_item_.end();

    // If the old path was found, remove it.
    assert(found_old_filepath);
    if (found_old_filepath)
    {
        RgMenuFileItem* menu_item = menu_item_iter->second;

        // Erase the existing file path, and make the new path point to the existing menu item.
        full_file_path_to_menu_item_.erase(menu_item_iter);
        full_file_path_to_menu_item_[new_file_path] = menu_item;
    }

    // Signal to the BuildView that a file has been renamed.
    emit FileRenamed(old_file_path, new_file_path);
}

void RgMenu::SelectFile(RgMenuFileItem* selected)
{
    if (selected != nullptr)
    {
        std::string old_filename;
        if (selected_file_item_ != nullptr)
        {
            selected_file_item_->SetHovered(false);
            old_filename = selected_file_item_->GetFilename();
        }

        // Assign the new selection as the currently-selected file item.
        selected_file_item_ = selected;
        last_selected_item_ = selected;

        // Set the file item as the new selection, and expand the entry point list.
        selected_file_item_->SetHovered(true);

        // Update the cursor type.
        UpdateCursor(selected_file_item_);

        // Set the file item as the new highlighted item.
        UpdateHighlight(selected);

        // Set the file item as the current item.
        UpdateCurrentItem(selected);

        const std::string& new_file_name = selected->GetFilename();
        StringToFileItemMap::iterator full_filename_iter = full_file_path_to_menu_item_.find(new_file_name);
        if (full_filename_iter != full_file_path_to_menu_item_.end())
        {
            emit SelectedFileChanged(old_filename, new_file_name);
        }
    }
}

void RgMenu::DisplayFileInEditor(RgMenuFileItem* selected)
{
    std::string old_filename;
    if (selected_file_item_ != nullptr)
    {
        selected_file_item_->SetHovered(false);
        old_filename = selected_file_item_->GetFilename();
    }

    const std::string& new_file_name = selected->GetFilename();
    StringToFileItemMap::iterator full_filename_iter = full_file_path_to_menu_item_.find(new_file_name);
    if (full_filename_iter != full_file_path_to_menu_item_.end())
    {
        // Set the file item as the new highlighted item.
        UpdateHighlight(selected);

        // Set this item as the current one.
        UpdateCurrentItem(selected);

        // Update the cursors.
        UpdateCursor(selected);

        // Assign the new selection as the currently-selected file item.
        selected_file_item_ = selected;
        last_selected_item_ = selected;

        emit SelectedFileChanged(old_filename, new_file_name);
    }
}

void RgMenu::HandleNextItemAction()
{
    // Compute the index of the next button to focus on. Take the extra non-file items into account.
    int offset = GetButtonCount() - 1;
    size_t end_index = menu_items_.size() + offset;

    // Increment focus index and wrap around.
    focus_index_++;
    if (focus_index_ > end_index)
    {
        focus_index_ = 0;
    }

    // Update the tab focus index.
    tab_focus_index_ = focus_index_;

    // Refresh focus.
    SelectFocusItem(FileMenuActionType::kArrowAction);
}

void RgMenu::HandlePreviousItemAction()
{
    // Compute the index of the next button to focus on. Take the extra non-file items into account.
    int offset = GetButtonCount() - 1;
    size_t end_index = menu_items_.size() + offset;

    // Decrement focus index and wrap around.
    if (focus_index_ != 0)
    {
        focus_index_--;
    }
    else
    {
        focus_index_ = end_index;
    }

    // Update the tab focus index.
    tab_focus_index_ = focus_index_;

    // Refresh focus.
    SelectFocusItem(FileMenuActionType::kArrowAction);
}

void RgMenu::HandleOpenContextMenuAction()
{
    if (selected_file_item_ != nullptr)
    {
        selected_file_item_->OpenContextMenu();
    }
}

void RgMenu::HandleRenameSelectedFileAction()
{
    if (selected_file_item_ != nullptr)
    {
        selected_file_item_->ShowRenameControls(true);
    }
}

void RgMenu::UpdateCursor(RgMenuFileItem* current_stage_item)
{
    // Reset cursor to pointing hand cursor for all items.
    foreach(auto item, menu_items_)
    {
        item->setCursor(Qt::PointingHandCursor);
    }

    // Set the cursor to arrow cursor for the selected item.
    current_stage_item->setCursor(Qt::ArrowCursor);
}

void RgMenu::UpdateHighlight(RgMenuFileItem* selected)
{
    // Reset the highlight for all items.
    foreach(auto item, menu_items_)
    {
        item->SetHovered(false);
    }

    selected->SetHovered(true);
}

void RgMenu::mousePressEvent(QMouseEvent* event)
{
    emit FileMenuFocusInEvent();

    QFrame::mousePressEvent(event);
}

void RgMenu::UpdateCurrentItem(RgMenuFileItem* selected)
{
    for (auto item : menu_items_)
    {
        item->SetCurrent(false);
    }

    selected->SetCurrent(true);
}

void RgMenu::ClearFileMenuHighlight()
{
    for (auto item : menu_items_)
    {
        item->SetHovered(false);
    }
}

void RgMenu::ClearFileMenuItemSelection()
{
    // Set the stylesheet for each file menu item to be not selected (grayed out),
    // along with the mouse cursor to pointing hand cursor.
    for (RgMenuFileItem* item : menu_items_)
    {
        item->SetHovered(false);
        item->SetCurrent(false);
        item->setCursor(Qt::PointingHandCursor);
    }
}

// C++.
#include <cassert>
#include <sstream>

// Qt.
#include <QWidget>
#include <QBoxLayout>
#include <QMimeData>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_menu_binary.h"
#include "radeon_gpu_analyzer_gui/qt/rg_link_source_menu_item.h"
#include "radeon_gpu_analyzer_gui/qt/rg_build_view.h"
#include "radeon_gpu_analyzer_gui/qt/rg_menu_build_settings_item.h"
#include "radeon_gpu_analyzer_gui/qt/rg_menu_file_item_opencl.h"
#include "radeon_gpu_analyzer_gui/qt/rg_menu_item.h"
#include "radeon_gpu_analyzer_gui/qt/rg_menu_titlebar.h"
#include "radeon_gpu_analyzer_gui/qt/rg_handle_tab_focus_event_filter.h"
#include "radeon_gpu_analyzer_gui/rg_definitions.h"
#include "radeon_gpu_analyzer_gui/rg_string_constants.h"
#include "radeon_gpu_analyzer_gui/rg_utils.h"
#include "QtCommon/Scaling/ScalingManager.h"

static const char* kStrButtonFocusInStylesheetBinary = "QPushButton { background: rgb(253,255,174); border: 2px inset rgb(128, 0, 128); }";

RgMenuBinary::RgMenuBinary(QWidget* parent)
    : RgMenu(parent)
{
    // Enable drops so the the menu can handle opening dropped files.
    setAcceptDrops(true);
}

void RgMenuBinary::InitializeDefaultMenuItems(const std::shared_ptr<RgProjectClone>)
{
    // Insert the "Build Settings" item into the top of the menu.
    build_settings_menu_item_ = new RgMenuBuildSettingsItem();
    layout_->insertWidget(0, build_settings_menu_item_);
    // Disable the "Build Settings" oitem for Binary mode.
    // TODO - AMK3 - Unhide these to show build settings file button.
    auto build_settings_button = build_settings_menu_item_->GetBuildSettingsButton();
    if (build_settings_button)
    {
        build_settings_button->setEnabled(false);
        build_settings_button->setVisible(false);
    }

    // Insert the "Link Source" menu item to the top of the menu.
    link_source_menu_item_ = new RgLinkSourceMenuItem(this);
    layout_->insertWidget(0, link_source_menu_item_);
    auto link_source_file_button = link_source_menu_item_->GetLinkSourceButton();
    if (link_source_file_button)
    {
        link_source_file_button->setEnabled(false);
        link_source_file_button->setVisible(false);
    }

    link_source_menu_item_->ToggleLineSeparatorVisibilty(false);

    link_source_menu_item_->ToggleLoadCodeObjectButtonVisibilty(true);

    ConnectDefaultItemSignals();

    //ConnectButtonSignals();

    //// Make the menu as wide as the items.
    const int height = link_source_menu_item_->height();
    this->resize(link_source_menu_item_->width(), 2 * (height));

    //// Register the buttons with the scaling manager.
    ScalingManager::Get().RegisterObject(build_settings_menu_item_);
    ScalingManager::Get().RegisterObject(link_source_menu_item_);
}

void RgMenuBinary::ConnectDefaultItemSignals()
{
    // Handler invoked when the "Load CodeObj Binary" button is clicked within an API item.
    bool is_connected = connect(link_source_menu_item_->GetLoadCodeObjButton(), &QPushButton::clicked, this, &RgMenu::CreateFileButtonClicked);
    assert(is_connected);
    
    // Handler invoked when the "Link Source File" button is clicked within an API item.
    //is_connected = connect(link_source_menu_item_->GetLinkSourceButton(), &QPushButton::clicked, this, &RgMenu::OpenFileButtonClicked);
    //assert(is_connected);
}

void RgMenuBinary::ConnectButtonSignals()
{
    // Handler invoked when the "Build settings" button is clicked.
    bool is_connected = connect(build_settings_menu_item_->GetBuildSettingsButton(), &QPushButton::clicked, this, &RgMenu::BuildSettingsButtonClicked);
    assert(is_connected);

    is_connected = connect(build_settings_menu_item_->GetBuildSettingsButton(), &QPushButton::clicked, this, &RgMenu::HandleBuildSettingsButtonClicked);
    assert(is_connected);
}

void RgMenuBinary::ConnectMenuFileItemSignals(RgMenuFileItem* menu_item)
{
    RgMenuFileItemOpencl* opencl_item = static_cast<RgMenuFileItemOpencl*>(menu_item);

    assert(opencl_item != nullptr);
    if (opencl_item != nullptr)
    {
        // Connect the close button for the file's menu item.
        bool is_connected = connect(opencl_item, &RgMenuFileItemOpencl::MenuItemCloseButtonClicked, this, &RgMenu::MenuItemCloseButtonClicked);
        assert(is_connected);

        // Connect the file menu item selection handler for each new item.
        is_connected =  connect(opencl_item, &RgMenuFileItemOpencl::MenuItemSelected, this, &RgMenu::MenuItemClicked);
        assert(is_connected);

        // Connect the file menu item selection handler to update build settings button.
        is_connected = connect(menu_item, &RgMenuFileItemOpencl::MenuItemSelected, this, &RgMenu::HandleActivateItemAction);
        assert(is_connected);

        // Connect the file menu item rename handler for each new item.
        is_connected =  connect(opencl_item, &RgMenuFileItemOpencl::FileRenamed, this, &RgMenu::HandleRenamedFile);
        assert(is_connected);

        // Connect the file menu item's entry point changed handler.
        is_connected =  connect(opencl_item, &RgMenuFileItemOpencl::SelectedEntrypointChanged, this, &RgMenuBinary::SelectedEntrypointChanged);
        assert(is_connected);
    }
}

bool RgMenuBinary::AddItem(const std::string& full_path, bool is_new_file_item)
{
    bool wasAdded = false;

    // Extract the file name from the full file path.
    std::string filename;
    bool is_ok = RgUtils::ExtractFileName(full_path, filename);
    assert(is_ok);
    if (is_ok)
    {
        // If a file with that name hasn't been added.
        if (full_file_path_to_menu_item_.find(full_path) == full_file_path_to_menu_item_.end())
        {
            // Create the menu item widget and add it to the UI and to the list.
            RgMenuFileItemOpencl* new_menu_item = new RgMenuFileItemOpencl(full_path, this);
            menu_items_.push_back(new_menu_item);

            // Connect to this file item to change the stylesheet when the build is successful.
            bool is_connected = connect(this, &RgMenuBinary::ProjectBuildSuccess, new_menu_item, &RgMenuFileItemOpencl::HandleProjectBuildSuccess);
            assert(is_connected);

            // Emit a signal that indicates that the number of items in the menu change,
            // and specify that the menu is not empty.
            emit FileMenuItemCountChanged(false);

            // Add the file name and its full path to the map.
            full_file_path_to_menu_item_[full_path] = new_menu_item;

            // Insert the item just above the default "CL" menu item.
            const size_t index = (!menu_items_.empty()) ? (menu_items_.size() - 1) : 0;
            layout_->insertWidget(static_cast<const int>(index), new_menu_item, Qt::AlignTop);

            // Register the object with the scaling manager.
            ScalingManager::Get().RegisterObject(new_menu_item);

            // Connect signals for the new file menu item.
            ConnectMenuFileItemSignals(new_menu_item);

            // Select the file that was just opened.
            HandleSelectedFileChanged(new_menu_item);

            if (is_new_file_item)
            {
                // If a file was newly-created (as opposed to being opened), display the
                // item's renaming control immediately so the user can choose a new filename.
                new_menu_item->ShowRenameControls(true);
            }

            wasAdded = true;
        }
        else
        {
            // Report the error.
            std::stringstream msg;
            msg << kStrErrCannotAddFileA << filename << kStrErrCannotAddFileB;
            RgUtils::ShowErrorMessageBox(msg.str().c_str(), this);
        }
    }

    if (wasAdded)
    {
        link_source_menu_item_->ToggleLoadCodeObjectButtonVisibilty(false);
    }

    return wasAdded;
}

void RgMenuBinary::RemoveItem(const std::string& full_filename)
{
    RgMenu::RemoveItem(full_filename);

    if (IsEmpty())
    {
        link_source_menu_item_->ToggleLoadCodeObjectButtonVisibilty(true);
    }
}

void RgMenuBinary::RemoveAllItems()
{
    // Step through each file item in menu and emulate hitting the close button.
    for (RgMenuFileItem* file_item : menu_items_)
    {
        if (file_item != nullptr)
        {
            emit MenuItemCloseButtonClicked(file_item->GetFilename());
        }
    }
}

void RgMenuBinary::ClearBuildOutputs()
{
    // Step through each file item in menu and clear the entry point list.
    for (RgMenuFileItem* file_item : menu_items_)
    {
        RgMenuFileItemOpencl* file_item_opencl = static_cast<RgMenuFileItemOpencl*>(file_item);
        assert(file_item_opencl != nullptr);
        if (file_item_opencl != nullptr)
        {
            file_item_opencl->ClearEntrypointsList();
        }
    }
}

bool RgMenuBinary::GetIsShowEntrypointListEnabled() const
{
    return is_show_entrypoint_list_enabled_;
}

bool RgMenuBinary::OffsetCurrentFileEntrypoint(int offset)
{
    bool ret = false;

    // The offset should be a positive or negative integer. If it's not, something is wrong.
    assert(offset != 0);

    // If focus index is in the range of the menu file items, select the appropriate file item.
    bool is_focus_in_file_item_range = focus_index_ < menu_items_.size();
    assert(is_focus_in_file_item_range);
    if (is_focus_in_file_item_range)
    {
        RgMenuFileItem* file_item = menu_items_[focus_index_];

        RgMenuFileItemOpencl* opencl_item = static_cast<RgMenuFileItemOpencl*>(file_item);

        assert(opencl_item != nullptr);
        if (opencl_item != nullptr)
        {
            // Get the list of entry point names in the current file.
            std::vector<std::string> entrypoint_names;
            opencl_item->GetEntrypointNames(entrypoint_names);

            if (!entrypoint_names.empty())
            {
                // Get the name of the currently selected entrypoint.
                std::string selected_entrypoint_name;
                bool is_entrypoint_selected = opencl_item->GetSelectedEntrypointName(selected_entrypoint_name);
                if (is_entrypoint_selected && !selected_entrypoint_name.empty())
                {
                    // Compute the index of the currently selected entrypoint.
                    auto current_index_iter = std::find(entrypoint_names.begin(), entrypoint_names.end(), selected_entrypoint_name);
                    int selected_index = current_index_iter - entrypoint_names.begin();

                    // Offset the index, and check that it's still a valid index in the name list.
                    selected_index += offset;
                    if (selected_index >= 0 && selected_index < entrypoint_names.size())
                    {
                        // Emit a signal indicating that the selected entry point has changed.
                        auto next_entrypoint_iter = entrypoint_names.begin() + selected_index;
                        emit SelectedEntrypointChanged(opencl_item->GetFilename(), *next_entrypoint_iter);
                        ret = true;
                    }
                }
            }
        }
    }

    return ret;
}

void RgMenuBinary::SetIsShowEntrypointListEnabled(bool is_enabled)
{
    is_show_entrypoint_list_enabled_ = is_enabled;

    // Disable the ability to expand file items' entry point list.
    if (!is_enabled && selected_file_item_ != nullptr)
    {
        RgMenuFileItemOpencl* menu_item_opencl = static_cast<RgMenuFileItemOpencl*>(selected_file_item_);
        assert(menu_item_opencl != nullptr);
        if (menu_item_opencl != nullptr)
        {
            menu_item_opencl->ShowEntrypointsList(false);
        }
    }
}

void RgMenuBinary::UpdateBuildOutput(const RgBuildOutputsMap& build_outputs)
{
    std::string gpu_with_outputs;
    std::shared_ptr<RgCliBuildOutput> gpu_outputs = nullptr;
    bool is_output_valid = RgUtils::GetFirstValidOutputGpu(build_outputs, gpu_with_outputs, gpu_outputs);
    if (is_output_valid && (gpu_outputs != nullptr))
    {

        assert(gpu_outputs != nullptr);
        if (gpu_outputs != nullptr)
        {
            // Step through each file item in the menu.
            for (RgMenuFileItem* file_item : menu_items_)
            {
                RgMenuFileItemOpencl* file_item_opencl = static_cast<RgMenuFileItemOpencl*>(file_item);

                // Find the build output with the current item's filename.
                const std::string& item_filename = file_item->GetFilename();
                auto output_iter = gpu_outputs->per_file_output.find(item_filename);
                if (output_iter != gpu_outputs->per_file_output.end())
                {
                    // Update the menu item with the outputs for the matching input file.
                    RgFileOutputs& file_outputs = output_iter->second;
                    const std::vector<RgEntryOutput>& entry_outputs = file_outputs.outputs;
                    file_item_opencl->UpdateBuildOutputs(entry_outputs);
                }
            }

            // Does the user have a file selected in the menu?
            RgMenuFileItemOpencl* file_item_opencl = static_cast<RgMenuFileItemOpencl*>(selected_file_item_);
            if (file_item_opencl != nullptr)
            {
                // Auto-expand the list of entrypoints in the selected file item.
                file_item_opencl->ShowEntrypointsList(true);
            }
        }
    }
}

void RgMenuBinary::HandleBuildStarted()
{
    // Call the base class implementation.
    RgMenu::HandleBuildStarted();

    // Don't allow the user to expand the entry point list for file items.
    SetIsShowEntrypointListEnabled(false);

    // While a build is in progress, disable adding/creating files or changing the build settings.
    link_source_menu_item_->GetLinkSourceButton()->setEnabled(false);
}

void RgMenuBinary::HandleBuildEnded()
{
    // Call the base class implementation.
    RgMenu::HandleBuildEnded();

    // Re-enable allowing the user to expand the entry point list for file items.
    SetIsShowEntrypointListEnabled(true);

    // After the build is over, re-enable the menu items.
    link_source_menu_item_->GetLinkSourceButton()->setEnabled(true);
}

void RgMenuBinary::SelectFocusItem(FileMenuActionType)
{
    assert(build_settings_menu_item_ != nullptr);
    if (build_settings_menu_item_ != nullptr)
    {
        // Clear button style sheets.
        if (!build_settings_menu_item_->IsCurrent())
        {
            build_settings_menu_item_->GetBuildSettingsButton()->setStyleSheet("");
        }
        QPushButton* load_button = link_source_menu_item_->GetLoadCodeObjButton();
        QPushButton* link_button = link_source_menu_item_->GetLinkSourceButton();
        assert(load_button != nullptr);
        assert(link_button != nullptr);
        if (load_button != nullptr)
        {
            load_button->setStyleSheet("");
        }
        if (link_button != nullptr)
        {
            link_button->setStyleSheet("");
        }

        // If focus index is in the range of the menu file items, select the appropriate file item.
        if (focus_index_ < menu_items_.size())
        {
            RgMenuFileItemOpencl* item = static_cast<RgMenuFileItemOpencl*>(menu_items_[focus_index_]);
            assert(item != nullptr);
            if (item != nullptr)
            {
                UpdateHighlight(item);
            }

            // Update the cursor.
            UpdateCursor(item);
        }
        else
        {
            // Deselect OpenCL menu file items.
            for (RgMenuFileItem* item : menu_items_)
            {
                assert(item != nullptr);
                if (item != nullptr)
                {
                    item->SetHovered(false);
                }
            }

            // If out of range, special case handle the last focus items.
            // Get index excluding file items.
            size_t index = focus_index_ - menu_items_.size();

            // Handle special cases for Add, Create and Build settings buttons.
            switch (index)
            {
            case static_cast<size_t>(FileMenuFocusItems::kAddButton):
            {
                if (load_button != nullptr)
                {
                    load_button->setStyleSheet(kStrButtonFocusInStylesheetBinary);
                }
                break;
            }
            case static_cast<size_t>(FileMenuFocusItems::kCreateButton):
            {
                if (link_button != nullptr)
                {
                    link_button->setStyleSheet(kStrButtonFocusInStylesheetBinary);
                }
                break;
            }
            case static_cast<size_t>(FileMenuFocusItems::kBuildSettingsButton):
                build_settings_menu_item_->GetBuildSettingsButton()->setStyleSheet(kStrButtonFocusInStylesheetBinary);
                break;
            default:
                // Should never get here.
                assert(false);
                break;
            }
        }
    }
}

void RgMenuBinary::HandleActivateItemAction()
{
    assert(link_source_menu_item_ != nullptr);
    if (link_source_menu_item_ != nullptr)
    {
        // If focus index is in the range of the menu file items, select the appropriate file item.
        if (focus_index_ < menu_items_.size())
        {
            SelectFile(menu_items_[focus_index_]);

            // Set the build settings button to have focus out style sheets.
            assert(build_settings_menu_item_ != nullptr);
            if (build_settings_menu_item_ != nullptr)
            {
                build_settings_menu_item_->SetCurrent(false);
                build_settings_menu_item_->GetBuildSettingsButton()->setStyleSheet(kStrButtonFocusOutStylesheet);

                // Set the cursors.
                build_settings_menu_item_->GetBuildSettingsButton()->setCursor(Qt::PointingHandCursor);
            }
        }
        else
        {
            // Deselect menu file items.
            for (RgMenuFileItem* item : menu_items_)
            {
                RgMenuFileItemOpencl* item_opencl = static_cast<RgMenuFileItemOpencl*>(item);
                assert(item_opencl != nullptr);
                if (item_opencl != nullptr)
                {
                    item_opencl->SetHovered(false);
                    item_opencl->SetCurrent(false);
                }
            }

            // If out of range, special case handle the last focus items.
            // Get index excluding file items.
            size_t index = focus_index_ - menu_items_.size();

            // Handle special cases for Add, Create and Build settings buttons.
            switch (index)
            {
            case static_cast<size_t>(FileMenuFocusItems::kAddButton):
            {
                QPushButton* load_button = link_source_menu_item_->GetLoadCodeObjButton();
                assert(load_button != nullptr);
                if (load_button != nullptr)
                {
                    load_button->click();
                }
                break;
            }
            case static_cast<size_t>(FileMenuFocusItems::kCreateButton):
            {
                QPushButton* link_button = link_source_menu_item_->GetLinkSourceButton();
                assert(link_button != nullptr);
                if (link_button != nullptr)
                {
                    link_button->click();
                }
                break;
            }
            case static_cast<size_t>(FileMenuFocusItems::kBuildSettingsButton):
                assert(build_settings_menu_item_ != nullptr);
                if (build_settings_menu_item_ != nullptr)
                {
                    build_settings_menu_item_->GetBuildSettingsButton()->setStyleSheet(kStrButtonFocusInStylesheet);
                    build_settings_menu_item_->GetBuildSettingsButton()->click();
                }
                break;
            default:
                // Should never get here.
                assert(false);
                break;
            }
        }
    }
}

void RgMenuBinary::HandleBuildSettingsButtonClicked(bool /*checked*/)
{
    // Set the stylesheet for each file menu item to be not selected (grayed out),
    // along with the mouse cursor to pointing hand cursor.
    for (RgMenuFileItem* item : menu_items_)
    {
        item->SetHovered(false);
        item->SetCurrent(false);
        item->setCursor(Qt::PointingHandCursor);
    }

    // Set the focus index.
    focus_index_ = menu_items_.size() + GetButtonCount() - 1;

    // Set the "current" property value to true for build settings button.
    build_settings_menu_item_->SetCurrent(true);

    // Set the focus for the build settings button stylesheet.
    build_settings_menu_item_->GetBuildSettingsButton()->setStyleSheet(kStrButtonFocusInStylesheetBinary);

    // Set the mouse cursor for the build settings button.
    build_settings_menu_item_->GetBuildSettingsButton()->setCursor(Qt::ArrowCursor);
}

void RgMenuBinary::HandleSelectedEntrypointChanged(const std::string&, const std::string& input_file_path, const std::string& selected_entrypoint_name)
{
    // Find the given input file and select the incoming entry point name.
    for (RgMenuFileItem* file_item : menu_items_)
    {
        if (file_item->GetFilename().compare(input_file_path) == 0)
        {
            RgMenuFileItemOpencl* file_item_opencl = static_cast<RgMenuFileItemOpencl*>(file_item);
            assert(file_item_opencl != nullptr);
            if (file_item_opencl != nullptr)
            {
                // Switch to the given entry point in the file item's entry point list.
                file_item_opencl->SwitchToEntrypointByName(selected_entrypoint_name);
            }
        }
    }
}

void RgMenuBinary::HandleNextItemAction()
{
    size_t end_index = menu_items_.size() + 2;

    // If a file item is currently selected, switch to the next available entrypoint.
    bool next_entrypoint_selected = false;
    if (focus_index_ < menu_items_.size())
    {
        next_entrypoint_selected = OffsetCurrentFileEntrypoint(1);
    }

    if (!next_entrypoint_selected)
    {
        // Increment focus index and wrap around.
        focus_index_++;
        if (focus_index_ > end_index)
        {
            focus_index_ = 0;
        }

        // Refresh focus.
        SelectFocusItem(FileMenuActionType::kArrowAction);
    }
}

void RgMenuBinary::HandlePreviousItemAction()
{
    size_t end_index = menu_items_.size() + 2;

    // If a file item is currently selected, switch to the next available entrypoint.
    bool previous_entrypoint_selected = false;
    if (focus_index_ < menu_items_.size())
    {
        previous_entrypoint_selected = OffsetCurrentFileEntrypoint(-1);
    }

    if (!previous_entrypoint_selected)
    {
        // Decrement focus index and wrap around.
        if (focus_index_ != 0)
        {
            focus_index_--;
        }
        else
        {
            focus_index_ = end_index;
        }

        // Refresh focus.
        SelectFocusItem(FileMenuActionType::kArrowAction);
    }
}

void RgMenuBinary::HandleSourceFileAdded()
{
    RgMenuBuildSettingsItem* build_settings_button = GetBuildSettingsItem();
    assert(build_settings_button != nullptr);
    if (build_settings_button != nullptr)
    {
        build_settings_button->SetCurrent(false);
    }
}

int RgMenuBinary::GetButtonCount() const
{
    // There are 2 extra buttons besides the file items:
    // Link existing source code file
    // Build settings
    static const int kExtraBinaryButtonCount = 2;
    return kExtraBinaryButtonCount;
}

void RgMenuBinary::HandleSelectedFileChanged(RgMenuFileItem* selected)
{
    // Set focus index to newly selected item.
    for (size_t i = 0; i < menu_items_.size(); i++)
    {
        if (menu_items_[i] == selected)
        {
            focus_index_ = i;
            tab_focus_index_ = i;
            break;
        }
    }

    // Display the currently selected file in the source editor.
    DisplayFileInEditor(menu_items_[focus_index_]);
}

void RgMenuBinary::SetButtonsNoFocus()
{
    assert(build_settings_menu_item_ != nullptr);

    // Set the build settings button to have focus out style sheets.
    if (build_settings_menu_item_ != nullptr)
    {
        QPushButton* build_settings_button = build_settings_menu_item_->GetBuildSettingsButton();
        assert(build_settings_button != nullptr);
        if (build_settings_button != nullptr)
        {
            build_settings_button->setStyleSheet(kStrButtonFocusOutStylesheet);
        }
    }
}

void RgMenuBinary::SelectTabFocusItem(bool)
{
}

void RgMenuBinary::HandleTabFocusPressed()
{
}

void RgMenuBinary::HandleShiftTabFocusPressed()
{
}

void RgMenuBinary::dragEnterEvent(QDragEnterEvent* event)
{
    assert(event != nullptr);
    if (event != nullptr)
    {
        const QMimeData* mime_data = event->mimeData();
        assert(mime_data != nullptr);
        if (mime_data != nullptr)
        {
            bool is_file_read_error     = false;

            // Make sure the drop data has one or more file urls.
            if (mime_data->hasUrls())
            {
                // Cycle thru all the files being dropped.
                foreach (auto file, mime_data->urls())
                {
                    if (file.isLocalFile())
                    {
                        // Accept the action, making it so we receive a dropEvent when the items are released.
                        event->setDropAction(Qt::DropAction::CopyAction);
                        event->accept();
                    }
                    else
                    {
                        is_file_read_error = true;
                    }
                }
                if (is_file_read_error)
                {
                    event->ignore();
                }
            }
            else
            {
                event->ignore();
            }
        }
    }
}

void RgMenuBinary::dragMoveEvent(QDragMoveEvent* event)
{
    event->setDropAction(Qt::CopyAction);
    event->accept();
}

void RgMenuBinary::dropEvent(QDropEvent* event)
{
    const QMimeData* mime_data = event->mimeData();

    // Make sure the drop data has a file.
    assert(mime_data != nullptr);
    if (mime_data != nullptr)
    {
        if (mime_data->hasUrls())
        {
            // Cycle thru all the files being dropped.
            foreach (auto file, mime_data->urls())
            {
                // Check to make sure the file is valid.
                if (file.isLocalFile())
                {
                    // Get the file path.
                    std::string file_path = file.toLocalFile().toStdString();

                    // Emit a signal to add an existing file.
                    emit DragAndDropExistingFile(file_path);
                }
            }
        }
        else
        {
            event->ignore();
        }
    }
    else
    {
        event->ignore();
    }
}

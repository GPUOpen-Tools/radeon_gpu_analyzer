//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for Item widget in RGA Build view's File Menu for OpenCL mode.
//=============================================================================

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
#include "radeon_gpu_analyzer_gui/qt/rg_menu_opencl.h"
#include "radeon_gpu_analyzer_gui/qt/rg_menu_file_item_opencl.h"
#include "radeon_gpu_analyzer_gui/rg_utils.h"
#include "radeon_gpu_analyzer_gui/rg_definitions.h"

#include "common/rga_xml_constants.h"

static const std::vector<std::string> kStrXmlNodeRtxVec = {kStrXmlNodeDxrRayGeneration,
                                                           kStrXmlNodeDxrIntersection,
                                                           kStrXmlNodeDxrAnyHit,
                                                           kStrXmlNodeDxrClosestHit,
                                                           kStrXmlNodeDxrMiss,
                                                           kStrXmlNodeDxrCallable,
                                                           kStrXmlNodeDxrTraversal,
                                                           kStrXmlNodeDxrUnknown,
                                                           kStrXmlNodeVkRayGeneration,
                                                           kStrXmlNodeVkIntersection,
                                                           kStrXmlNodeVkAnyHit,
                                                           kStrXmlNodeVkClosestHit,
                                                           kStrXmlNodeVkMiss,
                                                           kStrXmlNodeVkCallable,
                                                           kStrXmlNodeVkTraversal,
                                                           kStrXmlNodeVkUnknown};

// Stylesheet for file menu items for when they are selcted and not selected.
static const char* kStrFileMenuItemColor =
    "#itemBackground[current = false] {background-color: palette(midlight)} #itemBackground[current = true] {background-color: palette(base); border-style: "
    "solid; border-width: 1px;}";

static const int s_FILE_MENU_KERNEL_ITEM_HEIGHT     = 20;
static const int s_FILE_MENU_RT_KERNEL_ITEM_HEIGHT = s_FILE_MENU_KERNEL_ITEM_HEIGHT * 2;

bool IsRayTracingKernelType(const std::string& kernel_type)
{
    return std::find(std::begin(kStrXmlNodeRtxVec), std::end(kStrXmlNodeRtxVec), kernel_type) != std::end(kStrXmlNodeRtxVec);
}

std::pair<std::string, std::string> SeparateKernelAndKernelSubtype(const std::string& combined_name)
{
    size_t             offset = combined_name.find('_');
    const std::string& ext    = (offset != std::string::npos && offset < combined_name.size()) ? combined_name.substr(0, offset) : "";
    const std::string& kernel = (offset != std::string::npos && ++offset < combined_name.size()) ? combined_name.substr(offset) : "";
    return {kernel, ext};
}

// A delegate used to style a file item's entry point list.
class RgEntrypointItemStyleDelegate : public QStyledItemDelegate
{
public:
    RgEntrypointItemStyleDelegate(QObject* parent = nullptr)
        : QStyledItemDelegate(parent)
    {
    }

    // A custom row painter for the entry point list.
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
    {
        QStyleOptionViewItem item_option(option);

        if (index.isValid())
        {
            initStyleOption(&item_option, index);
        }

        // Invoke the item paint implementation with the updated item options.
        QStyledItemDelegate::paint(painter, item_option, index);
    }
};

RgMenuItemEntryListModel::RgMenuItemEntryListModel(QWidget* parent)
    : QObject(parent)
{
    // Initialize the item model.
    entry_point_item_model_ = new QStandardItemModel(parent);

    // Figure out which API is being used. This will determine which text is used for the item's entrypoints list.
    RgProjectAPI      current_api           = RgConfigManager::Instance().GetCurrentAPI();
    const std::string entrypoint_label_text = RgUtils::GetEntrypointsNameString(current_api);

    // Add the column header text.
    entry_point_item_model_->setHorizontalHeaderItem(0, new QStandardItem(entrypoint_label_text.c_str()));
}

QStandardItemModel* RgMenuItemEntryListModel::GetEntryItemModel() const
{
    return entry_point_item_model_;
}

void RgMenuItemEntryListModel::AddEntry(const RgEntryOutput& entrypoint)
{
    const auto& entrypoint_name            = entrypoint.entrypoint_name;
    bool        is_rt_entrypoint           = IsRayTracingKernelType(entrypoint.kernel_type);
    const auto& extremely_long_kernel_name = entrypoint.extremely_long_kernel_name;
    bool        has_extremely_long_name    = !entrypoint.extremely_long_kernel_name.empty();

    std::string display_text;

    int current_row_count = entry_point_item_model_->rowCount();
    int new_row_count     = current_row_count + 1;

    if (is_rt_entrypoint)
    {
        const auto&        p              = SeparateKernelAndKernelSubtype(entrypoint_name);
        const std::string& kernel_name    = p.first;
        const std::string& kernel_subtype = p.second;
        // Truncate long entry point names.
        RgUtils::GetDisplayText(kernel_name, display_text, entry_point_widget_width_, entry_point_tree_, kTextTruncateLengthBackOpencl);
        std::stringstream ss;
        ss << kernel_subtype << ":\n"
           << "    " << display_text;
        display_text = ss.str();
    }
    else
    {
        // Truncate long entry point names.
        RgUtils::GetDisplayText(entrypoint_name, display_text, entry_point_widget_width_, entry_point_tree_, kTextTruncateLengthBackOpencl);
    }

    // Save the entry point name and the possibly truncated display name.
    entry_point_names_.push_back(entrypoint_name);
    display_names_.push_back(display_text);
    if (has_extremely_long_name)
    {
        extremely_long_kernel_names_map[entrypoint_name] = extremely_long_kernel_name;
    }

    // Update the number of row items in the model.
    entry_point_item_model_->setRowCount(new_row_count);

    // Set the data for the new item row, and left-align the entry point name string.
    QModelIndex model_index = entry_point_item_model_->index(current_row_count, 0);
    entry_point_item_model_->setData(model_index, QString(display_text.c_str()));
    entry_point_item_model_->setData(model_index, Qt::AlignLeft, Qt::TextAlignmentRole);
    if (!has_extremely_long_name)
    {
        entry_point_item_model_->setData(model_index, QString(entrypoint_name.c_str()), RgMenuUserRoles::kCopyNameRole);
    }
    else
    {
        entry_point_item_model_->setData(model_index, QString(extremely_long_kernel_name.c_str()), RgMenuUserRoles::kCopyNameRole);
    }
    entry_point_item_model_->dataChanged(model_index, model_index);

    // Set the tooltip for the new entry point item.
    std::stringstream tooltip_text;
    tooltip_text << kStrMenuItemEntrypointTooltipText;
    if (!has_extremely_long_name)
    {
        tooltip_text << entrypoint_name;
    }
    else
    {
        tooltip_text << extremely_long_kernel_name;
    }
    entry_point_item_model_->setData(model_index, tooltip_text.str().c_str(), Qt::ToolTipRole);

    // Set the scaled height for size hint.
    QSize size = entry_point_item_model_->data(model_index, Qt::SizeHintRole).toSize();
    if (is_rt_entrypoint)
    {
        size.setHeight(s_FILE_MENU_RT_KERNEL_ITEM_HEIGHT);
    }
    else
    {
        size.setHeight(s_FILE_MENU_KERNEL_ITEM_HEIGHT);
    }
    entry_point_item_model_->setData(model_index, size, Qt::SizeHintRole);
}

void RgMenuItemEntryListModel::ClearEntries()
{
    // Clear the model data by removing all existing rows.
    int num_rows = entry_point_item_model_->rowCount();
    entry_point_item_model_->removeRows(0, num_rows);

    // Clear the entry point names as well.
    entry_point_names_.clear();
    display_names_.clear();
    extremely_long_kernel_names_map.clear();
}

void RgMenuItemEntryListModel::SetEntryPointWidgetWidth(const int width)
{
    entry_point_widget_width_ = width;
}

void RgMenuItemEntryListModel::SetEntryPointTreeWidget(RgMenuEntryPointTree* tree)
{
    entry_point_tree_ = tree;
}

void RgMenuItemEntryListModel::GetEntryPointNames(std::vector<std::string>& entrypoint_names)
{
    entrypoint_names = entry_point_names_;
}

std::string RgMenuItemEntryListModel::GetEntryPointName(const int index) const
{
    return entry_point_names_[index];
}

std::string RgMenuItemEntryListModel::GetEntryPointName(const std::string& display_entrypoint_name) const
{
    std::string value;
    if (!display_entrypoint_name.empty())
    {
        auto iter = std::find(display_names_.begin(), display_names_.end(), display_entrypoint_name);
        if (iter != display_names_.end())
        {
            value = *iter;
        }
    }
    return value;
}

bool RgMenuItemEntryListModel::GetSelectedEntrypointExtremelyLongName(const std::string& entrypoint_name, std::string& extremely_long_name) const
{
    bool ret   = false;
    auto found = extremely_long_kernel_names_map.find(entrypoint_name);
    if (found != extremely_long_kernel_names_map.end())
    {
        extremely_long_name = found->second;
        ret                 = true;
    }
    return ret;
}

RgMenuFileItemOpencl::RgMenuFileItemOpencl(const std::string& file_full_path, RgMenu* parent)
    : RgMenuFileItem(file_full_path, parent)
{
    ui_.setupUi(this);

    setStyleSheet(kStrFileMenuItemColor);

    UpdateFilepath(full_file_path_);

    // Start as a saved item.
    SetIsSaved(true);

    // Don't show QLineEdit item renaming control when an item is first created.
    ShowRenameControls(false);

    // The close button is hidden by default, and is made visible on item mouseover.
    ui_.closeButton->hide();

    // Set tool and status tip for close button.
    std::string tooltip = kStrFileMenuRemoveFileTooltipPrefix + full_file_path_ + kStrFileMenuRemoveFileTooltipSuffix;
    RgUtils::SetToolAndStatusTip(tooltip, ui_.closeButton);

    // Initialize the entry point list.
    InitializeEntrypointsList();

    // Connect signals for the file item.
    ConnectSignals();

    // Enable mouse tracking for the entry point list view.
    ui_.entrypointListView->setMouseTracking(true);

    // Set mouse pointer to pointing hand cursor.
    SetCursor();
}

void RgMenuFileItemOpencl::enterEvent(QEnterEvent* event)
{
    Q_UNUSED(event);

    ui_.closeButton->show();

    // Change the item color.
    SetHovered(true);
}

void RgMenuFileItemOpencl::leaveEvent(QEvent* event)
{
    Q_UNUSED(event);

    ui_.closeButton->hide();

    // Change the item color.
    SetHovered(false);
}

void RgMenuFileItemOpencl::mouseDoubleClickEvent(QMouseEvent* event)
{
    Q_UNUSED(event);

    // On double-click, allow the user to re-name the item's filename.
    ShowRenameControls(true);
}

void RgMenuFileItemOpencl::mousePressEvent(QMouseEvent* event)
{
    Q_UNUSED(event);

    emit MenuItemSelected(this);
}

void RgMenuFileItemOpencl::resizeEvent(QResizeEvent* event)
{
    Q_UNUSED(event);

    UpdateFilenameLabelText();
}

void RgMenuFileItemOpencl::showEvent(QShowEvent* event)
{
    Q_UNUSED(event);

    UpdateFilenameLabelText();
}

void RgMenuFileItemOpencl::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape)
    {
        is_escape_pressed_ = true;
        // Hide the rename box, and display the labels
        ShowRenameControls(false);
    }
    else
    {
        // Pass the event onto the base class
        QWidget::keyPressEvent(event);
    }
}

void RgMenuFileItemOpencl::ClearEntrypointsList()
{
    assert(entry_list_model_ != nullptr);
    if (entry_list_model_ != nullptr)
    {
        entry_list_model_->ClearEntries();
    }
}

void RgMenuFileItemOpencl::GetEntrypointNames(std::vector<std::string>& entrypoint_names) const
{
    entry_list_model_->GetEntryPointNames(entrypoint_names);
}

bool RgMenuFileItemOpencl::GetSelectedEntrypointName(std::string& entrypoint_name) const
{
    bool got_entrypoint_name = false;

    // Find the selected entry point in the list's selection model.
    QItemSelectionModel* selection_model = ui_.entrypointListView->selectionModel();
    assert(selection_model != nullptr);
    if (selection_model != nullptr)
    {
        // 1. Look for currently selected entry in the Selection Model.
        QModelIndex selected_entrypoint_index = selection_model->currentIndex();
        if (selected_entrypoint_index.isValid())
        {
            // Extract and return the entry point name from the list.
            entrypoint_name     = entry_list_model_->GetEntryPointName(selected_entrypoint_index.row());
            got_entrypoint_name = true;
        }
        else
        {
            // 2. If the Selection Model is empty (it may have been cleared after the build), check the last selected entry name.
            if (!last_selected_entry__name_.empty())
            {
                entrypoint_name     = last_selected_entry__name_;
                got_entrypoint_name = true;
            }
        }
    }

    return got_entrypoint_name;
}

bool RgMenuFileItemOpencl::GetSelectedEntrypointExtremelyLongName(const std::string& entrypoint_name, std::string& extremely_long_name) const
{
    return entry_list_model_->GetSelectedEntrypointExtremelyLongName(entrypoint_name, extremely_long_name);
}

void RgMenuFileItemOpencl::SetHovered(bool is_hovered)
{
    // Set "hovered" property to be utilized by this widget's stylesheet.
    ui_.itemBackground->setProperty(kStrFileMenuPropertyHovered, is_hovered);

    // Repolish the widget to ensure the style gets updated.
    ui_.itemBackground->style()->unpolish(ui_.itemBackground);
    ui_.itemBackground->style()->polish(ui_.itemBackground);
}

void RgMenuFileItemOpencl::SetCurrent(bool is_current, bool hide_entry_point_lists)
{
    // Set "current" property to be utilized by this widget's stylesheet.
    ui_.itemBackground->setProperty(kStrFileMenuPropertyCurrent, is_current);

    // Repolish the widget to ensure the style gets updated.
    ui_.itemBackground->style()->unpolish(ui_.itemBackground);
    ui_.itemBackground->style()->polish(ui_.itemBackground);

    // Toggle the visibility of the entrypoints list based on if the item is current or not.
    ShowEntrypointsList(is_current || !hide_entry_point_lists);
}

void RgMenuFileItemOpencl::UpdateFilenameLabelText()
{
    std::string text = filename_;
    std::string display_text;

    // Determine suffix based on whether or not the file is saved.
    if (!is_saved_)
    {
        text += kStrUnsavedFileSuffix;
    }

    // Get available space.
    const int available_space = ui_.filenameDisplayLayout->contentsRect().width();

    // Get and set display text.
    RgUtils::GetDisplayText(text, display_text, available_space, ui_.filenameLabel, kTextTruncateLengthBackOpencl);
    ui_.filenameLabel->setText(display_text.c_str());

    // Set the full path as a tooltip.
    this->setToolTip(full_file_path_.c_str());
}

QLineEdit* RgMenuFileItemOpencl::GetRenameLineEdit()
{
    return ui_.filenameLineEdit;
}

QLabel* RgMenuFileItemOpencl::GetItemLabel()
{
    return ui_.filenameLabel;
}

void RgMenuFileItemOpencl::UpdateFilepath(const std::string& new_file_path)
{
    full_file_path_ = new_file_path;

    // Only display the filename in the interface- not the full path to the file.
    bool is_ok = RgUtils::ExtractFileName(new_file_path, filename_);
    if (is_ok == false)
    {
        // Attempt to display as much of the full file path as possible.
        filename_ = new_file_path;
    }

    // Update the view to display the latest filename.
    UpdateFilenameLabelText();
}

void RgMenuFileItemOpencl::ConnectSignals()
{
    // Connect the item's close button.
    bool is_connected = connect(ui_.closeButton, &QPushButton::clicked, this, &RgMenuFileItemOpencl::HandleRemoveItemRequest);
    assert(is_connected);

    // Connect the filename QLineEdit signals, so the user can confirm a rename by pressing Return.
    is_connected = connect(ui_.filenameLineEdit, &QLineEdit::returnPressed, this, &RgMenuFileItemOpencl::HandleEnterPressed);
    assert(is_connected);

    // Connect the entry point list selected item changed signal.
    is_connected = connect(ui_.entrypointListView, &QTreeView::clicked, this, &RgMenuFileItemOpencl::HandleEntrypointClicked);
    assert(is_connected);

    // Slot/signal for the tree view.
    is_connected = connect(ui_.entrypointListView, &QTreeView::entered, this, &RgMenuFileItemOpencl::HandleTableItemEntered);
    assert(is_connected);

    // Connect the remove file context menu with the remove item request signal.
    is_connected = connect(context_menu_actions_.remove_file, &QAction::triggered, this, &RgMenuFileItemOpencl::HandleRemoveItemRequest);
    assert(is_connected);
}

void RgMenuFileItemOpencl::InitializeEntrypointsList()
{
    // Initialize the Expand/Contract icons. Use the expand icon by default.
    entry_list_model_ = new RgMenuItemEntryListModel(this);
    ui_.entrypointListView->setModel(entry_list_model_->GetEntryItemModel());
    ui_.entrypointListView->hide();

    // Set the widget width so the entry point names get truncated correctly.
    entry_list_model_->SetEntryPointWidgetWidth(ui_.entrypointListView->contentsRect().width());

    // Set the widget.
    entry_list_model_->SetEntryPointTreeWidget(ui_.entrypointListView);

    entry_point_style_delegate_ = new RgEntrypointItemStyleDelegate(ui_.entrypointListView);
    ui_.entrypointListView->setItemDelegate(entry_point_style_delegate_);
}

void RgMenuFileItemOpencl::ShowEntrypointsList(bool show_list)
{
    // Toggle the visibility of the entry point list.
    if (show_list)
    {
        QStandardItemModel* entrypoint_list_model = entry_list_model_->GetEntryItemModel();
        if (entrypoint_list_model != nullptr)
        {
            bool is_entrypoint_list_enabled = false;

            // Will the parent menu allow expanding a file item's entry point list?
            RgMenuOpencl* parent_file_menu = static_cast<RgMenuOpencl*>(GetParentMenu());
            assert(parent_file_menu != nullptr);
            if (parent_file_menu != nullptr)
            {
                is_entrypoint_list_enabled = parent_file_menu->GetIsShowEntrypointListEnabled();
            }

            // Show the entry point list if it's allowed.
            if (is_entrypoint_list_enabled)
            {
                // Show the entrypoints list only when it's non-empty.
                if (entrypoint_list_model->rowCount() > 0)
                {
                    ui_.entrypointListView->show();
                }
            }
        }
    }
    else
    {
        ui_.entrypointListView->hide();
    }
}

void RgMenuFileItemOpencl::ClearSelectedEntryPoints()
{
    QItemSelectionModel* selection_model = ui_.entrypointListView->selectionModel();
    if (selection_model != nullptr)
    {
        selection_model->clear();
    }
}

void RgMenuFileItemOpencl::SwitchToEntrypointByName(const std::string& display_name)
{
    // Get the list of entry point names for this file item.
    std::vector<std::string> entrypoint_names;
    GetEntrypointNames(entrypoint_names);

    int  selected_entrypoint_index = 0;
    bool found_entry_point         = false;
    for (const std::string& currentEntry : entrypoint_names)
    {
        if (display_name.compare(currentEntry) == 0)
        {
            found_entry_point = true;
            break;
        }
        selected_entrypoint_index++;
    }

    // Compute the model index for the selected entrypoint's row, and set the selection in the selection model.
    if (found_entry_point)
    {
        QItemSelectionModel* selection_model = ui_.entrypointListView->selectionModel();
        assert(selection_model != nullptr);
        if (selection_model != nullptr)
        {
            QModelIndex selected_row_index = entry_list_model_->GetEntryItemModel()->index(selected_entrypoint_index, 0);
            selection_model->setCurrentIndex(selected_row_index, QItemSelectionModel::SelectCurrent);
            last_selected_entry__name_ = entrypoint_names[selected_entrypoint_index];
        }
    }
}

void RgMenuFileItemOpencl::UpdateBuildOutputs(const std::vector<RgEntryOutput>& entry_outputs)
{
    std::string current_display_entrypoint_name;
    bool        is_entrypoint_selected = GetSelectedEntrypointName(current_display_entrypoint_name);

    assert(entry_list_model_ != nullptr);
    if (entry_list_model_ != nullptr)
    {
        // Clear the existing list of outputs.
        entry_list_model_->ClearEntries();

        if (!entry_outputs.empty())
        {
            // Add a new item in the list for each build output entry.
            for (const RgEntryOutput& entry_output : entry_outputs)
            {
                entry_list_model_->AddEntry(entry_output);
            }

            // Update the entry point table view height.
            ui_.entrypointListView->AdjustTreeSize(entry_list_model_->GetEntryItemModel());

            std::vector<std::string> entrypoint_names;
            GetEntrypointNames(entrypoint_names);
            if (!entrypoint_names.empty())
            {
                bool select_first_entrypoint = false;
                if (is_entrypoint_selected)
                {
                    // Attempt to re-select the same entry point from the new build outputs.
                    auto entry_iter = std::find(entrypoint_names.begin(), entrypoint_names.end(), current_display_entrypoint_name);
                    if (entry_iter != entrypoint_names.end())
                    {
                        // Re-select the previously selected entrypoint.
                        SwitchToEntrypointByName(entry_list_model_->GetEntryPointName(current_display_entrypoint_name));
                    }
                    else
                    {
                        // Select the first entry point by default since the previous selection no longer exists.
                        select_first_entrypoint = true;
                    }
                }
                else
                {
                    // Select the first entry point by default since there was no selection previously.
                    select_first_entrypoint = true;
                }

                // Fall back to selecting the first entrypoint.
                if (select_first_entrypoint)
                {
                    // If the user didn't have an entry point selected previously, automatically select the first entrypoint.
                    current_display_entrypoint_name = entrypoint_names[0];

                    const std::string& input_file_path = GetFilename();
                    emit               SelectedEntrypointChanged(input_file_path, current_display_entrypoint_name);
                }
            }
        }
    }
}

void RgMenuFileItemOpencl::HandleRemoveItemRequest()
{
    emit MenuItemCloseButtonClicked(this->GetFilename());
}

void RgMenuFileItemOpencl::HandleEntrypointClicked()
{
    // Signal to the menu that the user has clicked within this item. This will switch the current file.
    emit MenuItemSelected(this);

    // Is the selected item valid?
    QItemSelectionModel* selection_model = ui_.entrypointListView->selectionModel();
    if (selection_model->currentIndex().isValid())
    {
        // Pull the filename out of the selected item.
        const std::string& file_path    = GetFilename();
        int                selected_row = selection_model->currentIndex().row();

        // Get a list of all the entry point names for the input file.
        std::vector<std::string> entrypoint_names;
        GetEntrypointNames(entrypoint_names);

        bool is_valid_row = selected_row < entrypoint_names.size();
        assert(is_valid_row);
        if (is_valid_row)
        {
            // Emit a signal indicating that the selected entry point has changed.
            emit SelectedEntrypointChanged(file_path, entrypoint_names[selected_row]);
        }
    }
}

void RgMenuFileItemOpencl::HandleTableItemEntered(const QModelIndex& model_index)
{
    // Set the cursor if model_index valid.
    if (model_index.isValid())
    {
        ui_.entrypointListView->setCursor(Qt::PointingHandCursor);
    }
    else
    {
        ui_.entrypointListView->setCursor(Qt::ArrowCursor);
    }
}

void RgMenuFileItemOpencl::SetCursor()
{
    // Set the cursor to pointing hand cursor.
    ui_.closeButton->setCursor(Qt::PointingHandCursor);
}

void RgMenuFileItemOpencl::HandleProjectBuildSuccess()
{
    // Change the style sheet when the project built successfully.
    setStyleSheet(kStrFileMenuItemColor);
}

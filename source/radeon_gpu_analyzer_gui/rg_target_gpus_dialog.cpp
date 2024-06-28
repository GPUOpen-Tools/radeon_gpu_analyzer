// C++.
#include <cassert>
#include <sstream>
#include <set>
#include <algorithm>

// Qt.
#include <QDialog>
#include <QKeyEvent>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QVariant>

// Infra.
#include "qt_common/utils/qt_util.h"

// Shared.
#include "common/rga_sorting_utils.h"

// Shared.
#include "common/rga_sorting_utils.h"

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_target_gpus_dialog.h"
#include "radeon_gpu_analyzer_gui/rg_cli_launcher.h"
#include "radeon_gpu_analyzer_gui/rg_string_constants.h"
#include "radeon_gpu_analyzer_gui/rg_utils.h"
#include "radeon_gpu_analyzer_gui/rg_xml_cli_version_info.h"

// A class used to filter the GPU table widget.
class RgTableFilterProxyModel : public QSortFilterProxyModel
{
public:
    RgTableFilterProxyModel(RgTargetGpusDialog* parent_dialog = nullptr) : QSortFilterProxyModel(parent_dialog), parent_Dialog(parent_dialog) {}
    virtual ~RgTableFilterProxyModel() = default;

protected:
    // A row-filtering predicate responsible for matching rows against the user's search text.
    virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override
    {
        return parent_Dialog->IsRowVisible(source_row, source_parent, filterRegularExpression());
    }

private:
    // The parent dialog whose table is being filtered.
    RgTargetGpusDialog* parent_Dialog = nullptr;
};

bool RgTreeEventFilter::eventFilter(QObject* object, QEvent* event)
{
    bool is_handled = false;

    assert(event != nullptr);
    if (event != nullptr && event->type() == QEvent::Type::KeyPress)
    {
        // Watch for KeyPress events on only the Spacebar.
        QKeyEvent* key_event = dynamic_cast<QKeyEvent*>(event);
        assert(key_event != nullptr);
        if (key_event != nullptr && key_event->key() == Qt::Key_Space)
        {
            // Emit the signal used to activate the selected row.
            emit RowActivated();
            is_handled = true;
        }
    }

    if (!is_handled)
    {
        // Invoke the base implementation.
        is_handled = QObject::eventFilter(object, event);
    }

    return is_handled;
}

RgTargetGpusDialog::RgTargetGpusDialog(const QString& selected_gpus, QWidget* parent) :
    QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
    parent_(parent)
{
    ui_.setupUi(this);

    // Double-clicking an item within the QTreeView will toggle its check state- not expand/collapse the children.
    ui_.gpuTreeView->setExpandsOnDoubleClick(false);

    // Disable editing cells within the table.
    ui_.gpuTreeView->setEditTriggers(QAbstractItemView::EditTrigger::NoEditTriggers);

    // Create an eventFilter used to handle keyPress events in the GPU treeview.
    RgTreeEventFilter* keypress_event_filter = new RgTreeEventFilter(this);
    assert(keypress_event_filter != nullptr);
    if (keypress_event_filter != nullptr)
    {
        // Connect the handler that gets invoked when the user toggles a row with the spacebar.
        bool is_connected = connect(keypress_event_filter, &RgTreeEventFilter::RowActivated, this, &RgTargetGpusDialog::HandleRowToggled);
        assert(is_connected);
        if (is_connected)
        {
            // Install the eventFilter in the treeview.
            ui_.gpuTreeView->installEventFilter(keypress_event_filter);
        }
    }

    // Use the cached CLI version info structure to populate the GPU tree.
    std::shared_ptr<RgCliVersionInfo> version_info = RgConfigManager::Instance().GetVersionInfo();
    assert(version_info != nullptr);
    if (version_info != nullptr)
    {
        const std::string& current_mode = RgConfigManager::Instance().GetCurrentModeString();
        PopulateTableData(version_info, current_mode);
    }

    // Connect the target GPUs dialog signals.
    ConnectSignals();

    // Set window flags.
    setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint);

    // Split the incoming string of comma-separated families into a vector.
    std::vector<std::string> selected_gpus_vector;
    RgUtils::splitString(selected_gpus.toStdString(), RgConfigManager::kRgaListDelimiter, selected_gpus_vector);

    // Preprocess the gpu family names: since we get some family names
    // as a codename without the gfx notation, we would have to add the
    // gfx notation here again.
    std::transform(selected_gpus_vector.begin(), selected_gpus_vector.end(),
        selected_gpus_vector.begin(), [&](std::string& familyName)
    {
        std::stringstream fixed_name;
        std::string gfx_notation;
        bool hasGfxNotation = RgUtils::GetGfxNotation(familyName, gfx_notation);
        if (hasGfxNotation && familyName.find("/") == std::string::npos)
        {
            fixed_name << gfx_notation << "/" << familyName;
        }
        else
        {
            fixed_name << familyName;
        }
        return fixed_name.str();
    });

    // Update the state of the OK button based on the number of selected GPUs.
    ToggleOKButtonEnabled(!selected_gpus_vector.empty());

    // Initialize the model item's initial checkState for each row.
    InitializeCheckedRows(selected_gpus_vector);

    // Set the cursor for the treeview to be pointing hand cursor.
    ui_.gpuTreeView->setCursor(Qt::PointingHandCursor);
}

std::vector<std::string> RgTargetGpusDialog::GetSelectedCapabilityGroups() const
{
    std::vector<std::string> selected_families;

    // Step through all rows in the table to find rows that are checked.
    for (int row_index = 0; row_index < gpu_tree_model_->rowCount(); ++row_index)
    {
        QStandardItem* row_check_item = gpu_tree_model_->item(row_index, ColumnType::kSelectedCheckbox);
        if (row_check_item != nullptr)
        {
            bool is_row_checked = row_check_item->checkState();
            if (is_row_checked)
            {
                // If the row is checked, find the compute capability group associated with the row.
                auto row_to_group_index_iter = table_row_index_to_group_index_.find(row_index);
                if (row_to_group_index_iter != table_row_index_to_group_index_.end())
                {
                    int group_index = row_to_group_index_iter->second;
                    bool is_valid_index = (group_index >= 0 && group_index < capability_groups_.size());
                    assert(is_valid_index);
                    if (is_valid_index)
                    {
                        const CapabilityGroup& group_info = capability_groups_[group_index];

                        // Retrieve the compute capability string from the first row of the group.
                        std::string group_name = group_info.group_rows[0].compute_capability;
                        auto is_already_added_iter = std::find(selected_families.begin(), selected_families.end(), group_name);
                        if (is_already_added_iter == selected_families.end())
                        {
                            selected_families.push_back(group_name);
                        }
                    }
                }
            }
        }
    }

    return selected_families;
}

void RgTargetGpusDialog::HandleItemChanged(QStandardItem* item)
{
    // Disable the item check changed handler to avoid a recursive overflow.
    ToggleItemCheckChangedHandler(false);

    // Update the check state for the selected group's rows.
    QModelIndex selected_item_index = item->index();
    if (selected_item_index.isValid())
    {
        int selected_row = selected_item_index.row();
        auto group_index_iter = table_row_index_to_group_index_.find(selected_row);
        if (group_index_iter != table_row_index_to_group_index_.end())
        {
            int group_index = group_index_iter->second;
            bool is_checked = (item->checkState() == Qt::Checked);

            SetIsGroupChecked(group_index, is_checked);
        }
    }

    // Only allow the user to confirm their changes if there's at least one family selected.
    std::vector<std::string> selected_families = GetSelectedCapabilityGroups();
    bool is_ok_enabled = !selected_families.empty();

    // Update the enabledness of the dialog's OK button.
    ToggleOKButtonEnabled(is_ok_enabled);

    // Re-enable the item check changed handler.
    ToggleItemCheckChangedHandler(true);
}

void RgTargetGpusDialog::HandleRowToggled()
{
    // Use the treeView's selectionModel to find the currently selected row index.
    QItemSelectionModel* selection_model = ui_.gpuTreeView->selectionModel();
    assert(selection_model != nullptr);
    if (selection_model != nullptr)
    {
        // Is the selected row index valid?
        QModelIndex selected_row = selection_model->currentIndex();
        if (selected_row.isValid())
        {
            assert(table_filter_model_ != nullptr);
            if (table_filter_model_ != nullptr)
            {
                // The selection model index doesn't know anything about the filtering model. Convert
                // the (potentially filtered) selection model index to a source model index.
                QModelIndex source_model_index = table_filter_model_->mapToSource(selected_row);
                if (source_model_index.isValid())
                {
                    // Provide the source model index for the row being toggled.
                    ToggleRowChecked(source_model_index);
                }
            }
        }
    }
}

void RgTargetGpusDialog::HandleRowDoubleClicked(const QModelIndex& index)
{
    assert(table_filter_model_ != nullptr);
    if (table_filter_model_ != nullptr)
    {
        QModelIndex source_model_index = table_filter_model_->mapToSource(index);
        if (source_model_index.isValid())
        {
            // Provide the source model index for the row being toggled.
            ToggleRowChecked(source_model_index);
        }
    }
}

void RgTargetGpusDialog::HandleSearchTextChanged(const QString& search_text)
{
    // A regex used to check the user's search term against the incoming test string.
    QRegularExpression regex("", QRegularExpression::CaseInsensitiveOption);
    if (!search_text.isEmpty())
    {
        regex.setPattern(".*" + search_text + ".*");
    }

    // Set the filter model regex.
    table_filter_model_->setFilterRegularExpression(regex);

#ifdef _HIGHLIGHT_MATCHING_ROWS
    // Disable this for now as the value we get from the feature
    // is negligible compared to the performance impact - it makes
    // the view sluggish. To be optimized.

    // Highlight the matching rows.
    HighlightMatchingRows(regex);
#endif
}

void RgTargetGpusDialog::ComputeGroupColor(int group_index, QColor& color) const
{
    // Use 25% saturation for a more muted color palette, but 100% value to maintain brightness.
    static const int kBackgroundColorNumHueSteps = 10;
    static const float kBackgroundColorHueStepIncrement = 1.0f / kBackgroundColorNumHueSteps;
    static const float kBackgroundColorSaturation = 0.25f;
    static const float kBackgroundColorValue = 1.0f;

    // If the number of groups exceeds the number of hue steps, reset the hue to the start of the color wheel.
    int color_group_index = group_index % kBackgroundColorNumHueSteps;

    // Compute the group's color based on the group index vs. the total group count.
    float hue = color_group_index * kBackgroundColorHueStepIncrement;
    color.setHsvF(hue, kBackgroundColorSaturation, kBackgroundColorValue);
}

void RgTargetGpusDialog::ConnectSignals()
{
    // Connect the tree's model to the check changed handler.
    bool is_connected = connect(this->ui_.searchTextbox, &QLineEdit::textChanged, this, &RgTargetGpusDialog::HandleSearchTextChanged);
    assert(is_connected);

    // Connect the table's double click handler to toggle the row's check state.
    is_connected = connect(this->ui_.gpuTreeView, &QTreeView::doubleClicked, this, &RgTargetGpusDialog::HandleRowDoubleClicked);
    assert(is_connected);

    // Connect the OK button.
    is_connected = connect(this->ui_.OkPushButton, &QPushButton::clicked, this, &RgTargetGpusDialog::HandleOKButtonClicked);
    assert(is_connected);

    // Connect the Cancel button.
    is_connected = connect(this->ui_.cancelPushButton, &QPushButton::clicked, this, &RgTargetGpusDialog::HandleCancelButtonClicked);
    assert(is_connected);

    // Connect the tree's model to the check changed handler.
    ToggleItemCheckChangedHandler(true);
}

void RgTargetGpusDialog::InitializeCheckedRows(std::vector<std::string> compute_capability_group_names)
{
    // Step through each group and find the first row that belongs to the group.
    for (const std::string& group_name : compute_capability_group_names)
    {
        for (size_t group_index = 0; group_index < capability_groups_.size(); ++group_index)
        {
            const CapabilityGroup& group_data = capability_groups_[group_index];
            if (group_data.group_rows[0].compute_capability.compare(group_name) == 0)
            {
                // Found a table row that belongs to a group that's enabled. Check the row off.
                int row_index = group_data.group_rows[0].row_index;

                QStandardItem* row_check_item = gpu_tree_model_->item(row_index, ColumnType::kSelectedCheckbox);
                row_check_item->setCheckState(Qt::Checked);
                break;
            }
        }
    }
}

bool RgTargetGpusDialog::IsRowVisible(int row_index, const QModelIndex& source_parent, const QRegularExpression& search_filter)
{
    Q_UNUSED(source_parent);

    bool is_filtered = false;

    // Determine the group index for the given row.
    auto table_row_to_group_iter = table_row_index_to_group_index_.find(row_index);
    if (table_row_to_group_iter != table_row_index_to_group_index_.end())
    {
        int group_index = table_row_to_group_iter->second;
        const CapabilityGroup& group_info = capability_groups_[group_index];

        // Check if any of the cells associated with the group match the user's search string.
        for (const TableRow& current_row : group_info.group_rows)
        {
            // Does the architecture name match the filter string?
            is_filtered = IsRowMatchingSearchString(current_row, search_filter);

            // If the row is included, early out and don't check the other rows in the group.
            if (is_filtered)
            {
                break;
            }
        }

    }

    return is_filtered;
}

void RgTargetGpusDialog::HighlightMatchingRows(const QRegularExpression& search_filter)
{
    // Reset the table colors to default background colors.
    SetDefaultTableBackgroundColors();

    // Filter the matching rows and highlight them with a different color.
    // Container for indices of rows containing matching GPU names.
    std::vector<QModelIndex> matching_rows;

    for (int row_index = 0; row_index < table_row_index_to_group_index_.size(); row_index++)
    {
        auto table_row_to_group_iter = table_row_index_to_group_index_.find(row_index);
        if (table_row_to_group_iter != table_row_index_to_group_index_.end())
        {
            int                    group_index = table_row_to_group_iter->second;
            const CapabilityGroup& group_info  = capability_groups_[group_index];

            // Check if any of the cells associated with the group match the user's search string.
            for (const TableRow& current_row : group_info.group_rows)
            {
                bool is_matching = IsRowMatchingSearchString(current_row, search_filter);

                // If this is a matching row, set it as selected.
                if (is_matching)
                {
                    // Store the model index for this row.
                    QModelIndex model_index = gpu_tree_model_->index(current_row.row_index, 0);
                    matching_rows.push_back(model_index);
                }
            }
        }
    }

    // Highlight matching rows.
    for (const QModelIndex& index : matching_rows)
    {
        ui_.gpuTreeView->selectionModel()->select(index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
    }

    // Update the table.
    ui_.gpuTreeView->update();
}

void RgTargetGpusDialog::ToggleItemCheckChangedHandler(bool is_enabled)
{
    if (is_enabled)
    {
        [[maybe_unused]] bool is_connected = connect(gpu_tree_model_, &QStandardItemModel::itemChanged, this, &RgTargetGpusDialog::HandleItemChanged);
        assert(is_connected);
    }
    else
    {
        [[maybe_unused]] bool is_disconnected = disconnect(gpu_tree_model_, &QStandardItemModel::itemChanged, this, &RgTargetGpusDialog::HandleItemChanged);
        assert(is_disconnected);
    }
}

std::string RgTargetGpusDialog::GetItemText(const QStandardItem* item) const
{
    // Extract and return the item text.
    QVariant item_display = item->data(Qt::DisplayRole);
    return item_display.toString().toStdString();
}

void RgTargetGpusDialog::PopulateTableData(std::shared_ptr<RgCliVersionInfo> version_info, const std::string& mode)
{
    // Create the GPU tree model, and fill it.
    gpu_tree_model_ = new QStandardItemModel(0, ColumnType::kCount, this);
    QStandardItem* selected_checkbox = new QStandardItem();
    selected_checkbox->setCheckable(true);

    // Configure the column headers for the GPU table.
    gpu_tree_model_->setHorizontalHeaderItem(ColumnType::kSelectedCheckbox, selected_checkbox);
    gpu_tree_model_->setHorizontalHeaderItem(ColumnType::kProductName, new QStandardItem(kStrTargetGpuProductName));
    gpu_tree_model_->setHorizontalHeaderItem(ColumnType::kArchitecture, new QStandardItem(kStrTargetGpuArchitecture));
    gpu_tree_model_->setHorizontalHeaderItem(ColumnType::kComputeCapability, new QStandardItem(kStrTargetGpuComputeCapability));

    // Set tooltips on the table headers.
    gpu_tree_model_->setHeaderData(ColumnType::kProductName, Qt::Orientation::Horizontal, kStrTableTooltipColumnProductName, Qt::ToolTipRole);
    gpu_tree_model_->setHeaderData(ColumnType::kArchitecture, Qt::Orientation::Horizontal, kStrTableTooltipColumnArchitecture, Qt::ToolTipRole);
    gpu_tree_model_->setHeaderData(ColumnType::kComputeCapability, Qt::Orientation::Horizontal, kStrTableTooltipColumnComputeCapability, Qt::ToolTipRole);

    assert(version_info != nullptr);
    if (version_info != nullptr)
    {
        int current_row_index = 0;
        int group_index = 0;

        // Search the supported hardware data for the given mode's supported hardware list.
        auto current_mode_architectures_iter = version_info->gpu_architectures.find(mode);
        bool is_mode_found = (current_mode_architectures_iter != version_info->gpu_architectures.end());
        assert(is_mode_found);
        if (is_mode_found)
        {
            // Most recent hardware appears at the end of the architectures list. Reverse it so that it appears at the top of the table.
            std::vector<RgGpuArchitecture> architectures = current_mode_architectures_iter->second;
            std::sort(architectures.begin(), architectures.end(), GpuComparator<RgGpuArchitecture>{});

            std::reverse(architectures.begin(), architectures.end());

            // Step through each GPU hardware architecture.
            for (const RgGpuArchitecture& hardware_architecture : architectures)
            {
                const std::string& currentArchitecture = hardware_architecture.architecture_name;

                // Determine how many families are found within the architecture.
                std::vector<RgGpuFamily> gpu_families = hardware_architecture.gpu_families;
                int num_families_in_architecture = static_cast<int>(gpu_families.size());

                // Set the gfx notation as prefix if applicable to the family name.
                // For example, "Tonga" would become "gfx802/Tonga".
                for(int i = 0; i < num_families_in_architecture; i++)
                {
                    std::string gfx_notation;
                    std::stringstream family_name_revised;
                    bool has_gfx_notation = RgUtils::GetGfxNotation(gpu_families[i].family_name, gfx_notation);
                    if (has_gfx_notation && !gfx_notation.empty())
                    {
                        family_name_revised << gfx_notation << "/";
                    }
                    family_name_revised << gpu_families[i].family_name;
                    gpu_families[i].family_name = family_name_revised.str().c_str();
                }

                // Sort the GPU families in reverse order so that for the new AMD
                // GPU naming scheme (gfxABCD) we will have the newer families on top.
                std::sort(gpu_families.rbegin(), gpu_families.rend(), [&](RgGpuFamily familyA,
                    RgGpuFamily familyB) { return familyA.family_name < familyB.family_name; });

                // Step through each family within the architecture.
                for (int family_index = 0; family_index < num_families_in_architecture; family_index++)
                {
                    // Create a copy of the family info and sort by product name.
                    RgGpuFamily family_sorted_by_product_name = gpu_families[family_index];
                    const std::string& family_name = family_sorted_by_product_name.family_name;

                    // Sort the product name column alphabetically.
                    std::sort(family_sorted_by_product_name.product_names.begin(), family_sorted_by_product_name.product_names.end());

                    capability_groups_.resize(group_index + 1);
                    CapabilityGroup& capabilityGroup = capability_groups_[group_index];

                    // Create a node for each GPU's product name.
                    int num_gpu_names = static_cast<int>(family_sorted_by_product_name.product_names.size());
                    for (int product_index = 0; product_index < num_gpu_names; product_index++)
                    {
                        const std::string& product_name = family_sorted_by_product_name.product_names[product_index];

                        // Add a new row to the model by increasing the count by 1.
                        gpu_tree_model_->setRowCount(current_row_index + 1);

                        // Create a checkbox item in the first column.
                        QModelIndex checkbox_column_index = gpu_tree_model_->index(current_row_index, ColumnType::kSelectedCheckbox);
                        if (checkbox_column_index.isValid())
                        {
                            QStandardItem* checkbox_item = gpu_tree_model_->itemFromIndex(checkbox_column_index);
                            checkbox_item->setCheckState(Qt::Unchecked);
                            checkbox_item->setCheckable(true);
                        }

                        TableRow num_row = { current_row_index, currentArchitecture, product_name, family_name };
                        capabilityGroup.group_rows.push_back(num_row);

                        // Set the data for each column in the new row.
                        SetTableIndexData(current_row_index, ColumnType::kProductName, num_row.product_name);
                        SetTableIndexData(current_row_index, ColumnType::kArchitecture, num_row.architecture);
                        SetTableIndexData(current_row_index, ColumnType::kComputeCapability, num_row.compute_capability);

                        table_row_index_to_group_index_[current_row_index] = group_index;

                        // Update the row index for other GPUs.
                        current_row_index++;
                    }

                    group_index++;
                }
            }
        }
        else
        {
            // Show an error string telling the user that the given mode seems to be invalid.
            std::stringstream error_string;
            error_string << kStrErrCannotLoadSupportedGpusListForModeA;
            error_string << mode;
            error_string << kStrErrCannotLoadSupportedGpusListForModeB;
            RgUtils::ShowErrorMessageBox(error_string.str().c_str(), parent_);
        }
    }

    // Generate the colors for each group.
    SetDefaultTableBackgroundColors();

    // Filter the tree model with the proxy model.
    table_filter_model_ = new RgTableFilterProxyModel(this);
    table_filter_model_->setSourceModel(gpu_tree_model_);

    // Display the filtered model in the QTreeView.
    ui_.gpuTreeView->setModel(table_filter_model_);
    QtCommon::QtUtils::AutoAdjustTableColumns(ui_.gpuTreeView, 32, 10);
}

void RgTargetGpusDialog::SetIsGroupChecked(int group_index, bool is_checked)
{
    bool is_valid_index = (group_index >= 0 && group_index < capability_groups_.size());
    assert(is_valid_index);
    if (is_valid_index)
    {
        CapabilityGroup& target_group = capability_groups_[group_index];
        for (auto current_row : target_group.group_rows)
        {
            QStandardItem* check_item = gpu_tree_model_->item(current_row.row_index, ColumnType::kSelectedCheckbox);
            check_item->setCheckState(is_checked ? Qt::Checked : Qt::Unchecked);
        }
    }
}

void RgTargetGpusDialog::SetTableIndexData(int row, int column, const std::string& string_data)
{
    // Generate an index and verify that it's valid.
    QModelIndex model_index = gpu_tree_model_->index(row, column);
    bool is_valid = model_index.isValid();
    assert(is_valid);
    if (is_valid)
    {
        // Set the data at the correct index in the GPU tree.
        gpu_tree_model_->setData(model_index, string_data.c_str());
    }
}

void RgTargetGpusDialog::SetTableBackgroundColor(int row, const QColor& background_color)
{
    // Generate an index and verify that it's valid.
    for (int column_index = ColumnType::kSelectedCheckbox; column_index < ColumnType::kCount; ++column_index)
    {
        QModelIndex model_index = gpu_tree_model_->index(row, column_index);
        bool is_valid = model_index.isValid();
        assert(is_valid);
        if (is_valid)
        {
            // Set the data at the correct index in the GPU tree.
            gpu_tree_model_->setData(model_index, background_color, Qt::BackgroundRole);
        }
    }
}

void RgTargetGpusDialog::ToggleOKButtonEnabled(bool is_ok_enabled)
{
    ui_.OkPushButton->setEnabled(is_ok_enabled);
}

void RgTargetGpusDialog::ToggleRowChecked(const QModelIndex& source_model_index)
{
    // Find the group index associated with the product row.
    auto group_index_iter = table_row_index_to_group_index_.find(source_model_index.row());
    if (group_index_iter != table_row_index_to_group_index_.end())
    {
        assert(table_filter_model_ != nullptr);
        if (table_filter_model_ != nullptr)
        {
            // Find the filtered index for the checkbox item in the row.
            QAbstractItemModel* source_item_model = table_filter_model_->sourceModel();
            assert(source_item_model != nullptr);
            if (source_item_model != nullptr)
            {
                QModelIndex checkbox_column_filtered_index = source_item_model->index(source_model_index.row(), ColumnType::kSelectedCheckbox);
                bool is_filtered_index_valid = checkbox_column_filtered_index.isValid();
                assert(is_filtered_index_valid);
                if (is_filtered_index_valid)
                {
                    assert(gpu_tree_model_ != nullptr);
                    if (gpu_tree_model_ != nullptr)
                    {
                        // Retrieve a pointer to the checkbox item in the table.
                        QStandardItem* clicked_item = gpu_tree_model_->itemFromIndex(checkbox_column_filtered_index);
                        assert(clicked_item != nullptr);
                        if (clicked_item != nullptr)
                        {
                            // Determine the current check state for the row's checkbox item.
                            int group_index = group_index_iter->second;
                            bool check_state = (clicked_item->checkState() == Qt::Checked) ? true : false;

                            // Toggle the checkbox for the entire group of products.
                            SetIsGroupChecked(group_index, !check_state);
                        }
                    }
                }
            }
        }
    }
}

void RgTargetGpusDialog::SetDefaultTableBackgroundColors()
{
    for (int group_index = 0; group_index < static_cast<int>(capability_groups_.size()); ++group_index)
    {
        CapabilityGroup& group_info = capability_groups_[group_index];

        // Compute the group's color based on the group index.
        ComputeGroupColor(group_index, group_info.color);

        // Set the table background color for each row in the group.
        for (size_t row_index = 0; row_index < group_info.group_rows.size(); ++row_index)
        {
            auto group_row = group_info.group_rows[row_index];
            SetTableBackgroundColor(group_row.row_index, group_info.color);

            // Set each row to deselected.
            if (ui_.gpuTreeView->selectionModel() != nullptr)
            {
                QModelIndex model_index = gpu_tree_model_->index(static_cast<int>(row_index), 0);
                ui_.gpuTreeView->selectionModel()->select(model_index, QItemSelectionModel::Deselect | QItemSelectionModel::Rows);
            }
        }
    }
}

bool RgTargetGpusDialog::IsRowMatchingSearchString(const TableRow& current_row, const QRegularExpression& search_filter)
{
    bool is_matching = false;

    // Does the architecture name match the filter string?
    is_matching = search_filter.match(current_row.architecture.c_str()).hasMatch();
    if (!is_matching)
    {
        // Does the compute capability match the filter string?
        is_matching = search_filter.match(current_row.compute_capability.c_str()).hasMatch();
        if (!is_matching)
        {
            // Does the product name match the filter string?
            is_matching = search_filter.match(current_row.product_name.c_str()).hasMatch();
        }
    }

    return is_matching;
}

void RgTargetGpusDialog::HandleOKButtonClicked(bool /* checked */)
{
    this->accept();
}

void RgTargetGpusDialog::HandleCancelButtonClicked(bool /* checked */)
{
    this->reject();
}

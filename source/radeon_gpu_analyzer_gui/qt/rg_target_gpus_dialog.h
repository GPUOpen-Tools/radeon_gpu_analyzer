//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for the dialog responsible for displaying the supported GPUs tree.
//=============================================================================
#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_TARGET_GPUS_DIALOG_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_TARGET_GPUS_DIALOG_H_

// C++.
#include <memory>

// Local.
#include "ui_rg_target_gpus_dialog.h"

// Forward declarations.
class QStandardItem;
class QStandardItemModel;
class QDialog;
class QRegularExpression;
class RgTableFilterProxyModel;
struct RgCliVersionInfo;

// An eventFilter object used to watch for key press events within the GPU treeview.
class RgTreeEventFilter : public QObject
{
    Q_OBJECT
public:
    RgTreeEventFilter(QObject* parent) : QObject(parent) {}
    virtual ~RgTreeEventFilter() = default;

signals:
    // A signal invoked when the selected row has been activated.
    void RowActivated();

protected:
    // The overridden event filter.
    virtual bool eventFilter(QObject* object, QEvent* event) override;
};

// The dialog responsible for displaying the supported GPUs tree.
class RgTargetGpusDialog : public QDialog
{
    Q_OBJECT
public:
    RgTargetGpusDialog(const QString& selected_gpus, QWidget* parent);
    virtual ~RgTargetGpusDialog() = default;

    // Retrieve the list of family names that the user selected within the GPU tree dialog.
    std::vector<std::string> GetSelectedCapabilityGroups() const;

    // Is the given row visible when filtered with the given search string?
    bool IsRowVisible(int row_index, const QModelIndex& source_parent, const QRegularExpression& search_filter);

public slots:
    // Handler invoked when the user changes the check state for a row.
    void HandleItemChanged(QStandardItem* item);

    // Handler invoked when the user toggles a single row using the Spacebar.
    void HandleRowToggled();

    // Handler invoked when the user double-clicks on a row in the GPU table.
    void HandleRowDoubleClicked(const QModelIndex& index);

    // Handler invoked when the search box's text has changed.
    void HandleSearchTextChanged(const QString& search_text);

private slots:
    // Handler for when the user clicks the OK button.
    void HandleOKButtonClicked(bool /* checked */);

    // Handler for when the user clicks the Cancel button.
    void HandleCancelButtonClicked(bool /* checked */);

private:
    // The types of columns being presented by the GPU table.
    enum ColumnType
    {
        kSelectedCheckbox,
        kProductName,
        kArchitecture,
        kComputeCapability,
        kCount
    };

    // A single row of data in the GPU table.
    struct TableRow
    {
        int row_index;
        std::string architecture;
        std::string product_name;
        std::string compute_capability;
    };

    // A compute capability group has a color and a number of rows associated with the group.
    struct CapabilityGroup
    {
        QColor color;
        std::vector<TableRow> group_rows;
    };

    // Compute the color for the capability group with the given index.
    void ComputeGroupColor(int group_index, QColor& color) const;

    // Connect signals to slots within the dialog.
    void ConnectSignals();

    // Initialize the checkbox for each row in the GPU table.
    void InitializeCheckedRows(std::vector<std::string> compute_capability_group_names);

    // Toggle the enabledness of the itemChanged handler for the GPU table widget.
    void ToggleItemCheckChangedHandler(bool is_enabled);

    // Retrieve the text displayed by the given item.
    std::string GetItemText(const QStandardItem* item) const;

    // Populate the table with available GPU data.
    void PopulateTableData(std::shared_ptr<RgCliVersionInfo> version_info, const std::string& mode);

    // Indicate whether a group is checked.
    void SetIsGroupChecked(int group_index, bool is_checked);

    // Set the data to insert at the given position within the table.
    void SetTableIndexData(int row, int column, const std::string& string_data);

    // Set the background color for the given table row.
    void SetTableBackgroundColor(int row, const QColor& background_color);

    // Toggle the enabledness of the Ok button.
    void ToggleOKButtonEnabled(bool is_ok_enabled);

    // Toggle the checkbox in the row specified with the given index.
    // Toggling the row will cause the entire capability group to be toggled.
    void ToggleRowChecked(const QModelIndex& index);

    // Highlight matching rows.
    void HighlightMatchingRows(const QRegularExpression& search_text);

    // Set the default table background colors.
    void SetDefaultTableBackgroundColors();

    // Check if the group rows match the search string.
    bool IsRowMatchingSearchString(const TableRow& current_row, const QRegularExpression& search_filter);

    // The model holding the data for the GPU tree view.
    QStandardItemModel* gpu_tree_model_ = nullptr;

    // The model used to search and filter through the table.
    RgTableFilterProxyModel* table_filter_model_ = nullptr;

    // A map of table row index to compute capability group index.
    std::map<int, int> table_row_index_to_group_index_;

    // A map of group index to group data.
    std::vector<CapabilityGroup> capability_groups_;

    // A pointer to the parent object.
    QWidget* parent_ = nullptr;

    // The interface for the RgTargetGpusDialog.
    Ui::RgTargetGpusDialog ui_;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_TARGET_GPUS_DIALOG_H_

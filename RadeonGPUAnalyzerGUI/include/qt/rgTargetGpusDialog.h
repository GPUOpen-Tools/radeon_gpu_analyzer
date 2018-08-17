#pragma once

// C++.
#include <memory>

// Local.
#include "ui_rgTargetGpusDialog.h"

// Forward declarations.
class QStandardItem;
class QStandardItemModel;
class QDialog;
class rgTableFilterProxyModel;
struct rgCliVersionInfo;

// The dialog responsible for displaying the supported GPUs tree.
class rgTargetGpusDialog : public QDialog
{
    Q_OBJECT
public:
    rgTargetGpusDialog(const QString& selectedGPUs, QWidget* pParent);
    virtual ~rgTargetGpusDialog() = default;

    // Retrieve the list of family names that the user selected within the GPU tree dialog.
    std::vector<std::string> GetSelectedCapabilityGroups() const;

    // Is the given row visible when filtered with the given search string?
    bool IsRowVisible(int rowIndex, const QModelIndex& sourceParent, const QRegExp& searchFilter);

public slots:
    // Handler invoked when the user changes the check state for a row.
    void HandleItemChanged(QStandardItem* pItem);

    // Handler invoked when the user double-clicks on a row in the GPU table.
    void HandleRowDoubleClicked(const QModelIndex& index);

    // Handler invoked when the search box's text has changed.
    void HandleSearchTextChanged(const QString& searchText);

private:
    // The types of columns being presented by the GPU table.
    enum ColumnType
    {
        SelectedCheckbox,
        Architecture,
        ComputeCapability,
        ProductName,
        Count
    };

    // A single row of data in the GPU table.
    struct TableRow
    {
        int m_rowIndex;
        std::string m_architecture;
        std::string m_productName;
        std::string m_computeCapability;
    };

    // A compute capability group has a color and a number of rows associated with the group.
    struct CapabilityGroup
    {
        std::vector<TableRow> m_groupRows;
    };

    // Compute the color for the capability group with the given index.
    void ComputeGroupColors(std::map<std::string, float>& architectureHues, std::map<std::string, float>& groupSaturations) const;

    // Connect signals to slots within the dialog.
    void ConnectSignals();

    // Initialize the checkbox for each row in the GPU table.
    void InitializeCheckedRows(std::vector<std::string> computeCapabilityGroupNames);

    // Toggle the enabledness of the itemChanged handler for the GPU table widget.
    void ToggleItemCheckChangedHandler(bool isEnabled);

    // Retrieve the text displayed by the given item.
    std::string GetItemText(const QStandardItem* pItem) const;

    // Populate the table with available GPU data.
    void PopulateTableData(std::shared_ptr<rgCliVersionInfo> pVersionInfo, const std::string& mode);

    // Set the check state of all GPUs in the given group.
    void SetIsGroupChecked(int groupIndex, bool isChecked);

    // Set the data to insert at the given position within the table.
    void SetTableIndexData(int row, int column, const std::string& stringData);

    // Set the background color for the given table row.
    void SetTableBackgroundColor(int row, const QColor& backgroundColor);

    // Toggle the enabledness of the Ok button.
    void ToggleOKButtonEnabled(bool isOkEnabled);

    // Highlight matching rows.
    void HighlightMatchingRows(const QRegExp& searchText);

    // Set the default table background colors.
    void SetDefaultTableBackgroundColors();

    // Set the row selected in gpu tree view model.
    void SetRowSelected(QModelIndex modelIndex);

    // Set the row deselected in gpu tree view model.
    void SetRowDeselected(int rowNumber);

    // Check if the group rows match the search string.
    bool IsRowMatchingSearchString(const TableRow& currentRow, const QRegExp searchFilter);

    // The model holding the data for the GPU tree view.
    QStandardItemModel* m_pGpuTreeModel = nullptr;

    // The model used to search and filter through the table.
    rgTableFilterProxyModel* m_pTableFilterModel = nullptr;

    // A map of table row index to compute capability group index.
    std::map<int, int> m_tableRowIndexToGroupIndex;

    // A vector of group index to group data.
    std::vector<CapabilityGroup> m_capabilityGroups;

    // A pointer to the parent object.
    QWidget* m_pParent = nullptr;

    // The interface for the rgTargetGpusDialog.
    Ui::rgTargetGpusDialog ui;
};
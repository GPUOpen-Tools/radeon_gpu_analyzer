// C++.
#include <cassert>
#include <sstream>

// Qt.
#include <QDialog>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QVariant>

// Infra.
#include <QtCommon/Util/QtUtil.h>

// Local.
#include <RadeonGPUAnalyzerGUI/include/qt/rgTargetGpusDialog.h>
#include <RadeonGPUAnalyzerGUI/include/rgCliLauncher.h>
#include <RadeonGPUAnalyzerGUI/include/rgStringConstants.h>
#include <RadeonGPUAnalyzerGUI/include/rgUtils.h>
#include <RadeonGPUAnalyzerGUI/include/rgXMLCliVersionInfo.h>

// A class used to filter the GPU table widget.
class rgTableFilterProxyModel : public QSortFilterProxyModel
{
public:
    rgTableFilterProxyModel(rgTargetGpusDialog* pParentDialog = nullptr) : QSortFilterProxyModel(pParentDialog), m_pParentDialog(pParentDialog) {}
    virtual ~rgTableFilterProxyModel() = default;

protected:
    // A row-filtering predicate responsible for matching rows against the user's search text.
    virtual bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override
    {
        bool isFiltered = false;

        // Filter each row using rgFilterProxyModel's regex.
        if (filterRegExp().isEmpty() == false)
        {
            isFiltered = m_pParentDialog->IsRowVisible(sourceRow, sourceParent, filterRegExp());
        }
        else
        {
            // Invoke the default filtering on the row.
            isFiltered = QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
        }

        return isFiltered;
    }

private:
    // The parent dialog whose table is being filtered.
    rgTargetGpusDialog* m_pParentDialog = nullptr;
};

rgTargetGpusDialog::rgTargetGpusDialog(const QString& selectedGPUs, QWidget* pParent) :
    QDialog(pParent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
    m_pParent(pParent)
{
    ui.setupUi(this);

    // Double-clicking an item within the QTreeView will toggle its check state- not expand/collapse the children.
    ui.gpuTreeView->setExpandsOnDoubleClick(false);

    // This prevents the dotted gray border around cells that appears upon selection.
    ui.gpuTreeView->setFocusPolicy(Qt::NoFocus);

    // Disable editing cells within the table.
    ui.gpuTreeView->setEditTriggers(QAbstractItemView::EditTrigger::NoEditTriggers);

    // Don't allow the user to select rows themselves.
    ui.gpuTreeView->setSelectionMode(QTreeView::NoSelection);

    // Use the cached CLI version info structure to populate the GPU tree.
    std::shared_ptr<rgCliVersionInfo> pVersionInfo = rgConfigManager::Instance().GetVersionInfo();
    assert(pVersionInfo != nullptr);
    if (pVersionInfo != nullptr)
    {
        const std::string& currentMode = rgConfigManager::Instance().GetCurrentMode();
        PopulateTableData(pVersionInfo, currentMode);
    }

    // Connect the target GPUs dialog signals.
    ConnectSignals();

    // Set window flags.
    setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint);

    // Split the incoming string of comma-separated families into a vector.
    std::vector<std::string> selectedGPUsVector;
    rgUtils::splitString(selectedGPUs.toStdString(), rgConfigManager::RGA_LIST_DELIMITER, selectedGPUsVector);

    // Update the state of the OK button based on the number of selected GPUs.
    ToggleOKButtonEnabled(!selectedGPUsVector.empty());

    // Initialize the model item's initial checkState for each row.
    InitializeCheckedRows(selectedGPUsVector);

    // Set the cursor for the treeview to be pointing hand cursor.
    ui.gpuTreeView->setCursor(Qt::PointingHandCursor);
}

std::vector<std::string> rgTargetGpusDialog::GetSelectedCapabilityGroups() const
{
    std::vector<std::string> selectedFamilies;

    // Step through all rows in the table to find rows that are checked.
    for (int rowIndex = 0; rowIndex < m_pGpuTreeModel->rowCount(); ++rowIndex)
    {
        QStandardItem* pRowCheckItem = m_pGpuTreeModel->item(rowIndex, ColumnType::SelectedCheckbox);
        if (pRowCheckItem != nullptr)
        {
            bool isRowChecked = pRowCheckItem->checkState();
            if (isRowChecked)
            {
                // If the row is checked, find the compute capability group associated with the row.
                auto rowToGroupIndexIter = m_tableRowIndexToGroupIndex.find(rowIndex);
                if (rowToGroupIndexIter != m_tableRowIndexToGroupIndex.end())
                {
                    int groupIndex = rowToGroupIndexIter->second;
                    bool isValidIndex = (groupIndex >= 0 && groupIndex < m_capabilityGroups.size());
                    assert(isValidIndex);
                    if (isValidIndex)
                    {
                        const CapabilityGroup& groupInfo = m_capabilityGroups[groupIndex];

                        // Retrieve the compute capability string from the first row of the group.
                        const std::string& groupName = groupInfo.m_groupRows[0].m_computeCapability;
                        auto isAlreadyAddedIter = std::find(selectedFamilies.begin(), selectedFamilies.end(), groupName);
                        if (isAlreadyAddedIter == selectedFamilies.end())
                        {
                            selectedFamilies.push_back(groupName);
                        }
                    }
                }
            }
        }
    }


    return selectedFamilies;
}

void rgTargetGpusDialog::HandleItemChanged(QStandardItem* pItem)
{
    // Disable the item check changed handler to avoid a recursive overflow.
    ToggleItemCheckChangedHandler(false);

    // Update the check state for the selected group's rows.
    QModelIndex selectedItemIndex = pItem->index();
    if (selectedItemIndex.isValid())
    {
        int selectedRow = selectedItemIndex.row();
        auto groupIndexIter = m_tableRowIndexToGroupIndex.find(selectedRow);
        if (groupIndexIter != m_tableRowIndexToGroupIndex.end())
        {
            int groupIndex = groupIndexIter->second;
            bool isChecked = (pItem->checkState() == Qt::Checked);
            SetIsGroupChecked(groupIndex, isChecked);
        }
    }

    // Only allow the user to confirm their changes if there's at least one family selected.
    std::vector<std::string> selectedFamilies = GetSelectedCapabilityGroups();
    bool isOkEnabled = !selectedFamilies.empty();

    // Update the enabledness of the dialog's OK button.
    ToggleOKButtonEnabled(isOkEnabled);

    // Re-enable the item check changed handler.
    ToggleItemCheckChangedHandler(true);
}

void rgTargetGpusDialog::HandleRowDoubleClicked(const QModelIndex& index)
{
    // Find the group index associated with the product row.
    auto groupIndexIter = m_tableRowIndexToGroupIndex.find(index.row());
    if (groupIndexIter != m_tableRowIndexToGroupIndex.end())
    {
        // Find the filtered index for the checkbox item in the row.
        QModelIndex checkboxColumnFilteredIndex = m_pTableFilterModel->index(index.row(), ColumnType::SelectedCheckbox);
        bool isFilteredIndexValid = checkboxColumnFilteredIndex.isValid();
        assert(isFilteredIndexValid);
        if (isFilteredIndexValid)
        {
            // Convert the filtered index to the source model index.
            QModelIndex sourceModelIndex = m_pTableFilterModel->mapToSource(checkboxColumnFilteredIndex);
            bool isSourceModelIndexValid = sourceModelIndex.isValid();
            assert(isSourceModelIndexValid);
            if (isSourceModelIndexValid)
            {
                // Retrieve a pointer to the checkbox item in the table.
                QStandardItem* pClickedItem = m_pGpuTreeModel->itemFromIndex(sourceModelIndex);
                assert(pClickedItem != nullptr);
                if (pClickedItem != nullptr)
                {
                    // Determine the current check state for the row's checkbox item.
                    int groupIndex = groupIndexIter->second;
                    CapabilityGroup& group = m_capabilityGroups[groupIndex];
                    bool checkState = (pClickedItem->checkState() == Qt::Checked) ? true : false;

                    // Toggle the checkbox for the entire group of products.
                    SetIsGroupChecked(groupIndex, !checkState);
                }
            }
        }
    }
}

void rgTargetGpusDialog::HandleSearchTextChanged(const QString& searchText)
{
    // A regex used to check the user's search term against the incoming test string.
    QRegExp regex("", Qt::CaseInsensitive);
    if (!searchText.isEmpty())
    {
        regex.setPattern(".*" + searchText + ".*");
    }

    // Set the filter model regex.
    m_pTableFilterModel->setFilterRegExp(regex);

    // Hightlight the matching rows.
    HighlightMatchingRows(regex);
}

void rgTargetGpusDialog::ComputeGroupColors(std::map<std::string, float>& architectureHues, std::map<std::string, float>& groupSaturations) const
{
    // The starting saturation for all groups within an architecture. Smaller values will result in lighter initial colors.
    static const float s_BACKGROUND_COLOR_BASE_SATURATION = 0.25f;

    // The overall saturation range to cover among all compute capability groups within an architecture.
    // To work correctly, (s_BACKGROUND_COLOR_BASE_SATURATION + s_BACKGROUND_COLOR_SATURATION_RANGE) must be <= 1.0.
    static const float s_BACKGROUND_COLOR_SATURATION_RANGE = 0.7f;

    // Use the cached CLI version info structure to populate the GPU tree.
    std::shared_ptr<rgCliVersionInfo> pVersionInfo = rgConfigManager::Instance().GetVersionInfo();
    assert(pVersionInfo != nullptr);
    if (pVersionInfo != nullptr)
    {
        // Retrieve the list of supported GPU architectures for the current mode.
        const std::string& currentMode = rgConfigManager::Instance().GetCurrentMode();
        auto currentModeArchitecturesIter = pVersionInfo->m_gpuArchitectures.find(currentMode);
        if (currentModeArchitecturesIter != pVersionInfo->m_gpuArchitectures.end())
        {
            const std::vector<rgGpuArchitecture>& modeArchitectures = currentModeArchitecturesIter->second;

            size_t numArchitectures = modeArchitectures.size();

            // Determine the overall hue for each architecture group.
            float hueIncrementPerArchitecture = 1.0f / static_cast<float>(numArchitectures);

            for (size_t architectureIndex = 0; architectureIndex < numArchitectures; ++architectureIndex)
            {
                // The base hue for this architecture.
                float architectureHue = hueIncrementPerArchitecture * architectureIndex;

                const std::string& architectureName = modeArchitectures[architectureIndex].m_architectureName;
                architectureHues[architectureName] = architectureHue;

                const rgGpuArchitecture& currentArchitecture = modeArchitectures[architectureIndex];

                const std::vector<rgGpuFamily>& architectureFamilies = currentArchitecture.m_gpuFamilies;

                float groupSaturationIncrement = s_BACKGROUND_COLOR_SATURATION_RANGE / static_cast<float>(architectureFamilies.size());

                for (size_t familyIndex = 0; familyIndex < architectureFamilies.size(); ++familyIndex)
                {
                    const std::string& familyName = architectureFamilies[familyIndex].m_familyName;

                    float saturation = s_BACKGROUND_COLOR_BASE_SATURATION + (familyIndex * groupSaturationIncrement);
                    groupSaturations[familyName] = saturation;
                }
            }
        }
    }

}

void rgTargetGpusDialog::ConnectSignals()
{
    // Connect the tree's model to the check changed handler.
    bool isConnected = connect(this->ui.searchTextbox, &QLineEdit::textChanged, this, &rgTargetGpusDialog::HandleSearchTextChanged);
    assert(isConnected);

    // Connect the table's double click handler to toggle the row's check state.
    isConnected = connect(this->ui.gpuTreeView, &QTreeView::doubleClicked, this, &rgTargetGpusDialog::HandleRowDoubleClicked);
    assert(isConnected);

    // Connect the tree's model to the check changed handler.
    ToggleItemCheckChangedHandler(true);
}

void rgTargetGpusDialog::InitializeCheckedRows(std::vector<std::string> computeCapabilityGroupNames)
{
    // Step through each group and find the first row that belongs to the group.
    for (const std::string& groupName : computeCapabilityGroupNames)
    {
        for (size_t groupIndex = 0; groupIndex < m_capabilityGroups.size(); ++groupIndex)
        {
            const CapabilityGroup& groupData = m_capabilityGroups[groupIndex];
            if (groupData.m_groupRows[0].m_computeCapability.compare(groupName) == 0)
            {
                // Found a table row that belongs to a group that's enabled. Check the row off.
                int rowIndex = groupData.m_groupRows[0].m_rowIndex;

                QStandardItem* pRowCheckItem = m_pGpuTreeModel->item(rowIndex, ColumnType::SelectedCheckbox);
                pRowCheckItem->setCheckState(Qt::Checked);
                break;
            }
        }
    }
}

bool rgTargetGpusDialog::IsRowVisible(int rowIndex, const QModelIndex& sourceParent, const QRegExp& searchFilter)
{
    bool isFiltered = false;

    // Determine the group index for the given row.
    auto tableRowToGroupIter = m_tableRowIndexToGroupIndex.find(rowIndex);
    if (tableRowToGroupIter != m_tableRowIndexToGroupIndex.end())
    {
        int groupIndex = tableRowToGroupIter->second;
        const CapabilityGroup& groupInfo = m_capabilityGroups[groupIndex];

        // Check if any of the cells associated with the group match the user's search string.
        for (const TableRow& currentRow : groupInfo.m_groupRows)
        {
            // Does the architecture name match the filter string?
            isFiltered = IsRowMatchingSearchString(currentRow, searchFilter);

            // If the row is included, early out and don't check the other rows in the group.
            if (isFiltered)
            {
                break;
            }
        }

    }

    return isFiltered;
}

void rgTargetGpusDialog::HighlightMatchingRows(const QRegExp& searchFilter)
{
    // Reset the table colors to default background colors.
    SetDefaultTableBackgroundColors();

    // Filter the matching rows and highlight them with a different color.
    if (!searchFilter.isEmpty())
    {
        for (int rowIndex = 0; rowIndex < m_tableRowIndexToGroupIndex.size(); rowIndex++)
        {
            auto tableRowToGroupIter = m_tableRowIndexToGroupIndex.find(rowIndex);
            if (tableRowToGroupIter != m_tableRowIndexToGroupIndex.end())
            {
                int groupIndex = tableRowToGroupIter->second;
                const CapabilityGroup& groupInfo = m_capabilityGroups[groupIndex];

                // Check if any of the cells associated with the group match the user's search string.
                for (const TableRow& currentRow : groupInfo.m_groupRows)
                {
                    bool isMatching = IsRowMatchingSearchString(currentRow, searchFilter);

                    // If this is a matching row, set it as selected.
                    if (isMatching)
                    {
                        // Get the model index for this row.
                        QModelIndex modelIndex = m_pGpuTreeModel->index(currentRow.m_rowIndex, ColumnType::ProductName);

                        // Highlight this row.
                        SetRowSelected(m_pTableFilterModel->mapFromSource(modelIndex));
                    }
                }
            }
        }
    }
}

void rgTargetGpusDialog::ToggleItemCheckChangedHandler(bool isEnabled)
{
    if (isEnabled)
    {
        bool isConnected = connect(m_pGpuTreeModel, &QStandardItemModel::itemChanged, this, &rgTargetGpusDialog::HandleItemChanged);
        assert(isConnected);
    }
    else
    {
        bool isDisconnected = disconnect(m_pGpuTreeModel, &QStandardItemModel::itemChanged, this, &rgTargetGpusDialog::HandleItemChanged);
        assert(isDisconnected);
    }
}

std::string rgTargetGpusDialog::GetItemText(const QStandardItem* pItem) const
{
    // Extract and return the item text.
    QVariant itemDisplay = pItem->data(Qt::DisplayRole);
    return itemDisplay.toString().toStdString();
}

void rgTargetGpusDialog::PopulateTableData(std::shared_ptr<rgCliVersionInfo> pVersionInfo, const std::string& mode)
{
    // Create the GPU tree model, and fill it.
    m_pGpuTreeModel = new QStandardItemModel(0, ColumnType::Count, this);
    QStandardItem* pSelectedCheckbox = new QStandardItem();
    pSelectedCheckbox->setCheckable(true);

    // Configure the column headers for the GPU table.
    m_pGpuTreeModel->setHorizontalHeaderItem(ColumnType::SelectedCheckbox,  pSelectedCheckbox);
    m_pGpuTreeModel->setHorizontalHeaderItem(ColumnType::Architecture,      new QStandardItem(STR_TARGET_GPU_ARCHITECTURE));
    m_pGpuTreeModel->setHorizontalHeaderItem(ColumnType::ComputeCapability, new QStandardItem(STR_TARGET_GPU_COMPUTE_CAPABILITY));
    m_pGpuTreeModel->setHorizontalHeaderItem(ColumnType::ProductName,       new QStandardItem(STR_TARGET_GPU_PRODUCT_NAME));

    // Set tooltips on the table headers.
    m_pGpuTreeModel->setHeaderData(ColumnType::Architecture,        Qt::Orientation::Horizontal, STR_TABLE_TOOLTIP_COLUMN_ARCHITECTURE,         Qt::ToolTipRole);
    m_pGpuTreeModel->setHeaderData(ColumnType::ComputeCapability,   Qt::Orientation::Horizontal, STR_TABLE_TOOLTIP_COLUMN_COMPUTE_CAPABILITY,   Qt::ToolTipRole);
    m_pGpuTreeModel->setHeaderData(ColumnType::ProductName,         Qt::Orientation::Horizontal, STR_TABLE_TOOLTIP_COLUMN_PRODUCT_NAME,         Qt::ToolTipRole);

    assert(pVersionInfo != nullptr);
    if (pVersionInfo != nullptr)
    {
        int currentRowIndex = 0;
        int groupIndex = 0;

        // Search the supported hardware data for the given mode's supported hardware list.
        auto currentModeArchitecturesIter = pVersionInfo->m_gpuArchitectures.find(mode);
        bool isModeFound = (currentModeArchitecturesIter != pVersionInfo->m_gpuArchitectures.end());
        assert(isModeFound);
        if (isModeFound)
        {
            // Most recent hardware appears at the end of the architectures list. Reverse it so that it appears at the top of the table.
            std::vector<rgGpuArchitecture> architectures = currentModeArchitecturesIter->second;
            std::reverse(architectures.begin(), architectures.end());

            // Step through each GPU hardware architecture.
            for (const rgGpuArchitecture& hardwareArchitecture : architectures)
            {
                const std::string& currentArchitecture = hardwareArchitecture.m_architectureName;

                // Determine how many families are found within the architecture.
                const std::vector<rgGpuFamily>& gpuFamilies = hardwareArchitecture.m_gpuFamilies;
                int numFamiliesInArchitecture = static_cast<int>(gpuFamilies.size());

                // Step through each family within the architecture.
                for (int familyIndex = 0; familyIndex < numFamiliesInArchitecture; familyIndex++)
                {
                    // Create a copy of the family info and sort by product name.
                    rgGpuFamily familySortedByProductName = gpuFamilies[familyIndex];
                    const std::string& familyName = familySortedByProductName.m_familyName;

                    // Sort the product name column alphabetically.
                    std::sort(familySortedByProductName.m_productNames.begin(), familySortedByProductName.m_productNames.end());

                    m_capabilityGroups.resize(groupIndex + 1);
                    CapabilityGroup& capabilityGroup = m_capabilityGroups[groupIndex];

                    // Create a node for each GPU's product name.
                    int numGPUNames = static_cast<int>(familySortedByProductName.m_productNames.size());
                    for (int productIndex = 0; productIndex < numGPUNames; productIndex++)
                    {
                        const std::string& productName = familySortedByProductName.m_productNames[productIndex];

                        // Add a new row to the model by increasing the count by 1.
                        m_pGpuTreeModel->setRowCount(currentRowIndex + 1);

                        // Create a checkbox item in the first column.
                        QModelIndex checkboxColumnIndex = m_pGpuTreeModel->index(currentRowIndex, ColumnType::SelectedCheckbox);
                        if (checkboxColumnIndex.isValid())
                        {
                            QStandardItem* pCheckboxItem = m_pGpuTreeModel->itemFromIndex(checkboxColumnIndex);
                            pCheckboxItem->setCheckState(Qt::Unchecked);
                            pCheckboxItem->setCheckable(true);
                        }

                        TableRow newRow = { currentRowIndex, currentArchitecture, productName, familyName};
                        capabilityGroup.m_groupRows.push_back(newRow);

                        // Set the data for each column in the new row.
                        SetTableIndexData(currentRowIndex, ColumnType::Architecture,      newRow.m_architecture);
                        SetTableIndexData(currentRowIndex, ColumnType::ComputeCapability, newRow.m_computeCapability);
                        SetTableIndexData(currentRowIndex, ColumnType::ProductName,       newRow.m_productName);

                        m_tableRowIndexToGroupIndex[currentRowIndex] = groupIndex;

                        // Update the row index for other GPUs.
                        currentRowIndex++;
                    }

                    groupIndex++;
                }
            }
        }
        else
        {
            // Show an error string telling the user that the given mode seems to be invalid.
            std::stringstream errorString;
            errorString << STR_ERR_CANNOT_LOAD_SUPPORTED_GPUS_LIST_FOR_MODE_A;
            errorString << mode;
            errorString << STR_ERR_CANNOT_LOAD_SUPPORTED_GPUS_LIST_FOR_MODE_B;
            rgUtils::ShowErrorMessageBox(errorString.str().c_str(), m_pParent);
        }
    }

    // Generate the colors for each group.
    SetDefaultTableBackgroundColors();

    // Filter the tree model with the proxy model.
    m_pTableFilterModel = new rgTableFilterProxyModel(this);
    m_pTableFilterModel->setSourceModel(m_pGpuTreeModel);

    // Display the filtered model in the QTreeView.
    ui.gpuTreeView->setModel(m_pTableFilterModel);
    QtCommon::QtUtil::AutoAdjustTableColumns(ui.gpuTreeView, 32, 10);
}

void rgTargetGpusDialog::SetIsGroupChecked(int groupIndex, bool isChecked)
{
    bool isValidIndex = (groupIndex >= 0 && groupIndex < m_capabilityGroups.size());
    assert(isValidIndex);
    if (isValidIndex)
    {
        CapabilityGroup& targetGroup = m_capabilityGroups[groupIndex];
        for (auto currentRow : targetGroup.m_groupRows)
        {
            QStandardItem* pCheckItem = m_pGpuTreeModel->item(currentRow.m_rowIndex, ColumnType::SelectedCheckbox);
            pCheckItem->setCheckState(isChecked ? Qt::Checked : Qt::Unchecked);
        }
    }
}

void rgTargetGpusDialog::SetTableIndexData(int row, int column, const std::string& stringData)
{
    // Generate an index and verify that it's valid.
    QModelIndex modelIndex = m_pGpuTreeModel->index(row, column);
    bool isValid = modelIndex.isValid();
    assert(isValid);
    if (isValid)
    {
        // Set the data at the correct index in the GPU tree.
        m_pGpuTreeModel->setData(modelIndex, stringData.c_str());
    }
}

void rgTargetGpusDialog::SetTableBackgroundColor(int row, const QColor& backgroundColor)
{
    // Generate an index and verify that it's valid.
    for (int columnIndex = ColumnType::SelectedCheckbox; columnIndex < ColumnType::Count; ++columnIndex)
    {
        QModelIndex modelIndex = m_pGpuTreeModel->index(row, columnIndex);
        bool isValid = modelIndex.isValid();
        assert(isValid);
        if (isValid)
        {
            // Set the data at the correct index in the GPU tree.
            m_pGpuTreeModel->setData(modelIndex, backgroundColor, Qt::BackgroundColorRole);
        }
    }
}

void rgTargetGpusDialog::ToggleOKButtonEnabled(bool isOkEnabled)
{
    ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(isOkEnabled);
}

void rgTargetGpusDialog::SetDefaultTableBackgroundColors()
{
    // The color's value remains constant for all rows.
    static const float s_BACKGROUND_COLOR_VALUE = 1.0f;

    // Compute the base hue for each architecture, and the saturations for each capability group.
    std::map<std::string, float> architectureGroupHues;
    std::map<std::string, float> computeCapabilitySaturations;
    ComputeGroupColors(architectureGroupHues, computeCapabilitySaturations);

    // Step through each capability group and initialize the row's background color.
    for (int groupIndex = 0; groupIndex < static_cast<int>(m_capabilityGroups.size()); ++groupIndex)
    {
        CapabilityGroup& groupInfo = m_capabilityGroups[groupIndex];

        // Set the table background color for each row in the group.
        for (size_t rowIndex = 0; rowIndex < groupInfo.m_groupRows.size(); ++rowIndex)
        {
            auto groupRow = groupInfo.m_groupRows[rowIndex];

            auto architectureHueIter = architectureGroupHues.find(groupRow.m_architecture);
            assert(architectureHueIter != architectureGroupHues.end());
            if (architectureHueIter != architectureGroupHues.end())
            {
                float architectureHue = architectureHueIter->second;

                auto computeCapabilitySaturationsIter = computeCapabilitySaturations.find(groupRow.m_computeCapability);
                assert(computeCapabilitySaturationsIter != computeCapabilitySaturations.end());
                if (computeCapabilitySaturationsIter != computeCapabilitySaturations.end())
                {
                    // Extract the saturation for the group.
                    float groupSaturation = computeCapabilitySaturationsIter->second;

                    // Initialize the color for the row.
                    QColor color;
                    color.setHsvF(architectureHue, groupSaturation, s_BACKGROUND_COLOR_VALUE);

                    // Set the background color for the table row.
                    SetTableBackgroundColor(groupRow.m_rowIndex, color);

                    // Set each row to deselected.
                    SetRowDeselected(groupRow.m_rowIndex);
                }
            }
        }
    }
}

void rgTargetGpusDialog::SetRowSelected(QModelIndex modelIndex)
{
    for (int columnIndex = ColumnType::SelectedCheckbox; columnIndex < ColumnType::Count; ++columnIndex)
    {
        if (ui.gpuTreeView->selectionModel() != nullptr && m_pGpuTreeModel != nullptr)
        {
            QModelIndex index = m_pGpuTreeModel->index(modelIndex.row(), columnIndex);
            ui.gpuTreeView->selectionModel()->setCurrentIndex(index, QItemSelectionModel::Select);
        }
    }

    // Update the table.
    ui.gpuTreeView->update();
}

void rgTargetGpusDialog::SetRowDeselected(int rowNumber)
{
    if (ui.gpuTreeView->selectionModel() != nullptr )
    {
        for (int columnIndex = ColumnType::SelectedCheckbox; columnIndex < ColumnType::Count; ++columnIndex)
        {
            QModelIndex modelIndex = m_pGpuTreeModel->index(rowNumber, columnIndex);
            bool isValid = modelIndex.isValid();
            assert(isValid);
            if (isValid)
            {
                // Set the data at the correct index in the GPU tree.
                ui.gpuTreeView->selectionModel()->setCurrentIndex(modelIndex, QItemSelectionModel::Deselect);
            }
        }

        // Update the table.
        ui.gpuTreeView->update();
    }
}

bool rgTargetGpusDialog::IsRowMatchingSearchString(const TableRow& currentRow, const QRegExp searchFilter)
{
    bool isMatching = false;

    // Does the architecture name match the filter string?
    isMatching = searchFilter.exactMatch(currentRow.m_architecture.c_str());
    if (!isMatching)
    {
        // Does the compute capability match the filter string?
        isMatching = searchFilter.exactMatch(currentRow.m_computeCapability.c_str());
        if (!isMatching)
        {
            // Does the product name match the filter string?
            isMatching = searchFilter.exactMatch(currentRow.m_productName.c_str());
        }
    }

    return isMatching;
}
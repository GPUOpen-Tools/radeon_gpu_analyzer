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
#include <QtCommon/Util/QtUtil.h>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgTargetGpusDialog.h>
#include <RadeonGPUAnalyzerGUI/Include/rgCliLauncher.h>
#include <RadeonGPUAnalyzerGUI/Include/rgStringConstants.h>
#include <RadeonGPUAnalyzerGUI/Include/rgUtils.h>
#include <RadeonGPUAnalyzerGUI/Include/rgXMLCliVersionInfo.h>

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

bool rgTreeEventFilter::eventFilter(QObject* pObject, QEvent* pEvent)
{
    bool isHandled = false;

    assert(pEvent != nullptr);
    if (pEvent != nullptr && pEvent->type() == QEvent::Type::KeyPress)
    {
        // Watch for KeyPress events on only the Spacebar.
        QKeyEvent* pKeyEvent = dynamic_cast<QKeyEvent*>(pEvent);
        assert(pKeyEvent != nullptr);
        if (pKeyEvent != nullptr && pKeyEvent->key() == Qt::Key_Space)
        {
            // Emit the signal used to activate the selected row.
            emit RowActivated();
            isHandled = true;
        }
    }

    if (!isHandled)
    {
        // Invoke the base implementation.
        isHandled = QObject::eventFilter(pObject, pEvent);
    }

    return isHandled;
}

rgTargetGpusDialog::rgTargetGpusDialog(const QString& selectedGPUs, QWidget* pParent) :
    QDialog(pParent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
    m_pParent(pParent)
{
    ui.setupUi(this);

    // Double-clicking an item within the QTreeView will toggle its check state- not expand/collapse the children.
    ui.gpuTreeView->setExpandsOnDoubleClick(false);

    // Disable editing cells within the table.
    ui.gpuTreeView->setEditTriggers(QAbstractItemView::EditTrigger::NoEditTriggers);

    // Create an eventFilter used to handle keyPress events in the GPU treeview.
    rgTreeEventFilter* pKeypressEventFilter = new rgTreeEventFilter(this);
    assert(pKeypressEventFilter != nullptr);
    if (pKeypressEventFilter != nullptr)
    {
        // Connect the handler that gets invoked when the user toggles a row with the spacebar.
        bool isConnected = connect(pKeypressEventFilter, &rgTreeEventFilter::RowActivated, this, &rgTargetGpusDialog::HandleRowToggled);
        assert(isConnected);
        if (isConnected)
        {
            // Install the eventFilter in the treeview.
            ui.gpuTreeView->installEventFilter(pKeypressEventFilter);
        }
    }

    // Use the cached CLI version info structure to populate the GPU tree.
    std::shared_ptr<rgCliVersionInfo> pVersionInfo = rgConfigManager::Instance().GetVersionInfo();
    assert(pVersionInfo != nullptr);
    if (pVersionInfo != nullptr)
    {
        const std::string& currentMode = rgConfigManager::Instance().GetCurrentModeString();
        PopulateTableData(pVersionInfo, currentMode);
    }

    // Connect the target GPUs dialog signals.
    ConnectSignals();

    // Set window flags.
    setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint);

    // Split the incoming string of comma-separated families into a vector.
    std::vector<std::string> selectedGPUsVector;
    rgUtils::splitString(selectedGPUs.toStdString(), rgConfigManager::RGA_LIST_DELIMITER, selectedGPUsVector);

    // Preprocess the gpu family names: since we get some family names
    // as a codename without the gfx notation, we would have to add the
    // gfx notation here again.
    std::transform(selectedGPUsVector.begin(), selectedGPUsVector.end(),
        selectedGPUsVector.begin(), [&](std::string& familyName)
    {
        std::stringstream fixedName;
        std::string gfxNotation;
        bool hasGfxNotation = rgUtils::GetGfxNotation(familyName, gfxNotation);
        if (hasGfxNotation && familyName.find("/") == std::string::npos)
        {
            fixedName << gfxNotation << "/" << familyName;
        }
        else
        {
            fixedName << familyName;
        }
        return fixedName.str();
    });

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
                        std::string groupName = groupInfo.m_groupRows[0].m_computeCapability;
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

void rgTargetGpusDialog::HandleRowToggled()
{
    // Use the treeView's selectionModel to find the currently selected row index.
    QItemSelectionModel* pSelectionModel = ui.gpuTreeView->selectionModel();
    assert(pSelectionModel != nullptr);
    if (pSelectionModel != nullptr)
    {
        // Is the selected row index valid?
        QModelIndex selectedRow = pSelectionModel->currentIndex();
        if (selectedRow.isValid())
        {
            assert(m_pTableFilterModel != nullptr);
            if (m_pTableFilterModel != nullptr)
            {
                // The selection model index doesn't know anything about the filtering model. Convert
                // the (potentially filtered) selection model index to a source model index.
                QModelIndex sourceModelIndex = m_pTableFilterModel->mapToSource(selectedRow);
                if (sourceModelIndex.isValid())
                {
                    // Provide the source model index for the row being toggled.
                    ToggleRowChecked(sourceModelIndex);
                }
            }
        }
    }
}

void rgTargetGpusDialog::HandleRowDoubleClicked(const QModelIndex& index)
{
    assert(m_pTableFilterModel != nullptr);
    if (m_pTableFilterModel != nullptr)
    {
        QModelIndex sourceModelIndex = m_pTableFilterModel->mapToSource(index);
        if (sourceModelIndex.isValid())
        {
            // Provide the source model index for the row being toggled.
            ToggleRowChecked(sourceModelIndex);
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

#ifdef _HIGHLIGHT_MATCHING_ROWS
    // Disable this for now as the value we get from the feature
    // is negligible compared to the performance impact - it makes
    // the view sluggish. To be optimized.

    // Highlight the matching rows.
    HighlightMatchingRows(regex);
#endif
}

void rgTargetGpusDialog::ComputeGroupColor(int groupIndex, QColor& color) const
{
    // Use 25% saturation for a more muted color palette, but 100% value to maintain brightness.
    static const int s_BACKGROUND_COLOR_NUM_HUE_STEPS = 10;
    static const float s_BACKGROUND_COLOR_HUE_STEP_INCREMENT = 1.0f / s_BACKGROUND_COLOR_NUM_HUE_STEPS;
    static const float s_BACKGROUND_COLOR_SATURATION = 0.25f;
    static const float s_BACKGROUND_COLOR_VALUE = 1.0f;

    // If the number of groups exceeds the number of hue steps, reset the hue to the start of the color wheel.
    int colorGroupIndex = groupIndex % s_BACKGROUND_COLOR_NUM_HUE_STEPS;

    // Compute the group's color based on the group index vs. the total group count.
    float hue = colorGroupIndex * s_BACKGROUND_COLOR_HUE_STEP_INCREMENT;
    color.setHsvF(hue, s_BACKGROUND_COLOR_SATURATION, s_BACKGROUND_COLOR_VALUE);
}

void rgTargetGpusDialog::ConnectSignals()
{
    // Connect the tree's model to the check changed handler.
    bool isConnected = connect(this->ui.searchTextbox, &QLineEdit::textChanged, this, &rgTargetGpusDialog::HandleSearchTextChanged);
    assert(isConnected);

    // Connect the table's double click handler to toggle the row's check state.
    isConnected = connect(this->ui.gpuTreeView, &QTreeView::doubleClicked, this, &rgTargetGpusDialog::HandleRowDoubleClicked);
    assert(isConnected);

    // Connect the OK button.
    isConnected = connect(this->ui.OkPushButton, &QPushButton::clicked, this, &rgTargetGpusDialog::HandleOKButtonClicked);
    assert(isConnected);

    // Connect the Cancel button.
    isConnected = connect(this->ui.cancelPushButton, &QPushButton::clicked, this, &rgTargetGpusDialog::HandleCancelButtonClicked);
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
        // Container for indices of rows containing matching GPU names.
        std::vector<QModelIndex>  matchingRows;

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
                        // Store the model index for this row.
                        QModelIndex modelIndex = m_pGpuTreeModel->index(currentRow.m_rowIndex, 0);
                        matchingRows.push_back(modelIndex);
                    }
                }
            }
        }

        // Highlight matching rows.
        for (const QModelIndex& index : matchingRows)
        {
            ui.gpuTreeView->selectionModel()->select(index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
        }

        // Update the table.
        ui.gpuTreeView->update();
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
    m_pGpuTreeModel->setHorizontalHeaderItem(ColumnType::SelectedCheckbox, pSelectedCheckbox);
    m_pGpuTreeModel->setHorizontalHeaderItem(ColumnType::ProductName, new QStandardItem(STR_TARGET_GPU_PRODUCT_NAME));
    m_pGpuTreeModel->setHorizontalHeaderItem(ColumnType::Architecture, new QStandardItem(STR_TARGET_GPU_ARCHITECTURE));
    m_pGpuTreeModel->setHorizontalHeaderItem(ColumnType::ComputeCapability, new QStandardItem(STR_TARGET_GPU_COMPUTE_CAPABILITY));

    // Set tooltips on the table headers.
    m_pGpuTreeModel->setHeaderData(ColumnType::ProductName, Qt::Orientation::Horizontal, STR_TABLE_TOOLTIP_COLUMN_PRODUCT_NAME, Qt::ToolTipRole);
    m_pGpuTreeModel->setHeaderData(ColumnType::Architecture, Qt::Orientation::Horizontal, STR_TABLE_TOOLTIP_COLUMN_ARCHITECTURE, Qt::ToolTipRole);
    m_pGpuTreeModel->setHeaderData(ColumnType::ComputeCapability, Qt::Orientation::Horizontal, STR_TABLE_TOOLTIP_COLUMN_COMPUTE_CAPABILITY, Qt::ToolTipRole);

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
                std::vector<rgGpuFamily> gpuFamilies = hardwareArchitecture.m_gpuFamilies;
                int numFamiliesInArchitecture = static_cast<int>(gpuFamilies.size());

                // Set the gfx notation as prefix if applicable to the family name.
                // For example, "Tonga" would become "gfx802/Tonga".
                for(int i = 0; i < numFamiliesInArchitecture; i++)
                {
                    std::string gfxNotation;
                    std::stringstream familyNameRevised;
                    bool hasGfxNotation = rgUtils::GetGfxNotation(gpuFamilies[i].m_familyName, gfxNotation);
                    if (hasGfxNotation && !gfxNotation.empty())
                    {
                        familyNameRevised << gfxNotation << "/";
                    }
                    familyNameRevised << gpuFamilies[i].m_familyName;
                    gpuFamilies[i].m_familyName = familyNameRevised.str().c_str();
                }

                // Sort the GPU families in reverse order so that for the new AMD
                // GPU naming scheme (gfxABCD) we will have the newer families on top.
                std::sort(gpuFamilies.rbegin(), gpuFamilies.rend(), [&](rgGpuFamily familyA,
                    rgGpuFamily familyB) { return familyA.m_familyName < familyB.m_familyName; });

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

                        TableRow newRow = { currentRowIndex, currentArchitecture, productName, familyName };
                        capabilityGroup.m_groupRows.push_back(newRow);

                        // Set the data for each column in the new row.
                        SetTableIndexData(currentRowIndex, ColumnType::ProductName, newRow.m_productName);
                        SetTableIndexData(currentRowIndex, ColumnType::Architecture, newRow.m_architecture);
                        SetTableIndexData(currentRowIndex, ColumnType::ComputeCapability, newRow.m_computeCapability);

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
    ui.OkPushButton->setEnabled(isOkEnabled);
}

void rgTargetGpusDialog::ToggleRowChecked(const QModelIndex& sourceModelIndex)
{
    // Find the group index associated with the product row.
    auto groupIndexIter = m_tableRowIndexToGroupIndex.find(sourceModelIndex.row());
    if (groupIndexIter != m_tableRowIndexToGroupIndex.end())
    {
        assert(m_pTableFilterModel != nullptr);
        if (m_pTableFilterModel != nullptr)
        {
            // Find the filtered index for the checkbox item in the row.
            QAbstractItemModel* pSourceItemModel = m_pTableFilterModel->sourceModel();
            assert(pSourceItemModel != nullptr);
            if (pSourceItemModel != nullptr)
            {
                QModelIndex checkboxColumnFilteredIndex = pSourceItemModel->index(sourceModelIndex.row(), ColumnType::SelectedCheckbox);
                bool isFilteredIndexValid = checkboxColumnFilteredIndex.isValid();
                assert(isFilteredIndexValid);
                if (isFilteredIndexValid)
                {
                    assert(m_pGpuTreeModel != nullptr);
                    if (m_pGpuTreeModel != nullptr)
                    {
                        // Retrieve a pointer to the checkbox item in the table.
                        QStandardItem* pClickedItem = m_pGpuTreeModel->itemFromIndex(checkboxColumnFilteredIndex);
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
    }
}

void rgTargetGpusDialog::SetDefaultTableBackgroundColors()
{
    for (int groupIndex = 0; groupIndex < static_cast<int>(m_capabilityGroups.size()); ++groupIndex)
    {
        CapabilityGroup& groupInfo = m_capabilityGroups[groupIndex];

        // Compute the group's color based on the group index.
        ComputeGroupColor(groupIndex, groupInfo.m_color);

        // Set the table background color for each row in the group.
        for (size_t rowIndex = 0; rowIndex < groupInfo.m_groupRows.size(); ++rowIndex)
        {
            auto groupRow = groupInfo.m_groupRows[rowIndex];
            SetTableBackgroundColor(groupRow.m_rowIndex, groupInfo.m_color);

            // Set each row to deselected.
            if (ui.gpuTreeView->selectionModel() != nullptr)
            {
                QModelIndex modelIndex = m_pGpuTreeModel->index(static_cast<int>(rowIndex), 0);
                ui.gpuTreeView->selectionModel()->select(modelIndex, QItemSelectionModel::Deselect | QItemSelectionModel::Rows);
            }
        }
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

void rgTargetGpusDialog::HandleOKButtonClicked(bool /* checked */)
{
    this->accept();
}

void rgTargetGpusDialog::HandleCancelButtonClicked(bool /* checked */)
{
    this->reject();
}
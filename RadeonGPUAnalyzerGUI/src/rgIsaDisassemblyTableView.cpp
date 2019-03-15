// C++.
#include <cassert>
#include <sstream>

// Qt.
#include <QLabel>
#include <QMenu>
#include <QPainter>
#include <QScrollBar>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QStyledItemDelegate>

// Infra.
#include <QtCommon/Util/QtUtil.h>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgIsaDisassemblyTableModel.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgIsaDisassemblyTableView.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDataTypes.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDefinitions.h>
#include <RadeonGPUAnalyzerGUI/Include/rgStringConstants.h>
#include <RadeonGPUAnalyzerGUI/Include/rgUtils.h>

// A class used to filter the ISA disassembly table columns.
class rgIsaDisassemblyTableModelFilteringModel : public QSortFilterProxyModel
{
public:
    rgIsaDisassemblyTableModelFilteringModel(rgIsaDisassemblyTableModel* pSourceModel, QObject* pParent = nullptr) :
        QSortFilterProxyModel(pParent), m_pSourceModel(pSourceModel) {}
    ~rgIsaDisassemblyTableModelFilteringModel() = default;

protected:
    // A column-filtering predicate.
    virtual bool filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const override
    {
        // Ask the model if the given column index should be displayed.
        return m_pSourceModel->IsColumnVisible(source_column);
    }

private:
    // The source model being filtered.
    rgIsaDisassemblyTableModel* m_pSourceModel = nullptr;
};

rgIsaDisassemblyTableView::rgIsaDisassemblyTableView(QWidget* pParent) :
    QWidget(pParent)
{
    ui.setupUi(this);

    // Use the instruction treeview as the focus proxy for this view.
    setFocusProxy(ui.instructionsTreeView);

    // Create the model holding the disassembly table data.
    m_pIsaTableModel = new rgIsaDisassemblyTableModel(0, this);
    ui.instructionsTreeView->SetModel(m_pIsaTableModel);

    // Create a column-filtering model to filter the table model.
    m_pIsaTableFilteringModel = new rgIsaDisassemblyTableModelFilteringModel(m_pIsaTableModel, this);

    // Set the item model for the filtering proxy model.
    QStandardItemModel* pItemModel = m_pIsaTableModel->GetTableModel();
    m_pIsaTableFilteringModel->setSourceModel(pItemModel);

    // Set the filter model on the table.
    ui.instructionsTreeView->setModel(m_pIsaTableFilteringModel);

    // Allow the last column to stretch.
    ui.instructionsTreeView->header()->setStretchLastSection(true);

    // Initialize the context menu.
    InitializeContextMenu();

    // Connect the table's selection model.
    ConnectSelectionSignals();

    // Connect the signals.
    ConnectSignals();
}

bool rgIsaDisassemblyTableView::IsLineInEntrypoint(int lineIndex)
{
    bool isInEntrypoint = false;

    // If the disassembly hasn't already been cached, load it.
    bool isDisassemblyLoaded = IsDisassemblyLoaded();
    if (!isDisassemblyLoaded)
    {
        // Load the disassembly.
        isDisassemblyLoaded = LoadDisassembly(GetDisassemblyFilePath());
    }

    assert(isDisassemblyLoaded);
    if (isDisassemblyLoaded)
    {
        isInEntrypoint = m_pIsaTableModel->IsSourceLineInEntrypoint(lineIndex);
    }

    return isInEntrypoint;
}

bool rgIsaDisassemblyTableView::LoadDisassembly(const std::string& disassemblyCsvFilePath)
{
    m_isDisassemblyCached = m_pIsaTableModel->PopulateFromCsvFile(disassemblyCsvFilePath);

    if (m_isDisassemblyCached)
    {
        // Cache the path to the disassembly CSV file being loaded.
        m_disassemblyFilePath = disassemblyCsvFilePath;

        // Initialize jump link labels that need to be inserted into specific table ells.
        InitializeLinkLabels();

        // Adjust the table column widths after populating with data.
        QtCommon::QtUtil::AutoAdjustTableColumns(ui.instructionsTreeView, 10, 20);
    }

    return m_isDisassemblyCached;
}

void rgIsaDisassemblyTableView::RequestTableResize()
{
    // The maximum number of rows to measure when computing the width of each column.
    static const int s_MAX_NUM_ROWS = 32;

    // The amount of extra padding to add when resizing.
    static const int s_COLUMN_PADDING = 20;

    // Compute the ideal width of the table and emit a signal to readjust the view dimensions.
    int minWidth = QtCommon::QtUtil::ComputeMinimumTableWidth(ui.instructionsTreeView, s_MAX_NUM_ROWS, s_COLUMN_PADDING);
    emit DisassemblyTableWidthResizeRequested(minWidth);
}

void rgIsaDisassemblyTableView::UpdateCorrelatedSourceFileLine(int lineNumber)
{
    // Set the highlighted lines in the table model.
    bool isCorrelated = m_pIsaTableModel->SetCorrelatedSourceLineIndex(lineNumber);

    // Scroll to the highlighted lines only if the high-level source file's current table has a correlated line with the disassembly table.
    if (isCorrelated)
    {
        const std::vector<int>& disassemblyLineIndices = m_pIsaTableModel->GetCorrelatedLineIndices();
        if (!disassemblyLineIndices.empty())
        {
            // Extract the first highlighted Isa row index, and scroll to it.
            int firstHighlightedDisassemblyRowIndex = disassemblyLineIndices[0];
            ScrollToLine(firstHighlightedDisassemblyRowIndex);
        }
    }

    // Trigger a repaint of the treeview.
    ui.instructionsTreeView->repaint();
}

void rgIsaDisassemblyTableView::UpdateFilteredTable()
{
    // Invalidate the table's filtering model, since column visibility has changed.
    m_pIsaTableFilteringModel->invalidate();

    // Add all label rows into the table after filtering.
    InitializeLabelRows();

    // Add all link labels into the table after filtering.
    InitializeLinkLabels();

    // Adjust the table column widths after populating with data.
    QtCommon::QtUtil::AutoAdjustTableColumns(ui.instructionsTreeView, 32, 20);
}

std::string rgIsaDisassemblyTableView::GetDisassemblyFilePath() const
{
    return m_disassemblyFilePath;
}

void rgIsaDisassemblyTableView::SetDisassemblyFilePath(const std::string& disassemblyFilePath)
{
    m_disassemblyFilePath = disassemblyFilePath;
}

bool rgIsaDisassemblyTableView::IsDisassemblyLoaded() const
{
    return m_isDisassemblyCached;
}

bool rgIsaDisassemblyTableView::IsSourceLineCorrelated(int lineIndex) const
{
    assert(m_pIsaTableModel != nullptr);
    return (m_pIsaTableModel != nullptr ? m_pIsaTableModel->IsSourceLineCorrelated(lineIndex) : false);
}

void rgIsaDisassemblyTableView::HandleBranchLinkClicked(const QString& link)
{
    assert(m_pIsaTableModel != nullptr);
    if (m_pIsaTableModel != nullptr)
    {
        // Obtain the map of all labels within the disassembly table.
        std::map<std::string, int> labelNameToLineNumber;
        m_pIsaTableModel->GetLabelNameToLineIndexMap(labelNameToLineNumber);

        // Find the given link name within the map, and scroll to the line index.
        auto lineIndexIter = labelNameToLineNumber.find(link.toStdString());
        if (lineIndexIter != labelNameToLineNumber.end())
        {
            int labelLineNumber = lineIndexIter->second;

            // Advance the focused line by 1. That's where the label's first line of code is.
            int labelCodeLine = labelLineNumber + 1;

            // Retrieve the source line number associated with the label's ISA line number.
            int inputSourceLineNumber = kInvalidCorrelationLineIndex;
            m_pIsaTableModel->GetInputSourceLineNumberFromInstructionRow(labelCodeLine, inputSourceLineNumber);

            // Emit the signal used to highlight the label's correlated source code.
            emit InputSourceHighlightedLineChanged(inputSourceLineNumber);

            // Scroll the table so that the label is visible.
            ScrollToLine(labelLineNumber);
        }
    }
}

void rgIsaDisassemblyTableView::HandleCopyDisassemblyClicked()
{
    QVector<int> selectedRowNumbers;

    QItemSelectionModel* pSelectionModel = ui.instructionsTreeView->selectionModel();
    const QItemSelection selection = pSelectionModel->selection();
    if (!selection.isEmpty())
    {
        // Get the selected line numbers.
        int startRow = 0;
        int endRow = 0;
        QItemSelectionModel* pTableSelectionModel = ui.instructionsTreeView->selectionModel();
        QModelIndexList selectedRows = pTableSelectionModel->selectedRows();
        for (auto& currentIndex : selectedRows)
        {
            int rowIndex = currentIndex.row();
            selectedRowNumbers << rowIndex;
        }

        // Make sure the row numbers are in descending order since the user could've made selections out of order.
        std::sort(selectedRowNumbers.begin(), selectedRowNumbers.end());

        // Copy the range of row data to the user's clipboard.
        m_pIsaTableModel->CopyRowsToClipboard(selectedRowNumbers);

    }
}

void rgIsaDisassemblyTableView::HandleOpenDisassemblyInFileBrowserClicked()
{
    // Use the path to the loaded CSV file to figure out which folder to open.
    std::string buildOutputDirectory;
    bool isOk = rgUtils::ExtractFileDirectory(m_disassemblyFilePath, buildOutputDirectory);
    assert(isOk);
    if (isOk)
    {
        // Open the directory in the system's file browser.
        rgUtils::OpenFolderInFileBrowser(buildOutputDirectory);
    }
}

void rgIsaDisassemblyTableView::HandleCurrentSelectionChanged(const QItemSelection& selected, const QItemSelection& /*deselected*/)
{
    // Use the model's current selection to check what needs to be highlighted.
    QItemSelectionModel* pSelectionModel = ui.instructionsTreeView->selectionModel();
    assert(pSelectionModel != nullptr);
    if (pSelectionModel != nullptr)
    {
        const QItemSelection& currentSelection = pSelectionModel->selection();

        QModelIndexList selectedIndices = currentSelection.indexes();
        if (!selectedIndices.isEmpty())
        {
            QModelIndex firstSelectedIndex = selectedIndices[0];

            if (firstSelectedIndex.isValid())
            {
                int firstSelectedRowIndex = firstSelectedIndex.row();

                // Determine which line in the input source file is correlated with the given Isa row.
                int inputSourceLineNumber = kInvalidCorrelationLineIndex;
                m_pIsaTableModel->GetInputSourceLineNumberFromInstructionRow(firstSelectedRowIndex, inputSourceLineNumber);

                // Did the user select a line that isn't currently highlighted?
                const std::vector<int>& indices = m_pIsaTableModel->GetCorrelatedLineIndices();
                auto correlatedLinesIter = std::find(indices.begin(), indices.end(), firstSelectedRowIndex);
                if (correlatedLinesIter == indices.end())
                {
                    // Invalidate the currently highlighted lines.
                    m_pIsaTableModel->SetCorrelatedSourceLineIndex(kInvalidCorrelationLineIndex);
                }

                // Do the instruction lines in the selected block all contiguously map to the same input source file line?
                int correlatedLineIndex = kInvalidCorrelationLineIndex;
                bool isContiguousBlockSelected = IsContiguousCorrelatedRangeSelected(correlatedLineIndex);

                if (isContiguousBlockSelected)
                {
                    // Emit a signal indicating that the input source file's correlation highlight line should be updated.
                    emit InputSourceHighlightedLineChanged(inputSourceLineNumber);
                }
                else
                {
                    // Clear the correlated lines in the disassembly table.
                    UpdateCorrelatedSourceFileLine(kInvalidCorrelationLineIndex);

                    // Clear the highlighted line in the source editor.
                    emit InputSourceHighlightedLineChanged(kInvalidCorrelationLineIndex);
                }
            }
        }
    }
}

void rgIsaDisassemblyTableView::HandleOpenContextMenu(const QPoint& widgetClickPosition)
{
    // Convert the widget's local click position to the global screen position.
    const QPoint clickPoint = mapToGlobal(widgetClickPosition);

    // Open the context menu at the user's click position.
    m_pContextMenu->exec(clickPoint);
}

void rgIsaDisassemblyTableView::ConnectContextMenuSignals()
{
    // Connect the handler responsible for triggering a copy to clipboard from the table.
    bool isConnected = connect(m_pCopySelectedDisassembly, &QAction::triggered, this, &rgIsaDisassemblyTableView::HandleCopyDisassemblyClicked);
    assert(isConnected);

    // Connect the handler responsible for opening the current disassembly build output folder in the system file browser.
    isConnected = connect(m_pOpenDisassemblyInFileBrowser, &QAction::triggered, this, &rgIsaDisassemblyTableView::HandleOpenDisassemblyInFileBrowserClicked);
    assert(isConnected);

    // Use a custom context menu for the table.
    this->setContextMenuPolicy(Qt::CustomContextMenu);

    // Connect the handler responsible for showing the table's context menu.
    isConnected = connect(this, &QWidget::customContextMenuRequested, this, &rgIsaDisassemblyTableView::HandleOpenContextMenu);
    assert(isConnected);
}

void rgIsaDisassemblyTableView::ConnectSelectionSignals()
{
    QItemSelectionModel* pSelectionModel = ui.instructionsTreeView->selectionModel();
    assert(pSelectionModel != nullptr);
    if (pSelectionModel != nullptr)
    {
        // Connect the table's selection changed handler.
        bool isConnected = connect(pSelectionModel, &QItemSelectionModel::selectionChanged, this, &rgIsaDisassemblyTableView::HandleCurrentSelectionChanged);
        assert(isConnected);
    }
}

void rgIsaDisassemblyTableView::ConnectSignals()
{
    // Connect the disassembly table's focus in signal.
    bool isConnected = connect(ui.instructionsTreeView, &rgIsaDisassemblyCustomTableView::FrameFocusInSignal, this, &rgIsaDisassemblyTableView::FrameFocusInSignal);
    assert(isConnected);

    // Connect the disassembly table's focus out signal.
    isConnected = connect(ui.instructionsTreeView, &rgIsaDisassemblyCustomTableView::FrameFocusOutSignal, this, &rgIsaDisassemblyTableView::FrameFocusOutSignal);
    assert(isConnected);

    // Connect the disassembly table view's enable scroll bar signal.
    isConnected = connect(this, &rgIsaDisassemblyTableView::EnableScrollbarSignals, ui.instructionsTreeView, &rgIsaDisassemblyCustomTableView::EnableScrollbarSignals);
    assert(isConnected);

    // Connect the disassembly table view's disable scroll bar signal.
    isConnected = connect(this, &rgIsaDisassemblyTableView::DisableScrollbarSignals, ui.instructionsTreeView, &rgIsaDisassemblyCustomTableView::DisableScrollbarSignals);
    assert(isConnected);

    // Connect the disassembly table's target GPU push button focus in signal.
    isConnected = connect(ui.instructionsTreeView, &rgIsaDisassemblyCustomTableView::FocusTargetGpuPushButton, this, &rgIsaDisassemblyTableView::FocusTargetGpuPushButton);
    assert(isConnected);

    // Connect the disassembly table's target GPU push button focus in signal.
    isConnected = connect(ui.instructionsTreeView, &rgIsaDisassemblyCustomTableView::SwitchDisassemblyContainerSize, this, &rgIsaDisassemblyTableView::SwitchDisassemblyContainerSize);
    assert(isConnected);

    // Connect the disassembly table's columns push button focus in signal.
    isConnected = connect(ui.instructionsTreeView, &rgIsaDisassemblyCustomTableView::FocusColumnPushButton, this, &rgIsaDisassemblyTableView::FocusColumnPushButton);
    assert(isConnected);

    // Connect the disassembly table's source window focus in signal.
    isConnected = connect(ui.instructionsTreeView, &rgIsaDisassemblyCustomTableView::FocusSourceWindow, this, &rgIsaDisassemblyTableView::FocusSourceWindow);
    assert(isConnected);

    // Connect the disassembly table view's update current sub widget signal.
    isConnected = connect(this, &rgIsaDisassemblyTableView::UpdateCurrentSubWidget, ui.instructionsTreeView, &rgIsaDisassemblyCustomTableView::HandleUpdateCurrentSubWidget);
    assert(isConnected);

    // Connect the disassembly table's focus cli output window signal.
    isConnected = connect(ui.instructionsTreeView, &rgIsaDisassemblyCustomTableView::FocusCliOutputWindow, this, &rgIsaDisassemblyTableView::FocusCliOutputWindow);
    assert(isConnected);
}

bool rgIsaDisassemblyTableView::GetSelectedRowRange(int& minRow, int& maxRow) const
{
    bool gotRange = false;

    QItemSelectionModel* pTableSelectionModel = ui.instructionsTreeView->selectionModel();
    auto rowsSelection = pTableSelectionModel->selectedRows();

    if (!rowsSelection.isEmpty())
    {
        // Find the bounds of the selected row indices.
        minRow = INT_MAX;
        maxRow = INT_MIN;
        for (QModelIndex& currentIndex : rowsSelection)
        {
            int rowIndex = currentIndex.row();
            minRow = (rowIndex < minRow) ? rowIndex : minRow;
            maxRow = (rowIndex > maxRow) ? rowIndex : maxRow;
        }

        gotRange = true;
    }

    return gotRange;
}

void rgIsaDisassemblyTableView::InitializeContextMenu()
{
    // Create the context menu instance.
    m_pContextMenu = new QMenu(this);

    // Set the cursor for the context menu.
    m_pContextMenu->setCursor(Qt::PointingHandCursor);

    // Create the menu items to insert into the context menu.
    m_pCopySelectedDisassembly = new QAction(STR_DISASSEMBLY_TABLE_CONTEXT_MENU_COPY, this);
    m_pContextMenu->addAction(m_pCopySelectedDisassembly);

    // Create an item allowing the user to browse to the disassembly build output folder.
    m_pOpenDisassemblyInFileBrowser = new QAction(STR_DISASSEMBLY_TABLE_CONTEXT_MENU_OPEN_IN_FILE_BROWSER, this);
    m_pContextMenu->addAction(m_pOpenDisassemblyInFileBrowser);

    // Connect the context menu signals.
    ConnectContextMenuSignals();
}

void rgIsaDisassemblyTableView::InitializeLabelRows()
{
    // Clear existing label lines from the model.
    m_pIsaTableModel->ClearLabelLines();

    // Always insert labels into the left-most visible column in the table.
    m_pIsaTableModel->InsertLabelRows();
}

void rgIsaDisassemblyTableView::InitializeLinkLabels()
{
    // A map of row index to the label's name being linked to in the operands column.
    std::map<int, std::string> linkLabels;
    m_pIsaTableModel->GetLineIndexToLabelNameMap(linkLabels);

    int columnIndex = rgIsaDisassemblyTableModel::GetTableColumnIndex(rgIsaDisassemblyTableColumns::Operands);

    // Hold a vector of all Label links inserted into the table.
    std::vector<QLabel*> labelLinks;

    auto startLabel = linkLabels.begin();
    auto endLabel = linkLabels.end();
    for (auto labelsIter = startLabel; labelsIter != endLabel; ++labelsIter)
    {
        // Extract the line number and label text from the map.
        int labelLineIndex = labelsIter->first;
        std::string& labelText = labelsIter->second;

        QStandardItemModel* pTableModel = m_pIsaTableModel->GetTableModel();

        // Compute the index in the table where the link needs to get inserted.
        QModelIndex labelIndex = pTableModel->index(labelLineIndex, columnIndex);
        QModelIndex filteredIndex = m_pIsaTableFilteringModel->mapFromSource(labelIndex);

        if (filteredIndex.isValid())
        {
            // Create a label with a richtext link within it.
            QLabel* pLinkLabel = new QLabel(ui.instructionsTreeView);
            pLinkLabel->setTextFormat(Qt::TextFormat::RichText);
            pLinkLabel->setOpenExternalLinks(false);
            pLinkLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

            // Set the richtext markup string to add a clickable URL to the label.
            std::stringstream linkMarkup;
            linkMarkup << "<a href=\"";
            linkMarkup << labelText;
            linkMarkup << ":\"><span style=\"text-decoration: underline; color:#0000ff;\">";
            linkMarkup << labelText;
            linkMarkup << "</span></a>";
            pLinkLabel->setText(linkMarkup.str().c_str());
            pLinkLabel->setFont(ui.instructionsTreeView->font());

            // Connect the link clicked handler to the new label.
            bool isConnected = connect(pLinkLabel, &QLabel::linkActivated, this, &rgIsaDisassemblyTableView::HandleBranchLinkClicked);
            assert(isConnected);

            // Insert the link label into the table.
            ui.instructionsTreeView->setIndexWidget(filteredIndex, pLinkLabel);

            // Add the new link to the list of label widgets.
            labelLinks.push_back(pLinkLabel);
        }
    }

    // Provide the list of inserted links so that the custom QTreeView knows where they're located within the table.
    ui.instructionsTreeView->SetLabelLinkWidgets(labelLinks);
}

bool rgIsaDisassemblyTableView::IsContiguousCorrelatedRangeSelected(int& correlatedLineIndex) const
{
    bool isNonContiguous = false;

    // Find the bounds of the selected row indices.
    int minRow = 0;
    int maxRow = 0;
    bool gotRange = GetSelectedRowRange(minRow, maxRow);
    if (gotRange)
    {
        // Step through each row in the range and determine which input source file line it's mapped to.
        int correlatedLine = kInvalidCorrelationLineIndex;
        for (int selectedRowIndex = minRow; selectedRowIndex <= maxRow; ++selectedRowIndex)
        {
            int inputLineIndex = 0;
            bool gotLine = m_pIsaTableModel->GetInputSourceLineNumberFromInstructionRow(selectedRowIndex, inputLineIndex);
            if (gotLine)
            {
                if (correlatedLine != kInvalidCorrelationLineIndex)
                {
                    if (inputLineIndex != correlatedLine)
                    {
                        // The instruction was emitted due to a different input source line. This isn't a contiguous block.
                        isNonContiguous = true;
                        correlatedLineIndex = kInvalidCorrelationLineIndex;
                        break;
                    }
                }
                else
                {
                    correlatedLine = inputLineIndex;
                    correlatedLineIndex = inputLineIndex;
                }
            }
        }
    }

    return !isNonContiguous;
}

void rgIsaDisassemblyTableView::ScrollToLine(int lineNumber)
{
    // Find the row of the index at the top left of the table.
    int firstVisibleRow = 0;
    const QModelIndex firstVisibleIndex = ui.instructionsTreeView->indexAt(ui.instructionsTreeView->rect().topLeft());
    if (firstVisibleIndex.isValid())
    {
        firstVisibleRow = firstVisibleIndex.row();
    }

    // When the lastVisibleIndex is invalid, it means that the table is already fully visible.
    // Initializing this to a large number ensures that we won't attempt to scroll the table when the lastVisibleIndex is invalid.
    int lastVisibleRow = INT_MAX;

    // Find the row of the index at the bottom left of the table.
    const QModelIndex lastVisibleIndex = ui.instructionsTreeView->indexAt(ui.instructionsTreeView->rect().bottomLeft());
    if (lastVisibleIndex.isValid())
    {
        lastVisibleRow = lastVisibleIndex.row();
    }

    // If the line we're scrolling to is already visible within view, don't scroll the view.
    bool isLineVisible = (lineNumber >= firstVisibleRow) && (lineNumber <= lastVisibleRow);
    if (!isLineVisible)
    {
        // Scroll the instruction table's vertical scrollbar to the given line.
        QScrollBar* pScrollbar = ui.instructionsTreeView->verticalScrollBar();
        pScrollbar->setValue(lineNumber);
    }
}

void rgIsaDisassemblyTableView::keyPressEvent(QKeyEvent* pEvent)
{
    Qt::KeyboardModifiers keyboardModifiers = QApplication::keyboardModifiers();
    if ((keyboardModifiers & Qt::ControlModifier) && (pEvent->key() == Qt::Key_C))
    {
        HandleCopyDisassemblyClicked();
    }
}
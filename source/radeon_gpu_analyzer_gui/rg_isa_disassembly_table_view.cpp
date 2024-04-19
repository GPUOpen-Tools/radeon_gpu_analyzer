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
#include "QtCommon/Util/QtUtil.h"
#include "QtCommon/Scaling/ScalingManager.h"

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_isa_disassembly_table_model.h"
#include "radeon_gpu_analyzer_gui/qt/rg_isa_disassembly_table_view.h"
#include "radeon_gpu_analyzer_gui/rg_data_types.h"
#include "radeon_gpu_analyzer_gui/rg_definitions.h"
#include "radeon_gpu_analyzer_gui/rg_string_constants.h"
#include "radeon_gpu_analyzer_gui/rg_utils.h"

// The bottom margin of the disassembly table.
static const int kTableBottomMargin = 5;

// A class used to filter the ISA disassembly table columns.
class RgIsaDisassemblyTableModelFilteringModel : public QSortFilterProxyModel
{
public:
    RgIsaDisassemblyTableModelFilteringModel(RgIsaDisassemblyTableModel* source_model, QObject* parent = nullptr) :
        QSortFilterProxyModel(parent), source_model_(source_model) {}
    ~RgIsaDisassemblyTableModelFilteringModel() = default;

protected:
    // A column-filtering predicate.
    virtual bool filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const override
    {
        Q_UNUSED(source_parent);

        // Ask the model if the given column index should be displayed.
        return source_model_->IsColumnVisible(source_column);
    }

public:
    // Invalidate the filtering model.
    void InvalidateModel()
    {
        beginResetModel();
        invalidate();
        endResetModel();
    }

private:
    // The source model being filtered.
    RgIsaDisassemblyTableModel* source_model_ = nullptr;
};

RgIsaDisassemblyTableView::RgIsaDisassemblyTableView(QWidget* parent) :
    QWidget(parent)
{
    ui_.setupUi(this);

    // Use the instruction treeview as the focus proxy for this view.
    setFocusProxy(ui_.instructionsTreeView);

    // Create the model holding the disassembly table data.
    isa_table_model_ = new RgIsaDisassemblyTableModel(0, this);
    ui_.instructionsTreeView->SetModel(isa_table_model_);

    // Create a column-filtering model to filter the table model.
    isa_table_filtering_model_ = new RgIsaDisassemblyTableModelFilteringModel(isa_table_model_, this);

    // Set the item model for the filtering proxy model.
    QStandardItemModel* item_model = isa_table_model_->GetTableModel();
    isa_table_filtering_model_->setSourceModel(item_model);

    // Set the filter model on the table.
    ui_.instructionsTreeView->setModel(isa_table_filtering_model_);

    // Allow the last column to stretch.
    ui_.instructionsTreeView->header()->setStretchLastSection(true);

    // Initialize the context menu.
    InitializeContextMenu();

    // Connect the table's selection model.
    ConnectSelectionSignals();

    // Connect the signals.
    ConnectSignals();
}

bool RgIsaDisassemblyTableView::IsLineInEntrypoint(int line_index)
{
    bool is_entry_point = false;

    // If the disassembly hasn't already been cached, load it.
    bool is_disassembly_loaded = IsDisassemblyLoaded();
    if (!is_disassembly_loaded)
    {
        // Load the disassembly.
        is_disassembly_loaded = LoadDisassembly(GetDisassemblyFilePath());
    }

    assert(is_disassembly_loaded);
    if (is_disassembly_loaded)
    {
        is_entry_point = isa_table_model_->IsSourceLineInEntrypoint(line_index);
    }

    bool is_live_vgpr_loaded = IsLiveVgprLoaded();
    if (!is_live_vgpr_loaded)
    {
        // Load the live VGPR data.
        is_live_vgpr_loaded = LoadLiveVgpr(GetLiveVgprsFilePath());
    }

    return is_entry_point;
}

void RgIsaDisassemblyTableView::InitializeModelData()
{
    isa_table_model_->InitializeModelData();
}

bool RgIsaDisassemblyTableView::LoadDisassembly(const std::string& disassembly_csv_file_path)
    {
    is_disassembly_cached_ = isa_table_model_->PopulateFromCsvFile(disassembly_csv_file_path);

    if (is_disassembly_cached_)
    {
        // Cache the path to the disassembly CSV file being loaded.
        disassembly_file_path_ = disassembly_csv_file_path;

        // Initialize jump link labels that need to be inserted into specific table ells.
        InitializeLinkLabels();

        // Adjust the table column widths after populating with data.
        QtCommon::QtUtil::AutoAdjustTableColumns(ui_.instructionsTreeView, 10, 20);
    }

    return is_disassembly_cached_;
}

bool RgIsaDisassemblyTableView::LoadLiveVgpr(const std::string& live_vgpr_file_path)
{
    is_live_vgpr_cached_ = isa_table_model_->LoadLiveVgprsData(live_vgpr_file_path);

    if (is_live_vgpr_cached_)
    {
       // Cache the path to the live Vgprs file being loaded.
        live_vgprs_file_path_ = live_vgpr_file_path;

        // Adjust the table column widths after populating with data.
        QtCommon::QtUtil::AutoAdjustTableColumns(ui_.instructionsTreeView, 10, 20);

        // Set the VGPR column width.
        SetVgprColumnWidth();
    }

    return is_live_vgpr_cached_;
}

void RgIsaDisassemblyTableView::SetVgprColumnWidth()
{
    static const int kVgprColumnWidthExtension = 150;

    // Fist check if the Live VGPR column is currently visible.
    std::shared_ptr<RgGlobalSettings> global_settings = RgConfigManager::Instance().GetGlobalConfig();
    assert(global_settings != nullptr);
    if (global_settings != nullptr)
    {
        int  vgpr_column = RgIsaDisassemblyTableModel::GetTableColumnIndex(RgIsaDisassemblyTableColumns::kLiveVgprs);
        bool is_visible  = global_settings->visible_disassembly_view_columns[vgpr_column];
        if (is_visible)
        {
            // Increase the width of Live VGPRs column to
            // encompass the colored widget drawn in the column.
            QHeaderView* header = ui_.instructionsTreeView->header();
            assert(header != nullptr);
            if (header != nullptr)
            {
                // Check the global settings to determine which disassembly table columns are visible.
                global_settings = RgConfigManager::Instance().GetGlobalConfig();
                assert(global_settings != nullptr);
                if (global_settings != nullptr)
                {
                    // Get the table model.
                    if (isa_table_model_ != nullptr)
                    {
                        QStandardItemModel* isa_table_model = isa_table_model_->GetTableModel();
                        assert(isa_table_model != nullptr);
                        if (isa_table_model != nullptr)
                        {
                            // VGPR column index to be calculated.
                            int vgpr_index = 0;

                            // Get start and end column.
                            int start_column = static_cast<int>(RgIsaDisassemblyTableColumns::kAddress);
                            int end_column   = static_cast<int>(RgIsaDisassemblyTableColumns::kCount);

                            // Calculate the VGPR column index from the currently displayed columns.
                            for (int column_index = start_column; column_index < end_column; ++column_index)
                            {
                                if (global_settings->visible_disassembly_view_columns[column_index])
                                {
                                    // Compare the header name for each column until the VGPR column is found.
                                    QStandardItem* item  = isa_table_model->horizontalHeaderItem(column_index);
                                    assert(item != nullptr);
                                    if (item != nullptr)
                                    {
                                        QString title = item->text();
                                        if (title.contains(kStrDisassemblyTableLiveVgprHeaderPart))
                                        {
                                            // This is the VGPR column, so break out of here.
                                            break;
                                        }
                                        vgpr_index++;
                                    }
                                }
                            }

                            // Set header resize mode.
                            header->setSectionResizeMode(QHeaderView::Interactive);

                            // Set column width for the VGPR column.
                            int column_width = ui_.instructionsTreeView->columnWidth(vgpr_index);
                            ui_.instructionsTreeView->setColumnWidth(vgpr_index, column_width + kVgprColumnWidthExtension);

                            // Set the mode to fixed so the user cannot resize the VGPR
                            // column smaller than it needs to be to display the color swatch.
                            header->setSectionResizeMode(vgpr_index, QHeaderView::Fixed);
                        }
                    }
                }
            }
        }
    }
}

void RgIsaDisassemblyTableView::RequestTableResize()
{
    // The maximum number of rows to measure when computing the width of each column.
    static const int kMaxNumRows = 32;

    // The amount of extra padding to add when resizing.
    static const int kColumnPadding = 20;

    // Compute the ideal width of the table and emit a signal to readjust the view dimensions.
    int min_width = QtCommon::QtUtil::ComputeMinimumTableWidth(ui_.instructionsTreeView, kMaxNumRows, kColumnPadding);
    emit DisassemblyTableWidthResizeRequested(min_width);
}

void RgIsaDisassemblyTableView::UpdateCorrelatedSourceFileLine(int line_number)
{
    // Set the highlighted lines in the table model.
    bool is_correlated = isa_table_model_->SetCorrelatedSourceLineIndex(line_number);

    // Scroll to the highlighted lines only if the high-level source file's current table has a correlated line with the disassembly table.
    if (is_correlated)
    {
        const std::vector<int>& disassembly_line_indices = isa_table_model_->GetCorrelatedLineIndices();
        if (!disassembly_line_indices.empty())
        {
            // Extract the first highlighted Isa row index, and scroll to it.
            int first_highlighted_disassembly_row_index = disassembly_line_indices[0];
            ScrollToLine(first_highlighted_disassembly_row_index);
        }
    }

    // Trigger an update of the treeview.
    ui_.instructionsTreeView->update();
}

void RgIsaDisassemblyTableView::UpdateFilteredTable()
{
    // Invalidate the table's filtering model, since column visibility has changed.
    isa_table_filtering_model_->invalidate();

    // Add all label rows into the table after filtering.
    InitializeLabelRows();

    // Add all link labels into the table after filtering.
    InitializeLinkLabels();

    // Adjust the table column widths after populating with data.
    QtCommon::QtUtil::AutoAdjustTableColumns(ui_.instructionsTreeView, 32, 20);

    // Set the VGPR column width.
    SetVgprColumnWidth();
}

std::string RgIsaDisassemblyTableView::GetDisassemblyFilePath() const
{
    return disassembly_file_path_;
}

std::string RgIsaDisassemblyTableView::GetLiveVgprsFilePath() const
{
    return live_vgprs_file_path_;
}

void RgIsaDisassemblyTableView::SetDisassemblyFilePath(const std::string& disassembly_file_path)
{
    disassembly_file_path_ = disassembly_file_path;
}

void RgIsaDisassemblyTableView::SetLiveVgprsFilePath(const std::string& live_vgprs_file_path)
{
    live_vgprs_file_path_ = live_vgprs_file_path;
}

bool RgIsaDisassemblyTableView::IsDisassemblyLoaded() const
{
    return is_disassembly_cached_;
}

bool RgIsaDisassemblyTableView::IsLiveVgprLoaded() const
{
    return is_live_vgpr_cached_;
}

bool RgIsaDisassemblyTableView::IsSourceLineCorrelated(int line_index) const
{
    assert(isa_table_model_ != nullptr);
    return (isa_table_model_ != nullptr ? isa_table_model_->IsSourceLineCorrelated(line_index) : false);
}

void RgIsaDisassemblyTableView::HandleBranchLinkClicked(const QString& link)
{
    assert(isa_table_model_ != nullptr);
    if (isa_table_model_ != nullptr)
    {
        // Obtain the map of all labels within the disassembly table.
        std::map<std::string, int> label_name_to_line_number;
        isa_table_model_->GetLabelNameToLineIndexMap(label_name_to_line_number);

        // Find the given link name within the map, and scroll to the line index.
        auto lineIndexIter = label_name_to_line_number.find(link.toStdString());
        if (lineIndexIter != label_name_to_line_number.end())
        {
            int label_line_number = lineIndexIter->second;

            // Advance the focused line by 1. That's where the label's first line of code is.
            int label_code_line = label_line_number + 1;

            // Retrieve the source line number associated with the label's ISA line number.
            int input_source_line_number = kInvalidCorrelationLineIndex;
            isa_table_model_->GetInputSourceLineNumberFromInstructionRow(label_code_line, input_source_line_number);

            // Emit the signal used to highlight the label's correlated source code.
            emit InputSourceHighlightedLineChanged(input_source_line_number);

            // Scroll the table so that the label is visible.
            ScrollToLine(label_line_number);
        }
    }
}

void RgIsaDisassemblyTableView::HandleCopyDisassemblyClicked()
{
    QVector<int> selected_row_numbers;

    QItemSelectionModel* selection_model = ui_.instructionsTreeView->selectionModel();
    const QItemSelection selection = selection_model->selection();
    if (!selection.isEmpty())
    {
        // Get the selected line numbers.
        QItemSelectionModel* table_selection_model = ui_.instructionsTreeView->selectionModel();
        QModelIndexList selected_rows = table_selection_model->selectedRows();
        for (auto& current_index : selected_rows)
        {
            int row_index = current_index.row();
            selected_row_numbers << row_index;
        }

        // Make sure the row numbers are in descending order since the user could've made selections out of order.
        std::sort(selected_row_numbers.begin(), selected_row_numbers.end());

        // Copy the range of row data to the user's clipboard.
        isa_table_model_->CopyRowsToClipboard(selected_row_numbers);
    }
}

void RgIsaDisassemblyTableView::HandleOpenDisassemblyInFileBrowserClicked()
{
    // Use the path to the loaded CSV file to figure out which folder to open.
    std::string build_output_directory;
    bool is_ok = RgUtils::ExtractFileDirectory(disassembly_file_path_, build_output_directory);
    assert(is_ok);
    if (is_ok)
    {
        // Open the directory in the system's file browser.
        RgUtils::OpenFolderInFileBrowser(build_output_directory);
    }
}

void RgIsaDisassemblyTableView::HandleCurrentSelectionChanged(const QItemSelection& selected, const QItemSelection& /*deselected*/)
{
    Q_UNUSED(selected);

    // Use the model's current selection to check what needs to be highlighted.
    QItemSelectionModel* selection_model = ui_.instructionsTreeView->selectionModel();
    assert(selection_model != nullptr);
    if (selection_model != nullptr)
    {
        const QItemSelection& current_selection = selection_model->selection();

        QModelIndexList selected_indices = current_selection.indexes();
        if (!selected_indices.isEmpty())
        {
            QModelIndex first_selected_index = selected_indices[0];

            if (first_selected_index.isValid())
            {
                int first_selected_row_index = first_selected_index.row();

                // Determine which line in the input source file is correlated with the given Isa row.
                int input_source_line_number = kInvalidCorrelationLineIndex;
                isa_table_model_->GetInputSourceLineNumberFromInstructionRow(first_selected_row_index, input_source_line_number);

                // Did the user select a line that isn't currently highlighted?
                const std::vector<int>& indices = isa_table_model_->GetCorrelatedLineIndices();
                auto correlated_lines_iter = std::find(indices.begin(), indices.end(), first_selected_row_index);
                if (correlated_lines_iter == indices.end())
                {
                    // Invalidate the currently highlighted lines.
                    isa_table_model_->SetCorrelatedSourceLineIndex(kInvalidCorrelationLineIndex);
                }

                // Do the instruction lines in the selected block all contiguously map to the same input source file line?
                int correlated_line_index = kInvalidCorrelationLineIndex;
                bool is_contiguous_block_selected = IsContiguousCorrelatedRangeSelected(correlated_line_index);

                if (is_contiguous_block_selected)
                {
                    // Emit a signal indicating that the input source file's correlation highlight line should be updated.
                    emit InputSourceHighlightedLineChanged(input_source_line_number);
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

void RgIsaDisassemblyTableView::HandleOpenContextMenu(const QPoint& widget_click_position)
{
    // Convert the widget's local click position to the global screen position.
    const QPoint click_point = mapToGlobal(widget_click_position);

    // Open the context menu at the user's click position.
    context_menu_->exec(click_point);
}

void RgIsaDisassemblyTableView::ConnectContextMenuSignals()
{
    // Connect the handler responsible for triggering a copy to clipboard from the table.
    bool is_connected = connect(copy_selected_disassembly_, &QAction::triggered, this, &RgIsaDisassemblyTableView::HandleCopyDisassemblyClicked);
    assert(is_connected);

    // Connect the handler responsible for opening the current disassembly build output folder in the system file browser.
    is_connected = connect(open_disassembly_in_file_browser_, &QAction::triggered, this, &RgIsaDisassemblyTableView::HandleOpenDisassemblyInFileBrowserClicked);
    assert(is_connected);

    // Connect the handler responsible for showing the max VGPR line.
    is_connected = connect(show_maximum_vgpr_lines_, &QAction::triggered, this, &RgIsaDisassemblyTableView::HandleShowNextMaxVgprSignal);
    assert(is_connected);

    // Use a custom context menu for the table.
    this->setContextMenuPolicy(Qt::CustomContextMenu);

    // Connect the handler responsible for showing the table's context menu.
    is_connected = connect(this, &QWidget::customContextMenuRequested, this, &RgIsaDisassemblyTableView::HandleOpenContextMenu);
    assert(is_connected);
}

void RgIsaDisassemblyTableView::ConnectSelectionSignals()
{
    QItemSelectionModel* selection_model = ui_.instructionsTreeView->selectionModel();
    assert(selection_model != nullptr);
    if (selection_model != nullptr)
    {
        // Connect the table's selection changed handler.
        bool is_connected = connect(selection_model, &QItemSelectionModel::selectionChanged, this, &RgIsaDisassemblyTableView::HandleCurrentSelectionChanged);
        assert(is_connected);
    }
}

void RgIsaDisassemblyTableView::ConnectSignals()
{
    // Connect the disassembly table's focus in signal.
    bool is_connected = connect(ui_.instructionsTreeView, &RgIsaDisassemblyCustomTableView::FrameFocusInSignal, this, &RgIsaDisassemblyTableView::FrameFocusInSignal);
    assert(is_connected);

    // Connect the disassembly table's focus out signal.
    is_connected = connect(ui_.instructionsTreeView, &RgIsaDisassemblyCustomTableView::FrameFocusOutSignal, this, &RgIsaDisassemblyTableView::FrameFocusOutSignal);
    assert(is_connected);

    // Connect the disassembly table view's enable scroll bar signal.
    is_connected = connect(this, &RgIsaDisassemblyTableView::EnableScrollbarSignals, ui_.instructionsTreeView, &RgIsaDisassemblyCustomTableView::EnableScrollbarSignals);
    assert(is_connected);

    // Connect the disassembly table view's disable scroll bar signal.
    is_connected = connect(this, &RgIsaDisassemblyTableView::DisableScrollbarSignals, ui_.instructionsTreeView, &RgIsaDisassemblyCustomTableView::DisableScrollbarSignals);
    assert(is_connected);

    // Connect the disassembly table's target GPU push button focus in signal.
    is_connected = connect(ui_.instructionsTreeView, &RgIsaDisassemblyCustomTableView::FocusTargetGpuPushButton, this, &RgIsaDisassemblyTableView::FocusTargetGpuPushButton);
    assert(is_connected);

    // Connect the disassembly table's target GPU push button focus in signal.
    is_connected = connect(ui_.instructionsTreeView, &RgIsaDisassemblyCustomTableView::SwitchDisassemblyContainerSize, this, &RgIsaDisassemblyTableView::SwitchDisassemblyContainerSize);
    assert(is_connected);

    // Connect the disassembly table's columns push button focus in signal.
    is_connected = connect(ui_.instructionsTreeView, &RgIsaDisassemblyCustomTableView::FocusColumnPushButton, this, &RgIsaDisassemblyTableView::FocusColumnPushButton);
    assert(is_connected);

    // Connect the disassembly table's source window focus in signal.
    is_connected = connect(ui_.instructionsTreeView, &RgIsaDisassemblyCustomTableView::FocusSourceWindow, this, &RgIsaDisassemblyTableView::FocusSourceWindow);
    assert(is_connected);

    // Connect the disassembly table view's update current sub widget signal.
    is_connected = connect(this, &RgIsaDisassemblyTableView::UpdateCurrentSubWidget, ui_.instructionsTreeView, &RgIsaDisassemblyCustomTableView::HandleUpdateCurrentSubWidget);
    assert(is_connected);

    // Connect the disassembly table's focus cli output window signal.
    is_connected = connect(ui_.instructionsTreeView, &RgIsaDisassemblyCustomTableView::FocusCliOutputWindow, this, &RgIsaDisassemblyTableView::FocusCliOutputWindow);
    assert(is_connected);
}

bool RgIsaDisassemblyTableView::GetSelectedRowRange(int& min_row, int& max_row) const
{
    bool got_range = false;

    QItemSelectionModel* table_selection_model = ui_.instructionsTreeView->selectionModel();
    auto rows_selection = table_selection_model->selectedRows();

    if (!rows_selection.isEmpty())
    {
        // Find the bounds of the selected row indices.
        min_row = INT_MAX;
        max_row = INT_MIN;
        for (QModelIndex& current_index : rows_selection)
        {
            int row_index = current_index.row();
            min_row = (row_index < min_row) ? row_index : min_row;
            max_row = (row_index > max_row) ? row_index : max_row;
        }

        got_range = true;
    }

    return got_range;
}

void RgIsaDisassemblyTableView::InitializeContextMenu()
{
    // Create the context menu instance.
    context_menu_ = new QMenu(this);

    // Set the cursor for the context menu.
    context_menu_->setCursor(Qt::PointingHandCursor);

    // Create the menu items to insert into the context menu.
    copy_selected_disassembly_ = new QAction(kStrDisassemblyTableContextMenuCopy, this);
    context_menu_->addAction(copy_selected_disassembly_);

    // Create an item allowing the user to jump to highest VGPR pressure line.
    show_maximum_vgpr_lines_ = new QAction(kStrDisassemblyTableContextMenuGoToMaxVgpr, this);
    context_menu_->addAction(show_maximum_vgpr_lines_);

    // Create an item allowing the user to browse to the disassembly build output folder.
    open_disassembly_in_file_browser_ = new QAction(kStrDisassemblyTableContextMenuOpenInFileBrowser, this);
    context_menu_->addAction(open_disassembly_in_file_browser_);

    // Connect the context menu signals.
    ConnectContextMenuSignals();
}

void RgIsaDisassemblyTableView::EnableShowMaxVgprContextOption(bool is_enabled) const
{
    show_maximum_vgpr_lines_->setEnabled(is_enabled);
}

void RgIsaDisassemblyTableView::InitializeLabelRows()
{
    // Clear existing label lines from the model.
    isa_table_model_->ClearLabelLines();

    // Always insert labels into the left-most visible column in the table.
    isa_table_model_->InsertLabelRows();
}

void RgIsaDisassemblyTableView::InitializeLinkLabels()
{
    // A map of row index to the label's name being linked to in the operands column.
    std::map<int, std::string> link_labels;
    isa_table_model_->GetLineIndexToLabelNameMap(link_labels);

    int column_index = RgIsaDisassemblyTableModel::GetTableColumnIndex(RgIsaDisassemblyTableColumns::kOperands);

    // Hold a vector of all Label links inserted into the table.
    std::vector<QLabel*> labelLinks;

    auto start_label = link_labels.begin();
    auto end_label = link_labels.end();
    for (auto labels_iter = start_label; labels_iter != end_label; ++labels_iter)
    {
        // Extract the line number and label text from the map.
        int label_line_index = labels_iter->first;
        std::string& label_text = labels_iter->second;

        QStandardItemModel* table_model = isa_table_model_->GetTableModel();

        // Compute the index in the table where the link needs to get inserted.
        QModelIndex label_index = table_model->index(label_line_index, column_index);
        QModelIndex filtered_index = isa_table_filtering_model_->mapFromSource(label_index);

        if (filtered_index.isValid())
        {
            // Create a label with a richtext link within it.
            QLabel* link_label = new QLabel(ui_.instructionsTreeView);
            link_label->setTextFormat(Qt::TextFormat::RichText);
            link_label->setOpenExternalLinks(false);
            link_label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

            // Set the richtext markup string to add a clickable URL to the label.
            std::stringstream line_markup;
            line_markup << "<a href=\"";
            line_markup << label_text;
            line_markup << ":\"><span style=\"text-decoration: underline; color:#0000ff;\">";
            line_markup << label_text;
            line_markup << "</span></a>";
            link_label->setText(line_markup.str().c_str());
            link_label->setFont(ui_.instructionsTreeView->font());

            // Connect the link clicked handler to the new label.
            bool is_connected = connect(link_label, &QLabel::linkActivated, this, &RgIsaDisassemblyTableView::HandleBranchLinkClicked);
            assert(is_connected);

            // Insert the link label into the table.
            ui_.instructionsTreeView->setIndexWidget(filtered_index, link_label);

            // Add the new link to the list of label widgets.
            labelLinks.push_back(link_label);
        }
    }

    // Provide the list of inserted links so that the custom QTreeView knows where they're located within the table.
    ui_.instructionsTreeView->SetLabelLinkWidgets(labelLinks);
}

bool RgIsaDisassemblyTableView::IsContiguousCorrelatedRangeSelected(int& correlated_line_index) const
{
    bool is_non_contiguous = false;

    // Find the bounds of the selected row indices.
    int min_row = 0;
    int max_row = 0;
    bool got_range = GetSelectedRowRange(min_row, max_row);
    if (got_range)
    {
        // Step through each row in the range and determine which input source file line it's mapped to.
        int correlated_line = kInvalidCorrelationLineIndex;
        for (int selected_row_index = min_row; selected_row_index <= max_row; ++selected_row_index)
        {
            int input_line_index = 0;
            bool got_line = isa_table_model_->GetInputSourceLineNumberFromInstructionRow(selected_row_index, input_line_index);
            if (got_line)
            {
                if (correlated_line != kInvalidCorrelationLineIndex)
                {
                    if (input_line_index != correlated_line)
                    {
                        // The instruction was emitted due to a different input source line. This isn't a contiguous block.
                        is_non_contiguous = true;
                        correlated_line_index = kInvalidCorrelationLineIndex;
                        break;
                    }
                }
                else
                {
                    correlated_line = input_line_index;
                    correlated_line_index = input_line_index;
                }
            }
        }
    }

    return !is_non_contiguous;
}

void RgIsaDisassemblyTableView::ScrollToLine(int line_number)
{
    // Find the row of the index at the top left of the table.
    int first_visible_row = 0;
    const QModelIndex first_visible_index = ui_.instructionsTreeView->indexAt(ui_.instructionsTreeView->rect().topLeft());
    if (first_visible_index.isValid())
    {
        first_visible_row = first_visible_index.row();
    }

    // When the lastVisibleIndex is invalid, it means that the table is already fully visible.
    // Initializing this to a large number ensures that we won't attempt to scroll the table when the lastVisibleIndex is invalid.
    int last_visible_row = INT_MAX;

    // Find the row of the index at the bottom left of the table.
    const QModelIndex last_visible_index = ui_.instructionsTreeView->indexAt(ui_.instructionsTreeView->rect().bottomLeft());
    if (last_visible_index.isValid())
    {
        last_visible_row = last_visible_index.row();
    }

    // If the line we're scrolling to is already visible within view, don't scroll the view.
    const int adjust_visible_row          = kTableBottomMargin * ScalingManager::Get().GetScaleFactor();
    bool      is_line_visible = (line_number >= first_visible_row) && (line_number <= last_visible_row - adjust_visible_row);
    if (!is_line_visible)
    {
        // Scroll the instruction table's vertical scrollbar to the given line.
        QScrollBar* scroll_bar = ui_.instructionsTreeView->verticalScrollBar();
        scroll_bar->setValue(line_number);
    }
}

void RgIsaDisassemblyTableView::keyPressEvent(QKeyEvent* event)
{
    Qt::KeyboardModifiers keyboard_modifiers = QApplication::keyboardModifiers();
    if ((keyboard_modifiers & Qt::ControlModifier) && (event->key() == Qt::Key_C))
    {
        HandleCopyDisassemblyClicked();
    }
}

void RgIsaDisassemblyTableView::ResetCurrentMaxVgprIndex()
{
    isa_table_model_->ResetCurrentMaxVgprValues();
}

bool RgIsaDisassemblyTableView::IsShowCurrentMaxVgprEnabled() const
{
    return isa_table_model_->IsShowCurrentMaxVgprEnabled();
}

void RgIsaDisassemblyTableView::HandleShowNextMaxVgprSignal()
{
    // Need to scroll to the next line.
    int  line_number   = 0;
    bool is_valid_line = isa_table_model_->GetNextMaxVgprLineNumber(line_number);

    // Scroll to the line if it is a valid line.
    if (is_valid_line)
    {
        ScrollToLine(line_number);
    }

    // Set the current max vgpr line background color.
    isa_table_model_->HighlightCurrentMaxVgprLine();

    // Trigger an update of the treeview.
    ui_.instructionsTreeView->update();
}

void RgIsaDisassemblyTableView::HandleShowPreviousMaxVgprSignal()
{
    // Need to scroll to the next line.
    int  line_number   = 0;
    bool is_valid_line = isa_table_model_->GetPreviousMaxVgprLineNumber(line_number);

    // Scroll to the line if it is a valid line.
    if (is_valid_line)
    {
        ScrollToLine(line_number);
    }

    // Set the current max vgpr line background color.
    isa_table_model_->HighlightCurrentMaxVgprLine();

    // Trigger an update of the treeview.
    ui_.instructionsTreeView->update();
}
// C++.
#include <algorithm>
#include <cassert>
#include <sstream>

// Qt.
#include <QClipboard>
#include <QFile>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QTextStream>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_isa_disassembly_table_model.h"
#include "radeon_gpu_analyzer_gui/qt/rg_output_file_utils.h"
#include "radeon_gpu_analyzer_gui/rg_config_manager.h"
#include "radeon_gpu_analyzer_gui/rg_definitions.h"
#include "radeon_gpu_analyzer_gui/rg_data_types.h"
#include "radeon_gpu_analyzer_gui/rg_string_constants.h"
#include "radeon_gpu_analyzer_gui/rg_utils.h"
#include "ui_rg_isa_disassembly_table_view.h"

// Shared with CLI.
#include "source/common/rga_cli_defs.h"

// The color to use for label backgrounds within the table.
static const QColor kBranchLabelInstructionColor = QColor(37, 36, 225);
static const QColor kBranchLabelBackgroundColor = QColor("lightGray").lighter(113);

RgIsaDisassemblyTableModel::RgIsaDisassemblyTableModel(uint32_t model_count, QWidget* parent) :
    ModelViewMapper(model_count)
{
    // Create the table model.
    isa_table_model_ = new QStandardItemModel(0, GetTableColumnIndex(RgIsaDisassemblyTableColumns::kCount), parent);

    // Add column headers to the table.
    isa_table_model_->setHorizontalHeaderItem(GetTableColumnIndex(RgIsaDisassemblyTableColumns::kAddress),        new QStandardItem(kStrDisassemblyTableColumnAddress));
    isa_table_model_->setHorizontalHeaderItem(GetTableColumnIndex(RgIsaDisassemblyTableColumns::kOpcode),         new QStandardItem(kStrDisassemblyTableColumnOpcode));
    isa_table_model_->setHorizontalHeaderItem(GetTableColumnIndex(RgIsaDisassemblyTableColumns::kOperands),       new QStandardItem(kStrDisassemblyTableColumnOperands));
    isa_table_model_->setHorizontalHeaderItem(GetTableColumnIndex(RgIsaDisassemblyTableColumns::kFunctionalUnit), new QStandardItem(kStrDisassemblyTableColumnFunctionalUnit));
    isa_table_model_->setHorizontalHeaderItem(GetTableColumnIndex(RgIsaDisassemblyTableColumns::kCycles),         new QStandardItem(kStrDisassemblyTableColumnCycles));
    isa_table_model_->setHorizontalHeaderItem(GetTableColumnIndex(RgIsaDisassemblyTableColumns::kBinaryEncoding), new QStandardItem(kStrDisassemblyTableColumnBinaryEncoding));
    isa_table_model_->setHorizontalHeaderItem(GetTableColumnIndex(RgIsaDisassemblyTableColumns::kLiveVgprs),      new QStandardItem(kStrDisassemblyTableLiveVgprs));
}

const std::vector<int>& RgIsaDisassemblyTableModel::GetCorrelatedLineIndices() const
{
    return correlated_isa_line_indices_;
}

QStandardItemModel* RgIsaDisassemblyTableModel::GetTableModel() const
{
    return isa_table_model_;
}

bool RgIsaDisassemblyTableModel::GetInputSourceLineNumberFromInstructionRow(int instruction_line_index, int& input_source_line_number)
{
    bool ret = false;

    auto input_source_line_iter = instruction_line_number_to_input_source_line_number_.find(instruction_line_index);
    if (input_source_line_iter != instruction_line_number_to_input_source_line_number_.end())
    {
        // Return the input source file's line number that corresponds with the given disassembly line number.
        input_source_line_number = input_source_line_iter->second;
        ret = true;
    }

    return ret;
}

void RgIsaDisassemblyTableModel::GetLabelNameToLineIndexMap(std::map<std::string, int>& link_labels) const
{
    int num_lines = static_cast<int>(disassembled_isa_lines_.size());
    for (int line_index = 0; line_index < num_lines; ++line_index)
    {
        std::shared_ptr<RgIsaLine> current_line = disassembled_isa_lines_[line_index];
        if (current_line->type == RgIsaLineType::kLabel)
        {
            // Cast the line into a label line.
            std::shared_ptr<RgIsaLineLabel> label_line = std::static_pointer_cast<RgIsaLineLabel>(current_line);

            // Extract the label name and insert into the output map.
            std::string label_name = label_line->label_name;
            link_labels[label_name] = line_index;
        }
    }
}

void RgIsaDisassemblyTableModel::GetLineIndexToLabelNameMap(std::map<int, std::string>& link_labels) const
{
    // Step through each disassembled line and collect all label line numbers, and their destination operand.
    int num_lines = static_cast<int>(disassembled_isa_lines_.size());
    for (int line_index = 0; line_index < num_lines; ++line_index)
    {
        std::shared_ptr<RgIsaLine> current_line = disassembled_isa_lines_[line_index];
        if (current_line->type == RgIsaLineType::kInstruction)
        {
            // Look for branch instructions, and extract the destination from the operands text.
            std::shared_ptr<RgIsaLineInstruction> instruction_line = std::static_pointer_cast<RgIsaLineInstruction>(current_line);
            if (instruction_line->functional_unit.compare("Branch") == 0)
            {
                std::string jump_destination = instruction_line->operands;
                link_labels[line_index] = jump_destination;
            }
        }
    }
}

void RgIsaDisassemblyTableModel::InsertLabelRows()
{
    // Try to insert labels into the start column if possible.
    int start_column = GetTableColumnIndex(RgIsaDisassemblyTableColumns::kAddress);
    int end_column = GetTableColumnIndex(RgIsaDisassemblyTableColumns::kCount);

    // The column index to insert the labels into.
    label_column_index_ = start_column;

    // Check the global settings to determine which disassembly table columns are visible.
    std::shared_ptr<RgGlobalSettings> global_settings = RgConfigManager::Instance().GetGlobalConfig();
    assert(global_settings != nullptr);
    if (global_settings != nullptr)
    {
        // Determine which column is the left-most visible column in the table.
        for (int column_index = start_column; column_index < end_column; ++column_index)
        {
            bool is_valid_column_index = (column_index >= start_column) && (column_index < end_column);
            bool is_visible = global_settings->visible_disassembly_view_columns[column_index];
            if (is_visible)
            {
                label_column_index_ = column_index;
                break;
            }
        }
    }

    // Step through each ISA line and update the model data if it's a label row.
    int row_count = static_cast<int>(disassembled_isa_lines_.size());
    for (int row_index = 0; row_index < row_count; ++row_index)
    {
        std::shared_ptr<RgIsaLine> isa_line = disassembled_isa_lines_[row_index];

        if (isa_line->type == RgIsaLineType::kLabel)
        {
            // Cast into a RgIsaLineLabel to extract the label name.
            std::shared_ptr<RgIsaLineLabel> label_line = std::static_pointer_cast<RgIsaLineLabel>(isa_line);

            // Add the label text to the model.
            SetTableModelText(label_line->label_name, row_index, label_column_index_);

            // Set the background color to yellow to make the label stand out more.
            SetTableModelBackgroundColor(kBranchLabelBackgroundColor, row_index, label_column_index_);
        }
    }
}

int RgIsaDisassemblyTableModel::GetTableColumnIndex(RgIsaDisassemblyTableColumns column)
{
    return static_cast<int>(column);
}

bool RgIsaDisassemblyTableModel::IsBranchOperandItem(const QModelIndex& model_index)
{
    bool is_label_link_item = false;

    // Find the disassembly row for the item.
    int row_index = model_index.row();
    std::shared_ptr<RgIsaLine> isa_line = disassembled_isa_lines_[row_index];

    // Determine what kind of instruction is represented in this row.
    if (isa_line->type == RgIsaLineType::kInstruction)
    {
        // Cast the line to access the instruction data.
        std::shared_ptr<RgIsaLineInstruction> instruction_line = std::static_pointer_cast<RgIsaLineInstruction>(isa_line);

        // Is this a branch instruction?
        if (instruction_line->functional_unit.compare("Branch") == 0)
        {
            // Is the given index in the Operands column? That's the column a label needs to be painted in.
            int column_index = model_index.column();
            bool is_operands_column = column_index == GetTableColumnIndex(RgIsaDisassemblyTableColumns::kOperands);
            if (is_operands_column)
            {
                is_label_link_item = true;
            }
        }
    }

    return is_label_link_item;
}

bool RgIsaDisassemblyTableModel::IsColumnVisible(int column_index) const
{
    bool is_visible = false;

    // Check the global config settings to see if the column should be visible.
    RgConfigManager& config_manager = RgConfigManager::Instance();
    std::shared_ptr<RgGlobalSettings> global_settings = config_manager.GetGlobalConfig();
    assert(global_settings != nullptr);
    if (global_settings != nullptr)
    {
        // Verify that the incoming column index is valid.
        bool is_valid_index = (column_index >= 0) && (column_index < global_settings->visible_disassembly_view_columns.size());
        assert(is_valid_index);
        if (is_valid_index)
        {
            is_visible = global_settings->visible_disassembly_view_columns[column_index];
        }
    }

    return is_visible;
}

bool RgIsaDisassemblyTableModel::IsIsaLineCorrelated(int line_index) const
{
    bool ret = false;

    // Check if the given line index exists in the current cache of correlated line indices.
    auto line_iter = std::find(correlated_isa_line_indices_.begin(), correlated_isa_line_indices_.end(), line_index);
    if (line_iter != correlated_isa_line_indices_.end())
    {
        ret = true;
    }

    return ret;
}

bool RgIsaDisassemblyTableModel::IsCurrentMaxVgprLine(int line_index) const
{
    bool status = false;

    // Check if the given line index exists in the current cache of max VGPR lines numbers.
    auto line_iter = std::find(livereg_data_.max_vgpr_line_numbers.begin(), livereg_data_.max_vgpr_line_numbers.end(), line_index);
    if (line_iter != livereg_data_.max_vgpr_line_numbers.end())
    {
        const int index = line_iter - livereg_data_.max_vgpr_line_numbers.begin();
        if (livereg_data_.is_current_max_vgpr_line_number.at(index))
        {
            // If this is the current maximum VGPR line, set status to true.
            status = true;
        }
    }

    return status;
}

bool RgIsaDisassemblyTableModel::IsSourceLineCorrelated(int line_index) const
{
    return (input_source_line_index_to_instruction_line_indices_.find(line_index) != input_source_line_index_to_instruction_line_indices_.end());
}

bool RgIsaDisassemblyTableModel::IsSourceLineInEntrypoint(int line_index) const
{
    bool ret = false;

    // Does the given line fall within the line number bounds for this entrypoint?
    if ((line_index >= source_file_entrypoint_start_line_) && (line_index <= source_file_entrypoint_end_line_))
    {
        ret = true;
    }

    return ret;
}

std::vector<int> RgIsaDisassemblyTableModel::GetMaximumVgprLineNumbers() const
{
    //  Return the line numbers for the max VGPR values.
    return livereg_data_.max_vgpr_line_numbers;
}

bool RgIsaDisassemblyTableModel::PopulateFromCsvFile(const std::string& csv_file_full_path)
{
    // Attempt to load the CSV file, and then update the model's data.
    bool status = LoadCsvData(csv_file_full_path);
    assert(status);

    return status;
}

bool RgIsaDisassemblyTableModel::LoadLiveVgprsData(const std::string& live_vgpr_file_full_path)
{
    bool status = false;

    // If the live vgpr file is not empty, attempt to load it, and then update the model's data.
    if (!live_vgpr_file_full_path.empty())
    {
        // Parse the live VGPR data.
        status = RgOutputFileUtils::ParseLiveVgprsData(live_vgpr_file_full_path,
                                                       disassembled_isa_lines_,
                                                       vgpr_isa_lines_,
                                                       vgpr_file_lines_,
                                                       livereg_data_);
        assert(status);

        // Populate the view with live VGPR data.
        PopulateLiveVgprData();
    }

    return status;
}

void RgIsaDisassemblyTableModel::PopulateLiveVgprData()
{
    // Update the table header to show the max number of VGPRs used.
    QStandardItem* item = isa_table_model_->horizontalHeaderItem(GetTableColumnIndex(RgIsaDisassemblyTableColumns::kLiveVgprs));
    assert(item != nullptr);
    if (item != nullptr)
    {
        QString max_string = QString(kStrDisassemblyTableLiveVgprs)
                                 .arg(QString::number(livereg_data_.used))
                                 .arg(QString::number(livereg_data_.allocated) + "/" + QString::number(livereg_data_.total_vgprs));
        item->setText(max_string);

        // Show the hazard icon if we've hit more than the max live VGPRs.
        if (livereg_data_.max_vgprs >= livereg_data_.total_vgprs)
        {
            // Add the VGPR notification icon.
            item->setIcon(QIcon(kIconMaxVgprNotification));

            // Add a tooltip to the VGPR column.
            item->setToolTip(QString(kLiveVgprMaxVgprTooltip));
        }

        // Show the warning icon and a tooltip if instructions did not match.
        if (livereg_data_.unmatched_count > 0)
        {
            // Add the VGPR notification icon.
            item->setIcon(QIcon(kIconMaxVgprNoIsaMatch));

            // Add a tooltip to the VGPR column.
            item->setToolTip(QString(kLiveVgprNATooltip).arg(livereg_data_.unmatched_count));
        }

        assert(isa_table_model_ != nullptr);
        if (isa_table_model_ != nullptr)
        {
            // Set the Live VGPR column header.
            isa_table_model_->setHorizontalHeaderItem(GetTableColumnIndex(RgIsaDisassemblyTableColumns::kLiveVgprs), item);
        }
    }
}

bool RgIsaDisassemblyTableModel::SetCorrelatedSourceLineIndex(int line_index)
{
    bool is_correlated = false;

    // Determine the set of ISA instructions that should become highlighted.
    std::vector<int> is_line_indices;
    is_correlated = GetDisassemblyLineIndicesFromInputSourceLine(line_index, is_line_indices);

    if (is_correlated)
    {
        // Assign the list of highlighted lines to paint.
        correlated_isa_line_indices_ = is_line_indices;
    }
    else
    {
        // Clear the list and highlight nothing.
        correlated_isa_line_indices_.clear();
    }

    return is_correlated;
}

bool RgIsaDisassemblyTableModel::GetDisassemblyLineIndicesFromInputSourceLine(int input_source_line, std::vector<int>& disassembly_lines)
{
    bool ret = false;

    auto disassembly_instruction_lines_iter = input_source_line_index_to_instruction_line_indices_.find(input_source_line);
    if (disassembly_instruction_lines_iter != input_source_line_index_to_instruction_line_indices_.end())
    {
        // Return the list of disassembly line indices.
        disassembly_lines = disassembly_instruction_lines_iter->second;
        ret = true;
    }

    return ret;
}

int RgIsaDisassemblyTableModel::GetCsvColumnIndex(RgIsaDisassemblyCsvFileColumns column) const
{
    return static_cast<uint>(column);
}

QColor RgIsaDisassemblyTableModel::GetFunctionalUnitColor(const std::string& functional_unit) const
{
    QColor color;

    static const std::map<std::string, QColor> kFunctionalUnitColors =
    {
        { FUNC_UNIT_SALU, QColor(28, 124, 84) },
        { FUNC_UNIT_VALU, QColor(75, 124, 140) },
        { FUNC_UNIT_SMEM, QColor(227, 29, 63).darker(110) },
        { FUNC_UNIT_VMEM, QColor(227, 29, 63).darker(90) },
        { FUNC_UNIT_INTERNAL_FLOW, QColor("black").lighter(150) },
        { FUNC_UNIT_BRANCH, kBranchLabelInstructionColor },
    };

    auto unit = kFunctionalUnitColors.find(functional_unit);
    if (unit != kFunctionalUnitColors.end())
    {
        color = unit->second;
    }

    return color;
}

bool RgIsaDisassemblyTableModel::LoadCsvData(const std::string& csv_file_full_path)
{
    bool ret = true;

    // Clear out existing data.
    disassembled_isa_lines_.clear();

    QFile csv_file(csv_file_full_path.c_str());
    QTextStream file_stream(&csv_file);

    // Attempt to open the file to read each instruction line.
    bool is_file_opened = csv_file.open(QFile::ReadOnly | QFile::Text);
    assert(is_file_opened);
    if (is_file_opened)
    {
        // Read the first ISA instruction line and just move on, as it's just column labels.
        QString isa_line = file_stream.readLine();

        // Parse each new line in the ISA disassembly file.
        do
        {
            // Read the next line in the file.
            isa_line = file_stream.readLine();
            if (!isa_line.isEmpty())
            {
                // Parse the line that was just read into a new RgIsaLine.
                std::shared_ptr<RgIsaLine> new_line = nullptr;
                int input_source_line_index = kInvalidCorrelationLineIndex;
                bool is_split_successful = ParseCsvIsaLine(isa_line.toStdString(), new_line, input_source_line_index);

                // Was the line processed correctly?
                assert(is_split_successful);
                if (is_split_successful)
                {
                    assert(new_line != nullptr);
                    if (new_line != nullptr)
                    {
                        // Does the parsed ISA line have an associated input source line?
                        if (input_source_line_index != kInvalidCorrelationLineIndex)
                        {
                            // Use a map to associate the current line of disassembly with the input source line index.
                            int disassembly_line_index = static_cast<int>(disassembled_isa_lines_.size());
                            instruction_line_number_to_input_source_line_number_[disassembly_line_index] = input_source_line_index;

                            // Find the minimum line number of the entry point in the input file.
                            if (input_source_line_index < source_file_entrypoint_start_line_)
                            {
                                source_file_entrypoint_start_line_ = input_source_line_index;
                            }

                            // Find the maximum line number of the entry point in the input file.
                            if (input_source_line_index > source_file_entrypoint_end_line_)
                            {
                                source_file_entrypoint_end_line_ = input_source_line_index;
                            }

                            // Also create a mapping of input source code line index to a list of all associated disassembly instruction line indices.
                            std::vector<int>& disassembly_line_indices = input_source_line_index_to_instruction_line_indices_[input_source_line_index];
                            disassembly_line_indices.push_back(disassembly_line_index);
                        }

                        // Add the ISA line's structure to the list of instruction lines.
                        disassembled_isa_lines_.push_back(new_line);
                    }
                }
                else
                {
                    ret = false;
                }
            }
        } while (!file_stream.atEnd());
    }
    else
    {
        // Failed to open the disassembly file.
        ret = false;
    }

    return ret;
}

void RgIsaDisassemblyTableModel::SetTableModelText(const std::string& model_text, uint row_index, uint column_index)
{
    // Set the cell's text, and make it left-aligned.
    isa_table_model_->setData(isa_table_model_->index(row_index, column_index), QString(model_text.c_str()));
    isa_table_model_->setData(isa_table_model_->index(row_index, column_index), Qt::AlignLeft, Qt::TextAlignmentRole);
}

void RgIsaDisassemblyTableModel::SetTableModelTooltip(const std::string& model_text, uint row_index, uint column_index)
{
    isa_table_model_->setData(isa_table_model_->index(row_index, column_index), QString(model_text.c_str()), Qt::ToolTipRole);
}

void RgIsaDisassemblyTableModel::SetTableModelTextColor(const QColor& model_color, uint row_index)
{
    // Set the table's model color for each column in the given row.
    for (int column_index = 0; column_index < GetTableColumnIndex(RgIsaDisassemblyTableColumns::kCount); ++column_index)
    {
        // Set the foreground color role in the model.
        isa_table_model_->setData(isa_table_model_->index(row_index, column_index), model_color, Qt::ForegroundRole);
    }
}

void RgIsaDisassemblyTableModel::SetTableModelBackgroundColor(const QColor& model_color, uint row_index, uint column_index)
{
    // Set the background color role in the model.
    isa_table_model_->setData(isa_table_model_->index(row_index, column_index), model_color, Qt::BackgroundColorRole);
}

bool RgIsaDisassemblyTableModel::ParseCsvIsaLine(const std::string& disassembled_line, std::shared_ptr<RgIsaLine>& parsed_line, int& input_source_line_index)
{
    bool ret = false;

    // Some parsed CSV lines don't have an input source code line number associated with them. In these cases, return -1.
    input_source_line_index = kInvalidCorrelationLineIndex;

    std::vector<std::string> line_tokens;
    std::stringstream line_stream;
    line_stream.str(disassembled_line);
    std::string substr;

    // Step through the entire line of text, and split into tokens based on comma position.
    while (std::getline(line_stream, substr, ','))
    {
        // Are there any quotation marks within the token? If so, parsing is handled differently.
        size_t num_quotes_in_token = std::count(substr.begin(), substr.end(), '\"');
        switch (num_quotes_in_token)
        {
        case 0:
            {
                // If there are no quotes, just add the token to the line tokens list.
                line_tokens.push_back(substr);
            }
            break;
        case 1:
            {
                // Found a start quote. Keep reading new tokens to find the matching end quote.
                std::stringstream token_stream;
                do
                {
                    // Add the token to the quoted column string.
                    token_stream << substr << ',';
                    std::getline(line_stream, substr, ',');
                } while (!(substr.find('"') != substr.npos));

                // Add the final portion of the token to the stream.
                token_stream << substr;

                // Remove the quotation marks from the final token string.
                std::string quoted_token = token_stream.str();
                quoted_token.erase(std::remove(quoted_token.begin(), quoted_token.end(), '\"'), quoted_token.end());

                // Add the token to the line tokens list.
                line_tokens.push_back(quoted_token);
            }
            break;
        case 2:
            {
                // There's a single token surrounded with 2 quotes. Just remove the quotes and add the token to the lines.
                substr.erase(std::remove(substr.begin(), substr.end(), '\"'), substr.end());
                line_tokens.push_back(substr);
            }
            break;
        default:
            // If this happens, the format of the ISA line likely wasn't handled correctly.
            assert(false);
        }
    }

    int num_columns = static_cast<int>(line_tokens.size());
    const int num_csv_columns = static_cast<int>(RgIsaDisassemblyCsvFileColumns::kCount);
    switch (num_columns)
    {
    case 1:
        {
            // This line is a label line that indicates a new section of instructions.
            std::shared_ptr<RgIsaLineLabel> label_line = std::make_shared<RgIsaLineLabel>();
            label_line->label_name = line_tokens[0];
            label_line->type       = RgIsaLineType::kLabel;

            // The line was parsed successfully.
            parsed_line = std::static_pointer_cast<RgIsaLine>(label_line);
            ret = true;
        }
        break;
    case num_csv_columns:
        {
            // Add each token to the output structure.
            std::shared_ptr<RgIsaLineInstruction> instruction_line = std::make_shared<RgIsaLineInstruction>();

            // Assign the values into the instruction line object.
            instruction_line->address             = line_tokens[GetCsvColumnIndex(RgIsaDisassemblyCsvFileColumns::kAddress)];
            instruction_line->opcode              = line_tokens[GetCsvColumnIndex(RgIsaDisassemblyCsvFileColumns::kOpcode)];
            instruction_line->operands            = line_tokens[GetCsvColumnIndex(RgIsaDisassemblyCsvFileColumns::kOperands)];
            instruction_line->functional_unit     = line_tokens[GetCsvColumnIndex(RgIsaDisassemblyCsvFileColumns::kFunctionalUnit)];
            instruction_line->cycles              = line_tokens[GetCsvColumnIndex(RgIsaDisassemblyCsvFileColumns::kCycles)];
            instruction_line->binary_encoding     = line_tokens[GetCsvColumnIndex(RgIsaDisassemblyCsvFileColumns::kBinaryEncoding)];

            // Extract the source line number as an integer.
            input_source_line_index = std::stoi(line_tokens[GetCsvColumnIndex(RgIsaDisassemblyCsvFileColumns::kSourceLineNumber)].c_str());

            // The line was parsed successfully, so assign it to the output instance.
            parsed_line = std::static_pointer_cast<RgIsaLineInstruction>(instruction_line);
            ret = true;
        }
        break;
    default:
        // Catch cases where the type of line format is unhandled.
        assert(false);
        break;
    }

    return ret;
}

void RgIsaDisassemblyTableModel::ClearLabelLines()
{
    // Empty out the contents of all lines with labels.
    int row_count = static_cast<int>(disassembled_isa_lines_.size());
    for (int row_index = 0; row_index < row_count; ++row_index)
    {
        std::shared_ptr<RgIsaLine> isa_line = disassembled_isa_lines_[row_index];
        if (isa_line->type == RgIsaLineType::kLabel)
        {
            // Add the label text to the model.
            SetTableModelText("", row_index, label_column_index_);

            // Set the background color to white, because the data was erased.
            SetTableModelBackgroundColor(QColor(Qt::white), row_index, label_column_index_);
        }
    }
}

void RgIsaDisassemblyTableModel::GetColumnMaxWidths(const QVector<int>& selected_row_numbers, std::vector<int>& widths) const
{
    const auto& global_settings = RgConfigManager::Instance().GetGlobalConfig();
    for (int col = 0, num_columns = isa_table_model_->columnCount(); col < num_columns; ++col)
    {
        if (global_settings->visible_disassembly_view_columns[col])
        {
            int max_width = 0;
            if (selected_row_numbers.at(selected_row_numbers.size() - 1) < disassembled_isa_lines_.size())
            {
                foreach (auto row, selected_row_numbers)
                {
                    if (disassembled_isa_lines_[row]->type == RgIsaLineType::kInstruction)
                    {
                        QVariant  call_text = isa_table_model_->data(isa_table_model_->index(row, col));
                        max_width = std::max(max_width, call_text.toString().size());
                    }
                }
            }
            widths.push_back(max_width);
        }
    }
}

void RgIsaDisassemblyTableModel::CopyRowsToClipboard(const QVector<int>& selected_row_numbers)
{
    const int MIN_COLUMN_OFFSET = 4;
    std::shared_ptr<RgGlobalSettings> global_settings = RgConfigManager::Instance().GetGlobalConfig();

    // Calculate maximum width for each column in the selected ISA region.
    std::vector<int>  max_column_width;
    GetColumnMaxWidths(selected_row_numbers, max_column_width);

    // Add a tab as a delimiter between each column.
    const char COLUMN_DELIMITER_SYMBOL = ' ';

    // Build a string including each line in the selected range.
    std::stringstream clipboard_text;
    foreach(auto row_index, selected_row_numbers)
    {
        // Is the current row an instruction or a label?
        std::shared_ptr<RgIsaLine> isa_line = disassembled_isa_lines_[row_index];
        if (isa_line->type == RgIsaLineType::kInstruction)
        {
            bool is_first_token_in_line = true;
            std::string  prev_column_text = "";

            // Add the contents of visible columns to the clipboard text.
            for (int column_index = 0, prev_visible_column_index = 0; column_index < global_settings->visible_disassembly_view_columns.size(); ++column_index)
            {
                // Is the current column visible?
                bool is_column_visible = global_settings->visible_disassembly_view_columns[column_index];
                if (is_column_visible)
                {
                    if (is_first_token_in_line)
                    {
                        // We don't need to append the delimiter before the first token in the line.
                        is_first_token_in_line = false;
                    }
                    else
                    {
                        // Append sufficient number of spaces so that the columns are aligned.
                        // The number of needed spaces is based on the width of the text in the previous column and its maximum width.
                        assert(prev_visible_column_index < max_column_width.size());
                        std::string  delimiter = "";
                        for (size_t i = 0, num_spaces = MIN_COLUMN_OFFSET + max_column_width[prev_visible_column_index++] - prev_column_text.size();
                            i < num_spaces; ++i)
                        {
                            delimiter += COLUMN_DELIMITER_SYMBOL;
                        }
                        clipboard_text << delimiter;
                    }

                    // Get the cell data.
                    QModelIndex cell_index = isa_table_model_->index(row_index, column_index);
                    QVariant cell_text = isa_table_model_->data(cell_index);

                    // If the data is invalid, get it from somewhere else.
                    if (!cell_text.isValid())
                    {
                        // Check if the functional unit is set to "Branch".
                        // If it is, get the data from Operands column from disassembled ISA Lines instead.
                        QString cell_text_string = isa_table_model_->data(isa_table_model_->index(row_index, static_cast<int>(RgIsaDisassemblyTableColumns::kFunctionalUnit))).toString();
                        if (cell_text_string.compare("Branch") == 0)
                        {
                            std::shared_ptr<RgIsaLine> isa_line = disassembled_isa_lines_[row_index];
                            assert(isa_line != nullptr);
                            if (isa_line != nullptr)
                            {
                                std::shared_ptr<RgIsaLineInstruction> instruction_line = std::static_pointer_cast<RgIsaLineInstruction>(isa_line);
                                assert(instruction_line != nullptr);
                                if (instruction_line != nullptr)
                                {
                                    cell_text = QString::fromStdString(instruction_line->operands);
                                }
                            }
                        }
                    }

                    // Add the data to the clipboard string stream.
                    clipboard_text << (prev_column_text = cell_text.toString().toStdString());
                }
            }
        }
        else if (isa_line->type == RgIsaLineType::kLabel)
        {
            // Extract the label text from whatever column the label is in.
            QModelIndex cell_index = isa_table_model_->index(row_index, label_column_index_);
            QVariant cell_text = isa_table_model_->data(cell_index);

            // Append the label name to the clipboard text.
            clipboard_text << cell_text.toString().toStdString();
        }

        // Add a newline to the end of each row of data.
        clipboard_text << std::endl;
    }

    // Add the text into the clipboard.
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(clipboard_text.str().c_str());
}

void RgIsaDisassemblyTableModel::InitializeModelData()
{
    // Update the number of rows in the table.
    int line_count = static_cast<int>(disassembled_isa_lines_.size());
    isa_table_model_->setRowCount(line_count);

    // Step through each ISA line and update the row item's data.
    for (int line_index = 0; line_index < line_count; ++line_index)
    {
        std::shared_ptr<RgIsaLine> isa_line = disassembled_isa_lines_[line_index];

        if (isa_line->type == RgIsaLineType::kInstruction)
        {
            std::shared_ptr<RgIsaLineInstruction> instruction_line = std::static_pointer_cast<RgIsaLineInstruction>(isa_line);

            // Update the model cells with data from each disassembled ISA instruction line.
            SetTableModelText(instruction_line->address,            line_index, GetTableColumnIndex(RgIsaDisassemblyTableColumns::kAddress));
            SetTableModelText(instruction_line->opcode,             line_index, GetTableColumnIndex(RgIsaDisassemblyTableColumns::kOpcode));
            SetTableModelText(instruction_line->functional_unit,    line_index, GetTableColumnIndex(RgIsaDisassemblyTableColumns::kFunctionalUnit));
            SetTableModelText(instruction_line->cycles,             line_index, GetTableColumnIndex(RgIsaDisassemblyTableColumns::kCycles));
            SetTableModelText(instruction_line->binary_encoding,    line_index, GetTableColumnIndex(RgIsaDisassemblyTableColumns::kBinaryEncoding));
            SetTableModelText(instruction_line->num_live_registers, line_index, GetTableColumnIndex(RgIsaDisassemblyTableColumns::kLiveVgprs));

            // Set the tooltip for the live VGPR cell.
            std::string tooltip;
            CreateTooltip(tooltip, instruction_line->num_live_registers);
            SetTableModelTooltip(tooltip, line_index, GetTableColumnIndex(RgIsaDisassemblyTableColumns::kLiveVgprs));

            // Branch instructions are rendered with a separate delegate, so we don't need to add them to the table model.
            if (instruction_line->functional_unit.compare("Branch") != 0)
            {
                SetTableModelText(instruction_line->operands, line_index, GetTableColumnIndex(RgIsaDisassemblyTableColumns::kOperands));
            }

            // The functional unit column's text color is based on value.
            QColor text_color = GetFunctionalUnitColor(instruction_line->functional_unit);
            SetTableModelTextColor(text_color, line_index);
        }
        else if (isa_line->type == RgIsaLineType::kLabel)
        {
        	// Get the instruction line.
            std::shared_ptr<RgIsaLine>            isa_line         = disassembled_isa_lines_[line_index];
            std::shared_ptr<RgIsaLineInstruction> instruction_line = std::static_pointer_cast<RgIsaLineInstruction>(isa_line);

            // Update the model cells with data from each disassembled ISA instruction line.
            SetTableModelText(instruction_line->num_live_registers, line_index, GetTableColumnIndex(RgIsaDisassemblyTableColumns::kLiveVgprs));
        }
    }

    InsertLabelRows();
}

void RgIsaDisassemblyTableModel::CreateTooltip(std::string& tooltip, const std::string& num_live_registers) const
{
    // Extract live VGPRs and granularity values.
    QStringList values     = QString::fromStdString(num_live_registers).split(",");

    // Verify the split.
    if (values.size() == 2)
    {
        int used                   = values.at(0).toInt();
        int block_allocation_value = values.at(1).toInt();
        int allocated              = 0;
        int reduction              = 0;

        assert(block_allocation_value != 0);
        if (block_allocation_value != 0)
        {
            // Calculate various values.
            if (used % block_allocation_value == 0)
            {
                allocated = used;
                tooltip   = QString(kLiveVgprTooltip1).arg(used).arg(allocated).arg(livereg_data_.total_vgprs).toStdString();
            }
            else
            {
                allocated = ((used / block_allocation_value) + 1) * block_allocation_value;
                reduction = used % block_allocation_value;
                tooltip   = (QString(kLiveVgprTooltip1).arg(used).arg(allocated).arg(livereg_data_.total_vgprs) +
                           QString(kLiveVgprTooltip2).arg(reduction).arg(block_allocation_value).arg(block_allocation_value))
                              .toStdString();
            }
        }
    }
}

void RgIsaDisassemblyTableModel::HighlightCurrentMaxVgprLine()
{
    // Set all lines to be not current max VGPR lines.
    std::fill(livereg_data_.is_current_max_vgpr_line_number.begin(), livereg_data_.is_current_max_vgpr_line_number.end(), false);

    // Set the boolean for the current line to true so it'll be highlighted.
    livereg_data_.is_current_max_vgpr_line_number.at(current_max_vgpr_index_) = true;
}

void RgIsaDisassemblyTableModel::UpdateCurrentMaxVgprLine()
{
    if (livereg_data_.is_current_max_vgpr_line_number.size() > 0)
    {
        // Update the current maximum VGPR line index.
        if (current_max_vgpr_index_ < livereg_data_.is_current_max_vgpr_line_number.size() - 1)
        {
            current_max_vgpr_index_++;
        }
        else
        {
            current_max_vgpr_index_ = 0;
        }
    }
}

void RgIsaDisassemblyTableModel::ResetCurrentMaxVgprValues()
{
    // Reset the maximum VGPR values.
    current_max_vgpr_index_ = 0;
    is_show_current_max_vgpr_enabled_ = false;

    // Set all lines to be not current max VGPR lines.
    std::fill(livereg_data_.is_current_max_vgpr_line_number.begin(), livereg_data_.is_current_max_vgpr_line_number.end(), false);
}

bool RgIsaDisassemblyTableModel::IsShowCurrentMaxVgprEnabled()
{
    return is_show_current_max_vgpr_enabled_;
}

bool RgIsaDisassemblyTableModel::GetNextMaxVgprLineNumber(int& line_number)
{
    bool is_valid_line = false;

    // Return a valid line number.
    if (livereg_data_.max_vgpr_line_numbers.size() > 0)
    {
        if (is_show_current_max_vgpr_enabled_)
        {
            // Update the current maximum VGPR line index.
            if (current_max_vgpr_index_ < livereg_data_.is_current_max_vgpr_line_number.size() - 1)
            {
                current_max_vgpr_index_++;
            }
            else
            {
                current_max_vgpr_index_ = 0;
            }
        }
        else
        {
            is_show_current_max_vgpr_enabled_ = true;
        }

        line_number = livereg_data_.max_vgpr_line_numbers.at(current_max_vgpr_index_);
        is_valid_line = true;
    }

    return is_valid_line;
}

bool RgIsaDisassemblyTableModel::GetPreviousMaxVgprLineNumber(int& line_number)
{
    bool is_valid_line = false;

    if (is_show_current_max_vgpr_enabled_)
    {
        if (current_max_vgpr_index_ > 0)
        {
            current_max_vgpr_index_--;
        }
        else
        {
            current_max_vgpr_index_ = livereg_data_.max_vgpr_line_numbers.size() - 1;
        }
    }
    else
    {
        is_show_current_max_vgpr_enabled_ = true;
    }

    // Return a valid line number.
    if (livereg_data_.max_vgpr_line_numbers.size() > 0)
    {
        line_number   = livereg_data_.max_vgpr_line_numbers.at(current_max_vgpr_index_);
        is_valid_line = true;
    }

    return is_valid_line;
}
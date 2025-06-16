//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for shader ISA Disassembly view item model.
//=============================================================================

// C++.
#include <cassert>
#include <sstream>

// Qt.
#include <QFile>
#include <QFont>
#include <QFontMetrics>
#include <QLabel>
#include <QMenu>
#include <QObject>
#include <QPainter>
#include <QScrollBar>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QWidget>

// Infra.
#include "amdisa/isa_decoder.h"
#include "qt_common/utils/qt_util.h"
#include "qt_isa_gui/widgets/isa_tree_view.h"

// Shared.
#include "common/rga_shared_utils.h"

// Local.
#include "radeon_gpu_analyzer_gui/rg_data_types.h"
#include "radeon_gpu_analyzer_gui/rg_definitions.h"
#include "radeon_gpu_analyzer_gui/rg_isa_decode_manager.h"
#include "radeon_gpu_analyzer_gui/rg_string_constants.h"
#include "radeon_gpu_analyzer_gui/rg_utils.h"
#include "radeon_gpu_analyzer_gui/qt/rg_isa_item_model.h"
#include "radeon_gpu_analyzer_gui/qt/rg_output_file_utils.h"

static const std::string kStrNA = "N/A";

const std::array<std::string, RgIsaItemModel::kColumnCount - IsaItemModel::kColumnCount> RgIsaItemModel::kColumnNames = {
    "VGPR pressure (used:%1, allocated:%2/%3)",
    "Functional group"};  ///< Predefined column headers.

RgIsaItemModel::RgIsaItemModel(QObject* parent)
    : IsaItemModel(parent, RgIsaDecodeManager::GetInstance().Get())
{
}

int RgIsaItemModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);

    return RgIsaItemModel::Columns::kColumnCount;
}

QVariant RgIsaItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    QVariant header_data;

    if (section < IsaItemModel::kColumnCount)
    {
        return IsaItemModel::headerData(section, orientation, role);
    }

    if (orientation == Qt::Vertical || section < 0 || section > RgIsaItemModel::Columns::kColumnCount)
    {
        return header_data;
    }

    if (section == RgIsaItemModel::Columns::kIsaColumnVgprPressure)
    {
        if (role == Qt::DisplayRole)
        {
            header_data = QString(kColumnNames[section - IsaItemModel::kColumnCount].c_str())
                              .arg(QString::number(current_livereg_data_.used))
                              .arg(QString::number(current_livereg_data_.allocated))
                              .arg(QString::number(current_livereg_data_.total_vgprs));
        }

        // Show the hazard icon if we've hit more than the max live VGPRs.
        if (current_livereg_data_.max_vgprs >= current_livereg_data_.total_vgprs)
        {
            if (role == Qt::DecorationRole)
            {
                header_data = QIcon(kIconMaxVgprNotification);
            }
            else if (role == Qt::ToolTipRole)
            {
                header_data = QString(kLiveVgprMaxVgprTooltip);
            }
        }

        // Show the warning icon and a tooltip if instructions did not match.
        if (current_livereg_data_.unmatched_count > 0)
        {
            if (role == Qt::DecorationRole)
            {
                header_data = QIcon(kIconMaxVgprNoIsaMatch);
            }
            else if (role == Qt::ToolTipRole)
            {
                header_data = QString(kLiveVgprNATooltip).arg(current_livereg_data_.unmatched_count);
            }
        }
    }
    else if (role == Qt::DisplayRole)
    {
        header_data = kColumnNames[section - IsaItemModel::kColumnCount].c_str();
    }

    return header_data;
}

QVariant RgIsaItemModel::data(const QModelIndex& index, int role) const
{
    QVariant data;

    if (!index.isValid())
    {
        return data;
    }

    if (role >= IsaItemModel::UserRoles::kLabelBranchRole && role < IsaItemModel::UserRoles::kUserRolesCount)
    {
        // Ask shared model for shared roles.
        return IsaItemModel::data(index, role);
    }
    else if (role >= IsaItemModel::UserRoles::kUserRolesCount && role < RgIsaItemModel::UserRoles::kUserRolesCount)
    {
        if (index.parent().isValid())
        {
            RgIndexData index_data = current_index_data_.at(index.parent().row()).at(index.row());
            switch (role)
            {
            case RgIsaItemModel::UserRoles::kMaxVgprLineRole:
                data.setValue(index_data.is_max_vgpr_row);
                break;
            case RgIsaItemModel::UserRoles::kSrcLineToIsaRowRole:
                data.setValue(index_data.is_active_correlation);
                break;
            case RgIsaItemModel::UserRoles::kIsaRowToSrcLineRole:
                data.setValue(index_data.input_source_line_index);
                break;
            default:
                break;
            }
            return data;
        }
    }

    if (index.column() < IsaItemModel::kColumnCount)
    {
        return IsaItemModel::data(index, role);
    }

    switch (role)
    {
    case Qt::ForegroundRole:
    {
        // This role is not useful for columns with multi-coloring in the same column.

        // Default to color theme's text color.
        data.setValue(QtCommon::QtUtils::ColorTheme::Get().GetCurrentThemeColors().graphics_scene_text_color);
        break;
    }
    case Qt::DisplayRole:
    {
        if (index.parent().isValid())
        {
            RgIndexData index_data = current_index_data_.at(index.parent().row()).at(index.row());

            switch (index.column())
            {
            case Columns::kIsaColumnVgprPressure:
                data.setValue(QString(index_data.num_live_registers.c_str()));
                break;
            case Columns::kIsaColumnFunctionalUnit:
            {
                const auto instruction_info =
                    qvariant_cast<amdisa::InstructionInfo>(index.siblingAtColumn(IsaItemModel::Columns::kOpCode).data(IsaItemModel::UserRoles::kDecodedIsa));
                data.setValue(instruction_info);
            }
            break;
            default:
                break;
            }
        }
        break;
    }
    case Qt::ToolTipRole:
        if (index.parent().isValid())
        {
            RgIndexData index_data = current_index_data_.at(index.parent().row()).at(index.row());

            switch (index.column())
            {
            case Columns::kIsaColumnVgprPressure:
                data.setValue(QString(index_data.vgpr_tooltip.c_str()));
                break;
            default:
                break;
            }
        }
        break;
    default:
        break;
    }
    return data;
}

void RgIsaItemModel::ParseCsvLine(QString isa_line, std::vector<std::string>& line_tokens, std::vector<std::string>& operand_tokens_str)
{
    std::stringstream line_stream;
    line_stream.str(isa_line.toStdString());
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

            // If there are multiple operand tokens parse them into a separate list of token strings.
            std::string operand_token = std::string(substr.begin() + 1, substr.end());
            operand_tokens_str.push_back(operand_token);

            // Also create a string of all the operands so that the indices in the line_tokens list stays intact.
            std::stringstream token_stream;
            do
            {
                // Add the token to the quoted column string.
                token_stream << substr << ',';
                std::getline(line_stream, substr, ',');

                // Add the operand token to the operand only list.
                if (!(substr.find('"') != substr.npos))
                {
                    operand_tokens_str.push_back(substr);
                }

            } while (!(substr.find('"') != substr.npos));

            // Remove the quotes from the last operand and add it to the operands list.
            operand_token = std::string(substr.begin(), substr.end() - 1);
            operand_tokens_str.push_back(operand_token);

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

        // If there was only 1 operands token then operands_token_str is empty, grab the openands index from the line_tokens list.
        if (operand_tokens_str.size() == 0 && line_tokens.size() > CsvFileColumns::kOperands)
        {
            operand_tokens_str.push_back(line_tokens[CsvFileColumns::kOperands]);
        }
    }
}

void RgIsaItemModel::ReadIsaCsvFile(std::string                                        csv_file_full_path,
                                    std::vector<std::shared_ptr<IsaItemModel::Block>>& blocks,
                                    std::vector<std::vector<RgIndexData>>&             index_data)
{
    QFontMetrics font_metrics(fixed_font_);

    QFile       csv_file(csv_file_full_path.c_str());
    QTextStream file_stream(&csv_file);

    // Attempt to open the file to read each instruction line.
    bool is_file_opened = csv_file.open(QFile::ReadOnly | QFile::Text);
    assert(is_file_opened);
    if (is_file_opened)
    {
        uint64_t line_number         = 0;
        int      code_block_position = 0;

        // Read the first ISA instruction line and just move on, as it's just column labels.
        QString isa_line = file_stream.readLine();

        InstructionBlock* current_code_block       = new InstructionBlock(code_block_position++, line_number++, " ");
        current_code_block->token.type             = IsaItemModel::TokenType::kLabelType;
        current_code_block->token.x_position_start = 0;
        current_code_block->token.x_position_end   = font_metrics.horizontalAdvance(" ");

        blocks.emplace_back(current_code_block);

        std::vector<RgIndexData> empty_index_data_block;
        index_data.push_back(empty_index_data_block);

        do
        {
            isa_line = file_stream.readLine();

            if (!isa_line.isEmpty())
            {
                std::vector<std::string> line_tokens;
                std::vector<std::string> operands;

                ParseCsvLine(isa_line, line_tokens, operands);

                int       num_columns            = static_cast<int>(line_tokens.size());
                const int num_csv_columns        = static_cast<int>(CsvFileColumns::kCount);
                switch (num_columns)
                {
                case 1:
                {
                    std::string code_block_label = line_tokens[0];
                    if (code_block_label.find(":", code_block_label.size() - 1))
                    {
                        code_block_label = code_block_label.erase(code_block_label.size() - 1);
                    }

                    InstructionBlock* code_block       = new InstructionBlock(code_block_position++, line_number++, code_block_label);
                    code_block->token.type             = IsaItemModel::TokenType::kLabelType;
                    code_block->token.x_position_start = 0;
                    code_block->token.x_position_end   = font_metrics.horizontalAdvance(line_tokens[0].c_str());

                    current_code_block = code_block;

                    blocks.emplace_back(code_block);

                    index_data.push_back(empty_index_data_block);
                }
                break;
                case num_csv_columns:
                {
                    std::string address         = line_tokens[CsvFileColumns::kAddress];
                    std::string op_code         = line_tokens[CsvFileColumns::kOpcode];
                    std::string binary_encoding = line_tokens[CsvFileColumns::kBinaryEncoding];
                    int input_source_line_index = std::stoi(line_tokens[CsvFileColumns::kSourceLineNumber].c_str());

                    InstructionRow* instruction_line = new InstructionRow(line_number++, op_code, address, binary_encoding);

                    ParseSelectableTokens(op_code, instruction_line->op_code_token, operands, instruction_line->operand_tokens, fixed_font_character_width_);

                    current_code_block->instruction_lines.emplace_back(instruction_line);

                    RgIndexData rg_index_data{};
                    rg_index_data.input_source_line_index = input_source_line_index;

                    index_data.at(index_data.size() - 1).push_back(rg_index_data);
                }
                break;
                default:
                    // Catch cases where the type of line format is unhandled.
                    assert(false);
                    break;
                }
            }
        } while (!file_stream.atEnd());
    }
}

int RgIsaItemModel::CalculateMaxVgprs(std::vector<std::vector<RgIndexData>>&             index_data,
                                      std::vector<std::pair<int, int>>&                  max_line_numbers,
                                      std::vector<std::shared_ptr<IsaItemModel::Block>>& blocks)
{
    int max_vgprs_used = 0;

    for (int i = 0; i < blocks.size() && i < index_data.size(); i++)
    {
        for (int j = 0; j < blocks.at(i)->instruction_lines.size() && j < index_data.at(i).size(); j++)
        {
            if (index_data.at(i).at(j).opcode.compare(kStrNA) == 0)
            {
                continue;
            }

            QString vgpr_value = QString::fromStdString(index_data.at(i).at(j).num_live_registers);
            if (vgpr_value.toInt() >= max_vgprs_used)
            {
                if (vgpr_value.toInt() > max_vgprs_used)
                {
                    // Clear the values saved so far.
                    max_line_numbers.clear();
                }

                // Save the max VGPR value.
                max_vgprs_used = vgpr_value.toInt();

                // Save the line number here as well.
                max_line_numbers.push_back(std::pair<int, int>(i, j));
            }
        }
    }

    return max_vgprs_used;
}

bool RgIsaItemModel::SetCurrentMaxVgprLine(EntryData::Operation op)
{
    bool ret = false;

    // Reset all max vgpr rows to false.
    for (const auto& itr : current_livereg_data_.max_vgpr_line_numbers)
    {
        if (itr.first < current_index_data_.size() && itr.second < current_index_data_.at(itr.first).size())
        {
            current_index_data_.at(itr.first).at(itr.second).is_max_vgpr_row = false;
        }
    }

    switch (op)
    {
    case EntryData::Operation::kGoToNextMaxVgpr:
        if (current_livereg_data_.max_vgpr_line_numbers.size() > 0)
        {
            if (current_livereg_data_.current_max_vgpr_line_numbers_index < current_livereg_data_.max_vgpr_line_numbers.size() - 1)
            {
                current_livereg_data_.current_max_vgpr_line_numbers_index++;
            }
            else
            {
                current_livereg_data_.current_max_vgpr_line_numbers_index = 0;
            }

            const auto& curr_row = current_livereg_data_.max_vgpr_line_numbers[current_livereg_data_.current_max_vgpr_line_numbers_index];
            current_index_data_.at(curr_row.first).at(curr_row.second).is_max_vgpr_row = true;
            ret                                                                        = true;
        }
        break;

    case EntryData::Operation::kGoToPrevMaxVgpr:
        if (current_livereg_data_.max_vgpr_line_numbers.size() > 0)
        {
            if (current_livereg_data_.current_max_vgpr_line_numbers_index > 0)
            {
                current_livereg_data_.current_max_vgpr_line_numbers_index--;
            }
            else
            {
                current_livereg_data_.current_max_vgpr_line_numbers_index = static_cast<int>(current_livereg_data_.max_vgpr_line_numbers.size()) - 1;
            }

            const auto& curr_row = current_livereg_data_.max_vgpr_line_numbers[current_livereg_data_.current_max_vgpr_line_numbers_index];
            current_index_data_.at(curr_row.first).at(curr_row.second).is_max_vgpr_row = true;
            ret                                                                        = true;
        }
        break;
    default:
        break;
    }
    return ret;
}

void RgIsaItemModel::SetLineCorrelatedIndices(int input_source_line_index)
{
    for (int i = 0; i < current_index_data_.size(); i++)
    {
        for (int j = 0; j < current_index_data_.at(i).size(); j++)
        {
            auto& index_data = current_index_data_.at(i).at(j);
            if (index_data.input_source_line_index == input_source_line_index)
            {
                index_data.is_active_correlation = true;
            }
            else
            {
                index_data.is_active_correlation = false;
            }
        }
    }
}

static bool AreOpcodesEqual(const std::string& blocks_op_code, const std::string& vgpr_op_code)
{
    return (vgpr_op_code.compare(blocks_op_code) == 0) || (blocks_op_code.compare(vgpr_op_code + "_e32") == 0) ||
           (blocks_op_code.compare(vgpr_op_code + "_e64") == 0) || (blocks_op_code.compare(vgpr_op_code + "_sdwa") == 0) ||
           (blocks_op_code.compare(vgpr_op_code + "_dpp") == 0);
}

bool RgIsaItemModel::ParseLiveVgprsData(const std::string&                                 live_vgpr_file_full_path,
                                        std::vector<std::shared_ptr<IsaItemModel::Block>>& blocks,
                                        std::vector<std::vector<RgIndexData>>&             index_data,
                                        RgLiveregData&                                     livereg_data)
{
    bool status = false;

    std::vector<QString> vgpr_file_lines;

    // Read the live VGPR output file.
    RgOutputFileUtils::ReadLiveVgprsFile(live_vgpr_file_full_path, vgpr_file_lines);

    // The index in the parsed vgpr file lines list, without any of the comment lines in the isa.
    int  file_lines_index = 0;
    bool is_first_block   = true;

    for (int i = 0; i < blocks.size() && i < index_data.size(); i++)
    {
        for (int j = 0; j < blocks.at(i)->instruction_lines.size() && j < index_data.at(i).size(); j++)
        {
            auto instruction_line = blocks.at(i)->instruction_lines.at(j);
            if (instruction_line && instruction_line->row_type == RowType::kComment)
            {
                continue;
            }

            auto instruction_row = std::static_pointer_cast<IsaItemModel::InstructionRow>(instruction_line);
            if (instruction_row)
            {
                auto& index_data_entry = index_data.at(i).at(j);
                if (file_lines_index < vgpr_file_lines.size() && instruction_line->row_type != RowType::kComment)
                {
                    QString live_vgpr_line = vgpr_file_lines.at(file_lines_index);
                    while (!RgOutputFileUtils::IsValidLine(live_vgpr_line))
                    {
                        file_lines_index++;
                        if (file_lines_index >= vgpr_file_lines.size())
                        {
                            break;
                        }
                        live_vgpr_line = vgpr_file_lines.at(file_lines_index);
                    }
                    if (file_lines_index >= vgpr_file_lines.size())
                    {
                        break;
                    }

                    if (RgOutputFileUtils::IsValidLine(live_vgpr_line))
                    {
                        bool is_label = RgOutputFileUtils::IsLabelLine(live_vgpr_line);

                        const QRegularExpression label_regex("^\\s*(\\d+)\\s*\\|\\s*(\\d+)\\s*\\|\\s*([:\\^vx\\s*]+)\\s*\\|\\s*[^:]*:\\s*(\\w+)");
                        const QRegularExpression non_label_regex("^\\s*(\\d+)\\s*\\|\\s*(\\d+)\\s*\\|\\s*([:^vx\\s*]+)\\s*\\|\\s*(\\w+)\\s*");
                        QRegularExpressionMatch  match;

                        if (is_label)
                        {
                            match = non_label_regex.match(live_vgpr_line);

                            // Get the label text of the first block
                            if (is_first_block && match.hasMatch())
                            {
                                auto first_block = std::static_pointer_cast<IsaItemModel::InstructionBlock>(blocks.at(0));

                                QString     label_text   = match.captured(4);
                                std::string label_string = "label__";
                                label_text.remove(0, label_string.size());

                                first_block->token.token_text = label_text.toStdString();
                                is_first_block                = false;
                            }

                            match = label_regex.match(live_vgpr_line);
                        }
                        else
                        {
                            match = non_label_regex.match(live_vgpr_line);
                        }

                        if (match.hasMatch())
                        {
                            std::string num_live_registers = match.captured(2).toStdString();
                            std::string opcode             = match.captured(4).toStdString();

                            if (!AreOpcodesEqual(instruction_row->op_code_token.token_text, opcode))
                            {
                                index_data_entry.num_live_registers = "N/A";
                                index_data_entry.opcode             = "N/A";
                                continue;
                            }
                            else
                            {
                                index_data_entry.num_live_registers = match.captured(2).toStdString();
                                index_data_entry.opcode             = match.captured(4).toStdString();
                            }
                        }
                        else
                        {
                            index_data_entry.opcode             = "N/A";
                            index_data_entry.num_live_registers = "N/A";
                        }
                    }

                    file_lines_index++;
                }
            }
        }
    }

    // Now extract the architecture specific information.
    status = RgOutputFileUtils::GetArchInformation(livereg_data, vgpr_file_lines);

    // Calculate the max live VGPR values and extract the live VGPR numbers.
    if (status)
    {
        // Extract maximum VGPR lines.
        livereg_data.max_vgprs = RgIsaItemModel::CalculateMaxVgprs(index_data, livereg_data.max_vgpr_line_numbers, blocks);

        livereg_data.unmatched_count = 0;

        for (int i = 0; i < blocks.size() && i < index_data.size(); i++)
        {
            for (int j = 0; j < blocks.at(i)->instruction_lines.size() && j < index_data.at(i).size(); j++)
            {
                if (blocks.at(i)->instruction_lines.at(j)->row_type == RowType::kComment)
                {
                    continue;
                }

                auto instruction_row = std::static_pointer_cast<IsaItemModel::InstructionRow>(blocks.at(i)->instruction_lines.at(j));
                if (AreOpcodesEqual(instruction_row->op_code_token.token_text, index_data.at(i).at(j).opcode))
                {
                    // Save both the number of live VGPRs and the block granularity.
                    index_data.at(i).at(j).num_live_registers =
                        index_data.at(i).at(j).num_live_registers + "," + std::to_string(livereg_data.vgprs_granularity);

                    // Create Vgpr column tooltip.
                    CreateVgprTooltip(index_data.at(i).at(j).vgpr_tooltip, index_data.at(i).at(j).num_live_registers);
                }
                else
                {
                    // If the instructions did not match, display "N/A".
                    index_data.at(i).at(j).num_live_registers = "N/A";

                    if (instruction_row->op_code_token.token_text != "s_nop")
                    {
                        livereg_data.unmatched_count++;
                    }
                }
            }
        }
    }

    return status;
}

void RgIsaItemModel::UpdateData(void* data)
{
    const EntryData entry_data = *(static_cast<EntryData*>(data));

    const std::string csv_file_full_path = entry_data.isa_file_path;

    switch (entry_data.operation)
    {
    case EntryData::Operation::kUpdateLineCorrelation:
        SetLineCorrelatedIndices(entry_data.input_source_line_index);
        break;

    case EntryData::Operation::kGoToNextMaxVgpr:
    case EntryData::Operation::kGoToPrevMaxVgpr:
        SetCurrentMaxVgprLine(entry_data.operation);
        break;

    case EntryData::Operation::kEvictData:

        if (cached_isa_.find(csv_file_full_path) != cached_isa_.end())
        {
            cached_isa_.erase(csv_file_full_path);
            cached_index_data_.erase(csv_file_full_path);
            cached_livereg_data_.erase(csv_file_full_path);
        }

        if (cached_isa_.empty())
        {
            // Notify the model that it is about to be reset.
            ResetModelObject reset_model_object(this);

            blocks_.clear();
            current_index_data_.clear();
            current_livereg_data_ = RgLiveregData{};
        }

        break;

    case EntryData::Operation::kLoadData:
    default:

        // Notify the model that it is about to be reset.
        ResetModelObject reset_model_object(this);

        blocks_.clear();
        current_index_data_.clear();
        current_livereg_data_ = RgLiveregData{};

        if (!SetArchitecture(entry_data.target_gpu))
        {
            qDebug() << entry_data.target_gpu << " xml spec missing.";
        }

        // If the the file has been read before, use the cached value.
        if (cached_isa_.find(csv_file_full_path) != cached_isa_.end())
        {
            blocks_               = cached_isa_.at(csv_file_full_path);
            current_index_data_   = cached_index_data_.at(csv_file_full_path);
            current_livereg_data_ = cached_livereg_data_.at(csv_file_full_path);
        }
        else
        {
            std::vector<std::shared_ptr<IsaItemModel::Block>> new_isa;
            ReadIsaCsvFile(csv_file_full_path, blocks_, current_index_data_);

            const std::string live_vgpr_file_full_path = entry_data.vgpr_file_path;

            ParseLiveVgprsData(live_vgpr_file_full_path, blocks_, current_index_data_, current_livereg_data_);

            cached_isa_.insert(std::pair<std::string, std::vector<std::shared_ptr<IsaItemModel::Block>>>(csv_file_full_path, blocks_));
            cached_index_data_.insert(std::pair<std::string, std::vector<std::vector<RgIndexData>>>(csv_file_full_path, current_index_data_));
            cached_livereg_data_.insert(std::pair<std::string, RgLiveregData>(csv_file_full_path, current_livereg_data_));
        }

        MapBlocksToBranchInstructions();

        CacheSizeHints();

        break;
    }
}

bool RgIsaItemModel::GetMaxVgprPressureIndices(std::vector<QModelIndex>& source_indices) const
{
    bool ret = false;
    for (const auto& max_vgpr_line_number : current_livereg_data_.max_vgpr_line_numbers)
    {
        const auto&       row_pair     = max_vgpr_line_number;
        const auto&       parent_row   = row_pair.first;
        const auto&       child_row    = row_pair.second;
        const QModelIndex parent_index = index(parent_row, 0);
        source_indices.push_back(index(child_row, 0, parent_index));
        ret = true;
    }
    return ret;
}

QModelIndex RgIsaItemModel::GetFirstLineCorrelatedIndex(int input_source_line_index) const
{
    if (input_source_line_index != kInvalidCorrelationLineIndex)
    {
        for (int i = 0; i < current_index_data_.size(); i++)
        {
            for (int j = 0; j < current_index_data_.at(i).size(); j++)
            {
                const auto& index_data = current_index_data_.at(i).at(j);
                if (index_data.input_source_line_index == input_source_line_index)
                {
                    return index(j, 0, index(i, 0));
                }
            }
        }
    }
    return QModelIndex{};
}

QModelIndex RgIsaItemModel::GetMaxVgprIndex() const
{
    auto curr = current_livereg_data_.current_max_vgpr_line_numbers_index;
    if (curr >= 0 && curr < current_livereg_data_.max_vgpr_line_numbers.size())
    {
        const auto& [i, j] = current_livereg_data_.max_vgpr_line_numbers[curr];
        return index(j, 0, index(i, 0));
    }

    return QModelIndex{};
}

void RgIsaItemModel::CacheSizeHints()
{
    // Cache size hints for shared columns.
    IsaItemModel::CacheSizeHints();

    // Reset RGA specific column widths.
    column_widths_.fill(0);

    const qreal padding_length = static_cast<qreal>(IsaItemModel::kColumnPadding.size());

    // Calculate width for VGPR column.
    const auto max_vgpr_text_length = QString::number(current_livereg_data_.total_vgprs).size() + (2 * padding_length);
    uint32_t   vgpr_column_width    = static_cast<uint32_t>(std::ceil(max_vgpr_text_length * fixed_font_character_width_));
    // Calculate the max width of swatch rectangle.
    vgpr_column_width += (current_livereg_data_.total_vgprs / current_livereg_data_.vgprs_granularity + 1) * current_livereg_data_.vgprs_granularity;
    column_widths_[kIsaColumnVgprPressure - IsaItemModel::kColumnCount] = vgpr_column_width;

    // Calculate functional group column width.
    size_t max_functional_group_text_length = 0;
    for (const char* functional_group_name : amdisa::kFunctionalGroupName)
    {
        max_functional_group_text_length = std::max(max_functional_group_text_length, std::strlen(functional_group_name));
    }
    max_functional_group_text_length += padding_length;
    const uint32_t functional_group_column_width = static_cast<uint32_t>(std::ceil(max_functional_group_text_length * fixed_font_character_width_));
    column_widths_[kIsaColumnFunctionalUnit - IsaItemModel::kColumnCount] = functional_group_column_width;
}

QSize RgIsaItemModel::ColumnSizeHint(int column_index, IsaTreeView* tree) const
{
    QSize size_hint(0, 0);

    if (column_index < 0 || column_index > kColumnCount)
    {
        return size_hint;
    }

    if (column_index < IsaItemModel::kColumnCount)
    {
        return IsaItemModel::ColumnSizeHint(column_index, tree);
    }

    size_hint.setHeight(QFontMetrics(fixed_font_, tree).height() + 2);
    size_hint.setWidth(column_widths_[column_index - IsaItemModel::kColumnCount]);

    return size_hint;
}

void RgIsaItemModel::CreateVgprTooltip(std::string& tooltip, const std::string& num_live_registers) const
{
    // Extract live VGPRs and granularity values.
    QStringList values = QString::fromStdString(num_live_registers).split(",");

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
                tooltip   = QString(kLiveVgprTooltip1).arg(used).arg(allocated).arg(current_livereg_data_.total_vgprs).toStdString();
            }
            else
            {
                allocated = ((used / block_allocation_value) + 1) * block_allocation_value;
                reduction = used % block_allocation_value;
                tooltip   = (QString(kLiveVgprTooltip1).arg(used).arg(allocated).arg(current_livereg_data_.total_vgprs) +
                           QString(kLiveVgprTooltip2).arg(reduction).arg(block_allocation_value).arg(block_allocation_value))
                              .toStdString();
            }
        }
    }
}

bool RgIsaItemModel::SetArchitecture(const std::string target_gpu)
{
    bool                    success      = true;
    amdisa::GpuArchitecture architecture = amdisa::GpuArchitecture::kUnknown;
    if (RgaSharedUtils::IsNavi4Target(target_gpu))
    {
        architecture = amdisa::GpuArchitecture::kRdna4;
    }
    else if (RgaSharedUtils::IsStrix(target_gpu))
    {
        architecture = amdisa::GpuArchitecture::kRdna3_5;
    }
    else if (RgaSharedUtils::IsNavi3Target(target_gpu))
    {
        architecture = amdisa::GpuArchitecture::kRdna3;
    }
    else if (RgaSharedUtils::IsNavi21AndBeyond(target_gpu))
    {
        architecture = amdisa::GpuArchitecture::kRdna2;
    }
    else if (RgaSharedUtils::IsNaviTarget(target_gpu))
    {
        architecture = amdisa::GpuArchitecture::kRdna1;
    }
    else if (RgaSharedUtils::IsVegaTarget(target_gpu))
    {
        if (RgaSharedUtils::IsMi300Target(target_gpu))
        {
            architecture = amdisa::GpuArchitecture::kCdna3;
        }
        else if (RgaSharedUtils::IsMi200Target(target_gpu))
        {
            architecture = amdisa::GpuArchitecture::kCdna2;
        }
        else
        {
            success = false;
        }
    }
    else
    {
        success = false;
    }

    IsaItemModel::SetArchitecture(architecture, false);

    return success;
}

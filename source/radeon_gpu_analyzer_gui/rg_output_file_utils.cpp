// Qt.
#include <QString>
#include <QFile>
#include <QRegularExpression>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QTextStream>

// Local.
#include "source/radeon_gpu_analyzer_gui/rg_data_types.h"
#include "source/radeon_gpu_analyzer_gui/rg_definitions.h"
#include "source/radeon_gpu_analyzer_gui/rg_string_constants.h"
#include "radeon_gpu_analyzer_gui/qt/rg_output_file_utils.h"
#include "radeon_gpu_analyzer_gui/rg_utils.h"

bool RgOutputFileUtils::ParseLiveVgprsData(const std::string&                                  live_vgpr_file_full_path,
                                           std::vector<std::shared_ptr<RgIsaLine>>&            disassembled_isa_lines,
                                           std::vector<std::shared_ptr<RgIsaLineInstruction>>& vgpr_isa_lines,
                                           std::vector<QString>&                               vgpr_file_lines,
                                           RgLiveregData&                                      livereg_data)
{
    bool status = false;

    // Read the live VGPR output file.
    RgOutputFileUtils::ReadLiveVgprsFile(live_vgpr_file_full_path, vgpr_file_lines);

    // Parse each line.
    for (const auto& live_vgpr_line : vgpr_file_lines)
    {
        // Process the line if it is a valid line.
        if (RgOutputFileUtils::IsValidLine(live_vgpr_line))
        {
            bool is_label = IsLabelLine(live_vgpr_line);
            RgOutputFileUtils::ParseLiveVgprIsaLine(live_vgpr_line, vgpr_isa_lines, is_label);
        }
    }

    // Now extract the architecture specific information.
    status = RgOutputFileUtils::GetArchInformation(livereg_data, vgpr_file_lines);

    // Calculate the max live VGPR values and extract the live VGPR numbers.
    if (status)
    {
        // Extract maximum VGPR lines.
        livereg_data.max_vgprs = RgOutputFileUtils::CalculateMaxVgprs(vgpr_isa_lines, livereg_data.max_vgpr_line_numbers, disassembled_isa_lines);

        // Allocate space for the vector.
        livereg_data.is_current_max_vgpr_line_number.resize(livereg_data.max_vgpr_line_numbers.size());

        // Set all booleans to false.
        std::fill(livereg_data.is_current_max_vgpr_line_number.begin(), livereg_data.is_current_max_vgpr_line_number.end(), false);

        // Keep track of the line number.
        int line_number = 0;

        // Iterate through VGPR opcodes and compare them to disassembly opcodes.
        for (const auto& line : vgpr_isa_lines)
        {
            std::shared_ptr<RgIsaLineInstruction> vgpr_line       = std::static_pointer_cast<RgIsaLineInstruction>(line);
            std::shared_ptr<RgIsaLineInstruction> disassembly_line = std::static_pointer_cast<RgIsaLineInstruction>(disassembled_isa_lines[line_number]);

            // If this is a label line, or a s_nop line, just continue onto the next one.
            while ((disassembly_line->type == RgIsaLineType::kLabel) || (disassembly_line->opcode.compare("s_nop") == 0))
            {
                // If there is a matching s_nop line in livereg output, break out of here.
                // Not all livereg output has a matching s_nop line.
                if ((vgpr_line->opcode.compare("s_nop") == 0) && (disassembly_line->type == RgIsaLineType::kInstruction))
                {
                    break;
                }

                // Set the live register value to "label".
                disassembly_line->num_live_registers = kLiveVgprLabelString;

                // Advance to the next line.
                line_number++;
                disassembly_line = std::static_pointer_cast<RgIsaLineInstruction>(disassembled_isa_lines[line_number]);
            }

            assert(disassembly_line->type == RgIsaLineType::kInstruction);
            if (disassembly_line->type == RgIsaLineType::kInstruction)
            {
                // If the opcode has a trailing "_e32", "_e64", "_sdwa" or "_dpp", it is still a match.
                if ((vgpr_line->opcode.compare(disassembly_line->opcode) == 0) 
                    || (disassembly_line->opcode.compare(vgpr_line->opcode + "_e32") == 0) 
                    || (disassembly_line->opcode.compare(vgpr_line->opcode + "_e64") == 0) 
                    || (disassembly_line->opcode.compare(vgpr_line->opcode + "_sdwa") == 0) 
                    || (disassembly_line->opcode.compare(vgpr_line->opcode + "_dpp") == 0))
                {
                    // Save both the number of live VGPRs and the block granularity.
                    disassembly_line->num_live_registers = vgpr_line->num_live_registers + "," + std::to_string(livereg_data.vgprs_granularity);
                }
                else
                {
                    // If the instructions did not match, display "N/A".
                    disassembly_line->num_live_registers = "N/A";
                    livereg_data.unmatched_count++;
                }
            }
            line_number++;

            // Check for valid line number.
            if (line_number > disassembled_isa_lines.size() - 1)
            {
                break;
            }
        }
    }

    return status;
}

int RgOutputFileUtils::CalculateMaxVgprs(const std::vector<std::shared_ptr<RgIsaLineInstruction>>& vgpr_isa_lines,
                                         std::vector<int>& max_line_numbers,
                                         std::vector<std::shared_ptr<RgIsaLine>>& disassembled_isa_lines)
{
    int max_vgprs_used    = 0;
    int isa_line_number   = 0;
    int vgpr_line_number  = 0;
    while (isa_line_number < disassembled_isa_lines.size())
    {
        const std::shared_ptr<RgIsaLineInstruction> current_vgpr_line = vgpr_isa_lines[vgpr_line_number];
        QString                                     value             = QString::fromStdString(current_vgpr_line->num_live_registers);
        std::shared_ptr<RgIsaLineInstruction>       disassembly_line  = std::static_pointer_cast<RgIsaLineInstruction>(disassembled_isa_lines[isa_line_number]);

        if (disassembly_line->type == RgIsaLineType::kLabel)
        {
            isa_line_number++;
        }
        else if ((disassembly_line->type == RgIsaLineType::kInstruction) && (disassembly_line->opcode == "s_nop"))
        {
            isa_line_number++;

            // If the VGPR line has a matching s_nop, bump up that line number too.
            if (current_vgpr_line->opcode == "s_nop")
            {
                vgpr_line_number++;
            }
        }
        else if (disassembly_line->type == RgIsaLineType::kInstruction)
        {
            // Update the max VGPR values.
            if (value.toInt() >= max_vgprs_used)
            {
                if (value.toInt() > max_vgprs_used)
                {
                    // Clear the values saved so far.
                    max_line_numbers.clear();
                }

                // Save the max VGPR value.
                max_vgprs_used = value.toInt();

                // Save the line number here as well.
                max_line_numbers.push_back(isa_line_number);
            }

            // Bump up the line numbers.
            isa_line_number++;
            vgpr_line_number++;
        }
    }

    return max_vgprs_used;
}

bool RgOutputFileUtils::IsValidLine(const QString& livereg_line)
{
    bool result = false;

    // Parse the line.
    const QRegularExpression reg("^\\s*(\\d+)\\s*\\|\\s*\\d+\\s*\\|\\s*");
    QRegularExpressionMatch  match = reg.match(livereg_line);

    if (match.hasMatch())
    {
        result = true;
    }

    return result;
}

bool RgOutputFileUtils::IsLabelLine(const QString& livereg_line)
{
    bool status = false;

    // Parse the line.
    const QRegularExpression reg("^\\s*(\\d+)\\s*\\|\\s*(\\d+)\\s*\\|.*\\|\\s*(label_.*):");
    QRegularExpressionMatch  match = reg.match(livereg_line);

    if (match.hasMatch())
    {
        status = true;
    }

    return status;
}

bool RgOutputFileUtils::ReadLiveVgprsFile(const std::string& live_vgpr_file_full_path, std::vector<QString>& vgpr_file_lines)
{
    bool status = false;

    QString text;
    bool    is_file_opened = RgUtils::ReadTextFile(live_vgpr_file_full_path, text);
    QTextStream file_stream(&text);

    // Process the data.
    assert(is_file_opened);
    if (is_file_opened)
    {
        // Read the live VGPR file line.
        QString live_vgpr_line = file_stream.readLine();
        while (!live_vgpr_line.isNull())
        {
            // Save the line.
            vgpr_file_lines.push_back(live_vgpr_line);

            // Read the next live VGPR file line.
            live_vgpr_line = file_stream.readLine();
        }
        status = true;
    }
    return status;
}

void RgOutputFileUtils::ParseLiveVgprIsaLine(const QString& livereg_line, std::vector<std::shared_ptr<RgIsaLineInstruction>>& vgpr_isa_lines, bool is_label)
{
    const QRegularExpression label_regex("^\\s*(\\d+)\\s*\\|\\s*(\\d+)\\s*\\|\\s*([:\\^vx\\s*]+)\\s*\\|\\s*[^:]*:\\s*(\\w+)");
    const QRegularExpression non_label_regex("^\\s*(\\d+)\\s*\\|\\s*(\\d+)\\s*\\|\\s*([:^vx\\s*]+)\\s*\\|\\s*(\\w+)\\s*");
    QRegularExpressionMatch  match;

    if (is_label)
    {
        match = label_regex.match(livereg_line);
    }
    else
    {
        match = non_label_regex.match(livereg_line);
    }

    // Create a new line.
    std::shared_ptr<RgIsaLineInstruction> new_line = std::make_shared<RgIsaLineInstruction>();

    if (match.hasMatch())
    {
        // Extract the number of live registers and the operand.
        new_line->num_live_registers   = std::to_string(match.captured(2).toInt());
        new_line->opcode               = match.captured(4).toStdString();
    }
    else
    {
        // If there was no match, display "N/A" instead of the number of registers used.
        new_line->num_live_registers                   = "N/A";
    }

    // Save the newly created line.
    vgpr_isa_lines.push_back(new_line);
}

bool RgOutputFileUtils::GetArchInformation(RgLiveregData& livereg_data,
                                           std::vector<QString>& vgpr_file_lines)
{
    // Initialize flags.
    bool is_vgprs_total = false;
    bool is_vgprs_granularity = false;
    bool is_vgprs_used_and_allocated = false;
    bool is_no_vgprs_used = false;

    // Create regular expressions.
    const QRegularExpression vgprs_total_regex("^\\s+VGPRs total:\\s+(\\d+)");
    const QRegularExpression vgprs_granularity_regex("^\\s+VGPR allocation granularity:\\s+(\\d+)");
    const QRegularExpression vgprs_used_and_allocated_by_hw_regex("^Maximum\\s*#\\s*VGPR\\s*used\\s*(\\d+),\\s*VGPRs\\s*allocated\\s*by\\s*HW:\\s*(\\d+)");
    const QRegularExpression no_vgprs_used_regex("^\\s*No VGPRs used\\s*");

    // Look thru the file lines for max VGPRs and block granularity.
    for (const auto& live_vgpr_line : vgpr_file_lines)
    {
        QRegularExpressionMatch match = vgprs_total_regex.match(live_vgpr_line);
        if (match.hasMatch())
        {
            livereg_data.total_vgprs = match.captured(1).toInt();
            is_vgprs_total           = true;
        }
        match = vgprs_granularity_regex.match(live_vgpr_line);
        if (match.hasMatch())
        {
            livereg_data.vgprs_granularity  = match.captured(1).toInt();
            is_vgprs_granularity            = true;
        }
        match = vgprs_used_and_allocated_by_hw_regex.match(live_vgpr_line);
        if (match.hasMatch())
        {
            livereg_data.used                   = match.captured(1).toInt();
            livereg_data.allocated              = match.captured(2).toInt();
            is_vgprs_used_and_allocated         = true;
        }
        match = no_vgprs_used_regex.match(live_vgpr_line);
        if (match.hasMatch())
        {
            is_no_vgprs_used = true;
        }
    }
    return (is_vgprs_total && is_vgprs_granularity && is_vgprs_used_and_allocated) || is_no_vgprs_used;
}
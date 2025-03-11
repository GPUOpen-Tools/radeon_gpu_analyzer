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

int RgOutputFileUtils::CalculateMaxVgprs(const std::vector<std::shared_ptr<RgIsaLineInstruction>>& vgpr_isa_lines,
                                         std::vector<int>& max_line_numbers,
                                         std::vector<std::shared_ptr<RgIsaLine>>& disassembled_isa_lines)
{
    int max_vgprs_used    = 0;
    int isa_line_number   = 0;
    int vgpr_line_number  = 0;

    auto evaluate = [&vgpr_isa_lines, &disassembled_isa_lines, &vgpr_line_number, &isa_line_number]() {
        if (vgpr_isa_lines.size() < disassembled_isa_lines.size())
            return vgpr_line_number < vgpr_isa_lines.size();
        else
            return isa_line_number < disassembled_isa_lines.size();
    };

    while (evaluate())
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
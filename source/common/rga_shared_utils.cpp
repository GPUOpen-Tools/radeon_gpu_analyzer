//=============================================================================
/// Copyright (c) 2021-2026 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for an rga common utilities.
//=============================================================================

// C++
#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>
#include <time.h>
#include <ctime>

// Infra.
#include "external/amdt_base_tools/Include/gtString.h"
#include "external/amdt_os_wrappers/Include/osFilePath.h"
#include "external/amdt_os_wrappers/Include/osFile.h"
#include "external/amdt_os_wrappers/Include/osDirectory.h"

// Local
#include "common/rg_log.h"
#include "common/rga_shared_data_types.h"
#include "common/rga_shared_utils.h"
#include <amdisa/isa_decoder.h>

// *** INTERNALLY LINKED SYMBOLS - BEGIN ***
static const int kWindowsDateStringLength      = 14;
static const int kWindowsDateStringYearOffset  = 10;
static const int kWindowsDateStringDayOffset   = 7;
static const int kWindowsDateStringMonthOffset = 4;
// *** INTERNALLY LINKED SYMBOLS - END ***

// Instructions shader disassembly text delimiter.
static const char* kCodeCommentToken       = "//";
static const char  kColumnToken            = ':';
std::size_t        kCodeCommentTokenLength = 2 + 1;  // "// "
std::size_t        kColumnTokenLength      = 2;      // ": "
// ----

static const char* kStrErrorFailedToOpenXmlFile = "Error: failed to open device dictionary xml file: ";
static const char* kStrErrorFailedToReadXmlFile = "Error: failed to parse device dictionary xml file: ";

bool RgaSharedUtils::ConvertDateString(std::string& date_string)
{
    bool ret = false;

#ifdef _WIN32

    bool is_conversion_required = (date_string != kStrRgaBuildDateDev);

    // Convert Windows date format to "YYYY-MM-DD".
    if (is_conversion_required)
    {
        ret = (date_string.find('/') != std::string::npos && date_string.size() == kWindowsDateStringLength);
        if (ret)
        {
            date_string = date_string.substr(kWindowsDateStringMonthOffset, 2) + "/" + date_string.substr(kWindowsDateStringDayOffset, 2) + "/" +
                          date_string.substr(kWindowsDateStringYearOffset, 4);
        }
        else
        {
            // We shouldn't get here.
            assert(false);
        }
    }

    // If a conversion is not required, this is really a success.
    ret = (!is_conversion_required || ret);

#else
    // Conversion is only required on Windows.
    ret = true;
#endif  // !_WIN32

    return ret;
}

// Compares two paths, based on ignored case and with standardized path separators.
// Returns true if the paths match; false otherwise.
bool RgaSharedUtils::ComparePaths(const std::string& path1, const std::string& path2)
{
    std::string p1_lower = path1;
    std::string p2_lower = path2;

    // Make both paths lowercase and with forward slashes.
    std::transform(
        p1_lower.begin(), p1_lower.end(), p1_lower.begin(), [](unsigned char c) { return static_cast<unsigned char>((c == '\\') ? '/' : std::tolower(c)); });
    std::transform(
        p2_lower.begin(), p2_lower.end(), p2_lower.begin(), [](unsigned char c) { return static_cast<unsigned char>((c == '\\') ? '/' : std::tolower(c)); });

    return (p1_lower == p2_lower);
}

// Get current system time.
static bool CurrentTime(struct tm& time_data)
{
    bool              ret = false;
    std::stringstream suffix;
    time_t            current_time = std::time(0);
#ifdef _WIN32
    struct tm* time_local = &time_data;
    ret                   = (localtime_s(time_local, &current_time) == 0);
#else
    struct tm* time_local = localtime(&current_time);
    if (time_local != nullptr)
    {
        time_data = *time_local;
        ret       = true;
    }
#endif
    return ret;
}

// Delete log files older than "daysNum" days.
bool RgaSharedUtils::DeleteOldLogs(const std::string& dir, const std::string& base_file_name, unsigned int days_num)
{
    bool         ret         = false;
    const double seconds_num = static_cast<double>(days_num * 24 * 60 * 60);
    gtString     dir_gtstr, base_file_name_gtstr;
    dir_gtstr << dir.c_str();
    base_file_name_gtstr << base_file_name.c_str();
    osFilePath log_file_path(base_file_name_gtstr);
    log_file_path.setFileDirectory(dir_gtstr);
    osDirectory log_dir;
    ret = log_file_path.getFileDirectory(log_dir) && log_dir.exists();
    assert(log_dir.exists());

    if (ret)
    {
        gtString           log_file_pattern, filename_gtstr, file_ext_gtstr;
        gtList<osFilePath> file_paths;
        log_file_path.getFileName(filename_gtstr);
        log_file_path.getFileExtension(file_ext_gtstr);
        if ((ret = !filename_gtstr.isEmpty()) == true)
        {
            log_file_pattern << filename_gtstr.asASCIICharArray() << "*." << file_ext_gtstr.asASCIICharArray();
            if (log_dir.getContainedFilePaths(log_file_pattern, osDirectory::SORT_BY_DATE_ASCENDING, file_paths))
            {
                for (const osFilePath& path : file_paths)
                {
                    osStatStructure file_stat;
                    if ((ret = (osWStat(path.asString(), file_stat) == 0)) == true)
                    {
                        time_t    file_time = file_stat.st_ctime;
                        struct tm time;
                        if ((ret = CurrentTime(time)) == true)
                        {
                            time_t curr_time = std::mktime(&time);
                            if (std::difftime(curr_time, file_time) > seconds_num)
                            {
                                std::remove(path.asString().asASCIICharArray());
                            }
                        }
                    }
                }
            }
        }
    }

    return ret;
}

std::string RgaSharedUtils::ConstructLogFileName(const std::string& base_file_name)
{
    struct tm tt
    {
    };
    osFilePath log_file_name;
    bool       status = !base_file_name.empty();
    if (status)
    {
        gtString base_file_name_gtstr;
        base_file_name_gtstr << base_file_name.c_str();
        log_file_name = base_file_name_gtstr;
    }

    // Lambda extending numbers < 10 with leading 0.
    auto zero_ext = [](int n) {
        std::string n_str = std::to_string(n);
        return (n < 10 ? std::string("0") + n_str : n_str);
    };
    status = status && CurrentTime(tt);
    if (status)
    {
        // Append current date/time to the log file name.
        std::stringstream suffix;
        suffix << "-" << std::to_string(tt.tm_year + 1900) << zero_ext(tt.tm_mon + 1) << zero_ext(tt.tm_mday) << "-" << zero_ext(tt.tm_hour)
               << zero_ext(tt.tm_min) << zero_ext(tt.tm_sec);

        gtString filename_gtstr;
        log_file_name.getFileName(filename_gtstr);
        std::string fileName = filename_gtstr.asASCIICharArray();
        filename_gtstr.fromASCIIString((fileName + suffix.str()).c_str());
        log_file_name.setFileName(filename_gtstr);
    }

    return (status ? log_file_name.asString().asASCIICharArray() : "");
}

bool RgaSharedUtils::InitLogFile(const std::string& dir, const std::string& base_file_name, unsigned int oldFilesDaysNum)
{
    bool status = DeleteOldLogs(dir, base_file_name, oldFilesDaysNum);
    if (status)
    {
        std::string log_filename = ConstructLogFileName(base_file_name);
        if ((status = !log_filename.empty()) == true)
        {
            status = RgLog::OpenLogFile(dir, log_filename);
        }
    }
    return status;
}

void RgaSharedUtils::CloseLogFile()
{
    RgLog::Close();
}

std::string RgaSharedUtils::ToLower(const std::string& str)
{
    std::string lstr = str;
    std::transform(lstr.begin(), lstr.end(), lstr.begin(), [](const char& c) { return static_cast<char>(std::tolower(c)); });
    return lstr;
}

std::string RgaSharedUtils::ToUpper(const std::string& str)
{
    std::string ustr = str;
    std::transform(ustr.begin(), ustr.end(), ustr.begin(), [](const char& c) { return static_cast<char>(std::toupper(c)); });
    return ustr;
}

bool RgaSharedUtils::IsNavi4Target(const std::string& target_name)
{
    // Token to identify Navi4 targets.
    static const char* kNavi4TargetToken = "gfx12";
    return (target_name.find(kNavi4TargetToken) != std::string::npos);
}

bool RgaSharedUtils::IsNavi3Dot5Target(const std::string& target_name)
{
    // Token to identify Navi3.5 targets.
    static const char* kNavi3Dot5TargetToken = "gfx115";
    return (target_name.find(kNavi3Dot5TargetToken) != std::string::npos);
}

bool RgaSharedUtils::IsNavi3AndBeyond(const std::string& target_name)
{
    return IsNavi4Target(target_name) || IsNavi3Dot5Target(target_name) || IsNavi3Target(target_name);
}

bool RgaSharedUtils::IsNavi3Target(const std::string& target_name)
{
    // Token to identify Navi3 targets.
    static const char* kNavi3TargetToken = "gfx11";
    return (target_name.find(kNavi3TargetToken) != std::string::npos);
}

bool RgaSharedUtils::IsNaviTarget(const std::string& target_name)
{
    // Token to identify Navi targets.
    static const char* kNaviTargetToken = "gfx1";
    return (target_name.find(kNaviTargetToken) != std::string::npos);
}

bool RgaSharedUtils::IsNavi21AndBeyond(const std::string& target_name)
{
    return (IsNaviTarget(target_name) && target_name >= "gfx1030");
}

bool RgaSharedUtils::IsNavi21(const std::string& target_name)
{
    return target_name == "gfx1030";
}

bool RgaSharedUtils::IsMi200Target(const std::string& target_name)
{
    return target_name == "gfx90a";
}

bool RgaSharedUtils::IsMi300Target(const std::string& target_name)
{
    return target_name == "gfx942";
}

bool RgaSharedUtils::IsMi350Target(const std::string& target_name)
{
    return target_name == "gfx950";
}

bool RgaSharedUtils::IsMi450Target(const std::string& target_name)
{
    // Token to identify MI400 targets.
    static const char* kMi400TargetToken = "gfx125";
    return (target_name.find(kMi400TargetToken) != std::string::npos);
}

bool RgaSharedUtils::HasAgprSupport(const std::string& target_name)
{
    return IsMi200Target(target_name) || IsMi300Target(target_name) || IsMi350Target(target_name);
}

bool RgaSharedUtils::IsVegaTarget(const std::string& target_name)
{
    // Token to identify Vega targets.
    static const char* kVegaTargetToken = "gfx9";
    return (target_name.find(kVegaTargetToken) != std::string::npos);
}

bool RgaSharedUtils::IsIsaInstructionLine(const std::string& isa_line, std::size_t& comment_pos, std::string& pc_address, std::string& binary_representation)
{
    // Go to the inline comment in the line read
    std::size_t pos_comment = isa_line.find(kCodeCommentToken);
    std::size_t pos_address = pos_comment;

    // Ignore sp3 program line comments
    bool is_code_line = false;
    if (pos_comment > 0 && pos_comment != std::string::npos)
    {
        // Parse dword/s if match found
        pos_comment = isa_line.find(kColumnToken, pos_comment);
        if (pos_comment != std::string::npos)
        {
            // Skip through to the first dword (": ", 2 characters)
            if ((pos_comment + kColumnTokenLength) < isa_line.length())
            {
                pos_comment += kColumnTokenLength;
                is_code_line = true;
            }
        }
    }

    // Ignore the code lines with special words that look like assembly.
    is_code_line = is_code_line && (isa_line.find(".long") == std::string::npos);
    is_code_line = is_code_line && (isa_line.find(".ascii") == std::string::npos);
    is_code_line = is_code_line && (isa_line.find(".byte") == std::string::npos);

    // If parsing was successful, then decode the instructions
    if (is_code_line)
    {
        comment_pos = pos_comment;

        // Skip through from the "//" upto the first dword (": ", 2 characters)
        pc_address = isa_line.substr(pos_address + kCodeCommentTokenLength, pos_comment - pos_address - kCodeCommentTokenLength - kColumnTokenLength);

        binary_representation = isa_line.substr(pos_comment);
    }

    return is_code_line;
}

bool RgaSharedUtils::GetSourceLineInfo(const std::string& isa_line,
                                       const std::string& prev_isa_line,
                                       std::string&       src_file_path,
                                       std::string&       src_line,
                                       uint32_t&          src_line_number)
{
    bool ret = false;

    // Source line info has the following format:
    // ; C:\DEV\work\line_numbers\test.cl:4    <-- prevIsaLine
    // ; A[0] = 0.0f;                          <-- isaLine
    const size_t src_line_offset = 2;
    size_t       colon_offset    = 0;
    if (prev_isa_line.find(';') == 0 && isa_line.find(';') == 0 && ((colon_offset = prev_isa_line.rfind(':')) != std::string::npos))
    {
        std::string line_num_str = prev_isa_line.substr(colon_offset + 1);
        if (!line_num_str.empty() && std::all_of(line_num_str.begin(), line_num_str.end(), ::isdigit))
        {
            uint32_t parsed_line_num = std::atoi(line_num_str.c_str());
            if (parsed_line_num > 0)
            {
                src_file_path   = prev_isa_line.substr(src_line_offset, colon_offset - src_line_offset);
                // Normalize path separators for cross-compiled binaries (e.g. Windows '\' on Linux).
                {
                    gtString gt_path;
                    gt_path.fromASCIIString(src_file_path.c_str());
                    osFilePath::adjustStringToCurrentOS(gt_path);
                    src_file_path = gt_path.asASCIICharArray();
                }
                src_line        = isa_line.substr(src_line_offset);
                src_line_number = parsed_line_num;
                ret             = true;
            }
        }
    }

    return ret;
}

void RgaSharedUtils::ParseCsvLine(std::string isa_line, std::vector<std::string>& line_tokens, std::vector<std::string>& operand_tokens_str)
{
    std::stringstream line_stream;
    line_stream.str(isa_line);
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
        if (operand_tokens_str.size() == 0 && line_tokens.size() > static_cast<int>(RgCsvFileColumns::kOperands))
        {
            operand_tokens_str.push_back(line_tokens[static_cast<int>(RgCsvFileColumns::kOperands)]);
        }
    }
}

bool RgaSharedUtils::GetGpuArchitectureFromTarget(const std::string& target_gpu, amdisa::GpuArchitecture& architecture)
{
    bool success = true;
    architecture = amdisa::GpuArchitecture::kUnknown;
    if (RgaSharedUtils::IsMi450Target(target_gpu))
    {
        architecture = amdisa::GpuArchitecture::kCdna5;
    }
    else if (RgaSharedUtils::IsNavi4Target(target_gpu))
    {
        architecture = amdisa::GpuArchitecture::kRdna4;
    }
    else if (RgaSharedUtils::IsNavi3Dot5Target(target_gpu))
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
        if (RgaSharedUtils::IsMi350Target(target_gpu))
        {
            architecture = amdisa::GpuArchitecture::kCdna4;
        }
        else if (RgaSharedUtils::IsMi300Target(target_gpu))
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
    return success;
}

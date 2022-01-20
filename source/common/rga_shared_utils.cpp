// C++
#include <cassert>
#include <sstream>
#include <time.h>
#include <ctime>

// Infra.
#ifdef _WIN32
    #pragma warning(push)
    #pragma warning(disable:4309)
#endif
#include "external/amdt_base_tools/Include/gtString.h"
#include "external/amdt_os_wrappers/Include/osFilePath.h"
#include "external/amdt_os_wrappers/Include/osFile.h"
#include "external/amdt_os_wrappers/Include/osDirectory.h"
#ifdef _WIN32
    #pragma warning(pop)
#endif

// Local
#include "common/rga_shared_utils.h"
#include "common/rg_log.h"

// *** INTERNALLY LINKED SYMBOLS - BEGIN ***
static const int  kWindowsDateStringLength = 14;
static const int  kWindowsDateStringYearOffset = 10;
static const int  kWindowsDateStringDayOffset = 7;
static const int  kWindowsDateStringMonthOffset = 4;
// *** INTERNALLY LINKED SYMBOLS - END ***

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
            date_string = date_string.substr(kWindowsDateStringMonthOffset, 2) + "/" +
                date_string.substr(kWindowsDateStringDayOffset, 2) + "/" +
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
#endif // !_WIN32

    return ret;
}

// Compares two paths, based on ignored case and with standardized path separators.
// Returns true if the paths match; false otherwise.
bool RgaSharedUtils::ComparePaths(const std::string& path1, const std::string& path2)
{
    std::string p1_lower = path1;
    std::string p2_lower = path2;

    // Make both paths lowercase and with forward slashes.
    std::transform(p1_lower.begin(), p1_lower.end(), p1_lower.begin(), [](unsigned char c) {return (c == '\\') ? '/' : std::tolower(c); });
    std::transform(p2_lower.begin(), p2_lower.end(), p2_lower.begin(), [](unsigned char c) {return (c == '\\') ? '/' : std::tolower(c); });

    return (p1_lower == p2_lower);
}

// Get current system time.
static bool CurrentTime(struct tm& time_data)
{
    bool ret = false;
    std::stringstream suffix;
    time_t  current_time = std::time(0);
#ifdef _WIN32
    struct tm* time_local = &time_data;
    ret = (localtime_s(time_local, &current_time) == 0);
#else
    struct tm* time_local = localtime(&current_time);
    if (time_local != nullptr)
    {
        time_data = *time_local;
        ret = true;
    }
#endif
    return ret;
}

// Delete log files older than "daysNum" days.
bool RgaSharedUtils::DeleteOldLogs(const std::string& dir, const std::string& base_file_name, unsigned int days_num)
{
    bool ret = false;
    const double seconds_num = static_cast<double>(days_num * 24 * 60 * 60);
    gtString dir_gtstr, base_file_name_gtstr;
    dir_gtstr << dir.c_str();
    base_file_name_gtstr << base_file_name.c_str();
    osFilePath  log_file_path(base_file_name_gtstr);
    log_file_path.setFileDirectory(dir_gtstr);
    osDirectory log_dir;
    ret = log_file_path.getFileDirectory(log_dir) && log_dir.exists();
    assert(log_dir.exists());

    if (ret)
    {
        gtString log_file_pattern, filename_gtstr, file_ext_gtstr;
        gtList<osFilePath>  file_paths;
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
                        time_t file_time = file_stat.st_ctime;
                        struct tm  time;
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
    struct tm tt;
    osFilePath log_file_name;
    bool status = !base_file_name.empty();
    if (status)
    {
        gtString base_file_name_gtstr;
        base_file_name_gtstr << base_file_name.c_str();
        log_file_name = base_file_name_gtstr;
    }

    // Lambda extending numbers < 10 with leading 0.
    auto zero_ext = [](int n) {std::string n_str = std::to_string(n); return (n < 10 ? std::string("0") + n_str : n_str); };
    status = status && CurrentTime(tt);
    if (status)
    {
        // Append current date/time to the log file name.
        std::stringstream  suffix;
        suffix << "-" << std::to_string(tt.tm_year + 1900) << zero_ext(tt.tm_mon + 1) << zero_ext(tt.tm_mday) <<
                  "-" << zero_ext(tt.tm_hour) << zero_ext(tt.tm_min) << zero_ext(tt.tm_sec);

        gtString  filename_gtstr;
        log_file_name.getFileName(filename_gtstr);
        std::string  fileName = filename_gtstr.asASCIICharArray();
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
        std::string  log_filename = ConstructLogFileName(base_file_name);
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

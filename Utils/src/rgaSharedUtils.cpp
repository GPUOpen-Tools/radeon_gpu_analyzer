
// C++
#include <cassert>
#include <sstream>
#include <time.h>
#include <ctime>

// Infra.
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osFile.h>
#include <AMDTOSWrappers/Include/osDirectory.h>

// Local
#include "../include/rgaSharedUtils.h"
#include "../include/rgLog.h"

// *** INTERNALLY LINKED SYMBOLS - BEGIN ***
static const int  WINDOWS_DATE_STRING_LEN = 14;
static const int  WINDOWS_DATE_STRING_YEAR_OFFSET = 10;
static const int  WINDOWS_DATE_STRING_DAY_OFFSET = 7;
static const int  WINDOWS_DATE_STRING_MONTH_OFFSET = 4;
// *** INTERNALLY LINKED SYMBOLS - END ***

bool rgaSharedUtils::ConvertDateString(std::string& dateString)
{
    bool ret = false;

#ifdef _WIN32

    bool isConversionRequired = (dateString != STR_RGA_BUILD_DATE_DEV);

    // Convert Windows date format to "YYYY-MM-DD".
    if (isConversionRequired)
    {
        ret = (dateString.find('/') != std::string::npos && dateString.size() == WINDOWS_DATE_STRING_LEN);
        if (ret)
        {
            dateString = dateString.substr(WINDOWS_DATE_STRING_MONTH_OFFSET, 2) + "/" +
                dateString.substr(WINDOWS_DATE_STRING_DAY_OFFSET, 2) + "/" +
                dateString.substr(WINDOWS_DATE_STRING_YEAR_OFFSET, 4);
        }
        else
        {
            // We shouldn't get here.
            assert(false);
        }
    }

    // If a conversion is not required, this is really a success.
    ret = (!isConversionRequired || ret);

#else
    // Conversion is only required on Windows.
    ret = true;
#endif // !_WIN32

    return ret;
}

// Get current system time.
static bool  CurrentTime(struct tm& time)
{
    bool  ret = false;
    std::stringstream  suffix;
    time_t  currentTime = std::time(0);
#ifdef _WIN32
    struct tm* pTime = &time;
    ret = (localtime_s(pTime, &currentTime) == 0);
#else
    struct tm*  pTime = localtime(&currentTime);
    if (pTime != nullptr)
    {
        time = *pTime;
        ret = true;
    }
#endif
    return ret;
}

// Delete log files older than "daysNum" days.
static bool  DeleteOldLogs(const std::string& dir, const std::string& baseFileName, unsigned int daysNum)
{
    bool  ret = false;
    const double  secondsNum = static_cast<double>(daysNum * 24 * 60 * 60);
    gtString    gDir, gBaseFileName;
    gDir << dir.c_str();
    gBaseFileName << baseFileName.c_str();
    osFilePath  logFilePath(gBaseFileName);
    logFilePath.setFileDirectory(gDir);
    osDirectory logDir;
    ret = logFilePath.getFileDirectory(logDir) && logDir.exists();
    assert(logDir.exists());

    if (ret)
    {
        gtString  logFilePattern, gFileName, gFileExt;
        gtList<osFilePath>  filePaths;
        logFilePath.getFileName(gFileName);
        logFilePath.getFileExtension(gFileExt);
        if ((ret = !gFileName.isEmpty()) == true)
        {
            logFilePattern << gFileName.asASCIICharArray() << "*." << gFileExt.asASCIICharArray();

            if (logDir.getContainedFilePaths(logFilePattern, osDirectory::SORT_BY_DATE_ASCENDING, filePaths))
            {
                for (const osFilePath& path : filePaths)
                {
                    osStatStructure  fileStat;
                    if ((ret = (osWStat(path.asString(), fileStat) == 0)) == true)
                    {
                        time_t  fileTime = fileStat.st_ctime;
                        struct tm  time;
                        if ((ret = CurrentTime(time)) == true)
                        {
                            time_t  curTime = std::mktime(&time);
                            if (std::difftime(curTime, fileTime) > secondsNum)
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

static std::string  ConstructLogFileName(const std::string& baseFileName)
{
    struct tm   tt;
    osFilePath  logFileName;
    bool  status = !baseFileName.empty();
    if (status)
    {
        gtString  gBaseFileName;
        gBaseFileName << baseFileName.c_str();
        logFileName = gBaseFileName;
    }

    // Lambda extending numbers < 10 with leading 0.
    auto ZeroExt = [](int n) {std::string nS = std::to_string(n); return (n < 10 ? std::string("0") + nS : nS); };

    status = status && CurrentTime(tt);

    if (status)
    {
        // Append current date/time to the log file name.
        std::stringstream  suffix;
        suffix << "-" << std::to_string(tt.tm_year + 1900) << ZeroExt(tt.tm_mon + 1) << ZeroExt(tt.tm_mday) <<
                  "-" << ZeroExt(tt.tm_hour) << ZeroExt(tt.tm_min) << ZeroExt(tt.tm_sec);

        gtString  gFileName;
        logFileName.getFileName(gFileName);
        std::string  fileName = gFileName.asASCIICharArray();
        gFileName.fromASCIIString((fileName + suffix.str()).c_str());
        logFileName.setFileName(gFileName);
    }

    return (status ? logFileName.asString().asASCIICharArray() : "");
}

bool  rgaSharedUtils::InitLogFile(const std::string& dir, const std::string& baseFileName, unsigned int oldFilesDaysNum)
{
    bool  status = DeleteOldLogs(dir, baseFileName, oldFilesDaysNum);

    if (status)
    {
        std::string  logFileName = ConstructLogFileName(baseFileName);
        if ((status = !logFileName.empty()) == true)
        {
            status = rgLog::OpenLogFile(dir, logFileName);
        }
    }

    return status;
}

void  rgaSharedUtils::CloseLogFile()
{
    rgLog::Close();
}

#pragma once

// C++.
#include <string>

// Shared between the CLI and the GUI.
#include <Utils/Include/rgaVersionInfo.h>

class rgaSharedUtils
{
public:
    // Convert Windows date format.
    static bool ConvertDateString(std::string& dateString);

    // Compares two paths, based on ignored case and with standardized path separators.
    // Returns true if the paths match; false otherwise.
    static bool ComparePaths(const std::string& path1, const std::string& path2);

    // Construct a name for log file based on specified "baseFileName".
    // The file name will be postfixed with the current date & time.
    // Example:
    //   "baseFileName"   --> log.txt
    //   Constructed name --> log-20180215-104536.txt
    static std::string ConstructLogFileName(const std::string& baseFileName);

    // Delete log files matching the "baseFileName" pattern that are older than "daysNum" days.
    static bool DeleteOldLogs(const std::string& dir, const std::string& baseFileName, unsigned int daysNum);

    // Open new log file and delete old files (older than "oldFilesDaysNum" days).
    // "baseFileName" is the name prefix that will be used to construct log file name.
    // The current date and time will be appended to the base name.
    // Example:
    //   baseFileName:   "rga-cli.log"
    //   Log file name:  "rga-cli-20180210-161055.log
    static bool InitLogFile(const std::string& dir, const std::string& baseFileName, unsigned int oldFilesDaysNum);

    // Close the log file.
    static void  CloseLogFile();

private:
    rgaSharedUtils() = delete;
    ~rgaSharedUtils() = delete;
};

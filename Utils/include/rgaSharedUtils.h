#pragma once

// C++.
#include <string>

// Shared between the CLI and the GUI.
#include <Utils/include/rgaVersionInfo.h>

class rgaSharedUtils
{
public:
    // Convert Windows date format.
    static bool ConvertDateString(std::string& dateString);

    // Open new log file and delete old files (older than "oldFilesDaysNum" days).
    // "baseFileName" is the name prefix that will be used to construct log file name.
    // The current date and time will be appended to the base name.
    // Example:
    //   baseFileName:   "rga-cli.log"
    //   Log file name:  "rga-cli-20180210-161055.log
    static bool  InitLogFile(const std::string& dir, const std::string& baseFileName, unsigned int oldFilesDaysNum);

    // Close the log file.
    static void  CloseLogFile();

private:
    rgaSharedUtils() = delete;
    ~rgaSharedUtils() = delete;
};

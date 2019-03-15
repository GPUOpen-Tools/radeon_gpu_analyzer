#pragma once

// C++.
#include <memory>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/rgDataTypes.h>

// A class responsible for parsing the CLI's version-info results.
class rgXMLCliVersionInfo
{
public:
    // Read the version info from file.
    static bool ReadVersionInfo(const std::string& versionInfoFilePath, std::shared_ptr<rgCliVersionInfo>& pVersionInfo);
};
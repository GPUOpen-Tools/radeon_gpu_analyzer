#pragma once

// C++.
#include <vector>
#include <string>
#include <memory>

// Local.
#include <RadeonGPUAnalyzerGUI/include/rgDataTypes.h>

class rgXmlConfigFile
{
public:

    // Read the project configuration file from the given file path, and return a pointer to a
    // Project object that contains the data from the configuration file.
    static bool ReadProjectConfigFile(const std::string& configFilePath, std::shared_ptr<rgProject>& pProject);

    // Write the Project data into a configuration file.
    static bool WriteConfigFile(const rgProject& project, const std::string& configFilePath);

    // Read the global settings from the global configuration files which is located in the given path.
    static bool ReadGlobalSettings(const std::string& globalConfigFilePath, std::shared_ptr<rgGlobalSettings>& pGlobalSettings);

    // Save the global settings to an RGA global configuration file in the given full path.
    static bool WriteGlobalSettings(std::shared_ptr<rgGlobalSettings> pGlobalSettings, const std::string& globalConfigFilePath);
};
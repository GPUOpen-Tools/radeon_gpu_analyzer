#pragma once

// C++.
#include <string>
#include <memory>

// Forward declarations.
struct rgBuildSettings;
struct rgBuildSettingsOpenCL;
struct rgBuildSettingsVulkan;

class rgCliUtils
{
public:
    // Generate the build settings string that should be passed to the CLI for rgCLBuildSettings.
    // "additionalOptions" flag specifies whether the text from "Additional compiler options" box should be added or not.
    static bool GenerateOpenClBuildSettingsString(const rgBuildSettingsOpenCL& buildSettings, std::string& str, bool additionalOptions = true);

    // Generate the build settings string that should be passed to the CLI for rgVulkanBuildSettings.
    static bool GenerateVulkanBuildSettingsString(const rgBuildSettingsVulkan& buildSettings, std::string& str, bool additionalOptions = true);

private:
    rgCliUtils() = delete;
    ~rgCliUtils() = delete;
};


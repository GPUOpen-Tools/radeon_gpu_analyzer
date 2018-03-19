#pragma once

// C++.
#include <string>
#include <memory>

// Forward declarations.
struct rgBuildSettings;
struct rgCLBuildSettings;

class rgCliUtils
{
public:

    // Generate the build settings string that should be passed to the CLI for rgCLBuildSettings.
    // "additionalOptions" flag specifies whether the text from "Additional compiler options" box should be added or not.
    static bool GenerateBuildSettingsString(std::shared_ptr<rgCLBuildSettings> pBuildSettings, std::string& str, bool additionalOptions = true);

private:
    rgCliUtils() = delete;
    ~rgCliUtils() = delete;
};


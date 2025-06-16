//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for RGA CLI utilities.
//=============================================================================

#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_CLI_UTILS_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_CLI_UTILS_H_

// C++.
#include <string>
#include <memory>

// Forward declarations.
struct RgBuildSettings;
struct RgBuildSettingsOpencl;
struct RgBuildSettingsVulkan;
struct RgBuildSettingsBinary;

class RgCliUtils
{
public:
    // Generate the build settings string that should be passed to the CLI for RgCLBuildSettings.
    // "additionalOptions" flag specifies whether the text from "Additional compiler options" box should be added or not.
    static bool GenerateOpenclBuildSettingsString(const RgBuildSettingsOpencl& build_settings, std::string& str, bool additional_options = true);

    // Generate the build settings string that should be passed to the CLI for RgVulkanBuildSettings.
    static bool GenerateVulkanBuildSettingsString(const RgBuildSettingsVulkan& build_settings, std::string& str, bool additional_options = true);

    // Generate the build settings string that should be passed to the CLI for RgCLBuildSettings.
    // "additionalOptions" flag specifies whether the text from "Additional compiler options" box should be added or not.
    static bool GenerateBinaryBuildSettingsString(const RgBuildSettingsBinary& build_settings, std::string& str, bool additional_options = true);

private:
    RgCliUtils() = delete;
    ~RgCliUtils() = delete;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_CLI_UTILS_H_


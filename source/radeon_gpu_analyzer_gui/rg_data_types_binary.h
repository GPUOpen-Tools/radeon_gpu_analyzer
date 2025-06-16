//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for the RGA gui binary mode data types.
//=============================================================================
#pragma once

// Local.
#include "radeon_gpu_analyzer_gui/rg_data_types.h"
#include "radeon_gpu_analyzer_gui/rg_config_manager.h"

// *** BINARY STRING CONSTANTS - START ***

// Shader source file extensions.
static const char* kStrSourceFileExtensionBin = ".bin";
static const char* kStrSourceFileExtensionElf = ".elf";
static const char* kStrSourceFileExtensionCO  = ".co";

// Binary API Name.
static const char* kStrApiNameBinary = "Binary Analysis";
static const char* kStrApiAbbreviationBinary = "Binary";

// Default Binary Build Settings string.
static const char* kStrDefaultBuildSettingsBinary = "Default Binary Analysis build settings";

// Open Existing Code Object menu item.
static const char* kStrMenuBarOpenExistingFileBinary        = "&Load Code Object Binary";
static const char* kStrMenuBarOpenExistingFileTooltipBinary = "Open an existing Code Object (.bin) file (Ctrl+O).";

// Rename project dialog title string.
static const char* kStrRenameProjectDialogBoxTitleBinary = "New Binary Analysis Project";

// Link Existing Source File.
static const char* kStrMenuBarLinkSourceFileTooltipBinary = "Link existing source code file (Ctrl+O).";


// *** BINARY STRING CONSTANTS - END ***

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// *** BINARY TYPE DECLARATIONS - START ***

// Binary build settings.
struct RgBuildSettingsBinary : public RgBuildSettings
{
    RgBuildSettingsBinary()          = default;
    virtual ~RgBuildSettingsBinary() = default;

    // Copy constructor used to initialize using another instance.
    RgBuildSettingsBinary(const RgBuildSettingsBinary& other)
        : RgBuildSettings(other)
    {
    }

    // Determine if the supplied settings are the same as the current settings.
    virtual bool HasSameSettings(const RgBuildSettingsBinary& other) const
    {
        bool isSame = RgBuildSettings::HasSameSettings(other);
        return isSame;
    }
};

// A clone of an Binary project.
struct RgProjectCloneBinary : public RgProjectClone
{
    RgProjectCloneBinary()
    {
        // Instantiate an Binary build settings instance by default.
        build_settings = std::make_shared<RgBuildSettingsBinary>();
    }

    RgProjectCloneBinary(const std::string& clone_name, std::shared_ptr<RgBuildSettingsBinary> build_settings)
        : RgProjectClone(clone_name, build_settings)
    {
    }
};

// An Binary project.
struct RgProjectBinary : public RgProject
{
    RgProjectBinary()
        : RgProject("", "", RgProjectAPI::kBinary)
    {
    }

    // CTOR #1.
    RgProjectBinary(const std::string& project_name, const std::string& project_file_full_path)
        : RgProject(project_name, project_file_full_path, RgProjectAPI::kBinary)
    {
    }

    // CTOR #2.
    RgProjectBinary(const std::string& project_name, const std::string& project_file_full_path, const std::vector<std::shared_ptr<RgProjectClone>>& clones)
        : RgProject(project_name, project_file_full_path, RgProjectAPI::kBinary, clones)
    {
    }
};

// *** BINARY TYPE DECLARATIONS - END ***

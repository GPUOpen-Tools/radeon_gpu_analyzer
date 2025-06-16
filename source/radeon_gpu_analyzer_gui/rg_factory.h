//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for the RBase factory used to create API-specific object instances.
//=============================================================================
#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_FACTORY_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_FACTORY_H_

// Qt.
#include <QStatusBar>

// Local.
#include "radeon_gpu_analyzer_gui/rg_data_types.h"

// Forward declares.
class QWidget;
class RgAppState;
class RgBuildSettingsView;
class RgBuildView;
class RgIsaDisassemblyView;
class RgMenu;
class RgMainWindow;
class RgRenameProjectDialog;
class RgSettingsTab;
class RgStartTab;
class RgStatusBar;

// Base factory used to create API-specific object instances.
class RgFactory
{
public:
    virtual ~RgFactory() = default;

    // Get a factory instance based on the given API.
    static std::shared_ptr<RgFactory> CreateFactory(RgProjectAPI api);

    // Create a new application state.
    virtual std::shared_ptr<RgAppState> CreateAppState() = 0;

    // Create a project instance.
    virtual std::shared_ptr<RgProject> CreateProject(const std::string& project_name, const std::string& project_file_full_path) = 0;

    // Create a project clone instance.
    virtual std::shared_ptr<RgProjectClone> CreateProjectClone(const std::string& clone_name) = 0;

    // Create a build settings instance.
    // initial_build_settings is an optional pointer to an existing build settings instance to create a copy of.
    // If initial_build_settings is not provided, the API's default build settings will be used.
    virtual std::shared_ptr<RgBuildSettings> CreateBuildSettings(std::shared_ptr<RgBuildSettings> initial_build_settings) = 0;

    // *** Views - BEGIN ***

    // Create an API-specific build settings view instance.
    virtual RgBuildSettingsView* CreateBuildSettingsView(QWidget* parent, std::shared_ptr<RgBuildSettings> build_settings, bool is_global_settings) = 0;

    // Create an API-specific build view.
    virtual RgBuildView* CreateBuildView(QWidget* parent) = 0;

    // Create an API-specific disassembly view.
    virtual RgIsaDisassemblyView* CreateDisassemblyView(QWidget* parent) = 0;

    // Create an API-specific file menu.
    virtual RgMenu* CreateFileMenu(QWidget* parent) = 0;

    // Apply the stylesheet for the api-specific file menu.
    virtual void ApplyFileMenuStylesheet(QWidget* widget) = 0;

    // Create an API-specific start tab.
    virtual RgStartTab* CreateStartTab(QWidget* parent) = 0;

    // Create an API-specific rename project dialog box.
    virtual RgRenameProjectDialog* CreateRenameProjectDialog(std::string& project_name, QWidget* parent) = 0;

    // Create an API-specific settings tab.
    virtual RgSettingsTab* CreateSettingsTab(QWidget* parent) = 0;

    // Create an API-specific status bar.
    virtual RgStatusBar* CreateStatusBar(QStatusBar* status_bar, QWidget* parent) = 0;

    // *** Views - END ***

protected:
    RgStylesheetPackage stylesheet_package_;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_FACTORY_H_

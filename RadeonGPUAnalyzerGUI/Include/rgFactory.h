#pragma once

// Qt.
#include <QStatusBar>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/rgDataTypes.h>

// Forward declares.
class QWidget;
class rgAppState;
class rgBuildSettingsView;
class rgBuildView;
class rgIsaDisassemblyView;
class rgMenu;
class rgMainWindow;
class rgRenameProjectDialog;
class rgSettingsTab;
class rgStartTab;
class rgStatusBar;

// Base factory used to create API-specific object instances.
class rgFactory
{
public:
    virtual ~rgFactory() = default;

    // Get a factory instance based on the given API.
    static std::shared_ptr<rgFactory> CreateFactory(rgProjectAPI api);

    // Create a new application state.
    virtual std::shared_ptr<rgAppState> CreateAppState() = 0;

    // Create a project instance.
    virtual std::shared_ptr<rgProject> CreateProject(const std::string& projectName, const std::string& projectFileFullPath) = 0;

    // Create a project clone instance.
    virtual std::shared_ptr<rgProjectClone> CreateProjectClone(const std::string& cloneName) = 0;

    // Create a build settings instance.
    // pInitialBuildSettings is an optional pointer to an existing build settings instance to create a copy of.
    // If pInitialBuildSettings is not provided, the API's default build settings will be used.
    virtual std::shared_ptr<rgBuildSettings> CreateBuildSettings(std::shared_ptr<rgBuildSettings> pInitialBuildSettings) = 0;

    // *** Views - BEGIN ***

    // Create an API-specific build settings view instance.
    virtual rgBuildSettingsView* CreateBuildSettingsView(QWidget* pParent, std::shared_ptr<rgBuildSettings> pBuildSettings, bool isGlobalSettings) = 0;

    // Create an API-specific build view.
    virtual rgBuildView* CreateBuildView(QWidget* pParent) = 0;

    // Create an API-specific disassembly view.
    virtual rgIsaDisassemblyView* CreateDisassemblyView(QWidget* pParent) = 0;

    // Create an API-specific file menu.
    virtual rgMenu* CreateFileMenu(QWidget* pParent) = 0;

    // Create an API-specific start tab.
    virtual rgStartTab* CreateStartTab(QWidget* pParent) = 0;

    // Create an API-specific rename project dialog box.
    virtual rgRenameProjectDialog* CreateRenameProjectDialog(std::string& projectName, QWidget* pParent) = 0;

    // Create an API-specific settings tab.
    virtual rgSettingsTab* CreateSettingsTab(QWidget* pParent) = 0;

    // Create an API-specific status bar.
    virtual rgStatusBar* CreateStatusBar(QStatusBar* pStatusBar, QWidget* pParent) = 0;

    // *** Views - END ***

protected:
    rgStylesheetPackage m_stylesheetPackage;
};

#pragma once

// C++.
#include <memory>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/rgFactory.h>

// Forward declares.
class QWidget;
class rgBuildView;
class rgIsaDisassemblyView;

// Base factory used to create API-specific object instances.
class rgFactoryOpenCL : public rgFactory
{
public:
    // Create a new OpenCL application state.
    virtual std::shared_ptr<rgAppState> CreateAppState() override;

    // Create a project instance.
    virtual std::shared_ptr<rgProject> CreateProject(const std::string& projectName, const std::string& projectFileFullPath) override;

    // Create a project clone instance.
    virtual std::shared_ptr<rgProjectClone> CreateProjectClone(const std::string& cloneName) override;

    // Create a build settings instance.
    virtual std::shared_ptr<rgBuildSettings> CreateBuildSettings(std::shared_ptr<rgBuildSettings> pInitialBuildSettings) override;

    // *** Views - BEGIN ***

    // Create an OpenCL-specific build settings view instance.
    virtual rgBuildSettingsView* CreateBuildSettingsView(QWidget* pParent, std::shared_ptr<rgBuildSettings> pBuildSettings, bool isGlobalSettings) override;

    // Create an OpenCL build view.
    virtual rgBuildView* CreateBuildView(QWidget* pParent) override;

    // Create an OpenCL disassembly view.
    virtual rgIsaDisassemblyView* CreateDisassemblyView(QWidget* pParent) override;

    // Create an OpenCL file menu.
    virtual rgMenu* CreateFileMenu(QWidget* pParent) override;

    // Create an OpenCL-specific start tab.
    virtual rgStartTab* CreateStartTab(QWidget* pParent) override;

    // Create an OpenCL-specific project rename dialog box.
    rgRenameProjectDialog* CreateRenameProjectDialog(std::string& projectName, QWidget* pParent) override;

    // Create an API-specific settings tab.
    virtual rgSettingsTab* CreateSettingsTab(QWidget* pParent) override;

    // Create an OpenCL-specific status bar.
    virtual rgStatusBar* CreateStatusBar(QStatusBar* pStatusBar, QWidget* pParent) override;

    // *** Views - END ***
};

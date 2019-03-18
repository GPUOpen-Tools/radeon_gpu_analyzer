#pragma once

// Local.
#include <RadeonGPUAnalyzerGUI/Include/rgFactoryGraphics.h>

class rgFactoryVulkan : public rgFactoryGraphics
{
public:
    // Create a new Vulkan application state.
    virtual std::shared_ptr<rgAppState> CreateAppState() override;

    // Create a project instance.
    virtual std::shared_ptr<rgProject> CreateProject(const std::string& projectName, const std::string& projectFileFullPath) override;

    // Create a project clone instance.
    virtual std::shared_ptr<rgProjectClone> CreateProjectClone(const std::string& cloneName)  override;

    // Create a build settings instance.
    virtual std::shared_ptr<rgBuildSettings> CreateBuildSettings(std::shared_ptr<rgBuildSettings> pInitialBuildSettings) override;

    // *** Views - BEGIN ***

    // Create a Vulkan-specific build settings view instance.
    virtual rgBuildSettingsView* CreateBuildSettingsView(QWidget* pParent, std::shared_ptr<rgBuildSettings> pBuildSettings, bool isGlobalSettings) override;

    // Create a Vulkan-specific build view.
    virtual rgBuildView* CreateBuildView(QWidget* pParent) override;

    // Create a Vulkan disassembly view.
    virtual rgIsaDisassemblyView* CreateDisassemblyView(QWidget* pParent) override;

    // Create a Vulkan-specific file menu.
    virtual rgMenu* CreateFileMenu(QWidget* pParent) override;

    // Create a Vulkan-specific start tab.
    virtual rgStartTab* CreateStartTab(QWidget* pParent) override;

    // Create a Vulkan-specific project rename dialog box.
    rgRenameProjectDialog* CreateRenameProjectDialog(std::string& projectName, QWidget* pParent) override;

    // Create an API-specific settings tab.
    virtual rgSettingsTab* CreateSettingsTab(QWidget* pParent) override;

    // Create a Vulkan-specific status bar.
    virtual rgStatusBar* CreateStatusBar(QStatusBar* pStatusBar, QWidget* pParent) override;

    // Create a Vulkan-specific Pipeline State model instance.
    virtual rgPipelineStateModel* CreatePipelineStateModel(QWidget* pParent) override;

    // *** Views - END ***
};

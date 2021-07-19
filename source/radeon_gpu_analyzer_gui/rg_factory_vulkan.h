#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_FACTORY_VULKAN_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_FACTORY_VULKAN_H_

// Local.
#include "radeon_gpu_analyzer_gui/rg_factory_graphics.h"

class RgFactoryVulkan : public RgFactoryGraphics
{
public:
    // Create a new Vulkan application state.
    virtual std::shared_ptr<RgAppState> CreateAppState() override;

    // Create a project instance.
    virtual std::shared_ptr<RgProject> CreateProject(const std::string& project_name, const std::string& project_file_full_path) override;

    // Create a project clone instance.
    virtual std::shared_ptr<RgProjectClone> CreateProjectClone(const std::string& clone_name)  override;

    // Create a build settings instance.
    virtual std::shared_ptr<RgBuildSettings> CreateBuildSettings(std::shared_ptr<RgBuildSettings> initial_build_settings) override;

    // *** Views - BEGIN ***

    // Create a Vulkan-specific build settings view instance.
    virtual RgBuildSettingsView* CreateBuildSettingsView(QWidget* parent, std::shared_ptr<RgBuildSettings> build_settings, bool is_global_settings) override;

    // Create a Vulkan-specific build view.
    virtual RgBuildView* CreateBuildView(QWidget* parent) override;

    // Create a Vulkan disassembly view.
    virtual RgIsaDisassemblyView* CreateDisassemblyView(QWidget* parent) override;

    // Create a Vulkan-specific file menu.
    virtual RgMenu* CreateFileMenu(QWidget* parent) override;

    // Create a Vulkan-specific start tab.
    virtual RgStartTab* CreateStartTab(QWidget* parent) override;

    // Create a Vulkan-specific project rename dialog box.
    RgRenameProjectDialog* CreateRenameProjectDialog(std::string& project_name, QWidget* parent) override;

    // Create an API-specific settings tab.
    virtual RgSettingsTab* CreateSettingsTab(QWidget* parent) override;

    // Create a Vulkan-specific status bar.
    virtual RgStatusBar* CreateStatusBar(QStatusBar* status_bar, QWidget* parent) override;

    // Create a Vulkan-specific Pipeline State model instance.
    virtual RgPipelineStateModel* CreatePipelineStateModel(QWidget* parent) override;

    // *** Views - END ***
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_FACTORY_VULKAN_H_

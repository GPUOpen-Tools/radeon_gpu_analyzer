#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_FACTORY_OPENCL_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_FACTORY_OPENCL_H_

// C++.
#include <memory>

// Local.
#include "radeon_gpu_analyzer_gui/rg_factory.h"

// Forward declares.
class QWidget;
class RgBuildView;
class RgIsaDisassemblyView;

// Base factory used to create API-specific object instances.
class RgFactoryOpencl : public RgFactory
{
public:
    // Create a new OpenCL application state.
    virtual std::shared_ptr<RgAppState> CreateAppState() override;

    // Create a project instance.
    virtual std::shared_ptr<RgProject> CreateProject(const std::string& project_name, const std::string& project_file_full_path) override;

    // Create a project clone instance.
    virtual std::shared_ptr<RgProjectClone> CreateProjectClone(const std::string& clone_name) override;

    // Create a build settings instance.
    virtual std::shared_ptr<RgBuildSettings> CreateBuildSettings(std::shared_ptr<RgBuildSettings> initial_build_settings) override;

    // *** Views - BEGIN ***

    // Create an OpenCL-specific build settings view instance.
    virtual RgBuildSettingsView* CreateBuildSettingsView(QWidget* parent, std::shared_ptr<RgBuildSettings> build_settings, bool is_global_settings) override;

    // Create an OpenCL build view.
    virtual RgBuildView* CreateBuildView(QWidget* parent) override;

    // Create an OpenCL disassembly view.
    virtual RgIsaDisassemblyView* CreateDisassemblyView(QWidget* parent) override;

    // Create an OpenCL file menu.
    virtual RgMenu* CreateFileMenu(QWidget* parent) override;

    // Create an OpenCL-specific start tab.
    virtual RgStartTab* CreateStartTab(QWidget* parent) override;

    // Create an OpenCL-specific project rename dialog box.
    RgRenameProjectDialog* CreateRenameProjectDialog(std::string& project_name, QWidget* parent) override;

    // Create an API-specific settings tab.
    virtual RgSettingsTab* CreateSettingsTab(QWidget* parent) override;

    // Create an OpenCL-specific status bar.
    virtual RgStatusBar* CreateStatusBar(QStatusBar* status_bar, QWidget* parent) override;

    // *** Views - END ***
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_FACTORY_OPENCL_H_

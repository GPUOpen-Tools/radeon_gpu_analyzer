// C++.
#include <cassert>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_app_state_opencl.h"
#include "radeon_gpu_analyzer_gui/qt/rg_build_settings_view_opencl.h"
#include "radeon_gpu_analyzer_gui/qt/rg_build_view_opencl.h"
#include "radeon_gpu_analyzer_gui/qt/rg_isa_disassembly_view_opencl.h"
#include "radeon_gpu_analyzer_gui/qt/rg_menu_opencl.h"
#include "radeon_gpu_analyzer_gui/qt/rg_rename_project_dialog.h"
#include "radeon_gpu_analyzer_gui/qt/rg_settings_tab_opencl.h"
#include "radeon_gpu_analyzer_gui/qt/rg_start_tab_opencl.h"
#include "radeon_gpu_analyzer_gui/qt/rg_status_bar_opencl.h"
#include "radeon_gpu_analyzer_gui/rg_data_types_opencl.h"
#include "radeon_gpu_analyzer_gui/rg_factory_opencl.h"
#include "radeon_gpu_analyzer_gui/rg_string_constants.h"
#include "radeon_gpu_analyzer_gui/rg_utils.h"

static RgStylesheetPackage kStylesheetPackage = { kStrFileMenuStylesheetFile,
                                                    kStrFileMenuStylesheetFileOpencl,
                                                    kStrMainWindowStylesheetFile,
                                                    kStrApplicationStylesheetFileOpencl,
                                                    kStrMainWindowStylesheetFileOpencl,
                                                  };

std::shared_ptr<RgAppState> RgFactoryOpencl::CreateAppState()
{
    return std::make_shared<RgAppStateOpencl>();
}

std::shared_ptr<RgProject> RgFactoryOpencl::CreateProject(const std::string& project_name, const std::string& project_file_full_path)
{
    return std::make_shared<RgProjectOpencl>(project_name, project_file_full_path);
}

std::shared_ptr<RgProjectClone> RgFactoryOpencl::CreateProjectClone(const std::string& clone_name)
{
    // Create a copy of the global OpenCL build settings.
    std::shared_ptr<RgBuildSettings> global_cl_settings = RgConfigManager::Instance().GetUserGlobalBuildSettings(RgProjectAPI::kOpenCL);
    std::shared_ptr<RgBuildSettingsOpencl> clone_settings_copy = std::dynamic_pointer_cast<RgBuildSettingsOpencl>(CreateBuildSettings(global_cl_settings));

    // Insert the copied global settings into the new project clone.
    return std::make_shared<RgProjectCloneOpencl>(clone_name, clone_settings_copy);
}

std::shared_ptr<RgBuildSettings> RgFactoryOpencl::CreateBuildSettings(std::shared_ptr<RgBuildSettings> initial_build_settings)
{
    std::shared_ptr<RgBuildSettings> build_settings = nullptr;

    if (initial_build_settings != nullptr)
    {
        std::shared_ptr<RgBuildSettingsOpencl> build_settings_opencl = std::dynamic_pointer_cast<RgBuildSettingsOpencl>(initial_build_settings);
        build_settings = std::make_shared<RgBuildSettingsOpencl>(*build_settings_opencl);
    }
    else
    {
        build_settings = std::make_shared<RgBuildSettingsOpencl>();
    }

    return build_settings;
}

RgBuildSettingsView* RgFactoryOpencl::CreateBuildSettingsView(QWidget* parent, std::shared_ptr<RgBuildSettings> build_settings, bool is_global_settings)
{
    assert(build_settings != nullptr);
    return new RgBuildSettingsViewOpencl(parent, *std::static_pointer_cast<RgBuildSettingsOpencl>(build_settings), is_global_settings);
}

RgBuildView* RgFactoryOpencl::CreateBuildView(QWidget* parent)
{
    return new RgBuildViewOpencl(parent);
}

RgIsaDisassemblyView* RgFactoryOpencl::CreateDisassemblyView(QWidget* parent)
{
    return new RgIsaDisassemblyViewOpencl(parent);
}

RgMenu* RgFactoryOpencl::CreateFileMenu(QWidget* parent)
{
    // Create the api-specific file menu.
    RgMenu* menu = new RgMenuOpencl(parent);

    // Apply the file menu stylesheet.
    ApplyFileMenuStylesheet(menu);

    return menu;
}

void RgFactoryOpencl::ApplyFileMenuStylesheet(QWidget* widget)
{
    std::vector<std::string> stylesheet_file_names;
    stylesheet_file_names.push_back(kStylesheetPackage.file_menu_stylesheet);
    stylesheet_file_names.push_back(kStylesheetPackage.file_menu_api_stylesheet);
    [[maybe_unused]] bool status = RgUtils::LoadAndApplyStyle(stylesheet_file_names, widget);
    assert(status);
}

RgStartTab* RgFactoryOpencl::CreateStartTab(QWidget* parent)
{
    // Create the API-specific start tab.
    RgStartTab* start_tab = new RgStartTabOpencl(parent);

    return start_tab;
}

RgStatusBar* RgFactoryOpencl::CreateStatusBar(QStatusBar* status_bar, QWidget* parent)
{
    return new RgStatusBarOpencl(status_bar, parent);
}

RgSettingsTab* RgFactoryOpencl::CreateSettingsTab(QWidget* parent)
{
    return new RgSettingsTabOpencl(parent);
}

RgRenameProjectDialog* RgFactoryOpencl::CreateRenameProjectDialog(std::string& project_name, QWidget* parent)
{
    RgRenameProjectDialog* rename_dialog = new RgRenameProjectDialog(project_name, parent);
    rename_dialog->setWindowTitle(kStrRenameProjectDialogBoxTitleOpencl);

    // Center the dialog on the view (registering with the scaling manager
    // shifts it out of the center so we need to manually center it).
    RgUtils::CenterOnWidget(rename_dialog, parent);

    return rename_dialog;
}

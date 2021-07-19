// C++.
#include <cassert>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_app_state_vulkan.h"
#include "radeon_gpu_analyzer_gui/qt/rg_build_view_vulkan.h"
#include "radeon_gpu_analyzer_gui/qt/rg_isa_disassembly_view_vulkan.h"
#include "radeon_gpu_analyzer_gui/qt/rg_menu_vulkan.h"
#include "radeon_gpu_analyzer_gui/qt/rg_rename_project_dialog.h"
#include "radeon_gpu_analyzer_gui/qt/rg_settings_tab_vulkan.h"
#include "radeon_gpu_analyzer_gui/qt/rg_start_tab_vulkan.h"
#include "radeon_gpu_analyzer_gui/qt/rg_status_bar_vulkan.h"
#include "radeon_gpu_analyzer_gui/qt/rg_build_settings_view_vulkan.h"
#include "radeon_gpu_analyzer_gui/qt/rg_pipeline_state_model_vulkan.h"
#include "radeon_gpu_analyzer_gui/qt/rg_pipeline_state_view.h"
#include "radeon_gpu_analyzer_gui/rg_factory_vulkan.h"
#include "radeon_gpu_analyzer_gui/rg_data_types_vulkan.h"
#include "radeon_gpu_analyzer_gui/rg_string_constants.h"
#include "radeon_gpu_analyzer_gui/rg_utils.h"

// The Vulkan menu widget does not use API-specific styling. We specify the basic menu style file
// because the pipeline menu styles are graphics-API agnostic.
static RgStylesheetPackage kStylesheetPackage = { kStrFileMenuStylesheetFile,
                                                    kStrFileMenuStylesheetFileVulkan,
                                                    kStrMainWindowStylesheetFile,
                                                    kStrApplicationStylesheetFileVulkan,
                                                    kStrMainWindowStylesheetFileVulkan,
                                                  };

std::shared_ptr<RgAppState> RgFactoryVulkan::CreateAppState()
{
    return std::make_shared<RgAppStateVulkan>();
}

std::shared_ptr<RgProject> RgFactoryVulkan::CreateProject(const std::string& project_name, const std::string& project_file_full_path)
{
    return std::make_shared<RgProjectVulkan>(project_name, project_file_full_path);
}

std::shared_ptr<RgProjectClone> RgFactoryVulkan::CreateProjectClone(const std::string& clone_name)
{
    // Create a copy of the global Vulkan build settings.
    std::shared_ptr<RgBuildSettings> global_vulkan_settings = RgConfigManager::Instance().GetUserGlobalBuildSettings(RgProjectAPI::kVulkan);
    std::shared_ptr<RgBuildSettingsVulkan> clone_settings_copy = std::dynamic_pointer_cast<RgBuildSettingsVulkan>(CreateBuildSettings(global_vulkan_settings));

    // Insert the copied global settings into the new project clone.
    return std::make_shared<RgProjectCloneVulkan>(clone_name, clone_settings_copy);
}

std::shared_ptr<RgBuildSettings> RgFactoryVulkan::CreateBuildSettings(std::shared_ptr<RgBuildSettings> initial_build_settings)
{
    std::shared_ptr<RgBuildSettings> build_settings = nullptr;

    // Create a new rgCLBuildSettings instance based on the incoming build settings.
    if (initial_build_settings != nullptr)
    {
        auto build_settings_vulkan = std::dynamic_pointer_cast<RgBuildSettingsVulkan>(initial_build_settings);
        build_settings = std::make_shared<RgBuildSettingsVulkan>(*build_settings_vulkan);
    }
    else
    {
        build_settings = std::make_shared<RgBuildSettingsVulkan>();
    }

    return build_settings;
}

RgBuildSettingsView* RgFactoryVulkan::CreateBuildSettingsView(QWidget* parent, std::shared_ptr<RgBuildSettings> build_settings, bool is_global_settings)
{
    assert(build_settings != nullptr);
    return new RgBuildSettingsViewVulkan(parent, *std::static_pointer_cast<RgBuildSettingsVulkan>(build_settings), is_global_settings);
}

RgBuildView* RgFactoryVulkan::CreateBuildView(QWidget* parent)
{
    return new RgBuildViewVulkan(parent);
}

RgIsaDisassemblyView* RgFactoryVulkan::CreateDisassemblyView(QWidget* parent)
{
    return new RgIsaDisassemblyViewVulkan(parent);
}

RgMenu* RgFactoryVulkan::CreateFileMenu(QWidget* parent)
{
    RgMenu* menu = new RgMenuVulkan(parent);

    // Apply the file menu stylesheet.
    std::vector<std::string> stylesheet_file_names;
    stylesheet_file_names.push_back(kStylesheetPackage.file_menu_stylesheet);
    stylesheet_file_names.push_back(kStylesheetPackage.file_menu_api_stylesheet);
    bool status = RgUtils::LoadAndApplyStyle(stylesheet_file_names, menu);
    assert(status);

    return menu;
}

RgStartTab* RgFactoryVulkan::CreateStartTab(QWidget* parent)
{
    // Create the API-specific start tab.
    RgStartTab* start_tab = new RgStartTabVulkan(parent);

    return start_tab;
}

RgStatusBar* RgFactoryVulkan::CreateStatusBar(QStatusBar* status_bar, QWidget* parent)
{
    return new RgStatusBarVulkan(status_bar, parent);
}

RgPipelineStateModel* RgFactoryVulkan::CreatePipelineStateModel(QWidget* parent)
{
    return new RgPipelineStateModelVulkan(parent);
}

RgSettingsTab* RgFactoryVulkan::CreateSettingsTab(QWidget* parent)
{
    return new RgSettingsTabVulkan(parent);
}

RgRenameProjectDialog* RgFactoryVulkan::CreateRenameProjectDialog(std::string& project_name, QWidget* parent)
{
    RgRenameProjectDialog* rename_dialog = new RgRenameProjectDialog(project_name, parent);
    rename_dialog->setWindowTitle(kStrRenameProjectDialogBoxTitleVulkan);

    // Register the rename dialog with the scaling manager.
    ScalingManager::Get().RegisterObject(rename_dialog);

    // Center the dialog on the view (registering with the scaling manager
    // shifts it out of the center so we need to manually center it).
    RgUtils::CenterOnWidget(rename_dialog, parent);

    return rename_dialog;
}

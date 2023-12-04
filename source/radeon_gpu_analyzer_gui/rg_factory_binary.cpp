// C++.
#include <cassert>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_app_state_binary.h"
#include "radeon_gpu_analyzer_gui/qt/rg_build_settings_view_opencl.h"
#include "radeon_gpu_analyzer_gui/qt/rg_build_view_binary.h"
#include "radeon_gpu_analyzer_gui/qt/rg_isa_disassembly_view_binary.h"
#include "radeon_gpu_analyzer_gui/qt/rg_menu_binary.h"
#include "radeon_gpu_analyzer_gui/qt/rg_rename_project_dialog.h"
#include "radeon_gpu_analyzer_gui/qt/rg_settings_tab_binary.h"
#include "radeon_gpu_analyzer_gui/qt/rg_start_tab_binary.h"
#include "radeon_gpu_analyzer_gui/qt/rg_status_bar_binary.h"
#include "radeon_gpu_analyzer_gui/rg_data_types_binary.h"
#include "radeon_gpu_analyzer_gui/rg_factory_binary.h"
#include "radeon_gpu_analyzer_gui/rg_string_constants.h"
#include "radeon_gpu_analyzer_gui/rg_utils.h"

static RgStylesheetPackage kStylesheetPackage = {
    kStrFileMenuStylesheetFile,
    kStrFileMenuStylesheetFileBinary,
    kStrMainWindowStylesheetFile,
    kStrApplicationStylesheetFileBinary,
    kStrMainWindowStylesheetFileBinary,
};

std::shared_ptr<RgAppState> RgFactoryBinary::CreateAppState()
{
    return std::make_shared<RgAppStateBinary>();
}

std::shared_ptr<RgProject> RgFactoryBinary::CreateProject(const std::string& project_name, const std::string& project_file_full_path)
{
    return std::make_shared<RgProjectBinary>(project_name, project_file_full_path);
}

std::shared_ptr<RgProjectClone> RgFactoryBinary::CreateProjectClone(const std::string& clone_name)
{
    // Create a copy of the global Binary build settings.
    std::shared_ptr<RgBuildSettings>       global_bin_settings  = RgConfigManager::Instance().GetUserGlobalBuildSettings(RgProjectAPI::kBinary);
    std::shared_ptr<RgBuildSettingsBinary> clone_settings_copy = std::dynamic_pointer_cast<RgBuildSettingsBinary>(CreateBuildSettings(global_bin_settings));

    // Insert the copied global settings into the new project clone.
    return std::make_shared<RgProjectCloneBinary>(clone_name, clone_settings_copy);
}

std::shared_ptr<RgBuildSettings> RgFactoryBinary::CreateBuildSettings(std::shared_ptr<RgBuildSettings> initial_build_settings)
{
    std::shared_ptr<RgBuildSettings> build_settings = nullptr;

    if (initial_build_settings != nullptr)
    {
        std::shared_ptr<RgBuildSettingsBinary> build_settings_binary = std::dynamic_pointer_cast<RgBuildSettingsBinary>(initial_build_settings);
        build_settings                                               = std::make_shared<RgBuildSettingsBinary>(*build_settings_binary);
    }
    else
    {
        build_settings = std::make_shared<RgBuildSettingsBinary>();
    }

    return build_settings;
}

RgBuildSettingsView* RgFactoryBinary::CreateBuildSettingsView(QWidget* parent, std::shared_ptr<RgBuildSettings> build_settings, bool is_global_settings)
{
    return nullptr;
}

RgBuildView* RgFactoryBinary::CreateBuildView(QWidget* parent)
{
    return new RgBuildViewBinary(parent);
}

RgIsaDisassemblyView* RgFactoryBinary::CreateDisassemblyView(QWidget* parent)
{
    return new RgIsaDisassemblyViewBinary(parent);
}

RgMenu* RgFactoryBinary::CreateFileMenu(QWidget* parent)
{
    // Create the api-specific file menu.
    RgMenu* menu = new RgMenuBinary(parent);

    // Apply the file menu stylesheet.
    std::vector<std::string> stylesheet_file_names;
    stylesheet_file_names.push_back(kStylesheetPackage.file_menu_stylesheet);
    stylesheet_file_names.push_back(kStylesheetPackage.file_menu_api_stylesheet);
    bool status = RgUtils::LoadAndApplyStyle(stylesheet_file_names, menu);
    assert(status);

    return menu;
}

RgStartTab* RgFactoryBinary::CreateStartTab(QWidget* parent)
{
    // Create the API-specific start tab.
    RgStartTab* start_tab = new RgStartTabBinary(parent);

    return start_tab;
}

RgStatusBar* RgFactoryBinary::CreateStatusBar(QStatusBar* status_bar, QWidget* parent)
{
    return new RgStatusBarBinary(status_bar, parent);
}

RgSettingsTab* RgFactoryBinary::CreateSettingsTab(QWidget* parent)
{
    return new RgSettingsTabBinary(parent);
}

RgRenameProjectDialog* RgFactoryBinary::CreateRenameProjectDialog(std::string& project_name, QWidget* parent)
{
    RgRenameProjectDialog* rename_dialog = new RgRenameProjectDialog(project_name, parent);
    rename_dialog->setWindowTitle(kStrRenameProjectDialogBoxTitleBinary);

    // Register the rename dialog with the scaling manager.
    ScalingManager::Get().RegisterObject(rename_dialog);

    // Center the dialog on the view (registering with the scaling manager
    // shifts it out of the center so we need to manually center it).
    RgUtils::CenterOnWidget(rename_dialog, parent);

    return rename_dialog;
}

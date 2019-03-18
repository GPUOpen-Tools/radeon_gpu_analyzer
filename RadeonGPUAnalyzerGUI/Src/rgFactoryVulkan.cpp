// C++.
#include <cassert>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgAppStateVulkan.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgBuildViewVulkan.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgIsaDisassemblyViewVulkan.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMenuVulkan.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgRenameProjectDialog.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgSettingsTabVulkan.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgStartTabVulkan.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgStatusBarVulkan.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgBuildSettingsViewVulkan.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgPipelineStateModelVulkan.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgPipelineStateView.h>
#include <RadeonGPUAnalyzerGUI/Include/rgFactoryVulkan.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDataTypesVulkan.h>
#include <RadeonGPUAnalyzerGUI/Include/rgStringConstants.h>
#include <RadeonGPUAnalyzerGUI/Include/rgUtils.h>

// The Vulkan menu widget does not use API-specific styling. We specify the basic menu style file
// because the pipeline menu styles are graphics-API agnostic.
static rgStylesheetPackage s_STYLESHEET_PACKAGE = { STR_FILE_MENU_STYLESHEET_FILE,
                                                    STR_FILE_MENU_STYLESHEET_FILE_VULKAN,
                                                    STR_MAIN_WINDOW_STYLESHEET_FILE,
                                                    STR_APPLICATION_STYLESHEET_FILE_VULKAN,
                                                    STR_MAIN_WINDOW_STYLESHEET_FILE_VULKAN,
                                                  };

std::shared_ptr<rgAppState> rgFactoryVulkan::CreateAppState()
{
    return std::make_shared<rgAppStateVulkan>();
}

std::shared_ptr<rgProject> rgFactoryVulkan::CreateProject(const std::string& projectName, const std::string& projectFileFullPath)
{
    return std::make_shared<rgProjectVulkan>(projectName, projectFileFullPath);
}

std::shared_ptr<rgProjectClone> rgFactoryVulkan::CreateProjectClone(const std::string& cloneName)
{
    // Create a copy of the global Vulkan build settings.
    std::shared_ptr<rgBuildSettings> pGlobalVulkanSettings = rgConfigManager::Instance().GetUserGlobalBuildSettings(rgProjectAPI::Vulkan);
    std::shared_ptr<rgBuildSettingsVulkan> pCloneSettingsCopy = std::dynamic_pointer_cast<rgBuildSettingsVulkan>(CreateBuildSettings(pGlobalVulkanSettings));

    // Insert the copied global settings into the new project clone.
    return std::make_shared<rgProjectCloneVulkan>(cloneName, pCloneSettingsCopy);
}

std::shared_ptr<rgBuildSettings> rgFactoryVulkan::CreateBuildSettings(std::shared_ptr<rgBuildSettings> pInitialBuildSettings)
{
    std::shared_ptr<rgBuildSettings> pBuildSettings = nullptr;

    // Create a new rgCLBuildSettings instance based on the incoming build settings.
    if (pInitialBuildSettings != nullptr)
    {
        auto pBuildSettingsVulkan = std::dynamic_pointer_cast<rgBuildSettingsVulkan>(pInitialBuildSettings);
        pBuildSettings = std::make_shared<rgBuildSettingsVulkan>(*pBuildSettingsVulkan);
    }
    else
    {
        pBuildSettings = std::make_shared<rgBuildSettingsVulkan>();
    }

    return pBuildSettings;
}

rgBuildSettingsView* rgFactoryVulkan::CreateBuildSettingsView(QWidget* pParent, std::shared_ptr<rgBuildSettings> pBuildSettings, bool isGlobalSettings)
{
    assert(pBuildSettings != nullptr);
    return new rgBuildSettingsViewVulkan(pParent, *std::static_pointer_cast<rgBuildSettingsVulkan>(pBuildSettings), isGlobalSettings);
}

rgBuildView* rgFactoryVulkan::CreateBuildView(QWidget* pParent)
{
    return new rgBuildViewVulkan(pParent);
}

rgIsaDisassemblyView* rgFactoryVulkan::CreateDisassemblyView(QWidget* pParent)
{
    return new rgIsaDisassemblyViewVulkan(pParent);
}

rgMenu* rgFactoryVulkan::CreateFileMenu(QWidget* pParent)
{
    rgMenu* pMenu = new rgMenuVulkan(pParent);

    // Apply the file menu stylesheet.
    std::vector<std::string> stylesheetFileNames;
    stylesheetFileNames.push_back(s_STYLESHEET_PACKAGE.m_fileMenuStylesheet);
    stylesheetFileNames.push_back(s_STYLESHEET_PACKAGE.m_fileMenuApiStylesheet);
    bool status = rgUtils::LoadAndApplyStyle(stylesheetFileNames, pMenu);
    assert(status);

    return pMenu;
}

rgStartTab* rgFactoryVulkan::CreateStartTab(QWidget* pParent)
{
    // Create the API-specific start tab.
    rgStartTab* pStartTab = new rgStartTabVulkan(pParent);

    return pStartTab;
}

rgStatusBar* rgFactoryVulkan::CreateStatusBar(QStatusBar* pStatusBar, QWidget* pParent)
{
    return new rgStatusBarVulkan(pStatusBar, pParent);
}

rgPipelineStateModel* rgFactoryVulkan::CreatePipelineStateModel(QWidget* pParent)
{
    return new rgPipelineStateModelVulkan(pParent);
}

rgSettingsTab* rgFactoryVulkan::CreateSettingsTab(QWidget* pParent)
{
    return new rgSettingsTabVulkan(pParent);
}

rgRenameProjectDialog* rgFactoryVulkan::CreateRenameProjectDialog(std::string& projectName, QWidget* pParent)
{
    rgRenameProjectDialog* pRenameDialog = new rgRenameProjectDialog(projectName, pParent);
    pRenameDialog->setWindowTitle(STR_RENAME_PROJECT_DIALOG_BOX_TITLE_VULKAN);

    // Register the rename dialog with the scaling manager.
    ScalingManager::Get().RegisterObject(pRenameDialog);

    // Center the dialog on the view (registering with the scaling manager
    // shifts it out of the center so we need to manually center it).
    rgUtils::CenterOnWidget(pRenameDialog, pParent);

    return pRenameDialog;
}
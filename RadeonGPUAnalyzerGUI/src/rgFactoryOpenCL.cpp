// C++.
#include <cassert>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgAppStateOpenCL.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgBuildSettingsViewOpenCL.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgBuildViewOpenCL.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgIsaDisassemblyViewOpenCL.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMenuOpenCL.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgRenameProjectDialog.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgSettingsTabOpenCL.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgStartTabOpenCL.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgStatusBarOpenCL.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDataTypesOpenCL.h>
#include <RadeonGPUAnalyzerGUI/Include/rgFactoryOpenCL.h>
#include <RadeonGPUAnalyzerGUI/Include/rgStringConstants.h>
#include <RadeonGPUAnalyzerGUI/Include/rgUtils.h>

static rgStylesheetPackage s_STYLESHEET_PACKAGE = { STR_FILE_MENU_STYLESHEET_FILE,
                                                    STR_FILE_MENU_STYLESHEET_FILE_OPENCL,
                                                    STR_MAIN_WINDOW_STYLESHEET_FILE,
                                                    STR_APPLICATION_STYLESHEET_FILE_OPENCL,
                                                    STR_MAIN_WINDOW_STYLESHEET_FILE_OPENCL,
                                                  };

std::shared_ptr<rgAppState> rgFactoryOpenCL::CreateAppState()
{
    return std::make_shared<rgAppStateOpenCL>();
}

std::shared_ptr<rgProject> rgFactoryOpenCL::CreateProject(const std::string& projectName, const std::string& projectFileFullPath)
{
    return std::make_shared<rgProjectOpenCL>(projectName, projectFileFullPath);
}

std::shared_ptr<rgProjectClone> rgFactoryOpenCL::CreateProjectClone(const std::string& cloneName)
{
    // Create a copy of the global OpenCL build settings.
    std::shared_ptr<rgBuildSettings> pGlobalCLSettings = rgConfigManager::Instance().GetUserGlobalBuildSettings(rgProjectAPI::OpenCL);
    std::shared_ptr<rgBuildSettingsOpenCL> pCloneSettingsCopy = std::dynamic_pointer_cast<rgBuildSettingsOpenCL>(CreateBuildSettings(pGlobalCLSettings));

    // Insert the copied global settings into the new project clone.
    return std::make_shared<rgProjectCloneOpenCL>(cloneName, pCloneSettingsCopy);
}

std::shared_ptr<rgBuildSettings> rgFactoryOpenCL::CreateBuildSettings(std::shared_ptr<rgBuildSettings> pInitialBuildSettings)
{
    std::shared_ptr<rgBuildSettings> pBuildSettings = nullptr;

    if (pInitialBuildSettings != nullptr)
    {
        std::shared_ptr<rgBuildSettingsOpenCL> pBuildSettingsOpenCL = std::dynamic_pointer_cast<rgBuildSettingsOpenCL>(pInitialBuildSettings);
        pBuildSettings = std::make_shared<rgBuildSettingsOpenCL>(*pBuildSettingsOpenCL);
    }
    else
    {
        pBuildSettings = std::make_shared<rgBuildSettingsOpenCL>();
    }

    return pBuildSettings;
}

rgBuildSettingsView* rgFactoryOpenCL::CreateBuildSettingsView(QWidget* pParent, std::shared_ptr<rgBuildSettings> pBuildSettings, bool isGlobalSettings)
{
    assert(pBuildSettings != nullptr);
    return new rgBuildSettingsViewOpenCL(pParent, *std::static_pointer_cast<rgBuildSettingsOpenCL>(pBuildSettings), isGlobalSettings);
}

rgBuildView* rgFactoryOpenCL::CreateBuildView(QWidget* pParent)
{
    return new rgBuildViewOpenCL(pParent);
}

rgIsaDisassemblyView* rgFactoryOpenCL::CreateDisassemblyView(QWidget* pParent)
{
    return new rgIsaDisassemblyViewOpenCL(pParent);
}

rgMenu* rgFactoryOpenCL::CreateFileMenu(QWidget* pParent)
{
    // Create the api-specific file menu.
    rgMenu* pMenu = new rgMenuOpenCL(pParent);

    // Apply the file menu stylesheet.
    std::vector<std::string> stylesheetFileNames;
    stylesheetFileNames.push_back(s_STYLESHEET_PACKAGE.m_fileMenuStylesheet);
    stylesheetFileNames.push_back(s_STYLESHEET_PACKAGE.m_fileMenuApiStylesheet);
    bool status = rgUtils::LoadAndApplyStyle(stylesheetFileNames, pMenu);
    assert(status);

    return pMenu;
}

rgStartTab* rgFactoryOpenCL::CreateStartTab(QWidget* pParent)
{
    // Create the API-specific start tab.
    rgStartTab* pStartTab = new rgStartTabOpenCL(pParent);

    return pStartTab;
}

rgStatusBar* rgFactoryOpenCL::CreateStatusBar(QStatusBar* pStatusBar, QWidget* pParent)
{
    return new rgStatusBarOpenCL(pStatusBar, pParent);
}

rgSettingsTab* rgFactoryOpenCL::CreateSettingsTab(QWidget* pParent)
{
    return new rgSettingsTabOpenCL(pParent);
}

rgRenameProjectDialog* rgFactoryOpenCL::CreateRenameProjectDialog(std::string& projectName, QWidget* pParent)
{
    rgRenameProjectDialog* pRenameDialog = new rgRenameProjectDialog(projectName, pParent);
    pRenameDialog->setWindowTitle(STR_RENAME_PROJECT_DIALOG_BOX_TITLE_OPENCL);

    // Register the rename dialog with the scaling manager.
    ScalingManager::Get().RegisterObject(pRenameDialog);

    // Center the dialog on the view (registering with the scaling manager
    // shifts it out of the center so we need to manually center it).
    rgUtils::CenterOnWidget(pRenameDialog, pParent);

    return pRenameDialog;
}
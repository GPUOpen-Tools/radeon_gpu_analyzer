// C++.
#include <cassert>

// Local.
#include <RadeonGPUAnalyzerGUI/include/rgOpenCLFactory.h>
#include <RadeonGPUAnalyzerGUI/include/qt/rgOpenCLBuildSettingsView.h>

std::shared_ptr<rgProject> rgOpenCLFactory::CreateProject(const std::string& projectName, const std::string& projectFileFullPath) const
{
    return std::make_shared<rgCLProject>(projectName, projectFileFullPath);
}

std::shared_ptr<rgProjectClone> rgOpenCLFactory::CreateProjectClone(const std::string& cloneName) const
{
    // Create a copy of the global OpenCL build settings.
    std::shared_ptr<rgBuildSettings> pGlobalCLSettings = rgConfigManager::Instance().GetUserGlobalBuildSettings(rgProjectAPI::OpenCL);
    std::shared_ptr<rgCLBuildSettings> pCloneSettingsCopy = std::dynamic_pointer_cast<rgCLBuildSettings>(CreateBuildSettings(pGlobalCLSettings));

    // Insert the copied global settings into the new project clone.
    return std::make_shared<rgCLProjectClone>(cloneName, pCloneSettingsCopy);
}

std::shared_ptr<rgBuildSettings> rgOpenCLFactory::CreateBuildSettings(std::shared_ptr<rgBuildSettings> pInitialBuildSettings) const
{
    std::shared_ptr<rgBuildSettings> pBuildSettings = nullptr;

    // Create a new rgCLBuildSettings instance based on the incoming build settings.
    std::shared_ptr<rgCLBuildSettings> pCLBuildSettings = std::dynamic_pointer_cast<rgCLBuildSettings>(pInitialBuildSettings);
    assert(pCLBuildSettings != nullptr);
    if (pCLBuildSettings != nullptr)
    {
        pBuildSettings = std::make_shared<rgCLBuildSettings>(*pCLBuildSettings);
    }

    return pBuildSettings;
}

rgBuildSettingsView* rgOpenCLFactory::CreateBuildSettingsView(QWidget* pParent, std::shared_ptr<rgBuildSettings> pBuildSettings, bool isGlobalSettings) const
{
    return new rgOpenCLBuildSettingsView(pParent, std::static_pointer_cast<rgCLBuildSettings>(pBuildSettings), isGlobalSettings);
}
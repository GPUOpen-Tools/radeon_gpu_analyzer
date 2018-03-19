#pragma once

// C++.
#include <memory>

// Local.
#include <RadeonGPUAnalyzerGUI/include/rgFactory.h>

// Forward declares.
class QWidget;

// Base factory used to create API-specific object instances.
class rgOpenCLFactory : public rgFactory
{
public:
    // Create a project instance.
    virtual std::shared_ptr<rgProject> CreateProject(const std::string& projectName, const std::string& projectFileFullPath) const override;

    // Create a project clone instance.
    virtual std::shared_ptr<rgProjectClone> CreateProjectClone(const std::string& cloneName) const override;

    // Create a build settings instance.
    virtual std::shared_ptr<rgBuildSettings> CreateBuildSettings(std::shared_ptr<rgBuildSettings> pInitialBuildSettings) const override;

    // Create an API-specific build settings view instance.
    virtual rgBuildSettingsView* CreateBuildSettingsView(QWidget* pParent, std::shared_ptr<rgBuildSettings> pBuildSettings, bool isGlobalSettings) const override;
};
#pragma once

// Local.
#include <RadeonGPUAnalyzerGUI/include/rgDataTypes.h>

// Forward declares.
class QWidget;
class rgBuildSettingsView;

// Base factory used to create API-specific object instances.
class rgFactory
{
public:
    // Get a factory instance based on the given API.
    static std::shared_ptr<rgFactory> CreateFactory(rgProjectAPI api);

    // Create a project instance.
    virtual std::shared_ptr<rgProject> CreateProject(const std::string& projectName, const std::string& projectFileFullPath) const = 0;

    // Create a project clone instance.
    virtual std::shared_ptr<rgProjectClone> CreateProjectClone(const std::string& cloneName) const = 0;

    // Create a build settings instance.
    virtual std::shared_ptr<rgBuildSettings> CreateBuildSettings(std::shared_ptr<rgBuildSettings> pInitialBuildSettings) const = 0;

    // Create an API-specific build settings view instance.
    virtual rgBuildSettingsView* CreateBuildSettingsView(QWidget* pParent, std::shared_ptr<rgBuildSettings> pBuildSettings, bool isGlobalSettings) const = 0;
};
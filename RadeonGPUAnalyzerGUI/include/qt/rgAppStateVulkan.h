#pragma once

// C++
#include <memory>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgAppState.h>

// Forward declarations.
class  rgBuildViewVulkan;
class  rgStartTabVulkan;
class  QAction;
struct rgProject;

// A Vulkan-mode-specific implementation of the rgAppState.
class rgAppStateVulkan : public rgAppStateGraphics
{
    Q_OBJECT

public:
    rgAppStateVulkan() = default;
    virtual ~rgAppStateVulkan() = default;

    // Reset the rgBuildView instance used to display the current project.
    virtual void ResetBuildView() override;

    // Connect signals with the rgBuildView.
    virtual void ConnectBuildViewSignals(rgBuildView* pBuildView) override;

    // Get the start tab instance.
    virtual rgStartTab* GetStartTab() override;

    // Get the state's rgBuildView instance.
    virtual rgBuildView* GetBuildView() override;

    // Set the start tab instance.
    virtual void SetStartTab(rgStartTab* pStartTab) override;

    // Cleanup the state before destroying it.
    virtual void Cleanup(QMenu* pMenubar) override;

    // Update the view when a project build is started.
    virtual void HandleProjectBuildStarted() override;

    // Reset the view after a build is completed or canceled.
    virtual void ResetViewStateAfterBuild() override {}

    // Open the specified files in build view.
    void OpenFilesInBuildView(const QStringList& filePaths) override;

    // Get the application stylesheet for this app state.
    virtual void GetApplicationStylesheet(std::vector<std::string>& stylesheetFileNames) override;

    // Get the main window stylesheet for this state.
    virtual void GetMainWindowStylesheet(std::vector<std::string>& stylesheetFileNames) override;

    // Handle switching to the pipeline state view.
    virtual void HandlePipelineStateEvent() override;

    // Get the global application settings view stylesheet for this state.
    virtual std::string GetGlobalSettingsViewStylesheet() const override;

public slots:
    void HandleProjectLoaded(std::shared_ptr<rgProject>);

private slots:
    // A handler invoked when the "Create new graphics pipeline" signal is emitted.
    void HandleCreateNewGraphicsPipeline();

    // A handler invoked when the "Create new compute pipeline" signal is emitted.
    void HandleCreateNewComputePipeline();

protected:
    // Create API-specific file actions.
    virtual void CreateApiSpecificFileActions(QMenu* pMenubar) override;

    // Create the AppState's internal rgBuildView instance.
    virtual void CreateBuildView() override;

    // Connect file menu actions.
    virtual void ConnectFileMenuActions() override;

private:
    // The start tab used in OpenCL mode.
    rgStartTabVulkan* m_pStartTab = nullptr;

    // The build view used in Vulkan mode.
    rgBuildViewVulkan* m_pBuildView = nullptr;

    // An action responsible for creating a new graphics pipeline.
    QAction* m_pNewGraphicsPipelineAction = nullptr;

    // An action responsible for creating a new compute pipeline.
    QAction* m_pNewComputePipelineAction = nullptr;
};


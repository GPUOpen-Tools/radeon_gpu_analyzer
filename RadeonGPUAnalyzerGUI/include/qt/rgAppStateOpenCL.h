#pragma once

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgAppState.h>

// Forward declarations.
class rgBuildViewOpenCL;
class rgStartTabOpenCL;
class QAction;

// An OpenCL-mode-specific implementation of the rgAppState.
class rgAppStateOpenCL : public rgAppState
{
    Q_OBJECT

public:
    rgAppStateOpenCL() = default;
    virtual ~rgAppStateOpenCL() = default;

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

    // Reset the view after a build is completed or cancelled.
    virtual void ResetViewStateAfterBuild() override;

    // Open the specified files in build view.
    void OpenFilesInBuildView(const QStringList& filePaths) override;

    // Get the application stylesheet for this app state.
    virtual void GetApplicationStylesheet(std::vector<std::string>& stylesheetFileNames) override;

    // Get the main window stylesheet for this state.
    virtual void GetMainWindowStylesheet(std::vector<std::string>& stylesheetFileNames) override;

    // Get the stylesheet for the application settings.
    virtual std::string GetGlobalSettingsViewStylesheet() const override;

private slots:
    // A handler invoked when the "Create new CL file" signal is emitted.
    void HandleCreateNewCLFile();

    // A handler invoked when the "Open existing CL file" signal is emitted.
    void HandleOpenExistingCLFile();

protected:
    // Create API-specific file actions.
    virtual void CreateApiSpecificFileActions(QMenu* pMenubar) override;

    // Create the AppState's internal rgBuildView instance.
    virtual void CreateBuildView() override;

    // Connect file menu actions.
    virtual void ConnectFileMenuActions() override;

private:
    // The start tab used in OpenCL mode.
    rgStartTabOpenCL* m_pStartTab = nullptr;

    // The build view used in Vulkan mode.
    rgBuildViewOpenCL* m_pBuildView = nullptr;

    // An action responsible for creating a new CL source file.
    QAction* m_pNewFileAction = nullptr;

    // The action used to open a source file.
    QAction* m_pOpenFileAction = nullptr;
};
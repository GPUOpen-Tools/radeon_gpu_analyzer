#pragma once

// Qt.
#include <QObject>
#include <QStatusBar>

// Forward declarations.
class QAction;
class QMenu;
class rgBuildView;
class rgMainWindow;
class rgSettingsTab;
class rgStartTab;

// The rgAppState object allows a project with a specific mode to interface with the main window.
class rgAppState : public QObject
{
    Q_OBJECT

public:
    rgAppState() = default;
    virtual ~rgAppState() = default;

    // Reset the rgBuildView instance used to display the current project.
    virtual void ResetBuildView() = 0;

    // Connect signals with the rgBuildView.
    virtual void ConnectBuildViewSignals(rgBuildView* pBuildView) = 0;

    // Get the start tab instance.
    virtual rgStartTab* GetStartTab() = 0;

    // Get the state's API-specific rgBuildView instance.
    virtual rgBuildView* GetBuildView() = 0;

    // Set the start tab instance.
    virtual void SetStartTab(rgStartTab* pStartTab) = 0;

    // Cleanup the state before destroying it.
    virtual void Cleanup(QMenu* pMenubar) = 0;

    // Update the view when a project build is started.
    virtual void HandleProjectBuildStarted() = 0;

    // Reset the view after a build is completed or canceled.
    virtual void ResetViewStateAfterBuild() = 0;

    // Create all file actions for the state.
    void CreateFileActions(QMenu* pMenubar);

    // Set the main window instance.
    void SetMainWindow(rgMainWindow* pMainWindow);

    // Set the settings tab widget.
    void SetSettingsTab(rgSettingsTab* pSettingsTab);

    // Get the application stylesheet for this state.
    virtual void GetApplicationStylesheet(std::vector<std::string>& stylesheetFileNames) = 0;

    // Get the main window stylesheet for this state.
    virtual void GetMainWindowStylesheet(std::vector<std::string>& stylesheetFileNames) = 0;

    // Get the global application settings view stylesheet for this state.
    virtual std::string GetGlobalSettingsViewStylesheet() const = 0;

    // Open the specified files in build view.
    virtual void OpenFilesInBuildView(const QStringList& filePaths) = 0;

    // Displays a prompt to save any modified settings or files.
    // Returns true if there were no modified files, or if the user made a decision about saving the files.
    // Returns false if the user clicked 'cancel', in which case any in-progress actions should also be canceled.
    bool ShowProjectSaveDialog();

    // Returns true if the app state is of a graphics API type, false otherwise.
    bool IsGraphics() const;

protected:
    // Create API-specific file actions, and add each new action to the top of the File menu.
    virtual void CreateApiSpecificFileActions(QMenu* pMenubar) = 0;

    // Create the AppState's internal rgBuildView instance.
    virtual void CreateBuildView() = 0;

    // Connect file menu actions.
    virtual void ConnectFileMenuActions() = 0;

    // The main window for the application.
    rgMainWindow* m_pMainWindow = nullptr;

    // The settings tab widget.
    rgSettingsTab* m_pSettingsTab = nullptr;

    // A flag indicating whether the state is graphics or not.
    bool m_isGraphics = false;
};

// The rgGraphicsAppState object allows a graphics project to interface with the main window.
class rgAppStateGraphics : public rgAppState
{
    Q_OBJECT

public:
    rgAppStateGraphics();
    virtual ~rgAppStateGraphics() = default;

    // Handle switching to the pipeline state editor.
    virtual void HandlePipelineStateEvent() = 0;
};
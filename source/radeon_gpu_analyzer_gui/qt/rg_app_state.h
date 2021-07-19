#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_APP_STATE_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_APP_STATE_H_

// Qt.
#include <QObject>
#include <QStatusBar>

// Forward declarations.
class QAction;
class QMenu;
class RgBuildView;
class RgMainWindow;
class RgSettingsTab;
class RgStartTab;

// The RgAppState object allows a project with a specific mode to interface with the main window.
class RgAppState : public QObject
{
    Q_OBJECT

public:
    RgAppState() = default;
    virtual ~RgAppState() = default;

    // Reset the RgBuildView instance used to display the current project.
    virtual void ResetBuildView() = 0;

    // Connect signals with the RgBuildView.
    virtual void ConnectBuildViewSignals(RgBuildView* build_view) = 0;

    // Get the start tab instance.
    virtual RgStartTab* GetStartTab() = 0;

    // Get the state's API-specific RgBuildView instance.
    virtual RgBuildView* GetBuildView() = 0;

    // Set the start tab instance.
    virtual void SetStartTab(RgStartTab* start_tab) = 0;

    // Cleanup the state before destroying it.
    virtual void Cleanup(QMenu* menu_bar) = 0;

    // Update the view when a project build is started.
    virtual void HandleProjectBuildStarted() = 0;

    // Reset the view after a build is completed or canceled.
    virtual void ResetViewStateAfterBuild() = 0;

    // Create all file actions for the state.
    void CreateFileActions(QMenu* menu_bar);

    // Set the main window instance.
    void SetMainWindow(RgMainWindow* main_window);

    // Set the settings tab widget.
    void SetSettingsTab(RgSettingsTab* settings_tab);

    // Get the application stylesheet for this state.
    virtual void GetApplicationStylesheet(std::vector<std::string>& stylesheet_file_names) = 0;

    // Get the main window stylesheet for this state.
    virtual void GetMainWindowStylesheet(std::vector<std::string>& stylesheet_file_names) = 0;

    // Get the global application settings view stylesheet for this state.
    virtual std::string GetGlobalSettingsViewStylesheet() const = 0;

    // Get the build application settings view stylesheet for this state.
    virtual std::string GetBuildSettingsViewStylesheet() const = 0;

    // Open the specified files in build view.
    virtual void OpenFilesInBuildView(const QStringList& file_paths) = 0;

    // Displays a prompt to save any modified settings or files.
    // Returns true if there were no modified files, or if the user made a decision about saving the files.
    // Returns false if the user clicked 'cancel', in which case any in-progress actions should also be canceled.
    bool ShowProjectSaveDialog();

    // Returns true if the app state is of a graphics API type, false otherwise.
    bool IsGraphics() const;

    // Check to see if the input file names in global build settings are blank.
    bool IsInputFileNameBlank() const;

signals:
    // A signal to indicate if the pipeline state menu option should be enabled.
    void EnablePipelineMenuItem(bool is_enabled);

    // A signal to indicate if the build settings menu option should be enabled.
    void EnableBuildSettingsMenuItem(bool is_enabled);

    // A signal to indicate if the Ctrl+S menu option should be enabled.
    void EnableSaveSettingsMenuItem(bool is_enabled);

protected:
    // Create API-specific file actions, and add each new action to the top of the File menu.
    virtual void CreateApiSpecificFileActions(QMenu* menu_bar) = 0;

    // Create the AppState's internal RgBuildView instance.
    virtual void CreateBuildView() = 0;

    // Connect file menu actions.
    virtual void ConnectFileMenuActions() = 0;

    // The main window for the application.
    RgMainWindow* main_window_ = nullptr;

    // The settings tab widget.
    RgSettingsTab* settings_tab_ = nullptr;

    // A flag indicating whether the state is graphics or not.
    bool is_graphics_ = false;
};

// The RgGraphicsAppState object allows a graphics project to interface with the main window.
class RgAppStateGraphics : public RgAppState
{
    Q_OBJECT

public:
    RgAppStateGraphics();
    virtual ~RgAppStateGraphics() = default;

    // Handle switching to the pipeline state editor.
    virtual void HandlePipelineStateEvent() = 0;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_APP_STATE_H_

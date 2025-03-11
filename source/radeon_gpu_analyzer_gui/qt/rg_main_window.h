#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_MAIN_WINDOW_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_MAIN_WINDOW_H_

// C++.
#include <memory>

// Qt.
#include <QtWidgets/QMainWindow>
#include <QTimer>

// Local.
#include "source/radeon_gpu_analyzer_gui/qt/rg_build_view.h"
#include "ui_rg_main_window.h"

// Forward declarations.
class QAction;
class QButtonGroup;
class QMenu;
class QScrollArea;
class RgAppState;
class RgBuildView;
class RgMenu;
class RgGoToLineDialog;
class RgSettingsTab;
class RgStatusBar;
struct RgProject;

// The application's main window instance.
class RgMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    RgMainWindow(QWidget* parent = Q_NULLPTR);
    virtual ~RgMainWindow() = default;

    // An enum containing the main views for the application.
    enum class MainWindowView
    {
        kHome,
        kBuildView
    };

    // Destroy the RgBuildView instance.
    void DestroyBuildView();

    // Initialize the API-specific components.
    // The reason why we are using this init function and not a CTOR is that we need to initialize
    // some API-specific components which are defined in classes that derive from RgMainWindow. Since
    // these functions are virtual, we cannot call them from the CTOR.
    void InitMainWindow();

    // Resets the actions to the default state (where the app started and home page is visible).
    void ResetActionsState();

    // Switch to a new view.
    void SwitchToView(MainWindowView tab);

    // Use an event filter to hide the API mode buttons.
    virtual bool eventFilter(QObject* obj, QEvent* event) override;

    // Method to load a binary code object file, directly via the command line.
    bool LoadBinaryCodeObject(const QString filename);

signals:
    // Signal emitted when the user triggers the Edit menu's Find action.
    void FindTriggered();

    // Signal emitted when the user triggers the Edit menu's Go To Line action.
    void GoToLineTriggered();

    // A signal emitted when the project build status changes.
    void IsBuildInProgress(bool is_building);

    // A signal to switch current container's size.
    void SwitchContainerSize();

    // A signal to indicate change of view.
    void HotKeyPressedSignal();

    // A signal emitted when the user clicks on "Highlight lines with maximum live VGPRs" menu item.
    void ShowMaximumVgprClickedSignal();

    // A signal to enable/disable the Edit->Go to next maximum live VGPR line option.
    void EnableShowMaxVgprOptionSignal(bool is_enabled);

    // *** TEST SIGNALS - BEGIN ***

    // A signal emitted when a project is loaded.
    void TEST_ProjectLoaded();

    // A signal emitted when the MainWindow has been created and initialized BuildView object.
    void TEST_BuildViewCreated(void* build_view);

    // *** TEST SIGNALS - END ***

public slots:
    // Handler for when a new project has been created.
    void HandleProjectCreated();

    // Handler for when the pending changes need to be saved.
    // Returns true when the user selected Yes/No from the confirmation
    // dialog, returns false when the user selected Cancel.
    bool HandleSavePendingChanges();

    // A handler to update the Edit->Go to next maximum live VGPR line option.
    void HandleEnableShowMaxVgprOptionSignal(bool is_enable);

protected:
    // The save action type.
    enum SaveActionType
    {
        // Save the current source file being edited.
        kSaveFile,

        // Save the current build or pipeline settings being edited.
        kSaveSettings
    };

    // Create the QAction objects.
    virtual void CreateFileMenuActions();

    // Add the build view interface to the MainWindow.
    bool AddBuildView();

    // Create the application state.
    void CreateAppState(RgProjectAPI api);

    // Switch the current API mode.
    void ChangeApiMode(RgProjectAPI api);

    // Connect the signals.
    void ConnectSignals();

    // Create the settings tab.
    void CreateSettingsTab();

    // Create the main window's start tab.
    void CreateStartTab();

    // Connect the signals for the current mode's start tab.
    void ConnectStartTabSignals();

    // Connect the signals for the BuildView interface.
    void ConnectBuildViewSignals();

    // Connect the menu signals to the RgBuildView and RgMainWindow.
    void ConnectMenuSignals();

    // Create the actions to be used by the Edit menu.
    void CreateEditMenuActions();

    // Create the actions to be used by the Build menu.
    void CreateBuildMenuActions();

    // Create the main window's menu bar.
    void CreateMenuBar();

    // Create the File menu items.
    void CreateFileMenu();

    // Create the Edit menu items.
    void CreateEditMenu();

    // Create the Help menu items.
    void CreateHelpmenu();

    // Create the Help menu actions.
    void CreateHelpmenuActions();

    // Create the Build menu items.
    void CreateBuildMenu();

    // Destroy the current file menu.
    void DestroyFileMenu();

    // Delete the existing start tab.
    void DestroyStartTab();

    // Delete the existing settings tab.
    void DestroySettingsTab();

    // Toggle the enabledness of the Build menu items.
    void EnableBuildMenu(bool is_enabled);

    // Toggle the enabledness of the Edit menu items.
    void EnableEditMenu(bool is_enabled);

    // Check to see if any input file names in  global settings are blank.
    bool IsInputFileNameBlank() const;

    // Open the program file at the given file path.
    bool OpenProjectFileAtPath(const std::string& program_file_path);

    // Set the main window title text.
    void SetWindowTitle(const std::string& window_title);

    // Set the application stylesheet.
    void SetApplicationStylesheet();

    // Clear the window title.
    void ResetWindowTitle();

    // Actions for the menus - START.

    QAction* focus_next_widget_action_ = nullptr;
    QAction* focus_prev_widget_action_ = nullptr;
    QAction* open_project_action_ = nullptr;
    QAction* save_action_ = nullptr;
    QAction* back_to_home_action_ = nullptr;
    QAction* build_project_action_ = nullptr;
    QAction* build_settings_action_ = nullptr;
    QAction* pipeline_state_action_ = nullptr;
    QAction* help_about_action_ = nullptr;
    QAction* exit_action_ = nullptr;
    QAction* go_to_line_action_ = nullptr;
    QAction* find_action_ = nullptr;
    QAction* show_max_vgprs_action_ = nullptr;
    QAction* cancel_build_action_ = nullptr;
    QAction* help_getting_started_guide_action_ = nullptr;
    QAction* help_manul_action_ = nullptr;

    // Actions for the menus - END.

    // The menu bar.
    QMenu* menu_bar_ = nullptr;

    // The custom status bar.
    RgStatusBar* status_bar_ = nullptr;

    // The app notification message widget.
    QWidget* app_notification_widget_ = nullptr;

    // Timer to synchronize the blinking of the notification.
    QTimer* app_notification_blinking_timer_ = new QTimer();

    Ui::RgMainWindow ui_;

    // The settings tab.
    RgSettingsTab* settings_tab_ = nullptr;

    // The save file action shortcut current state.
    bool save_file_action_active_ = false;

    // The save settings action shortcut current state.
    bool save_settings_action_active_ = false;

    // A pointer to the factory through which to create API-specific widgets.
    std::shared_ptr<RgFactory> factory_ = nullptr;

    // A pointer to the application state.
    std::shared_ptr<RgAppState> app_state_ = nullptr;

    // Save the current view.
    MainWindowView current_view_ = MainWindowView::kHome;

    // Window close event override.
    virtual void closeEvent(QCloseEvent* event) override;

    // Window file drag enter event.
    virtual void dragEnterEvent(QDragEnterEvent* event) override;

    // Window file drop event.
    virtual void dropEvent(QDropEvent* event) override;

    // Set the cursor to pointing hand cursor for various widgets.
    void SetCursor();

    // Switch the save shortcut between file and settings.
    void SwitchSaveShortcut(SaveActionType save_action_type);

    // Adjusts the actions state to the build view's initial state.
    void EnableBuildViewActions();

    // Create a custom status bar with Mode button to replace the default one.
    void CreateCustomStatusBar();

    // Apply main window stylesheet.
    void ApplyMainWindowStylesheet();

    // Create the app notification message label.
    void CreateAppNotificationMessageLabel(const std::string& message, const std::string& tooltip);

protected slots:
    // Handler for when the settings tab has pending changes.
    void HandleHasSettingsPendingStateChanged(bool has_pending_changes);

    // Handler for when the number of items in the project changed.
    void HandleProjectFileCountChanged(bool is_project_empty);

    // Handler for the build view edit mode changed signal.
    void HandleEditModeChanged(EditMode mode);

    // Handler for the show max VGPRs event.
    void HandleShowMaxVgprsEvent();

    // Handler for the main tab widget's tab change.
    void HandleMainTabWidgetTabChanged(int current_index);

    // Handler for the Open Program button click.
    void HandleOpenProjectFileEvent();

    // Open the program file at the given file path.
    void HandleOpenProjectFileAtPath(const std::string& program_file_path);

    // Handler for the Save File button click.
    void HandleSaveFileEvent();

    // Handler for the back to home menu_bar option.
    void HandleBackToHomeEvent();

    // Handler for the Exit button click.
    void HandleExitEvent();

    // Handler for a change in the editor's "modified" state.
    // This handler is fired whenever the source code editor changes (a text edit, undo, etc.).
    void OnCurrentEditorModificationStateChanged(bool is_modified);

    // Handler invoked when the Help->About item is clicked.
    void HandleAboutEvent();

    // Handler invoked when the Help->Getting started guide item is clicked.
    void HandleGettingStartedGuideEvent();

    // Handler invoked when the Help->Help manual item is clicked.
    void HandleHelpManual();

    // Handler for building the current program.
    void HandleBuildProjectEvent();

    // Handler for viewing the program build settings.
    void HandleBuildSettingsEvent();

    // Handler for viewing the pipeline state.
    void HandlePipelineStateEvent();

    // Handler for canceling the current build.
    void HandleCancelBuildEvent();

    // Handler for when the selected file item is changed in the file menu.
    void HandleSelectedFileChanged(const std::string& old_file, const std::string& new_file);

    // Handler for when a program is loaded within the RgBuildView.
    void HandleProjectLoaded(std::shared_ptr<RgProject> program);

    // Handler for when the current file has been modified outside the environment.
    void HandleCurrentFileModifiedOutsideEnv();

    // Handler for when the pipeline state menu item needs to be enabled.
    void HandleEnablePipelineMenuItem(bool is_enabled);

    // Handler for when the build settings menu item needs to be enabled.
    void HandleEnableBuildSettingsMenuItem(bool is_enabled);

    // Handler for when the save settings menu item needs to be enabled.
    void HandleEnableSaveSettingsMenuItem(bool is_enabled);

    // Handler for when the shortcut to switch widget focus to next widget is pressed.
    void HandleFocusNextWidget();

    // Handler for when the shortcut to switch widget focus to previous widget is pressed.
    void HandleFocusPrevWidget();

    // Handler for when the program build fails.
    void HandleProjectBuildFailure();

    // Handler for when the project build is canceled.
    void HandleProjectBuildCanceled();

    // Resets the view's state after a build process.
    void ResetViewStateAfterBuild();

    // Handler for when the program build succeeds.
    void HandleProjectBuildSuccess();

    // Handler for when the program build starts.
    void HandleProjectBuildStarted();

    // Handler invoked when the status bar text should be changed.
    void HandleStatusBarTextChanged(const std::string& status_bar_text, int timeout_ms);

    // Handler for when the "Save" button is clicked.
    void HandleSaveSettingsButtonClicked();

    // Handler for when the tab bar tab changes.
    void HandleTabBarTabChanged(bool save_changes);

    // Handler for when the text of the status bar is changed.
    void HandleStatusBarMessageChange(const QString&  msg);

    // Handler for when the custom status bar signals API change by user.
    void HandleChangeAPIMode(RgProjectAPI switch_to_api);

    // Handler to update the app notification message widget.
    void HandleUpdateAppNotificationMessage(const std::string& message, const std::string& tooltip);

    // Handler for when the blinking notification message timer fires.
    void HandleAppNotificationMessageTimerFired();
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_MAIN_WINDOW_H_

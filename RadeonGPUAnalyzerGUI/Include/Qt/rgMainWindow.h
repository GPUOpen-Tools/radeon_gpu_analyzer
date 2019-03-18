#pragma once

// C++.
#include <memory>

// Qt.
#include <QtWidgets/QMainWindow>
#include <QTimer>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgBuildView.h>
#include <ui_rgMainWindow.h>

// Forward declarations.
class QAction;
class QButtonGroup;
class QMenu;
class QScrollArea;
class rgAppState;
class rgBuildView;
class rgMenu;
class rgGoToLineDialog;
class rgSettingsTab;
class rgStatusBar;
struct rgProject;

// The application's main window instance.
class rgMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    rgMainWindow(QWidget* pParent = Q_NULLPTR);
    virtual ~rgMainWindow() = default;

    // An enum containing the main views for the application.
    enum class MainWindowView
    {
        Home,
        BuildView
    };

    // Destroy the rgBuildView instance.
    void DestroyBuildView();

    // Initialize the API-specific components.
    // The reason why we are using this init function and not a CTOR is that we need to initialize
    // some API-specific components which are defined in classes that derive from rgMainWindow. Since
    // these functions are virtual, we cannot call them from the CTOR.
    void InitMainWindow();

    // Resets the actions to the default state (where the app started and home page is visible).
    void ResetActionsState();

    // Switch to a new view.
    void SwitchToView(MainWindowView tab);

    // Use an event filter to hide the API mode buttons.
    virtual bool eventFilter(QObject* obj, QEvent* event) override;

signals:
    // Signal emitted when the user triggers the Edit menu's Find action.
    void FindTriggered();

    // A signal emitted when the project build status changes.
    void IsBuildInProgress(bool isBuilding);

    // A signal to switch current container's size.
    void SwitchContainerSize();

    // *** TEST SIGNALS - BEGIN ***

    // A signal emitted when a project is loaded.
    void TEST_ProjectLoaded();

    // A signal emitted when the MainWindow has been created and initialized BuildView object.
    void TEST_BuildViewCreated(void* pBuildView);

    // *** TEST SIGNALS - END ***

public slots:
    // Handler for when a new project has been created.
    void HandleProjectCreated();

protected:
    // The save action type.
    enum SaveActionType
    {
        // Save the current source file being edited.
        SaveFile,

        // Save the current build or pipeline settings being edited.
        SaveSettings
    };

    // Create the QAction objects.
    virtual void CreateFileMenuActions();

    // Add the build view interface to the MainWindow.
    void AddBuildView();

    // Create the application state.
    void CreateAppState(rgProjectAPI api);

    // Switch the current API mode.
    void ChangeApiMode(rgProjectAPI api);

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

    // Connect the menu signals to the rgBuildView and rgMainWindow.
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
    void CreateHelpMenu();

    // Create the Help menu actions.
    void CreateHelpMenuActions();

    // Create the Build menu items.
    void CreateBuildMenu();

    // Destroy the current file menu.
    void DestroyFileMenu();

    // Delete the existing start tab.
    void DestroyStartTab();

    // Delete the existing settings tab.
    void DestroySettingsTab();

    // Toggle the enabledness of the Build menu items.
    void EnableBuildMenu(bool isEnabled);

    // Toggle the enabledness of the Edit menu items.
    void EnableEditMenu(bool isEnabled);

    // Open the program file at the given file path.
    bool OpenProjectFileAtPath(const std::string& programFilePath);

    // Set the main window title text.
    void SetWindowTitle(const std::string& windowTitle);

    // Set the application stylesheet.
    void SetApplicationStylesheet();

    // Clear the window title.
    void ResetWindowTitle();

    // Actions for the menus - START.

    QAction* m_pFocusNextWidgetAction = nullptr;
    QAction* m_pFocusPrevWidgetAction = nullptr;
    QAction* m_pOpenProjectAction = nullptr;
    QAction* m_pSaveAction = nullptr;
    QAction* m_pBackToHomeAction = nullptr;
    QAction* m_pBuildProjectAction = nullptr;
    QAction* m_pBuildSettingsAction = nullptr;
    QAction* m_pPipelineStateAction = nullptr;
    QAction* m_pHelpAboutAction = nullptr;
    QAction* m_pExitAction = nullptr;
    QAction* m_pGoToLineAction = nullptr;
    QAction* m_pFindAction = nullptr;
    QAction* m_pCancelBuildAction = nullptr;
    QAction* m_pHelpGettingStartedGuideAction = nullptr;
    QAction* m_pHelpManualAction = nullptr;

    // Actions for the menus - END.

    // The menu bar.
    QMenu* m_pMenuBar = nullptr;

    // The custom status bar.
    rgStatusBar* m_pStatusBar = nullptr;

    // The app notification message widget.
    QWidget* m_pAppNotificationWidget = nullptr;

    // Timer to synchronize the blinking of the notification.
    QTimer* m_pAppNotificationBlinkingTimer = new QTimer();

    Ui::rgMainWindow ui;

    // The settings tab.
    rgSettingsTab* m_pSettingsTab = nullptr;

    // The save file action shortcut current state.
    bool m_saveFileActionActive = false;

    // The save settings action shortcut current state.
    bool m_saveSettingsActionActive = false;

    // A pointer to the factory through which to create API-specific widgets.
    std::shared_ptr<rgFactory> m_pFactory = nullptr;

    // A pointer to the application state.
    std::shared_ptr<rgAppState> m_pAppState = nullptr;

    // Save the current view.
    MainWindowView m_currentView = MainWindowView::Home;

    // Window close event override.
    virtual void closeEvent(QCloseEvent* pEvent) override;

    // Window file drag enter event.
    virtual void dragEnterEvent(QDragEnterEvent* pEvent) override;

    // Window file drop event.
    virtual void dropEvent(QDropEvent* pEvent) override;

    // Set the cursor to pointing hand cursor for various widgets.
    void SetCursor();

    // Switch the save shortcut between file and settings.
    void SwitchSaveShortcut(SaveActionType saveActionType);

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
    void HandleHasSettingsPendingStateChanged(bool hasPendingChanges);

    // Handler for when the number of items in the project changed.
    void HandleProjectFileCountChanged(bool isProjectEmpty);

    // Handler for the build view edit mode changed signal.
    void HandleEditModeChanged(EditMode mode);

    // Handler for the Go To line button click.
    void HandleGoToLineEvent();

    // Handler for the main tab widget's tab change.
    void HandleMainTabWidgetTabChanged(int currentIndex);

    // Handler for the Open Program button click.
    void HandleOpenProjectFileEvent();

    // Open the program file at the given file path.
    void HandleOpenProjectFileAtPath(const std::string& programFilePath);

    // Handler for the Save File button click.
    void HandleSaveFileEvent();

    // Handler for the back to home menubar option.
    void HandleBackToHomeEvent();

    // Handler for the Exit button click.
    void HandleExitEvent();

    // Handler for a change in the editor's "modified" state.
    // This handler is fired whenever the source code editor changes (a text edit, undo, etc.).
    void OnCurrentEditorModificationStateChanged(bool isModified);

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
    void HandleSelectedFileChanged(const std::string& oldFile, const std::string& newFile);

    // Handler for when a program is loaded within the rgBuildView.
    void HandleProjectLoaded(std::shared_ptr<rgProject> pProgram);

    // Handler for when the current file has been modified outside the environment.
    void HandleCurrentFileModifiedOutsideEnv();

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
    void HandleStatusBarTextChanged(const std::string& statusBarText, int timeoutMs);

    // Handler for when the "Save" button is clicked.
    void HandleSaveSettingsButtonClicked();

    // Handler for when the tab bar tab changes.
    void HandleTabBarTabChanged(bool saveChanges);

    // Handler for when the text of the status bar is changed.
    void HandleStatusBarMessageChange(const QString&  msg);

    // Handler for when the custom status bar signals API change by user.
    void HandleChangeAPIMode(rgProjectAPI switchToApi);

    // Handler to update the app notification message widget.
    void HandleUpdateAppNotificationMessage(const std::string& message, const std::string& tooltip);

    // Handler for when the blinking notification message timer fires.
    void HandleAppNotificationMessageTimerFired();
};

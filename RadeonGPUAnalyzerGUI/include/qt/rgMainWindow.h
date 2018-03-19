#pragma once

// C++.
#include <memory>

#include <QtWidgets/QMainWindow>
#include "ui_rgMainWindow.h"
#include <RadeonGPUAnalyzerGUI/include/qt/rgBuildView.h>

// Forward declarations.
class QAction;
class QButtonGroup;
class QMenu;
class QScrollArea;
class rgBuildView;
class rgFileMenu;
class rgGoToLineDialog;
class rgGlobalSettingsView;
class rgSettingsButtonsView;
class rgBuildSettingsView;
struct rgProject;
struct rgBuildSettings;

class rgMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    rgMainWindow(QWidget* pParent = Q_NULLPTR);

protected:
    // Re-implement eventFilter to handle up/down arrow keys.
    virtual bool eventFilter(QObject* pObject, QEvent* pEvent);

signals:
    // Signal emitted when the user triggers the Edit menu's Find action.
    void FindTriggered();

    // A signal emitted when the project build status changes.
    void IsBuildInProgress(bool isBuilding);

private:
    // Apply any string constants where needed.
    void ApplyStringConstants();

    // Connect the signals.
    void ConnectSignals();

    // Create the build view interface.
    void CreateBuildView();

    // Create the global build settings view.
    void CreateGlobalBuildSettingsView();

    // Connect the signals for the BuildView interface.
    void ConnectBuildViewSignals();

    // Connect the signals for settings tab list widget.
    void ConnectSettingsListWidgetSignals();

    // Create the actions to be used by the File menu.
    void CreateFileMenuActions();

    // Create the actions to be used by the Edit menu.
    void CreateEditMenuActions();

    // Create the actions to be used by the Build menu.
    void CreateBuildMenuActions();

    // Create the File menu items.
    void CreateFileMenu();

    // Create the Edit menu items.
    void CreateEditMenu();

    // Create the Help menu items.
    void CreateHelpMenu();

    // Create the conetxt menu items.
    void CreateContextMenu();

    // Create the Help menu actions.
    void CreateHelpMenuActions();

    // Create the Build menu items.
    void CreateBuildMenu();

    // Destroy the rgBuildView instance.
    void DestroyBuildView();

    // Toggle the enabledness of the Build menu items.
    void EnableBuildMenu(bool isEnabled);

    // Toggle the enabledness of the Edit menu items.
    void EnableEditMenu(bool isEnabled);

    // Ignore up/down key presses for top and bottom buttons.
    bool ProcessKeyPress(QKeyEvent* pKeyEvent, const QString& objectName);

    // Open the program file at the given file path.
    bool OpenProjectFileAtPath(const std::string& programFilePath);

    // Set the main window title text.
    void SetWindowTitle(const std::string& windowTitle);

    // Clear the window title.
    void ResetWindowTitle();

    // Switch to a new view.
    void SwitchToView(QWidget* pWidget);

    // Open the given files in the build view.
    void OpenFilesInBuildView(const QStringList& filePaths);

    // Reorder the tab order to allow for the recent projects list additions/removals.
    void ReorderTabOrder(QLayout* pLayout);

    // Actions for the menus - START.

    QAction* m_pFocusNextWidgetAction = nullptr;
    QAction* m_pFocusPrevWidgetAction = nullptr;
    QAction* m_pNewFileAction = nullptr;
    QAction* m_pOpenFileAction = nullptr;
    QAction* m_pOpenProjectAction = nullptr;
    QAction* m_pOpenRecentAction = nullptr;
    QAction* m_pOpenContainingFolderAction = nullptr;
    QAction* m_pSaveAction = nullptr;
    QAction* m_pBackToHomeAction = nullptr;
    QAction* m_pBuildProjectAction = nullptr;
    QAction* m_pBuildSettingsAction = nullptr;
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

    // The build view.
    rgBuildView* m_pBuildView = nullptr;

    // A button group used to handle the array of recent program buttons.
    QButtonGroup* m_pRecentProjectButtonGroup = nullptr;

    Ui::rgMainWindow ui;

    // The context menu for recent files
    QMenu m_menu;

    // The global settings view.
    rgGlobalSettingsView* m_pGlobalSettingsView = nullptr;

    // The settings buttons view.
    rgSettingsButtonsView* m_pSettingsButtonsView = nullptr;

    // The build settings view.
    rgBuildSettingsView* m_pBuildSettingsView = nullptr;

    // The save file action shortcut current state.
    bool m_saveFileActionActive = false;

    // The save settings action shortcut current state.
    bool m_saveSettingsActionActive = false;

    // A boolean to indicate a special resolution case.
    bool m_isSpecialResolution = false;

    // The scroll area for settings.
    QFrame* m_pScrollAreaWidgetContents;

private:
    // The entries in the setting list widget.
    enum SettingsListWidgetEntries
    {
        Global,
        OpenCL
    };

    // The save action type.
    enum SaveActionType
    {
        SaveFile,
        SaveBuildSettings
    };

    // Clear the list of recent program buttons.
    void ClearRecentProjectsList();

    // Get the currently-visible settings category.
    SettingsListWidgetEntries GetSelectedSettingCategory() const;

    // Populate the list of recent files in the welcome page.
    void PopulateRecentProjectsList();

    // Window close event override.
    virtual void closeEvent(QCloseEvent* pEvent) override;

    // Window file drop event.
    virtual void dropEvent(QDropEvent* pEvent) override;

    // Window file drag enter event.
    virtual void dragEnterEvent(QDragEnterEvent* pEvent) override;

    // Set the cursor to pointing hand cursor for various widgets.
    void SetCursor();

    // Switch the save shortcut between file and settings.
    void SwitchSaveShortcut(SaveActionType saveActionType);

    // Set build settings geometry according to the display dpi.
    void SetBuildSettingsGeometry();

    // Resets the actions to the default state (where the app started and home page is visible).
    void ResetActionsState();

    // Adjusts the actions state to the build view's initial state.
    void EnableBuildViewActions();

private slots:
    // Handler for the settings list widget.
    void HandleSettingsListWidgetClick(int index);

    // Handler for when the number of items in the project changed.
    void HandleProjectFileCountChanged(bool isProjectEmpty);

    // Handler for the Create New File button click.
    void HandleCreateNewFileEvent();

    // Handler for the build view edit mode changed signal.
    void HandleEditModeChanged(EditMode mode);

    // Handler for the Go To line button click.
    void HandleGoToLineEvent();

    // Handler for the main tab widget's tab change.
    void HandleMainTabWidgetTabChanged(int currentIndex);

    // Handler for the Open Existing File button click.
    void HandleOpenExistingFileEvent();

    // Handler for the Open Program button click.
    void HandleOpenProjectFileEvent();

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

    // Handler for canceling the current build.
    void HandleCancelBuildEvent();

    // Handler for a click on a recent program item.
    void HandleRecentProjectClickedEvent(int buttonId);

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

    // Handler for when the user requests a context menu for recent files
    void HandleContextMenuRequest(const QPoint& pos);

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

    // Handler for when global settings data changes.
    void HandleGlobalPendingChangesStateChanged(bool pendingChanges);

    // Handler for when build settings data changes.
    void HandleBuildSettingsPendingChangesStateChanged(bool pendingChanges);

    // Handler for when the "Save" button is clicked.
    void HandleSaveSettingsButtonClicked();

    // Handler for when the "Restore defaults" button is clicked.
    void HandleRestoreDefaultsSettingsClicked();

    // Handler for when the tab bar tab changes.
    void HandleTabBarTabChanged(bool saveChanges);

    // Handler for when the text of the status bar is changed.
    void HandleStatusBarMessageChange(const QString&  msg);
};

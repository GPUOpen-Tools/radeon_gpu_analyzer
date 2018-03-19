#pragma once

// C++.
#include <memory>

// Qt.
#include <QDateTime>

// Local.
#include <RadeonGPUAnalyzerGUI/include/qt/rgUnsavedItemsDialog.h>
#include <RadeonGPUAnalyzerGUI/include/rgDataTypes.h>
#include "ui_rgBuildView.h"

// Forward declarations.
class QSplitter;
class QWidget;
class rgCliOutputView;
class rgFactory;
class rgFileMenu;
class rgFileMenuTitlebar;
class rgFindTextWidget;
class rgIsaDisassemblyView;
class rgMaximizeSplitter;
class rgOpenCLBuildSettingsView;
class rgSourceCodeEditor;
class rgSourceEditorSearcher;
class rgSourceEditorTitlebar;
class rgViewContainer;
class rgViewManager;
class rgSettingsButtonsView;
class rgBuildSettingsView;
class rgBuildSettingsWidget;
struct rgBuildSettings;
struct rgCliBuildOutput;
struct rgProject;
enum rgProjectAPI : char;

// The set of display modes available within the rgBuildView.
enum class EditMode
{
    Empty,
    SourceCode,
    BuildSettings
};

Q_DECLARE_METATYPE(EditMode)

class rgBuildView : public QWidget
{
    Q_OBJECT
public:
    // An enumeration used to selectively save a subset of all modified files.
    enum rgFilesToSave
    {
        SourceFiles,
        BuildSettings,
        All
    };

public:
    rgBuildView(rgProjectAPI api, QWidget* pParent);
    virtual ~rgBuildView() = default;

    // Add an existing source file to the given project.
    bool AddExistingSourcefileToProject(const std::string& sourceFilepath);

    // Create a new empty project used by the rgBuildView.
    // Returns true if the project was actually created, and false otherwise.
    bool CreateNewEmptyProject();

    // Create a new empty source file in the existing project.
    // Returns true if the file was actually created, and false otherwise.
    bool CreateNewSourceFileInProject();

    // Return true when a project build is in progress.
    bool IsBuildInProgress() const;

    // Build a list of all source files that are currently modified.
    void GetUnsavedSourceFiles(QStringList& unsavedSourceFiles);

    // Check if the given source file has been successfully disassembled.
    bool IsFileDisassembled(const std::string& inputFilePath) const;

    // Check if the rgBuildView has any source files open.
    bool IsEmpty() const;

    // Attempt to load the build output for the project in the given directory.
    bool LoadBuildOutput(const std::string& projectFolder, const std::vector<std::string>* pTargetGpus = nullptr);

    // Load a project into the rgConfigManager using a full file path to an .rga file.
    bool LoadProjectFile(const std::string& projectFilePath);

    // Save the currently open and active file.
    void SaveCurrentFile();

    // Save all unsaved files.
    void SaveFiles(const QStringList& unsavedFiles);

    // Save the source file at the given path.
    void SaveSourceFile(const std::string& sourceFilePath);

    // Request saving of a single file.
    rgUnsavedItemsDialog::UnsavedFileDialogResult RequestSaveFile(const std::string& fullPath);

    // Display the dialog asking the user to save files. Users can save all files, source files only, or build settings only.
    bool ShowSaveDialog(rgFilesToSave filesToSave = rgFilesToSave::All);

    // Brings up a dialog asking if the user wants to save the given files, then saves them.
    rgUnsavedItemsDialog::UnsavedFileDialogResult RequestSaveFiles(const QStringList& unsavedFiles);

    // Requests a save for unsaved files and then removes all files if accepted.
    bool RequestRemoveAllFiles();

    // Sets the contents of the given file as the text in the rgSourceCodeEditor.
    void SetSourceCodeText(const std::string& fileFullPath);

    // Show the disassembly view for the currently selected file item.
    void ShowCurrentFileDisassembly();

    // Toggle the visibility of the disassembly view.
    void ToggleDisassemblyViewVisibility(bool isVisible);

    // Connect signals for the project build settings view.
    void ConnectBuildSettingsSignals();

    // Connect signals to the source editor's find widget.
    void ConnectFindSignals();

    // Connect signals for the build view.
    void ConnectBuildViewSignals();

    // Connect signals for the CLI output windows.
    void ConnectOutputWindowSignals();

    // Connect signals for the disassembly view.
    void ConnectDisassemblyViewSignals();

    // Connect interface signals to slots.
    void ConnectFileSignals();

    // Connect signals used for an individual rgSourceCodeEditor instance.
    void ConnectSourcecodeEditorSignals(rgSourceCodeEditor* pEditor);

    // Open the build settings interface.
    void OpenBuildSettings();

    // Populate the rgBuildView with the current project.
    bool PopulateBuildView();

    // Return the file menu within the build view.
    rgFileMenu* GetFileMenu() const { return m_pFileMenu; }

    // Builds the current project.
    void BuildCurrentProject();

    // Reset build view state.
    void ResetView();

    // Save the view's splitter positions.
    void SetConfigSplitterPositions();

    // Restore a saved view layout.
    void RestoreViewLayout();

    // Retrieve the rgSourceCodeEditor instance to use for the given filename.
    rgSourceCodeEditor* GetEditorForFilepath(const std::string fullFilepath);

    // Cancel the current build.
    void CancelCurrentBuild();

signals:
    // A signal emitted when the ability to correlate change state.
    void LineCorrelationEnabledStateChanged(rgSourceCodeEditor* pEditor, bool isEnabled);

    // A signal emitted when the number of files in the project has changed.
    void ProjectFileCountChanged(bool isProjectEmpty);

    // A signal invoked when the rgSourceCodeEditor editor modification state is changed.
    void CurrentEditorModificationStateChanged(bool isModified);

    // A signal invoked when the file in rgSourceCodeEditor has been modified outside the environment.
    void CurrentFileModified();

    // Create a new file from the default menu item.
    void CreateFileButtonClicked();

    // A signal when the edit mode is changed.
    void EditModeChanged(EditMode editMode);

    // Add a new file from the default menu item.
    void OpenFileButtonClicked();

    // A signal emitted when an existing source file has been renamed.
    void FileRenamed(const std::string& oldFilepath, const std::string& newFilepath);

    // A signal emitted when an individual clone's build settings should be saved.
    void ProjectBuildSettingsSaved(std::shared_ptr<rgBuildSettings> pBuildSettings);

    // A signal emitted when a new project has been loaded.
    void ProjectLoaded(std::shared_ptr<rgProject> pProject);

    // A signal emitted when a project clone has successfully finished building.
    void ProjectBuildSuccess();

    // A signal emitted when a project build process has started.
    void ProjectBuildStarted();

    // A signal emitted when a project clone has encountered a build failure.
    void ProjectBuildFailure();

    // A signal emitted when a project clone's build was canceled.
    void ProjectBuildCanceled();

    // Signal emitted when the user changes the selected entrypoint index for a given file.
    void SelectedEntrypointChanged(const std::string& targetGpu, const std::string& inputFilePath, const std::string& selectedEntrypointName);

    // Set the text in the main window's status bar.
    void SetStatusBarText(const std::string& statusBarText, int timeoutMilliseconds = 0);

    // Signal to force update of use default project name check box.
    void UpdateUseDefaultProjectNameCheckbox();

public slots:
    // Handler for when the "Save" button is clicked.
    void HandleSaveSettingsButtonClicked();

    // Handler invoked when the use triggers the Edit menu's Find action.
    void HandleFindTriggered();

    // Handler invoked when the project building status is changed.
    void HandleIsBuildInProgressChanged(bool isBuilding);

    // Handler invoked when a project clone finished building successfully.
    void HandleProjectBuildSuccess();

    // Handler invoked to scroll currently active source editor to required line.
    void HandleScrollCodeEditorToLine(int lineNum);

private slots:
    // Handler invoked when the user changes the selected entrypoint index for a given file.
    void HandleSelectedEntrypointChanged(const std::string& inputFilePath, const std::string& selectedEntrypointName);

    // Handler invoked when the source editor is hidden from view.
    void HandleSourceEditorHidden();

    // Handler invoked when the source editor is resized.
    void HandleSourceEditorResized();

    // Handler invoked when the active file is switched within the file menu.
    void HandleSelectedFileChanged(const std::string& currentFilename, const std::string& newFilename);

    // Handler invoked when the correlated highlighted line index in the current file is updated.
    void HandleHighlightedCorrelationLineUpdated(int lineNumber);

    // Handler invoked when the ability to correlate line numbers between input source and disassembly changes.
    void HandleIsLineCorrelationEnabled(rgSourceCodeEditor* pEditor, bool isEnabled);

    // Handler invoked when the user changes the selected line in the current source editor.
    void HandleSourceFileSelectedLineChanged(rgSourceCodeEditor* pEditor, int lineNumber);

    // Handler invoked when the build settings pending changes flag is updated,
    void HandleBuildSettingsPendingChangesStateChanged(bool hasPendingChanges);

    // Handler invoked when the project build settings have been saved.
    void HandleBuildSettingsSaved(std::shared_ptr<rgBuildSettings> pBuildSettings);

    // Handler invoked when the user changes the current target GPU.
    void HandleSelectedTargetGpuChanged(const std::string& targetGpu);

    // Handler invoked when a file within the file menu has been renamed.
    void HandleFileRenamed(const std::string& oldFilepath, const std::string& newFilepath);

    // Handle switching focus to the next view in the rgViewManager.
    void HandleFocusNextView();

    // Handler invoked when the find widget should be toggled.
    void HandleFindWidgetVisibilityToggled();

    // Handler invoked when the project is renamed.
    void HandleProjectRenamed(const std::string& projectName);

    // Handler invoked when the editor modification state changes.
    void HandleEditorModificationStateChanged(bool isModified);

    // Handler invoked when the close button is clicked on one of the file menu items.
    void HandleMenuItemCloseButtonClicked(const std::string fullPath);

    // Handler invoked when the application state changes.
    void HandleApplicationStateChanged(Qt::ApplicationState state);

    // Handler invoked when the requested width of the disassembly table is changed.
    void HandleDisassemblyTableWidthResizeRequested(int minimumWidth);

    // Handler for when the "Restore defaults" button is clicked.
    void HandleRestoreDefaultsSettingsClicked();

    // Handler for when the build settings frame border needs to be set red.
    void HandleSetFrameBorderRed();

    // Handler for when the build settings frame border needs to be set black.
    void HandleSetFrameBorderBlack();

    // Handler for when the disassembly view needs to be maximized.
    void HandleDisassemblyViewSizeMaximize();

    // Handler for when the disassembly view needs to be restored.
    void HandleDisassemblyViewSizeRestore();

    // Handler invoked when a splitter has been moved.
    void HandleSplitterMoved(int pos, int index);

    // ******************
    // BUILD MENU - START
    // ******************

    // Handler for when the file menu or menubar's Build Settings item is clicked.
    void HandleBuildSettingsMenuButtonClicked();

    // ****************
    // BUILD MENU - END
    // ****************

private:
    // Adds a file to the file menu.
    void AddFile(const std::string& fileFullPath, bool isNewFile = false);

    // Is the user currently allowed to change the rgBuildView's EditMode?
    bool CanSwitchEditMode();

    // Has the code in the given editor been modified since the latest build?
    bool CheckSourcesModifiedSinceLastBuild(rgSourceCodeEditor* pCodeEditor);

    // Clear the file menu and source editors.
    void ClearBuildView();

    // Clear the map of sourcecode editors.
    void ClearEditors();

    // Clear the entrypoint list for each file item.
    void ClearFileItemsEntrypointList();

    // Build a path to the project's build output folder.
    std::string CreateProjectBuildOutputPath() const;

    // Create the build settings interface to edit project settings.
    void CreateBuildSettingsView();

    // Create the find widget for the source editor.
    void CreateFindWidget();

    // Create the rgBuildView's ISA disassembly view.
    void CreateIsaDisassemblyView();

    // Create a new empty source file within the existing project.
    // Returns true if the file was actually created, and false otherwise.
    bool CreateNewSourceFile(const std::string& sourceFileName, std::string& fullSourceFilepath);

    // Destroy obsolete build outputs from a previous project build.
    void DestroyProjectBuildArtifacts();

    // Find the starting line for an entrypoint by starting from the line number provided by "list-kernels".
    int FindEntrypointStartLine(rgSourceCodeEditor* pEditor, int listKernelsStartLine) const;

    // Retrieve the filename for the given rgSourceCodeEditor instance.
    std::string GetFilepathForEditor(const rgSourceCodeEditor* pEditor);

    // Handle a new output string from the CLI.
    void HandleNewCLIOutputString(const std::string& cliOutputString);

    // Highlight the line of source code where the given named entrypoint is defined.
    void HighlightEntrypointStartLine(const std::string& inputFilePath, const std::string& selectedEntrypointName);

    // Check if the given source editor has line correlation enabled.
    bool IsLineCorrelationEnabled(rgSourceCodeEditor* pSourceEditor);

    // Invoke the CLI to query entrypoint names and start line numbers in the input file.
    bool LoadEntrypointLineNumbers();

    // Update the filename for an open file.
    void RenameFile(const std::string& oldFilepath, const std::string& newFilepath);

    // Update the project name.
    void RenameProject(const std::string& fullPath);

    // Remove all build outputs associated with the given input file.
    void DestroyBuildOutputsForFile(const std::string& inputFileFullPath);

    // Remove the relevant rgSourceCodeEditor from the view when a file has been closed.
    void RemoveEditor(const std::string& filename);

    // Remove the given input file from the rgBuildView.
    void RemoveInputFile(const std::string& inputFileFullPath);

    // Show the ISA disassembly view table.
    bool LoadDisassemblyFromBuildOutput();

    // Switch the current edit mode for the build view.
    void SwitchEditMode(EditMode mode);

    // Set the new content to display within the BuildView.
    void SetViewContentsWidget(QWidget* pOldContents, QWidget* pNewContents);

    // Handle displaying the build view when all files have been closed.
    void SwitchToFirstRemainingFile();

    // Switch the current rgSourceCodeEditor to the given instance.
    bool SwitchToEditor(rgSourceCodeEditor* pEditor);

    // Helper function to save the text in an rgSourceCodeEditor to file and indicate the editor is unmodified.
    void SaveEditorTextToFile(rgSourceCodeEditor* pEditor, const std::string& fullPath);

    // Helper function used to toggle the visibility of the find widget.
    void ToggleFindWidgetVisibility(bool isVisible);

    // Recompute the location and geometry of the rgFindTextWidget.
    void UpdateFindWidgetGeometry();

    // Update the text in the source editor's title bar.
    void UpdateSourceEditorTitlebar(rgSourceCodeEditor* pCodeEditor);

    // Update the correlation state for the given source file.
    void UpdateSourceFileCorrelationState(const std::string& filePath, bool isCorrelated);

    // Check if the currently open source file has been modified externally and handle reloading the file.
    void CheckExternalFileModification();

    // Create a project clone.
    void CreateProjectClone();

    // Try to find the entry point containing the given line.
    // Returns "true" if found or "false" otherwise. The name of found function is returned in "entryName".
    bool GetEntrypointNameForLineNumber(const std::string& filePath, int lineNumber, std::string& entryName);

    // A map of filename to the editor used to view the file.
    std::map<std::string, rgSourceCodeEditor*> m_sourceCodeEditors;

    // The project that's edited as part of the rgBuildView.
    std::shared_ptr<rgProject> m_pProject = nullptr;

    // A pointer to the factory used to create API specific objects.
    std::shared_ptr<rgFactory> m_pFactory = nullptr;

    // A map of GPU name to the build outputs for the GPU.
    rgBuildOutputsMap m_buildOutputs;

    // The index of the clone being edited.
    int m_cloneIndex;

    // The current editor mode for the rgBuildView.
    EditMode m_editMode = EditMode::Empty;

    // The time of the last successful build.
    QDateTime m_lastSuccessfulBuildTime;

    // The instance of the rgSourceCodeEditor being used currently.
    rgSourceCodeEditor* m_pCurrentCodeEditor = nullptr;

    // The panel that gets inserted when all files have been closed.
    QWidget* m_pEmptyPanel = nullptr;

    // The file menu.
    rgFileMenu* m_pFileMenu = nullptr;

    // The ISA disassembly view.
    rgIsaDisassemblyView* m_pDisassemblyView = nullptr;

    // The CLI output window.
    rgCliOutputView* m_pCliOutputWindow = nullptr;

    // The build settings interface.
    rgBuildSettingsView* m_pBuildSettingsView = nullptr;

    // The "Save" and "Restore Default" buttons.
    rgSettingsButtonsView* m_pSettingsButtonsView = nullptr;

    // The horizontal splitter responsible for dividing the rgBuildView's File Menu and Source Editor windows.
    QSplitter* m_pFileMenuSplitter = nullptr;

    // The horizontal splitter responsible for dividing the rgBuildView's File Menu + Source Editors and Disassembly view tables.
    rgMaximizeSplitter* m_pDisassemblyViewSplitter = nullptr;

    // The horizontal splitter responsible for dividing the rgBuildView's File Menu + Source + Disassembly views and the Output window.
    rgMaximizeSplitter* m_pOutputSplitter = nullptr;

    // Container for the various widgets that end up displayed as the Source View.
    QWidget* m_pSourceViewStack = nullptr;

    // View containers.
    rgViewContainer* m_pSourceViewContainer = nullptr;
    rgViewContainer* m_pFileMenuViewContainer = nullptr;
    rgViewContainer* m_pDisassemblyViewContainer = nullptr;
    rgViewContainer* m_pBuildOutputViewContainer = nullptr;

    // File menu titlebar.
    rgFileMenuTitlebar* m_pFileMenuTitlebar = nullptr;

    // Source editor titlebar.
    rgSourceEditorTitlebar* m_pSourceEditorTitlebar = nullptr;

    // View manager.
    rgViewManager* m_pViewManager = nullptr;

    // Map of editors to the last modification times of the underlying files.
    std::map<rgSourceCodeEditor*, QDateTime> m_fileModifiedTimeMap;

    // A map that associates an input file with a pair of the file's entrypoint start and end line numbers.
    std::map<std::string, EntryToSourceLineRange> m_entrypointLineNumbers;

    // A find widget used to edit source code.
    rgFindTextWidget* m_pFindWidget = nullptr;

    // An interface used to allow the rgFindTextWidget to search source editor code.
    rgSourceEditorSearcher* m_pSourceSearcher = nullptr;

    // A widget to contain the build settings and the save buttons.
    rgBuildSettingsWidget* m_pBuildSettingsWidget = nullptr;

    // A handle to signal that the current build should be canceled.
    bool m_cancelBuildSignal = false;

    // A flag indicating if the project is currently being built.
    bool m_isBuildInProgress = false;

    // The currently selected target GPU.
    std::string m_currentTargetGpu;

    // The build view UI.
    Ui::rgBuildView ui;
};
#pragma once

// C++.
#include <memory>

// Qt.
#include <QDateTime>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgSyntaxHighlighter.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgUnsavedItemsDialog.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDataTypes.h>
#include <ui_rgBuildView.h>

// Forward declarations.
class QSplitter;
class QWidget;
class rgCliOutputView;
class rgFactory;
class rgMenu;
class rgMenuTitlebar;
class rgFindTextWidget;
class rgIsaDisassemblyView;
class rgMaximizeSplitter;
class rgSourceCodeEditor;
class rgSourceEditorSearcher;
class rgSourceEditorTitlebar;
class rgViewContainer;
class rgViewManager;
class rgSettingsButtonsView;
class rgBuildSettingsView;
class rgBuildSettingsWidget;
class rgScrollArea;
struct rgBuildSettings;
struct rgCliBuildOutput;
struct rgProject;
enum class rgProjectAPI : char;

// The set of display modes available within the rgBuildView.
enum class EditMode
{
    Empty,
    SourceCode,
    BuildSettings,
    PipelineSettings,
};

Q_DECLARE_METATYPE(EditMode)

// An enumeration to indicate the view type.
enum class rgViewManagerViewContainerIndex
{
    FileMenu,
    SourceView,
    DisassemblyView,
    BuildOutputView,
    Count
};

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

    rgBuildView(rgProjectAPI api, QWidget* pParent);
    virtual ~rgBuildView() = default;

    // Connect signals for the project build settings view.
    virtual void ConnectBuildSettingsSignals() = 0;

    // Connect signals for the file menu.
    virtual bool ConnectMenuSignals() = 0;

    // Retrieve a pointer to the rgBuildView's API-specific file menu.
    virtual rgMenu* GetMenu() const = 0;

    // Add the files to the file menu.
    virtual bool PopulateMenu() = 0;

    // Set the default widget for build settings view.
    virtual void SetDefaultFocusWidget() const = 0;

    // Check if the given source file has been successfully disassembled.
    virtual bool IsGcnDisassemblyGenerated(const std::string& inputFilePath) const = 0;

    // Load the session metadata file at the given path.
    virtual bool LoadSessionMetadata(const std::string& metadataFilePath, std::shared_ptr<rgCliBuildOutput>& pBuildOutput) = 0;

    // Reload the content of the file modified by an external process.
    virtual void ReloadFile(const std::string& filePath);

    // Show the disassembly view for the currently selected file item.
    virtual void ShowCurrentFileDisassembly() = 0;

    // Save the currently open and active file.
    virtual void SaveCurrentFile();

    // Save the source file at the given path.
    virtual void SaveSourceFile(const std::string& sourceFilePath);

    // Save all unsaved files, settings, etc. before building the project.
    // Returns "true" if the user selected to proceed with the build, or "false" otherwise.
    virtual bool SaveCurrentState();

    // Builds the current project.
    void BuildCurrentProject();

    // Create the file menu.
    bool CreateFileMenu();

    // Create a new empty project used by the rgBuildView.
    // Returns true if the project was actually created, and false otherwise.
    bool CreateNewEmptyProject();

    // Return true when a project build is in progress.
    bool IsBuildInProgress() const;

    // Build a list of all source files that are currently modified.
    void GetUnsavedSourceFiles(QStringList& unsavedSourceFiles);

    // Initialize the rgBuildView user interface.
    void InitializeView();

    // Check if the rgBuildView has any source files open.
    bool HasSourceCodeEditors() const;

    // Check if some project is loaded.
    bool HasProject() const;

    // Attempt to load the build output for the project in the given directory.
    bool LoadBuildOutput(const std::string& projectFolder, const std::vector<std::string>* pTargetGpus = nullptr);

    // Load a project into the rgConfigManager using a full file path to an .rga file.
    bool LoadProjectFile(const std::string& projectFilePath);

    // Save all unsaved files.
    bool SaveFiles(const QStringList& unsavedFiles);

    // Request saving of a single file.
    rgUnsavedItemsDialog::UnsavedFileDialogResult RequestSaveFile(const std::string& fullPath);

    // Display the dialog asking the user to save files. Users can save all files, source files only, or build settings only.
    // filesToSave designates the items to be included as part of the save operation (all by default).
    // shouldSaveSourceFiles needs to be true for the user to be prompted for saving source files. Otherwise, files would be
    // ignored (other widgets like the build settings would still be taken care of).
    bool ShowSaveDialog(rgFilesToSave filesToSave = rgFilesToSave::All, bool shouldSaveSourceFiles = false);

    // Brings up a dialog asking if the user wants to save the given files, then saves them.
    rgUnsavedItemsDialog::UnsavedFileDialogResult RequestSaveFiles(const QStringList& unsavedFiles);

    // Requests a save for unsaved files and then removes all files if accepted.
    bool RequestRemoveAllFiles();

    // Sets the contents of the given file as the text in the rgSourceCodeEditor.
    void SetSourceCodeText(const std::string& fileFullPath);

    // Toggle the visibility of the disassembly view.
    void ToggleDisassemblyViewVisibility(bool isVisible);

    // Connect signals to the source editor's find widget.
    void ConnectFindSignals();

    // Connect signals for the build view.
    void ConnectBuildViewSignals();

    // Connect signals for the CLI output windows.
    void ConnectOutputWindowSignals();

    // Connect signals for the disassembly view.
    bool ConnectDisassemblyViewSignals();

    // Connect interface signals to slots.
    void ConnectFileSignals();

    // Connect signals used for an individual rgSourceCodeEditor instance.
    void ConnectSourcecodeEditorSignals(rgSourceCodeEditor* pEditor);

    // Open the build settings interface.
    void OpenBuildSettings();

    // Populate the rgBuildView with the current project.
    bool PopulateBuildView();

    // Reset build view state.
    void ResetView();

    // Save the view's splitter positions.
    void SetConfigSplitterPositions();

    // Restore a saved view layout.
    void RestoreViewLayout();

    // Retrieve the rgSourceCodeEditor instance to use for the given filename.
    rgSourceCodeEditor* GetEditorForFilepath(const std::string& fullFilepath, rgSrcLanguage lang = rgSrcLanguage::Unknown);

    // Cancel the current build.
    void CancelCurrentBuild();

    // Switch the focus to the source code editor.
    void FocusOnSourceCodeEditor();

    // Switch the focus to file menu.
    virtual void FocusOnFileMenu() = 0;

    // Retrieve the current project's API.
    std::string GetCurrentProjectAPIName() const;

signals:
    // A signal emitted when a new source file is added to a project.
    void AddedSourceFileToProject();

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

    // A signal emitted when a new project has been created.
    void ProjectCreated();

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

    // Signal emitted when the user changes the selected entry point index for a given file.
    void SelectedEntrypointChanged(const std::string& targetGpu, const std::string& inputFilePath, const std::string& selectedEntrypointName);

    // Set the text in the main window's status bar.
    void SetStatusBarText(const std::string& statusBarText, int timeoutMilliseconds = 0);

    // Update the file coloring for file menu.
    void UpdateFileColoring();

    // Update the application notification message.
    void UpdateApplicationNotificationMessageSignal(const std::string& message, const std::string& tooltip);

public slots:
    // Handle switching focus to the next view in the rgViewManager.
    void HandleFocusNextView();

    // Handle switching focus to the previous view in the rgViewManager.
    void HandleFocusPrevView();

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

    // Handler invoked when the build settings pending changes flag is updated,
    void HandleBuildSettingsPendingChangesStateChanged(bool hasPendingChanges);

    // Handler invoked when the project build settings have been saved.
    void HandleBuildSettingsSaved(std::shared_ptr<rgBuildSettings> pBuildSettings);

    // Handler for when the build settings frame border needs to be set green.
    void HandleSetFrameBorderGreen();

    // Handler for when the build settings frame border needs to be set red.
    void HandleSetFrameBorderRed();

    // Handler for when the build settings frame border needs to be set black.
    void HandleSetFrameBorderBlack();

    // Handler to give focus to output window.
    void HandleFocusCliOutputWindow();

    // Handler to switch the container size.
    void HandleSwitchContainerSize();

    // Handler to give focus to the previous view.
    void HandleFocusPreviousView();

    // Handler to set the file menu as the current focus view.
    void HandleFileMenuFocusInEvent();

protected slots:
    // Handler invoked when a file within the file menu has been renamed.
    virtual void HandleFileRenamed(const std::string& oldFilepath, const std::string& newFilepath);

    // Handler invoked when the active file is switched within the file menu.
    virtual void HandleSelectedFileChanged(const std::string& currentFilename, const std::string& newFilename) = 0;

    // Handler invoked when the user changes the selected line in the current source editor.
    virtual void HandleSourceFileSelectedLineChanged(rgSourceCodeEditor* pEditor, int lineNumber) = 0;

    // Set the project build settings border color.
    virtual void SetAPISpecificBorderColor() = 0;

private slots:
    // Handler invoked when the source editor is hidden from view.
    void HandleSourceEditorHidden();

    // Handler invoked when the source editor is resized.
    void HandleSourceEditorResized();

    // Handler invoked when the source editor requests to open a header file.
    void HandleSourceEditorOpenHeaderRequest(const QString& path);

    // Handler invoked when the "Dismiss Message" button is pushed in the code editor titlebar.
    void HandleCodeEditorTitlebarDismissMsgPressed();

    // Handler invoked when the correlated highlighted line index in the current file is updated.
    void HandleHighlightedCorrelationLineUpdated(int lineNumber);

    // Handler invoked when the ability to correlate line numbers between input source and disassembly changes.
    void HandleIsLineCorrelationEnabled(rgSourceCodeEditor* pEditor, bool isEnabled);

    // Handler invoked when the user changes the current target GPU.
    void HandleSelectedTargetGpuChanged(const std::string& targetGpu);

    // Handler invoked when the find widget should be toggled.
    void HandleFindWidgetVisibilityToggled();

    // Handler invoked when the project is renamed.
    void HandleProjectRenamed(const std::string& projectName);

    // Handler invoked when the editor modification state changes.
    void HandleEditorModificationStateChanged(bool isModified);

    // Handler invoked when the close button is clicked on one of the file menu items.
    void HandleMenuItemCloseButtonClicked(const std::string& fullPath);

    // Handler invoked when the application state changes.
    void HandleApplicationStateChanged(Qt::ApplicationState state);

    // Handler invoked when the requested width of the disassembly table is changed.
    void HandleDisassemblyTableWidthResizeRequested(int minimumWidth);

    // Handler for when the "Restore defaults" button is clicked.
    void HandleRestoreDefaultsSettingsClicked();

    // Handler for when the disassembly view needs to be maximized.
    void HandleDisassemblyViewSizeMaximize();

    // Handler for when the disassembly view needs to be restored.
    void HandleDisassemblyViewSizeRestore();

    // Handler invoked when a splitter has been moved.
    void HandleSplitterMoved(int pos, int index);

    // Handler invoked when the disassembly view is clicked.
    void HandleDisassemblyViewClicked();

    // Handler to set output window focus.
    void HandleSetOutputWindowFocus();

    // Handler to set disassembly view focus.
    void HandleSetDisassemblyViewFocus();

    // ******************
    // BUILD MENU - START
    // ******************

    // Handler for when the file menu or menubar's Build Settings item is clicked.
    void HandleBuildSettingsMenuButtonClicked();

    // ****************
    // BUILD MENU - END
    // ****************

protected:
    // Used to implement API-specific handling of build cancellation.
    // APIs may use this to update the UI when build status is changed, but does nothing by default.
    virtual void CurrentBuildCancelled() {}

    // Used to handle build success for an API-specific rgBuildView implementation.
    // APIs may use this to update the UI when build status is changed, but does nothing by default.
    virtual void CurrentBuildSucceeded() {}

    // Create an API-specific file menu.
    virtual bool CreateMenu(QWidget* pParent) = 0;

    // Connect API-specific rgBuildView signals to the disassembly view.
    virtual void ConnectDisassemblyViewApiSpecificSignals() {}

    // Destroy obsolete build outputs from a previous project build.
    virtual void DestroyProjectBuildArtifacts();

    // Remove all build outputs associated with the given input file.
    void DestroyBuildOutputsForFile(const std::string& inputFileFullPath);

    // Handle API-specific rgBuildView mode switching.
    // Do nothing by default, as rgBuildView does not require mode-specific behavior.
    virtual void HandleModeSpecificEditMode(EditMode newMode) {}

    // Initialize views specific to the current mode only.
    // Do nothing by default. Each mode-specific implementation is
    // responsible for initializing their own views.
    virtual void InitializeModeSpecificViews() {}

    // Check if the given source editor has line correlation enabled.
    virtual bool IsLineCorrelationEnabled(rgSourceCodeEditor* pSourceEditor);

    // Check if the current API has line correlation supported.
    virtual bool IsLineCorrelationSupported() const = 0;

    // Check if the given source file path already exists within the project's current clone.
    virtual bool IsSourceFileInProject(const std::string& sourceFilePath) const = 0;

    // Helper function used to toggle the visibility of the find widget.
    virtual void ToggleFindWidgetVisibility(bool isVisible);

    // Recompute the location and geometry of the rgFindTextWidget.
    virtual void UpdateFindWidgetGeometry();

    // Helper function to save the text in an rgSourceCodeEditor to file and indicate the editor is unmodified.
    void SaveEditorTextToFile(rgSourceCodeEditor* pEditor, const std::string& fullPath);

    // Helper function to discard the changes in an rgSourceCodeEditor.
    void DiscardEditorChanges(rgSourceCodeEditor* pEditor);

    // Build a path to the project's build output folder.
    std::string CreateProjectBuildOutputPath() const;

    // Retrieve the filename for the given rgSourceCodeEditor instance.
    std::string GetFilepathForEditor(const rgSourceCodeEditor* pEditor);

    // Handle a new output string from the CLI.
    void HandleNewCLIOutputString(const std::string& cliOutputString);

    // Set the new content to display within the BuildView.
    void SetViewContentsWidget(QWidget* pNewContents);

    // Display the "Are you sure you want to remove this file?" dialog, and if so, remove the file.
    bool ShowRemoveFileConfirmation(const std::string& messageString, const std::string& fullPath);

    // Switch the current rgSourceCodeEditor to the given instance.
    bool SwitchToEditor(rgSourceCodeEditor* pEditor);

    // Attach the Find widget to the given view when visible, or remove if it should be hidden.
    void UpdateFindWidgetViewAttachment(QWidget* pView, bool isVisible);

    // Update the text in the source editor's title bar.
    void UpdateSourceEditorTitlebar(rgSourceCodeEditor* pCodeEditor);

    // Switch the current edit mode for the build view.
    void SwitchEditMode(EditMode mode);

    // Remove the relevant rgSourceCodeEditor from the view when a file has been closed.
    void RemoveEditor(const std::string& filename, bool switchToNextFile = true);

    // Update the application notification message.
    virtual void UpdateApplicationNotificationMessage() = 0;

private:
    // Is the user currently allowed to change the rgBuildView's EditMode?
    bool CanSwitchEditMode();

    // Has the code in the given editor been modified since the latest build?
    bool CheckSourcesModifiedSinceLastBuild(rgSourceCodeEditor* pCodeEditor);

    // Clear the file menu and source editors.
    void ClearBuildView();

    // Clear the map of sourcecode editors.
    void ClearEditors();

    // Create the build settings interface to edit project settings.
    void CreateBuildSettingsView();

    // Create the find widget for the source editor.
    void CreateFindWidget();

    // Create the rgBuildView's ISA disassembly view.
    void CreateIsaDisassemblyView();

    // Retrieve the structure of build outputs from the provided API-specific build output pointer.
    bool GetInputFileOutputs(std::shared_ptr<rgCliBuildOutput> pBuildOutputs, InputFileToBuildOutputsMap& outputs) const;

    // Update the filename for an open file.
    void RenameFile(const std::string& oldFilepath, const std::string& newFilepath);

    // Update the project name.
    void RenameProject(const std::string& fullPath);

    // Remove the given input file from the rgBuildView.
    void RemoveInputFile(const std::string& inputFileFullPath);

    // Show the ISA disassembly view table.
    bool LoadDisassemblyFromBuildOutput();

    // Handle displaying the build view when all files have been closed.
    void SwitchToFirstRemainingFile();

    // Update the correlation state for the given source file.
    void UpdateSourceFileCorrelationState(const std::string& filePath, bool isCorrelated);

    // Update the FindWidget's search context using the current source code editor.
    void UpdateSourceEditorSearchContext();

    // Check if the currently open source file has been modified externally and handle reloading the file.
    void CheckExternalFileModification();

    // Create a project clone.
    void CreateProjectClone();

    // Open an include file in the user's app of choice (or, system default).
    bool OpenIncludeFile(const std::string& fullFilePath);

    // Set the build application settings stylesheet.
    void SetBuildSettingsStylesheet(const std::string& stylesheet);

protected:
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
    rgMenuTitlebar* m_pFileMenuTitlebar = nullptr;

    // Source editor titlebar.
    rgSourceEditorTitlebar* m_pSourceEditorTitlebar = nullptr;

    // View manager.
    rgViewManager* m_pViewManager = nullptr;

    // Map of editors to the last modification times of the underlying files.
    std::map<rgSourceCodeEditor*, QDateTime> m_fileModifiedTimeMap;

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

    // The parent widget.
    QWidget* m_pParent = nullptr;

    // The currently selected target GPU.
    std::string m_currentTargetGpu;

    // The build view UI.
    Ui::rgBuildView ui;
};

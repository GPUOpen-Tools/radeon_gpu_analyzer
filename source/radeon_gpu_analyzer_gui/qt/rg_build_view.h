//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for Build View class.
//=============================================================================

#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_BUILD_VIEW_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_BUILD_VIEW_H_

// C++.
#include <memory>

// Qt.
#include <QDateTime>

// Local.
#include "source/radeon_gpu_analyzer_gui/qt/rg_syntax_highlighter.h"
#include "source/radeon_gpu_analyzer_gui/qt/rg_unsaved_items_dialog.h"
#include "source/radeon_gpu_analyzer_gui/rg_data_types.h"
#include "ui_rg_build_view.h"

// Forward declarations.
class QSplitter;
class QWidget;
class QFileInfo;
class RgCliOutputView;
class RgFactory;
class RgMenu;
class RgMenuTitlebar;
class RgFindTextWidget;
class RgIsaDisassemblyView;
class RgMaximizeSplitter;
class RgSourceCodeEditor;
class RgSourceEditorSearcher;
class RgSourceEditorTitlebar;
class RgViewContainer;
class RgViewManager;
class RgSettingsButtonsView;
class RgBuildSettingsView;
class RgBuildSettingsWidget;
class RgScrollArea;
struct RgBuildSettings;
struct RgCliBuildOutput;
struct RgProject;
enum class RgProjectAPI : char;

// The set of display modes available within the RgBuildView.
enum class EditMode
{
    kEmpty,
    kSourceCode,
    kBuildSettings,
    kPipelineSettings,
};

Q_DECLARE_METATYPE(EditMode)

// An enumeration to indicate the view type.
enum class RgViewManagerViewContainerIndex
{
    kFileMenu,
    kSourceView,
    kDisassemblyView,
    kBuildOutputView,
    kCount
};

class RgBuildView : public QWidget
{
    Q_OBJECT
public:
    // An enumeration used to selectively save a subset of all modified files.
    enum RgFilesToSave
    {
        kSourceFiles,
        kBuildSettings,
        kAll
    };

    RgBuildView(RgProjectAPI api, QWidget* parent);
    virtual ~RgBuildView() = default;

    // Connect signals for the project build settings view.
    virtual void ConnectBuildSettingsSignals() = 0;

    // Connect signals for the file menu.
    virtual bool ConnectMenuSignals() = 0;

    // Retrieve a pointer to the RgBuildView's API-specific file menu.
    virtual RgMenu* GetMenu() const = 0;

    // Add the files to the file menu.
    virtual bool PopulateMenu() = 0;

    // Set the default widget for build settings view.
    virtual void SetDefaultFocusWidget() const = 0;

    // Check if the given source file has been successfully disassembled.
    virtual bool IsGcnDisassemblyGenerated(const std::string& input_file_path) const = 0;

    // Load the session metadata file at the given path.
    virtual bool LoadSessionMetadata(const std::string& metadata_file_path, std::shared_ptr<RgCliBuildOutput>& build_output) = 0;

    // Reload the content of the file modified by an external process.
    virtual void ReloadFile(const std::string& file_path);

    // Show the disassembly view for the currently selected file item.
    virtual void ShowCurrentFileDisassembly() = 0;

    // Save the currently open and active file.
    virtual void SaveCurrentFile(EditMode mode);

    // Save the source file at the given path.
    virtual void SaveSourceFile(const std::string& source_file_path);

    // Save all unsaved files, settings, etc. before building the project.
    // Returns "true" if the user selected to proceed with the build, or "false" otherwise.
    virtual bool SaveCurrentState();

    // Builds the current project. 
    // Optionally, when given a list of binaries file paths in binary analysis mode, only builds those binaries 
    // and adds them to the existing project, rather than clearing the project and building all binaries in the project settings. 
    // In other modes, since there are no binaries to build this is empty by default.
    void BuildCurrentProject(std::vector<std::string> binaries_to_build = {});

    // Create the file menu.
    bool CreateFileMenu();

    // Create a new empty project used by the RgBuildView.
    // Returns true if the project was actually created, and false otherwise.
    bool CreateNewEmptyProject();

    // Return true when a project build is in progress.
    bool IsBuildInProgress() const;

    // Build a list of all source files that are currently modified.
    void GetUnsavedSourceFiles(QStringList& unsaved_source_files);

    // Initialize the RgBuildView user interface.
    bool InitializeView();

    // Check if the RgBuildView has any source files open.
    bool HasSourceCodeEditors() const;

    // Check if some project is loaded.
    bool HasProject() const;

    // Attempt to load the build output for the project in the given directory.
    bool LoadBuildOutput(const std::string& project_folder, const std::vector<std::string>* target_gpus = nullptr);

    // Load a project into the RgConfigManager using a full file path to an .rga file.
    bool LoadProjectFile(const std::string& project_file_path);

    // Save all unsaved files.
    bool SaveFiles(const QStringList& unsaved_files);

    // Request saving of a single file.
    RgUnsavedItemsDialog::UnsavedFileDialogResult RequestSaveFile(const std::string& full_path);

    // Display the dialog asking the user to save files. Users can save all files, source files only, or build settings only.
    // files_to_save designates the items to be included as part of the save operation (all by default).
    // should_save_source_files needs to be true for the user to be prompted for saving source files. Otherwise, files would be
    // ignored (other widgets like the build settings would still be taken care of).
    bool ShowSaveDialog(RgFilesToSave files_to_save = RgFilesToSave::kAll, bool should_save_source_files = false);

    // Brings up a dialog asking if the user wants to save the given files, then saves them.
    RgUnsavedItemsDialog::UnsavedFileDialogResult RequestSaveFiles(const QStringList& unsaved_files);

    // Requests a save for unsaved files and then removes all files if accepted.
    bool RequestRemoveAllFiles();

    // Sets the contents of the given file as the text in the RgSourceCodeEditor.
    virtual void SetSourceCodeText(const std::string& file_full_path);

    // Toggle the visibility of the disassembly view.
    void ToggleDisassemblyViewVisibility(bool is_visible);

    // Maximizes the disassembly view.
    void MaximizeDisassemblyView();

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

    // Connect signals used for an individual RgSourceCodeEditor instance.
    void ConnectSourcecodeEditorSignals(RgSourceCodeEditor* editor);

    // Open the build settings interface.
    virtual void OpenBuildSettings();

    // Populate the RgBuildView with the current project.
    bool PopulateBuildView();

    // Reset build view state.
    void ResetView();

    // Save the view's splitter positions.
    void SetConfigSplitterPositions();

    // Restore a saved view layout.
    void RestoreViewLayout();

    // Retrieve the RgSourceCodeEditor instance to use for the given filename.
    RgSourceCodeEditor* GetEditorForFilepath(const std::string& full_file_path, RgSrcLanguage lang = RgSrcLanguage::Unknown);

    // Cancel the current build.
    void CancelCurrentBuild();

    // Switch the focus to the source code editor.
    void FocusOnSourceCodeEditor();

    // Switch the focus to file menu.
    virtual void FocusOnFileMenu() = 0;

    // Retrieve the current project's API.
    std::string GetCurrentProjectAPIName() const;

    // Save the current project settings file.
    bool SaveProjectConfigFile() const;

signals:
    // A signal emitted when a new source file is added to a project.
    void AddedSourceFileToProject();

    // A signal emitted when the ability to correlate change state.
    void LineCorrelationEnabledStateChanged(RgSourceCodeEditor* editor, bool is_enabled);

    // A signal emitted when the number of files in the project has changed.
    void ProjectFileCountChanged(bool is_project_empty);

    // A signal invoked when the RgSourceCodeEditor editor modification state is changed.
    void CurrentEditorModificationStateChanged(bool is_modified);

    // A signal invoked when the file in RgSourceCodeEditor has been modified outside the environment.
    void CurrentFileModified();

    // Create a new file from the default menu item.
    void CreateFileButtonClicked();

    // A signal when the edit mode is changed.
    void EditModeChanged(EditMode edit_mode);

    // Add a new file from the default menu item.
    void OpenFileButtonClicked();

    // A signal emitted when an existing source file has been renamed.
    void FileRenamed(const std::string& old_file_path, const std::string& new_file_path);

    // A signal to indicate change of view.
    void HotKeyPressedSignal();

    // A signal emitted when an individual clone's build settings should be saved.
    void ProjectBuildSettingsSaved(std::shared_ptr<RgBuildSettings> build_settings);

    // A signal emitted when a new project has been created.
    void ProjectCreated();

    // A signal emitted when a new project has been loaded.
    void ProjectLoaded(std::shared_ptr<RgProject> project);

    // A signal emitted when a project clone has successfully finished building.
    void ProjectBuildSuccess();

    // A signal emitted when a project build process has started.
    void ProjectBuildStarted();

    // A signal emitted when a project clone has encountered a build failure.
    void ProjectBuildFailure();

    // A signal emitted when a project clone's build was canceled.
    void ProjectBuildCanceled();

    // A signal emitted to programatically trigger a project build.
    void BuildProjectEvent();

    // A signal emitted to programatically disassemble the given binary files into the project.
    void DissasembleBinaryFilesEvent(std::vector<std::string> bin_file_paths = {});

    // Signal emitted when the user changes the selected entry point index for a given file.
    void SelectedEntrypointChanged(const std::string& target_gpu, const std::string& input_file_path, const std::string& selected_entrypoint_name);

    // Signal emitted when the user changes the selected entry point index for a given file.
    void SelectedExtremelyLongKernelNameChanged(bool showKernelNameLabel, const std::string& selected_kernel_name);

    // Set the text in the main window's status bar.
    void SetStatusBarText(const std::string& status_bar_text, int timeout_milliseconds = 0);

    // Update the file coloring for file menu.
    void UpdateFileColoring();

    // Update the application notification message.
    void UpdateApplicationNotificationMessageSignal(const std::string& message, const std::string& tooltip);

    // Emit a signal to indicate change in splitter location.
    void SplitterMoved();

    // A signal emitted when the user clicks on show maximum VGPRs menu item.
    void ShowMaximumVgprClickedSignal();

    // A signal to enable/disable the Edit->Go to next maximum live VGPR line option.
    void EnableShowMaxVgprOptionSignal(bool is_enabled);

public slots:
    // Handle switching focus to the next view in the RgViewManager.
    void HandleFocusNextView();

    // Handle switching focus to the previous view in the RgViewManager.
    void HandleFocusPrevView();

    // Handler for when the "Save" button is clicked.
    void HandleSaveSettingsButtonClicked();

    // Handler invoked when the use triggers the Edit menu's Find action.
    void HandleFindTriggered();

    // Handler for the Go To line button click.
    void HandleGoToLineTriggered();

    // Handler invoked when the project building status is changed.
    void HandleIsBuildInProgressChanged(bool is_building);

    // Handler invoked when a project clone finished building successfully.
    void HandleProjectBuildSuccess();

    // Handler invoked to scroll currently active source editor to required line.
    void HandleScrollCodeEditorToLine(int line_num);

    // Handler invoked when the build settings pending changes flag is updated,
    void HandleBuildSettingsPendingChangesStateChanged(bool has_pending_changes);

    // Handler invoked when the project build settings have been saved.
    void HandleBuildSettingsSaved(std::shared_ptr<RgBuildSettings> build_settings);

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
    virtual void HandleFileRenamed(const std::string& old_file_path, const std::string& new_file_path);

    // Handler invoked when the active file is switched within the file menu.
    virtual void HandleSelectedFileChanged(const std::string& current_file_name, const std::string& new_file_name) = 0;

    // Handler invoked when the user changes the selected line in the current source editor.
    virtual void HandleSourceFileSelectedLineChanged(RgSourceCodeEditor* editor, int line_number) = 0;

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
    void HandleHighlightedCorrelationLineUpdated(int line_number);

    // Handler invoked when the ability to correlate line numbers between input source and disassembly changes.
    void HandleIsLineCorrelationEnabled(RgSourceCodeEditor* editor, bool is_enabled);

    // Handler invoked when the user changes the current target GPU.
    void HandleSelectedTargetGpuChanged(const std::string& target_gpu);

    // Handler invoked when the find widget should be toggled.
    void HandleFindWidgetVisibilityToggled();

    // Handler invoked when the project is renamed.
    void HandleProjectRenamed(const std::string& project_name);

    // Handler invoked when the editor modification state changes.
    void HandleEditorModificationStateChanged(bool is_modified);

    // Handler invoked when the close button is clicked on one of the file menu items.
    void HandleMenuItemCloseButtonClicked(const std::string& full_path);

    // Handler invoked when the application state changes.
    void HandleApplicationStateChanged(Qt::ApplicationState state);

    // Handler invoked when the requested width of the disassembly table is changed.
    void HandleDisassemblyTableWidthResizeRequested(int minimum_width);

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

    // Handler for when the file menu or menu_bar's Build Settings item is clicked.
    void HandleBuildSettingsMenuButtonClicked();

    // ****************
    // BUILD MENU - END
    // ****************

protected:
    // Used to implement API-specific handling of build cancellation.
    // APIs may use this to update the UI when build status is changed, but does nothing by default.
    virtual void CurrentBuildCancelled() {}

    // Used to handle build success for an API-specific RgBuildView implementation.
    // APIs may use this to update the UI when build status is changed, but does nothing by default.
    virtual void CurrentBuildSucceeded() {}

    // Create an API-specific file menu.
    virtual bool CreateMenu(QWidget* parent) = 0;

    // Reapply the stylesheet for the API-specific file menu when the color theme has changed.
    virtual void ReapplyMenuStyleSheet() = 0;

    // Connect API-specific RgBuildView signals to the disassembly view.
    virtual void ConnectDisassemblyViewApiSpecificSignals() {}

    // Destroy obsolete build outputs from a previous project build.
    virtual void DestroyProjectBuildArtifacts();

    // Remove all build outputs associated with the given input file.
    void DestroyBuildOutputsForFile(const std::string& input_file_full_path);

    // Function to remove the file from the metadata. Does nothing unless reimplemented by the api specific class.
    virtual void RemoveFileFromMetadata(const std::string& full_path);

    // Handle API-specific RgBuildView mode switching.
    // Do nothing by default, as RgBuildView does not require mode-specific behavior.
    virtual void HandleModeSpecificEditMode(EditMode new_mode)
    {
        Q_UNUSED(new_mode);
    }

    // Initialize views specific to the current mode only.
    // Do nothing by default. Each mode-specific implementation is
    // responsible for initializing their own views.
    virtual bool InitializeModeSpecificViews()
    {
        return true;
    }

    // Check if the given source editor has line correlation enabled.
    virtual bool IsLineCorrelationEnabled(RgSourceCodeEditor* source_editor);

    // Check if the current API has line correlation supported.
    virtual bool IsLineCorrelationSupported() const = 0;

    // Check if the given source file path already exists within the project's current clone.
    virtual bool IsSourceFileInProject(const std::string& source_file_path) const = 0;

    // Helper function used to toggle the visibility of the find widget.
    virtual void ToggleFindWidgetVisibility(bool is_visible);

    // Recompute the location and geometry of the RgFindTextWidget.
    virtual void UpdateFindWidgetGeometry();

    // Helper function to save the text in an RgSourceCodeEditor to file and indicate the editor is unmodified.
    void SaveEditorTextToFile(RgSourceCodeEditor* editor, const std::string& full_path);

    // Helper function to discard the changes in an RgSourceCodeEditor.
    void DiscardEditorChanges(RgSourceCodeEditor* editor);

    // Build a path to the project's build output folder.
    std::string CreateProjectBuildOutputPath() const;

    // Retrieve the filename for the given RgSourceCodeEditor instance.
    std::string GetFilepathForEditor(const RgSourceCodeEditor* editor);

    // Handle a new output string from the CLI.
    void HandleNewCLIOutputString(const std::string& cli_output_string);

    // Set the new content to display within the BuildView.
    void SetViewContentsWidget(QWidget* new_contents);

    // Display the "Are you sure you want to remove this file?" dialog, and if so, remove the file.
    bool ShowRemoveFileConfirmation(const std::string& message_string, const std::string& full_path);

    // Switch the current RgSourceCodeEditor to the given instance.
    bool SwitchToEditor(RgSourceCodeEditor* editor);

    // Attach the Find widget to the given view when visible, or remove if it should be hidden.
    void UpdateFindWidgetViewAttachment(QWidget* view, bool is_visible);

    // Update the text in the source editor's title bar.
    void UpdateSourceEditorTitlebar(RgSourceCodeEditor* code_editor);

    // Switch the current edit mode for the build view.
    void SwitchEditMode(EditMode mode);

    // Remove the relevant RgSourceCodeEditor from the view when a file has been closed.
    void RemoveEditor(const std::string& filename, bool switch_to_next_file = true);

    // Update the application notification message.
    virtual void UpdateApplicationNotificationMessage() = 0;

    // Remove the given input file from the RgBuildView.
    void RemoveInputFile(const std::string& input_file_full_path);

private:
    // Is the user currently allowed to change the RgBuildView's EditMode?
    bool CanSwitchEditMode();

    // Has the code in the given editor been modified since the latest build?
    bool CheckSourcesModifiedSinceLastBuild(RgSourceCodeEditor* code_editor);

    // Clear the file menu and source editors.
    void ClearBuildView();

    // Clear the map of sourcecode editors.
    void ClearEditors();

    // Create the build settings interface to edit project settings.
    void CreateBuildSettingsView();

    // Create the find widget for the source editor.
    void CreateFindWidget();

    // Create the RgBuildView's ISA disassembly view.
    void CreateIsaDisassemblyView();

    // Retrieve the structure of build outputs from the provided API-specific build output pointer.
    bool GetInputFileOutputs(std::shared_ptr<RgCliBuildOutput> build_outputs, InputFileToBuildOutputsMap& outputs) const;

    // Update the filename for an open file.
    void RenameFile(const std::string& old_file_path, const std::string& new_file_path);

    // Update the project name.
    void RenameProject(const std::string& full_path);

    // Show the ISA disassembly view table.
    bool LoadDisassemblyFromBuildOutput();

    // Handle displaying the build view when all files have been closed.
    void SwitchToFirstRemainingFile();

    // Update the correlation state for the given source file.
    void UpdateSourceFileCorrelationState(const std::string& file_path, bool is_correlated);

    // Update the FindWidget's search context using the current source code editor.
    void UpdateSourceEditorSearchContext();

    // Check if the currently open source file has been modified externally.
    void CheckExternalFileModification();

    // Handle reloading the file after external file modification.
    virtual void HandleExternalFileModification(const QFileInfo& file_info);

    // Create a project clone.
    void CreateProjectClone();

    // Open an include file in the user's app of choice (or, system default).
    bool OpenIncludeFile(const std::string& full_file_path);

    // Set the build application settings stylesheet.
    void SetBuildSettingsStylesheet(const std::string& stylesheet);

protected:
    // A map of filename to the editor used to view the file.
    std::map<std::string, RgSourceCodeEditor*> source_code_editors_;

    // The project that's edited as part of the RgBuildView.
    std::shared_ptr<RgProject> project_ = nullptr;

    // A pointer to the factory used to create API specific objects.
    std::shared_ptr<RgFactory> factory_ = nullptr;

    // A map of GPU name to the build outputs for the GPU.
    RgBuildOutputsMap build_outputs_;

    // The index of the clone being edited.
    int clone_index_;

    // The current editor mode for the RgBuildView.
    EditMode edit_mode_ = EditMode::kEmpty;

    // The time of the last successful build.
    QDateTime last_successful_build_time_;

    // The instance of the RgSourceCodeEditor being used currently.
    RgSourceCodeEditor* current_code_editor_ = nullptr;

    // The panel that gets inserted when all files have been closed.
    QWidget* empty_panel_ = nullptr;

    // The ISA disassembly view.
    RgIsaDisassemblyView* disassembly_view_ = nullptr;

    // The CLI output window.
    RgCliOutputView* cli_output_window_ = nullptr;

    // The build settings interface.
    RgBuildSettingsView* build_settings_view_ = nullptr;

    // The "Save" and "Restore Default" buttons.
    RgSettingsButtonsView* settings_buttons_view_ = nullptr;

    // The horizontal splitter responsible for dividing the RgBuildView's File Menu and Source Editor windows.
    QSplitter* file_menu_splitter_ = nullptr;

    // The horizontal splitter responsible for dividing the RgBuildView's File Menu + Source Editors and Disassembly view tables.
    RgMaximizeSplitter* disassembly_view_splitter_ = nullptr;

    // The horizontal splitter responsible for dividing the RgBuildView's File Menu + Source + Disassembly views and the Output window.
    RgMaximizeSplitter* output_splitter_ = nullptr;

    // Container for the various widgets that end up displayed as the Source View.
    QWidget* source_view_stack_ = nullptr;

    // View containers.
    RgViewContainer* source_view_container_ = nullptr;
    RgViewContainer* file_menu_view_container_ = nullptr;
    RgViewContainer* disassembly_view_container_ = nullptr;
    RgViewContainer* build_output_view_container_ = nullptr;

    // File menu titlebar.
    RgMenuTitlebar* file_menu_titlebar_ = nullptr;

    // Source editor titlebar.
    RgSourceEditorTitlebar* source_editor_titlebar_ = nullptr;

    // View manager.
    RgViewManager* view_manager_ = nullptr;

    // Map of editors to the last modification times of the underlying files.
    std::map<RgSourceCodeEditor*, QDateTime> file_modified_time_map_;

    // Tracks files that have external modifications and require a dialog.
    std::unordered_map<std::string, bool> pending_file_modifications_;  

    // A find widget used to edit source code.
    RgFindTextWidget* find_widget_ = nullptr;

    // An interface used to allow the RgFindTextWidget to search source editor code.
    RgSourceEditorSearcher* source_searcher_ = nullptr;

    // A widget to contain the build settings and the save buttons.
    RgBuildSettingsWidget* build_settings_widget_ = nullptr;

    // A handle to signal that the current build should be canceled.
    bool cancel_bulid_signal_ = false;

    // A flag indicating if the project is currently being built.
    bool is_build_in_progress_ = false;

    // The parent widget.
    QWidget* parent_ = nullptr;

    // The currently selected target GPU.
    std::string current_target_gpu_;

    // The build view UI.
    Ui::RgBuildView ui_;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_BUILD_VIEW_H_

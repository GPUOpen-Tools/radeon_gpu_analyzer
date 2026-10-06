//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for Build View class for Binary Analysis mode.
//=============================================================================

#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_BUILD_VIEW_BINARY_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_BUILD_VIEW_BINARY_H_

// C++.
#include <set>
#include <unordered_set>

// Local.
#include "source/radeon_gpu_analyzer_gui/qt/rg_build_view.h"

// Forward declarations.
class RgMenuFileItem;
class RgMenuBinary;
class RgMenuFileItemOpencl;
class RgSourceCodeTabWidget;

class RgBuildViewBinary : public RgBuildView
{
    Q_OBJECT

public:
    RgBuildViewBinary(QWidget* parent);
    virtual ~RgBuildViewBinary() = default;

    // Connect the OpenCL-specific signals.
    virtual void ConnectBuildSettingsSignals() override;

    // Connect signals for the file menu.
    virtual bool ConnectMenuSignals() override;

    // Retrieve a pointer to the RgBuildView's API-specific file menu.
    virtual RgMenu* GetMenu() const override;

    // Add the files to the file menu.
    virtual bool PopulateMenu() override;

    // Set the default focus widget for build settings view.
    void SetDefaultFocusWidget() const override;

    // Check if the given source file has been successfully disassembled.
    virtual bool IsGcnDisassemblyGenerated(const std::string& input_file_path) const override;

    // Load the session metadata file at the given path.
    virtual bool LoadSessionMetadata(const std::string& metadata_file_path, std::shared_ptr<RgCliBuildOutput>& build_output) override;

    // Show the disassembly view for the currently selected file item.
    virtual void ShowCurrentFileDisassembly() override;

    // Override BuildCurrentProject to add pre-analysis source path discovery.
    virtual void BuildCurrentProject() override;

    // Save the currently open and active file.
    virtual void SaveCurrentFile(EditMode mode) override;

    // Add a file to the file menu.
    bool AddFile(const std::string& file_full_path, bool is_new_file = false);
    
    // Retrieve the RgSourceCodeEditor instance to use for the given filename.
    virtual RgSourceCodeEditor* GetEditorForFilepath(const std::string& full_file_path, ShaderSourceLanguage lang = ShaderSourceLanguage::Unknown) override;

    // Add existing codeobj files to the given project.
    bool AddExistingCodeObjFileToProject(const std::vector<std::string>& bin_file_paths);

    // Try to find the entry point containing the given line.
    // Returns "true" if found or "false" otherwise. The name of found function is returned in "entry_name".
    bool GetEntrypointNameForLineNumber(const std::string& binary_file_path,
                                        const std::string& src_file_path,
                                        int                line_number,
                                        std::string&       entry_name) const;

    // Check if the given source editor has line correlation enabled.
    bool IsLineCorrelationEnabled(RgSourceCodeEditor* source_editor) override;

    // Check if the current API has line correlation supported.
    bool IsLineCorrelationSupported() const override;

    // Reset disassembly status of current project binaries.
    void ResetCurrentProjectBinaries();

    // Requests a save for build settings and then removes all files if accepted.
    bool RequestRemoveAllFiles() override;

    // Update the filename for an open file.
    void RenameFile(const std::string& old_file_path, const std::string& new_file_path) override;
    
    // Build a list of all source files that are currently modified.
    void GetUnsavedSourceFiles(QStringList& unsaved_source_files) override;

protected slots:
    // Handler invoked when the active file is switched within the file menu.
    virtual void HandleSelectedFileChanged(const std::string& current_file_name, const std::string& new_file_name) override;

    // Handler invoked when the user changes the selected line in the current source editor.
    virtual void HandleSourceFileSelectedLineChanged(ShaderSourceCodeViewer* editor, int line_number) override;

    // Set the project build settings border color.
    virtual void SetAPISpecificBorderColor() override;

    // Handler for current source editor changed.
    void HandleCurrentCodeEditorChanged(RgSourceCodeEditor* editor);

private slots:
    // Handler invoked when the user changes the selected entry point index for a given file.
    void HandleSelectedEntrypointChanged(const std::string& input_file_path, const std::string& selected_entrypoint_name);

    // A handler invoked when a file or files are dragged and dropped on the menu.
    void HandleExistingFileDragAndDrop(const std::vector<std::string>& file_path_to_add);

    // A handler invoked when a valid menu file item has been clicked on.
    void HandleMenuItemClicked(RgMenuFileItem* item);

    // Handler invoked when the correlated highlighted line index in the current file is updated.
    void HandleHighlightedCorrelationLineUpdated(int line_number, const std::string& src_path) override;

protected:
    // Used to implement API-specific handling of build cancellation.
    virtual void CurrentBuildCancelled() override;

    // Used to handle build success for an API-specific RgBuildView implementation.
    virtual void CurrentBuildSucceeded() override;

    // Create an API-specific file menu.
    virtual bool CreateMenu(QWidget* parent) override;

    // Reapply the stylesheet for the API-specific file menu when the color theme has changed.
    virtual void ReapplyMenuStyleSheet() override;

    // Connect API-specific RgBuildView signals to the disassembly view.
    virtual void ConnectDisassemblyViewApiSpecificSignals() override;

    // Destroy obsolete build outputs from a previous project build.
    virtual void DestroyProjectBuildArtifacts() override;

    // Remove the given input file from the RgBuildView.
    virtual void RemoveInputFile(const std::string& input_file_full_path) override;

    // Function to remove the binary file from the metadata.
    void RemoveFileFromMetadata(const std::string& full_path);

    // Set the focus to the file menu.
    virtual void FocusOnFileMenu() override;

    // Check if the given source file path already exists within the project's current clone.
    virtual bool IsSourceFileInProject(const std::string& source_file_path) const override;

    // Update the application notification message.
    virtual void UpdateApplicationNotificationMessage() override;

    // Display the "Are you sure you want to remove this file?" dialog, and if so, remove the file.
    virtual bool ShowRemoveFileConfirmation(const std::string& message_string, const std::string& full_path) override;

    // Handle graphics-mode-specific RgBuildView switching.
    virtual void HandleModeSpecificEditMode(EditMode now_mode) override;

    // Remove the relevant RgSourceCodeEditor from the view when a file has been closed.
    virtual void RemoveEditor(const std::string& filename, bool switch_to_next_file = true) override;

    // Check if the currently open source file has been modified externally.
    void CheckExternalFileModification() override {};

    // Check if the currently open binary file has been modified externally.
    void CheckExternalFileModification(const std::string& filename);

private:
    // Clear the entry point list for each file item.
    void ClearFileItemsEntrypointList();

    // Highlight the line of source code where the given named entry point is defined.
    void HighlightEntrypointStartLine(const std::string& input_file_path, const std::string& selected_entrypoint_name);

    // Populate entry point names and start line numbers in the input file.
    bool LoadEntrypointLineNumbers();

    // Create widgets for src files.
    bool CreateSourceCodeWidgets();

    // Check if the given code obj file path already exists within the project's current clone.
    bool IsCodeObjFileInProject(const std::string& bin_file_path) const;

    // Handle reloading the file after external file modification.
    void HandleExternalFileModification(const QFileInfo& file_info) override;

    // Toggle the visibility of the disassembly view.
    void ToggleDisassemblyViewKernelLabelVisiblity(RgMenuFileItemOpencl* file_item, const std::string& selected_entrypoint_name);

    // Switch the current RgSourceCodeEditor to the given instance.
    bool SwitchToTabWidget(const std::string& filename);

    // Set the target gpu label and architecture in the model.
    void SetTargetGpuLabel() override;

    // Handle displaying the build view when all files have been closed.
    void SwitchToFirstRemainingFile() override;

    // Clear the map of sourcecode editors.
    void ClearEditors() override;

    // Pre-analysis step: extract source paths from debug info and prompt for missing ones.
    // If the user provides directories, they are saved to build settings before returning.
    // Returns true if the user provided new search directories.
    // When force_prompt is true the dialog is shown even if the prompt setting is disabled
    // (used when the user explicitly clicks the hyperlink to add search paths).
    bool PreAnalysisSourceDiscovery(bool force_prompt = false);

    // Returns true if the file extension indicates a non-application source (header/include) file.
    static bool IsNonApplicationSourceTab(const std::string& file_path);

    // Get the full path of the binary currently selected in the file menu.
    std::string GetSelectedBinaryFilePath() const;

    // Returns true if the user has explicitly closed this source file's tab.
    bool IsUserClosedSourceTab(const std::string& file_path) const;

    // A map that associates an src file with a pair of the file's entry point start and end line numbers.
    std::map<std::string, EntryToSourceLineRange> entrypoint_line_numbers_;

    // A map that associates an entry point with a set of src files.
    std::map<std::string, std::set<std::string>> entrypoint_to_src_files_map_;
    
    // Binary filename to source filenames.
    std::map<std::string, std::set<std::string>> binary_to_src_files_map_;

    // Map of Binary filename to the last modification times of the underlying files.
    std::map<std::string, QDateTime> binary_file_modified_time_map_;

    // Prevents showing the source search directories prompt more than once per analysis cycle.
    bool already_prompted_for_source_dirs_ = false;

    // Source files the user has explicitly closed. Correlation clicks won't reopen these.
    std::unordered_set<std::string> user_closed_source_files_;

    // The Binary file menu.
    RgMenuBinary* file_menu_ = nullptr;

    // The instance of the RgSourceCodeTabWidget being used currently.
    RgSourceCodeTabWidget* tab_widget_ = nullptr;
};
#endif  // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_BUILD_VIEW_BINARY_H_

#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_BUILD_VIEW_BINARY_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_BUILD_VIEW_BINARY_H_

// Local.
#include "source/radeon_gpu_analyzer_gui/qt/rg_build_view.h"

// Forward declarations.
class RgMenuFileItem;
class RgMenuBinary;
class RgMenuFileItemOpencl;

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

    // Reload the content of the file modified by an external process.
    void ReloadFile(const std::string& file_path) override;

    // Save the currently open and active file.
    void SaveCurrentFile(EditMode mode) override;

    // Add a file to the file menu.
    bool AddFile(const std::string& file_full_path, bool is_new_file = false);

    // Sets the contents of the given code obj as the text in the RgSourceCodeEditor.
    void SetSourceCodeText(const std::string& file_full_path) override;

    // Add an existing source file to the given project.
    bool AddExistingSourcefileToProject(const std::string& source_file_path);

    // Add an existing codeobj file to the given project.
    bool AddExistingCodeObjFileToProject(const std::string& bin_file_path);

    // Try to find the entry point containing the given line.
    // Returns "true" if found or "false" otherwise. The name of found function is returned in "entry_name".
    bool GetEntrypointNameForLineNumber(const std::string& file_path, int line_number, std::string& entry_name) const;

    // Check if the given source editor has line correlation enabled.
    bool IsLineCorrelationEnabled(RgSourceCodeEditor* source_editor);

    // Check if the current API has line correlation supported.
    virtual bool IsLineCorrelationSupported() const override;

    // Open the build settings interface override for binary mode (since none exists).
    void OpenBuildSettings() override{};

protected slots:
    // Handler invoked when the active file is switched within the file menu.
    virtual void HandleSelectedFileChanged(const std::string& current_file_name, const std::string& new_file_name) override;

    // Handler invoked when the user changes the selected line in the current source editor.
    virtual void HandleSourceFileSelectedLineChanged(RgSourceCodeEditor* editor, int line_number) override;

    // Set the project build settings border color.
    virtual void SetAPISpecificBorderColor() override;

private slots:
    // Handler invoked when the user changes the selected entry point index for a given file.
    void HandleSelectedEntrypointChanged(const std::string& input_file_path, const std::string& selected_entrypoint_name);

    // A handler invoked when a file is dragged and dropped on the menu.
    void HandleExistingFileDragAndDrop(const std::string& file_path_to_add);

    // A handler invoked when a valid menu file item has been clicked on.
    void HandleMenuItemClicked(RgMenuFileItem* item);

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

    // Set the focus to the file menu.
    virtual void FocusOnFileMenu() override;

    // Check if the given source file path already exists within the project's current clone.
    virtual bool IsSourceFileInProject(const std::string& source_file_path) const override;

    // Update the application notification message.
    virtual void UpdateApplicationNotificationMessage() override;

private:
    // Clear the entry point list for each file item.
    void ClearFileItemsEntrypointList();

    // Find the starting line for an entry point by starting from the line number provided by "list-kernels".
    int FindEntrypointStartLine(RgSourceCodeEditor* editor, int list_kernels_start_line) const;

    // Highlight the line of source code where the given named entry point is defined.
    void HighlightEntrypointStartLine(const std::string& input_file_path, const std::string& selected_entrypoint_name);

    // Invoke the CLI to query entry point names and start line numbers in the input file.
    bool LoadEntrypointLineNumbers();

    // Check if the given code obj file path already exists within the project's current clone.
    bool IsCodeObjFileInProject(const std::string& bin_file_path) const;

    // Handle reloading the file after external file modification.
    void HandleExternalFileModification(const QFileInfo& file_info) override;

    // Toggle the visibility of the disassembly view.
    void ToggleDisassemblyViewKernelLabelVisiblity(RgMenuFileItemOpencl* file_item, const std::string& selected_entrypoint_name);

    // A map that associates an input file with a pair of the file's entry point start and end line numbers.
    std::map<std::string, EntryToSourceLineRange> entrypoint_line_numbers_;

    // The Binary file menu.
    RgMenuBinary* file_menu_ = nullptr;
};
#endif  // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_BUILD_VIEW_BINARY_H_

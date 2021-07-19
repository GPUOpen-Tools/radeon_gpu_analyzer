#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_BUILD_VIEW_VULKAN_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_BUILD_VIEW_VULKAN_H_

// Local.
#include "source/radeon_gpu_analyzer_gui/qt/rg_build_view_graphics.h"
#include "source/common/rga_shared_data_types.h"

// Forward declarations.
class RgMenuFileItem;
class RgMenuPipelineStateItem;
class RgMenuVulkan;
class RgPipelineStateView;
class RgPipelineStateModel;
class RgPipelineStateModelVulkan;

class RgBuildViewVulkan : public RgBuildViewGraphics
{
    Q_OBJECT

public:
    RgBuildViewVulkan(QWidget* parent);
    virtual ~RgBuildViewVulkan() = default;

    // Connect the Vulkan-specific signals.
    virtual void ConnectBuildSettingsSignals() override;

    // Connect signals for the file menu.
    virtual bool ConnectMenuSignals() override;

    // Retrieve a pointer to the RgBuildView's API-specific file menu.
    virtual RgMenu* GetMenu() const override;

    // Get the pipeline state model instance.
    virtual RgPipelineStateModel* GetPipelineStateModel() override;

    // Add the files to the file menu.
    virtual bool PopulateMenu() override;

    // Check if the given source file has been successfully disassembled.
    virtual bool IsGcnDisassemblyGenerated(const std::string& input_file_path) const override;

    // Load the session metadata file at the given path.
    virtual bool LoadSessionMetadata(const std::string& metadata_file_path, std::shared_ptr<RgCliBuildOutput>& build_output) override;

    // Reload the content of the file modified by an external process.
    virtual void ReloadFile(const std::string& file_path) override;

    // Show the disassembly view for the currently selected file item.
    virtual void ShowCurrentFileDisassembly() override;

    // Save the currently open and active file.
    virtual void SaveCurrentFile(EditMode mode) override;

    // Save the source file at the given path.
    virtual void SaveSourceFile(const std::string& source_file_path) override;

    // Save all unsaved files, settings, etc. before building the project.
    // Returns "true" if the user selected to proceed with the build, or "false" otherwise.
    virtual bool SaveCurrentState() override;

    // Save the file represented by the given menu file item.
    void SaveFile(RgMenuFileItemGraphics* file_menu_item);

    // Connect signals for the pipeline state view.
    void ConnectPipelineStateViewSignals();

    // Create a default graphics pipeline project.
    bool CreateDefaultGraphicsPipeline();

    // Create a default compute pipeline project.
    bool CreateDefaultComputePipeline();

    // Check if the current API has line correlation supported.
    virtual bool IsLineCorrelationSupported() const override;

    // Get the graphics pipeline menu (left panel).
    virtual RgMenuGraphics* GetGraphicsFileMenu() override;

    // Set the focus to file menu.
    virtual void FocusOnFileMenu() override;

public slots:
    // A handler invoked when the build settings button is clicked.
    void HandleBuildSettingsMenuItemClicked();

    // A handler invoked when the pipeline state button is clicked.
    void HandlePipelineStateMenuItemClicked(RgMenuPipelineStateItem* item);

    // A handler invoked when the pipeline tree is in focus.
    void HandlePipelineStateTreeFocusIn();

    // A handler invoked when the pipeline tree is out of focus.
    void HandlePipelineStateTreeFocusOut();

    // A handler invoked when the list widget status is changed.
    void HandleEnumListWidgetStatus(bool is_open);

signals:
    // A signal to indicate if the pipeline state option should be enabled.
    void EnablePipelineMenuItem(bool is_enabled);

    // A signal to indicate if the build settings option should be enabled.
    void EnableBuildSettingsMenuItem(bool is_enabled);

    // A signal to indicate if the Ctrl+S menu option should be enabled.
    void EnableSaveSettingsMenuItem(bool is_enabled);

protected slots:
    // Handler invoked when a file within the file menu has been renamed.
    virtual void HandleFileRenamed(const std::string& old_file_path, const std::string& new_file_path) override;

    // Handler invoked when the active file is switched within the file menu.
    virtual void HandleSelectedFileChanged(const std::string& current_file_name, const std::string& new_file_name) override;

    // Handler invoked when the user changes the selected line in the current source editor.
    virtual void HandleSourceFileSelectedLineChanged(RgSourceCodeEditor* editor, int line_number) override;

    // A handler invoked when the pipeline state file should be saved.
    virtual bool HandlePipelineStateFileSaved() override;

    // A handler invoked when a pipeline state file should be loaded.
    virtual void HandlePipelineStateFileLoaded() override;

    // Set the project build settings border color.
    virtual void SetAPISpecificBorderColor() override;

    // Set the default widget for build settings view.
    void SetDefaultFocusWidget() const override;

    // Set the default widget for PSO Editor view.
    // This is the widget in the PSO Editor that will
    // have the focus when the PSO Editor gets the focus.
    void SetPsoEditorDefaultFocusWidget() const;

private slots:
    // A handler invoked when the user chooses to add existing file.
    void HandleAddFileButtonClicked(RgPipelineStage stage);

    // The handler that is being invoked when the user asks to add an existing file.
    void HandleAddExistingFile();

    // A handler invoked when the user chooses to create a new file.
    void HandleCreateNewFile();

    // A handler invoked when a stage item's Remove button is clicked.
    void HandleRemoveFileButtonClicked(RgPipelineStage stage);

    // A handler invoked when a stage item's "Restore original SPIR-V binary" button is clicked.
    void HandleRestoreOriginalSpvClicked(RgPipelineStage stage);

    // A handler invoked when a file is dragged and dropped on a stage item.
    void HandleExistingFileDragAndDrop(RgPipelineStage stage, const std::string& file_path_to_add);

    // A handler invoked when a valid menu file item has been clicked on.
    void HandleMenuItemClicked(RgMenuFileItem* item);

protected:
    // Used to implement API-specific handling of build cancellation.
    virtual void CurrentBuildCancelled() override;

    // Create an API-specific file menu.
    virtual bool CreateMenu(QWidget* parent) override;

    // Connect API-specific RgBuildView signals to the disassembly view.
    virtual void ConnectDisassemblyViewApiSpecificSignals() override;

    // Initialize views specific to the current mode only.
    virtual void InitializeModeSpecificViews() override;

    // Check if the given source editor has line correlation enabled.
    virtual bool IsLineCorrelationEnabled(RgSourceCodeEditor* source_editor) override;

    // Check if the given source file path already exists within the project's current clone.
    virtual bool IsSourceFileInProject(const std::string& source_file_path) const override;

    // Load the pipeline state file from the given file path.
    virtual bool LoadPipelineStateFile(const std::string& pipeline_state_file_path) override;

    // Display the "Are you sure you want to revert to original SPIR-V binary?" dialog.
    bool ShowRevertToSpvBinaryConfirmation(const std::string& file_path);

    // Replace the input file path in the Build Output data.
    // This function should be used when replacing the shader SPIR-V file with its disassembly and vice versa.
    bool ReplaceInputFileInBuildOutput(const std::string& old_file_path, const std::string& new_file_path);

    // Update the application notification message.
    virtual void UpdateApplicationNotificationMessage() override;

private:
    // Create a new empty source file within the existing project.
    // Returns true if the file was actually created, and false otherwise.
    bool CreateNewSourceFile(RgPipelineStage stage, const std::string& source_file_name, std::string& full_source_file_path);

    // Create the pipeline project.
    bool CreateProject(RgPipelineType pipeline_type);

    // Create the pipeline state create info.
    void CreatePipelineStateFile();

    // Create the pipeline state model instance.
    void CreatePipelineStateModel();

    // Create the pipeline state editor interface.
    void CreatePipelineStateView(QWidget* parent);

    // Add a file to the project.
    void SetStageSourceFile(RgPipelineStage stage, const std::string& filename);

    // Disassemble SPIR-V binary file.
    // Returns "true" if succeeded or "false" otherwise.
    // The name (full path) of output SPIR-V disassembly file is returned in "spv_disasm_file".
    bool DisasmSpvFile(const std::string& spv_file, std::string& spv_disasm_file);

    // The Vulkan file menu.
    RgMenuVulkan* file_menu_ = nullptr;

    // This member stores the most recent stage whose item was clicked.
    // We use this to know where to place the Add/Create file context menu.
    RgPipelineStage stage_clicked_;

    // The pipeline state settings model.
    RgPipelineStateModelVulkan* pipeline_state_model_ = nullptr;

    // Paths to files containing disassembled SPIR-V text for SPIR-V binary files added to the project.
    ShaderInputFileArray spv_disasm_files_;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_BUILD_VIEW_VULKAN_H_

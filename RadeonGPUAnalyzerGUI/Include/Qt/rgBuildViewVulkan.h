#pragma once

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgBuildViewGraphics.h>
#include <Utils/Include/rgaSharedDataTypes.h>

// Forward declarations.
class rgMenuFileItem;
class rgMenuPipelineStateItem;
class rgMenuVulkan;
class rgPipelineStateView;
class rgPipelineStateModel;
class rgPipelineStateModelVulkan;

class rgBuildViewVulkan : public rgBuildViewGraphics
{
    Q_OBJECT

public:
    rgBuildViewVulkan(QWidget* pParent);
    virtual ~rgBuildViewVulkan() = default;

    // Connect the Vulkan-specific signals.
    virtual void ConnectBuildSettingsSignals() override;

    // Connect signals for the file menu.
    virtual bool ConnectMenuSignals() override;

    // Retrieve a pointer to the rgBuildView's API-specific file menu.
    virtual rgMenu* GetMenu() const override;

    // Get the pipeline state model instance.
    virtual rgPipelineStateModel* GetPipelineStateModel() override;

    // Add the files to the file menu.
    virtual bool PopulateMenu() override;

    // Check if the given source file has been successfully disassembled.
    virtual bool IsGcnDisassemblyGenerated(const std::string& inputFilePath) const override;

    // Load the session metadata file at the given path.
    virtual bool LoadSessionMetadata(const std::string& metadataFilePath, std::shared_ptr<rgCliBuildOutput>& pBuildOutput) override;

    // Reload the content of the file modified by an external process.
    virtual void ReloadFile(const std::string& filePath) override;

    // Show the disassembly view for the currently selected file item.
    virtual void ShowCurrentFileDisassembly() override;

    // Save the currently open and active file.
    virtual void SaveCurrentFile() override;

    // Save the source file at the given path.
    virtual void SaveSourceFile(const std::string& sourceFilePath) override;

    // Save all unsaved files, settings, etc. before building the project.
    // Returns "true" if the user selected to proceed with the build, or "false" otherwise.
    virtual bool SaveCurrentState() override;

    // Save the file represented by the given menu file item.
    void SaveFile(rgMenuFileItemGraphics* pFileMenuItem);

    // Connect signals for the pipeline state view.
    void ConnectPipelineStateViewSignals();

    // Create a default graphics pipeline project.
    bool CreateDefaultGraphicsPipeline();

    // Create a default compute pipeline project.
    bool CreateDefaultComputePipeline();

    // Check if the current API has line correlation supported.
    virtual bool IsLineCorrelationSupported() const override;

    // Get the graphics pipeline menu (left panel).
    virtual rgMenuGraphics* GetGraphicsFileMenu() override;

    // Set the focus to file menu.
    virtual void FocusOnFileMenu() override;

public slots:
    // A handler invoked when the pipeline state button is clicked.
    void HandlePipelineStateMenuItemClicked(rgMenuPipelineStateItem* pItem);

    // A handler invoked when the pipeline tree is in focus.
    void HandlePipelineStateTreeFocusIn();

    // A handler invoked when the pipeline tree is out of focus.
    void HandlePipelineStateTreeFocusOut();

    // A handler invoked when the list widget status is changed.
    void HandleEnumListWidgetStatus(bool isOpen);

protected slots:
    // Handler invoked when a file within the file menu has been renamed.
    virtual void HandleFileRenamed(const std::string& oldFilepath, const std::string& newFilepath) override;

    // Handler invoked when the active file is switched within the file menu.
    virtual void HandleSelectedFileChanged(const std::string& currentFilename, const std::string& newFilename) override;

    // Handler invoked when the user changes the selected line in the current source editor.
    virtual void HandleSourceFileSelectedLineChanged(rgSourceCodeEditor* pEditor, int lineNumber) override;

    // A handler invoked when the pipeline state file should be saved.
    virtual void HandlePipelineStateFileSaved() override;

    // A handler invoked when a pipeline state file should be loaded.
    virtual void HandlePipelineStateFileLoaded() override;

    // Set the project build settings border color.
    virtual void SetAPISpecificBorderColor() override;

    // Set the default widget for build settings view.
    void SetDefaultFocusWidget() const override;

    // Set the default widget for PSO Editor view.
    // This is the widget in the PSO Editor that will
    // have the focus when the PSO Editor gets the focus.
    void SetPSOEditorDefaultFocusWidget() const;

private slots:
    // A handler invoked when the user chooses to add existing file.
    void HandleAddFileButtonClicked(rgPipelineStage stage);

    // The handler that is being invoked when the user asks to add an existing file.
    void HandleAddExistingFile();

    // A handler invoked when the user chooses to create a new file.
    void HandleCreateNewFile();

    // A handler invoked when a stage item's Remove button is clicked.
    void HandleRemoveFileButtonClicked(rgPipelineStage stage);

    // A handler invoked when a stage item's "Restore original SPIR-V binary" button is clicked.
    void HandleRestoreOriginalSpvClicked(rgPipelineStage stage);

    // A handler invoked when a file is dragged and dropped on a stage item.
    void HandleExistingFileDragAndDrop(rgPipelineStage stage, const std::string& filePathToAdd);

    // A handler invoked when a valid menu file item has been clicked on.
    void HandleMenuItemClicked(rgMenuFileItem* pItem);

protected:
    // Create an API-specific file menu.
    virtual bool CreateMenu(QWidget* pParent) override;

    // Connect API-specific rgBuildView signals to the disassembly view.
    virtual void ConnectDisassemblyViewApiSpecificSignals() override;

    // Initialize views specific to the current mode only.
    virtual void InitializeModeSpecificViews() override;

    // Check if the given source editor has line correlation enabled.
    virtual bool IsLineCorrelationEnabled(rgSourceCodeEditor* pSourceEditor) override;

    // Check if the given source file path already exists within the project's current clone.
    virtual bool IsSourceFileInProject(const std::string& sourceFilePath) const override;

    // Load the pipeline state file from the given file path.
    virtual bool LoadPipelineStateFile(const std::string& pipelineStateFilePath) override;

    // Display the "Are you sure you want to revert to original SPIR-V binary?" dialog.
    bool ShowRevertToSpvBinaryConfirmation(const std::string& filePath);

    // Replace the input file path in the Build Output data.
    // This function should be used when replacing the shader SPIR-V file with its disassembly and vice versa.
    bool ReplaceInputFileInBuildOutput(const std::string& oldFilePath, const std::string& newFilePath);

    // Update the application notification message.
    virtual void UpdateApplicationNotificationMessage() override;

private:
    // Create a new empty source file within the existing project.
    // Returns true if the file was actually created, and false otherwise.
    bool CreateNewSourceFile(rgPipelineStage stage, const std::string& sourceFileName, std::string& fullSourceFilepath);

    // Create the pipeline project.
    bool CreateProject(rgPipelineType pipelineType);

    // Create the pipeline state create info.
    void CreatePipelineStateFile();

    // Create the pipeline state model instance.
    void CreatePipelineStateModel();

    // Create the pipeline state editor interface.
    void CreatePipelineStateView(QWidget* pParent);

    // Add a file to the project.
    void SetStageSourceFile(rgPipelineStage stage, const std::string& fileName);

    // Disassemble SPIR-V binary file.
    // Returns "true" if succeeded or "false" otherwise.
    // The name (full path) of output SPIR-V disassembly file is returned in "spvDisasmFile".
    bool DisasmSpvFile(const std::string& spvFile, std::string& spvDisasmFile);

    // The Vulkan file menu.
    rgMenuVulkan* m_pFileMenu = nullptr;

    // This member stores the most recent stage whose item was clicked.
    // We use this to know where to place the Add/Create file context menu.
    rgPipelineStage m_stageClicked;

    // The pipeline state settings model.
    rgPipelineStateModelVulkan* m_pPipelineStateModel = nullptr;

    // Paths to files containing disassembled SPIR-V text for SPIR-V binary files added to the project.
    ShaderInputFileArray m_spvDisasmFiles;
};

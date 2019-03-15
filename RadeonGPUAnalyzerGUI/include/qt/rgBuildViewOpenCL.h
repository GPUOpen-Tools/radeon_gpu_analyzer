#pragma once

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgBuildView.h>

// Forward declarations.
class rgMenuFileItem;
class rgMenuOpenCL;

class rgBuildViewOpenCL : public rgBuildView
{
    Q_OBJECT

public:
    rgBuildViewOpenCL(QWidget* pParent);
    virtual ~rgBuildViewOpenCL() = default;

    // Connect the OpenCL-specific signals.
    virtual void ConnectBuildSettingsSignals() override;

    // Connect signals for the file menu.
    virtual bool ConnectMenuSignals() override;

    // Retrieve a pointer to the rgBuildView's API-specific file menu.
    virtual rgMenu* GetMenu() const override;

    // Add the files to the file menu.
    virtual bool PopulateMenu() override;

    // Check if the given source file has been successfully disassembled.
    virtual bool IsGcnDisassemblyGenerated(const std::string& inputFilePath) const override;

    // Load the session metadata file at the given path.
    virtual bool LoadSessionMetadata(const std::string& metadataFilePath, std::shared_ptr<rgCliBuildOutput>& pBuildOutput) override;

    // Show the disassembly view for the currently selected file item.
    virtual void ShowCurrentFileDisassembly() override;

    // Add a file to the file menu.
    bool AddFile(const std::string& fileFullPath, bool isNewFile = false);

    // Create a new empty source file in the existing project.
    // Returns true if the file was actually created, and false otherwise.
    bool CreateNewSourceFileInProject();

    // Create a new empty source file within the existing project.
    // Returns true if the file was actually created, and false otherwise.
    bool CreateNewSourceFile(const std::string& sourceFileName, std::string& fullSourceFilepath);

    // Add an existing source file to the given project.
    bool AddExistingSourcefileToProject(const std::string& sourceFilepath);

    // Try to find the entry point containing the given line.
    // Returns "true" if found or "false" otherwise. The name of found function is returned in "entryName".
    bool GetEntrypointNameForLineNumber(const std::string& filePath, int lineNumber, std::string& entryName) const;

    // Check if the current API has line correlation supported.
    virtual bool IsLineCorrelationSupported() const override;

protected slots:
    // Handler invoked when the active file is switched within the file menu.
    virtual void HandleSelectedFileChanged(const std::string& currentFilename, const std::string& newFilename) override;

    // Handler invoked when the user changes the selected line in the current source editor.
    virtual void HandleSourceFileSelectedLineChanged(rgSourceCodeEditor* pEditor, int lineNumber) override;

    // Set the project build settings border color.
    virtual void SetAPISpecificBorderColor() override;

private slots:
    // Handler invoked when the user changes the selected entry point index for a given file.
    void HandleSelectedEntrypointChanged(const std::string& inputFilePath, const std::string& selectedEntrypointName);

    // A handler invoked when a valid menu file item has been clicked on.
    void HandleMenuItemClicked(rgMenuFileItem* pItem);

protected:
    // Used to implement API-specific handling of build cancellation.
    virtual void CurrentBuildCancelled() override;

    // Used to handle build success for an API-specific rgBuildView implementation.
    virtual void CurrentBuildSucceeded() override;

    // Create an API-specific file menu.
    virtual bool CreateMenu(QWidget* pParent) override;

    // Connect API-specific rgBuildView signals to the disassembly view.
    virtual void ConnectDisassemblyViewApiSpecificSignals() override;

    // Destroy obsolete build outputs from a previous project build.
    virtual void DestroyProjectBuildArtifacts() override;

    // Set the focus to the file menu.
    virtual void FocusOnFileMenu() override;

    // Check if the given source file path already exists within the project's current clone.
    virtual bool IsSourceFileInProject(const std::string& sourceFilePath) const override;

    // Update the application notification message.
    virtual void UpdateApplicationNotificationMessage() override;

private:
    // Clear the entry point list for each file item.
    void ClearFileItemsEntrypointList();

    // Find the starting line for an entry point by starting from the line number provided by "list-kernels".
    int FindEntrypointStartLine(rgSourceCodeEditor* pEditor, int listKernelsStartLine) const;

    // Highlight the line of source code where the given named entry point is defined.
    void HighlightEntrypointStartLine(const std::string& inputFilePath, const std::string& selectedEntrypointName);

    // Invoke the CLI to query entry point names and start line numbers in the input file.
    bool LoadEntrypointLineNumbers();

    // A map that associates an input file with a pair of the file's entry point start and end line numbers.
    std::map<std::string, EntryToSourceLineRange> m_entrypointLineNumbers;

    // The OpenCL file menu.
    rgMenuOpenCL* m_pFileMenu = nullptr;
};
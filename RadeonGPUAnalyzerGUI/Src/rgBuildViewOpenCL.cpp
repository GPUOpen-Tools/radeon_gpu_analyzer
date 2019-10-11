// C++.
#include <cassert>
#include <sstream>
#include <thread>

// Qt.
#include <QWidget>
#include <QTextStream>

// Infra.
#include <QtCommon/Scaling/ScalingManager.h>
#include <RGA/Utils/Include/rgaSharedUtils.h>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgBuildSettingsViewOpenCL.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgBuildSettingsWidget.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgBuildViewOpenCL.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgCliOutputView.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgIsaDisassemblyView.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMenuBuildSettingsItem.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMenuFileItemOpenCL.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMenuOpenCL.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMenuTitlebar.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgRenameProjectDialog.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgSourceCodeEditor.h>
#include <RadeonGPUAnalyzerGUI/Include/rgCliLauncher.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDefinitions.h>
#include <RadeonGPUAnalyzerGUI/Include/rgFactoryOpenCL.h>
#include <RadeonGPUAnalyzerGUI/Include/rgStringConstants.h>
#include <RadeonGPUAnalyzerGUI/Include/rgUtils.h>
#include <RadeonGPUAnalyzerGUI/Include/rgXMLSessionConfig.h>

rgBuildViewOpenCL::rgBuildViewOpenCL(QWidget* pParent)
    : rgBuildView(rgProjectAPI::OpenCL, pParent)
{
}

void rgBuildViewOpenCL::ConnectBuildSettingsSignals()
{
    // Connect the parent's signals.
    rgBuildView::ConnectBuildSettingsSignals();

    rgBuildSettingsViewOpenCL* pBuildSettingsViewOpenCL = static_cast<rgBuildSettingsViewOpenCL*>(m_pBuildSettingsView);
    assert(pBuildSettingsViewOpenCL != nullptr);

    if (pBuildSettingsViewOpenCL != nullptr)
    {
        bool isConnected = connect(pBuildSettingsViewOpenCL, &rgBuildSettingsViewOpenCL::PendingChangesStateChanged, this, &rgBuildView::HandleBuildSettingsPendingChangesStateChanged);
        assert(isConnected);

        isConnected = connect(pBuildSettingsViewOpenCL, &rgBuildSettingsViewOpenCL::ProjectBuildSettingsSaved, this, &rgBuildView::HandleBuildSettingsSaved);
        assert(isConnected);

        // Connect to build settings view's edit line's "focus in" event to color the frame green.
        isConnected = connect(pBuildSettingsViewOpenCL, &rgBuildSettingsViewOpenCL::SetFrameBorderGreenSignal, this, &rgBuildView::HandleSetFrameBorderGreen);
        assert(isConnected);

        // Connect to build settings view's edit line's "focus out" event to color the frame black.
        isConnected = connect(pBuildSettingsViewOpenCL, &rgBuildSettingsViewOpenCL::SetFrameBorderBlackSignal, this, &rgBuildView::HandleSetFrameBorderBlack);
        assert(isConnected);
    }
}

bool rgBuildViewOpenCL::ConnectMenuSignals()
{
    bool isConnected = false;

    // Connect the file menu's file item entry point changed signal.
    assert(m_pFileMenu != nullptr);
    if (m_pFileMenu != nullptr)
    {
        rgMenuOpenCL* pMenuOpenCL = static_cast<rgMenuOpenCL*>(m_pFileMenu);
        assert(pMenuOpenCL != nullptr);
        if (pMenuOpenCL != nullptr)
        {
            // Connect the OpenCL menu's "Selected entry point changed" handler.
            isConnected = connect(pMenuOpenCL, &rgMenuOpenCL::SelectedEntrypointChanged, this, &rgBuildViewOpenCL::HandleSelectedEntrypointChanged);
            assert(isConnected);

            // Connect the rgBuildView's entry point changed signal to the file menu's handler.
            isConnected =  connect(this, &rgBuildViewOpenCL::SelectedEntrypointChanged, pMenuOpenCL, &rgMenuOpenCL::HandleSelectedEntrypointChanged);
            assert(isConnected);

            // Connect the file menu item selection handler for each new item.
            isConnected = connect(pMenuOpenCL, &rgMenuOpenCL::MenuItemClicked, this, &rgBuildViewOpenCL::HandleMenuItemClicked);
            assert(isConnected);

            // Notify the file menu that a new source file has been added.
            isConnected = connect(this, &rgBuildViewOpenCL::AddedSourceFileToProject, m_pFileMenu, &rgMenuOpenCL::HandleSourceFileAdded);
            assert(isConnected);
        }
    }

    return isConnected;
}

void rgBuildViewOpenCL::CurrentBuildCancelled()
{
    assert(m_pFileMenu != nullptr);
    if (m_pFileMenu != nullptr)
    {
        rgMenuOpenCL* pMenuOpenCL = static_cast<rgMenuOpenCL*>(m_pFileMenu);
        assert(pMenuOpenCL != nullptr);

        if (pMenuOpenCL != nullptr)
        {
            // Don't allow the user to expand file item's entry point list.
            pMenuOpenCL->SetIsShowEntrypointListEnabled(false);
        }
    }
}

void rgBuildViewOpenCL::CurrentBuildSucceeded()
{
    // Invoke the CLI to load the start line numbers for each entrypoint.
    LoadEntrypointLineNumbers();

    assert(m_pFileMenu != nullptr);
    if (m_pFileMenu != nullptr)
    {
        rgMenuOpenCL* pMenuOpenCL = static_cast<rgMenuOpenCL*>(m_pFileMenu);
        assert(pMenuOpenCL != nullptr);
        if (pMenuOpenCL != nullptr)
        {
            // Allow the user to expand the file's entry point list.
            pMenuOpenCL->SetIsShowEntrypointListEnabled(true);
        }
    }

    // Update the file menu item with the clone's build output.
    rgMenuOpenCL* pMenuOpenCL = static_cast<rgMenuOpenCL*>(m_pFileMenu);
    pMenuOpenCL->UpdateBuildOutput(m_buildOutputs);

    // Update the correlation state for each source code editor based on which source
    // files were built successfully.
    std::string outputGpu;
    std::shared_ptr<rgCliBuildOutput> pBuildOutput = nullptr;
    bool isOutputValid = rgUtils::GetFirstValidOutputGpu(m_buildOutputs, outputGpu, pBuildOutput);
    if (isOutputValid && pBuildOutput != nullptr)
    {
        // Store the path to the current source file using the file menu.
        std::string currentSourceFilePath;
        rgMenu* pMenu = GetMenu();
        assert(pMenu != nullptr);
        if (pMenu != nullptr)
        {
            currentSourceFilePath = pMenu->GetSelectedFilePath();
        }

        // Enable line correlation for all source files that were built successfully.
        std::shared_ptr<rgCliBuildOutputOpenCL> pBuildOutputOpenCL =
            std::dynamic_pointer_cast<rgCliBuildOutputOpenCL>(pBuildOutput);

        assert(pBuildOutputOpenCL != nullptr);
        if (pBuildOutputOpenCL != nullptr)
        {
            auto sourcePathIterStart = pBuildOutputOpenCL->m_perFileOutput.begin();
            auto sourcePathIterEnd = pBuildOutputOpenCL->m_perFileOutput.end();
            for (auto inputFilePathIter = sourcePathIterStart; inputFilePathIter != sourcePathIterEnd; ++inputFilePathIter)
            {
                // Get a pointer to the source editor for each input file.
                const std::string& sourceFilePath = inputFilePathIter->first;

                // Skip updating the current file within the loop. It  will be updated last.
                if (currentSourceFilePath.compare(sourceFilePath) != 0)
                {
                    // Only update the correlation if the source file still exists in the project.
                    // Previously-built files that have already been removed from the project may
                    // have artifacts loaded.
                    auto editorIter = m_sourceCodeEditors.find(sourceFilePath);
                    if (editorIter != m_sourceCodeEditors.end())
                    {
                        rgSourceCodeEditor* pEditor = GetEditorForFilepath(sourceFilePath);

                        // Emit the signal used to update the correlation enabledness.
                        emit LineCorrelationEnabledStateChanged(pEditor, true);
                    }
                }
            }

            // Update the currently selected file last.
            rgSourceCodeEditor* pEditor = GetEditorForFilepath(currentSourceFilePath);
            if (pEditor != nullptr)
            {
                // Emit the signal used to update the correlation enabledness.
                emit LineCorrelationEnabledStateChanged(pEditor, true);
            }
        }
    }
}

bool rgBuildViewOpenCL::CreateMenu(QWidget* pParent)
{
    m_pFileMenu = static_cast<rgMenuOpenCL*>(m_pFactory->CreateFileMenu(pParent));

    // Notify the file menu when the build succeeded.
    bool isConnected = connect(this, &rgBuildViewOpenCL::ProjectBuildSuccess, m_pFileMenu, &rgMenuOpenCL::ProjectBuildSuccess);
    assert(isConnected);

    // Notify the file menu to update file menu item coloring
    // when an already built project is being loaded.
    isConnected = connect(this, &rgBuildViewOpenCL::UpdateFileColoring, m_pFileMenu, &rgMenuOpenCL::ProjectBuildSuccess);
    assert(isConnected);

    // Register the file menu with scaling manager.
    assert(m_pFileMenu != nullptr);
    if (m_pFileMenu != nullptr)
    {
        ScalingManager::Get().RegisterObject(m_pFileMenu);
    }

    return m_pFileMenu != nullptr;
}

void rgBuildViewOpenCL::ConnectDisassemblyViewApiSpecificSignals()
{
    assert(m_pDisassemblyView != nullptr);
    if (m_pDisassemblyView != nullptr)
    {
        // Connect the handler invoked when the user changes the selected entrypoint.
        bool isConnected = connect(this, &rgBuildViewOpenCL::SelectedEntrypointChanged,
            m_pDisassemblyView, &rgIsaDisassemblyView::HandleSelectedEntrypointChanged);
        assert(isConnected);

        // Connect the rgIsaDisassemblyView's entry point changed handler.
        isConnected = connect(m_pDisassemblyView, &rgIsaDisassemblyView::SelectedEntrypointChanged,
            this, &rgBuildViewOpenCL::HandleSelectedEntrypointChanged);
        assert(isConnected);
    }
}

void rgBuildViewOpenCL::DestroyProjectBuildArtifacts()
{
    // Invoke the base implementation used to destroy project build artifacts.
    rgBuildView::DestroyProjectBuildArtifacts();

    // Clear any old build artifacts from the OpenCL-specific file menu items.
    ClearFileItemsEntrypointList();
}

bool rgBuildViewOpenCL::IsSourceFileInProject(const std::string& sourceFilePath) const
{
    bool res = false;

    std::vector<std::string> sourceFiles;
    rgConfigManager::Instance().GetProjectSourceFilePaths(m_pProject, m_cloneIndex, sourceFiles);

    for (std::vector<std::string>::iterator iter = sourceFiles.begin(); iter != sourceFiles.end(); ++iter)
    {
        if (rgaSharedUtils::ComparePaths(sourceFilePath, *iter))
        {
            res = true;
            break;
        }
    }

    return res;
}

rgMenu* rgBuildViewOpenCL::GetMenu() const
{
    return m_pFileMenu;
}

void rgBuildViewOpenCL::FocusOnFileMenu()
{
    // Switch the focus to the file menu.
    if (m_pFileMenu != nullptr)
    {
        m_pFileMenu->setFocus();
    }
}

bool rgBuildViewOpenCL::PopulateMenu()
{
    bool ret = false;

    // Fill up the file path list with the paths corrected by the user.
    std::vector<std::string> sourceFilePaths;
    rgConfigManager::Instance().GetProjectSourceFilePaths(m_pProject, m_cloneIndex, sourceFilePaths);
    if (!sourceFilePaths.empty())
    {
        // Add all the project's source files into the rgBuildView.
        for (int fileIndex = 0; fileIndex < sourceFilePaths.size(); ++fileIndex)
        {
            const std::string& filePath = sourceFilePaths[fileIndex];

            // Check that the file still exists before attempting to load it.
            bool isFileExists = rgUtils::IsFileExists(filePath);
            assert(isFileExists);
            if (isFileExists)
            {
                // Add the selected file to the menu.
                if (AddFile(filePath))
                {
                    // Set the source code view text with the contents of the selected file.
                    SetSourceCodeText(filePath);

                    // The rgBuildView was successfully populated with the current project.
                    ret = true;
                }
            }
            else
            {
                // Build an error string saying the file couldn't be found on disk.
                std::stringstream errorString;
                errorString << STR_ERR_CANNOT_LOAD_SOURCE_FILE_MSG;
                errorString << filePath;

                // Show the user the error message.
                rgUtils::ShowErrorMessageBox(errorString.str().c_str(), this);
            }
        }

        // Select the first available file.
        if (ret)
        {
            m_pFileMenu->SelectFirstItem();
        }
    }
    else
    {
        // It's OK if the project being loaded doesn't include any source files, so return true.
        ret = true;
    }

    return ret;
}

bool rgBuildViewOpenCL::IsGcnDisassemblyGenerated(const std::string& inputFilePath) const
{
    bool isCurrentFileDisassembled = false;

    auto targetGpuOutputsIter = m_buildOutputs.find(m_currentTargetGpu);
    if (targetGpuOutputsIter != m_buildOutputs.end())
    {
        std::shared_ptr<rgCliBuildOutputOpenCL> pBuildOutput =
            std::dynamic_pointer_cast<rgCliBuildOutputOpenCL>(targetGpuOutputsIter->second);

        assert(pBuildOutput != nullptr);
        if (pBuildOutput != nullptr)
        {
            auto inputFileOutputsIter = pBuildOutput->m_perFileOutput.find(inputFilePath);
            if (inputFileOutputsIter != pBuildOutput->m_perFileOutput.end())
            {
                rgFileOutputs& fileOutputs = inputFileOutputsIter->second;
                isCurrentFileDisassembled = !fileOutputs.m_outputs.empty();
            }
        }
    }

    return isCurrentFileDisassembled;
}

bool rgBuildViewOpenCL::LoadSessionMetadata(const std::string& metadataFilePath, std::shared_ptr<rgCliBuildOutput>& pBuildOutput)
{
    bool ret = false;

    std::shared_ptr<rgCliBuildOutputOpenCL> pGpuOutputOpenCL = nullptr;
    ret = rgXMLSessionConfig::ReadSessionMetadataOpenCL(metadataFilePath, pGpuOutputOpenCL);
    pBuildOutput = pGpuOutputOpenCL;

    return ret;
}

void rgBuildViewOpenCL::ShowCurrentFileDisassembly()
{
    bool isCurrentFileDisassembled = false;

    // Show the currently selected file's first entry point disassembly (if there is no currently selected entry).
    const std::string& inputFilepath = m_pFileMenu->GetSelectedFilePath();
    rgMenuFileItem* pSelectedFileItem = m_pFileMenu->GetSelectedFileItem();
    assert(pSelectedFileItem != nullptr);

    if (pSelectedFileItem != nullptr)
    {
        rgMenuFileItemOpenCL* pOpenCLFileItem = static_cast<rgMenuFileItemOpenCL*>(pSelectedFileItem);
        assert(pOpenCLFileItem != nullptr);

        if (pOpenCLFileItem != nullptr)
        {
            std::string currentEntrypointName;
            bool isEntrySelected = pOpenCLFileItem->GetSelectedEntrypointName(currentEntrypointName);

            // Get the list of entry point names for the selected input file.
            std::vector<std::string> entrypointNames;
            pOpenCLFileItem->GetEntrypointNames(entrypointNames);

            // Select the first available entry point if any exist.
            if (!entrypointNames.empty())
            {
                // Show the first entry point in the disassembly table.
                std::string& entrypointName = (isEntrySelected ? currentEntrypointName : entrypointNames[0]);
                m_pDisassemblyView->HandleSelectedEntrypointChanged(m_currentTargetGpu, inputFilepath, entrypointName);

                // Emit a signal indicating that the selected entry point has changed.
                emit SelectedEntrypointChanged(m_currentTargetGpu, inputFilepath, entrypointName);

                isCurrentFileDisassembled = true;
            }
        }
    }

    // Toggle the view based on if the current file has been disassembled or not.
    ToggleDisassemblyViewVisibility(isCurrentFileDisassembled);
}

bool rgBuildViewOpenCL::AddFile(const std::string& fileFullPath, bool isNewFile)
{
    bool wasAdded = false;
    if (m_pFileMenu)
    {
        rgMenuOpenCL* pFileMenu = nullptr;
        pFileMenu = static_cast<rgMenuOpenCL*>(m_pFileMenu);
        if (pFileMenu != nullptr)
        {
            wasAdded = pFileMenu->AddItem(fileFullPath, isNewFile);
        }
    }

    return wasAdded;
}

bool rgBuildViewOpenCL::CreateNewSourceFileInProject()
{
    // Generate a new empty source file in the correct location.
    std::string sourceFilename = STR_DEFAULT_SOURCE_FILENAME;

    std::string fullSourcefilePath;
    bool ret = CreateNewSourceFile(sourceFilename, fullSourcefilePath);

    if (ret)
    {
        // This should be a full filename, but new files don't have that.
        if (AddFile(fullSourcefilePath, true))
        {
            // Display the file's source code.
            SetSourceCodeText(fullSourcefilePath);
        }
    }

    return ret;
}

void rgBuildViewOpenCL::SetDefaultFocusWidget() const
{
    assert(m_pBuildSettingsView != nullptr);
    if (m_pBuildSettingsView != nullptr)
    {
        m_pBuildSettingsView->SetInitialWidgetFocus();
    }
}

bool rgBuildViewOpenCL::CreateNewSourceFile(const std::string& sourceFileName, std::string& fullSourceFilePath)
{
    bool ret = false;

    // True if a new project was created.
    bool wasProjectCreated = false;

    // True if we are creating a new file in an existing project.
    bool isExistingProject = (m_pProject != nullptr);

    if (!isExistingProject)
    {
        wasProjectCreated = CreateNewEmptyProject();

        if (wasProjectCreated)
        {
            emit ProjectCreated();
        }
    }

    if (isExistingProject || wasProjectCreated)
    {
        std::string newFilename;
        std::string newFileExtension;

        // Generate a path to where the new empty file will live in the projects directory.
        bool gotExtension = rgUtils::ProjectAPIToSourceFileExtension(m_pProject->m_api, newFileExtension);
        assert(gotExtension);
        if (gotExtension)
        {
            rgConfigManager::GenerateNewSourceFilepath(m_pProject->m_projectName, m_cloneIndex, sourceFileName, newFileExtension, newFilename, fullSourceFilePath);

            // Ensure that the folder where the file will be saved already exists.
            std::string sourcefileFolder;
            rgUtils::ExtractFileDirectory(fullSourceFilePath, sourcefileFolder);
            if (!rgUtils::IsDirExists(sourcefileFolder))
            {
                bool isDirCreated = rgUtils::CreateFolder(sourcefileFolder);
                assert(isDirCreated);
            }

            // Create a new file at the target location.
            QFile emptyFile(fullSourceFilePath.c_str());
            emptyFile.open(QIODevice::ReadWrite);

            // Add the template source code to the newly created file.
            QTextStream stream(&emptyFile);
            stream << rgUtils::GenerateTemplateCode(m_pProject->m_api, newFilename).c_str() << endl;
            emptyFile.close();

            // Add the source file's path to the project's clone.
            rgConfigManager::Instance().AddSourceFileToProject(fullSourceFilePath, m_pProject, m_cloneIndex);

            // Save the project after adding a source file.
            rgConfigManager::Instance().SaveProjectFile(m_pProject);

            // We are done.
            ret = true;
        }
    }

    return ret;
}

bool rgBuildViewOpenCL::AddExistingSourcefileToProject(const std::string& sourceFilePath)
{
    bool ret = false;

    bool isProjectCreated = (m_pProject != nullptr);
    if (!isProjectCreated)
    {
        isProjectCreated = CreateNewEmptyProject();

        if (isProjectCreated)
        {
            emit ProjectCreated();
        }
    }

    if (isProjectCreated)
    {
        if (!IsSourceFileInProject(sourceFilePath))
        {
            rgConfigManager& configManager = rgConfigManager::Instance();

            // Add the source file's path to the project's clone.
            configManager.AddSourceFileToProject(sourceFilePath, m_pProject, m_cloneIndex);

            // Save the project after adding a sourcefile.
            configManager.SaveProjectFile(m_pProject);

            // Add the selected file to the menu.
            AddFile(sourceFilePath);

            // Set the source code view text with the contents of the selected file.
            SetSourceCodeText(sourceFilePath);

            // The file was added to the project successfully.
            ret = true;

            // This will make the newly-added file the current item,
            // so grey out the Build settings button.
            emit AddedSourceFileToProject();
        }

        if (!ret)
        {
            // Report the error.
            std::stringstream msg;
            msg << STR_ERR_CANNOT_ADD_FILE_A << sourceFilePath << STR_ERR_CANNOT_ADD_FILE_B;
            rgUtils::ShowErrorMessageBox(msg.str().c_str(), this);
        }
    }

    return ret;
}

bool rgBuildViewOpenCL::GetEntrypointNameForLineNumber(const std::string& filePath, int lineNumber, std::string& entryName) const
{
    bool found = false;

    // 1. Check if "lineNumber" is within some kernel code.
    const auto& fileSrcLineData = m_entrypointLineNumbers.find(filePath);
    if (fileSrcLineData != m_entrypointLineNumbers.end())
    {
        for (const auto& entrySrcLineData : fileSrcLineData->second)
        {
            const std::pair<int, int>  startAndEndLines = entrySrcLineData.second;
            if (lineNumber >= startAndEndLines.first && lineNumber <= startAndEndLines.second)
            {
                entryName = entrySrcLineData.first;
                found = true;
                break;
            }
        }
    }

    // 2. Check if "lineNumber" is present in the line correlation table for currently active entry.
    if (!found)
    {
        found = m_pDisassemblyView->IsLineCorrelatedInEntry(filePath, m_currentTargetGpu, entryName, lineNumber);

        assert(m_pFileMenu != nullptr);
        if (m_pFileMenu != nullptr)
        {
            // Fall back to selecting the current entry point in the selected file item.
            rgMenuFileItem* pFileItem = m_pFileMenu->GetFileItemFromPath(filePath);
            assert(pFileItem != nullptr);
            if (pFileItem != nullptr)
            {
                rgMenuFileItemOpenCL* pFileItemOpenCL = static_cast<rgMenuFileItemOpenCL*>(pFileItem);
                assert(pFileItemOpenCL != nullptr);
                if (pFileItemOpenCL != nullptr)
                {
                    std::string entrypointName;
                    if (pFileItemOpenCL->GetSelectedEntrypointName(entrypointName))
                    {
                        entryName = entrypointName;
                    }
                }
            }
        }
    }

    return found;
}

void rgBuildViewOpenCL::HandleSelectedFileChanged(const std::string& oldFilepath, const std::string& newFilepath)
{
    // Get a pointer to the editor responsible for displaying the new file.
    rgSourceCodeEditor* pEditor = GetEditorForFilepath(newFilepath, rgSrcLanguage::OpenCL);
    assert(pEditor != nullptr);
    if (pEditor != nullptr)
    {
        bool isEditorSwitched = SwitchToEditor(pEditor);
        if (isEditorSwitched)
        {
            // Switch the disassembly view to show the currently-selected entry point in the newly-selected file item.
            if (m_pDisassemblyView != nullptr && !m_isBuildInProgress)
            {
                // Open the disassembly view for the source file only if it's disassembled.
                if (IsGcnDisassemblyGenerated(newFilepath))
                {
                    rgMenuFileItem* pFileItem = m_pFileMenu->GetFileItemFromPath(newFilepath);
                    assert(pFileItem != nullptr);
                    if (pFileItem != nullptr)
                    {
                        rgMenuFileItemOpenCL* pFileItemOpenCL = static_cast<rgMenuFileItemOpenCL*>(pFileItem);
                        assert(pFileItemOpenCL != nullptr);
                        if (pFileItemOpenCL != nullptr)
                        {
                            // Retrieve the name of the currently-selected entry point (if there is one).
                            std::string selectedEntrypointName;
                            bool isEntrypointSelected = pFileItemOpenCL->GetSelectedEntrypointName(selectedEntrypointName);

                            // Update the visibility of the disassembly view.
                            ToggleDisassemblyViewVisibility(isEntrypointSelected);

                            if (isEntrypointSelected)
                            {
                                // Make the disassembly view visible because the file has build outputs to display.
                                emit SelectedEntrypointChanged(m_currentTargetGpu, newFilepath, selectedEntrypointName);
                            }

                            // Update the titlebar for the current source editor.
                            UpdateSourceEditorTitlebar(pEditor);
                        }
                    }
                }
                else
                {
                    // Hide the disassembly view when switching to a file that hasn't been disassembled.
                    ToggleDisassemblyViewVisibility(false);
                }
            }
        }
    }
}

void rgBuildViewOpenCL::HandleSourceFileSelectedLineChanged(rgSourceCodeEditor* pEditor, int lineNumber)
{
    // Handle updating source correlation only when the project isn't currently being built.
    if (!m_isBuildInProgress)
    {
        if (m_pDisassemblyView != nullptr && !m_pDisassemblyView->IsEmpty())
        {
            const std::string& inputFilename = GetFilepathForEditor(pEditor);
            bool isDisassembled = IsGcnDisassemblyGenerated(inputFilename);
            if (isDisassembled)
            {
                int correlatedLineNumber = kInvalidCorrelationLineIndex;

                bool isCorrelationEnabled = IsLineCorrelationEnabled(pEditor);
                if (isCorrelationEnabled)
                {
                    correlatedLineNumber = lineNumber;
                }

                // If the line is associated with a named entrypoint, highlight it in the file menu item.
                std::string entryName;
                bool isValid = GetEntrypointNameForLineNumber(inputFilename, lineNumber, entryName);
                if (isValid)
                {
                    rgMenuOpenCL* pMenuOpenCL = static_cast<rgMenuOpenCL*>(m_pFileMenu);
                    assert(pMenuOpenCL != nullptr);
                    if (pMenuOpenCL != nullptr)
                    {
                        pMenuOpenCL->HandleSelectedEntrypointChanged(inputFilename, entryName);
                    }
                }

                // Send the input source file's correlation line index to the disassembly view.
                m_pDisassemblyView->HandleInputFileSelectedLineChanged(m_currentTargetGpu, inputFilename, entryName, correlatedLineNumber);
            }
        }
    }
}

void rgBuildViewOpenCL::HandleSelectedEntrypointChanged(const std::string& inputFilePath, const std::string& selectedEntrypointName)
{
    assert(m_pFileMenu != nullptr);
    if (m_pFileMenu != nullptr)
    {
        rgMenuOpenCL* pFileMenuOpenCL = static_cast<rgMenuOpenCL*>(m_pFileMenu);
        if (pFileMenuOpenCL != nullptr)
        {
            // Trigger the file menu to be updated, which will change the current selection in the current item's entry point list.
            pFileMenuOpenCL->HandleSelectedEntrypointChanged(inputFilePath, selectedEntrypointName);
        }
    }

    // Highlight the start line for the given entry point in the source editor.
    HighlightEntrypointStartLine(inputFilePath, selectedEntrypointName);
}

void rgBuildViewOpenCL::HandleMenuItemClicked(rgMenuFileItem* pItem)
{
    if (ShowSaveDialog())
    {
        m_pFileMenu->HandleSelectedFileChanged(pItem);
    }
}

void rgBuildViewOpenCL::ClearFileItemsEntrypointList()
{
    rgMenuOpenCL* pMenuOpenCL = static_cast<rgMenuOpenCL*>(m_pFileMenu);
    assert(pMenuOpenCL != nullptr);
    if (pMenuOpenCL != nullptr)
    {
        // Clear references to build outputs from the file menu,
        pMenuOpenCL->ClearBuildOutputs();
    }
}

int rgBuildViewOpenCL::FindEntrypointStartLine(rgSourceCodeEditor* pEditor, int listKernelsStartLine) const
{
    int actualStartLine = listKernelsStartLine;

    // Verify that the current source editor is valid.
    assert(pEditor != nullptr);
    if (pEditor != nullptr)
    {
        // Start at the given line and search for the next opening brace. This is the "real" start of the entrypoint.
        int searchLine = actualStartLine;
        bool searchingForStartLine = true;
        while (searchingForStartLine)
        {
            QString lineText;
            bool gotLineText = pEditor->GetTextAtLine(searchLine, lineText);
            if (gotLineText)
            {
                // Start at the index of the brace, and check if there's anything else in the line other than whitespace.
                int braceIndex = lineText.indexOf("{");
                if (braceIndex != -1)
                {
                    std::string lineTextString = lineText.toStdString();

                    // Search for alphanumeric characters the occur after the opening brace.
                    auto startCharacter = lineTextString.begin() + (braceIndex + 1);
                    auto endCharacter = lineTextString.end();

                    // Create a list of bools, used to determine if there are any alphanumeric characters in the line after the opening brace.
                    std::vector<bool> isAlphanumericList;
                    isAlphanumericList.resize(lineTextString.size());

                    // Step through each character in the line. If it's alphanumeric, add a "true" to the output list in the character's position.
                    std::transform(startCharacter, endCharacter, isAlphanumericList.begin(), [](char c) { return (isalnum(c) != 0); });

                    // If there are any 'true' values in the isAlphanumericList, it means that an alphanumeric character appeared after the opening brace.
                    auto alphanumericCharactersIter = std::find(isAlphanumericList.begin(), isAlphanumericList.end(), true);
                    if (alphanumericCharactersIter == isAlphanumericList.end())
                    {
                        // There was only whitespace after the opening brace. Advance one more line to where the entry point actually starts.
                        searchLine++;
                    }

                    actualStartLine = searchLine;
                    searchingForStartLine = false;
                }
                else
                {
                    // Step down to the next line to check if there's an opening brace for the entrypoint.
                    searchLine++;
                }
            }
            else
            {
                searchingForStartLine = false;
            }
        }
    }

    return actualStartLine;
}

void rgBuildViewOpenCL::HighlightEntrypointStartLine(const std::string& inputFilePath, const std::string& selectedEntrypointName)
{
    // Find the input file in the map of entry point start line numbers.
    auto inputFileIter = m_entrypointLineNumbers.find(inputFilePath);
    if (inputFileIter != m_entrypointLineNumbers.end())
    {
        // Search for the start line number for the given entry point name.
        EntryToSourceLineRange& fileEntrypointsInfo = inputFileIter->second;
        auto lineNumberIter = fileEntrypointsInfo.find(selectedEntrypointName);
        if (lineNumberIter != fileEntrypointsInfo.end())
        {
            rgSourceCodeEditor* pEditor = GetEditorForFilepath(inputFilePath);
            assert(pEditor != nullptr);
            if (pEditor != nullptr)
            {
                // Retrieve the entrypoint's start line index according to the "list-kernels" results.
                int listKernelsEntrypointStartLine = lineNumberIter->second.first;

                // If necessary, advance to the line with the entrypoint's opening brace. This is the real start of the entrypoint.
                int actualStartLine = FindEntrypointStartLine(pEditor, listKernelsEntrypointStartLine);

                // Scroll to the start of the entrypoint.
                pEditor->ScrollToLine(actualStartLine);

                // Move the cursor to the line where the entry point starts.
                QTextCursor cursor(pEditor->document()->findBlockByLineNumber(actualStartLine - 1));
                pEditor->setTextCursor(cursor);

                // Highlight the start line for the entrypoint.
                QList<int> lineIndices;
                lineIndices.push_back(actualStartLine);
                pEditor->SetHighlightedLines(lineIndices);
            }
        }
    }
}

bool rgBuildViewOpenCL::LoadEntrypointLineNumbers()
{
    bool ret = false;

    // Destroy the existing entry point line numbers map.
    m_entrypointLineNumbers.clear();

    // Invoke the CLI To query entry point names and start line numbers.
    ret = rgCliLauncher::ListKernels(m_pProject, m_cloneIndex, m_entrypointLineNumbers);
    if (!ret)
    {
        // Let the user know that the query failed.
        emit SetStatusBarText(STR_ERR_FAILED_TO_GET_ENTRYPOINT_LINE_NUMBERS, gs_STATUS_BAR_NOTIFICATION_TIMEOUT_MS);
    }

    return ret;
}

void rgBuildViewOpenCL::SetAPISpecificBorderColor()
{
    HandleSetFrameBorderGreen();
}

bool rgBuildViewOpenCL::IsLineCorrelationSupported() const
{
    return true;
}

void rgBuildViewOpenCL::UpdateApplicationNotificationMessage()
{
    // Add OpenCL application notification message here, when needed.
}

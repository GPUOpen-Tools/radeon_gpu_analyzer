// C++.
#include <cassert>
#include <sstream>
#include <thread>
#include <iostream>

// Qt.
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QWidget>

// Infra.
#include <QtCommon/Scaling/ScalingManager.h>
#include <Utils/Vulkan/Include/rgPsoFactoryVulkan.h>
#include <Utils/Vulkan/Include/rgPsoSerializerVulkan.h>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgBuildViewVulkan.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgBuildSettingsViewVulkan.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgBuildSettingsWidget.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgCliOutputView.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgIsaDisassemblyView.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMaximizeSplitter.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMenuBuildSettingsItem.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMenuFileItemGraphics.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMenuPipelineStateItem.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgPipelineStateModelVulkan.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgPipelineStateView.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMenuVulkan.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgSourceCodeEditor.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgSourceEditorTitlebar.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgViewManager.h>
#include <RadeonGPUAnalyzerGUI/Include/rgCliLauncher.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDefinitions.h>
#include <RadeonGPUAnalyzerGUI/Include/rgFactoryVulkan.h>
#include <RadeonGPUAnalyzerGUI/Include/rgUtilsVulkan.h>
#include <RadeonGPUAnalyzerGUI/Include/rgStringConstants.h>
#include <RadeonGPUAnalyzerGUI/Include/rgUtils.h>
#include <RadeonGPUAnalyzerGUI/Include/rgXMLSessionConfig.h>

// PSO editor container frame name.
static const char* s_PSO_EDITOR_FRAME_NAME = "PSOEditorContainerFrame";
static const char* s_APPLICATION_INFORMATION_MESSAGE = "Used vk-spv-offline mode.";
static const char* s_APPLICATION_INFORMATION_TOOLTIP = "Compilation failed with AMD's Vulkan ICD (amdvlk), used vk-spv-offline mode instead. Check the build output window for more details.";

rgBuildViewVulkan::rgBuildViewVulkan(QWidget* pParent)
    : rgBuildViewGraphics(rgProjectAPI::Vulkan, pParent)
{
}

void rgBuildViewVulkan::ConnectBuildSettingsSignals()
{
    // Connect the parent's signals.
    rgBuildView::ConnectBuildSettingsSignals();

    // Connect the notification of pending state changes from the Vulkan build settings to the build view.
    bool isConnected = connect(static_cast<rgBuildSettingsViewVulkan*>(m_pBuildSettingsView), &rgBuildSettingsViewVulkan::PendingChangesStateChanged, this, &rgBuildView::HandleBuildSettingsPendingChangesStateChanged);
    assert(isConnected);

    // Connect the notification that the build settings have been saved from the Vulkan build settings to the build view.
    isConnected = connect(static_cast<rgBuildSettingsViewVulkan*>(m_pBuildSettingsView), &rgBuildSettingsViewVulkan::ProjectBuildSettingsSaved, this, &rgBuildView::HandleBuildSettingsSaved);
    assert(isConnected);

    // Connect to build settings view's edit line's "focus in" event to color the frame red.
    isConnected = connect(static_cast<rgBuildSettingsViewVulkan*>(m_pBuildSettingsView), &rgBuildSettingsViewVulkan::SetFrameBorderRedSignal, this, &rgBuildView::HandleSetFrameBorderRed);
    assert(isConnected);

    // Connect to build settings view's edit line's "focus out" event to color the frame black.
    isConnected = connect(static_cast<rgBuildSettingsViewVulkan*>(m_pBuildSettingsView), &rgBuildSettingsViewVulkan::SetFrameBorderBlackSignal, this, &rgBuildView::HandleSetFrameBorderBlack);
    assert(isConnected);
}

void rgBuildViewVulkan::SetAPISpecificBorderColor()
{
    HandleSetFrameBorderRed();
}

bool rgBuildViewVulkan::ConnectMenuSignals()
{
    // Connect the actions for the context menu.
    bool isConnected = connect(m_pActionAddExistingFile, &QAction::triggered, this, &rgBuildViewVulkan::HandleAddExistingFile);
    assert(isConnected);
    isConnected = connect(m_pActionCreateNewFile, &QAction::triggered, this, &rgBuildViewVulkan::HandleCreateNewFile);
    assert(isConnected);

    rgMenuGraphics* pMenu = static_cast<rgMenuGraphics*>(m_pFileMenu);
    assert(pMenu != nullptr);
    if (pMenu != nullptr)
    {
        // Connect the file menu's "Add existing file" button handler.
        isConnected = connect(pMenu, &rgMenuGraphics::AddExistingFileButtonClicked, this, &rgBuildViewVulkan::HandleAddFileButtonClicked);
        assert(isConnected);

        // Connect the file menu item's drag and drop handler.
        isConnected = connect(pMenu, &rgMenuGraphics::DragAndDropExistingFile, this, &rgBuildViewVulkan::HandleExistingFileDragAndDrop);
        assert(isConnected);

        // Connect the load PSO file handler with drag and drop file.
        isConnected = connect(pMenu, &rgMenuGraphics::DragAndDropExistingPsoFile, this, &rgBuildViewVulkan::HandleLoadPipelineStateFile);
        assert(isConnected);

        // Connect the file menu's "Remove file" button handler.
        isConnected = connect(pMenu, &rgMenuGraphics::RemoveFileButtonClicked, this, &rgBuildViewVulkan::HandleRemoveFileButtonClicked);
        assert(isConnected);

        // Connect the file menu's "Restore original SPIR-V binary" handler.
        isConnected = connect(pMenu, &rgMenuGraphics::RestoreOriginalSpirvClicked, this, &rgBuildViewVulkan::HandleRestoreOriginalSpvClicked);
        assert(isConnected);

        // Connect the file menu item selection handler for each new item.
        isConnected = connect(pMenu, &rgMenuGraphics::MenuItemClicked, this, &rgBuildViewVulkan::HandleMenuItemClicked);
        assert(isConnected);

        // Connect the file menu item focus next view signal.
        isConnected = connect(pMenu, &rgMenuGraphics::FocusNextView, this, &rgBuildView::HandleFocusNextView);
        assert(isConnected);

        // Connect the file menu item focus previous view signal.
        isConnected = connect(pMenu, &rgMenuGraphics::FocusPrevView, this, &rgBuildView::HandleFocusPrevView);
        assert(isConnected);

        // Connect the "Pipeline state" button in the file menu.
        rgMenuPipelineStateItem* pPipelineStateItem = pMenu->GetPipelineStateItem();
        assert(pPipelineStateItem != nullptr);
        if (pPipelineStateItem != nullptr)
        {
            isConnected = connect(pPipelineStateItem, &rgMenuPipelineStateItem::PipelineStateButtonClicked, this, &rgBuildViewVulkan::HandlePipelineStateMenuItemClicked);
            assert(isConnected);
        }
    }

    return isConnected;
}

void rgBuildViewVulkan::HandlePipelineStateFileSaved()
{
    bool isOk = false;
    std::string errorString;

    std::shared_ptr<rgProjectCloneVulkan> pVulkanClone =
        std::dynamic_pointer_cast<rgProjectCloneVulkan>(m_pProject->m_clones[m_cloneIndex]);

    assert(pVulkanClone != nullptr);
    if (pVulkanClone != nullptr)
    {
        // Determine which PSO state file is being edited.
        const rgPipelineState& currentPsoState = pVulkanClone->m_psoStates[m_pipelineStateIndex];

        // Save the pipeline state file.
        isOk = m_pPipelineStateModel->SavePipelineStateFile(currentPsoState.m_pipelineStateFilePath, errorString);
    }

    assert(isOk);
    if (!isOk)
    {
        // The pipeline state failed to save properly. Is the user currently viewing the PSO editor?
        if (m_editMode != EditMode::PipelineSettings)
        {
            // If the user isn't in the PSO editor, switch to it so they can resolve the problem.
            SwitchEditMode(EditMode::PipelineSettings);
        }

        // Show the error string to the user, letting them know that the PSO file was not saved correctly.
        std::stringstream errorStream;
        errorStream << STR_ERR_CANNOT_SAVE_PIPELINE_STATE_FILE;
        errorStream << " ";
        errorStream << errorString;
        rgUtils::ShowErrorMessageBox(errorStream.str().c_str(), this);
    }
}

void rgBuildViewVulkan::HandlePipelineStateFileLoaded()
{
    // Open the file browser in the last selected directory.
    std::string pipelineStateFilePath = rgConfigManager::Instance().GetLastSelectedFolder();

    std::shared_ptr<rgProjectCloneVulkan> pVulkanClone =
        std::dynamic_pointer_cast<rgProjectCloneVulkan>(m_pProject->m_clones[m_cloneIndex]);

    assert(pVulkanClone != nullptr);
    if (pVulkanClone != nullptr)
    {
        // Select the relevant file filter based on the pipeline type (compute, graphics).
        bool isGraphicsPipeline = (pVulkanClone->m_pipeline.m_type == rgPipelineType::Graphics);
        const char* psoFileFilter = isGraphicsPipeline ? STR_DEFAULT_PIPELINE_FILE_EXTENSION_FILTER_GRAPHICS :
            STR_DEFAULT_PIPELINE_FILE_EXTENSION_FILTER_COMPUTE;

        // Display an "Open file" dialog to let the user choose
        // which pipeline state configuration file to use.
        bool isOk = rgUtils::OpenFileDialog(this, pipelineStateFilePath,
            STR_PIPELINE_STATE_FILE_DIALOG_CAPTION, psoFileFilter);

        if (isOk)
        {
            HandleLoadPipelineStateFile(pipelineStateFilePath);
        }
    }
}

bool rgBuildViewVulkan::LoadPipelineStateFile(const std::string& pipelineStateFilePath)
{
    bool isOk = false;

    std::shared_ptr<rgProjectCloneVulkan> pVulkanClone =
        std::dynamic_pointer_cast<rgProjectCloneVulkan>(m_pProject->m_clones[m_cloneIndex]);

    std::string errorString;
    assert(pVulkanClone != nullptr);
    if (pVulkanClone != nullptr)
    {
        // Determine which pipeline state file is being edited.
        rgPipelineState& currentPsoState = pVulkanClone->m_psoStates[m_pipelineStateIndex];

        // Attempt to load the pipeline state file in the model.
        isOk = m_pPipelineStateModel->LoadPipelineStateFile(this, pipelineStateFilePath, pVulkanClone->m_pipeline.m_type, errorString);
        assert(isOk);
        if (isOk)
        {
            // Assign the pipeline state file in the project.
            currentPsoState.m_originalPipelineStateFilePath = pipelineStateFilePath;

            // Initialize the model.
            m_pPipelineStateView->InitializeModel(m_pPipelineStateModel);

            // Scale the settings tree after each load.
            m_pPipelineStateView->ScaleSettingsTree();
        }
    }

    assert(isOk);
    if (isOk)
    {
        // Emit a signal indicating that the specified PSO file has been loaded successfully.
        emit PsoFileLoaded();
    }
    else
    {
        // Show the error string to the user, letting them know that the PSO file was not saved correctly.
        std::stringstream errorStream;
        errorStream << STR_ERR_CANNOT_LOAD_PIPELINE_STATE_FILE;
        errorStream << " ";
        errorStream << errorString;
        emit SetStatusBarText(errorStream.str().c_str());
    }

    return isOk;
}

void rgBuildViewVulkan::HandlePipelineStateMenuItemClicked(rgMenuPipelineStateItem* pItem)
{
    if (ShowSaveDialog())
    {
        SwitchEditMode(EditMode::PipelineSettings);

        assert(m_pFileMenu != nullptr);
        if (m_pFileMenu != nullptr)
        {
            m_pFileMenu->HandlePipelineStateButtonClicked(true);
        }

        assert(pItem != nullptr);
        if (pItem != nullptr)
        {
            pItem->SetCurrent(true);
        }

        // Reset the find text widget.
        assert(m_pPipelineStateView != nullptr);
        if (m_pPipelineStateView != nullptr)
        {
            m_pPipelineStateView->ResetSearch();
        }
    }
}

void rgBuildViewVulkan::HandleMenuItemClicked(rgMenuFileItem* pItem)
{
    if (ShowSaveDialog())
    {
        m_pFileMenu->HandleSelectedFileChanged(pItem);
    }
}

rgMenu* rgBuildViewVulkan::GetMenu() const
{
    return m_pFileMenu;
}

rgPipelineStateModel* rgBuildViewVulkan::GetPipelineStateModel()
{
    return m_pPipelineStateModel;
}

bool rgBuildViewVulkan::PopulateMenu()
{
    bool isLoadFailed = false;

    std::shared_ptr<rgProjectCloneVulkan> pVulkanClone =
        std::dynamic_pointer_cast<rgProjectCloneVulkan>(m_pProject->m_clones[m_cloneIndex]);

    assert(pVulkanClone != nullptr);
    if (pVulkanClone != nullptr)
    {
        // Determine the type of pipeline to populate the menu widget with.
        bool isGraphicsPipeline = (pVulkanClone->m_pipeline.m_type == rgPipelineType::Graphics);

        // 1st non-empty stage item.
        size_t firstNonEmptyStage = -1;

        // Step through each stage type in a graphics pipeline by default.
        size_t firstStage = static_cast<size_t>(rgPipelineStage::Vertex);
        size_t lastStage = static_cast<size_t>(rgPipelineStage::Fragment);

        // If a compute pipeline is being loaded, only attempt to load the single Compute stage.
        if (!isGraphicsPipeline)
        {
            firstStage = lastStage = static_cast<size_t>(rgPipelineStage::Compute);
        }

        for (size_t stageIndex = firstStage; stageIndex <= lastStage; ++stageIndex)
        {
            const std::string& stageFilePath = pVulkanClone->m_pipeline.m_shaderStages[stageIndex];

            if (!stageFilePath.empty())
            {
                // Check that the file still exists before attempting to load it.
                bool isFileExists = rgUtils::IsFileExists(stageFilePath);
                assert(isFileExists);
                if (isFileExists)
                {
                    // Add the selected file to the menu.
                    SetStageSourceFile(static_cast<rgPipelineStage>(stageIndex), stageFilePath);
                    firstNonEmptyStage = (firstNonEmptyStage == -1 ? stageIndex : firstNonEmptyStage);
                }
                else
                {
                    // Build an error string saying the file couldn't be found on disk.
                    std::stringstream errorString;
                    errorString << STR_ERR_CANNOT_LOAD_SOURCE_FILE_MSG;
                    errorString << stageFilePath;

                    // Show the user the error message.
                    rgUtils::ShowErrorMessageBox(errorString.str().c_str(), this);

                    isLoadFailed = true;
                }
            }
        }

        // Select the 1st non-empty File Menu item.
        assert(m_pFileMenu != nullptr);
        if (m_pFileMenu != nullptr && firstNonEmptyStage != -1)
        {
            auto pMenuItem = m_pFileMenu->GetStageItem(static_cast<rgPipelineStage>(firstNonEmptyStage));
            if (pMenuItem != nullptr && !pMenuItem->GetFilename().empty())
            {
                m_pFileMenu->HandleSelectedFileChanged(pMenuItem);
            }
        }
    }

    return !isLoadFailed;
}

bool rgBuildViewVulkan::IsGcnDisassemblyGenerated(const std::string& inputFilePath) const
{
    bool isCurrentFileDisassembled = false;

    auto targetGpuOutputsIter = m_buildOutputs.find(m_currentTargetGpu);
    if (targetGpuOutputsIter != m_buildOutputs.end())
    {
        std::shared_ptr<rgCliBuildOutputPipeline> pBuildOutput =
            std::dynamic_pointer_cast<rgCliBuildOutputPipeline>(targetGpuOutputsIter->second);

        assert(pBuildOutput != nullptr);
        if (pBuildOutput != nullptr)
        {
            auto inputFileIter = pBuildOutput->m_perFileOutput.find(inputFilePath);
            if (inputFileIter != pBuildOutput->m_perFileOutput.end())
            {
                rgFileOutputs& fileOutputs = inputFileIter->second;
                isCurrentFileDisassembled = !fileOutputs.m_outputs.empty();
            }
        }
    }

    return isCurrentFileDisassembled;
}

bool rgBuildViewVulkan::LoadSessionMetadata(const std::string& metadataFilePath, std::shared_ptr<rgCliBuildOutput>& pBuildOutput)
{
    bool ret = false;
    std::shared_ptr<rgCliBuildOutputPipeline> pGpuOutputVulkan = nullptr;
    ret = rgXMLSessionConfig::ReadSessionMetadataVulkan(metadataFilePath, pGpuOutputVulkan);
    assert(ret);
    if (ret)
    {
        pBuildOutput = pGpuOutputVulkan;
    }

    return ret;
}

void rgBuildViewVulkan::ReloadFile(const std::string& filePath)
{
    assert(m_pFileMenu != nullptr);
    if (m_pFileMenu != nullptr)
    {
        rgMenuFileItemGraphics* pFileItem = static_cast<rgMenuFileItemGraphics*>(m_pFileMenu->GetFileItemFromPath(filePath));
        assert(pFileItem);
        if (pFileItem != nullptr)
        {
            // If it's a SPIR-V binary file, disassemble it and update the disassembly text in the code editor.
            if (pFileItem->GetFileType() == rgVulkanInputType::Spirv)
            {
                std::string cliOutput, compilerPath;
                const std::string& disasmFilePath = m_spvDisasmFiles[pFileItem->GetStage()];
                assert(!disasmFilePath.empty());
                if (!disasmFilePath.empty())
                {
                    // Get the "alternative compiler path" setting value.
                    std::shared_ptr<rgProjectCloneVulkan> pVulkanClone =
                        std::dynamic_pointer_cast<rgProjectCloneVulkan>(m_pProject->m_clones[m_cloneIndex]);

                    assert(pVulkanClone != nullptr && pVulkanClone->m_pBuildSettings != nullptr);
                    if (pVulkanClone != nullptr && pVulkanClone->m_pBuildSettings != nullptr)
                    {
                        compilerPath = std::get<CompilerFolderType::Bin>(pVulkanClone->m_pBuildSettings->m_compilerPaths);
                    }

                    bool isOk = rgCliLauncher::DisassembleSpvToText(compilerPath, filePath, disasmFilePath, cliOutput);
                    HandleNewCLIOutputString(cliOutput);
                    if (isOk)
                    {
                        SetSourceCodeText(disasmFilePath);

                        // Update the last modification time.
                        QFileInfo fileInfo(filePath.c_str());
                        m_fileModifiedTimeMap[m_pCurrentCodeEditor] = fileInfo.lastModified();
                    }
                }
            }
            else
            {
                SetSourceCodeText(filePath);
            }
        }
    }
}

void rgBuildViewVulkan::ShowCurrentFileDisassembly()
{
    bool isCurrentFileDisassembled = false;

    // Show the currently selected file's first entry point disassembly (if there is no currently selected entry).
    const std::string& inputFilepath = m_pFileMenu->GetSelectedFilePath();
    rgMenuFileItem* pSelectedFileItem = m_pFileMenu->GetSelectedFileItem();
    assert(pSelectedFileItem != nullptr);

    if (pSelectedFileItem != nullptr)
    {
        rgMenuFileItemGraphics* pFileItemPipelineStage = static_cast<rgMenuFileItemGraphics*>(pSelectedFileItem);
        assert(pFileItemPipelineStage != nullptr);

        if (pFileItemPipelineStage != nullptr)
        {
            isCurrentFileDisassembled = IsGcnDisassemblyGenerated(inputFilepath);
            if (isCurrentFileDisassembled)
            {
                // Show the first entry point in the disassembly table.
                const std::string& currentEntrypointName = STR_DEFAULT_VULKAN_GLSL_ENTRYPOINT_NAME;
                m_pDisassemblyView->HandleSelectedEntrypointChanged(m_currentTargetGpu, inputFilepath, currentEntrypointName);

                // Emit a signal indicating that the selected entry point has changed.
                emit SelectedEntrypointChanged(m_currentTargetGpu, inputFilepath, currentEntrypointName);
            }
        }
    }

    // Toggle the view based on if the current file has been disassembled or not.
    ToggleDisassemblyViewVisibility(isCurrentFileDisassembled);
}

void rgBuildViewVulkan::SaveFile(rgMenuFileItemGraphics* pFileItem)
{
    // Get the file editor for the file menu item.
    rgSourceCodeEditor* pFileEditor = nullptr;
    const std::string filePath = pFileItem->GetFilename();
    auto iter = m_sourceCodeEditors.find(filePath);
    assert(iter != m_sourceCodeEditors.end());
    if (iter != m_sourceCodeEditors.end())
    {
        pFileEditor = iter->second;
    }

    assert(pFileEditor != nullptr);
    assert(m_cloneIndex < m_pProject->m_clones.size());
    if (pFileEditor != nullptr && m_pFileMenu != nullptr &&
        (m_cloneIndex < m_pProject->m_clones.size()))
    {
        // Check if the file needs reassembling (modified SPIR-V disassembly).
        bool needReassemble = false;
        assert(pFileItem != nullptr);
        if (pFileItem != nullptr)
        {
            needReassemble = (pFileItem->GetFileType() == rgVulkanInputType::Spirv);
        }

        // Check if we're saving a modified disassembled SPIR-V text first time.
        // If so, we have to replace the original spv binary with the modified disassembly file in the Project.
        if (needReassemble && pFileEditor->document()->isModified())
        {
            rgPipelineStage stage = pFileItem->GetStage();
            const std::string& spvDisasmFilePath = m_spvDisasmFiles[stage];
            QFileInfo fileInfo(spvDisasmFilePath.c_str());

            // Write the editor text to file if the file path is valid.
            if (!spvDisasmFilePath.empty())
            {
                SaveEditorTextToFile(pFileEditor, spvDisasmFilePath);
            }

            // Show the Code Editor title message warning the user about replacing original spv binary file
            // with its disassembly in the project.
            const std::string msg = fileInfo.fileName().toStdString() + ". " + STR_SOURCE_EDITOR_TITLEBAR_SPIRV_DISASM_SAVED_A + " " +
                STR_FILE_CONTEXT_MENU_RESTORE_SPV + " " + STR_SOURCE_EDITOR_TITLEBAR_SPIRV_DISASM_SAVED_B;
            pFileEditor->SetTitleBarText(msg);

            // If it's the current item, display the message on the title bar.
            if (m_pFileMenu->IsCurrentlySelectedFileItem(pFileItem) && m_pSourceEditorTitlebar != nullptr)
            {
                m_pSourceEditorTitlebar->ShowMessage(msg);
            }

            // Switch "last modified time" to the spv disassembly file.
            m_fileModifiedTimeMap[pFileEditor] = fileInfo.lastModified();

            // Replace the spv binary file with its disassembly in the project, File Menu and in the Code Editor map.
            const std::string& spvFilePath = pFileItem->GetFilename();
            auto editor = m_sourceCodeEditors.find(spvFilePath);
            assert(editor != m_sourceCodeEditors.end());
            if (editor != m_sourceCodeEditors.end())
            {
                // Replace file path in the Code Editor map.
                m_sourceCodeEditors.erase(editor);
                m_sourceCodeEditors[spvDisasmFilePath] = pFileEditor;
            }

            // Store the original spv binary path to the project so that a user can restore it later.
            std::shared_ptr<rgProjectCloneVulkan> pVulkanClone =
                std::dynamic_pointer_cast<rgProjectCloneVulkan>(m_pProject->m_clones[m_cloneIndex]);
            assert(pVulkanClone != nullptr);
            if (pVulkanClone != nullptr)
            {
                pVulkanClone->m_spvBackup.m_type = pVulkanClone->m_pipeline.m_type;
                pVulkanClone->m_spvBackup.m_shaderStages[stage] = spvFilePath;

                // Replace file path in the project.
                pVulkanClone->m_pipeline.m_shaderStages[stage] = spvDisasmFilePath;
                rgConfigManager::Instance().SaveProjectFile(m_pProject);
            }

            assert(m_pFileMenu != nullptr);
            if (m_pFileMenu != nullptr)
            {
                // Replace file path in the File Menu.
                m_pFileMenu->ReplaceStageFile(stage, spvDisasmFilePath, rgVulkanInputType::SpirvTxt);

                // Add "restore spv" option to the file menu item.
                m_pFileMenu->GetStageItem(stage)->AddContextMenuActionRestoreSpv();
            }

            // Replace file path in the Build Output.
            ReplaceInputFileInBuildOutput(spvFilePath, spvDisasmFilePath);

            // Replace file path in the Disassembly View tables.
            if (m_pDisassemblyView != nullptr)
            {
                m_pDisassemblyView->ReplaceInputFilePath(spvFilePath, spvDisasmFilePath);
            }
        }
    }
}

void rgBuildViewVulkan::SaveCurrentFile()
{
    assert(m_pCurrentCodeEditor != nullptr && m_pSourceEditorTitlebar != nullptr && m_pFileMenu != nullptr);
    if (m_pCurrentCodeEditor != nullptr && m_pSourceEditorTitlebar != nullptr && m_pFileMenu != nullptr)
    {
        bool needDisassembleSpv = false;

        rgMenuFileItemGraphics* pCurrentMenuItem = m_pFileMenu->GetCurrentFileMenuItem();
        assert(pCurrentMenuItem != nullptr);
        if (pCurrentMenuItem != nullptr)
        {
            needDisassembleSpv = (pCurrentMenuItem->GetFileType() == rgVulkanInputType::Spirv);
        }

        // Check if we're saving a modified disassembled SPIR-V text first time.
        // If so, we have to replace the original spv binary with the modified disassembly file in the Project.
        if (needDisassembleSpv && m_pCurrentCodeEditor->document()->isModified())
        {
            rgPipelineStage stage = m_pFileMenu->GetCurrentStage();
            const std::string& spvDisasmFilePath = m_spvDisasmFiles[stage];
            QFileInfo fileInfo(spvDisasmFilePath.c_str());

            // Write the editor text to file if the file path is valid.
            if (!spvDisasmFilePath.empty())
            {
                SaveEditorTextToFile(m_pCurrentCodeEditor, spvDisasmFilePath);
            }

            // Show the Code Editor title message warning the user about replacing original spv binary file
            // with its disassembly in the project.
            const std::string msg = fileInfo.fileName().toStdString() + ". " + STR_SOURCE_EDITOR_TITLEBAR_SPIRV_DISASM_SAVED_A + " " +
                                    STR_FILE_CONTEXT_MENU_RESTORE_SPV + " " + STR_SOURCE_EDITOR_TITLEBAR_SPIRV_DISASM_SAVED_B;
            m_pCurrentCodeEditor->SetTitleBarText(msg);
            m_pSourceEditorTitlebar->ShowMessage(msg);

            // Switch "last modified time" to the spv disassembly file.
            m_fileModifiedTimeMap[m_pCurrentCodeEditor] = fileInfo.lastModified();

            // Replace the spv binary file with its disassembly in the project, File Menu and in the Code Editor map.
            const std::string& spvFilePath = m_pFileMenu->GetSelectedFilePath();
            auto editor = m_sourceCodeEditors.find(spvFilePath);
            assert(editor != m_sourceCodeEditors.end());
            if (editor != m_sourceCodeEditors.end())
            {
                // Replace file path in the Code Editor map.
                m_sourceCodeEditors.erase(editor);
                m_sourceCodeEditors[spvDisasmFilePath] = m_pCurrentCodeEditor;
            }

            // Store the original spv binary path to the project so that a user can restore it later.
            std::shared_ptr<rgProjectCloneVulkan> pVulkanClone =
                std::dynamic_pointer_cast<rgProjectCloneVulkan>(m_pProject->m_clones[m_cloneIndex]);
            assert(pVulkanClone != nullptr);
            if (pVulkanClone != nullptr)
            {
                pVulkanClone->m_spvBackup.m_type = pVulkanClone->m_pipeline.m_type;
                pVulkanClone->m_spvBackup.m_shaderStages[stage] = spvFilePath;

                // Replace file path in the project.
                pVulkanClone->m_pipeline.m_shaderStages[stage] = spvDisasmFilePath;
                rgConfigManager::Instance().SaveProjectFile(m_pProject);
            }

            assert(m_pFileMenu != nullptr);
            if (m_pFileMenu != nullptr)
            {
                // Replace file path in the File Menu.
                m_pFileMenu->ReplaceStageFile(stage, spvDisasmFilePath, rgVulkanInputType::SpirvTxt);

                // Add "restore spv" option to the file menu item.
                m_pFileMenu->GetStageItem(stage)->AddContextMenuActionRestoreSpv();
            }

            // Replace file path in the Build Output.
            ReplaceInputFileInBuildOutput(spvFilePath, spvDisasmFilePath);

            // Replace file path in the Disassembly View tables.
            if (m_pDisassemblyView != nullptr)
            {
                m_pDisassemblyView->ReplaceInputFilePath(spvFilePath, spvDisasmFilePath);
            }
        }
        else
        {
            rgBuildView::SaveCurrentFile();
        }
    }
}

void rgBuildViewVulkan::SaveSourceFile(const std::string& sourceFilePath)
{
    // Do not save the file if it's a SPIR-V disassembly and the corresponding File Menu item
    // has the "SPIR-V binary" file type.
    bool isSpvBinary = false;
    if (m_pFileMenu != nullptr)
    {
        rgMenuFileItemGraphics* pFileItem = static_cast<rgMenuFileItemGraphics*>(m_pFileMenu->GetFileItemFromPath(sourceFilePath));
        assert(pFileItem != nullptr);
        if (pFileItem != nullptr)
        {
            isSpvBinary = (pFileItem->GetFileType() == rgVulkanInputType::Spirv);
        }
    }

    if (!isSpvBinary)
    {
        rgBuildView::SaveSourceFile(sourceFilePath);
    }
}

bool rgBuildViewVulkan::SaveCurrentState()
{
    // Save the pipeline state.
    HandlePipelineStateFileSaved();

    // Save all file items in the Vulkan way (meaning, check if they
    // are SPIR-V files that need to be reassembled and act accordingly.
    rgMenuGraphics* pMenu = static_cast<rgMenuGraphics*>(GetMenu());
    assert(pMenu != nullptr);
    if (pMenu != nullptr)
    {
        std::vector<rgMenuFileItem*> fileItems = pMenu->GetAllFileItems();
        for (rgMenuFileItem* pItem : fileItems)
        {
            rgMenuFileItemGraphics* pItemGraphics = static_cast<rgMenuFileItemGraphics*>(pItem);
            assert(pItemGraphics != nullptr);
            if (pItemGraphics != nullptr)
            {
                SaveFile(pItemGraphics);
            }
        }
    }

    // Call the base implementation to save the remaining files/settings.
    return rgBuildView::SaveCurrentState();
}

void rgBuildViewVulkan::ConnectPipelineStateViewSignals()
{
    assert(m_pPipelineStateView != nullptr);
    if (m_pPipelineStateView != nullptr)
    {
        // Connect the save button handler.
        bool isConnected = connect(m_pPipelineStateView, &rgPipelineStateView::SaveButtonClicked, this, &rgBuildViewVulkan::HandlePipelineStateFileSaved);
        assert(isConnected);

        // Connect the load button handler.
        isConnected = connect(m_pPipelineStateView, &rgPipelineStateView::LoadButtonClicked, this, &rgBuildViewVulkan::HandlePipelineStateFileLoaded);
        assert(isConnected);

        // Connect the load PSO file handler with drag and drop file.
        isConnected = connect(m_pPipelineStateView, &rgPipelineStateView::DragAndDropExistingFile, this, &rgBuildViewVulkan::HandleLoadPipelineStateFile);
        assert(isConnected);

        // Connect the PSO file loaded handler.
        isConnected = connect(this, &rgBuildViewGraphics::PsoFileLoaded, m_pPipelineStateView, &rgPipelineStateView::HandlePsoFileLoaded);
        assert(isConnected);

        // Connect the pipeline state view's editor's resized handler.
        isConnected = connect(m_pPipelineStateView, &rgPipelineStateView::EditorResized, this, &rgBuildViewGraphics::HandleStateEditorResized);
        assert(isConnected);

        // Connect the pipeline state tree in focus signal.
        isConnected = connect(m_pPipelineStateView, &rgPipelineStateView::PipelineStateTreeFocusIn, this, &rgBuildViewVulkan::HandlePipelineStateTreeFocusIn);
        assert(isConnected);

        // Connect the pipeline state tree focus out signal.
        isConnected = connect(m_pPipelineStateView, &rgPipelineStateView::PipelineStateTreeFocusOut, this, &rgBuildViewVulkan::HandlePipelineStateTreeFocusOut);
        assert(isConnected);

        isConnected = connect(m_pViewManager, &rgViewManager::PSOEditorWidgetFocusOutSignal, this, &rgBuildViewVulkan::HandlePipelineStateTreeFocusOut);
        assert(isConnected);

        isConnected = connect(m_pViewManager, &rgViewManager::PSOEditorWidgetFocusInSignal, this, &rgBuildViewVulkan::HandlePipelineStateTreeFocusIn);
        assert(isConnected);
    }
}

void rgBuildViewVulkan::InitializeModeSpecificViews()
{
    // Create the pipeline state model.
    CreatePipelineStateModel();

    // Create the Vulkan Pipeline State editor view.
    CreatePipelineStateView(this);
}

void rgBuildViewVulkan::FocusOnFileMenu()
{
    // Switch the focus to the file menu.
    if (m_pFileMenu != nullptr)
    {
        m_pFileMenu->setFocus();
    }
}

bool rgBuildViewVulkan::CreateDefaultGraphicsPipeline()
{
    return CreateProject(rgPipelineType::Graphics);
}

bool rgBuildViewVulkan::CreateDefaultComputePipeline()
{
    return CreateProject(rgPipelineType::Compute);
}

void rgBuildViewVulkan::HandleFileRenamed(const std::string& oldFilepath, const std::string& newFilepath)
{
    auto editorIter = m_sourceCodeEditors.find(oldFilepath);
    if (editorIter != m_sourceCodeEditors.end())
    {
        rgSourceCodeEditor* pEditor = editorIter->second;

        assert(pEditor != nullptr);
        if (pEditor != nullptr)
        {
            // Erase the existing file path, and insert the new one.
            m_sourceCodeEditors.erase(editorIter);
            m_sourceCodeEditors[newFilepath] = pEditor;
        }
    }

    // Update the project's source file list with the new filepath.
    std::shared_ptr<rgProjectVulkan> pProject = std::dynamic_pointer_cast<rgProjectVulkan>(m_pProject);
    assert(pProject != nullptr);
    if (pProject != nullptr)
    {
        rgConfigManager::UpdateShaderStageFilePath(oldFilepath, newFilepath, pProject, m_cloneIndex);

        // Save the updated project file.
        rgConfigManager::Instance().SaveProjectFile(pProject);
    }
}

void rgBuildViewVulkan::HandleSelectedFileChanged(const std::string& oldFilepath, const std::string& newFilepath)
{
    // Get a pointer to the editor responsible for displaying the new file.
    rgSourceCodeEditor* pEditor = GetEditorForFilepath(newFilepath);
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
                    // Update the visibility of the disassembly view.
                    ToggleDisassemblyViewVisibility(true);

                    // Make the disassembly view visible because the file has build outputs to display.
                    std::string selectedEntrypointName = STR_DEFAULT_VULKAN_GLSL_ENTRYPOINT_NAME;
                    emit SelectedEntrypointChanged(m_currentTargetGpu, newFilepath, selectedEntrypointName);

                    // Update the titlebar for the current source editor.
                    UpdateSourceEditorTitlebar(pEditor);
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

void rgBuildViewVulkan::HandleSourceFileSelectedLineChanged(rgSourceCodeEditor* pEditor, int lineNumber)
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

                // If the line is associated with a named entry point, highlight it in the file menu item.
                std::string entryName = STR_DEFAULT_VULKAN_GLSL_ENTRYPOINT_NAME;

                // Send the input source file's correlation line index to the disassembly view.
                m_pDisassemblyView->HandleInputFileSelectedLineChanged(m_currentTargetGpu, inputFilename, entryName, correlatedLineNumber);
            }
        }
    }
}

void rgBuildViewVulkan::HandleAddFileButtonClicked(rgPipelineStage stage)
{
    if (ShowSaveDialog())
    {
        // Check if the user would like to add an existing file or create a default one.
        // Set the current stage.
        m_stageClicked = stage;

        // Show the context menu for adding/creating file.
        m_pAddCreateContextMenu->exec(QCursor::pos());
    }
}

void rgBuildViewVulkan::HandleAddExistingFile()
{
    assert(m_pFileMenu != nullptr);
    if (m_pFileMenu != nullptr)
    {
        std::string filePathToAdd;
        bool isOk = rgUtils::OpenFileDialog(this, rgProjectAPI::Vulkan, filePathToAdd);
        if (isOk && !filePathToAdd.empty() && rgUtils::IsFileExists(filePathToAdd) &&
            !m_pFileMenu->IsFileInMenu(filePathToAdd))
        {
            // Convert the path separators to match the style used in the session metadata.
            std::string nativeFilePath = QDir::toNativeSeparators(filePathToAdd.c_str()).toStdString();
            SetStageSourceFile(m_stageClicked, nativeFilePath);

            // Insert the new source file path into the pipeline.
            rgConfigManager& configManager = rgConfigManager::Instance();
            configManager.AddShaderStage(m_stageClicked, nativeFilePath, m_pProject, m_cloneIndex);

            // Save the pipeline project now that the new file is added to the stage.
            configManager.SaveProjectFile(m_pProject);

            // Remove focus from build settings and pipeline state buttons.
            rgMenuVulkan* pVulkanFileMenu = static_cast<rgMenuVulkan*>(m_pFileMenu);
            assert(pVulkanFileMenu != nullptr);
            if (pVulkanFileMenu != nullptr)
            {
                pVulkanFileMenu->SetButtonsNoFocus();
            }
        }
        else if (!filePathToAdd.empty())
        {
            // Inform the user the the file already exists.
            std::stringstream msg;
            msg << STR_ERR_CANNOT_ADD_FILE_A << filePathToAdd << STR_ERR_CANNOT_ADD_FILE_B;
            rgUtils::ShowErrorMessageBox(msg.str().c_str(), this);
        }
    }
}

void rgBuildViewVulkan::HandleCreateNewFile()
{
    // Create a new source file with the default filename.
    std::string baseFilename = STR_DEFAULT_SOURCE_FILENAME;

    // Attempt to create a new source file for the given stage. The final filename may differ
    // from the requested name if the requested filename already exists in the project.
    std::string pathFinalFilePath;
    CreateNewSourceFile(m_stageClicked, baseFilename, pathFinalFilePath);

    // Show the contents of the new file within the editor.
    SetSourceCodeText(pathFinalFilePath);

    // Update the focus indices.
    assert(m_pFileMenu != nullptr);
    if (m_pFileMenu != nullptr)
    {
        m_pFileMenu->UpdateFocusIndex();
    }
}

void rgBuildViewVulkan::HandleExistingFileDragAndDrop(rgPipelineStage stage, const std::string& filePathToAdd)
{
    assert(m_pFileMenu != nullptr);
    if (m_pFileMenu != nullptr && !filePathToAdd.empty())
    {
        if (!m_pFileMenu->IsFileInMenu(filePathToAdd) &&
            rgUtils::IsFileExists(filePathToAdd))
        {
            // Convert the path separators to match the style used in the session metadata.
            std::string nativeFilePath = QDir::toNativeSeparators(filePathToAdd.c_str()).toStdString();
            SetStageSourceFile(stage, nativeFilePath);

            // Insert the new source file path into the pipeline.
            rgConfigManager& configManager = rgConfigManager::Instance();
            configManager.AddShaderStage(stage, nativeFilePath, m_pProject, m_cloneIndex);

            // Save the pipeline project now that the new file is added to the stage.
            configManager.SaveProjectFile(m_pProject);
        }
        else
        {
            // Inform the user the the file already exists.
            std::stringstream msg;
            msg << STR_ERR_CANNOT_ADD_FILE_A << filePathToAdd << STR_ERR_CANNOT_ADD_FILE_B;
            rgUtils::ShowErrorMessageBox(msg.str().c_str(), this);

            // Restore the item from being current (remove the highlight
            // that was assigned to it when it was hovered in the drag process).
            m_pFileMenu->SetCurrent(stage, false);
        }
    }
}

void rgBuildViewVulkan::HandleRemoveFileButtonClicked(rgPipelineStage stage)
{
    if (ShowSaveDialog())
    {
        bool projectExists = (m_pProject != nullptr);
        if (projectExists)
        {
            // Ensure that the incoming clone index is valid for the current project.
            bool isValidRange = (m_cloneIndex >= 0 && m_cloneIndex < m_pProject->m_clones.size());
            assert(isValidRange);

            if (isValidRange)
            {
                std::shared_ptr<rgGraphicsProjectClone> pGraphicsClone
                    = std::dynamic_pointer_cast<rgGraphicsProjectClone>(m_pProject->m_clones[m_cloneIndex]);

                std::string stageFilePath;
                bool isStageOccupied = rgConfigManager::Instance().GetShaderStageFilePath(stage, pGraphicsClone, stageFilePath);

                assert(isStageOccupied);
                if (isStageOccupied)
                {
                    std::stringstream msg;
                    msg << stageFilePath << STR_MENU_BAR_CONFIRM_REMOVE_FILE_DIALOG_WARNING;

                    // Ask the user if they really want to remove the source file.
                    if (ShowRemoveFileConfirmation(msg.str(), stageFilePath))
                    {
                        // Remove the source file's path from the project's clone.
                        rgConfigManager::Instance().RemoveShaderStage(stage, pGraphicsClone);

                        // Save the project after removing a source file.
                        rgConfigManager::Instance().SaveProjectFile(m_pProject);

                        // Update the file menu to remove the filename from the stage item.
                        rgMenuVulkan* pVulkanFileMenu = static_cast<rgMenuVulkan*>(m_pFileMenu);
                        assert(pVulkanFileMenu != nullptr);
                        if (pVulkanFileMenu != nullptr)
                        {
                            bool switchToNextFile = false;

                            // Remove the source file from the stage item.
                            pVulkanFileMenu->ClearStageSourceFile(stage);

                            // Clear out the source view only if this is the currently displayed file.
                            rgMenuFileItemGraphics* pStageItem = pVulkanFileMenu->GetStageItem(stage);
                            assert(pStageItem != nullptr);
                            if (pStageItem != nullptr)
                            {
                                // Set all of the sub buttons to have pointing hand cursor.
                                pStageItem->SetCursor(Qt::PointingHandCursor);

                                // Clear the edit mode.
                                if (m_editMode == EditMode::SourceCode && pVulkanFileMenu->GetCurrentStage() == pStageItem->GetStage())
                                {
                                    switchToNextFile = true;

                                    // Clear out the source view only if one of the buttons
                                    // isn't currently selected.
                                    if (!pVulkanFileMenu->IsButtonPressed())
                                    {
                                        SwitchEditMode(EditMode::Empty);
                                    }
                                }
                            }

                            RemoveEditor(stageFilePath, switchToNextFile);

                            // Set the focus to the file menu so subsequent tabs will be processed.
                            pVulkanFileMenu->setFocus();

                            // Emit a signal to update various menu items.
                            const bool isMenuEmpty = pVulkanFileMenu->IsEmpty();
                            emit pVulkanFileMenu->FileMenuItemCountChanged(isMenuEmpty);
                        }

                        DestroyBuildOutputsForFile(stageFilePath);

                        // Remove the file's build outputs from the disassembly view.
                        if (m_pDisassemblyView != nullptr)
                        {
                            m_pDisassemblyView->RemoveInputFileEntries(stageFilePath);

                            // Hide the disassembly view when there's no data in it.
                            if (m_pDisassemblyView->IsEmpty())
                            {
                                // Minimize the disassembly view before hiding it to preserve correct rgBuildView layout.
                                m_pDisassemblyViewSplitter->Restore();

                                // Hide the disassembly view now that it's empty.
                                ToggleDisassemblyViewVisibility(false);
                            }
                        }
                    }
                }
            }
        }
    }
}

void rgBuildViewVulkan::HandleRestoreOriginalSpvClicked(rgPipelineStage stage)
{
    auto pVulkanClone = std::dynamic_pointer_cast<rgProjectCloneVulkan>(m_pProject->m_clones[m_cloneIndex]);

    rgMenuFileItemGraphics* pFileItem = m_pFileMenu->GetStageItem(stage);
    assert(pFileItem != nullptr && m_pFileMenu != nullptr);
    if (pFileItem != nullptr && m_pFileMenu != nullptr)
    {
        const std::string spvDisasm = pFileItem->GetFilename();
        const std::string origSpv = pVulkanClone->m_spvBackup.m_shaderStages[stage];

        if (ShowRevertToSpvBinaryConfirmation(origSpv))
        {
            // Replace files in the project clone.
            pVulkanClone->m_pipeline.m_shaderStages[stage] = origSpv;
            pVulkanClone->m_spvBackup.m_shaderStages[stage].clear();

            // Save the project file.
            rgConfigManager::Instance().SaveProjectFile(m_pProject);

            // Replace file path in the Code Editor map.
            m_sourceCodeEditors.erase(spvDisasm);
            m_sourceCodeEditors[origSpv] = m_pCurrentCodeEditor;

            // Delete the edited backup SPIR-V text file from disk.
            QFile file(spvDisasm.c_str());
            file.remove();

            // Update the modification date.
            QFileInfo fileInfo(origSpv.c_str());
            m_fileModifiedTimeMap[m_pCurrentCodeEditor] = fileInfo.lastModified();

            // Disassemble the original SPIR-V binary.
            std::string spvDisasmOutput;
            if (DisasmSpvFile(origSpv, spvDisasmOutput))
            {
                SetSourceCodeText(spvDisasmOutput);

                // Store the path to the disassembled file so that we know where to save the modified disassembly text later.
                m_spvDisasmFiles[stage] = spvDisasmOutput;

                // Store the spv last modification time.
                QFileInfo fileInfo(origSpv.c_str());
                m_fileModifiedTimeMap[m_pCurrentCodeEditor] = fileInfo.lastModified();
            }

            // Replace file path in the File Menu.
            m_pFileMenu->ReplaceStageFile(stage, origSpv, rgVulkanInputType::Spirv);

            // Replace file path in the Build Output.
            ReplaceInputFileInBuildOutput(spvDisasmOutput, origSpv);

            // Replace file path in the Disassembly View tables.
            if (m_pDisassemblyView != nullptr)
            {
                m_pDisassemblyView->ReplaceInputFilePath(spvDisasmOutput, origSpv);
            }

            // Remove the "Restore original SPIR-V binary" item from the File Menu's context menu.
            pFileItem->RemoveContextMenuActionRestoreSpv();

            // Clear the Source Editor Title message for this editor.
            m_pCurrentCodeEditor->SetTitleBarText("");
            m_pSourceEditorTitlebar->SetTitlebarContentsVisibility(false);
        }
    }
}

bool rgBuildViewVulkan::CreateMenu(QWidget* pParent)
{
    m_pFileMenu = static_cast<rgMenuVulkan*>(m_pFactory->CreateFileMenu(pParent));

    // Register the file menu with scaling manager.
    assert(m_pFileMenu != nullptr);
    if (m_pFileMenu != nullptr)
    {
        ScalingManager::Get().RegisterObject(m_pFileMenu);
    }

    return m_pFileMenu != nullptr;
}

void rgBuildViewVulkan::HandleEnumListWidgetStatus(bool isOpen)
{
    assert(m_pPipelineStateView != nullptr);
    if (m_pPipelineStateView != nullptr)
    {
        m_pPipelineStateView->SetEnumListWidgetStatus(isOpen);
    }
}

void rgBuildViewVulkan::CreatePipelineStateModel()
{
    std::shared_ptr<rgFactoryGraphics> pGraphicsFactory = std::dynamic_pointer_cast<rgFactoryGraphics>(m_pFactory);

    assert(pGraphicsFactory != nullptr);
    if (pGraphicsFactory != nullptr)
    {
        m_pPipelineStateModel = static_cast<rgPipelineStateModelVulkan*>(pGraphicsFactory->CreatePipelineStateModel(this));
        assert(m_pPipelineStateModel != nullptr);
        if (m_pPipelineStateModel != nullptr)
        {
            // Connect the list widget status signal.
            bool isConnected = connect(m_pPipelineStateModel, &rgPipelineStateModelVulkan::EnumListWidgetStatusSignal, this, &rgBuildViewVulkan::HandleEnumListWidgetStatus);
            assert(isConnected);

            assert(m_pProject != nullptr);
            if (m_pProject != nullptr)
            {
                // Ensure that the incoming clone index is valid for the current project.
                bool res = (m_cloneIndex >= 0 && m_cloneIndex < m_pProject->m_clones.size());
                assert(res);

                if (res)
                {
                    std::shared_ptr<rgProjectCloneVulkan> pVulkanClone =
                        std::dynamic_pointer_cast<rgProjectCloneVulkan>(m_pProject->m_clones[m_cloneIndex]);

                    assert(pVulkanClone != nullptr);
                    if (pVulkanClone != nullptr)
                    {
                        bool isPipelineStateInitialized = false;
                        std::string errorString;

                        rgPipelineState& currentPipelineState = pVulkanClone->m_psoStates[m_pipelineStateIndex];
                        bool isStateFileExists = rgUtils::IsFileExists(currentPipelineState.m_pipelineStateFilePath);
                        if (isStateFileExists)
                        {
                            // Load the state file from disk.
                            isPipelineStateInitialized =
                                m_pPipelineStateModel->LoadPipelineStateFile(this, currentPipelineState.m_pipelineStateFilePath, pVulkanClone->m_pipeline.m_type, errorString);

                            assert(isPipelineStateInitialized);
                            if (!isPipelineStateInitialized)
                            {
                                std::stringstream errorStream;
                                errorStream << STR_ERR_CANNOT_LOAD_PIPELINE_STATE_FILE;
                                errorStream << " ";
                                errorStream << errorString;
                                emit SetStatusBarText(errorStream.str().c_str());
                            }
                        }

                        // If the pipeline state file didn't exist, or failed to load, initialize the default configuration.
                        if (!isPipelineStateInitialized)
                        {
                            // Initialize the pipeline state model based on the pipeline type.
                            m_pPipelineStateModel->InitializeDefaultPipelineState(this, pVulkanClone->m_pipeline.m_type);

                            // Save the new default pipeline state.
                            isPipelineStateInitialized = m_pPipelineStateModel->SavePipelineStateFile(currentPipelineState.m_pipelineStateFilePath, errorString);
                        }

                        // Was the pipeline state file loaded or created successfully?
                        assert(isPipelineStateInitialized);
                        if (!isPipelineStateInitialized)
                        {
                            std::stringstream errorStream;
                            errorStream << STR_ERR_CANNOT_INITIALIZE_PIPELINE_STATE_FILE;
                            errorStream << errorString;
                            rgUtils::ShowErrorMessageBox(errorStream.str().c_str(), this);
                        }
                    }
                }
            }
        }
    }
}

void rgBuildViewVulkan::CreatePipelineStateView(QWidget* pParent)
{
    // Create a border frame for the pipeline state editor.
    m_pPsoEditorFrame = new QFrame(this);
    m_pPsoEditorFrame->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
    m_pPsoEditorFrame->setFrameStyle(QFrame::Box);
    m_pPsoEditorFrame->setLayout(new QVBoxLayout);
    m_pPsoEditorFrame->setObjectName(s_PSO_EDITOR_FRAME_NAME);

    // Create a new Pipeline State editor view.
    m_pPipelineStateView = new rgPipelineStateView(pParent);

    assert(m_pPipelineStateView != nullptr);
    if (m_pPipelineStateView != nullptr)
    {
        // Add the state editor view to the parent frame.
        m_pPsoEditorFrame->layout()->addWidget(m_pPipelineStateView);

        // Create the find widget, register with the scaling manager.
        m_pPsoFindWidget = new rgFindTextWidget(m_pPipelineStateView);
        ScalingManager::Get().RegisterObject(m_pPsoFindWidget);

        // The find widget is hidden by default.
        m_pPsoFindWidget->hide();

        // Add the find widget to rgPipelineStateView's grid.
        m_pPipelineStateView->InsertFindWidget(m_pPsoFindWidget);

        // Connect the find widget signals.
        ConnectPSOFindSignals();

        assert(m_pProject != nullptr);
        if (m_pProject != nullptr)
        {
            // Ensure that the incoming clone index is valid for the current project.
            bool res = (m_cloneIndex >= 0 && m_cloneIndex < m_pProject->m_clones.size());
            assert(res);

            if (res)
            {
                std::shared_ptr<rgProjectCloneVulkan> pClone =
                    std::dynamic_pointer_cast<rgProjectCloneVulkan>(m_pProject->m_clones[m_cloneIndex]);

                assert(pClone != nullptr);
                if (pClone != nullptr)
                {
                    // Initialize the PSO editor model.
                    m_pPipelineStateView->InitializeModel(m_pPipelineStateModel);

                    // Connect signals for the new pipeline state view.
                    ConnectPipelineStateViewSignals();

                    // The view is hidden initially.
                    m_pPsoEditorFrame->hide();
                }
            }
        }
    }
}

void rgBuildViewVulkan::ConnectDisassemblyViewApiSpecificSignals()
{
    assert(m_pDisassemblyView != nullptr);
    if (m_pDisassemblyView != nullptr)
    {
        // Connect the handler invoked when the user changes the selected entry point.
        bool isConnected = connect(this, &rgBuildViewVulkan::SelectedEntrypointChanged,
            m_pDisassemblyView, &rgIsaDisassemblyView::HandleSelectedEntrypointChanged);
        assert(isConnected);

        // Connect the remove button focus events.
        isConnected = connect(m_pDisassemblyView, &rgIsaDisassemblyView::RemoveFileMenuButtonFocus,
            m_pFileMenu, &rgMenuVulkan::HandleRemoveFileMenuButtonFocus);
        assert(isConnected);
    }
}

bool rgBuildViewVulkan::IsLineCorrelationEnabled(rgSourceCodeEditor* pSourceEditor)
{
    // Source line correlation with disassembly is not available in Vulkan mode.
    return false;
}

bool rgBuildViewVulkan::IsSourceFileInProject(const std::string& sourceFilePath) const
{
    bool res = false;

    // Step through all possible pipeline stages to check if the given source file exists.
    uint32_t firstStage = static_cast<uint32_t>(rgPipelineStage::Vertex);
    uint32_t lastStage = static_cast<uint32_t>(rgPipelineStage::Compute);

    assert(m_pProject != nullptr);
    if (m_pProject != nullptr)
    {
        // Ensure that the incoming clone index is valid for the current project.
        bool isValidRange = (m_cloneIndex >= 0 && m_cloneIndex < m_pProject->m_clones.size());

        assert(isValidRange);
        if (isValidRange)
        {
            std::shared_ptr<rgProjectCloneVulkan> pVulkanClone = std::dynamic_pointer_cast<rgProjectCloneVulkan>(m_pProject->m_clones[m_cloneIndex]);
            for (uint32_t stageIndex = firstStage; stageIndex < lastStage; ++stageIndex)
            {
                rgPipelineStage currentStage = static_cast<rgPipelineStage>(stageIndex);

                std::string stageFilePath;
                if (rgConfigManager::Instance().GetShaderStageFilePath(currentStage, pVulkanClone, stageFilePath))
                {
                    if (!stageFilePath.empty() && stageFilePath.compare(sourceFilePath) == 0)
                    {
                        res = true;
                        break;
                    }
                }
            }
        }
    }

    return res;
}

bool rgBuildViewVulkan::ShowRevertToSpvBinaryConfirmation(const std::string& filePath)
{
    std::string msg = STR_MENU_BAR_VULKAN_CONFIRM_REVERT_TO_ORIG_SPV_A;
    msg += std::string("\n") + filePath + ".\n";
    msg += STR_MENU_BAR_VULKAN_CONFIRM_REVERT_TO_ORIG_SPV_B;

    return rgUtils::ShowConfirmationMessageBox(STR_MENU_BAR_CONFIRM_REMOVE_FILE_DIALOG_TITLE, msg.c_str(), this);
}

bool rgBuildViewVulkan::ReplaceInputFileInBuildOutput(const std::string& oldFilePath, const std::string& newFilePath)
{
    assert(!oldFilePath.empty());
    assert(!newFilePath.empty());
    bool ret = (!oldFilePath.empty() && !newFilePath.empty());
    if (ret)
    {
        auto firstTargetGpu = m_buildOutputs.begin();
        auto lastTargetGpu = m_buildOutputs.end();
        for (auto targetGpuIter = firstTargetGpu; targetGpuIter != lastTargetGpu; ++targetGpuIter)
        {
            ret = false;
            std::shared_ptr<rgCliBuildOutput> pBuildOutput = targetGpuIter->second;

            // Search for outputs for the given source file and replace its key (input file path) with the new one.
            assert(pBuildOutput != nullptr);
            if (pBuildOutput != nullptr)
            {
                auto it = pBuildOutput->m_perFileOutput.find(oldFilePath);
                if (it != pBuildOutput->m_perFileOutput.end())
                {
                    auto outputs = it->second;
                    pBuildOutput->m_perFileOutput.erase(it);
                    pBuildOutput->m_perFileOutput[newFilePath] = outputs;
                    ret = true;
                }
            }
        }
    }

    return ret;
}

bool rgBuildViewVulkan::CreateNewSourceFile(rgPipelineStage stage, const std::string& sourceFileName, std::string& fullSourceFilepath)
{
    bool ret = false;

    // True if we are creating a new file in an existing project.
    bool isExistingProject = (m_pProject != nullptr);

    if (isExistingProject)
    {
        std::shared_ptr<rgUtilsVulkan> pVulkanUtils = std::dynamic_pointer_cast<rgUtilsVulkan>(rgUtilsGraphics::CreateUtility(rgProjectAPI::Vulkan));
        assert(pVulkanUtils != nullptr);
        if (pVulkanUtils != nullptr)
        {
            // Use the Vulkan factory to build a file extension based on the stage being added.
            std::stringstream fileExtension;
            fileExtension << ".";
            fileExtension << pVulkanUtils->PipelineStageToAbbreviation(stage);

            // Generate a path to where the new empty file will live in the projects directory.
            std::string newFilename;
            rgConfigManager::GenerateNewSourceFilepath(m_pProject->m_projectName, m_cloneIndex, sourceFileName, fileExtension.str(), newFilename, fullSourceFilepath);

            // Ensure that the folder where the file will be saved already exists.
            std::string sourcefileFolder;
            rgUtils::ExtractFileDirectory(fullSourceFilepath, sourcefileFolder);
            if (!rgUtils::IsDirExists(sourcefileFolder))
            {
                bool isDirCreated = rgUtils::CreateFolder(sourcefileFolder);
                assert(isDirCreated);
            }

            // Create a new file at the target location.
            QFile emptyFile(fullSourceFilepath.c_str());
            emptyFile.open(QIODevice::ReadWrite);

            // Use the Vulkan factory to fill the new file with the default code for the stage being created.
            QTextStream stream(&emptyFile);
            stream << pVulkanUtils->GetDefaultShaderCode(stage).c_str() << endl;
            emptyFile.close();

            // Add the source file's path to the project's clone.
            rgConfigManager::Instance().AddShaderStage(stage, fullSourceFilepath, m_pProject, m_cloneIndex);

            // Save the project after adding a source file.
            rgConfigManager::Instance().SaveProjectFile(m_pProject);

            assert(m_pFileMenu != nullptr);
            if (m_pFileMenu != nullptr)
            {
                rgMenuVulkan* pVulkanFileMenu = static_cast<rgMenuVulkan*>(m_pFileMenu);
                assert(pVulkanFileMenu != nullptr);
                if (pVulkanFileMenu != nullptr)
                {
                    // Set the source file in the stage item.
                    pVulkanFileMenu->SetStageSourceFile(stage, fullSourceFilepath, rgVulkanInputType::Glsl, true);

                    // Use GLSL syntax highlighting.
                    assert(m_pCurrentCodeEditor != nullptr);
                    if (m_pCurrentCodeEditor != nullptr)
                    {
                        m_pCurrentCodeEditor->SetSyntaxHighlighting(rgSrcLanguage::GLSL);
                    }
                }
            }

            // We are done.
            ret = true;
        }
    }

    return ret;
}

bool rgBuildViewVulkan::CreateProject(rgPipelineType pipelineType)
{
    // True if we are creating a new file in an existing project.
    bool res = (m_pProject != nullptr);

    if (!res)
    {
        res = CreateNewEmptyProject();

        if (res)
        {
            assert(m_pProject != nullptr);
            if (m_pProject != nullptr)
            {
                // Ensure that the incoming clone index is valid for the current project.
                bool res = (m_cloneIndex >= 0 && m_cloneIndex < m_pProject->m_clones.size());
                assert(res);

                if (res)
                {
                    std::shared_ptr<rgProjectCloneVulkan> pClone =
                        std::dynamic_pointer_cast<rgProjectCloneVulkan>(m_pProject->m_clones[m_cloneIndex]);

                    // Initialize the clone's pipeline type.
                    pClone->m_pipeline.m_type = pipelineType;

                    // Create the pipeline CreateInfo object.
                    CreatePipelineStateFile();

                    // The pipeline type was updated after the clone was created, so re-save the
                    // project with the new change.
                    rgConfigManager& configManager = rgConfigManager::Instance();
                    configManager.SaveProjectFile(m_pProject);
                }
            }
        }
    }

    if (res)
    {
        emit ProjectCreated();
    }

    return res;
}

void rgBuildViewVulkan::CreatePipelineStateFile()
{
    assert(m_pProject != nullptr);
    if (m_pProject != nullptr)
    {
        // Ensure that the incoming clone index is valid for the current project.
        bool res = (m_cloneIndex >= 0 && m_cloneIndex < m_pProject->m_clones.size());
        assert(res);

        if (res)
        {
            std::shared_ptr<rgProjectCloneVulkan> pClone =
                std::dynamic_pointer_cast<rgProjectCloneVulkan>(m_pProject->m_clones[m_cloneIndex]);

            assert(pClone != nullptr);
            if (pClone != nullptr)
            {
                size_t pipelineCount = pClone->m_psoStates.size();

                // Generate a suitable pipeline name based on the number of existing pipelines.
                std::stringstream pipelineNameStream;
                pipelineNameStream << STR_DEFAULT_PIPELINE_NAME;
                pipelineNameStream << pipelineCount;
                std::string pipelineStateName = pipelineNameStream.str();

                bool isGraphicsPipeline = (pClone->m_pipeline.m_type == rgPipelineType::Graphics);
                const char* psoFileExtension = isGraphicsPipeline ? STR_DEFAULT_PIPELINE_FILE_EXTENSION_GRAPHICS :
                    STR_DEFAULT_PIPELINE_FILE_EXTENSION_COMPUTE;

                // Build a file path to a new pipeline state file.
                std::string pipelineStateFilePath;
                rgConfigManager::GenerateNewPipelineFilepath(m_pProject->m_projectName, m_cloneIndex, pipelineStateName, psoFileExtension, pipelineStateFilePath);

                // Does the Pipeline State File's target directory already exist? Pipeline State
                // files are stored alongside the clone's source files. If the directory does not
                // yet exist, it needs to be created before attempting to write the file.
                std::string directoryPath;
                bool isOk = rgUtils::ExtractFileDirectory(pipelineStateFilePath, directoryPath);
                if (isOk)
                {
                    bool directoryExists = rgUtils::IsDirExists(directoryPath);
                    if (!directoryExists)
                    {
                        isOk = rgUtils::CreateFolder(directoryPath);
                    }
                }

                if (isOk)
                {
                    rgPipelineState pipelineStateFile = {};
                    pipelineStateFile.m_name = pipelineStateName;
                    pipelineStateFile.m_pipelineStateFilePath = pipelineStateFilePath;
                    pipelineStateFile.m_isActive = true;

                    // Add the new PSO file to the current clone's PSO states vector.
                    pClone->m_psoStates.push_back(pipelineStateFile);
                }
            }
        }
    }
}

void rgBuildViewVulkan::SetStageSourceFile(rgPipelineStage stage, const std::string& filePathToAdd)
{
    assert(m_pFileMenu != nullptr);
    if (m_pFileMenu != nullptr)
    {
        std::shared_ptr<rgUtilsVulkan> pVulkanUtils = std::dynamic_pointer_cast<rgUtilsVulkan>(rgUtilsGraphics::CreateUtility(rgProjectAPI::Vulkan));
        assert(pVulkanUtils != nullptr);
        if (pVulkanUtils != nullptr)
        {
            rgMenuVulkan* pVulkanFileMenu = static_cast<rgMenuVulkan*>(m_pFileMenu);
            assert(pVulkanFileMenu != nullptr);
            if (pVulkanFileMenu != nullptr)
            {
                // Detect type of the file.
                auto fileType = rgUtils::DetectInputFileType(filePathToAdd);

                // Set the source file in the stage item.
                bool fileAdded = pVulkanFileMenu->SetStageSourceFile(stage, filePathToAdd, fileType.first, false);

                // Enable reverting to original SPIR-V binary if it's present in the Project Clone.
                if (fileAdded && fileType.first == rgVulkanInputType::SpirvTxt)
                {
                    std::shared_ptr<rgProjectCloneVulkan> pVulkanClone =
                        std::dynamic_pointer_cast<rgProjectCloneVulkan>(m_pProject->m_clones[m_cloneIndex]);

                    assert(pVulkanClone != nullptr);
                    if (pVulkanClone != nullptr)
                    {
                        if (!pVulkanClone->m_spvBackup.m_shaderStages[stage].empty())
                        {
                            m_pFileMenu->GetStageItem(stage)->AddContextMenuActionRestoreSpv();
                        }
                    }
                }

                if (fileAdded && m_pCurrentCodeEditor != nullptr)
                {
                    // Enable corresponding syntax highlighting.
                    m_pCurrentCodeEditor->SetSyntaxHighlighting(fileType.second);

                    // If the file being added is a SPIR-V binary, disassemble it and show the disassembly text in the source editor.
                    if (fileType.first == rgVulkanInputType::Spirv)
                    {
                        std::string spvDisasmFile;
                        if (DisasmSpvFile(filePathToAdd, spvDisasmFile))
                        {
                            SetSourceCodeText(spvDisasmFile);

                            // Store the path to the disassembled file so that we know where to save the modified disassembly text later.
                            m_spvDisasmFiles[stage] = spvDisasmFile;

                            // Store the spv last modification time.
                            QFileInfo fileInfo(filePathToAdd.c_str());
                            m_fileModifiedTimeMap[m_pCurrentCodeEditor] = fileInfo.lastModified();
                        }
                    }
                    else
                    {
                        SetSourceCodeText(filePathToAdd);
                    }
                }
            }
        }
    }
}

bool rgBuildViewVulkan::DisasmSpvFile(const std::string& spvFile, std::string& spvDisasmFile)
{
    std::string projDir;
    bool isOk = rgUtils::ExtractFileDirectory(m_pProject->m_projectFileFullPath, projDir);
    assert(isOk);
    if (isOk && rgUtils::ConstructSpvDisasmFileName(projDir, spvFile, spvDisasmFile))
    {
        // Create a sub-folder in the Project folder for the spv disasm file if it does not exist.
        std::string subfolder, compilerPath;
        isOk = rgUtils::ExtractFileDirectory(spvDisasmFile, subfolder);
        assert(isOk);
        if (isOk)
        {
            // Get the "alternative compiler path" setting value.
            std::shared_ptr<rgProjectCloneVulkan> pVulkanClone =
                std::dynamic_pointer_cast<rgProjectCloneVulkan>(m_pProject->m_clones[m_cloneIndex]);

            assert(pVulkanClone != nullptr && pVulkanClone->m_pBuildSettings != nullptr);
            if (pVulkanClone != nullptr && pVulkanClone->m_pBuildSettings != nullptr)
            {
                compilerPath = std::get<CompilerFolderType::Bin>(pVulkanClone->m_pBuildSettings->m_compilerPaths);
            }
            rgUtils::CreateFolder(subfolder);
            std::string cliOutput;
            isOk = rgCliLauncher::DisassembleSpvToText(compilerPath, spvFile, spvDisasmFile, cliOutput);
            HandleNewCLIOutputString(cliOutput);
        }
    }

    return isOk;
}

bool rgBuildViewVulkan::IsLineCorrelationSupported() const
{
    return false;
}

rgMenuGraphics* rgBuildViewVulkan::GetGraphicsFileMenu()
{
    return m_pFileMenu;
}

void rgBuildViewVulkan::HandlePipelineStateTreeFocusIn()
{
    // Change the container frame's color.
    assert(m_pPsoEditorFrame != nullptr);
    if (m_pPsoEditorFrame != nullptr)
    {
        // Change the color of the PSO editor frame border.
        QString styleSheetString = QString("#") + s_PSO_EDITOR_FRAME_NAME + QString(" { border: 1px solid red; background-color: transparent}");
        m_pPsoEditorFrame->setStyleSheet(styleSheetString);
    }
}

void rgBuildViewVulkan::HandlePipelineStateTreeFocusOut()
{
    // Change the container frame's color.
    assert(m_pPsoEditorFrame != nullptr);
    if (m_pPsoEditorFrame != nullptr)
    {
        QString styleSheetString = QString("#") + s_PSO_EDITOR_FRAME_NAME + QString(" { border: 1px solid black }");
        m_pPsoEditorFrame->setStyleSheet(styleSheetString);
    }
}

void rgBuildViewVulkan::UpdateApplicationNotificationMessage()
{
    std::string message = "";
    std::string tooltip = "";
    size_t pos = m_pCliOutputWindow->GetText().find("vk-spv-offline");
    if (pos != std::string::npos)
    {
        message = s_APPLICATION_INFORMATION_MESSAGE;
        tooltip = s_APPLICATION_INFORMATION_TOOLTIP;

        // Emit the signal to update the application notification message.
        emit UpdateApplicationNotificationMessageSignal(message, tooltip);
    }
}

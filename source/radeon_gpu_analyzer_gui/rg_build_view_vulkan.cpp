//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for Build View class for Vulkan.
//=============================================================================

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

// QtCommon.
#include "qt_common/utils/qt_util.h"

// Infra.
#include "source/common/vulkan/rg_pso_factory_vulkan.h"
#include "source/common/vulkan/rg_pso_serializer_vulkan.h"

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_build_view_vulkan.h"
#include "radeon_gpu_analyzer_gui/qt/rg_build_settings_view_vulkan.h"
#include "radeon_gpu_analyzer_gui/qt/rg_build_settings_widget.h"
#include "radeon_gpu_analyzer_gui/qt/rg_cli_output_view.h"
#include "radeon_gpu_analyzer_gui/qt/rg_isa_disassembly_view.h"
#include "radeon_gpu_analyzer_gui/qt/rg_maximize_splitter.h"
#include "radeon_gpu_analyzer_gui/qt/rg_menu_build_settings_item.h"
#include "radeon_gpu_analyzer_gui/qt/rg_menu_file_item_graphics.h"
#include "radeon_gpu_analyzer_gui/qt/rg_menu_pipeline_state_item.h"
#include "radeon_gpu_analyzer_gui/qt/rg_pipeline_state_model_vulkan.h"
#include "radeon_gpu_analyzer_gui/qt/rg_pipeline_state_view.h"
#include "radeon_gpu_analyzer_gui/qt/rg_menu_vulkan.h"
#include "radeon_gpu_analyzer_gui/qt/rg_source_code_editor.h"
#include "radeon_gpu_analyzer_gui/qt/rg_source_editor_titlebar.h"
#include "radeon_gpu_analyzer_gui/qt/rg_view_manager.h"
#include "radeon_gpu_analyzer_gui/rg_cli_launcher.h"
#include "radeon_gpu_analyzer_gui/rg_definitions.h"
#include "radeon_gpu_analyzer_gui/rg_factory_vulkan.h"
#include "radeon_gpu_analyzer_gui/rg_utils_vulkan.h"
#include "radeon_gpu_analyzer_gui/rg_string_constants.h"
#include "radeon_gpu_analyzer_gui/rg_utils.h"
#include "radeon_gpu_analyzer_gui/rg_xml_session_config.h"

// PSO editor container frame name.
static const char* kStrPsoEditorFrameName            = "PSOEditorContainerFrame";
static const char* kStrApplicationInformationMessage = "Used vk-spv-offline mode.";
static const char* kStrApplicationInformationTooltip =
    "Compilation failed with AMD's Vulkan ICD (amdvlk), used vk-spv-offline mode instead. Check the build output window for more details.";

RgBuildViewVulkan::RgBuildViewVulkan(QWidget* parent)
    : RgBuildViewGraphics(RgProjectAPI::kVulkan, parent)
{
}

RgBuildViewVulkan::~RgBuildViewVulkan()
{
    RG_SAFE_DELETE(disassembly_view_);
    RG_SAFE_DELETE(file_menu_);
    RG_SAFE_DELETE(pipeline_state_model_);
    RG_SAFE_DELETE(pipeline_state_view_);
}

void RgBuildViewVulkan::ConnectBuildSettingsSignals()
{
    // Connect the parent's signals.
    RgBuildView::ConnectBuildSettingsSignals();

    // Connect the notification of pending state changes from the Vulkan build settings to the build view.
    bool is_connected = connect(static_cast<RgBuildSettingsViewVulkan*>(build_settings_view_),
                                &RgBuildSettingsViewVulkan::PendingChangesStateChanged,
                                this,
                                &RgBuildView::HandleBuildSettingsPendingChangesStateChanged);
    assert(is_connected);

    // Connect the notification that the build settings have been saved from the Vulkan build settings to the build view.
    is_connected = connect(static_cast<RgBuildSettingsViewVulkan*>(build_settings_view_),
                           &RgBuildSettingsViewVulkan::ProjectBuildSettingsSaved,
                           this,
                           &RgBuildView::HandleBuildSettingsSaved);
    assert(is_connected);

    // Connect to build settings view's edit line's "focus in" event to color the frame red.
    is_connected = connect(static_cast<RgBuildSettingsViewVulkan*>(build_settings_view_),
                           &RgBuildSettingsViewVulkan::SetFrameBorderRedSignal,
                           this,
                           &RgBuildView::HandleSetFrameBorderRed);
    assert(is_connected);

    // Connect to build settings view's edit line's "focus out" event to color the frame black.
    is_connected = connect(static_cast<RgBuildSettingsViewVulkan*>(build_settings_view_),
                           &RgBuildSettingsViewVulkan::SetFrameBorderBlackSignal,
                           this,
                           &RgBuildView::HandleSetFrameBorderBlack);
    assert(is_connected);
}

void RgBuildViewVulkan::CurrentBuildCancelled()
{
    // Remove all the generated files in output directory.
    DestroyProjectBuildArtifacts();
}

void RgBuildViewVulkan::SetDefaultFocusWidget() const
{
    assert(build_settings_view_ != nullptr);
    if (build_settings_view_ != nullptr)
    {
        build_settings_view_->SetInitialWidgetFocus();
    }
}

void RgBuildViewVulkan::SetPsoEditorDefaultFocusWidget() const
{
    assert(pipeline_state_view_ != nullptr);
    if (pipeline_state_view_ != nullptr)
    {
        pipeline_state_view_->SetInitialWidgetFocus();
    }
}

void RgBuildViewVulkan::SetAPISpecificBorderColor()
{
    HandleSetFrameBorderRed();
}

bool RgBuildViewVulkan::ConnectMenuSignals()
{
    // Connect the actions for the context menu.
    bool is_connected = connect(action_add_existing_file_, &QAction::triggered, this, &RgBuildViewVulkan::HandleAddExistingFile);
    assert(is_connected);
    is_connected = connect(action_create_new_file_, &QAction::triggered, this, &RgBuildViewVulkan::HandleCreateNewFile);
    assert(is_connected);

    RgMenuGraphics* menu = static_cast<RgMenuGraphics*>(file_menu_);
    assert(menu != nullptr);
    if (menu != nullptr)
    {
        // Connect the file menu's "Add existing file" button handler.
        is_connected = connect(menu, &RgMenuGraphics::AddExistingFileButtonClicked, this, &RgBuildViewVulkan::HandleAddFileButtonClicked);
        assert(is_connected);

        // Connect the file menu item's drag and drop handler.
        is_connected = connect(menu, &RgMenuGraphics::DragAndDropExistingFile, this, &RgBuildViewVulkan::HandleExistingFileDragAndDrop);
        assert(is_connected);

        // Connect the load PSO file handler with drag and drop file.
        is_connected = connect(menu, &RgMenuGraphics::DragAndDropExistingPsoFile, this, &RgBuildViewVulkan::HandleLoadPipelineStateFile);
        assert(is_connected);

        // Connect the file menu's "Remove file" button handler.
        is_connected = connect(menu, &RgMenuGraphics::RemoveFileButtonClicked, this, &RgBuildViewVulkan::HandleRemoveFileButtonClicked);
        assert(is_connected);

        // Connect the file menu's "Restore original SPIR-V binary" handler.
        is_connected = connect(menu, &RgMenuGraphics::RestoreOriginalSpirvClicked, this, &RgBuildViewVulkan::HandleRestoreOriginalSpvClicked);
        assert(is_connected);

        // Connect the file menu item selection handler for each new item.
        is_connected = connect(menu, &RgMenuGraphics::MenuItemClicked, this, &RgBuildViewVulkan::HandleMenuItemClicked);
        assert(is_connected);

        // Connect the file menu item focus next view signal.
        is_connected = connect(menu, &RgMenuGraphics::FocusNextView, this, &RgBuildView::HandleFocusNextView);
        assert(is_connected);

        // Connect the file menu item focus previous view signal.
        is_connected = connect(menu, &RgMenuGraphics::FocusPrevView, this, &RgBuildView::HandleFocusPrevView);
        assert(is_connected);

        // Connect the file menu clicked signal.
        is_connected = connect(menu, &RgMenuGraphics::MenuClicked, this, &RgBuildViewVulkan::HandlePipelineStateTreeFocusOut);
        assert(is_connected);

        // Connect the "Pipeline state" button in the file menu.
        RgMenuPipelineStateItem* pipeline_state_item = menu->GetPipelineStateItem();
        assert(pipeline_state_item != nullptr);
        if (pipeline_state_item != nullptr)
        {
            is_connected = connect(
                pipeline_state_item, &RgMenuPipelineStateItem::PipelineStateButtonClicked, this, &RgBuildViewVulkan::HandlePipelineStateMenuItemClicked);
            assert(is_connected);
        }

        // Connect the "Build settings" button in the file menu.
        RgMenuBuildSettingsItem* build_settings_item = menu->GetBuildSettingsItem();
        assert(build_settings_item != nullptr);
        if (build_settings_item != nullptr)
        {
            is_connected = connect(
                build_settings_item, &RgMenuBuildSettingsItem::BuildSettingsButtonClicked, this, &RgBuildViewVulkan::HandleBuildSettingsMenuItemClicked);
            assert(is_connected);
        }
    }

    return is_connected;
}

void RgBuildViewVulkan::HandleBuildSettingsMenuItemClicked()
{
    // Disable the Ctrl+S menu option.
    emit EnableSaveSettingsMenuItem(false);
}

bool RgBuildViewVulkan::HandlePipelineStateFileSaved()
{
    bool        is_ok = false;
    std::string error_string;

    std::shared_ptr<RgProjectCloneVulkan> vulkan_clone = std::dynamic_pointer_cast<RgProjectCloneVulkan>(project_->clones[clone_index_]);

    assert(vulkan_clone != nullptr);
    if (vulkan_clone != nullptr)
    {
        // Determine which PSO state file is being edited.
        const RgPipelineState& current_pso_state = vulkan_clone->pso_states[pipeline_state_index_];

        // Save the pipeline state file.
        is_ok = pipeline_state_model_->SavePipelineStateFile(current_pso_state.pipeline_state_file_path, error_string);
        assert(is_ok);
    }

    assert(is_ok);
    if (!is_ok)
    {
        // The pipeline state failed to save properly. Is the user currently viewing the PSO editor?
        if (edit_mode_ != EditMode::kPipelineSettings)
        {
            // If the user isn't in the PSO editor, switch to it so they can resolve the problem.
            SwitchEditMode(EditMode::kPipelineSettings);
        }

        // Show the error string to the user, letting them know that the PSO file was not saved correctly.
        std::stringstream error_stream;
        error_stream << kStrErrCannotSavePipelineStateFile;
        error_stream << " ";
        error_stream << error_string;
        RgUtils::ShowErrorMessageBox(error_stream.str().c_str(), this);
    }

    return is_ok;
}

void RgBuildViewVulkan::HandlePipelineStateFileLoaded()
{
    // Open the file browser in the last selected directory.
    std::string pipeline_state_file_path = RgConfigManager::Instance().GetLastSelectedFolder();

    std::shared_ptr<RgProjectCloneVulkan> vulkan_clone = std::dynamic_pointer_cast<RgProjectCloneVulkan>(project_->clones[clone_index_]);

    assert(vulkan_clone != nullptr);
    if (vulkan_clone != nullptr)
    {
        // Select the relevant file filter based on the pipeline type (compute, graphics).
        bool        is_graphics_pipeline = (vulkan_clone->pipeline.type == RgPipelineType::kGraphics);
        const char* pso_file_filter = is_graphics_pipeline ? kStrDefaultPipelineFileExtensionFilterGraphics : kStrDefaultPipelineFileExtensionFilterCompute;

        // Display an "Open file" dialog to let the user choose
        // which pipeline state configuration file to use.
        bool is_ok = RgUtils::OpenFileDialog(this, pipeline_state_file_path, kStrPipelineStateFileDialogCaption, pso_file_filter);

        if (is_ok)
        {
            HandleLoadPipelineStateFile(pipeline_state_file_path);
        }
    }
}

bool RgBuildViewVulkan::LoadPipelineStateFile(const std::string& pipeline_state_file_path)
{
    bool is_ok = false;

    std::shared_ptr<RgProjectCloneVulkan> vulkan_clone = std::dynamic_pointer_cast<RgProjectCloneVulkan>(project_->clones[clone_index_]);

    std::string error_string;
    assert(vulkan_clone != nullptr);
    if (vulkan_clone != nullptr)
    {
        // Determine which pipeline state file is being edited.
        RgPipelineState& current_pso_state = vulkan_clone->pso_states[pipeline_state_index_];

        // Attempt to load the pipeline state file in the model.
        is_ok = pipeline_state_model_->LoadPipelineStateFile(this, pipeline_state_file_path, vulkan_clone->pipeline.type, error_string);
        assert(is_ok);
        if (is_ok)
        {
            // Assign the pipeline state file in the project.
            current_pso_state.original_pipeline_state_file_path = pipeline_state_file_path;

            // Initialize the model.
            pipeline_state_view_->InitializeModel(pipeline_state_model_);
        }
    }

    assert(is_ok);
    if (is_ok)
    {
        // Emit a signal indicating that the specified PSO file has been loaded successfully.
        emit PsoFileLoaded();
    }
    else
    {
        // Show the error string to the user, letting them know that the PSO file was not saved correctly.
        std::stringstream error_stream;
        error_stream << kStrErrCannotLoadPipelineStateFile;
        error_stream << " ";
        error_stream << error_string;
        emit SetStatusBarText(error_stream.str().c_str());
    }

    return is_ok;
}

void RgBuildViewVulkan::HandlePipelineStateMenuItemClicked(RgMenuPipelineStateItem* item)
{
    if (ShowSaveDialog())
    {
        SwitchEditMode(EditMode::kPipelineSettings);

        assert(file_menu_ != nullptr);
        if (file_menu_ != nullptr)
        {
            file_menu_->HandlePipelineStateButtonClicked(true);
        }

        assert(item != nullptr);
        if (item != nullptr)
        {
            item->SetCurrent(true);
        }

        // Reset the find text widget.
        assert(pipeline_state_view_ != nullptr);
        if (pipeline_state_view_ != nullptr)
        {
            pipeline_state_view_->ResetSearch();
        }

        // Set the PSO editor frame color to Vulkan red.
        HandlePipelineStateTreeFocusIn();

        // Enable the Ctrl+S menu option.
        emit EnableSaveSettingsMenuItem(true);
    }
}

void RgBuildViewVulkan::HandleMenuItemClicked(RgMenuFileItem* item)
{
    if (ShowSaveDialog())
    {
        file_menu_->HandleSelectedFileChanged(item);
    }
}

RgMenu* RgBuildViewVulkan::GetMenu() const
{
    return file_menu_;
}

RgPipelineStateModel* RgBuildViewVulkan::GetPipelineStateModel()
{
    return pipeline_state_model_;
}

bool RgBuildViewVulkan::PopulateMenu()
{
    bool is_file_loaded = false;

    std::shared_ptr<RgProjectCloneVulkan> vulkan_clone = std::dynamic_pointer_cast<RgProjectCloneVulkan>(project_->clones[clone_index_]);

    assert(vulkan_clone != nullptr);
    if (vulkan_clone != nullptr)
    {
        // Determine the type of pipeline to populate the menu widget with.
        bool is_graphics_pipeline = (vulkan_clone->pipeline.type == RgPipelineType::kGraphics);

        // 1st non-empty stage item.
        size_t first_non_empty_stage = static_cast<size_t>(-1);

        // Step through each stage type in a graphics pipeline by default.
        size_t first_stage = static_cast<size_t>(RgPipelineStage::kVertex);
        size_t last_stage  = static_cast<size_t>(RgPipelineStage::kFragment);

        // If a compute pipeline is being loaded, only attempt to load the single Compute stage.
        if (!is_graphics_pipeline)
        {
            first_stage = last_stage = static_cast<size_t>(RgPipelineStage::kCompute);
        }

        for (size_t stage_index = first_stage; stage_index <= last_stage; ++stage_index)
        {
            const std::string& stage_file_path = vulkan_clone->pipeline.shader_stages[stage_index];

            if (!stage_file_path.empty())
            {
                // Check that the file still exists before attempting to load it.
                bool is_file_exists = RgUtils::IsFileExists(stage_file_path);
                assert(is_file_exists);
                if (is_file_exists)
                {
                    // Add the selected file to the menu.
                    SetStageSourceFile(static_cast<RgPipelineStage>(stage_index), stage_file_path);
                    first_non_empty_stage = (first_non_empty_stage == -1 ? stage_index : first_non_empty_stage);
                }
                else
                {
                    // Build an error string saying the file couldn't be found on disk.
                    std::stringstream error_string;
                    error_string << kStrErrCannotLoadSourceFileMsg;
                    error_string << stage_file_path;

                    // Show the user the error message.
                    RgUtils::ShowErrorMessageBox(error_string.str().c_str(), this);

                    is_file_loaded = true;
                }
            }
        }

        // Select the 1st non-empty File Menu item.
        assert(file_menu_ != nullptr);
        if (file_menu_ != nullptr && first_non_empty_stage != -1)
        {
            auto menu_item = file_menu_->GetStageItem(static_cast<RgPipelineStage>(first_non_empty_stage));
            if (menu_item != nullptr && !menu_item->GetFilename().empty())
            {
                file_menu_->HandleSelectedFileChanged(menu_item);
            }
        }
    }

    return !is_file_loaded;
}

bool RgBuildViewVulkan::IsGcnDisassemblyGenerated(const std::string& input_file_path) const
{
    bool is_current_file_disassembled = false;

    auto targetGpuOutputsIter = build_outputs_.find(current_target_gpu_);
    if (targetGpuOutputsIter != build_outputs_.end())
    {
        std::shared_ptr<RgCliBuildOutputPipeline> build_output = std::dynamic_pointer_cast<RgCliBuildOutputPipeline>(targetGpuOutputsIter->second);

        assert(build_output != nullptr);
        if (build_output != nullptr)
        {
            auto input_file_iter = build_output->per_file_output.find(input_file_path);
            if (input_file_iter != build_output->per_file_output.end())
            {
                RgFileOutputs& file_outputs  = input_file_iter->second;
                is_current_file_disassembled = !file_outputs.outputs.empty();
            }
        }
    }

    return is_current_file_disassembled;
}

bool RgBuildViewVulkan::LoadSessionMetadata(const std::string& metadata_file_path, std::shared_ptr<RgCliBuildOutput>& build_output)
{
    bool ret = false;

    std::shared_ptr<RgCliBuildOutputPipeline> gpu_output_vulkan = nullptr;

    ret = RgXMLSessionConfig::ReadSessionMetadataVulkan(metadata_file_path, gpu_output_vulkan);
    assert(ret);
    if (ret)
    {
        build_output = gpu_output_vulkan;
    }

    return ret;
}

void RgBuildViewVulkan::ReloadFile(const std::string& file_path)
{
    assert(file_menu_ != nullptr);
    if (file_menu_ != nullptr)
    {
        RgMenuFileItemGraphics* file_item = static_cast<RgMenuFileItemGraphics*>(file_menu_->GetFileItemFromPath(file_path));
        assert(file_item);
        if (file_item != nullptr)
        {
            // If it's a SPIR-V binary file, disassemble it and update the disassembly text in the code editor.
            if (file_item->GetFileType() == RgVulkanInputType::kSpirv)
            {
                std::string        cli_output, compiler_path;
                const std::string& disasm_file_path = spv_disasm_files_[file_item->GetStage()];
                assert(!disasm_file_path.empty());
                if (!disasm_file_path.empty())
                {
                    // Get the "alternative compiler path" setting value.
                    std::shared_ptr<RgProjectCloneVulkan> vulkan_clone = std::dynamic_pointer_cast<RgProjectCloneVulkan>(project_->clones[clone_index_]);

                    assert(vulkan_clone != nullptr && vulkan_clone->build_settings != nullptr);
                    if (vulkan_clone != nullptr && vulkan_clone->build_settings != nullptr)
                    {
                        compiler_path = std::get<CompilerFolderType::kBin>(vulkan_clone->build_settings->compiler_paths);
                    }

                    bool is_ok = RgCliLauncher::DisassembleSpvToText(compiler_path, file_path, disasm_file_path, cli_output);
                    HandleNewCLIOutputString(cli_output);
                    if (is_ok)
                    {
                        SetSourceCodeText(disasm_file_path);

                        // Update the last modification time.
                        QFileInfo file_info(file_path.c_str());
                        file_modified_time_map_[current_code_editor_] = file_info.lastModified();
                    }
                }
            }
            else
            {
                SetSourceCodeText(file_path);
            }
        }
    }
}

void RgBuildViewVulkan::ShowCurrentFileDisassembly()
{
    bool is_current_file_disassembled = false;

    // Show the currently selected file's first entry point disassembly (if there is no currently selected entry).
    const std::string& input_filepath     = file_menu_->GetSelectedFilePath();
    RgMenuFileItem*    selected_file_item = file_menu_->GetSelectedFileItem();
    assert(selected_file_item != nullptr);

    if (selected_file_item != nullptr)
    {
        RgMenuFileItemGraphics* file_item_pipeline_stage = static_cast<RgMenuFileItemGraphics*>(selected_file_item);
        assert(file_item_pipeline_stage != nullptr);

        if (file_item_pipeline_stage != nullptr)
        {
            is_current_file_disassembled = IsGcnDisassemblyGenerated(input_filepath);
            if (is_current_file_disassembled)
            {
                // Show the first entry point in the disassembly table.
                const std::string& current_entrypoint_name = kStrDefaultVulkanGlslEntrypointName;
                disassembly_view_->HandleSelectedEntrypointChanged(current_target_gpu_, input_filepath, current_entrypoint_name);

                // Emit a signal indicating that the selected entry point has changed.
                emit SelectedEntrypointChanged(current_target_gpu_, input_filepath, current_entrypoint_name);
            }
        }
    }

    // Toggle the view based on if the current file has been disassembled or not.
    ToggleDisassemblyViewVisibility(is_current_file_disassembled);
}

void RgBuildViewVulkan::SaveFile(RgMenuFileItemGraphics* file_item)
{
    // Get the file editor for the file menu item.
    RgSourceCodeEditor* file_editor = nullptr;
    const std::string   file_path   = file_item->GetFilename();
    auto                iter        = source_code_editors_.find(file_path);
    assert(iter != source_code_editors_.end());
    if (iter != source_code_editors_.end())
    {
        file_editor = iter->second;
    }

    assert(file_editor != nullptr);
    assert(clone_index_ < project_->clones.size());
    if (file_editor != nullptr && file_menu_ != nullptr && (clone_index_ < project_->clones.size()))
    {
        // Check if the file needs reassembling (modified SPIR-V disassembly).
        bool need_reassemble = false;
        assert(file_item != nullptr);
        if (file_item != nullptr)
        {
            need_reassemble = (file_item->GetFileType() == RgVulkanInputType::kSpirv);
        }

        // Check if we're saving a modified disassembled SPIR-V text first time.
        // If so, we have to replace the original spv binary with the modified disassembly file in the Project.
        if (need_reassemble && file_editor->document()->isModified())
        {
            RgPipelineStage    stage                = file_item->GetStage();
            const std::string& spv_disasm_file_path = spv_disasm_files_[stage];
            QFileInfo          file_info(spv_disasm_file_path.c_str());

            // Write the editor text to file if the file path is valid.
            if (!spv_disasm_file_path.empty())
            {
                SaveEditorTextToFile(file_editor, spv_disasm_file_path);
            }

            // Show the Code Editor title message warning the user about replacing original spv binary file
            // with its disassembly in the project.
            const std::string msg = file_info.fileName().toStdString() + ". " + kStrSourceEditorTitlebarSpirvDisasmSavedA + " " +
                                    kStrFileContextMenuRestoreSpv + " " + kStrSourceEditorTitlebarSpirvDisasmSavedB;
            file_editor->SetTitleBarText(msg);

            // If it's the current item, display the message on the title bar.
            if (file_menu_->IsCurrentlySelectedFileItem(file_item) && source_editor_titlebar_ != nullptr)
            {
                source_editor_titlebar_->ShowMessage(msg);
            }

            // Switch "last modified time" to the spv disassembly file.
            file_modified_time_map_[file_editor] = file_info.lastModified();

            // Replace the spv binary file with its disassembly in the project, File Menu and in the Code Editor map.
            const std::string& spv_file_path = file_item->GetFilename();
            auto               editor        = source_code_editors_.find(spv_file_path);
            assert(editor != source_code_editors_.end());
            if (editor != source_code_editors_.end())
            {
                // Replace file path in the Code Editor map.
                source_code_editors_.erase(editor);
                source_code_editors_[spv_disasm_file_path] = file_editor;
            }

            // Store the original spv binary path to the project so that a user can restore it later.
            std::shared_ptr<RgProjectCloneVulkan> vulkan_clone = std::dynamic_pointer_cast<RgProjectCloneVulkan>(project_->clones[clone_index_]);
            assert(vulkan_clone != nullptr);
            if (vulkan_clone != nullptr)
            {
                vulkan_clone->spv_backup_.type                 = vulkan_clone->pipeline.type;
                vulkan_clone->spv_backup_.shader_stages[stage] = spv_file_path;

                // Replace file path in the project.
                vulkan_clone->pipeline.shader_stages[stage] = spv_disasm_file_path;
                RgConfigManager::Instance().SaveProjectFile(project_);
            }

            assert(file_menu_ != nullptr);
            if (file_menu_ != nullptr)
            {
                // Replace file path in the File Menu.
                file_menu_->ReplaceStageFile(stage, spv_disasm_file_path, RgVulkanInputType::kSpirvTxt);

                // Add "restore spv" option to the file menu item.
                file_menu_->GetStageItem(stage)->AddContextMenuActionRestoreSpv();
            }

            // Replace file path in the Build Output.
            ReplaceInputFileInBuildOutput(spv_file_path, spv_disasm_file_path);

            // Replace file path in the Disassembly View tables.
            if (disassembly_view_ != nullptr)
            {
                disassembly_view_->ReplaceInputFilePath(spv_file_path, spv_disasm_file_path);
            }
        }
    }
}

void RgBuildViewVulkan::SaveCurrentFile(EditMode mode)
{
    if (mode == EditMode::kSourceCode)
    {
        assert(current_code_editor_ != nullptr && source_editor_titlebar_ != nullptr && file_menu_ != nullptr);
        if (current_code_editor_ != nullptr && source_editor_titlebar_ != nullptr && file_menu_ != nullptr)
        {
            bool need_disassemble_spv = false;

            RgMenuFileItemGraphics* current_menu_item = file_menu_->GetCurrentFileMenuItem();
            if (current_menu_item != nullptr)
            {
                need_disassemble_spv = (current_menu_item->GetFileType() == RgVulkanInputType::kSpirv);
            }

            // Check if we're saving a modified disassembled SPIR-V text first time.
            // If so, we have to replace the original spv binary with the modified disassembly file in the Project.
            if (need_disassemble_spv && current_code_editor_->document()->isModified())
            {
                RgPipelineStage    stage                = file_menu_->GetCurrentStage();
                const std::string& spv_disasm_file_path = spv_disasm_files_[stage];
                QFileInfo          file_info(spv_disasm_file_path.c_str());

                // Write the editor text to file if the file path is valid.
                if (!spv_disasm_file_path.empty())
                {
                    SaveEditorTextToFile(current_code_editor_, spv_disasm_file_path);
                }

                // Show the Code Editor title message warning the user about replacing original spv binary file
                // with its disassembly in the project.
                const std::string msg = file_info.fileName().toStdString() + ". " + kStrSourceEditorTitlebarSpirvDisasmSavedA + " " +
                                        kStrFileContextMenuRestoreSpv + " " + kStrSourceEditorTitlebarSpirvDisasmSavedB;
                current_code_editor_->SetTitleBarText(msg);
                source_editor_titlebar_->ShowMessage(msg);

                // Switch "last modified time" to the spv disassembly file.
                file_modified_time_map_[current_code_editor_] = file_info.lastModified();

                // Replace the spv binary file with its disassembly in the project, File Menu and in the Code Editor map.
                const std::string& spv_file_path = file_menu_->GetSelectedFilePath();
                auto               editor        = source_code_editors_.find(spv_file_path);
                assert(editor != source_code_editors_.end());
                if (editor != source_code_editors_.end())
                {
                    // Replace file path in the Code Editor map.
                    source_code_editors_.erase(editor);
                    source_code_editors_[spv_disasm_file_path] = current_code_editor_;
                }

                // Store the original spv binary path to the project so that a user can restore it later.
                std::shared_ptr<RgProjectCloneVulkan> vulkan_clone = std::dynamic_pointer_cast<RgProjectCloneVulkan>(project_->clones[clone_index_]);
                assert(vulkan_clone != nullptr);
                if (vulkan_clone != nullptr)
                {
                    vulkan_clone->spv_backup_.type                 = vulkan_clone->pipeline.type;
                    vulkan_clone->spv_backup_.shader_stages[stage] = spv_file_path;

                    // Replace file path in the project.
                    vulkan_clone->pipeline.shader_stages[stage] = spv_disasm_file_path;
                    RgConfigManager::Instance().SaveProjectFile(project_);
                }

                assert(file_menu_ != nullptr);
                if (file_menu_ != nullptr)
                {
                    // Replace file path in the File Menu.
                    file_menu_->ReplaceStageFile(stage, spv_disasm_file_path, RgVulkanInputType::kSpirvTxt);

                    // Add "restore spv" option to the file menu item.
                    file_menu_->GetStageItem(stage)->AddContextMenuActionRestoreSpv();
                }

                // Replace file path in the Build Output.
                ReplaceInputFileInBuildOutput(spv_file_path, spv_disasm_file_path);

                // Replace file path in the Disassembly View tables.
                if (disassembly_view_ != nullptr)
                {
                    disassembly_view_->ReplaceInputFilePath(spv_file_path, spv_disasm_file_path);
                }
            }
            else
            {
                RgBuildView::SaveCurrentFile(mode);
            }
        }
    }
    else if (mode == EditMode::kPipelineSettings)
    {
        // Save the PSO editor state.
        HandlePipelineStateFileSaved();
    }
    else if (mode == EditMode::kBuildSettings)
    {
        // Don't need to do anything here.
        // Any changes will be saved by another part of the code.
    }
    else
    {
        // Shouldn't get here.
        assert(false);
    }
}

void RgBuildViewVulkan::SaveSourceFile(const std::string& source_file_path)
{
    // Do not save the file if it's a SPIR-V disassembly and the corresponding File Menu item
    // has the "SPIR-V binary" file type.
    bool is_spv_binary = false;
    if (file_menu_ != nullptr)
    {
        RgMenuFileItemGraphics* file_item = static_cast<RgMenuFileItemGraphics*>(file_menu_->GetFileItemFromPath(source_file_path));
        assert(file_item != nullptr);
        if (file_item != nullptr)
        {
            is_spv_binary = (file_item->GetFileType() == RgVulkanInputType::kSpirv);
        }
    }

    if (!is_spv_binary)
    {
        RgBuildView::SaveSourceFile(source_file_path);
    }
}

bool RgBuildViewVulkan::SaveCurrentState()
{
    // Save the pipeline state.
    bool status = HandlePipelineStateFileSaved();

    assert(status);
    if (status)
    {
        // Save all file items in the Vulkan way (meaning, check if they
        // are SPIR-V files that need to be reassembled and act accordingly.
        RgMenuGraphics* menu = static_cast<RgMenuGraphics*>(GetMenu());
        assert(menu != nullptr);
        if (menu != nullptr)
        {
            std::vector<RgMenuFileItem*> file_items = menu->GetAllFileItems();
            for (RgMenuFileItem* item : file_items)
            {
                RgMenuFileItemGraphics* item_graphics = static_cast<RgMenuFileItemGraphics*>(item);
                assert(item_graphics != nullptr);
                if (item_graphics != nullptr)
                {
                    SaveFile(item_graphics);
                }
            }
        }

        // Call the base implementation to save the remaining files/settings.
        return RgBuildView::SaveCurrentState();
    }

    return status;
}

void RgBuildViewVulkan::ConnectPipelineStateViewSignals()
{
    assert(pipeline_state_view_ != nullptr);
    if (pipeline_state_view_ != nullptr)
    {
        // Connect the save button handler.
        bool is_connected = connect(pipeline_state_view_, &RgPipelineStateView::SaveButtonClicked, this, &RgBuildViewVulkan::HandlePipelineStateFileSaved);
        assert(is_connected);

        // Connect the load button handler.
        is_connected = connect(pipeline_state_view_, &RgPipelineStateView::LoadButtonClicked, this, &RgBuildViewVulkan::HandlePipelineStateFileLoaded);
        assert(is_connected);

        // Connect the load PSO file handler with drag and drop file.
        is_connected = connect(pipeline_state_view_, &RgPipelineStateView::DragAndDropExistingFile, this, &RgBuildViewVulkan::HandleLoadPipelineStateFile);
        assert(is_connected);

        // Connect the PSO file loaded handler.
        is_connected = connect(this, &RgBuildViewGraphics::PsoFileLoaded, pipeline_state_view_, &RgPipelineStateView::HandlePsoFileLoaded);
        assert(is_connected);

        // Connect the pipeline state view's editor's resized handler.
        is_connected = connect(pipeline_state_view_, &RgPipelineStateView::EditorResized, this, &RgBuildViewGraphics::HandleStateEditorResized);
        assert(is_connected);

        // Connect the pipeline state tree in focus signal.
        is_connected = connect(pipeline_state_view_, &RgPipelineStateView::PipelineStateTreeFocusIn, this, &RgBuildViewVulkan::HandlePipelineStateTreeFocusIn);
        assert(is_connected);

        // Connect the pipeline state tree focus out signal.
        is_connected =
            connect(pipeline_state_view_, &RgPipelineStateView::PipelineStateTreeFocusOut, this, &RgBuildViewVulkan::HandlePipelineStateTreeFocusOut);
        assert(is_connected);

        is_connected = connect(view_manager_, &RgViewManager::PsoEditorWidgetFocusOutSignal, this, &RgBuildViewVulkan::HandlePipelineStateTreeFocusOut);
        assert(is_connected);

        is_connected = connect(view_manager_, &RgViewManager::PsoEditorWidgetFocusInSignal, this, &RgBuildViewVulkan::HandlePipelineStateTreeFocusIn);
        assert(is_connected);
    }
}

bool RgBuildViewVulkan::InitializeModeSpecificViews()
{
    bool status = false;

    // Create the pipeline state model.
    status = CreatePipelineStateModel();

    // Create the Vulkan Pipeline State editor view.
    if (status)
    {
        CreatePipelineStateView(this);
    }

    return status;
}

void RgBuildViewVulkan::FocusOnFileMenu()
{
    // Switch the focus to the file menu.
    if (file_menu_ != nullptr)
    {
        file_menu_->setFocus();
    }
}

bool RgBuildViewVulkan::CreateDefaultGraphicsPipeline()
{
    return CreateProject(RgPipelineType::kGraphics);
}

bool RgBuildViewVulkan::CreateDefaultComputePipeline()
{
    return CreateProject(RgPipelineType::kCompute);
}

void RgBuildViewVulkan::HandleFileRenamed(const std::string& old_file_path, const std::string& new_file_path)
{
    auto editor_iter = source_code_editors_.find(old_file_path);
    if (editor_iter != source_code_editors_.end())
    {
        RgSourceCodeEditor* editor = editor_iter->second;

        assert(editor != nullptr);
        if (editor != nullptr)
        {
            // Erase the existing file path, and insert the new one.
            source_code_editors_.erase(editor_iter);
            source_code_editors_[new_file_path] = editor;
        }
    }

    // Update the project's source file list with the new filepath.
    std::shared_ptr<RgProjectVulkan> project = std::dynamic_pointer_cast<RgProjectVulkan>(project_);
    assert(project != nullptr);
    if (project != nullptr)
    {
        RgConfigManager::UpdateShaderStageFilePath(old_file_path, new_file_path, project, clone_index_);

        // Save the updated project file.
        RgConfigManager::Instance().SaveProjectFile(project);
    }
}

void RgBuildViewVulkan::HandleSelectedFileChanged(const std::string& old_file_path, const std::string& new_file_path)
{
    Q_UNUSED(old_file_path);

    // Get a pointer to the editor responsible for displaying the new file.
    RgSourceCodeEditor* editor = GetEditorForFilepath(new_file_path);
    assert(editor != nullptr);
    if (editor != nullptr)
    {
        bool is_editor_wwitched = SwitchToEditor(editor);
        if (is_editor_wwitched)
        {
            // Switch the disassembly view to show the currently-selected entry point in the newly-selected file item.
            if (disassembly_view_ != nullptr && !is_build_in_progress_)
            {
                // Open the disassembly view for the source file only if it's disassembled.
                if (IsGcnDisassemblyGenerated(new_file_path))
                {
                    // Update the visibility of the disassembly view.
                    ToggleDisassemblyViewVisibility(true);

                    // Make the disassembly view visible because the file has build outputs to display.
                    std::string selected_entrypoint_name = kStrDefaultVulkanGlslEntrypointName;
                    emit        SelectedEntrypointChanged(current_target_gpu_, new_file_path, selected_entrypoint_name);

                    // Update the titlebar for the current source editor.
                    UpdateSourceEditorTitlebar(editor);
                }
                else
                {
                    // Hide the disassembly view when switching to a file that hasn't been disassembled.
                    ToggleDisassemblyViewVisibility(false);
                }

                // Disable/enable the Edit->Go to live VGPR option.
                emit EnableShowMaxVgprOptionSignal(disassembly_view_->IsMaxVgprColumnVisible());

                // Also enable/disable the context menu item.
                disassembly_view_->EnableShowMaxVgprContextOption();
            }
        }
    }
}

void RgBuildViewVulkan::HandleSourceFileSelectedLineChanged(RgSourceCodeEditor* editor, int line_number)
{
    Q_UNUSED(line_number);

    // Handle updating source correlation only when the project isn't currently being built.
    if (!is_build_in_progress_)
    {
        if (disassembly_view_ != nullptr && !disassembly_view_->IsEmpty())
        {
            const std::string& input_filename = GetFilepathForEditor(editor);
            bool               isDisassembled = IsGcnDisassemblyGenerated(input_filename);
            if (isDisassembled)
            {
                int correlated_line_number = kInvalidCorrelationLineIndex;

                // If the line is associated with a named entry point, highlight it in the file menu item.
                std::string entry_name = kStrDefaultVulkanGlslEntrypointName;

                // Send the input source file's correlation line index to the disassembly view.
                disassembly_view_->HandleInputFileSelectedLineChanged(current_target_gpu_, input_filename, entry_name, correlated_line_number);
            }
        }
    }
}

void RgBuildViewVulkan::HandleAddFileButtonClicked(RgPipelineStage stage)
{
    if (ShowSaveDialog())
    {
        // Check if the user would like to add an existing file or create a default one.
        // Set the current stage.
        stage_clicked_ = stage;

        // Show the context menu for adding/creating file.
        add_create_context_menu_->exec(QCursor::pos());
    }
}

void RgBuildViewVulkan::HandleAddExistingFile()
{
    assert(file_menu_ != nullptr);
    if (file_menu_ != nullptr)
    {
        std::string file_path_to_add;
        bool        is_ok = RgUtils::OpenFileDialog(this, RgProjectAPI::kVulkan, file_path_to_add);
        if (is_ok && !file_path_to_add.empty() && RgUtils::IsFileExists(file_path_to_add) && !file_menu_->IsFileInMenu(file_path_to_add))
        {
            // Convert the path separators to match the style used in the session metadata.
            std::string native_file_path = QDir::toNativeSeparators(file_path_to_add.c_str()).toStdString();
            SetStageSourceFile(stage_clicked_, native_file_path);

            // Insert the new source file path into the pipeline.
            RgConfigManager& config_manager = RgConfigManager::Instance();
            config_manager.AddShaderStage(stage_clicked_, native_file_path, project_, clone_index_);

            // Save the pipeline project now that the new file is added to the stage.
            config_manager.SaveProjectFile(project_);

            // Remove focus from build settings and pipeline state buttons.
            RgMenuVulkan* vulkan_file_menu = static_cast<RgMenuVulkan*>(file_menu_);
            assert(vulkan_file_menu != nullptr);
            if (vulkan_file_menu != nullptr)
            {
                vulkan_file_menu->SetButtonsNoFocus();
            }
        }
        else if (!file_path_to_add.empty())
        {
            // Inform the user the the file already exists.
            std::stringstream msg;
            msg << kStrErrCannotAddFileA << file_path_to_add.front() << kStrErrCannotAddFileB;
            RgUtils::ShowErrorMessageBox(msg.str().c_str(), this);
        }
    }
}

void RgBuildViewVulkan::HandleCreateNewFile()
{
    // Create a new source file with the default filename.
    std::string base_filename = kStrDefaultSourceFilename;

    // Attempt to create a new source file for the given stage. The final filename may differ
    // from the requested name if the requested filename already exists in the project.
    std::string pathFinalFilePath;
    CreateNewSourceFile(stage_clicked_, base_filename, pathFinalFilePath);

    // Show the contents of the new file within the editor.
    SetSourceCodeText(pathFinalFilePath);

    // Update the focus indices.
    assert(file_menu_ != nullptr);
    if (file_menu_ != nullptr)
    {
        file_menu_->UpdateFocusIndex();
    }
}

void RgBuildViewVulkan::HandleExistingFileDragAndDrop(RgPipelineStage stage, const std::string& file_path_to_add)
{
    assert(file_menu_ != nullptr);
    if (file_menu_ != nullptr && !file_path_to_add.empty())
    {
        if (!file_menu_->IsFileInMenu(file_path_to_add) && RgUtils::IsFileExists(file_path_to_add))
        {
            // Convert the path separators to match the style used in the session metadata.
            std::string native_file_path = QDir::toNativeSeparators(file_path_to_add.c_str()).toStdString();
            SetStageSourceFile(stage, native_file_path);

            // Insert the new source file path into the pipeline.
            RgConfigManager& config_manager = RgConfigManager::Instance();
            config_manager.AddShaderStage(stage, native_file_path, project_, clone_index_);

            // Save the pipeline project now that the new file is added to the stage.
            config_manager.SaveProjectFile(project_);
        }
        else
        {
            // Inform the user the the file already exists.
            std::stringstream msg;
            msg << kStrErrCannotAddFileA << file_path_to_add << kStrErrCannotAddFileB;
            RgUtils::ShowErrorMessageBox(msg.str().c_str(), this);

            // Restore the item from being current (remove the highlight
            // that was assigned to it when it was hovered in the drag process).
            file_menu_->SetCurrent(stage, false);
        }
    }
}

void RgBuildViewVulkan::HandleRemoveFileButtonClicked(RgPipelineStage stage)
{
    if (ShowSaveDialog())
    {
        bool project_exists = (project_ != nullptr);
        if (project_exists)
        {
            // Ensure that the incoming clone index is valid for the current project.
            bool is_valid_range = (clone_index_ >= 0 && clone_index_ < project_->clones.size());
            assert(is_valid_range);

            if (is_valid_range)
            {
                std::shared_ptr<RgGraphicsProjectClone> graphics_clone = std::dynamic_pointer_cast<RgGraphicsProjectClone>(project_->clones[clone_index_]);

                std::string stage_file_path;
                bool        is_stage_occupied = RgConfigManager::Instance().GetShaderStageFilePath(stage, graphics_clone, stage_file_path);

                assert(is_stage_occupied);
                if (is_stage_occupied)
                {
                    std::stringstream msg;
                    msg << stage_file_path << kStrMenuBarConfirmRemoveFileDialogWarning;

                    // Ask the user if they really want to remove the source file.
                    if (ShowRemoveFileConfirmation(msg.str(), stage_file_path))
                    {
                        // Remove the source file's path from the project's clone.
                        RgConfigManager::Instance().RemoveShaderStage(stage, graphics_clone);

                        // Save the project after removing a source file.
                        RgConfigManager::Instance().SaveProjectFile(project_);

                        // Update the file menu to remove the filename from the stage item.
                        RgMenuVulkan* vulkan_file_menu = static_cast<RgMenuVulkan*>(file_menu_);
                        assert(vulkan_file_menu != nullptr);
                        if (vulkan_file_menu != nullptr)
                        {
                            bool switch_to_next_file = false;

                            // Remove the source file from the stage item.
                            vulkan_file_menu->ClearStageSourceFile(stage);

                            // Clear out the source view only if this is the currently displayed file.
                            RgMenuFileItemGraphics* stage_item = vulkan_file_menu->GetStageItem(stage);
                            assert(stage_item != nullptr);
                            if (stage_item != nullptr)
                            {
                                // Set all of the sub buttons to have pointing hand cursor.
                                stage_item->SetCursor(Qt::PointingHandCursor);

                                // Clear the edit mode.
                                if (edit_mode_ == EditMode::kSourceCode && vulkan_file_menu->GetCurrentStage() == stage_item->GetStage())
                                {
                                    switch_to_next_file = true;

                                    // Clear out the source view only if one of the buttons
                                    // isn't currently selected.
                                    if (!vulkan_file_menu->IsButtonPressed())
                                    {
                                        SwitchEditMode(EditMode::kEmpty);
                                    }
                                }
                            }

                            RemoveEditor(stage_file_path, switch_to_next_file);

                            // Set the focus to the file menu so subsequent tabs will be processed.
                            vulkan_file_menu->setFocus();

                            // Emit a signal to update various menu items.
                            const bool is_menu_empty = vulkan_file_menu->IsEmpty();
                            emit       vulkan_file_menu->FileMenuItemCountChanged(is_menu_empty);
                        }

                        DestroyBuildOutputsForFile(stage_file_path);

                        // Remove the file's build outputs from the disassembly view.
                        if (disassembly_view_ != nullptr)
                        {
                            disassembly_view_->RemoveInputFileEntries(stage_file_path);

                            // Hide the disassembly view when there's no data in it.
                            if (disassembly_view_->IsEmpty())
                            {
                                // Minimize the disassembly view before hiding it to preserve correct RgBuildView layout.
                                disassembly_view_splitter_->Restore();

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

void RgBuildViewVulkan::HandleRestoreOriginalSpvClicked(RgPipelineStage stage)
{
    auto vulkan_clone = std::dynamic_pointer_cast<RgProjectCloneVulkan>(project_->clones[clone_index_]);

    RgMenuFileItemGraphics* file_item = file_menu_->GetStageItem(stage);
    assert(file_item != nullptr && file_menu_ != nullptr);
    if (file_item != nullptr && file_menu_ != nullptr)
    {
        const std::string spv_disasm = file_item->GetFilename();
        const std::string orig_spv   = vulkan_clone->spv_backup_.shader_stages[stage];

        if (ShowRevertToSpvBinaryConfirmation(orig_spv))
        {
            // Replace files in the project clone.
            vulkan_clone->pipeline.shader_stages[stage] = orig_spv;
            vulkan_clone->spv_backup_.shader_stages[stage].clear();

            // Save the project file.
            RgConfigManager::Instance().SaveProjectFile(project_);

            // Replace file path in the Code Editor map.
            source_code_editors_.erase(spv_disasm);
            source_code_editors_[orig_spv] = current_code_editor_;

            // Delete the edited backup SPIR-V text file from disk.
            QFile file(spv_disasm.c_str());
            file.remove();

            // Update the modification date.
            QFileInfo file_info(orig_spv.c_str());
            file_modified_time_map_[current_code_editor_] = file_info.lastModified();

            // Disassemble the original SPIR-V binary.
            std::string spv_disasm_output;
            if (DisasmSpvFile(orig_spv, spv_disasm_output))
            {
                SetSourceCodeText(spv_disasm_output);

                // Store the path to the disassembled file so that we know where to save the modified disassembly text later.
                spv_disasm_files_[stage] = spv_disasm_output;

                // Store the spv last modification time.
                QFileInfo spv_file_info(orig_spv.c_str());
                file_modified_time_map_[current_code_editor_] = spv_file_info.lastModified();
            }

            // Replace file path in the File Menu.
            file_menu_->ReplaceStageFile(stage, orig_spv, RgVulkanInputType::kSpirv);

            // Replace file path in the Build Output.
            ReplaceInputFileInBuildOutput(spv_disasm_output, orig_spv);

            // Replace file path in the Disassembly View tables.
            if (disassembly_view_ != nullptr)
            {
                disassembly_view_->ReplaceInputFilePath(spv_disasm_output, orig_spv);
            }

            // Remove the "Restore original SPIR-V binary" item from the File Menu's context menu.
            file_item->RemoveContextMenuActionRestoreSpv();

            // Clear the Source Editor Title message for this editor.
            current_code_editor_->SetTitleBarText("");
            source_editor_titlebar_->SetTitlebarContentsVisibility(false);
        }
    }
}

bool RgBuildViewVulkan::CreateMenu(QWidget* parent)
{
    file_menu_ = static_cast<RgMenuVulkan*>(factory_->CreateFileMenu(parent));

    // Connect disable pipeline state menu item signals.
    bool is_connected = connect(file_menu_, &RgMenuVulkan::EnablePipelineMenuItem, this, &RgBuildViewVulkan::EnablePipelineMenuItem);
    assert(is_connected);

    // Connect disable build settings menu item signals.
    is_connected = connect(file_menu_, &RgMenuVulkan::EnableBuildSettingsMenuItem, this, &RgBuildViewVulkan::EnableBuildSettingsMenuItem);
    assert(is_connected);

    connect(&QtCommon::QtUtils::ColorTheme::Get(), &QtCommon::QtUtils::ColorTheme::ColorThemeUpdated, this, &RgBuildViewVulkan::ReapplyMenuStyleSheet);

    return file_menu_ != nullptr;
}

void RgBuildViewVulkan::ReapplyMenuStyleSheet()
{
    factory_->ApplyFileMenuStylesheet(file_menu_);
}

void RgBuildViewVulkan::HandleEnumListWidgetStatus(bool is_open)
{
    assert(pipeline_state_view_ != nullptr);
    if (pipeline_state_view_ != nullptr)
    {
        pipeline_state_view_->SetEnumListWidgetStatus(is_open);
    }
}

bool RgBuildViewVulkan::CreatePipelineStateModel()
{
    bool status = true;

    std::shared_ptr<RgFactoryGraphics> graphics_factory = std::dynamic_pointer_cast<RgFactoryGraphics>(factory_);

    assert(graphics_factory != nullptr);
    if (graphics_factory != nullptr)
    {
        pipeline_state_model_ = static_cast<RgPipelineStateModelVulkan*>(graphics_factory->CreatePipelineStateModel(this));
        assert(pipeline_state_model_ != nullptr);
        if (pipeline_state_model_ != nullptr)
        {
            // Connect the list widget status signal.
            bool is_connected =
                connect(pipeline_state_model_, &RgPipelineStateModelVulkan::EnumListWidgetStatusSignal, this, &RgBuildViewVulkan::HandleEnumListWidgetStatus);
            assert(is_connected);

            // Connect the shortcut hot key signal.
            is_connected = connect(this, &RgBuildViewVulkan::HotKeyPressedSignal, pipeline_state_model_, &RgPipelineStateModelVulkan::HotKeyPressedSignal);
            assert(is_connected);

            assert(project_ != nullptr);
            if (project_ != nullptr)
            {
                // Ensure that the incoming clone index is valid for the current project.
                bool res = (clone_index_ >= 0 && clone_index_ < project_->clones.size());
                assert(res);

                if (res)
                {
                    std::shared_ptr<RgProjectCloneVulkan> vulkan_clone = std::dynamic_pointer_cast<RgProjectCloneVulkan>(project_->clones[clone_index_]);

                    assert(vulkan_clone != nullptr);
                    if (vulkan_clone != nullptr)
                    {
                        bool        is_pipeline_state_initialized = false;
                        std::string error_string;

                        assert(pipeline_state_index_ < vulkan_clone->pso_states.size());
                        if (pipeline_state_index_ < vulkan_clone->pso_states.size())
                        {
                            RgPipelineState& currentPipelineState = vulkan_clone->pso_states[pipeline_state_index_];
                            bool             is_state_file_exists = RgUtils::IsFileExists(currentPipelineState.pipeline_state_file_path);
                            if (is_state_file_exists)
                            {
                                // Load the state file from disk.
                                is_pipeline_state_initialized = pipeline_state_model_->LoadPipelineStateFile(
                                    this, currentPipelineState.pipeline_state_file_path, vulkan_clone->pipeline.type, error_string);

                                assert(is_pipeline_state_initialized);
                                if (!is_pipeline_state_initialized)
                                {
                                    std::stringstream error_stream;
                                    error_stream << kStrErrCannotLoadPipelineStateFile;
                                    error_stream << " ";
                                    error_stream << error_string;
                                    emit SetStatusBarText(error_stream.str().c_str());
                                }
                            }

                            // If the pipeline state file didn't exist, or failed to load, initialize the default configuration.
                            if (!is_pipeline_state_initialized)
                            {
                                // Initialize the pipeline state model based on the pipeline type.
                                pipeline_state_model_->InitializeDefaultPipelineState(this, vulkan_clone->pipeline.type);

                                // Save the new default pipeline state.
                                is_pipeline_state_initialized =
                                    pipeline_state_model_->SavePipelineStateFile(currentPipelineState.pipeline_state_file_path, error_string);
                            }
                        }
                        else
                        {
                            status = false;
                        }

                        // Was the pipeline state file loaded or created successfully?
                        assert(is_pipeline_state_initialized);
                        if (!is_pipeline_state_initialized)
                        {
                            std::stringstream error_stream;
                            error_stream << kStrErrCannotInitializePipelineStateFile;
                            error_stream << error_string;
                            RgUtils::ShowErrorMessageBox(error_stream.str().c_str(), this);
                        }
                    }
                }
            }
        }
    }

    return status;
}

void RgBuildViewVulkan::CreatePipelineStateView(QWidget* parent)
{
    // Create a border frame for the pipeline state editor.
    pso_editor_frame_ = new QFrame(this);
    pso_editor_frame_->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
    pso_editor_frame_->setFrameStyle(QFrame::Box);
    pso_editor_frame_->setLayout(new QVBoxLayout);
    pso_editor_frame_->setObjectName(kStrPsoEditorFrameName);

    // Create a new Pipeline State editor view.
    pipeline_state_view_ = new RgPipelineStateView(parent);

    assert(pipeline_state_view_ != nullptr);
    if (pipeline_state_view_ != nullptr)
    {
        // Add the state editor view to the parent frame.
        pso_editor_frame_->layout()->addWidget(pipeline_state_view_);

        // Create the find widget, register with the scaling manager.
        pso_find_widget_ = new RgFindTextWidget(pipeline_state_view_);

        // The find widget is hidden by default.
        pso_find_widget_->hide();

        // Add the find widget to RgPipelineStateView's grid.
        pipeline_state_view_->InsertFindWidget(pso_find_widget_);

        // Connect the find widget signals.
        ConnectPsoFindSignals();

        assert(project_ != nullptr);
        if (project_ != nullptr)
        {
            // Ensure that the incoming clone index is valid for the current project.
            bool res = (clone_index_ >= 0 && clone_index_ < project_->clones.size());
            assert(res);

            if (res)
            {
                std::shared_ptr<RgProjectCloneVulkan> clone = std::dynamic_pointer_cast<RgProjectCloneVulkan>(project_->clones[clone_index_]);

                assert(clone != nullptr);
                if (clone != nullptr)
                {
                    // Initialize the PSO editor model.
                    pipeline_state_view_->InitializeModel(pipeline_state_model_);

                    // Connect signals for the new pipeline state view.
                    ConnectPipelineStateViewSignals();

                    // The view is hidden initially.
                    pso_editor_frame_->hide();
                }
            }
        }
    }
}

void RgBuildViewVulkan::ConnectDisassemblyViewApiSpecificSignals()
{
    assert(disassembly_view_ != nullptr);
    if (disassembly_view_ != nullptr)
    {
        // Connect the handler invoked when the user changes the selected entry point.
        bool is_connected =
            connect(this, &RgBuildViewVulkan::SelectedEntrypointChanged, disassembly_view_, &RgIsaDisassemblyView::HandleSelectedEntrypointChanged);
        assert(is_connected);

        // Connect the remove button focus events.
        is_connected = connect(disassembly_view_, &RgIsaDisassemblyView::RemoveFileMenuButtonFocus, file_menu_, &RgMenuVulkan::HandleRemoveFileMenuButtonFocus);
        assert(is_connected);
    }
}

bool RgBuildViewVulkan::IsLineCorrelationEnabled(RgSourceCodeEditor* source_editor)
{
    Q_UNUSED(source_editor);

    // Source line correlation with disassembly is not available in Vulkan mode.
    return false;
}

bool RgBuildViewVulkan::IsSourceFileInProject(const std::string& source_file_path) const
{
    bool res = false;

    // Step through all possible pipeline stages to check if the given source file exists.
    uint32_t first_stage = static_cast<uint32_t>(RgPipelineStage::kVertex);
    uint32_t last_stage  = static_cast<uint32_t>(RgPipelineStage::kCompute);

    assert(project_ != nullptr);
    if (project_ != nullptr)
    {
        // Ensure that the incoming clone index is valid for the current project.
        bool is_valid_range = (clone_index_ >= 0 && clone_index_ < project_->clones.size());

        assert(is_valid_range);
        if (is_valid_range)
        {
            std::shared_ptr<RgProjectCloneVulkan> vulkan_clone = std::dynamic_pointer_cast<RgProjectCloneVulkan>(project_->clones[clone_index_]);
            for (uint32_t stage_index = first_stage; stage_index < last_stage; ++stage_index)
            {
                RgPipelineStage current_stage = static_cast<RgPipelineStage>(stage_index);

                std::string stage_file_path;
                if (RgConfigManager::Instance().GetShaderStageFilePath(current_stage, vulkan_clone, stage_file_path))
                {
                    if (!stage_file_path.empty() && stage_file_path.compare(source_file_path) == 0)
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

bool RgBuildViewVulkan::ShowRevertToSpvBinaryConfirmation(const std::string& file_path)
{
    std::string msg = kStrMenuBarVulkanConfirmRevertToOrigSpvA;
    msg += std::string("\n") + file_path + ".\n";
    msg += kStrMenuBarVulkanConfirmRevertToOrigSpvB;

    return RgUtils::ShowConfirmationMessageBox(kStrMenuBarConfirmRemoveFileDialogTitle, msg.c_str(), this);
}

bool RgBuildViewVulkan::ReplaceInputFileInBuildOutput(const std::string& old_file_path, const std::string& new_file_path)
{
    assert(!old_file_path.empty());
    assert(!new_file_path.empty());
    bool ret = (!old_file_path.empty() && !new_file_path.empty());
    if (ret)
    {
        auto first_target_gpu = build_outputs_.begin();
        auto last_target_gpu  = build_outputs_.end();
        for (auto targetGpuIter = first_target_gpu; targetGpuIter != last_target_gpu; ++targetGpuIter)
        {
            ret                                            = false;
            std::shared_ptr<RgCliBuildOutput> build_output = targetGpuIter->second;

            // Search for outputs for the given source file and replace its key (input file path) with the new one.
            assert(build_output != nullptr);
            if (build_output != nullptr)
            {
                auto it = build_output->per_file_output.find(old_file_path);
                if (it != build_output->per_file_output.end())
                {
                    auto outputs = it->second;
                    build_output->per_file_output.erase(it);
                    build_output->per_file_output[new_file_path] = outputs;
                    ret                                          = true;
                }
            }
        }
    }

    return ret;
}

bool RgBuildViewVulkan::CreateNewSourceFile(RgPipelineStage stage, const std::string& source_file_name, std::string& full_source_file_path)
{
    bool ret = false;

    // True if we are creating a new file in an existing project.
    bool is_existing_project = (project_ != nullptr);

    if (is_existing_project)
    {
        std::shared_ptr<RgUtilsVulkan> vulkan_utils = std::dynamic_pointer_cast<RgUtilsVulkan>(RgUtilsGraphics::CreateUtility(RgProjectAPI::kVulkan));
        assert(vulkan_utils != nullptr);
        if (vulkan_utils != nullptr)
        {
            // Use the Vulkan factory to build a file extension based on the stage being added.
            std::stringstream file_extension;
            file_extension << ".";
            file_extension << vulkan_utils->PipelineStageToAbbreviation(stage);

            // Generate a path to where the new empty file will live in the projects directory.
            std::string new_file_name;
            RgConfigManager::GenerateNewSourceFilepath(
                project_->project_name, clone_index_, source_file_name, file_extension.str(), new_file_name, full_source_file_path);

            // Ensure that the folder where the file will be saved already exists.
            std::string sourcefile_folder;
            RgUtils::ExtractFileDirectory(full_source_file_path, sourcefile_folder);
            if (!RgUtils::IsDirExists(sourcefile_folder))
            {
                [[maybe_unused]] bool is_dir_created = RgUtils::CreateFolder(sourcefile_folder);
                assert(is_dir_created);
            }

            // Create a new file at the target location.
            QFile empty_file(full_source_file_path.c_str());
            empty_file.open(QIODevice::ReadWrite);

            // Use the Vulkan factory to fill the new file with the default code for the stage being created.
            QTextStream stream(&empty_file);
            stream << vulkan_utils->GetDefaultShaderCode(stage).c_str() << Qt::endl;
            empty_file.close();

            // Add the source file's path to the project's clone.
            RgConfigManager::Instance().AddShaderStage(stage, full_source_file_path, project_, clone_index_);

            // Save the project after adding a source file.
            RgConfigManager::Instance().SaveProjectFile(project_);

            assert(file_menu_ != nullptr);
            if (file_menu_ != nullptr)
            {
                RgMenuVulkan* vulkan_file_menu = static_cast<RgMenuVulkan*>(file_menu_);
                assert(vulkan_file_menu != nullptr);
                if (vulkan_file_menu != nullptr)
                {
                    // Set the source file in the stage item.
                    vulkan_file_menu->SetStageSourceFile(stage, full_source_file_path, RgVulkanInputType::kGlsl, true);

                    // Use GLSL syntax highlighting.
                    assert(current_code_editor_ != nullptr);
                    if (current_code_editor_ != nullptr)
                    {
                        current_code_editor_->SetSyntaxHighlighting(RgSrcLanguage::kGLSL);
                    }
                }
            }

            // We are done.
            ret = true;
        }
    }

    return ret;
}

bool RgBuildViewVulkan::CreateProject(RgPipelineType pipeline_type)
{
    // True if we are creating a new file in an existing project.
    bool res = (project_ != nullptr);

    if (!res)
    {
        res = CreateNewEmptyProject();

        if (res)
        {
            assert(project_ != nullptr);
            if (project_ != nullptr)
            {
                // Ensure that the incoming clone index is valid for the current project.
                res = (clone_index_ >= 0 && clone_index_ < project_->clones.size());
                assert(res);

                if (res)
                {
                    std::shared_ptr<RgProjectCloneVulkan> clone = std::dynamic_pointer_cast<RgProjectCloneVulkan>(project_->clones[clone_index_]);

                    // Initialize the clone's pipeline type.
                    clone->pipeline.type = pipeline_type;

                    // Create the pipeline CreateInfo object.
                    CreatePipelineStateFile();

                    // The pipeline type was updated after the clone was created, so re-save the
                    // project with the new change.
                    RgConfigManager& config_manager = RgConfigManager::Instance();
                    config_manager.SaveProjectFile(project_);
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

void RgBuildViewVulkan::CreatePipelineStateFile()
{
    assert(project_ != nullptr);
    if (project_ != nullptr)
    {
        // Ensure that the incoming clone index is valid for the current project.
        bool res = (clone_index_ >= 0 && clone_index_ < project_->clones.size());
        assert(res);

        if (res)
        {
            std::shared_ptr<RgProjectCloneVulkan> clone = std::dynamic_pointer_cast<RgProjectCloneVulkan>(project_->clones[clone_index_]);

            assert(clone != nullptr);
            if (clone != nullptr)
            {
                size_t pipeline_count = clone->pso_states.size();

                // Generate a suitable pipeline name based on the number of existing pipelines.
                std::stringstream pipeline_name_stream;
                pipeline_name_stream << kStrDefaultPipelineName;
                pipeline_name_stream << pipeline_count;
                std::string pipeline_state_name = pipeline_name_stream.str();

                bool        is_graphics_pipeline = (clone->pipeline.type == RgPipelineType::kGraphics);
                const char* pso_file_extension   = is_graphics_pipeline ? kStrDefaultPipelineFileExtensionGraphics : kStrDefaultPipelineFileExtensionCompute;

                // Build a file path to a new pipeline state file.
                std::string pipeline_state_file_path;
                RgConfigManager::GenerateNewPipelineFilepath(
                    project_->project_name, clone_index_, pipeline_state_name, pso_file_extension, pipeline_state_file_path);

                // Does the Pipeline State File's target directory already exist? Pipeline State
                // files are stored alongside the clone's source files. If the directory does not
                // yet exist, it needs to be created before attempting to write the file.
                std::string directory_path;
                bool        is_ok = RgUtils::ExtractFileDirectory(pipeline_state_file_path, directory_path);
                if (is_ok)
                {
                    bool directory_exists = RgUtils::IsDirExists(directory_path);
                    if (!directory_exists)
                    {
                        is_ok = RgUtils::CreateFolder(directory_path);
                    }
                }

                if (is_ok)
                {
                    RgPipelineState pipeline_state_file          = {};
                    pipeline_state_file.name                     = pipeline_state_name;
                    pipeline_state_file.pipeline_state_file_path = pipeline_state_file_path;
                    pipeline_state_file.is_active                = true;

                    // Add the new PSO file to the current clone's PSO states vector.
                    clone->pso_states.push_back(pipeline_state_file);
                }
            }
        }
    }
}

void RgBuildViewVulkan::SetStageSourceFile(RgPipelineStage stage, const std::string& file_path_to_add)
{
    assert(file_menu_ != nullptr);
    if (file_menu_ != nullptr)
    {
        std::shared_ptr<RgUtilsVulkan> vulkan_utils = std::dynamic_pointer_cast<RgUtilsVulkan>(RgUtilsGraphics::CreateUtility(RgProjectAPI::kVulkan));
        assert(vulkan_utils != nullptr);
        if (vulkan_utils != nullptr)
        {
            RgMenuVulkan* vulkan_file_menu = static_cast<RgMenuVulkan*>(file_menu_);
            assert(vulkan_file_menu != nullptr);
            if (vulkan_file_menu != nullptr)
            {
                // Detect type of the file.
                auto file_type = RgUtils::DetectInputFileType(file_path_to_add);

                // Set the source file in the stage item.
                bool fileAdded = vulkan_file_menu->SetStageSourceFile(stage, file_path_to_add, file_type.first, false);

                // Enable reverting to original SPIR-V binary if it's present in the Project Clone.
                if (fileAdded && file_type.first == RgVulkanInputType::kSpirvTxt)
                {
                    std::shared_ptr<RgProjectCloneVulkan> vulkan_clone = std::dynamic_pointer_cast<RgProjectCloneVulkan>(project_->clones[clone_index_]);

                    assert(vulkan_clone != nullptr);
                    if (vulkan_clone != nullptr)
                    {
                        if (!vulkan_clone->spv_backup_.shader_stages[stage].empty())
                        {
                            file_menu_->GetStageItem(stage)->AddContextMenuActionRestoreSpv();
                        }
                    }
                }

                if (fileAdded && current_code_editor_ != nullptr)
                {
                    // Enable corresponding syntax highlighting.
                    current_code_editor_->SetSyntaxHighlighting(file_type.second);

                    // If the file being added is a SPIR-V binary, disassemble it and show the disassembly text in the source editor.
                    if (file_type.first == RgVulkanInputType::kSpirv)
                    {
                        std::string spv_disasm_file;
                        if (DisasmSpvFile(file_path_to_add, spv_disasm_file))
                        {
                            SetSourceCodeText(spv_disasm_file);

                            // Store the path to the disassembled file so that we know where to save the modified disassembly text later.
                            spv_disasm_files_[stage] = spv_disasm_file;

                            // Store the spv last modification time.
                            QFileInfo file_info(file_path_to_add.c_str());
                            file_modified_time_map_[current_code_editor_] = file_info.lastModified();
                        }
                    }
                    else
                    {
                        SetSourceCodeText(file_path_to_add);
                    }
                }
            }
        }
    }
}

bool RgBuildViewVulkan::DisasmSpvFile(const std::string& spv_file, std::string& spv_disasm_file)
{
    std::string proj_dir;
    bool        is_ok = RgUtils::ExtractFileDirectory(project_->project_file_full_path, proj_dir);
    assert(is_ok);
    if (is_ok && RgUtils::ConstructSpvDisasmFileName(proj_dir, spv_file, spv_disasm_file))
    {
        // Create a sub-folder in the Project folder for the spv disasm file if it does not exist.
        std::string subfolder, compiler_path;
        is_ok = RgUtils::ExtractFileDirectory(spv_disasm_file, subfolder);
        assert(is_ok);
        if (is_ok)
        {
            // Get the "alternative compiler path" setting value.
            std::shared_ptr<RgProjectCloneVulkan> vulkan_clone = std::dynamic_pointer_cast<RgProjectCloneVulkan>(project_->clones[clone_index_]);

            assert(vulkan_clone != nullptr && vulkan_clone->build_settings != nullptr);
            if (vulkan_clone != nullptr && vulkan_clone->build_settings != nullptr)
            {
                compiler_path = std::get<CompilerFolderType::kBin>(vulkan_clone->build_settings->compiler_paths);
            }
            RgUtils::CreateFolder(subfolder);
            std::string cli_output;
            is_ok = RgCliLauncher::DisassembleSpvToText(compiler_path, spv_file, spv_disasm_file, cli_output);
            HandleNewCLIOutputString(cli_output);
        }
    }

    return is_ok;
}

bool RgBuildViewVulkan::IsLineCorrelationSupported() const
{
    return false;
}

RgMenuGraphics* RgBuildViewVulkan::GetGraphicsFileMenu()
{
    return file_menu_;
}

void RgBuildViewVulkan::HandlePipelineStateTreeFocusIn()
{
    // Change the container frame's color.
    assert(pso_editor_frame_ != nullptr);
    if (pso_editor_frame_ != nullptr)
    {
        // Set the default widget focus.
        pipeline_state_view_->SetInitialWidgetFocus();

        // Change the color of the PSO editor frame border.
        QString style_sheet_string = QString("#") + kStrPsoEditorFrameName + QString(" { border: 1px solid red; background-color: transparent}");
        pso_editor_frame_->setStyleSheet(style_sheet_string);
    }
}

void RgBuildViewVulkan::HandlePipelineStateTreeFocusOut()
{
    // Change the container frame's color.
    assert(pso_editor_frame_ != nullptr);
    if (pso_editor_frame_ != nullptr)
    {
        QString style_sheet_string = QString("#") + kStrPsoEditorFrameName + QString(" { border: 1px solid palette(text) }");
        pso_editor_frame_->setStyleSheet(style_sheet_string);
    }
}

void RgBuildViewVulkan::UpdateApplicationNotificationMessage()
{
    std::string message = "";
    std::string tooltip = "";
    size_t      pos     = cli_output_window_->GetText().find("vk-spv-offline");
    if (pos != std::string::npos)
    {
        message = kStrApplicationInformationMessage;
        tooltip = kStrApplicationInformationTooltip;

        // Emit the signal to update the application notification message.
        emit UpdateApplicationNotificationMessageSignal(message, tooltip);
    }
}

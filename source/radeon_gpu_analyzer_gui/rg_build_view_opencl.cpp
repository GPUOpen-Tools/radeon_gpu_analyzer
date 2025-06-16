//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for Build View class for OpenCL.
//=============================================================================

// C++.
#include <cassert>
#include <sstream>
#include <thread>

// Qt.
#include <QWidget>
#include <QTextStream>
#include <QFile>

// QtCommon.
#include "qt_common/utils/qt_util.h"

// Infra.
#include "source/common/rga_shared_utils.h"

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_build_settings_view_opencl.h"
#include "radeon_gpu_analyzer_gui/qt/rg_build_settings_widget.h"
#include "radeon_gpu_analyzer_gui/qt/rg_build_view_opencl.h"
#include "radeon_gpu_analyzer_gui/qt/rg_cli_output_view.h"
#include "radeon_gpu_analyzer_gui/qt/rg_isa_disassembly_view.h"
#include "radeon_gpu_analyzer_gui/qt/rg_menu_build_settings_item.h"
#include "radeon_gpu_analyzer_gui/qt/rg_menu_file_item_opencl.h"
#include "radeon_gpu_analyzer_gui/qt/rg_menu_opencl.h"
#include "radeon_gpu_analyzer_gui/qt/rg_menu_titlebar.h"
#include "radeon_gpu_analyzer_gui/qt/rg_rename_project_dialog.h"
#include "radeon_gpu_analyzer_gui/qt/rg_source_code_editor.h"
#include "radeon_gpu_analyzer_gui/rg_cli_launcher.h"
#include "radeon_gpu_analyzer_gui/rg_definitions.h"
#include "radeon_gpu_analyzer_gui/rg_factory_opencl.h"
#include "radeon_gpu_analyzer_gui/rg_string_constants.h"
#include "radeon_gpu_analyzer_gui/rg_utils.h"
#include "radeon_gpu_analyzer_gui/rg_xml_session_config.h"

RgBuildViewOpencl::RgBuildViewOpencl(QWidget* parent)
    : RgBuildView(RgProjectAPI::kOpenCL, parent)
{
}

void RgBuildViewOpencl::ConnectBuildSettingsSignals()
{
    // Connect the parent's signals.
    RgBuildView::ConnectBuildSettingsSignals();

    RgBuildSettingsViewOpencl* build_settings_view_opencl = static_cast<RgBuildSettingsViewOpencl*>(build_settings_view_);
    assert(build_settings_view_opencl != nullptr);

    if (build_settings_view_opencl != nullptr)
    {
        bool is_connected = connect(build_settings_view_opencl, &RgBuildSettingsViewOpencl::PendingChangesStateChanged, this, &RgBuildView::HandleBuildSettingsPendingChangesStateChanged);
        assert(is_connected);

        is_connected = connect(build_settings_view_opencl, &RgBuildSettingsViewOpencl::ProjectBuildSettingsSaved, this, &RgBuildView::HandleBuildSettingsSaved);
        assert(is_connected);

        // Connect to build settings view's edit line's "focus in" event to color the frame green.
        is_connected = connect(build_settings_view_opencl, &RgBuildSettingsViewOpencl::SetFrameBorderGreenSignal, this, &RgBuildView::HandleSetFrameBorderGreen);
        assert(is_connected);

        // Connect to build settings view's edit line's "focus out" event to color the frame black.
        is_connected = connect(build_settings_view_opencl, &RgBuildSettingsViewOpencl::SetFrameBorderBlackSignal, this, &RgBuildView::HandleSetFrameBorderBlack);
        assert(is_connected);
    }
}

bool RgBuildViewOpencl::ConnectMenuSignals()
{
    bool is_connected = false;

    // Connect the file menu's file item entry point changed signal.
    assert(file_menu_ != nullptr);
    if (file_menu_ != nullptr)
    {
        RgMenuOpencl* menu_opencl = static_cast<RgMenuOpencl*>(file_menu_);
        assert(menu_opencl != nullptr);
        if (menu_opencl != nullptr)
        {
            // Connect the OpenCL menu's "Selected entry point changed" handler.
            is_connected = connect(menu_opencl, &RgMenuOpencl::SelectedEntrypointChanged, this, &RgBuildViewOpencl::HandleSelectedEntrypointChanged);
            assert(is_connected);

            // Connect the file menu item's drag and drop handler.
            is_connected = connect(menu_opencl, &RgMenuOpencl::DragAndDropExistingFile, this, &RgBuildViewOpencl::HandleExistingFileDragAndDrop);
            assert(is_connected);

            // Connect the RgBuildView's entry point changed signal to the file menu's handler.
            is_connected =  connect(this, &RgBuildViewOpencl::SelectedEntrypointChanged, menu_opencl, &RgMenuOpencl::HandleSelectedEntrypointChanged);
            assert(is_connected);

            // Connect the file menu item selection handler for each new item.
            is_connected = connect(menu_opencl, &RgMenuOpencl::MenuItemClicked, this, &RgBuildViewOpencl::HandleMenuItemClicked);
            assert(is_connected);

            // Notify the file menu that a new source file has been added.
            is_connected = connect(this, &RgBuildViewOpencl::AddedSourceFileToProject, file_menu_, &RgMenuOpencl::HandleSourceFileAdded);
            assert(is_connected);
        }
    }

    return is_connected;
}

void RgBuildViewOpencl::CurrentBuildCancelled()
{
    assert(file_menu_ != nullptr);
    if (file_menu_ != nullptr)
    {
        RgMenuOpencl* menu_opencl = static_cast<RgMenuOpencl*>(file_menu_);
        assert(menu_opencl != nullptr);

        if (menu_opencl != nullptr)
        {
            // Don't allow the user to expand file item's entry point list.
            menu_opencl->SetIsShowEntrypointListEnabled(false);
        }
    }

    // Remove all the generated files in output directory.
    DestroyProjectBuildArtifacts();
}

void RgBuildViewOpencl::CurrentBuildSucceeded()
{
    // Invoke the CLI to load the start line numbers for each entrypoint.
    LoadEntrypointLineNumbers();

    assert(file_menu_ != nullptr);
    if (file_menu_ != nullptr)
    {
        RgMenuOpencl* menu_opencl = static_cast<RgMenuOpencl*>(file_menu_);
        assert(menu_opencl != nullptr);
        if (menu_opencl != nullptr)
        {
            // Allow the user to expand the file's entry point list.
            menu_opencl->SetIsShowEntrypointListEnabled(true);
        }
    }

    // Update the file menu item with the clone's build output.
    RgMenuOpencl* menu_opencl = static_cast<RgMenuOpencl*>(file_menu_);
    menu_opencl->UpdateBuildOutput(build_outputs_);

    // Update the correlation state for each source code editor based on which source
    // files were built successfully.
    std::string output_gpu;
    std::shared_ptr<RgCliBuildOutput> build_output = nullptr;
    bool isOutputValid = RgUtils::GetFirstValidOutputGpu(build_outputs_, output_gpu, build_output);
    if (isOutputValid && build_output != nullptr)
    {
        // Store the path to the current source file using the file menu.
        std::string current_source_file_path;
        RgMenu* menu = GetMenu();
        assert(menu != nullptr);
        if (menu != nullptr)
        {
            current_source_file_path = menu->GetSelectedFilePath();
        }

        // Enable line correlation for all source files that were built successfully.
        std::shared_ptr<RgCliBuildOutputOpencl> build_output_opencl =
            std::dynamic_pointer_cast<RgCliBuildOutputOpencl>(build_output);

        assert(build_output_opencl != nullptr);
        if (build_output_opencl != nullptr)
        {
            auto source_path_iter_start = build_output_opencl->per_file_output.begin();
            auto source_path_iter_end = build_output_opencl->per_file_output.end();
            for (auto input_file_path_iter = source_path_iter_start; input_file_path_iter != source_path_iter_end; ++input_file_path_iter)
            {
                // Get a pointer to the source editor for each input file.
                const std::string& source_file_path = input_file_path_iter->first;

                // Skip updating the current file within the loop. It  will be updated last.
                if (current_source_file_path.compare(source_file_path) != 0)
                {
                    // Only update the correlation if the source file still exists in the project.
                    // Previously-built files that have already been removed from the project may
                    // have artifacts loaded.
                    auto editor_iter = source_code_editors_.find(source_file_path);
                    if (editor_iter != source_code_editors_.end())
                    {
                        RgSourceCodeEditor* editor = GetEditorForFilepath(source_file_path);

                        // Emit the signal used to update the correlation enabledness.
                        emit LineCorrelationEnabledStateChanged(editor, true);
                    }
                }
            }

            // Update the currently selected file last.
            RgSourceCodeEditor* editor = GetEditorForFilepath(current_source_file_path);
            if (editor != nullptr)
            {
                // Emit the signal used to update the correlation enabledness.
                emit LineCorrelationEnabledStateChanged(editor, true);
            }
        }
    }
}

bool RgBuildViewOpencl::CreateMenu(QWidget* parent)
{
    file_menu_ = static_cast<RgMenuOpencl*>(factory_->CreateFileMenu(parent));

    // Notify the file menu when the build succeeded.
    bool is_connected = connect(this, &RgBuildViewOpencl::ProjectBuildSuccess, file_menu_, &RgMenuOpencl::ProjectBuildSuccess);
    assert(is_connected);

    // Notify the file menu to update file menu item coloring
    // when an already built project is being loaded.
    is_connected = connect(this, &RgBuildViewOpencl::UpdateFileColoring, file_menu_, &RgMenuOpencl::ProjectBuildSuccess);
    assert(is_connected);

    connect(&QtCommon::QtUtils::ColorTheme::Get(), &QtCommon::QtUtils::ColorTheme::ColorThemeUpdated, this, &RgBuildViewOpencl::ReapplyMenuStyleSheet);

    return file_menu_ != nullptr;
}

void RgBuildViewOpencl::ReapplyMenuStyleSheet()
{
    factory_->ApplyFileMenuStylesheet(file_menu_);
}

void RgBuildViewOpencl::ConnectDisassemblyViewApiSpecificSignals()
{
    assert(disassembly_view_ != nullptr);
    if (disassembly_view_ != nullptr)
    {
        // Connect the handler invoked when the user changes the selected entrypoint.
        bool is_connected = connect(this, &RgBuildViewOpencl::SelectedEntrypointChanged,
            disassembly_view_, &RgIsaDisassemblyView::HandleSelectedEntrypointChanged);
        assert(is_connected);

        // Connect the RgIsaDisassemblyView's entry point changed handler.
        is_connected = connect(disassembly_view_, &RgIsaDisassemblyView::SelectedEntrypointChanged,
            this, &RgBuildViewOpencl::HandleSelectedEntrypointChanged);
        assert(is_connected);
    }
}

void RgBuildViewOpencl::DestroyProjectBuildArtifacts()
{
    // Invoke the base implementation used to destroy project build artifacts.
    RgBuildView::DestroyProjectBuildArtifacts();

    // Clear any old build artifacts from the OpenCL-specific file menu items.
    ClearFileItemsEntrypointList();
}

bool RgBuildViewOpencl::IsSourceFileInProject(const std::string& source_file_path) const
{
    bool res = false;

    std::vector<std::string> source_files;
    RgConfigManager::Instance().GetProjectSourceFilePaths(project_, clone_index_, source_files);

    for (std::vector<std::string>::iterator iter = source_files.begin(); iter != source_files.end(); ++iter)
    {
        if (RgaSharedUtils::ComparePaths(source_file_path, *iter))
        {
            res = true;
            break;
        }
    }

    return res;
}

RgMenu* RgBuildViewOpencl::GetMenu() const
{
    return file_menu_;
}

void RgBuildViewOpencl::FocusOnFileMenu()
{
    // Switch the focus to the file menu.
    if (file_menu_ != nullptr)
    {
        file_menu_->setFocus();
    }
}

bool RgBuildViewOpencl::PopulateMenu()
{
    bool ret = false;

    // Fill up the file path list with the paths corrected by the user.
    std::vector<std::string> source_file_paths;
    RgConfigManager::Instance().GetProjectSourceFilePaths(project_, clone_index_, source_file_paths);
    if (!source_file_paths.empty())
    {
        // Add all the project's source files into the RgBuildView.
        for (int file_index = 0; file_index < source_file_paths.size(); ++file_index)
        {
            const std::string& file_path = source_file_paths[file_index];

            // Check that the file still exists before attempting to load it.
            bool is_file_exists = RgUtils::IsFileExists(file_path);
            assert(is_file_exists);
            if (is_file_exists)
            {
                // Add the selected file to the menu.
                if (AddFile(file_path))
                {
                    // Set the source code view text with the contents of the selected file.
                    SetSourceCodeText(file_path);

                    // The RgBuildView was successfully populated with the current project.
                    ret = true;
                }
            }
            else
            {
                // Build an error string saying the file couldn't be found on disk.
                std::stringstream error_string;
                error_string << kStrErrCannotLoadSourceFileMsg;
                error_string << file_path;

                // Show the user the error message.
                RgUtils::ShowErrorMessageBox(error_string.str().c_str(), this);
            }
        }

        // Select the first available file.
        if (ret)
        {
            file_menu_->SelectFirstItem();
        }
    }
    else
    {
        // It's OK if the project being loaded doesn't include any source files, so return true.
        ret = true;
    }

    return ret;
}

bool RgBuildViewOpencl::IsGcnDisassemblyGenerated(const std::string& input_file_path) const
{
    bool is_current_file_disassembled = false;

    auto targetGpuOutputsIter = build_outputs_.find(current_target_gpu_);
    if (targetGpuOutputsIter != build_outputs_.end())
    {
        std::shared_ptr<RgCliBuildOutputOpencl> build_output =
            std::dynamic_pointer_cast<RgCliBuildOutputOpencl>(targetGpuOutputsIter->second);

        assert(build_output != nullptr);
        if (build_output != nullptr)
        {
            auto inputFileOutputsIter = build_output->per_file_output.find(input_file_path);
            if (inputFileOutputsIter != build_output->per_file_output.end())
            {
                RgFileOutputs& fileOutputs = inputFileOutputsIter->second;
                is_current_file_disassembled = !fileOutputs.outputs.empty();
            }
        }
    }

    return is_current_file_disassembled;
}

bool RgBuildViewOpencl::LoadSessionMetadata(const std::string& metadata_file_path, std::shared_ptr<RgCliBuildOutput>& build_output)
{
    bool ret = false;

    std::shared_ptr<RgCliBuildOutputOpencl> gpu_output_opencl = nullptr;
    ret = RgXMLSessionConfig::ReadSessionMetadataOpenCL(metadata_file_path, gpu_output_opencl);
    build_output = gpu_output_opencl;

    return ret;
}

void RgBuildViewOpencl::ShowCurrentFileDisassembly()
{
    bool is_current_file_disassembled = false;

    // Show the currently selected file's first entry point disassembly (if there is no currently selected entry).
    const std::string& input_filepath = file_menu_->GetSelectedFilePath();
    RgMenuFileItem* selected_file_item = file_menu_->GetSelectedFileItem();
    assert(selected_file_item != nullptr);

    if (selected_file_item != nullptr)
    {
        RgMenuFileItemOpencl* opencl_file_item = static_cast<RgMenuFileItemOpencl*>(selected_file_item);
        assert(opencl_file_item != nullptr);

        if (opencl_file_item != nullptr)
        {
            std::string current_entrypoint_name;
            bool is_entry_selected = opencl_file_item->GetSelectedEntrypointName(current_entrypoint_name);

            // Get the list of entry point names for the selected input file.
            std::vector<std::string> entrypoint_names;
            opencl_file_item->GetEntrypointNames(entrypoint_names);

            // Select the first available entry point if any exist.
            if (!entrypoint_names.empty())
            {
                // Show the first entry point in the disassembly table.
                std::string& entrypoint_name = (is_entry_selected ? current_entrypoint_name : entrypoint_names[0]);
                disassembly_view_->HandleSelectedEntrypointChanged(current_target_gpu_, input_filepath, entrypoint_name);

                // Emit a signal indicating that the selected entry point has changed.
                emit SelectedEntrypointChanged(current_target_gpu_, input_filepath, entrypoint_name);

                is_current_file_disassembled = true;
            }
        }
    }

    // Toggle the view based on if the current file has been disassembled or not.
    ToggleDisassemblyViewVisibility(is_current_file_disassembled);
}

bool RgBuildViewOpencl::AddFile(const std::string& file_full_path, bool is_new_file)
{
    bool was_added = false;
    if (file_menu_)
    {
        RgMenuOpencl* file_menu = nullptr;
        file_menu = static_cast<RgMenuOpencl*>(file_menu_);
        if (file_menu != nullptr)
        {
            was_added = file_menu->AddItem(file_full_path, is_new_file);
        }
    }

    return was_added;
}

bool RgBuildViewOpencl::CreateNewSourceFileInProject()
{
    // Generate a new empty source file in the correct location.
    std::string source_filename = kStrDefaultSourceFilename;

    std::string full_sourcefile_path;
    bool ret = CreateNewSourceFile(source_filename, full_sourcefile_path);

    if (ret)
    {
        // This should be a full filename, but new files don't have that.
        if (AddFile(full_sourcefile_path, true))
        {
            // Display the file's source code.
            SetSourceCodeText(full_sourcefile_path);
        }
    }

    return ret;
}

void RgBuildViewOpencl::SetDefaultFocusWidget() const
{
    assert(build_settings_view_ != nullptr);
    if (build_settings_view_ != nullptr)
    {
        build_settings_view_->SetInitialWidgetFocus();
    }
}

bool RgBuildViewOpencl::CreateNewSourceFile(const std::string& source_file_name, std::string& full_source_file_path)
{
    bool ret = false;

    // True if a new project was created.
    bool was_project_created = false;

    // True if we are creating a new file in an existing project.
    bool is_existing_project = (project_ != nullptr);

    if (!is_existing_project)
    {
        was_project_created = CreateNewEmptyProject();

        if (was_project_created)
        {
            emit ProjectCreated();
        }
    }

    if (is_existing_project || was_project_created)
    {
        std::string new_file_name;
        std::string new_file_extension;

        // Generate a path to where the new empty file will live in the projects directory.
        bool got_extension = RgUtils::ProjectAPIToSourceFileExtension(project_->api, new_file_extension);
        assert(got_extension);
        if (got_extension)
        {
            RgConfigManager::GenerateNewSourceFilepath(project_->project_name, clone_index_, source_file_name, new_file_extension, new_file_name, full_source_file_path);

            // Ensure that the folder where the file will be saved already exists.
            std::string source_file_folder;
            RgUtils::ExtractFileDirectory(full_source_file_path, source_file_folder);
            if (!RgUtils::IsDirExists(source_file_folder))
            {
                [[maybe_unused]] bool is_dir_created = RgUtils::CreateFolder(source_file_folder);
                assert(is_dir_created);
            }

            // Create a new file at the target location.
            QFile empty_file(full_source_file_path.c_str());
            empty_file.open(QIODevice::ReadWrite);

            // Add the template source code to the newly created file.
            QTextStream stream(&empty_file);
            stream << RgUtils::GenerateTemplateCode(project_->api, new_file_name).c_str() << Qt::endl;
            empty_file.close();

            // Add the source file's path to the project's clone.
            RgConfigManager::Instance().AddSourceFileToProject(full_source_file_path, project_, clone_index_);

            // Save the project after adding a source file.
            RgConfigManager::Instance().SaveProjectFile(project_);

            // We are done.
            ret = true;
        }
    }

    return ret;
}

void RgBuildViewOpencl::HandleExistingFileDragAndDrop(const std::string& file_path_to_add)
{
    assert(file_menu_ != nullptr);
    if (file_menu_ != nullptr && !file_path_to_add.empty())
    {
        AddExistingSourcefileToProject(file_path_to_add);
    }
}

bool RgBuildViewOpencl::AddExistingSourcefileToProject(const std::string& source_file_path)
{
    bool ret = false;

    bool is_project_created = (project_ != nullptr);
    if (!is_project_created)
    {
        is_project_created = CreateNewEmptyProject();

        if (is_project_created)
        {
            emit ProjectCreated();
        }
    }

    if (is_project_created)
    {
        if (!IsSourceFileInProject(source_file_path))
        {
            RgConfigManager& config_manager = RgConfigManager::Instance();

            // Add the source file's path to the project's clone.
            config_manager.AddSourceFileToProject(source_file_path, project_, clone_index_);

            // Save the project after adding a sourcefile.
            config_manager.SaveProjectFile(project_);

            // Add the selected file to the menu.
            AddFile(source_file_path);

            // Set the source code view text with the contents of the selected file.
            SetSourceCodeText(source_file_path);

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
            msg << kStrErrCannotAddFileA << source_file_path << kStrErrCannotAddFileB;
            RgUtils::ShowErrorMessageBox(msg.str().c_str(), this);
        }
    }

    return ret;
}

bool RgBuildViewOpencl::GetEntrypointNameForLineNumber(const std::string& file_path, int line_number, std::string& entry_name) const
{
    bool found = false;

    // Check if "line_number" is within some kernel code.
    const auto& file_src_line_data = entrypoint_line_numbers_.find(file_path);
    if (file_src_line_data != entrypoint_line_numbers_.end())
    {
        for (const auto& entry_src_line_data : file_src_line_data->second)
        {
            const std::pair<int, int>  startAndEndLines = entry_src_line_data.second;
            if (line_number >= startAndEndLines.first && line_number <= startAndEndLines.second)
            {
                entry_name = entry_src_line_data.first;
                found = true;
                break;
            }
        }
    }

    // Fall back to selecting the current entry point in the selected file item.
    if (!found)
    {
        assert(file_menu_ != nullptr);
        if (file_menu_ != nullptr)
        {
            
            RgMenuFileItem* file_item = file_menu_->GetFileItemFromPath(file_path);
            assert(file_item != nullptr);
            if (file_item != nullptr)
            {
                RgMenuFileItemOpencl* file_item_opencl = static_cast<RgMenuFileItemOpencl*>(file_item);
                assert(file_item_opencl != nullptr);
                if (file_item_opencl != nullptr)
                {
                    std::string entrypoint_name;
                    if (file_item_opencl->GetSelectedEntrypointName(entrypoint_name))
                    {
                        entry_name = entrypoint_name;
                        found      = true;
                    }
                }
            }
        }
    }

    return found;
}

void RgBuildViewOpencl::HandleSelectedFileChanged(const std::string& old_file_path, const std::string& new_file_path)
{
    Q_UNUSED(old_file_path);

    // Get a pointer to the editor responsible for displaying the new file.
    RgSourceCodeEditor* editor = GetEditorForFilepath(new_file_path, RgSrcLanguage::kOpenCL);
    assert(editor != nullptr);
    if (editor != nullptr)
    {
        bool is_editor_switched = SwitchToEditor(editor);
        if (is_editor_switched)
        {
            // Switch the disassembly view to show the currently-selected entry point in the newly-selected file item.
            if (disassembly_view_ != nullptr && !is_build_in_progress_)
            {
                // Open the disassembly view for the source file only if it's disassembled.
                if (IsGcnDisassemblyGenerated(new_file_path))
                {
                    RgMenuFileItem* file_item = file_menu_->GetFileItemFromPath(new_file_path);
                    assert(file_item != nullptr);
                    if (file_item != nullptr)
                    {
                        RgMenuFileItemOpencl* file_item_opencl = static_cast<RgMenuFileItemOpencl*>(file_item);
                        assert(file_item_opencl != nullptr);
                        if (file_item_opencl != nullptr)
                        {
                            // Retrieve the name of the currently-selected entry point (if there is one).
                            std::string selected_entrypoint_name;
                            bool is_entry_point_selected = file_item_opencl->GetSelectedEntrypointName(selected_entrypoint_name);

                            // Update the visibility of the disassembly view.
                            ToggleDisassemblyViewVisibility(is_entry_point_selected);

                            if (is_entry_point_selected)
                            {
                                // Make the disassembly view visible because the file has build outputs to display.
                                emit SelectedEntrypointChanged(current_target_gpu_, new_file_path, selected_entrypoint_name);

                                // Update correlation in the disassembly view.
                                RgSourceCodeEditor* filepath_editor = GetEditorForFilepath(new_file_path);
                                assert(filepath_editor != nullptr);
                                if (filepath_editor != nullptr)
                                {
                                    const int selected_line_number = filepath_editor->GetSelectedLineNumber();
                                    disassembly_view_->HandleInputFileSelectedLineChanged(current_target_gpu_, new_file_path, selected_entrypoint_name, selected_line_number);
                                }
                            }

                            // Update the titlebar for the current source editor.
                            UpdateSourceEditorTitlebar(editor);
                        }
                    }
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

void RgBuildViewOpencl::HandleSourceFileSelectedLineChanged(RgSourceCodeEditor* editor, int line_number)
{
    // Handle updating source correlation only when the project isn't currently being built.
    if (!is_build_in_progress_)
    {
        if (disassembly_view_ != nullptr && !disassembly_view_->IsEmpty())
        {
            const std::string& input_filename = GetFilepathForEditor(editor);
            bool is_disassembled = IsGcnDisassemblyGenerated(input_filename);
            if (is_disassembled)
            {
                int correlated_line_number = kInvalidCorrelationLineIndex;

                bool is_correlation_enabled = IsLineCorrelationEnabled(editor);
                if (is_correlation_enabled)
                {
                    correlated_line_number = line_number;
                }

                // If the line is associated with a named entrypoint, highlight it in the file menu item.
                std::string entry_name;
                bool is_valid = GetEntrypointNameForLineNumber(input_filename, line_number, entry_name);
                if (is_valid)
                {
                    RgMenuOpencl* menu_opencl = static_cast<RgMenuOpencl*>(file_menu_);
                    assert(menu_opencl != nullptr);
                    if (menu_opencl != nullptr)
                    {
                        menu_opencl->HandleSelectedEntrypointChanged(current_target_gpu_, input_filename, entry_name);
                    }
                }

                // Send the input source file's correlation line index to the disassembly view.
                disassembly_view_->HandleInputFileSelectedLineChanged(current_target_gpu_, input_filename, entry_name, correlated_line_number);
            }
        }
    }
}

void RgBuildViewOpencl::HandleSelectedEntrypointChanged(const std::string& input_file_path, const std::string& selected_entrypoint_name)
{
    assert(file_menu_ != nullptr);
    if (file_menu_ != nullptr)
    {
        RgMenuOpencl* file_menu_opencl = static_cast<RgMenuOpencl*>(file_menu_);
        if (file_menu_opencl != nullptr)
        {
            // Trigger the file menu to be updated, which will change the current selection in the current item's entry point list.
            file_menu_opencl->HandleSelectedEntrypointChanged(current_target_gpu_, input_file_path, selected_entrypoint_name);
        }
    }

    // Highlight the start line for the given entry point in the source editor.
    HighlightEntrypointStartLine(input_file_path, selected_entrypoint_name);
}

void RgBuildViewOpencl::HandleMenuItemClicked(RgMenuFileItem* item)
{
    if (ShowSaveDialog())
    {
        file_menu_->HandleSelectedFileChanged(item);
    }
}

void RgBuildViewOpencl::ClearFileItemsEntrypointList()
{
    RgMenuOpencl* menu_opencl = static_cast<RgMenuOpencl*>(file_menu_);
    assert(menu_opencl != nullptr);
    if (menu_opencl != nullptr)
    {
        // Clear references to build outputs from the file menu,
        menu_opencl->ClearBuildOutputs();
    }
}

int RgBuildViewOpencl::FindEntrypointStartLine(RgSourceCodeEditor* editor, int list_kernels_start_line) const
{
    int actual_start_line = list_kernels_start_line;

    // Verify that the current source editor is valid.
    assert(editor != nullptr);
    if (editor != nullptr)
    {
        // Start at the given line and search for the next opening brace. This is the "real" start of the entrypoint.
        int search_line = actual_start_line;
        bool searching_for_start_line = true;
        while (searching_for_start_line)
        {
            QString line_text;
            bool got_line_text = editor->GetTextAtLine(search_line, line_text);
            if (got_line_text)
            {
                // Start at the index of the brace, and check if there's anything else in the line other than whitespace.
                int brace_index = line_text.indexOf("{");
                if (brace_index != -1)
                {
                    std::string line_text_string = line_text.toStdString();

                    // Search for alphanumeric characters the occur after the opening brace.
                    auto start_character = line_text_string.begin() + (brace_index + 1);
                    auto end_character = line_text_string.end();

                    // Create a list of bools, used to determine if there are any alphanumeric characters in the line after the opening brace.
                    std::vector<bool> is_alphanumeric_list;
                    is_alphanumeric_list.resize(line_text_string.size());

                    // Step through each character in the line. If it's alphanumeric, add a "true" to the output list in the character's position.
                    std::transform(start_character, end_character, is_alphanumeric_list.begin(), [](char c) { return (isalnum(c) != 0); });

                    // If there are any 'true' values in the isAlphanumericList, it means that an alphanumeric character appeared after the opening brace.
                    auto alphanumeric_characters_iter = std::find(is_alphanumeric_list.begin(), is_alphanumeric_list.end(), true);
                    if (alphanumeric_characters_iter == is_alphanumeric_list.end())
                    {
                        // There was only whitespace after the opening brace. Advance one more line to where the entry point actually starts.
                        search_line++;
                    }

                    actual_start_line = search_line;
                    searching_for_start_line = false;
                }
                else
                {
                    // Step down to the next line to check if there's an opening brace for the entrypoint.
                    search_line++;
                }
            }
            else
            {
                searching_for_start_line = false;
            }
        }
    }

    return actual_start_line;
}

void RgBuildViewOpencl::HighlightEntrypointStartLine(const std::string& input_file_path, const std::string& selected_entrypoint_name)
{
    // Find the input file in the map of entry point start line numbers.
    auto input_file_iter = entrypoint_line_numbers_.find(input_file_path);
    if (input_file_iter != entrypoint_line_numbers_.end())
    {
        // Search for the start line number for the given entry point name.
        EntryToSourceLineRange& file_entrypoints_info = input_file_iter->second;
        auto lineNumberIter = file_entrypoints_info.find(selected_entrypoint_name);
        if (lineNumberIter != file_entrypoints_info.end())
        {
            RgSourceCodeEditor* editor = GetEditorForFilepath(input_file_path);
            assert(editor != nullptr);
            if (editor != nullptr)
            {
                // Retrieve the entrypoint's start line index according to the "list-kernels" results.
                int list_kernels_entrypoint_start_line = lineNumberIter->second.first;

                // If necessary, advance to the line with the entrypoint's opening brace. This is the real start of the entrypoint.
                int actual_start_line = FindEntrypointStartLine(editor, list_kernels_entrypoint_start_line);

                // Scroll to the start of the entrypoint.
                editor->ScrollToLine(actual_start_line);

                // Move the cursor to the line where the entry point starts.
                QTextCursor cursor(editor->document()->findBlockByLineNumber(actual_start_line - 1));
                editor->setTextCursor(cursor);

                // Highlight the start line for the entrypoint.
                QList<int> line_indices;
                line_indices.push_back(actual_start_line);
                editor->SetHighlightedLines(line_indices);
            }
        }
    }
}

bool RgBuildViewOpencl::LoadEntrypointLineNumbers()
{
    bool ret = false;

    // Destroy the existing entry point line numbers map.
    entrypoint_line_numbers_.clear();

    // Invoke the CLI To query entry point names and start line numbers.
    ret = RgCliLauncher::ListKernels(project_, clone_index_, entrypoint_line_numbers_);
    if (!ret)
    {
        // Let the user know that the query failed.
        emit SetStatusBarText(kStrErrFailedToGetEntrypointLineNumbers, kStatusBarNotificationTimeoutMs);
    }

    return ret;
}

void RgBuildViewOpencl::SetAPISpecificBorderColor()
{
    HandleSetFrameBorderGreen();
}

bool RgBuildViewOpencl::IsLineCorrelationSupported() const
{
    return true;
}

void RgBuildViewOpencl::UpdateApplicationNotificationMessage()
{
    // Add OpenCL application notification message here, when needed.
}

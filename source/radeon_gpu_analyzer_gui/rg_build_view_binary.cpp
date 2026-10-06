//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for Build View class for Binary Analysis mode.
//=============================================================================

// C++.
#include <cassert>
#include <filesystem>
#include <set>
#include <sstream>
#include <thread>
#include <unordered_set>

// Qt.
#include <QWidget>
#include <QTextStream>
#include <QScrollBar>
#include <QFileInfo>
#include <QMessageBox>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QTimer>
#include <QStyle>
#include <QScreen>
#include <QGuiApplication>

// QtCommon.
#include "qt_common/utils/qt_util.h"

// Infra.
#include "common/rga_shared_utils.h"

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_build_settings_view_binary.h"
#include "radeon_gpu_analyzer_gui/qt/rg_build_settings_widget.h"
#include "radeon_gpu_analyzer_gui/qt/rg_build_view_binary.h"
#include "radeon_gpu_analyzer_gui/qt/rg_cli_output_view.h"
#include "radeon_gpu_analyzer_gui/qt/rg_isa_disassembly_view.h"
#include "radeon_gpu_analyzer_gui/qt/rg_isa_disassembly_view_binary.h"
#include "radeon_gpu_analyzer_gui/qt/rg_maximize_splitter.h"
#include "radeon_gpu_analyzer_gui/qt/rg_menu_build_settings_item.h"
#include "radeon_gpu_analyzer_gui/qt/rg_menu_file_item_opencl.h"
#include "radeon_gpu_analyzer_gui/qt/rg_menu_binary.h"
#include "radeon_gpu_analyzer_gui/qt/rg_menu_titlebar.h"
#include "radeon_gpu_analyzer_gui/qt/rg_rename_project_dialog.h"
#include "radeon_gpu_analyzer_gui/qt/rg_source_code_editor.h"
#include "radeon_gpu_analyzer_gui/qt/rg_source_code_tab_widget.h"
#include "radeon_gpu_analyzer_gui/qt/rg_source_editor_titlebar.h"
#include "radeon_gpu_analyzer_gui/qt/rg_source_search_directories_view.h"
#include "radeon_gpu_analyzer_gui/qt/rg_view_container.h"
#include "radeon_gpu_analyzer_gui/qt/rg_view_manager.h"
#include "radeon_gpu_analyzer_gui/rg_cli_launcher.h"
#include "radeon_gpu_analyzer_gui/rg_definitions.h"
#include "radeon_gpu_analyzer_gui/rg_factory_binary.h"
#include "radeon_gpu_analyzer_gui/rg_string_constants.h"
#include "radeon_gpu_analyzer_gui/rg_utils.h"
#include "radeon_gpu_analyzer_gui/rg_xml_session_config.h"
#include "common/rga_entry_type.h"

// Returns the stage-specific file extension for an entrypoint type name string.
static std::string GetExtensionForEntrypointType(const std::string& type_name)
{
    auto entry_type = RgaEntryTypeUtils::GetEntryType(type_name);
    return entry_type ? RgaEntryTypeUtils::GetStageExtension(*entry_type) : "";
}

// Returns true if the extension is a known stage-specific shader extension.
// Files with generic extensions (e.g. .glsl, .hlsl, .spv, .hip) return false.
static bool IsStageSpecificExtension(const std::string& ext)
{
    for (const auto& type : {RgaEntryType::kVkVertex,   RgaEntryType::kVkGeometry,   RgaEntryType::kVkFragment,
                              RgaEntryType::kVkTessControl, RgaEntryType::kVkTessEval, RgaEntryType::kVkCompute,
                              RgaEntryType::kVkMesh,     RgaEntryType::kVkTask,
                              RgaEntryType::kGlVertex,   RgaEntryType::kGlGeometry,   RgaEntryType::kGlFragment,
                              RgaEntryType::kGlTessControl, RgaEntryType::kGlTessEval, RgaEntryType::kGlCompute,
                              RgaEntryType::kDxVertex,   RgaEntryType::kDxHull,       RgaEntryType::kDxDomain,
                              RgaEntryType::kDxGeometry, RgaEntryType::kDxPixel,      RgaEntryType::kDxCompute,
                              RgaEntryType::kDxMesh,     RgaEntryType::kDxAmplification})
    {
        if (RgaEntryTypeUtils::GetStageExtension(type) == ext)
            return true;
    }
    return false;
}


RgBuildViewBinary::RgBuildViewBinary(QWidget* parent)
    : RgBuildView(RgProjectAPI::kBinary, parent)
{
    tab_widget_ = new RgSourceCodeTabWidget(this);

    // Connect to the specific editor's signals.
    [[maybe_unused]] bool is_connected =
        connect(tab_widget_, &RgSourceCodeTabWidget::SourceCodeEditorChanged, this, &RgBuildViewBinary::HandleCurrentCodeEditorChanged);
    assert(is_connected);

    connect(tab_widget_, &RgSourceCodeTabWidget::SourceTabClosed, this, [this](const std::string& filepath)
    {
        user_closed_source_files_.insert(filepath);
    });
}

void RgBuildViewBinary::BuildCurrentProject()
{
    // Reset so substitute_paths are re-derived with the current search dirs.
    already_prompted_for_source_dirs_ = false;

    // Run pre-analysis source discovery before the build.
    // If the user provides search directories, they are saved to build settings
    // so the subsequent build picks them up via -I / --source-dir.
    PreAnalysisSourceDiscovery();

    // Proceed with normal build.
    RgBuildView::BuildCurrentProject();
}

bool RgBuildViewBinary::PreAnalysisSourceDiscovery(bool force_prompt)
{
    auto build_settings = std::dynamic_pointer_cast<RgBuildSettingsBinary>(
        project_->clones[clone_index_]->build_settings);
    if (build_settings == nullptr)
    {
        return false;
    }

    // Collect DWARF source paths from all binary files in the project.
    std::vector<std::string> binary_file_paths;
    RgConfigManager::Instance().GetProjectBinaryFilePaths(project_, clone_index_, binary_file_paths);

    std::vector<std::string> all_dwarf_paths;
    for (const auto& binary_path : binary_file_paths)
    {
        std::vector<std::string> source_paths;
        if (RgCliLauncher::ListSourcePaths(binary_path, source_paths))
        {
            for (const auto& src_path : source_paths)
            {
                if (!RgUtils::IsFileExists(src_path))
                {
                    all_dwarf_paths.push_back(src_path);
                }
            }
        }
    }

    if (all_dwarf_paths.empty())
    {
        return false;
    }

    // Always re-derive substitute_paths from the current search dirs + DWARF paths.
    // substitute_paths is ephemeral, never persisted, and SaveSettings() replaces
    // the build_settings pointer which discards any previously derived pairs.
    const auto& search_dirs = build_settings->additional_include_directories;
    build_settings->substitute_paths.clear();

    QStringList unresolved_files;
    for (const auto& src_path : all_dwarf_paths)
    {
        // Split on either separator so cross-compiled DWARF paths decompose correctly.
        auto        sep_pos  = src_path.find_last_of("/\\");
        std::string dwarf_dir = (sep_pos != std::string::npos) ? src_path.substr(0, sep_pos) : "";
        std::string filename  = (sep_pos != std::string::npos) ? src_path.substr(sep_pos + 1) : src_path;
        bool        resolved  = false;

        for (const auto& search_dir : search_dirs)
        {
            std::filesystem::path candidate = std::filesystem::path(search_dir) / filename;
            std::error_code       ec;
            if (std::filesystem::exists(candidate, ec) && std::filesystem::is_regular_file(candidate, ec))
            {
                build_settings->substitute_paths.push_back({dwarf_dir, search_dir});
                resolved = true;
                break;
            }
        }

        if (!resolved)
        {
            QString qpath = QString::fromStdString(src_path);
            if (!unresolved_files.contains(qpath))
            {
                unresolved_files.append(qpath);
            }
        }
    }

    // Skip the dialog prompt if we've already prompted this cycle.
    if (already_prompted_for_source_dirs_ && !force_prompt)
    {
        return false;
    }

    // All DWARF paths resolved by existing search dirs, no prompt needed
    // unless the user explicitly clicked "Click here" (force_prompt).
    if (unresolved_files.isEmpty() && !force_prompt)
    {
        return false;
    }

    if (!force_prompt && !build_settings->prompt_to_attach_source_dirs)
    {
        already_prompted_for_source_dirs_ = true;
        return false;
    }

    // Show dialog listing DWARF paths that still need resolution.
    // When force_prompt is true (user clicked "Click here"), show all DWARF paths
    // so the user can see the full picture and adjust search directories.
    QStringList dialog_files;
    if (force_prompt && unresolved_files.isEmpty())
    {
        for (const auto& p : all_dwarf_paths)
        {
            dialog_files.append(QString::fromStdString(p));
        }
    }
    else
    {
        dialog_files = unresolved_files;
    }

    static const char kDelimiter[] = ";";
    RgSourceSearchDirectoriesView dialog(kDelimiter, dialog_files, this);

    QString existing_dirs = QString::fromStdString(
        RgUtils::BuildSemicolonSeparatedStringList(search_dirs));
    dialog.SetListItems(existing_dirs);

    bool        user_accepted = false;
    QStringList new_dirs_list;
    connect(&dialog, &RgIncludeDirectoriesView::OKButtonClicked,
        [&user_accepted, &new_dirs_list](QStringList dirs)
        {
            user_accepted = true;
            new_dirs_list = dirs;
        });

    dialog.setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter,
        dialog.size(), QGuiApplication::primaryScreen()->availableGeometry()));
    dialog.exec();

    already_prompted_for_source_dirs_ = true;

    // Check whether the user actually changed the directory list.
    QStringList existing_dirs_list;
    for (const auto& dir : search_dirs)
    {
        existing_dirs_list.append(QString::fromStdString(dir));
    }
    bool dirs_changed = (new_dirs_list != existing_dirs_list);

    if (user_accepted && !new_dirs_list.isEmpty() && dirs_changed)
    {
        // Save the directories through the build settings view so the UI stays in sync.
        // Note: SaveSettings() replaces the clone's build_settings pointer, so we must
        // re-acquire it afterward to set substitute_paths on the live object.
        RgBuildSettingsViewBinary* settings_view =
            dynamic_cast<RgBuildSettingsViewBinary*>(build_settings_view_);
        if (settings_view != nullptr)
        {
            settings_view->HandleIncludeDirsUpdated(new_dirs_list);
            settings_view->SaveSettings();
        }
        else
        {
            // Fallback: directly update the project settings.
            std::vector<std::string> new_search_dirs;
            for (const auto& dir : new_dirs_list)
            {
                if (!dir.isEmpty())
                {
                    new_search_dirs.push_back(dir.toStdString());
                }
            }
            build_settings->additional_include_directories = new_search_dirs;
            SaveProjectConfigFile();
        }

        // Re-acquire build settings, SaveSettings() may have replaced the pointer.
        auto updated_settings = std::dynamic_pointer_cast<RgBuildSettingsBinary>(
            project_->clones[clone_index_]->build_settings);

        // Re-derive substitute pairs for ALL DWARF paths against the updated search dirs.
        // We must derive all of them (not just the previously unresolved ones) because
        // SaveSettings() replaced the build_settings pointer, discarding substitute_paths
        // that were derived on the old object earlier in this function.
        const auto& saved_dirs = updated_settings->additional_include_directories;
        for (const auto& src_path : all_dwarf_paths)
        {
            auto        sep_pos   = src_path.find_last_of("/\\");
            std::string dwarf_dir = (sep_pos != std::string::npos) ? src_path.substr(0, sep_pos) : "";
            std::string filename  = (sep_pos != std::string::npos) ? src_path.substr(sep_pos + 1) : src_path;

            for (const auto& search_dir : saved_dirs)
            {
                std::filesystem::path candidate = std::filesystem::path(search_dir) / filename;
                std::error_code       ec;
                if (std::filesystem::exists(candidate, ec) && std::filesystem::is_regular_file(candidate, ec))
                {
                    updated_settings->substitute_paths.push_back({dwarf_dir, search_dir});
                    break;
                }
            }
        }

        return true;
    }

    return false;
}

void RgBuildViewBinary::ConnectBuildSettingsSignals()
{
    // Connect the parent's signals.
    RgBuildView::ConnectBuildSettingsSignals();

    RgBuildSettingsViewBinary* build_settings_view_binary = static_cast<RgBuildSettingsViewBinary*>(build_settings_view_);
    assert(build_settings_view_binary != nullptr);

    if (build_settings_view_binary != nullptr)
    {
        bool is_connected = connect(build_settings_view_binary,
                                    &RgBuildSettingsViewBinary::PendingChangesStateChanged,
                                    this,
                                    &RgBuildView::HandleBuildSettingsPendingChangesStateChanged);
        assert(is_connected);

        is_connected = connect(build_settings_view_binary, &RgBuildSettingsViewBinary::ProjectBuildSettingsSaved, this, &RgBuildView::HandleBuildSettingsSaved);
        assert(is_connected);

        // Connect to build settings view's edit line's "focus in" event to color the frame green.
        is_connected =
            connect(build_settings_view_binary, &RgBuildSettingsViewBinary::SetFrameBorderPurpleSignal, this, &RgBuildView::HandleSetFrameBorderPurple);
        assert(is_connected);

        // Connect to build settings view's edit line's "focus out" event to color the frame black.
        is_connected =
            connect(build_settings_view_binary, &RgBuildSettingsViewBinary::SetFrameBorderBlackSignal, this, &RgBuildView::HandleSetFrameBorderBlack);
        assert(is_connected);
    }
}

bool RgBuildViewBinary::ConnectMenuSignals()
{
    bool is_connected = false;

    // Connect the file menu's file item entry point changed signal.
    assert(file_menu_ != nullptr);
    if (file_menu_ != nullptr)
    {
        RgMenuBinary* menu_binary = static_cast<RgMenuBinary*>(file_menu_);
        assert(menu_binary != nullptr);
        if (menu_binary != nullptr)
        {
            // Connect the OpenCL menu's "Selected entry point changed" handler.
            is_connected = connect(menu_binary, &RgMenuBinary::SelectedEntrypointChanged, this, &RgBuildViewBinary::HandleSelectedEntrypointChanged);
            assert(is_connected);

            // Connect the file menu item's drag and drop handler.
            is_connected = connect(menu_binary, &RgMenuBinary::DragAndDropExistingFile, this, &RgBuildViewBinary::HandleExistingFileDragAndDrop);
            assert(is_connected);

            // Connect the RgBuildView's entry point changed signal to the file menu's handler.
            is_connected = connect(this, &RgBuildViewBinary::SelectedEntrypointChanged, menu_binary, &RgMenuBinary::HandleSelectedEntrypointChanged);
            assert(is_connected);

            // Connect the file menu item selection handler for each new item.
            is_connected = connect(menu_binary, &RgMenuBinary::MenuItemClicked, this, &RgBuildViewBinary::HandleMenuItemClicked);
            assert(is_connected);

            // Notify the file menu that a new source file has been added.
            is_connected = connect(this, &RgBuildViewBinary::AddedSourceFileToProject, file_menu_, &RgMenuBinary::HandleSourceFileAdded);
            assert(is_connected);
        }
    }

    return is_connected;
}

void RgBuildViewBinary::CurrentBuildCancelled()
{
    assert(file_menu_ != nullptr);
    if (file_menu_ != nullptr)
    {
        RgMenuBinary* menu_binary = static_cast<RgMenuBinary*>(file_menu_);
        assert(menu_binary != nullptr);

        if (menu_binary != nullptr)
        {
            // Don't allow the user to expand file item's entry point list.
            menu_binary->SetIsShowEntrypointListEnabled(false);
        }
    }

    // Remove all the generated files in output directory.
    DestroyProjectBuildArtifacts();
}

void RgBuildViewBinary::CurrentBuildSucceeded()
{
    // Load the start line numbers for each entrypoint.
    LoadEntrypointLineNumbers();

    // Create read-only editors for src files.
    CreateSourceCodeWidgets();

    assert(file_menu_ != nullptr);
    if (file_menu_ != nullptr)
    {
        RgMenuBinary* menu_binary = static_cast<RgMenuBinary*>(file_menu_);
        assert(menu_binary != nullptr);
        if (menu_binary != nullptr)
        {
            // Allow the user to expand the file's entry point list.
            menu_binary->SetIsShowEntrypointListEnabled(true);
        }
    }

    // Update the file menu item with the clone's build output.
    RgMenuBinary* menu_binary = static_cast<RgMenuBinary*>(file_menu_);
    menu_binary->UpdateBuildOutput(build_outputs_);

    // Update the correlation state for each tab widget based on which binary files were built successfully.
    std::string                       output_gpu;
    std::shared_ptr<RgCliBuildOutput> build_output  = nullptr;
    bool                              isOutputValid = RgUtils::GetFirstValidOutputGpu(build_outputs_, output_gpu, build_output);
    if (isOutputValid && build_output != nullptr)
    {
        if (tab_widget_ != nullptr)
        {
            // Emit the signal used to update the correlation enabledness.
            tab_widget_->ForEachEditor([this](RgSourceCodeEditor* editor) { 
                if (editor != nullptr)
                {
                    emit LineCorrelationEnabledStateChanged(editor, true);
                }
            });
        }
    }

    // Save project file changes post build.
    SaveProjectConfigFile();
}

bool RgBuildViewBinary::CreateMenu(QWidget* parent)
{
    file_menu_ = static_cast<RgMenuBinary*>(factory_->CreateFileMenu(parent));

    // Notify the file menu when the build succeeded.
    bool is_connected = connect(this, &RgBuildViewBinary::ProjectBuildSuccess, file_menu_, &RgMenuBinary::ProjectBuildSuccess);
    assert(is_connected);

    // Notify the file menu to update file menu item coloring
    // when an already built project is being loaded.
    is_connected = connect(this, &RgBuildViewBinary::UpdateFileColoring, file_menu_, &RgMenuBinary::ProjectBuildSuccess);
    assert(is_connected);

    connect(&QtCommon::QtUtils::ColorTheme::Get(), &QtCommon::QtUtils::ColorTheme::ColorThemeUpdated, this, &RgBuildViewBinary::ReapplyMenuStyleSheet);

    return file_menu_ != nullptr;
}

void RgBuildViewBinary::ReapplyMenuStyleSheet()
{
    factory_->ApplyFileMenuStylesheet(file_menu_);
}

void RgBuildViewBinary::ConnectDisassemblyViewApiSpecificSignals()
{
    assert(disassembly_view_ != nullptr);
    if (disassembly_view_ != nullptr)
    {
        // Connect the handler invoked when the user changes the selected entrypoint.
        bool is_connected =
            connect(this, &RgBuildViewBinary::SelectedEntrypointChanged, disassembly_view_, &RgIsaDisassemblyView::HandleSelectedEntrypointChanged);
        assert(is_connected);

        // Connect the RgIsaDisassemblyView's entry point changed handler.
        is_connected = connect(disassembly_view_, &RgIsaDisassemblyView::SelectedEntrypointChanged, this, &RgBuildViewBinary::HandleSelectedEntrypointChanged);
        assert(is_connected);

        // Connect the handler invoked when the user changes the selected entrypoint.
        is_connected =
            connect(this, &RgBuildViewBinary::SelectedExtremelyLongKernelNameChanged, disassembly_view_, &RgIsaDisassemblyView::HandleSetKernelNameLabel);
        assert(is_connected);
    }
}

void RgBuildViewBinary::DestroyProjectBuildArtifacts()
{
    bool destroy_build_artifacts = true;

    // Verify that the clone index is valid for the given project.
    int  num_clones           = static_cast<int>(project_->clones.size());
    bool is_clone_index_valid = (clone_index_ >= 0 && clone_index_ < num_clones);
    assert(is_clone_index_valid);
    if (is_clone_index_valid)
    {
        std::shared_ptr<RgProjectClone> binary_clone = project_->clones[clone_index_];
        assert(binary_clone != nullptr);
        if (binary_clone != nullptr)
        {
            for (const auto& binary : binary_clone->binary_files)
            {
                if (binary.is_disassembly_generated)
                {
                    // We have atleast one build artifact that need to be retained.
                    destroy_build_artifacts = false;
                    break;
                }
            }
        }
    }

    if (destroy_build_artifacts)
    {
        // Invoke the base implementation used to destroy project build artifacts.
        RgBuildView::DestroyProjectBuildArtifacts();

        // Clear any old build artifacts from the OpenCL-specific file menu items.
        ClearFileItemsEntrypointList();
    }
}

void RgBuildViewBinary::RemoveInputFile(const std::string& input_file_full_path)
{
    RgConfigManager& config_manager = RgConfigManager::Instance();

    // Remove the file from the project.
    config_manager.RemoveProjectBinaryFilePath(project_, clone_index_, input_file_full_path);
    config_manager.SaveProjectFile(project_);

    RgMenu* menu = GetMenu();
    assert(menu != nullptr);
    if (menu != nullptr)
    {
        // Remove the file from the file menu.
        menu->RemoveItem(input_file_full_path);
    }

    // Remove the associated file editor.
    RemoveEditor(input_file_full_path);

    // Clean up outputs from previous builds associated with this file.
    DestroyBuildOutputsForFile(input_file_full_path);

    // Remove the file's build outputs from the disassembly view.
    if (disassembly_view_ != nullptr)
    {
        disassembly_view_->RemoveInputFileEntries(input_file_full_path);

        // Hide the disassembly view when there's no data in it.
        if (disassembly_view_->IsEmpty() && disassembly_view_splitter_ != nullptr)
        {
            // Minimize the disassembly view before hiding it to preserve correct RgBuildView layout.
            disassembly_view_splitter_->Restore();


            // Hide the disassembly view now that it's empty.
            ToggleDisassemblyViewVisibility(false);
        }
        else
        {
            // Trigger a correlation update after the source file has been removed.
            HandleSelectedTargetGpuChanged(current_target_gpu_);
        }
    }

    RemoveFileFromMetadata(input_file_full_path);
}

RgMenu* RgBuildViewBinary::GetMenu() const
{
    return file_menu_;
}

void RgBuildViewBinary::FocusOnFileMenu()
{
    // Switch the focus to the file menu.
    if (file_menu_ != nullptr)
    {
        file_menu_->setFocus();
    }
}

bool RgBuildViewBinary::PopulateMenu()
{
    bool ret = false;

    std::vector<std::string> binary_file_paths;
    RgConfigManager::Instance().GetProjectBinaryFilePaths(project_, clone_index_, binary_file_paths);

    if (!binary_file_paths.empty())
    {
        // Add all the project's source files into the RgBuildView.
        for (int file_index = 0; file_index < binary_file_paths.size(); ++file_index)
        {
            const std::string& file_path = binary_file_paths.at(file_index);

            // Check that the file still exists before attempting to load it.
            bool is_file_exists = RgUtils::IsFileExists(file_path);
            assert(is_file_exists);
            if (is_file_exists)
            {
                // Add the selected file to the menu.
                if (AddFile(file_path))
                {
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

bool RgBuildViewBinary::IsGcnDisassemblyGenerated(const std::string& input_file_path) const
{
    bool is_current_file_disassembled = false;

    auto targetGpuOutputsIter = build_outputs_.find(current_target_gpu_);
    if (targetGpuOutputsIter != build_outputs_.end())
    {
        assert(targetGpuOutputsIter->second != nullptr);
        if (targetGpuOutputsIter->second != nullptr)
        {
            auto inputFileOutputsIter = targetGpuOutputsIter->second->per_file_output.find(input_file_path);
            if (inputFileOutputsIter != targetGpuOutputsIter->second->per_file_output.end())
            {
                RgFileOutputs& fileOutputs   = inputFileOutputsIter->second;
                is_current_file_disassembled = !fileOutputs.outputs.empty();
            }
        }
    }

    return is_current_file_disassembled;
}

void ReadCsvFileForLineRanges(const std::string& csv_file_full_path, std::map<std::string, std::pair<uint32_t, uint32_t>>& src_file_line_ranges)
{
    QFile       csv_file(csv_file_full_path.c_str());
    QTextStream file_stream(&csv_file);

    bool is_file_opened = csv_file.open(QFile::ReadOnly | QFile::Text);
    assert(is_file_opened);
    if (is_file_opened)
    {
        // Read header line and check if line correlation columns are present.
        if (file_stream.atEnd())
        {
            return;
        }
        std::string header = file_stream.readLine().toStdString();
        if (header.find(kStrCsvColumnSourceLineNumber) == std::string::npos)
        {
            return;
        }

        // Parse each data row.
        while (!file_stream.atEnd())
        {
            std::string                line = file_stream.readLine().toStdString();
            std::vector<std::string>   line_tokens;
            std::vector<std::string>   operand_tokens;
            RgaSharedUtils::ParseCsvLine(line, line_tokens, operand_tokens);

            if (line_tokens.size() < static_cast<size_t>(RgCsvFileColumns::kCount))
            {
                continue;
            }

            const std::string& source_line_str = line_tokens[static_cast<int>(RgCsvFileColumns::kSourceLineNumber)];
            const std::string& source_path     = line_tokens[static_cast<int>(RgCsvFileColumns::kSourcePath)];

            if (source_line_str.empty() || source_path.empty() || source_path == kStrUnknownSourcePath)
            {
                continue;
            }

            uint32_t line_number = static_cast<uint32_t>(std::stoi(source_line_str));
            if (line_number == 0)
            {
                continue;
            }

            auto it = src_file_line_ranges.find(source_path);
            if (it == src_file_line_ranges.end())
            {
                src_file_line_ranges[source_path] = {line_number, line_number};
            }
            else
            {
                it->second.first  = std::min(it->second.first, line_number);
                it->second.second = std::max(it->second.second, line_number);
            }
        }
    }
}

bool RgBuildViewBinary::IsNonApplicationSourceTab(const std::string& file_path)
{
    // Extract the file extension (everything after the last dot).
    std::string ext;
    auto dot_pos = file_path.rfind('.');
    if (dot_pos != std::string::npos)
    {
        ext = file_path.substr(dot_pos);
        // Convert to lowercase for case-insensitive comparison.
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    }

    return (ext == ".h" || ext == ".hpp" || ext == ".hxx" || ext == ".hh" || ext == ".inc" || ext == ".inl");
}

bool RgBuildViewBinary::IsUserClosedSourceTab(const std::string& file_path) const
{
    return user_closed_source_files_.find(file_path) != user_closed_source_files_.end();
}

bool RgBuildViewBinary::LoadSessionMetadata(const std::string& metadata_file_path, std::shared_ptr<RgCliBuildOutput>& build_output)
{
    bool ret = false;

    std::shared_ptr<RgCliBuildOutputOpencl> gpu_output_opencl = nullptr;
    ret                                                       = RgXMLSessionConfig::ReadSessionMetadataOpenCL(metadata_file_path, gpu_output_opencl);
    if (ret == false)
    {
        std::shared_ptr<RgCliBuildOutputPipeline> gpu_output_vulkan = nullptr;
        ret = RgXMLSessionConfig::ReadSessionMetadataVulkan(metadata_file_path, gpu_output_vulkan, true);
        if (ret)
        {
            build_output = gpu_output_vulkan;
        }
        else
        {
            build_output = nullptr;
        }
    }
    else
    {
        if (project_ != nullptr && clone_index_ >= 0 && clone_index_ < project_->clones.size() && project_->clones[clone_index_] != nullptr)
        {
            for (const auto& binary_file_name : gpu_output_opencl->project_binaries)
            {
                // Search the list of binary file info for the one that matches the given editor.
                auto                     first_file = project_->clones[clone_index_]->binary_files.begin();
                auto                     last_file  = project_->clones[clone_index_]->binary_files.end();
                RgSourceFilePathSearcher path_searcher(binary_file_name);
                auto                     file_iter = std::find_if(first_file, last_file, path_searcher);
                assert(file_iter != last_file);
                if (file_iter != last_file)
                {
                    // Update the disassembly state for the code object file.
                    file_iter->is_disassembly_generated = true;

                    // Update the target gpu.
                    auto& file_output = gpu_output_opencl->per_file_output[binary_file_name];
                    if (!file_output.outputs.empty())
                    {
                        std::sort(file_output.outputs.begin(), file_output.outputs.end(), RgEntryOutputComparator());
                        {
                            const auto& entry_output = file_output.outputs.front();
                            if (!entry_output.outputs.empty())
                            {
                                file_iter->target_gpu = entry_output.outputs.front().gpu_name;
                            }
                        }
                    }
                }
            }
        }

        build_output = gpu_output_opencl;
    }

    return ret;
}

void RgBuildViewBinary::ShowCurrentFileDisassembly()
{
    bool is_current_file_disassembled = false;

    // Show the currently selected file's first entry point disassembly (if there is no currently selected entry).
    const std::string& input_filepath     = file_menu_->GetSelectedFilePath();
    RgMenuFileItem*    selected_file_item = file_menu_->GetSelectedFileItem();
    assert(selected_file_item != nullptr);

    if (selected_file_item != nullptr)
    {
        RgMenuFileItemOpencl* opencl_file_item = static_cast<RgMenuFileItemOpencl*>(selected_file_item);
        assert(opencl_file_item != nullptr);

        if (opencl_file_item != nullptr)
        {
            std::string current_entrypoint_name;
            bool        is_entry_selected = opencl_file_item->GetSelectedEntrypointName(current_entrypoint_name);

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

                // Toggle the Kernel name label in disassembly view.
                ToggleDisassemblyViewKernelLabelVisiblity(opencl_file_item, entrypoint_name);

                is_current_file_disassembled = true;
            }
        }
    }

    // Toggle the view based on if the current file has been disassembled or not.
    ToggleDisassemblyViewVisibility(is_current_file_disassembled);

    // If this binary has no line correlation data, maximize the disassembly view
    // so the empty source code panel is not shown.
    if (is_current_file_disassembled && project_ != nullptr && clone_index_ >= 0 && clone_index_ < project_->clones.size())
    {
        auto first_file = project_->clones[clone_index_]->binary_files.begin();
        auto last_file  = project_->clones[clone_index_]->binary_files.end();
        RgSourceFilePathSearcher path_searcher(input_filepath);
        auto file_iter = std::find_if(first_file, last_file, path_searcher);
        if (file_iter != last_file && !file_iter->is_correlated)
        {
            MaximizeDisassemblyView();
        }
    }
}

void RgBuildViewBinary::SaveCurrentFile(EditMode)
{
    // Don't need to do anything here.
    // N/A for Binary Analysis mode.
}

bool RgBuildViewBinary::AddFile(const std::string& file_full_path, bool is_new_file)
{
    bool was_added = false;
    if (file_menu_)
    {
        RgMenuBinary* file_menu = nullptr;
        file_menu               = static_cast<RgMenuBinary*>(file_menu_);
        if (file_menu != nullptr)
        {
            was_added = file_menu->AddItem(file_full_path, is_new_file);
        }
    }

    return was_added;
}

RgSourceCodeEditor* RgBuildViewBinary::GetEditorForFilepath(const std::string& full_file_path, ShaderSourceLanguage lang)
{
    RgSourceCodeEditor* editor = nullptr;

    if (project_ != nullptr && clone_index_ >= 0 && clone_index_ < project_->clones.size() && project_->clones[clone_index_] != nullptr)
    {
        auto                     first_file = project_->clones[clone_index_]->binary_files.begin();
        auto                     last_file  = project_->clones[clone_index_]->binary_files.end();
        RgSourceFilePathSearcher path_searcher(full_file_path);
        auto                     file_iter = std::find_if(first_file, last_file, path_searcher);
        if (file_iter == last_file)
        {
            editor = RgBuildView::GetEditorForFilepath(full_file_path, lang);
        }
    }

    return editor;
}

void RgBuildViewBinary::SetDefaultFocusWidget() const
{
    assert(build_settings_view_ != nullptr);
    if (build_settings_view_ != nullptr)
    {
        build_settings_view_->SetInitialWidgetFocus();
    }
}

void RgBuildViewBinary::HandleExistingFileDragAndDrop(const std::vector<std::string>& file_paths_to_add)
{
    assert(file_menu_ != nullptr);
    if (file_menu_ != nullptr && !file_paths_to_add.empty())
    {
        AddExistingCodeObjFileToProject(file_paths_to_add);
    }
}

bool RgBuildViewBinary::AddExistingCodeObjFileToProject(const std::vector<std::string>& bin_file_paths)
{
    // Reset the source directory prompt flag since new binaries are being added.
    // Existing substitute_paths are preserved, they still apply to previously loaded binaries.
    already_prompted_for_source_dirs_ = false;

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
        RgMenuBinary* file_menu = nullptr;
        file_menu               = static_cast<RgMenuBinary*>(file_menu_);
        if (file_menu != nullptr)
        {
            RgConfigManager& config_manager = RgConfigManager::Instance();

            // Get the initial entry name from the application arguments if there is one.
            QStringList file_and_entry_name;

            if (QCoreApplication::arguments().size() > 1)
            {
                file_and_entry_name = QCoreApplication::arguments().at(1).split("::");
            }

            QString initial_entry_name = "";
            if (file_and_entry_name.size() > 1)
            {
                initial_entry_name = file_and_entry_name[1];
            }

            bool file_already_in_project = false;

            std::vector<std::string> new_binaries_to_disassemble;

            for (auto bin_file_path : bin_file_paths)
            {
                if (!IsCodeObjFileInProject(bin_file_path))
                {
                    // Add the code obj file's path to the project's clone.
                    ret = config_manager.AddCodeObjFileToProject(bin_file_path, project_, clone_index_, initial_entry_name);

                    if (!ret)
                    {
                        // Report the error.
                        std::stringstream msg;
                        msg << kStrErrCannotAddFileA << bin_file_path;
                        RgUtils::ShowErrorMessageBox(msg.str().c_str(), this);

                        continue;
                    }

                    // Add the selected file to the menu.
                    AddFile(bin_file_path);

                    // Remember most recent time file was modified.
                    QFileInfo file_info(bin_file_path.c_str());
                    binary_file_modified_time_map_[bin_file_path] = file_info.lastModified();

                    // This will make the newly-added file the current item,
                    // so grey out the Build settings button.
                    emit AddedSourceFileToProject();

                    new_binaries_to_disassemble.push_back(bin_file_path);
                }
                else
                {
                    file_already_in_project = true;
                }
            }

            if (file_already_in_project)
            {
                if (bin_file_paths.size() == 1)
                {
                    // If they was only one file being added but failed tell them the file already exists.
                    std::stringstream msg;
                    msg << kStrErrCannotAddFileA << bin_file_paths.front() << kStrErrCannotAddFileB;
                    RgUtils::ShowErrorMessageBox(msg.str().c_str(), this);
                }
                else if (bin_file_paths.size() > 1)
                {
                    // If there were multiple files and one or more files could not be added, tell them not all files could be added.
                    std::stringstream msg;
                    msg << kStrErrCannotAddMultiFile;
                    RgUtils::ShowErrorMessageBox(msg.str().c_str(), this);
                }
            }

            // Only rebuild the project if any files were actually added.
            if (new_binaries_to_disassemble.size() > 0)
            {
                // Save the project after adding a code obj.
                config_manager.SaveProjectFile(project_);

                // This will enable the build action.
                emit ProjectFileCountChanged(false);

                // Trigger a project build event.
                emit BuildProjectEvent();
            }
        }
    }

    return ret;
}

void RgBuildViewBinary::HandleSelectedFileChanged(const std::string& old_file_path, const std::string& new_file_path)
{
    bool is_switched = SwitchToTabWidget(new_file_path);
    if (is_switched)
    {
        // Show source code editors for the new binary, hide those that are not relevant.
        if (tab_widget_ != nullptr && new_file_path != old_file_path)
        {
            tab_widget_->HideAllEditors();
            auto it = binary_to_src_files_map_.find(new_file_path);
            if (it != binary_to_src_files_map_.end())
            {
                for (const auto& src_file : it->second)
                {
                    // Don't show tabs for non-application sources (headers) or
                    // files the user has explicitly closed.
                    if (!IsNonApplicationSourceTab(src_file) && !IsUserClosedSourceTab(src_file))
                    {
                        tab_widget_->ShowEditor(src_file);
                    }
                }
            }
        }

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
                        std::string selected_entrypoint_name;
                        bool        is_entry_point_selected;

                        std::string initial_function_name = project_->clones[clone_index_]->build_settings->initial_entry_name;
                        static bool first_show            = true;
                        if (initial_function_name != "" && first_show)
                        {
                            selected_entrypoint_name = initial_function_name;
                            is_entry_point_selected  = true;
                            first_show               = false;
                        }
                        else
                        {
                            // Retrieve the name of the currently-selected entry point (if there is one).
                            is_entry_point_selected = file_item_opencl->GetSelectedEntrypointName(selected_entrypoint_name);
                        }

                        // Update the visibility of the disassembly view.
                        ToggleDisassemblyViewVisibility(is_entry_point_selected);

                        // Maximize or restore the disassembly view based on per-binary line correlation state.
                        if (is_entry_point_selected)
                        {
                            auto bin_first = project_->clones[clone_index_]->binary_files.begin();
                            auto bin_last  = project_->clones[clone_index_]->binary_files.end();
                            RgSourceFilePathSearcher bin_searcher(new_file_path);
                            auto bin_iter = std::find_if(bin_first, bin_last, bin_searcher);
                            if (bin_iter != bin_last)
                            {
                                if (!bin_iter->is_correlated)
                                {
                                    MaximizeDisassemblyView();
                                }
                                else if (disassembly_view_container_ != nullptr && disassembly_view_container_->IsInMaximizedState())
                                {
                                    disassembly_view_splitter_->Restore();
                                }
                            }
                        }

                        if (is_entry_point_selected)
                        {
                            // Make the disassembly view visible because the file has build outputs to display.
                            emit SelectedEntrypointChanged(current_target_gpu_, new_file_path, selected_entrypoint_name);

                            // Toggle the Kernel name label in disassembly view.
                            ToggleDisassemblyViewKernelLabelVisiblity(file_item_opencl, selected_entrypoint_name);

                            std::string entrypoint_name_key = RgUtils::GenerateEntrypointKey(new_file_path, "", selected_entrypoint_name);
                            auto        it                  = entrypoint_to_src_files_map_.find(entrypoint_name_key);
                            if (it != entrypoint_to_src_files_map_.end())
                            {
                                // Pick the source file whose extension matches the entrypoint type.
                                // For fused shaders, multiple source files map to the same entrypoint set,
                                // so extension matching selects the correct one (e.g. .vert for VK_Vertex).
                                const std::string expected_ext = GetExtensionForEntrypointType(selected_entrypoint_name);
                                auto              file_iter    = it->second.begin();
                                if (!expected_ext.empty())
                                {
                                    for (auto candidate = it->second.begin(); candidate != it->second.end(); ++candidate)
                                    {
                                        std::string ext;
                                        RgUtils::ExtractFileExtension(*candidate, ext);
                                        if (ext == expected_ext)
                                        {
                                            file_iter = candidate;
                                            break;
                                        }
                                    }
                                }
                                if (file_iter != it->second.end())
                                {
                                    RgSourceCodeEditor* filepath_editor = GetEditorForFilepath(*file_iter);
                                    RgSourceCodeEditor* current_editor  = tab_widget_->GetCurrentCodeEditor();
                                    if (filepath_editor != nullptr && tab_widget_ != nullptr)
                                    {
                                        if (filepath_editor != current_editor)
                                        {
                                            tab_widget_->SetCurrentCodeEditor(filepath_editor);

                                            // Update the titlebar for the current source editor.
                                            UpdateSourceEditorTitlebar(filepath_editor);
                                        }

                                        HighlightEntrypointStartLine(new_file_path, selected_entrypoint_name);

                                        // Always update correlation when switching binaries,
                                        // even if the source editor hasn't changed.
                                        const int selected_line_number = filepath_editor->GetSelectedLineNumber();
                                        const int correlated_line      = IsLineCorrelationSupported() ? selected_line_number : kInvalidCorrelationLineIndex;
                                        disassembly_view_->HandleInputFileSelectedLineChanged(
                                            current_target_gpu_, *file_iter, selected_entrypoint_name, correlated_line, new_file_path);
                                    }
                                }
                            }
                        }

                        // Set the target gpu name label.
                        SetTargetGpuLabel();
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

void RgBuildViewBinary::HandleSourceFileSelectedLineChanged(ShaderSourceCodeViewer* editor, int line_number)
{
    // Handle updating source correlation only when the project isn't currently being built.
    if (!is_build_in_progress_)
    {
        RgSourceCodeEditor* code_editor = qobject_cast<RgSourceCodeEditor*>(editor);

        if (disassembly_view_ != nullptr && !disassembly_view_->IsEmpty() && code_editor != nullptr &&
            !code_editor->IsPlaceholder())
        {
            RgMenuBinary* menu_binary = static_cast<RgMenuBinary*>(file_menu_);
            assert(menu_binary != nullptr);
            if (menu_binary != nullptr)
            {
                const std::string& binary_filename = menu_binary->GetSelectedFilePath();
                bool               is_disassembled = IsGcnDisassemblyGenerated(binary_filename);
                if (is_disassembled)
                {
                    int correlated_line_number = kInvalidCorrelationLineIndex;

                    bool is_correlation_enabled = IsLineCorrelationEnabled(code_editor);
                    if (is_correlation_enabled)
                    {
                        correlated_line_number = line_number;
                    }

                    const std::string& src_filename = GetFilepathForEditor(code_editor);
                    // If the line is associated with a named entrypoint, highlight it in the file menu item.
                    std::string entry_name;
                    bool        is_valid = GetEntrypointNameForLineNumber(binary_filename, src_filename, line_number, entry_name);
                    if (is_valid)
                    {
                        menu_binary->HandleSelectedEntrypointChanged(current_target_gpu_, binary_filename, entry_name);
                    }

                    // Send the input source file's correlation line index to the disassembly view.
                    disassembly_view_->HandleInputFileSelectedLineChanged(
                        current_target_gpu_, src_filename, entry_name, correlated_line_number, binary_filename);
                }
            }
        }
    }
}

void RgBuildViewBinary::HandleSelectedEntrypointChanged(const std::string& binary_filename, const std::string& selected_entrypoint_name)
{
    assert(file_menu_ != nullptr);
    if (file_menu_ != nullptr)
    {
        RgMenuBinary* file_menu_binary = static_cast<RgMenuBinary*>(file_menu_);
        if (file_menu_binary != nullptr)
        {
            // Trigger the file menu to be updated, which will change the current selection in the current item's entry point list.
            file_menu_binary->HandleSelectedEntrypointChanged(current_target_gpu_, binary_filename, selected_entrypoint_name);
        }
    }

    // Update the disassembly view to show the newly selected entrypoint.
    if (disassembly_view_ != nullptr)
    {
        disassembly_view_->HandleSelectedEntrypointChanged(current_target_gpu_, binary_filename, selected_entrypoint_name);
    }

    // Switch to the source tab that corresponds to this entrypoint.
    // Uses extension matching so fused shaders pick the correct file (e.g. VK_Vertex -> .vert).
    std::string entrypoint_name_key = RgUtils::GenerateEntrypointKey(binary_filename, "", selected_entrypoint_name);
    auto        it                  = entrypoint_to_src_files_map_.find(entrypoint_name_key);
    if (it != entrypoint_to_src_files_map_.end())
    {
        const std::string expected_ext = GetExtensionForEntrypointType(selected_entrypoint_name);
        auto              file_iter    = it->second.begin();
        if (!expected_ext.empty())
        {
            for (auto candidate = it->second.begin(); candidate != it->second.end(); ++candidate)
            {
                std::string ext;
                RgUtils::ExtractFileExtension(*candidate, ext);
                if (ext == expected_ext)
                {
                    file_iter = candidate;
                    break;
                }
            }
        }
        if (file_iter != it->second.end())
        {
            RgSourceCodeEditor* filepath_editor = GetEditorForFilepath(*file_iter);
            RgSourceCodeEditor* current_editor  = tab_widget_->GetCurrentCodeEditor();
            const bool          tab_changed     = (filepath_editor != nullptr && filepath_editor != current_editor);
            if (tab_changed && tab_widget_ != nullptr)
            {
                tab_widget_->SetCurrentCodeEditor(filepath_editor);
                UpdateSourceEditorTitlebar(filepath_editor);
            }

            // If the tab didn't change, HandleCurrentCodeEditorChanged won't fire, so update
            // correlation here. If the tab did change, HandleCurrentCodeEditorChanged handles it.
            if (!tab_changed && disassembly_view_ != nullptr && !disassembly_view_->IsEmpty() && filepath_editor != nullptr)
            {
                const std::string& src_filename = GetFilepathForEditor(filepath_editor);
                const int   line_number         = filepath_editor->GetSelectedLineNumber();
                int         correlated_line     = IsLineCorrelationEnabled(filepath_editor) ? line_number : kInvalidCorrelationLineIndex;
                std::string entry_name_copy     = selected_entrypoint_name;
                disassembly_view_->HandleInputFileSelectedLineChanged(
                    current_target_gpu_, src_filename, entry_name_copy, correlated_line, binary_filename);
            }
        }
    }

    // Highlight the start line for the given entry point in the source editor.
    HighlightEntrypointStartLine(binary_filename, selected_entrypoint_name);
}

void RgBuildViewBinary::HandleMenuItemClicked(RgMenuFileItem* item)
{
    if (ShowSaveDialog())
    {
        file_menu_->HandleSelectedFileChanged(item);
    }
}

void RgBuildViewBinary::HandleHighlightedCorrelationLineUpdated(int line_number, const std::string& src_path)
{
    std::string standardized_src = src_path;
    RgUtils::StandardizePathSeparator(standardized_src);

    RgSourceCodeEditor* editor    = nullptr;
    std::string         match_key;
    for (const auto& [key, value] : source_code_editors_)
    {
        std::string standardized_key = key;
        RgUtils::StandardizePathSeparator(standardized_key);
        if (standardized_key == standardized_src)
        {
            editor    = value;
            match_key = key;
            break;
        }
    }

    // For non-application sources (headers), ensure the tab is visible,
    // unless the user has explicitly closed it.
    if (editor != nullptr && IsNonApplicationSourceTab(match_key) && !IsUserClosedSourceTab(match_key))
    {
        tab_widget_->ShowEditor(match_key);
    }

    if (editor != nullptr && editor != current_code_editor_)
    {
        tab_widget_->SetCurrentCodeEditor(editor);
    }

    if (current_code_editor_ != nullptr)
    {
        // A list that gets filled with correlated line numbers to highlight in the source editor.
        QList<int> highlighted_lines;

        // Only fill up the list with valid correlated lines if possible. Otherwise, nothing will get highlighted.
        bool is_correlation_enabled = IsLineCorrelationEnabled(current_code_editor_);
        if (is_correlation_enabled)
        {
            // Only scroll to the highlighted line if it's a valid line number.
            if (line_number != kInvalidCorrelationLineIndex)
            {
                highlighted_lines.push_back(line_number);

                // Scroll the source editor to show the highlighted line.
                current_code_editor_->ScrollToLine(line_number);
            }
        }

        // Add the correlated input source line number to the editor's highlight list.
        current_code_editor_->HandleHighlightedLinesSet(highlighted_lines);
    }
}

void RgBuildViewBinary::HandleCurrentCodeEditorChanged(RgSourceCodeEditor* editor)
{
    if (editor == nullptr)
    {
        return;
    }

    current_code_editor_ = editor;
    UpdateSourceEditorSearchContext();

    const std::string& src_filename = GetFilepathForEditor(editor);
    if (src_filename.empty())
    {
        return;
    }

    // Resolve the entrypoint directly from the source file path to avoid bias toward
    // the previously selected entrypoint that occurs in GetEntrypointNameForLineNumber.
    const auto& file_src_line_data = entrypoint_line_numbers_.find(src_filename);
    if (file_src_line_data != entrypoint_line_numbers_.end() && !file_src_line_data->second.empty())
    {
        std::string entry_name;
        const int   line_number = editor->GetSelectedLineNumber();

        if (file_src_line_data->second.size() == 1)
        {
            // Single entrypoint, switch unconditionally.
            entry_name = file_src_line_data->second.begin()->first;
        }
        else
        {
            // Multiple entrypoints: first try to match by cursor line range.
            for (const auto& [name, range] : file_src_line_data->second)
            {
                if (line_number >= static_cast<int>(range.first) && line_number <= static_cast<int>(range.second))
                {
                    entry_name = name;
                    break;
                }
            }

            if (entry_name.empty())
            {
                // No line range matched, for fused shaders, use the source file extension
                // to determine which stage this file belongs to (e.g. .vert -> VK_Vertex).
                std::string src_ext;
                RgUtils::ExtractFileExtension(src_filename, src_ext);
                for (const auto& [name, range] : file_src_line_data->second)
                {
                    if (GetExtensionForEntrypointType(name) == src_ext)
                    {
                        entry_name = name;
                        break;
                    }
                }
            }

            if (entry_name.empty())
            {
                entry_name = file_src_line_data->second.begin()->first;
            }
        }

        RgMenuBinary* menu_binary = static_cast<RgMenuBinary*>(file_menu_);
        if (menu_binary != nullptr)
        {
            const std::string& binary_filename = menu_binary->GetSelectedFilePath();
            menu_binary->HandleSelectedEntrypointChanged(current_target_gpu_, binary_filename, entry_name);

            if (disassembly_view_ != nullptr && !disassembly_view_->IsEmpty())
            {
                int correlated_line = IsLineCorrelationEnabled(editor) ? line_number : kInvalidCorrelationLineIndex;
                disassembly_view_->HandleInputFileSelectedLineChanged(
                    current_target_gpu_, src_filename, entry_name, correlated_line, binary_filename);
            }
        }
    }
    else
    {
        HandleSourceFileSelectedLineChanged(editor, editor->GetSelectedLineNumber());
    }
}

void RgBuildViewBinary::ClearFileItemsEntrypointList()
{
    RgMenuBinary* menu_binary = static_cast<RgMenuBinary*>(file_menu_);
    assert(menu_binary != nullptr);
    if (menu_binary != nullptr)
    {
        // Clear references to build outputs from the file menu,
        menu_binary->ClearBuildOutputs();
    }
}

void RgBuildViewBinary::HighlightEntrypointStartLine(const std::string& binary_file_path, const std::string& selected_entrypoint_name)
{
    std::string entrypoint_key  = RgUtils::GenerateEntrypointKey(binary_file_path, "", selected_entrypoint_name);
    auto        input_file_iter = entrypoint_to_src_files_map_.find(entrypoint_key);
    if (input_file_iter != entrypoint_to_src_files_map_.end())
    {
        for (const auto& src_file_path : input_file_iter->second)
        {
            // Find the input file in the map of entry point start line numbers.
            auto src_file_iter = entrypoint_line_numbers_.find(src_file_path);
            if (src_file_iter != entrypoint_line_numbers_.end())
            {
                // Search for the start line number for the given entry point name.
                EntryToSourceLineRange& file_entrypoints_info = src_file_iter->second;
                auto                    lineNumberIter        = file_entrypoints_info.find(selected_entrypoint_name);
                if (lineNumberIter != file_entrypoints_info.end())
                {
                    RgSourceCodeEditor* editor = GetEditorForFilepath(src_file_path);
                    assert(editor != nullptr);
                    if (editor != nullptr)
                    {
                        // Retrieve the entrypoint's start line index according to the "list-kernels" results.
                        int actual_start_line = lineNumberIter->second.first;

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
    }
}

bool RgBuildViewBinary::LoadEntrypointLineNumbers()
{
    // Clear stale line correlation state from any previous build cycle.
    // This prevents accumulation of old source paths when include directories change across re-analyses.
    entrypoint_line_numbers_.clear();
    entrypoint_to_src_files_map_.clear();
    binary_to_src_files_map_.clear();
    user_closed_source_files_.clear();

    // Clear previous source editors and tab widget content.
    if (tab_widget_ != nullptr)
    {
        tab_widget_->ClearAll();
    }

    // Detach the search target before deleting editors to prevent
    // ShaderSourceCodeViewerSearcher::ResetSearch from accessing a deleted widget.
    if (source_searcher_ != nullptr)
    {
        source_searcher_->SetTargetEditor(nullptr);
    }

    for (auto& editor : source_code_editors_)
    {
        if (editor.second != nullptr)
        {
            editor.second->deleteLater();
        }
    }
    source_code_editors_.clear();
    current_code_editor_ = nullptr;

    // Find the first target ASIC that appears to have valid compilation results.
    auto asic_outputs_iter = build_outputs_.begin();
    for (; asic_outputs_iter != build_outputs_.end(); ++asic_outputs_iter)
    {
        if (asic_outputs_iter->second != nullptr && typeid(*asic_outputs_iter->second) == typeid(RgCliBuildOutputOpencl))
        {
            std::shared_ptr<RgCliBuildOutputOpencl> build_output = std::static_pointer_cast<RgCliBuildOutputOpencl>(asic_outputs_iter->second);
            if (build_output != nullptr)
            {
                for (const auto& binary_file_name : build_output->project_binaries)
                {
                    const auto& file_output = build_output->per_file_output[binary_file_name];
                    if (!file_output.outputs.empty())
                    {
                        for (const auto& entry_output : file_output.outputs)
                        {
                            for (const auto& output_item : entry_output.outputs)
                            {
                                if (output_item.file_type == RgCliOutputFileType::kIsaDisassemblyCsv)
                                {
                                    std::map<std::string, std::pair<uint32_t, uint32_t>> src_file_line_numbers;
                                    ReadCsvFileForLineRanges(output_item.file_path, src_file_line_numbers);

                                    if (!src_file_line_numbers.empty())
                                    {
                                        // Mark this binary as correlated since the ISA CSV contains line correlation data.
                                        auto first_file = project_->clones[clone_index_]->binary_files.begin();
                                        auto last_file  = project_->clones[clone_index_]->binary_files.end();
                                        RgSourceFilePathSearcher path_searcher(binary_file_name);
                                        auto file_iter = std::find_if(first_file, last_file, path_searcher);
                                        if (file_iter != last_file)
                                        {
                                            file_iter->is_correlated = true;
                                        }
                                    }

                                    for (const auto& [src_file, range] : src_file_line_numbers)
                                    {
                                        // For fused shaders, both stage CSVs reference all source files.
                                        // Only associate a source file with an entrypoint when the file's
                                        // stage-specific extension matches the entrypoint type, so each
                                        // source file maps to exactly one entrypoint.
                                        std::string src_ext;
                                        RgUtils::ExtractFileExtension(src_file, src_ext);
                                        const std::string expected_ext = GetExtensionForEntrypointType(entry_output.entrypoint_name);
                                        if (!expected_ext.empty() && IsStageSpecificExtension(src_ext) && src_ext != expected_ext)
                                        {
                                            continue;
                                        }

                                        entrypoint_line_numbers_[src_file][entry_output.entrypoint_name] = range;
                                        entrypoint_to_src_files_map_[RgUtils::GenerateEntrypointKey(binary_file_name, "", entry_output.entrypoint_name)].insert(
                                            src_file);
                                        binary_to_src_files_map_[binary_file_name].insert(src_file);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return !entrypoint_line_numbers_.empty();
}

bool RgBuildViewBinary::CreateSourceCodeWidgets()
{
    bool ret = true;

    // Phase 1: Collect all unique source files, separating found from missing.
    std::unordered_set<std::string> src_files_set;
    std::vector<std::string>        found_files;
    QStringList                     missing_files;

    for (const auto& [entry_point_key, src_files] : entrypoint_to_src_files_map_)
    {
        for (const auto& src_file : src_files)
        {
            if (src_files_set.find(src_file) == src_files_set.end())
            {
                src_files_set.insert(src_file);

                if (RgUtils::IsFileExists(src_file))
                {
                    found_files.push_back(src_file);
                }
                else
                {
                    missing_files.append(QString::fromStdString(src_file));
                }
            }
        }
    }

    // Phase 2: Create editor widgets for all found source files.
    for (const auto& src_file : found_files)
    {
        if (tab_widget_ != nullptr)
        {
            RgSourceCodeEditor* editor = GetEditorForFilepath(src_file, ShaderSourceLanguage::kOpenCL);
            assert(editor != nullptr);
            if (editor != nullptr)
            {
                tab_widget_->AddSourceFile(src_file, editor);
                SetSourceCodeText(src_file, false);
            }
        }
    }

    // Phase 3b: Hide tabs for non-application source files (headers).
    // They are fully loaded, just not visible until the user
    // navigates to them via ISA line correlation.
    for (const auto& src_file : found_files)
    {
        if (IsNonApplicationSourceTab(src_file))
        {
            tab_widget_->HideEditor(src_file);
        }
    }

    // Phase 4: Create read-only placeholder editors for missing source files.
    for (const auto& missing_file : missing_files)
    {
        if (tab_widget_ != nullptr)
        {
            std::string         missing_path = missing_file.toStdString();
            RgSourceCodeEditor* editor       = GetEditorForFilepath(missing_path, ShaderSourceLanguage::kOpenCL);
            if (editor != nullptr)
            {
                editor->setReadOnly(true);
                editor->SetIsPlaceholder(true);
                editor->setMouseTracking(true);

                // Detach the syntax highlighter so keywords like "for" aren't colored.
                QSyntaxHighlighter* highlighter = editor->findChild<QSyntaxHighlighter*>();
                if (highlighter != nullptr)
                {
                    highlighter->setDocument(nullptr);
                }

                // Build the placeholder text with the list of missing filenames.
                QString placeholder_text = QString(kStrMissingSourcePlaceholderText) + "\n";
                for (const auto& mf : missing_files)
                {
                    placeholder_text += "    " + QFileInfo(mf).fileName() + "\n";
                }
                placeholder_text += "\n" + QString(kStrMissingSourceClickHereText);

                editor->setPlainText(placeholder_text);

                // Style the "Click here" line as a hyperlink (blue, underlined).
                QTextCursor format_cursor = editor->document()->find(QString(kStrMissingSourceClickHereText));
                if (!format_cursor.isNull())
                {
                    QTextCharFormat link_format;
                    link_format.setForeground(QColor(0, 102, 204));
                    link_format.setFontUnderline(true);
                    format_cursor.mergeCharFormat(link_format);
                }

                editor->document()->setModified(false);

                tab_widget_->AddSourceFile(missing_path, editor);

                // Connect the "Click here" signal to show the search directories dialog.
                // Resetting the prompted flag lets PreAnalysisSourceDiscovery() show
                // the dialog again and derive substitute_paths from any new directories.
                connect(editor, &RgSourceCodeEditor::OpenSourceSearchDirectoriesRequested, this, [this]()
                {
                    already_prompted_for_source_dirs_ = false;
                    if (PreAnalysisSourceDiscovery(true))
                    {
                        ResetCurrentProjectBinaries();
                        BuildCurrentProject();
                    }
                });
            }
        }
    }

    if (!missing_files.isEmpty())
    {
        ret = false;
    }

    return ret;
}

bool RgBuildViewBinary::IsSourceFileInProject(const std::string& source_file_path) const
{
    Q_UNUSED(source_file_path);

    return false;
}

bool RgBuildViewBinary::IsCodeObjFileInProject(const std::string& bin_file_path) const
{
    bool res = false;

    std::vector<std::string> file_paths;
    RgConfigManager::Instance().GetProjectBinaryFilePaths(project_, clone_index_, file_paths);

    for (auto iter = file_paths.begin(); iter != file_paths.end(); ++iter)
    {
        if (RgaSharedUtils::ComparePaths(bin_file_path, *iter))
        {
            res = true;
            break;
        }
    }

    return res;
}

void RgBuildViewBinary::HandleExternalFileModification(const QFileInfo& file_info)
{
    std::string modified_file_path = file_info.filePath().toStdString();
    bool        is_file_exists     = RgUtils::IsFileExists(modified_file_path);
    if (is_file_exists)
    {
        QString message_text = QString(modified_file_path.c_str()) + "\n\n" + kStrReloadFileDialogTextBinary;

        // Show waring message box to warn the user.
        QMessageBox::warning(this, kStrReloadFileDialogTitle, message_text);

        // Remove file from current project.
        RemoveInputFile(modified_file_path);

        // Re-add the file to trigger a reload.
        std::vector<std::string> modified_binary_files = {modified_file_path};
        AddExistingCodeObjFileToProject(modified_binary_files);
    }
    else
    {
        QString message_text = QString(modified_file_path.c_str()) + "\n\n" + kStrRemoveFileDialogTextBinary;

        // Show waring message box to warn the user.
        QMessageBox::warning(this, kStrRemoveFileDialogTitle, message_text);

        RemoveInputFile(modified_file_path);
    }

    // Once the user has responded to a file modification dialog,
    // reset the pending modification status for this file.
    pending_file_modifications_.erase(modified_file_path);
}

void RgBuildViewBinary::ToggleDisassemblyViewKernelLabelVisiblity(RgMenuFileItemOpencl* file_item, const std::string& selected_entrypoint_name)
{
    if (file_item != nullptr)
    {
        std::string extremely_long_name;
        bool        is_visible = file_item->GetSelectedEntrypointExtremelyLongName(selected_entrypoint_name, extremely_long_name);
        emit        SelectedExtremelyLongKernelNameChanged(is_visible, extremely_long_name);
    }
}

bool RgBuildViewBinary::SwitchToTabWidget(const std::string& filename)
{
    bool ret = false;

    // Verify if the user is allowed to switch to source editing mode.
    bool is_switching_allowed = CanSwitchEditMode();
    if (is_switching_allowed)
    {
        // Switch to the new editor.
        SetViewContentsWidget(tab_widget_);

        // TODO AMK3
        //if (current_code_editor_ != nullptr)
        //{
        //    bool old_editor_is_modified = current_code_editor_->document()->isModified();
        //    bool new_editor_is_modified = editor->document()->isModified();

        //    // Check if the new editor has a different modification state then the old one
        //    if (old_editor_is_modified != new_editor_is_modified)
        //    {
        //        emit CurrentEditorModificationStateChanged(new_editor_is_modified);
        //    }
        //}

        // Update the editor context.
        UpdateSourceEditorSearchContext();

        //// Update the title bar text.
        //const std::string& title_bar_text = editor->GetTitleBarText();
        //if (!title_bar_text.empty())
        //{
        //    source_editor_titlebar_->ShowMessage(title_bar_text);
        //}
        //else
        //{
        //    source_editor_titlebar_->SetTitlebarContentsVisibility(false);
        //}

        // The tab widget isn't empty, and will switch to displaying source code.
        SwitchEditMode(EditMode::kSourceCodeTabs);

        // Check if the editor file has been modified externally.
        CheckExternalFileModification(filename);

        ret = true;
    }

    return ret;
}

void RgBuildViewBinary::SetTargetGpuLabel()
{
    if (!is_build_in_progress_ && disassembly_view_ != nullptr)
    {
        auto disassembly_view_binary_ = static_cast<RgIsaDisassemblyViewBinary*>(disassembly_view_);
        if (disassembly_view_binary_ != nullptr && tab_widget_ != nullptr)
        {
            assert(file_menu_ != nullptr);
            if (file_menu_ != nullptr)
            {
                RgMenuBinary* menu_binary = static_cast<RgMenuBinary*>(file_menu_);
                assert(menu_binary != nullptr);
                if (menu_binary != nullptr)
                {
                    // Search the list of binary file info for the one that matches the given editor.
                    if (project_ != nullptr && clone_index_ >= 0 && clone_index_ < project_->clones.size() && project_->clones[clone_index_] != nullptr)
                    {
                        auto                     first_file = project_->clones[clone_index_]->binary_files.begin();
                        auto                     last_file  = project_->clones[clone_index_]->binary_files.end();
                        RgSourceFilePathSearcher path_searcher(menu_binary->GetSelectedFilePath());
                        auto                     file_iter = std::find_if(first_file, last_file, path_searcher);
                        assert(file_iter != last_file);
                        if (file_iter != last_file)
                        {
                            disassembly_view_binary_->SetTargetGpuLabel(file_iter->target_gpu);
                        }
                    }            
                }
            }            
        }
    }
}

void RgBuildViewBinary::SwitchToFirstRemainingFile()
{
    // If there are code editors remaining, switch to the first remaining item.
    if (!source_code_editors_.empty())
    {
        RgMenu* menu = GetMenu();
        assert(menu != nullptr);
        if (menu != nullptr)
        {
            menu->SelectLastRemainingItem();
        }

        // Switch to viewing the RgSourceCodeEditor for the newly selected item.
        SwitchEditMode(EditMode::kSourceCodeTabs);
    }
    else
    {
        // When the last file has been removed, the editor is in the empty state.
        SwitchEditMode(EditMode::kEmpty);
    }
}

void RgBuildViewBinary::ClearEditors()
{
    std::vector<std::string> file_paths;
    for (auto it = binary_to_src_files_map_.begin(); it != binary_to_src_files_map_.end(); ++it)
    {
        file_paths.push_back(it->first);
    }

    for (const std::string& full_file_path : file_paths)
    {
        RemoveEditor(full_file_path);
    }
}

void RgBuildViewBinary::SetAPISpecificBorderColor()
{
    HandleSetFrameBorderPurple();
}

bool RgBuildViewBinary::GetEntrypointNameForLineNumber(const std::string& binary_file_path,
                                                       const std::string& src_file_path,
                                                       int                line_number,
                                                       std::string&       entry_name) const
{
    bool found = false;

    // Check if "line_number" is within some kernel code.
    const auto& file_src_line_data = entrypoint_line_numbers_.find(src_file_path);
    if (file_src_line_data != entrypoint_line_numbers_.end())
    {
        // Get the currently selected entrypoint so we can prefer it when multiple
        // entrypoints share the same source line range (e.g. shaders compiled from
        // identical source).
        std::string current_selection;
        if (file_menu_ != nullptr)
        {
            RgMenuFileItem* file_item = file_menu_->GetFileItemFromPath(binary_file_path);
            if (file_item != nullptr)
            {
                RgMenuFileItemOpencl* file_item_opencl = static_cast<RgMenuFileItemOpencl*>(file_item);
                if (file_item_opencl != nullptr)
                {
                    file_item_opencl->GetSelectedEntrypointName(current_selection);
                }
            }
        }

        for (const auto& entry_src_line_data : file_src_line_data->second)
        {
            const std::pair<int, int> startAndEndLines = entry_src_line_data.second;
            if (line_number >= startAndEndLines.first && line_number <= startAndEndLines.second)
            {
                // Keep the first match unless the currently selected entrypoint also covers this line.
                if (!found)
                {
                    entry_name = entry_src_line_data.first;
                    found      = true;
                }

                // If this match is the currently selected entrypoint, use it immediately.
                // Otherwise keep looking in case the current selection also covers this line.
                if (entry_src_line_data.first == current_selection)
                {
                    entry_name = entry_src_line_data.first;
                    break;
                }
            }
        }

        // Fall back to the first entrypoint for this source file.
        if (!found && !file_src_line_data->second.empty())
        {
            entry_name = file_src_line_data->second.begin()->first;
            found = true;
        }
    }

    // Fall back to selecting the current entry point in the selected file item.
    if (!found)
    {
        assert(file_menu_ != nullptr);
        if (file_menu_ != nullptr)
        {
            RgMenuFileItem* file_item = file_menu_->GetFileItemFromPath(binary_file_path);
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

std::string RgBuildViewBinary::GetSelectedBinaryFilePath() const
{
    return (file_menu_ != nullptr) ? file_menu_->GetSelectedFilePath() : std::string();
}

bool RgBuildViewBinary::IsLineCorrelationEnabled(RgSourceCodeEditor* source_editor)
{
    Q_UNUSED(source_editor);

    return IsLineCorrelationSupported();
}

bool RgBuildViewBinary::IsLineCorrelationSupported() const
{
    // By default, correlation is enabled; only disable correlation when the selected binary is known to lack line info.
    bool is_correlation_supported = true;

    const std::string binary_file_path = GetSelectedBinaryFilePath();
    if (!binary_file_path.empty() && project_ != nullptr && clone_index_ >= 0 && clone_index_ < project_->clones.size() &&
        project_->clones[clone_index_] != nullptr)
    {
        const auto&              binary_files = project_->clones[clone_index_]->binary_files;
        RgSourceFilePathSearcher path_searcher(binary_file_path);
        auto                     file_iter = std::find_if(binary_files.cbegin(), binary_files.cend(), path_searcher);
        if (file_iter != binary_files.cend())
        {
            is_correlation_supported = file_iter->is_correlated;
        }
    }

    return is_correlation_supported;
}

void RgBuildViewBinary::ResetCurrentProjectBinaries()
{
    // Reset binary files status.
    RgConfigManager::Instance().ResetProjectBinaryFileStatus(project_, clone_index_);

    // Save project file changes pre build.
    SaveProjectConfigFile();
}

bool RgBuildViewBinary::RequestRemoveAllFiles()
{
    bool is_save_accepted = ShowSaveDialog(RgBuildView::RgFilesToSave::kBuildSettings, false);
    if (is_save_accepted)
    {
        RgMenu* menu = GetMenu();
        assert(menu != nullptr);
        if (menu != nullptr)
        {
            // Remove all file menu items.
            auto it = binary_to_src_files_map_.begin();
            while (it != binary_to_src_files_map_.end())
            {
                std::string full_path = it->first;
                menu->RemoveItem(full_path);
                RemoveEditor(full_path);

                // Keep getting first item until all are removed.
                it = binary_to_src_files_map_.begin();
            }
        }
    }

    return is_save_accepted;
}

void RgBuildViewBinary::RenameFile(const std::string& old_file_path, const std::string& new_file_path)
{
    auto it = binary_to_src_files_map_.find(old_file_path);
    if (it != binary_to_src_files_map_.end())
    {
        const auto src_files = it->second;
        binary_to_src_files_map_.erase(it);
        binary_to_src_files_map_[new_file_path] = src_files;
    }

    for (auto entry_itr = entrypoint_to_src_files_map_.begin(); entry_itr != entrypoint_to_src_files_map_.end();)
    {
        std::string binary_file, gpu, entry_name;
        RgUtils::DecodeEntrypointKey(entry_itr->first, binary_file, gpu, entry_name);

        if (binary_file == old_file_path)
        {
            auto              src_files = std::move(entry_itr->second);
            const std::string new_key   = RgUtils::GenerateEntrypointKey(new_file_path, gpu, entry_name);
            auto              dest_itr  = entrypoint_to_src_files_map_.find(new_key);
            if (dest_itr != entrypoint_to_src_files_map_.end())
            {
                dest_itr->second.insert(src_files.begin(), src_files.end());
            }
            else
            {
                entrypoint_to_src_files_map_.emplace(new_key, std::move(src_files));
            }
            entry_itr = entrypoint_to_src_files_map_.erase(entry_itr);
        }
        else
        {
            ++entry_itr;
        }
    }

    // Update the project's source file list with the new filepath.
    RgConfigManager::Instance().UpdateBinaryFilepath(old_file_path, new_file_path, project_, clone_index_);

    // Save the updated project file.
    RgConfigManager::Instance().SaveProjectFile(project_);
}

void RgBuildViewBinary::GetUnsavedSourceFiles(QStringList& unsaved_source_files)
{
    Q_UNUSED(unsaved_source_files);
}

void RgBuildViewBinary::UpdateApplicationNotificationMessage()
{
    // Add Binary analysis application notification message here, when needed.
}

bool RgBuildViewBinary::ShowRemoveFileConfirmation(const std::string& message_string, const std::string& full_path)
{
    bool is_removed = false;

    if (!full_path.empty())
    {
        // Ask the user if they're sure they want to remove the file.
        is_removed = RgUtils::ShowConfirmationMessageBox(kStrMenuBarConfirmRemoveFileDialogTitle, message_string.c_str(), this);
    }

    return is_removed;
}

void RgBuildViewBinary::HandleModeSpecificEditMode(EditMode new_mode)
{
    if (new_mode == EditMode::kSourceCodeTabs)
    {
        // Enable maximizing the source editor/build settings container.
        assert(source_view_container_ != nullptr);
        if (source_view_container_ != nullptr)
        {
            source_view_container_->SetIsMaximizable(true);
        }

        // Hide the build settings, and show the code tab widget.
        if (tab_widget_ != nullptr)
        {
            // Set the tab widget instance in the view.
            SetViewContentsWidget(tab_widget_);
        }

        // Set the appropriate boolean in RgViewManager
        // to facilitate focusing the correct widget.
        view_manager_->SetIsSourceViewCurrent(true);
    }
}

void RgBuildViewBinary::RemoveEditor(const std::string& binary_filename, bool switch_to_next_file)
{
    auto it = binary_to_src_files_map_.find(binary_filename);
    if (it != binary_to_src_files_map_.end())
    {
        bool is_last_remaining_file = (binary_to_src_files_map_.size() == 1);
        binary_to_src_files_map_.erase(it);

        if (is_last_remaining_file)
        {
            if (tab_widget_ != nullptr)
            {
                tab_widget_->ClearAll();
            }

            // Detach the search target before deleting editors to prevent
            // ShaderSourceCodeViewerSearcher::ResetSearch from accessing a deleted widget.
            if (source_searcher_ != nullptr)
            {
                source_searcher_->SetTargetEditor(nullptr);
            }

            for (auto& editor : source_code_editors_)
            {
                if (editor.second != nullptr)
                {
                    editor.second->deleteLater();
                }
            }

            entrypoint_line_numbers_.clear();
            source_code_editors_.clear();
            current_code_editor_ = nullptr;
        }
    }

    for (auto entry_itr = entrypoint_to_src_files_map_.begin(); entry_itr != entrypoint_to_src_files_map_.end();)
    {
        std::string binary_file, gpu, entry_name;
        RgUtils::DecodeEntrypointKey(entry_itr->first, binary_file, gpu, entry_name);
        if (binary_file == binary_filename)
        {
            // Remove this entrypoint's line number data from each of its source files.
            for (const auto& src_file : entry_itr->second)
            {
                auto line_nums_itr = entrypoint_line_numbers_.find(src_file);
                if (line_nums_itr != entrypoint_line_numbers_.end())
                {
                    line_nums_itr->second.erase(entry_name);
                    if (line_nums_itr->second.empty())
                    {
                        entrypoint_line_numbers_.erase(line_nums_itr);
                    }
                }
            }

            entry_itr = entrypoint_to_src_files_map_.erase(entry_itr);
        }
        else
        {
            ++entry_itr;
        }
    }

    binary_file_modified_time_map_.erase(binary_filename);

    // Remove the editor from the map, and hide it from the interface.
    QWidget*                title_bar             = source_view_container_->GetTitleBar();
    RgSourceEditorTitlebar* source_view_title_bar = qobject_cast<RgSourceEditorTitlebar*>(title_bar);
    if (source_view_title_bar != nullptr)
    {
        source_view_title_bar->SetTitlebarContentsVisibility(false);
    }

    if (switch_to_next_file)
    {
        SwitchToFirstRemainingFile();
    }
}

void RgBuildViewBinary::CheckExternalFileModification(const std::string& filename)
{
    // If there are no active code editors, no files can be modified.
    auto it = binary_file_modified_time_map_.find(filename);
    if (it != binary_file_modified_time_map_.end())
    {
        // Get file modification time from the last time the file was saved in RGA.
        QDateTime expected_last_modified = it->second;

        // Get file info for the editor file.
        QFileInfo   file_info(filename.c_str());

        // If the modification time is not the same as remembered, the file has been changed externally.
        if (file_info.lastModified() != expected_last_modified)
        {
            // Only handle the modification if it hasn't been handled already (i.e., no dialog shown to the user).
            if (pending_file_modifications_.find(filename) == pending_file_modifications_.end())
            {
                // Mark this file as having pending modification
                pending_file_modifications_[filename] = true;

                HandleExternalFileModification(file_info);
            }
        }
    }
}

void RgBuildViewBinary::RemoveFileFromMetadata(const std::string& full_path)
{
    std::string project_directory;
    if (RgUtils::ExtractFileDirectory(project_->project_file_full_path, project_directory))
    {
        // Generate a clone name string based on the current clone index.
        std::string output_folder_path;

        bool is_ok = RgUtils::AppendFolderToPath(project_directory, kStrOutputFolderName, output_folder_path);
        assert(is_ok);
        if (is_ok)
        {
            std::string cloneNameString = RgUtils::GenerateCloneName(clone_index_);
            is_ok                       = RgUtils::AppendFolderToPath(output_folder_path, cloneNameString, output_folder_path);
            assert(is_ok);
            if (is_ok)
            {
                std::stringstream metadataFilenameStream;
                metadataFilenameStream << kStrSessionMetadataFilename;

                std::string full_metadata_file_path;
                is_ok = RgUtils::AppendFileNameToPath(output_folder_path, metadataFilenameStream.str(), full_metadata_file_path);
                assert(is_ok);
                if (is_ok)
                {
                    RgXMLSessionConfig::RemoveBinaryFileFromMetadata(full_metadata_file_path, full_path);
                }
            }
        }
    }
}

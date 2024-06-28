// C++.
#include <cassert>
#include <sstream>
#include <thread>

// Qt.
#include <QBoxLayout>
#include <QDateTime>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QList>
#include <QMessageBox>
#include <QScrollBar>
#include <QSizePolicy>
#include <QSplitter>
#include <QString>
#include <QVBoxLayout>
#include <QWidget>
#include <QProcess>
#include <QDesktopServices>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_app_state.h"
#include "radeon_gpu_analyzer_gui/qt/rg_build_view.h"
#include "radeon_gpu_analyzer_gui/qt/rg_build_settings_view.h"
#include "radeon_gpu_analyzer_gui/qt/rg_build_settings_view_vulkan.h"
#include "radeon_gpu_analyzer_gui/qt/rg_build_settings_widget.h"
#include "radeon_gpu_analyzer_gui/qt/rg_cli_output_view.h"
#include "radeon_gpu_analyzer_gui/qt/rg_menu.h"
#include "radeon_gpu_analyzer_gui/qt/rg_menu_build_settings_item.h"
#include "radeon_gpu_analyzer_gui/qt/rg_menu_file_item.h"
#include "radeon_gpu_analyzer_gui/qt/rg_menu_titlebar.h"
#include "radeon_gpu_analyzer_gui/qt/rg_find_text_widget.h"
#include "radeon_gpu_analyzer_gui/qt/rg_isa_disassembly_view_opencl.h"
#include "radeon_gpu_analyzer_gui/qt/rg_isa_disassembly_view_vulkan.h"
#include "radeon_gpu_analyzer_gui/qt/rg_main_window.h"
#include "radeon_gpu_analyzer_gui/qt/rg_maximize_splitter.h"
#include "radeon_gpu_analyzer_gui/qt/rg_rename_project_dialog.h"
#include "radeon_gpu_analyzer_gui/qt/rg_scroll_area.h"
#include "radeon_gpu_analyzer_gui/qt/rg_source_code_editor.h"
#include "radeon_gpu_analyzer_gui/qt/rg_settings_buttons_view.h"
#include "radeon_gpu_analyzer_gui/qt/rg_source_editor_titlebar.h"
#include "radeon_gpu_analyzer_gui/qt/rg_unsaved_items_dialog.h"
#include "radeon_gpu_analyzer_gui/qt/rg_view_container.h"
#include "radeon_gpu_analyzer_gui/qt/rg_view_manager.h"
#include "radeon_gpu_analyzer_gui/rg_config_manager.h"
#include "radeon_gpu_analyzer_gui/rg_cli_launcher.h"
#include "radeon_gpu_analyzer_gui/rg_data_types_vulkan.h"
#include "radeon_gpu_analyzer_gui/rg_data_types_opencl.h"
#include "radeon_gpu_analyzer_gui/rg_definitions.h"
#include "radeon_gpu_analyzer_gui/rg_factory.h"
#include "radeon_gpu_analyzer_gui/rg_source_editor_searcher.h"
#include "radeon_gpu_analyzer_gui/rg_string_constants.h"
#include "radeon_gpu_analyzer_gui/rg_utils.h"
#include "radeon_gpu_analyzer_gui/rg_xml_session_config.h"

static const int kStrFileMenuViewContainerWidth = 200;
static const int kFindTextWidgetHorizontalMargin = 10;
static const int kFindTextWidgetVerticalMargin = 10;

RgBuildView::RgBuildView(RgProjectAPI api, QWidget* parent) :
    QWidget(parent),
    clone_index_(0),
    parent_(parent)
{
    // Setup the UI.
    ui_.setupUi(this);

    // Create the factory used to create API specific objects within the RgBuildView.
    factory_ = RgFactory::CreateFactory(api);

    // Create the find widget.
    CreateFindWidget();
}

bool RgBuildView::CheckSourcesModifiedSinceLastBuild(RgSourceCodeEditor* code_editor)
{
    bool is_modified_after_build = false;

    assert(code_editor != nullptr);
    if (code_editor != nullptr)
    {
        auto file_modification_time_map_iter = file_modified_time_map_.find(code_editor);
        if (file_modification_time_map_iter != file_modified_time_map_.end())
        {
            // If the source file was modified after the last build, line correlation data has become invalid.
            const QDateTime& last_saved_time = file_modification_time_map_iter->second;
            if (last_saved_time > last_successful_build_time_)
            {
                is_modified_after_build = true;
            }
        }
    }

    return is_modified_after_build;
}

void RgBuildView::ClearBuildView()
{
    RgMenu* menu = GetMenu();
    if (menu != nullptr)
    {
        menu->ClearFiles();
    }

    // Clean up source editor instances.
    ClearEditors();

    // Clear the output window.
    if (cli_output_window_ != nullptr)
    {
        cli_output_window_->ClearText();
    }
}

void RgBuildView::ClearEditors()
{
    std::vector<std::string> file_paths;
    for (auto editor_iter = source_code_editors_.begin(); editor_iter != source_code_editors_.end(); ++editor_iter)
    {
        file_paths.push_back(editor_iter->first);
    }

    for (const std::string& full_file_path : file_paths)
    {
        RemoveEditor(full_file_path);
    }
}

void RgBuildView::ConnectFileSignals()
{
    RgMenu* menu = GetMenu();
    assert(menu != nullptr);
    if (menu != nullptr)
    {
        // Connect the file menu to the build view, so that the view knows when the current file is switched.
        bool is_connected = connect(menu, &RgMenu::SelectedFileChanged, this, &RgBuildView::HandleSelectedFileChanged);
        assert(is_connected);

        // Connect the file menu's MenuItemCloseButtonClicked signal with the menu item closed handler.
        is_connected = connect(menu, &RgMenu::MenuItemCloseButtonClicked, this, &RgBuildView::HandleMenuItemCloseButtonClicked);
        assert(is_connected);

        // Connect the file menu default item's "Add new file" button.
        is_connected = connect(menu, &RgMenu::CreateFileButtonClicked, this, &RgBuildView::CreateFileButtonClicked);
        assert(is_connected);

        // Connect the file menu default item's "Open existing file" button.
        is_connected = connect(menu, &RgMenu::OpenFileButtonClicked, this, &RgBuildView::OpenFileButtonClicked);
        assert(is_connected);

        // Connect the file menu's rename signal.
        is_connected = connect(menu, &RgMenu::FileRenamed, this, &RgBuildView::HandleFileRenamed);
        assert(is_connected);

        // Connect the file menu's project renamed signal.
        is_connected = connect(file_menu_titlebar_, &RgMenuTitlebar::TitleChanged, this, &RgBuildView::HandleProjectRenamed);
        assert(is_connected);

        // Connect the file menu's item count change signal.
        is_connected = connect(menu, &RgMenu::FileMenuItemCountChanged, this, &RgBuildView::ProjectFileCountChanged);
        assert(is_connected);

        // Connect the file menu's file build settings button click signal to the RgBuildView's handler.
        is_connected = connect(menu, &RgMenu::BuildSettingsButtonClicked, this, &RgBuildView::HandleFindWidgetVisibilityToggled);
        assert(is_connected);

        // Connect the file menu's next focus change signal.
        is_connected = connect(menu, &RgMenu::FocusNextView, this, &RgBuildView::HandleFocusNextView);
        assert(is_connected);

        // Connect the error location reported by CLI output window to the RgBuildView's handlers.
        is_connected = connect(cli_output_window_, &RgCliOutputView::SwitchToFile, menu, &RgMenu::HandleSwitchToFile);
        assert(is_connected);

        // Connect the focus file menu signal to the build view's handlers.
        is_connected = connect(cli_output_window_, &RgCliOutputView::FocusNextView, this, &RgBuildView::HandleFocusNextView);
        assert(is_connected);

        // Connect the focus output window signal to the build view's handlers.
        is_connected = connect(cli_output_window_, &RgCliOutputView::FocusOutputWindow, this, &RgBuildView::HandleSetOutputWindowFocus);
        assert(is_connected);

        // Connect the source editor's scroll-to-line signal.
        is_connected = connect(menu, &RgMenu::ScrollCodeEditorToLine, this, &RgBuildView::HandleScrollCodeEditorToLine);
        assert(is_connected);

        // Connect the "Build Settings" button in the file menu.
        RgMenuBuildSettingsItem* build_settings_item = menu->GetBuildSettingsItem();
        const QPushButton* pBuildSettingsFileMenuButton = build_settings_item->GetBuildSettingsButton();
        is_connected = connect(pBuildSettingsFileMenuButton, &QPushButton::clicked, this, &RgBuildView::HandleBuildSettingsMenuButtonClicked);
        assert(is_connected);

        // Connect to the file menu container's mouse click event.
        is_connected = connect(file_menu_view_container_, &RgViewContainer::ViewContainerMouseClickEventSignal, this, &RgBuildView::HandleSetFrameBorderBlack);
        assert(is_connected);

        // Connect menu signals.
        is_connected = ConnectMenuSignals();
        assert(is_connected);
    }
}

void RgBuildView::HandleFocusPrevView()
{
    assert(view_manager_ != nullptr);
    if (view_manager_ != nullptr)
    {
        view_manager_->FocusPrevView();
    }
}

void RgBuildView::HandleSetOutputWindowFocus()
{
    assert(view_manager_ != nullptr);
    if (view_manager_ != nullptr)
    {
        view_manager_->SetOutputWindowFocus();
    }
}

void RgBuildView::HandleSetDisassemblyViewFocus()
{
    assert(view_manager_ != nullptr);
    if (view_manager_ != nullptr)
    {
        view_manager_->SetDisassemblyViewFocus();
    }
}

void RgBuildView::ConnectBuildSettingsSignals()
{
    // "Save" button.
    bool is_connected = connect(settings_buttons_view_, &RgSettingsButtonsView::SaveSettingsButtonClickedSignal, this, &RgBuildView::HandleSaveSettingsButtonClicked);
    assert(is_connected);

    // "Restore defaults" button.
    is_connected = connect(settings_buttons_view_, &RgSettingsButtonsView::RestoreDefaultSettingsButtonClickedSignal, this, &RgBuildView::HandleRestoreDefaultsSettingsClicked);
    assert(is_connected);

    // Connect the save settings view clicked signal.
    is_connected = connect(settings_buttons_view_, &RgSettingsButtonsView::SettingsButtonsViewClickedSignal, this, &RgBuildView::SetAPISpecificBorderColor);
    assert(is_connected);
}

void RgBuildView::ConnectFindSignals()
{
    // Connect the find widget's close toggle handler.
    [[maybe_unused]] bool is_connected = connect(find_widget_, &RgFindTextWidget::CloseWidgetSignal, this, &RgBuildView::HandleFindWidgetVisibilityToggled);
    assert(is_connected);
}

void RgBuildView::ConnectBuildViewSignals()
{
    bool is_connected = connect(this, &RgBuildView::LineCorrelationEnabledStateChanged, this, &RgBuildView::HandleIsLineCorrelationEnabled);
    assert(is_connected);

    is_connected = connect(this, &RgBuildView::ProjectBuildSuccess, this, &RgBuildView::HandleProjectBuildSuccess);
    assert(is_connected);

    is_connected = connect(qGuiApp, &QGuiApplication::applicationStateChanged, this, &RgBuildView::HandleApplicationStateChanged);
    assert(is_connected);

    is_connected = connect(view_manager_, &RgViewManager::BuildSettingsWidgetFocusInSignal, this, &RgBuildView::SetAPISpecificBorderColor);
    assert(is_connected);

    is_connected = connect(view_manager_, &RgViewManager::BuildSettingsWidgetFocusInSignal, this, &RgBuildView::SetDefaultFocusWidget);
    assert(is_connected);

    is_connected = connect(view_manager_, &RgViewManager::BuildSettingsWidgetFocusOutSignal, this, &RgBuildView::HandleSetFrameBorderBlack);
    assert(is_connected);
}

void RgBuildView::ConnectOutputWindowSignals()
{
    bool is_connected = connect(this, &RgBuildView::ProjectBuildStarted, cli_output_window_, &RgCliOutputView::HandleBuildStarted);
    assert(is_connected);

    is_connected = connect(this, &RgBuildView::ProjectBuildFailure, cli_output_window_, &RgCliOutputView::HandleBuildEnded);
    assert(is_connected);

    is_connected = connect(this, &RgBuildView::ProjectBuildSuccess, cli_output_window_, &RgCliOutputView::HandleBuildEnded);
    assert(is_connected);

    is_connected = connect(this, &RgBuildView::ProjectBuildCanceled, cli_output_window_, &RgCliOutputView::HandleBuildEnded);
    assert(is_connected);

    assert(output_splitter_ != nullptr);
    if (output_splitter_ != nullptr)
    {
        // Connect the handler invoked when the output window splitter is resized.
        is_connected = connect(output_splitter_, &RgMaximizeSplitter::splitterMoved, this, &RgBuildView::HandleSplitterMoved);
        assert(is_connected);
    }
}

bool RgBuildView::ConnectDisassemblyViewSignals()
{
    bool is_connected = false;

    assert(disassembly_view_ != nullptr);
    if (disassembly_view_ != nullptr)
    {
        // Connect the handler invoked when the highlighted correlation line in the input source file should be updated.
        is_connected = connect(disassembly_view_, &RgIsaDisassemblyView::InputSourceHighlightedLineChanged,
            this, &RgBuildView::HandleHighlightedCorrelationLineUpdated);
        assert(is_connected);

        // Connect the RgIsaDisassemblyView's table resized handler.
        is_connected = connect(disassembly_view_, &RgIsaDisassemblyView::DisassemblyTableWidthResizeRequested,
            this, &RgBuildView::HandleDisassemblyTableWidthResizeRequested);
        assert(is_connected);

        // Connect the RgIsaDisassemblyView's Target GPU changed handler.
        is_connected = connect(disassembly_view_, &RgIsaDisassemblyView::SelectedTargetGpuChanged,
            this, &RgBuildView::HandleSelectedTargetGpuChanged);
        assert(is_connected);

        // Connect the RgIsaDisassemblyView's clicked handler.
        is_connected = connect(disassembly_view_, &RgIsaDisassemblyView::DisassemblyViewClicked,
            this, &RgBuildView::HandleDisassemblyViewClicked);
        assert(is_connected);

        // Connect the RgIsaDisassemblyViewTitlebar's double click handler.
        disassembly_view_->ConnectTitleBarDoubleClick(disassembly_view_container_);

        // Connect the focus disassembly view signal to the build view's handlers.
        is_connected = connect(disassembly_view_, &RgIsaDisassemblyView::FocusDisassemblyView, this, &RgBuildView::HandleSetDisassemblyViewFocus);
        assert(is_connected);

        // Connect the handler invoked when cli output window should be highlighted.
        is_connected = connect(disassembly_view_, &RgIsaDisassemblyView::FocusCliOutputWindow,
            this, &RgBuildView::HandleSetOutputWindowFocus);
        assert(is_connected);

        // Connect the splitter's frame in focus signal.
        if (disassembly_view_splitter_ != nullptr)
        {
            is_connected = connect(disassembly_view_splitter_, &RgMaximizeSplitter::FrameInFocusSignal,
                disassembly_view_, &RgIsaDisassemblyView::HandleDisassemblyTabViewClicked);
            assert(is_connected);
        }

        // Connect the source code editor focus in signal.
        assert(current_code_editor_ != nullptr);
        if (current_code_editor_ != nullptr)
        {
            // Connect the source editor's focus in handler.
            is_connected = connect(current_code_editor_, &RgSourceCodeEditor::SourceCodeEditorFocusInEvent,
                disassembly_view_, &RgIsaDisassemblyView::HandleFocusOutEvent);
            assert(is_connected);

            // Connect the source editor's scrollbar disabling signal.
            is_connected = connect(current_code_editor_, &RgSourceCodeEditor::DisableScrollbarSignals,
                disassembly_view_, &RgIsaDisassemblyView::DisableScrollbarSignals);
            assert(is_connected);

            // Connect the source editor's scrollbar enabling signal.
            is_connected = connect(current_code_editor_, &RgSourceCodeEditor::EnableScrollbarSignals,
                disassembly_view_, &RgIsaDisassemblyView::EnableScrollbarSignals);
            assert(is_connected);
        }

        RgMenu* menu = GetMenu();
        assert(menu != nullptr);
        if (menu != nullptr)
        {
            // Connect the file menu focus in signal.
            is_connected = connect(menu, &RgMenu::FileMenuFocusInEvent, disassembly_view_, &RgIsaDisassemblyView::HandleFocusOutEvent);
            assert(is_connected);

            // Connect the file menu focus in signal to set the current view focus value.
            is_connected = connect(menu, &RgMenu::FileMenuFocusInEvent, this, &RgBuildView::HandleFileMenuFocusInEvent);
            assert(is_connected);
        }

        // Connect the view manager focus in signals.
        assert(view_manager_ != nullptr);
        if (view_manager_ != nullptr)
        {
            is_connected = connect(view_manager_, &RgViewManager::FrameFocusInSignal, disassembly_view_, &RgIsaDisassemblyView::HandleDisassemblyTabViewClicked);
            assert(is_connected);

            is_connected = connect(view_manager_, &RgViewManager::FrameFocusOutSignal, disassembly_view_, &RgIsaDisassemblyView::HandleFocusOutEvent);
            assert(is_connected);
        }

        // Connect the focus column push button signal to the disassembly view's handlers.
        is_connected = connect(cli_output_window_, &RgCliOutputView::FocusColumnPushButton, disassembly_view_, &RgIsaDisassemblyView::HandleFocusColumnsPushButton);
        assert(is_connected);

        // Connect the focus source window signal to the disassembly view's handlers.
        is_connected = connect(disassembly_view_, &RgIsaDisassemblyView::FocusSourceWindow, this, &RgBuildView::HandleFocusPreviousView);
        assert(is_connected);

        // Connect the switch container size signal from the disassembly view.
        is_connected = connect(disassembly_view_, &RgIsaDisassemblyView::SwitchDisassemblyContainerSize, this, &RgBuildView::HandleSwitchContainerSize);
        assert(is_connected);

        // Connect the Ctrl+F4 hotkey pressed signal.
        is_connected = connect(this, &RgBuildView::ShowMaximumVgprClickedSignal, disassembly_view_, &RgIsaDisassemblyView::ShowMaximumVgprClickedSignal);
        assert(is_connected);

        // Connect the enable show max VGPR options signal.
        is_connected = connect(disassembly_view_, &RgIsaDisassemblyView::EnableShowMaxVgprOptionSignal, this, &RgBuildView::EnableShowMaxVgprOptionSignal);
        assert(is_connected);

        // Connect API-specific RgBuildView signals to the disassembly view.
        ConnectDisassemblyViewApiSpecificSignals();
    }

    assert(disassembly_view_splitter_ != nullptr);
    if (disassembly_view_splitter_ != nullptr)
    {
        // Connect the handler invoked when the disassembly container has been maximized.
        is_connected = connect(disassembly_view_splitter_, &RgMaximizeSplitter::ViewMaximized,
            this, &RgBuildView::HandleDisassemblyViewSizeMaximize);
        assert(is_connected);

        // Connect the handler invoked when the disassembly container has been restored to normal size.
        is_connected = connect(disassembly_view_splitter_, &RgMaximizeSplitter::ViewRestored,
            this, &RgBuildView::HandleDisassemblyViewSizeRestore);
        assert(is_connected);

        // Connect the handler invoked when the disassembly splitter is resized.
        is_connected = connect(disassembly_view_splitter_, &RgMaximizeSplitter::splitterMoved,
            this, &RgBuildView::HandleSplitterMoved);
        assert(is_connected);
    }

    return is_connected;
}

void RgBuildView::HandleSwitchContainerSize()
{
    assert(view_manager_ != nullptr);
    if (view_manager_ != nullptr)
    {
        view_manager_->SwitchContainerSize();
    }
}

void RgBuildView::HandleFocusCliOutputWindow()
{
    assert(view_manager_ != nullptr);
    if (view_manager_ != nullptr)
    {
        view_manager_->FocusNextView();
    }
}

void RgBuildView::HandleFocusPreviousView()
{
    assert(view_manager_ != nullptr);
    if (view_manager_ != nullptr)
    {
        view_manager_->FocusPrevView();
    }
}

void RgBuildView::HandleFileMenuFocusInEvent()
{
    assert(view_manager_ != nullptr);
    if (view_manager_ != nullptr)
    {
        view_manager_->SetCurrentFocusedView(RgViewManager::RgCurrentFocusedIndex::kFileMenuCurrent);
    }
}

void RgBuildView::ConnectSourcecodeEditorSignals(RgSourceCodeEditor* editor)
{
    // Connect the file modified handler.
    bool is_connected = connect(editor, &QPlainTextEdit::modificationChanged, this, &RgBuildView::HandleEditorModificationStateChanged);
    assert(is_connected);

    // Connect the source editor's selected line changed handler.
    is_connected = connect(editor, &RgSourceCodeEditor::SelectedLineChanged, this, &RgBuildView::HandleSourceFileSelectedLineChanged);
    assert(is_connected);

    // Connect the source editor's resized handler.
    is_connected = connect(editor, &RgSourceCodeEditor::EditorResized, this, &RgBuildView::HandleSourceEditorResized);
    assert(is_connected);

    // Connect the source editor's hidden handler.
    is_connected = connect(editor, &RgSourceCodeEditor::EditorHidden, this, &RgBuildView::HandleSourceEditorHidden);
    assert(is_connected);

    // Connect the editor titlebar's "Dismiss Message" handler.
    is_connected = connect(source_editor_titlebar_, &RgSourceEditorTitlebar::DismissMsgButtonClicked,
        this, &RgBuildView::HandleCodeEditorTitlebarDismissMsgPressed);
    assert(is_connected);
}

void RgBuildView::OpenBuildSettings()
{
    SwitchEditMode(EditMode::kBuildSettings);
}

bool RgBuildView::PopulateBuildView()
{
    bool ret = false;

    // Clear the RgBuildView's file menu and source editor before repopulating it.
    ClearBuildView();

    // Verify that each source file path referenced by the project is valid.
    // Allow the user to fix any invalid paths that are found.
    bool is_project_sources_valid = RgUtils::IsProjectSourcePathsValid(project_, clone_index_, this);

    // If the project source paths are valid, proceed.
    if (is_project_sources_valid)
    {
        bool is_populated = PopulateMenu();
        assert(is_populated);
        if (is_populated)
        {
            RgMenu* menu = GetMenu();
            assert(menu != nullptr);
            if (menu != nullptr)
            {
                // Does the menu contain any files after attempting to populate it?
                if (menu->IsEmpty())
                {
                    // There are no files to display. Open the RgBuildView in an empty state, and return true.
                    SwitchEditMode(EditMode::kEmpty);
                }
                ret = true;
            }
        }
    }

    return ret;
}

void RgBuildView::CreateProjectClone()
{
    if (project_ != nullptr)
    {
        // Create Clone 0, and add it into the new project.
        std::string clone_name = RgUtils::GenerateCloneName(clone_index_);
        std::shared_ptr<RgProjectClone> clone_0 = factory_->CreateProjectClone(clone_name);
        assert(clone_0 != nullptr);
        if (clone_0 != nullptr)
        {
            project_->clones.push_back(clone_0);

            // Save the project file.
            RgConfigManager& config_manager = RgConfigManager::Instance();
            config_manager.SaveProjectFile(project_);

            if (file_menu_titlebar_ != nullptr)
            {
                // Set project name title in file menu.
                std::stringstream title;
                title << RgUtils::GetProjectTitlePrefix(project_->api) << project_->project_name;
                file_menu_titlebar_->SetTitle(title.str().c_str());
            }

            // Indicate that a new project has been loaded.
            emit ProjectLoaded(project_);

            // Create a new build settings view after a new project has been created.
            CreateBuildSettingsView();
        }
    }
}

void RgBuildView::BuildCurrentProject()
{
    // Destroy outputs from previous builds.
    DestroyProjectBuildArtifacts();

    // Clear the output window.
    cli_output_window_->ClearText();

    // Set the "is currently building" flag.
    HandleIsBuildInProgressChanged(true);

    // Notify the system that a build has started.
    emit ProjectBuildStarted();

    // The function that will be invoked by the build thread.
    auto background_task = [&]
    {
        // Build an output path where all of the build artifacts will be dumped to.
        std::string output_path = CreateProjectBuildOutputPath();

        // Create the output directory if it doesn't already exist.
        bool is_ok = RgUtils::IsDirExists(output_path);
        if (!is_ok)
        {
            is_ok = RgUtils::CreateFolder(output_path);
            assert(is_ok);
        }

        if (is_ok)
        {
            // Create a new output folder specific to the current clone's build artifacts.
            std::stringstream clone_folder_name;
            clone_folder_name << kStrCloneFolderName;
            clone_folder_name << clone_index_;

            // Append the clone folder to the output folder.
            is_ok = RgUtils::AppendFolderToPath(output_path, clone_folder_name.str(), output_path);
            if (is_ok)
            {
                // Append a path separator to the new output path.
                is_ok = RgUtils::AppendPathSeparator(output_path, output_path);
                assert(is_ok);

                // Create the output folder if it does not exist.
                if (!RgUtils::IsDirExists(output_path))
                {
                    is_ok = RgUtils::CreateFolder(output_path);
                    assert(is_ok);
                }
            }
        }

        // If the correct build output paths exist, proceed with building the project.
        if (is_ok)
        {
            // Set up the function pointer responsible for handling new output from the CLI invocation.
            using std::placeholders::_1;
            std::function<void(const std::string&)> append_build_output = std::bind(&RgBuildView::HandleNewCLIOutputString, this, _1);

            // Build the current project clone.
            cancel_bulid_signal_ = false;

            // Verify that the clone index is valid.
            int num_clones = static_cast<int>(project_->clones.size());
            bool is_clone_index_valid = (clone_index_ >= 0 && clone_index_ < num_clones);
            assert(is_clone_index_valid);
            if (is_clone_index_valid)
            {
                // Attempt to build the clone.
                bool is_project_built = false;
                std::vector<std::string> gpus_with_build_outputs;
                RgProjectAPI current_api = RgConfigManager::Instance().GetCurrentAPI();

                // Get the binary output file name and create a project.
                assert(project_->clones[clone_index_]->build_settings != nullptr);
                if (current_api == RgProjectAPI::kOpenCL)
                {
                    std::shared_ptr<RgBuildSettings> project_settings = project_->clones[clone_index_]->build_settings;
                    std::string binary_name = kStrBuildSettingsOutputBinaryFileName;
                    is_project_built = RgCliLauncher::BuildProjectCloneOpencl(project_, clone_index_, output_path, binary_name, append_build_output, gpus_with_build_outputs, cancel_bulid_signal_);
                }
                else if (current_api == RgProjectAPI::kVulkan)
                {
                    std::shared_ptr<RgBuildSettings> project_settings = project_->clones[clone_index_]->build_settings;
                    std::string binary_name = kStrBuildSettingsOutputBinaryFileName;
                    assert(project_settings != nullptr);
                    if (project_settings != nullptr)
                    {
                        binary_name = project_settings->binary_file_name;
                    }
                    is_project_built = RgCliLauncher::BuildProjectCloneVulkan(project_, clone_index_, output_path, binary_name, append_build_output, gpus_with_build_outputs, cancel_bulid_signal_);
                }
                else if (current_api == RgProjectAPI::kBinary)
                {
                    std::shared_ptr<RgBuildSettings> project_settings = project_->clones[clone_index_]->build_settings;
                    assert(project_settings != nullptr);
                    if (project_settings != nullptr)
                    {
                        is_project_built = RgCliLauncher::BuildProjectCloneBinary(project_,
                                                                                  clone_index_,
                                                                                  output_path,
                                                                                  project_settings->binary_file_name,
                                                                                  append_build_output,
                                                                                  gpus_with_build_outputs,
                                                                                  cancel_bulid_signal_);
                    }
                }

                // Verify that the build was not canceled.
                if (!cancel_bulid_signal_)
                {
                    // If the project was built successfully, parse the session metadata file and populate an RgCliOutput structure.
                    if (is_project_built)
                    {
                        // Load the build outputs in the project's directory.
                        std::string project_directory;
                        if (RgUtils::ExtractFileDirectory(project_->project_file_full_path, project_directory))
                        {
                            bool is_output_loaded = LoadBuildOutput(project_directory, &gpus_with_build_outputs);
                            assert(is_output_loaded);
                            if (is_output_loaded)
                            {
                                // Trigger the build success signal.
                                emit ProjectBuildSuccess();
                            }
                            else
                            {
                                // Trigger the build failure signal.
                                emit ProjectBuildFailure();
                            }
                        }
                    }
                }
                else
                {
                    // Trigger the build cancellation signal.
                    emit ProjectBuildCanceled();

                    // Notify the user that the build was canceled.
                    HandleNewCLIOutputString(kStrBuildCanceled);
                }
            }
        }
    };

    // Launch the build thread.
    std::thread buildThread(background_task);
    buildThread.detach();
}

bool RgBuildView::CreateFileMenu()
{
    // Create the API-specific file menu.
    bool is_ok = CreateMenu(this);
    if (is_ok)
    {
        // Retrieve a pointer to the file menu.
        RgMenu* menu = GetMenu();
        assert(menu != nullptr);
        if (menu != nullptr)
        {
            assert(project_ != nullptr);
            if (project_ != nullptr)
            {
                // Ensure that the incoming clone index is valid for the current project.
                bool is_valid_range = (clone_index_ >= 0 && clone_index_ < project_->clones.size());
                assert(is_valid_range);

                if (is_valid_range)
                {
                    std::shared_ptr<RgProjectClone> project_clone = project_->clones[clone_index_];

                    // Initialize the menu with default items.
                    menu->InitializeDefaultMenuItems(project_clone);
                }
            }
        }

        // Create the menu title bar where the program name is displayed.
        file_menu_titlebar_ = new RgMenuTitlebar();

        // Wrap the file menu in a view container with its title bar.
        file_menu_view_container_->SetMainWidget(menu);
        file_menu_view_container_->SetTitlebarWidget(file_menu_titlebar_);
        file_menu_view_container_->setObjectName(kStrRgFileMenuViewContainer);

        // Connect signals for the file menu.
        ConnectFileSignals();
    }

    return is_ok;
}

bool RgBuildView::CreateNewEmptyProject()
{
    bool ret = false;
    std::string project_name;

    // Get the global configuration.
    RgConfigManager& config_manager = RgConfigManager::Instance();
    std::shared_ptr<RgGlobalSettings> global_settings = config_manager.GetGlobalConfig();

    if (global_settings->use_default_project_name == true)
    {
        // Generate a default project name.
        project_name = RgUtils::GenerateDefaultProjectName();

        // Create a project instance with the given name.
        project_ = factory_->CreateProject(project_name, RgConfigManager::GenerateProjectFilepath(project_name));
        assert(project_ != nullptr);

        // Create the clone
        CreateProjectClone();

        // We're done.
        ret = true;
    }
    else
    {
        // Repeatedly ask the user for a project name when their provided name is invalid.
        bool is_valid_project_path = false;
        do
        {
             auto rename_project_dialog = std::unique_ptr<RgRenameProjectDialog>(factory_->CreateRenameProjectDialog(project_name, parent_));

            // Prompt the user for the project name.
            int rc = rename_project_dialog->exec();
            if (rc == QDialog::Accepted)
            {
                // Generate the path to where the new project's project file would live.
                std::string project_file_path = RgConfigManager::GenerateProjectFilepath(project_name);

                // The path for the new project is valid only if it doesn't already exist.
                is_valid_project_path = !RgUtils::IsFileExists(project_file_path);
                if (is_valid_project_path)
                {
                    // Create a project instance with the given name.
                    project_ = factory_->CreateProject(project_name, project_file_path);
                    assert(project_ != nullptr);

                    // Create the clone.
                    CreateProjectClone();

                    // We're done.
                    ret = true;
                }
                else
                {
                    // Let the user know that the project name they have provided has already been used.
                    RgUtils::ShowErrorMessageBox(kStrNewProjectAlreadyExists, this);
                }
            }
            else
            {
                break;
            }

        } while (!is_valid_project_path);
    }

    return ret;
}

bool RgBuildView::InitializeView()
{
    bool status = false;

    // Create an empty panel to insert alongside the file menu when no files are open.
    empty_panel_ = new QWidget(this);

    // Create container for source view widgets.
    source_view_stack_ = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    source_view_stack_->setLayout(layout);

    // Wrap the source code stack in a view container and a source editor titlebar.
    source_view_container_ = new RgViewContainer();
    source_view_container_->SetMainWidget(source_view_stack_);
    source_editor_titlebar_ = new RgSourceEditorTitlebar(source_view_container_);
    source_view_container_->SetTitlebarWidget(source_editor_titlebar_);
    source_view_container_->setObjectName(kStrRgSourceViewContainer);

    // Create the disassembly view splitter.
    disassembly_view_splitter_ = new RgMaximizeSplitter(this);
    disassembly_view_splitter_->setOrientation(Qt::Orientation::Horizontal);
    disassembly_view_splitter_->setChildrenCollapsible(false);

    // Add the source view container to the disassembly view splitter.
    disassembly_view_splitter_->AddMaximizableWidget(source_view_container_);

    // Set up the splitter between the file menu and the rest of the views.
    file_menu_splitter_ = new QSplitter(Qt::Orientation::Horizontal, this);

    // Create the file menu's container.
    file_menu_view_container_ = new RgViewContainer();

    // Set the fixed width of file menu container.
    file_menu_view_container_->setFixedWidth(kStrFileMenuViewContainerWidth);

    // Add the file menu and the disassembly view splitter to the file menu splitter.
    file_menu_splitter_->addWidget(file_menu_view_container_);
    file_menu_splitter_->addWidget(disassembly_view_splitter_);

    // The file menu should not grow with the window, while the source code view should.
    file_menu_splitter_->setStretchFactor(0, 0);
    file_menu_splitter_->setStretchFactor(1, 1);

    // Disable the file menu splitter.
    file_menu_splitter_->handle(1)->setDisabled(true);

    // Create the output window.
    cli_output_window_ = new RgCliOutputView(this);

    // Wrap the build output view in a view container with an embedded titlebar.
    build_output_view_container_ = new RgViewContainer();
    build_output_view_container_->SetMainWidget(cli_output_window_);
    build_output_view_container_->setObjectName(kStrRgBuildOutputViewContainer);

    // Create a vertical splitter to divide the RgBuildView's FileMenu/SourceEditors and the Output Window.
    output_splitter_ = new RgMaximizeSplitter(this);
    output_splitter_->setOrientation(Qt::Orientation::Vertical);

    // Connect the build output window signals.
    ConnectOutputWindowSignals();

    // Add the file menu's splitter and the output window to the splitter.
    output_splitter_->addWidget(file_menu_splitter_);
    output_splitter_->AddMaximizableWidget(build_output_view_container_);

    // Let the file menu and code editor resize, and the output window will stay vertically squished.
    output_splitter_->setStretchFactor(0, 6);
    output_splitter_->setStretchFactor(1, 1);
    output_splitter_->setCollapsible(1, false);

    // Create a main window layout, and add the root-level splitter widget.
    QVBoxLayout* main_layout = new QVBoxLayout(this);
    main_layout->addWidget(output_splitter_);
    this->setLayout(main_layout);

    // Restore the previous session RgBuildView layout.
    RestoreViewLayout();

    // Setup view manager.
    view_manager_ = new RgViewManager(this);
    view_manager_->AddView(file_menu_view_container_, true);
    view_manager_->AddView(source_view_container_, true);
    view_manager_->AddView(build_output_view_container_, true);

    // Connect signals for the Build View.
    ConnectBuildViewSignals();

    // Declare EditMode as a meta type so it can be used with slots/signals.
    int id = qRegisterMetaType<EditMode>();
    Q_UNUSED(id);

    // Create the file menu.
    CreateFileMenu();

    // Update the menu's title bar to display the Project name.
    assert(file_menu_titlebar_ != nullptr);
    assert(project_ != nullptr);
    if (file_menu_titlebar_ != nullptr && project_ != nullptr)
    {
        // Set project name title in file menu.
        std::stringstream title;
        title << RgUtils::GetProjectTitlePrefix(project_->api) << project_->project_name;
        file_menu_titlebar_->SetTitle(title.str().c_str());
    }

    // Create a new build settings view after a new project has been created.
    CreateBuildSettingsView();

    // Create and initialize views specific to the current mode only.
    status = InitializeModeSpecificViews();

    // Apply the stylesheets for the build settings.
    RgProjectAPI current_api = RgConfigManager::Instance().GetCurrentAPI();
    std::shared_ptr<RgFactory> factory = RgFactory::CreateFactory(current_api);
    assert(factory != nullptr);
    if (factory != nullptr)
    {
        std::shared_ptr<RgAppState> app_state = factory->CreateAppState();
        assert(app_state != nullptr);
        if (app_state != nullptr)
        {
            SetBuildSettingsStylesheet(app_state->GetBuildSettingsViewStylesheet());
        }
    }

    return status;
}

void RgBuildView::SetBuildSettingsStylesheet(const std::string& stylesheet)
{
    [[maybe_unused]] RgProjectAPI current_api = RgConfigManager::Instance().GetCurrentAPI();
    assert(build_settings_view_ != nullptr || (current_api == RgProjectAPI::kBinary && build_settings_view_ == nullptr));
    if (build_settings_view_ != nullptr)
    {
        build_settings_view_->setStyleSheet(stylesheet.c_str());
    }
}

bool RgBuildView::HasSourceCodeEditors() const
{
    return source_code_editors_.empty();
}

bool RgBuildView::HasProject() const
{
    return (project_ != nullptr);
}

bool RgBuildView::LoadBuildOutput(const std::string& project_folder, const std::vector<std::string>* target_gpus)
{
    bool is_loaded = false;

    std::vector<std::string> target_gpu_family_results_to_load;

    // Build a list of possible target GPUs to attempt to load results for, based on the supported GPUs for the current mode.
    std::shared_ptr<RgCliVersionInfo> version_info = RgConfigManager::Instance().GetVersionInfo();
    assert(version_info != nullptr);
    if (version_info != nullptr)
    {
        // Determine which GPU architectures and families are supported in the current mode.
        const std::string& current_mode = RgConfigManager::Instance().GetCurrentModeString();
        auto mode_architectures_iter = version_info->gpu_architectures.find(current_mode);

        if (mode_architectures_iter != version_info->gpu_architectures.end())
        {
            const std::vector<RgGpuArchitecture>& mode_architectures = mode_architectures_iter->second;

            // Step through each architecture.
            for (auto architecture_iter = mode_architectures.begin(); architecture_iter != mode_architectures.end(); ++architecture_iter)
            {
                // Step through each family within the architecture.
                for (auto family_iter = architecture_iter->gpu_families.begin(); family_iter != architecture_iter->gpu_families.end(); ++family_iter)
                {
                    // Add the family name to the list of targets to attempt to load results for.
                    target_gpu_family_results_to_load.push_back(family_iter->family_name);
                }
            }
        }
    }

    // Build a path to the project's output directory.
    std::string build_output_path = CreateProjectBuildOutputPath();

    // Generate a clone name string based on the current clone index.
    std::string output_folder_path;

    bool is_ok = RgUtils::AppendFolderToPath(project_folder, kStrOutputFolderName, output_folder_path);
    assert(is_ok);
    if (is_ok)
    {
        // Append the clone folder to the build output path.
        std::string cloneNameString = RgUtils::GenerateCloneName(clone_index_);
        is_ok = RgUtils::AppendFolderToPath(output_folder_path, cloneNameString, output_folder_path);
        assert(is_ok);
        if (is_ok)
        {
            const std::vector<std::string>* gpus_to_load = nullptr;
            if (target_gpus != nullptr)
            {
                // If a list of GPUs was provided, attempt to load output for each.
                gpus_to_load = target_gpus;
            }
            else
            {
                // When no target GPUs to load are provided, fall back to attempting to load results for all possible target GPUs.
                // If the session metadata for the target GPU doesn't exist, there's no disassembly to load.
                gpus_to_load = &target_gpu_family_results_to_load;
            }

            assert(gpus_to_load != nullptr);
            if (gpus_to_load != nullptr)
            {
                // Attempt to load outputs for each GPU that was targeted.
                for (const std::string& current_gpu : *gpus_to_load)
                {
                    std::stringstream metadataFilenameStream;
                    metadataFilenameStream << current_gpu;
                    metadataFilenameStream << "_";
                    metadataFilenameStream << kStrSessionMetadataFilename;
                    bool is_loaded_for_gpu = false;

                    std::string full_metadata_file_path;
                    is_ok = RgUtils::AppendFileNameToPath(output_folder_path, metadataFilenameStream.str(), full_metadata_file_path);
                    assert(is_ok);
                    if (is_ok)
                    {
                        // Does the session metadata file exist?
                        bool is_metadata_exists = RgUtils::IsFileExists(full_metadata_file_path);
                        if (is_metadata_exists)
                        {
                            // Emit a signal so the file coloring in the file menu is updated.
                            emit UpdateFileColoring();

                            std::shared_ptr<RgCliBuildOutput> gpu_output = nullptr;
                            is_loaded_for_gpu = LoadSessionMetadata(full_metadata_file_path, gpu_output);

                            if (is_loaded_for_gpu && gpu_output != nullptr)
                            {
                                // Add the outputs to the map to store per-GPU results.
                                build_outputs_[current_gpu] = gpu_output;
                                is_loaded = true;
                            }
                        }
                    }
                }
            }
        }
    }

    return is_loaded;
}

bool RgBuildView::LoadProjectFile(const std::string& project_file_path)
{
    bool ret = false;

    RgConfigManager& config_manager = RgConfigManager::Instance();

    // Reset the view state before loading the project file.
    ResetView();

    // Get the configuration manager to load a project file.
    project_ = config_manager.LoadProjectFile(project_file_path);

    // Update the window title if the project loaded correctly.
    assert(project_ != nullptr);
    if (project_ != nullptr)
    {
        // Signal that a new project has been loaded into the RgBuildView.
        emit ProjectLoaded(project_);

        ret = true;
    }
    else
    {
        // Tell the user that the project file failed to load.
        std::stringstream error_stream;
        error_stream << kStrErrCannotLoadProjectFile << " ";
        error_stream << project_file_path;
        RgUtils::ShowErrorMessageBox(error_stream.str().c_str(), this);
    }

    return ret;
}

void RgBuildView::ReloadFile(const std::string& file_path)
{
    SetSourceCodeText(file_path);
}

void RgBuildView::SaveCurrentFile(EditMode)
{
    bool is_source_code_editor_valid = (current_code_editor_ != nullptr);
    assert(is_source_code_editor_valid);
    if (is_source_code_editor_valid)
    {
        RgMenu* menu = GetMenu();
        assert(menu != nullptr);
        if (menu != nullptr && menu->GetSelectedFileItem() != nullptr)
        {
            std::string current_file_name = menu->GetSelectedFilePath();

            // Ask the user for a filename if none exists so far.
            if (current_file_name.empty())
            {
                std::string filter = std::string(kStrFileDialogFilterOpencl) + ";;" + kStrFileDialogFilterAll;
                current_file_name = QFileDialog::getSaveFileName(this, kStrFileDialogSaveNewFile,
                    RgConfigManager::Instance().GetLastSelectedFolder().c_str(), filter.c_str()).toStdString();

                // Extract directory from full path.
                std::string file_directory;
                bool is_ok = RgUtils::ExtractFileDirectory(current_file_name, file_directory);
                assert(is_ok);

                if (is_ok)
                {
                    // Update last selected directory in global config.
                    std::shared_ptr<RgGlobalSettings> global_config = RgConfigManager::Instance().GetGlobalConfig();
                    global_config->last_selected_directory = file_directory;
                }
            }

            // Write the editor text to file if the file path is valid.
            if (!current_file_name.empty())
            {
                SaveEditorTextToFile(current_code_editor_, current_file_name);
            }
        }
    }
}

RgUnsavedItemsDialog::UnsavedFileDialogResult RgBuildView::RequestSaveFile(const std::string& full_path)
{
    QStringList unsaved_files;

    // Get editor that corresponds to this filepath.
    RgSourceCodeEditor* editor = GetEditorForFilepath(full_path);

    bool is_editor_valid = (editor != nullptr);
    assert(is_editor_valid);

    // Add file to the list if it is unsaved (modified).
    if (is_editor_valid && editor->document()->isModified())
    {
        unsaved_files << full_path.c_str();
    }

    // Ask the user to save edited files.
    RgUnsavedItemsDialog::UnsavedFileDialogResult user_response = RequestSaveFiles(unsaved_files);
    return user_response;
}

bool RgBuildView::ShowSaveDialog(RgFilesToSave files_to_save /* = rgFilesToSave::All */, bool should_save_source_files /* = false */)
{
    bool ret = false;
    QStringList unsaved_files;

    // This flag would be set to true if the user chose to cancel the operation.
    // In that case, we should return false from this function and the subsequent
    // logic would make sure to cancel the entire operation (as part of which showing
    // the save dialog was required).
    bool is_canceled = false;

    if (should_save_source_files && (files_to_save == RgFilesToSave::kSourceFiles || files_to_save == RgFilesToSave::kAll))
    {
        // Add unsaved source files to the list of files that must be saved.
        GetUnsavedSourceFiles(unsaved_files);
    }

    // Does the user have pending Build Settings changes to save?
    bool pending_build_settings_changes = false;

    // If the build settings have been modified but the changes are still pending, add the build settings file to the list.
    if (build_settings_view_ != nullptr)
    {
        if (files_to_save == RgFilesToSave::kBuildSettings || files_to_save == RgFilesToSave::kAll)
        {
            pending_build_settings_changes = build_settings_view_->GetHasPendingChanges();
            if (pending_build_settings_changes)
            {
                // Add a build settings item to the unsaved files list.
                unsaved_files << kStrMenuBuildSettings;
            }
        }

        // File changes are ignored unless it is:
        // Project close.
        // App exit.
        if (pending_build_settings_changes || should_save_source_files)
        {
            // Ask the user if they want to save files with modifications.
            RgUnsavedItemsDialog::UnsavedFileDialogResult save_result = RgUnsavedItemsDialog::kYes;
            if (!unsaved_files.empty())
            {
                save_result = RequestSaveFiles(unsaved_files);
                switch (save_result)
                {
                case RgUnsavedItemsDialog::kNo:
                {
                    if (pending_build_settings_changes)
                    {
                        // If the user clicks "No," they don't care about the pending changes. Revert them before moving on.
                        build_settings_view_->RevertPendingChanges();
                    }
                    else
                    {
                        // Discard the changes from all editors.
                        for (const QString& unsaved_file_path : unsaved_files)
                        {
                            RgSourceCodeEditor* editor = GetEditorForFilepath(unsaved_file_path.toStdString());
                            DiscardEditorChanges(editor);
                        }
                    }
                }
                break;
                case RgUnsavedItemsDialog::kCancel:
                    is_canceled = true;
                    break;
                default:
                    break;
                }
            }

            // If "Yes", proceed with the build. If "No", proceed with the build since the pending settings have been reverted.
            // If "Cancel," stop the attempt to build and continue where the user left off.
            ret = (save_result == RgUnsavedItemsDialog::kYes || save_result == RgUnsavedItemsDialog::kNo);
        }

        ret = (ret || (!pending_build_settings_changes)) && !is_canceled;
    }
    else
    {
        // Return true if the build settings view was not created yet.
        // This happens when the user has not created or opened a project,
        // and attempts to quit the application.
        ret = true;
    }

    return ret;
}

RgUnsavedItemsDialog::UnsavedFileDialogResult RgBuildView::RequestSaveFiles(const QStringList& unsaved_files)
{
    RgUnsavedItemsDialog::UnsavedFileDialogResult result = RgUnsavedItemsDialog::kCancel;

    // Don't display the dialog if there are no files, just return.
    if (unsaved_files.size() > 0)
    {
        // Create a modal unsaved file dialog.
        RgUnsavedItemsDialog* unsaved_file_dialog = new RgUnsavedItemsDialog(this);
        unsaved_file_dialog->setModal(true);
        unsaved_file_dialog->setWindowTitle(kStrUnsavedItemsDialogTitle);

        // Add unsaved files to the dialog list.
        unsaved_file_dialog->AddFiles(unsaved_files);

        // Register the dialog with the scaling manager.
        unsaved_file_dialog->show();

        // Center the dialog on the view (registering with the scaling manager
        // shifts it out of the center so we need to manually center it).
        RgUtils::CenterOnWidget(unsaved_file_dialog, this);

        // Execute the dialog and get the result.
        result = static_cast<RgUnsavedItemsDialog::UnsavedFileDialogResult>(unsaved_file_dialog->exec());

        switch (result)
        {
        case RgUnsavedItemsDialog::kYes:
            // Save all files and indicated dialog accepted.
            if (!SaveFiles(unsaved_files))
            {
                // The files weren't saved, so the action in-flight should be cancelled.
                result = RgUnsavedItemsDialog::kCancel;
            }
            break;
            // If the user chooses No or Cancel, nothing needs to happen except for making the dialog disappear.
        case RgUnsavedItemsDialog::kNo:
        case RgUnsavedItemsDialog::kCancel:
            break;
        default:
            // Shouldn't get here.
            assert(false);
        }

        // Free memory.
        RG_SAFE_DELETE(unsaved_file_dialog);
    }
    else
    {
        // No files need to be saved because none have been modified.
        result = RgUnsavedItemsDialog::kYes;
    }

    return result;
}

bool RgBuildView::SaveFiles(const QStringList& unsaved_files)
{
    bool is_saved = true;

    // Step through each of the files with pending changes.
    auto string_list_iter = unsaved_files.begin();
    while (string_list_iter != unsaved_files.end())
    {
        // If the file isn't the Build Settings item, it's a path to an input source file.
        const std::string& file_path = string_list_iter->toStdString();
        if (file_path.compare(kStrMenuBuildSettings) == 0)
        {
            // Submit all pending changes and save the build settings file.
            if (build_settings_view_ != nullptr)
            {
                is_saved = build_settings_view_->SaveSettings();
            }
        }
        else
        {
            SaveSourceFile(string_list_iter->toStdString().c_str());
        }

        string_list_iter++;

        if (!is_saved)
        {
            break;
        }
    }

    return is_saved;
}

void RgBuildView::SaveSourceFile(const std::string& source_file_path)
{
    RgSourceCodeEditor* editor = GetEditorForFilepath(source_file_path);

    bool isEditorValid = (editor != nullptr);
    assert(isEditorValid);

    if (isEditorValid)
    {
        SaveEditorTextToFile(editor, source_file_path);
    }
}

bool RgBuildView::SaveCurrentState()
{
    // Save all source files when a new build is started.
    QStringList unsaved_sources;
    GetUnsavedSourceFiles(unsaved_sources);
    if (!unsaved_sources.empty())
    {
        for (const QString& source_file_path : unsaved_sources)
        {
            SaveSourceFile(source_file_path.toStdString().c_str());
        }
    }

    // Save the current project first.
    SaveCurrentFile(edit_mode_);

    // The build settings must be saved in order to proceed with a build.
    bool should_proceed_with_build = ShowSaveDialog(RgFilesToSave::kBuildSettings);

    return should_proceed_with_build;
}

bool RgBuildView::RequestRemoveAllFiles()
{
    bool is_save_accepted = ShowSaveDialog(RgBuildView::RgFilesToSave::kAll, true);
    if (is_save_accepted)
    {
        RgMenu* menu = GetMenu();
        assert(menu != nullptr);
        if (menu != nullptr)
        {
            // Remove all file menu items.
            auto editor_iter = source_code_editors_.begin();
            while (editor_iter != source_code_editors_.end())
            {
                std::string full_path = editor_iter->first;
                menu->RemoveItem(full_path);
                RemoveEditor(full_path);

                // Keep getting first item until all are removed.
                editor_iter = source_code_editors_.begin();
            }
        }
    }

    return is_save_accepted;
}

void RgBuildView::SetSourceCodeText(const std::string& file_full_path)
{
    QString src_code;

    if (current_code_editor_ != nullptr)
    {
        if (!file_full_path.empty())
        {
            bool is_ok = RgUtils::ReadTextFile(file_full_path, src_code);
            if (is_ok)
            {
                // Save the current line number and vertical scroll position.
                int current_line_number = current_code_editor_->GetSelectedLineNumber();
                const int v_scroll_position = current_code_editor_->verticalScrollBar()->value();

                // Set the text.
                current_code_editor_->setText(src_code);

                // Remember most recent time file was modified.
                QFileInfo file_info(file_full_path.c_str());
                file_modified_time_map_[current_code_editor_] = file_info.lastModified();

                // Indicate that a freshly loaded file is considered unmodified.
                current_code_editor_->document()->setModified(false);

                // Set the highlighted line.
                QList<int> line_numbers;
                line_numbers << current_line_number;
                current_code_editor_->SetHighlightedLines(line_numbers);

                // Restore the cursor position after reloading the file.
                QTextCursor cursor(current_code_editor_->document()->findBlockByLineNumber(current_line_number - 1));
                current_code_editor_->setTextCursor(cursor);
                if (current_line_number <= current_code_editor_->document()->blockCount())
                {
                    current_code_editor_->verticalScrollBar()->setValue(v_scroll_position);
                }
            }
        }
        else
        {
            current_code_editor_->setText("");
            current_code_editor_->document()->setModified(false);
        }
    }
}

void RgBuildView::ToggleDisassemblyViewVisibility(bool is_visible)
{
    bool is_view_created = disassembly_view_container_ != nullptr && disassembly_view_ != nullptr;

    // If the disassembly view is being hidden, the splitter may have to be restored before hiding the view.
    if (!is_visible)
    {
        assert(disassembly_view_splitter_ != nullptr);
        if (disassembly_view_splitter_ != nullptr)
        {
            // Is there a container that's currently maximized in the splitter?
            QWidget* maximized_widget = disassembly_view_splitter_->GetMaximizedWidget();
            if (maximized_widget == disassembly_view_container_)
            {
                // Restore the maximized view before switching to the build settings.
                disassembly_view_splitter_->Restore();
            }
        }
    }

    if (is_visible)
    {
        // The view needs to exist if we try to make it visible.
        assert(is_view_created);
    }

    if (is_view_created)
    {
        // Show the disassembly view container to display the tables within.
        disassembly_view_container_->setVisible(is_visible);
        disassembly_view_container_->SetHiddenState(!is_visible);
    }

    assert(source_view_container_ != nullptr);
    if (source_view_container_ != nullptr)
    {
        // Only allow the source editor container to be maximized/restored when the disassembly view is available.
        source_view_container_->SetIsMaximizable(is_visible);
    }
}

void RgBuildView::HandleSourceEditorHidden()
{
    // Hide the find widget.
    ToggleFindWidgetVisibility(false);
}

void RgBuildView::HandleSourceEditorResized()
{
    if (find_widget_ != nullptr)
    {
        UpdateFindWidgetGeometry();
    }
}

void RgBuildView::HandleSourceEditorOpenHeaderRequest(const QString& path)
{
    // Check if the file exists.
    assert(project_ != nullptr);
    assert(clone_index_ < project_->clones.size());
    assert(project_->clones[clone_index_] != nullptr);
    assert(project_->clones[clone_index_]->build_settings != nullptr);
    if (project_ != nullptr && clone_index_ < project_->clones.size() &&
        project_->clones[clone_index_] != nullptr)
    {
        const std::vector<std::string>& include_paths =
            project_->clones[clone_index_]->build_settings->additional_include_directories;

        // The path to the file that we would like to open.
        std::string path_to_open;

        // Try to find if any file with that full path exists on the system.
        bool is_exist = RgUtils::IsFileExists(path.toStdString());
        if (!is_exist)
        {
            // Try the local directory.
            assert(current_code_editor_ != nullptr);
            if (current_code_editor_ != nullptr)
            {
                // Get the directory of the currently edited file.
                std::string file_directory;
                std::string file_path = GetFilepathForEditor(current_code_editor_);
                bool is_dir_extracted = RgUtils::ExtractFileDirectory(file_path, file_directory);
                if (is_dir_extracted)
                {
                    // Search for the user's file in the directory
                    // where the currently edited file is located.
                    std::stringstream full_path;
                    full_path << file_directory << "/" << path.toStdString();
                    if (RgUtils::IsFileExists(full_path.str()))
                    {
                        // We found it.
                        path_to_open = full_path.str();
                        is_exist = true;
                    }
                }
            }

            if (!is_exist)
            {
                // Try to create the path for each of the Additional Include paths.
                for (const std::string include_path : include_paths)
                {
                    std::stringstream full_path;
                    full_path << include_path;
                    full_path << "/" << path.toStdString();
                    if (RgUtils::IsFileExists(full_path.str()))
                    {
                        path_to_open = full_path.str();
                        is_exist = true;
                        break;
                    }
                }
            }
        }
        else
        {
            path_to_open = path.toStdString();
        }

        if (is_exist)
        {
            // Open the include file.
            bool is_launched = OpenIncludeFile(path_to_open);
            if (!is_launched)
            {
                // Notify the user that the viewer app could not be launched.
                std::stringstream err_msg;
                err_msg << kStrErrCouldNotOpenHeaderFileViewer <<
                    RgConfigManager::Instance().GetIncludeFileViewer();
                emit SetStatusBarText(err_msg.str());
            }
        }
        else
        {
            // Notify the user that the header could not be located.
            emit SetStatusBarText(kStrErrCouldNotLocateHeaderFile);
        }
    }
}

void RgBuildView::HandleCodeEditorTitlebarDismissMsgPressed()
{
    assert(current_code_editor_ != nullptr);
    if (current_code_editor_ != nullptr)
    {
        current_code_editor_->SetTitleBarText("");
    }
}

void RgBuildView::HandleHighlightedCorrelationLineUpdated(int line_number)
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
    current_code_editor_->SetHighlightedLines(highlighted_lines);
}

void RgBuildView::HandleIsLineCorrelationEnabled(RgSourceCodeEditor* editor, bool is_enabled)
{
    // Update the correlation state flag.
    std::string file_path = GetFilepathForEditor(editor);
    UpdateSourceFileCorrelationState(file_path, is_enabled);

    if (!is_enabled)
    {
        // Invalidate the highlighted correlation lines within the source editor and disassembly views.
        HandleSourceFileSelectedLineChanged(editor, kInvalidCorrelationLineIndex);
        HandleHighlightedCorrelationLineUpdated(kInvalidCorrelationLineIndex);
    }
    else
    {
        // Use the currently-selected line in the source editor to highlight correlated lines in the disassembly table.
        assert(editor != nullptr);
        if (editor != nullptr)
        {
            // Trigger a line correlation update by re-selecting the current line in the source editor.
            int selected_line_number = editor->GetSelectedLineNumber();
            HandleSourceFileSelectedLineChanged(editor, selected_line_number);
        }
    }

    // Update the editor's titlebar text.
    if (current_code_editor_ == editor)
    {
        UpdateSourceEditorTitlebar(editor);
    }
}

void RgBuildView::HandleBuildSettingsPendingChangesStateChanged(bool has_pending_changes)
{
    // Pull the build settings menu item out of the file menu.
    RgMenu* menu = GetMenu();
    assert(menu != nullptr);
    if (menu != nullptr)
    {
        RgMenuBuildSettingsItem* build_settings_menu_item = menu->GetBuildSettingsItem();
        assert(build_settings_menu_item != nullptr);

        // Toggle the pending changed flag.
        if (build_settings_menu_item != nullptr)
        {
            build_settings_menu_item->SetHasPendingChanges(has_pending_changes);

            // Update the file menu save build settings item visibility.
            emit CurrentEditorModificationStateChanged(has_pending_changes);
        }

        // Set the enabledness of the "Save" button.
        assert(settings_buttons_view_ != nullptr);
        if (settings_buttons_view_ != nullptr)
        {
            settings_buttons_view_->EnableSaveButton(has_pending_changes);
        }
    }
}

void RgBuildView::HandleBuildSettingsSaved(std::shared_ptr<RgBuildSettings> build_settings)
{
    assert(project_ != nullptr);
    if (project_ != nullptr)
    {
        bool project_has_clones = !project_->clones.empty();
        assert(project_has_clones);

        if (project_has_clones)
        {
            // Replace the clone's build settings with the latest updated settings.
            project_->clones[clone_index_]->build_settings = build_settings;

            // Save the project after adding a source file.
            RgConfigManager::Instance().SaveProjectFile(project_);
        }
    }

    // Signal that the build settings have changed and should be saved.
    emit ProjectBuildSettingsSaved(build_settings);
}

void RgBuildView::HandleSelectedTargetGpuChanged(const std::string& target_gpu)
{
    // Look for build output for the target GPU being switched to.
    auto target_gpu_build_outputs = build_outputs_.find(target_gpu);

    // Find the build outputs for the given GPU.
    bool is_valid_target_gpu = target_gpu_build_outputs != build_outputs_.end();
    assert(is_valid_target_gpu);
    if (is_valid_target_gpu)
    {
        // Switch the target GPU.
        current_target_gpu_ = target_gpu;

        InputFileToBuildOutputsMap outputs_map;
        bool got_outputs = GetInputFileOutputs(target_gpu_build_outputs->second, outputs_map);

        assert(got_outputs);
        if (got_outputs)
        {
            assert(current_code_editor_ != nullptr);
            if (current_code_editor_ != nullptr)
            {
                std::string current_file_path = GetFilepathForEditor(current_code_editor_);

                // Does the currently-selected source file have build output for the new Target GPU?
                auto source_file_outputs_iter = outputs_map.find(current_file_path);

                // Trigger an update to handle highlighting the correlated disassembly
                // lines associated with the selected line in the current file.
                bool is_file_built_for_target = source_file_outputs_iter != outputs_map.end();
                if (is_file_built_for_target)
                {
                    // Use the currently-selected line in the source editor to highlight correlated lines in the disassembly table.
                    int selectedLineNumber = current_code_editor_->GetSelectedLineNumber();
                    HandleSourceFileSelectedLineChanged(current_code_editor_, selectedLineNumber);
                }
            }
        }
    }
}

void RgBuildView::HandleFileRenamed(const std::string& old_file_path, const std::string& new_file_path)
{
    // Update references to the old file path within the BuildView.
    RenameFile(old_file_path, new_file_path);

    // Emit a signal to trigger the file rename within the project file.
    emit FileRenamed(old_file_path, new_file_path);

    // If the paths match, the file just finished being renamed from the file menu.
    if (old_file_path.compare(new_file_path) == 0)
    {
        // Switch the focus from the file menu item to the source editor.
        if (current_code_editor_ != nullptr)
        {
            current_code_editor_->setFocus();
        }
    }
}

void RgBuildView::HandleFocusNextView()
{
    assert(view_manager_ != nullptr);
    if (view_manager_ != nullptr)
    {
        // Manually advance to the next view within the RgViewManager.
        view_manager_->FocusNextView();
    }
}

void RgBuildView::HandleFindWidgetVisibilityToggled()
{
    if (find_widget_ != nullptr)
    {
        bool is_visible = find_widget_->isVisible();
        ToggleFindWidgetVisibility(!is_visible);
    }
}

void RgBuildView::HandleProjectRenamed(const std::string& project_name)
{
    // Get current project directory.
    std::string directory;
    RgUtils::ExtractFileDirectory(project_->project_file_full_path, directory);

    // Create full path by appending new name to directory.
    char separator = static_cast<char>(QDir::separator().unicode());
    std::string full_path = directory + separator + project_name + kStrProjectFileExtension;

    // Rename the project.
    RenameProject(full_path);
}

void RgBuildView::HandleEditorModificationStateChanged(bool is_modified)
{
    // Get the sender of the signal.
    RgSourceCodeEditor* editor = static_cast<RgSourceCodeEditor*>(sender());

    // Get the file path for the given editor instance.
    std::string full_file_path = GetFilepathForEditor(editor);

    RgMenu* menu = GetMenu();
    assert(menu != nullptr);
    if (menu != nullptr)
    {
        // Set menu item saved state.
        menu->SetItemIsSaved(full_file_path, !is_modified);
    }

    // If the editor being modified is the current one, emit the modification signal.
    if (editor == current_code_editor_)
    {
        emit CurrentEditorModificationStateChanged(is_modified);
    }

    // Was a source file modified and saved after the last build time?
    bool files_modified_after_build = CheckSourcesModifiedSinceLastBuild(editor);
    if (files_modified_after_build)
    {
        emit LineCorrelationEnabledStateChanged(editor, false);
    }
    else
    {
        emit LineCorrelationEnabledStateChanged(editor, !is_modified);
    }
}

void RgBuildView::HandleMenuItemCloseButtonClicked(const std::string& full_path)
{
    std::stringstream msg;
    msg << full_path << kStrMenuBarConfirmRemoveFileDialogWarning;

    if (ShowRemoveFileConfirmation(msg.str(), full_path))
    {
        // Remove the input file from the RgBuildView.
        RemoveInputFile(full_path);
    }
}

void RgBuildView::SetViewContentsWidget(QWidget* new_contents)
{
    assert(source_view_stack_ != nullptr);
    if (source_view_stack_ != nullptr)
    {
        // Hide all existing views before replacing with the new one.
        QLayout* layout = source_view_stack_->layout();

        assert(layout != nullptr);
        if (layout != nullptr)
        {
            for (int childIndex = 0; childIndex < layout->count(); ++childIndex)
            {
                QLayoutItem* pItemLayout = layout->itemAt(childIndex);
                assert(pItemLayout != nullptr);
                if (pItemLayout != nullptr)
                {
                    QWidget* item = pItemLayout->widget();
                    assert(item != nullptr);
                    if (item != nullptr)
                    {
                        item->hide();
                    }
                }
            }
        }

        // Verify that the new contents are valid.
        assert(new_contents != nullptr);
        if (new_contents != nullptr)
        {
            // Add the new contents, and make it visible.
            source_view_stack_->layout()->addWidget(new_contents);
            new_contents->show();

            // Use the active view as the focus proxy for the source view stack.
            source_view_stack_->setFocusProxy(new_contents);

            // Set focus to the new contents.
            new_contents->setFocus();
        }
    }
}

bool RgBuildView::ShowRemoveFileConfirmation(const std::string& message_string, const std::string& full_path)
{
    bool is_removed = false;

    if (!full_path.empty())
    {
        // Ask the user if they're sure they want to remove the file.
        is_removed = RgUtils::ShowConfirmationMessageBox(kStrMenuBarConfirmRemoveFileDialogTitle, message_string.c_str(), this);

        // Ask the user if we should save the changes. Continue only if the user did not ask to cancel the operation.
        is_removed = is_removed && (RequestSaveFile(full_path) != RgUnsavedItemsDialog::kCancel);
    }

    return is_removed;
}

void RgBuildView::HandleFindTriggered()
{
    // Toggle to show the find widget.
    ToggleFindWidgetVisibility(true);
}

void RgBuildView::HandleIsBuildInProgressChanged(bool is_building)
{
    is_build_in_progress_ = is_building;
}

void RgBuildView::HandleProjectBuildSuccess()
{
    // Reset the project building flag.
    HandleIsBuildInProgressChanged(false);

    // Update the last successful build time to now.
    last_successful_build_time_ = QDateTime::currentDateTime();

    RgMenu* menu = GetMenu();
    assert(menu != nullptr);
    if (menu != nullptr)
    {
        CurrentBuildSucceeded();
    }

    // The current project was built successfully. Open the disassembly view with the results.
    bool is_disassembly_loaded = LoadDisassemblyFromBuildOutput();
    assert(is_disassembly_loaded);
    if (is_disassembly_loaded)
    {
        // Switch to the Source Code view and show the disassembly view.
        if (menu != nullptr)
        {
            menu->DeselectItems();
            menu->SwitchToLastSelectedItem();
        }
        ShowCurrentFileDisassembly();
    }

    assert(current_code_editor_ != nullptr);
    if (current_code_editor_ != nullptr)
    {
        // Use the currently-selected line in the source editor to highlight correlated lines in the disassembly table.
        int selected_line_number = current_code_editor_->GetSelectedLineNumber();
        HandleSourceFileSelectedLineChanged(current_code_editor_, selected_line_number);
    }

    // Resize the disassembly view.
    HandleDisassemblyTableWidthResizeRequested(0);
    // Then maximize the size of the disassembly view for Binary Analysis mode.
    if (disassembly_view_container_ != nullptr && RgConfigManager::Instance().GetCurrentAPI() == RgProjectAPI::kBinary)
    {
        disassembly_view_container_->SetIsMaximizable(true);
        disassembly_view_container_->SwitchContainerSize();
        disassembly_view_container_->SetIsMaximizable(false);
    }

    // Update the notification message if needed.
    UpdateApplicationNotificationMessage();
}

void RgBuildView::HandleApplicationStateChanged(Qt::ApplicationState state)
{
    // When the application becomes active, check for external file modifications.
    if (state == Qt::ApplicationActive)
    {
        CheckExternalFileModification();
    }
}

void RgBuildView::HandleDisassemblyTableWidthResizeRequested(int minimum_width)
{
    assert(disassembly_view_ != nullptr);
    assert(disassembly_view_container_ != nullptr);
    assert(disassembly_view_splitter_ != nullptr);

    // Before resizing the disassembly table, make sure that it is not in maximized state.
    if (disassembly_view_ != nullptr && disassembly_view_container_ != nullptr
        && disassembly_view_splitter_ != nullptr && !disassembly_view_container_->IsInMaximizedState())
    {
        // Add a small portion of extra buffer space to the right side of the table.
        static const float RESIZE_EXTRA_MARGIN = 1.5f;
        minimum_width = static_cast<int>(minimum_width * RESIZE_EXTRA_MARGIN);

        QRect resource_usage_text_bounds;
        disassembly_view_->GetResourceUsageTextBounds(resource_usage_text_bounds);

        // Set maximum width for the widgets containing the ISA disassembly table.
        const int resource_usage_string_width = resource_usage_text_bounds.width();
        const int maximum_width = resource_usage_string_width > minimum_width ? resource_usage_string_width : minimum_width;

        int splitter_width = disassembly_view_splitter_->size().width();
        QList<int> splitter_widths;
        splitter_widths.push_back(splitter_width - maximum_width);
        splitter_widths.push_back(maximum_width);

        // Set the ideal width for both sides of the splitter.
        disassembly_view_splitter_->setSizes(splitter_widths);

        // Save the new sizes.
        SetConfigSplitterPositions();
    }
}

void RgBuildView::HandleBuildSettingsMenuButtonClicked()
{
    OpenBuildSettings();
}

void RgBuildView::ResetView()
{
    // Nullify any previous project object.
    if (project_ != nullptr)
    {
        project_ = nullptr;
    }

    // Clear the view.
    ClearBuildView();
}

void RgBuildView::DestroyBuildOutputsForFile(const std::string& input_file_full_path)
{
    bool is_project_empty = false;

    // Iterate through build output for all target GPUs.
    auto first_target_gpu = build_outputs_.begin();
    auto last_target_gpu = build_outputs_.end();
    for (auto target_gpu_iter = first_target_gpu; target_gpu_iter != last_target_gpu; ++target_gpu_iter)
    {
        std::shared_ptr<RgCliBuildOutput> build_output = target_gpu_iter->second;

        // Search for outputs for the given source file.
        assert(build_output != nullptr);
        if (build_output != nullptr)
        {
            auto input_file_outputs_iter = build_output->per_file_output.find(input_file_full_path);
            if (input_file_outputs_iter != build_output->per_file_output.end())
            {
                // Step through all outputs associated with the given input source file.
                RgFileOutputs& file_outputs = input_file_outputs_iter->second;
                for (const RgEntryOutput& entry_output : file_outputs.outputs)
                {
                    for (const RgOutputItem& outputItem : entry_output.outputs)
                    {
                        // Destroy the output file.
                        QFile::remove(outputItem.file_path.c_str());
                    }
                }

                // Erase the source file's outputs from the existing build output structure.
                build_output->per_file_output.erase(input_file_outputs_iter);
            }

            // Is this the last file being removed from the build output structure?
            if (build_output->per_file_output.empty())
            {
                is_project_empty = true;
            }
        }
    }

    // Destroy all remnants of previous builds of the project.
    if (is_project_empty)
    {
        DestroyProjectBuildArtifacts();
    }
}

void RgBuildView::RemoveEditor(const std::string& filename, bool switch_to_next_file)
{
    // Attempt to find the editor instance used to display the given file.
    RgSourceCodeEditor* editor = nullptr;
    auto editor_iter = source_code_editors_.find(filename);
    if (editor_iter != source_code_editors_.end())
    {
        editor = editor_iter->second;
    }

    assert(editor != nullptr);

    // Remove the editor from the map, and hide it from the interface.
    QWidget* title_bar = source_view_container_->GetTitleBar();
    RgSourceEditorTitlebar* source_view_title_bar = qobject_cast<RgSourceEditorTitlebar*>(title_bar);
    if (source_view_title_bar != nullptr)
    {
        source_view_title_bar->SetTitlebarContentsVisibility(false);
    }

    source_code_editors_.erase(editor_iter);

    if (editor == current_code_editor_)
    {
        // There is no more "Current Editor," because it is being closed.
        current_code_editor_->hide();
        current_code_editor_ = nullptr;
    }

    // Destroy the editor associated with the file that was closed.
    editor->deleteLater();

    if (switch_to_next_file)
    {
        SwitchToFirstRemainingFile();
    }
}

void RgBuildView::RemoveInputFile(const std::string& input_file_full_path)
{
    RgConfigManager& config_manager = RgConfigManager::Instance();

    // Remove the file from the project.
    config_manager.RemoveSourceFilePath(project_, clone_index_, input_file_full_path);
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
        if (disassembly_view_->IsEmpty())
        {
            // Restore disassembly_view_container_ in Binary mode.
            if (disassembly_view_container_ != nullptr && config_manager.GetCurrentAPI() == RgProjectAPI::kBinary)
            {
                disassembly_view_container_->SetIsMaximizable(true);
            }

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
}

bool RgBuildView::LoadDisassemblyFromBuildOutput()
{
    bool result = false;

    if (disassembly_view_container_ == nullptr || disassembly_view_ == nullptr)
    {
        // Create the ISA disassembly view.
        CreateIsaDisassemblyView();

        // Connect the disassembly view signals.
        result = ConnectDisassemblyViewSignals();
    }

    assert(disassembly_view_ != nullptr);
    if (disassembly_view_ != nullptr)
    {
        // Clear all previously loaded build output.
        disassembly_view_->ClearBuildOutput();

        assert(project_ != nullptr);
        if (project_ != nullptr)
        {
            // Ensure that the incoming clone index is valid for the current project.
            bool is_valid_range = (clone_index_ >= 0 && clone_index_ < project_->clones.size());
            assert(is_valid_range);

            if (is_valid_range)
            {
                // Ensure that the clone is valid before creating the RgBuildSettingsView.
                std::shared_ptr<RgProjectClone> clone = project_->clones[clone_index_];

                // Verify that the clone exists.
                assert(clone != nullptr);
                if (clone != nullptr)
                {
                    // Populate the disassembly view with the build output.
                    result = disassembly_view_->PopulateBuildOutput(clone, build_outputs_);
                }
            }
        }
    }

    return result;
}

void RgBuildView::DestroyProjectBuildArtifacts()
{
    // Navigate to the current clone's build output and destroy all artifacts.
    std::string project_build_output_path = CreateProjectBuildOutputPath();

    // Destroy all build artifacts in the project's output directory.
    QDir output_dir(project_build_output_path.c_str());
    output_dir.removeRecursively();

    // Clear references to outputs from previous project compilations.
    build_outputs_.clear();

    if (disassembly_view_ != nullptr)
    {
        // Clear any disassembly tables already loaded into the view.
        disassembly_view_->ClearBuildOutput();

        // Hide the disassembly view.
        ToggleDisassemblyViewVisibility(false);
    }
}

std::string RgBuildView::GetFilepathForEditor(const RgSourceCodeEditor* editor)
{
    // Return an empty string by default.
    std::string ret = "";

    auto editor_iter = source_code_editors_.begin();
    while (editor_iter != source_code_editors_.end())
    {
        // Return the filename if a match is found.
        if (editor_iter->second == editor)
        {
            ret = editor_iter->first;
            break;
        }

        editor_iter++;
    }

    return ret;
}

void RgBuildView::HandleNewCLIOutputString(const std::string& cli_output_string)
{
    // Send the CLI's output text to the output window.
    cli_output_window_->EmitSetText(cli_output_string.c_str());
}

bool RgBuildView::IsLineCorrelationEnabled(RgSourceCodeEditor* source_editor)
{
    bool is_correlation_enabled = false;

    assert(project_ != nullptr);
    if (project_ != nullptr)
    {
        // Ensure that the clone index is valid.
        bool is_valid_clone_index = clone_index_ >= 0 && clone_index_ < project_->clones.size();
        assert(is_valid_clone_index);
        if (is_valid_clone_index)
        {
            auto first_file = project_->clones[clone_index_]->source_files.begin();
            auto last_file = project_->clones[clone_index_]->source_files.end();

            // Search the list of source file info for the one that matches the given editor.
            std::string file_path = GetFilepathForEditor(source_editor);
            RgSourceFilePathSearcher path_searcher(file_path);
            auto file_iter = std::find_if(first_file, last_file, path_searcher);
            assert(file_iter != last_file);
            if (file_iter != last_file)
            {
                // Update the correlation state for the file.
                is_correlation_enabled = file_iter->is_correlated;
            }
        }
    }

    return is_correlation_enabled;
}

std::string RgBuildView::CreateProjectBuildOutputPath() const
{
    std::string result_path;

    // Build an output path where all of the build artifacts will be dumped to.
    std::string project_directory;
    bool is_ok = RgUtils::ExtractFileDirectory(project_->project_file_full_path, project_directory);
    assert(is_ok);
    if (is_ok)
    {
        std::string output_folder_path;
        is_ok = RgUtils::AppendFolderToPath(project_directory, kStrOutputFolderName, output_folder_path);
        assert(is_ok);
        if (is_ok)
        {
            result_path = output_folder_path;
        }
    }

    return result_path;
}

bool RgBuildView::SwitchToEditor(RgSourceCodeEditor* editor)
{
    bool ret = false;

    assert(editor != nullptr);
    if (editor != nullptr)
    {
        // Verify if the user is allowed to switch to source editing mode.
        bool is_switching_allowed = CanSwitchEditMode();
        if (is_switching_allowed)
        {
            // Switch to the new editor.
            SetViewContentsWidget(editor);

            if (current_code_editor_ != nullptr)
            {
                bool old_editor_is_modified = current_code_editor_->document()->isModified();
                bool new_editor_is_modified = editor->document()->isModified();

                // Check if the new editor has a different modification state then the old one
                if (old_editor_is_modified != new_editor_is_modified)
                {
                    emit CurrentEditorModificationStateChanged(new_editor_is_modified);
                }
            }

            // The editor being switched to is now the current editor.
            current_code_editor_ = editor;

            // Update the editor context.
            UpdateSourceEditorSearchContext();

            // Update the title bar text.
            const std::string& title_bar_text = editor->GetTitleBarText();
            if (!title_bar_text.empty())
            {
                source_editor_titlebar_->ShowMessage(title_bar_text);
            }
            else
            {
                source_editor_titlebar_->SetTitlebarContentsVisibility(false);
            }

            // The editor isn't empty, and will switch to displaying source code.
            SwitchEditMode(EditMode::kSourceCode);

            // Check if the editor file has been modified externally.
            CheckExternalFileModification();

            ret = true;
        }
    }

    return ret;
}

void RgBuildView::UpdateFindWidgetViewAttachment(QWidget* view, bool is_visible)
{
    assert(view != nullptr);
    if (view != nullptr)
    {
        if (find_widget_ != nullptr)
        {
            QLayout* layout = static_cast<QVBoxLayout*>(view->layout());
            assert(layout != nullptr);
            if (layout != nullptr)
            {
                if (is_visible)
                {
                    // Only make the widget visible if the source editor is currently visible.
                    bool is_editor_visible = view->isVisible();
                    if (is_editor_visible)
                    {
                        find_widget_->setParent(view);
                        // Insert the find widget into the top of the layout above the source editor.
                        find_widget_->setVisible(is_visible);
                        find_widget_->raise();

                        // Update the position of the find widget.
                        UpdateFindWidgetGeometry();
                    }
                }
                else
                {
                    layout->removeWidget(find_widget_);
                    find_widget_->setVisible(is_visible);
                }
            }
        }
    }
}

void RgBuildView::UpdateSourceEditorTitlebar(RgSourceCodeEditor* code_editor)
{
    std::string source_file_path = GetFilepathForEditor(code_editor);
    if (!source_file_path.empty())
    {
        // If the source file has already been disassembled, check if line correlation is currently enabled.
        bool is_correlation_enabled = false;
        bool is_disassembled = IsGcnDisassemblyGenerated(source_file_path);
        if (is_disassembled)
        {
            is_correlation_enabled = IsLineCorrelationEnabled(code_editor);
        }
        else
        {
            // The file hasn't been disassembled yet, so don't display a warning in the editor's titlebar.
            is_correlation_enabled = true;
        }

        // Update the title bar to show the current correlation state for the file.
        assert(source_editor_titlebar_ != nullptr);
        if (source_editor_titlebar_ != nullptr && IsLineCorrelationSupported())
        {
            source_editor_titlebar_->SetIsCorrelationEnabled(is_correlation_enabled);
        }
    }
}

bool RgBuildView::CanSwitchEditMode()
{
    bool ret = true;

    // If the user is currently viewing the Build Settings, ask them to save before changing the edit mode.
    if (edit_mode_ == EditMode::kBuildSettings)
    {
        // Require the user to decide whether or not to save their Build Settings changes.
        ret = ShowSaveDialog(RgFilesToSave::kBuildSettings);
    }

    return ret;
}

void RgBuildView::SwitchEditMode(EditMode mode)
{
    if (edit_mode_ != mode)
    {
        if (mode == EditMode::kEmpty)
        {
            SetViewContentsWidget(empty_panel_);
        }
        else
        {
            // Based on the incoming mode, hide/show specific widgets living within the BuildView.
            switch (mode)
            {
            case EditMode::kSourceCode:
            {
                // Enable maximizing the source editor/build settings container.
                assert(source_view_container_ != nullptr);
                if (source_view_container_ != nullptr)
                {
                    source_view_container_->SetIsMaximizable(true);
                }

                // Hide the build settings, and show the code editor.
                if (current_code_editor_ != nullptr)
                {
                    // Set the code editor instance in the view.
                    SetViewContentsWidget(current_code_editor_);
                }

                // Set the appropriate boolean in RgViewManager
                // to facilitate focusing the correct widget.
                view_manager_->SetIsSourceViewCurrent(true);
            }
            break;
            case EditMode::kBuildSettings:
            {
                // Disable maximizing the source editor/build settings container.
                assert(source_view_container_ != nullptr);
                if (source_view_container_ != nullptr)
                {
                    source_view_container_->SetIsMaximizable(false);
                }

                assert(source_editor_titlebar_ != nullptr);
                if (source_editor_titlebar_ != nullptr)
                {
                    // Hide the Source Code Editor titlebar message.
                    source_editor_titlebar_->SetTitlebarContentsVisibility(false);
                }

                if (disassembly_view_splitter_ != nullptr)
                {
                    // Is there a container that's currently maximized in the splitter?
                    QWidget* maximized_widget = disassembly_view_splitter_->GetMaximizedWidget();
                    if (maximized_widget != nullptr)
                    {
                        // Restore the maximized view before switching to the build settings.
                        disassembly_view_splitter_->Restore();
                    }
                }

                // Switch from showing the code editor, to showing the build settings.
                SetViewContentsWidget(build_settings_widget_);

                // Set the build settings frame border color.
                SetAPISpecificBorderColor();

                // If the disassembly view exists, hide it.
                if (disassembly_view_container_ != nullptr)
                {
                    ToggleDisassemblyViewVisibility(false);
                }

                // Initialize focus.
                if (build_settings_view_ != nullptr)
                {
                    build_settings_view_->SetInitialWidgetFocus();
                }

                // Set the appropriate booleans in RgViewManager
                // to facilitate focusing the correct widget.
                view_manager_->SetIsSourceViewCurrent(false);
                view_manager_->SetIsPsoEditorViewCurrent(false);
                view_manager_->SetIsBuildSettingsViewCurrent(true);
            }
            break;
            default:
                // Invoke the mode-specific edit mode switch handler.
                HandleModeSpecificEditMode(mode);
                view_manager_->SetIsSourceViewCurrent(false);
                view_manager_->SetIsPsoEditorViewCurrent(true);
                view_manager_->SetIsBuildSettingsViewCurrent(false);
                break;
            }

            // Update the file menu save text and action.
            emit EditModeChanged(mode);
        }

        // Update the current mode.
        edit_mode_ = mode;
    }
}

void RgBuildView::SwitchToFirstRemainingFile()
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
        SwitchEditMode(EditMode::kSourceCode);
    }
    else
    {
        // When the last file has been removed, the editor is in the empty state.
        SwitchEditMode(EditMode::kEmpty);
    }
}

void RgBuildView::CreateBuildSettingsView()
{
    assert(project_ != nullptr);
    if (project_ != nullptr)
    {
        // Ensure that the incoming clone index is valid for the current project.
        bool is_valid_range = (clone_index_ >= 0 && clone_index_ < project_->clones.size());
        assert(is_valid_range);

        if (is_valid_range)
        {
            // Ensure that the clone is valid before creating the RgBuildSettingsView.
            std::shared_ptr<RgProjectClone> clone = project_->clones[clone_index_];

            // Verify that the clone exists.
            assert(clone != nullptr);
            if (clone != nullptr)
            {
                assert(factory_ != nullptr);
                if (factory_ != nullptr)
                {
                    // Create the build settings interface.
                    build_settings_view_ = factory_->CreateBuildSettingsView(this, clone->build_settings, false);
                    [[maybe_unused]] RgProjectAPI current_api = RgConfigManager::Instance().GetCurrentAPI();
                    assert(build_settings_view_ != nullptr || (current_api == RgProjectAPI::kBinary && build_settings_view_ == nullptr));

                    // If the build settings view was created successfully, connect the signals.
                    if (build_settings_view_ != nullptr)
                    {
                        // Create the widget.
                        build_settings_widget_ = new RgBuildSettingsWidget(this);
                        build_settings_widget_->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
                        build_settings_widget_->setFrameStyle(QFrame::Box);
                        build_settings_widget_->setLayout(new QVBoxLayout);

                        // Create the Settings buttons view.
                        settings_buttons_view_ = new RgSettingsButtonsView(this);

                        // Hide the Restore Default Settings button from the project build settings.
                        settings_buttons_view_->HideRestoreDefaultSettingsButton(true);

                        // Create a scroll area to contain the build settings view.
                        RgScrollArea* scroll_area = new RgScrollArea(this);
                        scroll_area->setObjectName(kStrBuildViewSettingsScrollarea);
                        scroll_area->setStyleSheet(kStrBuildViewSettingsScrollareaStylesheet);
                        scroll_area->setFrameShape(QFrame::NoFrame);
                        scroll_area->setFocusPolicy(Qt::FocusPolicy::NoFocus);
                        scroll_area->setWidget(build_settings_view_);
                        scroll_area->setWidgetResizable(true);

                        // Connect the scroll area click to set the border to the API-specific color.
                        [[maybe_unused]] bool is_connected =
                            connect(scroll_area, &RgScrollArea::ScrollAreaClickedEvent, this, &RgBuildView::SetAPISpecificBorderColor);
                        assert(is_connected);

                        // Add various widgets to this tab.
                        build_settings_widget_->layout()->addWidget(scroll_area);
                        build_settings_widget_->layout()->addWidget(settings_buttons_view_);

                        // Hide the build settings view after creating it.
                        build_settings_widget_->hide();

                        // Connect signals for the build settings view.
                        ConnectBuildSettingsSignals();
                    }
                }
            }
        }
    }
}

void RgBuildView::CreateFindWidget()
{
    // Verify that this gets invoked only once.
    assert(find_widget_ == nullptr);
    if (find_widget_ == nullptr)
    {
        // Create the find widget, register with the scaling manager.
        find_widget_ = new RgFindTextWidget(this);

        // The find widget is hidden by default.
        find_widget_->hide();

        // Connect the find widget signals.
        ConnectFindSignals();

        // Create the source code searcher interface.
        source_searcher_ = new RgSourceEditorSearcher();
    }
}

void RgBuildView::CreateIsaDisassemblyView()
{
    // Create a factory matching the API mode.
    RgProjectAPI current_api = RgConfigManager::Instance().GetCurrentAPI();
    std::shared_ptr<RgFactory> factory = RgFactory::CreateFactory(current_api);
    assert(factory != nullptr);
    if (factory != nullptr)
    {
        RgIsaDisassemblyView* disassembly_view = factory->CreateDisassemblyView(this);
        assert(disassembly_view != nullptr);
        if (disassembly_view != nullptr)
        {
            disassembly_view_ = disassembly_view;

            // Wrap the disassembly view in a view container.
            disassembly_view_container_ = new RgViewContainer();
            disassembly_view_container_->SetMainWidget(disassembly_view_);

            // Connect the container single click signal to disassembly view focus in handler.
            [[maybe_unused]] bool is_connected = connect(disassembly_view_container_,
                                                         &RgViewContainer::ViewContainerMouseClickEventSignal,
                                                         disassembly_view_,
                &RgIsaDisassemblyView::HandleDisassemblyTabViewClicked);
            assert(is_connected);

            // Set the object name for the disassembly view container.
            disassembly_view_container_->setObjectName(kStrRgIsaDisassemblyViewContainer);

            // Add the disassembly view to the disassembly splitter.
            disassembly_view_splitter_->AddMaximizableWidget(disassembly_view_container_);

            // Hide the disassembly view when it is first created.
            disassembly_view_->setVisible(true);

            // Add the view to the view manager in the disassembly view position so the tabbing order is correct.
            view_manager_->AddView(disassembly_view_container_, true, static_cast<int>(RgViewManagerViewContainerIndex::kDisassemblyView));
        }
    }
}

bool RgBuildView::GetInputFileOutputs(std::shared_ptr<RgCliBuildOutput> build_outputs, InputFileToBuildOutputsMap& outputs) const
{
    bool ret = false;

    assert(build_outputs != nullptr);
    if (build_outputs != nullptr)
    {
        outputs = build_outputs->per_file_output;
        ret = true;
    }

    return ret;
}

RgSourceCodeEditor* RgBuildView::GetEditorForFilepath(const std::string& full_file_path, RgSrcLanguage lang)
{
    // The source code editor to use for the given filename.
    RgSourceCodeEditor* editor = nullptr;

    if (!full_file_path.empty())
    {
        auto editor_iter = source_code_editors_.find(full_file_path);
        if (editor_iter != source_code_editors_.end())
        {
            editor = editor_iter->second;
        }
        else
        {
            editor = new RgSourceCodeEditor(this, lang);
            source_code_editors_[full_file_path] = editor;

            // Connect to the specific editor's signals.
            [[maybe_unused]] bool is_connected =
                connect(editor, &RgSourceCodeEditor::OpenHeaderFileRequested, this, &RgBuildView::HandleSourceEditorOpenHeaderRequest);
            assert(is_connected);

            QFileInfo file_info(full_file_path.c_str());
            file_modified_time_map_[editor] = file_info.lastModified();

            ConnectSourcecodeEditorSignals(editor);

            // Set the fonts for the source code editor.
            RgConfigManager& config_manager = RgConfigManager::Instance();
            std::shared_ptr<RgGlobalSettings> global_settings = config_manager.GetGlobalConfig();
            assert(global_settings != nullptr);
            if (global_settings != nullptr)
            {
                QFont font;
                font.setFamily(QString::fromStdString(global_settings->font_family));
                font.setPointSize(global_settings->font_size);
                assert(editor != nullptr);
                if (editor != nullptr)
                {
                    editor->setFont(font);
                }
            }
        }
    }
    return editor;
}

void RgBuildView::CancelCurrentBuild()
{
    // Destroy all project outputs when the project build is canceled.
    DestroyProjectBuildArtifacts();

    // Reset the "is currently building" flag.
    HandleIsBuildInProgressChanged(false);

    // Signal that this build should be canceled
    // (this handle is being watched by the thread that launches the CLI).
    cancel_bulid_signal_ = true;

    CurrentBuildCancelled();
}

void RgBuildView::FocusOnSourceCodeEditor()
{
    // Switch the focus to the file editor to begin editing the file.
    if (current_code_editor_ != nullptr)
    {
        current_code_editor_->setFocus();
    }
}

void RgBuildView::HandleScrollCodeEditorToLine(int lineNum)
{
    current_code_editor_->ScrollToLine(lineNum);
    QTextCursor cursor(current_code_editor_->document()->findBlockByLineNumber(lineNum - 1));
    current_code_editor_->setTextCursor(cursor);

    // Switch focus to the code editor.
    current_code_editor_->setFocus();
}

void RgBuildView::RenameFile(const std::string& old_file_path, const std::string& new_file_path)
{
    auto editor_iter = source_code_editors_.find(old_file_path);
    if (editor_iter != source_code_editors_.end())
    {
        RgSourceCodeEditor* editor = editor_iter->second;

        // Erase the existing file path, and insert the new one.
        source_code_editors_.erase(editor_iter);
        source_code_editors_[new_file_path] = editor;
    }

    // Update the project's source file list with the new filepath.
    RgConfigManager::Instance().UpdateSourceFilepath(old_file_path, new_file_path, project_, clone_index_);

    // Save the updated project file.
    RgConfigManager::Instance().SaveProjectFile(project_);
}

void RgBuildView::RenameProject(const std::string& full_path)
{
    // Cache the original file path being renamed.
    assert(project_ != nullptr);
    if (project_ != nullptr)
    {
        std::string original_file_path = project_->project_file_full_path;

        // Rename the project config file.
        bool is_renamed = QFile::rename(project_->project_file_full_path.c_str(), full_path.c_str());
        assert(is_renamed);

        if (is_renamed)
        {
            // Set full path.
            project_->project_file_full_path = full_path;

            // Set project name.
            std::string filename;
            RgUtils::ExtractFileName(full_path, filename, false);
            project_->project_name = filename;
            RgConfigManager& config_manager = RgConfigManager::Instance();

            // Update the recent project list to reference the new path.
            config_manager.UpdateRecentProjectPath(original_file_path, full_path, project_->api);

            // Save the project file.
            config_manager.SaveProjectFile(project_);

            // Update main window title text.
            emit ProjectLoaded(project_);
        }
    }
}

void RgBuildView::ToggleFindWidgetVisibility(bool is_visible)
{
    if (edit_mode_ == EditMode::kSourceCode)
    {
        // Attach the Find widget to the source editor view.
        UpdateFindWidgetViewAttachment(source_view_stack_, is_visible);

        assert(find_widget_ != nullptr);
        if (find_widget_ != nullptr)
        {
            if (is_visible)
            {
                find_widget_->SetFocused();
            }
        }
    }
}

void RgBuildView::SaveEditorTextToFile(RgSourceCodeEditor* editor, const std::string& full_path)
{
    bool is_editor_valid = (editor != nullptr);
    assert(is_editor_valid);

    // Only save if the editor is modified.
    if (is_editor_valid && editor->document()->isModified())
    {
        [[maybe_unused]] bool is_save_successful = RgUtils::WriteTextFile(full_path, editor->toPlainText().toStdString());
        assert(is_save_successful);

        // Remember most recent time file was modified.
        QFileInfo file_info(full_path.c_str());
        file_modified_time_map_[editor] = file_info.lastModified();

        // Indicate that a freshly saved file is considered unmodified.
        editor->document()->setModified(false);
    }
}

void RgBuildView::DiscardEditorChanges(RgSourceCodeEditor* editor)
{
    bool isEditorValid = (editor != nullptr);
    assert(isEditorValid);
    if (isEditorValid)
    {
        editor->document()->setModified(false);
    }
}

void RgBuildView::UpdateFindWidgetGeometry()
{
    assert(find_widget_ != nullptr);
    if (find_widget_ != nullptr)
    {
        if (current_code_editor_ != nullptr)
        {
            // Compute the geometry for the widget relative to the source editor.
            int scrollbar_width = current_code_editor_->verticalScrollBar()->width();
            if (!current_code_editor_->verticalScrollBar()->isVisible())
            {
                scrollbar_width = 0;
            }

            // Start the Find widget at the far left of the attached control, and shift it to the
            // right as far as possible within the parent.
            int widget_horizontal_location = 0;

            // The total amount of horizontal space available for the Find widget to fit into.
            int available_editor_width = current_code_editor_->width() - scrollbar_width;

            // Try to display the find widget with the maximum dimensions that can fit within the source editor.
            int find_widget_width = find_widget_->maximumWidth();
            if (find_widget_width > available_editor_width)
            {
                find_widget_width = available_editor_width;
            }
            else
            {
                widget_horizontal_location += (available_editor_width - find_widget_width - kFindTextWidgetHorizontalMargin);
                assert(widget_horizontal_location >= 0);
                if (widget_horizontal_location < 0)
                {
                    widget_horizontal_location = 0;
                }
            }

            // Use the unmodified height of the Find Widget in the final dimension.
            int find_widget_height = find_widget_->maximumHeight();

            // Set the geometry for the widget manually.
            // Offset vertically so the widget doesn't overlap the editor's titlebar.
            find_widget_->setGeometry(widget_horizontal_location, kFindTextWidgetVerticalMargin, find_widget_width, find_widget_height);
        }
    }
}

void RgBuildView::UpdateSourceFileCorrelationState(const std::string& file_path, bool is_correlated)
{
    assert(project_ != nullptr);
    if (project_ != nullptr)
    {
        bool is_valid_clone_index = clone_index_ >= 0 && clone_index_ < project_->clones.size();
        assert(is_valid_clone_index);
        if (is_valid_clone_index)
        {
            auto first_file = project_->clones[clone_index_]->source_files.begin();
            auto last_file = project_->clones[clone_index_]->source_files.end();

            // Search for the given source file in the project's list of source files.
            RgSourceFilePathSearcher path_searcher(file_path);
            auto file_iter = std::find_if(first_file, last_file, path_searcher);
            if (file_iter != last_file)
            {
                // Update the correlation state for the file.
                file_iter->is_correlated = is_correlated;

                // Save the project file each time the state is changed.
                RgConfigManager::Instance().SaveProjectFile(project_);
            }
        }
    }
}

void RgBuildView::UpdateSourceEditorSearchContext()
{
    // Update the find widget's searcher to search the new editor.
    assert(source_searcher_ != nullptr);
    assert(find_widget_ != nullptr);
    if (source_searcher_ != nullptr && find_widget_ != nullptr)
    {
        // Update the FindWidget's search context to use the source editor searcher.
        find_widget_->SetSearchContext(source_searcher_);
        source_searcher_->SetTargetEditor(current_code_editor_);
    }
}

void RgBuildView::CheckExternalFileModification()
{
    // If there are no active code editors, no files can be modified.
    if (current_code_editor_ != nullptr)
    {
        // Get file modification time from the last time the file was saved in RGA.
        QDateTime expected_last_modified = file_modified_time_map_[current_code_editor_];

        // Get file info for the editor file.
        std::string filename = GetFilepathForEditor(current_code_editor_);
        QFileInfo file_info(filename.c_str());

        // If the modification time is not the same as remembered, the file has been changed externally.
        if (file_info.lastModified() != expected_last_modified)
        {
            HandleExternalFileModification(file_info);
        }
    }
}

void RgBuildView::HandleExternalFileModification(const QFileInfo& file_info)
{
    std::string modified_filename = file_info.filePath().toStdString();
    bool is_file_exists = RgUtils::IsFileExists(modified_filename);
    if (is_file_exists)
    {
        // Notify other components in the system that this file has been modified outside the app.
        emit CurrentFileModified();

        QString message_text = QString(modified_filename.c_str()) + "\n\n" + kStrReloadFileDialogText;

        // Show message box to ask if the user want to reload the file.
        int response = QMessageBox::question(this, kStrReloadFileDialogTitle, message_text, QMessageBox::Yes, QMessageBox::No);

        // Get user response.
        switch (response)
        {
        case QMessageBox::Yes:
            ReloadFile(modified_filename);
            break;
        case QMessageBox::No:
            file_modified_time_map_[current_code_editor_] = file_info.lastModified();

            // Indicate the document is unsaved, regardless of any previous state.
            current_code_editor_->document()->setModified(true);
            break;
        default:
            // Should never get here.
            assert(false);
        }
    }
    else
    {
        QString message_text = QString(modified_filename.c_str()) + "\n\n" + kStrCreateFileDialogText + "\n" + kStrRemoveFileDialogText;

        // Show message box to ask if the user want to sve the file.
        int response = QMessageBox::question(this, kStrCreateFileDialogTitle, message_text, QMessageBox::Yes, QMessageBox::No);

        // Get user response.
        switch (response)
        {
        case QMessageBox::Yes:
            current_code_editor_->document()->setModified(true);
            // Save the file to disk.
            SaveSourceFile(modified_filename);
            break;
        case QMessageBox::No:
            // Remove the input file from the RgBuildView.
            RemoveInputFile(modified_filename);
            break;
        default:
            // Should never get here.
            assert(false);
        }
    }
}

void RgBuildView::SetConfigSplitterPositions()
{
    RgConfigManager& config_manager = RgConfigManager::Instance();

    if (output_splitter_)
    {
        // Build output splitter.
        config_manager.SetSplitterValues(kStrSplitterNameBuildOutput, output_splitter_->ToStdVector());
    }

    if (disassembly_view_splitter_)
    {
        // Disassembly/source view splitter.
        config_manager.SetSplitterValues(kStrSplitterNameSourceDisassembly, disassembly_view_splitter_->ToStdVector()); 
    }
}

void RgBuildView::RestoreViewLayout()
{
    RgConfigManager& config_manager = RgConfigManager::Instance();
    std::vector<int> splitter_values;

    // Build output splitter.
    bool has_splitter_values = config_manager.GetSplitterValues(kStrSplitterNameBuildOutput, splitter_values);
    if (has_splitter_values)
    {
        output_splitter_->setSizes(QVector<int>(splitter_values.cbegin(), splitter_values.cend()).toList());
    }

    // Disassembly/source view splitter.
    has_splitter_values = config_manager.GetSplitterValues(kStrSplitterNameSourceDisassembly, splitter_values);
    if (has_splitter_values)
    {
        disassembly_view_splitter_->setSizes(QVector<int>(splitter_values.begin(), splitter_values.end()).toList());
    }
}

bool RgBuildView::IsBuildInProgress() const
{
    return is_build_in_progress_;
}

void RgBuildView::GetUnsavedSourceFiles(QStringList& unsaved_source_files)
{
    // Build a list of all unsaved file with modifications.
    auto editor_iter = source_code_editors_.begin();
    while (editor_iter != source_code_editors_.end())
    {
        RgSourceCodeEditor* editor = editor_iter->second;
        std::string full_path = editor_iter->first;

        bool is_editor_valid = (editor != nullptr);
        assert(is_editor_valid);

        // Add file to the list if it is unsaved (modified).
        if (is_editor_valid && editor->document()->isModified())
        {
            unsaved_source_files << full_path.c_str();
        }

        editor_iter++;
    }
}

bool RgBuildView::OpenIncludeFile(const std::string& full_file_path)
{
    bool ret = false;
    if (RgUtils::IsFileExists(full_file_path))
    {
        // Check if RGA is configured to use the system's default app, or the user's app of choice.
        std::string include_viewer = RgConfigManager::Instance().GetIncludeFileViewer();
        if (include_viewer.compare(kStrGlobalSettingsSrcViewIncludeViewerDefault) == 0)
        {
            // Launch the system's default viewer.
            QUrl file_url = QUrl::fromLocalFile(full_file_path.c_str());
            QDesktopServices::openUrl(file_url);
            ret = true;
        }
        else
        {
            // Launch the user's editor of choice.
            QStringList arg_list;
            arg_list.push_back(full_file_path.c_str());
            ret = QProcess::startDetached(include_viewer.c_str(), arg_list);
        }
    }

    return ret;
}

void RgBuildView::HandleSaveSettingsButtonClicked()
{
    SetAPISpecificBorderColor();

    if (edit_mode_ == EditMode::kSourceCode)
    {
        SaveCurrentFile(EditMode::kSourceCode);
    }
    else if (edit_mode_ == EditMode::kBuildSettings)
    {
        // Disable the "Save" button.
        assert(settings_buttons_view_ != nullptr);
        if (settings_buttons_view_ != nullptr)
        {
            settings_buttons_view_->EnableSaveButton(false);
        }

        build_settings_view_->SaveSettings();
    }
    else if (edit_mode_ == EditMode::kPipelineSettings)
    {
        // Save the PSO editor values.
        SaveCurrentFile(EditMode::kPipelineSettings);
    }
    else
    {
        // Shouldn't get here.
        assert(false);
    }
}

void RgBuildView::HandleRestoreDefaultsSettingsClicked()
{
    SetAPISpecificBorderColor();

    // Ask the user for confirmation.
    bool is_confirmation = RgUtils::ShowConfirmationMessageBox(kStrBuildSettingsDefaultSettingsConfirmationTitle, kStrBuildSettingsDefaultSettingsConfirmation, this);

    if (is_confirmation)
    {
        // Disable the "Save" button.
        assert(settings_buttons_view_ != nullptr);
        if (settings_buttons_view_ != nullptr)
        {
            settings_buttons_view_->EnableSaveButton(false);
        }

        [[maybe_unused]] RgProjectAPI current_api = RgConfigManager::Instance().GetCurrentAPI();
        assert(build_settings_view_ != nullptr || (current_api == RgProjectAPI::kBinary && build_settings_view_ == nullptr));
        if (build_settings_view_ != nullptr)
        {
            build_settings_view_->RestoreDefaultSettings();
        }
    }
}

void RgBuildView::HandleSetFrameBorderGreen()
{
    if (build_settings_widget_ != nullptr)
    {
        build_settings_widget_->setStyleSheet(kStrBuildViewBuildSettingsWidgetStylesheetGreen);
    }
}

void RgBuildView::HandleSetFrameBorderRed()
{
    if (build_settings_widget_ != nullptr)
    {
        build_settings_widget_->setStyleSheet(kStrBuildViewBuildSettingsWidgetStylesheetRed);
    }
}

void RgBuildView::HandleSetFrameBorderBlack()
{
    if (build_settings_widget_ != nullptr)
    {
        build_settings_widget_->setStyleSheet(kStrBuildViewBuildSettingsWidgetStylesheetBlack);
    }
}

void RgBuildView::HandleDisassemblyViewSizeMaximize()
{
    assert(disassembly_view_);
    assert(disassembly_view_container_);

    if (disassembly_view_ != nullptr)
    {
        disassembly_view_->setMaximumWidth(QWIDGETSIZE_MAX);
    }
    if (disassembly_view_container_ != nullptr)
    {
        disassembly_view_container_->setMaximumWidth(QWIDGETSIZE_MAX);
    }
}

void RgBuildView::HandleDisassemblyViewSizeRestore()
{
    HandleDisassemblyTableWidthResizeRequested(0);
}

void RgBuildView::HandleSplitterMoved(int pos, int index)
{
    Q_UNUSED(pos);
    Q_UNUSED(index);

    // Update the splitter dimensions in the config file.
    SetConfigSplitterPositions();
}

std::string RgBuildView::GetCurrentProjectAPIName() const
{
    std::string project_api = "";

    [[maybe_unused]] bool ok = RgUtils::ProjectAPIToString(project_->api, project_api);
    assert(ok);

    return project_api;
}

void RgBuildView::HandleDisassemblyViewClicked()
{
    qApp->focusObjectChanged(disassembly_view_);
    disassembly_view_->setFocus();
}

bool RgBuildView::SaveProjectConfigFile() const
{
    return RgConfigManager::Instance().SaveProjectFile(project_);
}

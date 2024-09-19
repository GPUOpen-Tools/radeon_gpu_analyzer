// C++.
#include <cassert>

// Qt.
#include <QAction>
#include <QMenuBar>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_app_state_opencl.h"
#include "radeon_gpu_analyzer_gui/qt/rg_build_view_opencl.h"
#include "radeon_gpu_analyzer_gui/qt/rg_main_window.h"
#include "radeon_gpu_analyzer_gui/qt/rg_settings_tab.h"
#include "radeon_gpu_analyzer_gui/qt/rg_start_tab_opencl.h"
#include "radeon_gpu_analyzer_gui/qt/rg_status_bar.h"
#include "radeon_gpu_analyzer_gui/qt/rg_menu.h"
#include "radeon_gpu_analyzer_gui/rg_data_types_opencl.h"
#include "radeon_gpu_analyzer_gui/rg_definitions.h"
#include "radeon_gpu_analyzer_gui/rg_factory.h"
#include "radeon_gpu_analyzer_gui/rg_utils.h"
#include "radeon_gpu_analyzer_gui/rg_string_constants.h"

void RgAppStateOpencl::ResetBuildView()
{
    // Destroy the RgBuildView instance. Subsequent project creation or load operations will
    // re-initialize a new RgBuildView.
    RG_SAFE_DELETE(build_view_);

    // Re-create an empty RgBuildView.
    CreateBuildView();
}

void RgAppStateOpencl::ConnectBuildViewSignals(RgBuildView* build_view)
{
    RgBuildViewOpencl* opencl_build_view = static_cast<RgBuildViewOpencl*>(build_view);
    assert(opencl_build_view != nullptr);
    if (opencl_build_view != nullptr)
    {
        // Connect the RgBuildViewOpenCL default menu item's Create button.
        bool is_connected = connect(opencl_build_view, &RgBuildViewOpencl::CreateFileButtonClicked, this, &RgAppStateOpencl::HandleCreateNewCLFile);
        assert(is_connected);

        // Connect the RgBuildViewOpenCL default menu item's Open button.
        is_connected = connect(opencl_build_view, &RgBuildViewOpencl::OpenFileButtonClicked, this, &RgAppStateOpencl::HandleOpenExistingCLFile);
        assert(is_connected);
    }
}

RgStartTab* RgAppStateOpencl::GetStartTab()
{
    return start_tab_;
}

RgBuildView* RgAppStateOpencl::GetBuildView()
{
    return build_view_;
}

void RgAppStateOpencl::SetStartTab(RgStartTab* start_tab)
{
    start_tab_ = static_cast<RgStartTabOpencl*>(start_tab);
}

void RgAppStateOpencl::Cleanup(QMenu* menu_bar)
{
    assert(new_file_action_ != nullptr);
    assert(menu_bar != nullptr);
    if (new_file_action_ != nullptr && menu_bar != nullptr)
    {
        // Remove the OpenCL-specific actions from the file menu.
        menu_bar->removeAction(new_file_action_);
    }
}

void RgAppStateOpencl::HandleProjectBuildStarted()
{
    // Do not allow creating/adding files.
    new_file_action_->setEnabled(false);
    open_file_action_->setEnabled(false);
}

void RgAppStateOpencl::ResetViewStateAfterBuild()
{
    new_file_action_->setEnabled(true);
    open_file_action_->setEnabled(true);
}

void RgAppStateOpencl::HandleCreateNewCLFile()
{
    assert(main_window_ != nullptr);
    if (main_window_ != nullptr)
    {
        // Ask user to save pending settings.
        bool user_canceled_action = false;
        if (settings_tab_ != nullptr)
        {
            user_canceled_action = !settings_tab_->PromptToSavePendingChanges();
        }

        if (!user_canceled_action)
        {
            if (build_view_ != nullptr)
            {
                // Prompt to save items in the build view if needed; including the project build settings.
                user_canceled_action = !build_view_->ShowSaveDialog();
            }

            if (!user_canceled_action)
            {
                // Track if a project was actually created.
                bool was_project_created = false;

                assert(build_view_ != nullptr);
                if (build_view_ != nullptr)
                {
                    main_window_->setAcceptDrops(false);
                    was_project_created = build_view_->CreateNewSourceFileInProject();
                    main_window_->setAcceptDrops(true);
                    if (was_project_created)
                    {
                        // Show the build view as the central widget.
                        main_window_->SwitchToView(RgMainWindow::MainWindowView::kBuildView);

                        // Adjust the menu button focus.
                        RgMenu* menu = build_view_->GetMenu();
                        assert(menu != nullptr);
                        if (menu != nullptr)
                        {
                            menu->SetButtonsNoFocus();
                        }
                    }
                    else
                    {
                        // The project was not created successfully, so clean up the build view.
                        main_window_->DestroyBuildView();
                    }

                    // Restore the layout if no project was created.
                    if (!was_project_created)
                    {
                        // Reset the actions to the default state.
                        main_window_->ResetActionsState();
                    }
                }
            }
        }
    }
}

void RgAppStateOpencl::HandleOpenExistingCLFile()
{
    assert(main_window_ != nullptr);
    if (main_window_ != nullptr)
    {
        // Ask user to save pending settings.
        bool user_canceled_action = false;
        if (settings_tab_ != nullptr)
        {
            user_canceled_action = !settings_tab_->PromptToSavePendingChanges();
        }

        if (!user_canceled_action)
        {
            if (build_view_ != nullptr)
            {
                // Prompt to save items in the build view if needed; including the project build settings.
                user_canceled_action = !build_view_->ShowSaveDialog();
            }

            if (!user_canceled_action)
            {
                // Retrieve the current API from the configuration manager.
                RgProjectAPI current_api = RgConfigManager::Instance().GetCurrentAPI();

                QStringList selected_files;
                bool        is_ok = RgUtils::OpenFileDialogForMultipleFiles(main_window_, current_api, selected_files);
                if (is_ok && !selected_files.empty())
                {
                    main_window_->setAcceptDrops(false);
                    OpenFilesInBuildView(selected_files);
                    main_window_->setAcceptDrops(true);
                }
            }
        }
    }
}

void RgAppStateOpencl::CreateApiSpecificFileActions(QMenu* menu_bar)
{
    new_file_action_ = new QAction(tr(kStrMenuBarCreateNewFileOpencl), this);
    new_file_action_->setShortcuts(QKeySequence::New);
    new_file_action_->setStatusTip(tr(kStrMenuBarCreateNewFileTooltipOpencl));
    bool is_connected = connect(new_file_action_, &QAction::triggered, this, &RgAppStateOpencl::HandleCreateNewCLFile);
    assert(is_connected);
    if (is_connected)
    {
        menu_bar->addAction(new_file_action_);
    }

    open_file_action_ = new QAction(tr(kStrMenuBarOpenExistingFileOpencl), this);
    open_file_action_->setShortcuts(QKeySequence::Open);
    open_file_action_->setStatusTip(tr(kStrMenuBarOpenExistingFileTooltipOpencl));
    is_connected = connect(open_file_action_, &QAction::triggered, this, &RgAppStateOpencl::HandleOpenExistingCLFile);
    assert(is_connected);
    if (is_connected)
    {
        menu_bar->addAction(open_file_action_);
    }
}

void RgAppStateOpencl::CreateBuildView()
{
    // Create a factory matching the new API mode to switch to.
    std::shared_ptr<RgFactory> factory = RgFactory::CreateFactory(RgProjectAPI::kOpenCL);
    assert(factory != nullptr);
    if (factory != nullptr)
    {
        // Instantiate the RgBuildView.
        build_view_ = static_cast<RgBuildViewOpencl*>(factory->CreateBuildView(main_window_));

        // Connect the project created handler so the RgMainWindow can
        // add the new RgBuildView instance to the widget hierarchy.
        [[maybe_unused]] bool is_connected = connect(build_view_, &RgBuildView::ProjectCreated, main_window_, &RgMainWindow::HandleProjectCreated);
        assert(is_connected);
    }
}

void RgAppStateOpencl::ConnectFileMenuActions()
{
    assert(start_tab_ != nullptr);
    if (start_tab_ != nullptr)
    {
        // Connect the "Create new CL file" button in the start page.
        bool is_connected = connect(start_tab_, &RgStartTabOpencl::CreateNewCLFileEvent, this, &RgAppStateOpencl::HandleCreateNewCLFile);
        assert(is_connected);

        // Connect the "Open existing CL file" button in the start page.
        is_connected = connect(start_tab_, &RgStartTabOpencl::OpenExistingFileEvent, this, &RgAppStateOpencl::HandleOpenExistingCLFile);
        assert(is_connected);
    }
}

void RgAppStateOpencl::OpenFilesInBuildView(const QStringList& file_paths)
{
    if (!file_paths.empty())
    {
        assert(build_view_ != nullptr);
        if (build_view_ != nullptr)
        {
            for (const QString& selectedFile : file_paths)
            {
                // Update the project to reference to selected source file.
                bool result = build_view_->AddExistingSourcefileToProject(selectedFile.toStdString());

                // Break out of the for loop if the operation failed.
                if (!result)
                {
                    break;
                }
            }

            // Switch to the build view if there's at least one file being edited.
            if (!build_view_->HasSourceCodeEditors())
            {
                // Show the build view as the central widget.
                main_window_->SwitchToView(RgMainWindow::MainWindowView::kBuildView);
            }
            else
            {
                // The project was not created successfully, so clean up the build view.
                main_window_->DestroyBuildView();
            }
        }
    }
}

void RgAppStateOpencl::GetApplicationStylesheet(std::vector<std::string>& stylesheet_file_names)
{
    stylesheet_file_names.push_back(kStrApplicationStylesheetFileOpencl);
    stylesheet_file_names.push_back(kStrApplicationStylesheetFile);
}

void RgAppStateOpencl::GetMainWindowStylesheet(std::vector<std::string>& stylesheet_file_names)
{
    stylesheet_file_names.push_back(kStrMainWindowStylesheetFileOpencl);
    stylesheet_file_names.push_back(kStrMainWindowStylesheetFile);
}

std::string RgAppStateOpencl::GetGlobalSettingsViewStylesheet() const
{
    const char* kStrOpenclAppSettingsStylesheet =
        "RgGlobalSettingsView #projectNameCheckBox:focus,"
        "RgGlobalSettingsView #projectNameCheckBox:hover,"
        "RgGlobalSettingsView #logFileLocationFolderOpenButton:focus,"
        "RgGlobalSettingsView #logFileLocationFolderOpenButton:hover,"
        "RgGlobalSettingsView #defaultApiOnStartupComboBox:focus,"
        "RgGlobalSettingsView #defaultApiOnStartupComboBox:hover,"
        "RgGlobalSettingsView #columnVisibilityArrowPushButton:focus,"
        "RgGlobalSettingsView #columnVisibilityArrowPushButton:hover,"
        "RgGlobalSettingsView #fontFamilyComboBox:focus,"
        "RgGlobalSettingsView #fontFamilyComboBox:hover,"
        "RgGlobalSettingsView #fontSizeComboBox:focus,"
        "RgGlobalSettingsView #fontSizeComboBox:hover,"
        "RgGlobalSettingsView #logFileLocationLineEdit:focus,"
        "RgGlobalSettingsView #logFileLocationLineEdit:hover,"
        "RgGlobalSettingsView #projectFileLocationLineEdit:focus,"
        "RgGlobalSettingsView #projectFileLocationLineEdit:hover,"
        "RgGlobalSettingsView #includeFilesViewerLineEdit:focus,"
        "RgGlobalSettingsView #includeFilesViewerLineEdit:hover,"
        "RgGlobalSettingsView #includeFilesViewerBrowseButton:focus,"
        "RgGlobalSettingsView #includeFilesViewerBrowseButton:hover,"
        "RgGlobalSettingsView #assocExtGlslLineEdit:focus,"
        "RgGlobalSettingsView #assocExtGlslLineEdit:hover,"
        "RgGlobalSettingsView #assocExtSpvasLineEdit:focus,"
        "RgGlobalSettingsView #assocExtSpvasLineEdit:hover,"
        "RgGlobalSettingsView #assocExtSpvBinaryLineEdit:focus,"
        "RgGlobalSettingsView #assocExtSpvBinaryLineEdit:hover,"
        "RgGlobalSettingsView #assocExtSpvasLabel:focus,"
        "RgGlobalSettingsView #assocExtSpvasLabel:hover,"
        "RgGlobalSettingsView #defaultLangComboBox:focus,"
        "RgGlobalSettingsView #defaultLangComboBox:hover"
        "{"
        "border: 1px solid lightGreen;"
        "background-color: palette(alternate-base);"
        "}";
    return kStrOpenclAppSettingsStylesheet;
}

std::string RgAppStateOpencl::GetBuildSettingsViewStylesheet() const
{
    const char* kStrOpenclBuildSettingsStylesheet =
        "RgBuildSettingsView #doubleAsSingleCheckBox:focus,"
        "RgBuildSettingsView #doubleAsSingleCheckBox:hover,"
        "RgBuildSettingsView #flushDenormalizedCheckBox:focus,"
        "RgBuildSettingsView #flushDenormalizedCheckBox:hover,"
        "RgBuildSettingsView #strictAliasingCheckBox:focus,"
        "RgBuildSettingsView #strictAliasingCheckBox:hover,"
        "RgBuildSettingsView #enableMADCheckBox:focus,"
        "RgBuildSettingsView #enableMADCheckBox:hover,"
        "RgBuildSettingsView #ignoreZeroSignednessCheckBox:focus,"
        "RgBuildSettingsView #ignoreZeroSignednessCheckBox:hover,"
        "RgBuildSettingsView #allowUnsafeOptimizationsCheckBox:focus,"
        "RgBuildSettingsView #allowUnsafeOptimizationsCheckBox:hover,"
        "RgBuildSettingsView #assumeNoNanNorInfiniteCheckBox:focus,"
        "RgBuildSettingsView #assumeNoNanNorInfiniteCheckBox:hover,"
        "RgBuildSettingsView #aggressiveMathOptimizationsCheckBox:focus,"
        "RgBuildSettingsView #aggressiveMathOptimizationsCheckBox:hover,"
        "RgBuildSettingsView #correctlyRoundSinglePrecisionCheckBox:focus,"
        "RgBuildSettingsView #correctlyRoundSinglePrecisionCheckBox:hover,"
        "RgBuildSettingsView #predefinedMacrosLineEdit:focus,"
        "RgBuildSettingsView #predefinedMacrosLineEdit:hover,"
        "RgBuildSettingsView #includeDirectoriesLineEdit:focus,"
        "RgBuildSettingsView #includeDirectoriesLineEdit:hover,"
        "RgBuildSettingsView #compilerBinariesLineEdit:focus,"
        "RgBuildSettingsView #compilerBinariesLineEdit:hover,"
        "RgBuildSettingsView #compilerIncludesLineEdit:focus,"
        "RgBuildSettingsView #compilerIncludesLineEdit:hover,"
        "RgBuildSettingsView #additionalOptionsTextEdit:focus,"
        "RgBuildSettingsView #additionalOptionsTextEdit:hover,"
        "RgBuildSettingsView #allOptionsTextEdit:focus,"
        "RgBuildSettingsView #allOptionsTextEdit:hover,"
        "RgBuildSettingsView #compilerLibrariesLineEdit:focus,"
        "RgBuildSettingsView #compilerLibrariesLineEdit:hover"
        "{"
        "border: 1px solid lightGreen;"
        "background-color: palette(alternate-base);"
        "}";
    return kStrOpenclBuildSettingsStylesheet;
}
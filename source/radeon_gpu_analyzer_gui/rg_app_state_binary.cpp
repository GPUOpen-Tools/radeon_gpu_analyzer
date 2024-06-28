// C++.
#include <cassert>

// Qt.
#include <QAction>
#include <QMenuBar>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_app_state_binary.h"
#include "radeon_gpu_analyzer_gui/qt/rg_build_view_binary.h"
#include "radeon_gpu_analyzer_gui/qt/rg_main_window.h"
#include "radeon_gpu_analyzer_gui/qt/rg_settings_tab.h"
#include "radeon_gpu_analyzer_gui/qt/rg_start_tab_binary.h"
#include "radeon_gpu_analyzer_gui/qt/rg_status_bar.h"
#include "radeon_gpu_analyzer_gui/qt/rg_menu.h"
#include "radeon_gpu_analyzer_gui/rg_data_types_binary.h"
#include "radeon_gpu_analyzer_gui/rg_definitions.h"
#include "radeon_gpu_analyzer_gui/rg_factory.h"
#include "radeon_gpu_analyzer_gui/rg_utils.h"
#include "radeon_gpu_analyzer_gui/rg_string_constants.h"

void RgAppStateBinary::ResetBuildView()
{
    // Destroy the RgBuildView instance. Subsequent project creation or load operations will
    // re-initialize a new RgBuildView.
    RG_SAFE_DELETE(build_view_);

    // Re-create an empty RgBuildView.
    CreateBuildView();
}

void RgAppStateBinary::ConnectBuildViewSignals(RgBuildView* build_view)
{
    RgBuildViewBinary* binary_build_view = static_cast<RgBuildViewBinary*>(build_view);
    assert(binary_build_view != nullptr);
    if (binary_build_view != nullptr)
    {
        // Connect the RgBuildViewBinary default menu item's Load Code object button.
        [[maybe_unused]] bool is_connected =
            connect(binary_build_view, &RgBuildViewBinary::CreateFileButtonClicked, this, &RgAppStateBinary::HandleLoadCodeObjectBinary);
        assert(is_connected);
    }
}

RgStartTab* RgAppStateBinary::GetStartTab()
{
    return start_tab_;
}

RgBuildView* RgAppStateBinary::GetBuildView()
{
    return build_view_;
}

void RgAppStateBinary::SetStartTab(RgStartTab* start_tab)
{
    start_tab_ = static_cast<RgStartTabBinary*>(start_tab);
}

void RgAppStateBinary::Cleanup(QMenu* menu_bar)
{
    assert(open_bin_file_action_ != nullptr);
    assert(menu_bar != nullptr);
    if (open_bin_file_action_ != nullptr && menu_bar != nullptr)
    {
        // Remove the Binary-specific actions from the file menu.
        menu_bar->removeAction(open_bin_file_action_);
    }
}

void RgAppStateBinary::HandleProjectBuildStarted()
{
    // Do not allow adding files.
    open_bin_file_action_->setEnabled(false);
}

void RgAppStateBinary::ResetViewStateAfterBuild()
{
    open_bin_file_action_->setEnabled(true);
}

void RgAppStateBinary::HandleLoadCodeObjectBinary()
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
                std::string file_path_to_add;
                bool         is_ok = RgUtils::OpenFileDialog(main_window_, current_api, file_path_to_add);
                if (is_ok && !file_path_to_add.empty() && RgUtils::IsFileExists(file_path_to_add))
                {
                    main_window_->setAcceptDrops(false);
                    QStringList selected_files(QString::fromStdString(file_path_to_add));
                    OpenFilesInBuildView(selected_files);
                    main_window_->setAcceptDrops(true);
                }
            }
        }
    }
}

void RgAppStateBinary::CreateApiSpecificFileActions(QMenu* menu_bar)
{
    open_bin_file_action_ = new QAction(tr(kStrMenuBarOpenExistingFileBinary), this);
    open_bin_file_action_->setShortcuts(QKeySequence::Open);
    open_bin_file_action_->setStatusTip(tr(kStrMenuBarOpenExistingFileTooltipBinary));
    bool is_connected = connect(open_bin_file_action_, &QAction::triggered, this, &RgAppStateBinary::HandleLoadCodeObjectBinary);
    assert(is_connected);
    if (is_connected)
    {
        menu_bar->addAction(open_bin_file_action_);
    }
}

void RgAppStateBinary::CreateBuildView()
{
    // Create a factory matching the new API mode to switch to.
    std::shared_ptr<RgFactory> factory = RgFactory::CreateFactory(RgProjectAPI::kBinary);
    assert(factory != nullptr);
    if (factory != nullptr)
    {
        // Instantiate the RgBuildView.
        build_view_ = static_cast<RgBuildViewBinary*>(factory->CreateBuildView(main_window_));

        // Connect the project created handler so the RgMainWindow can
        // add the new RgBuildView instance to the widget hierarchy.
        [[maybe_unused]] bool is_connected = connect(build_view_, &RgBuildView::ProjectCreated, main_window_, &RgMainWindow::HandleProjectCreated);
        assert(is_connected);
    }
}

void RgAppStateBinary::ConnectFileMenuActions()
{
    assert(start_tab_ != nullptr);
    if (start_tab_ != nullptr)
    {
        // Connect the "Load Code Object Binary" button in the start page.
        [[maybe_unused]] bool is_connected =
            connect(start_tab_, &RgStartTabBinary::OpenExistingCodeObjFileEvent, this, &RgAppStateBinary::HandleLoadCodeObjectBinary);
        assert(is_connected);
    }
}

void RgAppStateBinary::OpenFilesInBuildView(const QStringList& file_paths)
{
    if (!file_paths.empty())
    {
        assert(build_view_ != nullptr && main_window_ != nullptr);
        if (build_view_ != nullptr && main_window_ != nullptr)
        {
            for (const QString& selectedFile : file_paths)
            {
                // Update the project to reference to selected source file.
                bool result = build_view_->AddExistingCodeObjFileToProject(selectedFile.toStdString());

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

void RgAppStateBinary::GetApplicationStylesheet(std::vector<std::string>& stylesheet_file_names)
{
    stylesheet_file_names.push_back(kStrApplicationStylesheetFileBinary);
    stylesheet_file_names.push_back(kStrApplicationStylesheetFile);
}

void RgAppStateBinary::GetMainWindowStylesheet(std::vector<std::string>& stylesheet_file_names)
{
    stylesheet_file_names.push_back(kStrMainWindowStylesheetFileBinary);
    stylesheet_file_names.push_back(kStrMainWindowStylesheetFile);
}

std::string RgAppStateBinary::GetGlobalSettingsViewStylesheet() const
{
    const char* kStrBinaryAppSettingsStylesheet =
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
        "border: 1px solid rgb(128, 0, 128);"
        "background: rgb(240, 240, 240);"
        "}";
    return kStrBinaryAppSettingsStylesheet;
}

std::string RgAppStateBinary::GetBuildSettingsViewStylesheet() const
{
    const char* kStrBinaryBuildSettingsStylesheet = "";
    return kStrBinaryBuildSettingsStylesheet;
}

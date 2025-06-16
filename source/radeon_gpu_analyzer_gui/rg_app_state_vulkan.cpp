//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for a Vulkan-mode-specific implementation of the RgAppState.
//=============================================================================

// C++.
#include <cassert>

// Qt.
#include <QAction>
#include <QMenu>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_app_state_vulkan.h"
#include "radeon_gpu_analyzer_gui/qt/rg_build_view_vulkan.h"
#include "radeon_gpu_analyzer_gui/qt/rg_settings_tab.h"
#include "radeon_gpu_analyzer_gui/qt/rg_start_tab_vulkan.h"
#include "radeon_gpu_analyzer_gui/qt/rg_status_bar.h"
#include "radeon_gpu_analyzer_gui/qt/rg_main_window.h"
#include "radeon_gpu_analyzer_gui/qt/rg_menu_graphics.h"
#include "radeon_gpu_analyzer_gui/rg_data_types_vulkan.h"
#include "radeon_gpu_analyzer_gui/rg_definitions.h"
#include "radeon_gpu_analyzer_gui/rg_factory_vulkan.h"
#include "radeon_gpu_analyzer_gui/rg_string_constants.h"

void RgAppStateVulkan::ResetBuildView()
{
    // Destroy the existing RgBuildView instance. Subsequent project creation or load operations will
    // re-initialize a new RgBuildView.
    RG_SAFE_DELETE(build_view_);

    // Enable the "Create pipeline" actions in the app menu.
    if (new_graphics_pipeline_action_ != nullptr && new_compute_pipeline_action_ != nullptr)
    {
        new_graphics_pipeline_action_->setEnabled(true);
        new_compute_pipeline_action_->setEnabled(true);
    }

    // Re-create an empty RgBuildView.
    CreateBuildView();
}

void RgAppStateVulkan::ConnectBuildViewSignals(RgBuildView* build_view)
{
    [[maybe_unused]] bool is_connected = connect(build_view, &RgBuildView::ProjectLoaded, this, &RgAppStateVulkan::HandleProjectLoaded);
    assert(is_connected);
}

RgStartTab* RgAppStateVulkan::GetStartTab()
{
    return start_tab_;
}

RgBuildView* RgAppStateVulkan::GetBuildView()
{
    return build_view_;
}

void RgAppStateVulkan::SetStartTab(RgStartTab* start_tab)
{
    start_tab_ = static_cast<RgStartTabVulkan*>(start_tab);
}

void RgAppStateVulkan::Cleanup(QMenu* menu_bar)
{
    assert(new_graphics_pipeline_action_ != nullptr);
    assert(new_compute_pipeline_action_ != nullptr);
    assert(menu_bar != nullptr);
    if (new_graphics_pipeline_action_ != nullptr && new_compute_pipeline_action_ != nullptr && menu_bar != nullptr)
    {
        // Remove the Vulkan-specific actions from the file menu.
        menu_bar->removeAction(new_graphics_pipeline_action_);
        menu_bar->removeAction(new_compute_pipeline_action_);
    }
}

void RgAppStateVulkan::HandleCreateNewGraphicsPipeline()
{
    assert(main_window_ != nullptr);
    if (main_window_ != nullptr)
    {
        // Ask user to save pending settings.
        bool user_canceled_action = false;

        if (!IsInputFileNameBlank())
        {
            if (settings_tab_ != nullptr)
            {
                user_canceled_action = !settings_tab_->PromptToSavePendingChanges();
            }

            if (!user_canceled_action)
            {
                // Track if a project was actually created.
                bool was_project_created = false;

                assert(build_view_ != nullptr);
                if (build_view_ != nullptr)
                {
                    main_window_->setAcceptDrops(false);
                    was_project_created = build_view_->CreateDefaultGraphicsPipeline();
                    main_window_->setAcceptDrops(true);

                    if (was_project_created)
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

void RgAppStateVulkan::HandleCreateNewComputePipeline()
{
    assert(main_window_ != nullptr);
    if (main_window_ != nullptr)
    {
        // Ask user to save pending settings.
        bool user_canceled_action = false;
        if (!IsInputFileNameBlank())
        {
            if (settings_tab_ != nullptr)
            {
                user_canceled_action = !settings_tab_->PromptToSavePendingChanges();
            }

            if (!user_canceled_action)
            {
                // Track if a project was actually created.
                bool was_project_created = false;

                assert(build_view_ != nullptr);
                if (build_view_ != nullptr)
                {
                    main_window_->setAcceptDrops(false);
                    was_project_created = build_view_->CreateDefaultComputePipeline();
                    main_window_->setAcceptDrops(true);

                    if (was_project_created)
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

void RgAppStateVulkan::HandleProjectBuildStarted()
{
}

void RgAppStateVulkan::HandleProjectLoaded(std::shared_ptr<RgProject>)
{
    new_graphics_pipeline_action_->setEnabled(false);
    new_compute_pipeline_action_->setEnabled(false);
}

void RgAppStateVulkan::CreateApiSpecificFileActions(QMenu* menu_bar)
{
    // Create new Graphics PSO action.
    new_graphics_pipeline_action_ = new QAction(tr(kStrMenuBarCreateNewGraphicsPsoVulkan), this);
    new_graphics_pipeline_action_->setShortcut(QKeySequence(gs_ACTION_HOTKEY_NEW_VULKAN_GRAPHICS_PROJECT));
    new_graphics_pipeline_action_->setStatusTip(tr(kStrMenuBarCreateNewGraphicsPsoTooltipVulkan));
    bool is_connected = connect(new_graphics_pipeline_action_, &QAction::triggered, this, &RgAppStateVulkan::HandleCreateNewGraphicsPipeline);
    assert(is_connected);
    if (is_connected)
    {
        menu_bar->addAction(new_graphics_pipeline_action_);
    }

    // Create new Compute PSO action.
    new_compute_pipeline_action_ = new QAction(tr(kStrMenuBarCreateNewComputePsoVulkan), this);
    new_compute_pipeline_action_->setShortcut(QKeySequence(gs_ACTION_HOTKEY_NEW_VULKAN_COMPUTE_PROJECT));
    new_compute_pipeline_action_->setStatusTip(tr(kStrMenuBarCreateNewComputePsoTooltipVulkan));
    is_connected = connect(new_compute_pipeline_action_, &QAction::triggered, this, &RgAppStateVulkan::HandleCreateNewComputePipeline);
    assert(is_connected);
    if (is_connected)
    {
        menu_bar->addAction(new_compute_pipeline_action_);
    }

    new_graphics_pipeline_action_->setEnabled(true);
    new_compute_pipeline_action_->setEnabled(true);
}

void RgAppStateVulkan::CreateBuildView()
{
    // Create a factory matching the new API mode to switch to.
    std::shared_ptr<RgFactory> factory = RgFactory::CreateFactory(RgProjectAPI::kVulkan);
    assert(factory != nullptr);
    if (factory != nullptr)
    {
        // Instantiate the RgBuildView.
        build_view_ = static_cast<RgBuildViewVulkan*>(factory->CreateBuildView(main_window_));

        // Connect the project created handler so the RgMainWindow can
        // add the new RgBuildView instance to the widget hierarchy.
        bool is_connected = connect(build_view_, &RgBuildView::ProjectCreated, main_window_, &RgMainWindow::HandleProjectCreated);
        assert(is_connected);

        // Connect the shortcut hot key signal.
        is_connected = connect(main_window_, &RgMainWindow::HotKeyPressedSignal, build_view_, &RgBuildView::HotKeyPressedSignal);
        assert(is_connected);

        // Connect the enable pipeline state menu item signals.
        is_connected = connect(build_view_, &RgBuildViewVulkan::EnablePipelineMenuItem, this, &RgAppStateVulkan::EnablePipelineMenuItem);
        assert(is_connected);

        // Connect the enable build settings menu item signals.
        is_connected = connect(build_view_, &RgBuildViewVulkan::EnableBuildSettingsMenuItem, this, &RgAppStateVulkan::EnableBuildSettingsMenuItem);
        assert(is_connected);

        // Connect the enable Ctrl+S menu item signals.
        is_connected = connect(build_view_, &RgBuildViewVulkan::EnableSaveSettingsMenuItem, this, &RgAppStateVulkan::EnableSaveSettingsMenuItem);
        assert(is_connected);
    }
}

void RgAppStateVulkan::ConnectFileMenuActions()
{
    assert(start_tab_ != nullptr);
    if (start_tab_ != nullptr)
    {
        // Connect the "Create new graphics pipeline" button in the start page.
        bool is_connected = connect(start_tab_, &RgStartTabVulkan::CreateGraphicsPipelineEvent, this, &RgAppStateVulkan::HandleCreateNewGraphicsPipeline);
        assert(is_connected);

        // Connect the "Create new compute pipeline" button in the start page.
        is_connected = connect(start_tab_, &RgStartTabVulkan::CreateComputePipelineEvent, this, &RgAppStateVulkan::HandleCreateNewComputePipeline);
        assert(is_connected);
    }
}

void RgAppStateVulkan::GetApplicationStylesheet(std::vector<std::string>& stylesheet_file_names)
{
    stylesheet_file_names.push_back(kStrApplicationStylesheetFile);
    stylesheet_file_names.push_back(kStrApplicationStylesheetFileVulkan);
}

void RgAppStateVulkan::GetMainWindowStylesheet(std::vector<std::string>& stylesheet_file_names)
{
    stylesheet_file_names.push_back(kStrMainWindowStylesheetFileVulkan);
    stylesheet_file_names.push_back(kStrMainWindowStylesheetFile);
}

void RgAppStateVulkan::HandlePipelineStateEvent()
{
    // Switch to the pipeline state view.
    RgMenuGraphics* menu = build_view_->GetGraphicsFileMenu();
    assert(menu != nullptr);
    if (menu != nullptr)
    {
        RgMenuPipelineStateItem* pso_item = menu->GetPipelineStateItem();
        build_view_->HandlePipelineStateMenuItemClicked(pso_item);
    }
}

std::string RgAppStateVulkan::GetGlobalSettingsViewStylesheet() const
{
    const char* kStrVulkanAppSettingsStylesheet =
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
        "RgGlobalSettingsView #assocExtSpvBinaryLineEdit:focus,"
        "RgGlobalSettingsView #assocExtSpvBinaryLineEdit:hover,"
        "RgGlobalSettingsView #assocExtSpvasLineEdit:focus,"
        "RgGlobalSettingsView #assocExtSpvasLineEdit:hover,"
        "RgGlobalSettingsView #assocExtSpvasLabel:focus,"
        "RgGlobalSettingsView #assocExtSpvasLabel:hover,"
        "RgGlobalSettingsView #defaultLangComboBox:focus,"
        "RgGlobalSettingsView #defaultLangComboBox:hover"
        "{"
        "border: 1px solid rgb(224, 30, 55);"
        "background-color: palette(alternate-base);"
        "}";
    return kStrVulkanAppSettingsStylesheet;
}

std::string RgAppStateVulkan::GetBuildSettingsViewStylesheet() const
{
    const char* kStrVulkanBuildSettingsStylesheet =
        "RgBuildSettingsView #predefinedMacrosLineEdit:focus,"
        "RgBuildSettingsView #predefinedMacrosLineEdit:hover,"
        "RgBuildSettingsView #includeDirectoriesLineEdit:focus,"
        "RgBuildSettingsView #includeDirectoriesLineEdit:hover,"
        "RgBuildSettingsView #ICDLocationLineEdit:focus,"
        "RgBuildSettingsView #ICDLocationLineEdit:hover,"
        "RgBuildSettingsView #glslangOptionsLineEdit:focus,"
        "RgBuildSettingsView #glslangOptionsLineEdit:hover,"
        "RgBuildSettingsView #compilerBinariesLineEdit:focus,"
        "RgBuildSettingsView #compilerBinariesLineEdit:hover,"
        "RgBuildSettingsView #compilerIncludesLineEdit:focus,"
        "RgBuildSettingsView #compilerIncludesLineEdit:hover,"
        "RgBuildSettingsView #compilerLibrariesLineEdit:focus,"
        "RgBuildSettingsView #compilerLibrariesLineEdit:hover,"
        "RgBuildSettingsView #enableValidationLayersCheckBox:focus,"
        "RgBuildSettingsView #enableValidationLayersCheckBox:hover,"
        "RgBuildSettingsView #allOptionsTextEdit:focus,"
        "RgBuildSettingsView #allOptionsTextEdit:hover,"
        "RgBuildSettingsView #outputFileBinaryNameLineEdit:focus,"
        "RgBuildSettingsView #outputFileBinaryNameLineEdit:hover"
        "{"
        "border: 1px solid rgb(224, 30, 55);"
        "background-color: palette(alternate-base);"
        "}";
    return kStrVulkanBuildSettingsStylesheet;
}

void RgAppStateVulkan::OpenFilesInBuildView(const QStringList& file_paths)
{
    Q_UNUSED(file_paths);
}

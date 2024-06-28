// C++.
#include <cassert>

// Local.
#include "radeon_gpu_analyzer_gui/rg_string_constants.h"
#include "radeon_gpu_analyzer_gui/qt/rg_build_view_graphics.h"
#include "radeon_gpu_analyzer_gui/qt/rg_find_text_widget.h"
#include "radeon_gpu_analyzer_gui/qt/rg_maximize_splitter.h"
#include "radeon_gpu_analyzer_gui/qt/rg_menu_graphics.h"
#include "radeon_gpu_analyzer_gui/qt/rg_pipeline_state_model.h"
#include "radeon_gpu_analyzer_gui/qt/rg_pipeline_state_view.h"
#include "radeon_gpu_analyzer_gui/qt/rg_source_code_editor.h"
#include "radeon_gpu_analyzer_gui/qt/rg_source_editor_titlebar.h"
#include "radeon_gpu_analyzer_gui/qt/rg_view_container.h"
#include "radeon_gpu_analyzer_gui/rg_pipeline_state_searcher.h"
#include "radeon_gpu_analyzer_gui/rg_utils.h"

RgBuildViewGraphics::RgBuildViewGraphics(RgProjectAPI api, QWidget* parent)
    : RgBuildView(api, parent)
    , pipeline_state_index_(0),
    add_create_context_menu_(new QMenu(this)),
    action_add_existing_file_(new QAction(kStrAddExistingFile, this)),
    action_create_new_file_(new QAction(kStrCreateNewFile, this))
{
    add_create_context_menu_->setCursor(Qt::PointingHandCursor);
    add_create_context_menu_->addAction(action_create_new_file_);
    add_create_context_menu_->addAction(action_add_existing_file_);
}

void RgBuildViewGraphics::HandlePipelineStateSettingsMenuButtonClicked()
{
    OpenPipelineStateSettingsView();
}

void RgBuildViewGraphics::HandleModeSpecificEditMode(EditMode new_mode)
{
    // The RgBuildViewGraphics is able to switch to the PipelineSettings page.
    if (new_mode == EditMode::kPipelineSettings)
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

        assert(disassembly_view_splitter_ != nullptr);
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

        // Switch to showing the Pipeline State editor.
        assert(pso_editor_frame_ != nullptr);
        assert(pipeline_state_view_ != nullptr);
        if (pso_editor_frame_ != nullptr && pipeline_state_view_ != nullptr)
        {
            // If the FindWidget exists, update its search context.
            if (pso_find_widget_ != nullptr)
            {
                RgPipelineStateSearcher* pso_tree_searcher = pipeline_state_view_->GetSearcher();
                assert(pso_tree_searcher != nullptr);
                if (pso_tree_searcher != nullptr)
                {
                    pso_find_widget_->SetSearchContext(pso_tree_searcher);
                }
            }

            // Switch to the pipeline state view.
            SetViewContentsWidget(pso_editor_frame_);
        }

        // If the disassembly view exists, hide it.
        if (disassembly_view_container_ != nullptr)
        {
            ToggleDisassemblyViewVisibility(false);
        }

        // Initialize focus in the PSO editor.
        if (pipeline_state_view_ != nullptr)
        {
            pipeline_state_view_->SetInitialWidgetFocus();
        }
    }
}

void RgBuildViewGraphics::ConnectPsoFindSignals()
{
    // Connect the find widget's close toggle handler.
    [[maybe_unused]] bool is_connected =
        connect(pso_find_widget_, &RgFindTextWidget::CloseWidgetSignal, this, &RgBuildViewGraphics::HandleFindWidgetVisibilityToggled);
    assert(is_connected);
}

void RgBuildViewGraphics::HandleFindWidgetVisibilityToggled()
{
    if (pso_find_widget_ != nullptr)
    {
        bool is_visible = pso_find_widget_->isVisible();
        ToggleFindWidgetVisibility(!is_visible);
    }
}

void RgBuildViewGraphics::HandleLoadPipelineStateFile(const std::string& file_path)
{
    // Change the mouse cursor to hour glass cursor.
    QApplication::setOverrideCursor(Qt::WaitCursor);

    // Load the pipeline state file.
    bool is_ok = LoadPipelineStateFile(file_path);

    assert(is_ok);
    if (is_ok)
    {
        // Update the find widget to use the new pipeline searcher context.
        assert(pso_find_widget_ != nullptr);
        if (pso_find_widget_ != nullptr)
        {
            RgPipelineStateSearcher* pso_tree_searcher = pipeline_state_view_->GetSearcher();
            assert(pso_tree_searcher != nullptr);
            if (pso_tree_searcher != nullptr)
            {
                pso_find_widget_->SetSearchContext(pso_tree_searcher);
            }
        }

        // Save the project file with the updated pipeline state file path.
        RgConfigManager& config_manager = RgConfigManager::Instance();
        config_manager.SaveProjectFile(project_);

        // Extract directory from full path.
        std::string file_directory;
        is_ok = RgUtils::ExtractFileDirectory(file_path, file_directory);
        assert(is_ok);

        if (is_ok)
        {
            // Update last selected directory in global config.
            RgConfigManager::Instance().SetLastSelectedDirectory(file_directory);
        }

        // Report status to status bar.
        emit SetStatusBarText(kStrPipelineStateEditorFileLoadedSuccessfully, kStatusBarNotificationTimeoutMs);
    }
    else
    {
        // Report status to status bar.
        emit SetStatusBarText(kStrPipelineStateEditorFileFailedLoading, kStatusBarNotificationTimeoutMs);
    }

    // Change the mouse cursor back.
    QApplication::restoreOverrideCursor();
}

void RgBuildViewGraphics::ToggleFindWidgetVisibility(bool is_visible)
{
    if (edit_mode_ == EditMode::kPipelineSettings)
    {
        assert(pipeline_state_view_ != nullptr);
        if (pipeline_state_view_ != nullptr)
        {
            assert(pso_find_widget_ != nullptr);
            if (pso_find_widget_ != nullptr)
            {
                if (is_visible)
                {
                    // Get the currently-selected text in the pipeline state tree.
                    std::string search_string;
                    if (pipeline_state_view_->GetSelectedText(search_string))
                    {
                        // Insert the currently-selected text into the find widget.
                        pso_find_widget_->SetSearchString(search_string);
                    }

                    // Show the text find widget.
                    pso_find_widget_->show();

                    // Focus on the find widget after making it visible.
                    pso_find_widget_->SetFocused();
                }
                else
                {
                    // Hide the text find widget.
                    pso_find_widget_->hide();

                    // Switch focus back to the pipeline state editor.
                    pipeline_state_view_->setFocus();
                }
            }
        }
    }
    else
    {
        // Invoke the base implementation to handle the find widget finding in other edit modes.
        RgBuildView::ToggleFindWidgetVisibility(is_visible);
    }
}

void RgBuildViewGraphics::UpdateFindWidgetGeometry()
{
    if (edit_mode_ != EditMode::kPipelineSettings)
    {
        // Invoke the base implementation to handle other edit modes.
        RgBuildView::UpdateFindWidgetGeometry();
    }
}

void RgBuildViewGraphics::OpenPipelineStateSettingsView()
{
    SwitchEditMode(EditMode::kPipelineSettings);
}

void RgBuildViewGraphics::HandleStateEditorHidden()
{
    // Hide the find widget.
    ToggleFindWidgetVisibility(false);
}

void RgBuildViewGraphics::HandleStateEditorResized()
{
    // Update the geometry for the find widget within the editor view.
    if (pso_find_widget_ != nullptr)
    {
        UpdateFindWidgetGeometry();
    }
}

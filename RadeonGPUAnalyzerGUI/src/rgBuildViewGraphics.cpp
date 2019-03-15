// C++.
#include <cassert>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/rgStringConstants.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgBuildViewGraphics.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgFindTextWidget.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMaximizeSplitter.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMenuGraphics.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgPipelineStateModel.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgPipelineStateView.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgSourceCodeEditor.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgSourceEditorTitlebar.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgViewContainer.h>
#include <RadeonGPUAnalyzerGUI/Include/rgPipelineStateSearcher.h>
#include <RadeonGPUAnalyzerGUI/Include/rgUtils.h>

rgBuildViewGraphics::rgBuildViewGraphics(rgProjectAPI api, QWidget* pParent)
    : rgBuildView(api, pParent)
    , m_pipelineStateIndex(0),
    m_pAddCreateContextMenu(new QMenu(this)),
    m_pActionAddExistingFile(new QAction(STR_ADD_EXISTING_FILE, this)),
    m_pActionCreateNewFile(new QAction(STR_CREATE_NEW_FILE, this))
{
    m_pAddCreateContextMenu->setCursor(Qt::PointingHandCursor);
    m_pAddCreateContextMenu->addAction(m_pActionCreateNewFile);
    m_pAddCreateContextMenu->addAction(m_pActionAddExistingFile);
}

void rgBuildViewGraphics::HandlePipelineStateSettingsMenuButtonClicked()
{
    OpenPipelineStateSettingsView();
}

void rgBuildViewGraphics::HandleModeSpecificEditMode(EditMode newMode)
{
    // The rgBuildViewGraphics is able to switch to the PipelineSettings page.
    if (newMode == EditMode::PipelineSettings)
    {
        // Disable maximizing the source editor/build settings container.
        assert(m_pSourceViewContainer != nullptr);
        if (m_pSourceViewContainer != nullptr)
        {
            m_pSourceViewContainer->SetIsMaximizable(false);
        }

        assert(m_pSourceEditorTitlebar != nullptr);
        if (m_pSourceEditorTitlebar != nullptr)
        {
            // Hide the Source Code Editor titlebar message.
            m_pSourceEditorTitlebar->SetTitlebarContentsVisibility(false);
        }

        assert(m_pDisassemblyViewSplitter != nullptr);
        if (m_pDisassemblyViewSplitter != nullptr)
        {
            // Is there a container that's currently maximized in the splitter?
            QWidget* pMaximizedWidget = m_pDisassemblyViewSplitter->GetMaximizedWidget();
            if (pMaximizedWidget != nullptr)
            {
                // Restore the maximized view before switching to the build settings.
                m_pDisassemblyViewSplitter->Restore();
            }
        }

        // Switch to showing the Pipeline State editor.
        assert(m_pPsoEditorFrame != nullptr);
        assert(m_pPipelineStateView != nullptr);
        if (m_pPsoEditorFrame != nullptr && m_pPipelineStateView != nullptr)
        {
            // If the FindWidget exists, update its search context.
            if (m_pPsoFindWidget != nullptr)
            {
                rgPipelineStateSearcher* pPsoTreeSearcher = m_pPipelineStateView->GetSearcher();
                assert(pPsoTreeSearcher != nullptr);
                if (pPsoTreeSearcher != nullptr)
                {
                    m_pPsoFindWidget->SetSearchContext(pPsoTreeSearcher);
                }
            }

            // Switch to the pipeline state view.
            SetViewContentsWidget(m_pPsoEditorFrame);
        }

        // If the disassembly view exists, hide it.
        if (m_pDisassemblyViewContainer != nullptr)
        {
            ToggleDisassemblyViewVisibility(false);
        }

        // Initialize focus in the PSO editor.
        if (m_pPipelineStateView != nullptr)
        {
            m_pPipelineStateView->SetInitialWidgetFocus();
        }
    }
}

void rgBuildViewGraphics::ConnectPSOFindSignals()
{
    // Connect the find widget's close toggle handler.
    bool isConnected = connect(m_pPsoFindWidget, &rgFindTextWidget::CloseWidgetSignal, this, &rgBuildViewGraphics::HandleFindWidgetVisibilityToggled);
    assert(isConnected);
}

void rgBuildViewGraphics::HandleFindWidgetVisibilityToggled()
{
    if (m_pPsoFindWidget != nullptr)
    {
        bool isVisible = m_pPsoFindWidget->isVisible();
        ToggleFindWidgetVisibility(!isVisible);
    }
}

void rgBuildViewGraphics::HandleLoadPipelineStateFile(const std::string& filePath)
{
    // Change the mouse cursor to hour glass cursor.
    QApplication::setOverrideCursor(Qt::WaitCursor);

    // Load the pipeline state file.
    bool isOk = LoadPipelineStateFile(filePath);

    assert(isOk);
    if (isOk)
    {
        // Update the find widget to use the new pipeline searcher context.
        assert(m_pPsoFindWidget != nullptr);
        if (m_pPsoFindWidget != nullptr)
        {
            rgPipelineStateSearcher* pPsoTreeSearcher = m_pPipelineStateView->GetSearcher();
            assert(pPsoTreeSearcher != nullptr);
            if (pPsoTreeSearcher != nullptr)
            {
                m_pPsoFindWidget->SetSearchContext(pPsoTreeSearcher);
            }
        }

        // Save the project file with the updated pipeline state file path.
        rgConfigManager& configManager = rgConfigManager::Instance();
        configManager.SaveProjectFile(m_pProject);

        // Extract directory from full path.
        std::string fileDirectory;
        bool isOk = rgUtils::ExtractFileDirectory(filePath, fileDirectory);
        assert(isOk);

        if (isOk)
        {
            // Update last selected directory in global config.
            rgConfigManager::Instance().SetLastSelectedDirectory(fileDirectory);
        }

        // Report status to status bar.
        emit SetStatusBarText(STR_PIPELINE_STATE_EDITOR_FILE_LOADED_SUCCESSFULLY, gs_STATUS_BAR_NOTIFICATION_TIMEOUT_MS);
    }
    else
    {
        // Report status to status bar.
        emit SetStatusBarText(STR_PIPELINE_STATE_EDITOR_FILE_FAILED_LOADING, gs_STATUS_BAR_NOTIFICATION_TIMEOUT_MS);
    }

    // Change the mouse cursor back.
    QApplication::restoreOverrideCursor();
}

void rgBuildViewGraphics::ToggleFindWidgetVisibility(bool isVisible)
{
    if (m_editMode == EditMode::PipelineSettings)
    {
        assert(m_pPipelineStateView != nullptr);
        if (m_pPipelineStateView != nullptr)
        {
            assert(m_pPsoFindWidget != nullptr);
            if (m_pPsoFindWidget != nullptr)
            {
                if (isVisible)
                {
                    // Get the currently-selected text in the pipeline state tree.
                    std::string searchString;
                    if (m_pPipelineStateView->GetSelectedText(searchString))
                    {
                        // Insert the currently-selected text into the find widget.
                        m_pPsoFindWidget->SetSearchString(searchString);
                    }

                    // Show the text find widget.
                    m_pPsoFindWidget->show();

                    // Focus on the find widget after making it visible.
                    m_pPsoFindWidget->SetFocused();
                }
                else
                {
                    // Hide the text find widget.
                    m_pPsoFindWidget->hide();

                    // Switch focus back to the pipeline state editor.
                    m_pPipelineStateView->setFocus();
                }
            }
        }
    }
    else
    {
        // Invoke the base implementation to handle the find widget finding in other edit modes.
        rgBuildView::ToggleFindWidgetVisibility(isVisible);
    }
}

void rgBuildViewGraphics::UpdateFindWidgetGeometry()
{
    if (m_editMode != EditMode::PipelineSettings)
    {
        // Invoke the base implementation to handle other edit modes.
        rgBuildView::UpdateFindWidgetGeometry();
    }
}

void rgBuildViewGraphics::OpenPipelineStateSettingsView()
{
    SwitchEditMode(EditMode::PipelineSettings);
}

void rgBuildViewGraphics::HandleStateEditorHidden()
{
    // Hide the find widget.
    ToggleFindWidgetVisibility(false);
}

void rgBuildViewGraphics::HandleStateEditorResized()
{
    // Update the geometry for the find widget within the editor view.
    if (m_pPsoFindWidget != nullptr)
    {
        UpdateFindWidgetGeometry();
    }
}
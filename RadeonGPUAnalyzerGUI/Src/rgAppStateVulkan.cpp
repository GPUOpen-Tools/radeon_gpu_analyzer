// C++.
#include <cassert>

// Qt.
#include <QAction>
#include <QMenu>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgAppStateVulkan.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgBuildViewVulkan.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgSettingsTab.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgStartTabVulkan.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgStatusBar.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMainWindow.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMenuGraphics.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDataTypesVulkan.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDefinitions.h>
#include <RadeonGPUAnalyzerGUI/Include/rgFactoryVulkan.h>
#include <RadeonGPUAnalyzerGUI/Include/rgStringConstants.h>

void rgAppStateVulkan::ResetBuildView()
{
    // Destroy the existing rgBuildView instance. Subsequent project creation or load operations will
    // re-initialize a new rgBuildView.
    RG_SAFE_DELETE(m_pBuildView);

    // Enable the "Create pipeline" actions in the app menu.
    if (m_pNewGraphicsPipelineAction != nullptr && m_pNewComputePipelineAction != nullptr)
    {
        m_pNewGraphicsPipelineAction->setEnabled(true);
        m_pNewComputePipelineAction->setEnabled(true);
    }

    // Re-create an empty rgBuildView.
    CreateBuildView();
}

void rgAppStateVulkan::ConnectBuildViewSignals(rgBuildView* pBuildView)
{
    bool isConnected = connect(pBuildView, &rgBuildView::ProjectLoaded, this, &rgAppStateVulkan::HandleProjectLoaded);
    assert(isConnected);
}

rgStartTab* rgAppStateVulkan::GetStartTab()
{
    return m_pStartTab;
}

rgBuildView* rgAppStateVulkan::GetBuildView()
{
    return m_pBuildView;
}

void rgAppStateVulkan::SetStartTab(rgStartTab* pStartTab)
{
    m_pStartTab = static_cast<rgStartTabVulkan*>(pStartTab);
}

void rgAppStateVulkan::Cleanup(QMenu* pMenubar)
{
    assert(m_pNewGraphicsPipelineAction != nullptr);
    assert(m_pNewComputePipelineAction != nullptr);
    assert(pMenubar != nullptr);
    if (m_pNewGraphicsPipelineAction != nullptr && m_pNewComputePipelineAction != nullptr && pMenubar != nullptr)
    {
        // Remove the Vulkan-specific actions from the file menu.
        pMenubar->removeAction(m_pNewGraphicsPipelineAction);
        pMenubar->removeAction(m_pNewComputePipelineAction);
    }
}

void rgAppStateVulkan::HandleCreateNewGraphicsPipeline()
{
    assert(m_pMainWindow != nullptr);
    if (m_pMainWindow != nullptr)
    {
        // Ask user to save pending settings.
        bool userCanceledAction = false;

        if (!IsInputFileNameBlank())
        {
            if (m_pSettingsTab != nullptr)
            {
                userCanceledAction = !m_pSettingsTab->PromptToSavePendingChanges();
            }

            if (!userCanceledAction)
            {
                // Track if a project was actually created.
                bool wasProjectCreated = false;

                assert(m_pBuildView != nullptr);
                if (m_pBuildView != nullptr)
                {
                    m_pMainWindow->setAcceptDrops(false);
                    wasProjectCreated = m_pBuildView->CreateDefaultGraphicsPipeline();
                    m_pMainWindow->setAcceptDrops(true);

                    if (wasProjectCreated)
                    {
                        // Show the build view as the central widget.
                        m_pMainWindow->SwitchToView(rgMainWindow::MainWindowView::BuildView);
                    }
                    else
                    {
                        // The project was not created successfully, so clean up the build view.
                        m_pMainWindow->DestroyBuildView();
                    }
                }

                // Restore the layout if no project was created.
                if (!wasProjectCreated)
                {
                    // Reset the actions to the default state.
                    m_pMainWindow->ResetActionsState();
                }
            }
        }
    }
}

void rgAppStateVulkan::HandleCreateNewComputePipeline()
{
    assert(m_pMainWindow != nullptr);
    if (m_pMainWindow != nullptr)
    {
        // Ask user to save pending settings.
        bool userCanceledAction = false;
        if (!IsInputFileNameBlank())
        {
            if (m_pSettingsTab != nullptr)
            {
                userCanceledAction = !m_pSettingsTab->PromptToSavePendingChanges();
            }

            if (!userCanceledAction)
            {
                // Track if a project was actually created.
                bool wasProjectCreated = false;

                assert(m_pBuildView != nullptr);
                if (m_pBuildView != nullptr)
                {
                    m_pMainWindow->setAcceptDrops(false);
                    wasProjectCreated = m_pBuildView->CreateDefaultComputePipeline();
                    m_pMainWindow->setAcceptDrops(true);

                    if (wasProjectCreated)
                    {
                        // Show the build view as the central widget.
                        m_pMainWindow->SwitchToView(rgMainWindow::MainWindowView::BuildView);
                    }
                    else
                    {
                        // The project was not created successfully, so clean up the build view.
                        m_pMainWindow->DestroyBuildView();
                    }
                }

                // Restore the layout if no project was created.
                if (!wasProjectCreated)
                {
                    // Reset the actions to the default state.
                    m_pMainWindow->ResetActionsState();
                }
            }
        }
    }
}

void rgAppStateVulkan::HandleProjectBuildStarted()
{
}

void rgAppStateVulkan::HandleProjectLoaded(std::shared_ptr<rgProject>)
{
    m_pNewGraphicsPipelineAction->setEnabled(false);
    m_pNewComputePipelineAction->setEnabled(false);
}

void rgAppStateVulkan::CreateApiSpecificFileActions(QMenu* pMenubar)
{
    // Create new Graphics PSO action.
    m_pNewGraphicsPipelineAction = new QAction(tr(STR_MENU_BAR_CREATE_NEW_GRAPHICS_PSO_VULKAN), this);
    m_pNewGraphicsPipelineAction->setShortcut(QKeySequence(gs_ACTION_HOTKEY_NEW_VULKAN_GRAPHICS_PROJECT));
    m_pNewGraphicsPipelineAction->setStatusTip(tr(STR_MENU_BAR_CREATE_NEW_GRAPHICS_PSO_TOOLTIP_VULKAN));
    bool isConnected = connect(m_pNewGraphicsPipelineAction, &QAction::triggered, this, &rgAppStateVulkan::HandleCreateNewGraphicsPipeline);
    assert(isConnected);
    if (isConnected)
    {
        pMenubar->addAction(m_pNewGraphicsPipelineAction);
    }

    // Create new Compute PSO action.
    m_pNewComputePipelineAction = new QAction(tr(STR_MENU_BAR_CREATE_NEW_COMPUTE_PSO_VULKAN), this);
    m_pNewComputePipelineAction->setShortcut(QKeySequence(gs_ACTION_HOTKEY_NEW_VULKAN_COMPUTE_PROJECT));
    m_pNewComputePipelineAction->setStatusTip(tr(STR_MENU_BAR_CREATE_NEW_COMPUTE_PSO_TOOLTIP_VULKAN));
    isConnected = connect(m_pNewComputePipelineAction, &QAction::triggered, this, &rgAppStateVulkan::HandleCreateNewComputePipeline);
    assert(isConnected);
    if (isConnected)
    {
        pMenubar->addAction(m_pNewComputePipelineAction);
    }

    m_pNewGraphicsPipelineAction->setEnabled(true);
    m_pNewComputePipelineAction->setEnabled(true);
}

void rgAppStateVulkan::CreateBuildView()
{
    // Create a factory matching the new API mode to switch to.
    std::shared_ptr<rgFactory> pFactory = rgFactory::CreateFactory(rgProjectAPI::Vulkan);
    assert(pFactory != nullptr);
    if (pFactory != nullptr)
    {
        // Instantiate the rgBuildView.
        m_pBuildView = static_cast<rgBuildViewVulkan*>(pFactory->CreateBuildView(m_pMainWindow));

        // Connect the project created handler so the rgMainWindow can
        // add the new rgBuildView instance to the widget hierarchy.
        bool isConnected = connect(m_pBuildView, &rgBuildView::ProjectCreated, m_pMainWindow, &rgMainWindow::HandleProjectCreated);
        assert(isConnected);

        // Connect the shortcut hot key signal.
        isConnected = connect(m_pMainWindow, &rgMainWindow::HotKeyPressedSignal, m_pBuildView, &rgBuildView::HotKeyPressedSignal);
        assert(isConnected);
    }
}

void rgAppStateVulkan::ConnectFileMenuActions()
{
    assert(m_pStartTab != nullptr);
    if (m_pStartTab != nullptr)
    {
        // Connect the "Create new graphics pipeline" button in the start page.
        bool isConnected = connect(m_pStartTab, &rgStartTabVulkan::CreateGraphicsPipelineEvent, this, &rgAppStateVulkan::HandleCreateNewGraphicsPipeline);
        assert(isConnected);

        // Connect the "Create new compute pipeline" button in the start page.
        isConnected =  connect(m_pStartTab, &rgStartTabVulkan::CreateComputePipelineEvent, this, &rgAppStateVulkan::HandleCreateNewComputePipeline);
        assert(isConnected);
    }
}

void rgAppStateVulkan::GetApplicationStylesheet(std::vector<std::string>& stylesheetFileNames)
{
    stylesheetFileNames.push_back(STR_APPLICATION_STYLESHEET_FILE);
    stylesheetFileNames.push_back(STR_APPLICATION_STYLESHEET_FILE_VULKAN);
}

void rgAppStateVulkan::GetMainWindowStylesheet(std::vector<std::string>& stylesheetFileNames)
{
    stylesheetFileNames.push_back(STR_MAIN_WINDOW_STYLESHEET_FILE_VULKAN);
    stylesheetFileNames.push_back(STR_MAIN_WINDOW_STYLESHEET_FILE);
}

void rgAppStateVulkan::HandlePipelineStateEvent()
{
    // Switch to the pipeline state view.
    rgMenuGraphics* pMenu = m_pBuildView->GetGraphicsFileMenu();
    assert(pMenu != nullptr);
    if (pMenu != nullptr)
    {
        rgMenuPipelineStateItem* pPsoItem = pMenu->GetPipelineStateItem();
        m_pBuildView->HandlePipelineStateMenuItemClicked(pPsoItem);
    }
}

std::string rgAppStateVulkan::GetGlobalSettingsViewStylesheet() const
{
    const char* STR_VULKAN_APP_SETTINGS_STYLESHEET =
        "rgGlobalSettingsView #projectNameCheckBox:focus,"
        "rgGlobalSettingsView #projectNameCheckBox:hover,"
        "rgGlobalSettingsView #logFileLocationFolderOpenButton:focus,"
        "rgGlobalSettingsView #logFileLocationFolderOpenButton:hover,"
        "rgGlobalSettingsView #defaultApiOnStartupComboBox:focus,"
        "rgGlobalSettingsView #defaultApiOnStartupComboBox:hover,"
        "rgGlobalSettingsView #columnVisibilityArrowPushButton:focus,"
        "rgGlobalSettingsView #columnVisibilityArrowPushButton:hover,"
        "rgGlobalSettingsView #fontFamilyComboBox:focus,"
        "rgGlobalSettingsView #fontFamilyComboBox:hover,"
        "rgGlobalSettingsView #fontSizeComboBox:focus,"
        "rgGlobalSettingsView #fontSizeComboBox:hover,"
        "rgGlobalSettingsView #logFileLocationLineEdit:focus,"
        "rgGlobalSettingsView #logFileLocationLineEdit:hover,"
        "rgGlobalSettingsView #includeFilesViewerLineEdit:focus,"
        "rgGlobalSettingsView #includeFilesViewerLineEdit:hover,"
        "rgGlobalSettingsView #includeFilesViewerBrowseButton:focus,"
        "rgGlobalSettingsView #includeFilesViewerBrowseButton:hover,"
        "rgGlobalSettingsView #assocExtGlslLineEdit:focus,"
        "rgGlobalSettingsView #assocExtGlslLineEdit:hover,"
        "rgGlobalSettingsView #assocExtSpvBinaryLineEdit:focus,"
        "rgGlobalSettingsView #assocExtSpvBinaryLineEdit:hover,"
        "rgGlobalSettingsView #assocExtSpvasLineEdit:focus,"
        "rgGlobalSettingsView #assocExtSpvasLineEdit:hover,"
        "rgGlobalSettingsView #assocExtSpvasLabel:focus,"
        "rgGlobalSettingsView #assocExtSpvasLabel:hover,"
        "rgGlobalSettingsView #defaultLangComboBox:focus,"
        "rgGlobalSettingsView #defaultLangComboBox:hover"
        "{"
        "border: 1px solid rgb(224, 30, 55);"
        "background: rgb(240, 240, 240);"
        "}";
    return STR_VULKAN_APP_SETTINGS_STYLESHEET;
}

std::string rgAppStateVulkan::GetBuildSettingsViewStylesheet() const
{
    const char* STR_VULKAN_BUILD_SETTINGS_STYLESHEET =
        "rgBuildSettingsView #predefinedMacrosLineEdit:focus,"
        "rgBuildSettingsView #predefinedMacrosLineEdit:hover,"
        "rgBuildSettingsView #includeDirectoriesLineEdit:focus,"
        "rgBuildSettingsView #includeDirectoriesLineEdit:hover,"
        "rgBuildSettingsView #ICDLocationLineEdit:focus,"
        "rgBuildSettingsView #ICDLocationLineEdit:hover,"
        "rgBuildSettingsView #glslangOptionsLineEdit:focus,"
        "rgBuildSettingsView #glslangOptionsLineEdit:hover,"
        "rgBuildSettingsView #compilerBinariesLineEdit:focus,"
        "rgBuildSettingsView #compilerBinariesLineEdit:hover,"
        "rgBuildSettingsView #compilerIncludesLineEdit:focus,"
        "rgBuildSettingsView #compilerIncludesLineEdit:hover,"
        "rgBuildSettingsView #compilerLibrariesLineEdit:focus,"
        "rgBuildSettingsView #compilerLibrariesLineEdit:hover,"
        "rgBuildSettingsView #enableValidationLayersCheckBox:focus,"
        "rgBuildSettingsView #enableValidationLayersCheckBox:hover,"
        "rgBuildSettingsView #outputFileBinaryNameLineEdit:focus,"
        "rgBuildSettingsView #outputFileBinaryNameLineEdit:hover"
        "{"
        "border: 1px solid rgb(224, 30, 55);"
        "background: rgb(240, 240, 240);"
        "}";
    return STR_VULKAN_BUILD_SETTINGS_STYLESHEET;
}

void rgAppStateVulkan::OpenFilesInBuildView(const QStringList& filePaths)
{
    // @TODO: Need to implement this method.
}

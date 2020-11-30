// C++.
#include <cassert>

// Qt.
#include <QAction>
#include <QMenuBar>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgAppStateOpenCL.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgBuildViewOpenCL.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMainWindow.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgSettingsTab.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgStartTabOpenCL.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgStatusBar.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMenu.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDataTypesOpenCL.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDefinitions.h>
#include <RadeonGPUAnalyzerGUI/Include/rgFactory.h>
#include <RadeonGPUAnalyzerGUI/Include/rgUtils.h>
#include <RadeonGPUAnalyzerGUI/Include/rgStringConstants.h>


void rgAppStateOpenCL::ResetBuildView()
{
    // Destroy the rgBuildView instance. Subsequent project creation or load operations will
    // re-initialize a new rgBuildView.
    RG_SAFE_DELETE(m_pBuildView);

    // Re-create an empty rgBuildView.
    CreateBuildView();
}

void rgAppStateOpenCL::ConnectBuildViewSignals(rgBuildView* pBuildView)
{
    rgBuildViewOpenCL* pOpenCLBuildView = static_cast<rgBuildViewOpenCL*>(pBuildView);
    assert(pOpenCLBuildView != nullptr);
    if (pOpenCLBuildView != nullptr)
    {
        // Connect the rgBuildViewOpenCL default menu item's Create button.
        bool isConnected = connect(pOpenCLBuildView, &rgBuildViewOpenCL::CreateFileButtonClicked, this, &rgAppStateOpenCL::HandleCreateNewCLFile);
        assert(isConnected);

        // Connect the rgBuildViewOpenCL default menu item's Open button.
        isConnected = connect(pOpenCLBuildView, &rgBuildViewOpenCL::OpenFileButtonClicked, this, &rgAppStateOpenCL::HandleOpenExistingCLFile);
        assert(isConnected);
    }
}

rgStartTab* rgAppStateOpenCL::GetStartTab()
{
    return m_pStartTab;
}

rgBuildView* rgAppStateOpenCL::GetBuildView()
{
    return m_pBuildView;
}

void rgAppStateOpenCL::SetStartTab(rgStartTab* pStartTab)
{
    m_pStartTab = static_cast<rgStartTabOpenCL*>(pStartTab);
}

void rgAppStateOpenCL::Cleanup(QMenu* pMenubar)
{
    assert(m_pNewFileAction != nullptr);
    assert(pMenubar != nullptr);
    if (m_pNewFileAction != nullptr && pMenubar != nullptr)
    {
        // Remove the OpenCL-specific actions from the file menu.
        pMenubar->removeAction(m_pNewFileAction);
    }
}

void rgAppStateOpenCL::HandleProjectBuildStarted()
{
    // Do not allow creating/adding files.
    m_pNewFileAction->setEnabled(false);
    m_pOpenFileAction->setEnabled(false);
}

void rgAppStateOpenCL::ResetViewStateAfterBuild()
{
    m_pNewFileAction->setEnabled(true);
    m_pOpenFileAction->setEnabled(true);
}

void rgAppStateOpenCL::HandleCreateNewCLFile()
{
    assert(m_pMainWindow != nullptr);
    if (m_pMainWindow != nullptr)
    {
        // Ask user to save pending settings.
        bool userCanceledAction = false;
        if (m_pSettingsTab != nullptr)
        {
            userCanceledAction = !m_pSettingsTab->PromptToSavePendingChanges();
        }

        if (!userCanceledAction)
        {
            if (m_pBuildView != nullptr)
            {
                // Prompt to save items in the build view if needed; including the project build settings.
                userCanceledAction = !m_pBuildView->ShowSaveDialog();
            }

            if (!userCanceledAction)
            {
                // Track if a project was actually created.
                bool wasProjectCreated = false;

                assert(m_pBuildView != nullptr);
                if (m_pBuildView != nullptr)
                {
                    m_pMainWindow->setAcceptDrops(false);
                    wasProjectCreated = m_pBuildView->CreateNewSourceFileInProject();
                    m_pMainWindow->setAcceptDrops(true);
                    if (wasProjectCreated)
                    {
                        // Show the build view as the central widget.
                        m_pMainWindow->SwitchToView(rgMainWindow::MainWindowView::BuildView);

                        // Adjust the menu button focus.
                        rgMenu* pMenu = m_pBuildView->GetMenu();
                        assert(pMenu != nullptr);
                        if (pMenu != nullptr)
                        {
                            pMenu->SetButtonsNoFocus();
                        }
                    }
                    else
                    {
                        // The project was not created successfully, so clean up the build view.
                        m_pMainWindow->DestroyBuildView();
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
}

void rgAppStateOpenCL::HandleOpenExistingCLFile()
{
    assert(m_pMainWindow != nullptr);
    if (m_pMainWindow != nullptr)
    {
        // Ask user to save pending settings.
        bool userCanceledAction = false;
        if (m_pSettingsTab != nullptr)
        {
            userCanceledAction = !m_pSettingsTab->PromptToSavePendingChanges();
        }

        if (!userCanceledAction)
        {
            if (m_pBuildView != nullptr)
            {
                // Prompt to save items in the build view if needed; including the project build settings.
                userCanceledAction = !m_pBuildView->ShowSaveDialog();
            }

            if (!userCanceledAction)
            {
                // Retrieve the current API from the configuration manager.
                rgProjectAPI currentAPI = rgConfigManager::Instance().GetCurrentAPI();

                QStringList selectedFiles;
                bool isOk = rgUtils::OpenFileDialogForMultipleFiles(m_pMainWindow, currentAPI, selectedFiles);
                if (isOk && !selectedFiles.empty())
                {
                    m_pMainWindow->setAcceptDrops(false);
                    OpenFilesInBuildView(selectedFiles);
                    m_pMainWindow->setAcceptDrops(true);
                }
            }
        }
    }
}

void rgAppStateOpenCL::CreateApiSpecificFileActions(QMenu* pMenubar)
{
    m_pNewFileAction = new QAction(tr(STR_MENU_BAR_CREATE_NEW_FILE_OPENCL), this);
    m_pNewFileAction->setShortcuts(QKeySequence::New);
    m_pNewFileAction->setStatusTip(tr(STR_MENU_BAR_CREATE_NEW_FILE_TOOLTIP_OPENCL));
    bool isConnected = connect(m_pNewFileAction, &QAction::triggered, this, &rgAppStateOpenCL::HandleCreateNewCLFile);
    assert(isConnected);
    if (isConnected)
    {
        pMenubar->addAction(m_pNewFileAction);
    }

    m_pOpenFileAction = new QAction(tr(STR_MENU_BAR_OPEN_EXISTING_FILE_OPENCL), this);
    m_pOpenFileAction->setShortcuts(QKeySequence::Open);
    m_pOpenFileAction->setStatusTip(tr(STR_MENU_BAR_OPEN_EXISTING_FILE_TOOLTIP_OPENCL));
    isConnected = connect(m_pOpenFileAction, &QAction::triggered, this, &rgAppStateOpenCL::HandleOpenExistingCLFile);
    assert(isConnected);
    if (isConnected)
    {
        pMenubar->addAction(m_pOpenFileAction);
    }
}

void rgAppStateOpenCL::CreateBuildView()
{
    // Create a factory matching the new API mode to switch to.
    std::shared_ptr<rgFactory> pFactory = rgFactory::CreateFactory(rgProjectAPI::OpenCL);
    assert(pFactory != nullptr);
    if (pFactory != nullptr)
    {
        // Instantiate the rgBuildView.
        m_pBuildView = static_cast<rgBuildViewOpenCL*>(pFactory->CreateBuildView(m_pMainWindow));

        // Connect the project created handler so the rgMainWindow can
        // add the new rgBuildView instance to the widget hierarchy.
        bool isConnected = connect(m_pBuildView, &rgBuildView::ProjectCreated, m_pMainWindow, &rgMainWindow::HandleProjectCreated);
        assert(isConnected);
    }
}

void rgAppStateOpenCL::ConnectFileMenuActions()
{
    assert(m_pStartTab != nullptr);
    if (m_pStartTab != nullptr)
    {
        // Connect the "Create new CL file" button in the start page.
        bool isConnected = connect(m_pStartTab, &rgStartTabOpenCL::CreateNewCLFileEvent, this, &rgAppStateOpenCL::HandleCreateNewCLFile);
        assert(isConnected);

        // Connect the "Open existing CL file" button in the start page.
        isConnected = connect(m_pStartTab, &rgStartTabOpenCL::OpenExistingFileEvent, this, &rgAppStateOpenCL::HandleOpenExistingCLFile);
        assert(isConnected);
    }
}

void rgAppStateOpenCL::OpenFilesInBuildView(const QStringList& filePaths)
{
    if (!filePaths.empty())
    {
        assert(m_pBuildView != nullptr);
        if (m_pBuildView != nullptr)
        {
            for (const QString& selectedFile : filePaths)
            {
                // Update the project to reference to selected source file.
                bool result = m_pBuildView->AddExistingSourcefileToProject(selectedFile.toStdString());

                // Break out of the for loop if the operation failed.
                if (!result)
                {
                    break;
                }
            }

            // Switch to the build view if there's at least one file being edited.
            if (!m_pBuildView->HasSourceCodeEditors())
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
    }
}

void rgAppStateOpenCL::GetApplicationStylesheet(std::vector<std::string>& stylesheetFileNames)
{
    stylesheetFileNames.push_back(STR_APPLICATION_STYLESHEET_FILE_OPENCL);
    stylesheetFileNames.push_back(STR_APPLICATION_STYLESHEET_FILE);
}

void rgAppStateOpenCL::GetMainWindowStylesheet(std::vector<std::string>& stylesheetFileNames)
{
    stylesheetFileNames.push_back(STR_MAIN_WINDOW_STYLESHEET_FILE_OPENCL);
    stylesheetFileNames.push_back(STR_MAIN_WINDOW_STYLESHEET_FILE);
}

std::string rgAppStateOpenCL::GetGlobalSettingsViewStylesheet() const
{
    const char* STR_OPENCL_APP_SETTINGS_STYLESHEET =
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
        "rgGlobalSettingsView #assocExtSpvasLineEdit:focus,"
        "rgGlobalSettingsView #assocExtSpvasLineEdit:hover,"
        "rgGlobalSettingsView #assocExtSpvBinaryLineEdit:focus,"
        "rgGlobalSettingsView #assocExtSpvBinaryLineEdit:hover,"
        "rgGlobalSettingsView #assocExtSpvasLabel:focus,"
        "rgGlobalSettingsView #assocExtSpvasLabel:hover,"
        "rgGlobalSettingsView #defaultLangComboBox:focus,"
        "rgGlobalSettingsView #defaultLangComboBox:hover"
        "{"
        "border: 1px solid lightGreen;"
        "background: rgb(240, 240, 240);"
        "}";
    return STR_OPENCL_APP_SETTINGS_STYLESHEET;
}

std::string rgAppStateOpenCL::GetBuildSettingsViewStylesheet() const
{
    const char* STR_OPENCL_BUILD_SETTINGS_STYLESHEET =
        "rgBuildSettingsView #doubleAsSingleCheckBox:focus,"
        "rgBuildSettingsView #doubleAsSingleCheckBox:hover,"
        "rgBuildSettingsView #flushDenormalizedCheckBox:focus,"
        "rgBuildSettingsView #flushDenormalizedCheckBox:hover,"
        "rgBuildSettingsView #strictAliasingCheckBox:focus,"
        "rgBuildSettingsView #strictAliasingCheckBox:hover,"
        "rgBuildSettingsView #enableMADCheckBox:focus,"
        "rgBuildSettingsView #enableMADCheckBox:hover,"
        "rgBuildSettingsView #ignoreZeroSignednessCheckBox:focus,"
        "rgBuildSettingsView #ignoreZeroSignednessCheckBox:hover,"
        "rgBuildSettingsView #allowUnsafeOptimizationsCheckBox:focus,"
        "rgBuildSettingsView #allowUnsafeOptimizationsCheckBox:hover,"
        "rgBuildSettingsView #assumeNoNanNorInfiniteCheckBox:focus,"
        "rgBuildSettingsView #assumeNoNanNorInfiniteCheckBox:hover,"
        "rgBuildSettingsView #aggressiveMathOptimizationsCheckBox:focus,"
        "rgBuildSettingsView #aggressiveMathOptimizationsCheckBox:hover,"
        "rgBuildSettingsView #correctlyRoundSinglePrecisionCheckBox:focus,"
        "rgBuildSettingsView #correctlyRoundSinglePrecisionCheckBox:hover,"
        "rgBuildSettingsView #predefinedMacrosLineEdit:focus,"
        "rgBuildSettingsView #predefinedMacrosLineEdit:hover,"
        "rgBuildSettingsView #includeDirectoriesLineEdit:focus,"
        "rgBuildSettingsView #includeDirectoriesLineEdit:hover,"
        "rgBuildSettingsView #compilerBinariesLineEdit:focus,"
        "rgBuildSettingsView #compilerBinariesLineEdit:hover,"
        "rgBuildSettingsView #compilerIncludesLineEdit:focus,"
        "rgBuildSettingsView #compilerIncludesLineEdit:hover,"
        "rgBuildSettingsView #additionalOptionsTextEdit:focus,"
        "rgBuildSettingsView #additionalOptionsTextEdit:hover,"
        "rgBuildSettingsView #compilerLibrariesLineEdit:focus,"
        "rgBuildSettingsView #compilerLibrariesLineEdit:hover"
        "{"
        "border: 1px solid lightGreen;"
        "background: rgb(240, 240, 240);"
        "}";
    return STR_OPENCL_BUILD_SETTINGS_STYLESHEET;
}

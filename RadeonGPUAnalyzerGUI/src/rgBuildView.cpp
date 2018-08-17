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
#include <QTextStream>
#include <QVBoxLayout>
#include <QWidget>

// Infra.
#include <QtCommon/Scaling/ScalingManager.h>

// Local.
#include <RadeonGPUAnalyzerGUI/include/qt/rgBuildView.h>
#include <RadeonGPUAnalyzerGUI/include/qt/rgBuildSettingsWidget.h>
#include <RadeonGPUAnalyzerGUI/include/qt/rgCliOutputView.h>
#include <RadeonGPUAnalyzerGUI/include/qt/rgFileMenu.h>
#include <RadeonGPUAnalyzerGUI/include/qt/rgFileMenuBuildSettingsItem.h>
#include <RadeonGPUAnalyzerGUI/include/qt/rgFileMenuFileItem.h>
#include <RadeonGPUAnalyzerGUI/include/qt/rgFileMenuTitlebar.h>
#include <RadeonGPUAnalyzerGUI/include/qt/rgFindTextWidget.h>
#include <RadeonGPUAnalyzerGUI/include/qt/rgIsaDisassemblyView.h>
#include <RadeonGPUAnalyzerGUI/include/qt/rgMainWindow.h>
#include <RadeonGPUAnalyzerGUI/include/qt/rgMaximizeSplitter.h>
#include <RadeonGPUAnalyzerGUI/include/qt/rgOpenCLBuildSettingsView.h>
#include <RadeonGPUAnalyzerGUI/include/qt/rgRenameProjectDialog.h>
#include <RadeonGPUAnalyzerGUI/include/qt/rgSourceCodeEditor.h>
#include <RadeonGPUAnalyzerGUI/include/qt/rgSettingsButtonsView.h>
#include <RadeonGPUAnalyzerGUI/include/qt/rgSourceEditorTitlebar.h>
#include <RadeonGPUAnalyzerGUI/include/qt/rgUnsavedItemsDialog.h>
#include <RadeonGPUAnalyzerGUI/include/qt/rgViewContainer.h>
#include <RadeonGPUAnalyzerGUI/include/qt/rgViewManager.h>
#include <RadeonGPUAnalyzerGUI/include/rgConfigManager.h>
#include <RadeonGPUAnalyzerGUI/include/rgCliLauncher.h>
#include <RadeonGPUAnalyzerGUI/include/rgDefinitions.h>
#include <RadeonGPUAnalyzerGUI/include/rgFactory.h>
#include <RadeonGPUAnalyzerGUI/include/rgSourceEditorSearcher.h>
#include <RadeonGPUAnalyzerGUI/include/rgStringConstants.h>
#include <RadeonGPUAnalyzerGUI/include/rgUtils.h>
#include <RadeonGPUAnalyzerGUI/include/rgXMLSessionConfig.h>

rgBuildView::rgBuildView(rgProjectAPI api, QWidget* pParent) :
    QWidget(pParent),
    m_cloneIndex(0)
{
    // Setup the UI.
    ui.setupUi(this);

    // Create the factory used to create API specific objects within the rgBuildView.
    m_pFactory = rgFactory::CreateFactory(api);

    // Create the file menu.
    m_pFileMenu = new rgFileMenu(this);
    m_pFileMenuTitlebar = new rgFileMenuTitlebar();

    // Wrap the file menu in a view container with its titlebar.
    m_pFileMenuViewContainer = new rgViewContainer();
    m_pFileMenuViewContainer->SetMainWidget(m_pFileMenu);
    m_pFileMenuViewContainer->SetTitlebarWidget(m_pFileMenuTitlebar);

    // Create an empty panel to insert alongside the file menu when no files are open.
    m_pEmptyPanel = new QWidget(this);

    // Create container for source view widgets.
    m_pSourceViewStack = new QWidget();
    QVBoxLayout* pLayout = new QVBoxLayout();
    pLayout->setContentsMargins(0, 0, 0, 0);
    m_pSourceViewStack->setLayout(pLayout);

    // Wrap the source code stack in a view container and a source editor titlebar.
    m_pSourceViewContainer = new rgViewContainer();
    m_pSourceViewContainer->SetMainWidget(m_pSourceViewStack);
    m_pSourceEditorTitlebar = new rgSourceEditorTitlebar(m_pSourceViewContainer);
    m_pSourceViewContainer->SetTitlebarWidget(m_pSourceEditorTitlebar);

    // Create the disassembly view splitter.
    m_pDisassemblyViewSplitter = new rgMaximizeSplitter(this);
    m_pDisassemblyViewSplitter->setOrientation(Qt::Orientation::Horizontal);
    m_pDisassemblyViewSplitter->setChildrenCollapsible(false);

    // Add the source view container to the disassembly view splitter.
    m_pDisassemblyViewSplitter->AddMaximizableWidget(m_pSourceViewContainer);

    // Set up the splitter between the file menu and the rest of the views.
    m_pFileMenuSplitter = new QSplitter(Qt::Orientation::Horizontal, this);

    // Add the file menu and the disassembly view splitter to the file menu splitter.
    m_pFileMenuSplitter->addWidget(m_pFileMenuViewContainer);
    m_pFileMenuSplitter->addWidget(m_pDisassemblyViewSplitter);

    // The file menu should not grow with the window, while the source code view should.
    m_pFileMenuSplitter->setStretchFactor(0, 0);
    m_pFileMenuSplitter->setStretchFactor(1, 1);

    // Disable the file menu splitter.
    m_pFileMenuSplitter->handle(1)->setDisabled(true);

    // Create the output window.
    m_pCliOutputWindow = new rgCliOutputView(this);

    // Wrap the build output view in a view container with an embedded titlebar.
    m_pBuildOutputViewContainer = new rgViewContainer();
    m_pBuildOutputViewContainer->SetMainWidget(m_pCliOutputWindow);

    // Create a vertical splitter to divide the rgBuildView's FileMenu/SourceEditors and the Output Window.
    m_pOutputSplitter = new rgMaximizeSplitter(this);
    m_pOutputSplitter->setOrientation(Qt::Orientation::Vertical);

    // Connect the build output window signals.
    ConnectOutputWindowSignals();

    // Add the file menu's splitter and the output window to the splitter.
    m_pOutputSplitter->addWidget(m_pFileMenuSplitter);
    m_pOutputSplitter->AddMaximizableWidget(m_pBuildOutputViewContainer);

    // Let the file menu and code editor resize, and the output window will stay vertically squished.
    m_pOutputSplitter->setStretchFactor(0, 6);
    m_pOutputSplitter->setStretchFactor(1, 1);
    m_pOutputSplitter->setCollapsible(1, false);

    // Create a main window layout, and add the root-level splitter widget.
    QVBoxLayout* pMainLayout = new QVBoxLayout(this);
    pMainLayout->addWidget(m_pOutputSplitter);
    this->setLayout(pMainLayout);

    // Connect signals for the file menu.
    ConnectFileSignals();

    // Restore the previous session buildview layout.
    RestoreViewLayout();

    // Connect signals for the Build View.
    ConnectBuildViewSignals();

    // Setup view manager.
    m_pViewManager = new rgViewManager(this);
    m_pViewManager->AddView(m_pFileMenuViewContainer);
    m_pViewManager->AddView(m_pSourceViewContainer);
    m_pViewManager->AddView(m_pBuildOutputViewContainer, false);

    // Declare EditMode as a meta type so it can be used with slots/signals.
    int id = qRegisterMetaType<EditMode>();
    Q_UNUSED(id);
}

void rgBuildView::AddFile(const std::string& fileFullPath, bool isNewFile)
{
    if (m_pFileMenu)
    {
        m_pFileMenu->AddItem(fileFullPath, isNewFile);
    }
}

bool rgBuildView::CheckSourcesModifiedSinceLastBuild(rgSourceCodeEditor* pCodeEditor)
{
    bool isModifiedAfterBuild = false;

    assert(pCodeEditor != nullptr);
    if (pCodeEditor != nullptr)
    {
        auto fileModificationTimeMapIter = m_fileModifiedTimeMap.find(pCodeEditor);
        if (fileModificationTimeMapIter != m_fileModifiedTimeMap.end())
        {
            // If the source file was modified after the last build, line correlation data has become invalid.
            const QDateTime& lastSavedTime = fileModificationTimeMapIter->second;
            if (lastSavedTime > m_lastSuccessfulBuildTime)
            {
                isModifiedAfterBuild = true;
            }
        }
    }

    return isModifiedAfterBuild;
}

void rgBuildView::ClearBuildView()
{
    assert(m_pFileMenu != nullptr);
    if (m_pFileMenu != nullptr)
    {
        m_pFileMenu->ClearFiles();
    }

    // Clean up source editor instances.
    ClearEditors();

    // Clear the output window.
    m_pCliOutputWindow->ClearText();
}

void rgBuildView::ClearEditors()
{
    std::vector<std::string> filePaths;
    for (auto editorIter = m_sourceCodeEditors.begin(); editorIter != m_sourceCodeEditors.end(); ++editorIter)
    {
        filePaths.push_back(editorIter->first);
    }

    for (const std::string& fullFilePath : filePaths)
    {
        RemoveEditor(fullFilePath);
    }
}

void rgBuildView::ClearFileItemsEntrypointList()
{
    assert(m_pFileMenu != nullptr);
    if (m_pFileMenu != nullptr)
    {
        // Clear references to build outputs from the file menu,
        m_pFileMenu->ClearBuildOutputs();
    }
}

std::string rgBuildView::CreateProjectBuildOutputPath() const
{
    std::string resultPath;

    // Build an output path where all of the build artifacts will be dumped to.
    std::string projectDirectory;
    bool isOk = rgUtils::ExtractFileDirectory(m_pProject->m_projectFileFullPath, projectDirectory);
    assert(isOk);
    if (isOk)
    {
        std::string outputFolderPath;
        isOk = rgUtils::AppendFolderToPath(projectDirectory, STR_OUTPUT_FOLDER_NAME, outputFolderPath);
        assert(isOk);
        if (isOk)
        {
            resultPath = outputFolderPath;
        }
    }

    return resultPath;
}

void rgBuildView::ConnectFileSignals()
{
    assert(m_pFileMenu != nullptr);

    // Connect the file menu to the build view, so that the view knows when the current file is switched.
    bool isConnected = connect(m_pFileMenu, &rgFileMenu::SelectedFileChanged, this, &rgBuildView::HandleSelectedFileChanged);
    assert(isConnected);

    // Connect the file menu's MenuItemCloseButtonClicked signal with the menu item closed handler.
    isConnected = connect(m_pFileMenu, &rgFileMenu::MenuItemCloseButtonClicked, this, &rgBuildView::HandleMenuItemCloseButtonClicked);
    assert(isConnected);

    // Connect the file menu default item's "Add new file" button.
    isConnected = connect(m_pFileMenu, &rgFileMenu::CreateFileButtonClicked, this, &rgBuildView::CreateFileButtonClicked);
    assert(isConnected);

    // Connect the file menu default item's "Open existing file" button.
    isConnected = connect(m_pFileMenu, &rgFileMenu::OpenFileButtonClicked, this, &rgBuildView::OpenFileButtonClicked);
    assert(isConnected);

    // Connect the file menu's rename signal.
    isConnected = connect(m_pFileMenu, &rgFileMenu::FileRenamed, this, &rgBuildView::HandleFileRenamed);
    assert(isConnected);

    // Connect the file menu's project renamed signal.
    isConnected = connect(m_pFileMenuTitlebar, &rgFileMenuTitlebar::TitleChanged, this, &rgBuildView::HandleProjectRenamed);
    assert(isConnected);

    // Connect the file menu's item count change signal.
    isConnected = connect(m_pFileMenu, &rgFileMenu::FileMenuItemCountChanged, this, &rgBuildView::ProjectFileCountChanged);
    assert(isConnected);

    // Connect the file menu's file item entrypoint changed signal.
    isConnected = connect(m_pFileMenu, &rgFileMenu::SelectedEntrypointChanged, this, &rgBuildView::HandleSelectedEntrypointChanged);
    assert(isConnected);

    // Connect the file menu's file build settings button click signal to the rgBuildView's handler.
    isConnected = connect(m_pFileMenu, &rgFileMenu::BuildSettingsButtonClicked, this, &rgBuildView::HandleFindWidgetVisibilityToggled);
    assert(isConnected);

    // Connect the file menu's next focus change signal.
    isConnected = connect(m_pFileMenu, &rgFileMenu::FocusNextView, this, &rgBuildView::HandleFocusNextView);
    assert(isConnected);

    // Connect the rgBuildView's entrypoint changed signal to the file menu's handler.
    isConnected = connect(this, &rgBuildView::SelectedEntrypointChanged, m_pFileMenu, &rgFileMenu::HandleSelectedEntrypointChanged);
    assert(isConnected);

    // Connect the error location reported by CLI output window to the rgBuildView's handlers.
    isConnected = connect(m_pCliOutputWindow, &rgCliOutputView::SwitchToFile, m_pFileMenu, &rgFileMenu::HandleSwitchToFile);
    assert(isConnected);
    isConnected = connect(m_pFileMenu, &rgFileMenu::ScrollCodeEditorToLine, this, &rgBuildView::HandleScrollCodeEditorToLine);
    assert(isConnected);

    // Connect the "Build Settings" button in the menu file.
    rgFileMenuBuildSettingsItem* pBuildSettingsItem = m_pFileMenu->GetBuildSettingsItem();
    const QPushButton* pBuildSettingsFileMenuButton = pBuildSettingsItem->GetBuildSettingsButton();
    isConnected = connect(pBuildSettingsFileMenuButton, &QPushButton::clicked, this, &rgBuildView::HandleBuildSettingsMenuButtonClicked);
    assert(isConnected);

    // Connect to the file menu container's mouse click event.
    isConnected = connect(m_pFileMenuViewContainer, &rgViewContainer::ViewContainerMouseClickEventSignal, this, &rgBuildView::HandleSetFrameBorderBlack);
    assert(isConnected);
}

void rgBuildView::ConnectBuildSettingsSignals()
{
    bool isConnected = connect(static_cast<rgOpenCLBuildSettingsView*>(m_pBuildSettingsView), &rgOpenCLBuildSettingsView::PendingChangesStateChanged, this, &rgBuildView::HandleBuildSettingsPendingChangesStateChanged);
    assert(isConnected);

    isConnected = connect(static_cast<rgOpenCLBuildSettingsView*>(m_pBuildSettingsView), &rgOpenCLBuildSettingsView::ProjectBuildSettingsSaved, this, &rgBuildView::HandleBuildSettingsSaved);
    assert(isConnected);

    // Connect to build settings view's edit line's "focus in" event to color the frame red.
    isConnected = connect(static_cast<rgOpenCLBuildSettingsView*>(m_pBuildSettingsView), &rgOpenCLBuildSettingsView::SetFrameBorderRedSignal, this, &rgBuildView::HandleSetFrameBorderRed);
    assert(isConnected);

    // Connect to build settings view's edit line's "focus out" event to color the frame black.
    isConnected = connect(static_cast<rgOpenCLBuildSettingsView*>(m_pBuildSettingsView), &rgOpenCLBuildSettingsView::SetFrameBorderBlackSignal, this, &rgBuildView::HandleSetFrameBorderBlack);
    assert(isConnected);

    // "Save" button.
    isConnected = connect(m_pSettingsButtonsView, &rgSettingsButtonsView::SaveSettingsButtonClickedSignal, this, &rgBuildView::HandleSaveSettingsButtonClicked);
    assert(isConnected);

    // "Restore defaults" button.
    isConnected = connect(m_pSettingsButtonsView, &rgSettingsButtonsView::RestoreDefaultSettingsButtonClickedSignal, this, &rgBuildView::HandleRestoreDefaultsSettingsClicked);
    assert(isConnected);

    // Connect to build settings widget's "focus in" event to color the boundary red.
    isConnected = connect(m_pBuildSettingsWidget, &rgBuildSettingsWidget::FrameFocusInEventSignal, this, &rgBuildView::HandleSetFrameBorderRed);
    assert(isConnected);

    // Connect to build settings widget's "focus out" event to color the boundary black.
    isConnected = connect(m_pBuildSettingsWidget, &rgBuildSettingsWidget::FrameFocusOutEventSignal, this, &rgBuildView::HandleSetFrameBorderBlack);
    assert(isConnected);
}

void rgBuildView::ConnectFindSignals()
{
    // Connect the find widget's close toggle handler.
    bool isConnected = connect(m_pFindWidget, &rgFindTextWidget::CloseWidgetSignal, this, &rgBuildView::HandleFindWidgetVisibilityToggled);
    assert(isConnected);
}

void rgBuildView::ConnectBuildViewSignals()
{
    bool isConnected = connect(this, &rgBuildView::LineCorrelationEnabledStateChanged, this, &rgBuildView::HandleIsLineCorrelationEnabled);
    assert(isConnected);

    isConnected = connect(this, &rgBuildView::ProjectBuildSuccess, this, &rgBuildView::HandleProjectBuildSuccess);
    assert(isConnected);

    isConnected = connect(qGuiApp, &QGuiApplication::applicationStateChanged, this, &rgBuildView::HandleApplicationStateChanged);
    assert(isConnected);
}

void rgBuildView::ConnectOutputWindowSignals()
{
    bool isConnected = connect(this, &rgBuildView::ProjectBuildStarted, m_pCliOutputWindow, &rgCliOutputView::HandleBuildStarted);
    assert(isConnected);

    isConnected = connect(this, &rgBuildView::ProjectBuildFailure, m_pCliOutputWindow, &rgCliOutputView::HandleBuildEnded);
    assert(isConnected);

    isConnected = connect(this, &rgBuildView::ProjectBuildSuccess, m_pCliOutputWindow, &rgCliOutputView::HandleBuildEnded);
    assert(isConnected);

    isConnected = connect(this, &rgBuildView::ProjectBuildCanceled, m_pCliOutputWindow, &rgCliOutputView::HandleBuildEnded);
    assert(isConnected);

    assert(m_pOutputSplitter != nullptr);
    if (m_pOutputSplitter != nullptr)
    {
        // Connect the handler invoked when the output window splitter is resized.
        isConnected = connect(m_pOutputSplitter, &rgMaximizeSplitter::splitterMoved, this, &rgBuildView::HandleSplitterMoved);
        assert(isConnected);
    }
}

void rgBuildView::ConnectDisassemblyViewSignals()
{
    assert(m_pDisassemblyView != nullptr);
    if (m_pDisassemblyView != nullptr)
    {
        // Connect the handler invoked when the user changes the selected entrypoint.
        bool isConnected = connect(this, &rgBuildView::SelectedEntrypointChanged,
            m_pDisassemblyView, &rgIsaDisassemblyView::HandleSelectedEntrypointChanged);
        assert(isConnected);

        // Connect the handler invoked when the highlighted correlation line in the input source file should be updated.
        isConnected = connect(m_pDisassemblyView, &rgIsaDisassemblyView::InputSourceHighlightedLineChanged,
            this, &rgBuildView::HandleHighlightedCorrelationLineUpdated);
        assert(isConnected);

        // Connect the rgIsaDisassemblyView's entrypoint changed handler.
        isConnected = connect(m_pDisassemblyView, &rgIsaDisassemblyView::SelectedEntrypointChanged,
            this, &rgBuildView::HandleSelectedEntrypointChanged);
        assert(isConnected);

        // Connect the rgIsaDisassemblyView's table resized handler.
        isConnected = connect(m_pDisassemblyView, &rgIsaDisassemblyView::DisassemblyTableWidthResizeRequested,
            this, &rgBuildView::HandleDisassemblyTableWidthResizeRequested);
        assert(isConnected);

        // Connect the rgIsaDisassemblyView's Target GPU changed handler.
        isConnected = connect(m_pDisassemblyView, &rgIsaDisassemblyView::SelectedTargetGpuChanged,
            this, &rgBuildView::HandleSelectedTargetGpuChanged);
        assert(isConnected);
    }

    assert(m_pDisassemblyViewSplitter != nullptr);
    if (m_pDisassemblyViewSplitter != nullptr)
    {
        // Connect the handler invoked when the disassembly container has been maximized.
        bool isConnected = connect(m_pDisassemblyViewSplitter, &rgMaximizeSplitter::ViewMaximized,
            this, &rgBuildView::HandleDisassemblyViewSizeMaximize);
        assert(isConnected);

        // Connect the handler invoked when the disassembly container has been restored to normal size.
        isConnected = connect(m_pDisassemblyViewSplitter, &rgMaximizeSplitter::ViewRestored,
            this, &rgBuildView::HandleDisassemblyViewSizeRestore);
        assert(isConnected);

        // Connect the handler invoked when the disassembly splitter is resized.
        isConnected = connect(m_pDisassemblyViewSplitter, &rgMaximizeSplitter::splitterMoved,
            this, &rgBuildView::HandleSplitterMoved);
        assert(isConnected);
    }
}

void rgBuildView::ConnectSourcecodeEditorSignals(rgSourceCodeEditor* pEditor)
{
    // Connect the file modified handler.
    bool isConnected = connect(pEditor, &QPlainTextEdit::modificationChanged, this, &rgBuildView::HandleEditorModificationStateChanged);
    assert(isConnected);

    // Connect the source editor's selected line changed handler.
    isConnected = connect(pEditor, &rgSourceCodeEditor::SelectedLineChanged, this, &rgBuildView::HandleSourceFileSelectedLineChanged);
    assert(isConnected);

    // Connect the source editor's resized handler.
    isConnected = connect(pEditor, &rgSourceCodeEditor::EditorResized, this, &rgBuildView::HandleSourceEditorResized);
    assert(isConnected);

    // Connect the source editor's hidden handler.
    isConnected = connect(pEditor, &rgSourceCodeEditor::EditorHidden, this, &rgBuildView::HandleSourceEditorHidden);
    assert(isConnected);
}

void rgBuildView::OpenBuildSettings()
{
    SwitchEditMode(EditMode::BuildSettings);
}

bool rgBuildView::PopulateBuildView()
{
    bool ret = false;

    // Clear the rgBuildView's file menu and source editor before repopulating it.
    ClearBuildView();

    // Verify that each source file path referenced by the project is valid.
    // Allow the user to fix any invalid paths that are found.
    bool isProjectSourcesValid = rgUtils::IsProjectSourcePathsValid(m_pProject, m_cloneIndex, this);

    // If the project source paths are valid, proceed.
    if (isProjectSourcesValid)
    {
        // Fill up the file path list with the paths corrected by the user.
        std::vector<std::string> sourceFilepaths;
        rgConfigManager::Instance().GetProjectSourceFilePaths(m_pProject, m_cloneIndex, sourceFilepaths);

        if (!sourceFilepaths.empty())
        {
            // Add all the project's source files into the rgBuildView.
            for (int fileIndex = 0; fileIndex < sourceFilepaths.size(); ++fileIndex)
            {
                const std::string& filePath = sourceFilepaths[fileIndex];

                // Check that the file still exists before attempting to load it.
                bool isFileExists = rgUtils::IsFileExists(filePath);
                assert(isFileExists);
                if (isFileExists)
                {
                    // Add the selected file to the menu.
                    AddFile(filePath);

                    // Set the source code view text with the contents of the selected file.
                    SetSourceCodeText(filePath);

                    // The rgBuildView was successfully populated with the current project.
                    ret = true;
                }
                else
                {
                    // Build an error string saying the file couldn't be found on disk.
                    std::stringstream errorString;
                    errorString << STR_ERR_CANNOT_LOAD_SOURCE_FILE_MSG;
                    errorString << filePath;

                    // Show the user the error message.
                    rgUtils::ShowErrorMessageBox(errorString.str().c_str(), this);
                }
            }
        }
        else
        {
            // There are no files to display. Open the rgBuildView in an empty state, and return true.
            SwitchEditMode(EditMode::Empty);
            ret = true;
        }
    }

    return ret;
}

void rgBuildView::CreateProjectClone()
{
    if (m_pProject != nullptr)
    {
        // Create Clone 0, and add it into the new project.
        std::string cloneName = rgUtils::GenerateCloneName(m_cloneIndex);
        std::shared_ptr<rgProjectClone> pClone0 = m_pFactory->CreateProjectClone(cloneName);
        assert(pClone0 != nullptr);
        if (pClone0 != nullptr)
        {
            m_pProject->m_clones.push_back(pClone0);

            // Save the project file.
            rgConfigManager& configManager = rgConfigManager::Instance();
            configManager.SaveProjectFile(m_pProject);

            if (m_pFileMenu != nullptr)
            {
                // Set project name title in file menu.
                std::stringstream title;
                title << rgUtils::GetProjectTitlePrefix(m_pProject->m_api) << m_pProject->m_projectName;
                m_pFileMenuTitlebar->SetTitle(title.str().c_str());
            }

            // Indicate that a new project has been loaded.
            emit ProjectLoaded(m_pProject);

            // Create a new build settings view after a new project has been created.
            CreateBuildSettingsView();
        }
    }
}

bool rgBuildView::GetEntrypointNameForLineNumber(const std::string& filePath, int lineNumber, std::string& entryName)
{
    bool found = false;

    // 1. Check if "lineNumber" is within some kernel code.
    const auto& fileSrcLineData = m_entrypointLineNumbers.find(filePath);
    if (fileSrcLineData != m_entrypointLineNumbers.end())
    {
        for (const auto& entrySrcLineData : fileSrcLineData->second)
        {
            const std::pair<int, int>  startAndEndLines = entrySrcLineData.second;
            if (lineNumber >= startAndEndLines.first && lineNumber <= startAndEndLines.second)
            {
                entryName = entrySrcLineData.first;
                found = true;
                break;
            }
        }
    }

    // 2. Check if "lineNumber" is present in the line correlation table for currently active entry.
    if (!found)
    {
        found = m_pDisassemblyView->IsLineCorrelatedInEntry(m_currentTargetGpu, entryName, lineNumber);

        assert(m_pFileMenu != nullptr);
        if (m_pFileMenu != nullptr)
        {
            // Fall back to selecting the current entrypoint in the selected file item.
            rgFileMenuFileItem* pFileItem = m_pFileMenu->GetFileItemFromPath(filePath);
            assert(pFileItem != nullptr);
            if (pFileItem != nullptr)
            {
                std::string entrypointName;
                if (pFileItem->GetSelectedEntrypointName(entrypointName))
                {
                    entryName = entrypointName;
                }
            }
        }
    }

    return found;
}

bool rgBuildView::CreateNewEmptyProject()
{
    bool ret = false;
    std::string projectName;

    // Get the global configuration.
    rgConfigManager& configManager = rgConfigManager::Instance();
    std::shared_ptr<rgGlobalSettings> pGlobalSettings = configManager.GetGlobalConfig();

    if (pGlobalSettings->m_useDefaultProjectName == true)
    {
        // Generate a default project name.
        projectName = rgUtils::GenerateDefaultProjectName();

        // Create a project instance with the given name.
        m_pProject = m_pFactory->CreateProject(projectName, rgConfigManager::GenerateProjectFilepath(projectName));
        assert(m_pProject != nullptr);

        // Create the clone
        CreateProjectClone();

        // We're done.
        ret = true;
    }
    else
    {
        rgRenameProjectDialog* pRenameProjectDialog = new rgRenameProjectDialog(projectName, this);

        // Prompt the user for the project name.
        int rc = pRenameProjectDialog->exec();
        if (rc == QDialog::Accepted)
        {
            // Create a project instance with the given name.
            m_pProject = m_pFactory->CreateProject(projectName, rgConfigManager::GenerateProjectFilepath(projectName));
            assert(m_pProject != nullptr);

            // Emit a signal to force use default project name check box value update.
            emit UpdateUseDefaultProjectNameCheckbox();

            // Create the clone.
            CreateProjectClone();

            // We're done.
            ret = true;
        }
    }
    return ret;
}

bool rgBuildView::CreateNewSourceFileInProject()
{
    // Generate a new empty source file in the correct location.
    std::string sourceFilename = STR_DEFAULT_SOURCE_FILENAME;

    std::string fullSourcefilePath;
    bool ret = CreateNewSourceFile(sourceFilename, fullSourcefilePath);

    if (ret)
    {
        // This should be a full filename, but new files don't have that.
        AddFile(fullSourcefilePath, true);

        // Display the file's source code.
        SetSourceCodeText(fullSourcefilePath);
    }

    return ret;
}

bool rgBuildView::IsFileDisassembled(const std::string& inputFilePath) const
{
    bool isCurrentFileDisassembled = false;

    auto targetGpuOutputsIter = m_buildOutputs.find(m_currentTargetGpu);
    if (targetGpuOutputsIter != m_buildOutputs.end())
    {
        std::shared_ptr<rgCliBuildOutput> pBuildOutput = targetGpuOutputsIter->second;
        auto inputFileOutputsIter = pBuildOutput->m_perFileOutput.find(inputFilePath);
        if (inputFileOutputsIter != pBuildOutput->m_perFileOutput.end())
        {
            rgFileOutputs& fileOutputs = inputFileOutputsIter->second;
            isCurrentFileDisassembled = !fileOutputs.m_outputs.empty();
        }
    }

    return isCurrentFileDisassembled;
}

bool rgBuildView::IsEmpty() const
{
    return m_sourceCodeEditors.empty();
}

bool rgBuildView::LoadBuildOutput(const std::string& projectFolder, const std::vector<std::string>* pTargetGpus)
{
    bool isLoaded = false;

    std::vector<std::string> targetGpuFamilyResultsToLoad;

    // Build a list of possible target GPUs to attempt to load results for, based on the supported GPUs for the current mode.
    std::shared_ptr<rgCliVersionInfo> pVersionInfo = rgConfigManager::Instance().GetVersionInfo();
    assert(pVersionInfo != nullptr);
    if (pVersionInfo != nullptr)
    {
        // Determine which GPU architectures and families are supported in the current mode.
        const std::string& currentMode = rgConfigManager::Instance().GetCurrentMode();
        auto modeArchitecturesIter = pVersionInfo->m_gpuArchitectures.find(currentMode);
        if (modeArchitecturesIter != pVersionInfo->m_gpuArchitectures.end())
        {
            const std::vector<rgGpuArchitecture>& modeArchitectures = modeArchitecturesIter->second;

            // Step through each architecture.
            for (auto architectureIter = modeArchitectures.begin(); architectureIter != modeArchitectures.end(); ++architectureIter)
            {
                // Step through each family within the architecture.
                for (auto familyIter = architectureIter->m_gpuFamilies.begin(); familyIter != architectureIter->m_gpuFamilies.end(); ++familyIter)
                {
                    // Add the family name to the list of targets to atempt to load results for.
                    targetGpuFamilyResultsToLoad.push_back(familyIter->m_familyName);
                }
            }
        }
    }

    // Build a path to the project's output directory.
    std::string buildOutputPath = CreateProjectBuildOutputPath();

    // Generate a clone name string based on the current clone index.
    std::string outputFolderPath;

    bool isOk = rgUtils::AppendFolderToPath(projectFolder, STR_OUTPUT_FOLDER_NAME, outputFolderPath);
    assert(isOk);
    if (isOk)
    {
        // Append the clone folder to the build output path.
        std::string cloneNameString = rgUtils::GenerateCloneName(m_cloneIndex);
        bool isOk = rgUtils::AppendFolderToPath(outputFolderPath, cloneNameString, outputFolderPath);
        assert(isOk);
        if (isOk)
        {
            const std::vector<std::string>* pGpusToLoad = nullptr;
            if (pTargetGpus != nullptr)
            {
                // If a list of GPUs was provided, attempt to load output for each.
                pGpusToLoad = pTargetGpus;
            }
            else
            {
                // When no target GPUs to load are provided, fall back to attempting to load results for all possible target GPUs.
                // If the session metadata for the target GPU doesn't exist, there's no disassembly to load.
                pGpusToLoad = &targetGpuFamilyResultsToLoad;
            }

            assert(pGpusToLoad != nullptr);
            if (pGpusToLoad != nullptr)
            {
                // Attempt to load outputs for each GPU that was targeted.
                for (const std::string& currentGpu : *pGpusToLoad)
                {
                    std::stringstream metadataFilenameStream;
                    metadataFilenameStream << currentGpu;
                    metadataFilenameStream << "_";
                    metadataFilenameStream << STR_SESSION_METADATA_FILENAME;

                    std::string fullMetadataFilePath;
                    isOk = rgUtils::AppendFileNameToPath(outputFolderPath, metadataFilenameStream.str(), fullMetadataFilePath);
                    assert(isOk);
                    if (isOk)
                    {
                        // Does the session metadata file exist?
                        bool isMetadataExists = rgUtils::IsFileExists(fullMetadataFilePath);
                        if (isMetadataExists)
                        {
                            // The session-metadata file exists, meaning the project has been built previously.
                            std::shared_ptr<rgCliBuildOutput> pGpuOutput = nullptr;
                            isLoaded = rgXMLSessionConfig::ReadSessionMetadata(fullMetadataFilePath, pGpuOutput);
                            assert(isLoaded);
                            if (isLoaded)
                            {
                                assert(pGpuOutput != nullptr);
                                if (pGpuOutput != nullptr)
                                {
                                    // Add the outputs to the map to store per-GPU results.
                                    m_buildOutputs[currentGpu] = pGpuOutput;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return isLoaded;
}

bool rgBuildView::LoadProjectFile(const std::string& projectFilePath)
{
    bool ret = false;

    rgConfigManager& configManager = rgConfigManager::Instance();

    // Reset the view state before loading the project file.
    ResetView();

    // Get the configuration manager to load a project file.
    m_pProject = configManager.LoadProjectFile(projectFilePath);

    // Update the window title if the project loaded correctly.
    assert(m_pProject != nullptr);
    if (m_pProject != nullptr)
    {
        if (m_pFileMenuTitlebar != nullptr)
        {
            // Set project name title in file menu.
            std::stringstream title;
            title << rgUtils::GetProjectTitlePrefix(m_pProject->m_api) << m_pProject->m_projectName;
            m_pFileMenuTitlebar->SetTitle(title.str().c_str());
        }

        // Signal that a new project has been loaded into the rgBuildView.
        emit ProjectLoaded(m_pProject);

        // Create a new build settings view after a new project has been created.
        CreateBuildSettingsView();

        ret = true;
    }
    else
    {
        // Tell the user that the project file failed to load.
        std::stringstream errorStream;
        errorStream << STR_ERR_CANNOT_LOAD_PROJECT_FILE << " ";
        errorStream << projectFilePath;
        rgUtils::ShowErrorMessageBox(errorStream.str().c_str(), this);
    }

    return ret;
}

void rgBuildView::SaveCurrentFile()
{
    bool isSourceCodeEditorValid = (m_pCurrentCodeEditor != nullptr);
    assert(isSourceCodeEditorValid);
    if (isSourceCodeEditorValid)
    {
        std::string currentFilename = m_pFileMenu->GetSelectedFilePath();

        // Ask the user for a filename if none exists so far.
        if (currentFilename.empty())
        {
            currentFilename = QFileDialog::getSaveFileName(this, STR_FILE_DIALOG_SAVE_NEW_FILE,
                rgConfigManager::Instance().GetLastSelectedFolder().c_str(), STR_FILE_DIALOG_CL_FILTER).toStdString();

            // Extract directory from full path.
            std::string fileDirectory;
            bool isOk = rgUtils::ExtractFileDirectory(currentFilename, fileDirectory);
            assert(isOk);

            if (isOk)
            {
                // Update last selected directory in global config.
                std::shared_ptr<rgGlobalSettings> pGlobalConfig = rgConfigManager::Instance().GetGlobalConfig();
                pGlobalConfig->m_lastSelectedDirectory = fileDirectory;
            }
        }

        // Write the editor text to file if the file path is valid.
        if (!currentFilename.empty())
        {
            SaveEditorTextToFile(m_pCurrentCodeEditor, currentFilename);
        }
    }
}

rgUnsavedItemsDialog::UnsavedFileDialogResult rgBuildView::RequestSaveFile(const std::string& fullPath)
{
    QStringList unsavedFiles;

    // Get editor that corresponds to this filepath.
    rgSourceCodeEditor* pEditor = GetEditorForFilepath(fullPath);

    bool isEditorValid = (pEditor != nullptr);
    assert(isEditorValid);

    // Add file to the list if it is unsaved (modified).
    if (isEditorValid && pEditor->document()->isModified())
    {
        unsavedFiles << fullPath.c_str();
    }

    // Ask the user to save edited files.
    rgUnsavedItemsDialog::UnsavedFileDialogResult userResponse = RequestSaveFiles(unsavedFiles);
    return userResponse;
}

bool rgBuildView::ShowSaveDialog(rgFilesToSave filesToSave)
{
    QStringList unsavedFiles;

    if (filesToSave == rgFilesToSave::SourceFiles || filesToSave == rgFilesToSave::All)
    {
        // Add unsaved source files to the list of files that must be saved.
        GetUnsavedSourceFiles(unsavedFiles);
    }

    // Does the user have pending Build Settings changes to save?
    bool pendingBuildSettingsChanges = false;

    // If the build settings have been modified but the changes are still pending, add the build settings file to the list.
    rgOpenCLBuildSettingsView* pBuildSettingsView = static_cast<rgOpenCLBuildSettingsView*>(m_pBuildSettingsView);
    assert(pBuildSettingsView != nullptr);
    if (pBuildSettingsView != nullptr)
    {
        if (filesToSave == rgFilesToSave::BuildSettings || filesToSave == rgFilesToSave::All)
        {
            pendingBuildSettingsChanges = pBuildSettingsView->GetHasPendingChanges();
            if (pendingBuildSettingsChanges)
            {
                // Add a build settings item to the unsaved files list.
                unsavedFiles << STR_MENU_BUILD_SETTINGS;
            }
        }
    }

    // Ask the user if they want save files with modifications.
    rgUnsavedItemsDialog::UnsavedFileDialogResult saveResult = rgUnsavedItemsDialog::Yes;
    if (!unsavedFiles.empty())
    {
        saveResult = RequestSaveFiles(unsavedFiles);
        switch (saveResult)
        {
        case rgUnsavedItemsDialog::No:
            {
                if (pendingBuildSettingsChanges)
                {
                    // If the user clicks "No," they don't care about the pending changes. Revert them before moving on.
                    pBuildSettingsView->RevertPendingChanges();
                }
            }
            break;
        case rgUnsavedItemsDialog::Cancel:
            break;
        default:
            break;
        }
    }

    // If "Yes", proceed with the build. If "No", proceed with the build since the pending settings have been reverted.
    // If "Cancel," stop the attempt to build and continue where the user left off.
    return (saveResult == rgUnsavedItemsDialog::Yes || saveResult == rgUnsavedItemsDialog::No);
}

rgUnsavedItemsDialog::UnsavedFileDialogResult rgBuildView::RequestSaveFiles(const QStringList& unsavedFiles)
{
    rgUnsavedItemsDialog::UnsavedFileDialogResult result = rgUnsavedItemsDialog::Cancel;

    // Don't display the dialog if there are no files, just return.
    if (unsavedFiles.size() > 0)
    {
        // Create a modal unsaved file dialog.
        rgUnsavedItemsDialog* pUnsavedFileDialog = new rgUnsavedItemsDialog(this);
        pUnsavedFileDialog->setModal(true);
        pUnsavedFileDialog->setWindowTitle(STR_UNSAVED_ITEMS_DIALOG_TITLE);

        // Add unsaved files to the dialog list.
        pUnsavedFileDialog->AddFiles(unsavedFiles);

        // Register the dialog with the scaling manager.
        pUnsavedFileDialog->show();
        ScalingManager::Get().RegisterObject(pUnsavedFileDialog);

        // Center the dialog on the view (registering with the scaling manager
        // shifts it out of the center so we need to manually center it).
        rgUtils::CenterOnWidget(pUnsavedFileDialog, this);

        // Execute the dialog and get the result.
        result = static_cast<rgUnsavedItemsDialog::UnsavedFileDialogResult>(pUnsavedFileDialog->exec());

        switch (result)
        {
        case rgUnsavedItemsDialog::Yes:
            // Save all files and indicated dialog accepted.
            SaveFiles(unsavedFiles);
            break;
            // If the user chooses No or Cancel, nothing needs to happen except for making the dialog disappear.
        case rgUnsavedItemsDialog::No:
        case rgUnsavedItemsDialog::Cancel:
            break;
        default:
            // Shouldn't get here.
            assert(false);
        }
    }
    else
    {
        // No files need to be saved because none have been modified.
        result = rgUnsavedItemsDialog::Yes;
    }

    return result;
}

void rgBuildView::SaveFiles(const QStringList& unsavedFiles)
{
    // Step through each of the files with pending changes.
    auto stringListIter = unsavedFiles.begin();
    while (stringListIter != unsavedFiles.end())
    {
        // If the file isn't the Build Settings item, it's a path to an input source file.
        const std::string& filePath = stringListIter->toStdString();
        if (filePath.compare(STR_MENU_BUILD_SETTINGS) == 0)
        {
            // Submit all pending changes and save the build settings file.
            rgOpenCLBuildSettingsView* pBuildSettingsView = static_cast<rgOpenCLBuildSettingsView*>(m_pBuildSettingsView);
            assert(pBuildSettingsView != nullptr);
            if (pBuildSettingsView != nullptr)
            {
                pBuildSettingsView->SaveSettings();
            }
        }
        else
        {
            SaveSourceFile(stringListIter->toStdString().c_str());
        }

        stringListIter++;
    }
}

void rgBuildView::SaveSourceFile(const std::string& sourceFilePath)
{
    rgSourceCodeEditor* pEditor = GetEditorForFilepath(sourceFilePath);

    bool isEditorValid = (pEditor != nullptr);
    assert(isEditorValid);

    // Save the editor text.
    if (isEditorValid)
    {
        SaveEditorTextToFile(pEditor, sourceFilePath);
    }
}

bool rgBuildView::RequestRemoveAllFiles()
{
    bool isSaveAccepted = ShowSaveDialog();
    if (isSaveAccepted)
    {
        // Remove all file menu items.
        auto editorIter = m_sourceCodeEditors.begin();
        while (editorIter != m_sourceCodeEditors.end())
        {
            std::string fullPath = editorIter->first;
            m_pFileMenu->RemoveItem(fullPath);
            RemoveEditor(fullPath);

            // Keep getting first item until all are removed.
            editorIter = m_sourceCodeEditors.begin();
        }
    }

    return isSaveAccepted;
}

void rgBuildView::SetSourceCodeText(const std::string& fileFullPath)
{
    QString srcCode;

    if (m_pCurrentCodeEditor != nullptr)
    {
        if (!fileFullPath.empty())
        {
            bool isOk = rgUtils::ReadTextFile(fileFullPath, srcCode);
            if (isOk)
            {
                // Save the current line number and vertical scroll position.
                int currentLineNumber = m_pCurrentCodeEditor->GetSelectedLineNumber();
                const int vScrollPosition = m_pCurrentCodeEditor->verticalScrollBar()->value();

                // Set the text.
                m_pCurrentCodeEditor->setText(srcCode);

                // Remember most recent time file was modified.
                QFileInfo fileInfo(fileFullPath.c_str());
                m_fileModifiedTimeMap[m_pCurrentCodeEditor] = fileInfo.lastModified();

                // Indicate that a freshly loaded file is considered unmodified.
                m_pCurrentCodeEditor->document()->setModified(false);

                // Set the highlighted line.
                QList<int> lineNumbers;
                lineNumbers << currentLineNumber;
                m_pCurrentCodeEditor->SetHighlightedLines(lineNumbers);

                // Restore the cursor position after reloading the file.
                QTextCursor cursor(m_pCurrentCodeEditor->document()->findBlockByLineNumber(currentLineNumber - 1));
                m_pCurrentCodeEditor->setTextCursor(cursor);
                if (currentLineNumber <= m_pCurrentCodeEditor->document()->blockCount())
                {
                    m_pCurrentCodeEditor->verticalScrollBar()->setValue(vScrollPosition);
                }
            }
        }
        else
        {
            m_pCurrentCodeEditor->setText("");
            m_pCurrentCodeEditor->document()->setModified(false);
        }
    }
}

void rgBuildView::ShowCurrentFileDisassembly()
{
    bool isCurrentFileDisassembled = false;

    // Show the currently selected file's first entrypoint disassembly (if there is no currently selected entry).
    const std::string& inputFilepath = m_pFileMenu->GetSelectedFilePath();
    rgFileMenuFileItem* pSelectedFileItem = m_pFileMenu->GetSelectedFileItem();
    if (pSelectedFileItem != nullptr)
    {
        std::string currentEntrypointName;
        bool isEntrySelected = pSelectedFileItem->GetSelectedEntrypointName(currentEntrypointName);
        // Get the list of entrypoint names for the selected input file.
        std::vector<std::string> entrypointNames;
        pSelectedFileItem->GetEntrypointNames(entrypointNames);

        // Select the first available entrypoint if any exist.
        if (!entrypointNames.empty())
        {
            // Show the first entrypoint in the disassembly table.
            std::string& entrypointName = (isEntrySelected ? currentEntrypointName : entrypointNames[0]);
            m_pDisassemblyView->HandleSelectedEntrypointChanged(m_currentTargetGpu, inputFilepath, entrypointName);

            // Emit a signal indicating that the selected entrypoint has changed.
            emit SelectedEntrypointChanged(m_currentTargetGpu, inputFilepath, entrypointName);

            isCurrentFileDisassembled = true;
        }
    }

    // Toggle the view based on if the current file has been disassembled or not.
    ToggleDisassemblyViewVisibility(isCurrentFileDisassembled);
}

void rgBuildView::ToggleDisassemblyViewVisibility(bool isVisible)
{
    bool isViewCreated = m_pDisassemblyViewContainer != nullptr && m_pDisassemblyView != nullptr;

    // If the disassembly view is being hidden, the splitter may have to be restored before hiding the view.
    if (!isVisible)
    {
        assert(m_pDisassemblyViewSplitter != nullptr);
        if (m_pDisassemblyViewSplitter != nullptr)
        {
            // Is there a container that's currently maximized in the splitter?
            QWidget* pMaximizedWidget = m_pDisassemblyViewSplitter->GetMaximizedWidget();
            if (pMaximizedWidget == m_pDisassemblyViewContainer)
            {
                // Restore the maximized view before switching to the build settings.
                m_pDisassemblyViewSplitter->Restore();
            }
        }
    }

    if (isVisible)
    {
        // The view needs to exist if we try to make it visible.
        assert(isViewCreated);
    }

    if (isViewCreated)
    {
        // Show the disassembly view container to display the tables within.
        m_pDisassemblyViewContainer->setVisible(isVisible);
    }

    assert(m_pSourceViewContainer != nullptr);
    if (m_pSourceViewContainer != nullptr)
    {
        // Only allow the source editor container to be maximized/restored when the disassembly view is available.
        m_pSourceViewContainer->SetIsMaximizable(isVisible);
    }
}

void rgBuildView::HandleSelectedEntrypointChanged(const std::string& inputFilePath, const std::string& selectedEntrypointName)
{
    assert(m_pFileMenu != nullptr);
    if (m_pFileMenu != nullptr)
    {
        // Trigger the file menu to be updated, which will change the current selection in the current item's entrypoint list.
        m_pFileMenu->HandleSelectedEntrypointChanged(inputFilePath, selectedEntrypointName);
    }

    // Highlight the start line for the given entrypoint in the source editor.
    HighlightEntrypointStartLine(inputFilePath, selectedEntrypointName);
}

void rgBuildView::HandleSourceEditorHidden()
{
    // Hide the find widget.
    ToggleFindWidgetVisibility(false);
}

void rgBuildView::HandleSourceEditorResized()
{
    if (m_pFindWidget != nullptr)
    {
        UpdateFindWidgetGeometry();
    }
}

void rgBuildView::HandleSelectedFileChanged(const std::string& oldFilepath, const std::string& newFilepath)
{
    // Get a pointer to the editor responsible for displaying the new file.
    rgSourceCodeEditor* pEditor = GetEditorForFilepath(newFilepath);
    assert(pEditor != nullptr);
    if (pEditor != nullptr)
    {
        bool isEditorSwitched = SwitchToEditor(pEditor);
        if (isEditorSwitched)
        {
            // Switch the disassembly view to show the currently-selected entrypoint in the newly-selected file item.
            if (m_pDisassemblyView != nullptr && !m_isBuildInProgress)
            {
                // Open the disassembly view for the source file only if it's disassembled.
                if (IsFileDisassembled(newFilepath))
                {
                    rgFileMenuFileItem* pFileItem = m_pFileMenu->GetFileItemFromPath(newFilepath);

                    assert(pFileItem != nullptr);
                    if (pFileItem != nullptr)
                    {
                        // Retrieve the name of the currently-selected entrypoint (if there is one).
                        std::string selectedEntrypointName;
                        bool isEntrypointSelected = pFileItem->GetSelectedEntrypointName(selectedEntrypointName);

                        // Update the visibility of the disassembly view.
                        ToggleDisassemblyViewVisibility(isEntrypointSelected);

                        if (isEntrypointSelected)
                        {
                            // Make the disassembly view visible because the file has build outputs to display.
                            emit SelectedEntrypointChanged(m_currentTargetGpu, newFilepath, selectedEntrypointName);
                        }

                        // Update the titlebar for the current source editor.
                        UpdateSourceEditorTitlebar(pEditor);
                    }
                }
                else
                {
                    // Hide the disassembly view when switching to a file that hasn't been disassembled.
                    ToggleDisassemblyViewVisibility(false);
                }
            }
        }
    }
}

void rgBuildView::HandleHighlightedCorrelationLineUpdated(int lineNumber)
{
    // A list that gets filled with correlated line numbers to highlight in the source editor.
    QList<int> highlightedLines;

    // Only fill up the list with valid correlated lines if possible. Otherwise, nothing will get highlighted.
    bool isCorrelationEnabled = IsLineCorrelationEnabled(m_pCurrentCodeEditor);
    if (isCorrelationEnabled)
    {
        // Only scroll to the highlighted line if it's a valid line number.
        if (lineNumber != kInvalidCorrelationLineIndex)
        {
            highlightedLines.push_back(lineNumber);

            // Scroll the source editor to show the highlighted line.
            m_pCurrentCodeEditor->ScrollToLine(lineNumber);
        }
    }

    // Add the correlated input source line number to the editor's highlight list.
    m_pCurrentCodeEditor->SetHighlightedLines(highlightedLines);
}

void rgBuildView::HandleIsLineCorrelationEnabled(rgSourceCodeEditor* pEditor, bool isEnabled)
{
    // Update the correlation state flag.
    std::string filePath = GetFilepathForEditor(pEditor);
    UpdateSourceFileCorrelationState(filePath, isEnabled);

    if (!isEnabled)
    {
        // Invalidate the highlighted correlation lines within the source editor and disassembly views.
        HandleSourceFileSelectedLineChanged(pEditor, kInvalidCorrelationLineIndex);
        HandleHighlightedCorrelationLineUpdated(kInvalidCorrelationLineIndex);
    }
    else
    {
        // Use the currently-selected line in the source editor to highlight correlated lines in the disassembly table.
        assert(pEditor != nullptr);
        if (pEditor != nullptr)
        {
            // Trigger a line correlation update by re-selecting the current line in the source editor.
            int selectedLineNumber = pEditor->GetSelectedLineNumber();
            HandleSourceFileSelectedLineChanged(pEditor, selectedLineNumber);
        }
    }

    // Update the editor's titlebar text.
    if (m_pCurrentCodeEditor == pEditor)
    {
        UpdateSourceEditorTitlebar(pEditor);
    }
}

void rgBuildView::HandleSourceFileSelectedLineChanged(rgSourceCodeEditor* pEditor, int lineNumber)
{
    // Handle updating source correlation only when the project isn't currently being built.
    if (!m_isBuildInProgress)
    {
        if (m_pDisassemblyView != nullptr && !m_pDisassemblyView->IsEmpty())
        {
            const std::string& inputFilename = GetFilepathForEditor(pEditor);
            bool isDisassembled = IsFileDisassembled(inputFilename);
            if (isDisassembled)
            {
                int correlatedLineNumber = kInvalidCorrelationLineIndex;

                bool isCorrelationEnabled = IsLineCorrelationEnabled(pEditor);
                if (isCorrelationEnabled)
                {
                    correlatedLineNumber = lineNumber;
                }

                // If the line is associated with a named entrypoint, highlight it in the file menu item.
                std::string entryName;
                bool isValid = GetEntrypointNameForLineNumber(inputFilename, lineNumber, entryName);
                if (isValid)
                {
                    m_pFileMenu->HandleSelectedEntrypointChanged(inputFilename, entryName);
                }

                // Send the input source file's correlation line index to the disassembly view.
                m_pDisassemblyView->HandleInputFileSelectedLineChanged(m_currentTargetGpu, inputFilename, entryName, correlatedLineNumber);
            }
        }
    }
}

void rgBuildView::HandleBuildSettingsPendingChangesStateChanged(bool hasPendingChanges)
{
    // Pull the build settings menu item out of the file menu.
    rgFileMenuBuildSettingsItem* pBuildSettingsMenuItem = m_pFileMenu->GetBuildSettingsItem();
    assert(pBuildSettingsMenuItem != nullptr);

    // Toggle the pending changed flag.
    if (pBuildSettingsMenuItem != nullptr)
    {
        pBuildSettingsMenuItem->SetHasPendingChanges(hasPendingChanges);

        // Update the file menu save build settings item visibility.
        emit CurrentEditorModificationStateChanged(hasPendingChanges);
    }

    // Set the enabledness of the "Save" button.
    assert(m_pSettingsButtonsView != nullptr);
    if (m_pSettingsButtonsView != nullptr)
    {
        m_pSettingsButtonsView->EnableSaveButton(hasPendingChanges);
    }
}

void rgBuildView::HandleBuildSettingsSaved(std::shared_ptr<rgBuildSettings> pBuildSettings)
{
    assert(m_pProject != nullptr);
    if (m_pProject != nullptr)
    {
        bool projectHasClones = !m_pProject->m_clones.empty();
        assert(projectHasClones);

        if (projectHasClones)
        {
            // Replace the clone's build settings with the latest updated settings.
            m_pProject->m_clones[m_cloneIndex]->m_pBuildSettings = pBuildSettings;

            // Save the project after adding a source file.
            rgConfigManager::Instance().SaveProjectFile(m_pProject);
        }
    }

    // Signal that the build settings have changed and should be saved.
    emit ProjectBuildSettingsSaved(pBuildSettings);
}

void rgBuildView::HandleSelectedTargetGpuChanged(const std::string& targetGpu)
{
    // Look for build output for the target GPU being switched to.
    auto targetGpuBuildOutputs = m_buildOutputs.find(targetGpu);

    // Find the build outputs for the given GPU.
    bool isValidTargetGpu = targetGpuBuildOutputs != m_buildOutputs.end();
    assert(isValidTargetGpu);
    if (isValidTargetGpu)
    {
        // Switch the target GPU.
        m_currentTargetGpu = targetGpu;

        std::shared_ptr<rgCliBuildOutput> pFileOutputs = targetGpuBuildOutputs->second;
        assert(pFileOutputs != nullptr);
        if (pFileOutputs != nullptr)
        {
            assert(m_pCurrentCodeEditor != nullptr);
            if (m_pCurrentCodeEditor != nullptr)
            {
                std::string currentFilePath = GetFilepathForEditor(m_pCurrentCodeEditor);

                // Does the currently-selected source file have build output for the new Target GPU?
                auto sourceFileOutputsIter = pFileOutputs->m_perFileOutput.find(currentFilePath);

                // Trigger an update to handle highlighting the correlated disassembly
                // lines associated with the selected line in the current file.
                bool isFileBuiltForTarget = sourceFileOutputsIter != pFileOutputs->m_perFileOutput.end();
                if (isFileBuiltForTarget)
                {
                    // Use the currently-selected line in the source editor to highlight correlated lines in the disassembly table.
                    int selectedLineNumber = m_pCurrentCodeEditor->GetSelectedLineNumber();
                    HandleSourceFileSelectedLineChanged(m_pCurrentCodeEditor, selectedLineNumber);
                }
            }
        }
    }
}

void rgBuildView::HandleFileRenamed(const std::string& oldFilepath, const std::string& newFilepath)
{
    // Update references to the old file path within the BuildView.
    RenameFile(oldFilepath, newFilepath);

    // Emit a signal to trigger the file rename within the project file.
    emit FileRenamed(oldFilepath, newFilepath);

    // If the paths match, the file just finished being renamed from the file menu.
    if (oldFilepath.compare(newFilepath) == 0)
    {
        // Switch the focus from the file menu item to the source editor.
        if (m_pCurrentCodeEditor != nullptr)
        {
            m_pCurrentCodeEditor->setFocus();
        }
    }
}

void rgBuildView::HandleFocusNextView()
{
    // Manually advance to the next view within the rgViewManager.
    m_pViewManager->FocusNextView();
}

void rgBuildView::HandleFindWidgetVisibilityToggled()
{
    if (m_pFindWidget != nullptr)
    {
        bool isVisible = m_pFindWidget->isVisible();
        ToggleFindWidgetVisibility(!isVisible);
    }
}

void rgBuildView::HandleProjectRenamed(const std::string& projectName)
{
    // Get current project directory.
    std::string directory;
    rgUtils::ExtractFileDirectory(m_pProject->m_projectFileFullPath, directory);

    // Create full path by appending new name to directory.
    char separator = static_cast<char>(QDir::separator().unicode());
    std::string fullPath = directory + separator + projectName + STR_PROJECT_FILE_EXTENSION;

    // Rename the project.
    RenameProject(fullPath);
}

void rgBuildView::HandleEditorModificationStateChanged(bool isModified)
{
    // Get the sender of the signal.
    rgSourceCodeEditor* pEditor = static_cast<rgSourceCodeEditor*>(sender());

    // Get the file path for the given editor instance.
    std::string fullFilePath = GetFilepathForEditor(pEditor);

    // Set menu item saved state.
    m_pFileMenu->SetItemIsSaved(fullFilePath, !isModified);

    // If the editor being modified is the current one, emit the modification signal.
    if (pEditor == m_pCurrentCodeEditor)
    {
        emit CurrentEditorModificationStateChanged(isModified);
    }

    // Was a source file modified and saved after the last build time?
    bool filesModifiedAfterBuild = CheckSourcesModifiedSinceLastBuild(pEditor);
    if (filesModifiedAfterBuild)
    {
        emit LineCorrelationEnabledStateChanged(pEditor, false);
    }
    else
    {
        emit LineCorrelationEnabledStateChanged(pEditor, !isModified);
    }
}

void rgBuildView::HandleMenuItemCloseButtonClicked(const std::string fullPath)
{
    if (!fullPath.empty())
    {
        std::stringstream msg;
        msg << fullPath << STR_MENU_BAR_CONFIRM_REMOVE_FILE_DIALOG_WARNING;

        // Ask the user if they're sure they want to remove the file.
        bool shouldRemove = rgUtils::ShowConfirmationMessageBox(STR_MENU_BAR_CONFIRM_REMOVE_FILE_DIALOG_TITLE, msg.str().c_str(), this);

        // Ask the user if we should save the changes. Continue only if the user did not ask to cancel the operation.
        shouldRemove = shouldRemove && (RequestSaveFile(fullPath) != rgUnsavedItemsDialog::Cancel);
        if (shouldRemove)
        {
            // Remove the input file from the rgBuildView.
            RemoveInputFile(fullPath);
        }
    }
}

void rgBuildView::HandleFindTriggered()
{
    assert(m_pCurrentCodeEditor != nullptr);
    if (m_pCurrentCodeEditor != nullptr)
    {
        // Trigger showing the find widget only when the source editor is visible.
        if (m_pCurrentCodeEditor->isVisible())
        {
            // Create the find widget if it doesn't already exist.
            if (m_pFindWidget == nullptr)
            {
                CreateFindWidget();
            }

            // Toggle to "Find" mode, and show the find widget.
            if (m_pFindWidget != nullptr)
            {
                ToggleFindWidgetVisibility(true);
            }
        }
    }
}

void rgBuildView::HandleIsBuildInProgressChanged(bool isBuilding)
{
    m_isBuildInProgress = isBuilding;
}

void rgBuildView::HandleProjectBuildSuccess()
{
    // Reset the project building flag.
    HandleIsBuildInProgressChanged(false);

    // Update the last successful build time to now.
    m_lastSuccessfulBuildTime = QDateTime::currentDateTime();

    // Invoke the CLI to load the start line numbers for each entrypoint.
    LoadEntrypointLineNumbers();

    assert(m_pFileMenu != nullptr);
    if (m_pFileMenu != nullptr)
    {
        // Allow the user to expand the file's entrypoint list.
        m_pFileMenu->SetIsShowEntrypointListEnabled(true);

        // Update the file menu item with the clone's build output.
        m_pFileMenu->UpdateBuildOutput(m_buildOutputs);
    }

    // The current project was built successfully. Open the disassembly view with the results.
    bool isDisassemblyLoaded = LoadDisassemblyFromBuildOutput();
    assert(isDisassemblyLoaded);
    if (isDisassemblyLoaded)
    {
        // Switch the edit mode to display input source code alongside disassembly.
        if (m_editMode != EditMode::SourceCode)
        {
            SwitchEditMode(EditMode::SourceCode);
        }

        ShowCurrentFileDisassembly();
    }

    assert(m_pCurrentCodeEditor != nullptr);
    if (m_pCurrentCodeEditor != nullptr)
    {
        // Use the currently-selected line in the source editor to highlight correlated lines in the disassembly table.
        int selectedLineNumber = m_pCurrentCodeEditor->GetSelectedLineNumber();
        HandleSourceFileSelectedLineChanged(m_pCurrentCodeEditor, selectedLineNumber);
    }

    std::string outputGpu;
    std::shared_ptr<rgCliBuildOutput> pBuildOutput = nullptr;
    bool isOutputValid = rgUtils::GetFirstValidOutputGpu(m_buildOutputs, outputGpu, pBuildOutput);
    if (isOutputValid && pBuildOutput != nullptr)
    {
        // Store the path to the current source file using the file menu.
        std::string currentSourceFilePath;
        if (m_pFileMenu != nullptr)
        {
            currentSourceFilePath = m_pFileMenu->GetSelectedFilePath();
        }

        // Enable line correlation for all source files that were built successfully.
        auto sourcePathIterStart = pBuildOutput->m_perFileOutput.begin();
        auto sourcePathIterEnd = pBuildOutput->m_perFileOutput.end();
        for (auto inputFilePathIter = sourcePathIterStart; inputFilePathIter != sourcePathIterEnd; ++inputFilePathIter)
        {
            // Get a pointer to the source editor for each input file.
            const std::string& sourceFilePath = inputFilePathIter->first;

            // Skip updating the current file within the loop. It  will be updated last.
            if (currentSourceFilePath.compare(sourceFilePath) != 0)
            {
                // Only update the correlation if the source file still exists in the project.
                // Previously-built files that have already been removed from the project may have artifacts loaded.
                auto editorIter = m_sourceCodeEditors.find(sourceFilePath);
                if (editorIter != m_sourceCodeEditors.end())
                {
                    rgSourceCodeEditor* pEditor = GetEditorForFilepath(sourceFilePath);

                    // Emit the signal used to update the correlation enabledness.
                    emit LineCorrelationEnabledStateChanged(pEditor, true);
                }
            }
        }

        // Update the currently selected file last.
        rgSourceCodeEditor* pEditor = GetEditorForFilepath(currentSourceFilePath);
        assert(pEditor != nullptr);
        if (pEditor != nullptr)
        {
            // Emit the signal used to update the correlation enabledness.
            emit LineCorrelationEnabledStateChanged(pEditor, true);
        }
    }

    // Resize the disassembly view.
    HandleDisassemblyTableWidthResizeRequested(0);
}

void rgBuildView::HandleApplicationStateChanged(Qt::ApplicationState state)
{
    // When the application becomes active, check for external file modifications.
    if (state == Qt::ApplicationActive)
    {
        CheckExternalFileModification();
    }
}

void rgBuildView::HandleDisassemblyTableWidthResizeRequested(int minimumWidth)
{
    assert(m_pDisassemblyView != nullptr);
    assert(m_pDisassemblyViewContainer != nullptr);
    assert(m_pDisassemblyViewSplitter != nullptr);

    // Before resizing the disassembly table, make sure that it is not in maximized state.
    if (m_pDisassemblyView != nullptr && m_pDisassemblyViewContainer != nullptr
        && m_pDisassemblyViewSplitter != nullptr && !m_pDisassemblyViewContainer->IsInMaximizedState())
    {
        // Add a small portion of extra buffer space to the right side of the table.
        static const float RESIZE_EXTRA_MARGIN = 1.5f;
        minimumWidth = static_cast<int>(minimumWidth * RESIZE_EXTRA_MARGIN);

        QRect resourceUsageTextBounds;
        m_pDisassemblyView->GetResourceUsageTextBounds(resourceUsageTextBounds);

        // Set maximum width for the widgets containing the ISA disassembly table.
        const int resourceUsageStringWidth = resourceUsageTextBounds.width();
        const int maximumWidth = resourceUsageStringWidth > minimumWidth ? resourceUsageStringWidth : minimumWidth;

        int splitterWidth = m_pDisassemblyViewSplitter->size().width();
        QList<int> splitterWidths;
        splitterWidths.push_back(splitterWidth - maximumWidth);
        splitterWidths.push_back(maximumWidth);

        // Set the ideal width for both sides of the splitter.
        m_pDisassemblyViewSplitter->setSizes(splitterWidths);
    }
}

void rgBuildView::HandleBuildSettingsMenuButtonClicked()
{
    OpenBuildSettings();
}

void rgBuildView::BuildCurrentProject()
{
    // The build settings must be saved in order to proceed with a build.
    bool shouldProceedWithBuild = ShowSaveDialog(rgFilesToSave::BuildSettings);
    if (shouldProceedWithBuild)
    {
        // Destroy outputs from previous builds.
        DestroyProjectBuildArtifacts();

        // Clear the output window.
        m_pCliOutputWindow->ClearText();

        // Don't allow the user to expand the entrypoint list for file items.
        assert(m_pFileMenu != nullptr);
        if (m_pFileMenu != nullptr)
        {
            m_pFileMenu->SetIsShowEntrypointListEnabled(false);
        }

        // Set the "is currently building" flag.
        HandleIsBuildInProgressChanged(true);

        // Notify the system that a build has started.
        emit ProjectBuildStarted();

        // The function that will be invoked by the build thread.
        auto backgroundTask = [&]
        {
            // Build an output path where all of the build artifacts will be dumped to.
            std::string outputPath = CreateProjectBuildOutputPath();

            // Create the output directory if it doesn't already exist.
            bool isOk = rgUtils::IsDirExists(outputPath);
            if (!isOk)
            {
                isOk = rgUtils::CreateFolder(outputPath);
                assert(isOk);
            }

            if (isOk)
            {
                // Create a new output folder specific to the current clone's build artifacts.
                std::stringstream cloneFolderName;
                cloneFolderName << STR_CLONE_FOLDER_NAME;
                cloneFolderName << m_cloneIndex;

                // Append the clone folder to the output folder.
                isOk = rgUtils::AppendFolderToPath(outputPath, cloneFolderName.str(), outputPath);
                assert(isOk);
                if (isOk)
                {
                    // Append a path separator to the new output path.
                    isOk = rgUtils::AppendPathSeparator(outputPath, outputPath);
                    assert(isOk);

                    // Create the output folder if it does not exist.
                    if (!rgUtils::IsDirExists(outputPath))
                    {
                        isOk = rgUtils::CreateFolder(outputPath);
                        assert(isOk);
                    }
                }
            }

            // If the correct build output paths exist, proceed with building the project.
            if (isOk)
            {
                // Set up the function pointer responsible for handling new output from the CLI invocation.
                using std::placeholders::_1;
                std::function<void(const std::string&)> appendBuildOutput = std::bind(&rgBuildView::HandleNewCLIOutputString, this, _1);

                // Build the current project clone.
                m_cancelBuildSignal = false;

                // Verify that the clone index is valid.
                int numClones = static_cast<int>(m_pProject->m_clones.size());
                bool isCloneIndexValid = (m_cloneIndex >= 0 && m_cloneIndex < numClones);
                assert(isCloneIndexValid);
                if (isCloneIndexValid)
                {
                    // Attempt to build the clone.
                    std::vector<std::string> gpusWithBuildOutputs;
                    bool isProjectBuilt = rgCliLauncher::BuildProjectClone(m_pProject, m_cloneIndex, outputPath, appendBuildOutput, gpusWithBuildOutputs, m_cancelBuildSignal);

                    // Verify that the build was not canceled.
                    if (!m_cancelBuildSignal)
                    {
                        // If the project was built successfully, parse the session metadata file and populate an rgCliOutput structure.
                        if (isProjectBuilt)
                        {
                            // Load the build outputs in the project's directory.
                            std::string projectDirectory;
                            bool isOk = rgUtils::ExtractFileDirectory(m_pProject->m_projectFileFullPath, projectDirectory);
                            if (isOk)
                            {
                                bool isOutputLoaded = LoadBuildOutput(projectDirectory, &gpusWithBuildOutputs);
                                assert(isOutputLoaded);
                                if (isOutputLoaded)
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
                        HandleNewCLIOutputString(STR_STATUS_BAR_BUILD_CANCELED);
                    }
                }
            }
        };

        // Launch the build thread.
        std::thread buildThread(backgroundTask);
        buildThread.detach();
    }
}

void rgBuildView::ResetView()
{
    // Nullify any previous project object.
    if (m_pProject != nullptr)
    {
        m_pProject = nullptr;
    }

    // Clear the view.
    ClearBuildView();
}

void rgBuildView::DestroyBuildOutputsForFile(const std::string& inputFileFullPath)
{
    bool isProjectEmpty = false;

    // Iterate through build output for all target GPUs.
    auto firstTargetGpu = m_buildOutputs.begin();
    auto lastTargetGpu = m_buildOutputs.end();
    for (auto targetGpuIter = firstTargetGpu; targetGpuIter != lastTargetGpu; ++targetGpuIter)
    {
        std::shared_ptr<rgCliBuildOutput> pBuildOutput = targetGpuIter->second;

        // Search for outputs for the given source file.
        assert(pBuildOutput != nullptr);
        if (pBuildOutput != nullptr)
        {
            auto inputFileOutputsIter = pBuildOutput->m_perFileOutput.find(inputFileFullPath);
            if (inputFileOutputsIter != pBuildOutput->m_perFileOutput.end())
            {
                // Step through all outputs associated with the given input source file.
                rgFileOutputs& fileOutputs = inputFileOutputsIter->second;
                for (const rgEntryOutput& entryOutput : fileOutputs.m_outputs)
                {
                    for (const rgOutputItem& outputItem : entryOutput.m_outputs)
                    {
                        // Destroy the output file.
                        QFile::remove(outputItem.m_filePath.c_str());
                    }
                }

                // Erase the source file's outputs from the existing build output structure.
                pBuildOutput->m_perFileOutput.erase(inputFileOutputsIter);
            }

            // Is this the last file being removed from the build output structure?
            if (pBuildOutput->m_perFileOutput.empty())
            {
                isProjectEmpty = true;
            }
        }
    }

    // Destroy all remnants of previous builds of the project.
    if (isProjectEmpty)
    {
        DestroyProjectBuildArtifacts();
    }
}

void rgBuildView::RemoveEditor(const std::string& filename)
{
    // Attempt to find the editor instance used to display the given file.
    rgSourceCodeEditor* pEditor = nullptr;
    auto editorIter = m_sourceCodeEditors.find(filename);
    if (editorIter != m_sourceCodeEditors.end())
    {
        pEditor = editorIter->second;
    }

    assert(pEditor != nullptr);

    // Remove the editor from the map, and hide it from the interface.
    QWidget* pTitleBar = m_pSourceViewContainer->GetTitleBar();
    rgSourceEditorTitlebar* pSourceViewTitleBar = qobject_cast<rgSourceEditorTitlebar*>(pTitleBar);
    if (pSourceViewTitleBar != nullptr)
    {
        pSourceViewTitleBar->SetTitlebarContentsVisibility(false);
    }

    m_sourceCodeEditors.erase(editorIter);

    if (pEditor == m_pCurrentCodeEditor)
    {
        // There is no more "Current Editor," because it is being closed.
        m_pCurrentCodeEditor->hide();
        m_pCurrentCodeEditor = nullptr;
    }

    // Destroy the editor associated with the file that was closed.
    pEditor->deleteLater();

    SwitchToFirstRemainingFile();
}

void rgBuildView::RemoveInputFile(const std::string& inputFileFullPath)
{
    rgConfigManager& configManager = rgConfigManager::Instance();

    // Remove the file from the project.
    configManager.RemoveSourceFilePath(m_pProject, m_cloneIndex, inputFileFullPath);
    configManager.SaveProjectFile(m_pProject);

    // Remove the file from the file menu.
    m_pFileMenu->RemoveItem(inputFileFullPath);

    // Remove the associated file editor.
    RemoveEditor(inputFileFullPath);

    // Clean up outputs from previous builds associated with this file.
    DestroyBuildOutputsForFile(inputFileFullPath);

    // Remove the file's build outputs from the disassembly view.
    if (m_pDisassemblyView != nullptr)
    {
        m_pDisassemblyView->RemoveInputFileEntries(inputFileFullPath);

        // Hide the disassembly view when there's no data in it.
        if (m_pDisassemblyView->IsEmpty())
        {
            // Minimize the disassembly view before hiding it to preserve correct rgBuildView layout.
            m_pDisassemblyViewSplitter->Restore();

            // Hide the disassembly view now that it's empty.
            ToggleDisassemblyViewVisibility(false);
        }
        else
        {
            // Trigger a correlation update after the source file has been removed.
            HandleSelectedTargetGpuChanged(m_currentTargetGpu);
        }
    }
}

bool rgBuildView::LoadDisassemblyFromBuildOutput()
{
    bool result = false;

    if (m_pDisassemblyViewContainer == nullptr)
    {
        // Create the ISA disassembly view.
        CreateIsaDisassemblyView();

        // Connect the disassembly view signals.
        ConnectDisassemblyViewSignals();
    }

    assert(m_pDisassemblyView != nullptr);
    if (m_pDisassemblyView != nullptr)
    {
        // Clear all previously loaded build output.
        m_pDisassemblyView->ClearBuildOutput();

        assert(m_pProject != nullptr);
        if (m_pProject != nullptr)
        {
            // Ensure that the incoming clone index is valid for the current project.
            bool isValidRange = (m_cloneIndex >= 0 && m_cloneIndex < m_pProject->m_clones.size());
            assert(isValidRange);

            if (isValidRange)
            {
                // Ensure that the clone is valid before creating the rgBuildSettingsView.
                std::shared_ptr<rgProjectClone> pClone = m_pProject->m_clones[m_cloneIndex];

                // Verify that the clone exists.
                assert(pClone != nullptr);
                if (pClone != nullptr)
                {
                    std::vector<rgSourceFileInfo>& projectSourceFiles = pClone->m_sourceFiles;

                    // Build artifacts may contain disassembly for source files that are no longer
                    // in the project, so provide a list of files to load, along with the current build output.
                    result = m_pDisassemblyView->PopulateDisassemblyView(projectSourceFiles, m_buildOutputs);
                }
            }
        }
    }

    return result;
}

bool rgBuildView::CanSwitchEditMode()
{
    bool ret = true;

    // If the user is currently viewing the Build Settings, ask them to save before changing the edit mode.
    if (m_editMode == EditMode::BuildSettings)
    {
        // Require the user to decide whether or not to save their Build Settings changes.
        ret = ShowSaveDialog(rgFilesToSave::BuildSettings);
    }

    return ret;
}

void rgBuildView::SwitchEditMode(EditMode mode)
{
    if (m_editMode != mode)
    {
        // Based on the incoming mode, hide/show specific widgets living within the BuildView.
        if (mode == EditMode::Empty)
        {
            // When empty, hide the code editor and build settings interfaces.
            if (m_pCurrentCodeEditor != nullptr)
            {
                m_pCurrentCodeEditor->hide();
            }

            // Hide the build settings view.
            m_pBuildSettingsWidget->hide();

            // Add an empty placeholder widget so the file menu stays remains on the left side.
            m_pSourceViewStack->layout()->addWidget(m_pEmptyPanel);
            m_pEmptyPanel->show();
        }
        else
        {
            // Hide the empty panel before showing anything.
            m_pEmptyPanel->hide();

            if (mode == EditMode::SourceCode)
            {
                // Enable maximizing the source editor/build settings container.
                assert(m_pSourceViewContainer != nullptr);
                if (m_pSourceViewContainer != nullptr)
                {
                    m_pSourceViewContainer->SetIsMaximizable(true);
                }

                // Hide the build settings, and show the code editor.
                if (m_pCurrentCodeEditor != nullptr)
                {
                    SetViewContentsWidget(m_pBuildSettingsWidget, m_pCurrentCodeEditor);
                }
            }
            else if (mode == EditMode::BuildSettings)
            {
                // Disable maximizing the source editor/build settings container.
                assert(m_pSourceViewContainer != nullptr);
                if (m_pSourceViewContainer != nullptr)
                {
                    m_pSourceViewContainer->SetIsMaximizable(false);
                }

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

                // Switch from showing the code editor, to showing the build settings.
                SetViewContentsWidget(m_pCurrentCodeEditor, m_pBuildSettingsWidget);

                // If the disassembly view exists, hide it.
                if (m_pDisassemblyViewContainer != nullptr)
                {
                    ToggleDisassemblyViewVisibility(false);
                }
            }

            // Update the file menu save text and action.
            emit EditModeChanged(mode);
        }

        // Update the current mode.
        m_editMode = mode;
    }
}

void rgBuildView::SetViewContentsWidget(QWidget* pOldContents, QWidget* pNewContents)
{
    // If the build settings are visible, hide them.
    if (pOldContents != nullptr)
    {
        pOldContents->hide();
    }

    // Verify that the new contents are valid.
    assert(pNewContents);
    if (pNewContents != nullptr)
    {
        // Add the new contents, and make it visible.
        m_pSourceViewStack->layout()->addWidget(pNewContents);
        pNewContents->show();

        // Use the active view as the focus proxy for the source view stack.
        m_pSourceViewStack->setFocusProxy(pNewContents);

        // Set focus to the new contents.
        pNewContents->setFocus();
    }
}

void rgBuildView::SwitchToFirstRemainingFile()
{
    // If there are code editors remaining, switch to the first remaining item.
    if (!m_sourceCodeEditors.empty())
    {
        m_pFileMenu->SelectLastRemainingItem();

        // Switch to viewing the rgSourceCodeEditor for the newly selected item.
        SwitchEditMode(EditMode::SourceCode);
    }
    else
    {
        // When the last file has been removed, the editor is in the empty state.
        SwitchEditMode(EditMode::Empty);
    }
}

bool rgBuildView::SwitchToEditor(rgSourceCodeEditor* pEditor)
{
    bool ret = false;

    assert(pEditor != nullptr);
    if (pEditor != nullptr)
    {
        // Verify if the user is allowed to switch to source editing mode.
        bool isSwitchingAllowed = CanSwitchEditMode();
        if (isSwitchingAllowed)
        {
            // Switch to the new editor.
            SetViewContentsWidget(m_pCurrentCodeEditor, pEditor);

            if (m_pCurrentCodeEditor != nullptr)
            {
                bool oldEditorIsModified = m_pCurrentCodeEditor->document()->isModified();
                bool newEditorIsModified = pEditor->document()->isModified();

                // Check if the new editor has a different modification state then the old one
                if (oldEditorIsModified != newEditorIsModified)
                {
                    emit CurrentEditorModificationStateChanged(newEditorIsModified);
                }
            }

            // The editor being switched to is now the current editor.
            m_pCurrentCodeEditor = pEditor;

            // The editor isn't empty, and will switch to displaying source code.
            SwitchEditMode(EditMode::SourceCode);

            // Check if the editor file has been modified externally.
            CheckExternalFileModification();

            // Update the find widget's searcher to search the new editor.
            if (m_pSourceSearcher != nullptr)
            {
                m_pSourceSearcher->SetTargetEditor(m_pCurrentCodeEditor);
            }

            ret = true;
        }
    }

    return ret;
}

bool rgBuildView::AddExistingSourcefileToProject(const std::string& sourceFilepath)
{
    bool ret = false;

    bool isProjectCreated = (m_pProject != nullptr);
    if (!isProjectCreated)
    {
        isProjectCreated = CreateNewEmptyProject();
    }

    if (isProjectCreated)
    {
        // Generate a path to where the new empty file will live in the project directory.
        rgConfigManager& configManager = rgConfigManager::Instance();

        std::string filename;
        bool isOk = rgUtils::ExtractFileName(sourceFilepath, filename, false);
        assert(isOk);
        if (isOk)
        {
            if (!m_pFileMenu->IsFileInMenu(filename))
            {
                // Add the source file's path to the project's clone.
                configManager.AddSourceFileToProject(sourceFilepath, m_pProject, m_cloneIndex);

                // Save the project after adding a sourcefile.
                configManager.SaveProjectFile(m_pProject);

                // Add the selected file to the menu.
                AddFile(sourceFilepath);

                // Set the source code view text with the contents of the selected file.
                SetSourceCodeText(sourceFilepath);

                // The file was added to the project successfully.
                ret = true;
            }

            if (!ret)
            {
                // Report the error.
                std::stringstream msg;
                msg << STR_ERR_CANNOT_ADD_FILE_A << sourceFilepath << STR_ERR_CANNOT_ADD_FILE_B;
                rgUtils::ShowErrorMessageBox(msg.str().c_str(), this);
            }
        }

        // Switch the focus to the file editor to begin editing the file.
        if (m_pCurrentCodeEditor != nullptr)
        {
            m_pCurrentCodeEditor->setFocus();
        }
    }

    return ret;
}

void rgBuildView::CreateBuildSettingsView()
{
    assert(m_pProject != nullptr);
    if (m_pProject != nullptr)
    {
        // Ensure that the incoming clone index is valid for the current project.
        bool isValidRange = (m_cloneIndex >= 0 && m_cloneIndex < m_pProject->m_clones.size());
        assert(isValidRange);

        if (isValidRange)
        {
            // Ensure that the clone is valid before creating the rgBuildSettingsView.
            std::shared_ptr<rgProjectClone> pClone = m_pProject->m_clones[m_cloneIndex];

            // Verify that the clone exists.
            assert(pClone != nullptr);
            if (pClone != nullptr)
            {
                assert(m_pFactory != nullptr);
                if (m_pFactory != nullptr)
                {
                    // Create the build settings interface.
                    m_pBuildSettingsView = m_pFactory->CreateBuildSettingsView(this, pClone->m_pBuildSettings, false);
                    assert(m_pBuildSettingsView != nullptr);

                    // If the build settings view was created successfully, connect the signals.
                    if (m_pBuildSettingsView != nullptr)
                    {
                        // Create the widget.
                        m_pBuildSettingsWidget = new rgBuildSettingsWidget(this);
                        m_pBuildSettingsWidget->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
                        m_pBuildSettingsWidget->setFrameStyle(QFrame::Box);
                        m_pBuildSettingsWidget->setLayout(new QVBoxLayout);

                        // Create the Settings buttons view.
                        m_pSettingsButtonsView = new rgSettingsButtonsView(this);

                        // Add various widgets to this tab.
                        m_pBuildSettingsWidget->layout()->addWidget(m_pBuildSettingsView);
                        m_pBuildSettingsWidget->layout()->addWidget(m_pSettingsButtonsView);

                        // Hide the build settings view after creating it.
                        m_pBuildSettingsWidget->hide();

                        // Connect signals for the build settings view.
                        ConnectBuildSettingsSignals();
                    }
                }
            }
        }
    }
}

void rgBuildView::CreateFindWidget()
{
    // Verify that this gets invoked only once.
    assert(m_pFindWidget == nullptr);
    if (m_pFindWidget == nullptr)
    {
        // Create a find widget searcher, and target the current source editor.
        m_pSourceSearcher = new rgSourceEditorSearcher();
        m_pSourceSearcher->SetTargetEditor(m_pCurrentCodeEditor);

        m_pFindWidget = new rgFindTextWidget(m_pSourceSearcher, this);
        ScalingManager::Get().RegisterObject(m_pFindWidget);

        ConnectFindSignals();
    }
}

void rgBuildView::CreateIsaDisassemblyView()
{
    m_pDisassemblyView = new rgIsaDisassemblyView(this);

    // Wrap the disassembly view in a view container.
    m_pDisassemblyViewContainer = new rgViewContainer();
    m_pDisassemblyViewContainer->SetMainWidget(m_pDisassemblyView);

    // Add the disassembly view to the disassembly splitter.
    m_pDisassemblyViewSplitter->AddMaximizableWidget(m_pDisassemblyViewContainer);

    // Hide the disassembly view when it is first created.
    m_pDisassemblyView->setVisible(true);

    // Add the view to the view manager.
    m_pViewManager->AddView(m_pDisassemblyViewContainer);
}

bool rgBuildView::CreateNewSourceFile(const std::string& sourceFileName, std::string& fullSourceFilepath)
{
    bool ret = false;

    // True if a new project was created.
    bool wasProjectCreated = false;

    // True if we are creating a new file in an existing project.
    bool isExistingProject = (m_pProject != nullptr);

    if (!isExistingProject)
    {
        wasProjectCreated = CreateNewEmptyProject();
    }

    if (isExistingProject || wasProjectCreated)
    {
        std::string filename = sourceFileName;

        // Is there an existing file with the same name already opened in the file menu?
        // If so, append a numbered suffix to the end of the filename to make it more unique.
        bool fileAlreadyOpened = false;
        assert(m_pFileMenu != nullptr);
        if (m_pFileMenu != nullptr)
        {
            fileAlreadyOpened = m_pFileMenu->IsFileInMenu(filename);

            // If a file with the same name is already opened within the file menu, generate a new unique filename.
            if (fileAlreadyOpened)
            {
                std::string uniqueFilename;
                m_pFileMenu->GenerateUniqueFilename(filename, uniqueFilename);

                // Add the new file with the generated unique filename.
                filename = uniqueFilename;
            }

            // Generate a path to where the new empty file will live in the projects directory.
            std::string newFileExtension;
            bool isOk = rgUtils::ProjectAPIToSourceFileExtension(m_pProject->m_api, newFileExtension);
            assert(isOk);
            if (isOk)
            {
                rgConfigManager& configManager = rgConfigManager::Instance();
                configManager.GenerateNewSourceFilepath(m_pProject->m_projectName, m_cloneIndex, filename, newFileExtension, fullSourceFilepath);

                // Ensure that the folder where the file will be saved already exists.
                std::string sourcefileFolder;
                rgUtils::ExtractFileDirectory(fullSourceFilepath, sourcefileFolder);
                if (!rgUtils::IsDirExists(sourcefileFolder))
                {
                    bool isDirCreated = rgUtils::CreateFolder(sourcefileFolder);
                    assert(isDirCreated);
                }

                // Create a new file at the target location.
                QFile emptyFile(fullSourceFilepath.c_str());
                emptyFile.open(QIODevice::ReadWrite);

                // Add the template source code to the newly created file.
                QTextStream stream(&emptyFile);
                stream << rgUtils::GenerateTemplateCode(m_pProject->m_api, filename).c_str() << endl;
                emptyFile.close();

                // Add the source file's path to the project's clone.
                configManager.AddSourceFileToProject(fullSourceFilepath, m_pProject, m_cloneIndex);

                // Save the project after adding a source file.
                rgConfigManager::Instance().SaveProjectFile(m_pProject);

                // We are done.
                ret = true;
            }
        }
    }

    return ret;
}

void rgBuildView::DestroyProjectBuildArtifacts()
{
    // Navigate to the current clone's build output and destroy all artifacts.
    std::string projectBuildOutputPath = CreateProjectBuildOutputPath();

    // Destroy all build artifacts in the project's output directory.
    QDir outputDir(projectBuildOutputPath.c_str());
    outputDir.removeRecursively();

    // Clear references to outputs from previous project compilations.
    m_buildOutputs.clear();

    if (m_pDisassemblyView != nullptr)
    {
        // Clear any disassembly tables already loaded into the view.
        m_pDisassemblyView->ClearBuildOutput();

        // Hide the disassembly view.
        ToggleDisassemblyViewVisibility(false);
    }

    // Clear any old build artifacts from the file menu.
    ClearFileItemsEntrypointList();
}

int rgBuildView::FindEntrypointStartLine(rgSourceCodeEditor* pEditor, int listKernelsStartLine) const
{
    int actualStartLine = listKernelsStartLine;

    // Verify that the current source editor is valid.
    assert(pEditor != nullptr);
    if (pEditor != nullptr)
    {
        // Start at the given line and search for the next opening brace. This is the "real" start of the entrypoint.
        int searchLine = actualStartLine;
        bool searchingForStartLine = true;
        while (searchingForStartLine)
        {
            QString lineText;
            bool gotLineText = pEditor->GetTextAtLine(searchLine, lineText);
            if (gotLineText)
            {
                // Start at the index of the brace, and check if there's anything else in the line other than whitespace.
                int braceIndex = lineText.indexOf("{");
                if (braceIndex != -1)
                {
                    std::string lineTextString = lineText.toStdString();

                    // Search for alphanumeric characters the occur after the opening brace.
                    auto startCharacter = lineTextString.begin() + (braceIndex + 1);
                    auto endCharacter = lineTextString.end();

                    // Create a list of bools, used to determine if there are any alphanumeric characters in the line after the opening brace.
                    std::vector<bool> isAlphanumericList;
                    isAlphanumericList.resize(lineTextString.size());

                    // Step through each character in the line. If it's alphanumeric, add a "true" to the output list in the character's position.
                    std::transform(startCharacter, endCharacter, isAlphanumericList.begin(), [](char c) { return (isalnum(c) != 0); });

                    // If there are any 'true' values in the isAlphanumericList, it means that an alphanumeric character appeared after the opening brace.
                    auto alphanumericCharactersIter = std::find(isAlphanumericList.begin(), isAlphanumericList.end(), true);
                    if (alphanumericCharactersIter == isAlphanumericList.end())
                    {
                        // There was only whitespace after the opening brace. Advance one more line to where the entrypoint actually starts.
                        searchLine++;
                    }

                    actualStartLine = searchLine;
                    searchingForStartLine = false;
                }
                else
                {
                    // Step down to the next line to check if there's an opening brace for the entrypoint.
                    searchLine++;
                }
            }
            else
            {
                searchingForStartLine = false;
            }
        }
    }

    return actualStartLine;
}

rgSourceCodeEditor* rgBuildView::GetEditorForFilepath(const std::string fullFilepath)
{
    // The source code editor to use for the given filename.
    rgSourceCodeEditor* pEditor = nullptr;

    if (!fullFilepath.empty())
    {
        auto editorIter = m_sourceCodeEditors.find(fullFilepath);
        if (editorIter != m_sourceCodeEditors.end())
        {
            pEditor = editorIter->second;
        }
        else
        {
            pEditor = new rgSourceCodeEditor(this);
            m_sourceCodeEditors[fullFilepath] = pEditor;

            QFileInfo fileInfo(fullFilepath.c_str());
            m_fileModifiedTimeMap[pEditor] = fileInfo.lastModified();

            ConnectSourcecodeEditorSignals(pEditor);
        }
    }
    return pEditor;
}

void rgBuildView::CancelCurrentBuild()
{
    // Destroy all project outputs when the project build is canceled.
    DestroyProjectBuildArtifacts();

    // Reset the "is currently building" flag.
    HandleIsBuildInProgressChanged(false);

    // Signal that this build should be canceled
    // (this handle is being watched by the thread that launches the CLI).
    m_cancelBuildSignal = true;

    assert(m_pFileMenu != nullptr);
    if (m_pFileMenu != nullptr)
    {
        // Don't allow the user to expand file item's entrypoint list.
        m_pFileMenu->SetIsShowEntrypointListEnabled(false);
    }
}

void rgBuildView::HandleScrollCodeEditorToLine(int lineNum)
{
    m_pCurrentCodeEditor->ScrollToLine(lineNum);
    QTextCursor cursor(m_pCurrentCodeEditor->document()->findBlockByLineNumber(lineNum - 1));
    m_pCurrentCodeEditor->setTextCursor(cursor);

    // Switch focus to the code editor.
    m_pCurrentCodeEditor->setFocus();
}

bool rgBuildView::LoadEntrypointLineNumbers()
{
    bool ret = false;

    // Destroy the existing entrypoint line numbers map.
    m_entrypointLineNumbers.clear();

    // Invoke the CLI To query entrypoint names and start line numbers.
    ret = rgCliLauncher::ListKernels(m_pProject, m_cloneIndex, m_entrypointLineNumbers);
    if (!ret)
    {
        // Let the user know that the query failed.
        emit SetStatusBarText(STR_ERR_FAILED_TO_GET_ENTRYPOINT_LINE_NUMBERS, gs_STATUS_BAR_NOTIFICATION_TIMEOUT_MS);
    }

    return ret;
}

void rgBuildView::RenameFile(const std::string& oldFilepath, const std::string& newFilepath)
{
    auto editorIter = m_sourceCodeEditors.find(oldFilepath);
    if (editorIter != m_sourceCodeEditors.end())
    {
        rgSourceCodeEditor* pEditor = editorIter->second;

        // Erase the existing file path, and insert the new one.
        m_sourceCodeEditors.erase(editorIter);
        m_sourceCodeEditors[newFilepath] = pEditor;
    }

    // Update the project's source file list with the new filepath.
    rgConfigManager& configManager = rgConfigManager::Instance();
    configManager.UpdateSourceFilepath(oldFilepath, newFilepath, m_pProject, m_cloneIndex);

    // Save the updated project file.
    configManager.SaveProjectFile(m_pProject);
}

void rgBuildView::RenameProject(const std::string& fullPath)
{
    // Cache the original file path being renamed.
    std::string originalFilePath = m_pProject->m_projectFileFullPath;

    // Rename the project config file.
    bool isRenamed = QFile::rename(m_pProject->m_projectFileFullPath.c_str(), fullPath.c_str());
    assert(isRenamed);

    if (isRenamed)
    {
        // Set full path.
        m_pProject->m_projectFileFullPath = fullPath;

        // Set project name.
        std::string filename;
        rgUtils::ExtractFileName(fullPath, filename, false);
        m_pProject->m_projectName = filename;
        rgConfigManager& configManager = rgConfigManager::Instance();

        // Update the recent project list to reference the new path.
        configManager.UpdateRecentProjectPath(originalFilePath, fullPath);

        // Save the project file.
        configManager.SaveProjectFile(m_pProject);

        // Update main window title text.
        emit ProjectLoaded(m_pProject);
    }
}

std::string rgBuildView::GetFilepathForEditor(const rgSourceCodeEditor* pEditor)
{
    // Return an empty string by default.
    std::string ret = "";

    auto editorIter = m_sourceCodeEditors.begin();
    while (editorIter != m_sourceCodeEditors.end())
    {
        // Return the filename if a match is found.
        if (editorIter->second == pEditor)
        {
            ret = editorIter->first;
            break;
        }

        editorIter++;
    }

    return ret;
}

void rgBuildView::HandleNewCLIOutputString(const std::string& cliOutputString)
{
    // Send the CLI's output text to the output window.
    m_pCliOutputWindow->EmitSetText(cliOutputString.c_str());
}

void rgBuildView::HighlightEntrypointStartLine(const std::string& inputFilePath, const std::string& selectedEntrypointName)
{
    // Find the input file in the map of entrypoint start line numbers.
    auto inputFileIter = m_entrypointLineNumbers.find(inputFilePath);
    if (inputFileIter != m_entrypointLineNumbers.end())
    {
        // Search for the start line number for the given entrypoint name.
        EntryToSourceLineRange& fileEntrypointsInfo = inputFileIter->second;
        auto lineNumberIter = fileEntrypointsInfo.find(selectedEntrypointName);
        if (lineNumberIter != fileEntrypointsInfo.end())
        {
            rgSourceCodeEditor* pEditor = GetEditorForFilepath(inputFilePath);
            assert(pEditor != nullptr);
            if (pEditor != nullptr)
            {
                // Retrieve the entrypoint's start line index according to the "list-kernels" results.
                int listKernelsEntrypointStartLine = lineNumberIter->second.first;

                // If necessary, advance to the line with the entrypoint's opening brace. This is the real start of the entrypoint.
                int actualStartLine = FindEntrypointStartLine(pEditor, listKernelsEntrypointStartLine);

                // Scroll to the start of the entrypoint.
                pEditor->ScrollToLine(actualStartLine);

                // Move the cursor to the line where the entrypoint starts.
                QTextCursor cursor(pEditor->document()->findBlockByLineNumber(actualStartLine - 1));
                pEditor->setTextCursor(cursor);

                // Highlight the start line for the entrypoint.
                QList<int> lineIndices;
                lineIndices.push_back(actualStartLine);
                pEditor->SetHighlightedLines(lineIndices);
            }
        }
    }
}

bool rgBuildView::IsLineCorrelationEnabled(rgSourceCodeEditor* pSourceEditor)
{
    bool isCorrelationEnabled = false;

    assert(m_pProject != nullptr);
    if (m_pProject != nullptr)
    {
        // Ensure that the clone index is valid.
        bool isValidCloneIndex = m_cloneIndex >= 0 && m_cloneIndex < m_pProject->m_clones.size();
        assert(isValidCloneIndex);
        if (isValidCloneIndex)
        {
            auto firstFile = m_pProject->m_clones[m_cloneIndex]->m_sourceFiles.begin();
            auto lastFile = m_pProject->m_clones[m_cloneIndex]->m_sourceFiles.end();

            // Search the list of source file info for the one that matches the given editor.
            std::string filePath = GetFilepathForEditor(pSourceEditor);
            rgSourceFilePathSearcher pathSearcher(filePath);
            auto fileIter = std::find_if(firstFile, lastFile, pathSearcher);
            assert(fileIter != lastFile);
            if (fileIter != lastFile)
            {
                // Update the correlation state for the file.
                isCorrelationEnabled = fileIter->m_isCorrelated;
            }
        }
    }

    return isCorrelationEnabled;
}

void rgBuildView::SaveEditorTextToFile(rgSourceCodeEditor* pEditor, const std::string& fullPath)
{
    bool isEditorValid = (pEditor != nullptr);
    assert(isEditorValid);

    // Only save if the editor is modified.
    if (isEditorValid && pEditor->document()->isModified())
    {
        bool isSaveSuccessful = rgUtils::WriteTextFile(fullPath, pEditor->toPlainText().toStdString());
        assert(isSaveSuccessful);

        // Remember most recent time file was modified.
        QFileInfo fileInfo(fullPath.c_str());
        m_fileModifiedTimeMap[pEditor] = fileInfo.lastModified();

        // Indicate that a freshly saved file is considered unmodified.
        pEditor->document()->setModified(false);
    }
}

void rgBuildView::ToggleFindWidgetVisibility(bool isVisible)
{
    if (m_pFindWidget != nullptr)
    {
        QVBoxLayout* pLayout = static_cast<QVBoxLayout*>(m_pSourceViewStack->layout());
        if (isVisible)
        {
            // Only make the widget visible if the source editor is currently visible.
            bool isEditorVisible = m_pCurrentCodeEditor->isVisible();
            if (isEditorVisible)
            {
                // Insert the find widget into the top of the layout above the source editor.
                m_pFindWidget->setVisible(isVisible);

                // Update the position of the find widget.
                UpdateFindWidgetGeometry();

                m_pFindWidget->SetFocused();
            }
        }
        else
        {
            pLayout->removeWidget(m_pFindWidget);
            m_pFindWidget->setVisible(isVisible);
        }
    }
}

void rgBuildView::UpdateFindWidgetGeometry()
{
    assert(m_pFindWidget != nullptr);
    if (m_pFindWidget != nullptr)
    {
        assert(m_pCurrentCodeEditor != nullptr);
        if (m_pCurrentCodeEditor != nullptr)
        {
            // Convert the topleft of the source editor to a global position.
            QPoint pos(0, 0);
            pos = m_pCurrentCodeEditor->mapTo(this, pos);

            // Compute the geometry for the widget relative to the source editor.
            int scrollbarWidth = m_pCurrentCodeEditor->verticalScrollBar()->width();
            if (!m_pCurrentCodeEditor->verticalScrollBar()->isVisible())
            {
                scrollbarWidth = 0;
            }

            int availableEditorWidth = m_pCurrentCodeEditor->width() - scrollbarWidth;
            int x = pos.x();

            // Try to display the find widget with the maximum dimensions that can fit within the source editor.
            int findWidgetWidth = m_pFindWidget->maximumWidth();
            if (findWidgetWidth > availableEditorWidth)
            {
                findWidgetWidth = availableEditorWidth;
            }
            else
            {
                x += (availableEditorWidth - findWidgetWidth);
            }

            int w = findWidgetWidth;
            int h = m_pFindWidget->maximumHeight();

            // Set the geometry for the widget manually.
            // Offset vertically by a single pixel so the widget doesn't overlap the editor's titlebar.
            m_pFindWidget->setGeometry(x, pos.y() + 1, w, h);
        }
    }
}

void rgBuildView::UpdateSourceEditorTitlebar(rgSourceCodeEditor* pCodeEditor)
{
    std::string sourceFilePath = GetFilepathForEditor(pCodeEditor);
    if (!sourceFilePath.empty())
    {
        // If the source file has already been disassembled, check if line correlation is currently enabled.
        bool isCorrelationEnabled = false;
        bool isDisassembled = IsFileDisassembled(sourceFilePath);
        if (isDisassembled)
        {
            isCorrelationEnabled = IsLineCorrelationEnabled(pCodeEditor);
        }
        else
        {
            // The file hasn't been disassembled yet, so don't display a warning in the editor's titlebar.
            isCorrelationEnabled = true;
        }

        // Update the titlebar to show the current correlation state for the file.
        assert(m_pSourceEditorTitlebar != nullptr);
        if (m_pSourceEditorTitlebar != nullptr)
        {
            m_pSourceEditorTitlebar->SetIsCorrelationEnabled(isCorrelationEnabled);
        }
    }
}

void rgBuildView::UpdateSourceFileCorrelationState(const std::string& filePath, bool isCorrelated)
{
    assert(m_pProject != nullptr);
    if (m_pProject != nullptr)
    {
        bool isValidCloneIndex = m_cloneIndex >= 0 && m_cloneIndex < m_pProject->m_clones.size();
        assert(isValidCloneIndex);
        if (isValidCloneIndex)
        {
            auto firstFile = m_pProject->m_clones[m_cloneIndex]->m_sourceFiles.begin();
            auto lastFile = m_pProject->m_clones[m_cloneIndex]->m_sourceFiles.end();

            // Search for the given source file in the project's list of source files.
            rgSourceFilePathSearcher pathSearcher(filePath);
            auto fileIter = std::find_if(firstFile, lastFile, pathSearcher);
            if (fileIter != lastFile)
            {
                // Update the correlation state for the file.
                fileIter->m_isCorrelated = isCorrelated;

                // Save the project file each time the state is changed.
                rgConfigManager::Instance().SaveProjectFile(m_pProject);
            }
        }
    }
}

void rgBuildView::CheckExternalFileModification()
{
    // If there are no active code editors, no files can be modified.
    if (m_pCurrentCodeEditor != nullptr)
    {
        // Get file modification time from the last time the file was saved in RGA.
        QDateTime expectedLastModified = m_fileModifiedTimeMap[m_pCurrentCodeEditor];

        // Get file info for the editor file.
        std::string filename = GetFilepathForEditor(m_pCurrentCodeEditor);
        QFileInfo fileInfo(filename.c_str());

        // If the modification time is not the same as remembered, the file has been changed externally.
        if (fileInfo.lastModified() != expectedLastModified)
        {
            // Notify other components in the system that this file has been modified outside the app.
            emit CurrentFileModified();

            QString messageText = QString(filename.c_str()) + "\n\n" + STR_RELOAD_FILE_DIALOG_TEXT;

            // Show message box to ask if the user want to reload the file.
            int response = QMessageBox::question(this, STR_RELOAD_FILE_DIALOG_TITLE, messageText, QMessageBox::Yes, QMessageBox::No);

            // Get user response.
            switch (response)
            {
            case QMessageBox::Yes:
                SetSourceCodeText(filename);
                break;
            case QMessageBox::No:
                m_fileModifiedTimeMap[m_pCurrentCodeEditor] = fileInfo.lastModified();

                // Indicate the document is unsaved, regardless of any previous state.
                m_pCurrentCodeEditor->document()->setModified(true);
                break;
            default:
                // Should never get here.
                assert(false);
            }
        }
    }
}

void rgBuildView::SetConfigSplitterPositions()
{
    rgConfigManager& configManager = rgConfigManager::Instance();

    // Build output splitter.
    configManager.SetSplitterValues(STR_SPLITTER_NAME_BUILD_OUTPUT, m_pOutputSplitter->sizes().toVector().toStdVector());

    // Disassembly/source view splitter.
    configManager.SetSplitterValues(STR_SPLITTER_NAME_SOURCE_DISASSEMBLY, m_pDisassemblyViewSplitter->sizes().toVector().toStdVector());
}

void rgBuildView::RestoreViewLayout()
{
    rgConfigManager& configManager = rgConfigManager::Instance();
    std::vector<int> splitterValues;

    // Build output splitter.
    bool hasSplitterValues = configManager.GetSplitterValues(STR_SPLITTER_NAME_BUILD_OUTPUT, splitterValues);
    if (hasSplitterValues)
    {
        m_pOutputSplitter->setSizes(QVector<int>::fromStdVector(splitterValues).toList());
    }

    // Disassembly/source view splitter.
    hasSplitterValues = configManager.GetSplitterValues(STR_SPLITTER_NAME_SOURCE_DISASSEMBLY, splitterValues);
    if (hasSplitterValues)
    {
        m_pDisassemblyViewSplitter->setSizes(QVector<int>::fromStdVector(splitterValues).toList());
    }
}

bool rgBuildView::IsBuildInProgress() const
{
    return m_isBuildInProgress;
}

void rgBuildView::GetUnsavedSourceFiles(QStringList& unsavedSourceFiles)
{
    // Build a list of all unsaved file with modifications.
    auto editorIter = m_sourceCodeEditors.begin();
    while (editorIter != m_sourceCodeEditors.end())
    {
        rgSourceCodeEditor* pEditor = editorIter->second;
        std::string fullPath = editorIter->first;

        bool isEditorValid = (pEditor != nullptr);
        assert(isEditorValid);

        // Add file to the list if it is unsaved (modified).
        if (isEditorValid && pEditor->document()->isModified())
        {
            unsavedSourceFiles << fullPath.c_str();
        }

        editorIter++;
    }
}

void rgBuildView::HandleSaveSettingsButtonClicked()
{
    if (m_editMode == EditMode::SourceCode)
    {
        SaveCurrentFile();
    }
    else if (m_editMode == EditMode::BuildSettings)
    {
        // Disable the "Save" button.
        assert(m_pSettingsButtonsView != nullptr);
        if (m_pSettingsButtonsView != nullptr)
        {
            m_pSettingsButtonsView->EnableSaveButton(false);
        }

        m_pBuildSettingsView->SaveSettings();
    }
}

void rgBuildView::HandleRestoreDefaultsSettingsClicked()
{
    // Ask the user for confirmation.
    bool isConfirmation = rgUtils::ShowConfirmationMessageBox(STR_BUILD_SETTINGS_DEFAULT_SETTINGS_CONFIRMATION_TITLE, STR_BUILD_SETTINGS_DEFAULT_SETTINGS_CONFIRMATION, this);

    if (isConfirmation)
    {
        // Disable the "Save" button.
        assert(m_pSettingsButtonsView != nullptr);
        if (m_pSettingsButtonsView != nullptr)
        {
            m_pSettingsButtonsView->EnableSaveButton(false);
        }

        assert(m_pBuildSettingsView != nullptr);
        if (m_pBuildSettingsView != nullptr)
        {
            m_pBuildSettingsView->RestoreDefaultSettings();
        }
    }
}

void rgBuildView::HandleSetFrameBorderRed()
{
    m_pBuildSettingsWidget->setStyleSheet("#buildSettingsWidget { border: 1px solid rgb(224, 30, 55); }");
}

void rgBuildView::HandleSetFrameBorderBlack()
{
    m_pBuildSettingsWidget->setStyleSheet("#buildSettingsWidget { border: 1px solid black; }");
}

void rgBuildView::HandleDisassemblyViewSizeMaximize()
{
    assert(m_pDisassemblyView);
    assert(m_pDisassemblyViewContainer);

    if (m_pDisassemblyView != nullptr)
    {
        m_pDisassemblyView->setMaximumWidth(QWIDGETSIZE_MAX);
    }
    if (m_pDisassemblyViewContainer != nullptr)
    {
        m_pDisassemblyViewContainer->setMaximumWidth(QWIDGETSIZE_MAX);
    }
}

void rgBuildView::HandleDisassemblyViewSizeRestore()
{
    HandleDisassemblyTableWidthResizeRequested(0);
}

void rgBuildView::HandleSplitterMoved(int pos, int index)
{
    // Update the splitter dimensions in the config file.
    SetConfigSplitterPositions();
}
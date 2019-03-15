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

// Infra.
#include <QtCommon/Scaling/ScalingManager.h>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgBuildView.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgBuildSettingsView.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgBuildSettingsViewVulkan.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgBuildSettingsWidget.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgCliOutputView.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMenu.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMenuBuildSettingsItem.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMenuFileItem.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMenuTitlebar.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgFindTextWidget.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgIsaDisassemblyViewOpenCL.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgIsaDisassemblyViewVulkan.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMainWindow.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMaximizeSplitter.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgRenameProjectDialog.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgScrollArea.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgSourceCodeEditor.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgSettingsButtonsView.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgSourceEditorTitlebar.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgUnsavedItemsDialog.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgViewContainer.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgViewManager.h>
#include <RadeonGPUAnalyzerGUI/Include/rgConfigManager.h>
#include <RadeonGPUAnalyzerGUI/Include/rgCliLauncher.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDataTypesVulkan.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDefinitions.h>
#include <RadeonGPUAnalyzerGUI/Include/rgFactory.h>
#include <RadeonGPUAnalyzerGUI/Include/rgSourceEditorSearcher.h>
#include <RadeonGPUAnalyzerGUI/Include/rgStringConstants.h>
#include <RadeonGPUAnalyzerGUI/Include/rgUtils.h>
#include <RadeonGPUAnalyzerGUI/Include/rgXMLSessionConfig.h>

static const int s_FILE_MENU_VIEW_CONTAINER_WIDTH = 200;
static const int s_FIND_TEXT_WIDGET_HORIZONTAL_MARGIN = 10;
static const int s_FIND_TEXT_WIDGET_VERTICAL_MARGIN = 10;

rgBuildView::rgBuildView(rgProjectAPI api, QWidget* pParent) :
    QWidget(pParent),
    m_cloneIndex(0),
    m_pParent(pParent)
{
    // Setup the UI.
    ui.setupUi(this);

    // Create the factory used to create API specific objects within the rgBuildView.
    m_pFactory = rgFactory::CreateFactory(api);

    // Create the find widget.
    CreateFindWidget();
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
    rgMenu* pMenu = GetMenu();
    if (pMenu != nullptr)
    {
        pMenu->ClearFiles();
    }

    // Clean up source editor instances.
    ClearEditors();

    // Clear the output window.
    if (m_pCliOutputWindow != nullptr)
    {
        m_pCliOutputWindow->ClearText();
    }
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

void rgBuildView::ConnectFileSignals()
{
    rgMenu* pMenu = GetMenu();
    assert(pMenu != nullptr);
    if (pMenu != nullptr)
    {
        // Connect the file menu to the build view, so that the view knows when the current file is switched.
        bool isConnected = connect(pMenu, &rgMenu::SelectedFileChanged, this, &rgBuildView::HandleSelectedFileChanged);
        assert(isConnected);

        // Connect the file menu's MenuItemCloseButtonClicked signal with the menu item closed handler.
        isConnected = connect(pMenu, &rgMenu::MenuItemCloseButtonClicked, this, &rgBuildView::HandleMenuItemCloseButtonClicked);
        assert(isConnected);

        // Connect the file menu default item's "Add new file" button.
        isConnected = connect(pMenu, &rgMenu::CreateFileButtonClicked, this, &rgBuildView::CreateFileButtonClicked);
        assert(isConnected);

        // Connect the file menu default item's "Open existing file" button.
        isConnected = connect(pMenu, &rgMenu::OpenFileButtonClicked, this, &rgBuildView::OpenFileButtonClicked);
        assert(isConnected);

        // Connect the file menu's rename signal.
        isConnected = connect(pMenu, &rgMenu::FileRenamed, this, &rgBuildView::HandleFileRenamed);
        assert(isConnected);

        // Connect the file menu's project renamed signal.
        isConnected = connect(m_pFileMenuTitlebar, &rgMenuTitlebar::TitleChanged, this, &rgBuildView::HandleProjectRenamed);
        assert(isConnected);

        // Connect the file menu's item count change signal.
        isConnected = connect(pMenu, &rgMenu::FileMenuItemCountChanged, this, &rgBuildView::ProjectFileCountChanged);
        assert(isConnected);

        // Connect the file menu's file build settings button click signal to the rgBuildView's handler.
        isConnected = connect(pMenu, &rgMenu::BuildSettingsButtonClicked, this, &rgBuildView::HandleFindWidgetVisibilityToggled);
        assert(isConnected);

        // Connect the file menu's next focus change signal.
        isConnected = connect(pMenu, &rgMenu::FocusNextView, this, &rgBuildView::HandleFocusNextView);
        assert(isConnected);

        // Connect the error location reported by CLI output window to the rgBuildView's handlers.
        isConnected = connect(m_pCliOutputWindow, &rgCliOutputView::SwitchToFile, pMenu, &rgMenu::HandleSwitchToFile);
        assert(isConnected);

        // Connect the focus file menu signal to the build view's handlers.
        isConnected = connect(m_pCliOutputWindow, &rgCliOutputView::FocusNextView, this, &rgBuildView::HandleFocusNextView);
        assert(isConnected);

        // Connect the focus output window signal to the build view's handlers.
        isConnected = connect(m_pCliOutputWindow, &rgCliOutputView::FocusOutputWindow, this, &rgBuildView::HandleSetOutputWindowFocus);
        assert(isConnected);

        // Connect the source editor's scroll-to-line signal.
        isConnected = connect(pMenu, &rgMenu::ScrollCodeEditorToLine, this, &rgBuildView::HandleScrollCodeEditorToLine);
        assert(isConnected);

        // Connect the "Build Settings" button in the file menu.
        rgMenuBuildSettingsItem* pBuildSettingsItem = pMenu->GetBuildSettingsItem();
        const QPushButton* pBuildSettingsFileMenuButton = pBuildSettingsItem->GetBuildSettingsButton();
        isConnected = connect(pBuildSettingsFileMenuButton, &QPushButton::clicked, this, &rgBuildView::HandleBuildSettingsMenuButtonClicked);
        assert(isConnected);

        // Connect to the file menu container's mouse click event.
        isConnected = connect(m_pFileMenuViewContainer, &rgViewContainer::ViewContainerMouseClickEventSignal, this, &rgBuildView::HandleSetFrameBorderBlack);
        assert(isConnected);

        // Connect menu signals.
        isConnected = ConnectMenuSignals();
        assert(isConnected);
    }
}

void rgBuildView::HandleFocusPrevView()
{
    assert(m_pViewManager != nullptr);
    if (m_pViewManager != nullptr)
    {
        m_pViewManager->FocusPrevView();
    }
}

void rgBuildView::HandleSetOutputWindowFocus()
{
    assert(m_pViewManager != nullptr);
    if (m_pViewManager != nullptr)
    {
        m_pViewManager->SetOutputWindowFocus();
    }
}

void rgBuildView::HandleSetDisassemblyViewFocus()
{
    assert(m_pViewManager != nullptr);
    if (m_pViewManager != nullptr)
    {
        m_pViewManager->SetDisassemblyViewFocus();
    }
}

void rgBuildView::ConnectBuildSettingsSignals()
{
    // "Save" button.
    bool isConnected = connect(m_pSettingsButtonsView, &rgSettingsButtonsView::SaveSettingsButtonClickedSignal, this, &rgBuildView::HandleSaveSettingsButtonClicked);
    assert(isConnected);

    // "Restore defaults" button.
    isConnected = connect(m_pSettingsButtonsView, &rgSettingsButtonsView::RestoreDefaultSettingsButtonClickedSignal, this, &rgBuildView::HandleRestoreDefaultsSettingsClicked);
    assert(isConnected);

    // Connect the save settings view clicked signal.
    isConnected = connect(m_pSettingsButtonsView, &rgSettingsButtonsView::SettingsButtonsViewClickedSignal, this, &rgBuildView::SetAPISpecificBorderColor);
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

    isConnected = connect(m_pViewManager, &rgViewManager::BuildSettingsWidgetFocusInSignal, this, &rgBuildView::SetAPISpecificBorderColor);
    assert(isConnected);

    isConnected = connect(m_pViewManager, &rgViewManager::BuildSettingsWidgetFocusOutSignal, this, &rgBuildView::HandleSetFrameBorderBlack);
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

bool rgBuildView::ConnectDisassemblyViewSignals()
{
    bool isConnected = false;

    assert(m_pDisassemblyView != nullptr);
    if (m_pDisassemblyView != nullptr)
    {
        // Connect the handler invoked when the highlighted correlation line in the input source file should be updated.
        isConnected = connect(m_pDisassemblyView, &rgIsaDisassemblyView::InputSourceHighlightedLineChanged,
            this, &rgBuildView::HandleHighlightedCorrelationLineUpdated);
        assert(isConnected);

        // Connect the rgIsaDisassemblyView's table resized handler.
        isConnected = connect(m_pDisassemblyView, &rgIsaDisassemblyView::DisassemblyTableWidthResizeRequested,
            this, &rgBuildView::HandleDisassemblyTableWidthResizeRequested);
        assert(isConnected);

        // Connect the rgIsaDisassemblyView's Target GPU changed handler.
        isConnected = connect(m_pDisassemblyView, &rgIsaDisassemblyView::SelectedTargetGpuChanged,
            this, &rgBuildView::HandleSelectedTargetGpuChanged);
        assert(isConnected);

        // Connect the rgIsaDisassemblyView's clicked handler.
        isConnected = connect(m_pDisassemblyView, &rgIsaDisassemblyView::DisassemblyViewClicked,
            this, &rgBuildView::HandleDisassemblyViewClicked);
        assert(isConnected);

        // Connect the rgIsaDisassemblyViewTitlebar's double click handler.
        m_pDisassemblyView->ConnectTitleBarDoubleClick(m_pDisassemblyViewContainer);

        // Connect the focus disassembly view signal to the build view's handlers.
        isConnected = connect(m_pDisassemblyView, &rgIsaDisassemblyView::FocusDisassemblyView, this, &rgBuildView::HandleSetDisassemblyViewFocus);
        assert(isConnected);

        // Connect the handler invoked when cli output window should be highlighted.
        isConnected = connect(m_pDisassemblyView, &rgIsaDisassemblyView::FocusCliOutputWindow,
            this, &rgBuildView::HandleSetOutputWindowFocus);
        assert(isConnected);

        // Connect the splitter's frame in focus signal.
        if (m_pDisassemblyViewSplitter != nullptr)
        {
            isConnected = connect(m_pDisassemblyViewSplitter, &rgMaximizeSplitter::FrameInFocusSignal,
                m_pDisassemblyView, &rgIsaDisassemblyView::HandleDisassemblyTabViewClicked);
            assert(isConnected);
        }

        // Connect the source code editor focus in signal.
        assert(m_pCurrentCodeEditor != nullptr);
        if (m_pCurrentCodeEditor != nullptr)
        {
            // Connect the source editor's focus in handler.
            isConnected = connect(m_pCurrentCodeEditor, &rgSourceCodeEditor::SourceCodeEditorFocusInEvent,
                m_pDisassemblyView, &rgIsaDisassemblyView::HandleFocusOutEvent);
            assert(isConnected);

            // Connect the source editor's scrollbar disabling signal.
            isConnected = connect(m_pCurrentCodeEditor, &rgSourceCodeEditor::DisableScrollbarSignals,
                m_pDisassemblyView, &rgIsaDisassemblyView::DisableScrollbarSignals);
            assert(isConnected);

            // Connect the source editor's scrollbar enabling signal.
            isConnected = connect(m_pCurrentCodeEditor, &rgSourceCodeEditor::EnableScrollbarSignals,
                m_pDisassemblyView, &rgIsaDisassemblyView::EnableScrollbarSignals);
            assert(isConnected);
        }

        rgMenu* pMenu = GetMenu();
        assert(pMenu != nullptr);
        if (pMenu != nullptr)
        {
            // Connect the file menu focus in signal.
            isConnected = connect(pMenu, &rgMenu::FileMenuFocusInEvent, m_pDisassemblyView, &rgIsaDisassemblyView::HandleFocusOutEvent);
            assert(isConnected);
        }

        // Connect the view manager focus in signals.
        assert(m_pViewManager != nullptr);
        if (m_pViewManager != nullptr)
        {
            isConnected = connect(m_pViewManager, &rgViewManager::FrameFocusInSignal, m_pDisassemblyView, &rgIsaDisassemblyView::HandleDisassemblyTabViewClicked);
            assert(isConnected);

            isConnected = connect(m_pViewManager, &rgViewManager::FrameFocusOutSignal, m_pDisassemblyView, &rgIsaDisassemblyView::HandleFocusOutEvent);
            assert(isConnected);
        }

        // Connect the focus column push button signal to the disassembly view's handlers.
        isConnected = connect(m_pCliOutputWindow, &rgCliOutputView::FocusColumnPushButton, m_pDisassemblyView, &rgIsaDisassemblyView::HandleFocusColumnsPushButton);
        assert(isConnected);

        // Connect the focus source window signal to the disassembly view's handlers.
        isConnected = connect(m_pDisassemblyView, &rgIsaDisassemblyView::FocusSourceWindow, this, &rgBuildView::HandleFocusPreviousView);
        assert(isConnected);

        // Connect the switch container size signal from the disassembly view..
        isConnected = connect(m_pDisassemblyView, &rgIsaDisassemblyView::SwitchDisassemblyContainerSize, this, &rgBuildView::HandleSwitchContainerSize);
        assert(isConnected);

        // Connect API-specific rgBuildView signals to the disassembly view.
        ConnectDisassemblyViewApiSpecificSignals();
    }

    assert(m_pDisassemblyViewSplitter != nullptr);
    if (m_pDisassemblyViewSplitter != nullptr)
    {
        // Connect the handler invoked when the disassembly container has been maximized.
        isConnected = connect(m_pDisassemblyViewSplitter, &rgMaximizeSplitter::ViewMaximized,
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

    return isConnected;
}

void rgBuildView::HandleSwitchContainerSize()
{
    assert(m_pViewManager != nullptr);
    if (m_pViewManager != nullptr)
    {
        m_pViewManager->SwitchContainerSize();
    }
}

void rgBuildView::HandleFocusCliOutputWindow()
{
    assert(m_pViewManager != nullptr);
    if (m_pViewManager != nullptr)
    {
        m_pViewManager->FocusNextView();
    }
}

void rgBuildView::HandleFocusPreviousView()
{
    assert(m_pViewManager != nullptr);
    if (m_pViewManager != nullptr)
    {
        m_pViewManager->FocusPrevView();
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

    // Connect the editor titlebar's "Dismiss Message" handler.
    isConnected = connect(m_pSourceEditorTitlebar, &rgSourceEditorTitlebar::DismissMsgButtonClicked,
        this, &rgBuildView::HandleCodeEditorTitlebarDismissMsgPressed);
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
        bool isPopulated = PopulateMenu();
        rgMenu* pMenu = GetMenu();
        assert(pMenu != nullptr);
        if (pMenu != nullptr)
        {
            // Does the menu contain any files after attempting to populate it?
            if (pMenu->IsEmpty())
            {
                // There are no files to display. Open the rgBuildView in an empty state, and return true.
                SwitchEditMode(EditMode::Empty);
            }
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

            if (m_pFileMenuTitlebar != nullptr)
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

void rgBuildView::BuildCurrentProject()
{
    // Destroy outputs from previous builds.
    DestroyProjectBuildArtifacts();

    // Clear the output window.
    m_pCliOutputWindow->ClearText();

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
                bool isProjectBuilt = false;
                std::vector<std::string> gpusWithBuildOutputs;
                rgProjectAPI currentApi = rgConfigManager::Instance().GetCurrentAPI();
                if (currentApi == rgProjectAPI::OpenCL)
                {
                    isProjectBuilt = rgCliLauncher::BuildProjectCloneOpenCL(m_pProject, m_cloneIndex, outputPath, appendBuildOutput, gpusWithBuildOutputs, m_cancelBuildSignal);
                }
                else if (currentApi == rgProjectAPI::Vulkan)
                {
                    isProjectBuilt = rgCliLauncher::BuildProjectCloneVulkan(m_pProject, m_cloneIndex, outputPath, appendBuildOutput, gpusWithBuildOutputs, m_cancelBuildSignal);
                }

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

bool rgBuildView::CreateFileMenu()
{
    // Create the API-specific file menu.
    bool isOk = CreateMenu(this);
    if (isOk)
    {
        // Retrieve a pointer to the file menu.
        rgMenu* pMenu = GetMenu();
        assert(pMenu != nullptr);
        if (pMenu != nullptr)
        {
            assert(m_pProject != nullptr);
            if (m_pProject != nullptr)
            {
                // Ensure that the incoming clone index is valid for the current project.
                bool isValidRange = (m_cloneIndex >= 0 && m_cloneIndex < m_pProject->m_clones.size());
                assert(isValidRange);

                if (isValidRange)
                {
                    std::shared_ptr<rgProjectClone> pProjectClone = m_pProject->m_clones[m_cloneIndex];

                    // Initialize the menu with default items.
                    pMenu->InitializeDefaultMenuItems(pProjectClone);
                }
            }
        }

        // Create the menu title bar where the program name is displayed.
        m_pFileMenuTitlebar = new rgMenuTitlebar();

        // Register the menu title bar with scaling manager.
        ScalingManager::Get().RegisterObject(m_pFileMenuTitlebar);

        // Wrap the file menu in a view container with its title bar.
        m_pFileMenuViewContainer->SetMainWidget(pMenu);
        m_pFileMenuViewContainer->SetTitlebarWidget(m_pFileMenuTitlebar);
        m_pFileMenuViewContainer->setObjectName(STR_RG_FILE_MENU_VIEW_CONTAINER);

        // Connect signals for the file menu.
        ConnectFileSignals();
    }

    return isOk;
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
        // Repeatedly ask the user for a project name when their provided name is invalid.
        bool isValidProjectPath = false;
        do
        {
            rgRenameProjectDialog* pRenameProjectDialog = m_pFactory->CreateRenameProjectDialog(projectName, m_pParent);

            // Prompt the user for the project name.
            int rc = pRenameProjectDialog->exec();
            if (rc == QDialog::Accepted)
            {
                // Generate the path to where the new project's project file would live.
                std::string projectFilePath = rgConfigManager::GenerateProjectFilepath(projectName);

                // The path for the new project is valid only if it doesn't already exist.
                isValidProjectPath = !rgUtils::IsFileExists(projectFilePath);
                if (isValidProjectPath)
                {
                    // Create a project instance with the given name.
                    m_pProject = m_pFactory->CreateProject(projectName, projectFilePath);
                    assert(m_pProject != nullptr);

                    // Create the clone.
                    CreateProjectClone();

                    // We're done.
                    ret = true;
                }
                else
                {
                    // Let the user know that the project name they have provided has already been used.
                    rgUtils::ShowErrorMessageBox(STR_NEW_PROJECT_ALREADY_EXISTS, this);
                }
            }
            else
            {
                break;
            }

            // Free memory.
            RG_SAFE_DELETE(pRenameProjectDialog);

        } while (!isValidProjectPath);
    }

    return ret;
}

void rgBuildView::InitializeView()
{
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
    m_pSourceViewContainer->setObjectName(STR_RG_SOURCE_VIEW_CONTAINER);

    // Create the disassembly view splitter.
    m_pDisassemblyViewSplitter = new rgMaximizeSplitter(this);
    m_pDisassemblyViewSplitter->setOrientation(Qt::Orientation::Horizontal);
    m_pDisassemblyViewSplitter->setChildrenCollapsible(false);

    // Add the source view container to the disassembly view splitter.
    m_pDisassemblyViewSplitter->AddMaximizableWidget(m_pSourceViewContainer);

    // Set up the splitter between the file menu and the rest of the views.
    m_pFileMenuSplitter = new QSplitter(Qt::Orientation::Horizontal, this);

    // Create the file menu's container.
    m_pFileMenuViewContainer = new rgViewContainer();

    // Set the fixed width of file menu container.
    m_pFileMenuViewContainer->setFixedWidth(s_FILE_MENU_VIEW_CONTAINER_WIDTH);

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
    m_pBuildOutputViewContainer->setObjectName(STR_RG_BUILD_OUTPUT_VIEW_CONTAINER);

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

    // Restore the previous session rgBuildView layout.
    RestoreViewLayout();

    // Setup view manager.
    m_pViewManager = new rgViewManager(this);
    m_pViewManager->AddView(m_pFileMenuViewContainer, true);
    m_pViewManager->AddView(m_pSourceViewContainer, true);
    m_pViewManager->AddView(m_pBuildOutputViewContainer, true);

    // Connect signals for the Build View.
    ConnectBuildViewSignals();

    // Declare EditMode as a meta type so it can be used with slots/signals.
    int id = qRegisterMetaType<EditMode>();
    Q_UNUSED(id);

    // Create the file menu.
    CreateFileMenu();

    // Update the menu's title bar to display the Project name.
    assert(m_pFileMenuTitlebar != nullptr);
    assert(m_pProject != nullptr);
    if (m_pFileMenuTitlebar != nullptr && m_pProject != nullptr)
    {
        // Set project name title in file menu.
        std::stringstream title;
        title << rgUtils::GetProjectTitlePrefix(m_pProject->m_api) << m_pProject->m_projectName;
        m_pFileMenuTitlebar->SetTitle(title.str().c_str());
    }

    // Create a new build settings view after a new project has been created.
    CreateBuildSettingsView();

    // Create and initialize views specific to the current mode only.
    InitializeModeSpecificViews();
}

bool rgBuildView::HasSourceCodeEditors() const
{
    return m_sourceCodeEditors.empty();
}

bool rgBuildView::HasProject() const
{
    return (m_pProject != nullptr);
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
        const std::string& currentMode = rgConfigManager::Instance().GetCurrentModeString();
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
                    // Add the family name to the list of targets to attempt to load results for.
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
                    bool isLoadedForGpu = false;

                    std::string fullMetadataFilePath;
                    isOk = rgUtils::AppendFileNameToPath(outputFolderPath, metadataFilenameStream.str(), fullMetadataFilePath);
                    assert(isOk);
                    if (isOk)
                    {
                        // Does the session metadata file exist?
                        bool isMetadataExists = rgUtils::IsFileExists(fullMetadataFilePath);
                        if (isMetadataExists)
                        {
                            // Emit a signal so the file coloring in the file menu is updated.
                            emit UpdateFileColoring();

                            std::shared_ptr<rgCliBuildOutput> pGpuOutput = nullptr;
                            isLoadedForGpu = LoadSessionMetadata(fullMetadataFilePath, pGpuOutput);

                            if (isLoadedForGpu && pGpuOutput != nullptr)
                            {
                                // Add the outputs to the map to store per-GPU results.
                                m_buildOutputs[currentGpu] = pGpuOutput;
                                isLoaded = true;
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
        // Signal that a new project has been loaded into the rgBuildView.
        emit ProjectLoaded(m_pProject);

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

void rgBuildView::ReloadFile(const std::string& filePath)
{
    SetSourceCodeText(filePath);
}

void rgBuildView::SaveCurrentFile()
{
    bool isSourceCodeEditorValid = (m_pCurrentCodeEditor != nullptr);
    assert(isSourceCodeEditorValid);
    if (isSourceCodeEditorValid)
    {
        rgMenu* pMenu = GetMenu();
        assert(pMenu != nullptr);
        if (pMenu != nullptr && pMenu->GetSelectedFileItem() != nullptr)
        {
            std::string currentFilename = pMenu->GetSelectedFilePath();

            // Ask the user for a filename if none exists so far.
            if (currentFilename.empty())
            {
                std::string filter = std::string(STR_FILE_DIALOG_FILTER_OPENCL) + ";;" + STR_FILE_DIALOG_FILTER_ALL;
                currentFilename = QFileDialog::getSaveFileName(this, STR_FILE_DIALOG_SAVE_NEW_FILE,
                    rgConfigManager::Instance().GetLastSelectedFolder().c_str(), filter.c_str()).toStdString();

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

bool rgBuildView::ShowSaveDialog(rgFilesToSave filesToSave /* = rgFilesToSave::All */, bool shouldSaveSourceFiles /* = false */)
{
    bool ret = false;
    QStringList unsavedFiles;

    // This flag would be set to true if the user chose to cancel the operation.
    // In that case, we should return false from this function and the subsequent
    // logic would make sure to cancel the entire operation (as part of which showing
    // the save dialog was required).
    bool isCanceled = false;

    if (shouldSaveSourceFiles && (filesToSave == rgFilesToSave::SourceFiles || filesToSave == rgFilesToSave::All))
    {
        // Add unsaved source files to the list of files that must be saved.
        GetUnsavedSourceFiles(unsavedFiles);
    }

    // Does the user have pending Build Settings changes to save?
    bool pendingBuildSettingsChanges = false;

    // If the build settings have been modified but the changes are still pending, add the build settings file to the list.
    if (m_pBuildSettingsView != nullptr)
    {
        if (filesToSave == rgFilesToSave::BuildSettings || filesToSave == rgFilesToSave::All)
        {
            pendingBuildSettingsChanges = m_pBuildSettingsView->GetHasPendingChanges();
            if (pendingBuildSettingsChanges)
            {
                // Add a build settings item to the unsaved files list.
                unsavedFiles << STR_MENU_BUILD_SETTINGS;
            }
        }

        // File changes are ignored unless it is:
        // Project close.
        // App exit.
        if (pendingBuildSettingsChanges || shouldSaveSourceFiles)
        {
            // Ask the user if they want to save files with modifications.
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
                        m_pBuildSettingsView->RevertPendingChanges();
                    }
                    else
                    {
                        // Discard the changes from all editors.
                        for (const QString& unsavedFilePath : unsavedFiles)
                        {
                            rgSourceCodeEditor* pEditor = GetEditorForFilepath(unsavedFilePath.toStdString());
                            DiscardEditorChanges(pEditor);
                        }
                    }
                }
                break;
                case rgUnsavedItemsDialog::Cancel:
                    isCanceled = true;
                    break;
                default:
                    break;
                }
            }

            // If "Yes", proceed with the build. If "No", proceed with the build since the pending settings have been reverted.
            // If "Cancel," stop the attempt to build and continue where the user left off.
            ret = (saveResult == rgUnsavedItemsDialog::Yes || saveResult == rgUnsavedItemsDialog::No);
        }

        ret = (ret || (!pendingBuildSettingsChanges)) && !isCanceled;
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
            if (!SaveFiles(unsavedFiles))
            {
                // The files weren't saved, so the action in-flight should be cancelled.
                result = rgUnsavedItemsDialog::Cancel;
            }
            break;
            // If the user chooses No or Cancel, nothing needs to happen except for making the dialog disappear.
        case rgUnsavedItemsDialog::No:
        case rgUnsavedItemsDialog::Cancel:
            break;
        default:
            // Shouldn't get here.
            assert(false);
        }

        // Free memory.
        RG_SAFE_DELETE(pUnsavedFileDialog);
    }
    else
    {
        // No files need to be saved because none have been modified.
        result = rgUnsavedItemsDialog::Yes;
    }

    return result;
}

bool rgBuildView::SaveFiles(const QStringList& unsavedFiles)
{
    bool isSaved = true;

    // Step through each of the files with pending changes.
    auto stringListIter = unsavedFiles.begin();
    while (stringListIter != unsavedFiles.end())
    {
        // If the file isn't the Build Settings item, it's a path to an input source file.
        const std::string& filePath = stringListIter->toStdString();
        if (filePath.compare(STR_MENU_BUILD_SETTINGS) == 0)
        {
            // Submit all pending changes and save the build settings file.
            if (m_pBuildSettingsView != nullptr)
            {
                isSaved = m_pBuildSettingsView->SaveSettings();
            }
        }
        else
        {
            SaveSourceFile(stringListIter->toStdString().c_str());
        }

        stringListIter++;

        if (!isSaved)
        {
            break;
        }
    }

    return isSaved;
}

void rgBuildView::SaveSourceFile(const std::string& sourceFilePath)
{
    rgSourceCodeEditor* pEditor = GetEditorForFilepath(sourceFilePath);

    bool isEditorValid = (pEditor != nullptr);
    assert(isEditorValid);

    if (isEditorValid)
    {
        SaveEditorTextToFile(pEditor, sourceFilePath);
    }
}

bool rgBuildView::SaveCurrentState()
{
    // Save all source files when a new build is started.
    QStringList unsavedSources;
    GetUnsavedSourceFiles(unsavedSources);
    if (!unsavedSources.empty())
    {
        for (const QString& sourceFilePath : unsavedSources)
        {
            SaveSourceFile(sourceFilePath.toStdString().c_str());
        }
    }

    // Save the current project first.
    SaveCurrentFile();

    // The build settings must be saved in order to proceed with a build.
    bool shouldProceedWithBuild = ShowSaveDialog(rgFilesToSave::BuildSettings);

    return shouldProceedWithBuild;
}

bool rgBuildView::RequestRemoveAllFiles()
{
    bool isSaveAccepted = ShowSaveDialog(rgBuildView::rgFilesToSave::All, true);
    if (isSaveAccepted)
    {
        rgMenu* pMenu = GetMenu();
        assert(pMenu != nullptr);
        if (pMenu != nullptr)
        {
            // Remove all file menu items.
            auto editorIter = m_sourceCodeEditors.begin();
            while (editorIter != m_sourceCodeEditors.end())
            {
                std::string fullPath = editorIter->first;
                pMenu->RemoveItem(fullPath);
                RemoveEditor(fullPath);

                // Keep getting first item until all are removed.
                editorIter = m_sourceCodeEditors.begin();
            }
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
        m_pDisassemblyViewContainer->SetHiddenState(!isVisible);
    }

    assert(m_pSourceViewContainer != nullptr);
    if (m_pSourceViewContainer != nullptr)
    {
        // Only allow the source editor container to be maximized/restored when the disassembly view is available.
        m_pSourceViewContainer->SetIsMaximizable(isVisible);
    }
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

void rgBuildView::HandleSourceEditorOpenHeaderRequest(const QString& path)
{
    // Check if the file exists.
    assert(m_pProject != nullptr);
    assert(m_cloneIndex < m_pProject->m_clones.size());
    assert(m_pProject->m_clones[m_cloneIndex] != nullptr);
    assert(m_pProject->m_clones[m_cloneIndex]->m_pBuildSettings != nullptr);
    if (m_pProject != nullptr && m_cloneIndex < m_pProject->m_clones.size() &&
        m_pProject->m_clones[m_cloneIndex] != nullptr)
    {
        const std::vector<std::string>& includePaths =
            m_pProject->m_clones[m_cloneIndex]->m_pBuildSettings->m_additionalIncludeDirectories;

        // The path to the file that we would like to open.
        std::string pathToOpen;

        // Try to find if any file with that full path exists on the system.
        bool isExist = rgUtils::IsFileExists(path.toStdString());
        if (!isExist)
        {
            // Try the local directory.
            assert(m_pCurrentCodeEditor != nullptr);
            if (m_pCurrentCodeEditor != nullptr)
            {
                // Get the directory of the currently edited file.
                std::string fileDirectory;
                std::string filePath = GetFilepathForEditor(m_pCurrentCodeEditor);
                bool isDirExtracted = rgUtils::ExtractFileDirectory(filePath, fileDirectory);
                if (isDirExtracted)
                {
                    // Search for the user's file in the directory
                    // where the currently edited file is located.
                    std::stringstream fullPath;
                    fullPath << fileDirectory << "/" << path.toStdString();
                    if (rgUtils::IsFileExists(fullPath.str()))
                    {
                        // We found it.
                        pathToOpen = fullPath.str();
                        isExist = true;
                    }
                }
            }

            if (!isExist)
            {
                // Try to create the path for each of the Additional Include paths.
                for (const std::string includePath : includePaths)
                {
                    std::stringstream fullPath;
                    fullPath << includePath;
                    fullPath << "/" << path.toStdString();
                    if (rgUtils::IsFileExists(fullPath.str()))
                    {
                        pathToOpen = fullPath.str();
                        isExist = true;
                        break;
                    }
                }
            }
        }
        else
        {
            pathToOpen = path.toStdString();
        }

        if (isExist)
        {
            // Open the include file.
            bool isLaunched = OpenIncludeFile(pathToOpen);
            if (!isLaunched)
            {
                // Notify the user that the viewer app could not be launched.
                std::stringstream errMsg;
                errMsg << STR_ERR_COULD_NOT_OPEN_HEADER_FILE_VIEWER <<
                    rgConfigManager::Instance().GetIncludeFileViewer();
                emit SetStatusBarText(errMsg.str());
            }
        }
        else
        {
            // Notify the user that the header could not be located.
            emit SetStatusBarText(STR_ERR_COULD_NOT_LOCATE_HEADER_FILE);
        }
    }
}

void rgBuildView::HandleCodeEditorTitlebarDismissMsgPressed()
{
    assert(m_pCurrentCodeEditor != nullptr);
    if (m_pCurrentCodeEditor != nullptr)
    {
        m_pCurrentCodeEditor->SetTitleBarText("");
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

void rgBuildView::HandleBuildSettingsPendingChangesStateChanged(bool hasPendingChanges)
{
    // Pull the build settings menu item out of the file menu.
    rgMenu* pMenu = GetMenu();
    assert(pMenu != nullptr);
    if (pMenu != nullptr)
    {
        rgMenuBuildSettingsItem* pBuildSettingsMenuItem = pMenu->GetBuildSettingsItem();
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

        InputFileToBuildOutputsMap outputsMap;
        bool gotOutputs = GetInputFileOutputs(targetGpuBuildOutputs->second, outputsMap);

        assert(gotOutputs);
        if (gotOutputs)
        {
            assert(m_pCurrentCodeEditor != nullptr);
            if (m_pCurrentCodeEditor != nullptr)
            {
                std::string currentFilePath = GetFilepathForEditor(m_pCurrentCodeEditor);

                // Does the currently-selected source file have build output for the new Target GPU?
                auto sourceFileOutputsIter = outputsMap.find(currentFilePath);

                // Trigger an update to handle highlighting the correlated disassembly
                // lines associated with the selected line in the current file.
                bool isFileBuiltForTarget = sourceFileOutputsIter != outputsMap.end();
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
    assert(m_pViewManager != nullptr);
    if (m_pViewManager != nullptr)
    {
        // Manually advance to the next view within the rgViewManager.
        m_pViewManager->FocusNextView();
    }
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

    rgMenu* pMenu = GetMenu();
    assert(pMenu != nullptr);
    if (pMenu != nullptr)
    {
        // Set menu item saved state.
        pMenu->SetItemIsSaved(fullFilePath, !isModified);
    }

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

void rgBuildView::HandleMenuItemCloseButtonClicked(const std::string& fullPath)
{
    std::stringstream msg;
    msg << fullPath << STR_MENU_BAR_CONFIRM_REMOVE_FILE_DIALOG_WARNING;

    if (ShowRemoveFileConfirmation(msg.str(), fullPath))
    {
        // Remove the input file from the rgBuildView.
        RemoveInputFile(fullPath);
    }
}

void rgBuildView::SetViewContentsWidget(QWidget* pNewContents)
{
    assert(m_pSourceViewStack != nullptr);
    if (m_pSourceViewStack != nullptr)
    {
        // Hide all existing views before replacing with the new one.
        QLayout* pLayout = m_pSourceViewStack->layout();

        assert(pLayout != nullptr);
        if (pLayout != nullptr)
        {
            for (int childIndex = 0; childIndex < pLayout->count(); ++childIndex)
            {
                QLayoutItem* pItemLayout = pLayout->itemAt(childIndex);
                assert(pItemLayout != nullptr);
                if (pItemLayout != nullptr)
                {
                    QWidget* pItem = pItemLayout->widget();
                    assert(pItem != nullptr);
                    if (pItem != nullptr)
                    {
                        pItem->hide();
                    }
                }
            }
        }

        // Verify that the new contents are valid.
        assert(pNewContents != nullptr);
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
}

bool rgBuildView::ShowRemoveFileConfirmation(const std::string& messageString, const std::string& fullPath)
{
    bool isRemoved = false;

    if (!fullPath.empty())
    {
        // Ask the user if they're sure they want to remove the file.
        isRemoved = rgUtils::ShowConfirmationMessageBox(STR_MENU_BAR_CONFIRM_REMOVE_FILE_DIALOG_TITLE, messageString.c_str(), this);

        // Ask the user if we should save the changes. Continue only if the user did not ask to cancel the operation.
        isRemoved = isRemoved && (RequestSaveFile(fullPath) != rgUnsavedItemsDialog::Cancel);
    }

    return isRemoved;
}

void rgBuildView::HandleFindTriggered()
{
    // Toggle to show the find widget.
    ToggleFindWidgetVisibility(true);
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

    rgMenu* pMenu = GetMenu();
    assert(pMenu != nullptr);
    if (pMenu != nullptr)
    {
        CurrentBuildSucceeded();
    }

    // The current project was built successfully. Open the disassembly view with the results.
    bool isDisassemblyLoaded = LoadDisassemblyFromBuildOutput();
    assert(isDisassemblyLoaded);
    if (isDisassemblyLoaded)
    {
        // Switch to the Source Code view and show the disassembly view.
        if (pMenu != nullptr)
        {
            pMenu->DeselectItems();
            pMenu->SwitchToLastSelectedItem();
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

    // Resize the disassembly view.
    HandleDisassemblyTableWidthResizeRequested(0);

    // Update the notification message if needed.
    UpdateApplicationNotificationMessage();
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

        // Save the new sizes.
        SetConfigSplitterPositions();
    }
}

void rgBuildView::HandleBuildSettingsMenuButtonClicked()
{
    OpenBuildSettings();
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

void rgBuildView::RemoveEditor(const std::string& filename, bool switchToNextFile)
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

    if (switchToNextFile)
    {
        SwitchToFirstRemainingFile();
    }
}

void rgBuildView::RemoveInputFile(const std::string& inputFileFullPath)
{
    rgConfigManager& configManager = rgConfigManager::Instance();

    // Remove the file from the project.
    configManager.RemoveSourceFilePath(m_pProject, m_cloneIndex, inputFileFullPath);
    configManager.SaveProjectFile(m_pProject);

    rgMenu* pMenu = GetMenu();
    assert(pMenu != nullptr);
    if (pMenu != nullptr)
    {
        // Remove the file from the file menu.
        pMenu->RemoveItem(inputFileFullPath);
    }

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

    if (m_pDisassemblyViewContainer == nullptr || m_pDisassemblyView == nullptr)
    {
        // Create the ISA disassembly view.
        CreateIsaDisassemblyView();

        // Connect the disassembly view signals.
        result = ConnectDisassemblyViewSignals();
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
                    // Populate the disassembly view with the build output.
                    result = m_pDisassemblyView->PopulateBuildOutput(pClone, m_buildOutputs);
                }
            }
        }
    }

    return result;
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
            SetViewContentsWidget(pEditor);

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

            // Update the editor context.
            UpdateSourceEditorSearchContext();

            // Update the title bar text.
            const std::string& titleBarText = pEditor->GetTitleBarText();
            if (!titleBarText.empty())
            {
                m_pSourceEditorTitlebar->ShowMessage(titleBarText);
            }
            else
            {
                m_pSourceEditorTitlebar->SetTitlebarContentsVisibility(false);
            }

            // The editor isn't empty, and will switch to displaying source code.
            SwitchEditMode(EditMode::SourceCode);

            // Check if the editor file has been modified externally.
            CheckExternalFileModification();

            ret = true;
        }
    }

    return ret;
}

void rgBuildView::UpdateFindWidgetViewAttachment(QWidget* pView, bool isVisible)
{
    assert(pView != nullptr);
    if (pView != nullptr)
    {
        if (m_pFindWidget != nullptr)
        {
            QLayout* pLayout = static_cast<QVBoxLayout*>(pView->layout());
            assert(pLayout != nullptr);
            if (pLayout != nullptr)
            {
                if (isVisible)
                {
                    // Only make the widget visible if the source editor is currently visible.
                    bool isEditorVisible = pView->isVisible();
                    if (isEditorVisible)
                    {
                        m_pFindWidget->setParent(pView);
                        // Insert the find widget into the top of the layout above the source editor.
                        m_pFindWidget->setVisible(isVisible);
                        m_pFindWidget->raise();

                        // Update the position of the find widget.
                        UpdateFindWidgetGeometry();
                    }
                }
                else
                {
                    pLayout->removeWidget(m_pFindWidget);
                    m_pFindWidget->setVisible(isVisible);
                }
            }
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
        bool isDisassembled = IsGcnDisassemblyGenerated(sourceFilePath);
        if (isDisassembled)
        {
            isCorrelationEnabled = IsLineCorrelationEnabled(pCodeEditor);
        }
        else
        {
            // The file hasn't been disassembled yet, so don't display a warning in the editor's titlebar.
            isCorrelationEnabled = true;
        }

        // Update the title bar to show the current correlation state for the file.
        assert(m_pSourceEditorTitlebar != nullptr);
        if (m_pSourceEditorTitlebar != nullptr && IsLineCorrelationSupported())
        {
            m_pSourceEditorTitlebar->SetIsCorrelationEnabled(isCorrelationEnabled);
        }
    }
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
        if (mode == EditMode::Empty)
        {
            SetViewContentsWidget(m_pEmptyPanel);
        }
        else
        {
            // Based on the incoming mode, hide/show specific widgets living within the BuildView.
            switch (mode)
            {
            case EditMode::SourceCode:
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
                    // Set the code editor instance in the view.
                    SetViewContentsWidget(m_pCurrentCodeEditor);
                }

                // Set the appropriate boolean in rgViewManager
                // to facilitate focusing the correct widget.
                m_pViewManager->SetIsSourceViewCurrent(true);
            }
            break;
            case EditMode::BuildSettings:
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
                SetViewContentsWidget(m_pBuildSettingsWidget);

                // Set the build settings frame border color.
                SetAPISpecificBorderColor();

                // If the disassembly view exists, hide it.
                if (m_pDisassemblyViewContainer != nullptr)
                {
                    ToggleDisassemblyViewVisibility(false);
                }

                // Initialize focus.
                if (m_pBuildSettingsView != nullptr)
                {
                    m_pBuildSettingsView->SetInitialWidgetFocus();
                }

                // Set the appropriate booleans in rgViewManager
                // to facilitate focusing the correct widget.
                m_pViewManager->SetIsSourceViewCurrent(false);
                m_pViewManager->SetIsPSOEditorViewCurrent(false);
                m_pViewManager->SetIsBuildSettingsViewCurrent(true);
            }
            break;
            default:
                // Invoke the mode-specific edit mode switch handler.
                HandleModeSpecificEditMode(mode);
                m_pViewManager->SetIsSourceViewCurrent(false);
                m_pViewManager->SetIsPSOEditorViewCurrent(true);
                m_pViewManager->SetIsBuildSettingsViewCurrent(false);
                break;
            }

            // Update the file menu save text and action.
            emit EditModeChanged(mode);
        }

        // Update the current mode.
        m_editMode = mode;
    }
}

void rgBuildView::SwitchToFirstRemainingFile()
{
    // If there are code editors remaining, switch to the first remaining item.
    if (!m_sourceCodeEditors.empty())
    {
        rgMenu* pMenu = GetMenu();
        assert(pMenu != nullptr);
        if (pMenu != nullptr)
        {
            pMenu->SelectLastRemainingItem();
        }

        // Switch to viewing the rgSourceCodeEditor for the newly selected item.
        SwitchEditMode(EditMode::SourceCode);
    }
    else
    {
        // When the last file has been removed, the editor is in the empty state.
        SwitchEditMode(EditMode::Empty);
    }
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

                        // Hide the Restore Default Settings button from the project build settings.
                        m_pSettingsButtonsView->HideRestoreDefaultSettingsButton(true);

                        // Create a scroll area to contain the build settings view.
                        rgScrollArea* pScrollArea = new rgScrollArea(this);
                        pScrollArea->setObjectName(STR_BUILD_VIEW_SETTINGS_SCROLLAREA);
                        pScrollArea->setStyleSheet(STR_BUILD_VIEW_SETTINGS_SCROLLAREA_STYLESHEET);
                        pScrollArea->setFrameShape(QFrame::NoFrame);
                        pScrollArea->setFocusPolicy(Qt::FocusPolicy::NoFocus);
                        pScrollArea->setWidget(m_pBuildSettingsView);
                        pScrollArea->setWidgetResizable(true);

                        // Connect the scroll area click to set the border to the API-specific color.
                        bool isConnected = connect(pScrollArea, &rgScrollArea::ScrollAreaClickedEvent, this, &rgBuildView::SetAPISpecificBorderColor);
                        assert(isConnected);

                        // Add various widgets to this tab.
                        m_pBuildSettingsWidget->layout()->addWidget(pScrollArea);
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
        // Create the find widget, register with the scaling manager.
        m_pFindWidget = new rgFindTextWidget(this);
        ScalingManager::Get().RegisterObject(m_pFindWidget);

        // The find widget is hidden by default.
        m_pFindWidget->hide();

        // Connect the find widget signals.
        ConnectFindSignals();

        // Create the source code searcher interface.
        m_pSourceSearcher = new rgSourceEditorSearcher();
    }
}

void rgBuildView::CreateIsaDisassemblyView()
{
    // Create a factory matching the API mode.
    rgProjectAPI currentApi = rgConfigManager::Instance().GetCurrentAPI();
    std::shared_ptr<rgFactory> pFactory = rgFactory::CreateFactory(currentApi);
    assert(pFactory != nullptr);
    if (pFactory != nullptr)
    {
        rgIsaDisassemblyView* pDisassemblyView = pFactory->CreateDisassemblyView(this);
        assert(pDisassemblyView != nullptr);
        if (pDisassemblyView != nullptr)
        {
            m_pDisassemblyView = pDisassemblyView;

            // Wrap the disassembly view in a view container.
            m_pDisassemblyViewContainer = new rgViewContainer();
            m_pDisassemblyViewContainer->SetMainWidget(m_pDisassemblyView);

            // Set the object name for the disassembly view container.
            m_pDisassemblyViewContainer->setObjectName(STR_RG_ISA_DISASSEMBLY_VIEW_CONTAINER);

            // Add the disassembly view to the disassembly splitter.
            m_pDisassemblyViewSplitter->AddMaximizableWidget(m_pDisassemblyViewContainer);

            // Hide the disassembly view when it is first created.
            m_pDisassemblyView->setVisible(true);

            // Add the view to the view manager in the disassembly view position so the tabbing order is correct.
            m_pViewManager->AddView(m_pDisassemblyViewContainer, true, static_cast<int>(rgViewManagerViewContainerIndex::DisassemblyView));
        }
    }
}

bool rgBuildView::GetInputFileOutputs(std::shared_ptr<rgCliBuildOutput> pBuildOutputs, InputFileToBuildOutputsMap& outputs) const
{
    bool ret = false;

    assert(pBuildOutputs != nullptr);
    if (pBuildOutputs != nullptr)
    {
        outputs = pBuildOutputs->m_perFileOutput;
        ret = true;
    }

    return ret;
}

rgSourceCodeEditor* rgBuildView::GetEditorForFilepath(const std::string& fullFilepath, rgSrcLanguage lang)
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
            pEditor = new rgSourceCodeEditor(this, lang);
            m_sourceCodeEditors[fullFilepath] = pEditor;

            // Connect to the specific editor's signals.
            bool isConnected = connect(pEditor, &rgSourceCodeEditor::OpenHeaderFileRequested, this, &rgBuildView::HandleSourceEditorOpenHeaderRequest);
            assert(isConnected);

            QFileInfo fileInfo(fullFilepath.c_str());
            m_fileModifiedTimeMap[pEditor] = fileInfo.lastModified();

            ConnectSourcecodeEditorSignals(pEditor);

            // Set the fonts for the source code editor.
            rgConfigManager& configManager = rgConfigManager::Instance();
            std::shared_ptr<rgGlobalSettings> pGlobalSettings = configManager.GetGlobalConfig();
            assert(pGlobalSettings != nullptr);
            if (pGlobalSettings != nullptr)
            {
                QFont font;
                font.setFamily(QString::fromStdString(pGlobalSettings->m_fontFamily));
                font.setPointSize(pGlobalSettings->m_fontSize);
                assert(pEditor != nullptr);
                if (pEditor != nullptr)
                {
                    pEditor->setFont(font);
                }
            }
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

    CurrentBuildCancelled();
}

void rgBuildView::FocusOnSourceCodeEditor()
{
    // Switch the focus to the file editor to begin editing the file.
    if (m_pCurrentCodeEditor != nullptr)
    {
        m_pCurrentCodeEditor->setFocus();
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
    rgConfigManager::UpdateSourceFilepath(oldFilepath, newFilepath, m_pProject, m_cloneIndex);

    // Save the updated project file.
    rgConfigManager::Instance().SaveProjectFile(m_pProject);
}

void rgBuildView::RenameProject(const std::string& fullPath)
{
    // Cache the original file path being renamed.
    assert(m_pProject != nullptr);
    if (m_pProject != nullptr)
    {
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
            configManager.UpdateRecentProjectPath(originalFilePath, fullPath, m_pProject->m_api);

            // Save the project file.
            configManager.SaveProjectFile(m_pProject);

            // Update main window title text.
            emit ProjectLoaded(m_pProject);
        }
    }
}

void rgBuildView::ToggleFindWidgetVisibility(bool isVisible)
{
    if (m_editMode == EditMode::SourceCode)
    {
        // Attach the Find widget to the source editor view.
        UpdateFindWidgetViewAttachment(m_pSourceViewStack, isVisible);

        assert(m_pFindWidget != nullptr);
        if (m_pFindWidget != nullptr)
        {
            if (isVisible)
            {
                m_pFindWidget->SetFocused();
            }
        }
    }
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

void rgBuildView::DiscardEditorChanges(rgSourceCodeEditor* pEditor)
{
    bool isEditorValid = (pEditor != nullptr);
    assert(isEditorValid);
    if (isEditorValid)
    {
        pEditor->document()->setModified(false);
    }
}

void rgBuildView::UpdateFindWidgetGeometry()
{
    assert(m_pFindWidget != nullptr);
    if (m_pFindWidget != nullptr)
    {
        if (m_pCurrentCodeEditor != nullptr)
        {
            // Compute the geometry for the widget relative to the source editor.
            int scrollbarWidth = m_pCurrentCodeEditor->verticalScrollBar()->width();
            if (!m_pCurrentCodeEditor->verticalScrollBar()->isVisible())
            {
                scrollbarWidth = 0;
            }

            // Start the Find widget at the far left of the attached control, and shift it to the
            // right as far as possible within the parent.
            int widgetHorizontalLocation = 0;

            // The total amount of horizontal space available for the Find widget to fit into.
            int availableEditorWidth = m_pCurrentCodeEditor->width() - scrollbarWidth;

            // Try to display the find widget with the maximum dimensions that can fit within the source editor.
            int findWidgetWidth = m_pFindWidget->maximumWidth();
            if (findWidgetWidth > availableEditorWidth)
            {
                findWidgetWidth = availableEditorWidth;
            }
            else
            {
                widgetHorizontalLocation += (availableEditorWidth - findWidgetWidth - s_FIND_TEXT_WIDGET_HORIZONTAL_MARGIN);
                assert(widgetHorizontalLocation >= 0);
                if (widgetHorizontalLocation < 0)
                {
                    widgetHorizontalLocation = 0;
                }
            }

            // Use the unmodified height of the Find Widget in the final dimension.
            int findWidgetHeight = m_pFindWidget->maximumHeight();

            // Set the geometry for the widget manually.
            // Offset vertically so the widget doesn't overlap the editor's titlebar.
            m_pFindWidget->setGeometry(widgetHorizontalLocation, s_FIND_TEXT_WIDGET_VERTICAL_MARGIN, findWidgetWidth, findWidgetHeight);
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

void rgBuildView::UpdateSourceEditorSearchContext()
{
    // Update the find widget's searcher to search the new editor.
    assert(m_pSourceSearcher != nullptr);
    assert(m_pFindWidget != nullptr);
    if (m_pSourceSearcher != nullptr && m_pFindWidget != nullptr)
    {
        // Update the FindWidget's search context to use the source editor searcher.
        m_pFindWidget->SetSearchContext(m_pSourceSearcher);
        m_pSourceSearcher->SetTargetEditor(m_pCurrentCodeEditor);
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
                ReloadFile(filename);
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

bool rgBuildView::OpenIncludeFile(const std::string& fullFilePath)
{
    bool ret = false;
    if (rgUtils::IsFileExists(fullFilePath))
    {
        // Check if RGA is configured to use the system's default app, or the user's app of choice.
        std::string includeViewer = rgConfigManager::Instance().GetIncludeFileViewer();
        if (includeViewer.compare(STR_GLOBAL_SETTINGS_SRC_VIEW_INCLUDE_VIEWER_DEFAULT) == 0)
        {
            // Launch the system's default viewer.
            QUrl fileUrl = QUrl::fromLocalFile(fullFilePath.c_str());
            QDesktopServices::openUrl(fileUrl);
            ret = true;
        }
        else
        {
            // Launch the user's editor of choice.
            QStringList argList;
            argList.push_back(fullFilePath.c_str());
            ret = QProcess::startDetached(includeViewer.c_str(), argList);
        }
    }

    return ret;
}

void rgBuildView::HandleSaveSettingsButtonClicked()
{
    SetAPISpecificBorderColor();

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
    SetAPISpecificBorderColor();

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

void rgBuildView::HandleSetFrameBorderGreen()
{
    m_pBuildSettingsWidget->setStyleSheet(STR_BUILD_VIEW_BUILD_SETTINGS_WIDGET_STYLESHEET_GREEN);
}

void rgBuildView::HandleSetFrameBorderRed()
{
    m_pBuildSettingsWidget->setStyleSheet(STR_BUILD_VIEW_BUILD_SETTINGS_WIDGET_STYLESHEET_RED);
}

void rgBuildView::HandleSetFrameBorderBlack()
{
    m_pBuildSettingsWidget->setStyleSheet(STR_BUILD_VIEW_BUILD_SETTINGS_WIDGET_STYLESHEET_BLACK);
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

std::string rgBuildView::GetCurrentProjectAPIName() const
{
    std::string projectAPI = "";

    bool ok = rgUtils::ProjectAPIToString(m_pProject->m_api, projectAPI);
    assert(ok);

    return projectAPI;
}

void rgBuildView::HandleDisassemblyViewClicked()
{
    qApp->focusObjectChanged(m_pDisassemblyView);
    m_pDisassemblyView->setFocus();
}
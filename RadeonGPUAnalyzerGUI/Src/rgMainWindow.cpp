// C++.
#include <cassert>
#include <memory>
#include <string>
#include <sstream>

// Qt.
#include <QButtonGroup>
#include <QDesktopServices>
#include <QFile>
#include <QListWidgetItem>
#include <QMimeData>
#include <QPushButton>
#include <QTextStream>
#include <QToolTip>
#include <QWidget>
#include <QtWidgets/QApplication>

// Infra.
#include <QtCommon/Scaling/ScalingManager.h>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgAboutDialog.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgAppState.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgBuildView.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgGlobalSettingsView.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMenu.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMenuBuildSettingsItem.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgGoToLineDialog.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgIsaDisassemblyView.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMainWindow.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgRecentProjectWidget.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgSettingsButtonsView.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgSettingsTab.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgSourceCodeEditor.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgStartTab.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgStatusBar.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgViewContainer.h>
#include <RadeonGPUAnalyzerGUI/Include/rgConfigFile.h>
#include <RadeonGPUAnalyzerGUI/Include/rgConfigManager.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDataTypes.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDataTypesOpenCL.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDataTypesVulkan.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDefinitions.h>
#include <RadeonGPUAnalyzerGUI/Include/rgFactory.h>
#include <RadeonGPUAnalyzerGUI/Include/rgStringConstants.h>
#include <RadeonGPUAnalyzerGUI/Include/rgUtils.h>

static const int s_CUSTOM_STATUS_BAR_HEIGHT = 25;

rgMainWindow::rgMainWindow(QWidget* pParent)
    : QMainWindow(pParent)
{
    // Get the startup mode.
    rgConfigManager& configManager = rgConfigManager::Instance();
    rgProjectAPI currentAPI = configManager.GetCurrentAPI();

    // We must have a known mode at startup.
    assert(currentAPI != rgProjectAPI::Unknown);

    // Create the factory through which we create API-specific components.
    m_pFactory = rgFactory::CreateFactory(currentAPI);

    // Setup the UI.
    ui.setupUi(this);

    // Set the window icon to the product logo.
    setWindowIcon(QIcon(gs_ICON_RESOURCE_RGA_LOGO));

    // Set the background color to white.
    QPalette pal = palette();
    pal.setColor(QPalette::Background, Qt::white);
    this->setAutoFillBackground(true);
    this->setPalette(pal);

    // Install the event filter.
    installEventFilter(this);

    // Set the status bar message's font color to white.
    QPalette palette;
    palette.setColor(QPalette::WindowText, Qt::GlobalColor::white);
    statusBar()->setPalette(palette);

    // Declare DisassemblyViewSubWidgets as a meta type so it can be used with slots/signals.
    int id = qRegisterMetaType<DisassemblyViewSubWidgets>();
    Q_UNUSED(id);
}

void rgMainWindow::InitMainWindow()
{
    // Use the mode to setup API-specific actions.
    rgProjectAPI currentApi = rgConfigManager::Instance().GetCurrentAPI();
    ChangeApiMode(currentApi);

    // Enable drops so the main window can handle opening dropped source files.
    setAcceptDrops(true);

    // Set the mouse cursor to the pointing hand cursor for various widgets.
    SetCursor();

    // Set the parent widget for the tab bar so the dialogs get shown in the correct location.
    ui.mainTabWidget->GetTabBar()->SetParentWidget(this);

    // Set the focus to the tab bar.
    ui.mainTabWidget->setFocus();

    // Increase the double click interval as the default one is too quick.
    const int doubleClickInterval = qApp->doubleClickInterval();
    qApp->setDoubleClickInterval(doubleClickInterval * 2);
}

void rgMainWindow::ConnectSignals()
{
    // Tab changed signal to handle save shortcut updates.
    bool isConnected = connect(ui.mainTabWidget, &rgMainWindowTabWidget::currentChanged, this, &rgMainWindow::HandleMainTabWidgetTabChanged);
    assert(isConnected);

    // Connect the status bar's message change signal.
    isConnected = connect(this->statusBar(), &QStatusBar::messageChanged, this, &rgMainWindow::HandleStatusBarMessageChange);
    assert(isConnected);

    // Connect the default API's settings view's handler for pending changes.
    isConnected = connect(m_pSettingsTab, &rgSettingsTab::PendingChangesStateChanged, this, &rgMainWindow::HandleHasSettingsPendingStateChanged);
    assert(isConnected);

    // Connect the notification message blinking timer signal.
    isConnected = connect(m_pAppNotificationBlinkingTimer, &QTimer::timeout, this, &rgMainWindow::HandleAppNotificationMessageTimerFired);
    assert(isConnected);
}

void rgMainWindow::CreateSettingsTab()
{
    // Get settings tab container widget.
    assert(ui.mainTabWidget != nullptr);
    if (ui.mainTabWidget != nullptr)
    {
        QWidget* pTabContainer = ui.mainTabWidget->widget(1);
        assert(pTabContainer != nullptr);
        if (pTabContainer != nullptr)
        {
            assert(m_pFactory != nullptr);
            if (m_pFactory != nullptr)
            {
                // Delete any existing settings tab first.
                DestroySettingsTab();

                m_pSettingsTab = m_pFactory->CreateSettingsTab(ui.mainTabWidget);
                assert(m_pSettingsTab != nullptr);
                if (m_pSettingsTab != nullptr)
                {
                    // Initialize the tab before adding to the parent widget.
                    m_pSettingsTab->Initialize();

                    rgMainWindowTabBar* pTabBar = ui.mainTabWidget->GetTabBar();
                    assert(pTabBar != nullptr);
                    if (pTabBar != nullptr)
                    {
                        pTabBar->SetSettingsTab(m_pSettingsTab);
                    }

                    m_pAppState->SetSettingsTab(m_pSettingsTab);
                    ui.settingsTab->layout()->addWidget(m_pSettingsTab);

                    // Handle when the Settings tab signals that it has pending changes.
                    bool isConnected = connect(m_pSettingsTab, &rgSettingsTab::PendingChangesStateChanged, this, &rgMainWindow::HandleHasSettingsPendingStateChanged);
                    assert(isConnected);
                    Q_UNUSED(isConnected);
                }
            }
        }
    }
}

void rgMainWindow::DestroySettingsTab()
{
    if (m_pSettingsTab != nullptr)
    {
        delete m_pSettingsTab;
        m_pSettingsTab = nullptr;
    }
}

void rgMainWindow::CreateStartTab()
{
    assert(m_pFactory != nullptr);
    if (m_pFactory != nullptr)
    {
        rgStartTab* pStartTab = m_pFactory->CreateStartTab(this);

        // Register the start tab with the scaling manager.
        ScalingManager::Get().RegisterObject(pStartTab);

        assert(pStartTab != nullptr && m_pAppState != nullptr);
        if (pStartTab != nullptr && m_pAppState != nullptr)
        {
            // Destroy the existing start tab first.
            DestroyStartTab();

            // Set the start page for the mode, and add it to the main window's layout.
            m_pAppState->SetStartTab(pStartTab);
            ui.startTab->layout()->addWidget(pStartTab);

            // Initialize the start tab view.
            pStartTab->Initialize();

            // Connect signals for the new start tab.
            ConnectStartTabSignals();
        }
    }
}

void rgMainWindow::DestroyStartTab()
{
    // Remove the existing widget first, if there is one.
    // This is required because when the user switches APIs,
    // there'll be duplicate widgets on the home page if we
    // don't delete the existing widget here first.
    QLayoutItem* pLayoutItem = ui.startTab->layout()->takeAt(0);
    if (pLayoutItem != nullptr)
    {
        QWidget* pWidget = pLayoutItem->widget();
        assert(pWidget != nullptr);
        if (pWidget != nullptr)
        {
            pWidget->deleteLater();
        }
    }
}

void rgMainWindow::ConnectStartTabSignals()
{
    // Connect signals for the mode's start tab.
    rgStartTab* pStartTab = m_pAppState->GetStartTab();

    assert(pStartTab != nullptr);
    if (pStartTab != nullptr)
    {
        // Signal for the Open Program button click.
        bool isConnected = connect(pStartTab, &rgStartTab::OpenProjectFileEvent, this, &rgMainWindow::HandleOpenProjectFileEvent);
        assert(isConnected);

        // Signal to load the project file at the given path.
        isConnected = connect(pStartTab, &rgStartTab::OpenProjectFileAtPath, this, &rgMainWindow::HandleOpenProjectFileAtPath);
        assert(isConnected);

        // Signal emitted when the Help->About item is clicked.
        isConnected = connect(pStartTab, &rgStartTab::AboutEvent, this, &rgMainWindow::HandleAboutEvent);
        assert(isConnected);

        // Signal emitted when the Help->Getting started guide item is clicked.
        isConnected = connect(pStartTab, &rgStartTab::GettingStartedGuideEvent, this, &rgMainWindow::HandleGettingStartedGuideEvent);
        assert(isConnected);

        // Signal emitted when the Help->Help manual item is clicked.
        isConnected = connect(pStartTab, &rgStartTab::HelpManual, this, &rgMainWindow::HandleHelpManual);
        assert(isConnected);
    }
}

void rgMainWindow::AddBuildView()
{
    assert(m_pAppState != nullptr);
    if (m_pAppState != nullptr)
    {
        // Add the existing rgBuildView instance to the MainWindow's layout.
        rgBuildView* pBuildView = m_pAppState->GetBuildView();

        assert(pBuildView != nullptr);
        if (pBuildView != nullptr)
        {
            // Initialize the rgBuildView instance's interface.
            pBuildView->InitializeView();

            // Connect the menu signals to the rgBuildView and rgMainWindow.
            ConnectMenuSignals();

            // Add the rgBuildView instance to the main window's build page.
            ui.buildPage->layout()->addWidget(pBuildView);

            // Register the object with the scaling manager.
            ScalingManager::Get().RegisterObject(pBuildView);

            // Adjust the actions to the build view's initial state.
            EnableBuildViewActions();

#ifdef RGA_GUI_AUTOMATION
            emit TEST_BuildViewCreated(pBuildView);
#endif
        }
    }
}

void rgMainWindow::ConnectBuildViewSignals()
{
    assert(m_pAppState != nullptr);
    if (m_pAppState != nullptr)
    {
        // Get the API-specific rgBuildView instance used by the current mode.
        rgBuildView* pBuildView = m_pAppState->GetBuildView();

        // Verify that the build view exists before using it.
        assert(pBuildView != nullptr);
        if (pBuildView != nullptr)
        {
            // Editor text change handler.
            bool isConnected = connect(pBuildView, &rgBuildView::CurrentEditorModificationStateChanged, this, &rgMainWindow::OnCurrentEditorModificationStateChanged);
            assert(isConnected);

            // Connect the project count changed signal.
            isConnected = connect(pBuildView, &rgBuildView::ProjectFileCountChanged, this, &rgMainWindow::HandleProjectFileCountChanged);
            assert(isConnected);

            // Connect the project loaded signal.
            isConnected = connect(pBuildView, &rgBuildView::ProjectLoaded, this, &rgMainWindow::HandleProjectLoaded);
            assert(isConnected);

            // Connect the current file modified signal.
            isConnected = connect(pBuildView, &rgBuildView::CurrentFileModified, this, &rgMainWindow::HandleCurrentFileModifiedOutsideEnv);
            assert(isConnected);

            // Connect the project build success signal.
            isConnected = connect(pBuildView, &rgBuildView::ProjectBuildSuccess, this, &rgMainWindow::HandleProjectBuildSuccess);
            assert(isConnected);

            // Connect the project build started signal.
            isConnected = connect(pBuildView, &rgBuildView::ProjectBuildStarted, this, &rgMainWindow::HandleProjectBuildStarted);
            assert(isConnected);

            // Connect the update application notification message signal.
            isConnected = connect(pBuildView, &rgBuildView::UpdateApplicationNotificationMessageSignal, this, &rgMainWindow::HandleUpdateAppNotificationMessage);
            assert(isConnected);

            // Connect the project build failure signal.
            isConnected = connect(pBuildView, &rgBuildView::ProjectBuildFailure, this, &rgMainWindow::HandleProjectBuildFailure);
            assert(isConnected);

            // Connect the project build canceled signal.
            isConnected = connect(pBuildView, &rgBuildView::ProjectBuildCanceled, this, &rgMainWindow::HandleProjectBuildCanceled);
            assert(isConnected);

            // Connect the rgBuildView's status bar update signal.
            isConnected = connect(pBuildView, &rgBuildView::SetStatusBarText, this, &rgMainWindow::HandleStatusBarTextChanged);
            assert(isConnected);

            // Connect the main window's Find event with the rgBuildView.
            isConnected = connect(this, &rgMainWindow::FindTriggered, pBuildView, &rgBuildView::HandleFindTriggered);
            assert(isConnected);

            // Connect the rgMainWindow to the rgBuildView to update the "project is building" flag.
            isConnected = connect(this, &rgMainWindow::IsBuildInProgress, pBuildView, &rgBuildView::HandleIsBuildInProgressChanged);
            assert(isConnected);

            // Connect the edit mode changed signal.
            isConnected = connect(pBuildView, &rgBuildView::EditModeChanged, this, &rgMainWindow::HandleEditModeChanged);
            assert(isConnected);

            // Connect the app state to the rgBuildView.
            m_pAppState->ConnectBuildViewSignals(pBuildView);
        }
    }
}

void rgMainWindow::ConnectMenuSignals()
{
    assert(m_pAppState != nullptr);
    if (m_pAppState != nullptr)
    {
        // Get the API-specific rgBuildView instance used by the current mode.
        rgBuildView* pBuildView = m_pAppState->GetBuildView();

        // Verify that the build view exists before using it.
        assert(pBuildView != nullptr);
        if (pBuildView != nullptr)
        {
            rgMenu* pMenu = pBuildView->GetMenu();
            assert(pMenu != nullptr);
            if (pMenu != nullptr)
            {
                // Connect the file menu with the build start event.
                bool isConnected = connect(pBuildView, &rgBuildView::ProjectBuildStarted, pMenu, &rgMenu::HandleBuildStarted);
                assert(isConnected);

                // Connect the file menu with the build failure event.
                isConnected = connect(pBuildView, &rgBuildView::ProjectBuildFailure, pMenu, &rgMenu::HandleBuildEnded);
                assert(isConnected);

                // Connect the file menu with the build success event.
                isConnected = connect(pBuildView, &rgBuildView::ProjectBuildSuccess, pMenu, &rgMenu::HandleBuildEnded);
                assert(isConnected);

                // Connect the file menu with the build canceled event.
                isConnected = connect(pBuildView, &rgBuildView::ProjectBuildCanceled, pMenu, &rgMenu::HandleBuildEnded);
                assert(isConnected);

                // Connect the source file change event with the handler.
                isConnected = connect(pMenu, &rgMenu::SelectedFileChanged, this, &rgMainWindow::HandleSelectedFileChanged);
                assert(isConnected);

                // Connect the file menu's "Build Settings" button to the main window's handler.
                isConnected = connect(pMenu, &rgMenu::BuildSettingsButtonClicked, this, &rgMainWindow::HandleBuildSettingsEvent);
                assert(isConnected);
            }
        }
    }
}

void rgMainWindow::CreateFileMenuActions()
{
    // Open a project.
    m_pOpenProjectAction = new QAction(tr(STR_MENU_BAR_OPEN_PROJECT), this);
    m_pOpenProjectAction->setShortcut(QKeySequence(gs_ACTION_HOTKEY_OPEN_PROJECT));
    m_pOpenProjectAction->setStatusTip(tr(STR_MENU_BAR_OPEN_PROJECT_TOOLTIP));
    bool isConnected = connect(m_pOpenProjectAction, &QAction::triggered, this, &rgMainWindow::HandleOpenProjectFileEvent);
    assert(isConnected);

    // Save a file.
    m_pSaveAction = new QAction(tr(STR_MENU_BAR_SAVE_FILE), this);
    m_pSaveAction->setShortcuts(QKeySequence::Save);
    m_pSaveAction->setStatusTip(tr(STR_MENU_BAR_SAVE_FILE_TOOLTIP));
    isConnected = connect(m_pSaveAction, &QAction::triggered, this, &rgMainWindow::HandleSaveFileEvent);
    assert(isConnected);

    // Back to home.
    m_pBackToHomeAction = new QAction(tr(STR_MENU_BAR_BACK_TO_HOME), this);
    m_pBackToHomeAction->setShortcut(QKeySequence(gs_ACTION_HOTKEY_BACK_TO_HOME));
    m_pBackToHomeAction->setStatusTip(tr(STR_MENU_BAR_BACK_TO_HOME_TOOLTIP));
    isConnected = connect(m_pBackToHomeAction, &QAction::triggered, this, &rgMainWindow::HandleBackToHomeEvent);
    assert(isConnected);

    // Exit the project.
    m_pExitAction = new QAction(tr(STR_MENU_BAR_EXIT), this);
    m_pExitAction->setShortcut(QKeySequence(gs_ACTION_HOTKEY_EXIT));
    m_pExitAction->setStatusTip(tr(STR_MENU_BAR_EXIT_TOOLTIP));
    isConnected = connect(m_pExitAction, &QAction::triggered, this, &rgMainWindow::HandleExitEvent);
    assert(isConnected);

    // Start with some actions disabled.
    m_pSaveAction->setDisabled(true);
}

void rgMainWindow::CreateAppState(rgProjectAPI api)
{
    // Create a factory matching the new API mode to switch to.
    std::shared_ptr<rgFactory> pFactory = rgFactory::CreateFactory(api);
    assert(pFactory != nullptr);
    if (pFactory != nullptr)
    {
        std::shared_ptr<rgAppState> pApplicationState = pFactory->CreateAppState();
        assert(pApplicationState != nullptr);
        if (pApplicationState != nullptr)
        {
            pApplicationState->SetMainWindow(this);

            // Replace the app state with the one for the new mode.
            m_pAppState = pApplicationState;

            // Reset the rgBuildView instance used to interact with the project.
            m_pAppState->ResetBuildView();

            // Connect signals for the Build View.
            ConnectBuildViewSignals();

            // Set main window stylesheet.
            ApplyMainWindowStylesheet();

            // Use the factory for the new API mode.
            m_pFactory = pFactory;
        }
    }
}

void rgMainWindow::ChangeApiMode(rgProjectAPI api)
{
    if (m_pAppState != nullptr)
    {
        // Clean up the existing mode before switching to the new one.
        m_pAppState->Cleanup(m_pMenuBar);
    }

    // Update the current API value in config manager.
    rgConfigManager& configManager = rgConfigManager::Instance();
    configManager.SetCurrentAPI(api);

    // Create the new app state.
    CreateAppState(api);

    // Create the start tab for the new mode.
    CreateStartTab();

    // Create the settings tab for the new mode.
    CreateSettingsTab();

    // Create the custom status bar for the new mode.
    CreateCustomStatusBar();

    // Update the window title to reflect the current API mode.
    ResetWindowTitle();

    // Set application stylesheet.
    SetApplicationStylesheet();

    // Create the menu bar.
    CreateMenuBar();

    // Reset the state of the action to the default state.
    ResetActionsState();

    // Connect the signals.
    ConnectSignals();

    // Create the custom status bar.
    CreateCustomStatusBar();

    // Switch the main window view to the home page.
    SwitchToView(MainWindowView::Home);

    // Set focus to the main tab widget so keyboard shortcuts still work.
    ui.mainTabWidget->setFocus();
}

void rgMainWindow::DestroyFileMenu()
{
    RG_SAFE_DELETE(m_pMenuBar);
}

void rgMainWindow::CreateMenuBar()
{
    // Clear the existing menu bar and recreate it for the new mode.
    menuBar()->clear();

    // Create the file menu.
    CreateFileMenu();

    // Create the Edit menu.
    CreateEditMenu();

    // Create the Build menu.
    CreateBuildMenu();

    // Create the help menu.
    CreateHelpMenu();
}

void rgMainWindow::CreateFileMenu()
{
    // Add the File menu to the main window's menu bar.
    m_pMenuBar = menuBar()->addMenu(tr(STR_MENU_BAR_FILE));

    // Add file actions based on the current mode.
    m_pAppState->CreateFileActions(m_pMenuBar);

    // Create the actions to be used by the menus.
    CreateFileMenuActions();
    m_pMenuBar->addAction(m_pOpenProjectAction);
    m_pMenuBar->addAction(m_pSaveAction);
    m_pMenuBar->addSeparator();
    m_pMenuBar->addAction(m_pBackToHomeAction);
    m_pMenuBar->addSeparator();
    m_pMenuBar->addAction(m_pExitAction);

    // Set the mouse cursor to pointing hand cursor.
    m_pMenuBar->setCursor(Qt::PointingHandCursor);

#ifdef __linux
    // Workaround for broken shortcuts on linux - adding the actions to the main window as
    // well as the menu bar seems to fix the issue.
    // @TODO Remove this if/when we update to a version of Qt without the bug.
    addAction(m_pOpenProjectAction);
    addAction(m_pSaveAction);
    addAction(m_pBackToHomeAction);
    addAction(m_pExitAction);
#endif // __linux
}

void rgMainWindow::CreateEditMenu()
{
    // Create the actions to be used by the menus.
    CreateEditMenuActions();

    m_pMenuBar = menuBar()->addMenu(tr(STR_MENU_BAR_EDIT));
    m_pMenuBar->addAction(m_pGoToLineAction);
    m_pMenuBar->addAction(m_pFindAction);

    // Set the mouse cursor to pointing hand cursor.
    m_pMenuBar->setCursor(Qt::PointingHandCursor);

#ifdef __linux
    // Workaround for broken shortcuts on linux - adding the actions to the main window as
    // well as the menu bar seems to fix the issue.
    // @TODO Remove this if/when we update to a version of Qt without the bug.
    addAction(m_pGoToLineAction);
#endif // __linux
}

void rgMainWindow::CreateHelpMenu()
{
    CreateHelpMenuActions();

    m_pMenuBar = menuBar()->addMenu(tr(STR_MENU_BAR_HELP));

    // About.
    m_pMenuBar->addAction(m_pHelpAboutAction);

    // Help manual.
    m_pMenuBar->addAction(m_pHelpManualAction);

    // Getting started guide.
    m_pMenuBar->addAction(m_pHelpGettingStartedGuideAction);

    // Set the mouse cursor to pointing hand cursor.
    m_pMenuBar->setCursor(Qt::PointingHandCursor);
}

void rgMainWindow::CreateHelpMenuActions()
{
    // Create and connect the About action.
    m_pHelpAboutAction = new QAction(tr(STR_MENU_BAR_HELP_ABOUT), this);
    m_pHelpAboutAction->setStatusTip(tr(STR_MENU_BAR_HELP_ABOUT_TOOLTIP));
    m_pHelpAboutAction->setShortcut(QKeySequence(gs_ACTION_HOTKEY_ABOUT));
    bool isConnected = connect(m_pHelpAboutAction, &QAction::triggered, this, &rgMainWindow::HandleAboutEvent);
    assert(isConnected);

    // Create and connect the help manual guide action.
    m_pHelpManualAction = new QAction(tr(STR_MENU_BAR_HELP_MANUAL), this);
    m_pHelpManualAction->setStatusTip(tr(STR_MENU_BAR_HELP_MANUAL_TOOLTIP));
    m_pHelpManualAction->setShortcut(QKeySequence(gs_ACTION_HOTKEY_HELP_MANUAL));
    isConnected = connect(m_pHelpManualAction, &QAction::triggered, this, &rgMainWindow::HandleHelpManual);
    assert(isConnected);

    // Create and connect the getting started guide action.
    m_pHelpGettingStartedGuideAction = new QAction(tr(STR_MENU_BAR_HELP_GETTING_STARTED_GUIDE), this);
    m_pHelpGettingStartedGuideAction->setStatusTip(tr(STR_MENU_BAR_HELP_GETTING_STARTED_GUIDE_TOOLTIP));
    isConnected = connect(m_pHelpGettingStartedGuideAction, &QAction::triggered, this, &rgMainWindow::HandleGettingStartedGuideEvent);
    assert(isConnected);
}

void rgMainWindow::CreateBuildMenuActions()
{
    // Build current project action.
    m_pBuildProjectAction = new QAction(tr(STR_MENU_BAR_BUILD_PROJECT), this);
    m_pBuildProjectAction->setStatusTip(tr(STR_MENU_BAR_BUILD_PROJECT_TOOLTIP));
    m_pBuildProjectAction->setShortcut(QKeySequence(gs_ACTION_HOTKEY_BUILD_PROJECT));
    bool isConnected = connect(m_pBuildProjectAction, &QAction::triggered, this, &rgMainWindow::HandleBuildProjectEvent);
    assert(isConnected);

    // Open the project's build settings.
    m_pBuildSettingsAction = new QAction(tr(STR_MENU_BUILD_SETTINGS), this);
    m_pBuildSettingsAction->setShortcut(QKeySequence(gs_ACTION_HOTKEY_BUILD_SETTINGS));
    m_pBuildSettingsAction->setStatusTip(tr(STR_MENU_BAR_BUILD_SETTINGS_TOOLTIP));
    isConnected = connect(m_pBuildSettingsAction, SIGNAL(triggered()), this, SLOT(HandleBuildSettingsEvent()));
    assert(isConnected);

    // Open the project's pipeline state editor.
    m_pPipelineStateAction = new QAction(tr(STR_MENU_PIPELINE_STATE_EDITOR), this);
    m_pPipelineStateAction->setShortcut(QKeySequence(gs_ACTION_HOTKEY_PIPELINE_STATE));
    m_pPipelineStateAction->setStatusTip(tr(STR_MENU_BAR_PIPELINE_STATE_TOOLTIP));
    isConnected = connect(m_pPipelineStateAction, SIGNAL(triggered()), this, SLOT(HandlePipelineStateEvent()));
    assert(isConnected);

    // Cancel the current build.
    m_pCancelBuildAction = new QAction(tr(STR_MENU_CANCEL_BUILD), this);
    m_pCancelBuildAction->setShortcut(QKeySequence(gs_ACTION_HOTKEY_BUILD_CANCEL));
    m_pCancelBuildAction->setStatusTip(tr(STR_MENU_BAR_BUILD_CANCEL_TOOLTIP));
    isConnected = connect(m_pCancelBuildAction, SIGNAL(triggered()), this, SLOT(HandleCancelBuildEvent()));
    assert(isConnected);
}

void rgMainWindow::CreateBuildMenu()
{
    CreateBuildMenuActions();

    m_pMenuBar = menuBar()->addMenu(tr(STR_MENU_BAR_BUILD));
    m_pMenuBar->addAction(m_pBuildProjectAction);
    assert(m_pAppState != nullptr);
    // For graphics APIs, add the pipeline state action.
    if (m_pAppState != nullptr && m_pAppState->IsGraphics())
    {
        m_pMenuBar->addAction(m_pPipelineStateAction);
    }
    m_pMenuBar->addAction(m_pBuildSettingsAction);
    m_pMenuBar->addAction(m_pCancelBuildAction);

    // Set the mouse cursor to pointing hand cursor.
    m_pMenuBar->setCursor(Qt::PointingHandCursor);
}

void rgMainWindow::DestroyBuildView()
{
    // Before destroying the rgBuildView, switch the save mode so that save shortcut signals.
    // Signals will get forwarded to the rgMainWindow instead of the (dead) rgBuildView.
    SwitchSaveShortcut(SaveActionType::SaveFile);

    // Reset the actions state to default.
    ResetActionsState();

    // Remove the rgBuildView instance from the MainWindow layout.
    assert(m_pAppState != nullptr);
    if (m_pAppState != nullptr)
    {
        // Remove the rgBuildView instance widget from the MainWindow.
        rgBuildView* pBuildView = m_pAppState->GetBuildView();
        ui.buildPage->layout()->removeWidget(pBuildView);

        // Reset the app state's rgBuildView instance so that it can be used for a new project.
        m_pAppState->ResetBuildView();

        // Connect signals for the new rgBuildView instance.
        ConnectBuildViewSignals();

        // Remove the app notification label.
        this->statusBar()->removeWidget(m_pAppNotificationWidget);
    }
}

void rgMainWindow::EnableBuildMenu(bool isEnabled)
{
    // Toggle the actions associated with the build menu items.
    m_pBuildProjectAction->setEnabled(isEnabled);

    // Make sure that the Build and Cancel options are never both enabled.
    if (isEnabled)
    {
        m_pCancelBuildAction->setEnabled(false);
    }
}

void rgMainWindow::CreateEditMenuActions()
{
    // Build current project action.
    m_pGoToLineAction = new QAction(tr(STR_MENU_BAR_GO_TO_LINE), this);
    m_pGoToLineAction->setStatusTip(tr(STR_MENU_BAR_GO_TO_LINE_TOOLTIP));
    m_pGoToLineAction->setShortcut(QKeySequence(gs_ACTION_HOTKEY_GO_TO_LINE));
    bool isConnected = connect(m_pGoToLineAction, &QAction::triggered, this, &rgMainWindow::HandleGoToLineEvent);
    assert(isConnected);

    // Find a string in the source editor.
    m_pFindAction = new QAction(tr(STR_MENU_BAR_EDIT_QUICK_FIND), this);
    m_pFindAction->setShortcut(QKeySequence(gs_ACTION_HOTKEY_FIND));
    m_pFindAction->setStatusTip(tr(STR_MENU_BAR_EDIT_QUICK_FIND_TOOLTIP));
    isConnected = connect(m_pFindAction, &QAction::triggered, this, &rgMainWindow::FindTriggered);
    assert(isConnected);
}

void rgMainWindow::EnableEditMenu(bool isEnabled)
{
    // Toggle the actions associated with the Edit menu items.
    m_pGoToLineAction->setEnabled(isEnabled);
    m_pFindAction->setEnabled(isEnabled);
}

bool rgMainWindow::OpenProjectFileAtPath(const std::string& projectFilePath)
{
    bool isProjectLoaded = false;

    assert(!projectFilePath.empty());
    if (!projectFilePath.empty())
    {
        // Read the project file to determine the project's API. The GUI's current mode may have to
        // be changed to match the API used by the project.
        std::shared_ptr<rgProject> pProject = nullptr;
        isProjectLoaded = rgXmlConfigFile::ReadProjectConfigFile(projectFilePath, pProject);

        assert(isProjectLoaded);
        assert(pProject != nullptr);
        if (isProjectLoaded && pProject != nullptr)
        {
            // Change the API mode if the project API doesn't match the current mode.
            rgProjectAPI currentApi = rgConfigManager::Instance().GetCurrentAPI();
            if (pProject->m_api != currentApi)
            {
                ChangeApiMode(pProject->m_api);
            }
        }

        assert(m_pAppState != nullptr);
        if (m_pAppState != nullptr)
        {
            rgBuildView* pBuildView = m_pAppState->GetBuildView();
            if (pBuildView != nullptr)
            {
                // Attempt to load the user's selected project.
                isProjectLoaded = pBuildView->LoadProjectFile(projectFilePath);
                assert(isProjectLoaded);

                // If the project was loaded correctly, create and populate the rgBuildView with the source files.
                if (isProjectLoaded)
                {
                    // Insert the current mode's rgBuildView instance to the main window.
                    AddBuildView();

                    // Populate the rgBuildView with the loaded project.
                    bool isPopulated = pBuildView->PopulateBuildView();
                    if (isPopulated)
                    {
                        // Show the build view as the central widget.
                        SwitchToView(MainWindowView::BuildView);

                        // Get the directory where the project file lives.
                        std::string projectDirectory;
                        bool isOk = rgUtils::ExtractFileDirectory(projectFilePath, projectDirectory);
                        assert(isOk);
                        if (isOk)
                        {
                            // Try to load existing build output within the project directory.
                            bool isBuildOutputLoaded = pBuildView->LoadBuildOutput(projectDirectory);
                            if (isBuildOutputLoaded)
                            {
                                // Previous build outputs were loaded correctly.
                                // Emit the signal indicating a build has succeeded, which will re-populate the view.
                                pBuildView->HandleProjectBuildSuccess();

                                // Restore the rgBuildView to the last-used layout dimensions.
                                pBuildView->RestoreViewLayout();
                            }
                        }
                    }
                    else
                    {
                        // The project wasn't loaded properly. Return to the home page.
                        HandleBackToHomeEvent();
                        isProjectLoaded = false;
                    }
                }
            }
        }
    }

    return isProjectLoaded;
}

void rgMainWindow::SetWindowTitle(const std::string& titleText)
{
    // Add the application name to the title text.
    std::stringstream windowTitle;
    windowTitle << STR_APP_NAME << " - ";

    // Determine which API is being used.
    std::string apiName;
    rgProjectAPI projectAPI = rgConfigManager::Instance().GetCurrentAPI();
    bool isOk = rgUtils::ProjectAPIToString(projectAPI, apiName);
    assert(isOk);
    if (isOk)
    {
        // Add the current API name to the title text.
        windowTitle << apiName << " ";
        windowTitle << STR_TITLE_PROJECT << " - ";
    }

    // Finally append the incoming string to the title text.
    windowTitle << titleText;
    this->setWindowTitle(windowTitle.str().c_str());
}

void rgMainWindow::ResetWindowTitle()
{
    // Create the default title with the application name.
    std::stringstream windowTitle;
    windowTitle << STR_APP_NAME;

    // Determine which API is being used.
    std::string apiName;
    rgProjectAPI projectAPI = rgConfigManager::Instance().GetCurrentAPI();
    bool isOk = rgUtils::ProjectAPIToString(projectAPI, apiName);
    assert(isOk);
    if (isOk)
    {
        // Add the current API name to the title text.
        windowTitle << " - ";
        windowTitle << apiName << " ";
    }

    // Add the application name to the title text.
    this->setWindowTitle(windowTitle.str().c_str());
}

void rgMainWindow::HandleHasSettingsPendingStateChanged(bool hasPendingChanges)
{
    // Update the File->Save enabled state.
    OnCurrentEditorModificationStateChanged(hasPendingChanges);
}

void rgMainWindow::HandleProjectFileCountChanged(bool isProjectEmpty)
{
    // Toggle the actions associated with the build menu items.
    m_pBuildProjectAction->setEnabled(!isProjectEmpty);

    // Make sure that the Build and Cancel options are never both enabled.
    if (!isProjectEmpty)
    {
        m_pCancelBuildAction->setEnabled(false);
    }

    // In this specific case, make sure that the build settings and, if applicable,
    // the pipeline state items are enabled, since we are in a project (and not in the home page).
    m_pBuildSettingsAction->setEnabled(true);
    assert(m_pAppState != nullptr);
    if (m_pAppState != nullptr && m_pAppState->IsGraphics() &&
        m_pPipelineStateAction != nullptr)
    {
        m_pPipelineStateAction->setEnabled(true);
    }

    // Toggle the availability of the Edit menu.
    EnableEditMenu(!isProjectEmpty);
}

void rgMainWindow::SwitchToView(MainWindowView tab)
{
    QWidget* pWidget = nullptr;
    switch (tab)
    {
    case MainWindowView::Home:
        pWidget = ui.homePage;

        // Show the custom status bar so the mode and the API buttons show while on home page.
        assert(m_pStatusBar != nullptr);
        if (m_pStatusBar != nullptr)
        {
            m_pStatusBar->SetStatusBarVisibility(true);
            rgUtils::SetToolAndStatusTip("", this);
        }

        // Update the current view.
        m_currentView = tab;

        break;
    case MainWindowView::BuildView:
        pWidget = ui.buildPage;

        // Hide the custom status bar so the mode and the API buttons do not show while in build view.
        assert(m_pStatusBar != nullptr);
        if (m_pStatusBar != nullptr)
        {
            m_pStatusBar->SetStatusBarVisibility(false);
        }

        // Set initial focus to file menu.
        if (m_pAppState != nullptr)
        {
            rgBuildView* pBuildView = m_pAppState->GetBuildView();
            if (pBuildView != nullptr)
            {
                pBuildView->FocusOnFileMenu();
            }
        }

        // Update the current view.
        m_currentView = tab;

        break;
    default:
        assert(false);
    }

    if (pWidget != nullptr)
    {
        ui.stackedWidget->setCurrentWidget(pWidget);

        // Enable/disable back to home menu item.
        bool isBackToHomeEnabled = (pWidget != ui.homePage);
        m_pBackToHomeAction->setEnabled(isBackToHomeEnabled);
    }
}

void rgMainWindow::HandleOpenProjectFileEvent()
{
    // Ask user to save pending settings.
    bool didUserConfirm = true;

    if (!IsInputFileNameBlank())
    {

        assert(m_pSettingsTab != nullptr);
        if (m_pSettingsTab != nullptr)
        {
            didUserConfirm = m_pSettingsTab->PromptToSavePendingChanges();
        }

        if (didUserConfirm)
        {
            if (m_pAppState != nullptr)
            {
                didUserConfirm = m_pAppState->ShowProjectSaveDialog();
            }

            if (didUserConfirm)
            {
                std::string selectedFile;
                bool isOk = rgUtils::OpenProjectDialog(this, selectedFile);
                if (isOk && !selectedFile.empty())
                {
                    // Destroy current BuildView if it has some project open.
                    assert(m_pAppState != nullptr);
                    if (m_pAppState != nullptr)
                    {
                        rgBuildView* pBuildView = m_pAppState->GetBuildView();
                        if (pBuildView != nullptr && pBuildView->HasProject())
                        {
                            // Ask the user if they want to save all unsaved files.
                            bool isAccepted = pBuildView->RequestRemoveAllFiles();

                            // Destroy the rgBuildView instance since the project is being closed.
                            DestroyBuildView();
                        }
                    }

                    HandleStatusBarTextChanged(STR_MAIN_WINDOW_LOADING_PROJECT, gs_STATUS_BAR_NOTIFICATION_TIMEOUT_MS);
                    bool isProjectLoaded = OpenProjectFileAtPath(selectedFile);
                    assert(isProjectLoaded);
                    if (isProjectLoaded)
                    {
                        HandleStatusBarTextChanged(STR_MAIN_WINDOW_PROJECT_LOAD_SUCCESS, gs_STATUS_BAR_NOTIFICATION_TIMEOUT_MS);
                    }
                    else
                    {
                        HandleStatusBarTextChanged(STR_ERR_CANNOT_LOAD_PROJECT_FILE, gs_STATUS_BAR_NOTIFICATION_TIMEOUT_MS);
                    }
                }
            }
        }
    }
}

bool rgMainWindow::IsInputFileNameBlank() const
{
    bool result = false;

    assert(m_pAppState != nullptr);
    if (m_pAppState != nullptr)
    {
        if (m_pAppState->IsInputFileNameBlank())
        {
            result = true;
        }
    }

    return result;
}

void rgMainWindow::HandleOpenProjectFileAtPath(const std::string& programFilePath)
{
    // Add a loading message to the status bar.
    HandleStatusBarTextChanged(STR_MAIN_WINDOW_LOADING_PROJECT, gs_STATUS_BAR_NOTIFICATION_TIMEOUT_MS);

    bool isOk = OpenProjectFileAtPath(programFilePath);

    assert(isOk);
    if (isOk)
    {
        HandleStatusBarTextChanged(STR_MAIN_WINDOW_PROJECT_LOAD_SUCCESS, gs_STATUS_BAR_NOTIFICATION_TIMEOUT_MS);
    }
    else
    {
        HandleStatusBarTextChanged(STR_ERR_CANNOT_LOAD_PROJECT_FILE, gs_STATUS_BAR_NOTIFICATION_TIMEOUT_MS);
    }
}

void rgMainWindow::HandleSaveFileEvent()
{
    assert(m_pAppState != nullptr);
    if (m_pAppState != nullptr)
    {
        const int currentIndex = ui.mainTabWidget->currentIndex();
        if (currentIndex == 0)
        {
            // If the rgBuildView exists, forward the save signal.
            rgBuildView* pBuildView = m_pAppState->GetBuildView();
            if (pBuildView != nullptr)
            {
                // Save the current source file or build settings.
                pBuildView->HandleSaveSettingsButtonClicked();
            }
            else
            {
                if (m_pSettingsTab != nullptr)
                {
                    m_pSettingsTab->SavePendingChanges();
                }
            }
        }
        else if (currentIndex == 1)
        {
            if (m_pSettingsTab != nullptr)
            {
                if (!IsInputFileNameBlank())
                {
                    m_pSettingsTab->SavePendingChanges();
                }
            }
        }
    }
}

void rgMainWindow::HandleBackToHomeEvent()
{
    assert(m_pAppState != nullptr);
    if (m_pAppState != nullptr)
    {
        rgBuildView* pBuildView = m_pAppState->GetBuildView();
        if (pBuildView != nullptr)
        {
            pBuildView->SaveProjectConfigFile();

            // Refresh the recent projects list upon returning to the start tab.
            rgStartTab* pStartTab = m_pAppState->GetStartTab();

            assert(pStartTab != nullptr);
            if (pStartTab != nullptr)
            {
                pStartTab->PopulateRecentProjectsList();
            }

            // Ask the user if they want to save all unsaved files.
            bool isAccepted = pBuildView->RequestRemoveAllFiles();

            // Only exit if the dialog was accepted (ie. the user didn't press cancel).
            if (isAccepted)
            {
                // Switch the view back to the home page.
                SwitchToView(MainWindowView::Home);

                // Destroy the rgBuildView instance since the project is being closed.
                DestroyBuildView();

                // Reset the window title.
                ResetWindowTitle();

                // Reset the status bar.
                this->statusBar()->showMessage("");
            }
        }
    }
}

void rgMainWindow::HandleExitEvent()
{
    static const int s_TOP_MARGIN = 8;
    bool isSaveSettingsAccepted = true;

    // Prompt the user to save any pending changes on SETTINGS tab.

    if (!IsInputFileNameBlank())
    {
        assert(m_pSettingsTab != nullptr);
        if (m_pSettingsTab != nullptr)
        {
            isSaveSettingsAccepted = m_pSettingsTab->PromptToSavePendingChanges();
        }

        assert(m_pAppState != nullptr);
        if (m_pAppState != nullptr)
        {
            rgBuildView* pBuildView = m_pAppState->GetBuildView();
            if (pBuildView != nullptr)
            {
                // Ask the user if they want to save all unsaved files.
                bool isAccepted = pBuildView->ShowSaveDialog(rgBuildView::rgFilesToSave::All, true);

                // Only exit if the dialog was accepted (ie. the user didn't press cancel).
                if (isAccepted && isSaveSettingsAccepted)
                {
                    // Save the window geometry.
                    rgConfigManager::Instance().SetWindowGeometry(pos().x(), pos().y() + statusBar()->height() + s_TOP_MARGIN, size().width(), size().height(), windowState());

                    QApplication::exit(0);
                }
            }
            else
            {
                // Exit out of the application only if the user did NOT press cancel.
                if (isSaveSettingsAccepted)
                {

                    // Save the window geometry.
                    rgConfigManager::Instance().SetWindowGeometry(pos().x(), pos().y() + statusBar()->height() + s_TOP_MARGIN, size().width(), size().height(), windowState());

                    QApplication::exit(0);
                }
            }
        }
    }
}

void rgMainWindow::closeEvent(QCloseEvent* pEvent)
{
    // Ignore the close event so we can handle shutdown ourself.
    pEvent->ignore();

    HandleExitEvent();
}

void rgMainWindow::dragEnterEvent(QDragEnterEvent* pEvent)
{
    // Do not let the user drag and drop a file onto the "SETTINGS" tab.
    if (ui.mainTabWidget->currentIndex() == 1 || m_currentView == MainWindowView::BuildView)
    {
        pEvent->ignore();
    }
    else
    {
        const QMimeData* pMimeData = pEvent->mimeData();

        // Make sure the drop data has a list of file urls.
        if (pMimeData->hasUrls())
        {
            // Check to make sure at least one of the files is valid.
            for (QUrl& url : pMimeData->urls())
            {
                if (url.isLocalFile())
                {
                    std::string filePath = url.toLocalFile().toStdString();
                    if (rgUtils::IsSourceFileTypeValid(filePath))
                    {
                        // Accept the action, making it so we receive a dropEvent when the items are released.
                        pEvent->setDropAction(Qt::DropAction::CopyAction);
                        pEvent->accept();
                    }
                }
            }
        }
    }
}

void rgMainWindow::dropEvent(QDropEvent *pEvent)
{
    // Get list of all file urls.
    QList<QUrl> fileUrls = pEvent->mimeData()->urls();
    // If there is only one file and it is a project file (.rga),
    // call OpenProjectFileAtPath.
    if (fileUrls.size() == 1 && fileUrls.at(0).isLocalFile() && fileUrls.at(0).toLocalFile().endsWith(STR_PROJECT_FILE_EXTENSION))
    {
        bool isLoadSuccessful = OpenProjectFileAtPath(fileUrls.at(0).toLocalFile().toStdString());
        assert(isLoadSuccessful);
        if (isLoadSuccessful)
        {
            this->statusBar()->showMessage(STR_MAIN_WINDOW_PROJECT_LOAD_SUCCESS);
        }
        else
        {
            this->statusBar()->showMessage(STR_ERR_CANNOT_LOAD_PROJECT_FILE);
        }
    }
    else
    {
        // Convert url list to a string list of local file paths.
        QStringList filenameStrings;
        for (QUrl& fileUrl : fileUrls)
        {
            if (fileUrl.isLocalFile())
            {
                QString filename = fileUrl.toLocalFile();
                // Filter out all files that aren't valid source files.
                if (rgUtils::IsSourceFileTypeValid(filename.toStdString()))
                {
                    filenameStrings.push_back(filename);
                }
            }
        }
        // Disable additional drops and open the desired files in the build view.
        assert(m_pAppState != nullptr);
        if (m_pAppState != nullptr)
        {
            setAcceptDrops(false);
            m_pAppState->OpenFilesInBuildView(filenameStrings);
            setAcceptDrops(true);
        }
    }
    // Set the focus to main window so the user can use keyboard shortcuts.
    setFocus();
}

void rgMainWindow::OnCurrentEditorModificationStateChanged(bool isModified)
{
    m_pSaveAction->setEnabled(isModified);

    if (m_pSaveAction->text().compare(STR_MENU_BAR_SAVE_FILE) == 0)
    {
        // Save the current state of save file shortcut.
        m_saveFileActionActive = isModified;
    }
    else if (m_pSaveAction->text().compare(STR_MENU_BAR_SAVE_SETTINGS) == 0)
    {
        // Save the current state of save settings shortcut.
        m_saveSettingsActionActive = isModified;
    }
    else
    {
        // Should not get here.
        assert(false);
    }
}

void rgMainWindow::HandleAboutEvent()
{
    rgAboutDialog aboutDialog(this);

    // Display the About dialog.
    aboutDialog.exec();
}

void rgMainWindow::HandleGettingStartedGuideEvent()
{
    // Build the path to the quickstart guide document.
    static const char* QUICKSTART_GUIDE_RELATIVE_PATH = "/docs/help/rga/html/quickstart.html";
    QString quickstartFilePath = "file:///";
    QString appDirPath = qApp->applicationDirPath();
    quickstartFilePath.append(appDirPath);
    quickstartFilePath.append(QUICKSTART_GUIDE_RELATIVE_PATH);
    QDesktopServices::openUrl(QUrl(quickstartFilePath, QUrl::TolerantMode));
}

void rgMainWindow::HandleHelpManual()
{
    // Build the path to the help manual document.
    static const char* HELP_MANUAL_RELATIVE_PATH = "/docs/help/rga/html/help_manual.html";
    QString helpManualFilePath = "file:///";
    QString appDirPath = qApp->applicationDirPath();
    helpManualFilePath.append(appDirPath);
    helpManualFilePath.append(HELP_MANUAL_RELATIVE_PATH);
    QDesktopServices::openUrl(QUrl(helpManualFilePath, QUrl::TolerantMode));
}

void rgMainWindow::HandleBuildProjectEvent()
{
    assert(m_pAppState != nullptr);
    if (m_pAppState != nullptr)
    {
        // Emit a signal to indicate view change.
        emit HotKeyPressedSignal();

        rgBuildView* pBuildView = m_pAppState->GetBuildView();
        if (pBuildView != nullptr)
        {
            // Save all source files and settings when a new build is started.
            if (pBuildView->SaveCurrentState())
            {
                // Now build the project.
                pBuildView->BuildCurrentProject();
            }
        }
    }
}

void rgMainWindow::HandleBuildSettingsEvent()
{
    // Emit a signal to indicate view change.
    emit HotKeyPressedSignal();

    assert(m_pAppState != nullptr);
    if (m_pAppState != nullptr)
    {
        rgBuildView* pBuildView = m_pAppState->GetBuildView();
        if (pBuildView != nullptr)
        {
            pBuildView->OpenBuildSettings();
        }
    }

    // Disable the find functionality.
    m_pFindAction->setEnabled(false);

    // Disable the Go-to-line functionality.
    m_pGoToLineAction->setEnabled(false);

    // Switch the file menu save file action to save build settings action.
    SwitchSaveShortcut(SaveActionType::SaveSettings);

    // Update the file menu save action visibility.
    OnCurrentEditorModificationStateChanged(m_saveSettingsActionActive);

    // Give focus to "Build settings" button and remove focus from others.
    rgBuildView* pBuildView = m_pAppState->GetBuildView();
    if (pBuildView != nullptr)
    {
        rgMenu* pMenu = pBuildView->GetMenu();
        assert(pMenu != nullptr);
        if (pMenu != nullptr)
        {
            pMenu->DeselectItems();
            auto pBuildSettingsItem = pMenu->GetBuildSettingsItem();
            assert(pBuildSettingsItem != nullptr);
            if (pBuildSettingsItem != nullptr)
            {
                pBuildSettingsItem->SetCurrent(true);
                if (pBuildSettingsItem->GetBuildSettingsButton() != nullptr)
                {
                    pBuildSettingsItem->GetBuildSettingsButton()->setStyleSheet(s_BUTTON_FOCUS_IN_STYLESHEET);
                }
            }
        }
    }
}

void rgMainWindow::HandlePipelineStateEvent()
{
    // Switch to the pipeline state view.
    assert(m_pAppState != nullptr);
    if (m_pAppState != nullptr)
    {
        std::shared_ptr<rgAppStateGraphics> pGraphicsAppState =
            std::static_pointer_cast<rgAppStateGraphics>(m_pAppState);

        assert(pGraphicsAppState != nullptr);
        if (pGraphicsAppState != nullptr)
        {
            pGraphicsAppState->HandlePipelineStateEvent();
        }
    }
}

void rgMainWindow::HandleCancelBuildEvent()
{
    assert(m_pAppState != nullptr);
    if (m_pAppState != nullptr)
    {
        rgBuildView* pBuildView = m_pAppState->GetBuildView();
        if (pBuildView != nullptr)
        {
            // Ask the build view to cancel the current build.
            pBuildView->CancelCurrentBuild();
        }
    }
}

void rgMainWindow::HandleSelectedFileChanged(const std::string& oldFile, const std::string& newFile)
{
    // Enable the find functionality.
    m_pFindAction->setEnabled(true);

    // Enable the Go-to-line functionality.
    m_pGoToLineAction->setEnabled(true);
}

void rgMainWindow::HandleProjectCreated()
{
    // Add the rgBuildView instance to the window layout.
    AddBuildView();
}

void rgMainWindow::HandleProjectLoaded(std::shared_ptr<rgProject> pProject)
{
    assert(pProject != nullptr);
    if (pProject != nullptr)
    {
        // Update the application title with the project's name.
        SetWindowTitle(pProject->m_projectName);
        assert(pProject->m_clones[0] != nullptr);

        // If there are no files in the project, disable source and build related menu items.
        if ((pProject->m_clones[0] != nullptr) && (pProject->IsEmpty()))
        {
            // Disable the build-related actions, except for the build settings action.
            m_pBuildProjectAction->setEnabled(false);
            m_pBuildSettingsAction->setEnabled(true);
            assert(m_pAppState != nullptr);
            if (m_pAppState != nullptr && m_pAppState->IsGraphics() &&
                m_pPipelineStateAction != nullptr)
            {
                m_pPipelineStateAction->setEnabled(true);
            }
            m_pCancelBuildAction->setEnabled(false);

            // Disable the source-file related actions.
            EnableEditMenu(false);
        }
        emit TEST_ProjectLoaded();
    }
}

void rgMainWindow::HandleCurrentFileModifiedOutsideEnv()
{
    // Notify the user about the file modification through the status bar.
    HandleStatusBarTextChanged(STR_STATUS_BAR_FILE_MODIFIED_OUTSIDE_ENV, gs_STATUS_BAR_NOTIFICATION_TIMEOUT_MS);
}

// Manual handling of focus switching because qt doesn't allow the shortcut to be anything other than "Tab".
void rgMainWindow::HandleFocusNextWidget()
{
    QWidget* pFocusWidget = QApplication::focusWidget();

    if (pFocusWidget != nullptr)
    {
        QWidget* pTestWidget = QApplication::focusWidget()->nextInFocusChain();

        // Step through focus widgets until one is found which accepts focus.
        while (pTestWidget != nullptr)
        {
            // Check that the widget accepts focus and is visible to the current focus widget.
            if (pTestWidget->focusPolicy() != Qt::NoFocus &&
                pTestWidget->isVisibleTo(pFocusWidget))
            {
                pFocusWidget = pTestWidget;
                break;
            }

            // Test next widget.
            pTestWidget = pTestWidget->nextInFocusChain();
        }

        // Set focus on the new widget.
        pFocusWidget->setFocus(Qt::TabFocusReason);
    }
}

// Manual handling of focus switching because qt doesn't allow the shortcut to be anything other than "Tab".
void rgMainWindow::HandleFocusPrevWidget()
{
    QWidget* pFocusWidget = QApplication::focusWidget();

    if (pFocusWidget != nullptr)
    {
        QWidget* pTestWidget = QApplication::focusWidget()->previousInFocusChain();

        // Step through focus widgets until one is found which accepts focus.
        while (pTestWidget != nullptr)
        {
            // Check that the widget accepts focus and is visible to the current focus widget.
            if (pTestWidget->focusPolicy() != Qt::NoFocus &&
                pTestWidget->isVisibleTo(pFocusWidget))
            {
                pFocusWidget = pTestWidget;
                break;
            }

            // Test previous widget.
            pTestWidget = pTestWidget->previousInFocusChain();
        }

        // Set focus on the new widget.
        pFocusWidget->setFocus(Qt::TabFocusReason);
    }
}

void rgMainWindow::HandleProjectBuildFailure()
{
    rgUtils::SetStatusTip(STR_STATUS_BAR_BUILD_FAILED, this);

    // Show the build failure message.
    this->statusBar()->showMessage(STR_STATUS_BAR_BUILD_FAILED);

    // Reset the view's state.
    ResetViewStateAfterBuild();
}

void rgMainWindow::HandleProjectBuildCanceled()
{
    rgUtils::SetStatusTip(STR_STATUS_BAR_BUILD_CANCELED, this);

    // Show the build cancellation message.
    this->statusBar()->showMessage(STR_STATUS_BAR_BUILD_CANCELED);

    // Reset the view's state.
    ResetViewStateAfterBuild();
}

void rgMainWindow::ResetViewStateAfterBuild()
{
    // Reset the build-in-progress flag.
    emit IsBuildInProgress(false);

    // Use the App State interface to reset the view state.
    m_pAppState->ResetViewStateAfterBuild();

    // Re-enable all menu items, since the build is over.
    m_pBuildProjectAction->setEnabled(true);
    m_pCancelBuildAction->setEnabled(false);
    m_pOpenProjectAction->setEnabled(true);
    m_pBackToHomeAction->setEnabled(true);
    m_pBuildSettingsAction->setEnabled(true);
    assert(m_pAppState != nullptr);
    if (m_pAppState != nullptr && m_pAppState->IsGraphics() &&
        m_pPipelineStateAction != nullptr)
    {
        m_pPipelineStateAction->setEnabled(true);
    }

    assert(m_pAppState != nullptr);
    if (m_pAppState != nullptr)
    {
        rgBuildView* pBuildView = m_pAppState->GetBuildView();
        if (pBuildView != nullptr)
        {
            rgMenu* pMenu = pBuildView->GetMenu();
            assert(pMenu != nullptr);
            if (pMenu != nullptr)
            {
                // Set the focus back to the source view.
                rgSourceCodeEditor* pSourceCodeEditor = pBuildView->GetEditorForFilepath(pMenu->GetSelectedFilePath());
                if (pSourceCodeEditor != nullptr && pSourceCodeEditor->isVisible())
                {
                    pSourceCodeEditor->setFocus();
                }

                // Make sure that none of the buttons are highlighted.
                pMenu->SetButtonsNoFocus();
            }
        }
    }
}

void rgMainWindow::HandleProjectBuildSuccess()
{
    rgUtils::SetStatusTip(STR_STATUS_BAR_BUILD_SUCCEEDED, this);

    this->statusBar()->showMessage(STR_STATUS_BAR_BUILD_SUCCEEDED);

    // Reset the view's state.
    ResetViewStateAfterBuild();
}

void rgMainWindow::HandleProjectBuildStarted()
{
    // Mark that a build is now in progress.
    emit IsBuildInProgress(true);

    // Update the status bar.
    this->statusBar()->removeWidget(m_pAppNotificationWidget);
    rgUtils::SetStatusTip(STR_STATUS_BAR_BUILD_STARTED, this);

    // Do not allow another build while a build is already in progress.
    m_pBuildProjectAction->setEnabled(false);
    m_pCancelBuildAction->setEnabled(true);

    assert(m_pAppState != nullptr);
    if (m_pAppState != nullptr)
    {
        m_pAppState->HandleProjectBuildStarted();
    }

    // Do not allow creating a project.
    m_pOpenProjectAction->setEnabled(false);

    // Do not allow going back to the home page.
    m_pBackToHomeAction->setEnabled(false);

    // Do not allow going to the build settings or pipeline state view.
    m_pBuildSettingsAction->setEnabled(false);
    assert(m_pAppState != nullptr);
    if (m_pAppState != nullptr && m_pAppState->IsGraphics() &&
        m_pPipelineStateAction != nullptr)
    {
        m_pPipelineStateAction->setEnabled(false);
    }

    assert(m_pAppState != nullptr);
    if (m_pAppState != nullptr)
    {
        rgBuildView* pBuildView = m_pAppState->GetBuildView();
        if (pBuildView != nullptr)
        {
            // Hide the disassembly view.
            pBuildView->ToggleDisassemblyViewVisibility(false);
        }
    }
}

void rgMainWindow::HandleUpdateAppNotificationMessage(const std::string& message, const std::string& tooltip)
{
    // First remove the previously added permanent widget.
    if (m_pAppNotificationWidget != nullptr)
    {
        this->statusBar()->removeWidget(m_pAppNotificationWidget);
    }

    // Add the new location only if it exists.
    if (!message.empty())
    {
        // Set the notification message.
        CreateAppNotificationMessageLabel(message, tooltip);

        // Insert the notification widget to the status bar.
        this->statusBar()->addPermanentWidget(m_pAppNotificationWidget, 0);

        // Show the notification label.
        m_pAppNotificationWidget->show();

        // Start the blinking timer.
        const int INITIAL_BLINKING_TIMEOUT_MS = 500;
        m_pAppNotificationBlinkingTimer->start(INITIAL_BLINKING_TIMEOUT_MS);
    }
}

void rgMainWindow::HandleAppNotificationMessageTimerFired()
{
    // Static counter to count the number of times that this timer has fired.
    static uint32_t count = 0;
    if (count++ % 2 == 0)
    {
        // Even counter: remove notification.
        this->statusBar()->removeWidget(m_pAppNotificationWidget);
    }
    else
    {
        // Odd count: insert the notification widget to the status bar.
        this->statusBar()->addPermanentWidget(m_pAppNotificationWidget, 0);

        // Show the notification label.
        m_pAppNotificationWidget->show();
    }

    // Stop condition.
    if (count % 8 == 0)
    {
        // Condition met: stop blinking.
        count = 0;
        m_pAppNotificationBlinkingTimer->stop();
    }
    else
    {
        // Continue blinking.
        const int BLINKING_INTERVAL_MS = 800;
        m_pAppNotificationBlinkingTimer->start(BLINKING_INTERVAL_MS);
    }
}

void rgMainWindow::CreateAppNotificationMessageLabel(const std::string& message, const std::string& tooltip)
{
    // Delete the existing widgets.
    if (m_pAppNotificationWidget != nullptr)
    {
        RG_SAFE_DELETE(m_pAppNotificationWidget);
    }

    assert(m_pAppState != nullptr);
    if (m_pAppState != nullptr)
    {
        // Create a widget to hold the labels.
        m_pAppNotificationWidget = new QWidget();
        m_pAppNotificationWidget->setToolTip(QString::fromStdString(tooltip));

        // Create icon and text labels.
        QIcon* pIcon = new QIcon(gs_ICON_RESOURCE_REMOVE_NOTIFICATION);
        QPixmap pixmap = pIcon->pixmap(QSize(24, 24));
        QLabel* pIconLabel = new QLabel(m_pAppNotificationWidget);
        pIconLabel->setPixmap(pixmap);
        QLabel* pTextLabel = new QLabel(QString::fromStdString(message), m_pAppNotificationWidget);
        pTextLabel->setStyleSheet("color: white");

        // Set label sizes.
        pTextLabel->setFixedSize(190, 18);
        pIconLabel->setFixedSize(24, 18);

        // Add labels to a widget and set layout.
        QHBoxLayout* pHLayout = new QHBoxLayout(m_pAppNotificationWidget);
        pHLayout->addWidget(pIconLabel);
        pHLayout->addWidget(pTextLabel);
        pHLayout->setContentsMargins(0, 0, 0, 0);
        m_pAppNotificationWidget->setLayout(pHLayout);
        m_pAppNotificationWidget->setContentsMargins(0, 0, 25, 0);
    }
}

void rgMainWindow::HandleStatusBarTextChanged(const std::string& statusBarText, int timeoutMs)
{
    this->statusBar()->showMessage(statusBarText.c_str(), timeoutMs);
}

void rgMainWindow::SetCursor()
{
    // Set the cursor to pointing hand cursor.
    menuBar()->setCursor(Qt::PointingHandCursor);
}

void rgMainWindow::HandleGoToLineEvent()
{
    assert(m_pAppState != nullptr);
    if (m_pAppState != nullptr)
    {
        rgBuildView* pBuildView = m_pAppState->GetBuildView();
        if (pBuildView != nullptr)
        {
            rgMenu* pMenu = pBuildView->GetMenu();
            assert(pMenu != nullptr);
            if (pMenu != nullptr)
            {
                // Get the max number of lines.
                rgSourceCodeEditor* pSourceCodeEditor = pBuildView->GetEditorForFilepath(pMenu->GetSelectedFilePath());
                if (pSourceCodeEditor != nullptr && pSourceCodeEditor->isVisible())
                {
                    int maxLineNumber = pSourceCodeEditor->document()->lineCount();

                    // Create a modal Go To line dialog.
                    rgGoToLineDialog* pGoToLineDialog = new rgGoToLineDialog(maxLineNumber, this);
                    pGoToLineDialog->setModal(true);
                    pGoToLineDialog->setWindowTitle(STR_GO_TO_LINE_DIALOG_TITLE);

                    // Register the dialog with the scaling manager.
                    ScalingManager::Get().RegisterObject(pGoToLineDialog);

                    // Center the dialog on the view (registering with the scaling manager
                    // shifts it out of the center so we need to manually center it).
                    rgUtils::CenterOnWidget(pGoToLineDialog, this);

                    // Execute the dialog and get the result.
                    rgGoToLineDialog::rgGoToLineDialogResult result;
                    result = static_cast<rgGoToLineDialog::rgGoToLineDialogResult>(pGoToLineDialog->exec());

                    switch (result)
                    {
                    case rgGoToLineDialog::Ok:
                    {
                        // Go to the indicated line number.
                        int lineNumber = pGoToLineDialog->GetLineNumber();

                        // Scroll the editor to the indicated line.
                        pSourceCodeEditor->ScrollToLine(lineNumber);

                        // Set the highlighted line.
                        QList<int> lineNumbers;
                        lineNumbers << lineNumber;
                        pSourceCodeEditor->SetHighlightedLines(lineNumbers);

                        // Move the cursor as well.
                        QTextCursor cursor(pSourceCodeEditor->document()->findBlockByLineNumber(lineNumber - 1));
                        pSourceCodeEditor->setTextCursor(cursor);
                        break;
                    }
                    case rgGoToLineDialog::Cancel:
                    {
                        // Dialog rejected.
                        break;
                    }
                    default:
                    {
                        // Shouldn't get here.
                        assert(false);
                    }
                    }

                    // Free the memory.
                    delete pGoToLineDialog;
                }
            }
        }
    }
}

void rgMainWindow::HandleSaveSettingsButtonClicked()
{
    assert(m_pSettingsTab != nullptr);
    if (m_pSettingsTab != nullptr)
    {
        m_pSettingsTab->SavePendingChanges();
    }
}

void rgMainWindow::HandleTabBarTabChanged(bool saveChanges)
{
    if (saveChanges)
    {
        HandleSaveSettingsButtonClicked();
    }
    else
    {
        assert(m_pSettingsTab != nullptr);
        if (m_pSettingsTab != nullptr)
        {
            m_pSettingsTab->RevertPendingChanges();
        }
    }
}

void rgMainWindow::HandleStatusBarMessageChange(const QString& msg)
{
    // If a build is in progress, the status bar should not be empty,
    // but rather notify the user about the fact that a build is in progress.
    if (msg.isEmpty())
    {
        // Use the rgBuildView to check if there's a build in progress.
        bool isBuildInProgress = false;

        assert(m_pAppState != nullptr);
        if (m_pAppState != nullptr)
        {
            rgBuildView* pBuildView = m_pAppState->GetBuildView();
            if (pBuildView != nullptr)
            {
                isBuildInProgress = pBuildView->IsBuildInProgress();
            }
        }

        this->statusBar()->showMessage(isBuildInProgress ? STR_STATUS_BAR_BUILD_STARTED : msg);
    }
}

void rgMainWindow::HandleMainTabWidgetTabChanged(int currentIndex)
{
    // If the user selected "Settings" tab, change the Ctrl+S shortcut
    // to save the settings.
    if (currentIndex == 0)
    {
        // Switch the save shortcut.
        SwitchSaveShortcut(SaveActionType::SaveFile);

        // Update the file menu save action visibility.
        OnCurrentEditorModificationStateChanged(m_saveFileActionActive);
    }
    else if (currentIndex == 1)
    {
        // Switch the save shortcut.
        SwitchSaveShortcut(SaveActionType::SaveSettings);

        // Update the file menu save action visibility.
        OnCurrentEditorModificationStateChanged(m_saveSettingsActionActive);
    }

    // Hide the API list widget.
    m_pStatusBar->SetApiListVisibility(false);
}

void rgMainWindow::HandleEditModeChanged(EditMode mode)
{
    switch (mode)
    {
    case (EditMode::SourceCode):
    {
        // Switch the save shortcut.
        SwitchSaveShortcut(SaveActionType::SaveFile);

        break;
    }
    case (EditMode::BuildSettings):
    {
        // Switch the save shortcut.
        SwitchSaveShortcut(SaveActionType::SaveSettings);

        // Disable the edit menu when viewing the build settings.
        EnableEditMenu(false);

        break;
    }
    case (EditMode::PipelineSettings):
    {
        // Switch the save shortcut.
        SwitchSaveShortcut(SaveActionType::SaveSettings);

        // Enable the find functionality.
        m_pFindAction->setEnabled(true);

        // Disable the Go-to-line functionality.
        m_pGoToLineAction->setEnabled(false);

        break;
    }
    default:
        // We shouldn't get here.
        assert(false);
        break;
    }
}

void rgMainWindow::SwitchSaveShortcut(SaveActionType saveActionType)
{
    switch (saveActionType)
    {
    case (SaveActionType::SaveFile):
    {
        if (m_pSaveAction != nullptr && m_pSaveAction->text().compare(STR_MENU_BAR_SAVE_SETTINGS) == 0)
        {
            // Change the action text and tooltip and restore the "enabled" state.
            m_pSaveAction->setText(tr(STR_MENU_BAR_SAVE_FILE));
            m_pSaveAction->setStatusTip(tr(STR_MENU_BAR_SAVE_FILE_TOOLTIP));
            m_pSaveAction->setEnabled(m_saveFileActionActive);
        }
        break;
    }
    case (SaveActionType::SaveSettings):
    {
        if (m_pSaveAction != nullptr && m_pSaveAction->text().compare(STR_MENU_BAR_SAVE_FILE) == 0)
        {
            // Change the action text and tooltip and restore the "enabled" state.
            m_pSaveAction->setText(tr(STR_MENU_BAR_SAVE_SETTINGS));
            m_pSaveAction->setStatusTip(tr(STR_MENU_BAR_SAVE_SETTINGS_TOOLTIP));
            m_pSaveAction->setEnabled(m_saveSettingsActionActive);
        }
        break;
    }
    default:
        // We shouldn't get here.
        assert(false);
        break;
    }
}

void rgMainWindow::ResetActionsState()
{
    assert(m_pAppState != nullptr);
    if ((m_pAppState != nullptr))
    {
        // Block build-related and source-file-related actions.
        EnableBuildMenu(false);
        m_pBuildSettingsAction->setEnabled(false);
        if (m_pAppState->IsGraphics() && m_pPipelineStateAction != nullptr)
        {
            m_pPipelineStateAction->setEnabled(false);
        }
        m_pCancelBuildAction->setEnabled(false);
        EnableEditMenu(false);
    }
}

void rgMainWindow::EnableBuildViewActions()
{
    // Enable the build menu options after the build view has been opened.
    rgBuildView* pBuildView = m_pAppState->GetBuildView();
    assert(pBuildView != nullptr);
    if (pBuildView != nullptr)
    {
        bool isProjectEmpty = pBuildView->GetMenu()->IsEmpty();
        EnableBuildMenu(!isProjectEmpty);
        m_pBuildSettingsAction->setEnabled(true);
        assert(m_pAppState != nullptr);
        if (m_pAppState != nullptr && m_pPipelineStateAction != nullptr)
        {
            m_pPipelineStateAction->setEnabled(true);
        }
        m_pCancelBuildAction->setEnabled(false);

        // Set container switch size action.
        bool isConnected = connect(this, &rgMainWindow::SwitchContainerSize, pBuildView, &rgBuildView::HandleSwitchContainerSize);
        assert(isConnected);
    }
}

void rgMainWindow::SetApplicationStylesheet()
{
    // Set application-wide stylesheet.
    std::vector<std::string> stylesheetFileNames;
    m_pAppState->GetApplicationStylesheet(stylesheetFileNames);
    rgUtils::LoadAndApplyStyle(stylesheetFileNames, qApp);
}

void rgMainWindow::ApplyMainWindowStylesheet()
{
    // Apply main window stylesheet.
    assert(m_pAppState != nullptr);
    if (m_pAppState != nullptr)
    {
        std::vector<std::string> stylesheetFileNames;
        m_pAppState->GetMainWindowStylesheet(stylesheetFileNames);
        rgUtils::LoadAndApplyStyle(stylesheetFileNames, this);
    }
}

void rgMainWindow::CreateCustomStatusBar()
{
    assert(m_pFactory != nullptr);
    if (m_pFactory != nullptr)
    {
        // Delete any existing custom status bar first.
        if (m_pStatusBar != nullptr)
        {
            delete m_pStatusBar;
            m_pStatusBar = nullptr;
        }

        m_pStatusBar = m_pFactory->CreateStatusBar(statusBar(), this);
        assert(m_pStatusBar != nullptr);
        if (m_pStatusBar != nullptr)
        {
            // Set status bar dimensions.
            QSize size;
            size.setHeight(s_CUSTOM_STATUS_BAR_HEIGHT);
            size.setWidth(QWIDGETSIZE_MAX);
            statusBar()->setFixedSize(size);
            m_pStatusBar->setFixedSize(size);

            // Register the status bar with the scaling manager.
            ScalingManager& scalingManager = ScalingManager::Get();
            scalingManager.RegisterObject(statusBar());
            scalingManager.RegisterObject(m_pStatusBar);
            statusBar()->addWidget(m_pStatusBar);

            // Disable the resize grip.
            statusBar()->setSizeGripEnabled(false);

            // Show the custom widget.
            m_pStatusBar->show();

            // Connect the custom status bar's change API mode signal.
            bool isConnected = connect(m_pStatusBar, &rgStatusBar::ChangeAPIModeSignal, this, &rgMainWindow::HandleChangeAPIMode);
            assert(isConnected);

            // Connect the custom status bar's save pending settings changes signal.
            isConnected = connect(m_pStatusBar, &rgStatusBar::SavePendingChanges, this, &rgMainWindow::HandleSavePendingChanges);
            assert(isConnected);
        }
    }
}

bool rgMainWindow::HandleSavePendingChanges()
{
    bool isNotCancelled = false;
    assert(m_pSettingsTab != nullptr);
    if (m_pSettingsTab != nullptr)
    {
        isNotCancelled = m_pSettingsTab->PromptToSavePendingChanges();
    }

    return isNotCancelled;
}

void rgMainWindow::HandleChangeAPIMode(rgProjectAPI switchToApi)
{
    ChangeApiMode(switchToApi);
}

bool rgMainWindow::eventFilter(QObject* pObject, QEvent* pEvent)
{
    if (pEvent != nullptr)
    {
        if (pEvent->type() == QEvent::MouseButtonPress)
        {
            // Hide the API list widget when the user clicks somewhere else.
            m_pStatusBar->SetApiListVisibility(false);

            return true;
        }
        else if (pEvent->type() == QEvent::KeyPress)
        {
            QKeyEvent* pKeyEvent = static_cast<QKeyEvent*>(pEvent);
            const int key = pKeyEvent->key();
            Qt::KeyboardModifiers keyboardModifiers = QApplication::keyboardModifiers();
            if ((pKeyEvent->key() != Qt::Key_Up) && (pKeyEvent->key() != Qt::Key_Down))
            {
                // Hide the API list widget when the user presses a key.
                m_pStatusBar->SetApiListVisibility(false);

                if ((keyboardModifiers & Qt::ControlModifier) && (pKeyEvent->key() == Qt::Key_R))
                {
                    emit SwitchContainerSize();
                }
                return true;
            }
            else
            {
                return false;
            }
        }
        else
        {
            return QObject::eventFilter(pObject, pEvent);
        }
    }
    else
    {
        // Continue default processing.
        return QObject::eventFilter(pObject, pEvent);
    }
}


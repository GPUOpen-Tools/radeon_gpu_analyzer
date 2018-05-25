// C++.
#include <cassert>
#include <memory>
#include <string>
#include <sstream>

// Qt.
#include <QButtonGroup>
#include <QFile>
#include <QPushButton>
#include <QTextStream>
#include <QWidget>
#include <QMimeData>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDesktopWidget>
#include <QDesktopServices>

// Infra.
#include <QtCommon/Scaling/ScalingManager.h>

// Local.
#include <RadeonGPUAnalyzerGUI/include/qt/rgOpenCLBuildSettingsView.h>
#include <RadeonGPUAnalyzerGUI/include/qt/rgGlobalSettingsView.h>
#include <RadeonGPUAnalyzerGUI/include/qt/rgGoToLineDialog.h>
#include <RadeonGPUAnalyzerGUI/include/rgConfigManager.h>
#include <RadeonGPUAnalyzerGUI/include/qt/rgSourceCodeEditor.h>
#include <RadeonGPUAnalyzerGUI/include/qt/rgFileMenu.h>
#include <RadeonGPUAnalyzerGUI/include/qt/rgBuildView.h>
#include <RadeonGPUAnalyzerGUI/include/qt/rgMainWindow.h>
#include <RadeonGPUAnalyzerGUI/include/rgFactory.h>
#include <RadeonGPUAnalyzerGUI/include/rgStringConstants.h>
#include <RadeonGPUAnalyzerGUI/include/rgUtils.h>
#include <RadeonGPUAnalyzerGUI/include/rgDataTypes.h>
#include <RadeonGPUAnalyzerGUI/include/rgDefinitions.h>
#include <RadeonGPUAnalyzerGUI/include/qt/rgIsaDisassemblyView.h>
#include <RadeonGPUAnalyzerGUI/include/qt/rgViewContainer.h>
#include <RadeonGPUAnalyzerGUI/include/qt/rgAboutDialog.h>
#include <RadeonGPUAnalyzerGUI/include/qt/rgSettingsButtonsView.h>

rgMainWindow::rgMainWindow(QWidget* pParent)
    : QMainWindow(pParent)
{
    // Setup the UI.
    ui.setupUi(this);

    // Set the window icon to the product logo.
    setWindowIcon(QIcon(gs_ICON_RESOURCE_RGA_LOGO));

    // Set the background color to white.
    QPalette pal = palette();
    pal.setColor(QPalette::Background, Qt::white);
    this->setAutoFillBackground(true);
    this->setPalette(pal);

    // Initialize the scaling manager.
    ScalingManager& scalingManager = ScalingManager::Get();

    // Create the file menu.
    CreateFileMenu();

    // Create the Edit menu.
    CreateEditMenu();

    // Create the Build menu.
    CreateBuildMenu();

    // Create the help menu.
    CreateHelpMenu();

    // Create the context menu for recent files
    CreateContextMenu();

    // Reset the state of the action to the default state.
    ResetActionsState();

    // Create the global build settings view.
    CreateGlobalBuildSettingsView();

    // Connect the signals.
    ConnectSignals();

    // Connect settings tab list widget signals.
    ConnectSettingsListWidgetSignals();

    // Populate the welcome page's Recent Projects list.
    PopulateRecentProjectsList();

    // Update text components with appropriate string constants.
    ApplyStringConstants();

    // Apply style from stylesheet.
    rgUtils::LoadAndApplyStyle(STR_MAIN_WINDOW_STYLESHEET_FILE, this);

    // Start on the home page.
    SwitchToView(ui.homePage);

    // Enable drops so the main window can handle opening dropped source files.
    setAcceptDrops(true);

    // Set the mouse cursor to the pointing hand cursor for various widgets.
    SetCursor();

    // Set the parent widget for the tab bar so the dialogs get shown in the correct location.
    ui.mainTabWidget->GetTabBar()->SetParentWidget(this);

    // Set the focus to the tab bar.
    ui.mainTabWidget->setFocus();

    // Install an event filter to handle up/down arrow keys on the recent files list widget.
    qApp->installEventFilter(this);
}

void rgMainWindow::ApplyStringConstants()
{
    // Set label/button text.
    ui.newFilePushButton->setText(STR_MENU_BAR_CREATE_NEW_FILE);
    ui.existingFilePushButton->setText(STR_MENU_BAR_OPEN_EXISTING_FILE);

    // Set tooltips and status tips.
    rgUtils::SetToolAndStatusTip(STR_MENU_BAR_OPEN_EXISTING_FILE_TOOLTIP, ui.existingFilePushButton);
    rgUtils::SetToolAndStatusTip(STR_MENU_BAR_CREATE_NEW_FILE_TOOLTIP, ui.newFilePushButton);
    rgUtils::SetToolAndStatusTip(STR_MENU_BAR_OPEN_PROJECT_TOOLTIP, ui.recentProjectsOtherPushButton);
    rgUtils::SetToolAndStatusTip(STR_MENU_BAR_HELP_ABOUT_TOOLTIP, ui.aboutRGAPushButton);
    rgUtils::SetToolAndStatusTip(STR_MENU_BAR_HELP_MANUAL_TOOLTIP, ui.helpManualPushButton);
    rgUtils::SetToolAndStatusTip(STR_MENU_BAR_HELP_GETTING_STARTED_GUIDE_TOOLTIP, ui.gettingStartedPushButton);

    // Set tooltips for settings list widget items.
    assert(ui.settingsListWidget->item(0) != nullptr);
    assert(ui.settingsListWidget->item(1) != nullptr);
    if (ui.settingsListWidget->item(0) != nullptr)
    {
        ui.settingsListWidget->item(0)->setToolTip(ui.settingsListWidget->item(0)->text());
    }

    if (ui.settingsListWidget->item(1) != nullptr)
    {
        ui.settingsListWidget->item(1)->setToolTip(ui.settingsListWidget->item(1)->text());
    }
}

void rgMainWindow::ConnectSignals()
{
    // Create New File button.
    bool isConnected = connect(this->ui.newFilePushButton, &QPushButton::pressed, this, &rgMainWindow::HandleCreateNewFileEvent);
    assert(isConnected);

    // Open Existing File button.
    isConnected = connect(this->ui.existingFilePushButton, &QPushButton::pressed, this, &rgMainWindow::HandleOpenExistingFileEvent);
    assert(isConnected);

    // Open recent project with the "Other..." button.
    isConnected = connect(this->ui.recentProjectsOtherPushButton, &QPushButton::pressed, this, &rgMainWindow::HandleOpenProjectFileEvent);
    assert(isConnected);

    // About RGA.
    isConnected = connect(this->ui.aboutRGAPushButton, &QPushButton::pressed, this, &rgMainWindow::HandleAboutEvent);
    assert(isConnected);

    // Getting started guide.
    isConnected = connect(this->ui.gettingStartedPushButton, &QPushButton::pressed, this, &rgMainWindow::HandleGettingStartedGuideEvent);
    assert(isConnected);

    // Help manual.
    isConnected = connect(this->ui.helpManualPushButton, &QPushButton::pressed, this, &rgMainWindow::HandleHelpManual);
    assert(isConnected);

    // Global settings view has pending changes.
    isConnected = connect(m_pGlobalSettingsView, &rgGlobalSettingsView::PendingChangesStateChanged, this, &rgMainWindow::HandleGlobalPendingChangesStateChanged);
    assert(isConnected);

    // Build settings view has pending changes.
    isConnected = connect(static_cast<rgOpenCLBuildSettingsView*>(m_pBuildSettingsView), &rgOpenCLBuildSettingsView::PendingChangesStateChanged, this, &rgMainWindow::HandleBuildSettingsPendingChangesStateChanged);
    assert(isConnected);

    // "Save" button.
    isConnected = connect(m_pSettingsButtonsView, &rgSettingsButtonsView::SaveSettingsButtonClickedSignal, this, &rgMainWindow::HandleSaveSettingsButtonClicked);
    assert(isConnected);

    // "Restore defaults" button.
    isConnected = connect(m_pSettingsButtonsView, &rgSettingsButtonsView::RestoreDefaultSettingsButtonClickedSignal, this, &rgMainWindow::HandleRestoreDefaultsSettingsClicked);
    assert(isConnected);

    // Tab changed signal to handle save shortcut updates.
    isConnected = connect(ui.mainTabWidget, &rgMainWindowTabWidget::currentChanged, this, &rgMainWindow::HandleMainTabWidgetTabChanged);
    assert(isConnected);

    // Connect the tab bar's save changes signal.
    isConnected = connect(ui.mainTabWidget->GetTabBar(), &rgMainWindowTabBar::SaveBuildSettingsChangesSignal, this, &rgMainWindow::HandleTabBarTabChanged);
    assert(isConnected);

    // Connect the Save QAction to the main window's handler.
    isConnected = connect(m_pSaveAction, &QAction::triggered, this, &rgMainWindow::HandleSaveFileEvent);
    assert(isConnected);

    // Connect the status bar's message change signal.
    isConnected = connect(this->statusBar(), &QStatusBar::messageChanged, this, &rgMainWindow::HandleStatusBarMessageChange);
    assert(isConnected);
}

void rgMainWindow::CreateGlobalBuildSettingsView()
{
    rgConfigManager& configManager = rgConfigManager::Instance();
    rgProjectAPI currentAPI = configManager.GetCurrentAPI();

    std::shared_ptr<rgBuildSettings> pBuildSettings = configManager.GetUserGlobalBuildSettings(currentAPI);
    if (pBuildSettings != nullptr)
    {
        // Get settings tab container widget.
        QWidget* pTabContainer = ui.mainTabWidget->widget(1);
        bool isTabContainerValid = (pTabContainer != nullptr);
        assert(isTabContainerValid);
        if (isTabContainerValid)
        {
            // Set the background color.
            QPalette palette;
            palette.setColor(QPalette::Background, Qt::GlobalColor::transparent);

            // Create the widget for the scroll area.
            m_pScrollAreaWidgetContents = new QFrame(pTabContainer);
            QRect rect = geometry();
            rect.setHeight(rect.height() + 150 * ScalingManager::Get().GetScaleFactor());
            m_pScrollAreaWidgetContents->setGeometry(rect);
            m_pScrollAreaWidgetContents->setLayout(new QVBoxLayout);

            // Create the global settings view.
            std::shared_ptr<rgGlobalSettings> pGlobalSettings = configManager.GetGlobalConfig();
            m_pGlobalSettingsView = new rgGlobalSettingsView(this, pGlobalSettings);

            // Create the build settings view.
            std::shared_ptr<rgFactory> pFactory = rgFactory::CreateFactory(currentAPI);
            m_pBuildSettingsView = pFactory->CreateBuildSettingsView(m_pScrollAreaWidgetContents, pBuildSettings, true);
            rgOpenCLBuildSettingsView* pBuildSettingsView = static_cast<rgOpenCLBuildSettingsView*>(m_pBuildSettingsView);

            // Set the build settings geometry for various screen resolution and dpi settings.
            SetBuildSettingsGeometry();

            // Create the settings buttons view.
            m_pSettingsButtonsView = new rgSettingsButtonsView(m_pScrollAreaWidgetContents);

            // Register the newly-created widget with the scaling manager.
            ScalingManager::Get().RegisterObject(m_pGlobalSettingsView);
            ScalingManager::Get().RegisterObject(m_pBuildSettingsView);
            ScalingManager::Get().RegisterObject(m_pSettingsButtonsView);

            // Add various widgets to this tab.
            m_pScrollAreaWidgetContents->layout()->addWidget(m_pGlobalSettingsView);
            m_pScrollAreaWidgetContents->layout()->addWidget(m_pBuildSettingsView);
            m_pScrollAreaWidgetContents->layout()->addWidget(m_pSettingsButtonsView);

            // Add a vertical spacer at the bottom to push all the widgets up.
            m_pScrollAreaWidgetContents->layout()->addItem(new QSpacerItem(10, 10, QSizePolicy::Fixed, QSizePolicy::Expanding));

            // Set the size constraint.
            m_pScrollAreaWidgetContents->layout()->setSizeConstraint(QLayout::SetMinimumSize);

            // Set various properties for the container widget.
            m_pScrollAreaWidgetContents->layout()->setSpacing(0);
            m_pScrollAreaWidgetContents->layout()->setContentsMargins(20, 0, 0, 0);
            m_pScrollAreaWidgetContents->setPalette(palette);
            m_pScrollAreaWidgetContents->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

            // Create a scroll area.
            QVBoxLayout* pVBoxLayout = new QVBoxLayout(this);
            QScrollArea* pScrollArea = new QScrollArea;

            // Set various properties for the scroll area.
            pScrollArea->setPalette(palette);
            pScrollArea->setWidgetResizable(true);
            pScrollArea->setFrameShape(QFrame::NoFrame);
            pScrollArea->setAlignment(Qt::AlignTop);
            pScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
            pScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
            pScrollArea->setGeometry(geometry());

            // Add widgets.
            pVBoxLayout->addWidget(pScrollArea);
            pScrollArea->setWidget(m_pScrollAreaWidgetContents);
            ui.rightFrameLayout->addWidget(pScrollArea);

            // Start out with the build settings hidden.
            m_pBuildSettingsView->hide();

            // Set the settings list widget's current row to "Global".
            ui.settingsListWidget->setCurrentRow(SettingsListWidgetEntries::Global);

            // Set the focus to the settings list widget.
            ui.settingsListWidget->setFocus();
        }
    }
}

void rgMainWindow::CreateBuildView()
{
    // Need to create the build view only once.
    if (m_pBuildView == nullptr)
    {
        rgConfigManager& configManager = rgConfigManager::Instance();
        rgProjectAPI currentAPI = configManager.GetCurrentAPI();

        // Create the build view.
        m_pBuildView = new rgBuildView(currentAPI, this);
        ui.buildPage->layout()->addWidget(m_pBuildView);

        // Register the object with the scaling manager.
        ScalingManager::Get().RegisterObject(m_pBuildView);

        // Connect all of the signals required to support the BuildView.
        ConnectBuildViewSignals();

        // Adjust the actions to the build view's initial state.
        EnableBuildViewActions();
    }
}

void rgMainWindow::ConnectBuildViewSignals()
{
    // Verify that the build view exists before using it.
    assert(m_pBuildView);

    // Connect the file menu's "Build Settings" button to the main window's handler.
    bool isConnected = connect(m_pBuildView->GetFileMenu(), &rgFileMenu::BuildSettingsButtonClicked, this, &rgMainWindow::HandleBuildSettingsEvent);
    assert(isConnected);

    // Editor text change handler.
    isConnected = connect(m_pBuildView, &rgBuildView::CurrentEditorModificationStateChanged, this, &rgMainWindow::OnCurrentEditorModificationStateChanged);
    assert(isConnected);

    // Connect the rgBuildView default menu item's Create button.
    isConnected = connect(m_pBuildView, &rgBuildView::CreateFileButtonClicked, this, &rgMainWindow::HandleCreateNewFileEvent);
    assert(isConnected);

    // Connect the rgBuildView default menu item's Open button.
    isConnected = connect(m_pBuildView, &rgBuildView::OpenFileButtonClicked, this, &rgMainWindow::HandleOpenExistingFileEvent);
    assert(isConnected);

    isConnected = connect(m_pBuildView, &rgBuildView::ProjectFileCountChanged, this, &rgMainWindow::HandleProjectFileCountChanged);
    assert(isConnected);

    isConnected = connect(m_pBuildView, &rgBuildView::ProjectLoaded, this, &rgMainWindow::HandleProjectLoaded);
    assert(isConnected);

    isConnected = connect(m_pBuildView, &rgBuildView::CurrentFileModified, this, &rgMainWindow::HandleCurrentFileModifiedOutsideEnv);
    assert(isConnected);

    isConnected = connect(m_pBuildView, &rgBuildView::ProjectBuildSuccess, this, &rgMainWindow::HandleProjectBuildSuccess);
    assert(isConnected);

    isConnected = connect(m_pBuildView, &rgBuildView::ProjectBuildStarted, this, &rgMainWindow::HandleProjectBuildStarted);
    assert(isConnected);

    isConnected = connect(m_pBuildView, &rgBuildView::ProjectBuildFailure, this, &rgMainWindow::HandleProjectBuildFailure);
    assert(isConnected);

    isConnected = connect(m_pBuildView, &rgBuildView::ProjectBuildCanceled, this, &rgMainWindow::HandleProjectBuildCanceled);
    assert(isConnected);

    // Connect the rgBuildView's status bar update signal.
    isConnected = connect(m_pBuildView, &rgBuildView::SetStatusBarText, this, &rgMainWindow::HandleStatusBarTextChanged);
    assert(isConnected);

    // Connect the file menu with the build start event.
    isConnected = connect(m_pBuildView, &rgBuildView::ProjectBuildStarted, m_pBuildView->GetFileMenu(), &rgFileMenu::HandleBuildStarted);
    assert(isConnected);

    // Connect the file menu with the build failure event.
    isConnected = connect(m_pBuildView, &rgBuildView::ProjectBuildFailure, m_pBuildView->GetFileMenu(), &rgFileMenu::HandleBuildEnded);
    assert(isConnected);

    // Connect the file menu with the build success event.
    isConnected = connect(m_pBuildView, &rgBuildView::ProjectBuildSuccess, m_pBuildView->GetFileMenu(), &rgFileMenu::HandleBuildEnded);
    assert(isConnected);

    // Connect the file menu with the build canceled event.
    isConnected = connect(m_pBuildView, &rgBuildView::ProjectBuildCanceled, m_pBuildView->GetFileMenu(), &rgFileMenu::HandleBuildEnded);
    assert(isConnected);

    // Connect the main window's Find event with the rgBuildView.
    isConnected = connect(this, &rgMainWindow::FindTriggered, m_pBuildView, &rgBuildView::HandleFindTriggered);
    assert(isConnected);

    // Connect the rgMainWindow to the rgBuildView to update the "project is building" flag.
    isConnected = connect(this, &rgMainWindow::IsBuildInProgress, m_pBuildView, &rgBuildView::HandleIsBuildInProgressChanged);
    assert(isConnected);

    // Connect the source file change event with the handler.
    isConnected = connect(m_pBuildView->GetFileMenu(), &rgFileMenu::SelectedFileChanged, this, &rgMainWindow::HandleSelectedFileChanged);
    assert(isConnected);

    // Connect the use default project name check box change event with the handler.
    isConnected = connect(m_pBuildView, &rgBuildView::UpdateUseDefaultProjectNameCheckbox, m_pGlobalSettingsView, &rgGlobalSettingsView::HandleDefaultProjectNameCheckboxUpdate);
    assert(isConnected);

    // Connect the edit mode changed signal.
    isConnected = connect(m_pBuildView, &rgBuildView::EditModeChanged, this, &rgMainWindow::HandleEditModeChanged);
}

void rgMainWindow::CreateFileMenuActions()
{
    // Create New File action.
    m_pNewFileAction = new QAction(tr(STR_MENU_BAR_CREATE_NEW_FILE), this);
    m_pNewFileAction->setShortcuts(QKeySequence::New);
    m_pNewFileAction->setStatusTip(tr(STR_MENU_BAR_CREATE_NEW_FILE_TOOLTIP));
    connect(m_pNewFileAction, &QAction::triggered, this, &rgMainWindow::HandleCreateNewFileEvent);

    // Open a file.
    m_pOpenFileAction = new QAction(tr(STR_MENU_BAR_OPEN_EXISTING_FILE), this);
    m_pOpenFileAction->setShortcuts(QKeySequence::Open);
    m_pOpenFileAction->setStatusTip(tr(STR_MENU_BAR_OPEN_EXISTING_FILE_TOOLTIP));
    connect(m_pOpenFileAction, &QAction::triggered, this, &rgMainWindow::HandleOpenExistingFileEvent);

    // Open a project.
    m_pOpenProjectAction = new QAction(tr(STR_MENU_BAR_OPEN_PROJECT), this);
    m_pOpenProjectAction->setShortcut(QKeySequence(gs_ACTION_HOTKEY_OPEN_PROJECT));
    m_pOpenProjectAction->setStatusTip(tr(STR_MENU_BAR_OPEN_PROJECT_TOOLTIP));
    connect(m_pOpenProjectAction, &QAction::triggered, this, &rgMainWindow::HandleOpenProjectFileEvent);

    // Save a file.
    m_pSaveAction = new QAction(tr(STR_MENU_BAR_SAVE_FILE), this);
    m_pSaveAction->setShortcuts(QKeySequence::Save);
    m_pSaveAction->setStatusTip(tr(STR_MENU_BAR_SAVE_FILE_TOOLTIP));
    connect(m_pSaveAction, &QAction::triggered, this, &rgMainWindow::HandleSaveFileEvent);

    // Back to home.
    m_pBackToHomeAction = new QAction(tr(STR_MENU_BAR_BACK_TO_HOME), this);
    m_pBackToHomeAction->setShortcut(QKeySequence(gs_ACTION_HOTKEY_BACK_TO_HOME));
    m_pBackToHomeAction->setStatusTip(tr(STR_MENU_BAR_BACK_TO_HOME_TOOLTIP));
    connect(m_pBackToHomeAction, &QAction::triggered, this, &rgMainWindow::HandleBackToHomeEvent);

    // Exit the project.
    m_pExitAction = new QAction(tr(STR_MENU_BAR_EXIT), this);
    m_pExitAction->setShortcut(QKeySequence(gs_ACTION_HOTKEY_EXIT));
    m_pExitAction->setStatusTip(tr(STR_MENU_BAR_EXIT_TOOLTIP));
    connect(m_pExitAction, &QAction::triggered, this, &rgMainWindow::HandleExitEvent);

    // Start with some actions disabled.
    m_pSaveAction->setDisabled(true);
}

void rgMainWindow::CreateFileMenu()
{
    // Create the actions to be used by the menus.
    CreateFileMenuActions();

    m_pMenuBar = menuBar()->addMenu(tr(STR_MENU_BAR_FILE));
    m_pMenuBar->addAction(m_pNewFileAction);
    m_pMenuBar->addAction(m_pOpenFileAction);
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
    addAction(m_pNewFileAction);
    addAction(m_pOpenFileAction);
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

void rgMainWindow::CreateContextMenu()
{
    m_pOpenRecentAction = new QAction(STR_MAIN_WINDOW_LOAD_PROJECT, nullptr);
    m_menu.addAction(m_pOpenRecentAction);

    // Add a separator between the current menu items.
    m_menu.addSeparator();

    m_pOpenContainingFolderAction = new QAction(STR_FILE_CONTEXT_MENU_OPEN_CONTAINING_FOLDER, nullptr);
    m_menu.addAction(m_pOpenContainingFolderAction);
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
    connect(m_pBuildSettingsAction, SIGNAL(triggered()), this, SLOT(HandleBuildSettingsEvent()));

    // Cancel the current build.
    m_pCancelBuildAction = new QAction(tr(STR_MENU_CANCEL_BUILD), this);
    m_pCancelBuildAction->setShortcut(QKeySequence(gs_ACTION_HOTKEY_BUILD_CANCEL));
    m_pCancelBuildAction->setStatusTip(tr(STR_MENU_BAR_BUILD_CANCEL_TOOLTIP));
    connect(m_pCancelBuildAction, SIGNAL(triggered()), this, SLOT(HandleCancelBuildEvent()));
}

void rgMainWindow::CreateBuildMenu()
{
    CreateBuildMenuActions();

    m_pMenuBar = menuBar()->addMenu(tr(STR_MENU_BAR_BUILD));
    m_pMenuBar->addAction(m_pBuildProjectAction);
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

    // Destroy the rgBuildView instance.
    RG_SAFE_DELETE(m_pBuildView);
}

void rgMainWindow::EnableBuildMenu(bool isEnabled)
{
    // Toggle the actions associated with the build menu items.
    m_pBuildProjectAction->setEnabled(isEnabled);
    m_pBuildSettingsAction->setEnabled(isEnabled);

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

    // Create the rgBuildView.
    CreateBuildView();

    assert(m_pBuildView != nullptr);
    if (m_pBuildView != nullptr)
    {
        // Attempt to load the user's selected project.
        isProjectLoaded = m_pBuildView->LoadProjectFile(projectFilePath);
        assert(isProjectLoaded);

        // If the project was loaded correctly, create and populate the rgBuildView with the source files.
        if (isProjectLoaded)
        {
            // Populate the rgBuildView with the loaded project.
            bool isPopulated = m_pBuildView->PopulateBuildView();
            if (isPopulated)
            {
                // Show the build view as the central widget.
                SwitchToView(ui.buildPage);

                // Get the directory where the project file lives.
                std::string projectDirectory;
                bool isOk = rgUtils::ExtractFileDirectory(projectFilePath, projectDirectory);
                assert(isOk);
                if (isOk)
                {
                    // Try to load existing build output within the project directory.
                    bool isBuildOutputLoaded = m_pBuildView->LoadBuildOutput(projectDirectory);
                    if (isBuildOutputLoaded)
                    {
                        // Previous build outputs were loaded correctly.
                        // Emit the signal indicating a build has succeeded, which will re-populate the view.
                        m_pBuildView->HandleProjectBuildSuccess();

                        // Restore the rgBuildView to the last-used layout dimensions.
                        m_pBuildView->RestoreViewLayout();
                    }
                }
            }
            else
            {
                // The project wasn't loaded properly. Return to the home page.
                HandleBackToHomeEvent();
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
    // Add the application name to the title text.
    this->setWindowTitle(STR_APP_NAME);
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

    // In this specific case, make sure that the build settings item is enabled,
    // since we are in a project (and not in the home page).
    m_pBuildSettingsAction->setEnabled(true);

    // Toggle the availability of the Edit menu.
    EnableEditMenu(!isProjectEmpty);
}

void rgMainWindow::HandleCreateNewFileEvent()
{
    // Create the BuildView.
    CreateBuildView();

    // Track if a project was actually created.
    bool wasProjectCreated = false;

    assert(m_pBuildView != nullptr);
    if (m_pBuildView != nullptr)
    {
        setAcceptDrops(false);
        wasProjectCreated = m_pBuildView->CreateNewSourceFileInProject();
        setAcceptDrops(true);
        if (wasProjectCreated)
        {
            // Show the build view as the central widget.
            SwitchToView(ui.buildPage);
        }
        else
        {
            // The project was not created eventually, so clean up the build view.
            DestroyBuildView();
        }
    }

    // Restore the layout if no project was created.
    if (!wasProjectCreated)
    {
        // Reset the actions to the default state.
        ResetActionsState();
    }
}

void rgMainWindow::ClearRecentProjectsList()
{
    // Only clear the list if it already exists.
    if (m_pRecentProjectButtonGroup != nullptr)
    {
        // Remove all Recent Project buttons added to the list.
        QLayout* pRecentProjectsList = ui.recentProgramsWrapper->layout();
        assert(pRecentProjectsList != nullptr);

        for (QAbstractButton* pRecentProjectLinkButton : m_pRecentProjectButtonGroup->buttons())
        {
            // Remove each button widget from the list of recent projects, and destroy it.
            pRecentProjectsList->removeWidget(pRecentProjectLinkButton);
            RG_SAFE_DELETE(pRecentProjectLinkButton);
        }

        // Destroy the recent projects list button group.
        RG_SAFE_DELETE(m_pRecentProjectButtonGroup);

        // Reorder the tab order to allow for the recent projects list removals.
        ReorderTabOrder(pRecentProjectsList);
    }
}

rgMainWindow::SettingsListWidgetEntries rgMainWindow::GetSelectedSettingCategory() const
{
    // Cast the currently-selected row to a settings enum type.
    int currentRow = ui.settingsListWidget->currentRow();
    return static_cast<SettingsListWidgetEntries>(currentRow);
}

void rgMainWindow::PopulateRecentProjectsList()
{
    // Clear the existing list of recent project buttons added to the view.
    ClearRecentProjectsList();

    const std::vector<std::string>& recentProjects = rgConfigManager::Instance().GetRecentProjects();

    bool hasRecentProjects = recentProjects.size() > 0;

    // Change the visibility of the "No recent sessions" label depending on what's in the settings file.
    ui.noRecentSessionsDummyPushButton->setVisible(!hasRecentProjects);

    if (hasRecentProjects)
    {
        // Create a new button group to handle clicks on recent project buttons.
        m_pRecentProjectButtonGroup = new QButtonGroup(this);
        bool isConnected = connect(m_pRecentProjectButtonGroup, static_cast<void (QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked),
            this, &rgMainWindow::HandleRecentProjectClickedEvent);
        assert(isConnected);

        // If the button group handler is connected correctly, add a button for each recent project entry.
        if (isConnected)
        {
            QLayout* pRecentProjectsList = ui.recentProgramsWrapper->layout();
            assert(pRecentProjectsList != nullptr);

            int numRecentProjects = static_cast<int>(recentProjects.size());

            // The index of the button in the QButtonGroup. This is how the
            // button is being identified within the group when signals are being fired.
            int buttonIndex = numRecentProjects;
            for (std::vector<std::string>::const_reverse_iterator projectIter = recentProjects.rbegin();
                projectIter != recentProjects.rend(); ++projectIter)
            {
                // Display the most recent projects at the end of the list.
                const std::string& projectPath = *projectIter;

                // Extract just the filename to display in the list of recent projects.
                std::string projectName;
                bool isOk = rgUtils::ExtractFileName(projectPath, projectName, true);
                assert(isOk);

                if (isOk)
                {
                    // Create a new QPushButton for each recent file being added to the list.
                    QPushButton* pRecentProjectPushButton = new QPushButton(this);

                    // The text as the filename, and the tooltip as the full path.
                    pRecentProjectPushButton->setText(projectName.c_str());
                    pRecentProjectPushButton->setToolTip(projectPath.c_str());

                    // Build and set the status tip message.
                    std::stringstream statusTipMsg;
                    statusTipMsg << STR_MAIN_WINDOW_LOAD_PROJECT << " " << projectName.c_str();
                    pRecentProjectPushButton->setStatusTip(statusTipMsg.str().c_str());

                    // Add the layout to the list of recent project buttons.
                    pRecentProjectsList->addWidget(pRecentProjectPushButton);

                    // Set the cursor to pointing hand cursor.
                    pRecentProjectPushButton->setCursor(Qt::PointingHandCursor);

                    // Set the context menu.
                    pRecentProjectPushButton->setContextMenuPolicy(Qt::CustomContextMenu);

                    // Connect signal/slot for the context menu.
                    connect(pRecentProjectPushButton, &QPushButton::customContextMenuRequested, this, &rgMainWindow::HandleContextMenuRequest);

                    // Add the recent project button to the group.
                    m_pRecentProjectButtonGroup->addButton(pRecentProjectPushButton, --buttonIndex);
                }
            }
            // Reorder the tab order to allow for the recent projects list additions.
            ReorderTabOrder(pRecentProjectsList);
        }
    }
}

void rgMainWindow::SwitchToView(QWidget* pWidget)
{
    if (pWidget != nullptr)
    {
        ui.stackedWidget->setCurrentWidget(pWidget);

        // Enable/disable back to home menu item.
        bool isBackToHomeEnabled = (pWidget != ui.homePage);
        m_pBackToHomeAction->setEnabled(isBackToHomeEnabled);
    }
}

void rgMainWindow::OpenFilesInBuildView(const QStringList& filePaths)
{
    if (!filePaths.empty())
    {
        // Create the BuildView.
        CreateBuildView();

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
            if (!m_pBuildView->IsEmpty())
            {
                // Show the build view as the central widget.
                SwitchToView(ui.buildPage);
            }
            else
            {
                // The project was not created eventually, so clean up the build view.
                DestroyBuildView();
            }
        }
    }
}

void rgMainWindow::HandleOpenExistingFileEvent()
{
    // Retrieve the current API from the configuration manager.
    rgProjectAPI currentAPI = rgConfigManager::Instance().GetCurrentAPI();

    QStringList selectedFiles;
    bool isOk = rgUtils::OpenFileDialogForMultipleFiles(this, currentAPI, selectedFiles);
    if (isOk && !selectedFiles.empty())
    {
        setAcceptDrops(false);
        OpenFilesInBuildView(selectedFiles);
        setAcceptDrops(true);
    }
}

void rgMainWindow::HandleOpenProjectFileEvent()
{
    std::string selectedFile;
    bool isOk = rgUtils::OpenProjectDialog(this, selectedFile);
    if (isOk && !selectedFile.empty())
    {
        this->statusBar()->showMessage(STR_MAIN_WINDOW_LOADING_PROJECT);
        bool isProjectLoaded = OpenProjectFileAtPath(selectedFile);
        assert(isProjectLoaded);
        if (isProjectLoaded)
        {
            this->statusBar()->showMessage(STR_MAIN_WINDOW_PROJECT_LOAD_SUCCESS);
        }
        else
        {
            this->statusBar()->showMessage(STR_ERR_CANNOT_LOAD_PROJECT_FILE);
        }
    }
}

void rgMainWindow::HandleSaveFileEvent()
{
    // If the rgBuildView exists, forward the save signal.
    if (m_pBuildView != nullptr)
    {
        // Save the current source file or build settings.
        m_pBuildView->HandleSaveSettingsButtonClicked();
    }
    else
    {
        // If Save global build settings.
        if (m_pSettingsButtonsView != nullptr)
        {
            m_pSettingsButtonsView->SaveSettingsButtonClickedSignal();
        }
    }
}

void rgMainWindow::HandleBackToHomeEvent()
{
    if (m_pBuildView != nullptr)
    {
        // Refresh the recent projects list upon returning to the start page.
        PopulateRecentProjectsList();

        // Ask the user if they want to save all unsaved files.
        bool isAccepted = m_pBuildView->RequestRemoveAllFiles();

        // Only exit if the dialog was accepted (ie. the user didn't press cancel).
        if (isAccepted)
        {
            // Switch the view back to the home page.
            SwitchToView(ui.homePage);

            // Destroy the rgBuildView instance since the project is being closed.
            DestroyBuildView();

            // Reset the window title.
            ResetWindowTitle();

            // Reset the status bar.
            this->statusBar()->showMessage("");
        }
    }
}

void rgMainWindow::HandleExitEvent()
{
    // Prompt the user to save any pending changes on SETTINGS tab.
    rgMainWindowTabBar* pTabBar = static_cast<rgMainWindowTabBar*>(ui.mainTabWidget->GetTabBar());
    rgUnsavedItemsDialog::UnsavedFileDialogResult isSaveSettingsAccepted = pTabBar->SaveSettings();
    if (isSaveSettingsAccepted == rgUnsavedItemsDialog::UnsavedFileDialogResult::Yes)
    {
        // Save the settings
        HandleSaveSettingsButtonClicked();
    }

    if (m_pBuildView != nullptr)
    {
        // Ask the user if they want to save all unsaved files.
        bool isAccepted = m_pBuildView->ShowSaveDialog();

        // Only exit if the dialog was accepted (ie. the user didn't press cancel).
        if (isAccepted && isSaveSettingsAccepted != rgUnsavedItemsDialog::UnsavedFileDialogResult::Cancel)
        {
            QApplication::exit(0);
        }
    }
    else
    {
        // Exit out of the application only if the user did NOT press cancel.
        if (isSaveSettingsAccepted != rgUnsavedItemsDialog::UnsavedFileDialogResult::Cancel)
        {
            QApplication::exit(0);
        }
    }
}

void rgMainWindow::closeEvent(QCloseEvent* pEvent)
{
    // Ignore the close event so we can handle shutdown ourself.
    pEvent->ignore();

    HandleExitEvent();
}

void rgMainWindow::dropEvent(QDropEvent *pEvent)
{
    // Get list of all file urls.
    QList<QUrl> fileUrls = pEvent->mimeData()->urls();

    // If there is only one file and it is a project file (.rga),
    // call OpenProjectFileAtPath.
    if (fileUrls.size() == 1 && fileUrls.at(0).isLocalFile() && fileUrls.at(0).toLocalFile().endsWith(STR_PROJECT_FILE_EXTENSION))
    {
        bool isLoadSuccsesful = OpenProjectFileAtPath(fileUrls.at(0).toLocalFile().toStdString());
        assert(isLoadSuccsesful);

        if (isLoadSuccsesful)
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
        // Convert url list to a string list of local filepaths.
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

        // Disable additional drops and open the desired files in the buildview.
        setAcceptDrops(false);
        OpenFilesInBuildView(filenameStrings);
        setAcceptDrops(true);
    }
}

void rgMainWindow::dragEnterEvent(QDragEnterEvent* pEvent)
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
                    // Accept the action, making it so we recieve a dropEvent when the items are released.
                    pEvent->setDropAction(Qt::DropAction::CopyAction);
                    pEvent->accept();
                }

            }
        }
    }
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
    // Display the About dialog.
    rgAboutDialog aboutDialog(this);
    aboutDialog.exec();
}

void rgMainWindow::HandleGettingStartedGuideEvent()
{
    // Build the path to the quickstart guide document.
    static const char* QUICKSTART_GUIDE_RELATIVE_PATH = "/Documentation/html/quickstart.html";
    QString quickstartFilePath = "file:///";
    QString appDirPath = qApp->applicationDirPath();
    quickstartFilePath.append(appDirPath);
    quickstartFilePath.append(QUICKSTART_GUIDE_RELATIVE_PATH);
    QDesktopServices::openUrl(QUrl(quickstartFilePath, QUrl::TolerantMode));
}

void rgMainWindow::HandleHelpManual()
{
    // Build the path to the help manual document.
    static const char* HELP_MANUAL_RELATIVE_PATH = "/Documentation/html/index.html";
    QString helpManualFilePath = "file:///";
    QString appDirPath = qApp->applicationDirPath();
    helpManualFilePath.append(appDirPath);
    helpManualFilePath.append(HELP_MANUAL_RELATIVE_PATH);
    QDesktopServices::openUrl(QUrl(helpManualFilePath, QUrl::TolerantMode));
}

void rgMainWindow::HandleBuildProjectEvent()
{
    assert(m_pBuildView != nullptr);
    if (m_pBuildView != nullptr)
    {
        // Save all source files when a new build is started.
        QStringList unsavedSources;
        m_pBuildView->GetUnsavedSourceFiles(unsavedSources);
        if (!unsavedSources.empty())
        {
            for (QString sourceFilePath : unsavedSources)
            {
                m_pBuildView->SaveSourceFile(sourceFilePath.toStdString().c_str());
            }
        }

        // Save the current project first.
        m_pBuildView->SaveCurrentFile();

        // Now build the project.
        m_pBuildView->BuildCurrentProject();
    }
}

void rgMainWindow::HandleBuildSettingsEvent()
{
    m_pBuildView->OpenBuildSettings();

    // Disable the find functionality.
    m_pFindAction->setEnabled(false);

    // Disable the Go-to-line functionality.
    m_pGoToLineAction->setEnabled(false);

    // Switch the file menu save file action to save build settings action.
    SwitchSaveShortcut(SaveActionType::SaveBuildSettings);

    // Update the file menu save action visibility.
    OnCurrentEditorModificationStateChanged(m_saveSettingsActionActive);
}

void rgMainWindow::HandleCancelBuildEvent()
{
    // Ask the build view to cancel the current build.
    m_pBuildView->CancelCurrentBuild();
}

void rgMainWindow::HandleRecentProjectClickedEvent(int recentFileIndex)
{
    std::shared_ptr<rgGlobalSettings> pGlobalSettings = rgConfigManager::Instance().GetGlobalConfig();

    if (pGlobalSettings != nullptr)
    {
        bool isValidRange = (recentFileIndex >= 0 && recentFileIndex < pGlobalSettings->m_recentProjects.size());
        assert(isValidRange);

        // If the file index is valid, attempt to open the file.
        if (isValidRange)
        {
            // Pull the recent project info out of the global settings structure.
            std::string projectPath = pGlobalSettings->m_recentProjects[recentFileIndex];

            // Attempt to load the project.
            this->statusBar()->showMessage(STR_MAIN_WINDOW_LOADING_PROJECT);
            bool isLoadSuccsesful = OpenProjectFileAtPath(projectPath);
            assert(isLoadSuccsesful);

            if (isLoadSuccsesful)
            {
                this->statusBar()->showMessage(STR_MAIN_WINDOW_PROJECT_LOAD_SUCCESS);
            }
            else
            {
                this->statusBar()->showMessage(STR_ERR_CANNOT_LOAD_PROJECT_FILE);
            }
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

void rgMainWindow::HandleProjectLoaded(std::shared_ptr<rgProject> pProject)
{
    assert(pProject != nullptr);
    if (pProject != nullptr)
    {
        // Update the application title with the project's name.
        SetWindowTitle(pProject->m_projectName);
        assert(pProject->m_clones[0] != nullptr);

        // If there are no files in the project, disable source and build related menu items.
        if ((pProject->m_clones[0] != nullptr) && (pProject->m_clones[0]->m_sourceFiles.empty()))
        {
            // Disable the build-related actions, except for the build settings action.
            m_pBuildProjectAction->setEnabled(false);
            m_pBuildSettingsAction->setEnabled(true);
            m_pCancelBuildAction->setEnabled(false);

            // Disable the source-file related actions.
            EnableEditMenu(false);
        }
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

void rgMainWindow::HandleContextMenuRequest(const QPoint& pos)
{
    // Set context menu mouse cursor
    m_menu.setCursor(Qt::PointingHandCursor);

    // The list of actions in the context menu.
    QList<QAction*> menuActions = m_menu.actions();

    // Extract the file name clicked on
    QObject* pSender = QObject::sender();
    QPushButton* pButton = qobject_cast<QPushButton*>(pSender);
    QString fileName = pButton->text();

    foreach(auto pAction, menuActions)
    {
        if (pAction == m_pOpenRecentAction)
        {
            // Build the Load <file name> string.
            QString loadString;
            loadString.append(STR_MAIN_WINDOW_LOAD_PROJECT);
            loadString.append(" ");
            loadString.append(fileName);
            pAction->setText(loadString);
            break;
        }
    }

    QAction* pAction = m_menu.exec(QCursor::pos());

    // Process the context menu selection
    if (pAction != nullptr)
    {
        QString menuSelection = pAction->text();

        // Find out the index into the button group for this file
        QList<QAbstractButton *> buttonList = m_pRecentProjectButtonGroup->buttons();
        int recentFileIndex = 0;

        // Find the index for the button clicked on
        foreach(auto pButton, buttonList)
        {
            recentFileIndex++;
            if (fileName.compare(pButton->text()) == 0)
            {
                break;
            }
        }

        // Determine the index of the recent item that was clicked.
        int selectedFileIndex = buttonList.count() - recentFileIndex;
        bool isIndexValid = (selectedFileIndex >= 0 && selectedFileIndex < buttonList.count());
        assert(isIndexValid);
        if (isIndexValid)
        {
            // Pull the recent project info out of the global settings structure.
            std::shared_ptr<rgGlobalSettings> pGlobalSettings = rgConfigManager::Instance().GetGlobalConfig();
            const std::string& projectPath = pGlobalSettings->m_recentProjects[selectedFileIndex];

            if (pAction == m_pOpenRecentAction)
            {
                // Attempt to load the recent project.
                OpenProjectFileAtPath(projectPath);
            }
            else if (pAction == m_pOpenContainingFolderAction)
            {
                // Get the directory where the project's settings file lives.
                std::string fileDirectory;
                bool gotDirectory = rgUtils::ExtractFileDirectory(projectPath, fileDirectory);
                assert(gotDirectory);

                if (gotDirectory)
                {
                    // Open a system file browser window pointing to the given directory.
                    rgUtils::OpenFolderInFileBrowser(fileDirectory);
                }
            }
        }
    }
}

void rgMainWindow::HandleProjectBuildFailure()
{
    // Show the build failure message.
    this->statusBar()->showMessage(STR_STATUS_BAR_BUILD_FAILED);

    // Reset the view's state.
    ResetViewStateAfterBuild();
}

void rgMainWindow::HandleProjectBuildCanceled()
{
    // Show the build cancellation message.
    this->statusBar()->showMessage(STR_STATUS_BAR_BUILD_CANCELED);

    // Reset the view's state.
    ResetViewStateAfterBuild();
}

void rgMainWindow::ResetViewStateAfterBuild()
{
    // Reset the build-in-progress flag.
    emit IsBuildInProgress(false);

    // Re-enable all menu items, since the build is over.
    m_pBuildProjectAction->setEnabled(true);
    m_pCancelBuildAction->setEnabled(false);
    m_pNewFileAction->setEnabled(true);
    m_pOpenFileAction->setEnabled(true);
    m_pOpenProjectAction->setEnabled(true);
    m_pBackToHomeAction->setEnabled(true);
    m_pBuildSettingsAction->setEnabled(true);

    // Set the focus back to the source view.
    rgSourceCodeEditor* pSourceCodeEditor = m_pBuildView->GetEditorForFilepath(m_pBuildView->GetFileMenu()->GetSelectedFilePath());
    if (pSourceCodeEditor != nullptr && pSourceCodeEditor->isVisible())
    {
        pSourceCodeEditor->setFocus();
    }
}

void rgMainWindow::HandleProjectBuildSuccess()
{
    this->statusBar()->showMessage(STR_STATUS_BAR_BUILD_SUCCEEDED);

    // Reset the view's state.
    ResetViewStateAfterBuild();
}

void rgMainWindow::HandleProjectBuildStarted()
{
    // Mark that a build is now in progress.
    emit IsBuildInProgress(true);

    this->statusBar()->showMessage(STR_STATUS_BAR_BUILD_STARTED);

    // Do not allow another build while a build is already in progress.
    m_pBuildProjectAction->setEnabled(false);
    m_pCancelBuildAction->setEnabled(true);

    // Do not allow creating/adding files.
    m_pNewFileAction->setEnabled(false);
    m_pOpenFileAction->setEnabled(false);

    // Do not allow creating a project.
    m_pOpenProjectAction->setEnabled(false);

    // Do not allow going back to the home page.
    m_pBackToHomeAction->setEnabled(false);

    // Do not allow going to the build settings view.
    m_pBuildSettingsAction->setEnabled(false);

    // Show only the source editor when building the project.
    if (m_pBuildView != nullptr)
    {
        // Hide the disassembly view.
        m_pBuildView->ToggleDisassemblyViewVisibility(false);
    }
}

void rgMainWindow::HandleStatusBarTextChanged(const std::string& statusBarText, int timeoutMs)
{
    this->statusBar()->showMessage(statusBarText.c_str(), timeoutMs);
}

void rgMainWindow::SetCursor()
{
    // Set the cursor to pointing hand cursor.
    ui.recentProjectsOtherPushButton->setCursor(Qt::PointingHandCursor);
    ui.newFilePushButton->setCursor(Qt::PointingHandCursor);
    ui.existingFilePushButton->setCursor(Qt::PointingHandCursor);
    ui.aboutRGAPushButton->setCursor(Qt::PointingHandCursor);
    ui.gettingStartedPushButton->setCursor(Qt::PointingHandCursor);
    ui.helpManualPushButton->setCursor(Qt::PointingHandCursor);
    menuBar()->setCursor(Qt::PointingHandCursor);
    ui.settingsListWidget->setCursor(Qt::PointingHandCursor);
}

void rgMainWindow::HandleGoToLineEvent()
{
    // Get the max number of lines.
    rgSourceCodeEditor* pSourceCodeEditor = m_pBuildView->GetEditorForFilepath(m_pBuildView->GetFileMenu()->GetSelectedFilePath());
    if (pSourceCodeEditor->isVisible())
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
    }
}

void rgMainWindow::HandleGlobalPendingChangesStateChanged(bool hasPendingChanges)
{
    // Update the "Save" button located on settings buttons view.
    m_pSettingsButtonsView->EnableSaveButton(hasPendingChanges);

    // Update the settings list widget "Application" entry.
    QListWidgetItem* pItem = ui.settingsListWidget->item(SettingsListWidgetEntries::Global);
    assert(pItem);

    if (pItem != nullptr)
    {
        if (hasPendingChanges)
        {
            pItem->setText(QString(STR_BUILD_SETTINGS_APPLICATION) + STR_UNSAVED_FILE_SUFFIX);
        }
        else
        {
            pItem->setText(STR_BUILD_SETTINGS_APPLICATION);
        }

        // Update the File menu shortcut for save settings.
        OnCurrentEditorModificationStateChanged(hasPendingChanges);

        // Let the tab widget know that data has changed.
        // This is to allow the tab widget to display a dialog
        // to prompt the user to save changes before switching to the start tab.
        rgMainWindowTabBar* pTabBar = static_cast<rgMainWindowTabBar*>(ui.mainTabWidget->GetTabBar());
        pTabBar->UpdateApplicationPendingChanges(hasPendingChanges);
    }
}

void rgMainWindow::HandleBuildSettingsPendingChangesStateChanged(bool hasPendingChanges)
{
    // Enable the "Save" button located on settings buttons view.
    m_pSettingsButtonsView->EnableSaveButton(hasPendingChanges);

    // Update the settings list widget "OpenCL" entry.
    QListWidgetItem* pItem = ui.settingsListWidget->item(SettingsListWidgetEntries::OpenCL);
    assert(pItem);

    if (pItem != nullptr)
    {
        if (hasPendingChanges)
        {
            pItem->setText(QString(STR_DEFAULT_OPENCL_BUILD_SETTINGS) + STR_UNSAVED_FILE_SUFFIX);
        }
        else
        {
            pItem->setText(STR_DEFAULT_OPENCL_BUILD_SETTINGS);
        }

        // Update the File menu shortcut for save settings.
        OnCurrentEditorModificationStateChanged(hasPendingChanges);

        // Let the tab widget know that data has changed.
        // This is to allow the tab widget to display a dialog
        // to prompt the user to save changes before switching to the start tab.
        rgMainWindowTabBar* pTabBar = static_cast<rgMainWindowTabBar*>(ui.mainTabWidget->GetTabBar());
        pTabBar->UpdateBuildPendingChanges(hasPendingChanges);
    }
}

void rgMainWindow::HandleSaveSettingsButtonClicked()
{
    // Disable the "Save" button.
    m_pSettingsButtonsView->EnableSaveButton(false);

    assert(m_pGlobalSettingsView != nullptr);
    if (m_pGlobalSettingsView != nullptr && m_pGlobalSettingsView->GetHasPendingChanges())
    {
        // Save global application settings.
        m_pGlobalSettingsView->SaveSettings();
    }

    assert(m_pBuildSettingsView != nullptr);
    if (m_pBuildSettingsView != nullptr && m_pBuildSettingsView->GetHasPendingChanges())
    {
        // Save default build settings.
        m_pBuildSettingsView->SaveSettings();
    }

    // Update the File menu shortcut for save settings.
    OnCurrentEditorModificationStateChanged(false);
}

void rgMainWindow::HandleTabBarTabChanged(bool saveChanges)
{
    if (saveChanges)
    {
        HandleSaveSettingsButtonClicked();
    }
    else
    {
        // If the settings don't need to be saved,
        // we need to update the list widget string
        // and populate the ui with previous data.
        HandleBuildSettingsPendingChangesStateChanged(false);

        // Check for the API.
        rgConfigManager& configManager = rgConfigManager::Instance();
        rgProjectAPI currentAPI = configManager.GetCurrentAPI();

        switch (currentAPI)
        {
        case OpenCL:
            {
                // Cast base class to the derived class.
                rgOpenCLBuildSettingsView* pBuildSettingsView = static_cast<rgOpenCLBuildSettingsView*>(m_pBuildSettingsView);

                // Revert the changes.
                pBuildSettingsView->RevertPendingChanges();
            }
            break;
        case Unknown:
        default:
            // We shouldn't get here.
            assert(false);
            break;
        }

        // Revert pending changes for the "Application" settings as well.
        HandleGlobalPendingChangesStateChanged(saveChanges);
        m_pGlobalSettingsView->RevertPendingChanges();
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
        if (m_pBuildView != nullptr)
        {
            isBuildInProgress = m_pBuildView->IsBuildInProgress();
        }

        this->statusBar()->showMessage(isBuildInProgress ? STR_STATUS_BAR_BUILD_STARTED : msg);
    }
}

void rgMainWindow::HandleRestoreDefaultsSettingsClicked()
{
    // Ask the user for confirmation.
    bool isConfirmation = rgUtils::ShowConfirmationMessageBox(STR_BUILD_SETTINGS_DEFAULT_SETTINGS_CONFIRMATION_TITLE, STR_BUILD_SETTINGS_DEFAULT_SETTINGS_CONFIRMATION, this);

    if (isConfirmation)
    {
        // Disable the "Save" button.
        m_pSettingsButtonsView->EnableSaveButton(false);

        SettingsListWidgetEntries selectedCategory = GetSelectedSettingCategory();
        switch (selectedCategory)
        {
        case Global:
            {
                m_pGlobalSettingsView->RestoreDefaultSettings();
            }
            break;
        case OpenCL:
            {
                // Execute the "Save" button.
                m_pBuildSettingsView->RestoreDefaultSettings();
            }
            break;
        default:
            assert(false);
            break;
        }
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
        SwitchSaveShortcut(SaveActionType::SaveBuildSettings);

        // Update the file menu save action visibility.
        OnCurrentEditorModificationStateChanged(m_saveSettingsActionActive);
    }
}

void rgMainWindow::HandleSettingsListWidgetClick(int index)
{
    assert(m_pBuildSettingsView);
    assert(m_pGlobalSettingsView);

    bool isSaveEnabled = false;

    switch (index)
    {
    case SettingsListWidgetEntries::Global:
        {
            if (m_pBuildSettingsView != nullptr)
            {
                m_pBuildSettingsView->hide();
            }

            if (m_pGlobalSettingsView != nullptr)
            {
                m_pGlobalSettingsView->show();
            }

            isSaveEnabled = m_pGlobalSettingsView->GetHasPendingChanges();
        }
        break;
    case SettingsListWidgetEntries::OpenCL:
        {
            if (m_pBuildSettingsView != nullptr)
            {
                m_pBuildSettingsView->show();
            }

            if (m_pGlobalSettingsView != nullptr)
            {
                m_pGlobalSettingsView->hide();
            }

            isSaveEnabled = m_pBuildSettingsView->GetHasPendingChanges();
        }
        break;
    default:
        // We shouldn't get here.
        assert(false);
        break;
    }

    m_pSettingsButtonsView->EnableSaveButton(isSaveEnabled);

}

void rgMainWindow::ConnectSettingsListWidgetSignals()
{
    bool isConnected = false;

    isConnected = connect(ui.settingsListWidget, &QListWidget::currentRowChanged, this, &rgMainWindow::HandleSettingsListWidgetClick);
    assert(isConnected);
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
        SwitchSaveShortcut(SaveActionType::SaveBuildSettings);

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
    case (SaveActionType::SaveBuildSettings):
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

void rgMainWindow::SetBuildSettingsGeometry()
{
    // The build settings scroll area sizes for various dpis on a 4k display.
    std::map<double, int> mScrollAreaHeights = { { 1.0, 1200 },{ 1.25, 1050 },
    { 1.5, 975 },{ 1.75, 975 },
    { 2.0, 850 } };

    // Constants which we will use to identify the current display case.
    const int SCROLL_AREA_HEIGHT_FOR_768_DISPLAY = 550;
    const int SCROLL_AREA_HEIGHT_FOR_1200P_DISPLAY = 950;
    const int SCROLL_AREA_HEIGHT_FOR_1080P_DISPLAY = 830;
    const int SCROLL_4K_DISPLAY_MINIMUM_WIDTH = 3800;
    const int SCROLL_1080 = 1080;
    const int SCROLL_768_WIDTH_RESOLUTION = 768;

    // Get the current API type.
    rgConfigManager& configManager = rgConfigManager::Instance();
    rgProjectAPI currentAPI = configManager.GetCurrentAPI();

    switch (currentAPI)
    {
    case OpenCL:
    {
        // Cast base class to the derived class.
        rgOpenCLBuildSettingsView* pBuildSettingsView = static_cast<rgOpenCLBuildSettingsView*>(m_pBuildSettingsView);

        if (pBuildSettingsView != nullptr)
        {
            // Get the scale factor.
            double scaleFactor = ScalingManager::Get().GetScaleFactor();

            QDesktopWidget* pDesktopWidget = QApplication::desktop();
            if (pDesktopWidget != nullptr)
            {
                const int screenNumber = pDesktopWidget->screenNumber(geometry().center());
                const QRect desktopRes = QApplication::desktop()->availableGeometry(screenNumber);

                // Resize the scroll area so it'll fit properly on various resolutions from 100% to 150% dpi.
                if (desktopRes.width() > SCROLL_4K_DISPLAY_MINIMUM_WIDTH)
                {
                    pBuildSettingsView->SetScrollAreaFixedHeight(mScrollAreaHeights[scaleFactor]);
                }
                else if (desktopRes.height() > SCROLL_1080)
                {
                    // Assuming 1200 here.
                    // The factor which we will use to adjust the height.
                    const int SCROLL_1200_FACTOR = 237.5;

                    if (scaleFactor < 1.25)
                    {
                        // 100%.
                        pBuildSettingsView->SetScrollAreaFixedHeight(SCROLL_AREA_HEIGHT_FOR_1200P_DISPLAY);
                    }
                    else if (scaleFactor < 1.5)
                    {
                        // 125%.
                        pBuildSettingsView->SetScrollAreaFixedHeight(SCROLL_AREA_HEIGHT_FOR_1200P_DISPLAY - SCROLL_1200_FACTOR);
                    }
                    else if (scaleFactor < 1.75)
                    {
                        // 150%.
                        pBuildSettingsView->SetScrollAreaFixedHeight(SCROLL_AREA_HEIGHT_FOR_1200P_DISPLAY - SCROLL_1200_FACTOR * 1.5);
                    }
                }
                else if (desktopRes.height() <= SCROLL_768_WIDTH_RESOLUTION)
                {
                    /// The factor which we will use to adjust the height.
                    const int SCROLL_768_FACTOR = 160;

                    if (scaleFactor < 1.25)
                    {
                        // 100%.
                        pBuildSettingsView->SetScrollAreaFixedHeight(SCROLL_AREA_HEIGHT_FOR_768_DISPLAY);

                        // Set the boolean to indicate this is a special resolution case.
                        m_isSpecialResolution = true;
                    }
                    else if (scaleFactor < 1.5)
                    {
                        // 125%.
                        pBuildSettingsView->SetScrollAreaFixedHeight(SCROLL_AREA_HEIGHT_FOR_768_DISPLAY - SCROLL_768_FACTOR);

                        // Set the boolean to indicate this is a special resolution case.
                        m_isSpecialResolution = true;
                    }
                }
                else
                {
                    // Assuming 1080 here.
                    // The factor which we will use to adjust the height.
                    const int SCROLL_1080_FACTOR = 200;

                    if (scaleFactor < 1.25)
                    {
                        // 100%.
                        pBuildSettingsView->SetScrollAreaFixedHeight(SCROLL_AREA_HEIGHT_FOR_1080P_DISPLAY);
                    }
                    else if (scaleFactor < 1.5)
                    {
                        // 125%.
                        pBuildSettingsView->SetScrollAreaFixedHeight(SCROLL_AREA_HEIGHT_FOR_1080P_DISPLAY - SCROLL_1080_FACTOR);
                    }
                }
            }
        }

        break;
    }
    case Unknown:
    default:
        // We shouldn't get here.
        assert(false);
        break;
    }
}

void rgMainWindow::ResetActionsState()
{
    // Block build-related and source-file-related actions.
    EnableBuildMenu(false);
    m_pCancelBuildAction->setEnabled(false);
    EnableEditMenu(false);
}

void rgMainWindow::EnableBuildViewActions()
{
    // Enable the build menu options after the build view has been opened.
    EnableBuildMenu(true);
    m_pCancelBuildAction->setEnabled(false);

    // Enable the edit menu options after the build view has been opened.
    EnableEditMenu(true);
}

void rgMainWindow::ReorderTabOrder(QLayout* pLayout)
{
    setTabOrder(ui.startTab, ui.newFilePushButton);
    setTabOrder(ui.newFilePushButton, ui.existingFilePushButton);

    // Get the recent projects buttons list.
    QList<QAbstractButton*> recentProjectsList;
    if (m_pRecentProjectButtonGroup != nullptr)
    {
        recentProjectsList = m_pRecentProjectButtonGroup->buttons();
    }

    // If the recent projects list is not empty, add tab order for these projects.
    if (!recentProjectsList.isEmpty())
    {
        setTabOrder(ui.existingFilePushButton, recentProjectsList.at(0));

        for (int i = 1; i < recentProjectsList.count()-1; i++)
        {
            setTabOrder(recentProjectsList.at(i), recentProjectsList.at(i+1));
        }
        setTabOrder(recentProjectsList.at(recentProjectsList.count() - 1), ui.recentProjectsOtherPushButton);
    }
    else
    {
        setTabOrder(ui.existingFilePushButton, ui.recentProjectsOtherPushButton);
    }

    setTabOrder(ui.recentProjectsOtherPushButton, ui.aboutRGAPushButton);
    setTabOrder(ui.aboutRGAPushButton, ui.helpManualPushButton);
    setTabOrder(ui.helpManualPushButton, ui.gettingStartedPushButton);
}

bool rgMainWindow::eventFilter(QObject* pObject, QEvent* pEvent)
{
    assert(pObject != nullptr);
    assert(pEvent != nullptr);

    bool result = false;

    if (pEvent != nullptr && pEvent->type() == QEvent::KeyPress)
    {
        QKeyEvent* pKeyEvent = static_cast<QKeyEvent*>(pEvent);
        assert(pKeyEvent != nullptr);

        // Find out if we need to process this key press.
        QString objectName;
        if (pObject != nullptr)
        {
            objectName = pObject->objectName();
        }
        bool processKeyPress = ProcessKeyPress(pKeyEvent, objectName);

        if (processKeyPress)
        {
            if (pObject == ui.recentProgramsWrapper)
            {
                if (pKeyEvent != nullptr)
                {
                    switch (pKeyEvent->key())
                    {
                    case Qt::Key_Up:
                        // Give focus to the previous widget.
                        ui.existingFilePushButton->setFocus();
                        result = true;

                        break;
                    case Qt::Key_Down:
                        // Give focus to the next widget.
                        ui.recentProjectsOtherPushButton->setFocus();
                        result = true;

                        break;
                    }
                }
            }
        }
        else
        {
            // Ignore up/down key presses for top and bottom buttons.
            result = true;
        }
    }
    return result;
}

bool rgMainWindow::ProcessKeyPress(QKeyEvent* pKeyEvent, const QString& objectName)
{
    assert(pKeyEvent != nullptr);

    static const char* STR_NEW_FILE_PUSH_BUTTON = "newFilePushButton";
    static const char* STR_GETTING_STARTED_PUSH_BUTTON = "gettingStartedPushButton";

    bool result = true;

    if (pKeyEvent != nullptr)
    {
        if (objectName.compare(STR_NEW_FILE_PUSH_BUTTON) == 0 && pKeyEvent->key() == Qt::Key_Up)
        {
            result = false;
        }
        else if (objectName.compare(STR_GETTING_STARTED_PUSH_BUTTON) == 0 && pKeyEvent->key() == Qt::Key_Down)
        {
            result = false;
        }
    }

    return result;
}
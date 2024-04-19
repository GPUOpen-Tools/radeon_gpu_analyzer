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
#include "QtCommon/Scaling/ScalingManager.h"

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_about_dialog.h"
#include "radeon_gpu_analyzer_gui/qt/rg_app_state.h"
#include "radeon_gpu_analyzer_gui/qt/rg_build_view.h"
#include "radeon_gpu_analyzer_gui/qt/rg_global_settings_view.h"
#include "radeon_gpu_analyzer_gui/qt/rg_menu.h"
#include "radeon_gpu_analyzer_gui/qt/rg_menu_build_settings_item.h"
#include "radeon_gpu_analyzer_gui/qt/rg_go_to_line_dialog.h"
#include "radeon_gpu_analyzer_gui/qt/rg_isa_disassembly_view.h"
#include "radeon_gpu_analyzer_gui/qt/rg_main_window.h"
#include "radeon_gpu_analyzer_gui/qt/rg_recent_project_widget.h"
#include "radeon_gpu_analyzer_gui/qt/rg_settings_buttons_view.h"
#include "radeon_gpu_analyzer_gui/qt/rg_settings_tab.h"
#include "radeon_gpu_analyzer_gui/qt/rg_source_code_editor.h"
#include "radeon_gpu_analyzer_gui/qt/rg_start_tab.h"
#include "radeon_gpu_analyzer_gui/qt/rg_status_bar.h"
#include "radeon_gpu_analyzer_gui/qt/rg_view_container.h"
#include "radeon_gpu_analyzer_gui/rg_config_file.h"
#include "radeon_gpu_analyzer_gui/rg_config_manager.h"
#include "radeon_gpu_analyzer_gui/rg_data_types.h"
#include "radeon_gpu_analyzer_gui/rg_data_types_opencl.h"
#include "radeon_gpu_analyzer_gui/rg_data_types_vulkan.h"
#include "radeon_gpu_analyzer_gui/rg_definitions.h"
#include "radeon_gpu_analyzer_gui/rg_factory.h"
#include "radeon_gpu_analyzer_gui/rg_string_constants.h"
#include "radeon_gpu_analyzer_gui/rg_utils.h"

static const int kCustomStatusBarHeight = 25;

RgMainWindow::RgMainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    // Get the startup mode.
    RgConfigManager& config_manager = RgConfigManager::Instance();
    RgProjectAPI current_api = config_manager.GetCurrentAPI();

    // We must have a known mode at startup.
    assert(current_api != RgProjectAPI::kUnknown);

    // Create the factory through which we create API-specific components.
    factory_ = RgFactory::CreateFactory(current_api);

    // Setup the UI.
    ui_.setupUi(this);

    // Set the window icon to the product logo.
    setWindowIcon(QIcon(kIconResourceRgaLogo));

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

void RgMainWindow::InitMainWindow()
{
    // Use the mode to setup API-specific actions.
    RgProjectAPI current_api = RgConfigManager::Instance().GetCurrentAPI();
    ChangeApiMode(current_api);

    // Enable drops so the main window can handle opening dropped source files.
    setAcceptDrops(true);

    // Set the mouse cursor to the pointing hand cursor for various widgets.
    SetCursor();

    // Set the parent widget for the tab bar so the dialogs get shown in the correct location.
    ui_.mainTabWidget->GetTabBar()->SetParentWidget(this);

    // Set the focus to the tab bar.
    ui_.mainTabWidget->setFocus();

    // Increase the double click interval as the default one is too quick.
    const int doubleClickInterval = qApp->doubleClickInterval();
    qApp->setDoubleClickInterval(doubleClickInterval * 2);
}

void RgMainWindow::ConnectSignals()
{
    // Tab changed signal to handle save shortcut updates.
    bool is_connected = connect(ui_.mainTabWidget, &RgMainWindowTabWidget::currentChanged, this, &RgMainWindow::HandleMainTabWidgetTabChanged);
    assert(is_connected);

    // Connect the status bar's message change signal.
    is_connected = connect(this->statusBar(), &QStatusBar::messageChanged, this, &RgMainWindow::HandleStatusBarMessageChange);
    assert(is_connected);

    // Connect the default API's settings view's handler for pending changes.
    is_connected = connect(settings_tab_, &RgSettingsTab::PendingChangesStateChanged, this, &RgMainWindow::HandleHasSettingsPendingStateChanged);
    assert(is_connected);

    // Connect the notification message blinking timer signal.
    is_connected = connect(app_notification_blinking_timer_, &QTimer::timeout, this, &RgMainWindow::HandleAppNotificationMessageTimerFired);
    assert(is_connected);
}

void RgMainWindow::CreateSettingsTab()
{
    // Get settings tab container widget.
    assert(ui_.mainTabWidget != nullptr);
    if (ui_.mainTabWidget != nullptr)
    {
        QWidget* tab_container = ui_.mainTabWidget->widget(1);
        assert(tab_container != nullptr);
        if (tab_container != nullptr)
        {
            assert(factory_ != nullptr);
            if (factory_ != nullptr)
            {
                // Delete any existing settings tab first.
                DestroySettingsTab();

                settings_tab_ = factory_->CreateSettingsTab(ui_.mainTabWidget);
                assert(settings_tab_ != nullptr);
                if (settings_tab_ != nullptr)
                {
                    // Initialize the tab before adding to the parent widget.
                    settings_tab_->Initialize();

                    RgMainWindowTabBar* tab_bar = ui_.mainTabWidget->GetTabBar();
                    assert(tab_bar != nullptr);
                    if (tab_bar != nullptr)
                    {
                        tab_bar->SetSettingsTab(settings_tab_);
                    }

                    app_state_->SetSettingsTab(settings_tab_);
                    ui_.settingsTab->layout()->addWidget(settings_tab_);

                    // Handle when the Settings tab signals that it has pending changes.
                    bool is_connected = connect(settings_tab_, &RgSettingsTab::PendingChangesStateChanged, this, &RgMainWindow::HandleHasSettingsPendingStateChanged);
                    assert(is_connected);
                    Q_UNUSED(is_connected);
                }
            }
        }
    }
}

void RgMainWindow::DestroySettingsTab()
{
    if (settings_tab_ != nullptr)
    {
        delete settings_tab_;
        settings_tab_ = nullptr;
    }
}

void RgMainWindow::CreateStartTab()
{
    assert(factory_ != nullptr);
    if (factory_ != nullptr)
    {
        RgStartTab* start_tab = factory_->CreateStartTab(this);

        // Register the start tab with the scaling manager.
        ScalingManager::Get().RegisterObject(start_tab);

        assert(start_tab != nullptr && app_state_ != nullptr);
        if (start_tab != nullptr && app_state_ != nullptr)
        {
            // Destroy the existing start tab first.
            DestroyStartTab();

            // Set the start page for the mode, and add it to the main window's layout.
            app_state_->SetStartTab(start_tab);
            ui_.startTab->layout()->addWidget(start_tab);

            // Initialize the start tab view.
            start_tab->Initialize();

            // Connect signals for the new start tab.
            ConnectStartTabSignals();
        }
    }
}

void RgMainWindow::DestroyStartTab()
{
    // Remove the existing widget first, if there is one.
    // This is required because when the user switches APIs,
    // there'll be duplicate widgets on the home page if we
    // don't delete the existing widget here first.
    QLayoutItem* layout_item = ui_.startTab->layout()->takeAt(0);
    if (layout_item != nullptr)
    {
        QWidget* widget = layout_item->widget();
        assert(widget != nullptr);
        if (widget != nullptr)
        {
            widget->deleteLater();
        }
    }
}

void RgMainWindow::ConnectStartTabSignals()
{
    // Connect signals for the mode's start tab.
    RgStartTab* start_tab = app_state_->GetStartTab();

    assert(start_tab != nullptr);
    if (start_tab != nullptr)
    {
        // Signal for the Open Program button click.
        bool is_connected = connect(start_tab, &RgStartTab::OpenProjectFileEvent, this, &RgMainWindow::HandleOpenProjectFileEvent);
        assert(is_connected);

        // Signal to load the project file at the given path.
        is_connected = connect(start_tab, &RgStartTab::OpenProjectFileAtPath, this, &RgMainWindow::HandleOpenProjectFileAtPath);
        assert(is_connected);

        // Signal emitted when the Help->About item is clicked.
        is_connected = connect(start_tab, &RgStartTab::AboutEvent, this, &RgMainWindow::HandleAboutEvent);
        assert(is_connected);

        // Signal emitted when the Help->Getting started guide item is clicked.
        is_connected = connect(start_tab, &RgStartTab::GettingStartedGuideEvent, this, &RgMainWindow::HandleGettingStartedGuideEvent);
        assert(is_connected);

        // Signal emitted when the Help->Help manual item is clicked.
        is_connected = connect(start_tab, &RgStartTab::HelpManual, this, &RgMainWindow::HandleHelpManual);
        assert(is_connected);
    }
}

bool RgMainWindow::AddBuildView()
{
    bool status = false;

    assert(app_state_ != nullptr);
    if (app_state_ != nullptr)
    {
        // Add the existing RgBuildView instance to the MainWindow's layout.
        RgBuildView* build_view = app_state_->GetBuildView();

        assert(build_view != nullptr);
        if (build_view != nullptr)
        {
            // Initialize the RgBuildView instance's interface.
            status = build_view->InitializeView();

            // Connect the menu signals to the RgBuildView and RgMainWindow.
            ConnectMenuSignals();

            // Add the RgBuildView instance to the main window's build page.
            ui_.buildPage->layout()->addWidget(build_view);

            // Register the object with the scaling manager.
            ScalingManager::Get().RegisterObject(build_view);

            // Adjust the actions to the build view's initial state.
            EnableBuildViewActions();

#ifdef RGA_GUI_AUTOMATION
            emit TEST_BuildViewCreated(build_view);
#endif
        }
    }

    return status;
}

void RgMainWindow::ConnectBuildViewSignals()
{
    assert(app_state_ != nullptr);
    if (app_state_ != nullptr)
    {
        // Get the API-specific RgBuildView instance used by the current mode.
        RgBuildView* build_view = app_state_->GetBuildView();

        // Verify that the build view exists before using it.
        assert(build_view != nullptr);
        if (build_view != nullptr)
        {
            // Editor text change handler.
            bool is_connected = connect(build_view, &RgBuildView::CurrentEditorModificationStateChanged, this, &RgMainWindow::OnCurrentEditorModificationStateChanged);
            assert(is_connected);

            // Connect the project count changed signal.
            is_connected = connect(build_view, &RgBuildView::ProjectFileCountChanged, this, &RgMainWindow::HandleProjectFileCountChanged);
            assert(is_connected);

            // Connect the project loaded signal.
            is_connected = connect(build_view, &RgBuildView::ProjectLoaded, this, &RgMainWindow::HandleProjectLoaded);
            assert(is_connected);

            // Connect the current file modified signal.
            is_connected = connect(build_view, &RgBuildView::CurrentFileModified, this, &RgMainWindow::HandleCurrentFileModifiedOutsideEnv);
            assert(is_connected);

            // Connect the project build success signal.
            is_connected = connect(build_view, &RgBuildView::ProjectBuildSuccess, this, &RgMainWindow::HandleProjectBuildSuccess);
            assert(is_connected);

            // Connect the project build started signal.
            is_connected = connect(build_view, &RgBuildView::ProjectBuildStarted, this, &RgMainWindow::HandleProjectBuildStarted);
            assert(is_connected);

            // Connect the project build event trigger action signal.
            is_connected = connect(build_view, &RgBuildView::BuildProjectEvent, this, &RgMainWindow::HandleBuildProjectEvent);
            assert(is_connected);

            // Connect the update application notification message signal.
            is_connected = connect(build_view, &RgBuildView::UpdateApplicationNotificationMessageSignal, this, &RgMainWindow::HandleUpdateAppNotificationMessage);
            assert(is_connected);

            // Connect the project build failure signal.
            is_connected = connect(build_view, &RgBuildView::ProjectBuildFailure, this, &RgMainWindow::HandleProjectBuildFailure);
            assert(is_connected);

            // Connect the project build canceled signal.
            is_connected = connect(build_view, &RgBuildView::ProjectBuildCanceled, this, &RgMainWindow::HandleProjectBuildCanceled);
            assert(is_connected);

            // Connect the RgBuildView's status bar update signal.
            is_connected = connect(build_view, &RgBuildView::SetStatusBarText, this, &RgMainWindow::HandleStatusBarTextChanged);
            assert(is_connected);

            // Connect the RgBuildView's enable show max VGPR option signal.
            is_connected = connect(build_view, &RgBuildView::EnableShowMaxVgprOptionSignal, this, &RgMainWindow::HandleEnableShowMaxVgprOptionSignal);
            assert(is_connected);

            // Connect the main window's Find event with the RgBuildView.
            is_connected = connect(this, &RgMainWindow::FindTriggered, build_view, &RgBuildView::HandleFindTriggered);
            assert(is_connected);

            // Connect the RgMainWindow to the RgBuildView to update the "project is building" flag.
            is_connected = connect(this, &RgMainWindow::IsBuildInProgress, build_view, &RgBuildView::HandleIsBuildInProgressChanged);
            assert(is_connected);

            // Connect the edit mode changed signal.
            is_connected = connect(build_view, &RgBuildView::EditModeChanged, this, &RgMainWindow::HandleEditModeChanged);
            assert(is_connected);

            // Connect the disable pipeline state menu item signal.
            is_connected = connect(app_state_.get(), &RgAppState::EnablePipelineMenuItem, this, &RgMainWindow::HandleEnablePipelineMenuItem);
            assert(is_connected);

            // Connect the disable build settings menu item signal.
            is_connected = connect(app_state_.get(), &RgAppState::EnableBuildSettingsMenuItem, this, &RgMainWindow::HandleEnableBuildSettingsMenuItem);
            assert(is_connected);

            // Connect the enable Ctrl+S menu item signal.
            is_connected = connect(app_state_.get(), &RgAppState::EnableSaveSettingsMenuItem, this, &RgMainWindow::HandleEnableSaveSettingsMenuItem);
            assert(is_connected);

            // Connect the app state to the RgBuildView.
            app_state_->ConnectBuildViewSignals(build_view);
        }
    }
}

void RgMainWindow::HandleEnableShowMaxVgprOptionSignal(bool is_enabled)
{
    show_max_vgprs_action_->setEnabled(is_enabled);
}

void RgMainWindow::HandleEnablePipelineMenuItem(bool is_enabled)
{
    // Update the Build menu item for "Pipeline state".
    pipeline_state_action_->setEnabled(is_enabled);
}

void RgMainWindow::HandleEnableBuildSettingsMenuItem(bool is_enabled)
{
	// Update the Build menu item for "Build settings".
    build_settings_action_->setEnabled(is_enabled);
}

void RgMainWindow::HandleEnableSaveSettingsMenuItem(bool is_enabled)
{
    // Update the Build menu item for "Save settings".
    save_action_->setEnabled(is_enabled);
}

void RgMainWindow::ConnectMenuSignals()
{
    assert(app_state_ != nullptr);
    if (app_state_ != nullptr)
    {
        // Get the API-specific RgBuildView instance used by the current mode.
        RgBuildView* build_view = app_state_->GetBuildView();

        // Verify that the build view exists before using it.
        assert(build_view != nullptr);
        if (build_view != nullptr)
        {
            RgMenu* menu = build_view->GetMenu();
            assert(menu != nullptr);
            if (menu != nullptr)
            {
                // Connect the file menu with the build start event.
                bool is_connected = connect(build_view, &RgBuildView::ProjectBuildStarted, menu, &RgMenu::HandleBuildStarted);
                assert(is_connected);

                // Connect the file menu with the build failure event.
                is_connected = connect(build_view, &RgBuildView::ProjectBuildFailure, menu, &RgMenu::HandleBuildEnded);
                assert(is_connected);

                // Connect the file menu with the build success event.
                is_connected = connect(build_view, &RgBuildView::ProjectBuildSuccess, menu, &RgMenu::HandleBuildEnded);
                assert(is_connected);

                // Connect the file menu with the build canceled event.
                is_connected = connect(build_view, &RgBuildView::ProjectBuildCanceled, menu, &RgMenu::HandleBuildEnded);
                assert(is_connected);

                // Connect the source file change event with the handler.
                is_connected = connect(menu, &RgMenu::SelectedFileChanged, this, &RgMainWindow::HandleSelectedFileChanged);
                assert(is_connected);

                // Connect the file menu's "Build Settings" button to the main window's handler.
                is_connected = connect(menu, &RgMenu::BuildSettingsButtonClicked, this, &RgMainWindow::HandleBuildSettingsEvent);
                assert(is_connected);
            }
        }
    }
}

void RgMainWindow::CreateFileMenuActions()
{
    // Open a project.
    open_project_action_ = new QAction(tr(kStrMenuBarOpenProject), this);
    open_project_action_->setShortcut(QKeySequence(kActionHotkeyOpenProject));
    open_project_action_->setStatusTip(tr(kStrMenuBarOpenProjectTooltip));
    bool is_connected = connect(open_project_action_, &QAction::triggered, this, &RgMainWindow::HandleOpenProjectFileEvent);
    assert(is_connected);

    // Save a file.
    save_action_ = new QAction(tr(kStrMenuBarSaveFile), this);
    save_action_->setShortcuts(QKeySequence::Save);
    save_action_->setStatusTip(tr(kStrMenuBarSaveFileTooltip));
    is_connected = connect(save_action_, &QAction::triggered, this, &RgMainWindow::HandleSaveFileEvent);
    assert(is_connected);

    // Back to home.
    back_to_home_action_ = new QAction(tr(kStrMenuBarBackToHome), this);
    back_to_home_action_->setShortcut(QKeySequence(kActionHotkeyBackToHome));
    back_to_home_action_->setStatusTip(tr(kStrMenuBarBackToHomeTooltip));
    is_connected = connect(back_to_home_action_, &QAction::triggered, this, &RgMainWindow::HandleBackToHomeEvent);
    assert(is_connected);

    // Exit the project.
    exit_action_ = new QAction(tr(kStrMenuBarExit), this);
    exit_action_->setShortcut(QKeySequence(kActionHotkeyExit));
    exit_action_->setStatusTip(tr(kStrMenuBarExitTooltip));
    is_connected = connect(exit_action_, &QAction::triggered, this, &RgMainWindow::HandleExitEvent);
    assert(is_connected);

    // Start with some actions disabled.
    save_action_->setDisabled(true);
}

void RgMainWindow::CreateAppState(RgProjectAPI api)
{
    // Create a factory matching the new API mode to switch to.
    std::shared_ptr<RgFactory> factory = RgFactory::CreateFactory(api);
    assert(factory != nullptr);
    if (factory != nullptr)
    {
        std::shared_ptr<RgAppState> application_state = factory->CreateAppState();
        assert(application_state != nullptr);
        if (application_state != nullptr)
        {
            application_state->SetMainWindow(this);

            // Replace the app state with the one for the new mode.
            app_state_ = application_state;

            // Reset the RgBuildView instance used to interact with the project.
            app_state_->ResetBuildView();

            // Connect signals for the Build View.
            ConnectBuildViewSignals();

            // Set main window stylesheet.
            ApplyMainWindowStylesheet();

            // Use the factory for the new API mode.
            factory_ = factory;
        }
    }
}

void RgMainWindow::ChangeApiMode(RgProjectAPI api)
{
    if (app_state_ != nullptr)
    {
        // Clean up the existing mode before switching to the new one.
        app_state_->Cleanup(menu_bar_);
    }

    // Update the current API value in config manager.
    RgConfigManager& config_manager = RgConfigManager::Instance();
    config_manager.SetCurrentAPI(api);

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
    SwitchToView(MainWindowView::kHome);

    // Set focus to the main tab widget so keyboard shortcuts still work.
    ui_.mainTabWidget->setFocus();
}

void RgMainWindow::DestroyFileMenu()
{
    RG_SAFE_DELETE(menu_bar_);
}

void RgMainWindow::CreateMenuBar()
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
    CreateHelpmenu();
}

void RgMainWindow::CreateFileMenu()
{
    // Add the File menu to the main window's menu bar.
    menu_bar_ = menuBar()->addMenu(tr(kStrMenuBarFile));

    // Add file actions based on the current mode.
    app_state_->CreateFileActions(menu_bar_);

    // Create the actions to be used by the menus.
    CreateFileMenuActions();
    menu_bar_->addAction(open_project_action_);
    menu_bar_->addAction(save_action_);
    menu_bar_->addSeparator();
    menu_bar_->addAction(back_to_home_action_);
    menu_bar_->addSeparator();
    menu_bar_->addAction(exit_action_);

    // Set the mouse cursor to pointing hand cursor.
    menu_bar_->setCursor(Qt::PointingHandCursor);
}

void RgMainWindow::CreateEditMenu()
{
    // Create the actions to be used by the menus.
    CreateEditMenuActions();

    menu_bar_ = menuBar()->addMenu(tr(kStrMenuBarEdit));
    menu_bar_->addAction(go_to_line_action_);
    menu_bar_->addAction(find_action_);
    menu_bar_->addAction(show_max_vgprs_action_);

    // Set the mouse cursor to pointing hand cursor.
    menu_bar_->setCursor(Qt::PointingHandCursor);
}

void RgMainWindow::CreateHelpmenu()
{
    CreateHelpmenuActions();

    menu_bar_ = menuBar()->addMenu(tr(kStrMenuBarHelp));

    // About.
    menu_bar_->addAction(help_about_action_);

    // Help manual.
    menu_bar_->addAction(help_manul_action_);

    // Getting started guide.
    menu_bar_->addAction(help_getting_started_guide_action_);

    // Set the mouse cursor to pointing hand cursor.
    menu_bar_->setCursor(Qt::PointingHandCursor);
}

void RgMainWindow::CreateHelpmenuActions()
{
    // Create and connect the About action.
    help_about_action_ = new QAction(tr(kStrMenuBarHelpAbout), this);
    help_about_action_->setStatusTip(tr(kStrMenuBarHelpAboutTooltip));
    help_about_action_->setShortcut(QKeySequence(kActionHotkeyAbout));
    bool is_connected = connect(help_about_action_, &QAction::triggered, this, &RgMainWindow::HandleAboutEvent);
    assert(is_connected);

    // Create and connect the help manual guide action.
    help_manul_action_ = new QAction(tr(kStrMenuBarHelpManual), this);
    help_manul_action_->setStatusTip(tr(kStrMenuBarHelpManualTooltip));
    help_manul_action_->setShortcut(QKeySequence(kActionHotkeyHelpManual));
    is_connected = connect(help_manul_action_, &QAction::triggered, this, &RgMainWindow::HandleHelpManual);
    assert(is_connected);

    // Create and connect the getting started guide action.
    help_getting_started_guide_action_ = new QAction(tr(kStrMenuBarHelpGettingStartedGuide), this);
    help_getting_started_guide_action_->setStatusTip(tr(kStrMenuBarHelpGettingStartedGuideTooltip));
    is_connected = connect(help_getting_started_guide_action_, &QAction::triggered, this, &RgMainWindow::HandleGettingStartedGuideEvent);
    assert(is_connected);
}

void RgMainWindow::CreateBuildMenuActions()
{
    // Build current project action.
    build_project_action_ = new QAction(tr(kStrMenuBarBuildProject), this);
    build_project_action_->setStatusTip(tr(kStrMenuBarBuildProjectTooltip));
    build_project_action_->setShortcut(QKeySequence(kActionHotkeyBuildProject));
    bool is_connected = connect(build_project_action_, &QAction::triggered, this, &RgMainWindow::HandleBuildProjectEvent);
    assert(is_connected);

    // Open the project's build settings.
    build_settings_action_ = new QAction(tr(kStrMenuBuildSettings), this);
    build_settings_action_->setShortcut(QKeySequence(kActionHotkeyBuildSettings));
    build_settings_action_->setStatusTip(tr(kStrMenuBarBuildSettingsTooltip));
    is_connected = connect(build_settings_action_, SIGNAL(triggered()), this, SLOT(HandleBuildSettingsEvent()));
    assert(is_connected);

    // Open the project's pipeline state editor.
    pipeline_state_action_ = new QAction(tr(kStrMenuPipelineStateEditor), this);
    pipeline_state_action_->setShortcut(QKeySequence(kActionHotkeyPipelineState));
    pipeline_state_action_->setStatusTip(tr(kStrMenuBarPipelineStateTooltip));
    is_connected = connect(pipeline_state_action_, SIGNAL(triggered()), this, SLOT(HandlePipelineStateEvent()));
    assert(is_connected);

    // Cancel the current build.
    cancel_build_action_ = new QAction(tr(kStrMenuCancelBuild), this);
    cancel_build_action_->setShortcut(QKeySequence(kActionHotkeyBuildCancel));
    cancel_build_action_->setStatusTip(tr(kStrMenuBarBuildCancelTooltip));
    is_connected = connect(cancel_build_action_, SIGNAL(triggered()), this, SLOT(HandleCancelBuildEvent()));
    assert(is_connected);
}

void RgMainWindow::CreateBuildMenu()
{
    CreateBuildMenuActions();

    menu_bar_ = menuBar()->addMenu(tr(kStrMenuBarBuild));
    menu_bar_->addAction(build_project_action_);
    assert(app_state_ != nullptr);
    // For graphics APIs, add the pipeline state action.
    if (app_state_ != nullptr && app_state_->IsGraphics())
    {
        menu_bar_->addAction(pipeline_state_action_);
    }
    menu_bar_->addAction(build_settings_action_);
    menu_bar_->addAction(cancel_build_action_);

    // Set the mouse cursor to pointing hand cursor.
    menu_bar_->setCursor(Qt::PointingHandCursor);
}

void RgMainWindow::DestroyBuildView()
{
    // Before destroying the RgBuildView, switch the save mode so that save shortcut signals.
    // Signals will get forwarded to the RgMainWindow instead of the (dead) RgBuildView.
    SwitchSaveShortcut(SaveActionType::kSaveFile);

    // Reset the actions state to default.
    ResetActionsState();

    // Remove the RgBuildView instance from the MainWindow layout.
    assert(app_state_ != nullptr);
    if (app_state_ != nullptr)
    {
        // Remove the RgBuildView instance widget from the MainWindow.
        RgBuildView* build_view = app_state_->GetBuildView();
        ui_.buildPage->layout()->removeWidget(build_view);

        // Reset the app state's RgBuildView instance so that it can be used for a new project.
        app_state_->ResetBuildView();

        // Connect signals for the new RgBuildView instance.
        ConnectBuildViewSignals();

        // Remove the app notification label.
        this->statusBar()->removeWidget(app_notification_widget_);
    }
}

void RgMainWindow::EnableBuildMenu(bool is_enabled)
{
    // Toggle the actions associated with the build menu items.
    build_project_action_->setEnabled(is_enabled);

    // Make sure that the Build and Cancel options are never both enabled.
    if (is_enabled)
    {
        cancel_build_action_->setEnabled(false);
    }
}

void RgMainWindow::CreateEditMenuActions()
{
    // Build current project action.
    go_to_line_action_ = new QAction(tr(kStrMenuBarGoToLine), this);
    go_to_line_action_->setStatusTip(tr(kStrMenuBarGoToLineTooltip));
    go_to_line_action_->setShortcut(QKeySequence(kActionHotkeyGoToLine));
    bool is_connected = connect(go_to_line_action_, &QAction::triggered, this, &RgMainWindow::HandleGoToLineEvent);
    assert(is_connected);

    // Find a string in the source editor.
    find_action_ = new QAction(tr(kStrMenuBarEditQuickFind), this);
    find_action_->setShortcut(QKeySequence(kActionHotkeyFind));
    find_action_->setStatusTip(tr(kStrMenuBarEditQuickFindTooltip));
    is_connected = connect(find_action_, &QAction::triggered, this, &RgMainWindow::FindTriggered);
    assert(is_connected);

    // Show maximum VGPRs.
    show_max_vgprs_action_ = new QAction(tr(kStrMenuShowMaxVgprLines), this);
    show_max_vgprs_action_->setShortcut(QKeySequence(kActionHotkeyShowMaxVgprs));
    show_max_vgprs_action_->setStatusTip(tr(kStrMenuShowMaxVgprLinesTooltip));

    // Connect the show maximum VGPR action.
    is_connected = connect(show_max_vgprs_action_, &QAction::triggered, this, &RgMainWindow::HandleShowMaxVgprsEvent);
    assert(is_connected);
}

void RgMainWindow::EnableEditMenu(bool is_enabled)
{
    assert(app_state_ != nullptr);
    // Toggle the actions associated with the Edit menu items.
    go_to_line_action_->setEnabled(is_enabled);
    find_action_->setEnabled(is_enabled);
    show_max_vgprs_action_->setEnabled(is_enabled);
}

bool RgMainWindow::OpenProjectFileAtPath(const std::string& project_file_path)
{
    bool is_project_loaded = false;

    assert(!project_file_path.empty());
    if (!project_file_path.empty())
    {
        // Read the project file to determine the project's API. The GUI's current mode may have to
        // be changed to match the API used by the project.
        std::shared_ptr<RgProject> project = nullptr;
        is_project_loaded = RgXmlConfigFile::ReadProjectConfigFile(project_file_path, project);

        assert(is_project_loaded);
        assert(project != nullptr);
        if (is_project_loaded && project != nullptr)
        {
            // Change the API mode if the project API doesn't match the current mode.
            RgProjectAPI current_api = RgConfigManager::Instance().GetCurrentAPI();
            if (project->api != current_api)
            {
                ChangeApiMode(project->api);
            }
        }

        assert(app_state_ != nullptr);
        if (app_state_ != nullptr)
        {
            RgBuildView* build_view = app_state_->GetBuildView();
            if (build_view != nullptr)
            {
                // Attempt to load the user's selected project.
                is_project_loaded = build_view->LoadProjectFile(project_file_path);
                assert(is_project_loaded);

                // If the project was loaded correctly, create and populate the RgBuildView with the source files.
                if (is_project_loaded)
                {
                    // Insert the current mode's RgBuildView instance to the main window.
                    bool build_view_added = AddBuildView();

                    if (build_view_added)
                    {
                        // Populate the RgBuildView with the loaded project.
                        bool is_populated = build_view->PopulateBuildView();
                        if (is_populated)
                        {
                            // Show the build view as the central widget.
                            SwitchToView(MainWindowView::kBuildView);

                            // Get the directory where the project file lives.
                            std::string project_directory;
                            bool        is_ok = RgUtils::ExtractFileDirectory(project_file_path, project_directory);
                            assert(is_ok);
                            if (is_ok)
                            {
                                // Try to load existing build output within the project directory.
                                bool is_build_output_loaded = build_view->LoadBuildOutput(project_directory);
                                if (is_build_output_loaded)
                                {
                                    // Previous build outputs were loaded correctly.
                                    // Emit the signal indicating a build has succeeded, which will re-populate the view.
                                    build_view->HandleProjectBuildSuccess();

                                    // Restore the RgBuildView to the last-used layout dimensions.
                                    build_view->RestoreViewLayout();
                                }
                            }
                        }
                        else
                        {
                            // The project wasn't loaded properly. Return to the home page.
                            HandleBackToHomeEvent();
                            is_project_loaded = false;
                        }
                    }
                    else
                    {
                        // Destroy the RgBuildView instance since the project is not being opened.
                        DestroyBuildView();

                         // Reset the window title.
                        ResetWindowTitle();

                        // Reset the status bar.
                        this->statusBar()->showMessage("");
                    }
                }
            }
        }
    }

    return is_project_loaded;
}

void RgMainWindow::SetWindowTitle(const std::string& title_text)
{
    // Add the application name to the title text.
    std::stringstream window_title;
    window_title << kStrAppName << " - ";

    // Determine which API is being used.
    std::string api_name;
    RgProjectAPI project_api = RgConfigManager::Instance().GetCurrentAPI();
    bool is_ok = RgUtils::ProjectAPIToString(project_api, api_name, false);
    assert(is_ok);
    if (is_ok)
    {
        // Add the current API name to the title text.
        window_title << api_name << " ";
        window_title << kStrTitleProject << " - ";
    }

    // Finally append the incoming string to the title text.
    window_title << title_text;
    this->setWindowTitle(window_title.str().c_str());
}

void RgMainWindow::ResetWindowTitle()
{
    // Create the default title with the application name.
    std::stringstream window_title;
    window_title << kStrAppName;

    // Determine which API is being used.
    std::string api_name;
    RgProjectAPI project_api = RgConfigManager::Instance().GetCurrentAPI();
    bool is_ok = RgUtils::ProjectAPIToString(project_api, api_name, false);
    assert(is_ok);
    if (is_ok)
    {
        // Add the current API name to the title text.
        window_title << " - ";
        window_title << api_name << " ";
    }

    // Add the application name to the title text.
    this->setWindowTitle(window_title.str().c_str());
}

void RgMainWindow::HandleHasSettingsPendingStateChanged(bool has_pending_changes)
{
    // Update the File->Save enabled state.
    OnCurrentEditorModificationStateChanged(has_pending_changes);
}

void RgMainWindow::HandleProjectFileCountChanged(bool is_project_empty)
{
    assert(app_state_ != nullptr);
    if (app_state_ != nullptr && app_state_->IsAnalysis())
    {
        return;
    }

    // Toggle the actions associated with the build menu items.
    build_project_action_->setEnabled(!is_project_empty);

    // Make sure that the Build and Cancel options are never both enabled.
    if (!is_project_empty)
    {
        cancel_build_action_->setEnabled(false);
    }

    // In this specific case, make sure that the build settings and, if applicable,
    // the pipeline state items are enabled, since we are in a project (and not in the home page).
    build_settings_action_->setEnabled(true);
    assert(app_state_ != nullptr);
    if (app_state_ != nullptr && app_state_->IsGraphics() &&
        pipeline_state_action_ != nullptr)
    {
        pipeline_state_action_->setEnabled(true);
    }

    // Toggle the availability of the Edit menu.
    go_to_line_action_->setEnabled(!is_project_empty);
    find_action_->setEnabled(!is_project_empty);
}

void RgMainWindow::SwitchToView(MainWindowView tab)
{
    QWidget* widget = nullptr;
    switch (tab)
    {
    case MainWindowView::kHome:
        widget = ui_.homePage;

        // Show the custom status bar so the mode and the API buttons show while on home page.
        assert(status_bar_ != nullptr);
        if (status_bar_ != nullptr)
        {
            status_bar_->SetStatusBarVisibility(true);
            RgUtils::SetToolAndStatusTip("", this);
        }

        // Update the current view.
        current_view_ = tab;

        break;
    case MainWindowView::kBuildView:
        widget = ui_.buildPage;

        // Hide the custom status bar so the mode and the API buttons do not show while in build view.
        assert(status_bar_ != nullptr);
        if (status_bar_ != nullptr)
        {
            status_bar_->SetStatusBarVisibility(false);
        }

        // Set initial focus to file menu.
        if (app_state_ != nullptr)
        {
            RgBuildView* build_view = app_state_->GetBuildView();
            if (build_view != nullptr)
            {
                build_view->FocusOnFileMenu();
            }
        }

        // Update the current view.
        current_view_ = tab;

        break;
    default:
        assert(false);
    }

    if (widget != nullptr)
    {
        ui_.stackedWidget->setCurrentWidget(widget);

        // Enable/disable back to home menu item.
        bool isBackToHomeEnabled = (widget != ui_.homePage);
        back_to_home_action_->setEnabled(isBackToHomeEnabled);
    }
}

void RgMainWindow::HandleOpenProjectFileEvent()
{
    // Ask user to save pending settings.
    bool did_uesr_confirm = true;

    if (!IsInputFileNameBlank())
    {

        assert(settings_tab_ != nullptr);
        if (settings_tab_ != nullptr)
        {
            did_uesr_confirm = settings_tab_->PromptToSavePendingChanges();
        }

        if (did_uesr_confirm)
        {
            if (app_state_ != nullptr)
            {
                did_uesr_confirm = app_state_->ShowProjectSaveDialog();
            }

            if (did_uesr_confirm)
            {
                std::string selected_file;
                bool is_ok = RgUtils::OpenProjectDialog(this, selected_file);

                // Verify that the project name is valid.
                std::string error_string;
                bool is_valid_project_name = RgUtils::IsValidProjectName(selected_file, error_string);
                if (is_valid_project_name && is_ok && !selected_file.empty())
                {
                    // Destroy current BuildView if it has some project open.
                    assert(app_state_ != nullptr);
                    if (app_state_ != nullptr)
                    {
                        RgBuildView* build_view = app_state_->GetBuildView();
                        if (build_view != nullptr && build_view->HasProject())
                        {
                            // Destroy the RgBuildView instance since the project is being closed.
                            DestroyBuildView();
                        }
                    }

                    HandleStatusBarTextChanged(kStrMainWindowLoadingProject, kStatusBarNotificationTimeoutMs);
                    bool is_project_loaded = OpenProjectFileAtPath(selected_file);
                    assert(is_project_loaded);
                    if (is_project_loaded)
                    {
                        HandleStatusBarTextChanged(kStrMainWindowProjectLoadSuccess, kStatusBarNotificationTimeoutMs);
                    }
                    else
                    {
                        HandleStatusBarTextChanged(kStrErrCannotLoadProjectFile, kStatusBarNotificationTimeoutMs);
                    }
                }
                if (!is_valid_project_name && !selected_file.empty())
                {
                    // Display error message box.
                    std::stringstream msg;
                    std::string filename;
                    msg << error_string << " \"";
                    RgUtils::ExtractFileName(selected_file, filename, false);
                    msg << filename << "\".";
                    RgUtils::ShowErrorMessageBox(msg.str().c_str(), this);
                }
            }
        }
    }
}

bool RgMainWindow::IsInputFileNameBlank() const
{
    bool result = false;

    assert(app_state_ != nullptr);
    if (app_state_ != nullptr)
    {
        if (app_state_->IsInputFileNameBlank())
        {
            result = true;
        }
    }

    return result;
}

void RgMainWindow::HandleOpenProjectFileAtPath(const std::string& program_file_path)
{
    // Add a loading message to the status bar.
    HandleStatusBarTextChanged(kStrMainWindowLoadingProject, kStatusBarNotificationTimeoutMs);

    bool is_ok = OpenProjectFileAtPath(program_file_path);

    assert(is_ok);
    if (is_ok)
    {
        HandleStatusBarTextChanged(kStrMainWindowProjectLoadSuccess, kStatusBarNotificationTimeoutMs);
    }
    else
    {
        HandleStatusBarTextChanged(kStrErrCannotLoadProjectFile, kStatusBarNotificationTimeoutMs);
    }
}

void RgMainWindow::HandleSaveFileEvent()
{
    assert(app_state_ != nullptr);
    if (app_state_ != nullptr)
    {
        const int current_index = ui_.mainTabWidget->currentIndex();
        if (current_index == 0)
        {
            // If the RgBuildView exists, forward the save signal.
            RgBuildView* build_view = app_state_->GetBuildView();
            if (build_view != nullptr)
            {
                // Save the current source file or build settings.
                build_view->HandleSaveSettingsButtonClicked();
            }
            else
            {
                if (settings_tab_ != nullptr)
                {
                    settings_tab_->SavePendingChanges();
                }
            }
        }
        else if (current_index == 1)
        {
            if (settings_tab_ != nullptr)
            {
                if (!IsInputFileNameBlank())
                {
                    settings_tab_->SavePendingChanges();
                }
            }
        }
    }
}

void RgMainWindow::HandleBackToHomeEvent()
{
    assert(app_state_ != nullptr);
    if (app_state_ != nullptr)
    {
        RgBuildView* build_view = app_state_->GetBuildView();
        if (build_view != nullptr)
        {
            build_view->SaveProjectConfigFile();

            // Refresh the recent projects list upon returning to the start tab.
            RgStartTab* start_tab = app_state_->GetStartTab();

            assert(start_tab != nullptr);
            if (start_tab != nullptr)
            {
                start_tab->PopulateRecentProjectsList();
            }

            // Ask the user if they want to save all unsaved files.
            bool is_accepted = build_view->RequestRemoveAllFiles();

            // Only exit if the dialog was accepted (ie. the user didn't press cancel).
            if (is_accepted)
            {
                // Switch the view back to the home page.
                SwitchToView(MainWindowView::kHome);

                // Destroy the RgBuildView instance since the project is being closed.
                DestroyBuildView();

                // Reset the window title.
                ResetWindowTitle();

                // Reset the status bar.
                this->statusBar()->showMessage("");
            }
        }
    }
}

void RgMainWindow::HandleExitEvent()
{
    static const int kTopMargin = 8;
    bool is_save_settings_accepted = true;

    // Prompt the user to save any pending changes on SETTINGS tab.

    if (!IsInputFileNameBlank())
    {
        assert(settings_tab_ != nullptr);
        if (settings_tab_ != nullptr)
        {
            is_save_settings_accepted = settings_tab_->PromptToSavePendingChanges();
        }

        assert(app_state_ != nullptr);
        if (app_state_ != nullptr)
        {
            RgBuildView* build_view = app_state_->GetBuildView();
            if (build_view != nullptr)
            {
                // Ask the user if they want to save all unsaved files.
                bool is_accepted = build_view->ShowSaveDialog(RgBuildView::RgFilesToSave::kAll, true);

                // Only exit if the dialog was accepted (ie. the user didn't press cancel).
                if (is_accepted && is_save_settings_accepted)
                {
                    // Save the window geometry.
                    RgConfigManager::Instance().SetWindowGeometry(pos().x(), pos().y() + statusBar()->height() + kTopMargin, size().width(), size().height(), windowState());

                    QApplication::exit(0);
                }
            }
            else
            {
                // Exit out of the application only if the user did NOT press cancel.
                if (is_save_settings_accepted)
                {

                    // Save the window geometry.
                    RgConfigManager::Instance().SetWindowGeometry(pos().x(), pos().y() + statusBar()->height() + kTopMargin, size().width(), size().height(), windowState());

                    QApplication::exit(0);
                }
            }
        }
    }
}

void RgMainWindow::closeEvent(QCloseEvent* event)
{
    // Ignore the close event so we can handle shutdown ourself.
    event->ignore();

    HandleExitEvent();
}

void RgMainWindow::dragEnterEvent(QDragEnterEvent* event)
{
    // Do not let the user drag and drop a file onto the "SETTINGS" tab.
    if (ui_.mainTabWidget->currentIndex() == 1 || current_view_ == MainWindowView::kBuildView)
    {
        event->ignore();
    }
    else
    {
        const QMimeData* mime_data = event->mimeData();

        // Make sure the drop data has a list of file urls.
        if (mime_data->hasUrls())
        {
            // Check to make sure at least one of the files is valid.
            for (QUrl& url : mime_data->urls())
            {
                if (url.isLocalFile())
                {
                    std::string file_path = url.toLocalFile().toStdString();
                    if (RgUtils::IsSourceFileTypeValid(file_path))
                    {
                        // Accept the action, making it so we receive a dropEvent when the items are released.
                        event->setDropAction(Qt::DropAction::CopyAction);
                        event->accept();
                    }
                }
            }
        }
    }
}

void RgMainWindow::dropEvent(QDropEvent *event)
{
    // Get list of all file urls.
    QList<QUrl> file_urls = event->mimeData()->urls();
    // If there is only one file and it is a project file (.rga),
    // call OpenProjectFileAtPath.
    if (file_urls.size() == 1 && file_urls.at(0).isLocalFile() && file_urls.at(0).toLocalFile().endsWith(kStrProjectFileExtension))
    {
        // Verify that the project file name is valid.
        std::string error_message;
        bool is_project_file_valid = RgUtils::IsValidProjectName(file_urls.at(0).toLocalFile().toStdString(), error_message);
        if (is_project_file_valid)
        {
            bool is_load_successful = OpenProjectFileAtPath(file_urls.at(0).toLocalFile().toStdString());
            assert(is_load_successful);
            if (is_load_successful)
            {
                this->statusBar()->showMessage(kStrMainWindowProjectLoadSuccess);
            }
            else
            {
                this->statusBar()->showMessage(kStrErrCannotLoadProjectFile);
            }
        }
        else
        {
            // Show the error message to the user.
            std::stringstream error_stream;
            error_stream << kStrErrIllegalProjectName;
            error_stream << " ";
            error_stream << error_message;
            RgUtils::ShowErrorMessageBox(error_stream.str().c_str(), this);
        }
    }
    else
    {
        // Convert url list to a string list of local file paths.
        QStringList filename_strings;
        bool is_file_valid = false;
        for (QUrl& file_url : file_urls)
        {
            if (file_url.isLocalFile())
            {
                QString filename = file_url.toLocalFile();
                std::string error_message;
                is_file_valid = RgUtils::IsSourceFileTypeValid(filename.toStdString());
                // Filter out all files that aren't valid source files.
                if (is_file_valid)
                {
                    filename_strings.push_back(filename);
                }
            }
        }
        // Disable additional drops and open the desired files in the build view.
        assert(app_state_ != nullptr);
        if (app_state_ != nullptr)
        {
            setAcceptDrops(false);
            if (is_file_valid)
            {
                app_state_->OpenFilesInBuildView(filename_strings);
            }
            setAcceptDrops(true);
        }
    }
    // Set the focus to main window so the user can use keyboard shortcuts.
    setFocus();
}

void RgMainWindow::OnCurrentEditorModificationStateChanged(bool is_modified)
{
    save_action_->setEnabled(is_modified);

    if (save_action_->text().compare(kStrMenuBarSaveFile) == 0)
    {
        // Save the current state of save file shortcut.
        save_file_action_active_ = is_modified;
    }
    else if (save_action_->text().compare(kStrMenuBarSaveSettings) == 0)
    {
        // Save the current state of save settings shortcut.
        save_settings_action_active_ = is_modified;
    }
    else
    {
        // Should not get here.
        assert(false);
    }
}

void RgMainWindow::HandleAboutEvent()
{
    RgAboutDialog aboutDialog(this);

    // Display the About dialog.
    aboutDialog.exec();
}

void RgMainWindow::HandleGettingStartedGuideEvent()
{
    // Build the path to the quickstart guide document.
    static const char* QUICKSTART_GUIDE_RELATIVE_PATH = "/help/rga/quickstart.html";
    QString quickstart_file_path = "file:///";
    QString app_dir_path = qApp->applicationDirPath();
    quickstart_file_path.append(app_dir_path);
    quickstart_file_path.append(QUICKSTART_GUIDE_RELATIVE_PATH);
    QDesktopServices::openUrl(QUrl(quickstart_file_path, QUrl::TolerantMode));
}

void RgMainWindow::HandleHelpManual()
{
    // Build the path to the help manual document.
    static const char* HELP_MANUAL_RELATIVE_PATH = "/help/rga/help_manual.html";
    QString help_manual_file_path = "file:///";
    QString app_dir_path = qApp->applicationDirPath();
    help_manual_file_path.append(app_dir_path);
    help_manual_file_path.append(HELP_MANUAL_RELATIVE_PATH);
    QDesktopServices::openUrl(QUrl(help_manual_file_path, QUrl::TolerantMode));
}

void RgMainWindow::HandleBuildProjectEvent()
{
    assert(app_state_ != nullptr);
    if (app_state_ != nullptr)
    {
        // Emit a signal to indicate view change.
        emit HotKeyPressedSignal();

        RgBuildView* build_view = app_state_->GetBuildView();
        if (build_view != nullptr)
        {
            // Save all source files and settings when a new build is started.
            if (build_view->SaveCurrentState())
            {
                // Now build the project.
                build_view->BuildCurrentProject();
            }
        }
    }
}

void RgMainWindow::HandleBuildSettingsEvent()
{
    // Emit a signal to indicate view change.
    emit HotKeyPressedSignal();

    assert(app_state_ != nullptr);
    if (app_state_ != nullptr)
    {
        RgBuildView* build_view = app_state_->GetBuildView();
        if (build_view != nullptr)
        {
            build_view->OpenBuildSettings();

            // Disable the Build menu item for "Pipeline state".
            pipeline_state_action_->setEnabled(true);

            // Enable the Build menu item for "Build settings".
            build_settings_action_->setEnabled(false);

        }
    }

    // Disable the items in the Edit menu.
    EnableEditMenu(false);
    HandleEnableShowMaxVgprOptionSignal(false);

    // Switch the file menu save file action to save build settings action.
    SwitchSaveShortcut(SaveActionType::kSaveSettings);

    // Update the file menu save action visibility.
    OnCurrentEditorModificationStateChanged(save_settings_action_active_);

    // Give focus to "Build settings" button and remove focus from others.
    RgBuildView* build_view = app_state_->GetBuildView();
    if (build_view != nullptr)
    {
        RgMenu* menu = build_view->GetMenu();
        assert(menu != nullptr);
        if (menu != nullptr)
        {
            menu->DeselectItems();
            auto build_settings_item = menu->GetBuildSettingsItem();
            assert(build_settings_item != nullptr);
            if (build_settings_item != nullptr)
            {
                build_settings_item->SetCurrent(true);
                if (build_settings_item->GetBuildSettingsButton() != nullptr)
                {
                    build_settings_item->GetBuildSettingsButton()->setStyleSheet(kStrButtonFocusInStylesheet);
                }
            }
        }
    }
}

void RgMainWindow::HandlePipelineStateEvent()
{
    // Switch to the pipeline state view.
    assert(app_state_ != nullptr);
    if (app_state_ != nullptr)
    {
        std::shared_ptr<RgAppStateGraphics> graphics_app_state =
            std::static_pointer_cast<RgAppStateGraphics>(app_state_);

        assert(graphics_app_state != nullptr);
        if (graphics_app_state != nullptr)
        {
            graphics_app_state->HandlePipelineStateEvent();

            // Disable the Build menu item for "Pipeline state".
            pipeline_state_action_->setEnabled(false);

            // Enable the Build menu item for "Build settings".
            build_settings_action_->setEnabled(true);

            // Enable the save settings action in Build menu.
            OnCurrentEditorModificationStateChanged(true);
        }
    }
}

void RgMainWindow::HandleCancelBuildEvent()
{
    assert(app_state_ != nullptr);
    if (app_state_ != nullptr)
    {
        RgBuildView* build_view = app_state_->GetBuildView();
        if (build_view != nullptr)
        {
            // Ask the build view to cancel the current build.
            build_view->CancelCurrentBuild();
        }
    }
}

void RgMainWindow::HandleSelectedFileChanged(const std::string& old_file, const std::string& new_file)
{
    Q_UNUSED(old_file);
    Q_UNUSED(new_file);

    assert(app_state_ != nullptr);
    if (app_state_ != nullptr && app_state_->IsAnalysis())
    {
        ;
    }
    else
    {
        // Enable the Edit menu functionality.
        go_to_line_action_->setEnabled(true);
        find_action_->setEnabled(true);
    }
}

void RgMainWindow::HandleProjectCreated()
{
    // Add the RgBuildView instance to the window layout.
    AddBuildView();
}

void RgMainWindow::HandleProjectLoaded(std::shared_ptr<RgProject> project)
{
    assert(project != nullptr);
    if (project != nullptr)
    {
        // Update the application title with the project's name.
        SetWindowTitle(project->project_name);
        assert(project->clones[0] != nullptr);

        // If there are no files in the project, disable source and build related menu items.
        if ((project->clones[0] != nullptr) && (project->IsEmpty()))
        {
            // Disable the build-related actions, except for the build settings action.
            build_project_action_->setEnabled(false);
            build_settings_action_->setEnabled(true);
            assert(app_state_ != nullptr);
            if (app_state_ != nullptr && app_state_->IsGraphics() &&
                pipeline_state_action_ != nullptr)
            {
                pipeline_state_action_->setEnabled(true);
            }
            cancel_build_action_->setEnabled(false);

            // Disable the source-file related actions.
            EnableEditMenu(false);
        }
        emit TEST_ProjectLoaded();
    }
}

void RgMainWindow::HandleCurrentFileModifiedOutsideEnv()
{
    // Notify the user about the file modification through the status bar.
    HandleStatusBarTextChanged(kStrStatusBarFileModifiedOutsideEnv, kStatusBarNotificationTimeoutMs);
}

// Manual handling of focus switching because qt doesn't allow the shortcut to be anything other than "Tab".
void RgMainWindow::HandleFocusNextWidget()
{
    QWidget* focus_widget = QApplication::focusWidget();

    if (focus_widget != nullptr)
    {
        QWidget* test_widget = QApplication::focusWidget()->nextInFocusChain();

        // Step through focus widgets until one is found which accepts focus.
        while (test_widget != nullptr)
        {
            // Check that the widget accepts focus and is visible to the current focus widget.
            if (test_widget->focusPolicy() != Qt::NoFocus &&
                test_widget->isVisibleTo(focus_widget))
            {
                focus_widget = test_widget;
                break;
            }

            // Test next widget.
            test_widget = test_widget->nextInFocusChain();
        }

        // Set focus on the new widget.
        focus_widget->setFocus(Qt::TabFocusReason);
    }
}

// Manual handling of focus switching because qt doesn't allow the shortcut to be anything other than "Tab".
void RgMainWindow::HandleFocusPrevWidget()
{
    QWidget* focus_widget = QApplication::focusWidget();

    if (focus_widget != nullptr)
    {
        QWidget* test_widget = QApplication::focusWidget()->previousInFocusChain();

        // Step through focus widgets until one is found which accepts focus.
        while (test_widget != nullptr)
        {
            // Check that the widget accepts focus and is visible to the current focus widget.
            if (test_widget->focusPolicy() != Qt::NoFocus &&
                test_widget->isVisibleTo(focus_widget))
            {
                focus_widget = test_widget;
                break;
            }

            // Test previous widget.
            test_widget = test_widget->previousInFocusChain();
        }

        // Set focus on the new widget.
        focus_widget->setFocus(Qt::TabFocusReason);
    }
}

void RgMainWindow::HandleProjectBuildFailure()
{
    std::string strStatusBarBuildFailed;
    status_bar_->ConstructStatusMessageString(RgStatusBar::StatusType::kFailed, strStatusBarBuildFailed);

    RgUtils::SetStatusTip(strStatusBarBuildFailed.c_str(), this);

    // Show the build failure message.
    this->statusBar()->showMessage(strStatusBarBuildFailed.c_str());

    // Reset the view's state.
    ResetViewStateAfterBuild();
}

void RgMainWindow::HandleProjectBuildCanceled()
{
    std::string strStatusBarBuildCanceled;
    status_bar_->ConstructStatusMessageString(RgStatusBar::StatusType::kCanceled, strStatusBarBuildCanceled);

    RgUtils::SetStatusTip(strStatusBarBuildCanceled.c_str(), this);

    // Show the build cancellation message.
    this->statusBar()->showMessage(strStatusBarBuildCanceled.c_str());

    // Reset the view's state.
    ResetViewStateAfterBuild();
}

void RgMainWindow::ResetViewStateAfterBuild()
{
    // Reset the build-in-progress flag.
    emit IsBuildInProgress(false);

    // Use the App State interface to reset the view state.
    app_state_->ResetViewStateAfterBuild();

    assert(app_state_ != nullptr);
    if (app_state_ != nullptr && app_state_->IsAnalysis())
    {
        build_project_action_->setEnabled(false);
        build_settings_action_->setEnabled(false);
        cancel_build_action_->setEnabled(false);
        
        go_to_line_action_->setEnabled(false);
        find_action_->setEnabled(false);

        open_project_action_->setEnabled(true);
        back_to_home_action_->setEnabled(true);
    }
    else
    {
        // Re-enable all menu items, since the build is over.
        build_project_action_->setEnabled(true);
        cancel_build_action_->setEnabled(false);
        open_project_action_->setEnabled(true);
        back_to_home_action_->setEnabled(true);
        build_settings_action_->setEnabled(true);
        assert(app_state_ != nullptr);
        if (app_state_ != nullptr && app_state_->IsGraphics() && pipeline_state_action_ != nullptr)
        {
            pipeline_state_action_->setEnabled(true);
        }
    }    

    assert(app_state_ != nullptr);
    if (app_state_ != nullptr)
    {
        RgBuildView* build_view = app_state_->GetBuildView();
        if (build_view != nullptr)
        {
            RgMenu* menu = build_view->GetMenu();
            assert(menu != nullptr);
            if (menu != nullptr)
            {
                // Set the focus back to the source view.
                RgSourceCodeEditor* source_code_editor = build_view->GetEditorForFilepath(menu->GetSelectedFilePath());
                if (source_code_editor != nullptr && source_code_editor->isVisible())
                {
                    source_code_editor->setFocus();
                }

                // Make sure that none of the buttons are highlighted.
                menu->SetButtonsNoFocus();
            }
        }
    }
}

void RgMainWindow::HandleProjectBuildSuccess()
{
    std::string strStatusBarBuildSucceeded;
    status_bar_->ConstructStatusMessageString(RgStatusBar::StatusType::kSucceeded, strStatusBarBuildSucceeded);

    RgUtils::SetStatusTip(strStatusBarBuildSucceeded.c_str(), this);

    this->statusBar()->showMessage(strStatusBarBuildSucceeded.c_str());

    // Reset the view's state.
    ResetViewStateAfterBuild();
}

void RgMainWindow::HandleProjectBuildStarted()
{
    // Mark that a build is now in progress.
    emit IsBuildInProgress(true);

    // Update the status bar.
    this->statusBar()->removeWidget(app_notification_widget_);

    std::string strStatusBarBuildStarted;
    status_bar_->ConstructStatusMessageString(RgStatusBar::StatusType::kStarted, strStatusBarBuildStarted);

    RgUtils::SetStatusTip(strStatusBarBuildStarted.c_str(), this);

    // Do not allow another build while a build is already in progress.
    build_project_action_->setEnabled(false);
    cancel_build_action_->setEnabled(true);

    assert(app_state_ != nullptr);
    if (app_state_ != nullptr)
    {
        app_state_->HandleProjectBuildStarted();
    }

    // Do not allow creating a project.
    open_project_action_->setEnabled(false);

    // Do not allow going back to the home page.
    back_to_home_action_->setEnabled(false);

    // Do not allow going to the build settings or pipeline state view.
    build_settings_action_->setEnabled(false);
    assert(app_state_ != nullptr);
    if (app_state_ != nullptr && app_state_->IsGraphics() &&
        pipeline_state_action_ != nullptr)
    {
        pipeline_state_action_->setEnabled(false);
    }

    assert(app_state_ != nullptr);
    if (app_state_ != nullptr)
    {
        RgBuildView* build_view = app_state_->GetBuildView();
        if (build_view != nullptr)
        {
            // Hide the disassembly view.
            build_view->ToggleDisassemblyViewVisibility(false);
        }
    }
}

void RgMainWindow::HandleUpdateAppNotificationMessage(const std::string& message, const std::string& tooltip)
{
    // First remove the previously added permanent widget.
    if (app_notification_widget_ != nullptr)
    {
        this->statusBar()->removeWidget(app_notification_widget_);
    }

    // Add the new location only if it exists.
    if (!message.empty())
    {
        // Set the notification message.
        CreateAppNotificationMessageLabel(message, tooltip);

        // Insert the notification widget to the status bar.
        this->statusBar()->addPermanentWidget(app_notification_widget_, 0);

        // Show the notification label.
        app_notification_widget_->show();

        // Start the blinking timer.
        const int kInitialBlinkingTimeoutMs = 500;
        app_notification_blinking_timer_->start(kInitialBlinkingTimeoutMs);
    }
}

void RgMainWindow::HandleAppNotificationMessageTimerFired()
{
    // Static counter to count the number of times that this timer has fired.
    static uint32_t count = 0;
    if (count++ % 2 == 0)
    {
        // Even counter: remove notification.
        this->statusBar()->removeWidget(app_notification_widget_);
    }
    else
    {
        // Odd count: insert the notification widget to the status bar.
        this->statusBar()->addPermanentWidget(app_notification_widget_, 0);

        // Show the notification label.
        app_notification_widget_->show();
    }

    // Stop condition.
    if (count % 8 == 0)
    {
        // Condition met: stop blinking.
        count = 0;
        app_notification_blinking_timer_->stop();
    }
    else
    {
        // Continue blinking.
        const int kBlinkingIntervalMs = 800;
        app_notification_blinking_timer_->start(kBlinkingIntervalMs);
    }
}

void RgMainWindow::CreateAppNotificationMessageLabel(const std::string& message, const std::string& tooltip)
{
    // Delete the existing widgets.
    if (app_notification_widget_ != nullptr)
    {
        RG_SAFE_DELETE(app_notification_widget_);
    }

    assert(app_state_ != nullptr);
    if (app_state_ != nullptr)
    {
        // Create a widget to hold the labels.
        app_notification_widget_ = new QWidget();
        app_notification_widget_->setToolTip(QString::fromStdString(tooltip));

        // Create icon and text labels.
        QIcon* icon = new QIcon(kIconResourceRemoveNotification);
        QPixmap pixmap = icon->pixmap(QSize(24, 24));
        QLabel* icon_label = new QLabel(app_notification_widget_);
        icon_label->setPixmap(pixmap);
        QLabel* text_label = new QLabel(QString::fromStdString(message), app_notification_widget_);
        text_label->setStyleSheet("color: white");

        // Set label sizes.
        text_label->setFixedSize(190, 18);
        icon_label->setFixedSize(24, 18);

        // Add labels to a widget and set layout.
        QHBoxLayout* h_layout = new QHBoxLayout(app_notification_widget_);
        h_layout->addWidget(icon_label);
        h_layout->addWidget(text_label);
        h_layout->setContentsMargins(0, 0, 0, 0);
        app_notification_widget_->setLayout(h_layout);
        app_notification_widget_->setContentsMargins(0, 0, 25, 0);
    }
}

void RgMainWindow::HandleStatusBarTextChanged(const std::string& status_bar_text, int timeoutMs)
{
    this->statusBar()->showMessage(status_bar_text.c_str(), timeoutMs);
}

void RgMainWindow::SetCursor()
{
    // Set the cursor to pointing hand cursor.
    menuBar()->setCursor(Qt::PointingHandCursor);
}

void RgMainWindow::HandleShowMaxVgprsEvent()
{
    // Emit the signal to show maximum VGPR lines.
    emit ShowMaximumVgprClickedSignal();
}

void RgMainWindow::HandleGoToLineEvent()
{
    assert(app_state_ != nullptr);
    if (app_state_ != nullptr)
    {
        RgBuildView* build_view = app_state_->GetBuildView();
        if (build_view != nullptr)
        {
            RgMenu* menu = build_view->GetMenu();
            assert(menu != nullptr);
            if (menu != nullptr)
            {
                // Get the max number of lines.
                RgSourceCodeEditor* source_code_editor = build_view->GetEditorForFilepath(menu->GetSelectedFilePath());
                if (source_code_editor != nullptr && source_code_editor->isVisible())
                {
                    int max_line_number = source_code_editor->document()->lineCount();

                    // Create a modal Go To line dialog.
                    RgGoToLineDialog* go_to_line_dialog = new RgGoToLineDialog(max_line_number, this);
                    go_to_line_dialog->setModal(true);
                    go_to_line_dialog->setWindowTitle(kStrGoToLineDialogTitle);

                    // Register the dialog with the scaling manager.
                    ScalingManager::Get().RegisterObject(go_to_line_dialog);

                    // Center the dialog on the view (registering with the scaling manager
                    // shifts it out of the center so we need to manually center it).
                    RgUtils::CenterOnWidget(go_to_line_dialog, this);

                    // Execute the dialog and get the result.
                    RgGoToLineDialog::RgGoToLineDialogResult result;
                    result = static_cast<RgGoToLineDialog::RgGoToLineDialogResult>(go_to_line_dialog->exec());

                    switch (result)
                    {
                    case RgGoToLineDialog::kOk:
                    {
                        // Go to the indicated line number.
                        int line_number = go_to_line_dialog->GetLineNumber();

                        // Scroll the editor to the indicated line.
                        source_code_editor->ScrollToLine(line_number);

                        // Set the highlighted line.
                        QList<int> line_numbers;
                        line_numbers << line_number;
                        source_code_editor->SetHighlightedLines(line_numbers);

                        // Move the cursor as well.
                        QTextCursor cursor(source_code_editor->document()->findBlockByLineNumber(line_number - 1));
                        source_code_editor->setTextCursor(cursor);
                        break;
                    }
                    case RgGoToLineDialog::kCancel:
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
                    delete go_to_line_dialog;
                }
            }
        }
    }
}

void RgMainWindow::HandleSaveSettingsButtonClicked()
{
    assert(settings_tab_ != nullptr);
    if (settings_tab_ != nullptr)
    {
        settings_tab_->SavePendingChanges();
    }
}

void RgMainWindow::HandleTabBarTabChanged(bool save_changes)
{
    if (save_changes)
    {
        HandleSaveSettingsButtonClicked();
    }
    else
    {
        assert(settings_tab_ != nullptr);
        if (settings_tab_ != nullptr)
        {
            settings_tab_->RevertPendingChanges();
        }
    }
}

void RgMainWindow::HandleStatusBarMessageChange(const QString& msg)
{
    // If a build is in progress, the status bar should not be empty,
    // but rather notify the user about the fact that a build is in progress.
    if (msg.isEmpty())
    {
        // Use the RgBuildView to check if there's a build in progress.
        bool is_build_in_progress = false;

        assert(app_state_ != nullptr);
        if (app_state_ != nullptr)
        {
            RgBuildView* build_view = app_state_->GetBuildView();
            if (build_view != nullptr)
            {
                is_build_in_progress = build_view->IsBuildInProgress();
            }
        }

        std::string strStatusBarBuildStarted;
        status_bar_->ConstructStatusMessageString(RgStatusBar::StatusType::kStarted, strStatusBarBuildStarted);

        this->statusBar()->showMessage(is_build_in_progress ? strStatusBarBuildStarted.c_str() : msg);
    }
}

void RgMainWindow::HandleMainTabWidgetTabChanged(int current_index)
{
    // If the user selected "Settings" tab, change the Ctrl+S shortcut
    // to save the settings.
    if (current_index == 0)
    {
        // Switch the save shortcut.
        SwitchSaveShortcut(SaveActionType::kSaveFile);

        // Update the file menu save action visibility.
        OnCurrentEditorModificationStateChanged(save_file_action_active_);
    }
    else if (current_index == 1)
    {
        // Switch the save shortcut.
        SwitchSaveShortcut(SaveActionType::kSaveSettings);

        // Update the file menu save action visibility.
        OnCurrentEditorModificationStateChanged(save_settings_action_active_);
    }

    // Hide the API list widget.
    status_bar_->SetApiListVisibility(false);
}

void RgMainWindow::HandleEditModeChanged(EditMode mode)
{
    switch (mode)
    {
    case (EditMode::kSourceCode):
    {
        // Switch the save shortcut.
        SwitchSaveShortcut(SaveActionType::kSaveFile);

        break;
    }
    case (EditMode::kBuildSettings):
    {
        // Switch the save shortcut.
        SwitchSaveShortcut(SaveActionType::kSaveSettings);

        // Disable the edit menu when viewing the build settings.
        EnableEditMenu(false);

        break;
    }
    case (EditMode::kPipelineSettings):
    {
        // Switch the save shortcut.
        SwitchSaveShortcut(SaveActionType::kSaveSettings);

        // Enable the find functionality.
        find_action_->setEnabled(true);

        // Disable the Go-to-line functionality.
        go_to_line_action_->setEnabled(false);

        // Disable the Show max VGPRs functionality.
        show_max_vgprs_action_->setEnabled(false);

        break;
    }
    default:
        // We shouldn't get here.
        assert(false);
        break;
    }
}

void RgMainWindow::SwitchSaveShortcut(SaveActionType save_action_type)
{
    switch (save_action_type)
    {
    case (SaveActionType::kSaveFile):
    {
        if (save_action_ != nullptr && save_action_->text().compare(kStrMenuBarSaveSettings) == 0)
        {
            // Change the action text and tooltip and restore the "enabled" state.
            save_action_->setText(tr(kStrMenuBarSaveFile));
            save_action_->setStatusTip(tr(kStrMenuBarSaveFileTooltip));
            save_action_->setEnabled(save_file_action_active_);
        }
        break;
    }
    case (SaveActionType::kSaveSettings):
    {
        if (save_action_ != nullptr && save_action_->text().compare(kStrMenuBarSaveFile) == 0)
        {
            // Change the action text and tooltip and restore the "enabled" state.
            save_action_->setText(tr(kStrMenuBarSaveSettings));
            save_action_->setStatusTip(tr(kStrMenuBarSaveSettingsTooltip));
            save_action_->setEnabled(save_settings_action_active_);
        }
        break;
    }
    default:
        // We shouldn't get here.
        assert(false);
        break;
    }
}

void RgMainWindow::ResetActionsState()
{
    assert(app_state_ != nullptr);
    if ((app_state_ != nullptr))
    {
        // Block build-related and source-file-related actions.
        EnableBuildMenu(false);
        build_settings_action_->setEnabled(false);
        if (app_state_->IsGraphics() && pipeline_state_action_ != nullptr)
        {
            pipeline_state_action_->setEnabled(false);
        }
        cancel_build_action_->setEnabled(false);
        EnableEditMenu(false);
    }
}

void RgMainWindow::EnableBuildViewActions()
{
    // Enable the build menu options after the build view has been opened.
    RgBuildView* build_view = app_state_->GetBuildView();
    assert(build_view != nullptr);
    if (build_view != nullptr)
    {
        bool is_project_empty = build_view->GetMenu()->IsEmpty();
        EnableBuildMenu(!is_project_empty);
        build_settings_action_->setEnabled(true);
        assert(app_state_ != nullptr);
        if (app_state_ != nullptr && pipeline_state_action_ != nullptr)
        {
            pipeline_state_action_->setEnabled(true);
        }
        cancel_build_action_->setEnabled(false);

        // Set container switch size action.
        bool is_connected = connect(this, &RgMainWindow::SwitchContainerSize, build_view, &RgBuildView::HandleSwitchContainerSize);
        assert(is_connected);

        // Process the Ctrl+F4 hotkey in disassembly view.
        is_connected = connect(this, &RgMainWindow::ShowMaximumVgprClickedSignal, build_view, &RgBuildView::ShowMaximumVgprClickedSignal);
        assert(is_connected);
    }
}

void RgMainWindow::SetApplicationStylesheet()
{
    // Set application-wide stylesheet.
    std::vector<std::string> stylesheet_file_names;
    app_state_->GetApplicationStylesheet(stylesheet_file_names);
    RgUtils::LoadAndApplyStyle(stylesheet_file_names, qApp);
}

void RgMainWindow::ApplyMainWindowStylesheet()
{
    // Apply main window stylesheet.
    assert(app_state_ != nullptr);
    if (app_state_ != nullptr)
    {
        std::vector<std::string> stylesheet_file_names;
        app_state_->GetMainWindowStylesheet(stylesheet_file_names);
        RgUtils::LoadAndApplyStyle(stylesheet_file_names, this);
    }
}

void RgMainWindow::CreateCustomStatusBar()
{
    assert(factory_ != nullptr);
    if (factory_ != nullptr)
    {
        // Delete any existing custom status bar first.
        if (status_bar_ != nullptr)
        {
            delete status_bar_;
            status_bar_ = nullptr;
        }

        status_bar_ = factory_->CreateStatusBar(statusBar(), this);
        assert(status_bar_ != nullptr);
        if (status_bar_ != nullptr)
        {
            // Set status bar dimensions.
            QSize size;
            size.setHeight(kCustomStatusBarHeight);
            size.setWidth(QWIDGETSIZE_MAX);
            statusBar()->setFixedSize(size);
            status_bar_->setFixedSize(size);

            // Register the status bar with the scaling manager.
            ScalingManager& scaling_manager = ScalingManager::Get();
            scaling_manager.RegisterObject(statusBar());
            scaling_manager.RegisterObject(status_bar_);
            statusBar()->addWidget(status_bar_);

            // Disable the resize grip.
            statusBar()->setSizeGripEnabled(false);

            // Show the custom widget.
            status_bar_->show();

            // Connect the custom status bar's change API mode signal.
            bool is_connected = connect(status_bar_, &RgStatusBar::ChangeAPIModeSignal, this, &RgMainWindow::HandleChangeAPIMode);
            assert(is_connected);

            // Connect the custom status bar's save pending settings changes signal.
            is_connected = connect(status_bar_, &RgStatusBar::SavePendingChanges, this, &RgMainWindow::HandleSavePendingChanges);
            assert(is_connected);
        }
    }
}

bool RgMainWindow::HandleSavePendingChanges()
{
    bool is_not_cancelled = false;
    assert(settings_tab_ != nullptr);
    if (settings_tab_ != nullptr)
    {
        is_not_cancelled = settings_tab_->PromptToSavePendingChanges();
    }

    return is_not_cancelled;
}

void RgMainWindow::HandleChangeAPIMode(RgProjectAPI switch_to_api)
{
    ChangeApiMode(switch_to_api);
}

bool RgMainWindow::eventFilter(QObject* object, QEvent* event)
{
    if (event != nullptr)
    {
        if (event->type() == QEvent::MouseButtonPress)
        {
            // Hide the API list widget when the user clicks somewhere else.
            status_bar_->SetApiListVisibility(false);

            return true;
        }
        else if (event->type() == QEvent::KeyPress)
        {
            QKeyEvent* key_event = static_cast<QKeyEvent*>(event);
            Qt::KeyboardModifiers keyboard_modifiers = QApplication::keyboardModifiers();
            if ((key_event->key() != Qt::Key_Up) && (key_event->key() != Qt::Key_Down))
            {
                // Hide the API list widget when the user presses a key.
                status_bar_->SetApiListVisibility(false);

                if ((keyboard_modifiers & Qt::ControlModifier) && (key_event->key() == Qt::Key_R))
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
            return QObject::eventFilter(object, event);
        }
    }
    else
    {
        // Continue default processing.
        return QObject::eventFilter(object, event);
    }
}

bool RgMainWindow::LoadBinaryCodeObject(const QString filename)
{
    QStringList filename_strings;
    bool        is_file_valid = RgUtils::IsFileExists(filename.toStdString()) && 
        RgUtils::IsSourceFileTypeValid(filename.toStdString());
    if (is_file_valid)
    {
        filename_strings.push_back(filename);

        // Disable additional drops and open the desired files in the build view.
        assert(app_state_ != nullptr);
        if (app_state_ != nullptr)
        {
            setAcceptDrops(false);
            if (is_file_valid)
            {
                app_state_->OpenFilesInBuildView(filename_strings);
            }
            setAcceptDrops(true);
        }

        // Set the focus to main window so the user can use keyboard shortcuts.
        setFocus();
    }
    return is_file_valid;
}

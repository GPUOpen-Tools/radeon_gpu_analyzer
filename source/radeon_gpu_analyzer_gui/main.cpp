// C++.
#include <cassert>
#include <memory>

// Qt.
#include <QtWidgets/QApplication>

// Infra.
#include "QtCommon/Scaling/ScalingManager.h"
#include "source/common/rg_log.h"
#ifdef RGA_GUI_AUTOMATION
#include <rg_test_client_thread.h>
#endif

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_app_state.h"
#include "radeon_gpu_analyzer_gui/qt/rg_main_window.h"
#include "radeon_gpu_analyzer_gui/qt/rg_startup_dialog.h"
#include "radeon_gpu_analyzer_gui/rg_config_manager.h"
#include "radeon_gpu_analyzer_gui/rg_string_constants.h"
#include "radeon_gpu_analyzer_gui/rg_utils.h"
#include "radeon_gpu_analyzer_gui/rg_factory.h"

int main(int argc, char *argv[])
{
    int result = -1;
    static const int kTopMargin = 8;
    const int kExplicitMinWidth = 700;
    const int kExplicitMinHeight = 700;

#ifdef RGA_GUI_AUTOMATION
    // Parse the command line and start Testing Thread.
    RgTestConfig test_config = RgTestClientThread::ParseCmdLine(argc, argv);
    RgTestClientThread  tester(test_config);
    if (test_config.list_tests_ || test_config.test_gui_)
    {
        if (test_config.test_gui_ && test_config.iteration_ == 1)
        {
            // Delete old config & project files.
            RgTestClientThread::DeleteOldFiles();
        }

        // Start the testing thread.
        tester.start();
    }

    // Don't start GUI if test list is requested.
    if (test_config.list_tests_)
    {
        tester.wait();
        return 0;
    }
#endif

    // Start the Qt application.
    QApplication application_instance(argc, argv);

    // Initialize the configuration manager.
    bool is_initialized = RgConfigManager::Instance().Init();
    assert(is_initialized);

    // Get the global settings from the config manager.
    std::shared_ptr<RgGlobalSettings> global_settings = RgConfigManager::Instance().GetGlobalConfig();
    assert(global_settings != nullptr);

    bool enable_feature_interop = QCoreApplication::arguments().count() == 2;
    if (enable_feature_interop)
    {
        RgConfigManager::Instance().SetCurrentAPI(RgProjectAPI::kBinary);
    }
    else
    {
        if (global_settings != nullptr)
        {
            RgProjectAPI selected_api = global_settings->default_api;
            if (global_settings->should_prompt_for_api || selected_api == RgProjectAPI::kUnknown)
            {
                RgStartupDialog startup_dialog;

#ifdef RGA_GUI_AUTOMATION
                tester.StartupDlgCreated(&startup_dialog);
#endif

                if (startup_dialog.exec() == QDialog::Rejected)
                {
                    // exit RGA.
                    exit(0);
                }
                else
                {
                    // Use the API that the user selected.
                    selected_api = startup_dialog.SelectedApi();

                    // If the user doesn't want to be asked again, then set NOT to prompt at startup.
                    RgConfigManager::Instance().SetPromptForAPIAtStartup(!startup_dialog.ShouldNotAskAgain());

                    if (startup_dialog.ShouldNotAskAgain())
                    {
                        // Save the selected API as the new default.
                        RgConfigManager::Instance().SetDefaultAPI(selected_api);

                        // Save the fact that they don't want to be prompted again.
                        RgConfigManager::Instance().SaveGlobalConfigFile();
                    }
                }
            }

            RgConfigManager::Instance().SetCurrentAPI(selected_api);
        }
    }

    // Get the window geometry from config manager.
    RgWindowConfig window_config = {};
    RgConfigManager::Instance().GetWindowGeometry(window_config);

    // Create the main window instance.
    RgMainWindow main_window;

#ifdef RGA_GUI_AUTOMATION
        tester.MainWndCreated(&main_window);
#endif

    // Initialize the main window.
    main_window.InitMainWindow();

    // Update the Y position.
    if (window_config.window_y_pos <= 0)
    {
        window_config.window_y_pos = main_window.statusBar()->height() + kTopMargin;
    }

    // Force an explicit minimum size.
    main_window.setMinimumSize(kExplicitMinWidth, kExplicitMinHeight);

    // Initialize the scaling manager.
    ScalingManager& scaling_manager = ScalingManager::Get();
    scaling_manager.Initialize(&main_window);
    scaling_manager.RegisterAll();

    // Show maximized if either geometry value is zero, or if the window was closed maximized.
    if (window_config.window_width == 0 || window_config.window_height == 0 || window_config.window_state & Qt::WindowMaximized || window_config.window_state & Qt::WindowFullScreen)
    {
        main_window.showMaximized();
    }
    else
    {
        // Set the window geometry and size policy.
        QSizePolicy size_policy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
        main_window.setSizePolicy(size_policy);
        main_window.setGeometry(window_config.window_x_pos, window_config.window_y_pos, window_config.window_width, window_config.window_height);

        // Show main window.
        main_window.show();
    }

    // Before starting the app, check if there is any queued fatal initialization error.
    std::string fatal_error_msg = RgConfigManager::Instance().GetFatalErrorMessage();
    if (!fatal_error_msg.empty())
    {
        // Display the fatal error to the user and exit after the user confirms.
        RgUtils::ShowErrorMessageBox(fatal_error_msg.c_str());
        RgLog::file << fatal_error_msg << std::endl;
        exit(-1);
    }

    if (enable_feature_interop)
    {
        if (!main_window.LoadBinaryCodeObject(QCoreApplication::arguments().at(1)))
        {
            // Display the fatal error to the user and exit after the user confirms.
            RgUtils::ShowErrorMessageBox(kStrLogCannotLoadBinaryCodeObject);
            RgLog::file << kStrLogCannotLoadBinaryCodeObject << std::endl;
            exit(-1);
        }
    }

    result = application_instance.exec();
    RgConfigManager::Instance().Close();

    return result;
}

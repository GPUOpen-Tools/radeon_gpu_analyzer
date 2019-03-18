// C++.
#include <cassert>
#include <memory>

// Qt.
#include <QtWidgets/QApplication>

// Infra.
#include <QtCommon/Scaling/ScalingManager.h>
#include <RadeonGPUAnalyzerGUI/../Utils/Include/rgLog.h>
#ifdef RGA_GUI_AUTOMATION
#include <rgTestClientThread.h>
#endif

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgAppState.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMainWindow.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgStartupDialog.h>
#include <RadeonGPUAnalyzerGUI/Include/rgConfigManager.h>
#include <RadeonGPUAnalyzerGUI/Include/rgStringConstants.h>
#include <RadeonGPUAnalyzerGUI/Include/rgUtils.h>
#include <RadeonGPUAnalyzerGUI/Include/rgFactory.h>

int main(int argc, char *argv[])
{
    int result = -1;
    const int EXPLICIT_MIN_WIDTH = 700;
    const int EXPLICIT_MIN_HEIGHT = 700;

#ifdef RGA_GUI_AUTOMATION
    // Parse the command line and start Testing Thread.
    rgTestConfig testConfig = rgTestClientThread::ParseCmdLine(argc, argv);
    rgTestClientThread  tester(testConfig);
    if (testConfig.m_listTests || testConfig.m_testGui)
    {
        if (testConfig.m_testGui && testConfig.m_iteration == 1)
        {
            // Delete old config & project files.
            rgTestClientThread::DeleteOldFiles();
        }

        // Start the testing thread.
        tester.start();
    }

    // Don't start GUI if test list is requested.
    if (testConfig.m_listTests)
    {
        tester.wait();
        return 0;
    }
#endif

    // Initialize the configuration manager.
    bool isInitialized = rgConfigManager::Instance().Init();
    assert(isInitialized);

    // Get the global settings from the config manager.
    std::shared_ptr<rgGlobalSettings> pGlobalSettings = rgConfigManager::Instance().GetGlobalConfig();
    assert(pGlobalSettings != nullptr);

    // Start the Qt application.
    QApplication applicationInstance(argc, argv);

    if (pGlobalSettings != nullptr)
    {
        rgProjectAPI selectedApi = pGlobalSettings->m_defaultAPI;
        if (pGlobalSettings->m_shouldPromptForAPI || selectedApi == rgProjectAPI::Unknown)
        {
            rgStartupDialog startupDialog;

#ifdef RGA_GUI_AUTOMATION
            tester.StartupDlgCreated(&startupDialog);
#endif
            int result = startupDialog.exec();

            if (result == QDialog::Rejected)
            {
                // exit RGA.
                exit(0);
            }
            else
            {
                // Use the API that the user selected.
                selectedApi = startupDialog.SelectedApi();

                // If the user doesn't want to be asked again, then set NOT to prompt at startup.
                rgConfigManager::Instance().SetPromptForAPIAtStartup(!startupDialog.ShouldNotAskAgain());

                if (startupDialog.ShouldNotAskAgain())
                {
                    // Save the selected API as the new default.
                    rgConfigManager::Instance().SetDefaultAPI(selectedApi);

                    // Save the fact that they don't want to be prompted again.
                    rgConfigManager::Instance().SaveGlobalConfigFile();
                }
            }
        }

        rgConfigManager::Instance().SetCurrentAPI(selectedApi);
    }

    // Create the main window instance.
    rgMainWindow mainWindow;

#ifdef RGA_GUI_AUTOMATION
        tester.MainWndCreated(&mainWindow);
#endif

    // Initialize the main window.
    mainWindow.InitMainWindow();

    // Force an explicit minimum size.
    mainWindow.setMinimumSize(EXPLICIT_MIN_WIDTH, EXPLICIT_MIN_HEIGHT);
    mainWindow.show();

    // Initialize the scaling manager.
    ScalingManager& scalingManager = ScalingManager::Get();
    scalingManager.Initialize(&mainWindow);
    scalingManager.RegisterAll();

    // Show maximized after scaling operations
    mainWindow.showMaximized();

    // Before starting the app, check if there is any queued fatal initialization error.
    std::string fatalErrorMsg = rgConfigManager::Instance().GetFatalErrorMessage();
    if (!fatalErrorMsg.empty())
    {
        // Display the fatal error to the user and exit after the user confirms.
        rgUtils::ShowErrorMessageBox(fatalErrorMsg.c_str());
        rgLog::file << fatalErrorMsg << std::endl;
        exit(-1);
    }

    result = applicationInstance.exec();
    rgConfigManager::Instance().Close();

    return result;
}

// C++.
#include <cassert>
#include <memory>

// Qt.
#include <QtWidgets/QApplication>

// Infra.
#include <QtCommon/Scaling/ScalingManager.h>
#include <RadeonGPUAnalyzerGUI/../Utils/include/rgLog.h>

// Local.
#include <RadeonGPUAnalyzerGUI/include/qt/rgMainWindow.h>
#include <RadeonGPUAnalyzerGUI/include/rgConfigManager.h>
#include <RadeonGPUAnalyzerGUI/include/rgStringConstants.h>
#include <RadeonGPUAnalyzerGUI/include/rgUtils.h>

int main(int argc, char *argv[])
{
    const int EXPLICIT_MIN_WIDTH = 700;
    const int EXPLICIT_MIN_HEIGHT = 700;

    // Initialize the configuration manager.
    bool isInitialized = rgConfigManager::Instance().Init();
    assert(isInitialized);

    // Initialize the configuration manager.
    std::shared_ptr<rgGlobalSettings> pGlobalSettings = rgConfigManager::Instance().GetGlobalConfig();

    // Start the Qt application.
    QApplication applicationInstance(argc, argv);
    rgMainWindow mainWindow;

    // Force an explicit minimum size.
    mainWindow.setMinimumSize(EXPLICIT_MIN_WIDTH, EXPLICIT_MIN_HEIGHT);
    mainWindow.show();

    // Initialize the scaling manager.
    ScalingManager& scalingManager = ScalingManager::Get();
    scalingManager.Initialize(&mainWindow);
    scalingManager.RegisterAll();

    // Show maximized after scaling operations
    mainWindow.showMaximized();

    // Set application-wide stylesheet.
    rgUtils::LoadAndApplyStyle(STR_APPLICATION_STYLESHEET_FILE, &applicationInstance);

    // Before starting the app, check if there is any queued fatal initialization error.
    std::string fatalErrorMsg = rgConfigManager::Instance().GetFatalErrorMessage();
    if (!fatalErrorMsg.empty())
    {
        // Display the fatal error to the user and exit after the user confirms.
        rgUtils::ShowErrorMessageBox(fatalErrorMsg.c_str());
        rgLog::file << fatalErrorMsg << std::endl;
        exit(-1);
    }

    int result = applicationInstance.exec();

    rgConfigManager::Instance().Close();

    return result;
}

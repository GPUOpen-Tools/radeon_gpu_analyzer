// C++.
#include <sstream>
#include <cassert>

// Infra.
#include <QtCommon/Scaling/ScalingManager.h>

// Local.
#include <RadeonGPUAnalyzerGUI/include/rgDataTypes.h>
#include <RadeonGPUAnalyzerGUI/include/qt/rgAboutDialog.h>
#include <RadeonGPUAnalyzerGUI/include/rgConfigManager.h>

// Shared between CLI and GUI.
#include <RadeonGPUAnalyzerGUI/../Utils/include/rgaVersionInfo.h>
#include <RadeonGPUAnalyzerGUI/../Utils/include/rgaSharedUtils.h>


rgAboutDialog::rgAboutDialog(QWidget* pParent) : QDialog(pParent)
{
    ui.setupUi(this);

    // Set the size to fixed.
    QSize size;
    size.setWidth(300 * ScalingManager::Get().GetScaleFactor());
    size.setHeight(125 * ScalingManager::Get().GetScaleFactor());
    setFixedSize(size);

    // Disable the help button in the title bar, and disable resizing of this dialog.
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint | Qt::MSWindowsFixedSizeDialogHint);

    // Get the CLI version info.
    std::shared_ptr<rgCliVersionInfo> pCliVersion = rgConfigManager::Instance().GetVersionInfo();

    // Build the version strings for the CLI.
    if(pCliVersion != nullptr)
    {
        std::stringstream cliVersionString;
        cliVersionString << ui.CliVersionLabel->text().toStdString() << " " <<
            pCliVersion->m_version << " (" << pCliVersion->m_buildDate << ")";
        ui.CliVersionLabel->setText(cliVersionString.str().c_str());
    }

    // Build the version string for the GUI.
    std::stringstream guiVersionString;

    // Convert the date string.
    std::string guiDateString(STR_RGA_BUILD_DATE);
    bool isConverted = rgaSharedUtils::ConvertDateString(guiDateString);
    assert(isConverted);

    guiVersionString << ui.ApplicationVersionLabel->text().toStdString() << " " <<
        STR_RGA_VERSION << "." << STR_RGA_BUILD_NUM << " (" << guiDateString << ")";
    ui.ApplicationVersionLabel->setText(guiVersionString.str().c_str());
}

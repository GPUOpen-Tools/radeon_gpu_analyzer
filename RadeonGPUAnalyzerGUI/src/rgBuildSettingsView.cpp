// C++.
#include <sstream>

// Local.
#include <RadeonGPUAnalyzerGUI/include/qt/rgBuildSettingsView.h>
#include <RadeonGPUAnalyzerGUI/include/rgStringConstants.h>

rgBuildSettingsView::rgBuildSettingsView(QWidget* pParent, bool isGlobalSettings) :
    QWidget(pParent),
    m_isGlobalSettings(isGlobalSettings)
{
}
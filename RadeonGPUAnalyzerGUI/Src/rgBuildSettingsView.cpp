// C++.
#include <sstream>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgBuildSettingsView.h>
#include <RadeonGPUAnalyzerGUI/Include/rgStringConstants.h>

rgBuildSettingsView::rgBuildSettingsView(QWidget* pParent, bool isGlobalSettings) :
    rgSettingsView(pParent),
    m_isGlobalSettings(isGlobalSettings)
{
}

void rgBuildSettingsView::SetHasPendingChanges(bool hasPendingChanges)
{
    // Only emit the signal if the state of the pending changes is different
    // than it was before.
    if (m_hasPendingChanges != hasPendingChanges)
    {
        m_hasPendingChanges = hasPendingChanges;

        emit PendingChangesStateChanged(m_hasPendingChanges);
    }
}

// C++.
#include <cassert>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgAppState.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgSettingsTab.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgStartTab.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgBuildView.h>

void rgAppState::CreateFileActions(QMenu* pMenubar)
{
    // Create API-specific file menu actions.
    CreateApiSpecificFileActions(pMenubar);

    // Connect file menu actions.
    ConnectFileMenuActions();
}

void rgAppState::SetMainWindow(rgMainWindow* pMainWindow)
{
    m_pMainWindow = pMainWindow;
}

void rgAppState::SetSettingsTab(rgSettingsTab* pSettingsTab)
{
    m_pSettingsTab = pSettingsTab;
}

bool rgAppState::ShowProjectSaveDialog()
{
    bool shouldActionContinue = true;
    rgBuildView* pBuildView = GetBuildView();
    if (pBuildView != nullptr)
    {
        shouldActionContinue = pBuildView->ShowSaveDialog();
    }

    return shouldActionContinue;
}

bool rgAppState::IsGraphics() const
{
    return m_isGraphics;
}

rgAppStateGraphics::rgAppStateGraphics()
{
    // Set the graphics flag to true.
    m_isGraphics = true;
}

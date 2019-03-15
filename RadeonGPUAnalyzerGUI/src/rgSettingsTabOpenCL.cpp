// C++.
#include <cassert>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgSettingsTabOpenCL.h>

rgSettingsTabOpenCL::rgSettingsTabOpenCL(QWidget* pParent)
    : rgSettingsTab(pParent)
{
}

rgProjectAPI rgSettingsTabOpenCL::GetApiType()
{
    return rgProjectAPI::OpenCL;
}

// C++.
#include <cassert>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgSettingsTabVulkan.h>
#include <RadeonGPUAnalyzerGUI/Include/rgFactory.h>

rgSettingsTabVulkan::rgSettingsTabVulkan(QWidget* pParent)
    : rgSettingsTab(pParent)
{
}

rgProjectAPI rgSettingsTabVulkan::GetApiType()
{
    return rgProjectAPI::Vulkan;
}

// C++.
#include <cassert>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_settings_tab_vulkan.h"
#include "radeon_gpu_analyzer_gui/rg_factory.h"

RgSettingsTabVulkan::RgSettingsTabVulkan(QWidget* parent)
    : RgSettingsTab(parent)
{
}

RgProjectAPI RgSettingsTabVulkan::GetApiType()
{
    return RgProjectAPI::kVulkan;
}

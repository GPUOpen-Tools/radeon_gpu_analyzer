// C++.
#include <cassert>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_settings_tab_binary.h"

RgSettingsTabBinary::RgSettingsTabBinary(QWidget* parent)
    : RgSettingsTab(parent)
{
}

RgProjectAPI RgSettingsTabBinary::GetApiType()
{
    return RgProjectAPI::kBinary;
}

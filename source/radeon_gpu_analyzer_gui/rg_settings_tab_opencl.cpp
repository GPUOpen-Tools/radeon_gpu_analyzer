// C++.
#include <cassert>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_settings_tab_opencl.h"

RgSettingsTabOpencl::RgSettingsTabOpencl(QWidget* parent)
    : RgSettingsTab(parent)
{
}

RgProjectAPI RgSettingsTabOpencl::GetApiType()
{
    return RgProjectAPI::kOpenCL;
}

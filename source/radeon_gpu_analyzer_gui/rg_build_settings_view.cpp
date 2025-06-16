//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for Build settings view.
//=============================================================================

// C++.
#include <sstream>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_build_settings_view.h"
#include "radeon_gpu_analyzer_gui/rg_string_constants.h"

RgBuildSettingsView::RgBuildSettingsView(QWidget* parent, bool is_global_settings) :
    RgSettingsView(parent),
    is_global_settings_(is_global_settings)
{
}

void RgBuildSettingsView::SetHasPendingChanges(bool has_pending_changes)
{
    // Only emit the signal if the state of the pending changes is different
    // than it was before.
    if (has_pending_changes_ != has_pending_changes)
    {
        has_pending_changes_ = has_pending_changes;

        emit PendingChangesStateChanged(has_pending_changes_);
    }
}

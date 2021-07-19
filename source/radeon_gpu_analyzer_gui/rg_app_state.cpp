// C++.
#include <cassert>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_app_state.h"
#include "radeon_gpu_analyzer_gui/qt/rg_global_settings_view.h"
#include "radeon_gpu_analyzer_gui/qt/rg_settings_tab.h"
#include "radeon_gpu_analyzer_gui/qt/rg_start_tab.h"
#include "radeon_gpu_analyzer_gui/qt/rg_build_view.h"

void RgAppState::CreateFileActions(QMenu* menu_bar)
{
    // Create API-specific file menu actions.
    CreateApiSpecificFileActions(menu_bar);

    // Connect file menu actions.
    ConnectFileMenuActions();
}

void RgAppState::SetMainWindow(RgMainWindow* main_window)
{
    main_window_ = main_window;
}

void RgAppState::SetSettingsTab(RgSettingsTab* settings_tab)
{
    settings_tab_ = settings_tab;
}

bool RgAppState::ShowProjectSaveDialog()
{
    bool should_action_continue = true;
    RgBuildView* build_view = GetBuildView();
    if (build_view != nullptr)
    {
        should_action_continue = build_view->ShowSaveDialog();
    }

    return should_action_continue;
}

bool RgAppState::IsGraphics() const
{
    return is_graphics_;
}

RgAppStateGraphics::RgAppStateGraphics()
{
    // Set the graphics flag to true.
    is_graphics_ = true;
}

bool RgAppState::IsInputFileNameBlank() const
{
    bool result = false;

    // Handle blank input file in global settings view.
    RgGlobalSettingsView* global_settings = settings_tab_->GetGlobalSettingsView();
    if (global_settings != nullptr)
    {
        bool is_input_file_name_blank = global_settings->IsInputFileBlank();
        if (is_input_file_name_blank)
        {
            global_settings->ProcessInputFileBlank();
            result = true;
        }
    }

    return result;
}

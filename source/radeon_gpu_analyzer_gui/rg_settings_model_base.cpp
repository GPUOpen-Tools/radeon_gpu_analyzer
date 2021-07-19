// C++.
#include <cassert>
#include <sstream>

// Local.
#include "radeon_gpu_analyzer_gui/rg_utils.h"
#include "radeon_gpu_analyzer_gui/qt/rg_settings_model_base.h"
#include "radeon_gpu_analyzer_gui/rg_data_types.h"
#include "radeon_gpu_analyzer_gui/rg_string_constants.h"

RgSettingsModelBase::RgSettingsModelBase(unsigned int model_count) :
    ModelViewMapper(model_count) {}

void RgSettingsModelBase::RevertPendingChanges()
{
    // Clear the map of pending changes to the settings.
    pending_values_.clear();

    // After submitting the user's pending changes, the interface has no changes pending.
    SetHasPendingChanges(false);

    // Reset the build settings to the current settings, which will update the UI.
    InitializeModelValues();
}

bool RgSettingsModelBase::HasPendingChanges() const
{
    return has_pending_changes_;
}

void RgSettingsModelBase::InitializeModelValues()
{
    // Initialize the model values.
    InitializeModelValuesImpl();

    // Since the model data has just been initialized for the first time, it is in a "dirty" state.
    // Reset the model's pending changes so that the Build Settings aren't displayed
    SetHasPendingChanges(false);
}

void RgSettingsModelBase::SetHasPendingChanges(bool has_pending_changes)
{
    // Only confirm the state change if it's different from the current state.
    if (has_pending_changes_ != has_pending_changes)
    {
        has_pending_changes_ = has_pending_changes;

        // Emit a signal that the dirtiness of the model has changed.
        emit PendingChangesStateChanged(has_pending_changes);
    }
}

void RgSettingsModelBase::SubmitPendingChanges()
{
    // Step through each control and update the displayed value if necessary.
    uint32_t model_count = GetModelCount();
    for (uint32_t control_index = 0; control_index < model_count; ++control_index)
    {
        auto control_value = pending_values_.find(control_index);
        if (control_value != pending_values_.end())
        {
            UpdateModelValue(control_index, control_value->second, true);
        }
    }

    // After submitting the changes, clear all of the user's pending changes.
    RevertPendingChanges();
}

bool RgSettingsModelBase::GetHasPendingChanges() const
{
    bool has_pending_changes = false;

    // Step through the initial values, and compare to pending values.
    auto initial_values_start_iter = initial_values_.begin();
    auto initial_values_end_iter = initial_values_.end();
    for (auto initial_values_iter = initial_values_start_iter; initial_values_iter != initial_values_end_iter; ++initial_values_iter)
    {
        // Extract the initial value for the control.
        int model_index = initial_values_iter->first;
        const QVariant& initial_value = initial_values_iter->second;

        // Attempt to find a pending value for the control.
        auto pending_value_iter = pending_values_.find(model_index);
        if (pending_value_iter != pending_values_.end())
        {
            const QVariant& pending_value = pending_value_iter->second;

            // If the original and pending values match, there is no pending change.
            if (initial_value != pending_value)
            {
                has_pending_changes = true;
                break;
            }
        }
    }

    return has_pending_changes;
}

bool RgSettingsModelBase::GetPendingValue(int control, QVariant& value) const
{
    bool ret = false;

    auto pending_value_iter = pending_values_.find(control);
    if (pending_value_iter != pending_values_.end())
    {
        ret = true;
        value = pending_value_iter->second;
    }

    return ret;
}

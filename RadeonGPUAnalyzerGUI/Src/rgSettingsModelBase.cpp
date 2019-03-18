// C++.
#include <cassert>
#include <sstream>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/rgUtils.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgSettingsModelBase.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDataTypes.h>
#include <RadeonGPUAnalyzerGUI/Include/rgStringConstants.h>

rgSettingsModelBase::rgSettingsModelBase(unsigned int modelCount) :
    ModelViewMapper(modelCount) {}

void rgSettingsModelBase::RevertPendingChanges()
{
    // Clear the map of pending changes to the settings.
    m_pendingValues.clear();

    // After submitting the user's pending changes, the interface has no changes pending.
    SetHasPendingChanges(false);

    // Reset the build settings to the current settings, which will update the UI.
    InitializeModelValues();
}

bool rgSettingsModelBase::HasPendingChanges() const
{
    return m_hasPendingChanges;
}

void rgSettingsModelBase::InitializeModelValues()
{
    // Initialize the model values.
    InitializeModelValuesImpl();

    // Since the model data has just been initialized for the first time, it is in a "dirty" state.
    // Reset the model's pending changes so that the Build Settings aren't displayed
    SetHasPendingChanges(false);
}

void rgSettingsModelBase::SetHasPendingChanges(bool hasPendingChanges)
{
    // Only confirm the state change if it's different from the current state.
    if (m_hasPendingChanges != hasPendingChanges)
    {
        m_hasPendingChanges = hasPendingChanges;

        // Emit a signal that the dirtiness of the model has changed.
        emit PendingChangesStateChanged(hasPendingChanges);
    }
}

void rgSettingsModelBase::SubmitPendingChanges()
{
    // Step through each control and update the displayed value if necessary.
    uint32_t modelCount = GetModelCount();
    for (uint32_t controlIndex = 0; controlIndex < modelCount; ++controlIndex)
    {
        auto controlValue = m_pendingValues.find(controlIndex);
        if (controlValue != m_pendingValues.end())
        {
            UpdateModelValue(controlIndex, controlValue->second, true);
        }
    }

    // After submitting the changes, clear all of the user's pending changes.
    RevertPendingChanges();
}

bool rgSettingsModelBase::GetHasPendingChanges() const
{
    bool hasPendingChanges = false;

    // Step through the initial values, and compare to pending values.
    auto initialValuesStartIter = m_initialValues.begin();
    auto initialValuesEndIter = m_initialValues.end();
    for (auto initialValuesIter = initialValuesStartIter; initialValuesIter != initialValuesEndIter; ++initialValuesIter)
    {
        // Extract the initial value for the control.
        int modelIndex = initialValuesIter->first;
        const QVariant& initialValue = initialValuesIter->second;

        // Attempt to find a pending value for the control.
        auto pendingValueIter = m_pendingValues.find(modelIndex);
        if (pendingValueIter != m_pendingValues.end())
        {
            const QVariant& pendingValue = pendingValueIter->second;

            // If the original and pending values match, there is no pending change.
            if (initialValue != pendingValue)
            {
                hasPendingChanges = true;
                break;
            }
        }
    }

    return hasPendingChanges;
}

bool rgSettingsModelBase::GetPendingValue(int control, QVariant& value) const
{
    bool ret = false;

    auto pendingValueIter = m_pendingValues.find(control);
    if (pendingValueIter != m_pendingValues.end())
    {
        ret = true;
        value = pendingValueIter->second;
    }

    return ret;
}
#pragma once

// C++.
#include <map>

// Qt.
#include <QVariant>

// Infra.
#include <QtCommon/Util/ModelViewMapper.h>

// A structure containing field validation failure info.
struct rgInvalidFieldInfo
{
    // The target control with an invalid value.
    int m_targetControl;

    // A string describing what's wrong with the value.
    std::string m_errorString;
};

class rgSettingsModelBase : public ModelViewMapper
{
    Q_OBJECT
public:
    rgSettingsModelBase(unsigned int modelCount);
    virtual ~rgSettingsModelBase() = default;

    // Revert all pending changes.
    void RevertPendingChanges();

    // Return true if the settings model has any pending changes.
    bool HasPendingChanges() const;

    // Initialize the model values based on the current build settings.
    void InitializeModelValues();

    // Set the flag indicating if the user has unsaved changes.
    void SetHasPendingChanges(bool isUnsaved);

    // Replace the user's altered settings into the program instance.
    void SubmitPendingChanges();

    // A model-specific implementation used to update a model value.
    virtual void UpdateModelValue(int control, const QVariant& value, bool updateInitialValue) = 0;

    // Verify the validity of all pending settings. Fill the incoming vector with error info.
    virtual bool ValidatePendingSettings(std::vector<rgInvalidFieldInfo>& errorFields) = 0;

signals:
    // A signal emitted when the pending changes state for the model changes.
    void PendingChangesStateChanged(bool hasPendingChanges);

protected:
    // A model-specific implementation to initialize all model values.
    virtual void InitializeModelValuesImpl() = 0;

    // Check if the interface has pending changes that haven't yet been saved.
    bool GetHasPendingChanges() const;

    // Get the pending value of a target control.
    bool GetPendingValue(int control, QVariant& value) const;

    // A map of control to the original saved value.
    std::map<int, QVariant> m_initialValues;

    // A map of control to the user's altered value.
    std::map<int, QVariant> m_pendingValues;

    // A flag that indicates if the settings have been changed, but not yet saved.
    bool m_hasPendingChanges = false;
};
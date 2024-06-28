#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_SETTINGS_MODEL_BASE_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_SETTINGS_MODEL_BASE_H_

// C++.
#include <map>

// Qt.
#include <QVariant>

// Infra.
#include "qt_common/utils/model_view_mapper.h"

// A structure containing field validation failure info.
struct RgInvalidFieldInfo
{
    // The target control with an invalid value.
    int target_control;

    // A string describing what's wrong with the value.
    std::string error_string;
};

class RgSettingsModelBase : public ModelViewMapper
{
    Q_OBJECT
public:
    RgSettingsModelBase(unsigned int model_count);
    virtual ~RgSettingsModelBase() = default;

    // Revert all pending changes.
    void RevertPendingChanges();

    // Return true if the settings model has any pending changes.
    bool HasPendingChanges() const;

    // Initialize the model values based on the current build settings.
    void InitializeModelValues();

    // Set the flag indicating if the user has unsaved changes.
    void SetHasPendingChanges(bool is_unsaved);

    // Replace the user's altered settings into the program instance.
    void SubmitPendingChanges();

    // A model-specific implementation used to update a model value.
    virtual void UpdateModelValue(int control, const QVariant& value, bool update_initial_value) = 0;

    // Verify the validity of all pending settings. Fill the incoming vector with error info.
    virtual bool ValidatePendingSettings(std::vector<RgInvalidFieldInfo>& error_fields) = 0;

signals:
    // A signal emitted when the pending changes state for the model changes.
    void PendingChangesStateChanged(bool has_pending_changes);

protected:
    // A model-specific implementation to initialize all model values.
    virtual void InitializeModelValuesImpl() = 0;

    // Check if the interface has pending changes that haven't yet been saved.
    bool GetHasPendingChanges() const;

    // Get the pending value of a target control.
    bool GetPendingValue(int control, QVariant& value) const;

    // A map of control to the original saved value.
    std::map<int, QVariant> initial_values_;

    // A map of control to the user's altered value.
    std::map<int, QVariant> pending_values_;

    // A flag that indicates if the settings have been changed, but not yet saved.
    bool has_pending_changes_ = false;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_SETTINGS_MODEL_BASE_H_

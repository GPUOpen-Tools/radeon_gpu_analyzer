#pragma once

// C++.
#include <map>

// Qt.
#include <QVariant>

// Local.
#include <RadeonGPUAnalyzerGUI/include/qt/rgSettingsModelBase.h>

// Forward declarations.
struct rgGlobalSettings;

enum rgGlobalSettingsControls
{
    LogFileLocation,
    DisassemblyViewColumns,
    AlwaysUseGeneratedProjectNames,
    Count
};

class rgGlobalSettingsModel : public rgSettingsModelBase
{
    Q_OBJECT
public:
    rgGlobalSettingsModel(std::shared_ptr<rgGlobalSettings> pBuildSettings, unsigned int modelCount);
    virtual ~rgGlobalSettingsModel() = default;

    // Retrieve the current user-configured build settings.
    std::shared_ptr<rgGlobalSettings> GetGlobalSettings() const;

    // Retrieve the pending build settings. Pending changes differ from the original values, but have not yet been saved by the user.
    std::shared_ptr<rgGlobalSettings> GetPendingGlobalSettings() const;

    // Set the build settings
    void SetBuildSettings(std::shared_ptr<rgGlobalSettings> pBuildSettings);

    // Replace the user's altered settings into the program instance.
    void SubmitPendingChanges();

    // A model-specific implementation used to update a model value.
    virtual void UpdateModelValue(int control, const QVariant& value, bool updateInitialValue) override;

    // Verify the validity of all pending settings. Fill the incoming vector with error info.
    virtual bool ValidatePendingSettings(std::vector<rgInvalidFieldInfo>& errorFields) override;

protected:
    // A model-specific implementation to initialize all model values.
    virtual void InitializeModelValuesImpl() override;

private:
    // Get the pending value of a target control.
    bool GetPendingValue(rgGlobalSettingsControls control, QVariant& value) const;

    // Update the given control's value in the given settings structure.
    void UpdateSetting(int control, const QVariant& value, std::shared_ptr<rgGlobalSettings> pBuildSettings);

    // The target build settings being edited by this model.
    std::shared_ptr<rgGlobalSettings> m_pGlobalSettings = nullptr;

    // The pending build settings to be saved.
    std::shared_ptr<rgGlobalSettings> m_pPendingGlobalSettings = nullptr;
};
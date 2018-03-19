#pragma once

// C++.
#include <map>

// Qt.
#include <QVariant>

// Local.
#include <RadeonGPUAnalyzerGUI/include/qt/rgSettingsModelBase.h>

// Forward declares.
struct rgCLBuildSettings;

enum rgOpenCLBuildSettingsControls
{
    TargetGPUs,
    PredefinedMacros,
    AdditionalIncludeDirectories,
    OptimizationLevel,
    TreatDoubleFloatingPointAsSingle,
    FlushDenormalizedFloatsAsZero,
    AssumeStrictAliasingRules,
    EnableMADInstructions,
    IgnoreSignednessOfZeros,
    AllowUnsafeOptimizations,
    AssumeNoNaNNorInfinite,
    AgressiveMathOptimizations,
    CorrectlyRoundSingleDivideAndSqrt,
    AdditionalOptions,
    AlternativeCompilerBinDir,
    AlternativeCompilerIncDir,
    AlternativeCompilerLibDir,
    Count
};

class rgOpenCLBuildSettingsModel : public rgSettingsModelBase
{
    Q_OBJECT
public:
    rgOpenCLBuildSettingsModel(std::shared_ptr<rgCLBuildSettings> pBuildSettings, unsigned int modelCount);
    virtual ~rgOpenCLBuildSettingsModel() = default;

    // Retrieve the current user-configured build settings.
    std::shared_ptr<rgCLBuildSettings> GetBuildSettings() const;

    // Retrieve the pending build settings. Pending changes differ from the original values, but have not yet been saved by the user.
    std::shared_ptr<rgCLBuildSettings> GetPendingBuildSettings() const;

    // Set the build settings
    void SetBuildSettings(std::shared_ptr<rgCLBuildSettings> pBuildSettings);

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
    bool GetPendingValue(rgOpenCLBuildSettingsControls control, QVariant& value) const;

    // Check the validity of the Target GPU field.
    bool IsTargetGpusStringValid(std::vector<rgInvalidFieldInfo>& errorFields) const;

    // Update the given control's value in the given build settings structure.
    void UpdateSetting(int control, const QVariant& value, std::shared_ptr<rgCLBuildSettings> pBuildSettings);

    // The target build settings being edited by this model.
    std::shared_ptr<rgCLBuildSettings> m_pBuildSettings = nullptr;

    // The pending build settings to be saved.
    std::shared_ptr<rgCLBuildSettings> m_pPendingBuildSettings = nullptr;
};
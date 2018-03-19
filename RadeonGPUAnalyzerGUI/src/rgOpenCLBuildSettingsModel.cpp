// C++.
#include <cassert>
#include <sstream>

// Local.
#include <RadeonGPUAnalyzerGUI/include/rgUtils.h>
#include <RadeonGPUAnalyzerGUI/include/qt/rgOpenCLBuildSettingsModel.h>
#include <RadeonGPUAnalyzerGUI/include/rgDataTypes.h>
#include <RadeonGPUAnalyzerGUI/include/rgStringConstants.h>

rgOpenCLBuildSettingsModel::rgOpenCLBuildSettingsModel(std::shared_ptr<rgCLBuildSettings> pBuildSettings, unsigned int modelCount) :
    rgSettingsModelBase(modelCount),
    m_pBuildSettings(pBuildSettings)
{
    assert(pBuildSettings != nullptr);
    if (pBuildSettings != nullptr)
    {
        m_pPendingBuildSettings = std::make_shared<rgCLBuildSettings>(*pBuildSettings);
    }
}

std::shared_ptr<rgCLBuildSettings> rgOpenCLBuildSettingsModel::GetBuildSettings() const
{
    return m_pBuildSettings;
}

std::shared_ptr<rgCLBuildSettings> rgOpenCLBuildSettingsModel::GetPendingBuildSettings() const
{
    return m_pPendingBuildSettings;
}

void rgOpenCLBuildSettingsModel::InitializeModelValuesImpl()
{
    // Every call below is responsible for initializing the underlying model data for the first time
    // based on what's in m_pBuildSettings. Each value is initialized by providing the enum associated
    // with the widget being initialized, and the corresponding value in the m_pBuildSettings instance.

    // The items below are common build settings for all API types.
    QString targetGpusList(rgUtils::BuildSemicolonSeparatedStringList(m_pBuildSettings->m_targetGpus).c_str());
    UpdateModelValue(TargetGPUs, targetGpusList, true);

    QString predefinedMacros(rgUtils::BuildSemicolonSeparatedStringList(m_pBuildSettings->m_predefinedMacros).c_str());
    UpdateModelValue(PredefinedMacros, predefinedMacros, true);

    QString additionalIncludeDirs(rgUtils::BuildSemicolonSeparatedStringList(m_pBuildSettings->m_additionalIncludeDirectories).c_str());
    UpdateModelValue(AdditionalIncludeDirectories, additionalIncludeDirs, true);

    // Items below are OpenCL-specific build settings only.
    QString optimizationsLevel(m_pBuildSettings->m_optimizationLevel.c_str());
    UpdateModelValue(OptimizationLevel, optimizationsLevel, true);
    UpdateModelValue(TreatDoubleFloatingPointAsSingle, m_pBuildSettings->m_isTreatDoubleAsSingle, true);
    UpdateModelValue(FlushDenormalizedFloatsAsZero, m_pBuildSettings->m_isDenormsAsZeros, true);
    UpdateModelValue(AssumeStrictAliasingRules, m_pBuildSettings->m_isStrictAliasing, true);
    UpdateModelValue(EnableMADInstructions, m_pBuildSettings->m_isEnableMAD, true);
    UpdateModelValue(IgnoreSignednessOfZeros, m_pBuildSettings->m_isIgnoreZeroSignedness, true);
    UpdateModelValue(AllowUnsafeOptimizations, m_pBuildSettings->m_isUnsafeOptimizations, true);
    UpdateModelValue(AssumeNoNaNNorInfinite, m_pBuildSettings->m_isNoNanNorInfinite, true);
    UpdateModelValue(AgressiveMathOptimizations, m_pBuildSettings->m_isAggressiveMathOptimizations, true);
    UpdateModelValue(CorrectlyRoundSingleDivideAndSqrt, m_pBuildSettings->m_isCorrectlyRoundDivSqrt, true);
    UpdateModelValue(AdditionalOptions, m_pBuildSettings->m_additionalOptions.c_str(), true);
    UpdateModelValue(AlternativeCompilerBinDir, std::get<CompilerFolderType::Bin>(m_pBuildSettings->m_compilerPaths).c_str(), true);
    UpdateModelValue(AlternativeCompilerIncDir, std::get<CompilerFolderType::Include>(m_pBuildSettings->m_compilerPaths).c_str(), true);
    UpdateModelValue(AlternativeCompilerLibDir, std::get<CompilerFolderType::Lib>(m_pBuildSettings->m_compilerPaths).c_str(), true);
}

void rgOpenCLBuildSettingsModel::UpdateModelValue(int control, const QVariant& value, bool updateInitialValue)
{
    if (updateInitialValue)
    {
        m_initialValues[control] = value;
    }
    else
    {
        m_pendingValues[control] = value;
        UpdateSetting(control, value, m_pPendingBuildSettings);
    }

    // If necessary, update the program with the new value.
    if (updateInitialValue)
    {
        UpdateSetting(control, value, m_pBuildSettings);
    }

    bool hasPendingChanges = GetHasPendingChanges();

    // Set the model data, and mark the model as having pending changes.
    SetModelData(control, value);
    SetHasPendingChanges(hasPendingChanges);
}

void rgOpenCLBuildSettingsModel::SetBuildSettings(std::shared_ptr<rgCLBuildSettings> pBuildSettings)
{
    bool isProgramValid = (pBuildSettings != nullptr);
    assert(isProgramValid);

    if (isProgramValid)
    {
        // Clear pending changes when replacing settings.
        RevertPendingChanges();

        // Replace the build settings used in the model, and re-initialize the default values.
        m_pBuildSettings = pBuildSettings;

        // Re-initialize the view based on the values in the newly-assigned settings.
        InitializeModelValues();
    }
}

void rgOpenCLBuildSettingsModel::SubmitPendingChanges()
{
    // Step through each control and update the displayed value if necessary.
    for (int controlIndex = 0; controlIndex < rgOpenCLBuildSettingsControls::Count; ++controlIndex)
    {
        rgOpenCLBuildSettingsControls control = static_cast<rgOpenCLBuildSettingsControls>(controlIndex);

        auto controlValue = m_pendingValues.find(control);
        if (controlValue != m_pendingValues.end())
        {
            UpdateModelValue(control, controlValue->second, true);
        }
    }

    // After submitting the changes, clear all of the user's pending changes.
    RevertPendingChanges();
}

bool rgOpenCLBuildSettingsModel::ValidatePendingSettings(std::vector<rgInvalidFieldInfo>& errorFields)
{
    // Check that the target GPUs string contains valid values.
    bool isValid = IsTargetGpusStringValid(errorFields);

    return isValid;
}

bool rgOpenCLBuildSettingsModel::GetPendingValue(rgOpenCLBuildSettingsControls control, QVariant& value) const
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

bool rgOpenCLBuildSettingsModel::IsTargetGpusStringValid(std::vector<rgInvalidFieldInfo>& errorFields) const
{
    bool isValid = false;

    // The target GPUs string must be non-empty.
    QVariant targetGpusValue;
    bool pendingTargetGpuValue = GetPendingValue(rgOpenCLBuildSettingsControls::TargetGPUs, targetGpusValue);
    if (pendingTargetGpuValue)
    {
        bool isValidString = true;

        std::string targetGpusString = targetGpusValue.toString().toStdString();
        if (targetGpusString.empty())
        {
            // The Target GPUs field is invalid since it is empty.
            rgInvalidFieldInfo emptyTargetGpusField =
            {
                rgOpenCLBuildSettingsControls::TargetGPUs,
                STR_ERR_TARGET_GPUS_CANNOT_BE_EMPTY
            };

            // Add the error to the output list.
            errorFields.push_back(emptyTargetGpusField);
            isValidString = false;
        }
        else
        {
            // Split the comma-separated GPUs string.
            std::vector<std::string> targetGPUsVector;
            rgUtils::splitString(targetGpusString, rgConfigManager::RGA_LIST_DELIMITER, targetGPUsVector);

            // Use the Config Manager to verify that the specified GPUs are valid.
            std::vector<std::string> invalidGpus;
            rgConfigManager& configManager = rgConfigManager::Instance();
            for (const std::string& targetGpuFamilyName : targetGPUsVector)
            {
                std::string trimmedName;
                rgUtils::TrimLeadingAndTrailingWhitespace(targetGpuFamilyName, trimmedName);

                // Is the given target GPU family name supported?
                if (!configManager.IsGpuFamilySupported(trimmedName))
                {
                    // Add the GPU to a list of invalid names if it's not supported.
                    invalidGpus.push_back(trimmedName);
                }
            }

            if (!invalidGpus.empty())
            {
                // Build an error string indicating the invalid GPUs.
                std::stringstream errorStream;
                errorStream << STR_ERR_INVALID_GPUS_SPECIFIED;

                int numInvalid = static_cast<int>(invalidGpus.size());
                for (int gpuIndex = 0; gpuIndex < numInvalid; ++gpuIndex)
                {
                    errorStream << invalidGpus[gpuIndex];

                    if (gpuIndex != (numInvalid - 1))
                    {
                        errorStream << ", ";
                    }
                }

                // Add an error field indicating that the GPU name is invalid.
                rgInvalidFieldInfo invalidGpuStringField =
                {
                    rgOpenCLBuildSettingsControls::TargetGPUs,
                    errorStream.str()
                };

                // Add the error to the output list.
                errorFields.push_back(invalidGpuStringField);
                isValidString = false;
            }
        }

        isValid = isValidString;
    }
    else
    {
        // If there's no pending value to validate, the Target GPUs string is assumed to be valid already.
        isValid = true;
    }

    return isValid;
}

void rgOpenCLBuildSettingsModel::UpdateSetting(int control, const QVariant& value, std::shared_ptr<rgCLBuildSettings> pBuildSettings)
{
    switch (control)
    {
    case TargetGPUs:
    {
        std::vector<std::string> targetGPUsVector;
        const std::string& commaSeparatedTargetGPUs = value.toString().toStdString();
        rgUtils::splitString(commaSeparatedTargetGPUs, rgConfigManager::RGA_LIST_DELIMITER, targetGPUsVector);
        pBuildSettings->m_targetGpus = targetGPUsVector;
    }
    break;
    case PredefinedMacros:
    {
        std::vector<std::string> predefinedMacrosVector;
        const std::string& commaSeparatedPredefinedMacros = value.toString().toStdString();
        rgUtils::splitString(commaSeparatedPredefinedMacros, rgConfigManager::RGA_LIST_DELIMITER, predefinedMacrosVector);
        pBuildSettings->m_predefinedMacros = predefinedMacrosVector;
    }
    break;
    case AdditionalIncludeDirectories:
    {
        std::vector<std::string> additionalIncludeDirectoriesVector;
        const std::string& commaSeparatedAdditionalIncludeDirectories = value.toString().toStdString();
        rgUtils::splitString(commaSeparatedAdditionalIncludeDirectories, rgConfigManager::RGA_LIST_DELIMITER, additionalIncludeDirectoriesVector);
        pBuildSettings->m_additionalIncludeDirectories = additionalIncludeDirectoriesVector;
    }
    break;
    case OptimizationLevel:
        pBuildSettings->m_optimizationLevel = value.toString().toStdString();
        break;
    case TreatDoubleFloatingPointAsSingle:
        pBuildSettings->m_isTreatDoubleAsSingle = value.toBool();
        break;
    case FlushDenormalizedFloatsAsZero:
        pBuildSettings->m_isDenormsAsZeros = value.toBool();
        break;
    case AssumeStrictAliasingRules:
        pBuildSettings->m_isStrictAliasing = value.toBool();
        break;
    case EnableMADInstructions:
        pBuildSettings->m_isEnableMAD = value.toBool();
        break;
    case IgnoreSignednessOfZeros:
        pBuildSettings->m_isIgnoreZeroSignedness = value.toBool();
        break;
    case AllowUnsafeOptimizations:
        pBuildSettings->m_isUnsafeOptimizations = value.toBool();
        break;
    case AssumeNoNaNNorInfinite:
        pBuildSettings->m_isNoNanNorInfinite = value.toBool();
        break;
    case AgressiveMathOptimizations:
        pBuildSettings->m_isAggressiveMathOptimizations = value.toBool();
        break;
    case CorrectlyRoundSingleDivideAndSqrt:
        pBuildSettings->m_isCorrectlyRoundDivSqrt = value.toBool();
        break;
    case AdditionalOptions:
        pBuildSettings->m_additionalOptions = value.toString().toStdString();
        break;
    case AlternativeCompilerBinDir:
        std::get<CompilerFolderType::Bin>(pBuildSettings->m_compilerPaths) = value.toString().toStdString();
        break;
    case AlternativeCompilerIncDir:
        std::get<CompilerFolderType::Include>(pBuildSettings->m_compilerPaths) = value.toString().toStdString();
        break;
    case AlternativeCompilerLibDir:
        std::get<CompilerFolderType::Lib>(pBuildSettings->m_compilerPaths) = value.toString().toStdString();
        break;
    default:
        assert(false);
        break;
    }
}
// C++.
#include <cassert>
#include <sstream>

// Local.
#include <RadeonGPUAnalyzerGUI/include/rgUtils.h>
#include <RadeonGPUAnalyzerGUI/include/qt/rgGlobalSettingsModel.h>
#include <RadeonGPUAnalyzerGUI/include/rgDataTypes.h>
#include <RadeonGPUAnalyzerGUI/include/rgStringConstants.h>

rgGlobalSettingsModel::rgGlobalSettingsModel(std::shared_ptr<rgGlobalSettings> pBuildSettings, unsigned int modelCount) :
    rgSettingsModelBase(modelCount),
    m_pGlobalSettings(pBuildSettings)
{
    assert(pBuildSettings != nullptr);
    if (pBuildSettings != nullptr)
    {
        m_pPendingGlobalSettings = std::make_shared<rgGlobalSettings>(*pBuildSettings);
    }
}

std::shared_ptr<rgGlobalSettings> rgGlobalSettingsModel::GetGlobalSettings() const
{
    return m_pGlobalSettings;
}

std::shared_ptr<rgGlobalSettings> rgGlobalSettingsModel::GetPendingGlobalSettings() const
{
    return m_pPendingGlobalSettings;
}

void rgGlobalSettingsModel::InitializeModelValuesImpl()
{
    // Initialize the log file location.
    QString logFileLocation(m_pGlobalSettings->m_logFileLocation.c_str());
    UpdateModelValue(LogFileLocation, logFileLocation , true);

    std::string columnsString = rgUtils::BuildSemicolonSeparatedBoolList(m_pGlobalSettings->m_visibleDisassemblyViewColumns);
    UpdateModelValue(DisassemblyViewColumns, columnsString.c_str(), true);

    // Initialize the use-generated project names checkbox.
    UpdateModelValue(AlwaysUseGeneratedProjectNames, m_pGlobalSettings->m_useDefaultProjectName, true);
}

void rgGlobalSettingsModel::UpdateModelValue(int control, const QVariant& value, bool updateInitialValue)
{
    if (updateInitialValue)
    {
        m_initialValues[control] = value;
    }
    else
    {
        m_pendingValues[control] = value;
        UpdateSetting(control, value, m_pPendingGlobalSettings);
    }

    // If necessary, update the program with the new value.
    if (updateInitialValue)
    {
        UpdateSetting(control, value, m_pGlobalSettings);
    }

    bool hasPendingChanges = GetHasPendingChanges();

    // Set the model data, and mark the model as having pending changes.
    SetModelData(control, value);
    SetHasPendingChanges(hasPendingChanges);
}

void rgGlobalSettingsModel::SetBuildSettings(std::shared_ptr<rgGlobalSettings> pBuildSettings)
{
    bool isProgramValid = (pBuildSettings != nullptr);
    assert(isProgramValid);

    if (isProgramValid)
    {
        // Clear pending changes when replacing settings.
        RevertPendingChanges();

        // Replace the build settings used in the model, and re-initialize the default values.
        m_pGlobalSettings = pBuildSettings;

        // Re-initialize the view based on the values in the newly-assigned settings.
        InitializeModelValues();
    }
}

void rgGlobalSettingsModel::SubmitPendingChanges()
{
    // Step through each control and update the displayed value if necessary.
    for (int controlIndex = 0; controlIndex < rgGlobalSettingsControls::Count; ++controlIndex)
    {
        rgGlobalSettingsControls control = static_cast<rgGlobalSettingsControls>(controlIndex);

        auto controlValue = m_pendingValues.find(control);
        if (controlValue != m_pendingValues.end())
        {
            UpdateModelValue(control, controlValue->second, true);
        }
    }

    // After submitting the changes, clear all of the user's pending changes.
    RevertPendingChanges();
}

bool rgGlobalSettingsModel::ValidatePendingSettings(std::vector<rgInvalidFieldInfo>& errorFields)
{
    // There are no values to validate within the model- just return true.
    return true;
}

bool rgGlobalSettingsModel::GetPendingValue(rgGlobalSettingsControls control, QVariant& value) const
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

void rgGlobalSettingsModel::UpdateSetting(int control, const QVariant& value, std::shared_ptr<rgGlobalSettings> pGlobalSettings)
{
    switch (control)
    {
    case LogFileLocation:
        {
            const std::string& logFileLocation = value.toString().toStdString();
            pGlobalSettings->m_logFileLocation = logFileLocation;
        }
        break;
    case DisassemblyViewColumns:
        {
            std::vector<std::string> selectedDisassemblyColumnsVector;
            const std::string& commaSeparatedColumns = value.toString().toStdString();
            rgUtils::splitString(commaSeparatedColumns, rgConfigManager::RGA_LIST_DELIMITER, selectedDisassemblyColumnsVector);

            int columnCount = static_cast<int>(selectedDisassemblyColumnsVector.size());
            for (int columnIndex = 0; columnIndex < columnCount; ++columnIndex)
            {
                bool isValidIndex = (columnIndex >= 0 && columnIndex < columnCount);
                assert(isValidIndex);
                if (isValidIndex)
                {
                    bool isVisible = (std::stoi(selectedDisassemblyColumnsVector[columnIndex]) == 1);
                    pGlobalSettings->m_visibleDisassemblyViewColumns[columnIndex] = isVisible;
                }
            }
        }
        break;
    case AlwaysUseGeneratedProjectNames:
        pGlobalSettings->m_useDefaultProjectName = value.toBool();
        break;
    default:
        // This should never happen.
        assert(false);
        break;
    }
}
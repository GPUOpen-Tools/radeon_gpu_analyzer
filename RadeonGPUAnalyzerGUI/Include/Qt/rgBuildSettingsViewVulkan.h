#pragma once

// C++.
#include <memory>

// Local.
#include "ui_rgBuildSettingsViewVulkan.h"
#include <RadeonGPUAnalyzerGUI/Include/rgDataTypes.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDataTypesVulkan.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgTargetGpusDialog.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgBuildSettingsView.h>

// Forward declarations.
class rgIncludeDirectoriesView;
class rgPreprocessorDirectivesDialog;
class QWidget;

class rgBuildSettingsViewVulkan : public rgBuildSettingsView
{
    Q_OBJECT

public:
    rgBuildSettingsViewVulkan(QWidget* pParent, const rgBuildSettingsVulkan& buildSettings, bool isGlobalSettings);
    virtual ~rgBuildSettingsViewVulkan() = default;

    // Event Filter for sub-widgets.
    virtual bool eventFilter(QObject* pObject, QEvent* pEvent) override;

    // Re-implement mousePressEvent method.
    virtual void mousePressEvent(QMouseEvent* pEvent) override;

public:
    virtual bool GetHasPendingChanges() const override;
    virtual bool RevertPendingChanges() override;
    virtual void RestoreDefaultSettings() override;
    virtual bool SaveSettings() override;
    virtual std::string GetTitleString() override;
    virtual void SetInitialWidgetFocus() override;

public slots:
    // Handler for the add the target GPU button.
    void HandleAddTargetGpusButtonClick();

    // Handler for when the text edit boxes change.
    void HandleTextEditChanged();

    // Handler for when the pending changes state changes.
    void HandlePendingChangesStateChanged(bool hasPendingChanges);

    // Handler for when the include directory browse button is clicked.
    void HandleIncludeDirsBrowseButtonClick();

    // Handler for when the include directories are updated.
    void HandleIncludeDirsUpdated(QStringList includeFiles);

    // Handler for when the preprocessor directives browse button is clicked.
    void HandlePreprocessorDirectivesBrowseButtonClick();

    // Handler for when the preprocessor directives are updated.
    void HandlePreprocessorDirectivesUpdated(QStringList preprocessorDirectives);

signals:
    void ProjectBuildSettingsSaved(std::shared_ptr<rgBuildSettings> pBuildSettings);
    void SetFrameBorderRedSignal();
    void SetFrameBorderBlackSignal();

private slots:
    // Handler for when the line edits get the focus.
    void HandleLineEditFocusInEvent();

    // Handler for when the line edits lose the focus.
    void HandleLineEditFocusOutEvent();

    // Handler for when the check boxes are clicked.
    void HandleCheckBoxClickedEvent();

    // Handler for when the check box state is changed.
    void HandleCheckboxStateChanged();

    // Handler for when the user clicks on the ICD location browse button.
    void HandleICDLocationBrowseButtonClick(bool /* checked */);

    // Handler for when the user manually changes the ICD location text.
    void HandleICDLocationLineEditChanged(const QString& text);

    // Handler for when the user manually changes the glslang options text.
    void HandleGlslangOptionsLineEditChanged(const QString& text);

    // Handler for when the user clicks on the Alternative Compiler path browse button.
    void HandleAlternativeCompilerBrowseButtonClicked();

    // Handler for when the user manually changes the Alternative Compiler path text.
    void HandleAlternativeCompilerLineEditChanged(const QString& text);

private:
    // Make the UI reflect the values in the supplied settings struct.
    void PushToWidgets(const rgBuildSettingsVulkan& settings);

    // Use the values in the UI to create a settings struct which
    // contains the pending changes.
    rgBuildSettingsVulkan PullFromWidgets() const;

    // Connect the signals.
    void ConnectSignals();

    // Connect focus in/out events for line edits.
    void ConnectLineEditFocusEvents();

    // Connect clicked events for check boxes.
    void ConnectCheckBoxClickedEvents();

    // Get the string to use for this view's tooltip text.
    const std::string GetTitleTooltipString() const;

    // Hide HLSL options for now.
    void HideHLSLOptions();

    // Update the generated command line text.
    void UpdateCommandLineText();

    // Check the validity of the TargetGPUs string.
    bool IsTargetGpusStringValid(std::vector<std::string>& errors) const;

    // Check the validity of pending settings within the view.
    bool ValidatePendingSettings();

    // Set the cursor to pointing hand cursor for various widgets.
    void SetCursor();

    // Target GPU Selection dialog.
    rgTargetGpusDialog* m_pTargetGpusDialog = nullptr;

    // The include directories dialog.
    rgIncludeDirectoriesView* m_pIncludeDirectoriesView = nullptr;

    // The preprocessor directives editor dialog.
    rgPreprocessorDirectivesDialog* m_pPreprocessorDirectivesDialog = nullptr;

    // Initial version of the settings that the view was created with.
    // Note: They will also get updated when the user clicks 'Save', so it can't
    // be const.
    rgBuildSettingsVulkan m_initialSettings;

    // The generated interface view object.
    Ui::rgBuildSettingsViewVulkan ui;
};
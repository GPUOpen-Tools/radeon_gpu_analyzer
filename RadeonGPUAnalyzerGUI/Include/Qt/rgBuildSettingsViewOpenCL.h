#pragma once

// C++.
#include <memory>

// Local.
#include "ui_rgBuildSettingsViewOpenCL.h"
#include <RadeonGPUAnalyzerGUI/Include/rgDataTypes.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDataTypesOpenCL.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgTargetGpusDialog.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgBuildSettingsView.h>

// Forward declarations.
class rgIncludeDirectoriesView;
class rgPreprocessorDirectivesDialog;
class QWidget;

class rgBuildSettingsViewOpenCL : public rgBuildSettingsView
{
    Q_OBJECT

public:
    rgBuildSettingsViewOpenCL(QWidget* pParent, const rgBuildSettingsOpenCL& buildSettings, bool isGlobalSettings);
    virtual ~rgBuildSettingsViewOpenCL() = default;

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

    // Update the generated command line text.
    void UpdateCommandLineText() override;

public slots:
    void HandleAddTargetGpusButtonClick();
    void HandleTextEditChanged();
    void HandleComboboxIndexChanged(int index);
    void HandleCheckboxStateChanged();
    void HandlePendingChangesStateChanged(bool hasPendingChanges);
    void HandleIncludeDirsBrowseButtonClick();
    void HandleIncludeDirsUpdated(QStringList includeFiles);
    void HandlePreprocessorDirectivesBrowseButtonClick();
    void HandlePreprocessorDirectivesUpdated(QStringList preprocessorDirectives);
    void HandleAdditionalOptionsTextChanged();

signals:
    void ProjectBuildSettingsSaved(std::shared_ptr<rgBuildSettings> pBuildSettings);
    void SetFrameBorderGreenSignal();
    void SetFrameBorderBlackSignal();

private slots:
    void HandleLineEditFocusInEvent();
    void HandleLineEditFocusOutEvent();
    void HandleCheckBoxClickedEvent();
    void HandleComboBoxFocusInEvent();
    void HandleCompilerFolderBrowseButtonClick(CompilerFolderType folderType);
    void HandleCompilerFolderEditChanged(CompilerFolderType folderType);

private:
    // Make the UI reflect the values in the supplied settings struct.
    void PushToWidgets(const rgBuildSettingsOpenCL& buildSettings);

    // Use the values in the UI to create a settings struct which
    // contains the pending changes.
    rgBuildSettingsOpenCL PullFromWidgets() const;

    // Connect the signals.
    void ConnectSignals();

    // Connect the combobox click event.
    void ConnectComboboxClickEvent();

    // Connect focus in/out events for line edits.
    void ConnectLineEditFocusEvents();

    // Connect focus in/out events for checkboxes.
    void ConnectCheckBoxClickedEvents();

    // Get the string to use for this view's tooltip text.
    const std::string GetTitleTooltipString() const;

    // Check the validity of the Target GPU field.
    bool IsTargetGpusStringValid(std::vector<std::string>& errors) const;

    // Check the validity of pending settings within the view.
    bool ValidatePendingSettings();

    // Set the cursor to pointing hand cursor for various widgets.
    void SetCursor();

    // Set the geometry of tool tip boxes.
    void SetToolTipGeometry();

    // Target GPU Selection dialog.
    rgTargetGpusDialog* m_pTargetGpusDialog = nullptr;

    // Initial version of the settings that the view was created with.
    // Note: They will also get updated when the user clicks 'Save', so it can't
    // be const.
    rgBuildSettingsOpenCL m_initialSettings;

    // The include directories dialog.
    rgIncludeDirectoriesView* m_pIncludeDirectoriesView = nullptr;

    // The preprocessor directives editor dialog.
    rgPreprocessorDirectivesDialog* m_pPreprocessorDirectivesDialog = nullptr;

    // The generated interface view object.
    Ui::rgBuildSettingsViewOpenCL ui;
};
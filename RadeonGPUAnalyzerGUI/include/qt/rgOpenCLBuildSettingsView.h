#pragma once

// C++.
#include <memory>

// Local.
#include "ui_rgOpenCLSettingsView.h"
#include <RadeonGPUAnalyzerGUI/include/rgDataTypes.h>
#include <RadeonGPUAnalyzerGUI/include/qt/rgOpenCLBuildSettingsModel.h>
#include <RadeonGPUAnalyzerGUI/include/qt/rgTargetGpusDialog.h>
#include <RadeonGPUAnalyzerGUI/include/qt/rgBuildSettingsView.h>

// Forward declarations.
class rgOpenCLBuildSettingsModel;
class rgIncludeDirectoriesView;
class rgPreprocessorDirectivesDialog;
class QWidget;

class rgOpenCLBuildSettingsView : public rgBuildSettingsView
{
    Q_OBJECT

public:
    rgOpenCLBuildSettingsView(QWidget* pParent, std::shared_ptr<rgCLBuildSettings> pBuildSettings, bool isGlobalSettings);
    virtual ~rgOpenCLBuildSettingsView() = default;

    // Event Filter for sub-widgets.
    virtual bool eventFilter(QObject* obj, QEvent* event) override;

    // Set the minimum height for the scroll area.
    void SetScrollAreaFixedHeight(const int minHeight);

public:
    virtual bool GetHasPendingChanges() const override;
    virtual bool RevertPendingChanges() override;
    virtual void RestoreDefaultSettings() override;
    virtual void SaveSettings() override;

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
    void SetFrameBorderRedSignal();
    void SetFrameBorderBlackSignal();

private slots:
    void HandleLineEditFocusInEvent();
    void HandleLineEditFocusOutEvent();
    void HandleCheckBoxClickedEvent();
    void HandleComboBoxFocusInEvent();
    void HandleCompilerFolderBrowseButtonClick(CompilerFolderType folderType);
    void HandleCompilerFolderEditChanged(CompilerFolderType folderType);

private:
    // Connect the signals.
    void ConnectSignals();

    // Connect the combobox click event.
    void ConnectComboboxClickEvent();

    // Connect focus in/out events for line edits.
    void ConnectLineEditFocusEvents();

    // Connect focus in/out events for checkboxes.
    void ConnectCheckBoxClickedEvents();

    // Get the title string.
    const std::string GetTitleString() const;

    // Get the string to use for this view's tooltip text.
    const std::string GetTitleTooltipString() const;

    // Initialize the model and bind it to the view controls.
    void InitializeModel();

    // Update the generated command line text.
    void UpdateCommandLineText();

    // Set the "Additional Options" text.
    void SetAdditionalOptionsText(const std::string& text);

    // Check the validity of pending settings within the view.
    bool ValidatePendingSettings();

    // Set the cursor to pointing hand cursor for various widgets.
    void SetCursor();

    // Set the geometry of tool tip boxes.
    void SetToolTipGeometry();

    // Target GPU Selection dialog.
    rgTargetGpusDialog* m_pTargetGpusDialog = nullptr;

    // The settings model bound to this view.
    rgOpenCLBuildSettingsModel* m_pSettingsModel = nullptr;

    // Flag used to indicate if the view relates to global or project-specific settings.
    bool m_isGlobalSettings = false;

    // The include directories dialog.
    rgIncludeDirectoriesView* m_pIncludeDirectoriesView = nullptr;

    // The preprocessor directives editor dialog.
    rgPreprocessorDirectivesDialog* m_pPreprocessorDirectivesDialog = nullptr;

    // The generated interface view object.
    Ui::rgOpenCLSettingsView ui;
};
#pragma once

// C++.
#include <memory>

// Qt.
#include <QtWidgets/QWidget>
#include <QMenu>

// Local.
#include "ui_rgSettingsTab.h"
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgBuildSettingsView.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgUnsavedItemsDialog.h>

// Forward declarations.
class QMenu;
class rgAppState;
class rgRecentProjectWidget;
class rgGlobalSettingsView;

enum class rgProjectAPI : char;

// The entries in the setting list widget.
enum class SettingsListWidgetEntries
{
    Global,
    Api
};

class rgSettingsTab : public QWidget
{
    Q_OBJECT

public:
    rgSettingsTab(QWidget* pParent);
    virtual ~rgSettingsTab() = default;

    // Initialize the settings tab.
    void Initialize();

    // Add a view to the settings tab.
    void AddSettingsView(rgBuildSettingsView* pSettingsView);

    // Get the global settings view.
    rgGlobalSettingsView* GetGlobalSettingsView();

    // Update the title of the API-specific build settings.
    void UpdateBuildSettingsTitle(bool hasPendingChanges);

    // Prompt user to save pending changes.
    // Returns false if user cancels the prompt; true otherwise.
    bool PromptToSavePendingChanges();

    // Save the pending changes without prompting the user.
    void SavePendingChanges();

    // Select the next row in the settings list widget.
    void SelectNextListWidgetItem(const int keyPressed);

    // Revert pending changes.
    void RevertPendingChanges();

    // Set the global application settings stylesheet.
    void SetGlobalSettingsStylesheet(const std::string& stylesheet);

    // Set the build application settings stylesheet.
    void SetBuildSettingsStylesheet(const std::string& stylesheet);

    // Get the settings list widget.
    rgListWidget* GetSettingsListWidget();

signals:
    // Indicate whether the settings tab has pending changes.
    void PendingChangesStateChanged(bool hasPendingChanges);

    // Update the command line text.
    void UpdateCommandLineTextSignal();

protected slots:
    // Handler for when the state of the pending build settings is changed.
    void HandleBuildSettingsPendingChangesStateChanged(bool pendingChanges);

    // Handler for when global settings data changes.
    void HandleGlobalPendingChangesStateChanged(bool pendingChanges);

    // Handler for the settings list widget.
    void HandleSettingsListWidgetClick(int index);

    // Handler for when the "Save" button is clicked.
    void HandleSaveSettingsButtonClicked();

    // Handler for when the "Restore defaults" button is clicked.
    void HandleRestoreDefaultsSettingsClicked();

    // Handler for when one of the input file line edits is blank.
    void HandleInputFileNameBlank(bool isBlank);

protected:
    // Re-implement eventFilter to handle clicks between settings pages.
    virtual bool eventFilter(QObject* pObject, QEvent* pEvent) override;

    // Get the API Type.
    virtual rgProjectAPI GetApiType() = 0;

    // The build settings view.
    rgBuildSettingsView* m_pBuildSettingsView = nullptr;

    // The global settings view.
    rgGlobalSettingsView* m_pGlobalSettingsView = nullptr;

    // Create the API-Specific build settings widget.
    rgBuildSettingsView* CreateApiBuildSettingsView();

    // The generated view object.
    Ui::rgSettingsTab ui;

private:
    // Display the dialog to prompt for saving the settings.
    rgUnsavedItemsDialog::UnsavedFileDialogResult ShowSaveSettingsConfirmationDialog();

    // Get the currently-visible settings category.
    SettingsListWidgetEntries GetSelectedSettingCategory() const;

    // Subclasses can use this to set whether they have pending changes;
    // this will emit the pending changes signal as needed.
    void NotifyOfPendingChanges();

    // Connect the signals.
    void ConnectSignals();

    // Set the view's cursor for each relevant widget.
    void SetCursor();

    // Save changed settings.
    void SaveSettings();

    // Indicates that the user has build pending changes on Settings pane.
    bool m_hasBuildPendingChanges = false;

    // Indicates that the user has application pending changes on Settings pane.
    bool m_hasApplicationPendingChanges = false;

    // Flag used to store whether a signal was emitted regarding pending changes.
    bool m_hasPendingChanges = false;
};

#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_SETTINGS_TAB_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_SETTINGS_TAB_H_

// C++.
#include <memory>

// Qt.
#include <QtWidgets/QWidget>
#include <QMenu>

// Local.
#include "ui_rg_settings_tab.h"
#include "source/radeon_gpu_analyzer_gui/qt/rg_build_settings_view.h"
#include "source/radeon_gpu_analyzer_gui/qt/rg_unsaved_items_dialog.h"

// Forward declarations.
class QMenu;
class RgAppState;
class RgRecentProjectWidget;
class RgGlobalSettingsView;

enum class RgProjectAPI : char;

// The entries in the setting list widget.
enum class SettingsListWidgetEntries
{
    kGlobal,
    kApi
};

class RgSettingsTab : public QWidget
{
    Q_OBJECT

public:
    RgSettingsTab(QWidget* parent);
    virtual ~RgSettingsTab() = default;

    // Initialize the settings tab.
    void Initialize();

    // Add a view to the settings tab.
    void AddSettingsView(RgBuildSettingsView* settings_view);

    // Get the global settings view.
    RgGlobalSettingsView* GetGlobalSettingsView();

    // Update the title of the API-specific build settings.
    void UpdateBuildSettingsTitle(bool has_pending_changes);

    // Prompt user to save pending changes.
    // Returns false if user cancels the prompt; true otherwise.
    bool PromptToSavePendingChanges();

    // Save the pending changes without prompting the user.
    void SavePendingChanges();

    // Select the next row in the settings list widget.
    void SelectNextListWidgetItem(const int key_pressed);

    // Revert pending changes.
    void RevertPendingChanges();

    // Set the global application settings stylesheet.
    void SetGlobalSettingsStylesheet(const std::string& stylesheet);

    // Set the build application settings stylesheet.
    void SetBuildSettingsStylesheet(const std::string& stylesheet);

    // Get the settings list widget.
    RgListWidget* GetSettingsListWidget();

signals:
    // Indicate whether the settings tab has pending changes.
    void PendingChangesStateChanged(bool has_pending_changes);

    // Update the command line text.
    void UpdateCommandLineTextSignal();

protected slots:
    // Handler for when the state of the pending build settings is changed.
    void HandleBuildSettingsPendingChangesStateChanged(bool pending_changes);

    // Handler for when global settings data changes.
    void HandleGlobalPendingChangesStateChanged(bool pending_changes);

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
    virtual bool eventFilter(QObject* object, QEvent* event) override;

    // Get the API Type.
    virtual RgProjectAPI GetApiType() = 0;

    // The build settings view.
    RgBuildSettingsView* build_settings_view_ = nullptr;

    // The global settings view.
    RgGlobalSettingsView* global_settings_view_ = nullptr;

    // Create the API-Specific build settings widget.
    RgBuildSettingsView* CreateApiBuildSettingsView();

    // The generated view object.
    Ui::RgSettingsTab ui_;

private:
    // Display the dialog to prompt for saving the settings.
    RgUnsavedItemsDialog::UnsavedFileDialogResult ShowSaveSettingsConfirmationDialog();

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
    bool has_build_pending_changes_ = false;

    // Indicates that the user has application pending changes on Settings pane.
    bool has_application_pending_changes_ = false;

    // Flag used to store whether a signal was emitted regarding pending changes.
    bool has_pending_changes_ = false;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_SETTINGS_TAB_H_

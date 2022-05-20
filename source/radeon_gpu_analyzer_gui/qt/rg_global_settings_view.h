#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_GLOBAL_SETTINGS_VIEW_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_GLOBAL_SETTINGS_VIEW_H_

// C++.
#include <memory>

// Infra.
#include "QtCommon/CustomWidgets/ListWidget.h"
#include "QtCommon/Scaling/ScalingManager.h"

// Local.
#include "ui_rg_global_settings_view.h"
#include "source/radeon_gpu_analyzer_gui/rg_data_types.h"
#include "source/radeon_gpu_analyzer_gui/qt/rg_isa_disassembly_table_model.h"
#include "source/radeon_gpu_analyzer_gui/qt/rg_build_settings_view.h"

// Forward declarations.
class QWidget;

class RgGlobalSettingsView : public RgBuildSettingsView
{
    Q_OBJECT

public:
    RgGlobalSettingsView(QWidget* parent, const RgGlobalSettings& global_settings);
    virtual ~RgGlobalSettingsView();
    void CloseListWidget();

public:
    virtual bool GetHasPendingChanges() const override;
    virtual bool RevertPendingChanges() override;
    virtual void RestoreDefaultSettings() override;
    virtual bool SaveSettings() override;
    virtual std::string GetTitleString() override;
    virtual void SetInitialWidgetFocus() override;

    // Update the generated command line text.
    // This method is not needed for global settings,
    // but is a pure virtual method declared in the base
    // class so we'll just define it here with no code.
    void UpdateCommandLineText() override {};

    // Check to see if any of the input file line edits is blank.
    bool IsInputFileBlank() const;

    // Process the blank input file.
    void ProcessInputFileBlank() const;

signals:
    // A signal to indicate that one or more of the input file names is empty.
    void InputFileNameBlankSignal(bool is_empty);

public slots:
    // Handler for when the pending changes state has changed.
    void HandlePendingChangesStateChanged(bool has_pending_changes);

    // Handler for when the log file location browse button is clicked.
    void HandleLogFileLocationBrowseButtonClick(bool checked);

    // Handler for when the project file location browse button is clicked.
    void HandleProjectFileLocationBrowseButtonClick(bool checked);

    // Handler for when the include files viewer browse button is clicked.
    void HandleIncludeFilesViewerBrowseButtonClick(bool checked);

    // Handler for when the column visibility combo box item is clicked.
    void HandleColumnVisibilityComboBoxItemClicked(const QString& text, bool checked);

    // Handler for when the column visibility filter state has changed.
    void HandleColumnVisibilityFilterStateChanged(bool checked);

    // Handler for when the view column button is clicked.
    void HandleViewColumnsButtonClick(bool checked);

    // Handler for when the project name check box state has changed.
    void HandleProjectNameCheckboxStateChanged(int checked);

    // Handler for when log file editing is finished.
    void HandleLogFileEditingFinished();

    // Handler for when project file editing is finished.
    void HandleProjectFileEditingFinished();

    // Handler for when the log file edit box changed.
    void HandleLogFileEditBoxChanged(const QString& text);

    // Handler for when the project file edit box changed.
    void HandleProjectFileEditBoxChanged(const QString& text);

    // Handler for any text box which signals about pending changes.
    void HandleTextBoxChanged(const QString& text);

    // Handler for any combo box which signals about pending changes.
    void HandleComboBoxChanged(int index);

    // Handler for when the font family has changed.
    void HandleFontFamilyChanged(const QFont& font);

    // Handler for when include files viewer text box editing is finished.
    void HandleIncludeFilesViewerEditingFinished();

private slots:
    // Handler for when the line edits lose focus.
    void HandleFocusOutEvent();

protected:
    virtual void showEvent(QShowEvent* event) override;

    // The widget used to display all columns available for display in the disassembly table.
    ListWidget* disassembly_columns_list_widget_ = nullptr;

private:
    // Make the UI reflect the values in the supplied global settings struct.
    void PushToWidgets(const RgGlobalSettings& global_settings);

    // Use the values in the UI to create a global settings struct which
    // contains the pending changes.
    RgGlobalSettings PullFromWidgets() const;

    // Remove all items from the given list widget.
    void ClearListWidget(ListWidget* &list_widget);

    // Connect the signals.
    void ConnectSignals();

    // Create the controls responsible for picking the visible columns in the disassembly table.
    void CreateColumnVisibilityControls();

    // Get the name of the given disassembly column as a string.
    std::string GetDisassemblyColumnName(RgIsaDisassemblyTableColumns column) const;

    // Populate the names in the column visibility list.
    void PopulateColumnVisibilityList();

    // Set the cursor to pointing hand cursor for various widgets.
    void SetCursor();

    // Set the tooltip for the default program name check box.
    void SetCheckboxToolTip(const std::string& text);

    // Set the tooltips for the edit boxes.
    void SetEditBoxToolTips();

    // Update the "All" checkbox text color to grey if it is already checked,
    // to black otherwise.
    void UpdateAllCheckBoxText();

    // A custom event filter for the disassembly columns list widget.
    QObject* disassembly_columns_list_event_filter_ = nullptr;

    // A pointer to the parent widget.
    QWidget* parent_ = nullptr;

    // Initial version of the settings that the view was created with.
    // Note: They will also get updated when the user clicks 'Save', so it can't
    // be const.
    RgGlobalSettings initial_settings_;

    // The generated interface view object.
    Ui::RgGlobalSettingsView ui_;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_GLOBAL_SETTINGS_VIEW_H_

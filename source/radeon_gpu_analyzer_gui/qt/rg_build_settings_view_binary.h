#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_BUILD_SETTINGS_VIEW_BINARY_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_BUILD_SETTINGS_VIEW_BINARY_H_

// C++.
#include <memory>

// Local.
#include "ui_rg_build_settings_view_binary.h"
#include "source/radeon_gpu_analyzer_gui/rg_data_types.h"
#include "source/radeon_gpu_analyzer_gui/rg_data_types_binary.h"
#include "source/radeon_gpu_analyzer_gui/qt/rg_target_gpus_dialog.h"
#include "source/radeon_gpu_analyzer_gui/qt/rg_build_settings_view.h"

// Forward declarations.
class RgIncludeDirectoriesView;
class RgPreprocessorDirectivesDialog;
class QWidget;

class RgBuildSettingsViewBinary : public RgBuildSettingsView
{
    Q_OBJECT

public:
    RgBuildSettingsViewBinary(QWidget* parent, const RgBuildSettingsBinary& build_settings, bool is_global_settings);
    virtual ~RgBuildSettingsViewBinary() = default;

    // Event Filter for sub-widgets.
    virtual bool eventFilter(QObject* object, QEvent* event) override;

    // Re-implement mousePressEvent method.
    virtual void mousePressEvent(QMouseEvent* event) override;

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
    void HandleTextEditChanged();
    void HandleComboboxIndexChanged(int index);
    void HandleCheckboxStateChanged();
    void HandlePendingChangesStateChanged(bool has_pending_changes);
    void HandleIncludeDirsBrowseButtonClick();
    void HandleIncludeDirsUpdated(QStringList include_files);

signals:
    void ProjectBuildSettingsSaved(std::shared_ptr<RgBuildSettings> build_settings);
    void SetFrameBorderPurpleSignal();
    void SetFrameBorderBlackSignal();

private slots:
    void HandleLineEditFocusInEvent();
    void HandleLineEditFocusOutEvent();
    void HandleCheckBoxClickedEvent();
    void HandleComboBoxFocusInEvent();

private:
    // Make the UI reflect the values in the supplied settings struct.
    void PushToWidgets(const RgBuildSettingsBinary& build_settings);

    // Use the values in the UI to create a settings struct which
    // contains the pending changes.
    RgBuildSettingsBinary PullFromWidgets() const;

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

    // Set the cursor to pointing hand cursor for various widgets.
    void SetCursor();

    // Set the geometry of tool tip boxes.
    void SetToolTipGeometry();

    // Initial version of the settings that the view was created with.
    // Note: They will also get updated when the user clicks 'Save', so it can't
    // be const.
    RgBuildSettingsBinary initial_settings_;

    // The include directories dialog.
    RgIncludeDirectoriesView* include_directories_view_ = nullptr;

    // The generated interface view object.
    Ui::RgBuildSettingsViewBinary ui_;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_BUILD_SETTINGS_VIEW_OPENCL_H_

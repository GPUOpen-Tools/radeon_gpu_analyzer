#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_BUILD_SETTINGS_VIEW_VULKAN_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_BUILD_SETTINGS_VIEW_VULKAN_H_

// C++.
#include <memory>

// Local.
#include "ui_rg_build_settings_view_vulkan.h"
#include "source/radeon_gpu_analyzer_gui/rg_data_types.h"
#include "source/radeon_gpu_analyzer_gui/rg_data_types_vulkan.h"
#include "source/radeon_gpu_analyzer_gui/qt/rg_target_gpus_dialog.h"
#include "source/radeon_gpu_analyzer_gui/qt/rg_build_settings_view.h"

// Forward declarations.
class RgIncludeDirectoriesView;
class RgPreprocessorDirectivesDialog;
class QWidget;

class RgBuildSettingsViewVulkan : public RgBuildSettingsView
{
    Q_OBJECT

public:
    RgBuildSettingsViewVulkan(QWidget* parent, const RgBuildSettingsVulkan& build_settings, bool is_global_settings);
    virtual ~RgBuildSettingsViewVulkan() = default;

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
    // Handler for the add the target GPU button.
    void HandleAddTargetGpusButtonClick();

    // Handler for when the text edit boxes change.
    void HandleTextEditChanged();

    // Handler for when the pending changes state changes.
    void HandlePendingChangesStateChanged(bool has_pending_changes);

    // Handler for when the include directory browse button is clicked.
    void HandleIncludeDirsBrowseButtonClick();

    // Handler for when the include directories are updated.
    void HandleIncludeDirsUpdated(QStringList include_files);

    // Handler for when the preprocessor directives browse button is clicked.
    void HandlePreprocessorDirectivesBrowseButtonClick();

    // Handler for when the preprocessor directives are updated.
    void HandlePreprocessorDirectivesUpdated(QStringList preprocessor_directives);

    // Handler for when output binary editing is finished.
    void HandleOutputBinaryFileEditingFinished();

    // Handler for when the output binary file edit box changed.
    void HandleOutputBinaryEditBoxChanged(const QString& text);

signals:
    void ProjectBuildSettingsSaved(std::shared_ptr<RgBuildSettings> build_settings);
    void SetFrameBorderRedSignal();
    void SetFrameBorderBlackSignal();

private slots:
    // Handler for when the browse buttons get the focus.
    void HandleBrowseButtonFocusInEvent();

    // Handler for when the browse buttons lose the focus.
    void HandleBrowseButtonFocusOutEvent();

    // Handler for when the check box gets the focus.
    void HandleCheckBoxFocusInEvent();

    // Handler for when the check box loses the focus.
    void HandleCheckBoxFocusOutEvent();

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
    void PushToWidgets(const RgBuildSettingsVulkan& settings);

    // Use the values in the UI to create a settings struct which
    // contains the pending changes.
    RgBuildSettingsVulkan PullFromWidgets() const;

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

    // Check the validity of the TargetGPUs string.
    bool IsTargetGpusStringValid(std::vector<std::string>& errors) const;

    // Check the validity of pending settings within the view.
    bool ValidatePendingSettings();

    // Set the cursor to pointing hand cursor for various widgets.
    void SetCursor();

    // Target GPU Selection dialog.
    RgTargetGpusDialog* target_gpus_dialog_ = nullptr;

    // The include directories dialog.
    RgIncludeDirectoriesView* include_directories_view_ = nullptr;

    // The preprocessor directives editor dialog.
    RgPreprocessorDirectivesDialog* preprocessor_directives_dialog_ = nullptr;

    // Initial version of the settings that the view was created with.
    // Note: They will also get updated when the user clicks 'Save', so it can't
    // be const.
    RgBuildSettingsVulkan initial_settings_;

    // The generated interface view object.
    Ui::RgBuildSettingsViewVulkan ui_;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_BUILD_SETTINGS_VIEW_VULKAN_H_

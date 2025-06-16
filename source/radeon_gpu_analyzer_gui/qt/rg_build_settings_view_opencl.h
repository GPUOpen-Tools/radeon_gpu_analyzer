//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for Build settings view for OpenCL.
//=============================================================================

#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_BUILD_SETTINGS_VIEW_OPENCL_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_BUILD_SETTINGS_VIEW_OPENCL_H_

// C++.
#include <memory>

// Local.
#include "ui_rg_build_settings_view_opencl.h"
#include "source/radeon_gpu_analyzer_gui/rg_data_types.h"
#include "source/radeon_gpu_analyzer_gui/rg_data_types_opencl.h"
#include "source/radeon_gpu_analyzer_gui/qt/rg_target_gpus_dialog.h"
#include "source/radeon_gpu_analyzer_gui/qt/rg_build_settings_view.h"

// Forward declarations.
class RgIncludeDirectoriesView;
class RgPreprocessorDirectivesDialog;
class QWidget;

class RgBuildSettingsViewOpencl : public RgBuildSettingsView
{
    Q_OBJECT

public:
    RgBuildSettingsViewOpencl(QWidget* parent, const RgBuildSettingsOpencl& build_settings, bool is_global_settings);
    virtual ~RgBuildSettingsViewOpencl() = default;

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
    void HandleAddTargetGpusButtonClick();
    void HandleTextEditChanged();
    void HandleComboboxIndexChanged(int index);
    void HandleCheckboxStateChanged();
    void HandlePendingChangesStateChanged(bool has_pending_changes);
    void HandleIncludeDirsBrowseButtonClick();
    void HandleIncludeDirsUpdated(QStringList include_files);
    void HandlePreprocessorDirectivesBrowseButtonClick();
    void HandlePreprocessorDirectivesUpdated(QStringList preprocessor_directives);
    void HandleAdditionalOptionsTextChanged();

signals:
    void ProjectBuildSettingsSaved(std::shared_ptr<RgBuildSettings> build_settings);
    void SetFrameBorderGreenSignal();
    void SetFrameBorderBlackSignal();

private slots:
    void HandleLineEditFocusInEvent();
    void HandleLineEditFocusOutEvent();
    void HandleCheckBoxClickedEvent();
    void HandleComboBoxFocusInEvent();
    void HandleCompilerFolderBrowseButtonClick(CompilerFolderType folder_type);
    void HandleCompilerFolderEditChanged(CompilerFolderType folder_type);

private:
    // Make the UI reflect the values in the supplied settings struct.
    void PushToWidgets(const RgBuildSettingsOpencl& build_settings);

    // Use the values in the UI to create a settings struct which
    // contains the pending changes.
    RgBuildSettingsOpencl PullFromWidgets() const;

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
    RgTargetGpusDialog* target_gpus_dialog_ = nullptr;

    // Initial version of the settings that the view was created with.
    // Note: They will also get updated when the user clicks 'Save', so it can't
    // be const.
    RgBuildSettingsOpencl initial_settings_;

    // The include directories dialog.
    RgIncludeDirectoriesView* include_directories_view_ = nullptr;

    // The preprocessor directives editor dialog.
    RgPreprocessorDirectivesDialog* preprocessor_directives_dialog_ = nullptr;

    // The generated interface view object.
    Ui::rgBuildSettingsViewOpenCL ui_;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_BUILD_SETTINGS_VIEW_OPENCL_H_

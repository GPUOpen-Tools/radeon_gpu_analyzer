//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for Global settings view.
//=============================================================================

#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_GLOBAL_SETTINGS_VIEW_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_GLOBAL_SETTINGS_VIEW_H_

// C++.
#include <memory>

#include <QWidget>

// Infra.
#include "qt_common/custom_widgets/list_widget.h"
#include "qt_isa_gui/widgets/isa_item_model.h"

// Local.
#include "ui_rg_global_settings_view.h"
#include "radeon_gpu_analyzer_gui/rg_data_types.h"
#include "radeon_gpu_analyzer_gui/qt/rg_build_settings_view.h"
#include "radeon_gpu_analyzer_gui/qt/rg_isa_item_model.h"

// Forward declarations.
class QWidget;

class RgGlobalSettingsView : public RgBuildSettingsView
{
    Q_OBJECT

public:
    RgGlobalSettingsView(QWidget* parent, const RgGlobalSettings& global_settings);
    virtual ~RgGlobalSettingsView();

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

    // Getter for the combo box.
    ArrowIconComboBox* GetColumnVisibilityComboBox();

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
    void HandleColumnVisibilityComboBoxItemClicked(QCheckBox* check_box);

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

    /// @brief Handle Color theme changed in the settings.
    ///
    /// color_theme_option Color theme option that was selected.
    void HandleColorThemeComboBoxChanged(QListWidgetItem* color_theme_option);

    /// @brief Set the color theme for the application.
    ///
    /// @return Whether the the user wanted to restart the application.
    bool SetColorTheme();

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    /// @brief Handle Color scheme changed in the OS.
    ///
    /// color_scheme The color scheme selected by the OS.
    void HandleOsColorSchemeChanged(Qt::ColorScheme color_scheme);
#endif

private slots:
    // Handler for when the line edits lose focus.
    void HandleFocusOutEvent();

protected:
    virtual void showEvent(QShowEvent* event) override;

private:
    // Make the UI reflect the values in the supplied global settings struct.
    void PushToWidgets(const RgGlobalSettings& global_settings);

    // Use the values in the UI to create a global settings struct which
    // contains the pending changes.
    RgGlobalSettings PullFromWidgets() const;

    // Connect the signals.
    void ConnectSignals();

    // Create the controls responsible for picking the visible columns in the disassembly table.
    void CreateColumnVisibilityControls();

    // Populate the names in the column visibility list.
    void PopulateColumnVisibilityList();

    // Populate Font size dropdown.
    void PopulateFontSizeDropdown();

    // Set the cursor to pointing hand cursor for various widgets.
    void SetCursor();

    // Set the tooltip for the default program name check box.
    void SetCheckboxToolTip(const std::string& text);

    // Set the tooltips for the edit boxes.
    void SetEditBoxToolTips();

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

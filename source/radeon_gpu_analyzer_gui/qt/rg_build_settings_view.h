//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for Build settings view.
//=============================================================================

#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_BUILD_SETTINGS_VIEW_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_BUILD_SETTINGS_VIEW_H_

// C++.
#include <memory>

// Qt.
#include <QWidget>

// Local.
#include "source/radeon_gpu_analyzer_gui/qt/rg_settings_view.h"

// Forward declarations.
struct RgBuildSettings;

class RgBuildSettingsView : public RgSettingsView
{
    Q_OBJECT

public:
    RgBuildSettingsView(QWidget* parent, bool is_global_settings);
    virtual ~RgBuildSettingsView() = default;

public:
    // Does the settings view have any pending changes that need to be saved?
    virtual bool GetHasPendingChanges() const = 0;

    // Revert the pending changes.
    virtual bool RevertPendingChanges() = 0;

    // Restore the current setting values to defaults.
    virtual void RestoreDefaultSettings() = 0;

    // Save all pending settings to disk.
    virtual bool SaveSettings() = 0;

    // Get the title string.
    virtual std::string GetTitleString() = 0;

public slots:
    // Update the generated command line text.
    virtual void UpdateCommandLineText() = 0;

signals:
    // A signal emitted when the "has pending changes" state changes for the view.
    void PendingChangesStateChanged(bool has_pending_changes);

protected:
    // Subclasses can use this to set whether they have pending changes;
    // this will emit the pending changes signal as needed.
    void SetHasPendingChanges(bool has_pending_changes);

    // Flag used to indicate if the view relates to global or project-specific settings.
    bool is_global_settings_ = false;

    // Flag used to store whether a signal was emited regarding pending changes.
    bool has_pending_changes_ = false;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_BUILD_SETTINGS_VIEW_H_

//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for settings button view.
//=============================================================================
#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_SETTINGS_BUTTONS_VIEW_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_SETTINGS_BUTTONS_VIEW_H_

// C++.
#include <memory>
// Local.
#include "ui_rg_settings_buttons_view.h"
#include "source/radeon_gpu_analyzer_gui/rg_data_types.h"

// Forward declarations.
class QWidget;
class RgOpenCLBuildSettingsModel;

class RgSettingsButtonsView : public QWidget
{
    Q_OBJECT

public:
    RgSettingsButtonsView(QWidget* parent);
    virtual ~RgSettingsButtonsView();

    // Enable/disable save button.
    void EnableSaveButton(bool is_enabled);

    // Hide / show the Restore defaultSettings button.
    void HideRestoreDefaultSettingsButton(bool is_hidden);

    // Re-implement mousePressEvent method.
    virtual void mousePressEvent(QMouseEvent *event) override;

signals:
    void RestoreDefaultSettingsButtonClickedSignal();
    void SaveSettingsButtonClickedSignal();
    void SettingsButtonsViewClickedSignal();

private slots:
    void HandleRestoreDefaultSettingsButtonClick();
    void HandleSaveSettingsButtonClick();

private:
    // Connect the signals.
    void ConnectSignals();

    // Set the cursor to pointing hand cursor for various widgets.
    void SetCursor();

    // The generated interface view object.
    Ui::RgSettingsButtonsView ui_;

    // The action used to Restore settings.
    QAction* restore_settings_action_ = nullptr;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_SETTINGS_BUTTONS_VIEW_H_

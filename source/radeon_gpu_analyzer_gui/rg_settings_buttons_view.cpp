//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for settings button view.
//=============================================================================
// C++.
#include <cassert>
#include <sstream>

// Qt.
#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QWidget>

// Infra.
#include "qt_common/custom_widgets/arrow_icon_combo_box.h"
#include "qt_common/utils/common_definitions.h"
#include "qt_common/custom_widgets/list_widget.h"
#include "qt_common/utils/qt_util.h"
#include "qt_common/utils/restore_cursor_position.h"

// Local.
#include "radeon_gpu_analyzer_gui/rg_config_manager.h"
#include "radeon_gpu_analyzer_gui/rg_definitions.h"
#include "radeon_gpu_analyzer_gui/qt/rg_settings_buttons_view.h"
#include "radeon_gpu_analyzer_gui/qt/rg_hide_list_widget_event_filter.h"
#include "radeon_gpu_analyzer_gui/rg_string_constants.h"
#include "radeon_gpu_analyzer_gui/rg_utils.h"

RgSettingsButtonsView::RgSettingsButtonsView(QWidget* parent) :
    QWidget(parent)
{
    // Setup the UI.
    ui_.setupUi(this);

    // Connect the signals.
    ConnectSignals();

    // Set the mouse cursor to the pointing hand cursor for various widgets.
    SetCursor();

    // Disable the "Save" button located on build settings view initially.
    ui_.saveSettingsButton->setEnabled(false);
}

RgSettingsButtonsView::~RgSettingsButtonsView()
{
}

void RgSettingsButtonsView::ConnectSignals()
{
    // Restore default settings button.
    bool is_connected = connect(this->ui_.defaultSettingsPushButton, &QPushButton::clicked, this, &RgSettingsButtonsView::HandleRestoreDefaultSettingsButtonClick);
    assert(is_connected);

    // Save settings button.
    is_connected = connect(this->ui_.saveSettingsButton, &QPushButton::clicked, this, &RgSettingsButtonsView::HandleSaveSettingsButtonClick);
    assert(is_connected);

    restore_settings_action_ = new QAction(tr(kStrRestoreDefaultSettings), this);
    assert(restore_settings_action_ != nullptr);
    if (restore_settings_action_ != nullptr)
    {
        // Configure the hot key for the Restore default settings action.
        restore_settings_action_->setShortcut(QKeySequence(kRestoreDefaultSettings));

        // Connect the handler for the "Restore default settings" button hot key action.
        is_connected = connect(restore_settings_action_, &QAction::triggered, this, &RgSettingsButtonsView::HandleRestoreDefaultSettingsButtonClick);
        assert(is_connected);

        // Add a hot key action to the button.
        ui_.defaultSettingsPushButton->addAction(restore_settings_action_);
    }
}

void RgSettingsButtonsView::SetCursor()
{
    // Set the cursor to pointing hand cursor.
    ui_.defaultSettingsPushButton->setCursor(Qt::PointingHandCursor);
    ui_.saveSettingsButton->setCursor(Qt::PointingHandCursor);
}

void RgSettingsButtonsView::HandleRestoreDefaultSettingsButtonClick()
{
    // Emit the signal to indicate clicking of "Restore default settings" button.
    emit RestoreDefaultSettingsButtonClickedSignal();
}

void RgSettingsButtonsView::HandleSaveSettingsButtonClick()
{
    // Emit the signal to indicate clicking of "Save" button.
    emit SaveSettingsButtonClickedSignal();
}

void RgSettingsButtonsView::EnableSaveButton(bool is_enabled)
{
    ui_.saveSettingsButton->setEnabled(is_enabled);
}

void RgSettingsButtonsView::HideRestoreDefaultSettingsButton(bool is_hidden)
{
    ui_.defaultSettingsPushButton->setHidden(is_hidden);
}

void RgSettingsButtonsView::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);

    emit SettingsButtonsViewClickedSignal();
}

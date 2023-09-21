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
#include "QtCommon/CustomWidgets/ArrowIconWidget.h"
#include "QtCommon/Util/CommonDefinitions.h"
#include "QtCommon/CustomWidgets/ListWidget.h"
#include "QtCommon/Util/QtUtil.h"
#include "QtCommon/Util/RestoreCursorPosition.h"

// Local.
#include "radeon_gpu_analyzer_gui/rg_config_manager.h"
#include "radeon_gpu_analyzer_gui/rg_definitions.h"
#include "radeon_gpu_analyzer_gui/qt/rg_settings_buttons_view.h"
#include "radeon_gpu_analyzer_gui/qt/rg_hide_list_widget_event_filter.h"
#include "radeon_gpu_analyzer_gui/qt/rg_isa_disassembly_table_model.h"
#include "radeon_gpu_analyzer_gui/rg_string_constants.h"
#include "radeon_gpu_analyzer_gui/rg_utils.h"

RgSettingsButtonsView::RgSettingsButtonsView(QWidget* parent) :
    QWidget(parent)
{
    // Setup the UI.
    ui_.setupUi(this);

    // Set the background to white.
    QPalette pal = palette();
    pal.setColor(QPalette::Background, Qt::white);
    this->setAutoFillBackground(true);
    this->setPalette(pal);

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

//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for source code editor titlebar.
//=============================================================================

// C++.
#include <cassert>
#include <sstream>

// Local
#include "radeon_gpu_analyzer_gui/qt/rg_source_editor_titlebar.h"
#include "radeon_gpu_analyzer_gui/rg_string_constants.h"
#include "radeon_gpu_analyzer_gui/rg_utils.h"

RgSourceEditorTitlebar::RgSourceEditorTitlebar(QWidget* parent) :
    QFrame(parent)
{
    ui_.setupUi(this);

    // Initialize the title bar text and tooltip.
    std::stringstream titlebar_text;
    titlebar_text << kStrSourceEditorTitlebarCorrelationDisabledA;
    titlebar_text << kStrSourceEditorTitlebarCorrelationDisabledB;
    ui_.sourceCorrelationLabel->setText(titlebar_text.str().c_str());

    // Initialize the title bar contents to be hidden.
    SetTitlebarContentsVisibility(false);

    // Set the mouse cursor to pointing hand cursor.
    SetCursor();

    // Connect the signals.
    ConnectSignals();

    // Prep the dismiss message push button.
    ui_.dismissMessagePushButton->setIcon(QIcon(":/icons/delete_icon.svg"));
    ui_.dismissMessagePushButton->setStyleSheet("border: none");
}

void RgSourceEditorTitlebar::ConnectSignals()
{
    bool is_connected = connect(ui_.dismissMessagePushButton, &QPushButton::clicked, this, &RgSourceEditorTitlebar::HandleDismissMessagePushButtonClicked);
    assert(is_connected);

    is_connected = connect(ui_.dismissMessagePushButton, &QPushButton::clicked, this, &RgSourceEditorTitlebar::DismissMsgButtonClicked);
    assert(is_connected);
}

void RgSourceEditorTitlebar::HandleDismissMessagePushButtonClicked(/* bool checked */)
{
    SetTitlebarContentsVisibility(false);
}

void RgSourceEditorTitlebar::SetIsCorrelationEnabled(bool is_enabled)
{
    // Only show the title bar content if source line correlation is disabled.
    SetTitlebarContentsVisibility(!is_enabled);
}

void RgSourceEditorTitlebar::ShowMessage(const std::string& msg)
{
    ui_.sourceCorrelationLabel->setText(msg.c_str());
    SetTitlebarContentsVisibility(true);
}

void RgSourceEditorTitlebar::SetCursor()
{
    // Set the mouse cursor to pointing hand cursor.
    ui_.viewMaximizeButton->setCursor(Qt::PointingHandCursor);
    ui_.dismissMessagePushButton->setCursor(Qt::PointingHandCursor);
}

void RgSourceEditorTitlebar::SetTitlebarContentsVisibility(bool is_visible)
{
    ui_.sourceCorrelationIcon->setVisible(is_visible);
    ui_.sourceCorrelationLabel->setVisible(is_visible);
    ui_.dismissMessagePushButton->setVisible(is_visible);
}

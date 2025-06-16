//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for a Binary-specific implementation of the start tab.
//=============================================================================

// C++.
#include <cassert>

// Qt.
#include <QAction>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_start_tab_binary.h"
#include "radeon_gpu_analyzer_gui/qt/rg_app_state_binary.h"
#include "radeon_gpu_analyzer_gui/rg_data_types_binary.h"
#include "radeon_gpu_analyzer_gui/rg_utils.h"

RgStartTabBinary::RgStartTabBinary(QWidget* parent)
    : RgStartTab(parent)
{
    // Initialize the OpenCL start buttons.
    InitializeStartButtons();

    // Connect OpenCL start page signals.
    ConnectSignals();
}

void RgStartTabBinary::ApplyApiStringConstants()
{
    // Set label/button text.
    add_existing_code_obj_file_->setText(kStrMenuBarOpenExistingFileBinary);

    // Set tooltips and status tips.
    RgUtils::SetToolAndStatusTip(kStrMenuBarOpenExistingFileTooltipBinary, add_existing_code_obj_file_);
}

void RgStartTabBinary::GetStartButtons(std::vector<QPushButton*>& start_buttons)
{
    start_buttons.push_back(add_existing_code_obj_file_);
}

void RgStartTabBinary::InitializeStartButtons()
{
    // Create the "Create .cl file" button.
    add_existing_code_obj_file_ = new QPushButton(this);
    add_existing_code_obj_file_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    add_existing_code_obj_file_->setText(kStrMenuBarOpenExistingFileBinary);
    add_existing_code_obj_file_->setToolTip(kStrMenuBarOpenExistingFileTooltipBinary);
}

void RgStartTabBinary::ConnectSignals()
{
    // Create New File action.
    [[maybe_unused]] bool is_connected = connect(add_existing_code_obj_file_, &QPushButton::clicked, this, &RgStartTabBinary::OpenExistingCodeObjFileEvent);
    assert(is_connected);
}

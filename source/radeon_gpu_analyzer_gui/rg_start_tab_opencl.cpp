//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for a OpenCL-specific implementation of the start tab.
//=============================================================================

// C++.
#include <cassert>

// Qt.
#include <QAction>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_start_tab_opencl.h"
#include "radeon_gpu_analyzer_gui/qt/rg_app_state_opencl.h"
#include "radeon_gpu_analyzer_gui/rg_data_types_opencl.h"
#include "radeon_gpu_analyzer_gui/rg_utils.h"

RgStartTabOpencl::RgStartTabOpencl(QWidget* parent)
    : RgStartTab(parent)
{
    // Initialize the OpenCL start buttons.
    InitializeStartButtons();

    // Connect OpenCL start page signals.
    ConnectSignals();
}

void RgStartTabOpencl::ApplyApiStringConstants()
{
    // Set label/button text.
    create_new_cl_source_file_->setText(kStrMenuBarCreateNewFileOpencl);
    add_existing_cl_source_file_->setText(kStrMenuBarOpenExistingFileOpencl);

    // Set tooltips and status tips.
    RgUtils::SetToolAndStatusTip(kStrMenuBarCreateNewFileTooltipOpencl, create_new_cl_source_file_);
    RgUtils::SetToolAndStatusTip(kStrMenuBarOpenExistingFileTooltipOpencl, add_existing_cl_source_file_);
}

void RgStartTabOpencl::GetStartButtons(std::vector<QPushButton*>& start_buttons)
{
    start_buttons.push_back(create_new_cl_source_file_);
    start_buttons.push_back(add_existing_cl_source_file_);
}

void RgStartTabOpencl::InitializeStartButtons()
{
    // Create the "Create .cl file" button.
    create_new_cl_source_file_ = new QPushButton(this);
    create_new_cl_source_file_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    create_new_cl_source_file_->setText(kStrMenuBarCreateNewFileOpencl);
    create_new_cl_source_file_->setToolTip(kStrMenuBarCreateNewFileTooltipOpencl);

    // Create the "Add existing .cl file" button.
    add_existing_cl_source_file_ = new QPushButton(this);
    add_existing_cl_source_file_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    add_existing_cl_source_file_->setText(kStrMenuBarOpenExistingFileOpencl);
    add_existing_cl_source_file_->setToolTip(kStrMenuBarOpenExistingFileTooltipOpencl);
}

void RgStartTabOpencl::ConnectSignals()
{
    // Create New File action.
    bool is_connected = connect(create_new_cl_source_file_, &QPushButton::clicked, this, &RgStartTabOpencl::CreateNewCLFileEvent);
    assert(is_connected);

    // Open a file.
    is_connected =  connect(add_existing_cl_source_file_, &QPushButton::clicked, this, &RgStartTabOpencl::OpenExistingFileEvent);
    assert(is_connected);
}

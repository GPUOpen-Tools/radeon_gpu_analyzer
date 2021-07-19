// C++.
#include <cassert>

// Infra.
#include "QtCommon/Scaling/ScalingManager.h"

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_start_tab_vulkan.h"
#include "radeon_gpu_analyzer_gui/rg_data_types_vulkan.h"
#include "radeon_gpu_analyzer_gui/rg_utils.h"

RgStartTabVulkan::RgStartTabVulkan(QWidget* parent)
    : RgStartTab(parent)
{
    // Initialize the start buttons.
    InitializeStartButtons();

    // Connect the start page signals.
    ConnectSignals();
}

void RgStartTabVulkan::ApplyApiStringConstants()
{
    // Set label/button text.
    create_graphics_pipeline_push_button_->setText(kStrMenuBarCreateNewGraphicsPsoVulkan);
    create_compute_pipeline_push_button_->setText(kStrMenuBarCreateNewComputePsoVulkan);

    // Set tooltips and status tips.
    RgUtils::SetToolAndStatusTip(kStrMenuBarCreateNewGraphicsPsoTooltipVulkan, create_graphics_pipeline_push_button_);
    RgUtils::SetToolAndStatusTip(kStrMenuBarCreateNewComputePsoTooltipVulkan, create_compute_pipeline_push_button_);
}

void RgStartTabVulkan::GetStartButtons(std::vector<QPushButton*>& start_buttons)
{
    start_buttons.push_back(create_graphics_pipeline_push_button_);
    start_buttons.push_back(create_compute_pipeline_push_button_);
}

void RgStartTabVulkan::InitializeStartButtons()
{
    // Create the "Create graphics pipeline" button.
    create_graphics_pipeline_push_button_ = new QPushButton(this);
    create_graphics_pipeline_push_button_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    create_graphics_pipeline_push_button_->setText(kStrMenuBarCreateNewGraphicsPsoVulkan);

    // Create the "Create compute pipeline" button.
    create_compute_pipeline_push_button_ = new QPushButton(this);
    create_compute_pipeline_push_button_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    create_compute_pipeline_push_button_->setText(kStrMenuBarCreateNewComputePsoVulkan);
}

void RgStartTabVulkan::ConnectSignals()
{
    // Create new Graphics PSO action.
    bool is_connected = connect(create_graphics_pipeline_push_button_, &QPushButton::clicked, this, &RgStartTabVulkan::CreateGraphicsPipelineEvent);
    assert(is_connected);

    // Connect the new Compute PSO action.
    is_connected =  connect(create_compute_pipeline_push_button_, &QPushButton::clicked, this, &RgStartTabVulkan::CreateComputePipelineEvent);
    assert(is_connected);
}

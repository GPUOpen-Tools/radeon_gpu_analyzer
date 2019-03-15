// C++.
#include <cassert>

// Infra.
#include <QtCommon/Scaling/ScalingManager.h>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgStartTabVulkan.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDataTypesVulkan.h>
#include <RadeonGPUAnalyzerGUI/Include/rgUtils.h>

rgStartTabVulkan::rgStartTabVulkan(QWidget* pParent)
    : rgStartTab(pParent)
{
    // Initialize the start buttons.
    InitializeStartButtons();

    // Connect the start page signals.
    ConnectSignals();
}

void rgStartTabVulkan::ApplyApiStringConstants()
{
    // Set label/button text.
    m_pCreateGraphicsPipelinePushButton->setText(STR_MENU_BAR_CREATE_NEW_GRAPHICS_PSO_VULKAN);
    m_pCreateComputePipelinePushButton->setText(STR_MENU_BAR_CREATE_NEW_COMPUTE_PSO_VULKAN);

    // Set tooltips and status tips.
    rgUtils::SetToolAndStatusTip(STR_MENU_BAR_CREATE_NEW_GRAPHICS_PSO_TOOLTIP_VULKAN, m_pCreateGraphicsPipelinePushButton);
    rgUtils::SetToolAndStatusTip(STR_MENU_BAR_CREATE_NEW_COMPUTE_PSO_TOOLTIP_VULKAN, m_pCreateComputePipelinePushButton);
}

void rgStartTabVulkan::GetStartButtons(std::vector<QPushButton*>& startButtons)
{
    startButtons.push_back(m_pCreateGraphicsPipelinePushButton);
    startButtons.push_back(m_pCreateComputePipelinePushButton);
}

void rgStartTabVulkan::InitializeStartButtons()
{
    // Create the "Create graphics pipeline" button.
    m_pCreateGraphicsPipelinePushButton = new QPushButton(this);
    m_pCreateGraphicsPipelinePushButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_pCreateGraphicsPipelinePushButton->setText(STR_MENU_BAR_CREATE_NEW_GRAPHICS_PSO_VULKAN);

    // Create the "Create compute pipeline" button.
    m_pCreateComputePipelinePushButton = new QPushButton(this);
    m_pCreateComputePipelinePushButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_pCreateComputePipelinePushButton->setText(STR_MENU_BAR_CREATE_NEW_COMPUTE_PSO_VULKAN);
}

void rgStartTabVulkan::ConnectSignals()
{
    // Create new Graphics PSO action.
    bool isConnected = connect(m_pCreateGraphicsPipelinePushButton, &QPushButton::clicked, this, &rgStartTabVulkan::CreateGraphicsPipelineEvent);
    assert(isConnected);

    // Connect the new Compute PSO action.
    isConnected =  connect(m_pCreateComputePipelinePushButton, &QPushButton::clicked, this, &rgStartTabVulkan::CreateComputePipelineEvent);
    assert(isConnected);
}
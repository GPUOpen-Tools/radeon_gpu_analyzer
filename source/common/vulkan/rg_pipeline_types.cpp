//=============================================================================
/// Copyright (c) 2021-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for an vulkan pso types.
//=============================================================================
// C++.
#include <cassert>

// Local.
#include "source/common/vulkan/rg_pipeline_types.h"

// The default width & height dimensions for the viewport and scissor rect.
static const uint32_t S_DEFAULT_VIEWPORT_WIDTH = 1920;
static const uint32_t S_DEFAULT_VIEWPORT_HEIGHT = 1080;

// The default format to use for the PSO's renderpass color attachment.
static const VkFormat kDefaultAttachmentFormat = VK_FORMAT_B8G8R8A8_UNORM;

void RgPsoCreateInfoVulkan::InitializePipelineLayoutCreateInfo()
{
    // Zero out the create info structure.
    m_pPipelineLayoutCreateInfo = new VkPipelineLayoutCreateInfo{};
    m_pPipelineLayoutCreateInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    m_pPipelineLayoutCreateInfo->flags = 0;
    m_pPipelineLayoutCreateInfo->setLayoutCount = 0;
    m_pPipelineLayoutCreateInfo->pSetLayouts = nullptr;
    m_pPipelineLayoutCreateInfo->pushConstantRangeCount = 0;
    m_pPipelineLayoutCreateInfo->pPushConstantRanges = nullptr;
}

RgPsoGraphicsVulkan::~RgPsoGraphicsVulkan()
{
    if (m_pipelineCreateInfo.pStages != nullptr)
    {
        for (uint32_t i = 0; i < m_pipelineCreateInfo.stageCount; ++i)
        {
            if (m_pipelineCreateInfo.pStages[i].pSpecializationInfo != nullptr)
            {
                delete[] m_pipelineCreateInfo.pStages[i].pSpecializationInfo->pMapEntries;
                delete[] m_pipelineCreateInfo.pStages[i].pSpecializationInfo->pData;
                delete m_pipelineCreateInfo.pStages[i].pSpecializationInfo;
            }
            delete[] m_pipelineCreateInfo.pStages[i].pName;
        }
        delete[] m_pipelineCreateInfo.pStages;
        m_pipelineCreateInfo.pStages = nullptr;
    }

    if (m_pipelineCreateInfo.pDynamicState != nullptr)
    {
        delete[] m_pipelineCreateInfo.pDynamicState->pDynamicStates;
        delete m_pipelineCreateInfo.pDynamicState;
        m_pipelineCreateInfo.pDynamicState = nullptr;
    }

    if (m_pVertexInputStateCreateInfo != nullptr)
    {
        delete[] m_pVertexInputStateCreateInfo->pVertexAttributeDescriptions;
        m_pVertexInputStateCreateInfo->pVertexAttributeDescriptions = nullptr;
        delete[] m_pVertexInputStateCreateInfo->pVertexBindingDescriptions;
        m_pVertexInputStateCreateInfo->pVertexBindingDescriptions = nullptr;
        delete m_pVertexInputStateCreateInfo;
        m_pVertexInputStateCreateInfo = nullptr;
    }

    delete m_pInputAssemblyStateCreateInfo;
    m_pInputAssemblyStateCreateInfo = nullptr;

    delete m_pTessellationStateCreateInfo;
    m_pTessellationStateCreateInfo = nullptr;

    if (m_pViewportStateCreateInfo != nullptr)
    {
        delete[] m_pViewportStateCreateInfo->pScissors;
        m_pViewportStateCreateInfo->pScissors = nullptr;
        delete[] m_pViewportStateCreateInfo->pViewports;
        m_pViewportStateCreateInfo->pViewports = nullptr;
        delete m_pViewportStateCreateInfo;
        m_pViewportStateCreateInfo = nullptr;
    }

    delete m_pRasterizationStateCreateInfo;
    m_pRasterizationStateCreateInfo = nullptr;

    if (m_pMultisampleStateCreateInfo != nullptr)
    {
        delete[] m_pMultisampleStateCreateInfo->pSampleMask;
        m_pMultisampleStateCreateInfo->pSampleMask = nullptr;
        delete m_pMultisampleStateCreateInfo;
        m_pMultisampleStateCreateInfo = nullptr;
    }

    delete m_pDepthStencilStateCreateInfo;
    m_pDepthStencilStateCreateInfo = nullptr;

    if (m_pColorBlendStateCreateInfo != nullptr)
    {
        delete[] m_pColorBlendStateCreateInfo->pAttachments;
        m_pColorBlendStateCreateInfo->pAttachments = nullptr;
        delete m_pColorBlendStateCreateInfo;
        m_pColorBlendStateCreateInfo = nullptr;
    }

    if (m_pRenderPassCreateInfo != nullptr)
    {
        if (m_pRenderPassCreateInfo->pSubpasses != nullptr)
        {
            for (uint32_t i = 0; i < m_pRenderPassCreateInfo->subpassCount; ++i)
            {
                delete[] m_pRenderPassCreateInfo->pSubpasses[i].pInputAttachments;
                delete[] m_pRenderPassCreateInfo->pSubpasses[i].pColorAttachments;
                delete[] m_pRenderPassCreateInfo->pSubpasses[i].pResolveAttachments;
                delete m_pRenderPassCreateInfo->pSubpasses[i].pDepthStencilAttachment;
                delete[] m_pRenderPassCreateInfo->pSubpasses[i].pPreserveAttachments;
            }

            delete[] m_pRenderPassCreateInfo->pSubpasses;
            m_pRenderPassCreateInfo->pSubpasses = nullptr;
        }

        delete[] m_pRenderPassCreateInfo->pAttachments;
        m_pRenderPassCreateInfo->pAttachments = nullptr;
        delete[] m_pRenderPassCreateInfo->pDependencies;
        m_pRenderPassCreateInfo->pDependencies = nullptr;
        delete m_pRenderPassCreateInfo;
        m_pRenderPassCreateInfo = nullptr;
    }

    if (m_pPipelineLayoutCreateInfo != nullptr)
    {
        delete[] m_pPipelineLayoutCreateInfo->pSetLayouts;
        m_pPipelineLayoutCreateInfo->pSetLayouts = nullptr;
        delete[] m_pPipelineLayoutCreateInfo->pPushConstantRanges;
        m_pPipelineLayoutCreateInfo->pPushConstantRanges = nullptr;
        delete m_pPipelineLayoutCreateInfo;
        m_pPipelineLayoutCreateInfo = nullptr;
    }

    for (VkDescriptorSetLayoutCreateInfo* descriptor_set_layout_create_info : m_descriptorSetLayoutCreateInfo)
    {
        if (descriptor_set_layout_create_info != nullptr)
        {
            delete[] descriptor_set_layout_create_info->pBindings;
            descriptor_set_layout_create_info->pBindings = nullptr;
            delete descriptor_set_layout_create_info;
            descriptor_set_layout_create_info = nullptr;
        }
    }
    m_descriptorSetLayoutCreateInfo.clear();
}

void RgPsoGraphicsVulkan::Initialize()
{
    // Zero out the initial state structure.
    InitializeGraphicsPipelineStateCreateInfo();

    // Initialize each CreateInfo sub-structure with suitable default values.
    InitializeVertexInputStateCreateInfo();
    InitializeInputAssemblyStateCreateInfo();
    InitializeTessellationStateCreateInfo();
    InitializeViewportStateCreateInfo();
    InitializeRasterizationStateCreateInfo();
    InitializeMultisampleStateCreateInfo();
    InitializeDepthStencilStateCreateInfo();
    InitializeColorBlendStateCreateInfo();

    // Initialize the pipeline layout create info.
    InitializePipelineLayoutCreateInfo();

    // Initialize the render pass create info.
    InitializeRenderPassCreateInfo();
}

VkRenderPassCreateInfo* RgPsoGraphicsVulkan::GetRenderPassCreateInfo()
{
    return m_pRenderPassCreateInfo;
}

VkGraphicsPipelineCreateInfo* RgPsoGraphicsVulkan::GetGraphicsPipelineCreateInfo()
{
    return &m_pipelineCreateInfo;
}

VkPipelineVertexInputStateCreateInfo* RgPsoGraphicsVulkan::GetPipelineVertexInputStateCreateInfo()
{
    return m_pVertexInputStateCreateInfo;
}

VkPipelineInputAssemblyStateCreateInfo* RgPsoGraphicsVulkan::GetPipelineInputAssemblyStateCreateInfo()
{
    return m_pInputAssemblyStateCreateInfo;
}

VkPipelineTessellationStateCreateInfo* RgPsoGraphicsVulkan::GetPipelineTessellationStateCreateInfo()
{
    return m_pTessellationStateCreateInfo;
}

VkPipelineViewportStateCreateInfo* RgPsoGraphicsVulkan::GetPipelineViewportStateCreateInfo()
{
    return m_pViewportStateCreateInfo;
}

VkPipelineRasterizationStateCreateInfo* RgPsoGraphicsVulkan::GetPipelineRasterizationStateCreateInfo()
{
    return m_pRasterizationStateCreateInfo;
}

VkPipelineMultisampleStateCreateInfo* RgPsoGraphicsVulkan::GetPipelineMultisampleStateCreateInfo()
{
    return m_pMultisampleStateCreateInfo;
}

VkPipelineDepthStencilStateCreateInfo* RgPsoGraphicsVulkan::GetPipelineDepthStencilStateCreateInfo()
{
    return m_pDepthStencilStateCreateInfo;
}

VkPipelineColorBlendStateCreateInfo* RgPsoGraphicsVulkan::GetPipelineColorBlendStateCreateInfo()
{
    return m_pColorBlendStateCreateInfo;
}

VkPipelineLayoutCreateInfo* RgPsoCreateInfoVulkan::GetPipelineLayoutCreateInfo()
{
    return m_pPipelineLayoutCreateInfo;
}

std::vector<VkDescriptorSetLayoutCreateInfo*>& RgPsoCreateInfoVulkan::GetDescriptorSetLayoutCreateInfo()
{
    return m_descriptorSetLayoutCreateInfo;
}

const std::vector<VkDescriptorSetLayoutBinding*> RgPsoCreateInfoVulkan::GetDescriptorSetLayoutBinding() const
{
    return m_descriptorSetLayoutBindings;
}

const std::vector<VkSamplerCreateInfo*> RgPsoCreateInfoVulkan::GetSamplerCreateInfo() const
{
    return m_samplerCreateInfo;
}

void RgPsoCreateInfoVulkan::AddDescriptorSetLayoutCreateInfo(VkDescriptorSetLayoutCreateInfo* descriptor_set_layout_create_info)
{
    // Add the item to our descriptor set layout collection.
    m_descriptorSetLayoutCreateInfo.push_back(descriptor_set_layout_create_info);

    // Update the size from the pipeline layout create info to match the descriptor set layout.
    m_pPipelineLayoutCreateInfo->setLayoutCount = static_cast<uint32_t>(m_descriptorSetLayoutCreateInfo.size());
}

void RgPsoGraphicsVulkan::InitializeGraphicsPipelineStateCreateInfo()
{
    // Zero out all fields before initializing defaults.
    m_pipelineCreateInfo = {};

    m_pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    m_pipelineCreateInfo.flags = 0;
    m_pipelineCreateInfo.stageCount = 0;
    m_pipelineCreateInfo.layout = VK_NULL_HANDLE;
    m_pipelineCreateInfo.renderPass = VK_NULL_HANDLE;
    m_pipelineCreateInfo.subpass = 0;
    m_pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
    m_pipelineCreateInfo.basePipelineIndex = -1;

    // Assign the default CreateInfo structures to the main Graphics Pipeline CreateInfo struct.
    m_pipelineCreateInfo.pVertexInputState = m_pVertexInputStateCreateInfo;
    m_pipelineCreateInfo.pInputAssemblyState = m_pInputAssemblyStateCreateInfo;
    m_pipelineCreateInfo.pTessellationState = m_pTessellationStateCreateInfo;
    m_pipelineCreateInfo.pViewportState = m_pViewportStateCreateInfo;
    m_pipelineCreateInfo.pRasterizationState = m_pRasterizationStateCreateInfo;
    m_pipelineCreateInfo.pMultisampleState = m_pMultisampleStateCreateInfo;
    m_pipelineCreateInfo.pDepthStencilState = m_pDepthStencilStateCreateInfo;
    m_pipelineCreateInfo.pColorBlendState = m_pColorBlendStateCreateInfo;

    // Note: Dynamic state configuration is not utilized in PSO creation.
    // It is therefore purposely excluded from being initialized here.
}

void RgPsoGraphicsVulkan::InitializeVertexInputStateCreateInfo()
{
    m_pVertexInputStateCreateInfo = new VkPipelineVertexInputStateCreateInfo{};
    m_pVertexInputStateCreateInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    m_pVertexInputStateCreateInfo->flags = 0;
    m_pVertexInputStateCreateInfo->vertexBindingDescriptionCount = 0;
    m_pVertexInputStateCreateInfo->pVertexBindingDescriptions = nullptr;
    m_pVertexInputStateCreateInfo->vertexAttributeDescriptionCount = 0;
    m_pVertexInputStateCreateInfo->pVertexAttributeDescriptions = nullptr;

    // Assign to the graphics pipeline state create info structure.
    m_pipelineCreateInfo.pVertexInputState = m_pVertexInputStateCreateInfo;
}

void RgPsoGraphicsVulkan::InitializeInputAssemblyStateCreateInfo()
{
    m_pInputAssemblyStateCreateInfo = new VkPipelineInputAssemblyStateCreateInfo{};
    m_pInputAssemblyStateCreateInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    m_pInputAssemblyStateCreateInfo->flags = 0;
    m_pInputAssemblyStateCreateInfo->topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    m_pInputAssemblyStateCreateInfo->primitiveRestartEnable = VK_FALSE;

    // Assign to the graphics pipeline state create info structure.
    m_pipelineCreateInfo.pInputAssemblyState = m_pInputAssemblyStateCreateInfo;
}

void RgPsoGraphicsVulkan::InitializeTessellationStateCreateInfo()
{
    m_pTessellationStateCreateInfo = new VkPipelineTessellationStateCreateInfo{};
    m_pTessellationStateCreateInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
    m_pTessellationStateCreateInfo->flags = 0;
    m_pTessellationStateCreateInfo->patchControlPoints = 0;

    // Assign to the graphics pipeline state create info structure.
    m_pipelineCreateInfo.pTessellationState = m_pTessellationStateCreateInfo;
}

void RgPsoGraphicsVulkan::InitializeViewportStateCreateInfo()
{
    m_pViewportStateCreateInfo = new VkPipelineViewportStateCreateInfo{};
    m_pViewportStateCreateInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    m_pViewportStateCreateInfo->flags = 0;
    m_pViewportStateCreateInfo->viewportCount = 1;
    m_pViewportStateCreateInfo->pViewports = new VkViewport{ 0, 0,
        static_cast<float>(S_DEFAULT_VIEWPORT_WIDTH), static_cast<float>(S_DEFAULT_VIEWPORT_HEIGHT),
        0, 1 };
    m_pViewportStateCreateInfo->scissorCount = 1;
    m_pViewportStateCreateInfo->pScissors = new VkRect2D{ { 0, 0 },
        { S_DEFAULT_VIEWPORT_WIDTH, S_DEFAULT_VIEWPORT_HEIGHT } };

    // Assign to the graphics pipeline state create info structure.
    m_pipelineCreateInfo.pViewportState = m_pViewportStateCreateInfo;
}

void RgPsoGraphicsVulkan::InitializeRasterizationStateCreateInfo()
{
    m_pRasterizationStateCreateInfo = new VkPipelineRasterizationStateCreateInfo{};
    m_pRasterizationStateCreateInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    m_pRasterizationStateCreateInfo->flags = 0;
    m_pRasterizationStateCreateInfo->depthClampEnable = VK_FALSE;
    m_pRasterizationStateCreateInfo->rasterizerDiscardEnable = VK_FALSE;
    m_pRasterizationStateCreateInfo->polygonMode = VK_POLYGON_MODE_FILL;
    m_pRasterizationStateCreateInfo->cullMode = VK_CULL_MODE_BACK_BIT;
    m_pRasterizationStateCreateInfo->frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    m_pRasterizationStateCreateInfo->depthBiasEnable = VK_FALSE;
    m_pRasterizationStateCreateInfo->depthBiasConstantFactor = 0.0f;
    m_pRasterizationStateCreateInfo->depthBiasClamp = 0.0f;
    m_pRasterizationStateCreateInfo->depthBiasSlopeFactor = 0.0f;
    m_pRasterizationStateCreateInfo->lineWidth = 1.0f;

    // Assign to the graphics pipeline state create info structure.
    m_pipelineCreateInfo.pRasterizationState = m_pRasterizationStateCreateInfo;
}

void RgPsoGraphicsVulkan::InitializeMultisampleStateCreateInfo()
{
    m_pMultisampleStateCreateInfo = new VkPipelineMultisampleStateCreateInfo{};
    m_pMultisampleStateCreateInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    m_pMultisampleStateCreateInfo->flags = 0;
    m_pMultisampleStateCreateInfo->rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    m_pMultisampleStateCreateInfo->sampleShadingEnable = VK_FALSE;
    m_pMultisampleStateCreateInfo->minSampleShading = 1.0f;
    m_pMultisampleStateCreateInfo->pSampleMask = nullptr;
    m_pMultisampleStateCreateInfo->alphaToCoverageEnable = VK_FALSE;
    m_pMultisampleStateCreateInfo->alphaToOneEnable = VK_FALSE;

    // Assign to the graphics pipeline state create info structure.
    m_pipelineCreateInfo.pMultisampleState = m_pMultisampleStateCreateInfo;
}

void RgPsoGraphicsVulkan::InitializeDepthStencilStateCreateInfo()
{
    m_pDepthStencilStateCreateInfo = new VkPipelineDepthStencilStateCreateInfo{};
    m_pDepthStencilStateCreateInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    m_pDepthStencilStateCreateInfo->flags = 0;
    m_pDepthStencilStateCreateInfo->depthTestEnable = VK_FALSE;
    m_pDepthStencilStateCreateInfo->depthWriteEnable = VK_FALSE;
    m_pDepthStencilStateCreateInfo->depthCompareOp = VK_COMPARE_OP_LESS;
    m_pDepthStencilStateCreateInfo->depthBoundsTestEnable = VK_FALSE;
    m_pDepthStencilStateCreateInfo->stencilTestEnable = VK_FALSE;
    InitializeDefaultStencilOpState(m_pDepthStencilStateCreateInfo->front);
    InitializeDefaultStencilOpState(m_pDepthStencilStateCreateInfo->back);
    m_pDepthStencilStateCreateInfo->minDepthBounds = 0.0f;
    m_pDepthStencilStateCreateInfo->maxDepthBounds = 1.0f;

    // Assign to the graphics pipeline state create info structure.
    m_pipelineCreateInfo.pDepthStencilState = m_pDepthStencilStateCreateInfo;
}

void RgPsoGraphicsVulkan::InitializeColorBlendStateCreateInfo()
{
    m_pColorBlendStateCreateInfo = new VkPipelineColorBlendStateCreateInfo{};
    m_pColorBlendStateCreateInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    m_pColorBlendStateCreateInfo->flags = 0;
    m_pColorBlendStateCreateInfo->logicOpEnable = VK_FALSE;
    m_pColorBlendStateCreateInfo->logicOp = VK_LOGIC_OP_NO_OP;
    m_pColorBlendStateCreateInfo->attachmentCount = 0;
    m_pColorBlendStateCreateInfo->pAttachments = nullptr;
    m_pColorBlendStateCreateInfo->blendConstants[0] = 0.0f;
    m_pColorBlendStateCreateInfo->blendConstants[1] = 0.0f;
    m_pColorBlendStateCreateInfo->blendConstants[2] = 0.0f;
    m_pColorBlendStateCreateInfo->blendConstants[3] = 0.0f;

    // Assign to the graphics pipeline state create info structure.
    m_pipelineCreateInfo.pColorBlendState = m_pColorBlendStateCreateInfo;
}

void RgPsoGraphicsVulkan::InitializeDefaultStencilOpState(VkStencilOpState& createInfo)
{
    createInfo = {};
    createInfo.failOp = VK_STENCIL_OP_KEEP;
    createInfo.passOp = VK_STENCIL_OP_KEEP;
    createInfo.depthFailOp = VK_STENCIL_OP_KEEP;
    createInfo.compareOp = VK_COMPARE_OP_NEVER;
    createInfo.compareMask = 0;
    createInfo.writeMask = 0;
    createInfo.reference = 0;
}

void RgPsoGraphicsVulkan::InitializeRenderPassCreateInfo()
{
    // Zero out the create info structure.
    m_pRenderPassCreateInfo = new VkRenderPassCreateInfo{};

    // A single color buffer attachment for 1 image in the swap chain.
    VkAttachmentDescription* pColorAttachment = new VkAttachmentDescription{};

    pColorAttachment->format = kDefaultAttachmentFormat;
    pColorAttachment->samples = VK_SAMPLE_COUNT_1_BIT;

    // Want to clear color data before rendering, and store the results.
    pColorAttachment->loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    pColorAttachment->storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    pColorAttachment->stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    pColorAttachment->stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    // Don't know the initial state of the attachment,
    // but when completed, the attachment should be ready to be presented.
    pColorAttachment->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    pColorAttachment->finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // Create a color attachment reference. Fragment shaders can write to this default attachment.
    VkAttachmentReference* pColorAttachmentRef = new VkAttachmentReference{};
    pColorAttachmentRef->attachment = 0;
    pColorAttachmentRef->layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription* pSubpass = new VkSubpassDescription{};
    pSubpass->pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    pSubpass->colorAttachmentCount = 1;
    pSubpass->pColorAttachments = pColorAttachmentRef;

    VkSubpassDependency* pDependency = new VkSubpassDependency{};
    pDependency->srcSubpass = VK_SUBPASS_EXTERNAL;
    pDependency->dstSubpass = 0;
    pDependency->srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    pDependency->srcAccessMask = 0;
    pDependency->dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    pDependency->dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    m_pRenderPassCreateInfo->sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    m_pRenderPassCreateInfo->attachmentCount = 1;
    m_pRenderPassCreateInfo->pAttachments = pColorAttachment;
    m_pRenderPassCreateInfo->subpassCount = 1;
    m_pRenderPassCreateInfo->pSubpasses = pSubpass;
    m_pRenderPassCreateInfo->dependencyCount = 1;
    m_pRenderPassCreateInfo->pDependencies = pDependency;
}

void RgPsoComputeVulkan::Initialize()
{
    // Zero out the initial state structure.
    m_pipelineCreateInfo = {};

    // Initialize the default compute pipeline state create info.
    InitializeComputePipelineStateCreateInfo();

    // Initialize the pipeline layout create info.
    InitializePipelineLayoutCreateInfo();
}

void RgPsoComputeVulkan::InitializeComputePipelineStateCreateInfo()
{
    // Initialize the default compute pipeline state create info.
    m_pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    m_pipelineCreateInfo.flags = 0;
    m_pipelineCreateInfo.layout = VK_NULL_HANDLE;
    m_pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
    m_pipelineCreateInfo.basePipelineIndex = -1;

    // Initialize the default compute stage info.
    m_pipelineCreateInfo.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    m_pipelineCreateInfo.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
}

VkComputePipelineCreateInfo* RgPsoComputeVulkan::GetComputePipelineCreateInfo()
{
    return &m_pipelineCreateInfo;
}


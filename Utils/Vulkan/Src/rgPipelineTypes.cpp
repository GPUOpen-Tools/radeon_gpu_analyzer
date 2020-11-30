// C++.
#include <cassert>

// Local.
#include <Utils/Vulkan/Include/rgPipelineTypes.h>

// The default width & height dimensions for the viewport and scissor rect.
static const uint32_t S_DEFAULT_VIEWPORT_WIDTH = 1920;
static const uint32_t S_DEFAULT_VIEWPORT_HEIGHT = 1080;

// The default format to use for the PSO's renderpass color attachment.
static const VkFormat kDefaultAttachmentFormat = VK_FORMAT_B8G8R8A8_UNORM;

void rgPsoCreateInfoVulkan::InitializePipelineLayoutCreateInfo()
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

void rgPsoGraphicsVulkan::Initialize()
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

VkRenderPassCreateInfo* rgPsoGraphicsVulkan::GetRenderPassCreateInfo()
{
    return m_pRenderPassCreateInfo;
}

VkGraphicsPipelineCreateInfo* rgPsoGraphicsVulkan::GetGraphicsPipelineCreateInfo()
{
    return &m_pipelineCreateInfo;
}

VkPipelineVertexInputStateCreateInfo* rgPsoGraphicsVulkan::GetPipelineVertexInputStateCreateInfo()
{
    return m_pVertexInputStateCreateInfo;
}

VkPipelineInputAssemblyStateCreateInfo* rgPsoGraphicsVulkan::GetPipelineInputAssemblyStateCreateInfo()
{
    return m_pInputAssemblyStateCreateInfo;
}

VkPipelineTessellationStateCreateInfo* rgPsoGraphicsVulkan::GetPipelineTessellationStateCreateInfo()
{
    return m_pTessellationStateCreateInfo;
}

VkPipelineViewportStateCreateInfo* rgPsoGraphicsVulkan::GetPipelineViewportStateCreateInfo()
{
    return m_pViewportStateCreateInfo;
}

VkPipelineRasterizationStateCreateInfo* rgPsoGraphicsVulkan::GetPipelineRasterizationStateCreateInfo()
{
    return m_pRasterizationStateCreateInfo;
}

VkPipelineMultisampleStateCreateInfo* rgPsoGraphicsVulkan::GetPipelineMultisampleStateCreateInfo()
{
    return m_pMultisampleStateCreateInfo;
}

VkPipelineDepthStencilStateCreateInfo* rgPsoGraphicsVulkan::GetPipelineDepthStencilStateCreateInfo()
{
    return m_pDepthStencilStateCreateInfo;
}

VkPipelineColorBlendStateCreateInfo* rgPsoGraphicsVulkan::GetPipelineColorBlendStateCreateInfo()
{
    return m_pColorBlendStateCreateInfo;
}

VkPipelineLayoutCreateInfo* rgPsoCreateInfoVulkan::GetPipelineLayoutCreateInfo()
{
    return m_pPipelineLayoutCreateInfo;
}

std::vector<VkDescriptorSetLayoutCreateInfo*>& rgPsoCreateInfoVulkan::GetDescriptorSetLayoutCreateInfo()
{
    return m_descriptorSetLayoutCreateInfo;
}

const std::vector<VkDescriptorSetLayoutBinding*> rgPsoCreateInfoVulkan::GetDescriptorSetLayoutBinding() const
{
    return m_descriptorSetLayoutBindings;
}

const std::vector<VkSamplerCreateInfo*> rgPsoCreateInfoVulkan::GetSamplerCreateInfo() const
{
    return m_samplerCreateInfo;
}

void rgPsoCreateInfoVulkan::AddDescriptorSetLayoutCreateInfo(VkDescriptorSetLayoutCreateInfo* descriptor_set_layout_create_info)
{
    // Add the item to our descriptor set layout collection.
    m_descriptorSetLayoutCreateInfo.push_back(descriptor_set_layout_create_info);

    // Update the size from the pipeline layout create info to match the descriptor set layout.
    m_pPipelineLayoutCreateInfo->setLayoutCount = static_cast<uint32_t>(m_descriptorSetLayoutCreateInfo.size());
}

void rgPsoGraphicsVulkan::InitializeGraphicsPipelineStateCreateInfo()
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

void rgPsoGraphicsVulkan::InitializeVertexInputStateCreateInfo()
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

void rgPsoGraphicsVulkan::InitializeInputAssemblyStateCreateInfo()
{
    m_pInputAssemblyStateCreateInfo = new VkPipelineInputAssemblyStateCreateInfo{};
    m_pInputAssemblyStateCreateInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    m_pInputAssemblyStateCreateInfo->flags = 0;
    m_pInputAssemblyStateCreateInfo->topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    m_pInputAssemblyStateCreateInfo->primitiveRestartEnable = VK_FALSE;

    // Assign to the graphics pipeline state create info structure.
    m_pipelineCreateInfo.pInputAssemblyState = m_pInputAssemblyStateCreateInfo;
}

void rgPsoGraphicsVulkan::InitializeTessellationStateCreateInfo()
{
    m_pTessellationStateCreateInfo = new VkPipelineTessellationStateCreateInfo{};
    m_pTessellationStateCreateInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
    m_pTessellationStateCreateInfo->flags = 0;
    m_pTessellationStateCreateInfo->patchControlPoints = 0;

    // Assign to the graphics pipeline state create info structure.
    m_pipelineCreateInfo.pTessellationState = m_pTessellationStateCreateInfo;
}

void rgPsoGraphicsVulkan::InitializeViewportStateCreateInfo()
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

void rgPsoGraphicsVulkan::InitializeRasterizationStateCreateInfo()
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

void rgPsoGraphicsVulkan::InitializeMultisampleStateCreateInfo()
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

void rgPsoGraphicsVulkan::InitializeDepthStencilStateCreateInfo()
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

void rgPsoGraphicsVulkan::InitializeColorBlendStateCreateInfo()
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

void rgPsoGraphicsVulkan::InitializeDefaultStencilOpState(VkStencilOpState& createInfo)
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

void rgPsoGraphicsVulkan::InitializeRenderPassCreateInfo()
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

void rgPsoComputeVulkan::Initialize()
{
    // Zero out the initial state structure.
    m_pipelineCreateInfo = {};

    // Initialize the default compute pipeline state create info.
    InitializeComputePipelineStateCreateInfo();

    // Initialize the pipeline layout create info.
    InitializePipelineLayoutCreateInfo();
}

void rgPsoComputeVulkan::InitializeComputePipelineStateCreateInfo()
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

VkComputePipelineCreateInfo* rgPsoComputeVulkan::GetComputePipelineCreateInfo()
{
    return &m_pipelineCreateInfo;
}


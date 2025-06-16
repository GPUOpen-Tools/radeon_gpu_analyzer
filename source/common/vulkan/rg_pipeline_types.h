//=============================================================================
/// Copyright (c) 2021-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for an vulkan pso types.
//=============================================================================

#pragma once

// C++.
#include <vector>

// Volk.
#include <volk/volk.h>

// Local.
#include "source/common/rga_shared_data_types.h"

// Wrapper for VkPipelineLayoutCreateInfo.
struct rgVkPipelineLayoutCreateInfo
{
    // CTOR.
    rgVkPipelineLayoutCreateInfo() = default;

    // Do not free the memory - leave that for the code that instantiates this object.
    ~rgVkPipelineLayoutCreateInfo() = default;

    // The original object.
    VkPipelineLayoutCreateInfo* m_pVkPipelineLayoutCreateInfo = new VkPipelineLayoutCreateInfo{};

    // The indices for where to get the descriptor set layout create info
    // objects in the container of descriptor set layout create info objects.
    std::vector<size_t> m_descriptorSetLayoutIndices;
};

// A structure containing all Vulkan create info relevant to either graphics and compute pipelines.
class RgPsoCreateInfoVulkan : public RgPsoCreateInfo
{
public:

    RgPsoCreateInfoVulkan() = default;
    virtual ~RgPsoCreateInfoVulkan() = default;

    // Retrieve a pointer to the pipeline layout create info structure.
    VkPipelineLayoutCreateInfo* GetPipelineLayoutCreateInfo();

    // Retrieve a pointer to the descriptor set layout create info structure.
    std::vector<VkDescriptorSetLayoutCreateInfo*>& GetDescriptorSetLayoutCreateInfo();

    // Retrieve a pointer to the descriptor set layout binding create info structure.
    const std::vector<VkDescriptorSetLayoutBinding*> GetDescriptorSetLayoutBinding() const;

    // Get the pointer to the sampler create info structures.
    const std::vector<VkSamplerCreateInfo*> GetSamplerCreateInfo() const;

    // Add the given descriptor set to the pipeline state create info.
    void AddDescriptorSetLayoutCreateInfo(VkDescriptorSetLayoutCreateInfo* descriptor_set_layout_create_info);

protected:
    // Initialize the default pipeline layout create info.
    void InitializePipelineLayoutCreateInfo();

    // The pipeline layout create info.
    VkPipelineLayoutCreateInfo* m_pPipelineLayoutCreateInfo = nullptr;

    // The descriptor set layout create info.
    std::vector<VkDescriptorSetLayoutCreateInfo*> m_descriptorSetLayoutCreateInfo = {};

    // The descriptor set layout create info.
    std::vector<VkDescriptorSetLayoutBinding*> m_descriptorSetLayoutBindings = {};

    // The sampler create info.
    std::vector<VkSamplerCreateInfo*> m_samplerCreateInfo = {};
};

// A structure containing all create info necessary to instantiate a new Vulkan graphics pipeline.
class RgPsoGraphicsVulkan : public RgPsoCreateInfoVulkan
{
public:
    RgPsoGraphicsVulkan() = default;
    virtual ~RgPsoGraphicsVulkan() = default;

    // Initialize the graphics pipeline create info using suitable defaults.
    void Initialize();

    // *** Setters - BEGIN. ***

    void SetRenderPassCreateInfo(VkRenderPassCreateInfo* render_pass_create_info) { m_pRenderPassCreateInfo = render_pass_create_info; }
    void SetMultisampleStateCreateInfo(VkPipelineMultisampleStateCreateInfo* pMultiSampleStateCreateInfo) { /*m_pMultisampleStateCreateInfo*/ m_pipelineCreateInfo.pMultisampleState = pMultiSampleStateCreateInfo; }
    void SetVertexInputStateCreateInfo(VkPipelineVertexInputStateCreateInfo* pVertexInputStateCreateInfo) { /*m_pVertexInputStateCreateInfo*/ m_pipelineCreateInfo.pVertexInputState = pVertexInputStateCreateInfo; }
    void SetRasterizationStateCreateInfo(VkPipelineRasterizationStateCreateInfo* pRasterizationStateCreateInfo) { /*m_pRasterizationStateCreateInfo*/ m_pipelineCreateInfo.pRasterizationState = pRasterizationStateCreateInfo; }
    void SetInputAssemblyStateCreateInfo(VkPipelineInputAssemblyStateCreateInfo* pInputAssemblyStateCreateInfo) { /*m_pInputAssemblyStateCreateInfo*/ m_pipelineCreateInfo.pInputAssemblyState = pInputAssemblyStateCreateInfo; }
    void SetColorBlendStateCreateInfo(VkPipelineColorBlendStateCreateInfo* pColorBlendStateCreateInfo) { /*m_pColorBlendStateCreateInfo*/ m_pipelineCreateInfo.pColorBlendState = pColorBlendStateCreateInfo; }
    void SetViweportStateCreateInfo(VkPipelineViewportStateCreateInfo* pViewportStateCreateInfo) { /*m_pViewportStateCreateInfo*/ m_pipelineCreateInfo.pViewportState = pViewportStateCreateInfo; }
    void SetDepthStencilStateCreateInfo(VkPipelineDepthStencilStateCreateInfo* pDepthStencilStateCreateInfo) { /*m_pDepthStencilStateCreateInfo*/ m_pipelineCreateInfo.pDepthStencilState = pDepthStencilStateCreateInfo; }
    void SetPipelineLayoutCreateInfo(VkPipelineLayoutCreateInfo* pipeline_layout_create_info) { m_pPipelineLayoutCreateInfo = pipeline_layout_create_info; }

    // *** Setters - END. ***

    // Retrieve a pointer to the Render Pass create info structure.
    VkRenderPassCreateInfo* GetRenderPassCreateInfo();

    // Retrieve a pointer to the graphics pipeline create info structure.
    VkGraphicsPipelineCreateInfo* GetGraphicsPipelineCreateInfo();

    // Retrieve a pointer to the graphics pipeline Vertex Input state create info structure.
    VkPipelineVertexInputStateCreateInfo* GetPipelineVertexInputStateCreateInfo();

    // Retrieve a pointer to the graphics pipeline Input Assembly state create info structure.
    VkPipelineInputAssemblyStateCreateInfo* GetPipelineInputAssemblyStateCreateInfo();

    // Retrieve a pointer to the graphics pipeline Tessellation State create info structure.
    VkPipelineTessellationStateCreateInfo* GetPipelineTessellationStateCreateInfo();

    // Retrieve a pointer to the graphics pipeline Viewport State create info structure.
    VkPipelineViewportStateCreateInfo* GetPipelineViewportStateCreateInfo();

    // Retrieve a pointer to the graphics pipeline Rasterization State create info structure.
    VkPipelineRasterizationStateCreateInfo* GetPipelineRasterizationStateCreateInfo();

    // Retrieve a pointer to the graphics pipeline Multisample State create info structure.
    VkPipelineMultisampleStateCreateInfo* GetPipelineMultisampleStateCreateInfo();

    // Retrieve a pointer to the graphics pipeline Depth Stencil State create info structure.
    VkPipelineDepthStencilStateCreateInfo* GetPipelineDepthStencilStateCreateInfo();

    // Retrieve a pointer to the graphics pipeline Color Blend State create info structure.
    VkPipelineColorBlendStateCreateInfo* GetPipelineColorBlendStateCreateInfo();

private:
    // Initialize default graphics pipeline state create info.
    void InitializeGraphicsPipelineStateCreateInfo();

    // Initialize default vertex input state create info.
    void InitializeVertexInputStateCreateInfo();

    // Initialize default input assembly state create info.
    void InitializeInputAssemblyStateCreateInfo();

    // Initialize default tessellation state create info.
    void InitializeTessellationStateCreateInfo();

    // Initialize default viewport state create info.
    void InitializeViewportStateCreateInfo();

    // Initialize default rasterization state create info.
    void InitializeRasterizationStateCreateInfo();

    // Initialize default multisampling state create info.
    void InitializeMultisampleStateCreateInfo();

    // Initialize default depth stencil state create info.
    void InitializeDepthStencilStateCreateInfo();

    // Initialize default color blend state create info.
    void InitializeColorBlendStateCreateInfo();

    // Initialize the default Stencil Op state.
    void InitializeDefaultStencilOpState(VkStencilOpState& createInfo);

    // Initialize the default render pass create info.
    void InitializeRenderPassCreateInfo();

    // Graphics pipeline create info.
    VkGraphicsPipelineCreateInfo m_pipelineCreateInfo = {};

    // Vertex input state create info.
    VkPipelineVertexInputStateCreateInfo*      m_pVertexInputStateCreateInfo = nullptr;

    // Input assembly state create info.
    VkPipelineInputAssemblyStateCreateInfo*    m_pInputAssemblyStateCreateInfo = nullptr;

    // Tessellation state create info.
    VkPipelineTessellationStateCreateInfo*     m_pTessellationStateCreateInfo = nullptr;

    // Viewport state create info.
    VkPipelineViewportStateCreateInfo*         m_pViewportStateCreateInfo = nullptr;

    // Rasterization state create info.
    VkPipelineRasterizationStateCreateInfo*    m_pRasterizationStateCreateInfo = nullptr;

    // Multisample state create info.
    VkPipelineMultisampleStateCreateInfo*      m_pMultisampleStateCreateInfo = nullptr;

    // Depth stencil state create info.
    VkPipelineDepthStencilStateCreateInfo*     m_pDepthStencilStateCreateInfo = nullptr;

    // Color blend state create info.
    VkPipelineColorBlendStateCreateInfo*       m_pColorBlendStateCreateInfo = nullptr;

    // Render pass create info.
    VkRenderPassCreateInfo* m_pRenderPassCreateInfo = nullptr;
};

// A structure containing all create info necessary to instantiate a new Vulkan compute pipeline.
class RgPsoComputeVulkan : public RgPsoCreateInfoVulkan
{
public:
    RgPsoComputeVulkan() = default;
    virtual ~RgPsoComputeVulkan() = default;

    // Initialize the compute pipeline create info using suitable defaults.
    void Initialize();

    // Retrieve the compute pipeline create info.
    VkComputePipelineCreateInfo* GetComputePipelineCreateInfo();

private:
    // Initialize default compute pipeline state create info.
    void InitializeComputePipelineStateCreateInfo();

    // The compute pipeline create info.
    VkComputePipelineCreateInfo m_pipelineCreateInfo = {};
};

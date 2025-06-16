//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for the RGA gui vulkan data types.
//=============================================================================
#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_DATA_TYPES_VULKAN_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_DATA_TYPES_VULKAN_H_

// Local.
#include "radeon_gpu_analyzer_gui/rg_data_types.h"

// *** VULKAN STRING CONSTANTS - START ***

// Shader source file extensions.
static const char* kStrSourceFileExtensionVulkanGlsl = ".glsl";
static const char* kStrSourceFileExtensionSpirv = ".spv";

// Vulkan API Name.
static const char* kStrApiNameVulkan = "Vulkan";
static const char* kStrApiAbbreviationVulkan = "VK";

// Create New Vulkan Graphics Pipeline menu item.
static const char* kStrMenuBarCreateNewGraphicsPsoVulkan = "Create new Vulkan graphics pipeline";
static const char* kStrMenuBarCreateNewGraphicsPsoTooltipVulkan = "Create a new Vulkan graphics pipeline, which is a container for Vulkan graphics shaders and state, that can be compiled and analyzed through RGA (Ctrl+Alt+G).";

// Create New Vulkan Compute Pipeline menu item.
static const char* kStrMenuBarCreateNewComputePsoVulkan = "Create new Vulkan compute pipeline";
static const char* kStrMenuBarCreateNewComputePsoTooltipVulkan = "Create a new Vulkan compute pipeline, which is a container for a Vulkan compute shader and state, that can be compiled and analyzed through RGA (Ctrl+Alt+C).";

// Vulkan file menu shortcuts.
static const char* gs_ACTION_HOTKEY_NEW_VULKAN_GRAPHICS_PROJECT = "Ctrl+Alt+G";
static const char* gs_ACTION_HOTKEY_NEW_VULKAN_COMPUTE_PROJECT = "Ctrl+Alt+C";

// Rename project dialog title string.
static const char* kStrRenameProjectDialogBoxTitleVulkan = "New Vulkan project";

// Vulkan ICD extension.
#ifdef _WIN32
static const char* kStrDialogFilterIcd = "Vulkan ICD files (*.dll)";
#else
static const char* kStrDialogFilterIcd = "Vulkan ICD files (*.so)";
#endif // _WIN32

// *** VULKAN  CONSTANTS - END ***

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// *** VULKAN PIPELINE STATE EDITOR  - START ***

// Vulkan Pipeline Layout Create Info structure name.
static const char* kStrVulkanPipelineLayoutCreateInfo = "VkPipelineLayoutCreateInfo";

// Vulkan Descriptor Set Layout Create Info structure name.
static const char* kStrVulkanDescriptorSetLayoutCreateInfo = "VkDescriptorSetLayoutCreateInfo";

// Vulkan Graphics Pipeline Create Info structure name.
static const char* kStrVulkanGraphicsPipelineCreateInfo = "VkGraphicsPipelineCreateInfo";

// Vulkan Render Pass Create Info structure name.
static const char* kStrVulkanRenderPassCreateInfo = "VkRenderPassCreateInfo";

// The root graphics pipeline state node.
static const char* kStrVulkanGraphicsPipelineState = "Graphics pipeline state";

// The root compute pipeline state node.
static const char* kStrVulkanComputePipelineState = "Compute pipeline state";

// Vulkan flags member string.
static const char* kStrVulkanPipelineMemberFlags = "flags";

// Vulkan Pipeline subpass member string.
static const char* kStrVulkanPipelineMemberSubpass = "subpass";

// Vulkan Pipeline basePipelineIndex member string.
static const char* kStrVulkanPipelineMemberBaseIndex = "basePipelineIndex";

// Vulkan Pipeline pVertexInputState member string.
static const char* kStrVulkanPipelineMemberPvertexInputState = "pVertexInputState";

// Vulkan Pipeline vertexBindingDescriptionCount member string.
static const char* kStrVulkanPipelineMemberVertexBindingDescriptionCount = "vertexBindingDescriptionCount";

// Vulkan Pipeline pVertexBindingDescriptions member string.
static const char* kStrVulkanPipelineMemberPvertexBindingDescriptions = "pVertexBindingDescriptions";

// Vulkan Vertex Input Binding Description struct type string.
static const char* kStrVulkanVertexInputBindingDescription = "VkVertexInputBindingDescription";

// Vulkan Vertex Input Attribute Description struct type string.
static const char* kStrVulkanVertexInputAttributeDescription = "VkVertexInputAttributeDescription";

// Vulkan Pipeline vertexAttributeDescriptionCount member string.
static const char* kStrVulkanPipelineMemberVertexAttributeDescriptionCount = "vertexAttributeDescriptionCount";

// Vulkan Pipeline pVertexAttributeDescriptions member string.
static const char* kStrVulkanPipelineMemberPvertexAttributeDescriptions = "pVertexAttributeDescriptions";

// Vulkan Pipeline binding member string.
static const char* kStrVulkanPipelineMemberVertexBinding = "binding";

// Vulkan Pipeline stride member string.
static const char* kStrVulkanPipelineMemberVertexStride = "stride";

// Vulkan Pipeline inputRate member string.
static const char* kStrVulkanPipelineMemberVertexInputRate = "inputRate";

// Vulkan Pipeline location member string.
static const char* kStrVulkanPipelineMemberVertexLocation = "location";

// Vulkan Pipeline format member string.
static const char* kStrVulkanPipelineMemberVertexFormat = "format";

// Vulkan Pipeline offset member string.
static const char* kStrVulkanPipelineMemberOffset = "offset";

// Vulkan Pipeline extent member string.
static const char* kStrVulkanPipelineMemberExtent = "extent";

// Vulkan Pipeline pInputAssemblyState member string.
static const char* kStrVulkanPipelineMemberPinputAssemblyState = "pInputAssemblyState";

// Vulkan Pipeline topology member string.
static const char* kStrVulkanPipelineMemberTopology = "topology";

// Vulkan Pipeline primitiveRestartEnable member string.
static const char* kStrVulkanPipelineMemberPrimitiveRestartEnable = "primitiveRestartEnable";

// Vulkan Pipeline pTessellationState member string.
static const char* kStrVulkanPipelineMemberPtessellationState = "pTessellationState";

// Vulkan Pipeline patchControlPoints member string.
static const char* kStrVulkanPipelineMemberPatchControlPoints = "patchControlPoints";

// Vulkan Pipeline pViewportState member string.
static const char* kStrVulkanPipelineMemberPviewportState = "pViewportState";

// Vulkan Pipeline viewportCount member string.
static const char* kStrVulkanPipelineMemberViewportCount = "viewportCount";

// Vulkan Pipeline pViewports member string.
static const char* kStrVulkanPipelineMemberPviewports = "pViewports";

// Vulkan Pipeline scissorCount member string.
static const char* kStrVulkanPipelineMemberScissorCount = "scissorCount";

// Vulkan Pipeline pScissors member string.
static const char* kStrVulkanPipelineMemberPscissors = "pScissors";

// Vulkan Pipeline VkViewport member string.
static const char* kStrVulkanPipelineMemberVkViewport = "VkViewport";

// Vulkan x member string.
static const char* kStrVulkanMemberX = "x";

// Vulkan y member string.
static const char* kStrVulkanMemberY = "y";

// Vulkan width member string.
static const char* kStrVulkanMemberWidth = "width";

// Vulkan height member string.
static const char* kStrVulkanMemberHeight = "height";

// Vulkan viewport minDepth member string.
static const char* kStrVulkanPipelineMemberViewportMinDepth = "minDepth";

// Vulkan viewport maxDepth member string.
static const char* kStrVulkanPipelineMemberViewportMaxDepth = "maxDepth";

// Vulkan Pipeline VkRect2d member string.
static const char* kStrVulkanPipelineMemberScissorRect = "VkRect2D";

// Vulkan Pipeline pRasterizationState member string.
static const char* kStrVulkanPipelineMemberPrasterizationState = "pRasterizationState";

// Vulkan Pipeline depthClampEnable member string.
static const char* kStrVulkanPipelineMemberDepthClampEnable = "depthClampEnable";

// Vulkan Pipeline rasterizerDiscardEnable member string.
static const char* kStrVulkanPipelineMemberRasterizerDiscardEnable = "rasterizerDiscardEnable";

// Vulkan Pipeline polygonMode member string.
static const char* kStrVulkanPipelineMemberRasterizerPolygonMode = "polygonMode";

// Vulkan Pipeline cullMode member string.
static const char* kStrVulkanPipelineMemberRasterizerCullMode = "cullMode";

// Vulkan Pipeline frontFace member string.
static const char* kStrVulkanPipelineMemberRasterizerFrontFace = "frontFace";

// Vulkan Pipeline depthBiasEnable member string.
static const char* kStrVulkanPipelineMemberRasterizerDepthBiasEnable = "depthBiasEnable";

// Vulkan Pipeline depthBiasConstantFactor member string.
static const char* kStrVulkanPipelineMemberRasterizerDepthBiasConstantFactor = "depthBiasConstantFactor";

// Vulkan Pipeline depthBiasClamp member string.
static const char* kStrVulkanPipelineMemberRasterizerDepthBiasClamp = "depthBiasClamp";

// Vulkan Pipeline depthBiasSlopeFactor member string.
static const char* kStrVulkanPipelineMemberRasterizerDepthBiasSlopeFactor = "depthBiasSlopeFactor";

// Vulkan Pipeline lineWidth member string.
static const char* kStrVulkanPipelineMemberRasterizerLineWidth = "lineWidth";

// Vulkan Pipeline pMultisampleState member string.
static const char* kStrVulkanPipelineMemberPmultisampleState = "pMultisampleState";

// Vulkan Pipeline rasterizationSamples member string.
static const char* kStrVulkanMultisampleRasterizationSamples = "rasterizationSamples";

// Vulkan Pipeline multisample state pSampleMask type string.
static const char* kStrVulkanMultisampleRasterizationSampleFlagsType = "VkSampleMask";

// Vulkan Pipeline multisample state pSampleMask element type string.
static const char* kStrVulkanMultisampleRasterizationSampleFlagsElementType = "uint32_t";

// Vulkan Pipeline sampleShadingEnable member string.
static const char* kStrVulkanMultisampleSampleShadingEnable = "sampleShadingEnable";

// Vulkan Pipeline minSampleShading member string.
static const char* kStrVulkanMultisampleMinSampleShading = "minSampleShading";

// Vulkan Pipeline pSampleMask member string.
static const char* kStrVulkanMultisampleSampleMask = "pSampleMask";

// Vulkan Pipeline alphaToCoverageEnable member string.
static const char* kStrVulkanMultisampleAlphaToCoverageEnable = "alphaToCoverageEnable";

// Vulkan Pipeline alphaToOneEnable member string.
static const char* kStrVulkanMultisampleAlphaToOneEnable = "alphaToOneEnable";

// Vulkan Pipeline pDepthStencilState member string.
static const char* kStrVulkanPipelineMemberPdepthStencilState = "pDepthStencilState";

// Vulkan depth stencil depthTestEnable member string.
static const char* kStrVulkanDepthStencilDepthTestEnable = "depthTestEnable";

// Vulkan depth stencil depthWriteEnable member string.
static const char* kStrVulkanDepthStencilDepthWriteEnable = "depthWriteEnable";

// Vulkan depth stencil depthCompareOp member string.
static const char* kStrVulkanDepthStencilDepthCompareOp = "depthCompareOp";

// Vulkan depth stencil depthBoundsTestEnable member string.
static const char* kStrVulkanDepthStencilDepthBoundsTestEnable = "depthBoundsTestEnable";

// Vulkan depth stencil stencilTestEnable member string.
static const char* kStrVulkanDepthStencilStencilTestEnable = "stencilTestEnable";

// Vulkan depth stencil front member string.
static const char* kStrVulkanDepthStencilFront = "front";

// Vulkan depth stencil back member string.
static const char* kStrVulkanDepthStencilBack = "back";

// Vulkan depth stencil minDepthBounds member string.
static const char* kStrVulkanDepthStencilMinDepthBounds = "minDepthBounds";

// Vulkan depth stencil maxDepthBounds member string.
static const char* kStrVulkanDepthStencilMaxDepthBounds = "maxDepthBounds";

// Vulkan depth stencil op state failOp member string.
static const char* kStrVulkanDepthStencilStateFailOp = "failOp";

// Vulkan depth stencil op state passOp member string.
static const char* kStrVulkanDepthStencilStatePassOp = "passOp";

// Vulkan depth stencil op state depthFailOp member string.
static const char* kStrVulkanDepthStencilStateDepthFailOp = "depthFailOp";

// Vulkan depth stencil op state compareOp member string.
static const char* kStrVulkanDepthStencilStateCompareOp = "compareOp";

// Vulkan depth stencil op state compareMask member string.
static const char* kStrVulkanDepthStencilStateCompareMask = "compareMask";

// Vulkan depth stencil op state writeMask member string.
static const char* kStrVulkanDepthStencilStateWriteMask = "writeMask";

// Vulkan depth stencil op state reference member string.
static const char* kStrVulkanDepthStencilStateReference = "reference";

// Vulkan color blend state logicOpEnable member string.
static const char* kStrVulkanColorBlendStateLogicOpEnable = "logicOpEnable";

// Vulkan color blend state logicOp member string.
static const char* kStrVulkanColorBlendStateLogicOp = "logicOp";

// Vulkan color blend state attachmentCount member string.
static const char* kStrVulkanColorBlendStateAttachmentCount = "attachmentCount";

// Vulkan color blend state pAttachments member string.
static const char* kStrVulkanColorBlendStateAttachments = "pAttachments";

// Vulkan color blend state blendConstants member string.
static const char* kStrVulkanColorBlendStateBlendConstants = "blendConstants";

// Vulkan color blend attachment state type string.
static const char* kStrVulkanColorBlendAttachmentState = "VkPipelineColorBlendAttachmentState";

// Vulkan color blend attachment state blendEnable member string.
static const char* kStrVulkanColorBlendAttachmentStateBlendEnable = "blendEnable";

// Vulkan color blend attachment state srcColorBlendFactor member string.
static const char* kStrVulkanColorBlendAttachmentStateSrcColorBlendFactor = "srcColorBlendFactor";

// Vulkan color blend attachment state dstColorBlendFactor member string.
static const char* kStrVulkanColorBlendAttachmentStateDstColorBlendFactor = "dstColorBlendFactor";

// Vulkan color blend attachment state colorBlendOp member string.
static const char* kStrVulkanColorBlendAttachmentStateColorBlendOp = "colorBlendOp";

// Vulkan color blend attachment state srcAlphaBlendFactor member string.
static const char* kStrVulkanColorBlendAttachmentStateSrcAlphaBlendFactor = "srcAlphaBlendFactor";

// Vulkan color blend attachment state dstAlphaBlendFactor member string.
static const char* kStrVulkanColorBlendAttachmentStateDstAlphaBlendFactor = "dstAlphaBlendFactor";

// Vulkan color blend attachment state alphaBlendOp member string.
static const char* kStrVulkanColorBlendAttachmentStateAlphaBlendOp = "alphaBlendOp";

// Vulkan color blend attachment state colorWriteMask member string.
static const char* kStrVulkanColorBlendAttachmentStateColorWriteMask = "colorWriteMask";

// Vulkan Pipeline pColorBlendState member string.
static const char* kStrVulkanPipelineMemberPcolorBlendState = "pColorBlendState";

// Vulkan Pipeline pDynamicState member string.
static const char* kStrVulkanPipelineMemberPdynamicState = "pDynamicState";

// Vulkan Graphics Pipeline Create Info structure name.
static const char* kStrVulkanComputePipelineCreateInfo = "VkComputePipelineCreateInfo";

// Vulkan Render Pass attachmentCount member string.
static const char* kStrVulkanRenderPassAttachmentCount = "attachmentCount";

// Vulkan Render Pass pAttachments array member string.
static const char* kStrVulkanRenderPassAttachments = "pAttachments";

// Vulkan Render Pass subpassCount member string.
static const char* kStrVulkanRenderPassSubpassCount = "subpassCount";

// Vulkan Render Pass pSubpasses array member string.
static const char* kStrVulkanRenderPassSubpasses = "pSubpasses";

// Vulkan Render Pass pipelineBindPoint array member string.
static const char* kStrVulkanRenderPassDependencyPipelineBindPoint = "pipelineBindPoint";

// Vulkan Render Pass inputAttachmentCount array member string.
static const char* kStrVulkanRenderPassDependencyInputAttachmentCount = "inputAttachmentCount";

// Vulkan Render Pass pInputAttachments array member string.
static const char* kStrVulkanRenderPassDependencyInputAttachments = "pInputAttachments";

// Vulkan Render Pass colorAttachmentCount array member string.
static const char* kStrVulkanRenderPassDependencyColorAttachmentCount = "colorAttachmentCount";

// Vulkan Render Pass pColorAttachments array member string.
static const char* kStrVulkanRenderPassDependencyColorAttachments = "pColorAttachments";

// Vulkan Render Pass pResolveAttachments array member string.
static const char* kStrVulkanRenderPassDependencyResolveAttachments = "pResolveAttachments";

// Vulkan Render Pass pDepthStencilAttachment array member string.
static const char* kStrVulkanRenderPassDependencyDepthStencilAttachment = "pDepthStencilAttachment";

// Vulkan Render Pass preserveAttachmentCount array member string.
static const char* kStrVulkanRenderPassDependencyPreserveAttachmentCount = "preserveAttachmentCount";

// Vulkan Render Pass pPreserveAttachments array member string.
static const char* kStrVulkanRenderPassPreserveAttachments = "pPreserveAttachments";

// Vulkan Render Pass preserveAttachment element member string.
static const char* kStrVulkanRenderPassPreserveAttachment = "preserveAttachment";

// Vulkan Render Pass dependencyCount member string.
static const char* kStrVulkanRenderPassDependencyCount = "dependencyCount";

// Vulkan Render Pass pDependencies array member string.
static const char* kStrVulkanRenderPassDependencies = "pDependencies";

// Vulkan Render Pass dependency srcSubpass member string.
static const char* kStrVulkanRenderPassDependencySrcSubpass = "srcSubpass";

// Vulkan Render Pass dependency dstSubpass member string.
static const char* kStrVulkanRenderPassDependencyDstSubpass = "dstSubpass";

// Vulkan Render Pass dependency srcStageMask member string.
static const char* kStrVulkanRenderPassDependencySrcStageMask= "srcStageMask";

// Vulkan Render Pass dependency dstStageMask member string.
static const char* kStrVulkanRenderPassDependencyDstStageMask = "dstStageMask";

// Vulkan Render Pass dependency srcAccessMask member string.
static const char* kStrVulkanRenderPassDependencySrcAccessMask = "srcAccessMask";

// Vulkan Render Pass dependency dstAccessMask member string.
static const char* kStrVulkanRenderPassDependencyDstAccessMask = "dstAccessMask";

// Vulkan Render Pass dependency dependencyFlags member string.
static const char* kStrVulkanRenderPassDependencyDependencyFlags = "dependencyFlags";

// Vulkan Render Pass attachment samples member string.
static const char* kStrVulkanRenderPassAttachmentSamples = "samples";

// Vulkan Render Pass loadOp member string.
static const char* kStrVulkanRenderPassLoadOp = "loadOp";

// Vulkan Render Pass storeOp member string.
static const char* kStrVulkanRenderPassStoreOp = "storeOp";

// Vulkan Render Pass stencilLoadOp member string.
static const char* kStrVulkanRenderPassStencilLoadOp = "stencilLoadOp";

// Vulkan Render Pass stencilStoreOp member string.
static const char* kStrVulkanRenderPassStencilStoreOp = "stencilStoreOp";

// Vulkan Render Pass initialLayout member string.
static const char* kStrVulkanRenderPassInitialLayout = "initialLayout";

// Vulkan Render Pass finalLayout member string.
static const char* kStrVulkanRenderPassFinalLayout = "finalLayout";

// Vulkan Render Pass attachment description type member string.
static const char* kStrVulkanRenderPassAttachmentDescription = "VkAttachmentDescription";

// Vulkan Render Pass Subpass description type member string.
static const char* kStrVulkanRenderPassSubpassDescription = "VkSubpassDescription";

// Vulkan Render Pass dependency description type member string.
static const char* kStrVulkanRenderPassDependencyDescription = "VkSubpassDependency";

// Vulkan Render Pass Subpass pipelineBindPoint member string.
static const char* kStrVulkanRenderPassSubpassPipelineBindPoint = "pipelineBindPoint";

// Vulkan Render Pass Subpass inputAttachmentCount member string.
static const char* kStrVulkanRenderPassSubpassInputAttachmentCount = "inputAttachmentCount";

// Vulkan Render Pass Subpass pInputAttachments member string.
static const char* kStrVulkanRenderPassSubpassInputAttachments = "pInputAttachments";

// Vulkan Render Pass Subpass attachment reference type member string.
static const char* kStrVulkanRenderSubpassAttachmentReference = "VkAttachmentReference";

// Vulkan Render Pass Subpass preserve attachment type member string.
static const char* kStrVulkanRenderSubpassPreserveAttachmentElementType = "uint32_t";

// Vulkan Render Pass Subpass colorAttachmentCount member string.
static const char* kStrVulkanRenderPassSubpassColorAttachmentCount = "colorAttachmentCount";

// Vulkan Render Pass Subpass pColorAttachments member string.
static const char* kStrVulkanRenderPassSubpassColorAttachments = "pColorAttachments";

// Vulkan Render Pass Subpass resolveAttachmentCount member string.
static const char* kStrVulkanRenderPassSubpassResolveAttachmentCount = "resolveAttachmentCount";

// Vulkan Render Pass Subpass pResolveAttachments member string.
static const char* kStrVulkanRenderPassSubpassResolveAttachments = "pResolveAttachments";

// Vulkan Render Pass Subpass pDepthStencilAttachment member string.
static const char* kStrVulkanRenderPassSubpassDepthStencilAttachment = "pDepthStencilAttachment";

// Vulkan Render Pass Subpass preserveAttachmentCount member string.
static const char* kStrVulkanRenderPassSubpassPreserveAttachmentCount = "preserveAttachmentCount";

// Vulkan Render Pass Subpass pPreserveAttachments member string.
static const char* kStrVulkanRenderPassSubpassPreserveAttachments = "pPreserveAttachments";

// Vulkan Render Pass Subpass attachment member string.
static const char* kStrVulkanRenderPassSubpassAttachmentIndex = "attachment";

// Vulkan Render Pass Subpass layout member string.
static const char* kStrVulkanRenderPassSubpassAttachmentLayout = "layout";

// Vulkan Pipeline Layout descriptorSetCount member string.
static const char* kStrVulkanPipelineLayoutDescriptorSetLayoutCount = "setLayoutCount";

// Vulkan Pipeline Layout pSetLayouts array member string.
static const char* kStrVulkanPipelineLayoutSetLayouts = "pSetLayouts";

// Vulkan Descriptor Set Layout handle type.
static const char* kStrVulkanDescriptorSetLayoutHandle = "VkDescriptorSetLayout";

// Vulkan Descriptor Set Layouts header item title.
static const char* kStrVulkanDescriptorSetLayoutsHeader = "Descriptor Set Layouts";

// Vulkan Descriptor Set Layout count.
static const char* kStrVulkanDescriptorSetLayoutCount = "Descriptor Set Layout count";

// Vulkan Pipeline Layout pushConstantRangeCount array member string.
static const char* kStrVulkanPipelineLayoutPushConstantRangeCount = "pushConstantRangeCount";

// Vulkan Pipeline Layout pPushConstantRanges array member string.
static const char* kStrVulkanPipelineLayoutPushConstantRanges = "pPushConstantRanges";

// Vulkan Push Constant range type string.
static const char* kStrVulkanPushConstantRangeType = "VkPushConstantRange";

// Vulkan Pipeline Layout Stage Flags member string.
static const char* kStrVulkanPipelineLayoutStageFlags = "stageFlags";

// Vulkan Push Constants offset member string.
static const char* kStrVulkanPipelineLayoutPushConstantOffset = "offset";

// Vulkan Push Constants size member string.
static const char* kStrVulkanPipelineLayoutPushConstantSize = "size";

// Vulkan Descriptor Set Layout bindingCount member string.
static const char* kStrVulkanPipelineLayoutDescriptorSetLayoutBindingCount = "bindingCount";

// Vulkan Descriptor Set Layout pBindings member string.
static const char* kStrVulkanPipelineLayoutDescriptorSetLayoutBindings = "pBindings";

// Vulkan Descriptor Set Layout Binding type string.
static const char* kStrVulkanDescriptorSetLayoutBindingType = "VkDescriptorSetLayoutBinding";

// Vulkan Descriptor Set Layout binding member string.
static const char* kStrVulkanDescriptorSetLayoutBinding = "binding";

// Vulkan Descriptor Set Layout Binding descriptor type member string.
static const char* kStrVulkanDescriptorSetLayoutBindingDescriptorType = "descriptorType";

// Vulkan Descriptor Set Layout Binding descriptor count member string.
static const char* kStrVulkanDescriptorSetLayoutBindingDescriptorCount = "descriptorCount";

// *** VULKAN PIPELINE STATE EDITOR STRINGS - END ***

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// *** VULKAN SHADER ABBREVIATION STAGE NAME STRINGS - START ***

// The Vertex stage abbreviation string.
static const char* kStrVulkanStageNameVertexAbbreviation = "vert";

// The Vulkan Tessellation Control stage abbreviation string.
static const char* kStrVulkanStageNameTessellationControlAbbreviation = "tesc";

// The Vulkan Tessellation Evaluation stage abbreviation string.
static const char* kStrVulkanStageNameTessellationEvaluationAbbreviation = "tese";

// The geometry stage abbreviation string.
static const char* kStrVulkanStageNameGeometryAbbreviation = "geom";

// The Vulkan Fragment stage abbreviation string.
static const char* kStrVulkanStageNameFragmentAbbreviation = "frag";

// The Compute stage abbreviation string.
static const char* kStrVulkanStageNameComputeAbbreviation = "comp";

// *** VULKAN SHADER ABBREVIATION STAGE NAME  - END ***

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// *** VULKAN STAGE NAME  - START ***

// The Vertex stage name string.
static const char* kStrVulkanStageNameVertex = "Vertex";

// The Vulkan Tessellation Control stage name string.
static const char* kStrVulkanStageNameTessellationControl = "Tessellation Control";

// The Vulkan Tessellation Evaluation stage name string.
static const char* kStrVulkanStageNameTessellationEvaluation = "Tessellation Evaluation";

// The geometry stage name string.
static const char* kStrVulkanStageNameGeometry = "Geometry";

// The Vulkan Fragment stage name string.
static const char* kStrVulkanStageNameFragment = "Fragment";

// The Compute stage name string.
static const char* kStrVulkanStageNameCompute = "Compute";

// *** VULKAN STAGE NAME  - END ***

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// *** VULKAN SOURCE CODE  - START ***

// The default source code for a Vulkan glsl vertex shader.
static const char* kStrNewFileTemplateCodeVulkanGlslVertexShader =
    "/* Auto-generated with Radeon GPU Analyzer (RGA).*/\n"
    "#version 450\n"
    "\n"
    "#extension GL_GOOGLE_include_directive : require\n"
    "\n"
    "out gl_PerVertex\n"
    "{\n"
    "    vec4 gl_Position;\n"
    "};\n"
    "\n"
    "layout(location = 0) out vec3 fragColor;\n"
    "\n"
    "void main()\n"
    "{\n"
    "    float x = -1.0 + float((gl_VertexIndex & 1) << 2);\n"
    "    float y = -1.0 + float((gl_VertexIndex & 2) << 1);\n"
    "    gl_Position = vec4(x, y, 0, 1);\n"
    "\n"
    "    fragColor = vec3(1.0f, 0.0f, 0.0f);\n"
    "}";

// The default source code for a Vulkan glsl passthrough geometry shader.
static const char* kStrNewFileTemplateCodeVulkanGlslGeometryShader =
    "/* Auto-generated with Radeon GPU Analyzer (RGA).*/\n"
    "#version 450\n"
    "\n"
    "#extension GL_GOOGLE_include_directive : require\n"
    "\n"
    "layout (triangles) in;\n"
    "layout (triangle_strip, max_vertices=3) out;\n"
    "layout (location = 0) out vec3 outColor;\n"
    "\n"
    "void main()\n"
    "{\n"
    "    for(int vertexIndex = 0; vertexIndex < 3; vertexIndex++)\n"
    "    {\n"
    "        gl_Position = gl_in[vertexIndex].gl_Position;\n"
    "        EmitVertex();\n"
    "    }\n"
    "    EndPrimitive();\n"
    "}";

// The default source code for a Vulkan glsl passthrough tessellation control shader.
static const char* kStrNewFileTemplateCodeVulkanGlslTessellationControlShader =
    "/* Auto-generated with Radeon GPU Analyzer (RGA).*/\n"
    "#version 450\n"
    "\n"
    "#extension GL_GOOGLE_include_directive : require\n"
    "\n"
    "layout (vertices = 3) out;\n"
    "\n"
    "void main(void)\n"
    "{\n"
    "    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;\n"
    "}";

// The default source code for a Vulkan glsl passthrough tessellation evaluation shader.
static const char* kStrNewFileTemplateCodeVulkanGlslTessellationEvaluationShader =
    "/* Auto-generated with Radeon GPU Analyzer (RGA).*/\n"
    "#version 450\n"
    "\n"
    "#extension GL_GOOGLE_include_directive : require\n"
    "\n"
    "layout (triangles) in;\n"
    "\n"
    "void main(void)\n"
    "{\n"
    "    gl_Position = (gl_TessCoord.x * gl_in[0].gl_Position) +\n"
    "                  (gl_TessCoord.y * gl_in[1].gl_Position) +\n"
    "                  (gl_TessCoord.z * gl_in[2].gl_Position);\n"
    "}";

// The default source code for a Vulkan glsl fragment shader.
static const char* kStrNewFileTemplateCodeVulkanGlslFragmentShader =
    "/* Auto-generated with Radeon GPU Analyzer (RGA).*/\n"
    "#version 450\n"
    "\n"
    "#extension GL_GOOGLE_include_directive : require\n"
    "\n"
    "layout(location = 0) in vec3 fragColor;\n"
    "layout(location = 0) out vec4 outColor;\n"
    "\n"
    "void main()\n"
    "{\n"
    "    outColor = vec4(fragColor, 1.0f);\n"
    "}";

// The default source code for a Vulkan glsl compute shader.
static const char* kStrNewFileTemplateCodeVulkanGlslComputeShader =
    "/* Auto-generated with Radeon GPU Analyzer (RGA).*/\n"
    "#version 450\n"
    "\n"
    "#extension GL_GOOGLE_include_directive : require\n"
    "\n"
    "void main()\n"
    "{\n"
    "    uint index = gl_GlobalInvocationID.x;\n"
    "}\n";

// *** VULKAN SOURCE CODE STRINGS - END ***

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// *** VULKAN TYPE DECLARATIONS - START ***

// Vulkan build settings.
struct RgBuildSettingsVulkan : public RgBuildSettings
{
    RgBuildSettingsVulkan() = default;
    virtual ~RgBuildSettingsVulkan() = default;

    // Copy constructor used to initialize using another instance.
    RgBuildSettingsVulkan(const RgBuildSettingsVulkan& other) :
        RgBuildSettings(other),
        is_generate_debug_info_checked(other.is_generate_debug_info_checked),
        is_no_explicit_bindings_checked(other.is_no_explicit_bindings_checked),
        is_use_hlsl_block_offsets_checked(other.is_use_hlsl_block_offsets_checked),
        is_use_hlsl_io_mapping_checked(other.is_use_hlsl_io_mapping_checked),
        is_enable_validation_layers_checked(other.is_enable_validation_layers_checked),
        icd_location(other.icd_location),
        glslang_options(other.glslang_options),
        binary_file_name(other.binary_file_name)
    {}

    virtual bool HasSameSettings(const RgBuildSettingsVulkan& other) const
    {
        bool isSame = RgBuildSettings::HasSameSettings(other) &&
            (is_generate_debug_info_checked == other.is_generate_debug_info_checked) &&
            (is_no_explicit_bindings_checked == other.is_no_explicit_bindings_checked) &&
            (is_use_hlsl_block_offsets_checked == other.is_use_hlsl_block_offsets_checked) &&
            (is_use_hlsl_io_mapping_checked == other.is_use_hlsl_io_mapping_checked) &&
            (is_enable_validation_layers_checked == other.is_enable_validation_layers_checked) &&
            (icd_location == other.icd_location) &&
            (glslang_options == other.glslang_options) &&
            (binary_file_name == other.binary_file_name);

        return isSame;
    }

    bool is_generate_debug_info_checked = false;
    bool is_no_explicit_bindings_checked = false;
    bool is_use_hlsl_block_offsets_checked = false;
    bool is_use_hlsl_io_mapping_checked = false;
    bool is_enable_validation_layers_checked = false;
    std::string icd_location;
    std::string glslang_options;
    std::string binary_file_name;
};

// A clone of an Vulkan project.
struct RgProjectCloneVulkan : public RgGraphicsProjectClone
{
    // CTOR #1.
    RgProjectCloneVulkan()
    {
        // Instantiate a Vulkan build settings instance by default.
        build_settings = std::make_shared<RgBuildSettingsVulkan>();
    }

    // CTOR #2.
    RgProjectCloneVulkan(const std::string& clone_name, std::shared_ptr<RgBuildSettingsVulkan> build_settings) :
        RgGraphicsProjectClone(clone_name, build_settings) {}

    // Paths to original SPIR-V binary files.
    RgPipelineShaders spv_backup_;
};

// An Vulkan project.
struct RgProjectVulkan : public RgProject
{
    // CTOR #1.
    RgProjectVulkan() : RgProject("", "", RgProjectAPI::kVulkan) {}

    // CTOR #2.
    RgProjectVulkan(const std::string& project_name, const std::string& project_file_full_path) : RgProject(project_name,
        project_file_full_path, RgProjectAPI::kVulkan) {}

    // CTOR #3.
    RgProjectVulkan(const std::string& project_name, const std::string& project_file_full_path,
        const std::vector<std::shared_ptr<RgProjectClone>>& clones) :
        RgProject(project_name, project_file_full_path, RgProjectAPI::kVulkan, clones) {}
};

// *** VULKAN TYPE DECLARATIONS - END ***
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_DATA_TYPES_VULKAN_H_

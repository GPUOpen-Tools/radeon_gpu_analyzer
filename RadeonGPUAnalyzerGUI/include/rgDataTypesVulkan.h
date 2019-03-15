#pragma once

// Local.
#include <RadeonGPUAnalyzerGUI/Include/rgDataTypes.h>

// *** VULKAN STRING CONSTANTS - START ***

// Shader source file extensions.
static const char* STR_SOURCE_FILE_EXTENSION_VULKAN_GLSL = ".glsl";
static const char* STR_SOURCE_FILE_EXTENSION_SPIRV = ".spv";

// Vulkan API Name.
static const char* STR_API_NAME_VULKAN = "Vulkan";
static const char* STR_API_ABBREVIATION_VULKAN = "VK";

// Create New Vulkan Graphics Pipeline menu item.
static const char* STR_MENU_BAR_CREATE_NEW_GRAPHICS_PSO_VULKAN = "Create new Vulkan graphics pipeline";
static const char* STR_MENU_BAR_CREATE_NEW_GRAPHICS_PSO_TOOLTIP_VULKAN = "Create a new Vulkan graphics pipeline, which is a container for Vulkan graphics shaders and state, that can be compiled and analyzed through RGA (Ctrl+Shift+G).";

// Create New Vulkan Compute Pipeline menu item.
static const char* STR_MENU_BAR_CREATE_NEW_COMPUTE_PSO_VULKAN = "Create new Vulkan compute pipeline";
static const char* STR_MENU_BAR_CREATE_NEW_COMPUTE_PSO_TOOLTIP_VULKAN = "Create a new Vulkan compute pipeline, which is a container for a Vulkan compute shader and state, that can be compiled and analyzed through RGA (Ctrl+Shift+C).";

// Vulkan file menu shortcuts.
static const char* gs_ACTION_HOTKEY_NEW_VULKAN_GRAPHICS_PROJECT = "Ctrl+Shift+G";
static const char* gs_ACTION_HOTKEY_NEW_VULKAN_COMPUTE_PROJECT = "Ctrl+Shift+C";

// Rename project dialog title string.
static const char* STR_RENAME_PROJECT_DIALOG_BOX_TITLE_VULKAN = "New Vulkan project";

// Vulkan ICD extension.
#ifdef _WIN32
static const char* STR_DIALOG_FILTER_ICD = "Vulkan ICD files (*.dll)";
#else
static const char* STR_DIALOG_FILTER_ICD = "Vulkan ICD files (*.so)";
#endif // _WIN32

// *** VULKAN STRING CONSTANTS - END ***

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// *** VULKAN PIPELINE STATE EDITOR STRINGS - START ***

// Vulkan Pipeline Layout Create Info structure name.
static const char* STR_VULKAN_PIPELINE_LAYOUT_CREATE_INFO = "VkPipelineLayoutCreateInfo";

// Vulkan Descriptor Set Layout Create Info structure name.
static const char* STR_VULKAN_DESCRIPTOR_SET_LAYOUT_CREATE_INFO = "VkDescriptorSetLayoutCreateInfo";

// Vulkan Graphics Pipeline Create Info structure name.
static const char* STR_VULKAN_GRAPHICS_PIPELINE_CREATE_INFO = "VkGraphicsPipelineCreateInfo";

// Vulkan Render Pass Create Info structure name.
static const char* STR_VULKAN_RENDER_PASS_CREATE_INFO = "VkRenderPassCreateInfo";

// The root graphics pipeline state node.
static const char* STR_VULKAN_GRAPHICS_PIPELINE_STATE = "Graphics pipeline state";

// The root compute pipeline state node.
static const char* STR_VULKAN_COMPUTE_PIPELINE_STATE = "Compute pipeline state";

// Vulkan flags member string.
static const char* STR_VULKAN_PIPELINE_MEMBER_FLAGS = "flags";

// Vulkan Pipeline subpass member string.
static const char* STR_VULKAN_PIPELINE_MEMBER_SUBPASS = "subpass";

// Vulkan Pipeline basePipelineIndex member string.
static const char* STR_VULKAN_PIPELINE_MEMBER_BASE_INDEX = "basePipelineIndex";

// Vulkan Pipeline pVertexInputState member string.
static const char* STR_VULKAN_PIPELINE_MEMBER_PVERTEX_INPUT_STATE = "pVertexInputState";

// Vulkan Pipeline vertexBindingDescriptionCount member string.
static const char* STR_VULKAN_PIPELINE_MEMBER_VERTEX_BINDING_DESCRIPTION_COUNT = "vertexBindingDescriptionCount";

// Vulkan Pipeline pVertexBindingDescriptions member string.
static const char* STR_VULKAN_PIPELINE_MEMBER_PVERTEX_BINDING_DESCRIPTIONS = "pVertexBindingDescriptions";

// Vulkan Vertex Input Binding Description struct type string.
static const char* STR_VULKAN_VERTEX_INPUT_BINDING_DESCRIPTION = "VkVertexInputBindingDescription";

// Vulkan Vertex Input Attribute Description struct type string.
static const char* STR_VULKAN_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION = "VkVertexInputAttributeDescription";

// Vulkan Pipeline vertexAttributeDescriptionCount member string.
static const char* STR_VULKAN_PIPELINE_MEMBER_VERTEX_ATTRIBUTE_DESCRIPTION_COUNT = "vertexAttributeDescriptionCount";

// Vulkan Pipeline pVertexAttributeDescriptions member string.
static const char* STR_VULKAN_PIPELINE_MEMBER_PVERTEX_ATTRIBUTE_DESCRIPTIONS = "pVertexAttributeDescriptions";

// Vulkan Pipeline binding member string.
static const char* STR_VULKAN_PIPELINE_MEMBER_VERTEX_BINDING = "binding";

// Vulkan Pipeline stride member string.
static const char* STR_VULKAN_PIPELINE_MEMBER_VERTEX_STRIDE = "stride";

// Vulkan Pipeline inputRate member string.
static const char* STR_VULKAN_PIPELINE_MEMBER_VERTEX_INPUT_RATE = "inputRate";

// Vulkan Pipeline location member string.
static const char* STR_VULKAN_PIPELINE_MEMBER_VERTEX_LOCATION = "location";

// Vulkan Pipeline format member string.
static const char* STR_VULKAN_PIPELINE_MEMBER_VERTEX_FORMAT = "format";

// Vulkan Pipeline offset member string.
static const char* STR_VULKAN_PIPELINE_MEMBER_OFFSET = "offset";

// Vulkan Pipeline extent member string.
static const char* STR_VULKAN_PIPELINE_MEMBER_EXTENT = "extent";

// Vulkan Pipeline pInputAssemblyState member string.
static const char* STR_VULKAN_PIPELINE_MEMBER_PINPUT_ASSEMBLY_STATE = "pInputAssemblyState";

// Vulkan Pipeline topology member string.
static const char* STR_VULKAN_PIPELINE_MEMBER_TOPOLOGY = "topology";

// Vulkan Pipeline primitiveRestartEnable member string.
static const char* STR_VULKAN_PIPELINE_MEMBER_PRIMITIVE_RESTART_ENABLE = "primitiveRestartEnable";

// Vulkan Pipeline pTessellationState member string.
static const char* STR_VULKAN_PIPELINE_MEMBER_PTESSELLATION_STATE = "pTessellationState";

// Vulkan Pipeline patchControlPoints member string.
static const char* STR_VULKAN_PIPELINE_MEMBER_PATCH_CONTROL_POINTS = "patchControlPoints";

// Vulkan Pipeline pViewportState member string.
static const char* STR_VULKAN_PIPELINE_MEMBER_PVIEWPORT_STATE = "pViewportState";

// Vulkan Pipeline viewportCount member string.
static const char* STR_VULKAN_PIPELINE_MEMBER_VIEWPORT_COUNT = "viewportCount";

// Vulkan Pipeline pViewports member string.
static const char* STR_VULKAN_PIPELINE_MEMBER_PVIEWPORTS = "pViewports";

// Vulkan Pipeline scissorCount member string.
static const char* STR_VULKAN_PIPELINE_MEMBER_SCISSOR_COUNT = "scissorCount";

// Vulkan Pipeline pScissors member string.
static const char* STR_VULKAN_PIPELINE_MEMBER_PSCISSORS = "pScissors";

// Vulkan Pipeline VkViewport member string.
static const char* STR_VULKAN_PIPELINE_MEMBER_VK_VIEWPORT = "VkViewport";

// Vulkan x member string.
static const char* STR_VULKAN_MEMBER_X = "x";

// Vulkan y member string.
static const char* STR_VULKAN_MEMBER_Y = "y";

// Vulkan width member string.
static const char* STR_VULKAN_MEMBER_WIDTH = "width";

// Vulkan height member string.
static const char* STR_VULKAN_MEMBER_HEIGHT = "height";

// Vulkan viewport minDepth member string.
static const char* STR_VULKAN_PIPELINE_MEMBER_VIEWPORT_MIN_DEPTH = "minDepth";

// Vulkan viewport maxDepth member string.
static const char* STR_VULKAN_PIPELINE_MEMBER_VIEWPORT_MAX_DEPTH = "maxDepth";

// Vulkan Pipeline VkRect2d member string.
static const char* STR_VULKAN_PIPELINE_MEMBER_SCISSOR_RECT = "VkRect2D";

// Vulkan Pipeline pRasterizationState member string.
static const char* STR_VULKAN_PIPELINE_MEMBER_PRASTERIZATION_STATE = "pRasterizationState";

// Vulkan Pipeline depthClampEnable member string.
static const char* STR_VULKAN_PIPELINE_MEMBER_DEPTH_CLAMP_ENABLE = "depthClampEnable";

// Vulkan Pipeline rasterizerDiscardEnable member string.
static const char* STR_VULKAN_PIPELINE_MEMBER_RASTERIZER_DISCARD_ENABLE = "rasterizerDiscardEnable";

// Vulkan Pipeline polygonMode member string.
static const char* STR_VULKAN_PIPELINE_MEMBER_RASTERIZER_POLYGON_MODE = "polygonMode";

// Vulkan Pipeline cullMode member string.
static const char* STR_VULKAN_PIPELINE_MEMBER_RASTERIZER_CULL_MODE = "cullMode";

// Vulkan Pipeline frontFace member string.
static const char* STR_VULKAN_PIPELINE_MEMBER_RASTERIZER_FRONT_FACE = "frontFace";

// Vulkan Pipeline depthBiasEnable member string.
static const char* STR_VULKAN_PIPELINE_MEMBER_RASTERIZER_DEPTH_BIAS_ENABLE = "depthBiasEnable";

// Vulkan Pipeline depthBiasConstantFactor member string.
static const char* STR_VULKAN_PIPELINE_MEMBER_RASTERIZER_DEPTH_BIAS_CONSTANT_FACTOR = "depthBiasConstantFactor";

// Vulkan Pipeline depthBiasClamp member string.
static const char* STR_VULKAN_PIPELINE_MEMBER_RASTERIZER_DEPTH_BIAS_CLAMP = "depthBiasClamp";

// Vulkan Pipeline depthBiasSlopeFactor member string.
static const char* STR_VULKAN_PIPELINE_MEMBER_RASTERIZER_DEPTH_BIAS_SLOPE_FACTOR = "depthBiasSlopeFactor";

// Vulkan Pipeline lineWidth member string.
static const char* STR_VULKAN_PIPELINE_MEMBER_RASTERIZER_LINE_WIDTH = "lineWidth";

// Vulkan Pipeline pMultisampleState member string.
static const char* STR_VULKAN_PIPELINE_MEMBER_PMULTISAMPLE_STATE = "pMultisampleState";

// Vulkan Pipeline rasterizationSamples member string.
static const char* STR_VULKAN_MULTISAMPLE_RASTERIZATION_SAMPLES = "rasterizationSamples";

// Vulkan Pipeline multisample state pSampleMask type string.
static const char* STR_VULKAN_MULTISAMPLE_RASTERIZATION_SAMPLE_FLAGS_TYPE = "VkSampleMask";

// Vulkan Pipeline multisample state pSampleMask element type string.
static const char* STR_VULKAN_MULTISAMPLE_RASTERIZATION_SAMPLE_FLAGS_ELEMENT_TYPE = "uint32_t";

// Vulkan Pipeline sampleShadingEnable member string.
static const char* STR_VULKAN_MULTISAMPLE_SAMPLE_SHADING_ENABLE = "sampleShadingEnable";

// Vulkan Pipeline minSampleShading member string.
static const char* STR_VULKAN_MULTISAMPLE_MIN_SAMPLE_SHADING = "minSampleShading";

// Vulkan Pipeline pSampleMask member string.
static const char* STR_VULKAN_MULTISAMPLE_P_SAMPLE_MASK = "pSampleMask";

// Vulkan Pipeline alphaToCoverageEnable member string.
static const char* STR_VULKAN_MULTISAMPLE_ALPHA_TO_COVERAGE_ENABLE = "alphaToCoverageEnable";

// Vulkan Pipeline alphaToOneEnable member string.
static const char* STR_VULKAN_MULTISAMPLE_ALPHA_TO_ONE_ENABLE = "alphaToOneEnable";

// Vulkan Pipeline pDepthStencilState member string.
static const char* STR_VULKAN_PIPELINE_MEMBER_PDEPTH_STENCIL_STATE = "pDepthStencilState";

// Vulkan depth stencil depthTestEnable member string.
static const char* STR_VULKAN_DEPTH_STENCIL_DEPTH_TEST_ENABLE = "depthTestEnable";

// Vulkan depth stencil depthWriteEnable member string.
static const char* STR_VULKAN_DEPTH_STENCIL_DEPTH_WRITE_ENABLE = "depthWriteEnable";

// Vulkan depth stencil depthCompareOp member string.
static const char* STR_VULKAN_DEPTH_STENCIL_DEPTH_COMPARE_OP = "depthCompareOp";

// Vulkan depth stencil depthBoundsTestEnable member string.
static const char* STR_VULKAN_DEPTH_STENCIL_DEPTH_BOUNDS_TEST_ENABLE = "depthBoundsTestEnable";

// Vulkan depth stencil stencilTestEnable member string.
static const char* STR_VULKAN_DEPTH_STENCIL_STENCIL_TEST_ENABLE = "stencilTestEnable";

// Vulkan depth stencil front member string.
static const char* STR_VULKAN_DEPTH_STENCIL_FRONT = "front";

// Vulkan depth stencil back member string.
static const char* STR_VULKAN_DEPTH_STENCIL_BACK = "back";

// Vulkan depth stencil minDepthBounds member string.
static const char* STR_VULKAN_DEPTH_STENCIL_MIN_DEPTH_BOUNDS = "minDepthBounds";

// Vulkan depth stencil maxDepthBounds member string.
static const char* STR_VULKAN_DEPTH_STENCIL_MAX_DEPTH_BOUNDS = "maxDepthBounds";

// Vulkan depth stencil op state failOp member string.
static const char* STR_VULKAN_DEPTH_STENCIL_STATE_FAIL_OP = "failOp";

// Vulkan depth stencil op state passOp member string.
static const char* STR_VULKAN_DEPTH_STENCIL_STATE_PASS_OP = "passOp";

// Vulkan depth stencil op state depthFailOp member string.
static const char* STR_VULKAN_DEPTH_STENCIL_STATE_DEPTH_FAIL_OP = "depthFailOp";

// Vulkan depth stencil op state compareOp member string.
static const char* STR_VULKAN_DEPTH_STENCIL_STATE_COMPARE_OP = "compareOp";

// Vulkan depth stencil op state compareMask member string.
static const char* STR_VULKAN_DEPTH_STENCIL_STATE_COMPARE_MASK = "compareMask";

// Vulkan depth stencil op state writeMask member string.
static const char* STR_VULKAN_DEPTH_STENCIL_STATE_WRITE_MASK = "writeMask";

// Vulkan depth stencil op state reference member string.
static const char* STR_VULKAN_DEPTH_STENCIL_STATE_REFERENCE = "reference";

// Vulkan color blend state logicOpEnable member string.
static const char* STR_VULKAN_COLOR_BLEND_STATE_LOGIC_OP_ENABLE = "logicOpEnable";

// Vulkan color blend state logicOp member string.
static const char* STR_VULKAN_COLOR_BLEND_STATE_LOGIC_OP = "logicOp";

// Vulkan color blend state attachmentCount member string.
static const char* STR_VULKAN_COLOR_BLEND_STATE_ATTACHMENT_COUNT = "attachmentCount";

// Vulkan color blend state pAttachments member string.
static const char* STR_VULKAN_COLOR_BLEND_STATE_P_ATTACHMENTS = "pAttachments";

// Vulkan color blend state blendConstants member string.
static const char* STR_VULKAN_COLOR_BLEND_STATE_BLEND_CONSTANTS = "blendConstants";

// Vulkan color blend attachment state type string.
static const char* STR_VULKAN_COLOR_BLEND_ATTACHMENT_STATE = "VkPipelineColorBlendAttachmentState";

// Vulkan color blend attachment state blendEnable member string.
static const char* STR_VULKAN_COLOR_BLEND_ATTACHMENT_STATE_BLEND_ENABLE = "blendEnable";

// Vulkan color blend attachment state srcColorBlendFactor member string.
static const char* STR_VULKAN_COLOR_BLEND_ATTACHMENT_STATE_SRC_COLOR_BLEND_FACTOR = "srcColorBlendFactor";

// Vulkan color blend attachment state dstColorBlendFactor member string.
static const char* STR_VULKAN_COLOR_BLEND_ATTACHMENT_STATE_DST_COLOR_BLEND_FACTOR = "dstColorBlendFactor";

// Vulkan color blend attachment state colorBlendOp member string.
static const char* STR_VULKAN_COLOR_BLEND_ATTACHMENT_STATE_COLOR_BLEND_OP = "colorBlendOp";

// Vulkan color blend attachment state srcAlphaBlendFactor member string.
static const char* STR_VULKAN_COLOR_BLEND_ATTACHMENT_STATE_SRC_ALPHA_BLEND_FACTOR = "srcAlphaBlendFactor";

// Vulkan color blend attachment state dstAlphaBlendFactor member string.
static const char* STR_VULKAN_COLOR_BLEND_ATTACHMENT_STATE_DST_ALPHA_BLEND_FACTOR = "dstAlphaBlendFactor";

// Vulkan color blend attachment state alphaBlendOp member string.
static const char* STR_VULKAN_COLOR_BLEND_ATTACHMENT_STATE_ALPHA_BLEND_OP = "alphaBlendOp";

// Vulkan color blend attachment state colorWriteMask member string.
static const char* STR_VULKAN_COLOR_BLEND_ATTACHMENT_STATE_COLOR_WRITE_MASK = "colorWriteMask";

// Vulkan Pipeline pColorBlendState member string.
static const char* STR_VULKAN_PIPELINE_MEMBER_PCOLOR_BLEND_STATE = "pColorBlendState";

// Vulkan Pipeline pDynamicState member string.
static const char* STR_VULKAN_PIPELINE_MEMBER_PDYNAMIC_STATE = "pDynamicState";

// Vulkan Graphics Pipeline Create Info structure name.
static const char* STR_VULKAN_COMPUTE_PIPELINE_CREATE_INFO = "VkComputePipelineCreateInfo";

// Vulkan Render Pass attachmentCount member string.
static const char* STR_VULKAN_RENDER_PASS_ATTACHMENT_COUNT = "attachmentCount";

// Vulkan Render Pass pAttachments array member string.
static const char* STR_VULKAN_RENDER_PASS_P_ATTACHMENTS = "pAttachments";

// Vulkan Render Pass subpassCount member string.
static const char* STR_VULKAN_RENDER_PASS_SUBPASS_COUNT = "subpassCount";

// Vulkan Render Pass pSubpasses array member string.
static const char* STR_VULKAN_RENDER_PASS_P_SUBPASSES = "pSubpasses";

// Vulkan Render Pass pipelineBindPoint array member string.
static const char* STR_VULKAN_RENDER_PASS_DEPENDENCY_PIPELINE_BIND_POINT = "pipelineBindPoint";

// Vulkan Render Pass inputAttachmentCount array member string.
static const char* STR_VULKAN_RENDER_PASS_DEPENDENCY_INPUT_ATTACHMENT_COUNT = "inputAttachmentCount";

// Vulkan Render Pass pInputAttachments array member string.
static const char* STR_VULKAN_RENDER_PASS_DEPENDENCY_P_INPUT_ATTACHMENTS = "pInputAttachments";

// Vulkan Render Pass colorAttachmentCount array member string.
static const char* STR_VULKAN_RENDER_PASS_DEPENDENCY_COLOR_ATTACHMENT_COUNT = "colorAttachmentCount";

// Vulkan Render Pass pColorAttachments array member string.
static const char* STR_VULKAN_RENDER_PASS_DEPENDENCY_COLOR_ATTACHMENTS = "pColorAttachments";

// Vulkan Render Pass pResolveAttachments array member string.
static const char* STR_VULKAN_RENDER_PASS_DEPENDENCY_P_RESOLVE_ATTACHMENTS = "pResolveAttachments";

// Vulkan Render Pass pDepthStencilAttachment array member string.
static const char* STR_VULKAN_RENDER_PASS_DEPENDENCY_P_DEPTH_STENCIL_ATTACHMENT = "pDepthStencilAttachment";

// Vulkan Render Pass preserveAttachmentCount array member string.
static const char* STR_VULKAN_RENDER_PASS_DEPENDENCY_PRESERVE_ATTACHMENT_COUNT = "preserveAttachmentCount";

// Vulkan Render Pass pPreserveAttachments array member string.
static const char* STR_VULKAN_RENDER_PASS_P_PRESERVE_ATTACHMENTS = "pPreserveAttachments";

// Vulkan Render Pass preserveAttachment element member string.
static const char* STR_VULKAN_RENDER_PASS_PRESERVE_ATTACHMENT = "preserveAttachment";

// Vulkan Render Pass dependencyCount member string.
static const char* STR_VULKAN_RENDER_PASS_DEPENDENCY_COUNT = "dependencyCount";

// Vulkan Render Pass pDependencies array member string.
static const char* STR_VULKAN_RENDER_PASS_P_DEPENDENCIES = "pDependencies";

// Vulkan Render Pass dependency srcSubpass member string.
static const char* STR_VULKAN_RENDER_PASS_DEPENDENCY_SRC_SUBPASS = "srcSubpass";

// Vulkan Render Pass dependency dstSubpass member string.
static const char* STR_VULKAN_RENDER_PASS_DEPENDENCY_DST_SUBPASS = "dstSubpass";

// Vulkan Render Pass dependency srcStageMask member string.
static const char* STR_VULKAN_RENDER_PASS_DEPENDENCY_SRC_STAGE_MASK= "srcStageMask";

// Vulkan Render Pass dependency dstStageMask member string.
static const char* STR_VULKAN_RENDER_PASS_DEPENDENCY_DST_STAGE_MASK = "dstStageMask";

// Vulkan Render Pass dependency srcAccessMask member string.
static const char* STR_VULKAN_RENDER_PASS_DEPENDENCY_SRC_ACCESS_MASK = "srcAccessMask";

// Vulkan Render Pass dependency dstAccessMask member string.
static const char* STR_VULKAN_RENDER_PASS_DEPENDENCY_DST_ACCESS_MASK = "dstAccessMask";

// Vulkan Render Pass dependency dependencyFlags member string.
static const char* STR_VULKAN_RENDER_PASS_DEPENDENCY_DEPENDENCY_FLAGS = "dependencyFlags";

// Vulkan Render Pass attachment samples member string.
static const char* STR_VULKAN_RENDER_PASS_ATTACHMENT_SAMPLES = "samples";

// Vulkan Render Pass loadOp member string.
static const char* STR_VULKAN_RENDER_PASS_LOAD_OP = "loadOp";

// Vulkan Render Pass storeOp member string.
static const char* STR_VULKAN_RENDER_PASS_STORE_OP = "storeOp";

// Vulkan Render Pass stencilLoadOp member string.
static const char* STR_VULKAN_RENDER_PASS_STENCIL_LOAD_OP = "stencilLoadOp";

// Vulkan Render Pass stencilStoreOp member string.
static const char* STR_VULKAN_RENDER_PASS_STENCIL_STORE_OP = "stencilStoreOp";

// Vulkan Render Pass initialLayout member string.
static const char* STR_VULKAN_RENDER_PASS_INITIAL_LAYOUT = "initialLayout";

// Vulkan Render Pass finalLayout member string.
static const char* STR_VULKAN_RENDER_PASS_FINAL_LAYOUT = "finalLayout";

// Vulkan Render Pass attachment description type member string.
static const char* STR_VULKAN_RENDER_PASS_ATTACHMENT_DESCRIPTION = "VkAttachmentDescription";

// Vulkan Render Pass Subpass description type member string.
static const char* STR_VULKAN_RENDER_PASS_SUBPASS_DESCRIPTION = "VkSubpassDescription";

// Vulkan Render Pass dependency description type member string.
static const char* STR_VULKAN_RENDER_PASS_DEPENDENCY_DESCRIPTION = "VkSubpassDependency";

// Vulkan Render Pass Subpass pipelineBindPoint member string.
static const char* STR_VULKAN_RENDER_PASS_SUBPASS_PIPELINE_BIND_POINT = "pipelineBindPoint";

// Vulkan Render Pass Subpass inputAttachmentCount member string.
static const char* STR_VULKAN_RENDER_PASS_SUBPASS_INPUT_ATTACHMENT_COUNT = "inputAttachmentCount";

// Vulkan Render Pass Subpass pInputAttachments member string.
static const char* STR_VULKAN_RENDER_PASS_SUBPASS_P_INPUT_ATTACHMENTS = "pInputAttachments";

// Vulkan Render Pass Subpass attachment reference type member string.
static const char* STR_VULKAN_RENDER_SUBPASS_ATTACHMENT_REFERENCE = "VkAttachmentReference";

// Vulkan Render Pass Subpass preserve attachment type member string.
static const char* STR_VULKAN_RENDER_SUBPASS_PRESERVE_ATTACHMENT_ELEMENT_TYPE = "uint32_t";

// Vulkan Render Pass Subpass colorAttachmentCount member string.
static const char* STR_VULKAN_RENDER_PASS_SUBPASS_COLOR_ATTACHMENT_COUNT = "colorAttachmentCount";

// Vulkan Render Pass Subpass pColorAttachments member string.
static const char* STR_VULKAN_RENDER_PASS_SUBPASS_P_COLOR_ATTACHMENTS = "pColorAttachments";

// Vulkan Render Pass Subpass resolveAttachmentCount member string.
static const char* STR_VULKAN_RENDER_PASS_SUBPASS_RESOLVE_ATTACHMENT_COUNT = "resolveAttachmentCount";

// Vulkan Render Pass Subpass pResolveAttachments member string.
static const char* STR_VULKAN_RENDER_PASS_SUBPASS_P_RESOLVE_ATTACHMENTS = "pResolveAttachments";

// Vulkan Render Pass Subpass pDepthStencilAttachment member string.
static const char* STR_VULKAN_RENDER_PASS_SUBPASS_P_DEPTH_STENCIL_ATTACHMENT = "pDepthStencilAttachment";

// Vulkan Render Pass Subpass preserveAttachmentCount member string.
static const char* STR_VULKAN_RENDER_PASS_SUBPASS_PRESERVE_ATTACHMENT_COUNT = "preserveAttachmentCount";

// Vulkan Render Pass Subpass pPreserveAttachments member string.
static const char* STR_VULKAN_RENDER_PASS_SUBPASS_P_PRESERVE_ATTACHMENTS = "pPreserveAttachments";

// Vulkan Render Pass Subpass attachment member string.
static const char* STR_VULKAN_RENDER_PASS_SUBPASS_ATTACHMENT_INDEX = "attachment";

// Vulkan Render Pass Subpass layout member string.
static const char* STR_VULKAN_RENDER_PASS_SUBPASS_ATTACHMENT_LAYOUT = "layout";

// Vulkan Pipeline Layout descriptorSetCount member string.
static const char* STR_VULKAN_PIPELINE_LAYOUT_DESCRIPTOR_SET_LAYOUT_COUNT = "setLayoutCount";

// Vulkan Pipeline Layout pSetLayouts array member string.
static const char* STR_VULKAN_PIPELINE_LAYOUT_P_SET_LAYOUTS = "pSetLayouts";

// Vulkan Descriptor Set Layout handle type.
static const char* STR_VULKAN_DESCRIPTOR_SET_LAYOUT_HANDLE = "VkDescriptorSetLayout";

// Vulkan Descriptor Set Layouts header item title.
static const char* STR_VULKAN_DESCRIPTOR_SET_LAYOUTS_HEADER = "Descriptor Set Layouts";

// Vulkan Descriptor Set Layout count.
static const char* STR_VULKAN_DESCRIPTOR_SET_LAYOUT_COUNT = "Descriptor Set Layout count";

// Vulkan Pipeline Layout pushConstantRangeCount array member string.
static const char* STR_VULKAN_PIPELINE_LAYOUT_PUSH_CONSTANT_RANGE_COUNT = "pushConstantRangeCount";

// Vulkan Pipeline Layout pPushConstantRanges array member string.
static const char* STR_VULKAN_PIPELINE_LAYOUT_P_PUSH_CONSTANT_RANGES = "pPushConstantRanges";

// Vulkan Push Constant range type string.
static const char* STR_VULKAN_PUSH_CONSTANT_RANGE_TYPE = "VkPushConstantRange";

// Vulkan Pipeline Layout Stage Flags member string.
static const char* STR_VULKAN_PIPELINE_LAYOUT_STAGE_FLAGS = "stageFlags";

// Vulkan Push Constants offset member string.
static const char* STR_VULKAN_PIPELINE_LAYOUT_PUSH_CONSTANT_OFFSET = "offset";

// Vulkan Push Constants size member string.
static const char* STR_VULKAN_PIPELINE_LAYOUT_PUSH_CONSTANT_SIZE = "size";

// Vulkan Descriptor Set Layout bindingCount member string.
static const char* STR_VULKAN_PIPELINE_LAYOUT_DESCRIPTOR_SET_LAYOUT_BINDING_COUNT = "bindingCount";

// Vulkan Descriptor Set Layout pBindings member string.
static const char* STR_VULKAN_PIPELINE_LAYOUT_DESCRIPTOR_SET_LAYOUT_P_BINDINGS = "pBindings";

// Vulkan Descriptor Set Layout Binding type string.
static const char* STR_VULKAN_DESCRIPTOR_SET_LAYOUT_BINDING_TYPE = "VkDescriptorSetLayoutBinding";

// Vulkan Descriptor Set Layout binding member string.
static const char* STR_VULKAN_DESCRIPTOR_SET_LAYOUT_BINDING = "binding";

// Vulkan Descriptor Set Layout Binding descriptor type member string.
static const char* STR_VULKAN_DESCRIPTOR_SET_LAYOUT_BINDING_DESCRIPTOR_TYPE = "descriptorType";

// Vulkan Descriptor Set Layout Binding descriptor count member string.
static const char* STR_VULKAN_DESCRIPTOR_SET_LAYOUT_BINDING_DESCRIPTOR_COUNT = "descriptorCount";


// *** VULKAN PIPELINE STATE EDITOR STRINGS - END ***

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// *** VULKAN SHADER ABBREVIATION STAGE NAME STRINGS - START ***

// The Vertex stage abbreviation string.
static const char* STR_VULKAN_STAGE_NAME_VERTEX_ABBREVIATION = "vert";

// The Vulkan Tessellation Control stage abbreviation string.
static const char* STR_VULKAN_STAGE_NAME_TESSELLATION_CONTROL_ABBREVIATION = "tesc";

// The Vulkan Tessellation Evaluation stage abbreviation string.
static const char* STR_VULKAN_STAGE_NAME_TESSELLATION_EVALUATION_ABBREVIATION = "tese";

// The geometry stage abbreviation string.
static const char* STR_VULKAN_STAGE_NAME_GEOMETRY_ABBREVIATION = "geom";

// The Vulkan Fragment stage abbreviation string.
static const char* STR_VULKAN_STAGE_NAME_FRAGMENT_ABBREVIATION = "frag";

// The Compute stage abbreviation string.
static const char* STR_VULKAN_STAGE_NAME_COMPUTE_ABBREVIATION = "comp";

// *** VULKAN SHADER ABBREVIATION STAGE NAME STRINGS - END ***

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// *** VULKAN STAGE NAME STRINGS - START ***

// The Vertex stage name string.
static const char* STR_VULKAN_STAGE_NAME_VERTEX = "Vertex";

// The Vulkan Tessellation Control stage name string.
static const char* STR_VULKAN_STAGE_NAME_TESSELLATION_CONTROL = "Tessellation Control";

// The Vulkan Tessellation Evaluation stage name string.
static const char* STR_VULKAN_STAGE_NAME_TESSELLATION_EVALUATION = "Tessellation Evaluation";

// The geometry stage name string.
static const char* STR_VULKAN_STAGE_NAME_GEOMETRY = "Geometry";

// The Vulkan Fragment stage name string.
static const char* STR_VULKAN_STAGE_NAME_FRAGMENT = "Fragment";

// The Compute stage name string.
static const char* STR_VULKAN_STAGE_NAME_COMPUTE = "Compute";

// *** VULKAN STAGE NAME STRINGS - END ***

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// *** VULKAN SOURCE CODE STRINGS - START ***

// The default source code for a Vulkan glsl vertex shader.
static const char* STR_NEW_FILE_TEMPLATE_CODE_VULKAN_GLSL_VERTEX_SHADER =
    "/* Auto-generated with Radeon GPU Analyzer (RGA).*/\n"
    "#version 450\n"
    "\n"
    "#extension GL_GOOGLE_include_directive : require\n"
    "#extension GL_ARB_separate_shader_objects : enable\n"
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
static const char* STR_NEW_FILE_TEMPLATE_CODE_VULKAN_GLSL_GEOMETRY_SHADER =
    "/* Auto-generated with Radeon GPU Analyzer (RGA).*/\n"
    "#version 450\n"
    "\n"
    "#extension GL_GOOGLE_include_directive : require\n"
    "#extension GL_ARB_separate_shader_objects : enable\n"
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
static const char* STR_NEW_FILE_TEMPLATE_CODE_VULKAN_GLSL_TESSELLATION_CONTROL_SHADER =
    "/* Auto-generated with Radeon GPU Analyzer (RGA).*/\n"
    "#version 450\n"
    "\n"
    "#extension GL_GOOGLE_include_directive : require\n"
    "#extension GL_ARB_separate_shader_objects : enable\n"
    "\n"
    "layout (vertices = 3) out;\n"
    "\n"
    "void main(void)\n"
    "{\n"
    "    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;\n"
    "}";

// The default source code for a Vulkan glsl passthrough tessellation evaluation shader.
static const char* STR_NEW_FILE_TEMPLATE_CODE_VULKAN_GLSL_TESSELLATION_EVALUATION_SHADER =
    "/* Auto-generated with Radeon GPU Analyzer (RGA).*/\n"
    "#version 450\n"
    "\n"
    "#extension GL_GOOGLE_include_directive : require\n"
    "#extension GL_ARB_separate_shader_objects : enable\n"
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
static const char* STR_NEW_FILE_TEMPLATE_CODE_VULKAN_GLSL_FRAGMENT_SHADER =
    "/* Auto-generated with Radeon GPU Analyzer (RGA).*/\n"
    "#version 450\n"
    "\n"
    "#extension GL_GOOGLE_include_directive : require\n"
    "#extension GL_ARB_separate_shader_objects : enable\n"
    "\n"
    "layout(location = 0) in vec3 fragColor;\n"
    "layout(location = 0) out vec4 outColor;\n"
    "\n"
    "void main()\n"
    "{\n"
    "    outColor = vec4(fragColor, 1.0f);\n"
    "}";

// The default source code for a Vulkan glsl compute shader.
static const char* STR_NEW_FILE_TEMPLATE_CODE_VULKAN_GLSL_COMPUTE_SHADER =
    "/* Auto-generated with Radeon GPU Analyzer (RGA).*/\n"
    "#version 450\n"
    "\n"
    "#extension GL_GOOGLE_include_directive : require\n"
    "#extension GL_ARB_separate_shader_objects : enable\n"
    "\n"
    "void main()\n"
    "{\n"
    "    uint index = gl_GlobalInvocationID.x;\n"
    "}\n";

// *** VULKAN SOURCE CODE STRINGS - END ***

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// *** VULKAN TYPE DECLARATIONS - START ***

// Vulkan build settings.
struct rgBuildSettingsVulkan : public rgBuildSettings
{
    rgBuildSettingsVulkan() = default;
    virtual ~rgBuildSettingsVulkan() = default;

    // Copy constructor used to initialize using another instance.
    rgBuildSettingsVulkan(const rgBuildSettingsVulkan& other) :
        rgBuildSettings(other),
        m_isGenerateDebugInfoChecked(other.m_isGenerateDebugInfoChecked),
        m_isNoExplicitBindingsChecked(other.m_isNoExplicitBindingsChecked),
        m_isUseHlslBlockOffsetsChecked(other.m_isUseHlslBlockOffsetsChecked),
        m_isUseHlslIoMappingChecked(other.m_isUseHlslIoMappingChecked),
        m_isEnableValidationLayersChecked(other.m_isEnableValidationLayersChecked),
        m_ICDLocation(other.m_ICDLocation),
        m_glslangOptions(other.m_glslangOptions)
    {}

    virtual bool HasSameSettings(const rgBuildSettingsVulkan& other) const
    {
        bool isSame = rgBuildSettings::HasSameSettings(other) &&
            (m_isGenerateDebugInfoChecked == other.m_isGenerateDebugInfoChecked) &&
            (m_isNoExplicitBindingsChecked == other.m_isNoExplicitBindingsChecked) &&
            (m_isUseHlslBlockOffsetsChecked == other.m_isUseHlslBlockOffsetsChecked) &&
            (m_isUseHlslIoMappingChecked == other.m_isUseHlslIoMappingChecked) &&
            (m_isEnableValidationLayersChecked == other.m_isEnableValidationLayersChecked) &&
            (m_ICDLocation == other.m_ICDLocation) &&
            (m_glslangOptions == other.m_glslangOptions);

        return isSame;
    }

    bool m_isGenerateDebugInfoChecked = false;
    bool m_isNoExplicitBindingsChecked = false;
    bool m_isUseHlslBlockOffsetsChecked = false;
    bool m_isUseHlslIoMappingChecked = false;
    bool m_isEnableValidationLayersChecked = false;
    std::string m_ICDLocation;
    std::string m_glslangOptions;
};

// A clone of an Vulkan project.
struct rgProjectCloneVulkan : public rgGraphicsProjectClone
{
    // CTOR #1.
    rgProjectCloneVulkan()
    {
        // Instantiate a Vulkan build settings instance by default.
        m_pBuildSettings = std::make_shared<rgBuildSettingsVulkan>();
    }

    // CTOR #2.
    rgProjectCloneVulkan(const std::string& cloneName, std::shared_ptr<rgBuildSettingsVulkan> pBuildSettings) :
        rgGraphicsProjectClone(cloneName, pBuildSettings) {}

    // Paths to original SPIR-V binary files.
    rgPipelineShaders m_spvBackup;
};

// An Vulkan project.
struct rgProjectVulkan : public rgProject
{
    // CTOR #1.
    rgProjectVulkan() : rgProject("", "", rgProjectAPI::Vulkan) {}

    // CTOR #2.
    rgProjectVulkan(const std::string& projectName, const std::string& projectFileFullPath) : rgProject(projectName,
        projectFileFullPath, rgProjectAPI::Vulkan) {}

    // CTOR #3.
    rgProjectVulkan(const std::string& projectName, const std::string& projectFileFullPath,
        const std::vector<std::shared_ptr<rgProjectClone>>& clones) :
        rgProject(projectName, projectFileFullPath, rgProjectAPI::Vulkan, clones) {}
};

// *** VULKAN TYPE DECLARATIONS - END ***

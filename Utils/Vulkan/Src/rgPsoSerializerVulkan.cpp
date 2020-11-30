// C++
#include <fstream>
#include <sstream>

// Infra.
#include <json/json-3.2.0/single_include/nlohmann/json.hpp>
#include <Utils/Vulkan/Include/rgPsoSerializerVulkan.h>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/rgDefinitions.h>

// Error string definitions.
static const char* STR_ERR_FAILED_TO_WRITE_FILE = "Error: failed to open file for writing: ";
static const char* STR_ERR_FAILED_TO_READ_FILE = "Error: failed to open file for reading: ";
static const char* STR_ERR_FAILED_TO_LOAD_PIPELINE_TYPE_MISMATCH = "The file's pipeline type does not match the project's pipeline type.";
static const char* STR_ERR_FAILED_TO_READ_PIPELINE_VERSION = "The pipeline file version is unknown and cannot be loaded.";
static const char* STR_ERR_FAILED_UNSUPPORTED_VERSION = "The pipeline file version is unrecognized.";

// A version enumeration used to specify the revision of the pipeline file schema.
enum class rgPipelineModelVersion
{
    // An unknown/unrecognized file version.
    Unknown,

    // The initial version of the pipeline model. Majority of pipeline structures are serialized.
    VERSION_1_0,

    // This version resolves a failure in reading & writing VkStencilOpState's 'failOp' member.
    VERSION_1_1,

    // This version resolves a failure in reading & writing the multisampling state's VkSampleMask.
    VERSION_1_2,
};

// This declaration is used to specify the current schema revision for the pipeline state file.
static const rgPipelineModelVersion s_CURRENT_PIPELINE_VERSION = rgPipelineModelVersion::VERSION_1_2;

// Pipeline CreateInfo member name string constants.
static const char* STR_PIPELINE_MODEL_VERSION                           = "version";
static const char* STR_MEMBER_VALUE_TRUE                                = "true";
static const char* STR_MEMBER_VALUE_FALSE                               = "false";
static const char* STR_MEMBER_NAME_TYPE                                 = "sType";
static const char* STR_MEMBER_NAME_PNEXT                                = "pNext";
static const char* STR_MEMBER_NAME_FLAGS                                = "flags";
static const char* STR_MEMBER_NAME_PSTAGES                              = "pStages";
static const char* STR_MEMBER_NAME_STAGE_COUNT                          = "stageCount";
static const char* STR_MEMBER_NAME_P_VERTEX_INPUT_STATE                 = "pVertexInputState";
static const char* STR_MEMBER_NAME_P_INPUT_ASSEMBLY_STATE               = "pInputAssemblyState";
static const char* STR_MEMBER_NAME_P_TESSELLATION_STATE                 = "pTessellationState";
static const char* STR_MEMBER_NAME_P_VIEWPORT_STATE                     = "pViewportState";
static const char* STR_MEMBER_NAME_P_RASTERIZATION_STATE                = "pRasterizationState";
static const char* STR_MEMBER_NAME_P_MULTISAMPLE_STATE                  = "pMultisampleState";
static const char* STR_MEMBER_NAME_P_DEPTH_STENCIL_STATE                = "pDepthStencilState";
static const char* STR_MEMBER_NAME_P_COLOR_BLEND_STATE                  = "pColorBlendState";
static const char* STR_MEMBER_NAME_P_DYNAMIC_STATE                      = "pDynamicState";
static const char* STR_MEMBER_NAME_LAYOUT                               = "layout";
static const char* STR_MEMBER_NAME_RENDER_PASS                          = "renderPass";
static const char* STR_MEMBER_NAME_SUBPASS                              = "subpass";
static const char* STR_MEMBER_NAME_BASE_PIPELINE_HANDLE                 = "basePipelineHandle";
static const char* STR_MEMBER_NAME_BASE_PIPELINE_INDEX                  = "basePipelineIndex";
static const char* STR_MEMBER_NAME_MODULE                               = "module";
static const char* STR_MEMBER_NAME_NAME                                 = "name";
static const char* STR_MEMBER_NAME_STAGE                                = "stage";
static const char* STR_MEMBER_NAME_P_SPECIALIZATION_INFO                = "pSpecializationInfo";
static const char* STR_MEMBER_NAME_VERTEX_BINDING_DESCRIPTION_COUNT     = "vertexBindingDescriptionCount";
static const char* STR_MEMBER_NAME_VERTEX_ATTRIBUTE_DESCRIPTION_COUNT   = "vertexAttributeDescriptionCount";
static const char* STR_MEMBER_NAME_P_VERTEX_BINDING_DESCRIPTIONS        = "pVertexBindingDescriptions";
static const char* STR_MEMBER_NAME_P_VERTEX_ATTRIBUTE_DESCRIPTIONS      = "pVertexAttributeDescriptions";
static const char* STR_MEMBER_NAME_TOPOLOGY                             = "topology";
static const char* STR_MEMBER_NAME_PRIMITIVE_RESTART_ENABLED            = "primitiveRestartEnable";
static const char* STR_MEMBER_NAME_PATCH_CONTROL_POINTS                 = "patchControlPoints";
static const char* STR_MEMBER_NAME_VIEWPORT_COUNT                       = "viewportCount";
static const char* STR_MEMBER_NAME_SCISSOR_COUNT                        = "scissorCount";
static const char* STR_MEMBER_NAME_P_VIEWPORTS                          = "pViewports";
static const char* STR_MEMBER_NAME_P_SCISSORS                           = "pScissors";
static const char* STR_MEMBER_NAME_DEPTH_CLAMP_ENABLE                   = "depthClampEnable";
static const char* STR_MEMBER_NAME_RASTERIZER_DISCARD_ENABLE            = "rasterizerDiscardEnable";
static const char* STR_MEMBER_NAME_POLYGON_MODE                         = "polygonMode";
static const char* STR_MEMBER_NAME_CULL_MODE                            = "cullMode";
static const char* STR_MEMBER_NAME_FRONT_FACE                           = "frontFace";
static const char* STR_MEMBER_NAME_DEPTH_BIAS_ENABLE                    = "depthBiasEnable";
static const char* STR_MEMBER_NAME_DEPTH_BIAS_CONSTANT_FACTOR           = "depthBiasConstantFactor";
static const char* STR_MEMBER_NAME_DEPTH_BIAS_CLAMP                     = "depthBiasClamp";
static const char* STR_MEMBER_NAME_DEPTH_BIAS_SLOPE_FACTOR              = "depthBiasSlopeFactor";
static const char* STR_MEMBER_NAME_LINE_WIDTH                           = "lineWidth";
static const char* STR_MEMBER_NAME_RASTERIZATION_SAMPLE                 = "rasterizationSamples";
static const char* STR_MEMBER_NAME_SAMPLE_SHADING_ENABLE                = "sampleShadingEnable";
static const char* STR_MEMBER_NAME_MIN_SAMPLE_SHADING                   = "minSampleShading";
static const char* STR_MEMBER_NAME_P_SAMPLE_MASK                        = "pSampleMask";
static const char* STR_MEMBER_NAME_ALPHA_TO_COVERAGE_ENABLE             = "alphaToCoverageEnable";
static const char* STR_MEMBER_NAME_ALPHA_TO_ONE_ENABLE                  = "alphaToOneEnable";
static const char* STR_MEMBER_NAME_FAIL_OP                              = "failOp";
static const char* STR_MEMBER_NAME_PASS_OP                              = "passOp";
static const char* STR_MEMBER_NAME_DEPTH_FAIL_OP                        = "depthFailOp";
static const char* STR_MEMBER_NAME_COMPARE_OP                           = "compareOp";
static const char* STR_MEMBER_NAME_COMPARE_MASK                         = "compareMask";
static const char* STR_MEMBER_NAME_WRITE_MASK                           = "writeMask";
static const char* STR_MEMBER_NAME_REFERENCE                            = "reference";
static const char* STR_MEMBER_NAME_DEPTH_TEST_ENABLE                    = "depthTestEnable";
static const char* STR_MEMBER_NAME_DEPTH_WRITE_ENABLE                   = "depthWriteEnable";
static const char* STR_MEMBER_NAME_DEPTH_COMPARE_OP                     = "depthCompareOp";
static const char* STR_MEMBER_NAME_DEPTH_BOUNDS_TEST_ENABLE             = "depthBoundsTestEnable";
static const char* STR_MEMBER_NAME_STENCIL_TEST_ENABLE                  = "stencilTestEnable";
static const char* STR_MEMBER_NAME_FRONT                                = "front";
static const char* STR_MEMBER_NAME_BACK                                 = "back";
static const char* STR_MEMBER_NAME_MIN_DEPTH_BOUNDS                     = "minDepthBounds";
static const char* STR_MEMBER_NAME_MAX_DEPTH_BOUNDS                     = "maxDepthBounds";
static const char* STR_MEMBER_NAME_BLEND_ENABLE                         = "blendEnable";
static const char* STR_MEMBER_NAME_SRC_COLOR_BLEND_FACTOR               = "srcColorBlendFactor";
static const char* STR_MEMBER_NAME_DST_COLOR_BLEND_FACTOR               = "dstColorBlendFactor";
static const char* STR_MEMBER_NAME_COLOR_BLEND_OP                       = "colorBlendOp";
static const char* STR_MEMBER_NAME_SRC_ALPHA_BLEND_FACTOR               = "srcAlphaBlendFactor";
static const char* STR_MEMBER_NAME_DST_ALPHA_BLEND_FACTOR               = "dstAlphaBlendFactor";
static const char* STR_MEMBER_NAME_ALPHA_BLEND_OP                       = "alphaBlendOp";
static const char* STR_MEMBER_NAME_COLOR_WRITE_MASK                     = "colorWriteMask";
static const char* STR_MEMBER_NAME_LOGIC_OP_ENABLE                      = "logicOpEnable";
static const char* STR_MEMBER_NAME_LOGIC_OP                             = "logicOp";
static const char* STR_MEMBER_NAME_ATTACHMENT_COUNT                     = "attachmentCount";
static const char* STR_MEMBER_NAME_P_ATTACHMENTS                        = "pAttachments";
static const char* STR_MEMBER_NAME_BLEND_CONSTANTS                      = "blendConstants";
static const char* STR_MEMBER_NAME_DYNAMIC_STATE_COUNT                  = "dynamicStateCount";
static const char* STR_MEMBER_NAME_P_DYNAMIC_STATES                     = "pDynamicStates";
static const char* STR_MEMBER_NAME_MAP_ENTRY_COUNT                      = "mapEntryCount";
static const char* STR_MEMBER_NAME_P_MAP_ENTRIES                        = "pMapEntries";
static const char* STR_MEMBER_NAME_DATA_SIZE                            = "dataSize";
static const char* STR_MEMBER_NAME_P_DATA                               = "pData";
static const char* STR_MEMBER_NAME_BINDING                              = "binding";
static const char* STR_MEMBER_NAME_STRIDE                               = "stride";
static const char* STR_MEMBER_NAME_INPUT_RATE                           = "inputRate";
static const char* STR_MEMBER_NAME_CONSTANT_ID                          = "constantID";
static const char* STR_MEMBER_NAME_OFFSET                               = "offset";
static const char* STR_MEMBER_NAME_SIZE                                 = "size";
static const char* STR_MEMBER_NAME_LOCATION                             = "location";
static const char* STR_MEMBER_NAME_FORMAT                               = "format";
static const char* STR_MEMBER_NAME_X                                    = "x";
static const char* STR_MEMBER_NAME_Y                                    = "y";
static const char* STR_MEMBER_NAME_WIDTH                                = "width";
static const char* STR_MEMBER_NAME_HEIGHT                               = "height";
static const char* STR_MEMBER_NAME_MIN_DEPTH                            = "minDepth";
static const char* STR_MEMBER_NAME_MAX_DEPTH                            = "maxDepth";
static const char* STR_MEMBER_NAME_EXTENT                               = "extent";
static const char* STR_MEMBER_NAME_SET_LAYOUT_COUNT                     = "setLayoutCount";
static const char* STR_MEMBER_NAME_P_SET_LAYOUTS                        = "pSetLayouts";
static const char* STR_MEMBER_NAME_PUSH_CONSTANT_RANGE_COUNT            = "pushConstantRangeCount";
static const char* STR_MEMBER_NAME_P_PUSH_CONSTANT_RANGES               = "pPushConstantRanges";
static const char* STR_MEMBER_NAME_CODE_SIZE                            = "codeSize";
static const char* STR_MEMBER_NAME_BINDING_COUNT                        = "bindingCount";
static const char* STR_MEMBER_NAME_P_BINDINGS                           = "pBindings";
static const char* STR_MEMBER_NAME_DESCRIPTOR_TYPE                      = "descriptorType";
static const char* STR_MEMBER_NAME_DESCRIPTOR_COUNT                     = "descriptorCount";
static const char* STR_MEMBER_NAME_STAGE_FLAGS                          = "stageFlags";
static const char* STR_MEMBER_NAME_SUBPASS_COUNT                        = "subpassCount";
static const char* STR_MEMBER_NAME_P_SUBPASSES                          = "pSubpasses";
static const char* STR_MEMBER_NAME_DEPENDENCY_COUNT                     = "dependencyCount";
static const char* STR_MEMBER_NAME_P_DEPENDENCIES                       = "pDependencies";
static const char* STR_MEMBER_NAME_SAMPLES                              = "samples";
static const char* STR_MEMBER_NAME_LOAD_OP                              = "loadOp";
static const char* STR_MEMBER_NAME_STORE_OP                             = "storeOp";
static const char* STR_MEMBER_NAME_STENCIL_LOAD_OP                      = "stencilLoadOp";
static const char* STR_MEMBER_NAME_STENCIL_STORE_OP                     = "stencilStoreOp";
static const char* STR_MEMBER_NAME_INITIAL_LAYOUT                       = "initialLayout";
static const char* STR_MEMBER_NAME_FINAL_LAYOUT                         = "finalLayout";
static const char* STR_MEMBER_NAME_PIPELINE_BIND_POINT                  = "pipelineBindPoint";
static const char* STR_MEMBER_NAME_INPUT_ATTACHMENT_COUNT               = "inputAttachmentCount";
static const char* STR_MEMBER_NAME_P_INPUT_ATTACHMENTS                  = "pInputAttachments";
static const char* STR_MEMBER_NAME_COLOR_ATTACHMENT_COUNT               = "colorAttachmentCount";
static const char* STR_MEMBER_NAME_P_COLOR_ATTACHMENTS                  = "pColorAttachments";
static const char* STR_MEMBER_NAME_P_RESOLVE_ATTACHMENTS                = "pResolveAttachments";
static const char* STR_MEMBER_NAME_P_DEPTH_STENCIL_ATTACHMENT           = "pDepthStencilAttachment";
static const char* STR_MEMBER_NAME_PRESERVE_ATTACHMENT_COUNT            = "preserveAttachmentCount";
static const char* STR_MEMBER_NAME_P_PRESERVE_ATTACHMENTS               = "pPreserveAttachments";
static const char* STR_MEMBER_NAME_SRC_SUBPASS                          = "srcSubpass";
static const char* STR_MEMBER_NAME_DST_SUBPASS                          = "dstSubpass";
static const char* STR_MEMBER_NAME_SRC_STAGE_MASK                       = "srcStageMask";
static const char* STR_MEMBER_NAME_DST_STAGE_MASK                       = "dstStageMask";
static const char* STR_MEMBER_NAME_SRC_ACCESS_MASK                      = "srcAccessMask";
static const char* STR_MEMBER_NAME_DST_ACCESS_MASK                      = "dstAccessMask";
static const char* STR_MEMBER_NAME_DEPENDENCY_FLAGS                     = "dependencyFlags";
static const char* STR_MEMBER_NAME_ATTACHMENT                           = "attachment";
static const char* STR_MEMBER_NAME_VK_GRAPHICS_PIPELINE_CREATE_INFO     = "VkGraphicsPipelineCreateInfo";
static const char* STR_MEMBER_NAME_VK_RENDER_PASS_CREATE_INFO           = "VkRenderPassCreateInfo";
static const char* STR_MEMBER_NAME_VK_PIPELINE_LAYOUT_CREATE_INFO       = "VkPipelineLayoutCreateInfo";
static const char* STR_MEMBER_NAME_VK_DESCRIPTOR_SET_LAYOUT_CREATE_INFO = "VkDescriptorSetLayoutCreateInfo";
static const char* STR_MEMBER_NAME_VK_COMPUTE_PIPELINE_CREATE_INFO      = "VkComputePipelineCreateInfo";

// Helper function that reads the JSON file from file_stream into structure.
// The function sets returns false if any failure happened (otherwise true is returned).
// The function stores the relevant error message in error_string.
static bool ReadJsonFile(std::ifstream& file_stream, const std::string& file_path, nlohmann::json& structure, std::string &error_string)
{
    bool ret = true;

    // Read the file into the JSON structure.
    try
    {
        // Try to read the file.
        file_stream >> structure;
    }
    catch (...)
    {
        // Failed reading.
        ret = false;

        // Inform the caller.
        std::stringstream error_stream;
        error_stream << STR_ERR_FAILED_TO_READ_FILE;
        error_stream << file_path;
        error_string = error_stream.str();
    }

    return ret;
}

static VkBool32 ReadBool(const nlohmann::json& file)
{
    bool isTrue = (file == STR_MEMBER_VALUE_TRUE);
    return isTrue ? VK_TRUE : VK_FALSE;
}

static const char* WriteBool(VkBool32 val)
{
    return (val == VK_TRUE) ? STR_MEMBER_VALUE_TRUE : STR_MEMBER_VALUE_FALSE;
}

static void* ReadHandle(const nlohmann::json& file)
{
    std::string handleHexString = file;
    handleHexString = handleHexString.substr(2);

    size_t addr = std::strtoull(handleHexString.c_str(), nullptr, 16);
    return (void*)addr;
}

static bool IsCreateInfoExists(const nlohmann::json& file, const std::string& name)
{
    bool res = false;

    auto result = file.find(name);
    if (result != file.end())
    {
        if (file[name] != nullptr)
        {
            res = true;
        }
    }

    return res;
}

// The RGA Vulkan pipeline state serializer base class. This class handles serializing full create
// info for a graphics/compute pipeline, descriptor sets, pipeline layout, and render pass.
class rgPsoSerializerVulkanImpl_Version_1_0
{
public:
    rgPsoSerializerVulkanImpl_Version_1_0() = default;
    virtual ~rgPsoSerializerVulkanImpl_Version_1_0() = default;

    void ReadGraphicsPipelineCreateInfoStructure(rgPsoGraphicsVulkan* create_info, const nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            VkGraphicsPipelineCreateInfo* pGraphicsPipelineCreateInfo = create_info->GetGraphicsPipelineCreateInfo();
            assert(pGraphicsPipelineCreateInfo != nullptr);
            if (pGraphicsPipelineCreateInfo != nullptr)
            {
                pGraphicsPipelineCreateInfo->sType = file[STR_MEMBER_NAME_TYPE];
                pGraphicsPipelineCreateInfo->pNext = ReadHandle(file[STR_MEMBER_NAME_PNEXT]);
                pGraphicsPipelineCreateInfo->flags = file[STR_MEMBER_NAME_FLAGS];
                pGraphicsPipelineCreateInfo->stageCount = file[STR_MEMBER_NAME_STAGE_COUNT];

                VkPipelineShaderStageCreateInfo* pShaderStages = nullptr;
                if (pGraphicsPipelineCreateInfo->stageCount > 0)
                {
                    pShaderStages = new VkPipelineShaderStageCreateInfo[pGraphicsPipelineCreateInfo->stageCount]{};
                    for (uint32_t stage_index = 0; stage_index < pGraphicsPipelineCreateInfo->stageCount; ++stage_index)
                    {
                        ReadStructure(pShaderStages + stage_index, file[STR_MEMBER_NAME_PSTAGES][stage_index]);
                    }
                }
                pGraphicsPipelineCreateInfo->pStages = pShaderStages;

                VkPipelineVertexInputStateCreateInfo* pVertexInputState = nullptr;
                if (IsCreateInfoExists(file, STR_MEMBER_NAME_P_VERTEX_INPUT_STATE))
                {
                    pVertexInputState = create_info->GetPipelineVertexInputStateCreateInfo();
                    ReadStructure(pVertexInputState, file[STR_MEMBER_NAME_P_VERTEX_INPUT_STATE]);
                }

                VkPipelineInputAssemblyStateCreateInfo* pVertexInputAssemblyState = nullptr;
                if (IsCreateInfoExists(file, STR_MEMBER_NAME_P_INPUT_ASSEMBLY_STATE))
                {
                    pVertexInputAssemblyState = create_info->GetPipelineInputAssemblyStateCreateInfo();
                    ReadStructure(pVertexInputAssemblyState, file[STR_MEMBER_NAME_P_INPUT_ASSEMBLY_STATE]);
                }

                VkPipelineTessellationStateCreateInfo* pTessellationState = nullptr;
                if (IsCreateInfoExists(file, STR_MEMBER_NAME_P_TESSELLATION_STATE))
                {
                    pTessellationState = create_info->GetPipelineTessellationStateCreateInfo();
                    ReadStructure(pTessellationState, file[STR_MEMBER_NAME_P_TESSELLATION_STATE]);
                }

                VkPipelineViewportStateCreateInfo* pViewportState = nullptr;
                if (IsCreateInfoExists(file, STR_MEMBER_NAME_P_VIEWPORT_STATE))
                {
                    pViewportState = create_info->GetPipelineViewportStateCreateInfo();
                    ReadStructure(pViewportState, file[STR_MEMBER_NAME_P_VIEWPORT_STATE]);
                }

                VkPipelineRasterizationStateCreateInfo* pRasterizationState = nullptr;
                if (IsCreateInfoExists(file, STR_MEMBER_NAME_P_RASTERIZATION_STATE))
                {
                    pRasterizationState = create_info->GetPipelineRasterizationStateCreateInfo();
                    ReadStructure(pRasterizationState, file[STR_MEMBER_NAME_P_RASTERIZATION_STATE]);
                }

                VkPipelineMultisampleStateCreateInfo* pMultisampleState = nullptr;
                if (IsCreateInfoExists(file, STR_MEMBER_NAME_P_MULTISAMPLE_STATE))
                {
                    pMultisampleState = create_info->GetPipelineMultisampleStateCreateInfo();
                    ReadStructure(pMultisampleState, file[STR_MEMBER_NAME_P_MULTISAMPLE_STATE]);
                }

                VkPipelineDepthStencilStateCreateInfo* pDepthStencilState = nullptr;
                if (IsCreateInfoExists(file, STR_MEMBER_NAME_P_DEPTH_STENCIL_STATE))
                {
                    pDepthStencilState = create_info->GetPipelineDepthStencilStateCreateInfo();
                    ReadStructure(pDepthStencilState, file[STR_MEMBER_NAME_P_DEPTH_STENCIL_STATE]);
                }

                VkPipelineColorBlendStateCreateInfo* pColorBlendState = nullptr;
                if (IsCreateInfoExists(file, STR_MEMBER_NAME_P_COLOR_BLEND_STATE))
                {
                    pColorBlendState = create_info->GetPipelineColorBlendStateCreateInfo();
                    ReadStructure(pColorBlendState, file[STR_MEMBER_NAME_P_COLOR_BLEND_STATE]);
                }

                if (IsCreateInfoExists(file, STR_MEMBER_NAME_LAYOUT))
                {
                    pGraphicsPipelineCreateInfo->layout = reinterpret_cast<VkPipelineLayout>(ReadHandle(file[STR_MEMBER_NAME_LAYOUT]));
                }

                if (IsCreateInfoExists(file, STR_MEMBER_NAME_RENDER_PASS))
                {
                    pGraphicsPipelineCreateInfo->renderPass = reinterpret_cast<VkRenderPass>(ReadHandle(file[STR_MEMBER_NAME_RENDER_PASS]));
                }

                pGraphicsPipelineCreateInfo->subpass = file[STR_MEMBER_NAME_SUBPASS];

                if (IsCreateInfoExists(file, STR_MEMBER_NAME_BASE_PIPELINE_HANDLE))
                {
                    pGraphicsPipelineCreateInfo->basePipelineHandle = reinterpret_cast<VkPipeline>(ReadHandle(file[STR_MEMBER_NAME_BASE_PIPELINE_HANDLE]));
                }

                pGraphicsPipelineCreateInfo->basePipelineIndex = file[STR_MEMBER_NAME_BASE_PIPELINE_INDEX];
            }
        }
    }

    void WriteStructure(const VkGraphicsPipelineCreateInfo* create_info, nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            file[STR_MEMBER_NAME_TYPE] = create_info->sType;
            file[STR_MEMBER_NAME_PNEXT] = WriteHandle(reinterpret_cast<uint64_t>(create_info->pNext));
            file[STR_MEMBER_NAME_FLAGS] = create_info->flags;
            file[STR_MEMBER_NAME_STAGE_COUNT] = create_info->stageCount;

            for (uint32_t index = 0; index < create_info->stageCount; ++index)
            {
                WriteStructure((create_info->pStages + index), file[STR_MEMBER_NAME_PSTAGES][index]);
            }

            if (create_info->pVertexInputState != nullptr)
            {
                WriteStructure(create_info->pVertexInputState, file[STR_MEMBER_NAME_P_VERTEX_INPUT_STATE]);
            }

            if (create_info->pInputAssemblyState != nullptr)
            {
                WriteStructure(create_info->pInputAssemblyState, file[STR_MEMBER_NAME_P_INPUT_ASSEMBLY_STATE]);
            }

            if (create_info->pTessellationState != nullptr)
            {
                WriteStructure(create_info->pTessellationState, file[STR_MEMBER_NAME_P_TESSELLATION_STATE]);
            }

            if (create_info->pViewportState != nullptr)
            {
                WriteStructure(create_info->pViewportState, file[STR_MEMBER_NAME_P_VIEWPORT_STATE]);
            }

            if (create_info->pRasterizationState != nullptr)
            {
                WriteStructure(create_info->pRasterizationState, file[STR_MEMBER_NAME_P_RASTERIZATION_STATE]);
            }

            if (create_info->pMultisampleState != nullptr)
            {
                WriteStructure(create_info->pMultisampleState, file[STR_MEMBER_NAME_P_MULTISAMPLE_STATE]);
            }

            if (create_info->pDepthStencilState != nullptr)
            {
                WriteStructure(create_info->pDepthStencilState, file[STR_MEMBER_NAME_P_DEPTH_STENCIL_STATE]);
            }

            if (create_info->pColorBlendState != nullptr)
            {
                WriteStructure(create_info->pColorBlendState, file[STR_MEMBER_NAME_P_COLOR_BLEND_STATE]);
            }

            if (create_info->pDynamicState != nullptr)
            {
                WriteStructure(create_info->pDynamicState, file[STR_MEMBER_NAME_P_DYNAMIC_STATE]);
            }

            if (create_info->layout != VK_NULL_HANDLE)
            {
                file[STR_MEMBER_NAME_LAYOUT] = WriteHandle(reinterpret_cast<uint64_t>(create_info->layout));
            }

            if (create_info->renderPass != VK_NULL_HANDLE)
            {
                file[STR_MEMBER_NAME_RENDER_PASS] = WriteHandle(reinterpret_cast<uint64_t>(create_info->renderPass));
            }
            file[STR_MEMBER_NAME_SUBPASS] = create_info->subpass;
            if (create_info->basePipelineHandle != VK_NULL_HANDLE)
            {
                file[STR_MEMBER_NAME_BASE_PIPELINE_HANDLE] = WriteHandle(reinterpret_cast<uint64_t>(create_info->basePipelineHandle));
            }
            file[STR_MEMBER_NAME_BASE_PIPELINE_INDEX] = create_info->basePipelineIndex;
        }
    }

    void WriteStructure(const VkPipelineShaderStageCreateInfo* create_info, nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            file[STR_MEMBER_NAME_TYPE] = create_info->sType;
            file[STR_MEMBER_NAME_PNEXT] = WriteHandle(reinterpret_cast<uint64_t>(create_info->pNext));
            file[STR_MEMBER_NAME_FLAGS] = create_info->flags;
            file[STR_MEMBER_NAME_STAGE] = create_info->stage;
            file[STR_MEMBER_NAME_MODULE] = WriteHandle(reinterpret_cast<uint64_t>(create_info->module));

            if (create_info->pName != nullptr)
            {
                file[STR_MEMBER_NAME_NAME] = std::string(create_info->pName);
            }

            if (create_info->pSpecializationInfo != nullptr)
            {
                WriteStructure(create_info->pSpecializationInfo, file[STR_MEMBER_NAME_P_SPECIALIZATION_INFO]);
            }
        }
    }

    void ReadStructure(VkPipelineShaderStageCreateInfo* create_info, const nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            create_info->sType = file[STR_MEMBER_NAME_TYPE];
            create_info->pNext = ReadHandle(file[STR_MEMBER_NAME_PNEXT]);
            create_info->flags = file[STR_MEMBER_NAME_FLAGS];
            create_info->stage = file[STR_MEMBER_NAME_STAGE];
            create_info->module = (VkShaderModule)ReadHandle(file[STR_MEMBER_NAME_MODULE]);
            if (IsCreateInfoExists(file, STR_MEMBER_NAME_NAME))
            {
                std::string entrypointName = file[STR_MEMBER_NAME_NAME];
                size_t bufferSize = entrypointName.length() + 1;
                char* pEntrypointName = new char[bufferSize] {};
                STRCPY(pEntrypointName, bufferSize, entrypointName.c_str());
                create_info->pName = pEntrypointName;
            }

            if (IsCreateInfoExists(file, STR_MEMBER_NAME_P_SPECIALIZATION_INFO))
            {
                VkSpecializationInfo* pInfo = new VkSpecializationInfo{};
                ReadStructure(pInfo, file[STR_MEMBER_NAME_P_SPECIALIZATION_INFO]);
                create_info->pSpecializationInfo = pInfo;
            }
        }
    }

    std::string WriteHandle(uint64_t handle)
    {
        const size_t bufferSize = 1024;
        char buffer[bufferSize];
        snprintf(buffer, bufferSize, "0x%p", (void*)handle);
        return std::string(buffer);
    }

    void WriteStructure(const VkPipelineVertexInputStateCreateInfo* create_info, nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            file[STR_MEMBER_NAME_TYPE] = create_info->sType;
            file[STR_MEMBER_NAME_PNEXT] = WriteHandle(reinterpret_cast<uint64_t>(create_info->pNext));
            file[STR_MEMBER_NAME_FLAGS] = create_info->flags;
            file[STR_MEMBER_NAME_VERTEX_BINDING_DESCRIPTION_COUNT] = create_info->vertexBindingDescriptionCount;
            file[STR_MEMBER_NAME_VERTEX_ATTRIBUTE_DESCRIPTION_COUNT] = create_info->vertexAttributeDescriptionCount;
            for (uint32_t index = 0; index < create_info->vertexBindingDescriptionCount; ++index)
            {
                WriteStructure(create_info->pVertexBindingDescriptions + index, file[STR_MEMBER_NAME_P_VERTEX_BINDING_DESCRIPTIONS][index]);
            }
            for (uint32_t index = 0; index < create_info->vertexAttributeDescriptionCount; ++index)
            {
                WriteStructure(create_info->pVertexAttributeDescriptions + index, file[STR_MEMBER_NAME_P_VERTEX_ATTRIBUTE_DESCRIPTIONS][index]);
            }
        }
    }

    void ReadStructure(VkPipelineVertexInputStateCreateInfo* create_info, const nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            create_info->sType = file[STR_MEMBER_NAME_TYPE];
            create_info->pNext = ReadHandle(file[STR_MEMBER_NAME_PNEXT]);
            create_info->flags = file[STR_MEMBER_NAME_FLAGS];
            create_info->vertexBindingDescriptionCount = file[STR_MEMBER_NAME_VERTEX_BINDING_DESCRIPTION_COUNT];

            VkVertexInputBindingDescription* pVertexBindingDescriptions = nullptr;
            if (create_info->vertexBindingDescriptionCount > 0)
            {
                pVertexBindingDescriptions = new VkVertexInputBindingDescription[create_info->vertexBindingDescriptionCount]{};
                for (uint32_t bindingIndex = 0; bindingIndex < create_info->vertexBindingDescriptionCount; ++bindingIndex)
                {
                    ReadStructure(pVertexBindingDescriptions + bindingIndex, file[STR_MEMBER_NAME_P_VERTEX_BINDING_DESCRIPTIONS][bindingIndex]);
                }
            }
            create_info->pVertexBindingDescriptions = pVertexBindingDescriptions;

            create_info->vertexAttributeDescriptionCount = file[STR_MEMBER_NAME_VERTEX_ATTRIBUTE_DESCRIPTION_COUNT];

            VkVertexInputAttributeDescription* pVertexAttributeDescriptions = nullptr;
            if (create_info->vertexAttributeDescriptionCount > 0)
            {
                pVertexAttributeDescriptions = new VkVertexInputAttributeDescription[create_info->vertexAttributeDescriptionCount]{};
                for (uint32_t attributeIndex = 0; attributeIndex < create_info->vertexAttributeDescriptionCount; ++attributeIndex)
                {
                    ReadStructure(pVertexAttributeDescriptions + attributeIndex, file[STR_MEMBER_NAME_P_VERTEX_ATTRIBUTE_DESCRIPTIONS][attributeIndex]);
                }
            }
            create_info->pVertexAttributeDescriptions = pVertexAttributeDescriptions;
        }
    }

    void WriteStructure(const VkPipelineInputAssemblyStateCreateInfo* create_info, nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            file[STR_MEMBER_NAME_TYPE] = create_info->sType;
            file[STR_MEMBER_NAME_PNEXT] = WriteHandle(reinterpret_cast<uint64_t>(create_info->pNext));
            file[STR_MEMBER_NAME_FLAGS] = create_info->flags;
            file[STR_MEMBER_NAME_TOPOLOGY] = create_info->topology;
            file[STR_MEMBER_NAME_PRIMITIVE_RESTART_ENABLED] = WriteBool(create_info->primitiveRestartEnable);
        }
    }

    void ReadStructure(VkPipelineInputAssemblyStateCreateInfo* create_info, const nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            create_info->sType = file[STR_MEMBER_NAME_TYPE];
            create_info->pNext = ReadHandle(file[STR_MEMBER_NAME_PNEXT]);
            create_info->flags = file[STR_MEMBER_NAME_FLAGS];
            create_info->topology = file[STR_MEMBER_NAME_TOPOLOGY];
            create_info->primitiveRestartEnable = ReadBool(file[STR_MEMBER_NAME_PRIMITIVE_RESTART_ENABLED]);
        }
    }

    void WriteStructure(const VkPipelineTessellationStateCreateInfo* create_info, nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            file[STR_MEMBER_NAME_TYPE] = create_info->sType;
            file[STR_MEMBER_NAME_PNEXT] = WriteHandle(reinterpret_cast<uint64_t>(create_info->pNext));
            file[STR_MEMBER_NAME_FLAGS] = create_info->flags;
            file[STR_MEMBER_NAME_PATCH_CONTROL_POINTS] = create_info->patchControlPoints;
        }
    }

    void ReadStructure(VkPipelineTessellationStateCreateInfo* create_info, const nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            create_info->sType = file[STR_MEMBER_NAME_TYPE];
            create_info->pNext = ReadHandle(file[STR_MEMBER_NAME_PNEXT]);
            create_info->flags = file[STR_MEMBER_NAME_FLAGS];
            create_info->patchControlPoints = file[STR_MEMBER_NAME_PATCH_CONTROL_POINTS];
        }
    }

    void WriteStructure(const VkPipelineViewportStateCreateInfo* create_info, nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            file[STR_MEMBER_NAME_TYPE] = create_info->sType;
            file[STR_MEMBER_NAME_PNEXT] = WriteHandle(reinterpret_cast<uint64_t>(create_info->pNext));
            file[STR_MEMBER_NAME_FLAGS] = create_info->flags;
            file[STR_MEMBER_NAME_VIEWPORT_COUNT] = create_info->viewportCount;
            file[STR_MEMBER_NAME_SCISSOR_COUNT] = create_info->scissorCount;
            if (create_info->pViewports != nullptr)
            {
                for (uint32_t index = 0; index < create_info->viewportCount; ++index)
                {
                    WriteStructure(create_info->pViewports + index, file[STR_MEMBER_NAME_P_VIEWPORTS][index]);
                }
            }

            if (create_info->pScissors != nullptr)
            {
                for (uint32_t index = 0; index < create_info->scissorCount; ++index)
                {
                    WriteStructure(create_info->pScissors + index, file[STR_MEMBER_NAME_P_SCISSORS][index]);
                }
            }
        }
    }

    void ReadStructure(VkPipelineViewportStateCreateInfo* create_info, const nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            create_info->sType = file[STR_MEMBER_NAME_TYPE];
            create_info->pNext = ReadHandle(file[STR_MEMBER_NAME_PNEXT]);
            create_info->flags = file[STR_MEMBER_NAME_FLAGS];

            // Parse the array of viewport rectangles.
            create_info->viewportCount = file[STR_MEMBER_NAME_VIEWPORT_COUNT];
            VkViewport* pViewports = nullptr;
            if (IsCreateInfoExists(file, STR_MEMBER_NAME_P_VIEWPORTS))
            {
                if (create_info->viewportCount > 0)
                {
                    pViewports = new VkViewport[create_info->viewportCount]{};
                    for (uint32_t index = 0; index < create_info->viewportCount; ++index)
                    {
                        ReadStructure(pViewports + index, file[STR_MEMBER_NAME_P_VIEWPORTS][index]);
                    }
                }
            }
            else
            {
                // In case that there is no viewport given, we would allocate a default one.
                create_info->viewportCount = 1;
                pViewports = new VkViewport{};
                pViewports->height = 1080;
                pViewports->width = 1920;
                pViewports->maxDepth = 1;
            }

            // Set the viewport in the create info structure.
            create_info->pViewports = pViewports;

            // Parse the array of scissor rectangles.
            create_info->scissorCount = file[STR_MEMBER_NAME_SCISSOR_COUNT];
            VkRect2D* pScissors = nullptr;
            if (IsCreateInfoExists(file, STR_MEMBER_NAME_P_SCISSORS))
            {
                if (create_info->scissorCount > 0)
                {
                    pScissors = new VkRect2D[create_info->scissorCount]{};
                    for (uint32_t index = 0; index < create_info->scissorCount; ++index)
                    {
                        ReadStructure(pScissors + index, file[STR_MEMBER_NAME_P_SCISSORS][index]);
                    }
                }
            }
            else
            {
                // In case that there is no scissor given, we would allocate a default one.
                pScissors = new VkRect2D[1]{};
                create_info->scissorCount = 1;
                pScissors->extent.height = 1080;
                pScissors->extent.width = 1920;
            }

            // Set the scissors in the create info structure.
            create_info->pScissors = pScissors;
        }
    }

    void WriteStructure(const VkPipelineRasterizationStateCreateInfo* create_info, nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            file[STR_MEMBER_NAME_TYPE] = create_info->sType;
            file[STR_MEMBER_NAME_PNEXT] = WriteHandle(reinterpret_cast<uint64_t>(create_info->pNext));
            file[STR_MEMBER_NAME_FLAGS] = create_info->flags;
            file[STR_MEMBER_NAME_DEPTH_CLAMP_ENABLE] = WriteBool(create_info->depthClampEnable);
            file[STR_MEMBER_NAME_RASTERIZER_DISCARD_ENABLE] = WriteBool(create_info->rasterizerDiscardEnable);
            file[STR_MEMBER_NAME_POLYGON_MODE] = create_info->polygonMode;
            file[STR_MEMBER_NAME_CULL_MODE] = create_info->cullMode;
            file[STR_MEMBER_NAME_FRONT_FACE] = create_info->frontFace;
            file[STR_MEMBER_NAME_DEPTH_BIAS_ENABLE] = WriteBool(create_info->depthBiasEnable);
            file[STR_MEMBER_NAME_DEPTH_BIAS_CONSTANT_FACTOR] = create_info->depthBiasConstantFactor;
            file[STR_MEMBER_NAME_DEPTH_BIAS_CLAMP] = create_info->depthBiasClamp;
            file[STR_MEMBER_NAME_DEPTH_BIAS_SLOPE_FACTOR] = create_info->depthBiasSlopeFactor;
            file[STR_MEMBER_NAME_LINE_WIDTH] = create_info->lineWidth;
        }
    }

    void ReadStructure(VkPipelineRasterizationStateCreateInfo* create_info, const nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            create_info->sType = file[STR_MEMBER_NAME_TYPE];
            create_info->pNext = ReadHandle(file[STR_MEMBER_NAME_PNEXT]);
            create_info->flags = file[STR_MEMBER_NAME_FLAGS];
            create_info->depthClampEnable = ReadBool(file[STR_MEMBER_NAME_DEPTH_CLAMP_ENABLE]);
            create_info->rasterizerDiscardEnable = ReadBool(file[STR_MEMBER_NAME_RASTERIZER_DISCARD_ENABLE]);
            create_info->polygonMode = file[STR_MEMBER_NAME_POLYGON_MODE];
            create_info->cullMode = file[STR_MEMBER_NAME_CULL_MODE];
            create_info->frontFace = file[STR_MEMBER_NAME_FRONT_FACE];
            create_info->depthBiasEnable = ReadBool(file[STR_MEMBER_NAME_DEPTH_BIAS_ENABLE]);
            create_info->depthBiasConstantFactor = file[STR_MEMBER_NAME_DEPTH_BIAS_CONSTANT_FACTOR];
            create_info->depthBiasClamp = file[STR_MEMBER_NAME_DEPTH_BIAS_CLAMP];
            create_info->depthBiasSlopeFactor = file[STR_MEMBER_NAME_DEPTH_BIAS_SLOPE_FACTOR];
            create_info->lineWidth = file[STR_MEMBER_NAME_LINE_WIDTH];
        }
    }

    void WriteStructure(const VkPipelineMultisampleStateCreateInfo* create_info, nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            file[STR_MEMBER_NAME_TYPE] = create_info->sType;
            file[STR_MEMBER_NAME_PNEXT] = WriteHandle(reinterpret_cast<uint64_t>(create_info->pNext));
            file[STR_MEMBER_NAME_FLAGS] = create_info->flags;
            file[STR_MEMBER_NAME_RASTERIZATION_SAMPLE] = create_info->rasterizationSamples;
            file[STR_MEMBER_NAME_SAMPLE_SHADING_ENABLE] = WriteBool(create_info->sampleShadingEnable);
            file[STR_MEMBER_NAME_MIN_SAMPLE_SHADING] = create_info->minSampleShading;
            WriteSampleMask(create_info, file);
            file[STR_MEMBER_NAME_ALPHA_TO_COVERAGE_ENABLE] = WriteBool(create_info->alphaToCoverageEnable);
            file[STR_MEMBER_NAME_ALPHA_TO_ONE_ENABLE] = WriteBool(create_info->alphaToOneEnable);
        }
    }

    void ReadStructure(VkPipelineMultisampleStateCreateInfo* create_info, const nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            create_info->sType = file[STR_MEMBER_NAME_TYPE];
            create_info->pNext = ReadHandle(file[STR_MEMBER_NAME_PNEXT]);
            create_info->flags = file[STR_MEMBER_NAME_FLAGS];
            create_info->rasterizationSamples = file[STR_MEMBER_NAME_RASTERIZATION_SAMPLE];
            create_info->sampleShadingEnable = ReadBool(file[STR_MEMBER_NAME_SAMPLE_SHADING_ENABLE]);
            create_info->minSampleShading = file[STR_MEMBER_NAME_MIN_SAMPLE_SHADING];
            create_info->pSampleMask = ReadSampleMask(create_info, file);
            create_info->alphaToCoverageEnable = ReadBool(file[STR_MEMBER_NAME_ALPHA_TO_COVERAGE_ENABLE]);
            create_info->alphaToOneEnable = ReadBool(file[STR_MEMBER_NAME_ALPHA_TO_ONE_ENABLE]);
        }
    }

    virtual void WriteStructure(const VkStencilOpState* create_info, nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            file[STR_MEMBER_NAME_PASS_OP] = create_info->passOp;
            file[STR_MEMBER_NAME_DEPTH_FAIL_OP] = create_info->depthFailOp;
            file[STR_MEMBER_NAME_COMPARE_OP] = create_info->compareOp;
            file[STR_MEMBER_NAME_COMPARE_MASK] = create_info->compareMask;
            file[STR_MEMBER_NAME_WRITE_MASK] = create_info->writeMask;
            file[STR_MEMBER_NAME_REFERENCE] = create_info->reference;
        }
    }

    virtual void ReadStructure(VkStencilOpState* create_info, const nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            create_info->passOp = file[STR_MEMBER_NAME_PASS_OP];
            create_info->depthFailOp = file[STR_MEMBER_NAME_DEPTH_FAIL_OP];
            create_info->compareOp = file[STR_MEMBER_NAME_COMPARE_OP];
            create_info->compareMask = file[STR_MEMBER_NAME_COMPARE_MASK];
            create_info->writeMask = file[STR_MEMBER_NAME_WRITE_MASK];
            create_info->reference = file[STR_MEMBER_NAME_REFERENCE];
        }
    }

    void WriteStructure(const VkPipelineDepthStencilStateCreateInfo* create_info, nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            file[STR_MEMBER_NAME_TYPE] = create_info->sType;
            file[STR_MEMBER_NAME_PNEXT] = WriteHandle(reinterpret_cast<uint64_t>(create_info->pNext));
            file[STR_MEMBER_NAME_FLAGS] = create_info->flags;
            file[STR_MEMBER_NAME_DEPTH_TEST_ENABLE] = WriteBool(create_info->depthTestEnable);
            file[STR_MEMBER_NAME_DEPTH_WRITE_ENABLE] = WriteBool(create_info->depthWriteEnable);
            file[STR_MEMBER_NAME_DEPTH_COMPARE_OP] = create_info->depthCompareOp;
            file[STR_MEMBER_NAME_DEPTH_BOUNDS_TEST_ENABLE] = WriteBool(create_info->depthBoundsTestEnable);
            file[STR_MEMBER_NAME_STENCIL_TEST_ENABLE] = WriteBool(create_info->stencilTestEnable);
            WriteStructure(&create_info->front, file[STR_MEMBER_NAME_FRONT]);
            WriteStructure(&create_info->back, file[STR_MEMBER_NAME_BACK]);
            file[STR_MEMBER_NAME_MIN_DEPTH_BOUNDS] = create_info->minDepthBounds;
            file[STR_MEMBER_NAME_MAX_DEPTH_BOUNDS] = create_info->maxDepthBounds;
        }
    }

    void ReadStructure(VkPipelineDepthStencilStateCreateInfo* create_info, const nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            create_info->sType = file[STR_MEMBER_NAME_TYPE];
            create_info->pNext = ReadHandle(file[STR_MEMBER_NAME_PNEXT]);
            create_info->flags = file[STR_MEMBER_NAME_FLAGS];
            create_info->depthTestEnable = ReadBool(file[STR_MEMBER_NAME_DEPTH_TEST_ENABLE]);
            create_info->depthWriteEnable = ReadBool(file[STR_MEMBER_NAME_DEPTH_WRITE_ENABLE]);
            create_info->depthCompareOp = file[STR_MEMBER_NAME_DEPTH_COMPARE_OP];

            create_info->depthBoundsTestEnable = ReadBool(file[STR_MEMBER_NAME_DEPTH_BOUNDS_TEST_ENABLE]);
            create_info->stencilTestEnable = ReadBool(file[STR_MEMBER_NAME_STENCIL_TEST_ENABLE]);
            ReadStructure(&create_info->front, file[STR_MEMBER_NAME_FRONT]);
            ReadStructure(&create_info->back, file[STR_MEMBER_NAME_BACK]);
            create_info->minDepthBounds = file[STR_MEMBER_NAME_MIN_DEPTH_BOUNDS];
            create_info->maxDepthBounds = file[STR_MEMBER_NAME_MAX_DEPTH_BOUNDS];
        }
    }

    void WriteStructure(const VkPipelineColorBlendAttachmentState* create_info, nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            file[STR_MEMBER_NAME_BLEND_ENABLE] = WriteBool(create_info->blendEnable);
            file[STR_MEMBER_NAME_SRC_COLOR_BLEND_FACTOR] = create_info->srcColorBlendFactor;
            file[STR_MEMBER_NAME_DST_COLOR_BLEND_FACTOR] = create_info->dstColorBlendFactor;
            file[STR_MEMBER_NAME_COLOR_BLEND_OP] = create_info->colorBlendOp;
            file[STR_MEMBER_NAME_SRC_ALPHA_BLEND_FACTOR] = create_info->srcAlphaBlendFactor;
            file[STR_MEMBER_NAME_DST_ALPHA_BLEND_FACTOR] = create_info->dstAlphaBlendFactor;
            file[STR_MEMBER_NAME_ALPHA_BLEND_OP] = create_info->alphaBlendOp;
            file[STR_MEMBER_NAME_COLOR_WRITE_MASK] = create_info->colorWriteMask;
        }
    }

    void ReadStructure(VkPipelineColorBlendAttachmentState* create_info, const nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            create_info->blendEnable = ReadBool(file[STR_MEMBER_NAME_BLEND_ENABLE]);
            create_info->srcColorBlendFactor = file[STR_MEMBER_NAME_SRC_COLOR_BLEND_FACTOR];
            create_info->dstColorBlendFactor = file[STR_MEMBER_NAME_DST_COLOR_BLEND_FACTOR];
            create_info->colorBlendOp = file[STR_MEMBER_NAME_COLOR_BLEND_OP];
            create_info->srcAlphaBlendFactor = file[STR_MEMBER_NAME_SRC_ALPHA_BLEND_FACTOR];
            create_info->dstAlphaBlendFactor = file[STR_MEMBER_NAME_DST_ALPHA_BLEND_FACTOR];
            create_info->alphaBlendOp = file[STR_MEMBER_NAME_ALPHA_BLEND_OP];
            create_info->colorWriteMask = file[STR_MEMBER_NAME_COLOR_WRITE_MASK];
        }
    }

    void WriteStructure(const VkPipelineColorBlendStateCreateInfo* create_info, nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            file[STR_MEMBER_NAME_TYPE] = create_info->sType;
            file[STR_MEMBER_NAME_PNEXT] = WriteHandle(reinterpret_cast<uint64_t>(create_info->pNext));
            file[STR_MEMBER_NAME_FLAGS] = create_info->flags;
            file[STR_MEMBER_NAME_LOGIC_OP_ENABLE] = WriteBool(create_info->logicOpEnable);
            file[STR_MEMBER_NAME_LOGIC_OP] = create_info->logicOp;
            file[STR_MEMBER_NAME_ATTACHMENT_COUNT] = create_info->attachmentCount;
            for (uint32_t attachmentIndex = 0; attachmentIndex < create_info->attachmentCount; attachmentIndex++)
            {
                WriteStructure(create_info->pAttachments + attachmentIndex, file[STR_MEMBER_NAME_P_ATTACHMENTS][attachmentIndex]);
            }
            for (uint32_t constantIndex = 0; constantIndex < 4; constantIndex++)
            {
                file[STR_MEMBER_NAME_BLEND_CONSTANTS][constantIndex] = create_info->blendConstants[constantIndex];
            }
        }
    }

    void ReadStructure(VkPipelineColorBlendStateCreateInfo* create_info, const nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            create_info->sType = file[STR_MEMBER_NAME_TYPE];
            create_info->pNext = ReadHandle(file[STR_MEMBER_NAME_PNEXT]);
            create_info->flags = file[STR_MEMBER_NAME_FLAGS];
            create_info->logicOpEnable = ReadBool(file[STR_MEMBER_NAME_LOGIC_OP_ENABLE]);
            create_info->logicOp = file[STR_MEMBER_NAME_LOGIC_OP];
            create_info->attachmentCount = file[STR_MEMBER_NAME_ATTACHMENT_COUNT];

            VkPipelineColorBlendAttachmentState* pAttachments = nullptr;
            if (create_info->attachmentCount > 0)
            {
                pAttachments = new VkPipelineColorBlendAttachmentState[create_info->attachmentCount]{};
                for (uint32_t attachmentIndex = 0; attachmentIndex < create_info->attachmentCount; attachmentIndex++)
                {
                    ReadStructure(pAttachments + attachmentIndex, file[STR_MEMBER_NAME_P_ATTACHMENTS][attachmentIndex]);
                }
            }
            create_info->pAttachments = pAttachments;

            for (uint32_t constantIndex = 0; constantIndex < 4; constantIndex++)
            {
                create_info->blendConstants[constantIndex] = file[STR_MEMBER_NAME_BLEND_CONSTANTS][constantIndex];
            }
        }
    }

    void WriteStructure(const VkPipelineDynamicStateCreateInfo* create_info, nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            file[STR_MEMBER_NAME_TYPE] = create_info->sType;
            file[STR_MEMBER_NAME_PNEXT] = WriteHandle(reinterpret_cast<uint64_t>(create_info->pNext));
            file[STR_MEMBER_NAME_FLAGS] = create_info->flags;
            file[STR_MEMBER_NAME_DYNAMIC_STATE_COUNT] = create_info->dynamicStateCount;

            for (uint32_t stateIndex = 0; stateIndex < create_info->dynamicStateCount; ++stateIndex)
            {
                file[STR_MEMBER_NAME_P_DYNAMIC_STATES][stateIndex] = *(create_info->pDynamicStates + stateIndex);
            }
        }
    }

    void ReadStructure(VkPipelineDynamicStateCreateInfo* create_info, const nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            create_info->sType = file[STR_MEMBER_NAME_TYPE];
            create_info->pNext = ReadHandle(file[STR_MEMBER_NAME_PNEXT]);
            create_info->flags = file[STR_MEMBER_NAME_FLAGS];
            create_info->dynamicStateCount = file[STR_MEMBER_NAME_DYNAMIC_STATE_COUNT];

            VkDynamicState* pDynamicStates = nullptr;
            if (create_info->dynamicStateCount > 0)
            {
                pDynamicStates = new VkDynamicState[create_info->dynamicStateCount]{};
                for (uint32_t stateIndex = 0; stateIndex < create_info->dynamicStateCount; ++stateIndex)
                {
                    pDynamicStates[stateIndex] = file[STR_MEMBER_NAME_P_DYNAMIC_STATES][stateIndex];
                }
            }
            create_info->pDynamicStates = pDynamicStates;
        }
    }

    void WriteStructure(const VkSpecializationInfo* create_info, nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            file[STR_MEMBER_NAME_MAP_ENTRY_COUNT] = create_info->mapEntryCount;
            for (uint32_t index = 0; index < create_info->mapEntryCount; ++index)
            {
                WriteStructure((create_info->pMapEntries + index), file[STR_MEMBER_NAME_P_MAP_ENTRIES][index]);
            }
            file[STR_MEMBER_NAME_DATA_SIZE] = create_info->dataSize;

            // create_info->dataSize is the number of bytes being serialized. In order to serialize
            // binary data, read/write each byte separately as an array.
            for (size_t byteIndex = 0; byteIndex < create_info->dataSize; ++byteIndex)
            {
                file[STR_MEMBER_NAME_P_DATA][byteIndex] = *((const uint8_t*)create_info->pData + byteIndex);
            }
        }
    }

    void ReadStructure(VkSpecializationInfo* create_info, const nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            create_info->mapEntryCount = file[STR_MEMBER_NAME_MAP_ENTRY_COUNT];
            VkSpecializationMapEntry* pMapEntries = nullptr;
            if (create_info->mapEntryCount > 0)
            {
                pMapEntries = new VkSpecializationMapEntry[create_info->mapEntryCount];
                for (uint32_t entryIndex = 0; entryIndex < create_info->mapEntryCount; ++entryIndex)
                {
                    ReadStructure(pMapEntries + entryIndex, file[STR_MEMBER_NAME_P_MAP_ENTRIES][entryIndex]);
                }
            }
            create_info->pMapEntries = pMapEntries;
            create_info->dataSize = file[STR_MEMBER_NAME_DATA_SIZE];

            // Allocate a byte array where the deserialized data will be copied.
            uint8_t* pDataBytes = new uint8_t[create_info->dataSize]{};

            // create_info->dataSize is the number of bytes being serialized.
            for (size_t byteIndex = 0; byteIndex < create_info->dataSize; ++byteIndex)
            {
                *(pDataBytes + byteIndex) = file[STR_MEMBER_NAME_P_DATA][byteIndex];
            }
            create_info->pData = pDataBytes;
        }
    }

    void WriteStructure(const VkVertexInputBindingDescription* create_info, nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            file[STR_MEMBER_NAME_BINDING] = create_info->binding;
            file[STR_MEMBER_NAME_STRIDE] = create_info->stride;
            file[STR_MEMBER_NAME_INPUT_RATE] = create_info->inputRate;
        }
    }

    void ReadStructure(VkVertexInputBindingDescription* create_info, const nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            create_info->binding = file[STR_MEMBER_NAME_BINDING];
            create_info->stride = file[STR_MEMBER_NAME_STRIDE];
            create_info->inputRate = file[STR_MEMBER_NAME_INPUT_RATE];
        }
    }

    void WriteStructure(const VkSpecializationMapEntry* create_info, nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            file[STR_MEMBER_NAME_CONSTANT_ID] = create_info->constantID;
            file[STR_MEMBER_NAME_OFFSET] = create_info->offset;
            file[STR_MEMBER_NAME_SIZE] = create_info->size;
        }
    }

    void ReadStructure(VkSpecializationMapEntry* create_info, const nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            create_info->constantID = file[STR_MEMBER_NAME_CONSTANT_ID];
            create_info->offset = file[STR_MEMBER_NAME_OFFSET];
            create_info->size = file[STR_MEMBER_NAME_SIZE];
        }
    }

    void WriteStructure(const VkVertexInputAttributeDescription* create_info, nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            file[STR_MEMBER_NAME_LOCATION] = create_info->location;
            file[STR_MEMBER_NAME_BINDING] = create_info->binding;
            file[STR_MEMBER_NAME_FORMAT] = create_info->format;
            file[STR_MEMBER_NAME_OFFSET] = create_info->offset;
        }
    }

    void ReadStructure(VkVertexInputAttributeDescription* create_info, const nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            create_info->location = file[STR_MEMBER_NAME_LOCATION];
            create_info->binding = file[STR_MEMBER_NAME_BINDING];
            create_info->format = file[STR_MEMBER_NAME_FORMAT];
            create_info->offset = file[STR_MEMBER_NAME_OFFSET];
        }
    }

    void WriteStructure(const VkViewport* create_info, nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            file[STR_MEMBER_NAME_X] = create_info->x;
            file[STR_MEMBER_NAME_Y] = create_info->y;
            file[STR_MEMBER_NAME_WIDTH] = create_info->width;
            file[STR_MEMBER_NAME_HEIGHT] = create_info->height;
            file[STR_MEMBER_NAME_MIN_DEPTH] = create_info->minDepth;
            file[STR_MEMBER_NAME_MAX_DEPTH] = create_info->maxDepth;
        }
    }

    void ReadStructure(VkViewport* create_info, const nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            create_info->x = file[STR_MEMBER_NAME_X];
            create_info->y = file[STR_MEMBER_NAME_Y];
            create_info->width = file[STR_MEMBER_NAME_WIDTH];
            create_info->height = file[STR_MEMBER_NAME_HEIGHT];
            create_info->minDepth = file[STR_MEMBER_NAME_MIN_DEPTH];
            create_info->maxDepth = file[STR_MEMBER_NAME_MAX_DEPTH];
        }
    }

    void WriteStructure(const VkRect2D* create_info, nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            WriteStructure(&create_info->offset, file[STR_MEMBER_NAME_OFFSET]);
            WriteStructure(&create_info->extent, file[STR_MEMBER_NAME_EXTENT]);
        }
    }

    void ReadStructure(VkRect2D* create_info, const nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            ReadStructure(&create_info->offset, file[STR_MEMBER_NAME_OFFSET]);
            ReadStructure(&create_info->extent, file[STR_MEMBER_NAME_EXTENT]);
        }
    }

    void WriteStructure(const VkOffset2D* create_info, nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            file[STR_MEMBER_NAME_X] = create_info->x;
            file[STR_MEMBER_NAME_Y] = create_info->y;
        }
    }

    void ReadStructure(VkOffset2D* create_info, const nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            create_info->x = file[STR_MEMBER_NAME_X];
            create_info->y = file[STR_MEMBER_NAME_Y];
        }
    }

    void WriteStructure(const VkExtent2D* create_info, nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            file[STR_MEMBER_NAME_WIDTH] = create_info->width;
            file[STR_MEMBER_NAME_HEIGHT] = create_info->height;
        }
    }

    void ReadStructure(VkExtent2D* create_info, const nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            create_info->width = file[STR_MEMBER_NAME_WIDTH];
            create_info->height = file[STR_MEMBER_NAME_HEIGHT];
        }
    }

    void WriteStructure(const VkComputePipelineCreateInfo* create_info, nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            file[STR_MEMBER_NAME_TYPE] = create_info->sType;
            file[STR_MEMBER_NAME_PNEXT] = WriteHandle(reinterpret_cast<uint64_t>(create_info->pNext));
            file[STR_MEMBER_NAME_FLAGS] = create_info->flags;
            WriteStructure(&create_info->stage, file[STR_MEMBER_NAME_STAGE]);
            if (create_info->layout != VK_NULL_HANDLE)
            {
                file[STR_MEMBER_NAME_LAYOUT] = WriteHandle(reinterpret_cast<uint64_t>(create_info->layout));
            }

            if (create_info->basePipelineHandle != VK_NULL_HANDLE)
            {
                file[STR_MEMBER_NAME_BASE_PIPELINE_HANDLE] = WriteHandle(reinterpret_cast<uint64_t>(create_info->basePipelineHandle));
            }

            file[STR_MEMBER_NAME_BASE_PIPELINE_INDEX] = create_info->basePipelineIndex;
        }
    }

    void ReadStructure(VkComputePipelineCreateInfo* create_info, const nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            create_info->sType = file[STR_MEMBER_NAME_TYPE];
            create_info->pNext = ReadHandle(file[STR_MEMBER_NAME_PNEXT]);
            create_info->flags = file[STR_MEMBER_NAME_FLAGS];
            ReadStructure(&create_info->stage, file[STR_MEMBER_NAME_STAGE]);
            if (IsCreateInfoExists(file, STR_MEMBER_NAME_LAYOUT))
            {
                create_info->layout = (VkPipelineLayout)ReadHandle(file[STR_MEMBER_NAME_LAYOUT]);
            }

            if (IsCreateInfoExists(file, STR_MEMBER_NAME_BASE_PIPELINE_HANDLE))
            {
                create_info->basePipelineHandle = (VkPipeline)ReadHandle(file[STR_MEMBER_NAME_BASE_PIPELINE_HANDLE]);
            }

            create_info->basePipelineIndex = file[STR_MEMBER_NAME_BASE_PIPELINE_INDEX];
        }
    }

    void WriteStructure(const VkPipelineLayoutCreateInfo* create_info, nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            file[STR_MEMBER_NAME_TYPE] = create_info->sType;
            file[STR_MEMBER_NAME_PNEXT] = WriteHandle(reinterpret_cast<uint64_t>(create_info->pNext));
            file[STR_MEMBER_NAME_FLAGS] = create_info->flags;
            file[STR_MEMBER_NAME_SET_LAYOUT_COUNT] = create_info->setLayoutCount;

            if (create_info->setLayoutCount > 0)
            {
                // Set the indices of descriptor set layouts which are referenced by this pipeline layout.
                for (uint32_t i = 0; i < create_info->setLayoutCount; i++)
                {
                    file[STR_MEMBER_NAME_P_SET_LAYOUTS][i] = WriteHandle((uint64_t)i);
                }
            }
            file[STR_MEMBER_NAME_PUSH_CONSTANT_RANGE_COUNT] = create_info->pushConstantRangeCount;
            for (uint32_t pushConstantRangeIndex = 0; pushConstantRangeIndex < create_info->pushConstantRangeCount; ++pushConstantRangeIndex)
            {
                WriteStructure((create_info->pPushConstantRanges + pushConstantRangeIndex), file[STR_MEMBER_NAME_P_PUSH_CONSTANT_RANGES][pushConstantRangeIndex]);
            }
        }
    }

    void ReadStructure(VkPipelineLayoutCreateInfo* create_info, const nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            create_info->sType = file[STR_MEMBER_NAME_TYPE];
            create_info->pNext = ReadHandle(file[STR_MEMBER_NAME_PNEXT]);
            create_info->flags = file[STR_MEMBER_NAME_FLAGS];

            create_info->setLayoutCount = file[STR_MEMBER_NAME_SET_LAYOUT_COUNT];
            VkDescriptorSetLayout* pSetLayouts = nullptr;
            if (create_info->setLayoutCount > 0)
            {
                pSetLayouts = new VkDescriptorSetLayout[create_info->setLayoutCount]{};
                for (uint32_t setLayoutIndex = 0; setLayoutIndex < create_info->setLayoutCount; ++setLayoutIndex)
                {
                    pSetLayouts[setLayoutIndex] = (VkDescriptorSetLayout)ReadHandle(file[STR_MEMBER_NAME_P_SET_LAYOUTS][setLayoutIndex]);
                }
            }
            create_info->pSetLayouts = pSetLayouts;

            create_info->pushConstantRangeCount = file[STR_MEMBER_NAME_PUSH_CONSTANT_RANGE_COUNT];
            VkPushConstantRange* pPushConstantRanges = nullptr;
            if (create_info->pushConstantRangeCount > 0)
            {
                pPushConstantRanges = new VkPushConstantRange[create_info->pushConstantRangeCount];
                for (uint32_t pushConstantRangeIndex = 0; pushConstantRangeIndex < create_info->pushConstantRangeCount; ++pushConstantRangeIndex)
                {
                    ReadStructure(pPushConstantRanges + pushConstantRangeIndex, file[STR_MEMBER_NAME_P_PUSH_CONSTANT_RANGES][pushConstantRangeIndex]);
                }
            }
            create_info->pPushConstantRanges = pPushConstantRanges;
        }
    }

    void WriteStructure(const VkShaderModuleCreateInfo* create_info, nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            file[STR_MEMBER_NAME_TYPE] = create_info->sType;
            file[STR_MEMBER_NAME_PNEXT] = WriteHandle(reinterpret_cast<uint64_t>(create_info->pNext));
            file[STR_MEMBER_NAME_FLAGS] = create_info->flags;
            file[STR_MEMBER_NAME_CODE_SIZE] = create_info->codeSize;
        }
    }

    void ReadStructure(VkShaderModuleCreateInfo* create_info, const nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            create_info->sType = file[STR_MEMBER_NAME_TYPE];
            create_info->pNext = ReadHandle(file[STR_MEMBER_NAME_PNEXT]);
            create_info->flags = file[STR_MEMBER_NAME_FLAGS];
            create_info->codeSize = file[STR_MEMBER_NAME_CODE_SIZE];
        }
    }

    void WriteStructure(const VkDescriptorSetLayoutCreateInfo* create_info, nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            file[STR_MEMBER_NAME_TYPE] = create_info->sType;
            file[STR_MEMBER_NAME_PNEXT] = WriteHandle(reinterpret_cast<uint64_t>(create_info->pNext));
            file[STR_MEMBER_NAME_FLAGS] = create_info->flags;
            file[STR_MEMBER_NAME_BINDING_COUNT] = create_info->bindingCount;

            for (uint32_t bindingIndex = 0; bindingIndex < create_info->bindingCount; ++bindingIndex)
            {
                WriteStructure((create_info->pBindings + bindingIndex), file[STR_MEMBER_NAME_P_BINDINGS][bindingIndex]);
            }
        }
    }

    void ReadStructure(VkDescriptorSetLayoutCreateInfo* create_info, const nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            create_info->sType = file[STR_MEMBER_NAME_TYPE];
            create_info->pNext = ReadHandle(file[STR_MEMBER_NAME_PNEXT]);
            create_info->flags = file[STR_MEMBER_NAME_FLAGS];
            create_info->bindingCount = file[STR_MEMBER_NAME_BINDING_COUNT];

            VkDescriptorSetLayoutBinding* pBindings = nullptr;
            if (create_info->bindingCount > 0)
            {
                pBindings = new VkDescriptorSetLayoutBinding[create_info->bindingCount]{};
                for (uint32_t bindingIndex = 0; bindingIndex < create_info->bindingCount; ++bindingIndex)
                {
                    ReadStructure(pBindings + bindingIndex, file[STR_MEMBER_NAME_P_BINDINGS][bindingIndex]);
                }
            }
            create_info->pBindings = pBindings;
        }
    }

    void WriteStructure(const VkDescriptorSetLayoutBinding* create_info, nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            // Note: for now we are ignoring the immutable samplers.
            file[STR_MEMBER_NAME_BINDING] = create_info->binding;
            file[STR_MEMBER_NAME_DESCRIPTOR_TYPE] = create_info->descriptorType;
            file[STR_MEMBER_NAME_DESCRIPTOR_COUNT] = create_info->descriptorCount;
            file[STR_MEMBER_NAME_STAGE_FLAGS] = create_info->stageFlags;
        }
    }

    void ReadStructure(VkDescriptorSetLayoutBinding* create_info, const nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            create_info->binding = file[STR_MEMBER_NAME_BINDING];
            create_info->descriptorType = file[STR_MEMBER_NAME_DESCRIPTOR_TYPE];
            create_info->descriptorCount = file[STR_MEMBER_NAME_DESCRIPTOR_COUNT];
            create_info->stageFlags = file[STR_MEMBER_NAME_STAGE_FLAGS];
        }
    }

    void WriteStructure(const VkPushConstantRange* create_info, nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            file[STR_MEMBER_NAME_STAGE_FLAGS] = create_info->stageFlags;
            file[STR_MEMBER_NAME_OFFSET] = create_info->offset;
            file[STR_MEMBER_NAME_SIZE] = create_info->size;
        }
    }

    void ReadStructure(VkPushConstantRange* create_info, const nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            create_info->stageFlags = file[STR_MEMBER_NAME_STAGE_FLAGS];
            create_info->offset = file[STR_MEMBER_NAME_OFFSET];
            create_info->size = file[STR_MEMBER_NAME_SIZE];
        }
    }

    void WriteStructure(const VkRenderPassCreateInfo* create_info, nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            file[STR_MEMBER_NAME_TYPE] = create_info->sType;
            file[STR_MEMBER_NAME_PNEXT] = WriteHandle(reinterpret_cast<uint64_t>(create_info->pNext));
            file[STR_MEMBER_NAME_FLAGS] = create_info->flags;
            file[STR_MEMBER_NAME_ATTACHMENT_COUNT] = create_info->attachmentCount;

            for (uint32_t attachmentIndex = 0; attachmentIndex < create_info->attachmentCount; ++attachmentIndex)
            {
                WriteStructure(create_info->pAttachments + attachmentIndex, file[STR_MEMBER_NAME_P_ATTACHMENTS][attachmentIndex]);
            }

            file[STR_MEMBER_NAME_SUBPASS_COUNT] = create_info->subpassCount;
            for (uint32_t subpassIndex = 0; subpassIndex < create_info->subpassCount; ++subpassIndex)
            {
                WriteStructure(create_info->pSubpasses + subpassIndex, file[STR_MEMBER_NAME_P_SUBPASSES][subpassIndex]);
            }

            file[STR_MEMBER_NAME_DEPENDENCY_COUNT] = create_info->dependencyCount;
            for (uint32_t dependencyIndex = 0; dependencyIndex < create_info->dependencyCount; ++dependencyIndex)
            {
                WriteStructure(create_info->pDependencies + dependencyIndex, file[STR_MEMBER_NAME_P_DEPENDENCIES][dependencyIndex]);
            }
        }
    }

    void ReadStructure(VkRenderPassCreateInfo* create_info, const nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            create_info->sType = file[STR_MEMBER_NAME_TYPE];
            create_info->pNext = ReadHandle(file[STR_MEMBER_NAME_PNEXT]);
            create_info->flags = file[STR_MEMBER_NAME_FLAGS];
            create_info->attachmentCount = file[STR_MEMBER_NAME_ATTACHMENT_COUNT];

            VkAttachmentDescription* pAttachments = nullptr;
            if (create_info->attachmentCount > 0)
            {
                pAttachments = new VkAttachmentDescription[create_info->attachmentCount]{};
                for (uint32_t attachmentIndex = 0; attachmentIndex < create_info->attachmentCount; ++attachmentIndex)
                {
                    ReadStructure(pAttachments + attachmentIndex, file[STR_MEMBER_NAME_P_ATTACHMENTS][attachmentIndex]);
                }
            }
            create_info->pAttachments = pAttachments;

            create_info->subpassCount = file[STR_MEMBER_NAME_SUBPASS_COUNT];
            VkSubpassDescription* pSubpasses = nullptr;
            if (create_info->subpassCount > 0)
            {
                pSubpasses = new VkSubpassDescription[create_info->subpassCount]{};
                for (uint32_t subpassIndex = 0; subpassIndex < create_info->subpassCount; ++subpassIndex)
                {
                    ReadStructure(pSubpasses + subpassIndex, file[STR_MEMBER_NAME_P_SUBPASSES][subpassIndex]);
                }
            }
            create_info->pSubpasses = pSubpasses;

            create_info->dependencyCount = file[STR_MEMBER_NAME_DEPENDENCY_COUNT];
            VkSubpassDependency* pDependencies = nullptr;
            if (create_info->dependencyCount > 0)
            {
                pDependencies = new VkSubpassDependency[create_info->dependencyCount]{};
                for (uint32_t dependencyIndex = 0; dependencyIndex < create_info->dependencyCount; ++dependencyIndex)
                {
                    ReadStructure(pDependencies + dependencyIndex, file[STR_MEMBER_NAME_P_DEPENDENCIES][dependencyIndex]);
                }
            }
            create_info->pDependencies = pDependencies;
        }
    }

    void WriteStructure(const VkAttachmentDescription* create_info, nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            file[STR_MEMBER_NAME_FLAGS] = create_info->flags;
            file[STR_MEMBER_NAME_FORMAT] = create_info->format;
            file[STR_MEMBER_NAME_SAMPLES] = create_info->samples;
            file[STR_MEMBER_NAME_LOAD_OP] = create_info->loadOp;
            file[STR_MEMBER_NAME_STORE_OP] = create_info->storeOp;
            file[STR_MEMBER_NAME_STENCIL_LOAD_OP] = create_info->stencilLoadOp;
            file[STR_MEMBER_NAME_STENCIL_STORE_OP] = create_info->stencilStoreOp;
            file[STR_MEMBER_NAME_INITIAL_LAYOUT] = create_info->initialLayout;
            file[STR_MEMBER_NAME_FINAL_LAYOUT] = create_info->finalLayout;
        }
    }

    void ReadStructure(VkAttachmentDescription* create_info, const nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            create_info->flags = file[STR_MEMBER_NAME_FLAGS];
            create_info->format = file[STR_MEMBER_NAME_FORMAT];
            create_info->samples = file[STR_MEMBER_NAME_SAMPLES];
            create_info->loadOp = file[STR_MEMBER_NAME_LOAD_OP];
            create_info->storeOp = file[STR_MEMBER_NAME_STORE_OP];
            create_info->stencilLoadOp = file[STR_MEMBER_NAME_STENCIL_LOAD_OP];
            create_info->stencilStoreOp = file[STR_MEMBER_NAME_STENCIL_STORE_OP];
            create_info->initialLayout = file[STR_MEMBER_NAME_INITIAL_LAYOUT];
            create_info->finalLayout = file[STR_MEMBER_NAME_FINAL_LAYOUT];
        }
    }

    void WriteStructure(const VkSubpassDescription* create_info, nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            file[STR_MEMBER_NAME_FLAGS] = create_info->flags;
            file[STR_MEMBER_NAME_PIPELINE_BIND_POINT] = create_info->pipelineBindPoint;
            file[STR_MEMBER_NAME_INPUT_ATTACHMENT_COUNT] = create_info->inputAttachmentCount;
            for (uint32_t inputAttachmentIndex = 0; inputAttachmentIndex < create_info->inputAttachmentCount; ++inputAttachmentIndex)
            {
                WriteStructure(create_info->pInputAttachments + inputAttachmentIndex, file[STR_MEMBER_NAME_P_INPUT_ATTACHMENTS][inputAttachmentIndex]);
            }
            file[STR_MEMBER_NAME_COLOR_ATTACHMENT_COUNT] = create_info->colorAttachmentCount;
            for (uint32_t attachmentIndex = 0; attachmentIndex < create_info->colorAttachmentCount; ++attachmentIndex)
            {
                WriteStructure(create_info->pColorAttachments + attachmentIndex, file[STR_MEMBER_NAME_P_COLOR_ATTACHMENTS][attachmentIndex]);
                if (create_info->pResolveAttachments != nullptr)
                {
                    WriteStructure(create_info->pResolveAttachments + attachmentIndex, file[STR_MEMBER_NAME_P_RESOLVE_ATTACHMENTS][attachmentIndex]);
                }
            }

            if (create_info->pDepthStencilAttachment != nullptr)
            {
                WriteStructure(create_info->pDepthStencilAttachment, file[STR_MEMBER_NAME_P_DEPTH_STENCIL_ATTACHMENT]);
            }

            file[STR_MEMBER_NAME_PRESERVE_ATTACHMENT_COUNT] = create_info->preserveAttachmentCount;
            for (uint32_t preserveAttachmentIndex = 0; preserveAttachmentIndex < create_info->preserveAttachmentCount; ++preserveAttachmentIndex)
            {
                file[STR_MEMBER_NAME_P_PRESERVE_ATTACHMENTS][preserveAttachmentIndex] = *(create_info->pPreserveAttachments + preserveAttachmentIndex);
            }
        }
    }

    void ReadStructure(VkSubpassDescription* create_info, const nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            create_info->flags = file[STR_MEMBER_NAME_FLAGS];
            create_info->pipelineBindPoint = file[STR_MEMBER_NAME_PIPELINE_BIND_POINT];
            create_info->inputAttachmentCount = file[STR_MEMBER_NAME_INPUT_ATTACHMENT_COUNT];

            VkAttachmentReference* pInputAttachments = nullptr;
            if (create_info->inputAttachmentCount > 0)
            {
                pInputAttachments = new VkAttachmentReference[create_info->inputAttachmentCount]{};
                for (uint32_t inputAttachmentIndex = 0; inputAttachmentIndex < create_info->inputAttachmentCount; ++inputAttachmentIndex)
                {
                    ReadStructure(pInputAttachments + inputAttachmentIndex, file[STR_MEMBER_NAME_P_INPUT_ATTACHMENTS][inputAttachmentIndex]);
                }
            }
            create_info->pInputAttachments = pInputAttachments;

            create_info->colorAttachmentCount = file[STR_MEMBER_NAME_COLOR_ATTACHMENT_COUNT];
            VkAttachmentReference* pColorAttachments = nullptr;
            VkAttachmentReference* pResolveAttachments = nullptr;
            if (create_info->colorAttachmentCount > 0)
            {
                pColorAttachments = new VkAttachmentReference[create_info->colorAttachmentCount]{};
                for (uint32_t attachmentIndex = 0; attachmentIndex < create_info->colorAttachmentCount; ++attachmentIndex)
                {
                    ReadStructure(pColorAttachments + attachmentIndex, file[STR_MEMBER_NAME_P_COLOR_ATTACHMENTS][attachmentIndex]);
                }

                // Verify that an array of Resolve Attachments exists. If it does, it's the same dimension as the color attachments.
                if (IsCreateInfoExists(file, STR_MEMBER_NAME_P_RESOLVE_ATTACHMENTS))
                {
                    pResolveAttachments = new VkAttachmentReference[create_info->colorAttachmentCount]{};
                    for (uint32_t attachmentIndex = 0; attachmentIndex < create_info->colorAttachmentCount; ++attachmentIndex)
                    {
                        ReadStructure(pResolveAttachments + attachmentIndex, file[STR_MEMBER_NAME_P_RESOLVE_ATTACHMENTS][attachmentIndex]);
                    }
                }
            }
            create_info->pColorAttachments = pColorAttachments;
            create_info->pResolveAttachments = pResolveAttachments;

            VkAttachmentReference* pDepthStencilAttachment = nullptr;
            if (IsCreateInfoExists(file, STR_MEMBER_NAME_P_DEPTH_STENCIL_ATTACHMENT))
            {
                pDepthStencilAttachment = new VkAttachmentReference{};
                ReadStructure(pDepthStencilAttachment, file[STR_MEMBER_NAME_P_DEPTH_STENCIL_ATTACHMENT]);
            }
            create_info->pDepthStencilAttachment = pDepthStencilAttachment;

            create_info->preserveAttachmentCount = file[STR_MEMBER_NAME_PRESERVE_ATTACHMENT_COUNT];
            uint32_t* pPreserveAttachments = nullptr;
            if (create_info->preserveAttachmentCount > 0)
            {
                pPreserveAttachments = new uint32_t[create_info->preserveAttachmentCount]{};
                for (uint32_t preserveAttachmentIndex = 0; preserveAttachmentIndex < create_info->preserveAttachmentCount; ++preserveAttachmentIndex)
                {
                    *(pPreserveAttachments + preserveAttachmentIndex) = file[STR_MEMBER_NAME_P_PRESERVE_ATTACHMENTS][preserveAttachmentIndex];
                }
            }
            create_info->pPreserveAttachments = pPreserveAttachments;
        }
    }

    void WriteStructure(const VkSubpassDependency* create_info, nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            file[STR_MEMBER_NAME_SRC_SUBPASS] = create_info->srcSubpass;
            file[STR_MEMBER_NAME_DST_SUBPASS] = create_info->dstSubpass;
            file[STR_MEMBER_NAME_SRC_STAGE_MASK] = create_info->srcStageMask;
            file[STR_MEMBER_NAME_DST_STAGE_MASK] = create_info->dstStageMask;
            file[STR_MEMBER_NAME_SRC_ACCESS_MASK] = create_info->srcAccessMask;
            file[STR_MEMBER_NAME_DST_ACCESS_MASK] = create_info->dstAccessMask;
            file[STR_MEMBER_NAME_DEPENDENCY_FLAGS] = create_info->dependencyFlags;
        }
    }

    void ReadStructure(VkSubpassDependency* create_info, const nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            create_info->srcSubpass = file[STR_MEMBER_NAME_SRC_SUBPASS];
            create_info->dstSubpass = file[STR_MEMBER_NAME_DST_SUBPASS];
            create_info->srcStageMask = file[STR_MEMBER_NAME_SRC_STAGE_MASK];
            create_info->dstStageMask = file[STR_MEMBER_NAME_DST_STAGE_MASK];
            create_info->srcAccessMask = file[STR_MEMBER_NAME_SRC_ACCESS_MASK];
            create_info->dstAccessMask = file[STR_MEMBER_NAME_DST_ACCESS_MASK];
            create_info->dependencyFlags = file[STR_MEMBER_NAME_DEPENDENCY_FLAGS];
        }
    }

    void WriteStructure(const VkAttachmentReference* create_info, nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            file[STR_MEMBER_NAME_ATTACHMENT] = create_info->attachment;
            file[STR_MEMBER_NAME_LAYOUT] = create_info->layout;
        }
    }

    void ReadStructure(VkAttachmentReference* create_info, const nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            create_info->attachment = file[STR_MEMBER_NAME_ATTACHMENT];
            create_info->layout = file[STR_MEMBER_NAME_LAYOUT];
        }
    }

    void ReadDescriptorSetLayoutCreateInfoArray(rgPsoCreateInfoVulkan* create_info, const nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            // Look for a Descriptor Set Layout create info node. If this node exists, process it as an
            // array of items, because a PSO file can have multiple create info structures saved.
            if (IsCreateInfoExists(file, STR_MEMBER_NAME_VK_DESCRIPTOR_SET_LAYOUT_CREATE_INFO))
            {
                // The Pipeline Layout utilizes Descriptor Set Layout create info array.
                // Find the JSON root element, step through each child element, and read the data into memory.
                const nlohmann::json& descriptorSetLayoutsRoot = file[STR_MEMBER_NAME_VK_DESCRIPTOR_SET_LAYOUT_CREATE_INFO];
                auto firstItem = descriptorSetLayoutsRoot.begin();
                auto lastItem = descriptorSetLayoutsRoot.end();

                // Clear the existing default Descriptor Set Layout create info structures and load
                // from scratch using data loaded from the PSO file.
                std::vector<VkDescriptorSetLayoutCreateInfo*> descriptor_set_layout_collection = create_info->GetDescriptorSetLayoutCreateInfo();
                descriptor_set_layout_collection.clear();

                // Read each individual element in the array of create info.
                for (auto itemIter = firstItem; itemIter != lastItem; ++itemIter)
                {
                    VkDescriptorSetLayoutCreateInfo* pNewDescriptorSetLayout = new VkDescriptorSetLayoutCreateInfo{};
                    assert(pNewDescriptorSetLayout != nullptr);
                    if (pNewDescriptorSetLayout != nullptr)
                    {
                        ReadStructure(pNewDescriptorSetLayout, *itemIter);
                        create_info->AddDescriptorSetLayoutCreateInfo(pNewDescriptorSetLayout);
                    }
                }
            }
        }
    }

    void WriteDescriptorSetLayoutCreateInfoArray(rgPsoCreateInfoVulkan* create_info, nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            int index = 0;
            std::vector<VkDescriptorSetLayoutCreateInfo*> descriptor_set_layout_collection = create_info->GetDescriptorSetLayoutCreateInfo();
            for (VkDescriptorSetLayoutCreateInfo* descriptor_set_layout_create_info : descriptor_set_layout_collection)
            {
                assert(descriptor_set_layout_create_info != nullptr);
                if (descriptor_set_layout_create_info != nullptr)
                {
                    WriteStructure(descriptor_set_layout_create_info, file[STR_MEMBER_NAME_VK_DESCRIPTOR_SET_LAYOUT_CREATE_INFO][index]);
                    index++;
                }
            }
        }
    }

    bool ReadStructure(rgPsoGraphicsVulkan* create_info, const nlohmann::json& file)
    {
        bool is_loaded = false;

        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            // Does the given file's root element match what we expect to see for the project's pipeline type?
            bool isMatchingRootElement = IsCreateInfoExists(file, STR_MEMBER_NAME_VK_GRAPHICS_PIPELINE_CREATE_INFO);
            if (isMatchingRootElement)
            {
                // Deserialize the Graphics Pipeline create info.
                ReadGraphicsPipelineCreateInfoStructure(create_info, file[STR_MEMBER_NAME_VK_GRAPHICS_PIPELINE_CREATE_INFO]);

                // Deserialize the Render Pass create info.
                VkRenderPassCreateInfo* render_pass_create_info = create_info->GetRenderPassCreateInfo();
                assert(render_pass_create_info != nullptr);
                if (render_pass_create_info != nullptr)
                {
                    ReadStructure(render_pass_create_info, file[STR_MEMBER_NAME_VK_RENDER_PASS_CREATE_INFO]);
                }

                // Deserialize the Pipeline Layout create info.
                VkPipelineLayoutCreateInfo* pipeline_layout_create_info = create_info->GetPipelineLayoutCreateInfo();
                assert(pipeline_layout_create_info != nullptr);
                if (pipeline_layout_create_info != nullptr)
                {
                    ReadStructure(pipeline_layout_create_info, file[STR_MEMBER_NAME_VK_PIPELINE_LAYOUT_CREATE_INFO]);
                }

                // Read all Descriptor Set Layout create info structures.
                ReadDescriptorSetLayoutCreateInfoArray(create_info, file);

                // If all data was deserialized successfully, the PSO file is loaded.
                is_loaded = true;
            }
        }

        return is_loaded;
    }

    bool ReadStructure(rgPsoComputeVulkan* create_info, const nlohmann::json& file)
    {
        bool is_loaded = false;

        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            // Does the given file's root element match what we expect to see for the project's pipeline type?
            bool isMatchingRootElement = IsCreateInfoExists(file, STR_MEMBER_NAME_VK_COMPUTE_PIPELINE_CREATE_INFO);
            if (isMatchingRootElement)
            {
                VkComputePipelineCreateInfo* compute_pipeline_create_info = create_info->GetComputePipelineCreateInfo();
                assert(compute_pipeline_create_info != nullptr);
                if (compute_pipeline_create_info != nullptr)
                {
                    ReadStructure(compute_pipeline_create_info, file[STR_MEMBER_NAME_VK_COMPUTE_PIPELINE_CREATE_INFO]);
                }

                VkPipelineLayoutCreateInfo* pipeline_layout_create_info = create_info->GetPipelineLayoutCreateInfo();
                assert(pipeline_layout_create_info != nullptr);
                if (pipeline_layout_create_info != nullptr)
                {
                    ReadStructure(pipeline_layout_create_info, file[STR_MEMBER_NAME_VK_PIPELINE_LAYOUT_CREATE_INFO]);
                }

                // Read all Descriptor Set Layout create info structures.
                ReadDescriptorSetLayoutCreateInfoArray(create_info, file);

                // If all data was deserialized successfully, the PSO file is loaded.
                is_loaded = true;
            }
        }

        return is_loaded;
    }

    void WriteStructure(rgPsoGraphicsVulkan* create_info, nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            VkGraphicsPipelineCreateInfo* pGraphicsPsoCreateInfo = create_info->GetGraphicsPipelineCreateInfo();
            assert(pGraphicsPsoCreateInfo != nullptr);
            if (pGraphicsPsoCreateInfo != nullptr)
            {
                WriteStructure(pGraphicsPsoCreateInfo, file[STR_MEMBER_NAME_VK_GRAPHICS_PIPELINE_CREATE_INFO]);
            }

            VkRenderPassCreateInfo* render_pass_create_info = create_info->GetRenderPassCreateInfo();
            assert(render_pass_create_info != nullptr);
            if (render_pass_create_info != nullptr)
            {
                WriteStructure(render_pass_create_info, file[STR_MEMBER_NAME_VK_RENDER_PASS_CREATE_INFO]);
            }

            VkPipelineLayoutCreateInfo* pipeline_layout_create_info = create_info->GetPipelineLayoutCreateInfo();
            assert(pipeline_layout_create_info != nullptr);
            if (pipeline_layout_create_info != nullptr)
            {
                WriteStructure(pipeline_layout_create_info, file[STR_MEMBER_NAME_VK_PIPELINE_LAYOUT_CREATE_INFO]);
            }

            // Write the array of Descriptor Set Layout create info structures.
            WriteDescriptorSetLayoutCreateInfoArray(create_info, file);
        }
    }

    void WriteStructure(rgPsoComputeVulkan* create_info, nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            VkComputePipelineCreateInfo* pComputePsoCreateInfo = create_info->GetComputePipelineCreateInfo();
            assert(pComputePsoCreateInfo != nullptr);
            if (pComputePsoCreateInfo != nullptr)
            {
                WriteStructure(pComputePsoCreateInfo, file[STR_MEMBER_NAME_VK_COMPUTE_PIPELINE_CREATE_INFO]);
            }

            VkPipelineLayoutCreateInfo* pipeline_layout_create_info = create_info->GetPipelineLayoutCreateInfo();
            assert(pipeline_layout_create_info != nullptr);
            if (pipeline_layout_create_info != nullptr)
            {
                WriteStructure(pipeline_layout_create_info, file[STR_MEMBER_NAME_VK_PIPELINE_LAYOUT_CREATE_INFO]);
            }

            // Write the array of Descriptor Set Layout create info structures.
            WriteDescriptorSetLayoutCreateInfoArray(create_info, file);
        }
    }

    virtual VkSampleMask* ReadSampleMask(const VkPipelineMultisampleStateCreateInfo* create_info, const nlohmann::json& file)
    {
        VkSampleMask* pSampleMask = nullptr;
        if (IsCreateInfoExists(file, STR_MEMBER_NAME_P_SAMPLE_MASK))
        {
            pSampleMask = new VkSampleMask[(uint32_t)create_info->rasterizationSamples];
            for (uint32_t index = 0; index < (uint32_t)create_info->rasterizationSamples; ++index)
            {
                pSampleMask[index] = file[STR_MEMBER_NAME_P_SAMPLE_MASK][index];
            }
        }
        return pSampleMask;
    }

    virtual void WriteSampleMask(const VkPipelineMultisampleStateCreateInfo* create_info, nlohmann::json& file)
    {
        if (create_info->pSampleMask != nullptr)
        {
            for (uint32_t index = 0; index < (uint32_t)create_info->rasterizationSamples; ++index)
            {
                file[STR_MEMBER_NAME_P_SAMPLE_MASK][index] = *(create_info->pSampleMask + index);
            }
        }
    }
};

// This subclass is a version of the pipeline serializer that fixes
// serializing the VkStencilOpState structure's missing "failOp" member.
class rgPsoSerializerVulkanImpl_Version_1_1 : public rgPsoSerializerVulkanImpl_Version_1_0
{
public:
    rgPsoSerializerVulkanImpl_Version_1_1() = default;
    virtual ~rgPsoSerializerVulkanImpl_Version_1_1() = default;

    virtual void WriteStructure(const VkStencilOpState* create_info, nlohmann::json& file) override
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            file[STR_MEMBER_NAME_FAIL_OP] = create_info->failOp;
            file[STR_MEMBER_NAME_PASS_OP] = create_info->passOp;
            file[STR_MEMBER_NAME_DEPTH_FAIL_OP] = create_info->depthFailOp;
            file[STR_MEMBER_NAME_COMPARE_OP] = create_info->compareOp;
            file[STR_MEMBER_NAME_COMPARE_MASK] = create_info->compareMask;
            file[STR_MEMBER_NAME_WRITE_MASK] = create_info->writeMask;
            file[STR_MEMBER_NAME_REFERENCE] = create_info->reference;
        }
    }

    virtual void ReadStructure(VkStencilOpState* create_info, const nlohmann::json& file) override
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            create_info->failOp = file[STR_MEMBER_NAME_FAIL_OP];
            create_info->passOp = file[STR_MEMBER_NAME_PASS_OP];
            create_info->depthFailOp = file[STR_MEMBER_NAME_DEPTH_FAIL_OP];
            create_info->compareOp = file[STR_MEMBER_NAME_COMPARE_OP];
            create_info->compareMask = file[STR_MEMBER_NAME_COMPARE_MASK];
            create_info->writeMask = file[STR_MEMBER_NAME_WRITE_MASK];
            create_info->reference = file[STR_MEMBER_NAME_REFERENCE];
        }
    }
};

// This subclass is a version of the pipeline serializer that fixes
// serializing the VkSampleMask within a graphics pipeline's multisampling state.
class rgPsoSerializerVulkanImpl_Version_1_2 : public rgPsoSerializerVulkanImpl_Version_1_1
{
public:
    rgPsoSerializerVulkanImpl_Version_1_2() = default;
    virtual ~rgPsoSerializerVulkanImpl_Version_1_2() = default;

    virtual VkSampleMask* ReadSampleMask(const VkPipelineMultisampleStateCreateInfo* create_info, const nlohmann::json& file) override
    {
        VkSampleMask* pSampleMask = nullptr;
        if (IsCreateInfoExists(file, STR_MEMBER_NAME_P_SAMPLE_MASK))
        {
            // What's the required dimension of the pSampleMask bitfield array?
            int sampleMaskArrayDimension = 0;
            switch (create_info->rasterizationSamples)
            {
            case VK_SAMPLE_COUNT_1_BIT:
            case VK_SAMPLE_COUNT_2_BIT:
            case VK_SAMPLE_COUNT_4_BIT:
            case VK_SAMPLE_COUNT_8_BIT:
            case VK_SAMPLE_COUNT_16_BIT:
            case VK_SAMPLE_COUNT_32_BIT:
                // A single 32-bit value is all that's needed to hold the
                // pSampleMask bitfield when it's between 1 and 32-bits in size.
                sampleMaskArrayDimension = 1;
                break;
            case VK_SAMPLE_COUNT_64_BIT:
                // Two 32-bit values are needed to hold the 64-bit pSampleMask bitfield.
                sampleMaskArrayDimension = 2;
                break;
            default:
                // The RasterizationSamples value was unrecognized, and we can't proceed.
                assert(false);
            }

            pSampleMask = new VkSampleMask[sampleMaskArrayDimension]{};
            for (int index = 0; index < sampleMaskArrayDimension; ++index)
            {
                pSampleMask[index] = file[STR_MEMBER_NAME_P_SAMPLE_MASK][index];
            }
        }
        return pSampleMask;
    }

    virtual void WriteSampleMask(const VkPipelineMultisampleStateCreateInfo* create_info, nlohmann::json& file) override
    {
        // It's fine if pSampleMask is null- just don't write the field.
        if (create_info->pSampleMask != nullptr)
        {
            // What's the required dimension of the pSampleMask bitfield array?
            uint32_t sampleMaskArrayDimension = 0;
            switch (create_info->rasterizationSamples)
            {
            case VK_SAMPLE_COUNT_1_BIT:
            case VK_SAMPLE_COUNT_2_BIT:
            case VK_SAMPLE_COUNT_4_BIT:
            case VK_SAMPLE_COUNT_8_BIT:
            case VK_SAMPLE_COUNT_16_BIT:
            case VK_SAMPLE_COUNT_32_BIT:
                // A single 32-bit value is all that's needed to hold the
                // pSampleMask bitfield when it's between 1 and 32-bits in size.
                sampleMaskArrayDimension = 1;
                break;
            case VK_SAMPLE_COUNT_64_BIT:
                // Two 32-bit values are needed to hold the 64-bit pSampleMask bitfield.
                sampleMaskArrayDimension = 2;
                break;
            default:
                // The RasterizationSamples value was unrecognized, and we can't proceed.
                assert(false);
            }

            // Serialize the array of bitfields. It'll either be 1 or 2 32-bit values.
            for (uint32_t index = 0; index < sampleMaskArrayDimension; ++index)
            {
                file[STR_MEMBER_NAME_P_SAMPLE_MASK][index] = *(create_info->pSampleMask + index);
            }
        }
    }
};

std::shared_ptr<rgPsoSerializerVulkanImpl_Version_1_0> CreateSerializer(rgPipelineModelVersion serializerVersion)
{
    std::shared_ptr<rgPsoSerializerVulkanImpl_Version_1_0> pSerializer = nullptr;

    switch (serializerVersion)
    {
    case rgPipelineModelVersion::VERSION_1_0:
        pSerializer = std::make_shared<rgPsoSerializerVulkanImpl_Version_1_0>();
        break;
    case rgPipelineModelVersion::VERSION_1_1:
        pSerializer = std::make_shared<rgPsoSerializerVulkanImpl_Version_1_1>();
        break;
    case rgPipelineModelVersion::VERSION_1_2:
        pSerializer = std::make_shared<rgPsoSerializerVulkanImpl_Version_1_2>();
        break;
    default:
        // If we get here, there is no matching serializer type for the incoming version.
        assert(false);
    }

    return pSerializer;
}

bool RgPsoSerializerVulkan::ReadStructureFromFile(const std::string& file_path, rgPsoGraphicsVulkan** create_info_array, std::string& error_string)
{
    bool ret = false;
    bool should_abort = false;

    assert(create_info_array != nullptr);
    if (create_info_array != nullptr)
    {
        // Create a new PSO State structure.
        rgPsoGraphicsVulkan* create_info = new rgPsoGraphicsVulkan{};

        // Initialize the create info to assign structure pointers to internal create info members.
        create_info->Initialize();

        // Open a file to write the structure data to.
        std::ifstream file_stream;
        file_stream.open(file_path.c_str(), std::ofstream::in);

        assert(file_stream.is_open());
        if (file_stream.is_open())
        {
            // Read the JSON file.
            nlohmann::json structure;
            should_abort = !ReadJsonFile(file_stream, file_path, structure, error_string);

            // Close the file stream.
            file_stream.close();

            if (!should_abort)
            {
                // Is there a pipeline model version tag? Extract the version number if possible.
                rgPipelineModelVersion modelVersion = rgPipelineModelVersion::Unknown;
                if (IsCreateInfoExists(structure, STR_PIPELINE_MODEL_VERSION))
                {
                    modelVersion = static_cast<rgPipelineModelVersion>(structure[STR_PIPELINE_MODEL_VERSION].get<int>());
                }
                else
                {
                    // Versioning doesn't exist in the initial revision of the pipeline state file.
                    // When the model version tag isn't found, assume VERSION_1_0.
                    modelVersion = rgPipelineModelVersion::VERSION_1_0;
                }

                assert(modelVersion != rgPipelineModelVersion::Unknown);
                if (modelVersion != rgPipelineModelVersion::Unknown)
                {
                    // Always write the most recent version of the pipeline state file.
                    std::shared_ptr<rgPsoSerializerVulkanImpl_Version_1_0> pSerializer = CreateSerializer(modelVersion);
                    assert(pSerializer != nullptr);
                    if (pSerializer != nullptr)
                    {
                        // Read the structure data from the JSON file.
                        if (pSerializer->ReadStructure(create_info, structure))
                        {
                            // Assign the deserialized pipeline state file to the output pointer.
                            *create_info_array = create_info;

                            ret = true;
                        }
                        else
                        {
                            error_string = STR_ERR_FAILED_TO_LOAD_PIPELINE_TYPE_MISMATCH;
                        }
                    }
                    else
                    {
                        error_string = STR_ERR_FAILED_UNSUPPORTED_VERSION;
                    }
                }
                else
                {
                    error_string = STR_ERR_FAILED_TO_READ_PIPELINE_VERSION;
                }
            }
        }
        else
        {
            std::stringstream error_stream;
            error_stream << STR_ERR_FAILED_TO_READ_FILE;
            error_stream << file_path;
            error_string = error_stream.str();
        }
    }

    return ret;
}

bool RgPsoSerializerVulkan::ReadStructureFromFile(const std::string& file_path, rgPsoComputeVulkan** create_info_array, std::string& error_string)
{
    bool ret = false;
    bool should_abort = false;

    assert(create_info_array != nullptr);
    if (create_info_array != nullptr)
    {
        // Create a new PSO State structure.
        rgPsoComputeVulkan* create_info = new rgPsoComputeVulkan{};

        // Initialize the create info to assign structure pointers to internal create info members.
        create_info->Initialize();

        // Open a file to write the structure data to.
        std::ifstream file_stream;
        file_stream.open(file_path.c_str(), std::ofstream::in);

        assert(file_stream.is_open());
        if (file_stream.is_open())
        {
            // Read the JSON file.
            nlohmann::json structure;
            should_abort = !ReadJsonFile(file_stream, file_path, structure, error_string);

            if (!should_abort)
            {
                // Is there a pipeline model version tag? Extract the version number if possible.
                rgPipelineModelVersion modelVersion = rgPipelineModelVersion::Unknown;
                if (IsCreateInfoExists(structure, STR_PIPELINE_MODEL_VERSION))
                {
                    modelVersion = static_cast<rgPipelineModelVersion>(structure[STR_PIPELINE_MODEL_VERSION].get<int>());
                }
                else
                {
                    // Versioning doesn't exist in the initial revision of the pipeline state file.
                    // When the model version tag isn't found, assume VERSION_1_0.
                    modelVersion = rgPipelineModelVersion::VERSION_1_0;
                }

                assert(modelVersion != rgPipelineModelVersion::Unknown);
                if (modelVersion != rgPipelineModelVersion::Unknown)
                {
                    // Always write the most recent version of the pipeline state file.
                    std::shared_ptr<rgPsoSerializerVulkanImpl_Version_1_0> pSerializer = CreateSerializer(modelVersion);
                    assert(pSerializer != nullptr);
                    if (pSerializer != nullptr)
                    {
                        if (pSerializer->ReadStructure(create_info, structure))
                        {
                            // Assign the deserialized pipeline state file to the output pointer.
                            *create_info_array = create_info;
                            ret = true;
                        }
                        else
                        {
                            error_string = STR_ERR_FAILED_TO_LOAD_PIPELINE_TYPE_MISMATCH;
                        }
                    }
                    else
                    {
                        error_string = STR_ERR_FAILED_UNSUPPORTED_VERSION;
                    }
                }
                else
                {
                    error_string = STR_ERR_FAILED_TO_READ_PIPELINE_VERSION;
                }
            }
        }
        else
        {
            std::stringstream error_stream;
            error_stream << STR_ERR_FAILED_TO_READ_FILE;
            error_stream << file_path;
            error_string = error_stream.str();
        }
    }

    return ret;
}

bool RgPsoSerializerVulkan::WriteStructureToFile(rgPsoGraphicsVulkan* create_info, const std::string& file_path, std::string& error_string)
{
    bool ret = false;

    // Open a file to write the structure data to.
    std::ofstream file_stream;
    file_stream.open(file_path.c_str(), std::ofstream::out);

    if (file_stream.is_open())
    {
        nlohmann::json jsonFile;

        // Write the current pipeline version into the file.
        jsonFile[STR_PIPELINE_MODEL_VERSION] = s_CURRENT_PIPELINE_VERSION;

        // Always write the most recent version of the pipeline state file.
        std::shared_ptr<rgPsoSerializerVulkanImpl_Version_1_0> pSerializer = CreateSerializer(s_CURRENT_PIPELINE_VERSION);
        assert(pSerializer != nullptr);
        if (pSerializer != nullptr)
        {
            pSerializer->WriteStructure(create_info, jsonFile);

            // Write the JSON to disk and close the file with the given indentation.
            file_stream << jsonFile.dump(4);
            file_stream.close();

            ret = true;
        }
        else
        {
            error_string = STR_ERR_FAILED_UNSUPPORTED_VERSION;
        }
    }
    else
    {
        std::stringstream error_stream;
        error_stream << STR_ERR_FAILED_TO_WRITE_FILE;
        error_stream << file_path.c_str();
        error_string = error_stream.str();
    }

    return ret;
}

bool RgPsoSerializerVulkan::WriteStructureToFile(rgPsoComputeVulkan* create_info, const std::string& file_path, std::string& error_string)
{
    bool ret = false;

    // Open a file to write the structure data to.
    std::ofstream file_stream;
    file_stream.open(file_path.c_str(), std::ofstream::out);

    if (file_stream.is_open())
    {
        nlohmann::json jsonFile;

        // Write the current pipeline version into the file.
        jsonFile[STR_PIPELINE_MODEL_VERSION] = s_CURRENT_PIPELINE_VERSION;

        // Always write the most recent version of the pipeline state file.
        std::shared_ptr<rgPsoSerializerVulkanImpl_Version_1_0> pSerializer = CreateSerializer(s_CURRENT_PIPELINE_VERSION);
        assert(pSerializer != nullptr);
        if (pSerializer != nullptr)
        {
            pSerializer->WriteStructure(create_info, jsonFile);

            // Write the JSON to disk and close the file with the given indentation.
            file_stream << jsonFile.dump(4);
            file_stream.close();

            ret = true;
        }
        else
        {
            error_string = STR_ERR_FAILED_UNSUPPORTED_VERSION;
        }
    }
    else
    {
        std::stringstream error_stream;
        error_stream << STR_ERR_FAILED_TO_WRITE_FILE;
        error_stream << file_path.c_str();
        error_string = error_stream.str();
    }

    return ret;
}
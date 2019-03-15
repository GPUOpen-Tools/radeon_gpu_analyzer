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

// Helper function that reads the JSON file from fileStream into structure.
// The function sets returns false if any failure happened (otherwise true is returned).
// The function stores the relevant error message in errorString.
static bool ReadJsonFile(std::ifstream& fileStream, const std::string& filePath, nlohmann::json& structure, std::string &errorString)
{
    bool ret = true;

    // Read the file into the JSON structure.
    try
    {
        // Try to read the file.
        fileStream >> structure;
    }
    catch (...)
    {
        // Failed reading.
        ret = false;

        // Inform the caller.
        std::stringstream errorStream;
        errorStream << STR_ERR_FAILED_TO_READ_FILE;
        errorStream << filePath;
        errorString = errorStream.str();
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

    void ReadGraphicsPipelineCreateInfoStructure(rgPsoGraphicsVulkan* pCreateInfo, const nlohmann::json& file)
    {
        assert(pCreateInfo != nullptr);
        if (pCreateInfo != nullptr)
        {
            VkGraphicsPipelineCreateInfo* pGraphicsPipelineCreateInfo = pCreateInfo->GetGraphicsPipelineCreateInfo();
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
                    for (uint32_t stageIndex = 0; stageIndex < pGraphicsPipelineCreateInfo->stageCount; ++stageIndex)
                    {
                        ReadStructure(pShaderStages + stageIndex, file[STR_MEMBER_NAME_PSTAGES][stageIndex]);
                    }
                }
                pGraphicsPipelineCreateInfo->pStages = pShaderStages;

                VkPipelineVertexInputStateCreateInfo* pVertexInputState = nullptr;
                if (IsCreateInfoExists(file, STR_MEMBER_NAME_P_VERTEX_INPUT_STATE))
                {
                    pVertexInputState = pCreateInfo->GetPipelineVertexInputStateCreateInfo();
                    ReadStructure(pVertexInputState, file[STR_MEMBER_NAME_P_VERTEX_INPUT_STATE]);
                }

                VkPipelineInputAssemblyStateCreateInfo* pVertexInputAssemblyState = nullptr;
                if (IsCreateInfoExists(file, STR_MEMBER_NAME_P_INPUT_ASSEMBLY_STATE))
                {
                    pVertexInputAssemblyState = pCreateInfo->GetPipelineInputAssemblyStateCreateInfo();
                    ReadStructure(pVertexInputAssemblyState, file[STR_MEMBER_NAME_P_INPUT_ASSEMBLY_STATE]);
                }

                VkPipelineTessellationStateCreateInfo* pTessellationState = nullptr;
                if (IsCreateInfoExists(file, STR_MEMBER_NAME_P_TESSELLATION_STATE))
                {
                    pTessellationState = pCreateInfo->GetipelineTessellationStateCreateInfo();
                    ReadStructure(pTessellationState, file[STR_MEMBER_NAME_P_TESSELLATION_STATE]);
                }

                VkPipelineViewportStateCreateInfo* pViewportState = nullptr;
                if (IsCreateInfoExists(file, STR_MEMBER_NAME_P_VIEWPORT_STATE))
                {
                    pViewportState = pCreateInfo->GetPipelineViewportStateCreateInfo();
                    ReadStructure(pViewportState, file[STR_MEMBER_NAME_P_VIEWPORT_STATE]);
                }

                VkPipelineRasterizationStateCreateInfo* pRasterizationState = nullptr;
                if (IsCreateInfoExists(file, STR_MEMBER_NAME_P_RASTERIZATION_STATE))
                {
                    pRasterizationState = pCreateInfo->GetPipelineRasterizationStateCreateInfo();
                    ReadStructure(pRasterizationState, file[STR_MEMBER_NAME_P_RASTERIZATION_STATE]);
                }

                VkPipelineMultisampleStateCreateInfo* pMultisampleState = nullptr;
                if (IsCreateInfoExists(file, STR_MEMBER_NAME_P_MULTISAMPLE_STATE))
                {
                    pMultisampleState = pCreateInfo->GetPipelineMultisampleStateCreateInfo();
                    ReadStructure(pMultisampleState, file[STR_MEMBER_NAME_P_MULTISAMPLE_STATE]);
                }

                VkPipelineDepthStencilStateCreateInfo* pDepthStencilState = nullptr;
                if (IsCreateInfoExists(file, STR_MEMBER_NAME_P_DEPTH_STENCIL_STATE))
                {
                    pDepthStencilState = pCreateInfo->GetPipelineDepthStencilStateCreateInfo();
                    ReadStructure(pDepthStencilState, file[STR_MEMBER_NAME_P_DEPTH_STENCIL_STATE]);
                }

                VkPipelineColorBlendStateCreateInfo* pColorBlendState = nullptr;
                if (IsCreateInfoExists(file, STR_MEMBER_NAME_P_COLOR_BLEND_STATE))
                {
                    pColorBlendState = pCreateInfo->GetPipelineColorBlendStateCreateInfo();
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

    void WriteStructure(const VkGraphicsPipelineCreateInfo* pCreateInfo, nlohmann::json& file)
    {
        assert(pCreateInfo != nullptr);
        if (pCreateInfo != nullptr)
        {
            file[STR_MEMBER_NAME_TYPE] = pCreateInfo->sType;
            file[STR_MEMBER_NAME_PNEXT] = WriteHandle(reinterpret_cast<uint64_t>(pCreateInfo->pNext));
            file[STR_MEMBER_NAME_FLAGS] = pCreateInfo->flags;
            file[STR_MEMBER_NAME_STAGE_COUNT] = pCreateInfo->stageCount;

            for (uint32_t index = 0; index < pCreateInfo->stageCount; ++index)
            {
                WriteStructure((pCreateInfo->pStages + index), file[STR_MEMBER_NAME_PSTAGES][index]);
            }

            if (pCreateInfo->pVertexInputState != nullptr)
            {
                WriteStructure(pCreateInfo->pVertexInputState, file[STR_MEMBER_NAME_P_VERTEX_INPUT_STATE]);
            }

            if (pCreateInfo->pInputAssemblyState != nullptr)
            {
                WriteStructure(pCreateInfo->pInputAssemblyState, file[STR_MEMBER_NAME_P_INPUT_ASSEMBLY_STATE]);
            }

            if (pCreateInfo->pTessellationState != nullptr)
            {
                WriteStructure(pCreateInfo->pTessellationState, file[STR_MEMBER_NAME_P_TESSELLATION_STATE]);
            }

            if (pCreateInfo->pViewportState != nullptr)
            {
                WriteStructure(pCreateInfo->pViewportState, file[STR_MEMBER_NAME_P_VIEWPORT_STATE]);
            }

            if (pCreateInfo->pRasterizationState != nullptr)
            {
                WriteStructure(pCreateInfo->pRasterizationState, file[STR_MEMBER_NAME_P_RASTERIZATION_STATE]);
            }

            if (pCreateInfo->pMultisampleState != nullptr)
            {
                WriteStructure(pCreateInfo->pMultisampleState, file[STR_MEMBER_NAME_P_MULTISAMPLE_STATE]);
            }

            if (pCreateInfo->pDepthStencilState != nullptr)
            {
                WriteStructure(pCreateInfo->pDepthStencilState, file[STR_MEMBER_NAME_P_DEPTH_STENCIL_STATE]);
            }

            if (pCreateInfo->pColorBlendState != nullptr)
            {
                WriteStructure(pCreateInfo->pColorBlendState, file[STR_MEMBER_NAME_P_COLOR_BLEND_STATE]);
            }

            if (pCreateInfo->pDynamicState != nullptr)
            {
                WriteStructure(pCreateInfo->pDynamicState, file[STR_MEMBER_NAME_P_DYNAMIC_STATE]);
            }

            if (pCreateInfo->layout != VK_NULL_HANDLE)
            {
                file[STR_MEMBER_NAME_LAYOUT] = WriteHandle(reinterpret_cast<uint64_t>(pCreateInfo->layout));
            }

            if (pCreateInfo->renderPass != VK_NULL_HANDLE)
            {
                file[STR_MEMBER_NAME_RENDER_PASS] = WriteHandle(reinterpret_cast<uint64_t>(pCreateInfo->renderPass));
            }
            file[STR_MEMBER_NAME_SUBPASS] = pCreateInfo->subpass;
            if (pCreateInfo->basePipelineHandle != VK_NULL_HANDLE)
            {
                file[STR_MEMBER_NAME_BASE_PIPELINE_HANDLE] = WriteHandle(reinterpret_cast<uint64_t>(pCreateInfo->basePipelineHandle));
            }
            file[STR_MEMBER_NAME_BASE_PIPELINE_INDEX] = pCreateInfo->basePipelineIndex;
        }
    }

    void WriteStructure(const VkPipelineShaderStageCreateInfo* pCreateInfo, nlohmann::json& file)
    {
        assert(pCreateInfo != nullptr);
        if (pCreateInfo != nullptr)
        {
            file[STR_MEMBER_NAME_TYPE] = pCreateInfo->sType;
            file[STR_MEMBER_NAME_PNEXT] = WriteHandle(reinterpret_cast<uint64_t>(pCreateInfo->pNext));
            file[STR_MEMBER_NAME_FLAGS] = pCreateInfo->flags;
            file[STR_MEMBER_NAME_STAGE] = pCreateInfo->stage;
            file[STR_MEMBER_NAME_MODULE] = WriteHandle(reinterpret_cast<uint64_t>(pCreateInfo->module));

            if (pCreateInfo->pName != nullptr)
            {
                file[STR_MEMBER_NAME_NAME] = std::string(pCreateInfo->pName);
            }

            if (pCreateInfo->pSpecializationInfo != nullptr)
            {
                WriteStructure(pCreateInfo->pSpecializationInfo, file[STR_MEMBER_NAME_P_SPECIALIZATION_INFO]);
            }
        }
    }

    void ReadStructure(VkPipelineShaderStageCreateInfo* pCreateInfo, const nlohmann::json& file)
    {
        assert(pCreateInfo != nullptr);
        if (pCreateInfo != nullptr)
        {
            pCreateInfo->sType = file[STR_MEMBER_NAME_TYPE];
            pCreateInfo->pNext = ReadHandle(file[STR_MEMBER_NAME_PNEXT]);
            pCreateInfo->flags = file[STR_MEMBER_NAME_FLAGS];
            pCreateInfo->stage = file[STR_MEMBER_NAME_STAGE];
            pCreateInfo->module = (VkShaderModule)ReadHandle(file[STR_MEMBER_NAME_MODULE]);
            if (IsCreateInfoExists(file, STR_MEMBER_NAME_NAME))
            {
                std::string entrypointName = file[STR_MEMBER_NAME_NAME];
                size_t bufferSize = entrypointName.length() + 1;
                char* pEntrypointName = new char[bufferSize] {};
                STRCPY(pEntrypointName, bufferSize, entrypointName.c_str());
                pCreateInfo->pName = pEntrypointName;
            }

            if (IsCreateInfoExists(file, STR_MEMBER_NAME_P_SPECIALIZATION_INFO))
            {
                VkSpecializationInfo* pInfo = new VkSpecializationInfo{};
                ReadStructure(pInfo, file[STR_MEMBER_NAME_P_SPECIALIZATION_INFO]);
                pCreateInfo->pSpecializationInfo = pInfo;
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

    void WriteStructure(const VkPipelineVertexInputStateCreateInfo* pCreateInfo, nlohmann::json& file)
    {
        assert(pCreateInfo != nullptr);
        if (pCreateInfo != nullptr)
        {
            file[STR_MEMBER_NAME_TYPE] = pCreateInfo->sType;
            file[STR_MEMBER_NAME_PNEXT] = WriteHandle(reinterpret_cast<uint64_t>(pCreateInfo->pNext));
            file[STR_MEMBER_NAME_FLAGS] = pCreateInfo->flags;
            file[STR_MEMBER_NAME_VERTEX_BINDING_DESCRIPTION_COUNT] = pCreateInfo->vertexBindingDescriptionCount;
            file[STR_MEMBER_NAME_VERTEX_ATTRIBUTE_DESCRIPTION_COUNT] = pCreateInfo->vertexAttributeDescriptionCount;
            for (uint32_t index = 0; index < pCreateInfo->vertexBindingDescriptionCount; ++index)
            {
                WriteStructure(pCreateInfo->pVertexBindingDescriptions + index, file[STR_MEMBER_NAME_P_VERTEX_BINDING_DESCRIPTIONS][index]);
            }
            for (uint32_t index = 0; index < pCreateInfo->vertexAttributeDescriptionCount; ++index)
            {
                WriteStructure(pCreateInfo->pVertexAttributeDescriptions + index, file[STR_MEMBER_NAME_P_VERTEX_ATTRIBUTE_DESCRIPTIONS][index]);
            }
        }
    }

    void ReadStructure(VkPipelineVertexInputStateCreateInfo* pCreateInfo, const nlohmann::json& file)
    {
        assert(pCreateInfo != nullptr);
        if (pCreateInfo != nullptr)
        {
            pCreateInfo->sType = file[STR_MEMBER_NAME_TYPE];
            pCreateInfo->pNext = ReadHandle(file[STR_MEMBER_NAME_PNEXT]);
            pCreateInfo->flags = file[STR_MEMBER_NAME_FLAGS];
            pCreateInfo->vertexBindingDescriptionCount = file[STR_MEMBER_NAME_VERTEX_BINDING_DESCRIPTION_COUNT];

            VkVertexInputBindingDescription* pVertexBindingDescriptions = nullptr;
            if (pCreateInfo->vertexBindingDescriptionCount > 0)
            {
                pVertexBindingDescriptions = new VkVertexInputBindingDescription[pCreateInfo->vertexBindingDescriptionCount]{};
                for (uint32_t bindingIndex = 0; bindingIndex < pCreateInfo->vertexBindingDescriptionCount; ++bindingIndex)
                {
                    ReadStructure(pVertexBindingDescriptions + bindingIndex, file[STR_MEMBER_NAME_P_VERTEX_BINDING_DESCRIPTIONS][bindingIndex]);
                }
            }
            pCreateInfo->pVertexBindingDescriptions = pVertexBindingDescriptions;

            pCreateInfo->vertexAttributeDescriptionCount = file[STR_MEMBER_NAME_VERTEX_ATTRIBUTE_DESCRIPTION_COUNT];

            VkVertexInputAttributeDescription* pVertexAttributeDescriptions = nullptr;
            if (pCreateInfo->vertexAttributeDescriptionCount > 0)
            {
                pVertexAttributeDescriptions = new VkVertexInputAttributeDescription[pCreateInfo->vertexAttributeDescriptionCount]{};
                for (uint32_t attributeIndex = 0; attributeIndex < pCreateInfo->vertexAttributeDescriptionCount; ++attributeIndex)
                {
                    ReadStructure(pVertexAttributeDescriptions + attributeIndex, file[STR_MEMBER_NAME_P_VERTEX_ATTRIBUTE_DESCRIPTIONS][attributeIndex]);
                }
            }
            pCreateInfo->pVertexAttributeDescriptions = pVertexAttributeDescriptions;
        }
    }

    void WriteStructure(const VkPipelineInputAssemblyStateCreateInfo* pCreateInfo, nlohmann::json& file)
    {
        assert(pCreateInfo != nullptr);
        if (pCreateInfo != nullptr)
        {
            file[STR_MEMBER_NAME_TYPE] = pCreateInfo->sType;
            file[STR_MEMBER_NAME_PNEXT] = WriteHandle(reinterpret_cast<uint64_t>(pCreateInfo->pNext));
            file[STR_MEMBER_NAME_FLAGS] = pCreateInfo->flags;
            file[STR_MEMBER_NAME_TOPOLOGY] = pCreateInfo->topology;
            file[STR_MEMBER_NAME_PRIMITIVE_RESTART_ENABLED] = WriteBool(pCreateInfo->primitiveRestartEnable);
        }
    }

    void ReadStructure(VkPipelineInputAssemblyStateCreateInfo* pCreateInfo, const nlohmann::json& file)
    {
        assert(pCreateInfo != nullptr);
        if (pCreateInfo != nullptr)
        {
            pCreateInfo->sType = file[STR_MEMBER_NAME_TYPE];
            pCreateInfo->pNext = ReadHandle(file[STR_MEMBER_NAME_PNEXT]);
            pCreateInfo->flags = file[STR_MEMBER_NAME_FLAGS];
            pCreateInfo->topology = file[STR_MEMBER_NAME_TOPOLOGY];
            pCreateInfo->primitiveRestartEnable = ReadBool(file[STR_MEMBER_NAME_PRIMITIVE_RESTART_ENABLED]);
        }
    }

    void WriteStructure(const VkPipelineTessellationStateCreateInfo* pCreateInfo, nlohmann::json& file)
    {
        assert(pCreateInfo != nullptr);
        if (pCreateInfo != nullptr)
        {
            file[STR_MEMBER_NAME_TYPE] = pCreateInfo->sType;
            file[STR_MEMBER_NAME_PNEXT] = WriteHandle(reinterpret_cast<uint64_t>(pCreateInfo->pNext));
            file[STR_MEMBER_NAME_FLAGS] = pCreateInfo->flags;
            file[STR_MEMBER_NAME_PATCH_CONTROL_POINTS] = pCreateInfo->patchControlPoints;
        }
    }

    void ReadStructure(VkPipelineTessellationStateCreateInfo* pCreateInfo, const nlohmann::json& file)
    {
        assert(pCreateInfo != nullptr);
        if (pCreateInfo != nullptr)
        {
            pCreateInfo->sType = file[STR_MEMBER_NAME_TYPE];
            pCreateInfo->pNext = ReadHandle(file[STR_MEMBER_NAME_PNEXT]);
            pCreateInfo->flags = file[STR_MEMBER_NAME_FLAGS];
            pCreateInfo->patchControlPoints = file[STR_MEMBER_NAME_PATCH_CONTROL_POINTS];
        }
    }

    void WriteStructure(const VkPipelineViewportStateCreateInfo* pCreateInfo, nlohmann::json& file)
    {
        assert(pCreateInfo != nullptr);
        if (pCreateInfo != nullptr)
        {
            file[STR_MEMBER_NAME_TYPE] = pCreateInfo->sType;
            file[STR_MEMBER_NAME_PNEXT] = WriteHandle(reinterpret_cast<uint64_t>(pCreateInfo->pNext));
            file[STR_MEMBER_NAME_FLAGS] = pCreateInfo->flags;
            file[STR_MEMBER_NAME_VIEWPORT_COUNT] = pCreateInfo->viewportCount;
            file[STR_MEMBER_NAME_SCISSOR_COUNT] = pCreateInfo->scissorCount;
            if (pCreateInfo->pViewports != nullptr)
            {
                for (uint32_t index = 0; index < pCreateInfo->viewportCount; ++index)
                {
                    WriteStructure(pCreateInfo->pViewports + index, file[STR_MEMBER_NAME_P_VIEWPORTS][index]);
                }
            }

            if (pCreateInfo->pScissors != nullptr)
            {
                for (uint32_t index = 0; index < pCreateInfo->scissorCount; ++index)
                {
                    WriteStructure(pCreateInfo->pScissors + index, file[STR_MEMBER_NAME_P_SCISSORS][index]);
                }
            }
        }
    }

    void ReadStructure(VkPipelineViewportStateCreateInfo* pCreateInfo, const nlohmann::json& file)
    {
        assert(pCreateInfo != nullptr);
        if (pCreateInfo != nullptr)
        {
            pCreateInfo->sType = file[STR_MEMBER_NAME_TYPE];
            pCreateInfo->pNext = ReadHandle(file[STR_MEMBER_NAME_PNEXT]);
            pCreateInfo->flags = file[STR_MEMBER_NAME_FLAGS];

            // Parse the array of viewport rectangles.
            pCreateInfo->viewportCount = file[STR_MEMBER_NAME_VIEWPORT_COUNT];
            VkViewport* pViewports = nullptr;
            if (IsCreateInfoExists(file, STR_MEMBER_NAME_P_VIEWPORTS))
            {
                if (pCreateInfo->viewportCount > 0)
                {
                    pViewports = new VkViewport[pCreateInfo->viewportCount]{};
                    for (uint32_t index = 0; index < pCreateInfo->viewportCount; ++index)
                    {
                        ReadStructure(pViewports + index, file[STR_MEMBER_NAME_P_VIEWPORTS][index]);
                    }
                }
            }
            else
            {
                // In case that there is no viewport given, we would allocate a default one.
                pCreateInfo->viewportCount = 1;
                pViewports = new VkViewport{};
                pViewports->height = 1080;
                pViewports->width = 1920;
                pViewports->maxDepth = 1;
            }

            // Set the viewport in the create info structure.
            pCreateInfo->pViewports = pViewports;

            // Parse the array of scissor rectangles.
            pCreateInfo->scissorCount = file[STR_MEMBER_NAME_SCISSOR_COUNT];
            VkRect2D* pScissors = nullptr;
            if (IsCreateInfoExists(file, STR_MEMBER_NAME_P_SCISSORS))
            {
                if (pCreateInfo->scissorCount > 0)
                {
                    pScissors = new VkRect2D[pCreateInfo->scissorCount]{};
                    for (uint32_t index = 0; index < pCreateInfo->scissorCount; ++index)
                    {
                        ReadStructure(pScissors + index, file[STR_MEMBER_NAME_P_SCISSORS][index]);
                    }
                }
            }
            else
            {
                // In case that there is no scissor given, we would allocate a default one.
                pScissors = new VkRect2D[1]{};
                pCreateInfo->scissorCount = 1;
                pScissors->extent.height = 1080;
                pScissors->extent.width = 1920;
            }

            // Set the scissors in the create info structure.
            pCreateInfo->pScissors = pScissors;
        }
    }

    void WriteStructure(const VkPipelineRasterizationStateCreateInfo* pCreateInfo, nlohmann::json& file)
    {
        assert(pCreateInfo != nullptr);
        if (pCreateInfo != nullptr)
        {
            file[STR_MEMBER_NAME_TYPE] = pCreateInfo->sType;
            file[STR_MEMBER_NAME_PNEXT] = WriteHandle(reinterpret_cast<uint64_t>(pCreateInfo->pNext));
            file[STR_MEMBER_NAME_FLAGS] = pCreateInfo->flags;
            file[STR_MEMBER_NAME_DEPTH_CLAMP_ENABLE] = WriteBool(pCreateInfo->depthClampEnable);
            file[STR_MEMBER_NAME_RASTERIZER_DISCARD_ENABLE] = WriteBool(pCreateInfo->rasterizerDiscardEnable);
            file[STR_MEMBER_NAME_POLYGON_MODE] = pCreateInfo->polygonMode;
            file[STR_MEMBER_NAME_CULL_MODE] = pCreateInfo->cullMode;
            file[STR_MEMBER_NAME_FRONT_FACE] = pCreateInfo->frontFace;
            file[STR_MEMBER_NAME_DEPTH_BIAS_ENABLE] = WriteBool(pCreateInfo->depthBiasEnable);
            file[STR_MEMBER_NAME_DEPTH_BIAS_CONSTANT_FACTOR] = pCreateInfo->depthBiasConstantFactor;
            file[STR_MEMBER_NAME_DEPTH_BIAS_CLAMP] = pCreateInfo->depthBiasClamp;
            file[STR_MEMBER_NAME_DEPTH_BIAS_SLOPE_FACTOR] = pCreateInfo->depthBiasSlopeFactor;
            file[STR_MEMBER_NAME_LINE_WIDTH] = pCreateInfo->lineWidth;
        }
    }

    void ReadStructure(VkPipelineRasterizationStateCreateInfo* pCreateInfo, const nlohmann::json& file)
    {
        assert(pCreateInfo != nullptr);
        if (pCreateInfo != nullptr)
        {
            pCreateInfo->sType = file[STR_MEMBER_NAME_TYPE];
            pCreateInfo->pNext = ReadHandle(file[STR_MEMBER_NAME_PNEXT]);
            pCreateInfo->flags = file[STR_MEMBER_NAME_FLAGS];
            pCreateInfo->depthClampEnable = ReadBool(file[STR_MEMBER_NAME_DEPTH_CLAMP_ENABLE]);
            pCreateInfo->rasterizerDiscardEnable = ReadBool(file[STR_MEMBER_NAME_RASTERIZER_DISCARD_ENABLE]);
            pCreateInfo->polygonMode = file[STR_MEMBER_NAME_POLYGON_MODE];
            pCreateInfo->cullMode = file[STR_MEMBER_NAME_CULL_MODE];
            pCreateInfo->frontFace = file[STR_MEMBER_NAME_FRONT_FACE];
            pCreateInfo->depthBiasEnable = ReadBool(file[STR_MEMBER_NAME_DEPTH_BIAS_ENABLE]);
            pCreateInfo->depthBiasConstantFactor = file[STR_MEMBER_NAME_DEPTH_BIAS_CONSTANT_FACTOR];
            pCreateInfo->depthBiasClamp = file[STR_MEMBER_NAME_DEPTH_BIAS_CLAMP];
            pCreateInfo->depthBiasSlopeFactor = file[STR_MEMBER_NAME_DEPTH_BIAS_SLOPE_FACTOR];
            pCreateInfo->lineWidth = file[STR_MEMBER_NAME_LINE_WIDTH];
        }
    }

    void WriteStructure(const VkPipelineMultisampleStateCreateInfo* pCreateInfo, nlohmann::json& file)
    {
        assert(pCreateInfo != nullptr);
        if (pCreateInfo != nullptr)
        {
            file[STR_MEMBER_NAME_TYPE] = pCreateInfo->sType;
            file[STR_MEMBER_NAME_PNEXT] = WriteHandle(reinterpret_cast<uint64_t>(pCreateInfo->pNext));
            file[STR_MEMBER_NAME_FLAGS] = pCreateInfo->flags;
            file[STR_MEMBER_NAME_RASTERIZATION_SAMPLE] = pCreateInfo->rasterizationSamples;
            file[STR_MEMBER_NAME_SAMPLE_SHADING_ENABLE] = WriteBool(pCreateInfo->sampleShadingEnable);
            file[STR_MEMBER_NAME_MIN_SAMPLE_SHADING] = pCreateInfo->minSampleShading;
            WriteSampleMask(pCreateInfo, file);
            file[STR_MEMBER_NAME_ALPHA_TO_COVERAGE_ENABLE] = WriteBool(pCreateInfo->alphaToCoverageEnable);
            file[STR_MEMBER_NAME_ALPHA_TO_ONE_ENABLE] = WriteBool(pCreateInfo->alphaToOneEnable);
        }
    }

    void ReadStructure(VkPipelineMultisampleStateCreateInfo* pCreateInfo, const nlohmann::json& file)
    {
        assert(pCreateInfo != nullptr);
        if (pCreateInfo != nullptr)
        {
            pCreateInfo->sType = file[STR_MEMBER_NAME_TYPE];
            pCreateInfo->pNext = ReadHandle(file[STR_MEMBER_NAME_PNEXT]);
            pCreateInfo->flags = file[STR_MEMBER_NAME_FLAGS];
            pCreateInfo->rasterizationSamples = file[STR_MEMBER_NAME_RASTERIZATION_SAMPLE];
            pCreateInfo->sampleShadingEnable = ReadBool(file[STR_MEMBER_NAME_SAMPLE_SHADING_ENABLE]);
            pCreateInfo->minSampleShading = file[STR_MEMBER_NAME_MIN_SAMPLE_SHADING];
            pCreateInfo->pSampleMask = ReadSampleMask(pCreateInfo, file);
            pCreateInfo->alphaToCoverageEnable = ReadBool(file[STR_MEMBER_NAME_ALPHA_TO_COVERAGE_ENABLE]);
            pCreateInfo->alphaToOneEnable = ReadBool(file[STR_MEMBER_NAME_ALPHA_TO_ONE_ENABLE]);
        }
    }

    virtual void WriteStructure(const VkStencilOpState* pCreateInfo, nlohmann::json& file)
    {
        assert(pCreateInfo != nullptr);
        if (pCreateInfo != nullptr)
        {
            file[STR_MEMBER_NAME_PASS_OP] = pCreateInfo->passOp;
            file[STR_MEMBER_NAME_DEPTH_FAIL_OP] = pCreateInfo->depthFailOp;
            file[STR_MEMBER_NAME_COMPARE_OP] = pCreateInfo->compareOp;
            file[STR_MEMBER_NAME_COMPARE_MASK] = pCreateInfo->compareMask;
            file[STR_MEMBER_NAME_WRITE_MASK] = pCreateInfo->writeMask;
            file[STR_MEMBER_NAME_REFERENCE] = pCreateInfo->reference;
        }
    }

    virtual void ReadStructure(VkStencilOpState* pCreateInfo, const nlohmann::json& file)
    {
        assert(pCreateInfo != nullptr);
        if (pCreateInfo != nullptr)
        {
            pCreateInfo->passOp = file[STR_MEMBER_NAME_PASS_OP];
            pCreateInfo->depthFailOp = file[STR_MEMBER_NAME_DEPTH_FAIL_OP];
            pCreateInfo->compareOp = file[STR_MEMBER_NAME_COMPARE_OP];
            pCreateInfo->compareMask = file[STR_MEMBER_NAME_COMPARE_MASK];
            pCreateInfo->writeMask = file[STR_MEMBER_NAME_WRITE_MASK];
            pCreateInfo->reference = file[STR_MEMBER_NAME_REFERENCE];
        }
    }

    void WriteStructure(const VkPipelineDepthStencilStateCreateInfo* pCreateInfo, nlohmann::json& file)
    {
        assert(pCreateInfo != nullptr);
        if (pCreateInfo != nullptr)
        {
            file[STR_MEMBER_NAME_TYPE] = pCreateInfo->sType;
            file[STR_MEMBER_NAME_PNEXT] = WriteHandle(reinterpret_cast<uint64_t>(pCreateInfo->pNext));
            file[STR_MEMBER_NAME_FLAGS] = pCreateInfo->flags;
            file[STR_MEMBER_NAME_DEPTH_TEST_ENABLE] = WriteBool(pCreateInfo->depthTestEnable);
            file[STR_MEMBER_NAME_DEPTH_WRITE_ENABLE] = WriteBool(pCreateInfo->depthWriteEnable);
            file[STR_MEMBER_NAME_DEPTH_COMPARE_OP] = pCreateInfo->depthCompareOp;
            file[STR_MEMBER_NAME_DEPTH_BOUNDS_TEST_ENABLE] = WriteBool(pCreateInfo->depthBoundsTestEnable);
            file[STR_MEMBER_NAME_STENCIL_TEST_ENABLE] = WriteBool(pCreateInfo->stencilTestEnable);
            WriteStructure(&pCreateInfo->front, file[STR_MEMBER_NAME_FRONT]);
            WriteStructure(&pCreateInfo->back, file[STR_MEMBER_NAME_BACK]);
            file[STR_MEMBER_NAME_MIN_DEPTH_BOUNDS] = pCreateInfo->minDepthBounds;
            file[STR_MEMBER_NAME_MAX_DEPTH_BOUNDS] = pCreateInfo->maxDepthBounds;
        }
    }

    void ReadStructure(VkPipelineDepthStencilStateCreateInfo* pCreateInfo, const nlohmann::json& file)
    {
        assert(pCreateInfo != nullptr);
        if (pCreateInfo != nullptr)
        {
            pCreateInfo->sType = file[STR_MEMBER_NAME_TYPE];
            pCreateInfo->pNext = ReadHandle(file[STR_MEMBER_NAME_PNEXT]);
            pCreateInfo->flags = file[STR_MEMBER_NAME_FLAGS];
            pCreateInfo->depthTestEnable = ReadBool(file[STR_MEMBER_NAME_DEPTH_TEST_ENABLE]);
            pCreateInfo->depthWriteEnable = ReadBool(file[STR_MEMBER_NAME_DEPTH_WRITE_ENABLE]);
            pCreateInfo->depthCompareOp = file[STR_MEMBER_NAME_DEPTH_COMPARE_OP];

            pCreateInfo->depthBoundsTestEnable = ReadBool(file[STR_MEMBER_NAME_DEPTH_BOUNDS_TEST_ENABLE]);
            pCreateInfo->stencilTestEnable = ReadBool(file[STR_MEMBER_NAME_STENCIL_TEST_ENABLE]);
            ReadStructure(&pCreateInfo->front, file[STR_MEMBER_NAME_FRONT]);
            ReadStructure(&pCreateInfo->back, file[STR_MEMBER_NAME_BACK]);
            pCreateInfo->minDepthBounds = file[STR_MEMBER_NAME_MIN_DEPTH_BOUNDS];
            pCreateInfo->maxDepthBounds = file[STR_MEMBER_NAME_MAX_DEPTH_BOUNDS];
        }
    }

    void WriteStructure(const VkPipelineColorBlendAttachmentState* pCreateInfo, nlohmann::json& file)
    {
        assert(pCreateInfo != nullptr);
        if (pCreateInfo != nullptr)
        {
            file[STR_MEMBER_NAME_BLEND_ENABLE] = WriteBool(pCreateInfo->blendEnable);
            file[STR_MEMBER_NAME_SRC_COLOR_BLEND_FACTOR] = pCreateInfo->srcColorBlendFactor;
            file[STR_MEMBER_NAME_DST_COLOR_BLEND_FACTOR] = pCreateInfo->dstColorBlendFactor;
            file[STR_MEMBER_NAME_COLOR_BLEND_OP] = pCreateInfo->colorBlendOp;
            file[STR_MEMBER_NAME_SRC_ALPHA_BLEND_FACTOR] = pCreateInfo->srcAlphaBlendFactor;
            file[STR_MEMBER_NAME_DST_ALPHA_BLEND_FACTOR] = pCreateInfo->dstAlphaBlendFactor;
            file[STR_MEMBER_NAME_ALPHA_BLEND_OP] = pCreateInfo->alphaBlendOp;
            file[STR_MEMBER_NAME_COLOR_WRITE_MASK] = pCreateInfo->colorWriteMask;
        }
    }

    void ReadStructure(VkPipelineColorBlendAttachmentState* pCreateInfo, const nlohmann::json& file)
    {
        assert(pCreateInfo != nullptr);
        if (pCreateInfo != nullptr)
        {
            pCreateInfo->blendEnable = ReadBool(file[STR_MEMBER_NAME_BLEND_ENABLE]);
            pCreateInfo->srcColorBlendFactor = file[STR_MEMBER_NAME_SRC_COLOR_BLEND_FACTOR];
            pCreateInfo->dstColorBlendFactor = file[STR_MEMBER_NAME_DST_COLOR_BLEND_FACTOR];
            pCreateInfo->colorBlendOp = file[STR_MEMBER_NAME_COLOR_BLEND_OP];
            pCreateInfo->srcAlphaBlendFactor = file[STR_MEMBER_NAME_SRC_ALPHA_BLEND_FACTOR];
            pCreateInfo->dstAlphaBlendFactor = file[STR_MEMBER_NAME_DST_ALPHA_BLEND_FACTOR];
            pCreateInfo->alphaBlendOp = file[STR_MEMBER_NAME_ALPHA_BLEND_OP];
            pCreateInfo->colorWriteMask = file[STR_MEMBER_NAME_COLOR_WRITE_MASK];
        }
    }

    void WriteStructure(const VkPipelineColorBlendStateCreateInfo* pCreateInfo, nlohmann::json& file)
    {
        assert(pCreateInfo != nullptr);
        if (pCreateInfo != nullptr)
        {
            file[STR_MEMBER_NAME_TYPE] = pCreateInfo->sType;
            file[STR_MEMBER_NAME_PNEXT] = WriteHandle(reinterpret_cast<uint64_t>(pCreateInfo->pNext));
            file[STR_MEMBER_NAME_FLAGS] = pCreateInfo->flags;
            file[STR_MEMBER_NAME_LOGIC_OP_ENABLE] = WriteBool(pCreateInfo->logicOpEnable);
            file[STR_MEMBER_NAME_LOGIC_OP] = pCreateInfo->logicOp;
            file[STR_MEMBER_NAME_ATTACHMENT_COUNT] = pCreateInfo->attachmentCount;
            for (uint32_t attachmentIndex = 0; attachmentIndex < pCreateInfo->attachmentCount; attachmentIndex++)
            {
                WriteStructure(pCreateInfo->pAttachments + attachmentIndex, file[STR_MEMBER_NAME_P_ATTACHMENTS][attachmentIndex]);
            }
            for (uint32_t constantIndex = 0; constantIndex < 4; constantIndex++)
            {
                file[STR_MEMBER_NAME_BLEND_CONSTANTS][constantIndex] = pCreateInfo->blendConstants[constantIndex];
            }
        }
    }

    void ReadStructure(VkPipelineColorBlendStateCreateInfo* pCreateInfo, const nlohmann::json& file)
    {
        assert(pCreateInfo != nullptr);
        if (pCreateInfo != nullptr)
        {
            pCreateInfo->sType = file[STR_MEMBER_NAME_TYPE];
            pCreateInfo->pNext = ReadHandle(file[STR_MEMBER_NAME_PNEXT]);
            pCreateInfo->flags = file[STR_MEMBER_NAME_FLAGS];
            pCreateInfo->logicOpEnable = ReadBool(file[STR_MEMBER_NAME_LOGIC_OP_ENABLE]);
            pCreateInfo->logicOp = file[STR_MEMBER_NAME_LOGIC_OP];
            pCreateInfo->attachmentCount = file[STR_MEMBER_NAME_ATTACHMENT_COUNT];

            VkPipelineColorBlendAttachmentState* pAttachments = nullptr;
            if (pCreateInfo->attachmentCount > 0)
            {
                pAttachments = new VkPipelineColorBlendAttachmentState[pCreateInfo->attachmentCount]{};
                for (uint32_t attachmentIndex = 0; attachmentIndex < pCreateInfo->attachmentCount; attachmentIndex++)
                {
                    ReadStructure(pAttachments + attachmentIndex, file[STR_MEMBER_NAME_P_ATTACHMENTS][attachmentIndex]);
                }
            }
            pCreateInfo->pAttachments = pAttachments;

            for (uint32_t constantIndex = 0; constantIndex < 4; constantIndex++)
            {
                pCreateInfo->blendConstants[constantIndex] = file[STR_MEMBER_NAME_BLEND_CONSTANTS][constantIndex];
            }
        }
    }

    void WriteStructure(const VkPipelineDynamicStateCreateInfo* pCreateInfo, nlohmann::json& file)
    {
        assert(pCreateInfo != nullptr);
        if (pCreateInfo != nullptr)
        {
            file[STR_MEMBER_NAME_TYPE] = pCreateInfo->sType;
            file[STR_MEMBER_NAME_PNEXT] = WriteHandle(reinterpret_cast<uint64_t>(pCreateInfo->pNext));
            file[STR_MEMBER_NAME_FLAGS] = pCreateInfo->flags;
            file[STR_MEMBER_NAME_DYNAMIC_STATE_COUNT] = pCreateInfo->dynamicStateCount;

            for (uint32_t stateIndex = 0; stateIndex < pCreateInfo->dynamicStateCount; ++stateIndex)
            {
                file[STR_MEMBER_NAME_P_DYNAMIC_STATES][stateIndex] = *(pCreateInfo->pDynamicStates + stateIndex);
            }
        }
    }

    void ReadStructure(VkPipelineDynamicStateCreateInfo* pCreateInfo, const nlohmann::json& file)
    {
        assert(pCreateInfo != nullptr);
        if (pCreateInfo != nullptr)
        {
            pCreateInfo->sType = file[STR_MEMBER_NAME_TYPE];
            pCreateInfo->pNext = ReadHandle(file[STR_MEMBER_NAME_PNEXT]);
            pCreateInfo->flags = file[STR_MEMBER_NAME_FLAGS];
            pCreateInfo->dynamicStateCount = file[STR_MEMBER_NAME_DYNAMIC_STATE_COUNT];

            VkDynamicState* pDynamicStates = nullptr;
            if (pCreateInfo->dynamicStateCount > 0)
            {
                pDynamicStates = new VkDynamicState[pCreateInfo->dynamicStateCount]{};
                for (uint32_t stateIndex = 0; stateIndex < pCreateInfo->dynamicStateCount; ++stateIndex)
                {
                    pDynamicStates[stateIndex] = file[STR_MEMBER_NAME_P_DYNAMIC_STATES][stateIndex];
                }
            }
            pCreateInfo->pDynamicStates = pDynamicStates;
        }
    }

    void WriteStructure(const VkSpecializationInfo* pCreateInfo, nlohmann::json& file)
    {
        assert(pCreateInfo != nullptr);
        if (pCreateInfo != nullptr)
        {
            file[STR_MEMBER_NAME_MAP_ENTRY_COUNT] = pCreateInfo->mapEntryCount;
            for (uint32_t index = 0; index < pCreateInfo->mapEntryCount; ++index)
            {
                WriteStructure((pCreateInfo->pMapEntries + index), file[STR_MEMBER_NAME_P_MAP_ENTRIES][index]);
            }
            file[STR_MEMBER_NAME_DATA_SIZE] = pCreateInfo->dataSize;

            // pCreateInfo->dataSize is the number of bytes being serialized. In order to serialize
            // binary data, read/write each byte separately as an array.
            for (size_t byteIndex = 0; byteIndex < pCreateInfo->dataSize; ++byteIndex)
            {
                file[STR_MEMBER_NAME_P_DATA][byteIndex] = *((const uint8_t*)pCreateInfo->pData + byteIndex);
            }
        }
    }

    void ReadStructure(VkSpecializationInfo* pCreateInfo, const nlohmann::json& file)
    {
        assert(pCreateInfo != nullptr);
        if (pCreateInfo != nullptr)
        {
            pCreateInfo->mapEntryCount = file[STR_MEMBER_NAME_MAP_ENTRY_COUNT];
            VkSpecializationMapEntry* pMapEntries = nullptr;
            if (pCreateInfo->mapEntryCount > 0)
            {
                pMapEntries = new VkSpecializationMapEntry[pCreateInfo->mapEntryCount];
                for (uint32_t entryIndex = 0; entryIndex < pCreateInfo->mapEntryCount; ++entryIndex)
                {
                    ReadStructure(pMapEntries + entryIndex, file[STR_MEMBER_NAME_P_MAP_ENTRIES][entryIndex]);
                }
            }
            pCreateInfo->pMapEntries = pMapEntries;
            pCreateInfo->dataSize = file[STR_MEMBER_NAME_DATA_SIZE];

            // Allocate a byte array where the deserialized data will be copied.
            uint8_t* pDataBytes = new uint8_t[pCreateInfo->dataSize]{};

            // pCreateInfo->dataSize is the number of bytes being serialized.
            for (size_t byteIndex = 0; byteIndex < pCreateInfo->dataSize; ++byteIndex)
            {
                *(pDataBytes + byteIndex) = file[STR_MEMBER_NAME_P_DATA][byteIndex];
            }
            pCreateInfo->pData = pDataBytes;
        }
    }

    void WriteStructure(const VkVertexInputBindingDescription* pCreateInfo, nlohmann::json& file)
    {
        assert(pCreateInfo != nullptr);
        if (pCreateInfo != nullptr)
        {
            file[STR_MEMBER_NAME_BINDING] = pCreateInfo->binding;
            file[STR_MEMBER_NAME_STRIDE] = pCreateInfo->stride;
            file[STR_MEMBER_NAME_INPUT_RATE] = pCreateInfo->inputRate;
        }
    }

    void ReadStructure(VkVertexInputBindingDescription* pCreateInfo, const nlohmann::json& file)
    {
        assert(pCreateInfo != nullptr);
        if (pCreateInfo != nullptr)
        {
            pCreateInfo->binding = file[STR_MEMBER_NAME_BINDING];
            pCreateInfo->stride = file[STR_MEMBER_NAME_STRIDE];
            pCreateInfo->inputRate = file[STR_MEMBER_NAME_INPUT_RATE];
        }
    }

    void WriteStructure(const VkSpecializationMapEntry* pCreateInfo, nlohmann::json& file)
    {
        assert(pCreateInfo != nullptr);
        if (pCreateInfo != nullptr)
        {
            file[STR_MEMBER_NAME_CONSTANT_ID] = pCreateInfo->constantID;
            file[STR_MEMBER_NAME_OFFSET] = pCreateInfo->offset;
            file[STR_MEMBER_NAME_SIZE] = pCreateInfo->size;
        }
    }

    void ReadStructure(VkSpecializationMapEntry* pCreateInfo, const nlohmann::json& file)
    {
        assert(pCreateInfo != nullptr);
        if (pCreateInfo != nullptr)
        {
            pCreateInfo->constantID = file[STR_MEMBER_NAME_CONSTANT_ID];
            pCreateInfo->offset = file[STR_MEMBER_NAME_OFFSET];
            pCreateInfo->size = file[STR_MEMBER_NAME_SIZE];
        }
    }

    void WriteStructure(const VkVertexInputAttributeDescription* pCreateInfo, nlohmann::json& file)
    {
        assert(pCreateInfo != nullptr);
        if (pCreateInfo != nullptr)
        {
            file[STR_MEMBER_NAME_LOCATION] = pCreateInfo->location;
            file[STR_MEMBER_NAME_BINDING] = pCreateInfo->binding;
            file[STR_MEMBER_NAME_FORMAT] = pCreateInfo->format;
            file[STR_MEMBER_NAME_OFFSET] = pCreateInfo->offset;
        }
    }

    void ReadStructure(VkVertexInputAttributeDescription* pCreateInfo, const nlohmann::json& file)
    {
        assert(pCreateInfo != nullptr);
        if (pCreateInfo != nullptr)
        {
            pCreateInfo->location = file[STR_MEMBER_NAME_LOCATION];
            pCreateInfo->binding = file[STR_MEMBER_NAME_BINDING];
            pCreateInfo->format = file[STR_MEMBER_NAME_FORMAT];
            pCreateInfo->offset = file[STR_MEMBER_NAME_OFFSET];
        }
    }

    void WriteStructure(const VkViewport* pCreateInfo, nlohmann::json& file)
    {
        assert(pCreateInfo != nullptr);
        if (pCreateInfo != nullptr)
        {
            file[STR_MEMBER_NAME_X] = pCreateInfo->x;
            file[STR_MEMBER_NAME_Y] = pCreateInfo->y;
            file[STR_MEMBER_NAME_WIDTH] = pCreateInfo->width;
            file[STR_MEMBER_NAME_HEIGHT] = pCreateInfo->height;
            file[STR_MEMBER_NAME_MIN_DEPTH] = pCreateInfo->minDepth;
            file[STR_MEMBER_NAME_MAX_DEPTH] = pCreateInfo->maxDepth;
        }
    }

    void ReadStructure(VkViewport* pCreateInfo, const nlohmann::json& file)
    {
        assert(pCreateInfo != nullptr);
        if (pCreateInfo != nullptr)
        {
            pCreateInfo->x = file[STR_MEMBER_NAME_X];
            pCreateInfo->y = file[STR_MEMBER_NAME_Y];
            pCreateInfo->width = file[STR_MEMBER_NAME_WIDTH];
            pCreateInfo->height = file[STR_MEMBER_NAME_HEIGHT];
            pCreateInfo->minDepth = file[STR_MEMBER_NAME_MIN_DEPTH];
            pCreateInfo->maxDepth = file[STR_MEMBER_NAME_MAX_DEPTH];
        }
    }

    void WriteStructure(const VkRect2D* pCreateInfo, nlohmann::json& file)
    {
        assert(pCreateInfo != nullptr);
        if (pCreateInfo != nullptr)
        {
            WriteStructure(&pCreateInfo->offset, file[STR_MEMBER_NAME_OFFSET]);
            WriteStructure(&pCreateInfo->extent, file[STR_MEMBER_NAME_EXTENT]);
        }
    }

    void ReadStructure(VkRect2D* pCreateInfo, const nlohmann::json& file)
    {
        assert(pCreateInfo != nullptr);
        if (pCreateInfo != nullptr)
        {
            ReadStructure(&pCreateInfo->offset, file[STR_MEMBER_NAME_OFFSET]);
            ReadStructure(&pCreateInfo->extent, file[STR_MEMBER_NAME_EXTENT]);
        }
    }

    void WriteStructure(const VkOffset2D* pCreateInfo, nlohmann::json& file)
    {
        assert(pCreateInfo != nullptr);
        if (pCreateInfo != nullptr)
        {
            file[STR_MEMBER_NAME_X] = pCreateInfo->x;
            file[STR_MEMBER_NAME_Y] = pCreateInfo->y;
        }
    }

    void ReadStructure(VkOffset2D* pCreateInfo, const nlohmann::json& file)
    {
        assert(pCreateInfo != nullptr);
        if (pCreateInfo != nullptr)
        {
            pCreateInfo->x = file[STR_MEMBER_NAME_X];
            pCreateInfo->y = file[STR_MEMBER_NAME_Y];
        }
    }

    void WriteStructure(const VkExtent2D* pCreateInfo, nlohmann::json& file)
    {
        assert(pCreateInfo != nullptr);
        if (pCreateInfo != nullptr)
        {
            file[STR_MEMBER_NAME_WIDTH] = pCreateInfo->width;
            file[STR_MEMBER_NAME_HEIGHT] = pCreateInfo->height;
        }
    }

    void ReadStructure(VkExtent2D* pCreateInfo, const nlohmann::json& file)
    {
        assert(pCreateInfo != nullptr);
        if (pCreateInfo != nullptr)
        {
            pCreateInfo->width = file[STR_MEMBER_NAME_WIDTH];
            pCreateInfo->height = file[STR_MEMBER_NAME_HEIGHT];
        }
    }

    void WriteStructure(const VkComputePipelineCreateInfo* pCreateInfo, nlohmann::json& file)
    {
        assert(pCreateInfo != nullptr);
        if (pCreateInfo != nullptr)
        {
            file[STR_MEMBER_NAME_TYPE] = pCreateInfo->sType;
            file[STR_MEMBER_NAME_PNEXT] = WriteHandle(reinterpret_cast<uint64_t>(pCreateInfo->pNext));
            file[STR_MEMBER_NAME_FLAGS] = pCreateInfo->flags;
            WriteStructure(&pCreateInfo->stage, file[STR_MEMBER_NAME_STAGE]);
            if (pCreateInfo->layout != VK_NULL_HANDLE)
            {
                file[STR_MEMBER_NAME_LAYOUT] = WriteHandle(reinterpret_cast<uint64_t>(pCreateInfo->layout));
            }

            if (pCreateInfo->basePipelineHandle != VK_NULL_HANDLE)
            {
                file[STR_MEMBER_NAME_BASE_PIPELINE_HANDLE] = WriteHandle(reinterpret_cast<uint64_t>(pCreateInfo->basePipelineHandle));
            }

            file[STR_MEMBER_NAME_BASE_PIPELINE_INDEX] = pCreateInfo->basePipelineIndex;
        }
    }

    void ReadStructure(VkComputePipelineCreateInfo* pCreateInfo, const nlohmann::json& file)
    {
        assert(pCreateInfo != nullptr);
        if (pCreateInfo != nullptr)
        {
            pCreateInfo->sType = file[STR_MEMBER_NAME_TYPE];
            pCreateInfo->pNext = ReadHandle(file[STR_MEMBER_NAME_PNEXT]);
            pCreateInfo->flags = file[STR_MEMBER_NAME_FLAGS];
            ReadStructure(&pCreateInfo->stage, file[STR_MEMBER_NAME_STAGE]);
            if (IsCreateInfoExists(file, STR_MEMBER_NAME_LAYOUT))
            {
                pCreateInfo->layout = (VkPipelineLayout)ReadHandle(file[STR_MEMBER_NAME_LAYOUT]);
            }

            if (IsCreateInfoExists(file, STR_MEMBER_NAME_BASE_PIPELINE_HANDLE))
            {
                pCreateInfo->basePipelineHandle = (VkPipeline)ReadHandle(file[STR_MEMBER_NAME_BASE_PIPELINE_HANDLE]);
            }

            pCreateInfo->basePipelineIndex = file[STR_MEMBER_NAME_BASE_PIPELINE_INDEX];
        }
    }

    void WriteStructure(const VkPipelineLayoutCreateInfo* pCreateInfo, nlohmann::json& file)
    {
        assert(pCreateInfo != nullptr);
        if (pCreateInfo != nullptr)
        {
            file[STR_MEMBER_NAME_TYPE] = pCreateInfo->sType;
            file[STR_MEMBER_NAME_PNEXT] = WriteHandle(reinterpret_cast<uint64_t>(pCreateInfo->pNext));
            file[STR_MEMBER_NAME_FLAGS] = pCreateInfo->flags;
            file[STR_MEMBER_NAME_SET_LAYOUT_COUNT] = pCreateInfo->setLayoutCount;

            if (pCreateInfo->setLayoutCount > 0)
            {
                // Set the indices of descriptor set layouts which are referenced by this pipeline layout.
                for (uint32_t i = 0; i < pCreateInfo->setLayoutCount; i++)
                {
                    file[STR_MEMBER_NAME_P_SET_LAYOUTS][i] = WriteHandle((uint64_t)i);
                }
            }
            file[STR_MEMBER_NAME_PUSH_CONSTANT_RANGE_COUNT] = pCreateInfo->pushConstantRangeCount;
            for (uint32_t pushConstantRangeIndex = 0; pushConstantRangeIndex < pCreateInfo->pushConstantRangeCount; ++pushConstantRangeIndex)
            {
                WriteStructure((pCreateInfo->pPushConstantRanges + pushConstantRangeIndex), file[STR_MEMBER_NAME_P_PUSH_CONSTANT_RANGES][pushConstantRangeIndex]);
            }
        }
    }

    void ReadStructure(VkPipelineLayoutCreateInfo* pCreateInfo, const nlohmann::json& file)
    {
        assert(pCreateInfo != nullptr);
        if (pCreateInfo != nullptr)
        {
            pCreateInfo->sType = file[STR_MEMBER_NAME_TYPE];
            pCreateInfo->pNext = ReadHandle(file[STR_MEMBER_NAME_PNEXT]);
            pCreateInfo->flags = file[STR_MEMBER_NAME_FLAGS];

            pCreateInfo->setLayoutCount = file[STR_MEMBER_NAME_SET_LAYOUT_COUNT];
            VkDescriptorSetLayout* pSetLayouts = nullptr;
            if (pCreateInfo->setLayoutCount > 0)
            {
                pSetLayouts = new VkDescriptorSetLayout[pCreateInfo->setLayoutCount]{};
                for (uint32_t setLayoutIndex = 0; setLayoutIndex < pCreateInfo->setLayoutCount; ++setLayoutIndex)
                {
                    pSetLayouts[setLayoutIndex] = (VkDescriptorSetLayout)ReadHandle(file[STR_MEMBER_NAME_P_SET_LAYOUTS][setLayoutIndex]);
                }
            }
            pCreateInfo->pSetLayouts = pSetLayouts;

            pCreateInfo->pushConstantRangeCount = file[STR_MEMBER_NAME_PUSH_CONSTANT_RANGE_COUNT];
            VkPushConstantRange* pPushConstantRanges = nullptr;
            if (pCreateInfo->pushConstantRangeCount > 0)
            {
                pPushConstantRanges = new VkPushConstantRange[pCreateInfo->pushConstantRangeCount];
                for (uint32_t pushConstantRangeIndex = 0; pushConstantRangeIndex < pCreateInfo->pushConstantRangeCount; ++pushConstantRangeIndex)
                {
                    ReadStructure(pPushConstantRanges + pushConstantRangeIndex, file[STR_MEMBER_NAME_P_PUSH_CONSTANT_RANGES][pushConstantRangeIndex]);
                }
            }
            pCreateInfo->pPushConstantRanges = pPushConstantRanges;
        }
    }

    void WriteStructure(const VkShaderModuleCreateInfo* pCreateInfo, nlohmann::json& file)
    {
        assert(pCreateInfo != nullptr);
        if (pCreateInfo != nullptr)
        {
            file[STR_MEMBER_NAME_TYPE] = pCreateInfo->sType;
            file[STR_MEMBER_NAME_PNEXT] = WriteHandle(reinterpret_cast<uint64_t>(pCreateInfo->pNext));
            file[STR_MEMBER_NAME_FLAGS] = pCreateInfo->flags;
            file[STR_MEMBER_NAME_CODE_SIZE] = pCreateInfo->codeSize;
        }
    }

    void ReadStructure(VkShaderModuleCreateInfo* pCreateInfo, const nlohmann::json& file)
    {
        assert(pCreateInfo != nullptr);
        if (pCreateInfo != nullptr)
        {
            pCreateInfo->sType = file[STR_MEMBER_NAME_TYPE];
            pCreateInfo->pNext = ReadHandle(file[STR_MEMBER_NAME_PNEXT]);
            pCreateInfo->flags = file[STR_MEMBER_NAME_FLAGS];
            pCreateInfo->codeSize = file[STR_MEMBER_NAME_CODE_SIZE];
        }
    }

    void WriteStructure(const VkDescriptorSetLayoutCreateInfo* pCreateInfo, nlohmann::json& file)
    {
        assert(pCreateInfo != nullptr);
        if (pCreateInfo != nullptr)
        {
            file[STR_MEMBER_NAME_TYPE] = pCreateInfo->sType;
            file[STR_MEMBER_NAME_PNEXT] = WriteHandle(reinterpret_cast<uint64_t>(pCreateInfo->pNext));
            file[STR_MEMBER_NAME_FLAGS] = pCreateInfo->flags;
            file[STR_MEMBER_NAME_BINDING_COUNT] = pCreateInfo->bindingCount;

            for (uint32_t bindingIndex = 0; bindingIndex < pCreateInfo->bindingCount; ++bindingIndex)
            {
                WriteStructure((pCreateInfo->pBindings + bindingIndex), file[STR_MEMBER_NAME_P_BINDINGS][bindingIndex]);
            }
        }
    }

    void ReadStructure(VkDescriptorSetLayoutCreateInfo* pCreateInfo, const nlohmann::json& file)
    {
        assert(pCreateInfo != nullptr);
        if (pCreateInfo != nullptr)
        {
            pCreateInfo->sType = file[STR_MEMBER_NAME_TYPE];
            pCreateInfo->pNext = ReadHandle(file[STR_MEMBER_NAME_PNEXT]);
            pCreateInfo->flags = file[STR_MEMBER_NAME_FLAGS];
            pCreateInfo->bindingCount = file[STR_MEMBER_NAME_BINDING_COUNT];

            VkDescriptorSetLayoutBinding* pBindings = nullptr;
            if (pCreateInfo->bindingCount > 0)
            {
                pBindings = new VkDescriptorSetLayoutBinding[pCreateInfo->bindingCount]{};
                for (uint32_t bindingIndex = 0; bindingIndex < pCreateInfo->bindingCount; ++bindingIndex)
                {
                    ReadStructure(pBindings + bindingIndex, file[STR_MEMBER_NAME_P_BINDINGS][bindingIndex]);
                }
            }
            pCreateInfo->pBindings = pBindings;
        }
    }

    void WriteStructure(const VkDescriptorSetLayoutBinding* pCreateInfo, nlohmann::json& file)
    {
        assert(pCreateInfo != nullptr);
        if (pCreateInfo != nullptr)
        {
            // Note: for now we are ignoring the immutable samplers.
            file[STR_MEMBER_NAME_BINDING] = pCreateInfo->binding;
            file[STR_MEMBER_NAME_DESCRIPTOR_TYPE] = pCreateInfo->descriptorType;
            file[STR_MEMBER_NAME_DESCRIPTOR_COUNT] = pCreateInfo->descriptorCount;
            file[STR_MEMBER_NAME_STAGE_FLAGS] = pCreateInfo->stageFlags;
        }
    }

    void ReadStructure(VkDescriptorSetLayoutBinding* pCreateInfo, const nlohmann::json& file)
    {
        assert(pCreateInfo != nullptr);
        if (pCreateInfo != nullptr)
        {
            pCreateInfo->binding = file[STR_MEMBER_NAME_BINDING];
            pCreateInfo->descriptorType = file[STR_MEMBER_NAME_DESCRIPTOR_TYPE];
            pCreateInfo->descriptorCount = file[STR_MEMBER_NAME_DESCRIPTOR_COUNT];
            pCreateInfo->stageFlags = file[STR_MEMBER_NAME_STAGE_FLAGS];
        }
    }

    void WriteStructure(const VkPushConstantRange* pCreateInfo, nlohmann::json& file)
    {
        assert(pCreateInfo != nullptr);
        if (pCreateInfo != nullptr)
        {
            file[STR_MEMBER_NAME_STAGE_FLAGS] = pCreateInfo->stageFlags;
            file[STR_MEMBER_NAME_OFFSET] = pCreateInfo->offset;
            file[STR_MEMBER_NAME_SIZE] = pCreateInfo->size;
        }
    }

    void ReadStructure(VkPushConstantRange* pCreateInfo, const nlohmann::json& file)
    {
        assert(pCreateInfo != nullptr);
        if (pCreateInfo != nullptr)
        {
            pCreateInfo->stageFlags = file[STR_MEMBER_NAME_STAGE_FLAGS];
            pCreateInfo->offset = file[STR_MEMBER_NAME_OFFSET];
            pCreateInfo->size = file[STR_MEMBER_NAME_SIZE];
        }
    }

    void WriteStructure(const VkRenderPassCreateInfo* pCreateInfo, nlohmann::json& file)
    {
        assert(pCreateInfo != nullptr);
        if (pCreateInfo != nullptr)
        {
            file[STR_MEMBER_NAME_TYPE] = pCreateInfo->sType;
            file[STR_MEMBER_NAME_PNEXT] = WriteHandle(reinterpret_cast<uint64_t>(pCreateInfo->pNext));
            file[STR_MEMBER_NAME_FLAGS] = pCreateInfo->flags;
            file[STR_MEMBER_NAME_ATTACHMENT_COUNT] = pCreateInfo->attachmentCount;

            for (uint32_t attachmentIndex = 0; attachmentIndex < pCreateInfo->attachmentCount; ++attachmentIndex)
            {
                WriteStructure(pCreateInfo->pAttachments + attachmentIndex, file[STR_MEMBER_NAME_P_ATTACHMENTS][attachmentIndex]);
            }

            file[STR_MEMBER_NAME_SUBPASS_COUNT] = pCreateInfo->subpassCount;
            for (uint32_t subpassIndex = 0; subpassIndex < pCreateInfo->subpassCount; ++subpassIndex)
            {
                WriteStructure(pCreateInfo->pSubpasses + subpassIndex, file[STR_MEMBER_NAME_P_SUBPASSES][subpassIndex]);
            }

            file[STR_MEMBER_NAME_DEPENDENCY_COUNT] = pCreateInfo->dependencyCount;
            for (uint32_t dependencyIndex = 0; dependencyIndex < pCreateInfo->dependencyCount; ++dependencyIndex)
            {
                WriteStructure(pCreateInfo->pDependencies + dependencyIndex, file[STR_MEMBER_NAME_P_DEPENDENCIES][dependencyIndex]);
            }
        }
    }

    void ReadStructure(VkRenderPassCreateInfo* pCreateInfo, const nlohmann::json& file)
    {
        assert(pCreateInfo != nullptr);
        if (pCreateInfo != nullptr)
        {
            pCreateInfo->sType = file[STR_MEMBER_NAME_TYPE];
            pCreateInfo->pNext = ReadHandle(file[STR_MEMBER_NAME_PNEXT]);
            pCreateInfo->flags = file[STR_MEMBER_NAME_FLAGS];
            pCreateInfo->attachmentCount = file[STR_MEMBER_NAME_ATTACHMENT_COUNT];

            VkAttachmentDescription* pAttachments = nullptr;
            if (pCreateInfo->attachmentCount > 0)
            {
                pAttachments = new VkAttachmentDescription[pCreateInfo->attachmentCount]{};
                for (uint32_t attachmentIndex = 0; attachmentIndex < pCreateInfo->attachmentCount; ++attachmentIndex)
                {
                    ReadStructure(pAttachments + attachmentIndex, file[STR_MEMBER_NAME_P_ATTACHMENTS][attachmentIndex]);
                }
            }
            pCreateInfo->pAttachments = pAttachments;

            pCreateInfo->subpassCount = file[STR_MEMBER_NAME_SUBPASS_COUNT];
            VkSubpassDescription* pSubpasses = nullptr;
            if (pCreateInfo->subpassCount > 0)
            {
                pSubpasses = new VkSubpassDescription[pCreateInfo->subpassCount]{};
                for (uint32_t subpassIndex = 0; subpassIndex < pCreateInfo->subpassCount; ++subpassIndex)
                {
                    ReadStructure(pSubpasses + subpassIndex, file[STR_MEMBER_NAME_P_SUBPASSES][subpassIndex]);
                }
            }
            pCreateInfo->pSubpasses = pSubpasses;

            pCreateInfo->dependencyCount = file[STR_MEMBER_NAME_DEPENDENCY_COUNT];
            VkSubpassDependency* pDependencies = nullptr;
            if (pCreateInfo->dependencyCount > 0)
            {
                pDependencies = new VkSubpassDependency[pCreateInfo->dependencyCount]{};
                for (uint32_t dependencyIndex = 0; dependencyIndex < pCreateInfo->dependencyCount; ++dependencyIndex)
                {
                    ReadStructure(pDependencies + dependencyIndex, file[STR_MEMBER_NAME_P_DEPENDENCIES][dependencyIndex]);
                }
            }
            pCreateInfo->pDependencies = pDependencies;
        }
    }

    void WriteStructure(const VkAttachmentDescription* pCreateInfo, nlohmann::json& file)
    {
        assert(pCreateInfo != nullptr);
        if (pCreateInfo != nullptr)
        {
            file[STR_MEMBER_NAME_FLAGS] = pCreateInfo->flags;
            file[STR_MEMBER_NAME_FORMAT] = pCreateInfo->format;
            file[STR_MEMBER_NAME_SAMPLES] = pCreateInfo->samples;
            file[STR_MEMBER_NAME_LOAD_OP] = pCreateInfo->loadOp;
            file[STR_MEMBER_NAME_STORE_OP] = pCreateInfo->storeOp;
            file[STR_MEMBER_NAME_STENCIL_LOAD_OP] = pCreateInfo->stencilLoadOp;
            file[STR_MEMBER_NAME_STENCIL_STORE_OP] = pCreateInfo->stencilStoreOp;
            file[STR_MEMBER_NAME_INITIAL_LAYOUT] = pCreateInfo->initialLayout;
            file[STR_MEMBER_NAME_FINAL_LAYOUT] = pCreateInfo->finalLayout;
        }
    }

    void ReadStructure(VkAttachmentDescription* pCreateInfo, const nlohmann::json& file)
    {
        assert(pCreateInfo != nullptr);
        if (pCreateInfo != nullptr)
        {
            pCreateInfo->flags = file[STR_MEMBER_NAME_FLAGS];
            pCreateInfo->format = file[STR_MEMBER_NAME_FORMAT];
            pCreateInfo->samples = file[STR_MEMBER_NAME_SAMPLES];
            pCreateInfo->loadOp = file[STR_MEMBER_NAME_LOAD_OP];
            pCreateInfo->storeOp = file[STR_MEMBER_NAME_STORE_OP];
            pCreateInfo->stencilLoadOp = file[STR_MEMBER_NAME_STENCIL_LOAD_OP];
            pCreateInfo->stencilStoreOp = file[STR_MEMBER_NAME_STENCIL_STORE_OP];
            pCreateInfo->initialLayout = file[STR_MEMBER_NAME_INITIAL_LAYOUT];
            pCreateInfo->finalLayout = file[STR_MEMBER_NAME_FINAL_LAYOUT];
        }
    }

    void WriteStructure(const VkSubpassDescription* pCreateInfo, nlohmann::json& file)
    {
        assert(pCreateInfo != nullptr);
        if (pCreateInfo != nullptr)
        {
            file[STR_MEMBER_NAME_FLAGS] = pCreateInfo->flags;
            file[STR_MEMBER_NAME_PIPELINE_BIND_POINT] = pCreateInfo->pipelineBindPoint;
            file[STR_MEMBER_NAME_INPUT_ATTACHMENT_COUNT] = pCreateInfo->inputAttachmentCount;
            for (uint32_t inputAttachmentIndex = 0; inputAttachmentIndex < pCreateInfo->inputAttachmentCount; ++inputAttachmentIndex)
            {
                WriteStructure(pCreateInfo->pInputAttachments + inputAttachmentIndex, file[STR_MEMBER_NAME_P_INPUT_ATTACHMENTS][inputAttachmentIndex]);
            }
            file[STR_MEMBER_NAME_COLOR_ATTACHMENT_COUNT] = pCreateInfo->colorAttachmentCount;
            for (uint32_t attachmentIndex = 0; attachmentIndex < pCreateInfo->colorAttachmentCount; ++attachmentIndex)
            {
                WriteStructure(pCreateInfo->pColorAttachments + attachmentIndex, file[STR_MEMBER_NAME_P_COLOR_ATTACHMENTS][attachmentIndex]);
                if (pCreateInfo->pResolveAttachments != nullptr)
                {
                    WriteStructure(pCreateInfo->pResolveAttachments + attachmentIndex, file[STR_MEMBER_NAME_P_RESOLVE_ATTACHMENTS][attachmentIndex]);
                }
            }

            if (pCreateInfo->pDepthStencilAttachment != nullptr)
            {
                WriteStructure(pCreateInfo->pDepthStencilAttachment, file[STR_MEMBER_NAME_P_DEPTH_STENCIL_ATTACHMENT]);
            }

            file[STR_MEMBER_NAME_PRESERVE_ATTACHMENT_COUNT] = pCreateInfo->preserveAttachmentCount;
            for (uint32_t preserveAttachmentIndex = 0; preserveAttachmentIndex < pCreateInfo->preserveAttachmentCount; ++preserveAttachmentIndex)
            {
                file[STR_MEMBER_NAME_P_PRESERVE_ATTACHMENTS][preserveAttachmentIndex] = *(pCreateInfo->pPreserveAttachments + preserveAttachmentIndex);
            }
        }
    }

    void ReadStructure(VkSubpassDescription* pCreateInfo, const nlohmann::json& file)
    {
        assert(pCreateInfo != nullptr);
        if (pCreateInfo != nullptr)
        {
            pCreateInfo->flags = file[STR_MEMBER_NAME_FLAGS];
            pCreateInfo->pipelineBindPoint = file[STR_MEMBER_NAME_PIPELINE_BIND_POINT];
            pCreateInfo->inputAttachmentCount = file[STR_MEMBER_NAME_INPUT_ATTACHMENT_COUNT];

            VkAttachmentReference* pInputAttachments = nullptr;
            if (pCreateInfo->inputAttachmentCount > 0)
            {
                pInputAttachments = new VkAttachmentReference[pCreateInfo->inputAttachmentCount]{};
                for (uint32_t inputAttachmentIndex = 0; inputAttachmentIndex < pCreateInfo->inputAttachmentCount; ++inputAttachmentIndex)
                {
                    ReadStructure(pInputAttachments + inputAttachmentIndex, file[STR_MEMBER_NAME_P_INPUT_ATTACHMENTS][inputAttachmentIndex]);
                }
            }
            pCreateInfo->pInputAttachments = pInputAttachments;

            pCreateInfo->colorAttachmentCount = file[STR_MEMBER_NAME_COLOR_ATTACHMENT_COUNT];
            VkAttachmentReference* pColorAttachments = nullptr;
            VkAttachmentReference* pResolveAttachments = nullptr;
            if (pCreateInfo->colorAttachmentCount > 0)
            {
                pColorAttachments = new VkAttachmentReference[pCreateInfo->colorAttachmentCount]{};
                for (uint32_t attachmentIndex = 0; attachmentIndex < pCreateInfo->colorAttachmentCount; ++attachmentIndex)
                {
                    ReadStructure(pColorAttachments + attachmentIndex, file[STR_MEMBER_NAME_P_COLOR_ATTACHMENTS][attachmentIndex]);
                }

                // Verify that an array of Resolve Attachments exists. If it does, it's the same dimension as the color attachments.
                if (IsCreateInfoExists(file, STR_MEMBER_NAME_P_RESOLVE_ATTACHMENTS))
                {
                    pResolveAttachments = new VkAttachmentReference[pCreateInfo->colorAttachmentCount]{};
                    for (uint32_t attachmentIndex = 0; attachmentIndex < pCreateInfo->colorAttachmentCount; ++attachmentIndex)
                    {
                        ReadStructure(pResolveAttachments + attachmentIndex, file[STR_MEMBER_NAME_P_RESOLVE_ATTACHMENTS][attachmentIndex]);
                    }
                }
            }
            pCreateInfo->pColorAttachments = pColorAttachments;
            pCreateInfo->pResolveAttachments = pResolveAttachments;

            VkAttachmentReference* pDepthStencilAttachment = nullptr;
            if (IsCreateInfoExists(file, STR_MEMBER_NAME_P_DEPTH_STENCIL_ATTACHMENT))
            {
                pDepthStencilAttachment = new VkAttachmentReference{};
                ReadStructure(pDepthStencilAttachment, file[STR_MEMBER_NAME_P_DEPTH_STENCIL_ATTACHMENT]);
            }
            pCreateInfo->pDepthStencilAttachment = pDepthStencilAttachment;

            pCreateInfo->preserveAttachmentCount = file[STR_MEMBER_NAME_PRESERVE_ATTACHMENT_COUNT];
            uint32_t* pPreserveAttachments = nullptr;
            if (pCreateInfo->preserveAttachmentCount > 0)
            {
                pPreserveAttachments = new uint32_t[pCreateInfo->preserveAttachmentCount]{};
                for (uint32_t preserveAttachmentIndex = 0; preserveAttachmentIndex < pCreateInfo->preserveAttachmentCount; ++preserveAttachmentIndex)
                {
                    *(pPreserveAttachments + preserveAttachmentIndex) = file[STR_MEMBER_NAME_P_PRESERVE_ATTACHMENTS][preserveAttachmentIndex];
                }
            }
            pCreateInfo->pPreserveAttachments = pPreserveAttachments;
        }
    }

    void WriteStructure(const VkSubpassDependency* pCreateInfo, nlohmann::json& file)
    {
        assert(pCreateInfo != nullptr);
        if (pCreateInfo != nullptr)
        {
            file[STR_MEMBER_NAME_SRC_SUBPASS] = pCreateInfo->srcSubpass;
            file[STR_MEMBER_NAME_DST_SUBPASS] = pCreateInfo->dstSubpass;
            file[STR_MEMBER_NAME_SRC_STAGE_MASK] = pCreateInfo->srcStageMask;
            file[STR_MEMBER_NAME_DST_STAGE_MASK] = pCreateInfo->dstStageMask;
            file[STR_MEMBER_NAME_SRC_ACCESS_MASK] = pCreateInfo->srcAccessMask;
            file[STR_MEMBER_NAME_DST_ACCESS_MASK] = pCreateInfo->dstAccessMask;
            file[STR_MEMBER_NAME_DEPENDENCY_FLAGS] = pCreateInfo->dependencyFlags;
        }
    }

    void ReadStructure(VkSubpassDependency* pCreateInfo, const nlohmann::json& file)
    {
        assert(pCreateInfo != nullptr);
        if (pCreateInfo != nullptr)
        {
            pCreateInfo->srcSubpass = file[STR_MEMBER_NAME_SRC_SUBPASS];
            pCreateInfo->dstSubpass = file[STR_MEMBER_NAME_DST_SUBPASS];
            pCreateInfo->srcStageMask = file[STR_MEMBER_NAME_SRC_STAGE_MASK];
            pCreateInfo->dstStageMask = file[STR_MEMBER_NAME_DST_STAGE_MASK];
            pCreateInfo->srcAccessMask = file[STR_MEMBER_NAME_SRC_ACCESS_MASK];
            pCreateInfo->dstAccessMask = file[STR_MEMBER_NAME_DST_ACCESS_MASK];
            pCreateInfo->dependencyFlags = file[STR_MEMBER_NAME_DEPENDENCY_FLAGS];
        }
    }

    void WriteStructure(const VkAttachmentReference* pCreateInfo, nlohmann::json& file)
    {
        assert(pCreateInfo != nullptr);
        if (pCreateInfo != nullptr)
        {
            file[STR_MEMBER_NAME_ATTACHMENT] = pCreateInfo->attachment;
            file[STR_MEMBER_NAME_LAYOUT] = pCreateInfo->layout;
        }
    }

    void ReadStructure(VkAttachmentReference* pCreateInfo, const nlohmann::json& file)
    {
        assert(pCreateInfo != nullptr);
        if (pCreateInfo != nullptr)
        {
            pCreateInfo->attachment = file[STR_MEMBER_NAME_ATTACHMENT];
            pCreateInfo->layout = file[STR_MEMBER_NAME_LAYOUT];
        }
    }

    void ReadDescriptorSetLayoutCreateInfoArray(rgPsoCreateInfoVulkan* pCreateInfo, const nlohmann::json& file)
    {
        assert(pCreateInfo != nullptr);
        if (pCreateInfo != nullptr)
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
                std::vector<VkDescriptorSetLayoutCreateInfo*> descriptorSetLayoutCollection = pCreateInfo->GetDescriptorSetLayoutCreateInfo();
                descriptorSetLayoutCollection.clear();

                // Read each individual element in the array of create info.
                for (auto itemIter = firstItem; itemIter != lastItem; ++itemIter)
                {
                    VkDescriptorSetLayoutCreateInfo* pNewDescriptorSetLayout = new VkDescriptorSetLayoutCreateInfo{};
                    assert(pNewDescriptorSetLayout != nullptr);
                    if (pNewDescriptorSetLayout != nullptr)
                    {
                        ReadStructure(pNewDescriptorSetLayout, *itemIter);
                        pCreateInfo->AddDescriptorSetLayoutCreateInfo(pNewDescriptorSetLayout);
                    }
                }
            }
        }
    }

    void WriteDescriptorSetLayoutCreateInfoArray(rgPsoCreateInfoVulkan* pCreateInfo, nlohmann::json& file)
    {
        assert(pCreateInfo != nullptr);
        if (pCreateInfo != nullptr)
        {
            int index = 0;
            std::vector<VkDescriptorSetLayoutCreateInfo*> descriptorSetLayoutCollection = pCreateInfo->GetDescriptorSetLayoutCreateInfo();
            for (VkDescriptorSetLayoutCreateInfo* pDescriptorSetLayoutCreateInfo : descriptorSetLayoutCollection)
            {
                assert(pDescriptorSetLayoutCreateInfo != nullptr);
                if (pDescriptorSetLayoutCreateInfo != nullptr)
                {
                    WriteStructure(pDescriptorSetLayoutCreateInfo, file[STR_MEMBER_NAME_VK_DESCRIPTOR_SET_LAYOUT_CREATE_INFO][index]);
                    index++;
                }
            }
        }
    }

    bool ReadStructure(rgPsoGraphicsVulkan* pCreateInfo, const nlohmann::json& file)
    {
        bool isLoaded = false;

        assert(pCreateInfo != nullptr);
        if (pCreateInfo != nullptr)
        {
            // Does the given file's root element match what we expect to see for the project's pipeline type?
            bool isMatchingRootElement = IsCreateInfoExists(file, STR_MEMBER_NAME_VK_GRAPHICS_PIPELINE_CREATE_INFO);
            if (isMatchingRootElement)
            {
                // Deserialize the Graphics Pipeline create info.
                ReadGraphicsPipelineCreateInfoStructure(pCreateInfo, file[STR_MEMBER_NAME_VK_GRAPHICS_PIPELINE_CREATE_INFO]);

                // Deserialize the Render Pass create info.
                VkRenderPassCreateInfo* pRenderPassCreateInfo = pCreateInfo->GetRenderPassCreateInfo();
                assert(pRenderPassCreateInfo != nullptr);
                if (pRenderPassCreateInfo != nullptr)
                {
                    ReadStructure(pRenderPassCreateInfo, file[STR_MEMBER_NAME_VK_RENDER_PASS_CREATE_INFO]);
                }

                // Deserialize the Pipeline Layout create info.
                VkPipelineLayoutCreateInfo* pPipelineLayoutCreateInfo = pCreateInfo->GetPipelineLayoutCreateInfo();
                assert(pPipelineLayoutCreateInfo != nullptr);
                if (pPipelineLayoutCreateInfo != nullptr)
                {
                    ReadStructure(pPipelineLayoutCreateInfo, file[STR_MEMBER_NAME_VK_PIPELINE_LAYOUT_CREATE_INFO]);
                }

                // Read all Descriptor Set Layout create info structures.
                ReadDescriptorSetLayoutCreateInfoArray(pCreateInfo, file);

                // If all data was deserialized successfully, the PSO file is loaded.
                isLoaded = true;
            }
        }

        return isLoaded;
    }

    bool ReadStructure(rgPsoComputeVulkan* pCreateInfo, const nlohmann::json& file)
    {
        bool isLoaded = false;

        assert(pCreateInfo != nullptr);
        if (pCreateInfo != nullptr)
        {
            // Does the given file's root element match what we expect to see for the project's pipeline type?
            bool isMatchingRootElement = IsCreateInfoExists(file, STR_MEMBER_NAME_VK_COMPUTE_PIPELINE_CREATE_INFO);
            if (isMatchingRootElement)
            {
                VkComputePipelineCreateInfo* pComputePipelineCreateInfo = pCreateInfo->GetComputePipelineCreateInfo();
                assert(pComputePipelineCreateInfo != nullptr);
                if (pComputePipelineCreateInfo != nullptr)
                {
                    ReadStructure(pComputePipelineCreateInfo, file[STR_MEMBER_NAME_VK_COMPUTE_PIPELINE_CREATE_INFO]);
                }

                VkPipelineLayoutCreateInfo* pPipelineLayoutCreateInfo = pCreateInfo->GetPipelineLayoutCreateInfo();
                assert(pPipelineLayoutCreateInfo != nullptr);
                if (pPipelineLayoutCreateInfo != nullptr)
                {
                    ReadStructure(pPipelineLayoutCreateInfo, file[STR_MEMBER_NAME_VK_PIPELINE_LAYOUT_CREATE_INFO]);
                }

                // Read all Descriptor Set Layout create info structures.
                ReadDescriptorSetLayoutCreateInfoArray(pCreateInfo, file);

                // If all data was deserialized successfully, the PSO file is loaded.
                isLoaded = true;
            }
        }

        return isLoaded;
    }

    void WriteStructure(rgPsoGraphicsVulkan* pCreateInfo, nlohmann::json& file)
    {
        assert(pCreateInfo != nullptr);
        if (pCreateInfo != nullptr)
        {
            VkGraphicsPipelineCreateInfo* pGraphicsPsoCreateInfo = pCreateInfo->GetGraphicsPipelineCreateInfo();
            assert(pGraphicsPsoCreateInfo != nullptr);
            if (pGraphicsPsoCreateInfo != nullptr)
            {
                WriteStructure(pGraphicsPsoCreateInfo, file[STR_MEMBER_NAME_VK_GRAPHICS_PIPELINE_CREATE_INFO]);
            }

            VkRenderPassCreateInfo* pRenderPassCreateInfo = pCreateInfo->GetRenderPassCreateInfo();
            assert(pRenderPassCreateInfo != nullptr);
            if (pRenderPassCreateInfo != nullptr)
            {
                WriteStructure(pRenderPassCreateInfo, file[STR_MEMBER_NAME_VK_RENDER_PASS_CREATE_INFO]);
            }

            VkPipelineLayoutCreateInfo* pPipelineLayoutCreateInfo = pCreateInfo->GetPipelineLayoutCreateInfo();
            assert(pPipelineLayoutCreateInfo != nullptr);
            if (pPipelineLayoutCreateInfo != nullptr)
            {
                WriteStructure(pPipelineLayoutCreateInfo, file[STR_MEMBER_NAME_VK_PIPELINE_LAYOUT_CREATE_INFO]);
            }

            // Write the array of Descriptor Set Layout create info structures.
            WriteDescriptorSetLayoutCreateInfoArray(pCreateInfo, file);
        }
    }

    void WriteStructure(rgPsoComputeVulkan* pCreateInfo, nlohmann::json& file)
    {
        assert(pCreateInfo != nullptr);
        if (pCreateInfo != nullptr)
        {
            VkComputePipelineCreateInfo* pComputePsoCreateInfo = pCreateInfo->GetComputePipelineCreateInfo();
            assert(pComputePsoCreateInfo != nullptr);
            if (pComputePsoCreateInfo != nullptr)
            {
                WriteStructure(pComputePsoCreateInfo, file[STR_MEMBER_NAME_VK_COMPUTE_PIPELINE_CREATE_INFO]);
            }

            VkPipelineLayoutCreateInfo* pPipelineLayoutCreateInfo = pCreateInfo->GetPipelineLayoutCreateInfo();
            assert(pPipelineLayoutCreateInfo != nullptr);
            if (pPipelineLayoutCreateInfo != nullptr)
            {
                WriteStructure(pPipelineLayoutCreateInfo, file[STR_MEMBER_NAME_VK_PIPELINE_LAYOUT_CREATE_INFO]);
            }

            // Write the array of Descriptor Set Layout create info structures.
            WriteDescriptorSetLayoutCreateInfoArray(pCreateInfo, file);
        }
    }

    virtual VkSampleMask* ReadSampleMask(const VkPipelineMultisampleStateCreateInfo* pCreateInfo, const nlohmann::json& file)
    {
        VkSampleMask* pSampleMask = nullptr;
        if (IsCreateInfoExists(file, STR_MEMBER_NAME_P_SAMPLE_MASK))
        {
            pSampleMask = new VkSampleMask[(uint32_t)pCreateInfo->rasterizationSamples];
            for (uint32_t index = 0; index < (uint32_t)pCreateInfo->rasterizationSamples; ++index)
            {
                pSampleMask[index] = file[STR_MEMBER_NAME_P_SAMPLE_MASK][index];
            }
        }
        return pSampleMask;
    }

    virtual void WriteSampleMask(const VkPipelineMultisampleStateCreateInfo* pCreateInfo, nlohmann::json& file)
    {
        if (pCreateInfo->pSampleMask != nullptr)
        {
            for (uint32_t index = 0; index < (uint32_t)pCreateInfo->rasterizationSamples; ++index)
            {
                file[STR_MEMBER_NAME_P_SAMPLE_MASK][index] = *(pCreateInfo->pSampleMask + index);
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

    virtual void WriteStructure(const VkStencilOpState* pCreateInfo, nlohmann::json& file) override
    {
        assert(pCreateInfo != nullptr);
        if (pCreateInfo != nullptr)
        {
            file[STR_MEMBER_NAME_FAIL_OP] = pCreateInfo->failOp;
            file[STR_MEMBER_NAME_PASS_OP] = pCreateInfo->passOp;
            file[STR_MEMBER_NAME_DEPTH_FAIL_OP] = pCreateInfo->depthFailOp;
            file[STR_MEMBER_NAME_COMPARE_OP] = pCreateInfo->compareOp;
            file[STR_MEMBER_NAME_COMPARE_MASK] = pCreateInfo->compareMask;
            file[STR_MEMBER_NAME_WRITE_MASK] = pCreateInfo->writeMask;
            file[STR_MEMBER_NAME_REFERENCE] = pCreateInfo->reference;
        }
    }

    virtual void ReadStructure(VkStencilOpState* pCreateInfo, const nlohmann::json& file) override
    {
        assert(pCreateInfo != nullptr);
        if (pCreateInfo != nullptr)
        {
            pCreateInfo->failOp = file[STR_MEMBER_NAME_FAIL_OP];
            pCreateInfo->passOp = file[STR_MEMBER_NAME_PASS_OP];
            pCreateInfo->depthFailOp = file[STR_MEMBER_NAME_DEPTH_FAIL_OP];
            pCreateInfo->compareOp = file[STR_MEMBER_NAME_COMPARE_OP];
            pCreateInfo->compareMask = file[STR_MEMBER_NAME_COMPARE_MASK];
            pCreateInfo->writeMask = file[STR_MEMBER_NAME_WRITE_MASK];
            pCreateInfo->reference = file[STR_MEMBER_NAME_REFERENCE];
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

    virtual VkSampleMask* ReadSampleMask(const VkPipelineMultisampleStateCreateInfo* pCreateInfo, const nlohmann::json& file) override
    {
        VkSampleMask* pSampleMask = nullptr;
        if (IsCreateInfoExists(file, STR_MEMBER_NAME_P_SAMPLE_MASK))
        {
            // What's the required dimension of the pSampleMask bitfield array?
            int sampleMaskArrayDimension = 0;
            switch (pCreateInfo->rasterizationSamples)
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

    virtual void WriteSampleMask(const VkPipelineMultisampleStateCreateInfo* pCreateInfo, nlohmann::json& file) override
    {
        // It's fine if pSampleMask is null- just don't write the field.
        if (pCreateInfo->pSampleMask != nullptr)
        {
            // What's the required dimension of the pSampleMask bitfield array?
            uint32_t sampleMaskArrayDimension = 0;
            switch (pCreateInfo->rasterizationSamples)
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
                file[STR_MEMBER_NAME_P_SAMPLE_MASK][index] = *(pCreateInfo->pSampleMask + index);
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

bool rgPsoSerializerVulkan::ReadStructureFromFile(const std::string& filePath, rgPsoGraphicsVulkan** ppCreateInfo, std::string& errorString)
{
    bool ret = false;
    bool shouldAbort = false;

    assert(ppCreateInfo != nullptr);
    if (ppCreateInfo != nullptr)
    {
        // Create a new PSO State structure.
        rgPsoGraphicsVulkan* pCreateInfo = new rgPsoGraphicsVulkan{};

        // Initialize the create info to assign structure pointers to internal create info members.
        pCreateInfo->Initialize();

        // Open a file to write the structure data to.
        std::ifstream fileStream;
        fileStream.open(filePath.c_str(), std::ofstream::in);

        assert(fileStream.is_open());
        if (fileStream.is_open())
        {
            // Read the JSON file.
            nlohmann::json structure;
            shouldAbort = !ReadJsonFile(fileStream, filePath, structure, errorString);

            // Close the file stream.
            fileStream.close();

            if (!shouldAbort)
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
                        if (pSerializer->ReadStructure(pCreateInfo, structure))
                        {
                            // Assign the deserialized pipeline state file to the output pointer.
                            *ppCreateInfo = pCreateInfo;

                            ret = true;
                        }
                        else
                        {
                            errorString = STR_ERR_FAILED_TO_LOAD_PIPELINE_TYPE_MISMATCH;
                        }
                    }
                    else
                    {
                        errorString = STR_ERR_FAILED_UNSUPPORTED_VERSION;
                    }
                }
                else
                {
                    errorString = STR_ERR_FAILED_TO_READ_PIPELINE_VERSION;
                }
            }
        }
        else
        {
            std::stringstream errorStream;
            errorStream << STR_ERR_FAILED_TO_READ_FILE;
            errorStream << filePath;
            errorString = errorStream.str();
        }
    }

    return ret;
}

bool rgPsoSerializerVulkan::ReadStructureFromFile(const std::string& filePath, rgPsoComputeVulkan** ppCreateInfo, std::string& errorString)
{
    bool ret = false;
    bool shouldAbort = false;

    assert(ppCreateInfo != nullptr);
    if (ppCreateInfo != nullptr)
    {
        // Create a new PSO State structure.
        rgPsoComputeVulkan* pCreateInfo = new rgPsoComputeVulkan{};

        // Initialize the create info to assign structure pointers to internal create info members.
        pCreateInfo->Initialize();

        // Open a file to write the structure data to.
        std::ifstream fileStream;
        fileStream.open(filePath.c_str(), std::ofstream::in);

        assert(fileStream.is_open());
        if (fileStream.is_open())
        {
            // Read the JSON file.
            nlohmann::json structure;
            shouldAbort = !ReadJsonFile(fileStream, filePath, structure, errorString);

            if (!shouldAbort)
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
                        if (pSerializer->ReadStructure(pCreateInfo, structure))
                        {
                            // Assign the deserialized pipeline state file to the output pointer.
                            *ppCreateInfo = pCreateInfo;
                            ret = true;
                        }
                        else
                        {
                            errorString = STR_ERR_FAILED_TO_LOAD_PIPELINE_TYPE_MISMATCH;
                        }
                    }
                    else
                    {
                        errorString = STR_ERR_FAILED_UNSUPPORTED_VERSION;
                    }
                }
                else
                {
                    errorString = STR_ERR_FAILED_TO_READ_PIPELINE_VERSION;
                }
            }
        }
        else
        {
            std::stringstream errorStream;
            errorStream << STR_ERR_FAILED_TO_READ_FILE;
            errorStream << filePath;
            errorString = errorStream.str();
        }
    }

    return ret;
}

bool rgPsoSerializerVulkan::WriteStructureToFile(rgPsoGraphicsVulkan* pCreateInfo, const std::string& filePath, std::string& errorString)
{
    bool ret = false;

    // Open a file to write the structure data to.
    std::ofstream fileStream;
    fileStream.open(filePath.c_str(), std::ofstream::out);

    if (fileStream.is_open())
    {
        nlohmann::json jsonFile;

        // Write the current pipeline version into the file.
        jsonFile[STR_PIPELINE_MODEL_VERSION] = s_CURRENT_PIPELINE_VERSION;

        // Always write the most recent version of the pipeline state file.
        std::shared_ptr<rgPsoSerializerVulkanImpl_Version_1_0> pSerializer = CreateSerializer(s_CURRENT_PIPELINE_VERSION);
        assert(pSerializer != nullptr);
        if (pSerializer != nullptr)
        {
            pSerializer->WriteStructure(pCreateInfo, jsonFile);

            // Write the JSON to disk and close the file with the given indentation.
            fileStream << jsonFile.dump(4);
            fileStream.close();

            ret = true;
        }
        else
        {
            errorString = STR_ERR_FAILED_UNSUPPORTED_VERSION;
        }
    }
    else
    {
        std::stringstream errorStream;
        errorStream << STR_ERR_FAILED_TO_WRITE_FILE;
        errorStream << filePath.c_str();
        errorString = errorStream.str();
    }

    return ret;
}

bool rgPsoSerializerVulkan::WriteStructureToFile(rgPsoComputeVulkan* pCreateInfo, const std::string& filePath, std::string& errorString)
{
    bool ret = false;

    // Open a file to write the structure data to.
    std::ofstream fileStream;
    fileStream.open(filePath.c_str(), std::ofstream::out);

    if (fileStream.is_open())
    {
        nlohmann::json jsonFile;

        // Write the current pipeline version into the file.
        jsonFile[STR_PIPELINE_MODEL_VERSION] = s_CURRENT_PIPELINE_VERSION;

        // Always write the most recent version of the pipeline state file.
        std::shared_ptr<rgPsoSerializerVulkanImpl_Version_1_0> pSerializer = CreateSerializer(s_CURRENT_PIPELINE_VERSION);
        assert(pSerializer != nullptr);
        if (pSerializer != nullptr)
        {
            pSerializer->WriteStructure(pCreateInfo, jsonFile);

            // Write the JSON to disk and close the file with the given indentation.
            fileStream << jsonFile.dump(4);
            fileStream.close();

            ret = true;
        }
        else
        {
            errorString = STR_ERR_FAILED_UNSUPPORTED_VERSION;
        }
    }
    else
    {
        std::stringstream errorStream;
        errorStream << STR_ERR_FAILED_TO_WRITE_FILE;
        errorStream << filePath.c_str();
        errorString = errorStream.str();
    }

    return ret;
}
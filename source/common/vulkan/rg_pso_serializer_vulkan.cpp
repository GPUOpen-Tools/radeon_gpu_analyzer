// C++
#include <fstream>
#include <sstream>

// Infra.
#include "json/json-3.2.0/single_include/nlohmann/json.hpp"
#include "source/common/vulkan/rg_pso_serializer_vulkan.h"

// Local.
#include "source/radeon_gpu_analyzer_gui/rg_definitions.h"

// Error string definitions.
static const char* kStrErrFailedToWriteFile = "Error: failed to open file for writing: ";
static const char* kStrErrFailedToReadFile = "Error: failed to open file for reading: ";
static const char* kStrErrFailedToLoadPipelineTypeMismatch = "The file's pipeline type does not match the project's pipeline type.";
static const char* kStrErrFailedToReadPipelineVersion = "The pipeline file version is unknown and cannot be loaded.";
static const char* kStrErrFailedUnsupportedVersion = "The pipeline file version is unrecognized.";

// A version enumeration used to specify the revision of the pipeline file schema.
enum class RgPipelineModelVersion
{
    // An unknown/unrecognized file version.
    kUnknown,

    // The initial version of the pipeline model. Majority of pipeline structures are serialized.
    kVERSION_1_0,

    // This version resolves a failure in reading & writing VkStencilOpState's 'failOp' member.
    kVERSION_1_1,

    // This version resolves a failure in reading & writing the multisampling state's VkSampleMask.
    kVERSION_1_2,
};

// This declaration is used to specify the current schema revision for the pipeline state file.
static const RgPipelineModelVersion kCurrentPipelineVersion = RgPipelineModelVersion::kVERSION_1_2;

// Pipeline CreateInfo member name string constants.
static const char* kStrPipelineModelVersion                           = "version";
static const char* kStrMemberValueTrue                                = "true";
static const char* kStrMemberValueFalse                               = "false";
static const char* kStrMemberNameType                                 = "sType";
static const char* kStrMemberNamePnext                                = "pNext";
static const char* kStrMemberNameFlags                                = "flags";
static const char* kStrMemberNamePstages                              = "pStages";
static const char* kStrMemberNameStageCount                           = "stageCount";
static const char* kStrMemberNamePVertexInputState                    = "pVertexInputState";
static const char* kStrMemberNamePVertexInputAssemblyState            = "pInputAssemblyState";
static const char* kStrMemberNamePTessellationState                   = "pTessellationState";
static const char* kStrMemberNamePViewportState                       = "pViewportState";
static const char* kStrMemberNamePRasterizationState                  = "pRasterizationState";
static const char* kStrMemberNamePMultisampleState                    = "pMultisampleState";
static const char* kStrMemberNamePDepthStencilState                   = "pDepthStencilState";
static const char* kStrMemberNamePColorBlendState                     = "pColorBlendState";
static const char* kStrMemberNamePDynamicState                        = "pDynamicState";
static const char* kStrMemberNameLayout                               = "layout";
static const char* kStrMemberNameRenderPass                           = "renderPass";
static const char* kStrMemberNameSubpass                              = "subpass";
static const char* kStrMemberNameBasePipelineHandle                   = "basePipelineHandle";
static const char* kStrMemberNameBasePipelineIndex                    = "basePipelineIndex";
static const char* kStrMemberNameModule                               = "module";
static const char* kStrMemberNameName                                 = "name";
static const char* kStrMemberNameStage                                = "stage";
static const char* kStrMemberNamePSpecializationInfo                  = "pSpecializationInfo";
static const char* kStrMemberNameVertexBindingDescriptionCount        = "vertexBindingDescriptionCount";
static const char* kStrMemberNameVertexAttributeDescriptionCount      = "vertexAttributeDescriptionCount";
static const char* kStrMemberNamePVertexBindingDescriptions           = "pVertexBindingDescriptions";
static const char* kStrMemberNamePVertexAttributeDescriptions         = "pVertexAttributeDescriptions";
static const char* kStrMemberNameTopology                             = "topology";
static const char* kStrMemberNamePrimitiveRestartEnabled              = "primitiveRestartEnable";
static const char* kStrMemberNamePatchControlPoints                   = "patchControlPoints";
static const char* kStrMemberNameViewportCount                        = "viewportCount";
static const char* kStrMemberNameScissorCount                         = "scissorCount";
static const char* kStrMemberNamePViewports                           = "pViewports";
static const char* kStrMemberNamePScissors                            = "pScissors";
static const char* kStrMemberNameDepthClampEnable                     = "depthClampEnable";
static const char* kStrMemberNameRasterizerDiscardEnable              = "rasterizerDiscardEnable";
static const char* kStrMemberNamePolygonMode                          = "polygonMode";
static const char* kStrMemberNameCullMode                             = "cullMode";
static const char* kStrMemberNameFrontFace                            = "frontFace";
static const char* kStrMemberNameDepthBiasEnable                      = "depthBiasEnable";
static const char* kStrMemberNameDepthBiasConstantFactor              = "depthBiasConstantFactor";
static const char* kStrMemberNameDepthBiasClamp                       = "depthBiasClamp";
static const char* kStrMemberNameDepthBiasSlopeFactor                 = "depthBiasSlopeFactor";
static const char* kStrMemberNameLineWidth                            = "lineWidth";
static const char* kStrMemberNameRasterizationSample                  = "rasterizationSamples";
static const char* kStrMemberNameSampleShadingEnable                  = "sampleShadingEnable";
static const char* kStrMemberNameMinSampleShading                     = "minSampleShading";
static const char* kStrMemberNamePSampleMask                          = "pSampleMask";
static const char* kStrMemberNameAlphaToCoverageEnable                = "alphaToCoverageEnable";
static const char* kStrMemberNameAlphaToOneEnable                     = "alphaToOneEnable";
static const char* kStrMemberNameFailOp                               = "failOp";
static const char* kStrMemberNamePassOp                               = "passOp";
static const char* kStrMemberNameDepthFailOp                          = "depthFailOp";
static const char* kStrMemberNameCompareOp                            = "compareOp";
static const char* kStrMemberNameCompareMask                          = "compareMask";
static const char* kStrMemberNameWriteMask                            = "writeMask";
static const char* kStrMemberNameReference                            = "reference";
static const char* kStrMemberNameDepthTestEnable                      = "depthTestEnable";
static const char* kStrMemberNameDepthWriteEnable                     = "depthWriteEnable";
static const char* kStrMemberNameDepthCompareOp                       = "depthCompareOp";
static const char* kStrMemberNameDepthBoundsTestEnable                = "depthBoundsTestEnable";
static const char* kStrMemberNameStencilTestEnable                    = "stencilTestEnable";
static const char* kStrMemberNameFront                                = "front";
static const char* kStrMemberNameBack                                 = "back";
static const char* kStrMemberNameMinDepth_BOUNDS                      = "minDepthBounds";
static const char* kStrMemberNameMaxDepthBounds                       = "maxDepthBounds";
static const char* kStrMemberNameBlendEnable                          = "blendEnable";
static const char* kStrMemberNameSrcColorBlendFactor                  = "srcColorBlendFactor";
static const char* kStrMemberNameDstColorBlendFactor                  = "dstColorBlendFactor";
static const char* kStrMemberNameColorBlendOp                         = "colorBlendOp";
static const char* kStrMemberNameSrcAlphaBlendFactor                  = "srcAlphaBlendFactor";
static const char* kStrMemberNameDstAlphaBlendFactor                  = "dstAlphaBlendFactor";
static const char* kStrMemberNameAlphaBlendOp                         = "alphaBlendOp";
static const char* kStrMemberNameColorWriteMask                       = "colorWriteMask";
static const char* kStrMemberNameLogicOpEnable                        = "logicOpEnable";
static const char* kStrMemberNameLogicOp                              = "logicOp";
static const char* kStrMemberNameAttachmentCount                      = "attachmentCount";
static const char* kStrMemberNamePAttachments                         = "pAttachments";
static const char* kStrMemberNameBlendConstants                       = "blendConstants";
static const char* kStrMemberNameDynamicStateCount                    = "dynamicStateCount";
static const char* kStrMemberNamePDynamicStateS                       = "pDynamicStates";
static const char* kStrMemberNameMapEntryCount                        = "mapEntryCount";
static const char* kStrMemberNamePMapEntries                          = "pMapEntries";
static const char* kStrMemberNameDataSize                             = "dataSize";
static const char* kStrMemberNamePData                                = "pData";
static const char* kStrMemberNameBinding                              = "binding";
static const char* kStrMemberNameStride                               = "stride";
static const char* kStrMemberNameInputRate                            = "inputRate";
static const char* kStrMemberNameConstantId                           = "constantID";
static const char* kStrMemberNameOffset                               = "offset";
static const char* kStrMemberNameSize                                 = "size";
static const char* kStrMemberNameLocation                             = "location";
static const char* kStrMemberNameFormat                               = "format";
static const char* kStrMemberNameX                                    = "x";
static const char* kStrMemberNameY                                    = "y";
static const char* kStrMemberNameWidth                                = "width";
static const char* kStrMemberNameHeight                               = "height";
static const char* kStrMemberNameMinDepth                             = "minDepth";
static const char* kStrMemberNameMaxDepth                             = "maxDepth";
static const char* kStrMemberNameExtent                               = "extent";
static const char* kStrMemberNameSetLayoutCount                       = "setLayoutCount";
static const char* kStrMemberNamePSetLayouts                          = "pSetLayouts";
static const char* kStrMemberNamePushConstantRangeCount               = "pushConstantRangeCount";
static const char* kStrMemberNamePPushConstantRanges                  = "pPushConstantRanges";
static const char* kStrMemberNameCodeSize                             = "codeSize";
static const char* kStrMemberNameBindingCount                         = "bindingCount";
static const char* kStrMemberNamePBindings                            = "pBindings";
static const char* kStrMemberNameDescriptorType                       = "descriptorType";
static const char* kStrMemberNameDescriptorCount                      = "descriptorCount";
static const char* kStrMemberNameStageFlags                           = "stageFlags";
static const char* kStrMemberNameSubpassCount                         = "subpassCount";
static const char* kStrMemberNamePSubpasses                           = "pSubpasses";
static const char* kStrMemberNameDependencyCount                      = "dependencyCount";
static const char* kStrMemberNamePDependencies                        = "pDependencies";
static const char* kStrMemberNameSamples                              = "samples";
static const char* kStrMemberNameLoadOp                               = "loadOp";
static const char* kStrMemberNameStoreOp                              = "storeOp";
static const char* kStrMemberNameStencilLoadOp                        = "stencilLoadOp";
static const char* kStrMemberNameStencilStoreOp                       = "stencilStoreOp";
static const char* kStrMemberNameInitialLayout                        = "initialLayout";
static const char* kStrMemberNameFinalLayout                          = "finalLayout";
static const char* kStrMemberNamePipelineBindPoint                    = "pipelineBindPoint";
static const char* kStrMemberNameInputAttachmentCount                 = "inputAttachmentCount";
static const char* kStrMemberNamePInputAttachments                    = "pInputAttachments";
static const char* kStrMemberNameColorAttachmentCount                 = "colorAttachmentCount";
static const char* kStrMemberNamePColorAttachments                    = "pColorAttachments";
static const char* kStrMemberNamePResolveAttachments                  = "pResolveAttachments";
static const char* kStrMemberNamePDepthStencilAttachment              = "pDepthStencilAttachment";
static const char* kStrMemberNamePreserveAttachmentCount              = "preserveAttachmentCount";
static const char* kStrMemberNamePPreserveAttachments                 = "pPreserveAttachments";
static const char* kStrMemberNameSrcSubpass                           = "srcSubpass";
static const char* kStrMemberNameDstSubpass                           = "dstSubpass";
static const char* kStrMemberNameSrcStageMask                         = "srcStageMask";
static const char* kStrMemberNameDstStageMask                         = "dstStageMask";
static const char* kStrMemberNameSrcAccessMask                        = "srcAccessMask";
static const char* kStrMemberNameDstAccessMask                        = "dstAccessMask";
static const char* kStrMemberNameDependencyFlags                      = "dependencyFlags";
static const char* kStrMemberNameAttachment                           = "attachment";
static const char* kStrMemberNameVkGraphicsPipelineCreateInfo         = "VkGraphicsPipelineCreateInfo";
static const char* kStrMemberNameVkRenderPassCreateInfo               = "VkRenderPassCreateInfo";
static const char* kStrMemberNameVkPipelineLayoutCreateInfo           = "VkPipelineLayoutCreateInfo";
static const char* kStrMemberNameVkDescriptorSetLayoutCreateInfo      = "VkDescriptorSetLayoutCreateInfo";
static const char* kStrMemberNameVkComputePipelineCreateInfo          = "VkComputePipelineCreateInfo";

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
        error_stream << kStrErrFailedToReadFile;
        error_stream << file_path;
        error_string = error_stream.str();
    }

    return ret;
}

static VkBool32 ReadBool(const nlohmann::json& file)
{
    bool is_true = (file == kStrMemberValueTrue);
    return is_true ? VK_TRUE : VK_FALSE;
}

static const char* WriteBool(VkBool32 val)
{
    return (val == VK_TRUE) ? kStrMemberValueTrue : kStrMemberValueFalse;
}

static void* ReadHandle(const nlohmann::json& file)
{
    std::string handle_hex_string = file;
    handle_hex_string = handle_hex_string.substr(2);

    size_t addr = std::strtoull(handle_hex_string.c_str(), nullptr, 16);
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

    void ReadGraphicsPipelineCreateInfoStructure(RgPsoGraphicsVulkan* create_info, const nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            VkGraphicsPipelineCreateInfo* graphics_pipeline_create_info = create_info->GetGraphicsPipelineCreateInfo();
            assert(graphics_pipeline_create_info != nullptr);
            if (graphics_pipeline_create_info != nullptr)
            {
                graphics_pipeline_create_info->sType = file[kStrMemberNameType];
                graphics_pipeline_create_info->pNext = ReadHandle(file[kStrMemberNamePnext]);
                graphics_pipeline_create_info->flags = file[kStrMemberNameFlags];
                graphics_pipeline_create_info->stageCount = file[kStrMemberNameStageCount];

                VkPipelineShaderStageCreateInfo* shader_stages = nullptr;
                if (graphics_pipeline_create_info->stageCount > 0)
                {
                    shader_stages = new VkPipelineShaderStageCreateInfo[graphics_pipeline_create_info->stageCount]{};
                    for (uint32_t stage_index = 0; stage_index < graphics_pipeline_create_info->stageCount; ++stage_index)
                    {
                        ReadStructure(shader_stages + stage_index, file[kStrMemberNamePstages][stage_index]);
                    }
                }
                graphics_pipeline_create_info->pStages = shader_stages;

                VkPipelineVertexInputStateCreateInfo* vertex_input_state = nullptr;
                if (IsCreateInfoExists(file, kStrMemberNamePVertexInputState))
                {
                    vertex_input_state = create_info->GetPipelineVertexInputStateCreateInfo();
                    ReadStructure(vertex_input_state, file[kStrMemberNamePVertexInputState]);
                }

                VkPipelineInputAssemblyStateCreateInfo* vertex_input_assembly_state = nullptr;
                if (IsCreateInfoExists(file, kStrMemberNamePVertexInputAssemblyState))
                {
                    vertex_input_assembly_state = create_info->GetPipelineInputAssemblyStateCreateInfo();
                    ReadStructure(vertex_input_assembly_state, file[kStrMemberNamePVertexInputAssemblyState]);
                }

                VkPipelineTessellationStateCreateInfo* tesselation_state = nullptr;
                if (IsCreateInfoExists(file, kStrMemberNamePTessellationState))
                {
                    tesselation_state = create_info->GetPipelineTessellationStateCreateInfo();
                    ReadStructure(tesselation_state, file[kStrMemberNamePTessellationState]);
                }

                VkPipelineViewportStateCreateInfo* viewport_state = nullptr;
                if (IsCreateInfoExists(file, kStrMemberNamePViewportState))
                {
                    viewport_state = create_info->GetPipelineViewportStateCreateInfo();
                    ReadStructure(viewport_state, file[kStrMemberNamePViewportState]);
                }

                VkPipelineRasterizationStateCreateInfo* rasterization_state = nullptr;
                if (IsCreateInfoExists(file, kStrMemberNamePRasterizationState))
                {
                    rasterization_state = create_info->GetPipelineRasterizationStateCreateInfo();
                    ReadStructure(rasterization_state, file[kStrMemberNamePRasterizationState]);
                }

                VkPipelineMultisampleStateCreateInfo* multisample_state = nullptr;
                if (IsCreateInfoExists(file, kStrMemberNamePMultisampleState))
                {
                    multisample_state = create_info->GetPipelineMultisampleStateCreateInfo();
                    ReadStructure(multisample_state, file[kStrMemberNamePMultisampleState]);
                }

                VkPipelineDepthStencilStateCreateInfo* depth_stencil_state = nullptr;
                if (IsCreateInfoExists(file, kStrMemberNamePDepthStencilState))
                {
                    depth_stencil_state = create_info->GetPipelineDepthStencilStateCreateInfo();
                    ReadStructure(depth_stencil_state, file[kStrMemberNamePDepthStencilState]);
                }

                VkPipelineColorBlendStateCreateInfo* color_blend_state = nullptr;
                if (IsCreateInfoExists(file, kStrMemberNamePColorBlendState))
                {
                    color_blend_state = create_info->GetPipelineColorBlendStateCreateInfo();
                    ReadStructure(color_blend_state, file[kStrMemberNamePColorBlendState]);
                }

                if (IsCreateInfoExists(file, kStrMemberNameLayout))
                {
                    graphics_pipeline_create_info->layout = reinterpret_cast<VkPipelineLayout>(ReadHandle(file[kStrMemberNameLayout]));
                }

                if (IsCreateInfoExists(file, kStrMemberNameRenderPass))
                {
                    graphics_pipeline_create_info->renderPass = reinterpret_cast<VkRenderPass>(ReadHandle(file[kStrMemberNameRenderPass]));
                }

                graphics_pipeline_create_info->subpass = file[kStrMemberNameSubpass];

                if (IsCreateInfoExists(file, kStrMemberNameBasePipelineHandle))
                {
                    graphics_pipeline_create_info->basePipelineHandle = reinterpret_cast<VkPipeline>(ReadHandle(file[kStrMemberNameBasePipelineHandle]));
                }

                graphics_pipeline_create_info->basePipelineIndex = file[kStrMemberNameBasePipelineIndex];
            }
        }
    }

    void WriteStructure(const VkGraphicsPipelineCreateInfo* create_info, nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            file[kStrMemberNameType] = create_info->sType;
            file[kStrMemberNamePnext] = WriteHandle(reinterpret_cast<uint64_t>(create_info->pNext));
            file[kStrMemberNameFlags] = create_info->flags;
            file[kStrMemberNameStageCount] = create_info->stageCount;

            for (uint32_t index = 0; index < create_info->stageCount; ++index)
            {
                WriteStructure((create_info->pStages + index), file[kStrMemberNamePstages][index]);
            }

            if (create_info->pVertexInputState != nullptr)
            {
                WriteStructure(create_info->pVertexInputState, file[kStrMemberNamePVertexInputState]);
            }

            if (create_info->pInputAssemblyState != nullptr)
            {
                WriteStructure(create_info->pInputAssemblyState, file[kStrMemberNamePVertexInputAssemblyState]);
            }

            if (create_info->pTessellationState != nullptr)
            {
                WriteStructure(create_info->pTessellationState, file[kStrMemberNamePTessellationState]);
            }

            if (create_info->pViewportState != nullptr)
            {
                WriteStructure(create_info->pViewportState, file[kStrMemberNamePViewportState]);
            }

            if (create_info->pRasterizationState != nullptr)
            {
                WriteStructure(create_info->pRasterizationState, file[kStrMemberNamePRasterizationState]);
            }

            if (create_info->pMultisampleState != nullptr)
            {
                WriteStructure(create_info->pMultisampleState, file[kStrMemberNamePMultisampleState]);
            }

            if (create_info->pDepthStencilState != nullptr)
            {
                WriteStructure(create_info->pDepthStencilState, file[kStrMemberNamePDepthStencilState]);
            }

            if (create_info->pColorBlendState != nullptr)
            {
                WriteStructure(create_info->pColorBlendState, file[kStrMemberNamePColorBlendState]);
            }

            if (create_info->pDynamicState != nullptr)
            {
                WriteStructure(create_info->pDynamicState, file[kStrMemberNamePDynamicState]);
            }

            if (create_info->layout != VK_NULL_HANDLE)
            {
                file[kStrMemberNameLayout] = WriteHandle(reinterpret_cast<uint64_t>(create_info->layout));
            }

            if (create_info->renderPass != VK_NULL_HANDLE)
            {
                file[kStrMemberNameRenderPass] = WriteHandle(reinterpret_cast<uint64_t>(create_info->renderPass));
            }
            file[kStrMemberNameSubpass] = create_info->subpass;
            if (create_info->basePipelineHandle != VK_NULL_HANDLE)
            {
                file[kStrMemberNameBasePipelineHandle] = WriteHandle(reinterpret_cast<uint64_t>(create_info->basePipelineHandle));
            }
            file[kStrMemberNameBasePipelineIndex] = create_info->basePipelineIndex;
        }
    }

    void WriteStructure(const VkPipelineShaderStageCreateInfo* create_info, nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            file[kStrMemberNameType] = create_info->sType;
            file[kStrMemberNamePnext] = WriteHandle(reinterpret_cast<uint64_t>(create_info->pNext));
            file[kStrMemberNameFlags] = create_info->flags;
            file[kStrMemberNameStage] = create_info->stage;
            file[kStrMemberNameModule] = WriteHandle(reinterpret_cast<uint64_t>(create_info->module));

            if (create_info->pName != nullptr)
            {
                file[kStrMemberNameName] = std::string(create_info->pName);
            }

            if (create_info->pSpecializationInfo != nullptr)
            {
                WriteStructure(create_info->pSpecializationInfo, file[kStrMemberNamePSpecializationInfo]);
            }
        }
    }

    void ReadStructure(VkPipelineShaderStageCreateInfo* create_info, const nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            create_info->sType = file[kStrMemberNameType];
            create_info->pNext = ReadHandle(file[kStrMemberNamePnext]);
            create_info->flags = file[kStrMemberNameFlags];
            create_info->stage = file[kStrMemberNameStage];
            create_info->module = (VkShaderModule)ReadHandle(file[kStrMemberNameModule]);
            if (IsCreateInfoExists(file, kStrMemberNameName))
            {
                std::string entrypoint_name = file[kStrMemberNameName];
                size_t buffer_size = entrypoint_name.length() + 1;
                char* entrypoint_name_string = new char[buffer_size] {};
                STRCPY(entrypoint_name_string, buffer_size, entrypoint_name.c_str());
                create_info->pName = entrypoint_name_string;
            }

            if (IsCreateInfoExists(file, kStrMemberNamePSpecializationInfo))
            {
                VkSpecializationInfo* info = new VkSpecializationInfo{};
                ReadStructure(info, file[kStrMemberNamePSpecializationInfo]);
                create_info->pSpecializationInfo = info;
            }
        }
    }

    std::string WriteHandle(uint64_t handle)
    {
        const size_t buffer_size = 1024;
        char buffer[buffer_size];
        snprintf(buffer, buffer_size, "0x%p", (void*)handle);
        return std::string(buffer);
    }

    void WriteStructure(const VkPipelineVertexInputStateCreateInfo* create_info, nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            file[kStrMemberNameType] = create_info->sType;
            file[kStrMemberNamePnext] = WriteHandle(reinterpret_cast<uint64_t>(create_info->pNext));
            file[kStrMemberNameFlags] = create_info->flags;
            file[kStrMemberNameVertexBindingDescriptionCount] = create_info->vertexBindingDescriptionCount;
            file[kStrMemberNameVertexAttributeDescriptionCount] = create_info->vertexAttributeDescriptionCount;
            for (uint32_t index = 0; index < create_info->vertexBindingDescriptionCount; ++index)
            {
                WriteStructure(create_info->pVertexBindingDescriptions + index, file[kStrMemberNamePVertexBindingDescriptions][index]);
            }
            for (uint32_t index = 0; index < create_info->vertexAttributeDescriptionCount; ++index)
            {
                WriteStructure(create_info->pVertexAttributeDescriptions + index, file[kStrMemberNamePVertexAttributeDescriptions][index]);
            }
        }
    }

    void ReadStructure(VkPipelineVertexInputStateCreateInfo* create_info, const nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            create_info->sType = file[kStrMemberNameType];
            create_info->pNext = ReadHandle(file[kStrMemberNamePnext]);
            create_info->flags = file[kStrMemberNameFlags];
            create_info->vertexBindingDescriptionCount = file[kStrMemberNameVertexBindingDescriptionCount];

            VkVertexInputBindingDescription* vertex_binding_descriptions = nullptr;
            if (create_info->vertexBindingDescriptionCount > 0)
            {
                vertex_binding_descriptions = new VkVertexInputBindingDescription[create_info->vertexBindingDescriptionCount]{};
                for (uint32_t binding_index = 0; binding_index < create_info->vertexBindingDescriptionCount; ++binding_index)
                {
                    ReadStructure(vertex_binding_descriptions + binding_index, file[kStrMemberNamePVertexBindingDescriptions][binding_index]);
                }
            }
            create_info->pVertexBindingDescriptions = vertex_binding_descriptions;

            create_info->vertexAttributeDescriptionCount = file[kStrMemberNameVertexAttributeDescriptionCount];

            VkVertexInputAttributeDescription* vertex_attribute_descriptions = nullptr;
            if (create_info->vertexAttributeDescriptionCount > 0)
            {
                vertex_attribute_descriptions = new VkVertexInputAttributeDescription[create_info->vertexAttributeDescriptionCount]{};
                for (uint32_t attribute_index = 0; attribute_index < create_info->vertexAttributeDescriptionCount; ++attribute_index)
                {
                    ReadStructure(vertex_attribute_descriptions + attribute_index, file[kStrMemberNamePVertexAttributeDescriptions][attribute_index]);
                }
            }
            create_info->pVertexAttributeDescriptions = vertex_attribute_descriptions;
        }
    }

    void WriteStructure(const VkPipelineInputAssemblyStateCreateInfo* create_info, nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            file[kStrMemberNameType] = create_info->sType;
            file[kStrMemberNamePnext] = WriteHandle(reinterpret_cast<uint64_t>(create_info->pNext));
            file[kStrMemberNameFlags] = create_info->flags;
            file[kStrMemberNameTopology] = create_info->topology;
            file[kStrMemberNamePrimitiveRestartEnabled] = WriteBool(create_info->primitiveRestartEnable);
        }
    }

    void ReadStructure(VkPipelineInputAssemblyStateCreateInfo* create_info, const nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            create_info->sType = file[kStrMemberNameType];
            create_info->pNext = ReadHandle(file[kStrMemberNamePnext]);
            create_info->flags = file[kStrMemberNameFlags];
            create_info->topology = file[kStrMemberNameTopology];
            create_info->primitiveRestartEnable = ReadBool(file[kStrMemberNamePrimitiveRestartEnabled]);
        }
    }

    void WriteStructure(const VkPipelineTessellationStateCreateInfo* create_info, nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            file[kStrMemberNameType] = create_info->sType;
            file[kStrMemberNamePnext] = WriteHandle(reinterpret_cast<uint64_t>(create_info->pNext));
            file[kStrMemberNameFlags] = create_info->flags;
            file[kStrMemberNamePatchControlPoints] = create_info->patchControlPoints;
        }
    }

    void ReadStructure(VkPipelineTessellationStateCreateInfo* create_info, const nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            create_info->sType = file[kStrMemberNameType];
            create_info->pNext = ReadHandle(file[kStrMemberNamePnext]);
            create_info->flags = file[kStrMemberNameFlags];
            create_info->patchControlPoints = file[kStrMemberNamePatchControlPoints];
        }
    }

    void WriteStructure(const VkPipelineViewportStateCreateInfo* create_info, nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            file[kStrMemberNameType] = create_info->sType;
            file[kStrMemberNamePnext] = WriteHandle(reinterpret_cast<uint64_t>(create_info->pNext));
            file[kStrMemberNameFlags] = create_info->flags;
            file[kStrMemberNameViewportCount] = create_info->viewportCount;
            file[kStrMemberNameScissorCount] = create_info->scissorCount;
            if (create_info->pViewports != nullptr)
            {
                for (uint32_t index = 0; index < create_info->viewportCount; ++index)
                {
                    WriteStructure(create_info->pViewports + index, file[kStrMemberNamePViewports][index]);
                }
            }

            if (create_info->pScissors != nullptr)
            {
                for (uint32_t index = 0; index < create_info->scissorCount; ++index)
                {
                    WriteStructure(create_info->pScissors + index, file[kStrMemberNamePScissors][index]);
                }
            }
        }
    }

    void ReadStructure(VkPipelineViewportStateCreateInfo* create_info, const nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            create_info->sType = file[kStrMemberNameType];
            create_info->pNext = ReadHandle(file[kStrMemberNamePnext]);
            create_info->flags = file[kStrMemberNameFlags];

            // Parse the array of viewport rectangles.
            create_info->viewportCount = file[kStrMemberNameViewportCount];
            VkViewport* viewports = nullptr;
            if (IsCreateInfoExists(file, kStrMemberNamePViewports))
            {
                if (create_info->viewportCount > 0)
                {
                    viewports = new VkViewport[create_info->viewportCount]{};
                    for (uint32_t index = 0; index < create_info->viewportCount; ++index)
                    {
                        ReadStructure(viewports + index, file[kStrMemberNamePViewports][index]);
                    }
                }
            }
            else
            {
                // In case that there is no viewport given, we would allocate a default one.
                create_info->viewportCount = 1;
                viewports = new VkViewport{};
                viewports->height = 1080;
                viewports->width = 1920;
                viewports->maxDepth = 1;
            }

            // Set the viewport in the create info structure.
            create_info->pViewports = viewports;

            // Parse the array of scissor rectangles.
            create_info->scissorCount = file[kStrMemberNameScissorCount];
            VkRect2D* scissors = nullptr;
            if (IsCreateInfoExists(file, kStrMemberNamePScissors))
            {
                if (create_info->scissorCount > 0)
                {
                    scissors = new VkRect2D[create_info->scissorCount]{};
                    for (uint32_t index = 0; index < create_info->scissorCount; ++index)
                    {
                        ReadStructure(scissors + index, file[kStrMemberNamePScissors][index]);
                    }
                }
            }
            else
            {
                // In case that there is no scissor given, we would allocate a default one.
                scissors = new VkRect2D[1]{};
                create_info->scissorCount = 1;
                scissors->extent.height = 1080;
                scissors->extent.width = 1920;
            }

            // Set the scissors in the create info structure.
            create_info->pScissors = scissors;
        }
    }

    void WriteStructure(const VkPipelineRasterizationStateCreateInfo* create_info, nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            file[kStrMemberNameType] = create_info->sType;
            file[kStrMemberNamePnext] = WriteHandle(reinterpret_cast<uint64_t>(create_info->pNext));
            file[kStrMemberNameFlags] = create_info->flags;
            file[kStrMemberNameDepthClampEnable] = WriteBool(create_info->depthClampEnable);
            file[kStrMemberNameRasterizerDiscardEnable] = WriteBool(create_info->rasterizerDiscardEnable);
            file[kStrMemberNamePolygonMode] = create_info->polygonMode;
            file[kStrMemberNameCullMode] = create_info->cullMode;
            file[kStrMemberNameFrontFace] = create_info->frontFace;
            file[kStrMemberNameDepthBiasEnable] = WriteBool(create_info->depthBiasEnable);
            file[kStrMemberNameDepthBiasConstantFactor] = create_info->depthBiasConstantFactor;
            file[kStrMemberNameDepthBiasClamp] = create_info->depthBiasClamp;
            file[kStrMemberNameDepthBiasSlopeFactor] = create_info->depthBiasSlopeFactor;
            file[kStrMemberNameLineWidth] = create_info->lineWidth;
        }
    }

    void ReadStructure(VkPipelineRasterizationStateCreateInfo* create_info, const nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            create_info->sType = file[kStrMemberNameType];
            create_info->pNext = ReadHandle(file[kStrMemberNamePnext]);
            create_info->flags = file[kStrMemberNameFlags];
            create_info->depthClampEnable = ReadBool(file[kStrMemberNameDepthClampEnable]);
            create_info->rasterizerDiscardEnable = ReadBool(file[kStrMemberNameRasterizerDiscardEnable]);
            create_info->polygonMode = file[kStrMemberNamePolygonMode];
            create_info->cullMode = file[kStrMemberNameCullMode];
            create_info->frontFace = file[kStrMemberNameFrontFace];
            create_info->depthBiasEnable = ReadBool(file[kStrMemberNameDepthBiasEnable]);
            create_info->depthBiasConstantFactor = file[kStrMemberNameDepthBiasConstantFactor];
            create_info->depthBiasClamp = file[kStrMemberNameDepthBiasClamp];
            create_info->depthBiasSlopeFactor = file[kStrMemberNameDepthBiasSlopeFactor];
            create_info->lineWidth = file[kStrMemberNameLineWidth];
        }
    }

    void WriteStructure(const VkPipelineMultisampleStateCreateInfo* create_info, nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            file[kStrMemberNameType] = create_info->sType;
            file[kStrMemberNamePnext] = WriteHandle(reinterpret_cast<uint64_t>(create_info->pNext));
            file[kStrMemberNameFlags] = create_info->flags;
            file[kStrMemberNameRasterizationSample] = create_info->rasterizationSamples;
            file[kStrMemberNameSampleShadingEnable] = WriteBool(create_info->sampleShadingEnable);
            file[kStrMemberNameMinSampleShading] = create_info->minSampleShading;
            WriteSampleMask(create_info, file);
            file[kStrMemberNameAlphaToCoverageEnable] = WriteBool(create_info->alphaToCoverageEnable);
            file[kStrMemberNameAlphaToOneEnable] = WriteBool(create_info->alphaToOneEnable);
        }
    }

    void ReadStructure(VkPipelineMultisampleStateCreateInfo* create_info, const nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            create_info->sType = file[kStrMemberNameType];
            create_info->pNext = ReadHandle(file[kStrMemberNamePnext]);
            create_info->flags = file[kStrMemberNameFlags];
            create_info->rasterizationSamples = file[kStrMemberNameRasterizationSample];
            create_info->sampleShadingEnable = ReadBool(file[kStrMemberNameSampleShadingEnable]);
            create_info->minSampleShading = file[kStrMemberNameMinSampleShading];
            create_info->pSampleMask = ReadSampleMask(create_info, file);
            create_info->alphaToCoverageEnable = ReadBool(file[kStrMemberNameAlphaToCoverageEnable]);
            create_info->alphaToOneEnable = ReadBool(file[kStrMemberNameAlphaToOneEnable]);
        }
    }

    virtual void WriteStructure(const VkStencilOpState* create_info, nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            file[kStrMemberNamePassOp] = create_info->passOp;
            file[kStrMemberNameDepthFailOp] = create_info->depthFailOp;
            file[kStrMemberNameCompareOp] = create_info->compareOp;
            file[kStrMemberNameCompareMask] = create_info->compareMask;
            file[kStrMemberNameWriteMask] = create_info->writeMask;
            file[kStrMemberNameReference] = create_info->reference;
        }
    }

    virtual void ReadStructure(VkStencilOpState* create_info, const nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            create_info->passOp = file[kStrMemberNamePassOp];
            create_info->depthFailOp = file[kStrMemberNameDepthFailOp];
            create_info->compareOp = file[kStrMemberNameCompareOp];
            create_info->compareMask = file[kStrMemberNameCompareMask];
            create_info->writeMask = file[kStrMemberNameWriteMask];
            create_info->reference = file[kStrMemberNameReference];
        }
    }

    void WriteStructure(const VkPipelineDepthStencilStateCreateInfo* create_info, nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            file[kStrMemberNameType] = create_info->sType;
            file[kStrMemberNamePnext] = WriteHandle(reinterpret_cast<uint64_t>(create_info->pNext));
            file[kStrMemberNameFlags] = create_info->flags;
            file[kStrMemberNameDepthTestEnable] = WriteBool(create_info->depthTestEnable);
            file[kStrMemberNameDepthWriteEnable] = WriteBool(create_info->depthWriteEnable);
            file[kStrMemberNameDepthCompareOp] = create_info->depthCompareOp;
            file[kStrMemberNameDepthBoundsTestEnable] = WriteBool(create_info->depthBoundsTestEnable);
            file[kStrMemberNameStencilTestEnable] = WriteBool(create_info->stencilTestEnable);
            WriteStructure(&create_info->front, file[kStrMemberNameFront]);
            WriteStructure(&create_info->back, file[kStrMemberNameBack]);
            file[kStrMemberNameMinDepth_BOUNDS] = create_info->minDepthBounds;
            file[kStrMemberNameMaxDepthBounds] = create_info->maxDepthBounds;
        }
    }

    void ReadStructure(VkPipelineDepthStencilStateCreateInfo* create_info, const nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            create_info->sType = file[kStrMemberNameType];
            create_info->pNext = ReadHandle(file[kStrMemberNamePnext]);
            create_info->flags = file[kStrMemberNameFlags];
            create_info->depthTestEnable = ReadBool(file[kStrMemberNameDepthTestEnable]);
            create_info->depthWriteEnable = ReadBool(file[kStrMemberNameDepthWriteEnable]);
            create_info->depthCompareOp = file[kStrMemberNameDepthCompareOp];

            create_info->depthBoundsTestEnable = ReadBool(file[kStrMemberNameDepthBoundsTestEnable]);
            create_info->stencilTestEnable = ReadBool(file[kStrMemberNameStencilTestEnable]);
            ReadStructure(&create_info->front, file[kStrMemberNameFront]);
            ReadStructure(&create_info->back, file[kStrMemberNameBack]);
            create_info->minDepthBounds = file[kStrMemberNameMinDepth_BOUNDS];
            create_info->maxDepthBounds = file[kStrMemberNameMaxDepthBounds];
        }
    }

    void WriteStructure(const VkPipelineColorBlendAttachmentState* create_info, nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            file[kStrMemberNameBlendEnable] = WriteBool(create_info->blendEnable);
            file[kStrMemberNameSrcColorBlendFactor] = create_info->srcColorBlendFactor;
            file[kStrMemberNameDstColorBlendFactor] = create_info->dstColorBlendFactor;
            file[kStrMemberNameColorBlendOp] = create_info->colorBlendOp;
            file[kStrMemberNameSrcAlphaBlendFactor] = create_info->srcAlphaBlendFactor;
            file[kStrMemberNameDstAlphaBlendFactor] = create_info->dstAlphaBlendFactor;
            file[kStrMemberNameAlphaBlendOp] = create_info->alphaBlendOp;
            file[kStrMemberNameColorWriteMask] = create_info->colorWriteMask;
        }
    }

    void ReadStructure(VkPipelineColorBlendAttachmentState* create_info, const nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            create_info->blendEnable = ReadBool(file[kStrMemberNameBlendEnable]);
            create_info->srcColorBlendFactor = file[kStrMemberNameSrcColorBlendFactor];
            create_info->dstColorBlendFactor = file[kStrMemberNameDstColorBlendFactor];
            create_info->colorBlendOp = file[kStrMemberNameColorBlendOp];
            create_info->srcAlphaBlendFactor = file[kStrMemberNameSrcAlphaBlendFactor];
            create_info->dstAlphaBlendFactor = file[kStrMemberNameDstAlphaBlendFactor];
            create_info->alphaBlendOp = file[kStrMemberNameAlphaBlendOp];
            create_info->colorWriteMask = file[kStrMemberNameColorWriteMask];
        }
    }

    void WriteStructure(const VkPipelineColorBlendStateCreateInfo* create_info, nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            file[kStrMemberNameType] = create_info->sType;
            file[kStrMemberNamePnext] = WriteHandle(reinterpret_cast<uint64_t>(create_info->pNext));
            file[kStrMemberNameFlags] = create_info->flags;
            file[kStrMemberNameLogicOpEnable] = WriteBool(create_info->logicOpEnable);
            file[kStrMemberNameLogicOp] = create_info->logicOp;
            file[kStrMemberNameAttachmentCount] = create_info->attachmentCount;
            for (uint32_t attachment_index = 0; attachment_index < create_info->attachmentCount; attachment_index++)
            {
                WriteStructure(create_info->pAttachments + attachment_index, file[kStrMemberNamePAttachments][attachment_index]);
            }
            for (uint32_t constant_index = 0; constant_index < 4; constant_index++)
            {
                file[kStrMemberNameBlendConstants][constant_index] = create_info->blendConstants[constant_index];
            }
        }
    }

    void ReadStructure(VkPipelineColorBlendStateCreateInfo* create_info, const nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            create_info->sType = file[kStrMemberNameType];
            create_info->pNext = ReadHandle(file[kStrMemberNamePnext]);
            create_info->flags = file[kStrMemberNameFlags];
            create_info->logicOpEnable = ReadBool(file[kStrMemberNameLogicOpEnable]);
            create_info->logicOp = file[kStrMemberNameLogicOp];
            create_info->attachmentCount = file[kStrMemberNameAttachmentCount];

            VkPipelineColorBlendAttachmentState* attachments = nullptr;
            if (create_info->attachmentCount > 0)
            {
                attachments = new VkPipelineColorBlendAttachmentState[create_info->attachmentCount]{};
                for (uint32_t attachment_index = 0; attachment_index < create_info->attachmentCount; attachment_index++)
                {
                    ReadStructure(attachments + attachment_index, file[kStrMemberNamePAttachments][attachment_index]);
                }
            }
            create_info->pAttachments = attachments;

            for (uint32_t constant_index = 0; constant_index < 4; constant_index++)
            {
                create_info->blendConstants[constant_index] = file[kStrMemberNameBlendConstants][constant_index];
            }
        }
    }

    void WriteStructure(const VkPipelineDynamicStateCreateInfo* create_info, nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            file[kStrMemberNameType] = create_info->sType;
            file[kStrMemberNamePnext] = WriteHandle(reinterpret_cast<uint64_t>(create_info->pNext));
            file[kStrMemberNameFlags] = create_info->flags;
            file[kStrMemberNameDynamicStateCount] = create_info->dynamicStateCount;

            for (uint32_t state_index = 0; state_index < create_info->dynamicStateCount; ++state_index)
            {
                file[kStrMemberNamePDynamicStateS][state_index] = *(create_info->pDynamicStates + state_index);
            }
        }
    }

    void ReadStructure(VkPipelineDynamicStateCreateInfo* create_info, const nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            create_info->sType = file[kStrMemberNameType];
            create_info->pNext = ReadHandle(file[kStrMemberNamePnext]);
            create_info->flags = file[kStrMemberNameFlags];
            create_info->dynamicStateCount = file[kStrMemberNameDynamicStateCount];

            VkDynamicState* dynamic_states = nullptr;
            if (create_info->dynamicStateCount > 0)
            {
                dynamic_states = new VkDynamicState[create_info->dynamicStateCount]{};
                for (uint32_t stateIndex = 0; stateIndex < create_info->dynamicStateCount; ++stateIndex)
                {
                    dynamic_states[stateIndex] = file[kStrMemberNamePDynamicStateS][stateIndex];
                }
            }
            create_info->pDynamicStates = dynamic_states;
        }
    }

    void WriteStructure(const VkSpecializationInfo* create_info, nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            file[kStrMemberNameMapEntryCount] = create_info->mapEntryCount;
            for (uint32_t index = 0; index < create_info->mapEntryCount; ++index)
            {
                WriteStructure((create_info->pMapEntries + index), file[kStrMemberNamePMapEntries][index]);
            }
            file[kStrMemberNameDataSize] = create_info->dataSize;

            // create_info->dataSize is the number of bytes being serialized. In order to serialize
            // binary data, read/write each byte separately as an array.
            for (size_t byte_index = 0; byte_index < create_info->dataSize; ++byte_index)
            {
                file[kStrMemberNamePData][byte_index] = *((const uint8_t*)create_info->pData + byte_index);
            }
        }
    }

    void ReadStructure(VkSpecializationInfo* create_info, const nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            create_info->mapEntryCount = file[kStrMemberNameMapEntryCount];
            VkSpecializationMapEntry* map_entries = nullptr;
            if (create_info->mapEntryCount > 0)
            {
                map_entries = new VkSpecializationMapEntry[create_info->mapEntryCount];
                for (uint32_t entry_index = 0; entry_index < create_info->mapEntryCount; ++entry_index)
                {
                    ReadStructure(map_entries + entry_index, file[kStrMemberNamePMapEntries][entry_index]);
                }
            }
            create_info->pMapEntries = map_entries;
            create_info->dataSize = file[kStrMemberNameDataSize];

            // Allocate a byte array where the deserialized data will be copied.
            uint8_t* data_bytes = new uint8_t[create_info->dataSize]{};

            // create_info->dataSize is the number of bytes being serialized.
            for (size_t byteIndex = 0; byteIndex < create_info->dataSize; ++byteIndex)
            {
                *(data_bytes + byteIndex) = file[kStrMemberNamePData][byteIndex];
            }
            create_info->pData = data_bytes;
        }
    }

    void WriteStructure(const VkVertexInputBindingDescription* create_info, nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            file[kStrMemberNameBinding] = create_info->binding;
            file[kStrMemberNameStride] = create_info->stride;
            file[kStrMemberNameInputRate] = create_info->inputRate;
        }
    }

    void ReadStructure(VkVertexInputBindingDescription* create_info, const nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            create_info->binding = file[kStrMemberNameBinding];
            create_info->stride = file[kStrMemberNameStride];
            create_info->inputRate = file[kStrMemberNameInputRate];
        }
    }

    void WriteStructure(const VkSpecializationMapEntry* create_info, nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            file[kStrMemberNameConstantId] = create_info->constantID;
            file[kStrMemberNameOffset] = create_info->offset;
            file[kStrMemberNameSize] = create_info->size;
        }
    }

    void ReadStructure(VkSpecializationMapEntry* create_info, const nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            create_info->constantID = file[kStrMemberNameConstantId];
            create_info->offset = file[kStrMemberNameOffset];
            create_info->size = file[kStrMemberNameSize];
        }
    }

    void WriteStructure(const VkVertexInputAttributeDescription* create_info, nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            file[kStrMemberNameLocation] = create_info->location;
            file[kStrMemberNameBinding] = create_info->binding;
            file[kStrMemberNameFormat] = create_info->format;
            file[kStrMemberNameOffset] = create_info->offset;
        }
    }

    void ReadStructure(VkVertexInputAttributeDescription* create_info, const nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            create_info->location = file[kStrMemberNameLocation];
            create_info->binding = file[kStrMemberNameBinding];
            create_info->format = file[kStrMemberNameFormat];
            create_info->offset = file[kStrMemberNameOffset];
        }
    }

    void WriteStructure(const VkViewport* create_info, nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            file[kStrMemberNameX] = create_info->x;
            file[kStrMemberNameY] = create_info->y;
            file[kStrMemberNameWidth] = create_info->width;
            file[kStrMemberNameHeight] = create_info->height;
            file[kStrMemberNameMinDepth] = create_info->minDepth;
            file[kStrMemberNameMaxDepth] = create_info->maxDepth;
        }
    }

    void ReadStructure(VkViewport* create_info, const nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            create_info->x = file[kStrMemberNameX];
            create_info->y = file[kStrMemberNameY];
            create_info->width = file[kStrMemberNameWidth];
            create_info->height = file[kStrMemberNameHeight];
            create_info->minDepth = file[kStrMemberNameMinDepth];
            create_info->maxDepth = file[kStrMemberNameMaxDepth];
        }
    }

    void WriteStructure(const VkRect2D* create_info, nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            WriteStructure(&create_info->offset, file[kStrMemberNameOffset]);
            WriteStructure(&create_info->extent, file[kStrMemberNameExtent]);
        }
    }

    void ReadStructure(VkRect2D* create_info, const nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            ReadStructure(&create_info->offset, file[kStrMemberNameOffset]);
            ReadStructure(&create_info->extent, file[kStrMemberNameExtent]);
        }
    }

    void WriteStructure(const VkOffset2D* create_info, nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            file[kStrMemberNameX] = create_info->x;
            file[kStrMemberNameY] = create_info->y;
        }
    }

    void ReadStructure(VkOffset2D* create_info, const nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            create_info->x = file[kStrMemberNameX];
            create_info->y = file[kStrMemberNameY];
        }
    }

    void WriteStructure(const VkExtent2D* create_info, nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            file[kStrMemberNameWidth] = create_info->width;
            file[kStrMemberNameHeight] = create_info->height;
        }
    }

    void ReadStructure(VkExtent2D* create_info, const nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            create_info->width = file[kStrMemberNameWidth];
            create_info->height = file[kStrMemberNameHeight];
        }
    }

    void WriteStructure(const VkComputePipelineCreateInfo* create_info, nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            file[kStrMemberNameType] = create_info->sType;
            file[kStrMemberNamePnext] = WriteHandle(reinterpret_cast<uint64_t>(create_info->pNext));
            file[kStrMemberNameFlags] = create_info->flags;
            WriteStructure(&create_info->stage, file[kStrMemberNameStage]);
            if (create_info->layout != VK_NULL_HANDLE)
            {
                file[kStrMemberNameLayout] = WriteHandle(reinterpret_cast<uint64_t>(create_info->layout));
            }

            if (create_info->basePipelineHandle != VK_NULL_HANDLE)
            {
                file[kStrMemberNameBasePipelineHandle] = WriteHandle(reinterpret_cast<uint64_t>(create_info->basePipelineHandle));
            }

            file[kStrMemberNameBasePipelineIndex] = create_info->basePipelineIndex;
        }
    }

    void ReadStructure(VkComputePipelineCreateInfo* create_info, const nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            create_info->sType = file[kStrMemberNameType];
            create_info->pNext = ReadHandle(file[kStrMemberNamePnext]);
            create_info->flags = file[kStrMemberNameFlags];
            ReadStructure(&create_info->stage, file[kStrMemberNameStage]);
            if (IsCreateInfoExists(file, kStrMemberNameLayout))
            {
                create_info->layout = (VkPipelineLayout)ReadHandle(file[kStrMemberNameLayout]);
            }

            if (IsCreateInfoExists(file, kStrMemberNameBasePipelineHandle))
            {
                create_info->basePipelineHandle = (VkPipeline)ReadHandle(file[kStrMemberNameBasePipelineHandle]);
            }

            create_info->basePipelineIndex = file[kStrMemberNameBasePipelineIndex];
        }
    }

    void WriteStructure(const VkPipelineLayoutCreateInfo* create_info, nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            file[kStrMemberNameType] = create_info->sType;
            file[kStrMemberNamePnext] = WriteHandle(reinterpret_cast<uint64_t>(create_info->pNext));
            file[kStrMemberNameFlags] = create_info->flags;
            file[kStrMemberNameSetLayoutCount] = create_info->setLayoutCount;

            if (create_info->setLayoutCount > 0)
            {
                // Set the indices of descriptor set layouts which are referenced by this pipeline layout.
                for (uint32_t i = 0; i < create_info->setLayoutCount; i++)
                {
                    file[kStrMemberNamePSetLayouts][i] = WriteHandle((uint64_t)i);
                }
            }
            file[kStrMemberNamePushConstantRangeCount] = create_info->pushConstantRangeCount;
            for (uint32_t pushConstantRangeIndex = 0; pushConstantRangeIndex < create_info->pushConstantRangeCount; ++pushConstantRangeIndex)
            {
                WriteStructure((create_info->pPushConstantRanges + pushConstantRangeIndex), file[kStrMemberNamePPushConstantRanges][pushConstantRangeIndex]);
            }
        }
    }

    void ReadStructure(VkPipelineLayoutCreateInfo* create_info, const nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            create_info->sType = file[kStrMemberNameType];
            create_info->pNext = ReadHandle(file[kStrMemberNamePnext]);
            create_info->flags = file[kStrMemberNameFlags];

            create_info->setLayoutCount = file[kStrMemberNameSetLayoutCount];
            VkDescriptorSetLayout* set_layouts = nullptr;
            if (create_info->setLayoutCount > 0)
            {
                set_layouts = new VkDescriptorSetLayout[create_info->setLayoutCount]{};
                for (uint32_t set_layout_index = 0; set_layout_index < create_info->setLayoutCount; ++set_layout_index)
                {
                    set_layouts[set_layout_index] = (VkDescriptorSetLayout)ReadHandle(file[kStrMemberNamePSetLayouts][set_layout_index]);
                }
            }
            create_info->pSetLayouts = set_layouts;

            create_info->pushConstantRangeCount = file[kStrMemberNamePushConstantRangeCount];
            VkPushConstantRange* push_constant_ranges = nullptr;
            if (create_info->pushConstantRangeCount > 0)
            {
                push_constant_ranges = new VkPushConstantRange[create_info->pushConstantRangeCount];
                for (uint32_t pushConstantRangeIndex = 0; pushConstantRangeIndex < create_info->pushConstantRangeCount; ++pushConstantRangeIndex)
                {
                    ReadStructure(push_constant_ranges + pushConstantRangeIndex, file[kStrMemberNamePPushConstantRanges][pushConstantRangeIndex]);
                }
            }
            create_info->pPushConstantRanges = push_constant_ranges;
        }
    }

    void WriteStructure(const VkShaderModuleCreateInfo* create_info, nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            file[kStrMemberNameType] = create_info->sType;
            file[kStrMemberNamePnext] = WriteHandle(reinterpret_cast<uint64_t>(create_info->pNext));
            file[kStrMemberNameFlags] = create_info->flags;
            file[kStrMemberNameCodeSize] = create_info->codeSize;
        }
    }

    void ReadStructure(VkShaderModuleCreateInfo* create_info, const nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            create_info->sType = file[kStrMemberNameType];
            create_info->pNext = ReadHandle(file[kStrMemberNamePnext]);
            create_info->flags = file[kStrMemberNameFlags];
            create_info->codeSize = file[kStrMemberNameCodeSize];
        }
    }

    void WriteStructure(const VkDescriptorSetLayoutCreateInfo* create_info, nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            file[kStrMemberNameType] = create_info->sType;
            file[kStrMemberNamePnext] = WriteHandle(reinterpret_cast<uint64_t>(create_info->pNext));
            file[kStrMemberNameFlags] = create_info->flags;
            file[kStrMemberNameBindingCount] = create_info->bindingCount;

            for (uint32_t bindingIndex = 0; bindingIndex < create_info->bindingCount; ++bindingIndex)
            {
                WriteStructure((create_info->pBindings + bindingIndex), file[kStrMemberNamePBindings][bindingIndex]);
            }
        }
    }

    void ReadStructure(VkDescriptorSetLayoutCreateInfo* create_info, const nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            create_info->sType = file[kStrMemberNameType];
            create_info->pNext = ReadHandle(file[kStrMemberNamePnext]);
            create_info->flags = file[kStrMemberNameFlags];
            create_info->bindingCount = file[kStrMemberNameBindingCount];

            VkDescriptorSetLayoutBinding* bindings = nullptr;
            if (create_info->bindingCount > 0)
            {
                bindings = new VkDescriptorSetLayoutBinding[create_info->bindingCount]{};
                for (uint32_t binding_index = 0; binding_index < create_info->bindingCount; ++binding_index)
                {
                    ReadStructure(bindings + binding_index, file[kStrMemberNamePBindings][binding_index]);
                }
            }
            create_info->pBindings = bindings;
        }
    }

    void WriteStructure(const VkDescriptorSetLayoutBinding* create_info, nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            // Note: for now we are ignoring the immutable samplers.
            file[kStrMemberNameBinding] = create_info->binding;
            file[kStrMemberNameDescriptorType] = create_info->descriptorType;
            file[kStrMemberNameDescriptorCount] = create_info->descriptorCount;
            file[kStrMemberNameStageFlags] = create_info->stageFlags;
        }
    }

    void ReadStructure(VkDescriptorSetLayoutBinding* create_info, const nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            create_info->binding = file[kStrMemberNameBinding];
            create_info->descriptorType = file[kStrMemberNameDescriptorType];
            create_info->descriptorCount = file[kStrMemberNameDescriptorCount];
            create_info->stageFlags = file[kStrMemberNameStageFlags];
        }
    }

    void WriteStructure(const VkPushConstantRange* create_info, nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            file[kStrMemberNameStageFlags] = create_info->stageFlags;
            file[kStrMemberNameOffset] = create_info->offset;
            file[kStrMemberNameSize] = create_info->size;
        }
    }

    void ReadStructure(VkPushConstantRange* create_info, const nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            create_info->stageFlags = file[kStrMemberNameStageFlags];
            create_info->offset = file[kStrMemberNameOffset];
            create_info->size = file[kStrMemberNameSize];
        }
    }

    void WriteStructure(const VkRenderPassCreateInfo* create_info, nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            file[kStrMemberNameType] = create_info->sType;
            file[kStrMemberNamePnext] = WriteHandle(reinterpret_cast<uint64_t>(create_info->pNext));
            file[kStrMemberNameFlags] = create_info->flags;
            file[kStrMemberNameAttachmentCount] = create_info->attachmentCount;

            for (uint32_t attachment_index = 0; attachment_index < create_info->attachmentCount; ++attachment_index)
            {
                WriteStructure(create_info->pAttachments + attachment_index, file[kStrMemberNamePAttachments][attachment_index]);
            }

            file[kStrMemberNameSubpassCount] = create_info->subpassCount;
            for (uint32_t subpass_index = 0; subpass_index < create_info->subpassCount; ++subpass_index)
            {
                WriteStructure(create_info->pSubpasses + subpass_index, file[kStrMemberNamePSubpasses][subpass_index]);
            }

            file[kStrMemberNameDependencyCount] = create_info->dependencyCount;
            for (uint32_t dependency_index = 0; dependency_index < create_info->dependencyCount; ++dependency_index)
            {
                WriteStructure(create_info->pDependencies + dependency_index, file[kStrMemberNamePDependencies][dependency_index]);
            }
        }
    }

    void ReadStructure(VkRenderPassCreateInfo* create_info, const nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            create_info->sType = file[kStrMemberNameType];
            create_info->pNext = ReadHandle(file[kStrMemberNamePnext]);
            create_info->flags = file[kStrMemberNameFlags];
            create_info->attachmentCount = file[kStrMemberNameAttachmentCount];

            VkAttachmentDescription* attachments = nullptr;
            if (create_info->attachmentCount > 0)
            {
                attachments = new VkAttachmentDescription[create_info->attachmentCount]{};
                for (uint32_t attachment_index = 0; attachment_index < create_info->attachmentCount; ++attachment_index)
                {
                    ReadStructure(attachments + attachment_index, file[kStrMemberNamePAttachments][attachment_index]);
                }
            }
            create_info->pAttachments = attachments;

            create_info->subpassCount = file[kStrMemberNameSubpassCount];
            VkSubpassDescription* subpasses = nullptr;
            if (create_info->subpassCount > 0)
            {
                subpasses = new VkSubpassDescription[create_info->subpassCount]{};
                for (uint32_t subpass_index = 0; subpass_index < create_info->subpassCount; ++subpass_index)
                {
                    ReadStructure(subpasses + subpass_index, file[kStrMemberNamePSubpasses][subpass_index]);
                }
            }
            create_info->pSubpasses = subpasses;

            create_info->dependencyCount = file[kStrMemberNameDependencyCount];
            VkSubpassDependency* dependencies = nullptr;
            if (create_info->dependencyCount > 0)
            {
                dependencies = new VkSubpassDependency[create_info->dependencyCount]{};
                for (uint32_t dependency_index = 0; dependency_index < create_info->dependencyCount; ++dependency_index)
                {
                    ReadStructure(dependencies + dependency_index, file[kStrMemberNamePDependencies][dependency_index]);
                }
            }
            create_info->pDependencies = dependencies;
        }
    }

    void WriteStructure(const VkAttachmentDescription* create_info, nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            file[kStrMemberNameFlags] = create_info->flags;
            file[kStrMemberNameFormat] = create_info->format;
            file[kStrMemberNameSamples] = create_info->samples;
            file[kStrMemberNameLoadOp] = create_info->loadOp;
            file[kStrMemberNameStoreOp] = create_info->storeOp;
            file[kStrMemberNameStencilLoadOp] = create_info->stencilLoadOp;
            file[kStrMemberNameStencilStoreOp] = create_info->stencilStoreOp;
            file[kStrMemberNameInitialLayout] = create_info->initialLayout;
            file[kStrMemberNameFinalLayout] = create_info->finalLayout;
        }
    }

    void ReadStructure(VkAttachmentDescription* create_info, const nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            create_info->flags = file[kStrMemberNameFlags];
            create_info->format = file[kStrMemberNameFormat];
            create_info->samples = file[kStrMemberNameSamples];
            create_info->loadOp = file[kStrMemberNameLoadOp];
            create_info->storeOp = file[kStrMemberNameStoreOp];
            create_info->stencilLoadOp = file[kStrMemberNameStencilLoadOp];
            create_info->stencilStoreOp = file[kStrMemberNameStencilStoreOp];
            create_info->initialLayout = file[kStrMemberNameInitialLayout];
            create_info->finalLayout = file[kStrMemberNameFinalLayout];
        }
    }

    void WriteStructure(const VkSubpassDescription* create_info, nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            file[kStrMemberNameFlags] = create_info->flags;
            file[kStrMemberNamePipelineBindPoint] = create_info->pipelineBindPoint;
            file[kStrMemberNameInputAttachmentCount] = create_info->inputAttachmentCount;
            for (uint32_t input_attachment_index = 0; input_attachment_index < create_info->inputAttachmentCount; ++input_attachment_index)
            {
                WriteStructure(create_info->pInputAttachments + input_attachment_index, file[kStrMemberNamePInputAttachments][input_attachment_index]);
            }
            file[kStrMemberNameColorAttachmentCount] = create_info->colorAttachmentCount;
            for (uint32_t attachment_index = 0; attachment_index < create_info->colorAttachmentCount; ++attachment_index)
            {
                WriteStructure(create_info->pColorAttachments + attachment_index, file[kStrMemberNamePColorAttachments][attachment_index]);
                if (create_info->pResolveAttachments != nullptr)
                {
                    WriteStructure(create_info->pResolveAttachments + attachment_index, file[kStrMemberNamePResolveAttachments][attachment_index]);
                }
            }

            if (create_info->pDepthStencilAttachment != nullptr)
            {
                WriteStructure(create_info->pDepthStencilAttachment, file[kStrMemberNamePDepthStencilAttachment]);
            }

            file[kStrMemberNamePreserveAttachmentCount] = create_info->preserveAttachmentCount;
            for (uint32_t preserve_attachment_index = 0; preserve_attachment_index < create_info->preserveAttachmentCount; ++preserve_attachment_index)
            {
                file[kStrMemberNamePPreserveAttachments][preserve_attachment_index] = *(create_info->pPreserveAttachments + preserve_attachment_index);
            }
        }
    }

    void ReadStructure(VkSubpassDescription* create_info, const nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            create_info->flags = file[kStrMemberNameFlags];
            create_info->pipelineBindPoint = file[kStrMemberNamePipelineBindPoint];
            create_info->inputAttachmentCount = file[kStrMemberNameInputAttachmentCount];

            VkAttachmentReference* input_attachments = nullptr;
            if (create_info->inputAttachmentCount > 0)
            {
                input_attachments = new VkAttachmentReference[create_info->inputAttachmentCount]{};
                for (uint32_t input_attachment_index = 0; input_attachment_index < create_info->inputAttachmentCount; ++input_attachment_index)
                {
                    ReadStructure(input_attachments + input_attachment_index, file[kStrMemberNamePInputAttachments][input_attachment_index]);
                }
            }
            create_info->pInputAttachments = input_attachments;

            create_info->colorAttachmentCount = file[kStrMemberNameColorAttachmentCount];
            VkAttachmentReference* color_attachments = nullptr;
            VkAttachmentReference* resolve_attachments = nullptr;
            if (create_info->colorAttachmentCount > 0)
            {
                color_attachments = new VkAttachmentReference[create_info->colorAttachmentCount]{};
                for (uint32_t attachment_index = 0; attachment_index < create_info->colorAttachmentCount; ++attachment_index)
                {
                    ReadStructure(color_attachments + attachment_index, file[kStrMemberNamePColorAttachments][attachment_index]);
                }

                // Verify that an array of Resolve Attachments exists. If it does, it's the same dimension as the color attachments.
                if (IsCreateInfoExists(file, kStrMemberNamePResolveAttachments))
                {
                    resolve_attachments = new VkAttachmentReference[create_info->colorAttachmentCount]{};
                    for (uint32_t attachment_index = 0; attachment_index < create_info->colorAttachmentCount; ++attachment_index)
                    {
                        ReadStructure(resolve_attachments + attachment_index, file[kStrMemberNamePResolveAttachments][attachment_index]);
                    }
                }
            }
            create_info->pColorAttachments = color_attachments;
            create_info->pResolveAttachments = resolve_attachments;

            VkAttachmentReference* depth_stencil_attachment = nullptr;
            if (IsCreateInfoExists(file, kStrMemberNamePDepthStencilAttachment))
            {
                depth_stencil_attachment = new VkAttachmentReference{};
                ReadStructure(depth_stencil_attachment, file[kStrMemberNamePDepthStencilAttachment]);
            }
            create_info->pDepthStencilAttachment = depth_stencil_attachment;

            create_info->preserveAttachmentCount = file[kStrMemberNamePreserveAttachmentCount];
            uint32_t* preserve_attachment = nullptr;
            if (create_info->preserveAttachmentCount > 0)
            {
                preserve_attachment = new uint32_t[create_info->preserveAttachmentCount]{};
                for (uint32_t preserve_attachment_index = 0; preserve_attachment_index < create_info->preserveAttachmentCount; ++preserve_attachment_index)
                {
                    *(preserve_attachment + preserve_attachment_index) = file[kStrMemberNamePPreserveAttachments][preserve_attachment_index];
                }
            }
            create_info->pPreserveAttachments = preserve_attachment;
        }
    }

    void WriteStructure(const VkSubpassDependency* create_info, nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            file[kStrMemberNameSrcSubpass] = create_info->srcSubpass;
            file[kStrMemberNameDstSubpass] = create_info->dstSubpass;
            file[kStrMemberNameSrcStageMask] = create_info->srcStageMask;
            file[kStrMemberNameDstStageMask] = create_info->dstStageMask;
            file[kStrMemberNameSrcAccessMask] = create_info->srcAccessMask;
            file[kStrMemberNameDstAccessMask] = create_info->dstAccessMask;
            file[kStrMemberNameDependencyFlags] = create_info->dependencyFlags;
        }
    }

    void ReadStructure(VkSubpassDependency* create_info, const nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            create_info->srcSubpass = file[kStrMemberNameSrcSubpass];
            create_info->dstSubpass = file[kStrMemberNameDstSubpass];
            create_info->srcStageMask = file[kStrMemberNameSrcStageMask];
            create_info->dstStageMask = file[kStrMemberNameDstStageMask];
            create_info->srcAccessMask = file[kStrMemberNameSrcAccessMask];
            create_info->dstAccessMask = file[kStrMemberNameDstAccessMask];
            create_info->dependencyFlags = file[kStrMemberNameDependencyFlags];
        }
    }

    void WriteStructure(const VkAttachmentReference* create_info, nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            file[kStrMemberNameAttachment] = create_info->attachment;
            file[kStrMemberNameLayout] = create_info->layout;
        }
    }

    void ReadStructure(VkAttachmentReference* create_info, const nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            create_info->attachment = file[kStrMemberNameAttachment];
            create_info->layout = file[kStrMemberNameLayout];
        }
    }

    void ReadDescriptorSetLayoutCreateInfoArray(RgPsoCreateInfoVulkan* create_info, const nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            // Look for a Descriptor Set Layout create info node. If this node exists, process it as an
            // array of items, because a PSO file can have multiple create info structures saved.
            if (IsCreateInfoExists(file, kStrMemberNameVkDescriptorSetLayoutCreateInfo))
            {
                // The Pipeline Layout utilizes Descriptor Set Layout create info array.
                // Find the JSON root element, step through each child element, and read the data into memory.
                const nlohmann::json& descriptor_set_layouts_root = file[kStrMemberNameVkDescriptorSetLayoutCreateInfo];
                auto first_item = descriptor_set_layouts_root.begin();
                auto last_item = descriptor_set_layouts_root.end();

                // Clear the existing default Descriptor Set Layout create info structures and load
                // from scratch using data loaded from the PSO file.
                std::vector<VkDescriptorSetLayoutCreateInfo*> descriptor_set_layout_collection = create_info->GetDescriptorSetLayoutCreateInfo();
                descriptor_set_layout_collection.clear();

                // Read each individual element in the array of create info.
                for (auto item_iter = first_item; item_iter != last_item; ++item_iter)
                {
                    VkDescriptorSetLayoutCreateInfo* new_descriptor_set_layout = new VkDescriptorSetLayoutCreateInfo{};
                    assert(new_descriptor_set_layout != nullptr);
                    if (new_descriptor_set_layout != nullptr)
                    {
                        ReadStructure(new_descriptor_set_layout, *item_iter);
                        create_info->AddDescriptorSetLayoutCreateInfo(new_descriptor_set_layout);
                    }
                }
            }
        }
    }

    void WriteDescriptorSetLayoutCreateInfoArray(RgPsoCreateInfoVulkan* create_info, nlohmann::json& file)
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
                    WriteStructure(descriptor_set_layout_create_info, file[kStrMemberNameVkDescriptorSetLayoutCreateInfo][index]);
                    index++;
                }
            }
        }
    }

    bool ReadStructure(RgPsoGraphicsVulkan* create_info, const nlohmann::json& file)
    {
        bool is_loaded = false;

        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            // Does the given file's root element match what we expect to see for the project's pipeline type?
            bool is_matching_root_element = IsCreateInfoExists(file, kStrMemberNameVkGraphicsPipelineCreateInfo);
            if (is_matching_root_element)
            {
                // Deserialize the Graphics Pipeline create info.
                ReadGraphicsPipelineCreateInfoStructure(create_info, file[kStrMemberNameVkGraphicsPipelineCreateInfo]);

                // Deserialize the Render Pass create info.
                VkRenderPassCreateInfo* render_pass_create_info = create_info->GetRenderPassCreateInfo();
                assert(render_pass_create_info != nullptr);
                if (render_pass_create_info != nullptr)
                {
                    ReadStructure(render_pass_create_info, file[kStrMemberNameVkRenderPassCreateInfo]);
                }

                // Deserialize the Pipeline Layout create info.
                VkPipelineLayoutCreateInfo* pipeline_layout_create_info = create_info->GetPipelineLayoutCreateInfo();
                assert(pipeline_layout_create_info != nullptr);
                if (pipeline_layout_create_info != nullptr)
                {
                    ReadStructure(pipeline_layout_create_info, file[kStrMemberNameVkPipelineLayoutCreateInfo]);
                }

                // Read all Descriptor Set Layout create info structures.
                ReadDescriptorSetLayoutCreateInfoArray(create_info, file);

                // If all data was deserialized successfully, the PSO file is loaded.
                is_loaded = true;
            }
        }

        return is_loaded;
    }

    bool ReadStructure(RgPsoComputeVulkan* create_info, const nlohmann::json& file)
    {
        bool is_loaded = false;

        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            // Does the given file's root element match what we expect to see for the project's pipeline type?
            bool is_matching_root_element = IsCreateInfoExists(file, kStrMemberNameVkComputePipelineCreateInfo);
            if (is_matching_root_element)
            {
                VkComputePipelineCreateInfo* compute_pipeline_create_info = create_info->GetComputePipelineCreateInfo();
                assert(compute_pipeline_create_info != nullptr);
                if (compute_pipeline_create_info != nullptr)
                {
                    ReadStructure(compute_pipeline_create_info, file[kStrMemberNameVkComputePipelineCreateInfo]);
                }

                VkPipelineLayoutCreateInfo* pipeline_layout_create_info = create_info->GetPipelineLayoutCreateInfo();
                assert(pipeline_layout_create_info != nullptr);
                if (pipeline_layout_create_info != nullptr)
                {
                    ReadStructure(pipeline_layout_create_info, file[kStrMemberNameVkPipelineLayoutCreateInfo]);
                }

                // Read all Descriptor Set Layout create info structures.
                ReadDescriptorSetLayoutCreateInfoArray(create_info, file);

                // If all data was deserialized successfully, the PSO file is loaded.
                is_loaded = true;
            }
        }

        return is_loaded;
    }

    void WriteStructure(RgPsoGraphicsVulkan* create_info, nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            VkGraphicsPipelineCreateInfo* graphics_pso_create_info = create_info->GetGraphicsPipelineCreateInfo();
            assert(graphics_pso_create_info != nullptr);
            if (graphics_pso_create_info != nullptr)
            {
                WriteStructure(graphics_pso_create_info, file[kStrMemberNameVkGraphicsPipelineCreateInfo]);
            }

            VkRenderPassCreateInfo* render_pass_create_info = create_info->GetRenderPassCreateInfo();
            assert(render_pass_create_info != nullptr);
            if (render_pass_create_info != nullptr)
            {
                WriteStructure(render_pass_create_info, file[kStrMemberNameVkRenderPassCreateInfo]);
            }

            VkPipelineLayoutCreateInfo* pipeline_layout_create_info = create_info->GetPipelineLayoutCreateInfo();
            assert(pipeline_layout_create_info != nullptr);
            if (pipeline_layout_create_info != nullptr)
            {
                WriteStructure(pipeline_layout_create_info, file[kStrMemberNameVkPipelineLayoutCreateInfo]);
            }

            // Write the array of Descriptor Set Layout create info structures.
            WriteDescriptorSetLayoutCreateInfoArray(create_info, file);
        }
    }

    void WriteStructure(RgPsoComputeVulkan* create_info, nlohmann::json& file)
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            VkComputePipelineCreateInfo* compute_pso_create_info = create_info->GetComputePipelineCreateInfo();
            assert(compute_pso_create_info != nullptr);
            if (compute_pso_create_info != nullptr)
            {
                WriteStructure(compute_pso_create_info, file[kStrMemberNameVkComputePipelineCreateInfo]);
            }

            VkPipelineLayoutCreateInfo* pipeline_layout_create_info = create_info->GetPipelineLayoutCreateInfo();
            assert(pipeline_layout_create_info != nullptr);
            if (pipeline_layout_create_info != nullptr)
            {
                WriteStructure(pipeline_layout_create_info, file[kStrMemberNameVkPipelineLayoutCreateInfo]);
            }

            // Write the array of Descriptor Set Layout create info structures.
            WriteDescriptorSetLayoutCreateInfoArray(create_info, file);
        }
    }

    virtual VkSampleMask* ReadSampleMask(const VkPipelineMultisampleStateCreateInfo* create_info, const nlohmann::json& file)
    {
        VkSampleMask* sample_mask = nullptr;
        if (IsCreateInfoExists(file, kStrMemberNamePSampleMask))
        {
            sample_mask = new VkSampleMask[(uint32_t)create_info->rasterizationSamples];
            for (uint32_t index = 0; index < (uint32_t)create_info->rasterizationSamples; ++index)
            {
                sample_mask[index] = file[kStrMemberNamePSampleMask][index];
            }
        }
        return sample_mask;
    }

    virtual void WriteSampleMask(const VkPipelineMultisampleStateCreateInfo* create_info, nlohmann::json& file)
    {
        if (create_info->pSampleMask != nullptr)
        {
            for (uint32_t index = 0; index < (uint32_t)create_info->rasterizationSamples; ++index)
            {
                file[kStrMemberNamePSampleMask][index] = *(create_info->pSampleMask + index);
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
            file[kStrMemberNameFailOp] = create_info->failOp;
            file[kStrMemberNamePassOp] = create_info->passOp;
            file[kStrMemberNameDepthFailOp] = create_info->depthFailOp;
            file[kStrMemberNameCompareOp] = create_info->compareOp;
            file[kStrMemberNameCompareMask] = create_info->compareMask;
            file[kStrMemberNameWriteMask] = create_info->writeMask;
            file[kStrMemberNameReference] = create_info->reference;
        }
    }

    virtual void ReadStructure(VkStencilOpState* create_info, const nlohmann::json& file) override
    {
        assert(create_info != nullptr);
        if (create_info != nullptr)
        {
            create_info->failOp = file[kStrMemberNameFailOp];
            create_info->passOp = file[kStrMemberNamePassOp];
            create_info->depthFailOp = file[kStrMemberNameDepthFailOp];
            create_info->compareOp = file[kStrMemberNameCompareOp];
            create_info->compareMask = file[kStrMemberNameCompareMask];
            create_info->writeMask = file[kStrMemberNameWriteMask];
            create_info->reference = file[kStrMemberNameReference];
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
        VkSampleMask* sample_mask = nullptr;
        if (IsCreateInfoExists(file, kStrMemberNamePSampleMask))
        {
            // What's the required dimension of the pSampleMask bitfield array?
            int sample_mask_array_dimension = 0;
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
                sample_mask_array_dimension = 1;
                break;
            case VK_SAMPLE_COUNT_64_BIT:
                // Two 32-bit values are needed to hold the 64-bit pSampleMask bitfield.
                sample_mask_array_dimension = 2;
                break;
            default:
                // The RasterizationSamples value was unrecognized, and we can't proceed.
                assert(false);
            }

            sample_mask = new VkSampleMask[sample_mask_array_dimension]{};
            for (int index = 0; index < sample_mask_array_dimension; ++index)
            {
                sample_mask[index] = file[kStrMemberNamePSampleMask][index];
            }
        }
        return sample_mask;
    }

    virtual void WriteSampleMask(const VkPipelineMultisampleStateCreateInfo* create_info, nlohmann::json& file) override
    {
        // It's fine if pSampleMask is null- just don't write the field.
        if (create_info->pSampleMask != nullptr)
        {
            // What's the required dimension of the pSampleMask bitfield array?
            uint32_t sample_mask_array_dimension = 0;
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
                sample_mask_array_dimension = 1;
                break;
            case VK_SAMPLE_COUNT_64_BIT:
                // Two 32-bit values are needed to hold the 64-bit pSampleMask bitfield.
                sample_mask_array_dimension = 2;
                break;
            default:
                // The RasterizationSamples value was unrecognized, and we can't proceed.
                assert(false);
            }

            // Serialize the array of bitfields. It'll either be 1 or 2 32-bit values.
            for (uint32_t index = 0; index < sample_mask_array_dimension; ++index)
            {
                file[kStrMemberNamePSampleMask][index] = *(create_info->pSampleMask + index);
            }
        }
    }
};

std::shared_ptr<rgPsoSerializerVulkanImpl_Version_1_0> CreateSerializer(RgPipelineModelVersion serializerVersion)
{
    std::shared_ptr<rgPsoSerializerVulkanImpl_Version_1_0> serializer = nullptr;

    switch (serializerVersion)
    {
    case RgPipelineModelVersion::kVERSION_1_0:
        serializer = std::make_shared<rgPsoSerializerVulkanImpl_Version_1_0>();
        break;
    case RgPipelineModelVersion::kVERSION_1_1:
        serializer = std::make_shared<rgPsoSerializerVulkanImpl_Version_1_1>();
        break;
    case RgPipelineModelVersion::kVERSION_1_2:
        serializer = std::make_shared<rgPsoSerializerVulkanImpl_Version_1_2>();
        break;
    default:
        // If we get here, there is no matching serializer type for the incoming version.
        assert(false);
    }

    return serializer;
}

bool RgPsoSerializerVulkan::ReadStructureFromFile(const std::string& file_path, RgPsoGraphicsVulkan** create_info_array, std::string& error_string)
{
    bool ret = false;
    bool should_abort = false;

    assert(create_info_array != nullptr);
    if (create_info_array != nullptr)
    {
        // Create a new PSO State structure.
        RgPsoGraphicsVulkan* create_info = new RgPsoGraphicsVulkan{};

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
                RgPipelineModelVersion model_version = RgPipelineModelVersion::kUnknown;
                if (IsCreateInfoExists(structure, kStrPipelineModelVersion))
                {
                    model_version = static_cast<RgPipelineModelVersion>(structure[kStrPipelineModelVersion].get<int>());
                }
                else
                {
                    // Versioning doesn't exist in the initial revision of the pipeline state file.
                    // When the model version tag isn't found, assume VERSION_1_0.
                    model_version = RgPipelineModelVersion::kVERSION_1_0;
                }

                assert(model_version != RgPipelineModelVersion::kUnknown);
                if (model_version != RgPipelineModelVersion::kUnknown)
                {
                    // Always write the most recent version of the pipeline state file.
                    std::shared_ptr<rgPsoSerializerVulkanImpl_Version_1_0> serializer = CreateSerializer(model_version);
                    assert(serializer != nullptr);
                    if (serializer != nullptr)
                    {
                        // Read the structure data from the JSON file.
                        if (serializer->ReadStructure(create_info, structure))
                        {
                            // Assign the deserialized pipeline state file to the output pointer.
                            *create_info_array = create_info;

                            ret = true;
                        }
                        else
                        {
                            error_string = kStrErrFailedToLoadPipelineTypeMismatch;
                        }
                    }
                    else
                    {
                        error_string = kStrErrFailedUnsupportedVersion;
                    }
                }
                else
                {
                    error_string = kStrErrFailedToReadPipelineVersion;
                }
            }
        }
        else
        {
            std::stringstream error_stream;
            error_stream << kStrErrFailedToReadFile;
            error_stream << file_path;
            error_string = error_stream.str();
        }
    }

    return ret;
}

bool RgPsoSerializerVulkan::ReadStructureFromFile(const std::string& file_path, RgPsoComputeVulkan** create_info_array, std::string& error_string)
{
    bool ret = false;
    bool should_abort = false;

    assert(create_info_array != nullptr);
    if (create_info_array != nullptr)
    {
        // Create a new PSO State structure.
        RgPsoComputeVulkan* create_info = new RgPsoComputeVulkan{};

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
                RgPipelineModelVersion model_version = RgPipelineModelVersion::kUnknown;
                if (IsCreateInfoExists(structure, kStrPipelineModelVersion))
                {
                    model_version = static_cast<RgPipelineModelVersion>(structure[kStrPipelineModelVersion].get<int>());
                }
                else
                {
                    // Versioning doesn't exist in the initial revision of the pipeline state file.
                    // When the model version tag isn't found, assume VERSION_1_0.
                    model_version = RgPipelineModelVersion::kVERSION_1_0;
                }

                assert(model_version != RgPipelineModelVersion::kUnknown);
                if (model_version != RgPipelineModelVersion::kUnknown)
                {
                    // Always write the most recent version of the pipeline state file.
                    std::shared_ptr<rgPsoSerializerVulkanImpl_Version_1_0> serializer = CreateSerializer(model_version);
                    assert(serializer != nullptr);
                    if (serializer != nullptr)
                    {
                        if (serializer->ReadStructure(create_info, structure))
                        {
                            // Assign the deserialized pipeline state file to the output pointer.
                            *create_info_array = create_info;
                            ret = true;
                        }
                        else
                        {
                            error_string = kStrErrFailedToLoadPipelineTypeMismatch;
                        }
                    }
                    else
                    {
                        error_string = kStrErrFailedUnsupportedVersion;
                    }
                }
                else
                {
                    error_string = kStrErrFailedToReadPipelineVersion;
                }
            }
        }
        else
        {
            std::stringstream error_stream;
            error_stream << kStrErrFailedToReadFile;
            error_stream << file_path;
            error_string = error_stream.str();
        }
    }

    return ret;
}

bool RgPsoSerializerVulkan::WriteStructureToFile(RgPsoGraphicsVulkan* create_info, const std::string& file_path, std::string& error_string)
{
    bool ret = false;

    // Open a file to write the structure data to.
    std::ofstream file_stream;
    file_stream.open(file_path.c_str(), std::ofstream::out);

    if (file_stream.is_open())
    {
        nlohmann::json json_file;

        // Write the current pipeline version into the file.
        json_file[kStrPipelineModelVersion] = kCurrentPipelineVersion;

        // Always write the most recent version of the pipeline state file.
        std::shared_ptr<rgPsoSerializerVulkanImpl_Version_1_0> serializer = CreateSerializer(kCurrentPipelineVersion);
        assert(serializer != nullptr);
        if (serializer != nullptr)
        {
            serializer->WriteStructure(create_info, json_file);

            // Write the JSON to disk and close the file with the given indentation.
            file_stream << json_file.dump(4);
            file_stream.close();

            ret = true;
        }
        else
        {
            error_string = kStrErrFailedUnsupportedVersion;
        }
    }
    else
    {
        std::stringstream error_stream;
        error_stream << kStrErrFailedToWriteFile;
        error_stream << file_path.c_str();
        error_string = error_stream.str();
    }

    return ret;
}

bool RgPsoSerializerVulkan::WriteStructureToFile(RgPsoComputeVulkan* create_info, const std::string& file_path, std::string& error_string)
{
    bool ret = false;

    // Open a file to write the structure data to.
    std::ofstream file_stream;
    file_stream.open(file_path.c_str(), std::ofstream::out);

    if (file_stream.is_open())
    {
        nlohmann::json json_file;

        // Write the current pipeline version into the file.
        json_file[kStrPipelineModelVersion] = kCurrentPipelineVersion;

        // Always write the most recent version of the pipeline state file.
        std::shared_ptr<rgPsoSerializerVulkanImpl_Version_1_0> serializer = CreateSerializer(kCurrentPipelineVersion);
        assert(serializer != nullptr);
        if (serializer != nullptr)
        {
            serializer->WriteStructure(create_info, json_file);

            // Write the JSON to disk and close the file with the given indentation.
            file_stream << json_file.dump(4);
            file_stream.close();

            ret = true;
        }
        else
        {
            error_string = kStrErrFailedUnsupportedVersion;
        }
    }
    else
    {
        std::stringstream error_stream;
        error_stream << kStrErrFailedToWriteFile;
        error_stream << file_path.c_str();
        error_string = error_stream.str();
    }

    return ret;
}

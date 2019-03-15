#pragma once

// C++.
#include <map>

// Infra.
#include <Utils/Vulkan/Include/rgPipelineTypes.h>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgPipelineStateModel.h>

// Forward declarations.
class rgPsoGraphicsVulkan;

// rgPipelineStateModelVulkan is a Pipeline State model used to populate and edit Vulkan-specific
// values within the Pipeline State editor.
class rgPipelineStateModelVulkan : public rgPipelineStateModel
{
    Q_OBJECT

public:
    rgPipelineStateModelVulkan(QWidget* pParent = nullptr);
    virtual ~rgPipelineStateModelVulkan() = default;

    // Load a pipeline state configuration from the file at the given path.
    bool LoadPipelineStateFile(QWidget* pParent, const std::string& psoFilePath, rgPipelineType pipelineType, std::string& errorString);

    // Save the current pipeline state configuration to the given file path.
    bool SavePipelineStateFile(const std::string& psoFilePath, std::string& errorString);

signals:
    // A signal to indicate list widget status change.
    void EnumListWidgetStatusSignal(bool isOpen);

protected:
    // Check that the pipeline state is valid.
    virtual bool CheckValidPipelineState(std::string& errorString) const;

    // Initialize the model with the default graphics pipeline state.
    virtual void InitializeDefaultGraphicsPipeline() override;

    // Initialize the model with the default compute pipeline state.
    virtual void InitializeDefaultComputePipeline() override;

    // Initialize the Graphics Pipeline CreateInfo tree structure.
    virtual rgEditorElement* InitializeGraphicsPipelineCreateInfo(QWidget* pParent) override;

    // Initialize the Compute Pipeline CreateInfo tree structure.
    virtual rgEditorElement* InitializeComputePipelineCreateInfo(QWidget* pParent) override;

private:
// ****************
// Functions and declarations below are used for both graphics and compute pipelines.
// ****************

    // Initialize the Pipeline Layout CreateInfo tree structure.
    void InitializePipelineLayoutCreateInfo(rgEditorElement* pRootElement, VkPipelineLayoutCreateInfo* pPipelineLayoutCreateInfo);

    // Initialize the Descriptor Set Layout CreateInfo tree structure.
    void InitializeDescriptorSetLayoutCreateInfo(rgEditorElement* pRootElement, VkDescriptorSetLayoutCreateInfo* pDescriptorSetLayoutCreateInfo);

    // Handle resizing the number of Descriptor Set Layout create info structures.
    void HandleDescriptorSetLayoutCountChanged(rgEditorElement* pRootElement, rgPsoCreateInfoVulkan* pCreateInfo, bool firstInit = false);

    // Initialize the Descriptor Set Layout array create info rows.
    void InitializeDescriptorSetLayoutCreateInfoArray(rgEditorElement* pRootElement, rgPsoCreateInfoVulkan* pCreateInfo);

    // Handler invoked when the Pipeline Layout Descriptor Set Layout count is changed.
    void HandlePipelineLayoutDescriptorSetLayoutCountChanged(rgEditorElement* pRootElement,
        VkPipelineLayoutCreateInfo* pPipelineLayoutCreateInfo,
        bool firstInit = false);

    // Handler invoked when the Pipeline Layout push constants count changes.
    void HandlePushConstantsCountChanged(rgEditorElement* pRootElement,
        VkPipelineLayoutCreateInfo* pPipelineLayoutCreateInfo,
        bool firstInit = false);

    // Handler invoked when the descriptor set layout binding count changes.
    void HandleDescriptorSetLayoutBindingCountChanged(rgEditorElement* pRootElement,
        VkDescriptorSetLayoutCreateInfo* pDescriptorSetLayoutCreateInfo,
        bool firstInit = false);

    // Initialize a Descriptor Set Layout item.
    void InitializeDescriptorSetLayout(rgEditorElement* pRootElement,
        VkDescriptorSetLayout* pDescriptorSetLayout, int itemIndex);

    // Initialize a Push Constant Range item.
    void InitializePushConstantRange(rgEditorElement* pRootElement,
        VkPushConstantRange* pPushConstant, int itemIndex);

    // Initialize a descriptor set layout binding item.
    void InitializeDescriptorSetLayoutBinding(rgEditorElement* pRootElement,
        VkDescriptorSetLayoutBinding* pDescriptorSetLayout, int itemIndex);

    // The number of descriptor set layouts configured by the user.
    uint32_t m_descriptorSetLayoutCount;

// ****************
// Functions below are used to edit VkGraphicsPipelineCreateInfo.
// ****************

    // Initialize the VkGraphicsPipelineCreateInfo element hierarchy.
    void InitializeVkGraphicsPipelineCreateInfo(rgEditorElement* pRootElement, rgPsoGraphicsVulkan* pGraphicsPipelineCreateInfo);

    // Initialize the Vertex Input State CreateInfo tree structure.
    void InitializeVertexInputStateCreateInfo(rgEditorElement* pRootElement, VkPipelineVertexInputStateCreateInfo* pVertexInputStateCreateInfo);

    // Initialize a "VkVertexInputBindingDescription" create info node.
    void InitializeVertexInputBindingDescriptionCreateInfo(rgEditorElement* pRootElement,
        VkVertexInputBindingDescription* pItem, int itemIndex);

    // Initialize a "VkVertexInputAttributeDescription" create info node.
    void InitializeVertexInputAttributeDescriptionCreateInfo(rgEditorElement* pRootElement,
        VkVertexInputAttributeDescription* pInputAttributeDescription, int itemIndex);

    // Initialize the Input Assembly State CreateInfo tree structure.
    void InitializeInputAssemblyStateCreateInfo(rgEditorElement* pRootElement, VkPipelineInputAssemblyStateCreateInfo* pInputAssemblyStateCreateInfo);

    // Initialize the Tessellation State CreateInfo tree structure.
    void InitializeTessellationStateCreateInfo(rgEditorElement* pRootElement, VkPipelineTessellationStateCreateInfo* pTessellationStateCreateInfo);

    // Initialize the Viewport State CreateInfo tree structure.
    void InitializeViewportStateCreateInfo(rgEditorElement* pRootElement, VkPipelineViewportStateCreateInfo* pViewportStateCreateInfo);

    // Handler invoked when the viewport count changes.
    void HandlePipelineViewportCountChanged(rgEditorElement* pRootElement,
        VkPipelineViewportStateCreateInfo* pViewportStateCreateInfo,
        bool firstInit = false);

    // Handler invoked when the scissor count changes.
    void HandlePipelineScissorCountChanged(rgEditorElement* pRootElement,
        VkPipelineViewportStateCreateInfo* pViewportStateCreateInfo,
        bool firstInit = false);

    // Initialize a VkViewport description.
    void InitializePipelineViewportDescriptionCreateInfo(rgEditorElement* pRootElement,
        VkViewport* pViewportDescription, int itemIndex);

    // Initialize a VkRect2D scissor rectangle description.
    void InitializePipelineScissorDescriptionCreateInfo(rgEditorElement* pRootElement,
        VkRect2D* pScissorDescription, int itemIndex);

    // Initialize the Rasterization State CreateInfo tree structure.
    void InitializeRasterizationStateCreateInfo(rgEditorElement* pRootElement, VkPipelineRasterizationStateCreateInfo* pRasterizationStateCreateInfo);

    // Initialize the Multisample State CreateInfo tree structure.
    void InitializeMultisampleStateCreateInfo(rgEditorElement* pRootElement, VkPipelineMultisampleStateCreateInfo* pPipelineMultisampleStateCreateInfo);

    // Handler invoked when the multisampling state pSampleMask array dimension is changed.
    void HandleMultisamplingSampleMaskDimensionChanged(rgEditorElement* pRootElement,
        VkPipelineMultisampleStateCreateInfo* pMultisampleStateCreateInfo, bool firstInit = false);

    // Initialize a VkSampleMask array element.
    void InitializeSampleMask(rgEditorElement* pRootElement, uint32_t* pSampleMask, int itemIndex);

    // Initialize a single VkStencilOpState element.
    void InitializeStencilOpState(rgEditorElement* pDepthStencilStateRoot,
        VkStencilOpState* pStencilOpState);

    // Initialize the Depth Stencil State CreateInfo tree structure.
    void InitializeDepthStencilStateCreateInfo(rgEditorElement* pRootElement, VkPipelineDepthStencilStateCreateInfo* pPipelineDepthStencilStateCreateInfo);

    // Initialize the Color Blend State CreateInfo tree structure.
    void InitializeColorBlendStateCreateInfo(rgEditorElement* pRootElement, VkPipelineColorBlendStateCreateInfo* pPipelineColorBlendStateCreateInfo);

    // Handler invoked when the vertex binding description count is changed.
    void HandleVertexBindingDescriptionCountChanged(rgEditorElement* pRootElement,
        VkPipelineVertexInputStateCreateInfo* pInputStateCreateInfo,
        bool firstInit = false);

    // Handler invoked when the vertex attribute description count is changed.
    void HandleVertexAttributeDescriptionCountChanged(rgEditorElement* pRootElement,
        VkPipelineVertexInputStateCreateInfo* pInputStateCreateInfo,
        bool firstInit = false);

    // Handler invoked when the pipeline color blend attachment count is changed.
    void HandlePipelineColorBlendAttachmentCountChanged(rgEditorElement* pRootElement,
        VkPipelineColorBlendStateCreateInfo* pPipelineColorBlendStateCreateInfo,
        bool firstInit = false);

    // Initialize a VkPipelineColorBlendAttachmentState description structure.
    void InitializePipelineBlendAttachmentStateCreateInfo(rgEditorElement* pRootElement,
        VkPipelineColorBlendAttachmentState* pColorBlendAttachmentState, int itemIndex);

    // A depth stencil attachment reference that can be modified by the editor. This is necessary
    // due to the fact that the Vulkan structure uses a const pointer whose values cannot be altered.
    VkAttachmentReference* m_pDepthStencilAttachment = nullptr;

    // The graphics pipeline state structure being edited.
    rgPsoGraphicsVulkan* m_pGraphicsPipelineState = nullptr;

    // A map of the number of resolve attachments per subpass.
    std::map<int, uint32_t*> m_resolveAttachmentCountPerSubpass;

    // The dimension of a graphics pipeline's multisampling state VkSampleMask array.
    uint32_t m_sampleMaskDimension = 0;

// ****************
// Functions below are used to edit VkRenderPassCreateInfo.
// ****************

    // Initialize the Render Pass CreateInfo tree structure.
    void InitializeRenderPassCreateInfo(rgEditorElement* pRootElement, VkRenderPassCreateInfo* pCreateInfo);

    // Handler invoked when the Render Pass attachment count is changed.
    void HandleRenderPassAttachmentCountChanged(rgEditorElement* pRootElement,
        VkRenderPassCreateInfo* pRenderPassCreateInfo,
        bool firstInit = false);

    // Handler invoked when the Render Pass subpass count is changed.
    void HandleRenderPassSubpassCountChanged(rgEditorElement* pRootElement,
        VkRenderPassCreateInfo* pRenderPassCreateInfo,
        bool firstInit = false);

    // Handler invoked when the Render Pass dependency count is changed.
    void HandleRenderPassDependencyCountChanged(rgEditorElement* pRootElement,
        VkRenderPassCreateInfo* pRenderPassCreateInfo,
        bool firstInit = false);

    // Handler invoked when the Render Pass subpass input attachment count is changed.
    void HandleRenderPassSubpassInputAttachmentCountChanged(rgEditorElement* pRootElement,
        VkSubpassDescription* pSubpassCreateInfo,
        bool firstInit = false);

    // Handler invoked when the Render Pass subpass color attachment count is changed.
    void HandleRenderPassSubpassColorAttachmentCountChanged(rgEditorElement* pRootElement,
        VkSubpassDescription* pSubpassCreateInfo,
        bool firstInit = false);

    // Handler invoked when the Render Pass subpass resolve attachment count is changed.
    void HandleRenderPassSubpassResolveAttachmentCountChanged(int subpassIndex, rgEditorElement* pRootElement,
        VkSubpassDescription* pSubpassCreateInfo,
        bool firstInit = false);

    // Handler invoked when the Render Pass subpass preserve attachment count is changed.
    void HandleRenderPassSubpassPreserveAttachmentCountChanged(rgEditorElement* pRootElement,
        VkSubpassDescription* pSubpassCreateInfo,
        bool firstInit = false);

    // Initialize a Render Pass's attachment CreateInfo.
    void InitializeRenderPassAttachmentDescriptionCreateInfo(rgEditorElement* pRootElement,
        VkAttachmentDescription* pAttachmentDescription, int itemIndex);

    // Initialize a Render Pass's subpass CreateInfo.
    void InitializeRenderPassSubpassDescriptionCreateInfo(rgEditorElement* pRootElement,
        VkSubpassDescription* pSubpassDescription, int itemIndex);

    // Initialize a Render Subpass description's AttachmentReference.
    void InitializeAttachmentReference(rgEditorElement* pRootElement,
        VkAttachmentReference* pAttachmentReference, int itemIndex);

    // Initialize a Render Subpass preserve attachment.
    void InitializePreserveAttachment(rgEditorElement* pRootElement,
        uint32_t* pAttachmentReference, int itemIndex);

    // Initialize a RenderPass dependency description structure.
    void InitializeRenderPassDependencyDescriptionCreateInfo(rgEditorElement* pRootElement,
        VkSubpassDependency* pDependencyDescription, int itemIndex);

// ****************
// Functions and declarations below are used to edit VkComputePipelineCreateInfo.
// ****************

    // Initialize the VkComputePipelineCreateInfo element hierarchy.
    void InitializeVkComputePipelineCreateInfo(rgEditorElement* pRootElement, rgPsoComputeVulkan* pComputePipelineCreateInfo);

    // The compute pipeline state structure being edited.
    rgPsoComputeVulkan* m_pComputePipelineState = nullptr;
};
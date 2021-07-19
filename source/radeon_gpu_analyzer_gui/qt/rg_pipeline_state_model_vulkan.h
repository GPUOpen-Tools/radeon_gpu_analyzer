#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_PIPELINE_STATE_MODEL_VULKAN_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_PIPELINE_STATE_MODEL_VULKAN_H_

// C++.
#include <map>

// Infra.
#include "source/common/vulkan/rg_pipeline_types.h"

// Local.
#include "source/radeon_gpu_analyzer_gui/qt/rg_pipeline_state_model.h"

// Forward declarations.
class RgPsoGraphicsVulkan;

// RgPipelineStateModelVulkan is a Pipeline State model used to populate and edit Vulkan-specific
// values within the Pipeline State editor.
class RgPipelineStateModelVulkan : public RgPipelineStateModel
{
    Q_OBJECT

public:
    RgPipelineStateModelVulkan(QWidget* parent = nullptr);
    virtual ~RgPipelineStateModelVulkan() = default;

    // Load a pipeline state configuration from the file at the given path.
    bool LoadPipelineStateFile(QWidget* parent, const std::string& pso_file_path, RgPipelineType pipeline_type, std::string& error_string);

    // Save the current pipeline state configuration to the given file path.
    bool SavePipelineStateFile(const std::string& pso_file_path, std::string& error_string);

signals:
    // A signal to indicate list widget status change.
    void EnumListWidgetStatusSignal(bool is_open);

    // A signal to indicate change of view.
    void HotKeyPressedSignal();

protected:
    // Check that the pipeline state is valid.
    virtual bool CheckValidPipelineState(std::string& error_string) const;

    // Initialize the model with the default graphics pipeline state.
    virtual void InitializeDefaultGraphicsPipeline() override;

    // Initialize the model with the default compute pipeline state.
    virtual void InitializeDefaultComputePipeline() override;

    // Initialize the Graphics Pipeline CreateInfo tree structure.
    virtual RgEditorElement* InitializeGraphicsPipelineCreateInfo(QWidget* parent) override;

    // Initialize the Compute Pipeline CreateInfo tree structure.
    virtual RgEditorElement* InitializeComputePipelineCreateInfo(QWidget* parent) override;

private:
// ****************
// Functions and declarations below are used for both graphics and compute pipelines.
// ****************

    // Initialize the Pipeline Layout CreateInfo tree structure.
    void InitializePipelineLayoutCreateInfo(RgEditorElement* root_element, VkPipelineLayoutCreateInfo* pipeline_layout_create_info);

    // Initialize the Descriptor Set Layout CreateInfo tree structure.
    void InitializeDescriptorSetLayoutCreateInfo(RgEditorElement* root_element, VkDescriptorSetLayoutCreateInfo* descriptor_set_layout_create_info);

    // Handle resizing the number of Descriptor Set Layout create info structures.
    void HandleDescriptorSetLayoutCountChanged(RgEditorElement* root_element, RgPsoCreateInfoVulkan* create_info, bool first_init = false);

    // Initialize the Descriptor Set Layout array create info rows.
    void InitializeDescriptorSetLayoutCreateInfoArray(RgEditorElement* root_element, RgPsoCreateInfoVulkan* create_info);

    // Handler invoked when the Pipeline Layout Descriptor Set Layout count is changed.
    void HandlePipelineLayoutDescriptorSetLayoutCountChanged(RgEditorElement* root_element,
        VkPipelineLayoutCreateInfo* pipeline_layout_create_info,
        bool first_init = false);

    // Handler invoked when the Pipeline Layout push constants count changes.
    void HandlePushConstantsCountChanged(RgEditorElement* root_element,
        VkPipelineLayoutCreateInfo* pipeline_layout_create_info,
        bool first_init = false);

    // Handler invoked when the descriptor set layout binding count changes.
    void HandleDescriptorSetLayoutBindingCountChanged(RgEditorElement* root_element,
        VkDescriptorSetLayoutCreateInfo* descriptor_set_layout_create_info,
        bool first_init = false);

    // Initialize a Descriptor Set Layout item.
    void InitializeDescriptorSetLayout(RgEditorElement* root_element,
        VkDescriptorSetLayout* descriptor_set_layout, int item_index);

    // Initialize a Push Constant Range item.
    void InitializePushConstantRange(RgEditorElement* root_element,
        VkPushConstantRange* push_constant, int item_index);

    // Initialize a descriptor set layout binding item.
    void InitializeDescriptorSetLayoutBinding(RgEditorElement* root_element,
        VkDescriptorSetLayoutBinding* descriptor_set_layout, int item_index);

    // The number of descriptor set layouts configured by the user.
    uint32_t descriptor_set_layout_count_;

// ****************
// Functions below are used to edit VkGraphicsPipelineCreateInfo.
// ****************

    // Initialize the VkGraphicsPipelineCreateInfo element hierarchy.
    void InitializeVkGraphicsPipelineCreateInfo(RgEditorElement* root_element, RgPsoGraphicsVulkan* graphics_pipeline_create_info);

    // Initialize the Vertex Input State CreateInfo tree structure.
    void InitializeVertexInputStateCreateInfo(RgEditorElement* root_element, VkPipelineVertexInputStateCreateInfo* vertex_input_state_create_info);

    // Initialize a "VkVertexInputBindingDescription" create info node.
    void InitializeVertexInputBindingDescriptionCreateInfo(RgEditorElement* root_element,
        VkVertexInputBindingDescription* item, int item_index);

    // Initialize a "VkVertexInputAttributeDescription" create info node.
    void InitializeVertexInputAttributeDescriptionCreateInfo(RgEditorElement* root_element,
        VkVertexInputAttributeDescription* input_attribute_description, int item_index);

    // Initialize the Input Assembly State CreateInfo tree structure.
    void InitializeInputAssemblyStateCreateInfo(RgEditorElement* root_element, VkPipelineInputAssemblyStateCreateInfo* input_assembly_state_create_info);

    // Initialize the Tessellation State CreateInfo tree structure.
    void InitializeTessellationStateCreateInfo(RgEditorElement* root_element, VkPipelineTessellationStateCreateInfo* tessellation_state_create_info);

    // Initialize the Viewport State CreateInfo tree structure.
    void InitializeViewportStateCreateInfo(RgEditorElement* root_element, VkPipelineViewportStateCreateInfo* viewport_state_create_info);

    // Handler invoked when the viewport count changes.
    void HandlePipelineViewportCountChanged(RgEditorElement* root_element,
        VkPipelineViewportStateCreateInfo* viewport_state_create_info,
        bool first_init = false);

    // Handler invoked when the scissor count changes.
    void HandlePipelineScissorCountChanged(RgEditorElement* root_element,
        VkPipelineViewportStateCreateInfo* viewport_state_create_info,
        bool first_init = false);

    // Initialize a VkViewport description.
    void InitializePipelineViewportDescriptionCreateInfo(RgEditorElement* root_element,
        VkViewport* viewport_description, int item_index);

    // Initialize a VkRect2D scissor rectangle description.
    void InitializePipelineScissorDescriptionCreateInfo(RgEditorElement* root_element,
        VkRect2D* scissor_description, int item_index);

    // Initialize the Rasterization State CreateInfo tree structure.
    void InitializeRasterizationStateCreateInfo(RgEditorElement* root_element, VkPipelineRasterizationStateCreateInfo* rasterization_state_create_info);

    // Initialize the Multisample State CreateInfo tree structure.
    void InitializeMultisampleStateCreateInfo(RgEditorElement* root_element, VkPipelineMultisampleStateCreateInfo* pipeline_multisample_state_create_info);

    // Handler invoked when the multisampling state pSampleMask array dimension is changed.
    void HandleMultisamplingSampleMaskDimensionChanged(RgEditorElement* root_element,
        VkPipelineMultisampleStateCreateInfo* multisample_state_create_info, bool first_init = false);

    // Initialize a VkSampleMask array element.
    void InitializeSampleMask(RgEditorElement* root_element, uint32_t* sample_mask, int item_index);

    // Initialize a single VkStencilOpState element.
    void InitializeStencilOpState(RgEditorElement* depth_stencil_state_root,
        VkStencilOpState* stencil_op_state);

    // Initialize the Depth Stencil State CreateInfo tree structure.
    void InitializeDepthStencilStateCreateInfo(RgEditorElement* root_element, VkPipelineDepthStencilStateCreateInfo* pipeline_depth_stencil_state_create_info);

    // Initialize the Color Blend State CreateInfo tree structure.
    void InitializeColorBlendStateCreateInfo(RgEditorElement* root_element, VkPipelineColorBlendStateCreateInfo* pipeline_color_blend_state_create_info);

    // Handler invoked when the vertex binding description count is changed.
    void HandleVertexBindingDescriptionCountChanged(RgEditorElement* root_element,
        VkPipelineVertexInputStateCreateInfo* input_state_create_info,
        bool first_init = false);

    // Handler invoked when the vertex attribute description count is changed.
    void HandleVertexAttributeDescriptionCountChanged(RgEditorElement* root_element,
        VkPipelineVertexInputStateCreateInfo* input_state_create_info,
        bool first_init = false);

    // Handler invoked when the pipeline color blend attachment count is changed.
    void HandlePipelineColorBlendAttachmentCountChanged(RgEditorElement* root_element,
        VkPipelineColorBlendStateCreateInfo* pipeline_color_blend_state_create_info,
        bool first_init = false);

    // Initialize a VkPipelineColorBlendAttachmentState description structure.
    void InitializePipelineBlendAttachmentStateCreateInfo(RgEditorElement* root_element,
        VkPipelineColorBlendAttachmentState* color_blend_attachment_state, int item_index);

    // A depth stencil attachment reference that can be modified by the editor. This is necessary
    // due to the fact that the Vulkan structure uses a const pointer whose values cannot be altered.
    VkAttachmentReference* depth_stencil_attachment_ = nullptr;

    // The graphics pipeline state structure being edited.
    RgPsoGraphicsVulkan* graphics_pipeline_state_ = nullptr;

    // A map of the number of resolve attachments per subpass.
    std::map<int, uint32_t*> resolve_attachment_count_per_subpass_;

    // The dimension of a graphics pipeline's multisampling state VkSampleMask array.
    uint32_t sample_mask_dimension_ = 0;

// ****************
// Functions below are used to edit VkRenderPassCreateInfo.
// ****************

    // Initialize the Render Pass CreateInfo tree structure.
    void InitializeRenderPassCreateInfo(RgEditorElement* root_element, VkRenderPassCreateInfo* create_info);

    // Handler invoked when the Render Pass attachment count is changed.
    void HandleRenderPassAttachmentCountChanged(RgEditorElement* root_element,
        VkRenderPassCreateInfo* render_pass_create_info,
        bool first_init = false);

    // Handler invoked when the Render Pass subpass count is changed.
    void HandleRenderPassSubpassCountChanged(RgEditorElement* root_element,
        VkRenderPassCreateInfo* render_pass_create_info,
        bool first_init = false);

    // Handler invoked when the Render Pass dependency count is changed.
    void HandleRenderPassDependencyCountChanged(RgEditorElement* root_element,
        VkRenderPassCreateInfo* render_pass_create_info,
        bool first_init = false);

    // Handler invoked when the Render Pass subpass input attachment count is changed.
    void HandleRenderPassSubpassInputAttachmentCountChanged(RgEditorElement* root_element,
        VkSubpassDescription* subpass_create_info,
        bool first_init = false);

    // Handler invoked when the Render Pass subpass color attachment count is changed.
    void HandleRenderPassSubpassColorAttachmentCountChanged(RgEditorElement* root_element,
        VkSubpassDescription* subpass_create_info,
        bool first_init = false);

    // Handler invoked when the Render Pass subpass resolve attachment count is changed.
    void HandleRenderPassSubpassResolveAttachmentCountChanged(int subpassIndex, RgEditorElement* root_element,
        VkSubpassDescription* subpass_create_info,
        bool first_init = false);

    // Handler invoked when the Render Pass subpass preserve attachment count is changed.
    void HandleRenderPassSubpassPreserveAttachmentCountChanged(RgEditorElement* root_element,
        VkSubpassDescription* subpass_create_info,
        bool first_init = false);

    // Initialize a Render Pass's attachment CreateInfo.
    void InitializeRenderPassAttachmentDescriptionCreateInfo(RgEditorElement* root_element,
        VkAttachmentDescription* attachment_description, int item_index);

    // Initialize a Render Pass's subpass CreateInfo.
    void InitializeRenderPassSubpassDescriptionCreateInfo(RgEditorElement* root_element,
        VkSubpassDescription* subpass_description, int item_index);

    // Initialize a Render Subpass description's AttachmentReference.
    void InitializeAttachmentReference(RgEditorElement* root_element,
        VkAttachmentReference* attachment_reference, int item_index);

    // Initialize a Render Subpass preserve attachment.
    void InitializePreserveAttachment(RgEditorElement* root_element,
        uint32_t* attachment_reference, int item_index);

    // Initialize a RenderPass dependency description structure.
    void InitializeRenderPassDependencyDescriptionCreateInfo(RgEditorElement* root_element,
        VkSubpassDependency* dependency_description, int item_index);

// ****************
// Functions and declarations below are used to edit VkComputePipelineCreateInfo.
// ****************

    // Initialize the VkComputePipelineCreateInfo element hierarchy.
    void InitializeVkComputePipelineCreateInfo(RgEditorElement* root_element, RgPsoComputeVulkan* compute_pipeline_create_info);

    // The compute pipeline state structure being edited.
    RgPsoComputeVulkan* compute_pipeline_state_ = nullptr;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_PIPELINE_STATE_MODEL_VULKAN_H_

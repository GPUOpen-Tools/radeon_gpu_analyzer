#pragma once

// Type of Vulkan input file.
enum class RgVulkanInputType
{
    kUnknown,
    kGlsl,
    kHlsl,
    kSpirv,
    kSpirvTxt
};

// The base type for a Pipeline State file, which contains all
// configuration necessary to create a graphics or compute pipeline.
class RgPsoCreateInfo
{
public:
    RgPsoCreateInfo() = default;
    virtual ~RgPsoCreateInfo() = default;
};

// The type of stage used for a shader module.
enum RgPipelineStage : char
{
    // The vertex shader stage.
    kVertex,

    // The Vulkan Tessellation Control / DX12 Hull shader stage.
    kTessellationControl,

    // The Vulkan Tessellation Evaluation / DX12 Domain shader stage.
    kTessellationEvaluation,

    // The geometry shader stage.
    kGeometry,

    // The Vulkan Fragment / DX12 Pixel shader stage.
    kFragment,

    // The compute shader stage.
    kCompute,

    // The total count of pipeline stage types.
    kCount
};

#pragma once

// Type of Vulkan input file.
enum class rgVulkanInputType
{
    Unknown,
    Glsl,
    Hlsl,
    Spirv,
    SpirvTxt
};

// The base type for a Pipeline State file, which contains all
// configuration necessary to create a graphics or compute pipeline.
class rgPsoCreateInfo
{
public:
    rgPsoCreateInfo() = default;
    virtual ~rgPsoCreateInfo() = default;
};

// The type of stage used for a shader module.
enum rgPipelineStage : char
{
    // The vertex shader stage.
    Vertex,

    // The Vulkan Tessellation Control / DX12 Hull shader stage.
    TessellationControl,

    // The Vulkan Tessellation Evaluation / DX12 Domain shader stage.
    TessellationEvaluation,

    // The geometry shader stage.
    Geometry,

    // The Vulkan Fragment / DX12 Pixel shader stage.
    Fragment,

    // The compute shader stage.
    Compute,

    // The total count of pipeline stage types.
    Count
};
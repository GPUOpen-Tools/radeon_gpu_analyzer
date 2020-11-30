//=================================================================
// Copyright 2020 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

#ifndef __beDataTypes_h
#define __beDataTypes_h

#include <array>
#include <vector>
#include <string>

#ifdef _WIN32
    #pragma warning(push)
    #pragma warning(disable:4309)
#endif
#include <AMDTBaseTools/Include/gtString.h>
#ifdef _WIN32
    #pragma warning(pop)
#endif

// A structure to hold the shader file names of a given pipeline.
struct BeProgramPipeline
{
    // Clears all pipeline shaders.
    void ClearAll()
    {
        vertex_shader.makeEmpty();
        tessellation_control_shader.makeEmpty();
        tessellation_evaluation_shader.makeEmpty();
        geometry_shader.makeEmpty();
        fragment_shader.makeEmpty();
        compute_shader.makeEmpty();
    }

    // Vertex shader.
    gtString vertex_shader;

    // Tessellation control shader.
    gtString tessellation_control_shader;

    // Tessellation evaluation shader.
    gtString tessellation_evaluation_shader;

    // Geometry shader.
    gtString geometry_shader;

    // Fragment shader.
    gtString fragment_shader;

    // Compute shader.
    gtString compute_shader;
};

// The type of stage used for a shader module.
enum BePipelineStage : char
{
    kVertex,
    kTessellationControl,
    kTessellationEvaluation,
    kGeometry,
    kFragment,
    kCompute,

    kCount
};

// An array containing per-stage file names.
typedef std::array<std::string, BePipelineStage::kCount>  BeVkPipelineFiles;

// Physical adapter data.
struct BeVkPhysAdapterInfo
{
    uint32_t     id;
    std::string  name;
    std::string  vkDriverVersion;
    std::string  vkAPIVersion;
};

#endif // __beDataTypes_h

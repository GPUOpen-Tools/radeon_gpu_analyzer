//=================================================================
// Copyright 2017 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

#ifndef __beDataTypes_h
#define __beDataTypes_h

#include <array>

#ifdef _WIN32
    #pragma warning(push)
    #pragma warning(disable:4309)
#endif
#include <AMDTBaseTools/Include/gtString.h>
#ifdef _WIN32
    #pragma warning(pop)
#endif


// A structure to hold the shader file names of a given pipeline.
struct beProgramPipeline
{
    // Clears all pipeline shaders.
    void ClearAll()
    {
        m_vertexShader.makeEmpty();
        m_tessControlShader.makeEmpty();
        m_tessEvaluationShader.makeEmpty();
        m_geometryShader.makeEmpty();
        m_fragmentShader.makeEmpty();
        m_computeShader.makeEmpty();
    }

    // Vertex shader.
    gtString m_vertexShader;

    // Tessellation control shader.
    gtString m_tessControlShader;

    // Tessellation evaluation shader.
    gtString m_tessEvaluationShader;

    // Geometry shader.
    gtString m_geometryShader;

    // Fragment shader.
    gtString m_fragmentShader;

    // Compute shader.
    gtString m_computeShader;
};

// The type of stage used for a shader module.
enum bePipelineStage : char
{
    Vertex,
    TessellationControl,
    TessellationEvaluation,
    Geometry,
    Fragment,
    Compute,

    Count
};

// An array containing per-stage file names.
typedef std::array<std::string, bePipelineStage::Count>  beVkPipelineFiles;

// Physical adapter data.
struct beVkPhysAdapterInfo
{
    uint32_t     id;
    std::string  name;
    std::string  vkDriverVersion;
    std::string  vkAPIVersion;
};

#endif // __beDataTypes_h

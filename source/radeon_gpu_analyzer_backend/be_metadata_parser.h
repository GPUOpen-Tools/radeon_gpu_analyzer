//=============================================================================
/// Copyright (c) 2024-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for rga code object metadata parser class.
//=============================================================================

#ifndef RGA_RADEONGPUANALYZERBACKEND_SRC_BE_METADATA_PARSER_H_
#define RGA_RADEONGPUANALYZERBACKEND_SRC_BE_METADATA_PARSER_H_

// C++.
#include <string>
#include <vector>

// Local.
#include "radeon_gpu_analyzer_backend/be_program_builder.h"
#include "radeon_gpu_analyzer_backend/be_data_types.h"

// A set of utilities for handling mangled shader names.
class BeMangledKernelUtils
{
public:
    // Returns unmangled shader name if given a mangled shader_name,
    // otherwise just returns shader_name as is.
    static std::string DemangleShaderName(const std::string& shader_name);

    // Remove Quotes from provided string if it contains a mangled name.
    static std::string UnQuote(const std::string& str, char quote = '\"');

    // Add Quotes to provided string if it contains a mangled name.
    static std::string Quote(const std::string& str, char quote = '\"');
};

// Metadata Parsed from Amdgpu-dis code object Metadata string.
class BeAmdPalMetaData
{
public:

    // Enum to represent different stages in hardware pipeline.
    enum class StageType
    {
        kLS,
        kHS,
        kES,
        kGS,
        kVS,
        kPS,
        kCS
    };

    // Enum to represent different shader types.
    enum class ShaderType
    {
        kVertex,
        kHull,
        kDomain,
        kGeometry,
        kPixel,
        kCompute,
        kMesh,
        kTask
    };

    // Enum to represent different shader subtypes.
    enum class ShaderSubtype
    {
        kUnknown,
        kRayGeneration,
        kMiss,
        kAnyHit,
        kClosestHit,
        kIntersection,
        kCallable,
        kTraversal,
        kLaunchKernel
    };

    // Struct to hold hardware stage details.
    struct HardwareStageMetaData
    {
        StageType          stage_type;
        beKA::AnalysisData stats;
    };

    // Struct to hold shader function details.
    struct ShaderFunctionMetaData
    {
        std::string        name;
        ShaderSubtype      shader_subtype;
        beKA::AnalysisData stats;
    };

    // Struct to hold shader details.
    struct ShaderMetaData
    {
        ShaderType    shader_type;
        StageType     hardware_mapping;
        ShaderSubtype shader_subtype = ShaderSubtype::kUnknown;
    };

    // Struct to hold amdpal pipeline details.
    struct PipelineMetaData
    {
        std::string                         api;
        std::vector<HardwareStageMetaData>  hardware_stages;
        std::vector<ShaderFunctionMetaData> shader_functions;
        std::vector<ShaderMetaData>         shaders;
    };

    // Converts string stage name to StageType enum
    static StageType GetStageType(const std::string& stage_name);

    // Converts string shader name to ShaderType enum
    static ShaderType GetShaderType(const std::string& shader_name);

    // Converts string shader subtype to ShaderSubtype enum
    static ShaderSubtype GetShaderSubtype(const std::string& subtype_name);

    // Converts StageType enum to string stage name.
    static std::string GetStageName(StageType stage_type);

    // Converts ShaderType enum to string shader name.
    static std::string GetShaderName(ShaderType shader_type);

    // Converts beWaveSize enum to string.
    static std::string GetWaveSize(beWaveSize wave_size);

    // Converts string to beWaveSize enum.
    static beWaveSize GetWaveSize(std::string wave_size);

    // Converts uint64_t to beWaveSize enum.
    static beWaveSize GetWaveSize(uint64_t wave_size);

    // Converts ShaderSubtype enum to string shader subtype.
    static std::string GetShaderSubtypeName(ShaderSubtype subtype);

    // Parses amdgpu-dis output and extracts code object metadata.
    static beKA::beStatus ParseAmdgpudisMetadata(const std::string& amdgpu_dis_output, BeAmdPalMetaData::PipelineMetaData& pipeline);

};

#endif // RGA_RADEONGPUANALYZERBACKEND_SRC_BE_METADATA_PARSER_H_

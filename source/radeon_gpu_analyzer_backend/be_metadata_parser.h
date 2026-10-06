//=============================================================================
/// Copyright (c) 2024-2026 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for rga code object metadata parser class.
//=============================================================================

#ifndef RGA_RADEONGPUANALYZERBACKEND_SRC_BE_METADATA_PARSER_H_
#define RGA_RADEONGPUANALYZERBACKEND_SRC_BE_METADATA_PARSER_H_

// C++.
#include <iomanip>
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
    using ShaderType = BePipelineStage;

    // Enum to represent different shader subtypes.
    using ShaderSubtype = BeRtxPipelineStage;

    // Struct to holds a 128 bit hash.
    struct ApiShaderHash
    {
        // The low 64 bits of the hash.
        uint64_t low = 0;
        // The high 64 bits of the hash.
        uint64_t high = 0;

        // Overload the output stream operator for easy printing.
        friend std::ostream& operator<<(std::ostream& os, const ApiShaderHash& hash)
        {
            os << Hash128BitToStr(hash.high, hash.low);
            return os;
        }

    private:
        // Convert the 128 bit hash to a string representation.
        static std::string Hash64BitToStr(uint64_t hash_val)
        {
            char buffer[19] = {};
            snprintf(buffer, 19, "%016llX", static_cast<long long unsigned int>(hash_val));
            return std::string(buffer);
        }

        // Convert the hash to a 128 bit string representation.
        static std::string Hash128BitToStr(uint64_t upper_bits, uint64_t lower_bits)
        {
            std::string out;

            if (upper_bits == 0 && lower_bits == 0)
            {
                out = "N/A";
            }
            else
            {
                if (upper_bits == 0)
                {
                    out = "0x" + Hash64BitToStr(lower_bits);
                }
                else
                {
                    out = "0x" + Hash64BitToStr(upper_bits) + Hash64BitToStr(lower_bits);
                }
            }

            return out;
        }
    };

    // Struct to hold hardware stage details.
    struct HardwareStageMetaData
    {
        StageType          stage_type;
        std::string        entry_point;
        beKA::AnalysisData stats;
    };

    // Struct to hold shader function details.
    struct ShaderFunctionMetaData
    {
        std::string        name;      // Demangled name (for display/output).
        std::string        raw_name;  // Raw mangled name from metadata YAML key (for ELF symbol matching).
        ApiShaderHash      hash;
        ShaderSubtype      shader_subtype;
        beKA::AnalysisData stats;
    };

    // Struct to hold shader details.
    struct ShaderMetaData
    {
        ShaderType    shader_type;
        ApiShaderHash hash;
        StageType     hardware_mapping;
        ShaderSubtype shader_subtype = ShaderSubtype::kUnknown;
    };

    // Struct to hold amdpal pipeline details.
    struct PipelineMetaData
    {
        std::string                         device;
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
    static beKA::beStatus ParseMetadata(const std::string& metadata_text, PipelineMetaData& pipeline);

    // Converts ApiShaderHash to string representation.
    static std::string GetShaderHashString(const ApiShaderHash& hash);
};

// Metadata Parsed from amdhsa metadata string.
class BeAmdHsaMetaData
{
public:
    // Kernel statistics.
    struct KernelProperties
    {
        size_t wavefront_num_sgprs    = 0;
        size_t work_item_num_vgprs    = 0;
        size_t work_item_num_agprs    = 0;
        size_t wavefront_size         = 0;
        size_t workgroup_segment_size = 0;
        size_t private_segment_size   = 0;
        size_t sgpr_spills            = 0;
        size_t vgpr_spills            = 0;
        size_t isa_size               = 0;
    };

    // Maps  kernel_name --> KernelProperties.
    using KernelPropertiesMap = std::map<std::string, KernelProperties>;

    // Contatiner of Kernel Names.
    using KernelNames = std::vector<std::string>;

    // Struct to hold amdhsa kernel details.
    struct AmdHsaMetaData
    {
        std::string         device;
        KernelNames         kernel_names;
        KernelPropertiesMap props_map;
    };

    // Parses amdgpu-dis output and extracts code object metadata.
    static beKA::beStatus ParseMetadata(const std::string& metadata_text, AmdHsaMetaData& md);
};

#endif  // RGA_RADEONGPUANALYZERBACKEND_SRC_BE_METADATA_PARSER_H_

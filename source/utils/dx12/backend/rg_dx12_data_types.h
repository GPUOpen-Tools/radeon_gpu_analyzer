#pragma once

// C++.
#include <string>
#include <vector>
#include <map>
#include <memory>

// DX12.
#include <d3d12.h>

namespace rga
{
    // Per-stage configuration (based on user's invocation command).
    struct RgDx12ShaderConfig
    {
        // Full path to HLSL file containing the shader.
        std::string hlsl;

        // Full path to compiled DXBC of the shader as an input.
        std::string dxbc;

        // Full path to ISA disassembly output file.
        std::string isa;

        // Full path to AMDIL disassembly output file.
        std::string amdil;

        // Full path to resource usage output file.
        std::string stats;

        // Shader model for the shader (also known as Target).
        std::string shader_model;

        // The target entry point.
        std::string entry_point;

        // The compiled shader DXBC binary output file.
        std::string dxbcOut;

        // The compiled shader DXBC disassembly output file.
        std::string dxbc_disassembly;
    };

    // Global configuration (based on user's invocation command).
    struct RgDx12Config
    {
        // Per-stage configuration.
        RgDx12ShaderConfig comp;
        RgDx12ShaderConfig vert;
        RgDx12ShaderConfig hull;
        RgDx12ShaderConfig domain;
        RgDx12ShaderConfig geom;
        RgDx12ShaderConfig pixel;

        // Additional include directories.
        std::vector<std::string> include_dirs;

        // Additional include directories.
        std::vector<std::string> defines;

        // The name of the macro which defines the root signature in the HLSL code.
        std::string rs_macro;

        // The full path to the HLSL file where the root signature macro is defined in.
        std::string rs_macro_file;

        // Root signature version, by default "rootsig_1_1" is assumed.
        std::string rs_version = "rootsig_1_1";

        // Full path to the serialized root signature file.
        std::string rs_serialized;

        // Full path to the file that describes the pipeline state.
        std::string rs_pso;

        // DXR compilation mode (Pipeline or Shader).
        std::string dxr_mode;

        // DXR ISA disassembly output file.
        std::string dxr_isa_output;

        // DXR statistics output file.
        std::string dxr_statistics_output;

        // DXR binary output file.
        std::string dxr_binary_output;

        // Full path to the DXR state description file.
        std::string dxr_state_file;

        // DXR export (raygeneration shader name in Pipeline mode or shader name in Shader mode).
        std::string dxrExport;

        // Full path to the pipeline binary output file.
        std::string pipeline_binary;

        // HLSL->DXIL mapping file.
        std::string dxr_hlsl_mapping;

        // Output metadata file.
        std::string output_metadata;

        // DXR HLSL input.
        std::string dxr_hlsl_input;

        // The ID of the GPU for which to compile the pipeline. If not specified,
        // the pipeline would be compiled for the physically installed GPU.
        int target_gpu = -1;

        // True if we should list all supported targets.
        bool should_list_targets = false;

        // Enable the D3D12 debug layer.
        bool should_enable_debug_layer = false;

        // Offline session.
        bool is_offline_session = false;
    };

    // Per-stage boolean.
    struct RgPipelineBool
    {
        bool vert = false;
        bool hull = false;
        bool domain = false;
        bool geom = false;
        bool pixel = false;
    };

    // Per-stage bytecode.
    struct RgDx12PipelineByteCode
    {
        D3D12_SHADER_BYTECODE vert;
        D3D12_SHADER_BYTECODE hull;
        D3D12_SHADER_BYTECODE domain;
        D3D12_SHADER_BYTECODE geom;
        D3D12_SHADER_BYTECODE pixel;
    };

    // *** DXR-SPECIFIC TYPES - BEGIN ***

    // Type of source files that serve as an input.
    enum DxrSourceType
    {
        kUnknown = 0,

        // Precompiled binary (DXIL library for example).
        kBinary,

        // HLSL source code.
        kHlsl
    };

    struct RgDxrExport
    {
        RgDxrExport(const std::wstring& dxr_export_name) : export_name(dxr_export_name) {}
        RgDxrExport(const std::wstring& dxr_export_name, const std::wstring& dxrLinkageName) :
            export_name(dxr_export_name), linkage_name(dxrLinkageName){}

        // The export name.
        std::wstring export_name;

        // The linkage to which we would like to rename (if not empty).
        std::wstring linkage_name;
    };

    struct RgDxrDxilLibrary
    {
        // The type of input file: source means that a compilation
        // would be required by RGA.
        DxrSourceType input_type = DxrSourceType::kUnknown;

        // Full path to the relevant source file (DXIL or HLSL).
        std::string full_path;

        // Flags - assume none.
        D3D12_EXPORT_FLAGS flags = D3D12_EXPORT_FLAG_NONE;

        // The relevant exports that are defined in this library.
        std::vector<RgDxrExport> exports;

        // The contents of the compiled input in binary representation.
        std::vector<unsigned char> binary_data;
    };

    struct RgDxrHitGroup
    {
        RgDxrHitGroup() = default;
        RgDxrHitGroup(const std::wstring& hg_name) : hitgroup_name(hg_name) {}
        RgDxrHitGroup(const std::wstring& hg_name, const std::wstring& hg_intersection_shader,
            const std::wstring& hg_any_hit_shader, const std::wstring& hg_closest_hit_shader) : hitgroup_name(hg_name),
            intersection_shader(hg_intersection_shader), any_hit_shader(hg_any_hit_shader), closest_hit_shader(hg_closest_hit_shader) {}

        // Hit group name.
        std::wstring hitgroup_name;

        // Hit group's intersection shader.
        std::wstring intersection_shader;

        // Hit group's any hit shader.
        std::wstring any_hit_shader;

        // Hit group's closest hit shader.
        std::wstring closest_hit_shader;

        // Hit group type.
        D3D12_HIT_GROUP_TYPE type = D3D12_HIT_GROUP_TYPE_TRIANGLES;
    };

    struct RgDxrRootSignature
    {
        RgDxrRootSignature() = default;
        RgDxrRootSignature(const std::string& rs_full_path) : full_path(rs_full_path){}

        // Type of input file: binary (precompiled) or HLSL.
        DxrSourceType input_type = DxrSourceType::kUnknown;

        // Full path to the DXIL or HLSL file where the root signature is defined.
        std::string full_path;

        // The relevant exports to be associated with this root signature.
        std::vector<std::wstring> exports;

        // The contents of the compiled input in binary representation.
        std::vector<unsigned char> binary_data;
    };

    struct RgDx12DxrPipelineConfig
    {
        uint32_t max_trace_recursion_depth = 0;
        uint32_t flags = 0;
    };

    struct RgDxrShaderConfig
    {
        uint32_t max_payload_size_in_bytes = 0;
        uint32_t max_attribute_size_in_bytes = 0;

        // The relevant exports that are defined in this library.
        std::vector<std::wstring> exports;
    };

    // Holds the entire DXR state as expected to be received from the user through metadata file.
    struct RgDxrStateDesc
    {
        // Input files (for shader element).
        std::vector<std::shared_ptr<RgDxrDxilLibrary>> input_files;

        // Hit groups.
        std::vector<RgDxrHitGroup> hit_groups;

        // Local root signatures.
        std::vector<std::shared_ptr<RgDxrRootSignature>> local_root_signature;

        // Global root signatures.
        std::vector<std::shared_ptr<RgDxrRootSignature>> global_root_signature;

        // Shader config elements.
        std::vector<RgDxrShaderConfig> shader_config;

        // Pipeline config.
        RgDx12DxrPipelineConfig pipeline_config;
    };

    // The results per DXR shader.
    struct RgDxrShaderResults
    {
        std::string export_name;
        std::string isa_disassembly;
        std::string stats;
    };

    struct RgDxrPipelineResults
    {
        // Pipeline name is either a raygeneration shader name or an index (in All mode).
        std::string pipeline_name;

        // Full path to pipeline binary output file.
        std::string pipeline_binary;

        // Results for all of the pipeline's shaders.
        std::vector<RgDxrShaderResults> results;

        // True if pipeline was compiled in unified mode, false otherwise.
        bool isUnified = true;
    };

    // *** DXR-SPECIFIC TYPES - END ***
}

//=================================================================
// Copyright 2020 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================
#ifndef RGA_RADEONGPUANALYZERCLI_SRC_KC_CONFIG_H_
#define RGA_RADEONGPUANALYZERCLI_SRC_KC_CONFIG_H_

#include <iostream>
#include <string>
#include <vector>

#include "radeon_gpu_analyzer_backend/be_backend.h"
#include "radeon_gpu_analyzer_cli/kc_data_types.h"

/// A place to collect command line options.
/// This is a POD with a dump method.
struct Config
{
public:
    static std::string source_kind_dx11;
    static std::string source_kind_dx12;
    static std::string source_kind_dxr;
    static std::string source_kind_amdil;
    static std::string source_kind_opencl;
    static std::string source_kind_opengl;
    static std::string source_kind_glsl_vulkan_offline;
    static std::string source_kind_spirv_bin_offline;
    static std::string source_kind_spirv_txt_offline;
    static std::string source_kind_vulkan;
    static std::string source_kind_binary;

    enum ConfigCommand
    {
        kNone,
        kInvalid,
        kCompile,
        kListEntries,
        kHelp,
        kListAsics,
        kListAdapters,
        kVersion,
        kGenVersionInfoFile,
        kGenTemplateFile,
        kUpdate
    };

    Config();

    beKA::RgaMode            mode;                              ///< RGA mode.
    ConfigCommand            requested_command;                 ///< What the user requested to do
    std::vector<std::string> input_files;                       ///< Source file for processing.
    std::string              analysis_file;                     ///< Output analysis file.
    std::string              il_file;                           ///< Output IL Text file template.
    std::string              isa_file;                          ///< Output ISA Text file template.
    std::string              livereg_analysis_file;             ///< VGPR Live register analysis output file.
    std::string              sgpr_livereg_analysis_file;        ///< SGPR Live register analysis output file.
    std::string              block_cfg_file;                    ///< Output file for per-block control flow graph.
    std::string              inst_cfg_file;                     ///< Output file for per-instruction control flow graph.
    std::string              inference_analysis_file;           ///< Output file for SPP inference (text).
    std::string              inference_image_file;              ///< Output file for SPP inference (image).
    std::string              inference_bottleneck_threshold;    ///< The number of cycles that defines a bottleneck for SPP.
    std::string              inference_model_confidence;        ///< The prediction model's level of confidence.
    std::string              inference_engine_path;             ///< Alternative path to the inference engine to be used for SPP.
    std::string              binary_output_file;                ///< Output binary file template.
    std::string              binary_codeobj_file;               ///< Input binary file template - used primarily in binary mode.
    std::string              function;                          ///< Kernel/Function of interest in analysis.
    std::string              csv_separator;                     ///< Override for CSV list separator.
    std::string              metadata_file;                     ///< Output .metadata Text file template.
    std::vector<std::string> asics;                             ///< Target GPUs for compilation.
    std::vector<std::string> suppress_section;                  ///< List of sections to suppress in generated binary files.
    std::vector<std::string> opencl_options;                    ///< Options to be added to OpenCL compile.
    std::vector<std::string> defines;                           ///< Macros to be added to compile.
    std::vector<std::string> include_path;                      ///< Additional Include paths
    bool                     should_avoid_binary_device_prefix; ///< If true then CLI will not add the asic name to the generated binary output file.
    bool                     should_avoid_binary_suffix = false;///< If true, no extension would be added to the output binary file.
    bool                     should_retain_temp_files = false;  ///< If true, temporary files will not be cleaned up.
    std::string              version_info_file;                 ///< RGA CLI config file name.
    std::string              session_metadata_file;             ///< RGA CLI session metadata file name.
    std::string              log_file;                          ///< RGA CLI log file name (full path).
    int                      opt_level;                         ///< Optimization level.
    bool                     donot_rename_il_files = false;     ///< If true, generated IL files will not have the ASIC and function name prefixed.  

    // DX11/GL.
    std::string              source_kind;                       ///< Kind of source HLSL or GLSL (maybe more later like ASM kinds).
    std::string              profile;                           ///< Profile used with GSA compilations. Target in DX
    int                      dx_adapter;                        ///< ID of GPU adapter to use for DX.
    unsigned int             dx_flags;                          ///< Flags to pass to D3DCompile.
    std::string              dx_compiler_location;                        ///< D3DCompiler dll location
    std::string              fxc;                               ///< FXC path and arguments
    std::string              dump_ms_intermediate;              /// the location where to save the ms blob as text
    bool                     enable_shader_intrinsics;          /// true to enable DX shader intrinsics.
    bool                     dxbc_input_dx11;                   /// true if input file should be treated as a DXBC binary.
    bool                     amdil_input = false;               /// true when the input language is AMDIL rather than HLSL.
    int                      uav_slot;                          /// User-defined UAV slot for shader intrinsics.

    // DX12.
    std::string vs_hlsl;                                        ///< Full path to the hlsl file where the vertex shader is defined.
    std::string hs_hlsl;                                        ///< Full path to the hlsl file where the hull shader is defined.
    std::string ds_hlsl;                                        ///< Full path to the hlsl file where the domain shader is defined.
    std::string gs_hlsl;                                        ///< Full path to the hlsl file where the geometry shader is defined.
    std::string ps_hlsl;                                        ///< Full path to the hlsl file where the pixel shader is defined.
    std::string cs_hlsl;                                        ///< Full path to the hlsl file where the compute shader is defined.
    std::string all_hlsl;                                       ///< Full path to the hlsl file to be used for any present stage for which no hlsl input was provided.

    std::string vs_dxbc;                                        ///< Full path to the compiled DXBC binary where the vertex shader is defined.
    std::string hs_dxbc;                                        ///< Full path to the compiled DXBC binary where the hull shader is defined.
    std::string ds_dxbc;                                        ///< Full path to the compiled DXBC binary where the domain shader is defined.
    std::string gs_dxbc;                                        ///< Full path to the compiled DXBC binary where the geometry shader is defined.
    std::string ps_dxbc;                                        ///< Full path to the compiled DXBC binary where the pixel shader is defined.
    std::string cs_dxbc;                                        ///< Full path to the compiled DXBC binary where the compute shader is defined.

    std::string vs_dxil_disassembly;                            ///< Full path to the DXIL disassembly output file for vertex shader.
    std::string hs_dxil_disassembly;                            ///< Full path to the DXIL disassembly output file for hull shader.
    std::string gs_dxil_disassembly;                            ///< Full path to the DXIL disassembly output file for domain shader.
    std::string ds_dxil_disassembly;                            ///< Full path to the DXIL disassembly output file for geometry shader.
    std::string ps_dxil_disassembly;                            ///< Full path to the DXIL disassembly output file for pixel shader.
    std::string cs_dxil_disassembly;                            ///< Full path to the DXIL disassembly output file for compute shader.

    std::string vs_entry_point;                                 ///< Entry point name of vertex shader.
    std::string hs_entry_point;                                 ///< Entry point name of hull shader.
    std::string ds_entry_point;                                 ///< Entry point name of domain shader.
    std::string gs_entry_point;                                 ///< Entry point name of geometry shader.
    std::string ps_entry_point;                                 ///< Entry point name of pixel shader.
    std::string cs_entry_point;                                 ///< Entry point name of compute shader.

    std::string vs_model;                                       ///< Shader model for vertex shader.
    std::string hs_model;                                       ///< Shader model for hull shader.
    std::string ds_model;                                       ///< Shader model for domain shader.
    std::string gs_model;                                       ///< Shader model for geometry shader.
    std::string ps_model;                                       ///< Shader model for pixel shader.
    std::string cs_model;                                       ///< Shader model for compute shader.
    std::string all_model;                                      ///< Shader model for all stages.

    std::string pso_dx12;                                       ///< Full path to the pipeline state description file (graphics only).
    std::string pso_dx12_template;                              ///< Full path to where to save the template pipeline state description file.
    std::string rs_bin;                                         ///< Full path to the binary file that contains the compiled root signature.
    std::string rs_hlsl;                                        ///< Full path to the hlsl file that contains the root signature definition.
    std::string rs_macro;                                       ///< The name of the RootSignature macro in the HLSL code.
    std::string rs_macro_version;                               ///< Version of the hlsl-defined root signature.
    std::string elf_dis;                                        ///< Full path to output file for amdgpu-dis disassembly of pipeline binary.
    std::string dxc_path;                                       ///< Full path to DXC (provided by user through plug&play feature).
    std::string dxc_opt;                                        ///< Additional options to be passed to DXC when performing front-end compilation.
    std::string dxc_opt_file;                                   ///< Full path to file from which the additional options to be passed to DXC when performing front-end compilation.
    std::string dx12_autogen_dir;                               ///< Full path to folder where autogenerated files will be stored.

    // DX12: DXR.
    std::string dxr_state_desc;                                 ///< Full path to the DXR state description file.
    std::string dxr_hlsl;                                       ///< Full path to the DXR HLSL input file.
    std::string dxr_shader_model;                               ///< DXR shader model: shader model to be used for DXR front-end compilation.

    // DX12: debug layer.
    bool dx12_debug_layer_enabled  = false;                     ///< True to enable D3D12 debug layer.
    bool dx12_no_debug_output      = false;                     ///< False to capture debug output for D3D12 debug layer.

    // DX12: offline session.
    bool dx12_offline_session = false;                          ///< True to trigger DX12 offline mode.
    std::string alternative_amdxc;                              ///< Alternative version of amdxc64.dll to be used in offline mode.

    // Vulkan.
    std::string              vertex_shader;                     ///< Vertex shader full path
    std::string              tess_control_shader;               ///< Tessellation control shader full path
    std::string              tess_evaluation_shader;            ///< Tessellation evaluation shader full path
    std::string              geometry_shader;                   ///< Geometry shader full path
    std::string              fragment_shader;                   ///< Fragment shader full path
    std::string              compute_shader;                    ///< Compute shader full path
    std::string              pipe_file;                         ///< .pipe shader full path
    std::string              pso;                               ///< Vulkan Pipeline State Object file.
    std::string              spv_bin;                           ///< Path to SPIR-V binary file.
    std::string              icd_file;                          ///< Full path to an alternative Vulkan ICD to load instead of loader.
    std::string              loader_debug;                      ///< Value for the VK_LOADER_DEBUG environment variable.
    std::string              glslang_opt;                       ///< Additional options for glslang (the Vulkan front-end compiler).
    std::string              spv_txt;                           ///< Path to SPIR-V text file.
    std::string              vulkan_validation;                 ///< Path to output Vulkan validation info file.
    std::string              parsed_spv;                        ///< Path to parsed SPIR-V text output file.
    RgVulkanInputType        vert_shader_file_type;             ///< Type of vertex shader file.
    RgVulkanInputType        tesc_shader_file_type;             ///< Type of tessellation control shader file.
    RgVulkanInputType        tese_shader_file_type;             ///< Type of tessellation evaluation shader file.
    RgVulkanInputType        frag_shader_file_type;             ///< Type of fragment shader file.
    RgVulkanInputType        geom_shader_file_type;             ///< Type of geometry shader file.
    RgVulkanInputType        comp_shader_file_type;             ///< Type of compute shader file.
    bool                     is_hlsl_input;                     ///< Input files are HLSL shaders.
    bool                     is_glsl_input;                     ///< Input files are GLSL shaders.
    bool                     is_spv_input;                      ///< Input files are SPIR-V binary files.
    bool                     is_spv_txt_input;                  ///< Input files are SPIR-V text files.

    // Compiler paths
    std::string              compiler_bin_path;                 ///< Path to user-provided compiler "bin" folder.
    std::string              compiler_inc_path;                 ///< Path to user-provided compiler "include" folder.
    std::string              compiler_lib_path;                 ///< Path to user-provided compiler "lib" folder.

    // General CLI option.
    bool                     is_parsed_isa_required;            ///< True to generate "parsed" ISA in CSV format.
    bool                     is_line_numbers_required;          ///< True to generate source lines in the ISA disassembly.
    bool                     is_warnings_required;              ///< True to print warnings reported by the compiler.
    bool                     print_process_cmd_line;            ///< True to print command lines that RGA uses to launch external processes.
};

#endif // RGA_RADEONGPUANALYZERCLI_SRC_KC_CONFIG_H_

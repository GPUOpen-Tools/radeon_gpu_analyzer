//======================================================================
// Copyright 2020-2023 Advanced Micro Devices, Inc. All rights reserved.
//======================================================================

#define _HAS_AUTO_PTR_ETC 1

// C++.
#include <iostream>
#include <string>
#include <algorithm>
#include <cctype>
#include <limits>

// cxxopts.
#include "cxxopts/include/cxxopts.hpp"

#ifdef CMAKE_BUILD
#include "RadeonGPUAnalyzerCLIConfig.h"
#endif

// Local.
#include "radeon_gpu_analyzer_cli/kc_config.h"
#include "radeon_gpu_analyzer_cli/kc_parse_cmd_line.h"
#include "radeon_gpu_analyzer_cli/kc_cli_string_constants.h"
#include "radeon_gpu_analyzer_cli/kc_utils.h"

// Shared between CLI and GUI.
#include "common/rga_version_info.h"
#include "common/rga_cli_defs.h"

namespace po = cxxopts;

#if GSA_BUILD
#define KERNEL_OPTION "function"
#else
#define KERNEL_OPTION "kernel"
#endif

// Constants.
static const char* kStrErrorBothCfgAndCfgiSpecified = "Error: only one of \"--cfg\" and \"--cfg-i\" options can be specified.";
static const char* kStrErrorNoModeSpecified = "No mode specified. Please specify mode using - s <arg>.";
static const char* kStrDxAdaptersHelpCommonText = "This is only relevant if you have multiple display adapters installed on your system, and you would like RGA to use the driver which is associated with "
"a non-primary display adapter.By default RGA will use the driver that is associated with the primary display adapter.";
static const char* kStrInfoLegacyOpenclNoLongerSupported = "Legacy OpenCL mode (-s cl) is no longer supported.";

// Options Types Strings
static const char* generic_opt = "Generic";
static const char* dx_opt = "DirectX 11 mode";
static const char* macro_and_include_opt = "Macro and Include paths";
static const char* cl_opt = "Open CL";
static const char* pipelined_opt_offline = "Input shader type";
static const char* vulkan_opt = "Vulkan mode";
static const char* vulkan_input_type_opt = "Vulkan input type - specific hidden";
static const char* vulkan_opt_hidden = "Vulkan-specific hidden";
static const char* dxc_options = "DXC";
static const char* dx12_other_options = "Other DX12";
static const char* dx12_opt = "DirectX 12 mode";
static const char* dxr_opt = "DXR mode";
static const char* binary_opt = "Binary Analysis mode";
static const char* legacy_cl_opt = "Legacy CL";
static const char* il_dump_opt = "Il Dump";
static const char* line_numbers_opt = "Line Numbers";
static const char* warnings_opt = "Compiler Warnings";
static const char* opt_level_opt1 = "Optimization Level";
static const char* opt_level_opt2 = "Optimization Levels 2";
static const char* opencl_offline_compiler_paths_opt = "Alternative OpenCL Lightning Compiler";
static const char* compiler_paths_opt = "Alternative glslang compiler and SPIR-V tools.\nRGA uses the glslang package that it ships with as the default front-end compiler for Vulkan.\nUse this option to provide a custom glslang package";
static const char* hidden_opt = "Options that we don't show with --help.";

// Options and descriptions strings.
static const char* kStrOptionListAsic      = "l,list-asics";
static const char* kStrDescriptionListAsic = "List the known GPU codenames, architecture names and variant names. To target a specific GPU, use its codename as the argument to the \"-c\" command line switch.";
static const char* kStrOptionAsic          = "c,asic";
static const char* kStrDescriptionAsic     = "Which ASIC to target.  Repeatable.";
static const char* kStrOptionVersion       = "version";
static const char* kStrDescriptionVersion  = "Print version string.";
static const char* kStrOptionHelp          = "h,help";
static const char* kStrDescriptionHelp     = "Produce this help message.";
static const char* kStrOptionAnalysis      = "a,analysis";
static const char* kStrDescriptionAnalysis = "Path to output analysis file.";
static const char* kStrOptionBinary        = "b,binary";
static const char* kStrDescriptionBinary   = "Path to ELF binary output file.";
static const char* kStrOptionIsa           = "isa";
static const char* kStrDescriptionIsa      = "Path to output ISA disassembly file(s).";
static const char* kStrOptionLivereg       = "livereg";
static const char* kStrDescriptionLivereg  = "Path to live register analysis output file(s).";
static const char* kStrOptionSgpr          = "livereg-sgpr";
static const char* kStrDescriptionSgpr     = "Path to live register sgpr analysis output file(s).";
static const char* kStrOptionCfg           = "cfg";
static const char* kStrDescriptionCfg      = "Path to per-block control flow graph output file(s).";
static const char* kStrOptionCfgI          = "cfg-i";
static const char* kStrDescriptionCfgI     = "Path to per-instruction control flow graph output file(s).";
static const char* kStrOptionSourceKind    = "s,source-kind";
static const char* kStrDescriptionSourceKind =
    "Source platform: dx12 for DirectX 12, dxr for DXR, dx11 for DirectX 11, vulkan for Vulkan, opengl for OpenGL, "
    "opencl for OpenCL offline mode and amdil for AMDIL.";
static const char* kStrOptionUpdates       = "u,updates";
static const char* kStrDescriptionUpdates  = "Check for available updates.";
static const char* kStrOptionVerbose       = "v,verbose";
static const char* kStrDescriptionVerbose  = "Print command line strings that RGA uses to launch external processes.";
static const char* kStrOptionCO            = "co";
static const char* kStrDescriptionCO       = "Full path to the code object input file.";
static const char* kStrOptionIl            = "il";


bool ParseCmdLine(int argc, char* argv[], Config& config)
{
    //----- Command Line Options Parsing
    bool do_work = true;
    bool validation = false;

    try
    {
#if _WIN32
        const std::string program_name = "rga.exe";
#else
        const std::string program_name = "rga";
#endif
        // Generic Options: Valid got CL, DX, GL
        po::Options opts(program_name);
        opts.positional_help("");
        opts.custom_help("[options]");

        opts.add_options(generic_opt)
            // The next two options here can be repeated, so they are vectors.
            (kStrOptionListAsic, kStrDescriptionListAsic)
            (kStrOptionAsic, kStrDescriptionAsic, po::value<std::vector<std::string>>(config.asics))
            (kStrOptionVersion, kStrDescriptionVersion)
            (kStrOptionHelp, kStrDescriptionHelp)
            (kStrOptionAnalysis, kStrDescriptionAnalysis, po::value<std::string>(config.analysis_file))
            (kStrOptionBinary, kStrDescriptionBinary, po::value<std::string>(config.binary_output_file))
            (kStrOptionIsa, kStrDescriptionIsa, po::value<std::string>(config.isa_file))
            (kStrOptionLivereg, kStrDescriptionLivereg, po::value<std::string>(config.livereg_analysis_file))
            (kStrOptionSgpr, kStrDescriptionSgpr, po::value<std::string>(config.sgpr_livereg_analysis_file))
            (kStrOptionCfg, kStrDescriptionCfg, po::value<std::string>(config.block_cfg_file))
            (kStrOptionCfgI, kStrDescriptionCfgI, po::value<std::string>(config.inst_cfg_file))
            (kStrOptionSourceKind, kStrDescriptionSourceKind, po::value<std::string>(config.source_kind))
            (kStrOptionUpdates, kStrDescriptionUpdates)
            (kStrOptionVerbose, kStrDescriptionVerbose)
            ;

        // DX Options
        std::string adapters_desc = "List all of the supported display adapters that are installed on the system. " + std::string(kStrDxAdaptersHelpCommonText);
        std::string set_adapter_desc = "Specify the id of the display adapter whose driver you would like RGA to use. " + std::string(kStrDxAdaptersHelpCommonText);

        opts.add_options(dx_opt)
            ("f,function", "DX shader entry point.", po::value<std::string>(config.function))
            ("p,profile", "Profile to use for compilation.  This option is only required in case that the input is in HLSL. For example: vs_5_0, ps_5_0, etc.",
              po::value<std::string>(config.profile))
            ("DXFlags", "Flags to pass to D3DCompile.", po::value<unsigned int>(config.dx_flags))
            ("DXLocation", "Location to the D3DCompiler Dll required for compilation. If none is specified, the default D3D compiler that is bundled with the Analyzer will be used.",
              po::value<std::string>(config.dx_compiler_location))
            ("FXC", "FXC Command Line. Use full path and specify all arguments in \"\". For example: "
                                                      "   rga.exe  -f VsMain1 -s DXAsm -p vs_5_0 <Path>\\vsBlob.obj  --isa <Path>\\vsTest.isa --FXC \"<Path>\\fxc.exe /E VsMain1 /T vs_5_0 /Fo <Path>\\vsBlob.obj <Path>\\vsTest.fx\" "
                                                      "   In order to use it, DXAsm must be specified. /Fo switch must be used and output file must be the same as the input file for rga.",
              po::value<std::string>(config.fxc))
            ("DumpMSIntermediate", "Location to save the MS Blob as text. ", po::value<std::string>(config.dump_ms_intermediate))
            ("intrinsics", "Enable AMD D3D11 Shader Intrinsics extension.")
            ("adapters", adapters_desc.c_str())
            ("set-adapter", set_adapter_desc.c_str(), po::value<int>(config.dx_adapter))
            ("UAVSlot", "This value should be in the range of [0,63]. The driver will use the slot to track which UAV is being used to specify the intrinsic. The UAV slot that is selected cannot be used for any other purposes. This option is only relevant when AMD D3D11 Shader Intrinsics is enabled (specify --intrinsics).",
                po::value<int>(config.uav_slot))
            ("dxbc", "Treat input file as a DXBC binary.")
            ;

        // Macro and Include paths Options
        opts.add_options(macro_and_include_opt)
            // The next two options here can be repeated, so they are vectors.
            ("D,define", "Define symbol or symbol=value. Repeatable.", po::value<std::vector<std::string>>(config.defines))
            ("I,IncludePath", "Additional include path required for compilation.  Repeatable.", po::value<std::vector<std::string> >(config.include_path))
            ;

        // CL Option
        opts.add_options(cl_opt)
            ("list-kernels", "List kernel functions.")
            ("m,metadata", "Path to output Metadata file(s). "
                "Requires --" KERNEL_OPTION ".", po::value<std::string>(config.metadata_file))
            ("k,kernel", "Kernel to be compiled and analyzed. If not specified, all kernels will be targeted. Note "
                "that this option is only relevant when the compiler can generate per-kernel data. When the runtime uses the updated compiler to generate "
                "a Code Object type binary, the ISA would be generated for the entire program, since the binary has no per-kernel code sections.\n",
              po::value<std::string>(config.function))
            ("OpenCLoption", "OpenCL compiler options.  Repeatable.", po::value<std::vector<std::string>>(config.opencl_options))
            ;

        // Vulkan shader type.
        opts.add_options(pipelined_opt_offline)
            ("vert", "Full path to vertex shader input file.", po::value<std::string>(config.vertex_shader))
            ("tesc", "Full path to tessellation control shader input file.", po::value<std::string>(config.tess_control_shader))
            ("tese", "Full path to tessellation evaluation shader input file.", po::value<std::string>(config.tess_evaluation_shader))
            ("geom", "Full path to geometry shader input file.", po::value<std::string>(config.geometry_shader))
            ("frag", "Full path to fragment shader input file", po::value<std::string>(config.fragment_shader))
            ("comp", "Full path to compute shader input file.", po::value<std::string>(config.compute_shader))
            ;

#ifdef RGA_ENABLE_VULKAN
        // Prepare the --glslang-opt description.
        std::string glslang_opt_description = kStrCliOptGlslangOptDescriptionA;
        glslang_opt_description.append(kStrCliOptGlslangOptDescriptionB);

        // Live Vulkan mode options.
        opts.add_options(vulkan_opt)
            ("stage-glsl", "Full path to [stage] input file, while forcing interpretation of input file as glsl source file rather than SPIR-V binary. arg is the full path to stage shader input file. stage can be one "
                "of: \"vert\", \"tesc\", \"tese\", \"geom\", \"frag\", \"comp\". "
                "For example, use \"--vert-glsl shader.glsl\" to specify path to the vertex shader file and indicate that it is a glsl source file (rather than a SPIR-V binary).",
                po::value<std::string>())
            ("stage-spvas", "Full path to [stage] input file, while forcing interpretation of input file as SPIR-V textual source file rather than SPIR-V binary. arg is the full path to stage shader input file. stage can be one "
                "of: \"vert\", \"tesc\", \"tese\", \"geom\", \"frag\", \"comp\". "
                "For example, use \"--vert-spvas shader.txt\" to specify path to the vertex shader file and indicate that it is a SPIR-V textual source file (rather than a SPIR-V binary).",
                po::value<std::string>())
            ("pso", "Full path to .gpso (graphics) or .cpso (compute) pipeline state JSON file. If no pipeline state file is provided, a default pipeline state would be used. In that case, if your shaders do not match the default state, the compilation would fail.",
                po::value<std::string>(config.pso))
            ("icd", kStrCliOptIcdDescription, po::value<std::string>(config.icd_file))
            ("loader-debug", kStrCliOptVkLoaderDebugDescription, po::value<std::string>(config.loader_debug))
            ("glslang-opt", glslang_opt_description.c_str(), po::value<std::string>(config.glslang_opt))
            ("disassemble-spv", "Disassemble SPIR-V binary file. Accepts an optional argument with the full path to the output file where the disassembly would be saved. If not specified, the disassembly would be printed to stdout.", po::value<std::string>(config.spv_txt))
            ("validation", "Enable Vulkan validation layers and dump the output of validation layers to stdout.", po::value<bool>(validation))
            ("validation-file", "Enable Vulkan validation layers and dump the output of validation layers to the text file specified by the option argument.", po::value<std::string>(config.vulkan_validation))
#ifdef VK_HLSL
                ("list-entries", "List hlsl function names.");
#else
            ;
#endif
        // Per-stage input file types ("--stage-[file-type]").
        opts.add_options(vulkan_input_type_opt)
            ("vert-glsl", "", po::value<std::string>())("vert-spvas", "", po::value<std::string>())
            ("tesc-glsl", "", po::value<std::string>())("tesc-spvas", "", po::value<std::string>())
            ("tese-glsl", "", po::value<std::string>())("tese-spvas", "", po::value<std::string>())
            ("frag-glsl", "", po::value<std::string>())("frag-spvas", "", po::value<std::string>())
            ("geom-glsl", "", po::value<std::string>())("geom-spvas", "", po::value<std::string>())
            ("comp-glsl", "", po::value<std::string>())("comp-spvas", "", po::value<std::string>())
            ;

        // Vulkan hidden options (are not presented to the user in -h).
        opts.add_options(vulkan_opt_hidden)
            ("assemble-spv", "Assemble SPIR-V textual code to a SPIR-V binary file. arg is the full path to the output SPIR-V binary file.",
                po::value<std::string>(config.spv_bin))
            ("parse-spv", "Parse SPIR-V binary file. The option argument is the full path to the output text file with SPIR-V binary info. "
            "If path is not specified, the SPIR-V info will be printed to stdout.",
                po::value<std::string>(config.parsed_spv))
            ;
#endif

        // DX12-specific.
        // DXC.
        opts.add_options(dxc_options)
            ("dxc", "Path to an alternative DXC package (a folder containing DXC.exe, dxcompiler.dll and dxil.dll). If specified, RGA would use DXC from the given path rather than the package that it ships with.",
                po::value<std::string>(config.dxc_path))
            ("dxc-opt", "Additional options to be passed to DXC when performing front-end compilation.",
                po::value<std::string>(config.dxc_opt))
            ("dxc-opt-file", "Full path to text file from which to read the additional options to be passed to DXC when performing front-end compilation. Since specifying verbose options which include macro "
                "definitions can be tedious in the command line, and at times even impossible, you can use this option to specify the DXC command line arguments in a text file which would be read by RGA and passed as-is to DXC in the front-end compilation step.",
                po::value<std::string>(config.dxc_opt_file));
        ;

        // D3D12 debug layer.
        opts.add_options(dx12_other_options)
            ("debug-layer", "Enable the D3D12 debug layer.")
            ("no-debug-output", "If enabled RGA will not intercept the debug output for --debug-layer.")
        // DX12 offline mode.
            ("offline", "Assume no AMD display adapter is installed.")
            ("amdxc", "Path to a an alternative amdxc64.dll to be loaded in offline mode (must be used together with --offline). "
                "If provided, the alternative amdxc64.dll would be used as the DX12 offline driver, instead of the driver that is bundled with the tool.",
                 po::value<std::string>(config.alternative_amdxc))
            ;

        // DX12 graphics/compute.
        opts.add_options(dx12_opt)
            // HLSL.
            ("vs", "Full path to hlsl file where vertex shader is defined.", po::value<std::string>(config.vs_hlsl))
            ("hs", "Full path to hlsl file where hull shader is defined.", po::value<std::string>(config.hs_hlsl))
            ("ds", "Full path to hlsl file where domain shader is defined.", po::value<std::string>(config.ds_hlsl))
            ("gs", "Full path to hlsl file where geometry shader is defined.", po::value<std::string>(config.gs_hlsl))
            ("ps", "Full path to hlsl file where pixel shader is defined.", po::value<std::string>(config.ps_hlsl))
            ("cs", "Full path to hlsl file where compute shader is defined.", po::value<std::string>(config.cs_hlsl))
            ("all-hlsl", "Full path to the hlsl file to be used for all stages. "
                "You can use this option if all of your shaders are defined in the same hlsl file, to avoid repeating the --<stage> "
                "argument. If you use this option in addition to --<stage> or --<stage>-blob, then the --<stage> or --<stage>-blob option "
                "would override this option for stage <stage>.",
                po::value<std::string>(config.all_hlsl))

            // DXBC.
            ("vs-blob", "Full path to compiled DXBC or DXIL binary where vertex shader is found.", po::value<std::string>(config.vs_dxbc))
            ("hs-blob", "Full path to compiled DXBC or DXIL binary where hull shader is found.", po::value<std::string>(config.hs_dxbc)) 
            ("ds-blob", "Full path to compiled DXBC or DXIL binary domain shader is found.",  po::value<std::string>(config.ds_dxbc))
            ("gs-blob", "Full path to compiled DXBC or DXIL binary geometry shader is found.", po::value<std::string>(config.gs_dxbc))
            ("ps-blob", "Full path to compiled DXBC or DXIL binary pixel shader is found.",  po::value<std::string>(config.ps_dxbc))
            ("cs-blob", "Full path to compiled DXBC or DXIL binary where compute shader is found.", po::value<std::string>(config.cs_dxbc))
            
            // IL Disassembly.
            ("vs-dxil-dis", "Full path to the DXIL or DXBC disassembly output file for vertex shader. "
                "Note that this option is only valid for textual input (HLSL).",
                po::value<std::string>(config.vs_dxil_disassembly))
            ("hs-dxil-dis", "Full path to the DXIL or DXBC disassembly output file for hull shader. "
                "Note that this option is only valid for textual input (HLSL).",
                po::value<std::string>(config.hs_dxil_disassembly))
            ("ds-dxil-dis", "Full path to the DXIL or DXBC disassembly output file for domain shader. "
                "Note that this option is only valid for textual input (HLSL).",
                po::value<std::string>(config.ds_dxil_disassembly))
            ("gs-dxil-dis", "Full path to the DXIL or DXBC disassembly output file for geometry shader. "
                "Note that this option is only valid for textual input (HLSL).",
                po::value<std::string>(config.gs_dxil_disassembly))
            ("ps-dxil-dis", "Full path to the DXIL or DXBC disassembly output file for pixel shader. "
                "Note that this option is only valid for textual input (HLSL).",
                po::value<std::string>(config.ps_dxil_disassembly))
            ("cs-dxil-dis", "Full path to the DXIL or DXBC disassembly output file for compute shader. "
                "Note that this option is only valid for textual input (HLSL).",
                po::value<std::string>(config.cs_dxil_disassembly))

            // Target.
            ("vs-entry", "Entry-point name of vertex shader.", po::value<std::string>(config.vs_entry_point))
            ("hs-entry", "Entry-point name of hull shader.", po::value<std::string>(config.hs_entry_point))
            ("ds-entry", "Entry-point name of domain shader.", po::value<std::string>(config.ds_entry_point))
            ("gs-entry", "Entry-point name of geometry shader.", po::value<std::string>(config.gs_entry_point))
            ("ps-entry", "Entry-point name of pixel shader.", po::value<std::string>(config.ps_entry_point))
            ("cs-entry", "Entry-point name of compute shader.", po::value<std::string>(config.cs_entry_point))

            // Shader model.
            ("vs-model", "Shader model of vertex shader (e.g. vs_5_1 or vs_6_0).", po::value<std::string>(config.vs_model))
            ("hs-model", "Shader model of hull shader (e.g. hs_5_1 or hs_6_0).", po::value<std::string>(config.hs_model))
            ("ds-model", "Shader model of domain shader (e.g. ds_5_1 or ds_6_0).", po::value<std::string>(config.ds_model))
            ("gs-model", "Shader model of geometry shader (e.g. gs_5_1 or gs_6_0).", po::value<std::string>(config.gs_model))
            ("ps-model", "Shader model of pixel shader (e.g. ps_5_1 or ps_6_0).", po::value<std::string>(config.ps_model))
            ("cs-model", "Shader model of compute shader (e.g. cs_5_1 or cs_6_0).", po::value<std::string>(config.cs_model))
            ("all-model", "Shader model to be used for all stages (e.g. 5_1 or 6_0). Instead of "
                "specifying the model for each stage specifically, you can use this option to pass the shader model version and RGA "
                "would auto-generate the relevant model for every stage in the pipeline. Note that if you use this option together "
                "with any of the <stage>-model options, for any stage <stage> the <stage>-model option would override --all-model.",
                    po::value<std::string>(config.all_model))
            
            // Root signature.
            ("rs-bin", "Full path to the serialized root signature "
                "to be used in the compilation process.",
                po::value<std::string>(config.rs_bin))
            ("rs-hlsl", "Full path to the HLSL file where the "
                "RootSignature macro is defined. If there is only a "
                "single hlsl input file, this option is not required.",
                 po::value<std::string>(config.rs_hlsl))
            ("rs-macro", "The name of the RootSignature macro in the HLSL "
                "code. If specified, the root signature would be compiled from the HLSL source. Use this if your "
                "shader does not include the [RootSignature()] attribute but has a root signature macro defined"
                "in the source code. Please also check the description for --rs-hlsl, which is related to this option.",
                po::value<std::string>(config.rs_macro))
            ("rs-macro-version", "The version of the RootSignature macro "
                "specified through the rs - macro option.By default, 'rootsig_1_1' would be assumed.",
                po::value<std::string>(config.rs_macro_version))
            
            // Pipeline state.
            ("gpso", "Full path to .gpso file that describes the graphics pipeline state (required for graphics pipelines only)."
                " You can generate a template by using the --gpso-template option.",
                po::value<std::string>(config.pso_dx12))
            ("gpso-template", "Full path to where to save the template pipeline state description file. You can then edit the file to match your pipeline.",
                po::value<std::string>(config.pso_dx12_template))
            ("elf-dis", "Full path to output text file where disassembly of the pipeline ELF binary would be saved.",
                po::value<std::string>(config.elf_dis))
            ;

        // DXR-specific.
        opts.add_options(dxr_opt)
            ("hlsl", "Full path to DXR HLSL input file that contains the state definition.", po::value<std::string>(config.dxr_hlsl))
            ("mode", "DXR mode: 'shader' to compile a specific shader, or 'pipeline' to compile all pipelines in the State Object. By default, shader mode is assumed.", po::value<std::string>(config.dxr_mode))
            ("export", "The export name of the shader for which to retrieve results - only relevant to shader mode (--mode shader)."
                " this can be an export name of any shader in the state object.",
                po::value<std::vector<std::string>>(config.dxr_exports))
            ("dxr-model", "Shader model used for DXR HLSL compilation. Use this option to override "
                "the shader model that is used for HLSL compilation by default (lib_6_3).",
                po::value<std::string>(config.dxr_shader_model))
            ;

        // Binary Analysis mode.
        opts.add_options(binary_opt)
            (kStrOptionCO, kStrDescriptionCO, po::value<std::string>(config.binary_codeobj_file))
            ;

#ifdef _LEGACY_OPENCL_ENABLED
        // Legacy OpenCL options.
        opts.add_options(legacy_cl_opt)
            ("suppress", "Section to omit from binary output.  Repeatable. Available options: .source, .amdil, .debugil,"
                ".debug_info, .debug_abbrev, .debug_line, .debug_pubnames, .debug_pubtypes, .debug_loc, .debug_str,"
                ".llvmir, .text\nNote: Debug sections are valid only with \"-g\" compile option",
                 po::value<vector<std::string> >(config.suppress_section));
#endif // _LEGACY_OPENCL_ENABLED

        // IL dump.
        opts.add_options(il_dump_opt)
            (kStrOptionIl, "Path to output IL (intermediate language) disassembly file(s).", po::value<std::string>(config.il_file))
            ;

        // Line numbers.
        opts.add_options(line_numbers_opt)
            ("line-numbers", "Add source line numbers to ISA disassembly.");

        // Compiler warnings.
        opts.add_options(warnings_opt)
            ("w,warnings", "Print warnings reported by the compiler.");

        // Optimization Levels.
        opts.add_options(opt_level_opt1)
            ("O0", "Disable optimizations")
            ("O1", "Enable minimal optimizations");
        opts.add_options(opt_level_opt2)
            ("O2", "Optimize for speed")
            ("O3", "Apply full optimization");

        // Paths to alternative glslang/spirv-tools for Vulkan or LightningCompiler.
        opts.add_options(compiler_paths_opt)
            ("compiler-bin", "", po::value<std::string>(config.compiler_bin_path))
            ;

        // Paths to user-provided LightningCompiler inc/lib.
        opts.add_options(opencl_offline_compiler_paths_opt)
            ("compiler-inc", kStrCliDescAlternativeLightningCompilerIncFolder, po::value<std::string>(config.compiler_inc_path))
            ("compiler-lib", kStrCliDescAlternativeLightningCompilerLibFolder, po::value<std::string>(config.compiler_lib_path))
            ;

        // "Hidden" options.
        opts.add_options(hidden_opt)
            ("input", "Input for analysis.", po::value<std::vector<std::string>>(config.input_files))
            ("version-info", "Generate RGA CLI version info file. If no file name is provided, the version info is printed to stdout.")
            ("session-metadata", "Generate session metadata file with the list of output files generated by RGA.", po::value<std::string>(config.session_metadata_file))
            ("log",  "Path to the CLI log file", po::value<std::string>(config.log_file))
            ("no-suffix-bin", "If specified, do not add a suffix to names of generated binary files.")
            ("no-prefix-device-bin", "If specified, do not add a device prefix to names of generated binary files.")
			("state-desc", "Full path to the DXR state description file.", po::value<std::string>(config.dxr_state_desc))
            ("parse-isa", "Generate a CSV file with a breakdown of each ISA instruction into opcode, operands. etc.")
            ("csv-separator", "Override to default separator for analysis items.", po::value<std::string>(config.csv_separator))
            ("retain", "Retain temporary output files.")
            ("no-rename-il", "If specified, do not rename generated IL file.")
            ;

        opts.parse_positional("input");


        // Parse command line
        auto result = opts.parse(argc, argv);


        if (result.count("help") || result.arguments().empty() == 0)
        {
            config.requested_command = Config::kHelp;
        }

        if (result.count("retain"))
        {
            config.should_retain_temp_files = true;
        }

        if (result.count("no-rename-il"))
        {
            config.donot_rename_il_files = true;
        }

        if (result.count("list-asics"))
        {
            config.requested_command = Config::kListAsics;
        }
        else if (result.count("list-kernels") || result.count("list-entries"))
        {
            config.requested_command = Config::kListEntries;
        }
        else if (result.count("gpso-template"))
        {
            config.requested_command = Config::kGenTemplateFile;
        }

        if (result.count("intrinsics"))
        {
            config.enable_shader_intrinsics = true;
        }

        if (result.count("dxbc"))
        {
            config.dxbc_input_dx11 = true;
        }

        if (result.count("adapters"))
        {
            config.requested_command = Config::kListAdapters;
        }

        if (result.count("parse-isa"))
        {
            config.is_parsed_isa_required = true;
        }

        if (result.count("line-numbers"))
        {
            config.is_line_numbers_required = true;
        }

        if (result.count("warnings"))
        {
            config.is_warnings_required = true;
        }

        if (result.count("verbose"))
        {
            config.print_process_cmd_line = true;
        }

        if (result.count("debug-layer"))
        {
            config.dx12_debug_layer_enabled = true;
        }

        if (result.count("no-debug-output"))
        {
            if (!config.dx12_debug_layer_enabled)
            {
                std::cout << "Error: \'no-debug-output\' option is only valid when used with --debug-layer." << std::endl;
                do_work = false;
            }
            else
            {
                config.dx12_no_debug_output = true;
            }
        }

        if (result.count("offline") || !config.alternative_amdxc.empty())
        {
            config.dx12_offline_session = true;
        }

        if (result.count("cfg") && result.count("cfg-i"))
        {
            std::cerr << kStrErrorBothCfgAndCfgiSpecified << std::endl;
            do_work = false;
        }

        // Set the optimization level.
        if (result.count("O0"))
        {
            config.opt_level = 0;
        }
        else if (result.count("O1"))
        {
            config.opt_level = 1;
        }
        else if (result.count("O2"))
        {
            config.opt_level = 2;
        }
        else if (result.count("O3"))
        {
            config.opt_level = 3;
        }

        // Set the default livereg output file name if not provided by a user.
        if (result.count("livereg") && config.livereg_analysis_file.empty())
        {
            config.livereg_analysis_file = kStrDefaultFilenameLivereg;
        }

        // Set the default sgpr livereg output file name if not provided by a user.
        if (result.count("livereg-sgpr") && config.sgpr_livereg_analysis_file.empty())
        {
            config.sgpr_livereg_analysis_file = kStrDefaultFilenameLiveregSgpr;
        }        

        if (result.count("no-suffix-bin") > 0)
        {
            config.should_avoid_binary_suffix = true;
        }

        if (result.count("no-prefix-device-bin"))
        {
            config.should_avoid_binary_device_prefix = true;
        }

        if (result.count("version-info"))
        {
            config.version_info_file = (result.count("input") ? config.input_files[0] : "");
            config.requested_command = Config::kGenVersionInfoFile;
        }
        else if (result.count("version"))
        {
            config.requested_command = Config::kVersion;
        }
        else if (result.count("updates"))
        {
            config.requested_command = Config::kUpdate;
        }
        else if (!config.analysis_file.empty() || 
                 !config.il_file.empty() || 
                 !config.isa_file.empty() ||
                 !config.livereg_analysis_file.empty() || 
                 !config.sgpr_livereg_analysis_file.empty() || 
                 !config.binary_output_file.empty() ||
                 !config.metadata_file.empty() || 
                 !config.block_cfg_file.empty() ||
                 !config.inst_cfg_file.empty() || 
                 !config.spv_txt.empty() || 
                 !config.spv_bin.empty() ||
                 !config.parsed_spv.empty() ||
                 !config.binary_codeobj_file.empty())
        {
            config.requested_command = Config::kCompile;
        }

        if (config.requested_command == Config::kNone)
        {
            std::cout << kStrErrorNoValidCommandDetected << std::endl;
        }

        // If the "--validation" option for Vulkan mode is specified, the validation info should be dumped to stdout.
        if (validation && result.count("validation-file") == 0)
        {
            config.vulkan_validation = kStrCliVkValidationInfoStdout;
        }

        // Select the mode (source language).
        bool is_source_specified = true;
        std::string src_kind = config.source_kind;
        std::transform(src_kind.begin(), src_kind.end(), src_kind.begin(), [](unsigned char c){return std::toupper(c);});
        if (config.source_kind.empty())
        {
            if (config.requested_command != Config::kHelp &&
                config.requested_command != Config::kVersion &&
                config.requested_command != Config::kUpdate &&
                config.requested_command != Config::kGenVersionInfoFile)
            {
                std::cout << kStrErrorNoModeSpecified << std::endl;
            }
            is_source_specified = false;
        }
        else if (src_kind == Config::source_kind_dx11)
        {
            config.mode = beKA::RgaMode::kModeDx11;
        }
        else if (src_kind == Config::source_kind_dx12)
        {
            config.mode = beKA::RgaMode::kModeDx12;
        }
        else if (src_kind == Config::source_kind_dxr)
        {
            config.mode = beKA::RgaMode::kModeDxr;
        }
        else if (src_kind == Config::source_kind_amdil)
        {
            config.mode = beKA::RgaMode::kModeAmdil;
        }
        else if (src_kind == Config::source_kind_opencl)
        {
            config.mode = beKA::RgaMode::kModeOpenclOffline;
        }
        else if (src_kind == Config::source_kind_opengl)
        {
            config.mode = beKA::RgaMode::kModeOpengl;
        }
        else if (src_kind == Config::source_kind_glsl_vulkan_offline)
        {
            config.mode = beKA::RgaMode::kModeVkOffline;
        }
        else if (src_kind == Config::source_kind_spirv_bin_offline)
        {
            config.mode = beKA::RgaMode::kModeVkOfflineSpv;
        }
        else if (src_kind == Config::source_kind_spirv_txt_offline)
        {
            config.mode = beKA::RgaMode::kModeVkOfflineSpvTxt;
        }
#ifdef RGA_ENABLE_VULKAN
        else if (src_kind == Config::source_kind_vulkan)
        {
            config.mode = beKA::RgaMode::kModeVulkan;
        }
#endif
        else if (src_kind == Config::source_kind_opencl)
        {
            config.mode = beKA::RgaMode::kModeOpenclOffline;
        }
        else if (src_kind == Config::source_kind_binary)
        {
            config.mode = beKA::RgaMode::kModeBinary;
        }
        else
        {
            config.mode = beKA::RgaMode::kModeInvalid;
            std::cout << "Source language: " << config.source_kind << " not supported.\n";
        }

#ifdef RGA_ENABLE_VULKAN
        // Set the Vulkan per-stage input file types.
        if (config.mode == beKA::RgaMode::kModeVulkan)
        {
            std::vector<std::map<std::string, RgVulkanInputType>> type_pairs_map = {
                std::map<std::string, RgVulkanInputType>{{"glsl", RgVulkanInputType::kGlsl}},
                std::map<std::string, RgVulkanInputType>{{"spvas", RgVulkanInputType::kSpirvTxt}}};
            for (const auto& stage :
                 std::map<std::string, std::pair<std::string*, RgVulkanInputType*>>
                {
                    {"vert", {&config.vertex_shader, &config.vert_shader_file_type}},
                    {"tesc", {&config.tess_control_shader, &config.tesc_shader_file_type}},
                    {"tese", {&config.tess_evaluation_shader, &config.tese_shader_file_type}},
                    {"frag", {&config.fragment_shader, &config.frag_shader_file_type}},
                    {"geom", {&config.geometry_shader, &config.geom_shader_file_type}},
                    {"comp", {&config.compute_shader, &config.comp_shader_file_type}}
                })
            {
                for (const auto& type_pair_map : type_pairs_map)
                {
                    for (const std::pair<std::string, RgVulkanInputType>& file_type : type_pair_map)
                    {
                        if (result.count(stage.first + "-" + file_type.first))
                        {
                            *(stage.second.first)  = result[stage.first + "-" + file_type.first].as<std::string>();
                            *(stage.second.second) = file_type.second;
                        }
                    }
                }
            }
        }
#endif

         // Binary Analysis mode does not support the asic,c and binary,b and --il CLI option.
        if (config.mode == beKA::RgaMode::kModeBinary)
        {
            if (result.count("asic"))
            {
                std::cout << "unrecognized option \'" << kStrOptionAsic << "\'" << std::endl;
                do_work = false;
            }
            if (result.count("binary"))
            {
                std::cout << "unrecognized option \'" << kStrOptionBinary << "\'" << std::endl;
                do_work = false;
            }
            if (result.count(kStrOptionIl))
            {
                std::cout << "unrecognized option \'" << kStrOptionIl << "\'" << std::endl;
                do_work = false;
            }
        }

        std::cout << std::endl;
        if ((config.requested_command == Config::kHelp) && (!is_source_specified))
        {
            std::string product_version;
            std::cout << kStrRgaProductName << " " << kStrRgaVersionPrefix << kStrRgaVersion << "." << kStrRgaBuildNum << std::endl;
            std::cout << kStrRgaProductName << " is a compiler and code analysis tool for OpenCL";
#if _WIN32
            std::cout << ", DirectX";
#endif
            std::cout << ", OpenGL and Vulkan" << std::endl << std::endl;
#ifdef RGA_ENABLE_VULKAN
            std::cout << "To view help for Vulkan mode: -h -s vulkan" << std::endl;
#endif
            std::cout << "To view help for offline Vulkan (GLSL) mode: -h -s vk-offline" << std::endl;
            std::cout << "To view help for offline Vulkan (SPIR-V binary) mode: -h -s vk-spv-offline" << std::endl;
            std::cout << "To view help for offline Vulkan (SPIR-V text) mode: -h -s vk-spv-txt-offline" << std::endl;
#if _WIN32
            std::cout << "To view help for DirectX 12 mode: -h -s dx12" << std::endl;
            std::cout << "To view help for DirectX 11 mode: -h -s dx11" << std::endl;
            std::cout << "To view help for DXR mode: -h -s dxr" << std::endl;
            std::cout << "To view help for AMDIL mode: -h -s amdil" << std::endl;
#endif
            std::cout << "To view help for OpenGL mode: -h -s opengl" << std::endl;
            std::cout << "To view help for OpenCL Offline mode: -h -s opencl" << std::endl;
            std::cout << "To view help for Binary Analysis mode: -h -s bin" << std::endl;
#ifdef _LEGACY_OPENCL_ENABLED
            std::cout << "To view help for legacy OpenCL mode: -h -s cl" << std::endl;
#endif // !_LEGACY_OPENCL_ENABLED
            std::cout << std::endl;
            std::cout << "To see the current RGA version: --version" << std::endl;
            std::cout << "To check for available updates: --updates" << std::endl;
        }
#ifdef _LEGACY_OPENCL_ENABLED
        else if ((config.requested_command == Config::kHelp) && (config.mode == kModeOpenclOffline))
        {
            // Put all options valid for this mode to one group to make the description aligned.
            std::cout << "*** Legacy OpenCL mode options ***" << std::endl;
            std::cout << "==================================" << std::endl;
            std::cout << opts.help({
                         generic_opt, 
                         il_dump_opt,
                         macro_and_include_opt,
                         cl_opt,
                         legacy_cl_opt }) << std::endl;
            std::cout << "Examples:" << std::endl;
            std::cout << "  " << "Compile foo.cl for all supported devices; extract ISA, IL code and statistics:" << std::endl;
            std::cout << "    " << program_name << " -s cl --isa output/foo_isa.txt --il output/foo_il.txt -a output/stats.csv foo.cl" << std::endl;
            std::cout << "  " << "Compile foo.cl for gfx1010; extract ISA and perform live register analysis:" << std::endl;
            std::cout << "    " << program_name << " -s cl -c gfx1010 --isa output/foo_isa.txt --livereg output/regs.txt foo.cl" << std::endl;
            std::cout << "  " << "Compile foo.cl for gfx906; extract binary and control flow graphs:" << std::endl;
            std::cout << "    " << program_name << " -s cl -c gfx906 --bin output/foo.bin --cfg output/cfg.dot foo.cl" << std::endl;
            std::cout << "  " << "List the kernels available in foo.cl:" << std::endl;
            std::cout << "    " << program_name << " -s cl --list-kernels foo.cl" << std::endl;
            std::cout << "  " << "Compile foo.cl for gfx1030; extract the hardware resource usage statistics for myKernel.  Write the statistics to foo.csv:" << std::endl;
            std::cout << "    " << program_name << " -s cl -c \"gfx1030\" --kernel myKernel -a foo.csv foo.cl" << std::endl;
            std::cout << "  " << "List the ASICs supported by Legacy OpenCL mode:" << std::endl;
            std::cout << "    " << program_name << " -s cl --list-asics" << std::endl;
            std::cout << std::endl;
        }
#endif  // !_LEGACY_OPENCL_ENABLED
        else if ((config.requested_command == Config::kHelp) && (config.mode == beKA::RgaMode::kModeDxr))
        {
            std::cout << "*** DXR mode options (Windows only) ***" << std::endl;
            std::cout << "=======================================" << std::endl;
            std::cout << opts.help({
                generic_opt, 
                macro_and_include_opt, 
                dxr_opt, 
                dxc_options, 
                dx12_other_options }) << std::endl;
            std::cout << "Examples:" << std::endl;
            std::cout << "  View supported targets for DXR:" << std::endl;
            std::cout << "    " << program_name << " -s dxr -l" << std::endl;
            std::cout << "  Compile and generate RDNA ISA disassembly for a shader named MyRaygenShader which is defined in C:\\shaders\\Raytracing.hlsl:" << std::endl;
            std::cout << "    " << program_name << " -s dxr --hlsl C:\\shaders\\Raytracing.hlsl --export MyRaygenShader --isa C:\\output\\isa.txt" << std::endl;
            std::cout << "  Compile and generate RDNA ISA disassembly and HW resource usage statistics for a shader named MyClosestHitShader which is defined in C:\\shaders\\Raytracing.hlsl:" << std::endl;
            std::cout << "    " << program_name << " -s dxr --hlsl C:\\shaders\\Raytracing.hlsl --export MyClosestHitShader --isa C:\\output\\isa.txt -a C:\\output\\stats.txt" << std::endl;
            std::cout << "  Compile and generate RDNA ISA disassembly for all DXR pipelines defined in C:\\shaders\\Raytracing.hlsl:" << std::endl;
            std::cout << "    " << program_name << " -s dxr --mode pipeline --hlsl C:\\shaders\\Raytracing.hlsl --isa C:\\output\\isa.txt" << std::endl;
            std::cout << "  Compile and generate RDNA ISA disassembly for all DXR pipelines which are defined in C:\\shaders\\rt.hlsl with additional headers that are located in C:\\shaders\\include:" << std::endl;
            std::cout << "    " << program_name << " -s dxr --mode pipeline --hlsl C:\\shaders\\rt.hlsl -I C:\\shaders\\include --isa C:\\output\\isa.txt" << std::endl;

        }
        else if ((config.requested_command == Config::kHelp) && (config.mode == beKA::RgaMode::kModeOpenclOffline))
        {
            // Put all options valid for this mode to one group to make the description aligned.
            std::cout << "*** OpenCL Offline mode options ***" << std::endl;
            std::cout << "================================" << std::endl;
            opts.custom_help("[options] source_file(s)");
            std::cout << opts.help({
                generic_opt, 
                il_dump_opt, 
                warnings_opt, 
                macro_and_include_opt,
                cl_opt,
                line_numbers_opt }) << std::endl;

            po::Options opt_level_opts("");
            opt_level_opts.positional_help("");
            opt_level_opts.custom_help("");
            opt_level_opts.add_options(opt_level_opt1)
                ("O0", "Disable optimizations")
                ("O1", "Enable minimal optimizations")
                ("O2", "Optimize for speed")
                ("O3", "Apply full optimization")
                ;
            std::cout << opt_level_opts.help({opt_level_opt1}) << std::endl;

            po::Options compiler_opts("");
            compiler_opts.positional_help("");
            compiler_opts.custom_help("");
            compiler_opts.add_options()
                ("compiler-bin", kStrCliDescAlternativeLightningCompilerBinFolder, po::value<std::string>())
                ("compiler-inc", kStrCliDescAlternativeLightningCompilerIncFolder, po::value<std::string>())
                ("compiler-lib", kStrCliDescAlternativeLightningCompilerLibFolder, po::value<std::string>())
                ;
            std::cout << compiler_opts.help() << std::endl;
            std::cout << "Note: In case that your alternative compiler supports targets which are not known to the RGA build that you are using, "
                    "use the additional-targets text file, which contains a list of targets that would be dynamically loaded by RGA. "
                    "To add targets, simply list them in a new line in the file, and RGA would load them while running." << std::endl << std::endl;

            std::cout << "Examples:" << std::endl;
            std::cout << "  " << "Compile test.cl for gfx1030 and extract the binary:" << std::endl;
            std::cout << "    " << program_name << " -s opencl -c \"gfx1030\" -b output/test.bin test.cl" << std::endl;
            std::cout << "  " << "Compile and link src1.cl, src2.cl and src3.cl into an HSA Code Object for Vega (gfx900), and extract ISA disassembly:" << std::endl;
            std::cout << "    " << program_name << " -s opencl -c gfx900 --isa output/isa.txt src1.cl src2.cl src3.cl" << std::endl;
            std::cout << "  " << "Compile test.cl for all supported targets, extract ISA and perform live register analysis:" << std::endl;
            std::cout << "    " << program_name << " -s opencl --isa test_isa.txt --livereg regs.txt test.cl" << std::endl;
            std::cout << "  " << "List the kernels available in test.cl:" << std::endl;
            std::cout << "    " << program_name << " -s opencl --list-kernels test.cl" << std::endl;
            std::cout << "  " << "List the ASICs supported by the OpenCL Lightning Compiler that is bundled with the tool:" << std::endl;
            std::cout << "    " << program_name << " -s opencl --list-asics" << std::endl;
            std::cout << "  " << "Compile test.cl and extract ISA disassembly for gfx900 using alternative OpenCL compiler:" << std::endl;
            std::cout << "    " << program_name << " -s opencl -c gfx900 --isa test.isa --compiler-bin C:\\llvm\\dist\\bin --compiler-inc C:\\llvm\\dist\\include --compiler-lib C:\\llvm\\dist\\lib\\bitcode  test.cl" << std::endl;

            std::cout << std::endl;
        }
        else if ((config.requested_command == Config::kHelp) && (config.mode == beKA::RgaMode::kModeDx11))
        {
            std::cout << "*** DirectX 11 mode options (Windows Only) ***" << std::endl;
            std::cout << "==============================================" << std::endl;
            opts.custom_help("[options] source_file");
            std::cout << opts.help({
                         generic_opt,
                         il_dump_opt,
                         macro_and_include_opt,
                         dx_opt }) << std::endl;
            std::cout << "Examples:" << std::endl;
            std::cout << "  View supported ASICS for DX11:" << std::endl;
            std::cout << "    " << program_name << " -s dx11 -l" << std::endl;
            std::cout << "  Compile myShader.hlsl for all supported targets and extract the ISA disassembly:" << std::endl;
            std::cout << "    " << program_name << " -s dx11 -f VsMain -p vs_5_0 --isa output/myShader_isa.txt src/myShader.hlsl" << std::endl;
            std::cout << "  Compile myShader.hlsl for gfx1030; extract the ISA and perform live register analysis:" << std::endl;
            std::cout << "    " << program_name << " -s dx11 -c gfx1030 -f VsMain -p vs_5_0 --isa output/myShader_isa.txt --livereg output/regs.txt myShader.hlsl" << std::endl;
            std::cout << "  Compile myShader.hlsl for gfx1034; perform static analysis and save the statistics to myShader.csv:" << std::endl;
            std::cout << "    " << program_name << " -s dx11 -c gfx1034 -f VsMain -p vs_5_0 -a output/myShader.csv shaders/myShader.hlsl" << std::endl;
        }
        else if ((config.requested_command == Config::kHelp) && (config.mode == beKA::RgaMode::kModeDx12))
        {
        std::cout << "*** DirectX 12 mode options (Windows only) ***" << std::endl;
        std::cout << "==============================================" << std::endl;
        std::cout << opts.help({
                     generic_opt,
                     il_dump_opt,
                     macro_and_include_opt,
                     dx12_opt,
                     dxc_options,
                     dx12_other_options }) << std::endl;
        std::cout << "Examples:" << std::endl;
        std::cout << "  View supported ASICS for DX12:" << std::endl;
        std::cout << "    " << program_name << " -s dx12 -l" << std::endl;
        std::cout << "  Compile a cs_5_1 compute shader named \"CSMain\" for gfx1010, where the root signature is referenced through a [RootSignature()] attribute. Generate ISA disassembly and resource usage statistics:" << std::endl;
        std::cout << "    " << program_name << " -s dx12 -c gfx1010 --cs C:\\shaders\\FillLightGridCS_8.hlsl --cs-entry CSMain --cs-model cs_5_1 --isa C:\\output\\isa.txt -a C:\\output\\stats.txt" << std::endl;
        std::cout << "  Compile a graphics pipeline with model 6.0 vertex (\"VSMain\") and pixel (\"PSMain\") shaders defined in vert.hlsl and pixel.hlsl respectively, while the root signature is in a pre-compiled binary file. Generate ISA disassembly and resource usage statistics:" << std::endl;
        std::cout << "    " << program_name << " -s dx12 --vs C:\\shaders\\vert.hlsl --vs-model vs_6_0 --vs-entry VSMain --ps C:\\shaders\\pixel.hlsl --ps-model ps_6_0 --ps-entry PSMain --gpso C:\\shaders\\state.gpso --rs-bin C:\\rootSignatures\\rs.bin --isa C:\\output\\disassembly.txt -a C:\\output\\stats.txt " << std::endl;
        std::cout << "  Compile a graphics pipeline with vertex (\"VSMain\") and pixel (\"PSMain\") shaders defined both in shader.hlsl, while the root signature is in a pre-compiled binary file. Generate ISA disassembly and resource usage statistics:" << std::endl;
        std::cout << "    " << program_name << " -s dx12 --all-hlsl C:\\shaders\\shaders.hlsl --all-model 6_0 --vs-entry VSMain --ps-entry PSMain --gpso C:\\shaders\\state.gpso --rs-bin C:\\rootSignatures\\rs.bin --isa C:\\output\\disassembly.txt -a C:\\output\\stats.txt" << std::endl;
        std::cout << "  Compile a cs_5_1 compute shader named \"CSMain\" for Radeon VII (gfx906), where the root signature is in a binary file (C:\\RS\\FillLightRS.rs.fxo). Generate ISA disassembly and resource usage statistics:" << std::endl;
        std::cout << "    " << program_name << " -s dx12 -c gfx906 --cs C:\\shaders\\FillLightGridCS_8.hlsl --cs-model cs_5_1 --cs-entry main --rs-bin C:\\RS\\FillLightRS.rs.fxo --isa C:\\output\\lightcs_dis.txt -a C:\\output\\stats.txt" << std::endl;
        std::cout << "  Compile a DXIL or DXBC blob for Navi10 (gfx1010) and generate ISA disassembly:" << std::endl;
        std::cout << "    " << program_name << " -s dx12 -c gfx1010 --cs-blob C:\\shaders\\FillLightGridCS_8.obj --isa C:\\output\\lightcs_dis.txt" << std::endl;
        }
        else if ((config.requested_command == Config::kHelp) && (config.mode == beKA::RgaMode::kModeAmdil))
        {
            std::cout << "*** AMDIL mode options (Windows only) ***" << std::endl;
            std::cout << "=========================================" << std::endl;
            opts.custom_help("[options] source_file");
            std::cout << opts.help({ generic_opt }) << std::endl;
            std::cout << "  Generate ISA from AMDIL code for all supported targets:" << std::endl;
            std::cout << "    " << program_name << " -s amdil --isa output/isaFromAmdil.isa myAmdilCode.amdil" << std::endl;
            std::cout << "  Generate ISA for gfx1030 from AMDIL code and extract statistics:" << std::endl;
            std::cout << "    " << program_name << " -s amdil -c gfx1030 --isa output/isaFromAmdil.isa -a output/statsFromAmdil.csv myAmdilCode.amdil" << std::endl;
            std::cout << "  Generate ISA for gfx900 from AMDIL code and perform live register analysis:" << std::endl;
            std::cout << "    " << program_name << " -s amdil -c gfx900 --isa output/myShader.isa --livereg output/regs.txt myAmdilCode.amdil" << std::endl;
        }
        else if ((config.requested_command == Config::kHelp) &&
                 (config.mode == beKA::RgaMode::kModeVkOffline || config.mode == beKA::RgaMode::kModeVkOfflineSpv ||
                  config.mode == beKA::RgaMode::kModeVkOfflineSpvTxt))
        {
            // Get the mode name, and the relevant file extensions. We will use it when constructing the example string.
            const char* kFileExtSpv = "spv";
            const char* kFileExtSpvTxt = "txt";
            std::string rga_mode_name;
            std::string vert_ext;
            std::string frag_ext;
            switch (config.mode)
            {
            case beKA::RgaMode::kModeVkOffline:
                rga_mode_name = Config::source_kind_glsl_vulkan_offline;
                vert_ext = "vert";
                frag_ext = "frag";
                break;
            case beKA::RgaMode::kModeVkOfflineSpv:
                rga_mode_name = Config::source_kind_spirv_bin_offline;
                vert_ext = kFileExtSpv;
                frag_ext = kFileExtSpv;
                break;
            case beKA::RgaMode::kModeVkOfflineSpvTxt:
                rga_mode_name = Config::source_kind_spirv_txt_offline;
                vert_ext = kFileExtSpvTxt;
                frag_ext = kFileExtSpvTxt;
                break;
            }

            // Convert the mode name string to lower case for presentation.
            std::transform(rga_mode_name.begin(), rga_mode_name.end(), rga_mode_name.begin(), ::tolower);

            std::cout << "*** Vulkan Offline mode options ***" << std::endl;
            std::cout << "===================================" << std::endl << std::endl;
            std::cout << "The Vulkan offline mode is independent of the installed driver and may not provide assembly code and resource"
                         " usage that reflect the real-life case. To get results that reflect the real-life performance of your code, "
                         "please use RGA's Vulkan live-driver mode (-s vulkan)." << std::endl;
            if (config.mode == beKA::RgaMode::kModeVkOfflineSpv || config.mode == beKA::RgaMode::kModeVkOfflineSpvTxt)
            {
                opts.custom_help("[options] [optional: spv_input_file]");
            }
            if (config.mode == beKA::RgaMode::kModeVkOfflineSpv || config.mode == beKA::RgaMode::kModeVkOfflineSpvTxt)
            {
                opts.positional_help("\n\nNotes:\n * The input file(s) must be specified in one of two ways:\n"
                    "   1) A single SPIR-V input file provided as \"spv_input_file\", or\n"
                    "   2) One or more pipeline stage specific shader files specified by the pipeline stage options (--vert, --tesc, etc.).");
            }

            std::cout << opts.help({
                         generic_opt,
                         opt_level_opt1,
                         pipelined_opt_offline }) << std::endl;

            std::cout << "Examples:" << std::endl;
            std::cout << "  Compile vertex & fragment shaders for all supported devicesl; extract ISA and statistics:" << std::endl;
            std::cout << "    " << program_name << " -s " << rga_mode_name << " --isa output/isa.txt -a output/stats.csv --vert source/myVertexShader." << vert_ext << " --frag source/myFragmentShader." << frag_ext << std::endl;
            std::cout << "  Compile vertex & fragment shaders for gfx1030; extract ISA and statistics:" << std::endl;
            std::cout << "    " << program_name << " -s " << rga_mode_name << " -c gfx1030 --isa output/isa.txt -a output/.csv --vert source/myVertexShader." << vert_ext << " --frag source/myFragmentShader." << frag_ext << std::endl;
            std::cout << "  Compile vertex shader for Radeon gfx1034; extract ISA and binaries:" << std::endl;
            std::cout << "    " << program_name << " -s " << rga_mode_name << " -c \"gfx1034\" --isa output/isa.txt -b output/binary.bin -a output/stats.csv --vert c:\\source\\myVertexShader." << vert_ext << std::endl;
            if (config.mode == beKA::RgaMode::kModeVkOfflineSpv || config.mode == beKA::RgaMode::kModeVkOfflineSpvTxt)
            {
                std::cout << "  Extract ISA for a single SPIR-V file for gfx1034, without specifying the pipeline stages:" << std::endl;
                std::cout << "    " << program_name << " -s " << rga_mode_name << " -c gfx1034 --isa output/program_isa.txt source/program.spv" << std::endl;
            }
        }
        else if ((config.requested_command == Config::kHelp) && (config.mode == beKA::RgaMode::kModeOpengl))
        {
            std::cout << "*** OpenGL mode options ***" << std::endl;
            std::cout << "===========================" << std::endl;
            std::cout << opts.help({
                         generic_opt,
                         il_dump_opt,
                         pipelined_opt_offline }) << std::endl;
            std::cout << "Examples:" << std::endl;
            std::cout << "  Compile fragment shader for gfx1034; extract ISA, binaries and statistics:" << std::endl;
            std::cout << "    " << program_name << " -s opengl -c gfx1034 --isa output/opengl_isa.txt -b output/opengl_bin.bin -a output/opengl_stats.csv --frag source/myFragmentShader.frag" << std::endl;
            std::cout << "  Compile vertex & fragment shaders for gfx1030; Extract ISA and control flow graph: " << std::endl;
            std::cout << "    " << program_name << " -s opengl -c gfx1030 --isa output/opengl_isa.txt --cfg output/cfg.dot --vert myVertexShader.vert --frag cmyFragmentShader.frag" << std::endl;
            std::cout << "  Compile geometry shader for all supported devices; extract ISA and perform live register analysis:" << std::endl;
            std::cout << "    " << program_name << " -s opengl --isa output/opengl_isa.txt --livereg output/regs.txt --geom source/myVertexShader.geom" << std::endl;
        }
#ifdef RGA_ENABLE_VULKAN
        else if ((config.requested_command == Config::kHelp) && (config.mode == beKA::RgaMode::kModeVulkan))
        {
            std::cout << "*** Vulkan mode options ***" << std::endl;
            std::cout << "===========================" << std::endl;

            std::cout << opts.help({
                         generic_opt,
                         macro_and_include_opt,
                         pipelined_opt_offline }) << std::endl;

            std::cout << " By default, input files would be interpreted as SPIR-V binary files, unless the file extension is any of the following:" << std::endl <<
                "   .vert  --> GLSL source file." << std::endl <<
                "   .frag  --> GLSL source file." << std::endl <<
                "   .tesc  --> GLSL source file." << std::endl <<
                "   .tese  --> GLSL source file." << std::endl <<
                "   .geom  --> GLSL source file." << std::endl <<
                "   .comp  --> GLSL source file." << std::endl <<
                "   .glsl  --> GLSL source file." << std::endl <<
                "   .vs    --> GLSL source file." << std::endl <<
                "   .fs    --> GLSL source file." << std::endl << std::endl;

            std::cout << opts.help({ vulkan_opt }) << std::endl;

            std::cout << compiler_paths_opt << std::endl;

            po::Options compiler_opts(program_name);
            compiler_opts.positional_help("");
            compiler_opts.custom_help("[options]");
            compiler_opts.add_options("")
                ("compiler-bin", kStrCliDescAlternativeVkBinFolder, po::value<std::string>())
                ;
            std::cout << compiler_opts.help({ "" }) << std::endl;
        }
#endif
        else if (config.requested_command == Config::kHelp && config.mode == beKA::RgaMode::kModeBinary)
        {
            std::cout << "*** Binary Analysis mode options ***" << std::endl;
            std::cout << "===========================" << std::endl;
            // Duplicate generic options only for help doc., as binary analysis mode does not support the asic,c and binary,b cli option.
            po::Options binary_opts(program_name);
            binary_opts.positional_help("");
            binary_opts.custom_help("[options]");
            binary_opts.add_options(generic_opt)
                (kStrOptionListAsic, kStrDescriptionListAsic)
                (kStrOptionVersion, kStrDescriptionVersion)
                (kStrOptionHelp, kStrDescriptionHelp)
                (kStrOptionAnalysis, kStrDescriptionAnalysis, po::value<std::string>())
                (kStrOptionIsa, kStrDescriptionIsa, po::value<std::string>())
                (kStrOptionLivereg, kStrDescriptionLivereg, po::value<std::string>())
                (kStrOptionSgpr, kStrDescriptionSgpr, po::value<std::string>())
                (kStrOptionCfg, kStrDescriptionCfg, po::value<std::string>())
                (kStrOptionCfgI, kStrDescriptionCfgI, po::value<std::string>())
                (kStrOptionSourceKind, kStrDescriptionSourceKind, po::value<std::string>())
                (kStrOptionUpdates, kStrDescriptionUpdates)
                (kStrOptionVerbose, kStrDescriptionVerbose)
                ;
            binary_opts.add_options(binary_opt)
                (kStrOptionCO, kStrDescriptionCO, po::value<std::string>())
                ;
            std::cout << binary_opts.help({
                         generic_opt,
                         binary_opt }) << std::endl;
            std::cout << "Examples:" << std::endl;
            std::cout << "  Extract ISA, and statistics for:" << std::endl;
            std::cout << "    " << program_name
                 << " -s bin --isa output/binary_isa.txt -a output/binary_stats.csv --co source/myBinaryCodeObject.bin"
                 << std::endl;
            std::cout << " Extract ISA and control flow graph for: " << std::endl;
            std::cout << "    " << program_name
                 << " -s bin --isa output/binary_isa.txt --cfg output/cfg.dot --co source/myBinaryCodeObject.bin" << std::endl;
            std::cout << "  Extract ISA and perform live register analysis:" << std::endl;
            std::cout << "    " << program_name << " -s bin --isa output/binary_isa.txt --livereg output/regs.txt --co source/myBinaryCodeObject.bin" << std::endl;

        }
        else if ((config.requested_command == Config::kUpdate))
        {
            KcUtils::CheckForUpdates();
        }
    }
    catch (std::exception& e)
    {
        std::string exception_msg = e.what();
        if (exception_msg == "multiple occurrences")
        {
            exception_msg = "Error: Problem parsing arguments. Please check if a non-repeatable argument was used more then once and that all file paths are correct. Note that if a path contain a space, a \"\" should be used.";
        }
        std::cout << exception_msg << "\n";
        do_work = false;
    }

    if (!do_work)
    {
        config.requested_command = Config::kInvalid;
    }

    return do_work;
}

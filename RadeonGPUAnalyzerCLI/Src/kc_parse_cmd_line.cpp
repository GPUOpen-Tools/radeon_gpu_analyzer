//=================================================================
// Copyright 2020 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

// C++.
#include <iostream>
#include <string>
#include <algorithm>
#include <cctype>

#ifdef _WIN32
#pragma warning (push, 3) // disable warning level 4 for boost
#endif
#include <boost/filesystem/path.hpp>
#include <boost/program_options.hpp>

#ifdef _WIN32
#pragma warning (pop)
#endif

#ifdef CMAKE_BUILD
#include "RadeonGPUAnalyzerCLIConfig.h"
#endif

// Local.
#include "RadeonGPUAnalyzerCLI/Src/kc_config.h"
#include "RadeonGPUAnalyzerCLI/Src/kc_parse_cmd_line.h"
#include "RadeonGPUAnalyzerCLI/Src/kc_cli_string_constants.h"
#include "RadeonGPUAnalyzerCLI/Src/kc_utils.h"

// Shared between CLI and GUI.
#include "Utils/Include/rgaVersionInfo.h"
#include "Utils/Include/rgaCliDefs.h"

namespace po = boost::program_options;
using namespace std;

#if GSA_BUILD
#define KERNEL_OPTION "function"
#else
#define KERNEL_OPTION "kernel"
#endif

// Constants.
static char* kStrErrorBothCfgAndCfgiSpecified = "Error: only one of \"--cfg\" and \"--cfg-i\" options can be specified.";
static const char* kStrErrorNoModeSpecified = "No mode specified. Please specify mode using - s <arg>.";
static const char* kStrDxAdaptersHelpCommonText = "This is only relevant if you have multiple display adapters installed on your system, and you would like RGA to use the driver which is associated with "
"a non-primary display adapter.By default RGA will use the driver that is associated with the primary display adapter.";
static const char* kStrInfoLegacyOpenclNoLongerSupported = "Legacy OpenCL mode (-s cl) is no longer supported.";

bool ParseCmdLine(int argc, char* argv[], Config& config)
{
    //----- Command Line Options Parsing
    bool do_work = true;
    bool validation = false;

    try
    {
        // Parse command line options using Boost library

        // Generic Options: Valid got CL, DX, GL
        po::options_description generic_opt("Generic options");
        generic_opt.add_options()
            // The next two options here can be repeated, so they are vectors.
            ("list-asics,l", "List the known GPU codenames, architecture names and variant names.\nTo target a specific GPU, use its codename as the argument to the \"-c\" command line switch.")
            ("asic,c", po::value<vector<string>>(&config.asics), "Which ASIC to target.  Repeatable.")
            ("version", "Print version string.")
            ("help,h", "Produce this help message.")
            ("analysis,a", po::value<string>(&config.analysis_file), "Path to output analysis file.")
            ("binary,b", po::value<string>(&config.binary_output_file), "Path to ELF binary output file.")
            ("isa", po::value<string>(&config.isa_file), "Path to output ISA disassembly file(s).")
            ("livereg", po::value<string>(&config.livereg_analysis_file), "Path to live register analysis output file(s).")
            ("cfg", po::value<string>(&config.block_cfg_file), "Path to per-block control flow graph output file(s).")
            ("cfg-i", po::value<string>(&config.inst_cfg_file), "Path to per-instruction control flow graph output file(s).")
            ("source-kind,s", po::value<string>(&config.source_kind), "Source platform: dx12 for DirectX 12, dxr for DXR, dx11 for DirectX 11, vulkan for Vulkan, opengl for OpenGL, "
#ifdef _LEGACY_OPENCL_ENABLED
                "cl for OpenCL legacy,"
#endif // !_LEGACY_OPENCL_ENABLED
                "rocm-cl for OpenCL Lightning Compiler and amdil for AMDIL.")
            ("updates,u", "Check for available updates.")
            ("verbose,v", "Print command line strings that RGA uses to launch external processes.")
            ;

        // DX Options
        std::string adapters_desc = "List all of the supported display adapters that are installed on the system.\n" + std::string(kStrDxAdaptersHelpCommonText);
        std::string set_adapter_desc = "Specify the id of the display adapter whose driver you would like RGA to use.\n" + std::string(kStrDxAdaptersHelpCommonText);

        po::options_description dx_opt("DirectX 11 mode options");
        dx_opt.add_options()
            ("function,f", po::value<string>(&config.function), "DX shader entry point.")
            ("profile,p", po::value<string>(&config.profile), "Profile to use for compilation.  This option is only required in case that the input is in HLSL.\nFor example: vs_5_0, ps_5_0, etc.")
            ("DXFlags", po::value<unsigned int>(&config.dx_flags), "Flags to pass to D3DCompile.")
            ("DXLocation", po::value<string>(&config.dx_compiler_location), "Location to the D3DCompiler Dll required for compilation. If none is specified, the default D3D compiler that is bundled with the Analyzer will be used.")
            ("FXC", po::value<string>(&config.fxc), "FXC Command Line. Use full path and specify all arguments in \"\". For example:\n"
                                                      "   rga.exe  -f VsMain1 -s DXAsm -p vs_5_0 <Path>\\vsBlob.obj  --isa <Path>\\vsTest.isa --FXC \"<Path>\\fxc.exe /E VsMain1 /T vs_5_0 /Fo <Path>\\vsBlob.obj <Path>\\vsTest.fx\"\n "
                                                      "   In order to use it, DXAsm must be specified. /Fo switch must be used and output file must be the same as the input file for rga.")
            ("DumpMSIntermediate", po::value<string>(&config.dump_ms_intermediate), "Location to save the MS Blob as text. ")
            ("intrinsics", "Enable AMD D3D11 Shader Intrinsics extension.")
            ("adapters", adapters_desc.c_str())
            ("set-adapter", po::value<int>(&config.dx_adapter), set_adapter_desc.c_str())
            ("UAVSlot", po::value<int>(&config.uav_slot), "This value should be in the range of [0,63].\nThe driver will use the slot to track which UAV is being used to specify the intrinsic. The UAV slot that is selected cannot be used for any other purposes.\nThis option is only relevant when AMD D3D11 Shader Intrinsics is enabled (specify --intrinsics).")
            ("dxbc", "Treat input file as a DXBC binary.")
            ;

        po::options_description macro_and_include_opt("Macro and Include paths Options");
        macro_and_include_opt.add_options()
            // The next two options here can be repeated, so they are vectors.
            ("define,D", po::value< vector<string> >(&config.defines), "Define symbol or symbol=value. Repeatable.")
            ("IncludePath,I", po::value< vector<string> >(&config.include_path), "Additional include path required for compilation.  Repeatable.")
            ;

        // CL Option
        po::options_description cl_opt("");
        cl_opt.add_options()
            ("list-kernels", "List kernel functions.")
            ("metadata,m", po::value<string>(&config.metadata_file), "Path to output Metadata file(s).\n"
                "Requires --" KERNEL_OPTION ".")
            ("kernel,k", po::value<string>(&config.function), "Kernel to be compiled and analyzed. If not specified, all kernels will be targeted. Note "
                "that this option is only relevant when the compiler can generate per-kernel data. When the runtime uses the updated compiler to generate "
                "a Code Object type binary, the ISA would be generated for the entire program, since the binary has no per-kernel code sections.\n")
            ("OpenCLoption", po::value< vector<string> >(&config.opencl_options), "OpenCL compiler options.  Repeatable.");

        // Vulkan shader type.
        po::options_description pipelined_opt_offline("Input shader type");
        pipelined_opt_offline.add_options()
            ("vert", po::value<string>(&config.vertex_shader), "Full path to vertex shader input file.")
            ("tesc", po::value<string>(&config.tess_control_shader), "Full path to tessellation control shader input file.")
            ("tese", po::value<string>(&config.tess_evaluation_shader), "Full path to tessellation evaluation shader input file.")
            ("geom", po::value<string>(&config.geometry_shader), "Full path to geometry shader input file.")
            ("frag", po::value<string>(&config.fragment_shader), "Full path to fragment shader input file")
            ("comp", po::value<string>(&config.compute_shader), "Full path to compute shader input file.");

        // OpenGL is Vulkan minus .pipe option.
        po::options_description pipelined_opt_live = pipelined_opt_offline;

        // Add .pipe option for Vulkan offline.
        pipelined_opt_offline.add_options()("pipe", po::value<string>(&config.pipe_file), "Full path to .pipe input file.");

#ifdef RGA_ENABLE_VULKAN
        // Prepare the --glslang-opt description.
        std::string glslang_opt_description = CLI_OPT_GLSLANG_OPT_DESCRIPTION_A;
        glslang_opt_description.append(CLI_OPT_GLSLANG_OPT_DESCRIPTION_B);

        // Live Vulkan mode options.
        po::options_description vulkan_opt("Vulkan mode options");
        vulkan_opt.add_options()
            ("[stage]-glsl arg", "Full path to [stage] input file, while forcing interpretation of input file as glsl source file rather than SPIR-V binary. arg is the full path to [stage] shader input file. [stage] can be one "
                "of: \"vert\", \"tesc\", \"tese\", \"geom\", \"frag\", \"comp\". "
                "For example, use \"--vert-glsl shader.glsl\" to specify path to the vertex shader file and indicate that it is a glsl source file (rather than a SPIR-V binary).")
            ("[stage]-spvas arg", "Full path to [stage] input file, while forcing interpretation of input file as SPIR-V textual source file rather than SPIR-V binary. arg is the full path to [stage] shader input file. [stage] can be one "
                "of: \"vert\", \"tesc\", \"tese\", \"geom\", \"frag\", \"comp\". "
                "For example, use \"--vert-spvas shader.txt\" to specify path to the vertex shader file and indicate that it is a SPIR-V textual source file (rather than a SPIR-V binary).")
            ("pso", po::value<string>(&config.pso), "Full path to .gpso (graphics) or .cpso (compute) pipeline state JSON file. If no pipeline state file is provided, a default pipeline state would be used. In that case, if your shaders do not match the default state, the compilation would fail.")
            ("icd", po::value<string>(&config.icd_file), CLI_OPT_ICD_DESCRIPTION)
            ("loader-debug", po::value<string>(&config.loader_debug), CLI_OPT_VK_LOADER_DEBUG_DESCRIPTION)
            ("glslang-opt", po::value<string>(&config.glslang_opt), glslang_opt_description.c_str())
            ("disassemble-spv", po::value<string>(&config.spv_txt), "Disassemble SPIR-V binary file. Accepts an optional argument with the full path to the output file where the disassembly would be saved. If not specified, the disassembly would be printed to stdout.")
            ("validation", po::bool_switch(&validation), "Enable Vulkan validation layers and dump the output of validation layers to stdout.")
            ("validation-file", po::value<string>(&config.vulkan_validation), "Enable Vulkan validation layers and dump the output of validation layers to the text file specified by the option argument.")
            ("parse-spv", po::value<string>(&config.parsed_spv), "Parse SPIR-V binary file. The option argument is the full path to the output text file with SPIR-V binary info. "
            "If path is not specified, the SPIR-V info will be printed to stdout.")
#ifdef VK_HLSL
                ("list-entries", "List hlsl function names.");
#else
            ;
#endif
        // Per-stage input file types ("--[stage]-[file-type]").
        po::options_description vulkan_input_type_opt;
        for (const std::string& stage : { "vert", "tesc", "tese", "frag", "geom", "comp" })
        {
            for (const std::string& file_type : { "glsl", "spvas" })
            {
                vulkan_input_type_opt.add_options()
                    ((stage + "-" + file_type).c_str(), po::value<string>(), "");
            }
        }

        // Vulkan hidden options (are not presented to the user in -h).
        po::options_description vulkan_opt_hidden("Vulkan-specific hidden options");
        vulkan_opt_hidden.add_options()
            ("assemble-spv", po::value<string>(&config.spv_bin), "Assemble SPIR-V textual code to a SPIR-V binary file. arg is the full path to the output SPIR-V binary file.")
            ;
#endif

        // DX12-specific.
        // DXC.
        po::options_description dxc_options("DXC options");
        dxc_options.add_options()
            ("dxc", po::value<string>(&config.dxc_path), "Path to an alternative DXC package (a folder containing DXC.exe, dxcompiler.dll and dxil.dll). If specified, RGA would use DXC from the given path rather than the package that it ships with.")
            ("dxc-opt", po::value<string>(&config.dxc_opt), "Additional options to be passed to DXC when performing front-end compilation.")
            ("dxc-opt-file", po::value<string>(&config.dxc_opt_file), "Full path to text file from which to read the additional options to be passed to DXC when performing front-end compilation. Since specifying verbose options which include macro "
                "definitions can be tedious in the command line, and at times even impossible, you can use this option to specify the DXC command line arguments in a text file which would be read by RGA and passed as-is to DXC in the front-end compilation step.");
        ;

        // D3D12 debug layer.
        po::options_description dx12_other_options("Other DX12 options");
        dx12_other_options.add_options() ("debug-layer", "Enable the D3D12 debug layer.");

        // DX12 graphics/compute.
        po::options_description dx12_opt("DirectX 12 mode options");
        dx12_opt.add_options()
            // HLSL.
            ("vs", po::value<std::string>(&config.vs_hlsl), "Full path to hlsl file where vertex shader is defined.")
            ("hs", po::value<std::string>(&config.hs_hlsl), "Full path to hlsl file where hull shader is defined.")
            ("ds", po::value<std::string>(&config.ds_hlsl), "Full path to hlsl file where domain shader is defined.")
            ("gs", po::value<std::string>(&config.gs_hlsl), "Full path to hlsl file where geometry shader is defined.")
            ("ps", po::value<std::string>(&config.ps_hlsl), "Full path to hlsl file where pixel shader is defined.")
            ("cs", po::value<std::string>(&config.cs_hlsl), "Full path to hlsl file where compute shader is defined.")
            ("all-hlsl", po::value<std::string>(&config.all_hlsl), "Full path to the hlsl file to be used for all stages. "
                "You can use this option if all of your shaders are defined in the same hlsl file, to avoid repeating the --<stage> "
                "argument. If you use this option in addition to --<stage> or --<stage>-blob, then the --<stage> or --<stage>-blob option "
                "would override this option for stage <stage>.")
            // DXBC.
            ("vs-blob", po::value<std::string>(&config.vs_dxbc), "Full path to compiled DXBC or DXIL binary where vertex shader is found.")
            ("hs-blob", po::value<std::string>(&config.hs_dxbc), "Full path to compiled DXBC or DXIL binary where hull shader is found.")
            ("ds-blob", po::value<std::string>(&config.ds_dxbc), "Full path to compiled DXBC or DXIL binary domain shader is found.")
            ("gs-blob", po::value<std::string>(&config.gs_dxbc), "Full path to compiled DXBC or DXIL binary geometry shader is found.")
            ("ps-blob", po::value<std::string>(&config.ps_dxbc), "Full path to compiled DXBC or DXIL binary pixel shader is found.")
            ("cs-blob", po::value<std::string>(&config.cs_dxbc), "Full path to compiled DXBC or DXIL binary where compute shader is found.")
            // IL Disassembly.
            ("vs-dxil-dis", po::value<std::string>(&config.vs_dxil_disassembly), "Full path to the DXIL or DXBC disassembly output file for vertex shader. "
                "Note that this option is only valid for textual input (HLSL).")
            ("hs-dxil-dis", po::value<std::string>(&config.hs_dxil_disassembly), "Full path to the DXIL or DXBC disassembly output file for hull shader. "
                "Note that this option is only valid for textual input (HLSL).")
            ("ds-dxil-dis", po::value<std::string>(&config.ds_dxil_disassembly), "Full path to the DXIL or DXBC disassembly output file for domain shader. "
                "Note that this option is only valid for textual input (HLSL).")
            ("gs-dxil-dis", po::value<std::string>(&config.gs_dxil_disassembly), "Full path to the DXIL or DXBC disassembly output file for geometry shader. "
                "Note that this option is only valid for textual input (HLSL).")
            ("ps-dxil-dis", po::value<std::string>(&config.ps_dxil_disassembly), "Full path to the DXIL or DXBC disassembly output file for pixel shader. "
                "Note that this option is only valid for textual input (HLSL).")
            ("cs-dxil-dis", po::value<std::string>(&config.cs_dxil_disassembly), "Full path to the DXIL or DXBC disassembly output file for compute shader. "
                "Note that this option is only valid for textual input (HLSL).")
            // Target.
            ("vs-entry", po::value<std::string>(&config.vs_entry_point), "Entry-point name of vertex shader.")
            ("hs-entry", po::value<std::string>(&config.hs_entry_point), "Entry-point name of hull shader.")
            ("ds-entry", po::value<std::string>(&config.ds_entry_point), "Entry-point name of domain shader.")
            ("gs-entry", po::value<std::string>(&config.gs_entry_point), "Entry-point name of geometry shader.")
            ("ps-entry", po::value<std::string>(&config.ps_entry_point), "Entry-point name of pixel shader.")
            ("cs-entry", po::value<std::string>(&config.cs_entry_point), "Entry-point name of compute shader.")
            // Shader model.
            ("vs-model", po::value<std::string>(&config.vs_model), "Shader model of vertex shader (e.g. vs_5_1 or vs_6_0).")
            ("hs-model", po::value<std::string>(&config.hs_model), "Shader model of hull shader (e.g. hs_5_1 or hs_6_0).")
            ("ds-model", po::value<std::string>(&config.ds_model), "Shader model of domain shader (e.g. ds_5_1 or ds_6_0).")
            ("gs-model", po::value<std::string>(&config.gs_model), "Shader model of geometry shader (e.g. gs_5_1 or gs_6_0).")
            ("ps-model", po::value<std::string>(&config.ps_model), "Shader model of pixel shader (e.g. ps_5_1 or ps_6_0).")
            ("cs-model", po::value<std::string>(&config.cs_model), "Shader model of compute shader (e.g. cs_5_1 or cs_6_0).")
            ("all-model", po::value<std::string>(&config.all_model), "Shader model to be used for all stages (e.g. 5_1 or 6_0). Instead of "
                "specifying the model for each stage specifically, you can use this option to pass the shader model version and RGA "
                "would auto-generate the relevant model for every stage in the pipeline. Note that if you use this option together "
                "with any of the <stage>-model options, for any stage <stage> the <stage>-model option would override --all-model.")
            // Root signature.
            ("rs-bin", po::value<std::string>(&config.rs_bin), "Full path to the serialized root signature "
                "to be used in the compilation process.")
            ("rs-hlsl", po::value<std::string>(&config.rs_hlsl), "Full path to the HLSL file where the "
                "RootSignature macro is defined. If there is only a "
                "single hlsl input file, this option is not required.")
            ("rs-macro", po::value<std::string>(&config.rs_macro), "The name of the RootSignature macro in the HLSL "
                "code. If specified, the root signature would be compiled from the HLSL source. Use this if your "
                "shader does not include the [RootSignature()] attribute but has a root signature macro defined"
                "in the source code. Please also check the description for --rs-hlsl, which is related to this option.")
            ("rs-macro-version", po::value<std::string>(&config.rs_macro_version), "The version of the RootSignature macro "
                "specified through the rs - macro option.By default, 'rootsig_1_1' would be assumed.")
            // Pipeline state.
            ("gpso", po::value<std::string>(&config.pso_dx12), "Full path to .gpso file that describes the graphics pipeline state (required for graphics pipelines only)."
                " You can generate a template by using the --gpso-template option.")
            ("gpso-template", po::value<std::string>(&config.pso_dx12_template), "Full path to where to save the template pipeline state description file. You can then edit the file to match your pipeline.")
            ("elf-dis", po::value<std::string>(&config.elf_dis), "Full path to output text file where disassembly of the pipeline ELF binary would be saved.")
            ;

        // DXR-specific.
        po::options_description dxr_opt("DXR mode options");
        dxr_opt.add_options()
            ("hlsl", po::value<std::string>(&config.dxr_hlsl), "Full path to DXR HLSL input file that contains the state definition.")
            ("mode", po::value<std::string>(&config.dxr_mode), "DXR mode: 'shader' to compile a specific shader, or 'pipeline' to compile all pipelines in the State Object. By default, shader mode is assumed.")
            ("export", po::value<std::vector<std::string>>(&config.dxr_exports), "The export name of the shader for which to retrieve results - only relevant to shader mode (--mode shader)."
                " this can be an export name of any shader in the state object.")
            ("dxr-model", po::value<std::string>(&config.dxr_shader_model), "Shader model used for DXR HLSL compilation. Use this option to override "
                "the shader model that is used for HLSL compilation by default (lib_6_3).")
            ;

#ifdef _LEGACY_OPENCL_ENABLED
        // Legacy OpenCL options.
        po::options_description legacy_cl_opt("");
        legacy_cl_opt.add_options()
            ("suppress", po::value<vector<std::string> >(&config.suppress_section), "Section to omit from binary output.  Repeatable. Available options: .source, .amdil, .debugil,"
                ".debug_info, .debug_abbrev, .debug_line, .debug_pubnames, .debug_pubtypes, .debug_loc, .debug_str,"
                ".llvmir, .text\nNote: Debug sections are valid only with \"-g\" compile option");
#endif // _LEGACY_OPENCL_ENABLED

        // IL dump.
        po::options_description il_dump_opt("");
        il_dump_opt.add_options()
            ("il", po::value<std::string>(&config.il_file), "Path to output IL (intermediate language) disassembly file(s).");

        // Line numbers.
        po::options_description line_numbers_opt("");
        line_numbers_opt.add_options()
            ("line-numbers", "Add source line numbers to ISA disassembly.");

        // Compiler warnings.
        po::options_description warnings_opt("");
        warnings_opt.add_options()
            ("warnings,w", "Print warnings reported by the compiler.");

        // Optimization Levels.
        po::options_description opt_level_opt1("Optimization Levels");
        po::options_description opt_level_opt2("Optimization Levels");
        opt_level_opt1.add_options()
            ("O0", "Disable optimizations")
            ("O1", "Enable minimal optimizations");
        opt_level_opt2.add_options()
            ("O0", "Disable optimizations")
            ("O1", "Enable minimal optimizations")
            ("O2", "Optimize for speed")
            ("O3", "Apply full optimization");

        // Paths to user-provided ROCm compiler.
        po::options_description rocm_compiler_paths_opt("Alternative ROCm OpenCL compiler");
        rocm_compiler_paths_opt.add_options()
            ("compiler-bin", po::value<string>(&config.compiler_bin_path), CLI_DESC_ALTERNATIVE_ROCM_BIN_FOLDER)
            ("compiler-inc", po::value<string>(&config.compiler_inc_path), CLI_DESC_ALTERNATIVE_ROCM_INC_FOLDER)
            ("compiler-lib", po::value<string>(&config.compiler_lib_path), CLI_DESC_ALTERNATIVE_ROCM_LIB_FOLDER);

        // Paths to alternative glslang/spirv-tools for Vulkan.
        po::options_description vk_compiler_paths_opt("Alternative glslang compiler and SPIR-V tools\nRGA uses the glslang package that it ships with as the default front-end compiler for Vulkan. Use this option to provide a custom glslang package");
        vk_compiler_paths_opt.add_options()
            ("compiler-bin", po::value<string>(&config.compiler_bin_path), CLI_DESC_ALTERNATIVE_VK_BIN_FOLDER);

        // "Hidden" options.
        po::options_description hidden_opt("Options that we don't show with --help.");
        hidden_opt.add_options()
            ("?", "Produce help message.")
            ("input", po::value<std::vector<string>>(&config.input_files), "Input for analysis.")
            ("version-info", "Generate RGA CLI version info file. If no file name is provided, the version info is printed to stdout.")
            ("session-metadata", po::value<string>(&config.session_metadata_file), "Generate session metadata file with the list of output files generated by RGA.")
            ("log", po::value<string>(&config.log_file), "Path to the CLI log file")
            ("no-suffix-bin", "If specified, do not add a suffix to names of generated binary files.")
            ("no-prefix-device-bin", "If specified, do not add a device prefix to names of generated binary files.")
            ("state-desc", po::value<std::string>(&config.dxr_state_desc), "Full path to the DXR state description file.")
            ("parse-isa", "Generate a CSV file with a breakdown of each ISA instruction into opcode, operands. etc.")
            ("csv-separator", po::value<string>(&config.csv_separator), "Override to default separator for analysis items.")
            ("retain", "Retain temporary output files.")
            ;

        // All options available from command line
        po::options_description all_opt;
        all_opt.add(generic_opt).add(macro_and_include_opt).add(cl_opt).add(hidden_opt).add(dx_opt).add(dx12_opt).add(dxr_opt).add(pipelined_opt_offline)
              .add(il_dump_opt).add(line_numbers_opt).add(warnings_opt).add(opt_level_opt2).add(rocm_compiler_paths_opt).add(dxc_options).add(dx12_other_options);

#ifdef _LEGACY_OPENCL_ENABLED
        all_opt.add(legacy_cl_opt);
#endif // _LEGACY_OPENCL_ENABLED

#ifdef RGA_ENABLE_VULKAN
        all_opt.add(vulkan_opt).add(vulkan_input_type_opt).add(vulkan_opt_hidden);
#endif

        po::variables_map vm;
        po::positional_options_description positional_opt;
        positional_opt.add("input", -1);

        // Parse command line
        store(po::command_line_parser(argc, argv).
            options(all_opt).positional(positional_opt).run(), vm);

        // Handle Options
        notify(vm);

        if (vm.count("help") || vm.count("?") || vm.size() == 0)
        {
            config.requested_command = Config::kHelp;
        }

        if (vm.count("retain"))
        {
            config.should_retain_temp_files = true;
        }

        if (vm.count("list-asics"))
        {
            config.requested_command = Config::kListAsics;
        }
        else if (vm.count("list-kernels") || vm.count("list-entries"))
        {
            config.requested_command = Config::kListEntries;
        }
        else if (vm.count("gpso-template"))
        {
            config.requested_command = Config::kGenTemplateFile;
        }

        if (vm.count("intrinsics"))
        {
            config.enable_shader_intrinsics = true;
        }

        if (vm.count("dxbc"))
        {
            config.dxbc_input_dx11 = true;
        }

        if (vm.count("adapters"))
        {
            config.requested_command = Config::kListAdapters;
        }

        if (vm.count("parse-isa"))
        {
            config.is_parsed_isa_required = true;
        }

        if (vm.count("line-numbers"))
        {
            config.is_line_numbers_required = true;
        }

        if (vm.count("warnings"))
        {
            config.is_warnings_required = true;
        }

        if (vm.count("verbose"))
        {
            config.print_process_cmd_line = true;
        }

        if (vm.count("debug-layer"))
        {
            config.dx12_debug_layer_enabled = true;
        }

        if (vm.count("cfg") && vm.count("cfg-i"))
        {
            std::cerr << kStrErrorBothCfgAndCfgiSpecified << std::endl;
            do_work = false;
        }

        // Set the optimization level.
        if (vm.count("O0"))
        {
            config.opt_level = 0;
        }
        else if (vm.count("O1"))
        {
            config.opt_level = 1;
        }
        else if (vm.count("O2"))
        {
            config.opt_level = 2;
        }
        else if (vm.count("O3"))
        {
            config.opt_level = 3;
        }

        // Set the default livereg output file name if not provided by a user.
        if (vm.count("--livereg") && config.livereg_analysis_file.empty())
        {
            config.livereg_analysis_file = kStrDefaultFilenameLivereg;
        }

        if (vm.count("no-suffix-bin") > 0)
        {
            config.should_avoid_binary_suffix = true;
        }

        if (vm.count("no-prefix-device-bin"))
        {
            config.should_avoid_binary_device_prefix = true;
        }

        if (vm.count("version-info"))
        {
            config.version_info_file = (vm.count("input") ? config.input_files[0] : "");
            config.requested_command = Config::kGenVersionInfoFile;
        }
        else if (vm.count("version"))
        {
            config.requested_command = Config::kVersion;
        }
        else if (vm.count("updates"))
        {
            config.requested_command = Config::kUpdate;
        }
        else if (!config.analysis_file.empty() || !config.il_file.empty() || !config.isa_file.empty() ||
                 !config.livereg_analysis_file.empty() || !config.binary_output_file.empty() ||
                 !config.metadata_file.empty() || !config.block_cfg_file.empty() ||
                 !config.inst_cfg_file.empty() || !config.spv_txt.empty() || !config.spv_bin.empty() ||
                 !config.parsed_spv.empty())
        {
            config.requested_command = Config::kCompile;
        }

        if (config.requested_command == Config::kNone)
        {
            std::cout << kStrErrorNoValidCommandDetected << std::endl;
        }

        // If the "--validation" option for Vulkan mode is specified, the validation info should be dumped to stdout.
        if (validation && vm.count("validation-file") == 0)
        {
            config.vulkan_validation = KC_STR_VK_VALIDATION_INFO_STDOUT;
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
            config.mode = RgaMode::kModeDx11;
        }
        else if (src_kind == Config::source_kind_dx12)
        {
            config.mode = kModeDx12;
        }
        else if (src_kind == Config::source_kind_dxr)
        {
            config.mode = kModeDxr;
        }
        else if (src_kind == Config::source_kind_amdil)
        {
            config.mode = RgaMode::kModeAmdil;
        }
        else if (src_kind == Config::source_kind_opencl)
        {
            config.mode = kModeOpencl;
#ifndef _LEGACY_OPENCL_ENABLED
            // Legacy OpenCL mode is no longer supported - abort.
            std::cout << kStrInfoLegacyOpenclNoLongerSupported << std::endl;
            do_work = false;
#endif // _LEGACY_OPENCL_ENABLED
        }
        else if (src_kind == Config::source_kind_opengl)
        {
            config.mode = RgaMode::kModeOpengl;
        }
        else if (src_kind == Config::source_kind_glsl_vulkan_offline)
        {
            config.mode = RgaMode::kModeVkOffline;
        }
        else if (src_kind == Config::source_kind_spirv_bin_offline)
        {
            config.mode = RgaMode::kModeVkOfflineSpv;
        }
        else if (src_kind == Config::source_kind_spirv_txt_offline)
        {
            config.mode = RgaMode::kModeVkOfflineSpvTxt;
        }
#ifdef RGA_ENABLE_VULKAN
        else if (src_kind == Config::source_kind_vulkan)
        {
            config.mode = RgaMode::kModeVulkan;
        }
#endif
        else if (src_kind == Config::source_kind_rocm_opencl)
        {
            config.mode = RgaMode::kModeRocmOpencl;
        }
        else
        {
            config.mode = kModeInvalid;
            cout << "Source language: " << config.source_kind << " not supported.\n";
        }

#ifdef RGA_ENABLE_VULKAN
        // Set the Vulkan per-stage input file types.
        if (config.mode == RgaMode::kModeVulkan)
        {
            for (const auto& stage : std::map<std::string, std::pair<std::string*, RgVulkanInputType*>>
                    { {"vert", {&config.vertex_shader, &config.vert_shader_file_type}},         {"tesc", {&config.tess_control_shader, &config.tesc_shader_file_type}},
                      {"tese", {&config.tess_evaluation_shader, &config.tese_shader_file_type}}, {"frag", {&config.fragment_shader, &config.frag_shader_file_type}},
                      {"geom", {&config.geometry_shader, &config.geom_shader_file_type}},       {"comp", {&config.compute_shader, &config.comp_shader_file_type}} })
            {
                for (const std::pair<std::string, RgVulkanInputType>& fileType :
                         std::map<std::string, RgVulkanInputType>{ {"glsl", RgVulkanInputType::kGlsl} },
                         std::map<std::string, RgVulkanInputType>{ {"spvas", RgVulkanInputType::kSpirvTxt} })
                {
                    if (vm.count(stage.first + "-" + fileType.first))
                    {
                        *(stage.second.first) = vm[stage.first + "-" + fileType.first].as<std::string>();
                        *(stage.second.second) = fileType.second;
                    }
                }
            }
        }
#endif

#if _WIN32
        const string  program_name = "rga.exe";
#else
        const string  program_name = "rga";
#endif

        cout << std::endl;
        if ((config.requested_command == Config::kHelp) && (!is_source_specified))
        {
            std::string product_version;
            cout << kStrRgaProductName << " " << kStrRgaVersionPrefix << STR_RGA_VERSION << "." << STR_RGA_BUILD_NUM << std::endl;
            cout << kStrRgaProductName << " is a compiler and code analysis tool for OpenCL";
#if _WIN32
            cout << ", DirectX";
#endif
            cout << ", OpenGL and Vulkan" << std::endl << std::endl;
#ifdef RGA_ENABLE_VULKAN
            cout << "To view help for Vulkan mode: -h -s vulkan" << endl;
#endif
            cout << "To view help for offline Vulkan (GLSL) mode: -h -s vk-offline" << endl;
            cout << "To view help for offline Vulkan (SPIR-V binary) mode: -h -s vk-spv-offline" << endl;
            cout << "To view help for offline Vulkan (SPIR-V text) mode: -h -s vk-spv-txt-offline" << endl;
#if _WIN32
            cout << "To view help for DirectX 12 mode: -h -s dx12" << std::endl;
            cout << "To view help for DirectX 11 mode: -h -s dx11" << std::endl;
            cout << "To view help for DXR mode: -h -s dxr" << std::endl;
            cout << "To view help for AMDIL mode: -h -s amdil" << std::endl;
#endif
            cout << "To view help for OpenGL mode: -h -s opengl" << endl;
            cout << "To view help for ROCm OpenCL mode: -h -s rocm-cl" << std::endl;
#ifdef _LEGACY_OPENCL_ENABLED
            cout << "To view help for legacy OpenCL mode: -h -s cl" << std::endl;
#endif // !_LEGACY_OPENCL_ENABLED
            cout << std::endl;
            cout << "To see the current RGA version: --version" << std::endl;
            cout << "To check for available updates: --updates" << std::endl;
        }
        else if ((config.requested_command == Config::kHelp) && (config.mode == kModeOpencl))
        {
#ifdef _LEGACY_OPENCL_ENABLED
            // Put all options valid for this mode to one group to make the description aligned.
            po::options_description ocl_options;
            ocl_options.add(generic_opt).add(il_dump_opt).add(macro_and_include_opt).add(cl_opt).add(legacy_cl_opt);
            cout << "*** Legacy OpenCL mode options ***" << endl;
            cout << "==================================" << endl;
            cout << ocl_options << endl;
            cout << "Examples:" << endl;
            cout << "  " << "Compile foo.cl for all supported devices; extract ISA, IL code and statistics:" << endl;
            cout << "    " << program_name << " -s cl --isa output/foo_isa.txt --il output/foo_il.txt -a output/stats.csv foo.cl" << endl;
            cout << "  " << "Compile foo.cl for Fiji; extract ISA and perform live register analysis:" << endl;
            cout << "    " << program_name << " -s cl -c Fiji --isa output/foo_isa.txt --livereg output/regs.txt foo.cl" << endl;
            cout << "  " << "Compile foo.cl for gfx906; extract binary and control flow graphs:" << endl;
            cout << "    " << program_name << " -s cl -c gfx906 --bin output/foo.bin --cfg output/cfg.dot foo.cl" << endl;
            cout << "  " << "List the kernels available in foo.cl:" << endl;
            cout << "    " << program_name << " -s cl --list-kernels foo.cl" << endl;
            cout << "  " << "Compile foo.cl for Bonaire; extract the hardware resource usage statistics for myKernel.  Write the statistics to foo.csv:" << endl;
            cout << "    " << program_name << " -s cl -c \"Bonaire\" --kernel myKernel -a foo.csv foo.cl" << endl;
            cout << "  " << "List the ASICs supported by Legacy OpenCL mode:" << endl;
            cout << "    " << program_name << " -s cl --list-asics" << endl;
            cout << endl;
#endif // !_LEGACY_OPENCL_ENABLED
        }
        else if ((config.requested_command == Config::kHelp) && (config.mode == kModeDxr))
        {
            cout << "*** DXR mode options (Windows only) ***" << endl;
            cout << "=======================================" << endl << endl;
            cout << "Usage: " << program_name << " [options]" << endl << endl;
            cout << generic_opt << endl;
            cout << macro_and_include_opt << endl;
            cout << dxr_opt << endl;
            cout << dxc_options << endl;
            cout << dx12_other_options << endl;
            cout << "Examples:" << endl;
            cout << "  View supported targets for DXR:" << endl;
            cout << "    " << program_name << " -s dxr -l" << endl;
            cout << "  Compile and generate RDNA ISA disassembly for a shader named MyRaygenShader which is defined in C:\\shaders\\Raytracing.hlsl:" << endl;
            cout << "    " << program_name << " -s dxr --hlsl C:\\shaders\\Raytracing.hlsl --export MyRaygenShader --isa C:\\output\\isa.txt" << endl;
            cout << "  Compile and generate RDNA ISA disassembly and HW resource usage statistics for a shader named MyClosestHitShader which is defined in C:\\shaders\\Raytracing.hlsl:" << endl;
            cout << "    " << program_name << " -s dxr --hlsl C:\\shaders\\Raytracing.hlsl --export MyClosestHitShader --isa C:\\output\\isa.txt -a C:\\output\\stats.txt" << endl;
            cout << "  Compile and generate RDNA ISA disassembly for all DXR pipelines defined in C:\\shaders\\Raytracing.hlsl:" << endl;
            cout << "    " << program_name << " -s dxr --mode pipeline --hlsl C:\\shaders\\Raytracing.hlsl --isa C:\\output\\isa.txt" << endl;
                        cout << "  Compile and generate RDNA ISA disassembly for all DXR pipelines which are defined in C:\\shaders\\rt.hlsl with additional headers that are located in C:\\shaders\\include:" << endl;
            cout << "    " << program_name << " -s dxr --mode pipeline --hlsl C:\\shaders\\rt.hlsl -I C:\\shaders\\include --isa C:\\output\\isa.txt" << endl;

        }
        else if ((config.requested_command == Config::kHelp) && (config.mode == RgaMode::kModeRocmOpencl))
        {
            // Put all options valid for this mode to one group to make the description aligned.
            po::options_description  oclOptions;
            oclOptions.add(generic_opt).add(il_dump_opt).add(warnings_opt).add(macro_and_include_opt).add(cl_opt).add(line_numbers_opt).add(opt_level_opt2).add(rocm_compiler_paths_opt);
            cout << "*** ROCm OpenCL mode options ***" << endl;
            cout << "================================" << endl << endl;
            cout << "Usage: " << program_name << " [options] source_file(s)" << endl;
            cout << oclOptions << endl;
            cout << "Note: In case that your alternative compiler supports targets which are not known to the RGA build that you are using, "
                    "use the x64/ROCm/additional-targets text file, which contains a list of targets that would be dynamically loaded by RGA. "
                    "To add targets, simply list them in a new line in the file, and RGA would load them while running." << endl << endl;

            cout << "Examples:" << endl;
            cout << "  " << "Compile test.cl for Vega Frontier and extract the binary:" << endl;
            cout << "    " << program_name << " -s rocm-cl -c \"vega frontier\" -b output/test.bin test.cl" << endl;
            cout << "  " << "Compile and link src1.cl, src2.cl and src3.cl into an HSA Code Object for Vega (gfx900), and extract ISA disassembly:" << endl;
            cout << "    " << program_name << " -s rocm-cl -c gfx900 --isa output/isa.txt src1.cl src2.cl src3.cl" << endl;
            cout << "  " << "Compile test.cl for all supported targets, extract ISA and perform live register analysis:" << endl;
            cout << "    " << program_name << " -s rocm-cl --isa test_isa.txt --livereg regs.txt test.cl" << endl;
            cout << "  " << "List the kernels available in test.cl:" << endl;
            cout << "    " << program_name << " -s rocm-cl --list-kernels test.cl" << endl;
            cout << "  " << "List the ASICs supported by ROCm OpenCL Lightning Compiler:" << endl;
            cout << "    " << program_name << " -s rocm-cl --list-asics" << endl;
            cout << "  " << "Compile test.cl and extract ISA disassembly for gfx900 using alternative OpenCL compiler:" << endl;
            cout << "    " << program_name << " -s rocm-cl -c gfx900 --isa test.isa --compiler-bin C:\\llvm\\dist\\bin --compiler-inc C:\\llvm\\dist\\include --compiler-lib C:\\llvm\\dist\\lib\\bitcode  test.cl" << endl;

            cout << endl;
        }
        else if ((config.requested_command == Config::kHelp) && (config.mode == RgaMode::kModeDx11))
        {
            cout << "*** DirectX 11 mode options (Windows Only) ***" << endl;
            cout << "==============================================" << endl << endl;
            cout << "Usage: " << program_name << " [options] source_file" << endl << endl;
            cout << generic_opt << endl;
            cout << il_dump_opt << endl;
            cout << macro_and_include_opt << endl;
            cout << dx_opt << endl;
            cout << "Examples:" << endl;
            cout << "  View supported ASICS for DX11:" << endl;
            cout << "    " << program_name << " -s dx11 -l" << endl;
            cout << "  Compile myShader.hlsl for all supported targets and extract the ISA disassembly:" << endl;
            cout << "    " << program_name << " -s dx11 -f VsMain -p vs_5_0 --isa output/myShader_isa.txt src/myShader.hlsl" << endl;
            cout << "  Compile myShader.hlsl for Fiji; extract the ISA and perform live register analysis:" << endl;
            cout << "    " << program_name << " -s dx11 -c Fiji -f VsMain -p vs_5_0 --isa output/myShader_isa.txt --livereg output/regs.txt myShader.hlsl" << endl;
            cout << "  Compile myShader.hlsl for Radeon R9 390; perform static analysis and save the statistics to myShader.csv:" << endl;
            cout << "    " << program_name << " -s dx11 -c r9-390 -f VsMain -p vs_5_0 -a output/myShader.csv shaders/myShader.hlsl" << endl;
        }
        else if ((config.requested_command == Config::kHelp) && (config.mode == kModeDx12))
        {
        cout << "*** DirectX 12 mode options (Windows only) ***" << endl;
        cout << "==============================================" << endl << endl;
        cout << "Usage: " << program_name << " [options]" << endl << endl;
        cout << generic_opt << endl;
        cout << macro_and_include_opt << endl;
        cout << dx12_opt << endl;
        cout << dxc_options << endl;
        cout << dx12_other_options << endl;
        cout << "Examples:" << endl;
        cout << "  View supported ASICS for DX12:" << endl;
        cout << "    " << program_name << " -s dx12 -l" << endl;
        cout << "  Compile a cs_5_1 compute shader named \"CSMain\" for gfx1010, where the root signature is referenced through a [RootSignature()] attribute. Generate ISA disassembly and resource usage statistics:" << endl;
        cout << "    " << program_name << " -s dx12 -c gfx1010 --cs C:\\shaders\\FillLightGridCS_8.hlsl -cs-entry CSMain --cs-model cs_5_1 --isa C:\\output\\isa.txt -a C:\\output\\stats.txt" << endl;
        cout << "  Compile a graphics pipeline with model 6.0 vertex (\"VSMain\") and pixel (\"PSMain\") shaders defined in vert.hlsl and pixel.hlsl respectively, while the root signature is in a pre-compiled binary file. Generate ISA disassembly and resource usage statistics:" << endl;
        cout << "    " << program_name << " -s dx12 --vs C:\\shaders\\vert.hlsl --vs-model vs_6_0 --vs-entry VSMain --ps C:\\shaders\\pixel.hlsl --ps-model ps_6_0 --ps-entry PSMain --gpso C:\\shaders\\state.gpso --rs-bin C:\\rootSignatures\\rs.bin --isa C:\\output\\disassembly.txt -a C:\\output\\stats.txt " << endl;
        cout << "  Compile a graphics pipeline with vertex (\"VSMain\") and pixel (\"PSMain\") shaders defined both in shader.hlsl, while the root signature is in a pre-compiled binary file. Generate ISA disassembly and resource usage statistics:" << endl;
        cout << "    " << program_name << " -s dx12 --all-hlsl C:\\shaders\\shaders.hlsl --all-model 6_0 --vs-entry VSMain --ps-entry PSMain --gpso C:\\shaders\\state.gpso --rs-bin C:\\rootSignatures\\rs.bin --isa C:\\output\\disassembly.txt -a C:\\output\\stats.txt" << endl;
        cout << "  Compile a cs_5_1 compute shader named \"CSMain\" for Radeon VII (gfx906), where the root signature is in a binary file (C:\\RS\\FillLightRS.rs.fxo). Generate ISA disassembly and resource usage statistics:" << endl;
        cout << "    " << program_name << " -s dx12 -c gfx906 --cs C:\\shaders\\FillLightGridCS_8.hlsl --cs-model cs_5_1 --cs-entry main --rs-bin C:\\RS\\FillLightRS.rs.fxo --isa C:\\output\\lightcs_dis.txt -a C:\\output\\stats.txt" << endl;
        cout << "  Compile a DXIL or DXBC blob for Navi10 (gfx1010) and generate ISA disassembly:" << endl;
        cout << "    " << program_name << " -s dx12 -c gfx1010 --cs-blob C:\\shaders\\FillLightGridCS_8.obj --isa C:\\output\\lightcs_dis.txt" << endl;
        }
        else if ((config.requested_command == Config::kHelp) && (config.mode == RgaMode::kModeAmdil))
        {
            cout << "*** AMDIL mode options (Windows only) ***" << endl;
            cout << "=========================================" << endl << endl;
            cout << "Usage: " << program_name << " [options] source_file" << endl << endl;
            cout << generic_opt << endl;
            cout << "Examples:" << endl;
            cout << "  Generate ISA from AMDIL code for all supported targets:" << endl;
            cout << "    " << program_name << " -s amdil --isa output/isaFromAmdil.isa myAmdilCode.amdil" << endl;
            cout << "  Generate ISA for Fiji from AMDIL code and extract statistics:" << endl;
            cout << "    " << program_name << " -s amdil -c Fiji --isa output/isaFromAmdil.isa -a output/statsFromAmdil.csv myAmdilCode.amdil" << endl;
            cout << "  Generate ISA for gfx900 from AMDIL code and perform live register analysis:" << endl;
            cout << "    " << program_name << " -s amdil -c gfx900 --isa output/myShader.isa --livereg output/regs.txt myAmdilCode.amdil" << endl;
        }
        else if ((config.requested_command == Config::kHelp) && (config.mode == RgaMode::kModeVkOffline ||
                  config.mode == RgaMode::kModeVkOfflineSpv || config.mode == RgaMode::kModeVkOfflineSpvTxt))
        {
            // Get the mode name, and the relevant file extensions. We will use it when constructing the example string.
            const char* kFileExtSpv = "spv";
            const char* kFileExtSpvTxt = "txt";
            std::string rga_mode_name;
            std::string vert_ext;
            std::string frag_ext;
            switch (config.mode)
            {
            case RgaMode::kModeVkOffline:
                rga_mode_name = Config::source_kind_glsl_vulkan_offline;
                vert_ext = "vert";
                frag_ext = "frag";
                break;
            case RgaMode::kModeVkOfflineSpv:
                rga_mode_name = Config::source_kind_spirv_bin_offline;
                vert_ext = kFileExtSpv;
                frag_ext = kFileExtSpv;
                break;
            case RgaMode::kModeVkOfflineSpvTxt:
                rga_mode_name = Config::source_kind_spirv_txt_offline;
                vert_ext = kFileExtSpvTxt;
                frag_ext = kFileExtSpvTxt;
                break;
            }

            // Convert the mode name string to lower case for presentation.
            std::transform(rga_mode_name.begin(), rga_mode_name.end(), rga_mode_name.begin(), ::tolower);

            cout << "*** Vulkan Offline mode options ***" << endl;
            cout << "===================================" << endl << endl;
            cout << "The Vulkan offline mode is independent of the installed driver and may not provide assembly code and resource"
                    " usage that reflect the real-life case. To get results that reflect the real-life performance of your code, "
                    "please use RGA's Vulkan live-driver mode (-s vulkan)." << endl << endl;
            cout << "Usage: " << program_name << " [options]";
            if (config.mode == RgaMode::kModeVkOfflineSpv || config.mode == RgaMode::kModeVkOfflineSpvTxt)
            {
                cout << " [optional: spv_input_file]";
            }
            cout << endl << endl;
            if (config.mode == RgaMode::kModeVkOfflineSpv || config.mode == RgaMode::kModeVkOfflineSpvTxt)
            {
                cout << "Notes:" << endl;
                cout << " * The input file(s) must be specified in one of two ways:" << endl <<
                    "   1) A single SPIR-V input file provided as \"spv_input_file\", or\n" <<
                    "   2) One or more pipeline stage specific shader files specified by the pipeline stage options (--vert, --tesc, etc.)." << endl << endl;
            }
            cout << generic_opt << endl;
            cout << opt_level_opt1 << endl;
            cout << pipelined_opt_offline << endl;
            cout << "Examples:" << endl;
            cout << "  Compile vertex & fragment shaders for all supported devicesl; extract ISA, AMD IL and statistics:" << endl;
            cout << "    " << program_name << " -s " << rga_mode_name << " --isa output/isa.txt --il output/il.txt -a output/stats.csv --vert source/myVertexShader." << vert_ext << " --frag source/myFragmentShader." << frag_ext << endl;
            cout << "  Compile vertex & fragment shaders for Iceland and Fiji; extract ISA, AMD IL and statistics:" << endl;
            cout << "    " << program_name << " -s " << rga_mode_name << " -c Iceland -c Fiji --isa output/isa.txt --il output/il.amdil -a output/.csv --vert source/myVertexShader." << vert_ext << " --frag source/myFragmentShader." << frag_ext << endl;
            cout << "  Compile vertex shader for Radeon R9 390; extract ISA and binaries:" << endl;
            cout << "    " << program_name << " -s " << rga_mode_name << " -c \"R9 390\" --isa output/isa.txt -b output/binary.bin -a output/stats.csv --vert c:\\source\\myVertexShader." << vert_ext << endl;
            if (config.mode == RgaMode::kModeVkOfflineSpv || config.mode == RgaMode::kModeVkOfflineSpvTxt)
            {
                cout << "  Extract ISA for a single SPIR-V file for Baffin, without specifying the pipeline stages:" << endl;
                cout << "    " << program_name << " -s " << rga_mode_name << " -c Baffin --isa output/program_isa.txt source/program.spv" << endl;
            }
        }
        else if ((config.requested_command == Config::kHelp) && (config.mode == RgaMode::kModeOpengl))
        {
            cout << "*** OpenGL mode options ***" << endl;
            cout << "===========================" << endl << endl;
            cout << "Usage: " << program_name << " [options]" << endl << endl;
            cout << generic_opt << endl;
            cout << il_dump_opt << endl;
            cout << pipelined_opt_live << endl;
            cout << "Examples:" << endl;
            cout << "  Compile fragment shader for Baffin; extract ISA, binaries and statistics:" << endl;
            cout << "    " << program_name << " -s opengl --isa output/opengl_isa.txt -b output/opengl_bin.bin -a output/opengl_stats.csv --frag source/myFragmentShader.frag" << endl;
            cout << "  Compile vertex & fragment shaders for FirePro W7100; Extract ISA and control flow graph: " << endl;
            cout << "    " << program_name << " -s opengl -c W7100 --isa output/opengl_isa.txt --cfg output/cfg.dot --vert myVertexShader.vert --frag cmyFragmentShader.frag" << endl;
            cout << "  Compile geometry shader for all supported devices; extract ISA and perform live register analysis:" << endl;
            cout << "    " << program_name << " -s opengl --isa output/opengl_isa.txt --livereg output/regs.txt --geom source/myVertexShader.geom" << endl;
        }
#ifdef RGA_ENABLE_VULKAN
        else if ((config.requested_command == Config::kHelp) && (config.mode == RgaMode::kModeVulkan))
        {
            cout << "*** Vulkan mode options ***" << endl;
            cout << "===========================" << endl << endl;
            cout << "Usage: " << program_name << " [options]" << endl << endl;
            cout << generic_opt << endl;
            cout << macro_and_include_opt << endl;
            cout << pipelined_opt_live << endl;

            cout << "By default, input files would be interpreted as SPIR-V binary files, unless the file extension is any of the following:" << endl <<
                "  .vert  --> GLSL source file." << endl <<
                "  .frag  --> GLSL source file." << endl <<
                "  .tesc  --> GLSL source file." << endl <<
                "  .tese  --> GLSL source file." << endl <<
                "  .geom  --> GLSL source file." << endl <<
                "  .comp  --> GLSL source file." << endl <<
                "  .glsl  --> GLSL source file." << endl <<
                "  .vs    --> GLSL source file." << endl <<
                "  .fs    --> GLSL source file." << endl << endl;
            cout << vulkan_opt << endl;
            cout << vk_compiler_paths_opt << endl;
        }
#endif
        else if ((config.requested_command == Config::kUpdate))
        {
            KcUtils::CheckForUpdates();
        }
    }
    catch (exception& e)
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
//=================================================================
// Copyright 2017 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

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
#include <RadeonGPUAnalyzerCLIConfig.h>
#endif

// Local.
#include <RadeonGPUAnalyzerCLI/Src/kcConfig.h>
#include <RadeonGPUAnalyzerCLI/Src/kcParseCmdLine.h>
#include <RadeonGPUAnalyzerCLI/Src/kcCliStringConstants.h>
#include <RadeonGPUAnalyzerCLI/Src/kcUtils.h>

// Shared between CLI and GUI.
#include <Utils/Include/rgaVersionInfo.h>
#include <Utils/Include/rgaCliDefs.h>

namespace po = boost::program_options;
using namespace std;

#if GSA_BUILD
#define KERNEL_OPTION "function"
#else
#define KERNEL_OPTION "kernel"
#endif

bool ParseCmdLine(int argc, char* argv[], Config& config)
{
    //----- Command Line Options Parsing
    bool doWork = true;
    bool validation = false;

    try
    {
        // Parse command line options using Boost library

        // Generic Options: Valid got CL, DX, GL
        po::options_description genericOpt("Generic options");

        genericOpt.add_options()
            // The next two options here can be repeated, so they are vectors.
            ("csv-separator", po::value<string>(&config.m_CSVSeparator), "Override to default separator for analysis items.")
            ("list-asics,l", "List the known GPU codenames, architecture names and variant names.\nTo target a specific GPU, use its codename as the argument to the \"-c\" command line switch.")
            ("asic,c", po::value< vector<string> >(&config.m_ASICs), "Which ASIC to target.  Repeatable.")
            ("version", "Print version string.")
            ("help,h", "Produce this help message.")
            ("analysis,a", po::value<string>(&config.m_AnalysisFile), "Path to output analysis file.")
            ("binary,b", po::value<string>(&config.m_BinaryOutputFile), "Path to HSA Code Object binary output file.")
            ("isa", po::value<string>(&config.m_ISAFile), "Path to output ISA disassembly file(s).")
            ("livereg", po::value<string>(&config.m_LiveRegisterAnalysisFile), "Path to live register analysis output file(s).")
            ("cfg", po::value<string>(&config.m_blockCFGFile), "Path to per-block control flow graph output file(s).")
            ("cfg-i", po::value<string>(&config.m_instCFGFile), "Path to per-instruction control flow graph output file(s).")
            ("source-kind,s", po::value<string>(&config.m_SourceKind), "Source platform: dx12 for DirectX 12, dx11 for DirectX 11, vulkan for Vulkan, opengl for OpenGL, cl for OpenCL legacy, rocm-cl for OpenCL Lightning Compiler and amdil for AMDIL.")
            ("parse-isa", "Generate a CSV file with a breakdown of each ISA instruction into opcode, operands. etc.")
            ("updates,u", "Check for available updates.")
            ("verbose,v", "Print command line strings that RGA uses to launch external processes.")
            ;

        // DX Options
        std::string  adaptersDesc = "List all of the supported display adapters that are installed on the system.\n" + std::string(STR_DX_ADAPTERS_HELP_COMMON_TEXT);
        std::string  setAdaptDesc = "Specify the id of the display adapter whose driver you would like RGA to use.\n" + std::string(STR_DX_ADAPTERS_HELP_COMMON_TEXT);

        po::options_description dxOpt("DirectX 11 mode options");
        dxOpt.add_options()
            ("function,f", po::value<string>(&config.m_Function), "DX shader entry point.")
            ("profile,p", po::value<string>(&config.m_Profile), "Profile to use for compilation.  REQUIRED.\nFor example: vs_5_0, ps_5_0, etc.")
            ("DXFlags", po::value<unsigned int>(&config.m_DXFlags), "Flags to pass to D3DCompile.")
            ("DXLocation", po::value<string>(&config.m_DXLocation), "Location to the D3DCompiler Dll required for compilation. If none is specified, the default D3D compiler that is bundled with the Analyzer will be used.")
            ("FXC", po::value<string>(&config.m_FXC), "FXC Command Line. Use full path and specify all arguments in \"\". For example:\n"
                                                      "   rga.exe  -f VsMain1 -s DXAsm -p vs_5_0 <Path>\\vsBlob.obj  --isa <Path>\\vsTest.isa --FXC \"<Path>\\fxc.exe /E VsMain1 /T vs_5_0 /Fo <Path>\\vsBlob.obj <Path>\\vsTest.fx\"\n "
                                                      "   In order to use it, DXAsm must be specified. /Fo switch must be used and output file must be the same as the input file for rga.")
            ("DumpMSIntermediate", po::value<string>(&config.m_DumpMSIntermediate), "Location to save the MS Blob as text. ")
            ("intrinsics", "Enable AMD D3D11 Shader Intrinsics extension.")
            ("adapters", adaptersDesc.c_str())
            ("set-adapter", po::value<int>(&config.m_DXAdapter), setAdaptDesc.c_str())
            ("UAVSlot", po::value<int>(&config.m_UAVSlot), "This value should be in the range of [0,63].\nThe driver will use the slot to track which UAV is being used to specify the intrinsic. The UAV slot that is selected cannot be used for any other purposes.\nThis option is only relevant when AMD D3D11 Shader Intrinsics is enabled (specify --intrinsics).")
            ("dxbc", "Treat input file as a DXBC binary.")
            ;

        po::options_description macroAndIncludeOpt("Macro and Include paths Options");
        macroAndIncludeOpt.add_options()
            // The next two options here can be repeated, so they are vectors.
            ("define,D", po::value< vector<string> >(&config.m_Defines), "Define symbol or symbol=value. Repeatable.")
            ("IncludePath,I", po::value< vector<string> >(&config.m_IncludePath), "Additional include path required for compilation.  Repeatable.")
            ;

        // CL Option
        po::options_description clOpt("");
        clOpt.add_options()
            ("list-kernels", "List kernel functions.")
            ("metadata,m", po::value<string>(&config.m_MetadataFile), "Path to output Metadata file(s).\n"
                "Requires --" KERNEL_OPTION ".")
                ("kernel,k", po::value<string>(&config.m_Function), "Kernel to be compiled and analyzed. If not specified, all kernels will be targeted. Note "
                    "that this option is only relevant when the compiler can generate per-kernel data. When the runtime uses the updated compiler to generate "
                    "a Code Object type binary, the ISA would be generated for the entire program, since the binary has no per-kernel code sections.\n")
                    ("OpenCLoption", po::value< vector<string> >(&config.m_OpenCLOptions), "OpenCL compiler options.  Repeatable.");

        // Vulkan shader type.
        po::options_description pipelinedOpt("Input shader type");
        pipelinedOpt.add_options()
            ("vert", po::value<string>(&config.m_VertexShader), "Full path to vertex shader input file.")
            ("tesc", po::value<string>(&config.m_TessControlShader), "Full path to tessellation control shader input file.")
            ("tese", po::value<string>(&config.m_TessEvaluationShader), "Full path to tessellation evaluation shader input file.")
            ("geom", po::value<string>(&config.m_GeometryShader), "Full path to geometry shader input file.")
            ("frag", po::value<string>(&config.m_FragmentShader), "Full path to fragment shader input file")
            ("comp", po::value<string>(&config.m_ComputeShader), "Full path to compute shader input file.");

#ifdef RGA_ENABLE_VULKAN
        // Prepare the --glslang-opt description.
        std::string glslangOptDescription = CLI_OPT_GLSLANG_OPT_DESCRIPTION_A;
        glslangOptDescription.append(CLI_OPT_GLSLANG_OPT_DESCRIPTION_B);

        // Live Vulkan mode options.
        po::options_description vulkanOpt("Vulkan mode options");
        vulkanOpt.add_options()
            ("[stage]-glsl arg", "Full path to [stage] input file, while forcing interpretation of input file as glsl source file rather than SPIR-V binary. arg is the full path to [stage] shader input file. [stage] can be one "
                "of: \"vert\", \"tesc\", \"tese\", \"geom\", \"frag\", \"comp\". "
                "For example, use \"--vert-glsl shader.glsl\" to specify path to the vertex shader file and indicate that it is a glsl source file (rather than a SPIR-V binary).")
            ("pso", po::value<string>(&config.m_pso), "Full path to .gpso (graphics) or .cpso (compute) pipeline state JSON file. If no pipeline state file is provided, a default pipeline state would be used. In that case, if your shaders do not match the default state, the compilation would fail.")
            ("icd", po::value<string>(&config.m_icdFile), CLI_OPT_ICD_DESCRIPTION)
            ("loader-debug", po::value<string>(&config.m_loaderDebug), CLI_OPT_VK_LOADER_DEBUG_DESCRIPTION)
            ("glslang-opt", po::value<string>(&config.m_glslangOpt), glslangOptDescription.c_str())
            ("disassemble-spv", po::value<string>(&config.m_spvTxt), "Disassemble SPIR-V binary file. Accepts an optional argument with the full path to the output file where the disassembly would be saved. If not specified, the disassembly would be printed to stdout.")
            ("validation", po::bool_switch(&validation), "Enable Vulkan validation layers and dump the output of validation layers to stdout.")
            ("validation-file", po::value<string>(&config.m_vulkanValidation), "Enable Vulkan validation layers and dump the output of validation layers to the text file specified by the option argument.")
            ("parse-spv", po::value<string>(&config.m_parsedSpv), "Parse SPIR-V binary file. The option argument is the full path to the output text file with SPIR-V binary info. "
            "If path is not specified, the SPIR-V info will be printed to stdout.")
#ifdef VK_HLSL
                ("list-entries", "List hlsl function names.");
#else
            ;
#endif
        // Per-stage input file types ("--[stage]-[file-type]").
        po::options_description vulkanInputTypeOpt;
        for (const std::string& stage : { "vert", "tesc", "tese", "frag", "geom", "comp" })
        {
            for (const std::string& fileType : { "glsl" })
            {
                vulkanInputTypeOpt.add_options()
                    ((stage + "-" + fileType).c_str(), po::value<string>(), "");
            }
        }

        // Vulkan hidden options (are not presented to the user in -h).
        po::options_description vulkanOptHidden("Vulkan-specific hidden options");
        vulkanOptHidden.add_options()
            ("assemble-spv", po::value<string>(&config.m_spvBin), "Assemble SPIR-V textual code to a SPIR-V binary file. arg is the full path to the output SPIR-V binary file.");
#endif

        // DX12-specific.
        po::options_description dx12Opt("DirectX 12 mode options");
        dx12Opt.add_options()
            // HLSL.
            ("vs", po::value<string>(&config.m_vsHlsl), "Full path to hlsl file where vertex shader is defined.")
            ("hs", po::value<string>(&config.m_hsHlsl), "Full path to hlsl file where hull shader is defined.")
            ("ds", po::value<string>(&config.m_dsHlsl), "Full path to hlsl file where domain shader is defined.")
            ("gs", po::value<string>(&config.m_gsHlsl), "Full path to hlsl file where geometry shader is defined.")
            ("ps", po::value<string>(&config.m_psHlsl), "Full path to hlsl file where pixel shader is defined.")
            ("cs", po::value<string>(&config.m_csHlsl), "Full path to hlsl file where compute shader is defined.")
            ("all-hlsl", po::value<string>(&config.m_allHlsl), "Full path to the hlsl file to be used for all stages. "
                "You can use this option if all of your shaders are defined in the same hlsl file, to avoid repeating the --<stage> "
                "argument. If you use this option in addition to --<stage> or --<stage>-blob, then the --<stage> or --<stage>-blob option "
                "would override this option for stage <stage>.")
            // DXBC.
            ("vs-blob", po::value<string>(&config.m_vsDxbc), "Full path to compiled DXBC or DXIL binary where vertex shader is found.")
            ("hs-blob", po::value<string>(&config.m_hsDxbc), "Full path to compiled DXBC or DXIL binary where hull shader is found.")
            ("ds-blob", po::value<string>(&config.m_dsDxbc), "Full path to compiled DXBC or DXIL binary domain shader is found.")
            ("gs-blob", po::value<string>(&config.m_gsDxbc), "Full path to compiled DXBC or DXIL binary geometry shader is found.")
            ("ps-blob", po::value<string>(&config.m_psDxbc), "Full path to compiled DXBC or DXIL binary pixel shader is found.")
            ("cs-blob", po::value<string>(&config.m_csDxbc), "Full path to compiled DXBC or DXIL binary where compute shader is found.")
            // IL Disassembly.
            ("vs-dxil-dis", po::value<string>(&config.m_vsDxilDisassembly), "Full path to the DXIL or DXBC disassembly output file for vertex shader. "
                "Note that this option is only valid for textual input (HLSL).")
            ("hs-dxil-dis", po::value<string>(&config.m_hsDxilDisassembly), "Full path to the DXIL or DXBC disassembly output file for hull shader. "
                "Note that this option is only valid for textual input (HLSL).")
            ("ds-dxil-dis", po::value<string>(&config.m_dsDxilDisassembly), "Full path to the DXIL or DXBC disassembly output file for domain shader. "
                "Note that this option is only valid for textual input (HLSL).")
            ("gs-dxil-dis", po::value<string>(&config.m_gsDxilDisassembly), "Full path to the DXIL or DXBC disassembly output file for geometry shader. "
                "Note that this option is only valid for textual input (HLSL).")
            ("ps-dxil-dis", po::value<string>(&config.m_psDxilDisassembly), "Full path to the DXIL or DXBC disassembly output file for pixel shader. "
                "Note that this option is only valid for textual input (HLSL).")
            ("cs-dxil-dis", po::value<string>(&config.m_csDxilDisassembly), "Full path to the DXIL or DXBC disassembly output file for compute shader. "
                "Note that this option is only valid for textual input (HLSL).")
            // Target.
            ("vs-entry", po::value<string>(&config.m_vsEntryPoint), "Entry-point name of vertex shader.")
            ("hs-entry", po::value<string>(&config.m_hsEntryPoint), "Entry-point name of hull shader.")
            ("ds-entry", po::value<string>(&config.m_dsEntryPoint), "Entry-point name of domain shader.")
            ("gs-entry", po::value<string>(&config.m_gsEntryPoint), "Entry-point name of geometry shader.")
            ("ps-entry", po::value<string>(&config.m_psEntryPoint), "Entry-point name of pixel shader.")
            ("cs-entry", po::value<string>(&config.m_csEntryPoint), "Entry-point name of compute shader.")
            // Shader model.
            ("vs-model", po::value<string>(&config.m_vsModel), "Shader model of vertex shader (e.g. vs_5_1 or vs_6_0).")
            ("hs-model", po::value<string>(&config.m_hsModel), "Shader model of hull shader (e.g. hs_5_1 or hs_6_0).")
            ("ds-model", po::value<string>(&config.m_dsModel), "Shader model of domain shader (e.g. ds_5_1 or ds_6_0).")
            ("gs-model", po::value<string>(&config.m_gsModel), "Shader model of geometry shader (e.g. gs_5_1 or gs_6_0).")
            ("ps-model", po::value<string>(&config.m_psModel), "Shader model of pixel shader (e.g. ps_5_1 or ps_6_0).")
            ("cs-model", po::value<string>(&config.m_csModel), "Shader model of compute shader (e.g. cs_5_1 or cs_6_0).")
            ("all-model", po::value<string>(&config.m_allModel), "Shader model to be used for all stages (e.g. 5_1 or 6_0). Instead of "
                "specifying the model for each stage specifically, you can use this option to pass the shader model version and RGA "
                "would auto-generate the relevant model for every stage in the pipeline. Note that if you use this option together "
                "with any of the <stage>-model options, for any stage <stage> the <stage>-model option would override --all-model.")
            // Root signature.
            ("rs-bin", po::value<string>(&config.m_rsBin), "Full path to the serialized root signature "
                "to be used in the compilation process.")
            ("rs-hlsl", po::value<string>(&config.m_rsHlsl), "Full path to the HLSL file where the "
                "RootSignature macro is defined. If there is only a "
                "single hlsl input file, this option is not required.")
            ("rs-macro", po::value<string>(&config.m_rsMacro), "The name of the RootSignature macro in the HLSL "
                "code. If specified, the root signature would be compiled from the HLSL source. Use this if your "
                "shader does not include the [RootSignature()] attribute but has a root signature macro defined"
                "in the source code. Please also check the description for --rs-hlsl, which is related to this option.")
            ("rs-macro-version", po::value<string>(&config.m_rsMacroVersion), "The version of the RootSignature macro "
                "specified through the rs - macro option.By default, 'rootsig_1_1' would be assumed.")
            // Pipeline state.
            ("gpso", po::value<string>(&config.m_psoDx12), "Full path to .gpso file that describes the graphics pipeline state (required for graphics pipelines only)."
                " You can generate a template by using the --gpso-template option.")
            ("gpso-template", po::value<string>(&config.m_psoDx12Template), "Full path to where to save the template pipeline state description file. You can then edit the file to match your pipeline.")
            ("dxc", po::value<string>(&config.m_dxcPath), "Full path an alternative DXC package (a folder containing DXC.exe, dxcompiler.dll and dxil.dll). If specified, RGA would use DXC from the given path rather than the package that is ships with.")
            ("dxc-opt", po::value<string>(&config.m_dxcOpt), "Additional options to be passed to DXC when performing front-end compilation.")
            ;

        // Legacy OpenCL options.
        po::options_description legacyClOpt("");
        legacyClOpt.add_options()
            ("suppress", po::value<vector<string> >(&config.m_SuppressSection), "Section to omit from binary output.  Repeatable. Available options: .source, .amdil, .debugil,"
                ".debug_info, .debug_abbrev, .debug_line, .debug_pubnames, .debug_pubtypes, .debug_loc, .debug_str,"
                ".llvmir, .text\nNote: Debug sections are valid only with \"-g\" compile option");

        // IL dump.
        po::options_description ilDumpOpt("");
        ilDumpOpt.add_options()
            ("il", po::value<string>(&config.m_ILFile), "Path to output IL (intermediate language) disassembly file(s).");


        // Line numbers.
        po::options_description lineNumsOpt("");
        lineNumsOpt.add_options()
            ("line-numbers", "Add source line numbers to ISA disassembly.");

        // Compiler warnings.
        po::options_description warningsOpt("");
        warningsOpt.add_options()
            ("warnings,w", "Print warnings reported by the compiler.");

        // Optimization Levels.
        po::options_description optLevelOpt1("Optimization Levels");
        po::options_description optLevelOpt2("Optimization Levels");
        optLevelOpt1.add_options()
            ("O0", "Disable optimizations")
            ("O1", "Enable minimal optimizations");
        optLevelOpt2.add_options()
            ("O0", "Disable optimizations")
            ("O1", "Enable minimal optimizations")
            ("O2", "Optimize for speed")
            ("O3", "Apply full optimization");

        // Paths to user-provided ROCm compiler.
        po::options_description rocmCmplrPathsOpt("Alternative ROCm OpenCL compiler");
        rocmCmplrPathsOpt.add_options()
            ("compiler-bin", po::value<string>(&config.m_cmplrBinPath), CLI_DESC_ALTERNATIVE_ROCM_BIN_FOLDER)
            ("compiler-inc", po::value<string>(&config.m_cmplrIncPath), CLI_DESC_ALTERNATIVE_ROCM_INC_FOLDER)
            ("compiler-lib", po::value<string>(&config.m_cmplrLibPath), CLI_DESC_ALTERNATIVE_ROCM_LIB_FOLDER);

        // Paths to alternative glslang/spirv-tools for Vulkan.
        po::options_description vkCmplrPathsOpt("Alternative glslang compiler and SPIR-V tools\nRGA uses the glslang package that it ships with as the default front-end compiler for Vulkan. Use this option to provide a custom glslang package");
        vkCmplrPathsOpt.add_options()
            ("compiler-bin", po::value<string>(&config.m_cmplrBinPath), CLI_DESC_ALTERNATIVE_VK_BIN_FOLDER);

        // "Hidden" options.
        po::options_description hiddenOpt("Options that we don't show with --help.");
        hiddenOpt.add_options()
            ("?", "Produce help message.")
            ("input", po::value<std::vector<string>>(&config.m_InputFiles), "Input for analysis.")
            ("version-info", "Generate RGA CLI version info file. If no file name is provided, the version info is printed to stdout.")
            ("session-metadata", po::value<string>(&config.m_sessionMetadataFile), "Generate session metadata file with the list of output files generated by RGA.")
            ("log", po::value<string>(&config.m_logFile), "Path to the CLI log file")
            ;

        // all options available from command line
        po::options_description allOpt;
        allOpt.add(genericOpt).add(macroAndIncludeOpt).add(clOpt).add(legacyClOpt).add(hiddenOpt).add(dxOpt).add(dx12Opt).add(pipelinedOpt)
              .add(ilDumpOpt).add(lineNumsOpt).add(warningsOpt).add(optLevelOpt2).add(rocmCmplrPathsOpt);

#ifdef RGA_ENABLE_VULKAN
        allOpt.add(vulkanOpt).add(vulkanInputTypeOpt).add(vulkanOptHidden);
#endif

        po::variables_map vm;

        po::positional_options_description positionalOpt;
        positionalOpt.add("input", -1);

        // Parse command line
        store(po::command_line_parser(argc, argv).
            options(allOpt).positional(positionalOpt).run(), vm);

        // Handle Options
        notify(vm);

        if (vm.count("help") || vm.count("?") || vm.size() == 0)
        {
            config.m_RequestedCommand = Config::ccHelp;
        }

        if (vm.count("list-asics"))
        {
            config.m_RequestedCommand = Config::ccListAsics;
        }
        else if (vm.count("list-kernels") || vm.count("list-entries"))
        {
            config.m_RequestedCommand = Config::ccListEntries;
        }
        else if (vm.count("gpso-template"))
        {
            config.m_RequestedCommand = Config::ccGenTemplateFile;
        }


        if (vm.count("intrinsics"))
        {
            config.m_EnableShaderIntrinsics = true;
        }

        if (vm.count("dxbc"))
        {
            config.m_DxbcInputDx11 = true;
        }

        if (vm.count("adapters"))
        {
            config.m_RequestedCommand = Config::ccListAdapters;
        }

        if (vm.count("retain-user-filename"))
        {
            config.m_isRetainUserBinaryPath = true;
        }

        if (vm.count("parse-isa"))
        {
            config.m_isParsedISARequired = true;
        }

        if (vm.count("line-numbers"))
        {
            config.m_isLineNumbersRequired = true;
        }

        if (vm.count("warnings"))
        {
            config.m_isWarningsRequired = true;
        }

        if (vm.count("verbose"))
        {
            config.m_printProcessCmdLines = true;
        }

        if (vm.count("cfg") && vm.count("cfg-i"))
        {
            std::cerr << STR_ERR_BOTH_CFG_AND_CFGI_SPECIFIED << std::endl;
            doWork = false;
        }

        // Set the optimization level.
        if (vm.count("O0"))
        {
            config.m_optLevel = 0;
        }
        else if (vm.count("O1"))
        {
            config.m_optLevel = 1;
        }
        else if (vm.count("O2"))
        {
            config.m_optLevel = 2;
        }
        else if (vm.count("O3"))
        {
            config.m_optLevel = 3;
        }

        // Set the default livereg output file name if not provided by a user.
        if (vm.count("--livereg") && config.m_LiveRegisterAnalysisFile.empty())
        {
            config.m_LiveRegisterAnalysisFile = KC_STR_DEFAULT_LIVEREG_OUTPUT_FILE_NAME;
        }

        if (vm.count("version-info"))
        {
            config.m_versionInfoFile = (vm.count("input") ? config.m_InputFiles[0] : "");
            config.m_RequestedCommand = Config::ccGenVersionInfoFile;
        }
        else if (vm.count("version"))
        {
            config.m_RequestedCommand = Config::ccVersion;
        }
        else if (vm.count("updates"))
        {
            config.m_RequestedCommand = Config::ccUpdate;
        }
        else if (!config.m_AnalysisFile.empty() || !config.m_ILFile.empty() || !config.m_ISAFile.empty() ||
                 !config.m_LiveRegisterAnalysisFile.empty() || !config.m_BinaryOutputFile.empty() ||
                 !config.m_MetadataFile.empty() || !config.m_blockCFGFile.empty() ||
                 !config.m_instCFGFile.empty() || !config.m_spvTxt.empty() || !config.m_spvBin.empty() ||
                 !config.m_parsedSpv.empty())
        {
            config.m_RequestedCommand = Config::ccCompile;
        }

        if (config.m_RequestedCommand == Config::ccNone)
        {
            std::cout << STR_ERR_NO_VALID_CMD_DETECTED << std::endl;
        }

        // If the "--validation" option for Vulkan mode is specified, the validation info should be dumped to stdout.
        if (validation && vm.count("validation-file") == 0)
        {
            config.m_vulkanValidation = KC_STR_VK_VALIDATION_INFO_STDOUT;
        }

        // Select the mode (source language).
        bool bSourceSpecified = true;
        std::string srcKind = config.m_SourceKind;
        std::transform(srcKind.begin(), srcKind.end(), srcKind.begin(), [](unsigned char c){return std::toupper(c);});

        if (config.m_SourceKind.empty())
        {
            if (config.m_RequestedCommand != Config::ccHelp &&
                config.m_RequestedCommand != Config::ccVersion &&
                config.m_RequestedCommand != Config::ccUpdate &&
                config.m_RequestedCommand != Config::ccGenVersionInfoFile)
            {
                std::cout << STR_ERR_NO_MODE_SPECIFIED << std::endl;
            }
            bSourceSpecified = false;
        }
        else if (srcKind == Config::sourceKindDx11)
        {
            config.m_mode = Mode_DX11;
        }
        else if (srcKind == Config::sourceKindDx12)
        {
            config.m_mode = Mode_DX12;
        }
        else if (srcKind == Config::sourceKindAMDIL)
        {
            config.m_mode = Mode_AMDIL;
        }
        else if (srcKind == Config::sourceKindOpenCL)
        {
            config.m_mode = Mode_OpenCL;
        }
        else if (srcKind == Config::sourceKindOpenGL)
        {
            config.m_mode = Mode_OpenGL;
        }
        else if (srcKind == Config::sourceKindGLSLVulkanOffline)
        {
            config.m_mode = Mode_Vk_Offline;
        }
        else if (srcKind == Config::sourceKindSpirvBinOffline)
        {
            config.m_mode = Mode_Vk_Offline_Spv;
        }
        else if (srcKind == Config::sourceKindSpirvTxtOffline)
        {
            config.m_mode = Mode_Vk_Offline_SpvTxt;
        }
#ifdef RGA_ENABLE_VULKAN
        else if (srcKind == Config::sourceKindVulkan)
        {
            config.m_mode = Mode_Vulkan;
        }
#endif
        else if (srcKind == Config::sourceKindRocmOpenCL)
        {
            config.m_mode = Mode_Rocm_OpenCL;
        }
        else
        {
            config.m_mode = Mode_Invalid;
            cout << "Source language: " << config.m_SourceKind << " not supported.\n";
        }

#ifdef RGA_ENABLE_VULKAN
        // Set the Vulkan per-stage input file types.
        if (config.m_mode == Mode_Vulkan)
        {
            for (const auto& stage : std::map<std::string, std::pair<std::string*, rgVulkanInputType*>>
                    { {"vert", {&config.m_VertexShader, &config.m_vertShaderFileType}},         {"tesc", {&config.m_TessControlShader, &config.m_tescShaderFileType}},
                      {"tese", {&config.m_TessEvaluationShader, &config.m_teseShaderFileType}}, {"frag", {&config.m_FragmentShader, &config.m_fragShaderFileType}},
                      {"geom", {&config.m_GeometryShader, &config.m_geomShaderFileType}},       {"comp", {&config.m_ComputeShader, &config.m_compShaderFileType}} })
            {
                for (const std::pair<std::string, rgVulkanInputType>& fileType :
                         std::map<std::string, rgVulkanInputType>{ {"glsl", rgVulkanInputType::Glsl} })
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
        const string  programName = "rga.exe";
#else
        const string  programName = "rga";
#endif

        cout << std::endl;

        if ((config.m_RequestedCommand == Config::ccHelp) && (!bSourceSpecified))
        {
            std::string productVersion;

            cout << STR_RGA_PRODUCT_NAME << " " << STR_RGA_VERSION_PREFIX << STR_RGA_VERSION << "." << STR_RGA_BUILD_NUM << std::endl;
            cout << STR_RGA_PRODUCT_NAME << " is a compiler and code analysis tool for OpenCL";
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
            cout << "To view help for AMDIL mode: -h -s amdil" << std::endl;
#endif
            cout << "To view help for OpenGL mode: -h -s opengl" << endl;
            cout << "To view help for ROCm OpenCL mode: -h -s rocm-cl" << std::endl;
            cout << "To view help for legacy OpenCL mode: -h -s cl" << std::endl;
            cout << std::endl;
            cout << "To see the current RGA version: --version" << std::endl;
            cout << "To check for available updates: --updates" << std::endl;
        }
        else if ((config.m_RequestedCommand == Config::ccHelp) && (config.m_mode == Mode_OpenCL))
        {
            // Put all options valid for this mode to one group to make the description aligned.
            po::options_description  oclOptions;
            oclOptions.add(genericOpt).add(ilDumpOpt).add(macroAndIncludeOpt).add(clOpt).add(legacyClOpt);
            cout << "*** Legacy OpenCL mode options ***" << endl;
            cout << "==================================" << endl;
            cout << oclOptions << endl;
            cout << "Examples:" << endl;
            cout << "  " << "Compile foo.cl for all supported devices; extract ISA, IL code and statistics:" << endl;
            cout << "    " << programName << " -s cl --isa output/foo_isa.txt --il output/foo_il.txt -a output/stats.csv foo.cl" << endl;
            cout << "  " << "Compile foo.cl for Fiji; extract ISA and perform live register analysis:" << endl;
            cout << "    " << programName << " -s cl -c Fiji --isa output/foo_isa.txt --livereg output/regs.txt foo.cl" << endl;
            cout << "  " << "Compile foo.cl for gfx906; extract binary and control flow graphs:" << endl;
            cout << "    " << programName << " -s cl -c gfx906 --bin output/foo.bin --cfg output/cfg.dot foo.cl" << endl;
            cout << "  " << "List the kernels available in foo.cl:" << endl;
            cout << "    " << programName << " -s cl --list-kernels foo.cl" << endl;
            cout << "  " << "Compile foo.cl for Bonaire; extract the hardware resource usage statistics for myKernel.  Write the statistics to foo.csv:" << endl;
            cout << "    " << programName << " -s cl -c \"Bonaire\" --kernel myKernel -a foo.csv foo.cl" << endl;
            cout << "  " << "List the ASICs supported by Legacy OpenCL mode:" << endl;
            cout << "    " << programName << " -s cl --list-asics" << endl;
            cout << endl;
        }
        else if ((config.m_RequestedCommand == Config::ccHelp) && (config.m_mode == Mode_Rocm_OpenCL))
        {
            // Put all options valid for this mode to one group to make the description aligned.
            po::options_description  oclOptions;
            oclOptions.add(genericOpt).add(ilDumpOpt).add(warningsOpt).add(macroAndIncludeOpt).add(clOpt).add(lineNumsOpt).add(optLevelOpt2).add(rocmCmplrPathsOpt);
            cout << "*** ROCm OpenCL mode options ***" << endl;
            cout << "================================" << endl << endl;
            cout << "Usage: " << programName << " [options] source_file(s)" << endl;
            cout << oclOptions << endl;
            cout << "Note: In case that your alternative compiler supports targets which are not known to the RGA build that you are using, "
                    "use the x64/ROCm/additional-targets text file, which contains a list of targets that would be dynamically loaded by RGA. "
                    "To add targets, simply list them in a new line in the file, and RGA would load them while running." << endl << endl;

            cout << "Examples:" << endl;
            cout << "  " << "Compile test.cl for Vega Frontier and extract the binary:" << endl;
            cout << "    " << programName << " -s rocm-cl -c \"vega frontier\" -b output/test.bin test.cl" << endl;
            cout << "  " << "Compile and link src1.cl, src2.cl and src3.cl into an HSA Code Object for Vega (gfx900), and extract ISA disassembly:" << endl;
            cout << "    " << programName << " -s rocm-cl -c gfx900 --isa output/isa.txt src1.cl src2.cl src3.cl" << endl;
            cout << "  " << "Compile test.cl for all supported targets, extract ISA and perform live register analysis:" << endl;
            cout << "    " << programName << " -s rocm-cl --isa test_isa.txt --livereg regs.txt test.cl" << endl;
            cout << "  " << "List the kernels available in test.cl:" << endl;
            cout << "    " << programName << " -s rocm-cl --list-kernels test.cl" << endl;
            cout << "  " << "List the ASICs supported by ROCm OpenCL Lightning Compiler:" << endl;
            cout << "    " << programName << " -s rocm-cl --list-asics" << endl;
            cout << "  " << "Compile test.cl and extract ISA disassembly for gfx900 using alternative OpenCL compiler:" << endl;
            cout << "    " << programName << " -s rocm-cl -c gfx900 --isa test.isa --compiler-bin C:\\llvm\\dist\\bin --compiler-inc C:\\llvm\\dist\\include --compiler-lib C:\\llvm\\dist\\lib\\bitcode  test.cl" << endl;

            cout << endl;
        }
        else if ((config.m_RequestedCommand == Config::ccHelp) && (config.m_mode == Mode_DX11))
        {
            cout << "*** DirectX 11 mode options (Windows Only) ***" << endl;
            cout << "==============================================" << endl << endl;
            cout << "Usage: " << programName << " [options] source_file" << endl << endl;
            cout << genericOpt << endl;
            cout << ilDumpOpt << endl;
            cout << macroAndIncludeOpt << endl;
            cout << dxOpt << endl;
            cout << "Examples:" << endl;
            cout << "  View supported ASICS for DX11:" << endl;
            cout << "    " << programName << " -s dx11 -l" << endl;
            cout << "  Compile myShader.hlsl for all supported targets and extract the ISA disassembly:" << endl;
            cout << "    " << programName << " -s dx11 -f VsMain -p vs_5_0 --isa output/myShader_isa.txt src/myShader.hlsl" << endl;
            cout << "  Compile myShader.hlsl for Fiji; extract the ISA and perform live register analysis:" << endl;
            cout << "    " << programName << " -s dx11 -c Fiji -f VsMain -p vs_5_0 --isa output/myShader_isa.txt --livereg output/regs.txt myShader.hlsl" << endl;
            cout << "  Compile myShader.hlsl for Radeon R9 390; perform static analysis and save the statistics to myShader.csv:" << endl;
            cout << "    " << programName << " -s dx11 -c r9-390 -f VsMain -p vs_5_0 -a output/myShader.csv shaders/myShader.hlsl" << endl;
        }
        else if ((config.m_RequestedCommand == Config::ccHelp) && (config.m_mode == Mode_DX12))
        {
        cout << "*** DirectX 12 mode options (Windows only) ***" << endl;
        cout << "==============================================" << endl << endl;
        cout << "Usage: " << programName << " [options]" << endl << endl;
        cout << genericOpt << endl;
        cout << macroAndIncludeOpt << endl;
        cout << dx12Opt << endl;
        cout << "Examples:" << endl;
        cout << "  View supported ASICS for DX12:" << endl;
        cout << "    " << programName << " -s dx12 -l" << endl;
        cout << "  Compile a cs_5_1 compute shader named \"CSMain\" for gfx1010, where the root signature is referenced through a [RootSignature()] attribute. Generate ISA disassembly and resource usage statistics:" << endl;
        cout << "    " << programName << " -s dx12 -c gfx1010 --cs C:\\shaders\\FillLightGridCS_8.hlsl -cs-entry CSMain --cs-model cs_5_1 --isa C:\\output\\isa.txt -a C:\\output\\stats.txt" << endl;
        cout << "  Compile a graphics pipeline with model 6.0 vertex (\"VSMain\") and pixel (\"PSMain\") shaders defined in vert.hlsl and pixel.hlsl respectively, while the root signature is in a pre-compiled binary file. Generate ISA disassembly and resource usage statistics:" << endl;
        cout << "    " << programName << " -s dx12 --vs C:\\shaders\\vert.hlsl --vs-model vs_6_0 --vs-entry VSMain --ps C:\\shaders\\pixel.hlsl --ps-model ps_6_0 --ps-entry PSMain --gpso C:\\shaders\\state.gpso --rs-bin C:\\rootSignatures\\rs.bin --isa C:\\output\\disassembly.txt -a C:\\output\\stats.txt " << endl;
        cout << "  Compile a graphics pipeline with vertex (\"VSMain\") and pixel (\"PSMain\") shaders defined both in shader.hlsl, while the root signature is in a pre-compiled binary file. Generate ISA disassembly and resource usage statistics:" << endl;
        cout << "    " << programName << " -s dx12 --all-hlsl C:\\shaders\\shaders.hlsl --all-model 6_0 --vs-entry VSMain --ps-entry PSMain --gpso C:\\shaders\\state.gpso --rs-bin C:\\rootSignatures\\rs.bin --isa C:\\output\\disassembly.txt -a C:\\output\\stats.txt" << endl;
        cout << "  Compile a cs_5_1 compute shader named \"CSMain\" for Radeon VII (gfx906), where the root signature is in a binary file (C:\\RS\\FillLightRS.rs.fxo). Generate ISA disassembly and resource usage statistics:" << endl;
        cout << "    " << programName << " -s dx12 -c gfx906 --cs C:\\shaders\\FillLightGridCS_8.hlsl --cs-model cs_5_1 --cs-entry main --rs-bin C:\\RS\\FillLightRS.rs.fxo --isa C:\\output\\lightcs_dis.txt -a C:\\output\\stats.txt" << endl;
        cout << "  Compile a DXIL or DXBC blob for Navi10 (gfx1010) and generate ISA disassembly:" << endl;
        cout << "    " << programName << " -s dx12 -c gfx1010 --cs-blob C:\\shaders\\FillLightGridCS_8.obj --isa C:\\output\\lightcs_dis.txt" << endl;
        }
        else if ((config.m_RequestedCommand == Config::ccHelp) && (config.m_mode == Mode_AMDIL))
        {
            cout << "*** AMDIL mode options (Windows only) ***" << endl;
            cout << "=========================================" << endl << endl;
            cout << "Usage: " << programName << " [options] source_file" << endl << endl;
            cout << genericOpt << endl;
            cout << "Examples:" << endl;
            cout << "  Generate ISA from AMDIL code for all supported targets:" << endl;
            cout << "    " << programName << " -s amdil --isa output/isaFromAmdil.isa myAmdilCode.amdil" << endl;
            cout << "  Generate ISA for Fiji from AMDIL code and extract statistics:" << endl;
            cout << "    " << programName << " -s amdil -c Fiji --isa output/isaFromAmdil.isa -a output/statsFromAmdil.csv myAmdilCode.amdil" << endl;
            cout << "  Generate ISA for gfx900 from AMDIL code and perform live register analysis:" << endl;
            cout << "    " << programName << " -s amdil -c gfx900 --isa output/myShader.isa --livereg output/regs.txt myAmdilCode.amdil" << endl;
        }
        else if ((config.m_RequestedCommand == Config::ccHelp) && (config.m_mode == Mode_Vk_Offline ||
                  config.m_mode == Mode_Vk_Offline_Spv || config.m_mode == Mode_Vk_Offline_SpvTxt))
        {
            // Get the mode name, and the relevant file extensions. We will use it when constructing the example string.
            const char* FILE_EXT_SPV = "spv";
            const char* FILE_EXT_SPV_TXT = "txt";
            std::string rgaModeName;
            std::string vertExt;
            std::string fragExt;
            switch (config.m_mode)
            {
            case Mode_Vk_Offline:
                rgaModeName = Config::sourceKindGLSLVulkanOffline;
                vertExt = "vert";
                fragExt = "frag";
                break;
            case Mode_Vk_Offline_Spv:
                rgaModeName = Config::sourceKindSpirvBinOffline;
                vertExt = FILE_EXT_SPV;
                fragExt = FILE_EXT_SPV;
                break;
            case Mode_Vk_Offline_SpvTxt:
                rgaModeName = Config::sourceKindSpirvTxtOffline;
                vertExt = FILE_EXT_SPV_TXT;
                fragExt = FILE_EXT_SPV_TXT;
                break;
            }

            // Convert the mode name string to lower case for presentation.
            std::transform(rgaModeName.begin(), rgaModeName.end(), rgaModeName.begin(), ::tolower);

            cout << "*** Vulkan Offline mode options ***" << endl;
            cout << "===================================" << endl << endl;
            cout << "The Vulkan offline mode is independent of the installed driver and may not provide assembly code and resource"
                    " usage that reflect the real-life case. To get results that reflect the real-life performance of your code, "
                    "please use RGA's Vulkan live-driver mode (-s vulkan)." << endl << endl;
            cout << "Usage: " << programName << " [options]";
            if (config.m_mode == Mode_Vk_Offline_Spv || config.m_mode == Mode_Vk_Offline_SpvTxt)
            {
                cout << " [optional: spv_input_file]";
            }
            cout << endl << endl;
            if (config.m_mode == Mode_Vk_Offline_Spv || config.m_mode == Mode_Vk_Offline_SpvTxt)
            {
                cout << "Notes:" << endl;
                cout << " * The input file(s) must be specified in one of two ways:" << endl <<
                    "   1) A single SPIR-V input file provided as \"spv_input_file\", or\n" <<
                    "   2) One or more pipeline stage specific shader files specified by the pipeline stage options (--vert, --tesc, etc.)." << endl << endl;
            }
            cout << genericOpt << endl;
            cout << optLevelOpt1 << endl;
            cout << pipelinedOpt << endl;
            cout << "Examples:" << endl;
            cout << "  Compile vertex & fragment shaders for all supported devicesl; extract ISA, AMD IL and statistics:" << endl;
            cout << "    " << programName << " -s " << rgaModeName << " --isa output/isa.txt --il output/il.txt -a output/stats.csv --vert source/myVertexShader." << vertExt << " --frag source/myFragmentShader." << fragExt << endl;
            cout << "  Compile vertex & fragment shaders for Iceland and Fiji; extract ISA, AMD IL and statistics:" << endl;
            cout << "    " << programName << " -s " << rgaModeName << " -c Iceland -c Fiji --isa output/isa.txt --il output/il.amdil -a output/.csv --vert source/myVertexShader." << vertExt << " --frag source/myFragmentShader." << fragExt << endl;
            cout << "  Compile vertex shader for Radeon R9 390; extract ISA and binaries:" << endl;
            cout << "    " << programName << " -s " << rgaModeName << " -c \"R9 390\" --isa output/isa.txt -b output/binary.bin -a output/stats.csv --vert c:\\source\\myVertexShader." << vertExt << endl;
            if (config.m_mode == Mode_Vk_Offline_Spv || config.m_mode == Mode_Vk_Offline_SpvTxt)
            {
                cout << "  Extract ISA for a single SPIR-V file for Baffin, without specifying the pipeline stages:" << endl;
                cout << "    " << programName << " -s " << rgaModeName << " -c Baffin --isa output/program_isa.txt source/program.spv" << endl;
            }
        }
        else if ((config.m_RequestedCommand == Config::ccHelp) && (config.m_mode == Mode_OpenGL))
        {
            cout << "*** OpenGL mode options ***" << endl;
            cout << "===========================" << endl << endl;
            cout << "Usage: " << programName << " [options]" << endl << endl;
            cout << genericOpt << endl;
            cout << ilDumpOpt << endl;
            cout << pipelinedOpt << endl;
            cout << "Examples:" << endl;
            cout << "  Compile fragment shader for Baffin; extract ISA, binaries and statistics:" << endl;
            cout << "    " << programName << " -s opengl --isa output/opengl_isa.txt -b output/opengl_bin.bin -a output/opengl_stats.csv --frag source/myFragmentShader.frag" << endl;
            cout << "  Compile vertex & fragment shaders for FirePro W7100; Extract ISA and control flow graph: " << endl;
            cout << "    " << programName << " -s opengl -c W7100 --isa output/opengl_isa.txt --cfg output/cfg.dot --vert myVertexShader.vert --frag cmyFragmentShader.frag" << endl;
            cout << "  Compile geometry shader for all supported devices; extract ISA and perform live register analysis:" << endl;
            cout << "    " << programName << " -s opengl --isa output/opengl_isa.txt --livereg output/regs.txt --geom source/myVertexShader.geom" << endl;
        }
#ifdef RGA_ENABLE_VULKAN
        else if ((config.m_RequestedCommand == Config::ccHelp) && (config.m_mode == Mode_Vulkan))
        {
            cout << "*** Vulkan mode options ***" << endl;
            cout << "===========================" << endl << endl;
            cout << "Usage: " << programName << " [options]" << endl << endl;
            cout << genericOpt << endl;
            cout << macroAndIncludeOpt << endl;
            cout << pipelinedOpt << endl;

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
            cout << vulkanOpt << endl;
            cout << vkCmplrPathsOpt << endl;
        }
#endif
        else if ((config.m_RequestedCommand == Config::ccUpdate))
        {
            kcUtils::CheckForUpdates();
        }
    }
    catch (exception& e)
    {
        // Problem parsing options - report and exit
        std::string sExceptionMsg = e.what();

        // here I replace the blat error msg with a more informative one. BUG429870
        if (sExceptionMsg == "multiple occurrences")
        {
            sExceptionMsg = "Error: Problem parsing arguments. Please check if a non-repeatable argument was used more then once and that all file paths are correct. Note that if a path contain a space, a \"\" should be used.";
        }

        cout << sExceptionMsg << "\n";
        doWork = false;
    }

    if (!doWork)
    {
        config.m_RequestedCommand = Config::ccInvalid;
    }

    return doWork;
}
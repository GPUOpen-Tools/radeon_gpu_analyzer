//=================================================================
// Copyright 2017 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

#include <iostream>
#include <string>
#include <algorithm>
#ifdef _WIN32
    #pragma warning (push, 3) // disable warning level 4 for boost
#endif
#include <boost/filesystem/path.hpp>
#include <boost/program_options.hpp>
#include <boost/algorithm/string/predicate.hpp>

#ifdef _WIN32
    #pragma warning (pop)
#endif

#ifdef CMAKE_BUILD
#include <RadeonGPUAnalyzerCLIConfig.h>
#endif

#include <RadeonGPUAnalyzerCLI/src/kcConfig.h>
#include <RadeonGPUAnalyzerCLI/src/kcParseCmdLine.h>
#include <RadeonGPUAnalyzerCLI/src/kcCliStringConstants.h>
#include <VersionInfo/VersionInfo.h>

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

    try
    {
        // Parse command line options using Boost library

        // Generic Options: Valid got CL, DX, GL
        po::options_description genericOpt("Generic options");

        genericOpt.add_options()
        // The next two options here can be repeated, so they are vectors.
        ("csv-separator", po::value< string >(&config.m_CSVSeparator), "Override to default separator for analysis items.")
        ("list-asics,l", "List the known GPU codenames, architecture names and variant names.\nTo target a specific GPU, use its codename as the argument to the \"-c\" command line switch.")
        ("asic,c", po::value< vector<string> >(&config.m_ASICs), "Which ASIC to target.  Repeatable.")
        ("version", "Print version string.")
        ("help,h", "Produce this help message.")
        ("analysis,a", po::value<string>(&config.m_AnalysisFile), "Path to output analysis file.")
        ("isa", po::value<string>(&config.m_ISAFile), "Path to output ISA disassembly file(s).")
        ("livereg", po::value<string>(&config.m_LiveRegisterAnalysisFile), "Path to live register analysis output file(s).")
        ("cfg", po::value<string>(&config.m_ControlFlowGraphFile), "Path to control flow graph output file(s).")
        ("il", po::value<string>(&config.m_ILFile), "Path to output IL file(s).")
        ("source-kind,s", po::value<string>(&config.m_SourceKind), "Source platform: cl for OpenCL, hlsl for DirectX, opengl for OpenGL, vulkan for Vulkan and amdil for AMDIL; cl is set by default")
        ;

        // DX Options
        std::string  adaptersDesc = "List all of the supported display adapters that are installed on the system.\n" + std::string(STR_DX_ADAPTERS_HELP_COMMON_TEXT);
        std::string  setAdaptDesc = "Specify the id of the display adapter whose driver you would like RGA to use.\n" + std::string(STR_DX_ADAPTERS_HELP_COMMON_TEXT);

        po::options_description dxOpt("DirectX Shader Analyzer options");
        dxOpt.add_options()
        ("function,f", po::value<string>(&config.m_Function), "D3D shader to compile, DX ASM shader.")
        ("profile,p", po::value<string>(&config.m_Profile), "Profile to use for compilation.  REQUIRED.\nFor example: vs_5_0, ps_5_0, etc.")
        ("DXFlags", po::value<unsigned int>(&config.m_DXFlags), "Flags to pass to D3DCompile.")
        ("DXLocation", po::value<string>(&config.m_DXLocation), "Location to the D3DCompiler Dll required for compilation. If none is specified, the default D3D compiler that is bundled with the Analyzer will be used.")
        ("FXC", po::value<string>(&config.m_FXC), "FXC Command Line. Use full path and specify all arguments in \"\". For example:\n"
            "   rga.exe  -f VsMain1 -s DXAsm -p vs_5_0 <Path>\\vsBlob.obj  --isa <Path>\\vsTest.isa --FXC \"<Path>\\fxc.exe /E VsMain1 /T vs_5_0 /Fo <Path>\\vsBlob.obj <Path>\\vsTest.fx\"\n "
            "   In order to use it, DXAsm must be specified. /Fo switch must be used and output file must be the same as the input file for rga.")
        ("DumpMSIntermediate", po::value<string>(&config.m_DumpMSIntermediate), "Location to save the MS Blob as text. ")
        ("intrinsics", "Enable AMD D3D11 Shader Intrinsics extension.")
        ("adapters", adaptersDesc.c_str())
        ("set-adapter",    po::value<int>(&config.m_DXAdapter), setAdaptDesc.c_str())
        ("UAVSlot", po::value<int>(&config.m_UAVSlot), "This value should be in the range of [0,63].\nThe driver will use the slot to track which UAV is being used to specify the intrinsic. The UAV slot that is selected cannot be used for any other purposes.\nThis option is only relevant when AMD D3D11 Shader Intrinsics is enabled (specify --intrinsics).")
        ;

        po::options_description macroAndIncludeOpt("Macro and Include paths Options");
        macroAndIncludeOpt.add_options()
        // The next two options here can be repeated, so they are vectors.
        ("define,D", po::value< vector<string> >(&config.m_Defines), "Define symbol or symbol=value. Applicable only to CL and DX files. Repeatable.")
        ("IncludePath,I", po::value< vector<string> >(&config.m_IncludePath), "Additional include path required for compilation.  Repeatable.")
        ;

        // CL Option
        po::options_description clOpt("rga options");
        clOpt.add_options()
        ("list-kernels",                                                       "List kernel functions.")
        ("debugil",      po::value<string>(&config.m_DebugILFile),             "Path to output Debug IL file(s).")
        ("metadata",     po::value<string>(&config.m_MetadataFile),            "Path to output Metadata file(s).\n"
         "Requires --" KERNEL_OPTION ".")
        ("kernel,k",     po::value<string>(&config.m_Function),                "Kernel to be compiled and analyzed. If not specified, all kernels will be targeted.\n")
        ("binary,b",     po::value<string>(&config.m_BinaryOutputFile),        "Path to binary output file(s).")
        ("retain-user-filename",                                               "Retain the output path and name for the generated binary file as specified without adding the target asic name.")
        ("suppress",     po::value<vector<string> >(&config.m_SuppressSection), "Section to omit from binary output.  Repeatable. Available options: .source, .amdil, .debugil, .debug_info, .debug_abbrev, .debug_line, .debug_pubnames, .debug_pubtypes, .debug_loc, .debug_str, .llvmir, .text\nNote: Debug sections are valid only with \"-g\" compile option")
        ("OpenCLoption", po::value< vector<string> >(&config.m_OpenCLOptions), "OpenCL compiler options.  Repeatable.");

        // Vulkan-specific.
        po::options_description pipelinedOpt("");
        pipelinedOpt.add_options()
        ("vert", po::value<string>(&config.m_VertexShader), "Full path to vertex shader source file.")
        ("tesc", po::value<string>(&config.m_TessControlShader), "Full path to tessellation control shader source file.")
        ("tese", po::value<string>(&config.m_TessEvaluationShader), "Full path to tessellation evaluation shader source file.")
        ("geom", po::value<string>(&config.m_GeometryShader), "Full path to geometry shader source file.")
        ("frag", po::value<string>(&config.m_FragmentShader), "Full path to fragment shader source file")
        ("comp", po::value<string>(&config.m_ComputeShader), "Full path to compute shader source file.");


        po::options_description hiddenOpt("Options that we don't show with --help.");
        hiddenOpt.add_options()
        ("?",                                                 "Produce help message.")
        ("input",     po::value<string>(&config.m_InputFile), "Source program for analysis.");

        // all options available from command line
        po::options_description allOpt;
        allOpt.add(genericOpt).add(macroAndIncludeOpt).add(clOpt).add(hiddenOpt).add(dxOpt).add(pipelinedOpt);

        po::variables_map vm;

        po::positional_options_description positionalOpt;
        positionalOpt.add("input", -1);

        // Parse command line
        store(po::command_line_parser(argc, argv).
              options(allOpt).positional(positionalOpt).run(), vm);

        // Handle Options
        notify(vm);

        if (vm.count("help") || vm.count("?"))
        {
            config.m_RequestedCommand = Config::ccHelp;
            //doWork = false;
        }

        if (vm.count("list-asics"))
        {
            config.m_RequestedCommand = Config::ccListAsics;
        }

        if (vm.count("intrinsics"))
        {
            config.m_EnableShaderIntrinsics = true;
        }

        if (vm.count("adapters"))
        {
            config.m_RequestedCommand = Config::ccListAdapters;
        }

        if (vm.count("retain-user-filename"))
        {
            config.m_isRetainUserBinaryPath = true;
        }

        // Set the default livereg output file name if not provided by a user.
        if (vm.count("--livereg") && config.m_LiveRegisterAnalysisFile.empty())
        {
            config.m_LiveRegisterAnalysisFile = KC_STR_DEFAULT_LIVEREG_OUTPUT_FILE_NAME;
        }

        if (vm.count("list-kernels"))
        {
            config.m_RequestedCommand = Config::ccListKernels;

        }
        else if (vm.count("version"))
        {
            config.m_RequestedCommand = Config::ccVersion;
        }

        else if (config.m_AnalysisFile.size() > 0)
        {
            config.m_RequestedCommand = Config::ccCompile;
        }

        else if (config.m_ILFile.size() > 0)
        {
            config.m_RequestedCommand = Config::ccCompile;
        }
        else if (config.m_ISAFile.size() > 0)
        {
            config.m_RequestedCommand = Config::ccCompile;
        }
        else if (config.m_LiveRegisterAnalysisFile.size() > 0)
        {
            config.m_RequestedCommand = Config::ccCompile;
        }
        else if (config.m_BinaryOutputFile.size() > 0)
        {
            config.m_RequestedCommand = Config::ccCompile;
        }
        else if (config.m_DebugILFile.size() > 0)
        {
            config.m_RequestedCommand = Config::ccCompile;
        }
        else if (config.m_MetadataFile.size() > 0)
        {
            config.m_RequestedCommand = Config::ccCompile;
        }

        // what is the right way to do it?
        // Danana- todo: need to decide how to know. not all commands requires file (like -l)
        bool bSourceSpecified = false;

        if (config.m_SourceKind.length() == 0) // set the default and remember it for the help
        {
            config.m_SourceKind = Config::sourceKindOpenCL;
            bSourceSpecified = true;
        }

        if (boost::iequals(config.m_SourceKind, Config::sourceKindHLSL))
        {
            config.m_SourceLanguage = SourceLanguage_HLSL;
        }
        else if (boost::iequals(config.m_SourceKind, Config::sourceKindAMDIL))
        {
            config.m_SourceLanguage = SourceLanguage_AMDIL;
        }
        else if (boost::iequals(config.m_SourceKind, Config::sourceKindDXAsm))
        {
            config.m_SourceLanguage = SourceLanguage_DXasm;
        }
        else if (boost::iequals(config.m_SourceKind, Config::sourceKindDXAsmT))
        {
            config.m_SourceLanguage = SourceLanguage_DXasmT;
        }
        else if ((boost::iequals(config.m_SourceKind, Config::sourceKindOpenCL)))
        {
            config.m_SourceLanguage = SourceLanguage_OpenCL;
        }
        else if ((boost::iequals(config.m_SourceKind, Config::sourceKindGLSL)))
        {
            config.m_SourceLanguage = SourceLanguage_GLSL;
        }
        else if ((boost::iequals(config.m_SourceKind, Config::sourceKindOpenGL)))
        {
            config.m_SourceLanguage = SourceLanguage_GLSL_OpenGL;
        }
        else if ((boost::iequals(config.m_SourceKind, Config::sourceKindGLSLVulkan)))
        {
            config.m_SourceLanguage = SourceLanguage_GLSL_Vulkan;
        }
        else if ((boost::iequals(config.m_SourceKind, Config::sourceKindSpirvBin)))
        {
            config.m_SourceLanguage = SourceLanguage_SPIRV_Vulkan;
        }
        else if ((boost::iequals(config.m_SourceKind, Config::sourceKindSpirvTxt)))
        {
            config.m_SourceLanguage = SourceLanguage_SPIRVTXT_Vulkan;
        }
        else
        {
            config.m_SourceLanguage = SourceLanguage_Invalid;
            cout << "Source language: " <<  config.m_SourceKind << " not supported.\n";

        }


        // Require an input file.
        if (doWork && (vm.count("input") == 0 && config.m_RequestedCommand == Config::ccInvalid))
        {
            // TODO: get just the program name here.
            // Maybe use boost file system stuff to just get KernelAnalyzerCLI.
            //           cout << "Usage: " << argv[0] << " [options] source_file" << endl;
            //           cout << visibleOpt << "\n";
            doWork = false;
        }

        // handle the help. I do it here because we need the visibleOpt.
        // TODO: change the function into class and make available.
        string programName(argv[0]);

        // On Linux platforms we use a script file that is what the user should call,
        // and a binary file that the script invokes that has a "-bin" suffix.
        // The binary file is the one that performs the commands so argv[0] equals the binary file name.
        // Force the help text to display the script file name by removing the "-bin" suffix
        // from sProgramName. This is applicable to Linux platforms only.
        string suffixToRemove("-bin");
        size_t pos = programName.rfind(suffixToRemove);

        if (pos != string::npos)
        {
            // Remove the suffix
            programName.erase(pos);
        }

        cout << std::endl;

        if ((config.m_RequestedCommand == Config::ccHelp) && (bSourceSpecified))
        {
            std::string productVersion;

            cout << STR_RGA_PRODUCT_NAME << " " << STR_RGA_VERSION_PREFIX << STR_RGA_VERSION << "." << STR_RGA_BUILD_NUM << std::endl;
            cout << STR_RGA_PRODUCT_NAME << " is an analysis tool for OpenCL";
#if _WIN32
            cout << ", DirectX";
#endif
            cout << ", OpenGL and Vulkan" << std::endl << std::endl;
            cout << "To view help for OpenCL: -h -s cl" << std::endl;
            cout << "To view help for OpenGL: -h -s opengl" << endl;
            cout << "To view help for Vulkan (GLSL): -h -s vulkan" << endl;
            cout << "To view help for Vulkan (SPIR-V binary input): -h -s vulkan-spv" << endl;
            cout << "To view help for Vulkan (SPIR-V textual input): -h -s vulkan-spv-txt" << endl;
#if _WIN32
            cout << "To view help for DirectX: -h -s hlsl" << std::endl;
            cout << "To view help for AMDIL: -h -s amdil" << std::endl;
#endif
        }
        else if ((config.m_RequestedCommand == Config::ccHelp) && (config.m_SourceLanguage == SourceLanguage_OpenCL))
        {
            cout << "*** OpenCL Instructions & Options ***" << endl;
            cout << "=====================================" << endl;
            cout << "Usage: " << programName << " [options] source_file" << endl;
            cout << genericOpt << endl;
            cout << macroAndIncludeOpt << endl;
            cout << clOpt << endl;
            cout << "Examples:" << endl;
            cout << "  " << "Extract ISA, IL code and statistics for all devices" << endl;
            cout << "    " << programName << " foo.cl --isa outdir/foo/myIsa.isa --il outdir/foo/myIl.il -a outdir/foo/stats.csv" << endl;
            cout << "  " << "Extract ISA and perform live register analysis for Fiji" << endl;
            cout << "    " << programName << " foo.cl -c Fiji --isa outdir/foo/myIsa.isa --livereg outdir/foo/" << endl;
            cout << "  " << "Create binary files output/foo-ASIC.bin for foo.cl and suppress the .source section" << endl;
            cout << "    " << programName << " foo.cl --bin outdir/foo --suppress .source" << endl;
            cout << "  " << "List the kernels available in foo.cl." << endl;
            cout << "    " << programName << " foo.cl --list-kernels" << endl;
            cout << "  " << "Produce analysis of myKernel in foo.cl.  Write the analysis to foo.csv." << endl;
            cout << "    " << programName << " foo.cl --kernel myKernel --analysis foo.csv" << endl;
            cout << "  " << "List the ASICs that the runtime supports." << endl;
            cout << "    " << programName << " --list-asics" << endl;
            cout << "  " << "Produce foo-Cypress.amdil and foo-Cypress.amdisa files for myKernel compiled for Cypress ASICs." << endl;
            cout << "    " << programName << " foo.cl --kernel myKernel --il foo --isa foo --asic Cypress" << endl;
            cout << endl;
        }
        else if ((config.m_RequestedCommand == Config::ccHelp) && (config.m_SourceLanguage == SourceLanguage_HLSL))
        {
            cout << "*** DX Instructions & Options (Windows Only) ***" << endl;
            cout << "================================================" << endl;
            cout << "Usage: " << programName << " [options] source_file" << endl;
            cout << genericOpt << endl;
            cout << macroAndIncludeOpt << endl;
            cout << dxOpt << endl;
            cout << "Examples:" << endl;
            cout << " View supported ASICS for DirectX: " << programName << " -s hlsl -l" << endl;
            cout << " Extract the ISA: " << programName << " -s hlsl -f VsMain -p vs_5_0 --isa c:\\files\\myShader.isa c:\\files\\myShader.hlsl" << endl;
            cout << " Extract the ISA and perform live register analysis: " << programName << " -s hlsl -f VsMain -p vs_5_0 --isa c:\\output\\myShader.isa --livereg c:\\output\\ c:\\files\\myShader.hlsl" << endl;
            cout << " Output analysis: " << programName << " -s hlsl -f VsMain -p vs_5_0  -a c:\\files\\myShader.csv c:\\files\\myShader.hlsl" << endl;
            cout << " Compile using DX Assembly in binary format: " << programName << " -f  VsMain -s DXAsm -p vs_5_0 c:\\files\\myShader.obj  --isa c:\\temp\\dxTest.isa" << endl;
            cout << " Compile using FXC: " << programName << " -s DXAsm -f  VsMain -p vs_5_0 c:\\files\\myShader.obj --isa c:\\files\\myIsa.isa c:\\files\\myShader.hlsl -c tahiti --FXC \"\"C:\\Program Files (x86)\\Windows Kits\\8.1\\bin\\x86\\fxc.exe\" /E VsMain /T vs_5_0  /Fo c:\\files\\myShader.obj c:\\files\\myShader.fx\"" << endl;
            cout << " Compile using DX Assembly in text format: " << programName << " -f  VsMain -s DXAsmTxt -p vs_5_0 c:\\files\\myShaderblob.txt  --isa c:\\temp\\dxTest.isa c:\\files\\myShader.hlsl" << endl;
        }
        else if ((config.m_RequestedCommand == Config::ccHelp) && (config.m_SourceLanguage == SourceLanguage_AMDIL))
        {
            cout << "*** AMDIL Instructions & Options (Windows Only) ***" << endl;
            cout << "===================================================" << endl;
            cout << "Usage: " << programName << " [options] source_file" << endl;
            cout << genericOpt << endl;
            cout << "Examples:" << endl;
            cout << " Generate ISA from AMDIL code: " << programName << " -s amdil --isa c:\\files\\isaFromAmdil.isa c:\\files\\myAmdilCode.amdil" << endl;
            cout << " Generate ISA and performance statistics from AMDIL code: " << programName << " -s amdil --isa c:\\files\\isaFromAmdil.isa -a c:\\files\\statsFromAmdil.csv c:\\files\\myAmdilCode.amdil" << endl;
            cout << " Generate ISA from AMDIL code, and perform live register analysis: " << programName << " -s amdil --isa c:\\output\\myShader.isa --livereg c:\\output\\ c:\\files\\myAmdilCode.amdil" << endl;
        }
        else if ((config.m_RequestedCommand == Config::ccHelp) && (config.m_SourceLanguage == SourceLanguage_GLSL_Vulkan ||
            config.m_SourceLanguage == SourceLanguage_SPIRV_Vulkan || config.m_SourceLanguage == SourceLanguage_SPIRVTXT_Vulkan))
        {
            // Get the mode name, and the relevant file extensions. We will use it when constructing the example string.
            const char* FILE_EXT_SPV = "spv";
            const char* FILE_EXT_SPV_TXT = "txt";
            std::string rgaModeName;
            std::string vertExt;
            std::string fragExt;
            switch (config.m_SourceLanguage)
            {
            case SourceLanguage_GLSL_Vulkan:
                rgaModeName = Config::sourceKindGLSLVulkan;
                vertExt = "vert";
                fragExt = "frag";
                break;
            case SourceLanguage_SPIRV_Vulkan:
                rgaModeName = Config::sourceKindSpirvBin;
                vertExt = FILE_EXT_SPV;
                fragExt = FILE_EXT_SPV;
                break;
            case SourceLanguage_SPIRVTXT_Vulkan:
                rgaModeName = Config::sourceKindSpirvTxt;
                vertExt = FILE_EXT_SPV_TXT;
                fragExt = FILE_EXT_SPV_TXT;
                break;
            }

            // Convert the mode name string to lower case for presentation.
            std::transform(rgaModeName.begin(), rgaModeName.end(), rgaModeName.begin(), ::tolower);

            cout << "*** Vulkan Instructions & Options ***" << endl;
            cout << "=================================" << endl;
            cout << "Usage: " << programName << " [options]";
            if (config.m_SourceLanguage == SourceLanguage_SPIRV_Vulkan || config.m_SourceLanguage == SourceLanguage_SPIRVTXT_Vulkan)
            {
                cout << " [optional: spv_input_file]";
            }
            cout << endl << endl;
            if (config.m_SourceLanguage == SourceLanguage_SPIRV_Vulkan || config.m_SourceLanguage == SourceLanguage_SPIRVTXT_Vulkan)
            {
                cout << "Notes:" << endl;
                cout << " * The input file(s) must be specified in one of two ways:" << endl <<
                    "   1) A single SPIR-V input file provided as \"spv_input_file\", or\n" <<
                    "   2) One or more pipeline stage specific shader files specified by the pipeline stage options (--vert, --tesc, etc.)." << endl << endl;
            }
            cout << genericOpt << endl;
            cout << pipelinedOpt << endl;
            cout << "Examples:" << endl;
            cout << " Extract ISA, AMD IL and statistics for a Vulkan program that is comprised of a vertex shader and a fragment shader for all devices: " << programName << " -s " << rgaModeName << " --isa c:\\output\\vulkan_isa.isa --il c:\\output\\vulkan_il.amdil -a c:\\output\\vulkan_stats.stats --vert c:\\source\\myVertexShader." << vertExt << " --frag c:\\source\\myFragmentShader." << fragExt << endl;
            cout << " Extract ISA, AMD IL and statistics for a Vulkan program that is comprised of a vertex shader and a fragment shader for Iceland and Fiji: " << programName << " -s " << rgaModeName << " -c Iceland -c Fiji --isa c:\\output\\vulkan_isa.isa --il c:\\output\\vulkan_il.amdil -a c:\\output\\vulkan_stats.stats --vert c:\\source\\myVertexShader." << vertExt << " --frag c:\\source\\myFragmentShader." << fragExt << endl;
            cout << " Extract ISA and binaries for a Vulkan program that is comprised of a vertex shader and a fragment shader for all devices: " << programName << " -s " << rgaModeName << " --isa c:\\output\\vulkan_isa.isa -b c:\\output\\vulkan_bin.bin -a c:\\output\\vulkan_stats.stats --vert c:\\source\\myVertexShader." << vertExt << " --frag c:\\source\\myFragmentShader." << fragExt << endl;
            cout << " Extract ISA and perform live register analysis for a Vulkan program for all devices: " << programName << " -s " << rgaModeName << " --isa c:\\output\\vulkan_isa.isa --livereg c:\\output\\ --vert c:\\source\\myVertexShader." << vertExt << " --frag c:\\source\\myFragmentShader." << fragExt << endl;
            cout << " Extract ISA for a single SPIR-V file, without specifying the pipeline stages: " << programName << " -s " << rgaModeName << " --isa c:\\output\\program.isa c:\\source\\program.spv" << endl;
        }
        else if ((config.m_RequestedCommand == Config::ccHelp) && (config.m_SourceLanguage == SourceLanguage_GLSL_OpenGL))
        {
            cout << "*** OpenGL Instructions & Options ***" << endl;
            cout << "=================================" << endl;
            cout << "Usage: " << programName << " [options]" << endl;
            cout << genericOpt << endl;
            cout << pipelinedOpt << endl;
            cout << "Examples:" << endl;
            cout << " Extract ISA, binaries and statistics for an OpenGL program that is comprised of a vertex shader and a fragment shader for all devices: " << programName << " -s opengl --isa c:\\output\\opengl_isa.isa -b c:\\output\\opengl_bin.bin -a c:\\output\\opengl_stats.stats --vert c:\\source\\myVertexShader.vert --frag c:\\source\\myFragmentShader.frag" << endl;
            cout << " Extract ISA and statistics for an OpenGL program that is comprised of a vertex shader and a fragment shader for Tahiti: " << programName << " -s opengl -c Tahiti --isa c:\\output\\opengl_isa.isa -a c:\\output\\opengl_stats.stats --vert c:\\source\\myVertexShader.vert --frag c:\\source\\myFragmentShader.frag" << endl;
            cout << " Extract ISA and perform live register analysis for an OpenGL program that is comprised of a vertex shader and a fragment shader for Tahiti: " << programName << " -s opengl -c Tahiti --isa c:\\output\\opengl_isa.isa --livereg c:\\output\\ --vert c:\\source\\myVertexShader.vert --frag c:\\source\\myFragmentShader.frag" << endl;
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
        // TODO: Should be able to distinguish problem from --help/--version.
        //return false;
        doWork = false;
    }

    return doWork;
}


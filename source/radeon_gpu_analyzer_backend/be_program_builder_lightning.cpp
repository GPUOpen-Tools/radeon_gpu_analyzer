//======================================================================
// Copyright 2020-2023 Advanced Micro Devices, Inc. All rights reserved.
//======================================================================

// Infra.
#ifdef _WIN32
    #pragma warning(push)
    #pragma warning(disable:4309)
#endif
#include "external/amdt_os_wrappers/Include/osFilePath.h"
#include "external/amdt_os_wrappers/Include/osDirectory.h"
#include "external/amdt_base_tools/Include/gtAssert.h"
#include "external/amdt_os_wrappers/Include/osProcess.h"
#include "external/amdt_os_wrappers/Include/osApplication.h"
#ifdef _WIN32
    #pragma warning(pop)
#endif

// Yaml.
#include "yaml-cpp/yaml.h"

// Local
#include "source/radeon_gpu_analyzer_cli/kc_utils.h"
#include "radeon_gpu_analyzer_backend/be_utils.h"
#include "radeon_gpu_analyzer_backend/be_string_constants.h"
#include "radeon_gpu_analyzer_backend/be_program_builder_lightning.h"
#include "emulator/parser/be_isa_parser.h"

// *****************************************
// *** INTERNALLY LINKED SYMBOLS - START ***
// *****************************************

// Constants.
static const gtString kLcOpenclIncludeFile = L"opencl-c.h";

static const std::string  kStrLcOpenclStdOption = "-cl-std";
static const std::string  kStrLcOpenclStdDefaultValue = "cl2.0";
static const std::string  kStrLcOpenclDefs = "-D__OPENCL_VERSION__=200";

// Tokens.
static const std::string  kStrLcCompilerErrorToken = "error:";
static const std::string  kStrLcIsaDisassemblyToken = "Disassembly of section .text:";
static const std::string  kStrLcCodeObjectMetadataTokenStart = "---\n";
static const std::string  kStrLcCodeObjectMetadataTokenEnd = "\n...";

// Lightning compiler switches.
static const std::string  kStrLcObjDumpMetdataOptionToken = "-amdgpu-code-object-metadata";
static const std::string  kStrLcCompilerOpenclSwitchTriple = "--target=amdgcn-amd-amdhsa";
static const std::string  kStrLcCompilerOpenclSwitchInclude = "-include ";
static const std::string  kStrLcCompilerOpenclSwitchDevice = "-mcpu=";
static const std::string  kStrLcCompilerOpenclDefaultDevice = "gfx1100";
static const std::string  kStrLcCompilerOpenclSwitchVersion = "--version";
static const std::string  kStrLcCompilerOpenclSwitchPreprocessor = "-E";
static const std::string  kStrLcCompilerOpenclSwitchRcomLibPath  = "--rocm-device-lib-path=";

// This flag is set to dump LLVM IR for 1st pass only instead of all passes.
// This is used to prevent compilation hanging for real world kernels.
static const std::string  kStrLcCompilerOpenclSwitchIlDump = "-mllvm --print-before=amdgpu-opencl-12-adapter";
static const std::string  kStrLcCompilerOpenclSwitchDebugInfo = "-g";
static const std::string  kStrLcCompilerOpenclSwitchOptimizationLevel = "-O";

// LLVM objdump switches.
static const std::string  kStrLcObjDumpSwitchDevice = "--mcpu=";
static const std::string  kStrLcObjDumpSwitchDisassemble = "--disassemble --symbolize-operands";
static const std::string  kStrLcObjDumpSwitchDisassembleLineNumbers = "--disassemble --symbolize-operands --line-numbers --source";
static const std::string  kStrLcObjDumpSwitchMetadata1 = "--amdgpu-code-object-metadata --lf-output-style=GNU --notes";
static const std::string  kStrLcObjDumpSwitchMetadata2 = "--elf-output-style=GNU --notes";
static const std::string  kStrLcObjDumpSwitchTriple = "--triple=amdgcn-amd-amdhsa";
static const std::string  kStrLcObjDumpSwitchSymbols = "--symbols";
static const std::string  kStrLcObjDumpSwitchHelp = "--help";

// CodeObject MetaData keys.
static const std::string  kStrCodeObjectMetadataKeyKernels = "amdhsa.kernels";
static const std::string  kStrCodeObjectMetadataKeyKernelName = ".name";
static const std::string  kStrCodeObjectMetadataKeyWavefrontSgprs = ".sgpr_count";
static const std::string  kStrCodeObjectMetadataKeyWorkitemVgprs = ".vgpr_count";
static const std::string  kStrCodeObjectMetadataKeyWavefrontSize = ".wavefront_size";
static const std::string  kStrCodeObjectMetadataKeySpilledSgprs = ".sgpr_spill_count";
static const std::string  kStrCodeObjectMetadataKeySpilledVgprs = ".vgpr_spill_count";
static const std::string  kStrCodeObjectMetadataKeyGroupSegmentSize = ".group_segment_fixed_size";
static const std::string  kStrCodeObjectMetadataKeyPrivateSegmentSize = ".private_segment_fixed_size";

// Readobj symbols output keys.
static const std::string  kStrReadObjKeySymbols = "Symbols [";
static const std::string  kStrReadObjKeySymbol = "Symbol {";
static const std::string  kStrReadObjKeyName = "Name: ";
static const std::string  kStrReadObjKeyNameEnd = " (";
static const std::string  kStrReadObjKeySize = "Size: ";

// Numerical constants.
static const unsigned long kLcExecTimeoutMs = kProcessWaitInfinite;
static const unsigned long kLcPreprocessingTimeoutMs = kProcessWaitInfinite;
static const unsigned long kObjdumpExecTimeoutMs = kProcessWaitInfinite;
static const unsigned int  kLcAckSize = 256;

static const wchar_t* kLcOpenclBinDir = L"bin";
static const wchar_t* kLcOpenclIncludeDir = L"include";
static const wchar_t* kLcOpenclLibDir = L"lib/bitcode";

static const wchar_t* kLcOpenclCompilerExecutable = L"clang";
static const wchar_t* kLcOpenclAmdgpuObjdumpExecutable = L"amdgpu-objdump";
static const wchar_t* kLcOpenclLlvmObjdumpExecutable = L"llvm-objdump";
static const wchar_t* kLcOpenclLlvmReadobjExecutable = L"llvm-readobj";

// ***************************************
// *** INTERNALLY LINKED SYMBOLS - END ***
// ***************************************

static bool GetIsaSize(const std::string& isa_as_text, const std::string& kernel_name, size_t& size_in_bytes);
static beKA::beStatus  ParseCodeProps(const std::string & md_text, CodePropsMap& code_props);

beKA::beStatus BeProgramBuilderLightning::GetKernelIlText(const std::string & device, const std::string & kernel, std::string & il)
{
    GT_UNREFERENCED_PARAMETER(device);
    GT_UNREFERENCED_PARAMETER(kernel);
    GT_UNREFERENCED_PARAMETER(il);
    return beKA::beStatus();
}

beKA::beStatus BeProgramBuilderLightning::GetKernelIsaText(const std::string & device, const std::string & kernel, std::string & isa)
{
    GT_UNREFERENCED_PARAMETER(device);
    GT_UNREFERENCED_PARAMETER(kernel);
    GT_UNREFERENCED_PARAMETER(isa);
    return beKA::beStatus();
}

beKA::beStatus BeProgramBuilderLightning::GetStatistics(const std::string & device, const std::string & kernel, beKA::AnalysisData & analysis)
{
    GT_UNREFERENCED_PARAMETER(device);
    GT_UNREFERENCED_PARAMETER(kernel);
    GT_UNREFERENCED_PARAMETER(analysis);
    return beKA::beStatus();
}

beKA::beStatus BeProgramBuilderLightning::GetDeviceTable(std::vector<GDT_GfxCardInfo>& table)
{
    GT_UNREFERENCED_PARAMETER(table);
    return beKA::beStatus();
}

beKA::beStatus BeProgramBuilderLightning::GetCompilerVersion(beKA::RgaMode      mode,
                                                             const std::string& user_bin_folder,
                                                             bool               should_print_cmd,
                                                             std::string&       out_text)
{
    std::string error_text;
    beKA::beStatus  status = InvokeCompiler(mode, user_bin_folder, kStrLcCompilerOpenclSwitchVersion, should_print_cmd, out_text, error_text);
    return status;
}

beKA::beStatus BeProgramBuilderLightning::AddCompilerStandardOptions(beKA::RgaMode       mode,
                                                                     const CmpilerPaths& compiler_paths,
                                                                     std::string&        options)
{
    std::stringstream options_stream;
    beKA::beStatus status = beKA::kBeStatusSuccess;

    if (mode == beKA::RgaMode::kModeOpenclOffline)
    {
        // Add nogpulib and target triple.
        options_stream << " " << kStrLcCompilerOpenclSwitchTriple;

// The following is necessary if the bitcode files are built on Windows with Visual Studio.
#ifdef _WIN32
        // Short character flag required for bitcode built with MSVC.
        static const std::string  kStrLcCompilerOpenCLSwitchShortWcharWindows = "-fshort-wchar";
        // static flag improves performance on Windows platforms.
        static const std::string  kStrLcCompilerOpenclSwitchStatic = "-static";
        options_stream << " " << kStrLcCompilerOpenclSwitchStatic << " " << kStrLcCompilerOpenCLSwitchShortWcharWindows;
#endif

        // Add OpenCL include required by OpenCL compiler.
        osFilePath  include_file_path;
        if (!compiler_paths.inc.empty())
        {
            gtString include_dir;
            include_dir << compiler_paths.inc.c_str();
            include_file_path.setFileDirectory(include_dir);
        }
        else
        {
            osGetCurrentApplicationPath(include_file_path, false);
            include_file_path.clearFileExtension();
            include_file_path.appendSubDirectory(kLcOpenclRootDir);
            include_file_path.appendSubDirectory(kLcOpenclIncludeDir);
        }

        include_file_path.setFileName(kLcOpenclIncludeFile);
        options_stream << " " << kStrLcCompilerOpenclSwitchInclude << KcUtils::Quote(include_file_path.asString().asASCIICharArray());

        // Add OpenCL device libs.
        osFilePath  lib_file_path;
        if (!compiler_paths.lib.empty())
        {
            gtString lib_dir;
            lib_dir << compiler_paths.lib.c_str();
            lib_file_path.setFileDirectory(lib_dir);
        }
        else
        {
            osGetCurrentApplicationPath(lib_file_path, false);
            lib_file_path.clearFileExtension();
            lib_file_path.appendSubDirectory(kLcOpenclRootDir);
            lib_file_path.appendSubDirectory(kLcOpenclLibDir);
            lib_file_path.clearFileName();
        }
        
        options_stream << " " << kStrLcCompilerOpenclSwitchRcomLibPath << KcUtils::Quote(lib_file_path.asString().asASCIICharArray());

    }
    else
    {
        status = beKA::kBeStatusUnknownInputLang;
    }
    options = options_stream.str();

    return status;
}

beKA::beStatus BeProgramBuilderLightning::ConstructOpenCLCompilerOptions(const CmpilerPaths& compiler_paths,
    const OpenCLOptions& user_options,
    const std::vector<std::string>& src_file_names,
    const std::string& bin_file_names,
    const std::string& device,
    std::string& out_options)
{
    beKA::beStatus status;
    std::string  standard_options = "";
    status                          = AddCompilerStandardOptions(beKA::RgaMode::kModeOpenclOffline, compiler_paths, standard_options);
    if (status == beKA::kBeStatusSuccess)
    {
        std::stringstream options;
        options << standard_options;

        // Add options specifying the OpenCL standard.
        // Use default value if it is not provided in the user options.
        if (std::count_if(user_options.opencl_compile_options.begin(), user_options.opencl_compile_options.end(),
                          [&](const std::string& s) { return (s.find(kStrLcOpenclStdOption) != std::string::npos); }) == 0)
        {
            options << " " << kStrLcOpenclStdOption << "=" << kStrLcOpenclStdDefaultValue;
            options << " " << kStrLcOpenclDefs;
        }

        // Specify optimization level if required.
        if (user_options.optimization_level != -1)
        {
            options << " " << kStrLcCompilerOpenclSwitchOptimizationLevel << user_options.optimization_level;
        }

        // Add the source debug info switch.
        if (user_options.line_numbers)
        {
            options << " " << kStrLcCompilerOpenclSwitchDebugInfo;
        }

        // Add the device selection options.
        options << " " << kStrLcCompilerOpenclSwitchDevice << device;

        // Add the source and output file options.
        for (const std::string& srcName : src_file_names)
        {
            options << " " << KcUtils::Quote(srcName);
        }

        // We can either output the binary, or LLVM IR disassembly, not both.
        if (!user_options.should_generate_llvm_ir_disassembly)
        {
            // Binary.
            options << " -o " << KcUtils::Quote(bin_file_names);
        }
        else
        {
            // LLVM IR.
            options << " " << kStrLcCompilerOpenclSwitchIlDump;
            options << " -emit-llvm -S -o " << KcUtils::Quote(bin_file_names) << " ";
        }

        // Add user-provided options
        for (const std::string & option : user_options.opencl_compile_options)
        {
            options << " " << option;
        }

        // Add include paths provided by user.
        for (const std::string& include_path : user_options.include_paths)
        {
            options << " -I" << KcUtils::Quote(include_path);
        }

        // Add the definitions
        for (const std::string & def : user_options.defines)
        {
            size_t  asgnOffset = def.find('=');
            options << " -D" << (asgnOffset == std::string::npos ? def : def.substr(0, asgnOffset + 1) + KcUtils::Quote(def.substr(asgnOffset + 1)));
        }
        out_options = options.str();
    }

    return status;
}

beKA::beStatus BeProgramBuilderLightning::CompileOpenCLToBinary(const CmpilerPaths& compiler_paths,
    const OpenCLOptions& user_options,
    const std::vector<std::string>& src_file_names,
    const std::string& bin_file_names,
    const std::string& device,
    bool should_print_cmd,
    std::string& error_text)
{
    // Generate the compiler command line options string.
    beKA::beStatus status = beKA::kBeStatusSuccess;
    std::string options;
    std::string output_text;

    status = ConstructOpenCLCompilerOptions(compiler_paths, user_options, src_file_names, bin_file_names, device, options);
    if (status == beKA::kBeStatusSuccess)
    {
        // Run LC compiler.
        status = InvokeCompiler(beKA::RgaMode::kModeOpenclOffline, compiler_paths.bin, options, should_print_cmd, output_text, error_text);
    }

    if (status == beKA::kBeStatusSuccess)
    {
        status = VerifyCompilerOutput(bin_file_names, error_text);
    }

    return status;
}

beKA::beStatus BeProgramBuilderLightning::CompileOpenCLToLlvmIr(const CmpilerPaths&             compiler_paths,
                                                                const OpenCLOptions&            user_options,
                                                                const std::vector<std::string>& src_file_names,
                                                                const std::string&              il_file_name,
                                                                const std::string&              device,
                                                                bool                            should_print_cmd,
                                                                std::string&                    error_text)
{
    // Generate the compiler command line options string.
    beKA::beStatus status = beKA::kBeStatusSuccess;
    std::string    options;
    std::string    output_text;

    status = ConstructOpenCLCompilerOptions(compiler_paths, user_options, src_file_names, il_file_name, device, options);
    if (status == beKA::kBeStatusSuccess)
    {
        // Run LC compiler.
        status = InvokeCompiler(beKA::RgaMode::kModeOpenclOffline, compiler_paths.bin, options, should_print_cmd, output_text, error_text);
    }

    if (status == beKA::kBeStatusSuccess)
    {
        status = VerifyCompilerOutput(il_file_name, error_text);
    }

    return status;
}

beKA::beStatus BeProgramBuilderLightning::InvokeCompiler(beKA::RgaMode      mode,
    const std::string& user_bin_folder,
    const std::string& cmd_line_options,
    bool should_print_cmd,
    std::string& std_out,
    std::string& std_err,
    unsigned long timeout)
{
    beKA::beStatus status = beKA::kBeStatusSuccess;
    osFilePath lc_compiler_exec;
    long exit_code;

    // Use default timeout if not specified.
    if (timeout == 0)
    {
        timeout = kLcExecTimeoutMs;
    }

    // Select the compiler executable.
    if (!user_bin_folder.empty())
    {
        gtString  bin_folder;
        bin_folder << user_bin_folder.c_str();
        lc_compiler_exec.setFileDirectory(bin_folder);
        lc_compiler_exec.setFileName(kLcOpenclCompilerExecutable);
        lc_compiler_exec.setFileExtension(kLcCompilerExecutableExtension);
    }
    else
    {
        if (mode == beKA::RgaMode::kModeOpenclOffline)
        {
            osGetCurrentApplicationPath(lc_compiler_exec, false);
            lc_compiler_exec.appendSubDirectory(kLcOpenclRootDir);
            lc_compiler_exec.appendSubDirectory(kLcOpenclBinDir);
            lc_compiler_exec.setFileName(kLcOpenclCompilerExecutable);
            lc_compiler_exec.setFileExtension(kLcCompilerExecutableExtension);
        }
        else
        {
            return beKA::kBeStatusUnknownInputLang;
        }
    }

    KcUtils::ProcessStatus  procStatus = KcUtils::LaunchProcess(lc_compiler_exec.asString().asASCIICharArray(),
                                                                cmd_line_options,
                                                                "",
                                                                timeout,
                                                                should_print_cmd,
                                                                std_out,
                                                                std_err,
                                                                exit_code);

    switch (procStatus)
    {
    case KcUtils::ProcessStatus::kLaunchFailed:
    case KcUtils::ProcessStatus::kCreateTempFileFailed:
    case KcUtils::ProcessStatus::kReadTempFileFailed:
        status = beKA::kBeStatusLightningCompilerLaunchFailed;
        break;
    case KcUtils::ProcessStatus::kTimeOut:
        status = beKA::kBeStatusLightningCompilerTimeOut;
        break;
    }

    // Check the process exit status.
    if (status == beKA::beStatus::kBeStatusSuccess)
    {
        status = (exit_code == 0L ? beKA::beStatus::kBeStatusSuccess : beKA::beStatus::kBeStatusLightningCompilerGeneratedError);
    }

    return status;
}

beKA::beStatus BeProgramBuilderLightning::VerifyCompilerOutput(const std::string & out_filename, const std::string & error_text)
{
    gtString out_filename_gtstr;
    out_filename_gtstr.fromASCIIString(out_filename.c_str());
    osFilePath out_file_path(out_filename_gtstr);
    beKA::beStatus status = beKA::beStatus::kBeStatusSuccess;

    if (out_filename.empty() || !out_file_path.exists())
    {
        status = error_text.find(kStrLcCompilerErrorToken) != std::string::npos ? beKA::beStatus::kBeStatusLightningCompilerGeneratedError
                     : (out_filename.empty() ? beKA::beStatus::kBeStatusSuccess : beKA::beStatus::kBeStatusNoOutputFileGenerated);
    }

    return status;
}

beKA::beStatus BeProgramBuilderLightning::PreprocessOpencl(const CmpilerPaths& compiler_paths,
                                                           const std::string&  input_file,
                                                           const std::string&  args,
                                                           bool                should_print_cmd,
                                                           std::string&        output)
{
    std::string     standard_options = "";
     beKA::beStatus status           = AddCompilerStandardOptions(beKA::RgaMode::kModeOpenclOffline, compiler_paths, standard_options);
    if (status == beKA::beStatus::kBeStatusSuccess)
    {
        std::stringstream compiler_args;
        compiler_args << standard_options;
        compiler_args << " " << kStrLcCompilerOpenclSwitchPreprocessor << " " << args << " " << KcUtils::Quote(input_file);
        compiler_args << " " << kStrLcOpenclStdOption << "=" << kStrLcOpenclStdDefaultValue;
        compiler_args << " " << kStrLcOpenclDefs;

        // Add a default device selection option.
        // (as of v2.8 we need to specify *some* default device needed for clang-18 in RGA).
        compiler_args << " " << kStrLcCompilerOpenclSwitchDevice << kStrLcCompilerOpenclDefaultDevice;
        
        std::string std_out, std_err;

        beKA::beStatus status = InvokeCompiler(beKA::RgaMode::kModeOpenclOffline, compiler_paths.bin, compiler_args.str(), should_print_cmd, std_out, std_err, kLcPreprocessingTimeoutMs);

        if (status == beKA::beStatus::kBeStatusSuccess)
        {
            status = VerifyCompilerOutput("", std_err);
        }

        if (status == beKA::beStatus::kBeStatusSuccess)
        {
            output = std_out;
        }
        else
        {
            output = std_err;
        }
    }

    return status;
}

beKA::beStatus BeProgramBuilderLightning::DisassembleBinary(const std::string& user_bin_dir,
    const std::string& bin_filename,
    const std::string& device,
    bool line_numbers,
    bool should_print_cmd,
    std::string& out_isa_text,
    std::string& error_text)
{
    beKA::beStatus status = beKA::kBeStatusSuccess;
    gtString  bin_filename_gtstr;
    bin_filename_gtstr.fromASCIIString(bin_filename.c_str());

    if (osFilePath(bin_filename_gtstr).exists())
    {
        std::string objdump_options = "";
        out_isa_text = "";
        ObjDumpOp op = (line_numbers ? ObjDumpOp::kDisassembleWithLineNumbers : ObjDumpOp::kDisassemble);

        beKA::beStatus status = ConstructObjDumpOptions(op, user_bin_dir, bin_filename, device, should_print_cmd, objdump_options);

        if (status == beKA::kBeStatusSuccess)
        {
            status = InvokeObjDump(ObjDumpOp::kDisassemble, user_bin_dir, objdump_options, should_print_cmd, out_isa_text, error_text);
        }

        if (status == beKA::kBeStatusSuccess)
        {
            status = VerifyObjDumpOutput(out_isa_text);
        }

        if (status == beKA::kBeStatusSuccess)
        {
            status = FilterCodeObjOutput(ObjDumpOp::kDisassemble, out_isa_text);
        }
    }
    else
    {
        status = beKA::kBeStatusNoBinaryForDevice;
    }

    return status;
}

beKA::beStatus BeProgramBuilderLightning::ExtractMetadata(const std::string& user_bin_dir, const std::string & bin_filename,
                                                          bool print_cmd, std::string& metadata_text)
{
    beKA::beStatus  status = beKA::kBeStatusLightningExtractMetadataFailed;
    std::string readobj_output, options, errText;

    // Launch the LC ReadObj.
    status = ConstructObjDumpOptions(ObjDumpOp::kGetMetadata, user_bin_dir, bin_filename, "", print_cmd, options);

    if (status == beKA::kBeStatusSuccess)
    {
        status = InvokeObjDump(ObjDumpOp::kGetMetadata, user_bin_dir, options, print_cmd, readobj_output, errText);
    }

    if (status == beKA::kBeStatusSuccess)
    {
        metadata_text = readobj_output;
    }

    return status;
}

beKA::beStatus BeProgramBuilderLightning::ExtractKernelCodeProps(const std::string& user_bin_dir,
                                                                 const std::string& bin_filename,
                                                           bool should_print_cmd, CodePropsMap& code_props)
{
    beKA::beStatus  status = beKA::kBeStatusLightningExtractCodePropsFailed;
    std::string  metadata_text;
    if ((status = ExtractMetadata(user_bin_dir, bin_filename, should_print_cmd, metadata_text)) == beKA::beStatus::kBeStatusSuccess)
    {
        status = ParseCodeProps(metadata_text, code_props);
    }

    return status;
}

beKA::beStatus BeProgramBuilderLightning::ExtractKernelNames(const std::string& user_bin_dir, const std::string& bin_filename,
                                                             bool should_print_cmd, std::vector<std::string>& kernel_names)
{
    beKA::beStatus status = beKA::kBeStatusSuccess;
    std::string metadata, options, error_text;

    // Launch the LC ReadObj and parse its output.
    status = ConstructObjDumpOptions(ObjDumpOp::kGetMetadata, user_bin_dir, bin_filename, "", should_print_cmd, options);

    if (status == beKA::kBeStatusSuccess)
    {
        status = InvokeObjDump(ObjDumpOp::kGetMetadata, user_bin_dir, options, should_print_cmd, metadata, error_text);
    }

    if (status == beKA::kBeStatusSuccess)
    {
        YAML::Node codeobj_metadata_node, kernels_metadata_map, kernel_name;
        size_t start_offset = 0, end_offset;
        start_offset = metadata.find(kStrLcCodeObjectMetadataTokenStart);

        // Load all Metadata nodes found in the objdump output.
        while ((end_offset = metadata.find(kStrLcCodeObjectMetadataTokenEnd, start_offset)) != std::string::npos)
        {
            try
            {
                codeobj_metadata_node = YAML::Load(metadata.substr(start_offset + kStrLcCodeObjectMetadataTokenStart.size(), end_offset - (start_offset + kStrLcCodeObjectMetadataTokenStart.size())));
            }
            catch (YAML::ParserException&)
            {
                status = beKA::kBeStatusLightningExtractKernelNamesFailed;
            }

            if (status == beKA::kBeStatusSuccess && codeobj_metadata_node.IsMap())
            {
                // Look for kernels metadata.
                if (status == beKA::kBeStatusSuccess &&
                    (kernels_metadata_map = codeobj_metadata_node[kStrCodeObjectMetadataKeyKernels]).IsDefined())
                {
                    for (const YAML::Node& kernel_metadata : kernels_metadata_map)
                    {
                        if (status == beKA::kBeStatusSuccess && (kernel_name = kernel_metadata[kStrCodeObjectMetadataKeyKernelName]).IsDefined())
                        {
                            kernel_names.push_back(kernel_name.as<std::string>());
                        }
                        else
                        {
                            status = beKA::kBeStatusLightningExtractKernelNamesFailed;
                            break;
                        }
                    }
                }
            }
            start_offset = metadata.find(kStrLcCodeObjectMetadataTokenStart, end_offset);
        }
    }

    return status;
}

bool BeProgramBuilderLightning::VerifyOutputFile(const std::string & filename)
{
    bool  ret = BeUtils::IsFilePresent(filename);
    return ret;
}

int BeProgramBuilderLightning::GetIsaSize(const std::string & isa_text)
{
    ParserIsa isa_parser;
    int  isa_size = 0;

    if (isa_parser.ParseForSize(isa_text))
    {
        isa_size = isa_parser.GetCodeLength();
    }

    return isa_size;
}

int BeProgramBuilderLightning::GetKernelCodeSize(const std::string & user_bin_dir, const std::string & bin_file,
                                                 const std::string & kernel_name, bool should_print_cmd)
{
    beKA::beStatus status = beKA::beStatus::kBeStatusLightningGetKernelCodeSizeFailed;
    std::string symbols, options, error_text;
    int ret = -1, symSize;
    size_t offset, name_offset, name_end_offset, size_offset;

    // Launch the LC ReadObj.
    status = ConstructObjDumpOptions(ObjDumpOp::kGetKernelCodeSize, user_bin_dir, bin_file, "", should_print_cmd, options);

    if (status == beKA::beStatus::kBeStatusSuccess)
    {
        status = InvokeObjDump(ObjDumpOp::kGetKernelCodeSize, user_bin_dir, options, should_print_cmd, symbols, error_text);
    }

    if (status == beKA::beStatus::kBeStatusSuccess)
    {
        // Parse the readobj output.
        // readobj uses its own format, so we have to parse it manually.
        //
        //    File: gfx900_test.bin
        //    Format: elf64-amdgpu
        //    Arch: amdgcn
        //    AddressSize : 64bit
        //    LoadName : <Not found>

        //    Symbols [
        //      ...
        //      Symbol {
        //        Name: src_MyKernel (1)
        //        Value : 0x1000
        //        Size : 8
        //        Binding : Global (0x1)
        //        Type : Function (0x2)
        //        Other[(0x3)
        //          STV_PROTECTED (0x3)
        //        ]
        //        Section : .text (0x7)
        //      }
        //      ...
        //    ]

        if (status == beKA::beStatus::kBeStatusSuccess)
        {
            if ((offset = symbols.find(kStrReadObjKeySymbols)) != std::string::npos)
            {
                bool stop = false;
                while (!stop && (offset = symbols.find(kStrReadObjKeySymbol, offset)) != std::string::npos)
                {
                    name_offset = symbols.find(kStrReadObjKeyName, offset);
                    if (name_offset != std::string::npos)
                    {
                        size_offset = symbols.find(kStrReadObjKeySize, name_offset);
                        if (size_offset != std::string::npos)
                        {
                            size_offset += kStrReadObjKeySize.size();
                            name_end_offset = symbols.find(kStrReadObjKeyNameEnd, name_offset);
                        }
                        if (name_end_offset != std::string::npos)
                        {
                            name_offset += kStrReadObjKeyName.size();
                            size_t kernel_name_end = symbols.find(kStrReadObjKeyNameEnd, name_offset);
                            if (symbols.substr(name_offset, kernel_name_end - name_offset) == kernel_name)
                            {
                                size_t size_end_offset = symbols.find("\n    Bind", size_offset);
                                if (size_end_offset != std::string::npos)
                                {
                                    std::string size_as_string = symbols.substr(size_offset, size_end_offset - size_offset);
                                    symSize = std::atoi(size_as_string.c_str());
                                    ret = symSize;
                                    stop = true;
                                }
                                else
                                {
                                    ret = 0;
                                }
                            }
                            offset = size_offset;
                        }
                    }
                }
            }
        }
    }

    return ret;
}

beKA::beStatus BeProgramBuilderLightning::ConstructObjDumpOptions(ObjDumpOp op,
    const std::string& compiler_bin_dir,
    const std::string& bin_filename,
    const std::string& device,
    bool should_print_cmd,
    std::string& options)
{
    std::stringstream all_options;
    std::string op_selector;

    switch (op)
    {
    case ObjDumpOp::kDisassemble:
        op_selector = kStrLcObjDumpSwitchDisassemble;
        break;
    case ObjDumpOp::kDisassembleWithLineNumbers:
        op_selector = kStrLcObjDumpSwitchDisassembleLineNumbers;
        break;
    case ObjDumpOp::kGetMetadata:
        // Use OBJDUMP_METADATA_SWITCH_2 option set for LLVM versions >= 6 and unknown version (0).
        op_selector = DoesReadobjSupportMetadata(compiler_bin_dir, should_print_cmd) ? kStrLcObjDumpSwitchMetadata1 : kStrLcObjDumpSwitchMetadata2;
        break;
    case ObjDumpOp::kGetKernelCodeSize:
        op_selector = kStrLcObjDumpSwitchSymbols;
        break;
    default:
        return beKA::kBeStatusUnknownObjDumpOperation;
    }

    all_options << op_selector;
    // Add the platform triple and device for disassemble.
    if (op == ObjDumpOp::kDisassemble || op == ObjDumpOp::kDisassembleWithLineNumbers)
    {
        all_options << " " << kStrLcObjDumpSwitchTriple << " " << kStrLcObjDumpSwitchDevice << device;
    }

    // Add the binary file to process.
    all_options << " " << KcUtils::Quote(bin_filename);

    options = all_options.str();

    return beKA::kBeStatusSuccess;
}

beKA::beStatus BeProgramBuilderLightning::InvokeObjDump(ObjDumpOp op, const std::string& user_bin_dir, const std::string& cmd_line_options,
    bool should_print_cmd, std::string& out_text, std::string& error_text)
{
    osFilePath lc_objdump_exec;
    long exit_code;

    // llvm-objdump is currently not able to extract the CodeObj Metadata, so use llvm-readobj instead.
    const gtString  objdump_exec_name = ((op == ObjDumpOp::kGetMetadata || op == ObjDumpOp::kGetKernelCodeSize)
                                       ? kLcOpenclLlvmReadobjExecutable : kLcOpenclLlvmObjdumpExecutable);

    if (!user_bin_dir.empty())
    {
        gtString bin_folder;
        bin_folder << user_bin_dir.c_str();
        lc_objdump_exec.setFileDirectory(bin_folder);
        lc_objdump_exec.setFileName(objdump_exec_name);
        lc_objdump_exec.setFileExtension(kLcCompilerExecutableExtension);
    }
    else
    {
        osGetCurrentApplicationPath(lc_objdump_exec, false);
        lc_objdump_exec.appendSubDirectory(kLcOpenclRootDir);
        lc_objdump_exec.appendSubDirectory(kLcOpenclBinDir);
        lc_objdump_exec.setFileName(objdump_exec_name);
        lc_objdump_exec.setFileExtension(kLcCompilerExecutableExtension);
    }

    KcUtils::ProcessStatus status = KcUtils::LaunchProcess(lc_objdump_exec.asString().asASCIICharArray(),
        cmd_line_options,
        "",
        kObjdumpExecTimeoutMs,
        should_print_cmd,
        out_text,
        error_text,
        exit_code);

    return (status == KcUtils::ProcessStatus::kSuccess ? beKA::kBeStatusSuccess : beKA::kBeStatusLightningObjDumpLaunchFailed);
}

beKA::beStatus BeProgramBuilderLightning::VerifyObjDumpOutput(const std::string& objdump_out)
{
    return (objdump_out.find(kStrLcIsaDisassemblyToken) == std::string::npos ?
            beKA::kBeStatusLightningDisassembleFailed : beKA::kBeStatusSuccess);
}

beKA::beStatus BeProgramBuilderLightning::FilterCodeObjOutput(ObjDumpOp op, std::string& text)
{
    if (op != ObjDumpOp::kGetMetadata)
    {
        size_t code_offset = text.find(kStrLcIsaDisassemblyToken);
        if (code_offset == std::string::npos)
        {
            return beKA::beStatus::kBeStatusInvalid;
        }
        code_offset += kStrLcIsaDisassemblyToken.size();
        text = text.substr(code_offset, text.size() - code_offset);
    }

    return beKA::kBeStatusSuccess;
}

bool BeProgramBuilderLightning::DoesReadobjSupportMetadata(const std::string& user_bin_dir, bool should_print_cmd)
{
    std::string  out, err;
    bool ret = false;

    if (InvokeObjDump(ObjDumpOp::kGetMetadata, user_bin_dir, kStrLcObjDumpSwitchHelp, should_print_cmd, out, err) == beKA::beStatus::kBeStatusSuccess)
    {
        ret = (out.find(kStrLcObjDumpMetdataOptionToken) != std::string::npos);
    }

    return ret;
}

static bool GetIsaSize(const std::string& isa_text, const std::string& kernel_name, size_t& size_in_bytes)
{
    // Example of the ISA instruction in the disassembled ISA text:
    //
    // s_load_dword s4, s[6:7], 0x10          // 000000001118: C0020103 00000010
    //                                           `-- addr --'  `-- inst code --'

    const std::string kKernelEndToken    = "\n\n";
    const std::string kAddressCommentPrefix = "// ";
    const std::string kAddressCodeDelimiter = ":";

    const unsigned int kInstructionCodeLength64 = 16;
    const unsigned int kInstructionSize32     =  4;
    const unsigned int kInstructionSize64     =  8;

    bool status = true;
    size_t kernel_isa_begin, kernel_isa_end, address_begin, address_end;
    kernel_isa_begin = kernel_isa_end = address_begin = address_end = 0;
    std::string kernel_isa_text;
    uint64_t instruction_address_first, instruction_address_last;

    status = (!isa_text.empty() && !kernel_name.empty());
    size_in_bytes = 0;

    // Get the ISA text for the required kernel.
    if (status)
    {
        if (status = (kernel_isa_begin = isa_text.find(kernel_name + kAddressCodeDelimiter)) != std::string::npos)
        {
            if ((kernel_isa_end = isa_text.find(kKernelEndToken, kernel_isa_begin)) == std::string::npos)
            {
                kernel_isa_end = isa_text.size();
            }
        }
    }

    if (status)
    {
        kernel_isa_text = isa_text.substr(kernel_isa_begin, kernel_isa_end - kernel_isa_begin);
        address_begin = kernel_isa_text.find(kAddressCommentPrefix);
    }

    // Find the beginning and the end of the first instruction address.
    if (status && address_begin != std::string::npos)
    {
        address_begin += kAddressCommentPrefix.size();
        status = (address_end = kernel_isa_text.find(kAddressCodeDelimiter, address_begin)) != std::string::npos;
    }

    // Parse the address of the 1st instruction.
    if (status)
    {
        try
        {
            instruction_address_first = std::stoull(kernel_isa_text.substr(address_begin, address_end - address_begin), nullptr, 16);
        }
        catch (const std::invalid_argument&)
        {
            status = false;
        }
    }

    // Find the beginning and the end of the last instruction address.
    if (status)
    {
        status = (address_end = kernel_isa_text.rfind(kAddressCodeDelimiter)) != std::string::npos;
        if (status)
        {
            status = (address_begin = kernel_isa_text.rfind(kAddressCommentPrefix, address_end)) != std::string::npos;
        }
    }

    // Parse the address of the last instruction.
    if (status)
    {
        try
        {
            address_begin += kAddressCommentPrefix.size();
            instruction_address_last = std::stoull(kernel_isa_text.substr(address_begin, address_end - address_begin), nullptr, 16);
        }
        catch (const std::invalid_argument&)
        {
            status = false;
        }
    }

    // ISA size = address_of_last_instruction - address_of_1st_instruction + size_of_last_instruction.
    if (status)
    {
        size_t  lastInstSize = ( (kernel_isa_text.size() - address_end) >= kInstructionCodeLength64 ? kInstructionSize64 : kInstructionSize32 );
        size_in_bytes = static_cast<size_t>(instruction_address_last - instruction_address_first + lastInstSize);
    }

    return status;
}

// Parse an integer YAML node for CodeProps value.
// Do nothing if "oldResult" is false.
// If "zero_if_absent" is true, the value is set to 0 and "true" is returned if the required value is not found in the MD.
// (The Lightning Compiler does not generate some CodeProps values if they are 0).
static bool ParseCodePropsItem(const YAML::Node& code_props, const std::string& key, size_t& value, bool zero_if_absent = false)
{
    bool result = false;
    const YAML::Node& prop_metadata_node = code_props[key];
    if ((result = prop_metadata_node.IsDefined()) == true)
    {
        try
        {
            value = prop_metadata_node.as<size_t>();
        }
        catch (const YAML::TypedBadConversion<size_t>&)
        {
            result = false;
        }
    }
    else
    {
        if (zero_if_absent)
        {
            value = 0;
            result = true;
        }
    }

    return result;
}

// Parse a single YAML node for kernel metadata.
static bool ParseKernelCodeProps(const YAML::Node& kernel_metadata, KernelCodeProperties& code_props)
{
    bool result = ParseCodePropsItem(kernel_metadata, kStrCodeObjectMetadataKeyWavefrontSgprs, code_props.wavefront_num_sgprs, true);
    result = result && ParseCodePropsItem(kernel_metadata, kStrCodeObjectMetadataKeyWorkitemVgprs,  code_props.work_item_num_vgprs, true);
    result = result && ParseCodePropsItem(kernel_metadata, kStrCodeObjectMetadataKeyWavefrontSize,  code_props.wavefront_size);
    result = result && ParseCodePropsItem(kernel_metadata, kStrCodeObjectMetadataKeyGroupSegmentSize, code_props.workgroup_segment_size, true);
    result = result && ParseCodePropsItem(kernel_metadata, kStrCodeObjectMetadataKeyPrivateSegmentSize, code_props.private_segment_size, true);
    result = result && ParseCodePropsItem(kernel_metadata, kStrCodeObjectMetadataKeySpilledSgprs, code_props.sgpr_spills, true);
    result = result && ParseCodePropsItem(kernel_metadata, kStrCodeObjectMetadataKeySpilledVgprs, code_props.vgpr_spills, true);

    return result;
}

// Parse the provided CodeObj metadata and extract CodeProps data for all kernels.
static beKA::beStatus ParseCodeProps(const std::string& metadata_text, CodePropsMap& code_props)
{
    beKA::beStatus status = beKA::beStatus::kBeStatusSuccess;
    size_t start_offset, end_offset;
    YAML::Node codeobj_metadata_node, kernels_metadata_map, kernel_name;
    start_offset = metadata_text.find(kStrLcCodeObjectMetadataTokenStart);

    while ((end_offset = metadata_text.find(kStrLcCodeObjectMetadataTokenEnd, start_offset)) != std::string::npos)
    {
        try
        {
            const std::string& kernel_metadata_text =
                metadata_text.substr(start_offset, end_offset - start_offset + kStrLcCodeObjectMetadataTokenEnd.size());
            codeobj_metadata_node = YAML::Load(kernel_metadata_text);
        }
        catch (YAML::ParserException&)
        {
            status = beKA::beStatus::kBeStatusLightningParseCodeObjMDFailed;
            break;
        }

        if (codeobj_metadata_node.IsMap() &&
            (kernels_metadata_map = codeobj_metadata_node[kStrCodeObjectMetadataKeyKernels]).IsDefined())
        {
            for (const YAML::Node& kernel_metadata : kernels_metadata_map)
            {
                KernelCodeProperties kernel_code_props;
                if (status == beKA::beStatus::kBeStatusSuccess &&
                    (kernel_name = kernel_metadata[kStrCodeObjectMetadataKeyKernelName]).IsDefined() &&
                    ParseKernelCodeProps(kernel_metadata, kernel_code_props))
                {
                    code_props[kernel_name.as<std::string>()] = kernel_code_props;
                }
                else
                {
                    status = beKA::beStatus::kBeStatusLightningParseCodeObjMDFailed;
                    break;
                }
            }
        }
        start_offset = metadata_text.find(kStrLcCodeObjectMetadataTokenStart, end_offset);
    }

    return status;
}

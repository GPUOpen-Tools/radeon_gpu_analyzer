
// Infra
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osApplication.h>

// Yaml
#include <yaml-cpp/yaml.h>

// Local
#include <RadeonGPUAnalyzerCLI/src/kcUtils.h>
#include <RadeonGPUAnalyzerBackend/include/beUtils.h>
#include <RadeonGPUAnalyzerBackend/include/beStringConstants.h>
#include <RadeonGPUAnalyzerBackend/emulator/Parser/ISAParser.h>
#include "../include/beProgramBuilderLightning.h"

// *****************************************
// *** INTERNALLY LINKED SYMBOLS - START ***
// *****************************************

// The include and lib folders are relative to the LC_OPENCL_ROOT_FOLDER.
const gtString  LC_OPENCL_BIN_DIR     = L"bin";
const gtString  LC_OPENCL_INCLUDE_DIR = L"include";
const gtString  LC_OPENCL_LIB_DIR     = L"lib/bitcode";

const gtString               LC_OPENCL_INCLUDE_FILE = L"opencl-c.h";
const std::vector<gtString>  LC_OPENCL_LIB_FILES    = { L"opencl.amdgcn.bc", L"ockl.amdgcn.bc", L"irif.amdgcn.bc", L"ocml.amdgcn.bc",
                                                        L"oclc_correctly_rounded_sqrt_on.amdgcn.bc", L"oclc_correctly_rounded_sqrt_off.amdgcn.bc",
                                                        L"oclc_daz_opt_on.amdgcn.bc", L"oclc_daz_opt_off.amdgcn.bc",
                                                        L"oclc_unsafe_math_on.amdgcn.bc", L"oclc_unsafe_math_off.amdgcn.bc",
                                                        L"oclc_finite_only_on.amdgcn.bc", L"oclc_finite_only_off.amdgcn.bc",
                                                        L"oclc_isa_version_900.amdgcn.bc" };

const std::string  LC_OPENCL_STD_OPTION          = "-cl-std";
const std::string  LC_OPENCL_STD_DEFAULT_VALUE   = "cl2.0";

const std::string  LC_OPENCL_DEFS                = "-D__OPENCL_VERSION__=200";

const std::string  COMPILER_ERROR_TOKEN          = "error:";
const std::string  ISA_DISASM_TEXT_TOKEN         = "Disassembly of section .text:";
const std::string  CODEOBJ_METADATA_START_TOKEN  = "---\n";
const std::string  CODEOBJ_METADATA_END_TOKEN    = "\n...";

const std::string  OBJDUMP_METADATA_OPTION_TOKEN = "-amdgpu-code-object-metadata";

const std::string  COMPILER_OCL_TRIPLE_SWITCH    = "--target=amdgcn-amd-amdhsa-amdgizcl";
const std::string  COMPILER_OCL_INCLUDE_SWITCH   = "-include ";
const std::string  COMPILER_OCL_LIB_SWITCH       = "-Xclang -mlink-bitcode-file -Xclang ";
const std::string  COMPILER_DEVICE_SWITCH        = "-mcpu=";
const std::string  COMPILER_VERSION_SWITCH       = "--version";
const std::string  COMPILER_PREPROC_SWITCH       = "-E";
const std::string  COMPILER_DUMP_IL_SWITCH       = "-mllvm --print-after-all";
const std::string  COMPILER_DEBUG_INFO_SWITCH    = "-g";
const std::string  COMPILER_OPT_LEVEL_SWITCH     = "-O";

const std::string  OBJDUMP_DEVICE_SWITCH         = "-mcpu=";
const std::string  OBJDUMP_DISASM_SWITCH         = "-disassemble";
const std::string  OBJDUMP_DISASM_LINENUM_SWITCH = "-disassemble -line-numbers -source";
const std::string  OBJDUMP_METADATA_SWITCH_1     = "-amdgpu-code-object-metadata -elf-output-style=GNU -notes";
const std::string  OBJDUMP_METADATA_SWITCH_2     = "-elf-output-style=GNU -notes";
const std::string  OBJDUMP_TRIPLE_SWITCH         = "-triple=amdgcn-amd-amdhsa";
const std::string  OBJDUMP_SYMBOLS_SWITCH        = "-symbols";
const std::string  OBJDUMP_HELP_SWITCH           = "--help";

// CodeObject MetaData keys
const std::string  CODE_OBJ_MD_KERNELS_KEY       = "Kernels";
const std::string  CODE_OBJ_MD_KERNEL_NAME_KEY   = "Name";
const std::string  CODE_OBJ_MD_CODE_PROPS_KEY    = "CodeProps";
const std::string  CODE_OBJ_MD_WAVEFRONT_SGRPS   = "NumSGPRs";
const std::string  CODE_OBJ_MD_WORKITEM_VGPRS    = "NumVGPRs";
const std::string  CODE_OBJ_MD_WAVEFRONT_SIZE    = "WavefrontSize";
const std::string  CODE_OBJ_MD_SPILLED_SGPRS     = "NumSpilledSGPRs";
const std::string  CODE_OBJ_MD_SPILLED_VGPRS     = "NumSpilledVGPRs";
const std::string  CODE_OBJ_MD_GROUP_SEGMENT_SIZE   = "GroupSegmentFixedSize";
const std::string  CODE_OBJ_MD_PRIVATE_SEGMENT_SIZE = "PrivateSegmentFixedSize";

// Readobj symbols output keys
const std::string  BINARY_SYMBOLS_KEY            = "Symbols [";
const std::string  BINARY_SYMBOL_KEY             = "Symbol {";
const std::string  BINARY_SYMBOL_NAME_KEY        = "Name: ";
const std::string  BINARY_SYMBOL_NAME_END_KEY    = " (";
const std::string  BINARY_SYMBOL_SIZE_KEY        = "Size: ";

// ***************************************
// *** INTERNALLY LINKED SYMBOLS - END ***
// ***************************************

// Constants

static const  unsigned long  PROCESS_WAIT_INFINITE             = 0xFFFFFFFF;

static const  unsigned long  RGA_ROCM_COMPILER_EXEC_TIMEOUT_MS = PROCESS_WAIT_INFINITE;
static const  unsigned long  RGA_ROCM_PREPROC_TIMEOUT_MS       = PROCESS_WAIT_INFINITE;
static const  unsigned long  RGA_ROCM_OBJDUMP_EXEC_TIMEOUT_MS  = PROCESS_WAIT_INFINITE;

static const  unsigned int   RGA_ROCM_COMPILER_AKC_SIZE        = 256;

static bool GetIsaSize(const string& isaAsText, const std::string& kernelName, size_t& sizeInBytes);
static beKA::beStatus  ParseCodeProps(const std::string & MDText, CodePropsMap& codeProps);

beKA::beStatus beProgramBuilderLightning::GetBinary(const std::string & device, const beKA::BinaryOptions & binopts, std::vector<char>& binary)
{
    GT_UNREFERENCED_PARAMETER(device);
    GT_UNREFERENCED_PARAMETER(binopts);
    GT_UNREFERENCED_PARAMETER(binary);
    return beKA::beStatus();
}

beKA::beStatus beProgramBuilderLightning::GetKernelILText(const std::string & device, const std::string & kernel, std::string & il)
{
    GT_UNREFERENCED_PARAMETER(device);
    GT_UNREFERENCED_PARAMETER(kernel);
    GT_UNREFERENCED_PARAMETER(il);
    return beKA::beStatus();
}

beKA::beStatus beProgramBuilderLightning::GetKernelISAText(const std::string & device, const std::string & kernel, std::string & isa)
{
    GT_UNREFERENCED_PARAMETER(device);
    GT_UNREFERENCED_PARAMETER(kernel);
    GT_UNREFERENCED_PARAMETER(isa);
    return beKA::beStatus();
}

beKA::beStatus beProgramBuilderLightning::GetStatistics(const std::string & device, const std::string & kernel, beKA::AnalysisData & analysis)
{
    GT_UNREFERENCED_PARAMETER(device);
    GT_UNREFERENCED_PARAMETER(kernel);
    GT_UNREFERENCED_PARAMETER(analysis);
    return beKA::beStatus();
}

beKA::beStatus beProgramBuilderLightning::GetDeviceTable(std::vector<GDT_GfxCardInfo>& table)
{
    GT_UNREFERENCED_PARAMETER(table);
    return beKA::beStatus();
}

beKA::beStatus beProgramBuilderLightning::GetCompilerVersion(SourceLanguage lang, const std::string& userBinFolder, bool printCmd, std::string& outText)
{
    std::string  errText;

    beKA::beStatus  status = InvokeCompiler(lang, userBinFolder, COMPILER_VERSION_SWITCH, printCmd, outText, errText);

    return status;
}

beKA::beStatus beProgramBuilderLightning::AddCompilerStandardOptions(beKA::SourceLanguage lang, const CmplrPaths& cmplrPaths, std::string& options)
{
    std::stringstream  optsStream;
    beKA::beStatus  status = beKA::beStatus_SUCCESS;

    if (lang == beKA::SourceLanguage_Rocm_OpenCL)
    {
        // Add target triple.
        optsStream << COMPILER_OCL_TRIPLE_SWITCH;

        // Add OpenCL include required by ROCm OpenCL compiler.
        osFilePath  inclFilePath;
        if (!cmplrPaths.m_inc.empty())
        {
            gtString  incDir;
            incDir << cmplrPaths.m_inc.c_str();
            inclFilePath.setFileDirectory(incDir);
        }
        else
        {
            osGetCurrentApplicationPath(inclFilePath, false);
            inclFilePath.clearFileExtension();
            inclFilePath.appendSubDirectory(LC_OPENCL_ROOT_DIR);
            inclFilePath.appendSubDirectory(LC_OPENCL_INCLUDE_DIR);
        }

        inclFilePath.setFileName(LC_OPENCL_INCLUDE_FILE);
        optsStream << " " << COMPILER_OCL_INCLUDE_SWITCH << kcUtils::Quote(inclFilePath.asString().asASCIICharArray());

        // Add OpenCL device libs.
        osFilePath  libFilePath;
        if (!cmplrPaths.m_lib.empty())
        {
            gtString  libDir;
            libDir << cmplrPaths.m_lib.c_str();
            libFilePath.setFileDirectory(libDir);
        }
        else
        {
            osGetCurrentApplicationPath(libFilePath, false);
            libFilePath.clearFileExtension();
            libFilePath.appendSubDirectory(LC_OPENCL_ROOT_DIR);
            libFilePath.appendSubDirectory(LC_OPENCL_LIB_DIR);
        }

        for (const gtString & libFile : LC_OPENCL_LIB_FILES)
        {
            libFilePath.setFileName(libFile);
            optsStream << " " << COMPILER_OCL_LIB_SWITCH << kcUtils::Quote(libFilePath.asString().asASCIICharArray());
        }
    }
    else
    {
        status = beKA::beStatus_UnknownInputLang;
    }
    options = optsStream.str();

    return status;
}

beKA::beStatus beProgramBuilderLightning::ConstructOpenCLCompilerOptions(const CmplrPaths&                 cmplrPaths,
                                                                         const OpenCLOptions&              userOptions,
                                                                         const std::vector<std::string>&   srcFileNames,
                                                                         const std::string&                binFileName,
                                                                         const std::string&                device,
                                                                         std::string&                      outOptions)
{
    beKA::beStatus status;
    std::string  standardOptions = "";
    status = AddCompilerStandardOptions(beKA::SourceLanguage_Rocm_OpenCL, cmplrPaths, standardOptions);
    if (status == beKA::beStatus_SUCCESS)
    {
        std::stringstream options;
        options << standardOptions;
        // Add options specifying the OpenCL standard.
        // Use default value if it is not provided in the user options.
        if (std::count_if(userOptions.m_openCLCompileOptions.begin(), userOptions.m_openCLCompileOptions.end(),
                          [&](const std::string& s) { return (s.find(LC_OPENCL_STD_OPTION) != std::string::npos); }) == 0)
        {
            options << " " << LC_OPENCL_STD_OPTION << "=" << LC_OPENCL_STD_DEFAULT_VALUE;
            options << " " << LC_OPENCL_DEFS;
        }

        // Specify optimization level if required.
        if (userOptions.m_optLevel != -1)
        {
            options << " " << COMPILER_OPT_LEVEL_SWITCH << userOptions.m_optLevel;
        }

        // Add the source debug info switch.
        if (userOptions.m_lineNumbers)
        {
            options << " " << COMPILER_DEBUG_INFO_SWITCH;
        }

        // Add the device selection options.
        options << " " << COMPILER_DEVICE_SWITCH << device;

        // Enable IL dump if requested.
        if (userOptions.m_dumpIL)
        {
            options << " " << COMPILER_DUMP_IL_SWITCH;
        }

        // Add the source and output file options.
        for (const std::string & srcName : srcFileNames)
        {
            options << " " << kcUtils::Quote(srcName);
        }
        options << " -o " << kcUtils::Quote(binFileName);

        // Add user-provided options
        for (const std::string & option : userOptions.m_openCLCompileOptions)
        {
            options << " " << option;
        }

        // Add include paths provided by user.
        for (const std::string& incPath : userOptions.m_incPaths)
        {
            options << " -I" << kcUtils::Quote(incPath);
        }

        // Add the definitions
        for (const std::string & def : userOptions.m_defines)
        {
            size_t  asgnOffset = def.find('=');
            options << " -D" << (asgnOffset == std::string::npos ? def : def.substr(0, asgnOffset + 1) + kcUtils::Quote(def.substr(asgnOffset + 1)));
        }
        outOptions = options.str();
    }

    return status;
}

beKA::beStatus beProgramBuilderLightning::CompileOpenCLToBinary(const CmplrPaths&                cmplrPaths,
                                                                const OpenCLOptions&             userOptions,
                                                                const std::vector<std::string>&  srcFileNames,
                                                                const std::string&               binFileName,
                                                                const std::string&               device,
                                                                bool                             printCmd,
                                                                std::string&                     errText)
{
    // Generate the compiler command line options string.
    beKA::beStatus  status = beKA::beStatus_SUCCESS;
    std::string  options = "";
    std::string  outText;

    status = ConstructOpenCLCompilerOptions(cmplrPaths, userOptions, srcFileNames, binFileName, device, options);
    if (status == beKA::beStatus_SUCCESS)
    {
        // Run LC compiler.
        status = InvokeCompiler(beKA::SourceLanguage_Rocm_OpenCL, cmplrPaths.m_bin, options, printCmd, outText, errText);
    }

    if (status == beKA::beStatus_SUCCESS)
    {
        status = VerifyCompilerOutput(binFileName, errText);
    }

    return status;
}


beKA::beStatus beProgramBuilderLightning::InvokeCompiler(beKA::SourceLanguage   lang,
                                                         const std::string    & userBinFolder,
                                                         const std::string    & cmdLineOptions,
                                                         bool                   printCmd,
                                                         std::string          & stdOut,
                                                         std::string          & stdErr,
                                                         unsigned long          timeOut)
{
    beKA::beStatus  status = beKA::beStatus_SUCCESS;
    osFilePath      lcCompilerExec;
    long            exitCode;

    // Use default timeout if not specified.
    if (timeOut == 0)
    {
        timeOut = RGA_ROCM_COMPILER_EXEC_TIMEOUT_MS;
    }

    // Select the compiler executable
    if (!userBinFolder.empty())
    {
        gtString  binFolder;
        binFolder << userBinFolder.c_str();
        lcCompilerExec.setFileDirectory(binFolder);
        lcCompilerExec.setFileName(LC_OPENCL_COMPILER_EXEC);
        lcCompilerExec.setFileExtension(LC_COMPILER_EXEC_EXT);
    }
    else
    {
        if (lang == beKA::SourceLanguage::SourceLanguage_Rocm_OpenCL)
        {
            osGetCurrentApplicationPath(lcCompilerExec, false);
            lcCompilerExec.appendSubDirectory(LC_OPENCL_ROOT_DIR);
            lcCompilerExec.appendSubDirectory(LC_OPENCL_BIN_DIR);
            lcCompilerExec.setFileName(LC_OPENCL_COMPILER_EXEC);
            lcCompilerExec.setFileExtension(LC_COMPILER_EXEC_EXT);
        }
        else
        {
            return beKA::beStatus_UnknownInputLang;
        }
    }

    kcUtils::ProcessStatus  procStatus = kcUtils::LaunchProcess(lcCompilerExec.asString().asASCIICharArray(),
                                                                cmdLineOptions,
                                                                "",
                                                                timeOut,
                                                                printCmd,
                                                                stdOut,
                                                                stdErr,
                                                                exitCode);

    switch (procStatus)
    {
    case kcUtils::ProcessStatus::LaunchFailed:
    case kcUtils::ProcessStatus::CreateTempFileFailed:
    case kcUtils::ProcessStatus::ReadTempFileFailed:
        status = beKA::beStatus_LC_CompilerLaunchFailed;
        break;
    case kcUtils::ProcessStatus::TimeOut:
        status = beKA::beStatus_LC_CompilerTimeOut;
        break;
    }

    // Check the process exit status.
    if (status == beStatus_SUCCESS)
    {
        status = (exitCode == 0L ? beKA::beStatus_SUCCESS : beKA::beStatus_LC_CompilerGeneratedError);
    }

    return status;
}

beKA::beStatus beProgramBuilderLightning::VerifyCompilerOutput(const std::string & outFileName, const std::string & errText)
{
    gtString    outFileNameWChar;
    outFileNameWChar.fromASCIIString(outFileName.c_str());
    osFilePath  outFilePath(outFileNameWChar);
    beStatus    status = beStatus_SUCCESS;

    if (outFileName.empty() || !outFilePath.exists())
    {
        status = errText.find(COMPILER_ERROR_TOKEN) != std::string::npos ?
                     beStatus_LC_CompilerGeneratedError : (outFileName.empty() ? beStatus_SUCCESS : beStatus_NoOutputFileGenerated);
    }

    return status;
}

beStatus beProgramBuilderLightning::PreprocessOpenCL(const std::string& userBinDir, const std::string& inputFile,
                                                     const std::string& args, bool printCmd, std::string& output)
{
    std::stringstream  compilerArgs;
    compilerArgs << COMPILER_PREPROC_SWITCH << " " << args << " " << kcUtils::Quote(inputFile);
    compilerArgs << " " << LC_OPENCL_STD_OPTION << "=" << LC_OPENCL_STD_DEFAULT_VALUE;
    compilerArgs << " " << LC_OPENCL_DEFS;
    std::string  stdOut, stdErr;

    beStatus  status = InvokeCompiler(SourceLanguage::SourceLanguage_Rocm_OpenCL, userBinDir, compilerArgs.str(),
                                      printCmd, stdOut, stdErr, RGA_ROCM_PREPROC_TIMEOUT_MS);

    if (status == beStatus_SUCCESS)
    {
        status = VerifyCompilerOutput("", stdErr);
    }

    if (status == beStatus_SUCCESS)
    {
        output = stdOut;
    }
    else
    {
        output = stdErr;
    }

    return status;
}

beKA::beStatus beProgramBuilderLightning::DisassembleBinary(const std::string& userBinDir,
                                                            const std::string& binFileName,
                                                            const std::string& device,
                                                            bool               lineNumbers,
                                                            bool               printCmd,
                                                            std::string&       outISAText,
                                                            std::string&       errText)
{
    beKA::beStatus  status = beKA::beStatus_SUCCESS;
    gtString  binFileNameWchar;
    binFileNameWchar.fromASCIIString(binFileName.c_str());

    if (osFilePath(binFileNameWchar).exists())
    {
        std::string  objDumpOptions = "";
        outISAText = "";
        ObjDumpOp  op = (lineNumbers ? ObjDumpOp::DisassembleWithLineNums : ObjDumpOp::Disassemble);
        beKA::beStatus status = ConstructObjDumpOptions(op, userBinDir, binFileName, device, printCmd, objDumpOptions);

        if (status == beKA::beStatus_SUCCESS)
        {
            status = InvokeObjDump(ObjDumpOp::Disassemble, userBinDir, objDumpOptions, printCmd, outISAText, errText);
        }

        if (status == beKA::beStatus_SUCCESS)
        {
            status = VerifyObjDumpOutput(outISAText);
        }

        if (status == beKA::beStatus_SUCCESS)
        {
            status = FilterCodeObjOutput(ObjDumpOp::Disassemble, outISAText);
        }
    }
    else
    {
        status = beKA::beStatus_NO_BINARY_FOR_DEVICE;
    }

    return status;
}

beKA::beStatus beProgramBuilderLightning::ExtractMetadata(const std::string& userBinDir, const std::string & binFileName,
                                                          bool printCmd, std::string& metadataText)
{
    beKA::beStatus  status = beKA::beStatus_LC_ExtractMetadataFailed;
    std::string  readObjOutput, options, errText;

    // Launch the LC ReadObj.
    status = ConstructObjDumpOptions(ObjDumpOp::GetMetadata, userBinDir, binFileName, "", printCmd, options);

    if (status == beKA::beStatus_SUCCESS)
    {
        status = InvokeObjDump(ObjDumpOp::GetMetadata, userBinDir, options, printCmd, readObjOutput, errText);
    }

    if (status == beKA::beStatus_SUCCESS)
    {
        metadataText = readObjOutput;
    }

    return status;
}

beStatus beProgramBuilderLightning::ExtractKernelCodeProps(const std::string& userBinDir, const std::string& binFileName,
                                                           bool printCmd, CodePropsMap& codeProps)
{
    beKA::beStatus  status = beKA::beStatus_LC_ExtractCodePropsFailed;
    std::string  metadataText;

    if ((status = ExtractMetadata(userBinDir, binFileName, printCmd, metadataText)) == beStatus_SUCCESS)
    {
        status = ParseCodeProps(metadataText, codeProps);
    }

    return status;
}

beKA::beStatus beProgramBuilderLightning::ExtractKernelNames(const std::string& userBinDir, const std::string& binFileName,
                                                             bool printCmd, std::vector<std::string>& kernelNames)
{
    beKA::beStatus  status = beKA::beStatus_SUCCESS;
    std::string  metadata, options, errText;

    // Launch the LC ReadObj and parse its output.
    status = ConstructObjDumpOptions(ObjDumpOp::GetMetadata, userBinDir, binFileName, "", printCmd, options);

    if (status == beKA::beStatus_SUCCESS)
    {
        status = InvokeObjDump(ObjDumpOp::GetMetadata, userBinDir, options, printCmd, metadata, errText);
    }

    if (status == beKA::beStatus_SUCCESS)
    {
        YAML::Node  codeObjMDNode, kernelsMDMap, kernelName;
        size_t  startOffset = 0, endOffset;

        startOffset = metadata.find(CODEOBJ_METADATA_START_TOKEN);

        // Load all Metadata nodes found in the ObjDump output.
        while ((endOffset = metadata.find(CODEOBJ_METADATA_END_TOKEN, startOffset)) != std::string::npos)
        {
            try
            {
                codeObjMDNode = YAML::Load(metadata.substr(startOffset, endOffset - startOffset + CODEOBJ_METADATA_END_TOKEN.size()));
            }
            catch (YAML::ParserException&)
            {
                status = beKA::beStatus_LC_ExtractKernelNamesFailed;
            }

            if (status == beKA::beStatus_SUCCESS && codeObjMDNode.IsMap())
            {
                // Look for kernels metadata.
                if (status == beKA::beStatus_SUCCESS && (kernelsMDMap = codeObjMDNode[CODE_OBJ_MD_KERNELS_KEY]).IsDefined())
                {
                    for (const YAML::Node& kernelMD : kernelsMDMap)
                    {
                        if (status == beKA::beStatus_SUCCESS && (kernelName = kernelMD[CODE_OBJ_MD_KERNEL_NAME_KEY]).IsDefined())
                        {
                            kernelNames.push_back(kernelName.as<std::string>());
                        }
                        else
                        {
                            status = beKA::beStatus_LC_ExtractKernelNamesFailed;
                            break;
                        }
                    }
                }
            }
            startOffset = metadata.find(CODEOBJ_METADATA_START_TOKEN, endOffset);
        }
    }

    return status;
}

bool beProgramBuilderLightning::VerifyOutputFile(const std::string & fileName)
{
    bool  ret = beUtils::isFilePresent(fileName);

    return ret;
}

int beProgramBuilderLightning::GetIsaSize(const std::string & isaText)
{
    ParserISA isaParser;
    int  isaSize = 0;

    if (isaParser.ParseForSize(isaText))
    {
        isaSize = isaParser.GetCodeLen();
    }

    return isaSize;
}

int beProgramBuilderLightning::GetKernelCodeSize(const std::string & userBinDir, const std::string & binFile,
                                                 const std::string & kernelName, bool printCmd)
{
    beKA::beStatus  status = beStatus_LC_GetKernelCodeSizeFailed;
    std::string     symbols, options, errText;
    int             ret = -1, symSize;
    size_t          offset, nameOffset, nameEndOffset, sizeOffset;

    // Launch the LC ReadObj.
    status = ConstructObjDumpOptions(ObjDumpOp::GetKernelCodeSize, userBinDir, binFile, "", printCmd, options);

    if (status == beStatus_SUCCESS)
    {
        status = InvokeObjDump(ObjDumpOp::GetKernelCodeSize, userBinDir, options, printCmd, symbols, errText);
    }

    // Parse the readobj output.
    // readobj uses its own format, so we have to parse it manually.
    //
    //    File: gfx900_test.bin
    //    Format : ELF64 - amdgpu
    //    Arch : amdgcn
    //    AddressSize : 64bit
    //    LoadName :
    //    Symbols[
    //      Symbol{
    //        Name: foo (387)
    //        Value : 0x1000
    //        Size : 376
    //        Binding : Global(0x1)
    //        Type : AMDGPU_HSA_KERNEL(0xA)
    //        Other : 0
    //        Section : .text(0x6)
    //      }
    //    ]

    if (status == beStatus_SUCCESS)
    {
        if ((offset = symbols.find(BINARY_SYMBOLS_KEY)) != std::string::npos)
        {
            bool  stop = false;
            while (!stop && (offset = symbols.find(BINARY_SYMBOL_KEY, offset)) != std::string::npos)
            {
                if ((nameOffset = symbols.find(BINARY_SYMBOL_NAME_KEY, offset)) != std::string::npos &&
                    (sizeOffset = symbols.find(BINARY_SYMBOL_SIZE_KEY, nameOffset)) != std::string::npos &&
                    (nameEndOffset = symbols.find(BINARY_SYMBOL_NAME_END_KEY, nameOffset)) != std::string::npos)
                {
                    nameOffset += BINARY_SYMBOL_NAME_KEY.size();
                    if (symbols.substr(nameOffset, nameEndOffset - nameOffset) == kernelName)
                    {
                        if ((symSize = std::atoi(symbols.c_str() + sizeOffset + BINARY_SYMBOL_SIZE_KEY.size())) != 0)
                        {
                            ret = symSize - RGA_ROCM_COMPILER_AKC_SIZE;
                        }
                        stop = true;
                    }
                    offset = sizeOffset;
                }
            }
        }
    }

    return ret;
}

beKA::beStatus beProgramBuilderLightning::ConstructObjDumpOptions(ObjDumpOp          op,
                                                                  const std::string& compilerBinDir,
                                                                  const std::string& binFileName,
                                                                  const std::string& device,
                                                                  bool               printCmd,
                                                                  std::string&       options)
{
    std::stringstream  allOptions;
    std::string        opSelector;

    switch (op)
    {
    case ObjDumpOp::Disassemble:
        opSelector = OBJDUMP_DISASM_SWITCH;
        break;
    case ObjDumpOp::DisassembleWithLineNums:
        opSelector = OBJDUMP_DISASM_LINENUM_SWITCH;
        break;
    case ObjDumpOp::GetMetadata:
        // Use OBJDUMP_METADATA_SWITCH_2 option set for LLVM versions >= 6 and unknown version (0).
        opSelector = ReadobjSupportsGetMDOption(compilerBinDir, printCmd) ? OBJDUMP_METADATA_SWITCH_1 : OBJDUMP_METADATA_SWITCH_2;
        break;
    case ObjDumpOp::GetKernelCodeSize:
        opSelector = OBJDUMP_SYMBOLS_SWITCH;
        break;
    default:
        return beKA::beStatus_UnknownObjDumpOperation;
    }

    allOptions << opSelector;
    // Add the patform triple and device for disassemble.
    if (op == ObjDumpOp::Disassemble || op == ObjDumpOp::DisassembleWithLineNums)
    {
        allOptions << " " << OBJDUMP_TRIPLE_SWITCH << " " << OBJDUMP_DEVICE_SWITCH << device;
    }
    // Add the binary file to process.
    allOptions << " " << kcUtils::Quote(binFileName);

    options = allOptions.str();

    return beKA::beStatus_SUCCESS;
}

beKA::beStatus beProgramBuilderLightning::InvokeObjDump(ObjDumpOp op, const std::string& userBinDir, const std::string& cmdLineOptions,
                                                        bool printCmd, std::string& outText, std::string& errText)
{
    osFilePath  lcObjDumpExec;
    long        exitCode;
    // llvm-objdump is currently not able to extract the CodeObj Metadata, so use llvm-readobj instead.
    const gtString  objdumpExecName = ((op == ObjDumpOp::GetMetadata || op == ObjDumpOp::GetKernelCodeSize)
                                       ? LC_LLVM_READOBJ_EXEC : LC_LLVM_OBJDUMP_EXEC);

    if (!userBinDir.empty())
    {
        gtString  binFolder;
        binFolder << userBinDir.c_str();
        lcObjDumpExec.setFileDirectory(binFolder);
        lcObjDumpExec.setFileName(objdumpExecName);
        lcObjDumpExec.setFileExtension(LC_COMPILER_EXEC_EXT);
    }
    else
    {
        osGetCurrentApplicationPath(lcObjDumpExec, false);
        lcObjDumpExec.appendSubDirectory(LC_OPENCL_ROOT_DIR);
        lcObjDumpExec.appendSubDirectory(LC_OPENCL_BIN_DIR);
        lcObjDumpExec.setFileName(objdumpExecName);
        lcObjDumpExec.setFileExtension(LC_COMPILER_EXEC_EXT);
    }

    kcUtils::ProcessStatus  status = kcUtils::LaunchProcess(lcObjDumpExec.asString().asASCIICharArray(),
                                                            cmdLineOptions,
                                                            "",
                                                            RGA_ROCM_OBJDUMP_EXEC_TIMEOUT_MS,
                                                            printCmd,
                                                            outText,
                                                            errText,
                                                            exitCode);

    return (status == kcUtils::ProcessStatus::Success ? beKA::beStatus_SUCCESS : beKA::beStatus_LC_ObjDumpLaunchFailed);
}

beKA::beStatus beProgramBuilderLightning::VerifyObjDumpOutput(const std::string& objDumpOut)
{
    return (objDumpOut.find(ISA_DISASM_TEXT_TOKEN) == std::string::npos ?
            beKA::beStatus_LC_DisassembleFailed : beKA::beStatus_SUCCESS);
}

beKA::beStatus beProgramBuilderLightning::FilterCodeObjOutput(ObjDumpOp op, std::string& text)
{
    if (op == ObjDumpOp::GetMetadata)
    {
        // TODO
    }
    else
    {
        size_t  codeOffset = text.find(ISA_DISASM_TEXT_TOKEN);
        if (codeOffset == std::string::npos)
        {
            return beKA::beStatus::beStatus_Invalid;
        }
        codeOffset += ISA_DISASM_TEXT_TOKEN.size();
        text = text.substr(codeOffset, text.size() - codeOffset);
    }

    return beKA::beStatus_SUCCESS;
}

bool beProgramBuilderLightning::ReadobjSupportsGetMDOption(const std::string& userBinDir, bool printCmd)
{
    std::string  out, err;
    bool ret = false;

    if (InvokeObjDump(ObjDumpOp::GetMetadata, userBinDir, OBJDUMP_HELP_SWITCH, printCmd, out, err) == beStatus_SUCCESS)
    {
        ret = (out.find(OBJDUMP_METADATA_OPTION_TOKEN) != std::string::npos);
    }

    return ret;
}

static bool GetIsaSize(const std::string& isaText, const std::string& kernelName, size_t& sizeInBytes)
{
    // Example of the ISA instruction in the disassembled ISA text:
    //
    // s_load_dword s4, s[6:7], 0x10          // 000000001118: C0020103 00000010
    //                                           `-- addr --'  `-- inst code --'

    const std::string  KERNEL_END_TOKEN    = "\n\n";
    const std::string  ADDR_COMMENT_PREFIX = "// ";
    const std::string  ADDR_CODE_DELIMITER = ":";

    const unsigned int  INST_CODE_LEN_64 = 16;
    const unsigned int  INST_SIZE_32     =  4;
    const unsigned int  INST_SIZE_64     =  8;

    bool   status = true;
    size_t kernelIsaBegin, kernelIsaEnd, addrBegin, addrEnd;
    kernelIsaBegin = kernelIsaEnd = addrBegin = addrEnd = 0;
    std::string  kernelIsaText;
    uint64_t  firstInstAddr, lastInstAddr;

    status = (!isaText.empty() && !kernelName.empty());
    sizeInBytes = 0;

    // Get the ISA text for the required kernel.
    if (status)
    {
        if (status = (kernelIsaBegin = isaText.find(kernelName + ADDR_CODE_DELIMITER)) != std::string::npos)
        {
            if ((kernelIsaEnd = isaText.find(KERNEL_END_TOKEN, kernelIsaBegin)) == std::string::npos)
            {
                kernelIsaEnd = isaText.size();
            }
        }
    }

    if (status)
    {
        kernelIsaText = isaText.substr(kernelIsaBegin, kernelIsaEnd - kernelIsaBegin);
        addrBegin = kernelIsaText.find(ADDR_COMMENT_PREFIX);
    }

    // Find the beginning and the end of the first instruction address.
    if (status && addrBegin != string::npos)
    {
        addrBegin += ADDR_COMMENT_PREFIX.size();
        status = (addrEnd = kernelIsaText.find(ADDR_CODE_DELIMITER, addrBegin)) != std::string::npos;
    }

    // Parse the address of the 1st instruction.
    if (status)
    {
        try
        {
            firstInstAddr = std::stoull(kernelIsaText.substr(addrBegin, addrEnd - addrBegin), nullptr, 16);
        }
        catch (const std::invalid_argument&)
        {
            status = false;
        }
    }

    // Find the beginning and the end of the last instruction address.
    if (status)
    {
        status = (addrEnd = kernelIsaText.rfind(ADDR_CODE_DELIMITER)) != std::string::npos;
        if (status)
        {
            status = (addrBegin = kernelIsaText.rfind(ADDR_COMMENT_PREFIX, addrEnd)) != std::string::npos;
        }
    }

    // Parse the address of the last instruction.
    if (status)
    {
        try
        {
            addrBegin += ADDR_COMMENT_PREFIX.size();
            lastInstAddr = std::stoull(kernelIsaText.substr(addrBegin, addrEnd - addrBegin), nullptr, 16);
        }
        catch (const std::invalid_argument&)
        {
            status = false;
        }
    }

    // ISA size = address_of_last_instruction - address_of_1st_instruction + size_of_last_instruction.
    if (status)
    {
        size_t  lastInstSize = ( (kernelIsaText.size() - addrEnd) >= INST_CODE_LEN_64 ? INST_SIZE_64 : INST_SIZE_32 );
        sizeInBytes = static_cast<size_t>(lastInstAddr - firstInstAddr + lastInstSize);
    }

    return status;
}

// Parse an integer YAML node for CodeProps value.
// Do nothing if "oldResult" is false.
// If "zeroIfAbsent" is true, the value is set to 0 and "true" is returned if the required value is not found in the MD.
// (The Lightning Compiler does not generate some CodeProps values if they are 0).
static bool ParseCodePropsItem(const YAML::Node& codeProps, const std::string& key, size_t& value, bool zeroIfAbsent = false)
{
    bool  result;
    const YAML::Node& propMDNode = codeProps[key];
    if ((result = propMDNode.IsDefined()) == true)
    {
        try
        {
            value = propMDNode.as<size_t>();
        }
        catch (const YAML::TypedBadConversion<size_t>&)
        {
            result = false;
        }
    }
    else
    {
        if (zeroIfAbsent)
        {
            value = 0;
            result = true;
        }
    }

    return result;
}

// Parse a single YAML node for kernel MD.
static bool  ParseKernelCodeProps(const YAML::Node& kernelMD, KernelCodeProps& codeProps)
{
    YAML::Node  kernelCodePropsMD, propMDNode;

    bool  result = (kernelCodePropsMD = kernelMD[CODE_OBJ_MD_CODE_PROPS_KEY]).IsDefined();

    result = result && ParseCodePropsItem(kernelCodePropsMD, CODE_OBJ_MD_WAVEFRONT_SGRPS, codeProps.wavefrontNumSGPRs, true);
    result = result && ParseCodePropsItem(kernelCodePropsMD, CODE_OBJ_MD_WORKITEM_VGPRS,  codeProps.workitemNumVGPRs, true);
    result = result && ParseCodePropsItem(kernelCodePropsMD, CODE_OBJ_MD_WAVEFRONT_SIZE,  codeProps.wavefrontSize);
    result = result && ParseCodePropsItem(kernelCodePropsMD, CODE_OBJ_MD_GROUP_SEGMENT_SIZE, codeProps.workgroupSegmentSize, true);
    result = result && ParseCodePropsItem(kernelCodePropsMD, CODE_OBJ_MD_PRIVATE_SEGMENT_SIZE, codeProps.privateSegmentSize, true);
    result = result && ParseCodePropsItem(kernelCodePropsMD, CODE_OBJ_MD_SPILLED_SGPRS, codeProps.SGPRSpills, true);
    result = result && ParseCodePropsItem(kernelCodePropsMD, CODE_OBJ_MD_SPILLED_VGPRS, codeProps.VGPRSpills, true);

    return result;
}

// Parse the provided CodeObj metadata and extract CodeProps data for all kernels.
static beStatus  ParseCodeProps(const std::string & MDText, CodePropsMap& codeProps)
{
    beStatus  status = beStatus_SUCCESS;
    size_t  startOffset, endOffset;
    YAML::Node  codeObjMDNode, kernelsMDMap, kernelName;

    startOffset = MDText.find(CODEOBJ_METADATA_START_TOKEN);

    while ((endOffset = MDText.find(CODEOBJ_METADATA_END_TOKEN, startOffset)) != std::string::npos)
    {
        try
        {
            const std::string& kernelMDText = MDText.substr(startOffset, endOffset - startOffset + CODEOBJ_METADATA_END_TOKEN.size());
            codeObjMDNode = YAML::Load(kernelMDText);
        }
        catch (YAML::ParserException&)
        {
            status = beStatus_LC_ParseCodeObjMDFailed;
            break;
        }

        if (codeObjMDNode.IsMap() && (kernelsMDMap = codeObjMDNode[CODE_OBJ_MD_KERNELS_KEY]).IsDefined())
        {
            for (const YAML::Node& kernelMD : kernelsMDMap)
            {
                KernelCodeProps  kernelCodeProps;
                if (status == beStatus_SUCCESS &&
                    (kernelName = kernelMD[CODE_OBJ_MD_KERNEL_NAME_KEY]).IsDefined() &&
                    ParseKernelCodeProps(kernelMD, kernelCodeProps))
                {
                    codeProps[kernelName.as<std::string>()] = kernelCodeProps;
                }
                else
                {
                    status = beStatus_LC_ParseCodeObjMDFailed;
                    break;
                }
            }
        }
        startOffset = MDText.find(CODEOBJ_METADATA_START_TOKEN, endOffset);
    }

    return status;
}

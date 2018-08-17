#ifndef _BEPROGRAMBUILDERLIGHTNING_H_
#define _BEPROGRAMBUILDERLIGHTNING_H_

// C++.
#include <string>
#include <sstream>
#include <unordered_map>

// Infra:
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <RadeonGPUAnalyzerBackend/include/beProgramBuilder.h>
#include <RadeonGPUAnalyzerBackend/include/beOpenCLDefs.h>
#include <RadeonGPUAnalyzerBackend/include/beStringConstants.h>

using namespace beKA;

// Type of ObjDump operation
enum class
ObjDumpOp
{
    Disassemble,
    DisassembleWithLineNums,
    GetMetadata,
    GetKernelCodeSize
};


// Kernel statistics
struct KernelCodeProps
{
    size_t  wavefrontNumSGPRs;
    size_t  workitemNumVGPRs;
    size_t  wavefrontSize;
    size_t  workgroupSegmentSize;
    size_t  privateSegmentSize;
    size_t  SGPRSpills;
    size_t  VGPRSpills;
    size_t  isaSize;
};

// Maps  kernel_name --> KernelCodeProps.
typedef  std::map<std::string, KernelCodeProps>  CodePropsMap;

///
/// Builder implementation for compilation with ROCm Lightning compiler.
///
class beProgramBuilderLightning : public beProgramBuilder
{
public:
    beProgramBuilderLightning()               = default;
    ~beProgramBuilderLightning(void) override = default;

    beStatus GetBinary(const std::string& device, const beKA::BinaryOptions& binopts, std::vector<char>& binary) override;
    beStatus GetKernelILText(const std::string& device, const std::string& kernel, std::string& il) override;
    beStatus GetKernelISAText(const std::string& device, const std::string& kernel, std::string& isa) override;
    beStatus GetStatistics(const std::string& device, const std::string& kernel, beKA::AnalysisData& analysis) override;
    beStatus GetDeviceTable(std::vector<GDT_GfxCardInfo>& table) override;

    // Retrieve the ROCm compiler version.
    // The version reported by the compiler is returned in "outText" string.
    static beStatus GetCompilerVersion(SourceLanguage lang, const std::string& userBinDir, bool printCmd, std::string& outText);

    // Compile a set of OpenCL source files to a single executable.
    // The "errText" is the content of stderr dumped by the compiler in case of compilation failure.
    static beStatus CompileOpenCLToBinary(const CmplrPaths&                cmplrPaths,
                                          const OpenCLOptions&             userOptions,
                                          const std::vector<std::string>&  srcFileNames,
                                          const std::string&               binFileName,
                                          const std::string&               device,
                                          bool                             printCmd,
                                          std::string&                     errText);

    // Preprocess input file using the ROCm OpenCL compiler. The preprocessed program text
    // is returned in "output" string.
    static beStatus PreprocessOpenCL(const std::string& userBinDir, const std::string& inputFile,
                                     const std::string& args, bool printCmd, std::string& output);

    // Disassemble binary to ISA text.
    static beStatus  DisassembleBinary(const std::string& userBinDir, const std::string& binFileName, const std::string& device,
                                       bool lineNumbers, bool printCmd, std::string& outISAText, std::string& errMsg);

    // Extract CodeObject metadata.
    static beStatus  ExtractMetadata(const std::string& userBinDir, const std::string& binFileName,
                                     bool printCmd, std::string& metadataText);

    // Extract CodeObject Metadata kernels properties.
    static beStatus  ExtractKernelCodeProps(const std::string& userBinDir, const std::string& binFileName,
                                            bool printCmd, CodePropsMap& codeProps);

    // Extract names of kernels from the binary.
    static beStatus  ExtractKernelNames(const std::string& userBinDir, const std::string& binFileName,
                                        bool printCmd, std::vector<std::string>& kernelNames);

    // Check if the output file exists and not empty.
    static bool  VerifyOutputFile(const std::string& fileName);

    // Get the ISA size.
    static int  GetIsaSize(const std::string& isaText);

    // Extract the size of binary section for the provided kernel.
    static int  GetKernelCodeSize(const std::string& userBinDir, const std::string& binFile,
                                  const std::string& kernelName, bool printCmd);

protected:
    // Adds standard options required to compile source language with LC compiler to the "options" string.
    static beStatus AddCompilerStandardOptions(beKA::SourceLanguage lang, const CmplrPaths& cmplrPaths, std::string& options);

    // Builds a string containing all necessary options for OpenCL compiler.
    // Writes constructed options string to "options".
    static beStatus ConstructOpenCLCompilerOptions(const CmplrPaths&                 cmplrPaths,
                                                   const OpenCLOptions&              userOptions,
                                                   const std::vector<std::string>&   srcFileNames,
                                                   const std::string&                binFileName,
                                                   const std::string&                device,
                                                   std::string&                      outOptions);

    // Compile for the provided language with the given command line options.
    // The stdout and stderr generated by compiler are returned in "stdOut" and "stdErr".
    static beStatus InvokeCompiler(beKA::SourceLanguage lang, const std::string& userBinFolder, const std::string& cmdLineOptions,
                                   bool printCmd, std::string& stdOut, std::string& stdErr, unsigned long timeOut = 0);

    // Check if the compiler generated output file and reported no errors.
    static beStatus VerifyCompilerOutput(const std::string& outFileName, const std::string& errText);

    // Builds a string containing the standard options required by the "op" operation of ObjDump.
    static beStatus ConstructObjDumpOptions(ObjDumpOp          op,
                                            const std::string& compilerBinDir,
                                            const std::string& binFileName,
                                            const std::string& device,
                                            bool               printCmd,
                                            std::string&       options);

    // Launch the LC ObjDump of ReadObj (for CodeObj metadata).
    static beStatus InvokeObjDump(ObjDumpOp op, const std::string& userBinDir, const std::string& cmdLineOptions,
                                  bool printCmd, std::string& outText, std::string& errMsg);

    // Check if the ObjDump output is an error message or ISA text.
    static beStatus VerifyObjDumpOutput(const std::string& objDumpOut);

    // Filter the output text generated by ObjDump. Keep only CodeObj metadata or ISA (specified by "op") and remove averything else.
    static beStatus FilterCodeObjOutput(ObjDumpOp op, std::string& text);

    // Checks if the readobj supports "-amdgpu-code-object-metadata" option.
    static bool ReadobjSupportsGetMDOption(const std::string& userBinDir, bool printCmd);
};

#endif // _BEPROGRAMBUILDERLIGHTNING_H_

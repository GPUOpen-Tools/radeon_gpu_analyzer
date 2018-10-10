
// C++.
#include <vector>
#include <map>
#include <utility>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <cassert>

// Infra.
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <Utils/include/rgLog.h>

// Local.
#include <RadeonGPUAnalyzerCLI/src/kcCliStringConstants.h>
#include <RadeonGPUAnalyzerCLI/src/kcUtils.h>
#include <RadeonGPUAnalyzerCLI/src/kcCLICommanderLightning.h>
#include <RadeonGPUAnalyzerBackend/include/beProgramBuilderLightning.h>

// *****************************************
// *** INTERNALLY LINKED SYMBOLS - START ***
// *****************************************

// Targets of Lightning Compiler in LLVM format and corresponing DeviceInfo names.
static const std::vector<std::pair<std::string, std::string>>
LC_LLVM_TARGETS_TO_DEVICE_INFO_TARGETS = { {"gfx801", "Carrizo"},
                                           {"gfx802", "Tonga"},
                                           {"gfx803", "Fiji"},
                                           {"gfx803", "Ellesmere"},
                                           {"gfx803", "Baffin"},
                                           {"gfx900", "gfx900"},
                                           {"gfx902", "gfx902"} };

// For some devices, clang does not accept device names that RGA gets from DeviceInfo.
// This table maps the DeviceInfo names to names accepted by clang for such devices.
static const std::map<std::string, std::string>
LC_DEVICE_INFO_TO_CLANG_DEVICE_MAP = { {"ellesmere", "polaris10"},
                                       {"baffin",    "polaris11"} };

// Default target for Lightning Compiler (the latest supported target).
static const std::string  LC_DEFAULT_TARGET = LC_LLVM_TARGETS_TO_DEVICE_INFO_TARGETS.rbegin()->second;

static const gtString  LC_TEMP_BINARY_FILE_NAME = L"rga_lc_ocl_out";
static const gtString  LC_TEMP_BINARY_FILE_EXT  = L"bin";
static const gtString  LC_TEMP_ISA_FILE_NAME    = L"rga_lc_isa_";
static const gtString  LC_TEMP_ISA_FILE_EXT     = L"isa";
static const gtString  LC_TEMP_ERR_FILE_NAME    = L"rga_lc_stderr";
static const gtString  LC_TEMP_OUT_FILE_NAME    = L"rga_lc_stdout";
static const gtString  LC_TEMP_OUT_ERR_FILE_EXT = L"txt";

static const std::string  LC_KERNEL_ISA_HEADER_1 = "AMD Kernel Code for ";
static const std::string  LC_KERNEL_ISA_HEADER_2 = "Disassembly for ";
static const std::string  LC_KERNEL_ISA_HEADER_3 = "@kernel ";

static const std::string  COMPILER_VERSION_TOKEN  = "clang version ";

static const std::string  LC_ISA_INST_SUFFIX_1 = "_e32";
static const std::string  LC_ISA_INST_SUFFIX_2 = "_e64";

static const std::string  LC_ISA_BRANCH_TOKEN  = "branch";
static const std::string  LC_ISA_CALL_TOKEN    = "call";

static const std::string  LC_ISA_INST_ADDR_START_TOKEN = "//";
static const std::string  LC_ISA_INST_ADDR_END_TOKEN   = ":";
static const std::string  LC_ISA_COMMENT_START_TOKEN   = ";";

static const std::string  OPENCL_KERNEL_QUALIFIER_TOKEN_1   = "__kernel";
static const std::string  OPENCL_KERNEL_QUALIFIER_TOKEN_2   = "kernel";
static const std::string  OPENCL_ATTRIBUTE_QUALIFIER_TOKEN  = "__attribute__";
static const std::string  OPENCL_PRAGMA_TOKEN               = "pragma";

static const std::string  STR_NA_VALUE = "N/A";

// The device properties necessary for generating statistics.
struct DEVICE_PROPS
{
    uint64_t  AVAILABLE_SGPRS;
    uint64_t  AVAILABLE_VGPRS;
    uint64_t  AVAILABLE_LDS_BYTES;
    uint64_t  MIN_SGPRS;
    uint64_t  MIN_VGPRS;
};

static const  std::map<std::string, DEVICE_PROPS> rgDeviceProps =
    { {"carrizo",   {102, 256, 32768, 16,  4}},
      {"tonga",     {102, 256, 32768, 16, 64}},
      {"fiji",      {102, 256, 32768, 16,  4}},
      {"ellesmere", {102, 256, 32768, 16,  4}},
      {"baffin",    {102, 256, 32768, 16,  4}},
      {"polaris10", {102, 256, 32768, 16,  4}},
      {"polaris11", {102, 256, 32768, 16,  4}},
      {"gfx900",    {102, 256, 65536, 16,  4}},
      {"gfx902",    {102, 256, 65536, 16,  4}} };

static const size_t  ISA_INST_64BIT_CODE_TEXT_SIZE = 16;
static const int     ISA_INST_64BIT_BYTES_SIZE     = 8;
static const int     ISA_INST_32BIT_BYTES_SIZE     = 4;

// ***************************************
// *** INTERNALLY LINKED SYMBOLS - END ***
// ***************************************

static bool GetParsedIsaCSVText(const std::string& isaText, const std::string& device, bool lineNumbers, std::string& csvText);

// Get default target name.
static std::string GetDefaultTarget()
{
    return LC_DEFAULT_TARGET;
}

// Returns the list of additional LC targets specified in the "additional-targets" file.
static std::vector<std::string> GetExtraTargetList()
{
    std::vector<std::string> deviceList;
    osFilePath targetsFilePath;
    osGetCurrentApplicationPath(targetsFilePath, false);
    targetsFilePath.appendSubDirectory(LC_OPENCL_ROOT_DIR);
    targetsFilePath.setFileName(LC_EXTRA_TARGETS_FILE_NAME);
    targetsFilePath.clearFileExtension();

    std::ifstream targetsFile(targetsFilePath.asString().asASCIICharArray());
    if (targetsFile.good())
    {
        std::string device;

        while (std::getline(targetsFile, device))
        {
            if (device.find("//") == std::string::npos)
            {
                // Save the target name in lower case to avoid case-sensitivity issues.
                std::transform(device.begin(), device.end(), device.begin(), [](const char& c) {return std::tolower(c); });
                deviceList.push_back(device);
            }
        }
    }

    return deviceList;
}

static void  LogPreStep(const std::string& msg, const std::string& device = "")
{
    std::cout << msg << device << "... ";
}

static void  LogResult(bool result)
{
    std::cout << (result ? KA_CLI_STR_STATUS_SUCCESS : KA_CLI_STR_STATUS_FAILURE) << std::endl;
}

static void  LogErrorStatus(beStatus status, const std::string & errMsg)
{
    switch (status)
    {
    case beStatus_SUCCESS:
        break;
    case beStatus_LC_CompilerLaunchFailed:
        std::cout << std::endl << STR_ERR_CANNOT_INVOKE_COMPILER << std::endl;
        break;
    case beStatus_LC_CompilerGeneratedError:
        std::cout << std::endl << STR_ERR_ROCM_OPENCL_COMPILE_ERROR << std::endl;
        std::cout << errMsg << std::endl;
        break;
    case beStatus_NoOutputFileGenerated:
        std::cout << std::endl << STR_ERR_NO_OUTPUT_FILE_GENERATED << std::endl;
        break;
    case beStatus_NO_BINARY_FOR_DEVICE:
        std::cout << std::endl << STR_ERR_CANNOT_FIND_BINARY << std::endl;
        break;
    case beStatus_LC_DisassembleFailed:
        std::cout << std::endl << STR_ERR_ROCM_DISASM_ERROR << std::endl;
        std::cout << errMsg << std::endl;
        break;
    case beStatus_LC_CompilerTimeOut:
        std::cout << std::endl << STR_ERR_ROCM_OPENCL_COMPILE_TIMEOUT << std::endl;
        std::cout << errMsg << std::endl;
        break;
    default:
        std::cout << std::endl << (errMsg.empty() ? STR_ERR_UNKNOWN_COMPILATION_STATUS : errMsg) << std::endl;
        break;
    }
}

beKA::beStatus kcCLICommanderLightning::Init(const Config& config, LoggingCallBackFunc_t logCallback)
{
    m_LogCallback = logCallback;
    m_cmplrPaths  = {config.m_cmplrBinPath, config.m_cmplrIncPath, config.m_cmplrLibPath};
    m_printCmds   = config.m_printProcessCmdLines;

    return beKA::beStatus_SUCCESS;
}

// Checks if text[offset] is a start of OpenCL kernel qualifier ("kernel" of "__kernel" token).
// Spaces are ignored.
// The offset of the first symbol afther the qualifier is returned in "offset".
inline static
bool  IsKernelQual(const std::string& text, unsigned char prevSymbol, size_t& offset)
{
    bool  found = false;

    if ((prevSymbol == ' ' || prevSymbol == '\n' || prevSymbol == '}'))
    {
        size_t  qualSize = 0;

        if (text.compare(offset, OPENCL_KERNEL_QUALIFIER_TOKEN_1.size(), OPENCL_KERNEL_QUALIFIER_TOKEN_1) == 0)
        {
            qualSize = OPENCL_KERNEL_QUALIFIER_TOKEN_1.size();
        }
        else if (text.compare(offset, OPENCL_KERNEL_QUALIFIER_TOKEN_2.size(), OPENCL_KERNEL_QUALIFIER_TOKEN_2) == 0)
        {
            qualSize = OPENCL_KERNEL_QUALIFIER_TOKEN_2.size();
        }

        if (qualSize != 0 && (text[offset + qualSize] == ' ' || text[offset + qualSize] == '\n'))
        {
            offset += qualSize;
            found = true;
        }
    }

    return found;
}

// Skips the "__attribute__((...))" qualifier.
// Sets "offset" to point to the first symbol after the qualifier.
static void  SkipAttributeQual(const std::string& text, size_t& offset)
{
    if ((offset = text.find_first_not_of(" \n", offset)) != std::string::npos)
    {
        if (text.compare(offset, OPENCL_ATTRIBUTE_QUALIFIER_TOKEN.size(), OPENCL_ATTRIBUTE_QUALIFIER_TOKEN) == 0)
        {
            // Skip the attribute arguments as well.
            offset += OPENCL_ATTRIBUTE_QUALIFIER_TOKEN.size();
            size_t  currentOffset = offset;
            if ((currentOffset = text.find_first_of('(', currentOffset)) != std::string::npos)
            {
                uint32_t  parenthCount = 1;
                while (++currentOffset < text.size() && parenthCount > 0)
                {
                    parenthCount += (text[currentOffset] == '(' ? 1 : (text[currentOffset] == ')' ? -1 : 0));
                }
                offset = currentOffset;
            }
        }
    }
}

// Parse a preprocessor hint.
// Example:
//    "# 2 "some folder/test.cl" 24"
// Output:
//    {"2", "some folder/test.cl", "24"}
static void  ParsePreprocessorHint(const std::string& hintLine, std::vector<std::string>& hintItems)
{
    hintItems.clear();
    size_t  endOffset, offset = 0;

    while ((offset = hintLine.find_first_not_of(' ', offset)) != std::string::npos)
    {
        if (hintLine[offset] == '"' && (endOffset = hintLine.find('"', offset + 1)) != std::string::npos)
        {
            // The preprocessor generates double back-slash as a path delimiter on Windows.
            // Replace them with single slashes here.
            std::string filePath = hintLine.substr(offset + 1, endOffset - offset - 1);
            char prev = 0;
            auto found = [&](char& c) { bool ret = (prev == '\\' && c == '\\'); prev = c; return ret; };
            filePath.erase(std::remove_if(filePath.begin(), filePath.end(), found), filePath.end());
            hintItems.push_back(filePath);
            offset = endOffset + 1;
        }
        else
        {
            endOffset = hintLine.find_first_of(' ', offset);
            hintItems.push_back(hintLine.substr(offset, (endOffset != std::string::npos ? endOffset - offset : endOffset)));
            offset = (endOffset != std::string::npos ? endOffset + 1 : hintLine.size());
        }
    }
}

// Parse a preprocessor line that starts with '#'.
// Returns updated offset.
static size_t  ParsePreprocessorLine(const std::string& text, const std::string& fileName, size_t offset,
                                     unsigned int& fileOffset, unsigned int& lineNum)
{
    // Parse the preprocessor hint line to get the file name and line offset.
    // We are interested in hints like:  # <file_offset> <file_name>
    //                              or:  # <file_offset> <file_name> 2
    // If the source file found in the hint is not "our" file, put 0 as file offset.
    size_t  eol = text.find_first_of('\n', offset);
    std::vector<std::string>  hintItems;
    ParsePreprocessorHint(text.substr(offset + 1, eol - offset - 1), hintItems);
    if (hintItems.size() == 2 || (hintItems.size() == 3 && std::atoi(hintItems[2].c_str()) == 2))
    {
        unsigned int offset = std::atoi(hintItems[0].c_str());
        if (offset > 0 && hintItems[1] == fileName)
        {
            fileOffset = offset;
            lineNum = 0;
        }
        else
        {
            fileOffset = 0;
        }
    }

    return eol;
}

// Parses a kernel declaration. Puts kernel name and starting line number to the "entryDeclInfo".
// Returns "true" if successfully parsed the kernel declaration or "false" otherwise.
static bool  ParseKernelDecl(const std::string& text, size_t& offset, unsigned int fileOffset, size_t kernelQualStart,
                             unsigned int& lineNum, std::tuple<std::string, int, int>& entryDeclInfo)
{
    bool  ret = false;

    // Skip "__attribute__(...)" if it's present.
    SkipAttributeQual(text, offset);

    // The kernel name is the last lexical token before "(" or "<" symbol.
    size_t  kernelNameStart, kernelNameEnd;
    if ((kernelNameEnd = text.find_first_of("(<", offset)) != std::string::npos)
    {
        if ((kernelNameEnd = text.find_last_not_of(" \n", kernelNameEnd - 1)) != std::string::npos &&
            (kernelNameStart = text.find_last_of(" \n", kernelNameEnd)) != std::string::npos)
        {
            kernelNameStart++;
            std::string  kernelName = text.substr(kernelNameStart, kernelNameEnd - kernelNameStart + 1);
            offset = kernelNameEnd;
            if (!kernelName.empty())
            {
                // Store the found kernel name and corresponding line number to "entryDeclInfo".
                std::get<0>(entryDeclInfo) = kernelName;
                std::get<1>(entryDeclInfo) = (fileOffset == 0 ? 0 : fileOffset + lineNum);
                ret = true;
            }
        }
    }

    // Count the number of lines between the kernel qualifier and the kernel name.
    lineNum += (unsigned)std::count(text.begin() + kernelQualStart, text.begin() + kernelNameEnd, '\n');

     return ret;
}

// Extracts list of kernel names from OpenCL source text.
// Returns kernel names in "entryData" as a vector of pairs {kernel_name, src_line}.
static bool  ExtractEntriesPreprocessed(std::string& text, const std::string& fileName, rgEntryData& entryData)
{
    //  # 1 "test.cl" 2
    //  # 12 "test.cl"   <-- preprocessor hint (file offset = 12)
    //
    //  __kernel void bar(global int *N)  <-- The number of this line in the original file =
    //                                        the number of this line in preprocessed file + file offet.

    size_t  offset = 0, kernelQualStart = 0, size = text.size();
    unsigned int  fileOffset, lineNum = 0, bracketCount = 0;
    unsigned char  prevSymbol = '\n';
    bool  inKernel = false;
    std::tuple<std::string, int, int>  entryDeclInfo;

    // Replace tabs with spaces.
    std::replace(text.begin(), text.end(), '\t', ' ');

    // Start parsing.
    while (offset < size)
    {
        switch (text[offset])
        {
        case ' ' : break;
        case '\n': lineNum++; break;
        case '"' : while (++offset < size && (text[offset] != '"'  || text[offset - 1] == '\\')) {}; break;
        case '\'': while (++offset < size && (text[offset] != '\'' || text[offset - 1] == '\\')) {}; break;
        case '{' : bracketCount++; break;

        case '}' :
            if (--bracketCount == 0 && inKernel)
            {
                // Found the end of kernel body. Store the current line number.
                std::get<2>(entryDeclInfo) = (fileOffset == 0 ? 0 : fileOffset + lineNum);
                entryData.push_back(entryDeclInfo);
                inKernel = false;
            }
            break;

        case '#':
            if (prevSymbol == '\n' && text.compare(offset + 1, OPENCL_PRAGMA_TOKEN.size(), OPENCL_PRAGMA_TOKEN) != 0)
            {
                offset = ParsePreprocessorLine(text, fileName, offset, fileOffset, lineNum);
            }
            break;

        default:
            // Look for "kernel" or "__kernel" qualifiers.
            kernelQualStart = offset;
            if (IsKernelQual(text, prevSymbol, offset))
            {
                inKernel = ParseKernelDecl(text, offset, fileOffset, kernelQualStart, lineNum, entryDeclInfo);
            }
        }
        prevSymbol = text[offset++];
    }

    return true;
}

// Gather the definitions and include paths into a single "options" string.
static std::string  GatherOCLOptions(const Config& config)
{
    std::stringstream  optStream;
    for (const std::string&  def : config.m_Defines)
    {
        optStream << "-D" << def << " ";
    }
    for (const std::string&  inc : config.m_IncludePath)
    {
        optStream << "-I" << inc << " ";
    }
    return optStream.str();
}

bool kcCLICommanderLightning::InitRequestedAsicListLC(const Config& config)
{
    bool  ret = false;

    if (config.m_ASICs.empty())
    {
        // Use default target if no target is specified by user.
        m_targets.insert(LC_DEFAULT_TARGET);
        ret = true;
    }
    else
    {
        for (std::string device : config.m_ASICs)
        {
            std::set<std::string>  supportedTargets = GetSupportedTargets();
            std::string  matchedArchName;

            // If the device is specified in the LLVM format, convert it to the DeviceInfo format.
            auto llvmDevice = std::find_if(LC_LLVM_TARGETS_TO_DEVICE_INFO_TARGETS.cbegin(), LC_LLVM_TARGETS_TO_DEVICE_INFO_TARGETS.cend(),
                                           [&](const std::pair<std::string, std::string>& d){ return (d.first == device);});
            if (llvmDevice != LC_LLVM_TARGETS_TO_DEVICE_INFO_TARGETS.cend())
            {
                device = llvmDevice->second;
            }

            // Try to detect device.
            if ((ret = kcUtils::FindGPUArchName(device, matchedArchName, true, true)) == true)
            {
                // Check if the matched architecture name is present in the list of supported devices.
                for (std::string supportedDevice : supportedTargets)
                {
                    if (matchedArchName.find(supportedDevice) != std::string::npos)
                    {
                        std::transform(supportedDevice.begin(), supportedDevice.end(), supportedDevice.begin(),
                                       [](const char& c) {return std::tolower(c); });
                        m_targets.insert(supportedDevice);
                        ret = true;
                        break;
                    }
                }
            }

            if (!ret)
            {
                // Try additional devices from "additional-targets" file.
                std::vector<std::string> extraDevices = GetExtraTargetList();
                std::transform(device.begin(), device.end(), device.begin(), [](const char& c) {return std::tolower(c); });
                if (std::find(extraDevices.cbegin(), extraDevices.cend(), device) != extraDevices.cend())
                {
                    rgLog::stdErr << STR_WRN_USING_EXTRA_LC_DEVICE_1 << device << STR_WRN_USING_EXTRA_LC_DEVICE_2 << std::endl << std::endl;
                    m_targets.insert(device);
                    ret = true;
                }
            }

            if (!ret)
            {
                rgLog::stdErr << STR_ERR_UNKNOWN_DEVICE_PROVIDED_1 << device << STR_ERR_UNKNOWN_DEVICE_PROVIDED_2 << std::endl;
            }
        }
    }

    return !m_targets.empty();
}

bool kcCLICommanderLightning::Compile(const Config & config)
{
    beKA::beStatus result = beKA::beStatus_SUCCESS;

    if (!InitRequestedAsicListLC(config))
        return false;

    // Prepare OpenCL options and defines.
    OpenCLOptions options;
    options.m_selectedDevices      = m_targets;
    options.m_defines              = config.m_Defines;
    options.m_incPaths             = config.m_IncludePath;
    options.m_openCLCompileOptions = config.m_OpenCLOptions;
    options.m_optLevel             = config.m_optLevel;
    options.m_lineNumbers          = config.m_isLineNumbersRequired;
    options.m_dumpIL               = !config.m_ILFile.empty();

    // Run the back-end compilation procedure.
    switch (config.m_SourceLanguage)
    {
    case beKA::SourceLanguage_Rocm_OpenCL:
        result = CompileOpenCL(config, options);
        break;
    default:
        result = beKA::beStatus_General_FAILED;
        break;
    }

    // Generate CSV files with parsed ISA if required.
    if (config.m_isParsedISARequired && (result == beKA::beStatus_SUCCESS || m_targets.size() > 1))
    {
        result = ParseIsaFilesToCSV(config.m_isLineNumbersRequired) ? beKA::beStatus_SUCCESS : beKA::beStatus_ParseIsaToCsvFailed;
    }

    return (result == beKA::beStatus_SUCCESS);
}

void kcCLICommanderLightning::Version(Config & config, LoggingCallBackFunc_t callback)
{
    bool  ret;
    std::stringstream s_Log;
    kcCLICommander::Version(config, callback);

    std::string  outputText = "", version = "";

    beKA::beStatus  status = beProgramBuilderLightning::GetCompilerVersion(SourceLanguage_Rocm_OpenCL, config.m_cmplrBinPath,
                                                                           config.m_printProcessCmdLines, outputText);
    ret = (status == beKA::beStatus_SUCCESS);

    if (ret)
    {
        size_t  offset = outputText.find(COMPILER_VERSION_TOKEN);
        if (offset != std::string::npos)
        {
            offset += COMPILER_VERSION_TOKEN.size();
            size_t  offset1 = outputText.find(" ", offset);
            size_t  offset2 = outputText.find("\n", offset);
            if (offset1 != std::string::npos && offset2 != std::string::npos)
            {
                size_t  endOffset = min(offset1, offset2);
                version = outputText.substr(0, endOffset);
            }
        }
    }

    if (ret)
    {
        s_Log << STR_ROCM_OPENCL_COMPILER_VERSION_PREFIX << version << std::endl;
    }

    LogCallBack(s_Log.str());
}

bool kcCLICommanderLightning::PrintAsicList(std::ostream& log)
{
    bool ret = kcUtils::PrintAsicList(log, std::set<std::string>(), GetSupportedTargets());
    if (ret)
    {
        // Print additional rocm-cl target from the "additional-targets" file.
        std::vector<std::string> extraTargets = GetExtraTargetList();
        if (!extraTargets.empty())
        {
            rgLog::stdOut << std::endl << KC_STR_EXTRA_DEVICE_LIST_TITLE << std::endl << std::endl;
            for (const std::string& device : extraTargets)
            {
                rgLog::stdOut << KC_STR_DEVICE_LIST_OFFSET << device << std::endl;
            }
            rgLog::stdOut << std::endl;
        }
    }
    return ret;
}

beStatus kcCLICommanderLightning::CompileOpenCL(const Config& config, const OpenCLOptions& oclOptions)
{
    beStatus  status = beStatus_SUCCESS;

    // Run LC compiler for all requested devices
    for (const std::string& device : oclOptions.m_selectedDevices)
    {
        std::string  errText;
        LogPreStep(KA_CLI_STR_COMPILING, device);
        std::string  binFileName;
        gtString  ilFileName, binName;

        // Adjust the device name if necessary.
        std::string clangDevice = device;
        if (LC_DEVICE_INFO_TO_CLANG_DEVICE_MAP.count(clangDevice) > 0)
        {
            clangDevice = LC_DEVICE_INFO_TO_CLANG_DEVICE_MAP.at(device);
        }

        // Update the binary and ISA names for current device.
        beStatus currentStatus = AdjustBinaryFileName(config, clangDevice, binFileName);

        // If file with the same file exist, delete it.
        binName << binFileName.c_str();
        kcUtils::DeleteFile(binName);

        // Prepare a list of source files.
        std::vector<std::string>  srcFileNames;
        for (const std::string& inputFile : config.m_InputFiles)
        {
            srcFileNames.push_back(inputFile);
        }

        if (currentStatus != beStatus_SUCCESS)
        {
            LogErrorStatus(currentStatus, errText);
            continue;
        }

        // Compile source to binary.
        currentStatus = beProgramBuilderLightning::CompileOpenCLToBinary(m_cmplrPaths,
                                                                         oclOptions,
                                                                         srcFileNames,
                                                                         binFileName,
                                                                         clangDevice,
                                                                         m_printCmds,
                                                                         errText);
        LogResult(currentStatus == beStatus_SUCCESS);

        if (currentStatus == beStatus_SUCCESS)
        {
            // If "dump IL" option is passed to the ROCm compiler, it should dump the IL to stderr.
            if (oclOptions.m_dumpIL)
            {
                kcUtils::ConstructOutputFileName(config.m_ILFile, KC_STR_DEFAULT_LLVM_IR_SUFFIX, "", device, ilFileName);
                currentStatus = kcUtils::WriteTextFile(ilFileName.asASCIICharArray(), errText, nullptr) ?
                    beStatus_SUCCESS : beStatus_WriteToFile_FAILED;
            }

            // Disassemble binary to ISA text.
            if (!config.m_ISAFile.empty() || !config.m_AnalysisFile.empty() ||
                !config.m_LiveRegisterAnalysisFile.empty() || !config.m_blockCFGFile.empty() || !config.m_instCFGFile.empty())
            {
                LogPreStep(KA_CLI_STR_EXTRACTING_ISA, device);
                currentStatus = DisassembleBinary(binFileName, config.m_ISAFile, clangDevice, config.m_Function, config.m_isLineNumbersRequired, errText);
                LogResult(currentStatus == beStatus_SUCCESS);

                // Propagate the binary file name to the Output Files Metadata table.
                for (auto& outputMDNode : m_outputMetadata)
                {
                    const std::string& mdDevice = outputMDNode.first.first;
                    if (mdDevice == clangDevice)
                    {
                        outputMDNode.second.m_binFile = binFileName;
                        outputMDNode.second.m_isBinFileTemp = config.m_BinaryOutputFile.empty();
                    }
                }
            }
            else
            {
                m_outputMetadata[{device, ""}] = rgOutputFiles(rgEntryType::OpenCL_Kernel, "", binFileName);
            }
        }
        else
        {
            // Store error status to the metadata.
            rgOutputFiles output(rgEntryType::OpenCL_Kernel, "", "");
            output.m_status = false;
            m_outputMetadata[{device, ""}] = output;
        }

        status = (currentStatus == beStatus_SUCCESS ? status : currentStatus);
        LogErrorStatus(currentStatus, errText);
    }

    return status;
}

beKA::beStatus kcCLICommanderLightning::DisassembleBinary(const std::string& binFileName,
                                                          const std::string& userIsaFileName,
                                                          const std::string& device,
                                                          const std::string& kernel,
                                                          bool               lineNumbers,
                                                          std::string&       errText)
{
    std::string  outIsaText;
    std::vector<std::string>  kernelNames;

    beKA::beStatus status = beProgramBuilderLightning::DisassembleBinary(m_cmplrPaths.m_bin, binFileName, device, lineNumbers, m_printCmds, outIsaText, errText);

    if (status == beKA::beStatus_SUCCESS)
    {
        status = beProgramBuilderLightning::ExtractKernelNames(m_cmplrPaths.m_bin, binFileName, m_printCmds, kernelNames);
    }
    else
    {
        // Store error status to the metadata.
        rgOutputFiles output(rgEntryType::OpenCL_Kernel, "", "");
        output.m_status = false;
        m_outputMetadata[{device, ""}] = output;
    }

    if (status == beKA::beStatus_SUCCESS)
    {
        if (!kernel.empty() && std::find(kernelNames.cbegin(), kernelNames.cend(), kernel) == kernelNames.cend())
        {
            errText = std::string(STR_ERR_OPENCL_CANNOT_FIND_KERNEL) + kernel;
            status = beKA::beStatus_WrongKernelName;
        }
        else
        {
            status = SplitISA(binFileName, outIsaText, userIsaFileName, device, kernel, kernelNames) ?
                         beKA::beStatus_SUCCESS : beKA::beStatus_LC_SplitIsaFailed;
        }
    }

    return status;
}

bool  kcCLICommanderLightning::ParseIsaFilesToCSV(bool lineNumbers)
{
    bool  ret = true;

    for (const auto& outputMDItem : m_outputMetadata)
    {
        if (outputMDItem.second.m_status)
        {
            const rgOutputFiles& outputFiles = outputMDItem.second;
            std::string  isa, parsedIsa, parsedIsaFileName;
            const std::string& device = outputMDItem.first.first;
            const std::string& entry  = outputMDItem.first.second;

            bool  status = kcUtils::ReadTextFile(outputFiles.m_isaFile, isa, nullptr);

            if (status)
            {
                if ((status = GetParsedIsaCSVText(isa, device, lineNumbers, parsedIsa)) == true)
                {
                    status = (kcUtils::GetParsedISAFileName(outputFiles.m_isaFile, parsedIsaFileName) == beKA::beStatus_SUCCESS);
                    if (status)
                    {
                        status = (StoreISAToFile(parsedIsaFileName, parsedIsa) == beKA::beStatus_SUCCESS);
                    }
                    if (status)
                    {
                        m_outputMetadata[{device, entry}].m_isaCsvFile = parsedIsaFileName;
                    }
                }
            }
            ret &= status;
        }
    }

    return ret;
}

void kcCLICommanderLightning::RunCompileCommands(const Config& config, LoggingCallBackFunc_t logCallback)
{
    bool multiDevices = (config.m_ASICs.size() > 1);

    bool status = Compile(config);

    // Perform Live Registers analysis if required.
    if ((status || multiDevices) && !config.m_LiveRegisterAnalysisFile.empty())
    {
        PerformLiveRegAnalysis(config);
    }

    // Extract Control Flow Graph.
    if ((status || multiDevices) && (!config.m_blockCFGFile.empty() || !config.m_instCFGFile.empty()))
    {
        ExtractCFG(config);
    }

    // Extract CodeObj metadata if required.
    if ((status || multiDevices) && !config.m_MetadataFile.empty())
    {
        ExtractMetadata(config.m_MetadataFile);
    }

    // Extract Statistics if required.
    if ((status || multiDevices) && !config.m_AnalysisFile.empty())
    {
        ExtractStatistics(config);
    }
}

beKA::beStatus kcCLICommanderLightning::AdjustBinaryFileName(const Config&       config,
                                                             const std::string & device,
                                                             std::string&        binFileName)
{
    beKA::beStatus  status = beKA::beStatus_SUCCESS;

    gtString  name = L"";
    std::string  userBinName = config.m_BinaryOutputFile;

    // If binary output file name is not provided, create a binary file in the temp folder.
    if (userBinName == "")
    {
        userBinName = kcUtils::ConstructTempFileName(LC_TEMP_BINARY_FILE_NAME, LC_TEMP_BINARY_FILE_EXT).asASCIICharArray();
        if (userBinName == "")
        {
            status = beKA::beStatus_General_FAILED;
        }
    }

    if (status == beKA::beStatus_SUCCESS)
    {
        name = L"";
        kcUtils::ConstructOutputFileName(userBinName, KC_STR_DEFAULT_BIN_SUFFIX, "", device, name);
        binFileName = name.asASCIICharArray();
    }

    return status;
}

beKA::beStatus kcCLICommanderLightning::StoreISAToFile(const std::string& fileName, const std::string& isaText)
{
    return (kcUtils::WriteTextFile(fileName, isaText, m_LogCallback) ?
            beKA::beStatus_SUCCESS : beKA::beStatus_WriteToFile_FAILED);
}

bool kcCLICommanderLightning::PerformLiveRegAnalysis(const Config & config)
{
    bool  ret = true;
    std::stringstream  errMsg;

    LogPreStep(KA_CLI_STR_STARTING_LIVEREG);

    for (auto& outputMDItem : m_outputMetadata)
    {
        rgOutputFiles& outputFiles = outputMDItem.second;
        if (outputFiles.m_status)
        {
            const std::string& device = outputMDItem.first.first;
            const std::string& entryName = outputMDItem.first.second;
            gtString  liveRegOutFileName = L"";
            gtString  isaFileName;
            isaFileName << outputFiles.m_isaFile.c_str();

            // Construct a name for the output livereg file.
            kcUtils::ConstructOutputFileName(config.m_LiveRegisterAnalysisFile, KC_STR_DEFAULT_LIVE_REG_ANALYSIS_SUFFIX,
                entryName, device, liveRegOutFileName);
            if (!liveRegOutFileName.isEmpty())
            {
                kcUtils::PerformLiveRegisterAnalysis(isaFileName, liveRegOutFileName, m_LogCallback, config.m_printProcessCmdLines);

                if (beProgramBuilderLightning::VerifyOutputFile(liveRegOutFileName.asASCIICharArray()))
                {
                    // Store the name of livereg output file in the RGA output files metadata.
                    outputFiles.m_liveregFile = liveRegOutFileName.asASCIICharArray();
                }
                else
                {
                    errMsg << STR_ERR_CANNOT_PERFORM_LIVE_REG_ANALYSIS << " " << STR_KERNEL_NAME << entryName << std::endl;
                    LogResult(false);
                    ret = false;
                }
            }
            else
            {
                errMsg << STR_ERR_FAILED_CREATE_OUTPUT_FILE_NAME << entryName << std::endl;
                ret = false;
            }
        }
    }

    LogResult(ret);

    if (!ret)
    {
        m_LogCallback(errMsg.str());
    }

    return ret;
}

bool kcCLICommanderLightning::ExtractCFG(const Config& config)
{
    bool  ret = true;
    std::stringstream  errMsg;

    LogPreStep(KA_CLI_STR_STARTING_CFG);

    for (auto& outputMDItem : m_outputMetadata)
    {
        rgOutputFiles& outputFiles = outputMDItem.second;
        if (outputFiles.m_status)
        {
            const std::string& device = outputMDItem.first.first;
            const std::string& entryName = outputMDItem.first.second;
            gtString  cfgOutFileName = L"";
            gtString  isaFileName;
            isaFileName << outputFiles.m_isaFile.c_str();

            // Construct a name for the output livereg file.
            std::string baseFile = (!config.m_blockCFGFile.empty() ? config.m_blockCFGFile : config.m_instCFGFile);
            kcUtils::ConstructOutputFileName(baseFile, KC_STR_DEFAULT_CFG_EXT,
                                             entryName, device, cfgOutFileName);
            if (!cfgOutFileName.isEmpty())
            {
                kcUtils::GenerateControlFlowGraph(isaFileName, cfgOutFileName, m_LogCallback,
                                                  !config.m_instCFGFile.empty(), config.m_printProcessCmdLines);

                if (!beProgramBuilderLightning::VerifyOutputFile(cfgOutFileName.asASCIICharArray()))
                {
                    errMsg << STR_ERR_CANNOT_PERFORM_LIVE_REG_ANALYSIS << " " << STR_KERNEL_NAME << entryName << std::endl;
                    LogResult(false);
                    ret = false;
                }
            }
            else
            {
                errMsg << STR_ERR_FAILED_CREATE_OUTPUT_FILE_NAME << entryName << std::endl;
                ret = false;
            }
        }
    }

    LogResult(ret);

    if (!ret)
    {
        m_LogCallback(errMsg.str());
    }

    return ret;
}

beKA::beStatus kcCLICommanderLightning::ExtractMetadata(const std::string& metadataFileName) const
{
    beKA::beStatus  currentStatus;
    beKA::beStatus  status = beStatus_SUCCESS;
    std::string  metadataText;
    gtString  outFileName;

    // A set of already processed devices.
    std::set<std::string> devices;

    // outputMDNode is: pair{pair{device, kernel}, rgOutputFiles}.
    for (auto& outputMDNode : m_outputMetadata)
    {
        if (outputMDNode.second.m_status)
        {
            const std::string& device = outputMDNode.first.first;
            if (devices.count(device) == 0)
            {
                devices.insert(device);
                const std::string  binFileName = outputMDNode.second.m_binFile;

                kcUtils::ConstructOutputFileName(metadataFileName, KC_STR_DEFAULT_LC_METADATA_SUFFIX, "", device, outFileName);
                if (!outFileName.isEmpty())
                {
                    currentStatus = beProgramBuilderLightning::ExtractMetadata(m_cmplrPaths.m_bin, binFileName, m_printCmds, metadataText);
                    if (currentStatus == beKA::beStatus_SUCCESS && !metadataText.empty())
                    {
                        currentStatus = kcUtils::WriteTextFile(outFileName.asASCIICharArray(), metadataText, m_LogCallback) ?
                            beKA::beStatus_SUCCESS : beKA::beStatus_WriteToFile_FAILED;
                    }
                }
                else
                {
                    currentStatus = beKA::beStatus_LC_ExtractMetadataFailed;
                }
            }
            status = (currentStatus == beKA::beStatus_SUCCESS ? status : beKA::beStatus_LC_ExtractMetadataFailed);
        }
    }

    if (status != beKA::beStatus_SUCCESS)
    {
        std::stringstream  msg;
        msg << STR_ERR_FAILED_EXTRACT_METADATA << std::endl;
        m_LogCallback(msg.str());
    }

    return status;
}

// Get the ISA size and store it to "kernelCodeProps" structure.
static beStatus  GetIsaSize(const std::string& isaFileName, KernelCodeProps& kernelCodeProps)
{
    beStatus  status = beStatus_LC_GetISASizeFailed;
    if (!isaFileName.empty())
    {
        std::string  isaText;
        if (kcUtils::ReadTextFile(isaFileName, isaText, nullptr))
        {
            int  isaSize = beProgramBuilderLightning::GetIsaSize(isaText);
            if (isaSize != -1)
            {
                kernelCodeProps.isaSize = isaSize;
                status = beStatus_SUCCESS;
            }
        }
    }

    return status;
}

// Build the staistics in "AnalysisData" form.
static bool  BuildAnalysisData(const KernelCodeProps& kernelCodeProps, const std::string& device, AnalysisData& stats)
{
    uint64_t  minSGPRs = -1, minVGPRs = -1;

    // Set unknown values to -1.
    memset(&stats, 0, sizeof(AnalysisData));
    if (rgDeviceProps.count(device))
    {
        const DEVICE_PROPS& deviceProps = rgDeviceProps.at(device);
        stats.LDSSizeAvailable     = deviceProps.AVAILABLE_LDS_BYTES;
        stats.numSGPRsAvailable    = deviceProps.AVAILABLE_SGPRS;
        stats.numVGPRsAvailable    = deviceProps.AVAILABLE_VGPRS;
        minSGPRs = deviceProps.MIN_SGPRS;
        minVGPRs = deviceProps.MIN_VGPRS;
    }
    else
    {
        stats.LDSSizeAvailable     = -1;
        stats.numSGPRsAvailable    = -1;
        stats.numVGPRsAvailable    = -1;
    }
    stats.numThreadPerGroup    = -1;
    stats.numThreadPerGroupX   = -1;
    stats.numThreadPerGroupY   = -1;
    stats.numThreadPerGroupZ   = -1;

    stats.scratchMemoryUsed = kernelCodeProps.privateSegmentSize;
    stats.LDSSizeUsed       = kernelCodeProps.workgroupSegmentSize;
    stats.numSGPRsUsed      = max(minSGPRs, kernelCodeProps.wavefrontNumSGPRs);
    stats.numVGPRsUsed      = max(minVGPRs, kernelCodeProps.workitemNumVGPRs);
    stats.numSGPRSpills     = kernelCodeProps.SGPRSpills;
    stats.numVGPRSpills     = kernelCodeProps.VGPRSpills;
    stats.wavefrontSize     = ((CALuint64)1 << kernelCodeProps.wavefrontSize);
    stats.ISASize           = kernelCodeProps.isaSize;

    return true;
}

// Construct statistics text and store it to CSV file.
static bool  StoreStatistics(const Config& config, const std::string& baseStatFileName, const std::string& device,
                             const std::string& kernel, const AnalysisData& stats, std::string& outFileName)
{
    bool  ret = false;
    gtString  statFileName;
    kcUtils::ConstructOutputFileName(baseStatFileName, KC_STR_DEFAULT_STATISTICS_SUFFIX, kernel, device, statFileName);

    if (!statFileName.isEmpty())
    {
        outFileName = statFileName.asASCIICharArray();
        std::stringstream  statText;
        char separator = kcUtils::GetCsvSeparator(config);
        // Lambda returning "N/A" string if value = -1 or string representation of value itself otherwise.
        auto NAor = [](uint64_t val) { return (val == (int64_t)-1 ? STR_NA_VALUE : std::to_string(val)); };

        statText << kcUtils::GetStatisticsCsvHeaderString(separator) << std::endl;
        statText << device << separator;
        statText << NAor(stats.scratchMemoryUsed) << separator;
        statText << NAor(stats.numThreadPerGroup) << separator;
        statText << NAor(stats.wavefrontSize) << separator;
        statText << NAor(stats.LDSSizeAvailable) << separator;
        statText << NAor(stats.LDSSizeUsed) << separator;
        statText << NAor(stats.numSGPRsAvailable) << separator;
        statText << NAor(stats.numSGPRsUsed) << separator;
        statText << NAor(stats.numSGPRSpills) << separator;
        statText << NAor(stats.numVGPRsAvailable) << separator;
        statText << NAor(stats.numVGPRsUsed) << separator;
        statText << NAor(stats.numVGPRSpills) << separator;
        statText << NAor(stats.numThreadPerGroupX) << separator;
        statText << NAor(stats.numThreadPerGroupY) << separator;
        statText << NAor(stats.numThreadPerGroupZ) << separator;
        statText << NAor(stats.ISASize);
        statText << std::endl;

        ret = kcUtils::WriteTextFile(statFileName.asASCIICharArray(), statText.str(), nullptr);
    }

    return ret;
}

beStatus kcCLICommanderLightning::ExtractStatistics(const Config& config)
{
    std::string  device = "", statFileName = config.m_AnalysisFile, outStatFileName;
    beStatus     status = beStatus_SUCCESS;

    LogPreStep(KA_CLI_STR_EXTRACTING_STATISTICS);

    for (auto& outputMDItem : m_outputMetadata)
    {
        AnalysisData  statData;
        CodePropsMap  codeProps;
        const std::string& currentDevice = outputMDItem.first.first;
        if (device != currentDevice && outputMDItem.second.m_status)
        {
            status = beProgramBuilderLightning::ExtractKernelCodeProps(config.m_cmplrBinPath, outputMDItem.second.m_binFile,
                                                                       config.m_printProcessCmdLines, codeProps);
            if (status != beStatus_SUCCESS)
            {
                break;
            }
            for (auto& kernelCodeProps : codeProps)
            {
                if (config.m_Function.empty() || config.m_Function == kernelCodeProps.first)
                {
                    if (GetIsaSize(m_outputMetadata.at({ currentDevice, kernelCodeProps.first }).m_isaFile, kernelCodeProps.second) &&
                        BuildAnalysisData(kernelCodeProps.second, currentDevice, statData))
                    {
                        status = StoreStatistics(config, statFileName, currentDevice, kernelCodeProps.first, statData, outStatFileName) ?
                                     status : beStatus_WriteToFile_FAILED;
                        if (status == beStatus_SUCCESS)
                        {
                            auto outFiles = m_outputMetadata.find({currentDevice, kernelCodeProps.first});
                            if (outFiles != m_outputMetadata.end())
                            {
                                outFiles->second.m_statFile = outStatFileName;
                            }
                        }
                    }
                }
            }

            device = outputMDItem.first.first;
        }
    }

    LogResult(status == beStatus_SUCCESS);

    return status;
}

static void  GetherBranchTargets(std::stringstream& isa, std::unordered_map<std::string, bool>& branchTargets)
{
    // The format of branch instuction text:
    //
    //     s_cbranch_scc1 BB0_3        // 000000001110: BF85001C
    //           ^         ^                    ^          ^
    //           |         |                    |          |
    //      instruction  label               offset       code

    std::string  isaLine;

    // Skip lines before the actual ISA code.
    while (std::getline(isa, isaLine) && isaLine.find(LC_KERNEL_ISA_HEADER_3) == std::string::npos) {}

    // Gather target labes of all branch instructions.
    while (std::getline(isa, isaLine))
    {
        size_t  instEndOffset, branchTokenOffset, instOffset = isaLine.find_first_not_of(" \t");
        if (instOffset != std::string::npos)
        {
            if ((branchTokenOffset = isaLine.find(LC_ISA_BRANCH_TOKEN, instOffset)) != std::string::npos ||
                (branchTokenOffset = isaLine.find(LC_ISA_CALL_TOKEN, instOffset)) != std::string::npos)
            {
                if ((instEndOffset = isaLine.find_first_of(" \t", instOffset)) != std::string::npos &&
                    branchTokenOffset < instEndOffset)
                {
                    // Found branch instruction. Add its target label to the list.
                    size_t  labelStartOffset, labelEndOffset;
                    if ((labelStartOffset = isaLine.find_first_not_of(" \t", instEndOffset)) != std::string::npos &&
                        isaLine.compare(labelStartOffset, LC_ISA_INST_ADDR_START_TOKEN.size(), LC_ISA_INST_ADDR_START_TOKEN) != 0 &&
                        ((labelEndOffset = isaLine.find_first_of(" \t", labelStartOffset)) != std::string::npos))
                    {
                        branchTargets[isaLine.substr(labelStartOffset, labelEndOffset - labelStartOffset)] = true;
                    }
                }
            }
        }
    }
    isa.clear();
    isa.seekg(0);
}

// Checks if "isaLine" is a label that is not in the list of branch targets.
bool  IsUnreferencedLabel(const std::string& isaLine, const std::unordered_map<std::string, bool>& branchTargets)
{
    bool  ret = false;

    if (isaLine.find(':') == isaLine.size() - 1)
    {
        ret = (branchTargets.find(isaLine.substr(0, isaLine.size() - 1)) == branchTargets.end());
    }

    return ret;
}

// Remove non-standard instruction suffixes.
static void  FilterISALine(std::string & isaLine)
{
    size_t  offset = isaLine.find_first_not_of(" \t");
    if (offset != std::string::npos)
    {
        offset = isaLine.find_first_of(" ");
    }
    if (offset != std::string::npos)
    {
        size_t  suffixLen = 0;
        if (offset >= LC_ISA_INST_SUFFIX_1.size() &&
            isaLine.substr(offset - LC_ISA_INST_SUFFIX_1.size(), LC_ISA_INST_SUFFIX_1.size()) == LC_ISA_INST_SUFFIX_1)
        {
            suffixLen = LC_ISA_INST_SUFFIX_1.size();
        }
        else if (offset >= LC_ISA_INST_SUFFIX_2.size() &&
            isaLine.substr(offset - LC_ISA_INST_SUFFIX_2.size(), LC_ISA_INST_SUFFIX_2.size()) == LC_ISA_INST_SUFFIX_2)
        {
            suffixLen = LC_ISA_INST_SUFFIX_2.size();
        }
        // Remove the suffix.
        if (suffixLen != 0)
        {
            isaLine.erase(offset - suffixLen, suffixLen);
            // Restore the alignment of byte encoding.
            if ((offset = isaLine.find("//", offset)) != std::string::npos)
            {
                isaLine.insert(offset, suffixLen, ' ');
            }
        }
    }
}

// The Lightning Compiler may append useless code for some library functions to the ISA disassembly.
// This function eliminates such code.
// It also also removes unreferenced labels and non-standard instruction suffixes.
bool  kcCLICommanderLightning::ReduceISA(const std::string& binFile, IsaMap& kernelIsaTexts)
{
    bool  ret = false;

    for (auto& kernelIsa : kernelIsaTexts)
    {
        int  codeSize = beProgramBuilderLightning::GetKernelCodeSize(m_cmplrPaths.m_bin, binFile, kernelIsa.first, m_printCmds);
        assert(codeSize != -1);
        if (codeSize != -1)
        {
            // Copy ISA lines to new stream. Stop when found an instruction with address > codeSize.
            std::stringstream  oldIsa, newIsa, addrStream;
            oldIsa.str(kernelIsa.second);
            std::string  isaLine;
            int  addr, addrOffset = -1;

            // Gather the target labels of all branch instructions.
            std::unordered_map<std::string, bool>  branchTargets;
            branchTargets.clear();
            GetherBranchTargets(oldIsa, branchTargets);

            // Skip lines before the actual ISA code.
            while (std::getline(oldIsa, isaLine) && newIsa << isaLine << std::endl &&
                   isaLine.find(LC_KERNEL_ISA_HEADER_3) == std::string::npos) {}

            while (std::getline(oldIsa, isaLine))
            {
                // Add the ISA line to the new ISA text if it's not an unreferenced label.
                if (!IsUnreferencedLabel(isaLine, branchTargets))
                {
                    newIsa << isaLine << std::endl;
                }

                // Check if this instruction is the last one and we have to stop here.
                // Skip comment lines generated by disassembler.
                if (isaLine.find(LC_ISA_COMMENT_START_TOKEN) != 0)
                {
                    // Format of ISA disassembly instruction (64-bit and 32-bit):
                    //  s_load_dwordx2 s[0:1], s[6:7], 0x0     // 000000001108: C0060003 00000000
                    //  v_add_u32 v0, s8, v0                   // 000000001134: 68000008
                    //                                            `-- addr --'
                    size_t  addrStart, addrEnd;

                    FilterISALine(isaLine);

                    if ((addrStart = isaLine.find(LC_ISA_INST_ADDR_START_TOKEN)) != std::string::npos &&
                        (addrEnd = isaLine.find(LC_ISA_INST_ADDR_END_TOKEN, addrStart)) != std::string::npos)
                    {
                        addrStart += (LC_ISA_INST_ADDR_START_TOKEN.size() + 1);
                        addrStream.str(isaLine.substr(addrStart, addrEnd - addrStart));
                        addrStream.clear();
                        int instSize = (isaLine.size() - addrEnd < ISA_INST_64BIT_CODE_TEXT_SIZE) ? ISA_INST_32BIT_BYTES_SIZE : ISA_INST_64BIT_BYTES_SIZE;
                        if (addrStream >> std::hex >> addr)
                        {
                            // addrOffset is the binary address of 1st instruction.
                            addrOffset = (addrOffset == -1 ? addr : addrOffset);
                            if ((addr - addrOffset + instSize) >= codeSize)
                            {
                                ret = true;
                                break;
                            }
                        }
                        else
                        {
                            break;
                        }
                    }
                }
            }

            if (ret)
            {
                kernelIsa.second = newIsa.str();
            }
        }
    }

    return ret;
}

bool kcCLICommanderLightning::SplitISA(const std::string& binFile, const std::string& isaText,
                                       const std::string& userIsaFileName, const std::string& device,
                                       const std::string& kernel, const std::vector<std::string>& kernelNames)
{
    // kernelIsaTextMap maps kernel name --> kernel ISA text.
    IsaMap  kernelIsaTextMap;
    bool    ret, isIsaFileTemp = userIsaFileName.empty();

    // Split ISA text into per-kernel fragments.
    ret = SplitISAText(isaText, kernelNames, kernelIsaTextMap);

    // Eliminate the useless code.
    ret = ret && ReduceISA(binFile, kernelIsaTextMap);

    // Store per-kernel ISA texts to separate files and launch livereg tool for each file.
    if (ret)
    {
        // isaTextMapItem is a pair{kernelName, kernelIsaText}.
        for (const auto &isaTextMapItem : kernelIsaTextMap)
        {
            // Skip the kernels that are not requested.
            if (!kernel.empty() && kernel != isaTextMapItem.first)
            {
                continue;
            }

            gtString  isaFileName;
            if (isIsaFileTemp)
            {
                gtString  baseIsaFileName(LC_TEMP_ISA_FILE_NAME);
                baseIsaFileName << device.c_str() << "_" << isaTextMapItem.first.c_str();
                isaFileName = kcUtils::ConstructTempFileName(baseIsaFileName, LC_TEMP_ISA_FILE_EXT);
            }
            else
            {
                kcUtils::ConstructOutputFileName(userIsaFileName, KC_STR_DEFAULT_ISA_SUFFIX, isaTextMapItem.first, device, isaFileName);
            }
            if (!isaFileName.isEmpty())
            {
                if (kcUtils::WriteTextFile(isaFileName.asASCIICharArray(), isaTextMapItem.second, m_LogCallback))
                {
                    rgOutputFiles  outFiles = rgOutputFiles(rgEntryType::OpenCL_Kernel, isaFileName.asASCIICharArray());
                    outFiles.m_isIsaFileTemp = isIsaFileTemp;
                    m_outputMetadata[{device, isaTextMapItem.first}] = outFiles;
                }
            }
            else
            {
                std::stringstream  errMsg;
                errMsg << STR_ERR_FAILED_CREATE_TEMP_FILE << std::endl;
                m_LogCallback(errMsg.str());
                ret = false;
            }
        }
    }

    return ret;
}

bool kcCLICommanderLightning::SplitISAText(const std::string& isaText,
                                           const std::vector<std::string>& kernelNames,
                                           IsaMap& kernelIsaMap) const
{
    bool  status = true;
    const std::string  BLOCK_END_TOKEN      = "\n\n";
    const std::string  LABEL_NAME_END_TOKEN = ":\n";
    size_t  labelNameStart = 0, labelNameEnd = 0, kernelIsaEnd = 0, blockEnd = 0;

    labelNameStart = isaText.find_first_not_of('\n');

    // Gather the offsets of kernel names within the ISA text.
    // The format of each offset is:  pair{kernel_name_start, kernel_name_len}.
    std::vector<std::pair<size_t, size_t>>  kernelStartOffsets;
    if (!isaText.empty())
    {
        while ((labelNameEnd = isaText.find(LABEL_NAME_END_TOKEN, labelNameStart)) != std::string::npos)
        {
            // Check if this label is a kernel name.
            std::string  labelName = isaText.substr(labelNameStart, labelNameEnd - labelNameStart);
            if (std::count(kernelNames.begin(), kernelNames.end(), labelName) != 0)
            {
                kernelStartOffsets.push_back({labelNameStart, labelNameEnd - labelNameStart});
            }
            if ((labelNameStart = isaText.find(BLOCK_END_TOKEN, labelNameEnd)) == std::string::npos)
            {
                // End of file.
                break;
            }
            else
            {
                labelNameStart += BLOCK_END_TOKEN.size();
            }
        }
    }

    // Split the ISA text using collected offsets of kernel names.
    for (size_t i = 0, size = kernelStartOffsets.size(); i < size; i++)
    {
        size_t  isaTextStart = kernelStartOffsets[i].first;
        size_t  isaTextEnd = ( i < size - 1 ? kernelStartOffsets[i + 1].first - 1 : isaText.size() );
        if (isaTextStart <= isaTextEnd)
        {
            std::string  kernelIsa = isaText.substr(isaTextStart, isaTextEnd - isaTextStart);

            std::stringstream  kernelIsaText;
            const std::string& kernelName = isaText.substr(kernelStartOffsets[i].first, kernelStartOffsets[i].second);
            kernelIsaText << LC_KERNEL_ISA_HEADER_1 << "\"" << kernelName << "\"" << std::endl << std::endl
                << LC_KERNEL_ISA_HEADER_2 << "\"" << kernelName << "\":" << std::endl << std::endl << LC_KERNEL_ISA_HEADER_3;
            kernelIsaText << kernelIsa;
            kernelIsaMap[kernelName] = kernelIsaText.str();
            labelNameStart = kernelIsaEnd + BLOCK_END_TOKEN.size();
        }
        else
        {
            status = false;
            break;
        }
    }

    return status;
}

bool kcCLICommanderLightning::ListEntries(const Config& config, LoggingCallBackFunc_t callback)
{
    bool  ret = true;
    std::string  fileName;
    rgEntryData  entryData;
    std::stringstream  msg;

    if (config.m_SourceLanguage != SourceLanguage_OpenCL && config.m_SourceLanguage != SourceLanguage_Rocm_OpenCL)
    {
        msg << STR_ERR_COMMAND_NOT_SUPPORTED << std::endl;
        ret = false;
    }
    else
    {
        if (config.m_InputFiles.size() == 1)
        {
            fileName = config.m_InputFiles[0];
        }
        else if (config.m_InputFiles.size() > 1)
        {
            msg << STR_ERR_ONE_INPUT_FILE_EXPECTED << std::endl;
            ret = false;
        }
        else
        {
            msg << STR_ERR_NO_INPUT_FILE << std::endl;
            ret = false;
        }
    }

    if (ret && (ret = kcCLICommanderLightning::ExtractEntries(fileName, config, entryData)) == true)
    {
        // Sort the entry names in alphabetical order.
        std::sort(entryData.begin(), entryData.end(),
            [](const std::tuple<std::string, int, int>& a, const std::tuple<std::string, int, int>& b) {return (std::get<0>(a) < std::get<0>(b)); });

        // Dump the entry points.
        for (const auto& dataItem : entryData)
        {
            msg << std::get<0>(dataItem) << ": " << std::get<1>(dataItem) << "-" << std::get<2>(dataItem) << std::endl;
        }
        msg << std::endl;
    }

    callback(msg.str());

    return ret;
}

bool kcCLICommanderLightning::ExtractEntries(const std::string& fileName, const Config& config, rgEntryData& entryData)
{
    std::string  prepSrc;
    bool  ret = false;

    // Gather the options
    std::string  options = GatherOCLOptions(config);

    // Call ROCm compiler preprocessor.
    beStatus  status = beProgramBuilderLightning::PreprocessOpenCL("", fileName, options, config.m_printProcessCmdLines, prepSrc);

    if (status == beStatus_SUCCESS)
    {
        // Parse preprocessed source text and extract the kernel names.
        ret = ExtractEntriesPreprocessed(prepSrc, fileName, entryData);
    }
    else
    {
        // In case of error, prepSrc contains the error message printed by LC Preprocessor.
        LogErrorStatus(status, prepSrc);
    }

    return ret;
}

std::set<std::string> kcCLICommanderLightning::GetSupportedTargets()
{
    // Gather the supported devices in DeviceInfo format.
    std::set<std::string> devices;
    for (auto& d : LC_LLVM_TARGETS_TO_DEVICE_INFO_TARGETS) { devices.insert(d.second); }

    return devices;
}

// Convert ISA text to CSV form with additional data.
bool GetParsedIsaCSVText(const std::string& isaText, const std::string& device, bool addLineNumbers, std::string& csvText)
{
    bool  ret = false;
    std::string  parsedIsa;
    if (beProgramBuilder::ParseISAToCSV(isaText, device, parsedIsa, addLineNumbers, true) == beKA::beStatus_SUCCESS)
    {
        csvText = (addLineNumbers ? STR_CSV_PARSED_ISA_HEADER_LINE_NUMS : STR_CSV_PARSED_ISA_HEADER) + parsedIsa;
        ret = true;
    }
    return ret;
}

bool kcCLICommanderLightning::GenerateSessionMetadata(const Config& config, const rgOutputMetadata& outMetadata) const
{
    rgFileEntryData  fileKernelData;

    bool  ret = !config.m_sessionMetadataFile.empty();
    assert(ret);

    if (ret)
    {
        for (const std::string& inputFile : config.m_InputFiles)
        {
            rgEntryData  entryData;
            ret = ret && ExtractEntries(inputFile, config, entryData);
            if (ret)
            {
                fileKernelData[inputFile] = entryData;
            }
        }
    }

    if (ret && !outMetadata.empty())
    {
        ret = kcUtils::GenerateCliMetadataFile(config.m_sessionMetadataFile, fileKernelData, outMetadata);
    }

    return ret;
}

bool kcCLICommanderLightning::RunPostCompileSteps(const Config& config) const
{
    bool ret = false;
    if (!config.m_sessionMetadataFile.empty())
    {
        ret = GenerateSessionMetadata(config, m_outputMetadata);
        if (!ret)
        {
            std::stringstream  msg;
            msg << STR_ERR_FAILED_GENERATE_SESSION_METADATA << std::endl;
            m_LogCallback(msg.str());
        }
    }

    DeleteTempFiles();

    return ret;
}

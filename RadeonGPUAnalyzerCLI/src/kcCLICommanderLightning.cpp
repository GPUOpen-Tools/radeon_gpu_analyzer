
// C++.
#include <vector>
#include <map>
#include <utility>
#include <sstream>
#include <algorithm>
#include <iterator>

// Infra.
#include <AMDTOSWrappers/Include/osFilePath.h>

// Local.
#include <RadeonGPUAnalyzerCLI/src/kcCliStringConstants.h>
#include <RadeonGPUAnalyzerCLI/src/kcUtils.h>
#include <RadeonGPUAnalyzerCLI/src/kcCLICommanderLightning.h>
#include <RadeonGPUAnalyzerBackend/include/beProgramBuilderLightning.h>
#include <Core/ROCm/include/lc_targets.h>

// *****************************************
// *** INTERNALLY LINKED SYMBOLS - START ***
// *****************************************

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

static const std::string  LC_ROCM_ISA_INST_SUFFIX_1 = "_e32";
static const std::string  LC_ROCM_ISA_INST_SUFFIX_2 = "_e64";

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
    { {"gfx900", {102, 256, 32768, 16, 4}} };


// ***************************************
// *** INTERNALLY LINKED SYMBOLS - END ***
// ***************************************

static bool GetParsedIsaCSVText(const std::string& isaText, const std::string& device, bool lineNumbers, std::string& csvText);

// Get default target name.
static bool GetDefaultTarget(std::string& targetName, LoggingCallBackFunc_t logCallback)
{
    bool  ret = false;
    std::string matchedName, msg;

    // Look for the latest LC supported device that is also supported by DeviceInfo.
    for (auto targetIt = LC_TARGET_NAMES.crbegin(); targetIt != LC_TARGET_NAMES.crend(); targetIt++)
    {
        if (kcUtils::FindGPUArchName(*targetIt, matchedName, msg))
        {
            targetName = *targetIt;
            ret = true;
            break;
        }
    }

    if (!ret)
    {
        std::stringstream  msg;
        msg << STR_ERR_FAILED_GET_DEFAULT_TARGET << std::endl;
        logCallback(msg.str());
    }

    return ret;
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

beKA::beStatus kcCLICommanderLightning::Init(LoggingCallBackFunc_t logCallback)
{
    m_LC_targets  = LC_TARGET_NAMES;
    m_LogCallback = logCallback;

    return beKA::beStatus_SUCCESS;
}

// Checks if text[offset] is a start of OpenCL kernel qualifier ("kernel" of "__kernel" token).
// Spaces are ignored.
// The offset of the first symbol afther the qualifier is returned in "offset".
static bool  IsKernelQual(const std::string& text, unsigned char prevSymbol, size_t& offset)
{
    bool  found = false;

    if ((prevSymbol == ' ' || prevSymbol == '\n') && ((offset = text.find_first_not_of(" \n", offset)) != std::string::npos))
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
            hintItems.push_back(hintLine.substr(offset + 1, endOffset - offset - 1));
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

// Extracts list of kernel names from OpenCL source text.
// Returns kernel names in "entryData" as a vector of pairs {kernel_name, src_line}.
static bool  ExtractEntriesPreprocessed(std::string& text, const std::string& fileName, rgEntryData& entryData)
{
    //  # 1 "test.cl" 2
    //  # 12 "test.cl"   <-- preprocessor hint (file offset = 12)
    //
    //  __kernel void bar(global int *N)  <-- The number of this line in the original file =
    //                                        the number of this line in preprocessed file + file offet.

    size_t  offset = 0, kernelQualStart = 0, tokenStart = 0;
    unsigned int  fileOffset, lineNum = 0;
    unsigned char  prevSymbol = '\n';

    // Replace tabs with spaces.
    std::replace(text.begin(), text.end(), '\t', ' ');

    while (offset < text.size())
    {
        // Preprocessor hints start with '#'.
        if (prevSymbol == '\n' && text[offset] == '#' && text.compare(offset + 1, OPENCL_PRAGMA_TOKEN.size(), OPENCL_PRAGMA_TOKEN) != 0)
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
            offset = eol;
        }
        else if (text[offset] == '\n')
        {
            lineNum++;
        }
        else
        {
            // Look for "kernel" or "__kernel" qualifiers.
            kernelQualStart = offset;
            if (IsKernelQual(text, prevSymbol, offset))
            {
                // Skip "__attribute__(...)" if it's present.
                SkipAttributeQual(text, offset);

                // The kernel name is the last lexical token before "(" or "<" symbol.
                size_t  kernelNameEnd = text.find_first_of("(<", offset);
                if (kernelNameEnd != std::string::npos)
                {
                    bool  stop = false;
                    while (!stop)
                    {
                        size_t nextTokenStart = text.find_first_not_of(" \n", offset);
                        stop = (nextTokenStart == std::string::npos || nextTokenStart >= kernelNameEnd);
                        if (!stop)
                        {
                            tokenStart = nextTokenStart;
                            offset = text.find_first_of(" \n", tokenStart);
                        }
                    }
                    kernelNameEnd = min(kernelNameEnd, text.find_first_of(" \n", tokenStart));
                    std::string  kernelName = text.substr(tokenStart, kernelNameEnd - tokenStart);
                    if (!kernelName.empty())
                    {
                        // Store the found kernel name and corresponding line number to "entryData".
                        entryData.push_back({ kernelName, fileOffset == 0 ? 0 : fileOffset + lineNum });
                    }
                }
                // Count the number of lines between the kernel qualifier and the kernel name.
                lineNum += (unsigned)std::count(text.begin() + kernelQualStart, text.begin() + kernelNameEnd, '\n');
            }
        }
        offset++;
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

bool kcCLICommanderLightning::InitRequestedAsicListLC(const Config & config)
{
    bool  ret = true;

    if (config.m_ASICs.empty())
    {
        m_asics = m_LC_targets;
    }
    else
    {
        std::set<std::string>  userDevices;
        if ((ret = InitRequestedAsicList(config, m_LC_targets, userDevices, m_LogCallback)) == true)
        {
            for (auto & device : userDevices)
            {
                if (std::find(m_LC_targets.begin(), m_LC_targets.end(), device) == m_LC_targets.end())
                {
                    std::stringstream  msg;
                    msg << STR_ERR_ERROR << "'" << device << "'" << STR_ERR_NOT_KNOWN_ASIC << std::endl;
                    msg << STR_LIST_ASICS_HINT << std::endl;
                    m_LogCallback(msg.str());

                    ret = false;
                }
                m_asics.emplace(device);
            }
        }
    }

    return ret;
}

bool kcCLICommanderLightning::Compile(const Config & config)
{
    beKA::beStatus result = beKA::beStatus_SUCCESS;

    if (!InitRequestedAsicListLC(config))
        return false;

    // Prepare OpenCL options and defines.
    OpenCLOptions options;
    options.m_selectedDevices      = m_asics;
    options.m_defines              = config.m_Defines;
    options.m_incPaths             = config.m_IncludePath;
    options.m_openCLCompileOptions = config.m_OpenCLOptions;

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
    if (result == beKA::beStatus_SUCCESS && config.m_isParsedISARequired)
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

    beKA::beStatus  status = beProgramBuilderLightning::GetCompilerVersion(SourceLanguage_Rocm_OpenCL, outputText);
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
    return kcUtils::PrintAsicList(log, std::set<std::string>(), m_LC_targets);
}

beStatus kcCLICommanderLightning::CompileOpenCL(const Config& config, const OpenCLOptions& oclOptions)
{
    beStatus  status = beStatus_SUCCESS;

    // Run LC compiler for all requested devices
    for (const std::string & device : oclOptions.m_selectedDevices)
    {
        std::string  errText;
        LogPreStep(KA_CLI_STR_COMPILING, device);
        std::string  isaFileName, binFileName;
        gtString  ilFileName, binName;
        bool  dumpIL = !config.m_ILFile.empty();

        // Update the binary and ISA names for current device.
        beStatus currentStatus = AdjustBinaryFileName(config, device, binFileName);

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
        currentStatus = beProgramBuilderLightning::CompileOpenCLToBinary(oclOptions,
                                                                         srcFileNames,
                                                                         binFileName,
                                                                         device,
                                                                         dumpIL,
                                                                         config.m_isLineNumbersRequired,
                                                                         errText);
        LogResult(currentStatus == beStatus_SUCCESS);

        if (currentStatus == beStatus_SUCCESS)
        {
            // If "dump IL" option is passed to the ROCm compiler, it should dump the IL to stderr.
            if (dumpIL)
            {
                kcUtils::ConstructOutputFileName(config.m_ILFile, KC_STR_DEFAULT_LLVM_IR_SUFFIX, "", device, ilFileName);
                currentStatus = kcUtils::WriteTextFile(ilFileName.asASCIICharArray(), errText, nullptr) ?
                    beStatus_SUCCESS : beStatus_WriteToFile_FAILED;
            }

            // Disassemble binary to ISA text.
            if (!config.m_ISAFile.empty() || !config.m_AnalysisFile.empty() ||
                !config.m_LiveRegisterAnalysisFile.empty() || !config.m_ControlFlowGraphFile.empty())
            {
                LogPreStep(KA_CLI_STR_EXTRACTING_ISA, device);
                currentStatus = DisassembleBinary(binFileName, config.m_ISAFile, device, config.m_Function, config.m_isLineNumbersRequired, errText);
                LogResult(currentStatus == beStatus_SUCCESS);

                // Propagate the binary file name to the Output Files Metadata table.
                for (auto& outputMDNode : m_outputMetadata)
                {
                    const std::string& mdDevice = outputMDNode.first.first;
                    if (mdDevice == device)
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

    beKA::beStatus status = beProgramBuilderLightning::DisassembleBinary(binFileName, device, lineNumbers, outIsaText, errText);

    if (status == beKA::beStatus_SUCCESS)
    {
        status = beProgramBuilderLightning::ExtractKernelNames(binFileName, kernelNames);
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
            status = SplitISA(outIsaText, userIsaFileName, device, kernel, kernelNames) ?
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

    return ret;
}

void kcCLICommanderLightning::RunCompileCommands(const Config & config, LoggingCallBackFunc_t logCallback)
{
    bool  status = Compile(config);

    // Perform Live Registers analysis if required.
    if (status && !config.m_LiveRegisterAnalysisFile.empty())
    {
        PerformLiveRegAnalysis(config);
    }

    // Extract Control Flow Graph.
    if (status && !config.m_ControlFlowGraphFile.empty())
    {
        ExtractCFG(config);
    }

    // Extract CodeObj metadata if required.
    if (status && !config.m_MetadataFile.empty())
    {
        ExtractMetadata(config.m_MetadataFile);
    }

    // Extract Statistics if required.
    if (status && !config.m_AnalysisFile.empty())
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
        const std::string& device      = outputMDItem.first.first;
        const std::string& entryName   = outputMDItem.first.second;
        gtString  liveRegOutFileName = L"";
        gtString  isaFileName;
        isaFileName << outputFiles.m_isaFile.c_str();

        // Construct a name for the output livereg file.
        kcUtils::ConstructOutputFileName(config.m_LiveRegisterAnalysisFile, KC_STR_DEFAULT_LIVE_REG_ANALYSIS_SUFFIX,
                                         entryName, device, liveRegOutFileName);
        if (!liveRegOutFileName.isEmpty())
        {
            kcUtils::PerformLiveRegisterAnalysis(isaFileName, liveRegOutFileName, m_LogCallback);

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

    LogResult(ret);

    if (!ret)
    {
        m_LogCallback(errMsg.str());
    }

    return ret;
}

bool kcCLICommanderLightning::ExtractCFG(const Config & config)
{
    bool  ret = true;
    std::stringstream  errMsg;

    LogPreStep(KA_CLI_STR_STARTING_CFG);

    for (auto& outputMDItem : m_outputMetadata)
    {
        rgOutputFiles& outputFiles = outputMDItem.second;
        const std::string& device = outputMDItem.first.first;
        const std::string& entryName = outputMDItem.first.second;
        gtString  cfgOutFileName = L"";
        gtString  isaFileName;
        isaFileName << outputFiles.m_isaFile.c_str();

        // Construct a name for the output livereg file.
        kcUtils::ConstructOutputFileName(config.m_ControlFlowGraphFile, KC_STR_DEFAULT_CFG_SUFFIX, entryName, device, cfgOutFileName);
        if (!cfgOutFileName.isEmpty())
        {
            kcUtils::GenerateControlFlowGraph(isaFileName, cfgOutFileName, m_LogCallback);
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
        const std::string& device = outputMDNode.first.first;
        if (devices.count(device) == 0)
        {
            devices.insert(device);
            const std::string  binFileName = outputMDNode.second.m_binFile;

            kcUtils::ConstructOutputFileName(metadataFileName, KC_STR_DEFAULT_LC_METADATA_SUFFIX, "", device, outFileName);
            if (!outFileName.isEmpty())
            {
                currentStatus = beProgramBuilderLightning::ExtractMetadata(binFileName, metadataText);
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

    if (status != beKA::beStatus_SUCCESS)
    {
        std::stringstream  msg;
        msg << STR_ERR_FAILED_EXTRACT_METADATA << std::endl;
        m_LogCallback(msg.str());
    }

    return status;
}

// Get the ISA size and store it to "kernelCodeProps" structure.
static beStatus  GetIsaSize(const std::string isaFileName, KernelCodeProps& kernelCodeProps)
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
    stats.maxScratchRegsNeeded = -1;
    stats.numThreadPerGroup    = -1;
    stats.numThreadPerGroupX   = -1;
    stats.numThreadPerGroupY   = -1;
    stats.numThreadPerGroupZ   = -1;
    stats.LDSSizeUsed   = kernelCodeProps.workgroupGroupSegmentSize;
    stats.numSGPRsUsed  = max(minSGPRs, kernelCodeProps.wavefrontNumSGPRs);
    stats.numVGPRsUsed  = max(minVGPRs, kernelCodeProps.workitemNumVGPRs);
    stats.wavefrontSize = ((CALuint64)1 << kernelCodeProps.wavefrontSize);
    stats.ISASize       = kernelCodeProps.isaSize;

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
        statText << NAor(stats.maxScratchRegsNeeded) << separator;
        statText << NAor(stats.wavefrontSize) << separator;
        statText << NAor(stats.LDSSizeAvailable) << separator;
        statText << NAor(stats.LDSSizeUsed) << separator;
        statText << NAor(stats.numSGPRsAvailable) << separator;
        statText << NAor(stats.numSGPRsUsed) << separator;
        statText << NAor(stats.numVGPRsAvailable) << separator;
        statText << NAor(stats.numVGPRsUsed) << separator;
        statText << NAor(stats.numThreadPerGroupX) << separator;
        statText << NAor(stats.numThreadPerGroupY) << separator;
        statText << NAor(stats.numThreadPerGroupZ) << separator;
        statText << NAor(stats.ISASize);
        statText << std::endl;

        ret = kcUtils::WriteTextFile(statFileName.asASCIICharArray(), statText.str(), nullptr);
    }

    return ret;
}

beStatus kcCLICommanderLightning::ExtractStatistics(const Config& config) const
{
    std::string  device = "", statFileName = config.m_AnalysisFile, outStatFileName;
    beStatus     status = beStatus_SUCCESS;

    LogPreStep(KA_CLI_STR_EXTRACTING_STATISTICS);

    for (auto outputMDItem : m_outputMetadata)
    {
        AnalysisData  statData;
        CodePropsMap  codeProps;
        rgOutputFiles&  outFiles = outputMDItem.second;
        const std::string& currentDevice = outputMDItem.first.first;
        if (device != currentDevice)
        {
            status = beProgramBuilderLightning::ExtractKernelCodeProps(outFiles.m_binFile, codeProps);
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
                            outFiles.m_statFile = outStatFileName;
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

bool kcCLICommanderLightning::SplitISA(const std::string& isaText, const std::string& userIsaFileName,
                                       const std::string& device, const std::string& kernel,
                                       const std::vector<std::string>& kernelNames)
{
    // kernelIsaTextMap maps kernel name --> kernel ISA text.
    IsaMap  kernelIsaTextMap;

    // Filter the ISA text.
    std::string  filteredIsaText;
    bool  ret = FilterISA(isaText, filteredIsaText);
    bool  isIsaFileTemp = userIsaFileName.empty();

    // Split ISA text into per-kernel fragments.
    if (ret)
    {
        ret = SplitISAText(filteredIsaText, kernelNames, kernelIsaTextMap);
    }

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
                kcUtils::ConstructOutputFileName(userIsaFileName, "", isaTextMapItem.first, device, isaFileName);
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

bool kcCLICommanderLightning::FilterISA(const std::string & isaText, std::string& filteredIsaText)
{
    bool  ret = !isaText.empty();

    if (ret)
    {
        // Remove the instruction suffixes that are generated by the ROCm disassembler and can be ignored.
        std::stringstream isa(isaText), filteredIsa;
        std::string  line;
        while (std::getline(isa, line))
        {
            size_t  offset = line.find_first_not_of(" \t");
            if (offset != std::string::npos)
            {
                offset = line.find_first_of(" ");
            }
            if (offset != std::string::npos)
            {
                size_t  suffixLen = 0;
                if (offset >= LC_ROCM_ISA_INST_SUFFIX_1.size() &&
                    line.substr(offset - LC_ROCM_ISA_INST_SUFFIX_1.size(), LC_ROCM_ISA_INST_SUFFIX_1.size()) == LC_ROCM_ISA_INST_SUFFIX_1)
                {
                    suffixLen = LC_ROCM_ISA_INST_SUFFIX_1.size();
                }
                else if (offset >= LC_ROCM_ISA_INST_SUFFIX_2.size() &&
                         line.substr(offset - LC_ROCM_ISA_INST_SUFFIX_2.size(), LC_ROCM_ISA_INST_SUFFIX_2.size()) == LC_ROCM_ISA_INST_SUFFIX_2)
                {
                    suffixLen = LC_ROCM_ISA_INST_SUFFIX_2.size();
                }
                // Remove the suffix.
                if (suffixLen != 0)
                {
                    line.erase(offset - suffixLen, suffixLen);
                    // Restore the alignment of byte encoding.
                    if ((offset = line.find("//", offset)) != std::string::npos)
                    {
                        line.insert(offset, suffixLen, ' ');
                    }
                }
            }
            filteredIsa << line << std::endl;
        }
        filteredIsaText = filteredIsa.str();
    }

    return ret;
}

bool kcCLICommanderLightning::ExtractEntries(const std::string & fileName, const Config & config, rgEntryData& entryData)
{
    std::string  prepSrc;
    bool  ret = false;

    // Gather the options
    std::string  options = GatherOCLOptions(config);

    // Call ROCm compiler preprocessor.
    beStatus  status = beProgramBuilderLightning::PreprocessOpenCL(fileName, options, prepSrc);

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
    return LC_TARGET_NAMES;
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

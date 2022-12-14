// C++.
#include <vector>
#include <map>
#include <utility>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <cassert>
#include <regex>

// Infra.
#ifdef _WIN32
    #pragma warning(push)
    #pragma warning(disable:4309)
#endif
#include "external/amdt_os_wrappers/Include/osFilePath.h"
#include "external/amdt_os_wrappers/Include/osDirectory.h"
#include "external/amdt_os_wrappers/Include/osApplication.h"
#ifdef _WIN32
    #pragma warning(pop)
#endif
#include "common/rg_log.h"

// Local.
#include "radeon_gpu_analyzer_cli/kc_cli_string_constants.h"
#include "radeon_gpu_analyzer_cli/kc_utils.h"
#include "radeon_gpu_analyzer_cli/kc_xml_writer.h"
#include "radeon_gpu_analyzer_cli/kc_cli_commander_lightning.h"

// Backend.
#include "radeon_gpu_analyzer_backend/be_program_builder_lightning.h"
#include "radeon_gpu_analyzer_backend/be_string_constants.h"

// *****************************************
// *** INTERNALLY LINKED SYMBOLS - START ***
// *****************************************

// Targets of Lightning Compiler in LLVM format and corresponding DeviceInfo names.
static const std::vector<std::pair<std::string, std::string>>
kLcLlvmTargetsToDeviceInfoTargets = { {"gfx801", "carrizo"},
                                       {"gfx802", "tonga"},
                                       {"gfx803", "fiji"},
                                       {"gfx803", "ellesmere"},
                                       {"gfx803", "baffin"},
                                       {"gfx803", "gfx804"},
                                       {"gfx900", "gfx900"},
                                       {"gfx902", "gfx902"},
                                       {"gfx906", "gfx906"},
                                       {"gfx908", "gfx908"},
                                       {"gfx90c", "gfx90c"},
                                       {"gfx1010", "gfx1010"},
                                       {"gfx1011", "gfx1011"},
                                       {"gfx1012", "gfx1012"},
                                       {"gfx1030", "gfx1030"},
                                       {"gfx1031", "gfx1031"},
                                       {"gfx1032", "gfx1032"},
                                       {"gfx1034", "gfx1034"},
                                       {"gfx1035", "gfx1035"},
                                       {"gfx1100", "gfx1100"}};

// For some devices, clang does not accept device names that RGA gets from DeviceInfo.
// This table maps the DeviceInfo names to names accepted by clang for such devices.
static const std::map<std::string, std::string>
kLcDeviceInfoToClangDeviceMap = { {"ellesmere", "polaris10"},
                                  {"baffin",    "polaris11"},
                                  {"gfx804",    "gfx803"} };

// Default target for Lightning Compiler (the latest supported target).
static const std::string  kLcDefaultTarget = kLcLlvmTargetsToDeviceInfoTargets.rbegin()->second;

static const gtString  kTempBinaryFilename                  = L"rga_lc_ocl_out";
static const gtString  kTempBinaryFileExtension             = L"bin";
static const gtString  kTempIsaFilename                     = L"rga_lc_isa_";
static const gtString  kTempIsaFileExtension                = L"isa";

static const std::string  kLcKernelIsaHeader1               = "AMD Kernel Code for ";
static const std::string  kLcKernelIsaHeader2               = "Disassembly for ";
static const std::string  kLcKernelIsaHeader3               = "@kernel ";

static const std::string  kCompilerVersionToken             = "clang version ";
static const std::string  kCompilerWarningToken             = "warning:";

static const std::string  kLcIsaInstructionSuffix1          = "_e32";
static const std::string  kLcIsaInstructionSuffix2          = "_e64";

static const std::string  kLcIsaBranchToken                 = "branch";
static const std::string  kIsaCallToken                     = "call";

static const std::string  kIsaInstructionAddressStartToken  = "//";
static const std::string  kIsaInstructionAddressEndToken    = ":";
static const std::string  kIsaCommentStartToken             = ";";

static const std::string  kOpenclKernelQualifierToken1      = "__kernel";
static const std::string  kOpenclKernelQualifierToken2      = "kernel";
static const std::string  kOpenclAttributeQualifierToken    = "__attribute__";
static const std::string  kOpenclPragmaToken                = "pragma";

static const std::string  kStrDx11NaValue                       = "N/A";

const char* const kStrKernelName = "Kernel name: ";

// Error messages.
static const char* kStrErrorOpenclOfflineCompileError = "Error (reported by the OpenCL Compiler):";
static const char* kStrErrorOpenclOfflineCompileTimeout = "Error: the compilation process timed out.";
static const char* kStrErrorOpenclOfflineNoOutputFileGenerated = "Error: the output file was not generated.";
static const char* kStrErrorOpenclOfflineDisassemblerError = "Error: extracting ISA failed. The disassembler returned error:";
static const char* kStrErrorOpenclOfflineCannotFindKernel = "Error: cannot find OpenCL kernel: ";
static const char* kStrErrorOpenclOfflineUnknownDevice1 = "Error: unknown device name provided: ";
static const char* kStrErrorOpenclOfflineUnknownDevice2 = ". Cannot compile for this target.";
static const char* kStrErrorOpenclOfflineFailedToCreateTempFile = "Error: failed to create a temp file.";
static const char* kStrErrorOpenclOfflineFailedToCreateOutputFilenameForKernel = "Error: failed to construct output file name for kernel: ";
static const char* kStrErrorOpenclOfflineFailedToExtractMetadata = "Error: failed to extract Metadata.";
static const char* kStrErrorOpenclOfflineSplitIsaError = "Error: Unable to split ISA contents.";
static const char* kStrErrorOpenclOfflineLlvmIrDisassemblyFailure = "Error: failed to generate LLVM IR disassembly.";

// Warning messages.
static const char* kStrWarningOpenclOfflineUsingExtraDevice1 = "Warning: using unknown target GPU: ";
static const char* kStrWarningRocmclUsingExtraDevice2 = "; successful compilation and analysis are not guaranteed.";

// Info messages.
static const char* kStrInfoOpenclOfflinePerformingLiveregAnalysis = "Performing live register analysis";
static const char* kStrInfoOpenclOfflinePerformingStallAnalysis = "Performing stall analysis";
static const char* kStrInfoOpenclOfflineExtractingCfg = "Extracting control flow graph";
static const char* kStrInfoOpenclOfflineKernelForKernel = " for kernel ";

// The device properties necessary for generating statistics.
struct DeviceProps
{
    uint64_t  available_sgprs;
    uint64_t  available_vgprs;
    uint64_t  available_lds_bytes;
    uint64_t  min_sgprs;
    uint64_t  min_vgprs;
};

static const std::map<std::string, DeviceProps> kRgaDeviceProps =
    { {"carrizo",   {102, 256, 65536, 16,  4}},
      {"tonga",     {102, 256, 65536, 16, 64}},
      {"fiji",      {102, 256, 65536, 16,  4}},
      {"ellesmere", {102, 256, 65536, 16,  4}},
      {"baffin",    {102, 256, 65536, 16,  4}},
      {"polaris10", {102, 256, 65536, 16,  4}},
      {"polaris11", {102, 256, 65536, 16,  4}},
      {"gfx804",    {102, 256, 65536, 16,  4}},
      {"gfx900",    {102, 256, 65536, 16,  4}},
      {"gfx902",    {102, 256, 65536, 16,  4}},
      {"gfx906",    {102, 256, 65536, 16,  4}},
      {"gfx908",    {102, 256, 65536, 16,  4}},
      {"gfx90c",    {102, 256, 65536, 16,  4}},
      {"gfx1010",   {104, 256, 65536, 16,  4}},
      {"gfx1011",   {104, 256, 65536, 16,  4}},
      {"gfx1012",   {104, 256, 65536, 16,  4}},
      {"gfx1030",   {104, 256, 65536, 16,  4}},
      {"gfx1031",   {104, 256, 65536, 16,  4}},
      {"gfx1032",   {104, 256, 65536, 16,  4}},   
      {"gfx1034",   {104, 256, 65536, 16,  4}},
      {"gfx1035",   {104, 256, 65536, 16,  4}},
      {"gfx1100",   {104, 256, 65536, 16,  4}} };

static const size_t  kIsaInstruction64BitCodeTextSize   = 16;
static const int     kIsaInstruction64BitBytes          = 8;
static const int     kIsaInstruction32BitBytes          = 4;

// ***************************************
// *** INTERNALLY LINKED SYMBOLS - END ***
// ***************************************

// Get default target name.
static std::string GetDefaultTarget()
{
    return kLcDefaultTarget;
}

// Returns the list of additional LC targets specified in the "additional-targets" file.
static std::vector<std::string> GetExtraTargetList()
{
    static const wchar_t* kLcExtraTargetsFilename = L"additional-targets";
    std::vector<std::string> device_list;
    osFilePath targets_file_path;
    osGetCurrentApplicationPath(targets_file_path, false);
    targets_file_path.appendSubDirectory(kLcOpenclRootDir);
    targets_file_path.setFileName(kLcExtraTargetsFilename);
    targets_file_path.clearFileExtension();

    std::ifstream targets_file(targets_file_path.asString().asASCIICharArray());
    if (targets_file.good())
    {
        std::string device;
        while (std::getline(targets_file, device))
        {
            if (device.find("//") == std::string::npos)
            {
                // Save the target name in lower case to avoid case-sensitivity issues.
                std::transform(device.begin(), device.end(), device.begin(), [](const char& c) {return std::tolower(c); });
                device_list.push_back(device);
            }
        }
    }

    return device_list;
}

static void  LogPreStep(const std::string& msg, const std::string& device = "")
{
    std::cout << msg << device << "... ";
}

static void  LogResult(bool result)
{
    std::cout << (result ? kStrInfoSuccess : kStrInfoFailed) << std::endl;
}

static void  LogErrorStatus(beStatus status, const std::string & error_msg)
{
    const char* kStrErrorCannotFindBinary = "Error: cannot find binary file.";
    switch (status)
    {
    case kBeStatusSuccess:
        break;
    case kBeStatusLightningCompilerLaunchFailed:
        std::cout << std::endl << kStrErrorCannotInvokeCompiler << std::endl;
        break;
    case kBeStatusLightningCompilerGeneratedError:
        std::cout << std::endl << kStrErrorOpenclOfflineCompileError << std::endl;
        std::cout << error_msg << std::endl;
        break;
    case kBeStatusNoOutputFileGenerated:
        std::cout << std::endl << kStrErrorOpenclOfflineNoOutputFileGenerated << std::endl;
        break;
    case kBeStatusNoBinaryForDevice:
        std::cout << std::endl << kStrErrorCannotFindBinary << std::endl;
        break;
    case kBeStatusLightningDisassembleFailed:
        std::cout << std::endl << kStrErrorOpenclOfflineDisassemblerError << std::endl;
        std::cout << error_msg << std::endl;
        break;
    case kBeStatusLightningCompilerTimeOut:
        std::cout << std::endl << kStrErrorOpenclOfflineCompileTimeout << std::endl;
        std::cout << error_msg << std::endl;
        break;
    case kBeStatusLightningSplitIsaFailed:
        std::cout << std::endl << kStrErrorOpenclOfflineSplitIsaError << std::endl;
        std::cout << error_msg << std::endl;
        break;
    default:
        std::cout << std::endl << (error_msg.empty() ? kStrErrorUnknownCompilationStatus : error_msg) << std::endl;
        break;
    }
}

beKA::beStatus KcCLICommanderLightning::Init(const Config& config, LoggingCallbackFunction log_callback)
{
    log_callback_ = log_callback;
    compiler_paths_  = {config.compiler_bin_path, config.compiler_inc_path, config.compiler_lib_path};
    should_print_cmd_   = config.print_process_cmd_line;
    return beKA::kBeStatusSuccess;
}

// Checks if text[offset] is a start of OpenCL kernel qualifier ("kernel" of "__kernel" token).
// Spaces are ignored.
// The offset of the first symbol after the qualifier is returned in "offset".
inline static bool  IsKernelQual(const std::string& text, unsigned char prev_symbol, size_t& offset)
{
    bool is_found = false;
    if ((prev_symbol == ' ' || prev_symbol == '\n' || prev_symbol == '}'))
    {
        size_t  qual_size = 0;
        if (text.compare(offset, kOpenclKernelQualifierToken1.size(), kOpenclKernelQualifierToken1) == 0)
        {
            qual_size = kOpenclKernelQualifierToken1.size();
        }
        else if (text.compare(offset, kOpenclKernelQualifierToken2.size(), kOpenclKernelQualifierToken2) == 0)
        {
            qual_size = kOpenclKernelQualifierToken2.size();
        }

        if (qual_size != 0 && (text[offset + qual_size] == ' ' || text[offset + qual_size] == '\n'))
        {
            offset += qual_size;
            is_found = true;
        }
    }
    return is_found;
}

// Skips the "__attribute__((...))" qualifier.
// Sets "offset" to point to the first symbol after the qualifier.
static void  SkipAttributeQual(const std::string& text, size_t& offset)
{
    if ((offset = text.find_first_not_of(" \n", offset)) != std::string::npos)
    {
        if (text.compare(offset, kOpenclAttributeQualifierToken.size(), kOpenclAttributeQualifierToken) == 0)
        {
            // Skip the attribute arguments as well.
            offset += kOpenclAttributeQualifierToken.size();
            size_t  current_offset = offset;
            if ((current_offset = text.find_first_of('(', current_offset)) != std::string::npos)
            {
                uint32_t parent_count = 1;
                while (++current_offset < text.size() && parent_count > 0)
                {
                    parent_count += (text[current_offset] == '(' ? 1 : (text[current_offset] == ')' ? -1 : 0));
                }
                offset = current_offset;
            }
        }
    }
}

// Parse a preprocessor hint.
// Example:
//    "# 2 "some folder/test.cl" 24"
// Output:
//    {"2", "some folder/test.cl", "24"}
static void  ParsePreprocessorHint(const std::string& hint_line, std::vector<std::string>& hint_items)
{
    hint_items.clear();
    size_t  end_offset, offset = 0;
    while ((offset = hint_line.find_first_not_of(' ', offset)) != std::string::npos)
    {
        if (hint_line[offset] == '"' && (end_offset = hint_line.find('"', offset + 1)) != std::string::npos)
        {
            // The preprocessor generates double back-slash as a path delimiter on Windows.
            // Replace them with single slashes here.
            std::string file_path = hint_line.substr(offset + 1, end_offset - offset - 1);
            char prev = 0;
            auto found = [&](char& c) { bool ret = (prev == '\\' && c == '\\'); prev = c; return ret; };
            file_path.erase(std::remove_if(file_path.begin(), file_path.end(), found), file_path.end());
            hint_items.push_back(file_path);
            offset = end_offset + 1;
        }
        else
        {
            end_offset = hint_line.find_first_of(' ', offset);
            hint_items.push_back(hint_line.substr(offset, (end_offset != std::string::npos ? end_offset - offset : end_offset)));
            offset = (end_offset != std::string::npos ? end_offset + 1 : hint_line.size());
        }
    }
}

// Parse a preprocessor line that starts with '#'.
// Returns updated offset.
static size_t  ParsePreprocessorLine(const std::string& text, const std::string& filename, size_t offset,
                                     unsigned int& file_offset, unsigned int& line_number)
{
    // Parse the preprocessor hint line to get the file name and line offset.
    // We are interested in hints like:  # <file_offset> <file_name>
    //                              or:  # <file_offset> <file_name> 2
    // If the source file found in the hint is not "our" file, put 0 as file offset.
    size_t  eol = text.find_first_of('\n', offset);
    std::vector<std::string>  hint_items;
    ParsePreprocessorHint(text.substr(offset + 1, eol - offset - 1), hint_items);
    if (hint_items.size() == 2 || (hint_items.size() == 3 && std::atoi(hint_items[2].c_str()) == 2))
    {
        unsigned int offset = std::atoi(hint_items[0].c_str());
        if (offset > 0 && hint_items[1] == filename)
        {
            file_offset = offset;
            line_number = 0;
        }
        else
        {
            file_offset = 0;
        }
    }

    return eol;
}

// Parses a kernel declaration. Puts kernel name and starting line number to the "entryDeclInfo".
// Returns "true" if successfully parsed the kernel declaration or "false" otherwise.
static bool  ParseKernelDecl(const std::string& text, size_t& offset, unsigned int file_offset, size_t kernel_qual_start,
                             unsigned int& line_number, std::tuple<std::string, int, int>& entry_decl_info)
{
    bool ret = false;

    // Skip "__attribute__(...)" if it's present.
    SkipAttributeQual(text, offset);

    // The kernel name is the last lexical token before "(" or "<" symbol.
    size_t kernel_name_start, kernel_name_end;
    if ((kernel_name_end = text.find_first_of("(<", offset)) != std::string::npos)
    {
        if ((kernel_name_end = text.find_last_not_of(" \n", kernel_name_end - 1)) != std::string::npos &&
            (kernel_name_start = text.find_last_of(" \n", kernel_name_end)) != std::string::npos)
        {
            kernel_name_start++;
            std::string  kernel_name = text.substr(kernel_name_start, kernel_name_end - kernel_name_start + 1);
            offset = kernel_name_end;
            if (!kernel_name.empty())
            {
                // Store the found kernel name and corresponding line number to "entryDeclInfo".
                std::get<0>(entry_decl_info) = kernel_name;
                std::get<1>(entry_decl_info) = (file_offset == 0 ? 0 : file_offset + line_number);
                ret = true;
            }
        }
    }

    // Count the number of lines between the kernel qualifier and the kernel name.
    line_number += (unsigned)std::count(text.begin() + kernel_qual_start, text.begin() + kernel_name_end, '\n');
    return ret;
}

// Extracts list of kernel names from OpenCL source text.
// Returns kernel names in "entryData" as a vector of pairs {kernel_name, src_line}.
static bool  ExtractEntriesPreprocessed(std::string& text, const std::string& file_name, RgEntryData& entry_data)
{
    //  # 1 "test.cl" 2
    //  # 12 "test.cl"   <-- preprocessor hint (file offset = 12)
    //
    //  __kernel void bar(global int *N)  <-- The number of this line in the original file =
    //                                        the number of this line in preprocessed file + file offet.

    size_t offset = 0, kernel_qual_start = 0, size = text.size();
    unsigned int file_offset, line_number = 0, bracket_count = 0;
    unsigned char prev_symbol = '\n';
    bool  in_kernel = false;
    std::tuple<std::string, int, int>  entry_decl_info;

    // Replace tabs with spaces.
    std::replace(text.begin(), text.end(), '\t', ' ');

    // Start parsing.
    while (offset < size)
    {
        switch (text[offset])
        {
        case ' ' : break;
        case '\n': line_number++; break;
        case '"' : while (++offset < size && (text[offset] != '"'  || text[offset - 1] == '\\')) {}; break;
        case '\'': while (++offset < size && (text[offset] != '\'' || text[offset - 1] == '\\')) {}; break;
        case '{' : bracket_count++; break;

        case '}' :
            if (--bracket_count == 0 && in_kernel)
            {
                // Found the end of kernel body. Store the current line number.
                std::get<2>(entry_decl_info) = (file_offset == 0 ? 0 : file_offset + line_number);
                entry_data.push_back(entry_decl_info);
                in_kernel = false;
            }
            break;

        case '#':
            if (prev_symbol == '\n' && text.compare(offset + 1, kOpenclPragmaToken.size(), kOpenclPragmaToken) != 0)
            {
                offset = ParsePreprocessorLine(text, file_name, offset, file_offset, line_number);
            }
            break;

        default:
            // Look for "kernel" or "__kernel" qualifiers.
            kernel_qual_start = offset;
            if (IsKernelQual(text, prev_symbol, offset))
            {
                in_kernel = ParseKernelDecl(text, offset, file_offset, kernel_qual_start, line_number, entry_decl_info);
            }
        }
        prev_symbol = text[offset++];
    }

    return true;
}

// Gather the definitions and include paths into a single "options" string.
static std::string  GatherOCLOptions(const Config& config)
{
    std::stringstream opt_stream;
    for (const std::string& def : config.defines)
    {
        opt_stream << "-D" << def << " ";
    }
    for (const std::string& inc : config.include_path)
    {
        opt_stream << "-I" << inc << " ";
    }
    return opt_stream.str();
}

bool KcCLICommanderLightning::InitRequestedAsicListLC(const Config& config)
{
    bool ret = false;

    if (config.asics.empty())
    {
        // Use default target if no target is specified by user.
        targets_.insert(kLcDefaultTarget);
        ret = true;
    }
    else
    {
        for (std::string device : config.asics)
        {
            std::set<std::string> supported_targets;
            std::string matched_arch_name;

            bool is_supported_target_extracted = GetSupportedTargets(supported_targets);
            assert(is_supported_target_extracted);

            // If the device is specified in the LLVM format, convert it to the DeviceInfo format.
            auto llvm_device = std::find_if(kLcLlvmTargetsToDeviceInfoTargets.cbegin(), kLcLlvmTargetsToDeviceInfoTargets.cend(),
                                           [&](const std::pair<std::string, std::string>& d){ return (d.first == device);});
            if (llvm_device != kLcLlvmTargetsToDeviceInfoTargets.cend())
            {
                device = llvm_device->second;
            }

            // Try to detect device.
            if ((KcUtils::FindGPUArchName(device, matched_arch_name, true, true)) == true)
            {
                // Check if the matched architecture name is present in the list of supported devices.
                for (std::string supported_device : supported_targets)
                {
                    if (KcUtils::ToLower(matched_arch_name).find(supported_device) != std::string::npos)
                    {
                        targets_.insert(supported_device);
                        ret = true;
                        break;
                    }
                }
            }

            if (!ret)
            {
                // Try additional devices from "additional-targets" file.
                std::vector<std::string> extra_devices = GetExtraTargetList();
                std::transform(device.begin(), device.end(), device.begin(), [](const char& c) {return std::tolower(c); });
                if (std::find(extra_devices.cbegin(), extra_devices.cend(), device) != extra_devices.cend())
                {
                    RgLog::stdErr << kStrWarningOpenclOfflineUsingExtraDevice1 << device << kStrWarningRocmclUsingExtraDevice2 << std::endl << std::endl;
                    targets_.insert(device);
                    ret = true;
                }
            }

            if (!ret)
            {
                RgLog::stdErr << kStrErrorOpenclOfflineUnknownDevice1 << device << kStrErrorOpenclOfflineUnknownDevice2 << std::endl << std::endl;
            }
        }
    }

    return !targets_.empty();
}

bool KcCLICommanderLightning::Compile(const Config& config)
{
    bool ret = false;

    if (InitRequestedAsicListLC(config))
    {
        beKA::beStatus result = beKA::kBeStatusSuccess;

        // Prepare OpenCL options and defines.
        OpenCLOptions options;
        options.selected_devices = targets_;
        options.defines = config.defines;
        options.include_paths = config.include_path;
        options.opencl_compile_options = config.opencl_options;
        options.optimization_level = config.opt_level;
        options.line_numbers = config.is_line_numbers_required;
        options.should_dump_il = !config.il_file.empty();

        // Run the back-end compilation procedure.
        switch (config.mode)
        {
        case beKA::kModeOpenclOffline:
            result = CompileOpenCL(config, options);
            break;
        default:
            result = beKA::kBeStatusGeneralFailed;
            break;
        }

        // Generate CSV files with parsed ISA if required.
        if (config.is_parsed_isa_required && (result == beKA::kBeStatusSuccess || targets_.size() > 1))
        {
            result = ParseIsaFilesToCSV(config.is_line_numbers_required) ? beKA::kBeStatusSuccess : beKA::kBeStatusParseIsaToCsvFailed;
        }

        ret = (result == beKA::kBeStatusSuccess);
    }

    return ret;
}

void KcCLICommanderLightning::Version(Config& config, LoggingCallbackFunction callback)
{
    bool  ret;
    std::stringstream log;
    KcCliCommander::Version(config, callback);

    std::string output_text = "", version = "";
    beKA::beStatus status = BeProgramBuilderLightning::GetCompilerVersion(RgaMode::kModeOpenclOffline, config.compiler_bin_path,
                                                                           config.print_process_cmd_line, output_text);
    ret = (status == beKA::kBeStatusSuccess);
    if (ret)
    {
        size_t  offset = output_text.find(kCompilerVersionToken);
        if (offset != std::string::npos)
        {
            offset += kCompilerVersionToken.size();
            size_t  offset1 = output_text.find(" ", offset);
            size_t  offset2 = output_text.find("\n", offset);
            if (offset1 != std::string::npos && offset2 != std::string::npos)
            {
                size_t  end_offset = min(offset1, offset2);
                version = output_text.substr(0, end_offset);
            }
        }
    }

    if (ret)
    {
        const char* kStrOpenclOfflineCompilerVersionPrefix = "OpenCL Compiler: AMD Lightning Compiler - ";
        log << kStrOpenclOfflineCompilerVersionPrefix << version << std::endl;
    }

    LogCallback(log.str());
}

bool KcCLICommanderLightning::GenerateOpenclOfflineVersionInfo(const std::string& filename)
{
    std::set<std::string> targets;

    // Get the list of supported GPUs for current mode.
    bool result = GetSupportedTargets(targets);

    // Generate the Version Info header.
    result = result && KcXmlWriter::AddVersionInfoHeader(filename);

    // Add the list of supported GPUs to the Version Info file.
    result = result && KcXmlWriter::AddVersionInfoGPUList(RgaMode::kModeOpenclOffline, targets, filename);

    return result;
}

bool KcCLICommanderLightning::PrintAsicList(const Config&)
{
    std::set<std::string> targets;
    bool ret = GetSupportedTargets(targets);
    ret = ret && KcUtils::PrintAsicList(targets);

    if (ret)
    {
        // Print additional OpenCL Lightning Compiler target from the "additional-targets" file.
        std::vector<std::string> extra_targets = GetExtraTargetList();
        if (!extra_targets.empty())
        {
            static const char* kStrOpenclOfflineExtraDeviceListTitle = "Additional GPU targets (Warning: correct compilation and analysis are not guaranteed):";
            static const char* kStrOpenclOfflineDeviceListOffset = "    ";
            RgLog::stdOut << std::endl << kStrOpenclOfflineExtraDeviceListTitle << std::endl << std::endl;
            for (const std::string& device : extra_targets)
            {
                RgLog::stdOut << kStrOpenclOfflineDeviceListOffset << device << std::endl;
            }
            RgLog::stdOut << std::endl;
        }
    }
    return ret;
}

// Print warnings reported by compiler to stderr.
static bool DumpCompilerWarnings(const std::string& compiler_std_err)
{
    bool found_warnings = compiler_std_err.find(kCompilerWarningToken) != std::string::npos;
    if (found_warnings)
    {
        RgLog::stdErr << std::endl << compiler_std_err << std::endl;
    }
    return found_warnings;
}

beStatus KcCLICommanderLightning::CompileOpenCL(const Config& config, const OpenCLOptions& ocl_options)
{
    beStatus  status = kBeStatusSuccess;

    // Run LC compiler for all requested devices
    for (const std::string& device : ocl_options.selected_devices)
    {
        std::string  error_text;
        LogPreStep(kStrInfoCompiling, device);
        std::string  bin_filename;

        // Adjust the device name if necessary.
        std::string clang_device = device;
        if (kLcDeviceInfoToClangDeviceMap.count(clang_device) > 0)
        {
            clang_device = kLcDeviceInfoToClangDeviceMap.at(device);
        }

        // Update the binary and ISA names for current device.
        beStatus current_status = AdjustBinaryFileName(config, device, bin_filename);

        // If file with the same name exist, delete it.
        KcUtils::DeleteFile(bin_filename);

        // Prepare a list of source files.
        std::vector<std::string>  src_filenames;
        for (const std::string& input_file : config.input_files)
        {
            src_filenames.push_back(input_file);
        }

        if (current_status != kBeStatusSuccess)
        {
            LogErrorStatus(current_status, error_text);
            continue;
        }

        // Compile source to binary.
        current_status = BeProgramBuilderLightning::CompileOpenCLToBinary(compiler_paths_,
                                                                         ocl_options,
                                                                         src_filenames,
                                                                         bin_filename,
                                                                         clang_device,
                                                                         should_print_cmd_,
                                                                         error_text);
        LogResult(current_status == kBeStatusSuccess);

        if (current_status == kBeStatusSuccess)
        {
            // If "dump IL" option is passed to the Lightning Compiler, it should dump the IL to stderr.
            if (ocl_options.should_dump_il)
            {
                current_status = DumpIL(config, ocl_options, src_filenames, device, clang_device, error_text);
            }
            else if (config.is_warnings_required)
            {
                // Pass the warnings printed by the compiler to RGA stderr.
                DumpCompilerWarnings(error_text);
            }

            // Disassemble binary to ISA text.
            if (!config.isa_file.empty() || !config.analysis_file.empty() ||
                !config.livereg_analysis_file.empty() || !config.block_cfg_file.empty() || !config.inst_cfg_file.empty())
            {
                LogPreStep(kStrInfolExtractingIsaForDevice, device);
                current_status = DisassembleBinary(bin_filename, config.isa_file, clang_device, device, config.function, config.is_line_numbers_required, error_text);
                LogResult(current_status == kBeStatusSuccess);

                assert(current_status == kBeStatusSuccess);
                // Propagate the binary file name to the Output Files Metadata table.
                if (current_status == kBeStatusSuccess)
                {
                    for (auto& output_md_node : output_metadata_)
                    {
                        const std::string& md_device = output_md_node.first.first;
                        if (md_device == device)
                        {
                            output_md_node.second.bin_file = bin_filename;
                            output_md_node.second.is_bin_file_temp = config.binary_output_file.empty();
                        }
                    }
                }
            }
            else
            {
                output_metadata_[{device, ""}] = RgOutputFiles(RgEntryType::kOpenclKernel, "", bin_filename);
            }
        }
        else
        {
            // Store error status to the metadata.
            RgOutputFiles output(RgEntryType::kOpenclKernel, "", "");
            output.status = false;
            output_metadata_[{device, ""}] = output;
        }

        status = (current_status == kBeStatusSuccess ? status : current_status);
        LogErrorStatus(current_status, error_text);
    }

    return status;
}

beKA::beStatus KcCLICommanderLightning::DisassembleBinary(const std::string& binFileName,
                                                          const std::string& userIsaFileName,
                                                          const std::string& clangDevice,
                                                          const std::string& rgaDevice,
                                                          const std::string& kernel,
                                                          bool lineNumbers,
                                                          std::string& error_text)
{
    std::string  out_isa_text;
    std::vector<std::string>  kernel_names;
    beKA::beStatus status = BeProgramBuilderLightning::DisassembleBinary(compiler_paths_.bin, binFileName,
        clangDevice, lineNumbers, should_print_cmd_, out_isa_text, error_text);

    if (status == beKA::kBeStatusSuccess)
    {
        status = BeProgramBuilderLightning::ExtractKernelNames(compiler_paths_.bin, binFileName,
            should_print_cmd_, kernel_names);
    }
    else
    {
        // Store error status to the metadata.
        RgOutputFiles output(RgEntryType::kOpenclKernel, "", "");
        output.status = false;
        output_metadata_[{rgaDevice, ""}] = output;
    }

    if (status == beKA::kBeStatusSuccess)
    {
        if (!kernel.empty() && std::find(kernel_names.cbegin(), kernel_names.cend(), kernel) == kernel_names.cend())
        {
            error_text = std::string(kStrErrorOpenclOfflineCannotFindKernel) + kernel;
            status = beKA::kBeStatusWrongKernelName;
        }
        else
        {
            status = SplitISA(binFileName, out_isa_text, userIsaFileName, rgaDevice, kernel, kernel_names) ?
                         beKA::kBeStatusSuccess : beKA::kBeStatusLightningSplitIsaFailed;
        }
    }

    return status;
}

bool  KcCLICommanderLightning::ParseIsaFilesToCSV(bool line_numbers)
{
    bool  ret = true;
    for (const auto& output_md_item : output_metadata_)
    {
        if (output_md_item.second.status)
        {
            const RgOutputFiles& output_files = output_md_item.second;
            std::string  isa, parsed_isa, parsed_isa_filename;
            const std::string& device = output_md_item.first.first;
            const std::string& entry = output_md_item.first.second;

            bool  status = KcUtils::ReadTextFile(output_files.isa_file, isa, nullptr);
            if (status)
            {
                if ((status = GetParsedIsaCsvText(isa, device, line_numbers, parsed_isa)) == true)
                {
                    status = (KcUtils::GetParsedISAFileName(output_files.isa_file, parsed_isa_filename) == beKA::kBeStatusSuccess);
                    if (status)
                    {
                        status = (WriteIsaToFile(parsed_isa_filename, parsed_isa) == beKA::kBeStatusSuccess);
                    }
                    if (status)
                    {
                        output_metadata_[{device, entry}].isa_csv_file = parsed_isa_filename;
                    }
                }

                if (!status)
                {
                    RgLog::stdErr << kStrErrorFailedToConvertToCsvFormat << output_files.isa_file << std::endl;
                }
            }
            ret &= status;
        }
    }

    return ret;
}

void KcCLICommanderLightning::RunCompileCommands(const Config& config, LoggingCallbackFunction log_callback)
{
    bool is_multiple_devices = (config.asics.size() > 1);
    bool status = Compile(config);

    // Block post-processing until quality of analysis engine improves when processing llvm disassembly.
    bool is_livereg_required = !config.livereg_analysis_file.empty();
    if (is_livereg_required)
    {
        // Perform Live Registers analysis if required.
        if (status || is_multiple_devices)
        {
            PerformLiveRegAnalysis(config);
        }
    }

    bool is_cfg_required = (!config.block_cfg_file.empty() || !config.inst_cfg_file.empty());
    if (is_cfg_required)
    {
        // Extract Control Flow Graph.
        if (status || is_multiple_devices)
        {
            ExtractCFG(config);
        }
    }

    // Extract CodeObj metadata if required.
    if ((status || is_multiple_devices) && !config.metadata_file.empty())
    {
        ExtractMetadata(config.metadata_file);
    }

    // Extract Statistics if required.
    if ((status || is_multiple_devices) && !config.analysis_file.empty())
    {
        ExtractStatistics(config);
    }
}

beKA::beStatus KcCLICommanderLightning::AdjustBinaryFileName(const Config&      config,
                                                             const std::string& device,
                                                             std::string&       bin_filename)
{
    beKA::beStatus  status = beKA::kBeStatusSuccess;

    gtString name = L"";
    std::string user_bin_name = config.binary_output_file;

    // If binary output file name is not provided, create a binary file in the temp folder.
    if (user_bin_name == "")
    {
        user_bin_name = KcUtils::ConstructTempFileName(kTempBinaryFilename, kTempBinaryFileExtension).asASCIICharArray();
        if (user_bin_name == "")
        {
            status = beKA::kBeStatusGeneralFailed;
        }
    }

    if (status == beKA::kBeStatusSuccess)
    {
        name = L"";
        KcUtils::ConstructOutputFileName(user_bin_name, "", kStrDefaultExtensionBin, "", device, name);
        bin_filename = name.asASCIICharArray();
    }

    return status;
}

bool KcCLICommanderLightning::PerformLiveRegAnalysis(const Config& config)
{
    bool  ret = true;
    std::stringstream  error_msg;

    for (auto& output_md_item : output_metadata_)
    {
        RgOutputFiles& output_files = output_md_item.second;
        if (output_files.status)
        {
            const std::string& device      = output_md_item.first.first;
            const std::string& entry_name   = output_md_item.first.second;
            gtString  livereg_out_filename = L"";
            gtString  isa_filename;
            isa_filename << output_files.isa_file.c_str();
            gtString device_gtstr;
            device_gtstr << device.c_str();

            // Inform the user.
            std::cout << kStrInfoPerformingLiveregAnalysis1 << device << kStrInfoOpenclOfflineKernelForKernel << entry_name << "... ";

            // Construct a name for the output livereg file.
            KcUtils::ConstructOutputFileName(config.livereg_analysis_file, kStrDefaultExtensionLivereg,
                                             kStrDefaultExtensionText, entry_name, device, livereg_out_filename);
            if (!livereg_out_filename.isEmpty())
            {
                beWaveSize kernel_wave_size = (KcUtils::IsNaviTarget(device) ? beWaveSize::kWave64 : beWaveSize::kUnknown);

                // Perform live VGPR analysis and force wave 64 for OpenCL kernels. Currently the wave size information
                // is missing from LLVM disassembly, therefore Shae is not able to deduce the value from the disassembly.
                // Therefore we will use a default of wave64 (this would be ignored by Shae for pre-RDNA targets).
                KcUtils::PerformLiveRegisterAnalysis(isa_filename, device_gtstr, livereg_out_filename, log_callback_,
                    config.print_process_cmd_line, kernel_wave_size);
                if (BeProgramBuilderLightning::VerifyOutputFile(livereg_out_filename.asASCIICharArray()))
                {
                    // Store the name of livereg output file in the RGA output files metadata.
                    output_files.livereg_file = livereg_out_filename.asASCIICharArray();
                    std::cout << kStrInfoSuccess << std::endl;
                }
                else
                {
                    error_msg << kStrErrorCannotPerformLiveregAnalysis << " " << kStrKernelName << entry_name << std::endl;
                    std::cout << kStrInfoFailed << std::endl;
                    ret = false;
                }
            }
            else
            {
                error_msg << kStrErrorOpenclOfflineFailedToCreateOutputFilenameForKernel << entry_name << std::endl;
                ret = false;
            }
        }
    }

    if (!ret)
    {
        log_callback_(error_msg.str());
    }

    return ret;
}

bool KcCLICommanderLightning::ExtractCFG(const Config& config)
{
    bool  ret = true;
    std::stringstream  error_msg;

    for (auto& output_md_item : output_metadata_)
    {
        RgOutputFiles& outputFiles = output_md_item.second;

        if (outputFiles.status)
        {
            const std::string& device = output_md_item.first.first;
            const std::string& entry_name = output_md_item.first.second;
            gtString  cfg_out_filename = L"";
            gtString  isa_filename;
            isa_filename << outputFiles.isa_file.c_str();
            gtString device_gtstr;
            device_gtstr << device.c_str();

            bool is_per_basic_block_cfg = !config.block_cfg_file.empty();
            std::cout << (is_per_basic_block_cfg ? kStrInfoContructingPerBlockCfg1 : kStrInfoContructingPerInstructionCfg1) << device
                      << kStrInfoOpenclOfflineKernelForKernel << entry_name << "... ";

            // Construct a name for the output CFG file.
            std::string base_file = (is_per_basic_block_cfg ? config.block_cfg_file : config.inst_cfg_file);
            KcUtils::ConstructOutputFileName(base_file, KC_STR_DEFAULT_CFG_SUFFIX, kStrDefaultExtensionDot,
                                             entry_name, device, cfg_out_filename);
            if (!cfg_out_filename.isEmpty())
            {
                KcUtils::GenerateControlFlowGraph(isa_filename, device_gtstr, cfg_out_filename, log_callback_,
                                                  !config.inst_cfg_file.empty(), config.print_process_cmd_line);

                if (!BeProgramBuilderLightning::VerifyOutputFile(cfg_out_filename.asASCIICharArray()))
                {
                    error_msg << kStrErrorCannotGenerateCfg << " " << kStrKernelName << entry_name << std::endl;
                    std::cout << kStrInfoFailed << std::endl;
                    ret = false;
                }
                else
                {
                    std::cout << kStrInfoSuccess << std::endl;
                }
            }
            else
            {
                error_msg << kStrErrorOpenclOfflineFailedToCreateOutputFilenameForKernel << entry_name << std::endl;
                ret = false;
            }
        }
    }

    if (!ret)
    {
        log_callback_(error_msg.str());
    }

    return ret;
}

beKA::beStatus KcCLICommanderLightning::ExtractMetadata(const std::string& metadata_filename) const
{
    beKA::beStatus current_status;
    beKA::beStatus status = kBeStatusSuccess;
    std::string metadata_text;
    gtString out_filename;

    // A set of already processed devices.
    std::set<std::string> devices;

    // outputMDNode is: pair{pair{device, kernel}, rgOutputFiles}.
    for (auto& output_md_node : output_metadata_)
    {
        if (output_md_node.second.status)
        {
            const std::string& device = output_md_node.first.first;
            if (devices.count(device) == 0)
            {
                devices.insert(device);
                const std::string  bin_filename = output_md_node.second.bin_file;
                static const char* kStrDefaultExtensionMd = "md";
                KcUtils::ConstructOutputFileName(metadata_filename, "", kStrDefaultExtensionMd,
                    kStrDefaultExtensionText, device, out_filename);
                if (!out_filename.isEmpty())
                {
                    current_status = BeProgramBuilderLightning::ExtractMetadata(compiler_paths_.bin, bin_filename, should_print_cmd_, metadata_text);
                    if (current_status == beKA::kBeStatusSuccess && !metadata_text.empty())
                    {
                        current_status = KcUtils::WriteTextFile(out_filename.asASCIICharArray(), metadata_text, log_callback_) ?
                                                               beKA::kBeStatusSuccess : beKA::kBeStatusWriteToFileFailed;
                    }
                }
                else
                {
                    current_status = beKA::kBeStatusLightningExtractMetadataFailed;
                }
            }
            status = (current_status == beKA::kBeStatusSuccess ? status : beKA::kBeStatusLightningExtractMetadataFailed);
        }
    }

    if (status != beKA::kBeStatusSuccess)
    {
        std::stringstream  msg;
        msg << kStrErrorOpenclOfflineFailedToExtractMetadata << std::endl;
        log_callback_(msg.str());
    }

    return status;
}

// Get the ISA size and store it to "kernelCodeProps" structure.
static beStatus  GetIsaSize(const std::string& isaFileName, KernelCodeProperties& kernelCodeProps)
{
    beStatus  status = kBeStatusLightningGetISASizeFailed;
    if (!isaFileName.empty())
    {
        std::string  isa_text;
        if (KcUtils::ReadTextFile(isaFileName, isa_text, nullptr))
        {
            int  isa_size = BeProgramBuilderLightning::GetIsaSize(isa_text);
            if (isa_size != -1)
            {
                kernelCodeProps.isa_size = isa_size;
                status = kBeStatusSuccess;
            }
        }
    }

    return status;
}

// Build the statistics in "AnalysisData" form.
static bool  BuildAnalysisData(const KernelCodeProperties& kernel_code_props, const std::string& device, AnalysisData& stats)
{
    uint64_t  min_sgprs = 0, min_vgprs = 0;

    // Set unknown values to 0.
    memset(&stats, 0, sizeof(AnalysisData));
    if (kRgaDeviceProps.count(device))
    {
        const DeviceProps& deviceProps = kRgaDeviceProps.at(device);
        stats.lds_size_available     = deviceProps.available_lds_bytes;
        stats.num_sgprs_available    = deviceProps.available_sgprs;
        stats.num_vgprs_available    = deviceProps.available_vgprs;
        min_sgprs = deviceProps.min_sgprs;
        min_vgprs = deviceProps.min_vgprs;
    }
    else
    {
        stats.lds_size_available     = -1;
        stats.num_sgprs_available    = -1;
        stats.num_vgprs_available    = -1;
    }
    stats.num_threads_per_group_total    = -1;
    stats.num_threads_per_group_x   = -1;
    stats.num_threads_per_group_y   = -1;
    stats.num_threads_per_group_z   = -1;

    stats.scratch_memory_used = kernel_code_props.private_segment_size;
    stats.lds_size_used       = kernel_code_props.workgroup_segment_size;
    stats.num_sgprs_used      = max(min_sgprs, kernel_code_props.wavefront_num_sgprs);
    stats.num_vgprs_used      = max(min_vgprs, kernel_code_props.work_item_num_vgprs);
    stats.num_sgpr_spills     = kernel_code_props.sgpr_spills;
    stats.num_vgpr_spills     = kernel_code_props.vgpr_spills;
    stats.wavefront_size      = kernel_code_props.wavefront_size;
    stats.isa_size            = kernel_code_props.isa_size;

    assert(stats.wavefront_size == 32 || stats.wavefront_size == 64);

    return true;
}

// Construct statistics text and store it to CSV file.
static bool  StoreStatistics(const Config& config, const std::string& base_stats_filename, const std::string& device,
                             const std::string& kernel, const AnalysisData& stats, std::string& out_filename)
{
    bool  ret = false;
    gtString  stats_filename;
    KcUtils::ConstructOutputFileName(base_stats_filename, kStrDefaultExtensionStats,
                                     kStrDefaultExtensionCsv, kernel, device, stats_filename);

    if (!stats_filename.isEmpty())
    {
        out_filename = stats_filename.asASCIICharArray();
        std::stringstream  stats_text;
        char separator = KcUtils::GetCsvSeparator(config);
        // Lambda returning "N/A" string if value = -1 or string representation of value itself otherwise.
        auto na_or = [](uint64_t val) { return (val == (int64_t)-1 ? kStrDx11NaValue : std::to_string(val)); };

        stats_text << KcUtils::GetStatisticsCsvHeaderString(separator) << std::endl;
        stats_text << device << separator;
        stats_text << na_or(stats.scratch_memory_used) << separator;
        stats_text << na_or(stats.num_threads_per_group_total) << separator;
        stats_text << na_or(stats.wavefront_size) << separator;
        stats_text << na_or(stats.lds_size_available) << separator;
        stats_text << na_or(stats.lds_size_used) << separator;
        stats_text << na_or(stats.num_sgprs_available) << separator;
        stats_text << na_or(stats.num_sgprs_used) << separator;
        stats_text << na_or(stats.num_sgpr_spills) << separator;
        stats_text << na_or(stats.num_vgprs_available) << separator;
        stats_text << na_or(stats.num_vgprs_used) << separator;
        stats_text << na_or(stats.num_vgpr_spills) << separator;
        stats_text << na_or(stats.num_threads_per_group_x) << separator;
        stats_text << na_or(stats.num_threads_per_group_y) << separator;
        stats_text << na_or(stats.num_threads_per_group_z) << separator;
        stats_text << na_or(stats.isa_size);
        stats_text << std::endl;

        ret = KcUtils::WriteTextFile(stats_filename.asASCIICharArray(), stats_text.str(), nullptr);
    }

    return ret;
}

beStatus KcCLICommanderLightning::ExtractStatistics(const Config& config)
{
    std::string device = "", statFileName = config.analysis_file, outStatFileName;
    beStatus status = kBeStatusSuccess;

    LogPreStep(kStrInfoExtractingStats);

    for (auto& outputMDItem : output_metadata_)
    {
        AnalysisData stats_data;
        CodePropsMap code_props;
        const std::string& current_device = outputMDItem.first.first;
        if (device != current_device && outputMDItem.second.status)
        {
            status = BeProgramBuilderLightning::ExtractKernelCodeProps(config.compiler_bin_path, outputMDItem.second.bin_file,
                                                                       config.print_process_cmd_line, code_props);
            if (status != kBeStatusSuccess)
            {
                break;
            }
            for (auto& kernel_code_props : code_props)
            {
                if (config.function.empty() || config.function == kernel_code_props.first)
                {
                    std::string current_isa_file;
                    KcUtils::ConstructOutputFileName(config.isa_file, "", kStrDefaultExtensionIsa, kernel_code_props.first, current_device, current_isa_file);
                    if (GetIsaSize(current_isa_file, kernel_code_props.second) &&
                        BuildAnalysisData(kernel_code_props.second, current_device, stats_data))
                    {
                        status = StoreStatistics(config, statFileName, current_device, kernel_code_props.first, stats_data, outStatFileName) ?
                                     status : kBeStatusWriteToFileFailed;
                        if (status == kBeStatusSuccess)
                        {
                            auto out_files = output_metadata_.find({current_device, kernel_code_props.first});
                            if (out_files != output_metadata_.end())
                            {
                                out_files->second.stats_file = outStatFileName;
                            }
                        }
                    }
                }
            }

            device = outputMDItem.first.first;
        }
    }

    LogResult(status == kBeStatusSuccess);

    return status;
}

static void  GatherBranchTargets(std::stringstream& isa, std::unordered_map<std::string, bool>& branch_targets)
{
    // The format of branch instruction text:
    //
    //     s_cbranch_scc1 BB0_3        // 000000001110: BF85001C
    //           ^         ^                    ^          ^
    //           |         |                    |          |
    //      instruction  label               offset       code

    std::string  isa_line;

    // Skip lines before the actual ISA code.
    while (std::getline(isa, isa_line) && isa_line.find(kLcKernelIsaHeader3) == std::string::npos) {}

    // Gather target labels of all branch instructions.
    while (std::getline(isa, isa_line))
    {
        size_t  inst_end_offset, branch_token_offset, instOffset = isa_line.find_first_not_of(" \t");
        if (instOffset != std::string::npos)
        {
            if ((branch_token_offset = isa_line.find(kLcIsaBranchToken, instOffset)) != std::string::npos ||
                (branch_token_offset = isa_line.find(kIsaCallToken, instOffset)) != std::string::npos)
            {
                if ((inst_end_offset = isa_line.find_first_of(" \t", instOffset)) != std::string::npos &&
                    branch_token_offset < inst_end_offset)
                {
                    // Found branch instruction. Add its target label to the list.
                    size_t  label_start_offset, label_end_offset;
                    if ((label_start_offset = isa_line.find_first_not_of(" \t", inst_end_offset)) != std::string::npos &&
                        isa_line.compare(label_start_offset, kIsaInstructionAddressStartToken.size(), kIsaInstructionAddressStartToken) != 0 &&
                        ((label_end_offset = isa_line.find_first_of(" \t", label_start_offset)) != std::string::npos))
                    {
                        branch_targets[isa_line.substr(label_start_offset, label_end_offset - label_start_offset)] = true;
                    }
                }
            }
        }
    }
    isa.clear();
    isa.seekg(0);
}

// Checks if "isa_line" is a label that is not in the list of branch targets.
bool  IsUnreferencedLabel(const std::string& isa_line, const std::unordered_map<std::string, bool>& branch_targets)
{
    bool  ret = false;

    // Looking for strings of the pattern 'anylabel:' that are not function labels.
    size_t colon_indx = isa_line.find(':');
    size_t line_size = isa_line.size();
    if ((colon_indx == (line_size - 1)) && (line_size > 1))
    {
        std::string branch_name = isa_line.substr(0, isa_line.size() - 1);
        ret = (branch_targets.find(branch_name) == branch_targets.end());
    }

    return ret;
}

// Remove non-standard instruction suffixes.
static void  FilterISALine(std::string & isa_line)
{
    size_t  offset = isa_line.find_first_not_of(" \t");
    if (offset != std::string::npos)
    {
        offset = isa_line.find_first_of(" ");
    }
    if (offset != std::string::npos)
    {
        size_t  suffix_length = 0;
        if (offset >= kLcIsaInstructionSuffix1.size() &&
            isa_line.substr(offset - kLcIsaInstructionSuffix1.size(), kLcIsaInstructionSuffix1.size()) == kLcIsaInstructionSuffix1)
        {
            suffix_length = kLcIsaInstructionSuffix1.size();
        }
        else if (offset >= kLcIsaInstructionSuffix2.size() &&
            isa_line.substr(offset - kLcIsaInstructionSuffix2.size(), kLcIsaInstructionSuffix2.size()) == kLcIsaInstructionSuffix2)
        {
            suffix_length = kLcIsaInstructionSuffix2.size();
        }
        // Remove the suffix.
        if (suffix_length != 0)
        {
            isa_line.erase(offset - suffix_length, suffix_length);
            // Restore the alignment of byte encoding.
            if ((offset = isa_line.find("//", offset)) != std::string::npos)
            {
                isa_line.insert(offset, suffix_length, ' ');
            }
        }
    }
}

// The Lightning Compiler may append useless code for some library functions to the ISA disassembly.
// This function eliminates such code.
// It also also removes unreferenced labels and non-standard instruction suffixes.
bool  KcCLICommanderLightning::ReduceISA(const std::string& binFile, IsaMap& kernel_isa_text)
{
    bool  ret = false;
    for (auto& kernel_isa : kernel_isa_text)
    {
        int  code_size = BeProgramBuilderLightning::GetKernelCodeSize(compiler_paths_.bin, binFile, kernel_isa.first, should_print_cmd_);
        assert(code_size != -1);
        if (code_size != -1)
        {
            // Copy ISA lines to new stream. Stop when found an instruction with address > codeSize.
            std::stringstream  old_isa, new_isa, address_stream;
            old_isa.str(kernel_isa.second);
            std::string  isa_line;
            int  address, address_offset = -1;

            // Gather the target labels of all branch instructions.
            std::unordered_map<std::string, bool>  branch_targets;
            branch_targets.clear();
            GatherBranchTargets(old_isa, branch_targets);

            // Skip lines before the actual ISA code.
            while (std::getline(old_isa, isa_line) && new_isa << isa_line << std::endl &&
                isa_line.find(kLcKernelIsaHeader3) == std::string::npos) {}

            while (std::getline(old_isa, isa_line))
            {
                // Add the ISA line to the new ISA text if it's not an unreferenced label.
                if (!IsUnreferencedLabel(isa_line, branch_targets))
                {
                    if (isa_line.find(" <") != 0)
                    {
                        size_t branch_label_start = isa_line.find(" <") + 2;
                        size_t branch_label_end = isa_line.find(">:");
                        size_t address_end = isa_line.find_first_of(" ");
                        if (branch_label_end != std::string::npos)
                        {
                            // If this is a branch label, reformat and add the string so the RGA GUI recognizes the syntax.
                            std::string new_branch_label = isa_line.substr(0, address_end + 1) + isa_line.substr(branch_label_start, branch_label_end - (branch_label_start)) + ":";
                            new_isa << new_branch_label << std::endl;
                        }
                        else
                        {
                            // Add the line as is.
                            new_isa << isa_line << std::endl;
                        }
                    }
                }

                // Check if this instruction is the last one and we have to stop here.
                // Skip comment lines generated by disassembler.
                if (isa_line.find(kIsaCommentStartToken, 0) != 0)
                {

                    // Format of ISA disassembly instruction (64-bit and 32-bit):
                    //  s_load_dwordx2 s[0:1], s[6:7], 0x0     // 000000001108: C0060003 00000000
                    //  v_add_u32 v0, s8, v0                   // 000000001134: 68000008
                    //                                            `-- addr --'
                    size_t  address_start, address_end;

                    FilterISALine(isa_line);

                    if ((address_start = isa_line.find(kIsaInstructionAddressStartToken)) != std::string::npos &&
                        (address_end = isa_line.find(kIsaInstructionAddressEndToken, address_start)) != std::string::npos)
                    {
                        address_start += (kIsaInstructionAddressStartToken.size());
                        address_stream.str(isa_line.substr(address_start, address_end - address_start));
                        address_stream.clear();
                        int inst_size = (isa_line.size() - address_end < kIsaInstruction64BitCodeTextSize) ? kIsaInstruction32BitBytes : kIsaInstruction64BitBytes;
                        if (address_stream >> std::hex >> address)
                        {
                            // address_offset is the binary address of 1st instruction.
                            address_offset = (address_offset == -1 ? address : address_offset);
                            if ((address - address_offset + inst_size) >= code_size)
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
                kernel_isa.second = new_isa.str();
            }
        }
    }

    return ret;
}

bool KcCLICommanderLightning::SplitISA(const std::string& bin_file, const std::string& isa_text,
                                       const std::string& user_isa_file_name, const std::string& device,
                                       const std::string& kernel, const std::vector<std::string>& kernel_names)
{
    // kernelIsaTextMap maps kernel name --> kernel ISA text.
    IsaMap kernel_isa_text_map;
    bool ret, is_isa_file_temp = user_isa_file_name.empty();

    // Replace labels of format "address   <label_name>:" with
    //  "label_name:"
    std::string label_pattern_prefix("[0-9a-zA-Z]+ <");
    std::regex label_prefix_regex(label_pattern_prefix);
    std::regex label_suffix_regex(">:");
    std::string isa_text_clean_prefix = std::regex_replace(isa_text, label_prefix_regex, "");
    std::string new_isa_text = std::regex_replace(isa_text_clean_prefix, label_suffix_regex, ":");

    // Split ISA text into per-kernel fragments.
    ret = SplitISAText(new_isa_text, kernel_names, kernel_isa_text_map);

    // Eliminate the useless code.
    ret = ret && ReduceISA(bin_file, kernel_isa_text_map);

    // Store per-kernel ISA texts to separate files and launch livereg tool for each file.
    if (ret)
    {
        // isaTextMapItem is a pair{kernelName, kernelIsaText}.
        for (const auto &isa_text_map_item : kernel_isa_text_map)
        {
            // Skip the kernels that are not requested.
            if (!kernel.empty() && kernel != isa_text_map_item.first)
            {
                continue;
            }

            gtString  isa_filename;
            std::string function_name = isa_text_map_item.first;

            if (is_isa_file_temp)
            {
                gtString  base_isa_filename(kTempIsaFilename);
                base_isa_filename << device.c_str() << "_" << function_name.c_str();
                isa_filename = KcUtils::ConstructTempFileName(base_isa_filename, kTempIsaFileExtension);
            }
            else
            {
                KcUtils::ConstructOutputFileName(user_isa_file_name, "", kStrDefaultExtensionIsa,
                                                 function_name, device, isa_filename);
            }
            if (!isa_filename.isEmpty())
            {
                if (KcUtils::WriteTextFile(isa_filename.asASCIICharArray(), isa_text_map_item.second, log_callback_))
                {
                    RgOutputFiles  outFiles = RgOutputFiles(RgEntryType::kOpenclKernel, isa_filename.asASCIICharArray());
                    outFiles.is_isa_file_temp = is_isa_file_temp;
                    output_metadata_[{device, isa_text_map_item.first}] = outFiles;
                }
            }
            else
            {
                std::stringstream  error_msg;
                error_msg << kStrErrorOpenclOfflineFailedToCreateTempFile << std::endl;
                log_callback_(error_msg.str());
                ret = false;
            }
        }
    }

    return ret;
}

bool KcCLICommanderLightning::SplitISAText(const std::string& isa_text,
                                           const std::vector<std::string>& kernel_names,
                                           IsaMap& kernel_isa_map) const
{
    bool  status = true;
    const std::string  LABEL_NAME_END_TOKEN = ":\n";
    const std::string  BLOCK_END_TOKEN = "\n\n";
    size_t label_name_start = 0, label_name_end = 0, kernel_isa_end = 0, block_end = 0;

    label_name_start = isa_text.find_first_not_of('\n');

    std::vector<std::pair<size_t, size_t>> kernel_start_offsets;
    if (!isa_text.empty())
    {
        while ((label_name_end = isa_text.find(LABEL_NAME_END_TOKEN, label_name_start)) != std::string::npos)
        {
            // Check if this contains a kernel name.
            std::string  label_name = isa_text.substr(label_name_start, label_name_end - label_name_start);
            if (std::count(kernel_names.begin(), kernel_names.end(), label_name) != 0)
            {
                kernel_start_offsets.push_back({ label_name_start, label_name_end - label_name_start });
            }
            if ((label_name_start = isa_text.find(BLOCK_END_TOKEN, label_name_end)) == std::string::npos)
            {
                // End of file.
                break;
            }
            else
            {
                label_name_start += BLOCK_END_TOKEN.size();
            }
        }
    }

    // Split the ISA text using collected offsets of kernel names.
    for (size_t i = 0, size = kernel_start_offsets.size(); i < size; i++)
    {
        size_t  isa_text_start = kernel_start_offsets[i].first;
        size_t  isa_text_end = (i < size - 1 ? kernel_start_offsets[i + 1].first - 1 : isa_text.size());
        if (isa_text_start <= isa_text_end)
        {
            std::string  kernel_isa = isa_text.substr(isa_text_start, isa_text_end - isa_text_start);
            std::stringstream  kernel_isa_text;
            const std::string& kernel_name = isa_text.substr(kernel_start_offsets[i].first, kernel_start_offsets[i].second);
            kernel_isa_text << kLcKernelIsaHeader1 << "\"" << kernel_name << "\"" << std::endl << std::endl
                << kLcKernelIsaHeader2 << "\"" << kernel_name << "\":" << std::endl << std::endl << kLcKernelIsaHeader3;
            kernel_isa_text << kernel_isa;
            kernel_isa_map[kernel_name] = kernel_isa_text.str();
            label_name_start = kernel_isa_end + BLOCK_END_TOKEN.size();
        }
        else
        {
            status = false;
            break;
        }
    }

    return status;
}

bool KcCLICommanderLightning::ListEntries(const Config& config, LoggingCallbackFunction callback)
{
    return ListEntriesOpenclOffline(config, callback);
}

bool KcCLICommanderLightning::ListEntriesOpenclOffline(const Config& config, LoggingCallbackFunction callback)
{
    bool ret = true;
    std::string filename;
    RgEntryData entry_data;
    std::stringstream msg;

    if (config.mode != RgaMode::kModeOpenclOffline)
    {
        msg << kStrErrorCommandNotSupported << std::endl;
        ret = false;
    }
    else
    {
        if (config.input_files.size() == 1)
        {
            filename = config.input_files[0];
        }
        else if (config.input_files.size() > 1)
        {
            msg << kStrErrorSingleInputFileExpected << std::endl;
            ret = false;
        }
        else
        {
            msg << kStrErrorNoInputFile << std::endl;
            ret = false;
        }
    }

    if (ret && (ret = KcCLICommanderLightning::ExtractEntries(filename, config, entry_data)) == true)
    {
        // Sort the entry names in alphabetical order.
        std::sort(entry_data.begin(), entry_data.end(),
            [](const std::tuple<std::string, int, int>& a, const std::tuple<std::string, int, int>& b) {return (std::get<0>(a) < std::get<0>(b)); });

        // Dump the entry points.
        for (const auto& data_item : entry_data)
        {
            msg << std::get<0>(data_item) << ": " << std::get<1>(data_item) << "-" << std::get<2>(data_item) << std::endl;
        }
        msg << std::endl;
    }

    callback(msg.str());

    return ret;
}

bool KcCLICommanderLightning::ExtractEntries(const std::string& file_name, const Config& config, RgEntryData& entry_data)
{
    bool  ret = false;

    // Gather the options
    std::string  options = GatherOCLOptions(config);

    // Call OpenCL compiler preprocessor.
    std::string  prep_src;
    beStatus  status = BeProgramBuilderLightning::PreprocessOpencl(config.compiler_bin_path, file_name, options, config.print_process_cmd_line, prep_src);
    if (status == kBeStatusSuccess)
    {
        // Parse preprocessed source text and extract the kernel names.
        ret = ExtractEntriesPreprocessed(prep_src, file_name, entry_data);
    }
    else
    {
        // In case of error, prepSrc contains the error message printed by LC Preprocessor.
        LogErrorStatus(status, prep_src);
    }

    return ret;
}

bool KcCLICommanderLightning::GetSupportedTargets(std::set<std::string>& targets, bool)
{
    // Gather the supported devices in DeviceInfo format.
    targets.clear();

    for (const auto& d : kLcLlvmTargetsToDeviceInfoTargets)
    {
        targets.insert(d.second);
    }

    return !targets.empty();
}

bool KcCLICommanderLightning::GenerateSessionMetadata(const Config& config) const
{
    RgFileEntryData  file_kernel_data;
    bool ret = !config.session_metadata_file.empty();
    assert(ret);

    if (ret)
    {
        for (const std::string& input_file : config.input_files)
        {
            RgEntryData  entry_data;
            ret = ret && ExtractEntries(input_file, config, entry_data);
            if (ret)
            {
                file_kernel_data[input_file] = entry_data;
            }
        }
    }

    if (ret && !output_metadata_.empty())
    {
        ret = KcXmlWriter::GenerateClSessionMetadataFile(config.session_metadata_file, file_kernel_data, output_metadata_);
    }

    return ret;
}

bool KcCLICommanderLightning::RunPostCompileSteps(const Config& config)
{
    bool ret = false;
    if (!config.session_metadata_file.empty())
    {
        ret = GenerateSessionMetadata(config);
        if (!ret)
        {
            std::stringstream  msg;
            msg << kStrErrorFailedToGenerateSessionMetdata << std::endl;
            log_callback_(msg.str());
        }
    }

    DeleteTempFiles();

    return ret;
}

void KcCLICommanderLightning::DeleteTempFiles() const
{
    for (const auto& out_file_data : output_metadata_)
    {
        const RgOutputFiles out_files = out_file_data.second;
        gtString filename;
        if (out_files.is_bin_file_temp && KcUtils::FileNotEmpty(out_files.bin_file))
        {
            filename.fromASCIIString(out_files.bin_file.c_str());
            KcUtils::DeleteFile(filename);
        }
        if (out_files.is_isa_file_temp && KcUtils::FileNotEmpty(out_files.isa_file))
        {
            filename.fromASCIIString(out_files.isa_file.c_str());
            KcUtils::DeleteFile(filename);
        }
    }
}

beStatus KcCLICommanderLightning::DumpIL(const Config&                   config,
                                         const OpenCLOptions&            user_options,
                                         const std::vector<std::string>& src_file_names,
                                         const std::string&              device,
                                         const std::string&              clang_device,
                                         std::string&                    error_text)
{
    beStatus status = kBeStatusSuccess;

    // Generate new options instructing to generate LLVM IR disassembly.
    OpenCLOptions ocl_options_llvm_ir                       = user_options;
    ocl_options_llvm_ir.should_generate_llvm_ir_disassembly = true;

    for (const auto& src_file_name_with_ext : src_file_names)
    {
        // Convert the src kernel input file name to gtString.
        gtString src_file_name_with_ext_as_gtstr;
        src_file_name_with_ext_as_gtstr << src_file_name_with_ext.c_str();
        osFilePath src_file_path(src_file_name_with_ext_as_gtstr);

        // Extract the kernel's file name without directory and extension.
        gtString src_file_name;
        assert(!src_file_path.isDirectory());
        src_file_path.getFileName(src_file_name);

        gtString il_filename;
        KcUtils::ConstructOutputFileName(config.il_file, "", kStrDefaultExtensionLlvmir, src_file_name.asASCIICharArray(), device, il_filename);

        // Invoking clang with "-emit-llvm -S" for multiple .cl files is not supported.
        // Clang should be invoked with one input file at a time.
        status = BeProgramBuilderLightning::CompileOpenCLToLlvmIr(compiler_paths_,
                                                                  ocl_options_llvm_ir,
                                                                  std::vector<std::string>{src_file_name_with_ext},
                                                                  il_filename.asASCIICharArray(),
                                                                  clang_device,
                                                                  should_print_cmd_,
                                                                  error_text);
        if (status != kBeStatusSuccess)
        {
            std::stringstream msg;
            msg << kStrErrorOpenclOfflineLlvmIrDisassemblyFailure << std::endl;
            error_text.append(msg.str());
        }
    }
    return status;
}
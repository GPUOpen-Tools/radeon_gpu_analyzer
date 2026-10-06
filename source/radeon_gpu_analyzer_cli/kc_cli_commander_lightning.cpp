//=============================================================================
/// Copyright (c) 2020-2026 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for CLI Commander interface for compiling with the Lightning Compiler (LC).
//=============================================================================
// C++.
#include <vector>
#include <map>
#include <utility>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <cassert>

// Infra.
#include "external/amdt_os_wrappers/Include/osFilePath.h"
#include "external/amdt_os_wrappers/Include/osDirectory.h"
#include "external/amdt_os_wrappers/Include/osApplication.h"

// Shared.
#include "common/rga_analysis_summary.h"
#include "common/rga_cli_defs.h"
#include "common/rga_entry_type.h"
#include "common/rg_log.h"
#include "common/rga_shared_utils.h"

// Backend.
#include "radeon_gpu_analyzer_backend/be_isa_spec_metadata.h"
#include "radeon_gpu_analyzer_backend/be_metadata_parser.h"
#include "radeon_gpu_analyzer_backend/be_program_builder_lightning.h"
#include "radeon_gpu_analyzer_backend/be_string_constants.h"
#include "radeon_gpu_analyzer_backend/be_utils.h"

// Local.
#include "radeon_gpu_analyzer_cli/kc_cli_commander_lightning.h"
#include "radeon_gpu_analyzer_cli/kc_cli_isa_spec_loader.h"
#include "radeon_gpu_analyzer_cli/kc_cli_string_constants.h"
#include "radeon_gpu_analyzer_cli/kc_utils.h"
#include "radeon_gpu_analyzer_cli/kc_utils_lightning.h"
#include "radeon_gpu_analyzer_cli/kc_xml_writer.h"

// *****************************************
// *** INTERNALLY LINKED SYMBOLS - START ***
// *****************************************

// Targets of Lightning Compiler in LLVM format and corresponding DeviceInfo names.
static const std::vector<std::pair<std::string, std::string>> kLcLlvmTargetsToDeviceInfoTargets = {
    {"gfx801", "carrizo"},
    {"gfx802", "tonga"},
    {"gfx803", "fiji"},
    {"gfx803", "ellesmere"},
    {"gfx803", "baffin"},
    {"gfx803", "gfx804"},
    {"gfx900", "gfx900"},
    {"gfx902", "gfx902"},
    {"gfx904", "gfx904"},
    {"gfx906", "gfx906"},
    {"gfx908", "gfx908"},
    {"gfx90a", "gfx90a"},
    {"gfx90c", "gfx90c"},
    {"gfx942", "gfx942"},
    {"gfx950", "gfx950"},
    {"gfx1010", "gfx1010"},
    {"gfx1011", "gfx1011"},
    {"gfx1012", "gfx1012"},
    {"gfx1030", "gfx1030"},
    {"gfx1031", "gfx1031"},
    {"gfx1032", "gfx1032"},
    {"gfx1034", "gfx1034"},
    {"gfx1035", "gfx1035"},
    {"gfx1100", "gfx1100"},
    {"gfx1101", "gfx1101"},
    {"gfx1102", "gfx1102"},
    {"gfx1103", "gfx1103"},
    {"gfx1150", "gfx1150"},
    {"gfx1151", "gfx1151"},
    {"gfx1152", "gfx1152"},
    {"gfx1153", "gfx1153"},
    {"gfx1200", "gfx1200"},
    {"gfx1201", "gfx1201"},
    {"gfx1250", "gfx1250"},
};

// For some devices, clang does not accept device names that RGA gets from DeviceInfo.
// This table maps the DeviceInfo names to names accepted by clang for such devices.
static const std::map<std::string, std::string> kLcDeviceInfoToClangDeviceMap = {{"ellesmere", "polaris10"}, {"baffin", "polaris11"}, {"gfx804", "gfx803"}};

// Default target for Lightning Compiler (the latest supported target).
static const std::string kLcDefaultTarget = kLcLlvmTargetsToDeviceInfoTargets.rbegin()->second;

static const gtString kTempBinaryFilename      = L"rga_lc_ocl_out";
static const gtString kTempBinaryFileExtension = L"bin";
static const gtString kTempIsaFilename         = L"rga_lc_isa_";
static const gtString kTempIsaFileExtension    = L"isa";

static const std::string kCompilerVersionToken = "clang version ";
static const std::string kCompilerWarningToken = "warning:";

static const std::string kLcIsaInstructionSuffix1 = "_e32";
static const std::string kLcIsaInstructionSuffix2 = "_e64";

static const std::string kLcIsaBranchToken = "branch";
static const std::string kIsaCallToken     = "call";

static const std::string kIsaInstructionAddressStartToken = "//";
static const std::string kIsaInstructionAddressEndToken   = ":";
static const std::string kIsaCommentStartToken            = ";";

static const std::string kStrDx11NaValue = "N/A";

// Error messages.
static const char* kStrErrorOpenclOfflineCannotFindKernel         = "Error: cannot find OpenCL kernel: ";
static const char* kStrErrorOpenclOfflineUnknownDevice1           = "Error: unknown device name provided: ";
static const char* kStrErrorOpenclOfflineUnknownDevice2           = ". Cannot compile for this target.";
static const char* kStrErrorOpenclOfflineFailedToCreateTempFile   = "Error: failed to create a temp file.";
static const char* kStrErrorOpenclOfflineLlvmIrDisassemblyFailure = "Error: failed to generate LLVM IR disassembly.";

// Warning messages.
static const char* kStrWarningOpenclOfflineUsingExtraDevice1 = "Warning: using unknown target GPU: ";
static const char* kStrWarningRocmclUsingExtraDevice2        = "; successful compilation and analysis are not guaranteed.";

// Info messages.
static const char* kStrInfoOpenclOfflinePerformingLiveregAnalysis = "Performing live register analysis";
static const char* kStrInfoOpenclOfflineExtractingCfg             = "Extracting control flow graph";

static const size_t kIsaInstruction64BitCodeTextSize = 16;
static const int    kIsaInstruction64BitBytes        = 8;
static const int    kIsaInstruction32BitBytes        = 4;

// ***************************************
// *** INTERNALLY LINKED SYMBOLS - END ***
// ***************************************

// Returns the list of additional LC targets specified in the "additional-targets" file.
static std::vector<std::string> GetExtraTargetList()
{
    static const wchar_t*    kLcExtraTargetsFilename = L"additional-targets";
    std::vector<std::string> device_list;
    osFilePath               targets_file_path;
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
                std::transform(device.begin(), device.end(), device.begin(), [](const char& c) { return static_cast<char>(std::tolower(c)); });
                device_list.push_back(device);
            }
        }
    }

    return device_list;
}

static void LogPreStep(const std::string& msg, const std::string& device = "")
{
    std::cout << msg << device << "... ";
}

static void LogResult(bool result)
{
    std::cout << (result ? kStrInfoSuccess : kStrInfoFailed) << std::endl;
}

beKA::beStatus KcCLICommanderLightning::Init(const Config& config, LoggingCallbackFunction log_callback)
{
    log_callback_     = log_callback;
    compiler_paths_   = {config.compiler_bin_path, config.compiler_inc_path, config.compiler_lib_path};
    should_print_cmd_ = config.print_process_cmd_line;
    return beKA::kBeStatusSuccess;
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
            std::string           matched_arch_name;

            [[maybe_unused]] bool is_supported_target_extracted = GetSupportedTargets(supported_targets);
            assert(is_supported_target_extracted);

            // If the device is specified in the LLVM format, convert it to the DeviceInfo format.
            auto llvm_device = std::find_if(kLcLlvmTargetsToDeviceInfoTargets.cbegin(),
                                            kLcLlvmTargetsToDeviceInfoTargets.cend(),
                                            [&](const std::pair<std::string, std::string>& d) { return (d.first == device); });
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
                    if (RgaSharedUtils::ToLower(matched_arch_name).find(supported_device) != std::string::npos)
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
                std::transform(device.begin(), device.end(), device.begin(), [](const char& c) { return static_cast<char>(std::tolower(c)); });
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
        options.selected_devices       = targets_;
        options.defines                = config.defines;
        options.include_paths          = config.include_path;
        options.opencl_compile_options = config.opencl_options;
        options.optimization_level     = config.opt_level;
        options.line_numbers           = config.is_line_numbers_required;
        options.should_dump_il         = !config.il_file.empty();

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

        ret = (result == beKA::kBeStatusSuccess);
    }

    return ret;
}

void KcCLICommanderLightning::Version(Config& config, LoggingCallbackFunction callback)
{
    bool              ret;
    std::stringstream log;
    KcCliCommander::Version(config, callback);

    std::string    output_text = "", version = "";
    beKA::beStatus status =
        BeProgramBuilderLightning::GetCompilerVersion(beKA::RgaMode::kModeOpenclOffline, config.compiler_bin_path, config.print_process_cmd_line, output_text);
    ret = (status == beKA::kBeStatusSuccess);
    if (ret)
    {
        size_t offset = output_text.find(kCompilerVersionToken);
        if (offset != std::string::npos)
        {
            offset += kCompilerVersionToken.size();
            size_t offset1 = output_text.find(" ", offset);
            size_t offset2 = output_text.find("\n", offset);
            if (offset1 != std::string::npos && offset2 != std::string::npos)
            {
                size_t end_offset = std::min<size_t>(offset1, offset2);
                version           = output_text.substr(0, end_offset);
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

    // Add the list of supported GPUs to the Version Info file.
    result = result && KcXmlWriter::AddVersionInfoGPUList(beKA::RgaMode::kModeOpenclOffline, targets, filename);

    return result;
}

bool KcCLICommanderLightning::PrintAsicList(const Config&)
{
    std::set<std::string> targets;
    bool                  ret = GetSupportedTargets(targets);
    ret                       = ret && KcUtils::PrintAsicList(targets);

    if (ret)
    {
        // Print additional OpenCL Lightning Compiler target from the "additional-targets" file.
        std::vector<std::string> extra_targets = GetExtraTargetList();
        if (!extra_targets.empty())
        {
            static const char* kStrOpenclOfflineExtraDeviceListTitle = "Additional GPU targets (Warning: correct compilation and analysis are not guaranteed):";
            static const char* kStrOpenclOfflineDeviceListOffset     = "    ";
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

beKA::beStatus KcCLICommanderLightning::CompileOpenCL(const Config& config, const OpenCLOptions& ocl_options)
{
    beKA::beStatus status = beKA::beStatus::kBeStatusSuccess;

    // Run LC compiler for all requested devices
    for (const std::string& device : ocl_options.selected_devices)
    {
        std::string error_text;
        LogPreStep(kStrInfoCompiling, device);
        std::string bin_filename;

        // Adjust the device name if necessary.
        std::string clang_device = device;
        if (kLcDeviceInfoToClangDeviceMap.count(clang_device) > 0)
        {
            clang_device = kLcDeviceInfoToClangDeviceMap.at(device);
        }

        // Update the binary and ISA names for current device.
        beKA::beStatus current_status = AdjustBinaryFileName(config, device, bin_filename);

        // If file with the same name exist, delete it.
        KcUtils::DeleteFile(bin_filename);

        // Prepare a list of source files.
        std::vector<std::string> src_filenames;
        for (const std::string& input_file : config.input_files)
        {
            src_filenames.push_back(input_file);
        }

        if (current_status != beKA::beStatus::kBeStatusSuccess)
        {
            KcUtilsLightning::LogErrorStatus(current_status, error_text);
            continue;
        }

        // Compile source to binary.
        current_status = BeProgramBuilderLightning::CompileOpenCLToBinary(
            compiler_paths_, ocl_options, src_filenames, bin_filename, clang_device, should_print_cmd_, error_text);
        LogResult(current_status == beKA::beStatus::kBeStatusSuccess);

        if (current_status == beKA::beStatus::kBeStatusSuccess)
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
            if (!config.isa_file.empty() || !config.analysis_file.empty() || !config.livereg_analysis_file.empty() ||
                !config.sgpr_livereg_analysis_file.empty() || !config.block_cfg_file.empty() || !config.inst_cfg_file.empty())
            {
                LogPreStep(kStrInfoExtractingIsaForDevice, device);
                current_status =
                    DisassembleBinary(bin_filename, config.isa_file, clang_device, device, config.function, config.is_line_numbers_required, error_text);
                LogResult(current_status == beKA::beStatus::kBeStatusSuccess);

                assert(current_status == beKA::beStatus::kBeStatusSuccess);
                // Propagate the binary file name to the Output Files Metadata table.
                if (current_status == beKA::beStatus::kBeStatusSuccess)
                {
                    for (auto& output_md_node : output_metadata_)
                    {
                        const std::string& md_device = output_md_node.first.first;
                        if (md_device == device)
                        {
                            output_md_node.second.bin_file         = bin_filename;
                            output_md_node.second.is_bin_file_temp = config.binary_output_file.empty();
                        }
                    }
                }
            }
            else
            {
                output_metadata_[{device, ""}] = RgOutputFiles(RgaEntryType::kOpenclKernel, "", bin_filename);
            }
        }
        else
        {
            // Store error status to the metadata.
            RgOutputFiles output(RgaEntryType::kOpenclKernel, "", "");
            output.status                  = false;
            output_metadata_[{device, ""}] = output;
        }

        status = (current_status == beKA::beStatus::kBeStatusSuccess ? status : current_status);
        KcUtilsLightning::LogErrorStatus(current_status, error_text);
    }

    return status;
}

beKA::beStatus KcCLICommanderLightning::DisassembleBinary(const std::string& binFileName,
                                                          const std::string& userIsaFileName,
                                                          const std::string& clangDevice,
                                                          const std::string& rgaDevice,
                                                          const std::string& kernel,
                                                          bool               lineNumbers,
                                                          std::string&       error_text)
{
    std::string                      out_isa_text;
    BeAmdHsaMetaData::AmdHsaMetaData md;
    beKA::beStatus                   status =
        BeProgramBuilderLightning::DisassembleBinary(compiler_paths_.bin, binFileName, clangDevice, lineNumbers, should_print_cmd_, out_isa_text, error_text);
    if (!error_text.empty())
    {
        status = beKA::kBeStatusLightningDisassembleFailed;
        log_callback_(error_text);
    }

    if (status == beKA::kBeStatusSuccess)
    {
        std::string metadata_text;
        status = BeProgramBuilderLightning::ExtractMetadata(compiler_paths_.bin, binFileName, should_print_cmd_, metadata_text, error_text);
        if (!error_text.empty())
        {
            status = beKA::kBeStatusLightningExtractMetadataFailed;
            log_callback_(error_text);
            log_callback_("\n");
        }

        if (status == beKA::kBeStatusSuccess)
        {
            status = BeAmdHsaMetaData::ParseMetadata(metadata_text, md);
        }
    }

    if (status == beKA::kBeStatusSuccess)
    {
        if (!kernel.empty() && std::find(md.kernel_names.cbegin(), md.kernel_names.cend(), kernel) == md.kernel_names.cend())
        {
            error_text = std::string(kStrErrorOpenclOfflineCannotFindKernel) + kernel;
            status     = beKA::kBeStatusWrongKernelName;
        }
        else
        {
            status = SplitISA(binFileName, out_isa_text, userIsaFileName, rgaDevice, kernel, md.kernel_names) ? beKA::kBeStatusSuccess
                                                                                                              : beKA::kBeStatusLightningSplitIsaFailed;
        }
    }
    else
    {
        // Store error status to the metadata.
        RgOutputFiles output(RgaEntryType::kOpenclKernel, "", "");
        output.status                     = false;
        output_metadata_[{rgaDevice, ""}] = output;
    }

    return status;
}

void KcCLICommanderLightning::RunCompileCommands(const Config& config, LoggingCallbackFunction)
{
    if (!config.session_summary_file.empty())
    {
        KcCliIsaSpecLoader::LoadIsaSpecsFromXML(true, config.include_target_metadata, config.asics);
    }

    bool             is_multiple_devices = (config.asics.size() > 1);
    bool             status              = Compile(config);
    KcUtilsLightning util(config.binary_output_file, "", output_metadata_, should_print_cmd_, log_callback_);
    if (status || is_multiple_devices)
    {
        util.RunPostProcessingSteps(config, compiler_paths_);
    }
}

beKA::beStatus KcCLICommanderLightning::AdjustBinaryFileName(const Config& config, const std::string& device, std::string& bin_filename)
{
    beKA::beStatus status = beKA::kBeStatusSuccess;

    gtString    name          = L"";
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

bool KcCLICommanderLightning::SplitISA(const std::string&              bin_file,
                                       const std::string&              isa_text,
                                       const std::string&              user_isa_file_name,
                                       const std::string&              device,
                                       const std::string&              kernel,
                                       const std::vector<std::string>& kernel_names)
{
    // kernelIsaTextMap maps kernel name --> kernel ISA text.
    IsaMap kernel_isa_text_map;
    bool   ret, is_isa_file_temp = user_isa_file_name.empty();

    // Replace labels of format "address   <label_name>:" with "label_name:"
    std::string new_isa_text = KcUtilsLightning::FormatLlvmIsaLabels(isa_text);

    // Split ISA text into per-kernel fragments.
    ret = KcUtilsLightning::SplitISAText(new_isa_text, kernel_names, kernel_isa_text_map);

    // Eliminate the useless code.
    ret = ret && KcUtilsLightning::ReduceISA(bin_file, compiler_paths_, should_print_cmd_, kernel_isa_text_map);

    // Store per-kernel ISA texts to separate files and launch livereg tool for each file.
    if (ret)
    {
        // isaTextMapItem is a pair{kernelName, kernelIsaText}.
        for (const auto& isa_text_map_item : kernel_isa_text_map)
        {
            // Skip the kernels that are not requested.
            if (!kernel.empty() && kernel != isa_text_map_item.first)
            {
                continue;
            }

            gtString    isa_filename;
            std::string function_name = isa_text_map_item.first;

            if (is_isa_file_temp)
            {
                gtString base_isa_filename(kTempIsaFilename);
                base_isa_filename << device.c_str() << "_" << function_name.c_str();
                isa_filename = KcUtils::ConstructTempFileName(base_isa_filename, kTempIsaFileExtension);
            }
            else
            {
                KcUtils::ConstructOutputFileName(user_isa_file_name, "", kStrDefaultExtensionIsa, function_name, device, isa_filename);
            }
            if (!isa_filename.isEmpty())
            {
                if (KcUtils::WriteTextFile(isa_filename.asASCIICharArray(), isa_text_map_item.second, log_callback_))
                {
                    RgOutputFiles outFiles                              = RgOutputFiles(RgaEntryType::kOpenclKernel, isa_filename.asASCIICharArray());
                    outFiles.is_isa_file_temp                           = is_isa_file_temp;
                    output_metadata_[{device, isa_text_map_item.first}] = outFiles;
                }
            }
            else
            {
                std::stringstream error_msg;
                error_msg << kStrErrorOpenclOfflineFailedToCreateTempFile << std::endl;
                log_callback_(error_msg.str());
                ret = false;
            }
        }
    }

    return ret;
}

bool KcCLICommanderLightning::ListEntries(const Config& config, LoggingCallbackFunction callback)
{
    return ListEntriesOpenclOffline(config, callback);
}

bool KcCLICommanderLightning::ListEntriesOpenclOffline(const Config& config, LoggingCallbackFunction callback)
{
    bool              ret = true;
    std::string       filename;
    RgEntryData       entry_data;
    std::stringstream msg;

    if (config.mode != beKA::RgaMode::kModeOpenclOffline)
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

    if (ret && (ret = KcUtilsLightning::ExtractEntries(filename, config, compiler_paths_, entry_data)) == true)
    {
        // Sort the entry names in alphabetical order.
        std::sort(entry_data.begin(), entry_data.end(), [](const std::tuple<std::string, int, int>& a, const std::tuple<std::string, int, int>& b) {
            return (std::get<0>(a) < std::get<0>(b));
        });

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

bool KcCLICommanderLightning::RunPostCompileSteps(const Config& config)
{
    bool ret = false;

    if (!config.session_metadata_file.empty())
    {
        ret = GenerateSessionMetadata(config, compiler_paths_);
        if (!ret)
        {
            std::stringstream msg;
            msg << kStrErrorFailedToGenerateSessionMetdata << std::endl;
            log_callback_(msg.str());
        }
    }

    if (!config.session_summary_file.empty())
    {
        ret = GenerateSessionSummary(config);
    }

    KcUtilsLightning::DeleteTempFiles(output_metadata_);

    return ret;
}

beKA::beStatus KcCLICommanderLightning::DumpIL(const Config&                   config,
                                               const OpenCLOptions&            user_options,
                                               const std::vector<std::string>& src_file_names,
                                               const std::string&              device,
                                               const std::string&              clang_device,
                                               std::string&                    error_text)
{
    beKA::beStatus status = beKA::beStatus::kBeStatusSuccess;

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
        if (status != beKA::beStatus::kBeStatusSuccess)
        {
            std::stringstream msg;
            msg << kStrErrorOpenclOfflineLlvmIrDisassemblyFailure << std::endl;
            error_text.append(msg.str());
        }
    }
    return status;
}

bool KcCLICommanderLightning::GenerateSessionMetadata(const Config& config, const CmpilerPaths& compiler_paths) const
{
    RgFileEntryData file_kernel_data;
    bool            ret = !config.session_metadata_file.empty();
    assert(ret);

    if (ret)
    {
        for (const std::string& input_file : config.input_files)
        {
            RgEntryData entry_data;
            ret = ret && KcUtilsLightning::ExtractEntries(input_file, config, compiler_paths, entry_data);
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

bool KcCLICommanderLightning::GenerateSessionSummary(const Config& config)
{
    bool ret = !config.session_summary_file.empty() && !output_metadata_.empty();
    if (ret)
    {
        RgaAnalysisSummary                                        summary;
        std::map<std::string, RgaAnalysisSummary::AnalysisResult> results_by_device;

        for (const auto& [key, out_files] : output_metadata_)
        {
            const std::string& device      = key.first;
            const std::string& kernel_name = key.second;
            auto&              result      = results_by_device[device];

            if (result.target_architecture_.empty())
            {
                result.target_architecture_ = device;
                if (config.include_target_metadata)
                {
                    beKA::BeIsaSpecExplorer::PopulateFromSpec(device, result.target_architecture_metadata_);
                }
                result.inputs_.inputs_ = config.input_files;
                result.output_.api_    = kStrRgaModeOpenclOffline;
            }

            RgaAnalysisSummary::Kernel kernel;
            if (KcUtils::GenerateKernelSummary(device, kernel_name, out_files, kernel, log_callback_, should_print_cmd_))
            {
                kernel.kernel_id_ = static_cast<int>(result.output_.kernels_.size());
                result.output_.kernels_.push_back(std::move(kernel));
            }
        }

        for (auto& [device, result] : results_by_device)
        {
            summary.results_.push_back(std::move(result));
        }

        ret = KcXmlWriter::WriteAnalysisSummaryToFile(summary, config.session_summary_file);
        if (!ret)
        {
            RgLog::stdOut << kStrErrorFailedToGenerateSessionSummary << std::endl;
        }
    }
    return ret;
}

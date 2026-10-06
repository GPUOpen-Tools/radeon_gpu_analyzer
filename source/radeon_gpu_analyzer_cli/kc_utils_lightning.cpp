//=============================================================================
/// Copyright (c) 2026 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for OpenCL helper functions.
//=============================================================================
// C++
#include <sstream>
#include <regex>

// External.
#include "external/amdt_os_wrappers/Include/osFilePath.h"

// Shared.
#include "common/rg_log.h"
#include "common/rga_cli_defs.h"
#include "common/rga_shared_data_types.h"
#include "common/rga_shared_utils.h"
#include "common/rga_xml_constants.h"

// Backend.
#include "radeon_gpu_analyzer_backend/be_metadata_llvm.h"
#include "radeon_gpu_analyzer_backend/be_metadata_parser.h"
#include "radeon_gpu_analyzer_backend/be_program_builder_lightning.h"
#include "radeon_gpu_analyzer_backend/be_utils.h"

// Local.
#include "radeon_gpu_analyzer_cli/kc_cli_string_constants.h"
#include "radeon_gpu_analyzer_cli/kc_statistics_device_props.h"
#include "radeon_gpu_analyzer_cli/kc_utils.h"
#include "radeon_gpu_analyzer_cli/kc_utils_lightning.h"
#include "radeon_gpu_analyzer_cli/kc_xml_writer.h"

const char* const kStrKernelName = "Kernel name: ";

static const std::string kLcIsaInstructionSuffix1 = "_e32";
static const std::string kLcIsaInstructionSuffix2 = "_e64";

static const std::string kLcIsaBranchToken = "branch";
static const std::string kIsaCallToken     = "call";

static const std::string kIsaInstructionAddressStartToken = "//";
static const std::string kIsaInstructionAddressEndToken   = ":";
static const std::string kIsaCommentStartToken            = ";";

static const std::string kStrDx11NaValue = "N/A";

// Error messages.
static const char* kStrErrorOpenclOfflineCompileError                          = "Error (reported by the OpenCL Compiler):";
static const char* kStrErrorOpenclOfflineFailedToCreateOutputFilenameForKernel = "Error: failed to construct output file name for kernel: ";
static const char* kStrErrorOpenclOfflineFailedToExtractMetadata               = "Error: failed to extract Metadata.";
static const char* kStrErrorOpenclOfflineNoOutputFileGenerated                 = "Error: the output file was not generated.";
static const char* kStrErrorOpenclOfflineDisassemblerError                     = "Error: extracting ISA failed. The disassembler returned error:";
static const char* kStrErrorOpenclOfflineCompileTimeout                        = "Error: the compilation process timed out.";
static const char* kStrErrorOpenclOfflineSplitIsaError                         = "Error: Unable to split ISA contents.";

// Info messages.
static const char* kStrInfoOpenclOfflineKernelForKernel = " for kernel ";

static const std::string kOpenclKernelQualifierToken1   = "__kernel";
static const std::string kOpenclKernelQualifierToken2   = "kernel";
static const std::string kOpenclAttributeQualifierToken = "__attribute__";
static const std::string kOpenclPragmaToken             = "pragma";

static const size_t kIsaInstruction64BitCodeTextSize = 16;
static const int    kIsaInstruction64BitBytes        = 8;
static const int    kIsaInstruction32BitBytes        = 4;

static void LogPreStep(const std::string& msg, const std::string& device = "")
{
    std::cout << msg << device << "... ";
}

static void LogResult(bool result)
{
    std::cout << (result ? kStrInfoSuccess : kStrInfoFailed) << std::endl;
}

void KcUtilsLightning::LogErrorStatus(beKA::beStatus status, const std::string& error_msg)
{
    const char* kStrErrorCannotFindBinary = "Error: cannot find binary file.";
    switch (status)
    {
    case beKA::beStatus::kBeStatusSuccess:
        break;
    case beKA::beStatus::kBeStatusLightningCompilerLaunchFailed:
        std::cout << std::endl << kStrErrorCannotInvokeCompiler << std::endl;
        break;
    case beKA::beStatus::kBeStatusLightningCompilerGeneratedError:
        std::cout << std::endl << kStrErrorOpenclOfflineCompileError << std::endl;
        std::cout << error_msg << std::endl;
        break;
    case beKA::beStatus::kBeStatusNoOutputFileGenerated:
        std::cout << std::endl << kStrErrorOpenclOfflineNoOutputFileGenerated << std::endl;
        break;
    case beKA::beStatus::kBeStatusNoBinaryForDevice:
        std::cout << std::endl << kStrErrorCannotFindBinary << std::endl;
        break;
    case beKA::beStatus::kBeStatusLightningDisassembleFailed:
        std::cout << std::endl << kStrErrorOpenclOfflineDisassemblerError << std::endl;
        std::cout << error_msg << std::endl;
        break;
    case beKA::beStatus::kBeStatusLightningCompilerTimeOut:
        std::cout << std::endl << kStrErrorOpenclOfflineCompileTimeout << std::endl;
        std::cout << error_msg << std::endl;
        break;
    case beKA::beStatus::kBeStatusLightningSplitIsaFailed:
        std::cout << std::endl << kStrErrorOpenclOfflineSplitIsaError << std::endl;
        std::cout << error_msg << std::endl;
        break;
    default:
        std::cout << std::endl << (error_msg.empty() ? kStrErrorUnknownCompilationStatus : error_msg) << std::endl;
        break;
    }
}

bool KcUtilsLightning::ParseIsaFilesToCSV(bool line_numbers) const
{
    bool ret = true;
    for (const auto& output_md_item : output_metadata_)
    {
        if (output_md_item.second.status)
        {
            const RgOutputFiles& output_files = output_md_item.second;
            std::string          isa, parsed_isa, parsed_isa_filename;
            const std::string&   device = output_md_item.first.first;
            const std::string&   entry  = output_md_item.first.second;

            bool status = KcUtils::ReadTextFile(output_files.isa_file, isa, nullptr);
            if (status)
            {
                if ((status = KcUtilsLightning::GetParsedIsaCsvText(isa, device, line_numbers, parsed_isa)) == true)
                {
                    status = (KcUtils::GetParsedISAFileName(output_files.isa_file, parsed_isa_filename) == beKA::kBeStatusSuccess);
                    if (status)
                    {
                        status = (KcUtilsLightning::WriteIsaToFile(parsed_isa_filename, parsed_isa, log_callback_) == beKA::kBeStatusSuccess);
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

bool KcUtilsLightning::PerformLiveVgprAnalysis(const Config& config) const
{
    bool              ret = true;
    std::stringstream error_msg;

    for (auto& output_md_item : output_metadata_)
    {
        RgOutputFiles& output_files = output_md_item.second;
        if (output_files.status)
        {
            const std::string& device               = output_md_item.first.first;
            const std::string& entry_name           = output_md_item.first.second;
            const std::string& entry_abbrivation    = output_md_item.second.entry_abbreviation;
            gtString           livereg_out_filename = L"";
            gtString           isa_filename;
            isa_filename << output_files.isa_file.c_str();
            gtString device_gtstr;
            device_gtstr << device.c_str();

            // Inform the user.
            std::cout << kStrInfoPerformingLiveregAnalysisVgpr << device << kStrInfoOpenclOfflineKernelForKernel << entry_name << "... ";

            // Construct a name for the output livereg file.
            if (entry_abbrivation.empty())
            {
                KcUtils::ConstructOutputFileName(
                    config.livereg_analysis_file, kStrDefaultExtensionLivereg, kStrDefaultExtensionText, entry_name, device, livereg_out_filename);
            }
            else
            {
                KcUtils::ConstructOutputFileName(
                    config.livereg_analysis_file, kStrDefaultExtensionLivereg, kStrDefaultExtensionText, entry_abbrivation, device, livereg_out_filename);
            }

            if (!livereg_out_filename.isEmpty())
            {
                // Perform live VGPR analysis and force wave 64 for OpenCL kernels. Currently the wave size information
                // is missing from LLVM disassembly, therefore Shae is not able to deduce the value from the disassembly.
                // Therefore we will use a default of wave64 (this would be ignored by Shae for pre-RDNA targets).
                KcUtils::PerformLiveRegisterAnalysis(
                    isa_filename, device_gtstr, livereg_out_filename, log_callback_, config.print_process_cmd_line, false, output_files.wave_size);
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

bool KcUtilsLightning::PerformLiveSgprAnalysis(const Config& config) const
{
    bool              ret = true;
    std::stringstream error_msg;

    for (auto& output_md_item : output_metadata_)
    {
        RgOutputFiles& output_files = output_md_item.second;
        if (output_files.status)
        {
            const std::string& device               = output_md_item.first.first;
            const std::string& entry_name           = output_md_item.first.second;
            const std::string& entry_abbrivation    = output_md_item.second.entry_abbreviation;
            gtString           livereg_out_filename = L"";
            gtString           isa_filename;
            isa_filename << output_files.isa_file.c_str();
            gtString device_gtstr;
            device_gtstr << device.c_str();

            // Inform the user.
            std::cout << kStrInfoPerformingLiveregAnalysisSgpr << device << kStrInfoOpenclOfflineKernelForKernel << entry_name << "... ";

            // Construct a name for the output livereg file.
            if (entry_abbrivation.empty())
            {
                KcUtils::ConstructOutputFileName(
                    config.sgpr_livereg_analysis_file, kStrDefaultExtensionLiveregSgpr, kStrDefaultExtensionText, entry_name, device, livereg_out_filename);
            }
            else
            {
                KcUtils::ConstructOutputFileName(config.sgpr_livereg_analysis_file,
                                                 kStrDefaultExtensionLiveregSgpr,
                                                 kStrDefaultExtensionText,
                                                 entry_abbrivation,
                                                 device,
                                                 livereg_out_filename);
            }

            if (!livereg_out_filename.isEmpty())
            {
                // Perform live SGPR analysis and force wave 64 for OpenCL kernels. Currently the wave size information
                // is missing from LLVM disassembly, therefore Shae is not able to deduce the value from the disassembly.
                // Therefore we will use a default of wave64 (this would be ignored by Shae for pre-RDNA targets).
                KcUtils::PerformLiveRegisterAnalysis(
                    isa_filename, device_gtstr, livereg_out_filename, log_callback_, config.print_process_cmd_line, true, output_files.wave_size);
                if (BeProgramBuilderLightning::VerifyOutputFile(livereg_out_filename.asASCIICharArray()))
                {
                    // Store the name of livereg output file in the RGA output files metadata.
                    output_files.livereg_sgpr_file = livereg_out_filename.asASCIICharArray();
                    std::cout << kStrInfoSuccess << std::endl;
                }
                else
                {
                    error_msg << kStrErrorCannotPerformLiveregAnalysisSgpr << " " << kStrKernelName << entry_name << std::endl;
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

bool KcUtilsLightning::ExtractCFG(const Config& config) const
{
    bool              ret = true;
    std::stringstream error_msg;

    for (auto& output_md_item : output_metadata_)
    {
        RgOutputFiles& outputFiles = output_md_item.second;

        if (outputFiles.status)
        {
            const std::string& device            = output_md_item.first.first;
            const std::string& entry_name        = output_md_item.first.second;
            const std::string& entry_abbrivation = output_md_item.second.entry_abbreviation;
            gtString           cfg_out_filename  = L"";
            gtString           isa_filename;
            isa_filename << outputFiles.isa_file.c_str();
            gtString device_gtstr;
            device_gtstr << device.c_str();

            bool is_per_basic_block_cfg = !config.block_cfg_file.empty();
            std::cout << (is_per_basic_block_cfg ? kStrInfoContructingPerBlockCfg1 : kStrInfoContructingPerInstructionCfg1) << device
                      << kStrInfoOpenclOfflineKernelForKernel << entry_name << "... ";

            // Construct a name for the output CFG file.
            std::string base_file = (is_per_basic_block_cfg ? config.block_cfg_file : config.inst_cfg_file);
            if (entry_abbrivation.empty())
            {
                KcUtils::ConstructOutputFileName(base_file, KC_STR_DEFAULT_CFG_SUFFIX, kStrDefaultExtensionDot, entry_name, device, cfg_out_filename);
            }
            else
            {
                KcUtils::ConstructOutputFileName(base_file, KC_STR_DEFAULT_CFG_SUFFIX, kStrDefaultExtensionDot, entry_abbrivation, device, cfg_out_filename);
            }
            if (!cfg_out_filename.isEmpty())
            {
                KcUtils::GenerateControlFlowGraph(
                    isa_filename, device_gtstr, cfg_out_filename, log_callback_, !config.inst_cfg_file.empty(), config.print_process_cmd_line);

                if (!BeProgramBuilderLightning::VerifyOutputFile(cfg_out_filename.asASCIICharArray()))
                {
                    error_msg << kStrErrorCannotGenerateCfg << " " << kStrKernelName << entry_name << std::endl;
                    std::cout << kStrInfoFailed << std::endl;
                    ret = false;
                }
                else
                {
                    outputFiles.cfg_file = cfg_out_filename.asASCIICharArray();
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

beKA::beStatus KcUtilsLightning::ExtractMetadata(const CmpilerPaths& compiler_paths, const std::string& metadata_filename) const
{
    beKA::beStatus current_status = beKA::beStatus::kBeStatusSuccess;
    beKA::beStatus status         = beKA::beStatus::kBeStatusSuccess;
    std::string    metadata_text, error_text;
    gtString       out_filename;

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
                const std::string  bin_filename           = output_md_node.second.bin_file;
                static const char* kStrDefaultExtensionMd = "md";
                KcUtils::ConstructOutputFileName(metadata_filename, "", kStrDefaultExtensionMd, kStrDefaultExtensionText, device, out_filename);
                if (!out_filename.isEmpty())
                {
                    current_status = BeProgramBuilderLightning::ExtractMetadata(compiler_paths.bin, bin_filename, should_print_cmd_, metadata_text, error_text);
                    if (!error_text.empty())
                    {
                        current_status = beKA::kBeStatusLightningExtractMetadataFailed;
                        log_callback_(error_text);
                        log_callback_("\n");
                    }

                    if (current_status == beKA::beStatus::kBeStatusSuccess && !metadata_text.empty())
                    {
                        current_status = KcUtils::WriteTextFile(out_filename.asASCIICharArray(), metadata_text, log_callback_)
                                             ? beKA::beStatus::kBeStatusSuccess
                                             : beKA::beStatus::kBeStatusWriteToFileFailed;
                    }
                }
                else
                {
                    current_status = beKA::beStatus::kBeStatusLightningExtractMetadataFailed;
                }
            }
            status = (current_status == beKA::beStatus::kBeStatusSuccess ? status : beKA::beStatus::kBeStatusLightningExtractMetadataFailed);
        }
    }

    if (status != beKA::beStatus::kBeStatusSuccess)
    {
        std::stringstream msg;
        msg << kStrErrorOpenclOfflineFailedToExtractMetadata << std::endl;
        log_callback_(msg.str());
    }

    return status;
}

// Get the ISA size and store it to "kernelCodeProps" structure.
static beKA::beStatus GetIsaSize(const std::string& isaFileName, BeAmdHsaMetaData::KernelProperties& kernelCodeProps)
{
    beKA::beStatus status = beKA::beStatus::kBeStatusLightningGetISASizeFailed;
    if (!isaFileName.empty())
    {
        std::string isa_text;
        if (KcUtils::ReadTextFile(isaFileName, isa_text, nullptr))
        {
            int isa_size = BeProgramBuilderLightning::GetIsaSize(isa_text);
            if (isa_size != -1)
            {
                kernelCodeProps.isa_size = isa_size;
                status                   = beKA::beStatus::kBeStatusSuccess;
            }
        }
    }

    return status;
}

// Build the statistics in "AnalysisData" form.
static bool BuildAnalysisData(const BeAmdHsaMetaData::KernelProperties& kernel_code_props, const std::string& device, beKA::AnalysisData& stats)
{
    uint64_t min_sgprs = 0, min_vgprs = 0;

    // Set unknown values to 0.
    memset(&stats, 0, sizeof(beKA::AnalysisData));
    if (kRgaDeviceProps.count(device))
    {
        const DeviceProps& deviceProps = kRgaDeviceProps.at(device);
        stats.lds_size_available       = deviceProps.available_lds_bytes;
        stats.num_sgprs_available      = deviceProps.available_sgprs;
        stats.num_vgprs_available      = deviceProps.available_vgprs;
        stats.num_agprs_available      = deviceProps.available_agprs;
        min_sgprs                      = deviceProps.min_sgprs;
        min_vgprs                      = deviceProps.min_vgprs;
    }
    else
    {
        stats.lds_size_available  = static_cast<uint64_t>(-1);
        stats.num_sgprs_available = static_cast<uint64_t>(-1);
        stats.num_vgprs_available = static_cast<uint64_t>(-1);
    }
    stats.num_threads_per_group_total = static_cast<uint64_t>(-1);
    stats.num_threads_per_group_x     = static_cast<uint64_t>(-1);
    stats.num_threads_per_group_y     = static_cast<uint64_t>(-1);
    stats.num_threads_per_group_z     = static_cast<uint64_t>(-1);

    stats.scratch_memory_used = kernel_code_props.private_segment_size;
    stats.lds_size_used       = kernel_code_props.workgroup_segment_size;
    stats.num_sgprs_used      = std::max<uint64_t>(min_sgprs, kernel_code_props.wavefront_num_sgprs);
    stats.num_vgprs_used      = std::max<uint64_t>(min_vgprs, kernel_code_props.work_item_num_vgprs);
    stats.num_agprs_used      = kernel_code_props.work_item_num_agprs;

    // On architectures with unified VGPR/AGPR register files, the .vgpr_count attribute in the Code Object includes AGPRs.
    // Subtract AGPRs to get the actual VGPR count.
    if (stats.num_agprs_used > 0 && stats.num_vgprs_used >= stats.num_agprs_used)
    {
        stats.num_vgprs_used -= stats.num_agprs_used;
    }

    stats.num_sgpr_spills     = kernel_code_props.sgpr_spills;
    stats.num_vgpr_spills     = kernel_code_props.vgpr_spills;
    stats.wavefront_size      = kernel_code_props.wavefront_size;
    stats.isa_size            = kernel_code_props.isa_size;

    assert(stats.wavefront_size == 32 || stats.wavefront_size == 64);

    return true;
}

// Construct statistics text and store it to CSV file.
static bool StoreStatistics(const Config&             config,
                            const std::string&        base_stats_filename,
                            const std::string&        device,
                            const std::string&        kernel,
                            const beKA::AnalysisData& stats,
                            std::string&              out_filename)
{
    bool     ret = false;
    gtString stats_filename;
    KcUtils::ConstructOutputFileName(base_stats_filename, kStrDefaultExtensionStats, kStrDefaultExtensionCsv, kernel, device, stats_filename);

    if (!stats_filename.isEmpty())
    {
        out_filename = stats_filename.asASCIICharArray();
        std::stringstream stats_text;
        char              separator = KcUtils::GetCsvSeparator(config);

        bool include_agprs = RgaSharedUtils::HasAgprSupport(device);
        stats_text << KcUtils::GetStatisticsCsvHeaderString(separator, include_agprs) << std::endl;
        stats_text << device << separator;
        stats_text << beKA::AnalysisData::na_or(stats.scratch_memory_used) << separator;
        stats_text << beKA::AnalysisData::na_or(stats.num_threads_per_group_total) << separator;
        stats_text << beKA::AnalysisData::na_or(stats.wavefront_size) << separator;
        stats_text << beKA::AnalysisData::na_or(stats.lds_size_available) << separator;
        stats_text << beKA::AnalysisData::na_or(stats.lds_size_used) << separator;
        stats_text << beKA::AnalysisData::na_or(stats.num_sgprs_available) << separator;
        stats_text << beKA::AnalysisData::na_or(stats.num_sgprs_used) << separator;
        stats_text << beKA::AnalysisData::na_or(stats.num_sgpr_spills) << separator;
        stats_text << beKA::AnalysisData::na_or(stats.num_vgprs_available) << separator;
        stats_text << beKA::AnalysisData::na_or(stats.num_vgprs_used) << separator;
        stats_text << beKA::AnalysisData::na_or(stats.num_vgpr_spills) << separator;
        stats_text << beKA::AnalysisData::na_or(stats.num_threads_per_group_x) << separator;
        stats_text << beKA::AnalysisData::na_or(stats.num_threads_per_group_y) << separator;
        stats_text << beKA::AnalysisData::na_or(stats.num_threads_per_group_z) << separator;
        stats_text << beKA::AnalysisData::na_or(stats.isa_size);
        if (include_agprs)
        {
            stats_text << separator << beKA::AnalysisData::na_or(stats.num_agprs_available);
            stats_text << separator << beKA::AnalysisData::na_or(stats.num_agprs_used);
        }
        stats_text << std::endl;

        ret = KcUtils::WriteTextFile(stats_filename.asASCIICharArray(), stats_text.str(), nullptr);
    }

    return ret;
}

beKA::beStatus KcUtilsLightning::ExtractStatistics(const Config& config) const
{
    std::string    device = "", stat_filename = config.analysis_file, out_stat_filename;
    beKA::beStatus status = beKA::beStatus::kBeStatusSuccess;

    if (!stat_filename.empty())
    {
        LogPreStep(kStrInfoExtractingStats);
    }

    for (auto& output_md_item : output_metadata_)
    {
        beKA::AnalysisData               stats_data;
        BeAmdHsaMetaData::AmdHsaMetaData md;
        const std::string&               current_device = output_md_item.first.first;
        if (device != current_device && output_md_item.second.status)
        {
            std::string metadata_text, error_text;
            status = BeProgramBuilderLightning::ExtractMetadata(
                config.compiler_bin_path, output_md_item.second.bin_file, config.print_process_cmd_line, metadata_text, error_text);
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
            if (status != beKA::beStatus::kBeStatusSuccess)
            {
                break;
            }
            for (auto& kernel_code_props : md.props_map)
            {
                if (config.function.empty() || config.function == kernel_code_props.first)
                {
                    auto out_files = output_metadata_.find({current_device, kernel_code_props.first});
                    if (out_files != output_metadata_.end())
                    {
                        out_files->second.wave_size = BeAmdPalMetaData::GetWaveSize(kernel_code_props.second.wavefront_size);

                        if (!stat_filename.empty())
                        {
                            std::string        entry_name{kernel_code_props.first};
                            const std::string& entry_abbrivation = out_files->second.entry_abbreviation;
                            if (!entry_abbrivation.empty())
                            {
                                entry_name = entry_abbrivation;
                            }

                            std::string current_isa_file;
                            KcUtils::ConstructOutputFileName(config.isa_file, "", kStrDefaultExtensionIsa, entry_name, current_device, current_isa_file);
                            if (GetIsaSize(current_isa_file, kernel_code_props.second) &&
                                BuildAnalysisData(kernel_code_props.second, current_device, stats_data))
                            {
                                status = StoreStatistics(config, stat_filename, current_device, entry_name, stats_data, out_stat_filename)
                                             ? status
                                             : beKA::beStatus::kBeStatusWriteToFileFailed;
                                if (status == beKA::beStatus::kBeStatusSuccess)
                                {
                                    out_files->second.stats_file = out_stat_filename;
                                }
                            }
                        }
                    }
                }
            }

            device = output_md_item.first.first;
        }
    }

    if (!stat_filename.empty())
    {
        LogResult(status == beKA::beStatus::kBeStatusSuccess);
    }

    return status;
}

bool KcUtilsLightning::GetParsedIsaCsvText(const std::string& isa_text, const std::string& device, bool add_line_numbers, std::string& csv_text)
{
    // CSV headers defined in rga_shared_data_types.h.
    bool        ret = false;
    std::string parsed_isa;
    if (BeProgramBuilder::ParseIsaToCsv(isa_text, device, parsed_isa, add_line_numbers, true) == beKA::kBeStatusSuccess)
    {
        csv_text =
            (add_line_numbers ? kStrCsvHeaderWithLineCorrelation : kStrCsvHeaderNoLineCorrelation) + parsed_isa;
        ret = true;
    }
    return ret;
}

beKA::beStatus KcUtilsLightning::WriteIsaToFile(const std::string& file_name, const std::string& isa_text, LoggingCallbackFunction log_callback)
{
    beKA::beStatus ret = beKA::beStatus::kBeStatusInvalid;
    ret = KcUtils::WriteTextFile(file_name, isa_text, log_callback) ? beKA::beStatus::kBeStatusSuccess : beKA::beStatus::kBeStatusWriteToFileFailed;
    if (ret != beKA::beStatus::kBeStatusSuccess)
    {
        RgLog::stdErr << kStrErrorFailedToWriteIsaFile << file_name << std::endl;
    }
    return ret;
}

// Gather the definitions and include paths into a single "options" string.
static std::string GatherOCLOptions(const Config& config)
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

// Parse a preprocessor hint.
// Example:
//    "# 2 "some folder/test.cl" 24"
// Output:
//    {"2", "some folder/test.cl", "24"}
static void ParsePreprocessorHint(const std::string& hint_line, std::vector<std::string>& hint_items)
{
    hint_items.clear();
    size_t end_offset, offset = 0;
    while ((offset = hint_line.find_first_not_of(' ', offset)) != std::string::npos)
    {
        if (hint_line[offset] == '"' && (end_offset = hint_line.find('"', offset + 1)) != std::string::npos)
        {
            // The preprocessor generates double back-slash as a path delimiter on Windows.
            // Replace them with single slashes here.
            std::string file_path = hint_line.substr(offset + 1, end_offset - offset - 1);
            char        prev      = 0;
            auto        found     = [&](char& c) {
                bool ret = (prev == '\\' && c == '\\');
                prev     = c;
                return ret;
            };
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
static size_t ParsePreprocessorLine(const std::string& text, const std::string& filename, size_t offset, unsigned int& file_offset, unsigned int& line_number)
{
    // Parse the preprocessor hint line to get the file name and line offset.
    // We are interested in hints like:  # <file_offset> <file_name>
    //                              or:  # <file_offset> <file_name> 2
    // If the source file found in the hint is not "our" file, put 0 as file offset.
    size_t                   eol = text.find_first_of('\n', offset);
    std::vector<std::string> hint_items;
    ParsePreprocessorHint(text.substr(offset + 1, eol - offset - 1), hint_items);
    if (hint_items.size() == 2 || (hint_items.size() == 3 && std::atoi(hint_items[2].c_str()) == 2))
    {
        offset = std::atoi(hint_items[0].c_str());
        if (offset > 0 && hint_items[1] == filename)
        {
            file_offset = static_cast<unsigned int>(offset);
            line_number = 0;
        }
        else
        {
            file_offset = 0;
        }
    }

    return eol;
}

// Checks if text[offset] is a start of OpenCL kernel qualifier ("kernel" of "__kernel" token).
// Spaces are ignored.
// The offset of the first symbol after the qualifier is returned in "offset".
inline static bool IsKernelQual(const std::string& text, unsigned char prev_symbol, size_t& offset)
{
    bool is_found = false;
    if ((prev_symbol == ' ' || prev_symbol == '\n' || prev_symbol == '}'))
    {
        size_t qual_size = 0;
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
static void SkipAttributeQual(const std::string& text, size_t& offset)
{
    if ((offset = text.find_first_not_of(" \n", offset)) != std::string::npos)
    {
        if (text.compare(offset, kOpenclAttributeQualifierToken.size(), kOpenclAttributeQualifierToken) == 0)
        {
            // Skip the attribute arguments as well.
            offset += kOpenclAttributeQualifierToken.size();
            size_t current_offset = offset;
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

// Parses a kernel declaration. Puts kernel name and starting line number to the "entryDeclInfo".
// Returns "true" if successfully parsed the kernel declaration or "false" otherwise.
static bool ParseKernelDecl(const std::string&                 text,
                            size_t&                            offset,
                            unsigned int                       file_offset,
                            size_t                             kernel_qual_start,
                            unsigned int&                      line_number,
                            std::tuple<std::string, int, int>& entry_decl_info)
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
            std::string kernel_name = text.substr(kernel_name_start, kernel_name_end - kernel_name_start + 1);
            offset                  = kernel_name_end;
            if (!kernel_name.empty())
            {
                // Store the found kernel name and corresponding line number to "entryDeclInfo".
                std::get<0>(entry_decl_info) = kernel_name;
                std::get<1>(entry_decl_info) = (file_offset == 0 ? 0 : file_offset + line_number);
                ret                          = true;
            }
        }
    }

    // Count the number of lines between the kernel qualifier and the kernel name.
    line_number += (unsigned)std::count(text.begin() + kernel_qual_start, text.begin() + kernel_name_end, '\n');
    return ret;
}

// Extracts list of kernel names from OpenCL source text.
// Returns kernel names in "entryData" as a vector of pairs {kernel_name, src_line}.
static bool ExtractEntriesPreprocessed(std::string& text, const std::string& file_name, RgEntryData& entry_data)
{
    //  # 1 "test.cl" 2
    //  # 12 "test.cl"   <-- preprocessor hint (file offset = 12)
    //
    //  __kernel void bar(global int *N)  <-- The number of this line in the original file =
    //                                        the number of this line in preprocessed file + file offet.

    size_t                            offset = 0, kernel_qual_start = 0, size = text.size();
    unsigned int                      file_offset = 0, line_number = 0, bracket_count = 0;
    unsigned char                     prev_symbol = '\n';
    bool                              in_kernel   = false;
    std::tuple<std::string, int, int> entry_decl_info;

    // Replace tabs with spaces.
    std::replace(text.begin(), text.end(), '\t', ' ');

    // Start parsing.
    while (offset < size)
    {
        switch (text[offset])
        {
        case ' ':
            break;
        case '\n':
            line_number++;
            break;
        case '"':
            while (++offset < size && (text[offset] != '"' || text[offset - 1] == '\\'))
            {
            };
            break;
        case '\'':
            while (++offset < size && (text[offset] != '\'' || text[offset - 1] == '\\'))
            {
            };
            break;
        case '{':
            bracket_count++;
            break;

        case '}':
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

bool KcUtilsLightning::ExtractEntries(const std::string& file_name, const Config& config, const CmpilerPaths& compiler_paths, RgEntryData& entry_data)
{
    bool ret = false;

    // Gather the options
    std::string options = GatherOCLOptions(config);

    // Call OpenCL compiler preprocessor.
    std::string    prep_src;
    beKA::beStatus status = BeProgramBuilderLightning::PreprocessOpencl(compiler_paths, file_name, options, config.print_process_cmd_line, prep_src);
    if (status == beKA::beStatus::kBeStatusSuccess)
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

void KcUtilsLightning::RunPostProcessingSteps(const Config& config, const CmpilerPaths& compiler_paths) const
{
    beKA::beStatus status = beKA::beStatus::kBeStatusSuccess;

    // Generate CSV files with parsed ISA if required.
    if (config.is_parsed_isa_required)
    {
        status = ParseIsaFilesToCSV(config.is_line_numbers_required) ? beKA::beStatus::kBeStatusSuccess : beKA::beStatus::kBeStatusParseIsaToCsvFailed;
    }

    // Extract Statistics if required.
    if ((status == beKA::beStatus::kBeStatusSuccess) && !config.analysis_file.empty())
    {
        ExtractStatistics(config);
    }

    // Block post-processing until quality of analysis engine improves when processing llvm disassembly.
    bool is_livereg_required = !config.livereg_analysis_file.empty();
    if (is_livereg_required && (status == beKA::beStatus::kBeStatusSuccess))
    {
        // Perform Live Registers analysis if required.
        PerformLiveVgprAnalysis(config);
    }

    bool is_sgpr_livereg_required = !config.sgpr_livereg_analysis_file.empty();
    if (is_sgpr_livereg_required && (status == beKA::beStatus::kBeStatusSuccess))
    {
        // Perform Live Registers analysis if required.
        PerformLiveSgprAnalysis(config);
    }

    bool is_cfg_required = (!config.block_cfg_file.empty() || !config.inst_cfg_file.empty());
    if (is_cfg_required && (status == beKA::beStatus::kBeStatusSuccess))
    {
        // Extract Control Flow Graph.
        ExtractCFG(config);
    }

    // Extract CodeObj metadata if required.
    if ((status == beKA::beStatus::kBeStatusSuccess) && !config.metadata_file.empty())
    {
        ExtractMetadata(compiler_paths, config.metadata_file);
    }
}

void KcUtilsLightning::DeleteTempFiles(const RgClOutputMetadata& output_metadata)
{
    for (const auto& out_file_data : output_metadata)
    {
        const RgOutputFiles out_files = out_file_data.second;
        gtString            filename;
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

std::string KcUtilsLightning::PrefixWithISAHeader(const std::string& kernel_name, const std::string& kernel_isa_text)
{
    std::stringstream kernel_isa_text_ss;
    kernel_isa_text_ss << kLcKernelIsaHeader1 << "\"" << kernel_name << "\"" << std::endl
                       << std::endl
                       << kLcKernelIsaHeader2 << "\"" << kernel_name << "\":" << std::endl
                       << std::endl
                       << kLcKernelIsaHeader3;
    kernel_isa_text_ss << kernel_isa_text;
    return kernel_isa_text_ss.str();
}

std::string KcUtilsLightning::FormatLlvmIsaLabels(const std::string& isa_text)
{
    std::string label_pattern_prefix("[0-9a-zA-Z]+ <");
    std::regex  label_prefix_regex(label_pattern_prefix);
    std::regex  label_suffix_regex(">:");
    std::string isa_text_clean_prefix = std::regex_replace(isa_text, label_prefix_regex, "");
    return std::regex_replace(isa_text_clean_prefix, label_suffix_regex, ":");
}

bool KcUtilsLightning::SplitISAText(const std::string&                  isa_text,
                                    const std::vector<std::string>&     kernel_names,
                                    std::map<std::string, std::string>& kernel_isa_map)
{
    bool              status               = true;
    const std::string LABEL_NAME_END_TOKEN = ":\n";
    const std::string BLOCK_END_TOKEN      = "\n\n";
    size_t            label_name_start = 0, label_name_end = 0, kernel_isa_end = 0;

    label_name_start = isa_text.find_first_not_of('\n');

    std::vector<std::pair<size_t, size_t>> kernel_start_offsets;
    if (!isa_text.empty())
    {
        while ((label_name_end = isa_text.find(LABEL_NAME_END_TOKEN, label_name_start)) != std::string::npos)
        {
            // Check if this contains a kernel name.
            std::string label_name = isa_text.substr(label_name_start, label_name_end - label_name_start);
            if (std::count(kernel_names.begin(), kernel_names.end(), label_name) != 0)
            {
                kernel_start_offsets.push_back({label_name_start, label_name_end - label_name_start});
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
        size_t isa_text_start = kernel_start_offsets[i].first;
        size_t isa_text_end   = (i < size - 1 ? kernel_start_offsets[i + 1].first - 1 : isa_text.size());
        if (isa_text_start <= isa_text_end)
        {
            const std::string& kernel_isa  = isa_text.substr(isa_text_start, isa_text_end - isa_text_start);
            const std::string& kernel_name = isa_text.substr(kernel_start_offsets[i].first, kernel_start_offsets[i].second);
            kernel_isa_map[kernel_name]    = KcUtilsLightning::PrefixWithISAHeader(kernel_name, kernel_isa);
            label_name_start               = kernel_isa_end + BLOCK_END_TOKEN.size();
        }
        else
        {
            status = false;
            break;
        }
    }

    return status;
}

static void GatherBranchTargets(std::stringstream& isa, std::unordered_map<std::string, bool>& branch_targets)
{
    // The format of branch instruction text:
    //
    //     s_cbranch_scc1 BB0_3        // 000000001110: BF85001C
    //           ^         ^                    ^          ^
    //           |         |                    |          |
    //      instruction  label               offset       code

    std::string isa_line;

    // Skip lines before the actual ISA code.
    while (std::getline(isa, isa_line) && isa_line.find(kLcKernelIsaHeader3) == std::string::npos)
    {
    }

    // Gather target labels of all branch instructions.
    while (std::getline(isa, isa_line))
    {
        size_t inst_end_offset, branch_token_offset, instOffset = isa_line.find_first_not_of(" \t");
        if (instOffset != std::string::npos)
        {
            if ((branch_token_offset = isa_line.find(kLcIsaBranchToken, instOffset)) != std::string::npos ||
                (branch_token_offset = isa_line.find(kIsaCallToken, instOffset)) != std::string::npos)
            {
                if ((inst_end_offset = isa_line.find_first_of(" \t", instOffset)) != std::string::npos && branch_token_offset < inst_end_offset)
                {
                    // Found branch instruction. Add its target label to the list.
                    size_t label_start_offset, label_end_offset;
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
bool IsUnreferencedLabel(const std::string& isa_line, const std::unordered_map<std::string, bool>& branch_targets)
{
    bool ret = false;

    // Looking for strings of the pattern 'anylabel:' that are not function labels.
    size_t colon_indx = isa_line.find(':');
    size_t line_size  = isa_line.size();
    if ((colon_indx == (line_size - 1)) && (line_size > 1))
    {
        std::string branch_name = isa_line.substr(0, isa_line.size() - 1);
        ret                     = (branch_targets.find(branch_name) == branch_targets.end());
    }

    return ret;
}

// Remove non-standard instruction suffixes.
static void FilterISALine(std::string& isa_line)
{
    size_t offset = isa_line.find_first_not_of(" \t");
    if (offset != std::string::npos)
    {
        offset = isa_line.find_first_of(" ");
    }
    if (offset != std::string::npos)
    {
        size_t suffix_length = 0;
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
bool KcUtilsLightning::ReduceISA(const std::string&                  bin_file,
                                 const CmpilerPaths&                 compiler_paths,
                                 bool                                verbose,
                                 std::map<std::string, std::string>& kernel_isa_text)
{
    bool ret = false;
    for (auto& kernel_isa : kernel_isa_text)
    {
        int code_size = BeProgramBuilderLightning::GetKernelCodeSize(compiler_paths.bin, bin_file, kernel_isa.first, verbose);
        assert(code_size != -1);
        if (code_size != -1)
        {
            // Copy ISA lines to new stream. Stop when found an instruction with address > codeSize.
            std::stringstream old_isa, new_isa, address_stream;
            old_isa.str(kernel_isa.second);
            std::string isa_line;
            int         address, address_offset = -1;

            // Gather the target labels of all branch instructions.
            std::unordered_map<std::string, bool> branch_targets;
            branch_targets.clear();
            GatherBranchTargets(old_isa, branch_targets);

            // Skip lines before the actual ISA code.
            while (std::getline(old_isa, isa_line) && new_isa << isa_line << std::endl && isa_line.find(kLcKernelIsaHeader3) == std::string::npos)
            {
            }

            while (std::getline(old_isa, isa_line))
            {
                // Add the ISA line to the new ISA text if it's not an unreferenced label.
                if (!IsUnreferencedLabel(isa_line, branch_targets))
                {
                    if (isa_line.find(" <") != 0)
                    {
                        size_t branch_label_start = isa_line.find(" <") + 2;
                        size_t branch_label_end   = isa_line.find(">:");
                        size_t address_end        = isa_line.find_first_of(" ");
                        if (branch_label_end != std::string::npos)
                        {
                            // If this is a branch label, reformat and add the string so the RGA GUI recognizes the syntax.
                            std::string new_branch_label =
                                isa_line.substr(0, address_end + 1) + isa_line.substr(branch_label_start, branch_label_end - (branch_label_start)) + ":";
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
                    size_t address_start, address_end;

                    FilterISALine(isa_line);

                    if ((address_start = isa_line.find(kIsaInstructionAddressStartToken)) != std::string::npos &&
                        (address_end = isa_line.find(kIsaInstructionAddressEndToken, address_start)) != std::string::npos)
                    {
                        address_start += (kIsaInstructionAddressStartToken.size());
                        address_stream.str(isa_line.substr(address_start, address_end - address_start));
                        address_stream.clear();
                        int inst_size =
                            (isa_line.size() - address_end < kIsaInstruction64BitCodeTextSize) ? kIsaInstruction32BitBytes : kIsaInstruction64BitBytes;
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

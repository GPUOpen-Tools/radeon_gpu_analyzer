// C++
#include <sstream>

// External.
#include "external/amdt_os_wrappers/Include/osFilePath.h"

// Shared.
#include "common/rg_log.h"
#include "common/rga_shared_utils.h"

// Backend.
#include "source/radeon_gpu_analyzer_backend/be_program_builder_lightning.h"

// Local.
#include "source/radeon_gpu_analyzer_cli/kc_cli_commander_lightning_util.h"
#include "source/radeon_gpu_analyzer_cli/kc_cli_string_constants.h"
#include "source/radeon_gpu_analyzer_cli/kc_utils.h"
#include "radeon_gpu_analyzer_cli/kc_xml_writer.h"
#include "source/radeon_gpu_analyzer_cli/kc_statistics_device_props.h"

const char* const kStrKernelName = "Kernel name: ";

// Error messages.
static const char* kStrErrorOpenclOfflineCompileError                          = "Error (reported by the OpenCL Compiler):";
static const char* kStrErrorOpenclOfflineFailedToCreateOutputFilenameForKernel = "Error: failed to construct output file name for kernel: ";
static const char* kStrErrorOpenclOfflineFailedToExtractMetadata               = "Error: failed to extract Metadata.";
static const char* kStrErrorOpenclOfflineNoOutputFileGenerated                 = "Error: the output file was not generated.";
static const char* kStrErrorOpenclOfflineDisassemblerError                     = "Error: extracting ISA failed. The disassembler returned error:";
static const char* kStrErrorOpenclOfflineCompileTimeout                        = "Error: the compilation process timed out.";
static const char* kStrErrorOpenclOfflineSplitIsaError                         = "Error: Unable to split ISA contents.";


// Info messages.
static const char* kStrInfoOpenclOfflineKernelForKernel           = " for kernel ";

static const std::string kOpenclKernelQualifierToken1         = "__kernel";
static const std::string kOpenclKernelQualifierToken2         = "kernel";
static const std::string kOpenclAttributeQualifierToken       = "__attribute__";
static const std::string kOpenclPragmaToken                   = "pragma";


static void LogPreStep(const std::string& msg, const std::string& device = "")
{
    std::cout << msg << device << "... ";
}

static void LogResult(bool result)
{
    std::cout << (result ? kStrInfoSuccess : kStrInfoFailed) << std::endl;
}

void KcCLICommanderLightningUtil::LogErrorStatus(beKA::beStatus status, const std::string& error_msg)
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

bool KcCLICommanderLightningUtil::ParseIsaFilesToCSV(bool line_numbers) const
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
                if ((status = KcCLICommanderLightningUtil::GetParsedIsaCsvText(isa, device, line_numbers, parsed_isa)) == true)
                {
                    status = (KcUtils::GetParsedISAFileName(output_files.isa_file, parsed_isa_filename) == beKA::kBeStatusSuccess);
                    if (status)
                    {
                        status = (KcCLICommanderLightningUtil::WriteIsaToFile(parsed_isa_filename, parsed_isa, log_callback_) == beKA::kBeStatusSuccess);
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

bool KcCLICommanderLightningUtil::PerformLiveVgprAnalysis(const Config& config) const
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
                beWaveSize kernel_wave_size = (RgaSharedUtils::IsNaviTarget(device) ? beWaveSize::kWave64 : beWaveSize::kUnknown);

                // Perform live VGPR analysis and force wave 64 for OpenCL kernels. Currently the wave size information
                // is missing from LLVM disassembly, therefore Shae is not able to deduce the value from the disassembly.
                // Therefore we will use a default of wave64 (this would be ignored by Shae for pre-RDNA targets).
                KcUtils::PerformLiveRegisterAnalysis(
                    isa_filename, device_gtstr, livereg_out_filename, log_callback_, config.print_process_cmd_line, false, kernel_wave_size);
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

bool KcCLICommanderLightningUtil::PerformLiveSgprAnalysis(const Config& config) const
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
                KcUtils::ConstructOutputFileName(config.sgpr_livereg_analysis_file,
                                                 kStrDefaultExtensionLiveregSgpr,
                                                 kStrDefaultExtensionText, 
                                                 entry_name,
                                                 device,
                                                 livereg_out_filename);
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
                beWaveSize kernel_wave_size = (RgaSharedUtils::IsNaviTarget(device) ? beWaveSize::kWave64 : beWaveSize::kUnknown);

                // Perform live SGPR analysis and force wave 64 for OpenCL kernels. Currently the wave size information
                // is missing from LLVM disassembly, therefore Shae is not able to deduce the value from the disassembly.
                // Therefore we will use a default of wave64 (this would be ignored by Shae for pre-RDNA targets).
                KcUtils::PerformLiveRegisterAnalysis(
                    isa_filename, device_gtstr, livereg_out_filename, log_callback_, config.print_process_cmd_line, true, kernel_wave_size);
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

bool KcCLICommanderLightningUtil::ExtractCFG(const Config& config) const
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
            gtString           cfg_out_filename = L"";
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

beKA::beStatus KcCLICommanderLightningUtil::ExtractMetadata(const CmpilerPaths& compiler_paths, const std::string& metadata_filename) const
{
    beKA::beStatus current_status = beKA::beStatus::kBeStatusSuccess;
    beKA::beStatus status         = beKA::beStatus::kBeStatusSuccess;
    std::string    metadata_text;
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
                    current_status = BeProgramBuilderLightning::ExtractMetadata(compiler_paths.bin, bin_filename, should_print_cmd_, metadata_text);
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
static beKA::beStatus GetIsaSize(const std::string& isaFileName, KernelCodeProperties& kernelCodeProps)
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
static bool BuildAnalysisData(const KernelCodeProperties& kernel_code_props, const std::string& device, beKA::AnalysisData& stats)
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

        stats_text << KcUtils::GetStatisticsCsvHeaderString(separator) << std::endl;
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
        stats_text << std::endl;

        ret = KcUtils::WriteTextFile(stats_filename.asASCIICharArray(), stats_text.str(), nullptr);
    }

    return ret;
}

beKA::beStatus KcCLICommanderLightningUtil::ExtractStatistics(const Config& config) const
{
    std::string device = "", statFileName = config.analysis_file, outStatFileName;
    beKA::beStatus status = beKA::beStatus::kBeStatusSuccess;

    LogPreStep(kStrInfoExtractingStats);

    for (auto& outputMDItem : output_metadata_)
    {
        beKA::AnalysisData stats_data;
        CodePropsMap       code_props;
        const std::string& current_device = outputMDItem.first.first;
        if (device != current_device && outputMDItem.second.status)
        {
            status = BeProgramBuilderLightning::ExtractKernelCodeProps(
                config.compiler_bin_path, outputMDItem.second.bin_file, config.print_process_cmd_line, code_props);
            if (status != beKA::beStatus::kBeStatusSuccess)
            {
                break;
            }
            for (auto& kernel_code_props : code_props)
            {
                if (config.function.empty() || config.function == kernel_code_props.first)
                {
                    auto out_files = output_metadata_.find({current_device, kernel_code_props.first});
                    if (out_files != output_metadata_.end())
                    {
                        std::string        entry_name{ kernel_code_props.first };
                        const std::string& entry_abbrivation = out_files->second.entry_abbreviation;
                        if (!entry_abbrivation.empty())
                        {
                            entry_name = entry_abbrivation;
                        }

                        std::string current_isa_file;
                        KcUtils::ConstructOutputFileName(config.isa_file, "", kStrDefaultExtensionIsa, entry_name, current_device, current_isa_file);
                        if (GetIsaSize(current_isa_file, kernel_code_props.second) && BuildAnalysisData(kernel_code_props.second, current_device, stats_data))
                        {
                            status = StoreStatistics(config, statFileName, current_device, entry_name, stats_data, outStatFileName)
                                         ? status
                                         : beKA::beStatus::kBeStatusWriteToFileFailed;
                            if (status == beKA::beStatus::kBeStatusSuccess)
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

    LogResult(status == beKA::beStatus::kBeStatusSuccess);

    return status;
}

bool KcCLICommanderLightningUtil::GetParsedIsaCsvText(const std::string& isaText, const std::string& device, bool add_line_numbers, std::string& csv_text)
{
    static const char* kStrCsvParsedIsaHeader            = "Address, Opcode, Operands, Functional Unit, Cycles, Binary Encoding\n";
    static const char* kStrCsvParsedIsaHeaderLineNumbers = "Address, Source Line Number, Opcode, Operands, Functional Unit, Cycles, Binary Encoding\n";

    bool        ret = false;
    std::string parsed_isa;
    if (BeProgramBuilder::ParseIsaToCsv(isaText, device, parsed_isa, add_line_numbers, true) == beKA::kBeStatusSuccess)
    {
        csv_text = (add_line_numbers ? kStrCsvParsedIsaHeaderLineNumbers : kStrCsvParsedIsaHeader) + parsed_isa;
        ret      = true;
    }
    return ret;
}

beKA::beStatus KcCLICommanderLightningUtil::WriteIsaToFile(const std::string&      file_name,
                                                           const std::string&      isa_text,
                                                           LoggingCallbackFunction log_callback)
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

bool KcCLICommanderLightningUtil::ExtractEntries(const std::string&  file_name,
                                                 const Config&       config,
                                                 const CmpilerPaths& compiler_paths,
                                                 RgEntryData&        entry_data)
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

bool KcCLICommanderLightningUtil::GenerateSessionMetadata(const Config& config, const CmpilerPaths& compiler_paths) const
{
    RgFileEntryData file_kernel_data;
    bool            ret = !config.session_metadata_file.empty();
    assert(ret);

    if (ret)
    {
        for (const std::string& input_file : config.input_files)
        {
            RgEntryData entry_data;
            ret = ret && ExtractEntries(input_file, config, compiler_paths, entry_data);
            if (ret)
            {
                file_kernel_data[input_file] = entry_data;
            }
        }
    }

    if (ret && !output_metadata_.empty())
    {
        ret = KcXmlWriter::GenerateClSessionMetadataFile(config.session_metadata_file, binary_codeobj_file_, file_kernel_data, output_metadata_);
    }

    return ret;
}

void KcCLICommanderLightningUtil::DeleteTempFiles() const
{
    for (const auto& out_file_data : output_metadata_)
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

bool KcCLICommanderLightningUtil::RunPostCompileSteps(const Config& config, const CmpilerPaths& compiler_paths)
{
    bool ret = false;

    if (!config.session_metadata_file.empty())
    {
        ret = GenerateSessionMetadata(config, compiler_paths);
        if (!ret)
        {
            std::stringstream msg;
            msg << kStrErrorFailedToGenerateSessionMetdata << std::endl;
            log_callback_(msg.str());
        }
    }

    DeleteTempFiles();

    return ret;
}

std::string KcCLICommanderLightningUtil::PrefixWithISAHeader(const std::string& kernel_name, const std::string& kernel_isa_text)
{
    std::stringstream  kernel_isa_text_ss;
    kernel_isa_text_ss << kLcKernelIsaHeader1 << "\"" << kernel_name << "\"" << std::endl
                       << std::endl
                       << kLcKernelIsaHeader2 << "\"" << kernel_name << "\":" << std::endl
                       << std::endl
                       << kLcKernelIsaHeader3;
    kernel_isa_text_ss << kernel_isa_text;
    return kernel_isa_text_ss.str();
}

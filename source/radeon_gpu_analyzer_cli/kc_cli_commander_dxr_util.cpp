
// C++
#include <filesystem>
#include <sstream>

// External.
#include "external/amdt_os_wrappers/Include/osFilePath.h"

// Shared.
#include "common/rg_log.h"

// Backend.
#include "source/radeon_gpu_analyzer_backend/be_program_builder_lightning.h"
#include "source/radeon_gpu_analyzer_backend/be_utils.h"

// Local.
#include "source/radeon_gpu_analyzer_cli/kc_cli_commander_dxr_util.h"
#include "source/radeon_gpu_analyzer_cli/kc_cli_commander_vulkan_util.h"
#include "source/radeon_gpu_analyzer_cli/kc_cli_string_constants.h"
#include "source/radeon_gpu_analyzer_cli/kc_utils.h"
#include "source/radeon_gpu_analyzer_cli/kc_xml_writer.h"
#include "source/radeon_gpu_analyzer_cli/kc_statistics_device_props.h"


static const char* kStrKernelName                                 = "Kernel name: ";
static const char* kStrErrorFailedToCreateOutputFilenameForKernel = "Error: failed to construct output file name for kernel: ";
static const char* kStrInfoForShaderA                             = " shader";
static const char* kStrInfoForShaderB                             = " ... ";
static const char* kStrInfoExtractRayTracingResourceUsage         = "Extracting hardware resource usage for ";

// Shader stage suffix.
static const std::string kStrCS      = "cs";
static const std::string kStrUnified = "unified";

static void LogResult(bool result)
{
    std::cout << (result ? kStrInfoSuccess : kStrInfoFailed) << std::endl;
}

// Initialize CLI session files.
std::set<std::string> KcCLICommanderDxrUtil::session_output_files_ = {};

bool KcCLICommanderDxrUtil::ParseIsaFilesToCSV(bool line_numbers) const
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
                if ((status = KcCLICommanderVulkanUtil::GetParsedIsaCsvText(isa, device, line_numbers, parsed_isa)) == true)
                {
                    status = (KcUtils::GetParsedISAFileName(output_files.isa_file, parsed_isa_filename) == beKA::kBeStatusSuccess);
                    if (status)
                    {
                        status = (KcCLICommanderVulkanUtil::WriteIsaToFile(parsed_isa_filename, parsed_isa, log_callback_) == beKA::kBeStatusSuccess);
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

bool KcCLICommanderDxrUtil::PerformLiveVgprAnalysis(const Config& config) const
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
            gtString           livereg_out_filename = L"";
            gtString           isa_filename;
            isa_filename << output_files.isa_file.c_str();
            gtString device_gtstr;
            device_gtstr << device.c_str();

            // Inform the user.
            const auto&        p              = SeparateKernelAndKernelSubtype(entry_name);
            const std::string& kernel_name    = p.first;
            const std::string& kernel_subtype = p.second;
            std::cout << kStrInfoPerformingLiveregAnalysisVgpr << kernel_subtype << kStrInfoForShaderA;
            if (kernel_name != kStrCS)
            {
                std::cout << " " << kernel_name;
            }
            std::cout << kStrInfoForShaderB;

            // Construct a name for the output livereg file.
            std::string livereg_out_filename_std_string;
            KcCLICommanderDxrUtil::ConstructOutputFileName(
                config.livereg_analysis_file, kStrDefaultExtensionLivereg, kStrDefaultExtensionText, entry_name, device, livereg_out_filename_std_string);
            livereg_out_filename << livereg_out_filename_std_string.c_str();
            if (!livereg_out_filename.isEmpty())
            {
                KcUtils::PerformLiveRegisterAnalysis(
                    isa_filename, device_gtstr, livereg_out_filename, log_callback_, config.print_process_cmd_line, false, output_files.wave_size);
                if (BeUtils::IsFilePresent(livereg_out_filename.asASCIICharArray()))
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
                error_msg << kStrErrorFailedToCreateOutputFilenameForKernel << entry_name << std::endl;
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

bool KcCLICommanderDxrUtil::PerformLiveSgprAnalysis(const Config& config) const
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
            gtString           livereg_out_filename = L"";
            gtString           isa_filename;
            isa_filename << output_files.isa_file.c_str();
            gtString device_gtstr;
            device_gtstr << device.c_str();

            // Inform the user.
            const auto&        p              = SeparateKernelAndKernelSubtype(entry_name);
            const std::string& kernel_name    = p.first;
            const std::string& kernel_subtype = p.second;
            std::cout << kStrInfoPerformingLiveregAnalysisSgpr << kernel_subtype << kStrInfoForShaderA;
            if (kernel_name != kStrCS)
            {
                std::cout << " " << kernel_name;
            }
            std::cout << kStrInfoForShaderB;

            // Construct a name for the output livereg file.
            std::string livereg_out_filename_std_string;
            KcCLICommanderDxrUtil::ConstructOutputFileName(config.sgpr_livereg_analysis_file,
                                                           kStrDefaultExtensionLiveregSgpr,
                                                           kStrDefaultExtensionText,
                                                           entry_name,
                                                           device,
                                                           livereg_out_filename_std_string);
            livereg_out_filename << livereg_out_filename_std_string.c_str();
            if (!livereg_out_filename.isEmpty())
            {
                KcUtils::PerformLiveRegisterAnalysis(
                    isa_filename, device_gtstr, livereg_out_filename, log_callback_, config.print_process_cmd_line, true, output_files.wave_size);
                if (BeUtils::IsFilePresent(livereg_out_filename.asASCIICharArray()))
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
                error_msg << kStrErrorFailedToCreateOutputFilenameForKernel << entry_name << std::endl;
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

bool KcCLICommanderDxrUtil::ExtractCFG(const Config& config) const
{
    bool              ret = true;
    std::stringstream error_msg;

    for (auto& output_md_item : output_metadata_)
    {
        RgOutputFiles& outputFiles = output_md_item.second;

        if (outputFiles.status)
        {
            const std::string& device           = output_md_item.first.first;
            const std::string& entry_name       = output_md_item.first.second;
            gtString           cfg_out_filename = L"";
            gtString           isa_filename;
            isa_filename << outputFiles.isa_file.c_str();
            gtString device_gtstr;
            device_gtstr << device.c_str();

            // Inform the user.
            bool               is_per_basic_block_cfg = !config.block_cfg_file.empty();
            const auto&        p                      = SeparateKernelAndKernelSubtype(entry_name);
            const std::string& kernel_name            = p.first;
            const std::string& kernel_subtype         = p.second;
            std::cout << (is_per_basic_block_cfg ? kStrInfoContructingPerBlockCfg1 : kStrInfoContructingPerInstructionCfg1) << kernel_subtype
                      << kStrInfoForShaderA;
            if (kernel_name != kStrCS)
            {
                std::cout << " " << kernel_name;
            }
            std::cout << kStrInfoForShaderB;

            // Construct a name for the output CFG file.
            std::string base_file = (is_per_basic_block_cfg ? config.block_cfg_file : config.inst_cfg_file);
            std::string cfg_out_filename_std_string;
            KcCLICommanderDxrUtil::ConstructOutputFileName(
                base_file, KC_STR_DEFAULT_CFG_SUFFIX, kStrDefaultExtensionDot, entry_name, device, cfg_out_filename_std_string);
            cfg_out_filename << cfg_out_filename_std_string.c_str();
            if (!cfg_out_filename.isEmpty())
            {
                KcUtils::GenerateControlFlowGraph(
                    isa_filename, device_gtstr, cfg_out_filename, log_callback_, !config.inst_cfg_file.empty(), config.print_process_cmd_line);

                if (!BeUtils::IsFilePresent(cfg_out_filename.asASCIICharArray()))
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
                error_msg << kStrErrorFailedToCreateOutputFilenameForKernel << entry_name << std::endl;
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

beKA::beStatus KcCLICommanderDxrUtil::ExtractStatistics(const Config& config, const BeAmdPalMetaData::PipelineMetaData& amdpal_pipeline_md) const
{
    std::string base_stats_filename = config.analysis_file;
    beKA::beStatus status           = beKA::beStatus::kBeStatusSuccess;
    std::string device              = "";
    
    for (auto& outputMDItem : output_metadata_)
    {
        const std::string& current_device     = outputMDItem.first.first;
        const std::string& concat_kernel_name = outputMDItem.first.second;
        if (device != current_device && outputMDItem.second.status)
        {
            const auto&        p              = SeparateKernelAndKernelSubtype(concat_kernel_name);
            const std::string& kernel_name    = p.first;
            const std::string& kernel_subtype = p.second;

            bool               success = false;
            beKA::AnalysisData stats;
            for (const auto& shader : amdpal_pipeline_md.shader_functions)
            {
                if (kernel_name == shader.name)
                {
                    stats   = shader.stats;
                    success = true;
                }
            }
            std::stringstream ss;
            ss << "." << kernel_name;
            std::string dot_kernel_name{ss.str()};
            for (const auto& shader : amdpal_pipeline_md.shaders)
            {
                if (shader.shader_subtype != BeAmdPalMetaData::ShaderSubtype::kUnknown)
                {
                    for (const auto& hs : amdpal_pipeline_md.hardware_stages)
                    {
                        if (hs.stage_type == BeAmdPalMetaData::GetStageType(dot_kernel_name))
                        {
                            stats   = hs.stats;
                            success = true;
                        }
                    }
                }
            }

            if (success)
            {
                std::cout << kStrInfoExtractRayTracingResourceUsage << kernel_subtype << kStrInfoForShaderA;
                if (kernel_name != kStrCS)
                {
                    std::cout << " " << kernel_name;
                }
                std::cout << kStrInfoForShaderB;

                std::string stats_filename;
                KcCLICommanderDxrUtil::ConstructOutputFileName(
                    base_stats_filename, kStrDefaultExtensionStats, kStrDefaultExtensionCsv, concat_kernel_name, current_device, stats_filename);
                if (!outputMDItem.second.isa_file.empty() && !stats_filename.empty())
                {
                    stats                       = KcCLICommanderVulkanUtil::PopulateAnalysisData(stats, current_device);
                    bool        isComputeBitSet = KcUtils::IsComputeBitSet(outputMDItem.second.entry_type);
                    std::size_t stage{};
                    if (BeUtils::BeAmdgpudisStageNameToBeRayTracingStage(kernel_subtype, stage))
                    {
                        BeRtxPipelineFiles isa_files;
                        BeRtxPipelineFiles stats_files;
                        std::string        stats_str = KcCLICommanderVulkanUtil::BuildStatisticsStr(stats, stage, isComputeBitSet);

                        bool is_file_written = KcUtils::WriteTextFile(stats_filename, stats_str, log_callback_);
                        if (is_file_written)
                        {
                            if (is_file_written)
                            {
                                isa_files[stage]   = outputMDItem.second.isa_file;
                                stats_files[stage] = stats_filename;
                            }
                        }
                        status = KcCLICommanderDxrUtil::ConvertStats(isa_files, stats_files, config, current_device);
                        if (status == kBeStatusSuccess)
                        {
                            outputMDItem.second.stats_file = stats_filename;
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

bool KcCLICommanderDxrUtil::GenerateSessionMetadata(const Config& config) const
{
    RgFileEntryData file_kernel_data;
    bool            ret = !config.session_metadata_file.empty();
    assert(ret);

    if (ret && !output_metadata_.empty())
    {
        ret = KcXmlWriter::GenerateClSessionMetadataFile(config.session_metadata_file, binary_codeobj_file_, file_kernel_data, output_metadata_);
    }

    return ret;
}

void KcCLICommanderDxrUtil::DeleteTempFiles() const
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

bool KcCLICommanderDxrUtil::RunPostCompileSteps(const Config& config)
{
    bool ret = false;

    if (!config.session_metadata_file.empty())
    {
        ret = GenerateSessionMetadata(config);
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

std::string KcCLICommanderDxrUtil::CombineKernelAndKernelSubtype(const std::string& kernel, const std::string& kernel_subtype)
{
    std::stringstream ss;
    ss << kernel_subtype << "_";
    if (kernel == kStrCS)
    {
        ss << kStrUnified;
    }
    else
    {
        ss << kernel;
    }
    return ss.str();
}

std::pair<std::string, std::string> KcCLICommanderDxrUtil::SeparateKernelAndKernelSubtype(const std::string& combined_name)
{
    std::pair<std::string, std::string> ret;
    size_t             offset = combined_name.find('_');
    const std::string& ext    = (offset != std::string::npos &&   offset < combined_name.size()) ? combined_name.substr(0, offset) : "";
    const std::string& kernel = (offset != std::string::npos && ++offset < combined_name.size()) ? combined_name.substr(offset)    : "";
    if (kernel == kStrUnified)
    {
        ret = {kStrCS, ext};
    }
    else
    {
        ret = {kernel, ext};
    }
    return ret;
}

beKA::beStatus KcCLICommanderDxrUtil::ConvertStats(const BeRtxPipelineFiles& isaFiles,
                                                   const BeRtxPipelineFiles& stats_files,
                                                   const Config&             config,
                                                   const std::string&        device)
{
    beKA::beStatus status = beKA::beStatus::kBeStatusSuccess;
    for (int stage = 0; stage < BeRtxPipelineStage::kCountRtx && status == beKA::beStatus::kBeStatusSuccess; stage++)
    {
        if (!stats_files[stage].empty())
        {
            status = KcCLICommanderVulkanUtil::ConvertStats(isaFiles[stage], stats_files[stage], config, device);
        }
    }
    return status;
}

void KcCLICommanderDxrUtil::ConstructOutputFileName(const std::string& base_output_filename,
                                                    const std::string& default_suffix,
                                                    const std::string& default_extension,
                                                    const std::string& entry_point_name,
                                                    const std::string& device_name,
                                                    std::string&       generated_filename)
{
    KcUtils::ConstructOutputFileName(base_output_filename, default_suffix, default_extension, entry_point_name, device_name, generated_filename);
    auto found = session_output_files_.find(generated_filename);
    if (found == session_output_files_.end())
    {
        session_output_files_.insert(generated_filename);
    }
    else
    {
        unsigned int counter = 1;
        do
        {
            std::filesystem::path generated_filepath(generated_filename);
            std::filesystem::path directory       = generated_filepath.parent_path();
            std::string           filename_no_ext = generated_filepath.stem().string();
            std::string           extension       = generated_filepath.extension().string();
            generated_filepath                    = directory / (filename_no_ext + '_' + std::to_string(counter) + extension);
            generated_filename                    = generated_filepath.string();
            counter++;
            found = session_output_files_.find(generated_filename);
        } while (found != session_output_files_.end());
        session_output_files_.insert(generated_filename);
    }
}

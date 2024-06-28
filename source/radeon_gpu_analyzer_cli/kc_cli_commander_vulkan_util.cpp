//======================================================================
// Copyright 2023 Advanced Micro Devices, Inc. All rights reserved.
//======================================================================

// C++.
#include <sstream>

// External.
#include "external/amdt_os_wrappers/Include/osFilePath.h"

// Backend.
#include "radeon_gpu_analyzer_backend/emulator/parser/be_isa_parser.h"
#include "radeon_gpu_analyzer_backend/be_program_builder_vulkan.h"
#include "radeon_gpu_analyzer_backend/be_utils.h"

// Local.
#include "source/radeon_gpu_analyzer_cli/kc_cli_commander_vulkan_util.h"
#include "source/radeon_gpu_analyzer_cli/kc_data_types.h"
#include "source/radeon_gpu_analyzer_cli/kc_utils.h"
#include "radeon_gpu_analyzer_cli/kc_xml_writer.h"
#include "source/radeon_gpu_analyzer_cli/kc_statistics_device_props.h"

// Vulkan statistics tags.
static const std::string kStrVulkanStatsTitle                = "Statistics:";
static const std::string kStrVulkanStatsTagNumUsedSgprs      = "resourceUsage.numUsedVgprs";
static const std::string kStrVulkanStatsTagNumUsedVgprs      = "resourceUsage.numUsedSgprs";
static const std::string kStrVulkanStatsTagNumAvailableVgprs = "numAvailableVgprs";
static const std::string kStrVulkanStatsTagNumAvailableSgprs = "numAvailableSgprs";
static const std::string kStrVulkanStatsTagLdsSize           = "resourceUsage.ldsSizePerLocalWorkGroup";
static const std::string kStrVulkanStatsTagLdsUsage          = "resourceUsage.ldsUsageSizeInBytes";
static const std::string kStrVulkanStatsTagScratchMem        = "resourceUsage.scratchMemUsageInBytes";

bool KcCLICommanderVulkanUtil::ParseIsaFilesToCSV(bool line_numbers, const std::string& device_string, RgVkOutputMetadata& metadata) const
{
    bool ret = true;

    // Step through existing output items to determine which files to generate CSV ISA for.
    for (auto& output_file : metadata)
    {
        if (!output_file.input_file.empty())
        {
            std::string isa, parsed_isa, parsed_isa_filename;
            bool        status = KcUtils::ReadTextFile(output_file.isa_file, isa, nullptr);

            if (status)
            {
                // Convert the ISA text to CSV format.
                if ((status = KcCLICommanderVulkanUtil::GetParsedIsaCsvText(isa, device_string, line_numbers, parsed_isa)) == true)
                {
                    status = (KcUtils::GetParsedISAFileName(output_file.isa_file, parsed_isa_filename) == beKA::kBeStatusSuccess);
                    if (status)
                    {
                        // Attempt to write the ISA CSV to disk.
                        status = (KcCLICommanderVulkanUtil::WriteIsaToFile(parsed_isa_filename, parsed_isa, log_callback_) == beKA::kBeStatusSuccess);
                        if (status)
                        {
                            // Update the session metadata output to include the path to the ISA CSV.
                            output_file.isa_csv_file = parsed_isa_filename;
                        }
                    }
                }

                if (!status)
                {
                    RgLog::stdOut << kStrErrorFailedToConvertToCsvFormat << output_file.isa_file << std::endl;
                }
            }
            ret &= status;
        }
    }

    return ret;
}

bool KcCLICommanderVulkanUtil::PerformLiveVgprAnalysis(const Config& conf, const std::string& device, RgVkOutputMetadata& device_md) const
{
    bool ret = true;

    const std::string& device_suffix = (conf.asics.empty() && !physical_adapter_name_.empty() ? "" : device);
    gtString           device_gtstr;
    device_gtstr << device.c_str();

    std::cout << kStrInfoPerformingLiveregAnalysisVgpr << device << "... " << std::endl;

    std::size_t stage = 0;
    for (auto& stage_md : device_md)
    {
        if (!stage_md.input_file.empty() && ret)
        {
            std::string out_file_name;
            gtString    out_filename_gtstr, isa_filename_gtstr;

            // Construct a name for the livereg output file.
            ret = KcUtils::ConstructOutFileName(conf.livereg_analysis_file,
                                                vulkan_stage_file_suffix_[stage],
                                                device_suffix,
                                                kStrDefaultExtensionLivereg,
                                                out_file_name,
                                                !KcUtils::IsDirectory(conf.livereg_analysis_file));

            if (ret && !out_file_name.empty())
            {
                out_filename_gtstr << out_file_name.c_str();
                isa_filename_gtstr << stage_md.isa_file.c_str();

                KcUtils::PerformLiveRegisterAnalysis(isa_filename_gtstr, device_gtstr, out_filename_gtstr, log_callback_, conf.print_process_cmd_line);
                ret                   = BeUtils::IsFilePresent(out_file_name);
                stage_md.livereg_file = out_file_name;
            }
            else
            {
                RgLog::stdOut << kStrErrorFailedCreateOutputFilename << std::endl;
            }
        }
        ++stage;
    }

    LogResult(ret);

    return ret;
}

bool KcCLICommanderVulkanUtil::PerformLiveSgprAnalysis(const Config& conf, const std::string& device, RgVkOutputMetadata& device_md) const
{
    bool ret = true;

    const std::string& device_suffix = (conf.asics.empty() && !physical_adapter_name_.empty() ? "" : device);
    gtString           device_gtstr;
    device_gtstr << device.c_str();

    std::cout << kStrInfoPerformingLiveregAnalysisSgpr << device << "... " << std::endl;

    std::size_t stage = 0;
    for (auto& stage_md : device_md)
    {
        if (!stage_md.input_file.empty() && ret)
        {
            std::string out_file_name;
            gtString    out_filename_gtstr, isa_filename_gtstr;

            // Construct a name for the livereg output file.
            ret = KcUtils::ConstructOutFileName(conf.sgpr_livereg_analysis_file,
                                                vulkan_stage_file_suffix_[stage],
                                                device_suffix,
                                                kStrDefaultExtensionLiveregSgpr,
                                                out_file_name,
                                                !KcUtils::IsDirectory(conf.sgpr_livereg_analysis_file));

            if (ret && !out_file_name.empty())
            {
                out_filename_gtstr << out_file_name.c_str();
                isa_filename_gtstr << stage_md.isa_file.c_str();

                KcUtils::PerformLiveRegisterAnalysis(isa_filename_gtstr, device_gtstr, out_filename_gtstr, log_callback_, conf.print_process_cmd_line, true);
                ret                   = BeUtils::IsFilePresent(out_file_name);
                stage_md.livereg_sgpr_file = out_file_name;
            }
            else
            {
                RgLog::stdOut << kStrErrorFailedCreateOutputFilename << std::endl;
            }
        }
        ++stage;
    }

    LogResult(ret);

    return ret;
}

bool KcCLICommanderVulkanUtil::ExtractCFG(const Config& config, const std::string& device, const RgVkOutputMetadata& device_md) const
{
    bool ret = true;

    const std::string& device_suffix = (config.asics.empty() && !physical_adapter_name_.empty() ? "" : device);
    bool               per_inst_cfg  = (!config.inst_cfg_file.empty());
    gtString           device_gtstr;
    device_gtstr << device.c_str();

    std::cout << (per_inst_cfg ? kStrInfoContructingPerInstructionCfg1 : kStrInfoContructingPerBlockCfg1) << device << "..." << std::endl;

    std::size_t stage = 0;
    for (const auto& stage_md : device_md)
    {
        if (!stage_md.input_file.empty() && ret)
        {
            std::string out_filename;
            gtString    out_filename_gtstr, isa_filename_gtstr;

            // Construct a name for the CFG output file.
            const std::string cfg_output_file = (per_inst_cfg ? config.inst_cfg_file : config.block_cfg_file);
            ret                               = KcUtils::ConstructOutFileName(cfg_output_file, 
                                                    vulkan_stage_file_suffix_[stage], 
                                                    device_suffix, 
                                                    kStrDefaultExtensionDot,
                                                    out_filename, 
                                                    !KcUtils::IsDirectory(cfg_output_file));

            if (ret && !out_filename.empty())
            {
                out_filename_gtstr << out_filename.c_str();
                isa_filename_gtstr << stage_md.isa_file.c_str();

                KcUtils::GenerateControlFlowGraph(
                    isa_filename_gtstr, device_gtstr, out_filename_gtstr, log_callback_, per_inst_cfg, config.print_process_cmd_line);
                ret = BeUtils::IsFilePresent(out_filename);
            }
            else
            {
                RgLog::stdOut << kStrErrorFailedCreateOutputFilename << std::endl;
            }
        }

        ++stage;
    }

    LogResult(ret);

    return ret;
}

void KcCLICommanderVulkanUtil::RunPostProcessingSteps(const Config& config) const
{
    // *****************************
    // Post-process for all devices.
    // *****************************
    for (auto& device_md_node : output_metadata_)
    {
        bool               is_ok         = true;
        const std::string& device_string = device_md_node.first;
        auto&              device_md     = device_md_node.second;

        // Convert ISA text to CSV if required.
        if (is_ok && config.is_parsed_isa_required)
        {
            is_ok = ParseIsaFilesToCSV(true, device_string, device_md);
        }

        // Analyze live registers (vgpr) if requested.
        if (is_ok && !config.livereg_analysis_file.empty())
        {
            is_ok = PerformLiveVgprAnalysis(config, device_string, device_md);
        }

        // Analyze live registers (sgpr) if requested.
        if (is_ok && !config.sgpr_livereg_analysis_file.empty())
        {
            is_ok = PerformLiveSgprAnalysis(config, device_string, device_md);
        }

        // Generate CFG if requested.
        if (is_ok && (!config.block_cfg_file.empty() || !config.inst_cfg_file.empty()))
        {
            is_ok = ExtractCFG(config, device_string, device_md);
        }
    }
}

bool KcCLICommanderVulkanUtil::GenerateSessionMetadata(const Config& config) const
{
    return KcXmlWriter::GenerateVulkanSessionMetadataFile(config.session_metadata_file, output_metadata_);
}

void KcCLICommanderVulkanUtil::DeleteTempFiles(std::vector<std::string>& temp_files)
{
    for (const std::string& tmp_file : temp_files)
    {
        KcUtils::DeleteFile(tmp_file);
    }
    temp_files.clear();
}

// Parse the content of Vulkan stats and store values to "data" structure.
static bool ParseVulkanStats(const std::string isa_text, const std::string& stats_text, beKA::AnalysisData& data)
{
    bool              result = false;
    std::string       line, tag, dash, equals;
    std::stringstream text_content(stats_text), sLine;
    uint64_t          value;

    // Read the statistics text line by line and parse each line.
    // Skip the 1st line which is the title.
    if ((result = std::getline(text_content, line) && line == kStrVulkanStatsTitle) == true)
    {
        while (result && std::getline(text_content, line))
        {
            sLine.clear();
            sLine << line;
            sLine >> dash >> tag >> equals >> value;
            if ((result = (dash == "-" && equals == "=")) == true)
            {
                if (tag == kStrVulkanStatsTagNumUsedSgprs)
                {
                    data.num_vgprs_used = value;
                }
                else if (tag == kStrVulkanStatsTagNumAvailableVgprs)
                {
                    data.num_vgprs_available = value;
                }
                else if (tag == kStrVulkanStatsTagNumUsedVgprs)
                {
                    data.num_sgprs_used = value;
                }
                else if (tag == kStrVulkanStatsTagNumAvailableSgprs)
                {
                    data.num_sgprs_available = value;
                }
                else if (tag == kStrVulkanStatsTagLdsSize)
                {
                    data.lds_size_available = value;
                }
                else if (tag == kStrVulkanStatsTagLdsUsage)
                {
                    data.lds_size_used = value;
                }
                else if (tag == kStrVulkanStatsTagScratchMem)
                {
                    data.scratch_memory_used = value;
                }
            }
        }
    }

    // Add the ISA size.
    assert(result);
    if (result)
    {
        ParserIsa isa_parser;
        if ((result = isa_parser.ParseForSize(isa_text)) == true)
        {
            data.isa_size = isa_parser.GetCodeLength();
        }
    }

    assert(result);
    return result;
}

beKA::beStatus 
KcCLICommanderVulkanUtil::ConvertStats(const BeVkPipelineFiles& isaFiles,
                                       const BeVkPipelineFiles& stats_files,
                                       const Config&            config,
                                       const std::string&       device)
{
    beKA::beStatus status = beKA::beStatus::kBeStatusSuccess;
    for (int stage = 0; stage < BePipelineStage::kCount && status == beKA::beStatus::kBeStatusSuccess; stage++)
    {
        if (!stats_files[stage].empty())
        {
            status = KcCLICommanderVulkanUtil::ConvertStats(isaFiles[stage], stats_files[stage], config, device);
        }
    }
    return status;
}

beKA::beStatus KcCLICommanderVulkanUtil::ConvertStats(const std::string& isa_file,
                                                      const std::string& stats_file,
                                                      const Config&      config,
                                                      const std::string& device)
{
    bool        result = false;
    std::string stats_text, isa_text;
    auto        log_func = [](const std::string& s) { RgLog::stdOut << s; };

    bool is_stats_file_read = (result = KcUtils::ReadTextFile(stats_file, stats_text, log_func));
    bool is_isa_file_read   = (result = KcUtils::ReadTextFile(isa_file, isa_text, log_func));
    if (is_stats_file_read && is_isa_file_read)
    {
        beKA::AnalysisData stats_data;
        if ((result = ParseVulkanStats(isa_text, stats_text, stats_data)) == true)
        {
            gtString filename_gtstr;
            filename_gtstr << stats_file.c_str();
            KcUtils::CreateStatisticsFile(filename_gtstr, config, device, stats_data, nullptr);
            result = (KcUtils::FileNotEmpty(stats_file));
        }
    }
    return (result ? beKA::beStatus::kBeStatusSuccess : beKA::beStatus::kBeStatusVulkanParseStatsFailed);
}

beKA::beStatus KcCLICommanderVulkanUtil::WriteIsaToFile(const std::string& file_name, const std::string& isa_text, LoggingCallbackFunction log_callback)
{
    beKA::beStatus ret = beKA::beStatus::kBeStatusInvalid;
    ret = KcUtils::WriteTextFile(file_name, isa_text, log_callback) ? beKA::beStatus::kBeStatusSuccess : beKA::beStatus::kBeStatusWriteToFileFailed;
    if (ret != beKA::beStatus::kBeStatusSuccess)
    {
        RgLog::stdErr << kStrErrorFailedToWriteIsaFile << file_name << std::endl;
    }
    return ret;
}

bool KcCLICommanderVulkanUtil::GetParsedIsaCsvText(const std::string& isaText, const std::string& device, bool add_line_numbers, std::string& csv_text)
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

static std::string GetHardwareStageDotTokenStr(const std::string& hardware_stage_suffix)
{
    std::stringstream hardwareStageDotToken;
    hardwareStageDotToken << "." << hardware_stage_suffix;
    return hardwareStageDotToken.str();
}

std::string KcCLICommanderVulkanUtil::BuildStatisticsStr(const beKA::AnalysisData& stats, std::size_t stage, bool is_compute_bit_set)
{
    std::stringstream statistics_stream;

    statistics_stream << "Statistics:" << std::endl;
    statistics_stream << "    - shaderStageMask                           = " << stage << std::endl;
    statistics_stream << "    - resourceUsage.numUsedVgprs                = " << stats.num_vgprs_used << std::endl;
    statistics_stream << "    - resourceUsage.numUsedSgprs                = " << stats.num_sgprs_used << std::endl;
    statistics_stream << "    - resourceUsage.ldsSizePerLocalWorkGroup    = " << stats.lds_size_available << std::endl;
    statistics_stream << "    - resourceUsage.ldsUsageSizeInBytes         = " << stats.lds_size_used << std::endl;
    statistics_stream << "    - resourceUsage.scratchMemUsageInBytes      = " << stats.scratch_memory_used << std::endl;
    statistics_stream << "    - numPhysicalVgprs                          = " << 1536 << std::endl;
    statistics_stream << "    - numPhysicalSgprs                          = " << 2048 << std::endl;
    statistics_stream << "    - numAvailableVgprs                         = " << stats.num_vgprs_available << std::endl;
    statistics_stream << "    - numAvailableSgprs                         = " << stats.num_sgprs_available << std::endl;

    if (is_compute_bit_set)
    {
        statistics_stream << "    - computeWorkGroupSize" << 0 << " = " << stats.num_threads_per_group_x << std::endl;
        statistics_stream << "    - computeWorkGroupSize" << 1 << " = " << stats.num_threads_per_group_y << std::endl;
        statistics_stream << "    - computeWorkGroupSize" << 2 << " = " << stats.num_threads_per_group_z << std::endl;
    }

    return statistics_stream.str();
}

beKA::AnalysisData KcCLICommanderVulkanUtil::PopulateAnalysisData(const beKA::AnalysisData& stats,
                                                                  const std::string&        current_device)
{    
    beKA::AnalysisData ret = stats;
    if (kRgaDeviceProps.count(current_device))
    {
        // Lambda returning hardcoded value if value = -1 or the value itself otherwise.
        auto               hardcoded_or = [](uint64_t val, uint64_t hc_val) { return (val == (int64_t)-1 ? hc_val : val); };
        const DeviceProps& deviceProps  = kRgaDeviceProps.at(current_device);
        ret.lds_size_available          = deviceProps.available_lds_bytes;
        ret.num_sgprs_available         = hardcoded_or(stats.num_sgprs_available, deviceProps.available_sgprs);
        ret.num_vgprs_available         = hardcoded_or(stats.num_vgprs_available, deviceProps.available_vgprs);
    }
    return ret;
}

bool WriteStatsFile(const BeAmdPalMetaData::PipelineMetaData& amdpal_pipeline_md,
                    const std::string&                        hw_stage_str,
                    uint32_t                                  stage,
                    const RgVkOutputMetadata&                 device_md,
                    LoggingCallbackFunction                   callback,
                    BeVkPipelineFiles&                        isa_files,
                    BeVkPipelineFiles&                        stats_files)
{
    bool is_file_written = false;
    if (!hw_stage_str.empty() && stage < BePipelineStage::kCount)
    {
        auto hw_stage_type = BeAmdPalMetaData::GetStageType(GetHardwareStageDotTokenStr(hw_stage_str));
        for (const auto& hardware_stage : amdpal_pipeline_md.hardware_stages)
        {
            if (hardware_stage.stage_type == hw_stage_type)
            {
                beKA::AnalysisData stats{KcCLICommanderVulkanUtil::PopulateAnalysisData(hardware_stage.stats, device_md[stage].device)};
                bool               isComputeBitSet = KcUtils::IsComputeBitSet(device_md[stage].entry_type);
                std::string        stats_str       = KcCLICommanderVulkanUtil::BuildStatisticsStr(stats, stage, isComputeBitSet);

                is_file_written = KcUtils::WriteTextFile(device_md[stage].stats_file, stats_str, callback);
                if (is_file_written)
                {
                    isa_files[stage]   = device_md[stage].isa_file;
                    stats_files[stage] = device_md[stage].stats_file;
                }
            }
        }
    }
    return is_file_written;
}

bool WriteStatsFileWithHwMapping(uint32_t                                  stage,
                                 const BeAmdPalMetaData::PipelineMetaData& amdpal_pipeline_md,
                                 const std::map<std::string, std::string>& shader_to_disassembly,
                                 const RgVkOutputMetadata&                 device_md,
                                 BeVkPipelineFiles&                        isa_files,
                                 BeVkPipelineFiles&                        stats_files,
                                 LoggingCallbackFunction                   callback)
{
    bool        ret             = false;
    const auto& dx12_stage_name = kStrDx12StageNames[stage];
    std::string hw_mapping_name;
    bool        valid_hw_mapping_found = beProgramBuilderVulkan::GetAmdgpuDisApiShaderToHwMapping(amdpal_pipeline_md, dx12_stage_name, hw_mapping_name);
    if (valid_hw_mapping_found && shader_to_disassembly.find(hw_mapping_name) != shader_to_disassembly.end())
    {
        [[maybe_unused]] bool is_file_written = WriteStatsFile(amdpal_pipeline_md, hw_mapping_name, stage, device_md, callback, isa_files, stats_files);
        assert(is_file_written);
        if (KcUtils::FileNotEmpty(stats_files[stage]))
        {
            ret = true;
        }
    }
    return ret;
}

void WriteIsaFileWithHardcodedMapping(uint32_t                                  stage,
                                      const BeAmdPalMetaData::PipelineMetaData& amdpal_pipeline_md,
                                      const std::map<std::string, std::string>& shader_to_disassembly,
                                      const RgVkOutputMetadata&                 device_md,
                                      BeVkPipelineFiles&                        isa_files,
                                      BeVkPipelineFiles&                        stats_files,
                                      LoggingCallbackFunction                   callback)
{
    // Count the number of input shaders, since this changes the pipeline type and impacts merged shaders.
    const bool is_input_hs = !device_md[BePipelineStage::kTessellationControl].stats_file.empty();

    std::string amdgpu_stage_name;
    bool        is_valid_stage = BeUtils::BePipelineStageToAmdgpudisStageName(static_cast<BePipelineStage>(stage), amdgpu_stage_name);
    assert(is_valid_stage);
    if (is_valid_stage)
    {
        if (shader_to_disassembly.find(amdgpu_stage_name) != shader_to_disassembly.end())
        {
            [[maybe_unused]] bool is_file_written = WriteStatsFile(amdpal_pipeline_md, amdgpu_stage_name, stage, device_md, callback, isa_files, stats_files);
            assert(is_file_written);
            if (!KcUtils::FileNotEmpty(stats_files[stage]))
            {
                const char* kErrCannotWriteStatsFileA = "Error: failed to write shader statistics for amdgpu stage ";
                const char* kErrCannotWriteStatsFileB = ", output file name ";
                std::cout << kErrCannotWriteStatsFileA << amdgpu_stage_name << kErrCannotWriteStatsFileB << device_md[stage].stats_file << std::endl;
            }
        }
        else
        {
            bool is_file_written = false;

            // Special case for merged shaders.
            if (stage == BePipelineStage::kVertex)
            {
                if (is_input_hs && shader_to_disassembly.find("hs") != shader_to_disassembly.end())
                {
                    // Either VS-HS-DS-GS-PS or VS-HS-DS-PS: Use "hs" disassembly instead.
                    is_file_written = WriteStatsFile(amdpal_pipeline_md, "hs", stage, device_md, callback, isa_files, stats_files);
                }
                else if (shader_to_disassembly.find("gs") != shader_to_disassembly.end())
                {
                    // VS-PS or VS-GS-PS: Use "gs" disassembly instead.
                    is_file_written = WriteStatsFile(amdpal_pipeline_md, "gs", stage, device_md, callback, isa_files, stats_files);
                }
            }
            else if (stage == BePipelineStage::kTessellationEvaluation)
            {
                if (shader_to_disassembly.find("gs") != shader_to_disassembly.end())
                {
                    // Use "gs" disassembly instead.
                    is_file_written = WriteStatsFile(amdpal_pipeline_md, "gs", stage, device_md, callback, isa_files, stats_files);
                }
            }

            assert(is_file_written);
            if (!is_file_written)
            {
                const char*       kErrorCannotDetectMergedStage = "Error: unable to detect merged stage for ";
                std::stringstream msg;
                msg << kErrorCannotDetectMergedStage << amdgpu_stage_name << " shader." << std::endl;
                std::cout << msg.str() << std::endl;
            }
        }
    }
}

void KcCLICommanderVulkanUtil::ExtractStatistics(const Config&                             config,
                                                 const std::string&                        device,
                                                 const BeAmdPalMetaData::PipelineMetaData& amdpal_pipeline_md,
                                                 const std::map<std::string, std::string>& shader_to_disassembly)
{
    std::string base_stats_filename = config.analysis_file;
    beStatus    status              = beKA::beStatus::kBeStatusSuccess;

    auto itr = output_metadata_.find(device);
    if (itr != output_metadata_.end())
    {
        // device md exists.
        const std::string& device_string = itr->first;
        auto&              device_md     = itr->second;
        BeVkPipelineFiles  isa_files, stats_files;

        // Parse amdgpu-dis output.
        if (!shader_to_disassembly.empty())
        {
            // Write the Stats files.
            for (uint32_t stage = BePipelineStage::kVertex; stage < BePipelineStage::kCount; stage++)
            {
                if (!device_md[stage].stats_file.empty())
                {
                    bool is_file_written =
                        WriteStatsFileWithHwMapping(stage, amdpal_pipeline_md, shader_to_disassembly, device_md, isa_files, stats_files, log_callback_);
                    if (!is_file_written)
                    {
                        std::string amdgpu_stage_name;
                        bool        is_valid_stage = BeUtils::BePipelineStageToAmdgpudisStageName(static_cast<BePipelineStage>(stage), amdgpu_stage_name);
                        assert(is_valid_stage);
                        if (is_valid_stage)
                        {
                            const char* kWarnCannotWriteStatsFileA = "Warning: failed to find hardware mapping for amdgpu stage ";
                            const char* kWarnCannotWriteStatsFileB = ", falling back to hardcoded mapping for shader statistics... ";
                            std::cout << kWarnCannotWriteStatsFileA << amdgpu_stage_name << kWarnCannotWriteStatsFileB << "\n";
                        }

                        WriteIsaFileWithHardcodedMapping(stage, amdpal_pipeline_md, shader_to_disassembly, device_md, isa_files, stats_files, log_callback_);
                    }
                }
            }
        }

        status = KcCLICommanderVulkanUtil::ConvertStats(isa_files, stats_files, config, device_string);
    }
}

//======================================================================
// Copyright 2023 Advanced Micro Devices, Inc. All rights reserved.
//======================================================================

// C++.
#include <sstream>

// External.
#include "external/amdt_os_wrappers/Include/osFilePath.h"

// Backend.
#include "radeon_gpu_analyzer_backend/emulator/parser/be_isa_parser.h"

// Local.
#include "source/radeon_gpu_analyzer_cli/kc_cli_commander_vulkan_util.h"
#include "source/radeon_gpu_analyzer_cli/kc_data_types.h"
#include "source/radeon_gpu_analyzer_cli/kc_utils.h"
#include "radeon_gpu_analyzer_cli/kc_xml_writer.h"
#include "radeon_gpu_analyzer_backend/be_utils.h"

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

bool KcCLICommanderVulkanUtil::PerformLiveRegAnalysis(const Config& conf, const std::string& device, RgVkOutputMetadata& device_md) const
{
    bool ret = true;

    const std::string& device_suffix = (conf.asics.empty() && !physical_adapter_name_.empty() ? "" : device);
    gtString           device_gtstr;
    device_gtstr << device.c_str();

    std::cout << kStrInfoPerformingLiveregAnalysis1 << device << "... " << std::endl;

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

        // Analyze live registers if requested.
        if (is_ok && !config.livereg_analysis_file.empty())
        {
            is_ok = PerformLiveRegAnalysis(config, device_string, device_md);
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

beStatus KcCLICommanderVulkanUtil::ConvertStats(const BeVkPipelineFiles& isaFiles,
                                                const BeVkPipelineFiles& stats_files,
                                                const Config&            config,
                                                const std::string&       device)
{
    beStatus status = kBeStatusSuccess;

    for (int stage = 0; stage < BePipelineStage::kCount && status == kBeStatusSuccess; stage++)
    {
        if (!stats_files[stage].empty())
        {
            bool        result = false;
            std::string stats_text, isa_text;
            auto        log_func = [](const std::string& s) { RgLog::stdOut << s; };

            if (((result = KcUtils::ReadTextFile(stats_files[stage], stats_text, log_func) == true) &&
                 ((result = KcUtils::ReadTextFile(isaFiles[stage], isa_text, log_func)) == true)))
            {
                beKA::AnalysisData stats_data;
                if ((result = ParseVulkanStats(isa_text, stats_text, stats_data)) == true)
                {
                    gtString filename_gtstr;
                    filename_gtstr << stats_files[stage].c_str();
                    KcUtils::CreateStatisticsFile(filename_gtstr, config, device, stats_data, nullptr);
                    result = (KcUtils::FileNotEmpty(stats_files[stage]));
                }
            }
            status = (result ? kBeStatusSuccess : kBeStatusVulkanParseStatsFailed);
        }
    }

    return status;
}

beKA::beStatus KcCLICommanderVulkanUtil::WriteIsaToFile(const std::string& file_name, const std::string& isa_text, LoggingCallbackFunction log_callback)
{
    beStatus ret = beStatus::kBeStatusInvalid;
    ret          = KcUtils::WriteTextFile(file_name, isa_text, log_callback) ? beKA::kBeStatusSuccess : beKA::kBeStatusWriteToFileFailed;
    if (ret != beKA::kBeStatusSuccess)
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

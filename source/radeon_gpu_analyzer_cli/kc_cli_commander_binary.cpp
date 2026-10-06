//=============================================================================
/// Copyright (c) 2024-2026 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for CLI Commander interface for binary code objects.
//=============================================================================

// C++.
#include <filesystem>
#include <iostream>
#include <string>

// Shared.
#include "common/rga_cli_defs.h"
#include "common/rg_log.h"

// Backend.
#include "radeon_gpu_analyzer_backend/be_program_builder_binary.h"
#include "radeon_gpu_analyzer_backend/be_program_builder_lightning.h"
#include "radeon_gpu_analyzer_backend/be_program_builder_vulkan.h"
#include "radeon_gpu_analyzer_backend/be_utils.h"

// Local.
#include "radeon_gpu_analyzer_cli/kc_cli_commander_binary.h"
#include "radeon_gpu_analyzer_cli/kc_cli_isa_spec_loader.h"
#include "radeon_gpu_analyzer_cli/kc_cli_string_constants.h"
#include "radeon_gpu_analyzer_cli/kc_utils_binary.h"
#include "radeon_gpu_analyzer_cli/kc_xml_writer.h"

const char kMultipleBinaryFolderNumberWildcardToken = '*';

std::string create_folder_with_wildcard(const std::string& path, size_t folder_number)
{
    std::string result        = path;
    std::string modified_path = path;
    size_t      pos           = modified_path.find(kMultipleBinaryFolderNumberWildcardToken);
    if (pos != std::string::npos)
    {
        modified_path.replace(pos, 1, std::to_string(folder_number));
        std::filesystem::path dir_path(modified_path);
        std::filesystem::path parent_dir = dir_path.parent_path();
        if (!std::filesystem::exists(parent_dir))
        {
            std::filesystem::create_directories(parent_dir);
        }
        std::filesystem::create_directory(dir_path);
        result = dir_path.string();
    }
    return result;
}

Config create_updated_config_for_binary(const Config& config, size_t binary_index)
{
    Config config_updated                     = config;
    config_updated.isa_file                   = create_folder_with_wildcard(config_updated.isa_file, binary_index + 1);
    config_updated.livereg_analysis_file      = create_folder_with_wildcard(config_updated.livereg_analysis_file, binary_index + 1);
    config_updated.analysis_file              = create_folder_with_wildcard(config_updated.analysis_file, binary_index + 1);
    config_updated.sgpr_livereg_analysis_file = create_folder_with_wildcard(config_updated.sgpr_livereg_analysis_file, binary_index + 1);
    config_updated.binary_text_disassembly    = create_folder_with_wildcard(config_updated.binary_text_disassembly, binary_index + 1);
    config_updated.block_cfg_file             = create_folder_with_wildcard(config_updated.block_cfg_file, binary_index + 1);
    config_updated.inst_cfg_file              = create_folder_with_wildcard(config_updated.inst_cfg_file, binary_index + 1);
    return config_updated;
}

bool KcCliCommanderBinary::ListSourcePaths(const Config& config, LoggingCallbackFunction)
{
    if (config.input_files.empty())
    {
        RgLog::stdErr << "Error: no input file provided for --list-source-paths." << std::endl;
        return false;
    }

    if (config.input_files.size() > 1)
    {
        RgLog::stdErr << "Error: --list-source-paths expects exactly one input binary file." << std::endl;
        return false;
    }

    const std::string& binary_file = config.input_files[0];
    std::string        output;
    std::string        error_text;
    beKA::beStatus     status = BeProgramBuilderLightning::ListSourcePaths(binary_file, config.print_process_cmd_line, output, error_text);

    if (status != beKA::beStatus::kBeStatusSuccess)
    {
        if (!error_text.empty())
        {
            RgLog::stdErr << error_text << std::endl;
        }
        else
        {
            RgLog::stdErr << "Error: failed to extract source paths from binary." << std::endl;
        }
        return false;
    }

    // Print the output (source paths) to stdout.
    if (!output.empty())
    {
        std::cout << output;
        if (output.back() != '\n')
        {
            std::cout << std::endl;
        }
    }

    return true;
}

void KcCliCommanderBinary::RunCompileCommands(const Config& config, LoggingCallbackFunction log_callback)
{
    const bool            verbose = config.print_process_cmd_line;
    std::set<std::string> devices;
    beKA::beStatus        status = GetSupportedTargets(devices);
    if (status == beKA::beStatus::kBeStatusSuccess)
    {
        for (size_t i = 0; i < config.input_files.size(); i++)
        {
            Config             config_updated  = create_updated_config_for_binary(config, i);
            const auto&        input_file_name = config.input_files[i];
            const std::string& bin_file_name   = input_file_name;
            auto               found           = binary_file_to_binary_analysis_map_.find(bin_file_name);
            if (found == binary_file_to_binary_analysis_map_.end())
            {
                auto [it, inserted] = binary_file_to_binary_analysis_map_.emplace(bin_file_name, KcCliBinaryAnalysis{bin_file_name, log_callback});
                if (inserted)
                {
                    std::string text_disassembly, target_device;
                    status = it->second.DisassembleCodeObject(config_updated, text_disassembly);
                if (status == beKA::beStatus::kBeStatusSuccess && it->second.GetTargetDevice(target_device))
                    {
                        std::set<std::string> matched_devices;
                        status = InitRequestedAsicBinary(config_updated, verbose, devices, bin_file_name, target_device, matched_devices);
                        if (status == beKA::beStatus::kBeStatusSuccess && matched_devices.size() == 1)
                        {
                            std::vector<std::string> asics(matched_devices.begin(), matched_devices.end());
                            KcCliIsaSpecLoader::LoadIsaSpecsFromXML(!config.session_summary_file.empty(), config.include_target_metadata, asics);

                            status = it->second.AnalyzeCodeObject(config_updated, text_disassembly);
                        }
                    }
                }
            }
        }
    }
}

bool KcCliCommanderBinary::RunPostCompileSteps(const Config& config)
{
    return GenerateSessionSummary(config) || GenerateSessionMetadataFile(config);
}

bool KcCliCommanderBinary::GenerateBinaryAnalysisVersionInfo(const std::string& filename)
{
    std::set<std::string> targets;

    // Get the list of supported GPUs for current mode.
    bool result = GetSupportedTargets(targets) == beKA::beStatus::kBeStatusSuccess;

    // Generate the Version Info header.
    result = result && KcXmlWriter::AddVersionInfoHeader(filename);

    // Add the list of supported GPUs to the Version Info file.
    result = result && KcXmlWriter::AddVersionInfoGPUList(beKA::RgaMode::kModeBinary, targets, filename);

    return result;
}

beKA::beStatus KcCliCommanderBinary::GetSupportedTargets(std::set<std::string>& targets)
{
    beStatus                     status = beKA::beStatus::kBeStatusSuccess;
    std::vector<GDT_GfxCardInfo> card_list;
    if (!BeUtils::GetAllGraphicsCards(card_list, targets))
    {
        status = beKA::beStatus::kBeStatusNoDeviceFound;
    }
    return status;
}

beKA::beStatus KcCliCommanderBinary::InitRequestedAsicBinary(const Config&                config,
                                                             bool                         verbose,
                                                             const std::set<std::string>& supported_devices,
                                                             const std::string&           binary_codeobj_file,
                                                             const std::string&           target_device,
                                                             std::set<std::string>&       matched_targets)
{
    beKA::beStatus result = beKA::beStatus::kBeStatusSuccess;

    if (verbose)
    {
        KcUtilsBinary::LogPreStep(kStrInfoDetectBinTargetDevice, binary_codeobj_file);
    }

    if (InitRequestedAsicList({target_device}, config.mode, supported_devices, matched_targets, false))
    {
        if (matched_targets.size() != 1)
        {
            result = beKA::beStatus::kBeStatusUnknownDevice;
        }
    }
    else
    {
        result = beKA::beStatus::kBeStatusNoDeviceFound;
    }

    if (verbose)
    {
        KcUtilsBinary::LogResult(result == beKA::beStatus::kBeStatusSuccess);
        KcUtilsBinary::LogErrorStatus(result, binary_codeobj_file);
    }

    return result;
}

bool KcCliCommanderBinary::GenerateSessionMetadataFile(const Config& config)
{
    bool ret = !config.session_metadata_file.empty();
    if (ret)
    {
        for (const auto& [filename, analysis] : binary_file_to_binary_analysis_map_)
        {
            ret = analysis.GenerateSessionMetadataFile(config);
            if (!ret)
            {
                RgLog::stdOut << kStrErrorFailedToGenerateSessionMetdata << std::endl;
                break;
            }
        }
    }
    return ret;
}

bool KcCliCommanderBinary::GenerateSessionSummary(const Config& config)
{
    bool ret = !config.session_summary_file.empty();
    if (ret)
    {
        RgaAnalysisSummary summary;
        for (const auto& [filename, analysis] : binary_file_to_binary_analysis_map_)
        {
            RgaAnalysisSummary::AnalysisResult result;
            ret = analysis.GeneratCompilationSummary(config, result);
            if (ret)
            {
                summary.results_.emplace_back(result);
            }
        }
        // Write summary xml file.
        ret = KcXmlWriter::WriteAnalysisSummaryToFile(summary, config.session_summary_file);

        if (!ret)
        {
            RgLog::stdOut << kStrErrorFailedToGenerateSessionSummary << std::endl;
        }
    }
    return ret;
}

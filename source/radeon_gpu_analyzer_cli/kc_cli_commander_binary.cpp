//=============================================================================
/// Copyright (c) 2024-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for CLI Commander interface for binary code objects.
//=============================================================================

// C++.
#include <filesystem>
#include <string>

// Shared.
#include "common/rga_cli_defs.h"
#include "common/rg_log.h"

// Backend.
#include "radeon_gpu_analyzer_backend/be_program_builder_binary.h"
#include "radeon_gpu_analyzer_backend/be_program_builder_vulkan.h"
#include "radeon_gpu_analyzer_backend/be_utils.h"

// Local.
#include "radeon_gpu_analyzer_cli/kc_cli_commander_binary.h"
#include "radeon_gpu_analyzer_cli/kc_cli_string_constants.h"
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
            const std::string& bin_file_name   = KcUtils::Quote(input_file_name);
            auto               found           = binary_file_to_binary_analysis_map_.find(bin_file_name);
            if (found == binary_file_to_binary_analysis_map_.end())
            {
                status = IsBinaryInputValid(config_updated, verbose, bin_file_name);
                std::string amdgpu_dis_stdout;
                std::string amdgpu_dis_stderr;
                if (status == beKA::beStatus::kBeStatusSuccess)
                {
                    status = DisassembleBinary(bin_file_name, verbose, amdgpu_dis_stdout, amdgpu_dis_stderr);
                    assert(status == beKA::beStatus::kBeStatusSuccess);
                }

                if (status == beKA::beStatus::kBeStatusSuccess)
                {
                    std::set<std::string> matched_devices;
                    status = InitRequestedAsicBinary(config_updated, verbose, devices, bin_file_name, amdgpu_dis_stdout, matched_devices);
                    if (status == beKA::beStatus::kBeStatusSuccess && matched_devices.size() == 1)
                    {
                        std::vector<std::string> asics(matched_devices.begin(), matched_devices.end());
                        binary_file_to_binary_analysis_map_[bin_file_name] = KcCliBinaryAnalysis{asics[0], bin_file_name, log_callback};
                        status = binary_file_to_binary_analysis_map_[bin_file_name].AnalyzeCodeObject(config_updated, amdgpu_dis_stdout);
                    }
                }
            }
        }
    }
}

bool KcCliCommanderBinary::RunPostCompileSteps(const Config& config)
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

beKA::beStatus KcCliCommanderBinary::IsBinaryInputValid(const Config& config, bool verbose, const std::string& binary_codeobj_file) const
{
    if (verbose)
    {
        KcCliBinaryAnalysis::LogPreStep(kStrInfoValidateBinFile, binary_codeobj_file);
    }

    beKA::beStatus ret = beKA::beStatus::kBeStatusGeneralFailed;

    // Determine if an input file is required.
    bool is_input_file_required = (!config.isa_file.empty() || !config.analysis_file.empty() || !config.livereg_analysis_file.empty() ||
                                   !config.sgpr_livereg_analysis_file.empty() || !config.block_cfg_file.empty() || !config.inst_cfg_file.empty());

    if (is_input_file_required)
    {
        if (KcUtils::FileNotEmpty(binary_codeobj_file))
        {
            ret = beKA::beStatus::kBeStatusSuccess;
        }
        else
        {
            ret = beKA::beStatus::kBeStatusBinaryInvalidInput;
        }
    }
    else
    {
        // It is valid to provide no input if none is required.
        ret = beKA::beStatus::kBeStatusSuccess;
    }

    if (verbose)
    {
        KcCliBinaryAnalysis::LogResult(ret == beKA::beStatus::kBeStatusSuccess);
    }

    KcCliBinaryAnalysis::LogErrorStatus(ret, binary_codeobj_file);

    return ret;
}

beKA::beStatus KcCliCommanderBinary::DisassembleBinary(const std::string& bin_file, bool verbose, std::string& out_text, std::string& error_txt) const
{
    KcCliBinaryAnalysis::LogPreStep(kStrInfoDisassemblingBinary, bin_file);

    beKA::beStatus ret = KcUtils::InvokeAmdgpudis(bin_file, verbose, out_text, error_txt) ? beKA::beStatus::kBeStatusSuccess
                                                                                          : beKA::beStatus::kBeStatusVulkanAmdgpudisLaunchFailed;
    if (out_text.empty())
    {
        ret = beKA::beStatus::kBeStatusVulkanAmdgpudisLaunchFailed;
    }

    KcCliBinaryAnalysis::LogResult(ret == beKA::beStatus::kBeStatusSuccess);
    KcCliBinaryAnalysis::LogErrorStatus(ret, error_txt);
    return ret;
}

bool KcCliCommanderBinary::ExtractDeviceFromAmdgpudisOutput(const std::string& amdgpu_dis_output, std::string& device)
{
    bool ret = false;
    assert(!amdgpu_dis_output.empty());
    if (!amdgpu_dis_output.empty())
    {
        const char* kAmdgpuDisDeviceTextToken = "-mcpu=";

        // Get to the -mcpu section.
        size_t curr_pos = amdgpu_dis_output.find(kAmdgpuDisDeviceTextToken);
        assert(curr_pos != std::string::npos);
        if (curr_pos != std::string::npos)
        {
            const size_t device_offset_end   = amdgpu_dis_output.find(" ", curr_pos + strlen(kAmdgpuDisDeviceTextToken));
            const size_t device_offset_begin = curr_pos + strlen(kAmdgpuDisDeviceTextToken);
            device                           = amdgpu_dis_output.substr(device_offset_begin, device_offset_end - device_offset_begin);
            ret                              = true;
        }
    }
    return ret;
}

beKA::beStatus KcCliCommanderBinary::InitRequestedAsicBinary(const Config&                config,
                                                             bool                         verbose,
                                                             const std::set<std::string>& supported_devices,
                                                             const std::string&           binary_codeobj_file,
                                                             const std::string&           amdgpu_dis_output,
                                                             std::set<std::string>&       matched_targets)
{
    beKA::beStatus result = beKA::beStatus::kBeStatusSuccess;

    if (verbose)
    {
        KcCliBinaryAnalysis::LogPreStep(kStrInfoDetectBinTargetDevice, binary_codeobj_file);
    }

    std::string device;
    bool        is_device_extracted = ExtractDeviceFromAmdgpudisOutput(amdgpu_dis_output, device);
    assert(is_device_extracted);
    assert(!device.empty());

    if (InitRequestedAsicList({device}, config.mode, supported_devices, matched_targets, false))
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
        KcCliBinaryAnalysis::LogResult(result == beKA::beStatus::kBeStatusSuccess);
        KcCliBinaryAnalysis::LogErrorStatus(result, binary_codeobj_file);
    }

    return result;
}

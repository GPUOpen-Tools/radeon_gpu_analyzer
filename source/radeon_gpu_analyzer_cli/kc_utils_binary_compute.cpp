//=============================================================================
/// Copyright (c) 2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for binary analysis compute strategy.
//=============================================================================

// C++.
#include <cassert>

// Shared.
#include "common/rga_entry_type.h"
#include "common/rg_log.h"

// Backend.
#include "radeon_gpu_analyzer_backend/be_utils.h"
#include "radeon_gpu_analyzer_backend/be_opencl_definitions.h"
#include "radeon_gpu_analyzer_backend/be_program_builder_vulkan.h"

// Local.
#include "radeon_gpu_analyzer_cli/kc_cli_string_constants.h"
#include "radeon_gpu_analyzer_cli/kc_utils.h"
#include "radeon_gpu_analyzer_cli/kc_utils_binary_compute.h"
#include "radeon_gpu_analyzer_cli/kc_utils_lightning.h"
#include "radeon_gpu_analyzer_cli/kc_xml_writer.h"

static const char* kAmdgpuDisKernelName = "amdgpu_kernel_";

beKA::beStatus ComputeBinaryWorkflowStrategy::WriteOutputFiles(const Config&                             config,
                                                               const std::string&                        asic,
                                                               const std::map<std::string, std::string>& kernel_to_disassembly,
                                                               const BeAmdPalMetaData::PipelineMetaData& ,
                                                               std::string&                              error_msg)
{
    beKA::beStatus     status   = beKA::beStatus::kBeStatusGeneralFailed;
    const std::string& isa_file = config.isa_file;
    size_t             long_kernel_name_count = 0;

    for (const auto& kernel : kernel_to_disassembly)
    {
        const auto& amdgpu_kernel_name         = kernel.first;
        const auto& shader_kernel_content      = kernel.second;
        std::string amdgpu_kernel_abbreviation = "";

        std::string amdgpu_out_file_name;
        KcUtils::ConstructOutputFileName(isa_file, "", kStrDefaultExtensionIsa, amdgpu_kernel_name, asic, amdgpu_out_file_name);
        bool isOk = !amdgpu_out_file_name.empty();
        if (isOk)
        {
            if (KcUtils::IsFileNameTooLong(amdgpu_out_file_name))
            {
                isOk = false;
                std::stringstream kss;
                kss << kAmdgpuDisKernelName << long_kernel_name_count;
                amdgpu_kernel_abbreviation = kss.str();
                KcUtils::ConstructOutputFileName(isa_file, "", kStrDefaultExtensionIsa, amdgpu_kernel_abbreviation, asic, amdgpu_out_file_name);
                long_kernel_name_count++;
                isOk = !amdgpu_out_file_name.empty();
            }
        }

        if (isOk)
        {
            KcUtils::DeleteFile(amdgpu_out_file_name);

            [[maybe_unused]] bool is_file_written = KcUtils::WriteTextFile(amdgpu_out_file_name, shader_kernel_content, nullptr);
            assert(is_file_written);
            if (!KcUtils::FileNotEmpty(amdgpu_out_file_name))
            {
                status = beKA::beStatus::kBeStatusWriteToFileFailed;
                std::stringstream error_stream;
                error_stream << amdgpu_kernel_name << ", output file name " << amdgpu_out_file_name;
                error_msg = error_stream.str();
            }
            else
            {
                // Store output metadata.
                StoreOutputFilesToOutputMD(config, asic, amdgpu_kernel_name, amdgpu_kernel_abbreviation, amdgpu_out_file_name);
                status = beKA::beStatus::kBeStatusSuccess;
            }
        }
    }
    return status;
}

void ComputeBinaryWorkflowStrategy::StoreOutputFilesToOutputMD(const Config&      config,
                                                               const std::string& asic,
                                                               const std::string& kernel_name,
                                                               const std::string& kernel_abbreviation,
                                                               const std::string& isa_filename)
{
    RgOutputFiles outFiles                = RgOutputFiles(RgaEntryType::kOpenclKernel, isa_filename, binary_codeobj_file_);
    outFiles.input_file                   = kernel_name;
    outFiles.is_isa_file_temp             = config.isa_file.empty();
    outFiles.entry_abbreviation           = kernel_abbreviation;
    outFiles.wave_size                    = beWaveSize::kWave64;
    output_metadata_[{asic, kernel_name}] = outFiles;
}

void ComputeBinaryWorkflowStrategy::RunPostProcessingSteps(const Config& config, const BeAmdPalMetaData::PipelineMetaData&)
{
    CmpilerPaths compiler_paths   = {config.compiler_bin_path, config.compiler_inc_path, config.compiler_lib_path};
    bool         should_print_cmd = config.print_process_cmd_line;

    KcUtilsLightning util(output_metadata_, should_print_cmd, log_callback_);
    beKA::beStatus   status = beKA::beStatus::kBeStatusSuccess;

    // Generate CSV files with parsed ISA if required.
    if (config.is_parsed_isa_required)
    {
        status = util.ParseIsaFilesToCSV(config.is_line_numbers_required) ? beKA::beStatus::kBeStatusSuccess : beKA::beStatus::kBeStatusParseIsaToCsvFailed;
    }

    // Extract Statistics if required.
    if (status == beKA::beStatus::kBeStatusSuccess)
    {
        util.ExtractStatistics(config);
    }

    // Block post-processing until quality of analysis engine improves when processing llvm disassembly.
    bool is_livereg_required = !config.livereg_analysis_file.empty();
    if (is_livereg_required && (status == beKA::beStatus::kBeStatusSuccess))
    {
        // Perform Live Registers analysis if required.
        util.PerformLiveVgprAnalysis(config);
    }

    bool is_sgpr_livereg_required = !config.sgpr_livereg_analysis_file.empty();
    if (is_sgpr_livereg_required && (status == beKA::beStatus::kBeStatusSuccess))
    {
        // Perform Live Registers analysis if required.
        util.PerformLiveSgprAnalysis(config);
    }

    bool is_cfg_required = (!config.block_cfg_file.empty() || !config.inst_cfg_file.empty());
    if (is_cfg_required && (status == beKA::beStatus::kBeStatusSuccess))
    {
        // Extract Control Flow Graph.
        util.ExtractCFG(config);
    }

    // Extract CodeObj metadata if required.
    if ((status == beKA::beStatus::kBeStatusSuccess) && !config.metadata_file.empty())
    {
        util.ExtractMetadata(compiler_paths, config.metadata_file);
    }
}

bool ComputeBinaryWorkflowStrategy::GenerateSessionMetadataFile(const Config& config)
{
    RgFileEntryData file_kernel_data;
    bool            ret = !config.session_metadata_file.empty();
    assert(ret);
    if (ret && !output_metadata_.empty())
    {
        ret = KcXmlWriter::GenerateClSessionMetadataFile(config.session_metadata_file, file_kernel_data, output_metadata_);
        if (!ret)
        {
            std::stringstream msg;
            msg << kStrErrorFailedToGenerateSessionMetdata << std::endl;
            log_callback_(msg.str());
        }
    }

    KcUtilsLightning::DeleteTempFiles(output_metadata_);

    return ret;
}

//=============================================================================
/// Copyright (c) 2024-2026 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementaion for class analyzing binary code objects.
//=============================================================================

// C++
#include <iostream>
#include <memory>

// External.
#include "external/amdt_os_wrappers/Include/osFilePath.h"

// Shared.
#include "common/rga_cli_defs.h"
#include "common/rg_log.h"

// Backend.
#include "radeon_gpu_analyzer_backend/be_utils.h"
#include "radeon_gpu_analyzer_backend/be_program_builder_binary.h"
#include "radeon_gpu_analyzer_backend/be_program_builder_lightning.h"
#include "radeon_gpu_analyzer_backend/be_program_builder_vulkan.h"

// Local.
#include "radeon_gpu_analyzer_cli/kc_cli_binary_analysis.h"
#include "radeon_gpu_analyzer_cli/kc_cli_string_constants.h"
#include "radeon_gpu_analyzer_cli/kc_utils_binary.h"
#include "radeon_gpu_analyzer_cli/kc_utils_binary_compute.h"
#include "radeon_gpu_analyzer_cli/kc_utils_binary_graphics.h"
#include "radeon_gpu_analyzer_cli/kc_utils_binary_raytracing.h"
#include "radeon_gpu_analyzer_cli/kc_xml_writer.h"

beKA::beStatus KcCliBinaryAnalysis::DisassembleCodeObject(const Config& config, std::string& text_disassembly)
{
    beKA::beStatus status = IsBinaryInputValid(config);
    if (status == beKA::beStatus::kBeStatusSuccess)
    {
        if (config.print_process_cmd_line)
        {
            KcUtilsBinary::LogPreStep(kStrInfoDetectBinWorkflowType, binary_codeobj_file_);
        }

        std::string metadata_text, error_msg;
        status =
            BeProgramBuilderLightning::ExtractMetadata(config.compiler_bin_path, binary_codeobj_file_, config.print_process_cmd_line, metadata_text, error_msg);
        if (!error_msg.empty())
        {
            status = kBeStatusLightningExtractMetadataFailed;
            log_callback_(error_msg);
            log_callback_("\n");
        }

        if (status == beKA::beStatus::kBeStatusSuccess)
        {
            // Code object metadata extracted from disassembly.
            BeAmdPalMetaData::PipelineMetaData amdpal_pipeline_md;
            status = BeAmdPalMetaData::ParseMetadata(metadata_text, amdpal_pipeline_md);
            switch (status)
            {
            case beKA::beStatus::kBeStatusRayTracingCodeObjMetaDataSuccess:
            {
                target_device_                           = amdpal_pipeline_md.device;
                CmpilerPaths compiler_paths              = {config.compiler_bin_path, config.compiler_inc_path, config.compiler_lib_path};
                bool         verbose                     = config.print_process_cmd_line;
                bool         is_line_correlation_enabled = config.is_line_numbers_required;
                disassembly_strategy_                    = std::make_unique<KcCliLlvmObjdumpGraphicsStrategy>(
                    binary_codeobj_file_, target_device_, compiler_paths, is_line_correlation_enabled, verbose, amdpal_pipeline_md);
                analysis_strategy_ =
                    std::make_unique<KcCliRaytracingBinaryAnalysisStrategy>(binary_codeobj_file_, std::move(amdpal_pipeline_md), log_callback_);
                status = beKA::beStatus::kBeStatusSuccess;
                break;
            }
            case beKA::beStatus::kBeStatusGraphicsCodeObjMetaDataSuccess:
            {
                auto api = beProgramBuilderBinary::GetApiFromPipelineMetadata(amdpal_pipeline_md);
                switch (api)
                {
                case beProgramBuilderBinary::ApiEnum::kOpenGL:
                    disassembly_strategy_ = std::make_unique<KcCliAmdgpuDisGraphicsStrategy>();
                    analysis_strategy_ =
                        std::make_unique<KcCliGraphicsBinaryAnalysisStrategy>(binary_codeobj_file_, api, std::move(amdpal_pipeline_md), log_callback_);
                    status = beKA::beStatus::kBeStatusSuccess;
                    break;
                case beProgramBuilderBinary::ApiEnum::kDX12:
                case beProgramBuilderBinary::ApiEnum::kVulkan:
                {
                    target_device_                           = amdpal_pipeline_md.device;
                    CmpilerPaths compiler_paths              = {config.compiler_bin_path, config.compiler_inc_path, config.compiler_lib_path};
                    bool         verbose                     = config.print_process_cmd_line;
                    bool         is_line_correlation_enabled = config.is_line_numbers_required;
                    disassembly_strategy_                    = std::make_unique<KcCliLlvmObjdumpGraphicsStrategy>(
                        binary_codeobj_file_, target_device_, compiler_paths, is_line_correlation_enabled, verbose, amdpal_pipeline_md);
                    analysis_strategy_ =
                        std::make_unique<KcCliGraphicsBinaryAnalysisStrategy>(binary_codeobj_file_, api, std::move(amdpal_pipeline_md), log_callback_);
                    status = beKA::beStatus::kBeStatusSuccess;
                    break;
                }
                default:
                    assert(false);
                    status = beKA::beStatus::kBeStatusCodeObjMdParsingFailed;
                    break;
                }
                break;
            }
            case beKA::beStatus::kBeStatusComputeCodeObjMetaDataSuccess:
            {
                BeAmdHsaMetaData::AmdHsaMetaData amdhsa_kernels_md;
                status = BeAmdHsaMetaData::ParseMetadata(metadata_text, amdhsa_kernels_md);
                if (status == beKA::beStatus::kBeStatusSuccess && amdhsa_kernels_md.kernel_names.empty())
                {
                    log_callback_(kStrErrorNoKernelEntriesInCodeObject);
                    log_callback_("\n");
                    status = beKA::beStatus::kBeStatusBinaryInvalidInput;
                }
                if (status == beKA::beStatus::kBeStatusSuccess)
                {
                    target_device_                           = amdhsa_kernels_md.device;
                    CmpilerPaths compiler_paths              = {config.compiler_bin_path, config.compiler_inc_path, config.compiler_lib_path};
                    bool         verbose                     = config.print_process_cmd_line;
                    bool         is_line_correlation_enabled = config.is_line_numbers_required;
                    disassembly_strategy_                    = std::make_unique<KcCliLlvmObjdumpComputeStrategy>(
                        binary_codeobj_file_, target_device_, compiler_paths, is_line_correlation_enabled, verbose, amdhsa_kernels_md);
                    analysis_strategy_ = std::make_unique<KcCliComputeBinaryAnalysisStrategy>(binary_codeobj_file_, log_callback_);
                    status             = beKA::beStatus::kBeStatusSuccess;
                }
                break;
            }
            default:
                status = beKA::beStatus::kBeStatusBinaryInvalidInput;
                break;
            }
        }

        if (config.print_process_cmd_line)
        {
            KcUtilsBinary::LogResult(status == beKA::beStatus::kBeStatusSuccess);
            KcUtilsBinary::LogErrorStatus(status, binary_codeobj_file_);
        }
    }

    if (status == beKA::beStatus::kBeStatusSuccess && disassembly_strategy_ != nullptr)
    {
        status = disassembly_strategy_->Disassemble(binary_codeobj_file_, config.include_path, config.substitute_paths, config.print_process_cmd_line, text_disassembly);
        if (status == beKA::beStatus::kBeStatusSuccess)
        {
            if (target_device_.empty())
            {
                std::string target_device;
                if (KcUtilsBinary::ExtractDeviceFromIsaDisassembly(text_disassembly, target_device))
                {
                    target_device_ = target_device;
                }
                else
                {
                    // Fallback: extract device from ELF flags via llvm-readobj --file-header.
                    std::string header_text, error_msg;
                    if (BeProgramBuilderLightning::ExtractFileHeader(
                            config.compiler_bin_path, binary_codeobj_file_, config.print_process_cmd_line, header_text, error_msg) ==
                        beKA::beStatus::kBeStatusSuccess)
                    {
                        KcUtilsBinary::ExtractDeviceFromFileHeader(header_text, target_device_);
                    }
                }
            }
        }

        if (!text_disassembly.empty() && !config.binary_text_disassembly.empty())
        {
            status = WriteRawTextDisassembly(config, text_disassembly);
        }
    }

    return status;
}

beKA::beStatus KcCliBinaryAnalysis::AnalyzeCodeObject(const Config& config, const std::string& text_disassembly)
{
    beKA::beStatus status = ParseDisassembly(config, text_disassembly);
    if (status == beKA::beStatus::kBeStatusSuccess)
    {
        RunPostProcessingSteps(config);
    }
    return status;
}

bool KcCliBinaryAnalysis::GetTargetDevice(std::string& device)
{
    bool ret = false;
    if (!target_device_.empty())
    {
        device = target_device_;
        ret    = true;
    }
    return ret;
}

beKA::beStatus KcCliBinaryAnalysis::IsBinaryInputValid(const Config& config) const
{
    if (config.print_process_cmd_line)
    {
        KcUtilsBinary::LogPreStep(kStrInfoValidateBinFile, binary_codeobj_file_);
    }

    beKA::beStatus ret = beKA::beStatus::kBeStatusGeneralFailed;

    // Determine if an input file is required.
    bool is_input_file_required = (!config.isa_file.empty() || !config.analysis_file.empty() || !config.livereg_analysis_file.empty() ||
                                   !config.sgpr_livereg_analysis_file.empty() || !config.block_cfg_file.empty() || !config.inst_cfg_file.empty());

    if (is_input_file_required)
    {
        if (KcUtils::FileNotEmpty(binary_codeobj_file_))
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

    if (config.print_process_cmd_line)
    {
        KcUtilsBinary::LogResult(ret == beKA::beStatus::kBeStatusSuccess);
    }

    KcUtilsBinary::LogErrorStatus(ret, binary_codeobj_file_);

    return ret;
}

beKA::beStatus KcCliBinaryAnalysis::WriteRawTextDisassembly(const Config& config, const std::string& text_disassembly)
{
    beKA::beStatus status = beKA::beStatus::kBeStatusSuccess;
    // Construct a name for the output disassembly file.
    std::string output_disassembly_filename;
    bool        is_ok = KcUtils::ConstructOutFileName(
        config.binary_text_disassembly, kStrDefaultExtensionRawDisassembly, target_device_, kStrDefaultExtensionText, output_disassembly_filename, false);
    if (is_ok && KcUtils::WriteTextFile(output_disassembly_filename, text_disassembly, nullptr))
    {
        text_disassembly_file_ = output_disassembly_filename;
    }
    else
    {
        status = beKA::beStatus::kBeStatusWriteToFileFailed;
    }
    KcUtilsBinary::LogErrorStatus(status, binary_codeobj_file_);
    return status;
}

beKA::beStatus KcCliBinaryAnalysis::ParseDisassembly(const Config& config, const std::string& text_disassembly) const
{
    KcUtilsBinary::LogPreStep(kStrInfoExtractingIsaForDevice, target_device_);

    beKA::beStatus                     status = beKA::beStatus::kBeStatusGeneralFailed;
    std::map<std::string, std::string> kernel_to_disassembly;
    std::string                        error_msg;
    auto                               is_text_disassembly_parsed = ParseKernels(text_disassembly, kernel_to_disassembly, error_msg);
    assert(is_text_disassembly_parsed == beKA::beStatus::kBeStatusSuccess);
    if (is_text_disassembly_parsed == beKA::beStatus::kBeStatusSuccess)
    {
        status = WriteOutputFiles(config, kernel_to_disassembly, error_msg);
    }
    else
    {
        status = beKA::beStatus::kBeStatusWriteParsedIsaFileFailed;
    }

    KcUtilsBinary::LogResult(status == beKA::beStatus::kBeStatusSuccess);
    KcUtilsBinary::LogErrorStatus(status, binary_codeobj_file_ + "\n" + error_msg);

    return status;
}

beKA::beStatus KcCliBinaryAnalysis::ParseKernels(const std::string&                  text_disassembly,
                                                 std::map<std::string, std::string>& shader_to_disassembly,
                                                 std::string&                        error_msg) const
{
    beKA::beStatus ret = beKA::beStatus::kBeStatusGeneralFailed;
    if (disassembly_strategy_ != nullptr)
    {
        ret = disassembly_strategy_->ParseKernels(text_disassembly, shader_to_disassembly, error_msg);
    }
    return ret;
}

beKA::beStatus KcCliBinaryAnalysis::WriteOutputFiles(const Config&                             config,
                                                     const std::map<std::string, std::string>& kernel_to_disassembly,
                                                     std::string&                              error_msg) const
{
    beKA::beStatus ret = beKA::beStatus::kBeStatusGeneralFailed;
    if (analysis_strategy_ != nullptr)
    {
        ret = analysis_strategy_->WriteOutputFiles(config, target_device_, kernel_to_disassembly, error_msg);
    }
    return ret;
}

void KcCliBinaryAnalysis::RunPostProcessingSteps(const Config& config) const
{
    if (analysis_strategy_ != nullptr)
    {
        analysis_strategy_->RunPostProcessingSteps(config);
    }
}

bool KcCliBinaryAnalysis::GenerateSessionMetadataFile(const Config& config) const
{
    bool ret = false;
    if (analysis_strategy_ != nullptr)
    {
        ret = analysis_strategy_->GenerateSessionMetadataFile(config);
    }
    return ret;
}

bool KcCliBinaryAnalysis::GeneratCompilationSummary(const Config& config, RgaAnalysisSummary::AnalysisResult& result) const
{
    bool ret = false;
    assert(analysis_strategy_ != nullptr);
    if (analysis_strategy_ != nullptr)
    {
        ret = analysis_strategy_->GeneratCompilationSummary(config, target_device_, result);
    }
    return ret;
}
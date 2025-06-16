//=============================================================================
/// Copyright (c) 2024-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementaion for class analyzing binary code objects.
//=============================================================================

// C++
#include <iostream>

// External.
#include "external/amdt_os_wrappers/Include/osFilePath.h"

// Shared.
#include "common/rga_cli_defs.h"
#include "common/rg_log.h"

// Backend.
#include "radeon_gpu_analyzer_backend/be_utils.h"
#include "radeon_gpu_analyzer_backend/be_program_builder_vulkan.h"
#include "radeon_gpu_analyzer_backend/be_program_builder_binary.h"

// Local.
#include "radeon_gpu_analyzer_cli/kc_cli_binary_analysis.h"
#include "radeon_gpu_analyzer_cli/kc_cli_string_constants.h"
#include "radeon_gpu_analyzer_cli/kc_xml_writer.h"
#include "radeon_gpu_analyzer_cli/kc_utils_binary_compute.h"
#include "radeon_gpu_analyzer_cli/kc_utils_binary_raytracing.h"
#include "radeon_gpu_analyzer_cli/kc_utils_binary_graphics.h"

beKA::beStatus KcCliBinaryAnalysis::AnalyzeCodeObject(const Config& config, const std::string& amdgpu_dis_output)
{
    const bool     verbose = config.print_process_cmd_line;
    beKA::beStatus status  = beKA::beStatus::kBeStatusSuccess;

    if (status == beKA::beStatus::kBeStatusSuccess)
    {
        status = WriteFullDisassemblyText(config, amdgpu_dis_output);
        if (status == beKA::beStatus::kBeStatusSuccess)
        {
            status = DetectAndSetWorkflowStrategy(amdgpu_dis_output, verbose);
            if (status == beKA::beStatus::kBeStatusSuccess)
            {
                status = ParseDisassembly(config, amdgpu_dis_output);
                if (status == beKA::beStatus::kBeStatusSuccess)
                {
                    RunPostProcessingSteps(config);
                }
            }
        }
    }

    return status;
}

bool KcCliBinaryAnalysis::GenerateSessionMetadataFile(const Config& config) const
{
    bool ret = false;
    assert(workflow_strategy_ != nullptr);
    if (workflow_strategy_ != nullptr)
    {
        ret = workflow_strategy_->GenerateSessionMetadataFile(config);
    }
    return ret;
}

void KcCliBinaryAnalysis::LogPreStep(const std::string& msg, const std::string& device)
{
    RgLog::stdOut << msg << device << "... " << std::flush;
}

void KcCliBinaryAnalysis::LogResult(bool result)
{
    RgLog::stdOut << (result ? kStrInfoSuccess : kStrInfoFailed) << std::endl;
}

void KcCliBinaryAnalysis::LogErrorStatus(beStatus status, const std::string& err_msg)
{
    switch (status)
    {
    case beKA::beStatus::kBeStatusSuccess:
        break;
    case beKA::beStatus::kBeStatusNoDeviceFound:
        RgLog::stdOut << kStrErrorNoDeviceFound << err_msg << std::endl;
        break;
    case beKA::beStatus::kBeStatusUnknownDevice:
        RgLog::stdOut << kStrErrorUnknownDevice << err_msg << std::endl;
        break;
    case beKA::beStatus::kBeStatusBinaryInvalidInput:
        RgLog::stdOut << kStrErrorCannotReadFile << err_msg << std::endl;
        break;
    case beKA::beStatus::kBeStatusVulkanAmdgpudisLaunchFailed:
        RgLog::stdOut << kStrErrorFailedAmdGpuDisStatus << err_msg << std::endl;
        break;
    case beKA::beStatus::kBeStatusWriteToFileFailed:
    case beKA::beStatus::kBeStatusWriteParsedIsaFileFailed:
        RgLog::stdOut << kErrCannotWriteDisassemblyFile << err_msg << std::endl;
        break;
    default:
        RgLog::stdOut << std::endl << (err_msg.empty() ? kStrErrorUnknownAmdGpuDisStatus : err_msg) << std::endl;
        break;
    }
}

beKA::beStatus KcCliBinaryAnalysis::DetectAndSetWorkflowStrategy(const std::string& amdgpu_dis_output, bool verbose)
{
    if (verbose)
    {
        KcCliBinaryAnalysis::LogPreStep(kStrInfoDetectBinWorkflowType, binary_codeobj_file_);
    }

    // TODO: use std::make_unique function - currently incompatible with gcc7.
    beKA::beStatus status = BeAmdPalMetaData::ParseAmdgpudisMetadata(amdgpu_dis_output, amdpal_pipeline_md_);
    switch (status)
    {
    case beKA::beStatus::kBeStatusRayTracingCodeObjMetaDataSuccess:
    {
        auto api                   = beProgramBuilderBinary::GetApiFromPipelineMetadata(amdpal_pipeline_md_);
        amdgpudis_parser_strategy_ = std::unique_ptr<ParseAmdgpudisOutputGraphicStrategy>(new ParseAmdgpudisOutputGraphicStrategy());
        workflow_strategy_ = std::unique_ptr<RayTracingBinaryWorkflowStrategy>(new RayTracingBinaryWorkflowStrategy(binary_codeobj_file_, api, log_callback_));
        status             = beKA::beStatus::kBeStatusSuccess;
        break;
    }
    case beKA::beStatus::kBeStatusGraphicsCodeObjMetaDataSuccess:
        amdgpudis_parser_strategy_ = std::unique_ptr<ParseAmdgpudisOutputGraphicStrategy>(new ParseAmdgpudisOutputGraphicStrategy());
        workflow_strategy_         = std::unique_ptr<GraphicsBinaryWorkflowStrategy>(new GraphicsBinaryWorkflowStrategy(binary_codeobj_file_, log_callback_));
        status                     = beKA::beStatus::kBeStatusSuccess;
        break;
    case beKA::beStatus::kBeStatusComputeCodeObjMetaDataSuccess:
        amdgpudis_parser_strategy_ = std::unique_ptr<ParseAmdgpudisOutputComputeStrategy>(new ParseAmdgpudisOutputComputeStrategy());
        workflow_strategy_         = std::unique_ptr<ComputeBinaryWorkflowStrategy>(new ComputeBinaryWorkflowStrategy(binary_codeobj_file_, log_callback_));
        status                     = beKA::beStatus::kBeStatusSuccess;
        break;
    default:
        status = beKA::beStatus::kBeStatusBinaryInvalidInput;
        break;
    }

    if (verbose)
    {
        KcCliBinaryAnalysis::LogResult(status == beKA::beStatus::kBeStatusSuccess);
        KcCliBinaryAnalysis::LogErrorStatus(status, binary_codeobj_file_);
    }
    return status;
}

beKA::beStatus KcCliBinaryAnalysis::WriteFullDisassemblyText(const Config& config,
                                                             const std::string& amdgpu_dis_output)
{
    beKA::beStatus status = beKA::beStatus::kBeStatusSuccess;
    if (!amdgpu_dis_output.empty() && !config.binary_text_disassembly.empty())
    {
        // Construct a name for the output disassembly file.
        std::string output_disassembly_filename;
        bool        is_ok = KcUtils::ConstructOutFileName(
            config.binary_text_disassembly, kStrDefaultExtensionRawDisassembly, asic_, kStrDefaultExtensionText, output_disassembly_filename, false);
        if (is_ok && KcUtils::WriteTextFile(output_disassembly_filename, amdgpu_dis_output, nullptr))
        {
            text_disassembly_file_ = output_disassembly_filename;
        }
        else
        {
            status = beKA::beStatus::kBeStatusWriteToFileFailed;
        }
        LogErrorStatus(status, binary_codeobj_file_);
    }
    return status;
}

beKA::beStatus KcCliBinaryAnalysis::ParseDisassembly(const Config& config, const std::string& amdgpu_dis_output) const
{
    LogPreStep(kStrInfoExtractingIsaForDevice, asic_);

    beKA::beStatus                     status = beKA::beStatus::kBeStatusGeneralFailed;
    std::map<std::string, std::string> kernel_to_disassembly;
    std::string                        error_msg;
    auto                               is_amdgpu_dis_output_parsed = ParseAmdgpudisKernels(amdgpu_dis_output, kernel_to_disassembly, error_msg);
    assert(is_amdgpu_dis_output_parsed == beKA::beStatus::kBeStatusSuccess);
    if (is_amdgpu_dis_output_parsed == beKA::beStatus::kBeStatusSuccess)
    {
        status = WriteOutputFiles(config, kernel_to_disassembly, error_msg);
    }
    else
    {
        status = beKA::beStatus::kBeStatusWriteParsedIsaFileFailed;
    }

    LogResult(status == beKA::beStatus::kBeStatusSuccess);
    LogErrorStatus(status, binary_codeobj_file_ + "\n" + error_msg);

    return status;
}

beKA::beStatus KcCliBinaryAnalysis::ParseAmdgpudisKernels(const std::string&                  amdgpu_dis_output,
                                                          std::map<std::string, std::string>& shader_to_disassembly,
                                                          std::string&                        error_msg) const
{
    beKA::beStatus ret = beKA::beStatus::kBeStatusGeneralFailed;
    if (amdgpudis_parser_strategy_ != nullptr)
    {
        ret = amdgpudis_parser_strategy_->ParseAmdgpudisKernels(amdgpu_dis_output, shader_to_disassembly, error_msg);
    }
    return ret;
}

beKA::beStatus KcCliBinaryAnalysis::WriteOutputFiles(const Config&                             config,
                                                     const std::map<std::string, std::string>& kernel_to_disassembly,
                                                     std::string&                              error_msg) const
{
    beKA::beStatus ret = beKA::beStatus::kBeStatusGeneralFailed;
    if (workflow_strategy_ != nullptr)
    {
        ret =  workflow_strategy_->WriteOutputFiles(config, asic_, kernel_to_disassembly, amdpal_pipeline_md_, error_msg);
    }
    return ret;
}

void KcCliBinaryAnalysis::RunPostProcessingSteps(const Config& config) const
{
    if (workflow_strategy_ != nullptr)
    {
        workflow_strategy_->RunPostProcessingSteps(config, amdpal_pipeline_md_);
    }
}

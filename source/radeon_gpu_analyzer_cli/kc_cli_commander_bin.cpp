
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
#include "radeon_gpu_analyzer_cli/kc_cli_commander_bin.h"
#include "radeon_gpu_analyzer_cli/kc_cli_string_constants.h"
#include "radeon_gpu_analyzer_cli/kc_xml_writer.h"

static void LogPreStep(const std::string& msg, const std::string& device = "")
{
    RgLog::stdOut << msg << device << "... " << std::flush;
}

static void LogResult(bool result)
{
    RgLog::stdOut << (result ? kStrInfoSuccess : kStrInfoFailed) << std::endl;
}

static void LogErrorStatus(beStatus status, const std::string& errMsg)
{
    switch (status)
    {
    case beKA::beStatus::kBeStatusSuccess:
        break;
    case beKA::beStatus::kBeStatusNoDeviceFound:
        RgLog::stdOut << kStrErrorNoDeviceFound << errMsg << std::endl;
        break;
    case beKA::beStatus::kBeStatusUnknownDevice:
        RgLog::stdOut << kStrErrorUnknownDevice << errMsg << std::endl;
        break;
    case beKA::beStatus::kBeStatusBinaryInvalidInput:
        RgLog::stdOut << kStrErrorCannotReadFile << errMsg << std::endl;
        break;
    case beKA::beStatus::kBeStatusVulkanAmdgpudisLaunchFailed:
        RgLog::stdOut << kStrErrorFailedAmdGpuDisStatus << errMsg << std::endl;
        break;
    case beKA::beStatus::kBeStatusWriteToFileFailed:
    case beKA::beStatus::kBeStatusWriteParsedIsaFileFailed:
        RgLog::stdOut << kErrCannotWriteDisassemblyFile << errMsg << std::endl;
        break;
    default:
        RgLog::stdOut << std::endl << (errMsg.empty() ? kStrErrorUnknownAmdGpuDisStatus : errMsg) << std::endl;
        break;
    }
}

void KcCliCommanderBin::RunCompileCommands(const Config& config, LoggingCallbackFunction log_callback)
{
    log_callback_         = log_callback;

    beKA::beStatus status = IsBinaryInputValid(config);

    const std::string& bin_file = config.binary_codeobj_file;
    bool               verbose  = config.print_process_cmd_line;
    std::string        amdgpu_dis_stdout;
    std::string        amdgpu_dis_stderr;
    if (status == beKA::beStatus::kBeStatusSuccess)
    {
        status = InvokeAmdgpudis(bin_file, verbose, amdgpu_dis_stdout, amdgpu_dis_stderr);
        assert(status == beKA::beStatus::kBeStatusSuccess);
    }

    if (status == beKA::beStatus::kBeStatusSuccess)
    {
        status = InitRequestedAsicBinary(config, amdgpu_dis_stdout);
    }

    if (status == beKA::beStatus::kBeStatusSuccess)
    {
        status = DetectAndSetWorkflowStrategy(config, amdgpu_dis_stdout);
    }

    if (status == beKA::beStatus::kBeStatusSuccess)
    {
        status = DisassembleBinary(config, amdgpu_dis_stdout);
    }

    if (status == beKA::beStatus::kBeStatusSuccess)
    {
        RunPostProcessingSteps(config);
    }
}

beKA::beStatus KcCliCommanderBin::IsBinaryInputValid(const Config& config) const
{
    bool verbose = config.print_process_cmd_line;
    if (verbose)
    {
        LogPreStep(kStrInfoValidateBinFile, config.binary_codeobj_file);
    }

    beKA::beStatus ret = beKA::beStatus::kBeStatusGeneralFailed;

    // Determine if an input file is required.
    bool is_input_file_required = (!config.isa_file.empty() 
                                    || !config.analysis_file.empty() 
                                    || !config.livereg_analysis_file.empty() 
                                    || !config.sgpr_livereg_analysis_file.empty() 
                                    || !config.block_cfg_file.empty() 
                                    || !config.inst_cfg_file.empty());

    if (is_input_file_required)
    {
        if (KcUtils::FileNotEmpty(config.binary_codeobj_file.c_str()))
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
        LogResult(ret == beKA::beStatus::kBeStatusSuccess);
    }

    LogErrorStatus(ret, config.binary_codeobj_file);

    return ret;
}

beKA::beStatus KcCliCommanderBin::InvokeAmdgpudis(const std::string& bin_file, bool should_print_cmd, std::string& out_text, std::string& error_txt) const
{
    std::stringstream bin_file_with_quotes;
    bin_file_with_quotes << "\"" << bin_file << "\"";
    beKA::beStatus ret = beProgramBuilderVulkan::InvokeAmdgpudis(bin_file_with_quotes.str(), should_print_cmd, out_text, error_txt);
    if (ret == beKA::beStatus::kBeStatusSuccess && out_text.empty())
    {
        ret = beKA::beStatus::kBeStatusVulkanAmdgpudisLaunchFailed;
    }
    LogErrorStatus(ret, error_txt);
    return ret;
}

beKA::beStatus KcCliCommanderBin::DetectAndSetWorkflowStrategy(const Config& config, const std::string& amdgpu_dis_output)
{
    bool verbose = config.print_process_cmd_line;
    if (verbose)
    {
        LogPreStep(kStrInfoDetectBinWorkflowType, config.binary_codeobj_file);
    }

    // TODO: use std::make_unique function - currently incompatible with gcc7.
    beKA::beStatus status = beProgramBuilderVulkan::ParseAmdgpudisMetadata(amdgpu_dis_output, amdpal_pipeline_md_);
    switch (status)
    {
    case beKA::beStatus::kBeStatusVulkanRayTracingCodeObjMetaDataSuccess:
        amdgpudis_parser_strategy_ = std::unique_ptr<ParseAmdgpudisOutputGraphicStrategy>(new ParseAmdgpudisOutputGraphicStrategy());
        workflow_strategy_         = std::unique_ptr<RayTracingBinaryWorkflowStrategy>(new RayTracingBinaryWorkflowStrategy(log_callback_));
        status                     = beKA::beStatus::kBeStatusSuccess;
        break;
    case beKA::beStatus::kBeStatusVulkanGraphicsCodeObjMetaDataSuccess:
        amdgpudis_parser_strategy_ = std::unique_ptr<ParseAmdgpudisOutputGraphicStrategy>(new ParseAmdgpudisOutputGraphicStrategy());
        workflow_strategy_         = std::unique_ptr<GraphicsBinaryWorkflowStrategy>(new GraphicsBinaryWorkflowStrategy(log_callback_));
        status                     = beKA::beStatus::kBeStatusSuccess;
        break;
    case beKA::beStatus::kBeStatusVulkanComputeCodeObjMetaDataSuccess:
        amdgpudis_parser_strategy_ = std::unique_ptr<ParseAmdgpudisOutputComputeStrategy>(new ParseAmdgpudisOutputComputeStrategy());
        workflow_strategy_         = std::unique_ptr<ComputeBinaryWorkflowStrategy>(new ComputeBinaryWorkflowStrategy(log_callback_));
        status                     = beKA::beStatus::kBeStatusSuccess;
        break;
    default:
        status = beKA::beStatus::kBeStatusBinaryInvalidInput;
        break;
    }

    if (verbose)
    {
        LogResult(status == beKA::beStatus::kBeStatusSuccess);
        LogErrorStatus(status, config.binary_codeobj_file);
    }
    return status;
}

bool KcCliCommanderBin::ExtractDeviceFromAmdgpudisOutput(const std::string& amdgpu_dis_output, std::string& device) const
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

beKA::beStatus KcCliCommanderBin::GetSupportedTargets(std::set<std::string>& targets)
{
    beStatus                     status = beKA::beStatus::kBeStatusSuccess;
    std::vector<GDT_GfxCardInfo> card_list;
    if (!BeUtils::GetAllGraphicsCards(card_list, targets))
    {
        status = beKA::beStatus::kBeStatusNoDeviceFound;
    }
    return status;
}

beKA::beStatus KcCliCommanderBin::InitRequestedAsicBinary(const Config& config, const std::string& amdgpu_dis_output)
{
    std::set<std::string> devices, matched_targets;
    beKA::beStatus        result = beKA::beStatus::kBeStatusGeneralFailed;
    result                       = GetSupportedTargets(devices);
    if (result == beKA::beStatus::kBeStatusSuccess)
    {
        bool verbose = config.print_process_cmd_line;
        if (verbose)
        {
            LogPreStep(kStrInfoDetectBinTargetDevice, config.binary_codeobj_file);
        }

        std::string device;
        [[maybe_unused]] bool is_device_extracted = ExtractDeviceFromAmdgpudisOutput(amdgpu_dis_output, device);
        assert(is_device_extracted);
        assert(!device.empty());

        asic_.clear();

        if (InitRequestedAsicList({device}, config.mode, devices, matched_targets, false))
        {
            if (matched_targets.size() == 1)
            {
                std::vector<std::string> asics(matched_targets.begin(), matched_targets.end());
                asic_ = asics[0];
            }
            else
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
            LogResult(result == beKA::beStatus::kBeStatusSuccess);
            LogErrorStatus(result, config.binary_codeobj_file);
        }
    }
    return result;
}

beKA::beStatus KcCliCommanderBin::DisassembleBinary(const Config& config, const std::string& amdgpu_dis_output) const
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
    LogErrorStatus(status, config.binary_codeobj_file + "\n" + error_msg);

    return status;
}

beKA::beStatus KcCliCommanderBin::ParseAmdgpudisKernels(const std::string&                  amdgpu_dis_output,
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

beKA::beStatus KcCliCommanderBin::WriteOutputFiles(const Config&                             config,
                                                   const std::map<std::string, std::string>& kernel_to_disassembly,
                                                   std::string&                              error_msg) const
{
    beKA::beStatus ret = beKA::beStatus::kBeStatusGeneralFailed;
    if (workflow_strategy_ != nullptr)
    {
        ret = workflow_strategy_->WriteOutputFiles(config, asic_, kernel_to_disassembly, amdpal_pipeline_md_, error_msg);
    }
    return ret;
}

void KcCliCommanderBin::RunPostProcessingSteps(const Config& config) const
{
    if (workflow_strategy_ != nullptr)
    {
        workflow_strategy_->RunPostProcessingSteps(config, amdpal_pipeline_md_);
    }
}

bool KcCliCommanderBin::RunPostCompileSteps(const Config& config)
{
    bool ret = false;
    if (workflow_strategy_ != nullptr)
    {
        ret = workflow_strategy_->RunPostCompileSteps(config);
    }
    return ret;
}

bool KcCliCommanderBin::GenerateBinaryAnalysisVersionInfo(const std::string& fileName)
{
    std::set<std::string> targets;

    // Get the list of supported GPUs for current mode.
    bool result = GetSupportedTargets(targets) == beKA::beStatus::kBeStatusSuccess;

    // Generate the Version Info header.
    result = result && KcXmlWriter::AddVersionInfoHeader(fileName);

    // Add the list of supported GPUs to the Version Info file.
    result = result && KcXmlWriter::AddVersionInfoGPUList(beKA::RgaMode::kModeBinary, targets, fileName);

    return result;
}



//======================================================================
// Copyright 2019-2023 Advanced Micro Devices, Inc. All rights reserved.
//======================================================================
#ifdef _WIN32

// C++.
#include <cassert>
#include <memory>

// Infra.
#include "external/amdt_base_tools/Include/gtString.h"
#include "external/amdt_base_tools/Include/gtList.h"
#include "external/amdt_os_wrappers/Include/osDirectory.h"
#include "external/amdt_os_wrappers/Include/osFilePath.h"

// Backend.
#include "radeon_gpu_analyzer_backend/autogen/be_utils_dx12.h"
#include "radeon_gpu_analyzer_backend/be_data_types.h"
#include "radeon_gpu_analyzer_backend/be_metadata_parser.h"
#include "radeon_gpu_analyzer_backend/be_utils.h"

// Shared.
#include "common/rga_shared_utils.h"

// Local.
#include "radeon_gpu_analyzer_cli/kc_cli_commander_bin_util.h"
#include "radeon_gpu_analyzer_cli/kc_cli_commander_dx12.h"
#include "radeon_gpu_analyzer_cli/kc_cli_string_constants.h"

// Device info.
#include "DeviceInfoUtils.h"

// *****************************************
// *** INTERNALLY LINKED SYMBOLS - START ***
// *****************************************

// Constants - error messages.
static const char* kStrErrorDx12NoTargetProvided    = "Error: no supported target device provided.";
static const char* kStrErrorDx12IsaNotGeneratedA    = "Error: failed to generate ISA disassembly for ";
static const char* kStrErrorDx12AmdilNotGeneratedA  = "Error: failed to generate AMDIL disassembly for ";
static const char* kStrErrorDx12OutputNotGeneratedB = " shader";
static const char* kStrErrorDx12StatsNotGeneratedA  = "Error: failed to generate resource usage statistics for ";
static const char* kStrErrorDx12StatsNotGeneratedB  = " shader";
static const char* kStrErrorDx12BinaryNotGeneratedA = "Error: failed to extract pipeline binary for ";

static const char* kStrErrorInvalidDxcOptionArgument =
    "Error: argument to --dxc option should be path to the folder where DXC is located, not a full path to a file.";
static const char* kStrErrorGpsoFileWriteFailed = "Error: failed to write template .gpso file to: ";

// DXR-specific error messages.
static const char* kStrErrorDxrIsaNotGeneratedB         = " export.";
static const char* kStrErrorDxrIsaNotGeneratedBPipeline = " pipeline.";
static const char* kStrErrorDxrNoSupportedTargetsFound  = "Error: non of the targets which are supported by the driver is gfx1030 or beyond. Aborting.";

// Constants - warnings messages.
static const char* kStrWarningDx12AutoDeducingRootSignatureAsHlsl = "Warning: --rs-hlsl option not provided, assuming that root signature macro is defined in ";

// DXR-specific warning messages.
static const char* kStrWarningDxrSkippingUnsupportedTarget = "Warning: DXR mode only supports gfx1030 and beyond as a target. Skipping ";

// Constants - info messages.
static const char* kStrInfoTemplateGpsoFileGenerated              = "Template .gpso file created successfully.";
static const char* kStrInfoDx12PostProcessingSeparator            = "-=-=-=-=-=-=-";
static const char* kStrInfoDx12PostProcessing                     = "Post-processing...";
static const char* kStrInfoDxrUsingDefaultShaderModel             = "Info: using user-provided shader model instead of the default model (";
static const char* kStrInfoDxrUnifiedPipelineGenerated            = "Pipeline compiled in Unified mode, expect a single uber shader in the output.";
static const char* kStrInfoDxrIndirectPipelineGenerated           = "Pipeline compiled in Indirect mode.";
static const char* kStrInfoDxrExtractedDisassemblyA               = "Extracting disassembly for pipeline associated with ";
static const char* kStrInfoDxrExtractedDisassemblyB               = " shader ";
static const char* kStrInfoDxrExtractedDisassemblyC               = "...  ";
static const char* kStrInfoDisassemblingBinaryElfContainer        = "Disassembling pipeline binary ELF container ";
static const char* kStrInfoDisassemblingBinaryElfContainerSuccess = "Pipeline binary ELF container disassembled successfully.";
static const char* kStrInfoDisassemblingBinaryElfFailure          = "failure.";
static const char* kStrInfoDisassemblingBinaryElfContainerOnlyVegaRdna =
    "Disassembling pipeline binary ELF container (--elf-dis) is only supported for binaries generated for Vega, RDNA and beyond. Skipping for ";

// Constants - other.
const char  kStrFileNmaeTokenIndirect = '*';
const char* kStrDefaultDxrShaderModel = "lib_6_3";

static const char* kAmdgpuDisShaderIdentifiersToken = "_amdgpu_shader_identifiers";

// Update the user provided configuration if necessary.
static void UpdateConfig(const Config& user_input, Config& updated_config)
{
    updated_config = user_input;
    bool is_dxr = (user_input.mode == RgaMode::kModeDxr);
    if (!is_dxr)
    {
        if (!updated_config.all_hlsl.empty())
        {
            if (!updated_config.vs_entry_point.empty() && updated_config.vs_hlsl.empty() && updated_config.vs_dxbc.empty())
            {
                updated_config.vs_hlsl = updated_config.all_hlsl;
            }
            if (!updated_config.hs_entry_point.empty() && updated_config.hs_hlsl.empty() && updated_config.hs_dxbc.empty())
            {
                updated_config.hs_hlsl = updated_config.all_hlsl;
            }
            if (!updated_config.ds_entry_point.empty() && updated_config.ds_hlsl.empty() && updated_config.ds_dxbc.empty())
            {
                updated_config.ds_hlsl = updated_config.all_hlsl;
            }
            if (!updated_config.gs_entry_point.empty() && updated_config.gs_hlsl.empty() && updated_config.gs_dxbc.empty())
            {
                updated_config.gs_hlsl = updated_config.all_hlsl;
            }
            if (!updated_config.ps_entry_point.empty() && updated_config.ps_hlsl.empty() && updated_config.ps_dxbc.empty())
            {
                updated_config.ps_hlsl = updated_config.all_hlsl;
            }
            if (!updated_config.cs_entry_point.empty() && updated_config.cs_hlsl.empty() && updated_config.cs_dxbc.empty())
            {
                updated_config.cs_hlsl = updated_config.all_hlsl;
            }
        }

        if (!user_input.rs_macro.empty() && user_input.cs_hlsl.empty() && user_input.rs_hlsl.empty() && user_input.all_hlsl.empty())
        {
            // If in a graphics pipeline --rs-macro is used without --rs-hlsl, check if all stages point to the same file.
            // If this is the case, just use that file as if it was the input to --rs-hlsl.
            std::vector<std::string> present_stages;
            if (!user_input.vs_hlsl.empty())
            {
                present_stages.push_back(user_input.vs_hlsl);
            }
            if (!user_input.hs_hlsl.empty())
            {
                present_stages.push_back(user_input.hs_hlsl);
            }
            if (!user_input.ds_hlsl.empty())
            {
                present_stages.push_back(user_input.ds_hlsl);
            }
            if (!user_input.gs_hlsl.empty())
            {
                present_stages.push_back(user_input.gs_hlsl);
            }
            if (!user_input.ps_hlsl.empty())
            {
                present_stages.push_back(user_input.ps_hlsl);
            }

            // If we have a single HLSL file for all stages - use that file for --rs-hlsl.
            if (!present_stages.empty() &&
                (present_stages.size() == 1 || std::adjacent_find(present_stages.begin(), present_stages.end(), std::not_equal_to<>()) == present_stages.end()))
            {
                updated_config.rs_hlsl = present_stages[0];
                std::cout << kStrWarningDx12AutoDeducingRootSignatureAsHlsl << updated_config.rs_hlsl << std::endl;
            }
        }
    }
    else
    {
        if (user_input.dxr_shader_model.empty())
        {
            // Use the default shader model unless specified otherwise by the user.
            updated_config.dxr_shader_model = kStrDefaultDxrShaderModel;
        }
        else
        {
            std::cout << kStrInfoDxrUsingDefaultShaderModel <<
                kStrDefaultDxrShaderModel << "): " << user_input.dxr_shader_model << std::endl;
        }
    }
}

struct RaytracingPipelineMetaData : public BeAmdPalMetaData::PipelineMetaData
{
    // Return true if it is a compute pipeline metadata.
    bool IsComputePipeline();

    // Returns true if the compute pipeline metadata is that of a Unified RayGen shader.
    bool IsUnifiedRaygenShader();

    // Return true if it is a compute pipeline metadata.
    bool IsComputeLibrary();

    // Returns true if type is a Ray Tracing Shader type.
    static bool IsRayTracingShaderType(BeAmdPalMetaData::ShaderSubtype type);
};

bool RaytracingPipelineMetaData::IsComputePipeline()
{
    return shaders.size() == 1 && shader_functions.empty() && shaders.front().shader_type == BeAmdPalMetaData::ShaderType::kCompute;
}

bool RaytracingPipelineMetaData::IsUnifiedRaygenShader()
{
    return IsComputePipeline() && shaders.front().shader_subtype == BeAmdPalMetaData::ShaderSubtype::kRayGeneration;
}

bool RaytracingPipelineMetaData::IsComputeLibrary()
{
    return shaders.empty() && shader_functions.size() == 1;
}

bool RaytracingPipelineMetaData::IsRayTracingShaderType(BeAmdPalMetaData::ShaderSubtype type)
{
    bool ret = true;
    switch (type)
    {
    case BeAmdPalMetaData::ShaderSubtype::kRayGeneration:
    case BeAmdPalMetaData::ShaderSubtype::kMiss:
    case BeAmdPalMetaData::ShaderSubtype::kAnyHit:
    case BeAmdPalMetaData::ShaderSubtype::kClosestHit:
    case BeAmdPalMetaData::ShaderSubtype::kIntersection:
    case BeAmdPalMetaData::ShaderSubtype::kCallable:
        break;
    default:
        ret = false;
    }
    return ret;
}

bool IsDxrPostProcessingRequired(const Config& config)
{
    bool is_livereg_required   = !config.livereg_analysis_file.empty();
    bool is_live_sgpr_required = !config.sgpr_livereg_analysis_file.empty();
    bool is_stats_required     = !config.analysis_file.empty();
    bool is_cfg_required       = (!config.block_cfg_file.empty() || !config.inst_cfg_file.empty());
    return is_livereg_required || is_live_sgpr_required || is_stats_required || is_cfg_required;
}

// ****************************************
// *** INTERNALLY LINKED SYMBOLS - END ***
// ****************************************

void KcCliCommanderDX12::ListAdapters(Config& config, LoggingCallbackFunction)
{
    std::vector<std::string> supported_gpus;
    std::map<std::string, int> driver_mapping;
    dx12_backend_.GetSupportGpus(config, supported_gpus, driver_mapping);
}

void KcCliCommanderDX12::RunCompileCommands(const Config& config, LoggingCallbackFunction)
{
    bool is_ok = false;
    bool should_abort = false;

    // Container for all targets.
    std::vector<std::string> target_devices;

    // Targets that have been covered.
    std::vector<std::string> completed_targets;

    // Input validation - commands.
    if (config.isa_file.empty())
    {
        if (!config.livereg_analysis_file.empty())
        {
            std::cout << kStrErrorLiveregWithoutIsa << std::endl;
            should_abort = true;
        }
        else if (!config.sgpr_livereg_analysis_file.empty())
        {
            std::cout << kStrErrorLiveregSgprWithoutIsa << std::endl;
            should_abort = true;
        }
        else if (!config.block_cfg_file.empty() ||
            !config.inst_cfg_file.empty())
        {
            std::cout << kStrErrorCfgWithoutIsa << std::endl;
            should_abort = true;
        }
    }

    if (!should_abort)
    {
        if (config.pso_dx12_template.empty())
        {
            // Update the user provided config if necessary.
            Config config_updated;
            UpdateConfig(config, config_updated);

            // Validate the --dxc option argument.
            if (!config_updated.dxc_path.empty())
            {
                if (BeUtils::IsFilePresent(config_updated.dxc_path))
                {
                    std::cout << kStrErrorInvalidDxcOptionArgument << std::endl;
                    should_abort = true;
                }
            }

            if (!should_abort)
            {
                // Validate the input.
                bool is_dxr = (config_updated.mode == RgaMode::kModeDxr);
                bool is_input_valid = dx12_backend_.ValidateAndGeneratePipeline(config_updated, is_dxr);

                bool was_asic_list_auto_generated = false;
                if (is_input_valid)
                {
                    if (config_updated.asics.empty())
                    {
                        was_asic_list_auto_generated = true;

                        std::set<std::string> device_list;
                        is_ok = GetDX12DriverAsicList(config, device_list);
                        assert(is_ok);
                        assert(!device_list.empty());
                        if (is_ok && !device_list.empty())
                        {
                            // Sort and choose the latest target.
                            std::set<std::string, decltype(&BeUtils::DeviceNameLessThan)> sort_unique_names(
                                device_list.begin(), device_list.end(), BeUtils::DeviceNameLessThan);
                            target_devices.push_back(*sort_unique_names.rbegin());
                        }
                    }
                    else
                    {
                        target_devices = config_updated.asics;
                    }

                    if (is_dxr)
                    {
                        // DXR mode only supports gfx1030 and beyond.
                        // Filter unsupported targets with an appropriate message.
                        auto target_devices_tmp = target_devices;
                        target_devices.clear();
                        for (const std::string& target : target_devices_tmp)
                        {
                            // Convert to lower case.
                            const std::string target_lower = RgaSharedUtils::ToLower(target);

                            if (KcUtils::IsNavi21AndBeyond(target_lower))
                            {
                                target_devices.push_back(target_lower);
                            }
                            else
                            {
                                std::cout << kStrWarningDxrSkippingUnsupportedTarget << target << "." << std::endl;
                            }
                        }

                        assert(!target_devices.empty());
                        if (target_devices.empty() && was_asic_list_auto_generated)
                        {
                            std::cout << kStrErrorDxrNoSupportedTargetsFound << std::endl;
                        }
                    }

                    assert(!target_devices.empty());
                    if (!target_devices.empty())
                    {
                        // DX12 graphics or compute.
                        for (const std::string& target : target_devices)
                        {
                            // Track the devices that we covered so that we do not compile twice.
                            if (std::find(completed_targets.begin(),
                                completed_targets.end(), target) == completed_targets.end())
                            {
                                // Mark as covered.
                                completed_targets.push_back(target);

                                std::string out_text;
                                std::string error_msg;
                                beStatus rc = beStatus::kBeStatusInvalid;
                                std::cout << kStrInfoCompiling << target << "..." << std::endl;

                                if (is_dxr)
                                {
                                    std::vector<RgDxrPipelineResults> output_mapping;
                                    rc = dx12_backend_.CompileDXRPipeline(config_updated, target, out_text, output_mapping, error_msg);
                                    is_ok = (rc == kBeStatusSuccess);
                                    assert(is_ok);
                                    if (!out_text.empty())
                                    {
                                        std::cout << out_text << std::endl;
                                    }
                                    if (!error_msg.empty())
                                    {
                                        std::cout << error_msg << std::endl;
                                    }

                                    bool is_success = is_ok;
                                    if (is_success)
                                    {
                                        for (const RgDxrPipelineResults& curr_pipeline_results : output_mapping)
                                        {
                                            // Verify binary files created.
                                            bool should_extract_pipeline_binaries = !curr_pipeline_results.pipeline_binary.empty();
                                            if (should_extract_pipeline_binaries)
                                            {
                                                if (!KcUtils::FileNotEmpty(curr_pipeline_results.pipeline_binary))
                                                {
                                                    std::cout << kStrErrorDx12BinaryNotGeneratedA << curr_pipeline_results.pipeline_binary
                                                              << kStrErrorDxrIsaNotGeneratedBPipeline << "\n";
                                                    is_success = false;
                                                }
                                                else
                                                {
                                                    // Generate ELF disassembly.
                                                    std::string elf_disassembly;
                                                    is_success = DisassembleElfBinary(
                                                        config_updated, target, curr_pipeline_results.pipeline_binary, elf_disassembly, error_msg);
                                                    if (is_success)
                                                    {
                                                        is_success = PostProcessElfBinary(
                                                            config_updated, target, curr_pipeline_results.pipeline_binary, elf_disassembly, error_msg);
                                                    }
                                                    else
                                                    {
                                                        std::cout << error_msg << "\n";
                                                    }
                                                }

                                                // Clean up temporary files.
                                                if (!config.should_retain_temp_files && config.binary_output_file.empty())
                                                {
                                                    KcUtils::DeleteFileW(curr_pipeline_results.pipeline_binary);
                                                }
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    BeVkPipelineFiles isa_files;
                                    BeVkPipelineFiles amdil_files;
                                    BeVkPipelineFiles stats_files;
                                    std::string binary_file;
                                    
                                    rc = dx12_backend_.CompileDX12Pipeline(
                                        config_updated, target, out_text, error_msg, isa_files, amdil_files, stats_files, binary_file);

                                    is_ok = (rc == kBeStatusSuccess);
                                    if (!out_text.empty())
                                    {
                                        std::cout << out_text << std::endl;
                                    }
                                    if (is_ok)
                                    {
                                        if (!error_msg.empty())
                                        {
                                            std::cout << error_msg << std::endl;
                                        }

                                        bool is_success = true;
                                        for (int stage = 0; stage < BePipelineStage::kCount; stage++)
                                        {
                                            if (!isa_files[stage].empty() && !KcUtils::FileNotEmpty(isa_files[stage]))
                                            {
                                                std::cout << kStrErrorDx12IsaNotGeneratedA <<
                                                    kStrDx12StageNames[stage] << kStrErrorDx12OutputNotGeneratedB << std::endl;
                                                is_success = false;
                                            }
                                            if (!amdil_files[stage].empty() && !KcUtils::FileNotEmpty(amdil_files[stage]))
                                            {
                                                std::cout << kStrErrorDx12AmdilNotGeneratedA <<
                                                    kStrDx12StageNames[stage] << kStrErrorDx12OutputNotGeneratedB << std::endl;
                                                is_success = false;
                                            }
                                            if (!stats_files[stage].empty() && !KcUtils::FileNotEmpty(stats_files[stage]))
                                            {
                                                std::cout << kStrErrorDx12StatsNotGeneratedA
                                                    << kStrDx12StageNames[stage] << kStrErrorDx12StatsNotGeneratedB << std::endl;
                                                is_success = false;
                                            }
                                        }

                                        if (is_success)
                                        {
                                            if (!binary_file.empty() && !KcUtils::FileNotEmpty(binary_file))
                                            {
                                                std::cout << kStrErrorDx12BinaryNotGeneratedA << target << std::endl;
                                                is_success = false;
                                            }
                                            else if (!config_updated.elf_dis.empty())
                                            {
                                                // Disassemble the pipeline binary.
                                                std::string elf_disassembly;
                                                is_ok = DisassembleElfBinary(config_updated, target, binary_file, elf_disassembly, error_msg, true);

                                            }
                                        }

                                        if (is_success)
                                        {
                                            std::cout << kStrInfoSuccess << std::endl;
                                            if (!config_updated.livereg_analysis_file.empty() 
                                                || !config_updated.sgpr_livereg_analysis_file.empty()
                                                || !config_updated.inst_cfg_file.empty() 
                                                || !config_updated.block_cfg_file.empty() 
                                                || !config_updated.inference_analysis_file.empty())
                                            {
                                                // Post-processing.
                                                std::cout << kStrInfoDx12PostProcessingSeparator << std::endl;
                                                std::cout << kStrInfoDx12PostProcessing << std::endl;

                                                if (!config_updated.livereg_analysis_file.empty())
                                                {
                                                    // Live register analysis files.
                                                    for (int stage = 0; stage < BePipelineStage::kCount; stage++)
                                                    {
                                                        dx12_backend_.PerformLiveVgprAnalysis(
                                                            isa_files[stage], kStrDx12StageNames[stage], target, config_updated, is_ok);
                                                    }
                                                }

                                                if (!config_updated.sgpr_livereg_analysis_file.empty())
                                                {
                                                    // Live register analysis files.
                                                    for (int stage = 0; stage < BePipelineStage::kCount; stage++)
                                                    {
                                                        dx12_backend_.PerformLiveSgprAnalysis(isa_files[stage], kStrDx12StageNames[stage], target, config_updated, is_ok);
                                                    }
                                                }

                                                if (!config_updated.block_cfg_file.empty())
                                                {
                                                    // Per-block control-flow graphs.
                                                    for (int stage = 0; stage < BePipelineStage::kCount; stage++)
                                                    {
                                                        dx12_backend_.GeneratePerBlockCfg(
                                                            isa_files[stage], "", kStrDx12StageNames[stage], target, config_updated, is_dxr, is_ok);
                                                    }
                                                }

                                                if (!config_updated.inst_cfg_file.empty())
                                                {
                                                    // Per-instruction control-flow graphs.
                                                    for (int stage = 0; stage < BePipelineStage::kCount; stage++)
                                                    {
                                                        dx12_backend_.GeneratePerInstructionCfg(
                                                            isa_files[stage], "", kStrDx12StageNames[stage], target, config_updated, is_dxr, is_ok);
                                                    }
                                                }
                                            }
                                        }
                                    }
                                    else if (!error_msg.empty())
                                    {
                                        std::cout << error_msg << std::endl;
                                    }
                                    else
                                    {
                                        std::cout << kStrInfoFailed << std::endl;
                                    }

                                    if (target_devices.size() > 1)
                                    {
                                        // In case that we are compiling for multiple targets,
                                        // print a line of space between the different devices.
                                        std::cout << std::endl;
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        std::cout << kStrErrorDx12NoTargetProvided << std::endl;
                    }
                }
            }
        }
        else
        {
            bool is_file_written = KcUtils::WriteTextFile(config.pso_dx12_template, kStrTemplateGpsoFileContent, nullptr);
            assert(is_file_written);
            if (is_file_written)
            {
                std::cout << kStrInfoTemplateGpsoFileGenerated << std::endl;
            }
            else
            {
                std::cout << kStrErrorGpsoFileWriteFailed << config.pso_dx12_template << std::endl;
            }
        }
    }
}

bool KcCliCommanderDX12::PrintAsicList(const Config& config)
{
    std::set<std::string> target_gpus;
    return GetDX12DriverAsicList(config, target_gpus, true);
}

bool KcCliCommanderDX12::GetDX12DriverAsicList(const Config& config, std::set<std::string>& target_gpus, bool print /* = false */)
{
    std::vector<std::string>   supported_gpus;
    std::vector<std::string>   supported_gpus_filtered;
    std::map<std::string, int> device_id_mapping;

    // Retrieve the list of supported targets from the DX12 backend.
    beStatus rc     = dx12_backend_.GetSupportGpus(config, supported_gpus, device_id_mapping);
    bool     result = (rc == kBeStatusSuccess);
    assert(result);

    // DXR mode only supports Navi21 and beyond: filter unsupported targets.
    if (config.mode != RgaMode::kModeDxr)
    {
        supported_gpus_filtered = supported_gpus;
    }
    else
    {
        for (const std::string& targetName : supported_gpus)
        {
            if (KcUtils::IsNavi21AndBeyond(targetName))
            {
                supported_gpus_filtered.push_back(targetName);
            }
        }
    }

    // Filter duplicates and call the shared print routine.
    target_gpus = std::set<std::string>(supported_gpus_filtered.begin(), supported_gpus_filtered.end());
    if (print)
    {
        result = result && KcUtils::PrintAsicList(target_gpus);
    }
    return result;
}

bool KcCliCommanderDX12::DisassembleElfBinary(const Config&      config,
                                              const std::string& target,
                                              const std::string& pipeline_elf,
                                              std::string&       elf_disassembly,
                                              std::string&       error_msg,
                                              bool               verbose) const
{
    if (verbose)
    {
        std::cout << kStrInfoDisassemblingBinaryElfContainer << pipeline_elf << "... "
                  << "\n";
    }

    bool ret = false;
    if (KcUtils::IsNaviTarget(target) || KcUtils::IsVegaTarget(target))
    {
        const std::string quoted_binary_path = KcUtils::Quote(pipeline_elf);
        ret                                  = KcUtils::InvokeAmdgpudis(quoted_binary_path, config.print_process_cmd_line, elf_disassembly, error_msg);
        if (ret && !elf_disassembly.empty())
        {
            if (verbose || config.print_process_cmd_line)
            {
                std::cout << kStrInfoDisassemblingBinaryElfContainerSuccess << std::endl;
            }

            if (!config.elf_dis.empty())
            {
                std::string output_filename;
                bool        is_ok = KcUtils::ConstructOutFileName(config.elf_dis, "", target, kStrDefaultExtensionText, output_filename);
                if (is_ok)
                {
                    bool isElfDisassemblySaved = KcUtils::WriteTextFile(output_filename, elf_disassembly, nullptr);
                    assert(isElfDisassemblySaved);
                }
                else
                {
                    ret = false;
                    if (verbose)
                    {
                        std::cout << kStrInfoDisassemblingBinaryElfFailure << "\n";
                    }
                }
            }
        }
        else
        {
            if (verbose || config.print_process_cmd_line)
            {
                std::cout << kStrInfoDisassemblingBinaryElfFailure << "\n";
            }
        }
    }
    else
    {
        if (verbose)
        {
            std::cout << kStrInfoDisassemblingBinaryElfContainerOnlyVegaRdna << target << "." << std::endl;
        }
    }
    return ret;
}

bool KcCliCommanderDX12::PostProcessElfBinary(const Config&           config,
                                              const std::string&      target,
                                              const std::string&      pipeline_elf,
                                              const std::string&      elf_disassembly,
                                              std::string&            error_msg)
{
    bool                               is_success = false;
    std::map<std::string, std::string> kernel_to_disassembly;
    beKA::beStatus                     status = ParseAmdgpudisOutputGraphicStrategy{}.ParseAmdgpudisKernels(elf_disassembly, kernel_to_disassembly, error_msg);
    if (status == beKA::beStatus::kBeStatusSuccess)
    {
        RayTracingBinaryWorkflowStrategy processor{pipeline_elf, log_callback_};
        RaytracingPipelineMetaData       pipeline_md;
        beKA::beStatus                   md_status = BeAmdPalMetaData::ParseAmdgpudisMetadata(elf_disassembly, pipeline_md);
        assert(md_status == beKA::beStatus::kBeStatusRayTracingCodeObjMetaDataSuccess);
        if (md_status == beKA::beStatus::kBeStatusRayTracingCodeObjMetaDataSuccess)
        {
            bool ignore_pipeline_binary = true;
            if (pipeline_md.IsComputePipeline())
            {
                if (pipeline_md.IsUnifiedRaygenShader())
                {
                    std::cout << kStrInfoDxrUnifiedPipelineGenerated << "\n";
                    std::cout << kStrInfoDxrExtractedDisassemblyA << BeAmdPalMetaData::GetShaderSubtypeName(BeAmdPalMetaData::ShaderSubtype::kRayGeneration)
                              << kStrInfoDxrExtractedDisassemblyB << kStrInfoDxrExtractedDisassemblyC;
                    ignore_pipeline_binary = false;
                }
                else
                {
                    std::cout << kStrInfoDxrIndirectPipelineGenerated << "\n";
                }
            }
            else if (pipeline_md.IsComputeLibrary())
            {
                const auto& shader_function = pipeline_md.shader_functions.front();
                if (RaytracingPipelineMetaData::IsRayTracingShaderType(shader_function.shader_subtype))
                {
                    std::cout << kStrInfoDxrExtractedDisassemblyA << BeAmdPalMetaData::GetShaderSubtypeName(shader_function.shader_subtype)
                              << kStrInfoDxrExtractedDisassemblyB << shader_function.name << kStrInfoDxrExtractedDisassemblyC;
                    ignore_pipeline_binary = false;
                }
                else
                {
                    auto found = kernel_to_disassembly.find(shader_function.name);
                    if (found != kernel_to_disassembly.end())
                    {
                        kernel_to_disassembly.erase(found);
                    }
                }
            }

            if (!ignore_pipeline_binary)
            {
                status = processor.WriteOutputFiles(config, target, kernel_to_disassembly, pipeline_md, error_msg);
                if (status == beKA::beStatus::kBeStatusSuccess)
                {
                    std::cout << kStrInfoSuccess << "\n";
                    // Post-processing.
                    if (IsDxrPostProcessingRequired(config))
                    {
                        std::cout << kStrInfoDx12PostProcessingSeparator << "\n";
                        std::cout << kStrInfoDx12PostProcessing << "\n";
                        processor.RunPostProcessingSteps(config, pipeline_md);
                    }
                    is_success = true;
                }
                else
                {
                    std::cout << kStrInfoFailed << "\n";
                }
            }
        }
    }
    else
    {
        if (elf_disassembly.find(kAmdgpuDisShaderIdentifiersToken) != std::string::npos)
        {
            // The current pipeline binary is based on NPRT enabled in 24.20 driver.
            // This is just an implementation detail, and does not concern the user,
            // but does NOT indicate a disassemlbly parsing failure.
            is_success = true;
        }
    }
    return is_success;
}
#endif

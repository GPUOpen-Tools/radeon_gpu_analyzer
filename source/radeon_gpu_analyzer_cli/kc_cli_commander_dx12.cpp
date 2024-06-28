//======================================================================
// Copyright 2019-2023 Advanced Micro Devices, Inc. All rights reserved.
//======================================================================
#ifdef _WIN32

// C++.
#include <cassert>

// Infra.
#include "external/amdt_base_tools/Include/gtString.h"
#include "external/amdt_base_tools/Include/gtList.h"
#include "external/amdt_os_wrappers/Include/osDirectory.h"
#include "external/amdt_os_wrappers/Include/osFilePath.h"

// Backend.
#include "radeon_gpu_analyzer_backend/be_utils.h"
#include "radeon_gpu_analyzer_backend/be_data_types.h"
#include "radeon_gpu_analyzer_backend/autogen/be_utils_dx12.h"

// Shared.
#include "common/rga_shared_utils.h"

// Local.
#include "radeon_gpu_analyzer_cli/kc_cli_commander_dx12.h"
#include "radeon_gpu_analyzer_cli/kc_cli_string_constants.h"

// Device info.
#include "DeviceInfoUtils.h"

using namespace rga;

// *****************************************
// *** INTERNALLY LINKED SYMBOLS - START ***
// *****************************************

// Constants - error messages.
static const char* kStrErrorDx12NoTargetProvided = "Error: no supported target device provided.";
static const char* kStrErrorDx12IsaNotGeneratedA = "Error: failed to generate ISA disassembly for ";
static const char* kStrErrorDx12AmdilNotGeneratedA = "Error: failed to generate AMDIL disassembly for ";
static const char* kStrErrorDx12OutputNotGeneratedB = " shader";
static const char* kStrErrorDx12StatsNotGeneratedA = "Error: failed to generate resource usage statistics for ";
static const char* kStrErrorDx12StatsNotGeneratedB = " shader";
static const char* kStrErrorDx12BinaryNotGeneratedA = "Error: failed to extract pipeline binary for ";

static const char* kStrErrorInvalidDxcOptionArgument = "Error: argument to --dxc option should be path to the folder where DXC is located, not a full path to a file.";
static const char* kStrErrorGpsoFileWriteFailed = "Error: failed to write template .gpso file to: ";

// DXR-specific error messages.
static const char* kStrErrorDxrIsaNotGeneratedB = " export.";
static const char* kStrErrorDxrIsaNotGeneratedBPipeline = " pipeline.";
static const char* kStrErrorDxrNoSupportedTargetsFound = "Error: non of the targets which are supported by the driver is gfx1030 or beyond. Aborting.";

// Constants - warnings messages.
static const char* kStrWarningDx12AutoDeducingRootSignatureAsHlsl = "Warning: --rs-hlsl option not provided, assuming that root signature macro is defined in ";

// DXR-specific warning messages.
static const char* kStrWarningDxrSkippingUnsupportedTarget = "Warning: DXR mode only supports gfx1030 and beyond as a target. Skipping ";

// Constants - info messages.
static const char* kStrInfoTemplateGpsoFileGenerated = "Template .gpso file created successfully.";
static const char* kStrInfoDx12PostProcessingSeparator = "-=-=-=-=-=-=-";
static const char* kStrInfoDx12PostProcessing = "Post-processing...";
static const char* kStrInfoDxrAssumingShaderMode = "Info: no --mode argument detected, assuming '--mode shader' by default.";
static const char* kStrInfoDxrOutputPipelineNumber = "pipeline #";
static const char* kStrInfoDxrOutputPipelineName = "pipeline associated with raygeneration shader ";
static const char* kStrInfoDxrUsingDefaultShaderModel = "Info: using user-provided shader model instead of the default model (";

// Constants - other.
static const char* kStrDxrUnifiedSuffix = "_unified";
const char kStrFileNmaeTokenIndirect = '*';
const char* kStrDefaultDxrShaderModel = "lib_6_3";

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
        // Use shader mode by default.
        if (user_input.dxr_mode.compare(kStrDxrModeShader) != 0 &&
            user_input.dxr_mode.compare(kStrDxrModePipeline) != 0)
        {
            std::cout << kStrInfoDxrAssumingShaderMode << std::endl;
            updated_config.dxr_mode = kStrDxrModeShader;
        }

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


static bool PerformLiveVgprAnalysisDxr(const Config& config_updated, const std::string& isa_file, const std::string& target,
    const std::string& output_filename, const std::string&, const RgDxrShaderResults& shader_results)
{
    bool is_ok = false;
    if (!isa_file.empty())
    {
        std::cout << kStrInfoPerformingLiveregAnalysisVgpr;
        std::cout << kStrInfoDxrOutputShader << shader_results.export_name << kStrInfoDxrPerformingPostProcessing << std::endl;

        // Delete the file if it already exists.
        if (BeUtils::IsFilePresent(output_filename))
        {
            KcUtils::DeleteFile(output_filename.c_str());
        }

        is_ok = KcUtils::PerformLiveRegisterAnalysis(isa_file, target, output_filename, NULL,
            config_updated.print_process_cmd_line);

        if (is_ok)
        {
            if (KcUtils::FileNotEmpty(output_filename))
            {
                std::cout << kStrInfoSuccess << std::endl;
            }
            else
            {
                std::cout << kStrInfoFailed << std::endl;
                KcUtils::DeleteFile(output_filename);
            }
        }
    }

    return is_ok;
}

static bool PerformLiveSgprAnalysisDxr(const Config&             config_updated,
                                       const std::string&        isa_file,
                                       const std::string&        target,
                                       const std::string&        output_filename,
                                       const std::string&        ,
                                       const RgDxrShaderResults& shader_results)
{
    bool is_ok = false;
    if (!isa_file.empty())
    {
        std::cout << kStrInfoPerformingLiveregAnalysisSgpr;
        std::cout << kStrInfoDxrOutputShader << shader_results.export_name << kStrInfoDxrPerformingPostProcessing << std::endl;

        // Delete the file if it already exists.
        if (BeUtils::IsFilePresent(output_filename))
        {
            KcUtils::DeleteFile(output_filename.c_str());
        }

        is_ok = KcUtils::PerformLiveRegisterAnalysis(isa_file, target, output_filename, NULL, config_updated.print_process_cmd_line, true);

        if (is_ok)
        {
            if (KcUtils::FileNotEmpty(output_filename))
            {
                std::cout << kStrInfoSuccess << std::endl;
            }
            else
            {
                std::cout << kStrInfoFailed << std::endl;
                KcUtils::DeleteFile(output_filename);
            }
        }
    }

    return is_ok;
}

static bool GeneratePerBlockCfgDxr(const Config& config_updated, const std::string& isa_file, const std::string& target,
    const std::string& output_filename, const std::string&, const RgDxrShaderResults& shader_results)
{
    bool is_ok = false;
    if (!isa_file.empty())
    {
        std::cout << kStrInfoContructingPerBlockCfg1;
        std::cout << kStrInfoDxrOutputShader << shader_results.export_name << kStrInfoDxrPerformingPostProcessing << std::endl;

        // Delete the file if it already exists.
        if (BeUtils::IsFilePresent(output_filename))
        {
            KcUtils::DeleteFile(output_filename.c_str());
        }

        is_ok = KcUtils::GenerateControlFlowGraph(isa_file, target, output_filename, NULL,
            false, config_updated.print_process_cmd_line);

        if (is_ok)
        {
            if (KcUtils::FileNotEmpty(output_filename))
            {
                std::cout << kStrInfoSuccess << std::endl;
            }
            else
            {
                std::cout << kStrInfoFailed << std::endl;
                KcUtils::DeleteFile(output_filename);
            }
        }
    }

    return is_ok;
}

static bool GeneratePerInstructionCfgDxr(const Config& config_updated, const std::string& isa_file, const std::string& target,
    const std::string& output_filename, const std::string&, const RgDxrShaderResults& shader_results)
{
    bool is_ok = false;
    if (!isa_file.empty())
    {
        std::cout << kStrInfoContructingPerInstructionCfg1;
        std::cout << kStrInfoDxrOutputShader << shader_results.export_name << kStrInfoDxrPerformingPostProcessing << std::endl;

        // Delete the file if it already exists.
        if (BeUtils::IsFilePresent(output_filename))
        {
            KcUtils::DeleteFile(output_filename.c_str());
        }

        is_ok = KcUtils::GenerateControlFlowGraph(isa_file, target, output_filename, NULL,
            true, config_updated.print_process_cmd_line);

        if (is_ok)
        {
            if (KcUtils::FileNotEmpty(output_filename))
            {
                std::cout << kStrInfoSuccess << std::endl;
            }
            else
            {
                std::cout << kStrInfoFailed << std::endl;
                KcUtils::DeleteFile(output_filename);
            }
        }
    }

    return is_ok;
}

// Disassembles ELF container and saves it to the requested output file according to the given Config.
// pipelineBinary is the full path to the pipeline binary ELF container file.
static void DisassembleElfBinary(const std::string& target, const std::string& pipeline_binary, const Config& config, const std::string& output_filename, std::string error_msg)
{
    const char* kStrInfoDisassemblingBinaryElfContainer = "Disassembling pipeline binary ELF container ";
    const char* kStrInfoDisassemblingBinaryElfContainerSuccess = "Pipeline binary ELF container disassembled successfully.";
    const char* kStrInfoDisassemblingBinaryElfFailure = "failure.";
    const char* kStrInfoDisassemblingBinaryElfContainerOnlyVegaRdna = "Disassembling pipeline binary ELF container (--elf-dis) is only supported for binaries generated for Vega, RDNA and beyond. Skipping for ";
    std::cout << kStrInfoDisassemblingBinaryElfContainer << pipeline_binary << "... " << std::endl;
    std::string elf_disassembly;
    std::string elf_disassembly_dot_text;

    if (KcUtils::IsNaviTarget(target) || KcUtils::IsVegaTarget(target))
    {
        const std::string quoted_binary_path = KcUtils::Quote(pipeline_binary);
        bool is_elf_disassembled = BeUtils::DisassembleCodeObject(quoted_binary_path,
            config.print_process_cmd_line, elf_disassembly, elf_disassembly_dot_text, error_msg);
        assert(is_elf_disassembled);
        if (is_elf_disassembled && !elf_disassembly.empty())
        {
            std::cout << kStrInfoDisassemblingBinaryElfContainerSuccess << std::endl;
            [[maybe_unused]] bool isElfDisassemblySaved = KcUtils::WriteTextFile(output_filename, elf_disassembly, nullptr);
            assert(isElfDisassemblySaved);
        }
        else
        {
            std::cout << kStrInfoDisassemblingBinaryElfFailure << std::endl;
        }
    }
    else
    {
        std::cout << kStrInfoDisassemblingBinaryElfContainerOnlyVegaRdna << target << "." << std::endl;
    }
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

                // In DXR pipeline mode, always assume "all" as the export.
                if (is_dxr && !BeDx12Utils::IsDxrShaderMode(config_updated) && config_updated.dxr_exports.empty())
                {
                    config_updated.dxr_exports.push_back("all");
                }

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
                                            for (const RgDxrShaderResults& curr_shader_results : curr_pipeline_results.results)
                                            {
                                                // The key to the map is the export name.
                                                const std::string& currExport = curr_shader_results.export_name;

                                                // Verify ISA files created.
                                                if (!curr_shader_results.isa_disassembly.empty() && !KcUtils::FileNotEmpty(curr_shader_results.isa_disassembly))
                                                {
                                                    std::cout << kStrErrorDx12IsaNotGeneratedA << currExport
                                                        << kStrErrorDxrIsaNotGeneratedB << std::endl;
                                                    is_success = false;
                                                }

                                                if (!curr_shader_results.stats.empty() && !KcUtils::FileNotEmpty(curr_shader_results.stats))
                                                {
                                                    std::cout << kStrErrorDx12StatsNotGeneratedA << currExport
                                                        << kStrErrorDxrIsaNotGeneratedB << std::endl;
                                                    is_success = false;
                                                }
                                            }

                                            // Verify binary files created.
                                            bool should_extract_pipeline_binaries =
                                                config_updated.dxr_mode.compare(kStrDxrModeShader) != 0 && !curr_pipeline_results.pipeline_binary.empty();
                                            if (should_extract_pipeline_binaries)
                                            {
                                                if (!curr_pipeline_results.pipeline_binary.empty() && !KcUtils::FileNotEmpty(curr_pipeline_results.pipeline_binary))
                                                {
                                                    std::cout << kStrErrorDx12BinaryNotGeneratedA << curr_pipeline_results.pipeline_name
                                                        << kStrErrorDxrIsaNotGeneratedBPipeline << std::endl;
                                                    is_success = false;
                                                }
                                                else
                                                {
                                                    // Generate ELF disassembly if needed.
                                                    if (!config_updated.elf_dis.empty())
                                                    {
                                                        // Construct a name for the output file.
                                                        std::string output_filename;
                                                        is_ok = KcUtils::ConstructOutFileName(config_updated.elf_dis, "",
                                                            target, kStrDefaultExtensionText, output_filename);

                                                        // Disassemble the pipeline binary.
                                                        DisassembleElfBinary(target, curr_pipeline_results.pipeline_binary,
                                                            config_updated, output_filename, error_msg);
                                                    }
                                                }
                                            }
                                        }
                                    }

                                    if (is_success)
                                    {
                                        const bool is_shader_mode = BeDx12Utils::IsDxrShaderMode(config_updated);
                                        std::cout << kStrInfoSuccess << std::endl;
                                        if (!config_updated.livereg_analysis_file.empty() || 
                                            !config_updated.sgpr_livereg_analysis_file.empty() ||
                                            !config_updated.inference_analysis_file.empty() ||
                                            !config_updated.inst_cfg_file.empty() ||
                                            !config_updated.block_cfg_file.empty())
                                        {
                                            // Post-processing.
                                            std::cout << kStrInfoDx12PostProcessingSeparator << std::endl;
                                            std::cout << kStrInfoDx12PostProcessing << std::endl;

                                            // Live register analysis files.
                                            if (!config_updated.livereg_analysis_file.empty())
                                            {
                                                for (const RgDxrPipelineResults& curr_pipeline_results : output_mapping)
                                                {
                                                    if (!BeProgramBuilderDx12::IsDxrNullPipeline(curr_pipeline_results.pipeline_name))
                                                    {
                                                        // Announce the pipeline name in pipeline mode.
                                                        std::cout << kStrInfoPerformingLiveregAnalysisVgpr <<
                                                            (curr_pipeline_results.isUnified ? kStrInfoDxrOutputPipelineName : kStrInfoDxrOutputPipelineNumber) <<
                                                            curr_pipeline_results.pipeline_name << "..." << std::endl;
                                                    }

                                                    for (const RgDxrShaderResults& curr_shader_results : curr_pipeline_results.results)
                                                    {
                                                        std::stringstream filename_suffix_stream;
                                                        if (!BeProgramBuilderDx12::IsDxrNullPipeline(curr_pipeline_results.pipeline_name) &&
                                                            !curr_pipeline_results.isUnified)
                                                        {
                                                            filename_suffix_stream << curr_pipeline_results.pipeline_name << "_";
                                                        }
                                                        filename_suffix_stream << curr_shader_results.export_name;
                                                        if (!is_shader_mode && curr_pipeline_results.isUnified)
                                                        {
                                                            filename_suffix_stream << kStrDxrUnifiedSuffix;
                                                        }
                                                        std::string filename_suffix = filename_suffix_stream.str();

                                                        // Do not append a suffix in case that the file name is empty,
                                                        // to prevent a situation where we have the shader name appearing twice.
                                                        bool should_append_suffix = !KcUtils::IsDirectory(config.livereg_analysis_file);

                                                        std::string output_filename;
                                                        KcUtils::ConstructOutFileName(config.livereg_analysis_file, filename_suffix,
                                                            target, kStrDefaultExtensionLivereg, output_filename, should_append_suffix);

                                                        PerformLiveVgprAnalysisDxr(config_updated, curr_shader_results.isa_disassembly, target,
                                                            output_filename, filename_suffix, curr_shader_results);
                                                    }
                                                }
                                            }

                                            // Live register analysis files (Sgpr).
                                            if (!config_updated.sgpr_livereg_analysis_file.empty())
                                            {
                                                for (const RgDxrPipelineResults& curr_pipeline_results : output_mapping)
                                                {
                                                    if (!BeProgramBuilderDx12::IsDxrNullPipeline(curr_pipeline_results.pipeline_name))
                                                    {
                                                        // Announce the pipeline name in pipeline mode.
                                                        std::cout << kStrInfoPerformingLiveregAnalysisSgpr
                                                                  << (curr_pipeline_results.isUnified ? kStrInfoDxrOutputPipelineName
                                                                                                      : kStrInfoDxrOutputPipelineNumber)
                                                                  << curr_pipeline_results.pipeline_name << "..." << std::endl;
                                                    }

                                                    for (const RgDxrShaderResults& curr_shader_results : curr_pipeline_results.results)
                                                    {
                                                        std::stringstream filename_suffix_stream;
                                                        if (!BeProgramBuilderDx12::IsDxrNullPipeline(curr_pipeline_results.pipeline_name) &&
                                                            !curr_pipeline_results.isUnified)
                                                        {
                                                            filename_suffix_stream << curr_pipeline_results.pipeline_name << "_";
                                                        }
                                                        filename_suffix_stream << curr_shader_results.export_name;
                                                        if (!is_shader_mode && curr_pipeline_results.isUnified)
                                                        {
                                                            filename_suffix_stream << kStrDxrUnifiedSuffix;
                                                        }
                                                        std::string filename_suffix = filename_suffix_stream.str();

                                                        // Do not append a suffix in case that the file name is empty,
                                                        // to prevent a situation where we have the shader name appearing twice.
                                                        bool should_append_suffix = !KcUtils::IsDirectory(config.sgpr_livereg_analysis_file);

                                                        std::string output_filename;
                                                        KcUtils::ConstructOutFileName(config.sgpr_livereg_analysis_file,
                                                                                      filename_suffix,
                                                                                      target,
                                                                                      kStrDefaultExtensionLiveregSgpr,
                                                                                      output_filename,
                                                                                      should_append_suffix);

                                                        PerformLiveSgprAnalysisDxr(config_updated,
                                                                                   curr_shader_results.isa_disassembly,
                                                                                   target,
                                                                                   output_filename,
                                                                                   filename_suffix,
                                                                                   curr_shader_results);
                                                    }
                                                }
                                            }

                                            // Per-block control-flow graphs.
                                            if (!config_updated.block_cfg_file.empty())
                                            {
                                                for (const RgDxrPipelineResults& curr_pipeline_results : output_mapping)
                                                {
                                                    if (!BeProgramBuilderDx12::IsDxrNullPipeline(curr_pipeline_results.pipeline_name))
                                                    {
                                                        // Announce the pipeline name in pipeline mode.
                                                        std::cout << kStrInfoContructingPerBlockCfg1 <<
                                                            (curr_pipeline_results.isUnified ? kStrInfoDxrOutputPipelineName : kStrInfoDxrOutputPipelineNumber) <<
                                                            curr_pipeline_results.pipeline_name << "..." << std::endl;
                                                    }

                                                    for (const RgDxrShaderResults& curr_shader_results : curr_pipeline_results.results)
                                                    {
                                                        std::stringstream filename_suffix_stream;
                                                        if (!BeProgramBuilderDx12::IsDxrNullPipeline(curr_pipeline_results.pipeline_name) &&
                                                            !curr_pipeline_results.isUnified)
                                                        {
                                                            filename_suffix_stream << curr_pipeline_results.pipeline_name << "_";
                                                        }
                                                        filename_suffix_stream << curr_shader_results.export_name;
                                                        if (!is_shader_mode && curr_pipeline_results.isUnified)
                                                        {
                                                            filename_suffix_stream << kStrDxrUnifiedSuffix;
                                                        }
                                                        std::string filename_suffix = filename_suffix_stream.str();

                                                        // Do not append a suffix in case that the file name is empty,
                                                        // to prevent a situation where we have the shader name appearing twice.
                                                        bool should_append_suffix = !KcUtils::IsDirectory(config.block_cfg_file);

                                                        std::string output_filename;
                                                        KcUtils::ConstructOutFileName(config.block_cfg_file, filename_suffix,
                                                            target, kStrDefaultExtensionDot, output_filename, should_append_suffix);

                                                        GeneratePerBlockCfgDxr(config_updated, curr_shader_results.isa_disassembly, target,
                                                            output_filename, filename_suffix, curr_shader_results);
                                                    }
                                                }
                                            }

                                            // Per-instruction control-flow graphs.
                                            if (!config_updated.inst_cfg_file.empty())
                                            {
                                                for (const RgDxrPipelineResults& curr_pipeline_results : output_mapping)
                                                {
                                                    if (!BeProgramBuilderDx12::IsDxrNullPipeline(curr_pipeline_results.pipeline_name))
                                                    {
                                                        // Announce the pipeline name in pipeline mode.
                                                        std::cout << kStrInfoContructingPerInstructionCfg1 <<
                                                            (curr_pipeline_results.isUnified ? kStrInfoDxrOutputPipelineName : kStrInfoDxrOutputPipelineNumber) <<
                                                            curr_pipeline_results.pipeline_name << "..." << std::endl;
                                                    }

                                                    for (const RgDxrShaderResults& curr_shader_results : curr_pipeline_results.results)
                                                    {
                                                        std::stringstream filename_suffix_stream;
                                                        if (!BeProgramBuilderDx12::IsDxrNullPipeline(curr_pipeline_results.pipeline_name) &&
                                                            !curr_pipeline_results.isUnified)
                                                        {
                                                            filename_suffix_stream << curr_pipeline_results.pipeline_name << "_";
                                                        }
                                                        filename_suffix_stream << curr_shader_results.export_name;
                                                        if (!is_shader_mode && curr_pipeline_results.isUnified)
                                                        {
                                                            filename_suffix_stream << kStrDxrUnifiedSuffix;
                                                        }
                                                        std::string filename_suffix = filename_suffix_stream.str();

                                                        // Do not append a suffix in case that the file name is empty,
                                                        // to prevent a situation where we have the shader name appearing twice.
                                                        bool should_append_suffix = !KcUtils::IsDirectory(config.inst_cfg_file);

                                                        std::string output_filename;
                                                        KcUtils::ConstructOutFileName(config.inst_cfg_file, filename_suffix,
                                                            target, kStrDefaultExtensionDot, output_filename, should_append_suffix);

                                                        GeneratePerInstructionCfgDxr(config_updated, curr_shader_results.isa_disassembly, target,
                                                            output_filename, filename_suffix, curr_shader_results);
                                                    }
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
                                                // Construct a name for the output file.
                                                std::string output_filename;
                                                is_ok = KcUtils::ConstructOutFileName(config_updated.elf_dis, "",
                                                    target, kStrDefaultExtensionText, output_filename);

                                                // Disassemble the pipeline binary.
                                                DisassembleElfBinary(target, binary_file, config_updated, output_filename, error_msg);

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

#endif

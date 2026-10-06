//=============================================================================
/// Copyright (c) 2019-2026 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implememntation for CLI Commander interface for compiling for DX12.
//=============================================================================
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
#include "radeon_gpu_analyzer_backend/be_analysis_summary.h"
#include "radeon_gpu_analyzer_backend/be_data_types.h"
#include "radeon_gpu_analyzer_backend/be_isa_spec_metadata.h"
#include "radeon_gpu_analyzer_backend/be_metadata_parser.h"
#include "radeon_gpu_analyzer_backend/be_program_builder_binary.h"
#include "radeon_gpu_analyzer_backend/be_program_builder_lightning.h"
#include "radeon_gpu_analyzer_backend/be_utils.h"

// Binary analysis strategy classes, reused for DX12 gfx11+ pipeline binary analysis.
#include "radeon_gpu_analyzer_cli/kc_utils_binary_graphics.h"

// Shared.
#include "common/rga_entry_type.h"
#include "common/rg_log.h"
#include "common/rga_shared_utils.h"

// Local.
#include "radeon_gpu_analyzer_cli/kc_cli_commander_dx12.h"
#include "radeon_gpu_analyzer_cli/kc_cli_isa_spec_loader.h"
#include "radeon_gpu_analyzer_cli/kc_cli_string_constants.h"
#include "radeon_gpu_analyzer_cli/kc_utils_binary_parser.h"
#include "radeon_gpu_analyzer_cli/kc_utils_binary_raytracing.h"
#include "radeon_gpu_analyzer_cli/kc_utils.h"
#include "radeon_gpu_analyzer_cli/kc_xml_writer.h"

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
static const char* kStrWarningDxrLineNumbersRequiresHlslInput =
    "Warning: --line-numbers is only supported when the DXR input is an HLSL file (--hlsl).";

// Constants - info messages.
static const char* kStrInfoTemplateGpsoFileGenerated    = "Template .gpso file created successfully.";
static const char* kStrInfoDx12PostProcessingSeparator  = "-=-=-=-=-=-=-";
static const char* kStrInfoDx12PostProcessing           = "Post-processing...";
static const char* kStrInfoDxrUsingDefaultShaderModel   = "Info: using user-provided shader model instead of the default model (";
static const char* kStrInfoDxrUnifiedPipelineGenerated  = "Pipeline compiled in Unified mode, expect a single uber shader in the output.";
static const char* kStrInfoDxrIndirectPipelineGenerated = "Pipeline compiled in Indirect mode.";
static const char* kStrInfoDxrExtractedDisassemblyA     = "Extracting disassembly for pipeline associated with ";
static const char* kStrInfoDxrExtractedDisassemblyB     = " shader ";
static const char* kStrInfoDxrExtractedDisassemblyC     = "...  ";

// Constants - other.
const char  kStrFileNmaeTokenIndirect = '*';
const char* kStrDefaultDxrShaderModel = "lib_6_3";

// Update the user provided configuration if necessary.
static void UpdateConfig(const Config& user_input, Config& updated_config)
{
    updated_config = user_input;
    bool is_dxr    = (user_input.mode == RgaMode::kModeDxr);
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
            if (!updated_config.ms_entry_point.empty() && updated_config.ms_hlsl.empty() && updated_config.ms_dxbc.empty())
            {
                updated_config.ms_hlsl = updated_config.all_hlsl;
            }
            if (!updated_config.as_entry_point.empty() && updated_config.as_hlsl.empty() && updated_config.as_dxbc.empty())
            {
                updated_config.as_hlsl = updated_config.all_hlsl;
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
            if (!user_input.ms_hlsl.empty())
            {
                present_stages.push_back(user_input.ms_hlsl);
            }
            if (!user_input.as_hlsl.empty())
            {
                present_stages.push_back(user_input.as_hlsl);
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
            std::cout << kStrInfoDxrUsingDefaultShaderModel << kStrDefaultDxrShaderModel << "): " << user_input.dxr_shader_model << std::endl;
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
    bool is_parsed_isa         = config.is_parsed_isa_required;
    return is_livereg_required || is_live_sgpr_required || is_stats_required || is_cfg_required || is_parsed_isa;
}

// ****************************************
// *** INTERNALLY LINKED SYMBOLS - END ***
// ****************************************

void KcCliCommanderDX12::ListAdapters(Config& config, LoggingCallbackFunction)
{
    std::vector<std::string>   supported_gpus;
    std::map<std::string, int> driver_mapping;
    dx12_backend_.GetSupportGpus(config, supported_gpus, driver_mapping);
}

void KcCliCommanderDX12::RunCompileCommands(const Config& config, LoggingCallbackFunction)
{
    bool is_ok        = false;
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
        else if (!config.block_cfg_file.empty() || !config.inst_cfg_file.empty())
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
                bool is_dxr         = (config_updated.mode == RgaMode::kModeDxr);
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

                            if (RgaSharedUtils::IsNavi21AndBeyond(target_lower))
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

                        // --line-numbers is only meaningful for HLSL direct input. Warn and disable for RPSO/JSON inputs.
                        if (config_updated.is_line_numbers_required && config_updated.dxr_hlsl.empty())
                        {
                            std::cout << kStrWarningDxrLineNumbersRequiresHlslInput << std::endl;
                            config_updated.is_line_numbers_required = false;
                        }
                    }

                    assert(!target_devices.empty());
                    if (!target_devices.empty())
                    {
                        if (!config_updated.session_summary_file.empty())
                        {
                            KcCliIsaSpecLoader::LoadIsaSpecsFromXML(true, config_updated.include_target_metadata, target_devices);
                        }

                        // Warn if --line-numbers is used with DXBC/DXIL blob inputs (debug info must be pre-embedded).
                        if (!is_dxr && config_updated.is_line_numbers_required)
                        {
                            bool has_blob_input = !config_updated.vs_dxbc.empty() || !config_updated.hs_dxbc.empty() ||
                                                  !config_updated.ds_dxbc.empty() || !config_updated.gs_dxbc.empty() ||
                                                  !config_updated.ps_dxbc.empty() || !config_updated.cs_dxbc.empty();
                            if (has_blob_input)
                            {
                                std::cout << kStrWarningLineNumbersDxbcInput << std::endl;
                            }
                        }

                        // DX12 graphics or compute.
                        for (const std::string& target : target_devices)
                        {
                            // Track the devices that we covered so that we do not compile twice.
                            if (std::find(completed_targets.begin(), completed_targets.end(), target) == completed_targets.end())
                            {
                                // Mark as covered.
                                completed_targets.push_back(target);

                                std::string out_text;
                                std::string error_msg;
                                beStatus    rc = beStatus::kBeStatusInvalid;
                                std::cout << kStrInfoCompiling << target << "..." << std::endl;

                                if (is_dxr)
                                {
                                    std::vector<RgDxrPipelineResults> output_mapping;
                                    rc    = dx12_backend_.CompileDXRPipeline(config_updated, target, out_text, output_mapping, error_msg);
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
                                                    // Use llvm-objdump for ISA extraction with source-line correlation.
                                                    is_success =
                                                        PostProcessDxrPipelineBinary(config_updated, target, curr_pipeline_results.pipeline_binary, error_msg);
                                                    if (!is_success && !error_msg.empty())
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
                                    BePipelineFiles isa_files;
                                    BePipelineFiles amdil_files;
                                    BePipelineFiles stats_files;
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

                                        if (!binary_file.empty() && !KcUtils::FileNotEmpty(binary_file))
                                        {
                                            std::cout << kStrErrorDx12BinaryNotGeneratedA << target << std::endl;
                                            is_success = false;
                                        }
                                        else if (!binary_file.empty())
                                        {
                                            // Extract and parse metadata.
                                            std::string metadata_text, metadata_error;
                                            beStatus    md_status = BeProgramBuilderLightning::ExtractMetadata(config_updated.compiler_bin_path,
                                                                                                            binary_file,
                                                                                                            config_updated.print_process_cmd_line,
                                                                                                            metadata_text,
                                                                                                            metadata_error);

                                            BeAmdPalMetaData::PipelineMetaData amdpal_pipeline_md;
                                            if (md_status == kBeStatusSuccess)
                                            {
                                                BeAmdPalMetaData::ParseMetadata(metadata_text, amdpal_pipeline_md);
                                            }

                                            // Create disassembly and analysis strategies.
                                            CmpilerPaths compiler_paths = {
                                                config_updated.compiler_bin_path, config_updated.compiler_inc_path, config_updated.compiler_lib_path};
                                            KcCliLlvmObjdumpGraphicsStrategy    disassembly_strategy(binary_file,
                                                                                                  target,
                                                                                                  compiler_paths,
                                                                                                  config_updated.is_line_numbers_required,
                                                                                                  config_updated.print_process_cmd_line,
                                                                                                  amdpal_pipeline_md);
                                            KcCliGraphicsBinaryAnalysisStrategy analysis_strategy(
                                                binary_file, beProgramBuilderBinary::ApiEnum::kDX12, std::move(amdpal_pipeline_md), log_callback_);

                                            // Disassemble via llvm-objdump.
                                            std::string text_disassembly;
                                            beStatus    disasm_status = disassembly_strategy.Disassemble(
                                                binary_file, {}, {}, config_updated.print_process_cmd_line, text_disassembly);

                                            if (disasm_status == kBeStatusSuccess && !text_disassembly.empty())
                                            {
                                                if (!config_updated.elf_dis.empty())
                                                {
                                                    std::string output_filename;
                                                    if (KcUtils::ConstructOutFileName(
                                                            config_updated.elf_dis, "", target, kStrDefaultExtensionText, output_filename))
                                                    {
                                                        std::stringstream combined;
                                                        combined << metadata_text << "\n\n" << text_disassembly;
                                                        KcUtils::WriteTextFile(output_filename, combined.str(), nullptr);
                                                        std::cout << "Pipeline binary ELF container disassembled successfully." << std::endl;
                                                    }
                                                }

                                                // Parse kernels and write ISA + stats output files.
                                                std::map<std::string, std::string> kernel_to_disassembly;
                                                beKA::beStatus                     parse_status =
                                                    disassembly_strategy.ParseKernels(text_disassembly, kernel_to_disassembly, error_msg);
                                                if (parse_status == beKA::beStatus::kBeStatusSuccess && !kernel_to_disassembly.empty())
                                                {
                                                    analysis_strategy.WriteOutputFiles(config_updated, target, kernel_to_disassembly, error_msg);
                                                }

                                                // Post-processing (parsed ISA CSV, livereg, CFG).
                                                std::cout << kStrInfoSuccess << std::endl;
                                                std::cout << kStrInfoDx12PostProcessingSeparator << std::endl;
                                                std::cout << kStrInfoDx12PostProcessing << std::endl;
                                                analysis_strategy.RunPostProcessingSteps(config_updated);

                                                // Session summary.
                                                if (!config_updated.session_summary_file.empty())
                                                {
                                                    RgaAnalysisSummary::AnalysisResult result;
                                                    if (analysis_strategy.GeneratCompilationSummary(config_updated, target, result))
                                                    {
                                                        // Replace binary path with user-provided source inputs.
                                                        result.inputs_.inputs_.clear();
                                                        for (const std::string& input : {config_updated.vs_hlsl,
                                                                                         config_updated.hs_hlsl,
                                                                                         config_updated.ds_hlsl,
                                                                                         config_updated.gs_hlsl,
                                                                                         config_updated.ps_hlsl,
                                                                                         config_updated.cs_hlsl,
                                                                                         config_updated.all_hlsl,
                                                                                         config_updated.vs_dxbc,
                                                                                         config_updated.hs_dxbc,
                                                                                         config_updated.ds_dxbc,
                                                                                         config_updated.gs_dxbc,
                                                                                         config_updated.ps_dxbc,
                                                                                         config_updated.cs_dxbc,
                                                                                         config_updated.pso_dx12,
                                                                                         config_updated.rs_hlsl,
                                                                                         config_updated.rs_bin})
                                                        {
                                                            if (!input.empty())
                                                            {
                                                                result.inputs_.inputs_.push_back(input);
                                                            }
                                                        }
                                                        result.output_.api_ = kStrRgaModeDx12;
                                                        summary_.results_.push_back(std::move(result));
                                                    }
                                                }
                                            }
                                            else if (!error_msg.empty())
                                            {
                                                std::cout << error_msg << std::endl;
                                            }
                                        }

                                        // Clean up temporary pipeline binary if the user did not request it.
                                        if (config_updated.binary_output_file.empty() && !binary_file.empty() && !config.should_retain_temp_files)
                                        {
                                            KcUtils::DeleteFileW(binary_file);
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

                        if (!config_updated.session_summary_file.empty())
                        {
                            bool ret = KcXmlWriter::WriteAnalysisSummaryToFile(summary_, config_updated.session_summary_file);
                            if (!ret)
                            {
                                RgLog::stdOut << kStrErrorFailedToGenerateSessionSummary << std::endl;
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
            if (RgaSharedUtils::IsNavi21AndBeyond(targetName))
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

bool KcCliCommanderDX12::PostProcessDxrPipelineBinary(const Config& config, const std::string& target, const std::string& pipeline_elf, std::string& error_msg)
{
    bool is_success = false;

    // Extract metadata via llvm-readobj.
    std::string metadata_text, metadata_error;
    beStatus    md_extract_status =
        BeProgramBuilderLightning::ExtractMetadata(config.compiler_bin_path, pipeline_elf, config.print_process_cmd_line, metadata_text, metadata_error);
    if (md_extract_status != kBeStatusSuccess)
    {
        error_msg = metadata_error;
        return false;
    }

    // Parse metadata to detect pipeline type.
    RaytracingPipelineMetaData pipeline_md;
    beKA::beStatus             md_status = BeAmdPalMetaData::ParseMetadata(metadata_text, pipeline_md);
    if (md_status != beKA::beStatus::kBeStatusRayTracingCodeObjMetaDataSuccess && md_status != beKA::beStatus::kBeStatusComputeCodeObjMetaDataSuccess)
    {
        // Not a processable code object, silently skip (e.g. NPRT shader identifiers).
        return true;
    }

    // Disassemble and split ISA.
    CmpilerPaths                     compiler_paths = {config.compiler_bin_path, config.compiler_inc_path, config.compiler_lib_path};
    KcCliLlvmObjdumpGraphicsStrategy disassembly_strategy(
        pipeline_elf, target, compiler_paths, config.is_line_numbers_required, config.print_process_cmd_line, pipeline_md);

    std::string text_disassembly;
    beStatus    disasm_status = disassembly_strategy.Disassemble(pipeline_elf, {}, {}, config.print_process_cmd_line, text_disassembly);
    if (disasm_status != kBeStatusSuccess || text_disassembly.empty())
    {
        error_msg = "Failed to disassemble pipeline binary with llvm-objdump.";
        return false;
    }

    if (!config.elf_dis.empty())
    {
        std::string output_filename;
        if (KcUtils::ConstructOutFileName(config.elf_dis, "", target, kStrDefaultExtensionText, output_filename))
        {
            std::stringstream combined;
            combined << metadata_text << "\n\n" << text_disassembly;
            KcUtils::WriteTextFile(output_filename, combined.str(), nullptr);
        }
    }

    // Parse kernels from the disassembly.
    std::map<std::string, std::string> kernel_to_disassembly;
    beKA::beStatus                     parse_status = disassembly_strategy.ParseKernels(text_disassembly, kernel_to_disassembly, error_msg);
    if (parse_status != beKA::beStatus::kBeStatusSuccess || kernel_to_disassembly.empty())
    {
        // Fallback: use the entire ISA as a single kernel.
        auto kernel_names = beProgramBuilderBinary::GetKernelNames(pipeline_md);
        if (!kernel_names.empty())
        {
            kernel_to_disassembly[kernel_names.front()] = text_disassembly;
        }
    }

    // Determine pipeline type and feed into raytracing analysis strategy.
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
            auto found = kernel_to_disassembly.find(shader_function.raw_name);
            if (found != kernel_to_disassembly.end())
            {
                kernel_to_disassembly.erase(found);
            }
        }
    }

    if (!ignore_pipeline_binary)
    {
        KcCliRaytracingBinaryAnalysisStrategy processor{pipeline_elf, std::move(pipeline_md), log_callback_};
        beKA::beStatus                        status = processor.WriteOutputFiles(config, target, kernel_to_disassembly, error_msg);
        if (status == beKA::beStatus::kBeStatusSuccess)
        {
            std::cout << kStrInfoSuccess << "\n";
            // Post-processing.
            if (IsDxrPostProcessingRequired(config))
            {
                std::cout << kStrInfoDx12PostProcessingSeparator << "\n";
                std::cout << kStrInfoDx12PostProcessing << "\n";
                processor.RunPostProcessingSteps(config);
            }

            if (!config.session_summary_file.empty())
            {
                RgaAnalysisSummary::AnalysisResult result;
                if (processor.GeneratCompilationSummary(config, target, result) && !result.output_.kernels_.empty())
                {
                    result.inputs_.inputs_.clear();
                    for (const std::string& input : {config.dxr_hlsl, config.dxr_state_desc})
                    {
                        if (!input.empty())
                        {
                            result.inputs_.inputs_.push_back(input);
                        }
                    }
                    summary_.results_.push_back(std::move(result));
                }
            }

            is_success = true;
        }
        else
        {
            std::cout << kStrInfoFailed << "\n";
        }
    }
    else
    {
        // Not an actionable pipeline (e.g. indirect mode traverse shader).
        is_success = true;
    }

    return is_success;
}
#endif

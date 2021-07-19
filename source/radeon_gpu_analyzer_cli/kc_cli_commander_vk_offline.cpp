//=================================================================
// Copyright 2020-2021 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

// C++.
#include <iterator>

// Local.
#include "radeon_gpu_analyzer_cli/kc_cli_commander_vk_offline.h"
#include "radeon_gpu_analyzer_cli/kc_cli_string_constants.h"
#include "radeon_gpu_analyzer_cli/kc_utils.h"
#include "radeon_gpu_analyzer_cli/kc_statistics_parser_vulkan.h"

// Backend.
#include "radeon_gpu_analyzer_backend/be_program_builder_vulkan.h"
#include "radeon_gpu_analyzer_backend/be_backend.h"

// Shared between CLI and GUI.
#include "common/rga_version_info.h"

// Infra.
#include "AMDTOSWrappers/Include/osFilePath.h"
#include "AMDTOSWrappers/Include/osDirectory.h"

// Constants: error messages.
static const char* kStrErrorVkOfflineCannotExtractVulkanVersion = "Error: unable to extract the Vulkan version.";
static const char* kStrErrorVkOfflineMixedInputFiles = "Error: cannot mix stage-specific input files (--vert, --tesc, --tese, --geom, --frag, --comp) with a stage-less SPIR-V input file.";

// Unsupported devices.
static const std::set<std::string> kUnsupportedDevicesVkOffline = {"gfx908"};

KcCLICommanderVkOffline::KcCLICommanderVkOffline() : vulkan_builder_(new BeProgramBuilderVkOffline)
{
}

KcCLICommanderVkOffline::~KcCLICommanderVkOffline()
{
    delete vulkan_builder_;
}

bool KcCLICommanderVkOffline::GetSupportedDevices()
{
    bool ret = !supported_devices_cache_.empty();
    if (!ret)
    {
        ret = BeProgramBuilderVkOffline::GetSupportedDevices(supported_devices_cache_);
    }
    return (ret && !supported_devices_cache_.empty());
}

void KcCLICommanderVkOffline::Version(Config& config, LoggingCallbackFunction callback)
{
    GT_UNREFERENCED_PARAMETER(config);

    std::stringstream log_msg;
    log_msg << kStrRgaProductName << " " << kStrRgaVersionPrefix << kStrRgaVersion << "." << kStrRgaBuildNum << std::endl;

    if (vulkan_builder_ != nullptr)
    {
        gtString vk_version;
        bool rc = vulkan_builder_->GetVulkanVersion(vk_version);
        if (rc && !vk_version.isEmpty())
        {
            vk_version << L"\n";
            log_msg << vk_version.asASCIICharArray();
        }
        else
        {
            log_msg << kStrErrorVkOfflineCannotExtractVulkanVersion << std::endl;
        }
    }

    callback(log_msg.str());
}

void KcCLICommanderVkOffline::RunCompileCommands(const Config& config, LoggingCallbackFunction callback)
{
    log_callback_ = callback;
    bool status = true;

    // Output stream.
    std::stringstream log_msg, warning_msg;

    // Input validation.
    bool should_abort = false;
    bool is_stageless_input_file_present = false;
    bool is_vert_shader_present = (!config.vertex_shader.empty());
    bool is_tess_control_shader_present = (!config.tess_control_shader.empty());
    bool is_tess_evaluation_shader_present = (!config.tess_evaluation_shader.empty());
    bool is_geom_shader_present = (!config.geometry_shader.empty());
    bool is_frag_shader_present = (!config.fragment_shader.empty());
    bool is_comp_shader_present = (!config.compute_shader.empty());
    bool is_isa_required = (!config.isa_file.empty() || !config.livereg_analysis_file.empty() ||
                          !config.block_cfg_file.empty() || !config.inst_cfg_file.empty() || !config.analysis_file.empty());
    bool is_livereg_analysis_required = (!config.livereg_analysis_file.empty());
    bool is_stall_analysis_required = (!config.stall_analysis_file.empty());
    bool is_block_cfg_required = (!config.block_cfg_file.empty());
    bool is_inst_cfg_required = (!config.inst_cfg_file.empty());
    bool is_pipeline_binary_required = (!config.binary_output_file.empty());
    bool is_il_required = (!config.il_file.empty());
    bool is_stats_required = (!config.analysis_file.empty());
    bool is_pipe_input = (!config.pipe_file.empty());

    // Fatal error. This should not happen unless we have an allocation problem.
    if (vulkan_builder_ == nullptr)
    {
        should_abort = true;
        log_msg << kStrErrorMemoryAllocationFailure << std::endl;
    }

    // Cannot mix compute and non-compute shaders in Vulkan.
    if (is_comp_shader_present && (is_vert_shader_present || is_tess_control_shader_present ||
                                   is_tess_evaluation_shader_present || is_geom_shader_present || is_frag_shader_present))
    {
        log_msg << kStrErrorGraphicsComputeMix << std::endl;
        should_abort = true;
    }

    if (is_pipe_input)
    {
        if (KcUtils::GetFileExtension(config.pipe_file).compare("pipe") != 0)
        {
            const char* kStrErrorPipeFileExtension = "Error: .pipe file must have a .pipe extension.";
            std::cerr << kStrErrorPipeFileExtension << std::endl;
            should_abort = true;
        }
    }

    // Options to be passed to the backend.
    VkOfflineOptions vulkan_options(config.mode);
    if (!should_abort && !config.input_files.empty() && !config.input_files[0].empty())
    {
        is_stageless_input_file_present = true;
        should_abort = !KcUtils::ValidateShaderFileName("", config.input_files[0], log_msg);
        vulkan_options.stageless_input_file = config.input_files[0];
    }

    // Validate the input shaders.
    if (!should_abort && is_vert_shader_present)
    {
        should_abort = !KcUtils::ValidateShaderFileName(kStrVertexStage, config.vertex_shader, log_msg);
        vulkan_options.pipeline_shaders.vertex_shader << config.vertex_shader.c_str();
    }

    if (!should_abort && is_tess_control_shader_present)
    {
        should_abort = !KcUtils::ValidateShaderFileName(kStrTessellationControlStageName, config.tess_control_shader, log_msg);
        vulkan_options.pipeline_shaders.tessellation_control_shader << config.tess_control_shader.c_str();
    }

    if (!should_abort && is_tess_evaluation_shader_present)
    {
        should_abort = !KcUtils::ValidateShaderFileName(kStrTessellationEvaluationStageName, config.tess_evaluation_shader, log_msg);
        vulkan_options.pipeline_shaders.tessellation_evaluation_shader << config.tess_evaluation_shader.c_str();
    }

    if (!should_abort && is_geom_shader_present)
    {
        should_abort = !KcUtils::ValidateShaderFileName(kStrGeometryStageName, config.geometry_shader, log_msg);
        vulkan_options.pipeline_shaders.geometry_shader << config.geometry_shader.c_str();
    }

    if (!should_abort && is_frag_shader_present)
    {
        should_abort = !KcUtils::ValidateShaderFileName(kStrFragmentStageName, config.fragment_shader, log_msg);
        vulkan_options.pipeline_shaders.fragment_shader << config.fragment_shader.c_str();
    }

    if (!should_abort && is_comp_shader_present)
    {
        should_abort = !KcUtils::ValidateShaderFileName(kStrComputeStageName, config.compute_shader, log_msg);
        vulkan_options.pipeline_shaders.compute_shader << config.compute_shader.c_str();
    }

    // Validate the output directories.
    if (!should_abort && is_isa_required && !config.isa_file.empty())
    {
        should_abort = !KcUtils::ValidateShaderOutputDir(config.isa_file, log_msg);
    }

    if (!should_abort && is_livereg_analysis_required)
    {
        should_abort = !KcUtils::ValidateShaderOutputDir(config.livereg_analysis_file, log_msg);
    }

    if (!should_abort && is_block_cfg_required)
    {
        should_abort = !KcUtils::ValidateShaderOutputDir(config.block_cfg_file, log_msg);
    }

    if (!should_abort && is_inst_cfg_required)
    {
        should_abort = !KcUtils::ValidateShaderOutputDir(config.inst_cfg_file, log_msg);
    }

    if (!should_abort && is_il_required)
    {
        should_abort = !KcUtils::ValidateShaderOutputDir(config.il_file, log_msg);
    }

    if (!should_abort && is_stats_required)
    {
        should_abort = !KcUtils::ValidateShaderOutputDir(config.analysis_file, log_msg);
    }

    if (!should_abort && is_pipeline_binary_required)
    {
        should_abort = !KcUtils::ValidateShaderOutputDir(config.binary_output_file, log_msg);
    }

    if (!should_abort)
    {
        // Set the log callback for the backend.
        vulkan_builder_->SetLog(callback);

        // If the user did not specify any device, we should use all supported devices.
        std::set<std::string> target_devices;
        std::set<std::string> target_devices_tmp;

        if (supported_devices_cache_.empty())
        {
            // We need to populate the list of supported devices.
            bool is_device_list_extracted = GetSupportedDevices();
            if (!is_device_list_extracted)
            {
                std::stringstream error_msg;
                error_msg << kStrErrorCannotExtractSupportedDeviceList << std::endl;
                should_abort = true;
            }
        }

        if (!should_abort)
        {
            InitRequestedAsicList(config.asics, config.mode, supported_devices_cache_, target_devices_tmp, false);

            // Filter unsupported targets.
            for (const std::string device_name : target_devices_tmp)
            {
                if (kUnsupportedDevicesVkOffline.find(device_name) == kUnsupportedDevicesVkOffline.end())
                {
                    target_devices.insert(device_name);
                }
            }

            for (const std::string& device : target_devices)
            {
                // Generate the output message.
                log_msg << kStrInfoCompiling << device << "... ";

                gtString device_gt_str;
                device_gt_str << device.c_str();

                // Set the target device for the backend.
                vulkan_options.target_device_name = device;

                // Track the .pipe input based on user configuration.
                if (is_pipe_input)
                {
                    vulkan_options.pipe_file = config.pipe_file;
                }

                // Lower case of the device name to be used for generating output paths.
                std::string device_to_lower = KcUtils::ToLower(device);

                // Set the optimization level.
                if (config.opt_level == -1 || config.opt_level == 0 || config.opt_level == 1)
                {
                    vulkan_options.optimization_level = config.opt_level;
                }
                else
                {
                    vulkan_options.optimization_level = -1;
                    warning_msg << kStrWarningVkOfflineIncorrectOptLevel << std::endl;
                }

                // Adjust the output file names to the device and shader type.
                if (is_isa_required)
                {
                    // We must generate the ISA binaries (we will delete them in the end of the process).
                    vulkan_options.is_amd_isa_binaries_required = true;
                    std::string adjusted_base_file_name = KcUtils::AdjustBaseFileNameBinary(config.binary_output_file, device);
                    status &= KcUtils::AdjustRenderingPipelineOutputFileNames(adjusted_base_file_name, "", "isabin",
                        device_to_lower, vulkan_options.isa_binary_output_files);

                    if (is_isa_required)
                    {
                        vulkan_options.is_amd_isa_disassembly_required = true;
                        std::string adjusted_base_file_name = KcUtils::AdjustBaseFileNameIsaDisassembly(config.isa_file, device);

                        status &= KcUtils::AdjustRenderingPipelineOutputFileNames(adjusted_base_file_name, "", kStrDefaultExtensionIsa,
                            device_to_lower, vulkan_options.isa_disassembly_output_files);
                    }
                }

                // Adjust the output file names to the device and shader type.
                if (is_pipeline_binary_required)
                {
                    vulkan_options.is_pipeline_binary_required = true;
                    KcUtils::ConstructOutputFileName(config.binary_output_file, "", "bin", "", device, vulkan_options.pipeline_binary);
                }

                if (is_il_required)
                {
                    vulkan_options.is_amd_pal_disassembly_required = true;
                    std::string adjusted_base_file_name = KcUtils::AdjustBaseFileNameIlDisassembly(config.il_file, device);
                    status &= KcUtils::AdjustRenderingPipelineOutputFileNames(adjusted_base_file_name, "", kStrDefaultExtensionAmdil,
                        device_to_lower, vulkan_options.pal_il_disassembly_output_files);
                }

                if (is_stats_required)
                {
                    vulkan_options.is_stats_required = true;
                    std::string adjusted_base_file_name = KcUtils::AdjustBaseFileNameStats(config.analysis_file, device);
                    status &= KcUtils::AdjustRenderingPipelineOutputFileNames(adjusted_base_file_name, kStrDefaultExtensionStats,
                        kStrDefaultExtensionCsv, device_to_lower, vulkan_options.stats_output_files);
                }

                // Live register analysis.
                if (is_livereg_analysis_required)
                {
                    vulkan_options.is_livereg_required = true;
                    std::string adjusted_base_file_name = KcUtils::AdjustBaseFileNameLivereg(config.livereg_analysis_file, device);
                    status &= KcUtils::AdjustRenderingPipelineOutputFileNames(adjusted_base_file_name, kStrDefaultExtensionLivereg,
                        kStrDefaultExtensionText, device_to_lower, vulkan_options.livereg_output_files);
                }

                // Stall analysis.
                if (is_stall_analysis_required)
                {
                    vulkan_options.is_stall_analysis_required = true;
                    std::string adjustedBaseFileName = KcUtils::AdjustBaseFileNameStallAnalysis(config.stall_analysis_file, device);
                    status &= KcUtils::AdjustRenderingPipelineOutputFileNames(adjustedBaseFileName, kStrDefaultExtensionStalls,
                        kStrDefaultExtensionText, device_to_lower, vulkan_options.stall_output_files);
                }

                // CFG.
                if (is_block_cfg_required)
                {
                    std::string adjusted_base_file_name = KcUtils::AdjustBaseFileNameCfg(config.block_cfg_file, device);
                    status &= KcUtils::AdjustRenderingPipelineOutputFileNames(adjusted_base_file_name, KC_STR_DEFAULT_CFG_SUFFIX, kStrDefaultExtensionDot,
                        device_to_lower, vulkan_options.cfg_output_files);
                }

                if (is_inst_cfg_required)
                {
                    std::string adjusted_base_file_name = KcUtils::AdjustBaseFileNameCfg(config.inst_cfg_file, device);
                    status &= KcUtils::AdjustRenderingPipelineOutputFileNames(adjusted_base_file_name, KC_STR_DEFAULT_CFG_SUFFIX, kStrDefaultExtensionDot,
                        device_to_lower, vulkan_options.cfg_output_files);
                }
                if (!status)
                {
                    log_msg << kStrErrorFailedToAdjustFileName << std::endl;
                    should_abort = true;
                    break;
                }

                // A handle for canceling the build. Currently not in use.
                bool should_cancel = false;

                // Compile.
                gtString build_error_log;
                beKA::beStatus compilation_status = vulkan_builder_->Compile(vulkan_options, should_cancel, config.print_process_cmd_line, build_error_log);

                if (compilation_status == kBeStatusSuccess)
                {
                    log_msg << kStrInfoSuccess << std::endl;

                    // Parse ISA and write it to a csv file if required.
                    if (is_isa_required && config.is_parsed_isa_required)
                    {
                        bool status;
                        std::string  isa_text, parsed_isa_text, parsed_isa_filename;
                        BeProgramPipeline  isa_files = vulkan_options.isa_disassembly_output_files;
                        for (const gtString& isa_file_name : { isa_files.compute_shader, isa_files.fragment_shader, isa_files.geometry_shader,
                                                             isa_files.tessellation_control_shader, isa_files.tessellation_evaluation_shader, isa_files.vertex_shader })
                        {
                            if (!isa_file_name.isEmpty() && KcUtils::FileNotEmpty(isa_file_name.asASCIICharArray()))
                            {
                                if ((status = KcUtils::ReadTextFile(isa_file_name.asASCIICharArray(), isa_text, log_callback_)) == true)
                                {
                                    status = (BeProgramBuilder::ParseIsaToCsv(isa_text, device, parsed_isa_text) == beStatus::kBeStatusSuccess);
                                    if (status)
                                    {
                                        status = KcUtils::GetParsedISAFileName(isa_file_name.asASCIICharArray(), parsed_isa_filename);
                                    }
                                    if (status)
                                    {
                                        status = KcUtils::WriteTextFile(parsed_isa_filename, parsed_isa_text, log_callback_);
                                    }
                                }
                                if (!status)
                                {
                                    log_msg << kStrErrorVkOfflineCannotExtractVulkanVersion << std::endl;
                                }
                            }
                        }
                    }

                    // Parse the statistics file if required.
                    if (is_stats_required)
                    {
                        beKA::AnalysisData statistics;
                        KcVulkanStatisticsParser stats_parser;

                        if ((is_vert_shader_present || KcUtils::FileNotEmpty(vulkan_options.stats_output_files.vertex_shader.asASCIICharArray()))
                            || (is_stageless_input_file_present && KcUtils::FileNotEmpty(vulkan_options.stats_output_files.vertex_shader.asASCIICharArray())))
                        {
                            // If vertex shader is present, or a vertex shader was auto-generated by the offline backend, replace the statistics.
                            KcUtils::ReplaceStatisticsFile(vulkan_options.stats_output_files.vertex_shader, config, device, stats_parser, callback);
                        }

                        if (is_tess_control_shader_present || (is_stageless_input_file_present && KcUtils::FileNotEmpty(vulkan_options.stats_output_files.tessellation_control_shader.asASCIICharArray())))
                        {
                            KcUtils::ReplaceStatisticsFile(vulkan_options.stats_output_files.tessellation_control_shader, config, device, stats_parser, callback);
                        }

                        if (is_tess_evaluation_shader_present || (is_stageless_input_file_present && KcUtils::FileNotEmpty(vulkan_options.stats_output_files.tessellation_evaluation_shader.asASCIICharArray())))
                        {
                            KcUtils::ReplaceStatisticsFile(vulkan_options.stats_output_files.tessellation_evaluation_shader, config, device, stats_parser, callback);
                        }

                        if (is_geom_shader_present || (is_stageless_input_file_present && KcUtils::FileNotEmpty(vulkan_options.stats_output_files.geometry_shader.asASCIICharArray())))
                        {
                            KcUtils::ReplaceStatisticsFile(vulkan_options.stats_output_files.geometry_shader, config, device, stats_parser, callback);
                        }

                        if (is_frag_shader_present || (is_stageless_input_file_present && KcUtils::FileNotEmpty(vulkan_options.stats_output_files.fragment_shader.asASCIICharArray())))
                        {
                            KcUtils::ReplaceStatisticsFile(vulkan_options.stats_output_files.fragment_shader, config, device, stats_parser, callback);
                        }

                        if (is_comp_shader_present || (is_stageless_input_file_present && KcUtils::FileNotEmpty(vulkan_options.stats_output_files.compute_shader.asASCIICharArray())))
                        {
                            KcUtils::ReplaceStatisticsFile(vulkan_options.stats_output_files.compute_shader, config, device, stats_parser, callback);
                        }
                    }

                    // Perform live register analysis if required.
                    if (is_livereg_analysis_required)
                    {
                        if (is_vert_shader_present || KcUtils::FileNotEmpty(vulkan_options.isa_disassembly_output_files.vertex_shader.asASCIICharArray()))
                        {
                            // If vertex shader is present, or a vertex shader was auto-generated by the offline backend, perform live register analysis.
                            KcUtils::PerformLiveRegisterAnalysis(vulkan_options.isa_disassembly_output_files.vertex_shader,
                                                                 device_gt_str,
                                                                 vulkan_options.livereg_output_files.vertex_shader,
                                                                 callback,
                                                                 config.print_process_cmd_line);
                        }

                        if (is_tess_control_shader_present)
                        {
                            // When tessellation is enabled on Vega and Navi, Vertex and Tesc are merged.
                            KcUtils::PerformLiveRegisterAnalysis(vulkan_options.isa_disassembly_output_files.tessellation_control_shader,
                                                                 device_gt_str,
                                                                 vulkan_options.livereg_output_files.tessellation_control_shader,
                                                                 callback,
                                                                 config.print_process_cmd_line);
                        }

                        if (is_tess_evaluation_shader_present)
                        {
                            KcUtils::PerformLiveRegisterAnalysis(vulkan_options.isa_disassembly_output_files.tessellation_evaluation_shader,
                                                                 device_gt_str,
                                                                 vulkan_options.livereg_output_files.tessellation_evaluation_shader,
                                                                 callback,
                                                                 config.print_process_cmd_line);
                        }

                        if (is_geom_shader_present)
                        {
                            // When tessellation evaluation is present on Vega and Navi, GS and Tesc are merged.
                            KcUtils::PerformLiveRegisterAnalysis(vulkan_options.isa_disassembly_output_files.geometry_shader,
                                                                 device_gt_str,
                                                                 vulkan_options.livereg_output_files.geometry_shader,
                                                                 callback,
                                                                 config.print_process_cmd_line);
                        }

                        if (is_frag_shader_present)
                        {
                            KcUtils::PerformLiveRegisterAnalysis(vulkan_options.isa_disassembly_output_files.fragment_shader,
                                                                 device_gt_str,
                                                                 vulkan_options.livereg_output_files.fragment_shader,
                                                                 callback,
                                                                 config.print_process_cmd_line);
                        }

                        if (is_comp_shader_present)
                        {
                            KcUtils::PerformLiveRegisterAnalysis(vulkan_options.isa_disassembly_output_files.compute_shader,
                                                                 device_gt_str,
                                                                 vulkan_options.livereg_output_files.compute_shader,
                                                                 callback,
                                                                 config.print_process_cmd_line);
                        }

                        // Process stageless files.
                        if (is_pipe_input || !vulkan_options.stageless_input_file.empty())
                        {
                            for (const auto& isaAndLiveReg : {
                                   std::pair<gtString, gtString>{vulkan_options.isa_disassembly_output_files.vertex_shader, vulkan_options.livereg_output_files.vertex_shader},
                                   std::pair<gtString, gtString>{vulkan_options.isa_disassembly_output_files.tessellation_control_shader, vulkan_options.livereg_output_files.tessellation_control_shader},
                                   std::pair<gtString, gtString>{vulkan_options.isa_disassembly_output_files.tessellation_evaluation_shader, vulkan_options.livereg_output_files.tessellation_evaluation_shader},
                                   std::pair<gtString, gtString>{vulkan_options.isa_disassembly_output_files.geometry_shader, vulkan_options.livereg_output_files.geometry_shader},
                                   std::pair<gtString, gtString>{vulkan_options.isa_disassembly_output_files.fragment_shader, vulkan_options.livereg_output_files.fragment_shader},
                                   std::pair<gtString, gtString>{vulkan_options.isa_disassembly_output_files.compute_shader, vulkan_options.livereg_output_files.compute_shader} } )
                            {
                                if (KcUtils::FileNotEmpty(isaAndLiveReg.first.asASCIICharArray()))
                                {
                                    KcUtils::PerformLiveRegisterAnalysis( isaAndLiveReg.first, device_gt_str,
                                        isaAndLiveReg.second, callback, config.print_process_cmd_line);
                                }
                            }
                        }
                    }

                    // Perform stall analysis if required.
                    if (is_stall_analysis_required)
                    {
                        if (KcUtils::IsNaviTarget(device))
                        {
                            if (is_vert_shader_present || KcUtils::FileNotEmpty(vulkan_options.isa_disassembly_output_files.vertex_shader.asASCIICharArray()))
                            {
                                // If vertex shader is present, or a vertex shader was auto-generated by the offline backend, perform live register analysis.
                                KcUtils::PerformStallAnalysis(vulkan_options.isa_disassembly_output_files.vertex_shader,
                                                              device_gt_str,
                                                              vulkan_options.stall_output_files.vertex_shader,
                                                              callback,
                                                              config.print_process_cmd_line);
                            }

                            if (is_tess_control_shader_present)
                            {
                                // When tessellation is enabled on Vega and Navi, Vertex and Tesc are merged.
                                KcUtils::PerformStallAnalysis(vulkan_options.isa_disassembly_output_files.tessellation_control_shader,
                                                              device_gt_str,
                                                              vulkan_options.stall_output_files.tessellation_control_shader,
                                                              callback,
                                                              config.print_process_cmd_line);
                            }

                            if (is_tess_evaluation_shader_present)
                            {
                                KcUtils::PerformStallAnalysis(vulkan_options.isa_disassembly_output_files.tessellation_evaluation_shader,
                                                              device_gt_str,
                                                              vulkan_options.stall_output_files.tessellation_evaluation_shader,
                                                              callback,
                                                              config.print_process_cmd_line);
                            }

                            if (is_geom_shader_present)
                            {
                                // When tessellation evaluation is present on Vega and Navi, GS and Tesc are merged.
                                KcUtils::PerformStallAnalysis(vulkan_options.isa_disassembly_output_files.geometry_shader,
                                                              device_gt_str,
                                                              vulkan_options.stall_output_files.geometry_shader,
                                                              callback,
                                                              config.print_process_cmd_line);
                            }

                            if (is_frag_shader_present)
                            {
                                KcUtils::PerformStallAnalysis(vulkan_options.isa_disassembly_output_files.fragment_shader,
                                                              device_gt_str,
                                                              vulkan_options.stall_output_files.fragment_shader,
                                                              callback,
                                                              config.print_process_cmd_line);
                            }

                            if (is_comp_shader_present)
                            {
                                KcUtils::PerformStallAnalysis(vulkan_options.isa_disassembly_output_files.compute_shader,
                                                              device_gt_str,
                                                              vulkan_options.stall_output_files.compute_shader,
                                                              callback,
                                                              config.print_process_cmd_line);
                            }

                            // Process stageless files.
                            if (is_pipe_input || !vulkan_options.stageless_input_file.empty())
                            {
                                for (const auto& isa_and_stall_analysis :
                                     {std::pair<gtString, gtString>{vulkan_options.isa_disassembly_output_files.vertex_shader,
                                                                    vulkan_options.stall_output_files.vertex_shader},
                                      std::pair<gtString, gtString>{vulkan_options.isa_disassembly_output_files.tessellation_control_shader,
                                                                    vulkan_options.stall_output_files.tessellation_control_shader},
                                      std::pair<gtString, gtString>{vulkan_options.isa_disassembly_output_files.tessellation_evaluation_shader,
                                                                    vulkan_options.stall_output_files.tessellation_evaluation_shader},
                                      std::pair<gtString, gtString>{vulkan_options.isa_disassembly_output_files.geometry_shader,
                                                                    vulkan_options.stall_output_files.geometry_shader},
                                      std::pair<gtString, gtString>{vulkan_options.isa_disassembly_output_files.fragment_shader,
                                                                    vulkan_options.stall_output_files.fragment_shader},
                                      std::pair<gtString, gtString>{vulkan_options.isa_disassembly_output_files.compute_shader,
                                                                    vulkan_options.stall_output_files.compute_shader}})
                                {
                                    if (KcUtils::FileNotEmpty(isa_and_stall_analysis.first.asASCIICharArray()))
                                    {
                                        KcUtils::PerformStallAnalysis(isa_and_stall_analysis.first,
                                                                      device_gt_str,
                                                                      isa_and_stall_analysis.second,
                                                                      callback,
                                                                      config.print_process_cmd_line);
                                    }
                                }
                            }
                        }
                        else
                        {
                            std::cout << kStrWarningStallAnalysisNotSupportedForRdna << device << std::endl;
                        }
                    }

                    // Generate control flow graph if required.
                    if (is_inst_cfg_required || is_block_cfg_required)
                    {
                        if (is_vert_shader_present || KcUtils::FileNotEmpty(vulkan_options.isa_disassembly_output_files.vertex_shader.asASCIICharArray()))
                        {
                            // If vertex shader is present, or a vertex shader was auto-generated by the offline backend, generate the cfg.
                            KcUtils::GenerateControlFlowGraph(vulkan_options.isa_disassembly_output_files.vertex_shader,
                                                              device_gt_str,
                                                              vulkan_options.cfg_output_files.vertex_shader,
                                                              callback,
                                                              is_inst_cfg_required,
                                                              config.print_process_cmd_line);
                        }

                        if (is_tess_control_shader_present)
                        {
                            // When tessellation is enabled on Vega and Navi, Vertex and Tesc are merged.
                            KcUtils::GenerateControlFlowGraph(vulkan_options.isa_disassembly_output_files.tessellation_control_shader,
                                                              device_gt_str,
                                                              vulkan_options.cfg_output_files.tessellation_control_shader,
                                                              callback,
                                                              is_inst_cfg_required,
                                                              config.print_process_cmd_line);
                        }

                        if (is_tess_evaluation_shader_present)
                        {
                            KcUtils::GenerateControlFlowGraph(vulkan_options.isa_disassembly_output_files.tessellation_evaluation_shader,
                                                              device_gt_str,
                                                              vulkan_options.cfg_output_files.tessellation_evaluation_shader,
                                                              callback,
                                                              is_inst_cfg_required,
                                                              config.print_process_cmd_line);
                        }

                        if (is_geom_shader_present)
                        {
                            // When tessellation evaluation is present on Vega and Navi, GS and Tesc are merged.
                            KcUtils::GenerateControlFlowGraph(vulkan_options.isa_disassembly_output_files.geometry_shader,
                                                              device_gt_str,
                                                              vulkan_options.cfg_output_files.geometry_shader,
                                                              callback,
                                                              is_inst_cfg_required,
                                                              config.print_process_cmd_line);
                        }

                        if (is_frag_shader_present)
                        {
                            KcUtils::GenerateControlFlowGraph(vulkan_options.isa_disassembly_output_files.fragment_shader,
                                                              device_gt_str,
                                                              vulkan_options.cfg_output_files.fragment_shader,
                                                              callback,
                                                              is_inst_cfg_required,
                                                              config.print_process_cmd_line);
                        }

                        if (is_comp_shader_present)
                        {
                            KcUtils::GenerateControlFlowGraph(vulkan_options.isa_disassembly_output_files.compute_shader,
                                                              device_gt_str,
                                                              vulkan_options.cfg_output_files.compute_shader,
                                                              callback,
                                                              is_inst_cfg_required,
                                                              config.print_process_cmd_line);
                        }

                        // Process stageless files.
                        if (is_pipe_input || !vulkan_options.stageless_input_file.empty())
                        {
                            for (const auto& isa_and_cfg : {
                                std::pair<gtString, gtString>{vulkan_options.isa_disassembly_output_files.vertex_shader, vulkan_options.cfg_output_files.vertex_shader},
                                std::pair<gtString, gtString>{vulkan_options.isa_disassembly_output_files.tessellation_control_shader, vulkan_options.cfg_output_files.tessellation_control_shader},
                                std::pair<gtString, gtString>{vulkan_options.isa_disassembly_output_files.tessellation_evaluation_shader, vulkan_options.cfg_output_files.tessellation_evaluation_shader},
                                std::pair<gtString, gtString>{vulkan_options.isa_disassembly_output_files.geometry_shader, vulkan_options.cfg_output_files.geometry_shader},
                                std::pair<gtString, gtString>{vulkan_options.isa_disassembly_output_files.fragment_shader, vulkan_options.cfg_output_files.fragment_shader},
                                std::pair<gtString, gtString>{vulkan_options.isa_disassembly_output_files.compute_shader, vulkan_options.cfg_output_files.compute_shader} })
                            {
                                if (KcUtils::FileNotEmpty(isa_and_cfg.first.asASCIICharArray()))
                                {
                                    KcUtils::GenerateControlFlowGraph(isa_and_cfg.first, device_gt_str, isa_and_cfg.second,
                                        callback, is_inst_cfg_required, config.print_process_cmd_line);
                                }
                            }
                        }
                    }
                }
                else
                {
                    log_msg << kStrInfoFailed << std::endl;

                    switch (compilation_status)
                    {
                    case kBeStatusVulkanAmdspvCompilationFailure:
                        log_msg << build_error_log.asASCIICharArray();
                        break;
                    case kBeStatusVulkanAmdspvLaunchFailure:
                        log_msg << kStrErrorCannotInvokeCompiler << std::endl;
                        break;
                    case kBeStatusVulkanNoInputFile:
                        log_msg << kStrErrorNoInputFile << std::endl;
                        break;
                    case KBeStatusVulkanMixedInputFiles:
                        log_msg << kStrErrorVkOfflineMixedInputFiles << std::endl;
                        break;
                    case kBeStatusFailedOutputVerification:
                        log_msg << kStrErrorOutputFileVerificationFailed << std::endl;
                        break;
                    }
                }

                // Delete temporary files
                if (is_isa_required && config.isa_file.empty())
                {
                    KcUtils::DeletePipelineFiles(vulkan_options.isa_disassembly_output_files);
                }
                if (is_isa_required)
                {
                    // Remove the temporary "ISA binary" files.
                    KcUtils::DeletePipelineFiles(vulkan_options.isa_binary_output_files);
                }

                // Print the message for the current device.
                callback(log_msg.str());

                // Clear the output stream for the next iteration.
                log_msg.str("");
            }
        }
    }
    else
    {
        log_msg << kStrInfoAborting << std::endl;
    }

    // Print the output message.
    if (callback != nullptr)
    {
        callback(log_msg.str());
        callback(warning_msg.str());
    }
}

bool KcCLICommanderVkOffline::PrintAsicList(const Config&)
{
    std::set<std::string> targets;
    return KcUtils::PrintAsicList(targets, kUnsupportedDevicesVkOffline);
}


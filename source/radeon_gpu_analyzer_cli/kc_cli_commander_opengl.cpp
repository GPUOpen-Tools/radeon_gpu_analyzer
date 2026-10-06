//=============================================================================
/// Copyright (c) 2020-2026 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for CLI Commander interface for compiling with the OpenGL Compiler (glc).
//=============================================================================

// C++.
#include <iterator>

// Common.
#include "common/rg_log.h"

// Backend.
#include "DeviceInfo.h"
#include "radeon_gpu_analyzer_backend/be_analysis_summary.h"
#include "radeon_gpu_analyzer_backend/be_backend.h"
#include "radeon_gpu_analyzer_backend/be_isa_spec_metadata.h"
#include "radeon_gpu_analyzer_backend/be_utils.h"
// Infra.
#include "external/amdt_base_tools/Include/gtAssert.h"
#include "external/amdt_os_wrappers/Include/osFilePath.h"
#include "external/amdt_os_wrappers/Include/osFile.h"

// Local.
#include "radeon_gpu_analyzer_cli/kc_cli_commander_opengl.h"
#include "radeon_gpu_analyzer_cli/kc_cli_isa_spec_loader.h"
#include "radeon_gpu_analyzer_cli/kc_cli_string_constants.h"
#include "radeon_gpu_analyzer_cli/kc_statistics_parser_opengl.h"
#include "radeon_gpu_analyzer_cli/kc_utils.h"
#include "radeon_gpu_analyzer_cli/kc_xml_writer.h"

// Constants.
static const char* kStrErrorOpenglIsaParsingFailed = "Error: failed to parse ISA into CSV.";
static const char* kStrErrorOpenglCannotExtractVersion = "Error: unable to extract the OpenGL version.";
static const char* kStrErrNoInputFile = "Error: no input file received.";

// Unsupported devices.
static const std::set<std::string> kUnsupportedDevicesOpengl = {"gfx900",
                                                                "gfx902",
                                                                "gfx904",
                                                                "gfx906",
                                                                "gfx908",
                                                                "gfx90a",
                                                                "gfx90c",
                                                                "gfx942",
                                                                "gfx950",
                                                                "gfx1010",
                                                                "gfx1011",
                                                                "gfx1012",
                                                                "gfx1030",
                                                                "gfx1031",
                                                                "gfx1032",
                                                                "gfx1033",
                                                                "gfx1034",
                                                                "gfx1035", 
                                                                "gfx1250"};

void KcCliCommanderOpenGL::CreateStatisticsFile(const gtString&         statistics_file,
                                 const Config&           config,
                                 const std::string&      device,
                                 IStatisticsParser&      stats_parser,
                                 LoggingCallbackFunction log_callback)
{
    // Parse the backend statistics.
    beKA::AnalysisData statistics;
    stats_parser.ParseStatistics(device, statistics_file, statistics);

    // Set available-register values that the backend does not populate.
    statistics.num_vgprs_available = 256;
    statistics.num_sgprs_available = 106;
    statistics.lds_size_available  = 65536;

    // Delete the older statistics file.
    DeleteFile(statistics_file);

    KcUtils::CreateStatisticsFile(statistics_file, config, device, statistics, log_callback);
}

bool KcCliCommanderOpenGL::DeleteFile(const gtString& file_full_path)
{
    bool       ret = false;
    osFilePath path(file_full_path);

    if (path.exists())
    {
        osFile file(path);
        ret = file.deleteFile();
    }
    return ret;
}

struct KcCliCommanderOpenGL::OpenglDeviceInfo
{
    OpenglDeviceInfo() : device_family_id(0), device_id(0) {}

    OpenglDeviceInfo(size_t chip_family, size_t chip_revision) :
        device_family_id(chip_family), device_id(chip_revision) {}

    // HW family id.
    size_t device_family_id;

    // Chip id
    size_t device_id;
};

KcCliCommanderOpenGL::KcCliCommanderOpenGL() : ogl_builder_(new BeProgramBuilderOpengl)
{
}

KcCliCommanderOpenGL::~KcCliCommanderOpenGL()
{
    delete ogl_builder_;
    ogl_builder_ = nullptr;
}

void KcCliCommanderOpenGL::Version(Config& config, LoggingCallbackFunction callback)
{
    GT_UNREFERENCED_PARAMETER(config);

    KcCliCommander::Version(config, callback);

    //if (ogl_builder_ != nullptr)
    //{
    //    gtString gl_version;
    //    bool rc = ogl_builder_->GetOpenGLVersion(config.print_process_cmd_line, gl_version);
    //    callback(((rc && !gl_version.isEmpty()) ? std::string(gl_version.asASCIICharArray()) : kStrErrorOpenglCannotExtractVersion) + "\n");
    //}
}

bool KcCliCommanderOpenGL::PrintAsicList(const Config&)
{
    std::set<std::string> targets;
    return KcUtils::PrintAsicList(targets, kUnsupportedDevicesOpengl);
}

// Helper function to remove unnecessary file paths.
static bool GenerateRenderingPipelineOutputPaths(const Config& config, const std::string& base_output_filename, const std::string& default_suffix,
                                                 const std::string& default_ext, const std::string& device, BeProgramPipeline& pipeline_to_adjust)
{
    // Generate the output file paths.
    bool  ret = KcUtils::AdjustRenderingPipelineOutputFileNames(base_output_filename, default_suffix, default_ext, device, pipeline_to_adjust);

    if (ret)
    {
        // Clear irrelevant paths.
        bool is_vert_shader_present = (!config.vertex_shader.empty());
        bool is_tess_control_shader_present = (!config.tess_control_shader.empty());
        bool is_tess_evaluation_shader_present = (!config.tess_evaluation_shader.empty());
        bool is_geom_shader_present = (!config.geometry_shader.empty());
        bool is_frag_shader_present = (!config.fragment_shader.empty());
        bool is_comp_shader_present = (!config.compute_shader.empty());

        if (!is_vert_shader_present)
        {
            pipeline_to_adjust.vertex_shader.makeEmpty();
        }
        else
        {
            BeUtils::DeleteFileFromDisk(pipeline_to_adjust.vertex_shader);
        }

        if (!is_tess_control_shader_present)
        {
            pipeline_to_adjust.tessellation_control_shader.makeEmpty();
        }
        else
        {
            BeUtils::DeleteFileFromDisk(pipeline_to_adjust.tessellation_control_shader);
        }

        if (!is_tess_evaluation_shader_present)
        {
            pipeline_to_adjust.tessellation_evaluation_shader.makeEmpty();
        }
        else
        {
            BeUtils::DeleteFileFromDisk(pipeline_to_adjust.tessellation_evaluation_shader);
        }

        if (!is_geom_shader_present)
        {
            pipeline_to_adjust.geometry_shader.makeEmpty();
        }
        else
        {
            BeUtils::DeleteFileFromDisk(pipeline_to_adjust.geometry_shader);
        }

        if (!is_frag_shader_present)
        {
            pipeline_to_adjust.fragment_shader.makeEmpty();
        }
        else
        {
            BeUtils::DeleteFileFromDisk(pipeline_to_adjust.fragment_shader);
        }

        if (!is_comp_shader_present)
        {
            pipeline_to_adjust.compute_shader.makeEmpty();
        }
        else
        {
            BeUtils::DeleteFileFromDisk(pipeline_to_adjust.compute_shader);
        }
    }

    return ret;
}

void KcCliCommanderOpenGL::RunCompileCommands(const Config& config, LoggingCallbackFunction callback)
{
    // Output stream.
    std::stringstream log_msg;

    // Input validation.
    bool should_abort = false;
    bool status = true;
    bool is_vert_shader_present = (!config.vertex_shader.empty());
    bool is_tess_control_shader_present = (!config.tess_control_shader.empty());
    bool is_tess_evaluation_shader_present = (!config.tess_evaluation_shader.empty());
    bool is_geom_shader_present = (!config.geometry_shader.empty());
    bool is_frag_shader_present = (!config.fragment_shader.empty());
    bool is_comp_shader_present = (!config.compute_shader.empty());
    bool is_isa_required = (!config.isa_file.empty() || 
        !config.livereg_analysis_file.empty() || 
        !config.sgpr_livereg_analysis_file.empty() ||
        !config.block_cfg_file.empty() || 
        !config.inst_cfg_file.empty() || 
        !config.analysis_file.empty());
    bool is_il_required = !config.il_file.empty();
    bool is_livereg_analysis_required = (!config.livereg_analysis_file.empty());
    bool is_livereg_sgpr_analysis_required = (!config.sgpr_livereg_analysis_file.empty());
    bool is_block_cfg_required = (!config.block_cfg_file.empty());
    bool is_inst_cfg_required = (!config.inst_cfg_file.empty());
    bool is_isa_binary = (!config.binary_output_file.empty());
    bool is_stats_required = (!config.analysis_file.empty());

    // Fatal error. This should not happen unless we have an allocation problem.
    if (ogl_builder_ == nullptr)
    {
        should_abort = true;
        log_msg << kStrErrorMemoryAllocationFailure << std::endl;
    }

    // We must have a single input file at least.
    bool is_input_present = is_comp_shader_present || is_vert_shader_present || is_tess_control_shader_present ||
        is_tess_evaluation_shader_present || is_geom_shader_present || is_frag_shader_present;

    if (!is_input_present)
    {
        std::cout << kStrErrNoInputFile << std::endl;
        should_abort = true;
    }

    // Cannot mix compute and non-compute shaders.
    if (!should_abort && is_comp_shader_present && (is_vert_shader_present || is_tess_control_shader_present ||
                                   is_tess_evaluation_shader_present || is_geom_shader_present || is_frag_shader_present))
    {
        log_msg << kStrErrorGraphicsComputeMix << std::endl;
        should_abort = true;
    }

    // Options to be passed to the backend.
    OpenglOptions gl_options;

    // Validate the input shaders.
    if (!should_abort && is_vert_shader_present)
    {
        should_abort = !KcUtils::ValidateShaderFileName(kStrVertexStage, config.vertex_shader, log_msg);
        gl_options.pipeline_shaders.vertex_shader << config.vertex_shader.c_str();
    }

    if (!should_abort && is_tess_control_shader_present)
    {
        should_abort = !KcUtils::ValidateShaderFileName(kStrTessellationControlStageName, config.tess_control_shader, log_msg);
        gl_options.pipeline_shaders.tessellation_control_shader << config.tess_control_shader.c_str();
    }

    if (!should_abort && is_tess_evaluation_shader_present)
    {
        should_abort = !KcUtils::ValidateShaderFileName(kStrTessellationEvaluationStageName, config.tess_evaluation_shader, log_msg);
        gl_options.pipeline_shaders.tessellation_evaluation_shader << config.tess_evaluation_shader.c_str();
    }

    if (!should_abort && is_geom_shader_present)
    {
        should_abort = !KcUtils::ValidateShaderFileName(kStrGeometryStageName, config.geometry_shader, log_msg);
        gl_options.pipeline_shaders.geometry_shader << config.geometry_shader.c_str();
    }

    if (!should_abort && is_frag_shader_present)
    {
        should_abort = !KcUtils::ValidateShaderFileName(kStrFragmentStageName, config.fragment_shader, log_msg);
        gl_options.pipeline_shaders.fragment_shader << config.fragment_shader.c_str();
    }

    if (!should_abort && is_comp_shader_present)
    {
        should_abort = !KcUtils::ValidateShaderFileName(kStrComputeStageName, config.compute_shader, log_msg);
        gl_options.pipeline_shaders.compute_shader << config.compute_shader.c_str();
    }

    // Validate the output directories.
    if (!should_abort && is_isa_required && !config.isa_file.empty())
    {
        should_abort = !KcUtils::ValidateShaderOutputDir(config.isa_file, log_msg);
    }

    if (!should_abort && is_il_required && !config.il_file.empty())
    {
        should_abort = !KcUtils::ValidateShaderOutputDir(config.il_file, log_msg);
    }

    if (!should_abort && is_livereg_analysis_required)
    {
        should_abort = !KcUtils::ValidateShaderOutputDir(config.livereg_analysis_file, log_msg);
    }

    if (!should_abort && is_livereg_sgpr_analysis_required)
    {
        should_abort = !KcUtils::ValidateShaderOutputDir(config.sgpr_livereg_analysis_file, log_msg);
    }

    if (!should_abort && is_block_cfg_required)
    {
        should_abort = !KcUtils::ValidateShaderOutputDir(config.block_cfg_file, log_msg);
    }

    if (!should_abort && is_inst_cfg_required)
    {
        should_abort = !KcUtils::ValidateShaderOutputDir(config.inst_cfg_file, log_msg);
    }

    if (!should_abort && is_stats_required)
    {
        should_abort = !KcUtils::ValidateShaderOutputDir(config.analysis_file, log_msg);
    }

    if (!should_abort && is_isa_binary)
    {
        should_abort = !KcUtils::ValidateShaderOutputDir(config.binary_output_file, log_msg);
    }

    if (!should_abort)
    {
        // Set the log callback for the backend.
        log_callback_ = callback;
        ogl_builder_->SetLog(callback);

        std::set<std::string> target_devices;

        if (supported_devices_cache_.empty())
        {
            // We need to populate the list of supported devices.
            bool is_device_list_extracted = BeProgramBuilderOpengl::GetSupportedDevices(supported_devices_cache_);
            if (!is_device_list_extracted)
            {
                std::stringstream err_msg;
                err_msg << kStrErrorCannotExtractSupportedDeviceList << std::endl;
                should_abort = true;
            }
        }

        if (!should_abort)
        {
            InitRequestedAsicList(config.asics, config.mode, supported_devices_cache_, target_devices, false);

            // Sort the targets.
            std::set<std::string, decltype(&BeUtils::DeviceNameLessThan)> sorted_unique_names(target_devices.begin(),
                target_devices.end(), BeUtils::DeviceNameLessThan);

            if (!config.session_summary_file.empty())
            {
                std::vector<std::string> asic_names(sorted_unique_names.begin(), sorted_unique_names.end());
                KcCliIsaSpecLoader::LoadIsaSpecsFromXML(true, config.include_target_metadata, asic_names);
            }

            for (const std::string& device : sorted_unique_names)
            {
                if (!should_abort && kUnsupportedDevicesOpengl.find(device) == kUnsupportedDevicesOpengl.end())
                {
                    // Generate the output message.
                    log_msg << kStrInfoCompiling << device << "... ";

                    gtString device_gt_str;
                    device_gt_str << device.c_str();

                    // Set the target device info for the backend.
                    gl_options.device_name = device;

                    // Adjust the output file names to the device and shader type.
                    if (is_isa_required)
                    {
                        gl_options.is_amd_isa_disassembly_required = true;
                        std::string adjustedIsaFileName = KcUtils::AdjustBaseFileNameIsaDisassembly(config.isa_file, device);
                        status &= GenerateRenderingPipelineOutputPaths(config, adjustedIsaFileName, "", kStrDefaultExtensionIsa, device, gl_options.isa_disassembly_output_files);
                    }

                    if (is_il_required)
                    {
                        gl_options.is_il_disassembly_required = true;
                        std::string adjustedIsaFileName = KcUtils::AdjustBaseFileNameIlDisassembly(config.il_file, device);
                        status &= GenerateRenderingPipelineOutputPaths(config, adjustedIsaFileName, "", kStrDefaultExtensionAmdil, device, gl_options.il_disassembly_output_files);
                    }

                    if (is_livereg_analysis_required)
                    {
                        gl_options.is_livereg_required = true;
                        std::string adjustedIsaFileName = KcUtils::AdjustBaseFileNameLivereg(config.livereg_analysis_file, device);
                        status &= GenerateRenderingPipelineOutputPaths(config, adjustedIsaFileName, kStrDefaultExtensionLivereg,
                            kStrDefaultExtensionText, device, gl_options.livereg_output_files);
                    }

                    if (is_livereg_sgpr_analysis_required)
                    {
                        gl_options.is_livereg_sgpr_required  = true;
                        std::string adjustedIsaFileName     = KcUtils::AdjustBaseFileNameLiveregSgpr(config.sgpr_livereg_analysis_file, device);
                        status &= GenerateRenderingPipelineOutputPaths(
                            config, adjustedIsaFileName, kStrDefaultExtensionLiveregSgpr, kStrDefaultExtensionText, device, gl_options.livereg_sgpr_output_files);
                    }

                    if (is_block_cfg_required)
                    {
                        gl_options.is_cfg_required = true;
                        std::string adjustedIsaFileName = KcUtils::AdjustBaseFileNameCfg(config.block_cfg_file, device);
                        status &= GenerateRenderingPipelineOutputPaths(config, adjustedIsaFileName, KC_STR_DEFAULT_CFG_SUFFIX,
                                                                       kStrDefaultExtensionDot, device, gl_options.cfg_output_files);
                    }

                    if (is_inst_cfg_required)
                    {
                        gl_options.is_cfg_required = true;
                        std::string adjustedIsaFileName = KcUtils::AdjustBaseFileNameCfg(config.inst_cfg_file, device);
                        status &= GenerateRenderingPipelineOutputPaths(config, adjustedIsaFileName, KC_STR_DEFAULT_CFG_SUFFIX,
                                                                       kStrDefaultExtensionDot, device, gl_options.cfg_output_files);
                    }

                    if (is_stats_required)
                    {
                        gl_options.is_stats_required = true;
                        std::string adjustedIsaFileName = KcUtils::AdjustBaseFileNameStats(config.analysis_file, device);
                        status &= GenerateRenderingPipelineOutputPaths(config, adjustedIsaFileName, kStrDefaultExtensionStats,
                                                                       kStrDefaultExtensionText, device, gl_options.stats_output_files);
                    }

                    if (is_isa_binary)
                    {
                        gl_options.is_pipeline_binary_required  = true;
                        std::string adjustedIsaFileName = KcUtils::AdjustBaseFileNameBinary(config.binary_output_file, device);
                        KcUtils::ConstructOutputFileName(adjustedIsaFileName, "", kStrDefaultExtensionBin,
                                                         "", device, gl_options.program_binary_filename);
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
                    gtString glc_output;
                    gtString build_log;
                    beKA::beStatus buildStatus = ogl_builder_->Compile(gl_options, should_cancel, config.print_process_cmd_line, glc_output, build_log);
                    if (buildStatus == beKA::beStatus::kBeStatusSuccess)
                    {
                        log_msg << kStrInfoSuccess << std::endl;

                        // Parse and replace the statistics files.
                        beKA::AnalysisData statistics;
                        KcOpenGLStatisticsParser stats_parser;

                        // Parse ISA and write it to a csv file if required.
                        if (is_isa_required && config.is_parsed_isa_required)
                        {
                            std::string  isa_text, parsed_isa_text, parsed_isa_file_name;
                            BeProgramPipeline  isa_files = gl_options.isa_disassembly_output_files;
                            for (const gtString& isa_filename : { isa_files.compute_shader, isa_files.fragment_shader, isa_files.geometry_shader,
                                                                isa_files.tessellation_control_shader, isa_files.tessellation_evaluation_shader, isa_files.vertex_shader })
                            {
                                if (!isa_filename.isEmpty())
                                {
                                    if ((status = KcUtils::ReadTextFile(isa_filename.asASCIICharArray(), isa_text, log_callback_)) == true)
                                    {
                                        status = KcUtils::GetParsedIsaCsvText(isa_text, device, config.is_line_numbers_required, parsed_isa_text);
                                        if (status)
                                        {
                                            status = KcUtils::GetParsedISAFileName(isa_filename.asASCIICharArray(), parsed_isa_file_name);
                                        }
                                        if (status)
                                        {
                                            status = KcUtils::WriteTextFile(parsed_isa_file_name, parsed_isa_text, log_callback_);
                                        }
                                    }
                                    if (!status)
                                    {
                                        log_msg << kStrErrorOpenglIsaParsingFailed << std::endl;
                                    }
                                }
                            }
                        }

                        if (is_stats_required)
                        {
                            if (is_vert_shader_present)
                            {
                                CreateStatisticsFile(gl_options.stats_output_files.vertex_shader, config, device, stats_parser, callback);
                            }

                            if (is_tess_control_shader_present)
                            {
                                CreateStatisticsFile(gl_options.stats_output_files.tessellation_control_shader, config, device, stats_parser, callback);
                            }

                            if (is_tess_evaluation_shader_present)
                            {
                                CreateStatisticsFile(gl_options.stats_output_files.tessellation_evaluation_shader, config, device, stats_parser, callback);
                            }

                            if (is_geom_shader_present)
                            {
                                CreateStatisticsFile(gl_options.stats_output_files.geometry_shader, config, device, stats_parser, callback);
                            }

                            if (is_frag_shader_present)
                            {
                                CreateStatisticsFile(gl_options.stats_output_files.fragment_shader, config, device, stats_parser, callback);
                            }

                            if (is_comp_shader_present)
                            {
                                CreateStatisticsFile(gl_options.stats_output_files.compute_shader, config, device, stats_parser, callback);
                            }
                        }

                        // Perform live register analysis if required.
                        if (is_livereg_analysis_required)
                        {
                            if (is_vert_shader_present)
                            {
                                KcUtils::PerformLiveRegisterAnalysis(gl_options.isa_disassembly_output_files.vertex_shader,
                                                                     device_gt_str,
                                                                     gl_options.livereg_output_files.vertex_shader,
                                                                     callback,
                                                                     config.print_process_cmd_line);
                            }

                            if (is_tess_control_shader_present)
                            {
                                KcUtils::PerformLiveRegisterAnalysis(gl_options.isa_disassembly_output_files.tessellation_control_shader,
                                                                     device_gt_str,
                                                                     gl_options.livereg_output_files.tessellation_control_shader,
                                                                     callback,
                                                                     config.print_process_cmd_line);
                            }

                            if (is_tess_evaluation_shader_present)
                            {
                                KcUtils::PerformLiveRegisterAnalysis(gl_options.isa_disassembly_output_files.tessellation_evaluation_shader,
                                                                     device_gt_str,
                                                                     gl_options.livereg_output_files.tessellation_evaluation_shader,
                                                                     callback,
                                                                     config.print_process_cmd_line);
                            }

                            if (is_geom_shader_present)
                            {
                                KcUtils::PerformLiveRegisterAnalysis(gl_options.isa_disassembly_output_files.geometry_shader,
                                                                     device_gt_str,
                                                                     gl_options.livereg_output_files.geometry_shader,
                                                                     callback,
                                                                     config.print_process_cmd_line);
                            }

                            if (is_frag_shader_present)
                            {
                                KcUtils::PerformLiveRegisterAnalysis(gl_options.isa_disassembly_output_files.fragment_shader,
                                                                     device_gt_str,
                                                                     gl_options.livereg_output_files.fragment_shader,
                                                                     callback,
                                                                     config.print_process_cmd_line);
                            }

                            if (is_comp_shader_present)
                            {
                                KcUtils::PerformLiveRegisterAnalysis(gl_options.isa_disassembly_output_files.compute_shader,
                                                                     device_gt_str,
                                                                     gl_options.livereg_output_files.compute_shader,
                                                                     callback,
                                                                     config.print_process_cmd_line);
                            }
                        }

                        // Perform live register analysis (sgpr) if required.
                        if (is_livereg_sgpr_analysis_required)
                        {
                            if (is_vert_shader_present)
                            {
                                KcUtils::PerformLiveRegisterAnalysis(gl_options.isa_disassembly_output_files.vertex_shader,
                                                                     device_gt_str,
                                                                     gl_options.livereg_sgpr_output_files.vertex_shader,
                                                                     callback,
                                                                     config.print_process_cmd_line, true);
                            }

                            if (is_tess_control_shader_present)
                            {
                                KcUtils::PerformLiveRegisterAnalysis(gl_options.isa_disassembly_output_files.tessellation_control_shader,
                                                                     device_gt_str,
                                                                     gl_options.livereg_sgpr_output_files.tessellation_control_shader,
                                                                     callback,
                                                                     config.print_process_cmd_line, true);
                            }

                            if (is_tess_evaluation_shader_present)
                            {
                                KcUtils::PerformLiveRegisterAnalysis(gl_options.isa_disassembly_output_files.tessellation_evaluation_shader,
                                                                     device_gt_str,
                                                                     gl_options.livereg_sgpr_output_files.tessellation_evaluation_shader,
                                                                     callback,
                                                                     config.print_process_cmd_line, true);
                            }

                            if (is_geom_shader_present)
                            {
                                KcUtils::PerformLiveRegisterAnalysis(gl_options.isa_disassembly_output_files.geometry_shader,
                                                                     device_gt_str,
                                                                     gl_options.livereg_sgpr_output_files.geometry_shader,
                                                                     callback,
                                                                     config.print_process_cmd_line, true);
                            }

                            if (is_frag_shader_present)
                            {
                                KcUtils::PerformLiveRegisterAnalysis(gl_options.isa_disassembly_output_files.fragment_shader,
                                                                     device_gt_str,
                                                                     gl_options.livereg_sgpr_output_files.fragment_shader,
                                                                     callback,
                                                                     config.print_process_cmd_line, true);
                            }

                            if (is_comp_shader_present)
                            {
                                KcUtils::PerformLiveRegisterAnalysis(gl_options.isa_disassembly_output_files.compute_shader,
                                                                     device_gt_str,
                                                                     gl_options.livereg_sgpr_output_files.compute_shader,
                                                                     callback,
                                                                     config.print_process_cmd_line, true);
                            }
                        }

                        // Generate control flow graph if required.
                        if (is_block_cfg_required || is_inst_cfg_required)
                        {
                            if (is_vert_shader_present)
                            {
                                KcUtils::GenerateControlFlowGraph(gl_options.isa_disassembly_output_files.vertex_shader,
                                                                  device_gt_str,
                                                                  gl_options.cfg_output_files.vertex_shader,
                                                                  callback,
                                                                  is_inst_cfg_required,
                                                                  config.print_process_cmd_line);
                            }

                            if (is_tess_control_shader_present)
                            {
                                KcUtils::GenerateControlFlowGraph(gl_options.isa_disassembly_output_files.tessellation_control_shader,
                                                                  device_gt_str,
                                                                  gl_options.cfg_output_files.tessellation_control_shader,
                                                                  callback,
                                                                  is_inst_cfg_required,
                                                                  config.print_process_cmd_line);
                            }

                            if (is_tess_control_shader_present)
                            {
                                KcUtils::GenerateControlFlowGraph(gl_options.isa_disassembly_output_files.tessellation_evaluation_shader,
                                                                  device_gt_str,
                                                                  gl_options.cfg_output_files.tessellation_evaluation_shader,
                                                                  callback,
                                                                  is_inst_cfg_required,
                                                                  config.print_process_cmd_line);
                            }

                            if (is_geom_shader_present)
                            {
                                KcUtils::GenerateControlFlowGraph(gl_options.isa_disassembly_output_files.geometry_shader,
                                                                  device_gt_str,
                                                                  gl_options.cfg_output_files.geometry_shader,
                                                                  callback,
                                                                  is_inst_cfg_required,
                                                                  config.print_process_cmd_line);
                            }

                            if (is_frag_shader_present)
                            {
                                KcUtils::GenerateControlFlowGraph(gl_options.isa_disassembly_output_files.fragment_shader,
                                                                  device_gt_str,
                                                                  gl_options.cfg_output_files.fragment_shader,
                                                                  callback,
                                                                  is_inst_cfg_required,
                                                                  config.print_process_cmd_line);
                            }

                            if (is_comp_shader_present)
                            {
                                KcUtils::GenerateControlFlowGraph(gl_options.isa_disassembly_output_files.compute_shader,
                                                                  device_gt_str,
                                                                  gl_options.cfg_output_files.compute_shader,
                                                                  callback,
                                                                  is_inst_cfg_required,
                                                                  config.print_process_cmd_line);
                            }
                        }

                        // Print GLC's output string if user requests verbose output.
                        if (!glc_output.isEmpty() && config.print_process_cmd_line)
                        {
                            log_msg << glc_output.asASCIICharArray() << std::endl;
                        }

                        if (!config.session_summary_file.empty())
                        {
                            StoreOutputFilesToOutputMD(device, gl_options, config);
                        }
                    }
                    else
                    {
                        log_msg << kStrInfoFailed << std::endl;
                        if (buildStatus == beKA::kBeStatusOpenglVirtualContextLaunchFailed)
                        {
                            log_msg << kStrErrorCannotInvokeCompiler << std::endl;
                        }
                        else if (buildStatus == beKA::kBeStatusFailedOutputVerification)
                        {
                            log_msg << kStrErrorOutputFileVerificationFailed << std::endl;
                        }

                        // Notify the user about build errors in GLC's output if any.
                        if (!glc_output.isEmpty())
                        {
                            log_msg << glc_output.asASCIICharArray() << std::endl;
                        }
                    }

                    // Delete temporary files
                    if (is_isa_required && config.isa_file.empty())
                    {
                        KcUtils::DeletePipelineFiles(gl_options.isa_disassembly_output_files);
                    }

                    // Print the message for the current device.
                    callback(log_msg.str());

                    // Clear the output stream for the next iteration.
                    log_msg.str("");
                }
            }

            // Generate the XML session summary if requested.
            if (!config.session_summary_file.empty())
            {
                GenerateSessionSummary(config);
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
    }
}

void KcCliCommanderOpenGL::StoreOutputFilesToOutputMD(const std::string& device, const OpenglOptions& gl_options, const Config& config)
{
    // GL stage index mapping (mesh/task are not supported in OpenGL).
    static constexpr RgaEntryType kGlStageEntryTypes[BePipelineStage::kCount] = {
        RgaEntryType::kGlVertex,
        RgaEntryType::kGlTessControl,
        RgaEntryType::kGlTessEval,
        RgaEntryType::kGlGeometry,
        RgaEntryType::kGlFragment,
        RgaEntryType::kGlCompute,
        RgaEntryType::kUnknown,  // Mesh shader - not supported in OpenGL.
        RgaEntryType::kUnknown   // Task shader - not supported in OpenGL.
    };

    const std::string* kGlStageInputFiles[BePipelineStage::kCount] = {
        &config.vertex_shader,
        &config.tess_control_shader,
        &config.tess_evaluation_shader,
        &config.geometry_shader,
        &config.fragment_shader,
        &config.compute_shader,
        nullptr,  // mesh
        nullptr   // task
    };

    const gtString* kGlStageIsaFiles[BePipelineStage::kCount] = {
        &gl_options.isa_disassembly_output_files.vertex_shader,
        &gl_options.isa_disassembly_output_files.tessellation_control_shader,
        &gl_options.isa_disassembly_output_files.tessellation_evaluation_shader,
        &gl_options.isa_disassembly_output_files.geometry_shader,
        &gl_options.isa_disassembly_output_files.fragment_shader,
        &gl_options.isa_disassembly_output_files.compute_shader,
        nullptr,  // mesh
        nullptr   // task
    };

    const gtString* kGlStageStatsFiles[BePipelineStage::kCount] = {
        &gl_options.stats_output_files.vertex_shader,
        &gl_options.stats_output_files.tessellation_control_shader,
        &gl_options.stats_output_files.tessellation_evaluation_shader,
        &gl_options.stats_output_files.geometry_shader,
        &gl_options.stats_output_files.fragment_shader,
        &gl_options.stats_output_files.compute_shader,
        nullptr,  // mesh
        nullptr   // task
    };

    const gtString* kGlStageLiveregFiles[BePipelineStage::kCount] = {
        &gl_options.livereg_output_files.vertex_shader,
        &gl_options.livereg_output_files.tessellation_control_shader,
        &gl_options.livereg_output_files.tessellation_evaluation_shader,
        &gl_options.livereg_output_files.geometry_shader,
        &gl_options.livereg_output_files.fragment_shader,
        &gl_options.livereg_output_files.compute_shader,
        nullptr,  // mesh
        nullptr   // task
    };

    const gtString* kGlStageLiveregSgprFiles[BePipelineStage::kCount] = {
        &gl_options.livereg_sgpr_output_files.vertex_shader,
        &gl_options.livereg_sgpr_output_files.tessellation_control_shader,
        &gl_options.livereg_sgpr_output_files.tessellation_evaluation_shader,
        &gl_options.livereg_sgpr_output_files.geometry_shader,
        &gl_options.livereg_sgpr_output_files.fragment_shader,
        &gl_options.livereg_sgpr_output_files.compute_shader,
        nullptr,  // mesh
        nullptr   // task
    };

    const gtString* kGlStageCfgFiles[BePipelineStage::kCount] = {
        &gl_options.cfg_output_files.vertex_shader,
        &gl_options.cfg_output_files.tessellation_control_shader,
        &gl_options.cfg_output_files.tessellation_evaluation_shader,
        &gl_options.cfg_output_files.geometry_shader,
        &gl_options.cfg_output_files.fragment_shader,
        &gl_options.cfg_output_files.compute_shader,
        nullptr,  // mesh
        nullptr   // task
    };

    if (output_metadata_.find(device) == output_metadata_.end())
    {
        RgVkOutputMetadata md;
        RgOutputFiles      out_files(device);
        md.fill(out_files);
        output_metadata_[device] = md;
    }

    RgVkOutputMetadata& device_md = output_metadata_[device];
    for (int stage = 0; stage < BePipelineStage::kCount; stage++)
    {
        if (kGlStageIsaFiles[stage] == nullptr || kGlStageIsaFiles[stage]->isEmpty())
        {
            continue;
        }

        std::string isa_csv_filename;
        if (config.is_parsed_isa_required)
        {
            KcUtils::GetParsedISAFileName(kGlStageIsaFiles[stage]->asASCIICharArray(), isa_csv_filename);
        }

        RgOutputFiles& stage_md    = device_md[stage];
        stage_md.entry_type        = kGlStageEntryTypes[stage];
        stage_md.device            = device;
        stage_md.isa_file          = kGlStageIsaFiles[stage]->asASCIICharArray();
        stage_md.isa_csv_file      = isa_csv_filename;
        stage_md.stats_file        = (kGlStageStatsFiles[stage] && !kGlStageStatsFiles[stage]->isEmpty())
                                         ? kGlStageStatsFiles[stage]->asASCIICharArray()
                                         : "";
        stage_md.livereg_file      = (kGlStageLiveregFiles[stage] && !kGlStageLiveregFiles[stage]->isEmpty())
                                         ? kGlStageLiveregFiles[stage]->asASCIICharArray()
                                         : "";
        stage_md.livereg_sgpr_file = (kGlStageLiveregSgprFiles[stage] && !kGlStageLiveregSgprFiles[stage]->isEmpty())
                                         ? kGlStageLiveregSgprFiles[stage]->asASCIICharArray()
                                         : "";
        stage_md.cfg_file          = (kGlStageCfgFiles[stage] && !kGlStageCfgFiles[stage]->isEmpty())
                                         ? kGlStageCfgFiles[stage]->asASCIICharArray()
                                         : "";
        stage_md.input_file        = (kGlStageInputFiles[stage] && !kGlStageInputFiles[stage]->empty())
                                         ? *kGlStageInputFiles[stage]
                                         : "";
        stage_md.wave_size         = beWaveSize::kWave64;
    }
}

bool KcCliCommanderOpenGL::GenerateSessionSummary(const Config& config)
{
    bool ret = !config.session_summary_file.empty() && !output_metadata_.empty();
    if (ret)
    {
        RgaAnalysisSummary summary;

        for (const auto& [device, stage_array] : output_metadata_)
        {
            RgaAnalysisSummary::AnalysisResult result;
            result.target_architecture_ = device;
            if (config.include_target_metadata)
            {
                beKA::BeIsaSpecExplorer::PopulateFromSpec(device, result.target_architecture_metadata_);
            }
            result.output_.api_ = kStrRgaModeOpengl;

            for (int stage = 0; stage < BePipelineStage::kCount; stage++)
            {
                const RgOutputFiles& out = stage_array[stage];
                if (out.isa_file.empty())
                {
                    continue;
                }
                if (!out.input_file.empty())
                {
                    result.inputs_.inputs_.push_back(out.input_file);
                }
                std::string stage_name;
                RgaEntryTypeUtils::GetEntryTypeStr(out.entry_type, stage_name);
                RgaAnalysisSummary::Kernel kernel;
                if (KcUtils::GenerateKernelSummary(device, stage_name, out, kernel, log_callback_, config.print_process_cmd_line))
                {
                    kernel.kernel_id_ = static_cast<int>(result.output_.kernels_.size());
                    result.output_.kernels_.push_back(std::move(kernel));
                }
            }

            if (result.inputs_.inputs_.empty())
            {
                result.inputs_.inputs_ = config.input_files;
            }

            if (!result.output_.kernels_.empty())
            {
                summary.results_.push_back(std::move(result));
            }
        }

        ret = KcXmlWriter::WriteAnalysisSummaryToFile(summary, config.session_summary_file);
        if (!ret)
        {
            RgLog::stdOut << kStrErrorFailedToGenerateSessionSummary << std::endl;
        }
    }
    return ret;
}


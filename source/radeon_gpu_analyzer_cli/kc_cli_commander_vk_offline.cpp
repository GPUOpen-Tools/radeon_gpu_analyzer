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
#include "radeon_gpu_analyzer_cli/kc_cli_commander_vulkan_util.h"

// Backend.
#include "radeon_gpu_analyzer_backend/be_program_builder_vulkan.h"
#include "radeon_gpu_analyzer_backend/be_backend.h"
#include "radeon_gpu_analyzer_backend/be_utils.h"

// Shared between CLI and GUI.
#include "common/rga_version_info.h"

// Infra.
#include "external/amdt_os_wrappers/Include/osFilePath.h"
#include "external/amdt_os_wrappers/Include/osDirectory.h"



// Constants: error messages.
static const char* kStrErrorVkOfflineCannotExtractVulkanVersion = "Error: unable to extract the Vulkan version.";
static const char* kStrErrorVkOfflineMixedInputFiles = "Error: cannot mix stage-specific input files (--vert, --tesc, --tese, --geom, --frag, --comp) with a stage-less SPIR-V input file.";

// Unsupported devices.
static const std::set<std::string> kUnsupportedDevicesVkOffline = {"gfx908"};

// Suffixes for stage-specific output files.
static const std::array<std::string, BePipelineStage::kCount> kVulkanStageFileSuffixDefault = {"vert", "tesc", "tese", "geom", "frag", "comp"};

// Output metadata entry types for OpenGL pipeline stages.
static const std::array<RgEntryType, BePipelineStage::kCount> kVulkanOglStageEntryTypes = {RgEntryType::kGlVertex,
                                                                                           RgEntryType::kGlTessControl,
                                                                                           RgEntryType::kGlTessEval,
                                                                                           RgEntryType::kGlGeometry,
                                                                                           RgEntryType::kGlFragment,
                                                                                           RgEntryType::kGlCompute};
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

bool IsIsaRequired(const Config& config)
{
    return (!config.isa_file.empty() 
        || !config.livereg_analysis_file.empty() 
        || !config.block_cfg_file.empty() 
        || !config.inst_cfg_file.empty() 
        || !config.analysis_file.empty());
}

bool IsInputFileExtGlsl(const std::string& file_path)
{
    static const std::string kStrVulkanGlslFileExtension = "glsl";
    bool                     ret                         = false;

    // Get the file extension.
    size_t             offset = file_path.rfind('.');
    const std::string& ext    = (offset != std::string::npos && ++offset < file_path.size()) ? file_path.substr(offset) : "";
    if (ext == kStrVulkanGlslFileExtension)
    {
        ret = true;
    }
    return ret;
}

// Callback for printing to stdout.
static void LoggingCallback(const std::string& s)
{
    std::cout << s.c_str() << std::flush;
}

bool ValidateInputFiles(const Config& config, VkOfflineOptions& vulkan_options, bool& is_stageless_input_file_present, std::stringstream& log_msg)
{
    // Input validation.
    bool should_abort                      = false;
    bool is_vert_shader_present            = (!config.vertex_shader.empty());
    bool is_tess_control_shader_present    = (!config.tess_control_shader.empty());
    bool is_tess_evaluation_shader_present = (!config.tess_evaluation_shader.empty());
    bool is_geom_shader_present            = (!config.geometry_shader.empty());
    bool is_frag_shader_present            = (!config.fragment_shader.empty());
    bool is_comp_shader_present            = (!config.compute_shader.empty());
    bool is_isa_required                   = IsIsaRequired(config);
    bool is_livereg_analysis_required      = (!config.livereg_analysis_file.empty());
    bool is_block_cfg_required             = (!config.block_cfg_file.empty());
    bool is_inst_cfg_required              = (!config.inst_cfg_file.empty());
    bool is_pipeline_binary_required       = (!config.binary_output_file.empty());
    bool is_il_required                    = (!config.il_file.empty());
    bool is_stats_required                 = (!config.analysis_file.empty());
    bool is_pipe_input                     = (!config.pipe_file.empty());

    // Cannot mix compute and non-compute shaders in Vulkan.
    if (is_comp_shader_present &&
        (is_vert_shader_present 
            || is_tess_control_shader_present 
            || is_tess_evaluation_shader_present 
            || is_geom_shader_present 
            || is_frag_shader_present))
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

    if (!should_abort && !config.input_files.empty() && !config.input_files[0].empty())
    {
        is_stageless_input_file_present     = true;
        should_abort                        = !KcUtils::ValidateShaderFileName("", config.input_files[0], log_msg);
        vulkan_options.stageless_input_file = config.input_files[0];
    }

    const std::string kTempFileName = "rga-temp-out";
    // Validate the input shaders.
    if (!should_abort && is_vert_shader_present)
    {
        should_abort = !KcUtils::ValidateShaderFileName(kStrVertexStage, config.vertex_shader, log_msg);
        if (IsInputFileExtGlsl(config.vertex_shader))
        { 
            const auto& temp_vert_file = KcUtils::ConstructTempFileName(kTempFileName, kVulkanStageFileSuffixDefault[BePipelineStage::kVertex]);
            should_abort               = !KcUtils::CopyTextFile(config.vertex_shader, temp_vert_file, LoggingCallback);
            vulkan_options.pipeline_shaders.vertex_shader << temp_vert_file.c_str();
        }
        else
        {
            vulkan_options.pipeline_shaders.vertex_shader << config.vertex_shader.c_str();
        }        
    }

    if (!should_abort && is_tess_control_shader_present)
    {
        should_abort = !KcUtils::ValidateShaderFileName(kStrTessellationControlStageName, config.tess_control_shader, log_msg);
        if (IsInputFileExtGlsl(config.tess_control_shader))
        {
            const auto& temp_tesc_name = KcUtils::ConstructTempFileName(kTempFileName, kVulkanStageFileSuffixDefault[BePipelineStage::kTessellationControl]);
            should_abort               = !KcUtils::CopyTextFile(config.tess_control_shader, temp_tesc_name, LoggingCallback);
            vulkan_options.pipeline_shaders.tessellation_control_shader << temp_tesc_name.c_str();
        }
        else
        {
            vulkan_options.pipeline_shaders.tessellation_control_shader << config.tess_control_shader.c_str();
        } 
    }

    if (!should_abort && is_tess_evaluation_shader_present)
    {
        should_abort = !KcUtils::ValidateShaderFileName(kStrTessellationEvaluationStageName, config.tess_evaluation_shader, log_msg);
        if (IsInputFileExtGlsl(config.tess_evaluation_shader))
        {
            const auto& temp_tese_name = KcUtils::ConstructTempFileName(kTempFileName, kVulkanStageFileSuffixDefault[BePipelineStage::kTessellationEvaluation]);
            should_abort = !KcUtils::CopyTextFile(config.tess_evaluation_shader, temp_tese_name, LoggingCallback);
            vulkan_options.pipeline_shaders.tessellation_evaluation_shader << temp_tese_name.c_str();
        }
        else
        {
            vulkan_options.pipeline_shaders.tessellation_evaluation_shader << config.tess_evaluation_shader.c_str();
        }        
    }

    if (!should_abort && is_geom_shader_present)
    {
        should_abort = !KcUtils::ValidateShaderFileName(kStrGeometryStageName, config.geometry_shader, log_msg);
        if (IsInputFileExtGlsl(config.geometry_shader))
        {
            const auto& temp_geom_name = KcUtils::ConstructTempFileName(kTempFileName, kVulkanStageFileSuffixDefault[BePipelineStage::kGeometry]);
            should_abort               = !KcUtils::CopyTextFile(config.geometry_shader, temp_geom_name, LoggingCallback);
            vulkan_options.pipeline_shaders.geometry_shader << temp_geom_name.c_str();
        }
        else
        {
            vulkan_options.pipeline_shaders.geometry_shader << config.geometry_shader.c_str();
        }
    }

    if (!should_abort && is_frag_shader_present)
    {
        should_abort = !KcUtils::ValidateShaderFileName(kStrFragmentStageName, config.fragment_shader, log_msg);
        if (IsInputFileExtGlsl(config.fragment_shader))
        {
            const auto& temp_frag_name = KcUtils::ConstructTempFileName(kTempFileName, kVulkanStageFileSuffixDefault[BePipelineStage::kFragment]);
            should_abort               = !KcUtils::CopyTextFile(config.fragment_shader, temp_frag_name, LoggingCallback);
            vulkan_options.pipeline_shaders.fragment_shader << temp_frag_name.c_str();
        }
        else
        {
            vulkan_options.pipeline_shaders.fragment_shader << config.fragment_shader.c_str();
        }        
    }

    if (!should_abort && is_comp_shader_present)
    {
        should_abort = !KcUtils::ValidateShaderFileName(kStrComputeStageName, config.compute_shader, log_msg);
        if (IsInputFileExtGlsl(config.compute_shader))
        {
            const auto& temp_comp_name = KcUtils::ConstructTempFileName(kTempFileName, kVulkanStageFileSuffixDefault[BePipelineStage::kCompute]);
            should_abort               = !KcUtils::CopyTextFile(config.compute_shader, temp_comp_name, LoggingCallback);
            vulkan_options.pipeline_shaders.compute_shader << temp_comp_name.c_str();
        }
        else
        {
            vulkan_options.pipeline_shaders.compute_shader << config.compute_shader.c_str();
        } 
    }

    return should_abort;
}

bool ValidateOutputDir(const Config& config, VkOfflineOptions& vulkan_options, std::stringstream& log_msg)
{
    bool should_abort = false;

    bool is_isa_required = IsIsaRequired(config);
    bool is_livereg_analysis_required = (!config.livereg_analysis_file.empty());
    bool is_block_cfg_required        = (!config.block_cfg_file.empty());
    bool is_inst_cfg_required         = (!config.inst_cfg_file.empty());
    bool is_pipeline_binary_required  = (!config.binary_output_file.empty());
    bool is_il_required               = (!config.il_file.empty());
    bool is_stats_required            = (!config.analysis_file.empty());

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

    return should_abort;
}

void AdjustRenderingPipelineOutputFileNames(const Config&                   config,
                                            const std::string&              device,
                                            VkOfflineOptions&               vulkan_options,
                                            std::stringstream&              warning_msg,
                                            bool&                           status)
{
    bool is_isa_required                   = IsIsaRequired(config);
    bool is_livereg_analysis_required      = (!config.livereg_analysis_file.empty());
    bool is_block_cfg_required             = (!config.block_cfg_file.empty());
    bool is_inst_cfg_required              = (!config.inst_cfg_file.empty());
    bool is_il_required                    = (!config.il_file.empty());
    bool is_stats_required                 = (!config.analysis_file.empty());
    bool is_pipe_input                     = (!config.pipe_file.empty());

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
        std::string adjusted_base_file_name         = KcUtils::AdjustBaseFileNameBinary(config.binary_output_file, device);
        status &=
            KcUtils::AdjustRenderingPipelineOutputFileNames(adjusted_base_file_name, "", "isabin", device_to_lower, vulkan_options.isa_binary_output_files);

        if (is_isa_required)
        {
            vulkan_options.is_amd_isa_disassembly_required = true;
            std::string adjusted_base_file_name            = KcUtils::AdjustBaseFileNameIsaDisassembly(config.isa_file, device);

            status &= KcUtils::AdjustRenderingPipelineOutputFileNames(
                adjusted_base_file_name, "", kStrDefaultExtensionIsa, device_to_lower, vulkan_options.isa_disassembly_output_files);
        }
    }

    // Adjust the output file names to the device and shader type.
    vulkan_options.is_pipeline_binary_required = true;
    bool is_pipeline_binary_required = (!config.binary_output_file.empty());
    if (is_pipeline_binary_required)
    {
        KcUtils::ConstructOutputFileName(config.binary_output_file, "", "bin", "", device, vulkan_options.pipeline_binary);
    }
    else
    {
        vulkan_options.pipeline_binary = KcUtils::ConstructTempFileName("rga-temp-out", "bin");
    }

    if (is_il_required)
    {
        vulkan_options.is_amd_pal_disassembly_required = true;
        std::string adjusted_base_file_name            = KcUtils::AdjustBaseFileNameIlDisassembly(config.il_file, device);
        status &= KcUtils::AdjustRenderingPipelineOutputFileNames(
            adjusted_base_file_name, "", kStrDefaultExtensionAmdil, device_to_lower, vulkan_options.pal_il_disassembly_output_files);
    }

    if (is_stats_required)
    {
        vulkan_options.is_stats_required    = true;
        std::string adjusted_base_file_name = KcUtils::AdjustBaseFileNameStats(config.analysis_file, device);
        status &= KcUtils::AdjustRenderingPipelineOutputFileNames(
            adjusted_base_file_name, kStrDefaultExtensionStats, kStrDefaultExtensionCsv, device_to_lower, vulkan_options.stats_output_files);
    }

    // Live register analysis.
    if (is_livereg_analysis_required)
    {
        vulkan_options.is_livereg_required  = true;
        std::string adjusted_base_file_name = KcUtils::AdjustBaseFileNameLivereg(config.livereg_analysis_file, device);
        status &= KcUtils::AdjustRenderingPipelineOutputFileNames(
            adjusted_base_file_name, kStrDefaultExtensionLivereg, kStrDefaultExtensionText, device_to_lower, vulkan_options.livereg_output_files);
    }

    // CFG.
    if (is_block_cfg_required)
    {
        std::string adjusted_base_file_name = KcUtils::AdjustBaseFileNameCfg(config.block_cfg_file, device);
        status &= KcUtils::AdjustRenderingPipelineOutputFileNames(
            adjusted_base_file_name, KC_STR_DEFAULT_CFG_SUFFIX, kStrDefaultExtensionDot, device_to_lower, vulkan_options.cfg_output_files);
    }

    if (is_inst_cfg_required)
    {
        std::string adjusted_base_file_name = KcUtils::AdjustBaseFileNameCfg(config.inst_cfg_file, device);
        status &= KcUtils::AdjustRenderingPipelineOutputFileNames(
            adjusted_base_file_name, KC_STR_DEFAULT_CFG_SUFFIX, kStrDefaultExtensionDot, device_to_lower, vulkan_options.cfg_output_files);
    }
}

void LogErrorStatus(beKA::beStatus status, const gtString& build_error_log, std::stringstream& log_msg)
{
    log_msg << kStrInfoFailed << std::endl;

    switch (status)
    {
    case kBeStatusVulkanAmdllpcCompilationFailure:
        log_msg << build_error_log.asASCIICharArray();
        break;
    case kBeStatusVulkanAmdllpcLaunchFailure:
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


void GetBeProgramPipelineFiles(const BeProgramPipeline& files, std::vector<std::string>& fileStrings)
{
    auto append = [](const gtString& filename, std::vector<std::string>& filenameVec) {
        if (!filename.isEmpty() && KcUtils::FileNotEmpty(filename.asASCIICharArray()))
        {
            filenameVec.push_back(filename.asASCIICharArray());
        }
    };

    append(files.vertex_shader,                  fileStrings);
    append(files.tessellation_control_shader,    fileStrings);
    append(files.tessellation_evaluation_shader, fileStrings);
    append(files.geometry_shader,                fileStrings);
    append(files.fragment_shader,                fileStrings);
    append(files.compute_shader,                 fileStrings);
}

void DeleteTempFiles(const Config& config, const VkOfflineOptions& vulkan_options)
{
    // Delete temporary files
    std::vector<std::string> temp_files;

    bool is_isa_required = IsIsaRequired(config);
    if (is_isa_required && config.isa_file.empty())
    {
        GetBeProgramPipelineFiles(vulkan_options.isa_disassembly_output_files, temp_files);        
    }
    if (is_isa_required)
    {
        // Remove the temporary "ISA binary" files.
        GetBeProgramPipelineFiles(vulkan_options.isa_binary_output_files, temp_files);        
    }

    bool is_pipeline_binary_required = (!config.binary_output_file.empty());
    if (!is_pipeline_binary_required)
    {
        temp_files.push_back(vulkan_options.pipeline_binary);        
    }

    bool is_vert_shader_present            = (!config.vertex_shader.empty());
    bool is_tess_control_shader_present    = (!config.tess_control_shader.empty());
    bool is_tess_evaluation_shader_present = (!config.tess_evaluation_shader.empty());
    bool is_geom_shader_present            = (!config.geometry_shader.empty());
    bool is_frag_shader_present            = (!config.fragment_shader.empty());
    bool is_comp_shader_present            = (!config.compute_shader.empty());
    if (is_vert_shader_present && IsInputFileExtGlsl(config.vertex_shader))
    {
        temp_files.push_back(vulkan_options.pipeline_shaders.vertex_shader.asASCIICharArray());
    }
    if (is_tess_control_shader_present && IsInputFileExtGlsl(config.tess_control_shader))
    {
        temp_files.push_back(vulkan_options.pipeline_shaders.tessellation_control_shader.asASCIICharArray());
    }
    if (is_tess_evaluation_shader_present && IsInputFileExtGlsl(config.tess_evaluation_shader))
    {
        temp_files.push_back(vulkan_options.pipeline_shaders.tessellation_evaluation_shader.asASCIICharArray());
    }
    if (is_geom_shader_present && IsInputFileExtGlsl(config.geometry_shader))
    {
        temp_files.push_back(vulkan_options.pipeline_shaders.geometry_shader.asASCIICharArray());
    }
    if (is_frag_shader_present && IsInputFileExtGlsl(config.fragment_shader))
    {
        temp_files.push_back(vulkan_options.pipeline_shaders.fragment_shader.asASCIICharArray());
    }
    if (is_comp_shader_present && IsInputFileExtGlsl(config.compute_shader))
    {
        temp_files.push_back(vulkan_options.pipeline_shaders.compute_shader.asASCIICharArray());
    }

    KcCLICommanderVulkanUtil::DeleteTempFiles(temp_files);
}

void GetBeVkPipelineFileNames(const Config&           config,
                              const VkOfflineOptions& vulkan_options,
                              BeVkPipelineFiles&      spv_files,
                              BeVkPipelineFiles&      isa_files,
                              BeVkPipelineFiles&      stats_files)
{
    bool is_vert_shader_present = (!config.vertex_shader.empty());
    if (is_vert_shader_present)
    {
        spv_files[kVertex]   = vulkan_options.pipeline_shaders.vertex_shader.asASCIICharArray();
        isa_files[kVertex]   = vulkan_options.isa_disassembly_output_files.vertex_shader.asASCIICharArray();
        stats_files[kVertex] = vulkan_options.stats_output_files.vertex_shader.asASCIICharArray();
    }
    bool is_tess_control_shader_present = (!config.tess_control_shader.empty());
    if (is_tess_control_shader_present)
    {
        spv_files[kTessellationControl]   = vulkan_options.pipeline_shaders.tessellation_control_shader.asASCIICharArray();
        isa_files[kTessellationControl]   = vulkan_options.isa_disassembly_output_files.tessellation_control_shader.asASCIICharArray();
        stats_files[kTessellationControl] = vulkan_options.stats_output_files.tessellation_control_shader.asASCIICharArray();
    }
    bool is_tess_evaluation_shader_present = (!config.tess_evaluation_shader.empty());
    if (is_tess_evaluation_shader_present)
    {
        spv_files[kTessellationEvaluation]   = vulkan_options.pipeline_shaders.tessellation_evaluation_shader.asASCIICharArray();
        isa_files[kTessellationEvaluation]   = vulkan_options.isa_disassembly_output_files.tessellation_evaluation_shader.asASCIICharArray();
        stats_files[kTessellationEvaluation] = vulkan_options.stats_output_files.tessellation_evaluation_shader.asASCIICharArray();
    }
    bool is_geom_shader_present = (!config.geometry_shader.empty());
    if (is_geom_shader_present)
    {
        spv_files[kGeometry]   = vulkan_options.pipeline_shaders.geometry_shader.asASCIICharArray();
        isa_files[kGeometry]   = vulkan_options.isa_disassembly_output_files.geometry_shader.asASCIICharArray();
        stats_files[kGeometry] = vulkan_options.stats_output_files.geometry_shader.asASCIICharArray();
    }
    bool is_frag_shader_present = (!config.fragment_shader.empty());
    if (is_frag_shader_present)
    {
        spv_files[kFragment]   = vulkan_options.pipeline_shaders.fragment_shader.asASCIICharArray();
        isa_files[kFragment]   = vulkan_options.isa_disassembly_output_files.fragment_shader.asASCIICharArray();
        stats_files[kFragment] = vulkan_options.stats_output_files.fragment_shader.asASCIICharArray();
    }
    bool is_comp_shader_present = (!config.compute_shader.empty());
    if (is_comp_shader_present)
    {
        spv_files[kCompute]   = vulkan_options.pipeline_shaders.compute_shader.asASCIICharArray();
        isa_files[kCompute]   = vulkan_options.isa_disassembly_output_files.compute_shader.asASCIICharArray();
        stats_files[kCompute] = vulkan_options.stats_output_files.compute_shader.asASCIICharArray();
    }
}

void KcCLICommanderVkOffline::RunCompileCommands(const Config& config, LoggingCallbackFunction callback)
{
    log_callback_ = callback;
    bool status = true;

    // Output stream.
    std::stringstream log_msg, warning_msg;

    bool should_abort = false;
    bool is_stageless_input_file_present   = false;

    // Fatal error. This should not happen unless we have an allocation problem.
    if (vulkan_builder_ == nullptr)
    {
        should_abort = true;
        log_msg << kStrErrorMemoryAllocationFailure << std::endl;
    }

    // Options to be passed to the backend.
    VkOfflineOptions vulkan_options(config.mode);
    // Input validation.
    if (!should_abort)
    {
        should_abort = ValidateInputFiles(config, vulkan_options, is_stageless_input_file_present, log_msg);
    }
    // Validate the output directories.
    if (!should_abort)
    {
        should_abort = ValidateOutputDir(config, vulkan_options, log_msg);
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

            KcCLICommanderVulkanUtil vk_util(output_metadata_, "", log_callback_, kVulkanStageFileSuffixDefault);

            for (const std::string& device : target_devices)
            {
                // Generate the output message.
                log_msg << kStrInfoCompiling << device << "... ";
                
                // Adjust Rendering Pipeline Output FileNames.
                AdjustRenderingPipelineOutputFileNames(config, device, vulkan_options, warning_msg, status);
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
                beKA::beStatus compilation_status = kBeStatusGeneralFailed;
                compilation_status = vulkan_builder_->CompileWithAmdllpc(vulkan_options, should_cancel, config.print_process_cmd_line, build_error_log);
                if (compilation_status == kBeStatusSuccess)
                {
                    log_msg << kStrInfoSuccess << std::endl;
                    
                    std::string       error_msg;
                    BeVkPipelineFiles spv_files, isa_files, stats_files;
                    GetBeVkPipelineFileNames(config, vulkan_options, spv_files, isa_files, stats_files);

                    std::string                        amdgpu_dis_stdout;
                    std::map<std::string, std::string> shader_to_disassembly;
                    compilation_status = beProgramBuilderVulkan::AmdgpudisBinaryToDisassembly(
                        vulkan_options.pipeline_binary, isa_files, config.print_process_cmd_line, amdgpu_dis_stdout, shader_to_disassembly, error_msg);
                    if (compilation_status == kBeStatusSuccess)
                    {
                        StoreOutputFilesToOutputMD(device, spv_files, isa_files, stats_files);

                        vk_util.ExtractStatistics(config, device, amdgpu_dis_stdout, shader_to_disassembly);
                    }

                }
                else
                {
                    LogErrorStatus(compilation_status, build_error_log, log_msg);
                }

                // Print the message for the current device.
                callback(log_msg.str());

                // Clear the output stream for the next iteration.
                log_msg.str("");
            }

            vk_util.RunPostProcessingSteps(config);

            DeleteTempFiles(config, vulkan_options);

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

void KcCLICommanderVkOffline::StoreOutputFilesToOutputMD(const std::string&       device,
                                                         const BeVkPipelineFiles& spvFiles,
                                                         const BeVkPipelineFiles& isaFiles,
                                                         const BeVkPipelineFiles& statsFiles)
{
    // Check if the output Metadata for this device already exists.
    // It exists if some of shader files are GLSL/HLSL or SPIR-V text files. In that case,
    // the metadata has been created during pre-compiling the shaders to SPIR-V binary format
    bool device_md_exists = (output_metadata_.find(device) != output_metadata_.end());
    if (!device_md_exists)
    {
        RgVkOutputMetadata md;
        RgOutputFiles      out_files(device);
        md.fill(out_files);
        output_metadata_[device] = md;
    }

    RgVkOutputMetadata& device_md = output_metadata_[device];
    for (int stage = 0; stage < BePipelineStage::kCount; stage++)
    {
        if (!spvFiles[stage].empty())
        {
            // If the "input file" in the metadata for this stage is non-empty, keep it there (it's the path to the
            // GLSL/HLSL/spv-text file that was pre-compiled to a temporary SPIR-V binary file).
            // If it's empty, store the spv file path as an "input file".
            RgOutputFiles& stage_md = device_md[stage];
            if (stage_md.input_file.empty())
            {
                stage_md.input_file = spvFiles[stage];
                stage_md.entry_type = kVulkanOglStageEntryTypes[stage];
                stage_md.device     = device;
            }
            stage_md.isa_file   = isaFiles[stage];
            stage_md.stats_file = statsFiles[stage];
        }
    }
}

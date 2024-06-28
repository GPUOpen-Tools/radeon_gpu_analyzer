// C++.
#include <cassert>

// Shared.
#include "common/rg_log.h"

// Backend.
#include "radeon_gpu_analyzer_backend/be_utils.h"
#include "radeon_gpu_analyzer_backend/be_opencl_definitions.h"
#include "radeon_gpu_analyzer_backend/be_program_builder_vulkan.h"

// Local.
#include "radeon_gpu_analyzer_cli/kc_cli_commander_bin_util.h"
#include "radeon_gpu_analyzer_cli/kc_cli_commander_dxr_util.h"
#include "radeon_gpu_analyzer_cli/kc_cli_commander_lightning_util.h"
#include "radeon_gpu_analyzer_cli/kc_cli_commander_vulkan_util.h"
#include "radeon_gpu_analyzer_cli/kc_utils.h"

// Output metadata entry types for Vulkan pipeline stages.
static const BeVkPipelineEntries 
kDx12StageEntryTypes = 
{
    RgEntryType::kDxVertex,
    RgEntryType::kDxHull,
    RgEntryType::kDxDomain,
    RgEntryType::kDxGeometry,
    RgEntryType::kDxPixel,
    RgEntryType::kDxCompute,
};

// An array containing per-stage RgEntryType(s).
typedef std::array<RgEntryType, BeRtxPipelineStage::kCountRtx> BeRtxPipelineEntries;

// Output metadata entry types for Dxr rt pipeline stages.
static const BeRtxPipelineEntries 
kRtxStageEntryTypes = 
{
    RgEntryType::kDxrRayGeneration,
    RgEntryType::kDxrIntersection,
    RgEntryType::kDxrAnyHit,
    RgEntryType::kDxrClosestHit,
    RgEntryType::kDxrMiss,
    RgEntryType::kDxrCallable,
    RgEntryType::kDxrTraversal
};

static const char* kStrErrorCannotParseDisassembly     = "Error: failed to parse LLVM disassembly.";
static const char* kStrErrorNoShaderFoundInDisassembly = "Error: LLVM disassembly does not contatin valid kernels.";
static const char* kAmdgpuDisDotTextToken              = ".text";
static const char* kAmdgpuDisShaderEndToken            = "_symend:";
static const char* kAmdgpuDisDotSizeToken              = ".size";
static const char* kAmdgpuDisKernelNameStartToken      = "_symend-";
static const char* kAmdgpuDisKernelName                = "amdgpu_kernel_";

beKA::beStatus ParseAmdgpudisOutputGraphicStrategy::ParseAmdgpudisKernels(const std::string&                  amdgpu_dis_output,
                                                                          std::map<std::string, std::string>& kernel_to_disassembly,
                                                                          std::string&                        error_msg) const
{
    beKA::beStatus status = beKA::beStatus::kBeStatusGeneralFailed;
    status                = beProgramBuilderVulkan::ParseAmdgpudisOutput(amdgpu_dis_output, kernel_to_disassembly, error_msg);
    if (status != beKA::beStatus::kBeStatusSuccess)
    {
        std::map<std::string, std::string>  dxr_kernel_to_disassembly;
        std::string                         dxr_error_str;
        if (ParseAmdgpudisOutputComputeStrategy{}.ParseAmdgpudisKernels(amdgpu_dis_output, dxr_kernel_to_disassembly, dxr_error_str) ==
            beKA::beStatus::kBeStatusSuccess)
        {
            status                = beKA::beStatus::kBeStatusSuccess;
            kernel_to_disassembly = std::move(dxr_kernel_to_disassembly);
            error_msg             = std::move(dxr_error_str);
        }
    }
    return status;
}

beKA::beStatus ParseAmdgpudisOutputComputeStrategy::ParseAmdgpudisKernels(const std::string&                  amdgpu_dis_output,
                                                                          std::map<std::string, std::string>& kernel_to_disassembly,
                                                                          std::string&                        error_msg) const
{
    beKA::beStatus status = beKA::beStatus::kBeStatusGeneralFailed;
    assert(!amdgpu_dis_output.empty());
    if (!amdgpu_dis_output.empty())
    {
        // Get to the .text section.
        size_t curr_pos = amdgpu_dis_output.find(kAmdgpuDisDotTextToken);
        assert(curr_pos != std::string::npos);
        if (curr_pos != std::string::npos)
        {
            // These will be used to extract the disassembly for each shader.
            size_t kernel_offset_begin = 0;
            size_t kernel_offset_end   = 0;

            // Parse the .text section. Identify each shader's area by its ".size" token.
            curr_pos = amdgpu_dis_output.find(kAmdgpuDisDotSizeToken);
            if (curr_pos == std::string::npos)
            {
                error_msg = kStrErrorNoShaderFoundInDisassembly;
            }

            while (curr_pos != std::string::npos)
            {
                // Find the name of the kernel.
                std::string kernel_name;
                size_t      end_of_line = amdgpu_dis_output.find("\n", curr_pos);
                std::string line        = amdgpu_dis_output.substr(curr_pos, end_of_line - curr_pos);
                auto        found       = line.find(kAmdgpuDisKernelNameStartToken);
                if (found != std::string::npos)
                {
                    size_t start = found + strlen(kAmdgpuDisKernelNameStartToken);
                    kernel_name  = line.substr(start, end_of_line - start);
                    curr_pos     = end_of_line;
                }

                if (!kernel_name.empty() && curr_pos != std::string::npos)
                {
                    // Construct the shader end token "kernel_name_symend:".
                    std::stringstream kernel_end_token_stream;
                    kernel_end_token_stream << kernel_name << kAmdgpuDisShaderEndToken;
                    std::string kernel_token_end = kernel_end_token_stream.str();

                    // Extract the kernel disassembly.
                    kernel_offset_begin            = curr_pos;
                    kernel_offset_end              = amdgpu_dis_output.find(kernel_token_end, kernel_offset_begin);
                    std::string kernel_disassembly = amdgpu_dis_output.substr(curr_pos, kernel_offset_end - curr_pos);
                    BeUtils::TrimLeadingAndTrailingWhitespace(kernel_disassembly, kernel_disassembly);
                    kernel_to_disassembly[kernel_name] = KcCLICommanderLightningUtil::PrefixWithISAHeader(kernel_name, kernel_disassembly);
                }
                else
                {
                    error_msg         = kStrErrorCannotParseDisassembly;
                    status            = beKA::beStatus::kBeStatusCannotParseDisassemblyGeneral;
                    kernel_offset_end = found;
                }

                // Look for the next shader.
                curr_pos = amdgpu_dis_output.find(kAmdgpuDisDotSizeToken, kernel_offset_end);
            }
        }
        else
        {
            error_msg = kStrErrorCannotParseDisassembly;
        }
    }

    if (!kernel_to_disassembly.empty())
    {
        status = beKA::beStatus::kBeStatusSuccess;
    }
    return status;
}

beKA::beStatus GraphicsBinaryWorkflowStrategy::WriteOutputFiles(const Config&                             config,
                                                                const std::string&                        asic,
                                                                const std::map<std::string, std::string>& kernel_to_disassembly,
                                                                const BeAmdPalMetaData::PipelineMetaData& amdpal_pipeline_md,
                                                                std::string&                              )
{
    beKA::beStatus status = beKA::beStatus::kBeStatusGeneralFailed;
    assert(!kernel_to_disassembly.empty());
    if (!kernel_to_disassembly.empty())
    {
        // Retrive graphics api for current binary.
        graphics_api_ = beProgramBuilderBinary::GetApiFromPipelineMetadata(amdpal_pipeline_md);
        
        // Write the ISA disassembly files.
        for (uint32_t stage = BePipelineStage::kVertex; stage < BePipelineStage::kCount; stage++)
        {
            std::string isa_file;
            if (KcUtils::ConstructOutFileName(
                    config.isa_file, beProgramBuilderBinary::GetStageFileSuffixesFromApi(graphics_api_)[stage], asic, kStrVulkanIsaFileExtension, isa_file))
            {
                if (!isa_file.empty())
                {
                    KcUtils::DeleteFile(isa_file);
                }

                bool is_file_written = beProgramBuilderVulkan::WriteIsaFileWithHwMapping(stage, amdpal_pipeline_md, kernel_to_disassembly, isa_file);
                if (is_file_written)
                {
                    // For graphics workflows, also extract stats.
                    bool is_stats_file_required = !config.analysis_file.empty();
                    if (is_stats_file_required)
                    {
                        std::string stats_file;
                        if (KcUtils::ConstructOutFileName(config.analysis_file,
                                                          beProgramBuilderBinary::GetStageFileSuffixesFromApi(graphics_api_)[stage],
                                                          asic,
                                                          kStrVulkanStatsFileExtension,
                                                          stats_file))
                        {
                            StoreOutputFilesToOutputMD(config, asic, stage, isa_file, stats_file);

                            if (!stats_file.empty())
                            {
                                KcUtils::DeleteFile(stats_file);
                            }
                        }
                    }
                    else
                    {
                        StoreOutputFilesToOutputMD(config, asic, stage, isa_file, "");
                    }

                    status = beKA::beStatus::kBeStatusSuccess;
                }
            }
        }

        KcCLICommanderVulkanUtil vk_util(output_metadata_, "", log_callback_, beProgramBuilderBinary::GetStageFileSuffixesFromApi(graphics_api_));
        vk_util.ExtractStatistics(config, asic, amdpal_pipeline_md, kernel_to_disassembly);

    }

    return status;
}

RgEntryType GetEntryType(beProgramBuilderBinary::ApiEnum api, uint32_t stage)
{
    RgEntryType entry = RgEntryType::kUnknown;
    if (api == beProgramBuilderBinary::ApiEnum::kDXR)
    {
        assert(BeRtxPipelineStage::kRayGeneration <= stage && stage < BeRtxPipelineStage::kCountRtx);
        if (BeRtxPipelineStage::kRayGeneration <= stage && stage < BeRtxPipelineStage::kCountRtx)
        {
            entry = kRtxStageEntryTypes[stage];
        }
    }
    else
    {
        assert(BePipelineStage::kVertex <= stage && stage < BePipelineStage::kCount);
        if (BePipelineStage::kVertex <= stage && stage < BePipelineStage::kCount)
        {
            switch (api)
            {
            case beProgramBuilderBinary::ApiEnum::kDX12:
                entry = kDx12StageEntryTypes[stage];
                break;
            case beProgramBuilderBinary::ApiEnum::kVulkan:
                entry = kVulkanStageEntryTypes[stage];
                break;
            case beProgramBuilderBinary::ApiEnum::kOpenGL:
                entry = kOpenGLStageEntryTypes[stage];
                break;
            default:
                assert(false);
                break;
            }
        }
    }
    return entry;
}

void GraphicsBinaryWorkflowStrategy::StoreOutputFilesToOutputMD(const Config&      config,
                                                                const std::string& asic,
                                                                uint32_t           stage,
                                                                const std::string& isa_filename,
                                                                const std::string& stats_filename)
{
    bool device_md_exists = (output_metadata_.find(asic) != output_metadata_.end());
    if (!device_md_exists)
    {
        RgVkOutputMetadata md;
        RgOutputFiles      out_files(asic);
        md.fill(out_files);
        output_metadata_[asic] = md;
    }

    RgVkOutputMetadata& device_md = output_metadata_[asic];
    if (stage < BePipelineStage::kCount)
    {
        RgOutputFiles& stage_md = device_md[stage];
        if (stage_md.input_file.empty())
        {
            stage_md.input_file = config.binary_codeobj_file;
            stage_md.entry_type = GetEntryType(graphics_api_, stage);
            stage_md.device     = asic;
        }
        stage_md.isa_file   = isa_filename;
        if (!config.analysis_file.empty())
        {
            stage_md.stats_file = stats_filename;
        }
    }

    // If user does not provide an isa filename explicitly.
    if (config.isa_file.empty())
    {
        // ISA file generated is a temporary one.
        temp_files_.push_back(isa_filename);
    }
}

void GraphicsBinaryWorkflowStrategy::RunPostProcessingSteps(const Config& config, const BeAmdPalMetaData::PipelineMetaData&)
{
    const auto&              suffixes = beProgramBuilderBinary::GetStageFileSuffixesFromApi(graphics_api_);
    KcCLICommanderVulkanUtil vk_util(output_metadata_, "", log_callback_, suffixes);
    vk_util.RunPostProcessingSteps(config);
}

bool GraphicsBinaryWorkflowStrategy::RunPostCompileSteps(const Config& config)
{
    bool ret = false;

    const auto&              suffixes = beProgramBuilderBinary::GetStageFileSuffixesFromApi(graphics_api_);
    KcCLICommanderVulkanUtil util(output_metadata_, "", log_callback_, suffixes);

    if (!config.session_metadata_file.empty())
    {
        ret = util.GenerateSessionMetadata(config);
        if (!ret)
        {
            RgLog::stdOut << kStrErrorFailedToGenerateSessionMetdata << std::endl;
        }
    }

    if (!config.should_retain_temp_files)
    {
        util.DeleteTempFiles(temp_files_);
    }

    return ret;
}

bool ExtractShaderSubtype(const BeAmdPalMetaData::PipelineMetaData& pipeline, const std::string& kernel, std::string& shader_subtype)
{
    bool ret = false;
    for (const auto& shader_function : pipeline.shader_functions)
    {
        if (kernel == shader_function.name)
        {
            shader_subtype = BeAmdPalMetaData::GetShaderSubtypeName(shader_function.shader_subtype);
            ret            = true;
        }
    }
    for (const auto& shader : pipeline.shaders)
    {
        if (shader.shader_subtype != BeAmdPalMetaData::ShaderSubtype::kUnknown)
        {
            shader_subtype = BeAmdPalMetaData::GetShaderSubtypeName(shader.shader_subtype);
            ret            = true;
        }
    }
    return ret;
}

beKA::beStatus RayTracingBinaryWorkflowStrategy::WriteOutputFiles(const Config&                             config,
                                                                  const std::string&                        asic,
                                                                  const std::map<std::string, std::string>& kernel_to_disassembly,
                                                                  const BeAmdPalMetaData::PipelineMetaData& amdpal_pipeline_md,
                                                                  std::string&                              error_msg)
{
    beKA::beStatus status = beKA::beStatus::kBeStatusGeneralFailed;
    assert(!kernel_to_disassembly.empty());
    if (!kernel_to_disassembly.empty())
    {
        const std::string& isa_file = config.isa_file;
        for (const auto& kernel : kernel_to_disassembly)
        {
            const auto& amdgpu_kernel_name    = kernel.first;
            const auto& shader_kernel_content = kernel.second;
            std::string shader_kernel_subtype;
            if (ExtractShaderSubtype(amdpal_pipeline_md, amdgpu_kernel_name, shader_kernel_subtype))
            {
                std::string concat_kernel_name = KcCLICommanderDxrUtil::CombineKernelAndKernelSubtype(amdgpu_kernel_name, shader_kernel_subtype);
                std::string amdgpu_out_file_name;
                if (KcUtils::ConstructOutFileName(isa_file, concat_kernel_name, asic, kStrDefaultFilenameIsa, amdgpu_out_file_name))
                {
                    if (!amdgpu_out_file_name.empty())
                    {
                        KcUtils::DeleteFile(amdgpu_out_file_name);
                    }

                    [[maybe_unused]] bool is_file_written = KcUtils::WriteTextFile(amdgpu_out_file_name, shader_kernel_content, nullptr);
                    assert(is_file_written);
                    if (!KcUtils::FileNotEmpty(amdgpu_out_file_name))
                    {
                        status = beKA::beStatus::kBeStatusWriteToFileFailed;
                        std::stringstream error_stream;
                        error_stream << concat_kernel_name << ", output file name " << amdgpu_out_file_name;
                        error_msg = error_stream.str();
                    }
                    else
                    {
                        // Store output metadata.
                        StoreOutputFilesToOutputMD(config, asic, amdgpu_kernel_name, shader_kernel_subtype, amdgpu_out_file_name);
                        status = beKA::beStatus::kBeStatusSuccess;
                    }
                }
            }
        }
    }
    return status;
}

void RayTracingBinaryWorkflowStrategy::RunPostProcessingSteps(const Config& config, const BeAmdPalMetaData::PipelineMetaData& amdpal_pipeline_md)
{
    KcCLICommanderDxrUtil util(output_metadata_, config.print_process_cmd_line, log_callback_);
    beKA::beStatus        status = beKA::beStatus::kBeStatusSuccess;

    // Generate CSV files with parsed ISA if required.
    if (config.is_parsed_isa_required)
    {
        status = util.ParseIsaFilesToCSV(config.is_line_numbers_required) ? beKA::beStatus::kBeStatusSuccess : beKA::beStatus::kBeStatusParseIsaToCsvFailed;
    }

    // Analyze live registers if requested.
    bool is_livereg_required = !config.livereg_analysis_file.empty();
    if (is_livereg_required && (status == beKA::beStatus::kBeStatusSuccess))
    {
        // Perform Live Registers analysis if required.
        status = util.PerformLiveVgprAnalysis(config) ? beKA::beStatus::kBeStatusSuccess : beKA::beStatus::kBeStatusParseIsaToCsvFailed;
    }
    bool is_live_sgpr_required = !config.sgpr_livereg_analysis_file.empty();
    if (is_live_sgpr_required && (status == beKA::beStatus::kBeStatusSuccess))
    {
        // Perform Live Registers analysis if required.
        status = util.PerformLiveSgprAnalysis(config) ? beKA::beStatus::kBeStatusSuccess : beKA::beStatus::kBeStatusParseIsaToCsvFailed;
    }

    bool is_cfg_required = (!config.block_cfg_file.empty() || !config.inst_cfg_file.empty());
    if (is_cfg_required && (status == beKA::beStatus::kBeStatusSuccess))
    {
        // Extract Control Flow Graph.
        util.ExtractCFG(config);
    }

    // Extract Statistics if required.
    if ((status == beKA::beStatus::kBeStatusSuccess) && !config.analysis_file.empty())
    {
        util.ExtractStatistics(config, amdpal_pipeline_md);
    }
}

bool RayTracingBinaryWorkflowStrategy::RunPostCompileSteps(const Config& config)
{
    // Compute workflows rely on output_metadata for cleaningup temp files.
    KcCLICommanderDxrUtil util(output_metadata_, config.print_process_cmd_line, log_callback_);
    return util.RunPostCompileSteps(config);
}

uint32_t GetRaytracingStage(const std::string& curr_kernel_subtype)
{
    uint32_t stage = BeRtxPipelineStage::kRayGeneration;
    for (const auto& kernel_subtype : kStrRtxStageNames)
    {
        if (kernel_subtype == curr_kernel_subtype)
        {
            break;
        }
        stage++;
    }
    assert(BeRtxPipelineStage::kRayGeneration <= stage && stage < BeRtxPipelineStage::kCountRtx);
    return stage;
}



void RayTracingBinaryWorkflowStrategy::StoreOutputFilesToOutputMD(const Config&      config,
                                                                  const std::string& asic,
                                                                  const std::string& kernel,
                                                                  const std::string& kernel_subtype,
                                                                  const std::string& isa_filename)
{
    std::string   concat_kernel_name             = KcCLICommanderDxrUtil::CombineKernelAndKernelSubtype(kernel, kernel_subtype);
    uint32_t      stage                          = GetRaytracingStage(kernel_subtype);
    RgEntryType   entry                          = GetEntryType(beProgramBuilderBinary::ApiEnum::kDXR, stage);
    RgOutputFiles outFiles                       = RgOutputFiles(entry, isa_filename);
    outFiles.input_file                          = concat_kernel_name;
    outFiles.is_isa_file_temp                    = config.isa_file.empty();
    outFiles.bin_file                            = config.binary_codeobj_file;
    output_metadata_[{asic, concat_kernel_name}] = outFiles;
}

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
    RgOutputFiles outFiles                = RgOutputFiles(RgEntryType::kOpenclKernel, isa_filename);
    outFiles.input_file                   = kernel_name;
    outFiles.is_isa_file_temp             = config.isa_file.empty();
    outFiles.bin_file                     = config.binary_codeobj_file;
    outFiles.entry_abbreviation           = kernel_abbreviation;
    output_metadata_[{asic, kernel_name}] = outFiles;
}

void ComputeBinaryWorkflowStrategy::RunPostProcessingSteps(const Config& config, const BeAmdPalMetaData::PipelineMetaData&)
{
    CmpilerPaths compiler_paths = {config.compiler_bin_path, config.compiler_inc_path, config.compiler_lib_path};
    bool should_print_cmd       = config.print_process_cmd_line;

    KcCLICommanderLightningUtil util(output_metadata_, should_print_cmd, log_callback_);
    beKA::beStatus              status = beKA::beStatus::kBeStatusSuccess;

    // Generate CSV files with parsed ISA if required.
    if (config.is_parsed_isa_required)
    {
        status = util.ParseIsaFilesToCSV(config.is_line_numbers_required) ? beKA::beStatus::kBeStatusSuccess : beKA::beStatus::kBeStatusParseIsaToCsvFailed;
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

    // Extract Statistics if required.
    if ((status == beKA::beStatus::kBeStatusSuccess) && !config.analysis_file.empty())
    {
        util.ExtractStatistics(config);
    }
}

bool ComputeBinaryWorkflowStrategy::RunPostCompileSteps(const Config& config)
{
    // Compute workflows rely on output_metadata for cleaningup temp files.
    KcCLICommanderLightningUtil util(output_metadata_, config.print_process_cmd_line, log_callback_);
    CmpilerPaths                compiler_paths = {config.compiler_bin_path, config.compiler_inc_path, config.compiler_lib_path};
    return util.RunPostCompileSteps(config, compiler_paths);
}

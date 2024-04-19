//=================================================================
// Copyright 2020-2024 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

// Local.
#include "radeon_gpu_analyzer_backend/autogen/be_generator_dx12.h"
#include "radeon_gpu_analyzer_backend/autogen/be_utils_dx12.h"
#include "radeon_gpu_analyzer_backend/autogen/be_reflection_dx12.h"
#include "radeon_gpu_analyzer_backend/autogen/be_string_constants_dx12.h"

// CLI.
#include "radeon_gpu_analyzer_cli/kc_utils.h"

// C++
#include <chrono>
#include <iomanip>
#include <cassert>

BeDx12AutoGenStatus BeDx12AutoGenerator::PopulateAutoGenInputs(const Config& config, BeDx12AutoGenInput& input_files, std::stringstream& err)
{
    BeDx12AutoGenStatus status    = BeDx12AutoGenStatus::kRequired;
    input_files.is_gpso_specified = !config.pso_dx12.empty();
    std::string source_file_path_vs, source_file_path_ps, source_file_path_cs;
    bool        is_dxc_compiler_req = false;

    // VS
    bool        has_blob_vs = !config.vs_dxbc.empty();
    bool        has_src_vs  = !config.vs_entry_point.empty();
    bool        has_any_vs  = has_blob_vs || has_src_vs;
    if (source_file_path_vs.empty() && has_blob_vs)
    {
        source_file_path_vs = BeDx12Utils::GetAbsoluteFileName(KcUtils::Quote(config.vs_dxbc));
    }
    if (has_src_vs)
    {
        if (has_blob_vs)
        {
            status = BeDx12AutoGenStatus::kNotRequired;
        }
        std::string vs_shader_source_path;
        bool        is_ok = BeDx12Utils::GetShaderStageSourceFileName(config, BePipelineStage::kVertex, vs_shader_source_path);
        if (source_file_path_vs.empty() && is_ok)
        {
            source_file_path_vs = vs_shader_source_path;
        }
        if (!is_dxc_compiler_req)
        {
            is_dxc_compiler_req = true;
        }
    }
    
    // PS
    bool has_blob_ps = !config.ps_dxbc.empty();
    bool has_src_ps  = !config.ps_entry_point.empty();
    bool has_any_ps  = has_blob_ps || has_src_ps;
    if (source_file_path_ps.empty() && has_blob_ps)
    {
        source_file_path_ps = BeDx12Utils::GetAbsoluteFileName(KcUtils::Quote(config.ps_dxbc));
    }
    if (has_src_ps)
    {
        if (has_blob_ps)
        {
            status = BeDx12AutoGenStatus::kNotRequired;
        }
        std::string ps_shader_source_path;
        bool        is_ok = BeDx12Utils::GetShaderStageSourceFileName(config, BePipelineStage::kFragment, ps_shader_source_path);
        if (source_file_path_ps.empty() && is_ok)
        {
            source_file_path_ps = ps_shader_source_path;
        }
        if (!is_dxc_compiler_req)
        {
            is_dxc_compiler_req = true;
        }
    }
    
    // CS
    bool has_blob_cs = !config.cs_dxbc.empty();
    bool has_src_cs  = !config.cs_entry_point.empty();
    bool has_any_cs  = has_blob_cs || has_src_cs;
    if (source_file_path_cs.empty() && has_blob_cs)
    {
        source_file_path_cs = BeDx12Utils::GetAbsoluteFileName(KcUtils::Quote(config.cs_dxbc));
    }
    if (has_src_cs)
    {
        if (has_blob_cs)
        {
            status = BeDx12AutoGenStatus::kNotRequired;
        }
        std::string cs_shader_source_path;
        bool        is_ok = BeDx12Utils::GetShaderStageSourceFileName(config, BePipelineStage::kCompute, cs_shader_source_path);
        if (source_file_path_cs.empty() && is_ok)
        {
            source_file_path_cs = cs_shader_source_path;
        }
        if (!is_dxc_compiler_req)
        {
            is_dxc_compiler_req = true;
        }
    }

    bool is_graphics = has_any_vs || has_any_ps;
    if (is_graphics && has_any_cs)
    {
        status = BeDx12AutoGenStatus::kNotRequired;
    }

    if (status == BeDx12AutoGenStatus::kRequired)
    {
        input_files.is_root_signature_specified = !config.rs_bin.empty() || !config.rs_macro.empty();
        const bool is_autogenerate_req =
            is_graphics ? 
            (!input_files.is_root_signature_specified 
                || !input_files.is_gpso_specified 
                || !has_any_vs 
                || !has_any_ps) 
            : !input_files.is_root_signature_specified;
        if (!is_autogenerate_req)
        {
            status = BeDx12AutoGenStatus::kNotRequired;
        }
    }

    if (status == BeDx12AutoGenStatus::kRequired)
    {
        compiler_ = std::make_unique<BeDx12Compiler>(config, is_dxc_compiler_req, err);

        // VS
        bool is_ok_vs =
        compiler_->LoadSrcBlob(config,
                                BePipelineStage::kVertex,
                                has_blob_vs,
                                has_src_vs,
                                source_file_path_vs,
                                input_files.vs_blob,
                                input_files.is_root_signature_specified,
                                err);
        if (has_any_vs && !is_ok_vs)
        {
            status = BeDx12AutoGenStatus::kNotRequired;
        }

        // PS
        bool is_ok_ps =
        compiler_->LoadSrcBlob(config,
                                BePipelineStage::kFragment,
                                has_blob_ps,
                                has_src_ps,
                                source_file_path_ps,
                                input_files.ps_blob,
                                input_files.is_root_signature_specified, 
                                err);
        if (has_any_ps && !is_ok_ps)
        {
            status = BeDx12AutoGenStatus::kNotRequired;
        }

        // CS
        bool is_ok_cs =
        compiler_->LoadSrcBlob(config,
                                BePipelineStage::kCompute,
                                has_blob_cs,
                                has_src_cs,
                                source_file_path_cs,
                                input_files.cs_blob,
                                input_files.is_root_signature_specified,
                                err);
        if (has_any_cs && !is_ok_cs)
        {
            status = BeDx12AutoGenStatus::kNotRequired;
        }

        if (!is_ok_vs && !is_ok_ps && !is_ok_cs)
        {
            status = BeDx12AutoGenStatus::kNotRequired;
        }

        const bool need_to_auto_generate_any_file =
            is_graphics ? (!input_files.is_root_signature_specified || !input_files.is_gpso_specified || !has_any_vs || !has_any_ps)
                                                                    : !input_files.is_root_signature_specified;
        if (!need_to_auto_generate_any_file)
        {
            status = BeDx12AutoGenStatus::kNotRequired;
        }
        
    }

    return status;
}

beKA::beStatus BeDx12AutoGenerator::GenerateFiles(const Config&             ,
                                                  const BeDx12AutoGenInput& src_input,
                                                  const std::string&        autogen_dir,
                                                  BeDx12AutoGenFile&        root_signature,
                                                  BeDx12AutoGenFile&        gpso_file,
                                                  BeDx12AutoGenFile&        vertex_shader,
                                                  BeDx12AutoGenFile&        pixel_shader,
                                                  std::stringstream&        err) const
{
    beKA::beStatus  rc    = beKA::beStatus::kBeStatusSuccess;
    if (rc == beKA::beStatus::kBeStatusSuccess && compiler_ != nullptr)
    {
        BeDx12Reflection::HlslOutput hlsl_output;
        rc = BeDx12Reflection{}.AutoGenerateFiles(compiler_->GetUtils(), src_input, hlsl_output, err);
        if (rc == beKA::beStatus::kBeStatusSuccess)
        {
            // Save root signature.
            const bool has_generated_root_signature = !hlsl_output.root_signature.empty();
            if (has_generated_root_signature != !src_input.is_root_signature_specified)
            {
                rc = beKA::beStatus::kBeStatusDxcCannotAutoGenerateRootSignature;
            }
            if (rc == beKA::beStatus::kBeStatusSuccess && has_generated_root_signature)
            {
                root_signature.status =
                    GenerateRootSignatureFileName(autogen_dir, root_signature.filename) ? BeDx12AutoGenStatus::kRequired
                                                                                        : BeDx12AutoGenStatus::kFailed;
                if (root_signature.status == BeDx12AutoGenStatus::kRequired)
                {
                    bool is_file_written = KcUtils::WriteTextFile(root_signature.filename, hlsl_output.root_signature, nullptr);
                    if (is_file_written && KcUtils::FileNotEmpty(root_signature.filename))
                    {
                        root_signature.status = BeDx12AutoGenStatus::kSuccess;
                    }
                    else
                    {
                        root_signature.status = BeDx12AutoGenStatus::kFailed;
                        rc = beKA::beStatus::kBeStatusDxcCannotAutoGenerateRootSignature;
                    }
                }
            }
            
            // Save GPSO.
            const bool has_generated_gpso = !hlsl_output.gpso.empty();
            if (has_generated_gpso)
            {
                gpso_file.status = GenerateGpsoFileName(autogen_dir, gpso_file.filename) ? BeDx12AutoGenStatus::kRequired : BeDx12AutoGenStatus::kFailed;
                if (gpso_file.status == BeDx12AutoGenStatus::kRequired)
                {
                    bool is_file_written = KcUtils::WriteTextFile(gpso_file.filename, hlsl_output.gpso, nullptr);
                    if (is_file_written && KcUtils::FileNotEmpty(gpso_file.filename))
                    {
                        gpso_file.status = BeDx12AutoGenStatus::kSuccess;
                    }
                    else
                    {
                        gpso_file.status = BeDx12AutoGenStatus::kFailed;
                        rc = beKA::beStatus::kBeStatusDxcCannotAutoGenerateGpso;
                    }
                }
            }

            // Save vertex shader.
            const bool has_generated_vs = !hlsl_output.vs.empty();
            if (has_generated_vs)
            {
                vertex_shader.status =
                    GenerateVertexShaderFileName(autogen_dir, vertex_shader.filename) ? BeDx12AutoGenStatus::kRequired : BeDx12AutoGenStatus::kFailed;
                if (vertex_shader.status == BeDx12AutoGenStatus::kRequired)
                {
                    bool is_file_written = KcUtils::WriteTextFile(vertex_shader.filename, hlsl_output.vs, nullptr);
                    if (is_file_written && KcUtils::FileNotEmpty(vertex_shader.filename))
                    {
                        vertex_shader.status = BeDx12AutoGenStatus::kSuccess;
                    }
                    else
                    {
                        vertex_shader.status = BeDx12AutoGenStatus::kFailed;
                        rc = beKA::beStatus::kBeStatusDxcCannotAutoGenerateVertexShader;
                    }
                }
            }

            // Save pixel shader.
            const bool has_generated_ps = !hlsl_output.ps.empty();
            if (has_generated_ps)
            {
                pixel_shader.status =
                    GeneratePixelShaderFileName(autogen_dir, pixel_shader.filename) ? BeDx12AutoGenStatus::kRequired : BeDx12AutoGenStatus::kFailed;
                if (pixel_shader.status == BeDx12AutoGenStatus::kRequired)
                {
                    bool is_file_written = KcUtils::WriteTextFile(pixel_shader.filename, hlsl_output.ps, nullptr);
                    if (is_file_written && KcUtils::FileNotEmpty(pixel_shader.filename))
                    {
                        pixel_shader.status = BeDx12AutoGenStatus::kSuccess;
                    }
                    else
                    {
                        pixel_shader.status = BeDx12AutoGenStatus::kFailed;
                        rc = beKA::beStatus::kBeStatusDxcCannotAutoGeneratePixelShader;
                    }
                }
            }

        }
    }
    return rc;
}

bool GenerateFileName(const std::string& autogen_dir, const std::string& base_file_name, const std::string& file_ext, std::string& filename)
{
    std::stringstream ss;
    bool              is_directory = KcUtils::IsDirectory(autogen_dir);
    if (is_directory)
    {
        ss << "rga_autogen_";
        auto        now  = std::chrono::system_clock::now();
        std::time_t time = std::chrono::system_clock::to_time_t(now);
        std::tm     tm;
        localtime_s(&tm, &time);
        ss << std::put_time(&tm, "%Y%m%d_%H%M%S_");
    }
    ss << base_file_name;
    return KcUtils::ConstructOutFileName(autogen_dir, ss.str(), "", file_ext, filename, !is_directory);
}

bool BeDx12AutoGenerator::GenerateRootSignatureFileName(const std::string& autogen_dir, std::string& filename)
{
    return GenerateFileName(autogen_dir, "", kTempRootSignatureFileExtension, filename);
}

bool BeDx12AutoGenerator::GenerateGpsoFileName(const std::string& autogen_dir, std::string& filename)
{
    return GenerateFileName(autogen_dir, "", kTempGpsoFileExtension, filename);
}

bool BeDx12AutoGenerator::GenerateVertexShaderFileName(const std::string& autogen_dir, std::string& filename)
{
    return GenerateFileName(autogen_dir, "vs", kTempShaderFileExtension, filename);
}

bool BeDx12AutoGenerator::GeneratePixelShaderFileName(const std::string& autogen_dir, std::string& filename)
{
    return GenerateFileName(autogen_dir, "ps", kTempShaderFileExtension, filename);
}

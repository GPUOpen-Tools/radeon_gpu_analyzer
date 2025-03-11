//=================================================================
// Copyright 2024 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

// Local.
#include "radeon_gpu_analyzer_backend/autogen/be_reflection_dx12.h"
#include "radeon_gpu_analyzer_backend/autogen/be_utils_dx12.h"

beKA::beStatus BeDx12Reflection::AutoGenerateFiles(IDxcUtils*                dxc_utils,
                                                   const BeDx12AutoGenInput& refection_input,
                                                   HlslOutput&               hlsl_output,
                                                   std::stringstream&        err) const
{
    assert(dxc_utils != nullptr);
    beKA::beStatus    rc   = beKA::beStatus::kBeStatusSuccess;
    if (dxc_utils == nullptr)
    {
        rc = beKA::beStatus::kBeStatusDxcCheckHrFailed;
    }
    if (rc == beKA::beStatus::kBeStatusSuccess)
    {
        // Clear output.
        hlsl_output = {};

        const bool is_compute = !refection_input.cs_blob.empty();
        if (is_compute)
        {
            // Generate root signature.
            DxcReflectionOutput cs_reflection_output;
            rc = CreateReflection(dxc_utils, refection_input.cs_blob, cs_reflection_output);
            if (rc == beKA::beStatus::kBeStatusSuccess && !refection_input.is_root_signature_specified)
            {
                rc = GenerateRootSignatureCompute(cs_reflection_output, hlsl_output.root_signature, err);
            }
        }
        else
        {
            const bool       has_vs                = !refection_input.vs_blob.empty();
            const bool       has_ps                = !refection_input.ps_blob.empty();
            UINT64           shader_requires_flags = 0;
            DxcReflectionOutput vs_reflection_output = {};
            DxcReflectionOutput ps_reflection_output = {};
            if (has_vs)
            {
                rc = CreateReflection(dxc_utils, refection_input.vs_blob, vs_reflection_output);
                if (rc == beKA::beStatus::kBeStatusSuccess)
                {
                    shader_requires_flags |= vs_reflection_output.shader_reflection_ptr->GetRequiresFlags();
                }
            }
            if (has_ps)
            {
                rc = CreateReflection(dxc_utils, refection_input.ps_blob, ps_reflection_output);
                if (rc == beKA::beStatus::kBeStatusSuccess)
                {
                    shader_requires_flags |= ps_reflection_output.shader_reflection_ptr->GetRequiresFlags();
                }
            }

            if (rc == beKA::beStatus::kBeStatusSuccess)
            {

                // Generate root signature.
                if (!refection_input.is_root_signature_specified)
                {
                    rc = GenerateRootSignatureGraphics(
                        shader_requires_flags, has_vs, vs_reflection_output, has_ps, ps_reflection_output, hlsl_output.root_signature, err);
                }

                if (rc == beKA::beStatus::kBeStatusSuccess)
                {
                    // Genererate VS/PS Shader.
                    if (!has_vs)
                    {
                        rc = GenerateVertexShader(refection_input.parsed_gpso_file, ps_reflection_output, hlsl_output.vs, err);
                    }
                    if (!has_ps)
                    {
                        rc = GeneratePixelShader(refection_input.parsed_gpso_file, vs_reflection_output, hlsl_output.ps, err);
                    }
                }

                if (rc == beKA::beStatus::kBeStatusSuccess && !refection_input.parsed_gpso_file)
                {
                    // Generate GPSO.
                    rc = GenerateGpso(shader_requires_flags, has_vs, vs_reflection_output, has_ps, ps_reflection_output, hlsl_output.gpso, err);
                }

            }
        }
    }
    return rc;
}

beKA::beStatus BeDx12Reflection::CreateReflection(IDxcUtils*                    dxc_utils, 
                                                  const BeDx12ShaderBinaryBlob& src_blob,
                                                  DxcReflectionOutput&          dxc_output) const
{
    assert(dxc_utils != nullptr);
    beKA::beStatus rc = beKA::beStatus::kBeStatusSuccess;
    // Clear.
    dxc_output = {};

    DxcBuffer shader_data_buffer = {};
    shader_data_buffer.Ptr       = src_blob.data();
    shader_data_buffer.Size      = src_blob.size();

    rc = BeDx12Utils::CheckHr(dxc_utils->CreateReflection(&shader_data_buffer, IID_PPV_ARGS(&dxc_output.shader_reflection_ptr)));
    if (rc == beKA::beStatus::kBeStatusSuccess && dxc_output.shader_reflection_ptr)
    {
        rc = BeDx12Utils::CheckHr(dxc_output.shader_reflection_ptr->GetDesc(&dxc_output.shader_desc));
        if (rc == beKA::beStatus::kBeStatusSuccess)
        {
            D3D12_SHADER_INPUT_BIND_DESC bind_desc = {};
            for (UINT i = 0; i < dxc_output.shader_desc.BoundResources; ++i)
            {
                rc = BeDx12Utils::CheckHr((dxc_output.shader_reflection_ptr->GetResourceBindingDesc(i, &bind_desc)));
                if (rc != beKA::beStatus::kBeStatusSuccess)
                {
                    break;
                }
                dxc_output.resource_bindings.push_back(bind_desc);
            }

            if (rc == beKA::beStatus::kBeStatusSuccess)
            {
                D3D12_SIGNATURE_PARAMETER_DESC param_desc = {};
                for (UINT i = 0; i < dxc_output.shader_desc.InputParameters; ++i)
                {
                    rc = BeDx12Utils::CheckHr((dxc_output.shader_reflection_ptr->GetInputParameterDesc(i, &param_desc)));
                    if (rc != beKA::beStatus::kBeStatusSuccess)
                    {
                        break;
                    }
                    dxc_output.param_desc_inputs.push_back(param_desc);
                }
                if (rc == beKA::beStatus::kBeStatusSuccess)
                {
                    for (UINT i = 0; i < dxc_output.shader_desc.OutputParameters; ++i)
                    {
                        rc = BeDx12Utils::CheckHr((dxc_output.shader_reflection_ptr->GetOutputParameterDesc(i, &param_desc)));
                        if (rc != beKA::beStatus::kBeStatusSuccess)
                        {
                            break;
                        }
                        dxc_output.param_desc_outputs.push_back(param_desc);
                    }
                }
            }
        }
    }
    return rc;
}

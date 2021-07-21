#pragma once

// C++.
#include <string>

// DX.
#include <dxgi.h>
#include <dxgi1_2.h>
#include <dxgi1_3.h>
#include <dxgi1_4.h>
#include <wrl.h>

// Local.
#include "rg_dx12_data_types.h"
#include "rg_dx12_backend.h"

using namespace Microsoft::WRL;

namespace rga
{
    class rgDx12Frontend
    {
    public:
        rgDx12Frontend(D3D_FEATURE_LEVEL feature_level) : feature_level_(feature_level) {}

        // Init.
        bool Init(bool is_dxr_session);

        // Retrieve the list of supported targets and their driver IDs.
        bool GetSupportedTargets(std::vector<std::string>& supported_targets,
            std::map<std::string, unsigned>& target_to_id) const;

        // Compile compute pipeline.
        bool CompileComputePipeline(const RgDx12Config& config, std::string& error_msg) const;

        // Compile graphics pipeline.
        bool CompileGraphicsPipeline(const RgDx12Config& config, std::string& error_msg) const;

#ifdef RGA_DXR_ENABLED
        // Compile ray tracing pipeline.
        bool CompileRayTracingPipeline(const RgDx12Config& config, std::string& error_msg) const;

        // Compile ray tracing shader.
        bool CompileRayTracingShader(const RgDx12Config& config, std::string& error_msg) const;
#endif

    private:
        bool GetHardwareAdapter(IDXGIAdapter1** dxgi_dapter);

        bool CreateComputePipeline(const RgDx12Config& config,
            D3D12_COMPUTE_PIPELINE_STATE_DESC*& pso,
            std::string& error_msg) const;

        bool CreateGraphicsPipeline(const RgDx12Config& config,
            D3D12_GRAPHICS_PIPELINE_STATE_DESC*& pso,
            std::string& error_msg) const;

        bool ExtractRootSignature(const D3D12_SHADER_BYTECODE& bytecode,
            ID3D12RootSignature*& root_signature, std::string& error_msg) const;

        RgDx12Backend backend_;
        ComPtr<ID3D12Device> device_;
        ComPtr<ID3D12Device5> dxr_device_;
        D3D_FEATURE_LEVEL feature_level_ = D3D_FEATURE_LEVEL_12_0;
    };
}

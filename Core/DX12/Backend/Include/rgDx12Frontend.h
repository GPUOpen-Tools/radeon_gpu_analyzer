#pragma once

// Local.
#include <rgDx12DataTypes.h>
#include <rgDx12Backend.h>

// C++.
#include <string>

// DX.
#include <dxgi.h>
#include <dxgi1_2.h>
#include <dxgi1_3.h>
#include <dxgi1_4.h>
#include <wrl.h>
using namespace Microsoft::WRL;

namespace rga
{
    class rgDx12Frontend
    {
    public:
        // Init.
        bool Init();

        // Retrieve the list of supported targets and their driver IDs.
        bool GetSupportedTargets(std::vector<std::string>& supportedTargets,
            std::map<std::string, unsigned>& targetToId) const;

        // Compile compute pipeline.
        bool CompileComputePipeline(const rgDx12Config& config, std::string& errorMsg) const;

        // Compile graphics pipeline.
        bool CompileGraphicsPipeline(const rgDx12Config& config, std::string& errorMsg) const;

    private:
        bool GetHardwareAdapter(IDXGIAdapter1** ppAdapter);

        bool CreateComputePipeline(const rgDx12Config& config,
            D3D12_COMPUTE_PIPELINE_STATE_DESC*& pPso,
            std::string& errorMsg) const;

        bool CreateGraphicsPipeline(const rgDx12Config& config,
            D3D12_GRAPHICS_PIPELINE_STATE_DESC*& pPso,
            std::string& errorMsg) const;

        bool ExtractRootSignature(const D3D12_SHADER_BYTECODE& bytecode,
            ID3D12RootSignature*& pRootSignature, std::string& errorMsg) const;

        rgDx12Backend m_backend;
        ComPtr<ID3D12Device> m_device;
    };
}

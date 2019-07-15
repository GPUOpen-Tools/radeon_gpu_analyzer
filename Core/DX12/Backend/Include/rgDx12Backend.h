#pragma once
#include <rgDx12Api.h>

// C++.
#include <string>
#include <vector>
#include <map>

// D3D12.
#include <d3d12.h>

namespace rga
{
    // Per-shader compilation results.
    struct rgDx12ShaderResults
    {
        rgDx12ShaderResults() = default;
        ~rgDx12ShaderResults();

        // Disassembly.
        char* pDisassembly = NULL;

        // Number of VGPRs consumed by shader.
        size_t vgprUsed = 0;

        // Number of VGPRs made available to shader.
        size_t vgprAvailable = 0;

        // Number of physically available VGPRs.
        size_t vgprPhysical = 0;

        // Number of SGPRs consumed by shader.
        size_t sgprUsed = 0;

        // Number of SGPRs made available to shader.
        size_t sgprAvailable = 0;

        // Number of physically available SGPRs.
        size_t sgprPhysical = 0;

        // LDS consumed in bytes.
        size_t ldsUsedBytes = 0;

        // LDS size per thread group in bytes.
        size_t ldsAvailableBytes = 0;

        // Scratch memory consumed in bytes.
        size_t scratchUsedBytes = 0;

        // Shader mask that tells if stages had been
        // merged in case that multiple bits are set.
        unsigned shaderMask = 0;
    };

    // Graphics pipeline compilation results.
    struct RGA_API rgDx12PipelineResults
    {
        rgDx12ShaderResults m_vertex;
        rgDx12ShaderResults m_hull;
        rgDx12ShaderResults m_domain;
        rgDx12ShaderResults m_geometry;
        rgDx12ShaderResults m_pixel;
    };

    // Thread group dimensions.
    struct RGA_API rgDx12ThreadGroupSize
    {
        // Number of compute threads per thread group in X dimension.
        uint32_t x = 0;

        // Number of compute threads per thread group in Y dimension.
        uint32_t y = 0;

        // Number of compute threads per thread group in Z dimension.
        uint32_t z = 0;
    };

    class RGA_API rgDx12Backend
    {
    public:
        rgDx12Backend();
        ~rgDx12Backend();

        // Initialize rgDx12Backend instance.
        // This function expects a pointer to a valid ID3D12Device, created by the caller.
        // Returns true on success, false otherwise.
        bool Init(ID3D12Device* pD3D12Device);

        // Retrieve the list of supported GPUs for offline compilation and
        // a map that correlates between target names and their driver IDs.
        bool GetSupportedTargets(std::vector<std::string>& supportedTargets,
            std::map<std::string, unsigned>& targetToId) const;

        // Compile a graphics pipeline to generate the disassembly and compiler resource usage info.
        // pGraphicsPso should be fully set up for pipeline creation, and reference all shaders.
        // The output would be set into results.
        // Any error messages would be set into errorMsg.
        // Returns true on success, false otherwise.
        bool CompileGraphicsPipeline(const D3D12_GRAPHICS_PIPELINE_STATE_DESC* pGraphicsPso,
            rgDx12PipelineResults& results,
            std::string& errorMsg) const;

        // Compile a compute pipeline to generate the disassembly, compiler resource usage info
        // and thread group dimensions.
        // pComputePso should be fully set up for pipeline creation, and reference the shader.
        // The output would be set into computeShaderStats, threadGroupSize.
        // Any error messages would be set into errorMsg.
        // Returns true on success, false otherwise.
        bool CompileComputePipeline(const D3D12_COMPUTE_PIPELINE_STATE_DESC* pComputePso,
            rgDx12ShaderResults& computeShaderStats,
            rgDx12ThreadGroupSize& threadGroupSize,
            std::string& errorMsg) const;

    private:
        class Impl;
        Impl* m_pImpl;

        // No copy.
        rgDx12Backend(const rgDx12Backend& other) = delete;
        const rgDx12Backend& operator=(const rgDx12Backend& other) = delete;
    };
}
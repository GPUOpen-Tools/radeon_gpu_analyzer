#ifndef RGA_CORE_DX12_BACKEND_SRC_RG_DX12_BACKEND_H_
#define RGA_CORE_DX12_BACKEND_SRC_RG_DX12_BACKEND_H_

// C++.
#include <string>
#include <vector>
#include <map>

// D3D12.
#include <d3d12.h>

// Local.
#include "rg_dx12_data_types.h"

namespace rga
{
    // Per-shader compilation results.
    struct RgDx12ShaderResults
    {
        RgDx12ShaderResults() = default;
        virtual ~RgDx12ShaderResults();

        // Disassembly.
        char* disassembly = nullptr;

        // Number of VGPRs consumed by shader.
        size_t vgpr_used = 0;

        // Number of VGPRs made available to shader.
        size_t vgpr_available = 0;

        // Number of physically available VGPRs.
        size_t vgpr_physical = 0;

        // Number of SGPRs consumed by shader.
        size_t sgpr_used = 0;

        // Number of SGPRs made available to shader.
        size_t sgpr_available = 0;

        // Number of physically available SGPRs.
        size_t sgpr_physical = 0;

        // LDS consumed in bytes.
        size_t lds_used_bytes = 0;

        // LDS size per thread group in bytes.
        size_t lds_available_bytes  = 0;

        // Scratch memory consumed in bytes.
        size_t scratch_used_bytes = 0;

        // Shader assembly size in bytes.
        size_t code_size_bytes = 0;

        // Shader mask that tells if stages had been
        // merged in case that multiple bits are set.
        unsigned shader_mask = 0;
    };

    struct RgDx12ShaderResultsRayTracing : public RgDx12ShaderResults
    {
        // Stack size for ray tracing shader.
        size_t stack_size_bytes = 0;
        bool is_inlined = false;
    };

    // Graphics pipeline compilation results.
    struct RgDx12PipelineResults
    {
        RgDx12ShaderResults vertex;
        RgDx12ShaderResults hull;
        RgDx12ShaderResults domain;
        RgDx12ShaderResults geometry;
        RgDx12ShaderResults pixel;
    };

    // Thread group dimensions.
    struct RgDx12ThreadGroupSize
    {
        // Number of compute threads per thread group in X dimension.
        uint32_t x = 0;

        // Number of compute threads per thread group in Y dimension.
        uint32_t y = 0;

        // Number of compute threads per thread group in Z dimension.
        uint32_t z = 0;
    };

    class RgDx12Backend
    {
    public:
        RgDx12Backend();
        ~RgDx12Backend();

        // Initialize rgDx12Backend instance.
        // This function expects a pointer to a valid ID3D12Device, created by the caller.
        // Returns true on success, false otherwise.
        bool Init(ID3D12Device* d3d12_device);

        // Retrieve the list of supported GPUs for offline compilation and
        // a map that correlates between target names and their driver IDs.
        bool GetSupportedTargets(std::vector<std::string>& supported_targets,
            std::map<std::string, unsigned>& target_to_id) const;

        // Compile a graphics pipeline to generate the disassembly and compiler resource usage info.
        // graphics_pso should be fully set up for pipeline creation, and reference all shaders.
        // The output would be set into results, pipeline_binary.
        // Any error messages would be set into error_msg.
        // Returns true on success, false otherwise.
        bool CompileGraphicsPipeline(const D3D12_GRAPHICS_PIPELINE_STATE_DESC* graphics_pso,
            RgDx12PipelineResults& results,
            std::vector<char>& pipeline_binary,
            std::string& error_msg) const;

        // Compile a compute pipeline to generate the disassembly, compiler resource usage info
        // and thread group dimensions.
        // compute_pso should be fully set up for pipeline creation, and reference the shader.
        // The output would be set into compute_shader_stats, thread_group_size, pipeline_binary.
        // Any error messages would be set into error_msg.
        // Returns true on success, false otherwise.
        bool CompileComputePipeline(const D3D12_COMPUTE_PIPELINE_STATE_DESC* compute_pso,
            RgDx12ShaderResults& compute_shader_stats,
            RgDx12ThreadGroupSize& thread_group_size,
            std::vector<char>& pipeline_binary,
            std::string& error_msg) const;
#ifdef RGA_DXR_ENABLED
        // Compile a ray tracing pipeline to generate the disassembly and compiler resource usage info,
        // and extract the results for a single pipeline (designated by the raygeneration shader name).
        // ray_tracing_state_object should contain all required data for the DXR State Object creation.
        // Any error messages would be set into error_msg.
        // Returns true on success, false otherwise.
        bool CreateStateObject(const D3D12_STATE_OBJECT_DESC* ray_tracing_state_object,
            const std::wstring& raygen_shader_name,
            std::vector<std::shared_ptr<RgDx12ShaderResultsRayTracing>>& raytracing_shader_stats,
            std::vector<unsigned char>& pipeline_binary,
            bool& is_unified_mode,
            std::vector<std::string>& indirect_shader_names,
            std::string& error_msg) const;

        // Compile a ray tracing shader to generate the disassembly and compiler resource usage info,
        // and extract the results for a single pipeline (designated by the raygeneration shader name).
        // ray_tracing_state_object should contain all required data for the DXR State Object creation.
        // Any error messages would be set into error_msg.
        // Returns true on success, false otherwise.
        bool CreateStateObjectShader(const D3D12_STATE_OBJECT_DESC* ray_tracing_state_object,
            const std::wstring& shader_name,
            RgDx12ShaderResultsRayTracing& shader_stats,
            std::vector<unsigned char>& pipeline_binary,
            std::string& error_msg) const;

        // Compile a ray tracing pipeline to generate the disassembly and compiler resource usage info,
        // and extract the results for all generated pipelines.
        // ray_tracing_state_object should contain all required data for the DXR State Object creation.
        // Any error messages would be set into error_msg.
        // Returns true on success, false otherwise.
        bool CreateStateObjectAll(const RgDx12Config& config,
            const D3D12_STATE_OBJECT_DESC* ray_tracing_state_object,
            std::vector<std::shared_ptr<RgDx12ShaderResultsRayTracing>>& raytracing_shader_stats,
            std::vector<std::shared_ptr<std::vector<unsigned char>>>& pipeline_binary,
            std::string& error_msg) const;
#endif

    private:
        class Impl;
        Impl* impl_;

        // No copy.
        RgDx12Backend(const RgDx12Backend& other) = delete;
        const RgDx12Backend& operator=(const RgDx12Backend& other) = delete;
    };
}
#endif // RGA_CORE_DX12_BACKEND_SRC_RG_DX12_BACKEND_H_
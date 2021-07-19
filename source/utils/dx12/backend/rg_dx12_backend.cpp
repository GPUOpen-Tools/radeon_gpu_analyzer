// C++.
#include <sstream>
#include <fstream>
#include <cassert>
#include <algorithm>

// TinyXML2.
#include "tinyxml2.h"

// DX.
#include <d3dcommon.h>

// DX - AMD.
#include "AmdExtD3D.h"
#include "AmdExtD3DShaderAnalyzerApi.h"

// Local.
#include "rg_dx12_backend.h"
#include "rg_dx12_utils.h"

namespace rga
{
    // *** CONSTANTS - START ***

    // XML.
    static const char* kStrXmlNodeComments = "comments";
    static const char* kStrXmlNodeStage = "stage";
    static const char* kStrXmlNodeComment = "comment";
    static const char* kStrXmlNodeVs = "VS";
    static const char* kStrXmlNodeHs = "HS";
    static const char* kStrXmlNodeDs = "DS";
    static const char* kStrXmlNodeGs = "GS";
    static const char* kStrXmlNodePs = "PS";
    static const char* kStrXmlNodeCs = "CS";

    // Messages: errors.
    static const char* kStrErrorPipelineCreateFailure = "Error: failed to create D3D12 graphics pipeline with error code: ";
    static const char* kStrErrorPipelineStateCreateFailure = "Error: failed to create compute pipeline state.";
    static const char* kStrErrorPipelineBinarySizeQueryFailure = "Error: failed to retrieve pipeline binary size ";
    static const char* kStrErrorPipelineBinaryExtractionFailure = "Error: failed to extract pipeline binary ";
    static const char* kStrErrorStatisticsExtractionFailure = "Error: failed to retrieve hardware resource usage statistics ";
    static const char* kStrErrorDisassemblySizeExtractionFailure = "Error: failed to retrieve disassembly buffer size ";
    static const char* kStrErrorDisassemblyContentsExtractionFailurePiplineNumber = "for pipeline #";
    static const char* kStrErrorDisassemblyContentsExtractionFailure = "Error: failed to retrieve disassembly ";
    static const char* kStrErrorPipelineBinaryCountExtractionFailure = "Error: failed to retrieve the pipeline binary count ";
    static const char* kStrErrorPipelineIdentifiedByRaygenShaderName = " for pipeline identified by raygeneration shader name ";
    static const char* kStrErrorPerShaderName = " for shader ";
    static const char* kStrErrorPipelineIdentifiedByIndex = "for pipeline #";
    static const char* kStrErrorPipelineIdentifiedByIndexPipelineBinary = " for pipeline #";
    static const char* kStrErrorDxrEmptyShaderName = "Error: empty shader name, for shader #";
    static const char* kStrErrorDxrFailedToRetrieveShaderName = "Error: failed to retrieve shader name, for shader #";
    static const char* kStrErrorDxrFailedToCheckPipelineCompilationType = "Error: failed to check pipeline compilation type (Indirect/Unified) for pipeline #";
    static const char* kStrErrorDxrFailedToCreateStateObject = "Error: failed to create DXR state object. This is likely caused by an error in the state subobject definition.";
    static const char* kStrErrorDxrFailedToRetrievePipelineCount = "Error: failed to retrieve DXR pipeline count.";
    static const char* kStrHintDebugOutput = "Hint: consider enabling the D3D12 layer adding by adding --debug-layer to your rga command, and inspecting the Windows Debug Output for more detailed information on the actual error. "
        "The Windows Debug Output can be monitored by tools like https://docs.microsoft.com/en-us/sysinternals/downloads/debugview";

    // Messages: warnings.
    static const char* kStrWarningMoreThanSinglePipelineBinaryForSinglePipeline1 = "Warning: more than a single pipeline binary detected ";
    static const char* kStrWarningMoreThanSinglePipelineBinaryForSinglePipeline2 = " - only the first pipeline would be extracted.";

    // Messages: hints.
    static const char* kStrHintWrongShaderName1 = " even though state object was created successfully. Please check export name spelling (export names are case sensitive)";
    static const char* kStrHintWrongShaderName2 = ".";
    static const char* kStrHintWrongShaderName2Raygen = " and make sure that the export name is of a raygeneration shader.";

    // *** CONSTANTS - END ***

    // *** INTERNALLY LINKED UTILITIES - START ***

    // Extracts the XML file for the given pipeline state, and allocates the contents in the given buffer.
    // In case of any error messages, they will be set into error_msg.
    // Returns true on success, false otherwise.
    static bool ExtractXmlContent(IAmdExtD3DShaderAnalyzer1* amd_shader_analyzer_ext,
        AmdExtD3DPipelineHandle* pipeline_handle, char*& xml_buffer, std::string& error_msg)
    {
        bool ret = false;
        size_t isa_size = 0;
        AmdExtD3DPipelineDisassembly disassembly;
        HRESULT hr = amd_shader_analyzer_ext->GetShaderIsaCode(*pipeline_handle, nullptr, &isa_size, &disassembly);
        assert(hr == S_OK);
        assert(isa_size > 0);
        ret = (hr == S_OK && isa_size > 0);
        if (ret)
        {
            xml_buffer = new char[isa_size] {};
            hr = amd_shader_analyzer_ext->GetShaderIsaCode(*pipeline_handle, xml_buffer, &isa_size, &disassembly);
            assert(hr == S_OK);
            ret = (hr == S_OK);
        }

        return ret;
    }

    // Retrieve the disassembly for the given pipeline state object.
    // The given pipeline state object is expected to be a valid object, which was received from a
    // call to CreateGraphicsPipeline or CreateComputePipeline using the same ID3D12Device which
    // was used in the call to rgDx12Backend::Init().
    // Returns true on success, false otherwise.
    static bool RetrieveDisassemblyGraphics(IAmdExtD3DShaderAnalyzer1* amd_shader_analyzer_ext,
        AmdExtD3DPipelineHandle* d3d12_pipeline_state,
        RgDx12PipelineResults& results, std::string& error_msg)
    {
        bool ret = false;

        // Parse the XML string.
        tinyxml2::XMLDocument doc;
        char* xml_buffer = NULL;
        ret = ExtractXmlContent(amd_shader_analyzer_ext, d3d12_pipeline_state, xml_buffer, error_msg);
        assert(ret);
        assert(xml_buffer != NULL);
        if (ret && xml_buffer != NULL)
        {
            tinyxml2::XMLError rc = doc.Parse(xml_buffer);
            assert(rc == tinyxml2::XML_SUCCESS);
            if (rc == tinyxml2::XML_SUCCESS)
            {
                // Retrieve the XML file and the comments node.
                tinyxml2::XMLElement* comments_node = doc.FirstChildElement(kStrXmlNodeComments);
                assert(comments_node != nullptr);
                if (comments_node != nullptr)
                {
                    // Extract the disassembly for all shaders in the pipeline.
                    for (tinyxml2::XMLElement* shader_node = comments_node->FirstChildElement();
                        shader_node != NULL; shader_node = shader_node->NextSiblingElement())
                    {
                        assert(shader_node != nullptr);
                        if (shader_node != nullptr)
                        {
                            std::string shader_type = shader_node->Attribute(kStrXmlNodeStage);
                            assert(!shader_type.empty());
                            if (!shader_type.empty())
                            {
                                tinyxml2::XMLElement* disassembly_node = shader_node->FirstChildElement(kStrXmlNodeComment);
                                assert(disassembly_node != nullptr);
                                if (disassembly_node != nullptr)
                                {
                                    const char* disassembly = disassembly_node->GetText();
                                    assert(disassembly != NULL);
                                    if (disassembly != NULL)
                                    {
                                        // Clean up the string.
                                        std::string fixed_str = disassembly;
                                        size_t sz = fixed_str.size();
                                        fixed_str = RgDx12Utils::TrimNewline(fixed_str);
                                        disassembly = fixed_str.data();
                                        assert(sz > 0);
                                        if (sz > 0)
                                        {
                                            // Assign the disassembly to the correct shader.
                                            if (shader_type.compare(kStrXmlNodeVs) == 0)
                                            {
                                                results.vertex.disassembly = new char[sz] {};
                                                memcpy(results.vertex.disassembly, disassembly, sz);
                                            }
                                            else if (shader_type.compare(kStrXmlNodeHs) == 0)
                                            {
                                                results.hull.disassembly = new char[sz] {};
                                                memcpy(results.hull.disassembly, disassembly, sz);
                                            }
                                            else if (shader_type.compare(kStrXmlNodeDs) == 0)
                                            {
                                                results.domain.disassembly = new char[sz] {};
                                                memcpy(results.domain.disassembly, disassembly, sz);
                                            }
                                            else if (shader_type.compare(kStrXmlNodeGs) == 0)
                                            {
                                                results.geometry.disassembly = new char[sz] {};
                                                memcpy(results.geometry.disassembly, disassembly, sz);
                                            }
                                            else if (shader_type.compare(kStrXmlNodePs) == 0)
                                            {
                                                results.pixel.disassembly = new char[sz] {};
                                                memcpy(results.pixel.disassembly, disassembly, sz);
                                            }
                                            else
                                            {
                                                assert(false);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        // Free the XML buffer memory.
        if (xml_buffer != NULL)
        {
            delete[] xml_buffer;
            xml_buffer = NULL;
        }

        ret = (results.vertex.disassembly != nullptr ||
            results.hull.disassembly != nullptr ||
            results.domain.disassembly != nullptr ||
            results.geometry.disassembly != nullptr ||
            results.pixel.disassembly != nullptr);

        return ret;
    }

    static bool RetrieveDisassemblyCompute(IAmdExtD3DShaderAnalyzer1* amd_shader_analyzer_ext,
        AmdExtD3DPipelineHandle* d3d12_pipeline_state,
        RgDx12ShaderResults& results, std::string& error_msg)
    {
        bool ret = false;

        // Parse the XML string.
        tinyxml2::XMLDocument doc;
        char* xml_buffer = NULL;
        ret = ExtractXmlContent(amd_shader_analyzer_ext, d3d12_pipeline_state, xml_buffer, error_msg);
        assert(ret);
        assert(xml_buffer != NULL);
        if (ret && xml_buffer != NULL)
        {
            tinyxml2::XMLError rc = doc.Parse(xml_buffer);
            assert(rc == tinyxml2::XML_SUCCESS);
            if (rc == tinyxml2::XML_SUCCESS)
            {
                // Retrieve the XML file and the comments node.
                tinyxml2::XMLElement* comments_node = doc.FirstChildElement(kStrXmlNodeComments);
                assert(comments_node != nullptr);
                if (comments_node != nullptr)
                {
                    // Retrieve the compute shader disassembly.
                    tinyxml2::XMLElement* shader_node = comments_node->FirstChildElement();
                    assert(shader_node != nullptr);
                    if (shader_node != nullptr)
                    {
                        std::string shader_type = shader_node->Attribute(kStrXmlNodeStage);
                        assert(shader_type.compare(kStrXmlNodeCs) == 0);
                        if (shader_type.compare(kStrXmlNodeCs) == 0)
                        {
                            tinyxml2::XMLElement* disassembly_node =
                                shader_node->FirstChildElement(kStrXmlNodeComment);
                            assert(disassembly_node != nullptr);
                            if (disassembly_node != nullptr)
                            {
                                const char* disassembly = disassembly_node->GetText();
                                assert(disassembly_node != NULL);
                                if (disassembly_node != NULL)
                                {
                                    size_t sz = strlen(disassembly);
                                    assert(sz > 0);
                                    if (sz > 0)
                                    {
                                        results.disassembly = new char[sz] {};
                                        memcpy(results.disassembly, disassembly, sz);
                                        results.disassembly[sz - 1] = '\0';
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        // Free the XML buffer memory.
        if (xml_buffer != NULL)
        {
            delete[] xml_buffer;
            xml_buffer = NULL;
        }

        return ret;
    }

    static bool ExtractPipelineBinary(IAmdExtD3DShaderAnalyzer1* amd_shader_analyzer_ext,
        AmdExtD3DPipelineHandle* pipeline_handle, std::vector<char>& pipeline_binary_data, std::string& error_msg)
    {
        bool ret = false;
        AmdExtD3DPipelineElfHandle pipeline_binary = nullptr;
        uint32_t pipeline_binary_bytes = 0;
        HRESULT hr = amd_shader_analyzer_ext->GetPipelineElfBinary(*pipeline_handle, pipeline_binary, &pipeline_binary_bytes);
        assert(hr == S_OK);
        assert(pipeline_binary_bytes > 0);
        if (hr == S_OK && pipeline_binary_bytes > 0)
        {
            pipeline_binary_data.resize(pipeline_binary_bytes);
            void* data_buffer = static_cast<void*>(pipeline_binary_data.data());
            hr = amd_shader_analyzer_ext->GetPipelineElfBinary(*pipeline_handle, data_buffer, &pipeline_binary_bytes);
            assert(hr == S_OK);
            ret = (hr == S_OK);
            if (!ret)
            {
                error_msg.append(kStrErrorPipelineBinaryExtractionFailure);
                error_msg.append(".");
            }
        }
        else
        {
            error_msg.append(kStrErrorPipelineBinarySizeQueryFailure);
            ret = false;
        }
        return ret;
    }

    static void ExtractRayTracingStats(RgDx12ShaderResultsRayTracing& raytracing_shader_stats,
        const AmdExtD3DShaderStatsRayTracing& rt_stats)
    {
        // SGPRs.
        raytracing_shader_stats.sgpr_available = rt_stats.numAvailableSgprs;
        raytracing_shader_stats.sgpr_used = rt_stats.usageStats.numUsedSgprs;
        raytracing_shader_stats.sgpr_physical = rt_stats.numPhysicalSgprs;

        // VGPR.
        raytracing_shader_stats.vgpr_available = rt_stats.numAvailableVgprs;
        raytracing_shader_stats.vgpr_used = rt_stats.usageStats.numUsedVgprs;
        raytracing_shader_stats.vgpr_physical = rt_stats.numPhysicalVgprs;

        // LDS.
        raytracing_shader_stats.lds_available_bytes = rt_stats.usageStats.ldsSizePerThreadGroup;
        raytracing_shader_stats.lds_used_bytes = rt_stats.usageStats.ldsUsageSizeInBytes;

        // Scratch memory.
        raytracing_shader_stats.scratch_used_bytes = rt_stats.usageStats.scratchMemUsageInBytes;

        // Stack size.
        raytracing_shader_stats.stack_size_bytes = rt_stats.stackSizeInBytes;

        // Inline.
        raytracing_shader_stats.is_inlined = rt_stats.isInlined;
    }

    // *** INTERNALLY LINKED UTILITIES - END ***

    class RgDx12Backend::Impl
    {
    public:
        bool InitImpl(ID3D12Device* d3d12_device);

        bool GetSupportedTargets(std::vector<std::string>& supported_targets,
            std::map<std::string, unsigned>& target_to_id);

        bool CompileGraphicsPipeline(const D3D12_GRAPHICS_PIPELINE_STATE_DESC* graphics_pso,
            RgDx12PipelineResults& results,
            std::vector<char>& pipeline_binary,
            std::string& error_msg) const;

        bool CompileComputePipeline(const D3D12_COMPUTE_PIPELINE_STATE_DESC* compute_pso,
            RgDx12ShaderResults& shader_results,
            RgDx12ThreadGroupSize& thread_group_size,
            std::vector<char>& binary,
            std::string& error_msg) const;

        // Creates the state object and extracts the results for a single pipeline designated by the raygeneration shader name.
        bool CreateStateObject(const D3D12_STATE_OBJECT_DESC* ray_tracing_state_object,
            std::vector<std::shared_ptr<std::vector<std::shared_ptr<RgDx12ShaderResultsRayTracing>>>>& raytracing_shader_stats,
            std::vector<std::shared_ptr<std::vector<unsigned char>>>& pipeline_binary,
            std::vector<bool>& is_unified_mode, std::vector<std::shared_ptr<std::vector<std::string>>>& indirect_shader_names,
            std::string& error_msg) const;

        // Creates the state object and extracts the results for a single pipeline designated by the raygeneration shader name.
        bool CreateStateObjectShader(const D3D12_STATE_OBJECT_DESC* ray_tracing_state_object, const std::wstring& raygen_shader_name,
            RgDx12ShaderResultsRayTracing& raytracing_shader_stats, std::vector<unsigned char>& pipeline_binary, std::string& error_msg) const;

    private:
        HMODULE LoadUMDLibrary();
        HMODULE amd_d3d_dll_handle_ = NULL;
        IAmdExtD3DFactory* amd_ext_object_ = NULL;
        IAmdExtD3DShaderAnalyzer2* amd_shader_analyzer_ext_ = NULL;
    };

    static void SetShaderResults(const AmdExtD3DShaderStats& shader_stats, RgDx12ShaderResults& results)
    {
        // VGPRs.
        results.vgpr_used = shader_stats.usageStats.numUsedVgprs;
        results.vgpr_available = shader_stats.numAvailableVgprs;
        results.vgpr_physical = shader_stats.numPhysicalVgprs;

        // SGPRs.
        results.sgpr_used = shader_stats.usageStats.numUsedSgprs;
        results.sgpr_available = shader_stats.numAvailableSgprs;
        results.sgpr_physical = shader_stats.numPhysicalSgprs;

        // LDS.
        results.lds_used_bytes = shader_stats.usageStats.ldsUsageSizeInBytes;
        results.lds_available_bytes = shader_stats.usageStats.ldsSizePerThreadGroup;

        // Scratch memory.
        results.scratch_used_bytes = shader_stats.usageStats.scratchMemUsageInBytes;

        // Stage mask.
        results.shader_mask = shader_stats.shaderStageMask;
    }

    static void SetShaderResultsCompute(const AmdExtD3DComputeShaderStats& shader_stats,
        RgDx12ShaderResults& results, RgDx12ThreadGroupSize& thread_group_size)
    {
        // Set the common results.
        SetShaderResults(shader_stats, results);

        // Set the compute-specific results.
        thread_group_size.x = shader_stats.numThreadsPerGroupX;
        thread_group_size.y = shader_stats.numThreadsPerGroupY;
        thread_group_size.z = shader_stats.numThreadsPerGroupZ;
    }

    static void SetPipelineResults(const AmdExtD3DGraphicsShaderStats& stats, RgDx12PipelineResults& results)
    {
        SetShaderResults(stats.vertexShaderStats, results.vertex);
        SetShaderResults(stats.hullShaderStats, results.hull);
        SetShaderResults(stats.domainShaderStats, results.domain);
        SetShaderResults(stats.geometryShaderStats, results.geometry);
        SetShaderResults(stats.pixelShaderStats, results.pixel);
    }

    bool RgDx12Backend::Impl::InitImpl(ID3D12Device* d3d12_device)
    {
        // Load the user-mode driver.
        amd_d3d_dll_handle_ = LoadUMDLibrary();
        assert(amd_d3d_dll_handle_ != NULL);
        bool ret = (amd_d3d_dll_handle_ != NULL);
        if (ret)
        {
            PFNAmdExtD3DCreateInterface amd_ext_d3d_create_func =
                (PFNAmdExtD3DCreateInterface)GetProcAddress(amd_d3d_dll_handle_,
                    "AmdExtD3DCreateInterface");

            assert(amd_ext_d3d_create_func != NULL);
            if (amd_ext_d3d_create_func != NULL)
            {
                HRESULT hr = amd_ext_d3d_create_func(d3d12_device,
                    __uuidof(IAmdExtD3DFactory), reinterpret_cast<void**>(&amd_ext_object_));

                assert(hr == S_OK);
                if (hr == S_OK)
                {
                    hr = amd_ext_object_->CreateInterface(d3d12_device,
                        __uuidof(IAmdExtD3DShaderAnalyzer2),
                        reinterpret_cast<void**>(&amd_shader_analyzer_ext_));
                    assert(hr == S_OK);
                    assert(amd_shader_analyzer_ext_ != NULL);
                    ret = (hr == S_OK && amd_shader_analyzer_ext_ != NULL);
                }
            }
        }

        return ret;
    }

    bool RgDx12Backend::Init(ID3D12Device* d3d12_device)
    {
        bool ret = false;
        assert(impl_ != nullptr);
        if (impl_ != nullptr)
        {
            ret = impl_->InitImpl(d3d12_device);
        }
        return ret;
    }

    HMODULE RgDx12Backend::Impl::LoadUMDLibrary()
    {
        std::wstring umd_module_name = L"amdxc64.dll";
        HMODULE umd_dll = LoadLibrary(umd_module_name.c_str());
        return umd_dll;
    }

    bool RgDx12Backend::Impl::GetSupportedTargets(std::vector<std::string>& supported_targets,
        std::map<std::string, unsigned>& target_to_id)
    {
        assert(amd_shader_analyzer_ext_ != nullptr);
        bool ret = amd_shader_analyzer_ext_ != nullptr;
        if (ret)
        {
            // First call to get the available virtual GPUs
            // retrieves the number of supported virtual targets.
            AmdExtD3DGpuIdList gpu_id_list{};
            HRESULT hr = amd_shader_analyzer_ext_->GetAvailableVirtualGpuIds(&gpu_id_list);
            assert(gpu_id_list.numGpuIdEntries > 0);
            ret = (gpu_id_list.numGpuIdEntries > 0);
            if (ret)
            {
                // Second call retrieves the actual virtual GPUs.
                gpu_id_list.pGpuIdEntries = new AmdExtD3DGpuIdEntry[gpu_id_list.numGpuIdEntries]();
                hr = amd_shader_analyzer_ext_->GetAvailableVirtualGpuIds(&gpu_id_list);
                assert(hr == S_OK);
                ret = SUCCEEDED(hr);
                if (ret)
                {
                    // Look for the user's target of choice in the virtual GPU list.
                    for (unsigned i = 0; i < gpu_id_list.numGpuIdEntries; i++)
                    {
                        // Add the target name to the list of targets.
                        supported_targets.push_back(gpu_id_list.pGpuIdEntries[i].pGpuIdName);

                        // Track the mapping between the name to the ID.
                        target_to_id[gpu_id_list.pGpuIdEntries[i].pGpuIdName] = gpu_id_list.pGpuIdEntries[i].gpuId;
                    }
                }

                // Free the memory.
                delete[] gpu_id_list.pGpuIdEntries;
            }
        }
        return ret;
    }

    bool RgDx12Backend::GetSupportedTargets(std::vector<std::string>& supported_targets,
        std::map<std::string, unsigned>& target_to_id) const
    {
        bool ret = false;
        assert(impl_ != nullptr);
        if (impl_ != nullptr)
        {
            ret = impl_->GetSupportedTargets(supported_targets, target_to_id);
        }
        return ret;
    }

    bool RgDx12Backend::Impl::CompileGraphicsPipeline(const D3D12_GRAPHICS_PIPELINE_STATE_DESC* graphics_pso,
        RgDx12PipelineResults& results,
        std::vector<char>& pipeline_binary,
        std::string& error_msg) const
    {
        assert(amd_shader_analyzer_ext_ != nullptr);
        bool ret = amd_shader_analyzer_ext_ != nullptr;
        if (ret)
        {
            AmdExtD3DPipelineHandle* pipeline_handle = new AmdExtD3DPipelineHandle{};
            ID3D12PipelineState* pPipeline = NULL;
            AmdExtD3DGraphicsShaderStats stats;
            memset(&stats, 0, sizeof(AmdExtD3DGraphicsShaderStats));
            HRESULT hr = amd_shader_analyzer_ext_->CreateGraphicsPipelineState1(graphics_pso,
                IID_PPV_ARGS(&pPipeline), pipeline_handle);
            assert(hr == S_OK);
            ret = (hr == S_OK);

            if (ret)
            {
                amd_shader_analyzer_ext_->GetGraphicsShaderStats(*pipeline_handle, &stats);

                // Set the statistics values to the output structure.
                SetPipelineResults(stats, results);

                // Retrieve the disassembly.
                ret = RetrieveDisassemblyGraphics(amd_shader_analyzer_ext_, pipeline_handle, results, error_msg);
                assert(ret);

                // Retrieve the pipeline binary.
                ret = ExtractPipelineBinary(amd_shader_analyzer_ext_, pipeline_handle, pipeline_binary, error_msg);
                assert(ret);
            }
            else
            {
                // Log error messages.
                std::stringstream msg;
                msg << kStrErrorPipelineCreateFailure << hr;
                error_msg = msg.str();
            }
        }
        return ret;

    }

    bool RgDx12Backend::CompileGraphicsPipeline(const D3D12_GRAPHICS_PIPELINE_STATE_DESC* graphics_pso,
        RgDx12PipelineResults& results,
        std::vector<char>& pipeline_binary,
        std::string& error_msg) const
    {
        bool ret = false;
        assert(impl_ != nullptr);
        if (impl_ != nullptr)
        {
            ret = impl_->CompileGraphicsPipeline(graphics_pso, results, pipeline_binary, error_msg);
        }
        return ret;
    }

    bool RgDx12Backend::Impl::CompileComputePipeline(const D3D12_COMPUTE_PIPELINE_STATE_DESC* compute_pso,
        RgDx12ShaderResults& shader_results,
        RgDx12ThreadGroupSize& thread_group_size,
        std::vector<char>& binary,
        std::string& error_msg) const
    {
        assert(amd_shader_analyzer_ext_ != nullptr);
        bool ret = amd_shader_analyzer_ext_ != nullptr;
        if (ret)
        {
            ret = compute_pso != nullptr;
            assert(compute_pso != nullptr);
            if (ret)
            {
                AmdExtD3DPipelineHandle* pipeline_handle = new AmdExtD3DPipelineHandle{};
                ID3D12PipelineState* pPipeline = NULL;
                AmdExtD3DComputeShaderStats driverShaderStats;
                memset(&driverShaderStats, 0, sizeof(AmdExtD3DComputeShaderStats));
                HRESULT hr = amd_shader_analyzer_ext_->CreateComputePipelineState1(compute_pso,
                    IID_PPV_ARGS(&pPipeline), pipeline_handle);
                assert(hr == S_OK);
                assert(pPipeline != NULL);
                ret = (hr == S_OK && pPipeline != NULL);
                if (ret)
                {
                    amd_shader_analyzer_ext_->GetComputeShaderStats(*pipeline_handle, &driverShaderStats);

                    // Set results.
                    SetShaderResultsCompute(driverShaderStats, shader_results, thread_group_size);

                    // Retrieve the disassembly.
                    ret = RetrieveDisassemblyCompute(amd_shader_analyzer_ext_, pipeline_handle, shader_results, error_msg);
                    assert(ret);

                    // Retrieve the pipeline binary.
                    ret = ExtractPipelineBinary(amd_shader_analyzer_ext_, pipeline_handle, binary, error_msg);
                    assert(ret);
                }
                else
                {
                    error_msg.append(kStrErrorPipelineStateCreateFailure);
                    ret = false;
                }
            }
        }

        return ret;
    }

#ifdef RGA_DXR_ENABLED
    bool RgDx12Backend::Impl::CreateStateObject(const D3D12_STATE_OBJECT_DESC* ray_tracing_state_object,
        std::vector<std::shared_ptr<std::vector<std::shared_ptr<RgDx12ShaderResultsRayTracing>>>>& raytracing_shader_stats,
        std::vector<std::shared_ptr<std::vector<unsigned char>>>& pipeline_binary, std::vector<bool>& is_unified_mode,
        std::vector<std::shared_ptr<std::vector<std::string>>>& indirect_shader_names,
        std::string& error_msg) const
    {
        assert(amd_shader_analyzer_ext_ != nullptr);
        bool ret = amd_shader_analyzer_ext_ != nullptr;
        if (ret)
        {
            AmdExtD3DPipelineHandle* pipeline_handle = new AmdExtD3DPipelineHandle{};
            ID3D12StateObject* dxr_state_object = nullptr;
            HRESULT hr = amd_shader_analyzer_ext_->CreateStateObject(ray_tracing_state_object,
                IID_PPV_ARGS(&dxr_state_object), pipeline_handle);
            assert(SUCCEEDED(hr));
            ret = (SUCCEEDED(hr));
            if (ret)
            {
                // Retrieve the number of pipelines.
                uint32_t pipeline_count = 0;
                hr = amd_shader_analyzer_ext_->GetRayTracingPipelineElfCount(*pipeline_handle, &pipeline_count);
                assert(SUCCEEDED(hr));
                ret = (SUCCEEDED(hr));
                if (ret)
                {
                    // Allocate enough elements to store the flags for all generated pipelines.
                    is_unified_mode.resize(pipeline_count);

                    bool is_indirect_mode = false;
                    for (uint32_t pipeline_index = 0; pipeline_index < pipeline_count; ++pipeline_index)
                    {
                        // Tracking the shader names for the current pipeline.
                        std::shared_ptr<std::vector<std::string>> pipeline_shader_names = std::make_shared<std::vector<std::string>>();
                        std::shared_ptr<std::vector<std::shared_ptr<RgDx12ShaderResultsRayTracing>>> stats =
                            std::make_shared<std::vector<std::shared_ptr<RgDx12ShaderResultsRayTracing>>>();

                        // Check if the pipeline was compiled in Unified or Indirect mode.
                        hr = amd_shader_analyzer_ext_->IsRayTracingPipelineIndirect(*pipeline_handle, pipeline_index, &is_indirect_mode);
                        is_unified_mode[pipeline_index] = !is_indirect_mode;
                        assert(SUCCEEDED(hr));
                        if (SUCCEEDED(hr))
                        {
                            if (is_unified_mode[pipeline_index])
                            {
                                // Retrieve shader name.
                                size_t name_bytes = 0;
                                const uint32_t kShaderIndex = 0;
                                hr = amd_shader_analyzer_ext_->GetRayTracingPipelineShaderName(*pipeline_handle, pipeline_index, kShaderIndex, &name_bytes, nullptr);
                                assert(SUCCEEDED(hr));
                                if (SUCCEEDED(hr))
                                {
                                    std::wstring curr_shader_name_wide;
                                    curr_shader_name_wide.resize(name_bytes);
                                    hr = amd_shader_analyzer_ext_->GetRayTracingPipelineShaderName(*pipeline_handle, pipeline_index, kShaderIndex, &name_bytes, (wchar_t*)curr_shader_name_wide.data());
                                    if (!curr_shader_name_wide.empty())
                                    {
                                        // Convert the export name to non-wide string.
                                        std::string raygen_shader_name_as_str = RgDx12Utils::wstrToStr(curr_shader_name_wide);

                                        // Track the raygeneration shader name.
                                        pipeline_shader_names->push_back(raygen_shader_name_as_str);

                                        // Retrieve results for this pipeline.
                                        std::shared_ptr<RgDx12ShaderResultsRayTracing> curr_item = std::make_shared<RgDx12ShaderResultsRayTracing>();
                                        stats->push_back(curr_item);

                                        // Retrieve the disassembly for the pipeline.
                                        size_t diassembly_bytes = 0;
                                        hr = amd_shader_analyzer_ext_->GetRayTracingPipelineIsaDisassembly(*pipeline_handle,
                                            curr_shader_name_wide.c_str(), nullptr, &diassembly_bytes);
                                        assert(SUCCEEDED(hr));
                                        ret = (SUCCEEDED(hr));
                                        if (ret && diassembly_bytes > 0)
                                        {
                                            curr_item->disassembly = new char[diassembly_bytes] {};
                                            hr = amd_shader_analyzer_ext_->GetRayTracingPipelineIsaDisassembly(*pipeline_handle,
                                                curr_shader_name_wide.c_str(), curr_item->disassembly, &diassembly_bytes);
                                            assert(SUCCEEDED(hr));
                                            assert(curr_item->disassembly != nullptr);
                                            if (!SUCCEEDED(hr) || curr_item->disassembly == nullptr)
                                            {
                                                std::stringstream msg;
                                                msg << kStrErrorDisassemblyContentsExtractionFailure <<
                                                    kStrErrorPipelineIdentifiedByRaygenShaderName << raygen_shader_name_as_str;
                                                error_msg.append(msg.str());
                                                ret = false;
                                            }
                                        }
                                        else
                                        {
                                            std::stringstream msg;
                                            std::string shader_name_str = RgDx12Utils::wstrToStr(curr_shader_name_wide);
                                            msg << kStrErrorDisassemblySizeExtractionFailure <<
                                                kStrErrorPipelineIdentifiedByRaygenShaderName << shader_name_str;
                                            msg << kStrHintWrongShaderName1 << kStrHintWrongShaderName2Raygen;
                                            error_msg.append(msg.str());
                                            ret = false;
                                        }

                                        if (ret)
                                        {
                                            // Retrieve the statistics.
                                            AmdExtD3DShaderStatsRayTracing rt_stats_driver{};
                                            hr = amd_shader_analyzer_ext_->GetRayTracingPipelineStats(*pipeline_handle,
                                                curr_shader_name_wide.c_str(), &rt_stats_driver);
                                            assert(SUCCEEDED(hr));
                                            if (SUCCEEDED(hr))
                                            {
                                                ExtractRayTracingStats(*curr_item, rt_stats_driver);
                                            }
                                            else
                                            {
                                                std::stringstream msg;
                                                msg << kStrErrorStatisticsExtractionFailure <<
                                                    kStrErrorPipelineIdentifiedByRaygenShaderName << raygen_shader_name_as_str;
                                                error_msg.append(msg.str());
                                                ret = false;
                                            }

                                            bool is_binary_extraction_supported = (pipeline_count > 0);
                                            if (is_binary_extraction_supported)
                                            {
                                                // Extract the pipeline binary.
                                                // Retrieve the buffer size.
                                                uint32_t buffer_size = 0;
                                                hr = amd_shader_analyzer_ext_->GetRayTracingPipelineElfBinary(*pipeline_handle, pipeline_index, nullptr, &buffer_size);
                                                assert(buffer_size > 0);
                                                assert(SUCCEEDED(hr));
                                                if (buffer_size > 0 && SUCCEEDED(hr))
                                                {
                                                    // Get the contents.
                                                    std::shared_ptr<std::vector<unsigned char>> binary =
                                                        std::make_shared<std::vector<unsigned char>>();
                                                    binary->resize(buffer_size);
                                                    hr = amd_shader_analyzer_ext_->GetRayTracingPipelineElfBinary(*pipeline_handle,
                                                        0, binary->data(), &buffer_size);
                                                    assert(SUCCEEDED(hr));
                                                    assert(!binary->empty());
                                                    if (binary->empty())
                                                    {
                                                        std::stringstream msg;
                                                        msg << kStrErrorPipelineBinaryExtractionFailure <<
                                                            kStrErrorPipelineIdentifiedByRaygenShaderName << raygen_shader_name_as_str;
                                                    }

                                                    // Track the pipeline binary.
                                                    pipeline_binary.push_back(binary);
                                                }
                                                else
                                                {
                                                    std::stringstream msg;
                                                    msg << kStrErrorPipelineBinarySizeQueryFailure <<
                                                        kStrErrorPipelineIdentifiedByRaygenShaderName << raygen_shader_name_as_str;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            else
                            {
                                // The driver used Indirect compilation.
                                // Check how many shaders were created for this pipeline.
                                uint32_t shader_count = 0;
                                hr = amd_shader_analyzer_ext_->GetRayTracingPipelineShaderCount(*pipeline_handle, pipeline_index, &shader_count);
                                assert(shader_count > 0);
                                for (uint32_t shader_index = 0; shader_index < shader_count; shader_index++)
                                {
                                    // Retrieve shader name.
                                    size_t name_bytes = 0;
                                    hr = amd_shader_analyzer_ext_->GetRayTracingPipelineShaderName(*pipeline_handle, pipeline_index, shader_index, &name_bytes, nullptr);
                                    assert(SUCCEEDED(hr));
                                    if (SUCCEEDED(hr))
                                    {
                                        std::wstring curr_shader_name_wide;
                                        curr_shader_name_wide.resize(name_bytes);
                                        hr = amd_shader_analyzer_ext_->GetRayTracingPipelineShaderName(*pipeline_handle, pipeline_index, shader_index, &name_bytes, (wchar_t*)curr_shader_name_wide.data());
                                        if (!curr_shader_name_wide.empty())
                                        {
                                            // Convert the shader name to non-wide string.
                                            std::string curr_shader_name_as_str = RgDx12Utils::wstrToStr(curr_shader_name_wide);
                                            pipeline_shader_names->push_back(curr_shader_name_as_str);

                                            // Retrieve shader disassembly.
                                            std::shared_ptr<RgDx12ShaderResultsRayTracing> curr_results = std::make_shared<RgDx12ShaderResultsRayTracing>();
                                            size_t diassembly_bytes = 0;
                                            hr = amd_shader_analyzer_ext_->GetRayTracingShaderIsaDisassembly(*pipeline_handle,
                                                curr_shader_name_wide.c_str(), nullptr, &diassembly_bytes);
                                            assert(SUCCEEDED(hr));
                                            ret = (SUCCEEDED(hr));
                                            if (ret && diassembly_bytes > 0)
                                            {
                                                curr_results->disassembly = new char[diassembly_bytes] {};
                                                hr = amd_shader_analyzer_ext_->GetRayTracingShaderIsaDisassembly(*pipeline_handle,
                                                    curr_shader_name_wide.c_str(), curr_results->disassembly, &diassembly_bytes);
                                                assert(SUCCEEDED(hr));
                                                assert(curr_results->disassembly != nullptr);
                                                if (!SUCCEEDED(hr) || curr_results->disassembly == nullptr)
                                                {
                                                    std::stringstream msg;
                                                    msg << kStrErrorDisassemblyContentsExtractionFailure <<
                                                        kStrErrorDisassemblyContentsExtractionFailurePiplineNumber << pipeline_index << " " <<
                                                        kStrErrorPerShaderName << curr_shader_name_wide.c_str();
                                                    error_msg.append(msg.str());
                                                    ret = false;
                                                }
                                            }
                                            else
                                            {
                                                std::stringstream msg;
                                                msg << kStrErrorDisassemblySizeExtractionFailure <<
                                                    kStrErrorDisassemblyContentsExtractionFailurePiplineNumber << pipeline_index << " " <<
                                                    kStrErrorPerShaderName << curr_shader_name_wide.c_str();
                                                error_msg.append(msg.str());
                                                ret = false;
                                            }

                                            // Retrieve the statistics.
                                            AmdExtD3DShaderStatsRayTracing rt_stats_driver{};
                                            hr = amd_shader_analyzer_ext_->GetRayTracingPipelineStats(*pipeline_handle,
                                                curr_shader_name_wide.c_str(), &rt_stats_driver);
                                            assert(SUCCEEDED(hr));
                                            if (SUCCEEDED(hr))
                                            {
                                                ExtractRayTracingStats(*curr_results, rt_stats_driver);
                                            }
                                            else
                                            {
                                                std::stringstream msg;
                                                msg << kStrErrorStatisticsExtractionFailure <<
                                                    kStrErrorDisassemblyContentsExtractionFailurePiplineNumber << pipeline_index << " " <<
                                                    kStrErrorPerShaderName << curr_shader_name_wide.c_str();
                                                error_msg.append(msg.str());
                                                ret = false;
                                            }

                                            // Track the current results.
                                            stats->push_back(curr_results);
                                        }
                                        else
                                        {
                                            std::stringstream msg;
                                            msg << kStrErrorDxrEmptyShaderName << shader_index;
                                            error_msg.append(msg.str());
                                            ret = false;
                                        }
                                    }
                                    else
                                    {
                                        std::stringstream msg;
                                        msg << kStrErrorDxrFailedToRetrieveShaderName << shader_index <<
                                            " " << kStrErrorPipelineIdentifiedByIndex << pipeline_index;
                                        error_msg.append(msg.str());
                                        ret = false;
                                    }
                                }

                                bool is_binary_extraction_supported = (pipeline_count > 0);
                                if (is_binary_extraction_supported)
                                {
                                    // Retrieve the buffer size.
                                    uint32_t buffer_size = 0;
                                    hr = amd_shader_analyzer_ext_->GetRayTracingPipelineElfBinary(*pipeline_handle, 0, nullptr, &buffer_size);
                                    assert(buffer_size > 0);
                                    assert(SUCCEEDED(hr));
                                    if (buffer_size > 0 && SUCCEEDED(hr))
                                    {
                                        // Get the contents.
                                        std::shared_ptr<std::vector<unsigned char>> binary =
                                            std::make_shared<std::vector<unsigned char>>();
                                        binary->resize(buffer_size);
                                        hr = amd_shader_analyzer_ext_->GetRayTracingPipelineElfBinary(*pipeline_handle,
                                            0, binary->data(), &buffer_size);
                                        assert(SUCCEEDED(hr));
                                        assert(!binary->empty());
                                        if (binary->empty())
                                        {
                                            std::stringstream msg;
                                            msg << kStrErrorPipelineBinaryExtractionFailure <<
                                                kStrErrorPipelineIdentifiedByIndexPipelineBinary << pipeline_index;
                                        }

                                        // Track the pipeline binary.
                                        pipeline_binary.push_back(binary);
                                    }
                                    else
                                    {
                                        std::stringstream msg;
                                        msg << kStrErrorPipelineBinarySizeQueryFailure <<
                                            kStrErrorPipelineIdentifiedByIndex << pipeline_index;
                                    }
                                }
                            }
                        }
                        else
                        {
                            std::stringstream msg;
                            msg << kStrErrorDxrFailedToCheckPipelineCompilationType << pipeline_index;
                            error_msg.append(msg.str());
                            ret = false;
                        }

                        // Track the shader names.
                        indirect_shader_names.push_back(pipeline_shader_names);

                        // Track the results of the current indirect pipeline.
                        raytracing_shader_stats.push_back(stats);
                    }
                }
            }
            else
            {
                std::stringstream msg;
                msg << kStrErrorDxrFailedToCreateStateObject << std::endl << kStrHintDebugOutput;
                error_msg.append(msg.str());
                ret = false;
            }
        }

        return ret;
    }

    bool RgDx12Backend::Impl::CreateStateObjectShader(const D3D12_STATE_OBJECT_DESC* ray_tracing_state_object,
        const std::wstring& shader_name, RgDx12ShaderResultsRayTracing& shader_stats, std::vector<unsigned char>& pipeline_binary, std::string& error_msg) const
    {
        assert(amd_shader_analyzer_ext_ != nullptr);
        bool ret = amd_shader_analyzer_ext_ != nullptr;
        if (ret)
        {
            AmdExtD3DPipelineHandle* pipeline_handle = new AmdExtD3DPipelineHandle{};
            ID3D12StateObject* dxr_state_object = nullptr;
            HRESULT hr = amd_shader_analyzer_ext_->CreateStateObject(ray_tracing_state_object,
                IID_PPV_ARGS(&dxr_state_object), pipeline_handle);
            assert(SUCCEEDED(hr));
            ret = (SUCCEEDED(hr));
            if (ret)
            {
                // Convert the shader name to std::string for presentation purposes.
                std::string shader_name_std_str = RgDx12Utils::wstrToStr(shader_name);

                // Retrieve the disassembly for the pipeline.
                size_t diassembly_bytes = 0;
                hr = amd_shader_analyzer_ext_->GetRayTracingShaderIsaDisassembly(*pipeline_handle,
                    shader_name.c_str(), nullptr, &diassembly_bytes);
                assert(SUCCEEDED(hr));
                ret = (SUCCEEDED(hr));
                if (ret && diassembly_bytes > 0)
                {
                    shader_stats.disassembly = new char[diassembly_bytes] {};
                    hr = amd_shader_analyzer_ext_->GetRayTracingShaderIsaDisassembly(*pipeline_handle,
                        shader_name.c_str(), shader_stats.disassembly, &diassembly_bytes);
                    assert(SUCCEEDED(hr));
                    assert(shader_stats.disassembly != nullptr);
                    if (!SUCCEEDED(hr) || shader_stats.disassembly == nullptr)
                    {
                        std::stringstream msg;
                        msg << kStrErrorDisassemblyContentsExtractionFailure <<
                            kStrErrorPerShaderName << shader_name_std_str.c_str();
                        error_msg.append(msg.str());
                        ret = false;
                    }
                }
                else
                {
                    std::stringstream msg;
                    msg << kStrErrorDisassemblySizeExtractionFailure <<
                        kStrErrorPerShaderName << shader_name_std_str.c_str();
                    msg << kStrHintWrongShaderName1 << kStrHintWrongShaderName2;
                    error_msg.append(msg.str());
                    ret = false;
                }

                // Retrieve the statistics.
                if (ret)
                {
                    AmdExtD3DShaderStatsRayTracing rt_stats_driver{};
                    hr = amd_shader_analyzer_ext_->GetRayTracingPipelineShaderStats(*pipeline_handle,
                        shader_name.c_str(), &rt_stats_driver);
                    assert(SUCCEEDED(hr));
                    if (SUCCEEDED(hr))
                    {
                        ExtractRayTracingStats(shader_stats, rt_stats_driver);
                    }
                    else
                    {
                        std::stringstream msg;
                        msg << kStrErrorStatisticsExtractionFailure <<
                            kStrErrorPerShaderName << shader_name_std_str.c_str();
                        error_msg.append(msg.str());
                        ret = false;
                    }
                }

#ifdef _EXTRACT_BINARIES
                // Retrieve the pipeline binaries.
                uint32_t pipeline_count = 0;
                hr = amd_shader_analyzer_ext_->GetRayTracingPipelineElfCount(*pipeline_handle, &pipeline_count);
                assert(SUCCEEDED(hr));
                assert(pipeline_count == 1);
                ret = (SUCCEEDED(hr));
                if (ret)
                {
                    if (pipeline_count > 1)
                    {
                        // This is unexpected. Warn the user that only the first pipeline binary is going to be extracted.
                        std::stringstream msg;
                        msg << kStrWarningMoreThanSinglePipelineBinaryForSinglePipeline1 <<
                            kStrErrorPipelineIdentifiedByRaygenShaderName << shader_name_std_str.c_str() <<
                            kStrWarningMoreThanSinglePipelineBinaryForSinglePipeline2;
                        error_msg.append(msg.str());
                    }

                    // Retrieve the buffer size.
                    uint32_t buffer_size = 0;
                    hr = amd_shader_analyzer_ext_->GetRayTracingPipelineElfBinary(*pipeline_handle, 0, nullptr, &buffer_size);
                    assert(buffer_size > 0);
                    assert(SUCCEEDED(hr));
                    if (buffer_size > 0 && SUCCEEDED(hr))
                    {
                        // Get the contents.
                        pipeline_binary.clear();
                        pipeline_binary.resize(buffer_size);
                        hr = amd_shader_analyzer_ext_->GetRayTracingPipelineElfBinary(*pipeline_handle,
                            0, pipeline_binary.data(), &buffer_size);
                        assert(SUCCEEDED(hr));
                        assert(!pipeline_binary.empty());
                        if (pipeline_binary.empty())
                        {
                            std::stringstream msg;
                            msg << kStrErrorPipelineBinaryExtractionFailure <<
                                kStrErrorPipelineIdentifiedByIndexPipelineBinary << shader_name_std_str.c_str();
                        }
                    }
                    else
                    {
                        std::stringstream msg;
                        msg << kStrErrorPipelineBinarySizeQueryFailure <<
                            kStrErrorPipelineIdentifiedByRaygenShaderName << shader_name_std_str.c_str();
                    }
                }
                else
                {
                    std::stringstream msg;
                    msg << kStrErrorPipelineBinaryCountExtractionFailure <<
                        kStrErrorPipelineIdentifiedByRaygenShaderName << shader_name_std_str.c_str();
                    error_msg.append(msg.str());
                    ret = false;
                }
#endif // _EXTRACT_BINARIES
            }
            else
            {
                std::stringstream msg;
                msg << kStrErrorDxrFailedToCreateStateObject << std::endl << kStrHintDebugOutput;
                error_msg.append(msg.str());
                ret = false;
            }
        }

        return ret;
    }
#endif
    bool RgDx12Backend::CompileComputePipeline(const D3D12_COMPUTE_PIPELINE_STATE_DESC* compute_pso,
        RgDx12ShaderResults& compute_shader_stats,
        RgDx12ThreadGroupSize& thread_group_size,
        std::vector<char>& pipeline_binary,
        std::string& error_msg) const
    {
        bool ret = false;
        assert(impl_ != nullptr);
        if (impl_ != nullptr)
        {
            ret = impl_->CompileComputePipeline(compute_pso, compute_shader_stats, thread_group_size, pipeline_binary, error_msg);
        }
        return ret;
    }

#ifdef RGA_DXR_ENABLED
    bool RgDx12Backend::CreateStateObject(const D3D12_STATE_OBJECT_DESC* ray_tracing_state_object,
        std::vector<std::shared_ptr<std::vector<std::shared_ptr<RgDx12ShaderResultsRayTracing>>>>& raytracing_shader_stats,
        std::vector<std::shared_ptr<std::vector<unsigned char>>>& pipeline_binary, std::vector<bool>& is_unified_mode,
        std::vector< std::shared_ptr<std::vector<std::string>>>& indirect_shader_names,
        std::string& error_msg) const
    {
        bool ret = false;
        assert(impl_ != nullptr);
        if (impl_ != nullptr)
        {
            ret = impl_->CreateStateObject(ray_tracing_state_object, raytracing_shader_stats, pipeline_binary, is_unified_mode, indirect_shader_names, error_msg);
        }
        return ret;
    }

    bool RgDx12Backend::CreateStateObjectShader(const D3D12_STATE_OBJECT_DESC* ray_tracing_state_object, const std::wstring& raygen_shader_name,
        RgDx12ShaderResultsRayTracing& raytracing_shader_stats, std::vector<unsigned char>& pipeline_binary, std::string& error_msg) const
    {
        bool ret = false;
        assert(impl_ != nullptr);
        if (impl_ != nullptr)
        {
            ret = impl_->CreateStateObjectShader(ray_tracing_state_object, raygen_shader_name, raytracing_shader_stats, pipeline_binary, error_msg);
        }
        return ret;
    }

#endif

    rga::RgDx12Backend::RgDx12Backend() : impl_(new RgDx12Backend::Impl) {}

    rga::RgDx12Backend::~RgDx12Backend()
    {
        if (impl_ != nullptr)
        {
            delete impl_;
            impl_ = nullptr;
        }
    }

    rga::RgDx12ShaderResults::~RgDx12ShaderResults()
    {
        if (disassembly != nullptr)
        {
            delete disassembly;
            disassembly = nullptr;
        }
    }
}
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
    static bool ExtractXmlContent(IAmdExtD3DShaderAnalyzer3* amd_shader_analyzer_ext,
        AmdExtD3DPipelineHandle* pipeline_handle, char*& xml_buffer, std::string&)
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

    static bool ExtractAmdilDisassemblyGraphics(IAmdExtD3DShaderAnalyzer3* amd_shader_analyzer_ext,
                                                AmdExtD3DPipelineHandle*   pipeline_handle,
                                                char*&                     xml_buffer,
                                                RgDx12PipelineResults&     results,
                                                std::string&               )
    {
        bool                         ret        = false;
        size_t                       amdil_size = 0;
        AmdExtD3DPipelineDisassembly disassembly;
        HRESULT                      hr = amd_shader_analyzer_ext->GetShaderAmdIlDisassembly(*pipeline_handle, nullptr, &amdil_size, &disassembly);
        assert(hr == S_OK);
        assert(amdil_size > 0);
        ret = (hr == S_OK && amdil_size > 0);
        if (ret)
        {
            xml_buffer = new char[amdil_size]{};
            hr         = amd_shader_analyzer_ext->GetShaderAmdIlDisassembly(*pipeline_handle, xml_buffer, &amdil_size, &disassembly);
            assert(hr == S_OK);
            ret = (hr == S_OK);
            if (ret)
            {
                if (disassembly.pVertexDisassembly != nullptr)
                {
                    std::string amdil_vert           = disassembly.pVertexDisassembly;
                    results.vertex.disassembly_amdil = new char[amdil_vert.size()]{0};
                    memcpy(results.vertex.disassembly_amdil, disassembly.pVertexDisassembly, amdil_vert.size()-1);
                }
                if (disassembly.pHullDisassembly != nullptr)
                {
                    std::string amdil_hull         = disassembly.pHullDisassembly;
                    results.hull.disassembly_amdil = new char[amdil_hull.size()]{0};
                    std::copy(amdil_hull.begin(), amdil_hull.end(), results.hull.disassembly_amdil);
                    memcpy(results.hull.disassembly_amdil, disassembly.pHullDisassembly, amdil_hull.size() - 1);
                }
                if (disassembly.pDomainDisassembly != nullptr)
                {
                    std::string amdil_domain         = disassembly.pDomainDisassembly;
                    results.domain.disassembly_amdil = new char[amdil_domain.size()]{0};
                    memcpy(results.domain.disassembly_amdil, disassembly.pDomainDisassembly, amdil_domain.size() - 1);
                }
                if (disassembly.pGeometryDisassembly != nullptr)
                {
                    std::string amdil_geom             = disassembly.pGeometryDisassembly;
                    results.geometry.disassembly_amdil = new char[amdil_geom.size()]{0};
                    memcpy(results.geometry.disassembly_amdil, disassembly.pGeometryDisassembly, amdil_geom.size() - 1);
                }
                if (disassembly.pPixelDisassembly != nullptr)
                {
                    std::string amdil_pixel         = disassembly.pPixelDisassembly;
                    results.pixel.disassembly_amdil = new char[amdil_pixel.size()]{0};
                    memcpy(results.pixel.disassembly_amdil, disassembly.pPixelDisassembly, amdil_pixel.size() - 1);
                }
            }
        }

        return ret;
    }

    static bool ExtractAmdilDisassemblyCompute(IAmdExtD3DShaderAnalyzer3* amd_shader_analyzer_ext,
                                               AmdExtD3DPipelineHandle*   pipeline_handle,
                                               char*&                     xml_buffer,
                                               RgDx12ShaderResults&       results,
                                               std::string&               )
    {
        bool                         ret        = false;
        size_t                       amdil_size = 0;
        AmdExtD3DPipelineDisassembly disassembly;
        HRESULT                      hr = amd_shader_analyzer_ext->GetShaderAmdIlDisassembly(*pipeline_handle, nullptr, &amdil_size, &disassembly);
        assert(hr == S_OK);
        assert(amdil_size > 0);
        ret = (hr == S_OK && amdil_size > 0);
        if (ret)
        {
            xml_buffer = new char[amdil_size]{};
            hr         = amd_shader_analyzer_ext->GetShaderAmdIlDisassembly(*pipeline_handle, xml_buffer, &amdil_size, &disassembly);
            assert(hr == S_OK);
            ret = (hr == S_OK);
            if (ret)
            {
                if (disassembly.pComputeDisassembly != nullptr)
                {
                    std::string amdil_comp    = disassembly.pComputeDisassembly;
                    results.disassembly_amdil = new char[amdil_comp.size()]{};
                    memcpy(results.disassembly_amdil, disassembly.pComputeDisassembly, amdil_comp.size() - 1);
                }
            }
        }

        return ret;
    }

    // Retrieve the disassembly for the given pipeline state object.
    // The given pipeline state object is expected to be a valid object, which was received from a
    // call to CreateGraphicsPipeline or CreateComputePipeline using the same ID3D12Device which
    // was used in the call to rgDx12Backend::Init().
    // Returns true on success, false otherwise.
    static bool RetrieveDisassemblyGraphics(const RgDx12Config&        config,
                                            IAmdExtD3DShaderAnalyzer3* amd_shader_analyzer_ext,
                                            AmdExtD3DPipelineHandle*   d3d12_pipeline_state,
                                            RgDx12PipelineResults&     results,
                                            std::string&               error_msg)
    {
        bool ret = false;

        // Parse the XML string.
        tinyxml2::XMLDocument doc;
        char*                 xml_buffer = NULL;

        // AMDIL part.
        bool is_amdil_required = !config.vert.amdil.empty() || !config.hull.amdil.empty() || !config.domain.amdil.empty() || !config.geom.amdil.empty() ||
                                 !config.pixel.amdil.empty();
        if (is_amdil_required)
        {
            ret = ExtractAmdilDisassemblyGraphics(amd_shader_analyzer_ext, d3d12_pipeline_state, xml_buffer, results, error_msg);
            assert(ret);
        }

        // ISA - always required.
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

    static bool RetrieveDisassemblyCompute(const RgDx12Config         config,
                                           IAmdExtD3DShaderAnalyzer3* amd_shader_analyzer_ext,
                                           AmdExtD3DPipelineHandle*   d3d12_pipeline_state,
                                           RgDx12ShaderResults&       results,
                                           std::string&               error_msg)
    {
        bool ret = false;

        // Parse the XML string.
        tinyxml2::XMLDocument doc;
        char*                 xml_buffer = NULL;

        if (!config.comp.amdil.empty())
        {
            ret = ExtractAmdilDisassemblyCompute(amd_shader_analyzer_ext, d3d12_pipeline_state, xml_buffer, results, error_msg);
            assert(ret);
        }

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

    static bool ExtractPipelineBinary(IAmdExtD3DShaderAnalyzer3* amd_shader_analyzer_ext,
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
        bool InitImpl(ID3D12Device* d3d12_device, bool is_offline_session);

        bool GetSupportedTargets(std::vector<std::string>& supported_targets,
            std::map<std::string, unsigned>& target_to_id);

        bool CompileGraphicsPipeline(const RgDx12Config&                       config,
                                     const D3D12_GRAPHICS_PIPELINE_STATE_DESC* graphics_pso,
                                     RgDx12PipelineResults&                    results,
                                     std::vector<char>&                        pipeline_binary,
                                     std::string&                              error_msg) const;

        bool CompileComputePipeline(const RgDx12Config                       config,
                                    const D3D12_COMPUTE_PIPELINE_STATE_DESC* compute_pso,
                                    RgDx12ShaderResults&                     shader_results,
                                    RgDx12ThreadGroupSize&                   thread_group_size,
                                    std::vector<char>&                       binary,
                                    std::string&                             error_msg) const;

        // Creates the state object and extracts the compiled code object binaries for a single pipeline designated by the raygeneration shader name.
        bool CreateStateObject(const D3D12_STATE_OBJECT_DESC*                                                             ray_tracing_state_object,
                               std::vector<std::shared_ptr<std::vector<unsigned char>>>&                                  pipeline_binary,
                               std::string&                                                                               error_msg) const;

    private:
        HMODULE LoadUMDLibrary(bool is_offline_session);
        HMODULE amd_d3d_dll_handle_ = NULL;
        IAmdExtD3DFactory* amd_ext_object_ = NULL;
        IAmdExtD3DShaderAnalyzer3* amd_shader_analyzer_ext_ = NULL;
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

    bool RgDx12Backend::Impl::InitImpl(ID3D12Device* d3d12_device, bool is_offline_session)
    {
        // Load the user-mode driver.
        amd_d3d_dll_handle_ = LoadUMDLibrary(is_offline_session);
        assert(amd_d3d_dll_handle_ != NULL);
        bool ret = (amd_d3d_dll_handle_ != NULL);
        if (ret)
        {
            PFNAmdExtD3DCreateInterface amd_ext_d3d_create_func = (PFNAmdExtD3DCreateInterface)GetProcAddress(amd_d3d_dll_handle_, "AmdExtD3DCreateInterface");

            assert(amd_ext_d3d_create_func != NULL);
            if (amd_ext_d3d_create_func != NULL)
            {
                HRESULT hr = amd_ext_d3d_create_func(d3d12_device, __uuidof(IAmdExtD3DFactory), reinterpret_cast<void**>(&amd_ext_object_));
                assert(hr == S_OK);
                if (hr == S_OK)
                {
                    hr = amd_ext_object_->CreateInterface(
                        d3d12_device, __uuidof(IAmdExtD3DShaderAnalyzer3), reinterpret_cast<void**>(&amd_shader_analyzer_ext_));
                    assert(hr == S_OK);
                    assert(amd_shader_analyzer_ext_ != NULL);
                    ret = (hr == S_OK && amd_shader_analyzer_ext_ != NULL);
                }
            }
        }

        return ret;
    }

    bool RgDx12Backend::Init(ID3D12Device* d3d12_device, bool is_offline_session)
    {
        bool ret = false;
        assert(impl_ != nullptr);
        if (impl_ != nullptr)
        {
            ret = impl_->InitImpl(d3d12_device, is_offline_session);
        }
        return ret;
    }

    HMODULE RgDx12Backend::Impl::LoadUMDLibrary(bool is_offline_session)
    {
        std::wstring umd_module_name = L"amdxc64.dll";
        HMODULE umd_dll  = nullptr;
        if (!is_offline_session)
        {
            umd_dll = LoadLibrary(umd_module_name.c_str());
        }
        else
        {
            // The driver is already injected to the process's memory.
            umd_dll = GetModuleHandle(L"amdxc64");
        }
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

    bool RgDx12Backend::Impl::CompileGraphicsPipeline(const RgDx12Config&                       config,
                                                      const D3D12_GRAPHICS_PIPELINE_STATE_DESC* graphics_pso,
                                                      RgDx12PipelineResults&                    results,
                                                      std::vector<char>&                        pipeline_binary,
                                                      std::string&                              error_msg) const
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
                ret = RetrieveDisassemblyGraphics(config, amd_shader_analyzer_ext_, pipeline_handle, results, error_msg);
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

    bool RgDx12Backend::CompileGraphicsPipeline(const RgDx12Config&                       config,
                                                const D3D12_GRAPHICS_PIPELINE_STATE_DESC* graphics_pso,
                                                RgDx12PipelineResults&                    results,
                                                std::vector<char>&                        pipeline_binary,
                                                std::string&                              error_msg) const
    {
        bool ret = false;
        assert(impl_ != nullptr);
        if (impl_ != nullptr)
        {
            ret = impl_->CompileGraphicsPipeline(config, graphics_pso, results, pipeline_binary, error_msg);
        }
        return ret;
    }

    bool RgDx12Backend::Impl::CompileComputePipeline(const RgDx12Config                       config,
                                                     const D3D12_COMPUTE_PIPELINE_STATE_DESC* compute_pso,
                                                     RgDx12ShaderResults&                     shader_results,
                                                     RgDx12ThreadGroupSize&                   thread_group_size,
                                                     std::vector<char>&                       binary,
                                                     std::string&                             error_msg) const
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
                    ret = RetrieveDisassemblyCompute(config, amd_shader_analyzer_ext_, pipeline_handle, shader_results, error_msg);
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

    bool RgDx12Backend::Impl::CreateStateObject(
        const D3D12_STATE_OBJECT_DESC*                                                             ray_tracing_state_object,
        std::vector<std::shared_ptr<std::vector<unsigned char>>>&                                  pipeline_binaries,
        std::string&                                                                               error_msg) const
    {
        assert(amd_shader_analyzer_ext_ != nullptr);
        bool ret = amd_shader_analyzer_ext_ != nullptr;
        if (ret)
        {
            AmdExtD3DPipelineHandle* pipeline_handle  = new AmdExtD3DPipelineHandle{};
            ID3D12StateObject*       dxr_state_object = nullptr;
            HRESULT hr = amd_shader_analyzer_ext_->CreateStateObject(ray_tracing_state_object, IID_PPV_ARGS(&dxr_state_object), pipeline_handle);
            assert(SUCCEEDED(hr));
            ret = (SUCCEEDED(hr));
            if (ret)
            {
                // Retrieve the number of pipelines.
                uint32_t pipeline_count = 0;
                hr                      = amd_shader_analyzer_ext_->GetRayTracingPipelineElfCount(*pipeline_handle, &pipeline_count);
                assert(SUCCEEDED(hr));
                ret = (SUCCEEDED(hr));
                if (ret)
                {
                    for (uint32_t pipeline_index = 0; pipeline_index < pipeline_count; ++pipeline_index)
                    {
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
                                std::shared_ptr<std::vector<unsigned char>> binary = std::make_shared<std::vector<unsigned char>>();
                                binary->resize(buffer_size);
                                hr = amd_shader_analyzer_ext_->GetRayTracingPipelineElfBinary(*pipeline_handle, pipeline_index, binary->data(), &buffer_size);
                                assert(SUCCEEDED(hr));
                                assert(!binary->empty());
                                if (binary->empty())
                                {
                                    std::stringstream msg;
                                    msg << kStrErrorPipelineBinaryExtractionFailure << kStrErrorPipelineIdentifiedByIndexPipelineBinary << pipeline_index << "\n";
                                    error_msg.append(msg.str());
                                }

                                // Track the pipeline binary.
                                pipeline_binaries.push_back(binary);
                            }
                            else
                            {
                                std::stringstream msg;
                                msg << kStrErrorPipelineBinarySizeQueryFailure << kStrErrorPipelineIdentifiedByIndexPipelineBinary << pipeline_index << "\n";
                                error_msg.append(msg.str());
                            }
                        }
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

#endif
    bool RgDx12Backend::CompileComputePipeline(const RgDx12Config                       config,
                                               const D3D12_COMPUTE_PIPELINE_STATE_DESC* compute_pso,
                                               RgDx12ShaderResults&                     compute_shader_stats,
                                               RgDx12ThreadGroupSize&                   thread_group_size,
                                               std::vector<char>&                       pipeline_binary,
                                               std::string&                             error_msg) const
    {
        bool ret = false;
        assert(impl_ != nullptr);
        if (impl_ != nullptr)
        {
            ret = impl_->CompileComputePipeline(config, compute_pso, compute_shader_stats, thread_group_size, pipeline_binary, error_msg);
        }
        return ret;
    }

#ifdef RGA_DXR_ENABLED

    bool RgDx12Backend::CreateStateObject(const D3D12_STATE_OBJECT_DESC*                            ray_tracing_state_object,
                                          std::vector<std::shared_ptr<std::vector<unsigned char>>>& pipeline_binary,
                                          std::string&                                              error_msg) const
    {
        bool ret = false;
        assert(impl_ != nullptr);
        if (impl_ != nullptr)
        {
            ret = impl_->CreateStateObject(ray_tracing_state_object, pipeline_binary, error_msg);
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
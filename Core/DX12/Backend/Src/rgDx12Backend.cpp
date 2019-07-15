#include "rgDx12Backend.h"

// C++.
#include <sstream>
#include <fstream>
#include <cassert>
#include <algorithm>

// TinyXML2.
#include <tinyxml2.h>

// DX.
#include <d3dcommon.h>
#include <AmdExtD3D.h>
#include <AmdExtD3DShaderAnalyzerApi.h>

namespace rga
{
    // *** CONSTANTS - START ***

    // XML.
    static const char* STR_XML_NODE_COMMENTS = "comments";
    static const char* STR_XML_NODE_STAGE = "stage";
    static const char* STR_XML_NODE_COMMENT = "comment";
    static const char* STR_XML_ATTRIBUTE_VS = "VS";
    static const char* STR_XML_ATTRIBUTE_HS = "HS";
    static const char* STR_XML_ATTRIBUTE_DS = "DS";
    static const char* STR_XML_ATTRIBUTE_GS = "GS";
    static const char* STR_XML_ATTRIBUTE_PS = "PS";
    static const char* STR_XML_ATTRIBUTE_CS = "CS";

    // Messages.
    static const char* STR_ERROR_PIPELINE_CREATE_FAILURE = "Error: failed to create D3D12 graphics pipeline with error code: ";
    static const char* STR_ERROR_PIPELINE_STATE_CREATE_FAILURE = "Error: failed to create compute pipeline state.";

    // *** CONSTANTS - END ***

    // Extracts the XML file for the given pipeline state, and allocates the contents in the given buffer.
    // In case of any error messages, they will be set into errorMsg.
    // Returns true on success, false otherwise.
    static bool ExtractXmlContent(IAmdExtD3DShaderAnalyzer* pAmdShaderAnalyzerExt,
        AmdExtD3DPipelineHandle* pPipelineHandle, char*& pXmlBuffer, std::string& errorMsg)
    {
        bool ret = false;
        size_t isaSize = 0;
        AmdExtD3DPipelineDisassembly disassembly;
        HRESULT hr = pAmdShaderAnalyzerExt->GetShaderIsaCode(*pPipelineHandle, nullptr, &isaSize, &disassembly);
        assert(hr == S_OK);
        assert(isaSize > 0);
        ret = (hr == S_OK && isaSize > 0);
        if (ret)
        {
            pXmlBuffer = new char[isaSize] {};
            hr = pAmdShaderAnalyzerExt->GetShaderIsaCode(*pPipelineHandle, pXmlBuffer, &isaSize, &disassembly);
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
    static bool RetrieveDisassemblyGraphics(IAmdExtD3DShaderAnalyzer* pAmdShaderAnalyzerExt,
        AmdExtD3DPipelineHandle* pD3d12PipelineState,
        rgDx12PipelineResults& results, std::string& errorMsg)
    {
        bool ret = false;

        // Parse the XML string.
        tinyxml2::XMLDocument doc;
        char* pXmlBuffer = NULL;
        ret = ExtractXmlContent(pAmdShaderAnalyzerExt, pD3d12PipelineState, pXmlBuffer, errorMsg);
        assert(ret);
        assert(pXmlBuffer != NULL);
        if (ret && pXmlBuffer != NULL)
        {
            tinyxml2::XMLError rc = doc.Parse(pXmlBuffer);
            assert(rc == tinyxml2::XML_SUCCESS);
            if (rc == tinyxml2::XML_SUCCESS)
            {
                // Retrieve the XML file and the comments node.
                tinyxml2::XMLElement*pCommentsNode = doc.FirstChildElement(STR_XML_NODE_COMMENTS);
                assert(pCommentsNode != nullptr);
                if (pCommentsNode != nullptr)
                {
                    // Extract the disassembly for all shaders in the pipeline.
                    for (tinyxml2::XMLElement* pShaderNode = pCommentsNode->FirstChildElement();
                        pShaderNode != NULL; pShaderNode = pShaderNode->NextSiblingElement())
                    {
                        assert(pShaderNode != nullptr);
                        if (pShaderNode != nullptr)
                        {
                            std::string shaderType = pShaderNode->Attribute(STR_XML_NODE_STAGE);
                            assert(!shaderType.empty());
                            if (!shaderType.empty())
                            {
                                tinyxml2::XMLElement* pDisassemblyNode = pShaderNode->FirstChildElement(STR_XML_NODE_COMMENT);
                                assert(pDisassemblyNode != nullptr);
                                if (pDisassemblyNode != nullptr)
                                {
                                    const char* pDisassembly = pDisassemblyNode->GetText();
                                    assert(pDisassembly != NULL);
                                    if (pDisassembly != NULL)
                                    {
                                        size_t sz = strlen(pDisassembly);
                                        assert(sz > 0);
                                        if (sz > 0)
                                        {
                                            // Assign the disassembly to the correct shader.
                                            if (shaderType.compare(STR_XML_ATTRIBUTE_VS) == 0)
                                            {
                                                results.m_vertex.pDisassembly = new char[sz]();
                                                memcpy(results.m_vertex.pDisassembly, pDisassembly, sz);
                                            }
                                            else if (shaderType.compare(STR_XML_ATTRIBUTE_HS) == 0)
                                            {
                                                results.m_hull.pDisassembly = new char[sz]();
                                                memcpy(results.m_hull.pDisassembly, pDisassembly, sz);
                                            }
                                            else if (shaderType.compare(STR_XML_ATTRIBUTE_DS) == 0)
                                            {
                                                results.m_domain.pDisassembly = new char[sz]();
                                                memcpy(results.m_domain.pDisassembly, pDisassembly, sz);
                                            }
                                            else if (shaderType.compare(STR_XML_ATTRIBUTE_GS) == 0)
                                            {
                                                results.m_geometry.pDisassembly = new char[sz]();
                                                memcpy(results.m_geometry.pDisassembly, pDisassembly, sz);
                                            }
                                            else if (shaderType.compare(STR_XML_ATTRIBUTE_PS) == 0)
                                            {
                                                results.m_pixel.pDisassembly = new char[sz]();
                                                memcpy(results.m_pixel.pDisassembly, pDisassembly, sz);
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
        if (pXmlBuffer != NULL)
        {
            delete[] pXmlBuffer;
            pXmlBuffer = NULL;
        }

        ret = (results.m_vertex.pDisassembly != nullptr ||
            results.m_hull.pDisassembly != nullptr ||
            results.m_domain.pDisassembly != nullptr ||
            results.m_geometry.pDisassembly != nullptr ||
            results.m_pixel.pDisassembly != nullptr);

        return ret;
    }

    static bool RetrieveDisassemblyCompute(IAmdExtD3DShaderAnalyzer* pAmdShaderAnalyzerExt,
        AmdExtD3DPipelineHandle* pD3d12PipelineState,
        rgDx12ShaderResults& results, std::string& errorMsg)
    {
        bool ret = false;

        // Parse the XML string.
        tinyxml2::XMLDocument doc;
        char* pXmlBuffer = NULL;
        ret = ExtractXmlContent(pAmdShaderAnalyzerExt, pD3d12PipelineState, pXmlBuffer, errorMsg);
        assert(ret);
        assert(pXmlBuffer != NULL);
        if (ret && pXmlBuffer != NULL)
        {
            tinyxml2::XMLError rc = doc.Parse(pXmlBuffer);
            assert(rc == tinyxml2::XML_SUCCESS);
            if (rc == tinyxml2::XML_SUCCESS)
            {
                // Retrieve the XML file and the comments node.
                tinyxml2::XMLElement*pCommentsNode = doc.FirstChildElement(STR_XML_NODE_COMMENTS);
                assert(pCommentsNode != nullptr);
                if (pCommentsNode != nullptr)
                {
                    // Retrieve the compute shader disassembly.
                    tinyxml2::XMLElement* pShaderNode = pCommentsNode->FirstChildElement();
                    assert(pShaderNode != nullptr);
                    if (pShaderNode != nullptr)
                    {
                        std::string shaderType = pShaderNode->Attribute(STR_XML_NODE_STAGE);
                        assert(shaderType.compare(STR_XML_ATTRIBUTE_CS) == 0);
                        if (shaderType.compare(STR_XML_ATTRIBUTE_CS) == 0)
                        {
                            tinyxml2::XMLElement* pDisassemblyNode =
                                pShaderNode->FirstChildElement(STR_XML_NODE_COMMENT);
                            assert(pDisassemblyNode != nullptr);
                            if (pDisassemblyNode != nullptr)
                            {
                                const char* pDisassembly = pDisassemblyNode->GetText();
                                assert(pDisassemblyNode != NULL);
                                if (pDisassemblyNode != NULL)
                                {
                                    size_t sz = strlen(pDisassembly);
                                    assert(sz > 0);
                                    if (sz > 0)
                                    {
                                        results.pDisassembly = new char[sz]();
                                        memcpy(results.pDisassembly, pDisassembly, sz);
                                        results.pDisassembly[sz - 1] = '\0';
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        // Free the XML buffer memory.
        if (pXmlBuffer != NULL)
        {
            delete[] pXmlBuffer;
            pXmlBuffer = NULL;
        }

        return ret;
    }

    class rgDx12Backend::Impl
    {
    public:
        bool InitImpl(ID3D12Device* pD3D12Device);

        bool GetSupportedTargets(std::vector<std::string>& supportedTargets,
            std::map<std::string, unsigned>& targetToId);

        bool CompileGraphicsPipeline(const D3D12_GRAPHICS_PIPELINE_STATE_DESC* pGraphicsPso,
            rgDx12PipelineResults& results,
            std::string& errorMsg) const;

        bool CompileComputePipeline(const D3D12_COMPUTE_PIPELINE_STATE_DESC* pComputePso,
            rgDx12ShaderResults& shaderResults,
            rgDx12ThreadGroupSize& threadGroupSize,
            std::string& errorMsg) const;

    private:
        HMODULE LoadUMDLibrary();
        HMODULE m_hAmdD3dDll = NULL;
        IAmdExtD3DFactory* m_pAmdExtObject = NULL;
        IAmdExtD3DShaderAnalyzer* m_pAmdShaderAnalyzerExt = NULL;
    };

    static void SetShaderResults(const AmdExtD3DShaderStats& shaderStats, rgDx12ShaderResults& results)
    {
        // VGPRs.
        results.vgprUsed = shaderStats.usageStats.numUsedVgprs;
        results.vgprAvailable = shaderStats.numAvailableVgprs;
        results.vgprPhysical = shaderStats.numPhysicalVgprs;

        // SGPRs.
        results.sgprUsed = shaderStats.usageStats.numUsedSgprs;
        results.sgprAvailable = shaderStats.numAvailableSgprs;
        results.sgprPhysical = shaderStats.numPhysicalSgprs;

        // LDS.
        results.ldsUsedBytes = shaderStats.usageStats.ldsUsageSizeInBytes;
        results.ldsAvailableBytes = shaderStats.usageStats.ldsSizePerThreadGroup;

        // Scratch memory.
        results.scratchUsedBytes = shaderStats.usageStats.scratchMemUsageInBytes;

        // Stage mask.
        results.shaderMask = shaderStats.shaderStageMask;
    }

    static void SetShaderResultsCompute(const AmdExtD3DComputeShaderStats& shaderStats,
        rgDx12ShaderResults& results, rgDx12ThreadGroupSize& threadGroupSize)
    {
        // Set the common results.
        SetShaderResults(shaderStats, results);

        // Set the compute-specific results.
        threadGroupSize.x = shaderStats.numThreadsPerGroupX;
        threadGroupSize.y = shaderStats.numThreadsPerGroupY;
        threadGroupSize.z = shaderStats.numThreadsPerGroupZ;
    }

    static void SetPipelineResults(const AmdExtD3DGraphicsShaderStats& stats, rgDx12PipelineResults& results)
    {
        SetShaderResults(stats.vertexShaderStats, results.m_vertex);
        SetShaderResults(stats.hullShaderStats, results.m_hull);
        SetShaderResults(stats.domainShaderStats, results.m_domain);
        SetShaderResults(stats.geometryShaderStats, results.m_geometry);
        SetShaderResults(stats.pixelShaderStats, results.m_pixel);
    }

    bool rgDx12Backend::Impl::InitImpl(ID3D12Device* pD3D12Device)
    {
        // Load the user-mode driver.
        m_hAmdD3dDll = LoadUMDLibrary();
        assert(m_hAmdD3dDll != NULL);
        bool ret = (m_hAmdD3dDll != NULL);
        if (ret)
        {
            PFNAmdExtD3DCreateInterface pAmdExtD3dCreateFunc =
                (PFNAmdExtD3DCreateInterface)GetProcAddress(m_hAmdD3dDll,
                    "AmdExtD3DCreateInterface");

            if (pAmdExtD3dCreateFunc != NULL)
            {
                HRESULT hr = pAmdExtD3dCreateFunc(pD3D12Device,
                    __uuidof(IAmdExtD3DFactory), reinterpret_cast<void **>(&m_pAmdExtObject));

                if (hr == S_OK)
                {
                    hr = m_pAmdExtObject->CreateInterface(pD3D12Device,
                        __uuidof(IAmdExtD3DShaderAnalyzer),
                        reinterpret_cast<void **>(&m_pAmdShaderAnalyzerExt));
                    assert(hr == S_OK);
                    assert(m_pAmdShaderAnalyzerExt != NULL);
                    ret = (hr == S_OK && m_pAmdShaderAnalyzerExt != NULL);
                }
            }
        }

        return ret;

    }

    bool rgDx12Backend::Init(ID3D12Device* pD3D12Device)
    {
        bool ret = false;
        assert(m_pImpl != nullptr);
        if (m_pImpl != nullptr)
        {
            ret = m_pImpl->InitImpl(pD3D12Device);
        }
        return ret;
    }

    HMODULE rgDx12Backend::Impl::LoadUMDLibrary()
{
        std::wstring umdModuleName = L"amdxc64.dll";
        HMODULE umdDll = LoadLibrary(umdModuleName.c_str());
        return umdDll;
    }

    bool rgDx12Backend::Impl::GetSupportedTargets(std::vector<std::string>& supportedTargets,
        std::map<std::string, unsigned>& targetToId)
    {
        assert(m_pAmdShaderAnalyzerExt != nullptr);
        bool ret = m_pAmdShaderAnalyzerExt != nullptr;
        if (ret)
        {
            // First call to get the available virtual GPUs
            // retrieves the number of supported virtual targets.
            AmdExtD3DGpuIdList gpuIdList{};
            HRESULT hr = m_pAmdShaderAnalyzerExt->GetAvailableVirtualGpuIds(&gpuIdList);
            assert(gpuIdList.numGpuIdEntries > 0);
            ret = (gpuIdList.numGpuIdEntries > 0);
            if (ret)
            {
                // Second call retrieves the actual virtual GPUs.
                gpuIdList.pGpuIdEntries = new AmdExtD3DGpuIdEntry[gpuIdList.numGpuIdEntries]();
                hr = m_pAmdShaderAnalyzerExt->GetAvailableVirtualGpuIds(&gpuIdList);
                assert(hr == S_OK);
                ret = SUCCEEDED(hr);
                if (ret)
                {
                    // Look for the user's target of choice in the virtual GPU list.
                    for (unsigned i = 0; i < gpuIdList.numGpuIdEntries; i++)
                    {
                        // Add the target name to the list of targets.
                        supportedTargets.push_back(gpuIdList.pGpuIdEntries[i].pGpuIdName);

                        // Track the mapping between the name to the ID.
                        targetToId[gpuIdList.pGpuIdEntries[i].pGpuIdName] = gpuIdList.pGpuIdEntries[i].gpuId;
                    }
                }

                // Free the memory.
                delete[] gpuIdList.pGpuIdEntries;
            }
        }
        return ret;
    }

    bool rgDx12Backend::GetSupportedTargets(std::vector<std::string>& supportedTargets,
        std::map<std::string, unsigned>& targetToId) const
    {
        bool ret = false;
        assert(m_pImpl != nullptr);
        if (m_pImpl != nullptr)
        {
            ret = m_pImpl->GetSupportedTargets(supportedTargets, targetToId);
        }
        return ret;
    }

    bool rgDx12Backend::Impl::CompileGraphicsPipeline(const D3D12_GRAPHICS_PIPELINE_STATE_DESC* pGraphicsPso,
        rgDx12PipelineResults& results,
        std::string& errorMsg) const
    {
        assert(m_pAmdShaderAnalyzerExt != nullptr);
        bool ret = m_pAmdShaderAnalyzerExt != nullptr;
        if (ret)
        {
            AmdExtD3DPipelineHandle* pPipelineHandle = NULL;
            ID3D12PipelineState* pPipeline = NULL;
            AmdExtD3DGraphicsShaderStats stats;
            HRESULT hr = m_pAmdShaderAnalyzerExt->CreateGraphicsPipelineState(pGraphicsPso,
                IID_PPV_ARGS(&pPipeline), &stats, pPipelineHandle);
            assert(hr == S_OK);
            ret = (hr == S_OK);

            if (ret)
            {
                // Set the statistics values to the output structure.
                SetPipelineResults(stats, results);

                // Retrieve the disassembly.
                ret = RetrieveDisassemblyGraphics(m_pAmdShaderAnalyzerExt, pPipelineHandle, results, errorMsg);
                assert(ret);
            }
            else
            {
                // Log error messages.
                std::stringstream msg;
                msg << STR_ERROR_PIPELINE_CREATE_FAILURE << hr;
                errorMsg = msg.str();
            }
        }
        return ret;

    }

    bool rgDx12Backend::CompileGraphicsPipeline(const D3D12_GRAPHICS_PIPELINE_STATE_DESC* pGraphicsPso,
        rgDx12PipelineResults& results,
        std::string& errorMsg) const
    {
        bool ret = false;
        assert(m_pImpl != nullptr);
        if (m_pImpl != nullptr)
        {
            ret = m_pImpl->CompileGraphicsPipeline(pGraphicsPso, results, errorMsg);
        }
        return ret;
    }

    bool rgDx12Backend::Impl::CompileComputePipeline(const D3D12_COMPUTE_PIPELINE_STATE_DESC* pComputePso,
        rgDx12ShaderResults& shaderResults,
        rgDx12ThreadGroupSize& threadGroupSize,
        std::string& errorMsg) const
    {
        assert(m_pAmdShaderAnalyzerExt != nullptr);
        bool ret = m_pAmdShaderAnalyzerExt != nullptr;
        if (ret)
        {
            ret = pComputePso != nullptr;
            assert(pComputePso != nullptr);
            if (ret)
            {
                AmdExtD3DPipelineHandle* pPipelineHandle = new AmdExtD3DPipelineHandle{};
                ID3D12PipelineState* pPipeline = NULL;
                AmdExtD3DComputeShaderStats driverShaderStats;
                HRESULT hr = m_pAmdShaderAnalyzerExt->CreateComputePipelineState(pComputePso,
                    IID_PPV_ARGS(&pPipeline), &driverShaderStats, pPipelineHandle);
                assert(hr == S_OK);
                assert(pPipeline != NULL);
                ret = (hr == S_OK && pPipeline != NULL);
                if (ret)
                {
                    // Set results.
                    SetShaderResultsCompute(driverShaderStats, shaderResults, threadGroupSize);

                    // Retrieve the disassembly.
                    ret = RetrieveDisassemblyCompute(m_pAmdShaderAnalyzerExt, pPipelineHandle, shaderResults, errorMsg);
                    assert(ret);
                }
                else
                {
                    // Log error messages.
                    errorMsg = STR_ERROR_PIPELINE_STATE_CREATE_FAILURE;
                }
            }
        }
        return ret;

    }

    bool rgDx12Backend::CompileComputePipeline(const D3D12_COMPUTE_PIPELINE_STATE_DESC* pComputePso,
        rgDx12ShaderResults& shaderResults,
        rgDx12ThreadGroupSize& threadGroupSize,
        std::string& errorMsg) const
    {
        bool ret = false;
        assert(m_pImpl != nullptr);
        if (m_pImpl != nullptr)
        {
            ret = m_pImpl->CompileComputePipeline(pComputePso, shaderResults, threadGroupSize, errorMsg);
        }
        return ret;
    }

    rga::rgDx12Backend::rgDx12Backend() : m_pImpl(new rgDx12Backend::Impl) {}


    rga::rgDx12Backend::~rgDx12Backend()
    {
        if (m_pImpl != NULL)
        {
            delete m_pImpl;
            m_pImpl = NULL;
        }
    }

    rga::rgDx12ShaderResults::~rgDx12ShaderResults()
    {
        if (pDisassembly != NULL)
        {
            delete pDisassembly;
            pDisassembly = NULL;
        }
    }
}
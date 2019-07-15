/*
***********************************************************************************************************************
*
*  Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*
*  The above copyright notice and this permission notice shall be included in
*  all copies or substantial portions of the Software.
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*  THE SOFTWARE.
*
***********************************************************************************************************************
*/

/**
***********************************************************************************************************************
* @file  AmdExtD3DShaderAnalyzerApi.h
* @brief
*    AMD D3D Shader Analyzer API include file.
*    This include file contains the Shader Analyzer definitions required for the application to
*    use the API.
***********************************************************************************************************************
*/

#pragma once

#include <unknwn.h>
#include <d3d12.h>

typedef void* AmdExtD3DPipelineHandle;
// Process environment variable to set virtual GPU Id in call to PAL CreatePlatform during Adapter initialization.
static const char AmdExtVirtualGpuIdEnvVarStringName[] = "AmdVirtualGpuId";

/**
***********************************************************************************************************************
* @brief GPU Id and string name
***********************************************************************************************************************
*/
struct AmdExtD3DGpuIdEntry
{
    UINT        gpuId;       ///< GPU Id
    const char* pGpuIdName;  ///< GPU string name in the form of "gpuName:gfxIp"
};

/**
***********************************************************************************************************************
* @brief GPU Id list with Env. varaible name for virtual GPU.
***********************************************************************************************************************
*/
struct AmdExtD3DGpuIdList
{
    UINT                 numGpuIdEntries; // Number of GPU Ids
    AmdExtD3DGpuIdEntry* pGpuIdEntries;   // GPU Id entries
};

/**
***********************************************************************************************************************
* @brief Shader stage type flags
***********************************************************************************************************************
*/
enum AmdExtD3DShaderStageTypeFlags : UINT
{
    AmdExtD3DShaderStageCompute  = 0x00000001,
    AmdExtD3DShaderStageVertex   = 0x00000002,
    AmdExtD3DShaderStageHull     = 0x00000004,
    AmdExtD3DShaderStageDomain   = 0x00000008,
    AmdExtD3DShaderStageGeometry = 0x00000010,
    AmdExtD3DShaderStagePixel    = 0x00000020
};

/**
***********************************************************************************************************************
* @brief Shader usage statistics
***********************************************************************************************************************
*/
struct AmdExtD3DShaderUsageStats
{
    UINT    numUsedVgprs;              ///< Number of VGPRs used by shader
    UINT    numUsedSgprs;              ///< Number of SGPRs used by shader
    UINT    ldsSizePerThreadGroup;     ///< LDS size per thread group in bytes.
    size_t  ldsUsageSizeInBytes;       ///< LDS usage by shader.
    size_t  scratchMemUsageInBytes;    ///< Amount of scratch mem used by shader.
};

/**
***********************************************************************************************************************
* @brief Shader statistics, multiple bits set in the shader stage mask indicates that multiple shaders have been
*        combined by HW. The same information will be repeated for both the constituent shaders in this case.
***********************************************************************************************************************
*/
struct AmdExtD3DShaderStats
{
    UINT                      shaderStageMask;      ///< Indicates stages of pipeline shader stats are for.  Multiple
                                                    ///  bits set indicate shaders stages were merged.
    AmdExtD3DShaderUsageStats usageStats;           ///< Shader reg/LDS/Scratch mem usage.
    UINT                      numPhysicalVgprs;     ///< Number of physical Vgprs.
    UINT                      numPhysicalSgprs;     ///< Number of physical Sgprs.
    UINT                      numAvailableVgprs;    ///< Number of Vgprs made available to shader
    UINT                      numAvailableSgprs;    ///< Number of Sgprs made available to shader
    size_t                    isaSizeInBytes;       ///< Size of the shader ISA disassembly for this shader
};

/**
***********************************************************************************************************************
* @brief Shader statistics for graphics pipeline
***********************************************************************************************************************
*/
struct AmdExtD3DGraphicsShaderStats
{
    AmdExtD3DShaderStats vertexShaderStats;     ///< Vertex Shader stats
    AmdExtD3DShaderStats hullShaderStats;       ///< Hull Shader stats
    AmdExtD3DShaderStats domainShaderStats;     ///< Domain Shader stats
    AmdExtD3DShaderStats geometryShaderStats;   ///< Geometry Shader stats
    AmdExtD3DShaderStats pixelShaderStats;      ///< Pixel Shader stats
};

/**
***********************************************************************************************************************
* @brief Shader statistics for compute pipeline
***********************************************************************************************************************
*/
struct AmdExtD3DComputeShaderStats : public AmdExtD3DShaderStats
{
    UINT numThreadsPerGroupX;           ///< Number of compute threads per thread group in X dimension.
    UINT numThreadsPerGroupY;           ///< Number of compute threads per thread group in Y dimension.
    UINT numThreadsPerGroupZ;           ///< Number of compute threads per thread group in Z dimension.
};


/**
***********************************************************************************************************************
* @brief Shader disassembly code for different shader stage
***********************************************************************************************************************
*/
struct AmdExtD3DPipelineDisassembly
{
    const char* pComputeDisassembly;   ///< Pointer to compute shader disassembly buffer
    const char* pVertexDisassembly;    ///< Pointer to vertex shader disassembly buffer
    const char* pHullDisassembly;      ///< Pointer to hull shader disassembly buffer
    const char* pDomainDisassembly;    ///< Pointer to domain shader disassembly buffer
    const char* pGeometryDisassembly;  ///< Pointer to geometry shader disassembly buffer
    const char* pPixelDisassembly;     ///< Pointer to pixel shader disassembly buffer
};

/**
***********************************************************************************************************************
* @brief Shader Analyzer extension API object
***********************************************************************************************************************
*/
interface __declspec(uuid("A2783A2E-FECA-4881-B3FD-7F8F1D6F4A08"))
IAmdExtD3DShaderAnalyzer: public IUnknown
{
public:
    // Gets list of available virtual GPU Ids
    ///@note The client will need to call this function twice :
    /// First time, client call it with empty AmdExtD3DGpuIdList struct and driver will store the maximum available
    /// virtual GPU number in numGpuIdEntries and return it back to client.
    /// Second time, client is responsible for allocating enough space based on numGpuIdEntries driver returned and
    /// then call this function to get the available GPU ID entries
    virtual HRESULT GetAvailableVirtualGpuIds(AmdExtD3DGpuIdList* pGpuIdList) = 0;

    // Create Graphics pipeline state with shader stats and get the pipeline object handle
    virtual HRESULT CreateGraphicsPipelineState(const D3D12_GRAPHICS_PIPELINE_STATE_DESC* pDesc,
                                                REFIID                                    riid,
                                                void**                                    ppPipelineState,
                                                AmdExtD3DGraphicsShaderStats*             pGraphicsShaderStats,
                                                AmdExtD3DPipelineHandle*                  pPipelineHandle) = 0;

    // Create Compute pipeline state with shader stats and get the pipeline object handle
    virtual HRESULT CreateComputePipelineState(const D3D12_COMPUTE_PIPELINE_STATE_DESC *pDesc,
                                               REFIID                                   riid,
                                               void**                                   ppPipelineState,
                                               AmdExtD3DComputeShaderStats*             pComputeShaderStats,
                                               AmdExtD3DPipelineHandle*                 pPipelineHandle) = 0;

    // Get shader ISA code
    ///@note The client will need to call this function twice to get the ISA code :
    /// First time, client call it with null pIsaCode to query the side of whole ISA code buffer
    /// Second time, the client is responsible for allocating enough space for ISA code buffer. And the pointer to
    /// the ISA code buffer will be returned as pIsaCode. Also, client can get the per-shader stage disassembly
    /// (will be nullptr if it is not present) by passing in the empty AmdExtD3DPipelineDisassembly struct
    virtual HRESULT GetShaderIsaCode(AmdExtD3DPipelineHandle       pipelineHandle,
                                     char*                         pIsaCode,
                                     size_t*                       pIsaCodeSize,
                                     AmdExtD3DPipelineDisassembly* pDisassembly) = 0;
};
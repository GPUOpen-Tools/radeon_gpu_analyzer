//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for dx12 utils.
//=============================================================================
#pragma once

// C++.
#include <string>
#include <vector>

// D3D12.
#include "d3dx12/d3dx12.h"
#include <wrl/client.h>

namespace rga
{
    struct PSO_STREAM {
        CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
        CD3DX12_PIPELINE_STATE_STREAM_VS VS;
        CD3DX12_PIPELINE_STATE_STREAM_PS PS;
        CD3DX12_PIPELINE_STATE_STREAM_DS DS;
        CD3DX12_PIPELINE_STATE_STREAM_HS HS;
        CD3DX12_PIPELINE_STATE_STREAM_GS GS;
        CD3DX12_PIPELINE_STATE_STREAM_AS AS;
        CD3DX12_PIPELINE_STATE_STREAM_MS MS;

        CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER RasterizerState;
        CD3DX12_PIPELINE_STATE_STREAM_BLEND_DESC BlendState;
        CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL DepthStencilState;
        CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
        CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_DESC SampleDesc;
        CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
        CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT InputLayout;
    };

    class RgDx12Utils
    {
    public:
        // Gpso Schema constants.
        static const char* kStrElemSchemaVersion;
        static const char* kStrElemSchemaVersion10;
        static const char* kStrElemSchemaInputLayoutNum;
        static const char* kStrElemSchemaInputLayout;
        static const char* kStrElemSchemaPrimitiveTopologyType;
        static const char* kStrElemSchemaNumRenderTargets;
        static const char* kStrElemSchemaRtvFormats;

        // Files.
        static bool WriteTextFile(const std::string& filename, const std::string& content);
        static bool ReadTextFile(const std::string& filename, std::string& content);
        static bool WriteBinaryFile(const std::string& filename, const std::vector<char>& content);
        static bool ReadBinaryFile(const std::string& filename, std::vector<char>& content);
        static bool WriteBinaryFile(const std::string& filename, const std::vector<unsigned char>& content);
        static bool ReadBinaryFile(const std::string& filename, std::vector<unsigned char>& content);
        static bool IsFileExists(const std::string& full_path);

        // Strings.
        static void SplitString(const std::string& str, char delim, std::vector<std::string>& dst);
        static std::string ToLower(const std::string& str);
        static std::wstring strToWstr(const std::string& str);
        static std::string wstrToStr(const std::wstring& str);

        // Trim leading and trailing white space around the given string.
        // Returns the trimmed string.
        static std::string TrimWhitespace(const std::string& str);

        // Trim leading and trailing '\n' characters.
        // Returns the trimmed string.
        static std::string TrimNewline(const std::string& str);

        // Trim leading and trailing white space and \" characters around the given string.
        // Returns the trimmed string.
        static std::string TrimWhitespaceAndCommas(const std::string& str);

        // Init D3D12 graphics pipeline state descriptor to D3D12 default values.
        static void InitGraphicsPipelineStateDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC& pso_desc);
        static void InitGraphicsPipelineStream(PSO_STREAM& pso_desc);

        // Parse gpso file and set the read values to the pipeline state descriptor.
        // Returns true on success and false otherwise.
        static bool ParseGpsoFile(const std::string& filename, D3D12_GRAPHICS_PIPELINE_STATE_DESC& pso_desc);
        static bool ParseGpsoFile(const std::string& filename, PSO_STREAM& stream);

        // Convert a DXGI_FORMAT enum string representation to the relevant enum value.
        // Returns true on success, and false otherwise.
        static bool StrToDxgiFormat(const std::string& str, DXGI_FORMAT& format);

        // Convert a D3D12_INPUT_CLASSIFICATION enum string representation to the relevant enum value.
        // Returns true on success, and false otherwise.
        static bool StrToInputClassification(const std::string& str, D3D12_INPUT_CLASSIFICATION& input_classification);

        // Convert a D3D12_PRIMITIVE_TOPOLOGY_TYPE enum string representation to the relevant enum value.
        // Returns true on success, and false otherwise.
        static bool StrToPrimitiveTopologyType(const std::string& str, D3D12_PRIMITIVE_TOPOLOGY_TYPE& primitive_topology_type);

    private:
        RgDx12Utils() = delete;
        ~RgDx12Utils() = delete;
    };
}

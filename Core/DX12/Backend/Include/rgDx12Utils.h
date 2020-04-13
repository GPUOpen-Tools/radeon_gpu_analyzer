#pragma once

// C++.
#include <string>
#include <vector>

// D3D12.
#include <d3d12.h>
#include <wrl/client.h>
using namespace Microsoft::WRL;

namespace rga
{
    class rgDx12Utils
    {
    public:
        // Files.
        static bool WriteTextFile(const std::string& fileName, const std::string& content);
        static bool ReadTextFile(const std::string& fileName, std::string& content);
        static bool WriteBinaryFile(const std::string& fileName, const std::vector<char>& content);
        static bool ReadBinaryFile(const std::string& fileName, std::vector<char>& content);

        // Strings.
        static void SplitString(const std::string& str, char delim, std::vector<std::string>& dst);

        // Init D3D12 graphics pipeline state descriptor to D3D12 default values.
        static void InitGraphicsPipelineStateDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC& psoDesc);

        // Parse gpso file and set the read values to the pipeline state descriptor.
        // Returns true on success and false otherwise.
        static bool ParseGpsoFile(const std::string& fileName, D3D12_GRAPHICS_PIPELINE_STATE_DESC& psoDesc);

        // Convert a DXGI_FORMAT enum string representation to the relevant enum value.
        // Returns true on success, and false otherwise.
        static bool StrToDxgiFormat(const std::string& str, DXGI_FORMAT& format);

        // Convert a D3D12_INPUT_CLASSIFICATION enum string representation to the relevant enum value.
        // Returns true on success, and false otherwise.
        static bool StrToInputClassification(const std::string& str, D3D12_INPUT_CLASSIFICATION& inputClassification);

        // Convert a D3D12_PRIMITIVE_TOPOLOGY_TYPE enum string representation to the relevant enum value.
        // Returns true on success, and false otherwise.
        static bool StrToPrimitiveTopologyType(const std::string& str, D3D12_PRIMITIVE_TOPOLOGY_TYPE& primitiveTopologyType);

        // Trim leading and trailing white space around the given string.
        // Returns the trimmed string.
        static std::string TrimWhitespace(const std::string& str);

        // Trim leading and trailing '\n' characters.
        // Returns the trimmed string.
        static std::string TrimNewline(const std::string& str);

        // Trim leading and trailing white space and \" characters around the given string.
        // Returns the trimmed string.
        static std::string TrimWhitespaceAndCommas(const std::string& str);

        // Accepts a root signature blob and dumps it to a file according to the given file name.
        // Returns true on success, and false otherwise.
        static bool DumpRootSignature(const std::string& outputFileName, const ComPtr<ID3DBlob> signature);

        // Accepts a root signature descriptor and dumps the serialized root signature blob into a file according
        // to the given file name. Returns true on success, and false otherwise.
        static bool DumpRootSignature(const std::string& outputFileName, const ComPtr<ID3D12Device>,
            const D3D12_VERSIONED_ROOT_SIGNATURE_DESC& rsDesc);

    private:
        rgDx12Utils() = delete;
        ~rgDx12Utils() = delete;
    };
}

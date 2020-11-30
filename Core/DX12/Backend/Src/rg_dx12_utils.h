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
    class RgDx12Utils
    {
    public:
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

        // Parse gpso file and set the read values to the pipeline state descriptor.
        // Returns true on success and false otherwise.
        static bool ParseGpsoFile(const std::string& filename, D3D12_GRAPHICS_PIPELINE_STATE_DESC& pso_desc);

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

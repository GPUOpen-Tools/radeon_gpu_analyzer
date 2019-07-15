#pragma once

// C++.
#include <string>
#include <vector>

// D3D12.
#include <d3d12.h>

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

    private:
        rgDx12Utils() = delete;
        ~rgDx12Utils() = delete;
    };
}

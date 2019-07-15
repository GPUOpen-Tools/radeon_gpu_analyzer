#include <rgDx12Utils.h>

// C++.
#include <cassert>
#include <iostream>
#include <fstream>
#include <sstream>

namespace rga
{
    // *** CONSTANTS - START ***

    static const char* STR_ERR_TEXT_FILE_WRITE_FAILED = "Error: failed to write text file: ";
    static const char* STR_ERR_TEXT_FILE_READ_FAILED = "Error: failed to read text file: ";
    static const char* STR_ERR_BINARY_FILE_WRITE_FAILED = "Error: failed to write binary file: ";

    // *** CONSTANTS - END ***

    bool rgDx12Utils::WriteTextFile(const std::string& fileName, const std::string& content)
    {
        bool ret = false;
        std::ofstream output;
        output.open(fileName.c_str());

        if (output.is_open())
        {
            output << content << std::endl;
            output.close();
            ret = true;
        }
        else
        {
            std::cerr << STR_ERR_TEXT_FILE_WRITE_FAILED << fileName << std::endl;
        }

        return ret;
    }

    bool rgDx12Utils::ReadTextFile(const std::string& fileName, std::string& content)
    {
        bool ret = false;
        std::ifstream input;
        input.open(fileName.c_str());

        if (input.is_open())
        {
            std::stringstream  textStream;
            textStream << input.rdbuf();
            content = textStream.str();
            input.close();
            ret = true;
        }
        else
        {
            std::cout << STR_ERR_TEXT_FILE_READ_FAILED << fileName << std::endl;
        }

        return ret;
    }

    bool rgDx12Utils::ReadBinaryFile(const std::string& fileName, std::vector<char>& content)
    {
        bool ret = false;
        std::ifstream input;
        input.open(fileName.c_str(), std::ios::binary);

        if (input.is_open())
        {
            content = std::vector<char>(std::istreambuf_iterator<char>(input), {});
            ret = !content.empty();
        }
        return ret;
    }

    void rgDx12Utils::SplitString(const std::string& str, char delim, std::vector<std::string>& dst)
    {
        std::stringstream ss;
        ss.str(str);
        std::string substr;
        while (std::getline(ss, substr, delim))
        {
            dst.push_back(substr);
        }
    }

    void rgDx12Utils::InitGraphicsPipelineStateDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC& psoDesc)
    {
        // Reset the structure.
        memset(&psoDesc, 0, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

        // Default D3D12 rasterizer state.
        D3D12_RASTERIZER_DESC DefaultDx12RasterizerState;
        DefaultDx12RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
        DefaultDx12RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
        DefaultDx12RasterizerState.FrontCounterClockwise = FALSE;
        DefaultDx12RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
        DefaultDx12RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
        DefaultDx12RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
        DefaultDx12RasterizerState.DepthClipEnable = TRUE;
        DefaultDx12RasterizerState.MultisampleEnable = FALSE;
        DefaultDx12RasterizerState.AntialiasedLineEnable = FALSE;
        DefaultDx12RasterizerState.ForcedSampleCount = 0;
        DefaultDx12RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

        // Default D3D12 blend state.
        D3D12_BLEND_DESC DefaultDx12BlendState;
        DefaultDx12BlendState.AlphaToCoverageEnable = FALSE;
        DefaultDx12BlendState.IndependentBlendEnable = FALSE;
        const D3D12_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc =
        {
            FALSE,FALSE,
            D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
            D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
            D3D12_LOGIC_OP_NOOP,
            D3D12_COLOR_WRITE_ENABLE_ALL,
        };
        for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
        {
            DefaultDx12BlendState.RenderTarget[i] = defaultRenderTargetBlendDesc;
        }

        // Default D3D12 depth stencil state.
        D3D12_DEPTH_STENCIL_DESC defaultDx12DepthStencilState;
        defaultDx12DepthStencilState.DepthEnable = TRUE;
        defaultDx12DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
        defaultDx12DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
        defaultDx12DepthStencilState.StencilEnable = FALSE;
        defaultDx12DepthStencilState.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
        defaultDx12DepthStencilState.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
        const D3D12_DEPTH_STENCILOP_DESC defaultStencilOp =
        { D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_COMPARISON_FUNC_ALWAYS };
        defaultDx12DepthStencilState.FrontFace = defaultStencilOp;
        defaultDx12DepthStencilState.BackFace = defaultStencilOp;

        // Set the D3D12 defaults.
        psoDesc.BlendState = DefaultDx12BlendState;
        psoDesc.DepthStencilState = defaultDx12DepthStencilState;
        psoDesc.RasterizerState = DefaultDx12RasterizerState;

        psoDesc.SampleMask = UINT_MAX;

        // Assume triangle primitive topology type.
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

        // Assume single render target.
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;

        // Assume a single multisapmple per pixel.
        psoDesc.SampleDesc.Count = 1;
    }

    bool rgDx12Utils::WriteBinaryFile(const std::string& fileName, const std::vector<char>& content)
    {
        bool ret = false;
        std::ofstream output;
        output.open(fileName.c_str(), std::ios::binary);

        if (output.is_open() && !content.empty())
        {
            output.write(&content[0], content.size());
            output.close();
            ret = true;
        }
        else
        {
            std::cerr << STR_ERR_BINARY_FILE_WRITE_FAILED << fileName << std::endl;
        }

        return ret;
    }

}
// D3D12.
#include <d3d12.h>
#include "d3dcompiler.h"
#include "d3dcommon.h"
#include <wrl.h>

// C++.
#include <cassert>
#include <iostream>
#include <sstream>
#include <locale>
#include <codecvt>
#include <vector>

// Local.
#include <rgDx12Frontend.h>
#include <rgDx12Utils.h>

// Backend.
#include <beD3DIncludeManager.h>

namespace rga
{
    // *** CONSTANTS - START ***

    static const char* STR_ERROR_ROOT_SIGNATURE_EXTRACTION_FAILURE = "Error: failed to extract root signature from DXCB binary.";
    static const char* STR_ERROR_SHADER_COMPILATION_FAILURE = "Error: failed to compile shader: ";
    static const char* STR_ERROR_COMPUTE_SHADER_DISASSEMBLY_EXCTRACTION_FAILURE = "Error: failed to extract GCN ISA disassembly for compute shader.";
    static const char* STR_ERROR_GRAPHICS_SHADER_DISASSEMBLY_EXCTRACTION_FAILURE_A = "Error: failed to extract GCN ISA disassembly for ";
    static const char* STR_ERROR_GRAPHICS_SHADER_DISASSEMBLY_EXCTRACTION_FAILURE_B = " shader.";
    static const char* STR_ERROR_ROOT_SIGNATURE_COMPILE_FAILURE = "Error: failed to compile root signature.";
    static const char* STR_ERROR_DXBC_DISASSEMBLE_FAILURE = "Error: failed to disassemble compute shader DXBC binary.";
    static const char* STR_ERROR_DXBC_READ_FAILURE = "Error: failed to read input DXBC binary: ";
    static const char* STR_ERROR_EXTRACT_COMPUTE_SHADER_STATS_FAILURE = "Error: failed to extract compute shader statistics.";
    static const char* STR_ERROR_EXTRACT_COMPUTE_SHADER_DISASSEMBLY_FAILURE = "Error: failed to extract compute shader disassembly.";
    static const char* STR_ERROR_EXTRACT_GRAPHICS_SHADER_OUTPUT_FAILURE_A = "Error: failed to extract ";
    static const char* STR_ERROR_EXTRACT_GRAPHICS_SHADER_STATS_FAILURE_B = " shader statistics.";
    static const char* STR_ERROR_EXTRACT_GRAPHICS_SHADER_DISASSEMBLY_FAILURE_B = " shader disassembly.";
    static const char* STR_ERROR_FRONT_END_COMPILATION_FAILURE = "Error: front-end compilation of hlsl to DXBC failed.";
    static const char* STR_ERROR_FAILED_TO_FIND_DX12_ADAPTER = "Error: failed to find a DX12 display adapter.";
    static const char* STR_ERROR_FAILED_TO_CREATE_GRAPHICS_PIPELINE = "Error: failed to create graphics pipeline.";
    static const char* STR_ERROR_GPSO_FILE_PARSE_FAILED = "Error: failed to parse graphics pipeline description file.";
    static const char* STR_ERROR_RS_FILE_READ_FAILED = "Error: failed to read binary file for root signature: ";
    static const char* STR_ERROR_RS_FILE_COMPILE_FAILED = "Error: failed to compile root signature after reading binary data from file: ";
    static const char* STR_INFO_EXTRACT_COMPUTE_SHADER_STATS = "Extracting compute shader statistics...";
    static const char* STR_INFO_EXTRACT_COMPUTE_SHADER_DISASSEMBLY = "Extracting compute shader disassembly...";
    static const char* STR_INFO_EXTRACT_GRAPHICS_SHADER_DISASSEMBLY = " shader disassembly...";
    static const char* STR_INFO_EXTRACT_GRAPHICS_SHADER_OUTPUT_A = "Extracting ";
    static const char* STR_INFO_EXTRACT_GRAPHICS_SHADER_STATS_B = " shader statistics...";
    static const char* STR_INFO_EXTRACT_COMPUTE_SHADER_STATS_SUCCESS = "Compute shader statistics extracted successfully.";
    static const char* STR_INFO_EXTRACT_COMPUTE_SHADER_DISASSEMBLY_SUCCESS = "Compute shader disassembly extracted successfully.";
    static const char* STR_INFO_EXTRACT_GRAPHICS_SHADER_STATS_SUCCESS = " shader statistics extracted successfully.";
    static const char* STR_INFO_EXTRACT_GRAPHICS_SHADER_DISASSEMBLY_SUCCESS = " shader disassembly extracted successfully.";

    // *** CONSTANTS - END ***

    static void Dx12StatsToString(const rgDx12ShaderResults& stats, std::stringstream& serializedStats)
    {
        serializedStats << "Statistics:" << std::endl;
        serializedStats << "    - shaderStageMask                           = " << stats.shaderMask << std::endl;
        serializedStats << "    - resourceUsage.numUsedVgprs                = " << stats.vgprUsed << std::endl;
        serializedStats << "    - resourceUsage.numUsedSgprs                = " << stats.sgprUsed<< std::endl;
        serializedStats << "    - resourceUsage.ldsSizePerLocalWorkGroup    = " << stats.ldsAvailableBytes << std::endl;
        serializedStats << "    - resourceUsage.ldsUsageSizeInBytes         = " << stats.ldsUsedBytes << std::endl;
        serializedStats << "    - resourceUsage.scratchMemUsageInBytes      = " << stats.scratchUsedBytes << std::endl;
        serializedStats << "    - numPhysicalVgprs                          = " << stats.vgprPhysical << std::endl;
        serializedStats << "    - numPhysicalSgprs                          = " << stats.sgprPhysical << std::endl;
        serializedStats << "    - numAvailableVgprs                         = " << stats.vgprAvailable << std::endl;
        serializedStats << "    - numAvailableSgprs                         = " << stats.sgprAvailable << std::endl;
    }

    // Serialize a graphics shader's statistics.
    static bool SerializeDx12StatsGraphics(const rgDx12ShaderResults& stats, const std::string& outputFileName)
    {
        // Convert the statistics to string.
        std::stringstream serializedStats;
        Dx12StatsToString(stats, serializedStats);

        // Write the results to the output file.
        bool ret = rgDx12Utils::WriteTextFile(outputFileName, serializedStats.str());
        assert(ret);
        return ret;
    }

    // Serialize a compute shader's statistics including the thread group dimensions.
    static bool SerializeDx12StatsCompute(const rgDx12ShaderResults& stats, const rgDx12ThreadGroupSize& threadGroupSize,
        const std::string& outputFileName)
    {
        // Serialize the common shader stats.
        std::stringstream serializedStats;
        Dx12StatsToString(stats, serializedStats);

        // Serialize and append the compute thread group size.
        serializedStats << "    - computeWorkGroupSizeX" << " = " << threadGroupSize.x << std::endl;
        serializedStats << "    - computeWorkGroupSizeY" << " = " << threadGroupSize.y << std::endl;
        serializedStats << "    - computeWorkGroupSizeZ" << " = " << threadGroupSize.z << std::endl;

        // Write the results to the output file.
        bool ret = rgDx12Utils::WriteTextFile(outputFileName, serializedStats.str());
        assert(ret);
        return ret;
    }

    static bool CreateDefaultRootSignature(ID3D12Device* pDevice, ID3D12RootSignature*& pRootSignature)
    {
        bool ret = false;
        assert(pDevice != nullptr);
        if (pDevice != nullptr)
        {
            D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc;
            memset(&rootSignatureDesc, 0, sizeof(rootSignatureDesc));
            rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;
            ComPtr<ID3DBlob> signature;
            ComPtr<ID3DBlob> error;
            D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
            pDevice->CreateRootSignature(0, signature->GetBufferPointer(),
                signature->GetBufferSize(), IID_PPV_ARGS(&pRootSignature));
            assert(pRootSignature != nullptr);
            ret = (pRootSignature != nullptr);
        }
        return ret;
    }

    static std::string ExtractFileDirectory(const std::string& fullPath)
    {
        std::string ret;
        size_t pos = fullPath.rfind('\\');
        if (pos != std::string::npos)
        {
            ret = fullPath.substr(0, pos);
        }
        return ret;
    }

    static bool CompileHlslShader(const rgDx12Config& config, const std::string& hlslFullPath,
        const std::string& entryPoint, const std::string& shaderModel,
        D3D12_SHADER_BYTECODE &byteCode, std::string &errorMsg)
    {
        bool ret = false;

        // Convert the shader file name to wide characters.
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        std::wstring hlslFullPathWide = converter.from_bytes(hlslFullPath);

        // Extract the hlsl file's folder and instantiate the include manager.
        std::string hlslFileFolder = ExtractFileDirectory(hlslFullPath);
        D3DIncludeManager includeManager(hlslFileFolder.c_str(), config.includeDirs);

        // Preprocessor definitions.
        D3D_SHADER_MACRO* pMacros = new D3D_SHADER_MACRO[config.defines.size() + 1]{};

        // Handle the defines.
        if (!config.defines.empty())
        {
            int i = 0;
            for (const auto& preprocessorDefine : config.defines)
            {
                std::vector<std::string> macroParts;
                rgDx12Utils::SplitString(preprocessorDefine, '=', macroParts);
                assert(macroParts.size() > 0 && macroParts.size() < 3);
                if (macroParts.size() > 0 && macroParts.size() < 3)
                {
                    pMacros[i].Name = new char[macroParts[0].size() + 1]{};
                    memcpy((void*)pMacros[i].Name, macroParts[0].data(), macroParts[0].size());
                    if (macroParts.size() > 1)
                    {
                        // This is a X=Y macro.
                        pMacros[i].Definition = new char[macroParts[1].size() + 1]{};
                        memcpy((void*)pMacros[i].Definition, macroParts[1].data(), macroParts[1].size());
                    }
                    else
                    {
                        // This is just a definition.
                        pMacros[i].Definition = "";
                    }
                    i++;
                }
            }
        }

        // Front-end compilation.
        ComPtr<ID3DBlob> compiledBlob;
        ComPtr<ID3DBlob> pErrorMessages;
        UINT compileFlags = 0;
        HRESULT hr = D3DCompileFromFile(hlslFullPathWide.c_str(), pMacros, &includeManager,
            entryPoint.c_str(), shaderModel.c_str(),
            compileFlags, 0, &compiledBlob, &pErrorMessages);

        assert(hr == S_OK);
        if (hr == S_OK)
        {
            // Extract the byte code.
            byteCode.BytecodeLength = compiledBlob->GetBufferSize();
            char* pBuffer = new char[byteCode.BytecodeLength]();
            memcpy(pBuffer, compiledBlob->GetBufferPointer(), byteCode.BytecodeLength);
            byteCode.pShaderBytecode = pBuffer;
            ret = true;
        }
        else
        {
            if (pErrorMessages != nullptr && pErrorMessages->GetBufferSize() > 0)
            {
                std::cout << (char*)pErrorMessages->GetBufferPointer() << std::endl;;
            }
            std::cout << STR_ERROR_FRONT_END_COMPILATION_FAILURE << std::endl;
        }

        // Clean up.
        for (size_t i = 1; i < config.defines.size(); i++)
        {
            delete[] pMacros[i].Name;
            pMacros[i].Name = nullptr;
            delete[] pMacros[i].Definition;
            pMacros[i].Definition = nullptr;
        }
        delete[] pMacros;
        pMacros = nullptr;

        return ret;
    }


    static bool ReadDxbcBinary(const std::string& dxbcFullPath, D3D12_SHADER_BYTECODE &byteCode)
    {
        bool ret = false;

        // Read the compiled DXBC from a file.
        std::vector<char> compiledShader;
        bool isDxbcBinaryRead = rgDx12Utils::ReadBinaryFile(dxbcFullPath, compiledShader);
        assert(isDxbcBinaryRead);
        if (isDxbcBinaryRead)
        {
            char* pBuffer = new char[compiledShader.size()]();
            memcpy(pBuffer, compiledShader.data(), compiledShader.size());
            byteCode.pShaderBytecode = pBuffer;
            byteCode.BytecodeLength = compiledShader.size();
            ret = true;
        }
        else
        {
            std::cout << STR_ERROR_DXBC_READ_FAILURE << dxbcFullPath << std::endl;
        }

        return ret;
    }

    static bool WriteDxbcFile(const D3D12_SHADER_BYTECODE &byteCode, const std::string& outputFile)
    {
        char* pBuffer = (char*)byteCode.pShaderBytecode;
        std::vector<char> compBc(pBuffer, pBuffer + byteCode.BytecodeLength);
        bool isFileWritten = rgDx12Utils::WriteBinaryFile(outputFile, compBc);
        assert(isFileWritten);
        return isFileWritten;
    }

    static bool DisassembleDxbc(const D3D12_SHADER_BYTECODE &byteCode, const std::string& outputFile)
    {
        bool ret = false;

        ID3DBlob* pDisassembly = nullptr;
        HRESULT hr = D3DDisassemble(byteCode.pShaderBytecode, byteCode.BytecodeLength, 0, "", &pDisassembly);
        assert(hr == S_OK);
        assert(pDisassembly != nullptr);
        assert(pDisassembly->GetBufferSize() > 0);
        if (SUCCEEDED(hr) && pDisassembly != nullptr && pDisassembly->GetBufferSize() > 0)
        {
            std::string dxbcDisassembly((char*)pDisassembly->GetBufferPointer(),
                (char*)pDisassembly->GetBufferPointer() + pDisassembly->GetBufferSize());
            ret = rgDx12Utils::WriteTextFile(outputFile, dxbcDisassembly);
            assert(ret);
        }
        else
        {
            std::cout << STR_ERROR_DXBC_DISASSEMBLE_FAILURE << std::endl;
        }

        return ret;
    }

    static bool CompileRootSignature(const rgDx12Config&config, ComPtr<ID3DBlob>& compiledRs)
    {
        bool ret = false;

        if (!config.rsMacroFile.empty() && !config.rsMacro.empty())
        {
            // Read the content of the HLSL file.
            std::string hlslText;
            bool isFileRead = rgDx12Utils::ReadTextFile(config.rsMacroFile, hlslText);
            assert(isFileRead);
            assert(!hlslText.empty());
            if (isFileRead)
            {
                // Extract the HLSL file's folder and instantiate the include manager.
                std::string hlslFileFolder = ExtractFileDirectory(config.rsMacroFile);
                D3DIncludeManager includeManager(hlslFileFolder.c_str(), config.includeDirs);

                // Root signature compilation.
                ComPtr<ID3DBlob> pErrorMessages;
                HRESULT hr = D3DCompile(hlslText.c_str(), hlslText.size(), nullptr,
                    nullptr, &includeManager,
                    config.rsMacro.c_str(), config.rsVersion.c_str(), 0, 0, &compiledRs, &pErrorMessages);

                if (pErrorMessages != nullptr)
                {
                    std::cout << (char*)pErrorMessages->GetBufferPointer() << std::endl;
                }

                assert(hr == S_OK);
                assert(compiledRs != nullptr);
                ret = SUCCEEDED(hr) && compiledRs != nullptr;
                if (!ret)
                {
                    std::cout << STR_ERROR_ROOT_SIGNATURE_COMPILE_FAILURE << std::endl;
                }
            }
        }

        return ret;
    }

    bool rgDx12Frontend::CreateComputePipeline(const rgDx12Config& config,
        D3D12_COMPUTE_PIPELINE_STATE_DESC*& pComputePso,
        std::string& errorMsg) const
    {
        bool ret = false;
        pComputePso = new D3D12_COMPUTE_PIPELINE_STATE_DESC();

        // If the input is DXBC binary, we do not need front-end compilation.
        bool isFrontEndCompilationRequired = !config.comp.hlsl.empty();

        // If the root signature was already created, use it. Otherwise,
        // try to extract it from the DXBC binary after compiling the HLSL code.
        bool shouldExtractRootSignature = !config.rsMacro.empty();

        // Check if the user provided a serialized root signature file.
        bool isSerializedRootSignature = !config.rsSerialized.empty();

        // Buffer to hold DXBC compiled shader.
        D3D12_SHADER_BYTECODE byteCode;
        HRESULT hr = E_FAIL;

        if (isFrontEndCompilationRequired)
        {
            // Compile the HLSL file.
            ret = CompileHlslShader(config, config.comp.hlsl, config.comp.entryPoint,
                config.comp.shaderModel, byteCode, errorMsg);
            assert(ret);
            if (ret)
            {
                pComputePso->CS = byteCode;
            }
            else
            {
                errorMsg = STR_ERROR_SHADER_COMPILATION_FAILURE;
                errorMsg.append("compute\n");
            }
        }
        else
        {
            // Read the compiled HLSL binary.
            ret = ReadDxbcBinary(config.comp.dxbc,byteCode);
            assert(ret);
            if (ret)
            {
                pComputePso->CS = byteCode;
            }
        }

        if (!config.comp.dxbcOut.empty())
        {
            // If the user wants to dump the bytecode as a binary, do it here.
            bool isDxbcWritten = WriteDxbcFile(byteCode, config.comp.dxbcOut);
            assert(isDxbcWritten);
        }

        if (ret)
        {
            if (!config.comp.dxbcDisassembly.empty())
            {
                // If the user wants to dump the bytecode disassembly, do it here.
                bool isDxbcDisassemblyGenerated = DisassembleDxbc(byteCode, config.comp.dxbcDisassembly);
                assert(isDxbcDisassemblyGenerated);
            }

            if (shouldExtractRootSignature)
            {
                // If the input is HLSL, we need to compile the root signature out of the HLSL file.
                // Otherwise, if the input is a DXBC binary and it has a root signature baked into it,
                // then the root signature would be automatically fetched from the binary, there is no
                // need to set it into the PSO's root signature field.
                ComPtr<ID3DBlob> compiledRs;
                ret = CompileRootSignature(config, compiledRs);
                if (ret)
                {
                    // Create the root signature through the device and assign it to our PSO.
                    std::vector<uint8_t> rootSignatureBuffer;
                    rootSignatureBuffer.resize(compiledRs->GetBufferSize());
                    memcpy(rootSignatureBuffer.data(), compiledRs->GetBufferPointer(),
                        compiledRs->GetBufferSize());
                    m_device.Get()->CreateRootSignature(0, rootSignatureBuffer.data(),
                        rootSignatureBuffer.size(), IID_PPV_ARGS(&pComputePso->pRootSignature));
                    assert(pComputePso->pRootSignature != nullptr);
                    ret = pComputePso->pRootSignature != nullptr;
                }
            }
            else if (isSerializedRootSignature)
            {
                // Read the root signature from the file and recreate it.
                std::vector<char> rootSignatureBuffer;
                ret = rgDx12Utils::ReadBinaryFile(config.rsSerialized, rootSignatureBuffer);
                assert(ret);
                if (ret)
                {
                    m_device.Get()->CreateRootSignature(0, rootSignatureBuffer.data(),
                        rootSignatureBuffer.size(), IID_PPV_ARGS(&pComputePso->pRootSignature));
                    assert(pComputePso->pRootSignature != nullptr);
                    ret = pComputePso->pRootSignature != nullptr;
                }
            }
        }

        return ret;
    }

    static bool CompileGraphicsShaders(const rgDx12Config &config,
        rgDx12PipelineByteCode &byteCode, D3D12_GRAPHICS_PIPELINE_STATE_DESC*& pPso,
        std::string &errorMsg)
    {
        bool ret = false;

        // If the input is DXBC binary, we do not need front-end compilation.
        rgPipelineBool isFrontEndCompilationRequired;
        isFrontEndCompilationRequired.vert = !config.vert.hlsl.empty();
        isFrontEndCompilationRequired.hull = !config.hull.hlsl.empty();
        isFrontEndCompilationRequired.domain = !config.domain.hlsl.empty();
        isFrontEndCompilationRequired.geom = !config.geom.hlsl.empty();
        isFrontEndCompilationRequired.pixel = !config.pixel.hlsl.empty();

        // Vertex.
        if (isFrontEndCompilationRequired.vert)
        {
            // Compile the vertex shader.
            ret = CompileHlslShader(config, config.vert.hlsl, config.vert.entryPoint,
                config.vert.shaderModel, byteCode.vert, errorMsg);
            assert(ret);
            if (ret)
            {
                pPso->VS = byteCode.vert;
            }
            else
            {
                errorMsg = STR_ERROR_SHADER_COMPILATION_FAILURE;
                errorMsg.append("vertex\n");
            }
        }
        else if(!config.vert.dxbc.empty())
        {
            // Read the compiled vertex shader.
            ret = ReadDxbcBinary(config.vert.dxbc, byteCode.vert);
            assert(ret);
            if (ret)
            {
                pPso->VS = byteCode.vert;
            }
        }

        // Hull.
        if (isFrontEndCompilationRequired.hull)
        {
            // Compile the hull shader.
            ret = CompileHlslShader(config, config.hull.hlsl, config.hull.entryPoint,
                config.hull.shaderModel, byteCode.vert, errorMsg);
            assert(ret);
            if (ret)
            {
                pPso->HS = byteCode.hull;
            }
            else
            {
                errorMsg = STR_ERROR_SHADER_COMPILATION_FAILURE;
                errorMsg.append("hull\n");
            }
        }
        else if (!config.hull.dxbc.empty())
        {
            // Read the compiled hull shader.
            ret = ReadDxbcBinary(config.hull.dxbc, byteCode.hull);
            assert(ret);
            if (ret)
            {
                pPso->HS = byteCode.hull;
            }
        }

        // Domain.
        if (isFrontEndCompilationRequired.domain)
        {
            // Compile the domain shader.
            ret = CompileHlslShader(config, config.domain.hlsl, config.domain.entryPoint,
                config.domain.shaderModel, byteCode.domain, errorMsg);
            assert(ret);
            if (ret)
            {
                pPso->DS = byteCode.domain;
            }
            else
            {
                errorMsg = STR_ERROR_SHADER_COMPILATION_FAILURE;
                errorMsg.append("domain\n");
            }
        }
        else if (!config.domain.dxbc.empty())
        {
            // Read the compiled domain shader.
            ret = ReadDxbcBinary(config.domain.dxbc, byteCode.domain);
            assert(ret);
            if (ret)
            {
                pPso->DS = byteCode.domain;
            }
        }

        // Geometry.
        if (isFrontEndCompilationRequired.geom)
        {
            // Compile the geometry shader.
            ret = CompileHlslShader(config, config.geom.hlsl, config.geom.entryPoint,
                config.geom.shaderModel, byteCode.geom, errorMsg);
            assert(ret);
            if (ret)
            {
                pPso->GS = byteCode.geom;
            }
            else
            {
                errorMsg = STR_ERROR_SHADER_COMPILATION_FAILURE;
                errorMsg.append("geometry\n");
            }
        }
        else if (!config.geom.dxbc.empty())
        {
            // Read the compiled geometry shader.
            ret = ReadDxbcBinary(config.geom.dxbc, byteCode.geom);
            assert(ret);
            if (ret)
            {
                pPso->GS = byteCode.geom;
            }
        }

        // Pixel.
        if (isFrontEndCompilationRequired.pixel)
        {
            // Compile the pixel shader.
            ret = CompileHlslShader(config, config.pixel.hlsl, config.pixel.entryPoint,
                config.pixel.shaderModel, byteCode.pixel, errorMsg);
            assert(ret);
            if (ret)
            {
                pPso->PS = byteCode.pixel;
            }
            else
            {
                errorMsg = STR_ERROR_SHADER_COMPILATION_FAILURE;
                errorMsg.append("pixel\n");
            }
        }
        else if (!config.pixel.dxbc.empty())
        {
            // Read the compiled pixel shader.
            ret = ReadDxbcBinary(config.pixel.dxbc, byteCode.pixel);
            assert(ret);
            if (ret)
            {
                pPso->PS = byteCode.pixel;
            }
        }
        return ret;
    }

    static void DumpDxbcBinaries(const rgDx12Config &config, const D3D12_GRAPHICS_PIPELINE_STATE_DESC& pPso)
    {
        if (!config.vert.dxbcOut.empty())
        {
            bool isDxbcWritten = WriteDxbcFile(pPso.VS, config.vert.dxbcOut);
            assert(isDxbcWritten);
        }
        if (!config.hull.dxbcOut.empty())
        {
            bool isDxbcWritten = WriteDxbcFile(pPso.HS, config.hull.dxbcOut);
            assert(isDxbcWritten);
        }
        if (!config.domain.dxbcOut.empty())
        {
            bool isDxbcWritten = WriteDxbcFile(pPso.DS, config.domain.dxbcOut);
            assert(isDxbcWritten);
        }
        if (!config.geom.dxbcOut.empty())
        {
            bool isDxbcWritten = WriteDxbcFile(pPso.GS, config.geom.dxbcOut);
            assert(isDxbcWritten);
        }
        if (!config.pixel.dxbcOut.empty())
        {
            bool isDxbcWritten = WriteDxbcFile(pPso.PS, config.pixel.dxbcOut);
            assert(isDxbcWritten);
        }
    }

    static void DumpDxbcDisassembly(const rgDx12Config &config, const rgDx12PipelineByteCode &byteCode)
    {
        if (!config.vert.dxbcDisassembly.empty())
        {
            bool isDxbcDisassemblyGenerated = DisassembleDxbc(byteCode.vert, config.vert.dxbcDisassembly);
            assert(isDxbcDisassemblyGenerated);
        }
        if (!config.hull.dxbcDisassembly.empty())
        {
            bool isDxbcDisassemblyGenerated = DisassembleDxbc(byteCode.hull, config.hull.dxbcDisassembly);
            assert(isDxbcDisassemblyGenerated);
        }
        if (!config.domain.dxbcDisassembly.empty())
        {
            bool isDxbcDisassemblyGenerated = DisassembleDxbc(byteCode.domain, config.domain.dxbcDisassembly);
            assert(isDxbcDisassemblyGenerated);
        }
        if (!config.geom.dxbcDisassembly.empty())
        {
            bool isDxbcDisassemblyGenerated = DisassembleDxbc(byteCode.geom, config.geom.dxbcDisassembly);
            assert(isDxbcDisassemblyGenerated);
        }
        if (!config.pixel.dxbcDisassembly.empty())
        {
            bool isDxbcDisassemblyGenerated = DisassembleDxbc(byteCode.pixel, config.pixel.dxbcDisassembly);
            assert(isDxbcDisassemblyGenerated);
        }
    }

    bool rgDx12Frontend::CreateGraphicsPipeline(const rgDx12Config& config,
        D3D12_GRAPHICS_PIPELINE_STATE_DESC*& pPso, std::string& errorMsg) const
    {
        bool ret = false;

        // Create and initialize graphics pipeline state descriptor.
        pPso = new D3D12_GRAPHICS_PIPELINE_STATE_DESC();
        rgDx12Utils::InitGraphicsPipelineStateDesc(*pPso);

        // If the root signature was already created, use it. Otherwise,
        // try to extract it from the DXBC binary after compiling the HLSL code.
        bool shouldExtractRootSignature = !config.rsMacro.empty();

        // Check if the user provided a serialized root signature file.
        bool isSerializedRootSignature = !config.rsSerialized.empty();

        rgDx12PipelineByteCode byteCode;
        HRESULT hr = E_FAIL;

        // Compile the graphics pipeline shaders.
        ret = CompileGraphicsShaders(config, byteCode, pPso, errorMsg);

        // If the user wants to dump the bytecode as a binary, do it here.
        DumpDxbcBinaries(config, *pPso);

        assert(ret);
        if (ret)
        {
            // If the user wants to dump the bytecode disassembly, do it here.
            DumpDxbcDisassembly(config, byteCode);

            if (shouldExtractRootSignature)
            {
                ComPtr<ID3DBlob> compiledRs;
                ret = CompileRootSignature(config, compiledRs);
                if (ret)
                {
                    // Create the root signature through the device and assign it to our PSO.
                    std::vector<uint8_t> rootSignatureBuffer;
                    rootSignatureBuffer.resize(compiledRs->GetBufferSize());
                    memcpy(rootSignatureBuffer.data(), compiledRs->GetBufferPointer(),
                        compiledRs->GetBufferSize());
                    m_device.Get()->CreateRootSignature(0, rootSignatureBuffer.data(),
                        rootSignatureBuffer.size(), IID_PPV_ARGS(&pPso->pRootSignature));
                    assert(pPso->pRootSignature != nullptr);
                    ret = pPso->pRootSignature != nullptr;
                }
            }
            else if (isSerializedRootSignature)
            {
                // Read the root signature from the file and recreate it.
                std::vector<char> rootSignatureBuffer;
                ret = rgDx12Utils::ReadBinaryFile(config.rsSerialized, rootSignatureBuffer);
                assert(ret);
                if (ret)
                {
                    m_device.Get()->CreateRootSignature(0, rootSignatureBuffer.data(),
                        rootSignatureBuffer.size(), IID_PPV_ARGS(&pPso->pRootSignature));
                    assert(pPso->pRootSignature != nullptr);
                    ret = pPso->pRootSignature != nullptr;
                    if (pPso->pRootSignature == nullptr)
                    {
                        std::cout << STR_ERROR_RS_FILE_COMPILE_FAILED << config.rsSerialized << std::endl;
                    }
                }
                else
                {
                    std::cout << STR_ERROR_RS_FILE_READ_FAILED << config.rsSerialized << std::endl;
                }
            }
        }
        return ret;
    }

    bool rgDx12Frontend::ExtractRootSignature(const D3D12_SHADER_BYTECODE& bytecode,
        ID3D12RootSignature*& pRootSignature, std::string& errorMsg) const
    {
        bool ret = false;

        // Try to extract the root signature.
        ComPtr<ID3DBlob> pByteCodeRootSignature;
        HRESULT hr = D3DGetBlobPart(bytecode.pShaderBytecode, bytecode.BytecodeLength,
            D3D_BLOB_ROOT_SIGNATURE, 0, &pByteCodeRootSignature);

        if (SUCCEEDED(hr))
        {
            std::vector<uint8_t> rootSignature;
            rootSignature.resize(pByteCodeRootSignature->GetBufferSize());
            memcpy(rootSignature.data(), pByteCodeRootSignature->GetBufferPointer(),
                pByteCodeRootSignature->GetBufferSize());
            m_device.Get()->CreateRootSignature(0, rootSignature.data(),
                rootSignature.size(), IID_PPV_ARGS(&pRootSignature));
            assert(pRootSignature != nullptr);
            ret = (pRootSignature != nullptr);
        }
        else
        {
            errorMsg = STR_ERROR_ROOT_SIGNATURE_EXTRACTION_FAILURE;
        }

        return ret;
    }

    bool rga::rgDx12Frontend::GetHardwareAdapter(IDXGIAdapter1** ppAdapter)
    {
        bool ret = false;

        // Create the factory.
        UINT dxgiFactoryFlags = 0;
        ComPtr<IDXGIFactory4> pFactory;
        HRESULT hr = CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&pFactory));
        assert(SUCCEEDED(hr));
        if (SUCCEEDED(hr))
        {
            // Find the adapter.
            ComPtr<IDXGIAdapter1> adapter;
            *ppAdapter = nullptr;

            for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND !=
                pFactory->EnumAdapters1(adapterIndex, &adapter); ++adapterIndex)
            {
                DXGI_ADAPTER_DESC1 desc;
                adapter->GetDesc1(&desc);

                if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
                {
                    // Don't select the Basic Render Driver adapter.
                    continue;
                }

                if (SUCCEEDED(D3D12CreateDevice(adapter.Get(),
                    D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
                {
                    ret = true;
                    break;
                }
            }

            *ppAdapter = adapter.Detach();
        }

        return ret;
    }

    bool rgDx12Frontend::Init()
{
        bool ret = false;
        // Create the DXGI adapter.
        ComPtr<IDXGIAdapter1> pHwAdapter;
        ret = GetHardwareAdapter(&pHwAdapter);
        assert(ret);
        assert(pHwAdapter != nullptr);
        if (ret && pHwAdapter != nullptr)
        {
            // Create the D3D12 device.
            HRESULT hr = D3D12CreateDevice(pHwAdapter.Get(),
                D3D_FEATURE_LEVEL_11_0,
                IID_PPV_ARGS(&m_device));
            assert(SUCCEEDED(hr));
            if (SUCCEEDED(hr))
            {
                // Initialize the backend with the D3D12 device.
                ret = m_backend.Init(m_device.Get());
            }
        }
        else
        {
            std::cout << STR_ERROR_FAILED_TO_FIND_DX12_ADAPTER << std::endl;
        }
        return ret;
    }

    bool rgDx12Frontend::GetSupportedTargets(std::vector<std::string>& supportedTargets,
        std::map<std::string, unsigned>& targetToId) const
    {
        return m_backend.GetSupportedTargets(supportedTargets, targetToId);
    }

    bool rgDx12Frontend::CompileComputePipeline(const rgDx12Config& config, std::string& errorMsg) const
    {
        D3D12_COMPUTE_PIPELINE_STATE_DESC* pPso = nullptr;
        bool ret  = CreateComputePipeline(config, pPso, errorMsg);
        assert(ret);
        if (ret)
        {
            // Create the compute pipeline state.
            ID3D12PipelineState* pComputePso = nullptr;
            HRESULT hr = m_device->CreateComputePipelineState(pPso, IID_PPV_ARGS(&pComputePso));
            assert(SUCCEEDED(hr));
            ret = SUCCEEDED(hr);
            if (ret)
            {
                rgDx12ShaderResults results;
                rgDx12ThreadGroupSize threadGroupSize;
                ret = m_backend.CompileComputePipeline(pPso, results, threadGroupSize, errorMsg);
                assert(ret);
                if (ret)
                {
                    if (!config.comp.isa.empty())
                    {
                        // Save results to file: GCN ISA disassembly.
                        assert(results.pDisassembly != nullptr);
                        if (results.pDisassembly != nullptr)
                        {
                            // GCN ISA Disassembly.
                            std::cout << STR_INFO_EXTRACT_COMPUTE_SHADER_DISASSEMBLY << std::endl;
                            ret = rgDx12Utils::WriteTextFile(config.comp.isa, results.pDisassembly);

                            // Report the result to the user.
                            std::cout << (ret ? STR_INFO_EXTRACT_COMPUTE_SHADER_DISASSEMBLY_SUCCESS :
                                STR_ERROR_EXTRACT_COMPUTE_SHADER_DISASSEMBLY_FAILURE) << std::endl;
                        }
                        else
                        {
                            std::cout << STR_ERROR_COMPUTE_SHADER_DISASSEMBLY_EXCTRACTION_FAILURE << std::endl;
                        }
                        assert(ret);
                    }

                    if (!config.comp.stats.empty())
                    {
                        // Save results to file: statistics.
                        std::cout << STR_INFO_EXTRACT_COMPUTE_SHADER_STATS << std::endl;
                        bool isStatsSaved = SerializeDx12StatsCompute(results, threadGroupSize, config.comp.stats);
                        assert(isStatsSaved);
                        ret = ret && isStatsSaved;

                        // Report the result to the user.
                        std::cout << (isStatsSaved ? STR_INFO_EXTRACT_COMPUTE_SHADER_STATS_SUCCESS :
                            STR_ERROR_EXTRACT_COMPUTE_SHADER_STATS_FAILURE) << std::endl;
                    }
                    assert(ret);
                }
                else if (!errorMsg.empty())
                {
                    // Print the error messages to the user.
                    std::cout << errorMsg << std::endl;
                }
            }
        }
        return ret;
    }

    static bool WriteGraphicsShaderOutputFiles(const rgDx12ShaderConfig &shaderConfig,
        const rgDx12ShaderResults& shaderResults, const std::string& stageName)
    {
        bool ret = true;
        if (!shaderConfig.isa.empty())
        {
            // Save results to file: GCN ISA disassembly.
            assert(shaderResults.pDisassembly != nullptr);
            if (shaderResults.pDisassembly != nullptr)
            {
                // GCN ISA Disassembly.
                std::cout << STR_INFO_EXTRACT_GRAPHICS_SHADER_OUTPUT_A << stageName <<
                    STR_INFO_EXTRACT_GRAPHICS_SHADER_DISASSEMBLY << std::endl;
                ret = rgDx12Utils::WriteTextFile(shaderConfig.isa, shaderResults.pDisassembly);
                assert(ret);

                // Report the result to the user.
                if (ret)
                {
                    std::cout << stageName << STR_INFO_EXTRACT_GRAPHICS_SHADER_DISASSEMBLY_SUCCESS << std::endl;
                }
                else
                {
                    std::cout << STR_ERROR_EXTRACT_GRAPHICS_SHADER_OUTPUT_FAILURE_A << stageName <<
                        STR_ERROR_EXTRACT_GRAPHICS_SHADER_DISASSEMBLY_FAILURE_B << std::endl;
                }
            }
            else
            {
                std::cout << STR_ERROR_GRAPHICS_SHADER_DISASSEMBLY_EXCTRACTION_FAILURE_A << stageName <<
                    STR_ERROR_GRAPHICS_SHADER_DISASSEMBLY_EXCTRACTION_FAILURE_B << std::endl;
            }
        }

        if (!shaderConfig.stats.empty())
        {
            // Save results to file: statistics.
            std::cout << STR_INFO_EXTRACT_GRAPHICS_SHADER_OUTPUT_A << stageName <<
                STR_INFO_EXTRACT_GRAPHICS_SHADER_STATS_B << std::endl;
            bool isStatsSaved = SerializeDx12StatsGraphics(shaderResults, shaderConfig.stats);
            assert(isStatsSaved);
            ret = ret && isStatsSaved;

            // Report the result to the user.
            if (isStatsSaved)
            {
                std::cout << stageName << STR_INFO_EXTRACT_GRAPHICS_SHADER_STATS_SUCCESS << std::endl;
            }
            else
            {
                std::cout << STR_ERROR_EXTRACT_GRAPHICS_SHADER_OUTPUT_FAILURE_A << stageName <<
                    STR_ERROR_EXTRACT_GRAPHICS_SHADER_STATS_FAILURE_B << std::endl;
            }
        }

        return ret;
    }

    bool rgDx12Frontend::CompileGraphicsPipeline(const rgDx12Config& config, std::string& errorMsg) const
    {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC* pPso = nullptr;
        rgDx12PipelineResults results;

        bool ret = CreateGraphicsPipeline(config, pPso, errorMsg);
        assert(ret);
        if (ret)
        {
            // Create the graphics pipeline state.
            ID3D12PipelineState* pGraphicsPso = nullptr;

            // For graphics, we must use a .gpso file.
            bool isPsoFileParsed = false;
            if (!config.rsPso.empty())
            {
                // Load the .gpso file that the user provided, and extract the
                // relevant pipeline state.
                isPsoFileParsed = rgDx12Utils::ParseGpsoFile(config.rsPso, *pPso);
                assert(isPsoFileParsed);
            }
            else
            {
                // Use a default pso.
                assert(false);
                std::cout << "Warning: no pipeline state file received. Use the --gpso switch to provide a graphics pipeline description." << std::endl;
            }

            if (isPsoFileParsed)
            {
                // Check if the user provided a serialized root signature file.
                bool isSerializedRootSignature = !config.rsSerialized.empty();

                // Read the root signature from the file and recreate it.
                if (isSerializedRootSignature)
                {
                    std::vector<char> rootSignatureBuffer;
                    ret = rgDx12Utils::ReadBinaryFile(config.rsSerialized, rootSignatureBuffer);
                    assert(ret);
                    if (ret)
                    {
                        m_device.Get()->CreateRootSignature(0, rootSignatureBuffer.data(),
                            rootSignatureBuffer.size(), IID_PPV_ARGS(&pPso->pRootSignature));
                        assert(pPso->pRootSignature != nullptr);
                        ret = pPso->pRootSignature != nullptr;
                    }
                }
                else
                {
                    // If the root signature was already created, use it. Otherwise,
                    // try to extract it from the DXBC binary after compiling the HLSL code.
                    bool shouldExtractRootSignature = !config.rsMacro.empty();

                    // If the input is HLSL, we need to compile the root signature out of the HLSL file.
                    // Otherwise, if the input is a DXBC binary and it has a root signature baked into it,
                    // then the root signature would be automatically fetched from the binary, there is no
                    // need to set it into the PSO's root signature field.
                    ComPtr<ID3DBlob> compiledRs;
                    ret = CompileRootSignature(config, compiledRs);
                    if (ret)
                    {
                        // Create the root signature through the device and assign it to our PSO.
                        std::vector<uint8_t> rootSignatureBuffer;
                        rootSignatureBuffer.resize(compiledRs->GetBufferSize());
                        memcpy(rootSignatureBuffer.data(), compiledRs->GetBufferPointer(),
                            compiledRs->GetBufferSize());
                        m_device.Get()->CreateRootSignature(0, rootSignatureBuffer.data(),
                            rootSignatureBuffer.size(), IID_PPV_ARGS(&pPso->pRootSignature));
                        assert(pPso->pRootSignature != nullptr);
                        ret = pPso->pRootSignature != nullptr;
                    }
                }

                // Create the graphics pipeline.
                HRESULT hr = m_device->CreateGraphicsPipelineState(pPso, IID_PPV_ARGS(&pGraphicsPso));
                assert(SUCCEEDED(hr));
                ret = SUCCEEDED(hr);

                if (ret)
                {
                    ret = m_backend.CompileGraphicsPipeline(pPso, results, errorMsg);
                    assert(ret);

                    if (ret)
                    {
                        // Write the output files (for applicable stages).
                        ret = WriteGraphicsShaderOutputFiles(config.vert, results.m_vertex, "vertex");
                        assert(ret);
                        ret = WriteGraphicsShaderOutputFiles(config.hull, results.m_hull, "hull");
                        assert(ret);
                        ret = WriteGraphicsShaderOutputFiles(config.domain, results.m_domain, "domain");
                        assert(ret);
                        ret = WriteGraphicsShaderOutputFiles(config.geom, results.m_geometry, "geometry");
                        assert(ret);
                        ret = WriteGraphicsShaderOutputFiles(config.pixel, results.m_pixel, "pixel");
                        assert(ret);
                    }
                }
                else
                {
                    std::stringstream msg;
                    if (errorMsg.empty())
                    {
                        msg << std::endl;
                    }
                    msg << STR_ERROR_FAILED_TO_CREATE_GRAPHICS_PIPELINE << std::endl;
                    errorMsg.append(msg.str());
                }
            }
            else
            {
                std::cout << STR_ERROR_GPSO_FILE_PARSE_FAILED << std::endl;
            }
        }
        return ret;
    }
}
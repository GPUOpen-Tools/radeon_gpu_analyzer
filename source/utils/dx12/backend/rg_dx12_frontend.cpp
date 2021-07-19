// D3D12.
#include <d3d12.h>
#include <d3dcompiler.h>
#include <d3dcommon.h>
#include <wrl.h>
#include "../backend/d3dx12.h"

// C++.
#include <cassert>
#include <iostream>
#include <sstream>
#include <locale>
#include <codecvt>
#include <vector>
#include <iomanip>
#include <algorithm>

// Local.
#include "rg_dx12_frontend.h"
#include "rg_dx12_utils.h"
#include "rg_dx12_factory.h"
#include "rg_dxr_state_desc_reader.h"
#include "rg_dxr_output_metadata.h"

// Backend.
#include "be_d3d_include_manager.h"

namespace rga
{
    // *** CONSTANTS - START ***

    // Errors.
    static const char* kStrErrorRootSignatureExtractionFailure = "Error: failed to extract root signature from DXCB binary.";
    static const char* kStrErrorLocalRootSignatureCreateFromFileFailure = "Error: failed to create local root signature from file: ";
    static const char* kStrErrorGlobalRootSignatureCreateFromFileFailure = "Error: failed to create global root signature from file: ";
    static const char* kStrErrorShaderCompilationFailure = "Error: failed to compile shader: ";
    static const char* kStrErrorComputeShaderDisassemblyExtractionFailure = "Error: failed to extract GCN ISA disassembly for compute shader.";
    static const char* kStrErrorGraphicsShaderDisassemblyExtractionFailure1 = "Error: failed to extract GCN ISA disassembly for ";
    static const char* kStrErrorGraphicsShaderDisassemblyExtractionFailure2 = " shader.";
    static const char* kStrErrorRootSignatureCompileFailure = "Error: failed to compile root signature.";
    static const char* kStrErrorDxbcDisassembleFailure = "Error: failed to disassemble compute shader DXBC binary.";
    static const char* kStrErrorExtractComputeShaderStatsFailure = "Error: failed to extract compute shader statistics.";
    static const char* kStrErrorExtractComputeShaderDisassemblyFailure = "Error: failed to extract compute shader disassembly.";
    static const char* kStrErrorExtractComputePipelineBinaryFailure = "Error: failed to extract compute pipeline binary.";
    static const char* kStrErrorExtractGraphicsShaderStatsFailure = "Error: failed to extract graphics pipeline binary.";
    static const char* kStrErrorExtractGraphicsShaderOutputFailure1 = "Error: failed to extract ";
    static const char* kStrErrorExtractGraphicsShaderStatsFailure2 = " shader statistics.";
    static const char* kStrErrorExtractGraphicsShaderDisassemblyFailure2 = " shader disassembly.";
    static const char* kStrErrorFrontEndCompilationFailure = "Error: front-end compilation of hlsl to DXBC failed.";
    static const char* kStrErrorFailedToFindDx12Adapter = "Error: failed to find a DX12 display adapter.";
    static const char* kStrErrorGraphicsPipelineCreationFailure = "Error: graphics pipeline creation failed.";
    static const char* kStrErrorGpsoFileParseFailed = "Error: failed to parse graphics pipeline description file.";
    static const char* kStrErrorRsFileReadFailed = "Error: failed to read binary file for root signature: ";
    static const char* kStrErrorRsFileCompileFailed = "Error: failed to compile root signature after reading binary data from file: ";
    static const char* kStrErrorDxilFileReadFailed = "Error: failed to read binary DXIL library: ";
    static const char* kStrErrorFailedToCreateDxrInterface = "Error: failed to initialize DXR interface.";
    static const char* kStrErrorNoPipelineBinaryGeneratedForIndex = "Error: no pipeline binary generated for pipeline #";
    static const char* kStrErrorDxrInputFileNotFound = "Error: DXR input file not found: ";
    static const char* kStrErrorDxrLocalRootSignatureFileNotFound = "Error: local root signature file not found: ";
    static const char* kStrErrorDxrGlobalRootSignatureFileNotFound = "Error: global root signature file not found: ";
    static const char* kStrErrorDxrRootSignatureFailureHlsl1 = "Error: no root signature detected in HLSL code. If your root signature is defined in the HLSL code, make sure that "
        "the [RootSignature()] attribute is used for";
    static const char* kStrErrorDxrRootSignatureFailureHlsl2Graphics = " one of the pipeline's shaders ";
    static const char* kStrErrorDxrRootSignatureFailureHlsl2Compute = " the compute shader ";
    static const char* kStrErrorDxrRootSignatureFailureHlsl3 = "with the macro name as the argument, or use the --rs-macro option. If your root signature is precompiled into a binary, please use "
        "the --rs-bin option with the full path to the binary file as an argument. For more information about root signatures in HLSL, "
        "see https://docs.microsoft.com/en-us/windows/win32/direct3d12/specifying-root-signatures-in-hlsl#compiling-an-hlsl-root-signature";
    static const char* kStrErrorFailedToWriteOutputFile1 = "Error: failed to write ";
    static const char* kStrErrorFailedToWriteOutputFile2 = " file to ";
    static const char* kStrErrorFailedToWriteOutputFile3 = "make sure that the path is valid.";
    static const char* kStrErrorFailedToExtractRaytracingDisassembly = "Error: failed to extract disassembly.";
    static const char* kStrErrorFailedToExtractRaytracingStatistics = "Error: failed to extract hardware resource usage statistics.";
    static const char* kStrErrorFailedToExtractRaytracingBinary = "Error: failed to extract pipeline binary.";
    static const char* kStrErrorFailedToCreateComputePipeline = "Error: compute pipeline state creation failed. ";
    static const char* kStrErrorDxrShaderModeCompilationFailed = "Error: DXR shader mode compilation failed.";
    static const char* kStrErrorDxrFailedToReadHlslToDxilMappingFile1 = "Error: failed to read ";
    static const char* kStrErrorDxrFailedToParseHlslToDxilMappingFile1 = "Error: failed to parse ";
    static const char* kStrErrorDxrdHlslToDxilMappingFile2 = "HLSL->DXIL mapping file.";
    static const char* kStrErrorDxrFailedToRetrievePipelineShaderName = "Error: failed to retrieve shader name for shader #";
    static const char* kStrErrorAmdDisplayAdapterNotFound              = "Error: could not find an AMD display adapter on the system.";

    // Warnings.
    static const char* kStrWarningBinaryExtractionNotSupportedInIndirectMode = "Warning: pipeline binary extraction (-b option) is not supported when driver performs Indirect compilation.";
    static const char* kStrWarningBinaryExtractionNotSupportedMultiplePipelines1 = "Warning: pipeline binary extraction skipped for pipeline #";
    static const char* kStrWarningBinaryExtractionNotSupportedMultiplePipelines2 = " - there is currently no support for pipeline binary extraction when multiple pipelines are generated.";

    // Info.
    static const char* kStrInfoExtractComputeShaderStats = "Extracting compute shader statistics...";
    static const char* kStrInfoExtractComputeShaderDisassembly = "Extracting compute shader disassembly...";
    static const char* kStrInfoExtractGraphicsShaderDisassembly = " shader disassembly...";
    static const char* kStrInfoExtractComputePipelineBinary = "Extracting compute pipeline binary...";
    static const char* kStrInfoExtractGraphicsPipelineBinary = "Extracting graphics pipeline binary...";
    static const char* kStrInfoExtractGraphicsShaderOutput1 = "Extracting ";
    static const char* kStrInfoExtractGraphicsShaderStats2 = " shader statistics...";
    static const char* kStrInfoExtractComputeShaderStatsSuccess = "Compute shader statistics extracted successfully.";
    static const char* kStrInfoExtractComputeShaderDisassemblySuccess = "Compute shader disassembly extracted successfully.";
    static const char* kStrInfoExtractComputePipelineBinarySuccess = "Compute pipeline binary extracted successfully.";
    static const char* kStrInfoExtractGraphicsPipelineBinarySuccess = "Graphics pipeline binary extracted successfully.";
    static const char* kStrInfoExtractGraphicsShaderStatsSuccess = " shader statistics extracted successfully.";
    static const char* kStrInfoExtractGraphicsShaderDisassemblySuccess = " shader disassembly extracted successfully.";
    static const char* kStrInfoExtractRayTracingDisassemblySuccess = "Disassembly extracted successfully.";
    static const char* kStrInfoExtractRayTracingStatisticsSuccess = "Hardware resource usage statistics extracted successfully.";
    static const char* kStrInfoExtractRayTracingBinarySuccess = "Pipeline binary extracted successfully.";
    static const char* kStrInfoExtractRayTracingStatsShader = "Extracting statistics for shader ";
    static const char* kStrInfoExtractRayTracingPipelineBinaryByRaygen = "Extracting pipeline binary for pipeline associated with raygeneration shader ";
    static const char* kStrInfoExtractRayTracingDisassemblyPipelineByRaygen = "Extracting disassembly for pipeline associated with raygeneration shader ";
    static const char* kStrInfoExtractRayTracingResourceUsagePipelineByRaygen = "Extracting hardware resource usage for pipeline associated with raygeneration shader ";
    static const char* kStrInfoExtractRayTracingPipelineBinaryByIndex1 = "Extracting pipeline binary for pipeline #";
    static const char* kStrInfoExtractRayTracingDisassemblyPipelineByIndex1 = "Extracting disassembly for pipeline #";
    static const char* kStrInfoExtractRayTracingResourceUsagePipelineByIndex1 = "Extracting hardware resource usage for pipeline #";
    static const char* kStrInfoExtractRayTracingResultByIndex2 = " for shader ";
    static const char* kStrInfoExtractRayTracingDisassemblyShader = "Extracting disassembly for shader ";
    static const char* kStrInfoCompilingRootSignatureFromHlsl1 = "Compiling root signature defined in HLSL file ";
    static const char* kStrInfoCompilingRootSignatureFromHlsl2 = " in macro named ";
    static const char* kStrHintRootSignatureFailure = "Hint: this failure could be due to a missing or incompatible root signature.\n"
        "1. If your root signature is defined in the HLSL code, make sure that the [RootSignature()] attribute is used for one of the pipeline shaders "
        "with the macro name, or use the --rs-macro option.\n2. If your root signature is precompiled into a binary, please use the --rs-bin "
        "option with the full path to the binary file as an argument.";
    static const char* kStrInfoDxrPipelineCompiledUnified = "Pipeline compiled in Unified mode, expect a single uber shader in the output.";
    static const char* kStrInfoDxrPipelineCompiledIndirect1 = "Pipeline compiled in Indirect mode, expect ";
    static const char* kStrInfoDxrPipelineCompiledIndirect2 = " shaders in the output.";
    static const char* kStrInfoDxrPipelineCompilationGenerated1 = "Compilation generated ";
    static const char* kStrInfoDxrPipelineCompilationGeneratedMultiplePipelines2 = " pipelines.";
    static const char* kStrInfoDxrPipelineCompilationGeneratedSinglePipeline2 = " pipeline.";
    static const char* kStrInfoDxrPipelineTypeReport1 = "Pipeline #";
    static const char* kStrInfoDxrPipelineTypeReport2 = " associated with raygeneration shader ";

    // *** CONSTANTS - END ***

    // *** STATICALLY-LINKED UTITILIES - START ***

    static void Dx12StatsToString(const RgDx12ShaderResults& stats, std::stringstream& serialized_stats)
    {
        serialized_stats << "Statistics:" << std::endl;
        serialized_stats << "    - shaderStageMask                           = " << stats.shader_mask << std::endl;
        serialized_stats << "    - resourceUsage.numUsedVgprs                = " << stats.vgpr_used << std::endl;
        serialized_stats << "    - resourceUsage.numUsedSgprs                = " << stats.sgpr_used << std::endl;
        serialized_stats << "    - resourceUsage.ldsSizePerLocalWorkGroup    = " << stats.lds_available_bytes << std::endl;
        serialized_stats << "    - resourceUsage.ldsUsageSizeInBytes         = " << stats.lds_used_bytes << std::endl;
        serialized_stats << "    - resourceUsage.scratchMemUsageInBytes      = " << stats.scratch_used_bytes << std::endl;
        serialized_stats << "    - numPhysicalVgprs                          = " << stats.vgpr_physical << std::endl;
        serialized_stats << "    - numPhysicalSgprs                          = " << stats.sgpr_physical << std::endl;
        serialized_stats << "    - numAvailableVgprs                         = " << stats.vgpr_available << std::endl;
        serialized_stats << "    - numAvailableSgprs                         = " << stats.sgpr_available << std::endl;
    }

    static void Dx12StatsToStringRayTracing(const RgDx12ShaderResultsRayTracing& stats, std::stringstream& serialized_stats)
    {
        // Serialize the common part.
        Dx12StatsToString(stats, serialized_stats);

#ifdef _DXR_STATS_ENABLED
        // These are currently not supported by the driver.
        // Add DXR-specific part.
        serialized_stats << "    - stackSizeBytes                            = " << stats.stack_size_bytes << std::endl;
        serialized_stats << "    - isInlined                                 = " << (stats.is_inlined ? "yes" : "no") << std::endl;
#endif
    }

    // Serialize a graphics shader's statistics.
    static bool SerializeDx12StatsGraphics(const RgDx12ShaderResults& stats, const std::string& output_filename)
    {
        // Convert the statistics to string.
        std::stringstream serialized_stats;
        Dx12StatsToString(stats, serialized_stats);

        // Write the results to the output file.
        bool ret = RgDx12Utils::WriteTextFile(output_filename, serialized_stats.str());
        assert(ret);
        return ret;
    }

    // Serialize a compute shader's statistics including the thread group dimensions.
    static bool SerializeDx12StatsCompute(const RgDx12ShaderResults& stats, const RgDx12ThreadGroupSize& thread_group_size,
        const std::string& output_filename)
    {
        // Serialize the common shader stats.
        std::stringstream serialized_stats;
        Dx12StatsToString(stats, serialized_stats);

        // Serialize and append the compute thread group size.
        serialized_stats << "    - computeWorkGroupSizeX" << " = " << thread_group_size.x << std::endl;
        serialized_stats << "    - computeWorkGroupSizeY" << " = " << thread_group_size.y << std::endl;
        serialized_stats << "    - computeWorkGroupSizeZ" << " = " << thread_group_size.z << std::endl;

        // Write the results to the output file.
        bool ret = RgDx12Utils::WriteTextFile(output_filename, serialized_stats.str());
        assert(ret);
        return ret;
    }

    // Serialize a raytracing pipeline statistics.
    static bool SerializeDx12StatsRayTracing(const RgDx12ShaderResultsRayTracing& stats,
        const std::string& output_filename)
    {
        // Serialize.
        std::stringstream serialized_stats;
        Dx12StatsToStringRayTracing(stats, serialized_stats);

        // Write the results to the output file.
        bool ret = RgDx12Utils::WriteTextFile(output_filename, serialized_stats.str());
        assert(ret);
        return ret;
    }

    static bool CreateDefaultRootSignature(ID3D12Device* d3d12_device, ID3D12RootSignature*& d3d_root_signature)
    {
        bool ret = false;
        assert(d3d12_device != nullptr);
        if (d3d12_device != nullptr)
        {
            D3D12_ROOT_SIGNATURE_DESC root_signature_desc;
            memset(&root_signature_desc, 0, sizeof(root_signature_desc));
            root_signature_desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;
            ComPtr<ID3DBlob> signature;
            ComPtr<ID3DBlob> error;
            D3D12SerializeRootSignature(&root_signature_desc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
            d3d12_device->CreateRootSignature(0, signature->GetBufferPointer(),
                signature->GetBufferSize(), IID_PPV_ARGS(&d3d_root_signature));
            assert(d3d_root_signature != nullptr);
            ret = (d3d_root_signature != nullptr);
        }
        return ret;
    }

    static std::string ExtractFileDirectory(const std::string& full_path)
    {
        std::string ret;
        size_t pos = full_path.rfind('\\');
        if (pos != std::string::npos)
        {
            ret = full_path.substr(0, pos);
        }
        return ret;
    }

    static bool CompileHlslShader(const RgDx12Config& config, const std::string& hlsl_full_path,
        const std::string& entry_point, const std::string& shader_model,
        D3D12_SHADER_BYTECODE& bytecode, std::string& error_msg)
    {
        bool ret = false;

        // Convert the shader file name to wide characters.
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        std::wstring hlsl_full_path_wide = converter.from_bytes(hlsl_full_path);

        // Extract the hlsl file's folder and instantiate the include manager.
        std::string hlsl_file_folder = ExtractFileDirectory(hlsl_full_path);
        D3dIncludeManager include_manager(hlsl_file_folder.c_str(), config.include_dirs);

        // Preprocessor definitions.
        D3D_SHADER_MACRO* macros = new D3D_SHADER_MACRO[config.defines.size() + 1]{};

        // Handle the defines.
        if (!config.defines.empty())
        {
            int i = 0;
            for (const auto& preprocessor_define : config.defines)
            {
                std::vector<std::string> macro_parts;
                RgDx12Utils::SplitString(preprocessor_define, '=', macro_parts);
                assert(macro_parts.size() > 0 && macro_parts.size() < 3);
                if (macro_parts.size() > 0 && macro_parts.size() < 3)
                {
                    macros[i].Name = new char[macro_parts[0].size() + 1]{};
                    memcpy((void*)macros[i].Name, macro_parts[0].data(), macro_parts[0].size());
                    if (macro_parts.size() > 1)
                    {
                        // This is a X=Y macro.
                        macros[i].Definition = new char[macro_parts[1].size() + 1]{};
                        memcpy((void*)macros[i].Definition, macro_parts[1].data(), macro_parts[1].size());
                    }
                    else
                    {
                        // This is just a definition.
                        macros[i].Definition = "";
                    }
                    i++;
                }
            }
        }

        // Front-end compilation.
        ComPtr<ID3DBlob> compiled_blob;
        ComPtr<ID3DBlob> error_messages;
        UINT compile_flags = 0;
        HRESULT hr = D3DCompileFromFile(hlsl_full_path_wide.c_str(), macros, &include_manager,
            entry_point.c_str(), shader_model.c_str(),
            compile_flags, 0, &compiled_blob, &error_messages);

        assert(hr == S_OK);
        if (hr == S_OK)
        {
            // Extract the byte code.
            bytecode.BytecodeLength = compiled_blob->GetBufferSize();
            char* buffer = new char[bytecode.BytecodeLength]();
            memcpy(buffer, compiled_blob->GetBufferPointer(), bytecode.BytecodeLength);
            bytecode.pShaderBytecode = buffer;
            ret = true;
        }
        else
        {
            if (error_messages != nullptr && error_messages->GetBufferSize() > 0)
            {
                std::cerr << (char*)error_messages->GetBufferPointer() << std::endl;;
            }
            std::cerr << kStrErrorFrontEndCompilationFailure << std::endl;
        }

        // Clean up.
        for (size_t i = 1; i < config.defines.size(); i++)
        {
            delete[] macros[i].Name;
            macros[i].Name = nullptr;
            delete[] macros[i].Definition;
            macros[i].Definition = nullptr;
        }
        delete[] macros;
        macros = nullptr;

        return ret;
    }

    static bool ReadDxbcBinary(const std::string& dxbc_full_path, D3D12_SHADER_BYTECODE& bytecode)
    {
        bool ret = false;

        // Read the compiled DXBC from a file.
        std::vector<char> compiled_shader;
        bool is_dxbc_binary_read = RgDx12Utils::ReadBinaryFile(dxbc_full_path, compiled_shader);
        assert(is_dxbc_binary_read);
        if (is_dxbc_binary_read)
        {
            char* buffer = new char[compiled_shader.size()]();
            memcpy(buffer, compiled_shader.data(), compiled_shader.size());
            bytecode.pShaderBytecode = buffer;
            bytecode.BytecodeLength = compiled_shader.size();
            ret = true;
        }
        else
        {
            static const char* kStrErrorDxbcReadFailure = "Error: failed to read input DXBC binary: ";
            std::cerr << kStrErrorDxbcReadFailure << dxbc_full_path << std::endl;
        }

        return ret;
    }

    static bool WriteDxbcFile(const D3D12_SHADER_BYTECODE& bytecode, const std::string& output_file)
    {
        char* buffer = (char*)bytecode.pShaderBytecode;
        std::vector<char> comp_bytecode(buffer, buffer + bytecode.BytecodeLength);
        bool is_file_written = RgDx12Utils::WriteBinaryFile(output_file, comp_bytecode);
        assert(is_file_written);
        return is_file_written;
    }

    static bool DisassembleDxbc(const D3D12_SHADER_BYTECODE& bytecode, const std::string& output_file)
    {
        bool ret = false;

        ID3DBlob* disassembly = nullptr;
        HRESULT hr = D3DDisassemble(bytecode.pShaderBytecode, bytecode.BytecodeLength, 0, "", &disassembly);
        assert(hr == S_OK);
        assert(disassembly != nullptr);
        assert(disassembly->GetBufferSize() > 0);
        if (SUCCEEDED(hr) && disassembly != nullptr && disassembly->GetBufferSize() > 0)
        {
            std::string dxbc_disassembly((char*)disassembly->GetBufferPointer(),
                (char*)disassembly->GetBufferPointer() + disassembly->GetBufferSize());
            ret = RgDx12Utils::WriteTextFile(output_file, dxbc_disassembly);
            assert(ret);
        }
        else
        {
            std::cerr << kStrErrorDxbcDisassembleFailure << std::endl;
        }

        return ret;
    }

    static bool CompileRootSignature(const RgDx12Config& config, ComPtr<ID3DBlob>& compiled_rs)
    {
        bool ret = false;

        if (!config.rs_macro_file.empty() && !config.rs_macro.empty())
        {
            std::cout << kStrInfoCompilingRootSignatureFromHlsl1 <<
                config.rs_macro_file << kStrInfoCompilingRootSignatureFromHlsl2 <<
                config.rs_macro << "..." << std::endl;

            // Read the content of the HLSL file.
            std::string hlsl_text;
            bool is_file_read = RgDx12Utils::ReadTextFile(config.rs_macro_file, hlsl_text);
            assert(is_file_read);
            assert(!hlsl_text.empty());
            if (is_file_read)
            {
                // Extract the HLSL file's folder and instantiate the include manager.
                std::string hlslFileFolder = ExtractFileDirectory(config.rs_macro_file);
                D3dIncludeManager includeManager(hlslFileFolder.c_str(), config.include_dirs);

                // Root signature compilation.
                ComPtr<ID3DBlob> error_messages;
                HRESULT hr = D3DCompile(hlsl_text.c_str(), hlsl_text.size(), nullptr,
                    nullptr, &includeManager,
                    config.rs_macro.c_str(), config.rs_version.c_str(), 0, 0, &compiled_rs, &error_messages);

                if (error_messages != nullptr)
                {
                    std::cerr << (char*)error_messages->GetBufferPointer() << std::endl;
                }

                assert(hr == S_OK);
                assert(compiled_rs != nullptr);
                ret = SUCCEEDED(hr) && compiled_rs != nullptr;
                if (!ret)
                {
                    std::cerr << kStrErrorRootSignatureCompileFailure << std::endl;
                }
            }
        }

        return ret;
    }

    static bool CreateRootSignatureFromBinary(ID3D12Device* device, const std::string& full_path, ID3D12RootSignature*& root_signature)
    {
        // Read the root signature from the file and recreate it.
        std::vector<char> rs_data;
        std::vector<char> root_signature_buffer;
        bool ret = RgDx12Utils::ReadBinaryFile(full_path, root_signature_buffer);
        assert(ret);
        if (ret)
        {
            device->CreateRootSignature(0, root_signature_buffer.data(),
                root_signature_buffer.size(), IID_PPV_ARGS(&root_signature));
            assert(root_signature != nullptr);
            ret = (root_signature != nullptr);
        }
        return ret;
    }

    // Generates a new name by replacing the token with the given specific name.
    static std::string GenerateSpecificFileName(const std::string& basic_filename, const std::string& specific_filename)
    {
        const char kFilenameToken = '*';
        std::string fixed_str;
        std::vector<std::string> split_string;
        RgDx12Utils::SplitString(basic_filename, kFilenameToken, split_string);
        assert(split_string.size() == 2);
        if (split_string.size() == 2)
        {
            std::stringstream fixed_stream;
            fixed_stream << split_string[0].c_str() << specific_filename.c_str() << split_string[1].c_str();
            fixed_str = fixed_stream.str();
        }
        return fixed_str;
    }

    static bool IsDxrInputValid(const RgDxrStateDesc& state_desc)
    {
        bool ret = true;

        // Input files.
        for (const std::shared_ptr<RgDxrDxilLibrary>& local_rs : state_desc.input_files)
        {
            if (!RgDx12Utils::IsFileExists(local_rs->full_path))
            {
                std::cout << kStrErrorDxrInputFileNotFound << local_rs->full_path << std::endl;
                ret = false;
                break;
            }
        }

        if (ret)
        {
            // Local root signatures.
            for (const std::shared_ptr<RgDxrRootSignature>& local_rs : state_desc.local_root_signature)
            {
                if (!RgDx12Utils::IsFileExists(local_rs->full_path))
                {
                    std::cout << kStrErrorDxrLocalRootSignatureFileNotFound << local_rs->full_path << std::endl;
                    ret = false;
                    break;
                }
            }

            if (ret)
            {
                // Global root signatures.
                for (const std::shared_ptr<RgDxrRootSignature>& global_rs : state_desc.global_root_signature)
                {
                    if (!RgDx12Utils::IsFileExists(global_rs->full_path))
                    {
                        std::cout << kStrErrorDxrGlobalRootSignatureFileNotFound << global_rs->full_path << std::endl;
                        ret = false;
                        break;
                    }
                    else
                    {
                        std::cout << "Root signature compiled successfully." << std::endl;
                    }
                }
            }
        }

        return ret;
    }

    static void ReadHlslMappingFile(const std::string& dxr_hlsl_mapping, std::map<std::string, std::string>& hlsl_dxil_mapping)
    {
        std::string mapping_content;
        bool is_hlsl_mapping_file_read = RgDx12Utils::ReadTextFile(dxr_hlsl_mapping, mapping_content);
        assert(is_hlsl_mapping_file_read);
        if (is_hlsl_mapping_file_read)
        {
            try
            {
                // Filter out newlines.
                mapping_content.erase(std::remove(mapping_content.begin(), mapping_content.end(), '\n'), mapping_content.end());

                // Extract the HLSL->DXIL mapping.
                std::vector <std::string> mapping_components;
                RgDx12Utils::SplitString(mapping_content, '$', mapping_components);
                assert(!mapping_components.empty());
                assert(mapping_components.size() % 2 == 0);

                for (uint32_t i = 0; i < mapping_components.size() - 1; i += 2)
                {
                    hlsl_dxil_mapping[mapping_components[i]] = mapping_components[i + 1];
                }
            }
            catch (const std::exception&)
            {
                std::cerr << kStrErrorDxrFailedToParseHlslToDxilMappingFile1 <<
                    kStrErrorDxrdHlslToDxilMappingFile2 << std::endl;
            }
        }
        else
        {
            std::cerr << kStrErrorDxrFailedToReadHlslToDxilMappingFile1 <<
                kStrErrorDxrdHlslToDxilMappingFile2 << std::endl;
        }
    }

    // *** STATICALLY-LINKED UTITILIES - END ***

    bool rgDx12Frontend::CreateComputePipeline(const RgDx12Config& config,
        D3D12_COMPUTE_PIPELINE_STATE_DESC*& compute_pso,
        std::string& error_msg) const
    {
        bool ret = false;
        compute_pso = new D3D12_COMPUTE_PIPELINE_STATE_DESC();

        // If the input is DXBC binary, we do not need front-end compilation.
        bool is_front_end_compilation_required = !config.comp.hlsl.empty();

        // If the root signature was already created, use it. Otherwise,
        // try to extract it from the DXBC binary after compiling the HLSL code.
        bool should_extract_root_signature = !config.rs_macro.empty();

        // Check if the user provided a serialized root signature file.
        bool is_serialized_root_signature = !config.rs_serialized.empty();

        // Buffer to hold DXBC compiled shader.
        D3D12_SHADER_BYTECODE bytecode;
        HRESULT hr = E_FAIL;

        if (is_front_end_compilation_required)
        {
            // Compile the HLSL file.
            ret = CompileHlslShader(config, config.comp.hlsl, config.comp.entry_point,
                config.comp.shader_model, bytecode, error_msg);
            assert(ret);
            if (ret)
            {
                compute_pso->CS = bytecode;
            }
            else
            {
                error_msg = kStrErrorShaderCompilationFailure;
                error_msg.append("compute\n");
            }
        }
        else
        {
            // Read the compiled HLSL binary.
            ret = ReadDxbcBinary(config.comp.dxbc, bytecode);
            assert(ret);
            if (ret)
            {
                compute_pso->CS = bytecode;
            }
        }

        if (!config.comp.dxbcOut.empty())
        {
            // If the user wants to dump the bytecode as a binary, do it here.
            bool is_dxbc_written = WriteDxbcFile(bytecode, config.comp.dxbcOut);
            assert(is_dxbc_written);
        }

        if (ret)
        {
            if (!config.comp.dxbc_disassembly.empty())
            {
                // If the user wants to dump the bytecode disassembly, do it here.
                bool is_dxbc_disassembly_generated = DisassembleDxbc(bytecode, config.comp.dxbc_disassembly);
                assert(is_dxbc_disassembly_generated);
            }

            if (should_extract_root_signature)
            {
                // If the input is HLSL, we need to compile the root signature out of the HLSL file.
                // Otherwise, if the input is a DXBC binary and it has a root signature baked into it,
                // then the root signature would be automatically fetched from the binary, there is no
                // need to set it into the PSO's root signature field.
                ComPtr<ID3DBlob> compiled_rs;
                ret = CompileRootSignature(config, compiled_rs);
                if (ret)
                {
                    // Create the root signature through the device and assign it to our PSO.
                    std::vector<uint8_t> root_signature_buffer;
                    root_signature_buffer.resize(compiled_rs->GetBufferSize());
                    memcpy(root_signature_buffer.data(), compiled_rs->GetBufferPointer(),
                        compiled_rs->GetBufferSize());
                    device_.Get()->CreateRootSignature(0, root_signature_buffer.data(),
                        root_signature_buffer.size(), IID_PPV_ARGS(&compute_pso->pRootSignature));
                    assert(compute_pso->pRootSignature != nullptr);
                    ret = compute_pso->pRootSignature != nullptr;
                }
            }
            else if (is_serialized_root_signature)
            {
                // Read the root signature from the file and recreate it.
                std::vector<char> root_signature_buffer;
                ret = RgDx12Utils::ReadBinaryFile(config.rs_serialized, root_signature_buffer);
                assert(ret);
                if (ret)
                {
                    device_.Get()->CreateRootSignature(0, root_signature_buffer.data(),
                        root_signature_buffer.size(), IID_PPV_ARGS(&compute_pso->pRootSignature));
                    assert(compute_pso->pRootSignature != nullptr);
                    ret = compute_pso->pRootSignature != nullptr;
                }
            }
            else
            {
                // If we got here, it means that the user did not explicitly provide a root
                // signature (using --rs-bin or --rs-macro). Therefore, the assumption is that
                // the root signature was defined in HLSL code. By trying to explicitly create the
                // root signature from the blob, we effectively check if a root signature was
                // properly defined in the HLSL code.
                device_.Get()->CreateRootSignature(0, bytecode.pShaderBytecode,
                    bytecode.BytecodeLength, IID_PPV_ARGS(&compute_pso->pRootSignature));
                assert(compute_pso->pRootSignature != nullptr);
                ret = compute_pso->pRootSignature != nullptr;
                if (!ret)
                {
                    std::cout << kStrErrorDxrRootSignatureFailureHlsl1 <<
                        kStrErrorDxrRootSignatureFailureHlsl2Compute << kStrErrorDxrRootSignatureFailureHlsl3 << std::endl;
                }
            }
        }

        return ret;
    }

    static bool CompileGraphicsShaders(const RgDx12Config& config,
        RgDx12PipelineByteCode& bytecode, D3D12_GRAPHICS_PIPELINE_STATE_DESC*& pso,
        std::string& error_msg)
    {
        bool ret = false;

        // If the input is DXBC binary, we do not need front-end compilation.
        RgPipelineBool is_front_end_compilation_required;
        is_front_end_compilation_required.vert = !config.vert.hlsl.empty();
        is_front_end_compilation_required.hull = !config.hull.hlsl.empty();
        is_front_end_compilation_required.domain = !config.domain.hlsl.empty();
        is_front_end_compilation_required.geom = !config.geom.hlsl.empty();
        is_front_end_compilation_required.pixel = !config.pixel.hlsl.empty();

        // Vertex.
        if (is_front_end_compilation_required.vert)
        {
            // Compile the vertex shader.
            ret = CompileHlslShader(config, config.vert.hlsl, config.vert.entry_point,
                config.vert.shader_model, bytecode.vert, error_msg);
            assert(ret);
            if (ret)
            {
                pso->VS = bytecode.vert;
            }
            else
            {
                error_msg = kStrErrorShaderCompilationFailure;
                error_msg.append("vertex\n");
            }
        }
        else if (!config.vert.dxbc.empty())
        {
            // Read the compiled vertex shader.
            ret = ReadDxbcBinary(config.vert.dxbc, bytecode.vert);
            assert(ret);
            if (ret)
            {
                pso->VS = bytecode.vert;
            }
        }

        // Hull.
        if (is_front_end_compilation_required.hull)
        {
            // Compile the hull shader.
            ret = CompileHlslShader(config, config.hull.hlsl, config.hull.entry_point,
                config.hull.shader_model, bytecode.vert, error_msg);
            assert(ret);
            if (ret)
            {
                pso->HS = bytecode.hull;
            }
            else
            {
                error_msg = kStrErrorShaderCompilationFailure;
                error_msg.append("hull\n");
            }
        }
        else if (!config.hull.dxbc.empty())
        {
            // Read the compiled hull shader.
            ret = ReadDxbcBinary(config.hull.dxbc, bytecode.hull);
            assert(ret);
            if (ret)
            {
                pso->HS = bytecode.hull;
            }
        }

        // Domain.
        if (is_front_end_compilation_required.domain)
        {
            // Compile the domain shader.
            ret = CompileHlslShader(config, config.domain.hlsl, config.domain.entry_point,
                config.domain.shader_model, bytecode.domain, error_msg);
            assert(ret);
            if (ret)
            {
                pso->DS = bytecode.domain;
            }
            else
            {
                error_msg = kStrErrorShaderCompilationFailure;
                error_msg.append("domain\n");
            }
        }
        else if (!config.domain.dxbc.empty())
        {
            // Read the compiled domain shader.
            ret = ReadDxbcBinary(config.domain.dxbc, bytecode.domain);
            assert(ret);
            if (ret)
            {
                pso->DS = bytecode.domain;
            }
        }

        // Geometry.
        if (is_front_end_compilation_required.geom)
        {
            // Compile the geometry shader.
            ret = CompileHlslShader(config, config.geom.hlsl, config.geom.entry_point,
                config.geom.shader_model, bytecode.geom, error_msg);
            assert(ret);
            if (ret)
            {
                pso->GS = bytecode.geom;
            }
            else
            {
                error_msg = kStrErrorShaderCompilationFailure;
                error_msg.append("geometry\n");
            }
        }
        else if (!config.geom.dxbc.empty())
        {
            // Read the compiled geometry shader.
            ret = ReadDxbcBinary(config.geom.dxbc, bytecode.geom);
            assert(ret);
            if (ret)
            {
                pso->GS = bytecode.geom;
            }
        }

        // Pixel.
        if (is_front_end_compilation_required.pixel)
        {
            // Compile the pixel shader.
            ret = CompileHlslShader(config, config.pixel.hlsl, config.pixel.entry_point,
                config.pixel.shader_model, bytecode.pixel, error_msg);
            assert(ret);
            if (ret)
            {
                pso->PS = bytecode.pixel;
            }
            else
            {
                error_msg = kStrErrorShaderCompilationFailure;
                error_msg.append("pixel\n");
            }
        }
        else if (!config.pixel.dxbc.empty())
        {
            // Read the compiled pixel shader.
            ret = ReadDxbcBinary(config.pixel.dxbc, bytecode.pixel);
            assert(ret);
            if (ret)
            {
                pso->PS = bytecode.pixel;
            }
        }
        return ret;
    }

    static void DumpDxbcBinaries(const RgDx12Config& config, const D3D12_GRAPHICS_PIPELINE_STATE_DESC& pso)
    {
        if (!config.vert.dxbcOut.empty())
        {
            bool is_dxbc_written = WriteDxbcFile(pso.VS, config.vert.dxbcOut);
            assert(is_dxbc_written);
        }
        if (!config.hull.dxbcOut.empty())
        {
            bool is_dxbc_written = WriteDxbcFile(pso.HS, config.hull.dxbcOut);
            assert(is_dxbc_written);
        }
        if (!config.domain.dxbcOut.empty())
        {
            bool is_dxbc_written = WriteDxbcFile(pso.DS, config.domain.dxbcOut);
            assert(is_dxbc_written);
        }
        if (!config.geom.dxbcOut.empty())
        {
            bool is_dxbc_written = WriteDxbcFile(pso.GS, config.geom.dxbcOut);
            assert(is_dxbc_written);
        }
        if (!config.pixel.dxbcOut.empty())
        {
            bool is_dxbc_written = WriteDxbcFile(pso.PS, config.pixel.dxbcOut);
            assert(is_dxbc_written);
        }
    }

    static void DumpDxbcDisassembly(const RgDx12Config& config, const RgDx12PipelineByteCode& bytecode)
    {
        if (!config.vert.dxbc_disassembly.empty())
        {
            bool isDxbcDisassemblyGenerated = DisassembleDxbc(bytecode.vert, config.vert.dxbc_disassembly);
            assert(isDxbcDisassemblyGenerated);
        }
        if (!config.hull.dxbc_disassembly.empty())
        {
            bool isDxbcDisassemblyGenerated = DisassembleDxbc(bytecode.hull, config.hull.dxbc_disassembly);
            assert(isDxbcDisassemblyGenerated);
        }
        if (!config.domain.dxbc_disassembly.empty())
        {
            bool isDxbcDisassemblyGenerated = DisassembleDxbc(bytecode.domain, config.domain.dxbc_disassembly);
            assert(isDxbcDisassemblyGenerated);
        }
        if (!config.geom.dxbc_disassembly.empty())
        {
            bool isDxbcDisassemblyGenerated = DisassembleDxbc(bytecode.geom, config.geom.dxbc_disassembly);
            assert(isDxbcDisassemblyGenerated);
        }
        if (!config.pixel.dxbc_disassembly.empty())
        {
            bool isDxbcDisassemblyGenerated = DisassembleDxbc(bytecode.pixel, config.pixel.dxbc_disassembly);
            assert(isDxbcDisassemblyGenerated);
        }
    }

    bool rgDx12Frontend::CreateGraphicsPipeline(const RgDx12Config& config,
        D3D12_GRAPHICS_PIPELINE_STATE_DESC*& pso, std::string& error_msg) const
    {
        bool ret = false;

        // Create and initialize graphics pipeline state descriptor.
        pso = new D3D12_GRAPHICS_PIPELINE_STATE_DESC();
        RgDx12Utils::InitGraphicsPipelineStateDesc(*pso);

        // If the root signature was already created, use it. Otherwise,
        // try to extract it from the DXBC binary after compiling the HLSL code.
        bool should_extract_root_signature = !config.rs_macro.empty();

        // Check if the user provided a serialized root signature file.
        bool is_serialized_root_signature = !config.rs_serialized.empty();

        RgDx12PipelineByteCode bytecode = {};
        HRESULT hr = E_FAIL;

        // Compile the graphics pipeline shaders.
        ret = CompileGraphicsShaders(config, bytecode, pso, error_msg);

        // If the user wants to dump the bytecode as a binary, do it here.
        DumpDxbcBinaries(config, *pso);

        assert(ret);
        if (ret)
        {
            // If the user wants to dump the bytecode disassembly, do it here.
            DumpDxbcDisassembly(config, bytecode);

            if (should_extract_root_signature)
            {
                ComPtr<ID3DBlob> compiled_rs;
                ret = CompileRootSignature(config, compiled_rs);
                if (ret)
                {
                    // Create the root signature through the device and assign it to our PSO.
                    std::vector<uint8_t> root_signature_buffer;
                    root_signature_buffer.resize(compiled_rs->GetBufferSize());
                    memcpy(root_signature_buffer.data(), compiled_rs->GetBufferPointer(),
                        compiled_rs->GetBufferSize());
                    device_.Get()->CreateRootSignature(0, root_signature_buffer.data(),
                        root_signature_buffer.size(), IID_PPV_ARGS(&pso->pRootSignature));
                    assert(pso->pRootSignature != nullptr);
                    ret = pso->pRootSignature != nullptr;
                }
            }
            else if (is_serialized_root_signature)
            {
                // Read the root signature from the file and recreate it.
                std::vector<char> root_signature_buffer;
                ret = RgDx12Utils::ReadBinaryFile(config.rs_serialized, root_signature_buffer);
                assert(ret);
                if (ret)
                {
                    device_.Get()->CreateRootSignature(0, root_signature_buffer.data(),
                        root_signature_buffer.size(), IID_PPV_ARGS(&pso->pRootSignature));
                    assert(pso->pRootSignature != nullptr);
                    ret = pso->pRootSignature != nullptr;
                    if (pso->pRootSignature == nullptr)
                    {
                        std::cerr << kStrErrorRsFileCompileFailed << config.rs_serialized << std::endl;
                    }
                }
                else
                {
                    std::cerr << kStrErrorRsFileReadFailed << config.rs_serialized << std::endl;
                }
            }
            else
            {
                // Make sure that the root signature exists in one of the blobs.
                bool does_rs_exists = false;

                // Go through each of the existing blobs, and try to create the root signature
                // from it. If the root signature was defined together with the [RootSignature()]
                // attribute in HLSL code, the root signature creation from the blob should succeed.

                // Vert blob.
                if (bytecode.vert.pShaderBytecode != nullptr && bytecode.vert.BytecodeLength > 0)
                {
                    device_.Get()->CreateRootSignature(0, bytecode.vert.pShaderBytecode,
                        bytecode.vert.BytecodeLength, IID_PPV_ARGS(&pso->pRootSignature));
                    ret = pso->pRootSignature != nullptr;
                    does_rs_exists = (pso->pRootSignature != nullptr);
                }

                // Hull blob.
                if (!does_rs_exists && bytecode.hull.pShaderBytecode != nullptr && bytecode.hull.BytecodeLength > 0)
                {
                    device_.Get()->CreateRootSignature(0, bytecode.hull.pShaderBytecode,
                        bytecode.hull.BytecodeLength, IID_PPV_ARGS(&pso->pRootSignature));
                    ret = pso->pRootSignature != nullptr;
                    does_rs_exists = (pso->pRootSignature != nullptr);
                }

                // Domain blob.
                if (!does_rs_exists && bytecode.domain.pShaderBytecode != nullptr && bytecode.domain.BytecodeLength > 0)
                {
                    device_.Get()->CreateRootSignature(0, bytecode.domain.pShaderBytecode,
                        bytecode.domain.BytecodeLength, IID_PPV_ARGS(&pso->pRootSignature));
                    ret = pso->pRootSignature != nullptr;
                    does_rs_exists = (pso->pRootSignature != nullptr);
                }

                // Geometry blob.
                if (!does_rs_exists && bytecode.geom.pShaderBytecode != nullptr && bytecode.geom.BytecodeLength > 0)
                {
                    device_.Get()->CreateRootSignature(0, bytecode.geom.pShaderBytecode,
                        bytecode.geom.BytecodeLength, IID_PPV_ARGS(&pso->pRootSignature));
                    ret = pso->pRootSignature != nullptr;
                    does_rs_exists = (pso->pRootSignature != nullptr);
                }

                // Pixel blob.
                if (!does_rs_exists && bytecode.pixel.pShaderBytecode != nullptr && bytecode.pixel.BytecodeLength > 0)
                {
                    device_.Get()->CreateRootSignature(0, bytecode.pixel.pShaderBytecode,
                        bytecode.pixel.BytecodeLength, IID_PPV_ARGS(&pso->pRootSignature));
                    ret = pso->pRootSignature != nullptr;
                    does_rs_exists = (pso->pRootSignature != nullptr);
                }

                assert(does_rs_exists);
                if (!does_rs_exists)
                {
                    std::cout << kStrErrorDxrRootSignatureFailureHlsl1 <<
                        kStrErrorDxrRootSignatureFailureHlsl2Graphics << kStrErrorDxrRootSignatureFailureHlsl3 << std::endl;
                }
            }
        }
        return ret;
    }

    bool rgDx12Frontend::ExtractRootSignature(const D3D12_SHADER_BYTECODE& bytecode,
        ID3D12RootSignature*& root_signature, std::string& error_msg) const
    {
        bool ret = false;

        // Try to extract the root signature.
        ComPtr<ID3DBlob> bytecode_root_signature;
        HRESULT hr = D3DGetBlobPart(bytecode.pShaderBytecode, bytecode.BytecodeLength,
            D3D_BLOB_ROOT_SIGNATURE, 0, &bytecode_root_signature);

        if (SUCCEEDED(hr))
        {
            std::vector<uint8_t> root_signature_data;
            root_signature_data.resize(bytecode_root_signature->GetBufferSize());
            memcpy(root_signature_data.data(), bytecode_root_signature->GetBufferPointer(),
                bytecode_root_signature->GetBufferSize());
            device_.Get()->CreateRootSignature(0, root_signature_data.data(),
                root_signature_data.size(), IID_PPV_ARGS(&root_signature));
            assert(root_signature != nullptr);
            ret = (root_signature != nullptr);
        }
        else
        {
            error_msg = kStrErrorRootSignatureExtractionFailure;
        }

        return ret;
    }

    bool rga::rgDx12Frontend::GetHardwareAdapter(IDXGIAdapter1** dxgi_adapter)
    {
        bool ret = false;

        // Create the factory.
        UINT dxgi_factory_flags = 0;
        ComPtr<IDXGIFactory4> factory;
        HRESULT hr = CreateDXGIFactory2(dxgi_factory_flags, IID_PPV_ARGS(&factory));
        assert(SUCCEEDED(hr));
        if (SUCCEEDED(hr))
        {
            // Find the adapter.
            ComPtr<IDXGIAdapter1> adapter;
            *dxgi_adapter = nullptr;

            for (UINT adapter_index = 0; DXGI_ERROR_NOT_FOUND !=
                factory->EnumAdapters1(adapter_index, &adapter); ++adapter_index)
            {
                DXGI_ADAPTER_DESC1 desc;
                adapter->GetDesc1(&desc);

                // Look for a physical AMD display adapter.
                const UINT kAmdVendorId = 0x1002;
                if (desc.VendorId != kAmdVendorId || (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE))
                {
                    // Don't select the Basic Render Driver adapter.
                    continue;
                }

                if (SUCCEEDED(D3D12CreateDevice(adapter.Get(),
                    feature_level_, _uuidof(ID3D12Device), nullptr)))
                {
                    ret = true;
                    break;
                }
            }

            if (!ret)
            {
                std::cout << kStrErrorAmdDisplayAdapterNotFound << std::endl;
            }

            *dxgi_adapter = adapter.Detach();
        }

        return ret;
    }

    bool rgDx12Frontend::Init(bool is_dxr_session)
    {
        bool ret = false;

        // Create the DXGI adapter.
        ComPtr<IDXGIAdapter1> hw_adapter;
        ret = GetHardwareAdapter(&hw_adapter);
        assert(ret);
        assert(hw_adapter != nullptr);
        if (ret && hw_adapter != nullptr)
        {
            // Create the D3D12 device.
            HRESULT hr = D3D12CreateDevice(hw_adapter.Get(),
                feature_level_,
                IID_PPV_ARGS(&device_));
            assert(SUCCEEDED(hr));
            if (SUCCEEDED(hr))
            {
                // Initialize the backend with the D3D12 device.
                ret = backend_.Init(device_.Get());
                assert(ret);
                if (ret && is_dxr_session)
                {
                    // Create the DXR interface.
                    hr = device_->QueryInterface(IID_PPV_ARGS(&dxr_device_));
                    assert(SUCCEEDED(hr));
                    if (!SUCCEEDED(hr))
                    {
                        std::cout << kStrErrorFailedToCreateDxrInterface << std::endl;
                    }
                }
            }
        }
        else
        {
            std::cerr << kStrErrorFailedToFindDx12Adapter << std::endl;
        }
        return ret;
    }

    bool rgDx12Frontend::GetSupportedTargets(std::vector<std::string>& supported_targets,
        std::map<std::string, unsigned>& target_to_id) const
    {
        return backend_.GetSupportedTargets(supported_targets, target_to_id);
    }

    bool rgDx12Frontend::CompileComputePipeline(const RgDx12Config& config, std::string& error_msg) const
    {
        D3D12_COMPUTE_PIPELINE_STATE_DESC* pso = nullptr;
        bool ret = CreateComputePipeline(config, pso, error_msg);
        assert(ret);
        if (ret)
        {
            // Create the compute pipeline state.
            ID3D12PipelineState* compute_pso = nullptr;
            HRESULT hr = device_->CreateComputePipelineState(pso, IID_PPV_ARGS(&compute_pso));
            assert(SUCCEEDED(hr));
            ret = SUCCEEDED(hr);
            if (ret)
            {
                RgDx12ShaderResults results;
                RgDx12ThreadGroupSize thread_group_size;
                std::vector<char> pipeline_binary;
                ret = backend_.CompileComputePipeline(pso, results, thread_group_size, pipeline_binary, error_msg);
                assert(ret);
                if (ret)
                {
                    if (!config.comp.isa.empty())
                    {
                        // Save results to file: GCN ISA disassembly.
                        assert(results.disassembly != nullptr);
                        if (results.disassembly != nullptr)
                        {
                            // GCN ISA Disassembly.
                            std::cout << kStrInfoExtractComputeShaderDisassembly << std::endl;
                            ret = RgDx12Utils::WriteTextFile(config.comp.isa, results.disassembly);

                            // Report the result to the user.
                            std::cout << (ret ? kStrInfoExtractComputeShaderDisassemblySuccess :
                                kStrErrorExtractComputeShaderDisassemblyFailure) << std::endl;
                        }
                        else
                        {
                            std::cerr << kStrErrorComputeShaderDisassemblyExtractionFailure << std::endl;
                        }
                        assert(ret);
                    }

                    if (!config.comp.stats.empty())
                    {
                        // Save results to file: statistics.
                        std::cout << kStrInfoExtractComputeShaderStats << std::endl;
                        bool isStatsSaved = SerializeDx12StatsCompute(results, thread_group_size, config.comp.stats);
                        assert(isStatsSaved);
                        ret = ret && isStatsSaved;

                        // Report the result to the user.
                        std::cout << (isStatsSaved ? kStrInfoExtractComputeShaderStatsSuccess :
                            kStrErrorExtractComputeShaderStatsFailure) << std::endl;
                        assert(ret);
                    }

                    if (!config.pipeline_binary.empty())
                    {
                        // Pipeline binary.
                        std::cout << kStrInfoExtractComputePipelineBinary << std::endl;
                        ret = !pipeline_binary.empty() && RgDx12Utils::WriteBinaryFile(config.pipeline_binary, pipeline_binary);

                        // Report the result to the user.
                        std::cout << (ret ? kStrInfoExtractComputePipelineBinarySuccess :
                            kStrErrorExtractComputePipelineBinaryFailure) << std::endl;
                    }
                }
                else if (!error_msg.empty())
                {
                    // Print the error messages to the user.
                    std::cerr << error_msg << std::endl;
                }
            }
            else
            {
                std::cerr << kStrErrorFailedToCreateComputePipeline << std::endl;
                std::cerr << kStrHintRootSignatureFailure << std::endl;
            }
        }
        return ret;
    }

    static bool WriteGraphicsShaderOutputFiles(const RgDx12ShaderConfig& shader_config,
        const RgDx12ShaderResults& shader_results, const std::string& stage_name)
    {
        bool ret = true;
        if (!shader_config.isa.empty())
        {
            // Save results to file: GCN ISA disassembly.
            assert(shader_results.disassembly != nullptr);
            if (shader_results.disassembly != nullptr)
            {
                // GCN ISA Disassembly.
                std::cout << kStrInfoExtractGraphicsShaderOutput1 << stage_name <<
                    kStrInfoExtractGraphicsShaderDisassembly << std::endl;
                ret = RgDx12Utils::WriteTextFile(shader_config.isa, shader_results.disassembly);
                assert(ret);

                // Report the result to the user.
                if (ret)
                {
                    std::cout << stage_name << kStrInfoExtractGraphicsShaderDisassemblySuccess << std::endl;
                }
                else
                {
                    std::cerr << kStrErrorExtractGraphicsShaderOutputFailure1 << stage_name <<
                        kStrErrorExtractGraphicsShaderDisassemblyFailure2 << std::endl;
                }
            }
            else
            {
                std::cerr << kStrErrorGraphicsShaderDisassemblyExtractionFailure1 << stage_name <<
                    kStrErrorGraphicsShaderDisassemblyExtractionFailure2 << std::endl;
            }
        }

        if (!shader_config.stats.empty())
        {
            // Save results to file: statistics.
            std::cout << kStrInfoExtractGraphicsShaderOutput1 << stage_name <<
                kStrInfoExtractGraphicsShaderStats2 << std::endl;
            bool isStatsSaved = SerializeDx12StatsGraphics(shader_results, shader_config.stats);
            assert(isStatsSaved);
            ret = ret && isStatsSaved;

            // Report the result to the user.
            if (isStatsSaved)
            {
                std::cout << stage_name << kStrInfoExtractGraphicsShaderStatsSuccess << std::endl;
            }
            else
            {
                std::cout << kStrErrorExtractGraphicsShaderOutputFailure1 << stage_name <<
                    kStrErrorExtractGraphicsShaderStatsFailure2 << std::endl;
            }
        }

        return ret;
    }

    bool rgDx12Frontend::CompileGraphicsPipeline(const RgDx12Config& config, std::string& error_msg) const
    {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC* pso = nullptr;
        RgDx12PipelineResults results = {};
        std::vector<char> pipeline_binary;

        bool ret = CreateGraphicsPipeline(config, pso, error_msg);
        assert(ret);
        if (ret)
        {
            // Create the graphics pipeline state.
            ID3D12PipelineState* graphics_pso = nullptr;

            // For graphics, we must use a .gpso file.
            bool is_pso_file_parsed = false;
            if (!config.rs_pso.empty())
            {
                // Load the .gpso file that the user provided, and extract the
                // relevant pipeline state.
                is_pso_file_parsed = RgDx12Utils::ParseGpsoFile(config.rs_pso, *pso);
                assert(is_pso_file_parsed);
            }
            else
            {
                // Use a default pso.
                assert(false);
                std::cout << "Warning: no pipeline state file received. Use the --gpso switch to provide a graphics pipeline description." << std::endl;
            }

            if (is_pso_file_parsed)
            {
                // Check if the user provided a serialized root signature file.
                bool is_serialized_root_signature = !config.rs_serialized.empty();

                // Read the root signature from the file and recreate it.
                if (is_serialized_root_signature)
                {
                    std::vector<char> root_signature_buffer;
                    ret = RgDx12Utils::ReadBinaryFile(config.rs_serialized, root_signature_buffer);
                    assert(ret);
                    if (ret)
                    {
                        device_.Get()->CreateRootSignature(0, root_signature_buffer.data(),
                            root_signature_buffer.size(), IID_PPV_ARGS(&pso->pRootSignature));
                        assert(pso->pRootSignature != nullptr);
                        ret = pso->pRootSignature != nullptr;
                    }
                }
                else
                {
                    // If the root signature was already created, use it. Otherwise,
                    // try to extract it from the DXBC binary after compiling the HLSL code.
                    bool should_extract_root_signature = !config.rs_macro.empty();

                    // If the input is HLSL, we need to compile the root signature out of the HLSL file.
                    // Otherwise, if the input is a DXBC binary and it has a root signature baked into it,
                    // then the root signature would be automatically fetched from the binary, there is no
                    // need to set it into the PSO's root signature field.
                    ComPtr<ID3DBlob> compiled_rs;
                    ret = CompileRootSignature(config, compiled_rs);
                    if (ret)
                    {
                        // Create the root signature through the device and assign it to our PSO.
                        std::vector<uint8_t> root_signature_buffer;
                        root_signature_buffer.resize(compiled_rs->GetBufferSize());
                        memcpy(root_signature_buffer.data(), compiled_rs->GetBufferPointer(),
                            compiled_rs->GetBufferSize());
                        device_.Get()->CreateRootSignature(0, root_signature_buffer.data(),
                            root_signature_buffer.size(), IID_PPV_ARGS(&pso->pRootSignature));
                        assert(pso->pRootSignature != nullptr);
                        ret = pso->pRootSignature != nullptr;
                    }
                }

                // Create the graphics pipeline.
                HRESULT hr = device_->CreateGraphicsPipelineState(pso, IID_PPV_ARGS(&graphics_pso));
                assert(SUCCEEDED(hr));
                ret = SUCCEEDED(hr);

                if (ret)
                {
                    ret = backend_.CompileGraphicsPipeline(pso, results, pipeline_binary, error_msg);
                    assert(ret);

                    if (ret)
                    {
                        // Write the output files (for applicable stages).
                        ret = WriteGraphicsShaderOutputFiles(config.vert, results.vertex, "vertex");
                        assert(ret);
                        ret = WriteGraphicsShaderOutputFiles(config.hull, results.hull, "hull");
                        assert(ret);
                        ret = WriteGraphicsShaderOutputFiles(config.domain, results.domain, "domain");
                        assert(ret);
                        ret = WriteGraphicsShaderOutputFiles(config.geom, results.geometry, "geometry");
                        assert(ret);
                        ret = WriteGraphicsShaderOutputFiles(config.pixel, results.pixel, "pixel");
                        assert(ret);

                        // Write the pipeline binary file.
                        if (!config.pipeline_binary.empty())
                        {
                            std::cout << kStrInfoExtractGraphicsPipelineBinary << std::endl;
                            ret = !pipeline_binary.empty() && RgDx12Utils::WriteBinaryFile(config.pipeline_binary, pipeline_binary);

                            // Report the result to the user.
                            std::cout << (ret ? kStrInfoExtractGraphicsPipelineBinarySuccess :
                                kStrErrorExtractGraphicsShaderStatsFailure) << std::endl;
                        }
                    }
                }
                else
                {
                    std::stringstream msg;
                    if (!error_msg.empty())
                    {
                        msg << std::endl;
                    }
                    msg << kStrErrorGraphicsPipelineCreationFailure << std::endl;
                    msg << kStrHintRootSignatureFailure << std::endl;
                    error_msg.append(msg.str());
                }
            }
            else
            {
                std::cerr << kStrErrorGpsoFileParseFailed << std::endl;
            }
        }
        return ret;
    }

#ifdef RGA_DXR_ENABLED
    bool rgDx12Frontend::CompileRayTracingPipeline(const RgDx12Config& config, std::string& error_msg) const
    {
        bool ret = false;
        bool should_abort = false;
        RgDxrStateDesc state_desc;
        if (!config.dxr_hlsl_input.empty())
        {
            // If an HLSL input was given, we will only use the HLSL as an input, without
            // reading the state data.
            std::shared_ptr<RgDxrDxilLibrary> dxil_lib = std::make_shared<RgDxrDxilLibrary>();
            dxil_lib->input_type = DxrSourceType::kHlsl;
            dxil_lib->full_path = config.dxr_hlsl_input;
            state_desc.input_files.push_back(dxil_lib);
        }
        else
        {
            // Read the input metadata file.
            const char* kJsonInputFileName = config.dxr_state_file.c_str();
            bool is_state_desc_read = RgDxrStateDescReader::ReadDxrStateDesc(kJsonInputFileName, state_desc, error_msg);
            assert(is_state_desc_read);
            should_abort = !is_state_desc_read;
            if (!error_msg.empty())
            {
                std::cerr << error_msg << std::endl;
                error_msg.clear();
            }
        }

        if (!should_abort)
        {
            // Validate DXR input.
            should_abort = !IsDxrInputValid(state_desc);
            assert(!should_abort);

            if (!should_abort)
            {
                // A map to hold the HLSL->DXIL mapping if needed.
                std::map<std::string, std::string> hlsl_dxil_mapping;

                // Read the binary data for DXIL libraries.
                for (std::shared_ptr<RgDxrDxilLibrary> dxil_lib : state_desc.input_files)
                {
                    assert(dxil_lib != nullptr);
                    if (dxil_lib != nullptr)
                    {
                        std::string dxil_file_to_read;
                        if (dxil_lib->input_type == DxrSourceType::kBinary)
                        {
                            dxil_file_to_read = dxil_lib->full_path;
                        }
                        else
                        {
                            // Read the HLSL mapping file if we haven't done it already.
                            if (hlsl_dxil_mapping.empty())
                            {
                                ReadHlslMappingFile(config.dxr_hlsl_mapping, hlsl_dxil_mapping);
                            }

                            // Retrieve the DXIL binary which corresponds to the current HLSL file.
                            dxil_file_to_read = hlsl_dxil_mapping[dxil_lib->full_path];
                        }

                        // Read the binary DXIL file.
                        bool is_file_read = RgDx12Utils::ReadBinaryFile(dxil_file_to_read, dxil_lib->binary_data);
                        assert(is_file_read);
                        if (!is_file_read)
                        {
                            should_abort = true;
                            std::stringstream msg;
                            msg << kStrErrorDxilFileReadFailed << dxil_lib->full_path;
                            error_msg = msg.str();
                        }
                    }
                }

                if (!should_abort)
                {
                    // Assume pipeline mode by default, unless user specified shader mode explicitly.
                    D3D12_STATE_OBJECT_TYPE stateObjectType = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;
                    std::string modeLower = RgDx12Utils::ToLower(config.dxr_mode);
                    if (modeLower.compare("shader") == 0)
                    {
                        stateObjectType = D3D12_STATE_OBJECT_TYPE_COLLECTION;
                    }

                    // Create the State Object descriptor, based upon which the state object would be created.
                    CD3DX12_STATE_OBJECT_DESC dxr_state_desc(stateObjectType);

                    // Create the DXIL library subobjects.
                    for (std::shared_ptr<RgDxrDxilLibrary> dxil_lib : state_desc.input_files)
                    {
                        // Treat Binary and HLSL the same, since HLSL if applicable has already
                        // been converted to binary DXIL and read from disk.
                        if (dxil_lib->input_type == DxrSourceType::kBinary ||
                            (dxil_lib->input_type == DxrSourceType::kHlsl))
                        {
                            RgDx12Factory::CreateDxilLibrarySubobject(&dxr_state_desc, *dxil_lib, error_msg);
                        }
                        else
                        {
                            should_abort = true;
                        }
                    }

                    if (!should_abort)
                    {
                        // Create the hit groups and push a copy to our subobject container.
                        for (const RgDxrHitGroup& hitGroup : state_desc.hit_groups)
                        {
                            RgDx12Factory::CreateHitGroupSubobject(&dxr_state_desc, hitGroup);
                        }

                        // Create the shader config and push a copy to our subobject container.
                        for (const RgDxrShaderConfig& sc : state_desc.shader_config)
                        {
                            RgDx12Factory::CreateShaderConfigSubobject(&dxr_state_desc, sc.max_payload_size_in_bytes, sc.max_attribute_size_in_bytes);
                        }

                        // Local root signatures.
                        ID3D12RootSignature* local_root_signature = nullptr;
                        for (std::shared_ptr<RgDxrRootSignature> pLocalRs : state_desc.local_root_signature)
                        {
                            bool is_root_signature_created = CreateRootSignatureFromBinary(device_.Get(), pLocalRs->full_path, local_root_signature);
                            assert(is_root_signature_created);
                            assert(local_root_signature != nullptr);
                            if (!is_root_signature_created || local_root_signature == nullptr)
                            {
                                std::cout << kStrErrorLocalRootSignatureCreateFromFileFailure << pLocalRs->full_path << std::endl;
                                should_abort = true;
                                break;
                            }

                            RgDx12Factory::CreateLocalRootSignatureSubobject(&dxr_state_desc, local_root_signature, pLocalRs->exports);
                        }

                        if (!should_abort)
                        {
                            // Global root signatures.
                            ID3D12RootSignature* global_root_signature = nullptr;
                            for (std::shared_ptr<RgDxrRootSignature> pGlobalRs : state_desc.global_root_signature)
                            {
                                bool is_root_signature_created = CreateRootSignatureFromBinary(device_.Get(), pGlobalRs->full_path, global_root_signature);
                                assert(is_root_signature_created);
                                assert(global_root_signature != nullptr);
                                if (!is_root_signature_created || global_root_signature == nullptr)
                                {
                                    std::cout << kStrErrorGlobalRootSignatureCreateFromFileFailure << pGlobalRs->full_path << std::endl;
                                    should_abort = true;
                                    break;
                                }

                                RgDx12Factory::CreateGlobalRootSignatureSubobject(&dxr_state_desc, global_root_signature);
                            }

                            if (!should_abort)
                            {
                                // Pipeline config.
                                RgDx12Factory::CreatePipelineConfigSubobject(&dxr_state_desc, state_desc.pipeline_config.max_trace_recursion_depth);
                                ComPtr<ID3D12StateObject> dxrStateObject;
#ifdef _DXRTEST
                                PrintStateObjectDesc(dxr_state_desc);
                                HRESULT rc = dxr_device_->CreateStateObject(dxr_state_desc, IID_PPV_ARGS(&dxrStateObject));
                                assert(SUCCEEDED(rc));
#endif // _DXRTEST
                                // Containers to track the output files that we created.
                                // Since the compiler may generate an arbitrary number of shaders with names that are
                                // not known to us in advance, we would have to communicate this list to the calling process.

                                // Track the results.
                                std::vector<RgDxrPipelineResults> results_pipeline_mode;

                                const wchar_t* kStrExportNameAll = L"all";
                                const char* kStrModeNameShader = "shader";
                                const char* kOutputFilenameSuffixUnified = "unified";
                                std::wstring dxr_export_wide = RgDx12Utils::strToWstr(config.dxrExport);
                                std::string export_lower = RgDx12Utils::ToLower(config.dxrExport);
                                std::wstring dxr_export_wide_lower = RgDx12Utils::strToWstr(config.dxrExport);
                                bool is_shader_mode = modeLower.compare(kStrModeNameShader) == 0;
                                if (is_shader_mode)
                                {
                                    std::vector<std::shared_ptr<RgDx12ShaderResultsRayTracing>> rs;
                                    std::vector<unsigned char> pipeline_binary;
                                    bool is_unified_mode = true;
                                    std::vector<std::string> indirect_shader_names;

                                    // Compile and extract the results for a single shader.
                                    std::shared_ptr<RgDx12ShaderResultsRayTracing> raytracing_shader_stats = std::make_shared<RgDx12ShaderResultsRayTracing>();
                                    ret = backend_.CreateStateObjectShader(dxr_state_desc, dxr_export_wide, *raytracing_shader_stats, pipeline_binary, error_msg);
                                    assert(ret);
                                    if (ret)
                                    {
                                        rs.push_back(raytracing_shader_stats);
                                    }
                                    else
                                    {
                                        if (error_msg.empty())
                                        {
                                            std::cerr << kStrErrorDxrShaderModeCompilationFailed << std::endl;
                                        }
                                    }


                                    if (ret)
                                    {
                                        RgDxrPipelineResults pipeline_results;
                                        pipeline_results.isUnified = is_unified_mode;

                                        // If we are in Pipeline (non All) mode, we would use the export name as the name of the pipeline.
                                        pipeline_results.pipeline_name = "null";

                                        uint32_t i = 0;
                                        for (const auto& curr_results : rs)
                                        {
                                            // If we are in Unified mode, pipeline name is identical to export name (raygeneration shader name).
                                            RgDxrShaderResults shader_results_tracking;
                                            if (is_unified_mode || is_shader_mode)
                                            {
                                                shader_results_tracking.export_name = config.dxrExport;
                                            }
                                            else
                                            {
                                                assert(i < indirect_shader_names.size());
                                                if (i < indirect_shader_names.size())
                                                {
                                                    shader_results_tracking.export_name = indirect_shader_names[i];
                                                }
                                            }

                                            assert(is_unified_mode || i < indirect_shader_names.size());
                                            if (!config.dxr_isa_output.empty() && curr_results->disassembly != nullptr)
                                            {
                                                // Save output files and report to user: disassembly.
                                                if (is_shader_mode || is_unified_mode)
                                                {
                                                    std::cout << (!is_shader_mode ? kStrInfoExtractRayTracingDisassemblyPipelineByRaygen : kStrInfoExtractRayTracingDisassemblyShader)
                                                        << config.dxrExport << "..." << std::endl;
                                                }
                                                else
                                                {
                                                    std::cout << kStrInfoExtractRayTracingDisassemblyShader << "#" << (i + 1) << ": " << indirect_shader_names[i] << "..." << std::endl;
                                                }

                                                std::string output_filename_fixed = config.dxr_isa_output;
                                                if (!is_shader_mode)
                                                {
                                                    output_filename_fixed = GenerateSpecificFileName(output_filename_fixed, (is_unified_mode ? kOutputFilenameSuffixUnified : indirect_shader_names[i]));
                                                }

                                                bool is_disassembly_saved = RgDx12Utils::WriteTextFile(output_filename_fixed, curr_results->disassembly);
                                                std::cout << (is_disassembly_saved ? kStrInfoExtractRayTracingDisassemblySuccess :
                                                    kStrErrorFailedToExtractRaytracingDisassembly) << std::endl;
                                                assert(is_disassembly_saved);
                                                ret = ret && is_disassembly_saved;
                                                if (is_disassembly_saved)
                                                {
                                                    shader_results_tracking.isa_disassembly = output_filename_fixed;
                                                }
                                            }

                                            if (!config.dxr_statistics_output.empty())
                                            {
                                                // Save output files and report to user: statistics.
                                                if (is_shader_mode)
                                                {
                                                    std::cout << kStrInfoExtractRayTracingStatsShader << config.dxrExport << "..." << std::endl;
                                                }
                                                else
                                                {
                                                    std::cout << kStrInfoExtractRayTracingStatsShader << "#" << (i + 1) << ": " <<
                                                        indirect_shader_names[i] << "..." << std::endl;
                                                }

                                                std::string output_filename_fixed = config.dxr_statistics_output;
                                                if (!is_shader_mode)
                                                {
                                                    output_filename_fixed = GenerateSpecificFileName(output_filename_fixed, (is_unified_mode ? kOutputFilenameSuffixUnified : indirect_shader_names[i]));
                                                }

                                                bool is_statistics_saved = SerializeDx12StatsRayTracing(*curr_results, output_filename_fixed);
                                                std::cout << (is_statistics_saved ? kStrInfoExtractRayTracingStatisticsSuccess :
                                                    kStrErrorFailedToExtractRaytracingStatistics) << std::endl;
                                                assert(is_statistics_saved);
                                                ret = ret && is_statistics_saved;
                                                if (is_statistics_saved)
                                                {
                                                    shader_results_tracking.stats = output_filename_fixed;
                                                }
                                            }

                                            // Track the current results.
                                            pipeline_results.results.push_back(shader_results_tracking);

                                            // Increment the index for the current result.
                                            ++i;
                                        }

                                        // Pipeline binaries extraction is not supported in shader mode.
                                        if (!is_shader_mode && is_unified_mode && !config.dxr_binary_output.empty())
                                        {
                                            // Save output files and report to user: binaries.
                                            std::cout << kStrInfoExtractRayTracingPipelineBinaryByRaygen << config.dxrExport << "..." << std::endl;
                                            std::string output_filename_fixed = config.dxr_binary_output;
                                            output_filename_fixed = GenerateSpecificFileName(output_filename_fixed, (is_unified_mode ? kOutputFilenameSuffixUnified : ""));
                                            bool is_binary_saved = RgDx12Utils::WriteBinaryFile(output_filename_fixed, pipeline_binary);
                                            std::cout << (is_binary_saved ? kStrInfoExtractRayTracingBinarySuccess :
                                                kStrErrorFailedToExtractRaytracingBinary) << std::endl;
                                            assert(is_binary_saved);
                                            ret = ret && is_binary_saved;
                                            if (is_binary_saved)
                                            {
                                                pipeline_results.pipeline_binary = output_filename_fixed;
                                            }
                                        }

                                        // Track the pipeline results.
                                        results_pipeline_mode.push_back(pipeline_results);
                                    }
                                }
                                else
                                {
                                    // Pipeline mode: compile and extract the results for all generated pipelines.
                                    // Per-pipeline results.
                                    std::vector<std::shared_ptr<std::vector<std::shared_ptr<RgDx12ShaderResultsRayTracing>>>> rs;

                                    // Per-pipeline binary.
                                    std::vector<std::shared_ptr<std::vector<unsigned char>>> pipeline_binaries;

                                    // Per-pipeline flag that specifies whether the pipeline was compiled in Unified mode.
                                    std::vector<bool> is_unified;

                                    // Container for the raygeneration shader name for each generated pipeline.
                                    std::vector<std::shared_ptr<std::vector<std::string>>> raygeneration_shader_names;

                                    // Compile.
                                    ret = backend_.CreateStateObject(dxr_state_desc, rs, pipeline_binaries,
                                        is_unified, raygeneration_shader_names, error_msg);
                                    assert(ret);
                                    assert(!rs.empty());
                                    if (ret && !rs.empty())
                                    {
                                        // Notify the user about the number of pipelines that were generated.
                                        std::cout << kStrInfoDxrPipelineCompilationGenerated1 << rs.size() <<
                                           (rs.size() == 1 ? kStrInfoDxrPipelineCompilationGeneratedSinglePipeline2 :
                                               kStrInfoDxrPipelineCompilationGeneratedMultiplePipelines2) << std::endl;

                                        // For each pipeline, let's extract the results.
                                        for (uint32_t pipeline_index = 0; pipeline_index < rs.size(); pipeline_index++)
                                        {
                                            // Running number to identify the current pipeline in Indirect mode.
                                            const std::string pipeline_index_display_name = std::to_string(pipeline_index + 1);

                                            // The shader names for the current pipeline. In case we are in Unified mode, there will
                                            // be only a single shader name (the raygeneration shader).
                                            std::shared_ptr<std::vector<std::string>> pipeline_shader_names = raygeneration_shader_names[pipeline_index];
                                            const auto& curr_raygeneration_shader_names = *pipeline_shader_names;

                                            // The raygeneration shader name for this pipeline (essentially, the pipeline identifier).
                                            // We already have in hand the vector that holds the raygeneration shader name for the current pipeline
                                            // so we will take the first (and only) element.
                                            const char* curr_raygeneration_shader_name = curr_raygeneration_shader_names[0].data();

                                            // The results for the current pipeline.
                                            const auto& curr_results = rs[pipeline_index];

                                            // Report the type of compilation (Unified/Indirect) to the user for the current pipeline.
                                            std::cout << kStrInfoDxrPipelineTypeReport1 << (pipeline_index + 1);
                                            if (is_unified[pipeline_index])
                                            {
                                                std::cout << kStrInfoDxrPipelineTypeReport2 <<
                                                    curr_raygeneration_shader_name;
                                            }
                                            std::cout << ": " << std::endl;

                                            if (is_unified[pipeline_index])
                                            {
                                                // Unified mode - expect a single shader in output.
                                                assert(curr_results->size() == 1);
                                                std::cout << kStrInfoDxrPipelineCompiledUnified << std::endl;
                                            }
                                            else
                                            {
                                                // Indirect mode - expect N shaders in output.
                                                assert(curr_results->size() >= 1);
                                                std::cout << kStrInfoDxrPipelineCompiledIndirect1 << curr_results->size() <<
                                                    kStrInfoDxrPipelineCompiledIndirect2 << std::endl;
                                            }

                                            // Output metadata.
                                            RgDxrPipelineResults curr_pipeline_results_metadata;
                                            if (is_unified[pipeline_index])
                                            {
                                                // Since this pipeline was compiled in Unified mode, shader name is pipeline name.
                                                curr_pipeline_results_metadata.pipeline_name = curr_raygeneration_shader_name;
                                                curr_pipeline_results_metadata.isUnified = true;
                                            }
                                            else
                                            {
                                                // In Indirect mode we don't have the name of the raygeneration shader, just an index.
                                                curr_pipeline_results_metadata.pipeline_name = pipeline_index_display_name;
                                                curr_pipeline_results_metadata.isUnified = false;
                                            }

                                            // Go through pipeline's shaders.
                                            for (uint32_t shader_index = 0; shader_index < rs[pipeline_index]->size(); shader_index++)
                                            {
                                                // Metadata to track the output files for this shader.
                                                RgDxrShaderResults curr_shader_results_metadata;

                                                // Track the export name.
                                                if (is_unified[pipeline_index])
                                                {
                                                    // In unified mode, this is the raygeneration shader name.
                                                    curr_shader_results_metadata.export_name = curr_raygeneration_shader_name;
                                                }
                                                else
                                                {
                                                    // In indirect mode, this is the name of the current shader.
                                                    assert(shader_index < curr_raygeneration_shader_names.size());
                                                    if (shader_index < curr_raygeneration_shader_names.size())
                                                    {
                                                        curr_shader_results_metadata.export_name = curr_raygeneration_shader_names[shader_index].data();
                                                    }
                                                    else
                                                    {
                                                        std::cout << kStrErrorDxrFailedToRetrievePipelineShaderName <<
                                                            (shader_index + 1) << "." << std::endl;
                                                    }
                                                }

                                                // Build a suffix for this shader's output files.
                                                std::stringstream shader_output_file_suffix_stream;
                                                if (is_unified[pipeline_index])
                                                {
                                                    shader_output_file_suffix_stream << curr_shader_results_metadata.export_name.c_str() << "_";
                                                    shader_output_file_suffix_stream << kOutputFilenameSuffixUnified;
                                                }
                                                else
                                                {
                                                    shader_output_file_suffix_stream << pipeline_index_display_name << "_" <<
                                                        curr_shader_results_metadata.export_name.c_str();
                                                }
                                                const std::string shader_output_file_suffix = shader_output_file_suffix_stream.str().c_str();

                                                // ISA files.
                                                if (!config.dxr_isa_output.empty())
                                                {
                                                    // Generate an index-specific file name.
                                                    std::string isa_output_filename = GenerateSpecificFileName(config.dxr_isa_output, shader_output_file_suffix);
                                                    assert(!isa_output_filename.empty());
                                                    if (!isa_output_filename.empty())
                                                    {
                                                        // Save per-pipeline disassembly.
                                                        if (is_unified[pipeline_index])
                                                        {
                                                            std::cout << kStrInfoExtractRayTracingDisassemblyPipelineByRaygen << curr_shader_results_metadata.export_name;
                                                        }
                                                        else
                                                        {
                                                            std::cout << kStrInfoExtractRayTracingDisassemblyPipelineByIndex1 << pipeline_index_display_name <<
                                                                kStrInfoExtractRayTracingResultByIndex2 << curr_shader_results_metadata.export_name;
                                                        }
                                                        std::cout << "..." << std::endl;

                                                        bool is_disassembly_saved = RgDx12Utils::WriteTextFile(isa_output_filename, (*curr_results)[shader_index]->disassembly);
                                                        ret = ret && is_disassembly_saved;
                                                        assert(is_disassembly_saved);
                                                        if (is_disassembly_saved)
                                                        {
                                                            curr_shader_results_metadata.isa_disassembly = isa_output_filename;
                                                            std::cout << kStrInfoExtractRayTracingDisassemblySuccess << std::endl;
                                                        }
                                                        else
                                                        {
                                                            std::cerr << kStrErrorFailedToWriteOutputFile1 << "disassembly" <<
                                                                kStrErrorFailedToWriteOutputFile2 << isa_output_filename <<
                                                                kStrErrorFailedToWriteOutputFile3 << std::endl;
                                                        }
                                                    }
                                                }

                                                // Statistics files.
                                                if (!config.dxr_statistics_output.empty())
                                                {
                                                    // Generate an index-specific file name.
                                                    std::string stats_output_filename = GenerateSpecificFileName(config.dxr_statistics_output,
                                                        shader_output_file_suffix);
                                                    assert(!stats_output_filename.empty());
                                                    if (!stats_output_filename.empty())
                                                    {
                                                        // Save per-pipeline statistics.
                                                        if (is_unified[pipeline_index])
                                                        {
                                                            std::cout << kStrInfoExtractRayTracingResourceUsagePipelineByRaygen <<
                                                                curr_shader_results_metadata.export_name;
                                                        }
                                                        else
                                                        {
                                                            std::cout << kStrInfoExtractRayTracingResourceUsagePipelineByIndex1 << pipeline_index_display_name <<
                                                                kStrInfoExtractRayTracingResultByIndex2 << curr_shader_results_metadata.export_name;
                                                        }
                                                        std::cout << "..." << std::endl;

                                                        bool is_statistics_saved = SerializeDx12StatsRayTracing((*(*curr_results)[shader_index]), stats_output_filename);
                                                        assert(is_statistics_saved);
                                                        ret = ret && is_statistics_saved;
                                                        if (is_statistics_saved)
                                                        {
                                                            curr_shader_results_metadata.stats = stats_output_filename;
                                                            std::cout << kStrInfoExtractRayTracingStatisticsSuccess << std::endl;
                                                        }
                                                        else
                                                        {
                                                            std::cerr << kStrErrorFailedToWriteOutputFile1 << "statistics" <<
                                                                kStrErrorFailedToWriteOutputFile2 << stats_output_filename <<
                                                                kStrErrorFailedToWriteOutputFile3 << std::endl;
                                                        }
                                                    }
                                                }

                                                // Binary files: only if required by user and only once per pipeline.
                                                if (!config.dxr_binary_output.empty() && (shader_index == 0))
                                                {
                                                    assert(pipeline_index < pipeline_binaries.size());
                                                    if (pipeline_index < pipeline_binaries.size())
                                                    {
                                                        // Generate an index-specific file name.
                                                        std::string bin_output_filename = GenerateSpecificFileName(config.dxr_binary_output,
                                                            (is_unified[pipeline_index] ? curr_raygeneration_shader_name : pipeline_index_display_name));
                                                        assert(!bin_output_filename.empty());
                                                        if (!bin_output_filename.empty())
                                                        {
                                                            // Save per-pipeline binary.
                                                            if (is_unified[pipeline_index])
                                                            {
                                                                std::cout << kStrInfoExtractRayTracingPipelineBinaryByRaygen <<
                                                                    curr_raygeneration_shader_name;
                                                            }
                                                            else
                                                            {
                                                                std::cout << kStrInfoExtractRayTracingPipelineBinaryByIndex1 << pipeline_index_display_name;
                                                            }
                                                            std::cout << "..." << std::endl;

                                                            bool is_binary_saved = RgDx12Utils::WriteBinaryFile(bin_output_filename, *pipeline_binaries[pipeline_index]);
                                                            ret = ret && is_binary_saved;
                                                            assert(is_binary_saved);
                                                            if (is_binary_saved)
                                                            {
                                                                curr_pipeline_results_metadata.pipeline_binary = bin_output_filename;
                                                                std::cout << kStrInfoExtractRayTracingBinarySuccess << std::endl;
                                                            }
                                                            else
                                                            {
                                                                std::cout << kStrErrorFailedToWriteOutputFile1 << "binary" <<
                                                                    kStrErrorFailedToWriteOutputFile2 << bin_output_filename <<
                                                                    kStrErrorFailedToWriteOutputFile3 << std::endl;
                                                            }
                                                        }
                                                    }
                                                    else
                                                    {
                                                        bool is_multi_pipeline = (rs.size() > 1);
                                                        if(!is_multi_pipeline)
                                                        {
                                                            // Pipeline generation is supported for a single pipeline.
                                                            std::cerr << kStrErrorNoPipelineBinaryGeneratedForIndex << pipeline_index_display_name << std::endl;
                                                        }
                                                        else
                                                        {
                                                            // Pipeline generation is currently not supported by the driver for multiple pipelines.
                                                            std::cerr << kStrWarningBinaryExtractionNotSupportedMultiplePipelines1 <<
                                                                pipeline_index_display_name << kStrWarningBinaryExtractionNotSupportedMultiplePipelines2 << std::endl;
                                                        }
                                                    }
                                                }

                                                // Track current shader's results in the pipeline output metadata container.
                                                curr_pipeline_results_metadata.results.push_back(curr_shader_results_metadata);
                                            }

                                            // Track the current pipeline's results.
                                            results_pipeline_mode.push_back(curr_pipeline_results_metadata);
                                        }
                                    }
                                }

                                // Output metadata file.
                                if (!config.output_metadata.empty())
                                {
                                    // Write the output metadata file.
                                    RgDxrOutputMetadata::WriteOutputMetadata(config.output_metadata, results_pipeline_mode, error_msg);

#ifdef _DXR_TEST
                                    std::vector<RgDxrPipelineResults> test;
                                    RgDxrOutputMetadata::ReadOutputMetadata(config.output_metadata, test, error_msg);
#endif
                                }
                            }
                        }
                    }
                }
            }
        }
        else
        {
            should_abort = true;
        }

        return ret;
    }

    bool rgDx12Frontend::CompileRayTracingShader(const RgDx12Config& config, std::string& error_msg) const
    {
        bool ret = false;
        return ret;
    }
#endif
}
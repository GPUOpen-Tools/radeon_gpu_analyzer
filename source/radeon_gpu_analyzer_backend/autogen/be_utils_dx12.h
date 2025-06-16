//=============================================================================
/// Copyright (c) 2024-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for dx12 auto generation utilis.
//=============================================================================
#ifndef RGA_RADEONGPUANALYZERBACKEND_SRC_BE_UTILS_DX12_H_
#define RGA_RADEONGPUANALYZERBACKEND_SRC_BE_UTILS_DX12_H_

// CLI.
#include "source/radeon_gpu_analyzer_cli/kc_config.h"

// Local.
#include "radeon_gpu_analyzer_backend/be_include.h"
#include "radeon_gpu_analyzer_backend/autogen/be_include_dx12.h"

// C++
#include<string>

class BeDx12Utils
{
public:
    // Get Shader model prefix for givenpipelie stage.
    static std::string GetShaderModelPrefix(BePipelineStage stage);

    // Get Shader model prefix for given pipelie stage.
    static std::string GenerateShaderModel(const std::string& shader_model, const Config& config, BePipelineStage stage);

    // Get Name of the shader stage file, given stage.
    static bool GetShaderStageSourceFileName(const Config& config, const BePipelineStage& stage, std::string& source_filename);

    // Convert string to wstring.
    static std::wstring asCharArray(const std::string& str);

    // Convert wstring to string.
    static std::string asASCIICharArray(const std::wstring& wc_str);

    // Convert raw ptr to wchars.
    static const wchar_t* asCharArray(const void* ptr);

    // Convert raw ptr to chars.
    static const char* asASCIICharArray(const void* ptr);

    // Convert byte mask to component count.
    static bool MaskToComponentCount(BYTE mask, BYTE& component_count);

    // Check and convert HRESULT to beStatus.
    static beKA::beStatus CheckHr(HRESULT hr);

    // Given a file_path returns the absolute file path.
    static std::string GetAbsoluteFileName(const std::string& file_path);

    // Struct representing Dx12 Shader Model.
    struct ShaderModelVersion
    {
        int major = 0;
        int minor = 0;

        // If this shader model is a "legacy" shader model,
        // which means that it is supported by D3DCompileFromFile through DXBC rather than DXIL.
        inline bool IsBelowShaderModel_5_1() const
        {
            return (major < 5) || (major == 5 && minor < 1);
        }

        // Root signatures are still compiled via D3DCompile Function.
        // Shaders use dxc for compilation.
        inline bool IsShaderModel_5_1() const
        {
            return (major == 5 && minor == 1);
        }

        // Both root signature and shaders are compiled via dxc.
        inline bool IsAboveShaderModel_5_1() const
        {
            return (major > 5) || (major == 5 && minor > 1);
        }
    };

    // Parses the shader model string.
    // In case of a failure, for example, if the shader model format is invalid, false is return. 
    // Otherwise, true is returned.
    static bool ParseShaderModel(const std::string& shader_model_str, ShaderModelVersion& shader_model);
};

#endif  // RGA_RADEONGPUANALYZERBACKEND_SRC_BE_UTILS_DX12_H_

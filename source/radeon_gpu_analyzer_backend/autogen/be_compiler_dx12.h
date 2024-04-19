//=================================================================
// Copyright 2024 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================
#ifndef RGA_RADEONGPUANALYZERBACKEND_SRC_BE_COMPILER_DX12_H_
#define RGA_RADEONGPUANALYZERBACKEND_SRC_BE_COMPILER_DX12_H_

// CLI.
#include "source/radeon_gpu_analyzer_cli/kc_config.h"

// Local.
#include "radeon_gpu_analyzer_backend/autogen/be_include_dx12.h"

// C++.
#include <optional>

struct BeDx12CompilerOutput
{
    Microsoft::WRL::ComPtr<IDxcResult> dxc_result;
    bool                               has_root_signature = false;
};

class BeDx12Compiler
{
public:

    // Constructor. Loads dxc Library from config path.
    BeDx12Compiler(const Config& config, bool needs_dxc_compiler, std::stringstream& out);

    // Load Source code blob.
    bool LoadSrcBlob(const Config&           config,
                     BePipelineStage         stage,
                     bool                    has_blob,
                     bool                    has_source,
                     const std::string&      source_file_path,
                     BeDx12ShaderBinaryBlob& shader_blob,
                     bool&                   is_root_signature_specified,
                     std::stringstream&      err) const;

    // Get IDxcUtils
    IDxcUtils* GetUtils() const
    {
        return dxc_utils_.Get();
    }
    
    // Get IDxcCompiler3
    IDxcCompiler3* GetCompiler() const
    {
        return dxc_compiler_.Get();
    }

private:

    // Enable Debug Layer.
    static void EnableDebugLayer(const Config& config, std::stringstream& out);

    // Loads dxc library from dxc path.
    static std::optional<DxcCreateInstanceProc> LoadDxcLibrary(const Config& config, std::stringstream& out);

    // Loads dxc library from directory path.
    // If the function fails to load the library file, returns null.
    static std::optional<DxcCreateInstanceProc> LoadDxcLibraryFromDirectoryUtil(const wchar_t* dir);

    // Compiles shader src hlsl and detets root signature. 
    bool CompileSrc(const Config& config, BePipelineStage stage, const std::string& source_file_path, BeDx12CompilerOutput& dxc_output, std::stringstream& err) const;
    
    // Detect root signature in source_file_path.
    bool DetectRootSignatureInBlob(const std::string& source_file_path, bool& , std::stringstream& err) const;

    // Create vector of compiler args for dxc compiler.
    bool GetDxcCompilerArgs(const Config& config, const BePipelineStage& stage, std::vector<std::wstring>& compiler_args) const;

    // Get Model of the shader stage file, given stage.
    bool GetShaderStageModel(const Config& config, const BePipelineStage& stage, std::string& model_string) const;

    // Get Entry of the shader stage file, given stage.
    bool GetShaderStageEntry(const Config& config, const BePipelineStage& stage, std::string& entry_string) const;

    // IDxcUtils ptr;
    Microsoft::WRL::ComPtr<IDxcUtils>     dxc_utils_    = nullptr;

    // IDxcCompiler3 ptr;
    Microsoft::WRL::ComPtr<IDxcCompiler3> dxc_compiler_ = nullptr;
};

#endif  // RGA_RADEONGPUANALYZERBACKEND_SRC_BE_COMPILER_DX12_H_

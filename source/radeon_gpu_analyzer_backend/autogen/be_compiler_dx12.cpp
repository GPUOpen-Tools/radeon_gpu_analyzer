//======================================================================
// Copyright 2020-2024 Advanced Micro Devices, Inc. All rights reserved.
//======================================================================

// Local.
#include "radeon_gpu_analyzer_backend/autogen/be_compiler_dx12.h"
#include "radeon_gpu_analyzer_backend/autogen/be_utils_dx12.h"
#include "radeon_gpu_analyzer_backend/be_utils.h"
#include "radeon_gpu_analyzer_backend/autogen/be_include_handler_dxc.h"

// CLI.
#include "radeon_gpu_analyzer_cli/kc_utils.h"

// C++
#include <filesystem>
#include <cassert>

// D3D12.
#include <d3d12.h>

static const wchar_t* kDxCompilerFileName = L"dxcompiler.dll";
static const wchar_t* kDxIlFileName       = L"dxil.dll";
static const wchar_t* kDxcDefaultpath     = L"utils\\dx12\\dxc\\";

static const char* kStrDxcCreateInstance = "DxcCreateInstance";

static const char* kStrErrorDxcUtils = "Error: Failed to initialize DXC utils.\n";
static const char* kStrErrorDxcCompiler = "Error: Failed to initialize DXC compiler.\n";
static const char* kStrDebugLayerEnabled = "Info: enabled Debug Layer.\n";
static const char* kStrDebugLayerFailed = "Warning: failed to enable Debug Layer.\n";
static const char* kStrLoadDXCCompilerLibFailed = "Error: Failed to load DXC shader compiler library \"dxcompiler.dll\".\n";
static const char* kStrLoadDXCCompilerFallbackA = "Warning: could not find dxcompiler.dll in ";
static const char* kStrLoadDXCCompilerFallbackB = " falling back to loading the packaged version.\n";


BeDx12Compiler::BeDx12Compiler(const Config& config, bool needs_dxc_compiler, std::stringstream& out)
{
    EnableDebugLayer(config, out);

    std::optional<DxcCreateInstanceProc> dxc_create_instance_proc = LoadDxcLibrary(config, out);
    if (dxc_create_instance_proc.has_value() && dxc_create_instance_proc.value())
    {
        beKA::beStatus rc = BeDx12Utils::CheckHr(dxc_create_instance_proc.value()(CLSID_DxcUtils, IID_PPV_ARGS(&dxc_utils_)));
        if (rc == beKA::beStatus::kBeStatusSuccess)
        {
            if (needs_dxc_compiler)
            {
                Microsoft::WRL::ComPtr<IDxcCompiler> compiler;
                rc = BeDx12Utils::CheckHr(dxc_create_instance_proc.value()(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler)));
                if (rc == beKA::beStatus::kBeStatusSuccess)
                {
                    rc = BeDx12Utils::CheckHr(compiler->QueryInterface(IID_PPV_ARGS(&dxc_compiler_)));
                }
                else
                {
                    out << kStrErrorDxcCompiler;
                }
            }

        }
        else
        {
            out << kStrErrorDxcUtils;
        }
    }
}

bool BeDx12Compiler::LoadSrcBlob(const Config&           config,
                                 BePipelineStage         stage,
                                 bool                    has_blob,
                                 bool                    has_source,
                                 const std::string&      source_file_path,
                                 BeDx12ShaderBinaryBlob& shader_blob,
                                 bool&                   is_root_signature_specified,
                                 std::stringstream&      err) const
{
    bool ret = false;
    if (has_blob)
    {
        switch (stage)
        {
        case BePipelineStage::kVertex:
        case BePipelineStage::kFragment:
        case BePipelineStage::kCompute:
            ret = BeUtils::ReadBinaryFile(source_file_path, shader_blob);
            if (ret)
            {
                ret = DetectRootSignatureInBlob(source_file_path, is_root_signature_specified, err);
            }
            break;
        default:
            break;
        }
    }
    else if (has_source)
    {
        // Check if IDxcCompiler3 and IDxcUtils was initialized correctly.
        if (GetCompiler() != nullptr && GetUtils() != nullptr)
        {
            BeDx12CompilerOutput compiled_src_output;
            if (CompileSrc(config, stage, source_file_path, compiled_src_output, err))
            {
                if (compiled_src_output.has_root_signature)
                {
                    is_root_signature_specified = true;
                }

                Microsoft::WRL::ComPtr<IDxcBlob>     dxc_out_object_output;
                Microsoft::WRL::ComPtr<IDxcBlobWide> dxc_out_object_output_name;
                beKA::beStatus                       rc = BeDx12Utils::CheckHr(
                    compiled_src_output.dxc_result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&dxc_out_object_output), &dxc_out_object_output_name));
                if (rc == beKA::beStatus::kBeStatusSuccess)
                {
                    if (dxc_out_object_output && dxc_out_object_output->GetBufferPointer())
                    {
                        shader_blob.assign(static_cast<const char*>(dxc_out_object_output->GetBufferPointer()),
                                           static_cast<const char*>(dxc_out_object_output->GetBufferPointer()) + dxc_out_object_output->GetBufferSize());
                        ret = true;
                    }
                }
            }
        }
    }
    return ret;
}

void BeDx12Compiler::EnableDebugLayer(const Config& config, std::stringstream& out)
{
    if (config.dx12_debug_layer_enabled)
    {
        Microsoft::WRL::ComPtr<ID3D12Debug> debug_controller;
        beKA::beStatus                      rc = BeDx12Utils::CheckHr(D3D12GetDebugInterface(IID_PPV_ARGS(&debug_controller)));
        if (rc == beKA::beStatus::kBeStatusSuccess)
        {
            debug_controller->EnableDebugLayer();
            out << kStrDebugLayerEnabled;
        }
        else
        {
            out << kStrDebugLayerFailed;
        }
    }
}

std::optional<DxcCreateInstanceProc> BeDx12Compiler::LoadDxcLibrary(const Config& config, std::stringstream& out)
{
    std::optional<DxcCreateInstanceProc> result = std::nullopt;

    // Try specified location. It may fail.
    if (!config.dxc_path.empty())
    {
        result = BeDx12Compiler::LoadDxcLibraryFromDirectoryUtil(BeDx12Utils::asCharArray(config.dxc_path).c_str());
        if (!result.has_value())
        {
            out << kStrLoadDXCCompilerFallbackA << config.dxc_path << kStrLoadDXCCompilerFallbackB;            
        }
    }

    if (!result.has_value())
    {
        // Try default location. It must succeed to proceed further with the program.
        result = BeDx12Compiler::LoadDxcLibraryFromDirectoryUtil(L"");
        if (!result.has_value())
        {
            out << kStrLoadDXCCompilerLibFailed;
        }
    }

    return result;
}

std::optional<DxcCreateInstanceProc> BeDx12Compiler::LoadDxcLibraryFromDirectoryUtil(const wchar_t* dir)
{
    std::optional<DxcCreateInstanceProc> result;
    std::wstring dxcompiler_path_str;
    std::wstring dxil_path_str;
    if (dir && *dir)
    {
        const std::filesystem::path dxc_dir = std::filesystem::path(dir);
        dxcompiler_path_str                 = (dxc_dir / kDxCompilerFileName).c_str();
        dxil_path_str                       = (dxc_dir / kDxIlFileName).c_str();
    }
    else
    {
        std::wstringstream dxc_wss, dxil_wss;
        dxc_wss << kDxcDefaultpath << kDxCompilerFileName;
        dxcompiler_path_str = dxc_wss.str();
        dxil_wss << kDxcDefaultpath << kDxIlFileName;
        dxil_path_str = dxil_wss.str();
    }

    // Load module but ignore the result.
    // Loading "dxil.dll" must happen before "dxcompiler.dll" to pick the right one!
    LoadLibraryW(dxil_path_str.c_str());

    const HMODULE dxcompiler_module = LoadLibraryW(dxcompiler_path_str.c_str());
    if (dxcompiler_module)
    {
        result = (DxcCreateInstanceProc)GetProcAddress(dxcompiler_module, kStrDxcCreateInstance);
        if (!(result.has_value() && result.value()))
        {
            result = std::nullopt;
        }
    }
    else
    {
        result = std::nullopt;
    }
    return result;
}

bool BeDx12Compiler::CompileSrc(const Config&         config,
                                BePipelineStage       stage,
                                const std::string&    source_file_path,
                                BeDx12CompilerOutput& dxc_output,
                                std::stringstream&    err) const
{
    bool ret = false;
    // Check if IDxcCompiler3 and IDxcUtils was initialized correctly.
    if (GetCompiler() != nullptr && GetUtils() != nullptr)
    {
        Microsoft::WRL::ComPtr<IDxcBlobEncoding> shader_source;
        beKA::beStatus status = BeDx12Utils::CheckHr((dxc_utils_->LoadFile(BeDx12Utils::asCharArray(source_file_path).c_str(), nullptr, &shader_source)));
        if (status == beKA::beStatus::kBeStatusSuccess)
        {
            DxcBuffer shader_source_buffer = {};
            shader_source_buffer.Ptr       = shader_source->GetBufferPointer();
            shader_source_buffer.Size      = shader_source->GetBufferSize();
            shader_source_buffer.Encoding  = 0;

            BeDxcIncludeHandler dx12_include_handler{GetUtils(), source_file_path, config.include_path};
            std::vector<std::wstring> dxc_compiler_args;
            if (dx12_include_handler.InitIncludeHandler() && GetDxcCompilerArgs(config, stage, dxc_compiler_args))
            {
                std::vector<const wchar_t*> arguments_vector;
                arguments_vector.reserve(dxc_compiler_args.size());
                std::transform(dxc_compiler_args.begin(), dxc_compiler_args.end(), std::back_inserter(arguments_vector), [](const std::wstring& wstr) {
                    return wstr.c_str();
                });

                status = BeDx12Utils::CheckHr(dxc_compiler_->Compile(&shader_source_buffer,
                                                                     arguments_vector.data(),
                                                                     static_cast<uint32_t>(arguments_vector.size()),
                                                                     &dx12_include_handler,
                                                                     IID_PPV_ARGS(&dxc_output.dxc_result)));
                if (status == beKA::beStatus::kBeStatusSuccess)
                {
                    Microsoft::WRL::ComPtr<IDxcBlobEncoding> dxc_output_text;
                    Microsoft::WRL::ComPtr<IDxcBlob>         dxc_output_binary;
                    Microsoft::WRL::ComPtr<IDxcBlobWide>     dxc_output_name;
                    status = BeDx12Utils::CheckHr(dxc_output.dxc_result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&dxc_output_binary), &dxc_output_name));
                    if (status == beKA::beStatus::kBeStatusSuccess)
                    {
                        ret = true;
                        if (dxc_output_binary->GetBufferSize() == 0)
                        {
                            ret = false;
                        }

                        if (ret)
                        {
                            status = BeDx12Utils::CheckHr(dxc_output.dxc_result->GetOutput(DXC_OUT_ROOT_SIGNATURE, IID_PPV_ARGS(&dxc_output_binary), &dxc_output_name));
                            if (status == beKA::beStatus::kBeStatusSuccess && dxc_output_binary->GetBufferSize() > 0)
                            {
                                dxc_output.has_root_signature = true;
                            }
                        }
                    }
                    else
                    {
                        ret = false;
                    }

                    status = BeDx12Utils::CheckHr(dxc_output.dxc_result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&dxc_output_text), &dxc_output_name));
                    if (status == beKA::beStatus::kBeStatusSuccess)
                    {
                        if (dxc_output_text && dxc_output_text->GetBufferPointer())
                        {
                            std::string dxc_out_errors_string{static_cast<const char*>(dxc_output_text->GetBufferPointer())};
                            err << dxc_out_errors_string << "\n";
                            if (dxc_out_errors_string.find("error") != std::string::npos)
                            {
                                ret = false;
                            }
                        }
                    }
                    else
                    {
                        ret = false;
                    }
                }
            }            
        }
    }
    return ret;
}

bool BeDx12Compiler::DetectRootSignatureInBlob(const std::string& source_file_path, bool& is_root_signature_specified, std::stringstream& err) const
{
    bool ret = false;
    // Check if IDxcUtils was initialized.
    if (GetUtils() != nullptr)
    {
        Microsoft::WRL::ComPtr<IDxcBlobEncoding> shader_source;
        beKA::beStatus status = BeDx12Utils::CheckHr((dxc_utils_->LoadFile(BeDx12Utils::asCharArray(source_file_path).c_str(), nullptr, &shader_source)));
        if (status == beKA::beStatus::kBeStatusSuccess)
        {
            DxcBuffer shader_source_buffer = {};
            shader_source_buffer.Ptr       = shader_source->GetBufferPointer();
            shader_source_buffer.Size      = shader_source->GetBufferSize();
            void*         part_data        = nullptr;
            uint32_t      part_size        = 0;
            const HRESULT hr               = dxc_utils_->GetDxilContainerPart(&shader_source_buffer, DXC_PART_ROOT_SIGNATURE, &part_data, &part_size);
            is_root_signature_specified    = SUCCEEDED(hr);
            ret                            = true;
        }
    }
    return ret;
}

bool BeDx12Compiler::GetDxcCompilerArgs(const Config& config, const BePipelineStage& stage, std::vector<std::wstring>& compiler_args) const
{
    std::string model_str;
    bool        is_model_ok = GetShaderStageModel(config, stage, model_str);
    if (is_model_ok)
    {
        BeDx12Utils::ShaderModelVersion model_version;
        bool                            is_shader_model_valid = BeDx12Utils::ParseShaderModel(model_str, model_version);
        if (!is_shader_model_valid || !model_version.IsAboveShaderModel_5_1())
        {
            is_model_ok = false;
        }
        if (is_model_ok)
        {
            std::stringstream ss;
            ss << "-T " << model_str;
            compiler_args.push_back(BeDx12Utils::asCharArray(ss.str()));
        }
    }

    std::string entry_str;
    bool        is_entry_ok = GetShaderStageEntry(config, stage, entry_str);
    if (is_entry_ok)
    {
        std::stringstream ss;
        ss << "-E " << entry_str;
        compiler_args.push_back(BeDx12Utils::asCharArray(ss.str()));
    }

    for (const auto& define : config.defines)
    {
        std::stringstream ss;
        ss << "-D " << define;
        compiler_args.push_back(BeDx12Utils::asCharArray(ss.str()));
    }

    if (!config.dxc_opt.empty())
    {
        std::stringstream ss;
        ss << config.dxc_opt;
        compiler_args.push_back(BeDx12Utils::asCharArray(ss.str()));
    }

    return is_model_ok && is_entry_ok;
}

bool BeDx12Compiler::GetShaderStageModel(const Config& config, const BePipelineStage& stage, std::string& model_string) const
{
    bool ret = true;
    switch (stage)
    {
    case BePipelineStage::kVertex:
        model_string = BeDx12Utils::GenerateShaderModel(config.vs_model, config, BePipelineStage::kVertex);
        break;
    case BePipelineStage::kFragment:
        model_string = BeDx12Utils::GenerateShaderModel(config.ps_model, config, BePipelineStage::kFragment);
        break;
    case BePipelineStage::kCompute:
        model_string = BeDx12Utils::GenerateShaderModel(config.cs_model, config, BePipelineStage::kCompute);
        break;
    default:
        assert(false);
        ret = false;
    }
    return ret;
}

bool BeDx12Compiler::GetShaderStageEntry(const Config& config, const BePipelineStage& stage, std::string& entry_string) const
{
    bool ret = true;
    switch (stage)
    {
    case BePipelineStage::kVertex:
        entry_string = config.vs_entry_point;
        break;
    case BePipelineStage::kFragment:
        entry_string = config.ps_entry_point;
        break;
    case BePipelineStage::kCompute:
        entry_string = config.cs_entry_point;
        break;
    default:
        ret = false;
    }
    return ret;
}
//=================================================================
// Copyright 2024 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

// Local.
#include "radeon_gpu_analyzer_backend/autogen/be_include_handler_dxc.h"
#include "radeon_gpu_analyzer_backend/autogen/be_utils_dx12.h"

// C++
#include <cassert>


BeDxcIncludeHandler::BeDxcIncludeHandler(IDxcUtils* utils, const std::string& shader_file_path, const std::vector<std::string>& include_paths)
    : dxc_utils_{utils}
    , shader_file_directory_{std::filesystem::path{BeDx12Utils::asCharArray(shader_file_path).c_str()}.parent_path()}
    , include_paths_(include_paths)
{}

HRESULT STDMETHODCALLTYPE BeDxcIncludeHandler::LoadSource(_In_z_ LPCWSTR pFilename, _COM_Outptr_result_maybenull_ IDxcBlob** ppIncludeSource)
{
    if (!pFilename || !ppIncludeSource)
    {
        return E_INVALIDARG;
    }

    *ppIncludeSource = nullptr;
    if (default_include_handler_)
    {
        HRESULT hr = default_include_handler_->LoadSource(pFilename, ppIncludeSource);
        if (SUCCEEDED(hr) && *ppIncludeSource)
        {
            return S_OK;
        }

        std::filesystem::path file_name = pFilename;
        if (!file_name.is_absolute())
        {
            if (TryInclude(shader_file_directory_, file_name, ppIncludeSource))
            {
                return S_OK;
            }

            for (const auto& include_dir : include_paths_)
            {
                if (TryInclude(include_dir, file_name, ppIncludeSource))
                {
                    return S_OK;
                }
            }
        }
    }

    return E_FAIL;
}


bool BeDxcIncludeHandler::InitIncludeHandler()
{
    bool ret = true;
    if (!dxc_utils_)
    {
        ret = false;
    }

    if (ret)
    {
        if (BeDx12Utils::CheckHr(dxc_utils_->CreateDefaultIncludeHandler(&default_include_handler_)) != beKA::beStatus::kBeStatusSuccess)
        {
            ret = false;
        }

        if (ret && !default_include_handler_)
        {
            ret = false;
        }
    }

    return ret;
}

bool BeDxcIncludeHandler::TryInclude(const std::filesystem::path& directory, const std::filesystem::path& file_name, IDxcBlob** ppIncludeSource)
{
    if (!ppIncludeSource || !dxc_utils_)
    {
        return false;
    }

    const std::filesystem::path full_path = directory / file_name;
    if (std::filesystem::exists(full_path))
    {
        uint32_t          code_page = DXC_CP_ACP;
        IDxcBlobEncoding* blobEncoding;
        if (SUCCEEDED(dxc_utils_->LoadFile(full_path.c_str(), &code_page, &blobEncoding)))
        {
            *ppIncludeSource = blobEncoding;
            return true;
        }
    }

    return false;
}

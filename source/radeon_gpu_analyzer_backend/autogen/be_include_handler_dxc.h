//=============================================================================
/// Copyright (c) 2024-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for dxc include handler.
//=============================================================================

#ifndef RGA_RADEONGPUANALYZERBACKEND_SRC_BE_INCLUDE_HANDLER_DXC_H_
#define RGA_RADEONGPUANALYZERBACKEND_SRC_BE_INCLUDE_HANDLER_DXC_H_

// Local.
#include "radeon_gpu_analyzer_backend/autogen/be_include_dx12.h"

// C++.
#include <filesystem>

class BeDxcIncludeHandler : public IDxcIncludeHandler
{
public:
    BeDxcIncludeHandler(IDxcUtils* utils, const std::string& shader_file_path, const std::vector<std::string>& include_paths);

    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID, void**)
    {
        return E_FAIL;
    }
    virtual ULONG STDMETHODCALLTYPE AddRef(void)
    {
        return S_OK;
    }
    virtual ULONG STDMETHODCALLTYPE Release(void)
    {
        return S_OK;
    }

    virtual HRESULT STDMETHODCALLTYPE LoadSource(_In_z_ LPCWSTR pFilename, _COM_Outptr_result_maybenull_ IDxcBlob** ppIncludeSource) override;

    // Initializes IDxcIncludeHandler ptr.
    // returns false if the initialization fails.
    bool InitIncludeHandler(); 

private:
    IDxcUtils*                                 dxc_utils_;
    std::filesystem::path                      shader_file_directory_;
    const std::vector<std::string>&            include_paths_;
    Microsoft::WRL::ComPtr<IDxcIncludeHandler> default_include_handler_;

    bool TryInclude(const std::filesystem::path& directory, const std::filesystem::path& file_name, IDxcBlob** ppIncludeSource);
};

#endif  // RGA_RADEONGPUANALYZERBACKEND_SRC_BE_INCLUDE_HANDLER_DXC_H_

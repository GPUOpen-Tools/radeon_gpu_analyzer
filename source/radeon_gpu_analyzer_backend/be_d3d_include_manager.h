//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for rga backend d3d include manager.
//=============================================================================

#ifdef _WIN32
#ifndef RGA_RADEONGPUANALYZERBACKEND_SRC_BE_D3D_INCLUDE_MANAGER_H_
#define RGA_RADEONGPUANALYZERBACKEND_SRC_BE_D3D_INCLUDE_MANAGER_H_

// D3D.
#include <d3dcommon.h>

// C++.
#include <vector>
#include <string>

class D3dIncludeManager :
    public ID3DInclude
{
public:
    D3dIncludeManager(const std::string& shader_dir, const std::vector<std::string>& include_search_dirs);
    virtual ~D3dIncludeManager();
    virtual STDMETHODIMP Open(THIS_ D3D_INCLUDE_TYPE include_type, LPCSTR filename, LPCVOID parent_data, LPCVOID* ppData, UINT* pBytes);
    virtual STDMETHODIMP Close(THIS_ LPCVOID pBytes);

private:
    std::string shader_dir_;
    std::vector<std::string> include_search_dirs_;
};

#endif // RGA_RADEONGPUANALYZERBACKEND_SRC_BE_D3D_INCLUDE_MANAGER_H_
#endif

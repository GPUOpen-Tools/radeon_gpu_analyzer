//=================================================================
// Copyright 2017 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

// std
#include <vector>
#include <iostream>

// dx
#include <d3d11.h>
#include <dxgi.h>

// local
#include "rga_dx11_string_constants.h"

//
// Gets the names of supported display adapters installed on the system.
// The names are returned in the "adapterNames" vector.
// Returns "true" if succeeded or "false" otherwise.
//
static bool  GetSupportedAdapterNames(std::vector<std::wstring>& adapterNames)
{
    bool  ret = true;
    IDXGIFactory*  pFactory = nullptr;

    if (CreateDXGIFactory(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&pFactory)) == S_OK)
    {
        IDXGIAdapter* pAdapter;
        for (int i = 0; pFactory->EnumAdapters(i, &pAdapter) != DXGI_ERROR_NOT_FOUND; i++)
        {
            DXGI_ADAPTER_DESC  adapterDesc;
            if (pAdapter != nullptr && pAdapter->GetDesc(&adapterDesc) == S_OK)
            {
                std::wstring  name = adapterDesc.Description;
                if (name.find(STR_AMD_DISPLAY_ADAPTER_TOKEN_1) != std::wstring::npos ||
                    name.find(STR_AMD_DISPLAY_ADAPTER_TOKEN_2) != std::wstring::npos)
                {
                    adapterNames.push_back(name);
                }
            }
            else
            {
                ret = false;
                break;
            }
        }
        pFactory->Release();
    }
    else
    {
        ret = false;
    }

    return ret;
}

//
// Dumps the names of supported display adapters installed on the system.
// Returns "true" if succeeded or "false" otherwise.
//
static bool  DumpSupportedNames()
{
    std::vector<std::wstring>  adapterNames;
    bool  ret = false;

    if (GetSupportedAdapterNames(adapterNames))
    {
        for (const std::wstring& name : adapterNames)
        {
            std::wcout << name << std::endl;
        }
        ret = true;
    }

    return ret;
}

//
// Gets the name of display adapter name and full path to corresponding driver dll for the adapter specified by "adapterID".
// Returns "true" if succeeded or "false" otherwise.
//
static bool  GetAdapterInfo(int adapterID, std::wstring& adapterName, std::wstring& dxxModulePath)
{
    bool  result = true;
    IDXGIFactory*  pFactory = nullptr;
    
    if (CreateDXGIFactory(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&pFactory)) == S_OK)
    {
        IDXGIAdapter*      pAdapter;
        std::vector<IDXGIAdapter*>  amdAdapters;
        ID3D11Device*      pDevice;
        HMODULE            dll;
        const int          dllNameLen = 1024;
        wchar_t            dllFullName[dllNameLen];

        // Gather AMD adapters.
        for (int i = 0; pFactory->EnumAdapters(i, &pAdapter) != DXGI_ERROR_NOT_FOUND; i++)
        {
            DXGI_ADAPTER_DESC  adapterDesc;
            if (pAdapter != nullptr && pAdapter->GetDesc(&adapterDesc) == S_OK)
            {
                std::wstring  name = adapterDesc.Description;
                if (name.find(STR_AMD_DISPLAY_ADAPTER_TOKEN_1) != std::wstring::npos ||
                    name.find(STR_AMD_DISPLAY_ADAPTER_TOKEN_2) != std::wstring::npos)
                {
                    amdAdapters.push_back(pAdapter);
                    adapterName = name;
                }
            }
            else
            {
                result = false;
                break;
            }
        }

        if (adapterID >= amdAdapters.size())
        {
            result = false;
        }

        // Create a DX device for the Adapter with requested ID.
        if (result)
        {
            result = (D3D11CreateDevice(amdAdapters[adapterID], D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_UNKNOWN,
                                        nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &pDevice, nullptr, nullptr) == S_OK);
        }

        if (result)
        {
            std::wstring  dxxDllName = STR_AMD_DXX_MODULE_NAME;
            dll = GetModuleHandle(dxxDllName.c_str());
            result = (dll != nullptr);
        }

        if (result)
        {
            result = (GetModuleFileName(dll, dllFullName, dllNameLen) != 0);
        }

        if (result)
        {
            pDevice->Release();
            dxxModulePath = dllFullName;
        }

        pFactory->Release();
    }
    else
    {
        result = false;
    }

    return result;
}

//
// Dumps the information obtained for the display adapter specified by its ID.
// Returns "true" if succeeded or "false" otherwise.
//
static bool  DumpAdapterInfo(int ID)
{
    std::wstring  adapterName, dxxModulePath;
    bool  ret = false;

    if (GetAdapterInfo(ID, adapterName, dxxModulePath))
    {
        std::wcout << adapterName << std::endl << dxxModulePath << std::endl;
        ret = true;
    }

    return ret;
}

int main(int argc, char* argv[])
{
    bool  done    = false;
    bool  result = 0;

    if (argc == 2)
    {
        if (std::string(argv[1]) == "--list-adapters")
        {
            result = DumpSupportedNames();
            if (!result)
            {
                std::wcout << STR_ERR_GET_DISPLAY_ADAPTERS_FAILED << std::endl;
            }
            done = true;
        }
    }
    else if (argc == 3)
    {
        if (std::string(argv[1]) == "--get-adapter-info")
        {
            std::string  idStr = argv[2];
            int  ID;
            try
            {
                ID = std::stoi(idStr);
            }
            catch (...)
            {
                ID = -1;
                std::wcout << STR_ERR_BAD_DISPLAY_ADAPTER_ID << std::wstring(idStr.begin(), idStr.end()) << std::endl;
            }

            if (ID >= 0)
            {
                result = DumpAdapterInfo(ID);
                if (!result)
                {
                    std::wcout << STR_ERR_GET_DISPLAY_ADAPTER_INFO_FAILED << ID << std::endl;
                }
                done = true;
            }
        }
    }
    
    if (!done)
    {
        std::cout << USAGE_MESSAGE;
    }

    return (result ? 0 : 1);
}

//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for dx12 and dxr factory.
//=============================================================================

#pragma once

// C++.
#include <vector>

// DX.
#include <dxgi.h>
#include <dxgi1_2.h>
#include <dxgi1_3.h>
#include <dxgi1_4.h>
#include <wrl.h>
#include "d3dx12.h"
using namespace Microsoft::WRL;

// Local.
#include "rg_dx12_data_types.h"

namespace rga
{
    class RgDx12Factory
    {
    public:
        RgDx12Factory() = delete;
        ~RgDx12Factory() = delete;

        // Create a hit group subobject.
        static void CreateHitGroupSubobject(CD3DX12_STATE_OBJECT_DESC* state_object_x, const RgDxrHitGroup& hit_group_data);

        // Create a shader config subobject.
        static void CreateShaderConfigSubobject(CD3DX12_STATE_OBJECT_DESC* state_object_x, uint32_t payload_size, uint32_t attribute_size);

        // Create a pipeline config subobject.
        static void CreatePipelineConfigSubobject(CD3DX12_STATE_OBJECT_DESC* state_object_x, uint32_t max_recursion_depth);

        // Create a subobjects-to-exports association subobject.
        static D3D12_STATE_SUBOBJECT* CreateSubobjectToExportsAssociationSubobject(const D3D12_STATE_SUBOBJECT* subobject_to_associate, const std::vector<std::wstring>& exports);

        // Create a local root signature subobject.
        static void CreateLocalRootSignatureSubobject(CD3DX12_STATE_OBJECT_DESC* state_object_x, ID3D12RootSignature* root_signature, const std::vector<std::wstring>& exports);

        // Create a global root signature subobject.
        static void CreateGlobalRootSignatureSubobject(CD3DX12_STATE_OBJECT_DESC* state_object_x, ID3D12RootSignature* root_signature);

        // Create a DXIL library subobject.
        // Upon a failure, nullptr is returned and an error message is set to error_msg.
        static void CreateDxilLibrarySubobject(CD3DX12_STATE_OBJECT_DESC* state_object_desc_x, const RgDxrDxilLibrary& dxil_lib_data, std::string& error_msg);

        // Destroy a hit group subobject that was created by this factory.
        static void DestroyHitGroupSubobject(D3D12_STATE_SUBOBJECT*& hit_group_subobject);
    };
}

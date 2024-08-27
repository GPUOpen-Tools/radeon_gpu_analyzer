// C++.
#include <string>
#include <vector>
#include <cassert>
#include <sstream>
#include <algorithm>

// Local.
#include "rg_dx12_factory.h"
#include "rg_dx12_utils.h"
#include "rg_dx12_utils.h"

namespace rga
{
    void RgDx12Factory::CreateHitGroupSubobject(CD3DX12_STATE_OBJECT_DESC* state_object_x, const RgDxrHitGroup& hit_group_data)
    {
        auto hit_group = state_object_x->CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
        hit_group->SetHitGroupType(hit_group_data.type);
        if (!hit_group_data.intersection_shader.empty())
        {
            hit_group->SetIntersectionShaderImport(hit_group_data.intersection_shader.c_str());
        }
        if (!hit_group_data.any_hit_shader.empty())
        {
            hit_group->SetAnyHitShaderImport(hit_group_data.any_hit_shader.c_str());
        }
        if (!hit_group_data.closest_hit_shader.empty())
        {
            hit_group->SetClosestHitShaderImport(hit_group_data.closest_hit_shader.c_str());
        }
        if (!hit_group_data.hitgroup_name.empty())
        {
            hit_group->SetHitGroupExport(hit_group_data.hitgroup_name.c_str());
        }
    }

    void RgDx12Factory::CreateShaderConfigSubobject(CD3DX12_STATE_OBJECT_DESC* state_object_x,
        uint32_t payload_size, uint32_t attribute_size)
    {
        auto shader_config = state_object_x->CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
        shader_config->Config(payload_size, attribute_size);
    }

    void RgDx12Factory::CreatePipelineConfigSubobject(CD3DX12_STATE_OBJECT_DESC* state_object_x, uint32_t max_recursion_depth)
    {
        auto pipeline_config = state_object_x->CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();
        pipeline_config->Config(max_recursion_depth);
    }

    D3D12_STATE_SUBOBJECT* RgDx12Factory::CreateSubobjectToExportsAssociationSubobject(const D3D12_STATE_SUBOBJECT* subobject_to_associate,
        const std::vector<std::wstring>& exports)
    {
        D3D12_STATE_SUBOBJECT* pRet = new D3D12_STATE_SUBOBJECT{};
        pRet->Type = D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION;

        // Descriptor.
        D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION* desc = new D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION{};
        pRet->pDesc = desc;
        desc->pSubobjectToAssociate = subobject_to_associate;
        const uint32_t num_exports = static_cast<uint32_t>(exports.size());
        desc->NumExports = num_exports;
        desc->pExports = new LPCWSTR[num_exports]{};
        for (uint32_t i = 0; i < num_exports; i++)
        {
            desc->pExports[i] = exports[i].c_str();
        }
        return pRet;
    }

    void RgDx12Factory::CreateLocalRootSignatureSubobject(CD3DX12_STATE_OBJECT_DESC* state_object_x,
        ID3D12RootSignature* root_signature, const std::vector<std::wstring>& exports)
    {
        // Create the local root signature subobject.
        auto local_root_signature = state_object_x->CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
        local_root_signature->SetRootSignature(root_signature);

        // Define the associations.
        auto root_signature_association = state_object_x->CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
        root_signature_association->SetSubobjectToAssociate(*local_root_signature);
        for (const std::wstring& export_name : exports)
        {
            root_signature_association->AddExport(export_name.c_str());
        }
    }

    void RgDx12Factory::CreateGlobalRootSignatureSubobject(CD3DX12_STATE_OBJECT_DESC* state_object_x,
        ID3D12RootSignature* root_signature)
    {
        auto global_root_signature = state_object_x->CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();
        global_root_signature->SetRootSignature(root_signature);
    }

    void RgDx12Factory::CreateDxilLibrarySubobject(CD3DX12_STATE_OBJECT_DESC* state_object_desc_x,
        const RgDxrDxilLibrary& dxil_lib_data, std::string&)
    {
        assert(!dxil_lib_data.binary_data.empty());
        if (!dxil_lib_data.binary_data.empty())
        {
            auto lib = state_object_desc_x->CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
            D3D12_SHADER_BYTECODE libdxil = CD3DX12_SHADER_BYTECODE((void*)dxil_lib_data.binary_data.data(),
                dxil_lib_data.binary_data.size());
            lib->SetDXILLibrary(&libdxil);

            // Add the relevant exports for this library.
            const size_t num_exports = dxil_lib_data.exports.size();
            for (size_t i = 0; i < num_exports; i++)
            {
                if (!dxil_lib_data.exports[i].export_name.empty())
                {
                    if (dxil_lib_data.exports[i].linkage_name.empty())
                    {
                        lib->DefineExport(dxil_lib_data.exports[i].export_name.c_str());
                    }
                    else
                    {
                        lib->DefineExport(dxil_lib_data.exports[i].linkage_name.c_str(), dxil_lib_data.exports[i].export_name.c_str());
                    }
                }
            }
        }
    }

    void RgDx12Factory::DestroyHitGroupSubobject(D3D12_STATE_SUBOBJECT*& hit_group_subobject)
    {
        if (hit_group_subobject != nullptr)
        {
            if (hit_group_subobject->pDesc != nullptr)
            {
                D3D12_HIT_GROUP_DESC* pDesc = (D3D12_HIT_GROUP_DESC*)(hit_group_subobject->pDesc);
                delete[]pDesc->IntersectionShaderImport;
                pDesc->IntersectionShaderImport = nullptr;
                delete[]pDesc->AnyHitShaderImport;
                pDesc->AnyHitShaderImport = nullptr;
                delete[]pDesc->ClosestHitShaderImport;
                pDesc->ClosestHitShaderImport = nullptr;

                delete hit_group_subobject->pDesc;
                hit_group_subobject->pDesc = nullptr;
            }
            delete hit_group_subobject;
            hit_group_subobject = nullptr;
       }
    }

    void RgDx12Factory::DestroyDxilLibrarySubobject(D3D12_STATE_SUBOBJECT*& dxil_library_subobject)
    {
        if (dxil_library_subobject != nullptr)
        {
            if (dxil_library_subobject->pDesc != nullptr)
            {
                D3D12_DXIL_LIBRARY_DESC* desc = (D3D12_DXIL_LIBRARY_DESC*)(dxil_library_subobject->pDesc);
                desc->DXILLibrary.BytecodeLength = 0;
                delete[] desc->DXILLibrary.pShaderBytecode;
                desc->DXILLibrary.pShaderBytecode = nullptr;
                for (uint32_t i = 0; i < desc->NumExports; i++)
                {
                    desc->pExports[i];
                    delete[] desc->pExports[i].Name;
                    ((D3D12_EXPORT_DESC*)desc->pExports)[i].Name = nullptr;
                    delete[] desc->pExports[i].ExportToRename;
                    ((D3D12_EXPORT_DESC*)desc->pExports)[i].ExportToRename = nullptr;
                }
                delete[] desc->pExports;
                desc->pExports = nullptr;
            }

            delete dxil_library_subobject;
            dxil_library_subobject = nullptr;
        }
    }

}

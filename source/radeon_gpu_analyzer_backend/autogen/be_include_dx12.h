//=================================================================
// Copyright 2024 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

#ifndef RGA_RADEONGPUANALYZERBACKEND_SRC_BE_INCLUDE_DX12_H_
#define RGA_RADEONGPUANALYZERBACKEND_SRC_BE_INCLUDE_DX12_H_

#include <unordered_set>

#include "utils/dx12/backend/d3dx12.h"

#include "radeon_gpu_analyzer_backend/be_include.h"
#include "radeon_gpu_analyzer_backend/be_data_types.h"

#include <dxcapi.h>
#include <d3d12shader.h>

const std::string kPSEntryPoint  = "main";
const std::string kVSEntryPoint  = "main";
const std::string kRootSignature = "RGA_ROOT_SIGNATURE";

// Constants other.
static const char* kStrTemplateGpsoFileContent =
    "# schemaVersion\n1.0\n\n# InputLayoutNumElements (the number of "
    "D3D12_INPUT_ELEMENT_DESC elements in the D3D12_INPUT_LAYOUT_DESC structure - must match the following \"InputLayout\" "
    "section)\n2\n\n# InputLayout ( {SemanticName, SemanticIndex, Format, InputSlot, AlignedByteOffset, InputSlotClass, "
    "InstanceDataStepRate } )\n{ \"POSITION\", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,"
    "0 },\n{ \"COLOR\", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }\n\n#"
    "PrimitiveTopologyType (the D3D12_PRIMITIVE_TOPOLOGY_TYPE value to be used when creating the PSO)\n"
    "D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE\n\n# NumRenderTargets (the number of formats in the upcoming RTVFormats section)"
    "\n2\n\n# # RTVFormats { RTVFormat1, RTVFormat2, ... , RTVFormatNumRenderTargets } (an array of DXGI_FORMAT-typed values "
    "for the render target formats - the number of items in the array must match the above NumRenderTargets section)\n{ DXGI_FORMAT_R8G8B8A8_UNORM }";

// Autogeneration status.
enum class BeDx12AutoGenStatus
{
    kNotRequired = 0,
    kRequired,
    kFailed,
    kSuccess
};

using BeDx12ShaderBinaryBlob = std::vector<char>;

// Inputs for the DXC APIs.
struct BeDx12AutoGenInput
{
    BeDx12ShaderBinaryBlob                    vs_blob;
    BeDx12ShaderBinaryBlob                    ps_blob;
    BeDx12ShaderBinaryBlob                    cs_blob;
    bool                                      is_root_signature_specified = false;
    bool                                      is_gpso_specified           = false;
    const D3D12_GRAPHICS_PIPELINE_STATE_DESC* parsed_gpso_file            = nullptr;
};

// DXC autogenerated file data.
struct BeDx12AutoGenFile
{
    BeDx12AutoGenStatus status = BeDx12AutoGenStatus::kNotRequired;
    std::string         filename;
};

// DXC autogenerated pipeline data.
struct BeDx12AutoGenPipelineInfo
{
    BeDx12AutoGenStatus             autogen_status = BeDx12AutoGenStatus::kNotRequired;
    std::string                     autogen_dir;
    BeDx12AutoGenInput              source_code;
    BeDx12AutoGenFile               root_signature;
    BeDx12AutoGenFile               gpso_file;
    BeDx12AutoGenFile               vertex_shader;
    BeDx12AutoGenFile               pixel_shader;
    std::stringstream               dxc_out;
    bool                            should_retain_temp_files = false;
    std::unordered_set<std::string> other_temporary_files;  // isa, stats, etc.
};

#endif  // RGA_RADEONGPUANALYZERBACKEND_SRC_BE_INCLUDE_DX12_H_

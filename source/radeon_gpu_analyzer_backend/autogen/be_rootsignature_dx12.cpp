//=================================================================
// Copyright 2024 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

// Local.
#include "radeon_gpu_analyzer_backend/autogen/be_reflection_dx12.h"

static bool ShaderStageToVisibility(BePipelineStage stage, D3D12_SHADER_VISIBILITY& visibility)
{
    bool ret = true;
    switch (stage)
    {
    case BePipelineStage::kVertex:
        visibility = D3D12_SHADER_VISIBILITY_VERTEX;
        break;
    case BePipelineStage::kFragment:
        visibility = D3D12_SHADER_VISIBILITY_PIXEL;
        break;
    case BePipelineStage::kCompute:
        visibility = D3D12_SHADER_VISIBILITY_ALL;
        break;
    default:
        ret = false;
    }
    return ret;
}

static inline UINT BindCountToNumDescriptors(UINT bind_count)
{
    // In case of unbounded array, D3D12_SHADER_INPUT_BIND_DESC::bind_count comes 0,
    // but D3D12_DESCRIPTOR_RANGE::NumDescriptors must be 0xFFFFFFFF.
    return bind_count == 0 ? UINT32_MAX : bind_count;
}

// The algorithm is based on Microsft documentation, page "Root Signature Limits".
static bool CalculateRootSignatureCost(const D3D12_ROOT_PARAMETER* params, size_t param_count, std::uint32_t& cost)
{
    bool ret = true;
    cost = 0;
    for (size_t i = 0; i < param_count; ++i)
    {
        switch (params[i].ParameterType)
        {
        case D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS:
            cost += params[i].Constants.Num32BitValues;
            break;
        case D3D12_ROOT_PARAMETER_TYPE_CBV:
        case D3D12_ROOT_PARAMETER_TYPE_SRV:
        case D3D12_ROOT_PARAMETER_TYPE_UAV:
            cost += 2;
            break;
        case D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
            cost += 1;
            break;
        default:
            ret = false;
        }
    }
    return ret;
}

static constexpr uint32_t kMaxRootSignatureCost = 64;

class RootSignatureGenerator
{
public:
    RootSignatureGenerator(UINT64 shader_requires_flags);
    beKA::beStatus SetShaderResourceBindings(BePipelineStage stage, const D3D12_SHADER_INPUT_BIND_DESC* bindings, size_t binding_count, std::stringstream& err);
    void           FinalizeRootSignatureSetup(std::stringstream& err);
    const D3D12_ROOT_SIGNATURE_DESC& GetDesc() const
    {
        return desc_;
    }

private:
    const UINT64                                         shader_requires_flags_ = 0;
    std::vector<std::unique_ptr<D3D12_DESCRIPTOR_RANGE>> descriptor_ranges_;
    std::vector<D3D12_ROOT_PARAMETER>                    parameters_;
    D3D12_ROOT_SIGNATURE_DESC                            desc_       = {};
    bool                                                 is_compute_ = false;

    D3D12_DESCRIPTOR_RANGE* AllocateDescriptorRange();
    bool                    CalculateCost(std::uint32_t& root_signature_cost) const
    {
        return CalculateRootSignatureCost(parameters_.data(), parameters_.size(), root_signature_cost);
    }
    void CreateCompactRootSignature();
};

RootSignatureGenerator::RootSignatureGenerator(UINT64 shader_requires_flags)
    : shader_requires_flags_{shader_requires_flags}
{
}

beKA::beStatus RootSignatureGenerator::SetShaderResourceBindings(BePipelineStage                     stage,
                                                                 const D3D12_SHADER_INPUT_BIND_DESC* bindings,
                                                                 size_t                              binding_count,
                                                                 std::stringstream&                  err)
{
    beKA::beStatus          ret = beKA::beStatus::kBeStatusSuccess;
    D3D12_SHADER_VISIBILITY shader_visibility;
    ret = ShaderStageToVisibility(stage, shader_visibility) ? beKA::beStatus::kBeStatusSuccess : beKA::beStatus::kBeStatusDxcCannotAutoGenerateRootSignature;
    if (ret == beKA::beStatus::kBeStatusSuccess)
    {
        if (stage == BePipelineStage::kCompute)
        {
            is_compute_ = true;
        }

        for (size_t i = 0; i < binding_count; ++i)
        {
            const auto& bind_desc = bindings[i];
            switch (bind_desc.Type)
            {
            // CBV
            case D3D_SIT_CBUFFER:
            {
                D3D12_DESCRIPTOR_RANGE* desc_range            = AllocateDescriptorRange();
                desc_range->RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
                desc_range->NumDescriptors                    = BindCountToNumDescriptors(bind_desc.BindCount);
                desc_range->BaseShaderRegister                = bind_desc.BindPoint;
                desc_range->RegisterSpace                     = bind_desc.Space;
                desc_range->OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

                D3D12_ROOT_PARAMETER root_param                = {};
                root_param.ParameterType                       = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
                root_param.DescriptorTable.NumDescriptorRanges = 1;
                root_param.DescriptorTable.pDescriptorRanges   = desc_range;
                root_param.ShaderVisibility                    = shader_visibility;
                parameters_.push_back(root_param);
                break;
            }
            // UAV
            case D3D_SIT_UAV_RWTYPED:
            case D3D_SIT_UAV_RWSTRUCTURED:
            case D3D_SIT_UAV_RWBYTEADDRESS:
            case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
            case D3D_SIT_UAV_FEEDBACKTEXTURE:
            {
                D3D12_DESCRIPTOR_RANGE* desc_range            = AllocateDescriptorRange();
                desc_range->RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
                desc_range->NumDescriptors                    = BindCountToNumDescriptors(bind_desc.BindCount);
                desc_range->BaseShaderRegister                = bind_desc.BindPoint;
                desc_range->RegisterSpace                     = bind_desc.Space;
                desc_range->OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

                D3D12_ROOT_PARAMETER root_param                = {};
                root_param.ParameterType                       = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
                root_param.DescriptorTable.NumDescriptorRanges = 1;
                root_param.DescriptorTable.pDescriptorRanges   = desc_range;
                root_param.ShaderVisibility                    = shader_visibility;
                parameters_.push_back(root_param);
                break;
            }
            // SRV
            case D3D_SIT_TBUFFER:
            case D3D_SIT_TEXTURE:
            case D3D_SIT_BYTEADDRESS:
            case D3D_SIT_STRUCTURED:
            case D3D_SIT_RTACCELERATIONSTRUCTURE:
            {
                D3D12_DESCRIPTOR_RANGE* desc_range            = AllocateDescriptorRange();
                desc_range->RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
                desc_range->NumDescriptors                    = BindCountToNumDescriptors(bind_desc.BindCount);
                desc_range->BaseShaderRegister                = bind_desc.BindPoint;
                desc_range->RegisterSpace                     = bind_desc.Space;
                desc_range->OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

                D3D12_ROOT_PARAMETER root_param                = {};
                root_param.ParameterType                       = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
                root_param.DescriptorTable.NumDescriptorRanges = 1;
                root_param.DescriptorTable.pDescriptorRanges   = desc_range;
                root_param.ShaderVisibility                    = shader_visibility;
                parameters_.push_back(root_param);
                break;
            }
            // Sampler
            case D3D_SIT_SAMPLER:
            {
                D3D12_DESCRIPTOR_RANGE* desc_range            = AllocateDescriptorRange();
                desc_range->RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
                desc_range->NumDescriptors                    = BindCountToNumDescriptors(bind_desc.BindCount);
                desc_range->BaseShaderRegister                = bind_desc.BindPoint;
                desc_range->RegisterSpace                     = bind_desc.Space;
                desc_range->OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

                D3D12_ROOT_PARAMETER root_param                = {};
                root_param.ParameterType                       = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
                root_param.DescriptorTable.NumDescriptorRanges = 1;
                root_param.DescriptorTable.pDescriptorRanges   = desc_range;
                root_param.ShaderVisibility                    = shader_visibility;
                parameters_.push_back(root_param);
                break;
            }
            default:
                // D3D_SIT_UAV_APPEND_STRUCTURED and D3D_SIT_UAV_CONSUME_STRUCTURED do not occur in D3D12.
                // AppendStructuredBuffer, ConsumeStructuredBuffer appear as D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER.
                ret = beKA::beStatus::kBeStatusDxcCannotAutoGenerateRootSignature;
                err << "\n"
                    << "Error: unsupported binding type."
                    << "\n";
            }
        }
    }
    return ret;
}

void RootSignatureGenerator::FinalizeRootSignatureSetup(std::stringstream& err)
{
    desc_.Flags = D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
                  D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
    if (!is_compute_)
    {
        desc_.Flags |= D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    }
    if ((shader_requires_flags_ & D3D_SHADER_REQUIRES_RESOURCE_DESCRIPTOR_HEAP_INDEXING) != 0)
    {
        desc_.Flags |= D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED;
    }
    if ((shader_requires_flags_ & D3D_SHADER_REQUIRES_SAMPLER_DESCRIPTOR_HEAP_INDEXING) != 0)
    {
        desc_.Flags |= D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED;
    }

    CreateCompactRootSignature();
    std::uint32_t root_signature_cost;
    if (CalculateCost(root_signature_cost) && root_signature_cost > kMaxRootSignatureCost)
    {
        err << "Warning: Cost of root signature exceeds the limit of 64 DWORDs. Compilation will likely fail.\n";
    }

    desc_.NumParameters = static_cast<UINT>(parameters_.size());
    desc_.pParameters   = parameters_.size() ? parameters_.data() : nullptr;
}

D3D12_DESCRIPTOR_RANGE* RootSignatureGenerator::AllocateDescriptorRange()
{
    descriptor_ranges_.push_back(std::make_unique<D3D12_DESCRIPTOR_RANGE>());
    return descriptor_ranges_.back().get();
}

void RootSignatureGenerator::CreateCompactRootSignature()
{
    // These thresholds are selected arbitrarily.
    // They can be anything between 0 and kMaxRootSignatureCost.
    std::uint32_t root_signature_cost;
    bool          status = CalculateCost(root_signature_cost) && root_signature_cost <= kMaxRootSignatureCost / 4;
    if (!status)
    {
        // Sort by: RangeType, RegisterSpace, BaseShaderRegister.
        std::sort(parameters_.begin(), parameters_.end(), [](const D3D12_ROOT_PARAMETER& lhs, const D3D12_ROOT_PARAMETER& rhs) -> bool {
            assert(lhs.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE && lhs.DescriptorTable.NumDescriptorRanges == 1);
            assert(rhs.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE && rhs.DescriptorTable.NumDescriptorRanges == 1);

            const D3D12_DESCRIPTOR_RANGE& lhs_range = lhs.DescriptorTable.pDescriptorRanges[0];
            const D3D12_DESCRIPTOR_RANGE& rhs_range = rhs.DescriptorTable.pDescriptorRanges[0];

            assert(lhs_range.OffsetInDescriptorsFromTableStart == D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND && lhs_range.NumDescriptors > 0);
            assert(rhs_range.OffsetInDescriptorsFromTableStart == D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND && rhs_range.NumDescriptors > 0);

            if (lhs_range.RangeType != rhs_range.RangeType)
                return lhs_range.RangeType < rhs_range.RangeType;
            if (lhs_range.RegisterSpace != rhs_range.RegisterSpace)
                return lhs_range.RegisterSpace < rhs_range.RegisterSpace;
            return lhs_range.BaseShaderRegister < rhs_range.BaseShaderRegister;
        });

        // Merge identical bindings across shader stages, e.g.:
        // DescriptorTable(SRV(t13), visibility=SHADER_VISIBILITY_VERTEX),
        // DescriptorTable(SRV(t13), visibility=SHADER_VISIBILITY_PIXEL)
        // ->
        // DescriptorTable(SRV(t13), visibility=SHADER_VISIBILITY_ALL)
        for (size_t i = 1; i < parameters_.size();)
        {
            D3D12_ROOT_PARAMETER&         prev_param = parameters_[i - 1];
            D3D12_ROOT_PARAMETER&         curr_param = parameters_[i];
            const D3D12_DESCRIPTOR_RANGE& prev_range = prev_param.DescriptorTable.pDescriptorRanges[0];
            const D3D12_DESCRIPTOR_RANGE& curr_range = curr_param.DescriptorTable.pDescriptorRanges[0];

            if (curr_range.RangeType == prev_range.RangeType && curr_range.RegisterSpace == prev_range.RegisterSpace &&
                curr_range.BaseShaderRegister == prev_range.BaseShaderRegister && curr_range.NumDescriptors == prev_range.NumDescriptors)
            {
                assert(prev_param.ShaderVisibility != curr_param.ShaderVisibility);
                prev_param.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
                parameters_.erase(parameters_.begin() + i);
            }
            else
            {
                ++i;
            }
        }

        status = CalculateCost(root_signature_cost) && root_signature_cost <= kMaxRootSignatureCost / 2;
        if (!status)
        {
            // Merge bindings of subsequent registers, e.g.:
            // DescriptorTable(SRV(t13), visibility=SHADER_VISIBILITY_VERTEX),
            // DescriptorTable(SRV(t14, numDescriptors=3), visibility=SHADER_VISIBILITY_PIXEL)
            // DescriptorTable(SRV(t17), visibility=SHADER_VISIBILITY_ALL)
            // ->
            // DescriptorTable(SRV(t13, numDescriptors=5), visibility=SHADER_VISIBILITY_ALL)
            for (size_t i = 1; i < parameters_.size();)
            {
                D3D12_DESCRIPTOR_RANGE& prev_range = const_cast<D3D12_DESCRIPTOR_RANGE&>(parameters_[i - 1].DescriptorTable.pDescriptorRanges[0]);
                D3D12_DESCRIPTOR_RANGE& curr_range = const_cast<D3D12_DESCRIPTOR_RANGE&>(parameters_[i].DescriptorTable.pDescriptorRanges[0]);

                if (curr_range.NumDescriptors != UINT32_MAX && prev_range.NumDescriptors != UINT32_MAX &&  // Not including unbounded.
                    curr_range.RangeType == prev_range.RangeType && curr_range.RegisterSpace == prev_range.RegisterSpace &&
                    curr_range.BaseShaderRegister == prev_range.BaseShaderRegister + prev_range.NumDescriptors)
                {
                    prev_range.NumDescriptors += curr_range.NumDescriptors;
                    if (parameters_[i - 1].ShaderVisibility != parameters_[i].ShaderVisibility)
                    {
                        parameters_[i - 1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
                    }
                    parameters_.erase(parameters_.begin() + i);
                }
                else
                {
                    ++i;
                }
            }
        }   
    }
}

class RootSignatureUtil
{

    static void WriteFlags(std::stringstream& stream, D3D12_ROOT_SIGNATURE_FLAGS rs_flags)
    {
        stream << "RootFlags( ";
        if (rs_flags == D3D12_ROOT_SIGNATURE_FLAG_NONE)
        {
            stream << "0";
        }
        else
        {
            uint32_t index     = 0;
            auto     WriteFlag = [&](D3D12_ROOT_SIGNATURE_FLAGS rs_flag, const char* rs_flag_name) {
                if ((rs_flags & rs_flag) != 0)
                {
                    if (index > 0)
                    {
                        if (index % 2 == 0)
                        {
                            stream << "\n| ";
                        }
                        else
                        {
                            stream << " | ";
                        }
                    }
                    stream << rs_flag_name;
                    ++index;
                }
            };
            WriteFlag(D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT, "ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT");
            WriteFlag(D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS, "DENY_VERTEX_SHADER_ROOT_ACCESS");
            WriteFlag(D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS, "DENY_HULL_SHADER_ROOT_ACCESS");
            WriteFlag(D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS, "DENY_DOMAIN_SHADER_ROOT_ACCESS");
            WriteFlag(D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS, "DENY_GEOMETRY_SHADER_ROOT_ACCESS");
            WriteFlag(D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS, "DENY_PIXEL_SHADER_ROOT_ACCESS");
            WriteFlag(D3D12_ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT, "ALLOW_STREAM_OUTPUT");
            WriteFlag(D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE, "LOCAL_ROOT_SIGNATURE");
            WriteFlag(D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS, "DENY_AMPLIFICATION_SHADER_ROOT_ACCESS");
            WriteFlag(D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS, "DENY_MESH_SHADER_ROOT_ACCESS");
            WriteFlag(D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED, "CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED");
            WriteFlag(D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED, "SAMPLER_HEAP_DIRECTLY_INDEXED");
        }
        stream << " )";
    }

    static void WriteSpace(std::stringstream& stream, UINT space)
    {
        if (space != 0)
        {
            stream << ", space=" << space;
        }
    }

    static bool WriteShaderVisibility(std::stringstream& stream, D3D12_SHADER_VISIBILITY visibility)
    {
        bool ret = true;
        switch (visibility)
        {
        case D3D12_SHADER_VISIBILITY_ALL:;  
            // Don't print. It is the default.
            break;
        case D3D12_SHADER_VISIBILITY_VERTEX:
            stream << ", visibility=SHADER_VISIBILITY_VERTEX";
            break;
        case D3D12_SHADER_VISIBILITY_HULL:
            stream << ", visibility=SHADER_VISIBILITY_HULL";
            break;
        case D3D12_SHADER_VISIBILITY_DOMAIN:
            stream << ", visibility=SHADER_VISIBILITY_DOMAIN";
            break;
        case D3D12_SHADER_VISIBILITY_GEOMETRY:
            stream << ", visibility=SHADER_VISIBILITY_GEOMETRY";
            break;
        case D3D12_SHADER_VISIBILITY_PIXEL:
            stream << ", visibility=SHADER_VISIBILITY_PIXEL";
            break;
        case D3D12_SHADER_VISIBILITY_AMPLIFICATION:
            stream << ", visibility=SHADER_VISIBILITY_AMPLIFICATION";
            break;
        case D3D12_SHADER_VISIBILITY_MESH:
            stream << ", visibility=SHADER_VISIBILITY_MESH";
            break;
        default:
            ret = false;
        }
        return ret;
    }

    static bool WriteParameterRootConstants(std::stringstream& stream, D3D12_ROOT_PARAMETER param)
    {
        bool ret = true;
        if (param.ParameterType != D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS)
        {
            ret = false;
        }
        if (ret)
        {
            stream << ",\nRootConstants(num32BitConstants=" << param.Constants.Num32BitValues << ", b" << param.Constants.ShaderRegister;
            WriteSpace(stream, param.Constants.RegisterSpace);
            ret = WriteShaderVisibility(stream, param.ShaderVisibility);
            stream << ')';
        }
        return ret;
    }

    static bool WriteParameterRootDescriptor(std::stringstream& stream, D3D12_ROOT_PARAMETER param)
    {
        bool ret = true;
        switch (param.ParameterType)
        {
        case D3D12_ROOT_PARAMETER_TYPE_CBV:
            stream << ",\nCBV(b";
            break;
        case D3D12_ROOT_PARAMETER_TYPE_SRV:
            stream << ",\nSRV(t";
            break;
        case D3D12_ROOT_PARAMETER_TYPE_UAV:
            stream << ",\nUAV(u";
            break;
        default:
            ret = false;
        }

        if (ret)
        {
            stream << param.Descriptor.ShaderRegister;
            WriteSpace(stream, param.Descriptor.RegisterSpace);
            ret = WriteShaderVisibility(stream, param.ShaderVisibility);
            stream << ')';
        }
        return ret;
    }

    static void WriteDescriptorRange(std::stringstream& stream, D3D12_DESCRIPTOR_RANGE range)
    {
        switch (range.RangeType)
        {
        case D3D12_DESCRIPTOR_RANGE_TYPE_CBV:
            stream << "CBV(b" << range.BaseShaderRegister;
            break;
        case D3D12_DESCRIPTOR_RANGE_TYPE_SRV:
            stream << "SRV(t" << range.BaseShaderRegister;
            break;
        case D3D12_DESCRIPTOR_RANGE_TYPE_UAV:
            stream << "UAV(u" << range.BaseShaderRegister;
            break;
        case D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER:
            stream << "Sampler(s" << range.BaseShaderRegister;
            break;
        }

        if (range.NumDescriptors != 1)
        {
            if (range.NumDescriptors == UINT_MAX)
            {
                stream << ", numDescriptors=unbounded";
            }
            else
            {
                stream << ", numDescriptors=" << range.NumDescriptors;
            }
        }

        WriteSpace(stream, range.RegisterSpace);

        if (range.OffsetInDescriptorsFromTableStart != D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND)
        {
            stream << ", offset=" << range.OffsetInDescriptorsFromTableStart;
        }

        stream << ')';
    }

    static bool WriteParameterDescriptorTable(std::stringstream& stream, D3D12_ROOT_PARAMETER param)
    {
        bool ret = true;
        if (param.ParameterType != D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
        {
            ret = false;
        }
        if (ret)
        {
            stream << ",\nDescriptorTable(";
            for (UINT i = 0; i < param.DescriptorTable.NumDescriptorRanges; ++i)
            {
                if (i > 0)
                {
                    stream << ", ";
                }
                WriteDescriptorRange(stream, param.DescriptorTable.pDescriptorRanges[i]);
            }
            ret = WriteShaderVisibility(stream, param.ShaderVisibility);
            stream << ')';
        }
        return ret;
    }

    static bool WriteParameter(std::stringstream& stream, D3D12_ROOT_PARAMETER param)
    {
        bool ret = true;
        switch (param.ParameterType)
        {
        case D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS:
            ret = WriteParameterRootConstants(stream, param);
            break;
        case D3D12_ROOT_PARAMETER_TYPE_CBV:
            [[fallthrough]];
        case D3D12_ROOT_PARAMETER_TYPE_SRV:
            [[fallthrough]];
        case D3D12_ROOT_PARAMETER_TYPE_UAV:
            ret = WriteParameterRootDescriptor(stream, param);
            break;
        case D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
            ret = WriteParameterDescriptorTable(stream, param);
            break;
        default:
            ret = false;
        }
        return ret;
    }

    static bool WriteRootSignatureString(const D3D12_ROOT_SIGNATURE_DESC& desc, std::string& root_signature)
    {
        bool ret = true;
        if (desc.NumStaticSamplers != 0)
        {
            ret = false;
        }
        if (ret)
        {
            std::stringstream ss;

            WriteFlags(ss, desc.Flags);
            for (UINT i = 0; i < desc.NumParameters; ++i)
            {
                ret = WriteParameter(ss, desc.pParameters[i]);
                if (!ret)
                {
                    break;
                }
            }
            if (ret)
            {
                root_signature = ss.str();
            }
        }
        return ret;
    }

public:
    static bool WriteRootSignatureHlsl(const D3D12_ROOT_SIGNATURE_DESC& desc, std::string& root_signature)
    {
        std::string root_signature_string;
        bool        ret = WriteRootSignatureString(desc, root_signature_string);
        if (ret)
        {
            // Trim trailing line endings.
            while (!root_signature_string.empty() && (root_signature_string.back() == '\r' || root_signature_string.back() == '\n'))
            {
                root_signature_string.pop_back();
            }

            // Format string into a HLSL macro, respecting escape sequences and line endings.
            std::stringstream ss;
            ss << "// " << kAutoGeneratedShaderHeader << "\n\n";
            ss << "#define " << kRootSignature << " \\\n    \"";
           
            for (char ch : root_signature_string)
            {
                if (ch == '\n')
                {
                    // This is designed so that the macro looks like in Microsoft example:
                    //    "SRV(t0), " \
                    //    "UAV(u0), " \
                    // etc.
                    ss << " \" \\\n    \"";
                }
                else if (ch == '\r')
                {
                    // Ignore.
                }
                else if (ch == '\\' || ch == '"')
                {
                    // Escape sequence.
                    ss << '\\' << ch;
                }
                else
                {
                    // Normal character.
                    ss << ch;
                }
            }
            ss << "\"\n";
            root_signature = ss.str();
        }
        return ret;
    }

};


beKA::beStatus BeDx12Reflection::GenerateRootSignatureCompute(const DxcReflectionOutput& cs_dxc_output,
                                                              std::string&               root_signauture_hlsl,
                                                              std::stringstream&         err) const
{
    beKA::beStatus rc = beKA::beStatus::kBeStatusSuccess;
    if (!cs_dxc_output.shader_reflection_ptr)
    {
        rc = beKA::beStatus::kBeStatusDxcCannotAutoGenerateRootSignature;
    }
    if (rc == beKA::beStatus::kBeStatusSuccess)
    {
        const UINT64           shader_requires_flags = cs_dxc_output.shader_reflection_ptr->GetRequiresFlags();
        RootSignatureGenerator root_signature_generator{shader_requires_flags};
        rc = root_signature_generator.SetShaderResourceBindings(
            BePipelineStage::kCompute, cs_dxc_output.resource_bindings.data(), cs_dxc_output.resource_bindings.size(), err);
        if (rc == beKA::beStatus::kBeStatusSuccess)
        {
            root_signature_generator.FinalizeRootSignatureSetup(err);
            if (!RootSignatureUtil::WriteRootSignatureHlsl(root_signature_generator.GetDesc(), root_signauture_hlsl))
            {
                rc = beKA::beStatus::kBeStatusDxcCannotAutoGenerateRootSignature;
            }
        }
    }
    return rc;
}

beKA::beStatus BeDx12Reflection::GenerateRootSignatureGraphics(UINT64                     shader_requires_flags,
                                                               bool                       has_vs,
                                                               const DxcReflectionOutput& vs_output,
                                                               bool                       has_ps,
                                                               const DxcReflectionOutput& ps_output,
                                                               std::string&               root_signauture_hlsl,
                                                               std::stringstream&         err) const
{
    beKA::beStatus rc = beKA::beStatus::kBeStatusSuccess;
    RootSignatureGenerator root_signature_generator{shader_requires_flags};
    if (has_vs)
    {
        rc = root_signature_generator.SetShaderResourceBindings(
            BePipelineStage::kVertex, vs_output.resource_bindings.data(), vs_output.resource_bindings.size(), err);
    }
    if (has_ps)
    {
        rc = root_signature_generator.SetShaderResourceBindings(
            BePipelineStage::kFragment, ps_output.resource_bindings.data(), ps_output.resource_bindings.size(), err);
    }
    if (rc == beKA::beStatus::kBeStatusSuccess)
    {
        root_signature_generator.FinalizeRootSignatureSetup(err);
        if (!RootSignatureUtil::WriteRootSignatureHlsl(root_signature_generator.GetDesc(), root_signauture_hlsl))
        {
            rc = beKA::beStatus::kBeStatusDxcCannotAutoGenerateRootSignature;
        }
    }
    return rc;
}

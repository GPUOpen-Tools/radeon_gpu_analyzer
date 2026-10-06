//=============================================================================
/// Copyright (c) 2024-2026 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for rga code object metadata parser class.
//=============================================================================

// C++.
#include <unordered_map>
#include <regex>

// Yaml.
#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4996)
#pragma warning(disable : 4127)
#endif
#include "yaml-cpp/yaml.h"
#ifdef _WIN32
#pragma warning(pop)
#endif

// Local.
#include "radeon_gpu_analyzer_backend/be_metadata_parser.h"

// Hardware Stage Dot Strings.
const std::string kStrLS = ".ls";
const std::string kStrHS = ".hs";
const std::string kStrES = ".es";
const std::string kStrGS = ".gs";
const std::string kStrVS = ".vs";
const std::string kStrPS = ".ps";
const std::string kStrCS = ".cs";

// Graphics Shader Stage Dot Strings.
const std::string kStrVertex   = ".vertex";
const std::string kStrHull     = ".hull";
const std::string kStrDomain   = ".domain";
const std::string kStrGeometry = ".geometry";
const std::string kStrPixel    = ".pixel";
const std::string kStrCompute  = ".compute";
const std::string kStrMesh           = ".mesh";
const std::string kStrTask           = ".task";
const std::string kStrAmplification  = ".amplification";

// Raytracing Shader Stage Dot Strings.
const std::string kStrRayGeneration = "RayGeneration";
const std::string kStrMiss          = "Miss";
const std::string kStrAnyHit        = "AnyHit";
const std::string kStrClosestHit    = "ClosestHit";
const std::string kStrIntersection  = "Intersection";
const std::string kStrCallable      = "Callable";
const std::string kStrTraversal     = "Traversal";
const std::string kStrLaunchKernel  = "LaunchKernel";
const std::string kStrUnknown       = "Unknown";

// Wave size strings.
const std::string kStrWaveSizeUnknown = "Unknown";
const std::string kStrWaveSize32      = "32";
const std::string kStrWaveSize64      = "64";
const uint64_t    kWaveSize32         = 32;
const uint64_t    kWaveSize64         = 64;

// Amdgpudis dot tokens.
static const std::string kStrLcCodeObjectMetadataTokenStart          = "---\n";
static const std::string kStrLcCodeObjectMetadataTokenEnd            = "\n...";
static const std::string kStrCodeObjectMetadataKeyKernels            = "amdhsa.kernels";
static const std::string kStrCodeObjectMetadataKeyDeviceTarget       = "amdhsa.target";
static const std::string kStrCodeObjectMetadataKeyPipelines          = "amdpal.pipelines";
static const std::string kAmdgpuDisDotApiToken                       = ".api";
static const std::string kAmdgpuDisDotHardwareStagesToken            = ".hardware_stages";
static const std::string kAmdgpuDisDotEntryPointToken                = ".entry_point";
static const std::string kAmdgpuDisDotEntryPointSymbolToken          = ".entry_point_symbol";
static const std::string kAmdgpuDisDotScratchMemorySizeToken         = ".scratch_memory_size";
static const std::string kDotSgprCountToken                          = ".sgpr_count";
static const std::string kAmdgpuDisDotSgprLimitToken                 = ".sgpr_limit";
static const std::string kDotVgprCountToken                          = ".vgpr_count";
static const std::string kDotAgprCountToken                          = ".agpr_count";
static const std::string kAmdgpuDisDotVgprLimitToken                 = ".vgpr_limit";
static const std::string kAmdgpuDisDotShaderFunctionsToken           = ".shader_functions";
static const std::string kAmdgpuDisDotLdsSizeToken                   = ".lds_size";
static const std::string kAmdgpuDisDotShaderSubtypeToken             = ".shader_subtype";
static const std::string kAmdgpuDisDotShadersToken                   = ".shaders";
static const std::string kAmdgpuDisDotHardwareMappingToken           = ".hardware_mapping";
static const std::string kDotWavefrontSizeToken                      = ".wavefront_size";
static const std::string kAmdgpuDisDotApiShaderHashToken             = ".api_shader_hash";
static const std::string kAmdgpuDisDxilStdManglingPrefix             = "\001?";
static const std::string kAmdgpuDisDxilStdManglingSuffix             = "@@";
static const std::string kAmdgpuDisDxilStdHexPattern                 = "[A-F0-9]+:";
static const std::string kAmdgpuDisRaytracingStandAloneComputeShader = "lgc.shader.CS.main";

// CodeObject MetaData keys.
static const std::string kStrCodeObjectMetadataDevicePrefix          = "amdgcn--amdpal--gfx";
static const std::string kStrCodeObjectMetadataKeyKernelName         = ".name";
static const std::string kStrCodeObjectMetadataKeySpilledSgprs       = ".sgpr_spill_count";
static const std::string kStrCodeObjectMetadataKeySpilledVgprs       = ".vgpr_spill_count";
static const std::string kStrCodeObjectMetadataKeyGroupSegmentSize   = ".group_segment_fixed_size";
static const std::string kStrCodeObjectMetadataKeyPrivateSegmentSize = ".private_segment_fixed_size";

std::string BeMangledKernelUtils::DemangleShaderName(const std::string& kernel_name)
{
    // Mangled names follow the format:
    // (hex digits):(hex digits):\01?(shader function name)@@...
    // The \x01 byte may be absent in disassembly labels (llvm-objdump strips it).
    // Based on RGP's RgpIsaDemangleShaderFuncName demangling logic.

    std::string shader_name{kernel_name};
    shader_name = UnQuote(shader_name, '\"');
    shader_name = UnQuote(shader_name, '\'');

    // Search for the shader function name between the "\01?" and the "@@".
    // The \01 (0x01 byte) is made optional by the regex '?' quantifier so this
    // handles both metadata strings (with \x01) and disassembly labels (without).
    std::regex  mangling_prefix_suffix("\01?(\\w+)@@");
    std::smatch name_match;
    bool        contains_match = std::regex_search(shader_name, name_match, mangling_prefix_suffix);

    std::string func_name;
    if (!contains_match)
    {
        // Check for a hash separated by a colon. Traversal shaders don't have the
        // "\01?" marker but they do have a hash and a colon.
        mangling_prefix_suffix = std::regex("[A-F0-9]+:(\\w+)");
        contains_match         = std::regex_search(shader_name, name_match, mangling_prefix_suffix);

        if (!contains_match)
        {
            // If the name wasn't mangled, return a copy of the original name.
            func_name = shader_name;
        }
        else
        {
            func_name = name_match[1];
        }
    }
    else
    {
        func_name = name_match[1];
    }

    // If a match was found, check for the presence of a ".resume.N" substring
    // in the original name and append it to the demangled function name if found.
    if (contains_match)
    {
        std::regex  resume_regex("\\.resume\\.[0-9]+");
        std::smatch resume_match;
        if (std::regex_search(shader_name, resume_match, resume_regex))
        {
            const std::string resume_suffix = resume_match.str();
            // Avoid duplicating the suffix if func_name already ends with the same ".resume.N".
            if (func_name.size() < resume_suffix.size() ||
                func_name.compare(func_name.size() - resume_suffix.size(), resume_suffix.size(), resume_suffix) != 0)
            {
                func_name += resume_suffix;
            }
        }
    }

    return func_name;
}

std::string BeMangledKernelUtils::UnQuote(const std::string& str, char quote)
{
    std::string no_quotes{str};
    no_quotes.erase(std::remove(no_quotes.begin(), no_quotes.end(), quote), no_quotes.end());
    return no_quotes;
}

std::string BeMangledKernelUtils::Quote(const std::string& str, char quote)
{
    return quote + str + quote;
}

BeAmdPalMetaData::StageType BeAmdPalMetaData::GetStageType(const std::string& stage_name)
{
    static const std::unordered_map<std::string, StageType> stageMap = {{kStrLS, StageType::kLS},
                                                                        {kStrHS, StageType::kHS},
                                                                        {kStrES, StageType::kES},
                                                                        {kStrGS, StageType::kGS},
                                                                        {kStrVS, StageType::kVS},
                                                                        {kStrPS, StageType::kPS},
                                                                        {kStrCS, StageType::kCS}};

    auto it = stageMap.find(stage_name);
    if (it != stageMap.end())
    {
        return it->second;
    }
    throw std::runtime_error("Unknown stage type: " + stage_name);
}

BeAmdPalMetaData::ShaderType BeAmdPalMetaData::GetShaderType(const std::string& shader_name)
{
    static const std::unordered_map<std::string, ShaderType> shaderMap = {{kStrVertex, ShaderType::kVertex},
                                                                          {kStrHull, ShaderType::kTessellationControl},
                                                                          {kStrDomain, ShaderType::kTessellationEvaluation},
                                                                          {kStrGeometry, ShaderType::kGeometry},
                                                                          {kStrPixel, ShaderType::kFragment},
                                                                          {kStrCompute, ShaderType::kCompute},
                                                                          {kStrMesh, ShaderType::kMesh},
                                                                          {kStrTask, ShaderType::kTask},
                                                                          {kStrAmplification, ShaderType::kTask}};

    auto it = shaderMap.find(shader_name);
    if (it != shaderMap.end())
    {
        return it->second;
    }
    throw std::runtime_error("Unknown shader type: " + shader_name);
}

BeAmdPalMetaData::ShaderSubtype BeAmdPalMetaData::GetShaderSubtype(const std::string& subtype_name)
{
    static const std::unordered_map<std::string, ShaderSubtype> subtypeMap = {{kStrRayGeneration, ShaderSubtype::kRayGeneration},
                                                                              {kStrMiss, ShaderSubtype::kMiss},
                                                                              {kStrAnyHit, ShaderSubtype::kAnyHit},
                                                                              {kStrClosestHit, ShaderSubtype::kClosestHit},
                                                                              {kStrIntersection, ShaderSubtype::kIntersection},
                                                                              {kStrCallable, ShaderSubtype::kCallable},
                                                                              {kStrTraversal, ShaderSubtype::kTraversal},
                                                                              {kStrLaunchKernel, ShaderSubtype::kLaunchKernel},
                                                                              {kStrUnknown, ShaderSubtype::kUnknown}};

    auto it = subtypeMap.find(subtype_name);
    if (it != subtypeMap.end())
    {
        return it->second;
    }
    return ShaderSubtype::kUnknown;
}

std::string BeAmdPalMetaData::GetStageName(StageType stage_type)
{
    static const std::unordered_map<StageType, const std::string&> inverseStageMap = {{StageType::kLS, kStrLS},
                                                                                      {StageType::kHS, kStrHS},
                                                                                      {StageType::kES, kStrES},
                                                                                      {StageType::kGS, kStrGS},
                                                                                      {StageType::kVS, kStrVS},
                                                                                      {StageType::kPS, kStrPS},
                                                                                      {StageType::kCS, kStrCS}};

    auto it = inverseStageMap.find(stage_type);
    if (it != inverseStageMap.end())
    {
        return it->second;
    }
    throw std::runtime_error("Unknown stage type: " + std::to_string(static_cast<int>(stage_type)));
}

std::string BeAmdPalMetaData::GetShaderName(ShaderType shader_type)
{
    static const std::unordered_map<ShaderType, const std::string&> inverseShaderMap = {{ShaderType::kVertex, kStrVertex},
                                                                                        {ShaderType::kTessellationControl, kStrHull},
                                                                                        {ShaderType::kTessellationEvaluation, kStrDomain},
                                                                                        {ShaderType::kGeometry, kStrGeometry},
                                                                                        {ShaderType::kFragment, kStrPixel},
                                                                                        {ShaderType::kCompute, kStrCompute},
                                                                                        {ShaderType::kMesh, kStrMesh},
                                                                                        {ShaderType::kTask, kStrTask}};

    auto it = inverseShaderMap.find(shader_type);
    if (it != inverseShaderMap.end())
    {
        return it->second;
    }
    throw std::runtime_error("Unknown shader type: " + std::to_string(static_cast<int>(shader_type)));
}

std::string BeAmdPalMetaData::GetWaveSize(beWaveSize wave_size)
{
    static const std::unordered_map<beWaveSize, const std::string&> inverseWaveSizeMap = {{beWaveSize::kWave32, kStrWaveSize32},
                                                                                          {beWaveSize::kWave64, kStrWaveSize64}};

    auto it = inverseWaveSizeMap.find(wave_size);
    if (it != inverseWaveSizeMap.end())
    {
        return it->second;
    }
    return kStrWaveSizeUnknown;
}

beWaveSize BeAmdPalMetaData::GetWaveSize(std::string wave_size)
{
    static const std::unordered_map<std::string, beWaveSize> wavesizeMap = {{kStrWaveSize32, beWaveSize::kWave32}, {kStrWaveSize64, beWaveSize::kWave64}};

    auto it = wavesizeMap.find(wave_size);
    if (it != wavesizeMap.end())
    {
        return it->second;
    }

    return beWaveSize::kWave64;
}

beWaveSize BeAmdPalMetaData::GetWaveSize(uint64_t wave_size)
{
    static const std::unordered_map<uint64_t, beWaveSize> wavesizeMap = {{kWaveSize32, beWaveSize::kWave32}, {kWaveSize64, beWaveSize::kWave64}};

    auto it = wavesizeMap.find(wave_size);
    if (it != wavesizeMap.end())
    {
        return it->second;
    }

    return beWaveSize::kWave64;
}

std::string BeAmdPalMetaData::GetShaderSubtypeName(ShaderSubtype subtype)
{
    static const std::unordered_map<ShaderSubtype, const std::string&> inverseSubtypeMap = {
        {ShaderSubtype::kRayGeneration, kStrRayGeneration},
        {ShaderSubtype::kMiss, kStrMiss},
        {ShaderSubtype::kAnyHit, kStrAnyHit},
        {ShaderSubtype::kClosestHit, kStrClosestHit},
        {ShaderSubtype::kIntersection, kStrIntersection},
        {ShaderSubtype::kCallable, kStrCallable},
        {ShaderSubtype::kTraversal, kStrTraversal},
        {ShaderSubtype::kLaunchKernel, kStrLaunchKernel},
        {ShaderSubtype::kUnknown, kStrUnknown},
    };

    auto it = inverseSubtypeMap.find(subtype);
    if (it != inverseSubtypeMap.end())
    {
        return it->second;
    }
    return kStrUnknown;
}

std::string GetHardwareStagePropertyAsString(const YAML::Node& node, const std::string& key)
{
    std::string ret;
    if (node[key])
    {
        ret = node[key].as<std::string>();
    }
    return ret;
}

uint64_t GetHardwareStageProperty(const YAML::Node& node, const std::string& key)
{
    uint64_t          ret = beKA::kCalValue64Na;
    const YAML::Node& val = node[key];
    if (val && val.IsDefined() && val.IsScalar())
    {
        try
        {
            ret = val.as<uint64_t>();
        }
        catch (const YAML::BadConversion&)
        {
        }
    }
    return ret;
}

std::string GetHardwareStageString(const YAML::Node& node, const std::string& key)
{
    std::string       ret;
    const YAML::Node& val = node[key];
    if (val && val.IsDefined() && val.IsScalar())
    {
        try
        {
            ret = val.as<std::string>();
        }
        catch (const YAML::BadConversion&)
        {
        }
    }
    return ret;
}

beKA::beStatus BeAmdPalMetaData::ParseMetadata(const std::string& metadata_text, BeAmdPalMetaData::PipelineMetaData& pipeline_md)
{
    beKA::beStatus status       = beKA::beStatus::kBeStatusSuccess;
    size_t         start_offset = metadata_text.find(kStrLcCodeObjectMetadataTokenStart);
    size_t         end_offset;

    std::vector<BeAmdPalMetaData::PipelineMetaData> pipelines;
    while ((end_offset = metadata_text.find(kStrLcCodeObjectMetadataTokenEnd, start_offset)) != std::string::npos)
    {
        // For each metadata section ...
        try
        {
            const std::string& kernel_metadata_text  = metadata_text.substr(start_offset, end_offset - start_offset + kStrLcCodeObjectMetadataTokenEnd.size());
            YAML::Node         codeobj_metadata_node = YAML::Load(kernel_metadata_text);

            if (!codeobj_metadata_node.IsMap())
            {
                status = beKA::beStatus::kBeStatusCodeObjMdParsingFailed;
                break;
            }

            if (codeobj_metadata_node[kStrCodeObjectMetadataKeyKernels])
            {
                status = beKA::beStatus::kBeStatusComputeCodeObjMetaDataSuccess;
                break;
            }

            // For each pipeline ...
            for (const auto& pipeline_node : codeobj_metadata_node[kStrCodeObjectMetadataKeyPipelines])
            {
                BeAmdPalMetaData::PipelineMetaData pipeline;

                // try to extract device.
                size_t start_device = metadata_text.find(kStrCodeObjectMetadataDevicePrefix);
                if (start_device != std::string::npos && start_device < start_offset)
                {
                    size_t gfx_start = start_device + kStrCodeObjectMetadataDevicePrefix.size() - 3;  //  "gfx"
                    size_t gfx_end   = metadata_text.find('\n', gfx_start);
                    if (gfx_end != std::string::npos)
                    {
                        pipeline.device = metadata_text.substr(gfx_start, gfx_end - gfx_start);
                    }
                }

                // Parse api token.
                pipeline.api = pipeline_node[kAmdgpuDisDotApiToken].as<std::string>();

                // Parse stats for each hardware stage.
                for (const auto& stage_node : pipeline_node[kAmdgpuDisDotHardwareStagesToken])
                {
                    BeAmdPalMetaData::HardwareStageMetaData stage;

                    stage.stage_type                = BeAmdPalMetaData::GetStageType(stage_node.first.as<std::string>());
                    stage.entry_point = GetHardwareStagePropertyAsString(stage_node.second, kAmdgpuDisDotEntryPointSymbolToken);
                    if (stage.entry_point.empty())
                    {
                        stage.entry_point = GetHardwareStagePropertyAsString(stage_node.second, kAmdgpuDisDotEntryPointToken);
                    }
                    stage.stats.lds_size_used       = GetHardwareStageProperty(stage_node.second, kAmdgpuDisDotLdsSizeToken);
                    stage.stats.scratch_memory_used = GetHardwareStageProperty(stage_node.second, kAmdgpuDisDotScratchMemorySizeToken);
                    stage.stats.num_sgprs_used      = GetHardwareStageProperty(stage_node.second, kDotSgprCountToken);
                    stage.stats.num_sgprs_available = GetHardwareStageProperty(stage_node.second, kAmdgpuDisDotSgprLimitToken);
                    stage.stats.num_vgprs_used      = GetHardwareStageProperty(stage_node.second, kDotVgprCountToken);
                    stage.stats.num_vgprs_available = GetHardwareStageProperty(stage_node.second, kAmdgpuDisDotVgprLimitToken);
                    stage.stats.wavefront_size      = GetHardwareStageProperty(stage_node.second, kDotWavefrontSizeToken);

                    pipeline.hardware_stages.push_back(stage);
                }

                if (pipeline_node[kAmdgpuDisDotShaderFunctionsToken])
                {
                    bool is_traditional_compute_shader = true;
                    // Parse shader functions.
                    for (const auto& function_node : pipeline_node[kAmdgpuDisDotShaderFunctionsToken])
                    {
                        BeAmdPalMetaData::ShaderFunctionMetaData function;

                        std::string raw_key = function_node.first.as<std::string>();
                        function.raw_name   = raw_key;
                        function.name       = BeMangledKernelUtils::DemangleShaderName(raw_key);
                        if (function.name == kAmdgpuDisRaytracingStandAloneComputeShader)
                        {
                            break;
                        }

                        // Parse API shader hash
                        const YAML::Node& api_shader_hash_node = function_node.second[kAmdgpuDisDotApiShaderHashToken];
                        if (api_shader_hash_node.IsSequence() && api_shader_hash_node.size() == 2)
                        {
                            function.hash.low  = api_shader_hash_node[0].as<uint64_t>();
                            function.hash.high = api_shader_hash_node[1].as<uint64_t>();
                        }
                        else
                        {
                            status = beKA::beStatus::kBeStatusCodeObjMdParsingFailed;
                            break;
                        }

                        const std::string shader_subtype = GetHardwareStageString(function_node.second, kAmdgpuDisDotShaderSubtypeToken);
                        function.shader_subtype          = BeAmdPalMetaData::GetShaderSubtype(shader_subtype);
                        if (function.shader_subtype != BeAmdPalMetaData::ShaderSubtype::kUnknown)
                        {
                            is_traditional_compute_shader = false;
                        }
                        function.stats.lds_size_used       = GetHardwareStageProperty(function_node.second, kAmdgpuDisDotLdsSizeToken);
                        function.stats.scratch_memory_used = GetHardwareStageProperty(function_node.second, kAmdgpuDisDotScratchMemorySizeToken);
                        function.stats.num_sgprs_used      = GetHardwareStageProperty(function_node.second, kDotSgprCountToken);
                        function.stats.num_sgprs_available = GetHardwareStageProperty(function_node.second, kAmdgpuDisDotSgprLimitToken);
                        function.stats.num_vgprs_used      = GetHardwareStageProperty(function_node.second, kDotVgprCountToken);
                        function.stats.num_vgprs_available = GetHardwareStageProperty(function_node.second, kAmdgpuDisDotVgprLimitToken);
                        function.stats.wavefront_size      = GetHardwareStageProperty(function_node.second, kDotWavefrontSizeToken);

                        pipeline.shader_functions.push_back(function);
                    }

                    if (is_traditional_compute_shader)
                    {
                        status = beKA::beStatus::kBeStatusGraphicsCodeObjMetaDataSuccess;
                    }
                    else
                    {
                        status = beKA::beStatus::kBeStatusRayTracingCodeObjMetaDataSuccess;
                    }
                }
                else
                {
                    status = beKA::beStatus::kBeStatusGraphicsCodeObjMetaDataSuccess;
                }

                // Parse shader stages.
                for (const auto& shader_node : pipeline_node[kAmdgpuDisDotShadersToken])
                {
                    BeAmdPalMetaData::ShaderMetaData shader;
                    shader.shader_type = BeAmdPalMetaData::GetShaderType(shader_node.first.as<std::string>());
                    // Parse API shader hash
                    const YAML::Node& api_shader_hash_node = shader_node.second[kAmdgpuDisDotApiShaderHashToken];
                    if (api_shader_hash_node.IsSequence() && api_shader_hash_node.size() == 2)
                    {
                        shader.hash.low  = api_shader_hash_node[0].as<uint64_t>();
                        shader.hash.high = api_shader_hash_node[1].as<uint64_t>();
                    }
                    else
                    {
                        status = beKA::beStatus::kBeStatusCodeObjMdParsingFailed;
                        break;
                    }
                    const YAML::Node& hardware_mapping_node = shader_node.second[kAmdgpuDisDotHardwareMappingToken];
                    if (hardware_mapping_node.IsSequence() && hardware_mapping_node.size() > 0)
                    {
                        shader.hardware_mapping = BeAmdPalMetaData::GetStageType(hardware_mapping_node[0].as<std::string>());
                    }
                    else
                    {
                        status = beKA::beStatus::kBeStatusCodeObjMdParsingFailed;
                        break;
                    }
                    if (shader_node.second[kAmdgpuDisDotShaderSubtypeToken])
                    {
                        shader.shader_subtype = BeAmdPalMetaData::GetShaderSubtype(shader_node.second[kAmdgpuDisDotShaderSubtypeToken].as<std::string>());
                        if (shader.shader_subtype != BeAmdPalMetaData::ShaderSubtype::kUnknown)
                        {
                            status = beKA::beStatus::kBeStatusRayTracingCodeObjMetaDataSuccess;
                        }
                    }
                    pipeline.shaders.push_back(shader);
                }

                pipelines.push_back(pipeline);
            }
        }
        catch (const YAML::ParserException&)
        {
            status = beKA::beStatus::kBeStatusCodeObjMdParsingFailed;
            break;
        }
        catch (const std::runtime_error&)
        {
            status = beKA::beStatus::kBeStatusCodeObjMdParsingFailed;
            break;
        }

        start_offset = metadata_text.find(kStrLcCodeObjectMetadataTokenStart, end_offset);
        if (start_offset == std::string::npos)
        {
            break;
        }
    }

    if (status != beKA::beStatus::kBeStatusCodeObjMdParsingFailed && pipelines.size() > 0)
    {
        pipeline_md = std::move(pipelines[0]);
    }

    return status;
}

std::string BeAmdPalMetaData::GetShaderHashString(const ApiShaderHash& hash)
{
    std::ostringstream os;
    os << hash;
    return os.str();
}

// Parse an integer YAML node for CodeProps value.
// Do nothing if "oldResult" is false.
// If "zero_if_absent" is true, the value is set to 0 and "true" is returned if the required value is not found in the MD.
// (The Lightning Compiler does not generate some CodeProps values if they are 0).
static bool ParseCodePropsItem(const YAML::Node& code_props, const std::string& key, size_t& value, bool zero_if_absent = false)
{
    bool              result             = false;
    const YAML::Node& prop_metadata_node = code_props[key];
    if ((result = prop_metadata_node.IsDefined()) == true)
    {
        try
        {
            value = prop_metadata_node.as<size_t>();
        }
        catch (const YAML::TypedBadConversion<size_t>&)
        {
            result = false;
        }
    }
    else
    {
        if (zero_if_absent)
        {
            value  = 0;
            result = true;
        }
    }

    return result;
}

// Parse a single YAML node for kernel metadata.
static bool ParseKernelCodeProps(const YAML::Node& kernel_metadata, BeAmdHsaMetaData::KernelProperties& kernel_props)
{
    bool result = ParseCodePropsItem(kernel_metadata, kDotSgprCountToken, kernel_props.wavefront_num_sgprs, true);
    result      = result && ParseCodePropsItem(kernel_metadata, kDotVgprCountToken, kernel_props.work_item_num_vgprs, true);
    result      = result && ParseCodePropsItem(kernel_metadata, kDotAgprCountToken, kernel_props.work_item_num_agprs, true);
    result      = result && ParseCodePropsItem(kernel_metadata, kDotWavefrontSizeToken, kernel_props.wavefront_size);
    result      = result && ParseCodePropsItem(kernel_metadata, kStrCodeObjectMetadataKeyGroupSegmentSize, kernel_props.workgroup_segment_size, true);
    result      = result && ParseCodePropsItem(kernel_metadata, kStrCodeObjectMetadataKeyPrivateSegmentSize, kernel_props.private_segment_size, true);
    result      = result && ParseCodePropsItem(kernel_metadata, kStrCodeObjectMetadataKeySpilledSgprs, kernel_props.sgpr_spills, true);
    result      = result && ParseCodePropsItem(kernel_metadata, kStrCodeObjectMetadataKeySpilledVgprs, kernel_props.vgpr_spills, true);

    return result;
}

beKA::beStatus BeAmdHsaMetaData::ParseMetadata(const std::string& metadata_text, AmdHsaMetaData& md)
{
    beKA::beStatus status = beKA::beStatus::kBeStatusSuccess;
    size_t         start_offset, end_offset;
    YAML::Node     codeobj_metadata_node, kernels_metadata_map, kernel_name;
    start_offset = metadata_text.find(kStrLcCodeObjectMetadataTokenStart);

    while ((end_offset = metadata_text.find(kStrLcCodeObjectMetadataTokenEnd, start_offset)) != std::string::npos)
    {
        try
        {
            const std::string& kernel_metadata_text = metadata_text.substr(start_offset, end_offset - start_offset + kStrLcCodeObjectMetadataTokenEnd.size());
            codeobj_metadata_node                   = YAML::Load(kernel_metadata_text);
        }
        catch (YAML::ParserException&)
        {
            status = beKA::beStatus::kBeStatusLightningParseCodeObjMDFailed;
            break;
        }

        if (codeobj_metadata_node.IsMap() && (kernels_metadata_map = codeobj_metadata_node[kStrCodeObjectMetadataKeyKernels]).IsDefined())
        {
            for (const YAML::Node& kernel_metadata : kernels_metadata_map)
            {
                KernelProperties kernel_code_props;
                if (status == beKA::beStatus::kBeStatusSuccess && (kernel_name = kernel_metadata[kStrCodeObjectMetadataKeyKernelName]).IsDefined() &&
                    ParseKernelCodeProps(kernel_metadata, kernel_code_props))
                {
                    md.kernel_names.push_back(kernel_name.as<std::string>());
                    md.props_map[kernel_name.as<std::string>()] = kernel_code_props;
                }
                else
                {
                    status = beKA::beStatus::kBeStatusLightningParseCodeObjMDFailed;
                    break;
                }
            }
        }

        if (codeobj_metadata_node.IsMap() && (kernels_metadata_map = codeobj_metadata_node[kStrCodeObjectMetadataKeyDeviceTarget]).IsDefined())
        {
            auto device = kernels_metadata_map.as<std::string>();
            md.device   = device.substr(device.find("gfx"));
        }

        start_offset = metadata_text.find(kStrLcCodeObjectMetadataTokenStart, end_offset);
    }

    return status;
}

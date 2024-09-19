//=================================================================
// Copyright 2024 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

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

// Raytracing Shader Stage Dot Strings.
const std::string kStrUnknown       = "Unknown";
const std::string kStrRayGeneration = "RayGeneration";
const std::string kStrMiss          = "Miss";
const std::string kStrAnyHit        = "AnyHit";
const std::string kStrClosestHit    = "ClosestHit";
const std::string kStrIntersection  = "Intersection";
const std::string kStrCallable      = "Callable";
const std::string kStrTraversal     = "Traversal";
const std::string kStrLaunchKernel  = "LaunchKernel";

// Amdgpudis dot tokens.
static const std::string kStrLcCodeObjectMetadataTokenStart  = "---\n";
static const std::string kStrLcCodeObjectMetadataTokenEnd    = "\n...";
static const std::string kStrCodeObjectMetadataKeyKernels    = "amdhsa.kernels";
static const std::string kStrCodeObjectMetadataKeyPipelines  = "amdpal.pipelines";
static const std::string kAmdgpuDisDotApiToken               = ".api";
static const std::string kAmdgpuDisDotHardwareStagesToken    = ".hardware_stages";
static const std::string kAmdgpuDisDotScratchMemorySizeToken = ".scratch_memory_size";
static const std::string kAmdgpuDisDotSgprCountToken         = ".sgpr_count";
static const std::string kAmdgpuDisDotSgprLimitToken         = ".sgpr_limit";
static const std::string kAmdgpuDisDotVgprCountToken         = ".vgpr_count";
static const std::string kAmdgpuDisDotVgprLimitToken         = ".vgpr_limit";
static const std::string kAmdgpuDisDotShaderFunctionsToken   = ".shader_functions";
static const std::string kAmdgpuDisDotLdsSizeToken           = ".lds_size";
static const std::string kAmdgpuDisDotShaderSubtypeToken     = ".shader_subtype";
static const std::string kAmdgpuDisDotShadersToken           = ".shaders";
static const std::string kAmdgpuDisDotHardwareMappingToken   = ".hardware_mapping";
static const std::string kAmdgpuDisDxilStdManglingPrefix     = "\001?";
static const std::string kAmdgpuDisDxilStdManglingSuffix     = "@@";
static const std::string kAmdgpuDisDxilStdHexPattern         = "[A-F0-9]+:";


std::string BeMangledKernelUtils::DemangleShaderName(const std::string& kernel_name)
{
    std::string umangled_kernel_name{kernel_name};
    umangled_kernel_name = UnQuote(umangled_kernel_name, '\"');
    umangled_kernel_name = UnQuote(umangled_kernel_name, '\'');
    try
    {
        umangled_kernel_name = std::regex_replace(umangled_kernel_name, std::regex(kAmdgpuDisDxilStdHexPattern), "");
    }
    catch (...)
    {
        ;  // If unrecognized mangled name patter is detected, skip demangling.
    }
    std::size_t prefix = umangled_kernel_name.find(kAmdgpuDisDxilStdManglingPrefix);
    if (prefix != std::string::npos)
    {
        std::string name   = umangled_kernel_name.substr(prefix + kAmdgpuDisDxilStdManglingPrefix.size());
        std::size_t suffix = name.find(kAmdgpuDisDxilStdManglingSuffix);
        if (suffix != std::string::npos)
        {
            umangled_kernel_name = name.substr(0, suffix);
        }
    }
    return umangled_kernel_name;
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
    static const std::unordered_map<std::string, ShaderType> shaderMap = {{kStrVertex,   ShaderType::kVertex},
                                                                          {kStrHull,     ShaderType::kHull},
                                                                          {kStrDomain,   ShaderType::kDomain},
                                                                          {kStrGeometry, ShaderType::kGeometry},
                                                                          {kStrPixel,    ShaderType::kPixel},
                                                                          {kStrCompute,  ShaderType::kCompute}};

    auto it = shaderMap.find(shader_name);
    if (it != shaderMap.end())
    {
        return it->second;
    }
    throw std::runtime_error("Unknown shader type: " + shader_name);
}

BeAmdPalMetaData::ShaderSubtype BeAmdPalMetaData::GetShaderSubtype(const std::string& subtype_name)
{
    static const std::unordered_map<std::string, ShaderSubtype> subtypeMap = {{kStrUnknown,       ShaderSubtype::kUnknown},
                                                                              {kStrRayGeneration, ShaderSubtype::kRayGeneration},
                                                                              {kStrMiss,          ShaderSubtype::kMiss},
                                                                              {kStrAnyHit,        ShaderSubtype::kAnyHit},
                                                                              {kStrClosestHit,    ShaderSubtype::kClosestHit},
                                                                              {kStrIntersection,  ShaderSubtype::kIntersection},
                                                                              {kStrCallable,      ShaderSubtype::kCallable},
                                                                              {kStrTraversal,     ShaderSubtype::kTraversal},
                                                                              {kStrLaunchKernel,  ShaderSubtype::kLaunchKernel}};

    auto it = subtypeMap.find(subtype_name);
    if (it != subtypeMap.end())
    {
        return it->second;
    }
    throw std::runtime_error("Unknown shader subtype: " + subtype_name);
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
    static const std::unordered_map<ShaderType, const std::string&> inverseShaderMap = {{ShaderType::kVertex,   kStrVertex},
                                                                                        {ShaderType::kHull,     kStrHull},
                                                                                        {ShaderType::kDomain,   kStrDomain},
                                                                                        {ShaderType::kGeometry, kStrGeometry},
                                                                                        {ShaderType::kPixel,    kStrPixel},
                                                                                        {ShaderType::kCompute,  kStrCompute}};

    auto it = inverseShaderMap.find(shader_type);
    if (it != inverseShaderMap.end())
    {
        return it->second;
    }
    throw std::runtime_error("Unknown shader type: " + std::to_string(static_cast<int>(shader_type)));
}

std::string BeAmdPalMetaData::GetShaderSubtypeName(ShaderSubtype subtype)
{
    static const std::unordered_map<ShaderSubtype, const std::string&> inverseSubtypeMap = {{ShaderSubtype::kUnknown,       kStrUnknown},
                                                                                            {ShaderSubtype::kRayGeneration, kStrRayGeneration},
                                                                                            {ShaderSubtype::kMiss,          kStrMiss},
                                                                                            {ShaderSubtype::kAnyHit,        kStrAnyHit},
                                                                                            {ShaderSubtype::kClosestHit,    kStrClosestHit},
                                                                                            {ShaderSubtype::kIntersection,  kStrIntersection},
                                                                                            {ShaderSubtype::kCallable,      kStrCallable},
                                                                                            {ShaderSubtype::kTraversal,     kStrTraversal},
                                                                                            {ShaderSubtype::kLaunchKernel,  kStrLaunchKernel}};

    auto it = inverseSubtypeMap.find(subtype);
    if (it != inverseSubtypeMap.end())
    {
        return it->second;
    }
    throw std::runtime_error("Unknown shader subtype: " + std::to_string(static_cast<int>(subtype)));
}

uint64_t GetHardwareStageProperty(const YAML::Node& node, const std::string& key)
{
    uint64_t ret = static_cast<uint64_t>(-1);
    if (node[key])
    {
        ret = node[key].as<uint64_t>();
    }
    return ret;
}

beKA::beStatus BeAmdPalMetaData::ParseAmdgpudisMetadata(const std::string& amdgpu_dis_output, BeAmdPalMetaData::PipelineMetaData& pipeline_md)
{
    beKA::beStatus status       = beKA::beStatus::kBeStatusSuccess;
    size_t         start_offset = amdgpu_dis_output.find(kStrLcCodeObjectMetadataTokenStart);
    size_t         end_offset;

    std::vector<BeAmdPalMetaData::PipelineMetaData> pipelines;
    while ((end_offset = amdgpu_dis_output.find(kStrLcCodeObjectMetadataTokenEnd, start_offset)) != std::string::npos)
    {
        // For each metadata section ... 
        try
        {
            const std::string& kernel_metadata_text =
                amdgpu_dis_output.substr(start_offset, end_offset - start_offset + kStrLcCodeObjectMetadataTokenEnd.size());
            YAML::Node codeobj_metadata_node = YAML::Load(kernel_metadata_text);

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
                // Parse api token.
                BeAmdPalMetaData::PipelineMetaData pipeline;
                pipeline.api = pipeline_node[kAmdgpuDisDotApiToken].as<std::string>();

                // Parse stats for each hardware stage.
                for (const auto& stage_node : pipeline_node[kAmdgpuDisDotHardwareStagesToken])
                {
                    BeAmdPalMetaData::HardwareStageMetaData stage;

                    stage.stage_type                = BeAmdPalMetaData::GetStageType(stage_node.first.as<std::string>());
                    stage.stats.lds_size_used       = GetHardwareStageProperty(stage_node.second, kAmdgpuDisDotLdsSizeToken);
                    stage.stats.scratch_memory_used = GetHardwareStageProperty(stage_node.second, kAmdgpuDisDotScratchMemorySizeToken);
                    stage.stats.num_sgprs_used      = GetHardwareStageProperty(stage_node.second, kAmdgpuDisDotSgprCountToken);
                    stage.stats.num_sgprs_available = GetHardwareStageProperty(stage_node.second, kAmdgpuDisDotSgprLimitToken);
                    stage.stats.num_vgprs_used      = GetHardwareStageProperty(stage_node.second, kAmdgpuDisDotVgprCountToken);
                    stage.stats.num_vgprs_available = GetHardwareStageProperty(stage_node.second, kAmdgpuDisDotVgprLimitToken);

                    pipeline.hardware_stages.push_back(stage);
                }

                if (pipeline_node[kAmdgpuDisDotShaderFunctionsToken])
                {
                    // Parse shader functions.
                    for (const auto& function_node : pipeline_node[kAmdgpuDisDotShaderFunctionsToken])
                    {
                        BeAmdPalMetaData::ShaderFunctionMetaData function;

                        function.name           = BeMangledKernelUtils::DemangleShaderName(function_node.first.as<std::string>());
                        function.shader_subtype = BeAmdPalMetaData::GetShaderSubtype(function_node.second[kAmdgpuDisDotShaderSubtypeToken].as<std::string>());

                        function.stats.lds_size_used       = GetHardwareStageProperty(function_node.second, kAmdgpuDisDotLdsSizeToken);
                        function.stats.scratch_memory_used = GetHardwareStageProperty(function_node.second, kAmdgpuDisDotScratchMemorySizeToken);
                        function.stats.num_sgprs_used      = GetHardwareStageProperty(function_node.second, kAmdgpuDisDotSgprCountToken);
                        function.stats.num_sgprs_available = GetHardwareStageProperty(function_node.second, kAmdgpuDisDotSgprLimitToken);
                        function.stats.num_vgprs_used      = GetHardwareStageProperty(function_node.second, kAmdgpuDisDotVgprCountToken);
                        function.stats.num_vgprs_available = GetHardwareStageProperty(function_node.second, kAmdgpuDisDotVgprLimitToken);

                        pipeline.shader_functions.push_back(function);
                    }
                    status = beKA::beStatus::kBeStatusRayTracingCodeObjMetaDataSuccess;
                }
                else
                {
                    status = beKA::beStatus::kBeStatusGraphicsCodeObjMetaDataSuccess;
                }

                // Parse shader stages.
                for (const auto& shader_node : pipeline_node[kAmdgpuDisDotShadersToken])
                {
                    BeAmdPalMetaData::ShaderMetaData shader;
                    shader.shader_type                    = BeAmdPalMetaData::GetShaderType(shader_node.first.as<std::string>());
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

        start_offset = amdgpu_dis_output.find(kStrLcCodeObjectMetadataTokenStart, end_offset);
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

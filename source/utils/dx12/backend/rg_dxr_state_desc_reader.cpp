//=================================================================
// Copyright 2020 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

// C++.
#include <fstream>
#include <cassert>
#include <sstream>

// Local.
#include "rg_dxr_state_desc_reader.h"
#include "rg_dx12_utils.h"

// Json.
#include "json/json-3.2.0/single_include/nlohmann/json.hpp"

using namespace rga;

// Constants: JSON elements.
static const char* kStrJsonElemDxrState = "DXRState";
static const char* kStrJsonElemSchemaVersion = "SchemaVersion";
static const char* kStrJsonElemSchemaVersion10 = "1.0";
static const char* kStrJsonElemShaders = "Shaders";
static const char* kStrJsonElemType = "Type";
static const char* kStrJsonElemFilePath = "FilePath";
static const char* kStrJsonElemExports = "Exports";
static const char* kStrJsonElemFlags = "Flags";
static const char* kStrJsonElemDxilLibraryEntryPoints = "EntryPoints";
static const char* kStrJsonElemDxilLibraryTypeBinary = "Binary";
static const char* kStrJsonElemDxilLibraryTypeHlsl = "HLSL";
static const char* kStrJsonElemDxilLibraryExportName = "ExportName";
static const char* kStrJsonElemDxilLibraryLinkageName = "LinkageName";
static const char* kStrJsonElemHitgroups = "HitGroups";
static const char* kStrJsonElemHitgroupsName = "Name";
static const char* kStrJsonElemHitgroupsIntersectionShader = "IntersectionShader";
static const char* kStrJsonElemHitgroupsAnyHitShader = "AnyHitShader";
static const char* kStrJsonElemHitgroupsClosestHitShader = "ClosestHitShader";
static const char* kStrJsonElemHitgroupsTypeTriangles = "D3D12_HIT_GROUP_TYPE_TRIANGLES";
static const char* kStrJsonElemHitgroupsTypeProceduralPrimitive = "D3D12_HIT_GROUP_TYPE_PROCEDURAL_PRIMITIVE";
static const char* kStrJsonElemRootSignaturesLocal = "LocalRootSignatures";
static const char* kStrJsonElemRootSignaturesGlobal = "GlobalRootSignatures";
static const char* kStrJsonElemRaytracingPipelineConfig = "RaytracingPipelineConfig";
static const char* kStrJsonElemRaytracingPipelineConfigMaxTraceRecursionDepth = "MaxTraceRecursionDepth";
static const char* kStrJsonElemRaytracingPipelineConfigFlagsNone = "D3D12_RAYTRACING_PIPELINE_FLAG_NONE";
static const char* kStrJsonElemRaytracingPipelineConfigFlagsSkipTriangles = "D3D12_RAYTRACING_PIPELINE_FLAG_SKIP_TRIANGLES";
static const char* kStrJsonElemRaytracingPipelineConfigFlagsSkipProceduralPrimitives = "D3D12_RAYTRACING_PIPELINE_FLAG_SKIP_PROCEDURAL_PRIMITIVES";
static const char* kStrJsonElemRaytracingShaderConfig = "RaytracingShaderConfig";
static const char* kStrJsonElemRaytracingShaderConfigMaxPayloadSizeInBytes = "MaxPayloadSizeInBytes";
static const char* kStrJsonElemRaytracingShaderConfigMaxAttributeSizeInBytes = "MaxAttributeSizeInBytes";

// Constants: errors.
static const char* kStrErrorCannotOpenDxrStateJsonFile = "Error: could not open DXR state description file for reading: ";
static const char* kStrErrorCannotReadDxrStateJsonFile = "Error: failed to parse DXR state description file: ";
static const char* kStrErrorUnsupportedSchemaVersion = "Error: schema version not supported - this RGA version supports DXR JSON schema version 1.0 or below.";
static const char* kStrErrorUnknownShaderInputType = "Error: unknown shader input type, \"HLSL\" or \"Binary\" expected: ";
static const char* kStrErrorUnknownRootSignatureInputType1 = "Error: unknown ";
static const char* kStrErrorUnknownRootSignatureInputType2 = " root signature input type, \"HLSL\" or \"Binary\" expected: ";
static const char* kStrErrorUnknownGlobalRootSignatureInputType = "Error: unknown local root signature input type, \"HLSL\" or \"Binary\" expected: ";
static const char* kStrErrorUnknownHitgroupType = "Error: unknown shader input type, \"HLSL\" or \"Binary\" expected: ";
static const char* kStrErrorUnknownPipelineConfigFlags = "Error: unknown pipeline config flags: ";
static const char* kStrErrorNoFileTypeForRootSIgnatureElement1 = "Error: no file path found for ";
static const char* kStrErrorNoFileTypeForRootSIgnatureElement2 = " root signature element.";
static const char* kStrErrorDxrJsonElementNotFound = "Error: JSON element not found: ";
static const char* kStrErrorCouldNotFindSection1 = "Error: could not find ";
static const char* kStrErrorCouldNotFindSection2 = " section in DXR state JSON file.";

// Constants: warnings.
static const char* kStrWarningNoRootSignatureElementFound = "Warning: neither global nor local root signature element detected in DXR state JSON file. "
    "This might lead to an unsuccessful build since root signature information is required to create the DXR state object.";

static bool ExtractRootSignatures(const std::string& rs_type, nlohmann::basic_json<>::reference rs_element,
    std::string &error_msg, bool &should_abort, std::vector<std::shared_ptr<RgDxrRootSignature>>& root_signatures)
{
    for (const auto& curr_rs : rs_element)
    {
        if (!should_abort)
        {
            std::shared_ptr <RgDxrRootSignature> rs = std::make_shared<RgDxrRootSignature>();
            std::string input_type = curr_rs[kStrJsonElemType];
            assert(input_type.compare(kStrJsonElemDxilLibraryTypeHlsl) == 0 ||
                input_type.compare(kStrJsonElemDxilLibraryTypeBinary) == 0);
            if (input_type.compare(kStrJsonElemDxilLibraryTypeHlsl) == 0)
            {
                rs->input_type = DxrSourceType::kHlsl;
            }
            else if (input_type.compare(kStrJsonElemDxilLibraryTypeBinary) == 0)
            {
                rs->input_type = DxrSourceType::kBinary;
            }
            else
            {
                should_abort = true;
                std::stringstream msg;
                msg << kStrErrorUnknownRootSignatureInputType1 <<
                    rs_type << kStrErrorUnknownRootSignatureInputType2 << input_type;
                error_msg.append(msg.str());
            }

            if (!should_abort)
            {
                rs->full_path = curr_rs[kStrJsonElemFilePath].get<std::string>();
                assert(!rs->full_path.empty());
                if (!rs->full_path.empty())
                {
                    // Entry points.
                    auto entry_points_element = curr_rs[kStrJsonElemExports];
                    for (const std::string& export_name : entry_points_element)
                    {
                        if (!export_name.empty())
                        {
                            std::wstring export_name_wide = RgDx12Utils::strToWstr(export_name);
                            rs->exports.push_back(export_name_wide);
                        }
                    }

                    root_signatures.push_back(rs);
                }
                else
                {
                    should_abort = true;
                    std::stringstream msg;
                    msg << kStrErrorNoFileTypeForRootSIgnatureElement1 << rs_type <<
                        kStrErrorNoFileTypeForRootSIgnatureElement2;
                    error_msg.append(msg.str());
                }
            }
        }
    }

    return !should_abort;
}

bool rga::RgDxrStateDescReader::ReadDxrStateDesc(const std::string& json_file_path,
    RgDxrStateDesc& state_desc, std::string& error_msg)
{
    bool ret = false;
    bool should_abort = false;

    // Open a file to write the structure data to.
    std::ifstream file_stream;
    file_stream.open(json_file_path.c_str(), std::ofstream::in);

    assert(file_stream.is_open());
    if (file_stream.is_open())
    {
        // Read the JSON file.
        nlohmann::json structure;

        // Read the file into the JSON structure.
        try
        {
            // Try to read the file.
            file_stream >> structure;

            // Find the state element.
            auto elem = structure.find(kStrJsonElemDxrState);
            assert(elem != structure.end());
            assert(structure[kStrJsonElemDxrState] != nullptr);
            if (elem != structure.end() && structure[kStrJsonElemDxrState] != nullptr)
            {
                structure = structure[kStrJsonElemDxrState];
                assert(structure[kStrJsonElemSchemaVersion] != nullptr);
                if (structure[kStrJsonElemSchemaVersion] != nullptr)
                {
                    // Schema version.
                    std::string schema_version = structure[kStrJsonElemSchemaVersion];
                    assert(!schema_version.empty());
                    assert(schema_version.compare(kStrJsonElemSchemaVersion10) == 0);
                    if (!schema_version.empty() && schema_version.compare(kStrJsonElemSchemaVersion10) == 0)
                    {
                        // Shaders.
                        auto shaders_element_iter = structure.find(kStrJsonElemShaders);
                        assert(shaders_element_iter != structure.end());
                        assert(structure[kStrJsonElemShaders] != nullptr);
                        if (shaders_element_iter != structure.end() && structure[kStrJsonElemShaders] != nullptr)
                        {
                            auto shaders_element = structure[kStrJsonElemShaders];
                            for (const auto& shader : shaders_element)
                            {
                                std::shared_ptr<RgDxrDxilLibrary> lib = std::make_shared<RgDxrDxilLibrary>();
                                state_desc.input_files.push_back(lib);

                                // File type.
                                std::string input_type = shader[kStrJsonElemType];
                                assert(input_type.compare(kStrJsonElemDxilLibraryTypeHlsl) == 0 ||
                                    input_type.compare(kStrJsonElemDxilLibraryTypeBinary) == 0);

                                if (input_type.compare(kStrJsonElemDxilLibraryTypeHlsl) == 0)
                                {
                                    lib->input_type = DxrSourceType::kHlsl;
                                }
                                else if (input_type.compare(kStrJsonElemDxilLibraryTypeBinary) == 0)
                                {
                                    lib->input_type = DxrSourceType::kBinary;
                                }
                                else
                                {
                                    should_abort = true;
                                    std::stringstream msg;
                                    msg << kStrErrorUnknownShaderInputType << input_type;
                                    error_msg.append(msg.str());
                                }

                                if (!should_abort)
                                {
                                    // File path.
                                    auto elem = shader.find(kStrJsonElemFilePath);
                                    assert(elem != shader.end());
                                    if (elem != shader.end())
                                    {
                                        lib->full_path = shader[kStrJsonElemFilePath].get<std::string>();
                                        assert(!lib->full_path.empty());
                                    }
                                    else
                                    {
                                        should_abort = true;
                                        std::stringstream msg;
                                        msg << kStrErrorDxrJsonElementNotFound << kStrJsonElemFilePath << " under " << kStrJsonElemShaders;
                                        error_msg.append(msg.str());
                                    }
                                    assert(!should_abort);

                                    if (!should_abort)
                                    {
                                        // Entry points.
                                        auto entry_points_element = shader[kStrJsonElemDxilLibraryEntryPoints];
                                        for (const auto& ep : entry_points_element)
                                        {
                                            std::string export_name = ep[kStrJsonElemDxilLibraryExportName];
                                            std::string export_linkage_name = ep[kStrJsonElemDxilLibraryLinkageName];
                                            std::wstring export_name_wide = RgDx12Utils::strToWstr(export_name);
                                            std::wstring export_linkage_name__wide = RgDx12Utils::strToWstr(export_linkage_name);
                                            RgDxrExport currExport(export_name_wide, export_linkage_name__wide);
                                            lib->exports.push_back(currExport);
                                        }
                                    }
                                }
                            }
                        }
                        else
                        {
                            should_abort = true;
                            std::stringstream msg;
                            msg << kStrErrorCouldNotFindSection1 << kStrJsonElemShaders << kStrErrorCouldNotFindSection2;
                            error_msg.append(msg.str());
                        }

                        // Hit groups.
                        auto hitgroupsElementIter = structure.find(kStrJsonElemHitgroups);
                        if (!should_abort && hitgroupsElementIter != structure.end() && structure[kStrJsonElemHitgroups] != nullptr)
                        {
                            auto hitgroup_element = structure[kStrJsonElemHitgroups];
                            for (const auto& hitGroup : hitgroup_element)
                            {
                                RgDxrHitGroup hg;

                                // Type.
                                std::string hg_type = hitGroup[kStrJsonElemType];
                                assert(hg_type.compare(kStrJsonElemHitgroupsTypeTriangles) == 0 ||
                                    hg_type.compare(kStrJsonElemHitgroupsTypeProceduralPrimitive) == 0);
                                if (hg_type.compare(kStrJsonElemHitgroupsTypeTriangles) == 0)
                                {
                                    hg.type = D3D12_HIT_GROUP_TYPE_TRIANGLES;
                                }
                                else if (hg_type.compare(kStrJsonElemHitgroupsTypeProceduralPrimitive) == 0)
                                {
                                    hg.type = D3D12_HIT_GROUP_TYPE_PROCEDURAL_PRIMITIVE;
                                }
                                else
                                {
                                    std::stringstream msg;
                                    msg << kStrErrorUnknownHitgroupType << hg_type;
                                    error_msg.append(msg.str());
                                    should_abort = true;
                                }

                                if (!should_abort)
                                {
                                    // Name.
                                    auto hg_name_elem = hitGroup[kStrJsonElemHitgroupsName];
                                    std::string hg_name = (hg_name_elem != nullptr) ? hitGroup[kStrJsonElemHitgroupsName] : "";
                                    hg.hitgroup_name = RgDx12Utils::strToWstr(hg_name);

                                    // Shaders - intersection.
                                    auto intersection_elem = hitGroup[kStrJsonElemHitgroupsIntersectionShader];
                                    std::string intersection_shader = (intersection_elem != nullptr) ? hitGroup[kStrJsonElemHitgroupsIntersectionShader] : "";
                                    hg.intersection_shader = RgDx12Utils::strToWstr(intersection_shader);

                                    // Shaders - any hit.
                                    auto any_hit_elem = hitGroup[kStrJsonElemHitgroupsAnyHitShader];
                                    std::string any_hit_shader = (any_hit_elem != nullptr) ? hitGroup[kStrJsonElemHitgroupsAnyHitShader] : "";
                                    hg.any_hit_shader = RgDx12Utils::strToWstr(any_hit_shader);

                                    // Shaders - closest hit.
                                    auto closest_hit_elem = hitGroup[kStrJsonElemHitgroupsClosestHitShader];
                                    std::string closest_hit_shader = (closest_hit_elem != nullptr) ? hitGroup[kStrJsonElemHitgroupsClosestHitShader] : "";
                                    hg.closest_hit_shader = RgDx12Utils::strToWstr(closest_hit_shader);

                                    // Add the hit group to our collection.
                                    state_desc.hit_groups.push_back(hg);
                                }
                            }
                        }

                        bool is_any_rs_detected = false;
                        if (!should_abort)
                        {
                            // Local root signatures.
                            auto local_rs_iter = structure.find(kStrJsonElemRootSignaturesLocal);
                            if (local_rs_iter != structure.end() &&
                                structure[kStrJsonElemRootSignaturesLocal] != nullptr)
                            {
                                auto local_rs_element = structure[kStrJsonElemRootSignaturesLocal];
                                bool is_local_rs_extracted = ExtractRootSignatures("local", local_rs_element, error_msg, should_abort, state_desc.local_root_signature);
                                is_any_rs_detected = is_any_rs_detected || is_local_rs_extracted;
                            }
                        }

                        if (!should_abort)
                        {
                            // Global root signatures.
                            auto global_rs_iter = structure.find(kStrJsonElemRootSignaturesGlobal);
                            if (global_rs_iter != structure.end() &&
                                structure[kStrJsonElemRootSignaturesGlobal] != nullptr)
                            {
                                auto global_rs_element = structure[kStrJsonElemRootSignaturesGlobal];
                                bool is_global_rs_extracted = ExtractRootSignatures("global", global_rs_element, error_msg, should_abort, state_desc.global_root_signature);
                                is_any_rs_detected = is_any_rs_detected || is_global_rs_extracted;
                            }
                        }

                        if (!is_any_rs_detected)
                        {
                            std::stringstream msg;
                            msg << kStrWarningNoRootSignatureElementFound;
                            error_msg.append(msg.str());
                        }

                        if (!should_abort)
                        {
                            // Pipeline config.
                            auto pipeline_config_rs_iter = structure.find(kStrJsonElemRaytracingPipelineConfig);
                            if (pipeline_config_rs_iter != structure.end() &&
                                structure[kStrJsonElemRaytracingPipelineConfig] != nullptr)
                            {
                                // Assume pipeline config flags cannot be used in conjunction.
                                auto pipeline_config = structure[kStrJsonElemRaytracingPipelineConfig];
                                state_desc.pipeline_config.max_trace_recursion_depth = pipeline_config[kStrJsonElemRaytracingPipelineConfigMaxTraceRecursionDepth];

                                // Pipeline config flags are only supported in DXR 1.1, so we expect to not find them in 1.0 state files.
                                auto pipeline_config_flags_elem = pipeline_config[kStrJsonElemFlags];
                                if (pipeline_config_flags_elem != nullptr)
                                {
                                    std::string pipeline_config_flags = pipeline_config[kStrJsonElemFlags];
                                    if (pipeline_config_flags.empty() || pipeline_config_flags.compare(kStrJsonElemRaytracingPipelineConfigFlagsNone) == 0)
                                    {
                                        state_desc.pipeline_config.flags = 0;
                                    }
                                    else if (pipeline_config_flags.compare(kStrJsonElemRaytracingPipelineConfigFlagsSkipTriangles) == 0)
                                    {
                                        state_desc.pipeline_config.flags = 0x100;
                                    }
                                    else if (pipeline_config_flags.compare(kStrJsonElemRaytracingPipelineConfigFlagsSkipProceduralPrimitives) == 0)
                                    {
                                        state_desc.pipeline_config.flags = 0x200;
                                    }
                                    else
                                    {
                                        should_abort = true;
                                        std::stringstream msg;
                                        msg << kStrErrorUnknownPipelineConfigFlags << pipeline_config_flags;
                                        error_msg.append(msg.str());
                                    }
                                }
                                else
                                {
                                    state_desc.pipeline_config.flags = 0;
                                }
                            }
                        }

                        if (!should_abort)
                        {
                            // Shader config elements.
                            auto shader_config_element_iter = structure.find(kStrJsonElemRaytracingShaderConfig);
                            if (!should_abort && shader_config_element_iter != structure.end() && structure[kStrJsonElemRaytracingShaderConfig] != nullptr)
                            {
                                auto shader_config_element = structure[kStrJsonElemRaytracingShaderConfig];
                                for (const auto& config : shader_config_element)
                                {
                                    RgDxrShaderConfig sc;
                                    sc.max_payload_size_in_bytes = config[kStrJsonElemRaytracingShaderConfigMaxPayloadSizeInBytes];
                                    sc.max_attribute_size_in_bytes = config[kStrJsonElemRaytracingShaderConfigMaxAttributeSizeInBytes];

                                    // Export names.
                                    auto exports_element = config[kStrJsonElemExports];
                                    for (const std::string& export_name : exports_element)
                                    {
                                        if (!export_name.empty())
                                        {
                                            std::wstring export_name_wide = RgDx12Utils::strToWstr(export_name);
                                            sc.exports.push_back(export_name_wide);
                                        }
                                    }

                                    // Update the output structure.
                                    state_desc.shader_config.push_back(sc);
                                }
                            }
                        }
                    }
                    else
                    {
                        // Failed reading: report error.
                        should_abort = true;
                        std::stringstream msg;
                        msg << kStrErrorUnsupportedSchemaVersion << json_file_path;
                        error_msg.append(msg.str());
                    }
                }
                else
                {
                    should_abort = true;
                    std::stringstream msg;
                    msg << kStrErrorDxrJsonElementNotFound << kStrJsonElemSchemaVersion;
                    error_msg.append(msg.str());
                }
            }
            else
            {
                should_abort = true;
                std::stringstream msg;
                msg << kStrErrorDxrJsonElementNotFound << kStrJsonElemDxrState;
                error_msg.append(msg.str());
            }

            // Success if we encountered no error.
            ret = !should_abort;
        }
        catch (...)
        {
            // Failed reading: report error.
            std::stringstream msg;
            msg << kStrErrorCannotReadDxrStateJsonFile << json_file_path;
            error_msg.append(msg.str());
            ret = false;
        }

        // Close the file.
        file_stream.close();
    }
    else
    {
        // Cannot open file: report error.
        std::stringstream msg;
        msg << kStrErrorCannotOpenDxrStateJsonFile << json_file_path;
        error_msg.append(msg.str());
        ret = false;
    }

    return ret;
}

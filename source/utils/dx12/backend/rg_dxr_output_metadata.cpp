//=================================================================
// Copyright 2020 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

// C++.
#include <fstream>
#include <cassert>
#include <sstream>

// Local.
#include "rg_dxr_output_metadata.h"
#include "rg_dx12_utils.h"

// Json.
#include "json/json-3.11.3/single_include/nlohmann/json.hpp"

// Constants: JSON elements.
static const char* kDxrOutputMetadataJsonElemSchemaVersion        = "SchemaVersion";
static const char* kDxrOutputMetadataJsonElemSchemaVersionDefault = "1.0";
static const char* kDxrOutputMetadataJsonElemPipelines            = "Pipelines";
static const char* kDxrOutputMetadataJsonPipelineBinary           = "PipelineBinary";

// Constants: error messages.
static const char* kDxrOutputMetadataErrorFailedToWriteFile = "Error: failed to write JSON output metadata file.";
static const char* kDxrOutputMetadataErrorFailedToReadFile = "Error: failed to read JSON output metadata file.";

bool rga::RgDxrOutputMetadata::WriteOutputMetadata(const std::string& json_filename, const std::vector<RgDxrPipelineResults> pipeline_results, std::string& error_msg)
{
    bool ret = false;
    try
    {
        nlohmann::json structure;

        // Schema version.
        structure[kDxrOutputMetadataJsonElemSchemaVersion] = kDxrOutputMetadataJsonElemSchemaVersionDefault;
        for (uint32_t pipeline_index = 0; pipeline_index < pipeline_results.size(); pipeline_index++)
        {
            // Pipeline binary.
            structure[kDxrOutputMetadataJsonElemPipelines][pipeline_index][kDxrOutputMetadataJsonPipelineBinary] = pipeline_results[pipeline_index].pipeline_binary;
        }

        // Write the JSON file.
        std::ofstream file_stream;
        file_stream.open(json_filename, std::ofstream::out);
        assert(file_stream.is_open());
        if (file_stream.is_open())
        {
            file_stream << structure.dump(4);
            file_stream.close();
            ret = true;
        }
    }
    catch (...)
    {
        error_msg.append(kDxrOutputMetadataErrorFailedToWriteFile);
        ret = false;
    }
    return ret;
}

bool rga::RgDxrOutputMetadata::ReadOutputMetadata(const std::string& json_filename, std::vector<RgDxrPipelineResults>& pipeline_results, std::string& error_msg)
{
    bool ret = true;

    // Open a file to write the structure data to.
    std::ifstream file_stream;
    file_stream.open(json_filename.c_str(), std::ofstream::in);

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

            // Find the Pipelines element.
            auto pipelines_elem_iter = structure.find(kDxrOutputMetadataJsonElemPipelines);
            if (pipelines_elem_iter != structure.end() && structure[kDxrOutputMetadataJsonElemPipelines] != nullptr)
            {
                auto pipelines_element = structure[kDxrOutputMetadataJsonElemPipelines];
                for (const auto& pipeline : pipelines_element)
                {
                    RgDxrPipelineResults curr_pipeline_result;

                    // Binary.
                    std::string pipeline_binary_path = pipeline[kDxrOutputMetadataJsonPipelineBinary];
                    curr_pipeline_result.pipeline_binary = pipeline_binary_path;

                    // Track pipeline results.
                    pipeline_results.push_back(curr_pipeline_result);
                }
            }
        }
        catch (...)
        {
            // Failed reading: report error.
            std::stringstream msg;
            msg << kDxrOutputMetadataErrorFailedToReadFile << json_filename;
            error_msg.append(msg.str());
            ret = false;
        }

        // Close the file.
        file_stream.close();
    }

    return ret;
}

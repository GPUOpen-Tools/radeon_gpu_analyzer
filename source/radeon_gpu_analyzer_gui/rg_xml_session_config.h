//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for class parsing session metadata.
//=============================================================================
#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_XML_SESSION_CONFIG_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_XML_SESSION_CONFIG_H_

// C++.
#include <memory>

// Local.
#include "radeon_gpu_analyzer_gui/rg_data_types.h"

// Forward declarations.
namespace tinyxml2
{
    class XMLNode;
}

// A class responsible for parsing session metadata.
class RgXMLSessionConfig
{
public:
    // Read the OpenCL project session metadata XML at the given file path.
    static bool ReadSessionMetadataOpenCL(const std::string& session_metadata_file_path, std::shared_ptr<RgCliBuildOutputOpencl>& cli_output);

    // Read the Vulkan Pipeline session metadata XML at the given file path.
    static bool ReadSessionMetadataVulkan(const std::string&                         session_metadata_file_path,
                                          std::shared_ptr<RgCliBuildOutputPipeline>& cli_output,
                                          bool                                       is_codeobj_input_file = false);

    // Remove the node for the specified binary file from the session metadata xml.
    static bool RemoveBinaryFileFromMetadata(const std::string& session_metadata_file_path, const std::string& target_binary_full_file_path);

private:
    // Read the outputs node into an RgOutputItem array.
    static bool ReadBuildOutputs(tinyxml2::XMLNode* outputs_node, std::vector<RgOutputItem>& output);

    // Read an entry's output fields.
    static bool ReadEntryOutputs(tinyxml2::XMLNode* output_entry_node, RgFileOutputs& outputs);

    // Read each input file node's outputs.
    static bool ReadInputFiles(tinyxml2::XMLNode* input_file_node, std::shared_ptr<RgCliBuildOutputOpencl>& cli_output, std::string binary_file_name = "");

    // Read a pipeline stage's output file paths.
    static bool ReadPipelineStageOutputs(tinyxml2::XMLNode* outputs_node, RgEntryOutput& stage_output);
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_XML_SESSION_CONFIG_H_

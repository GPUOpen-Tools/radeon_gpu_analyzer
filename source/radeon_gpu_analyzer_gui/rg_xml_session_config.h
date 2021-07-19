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
    static bool ReadSessionMetadataVulkan(const std::string& session_metadata_file_path, std::shared_ptr<RgCliBuildOutputPipeline>& cli_output);

private:
    // Read the outputs node into an RgOutputItem array.
    static bool ReadBuildOutputs(tinyxml2::XMLNode* outputs_node, std::vector<RgOutputItem>& output);

    // Read an entry's output fields.
    static bool ReadEntryOutputs(tinyxml2::XMLNode* output_entry_node, RgFileOutputs& outputs);

    // Read each input file node's outputs.
    static bool ReadInputFiles(tinyxml2::XMLNode* input_file_node, std::shared_ptr<RgCliBuildOutputOpencl>& cli_output);

    // Read a pipeline stage's output file paths.
    static bool ReadPipelineStageOutputs(tinyxml2::XMLNode* outputs_node, RgEntryOutput& stage_output);
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_XML_SESSION_CONFIG_H_

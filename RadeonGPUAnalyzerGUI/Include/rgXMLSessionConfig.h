#pragma once

// C++.
#include <memory>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/rgDataTypes.h>

// Forward declarations.
namespace tinyxml2
{
    class XMLNode;
}

// A class responsible for parsing session metadata.
class rgXMLSessionConfig
{
public:
    // Read the OpenCL project session metadata XML at the given file path.
    static bool ReadSessionMetadataOpenCL(const std::string& sessionMetadataFilePath, std::shared_ptr<rgCliBuildOutputOpenCL>& pCliOutput);

    // Read the Vulkan Pipeline session metadata XML at the given file path.
    static bool ReadSessionMetadataVulkan(const std::string& sessionMetadataFilePath, std::shared_ptr<rgCliBuildOutputPipeline>& pCliOutput);

private:
    // Read the outputs node into an rgOutputItem array.
    static bool ReadBuildOutputs(tinyxml2::XMLNode* pOutputsNode, std::vector<rgOutputItem>& output);

    // Read an entry's output fields.
    static bool ReadEntryOutputs(tinyxml2::XMLNode* pOutputEntryNode, rgFileOutputs& pOutputs);

    // Read each input file node's outputs.
    static bool ReadInputFiles(tinyxml2::XMLNode* pInputFileNode, std::shared_ptr<rgCliBuildOutputOpenCL>& pCliOutput);

    // Read a pipeline stage's output file paths.
    static bool ReadPipelineStageOutputs(tinyxml2::XMLNode* pOutputsNode, rgEntryOutput& stageOutput);
};
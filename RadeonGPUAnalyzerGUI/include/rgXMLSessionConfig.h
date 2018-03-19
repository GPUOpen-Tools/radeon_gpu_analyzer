#pragma once

// C++.
#include <memory>

// Local.
#include <RadeonGPUAnalyzerGUI/include/rgDataTypes.h>

// Forward declarations.
namespace tinyxml2
{
    class XMLNode;
}

// A class responsible for parsing session metadata.
class rgXMLSessionConfig
{
public:
    // Read the session metadata XML at the given file path.
    static bool ReadSessionMetadata(const std::string& sessionMetadataFilePath, std::shared_ptr<rgCliBuildOutput>& pCliOutput);

private:
    // Read the outputs node into an rgOutputItem array.
    static bool ReadBuildOutputs(tinyxml2::XMLNode* pOutputsNode, std::vector<rgOutputItem>& output);

    // Read an entry's output fields.
    static bool ReadEntryOutputs(tinyxml2::XMLNode* pOutputEntryNode, rgFileOutputs& pOutputs);

    // Read each input file node's outputs.
    static bool ReadInputFiles(tinyxml2::XMLNode* pInputFileNode, std::shared_ptr<rgCliBuildOutput>& pCliOutput);
};
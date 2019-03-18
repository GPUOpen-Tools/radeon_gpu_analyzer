// C++.
#include <cassert>
#include <algorithm>

// XML.
#include <tinyxml2/Include/tinyxml2.h>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/rgResourceUsageCsvFileParser.h>
#include <RadeonGPUAnalyzerGUI/Include/rgStringConstants.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDefinitions.h>
#include <RadeonGPUAnalyzerGUI/Include/rgUtils.h>
#include <RadeonGPUAnalyzerGUI/Include/rgXMLSessionConfig.h>
#include <RadeonGPUAnalyzerGUI/Include/rgXMLUtils.h>
#include <Utils/Include/rgaXMLConstants.h>

bool rgXMLSessionConfig::ReadSessionMetadataOpenCL(const std::string& sessionMetadataFilePath, std::shared_ptr<rgCliBuildOutputOpenCL>& pCliOutput)
{
    bool ret = false;

    // Create the CLI output variable.
    pCliOutput = std::make_shared<rgCliBuildOutputOpenCL>();

    // Load the XML document.
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError rc = doc.LoadFile(sessionMetadataFilePath.c_str());

    if (rc == tinyxml2::XML_SUCCESS)
    {
        // Get the XML declaration node.
        tinyxml2::XMLNode* pNode = doc.FirstChild();
        if (pNode != nullptr)
        {
            // Traverse to the program metadata root element.
            pNode = pNode->NextSiblingElement(XML_NODE_METADATA);
            if (pNode != nullptr)
            {
                // Parse the metadata contents, starting with the output binary and input files.
                tinyxml2::XMLNode* pBinaryFilePathNode = pNode->FirstChildElement(XML_NODE_BINARY);
                if (pBinaryFilePathNode != nullptr)
                {
                    // Read the full file path to the compiled binary output.
                    std::string binaryFullFilePath;
                    ret = rgXMLUtils::ReadNodeTextString(pBinaryFilePathNode, binaryFullFilePath);

                    if (ret)
                    {
                        // Set the output's binary path to what was read from the node text.
                        pCliOutput->m_projectBinary = binaryFullFilePath;

                        // Parse the metadata contents, starting with the output binary and input files.
                        tinyxml2::XMLNode* pInputFileNode = pNode->FirstChildElement(XML_NODE_INPUT_FILE);
                        if (pInputFileNode != nullptr)
                        {
                            // Read all of the input file nodes.
                            ret = ReadInputFiles(pInputFileNode, pCliOutput);
                            assert(ret);
                        }
                    }
                }
            }
        }
    }

    return ret;
}

bool rgXMLSessionConfig::ReadSessionMetadataVulkan(const std::string& sessionMetadataFilePath, std::shared_ptr<rgCliBuildOutputPipeline>& pCliOutput)
{
    bool ret = false;

    // Create the CLI output variable.
    pCliOutput = std::make_shared<rgCliBuildOutputPipeline>();

    // Load the XML document.
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError rc = doc.LoadFile(sessionMetadataFilePath.c_str());

    if (rc == tinyxml2::XML_SUCCESS)
    {
        // Get the root pipeline metadata element. Pipeline outputs appear as children elements.
        tinyxml2::XMLNode* pPipelineMetadataNode = doc.FirstChildElement(XML_NODE_METADATA_PIPELINE);
        assert(pPipelineMetadataNode != nullptr);
        if (pPipelineMetadataNode != nullptr)
        {
            // Get the XML declaration node.
            tinyxml2::XMLNode* pPipelineNode = pPipelineMetadataNode->FirstChildElement(XML_NODE_PIPELINE);
            if (pPipelineNode != nullptr)
            {
                // Determine the type of pipeline that was built.
                tinyxml2::XMLNode* pPipelineTypeNode = pPipelineNode->FirstChildElement(XML_NODE_TYPE);
                if (pPipelineTypeNode != nullptr)
                {
                    // Read the pipeline type string.
                    std::string pipelineTypeString;
                    ret = rgXMLUtils::ReadNodeTextString(pPipelineTypeNode, pipelineTypeString);

                    assert(ret);
                    if (ret)
                    {
                        // Set the pipeline type based on the node text.
                        pCliOutput->m_type = (pipelineTypeString.compare(XML_NODE_PIPELINE_TYPE_GRAPHICS) == 0) ?
                            rgPipelineType::Graphics : rgPipelineType::Compute;

                        // Search for each stage element.
                        tinyxml2::XMLNode* pStageNode = pPipelineNode->FirstChildElement(XML_NODE_STAGE);
                        while (pStageNode != nullptr)
                        {
                            rgFileOutputs fileOutputs;

                            rgEntryOutput stageEntry = {};
                            stageEntry.m_entrypointName = STR_DEFAULT_VULKAN_GLSL_ENTRYPOINT_NAME;

                            // Search for the stage type node.
                            tinyxml2::XMLNode* pStageTypeNode = pStageNode->FirstChildElement(XML_NODE_TYPE);

                            assert(pStageTypeNode != nullptr);
                            if (pStageTypeNode != nullptr)
                            {
                                // Read the stage type node.
                                std::string pipelineStageTypeString;
                                ret = rgXMLUtils::ReadNodeTextString(pStageTypeNode, pipelineStageTypeString);

                                assert(ret);
                                if (ret)
                                {
                                    // Assign the stage type in the file's outputs struct.
                                    stageEntry.m_kernelType = pipelineStageTypeString;
                                }
                            }

                            assert(ret);
                            if (ret)
                            {
                                // Search for the input file node.
                                tinyxml2::XMLNode* pInputFileNode = pStageNode->FirstChildElement(XML_NODE_INPUT_FILE);
                                assert(pInputFileNode != nullptr);
                                if (pInputFileNode != nullptr)
                                {
                                    pInputFileNode = pInputFileNode->FirstChildElement(XML_NODE_PATH);
                                    assert(pInputFileNode != nullptr);
                                    if (pInputFileNode != nullptr)
                                    {
                                        // Read the input file path.
                                        std::string inputFilePathString;
                                        ret = rgXMLUtils::ReadNodeTextString(pInputFileNode, inputFilePathString);
                                        if (ret)
                                        {
                                            stageEntry.m_inputFilePath = inputFilePathString;
                                        }
                                    }
                                }
                            }

                            assert(ret);
                            if (ret)
                            {
                                // Find the stage node's output node.
                                tinyxml2::XMLNode* pStageOutputNode = pStageNode->FirstChildElement(XML_NODE_OUTPUT);
                                assert(pStageOutputNode != nullptr);
                                if (pStageOutputNode != nullptr)
                                {
                                    // Read the stage's output files.
                                    ret = ret && ReadPipelineStageOutputs(pStageOutputNode, stageEntry);
                                }
                            }

                            assert(ret);
                            if (ret)
                            {
                                fileOutputs.m_inputFilePath = stageEntry.m_inputFilePath;

                                fileOutputs.m_outputs.push_back(stageEntry);
                                pCliOutput->m_perFileOutput[stageEntry.m_inputFilePath] = fileOutputs;

                                // Search for the next stage node.
                                pStageNode = pStageNode->NextSiblingElement(XML_NODE_STAGE);
                            }


                            // If anything went wrong, end this loop.
                            if (!ret)
                            {
                                pStageNode = nullptr;
                            }
                        }
                    }
                }
            }
        }
    }

    return ret;
}

bool rgXMLSessionConfig::ReadBuildOutputs(tinyxml2::XMLNode* pOutputsNode, std::vector<rgOutputItem>& outputs)
{
    bool ret = false;

    // Extract the target GPU name.
    tinyxml2::XMLNode* pTargetGpuNode = pOutputsNode->FirstChildElement(XML_NODE_TARGET);

    while (pOutputsNode != nullptr)
    {
        rgOutputItem outputItem = {};

        if (pTargetGpuNode != nullptr)
        {
            // Read the target GPU that the output was generated for.
            std::string targetAsic;
            ret = rgXMLUtils::ReadNodeTextString(pTargetGpuNode, targetAsic);
            assert(ret);

            if (ret)
            {
                // Find the ISA disassembly text file path node.
                tinyxml2::XMLNode* pIsaDisassemblyTextNode = pOutputsNode->FirstChildElement(XML_NODE_ISA);
                if (pIsaDisassemblyTextNode != nullptr)
                {
                    // Read the file path to the disassembled ISA text.
                    std::string isaDisassemlyTextFilePath;
                    ret = rgXMLUtils::ReadNodeTextString(pIsaDisassemblyTextNode, isaDisassemlyTextFilePath);
                    assert(ret);

                    if (ret)
                    {
                        rgOutputItem csvOutputItem = { isaDisassemlyTextFilePath, targetAsic, rgCliOutputFileType::IsaDisassemblyText };
                        outputs.push_back(csvOutputItem);
                    }
                }

                // Find the ISA disassembly CSV file path node.
                tinyxml2::XMLNode* pIsaDisassemblyCsvNode = pOutputsNode->FirstChildElement(XML_NODE_CSV_ISA);
                if (pIsaDisassemblyCsvNode != nullptr)
                {
                    // Read the file path to the disassembled ISA CSV.
                    std::string isaDisassemlyCsvFilePath;
                    ret = rgXMLUtils::ReadNodeTextString(pIsaDisassemblyCsvNode, isaDisassemlyCsvFilePath);
                    assert(ret);

                    if (ret)
                    {
                        rgOutputItem csvOutputItem = { isaDisassemlyCsvFilePath, targetAsic, rgCliOutputFileType::IsaDisassemblyCsv };
                        outputs.push_back(csvOutputItem);
                    }
                }

                // Find the resource usage CSV file path node.
                tinyxml2::XMLNode* pResourceUsageCsvNode = pOutputsNode->FirstChildElement(XML_NODE_RES_USAGE);
                if (pResourceUsageCsvNode != nullptr)
                {
                    // Read the file path to the resource usage CSV file.
                    std::string resourceUsageCsvFilePath;
                    ret = rgXMLUtils::ReadNodeTextString(pResourceUsageCsvNode, resourceUsageCsvFilePath);
                    assert(ret);

                    if (ret)
                    {
                        rgOutputItem csvOutputItem = { resourceUsageCsvFilePath, targetAsic, rgCliOutputFileType::HwResourceUsageFile };
                        outputs.push_back(csvOutputItem);
                    }
                }
            }
        }

        // Try to find additional output nodes to parse.
        pOutputsNode = pOutputsNode->NextSibling();
    }

    return ret;
}

bool rgXMLSessionConfig::ReadEntryOutputs(tinyxml2::XMLNode* pOutputEntryNode, rgFileOutputs& pOutputs)
{
    bool ret = false;

    // Add each output to the input file's list of outputs.
    do
    {
        rgEntryOutput entry;
        // Extract the compiled kernel name.
        tinyxml2::XMLNode* pKernelNameNode = pOutputEntryNode->FirstChildElement(XML_NODE_NAME);
        if (pKernelNameNode != nullptr)
        {
            // Read the full file path to the input file.
            ret = rgXMLUtils::ReadNodeTextString(pKernelNameNode, entry.m_entrypointName);
            assert(ret);

            if (ret)
            {
                // Extract the kernel type.
                tinyxml2::XMLNode* pKernelTypeNode = pOutputEntryNode->FirstChildElement(XML_NODE_TYPE);
                if (pKernelTypeNode != nullptr)
                {
                    // Read the full file path to the input file.
                    ret = rgXMLUtils::ReadNodeTextString(pKernelTypeNode, entry.m_kernelType);
                    assert(ret);

                    if (ret)
                    {
                        // Find the the compiled outputs node.
                        tinyxml2::XMLNode* pOutputsNode = pOutputEntryNode->FirstChildElement(XML_NODE_OUTPUT);
                        if (pOutputsNode != nullptr)
                        {
                            // Read all of the build outputs.
                            ret = ReadBuildOutputs(pOutputsNode, entry.m_outputs);
                            assert(ret);

                            if (ret)
                            {
                                // Assign the input file path to each output entry so we know where it originated from.
                                entry.m_inputFilePath = pOutputs.m_inputFilePath;

                                // Add the populated output structure to the array of outputs for the given file.
                                pOutputs.m_outputs.push_back(entry);
                            }
                        }
                    }
                }
            }
        }

        // Move to the next output entry.
        pOutputEntryNode = pOutputEntryNode->NextSibling();
    } while (pOutputEntryNode != nullptr);

    return ret;
}

bool rgXMLSessionConfig::ReadInputFiles(tinyxml2::XMLNode* pInputFileNode, std::shared_ptr<rgCliBuildOutputOpenCL>& pCliOutput)
{
    bool ret = false;

    // Loop over each input file's outputs.
    do
    {
        // Parse the input file's info.
        tinyxml2::XMLNode* pInputFilePathNode = pInputFileNode->FirstChildElement(XML_NODE_PATH);
        if (pInputFilePathNode != nullptr)
        {
            // Read the full file path to the input file.
            std::string inputFullFilePath;
            ret = rgXMLUtils::ReadNodeTextString(pInputFilePathNode, inputFullFilePath);

            if (ret)
            {
                // Process the input file's output entry info.
                tinyxml2::XMLNode* pOutputEntryNode = pInputFileNode->FirstChildElement(XML_NODE_ENTRY);
                if (pOutputEntryNode != nullptr)
                {
                    rgFileOutputs& fileOutputs = pCliOutput->m_perFileOutput[inputFullFilePath];
                    fileOutputs.m_inputFilePath = inputFullFilePath;
                    ret = ReadEntryOutputs(pOutputEntryNode, fileOutputs);
                    assert(ret);
                }
            }
        }

        // Find the next input file sibling node.
        pInputFileNode = pInputFileNode->NextSibling();
    } while (pInputFileNode != nullptr);

    return ret;
}

bool rgXMLSessionConfig::ReadPipelineStageOutputs(tinyxml2::XMLNode* pOutputsNode, rgEntryOutput& stageOutput)
{
    bool ret = false;

    // Search for the listed stage output node strings.
    static const std::string s_STAGE_OUTPUT_NODES[] =
    {
        XML_NODE_ISA,
        XML_NODE_CSV_ISA,
        XML_NODE_RES_USAGE
    };

    // Loop to search for each possible output type.
    for (auto outputIter = std::begin(s_STAGE_OUTPUT_NODES);
        outputIter != std::end(s_STAGE_OUTPUT_NODES); ++outputIter)
    {
        // Search for given output type.
        const std::string& outputTypeString = *outputIter;
        tinyxml2::XMLNode* pOutputNode = pOutputsNode->FirstChildElement(outputTypeString.c_str());
        if (pOutputNode != nullptr)
        {
            rgOutputItem csvOutputItem = {};

            // Read the output file path.
            std::string outputFilePath;
            ret = rgXMLUtils::ReadNodeTextString(pOutputNode, outputFilePath);
            assert(ret);
            if (ret)
            {
                // Assign the parsed file path into the output.
                csvOutputItem.m_filePath = outputFilePath;

                // Assign the file path in the output object.
                if (outputTypeString.compare(XML_NODE_ISA) == 0)
                {
                    csvOutputItem.m_fileType = rgCliOutputFileType::IsaDisassemblyText;
                }
                else if (outputTypeString.compare(XML_NODE_CSV_ISA) == 0)
                {
                    csvOutputItem.m_fileType = rgCliOutputFileType::IsaDisassemblyCsv;
                }
                else if (outputTypeString.compare(XML_NODE_RES_USAGE) == 0)
                {
                    csvOutputItem.m_fileType = rgCliOutputFileType::HwResourceUsageFile;
                }
                else
                {
                    // The node text wasn't recognized.
                    assert(false);
                    ret = false;
                }

                if (ret)
                {
                    stageOutput.m_outputs.push_back(csvOutputItem);
                }
            }
        }
    }

    return ret;
}

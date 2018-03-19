// C++.
#include <cassert>
#include <algorithm>

// XML.
#include <tinyxml2/Include/tinyxml2.h>

// Local.
#include <RadeonGPUAnalyzerGUI/include/rgResourceUsageCsvFileParser.h>
#include <RadeonGPUAnalyzerGUI/include/rgStringConstants.h>
#include <RadeonGPUAnalyzerGUI/include/rgUtils.h>
#include <RadeonGPUAnalyzerGUI/include/rgXMLSessionConfig.h>
#include <RadeonGPUAnalyzerGUI/include/rgXMLUtils.h>
#include <Utils/include/rgaXMLConstants.h>

bool rgXMLSessionConfig::ReadSessionMetadata(const std::string& sessionMetadataFilePath, std::shared_ptr<rgCliBuildOutput>& pCliOutput)
{
    bool ret = false;

    // Create the CLI output variable.
    pCliOutput = std::make_shared<rgCliBuildOutput>();

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
                    assert(ret);

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
            ret = rgXMLUtils::ReadNodeTextString(pKernelNameNode, entry.m_kernelName);
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

bool rgXMLSessionConfig::ReadInputFiles(tinyxml2::XMLNode* pInputFileNode, std::shared_ptr<rgCliBuildOutput>& pCliOutput)
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
//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for class parsing session metadata.
//=============================================================================
// C++.
#include <cassert>
#include <algorithm>

// XML.
#include "tinyxml2.h"

// Local.
#include "radeon_gpu_analyzer_gui/rg_resource_usage_csv_file_parser.h"
#include "radeon_gpu_analyzer_gui/rg_string_constants.h"
#include "radeon_gpu_analyzer_gui/rg_definitions.h"
#include "radeon_gpu_analyzer_gui/rg_utils.h"
#include "radeon_gpu_analyzer_gui/rg_xml_session_config.h"
#include "radeon_gpu_analyzer_gui/rg_xml_utils.h"
#include "source/common/rga_xml_constants.h"

bool RgXMLSessionConfig::ReadSessionMetadataOpenCL(const std::string& session_metadata_file_path, std::shared_ptr<RgCliBuildOutputOpencl>& cli_output)
{
    bool ret = false;

    // Create the CLI output variable.
    cli_output = std::make_shared<RgCliBuildOutputOpencl>();

    // Load the XML document.
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError    rc = doc.LoadFile(session_metadata_file_path.c_str());

    if (rc == tinyxml2::XML_SUCCESS)
    {
        // Get the XML declaration node.
        tinyxml2::XMLNode* node = doc.FirstChild();

        if (node != nullptr)
        {
            // Traverse to the program metadata root element.
            tinyxml2::XMLNode* metadata_node = node->NextSiblingElement(kStrXmlNodeMetadata);
            if (metadata_node != nullptr)
            {
                do
                {
                    // Parse the metadata contents, starting with the output binary and input files.
                    tinyxml2::XMLNode* binary_file_path_node = metadata_node->FirstChildElement(kStrXmlNodeBinary);
                    if (binary_file_path_node != nullptr)
                    {
                        // Read the full file path to the compiled binary output.
                        std::string binary_full_file_path;
                        ret = RgXMLUtils::ReadNodeTextString(binary_file_path_node, binary_full_file_path);

                        if (ret)
                        {
                            // Set the output's binary path to what was read from the node text.
                            cli_output->project_binaries.push_back(binary_full_file_path);

                            // Parse the metadata contents, starting with the output binary and input files.
                            tinyxml2::XMLNode* input_file_node = metadata_node->FirstChildElement(kStrXmlNodeInputFile);
                            if (input_file_node != nullptr)
                            {
                                // Read all of the input file nodes.
                                ret = ReadInputFiles(input_file_node, cli_output, binary_full_file_path);
                                assert(ret);
                            }
                        }
                    }
                    else
                    {
                        // Parse the metadata contents, starting with the output binary and input files.
                        tinyxml2::XMLNode* input_file_node = metadata_node->FirstChildElement(kStrXmlNodeInputFile);
                        if (input_file_node != nullptr)
                        {
                            // Read all of the input file nodes.
                            ret = ReadInputFiles(input_file_node, cli_output);
                            assert(ret);
                        }
                    }
                    // Move to the next output entry.
                    metadata_node = metadata_node->NextSibling();
                } while (metadata_node != nullptr);
            }
        }
    }

    return ret;
}

bool RgXMLSessionConfig::ReadSessionMetadataVulkan(const std::string&                         session_metadata_file_path,
                                                   std::shared_ptr<RgCliBuildOutputPipeline>& cli_output,
                                                   bool                                       is_codeobj_input_file)
{
    bool ret = false;

    // Create the CLI output variable.
    cli_output = std::make_shared<RgCliBuildOutputPipeline>();

    // Load the XML document.
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError    rc = doc.LoadFile(session_metadata_file_path.c_str());

    if (rc == tinyxml2::XML_SUCCESS)
    {
        // Get the root pipeline metadata element. Pipeline outputs appear as children elements.
        tinyxml2::XMLNode* pipeline_metadata_node = doc.FirstChildElement(kStrXmlNodeMetadataPipeline);
        assert(pipeline_metadata_node != nullptr);
        if (pipeline_metadata_node != nullptr)
        {
            // Get the XML declaration node.
            tinyxml2::XMLNode* pipeline_node = pipeline_metadata_node->FirstChildElement(kStrXmlNodePipeline);
            if (pipeline_node != nullptr)
            {
                // Determine the type of pipeline that was built.
                tinyxml2::XMLNode* pipeline_type_node = pipeline_node->FirstChildElement(kStrXmlNodeType);
                if (pipeline_type_node != nullptr)
                {
                    // Read the pipeline type string.
                    std::string pipeline_type_string;
                    ret = RgXMLUtils::ReadNodeTextString(pipeline_type_node, pipeline_type_string);

                    assert(ret);
                    if (ret)
                    {
                        // Set the pipeline type based on the node text.
                        cli_output->type =
                            (pipeline_type_string.compare(kStrXmlNodePipelineTypeGraphics) == 0) ? RgPipelineType::kGraphics : RgPipelineType::kCompute;

                        // Search for each stage element.
                        tinyxml2::XMLNode* stage_node = pipeline_node->FirstChildElement(kStrXmlNodeStage);
                        while (stage_node != nullptr)
                        {
                            RgEntryOutput stage_entry   = {};
                            stage_entry.entrypoint_name = kStrDefaultVulkanGlslEntrypointName;

                            // Search for the stage type node.
                            tinyxml2::XMLNode* stage_type_node = stage_node->FirstChildElement(kStrXmlNodeType);

                            assert(stage_type_node != nullptr);
                            if (stage_type_node != nullptr)
                            {
                                // Read the stage type node.
                                std::string pipeline_stage_type_string;
                                ret = RgXMLUtils::ReadNodeTextString(stage_type_node, pipeline_stage_type_string);

                                assert(ret);
                                if (ret)
                                {
                                    // Assign the stage type in the file's outputs struct.
                                    stage_entry.kernel_type = pipeline_stage_type_string;
                                }
                            }

                            assert(ret);
                            if (ret)
                            {
                                // Search for the input file node.
                                tinyxml2::XMLNode* input_file_node = stage_node->FirstChildElement(kStrXmlNodeInputFile);
                                assert(input_file_node != nullptr);
                                if (input_file_node != nullptr)
                                {
                                    input_file_node = input_file_node->FirstChildElement(kStrXmlNodePath);
                                    assert(input_file_node != nullptr);
                                    if (input_file_node != nullptr)
                                    {
                                        // Read the input file path.
                                        std::string input_file_path_string;
                                        ret = RgXMLUtils::ReadNodeTextString(input_file_node, input_file_path_string);
                                        if (ret)
                                        {
                                            stage_entry.input_file_path = input_file_path_string;
                                        }
                                    }
                                }
                            }

                            assert(ret);
                            if (ret)
                            {
                                // Find the stage node's output node.
                                tinyxml2::XMLNode* stage_output_node = stage_node->FirstChildElement(kStrXmlNodeOutput);
                                assert(stage_output_node != nullptr);
                                if (stage_output_node != nullptr)
                                {
                                    // Read the stage's output files.
                                    ret = ret && ReadPipelineStageOutputs(stage_output_node, stage_entry);
                                }
                            }

                            assert(ret);
                            if (ret)
                            {
                                bool is_stage_entry_added = false;
                                if (is_codeobj_input_file)
                                {
                                    stage_entry.entrypoint_name = stage_entry.kernel_type;
                                    auto output_itr             = cli_output->per_file_output.find(stage_entry.input_file_path);
                                    if (output_itr != cli_output->per_file_output.end())
                                    {
                                        output_itr->second.outputs.push_back(stage_entry);
                                        is_stage_entry_added = true;
                                    }
                                }

                                if (!is_stage_entry_added)
                                {
                                    RgFileOutputs file_outputs;
                                    file_outputs.input_file_path = stage_entry.input_file_path;
                                    file_outputs.outputs.push_back(stage_entry);
                                    cli_output->per_file_output[stage_entry.input_file_path] = file_outputs;
                                }

                                // Search for the next stage node.
                                stage_node = stage_node->NextSiblingElement(kStrXmlNodeStage);
                            }

                            // If anything went wrong, end this loop.
                            if (!ret)
                            {
                                stage_node = nullptr;
                            }
                        }
                    }
                }
            }
        }
    }

    return ret;
}

bool RgXMLSessionConfig::RemoveBinaryFileFromMetadata(const std::string& session_metadata_file_path, const std::string& target_binary_full_file_path)
{
    bool ret = false;

    // Load the XML document.
    tinyxml2::XMLDocument doc(true, tinyxml2::Whitespace::COLLAPSE_WHITESPACE);

    tinyxml2::XMLError rc = doc.LoadFile(session_metadata_file_path.c_str());

    if (rc == tinyxml2::XML_SUCCESS)
    {
        // Get the XML declaration node.
        tinyxml2::XMLNode* first_node = doc.FirstChild();

        if (first_node != nullptr)
        {
            // Traverse to the program metadata root element.
            tinyxml2::XMLNode* metadata_node = first_node->NextSiblingElement(kStrXmlNodeMetadata);
            if (metadata_node != nullptr)
            {
                // Open the file and clear it's current contents. We will rewrite each node we want to keep back to the file.
#ifdef WIN32
                std::FILE* xml_file;
                fopen_s(&xml_file, session_metadata_file_path.c_str(), "w");
#else
                std::FILE* xml_file = std::fopen(session_metadata_file_path.c_str(), "w");
#endif
                if (xml_file != nullptr)
                {
                    // Write the first node to the file.
                    tinyxml2::XMLDocument new_first_doc(true, tinyxml2::Whitespace::COLLAPSE_WHITESPACE);
                    tinyxml2::XMLNode*    new_first_node = first_node->DeepClone(&new_first_doc);
                    new_first_doc.LinkEndChild(new_first_node);

                    tinyxml2::XMLPrinter first_node_printer(xml_file);
                    new_first_doc.Print(&first_node_printer);

                    tinyxml2::XMLNode* current_node = metadata_node;

                    do
                    {
                        // Copy all the metadata nodes except for the node corresponding to the given binary that should be removed.
                        tinyxml2::XMLNode* binary_file_path_node = current_node->FirstChildElement(kStrXmlNodeBinary);
                        if (binary_file_path_node != nullptr)
                        {
                            // Read the full file path to the compiled binary output.
                            std::string binary_full_file_path = "";
                            RgXMLUtils::ReadNodeTextString(binary_file_path_node, binary_full_file_path);

                            if (binary_full_file_path != target_binary_full_file_path)
                            {
                                tinyxml2::XMLDocument new_doc(true, tinyxml2::Whitespace::COLLAPSE_WHITESPACE);
                                tinyxml2::XMLNode*    new_node = current_node->DeepClone(&new_doc);
                                new_doc.LinkEndChild(new_node);

                                tinyxml2::XMLPrinter printer(xml_file);
                                new_doc.Print(&printer);
                            }
                        }

                        // Move to the next output entry.
                        current_node = current_node->NextSibling();
                    } while (current_node != nullptr);

                    std::fclose(xml_file);
                }
            }
        }
    }

    return ret;
}

bool RgXMLSessionConfig::ReadBuildOutputs(tinyxml2::XMLNode* outputs_node, std::vector<RgOutputItem>& outputs)
{
    bool ret = false;

    // Extract the target GPU name.
    tinyxml2::XMLNode* target_gpu_node = outputs_node->FirstChildElement(kStrXmlNodeTarget);

    while (outputs_node != nullptr)
    {
        RgOutputItem output_item = {};

        if (target_gpu_node != nullptr)
        {
            // Read the target GPU that the output was generated for.
            std::string target_asic;
            ret = RgXMLUtils::ReadNodeTextString(target_gpu_node, target_asic);
            assert(ret);

            if (ret)
            {
                // Find the ISA disassembly text file path node.
                tinyxml2::XMLNode* isa_disassembly_text_node = outputs_node->FirstChildElement(kStrXmlNodeIsa);
                if (isa_disassembly_text_node != nullptr)
                {
                    // Read the file path to the disassembled ISA text.
                    std::string isa_disassemly_text_file_path;
                    ret = RgXMLUtils::ReadNodeTextString(isa_disassembly_text_node, isa_disassemly_text_file_path);
                    assert(ret);

                    if (ret)
                    {
                        RgOutputItem csv_output_item = {isa_disassemly_text_file_path, target_asic, RgCliOutputFileType::kIsaDisassemblyText};
                        outputs.push_back(csv_output_item);
                    }
                }

                // Find the ISA disassembly CSV file path node.
                tinyxml2::XMLNode* isa_disassembly_csv_node = outputs_node->FirstChildElement(kStrXmlNodeCsvIsa);
                if (isa_disassembly_csv_node != nullptr)
                {
                    // Read the file path to the disassembled ISA CSV.
                    std::string isa_disassemly_csv_file_path;
                    ret = RgXMLUtils::ReadNodeTextString(isa_disassembly_csv_node, isa_disassemly_csv_file_path);
                    assert(ret);

                    if (ret)
                    {
                        RgOutputItem csv_output_item = {isa_disassemly_csv_file_path, target_asic, RgCliOutputFileType::kIsaDisassemblyCsv};
                        outputs.push_back(csv_output_item);
                    }
                }

                // Find the resource usage CSV file path node.
                tinyxml2::XMLNode* resource_usage_csv_node = outputs_node->FirstChildElement(kStrXmlNodeResUsage);
                if (resource_usage_csv_node != nullptr)
                {
                    // Read the file path to the resource usage CSV file.
                    std::string resource_usage_csv_file_path;
                    ret = RgXMLUtils::ReadNodeTextString(resource_usage_csv_node, resource_usage_csv_file_path);
                    assert(ret);

                    if (ret)
                    {
                        RgOutputItem csv_output_item = {resource_usage_csv_file_path, target_asic, RgCliOutputFileType::kHwResourceUsageFile};
                        outputs.push_back(csv_output_item);
                    }
                }

                // Find the live register file path node.
                tinyxml2::XMLNode* live_register_node = outputs_node->FirstChildElement(kStrXmlNodeLivereg);
                if (live_register_node != nullptr)
                {
                    // Read the file path to the resource usage CSV file.
                    std::string live_register_file_path;
                    ret = RgXMLUtils::ReadNodeTextString(live_register_node, live_register_file_path);
                    assert(ret);

                    if (ret)
                    {
                        RgOutputItem live_reg_output_item = {live_register_file_path, target_asic, RgCliOutputFileType::kLiveRegisterAnalysisReport};
                        outputs.push_back(live_reg_output_item);
                    }
                }
            }
        }

        // Try to find additional output nodes to parse.
        outputs_node = outputs_node->NextSibling();
    }

    return ret;
}

bool RgXMLSessionConfig::ReadEntryOutputs(tinyxml2::XMLNode* output_entry_node, RgFileOutputs& outputs)
{
    bool ret = false;

    // Add each output to the input file's list of outputs.
    do
    {
        RgEntryOutput entry;

        bool has_entry_name = false;

        // Extract the compiled kernel name.
        tinyxml2::XMLNode* kernel_name_node = output_entry_node->FirstChildElement(kStrXmlNodeName);
        if (kernel_name_node != nullptr)
        {
            has_entry_name = RgXMLUtils::ReadNodeTextString(kernel_name_node, entry.entrypoint_name);
        }

        // Extract the kernel type.
        tinyxml2::XMLNode* kernel_type_node = output_entry_node->FirstChildElement(kStrXmlNodeType);
        if (kernel_type_node != nullptr)
        {
            // Read the full file path to the input file.
            ret = RgXMLUtils::ReadNodeTextString(kernel_type_node, entry.kernel_type);
            assert(ret);

            if (ret)
            {
                // If there is no entry name, use the kernel type as the entry name.
                if (!has_entry_name)
                {
                    entry.entrypoint_name = entry.kernel_type;
                }

                tinyxml2::XMLNode* extremely_long_kernel_name_node = output_entry_node->FirstChildElement(kStrXmlNodeExtremelyLongName);
                if (extremely_long_kernel_name_node != nullptr)
                {
                    // Read the full name of the kernel for the input file.
                    ret = RgXMLUtils::ReadNodeTextString(extremely_long_kernel_name_node, entry.extremely_long_kernel_name);
                }

                // Find the the compiled outputs node.
                tinyxml2::XMLNode* outputs_node = output_entry_node->FirstChildElement(kStrXmlNodeOutput);
                if (outputs_node != nullptr)
                {
                    // Read all of the build outputs.
                    ret = ReadBuildOutputs(outputs_node, entry.outputs);
                    assert(ret);

                    if (ret)
                    {
                        // Assign the input file path to each output entry so we know where it originated from.
                        entry.input_file_path = outputs.input_file_path;

                        // Add the populated output structure to the array of outputs for the given file.
                        outputs.outputs.push_back(entry);
                    }
                }
            }
        }

        // Move to the next output entry.
        output_entry_node = output_entry_node->NextSibling();
    } while (output_entry_node != nullptr);

    return ret;
}

bool RgXMLSessionConfig::ReadInputFiles(tinyxml2::XMLNode* input_file_node, std::shared_ptr<RgCliBuildOutputOpencl>& cli_output, std::string binary_file_name)
{
    bool ret = false;

    // Loop over each input file's outputs.
    do
    {
        // Parse the input file's info.
        tinyxml2::XMLNode* input_file_path_node = input_file_node->FirstChildElement(kStrXmlNodePath);
        if (input_file_path_node != nullptr)
        {
            // Read the full file path to the input file.
            std::string input_full_file_path;
            ret = RgXMLUtils::ReadNodeTextString(input_file_path_node, input_full_file_path);

            if (ret)
            {
                // Process the input file's output entry info.
                tinyxml2::XMLNode* output_entry_node = input_file_node->FirstChildElement(kStrXmlNodeEntry);
                if (output_entry_node != nullptr)
                {
                    // Use the binary file name for the input file key for binary analysis mode when there is no source file.
                    if (input_full_file_path == "<Unknown>" && binary_file_name != "")
                    {
                        input_full_file_path = binary_file_name;
                    }

                    RgFileOutputs& file_outputs  = cli_output->per_file_output[input_full_file_path];
                    file_outputs.input_file_path = input_full_file_path;
                    ret                          = ReadEntryOutputs(output_entry_node, file_outputs);
                    assert(ret);
                }
            }
        }

        // Find the next input file sibling node.
        input_file_node = input_file_node->NextSibling();
    } while (input_file_node != nullptr);

    return ret;
}

bool RgXMLSessionConfig::ReadPipelineStageOutputs(tinyxml2::XMLNode* outputs_node, RgEntryOutput& stage_output)
{
    bool ret = false;

    // Search for the listed stage output node strings.
    static const std::string kStageOutputNodes[] = {kStrXmlNodeIsa, kStrXmlNodeCsvIsa, kStrXmlNodeResUsage, kStrXmlNodeLivereg};

    // Loop to search for each possible output type.
    for (auto output_iter = std::begin(kStageOutputNodes); output_iter != std::end(kStageOutputNodes); ++output_iter)
    {
        // Search for given output type.
        const std::string& output_type_string = *output_iter;
        tinyxml2::XMLNode* output_node        = outputs_node->FirstChildElement(output_type_string.c_str());
        if (output_node != nullptr)
        {
            RgOutputItem csv_output_item = {};

            // Read the output file path.
            std::string output_file_path;
            ret = RgXMLUtils::ReadNodeTextString(output_node, output_file_path);
            assert(ret);
            if (ret)
            {
                // Assign the parsed file path into the output.
                csv_output_item.file_path = output_file_path;

                // Assign the file path in the output object.
                if (output_type_string.compare(kStrXmlNodeIsa) == 0)
                {
                    csv_output_item.file_type = RgCliOutputFileType::kIsaDisassemblyText;
                }
                else if (output_type_string.compare(kStrXmlNodeCsvIsa) == 0)
                {
                    csv_output_item.file_type = RgCliOutputFileType::kIsaDisassemblyCsv;
                }
                else if (output_type_string.compare(kStrXmlNodeResUsage) == 0)
                {
                    csv_output_item.file_type = RgCliOutputFileType::kHwResourceUsageFile;
                }
                else if (output_type_string.compare(kStrXmlNodeLivereg) == 0)
                {
                    csv_output_item.file_type = RgCliOutputFileType::kLiveRegisterAnalysisReport;
                }
                else
                {
                    // The node text wasn't recognized.
                    assert(false);
                    ret = false;
                }

                if (ret)
                {
                    stage_output.outputs.push_back(csv_output_item);
                }
            }
        }
    }

    return ret;
}

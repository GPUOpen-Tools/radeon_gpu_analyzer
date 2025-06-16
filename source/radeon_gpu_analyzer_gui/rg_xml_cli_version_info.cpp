//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for parsing the CLI's version-info results.
//=============================================================================
// C++.
#include <algorithm>
#include <cassert>

// XML.
#include "tinyxml2.h"

// Local.
#include "radeon_gpu_analyzer_gui/rg_config_file.h"
#include "radeon_gpu_analyzer_gui/rg_xml_cli_version_info.h"
#include "radeon_gpu_analyzer_gui/rg_utils.h"
#include "radeon_gpu_analyzer_gui/rg_xml_utils.h"
#include "source/common/rga_xml_constants.h"

struct ArchitectureNameSearcher
{
    ArchitectureNameSearcher(const std::string& architecture_name) : architecture_name(architecture_name) {}

    // A predicate that will compare each architecture name with a target name to search for.
    bool operator()(const RgGpuArchitecture& architecture) const
    {
        return architecture.architecture_name.compare(architecture_name) == 0;
    }

    // The target architecture name to search for.
    std::string architecture_name;
};

bool RgXMLCliVersionInfo::ReadVersionInfo(const std::string& version_info_file_path, std::shared_ptr<RgCliVersionInfo>& version_info)
{
    bool ret = false;

    // Reset the output variable.
    version_info = std::make_shared<RgCliVersionInfo>();

    // Load the XML document.
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError rc = doc.LoadFile(version_info_file_path.c_str());

    if (rc == tinyxml2::XML_SUCCESS)
    {
        // Read the CLI version string.
        tinyxml2::XMLNode* version_node = doc.FirstChildElement(kStrXmlNodeVersion);
        if (version_node != nullptr)
        {
            std::string version_string;
            ret = RgXMLUtils::ReadNodeTextString(version_node, version_string);
            if (ret)
            {
                // Store the version string.
                version_info->version = version_string;

                // Read the build date string.
                tinyxml2::XMLNode* build_date_node = doc.FirstChildElement(kStrXmlNodeBuildDate);
                if (build_date_node != nullptr)
                {
                    std::string build_date_string;
                    ret = RgXMLUtils::ReadNodeTextString(build_date_node, build_date_string);
                    if (ret)
                    {
                        // Store the build date string.
                        version_info->build_date = build_date_string;

                        // Get the XML declaration node.
                        tinyxml2::XMLNode* node = doc.FirstChild();
                        if (node != nullptr)
                        {
                            // Attempt to find the "Mode" node, describing the GPUs that are supported for each build mode.
                            tinyxml2::XMLNode* mode_node = node->NextSiblingElement(kStrXmlNodeMode);
                            if (mode_node != nullptr)
                            {
                                // Loop over each Mode node.
                                do
                                {
                                    tinyxml2::XMLNode* mode_name_ptr = mode_node->FirstChildElement(kStrXmlNodeName);
                                    if (mode_name_ptr != nullptr)
                                    {
                                        // Read the mode's name string.
                                        std::string mode_name;
                                        ret = RgXMLUtils::ReadNodeTextString(mode_name_ptr, mode_name);
                                        if (ret)
                                        {
                                            // Traverse to the "SupportedGPUs" element.
                                            node = mode_node->FirstChildElement(kStrXmlNodeSupportedGpus);
                                            if (node != nullptr)
                                            {
                                                // Parse each GPU within the list of supported GPUs.
                                                tinyxml2::XMLNode* gpu_node = node->FirstChildElement(kStrXmlNodeGpu);
                                                if (gpu_node != nullptr)
                                                {
                                                    do
                                                    {
                                                        // Fill a structure with the parsed GPU info.
                                                        RgGpuFamily gpu_family_info = {};

                                                        // The ASIC's architecture will determine which bucket the family info goes into.
                                                        std::string architecture_name;

                                                        node = gpu_node->FirstChildElement(kStrXmlNodeGeneration);
                                                        if (node != nullptr)
                                                        {
                                                            // Read the GPU architecture name string.
                                                            ret = RgXMLUtils::ReadNodeTextString(node, architecture_name);
                                                            if (ret)
                                                            {
                                                                // Find the GPU family element.
                                                                node = node->NextSiblingElement(kStrXmlNodeCodename);
                                                                if (ret && node != nullptr)
                                                                {
                                                                    // Read the GPU family name text.
                                                                    ret = RgXMLUtils::ReadNodeTextString(node, gpu_family_info.family_name);
                                                                    if (ret)
                                                                    {
                                                                        node = node->NextSiblingElement(kStrXmlNodeProductNames);
                                                                        if (ret && node != nullptr)
                                                                        {
                                                                            // Read the GPU product names list.
                                                                            std::string product_names_list;
                                                                            ret = RgXMLUtils::ReadNodeTextString(node, product_names_list);
                                                                            if (ret)
                                                                            {
                                                                                // Split the list of final product names.
                                                                                std::vector<std::string> product_names;
                                                                                RgUtils::splitString(product_names_list, ',', product_names);

                                                                                // Trim whitespace from each product name string.
                                                                                for (size_t product_index = 0; product_index < product_names.size(); ++product_index)
                                                                                {
                                                                                    RgUtils::TrimLeadingAndTrailingWhitespace(product_names[product_index], product_names[product_index]);
                                                                                }

                                                                                // Add the list of product names to the current GPU info.
                                                                                gpu_family_info.product_names = product_names;
                                                                            }
                                                                        }
                                                                    }
                                                                }
                                                            }
                                                        }

                                                        // Verify that the GPU's architecture was read correctly.
                                                        bool is_architecture_name_string_populated = !architecture_name.empty();
                                                        assert(is_architecture_name_string_populated);

                                                        if (is_architecture_name_string_populated)
                                                        {
                                                            // Search through the architectures to find the one that we want to insert the family info into.
                                                            ArchitectureNameSearcher searcher(architecture_name);
                                                            std::vector<RgGpuArchitecture>& gpu_architectures = version_info->gpu_architectures[mode_name];

                                                            auto arch_iter = std::find_if(gpu_architectures.begin(), gpu_architectures.end(), searcher);

                                                            // If the architecture doesn't exist yet in the list, add a new instance.
                                                            if (arch_iter == gpu_architectures.end())
                                                            {
                                                                RgGpuArchitecture new_architecture = {};
                                                                new_architecture.architecture_name = architecture_name;
                                                                new_architecture.gpu_families.push_back(gpu_family_info);
                                                                gpu_architectures.push_back(new_architecture);
                                                            }
                                                            else
                                                            {
                                                                // Add the family info under the architecture.
                                                                arch_iter->gpu_families.push_back(gpu_family_info);
                                                            }
                                                        }

                                                        // Step to the next GPU node.
                                                        gpu_node = gpu_node->NextSiblingElement(kStrXmlNodeGpu);
                                                    } while (gpu_node != nullptr);
                                                }
                                            }
                                        }
                                    }

                                    // Advance to the next Mode element its supported GPUs list.
                                    mode_node = mode_node->NextSiblingElement(kStrXmlNodeMode);
                                } while (mode_node != nullptr);
                            }
                        }
                    }
                }
            }
        }
    }

    return ret;
}

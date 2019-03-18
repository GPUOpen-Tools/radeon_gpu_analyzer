// C++.
#include <algorithm>
#include <cassert>

// XML.
#include <tinyxml2/Include/tinyxml2.h>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/rgConfigFile.h>
#include <RadeonGPUAnalyzerGUI/Include/rgXMLCliVersionInfo.h>
#include <RadeonGPUAnalyzerGUI/Include/rgUtils.h>
#include <RadeonGPUAnalyzerGUI/Include/rgXMLUtils.h>
#include <Utils/Include/rgaXMLConstants.h>

struct ArchitectureNameSearcher
{
    ArchitectureNameSearcher(const std::string& architectureName) : m_architectureName(architectureName) {}

    // A predicate that will compare each architecture name with a target name to search for.
    bool operator()(const rgGpuArchitecture& architecture) const
    {
        return architecture.m_architectureName.compare(m_architectureName) == 0;
    }

    // The target architecture name to search for.
    std::string m_architectureName;
};

bool rgXMLCliVersionInfo::ReadVersionInfo(const std::string& versionInfoFilePath, std::shared_ptr<rgCliVersionInfo>& pVersionInfo)
{
    bool ret = false;

    // Reset the output variable.
    pVersionInfo = std::make_shared<rgCliVersionInfo>();

    // Load the XML document.
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError rc = doc.LoadFile(versionInfoFilePath.c_str());

    if (rc == tinyxml2::XML_SUCCESS)
    {
        // Read the CLI version string.
        tinyxml2::XMLNode* pVersionNode = doc.FirstChildElement(XML_NODE_VERSION);
        if (pVersionNode != nullptr)
        {
            std::string versionString;
            ret = rgXMLUtils::ReadNodeTextString(pVersionNode, versionString);
            if (ret)
            {
                // Store the version string.
                pVersionInfo->m_version = versionString;

                // Read the build date string.
                tinyxml2::XMLNode* pBuildDateNode = doc.FirstChildElement(XML_NODE_BUILD_DATE);
                if (pBuildDateNode != nullptr)
                {
                    std::string buildDateString;
                    ret = rgXMLUtils::ReadNodeTextString(pBuildDateNode, buildDateString);
                    if (ret)
                    {
                        // Store the build date string.
                        pVersionInfo->m_buildDate = buildDateString;

                        // Get the XML declaration node.
                        tinyxml2::XMLNode* pNode = doc.FirstChild();
                        if (pNode != nullptr)
                        {
                            // Attempt to find the "Mode" node, describing the GPUs that are supported for each build mode.
                            tinyxml2::XMLNode* pModeNode = pNode->NextSiblingElement(XML_NODE_MODE);
                            if (pModeNode != nullptr)
                            {
                                // Loop over each Mode node.
                                do
                                {
                                    tinyxml2::XMLNode* pModeName = pModeNode->FirstChildElement(XML_NODE_NAME);
                                    if (pModeName != nullptr)
                                    {
                                        // Read the mode's name string.
                                        std::string modeName;
                                        ret = rgXMLUtils::ReadNodeTextString(pModeName, modeName);
                                        if (ret)
                                        {
                                            // Traverse to the "SupportedGPUs" element.
                                            pNode = pModeNode->FirstChildElement(XML_NODE_SUPPORTED_GPUS);
                                            if (pNode != nullptr)
                                            {
                                                // Parse each GPU within the list of supported GPUs.
                                                tinyxml2::XMLNode* pGPUNode = pNode->FirstChildElement(XML_NODE_GPU);
                                                if (pGPUNode != nullptr)
                                                {
                                                    do
                                                    {
                                                        // Fill a structure with the parsed GPU info.
                                                        rgGpuFamily gpuFamilyInfo = {};

                                                        // The ASIC's architecture will determine which bucket the family info goes into.
                                                        std::string architectureName;

                                                        pNode = pGPUNode->FirstChildElement(XML_NODE_GENERATION);
                                                        if (pNode != nullptr)
                                                        {
                                                            // Read the GPU architecture name string.
                                                            ret = rgXMLUtils::ReadNodeTextString(pNode, architectureName);
                                                            if (ret)
                                                            {
                                                                // Find the GPU family element.
                                                                pNode = pNode->NextSiblingElement(XML_NODE_CODENAME);
                                                                if (ret && pNode != nullptr)
                                                                {
                                                                    // Read the GPU family name text.
                                                                    ret = rgXMLUtils::ReadNodeTextString(pNode, gpuFamilyInfo.m_familyName);
                                                                    if (ret)
                                                                    {
                                                                        pNode = pNode->NextSiblingElement(XML_NODE_PRODUCT_NAMES);
                                                                        if (ret && pNode != nullptr)
                                                                        {
                                                                            // Read the GPU product names list.
                                                                            std::string productNamesList;
                                                                            ret = rgXMLUtils::ReadNodeTextString(pNode, productNamesList);
                                                                            if (ret)
                                                                            {
                                                                                // Split the list of final product names.
                                                                                std::vector<std::string> productNames;
                                                                                rgUtils::splitString(productNamesList, ',', productNames);

                                                                                // Trim whitespace from each product name string.
                                                                                for (size_t productIndex = 0; productIndex < productNames.size(); ++productIndex)
                                                                                {
                                                                                    rgUtils::TrimLeadingAndTrailingWhitespace(productNames[productIndex], productNames[productIndex]);
                                                                                }

                                                                                // Add the list of product names to the current GPU info.
                                                                                gpuFamilyInfo.m_productNames = productNames;
                                                                            }
                                                                        }
                                                                    }
                                                                }
                                                            }
                                                        }

                                                        // Verify that the GPU's architecture was read correctly.
                                                        bool isArchitectureNameStringPopulated = !architectureName.empty();
                                                        assert(isArchitectureNameStringPopulated);

                                                        if (isArchitectureNameStringPopulated)
                                                        {
                                                            // Search through the architectures to find the one that we want to insert the family info into.
                                                            ArchitectureNameSearcher searcher(architectureName);
                                                            std::vector<rgGpuArchitecture>& gpuArchitectures = pVersionInfo->m_gpuArchitectures[modeName];

                                                            auto archIter = std::find_if(gpuArchitectures.begin(), gpuArchitectures.end(), searcher);

                                                            // If the architecture doesn't exist yet in the list, add a new instance.
                                                            if (archIter == gpuArchitectures.end())
                                                            {
                                                                rgGpuArchitecture newArchitecture = {};
                                                                newArchitecture.m_architectureName = architectureName;
                                                                newArchitecture.m_gpuFamilies.push_back(gpuFamilyInfo);
                                                                gpuArchitectures.push_back(newArchitecture);
                                                            }
                                                            else
                                                            {
                                                                // Add the family info under the architecture.
                                                                archIter->m_gpuFamilies.push_back(gpuFamilyInfo);
                                                            }
                                                        }

                                                        // Step to the next GPU node.
                                                        pGPUNode = pGPUNode->NextSiblingElement(XML_NODE_GPU);
                                                    } while (pGPUNode != nullptr);
                                                }
                                            }
                                        }
                                    }

                                    // Advance to the next Mode element its supported GPUs list.
                                    pModeNode = pModeNode->NextSiblingElement(XML_NODE_MODE);
                                } while (pModeNode != nullptr);
                            }
                        }
                    }
                }
            }
        }
    }

    return ret;
}
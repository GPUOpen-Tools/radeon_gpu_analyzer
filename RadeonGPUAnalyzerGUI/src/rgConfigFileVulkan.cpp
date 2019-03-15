// C++.
#include <cassert>
#include <memory>
#include <algorithm>

// Infra.
#include <tinyxml2/Include/tinyxml2.h>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/rgConfigFileDefinitions.h>
#include <RadeonGPUAnalyzerGUI/Include/rgConfigFileVulkan.h>
#include <RadeonGPUAnalyzerGUI/Include/rgXMLUtils.h>
#include <RadeonGPUAnalyzerGUI/Include/rgUtils.h>

bool rgConfigFileReaderVulkan::ReadProjectClone(tinyxml2::XMLDocument& doc, tinyxml2::XMLNode* pClonesRoot, std::shared_ptr<rgProjectVulkan>& pVulkanProject)
{
    // Get the clone ID.
    tinyxml2::XMLNode* pNode = pClonesRoot->FirstChildElement(XML_NODE_CLONE_ID);
    std::shared_ptr<rgProjectCloneVulkan> pClone = std::make_shared<rgProjectCloneVulkan>();
    bool ret = rgXMLUtils::ReadNodeTextUnsigned(pNode, pClone->m_cloneId);
    if (ret && pNode != nullptr)
    {
        // Get the name of the clone.
        pNode = pNode->NextSibling();
        ret = rgXMLUtils::ReadNodeTextString(pNode, pClone->m_cloneName);
    }

    // Read the pipeline object.
    ret = ret && ReadPipeline(doc, pClonesRoot, false , pClone->m_pipeline);
    assert(ret);

    // Read the backup SPIR-V binary paths (optional).
    ret = ret && ReadPipeline(doc, pClonesRoot, true, pClone->m_spvBackup);
    assert(ret);

    if (ret)
    {
        // Create a Vulkan build settings object.
        std::shared_ptr<rgBuildSettingsVulkan> pBuildSettings = std::make_shared<rgBuildSettingsVulkan>();
        assert(pBuildSettings != nullptr);
        if ((ret = pBuildSettings != nullptr) == true)
        {
            // Read the pipeline state.
            pNode = pNode->NextSiblingElement(XML_NODE_PIPELINE_STATE_ROOT);
            assert(pNode != nullptr);
            ret = (pNode != nullptr);
        }

        ret = ret && ReadPipelineState(pClone, doc, pNode);
        assert(ret);

        if (ret)
        {
            // Get the Vulkan build settings.
            pNode = pNode->NextSiblingElement(XML_NODE_BUILD_SETTINGS);
            assert(pNode != nullptr);
            ret = pNode != nullptr;
        }

        // Read the general build settings that aren't specific to a single API.
        ret = ret && ReadGeneralBuildSettings(pNode, pBuildSettings);
        assert(ret);

        // Read the build settings that apply only to Vulkan.
        ret = ret && ReadApiBuildSettings(pNode, pBuildSettings);
        assert(ret);

        if (ret)
        {
            // Add the build settings to the project clone.
            pClone->m_pBuildSettings = pBuildSettings;

            // We are done, add this clone to the project object.
            pVulkanProject->m_clones.push_back(pClone);
        }
    }

    return ret;
}

bool rgConfigFileReaderVulkan::ReadProjectConfigFile(tinyxml2::XMLDocument& doc, const char* pFileDataModelVersion, std::shared_ptr<rgProject>& pRgaProject)
{
    // The config files are the same format for v2.0 and v2.1
    bool isVersionCompatible =
        (RGA_DATA_MODEL_2_0.compare(pFileDataModelVersion) == 0) ||
        (RGA_DATA_MODEL_2_1.compare(pFileDataModelVersion) == 0);

    assert(isVersionCompatible);

    bool ret = false;
    pRgaProject = nullptr;

    if (isVersionCompatible)
    {
        // Find the project node.
        tinyxml2::XMLNode* pNode = doc.FirstChildElement(XML_NODE_PROJECT);
        if (pNode != nullptr)
        {
            pNode = pNode->FirstChild();

            // Verify that this is an Vulkan API config file.
            std::string pApiName;
            ret = rgXMLUtils::ReadNodeTextString(pNode, pApiName);
            if (ret && (pApiName.compare(STR_API_NAME_VULKAN) == 0) && pNode != nullptr)
            {
                // Go to the project name node.
                pNode = pNode->NextSibling();

                // Get the project name.
                std::string projectName;
                ret = rgXMLUtils::ReadNodeTextString(pNode, projectName);
                if (!projectName.empty() && pNode != nullptr)
                {
                    // Create the RGA project object.
                    std::shared_ptr<rgProjectVulkan> pVulkanProject = std::make_shared<rgProjectVulkan>();
                    pVulkanProject->m_projectName = projectName;
                    pRgaProject = pVulkanProject;

                    // Iterate through the project's clones: get the first clone.
                    pNode = pNode->NextSibling();
                    tinyxml2::XMLNode* pClonesRoot = pNode;

                    while (ret && pClonesRoot != nullptr)
                    {
                        // Read the project clone data.
                        ret = ReadProjectClone(doc, pClonesRoot, pVulkanProject);

                        // Go to the next clone element.
                        pClonesRoot = pClonesRoot->NextSibling();
                    }
                }
            }
        }
    }

    return ret;
}

bool rgConfigFileReaderVulkan::ReadApiBuildSettings(tinyxml2::XMLNode* pNode, std::shared_ptr<rgBuildSettings> pBuildSettings)
{
    bool ret = false;

    assert(pBuildSettings != nullptr);
    if (pBuildSettings != nullptr)
    {
        const std::shared_ptr<rgBuildSettingsVulkan> pBuildSettingsVulkan = std::dynamic_pointer_cast<rgBuildSettingsVulkan>(pBuildSettings);
        assert(pBuildSettingsVulkan != nullptr);
        if (pBuildSettingsVulkan != nullptr)
        {
            assert(pNode != nullptr);
            if (pNode != nullptr)
            {
                // Generate Debug Info.
                pNode = pNode->FirstChildElement(XML_NODE_VULKAN_GENERATE_DEBUG_INFO);
                ret = rgXMLUtils::ReadNodeTextBool(pNode, pBuildSettingsVulkan->m_isGenerateDebugInfoChecked);
            }

            if (ret && pNode != nullptr)
            {
                // No Explicit Bindings.
                pNode = pNode->NextSiblingElement(XML_NODE_VULKAN_NO_EXPLICIT_BINDINGS);
                ret = rgXMLUtils::ReadNodeTextBool(pNode, pBuildSettingsVulkan->m_isNoExplicitBindingsChecked);
            }

            if (ret && pNode != nullptr)
            {
                // Use HLSL Block Offsets.
                pNode = pNode->NextSiblingElement(XML_NODE_VULKAN_USE_HLSL_BLOCK_OFFSETS);
                ret = rgXMLUtils::ReadNodeTextBool(pNode, pBuildSettingsVulkan->m_isUseHlslBlockOffsetsChecked);
            }

            if (ret && pNode != nullptr)
            {
                // Use HLSL IO Mapping.
                pNode = pNode->NextSiblingElement(XML_NODE_VULKAN_USE_HLSL_IO_MAPPING);
                ret = rgXMLUtils::ReadNodeTextBool(pNode, pBuildSettingsVulkan->m_isUseHlslIoMappingChecked);
            }

            if (ret && pNode != nullptr)
            {
                // Get ICD location.
                pNode = pNode->NextSiblingElement(XML_NODE_VULKAN_ICD_LOCATION);
                ret = (pNode != nullptr);
                assert(pNode != nullptr);

                std::string icdLocation;
                bool shouldRead = rgXMLUtils::ReadNodeTextString(pNode, icdLocation);

                if (shouldRead)
                {
                    pBuildSettingsVulkan->m_ICDLocation = icdLocation;
                }
            }

            if (ret && pNode != nullptr)
            {
                // Use Enable validation layer.
                pNode = pNode->NextSiblingElement(XML_NODE_VULKAN_ENABLE_VALIDATION_LAYER);
                ret = rgXMLUtils::ReadNodeTextBool(pNode, pBuildSettingsVulkan->m_isEnableValidationLayersChecked);
            }

            if (ret && pNode != nullptr)
            {
                // Remember where we were in case we fail reading.
                auto pNodeOriginal = pNode;

                // Get glslang options.
                pNode = pNode->NextSiblingElement(XML_NODE_VULKAN_GLSLANG_OPTIONS_LOCATION);
                ret = (pNode != nullptr);
                if (pNode != nullptr)
                {
                    std::string glslangOptions;
                    bool shouldRead = rgXMLUtils::ReadNodeTextString(pNode, glslangOptions);

                    if (shouldRead)
                    {
                        pBuildSettingsVulkan->m_glslangOptions = glslangOptions;
                    }
                }
                else
                {
                    // If the rga project file does not have the additional glslang options
                    // node, that's OK - just assume that they are empty.
                    ret = true;

                    // Roll back to the last node.
                    pNode = pNodeOriginal;
                }
            }
        }
    }

    return ret;
}

bool rgConfigFileWriterVulkan::WriteProjectConfigFile(const rgProject& project, const std::string& configFilePath)
{
    bool ret = false;

    // Create the XML declaration node.
    tinyxml2::XMLDocument doc;
    AddConfigFileDeclaration(doc);

    // Create the Project element.
    tinyxml2::XMLElement* pProject = doc.NewElement(XML_NODE_PROJECT);
    tinyxml2::XMLElement* pApi = doc.NewElement(XML_NODE_API_NAME);
    std::string apiName;
    ret = rgUtils::ProjectAPIToString(project.m_api, apiName);
    if (ret)
    {
        // API name.
        pApi->SetText(apiName.c_str());
        pProject->InsertFirstChild(pApi);

        // Project name.
        tinyxml2::XMLElement* pProjectName = doc.NewElement(XML_NODE_PROJECT_NAME);
        pProjectName->SetText(project.m_projectName.c_str());
        pProject->InsertEndChild(pProjectName);

        // Handle the project's clones.
        std::vector<tinyxml2::XMLElement*> cloneElems;
        const rgProjectVulkan& vulkanProject = static_cast<const rgProjectVulkan&>(project);

        WriteCloneElements(vulkanProject, doc, cloneElems);
        for (tinyxml2::XMLElement* pCloneElem : cloneElems)
        {
            pProject->LinkEndChild(pCloneElem);
        }

        // Add the project node.
        doc.InsertEndChild(pProject);

        // Save the file.
        tinyxml2::XMLError rc = doc.SaveFile(configFilePath.c_str());
        ret = (rc == tinyxml2::XML_SUCCESS);
        assert(ret);
    }

    return ret;
}

bool rgConfigFileWriterVulkan::WriteBuildSettingsElement(const std::shared_ptr<rgBuildSettings> pBuildSettings, tinyxml2::XMLDocument& doc, tinyxml2::XMLElement*& pBuildSettingsElem)
{
    bool ret = false;

    assert(pBuildSettings != nullptr);
    if (pBuildSettings != nullptr)
    {
        const std::shared_ptr<rgBuildSettingsVulkan> pBuildSettingsVulkan = std::dynamic_pointer_cast<rgBuildSettingsVulkan>(pBuildSettings);
        assert(pBuildSettingsVulkan != nullptr);
        if (pBuildSettingsVulkan != nullptr)
        {
            // Write API-agnostic build settings.
            ret = WriteGeneralBuildSettings(pBuildSettings, doc, pBuildSettingsElem);

            // Write Vulkan-specific settings.

            // Generate debug information.
            rgXMLUtils::AppendXMLElement(doc, pBuildSettingsElem, XML_NODE_VULKAN_GENERATE_DEBUG_INFO, pBuildSettingsVulkan->m_isGenerateDebugInfoChecked);

            // No Explicit Bindings.
            rgXMLUtils::AppendXMLElement(doc, pBuildSettingsElem, XML_NODE_VULKAN_NO_EXPLICIT_BINDINGS, pBuildSettingsVulkan->m_isNoExplicitBindingsChecked);

            // Use HLSL Block Offsets.
            rgXMLUtils::AppendXMLElement(doc, pBuildSettingsElem, XML_NODE_VULKAN_USE_HLSL_BLOCK_OFFSETS, pBuildSettingsVulkan->m_isUseHlslBlockOffsetsChecked);

            // Use HLSL IO Mapping.
            rgXMLUtils::AppendXMLElement(doc, pBuildSettingsElem, XML_NODE_VULKAN_USE_HLSL_IO_MAPPING, pBuildSettingsVulkan->m_isUseHlslIoMappingChecked);

            // ICD location.
            rgXMLUtils::AppendXMLElement(doc, pBuildSettingsElem, XML_NODE_VULKAN_ICD_LOCATION, pBuildSettingsVulkan->m_ICDLocation.c_str());

            // Enable validation layers.
            rgXMLUtils::AppendXMLElement(doc, pBuildSettingsElem, XML_NODE_VULKAN_ENABLE_VALIDATION_LAYER, pBuildSettingsVulkan->m_isEnableValidationLayersChecked);

            // Glslang additional options.
            rgXMLUtils::AppendXMLElement(doc, pBuildSettingsElem, XML_NODE_VULKAN_GLSLANG_OPTIONS_LOCATION, pBuildSettingsVulkan->m_glslangOptions.c_str());
        }
    }

    return ret;
}

bool rgConfigFileWriterVulkan::WriteCloneElements(const rgProjectVulkan& project, tinyxml2::XMLDocument& doc, std::vector<tinyxml2::XMLElement*>& elems)
{
    bool ret = false;

    for (const std::shared_ptr<rgProjectClone>& pClone : project.m_clones)
    {
        std::shared_ptr<rgProjectCloneVulkan> pVulkanClone = std::dynamic_pointer_cast<rgProjectCloneVulkan>(pClone);
        if (pVulkanClone != nullptr)
        {
            // Project clone.
            tinyxml2::XMLElement* pCloneElement = doc.NewElement(XML_NODE_CLONE);

            // Clone ID.
            tinyxml2::XMLElement* pCloneId = doc.NewElement(XML_NODE_CLONE_ID);
            pCloneId->SetText(pVulkanClone->m_cloneId);
            pCloneElement->LinkEndChild(pCloneId);

            // Clone name.
            tinyxml2::XMLElement* pCloneName = doc.NewElement(XML_NODE_CLONE_NAME);
            pCloneName->SetText(pVulkanClone->m_cloneName.c_str());
            pCloneElement->LinkEndChild(pCloneName);

            // Write the pipeline input files.
            ret = WritePipeline(doc, pCloneElement, false, pVulkanClone->m_pipeline);

            // Write the backup spv files if there are any.
            const ShaderInputFileArray& spvBackupFiles = pVulkanClone->m_spvBackup.m_shaderStages;
            if (std::find_if(spvBackupFiles.cbegin(), spvBackupFiles.cend(), [&](const std::string& s) {return !s.empty(); }) != spvBackupFiles.cend())
            {
                ret = WritePipeline(doc, pCloneElement, true, pVulkanClone->m_spvBackup);
            }

            // Build settings.
            tinyxml2::XMLElement* pBuildSettings = doc.NewElement(XML_NODE_BUILD_SETTINGS);
            ret = ret && WriteBuildSettingsElement(pClone->m_pBuildSettings, doc, pBuildSettings);

            // Write the pipeline state.
            ret = ret && WritePipelineState(pVulkanClone, doc, pCloneElement);

            if (ret)
            {
                pCloneElement->LinkEndChild(pBuildSettings);
                elems.push_back(pCloneElement);
                ret = true;
            }
        }
    }

    return ret;
}
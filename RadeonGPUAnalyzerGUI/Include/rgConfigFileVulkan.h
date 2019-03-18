#pragma once

// Local.
#include <RadeonGPUAnalyzerGUI/Include/rgDataTypesVulkan.h>
#include <RadeonGPUAnalyzerGUI/Include/rgConfigFile.h>

// XML reader for Vulkan config files.
class rgConfigFileReaderVulkan : public rgXmlGraphicsConfigFileReaderImpl
{
public:
    // DTOR.
    virtual ~rgConfigFileReaderVulkan() = default;

    // Config file reader used for Vulkan project files.
    virtual bool ReadProjectConfigFile(tinyxml2::XMLDocument& doc, const char* pFileDataModelVersion, std::shared_ptr<rgProject>& pRgaProject) override;

    // Responsible for reading all Vulkan-specific build settings.
    virtual bool ReadApiBuildSettings(tinyxml2::XMLNode* pNode, std::shared_ptr<rgBuildSettings> pBuildSettings) override;

private:
    // Read single project clone info. The data is stored to the project structure pointer by "pVulkanProject".
    bool ReadProjectClone(tinyxml2::XMLDocument& doc, tinyxml2::XMLNode* pClonesRoot, std::shared_ptr<rgProjectVulkan>& pVulkanProject);
};

// XML writer for Vulkan config files.
class rgConfigFileWriterVulkan : public rgXmlGraphicsConfigFileWriterImpl
{
public:
    // DTOR.
    virtual ~rgConfigFileWriterVulkan() = default;

    // Writes an Vulkan project into the config file to the given location.
    virtual bool WriteProjectConfigFile(const rgProject& project, const std::string& configFilePath) override;

    // Write the given build settings structure to the given project config file document.
    virtual bool WriteBuildSettingsElement(const std::shared_ptr<rgBuildSettings> pBuildSettings, tinyxml2::XMLDocument& doc, tinyxml2::XMLElement*& pBuildSettingsElem) override;

private:
    // Write the given project's clones to the config file document.
    bool WriteCloneElements(const rgProjectVulkan& project, tinyxml2::XMLDocument& doc, std::vector<tinyxml2::XMLElement*>& elems);
};


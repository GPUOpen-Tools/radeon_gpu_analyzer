#pragma once

// C++.
#include <vector>
#include <string>
#include <memory>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/rgDataTypes.h>

// Forward declarations.
namespace tinyxml2
{
    class XMLDocument;
    class XMLElement;
    class XMLNode;
}

// The base class for an API-specific config file reader.
class rgXmlConfigFileReaderImpl
{
public:
    // DTOR.
    virtual ~rgXmlConfigFileReaderImpl() = default;

    // Reads the configuration file from the given XML document, and instantiates a new RGA project.
    virtual bool ReadProjectConfigFile(tinyxml2::XMLDocument& doc, const char* pFileDataModelVersion, std::shared_ptr<rgProject>& pProject) = 0;

    // Responsible for reading all API-specific build settings.
    virtual bool ReadApiBuildSettings(tinyxml2::XMLNode* pNode, std::shared_ptr<rgBuildSettings> pBuildSettings, const std::string& version) = 0;

    // Read general project build settings not related to a specific API.
    bool ReadGeneralBuildSettings(tinyxml2::XMLNode* pNode, std::shared_ptr<rgBuildSettings> pBuildSettings);
};

// The base class for an API-specific config file writer.
class rgXmlConfigFileWriterImpl
{
public:
    // DTOR.
    virtual ~rgXmlConfigFileWriterImpl() = default;

    // Writes the given project into a config file at the given location.
    virtual bool WriteProjectConfigFile(const rgProject& project, const std::string& configFilePath) = 0;

    // Write the given build settings structure to the given project config file document.
    virtual bool WriteBuildSettingsElement(const std::shared_ptr<rgBuildSettings> pBuildSettings, tinyxml2::XMLDocument& doc, tinyxml2::XMLElement*& pBuildSettingsElem) = 0;

    // Adds the opening RGA config file data to the given document: XML header, RGA data model version.
    static void AddConfigFileDeclaration(tinyxml2::XMLDocument& doc);

protected:
    // Write API-agnostic build settings.
    bool WriteGeneralBuildSettings(const std::shared_ptr<rgBuildSettings> pBuildSettings, tinyxml2::XMLDocument& doc, tinyxml2::XMLElement* pBuildSettingsElement);
};

// The base class for a graphics API config file reader.
class rgXmlGraphicsConfigFileReaderImpl : public rgXmlConfigFileReaderImpl
{
public:
    // Read a graphics/compute pipeline or corresponding SPIR-V backup files (optional data), depending on "isBackupSpv".
    // The root tag to look for is specified by the "pRootTag".
    bool ReadPipeline(tinyxml2::XMLDocument& doc, tinyxml2::XMLNode* pParentClone, bool isBackupSpv, rgPipelineShaders& pipeline) const;

    // Read the project's pipeline state elements.
    bool ReadPipelineState(std::shared_ptr<rgGraphicsProjectClone> pClone, tinyxml2::XMLDocument& doc, tinyxml2::XMLNode* pPipelineStateElement) const;
};

// The base class for a graphics API config file writer.
class rgXmlGraphicsConfigFileWriterImpl : public rgXmlConfigFileWriterImpl
{
public:
    // Write a graphics or compute pipeline's shader stage input files or corresponding SPIR-V backup files.
    // The root tag is specified by the "pRootTag".
    bool WritePipeline(tinyxml2::XMLDocument& doc, tinyxml2::XMLElement* pParentClone, bool isBackupSpv, const rgPipelineShaders& pipeline) const;

    // Write the project's pipeline state elements.
    bool WritePipelineState(const std::shared_ptr<rgGraphicsProjectClone> pClone, tinyxml2::XMLDocument& doc, tinyxml2::XMLElement* pParentCloneElement) const;
};

// A class containing utility functions used to read and write project configuration files.
class rgXmlConfigFile
{
public:
    // Read the project configuration file from the given file path, and return a pointer to a
    // Project object that contains the data from the configuration file.
    static bool ReadProjectConfigFile(const std::string& configFilePath, std::shared_ptr<rgProject>& pProject);

    // Write the Project data into a configuration file.
    static bool WriteProjectConfigFile(const rgProject& project, const std::string& configFilePath);

    // Read the global settings from the global configuration files which is located in the given path.
    static bool ReadGlobalSettings(const std::string& globalConfigFilePath, std::shared_ptr<rgGlobalSettings>& pGlobalSettings);

    // Save the global settings to an RGA global configuration file in the given full path.
    static bool WriteGlobalSettings(std::shared_ptr<rgGlobalSettings> pGlobalSettings, const std::string& globalConfigFilePath);
};
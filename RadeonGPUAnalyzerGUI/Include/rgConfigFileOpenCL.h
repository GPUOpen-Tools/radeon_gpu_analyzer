#pragma once

// Local.
#include <RadeonGPUAnalyzerGUI/Include/rgDataTypesOpenCL.h>
#include <RadeonGPUAnalyzerGUI/Include/rgConfigFile.h>

// Reader for OpenCL config files.
class rgConfigFileReaderOpenCL : public rgXmlConfigFileReaderImpl
{
public:
    // DTOR.
    virtual ~rgConfigFileReaderOpenCL() = default;

    // Config file reader used for OpenCL project files.
    virtual bool ReadProjectConfigFile(tinyxml2::XMLDocument& doc, const char* pFileDataModelVersion, std::shared_ptr<rgProject>& pRgaProject) override;

    // Parse OpenCL-specific project build settings.
    virtual bool ReadApiBuildSettings(tinyxml2::XMLNode* pNode, std::shared_ptr<rgBuildSettings> pBuildSettings, const std::string& version) override;
};

class rgConfigFileWriterOpenCL : public rgXmlConfigFileWriterImpl
{
public:
    // DTOR.
    virtual ~rgConfigFileWriterOpenCL() = default;

    // Writes an OpenCL project into the config file to the given location.
    virtual bool WriteProjectConfigFile(const rgProject& project, const std::string& configFilePath) override;

    // Write the given OpenCL build settings structure to the given project config file document.
    virtual bool WriteBuildSettingsElement(const std::shared_ptr<rgBuildSettings> pBuildSettings, tinyxml2::XMLDocument& doc, tinyxml2::XMLElement*& pBuildSettingsElem) override;

private:
    // Write the given OpenCL project's clones to the config file document.
    bool WriteOpenCLCloneElements(const rgProjectOpenCL& project, tinyxml2::XMLDocument& doc, std::vector<tinyxml2::XMLElement*>& elems);
};
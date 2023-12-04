#pragma once

// Local.
#include "radeon_gpu_analyzer_gui/rg_data_types_binary.h"
#include "radeon_gpu_analyzer_gui/rg_config_file.h"

// Reader for Binary config files.
class RgConfigFileReaderBinary : public RgXmlConfigFileReaderImpl
{
public:
    // DTOR.
    virtual ~RgConfigFileReaderBinary() = default;

    // Config file reader used for OpenCL project files.
    virtual bool ReadProjectConfigFile(tinyxml2::XMLDocument& doc, const char* file_data_model_version, std::shared_ptr<RgProject>& rga_project) override;

    // Parse OpenCL-specific project build settings.
    virtual bool ReadApiBuildSettings(tinyxml2::XMLNode* node, std::shared_ptr<RgBuildSettings> build_settings, const std::string& version) override;
};

class RgConfigFileWriterBinary : public RgXmlConfigFileWriterImpl
{
public:
    // DTOR.
    virtual ~RgConfigFileWriterBinary() = default;

    // Writes an OpenCL project into the config file to the given location.
    virtual bool WriteProjectConfigFile(const RgProject& project, const std::string& config_file_path) override;

    // Write the given OpenCL build settings structure to the given project config file document.
    virtual bool WriteBuildSettingsElement(const std::shared_ptr<RgBuildSettings> build_settings, tinyxml2::XMLDocument& doc, tinyxml2::XMLElement*& build_settings_elem) override;

private:
    // Write the given Binary project's clones to the config file document.
    bool WriteBinaryCloneElements(const RgProjectBinary& project, tinyxml2::XMLDocument& doc, std::vector<tinyxml2::XMLElement*>& elems);
};

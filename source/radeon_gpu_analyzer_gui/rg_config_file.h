#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_CONFIG_FILE_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_CONFIG_FILE_H_

// C++.
#include <vector>
#include <string>
#include <memory>

// Local.
#include "radeon_gpu_analyzer_gui/rg_data_types.h"

// Forward declarations.
namespace tinyxml2
{
    class XMLDocument;
    class XMLElement;
    class XMLNode;
}

// The base class for an API-specific config file reader.
class RgXmlConfigFileReaderImpl
{
public:
    // DTOR.
    virtual ~RgXmlConfigFileReaderImpl() = default;

    // Reads the configuration file from the given XML document, and instantiates a new RGA project.
    virtual bool ReadProjectConfigFile(tinyxml2::XMLDocument& doc, const char* file_data_model_version, std::shared_ptr<RgProject>& project) = 0;

    // Responsible for reading all API-specific build settings.
    virtual bool ReadApiBuildSettings(tinyxml2::XMLNode* node, std::shared_ptr<RgBuildSettings> build_settings, const std::string& version) = 0;

    // Read general project build settings not related to a specific API.
    bool ReadGeneralBuildSettings(tinyxml2::XMLNode* node, std::shared_ptr<RgBuildSettings> build_settings);
};

// The base class for an API-specific config file writer.
class RgXmlConfigFileWriterImpl
{
public:
    // DTOR.
    virtual ~RgXmlConfigFileWriterImpl() = default;

    // Writes the given project into a config file at the given location.
    virtual bool WriteProjectConfigFile(const RgProject& project, const std::string& config_file_path) = 0;

    // Write the given build settings structure to the given project config file document.
    virtual bool WriteBuildSettingsElement(const std::shared_ptr<RgBuildSettings> build_settings, tinyxml2::XMLDocument& doc, tinyxml2::XMLElement*& build_settings_elem) = 0;

    // Adds the opening RGA config file data to the given document: XML header, RGA data model version.
    static void AddConfigFileDeclaration(tinyxml2::XMLDocument& doc);

protected:
    // Write API-agnostic build settings.
    bool WriteGeneralBuildSettings(const std::shared_ptr<RgBuildSettings> build_settings, tinyxml2::XMLDocument& doc, tinyxml2::XMLElement* build_settings_build_settings_element);
};

// The base class for a graphics API config file reader.
class RgXmlGraphicsConfigFileReaderImpl : public RgXmlConfigFileReaderImpl
{
public:
    // Read a graphics/compute pipeline or corresponding SPIR-V backup files (optional data), depending on "is_backup_spv".
    // The root tag to look for is specified by the "pRootTag".
    bool ReadPipeline(tinyxml2::XMLDocument& doc, tinyxml2::XMLNode* parent_clone, bool is_backup_spv, RgPipelineShaders& pipeline) const;

    // Read the project's pipeline state elements.
    bool ReadPipelineState(std::shared_ptr<RgGraphicsProjectClone> clone, tinyxml2::XMLDocument& doc, tinyxml2::XMLNode* pipeline_state_element) const;
};

// The base class for a graphics API config file writer.
class RgXmlGraphicsConfigFileWriterImpl : public RgXmlConfigFileWriterImpl
{
public:
    // Write a graphics or compute pipeline's shader stage input files or corresponding SPIR-V backup files.
    // The root tag is specified by the "pRootTag".
    bool WritePipeline(tinyxml2::XMLDocument& doc, tinyxml2::XMLElement* parent_clone, bool is_backup_spv, const RgPipelineShaders& pipeline) const;

    // Write the project's pipeline state elements.
    bool WritePipelineState(const std::shared_ptr<RgGraphicsProjectClone> clone, tinyxml2::XMLDocument& doc, tinyxml2::XMLElement* parent_clone_element) const;
};

// A class containing utility functions used to read and write project configuration files.
class RgXmlConfigFile
{
public:
    // Read the project configuration file from the given file path, and return a pointer to a
    // Project object that contains the data from the configuration file.
    static bool ReadProjectConfigFile(const std::string& config_file_path, std::shared_ptr<RgProject>& project);

    // Write the Project data into a configuration file.
    static bool WriteProjectConfigFile(const RgProject& project, const std::string& config_file_path);

    // Read the global settings from the global configuration files which is located in the given path.
    static bool ReadGlobalSettings(const std::string& global_config_file_path, std::shared_ptr<RgGlobalSettings>& global_settings);

    // Save the global settings to an RGA global configuration file in the given full path.
    static bool WriteGlobalSettings(std::shared_ptr<RgGlobalSettings> global_settings, const std::string& global_config_file_path);
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_CONFIG_FILE_H_

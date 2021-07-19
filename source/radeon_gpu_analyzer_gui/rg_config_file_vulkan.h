#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_CONFIG_FILE_VULKAN_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_CONFIG_FILE_VULKAN_H_

// Local.
#include "radeon_gpu_analyzer_gui/rg_data_types_vulkan.h"
#include "radeon_gpu_analyzer_gui/rg_config_file.h"

// XML reader for Vulkan config files.
class RgConfigFileReaderVulkan : public RgXmlGraphicsConfigFileReaderImpl
{
public:
    // DTOR.
    virtual ~RgConfigFileReaderVulkan() = default;

    // Config file reader used for Vulkan project files.
    virtual bool ReadProjectConfigFile(tinyxml2::XMLDocument& doc, const char* file_data_model_version, std::shared_ptr<RgProject>& rga_project) override;

    // Responsible for reading all Vulkan-specific build settings.
    virtual bool ReadApiBuildSettings(tinyxml2::XMLNode* node, std::shared_ptr<RgBuildSettings> build_settings, const std::string& version) override;

private:
    // Read single project clone info. The data is stored to the project structure pointer by "pVulkanProject".
    bool ReadProjectClone(tinyxml2::XMLDocument& doc, tinyxml2::XMLNode* clones_root, std::shared_ptr<RgProjectVulkan>& vulkan_project);
};

// XML writer for Vulkan config files.
class RgConfigFileWriterVulkan : public RgXmlGraphicsConfigFileWriterImpl
{
public:
    // DTOR.
    virtual ~RgConfigFileWriterVulkan() = default;

    // Writes an Vulkan project into the config file to the given location.
    virtual bool WriteProjectConfigFile(const RgProject& project, const std::string& config_file_path) override;

    // Write the given build settings structure to the given project config file document.
    virtual bool WriteBuildSettingsElement(const std::shared_ptr<RgBuildSettings> build_settings, tinyxml2::XMLDocument& doc, tinyxml2::XMLElement*& build_settings_elem) override;

private:
    // Write the given project's clones to the config file document.
    bool WriteCloneElements(const RgProjectVulkan& project, tinyxml2::XMLDocument& doc, std::vector<tinyxml2::XMLElement*>& elems);
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_CONFIG_FILE_VULKAN_H_


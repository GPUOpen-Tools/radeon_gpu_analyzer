#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_CONFIG_FILE_OPENCL_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_CONFIG_FILE_OPENCL_H_

// Local.
#include "radeon_gpu_analyzer_gui/rg_data_types_opencl.h"
#include "radeon_gpu_analyzer_gui/rg_config_file.h"

// Reader for OpenCL config files.
class RgConfigFileReaderOpencl : public RgXmlConfigFileReaderImpl
{
public:
    // DTOR.
    virtual ~RgConfigFileReaderOpencl() = default;

    // Config file reader used for OpenCL project files.
    virtual bool ReadProjectConfigFile(tinyxml2::XMLDocument& doc, const char* file_data_model_version, std::shared_ptr<RgProject>& rga_project) override;

    // Parse OpenCL-specific project build settings.
    virtual bool ReadApiBuildSettings(tinyxml2::XMLNode* node, std::shared_ptr<RgBuildSettings> build_settings, const std::string& version) override;
};

class RgConfigFileWriterOpencl : public RgXmlConfigFileWriterImpl
{
public:
    // DTOR.
    virtual ~RgConfigFileWriterOpencl() = default;

    // Writes an OpenCL project into the config file to the given location.
    virtual bool WriteProjectConfigFile(const RgProject& project, const std::string& config_file_path) override;

    // Write the given OpenCL build settings structure to the given project config file document.
    virtual bool WriteBuildSettingsElement(const std::shared_ptr<RgBuildSettings> build_settings, tinyxml2::XMLDocument& doc, tinyxml2::XMLElement*& build_settings_elem) override;

private:
    // Write the given OpenCL project's clones to the config file document.
    bool WriteOpenCLCloneElements(const RgProjectOpencl& project, tinyxml2::XMLDocument& doc, std::vector<tinyxml2::XMLElement*>& elems);
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_CONFIG_FILE_OPENCL_H_

// C++.
#include <cassert>
#include <sstream>
#include <functional>

// XML.
#include <tinyxml2/Include/tinyxml2.h>

// Local.
#include "radeon_gpu_analyzer_gui/rg_config_file_opencl.h"
#include "radeon_gpu_analyzer_gui/rg_config_file_vulkan.h"
#include "radeon_gpu_analyzer_gui/rg_data_types_opencl.h"
#include "radeon_gpu_analyzer_gui/rg_data_types_vulkan.h"
#include "radeon_gpu_analyzer_gui/rg_factory.h"
#include "radeon_gpu_analyzer_gui/rg_string_constants.h"
#include "radeon_gpu_analyzer_gui/rg_config_file.h"
#include "radeon_gpu_analyzer_gui/rg_utils.h"
#include "radeon_gpu_analyzer_gui/rg_config_file_definitions.h"
#include "radeon_gpu_analyzer_gui/rg_xml_utils.h"
#include "radeon_gpu_analyzer_gui/rg_data_types.h"
#include "radeon_gpu_analyzer_gui/qt/rg_isa_disassembly_table_model.h"

// Infra.
#include "source/common/rg_log.h"

// *** INTERNALLY-LINKED AUXILIARY FUNCTIONS - BEGIN ***

// Read v2.0 of the DefaultBuildSettings XML node from the config file.
static bool ExtractDefaultBuildSettings_2_0(tinyxml2::XMLNode* default_build_settings_node, std::shared_ptr<RgGlobalSettings>& global_settings);

// Read v2.1 of the DefaultBuildSettings XML node from the config file.
static bool ExtractDefaultBuildSettings_2_1(tinyxml2::XMLNode* default_build_settings_node, std::shared_ptr<RgGlobalSettings>& global_settings);

// Read v2.0 of the GlobalSettings XML node from the config file.
static bool ExtractGlobalSettings_2_0(tinyxml2::XMLNode* global_settings_node, std::shared_ptr<RgGlobalSettings>& global_settings);

// Read v2.1 of the GlobalSettings XML node from the config file.
static bool ExtractGlobalSettings_2_1(tinyxml2::XMLNode* global_settings_node, std::shared_ptr<RgGlobalSettings>& global_settings);

// Read v2.2 of the GlobalSettings XML node from the config file.
static bool ExtractGlobalSettings_2_2(tinyxml2::XMLNode* global_settings_node, std::shared_ptr<RgGlobalSettings>& global_settings);

// Read v2.3 of the GlobalSettings XML node from the config file.
static bool ExtractGlobalSettings_2_3(tinyxml2::XMLNode* global_settings_node, std::shared_ptr<RgGlobalSettings>& global_settings);

// Takes the comma-separated target devices list from the GUI, and returns the list of GPUs in it.
static void ExtractTargetGpus(const std::string& target_devices, std::vector<std::string>& gpu_list)
{
    RgUtils::splitString(target_devices, RgConfigManager::kRgaListDelimiter, gpu_list);
}

// Takes the comma-separated predefined macros list from the GUI, and returns the list of macros in it.
static void ExtractPredefinedMacros(const std::string& predefined_macros, std::vector<std::string>& macro_list)
{
    RgUtils::splitString(predefined_macros, RgConfigManager::kRgaListDelimiter, macro_list);
}

// Takes the comma-separated directory list from the GUI, and returns the list of directories in it.
static void ExtractAdditionalIncludeDirectories(const std::string& additional_include_directories, std::vector<std::string>& dir_list)
{
    RgUtils::splitString(additional_include_directories, RgConfigManager::kRgaListDelimiter, dir_list);
}

// Takes the comma-separated list of disassembly columns, and returns the list of the columns in it.
static void ExtractDisassemblyColumns(const std::string& disassembly_columns, std::vector<bool>& column_indices)
{
    std::vector<std::string> split_tokens;
    RgUtils::splitString(disassembly_columns, RgConfigManager::kRgaListDelimiter, split_tokens);

    for (const std::string& token : split_tokens)
    {
        int index = std::stoi(token);
        column_indices.push_back(index == 1);
    }
}

static bool ExtractRecentProjects(tinyxml2::XMLNode* node, std::vector<std::shared_ptr<RgRecentProject>>& recent_projects)
{
    bool ret = false;

    assert(node != nullptr);
    if (node != nullptr)
    {
        // Find the first project in the list of recent projects.
        node = node->FirstChildElement(kXmlNodeGlobalRecentProjectRoot);

        // If there aren't any recent project paths, return early, as there's nothing to parse.
        if (node == nullptr)
        {
            ret = true;
        }
        else
        {
            // Step over each path to a recent project, and add it to the output list.
            while (node != nullptr)
            {
                tinyxml2::XMLNode* child_node = node->FirstChildElement(kXmlNodeGlobalRecentProjectPath);
                assert(child_node != nullptr);
                if (child_node != nullptr)
                {
                    // Read the project path element.
                    std::string project_path;
                    ret = RgXMLUtils::ReadNodeTextString(child_node, project_path);

                    assert(ret);
                    if (ret)
                    {
                        auto recent_project          = std::make_shared<RgRecentProject>();
                        recent_project->project_path = project_path;

                        // Get the project api element.
                        child_node = child_node->NextSiblingElement(kXmlNodeGlobalRecentProjectApi);

                        assert(child_node != nullptr);
                        if (child_node != nullptr)
                        {
                            // Read the project api element.
                            std::string project_api;
                            ret = RgXMLUtils::ReadNodeTextString(child_node, project_api);

                            assert(ret);
                            if (ret)
                            {
                                recent_project->api_type = RgUtils::ProjectAPIToEnum(project_api);
                            }
                        }
                        recent_projects.push_back(recent_project);
                    }
                }

                node = node->NextSiblingElement(kXmlNodeGlobalRecentProjectRoot);
            }
        }
    }
    return ret;
}

static bool ExtractFontInformation(tinyxml2::XMLNode* node, std::shared_ptr<RgGlobalSettings>& global_settings)
{
    bool ret = false;

    assert(node != nullptr);
    if (node != nullptr)
    {
        // Get the font family element.
        tinyxml2::XMLNode* child_node = node->FirstChildElement(kXmlNodeGlobalFontFamilyType);
        assert(child_node != nullptr);
        if (child_node != nullptr)
        {
            // Read font family element.
            assert(global_settings != nullptr);
            if (global_settings != nullptr)
            {
                ret = RgXMLUtils::ReadNodeTextString(child_node, global_settings->font_family);
                assert(ret);
                if (ret)
                {
                    // Get the font size element.
                    child_node = child_node->NextSiblingElement(kXmlNodeGlobalFontSize);
                    assert(child_node != nullptr);
                    if (child_node != nullptr)
                    {
                        // Read the font size element.
                        unsigned font_size;
                        ret = RgXMLUtils::ReadNodeTextUnsigned(child_node, font_size);
                        assert(ret);
                        if (ret)
                        {
                            global_settings->font_size = font_size;
                        }
                    }
                }

                // Read the include files viewer.
                ret = RgXMLUtils::ReadNodeTextString(child_node, global_settings->include_files_viewer);
                assert(ret);

                if (!ret)
                {
                    // If we could not read the include files viewer, that's OK. Use the system's default.
                    ret                                   = true;
                    global_settings->include_files_viewer = kStrGlobalSettingsSrcViewIncludeViewerDefault;
                }
            }
        }
    }
    return ret;
}

static bool ExtractSplitterConfigs(tinyxml2::XMLNode* node, std::vector<RgSplitterConfig>& splitter_configurations)
{
    bool ret = false;

    assert(node != nullptr);
    if (node != nullptr)
    {
        // Get "Layout" node.
        node = node->FirstChildElement(kXmlNodeGlobalGuiLayout);

        if (node != nullptr)
        {
            // Get "Splitter" node.
            node = node->FirstChildElement(kXmlNodeGlobalGuiSplitter);

            if (node == nullptr)
            {
                // this is a valid scenario if the config file was saved before the user opened a project, which means the splitter values may not have been defined yet.
                ret = true;
            }
            else
            {
                while (node != nullptr)
                {
                    std::string splitter_name;
                    std::string splitter_values;

                    // Get splitter name.
                    tinyxml2::XMLNode* splitter_node = node->FirstChildElement(kXmlNodeGlobalGuiSplitterName);
                    ret                              = RgXMLUtils::ReadNodeTextString(splitter_node, splitter_name);

                    if (!ret)
                    {
                        break;
                    }

                    // Get splitter values.
                    splitter_node = node->FirstChildElement(kXmlNodeGlobalGuiSplitterValues);
                    ret           = RgXMLUtils::ReadNodeTextString(splitter_node, splitter_values);

                    if (!ret)
                    {
                        break;
                    }

                    std::vector<std::string> value_string_list;
                    std::vector<int>         value_int_list;

                    // Create int list of splitter values from comma separated string.
                    RgUtils::splitString(splitter_values, RgConfigManager::kRgaListDelimiter, value_string_list);
                    for (std::string str_value : value_string_list)
                    {
                        value_int_list.push_back(stoi(str_value));
                    }

                    // Create splitter config object.
                    RgSplitterConfig splitter_config;
                    splitter_config.splitter_name   = splitter_name;
                    splitter_config.splitter_values = value_int_list;
                    splitter_configurations.push_back(splitter_config);

                    // Get next "Splitter" node.
                    node = node->NextSiblingElement(kXmlNodeGlobalGuiSplitter);
                }
            }
        }
    }

    return ret;
}

static bool ExtractWindowGeometry(tinyxml2::XMLNode* node, RgWindowConfig& window_config)
{
    bool ret = false;

    assert(node != nullptr);
    if (node != nullptr)
    {
        // Get "WindowSize" node.
        node = node->FirstChildElement(kXmlNodeGlobalGuiWindowGeometry);

        if (node != nullptr)
        {
            std::string window_value_str;

            // Get "WindowWidth" value.
            tinyxml2::XMLNode* window_size_node = node->FirstChildElement(kXmlNodeGlobalGuiWindowWidth);
            if (window_size_node != nullptr)
            {
                ret              = RgXMLUtils::ReadNodeTextString(window_size_node, window_value_str);
                int window_width = stoi(window_value_str);

                // Get "WindowHeight" value.
                window_size_node = node->FirstChildElement(kXmlNodeGlobalGuiWindowHeight);
                if (window_size_node != nullptr)
                {
                    ret               = RgXMLUtils::ReadNodeTextString(window_size_node, window_value_str);
                    int window_height = stoi(window_value_str);

                    // Get "WindowXPos" value.
                    window_size_node = node->FirstChildElement(kXmlNodeGlobalGuiWindowXPos);
                    if (window_size_node != nullptr)
                    {
                        ret              = RgXMLUtils::ReadNodeTextString(window_size_node, window_value_str);
                        int window_x_pos = stoi(window_value_str);

                        // Get "WindowYPos" value.
                        window_size_node = node->FirstChildElement(kXmlNodeGlobalGuiWindowYPos);
                        if (window_size_node != nullptr)
                        {
                            ret              = RgXMLUtils::ReadNodeTextString(window_size_node, window_value_str);
                            int window_y_pos = stoi(window_value_str);

                            // Update window config object.
                            window_config.window_height = window_height;
                            window_config.window_width  = window_width;

                            // If the position values are negative, fix them here.
                            if (window_x_pos < 0)
                            {
                                window_x_pos = 0;
                            }
                            window_config.window_x_pos = window_x_pos;
                            if (window_y_pos < 0)
                            {
                                window_y_pos = 0;
                            }
                            window_config.window_y_pos = window_y_pos;

                            // Get "WindowState" value.
                            window_size_node = node->FirstChildElement(kXmlNodeGlobalGuiWindowState);
                            if (window_size_node != nullptr)
                            {
                                ret                        = RgXMLUtils::ReadNodeTextString(window_size_node, window_value_str);
                                int window_state           = stoi(window_value_str);
                                window_config.window_state = window_state;
                            }
                        }
                    }
                }
            }
        }
    }

    return ret;
}

// Extracts the general build settings in RGA:
// - Target Devices
// - Predefined macros
// - Additional include directories
// - Additional options
bool RgXmlConfigFileReaderImpl::ReadGeneralBuildSettings(tinyxml2::XMLNode* node, std::shared_ptr<RgBuildSettings> build_settings)
{
    bool ret = false;
    if (node != nullptr)
    {
        node = node->FirstChildElement(kXmlNodeTargetDevices);
        ret  = (node != nullptr);
        assert(ret);
        if (ret)
        {
            std::string target_devices;
            bool        has_target_devices = RgXMLUtils::ReadNodeTextString(node, target_devices);
            if (has_target_devices)
            {
                // Target GPUs.
                ExtractTargetGpus(target_devices, build_settings->target_gpus);
                ret = (!build_settings->target_gpus.empty());
                assert(ret);
            }

            if (ret)
            {
                // Predefined macros.
                node = node->NextSiblingElement(kXmlNodePredefinedMacros);
                ret  = (node != nullptr);
                assert(ret);

                if (ret)
                {
                    std::string predefined_macros;
                    bool        should_read = RgXMLUtils::ReadNodeTextString(node, predefined_macros);

                    if (should_read)
                    {
                        ExtractPredefinedMacros(predefined_macros, build_settings->predefined_macros);
                    }

                    // Additional include directories.
                    node = node->NextSiblingElement(kXmlNodeAdditionalIncludeDirectories);
                    ret  = (node != nullptr);
                    assert(ret);

                    if (ret)
                    {
                        std::string additional_include_directories;
                        should_read = RgXMLUtils::ReadNodeTextString(node, additional_include_directories);
                        if (should_read)
                        {
                            ExtractAdditionalIncludeDirectories(additional_include_directories, build_settings->additional_include_directories);
                        }

                        // Read additional build options element.
                        node = node->NextSiblingElement(kXmlNodeAdditionalOptions);
                        ret  = (node != nullptr);
                        assert(ret);
                        if (node != nullptr)
                        {
                            RgXMLUtils::ReadNodeTextString(node, build_settings->additional_options);
                        }
                    }
                }
            }
        }
    }
    return ret;
}

// *** INTERNALLY-LINKED AUXILIARY FUNCTIONS - END ***

// Factory for creating the relevant config file reader in runtime.
class RgXMLConfigFileReaderImplFactory
{
public:
    static std::shared_ptr<RgXmlConfigFileReaderImpl> CreateReader(const std::string& api_name)
    {
        std::shared_ptr<RgXmlConfigFileReaderImpl> ret = nullptr;
        if (api_name.compare(kStrApiNameOpencl) == 0)
        {
            ret = std::make_shared<RgConfigFileReaderOpencl>();
        }
        else if (api_name.compare(kStrApiNameVulkan) == 0)
        {
            ret = std::make_shared<RgConfigFileReaderVulkan>();
        }
        return ret;
    }
};

// Support for reading v2.0 of the project files
static bool ReadProjectConfigFile_2_0(tinyxml2::XMLDocument&      doc,
                                      const char*                 file_data_model_version,
                                      tinyxml2::XMLNode*          project_node,
                                      std::shared_ptr<RgProject>& project)
{
    bool ret = false;
    if (project_node != nullptr)
    {
        bool is_project_node = (std::string(kXmlNodeProject).compare(project_node->Value()) == 0);
        assert(is_project_node);
        ret = is_project_node;
        if (ret)
        {
            // Get the relevant API name.
            tinyxml2::XMLNode* node                = project_node->FirstChild();
            bool               is_program_api_node = (node != nullptr) && (std::string(kXmlNodeApiName).compare(node->Value()) == 0);
            assert(is_program_api_node);
            ret = is_program_api_node;
            if (ret)
            {
                std::string api_name;
                ret = RgXMLUtils::ReadNodeTextString(node, api_name);

                // Create the concrete config file reader for the relevant API.
                std::shared_ptr<RgXmlConfigFileReaderImpl> reader = RgXMLConfigFileReaderImplFactory::CreateReader(api_name);
                if (reader != nullptr)
                {
                    // Let the concrete parser handle the configuration.
                    ret = reader->ReadProjectConfigFile(doc, file_data_model_version, project);
                    assert(ret && project != nullptr);
                }
            }
        }
    }

    return ret;
}

bool RgXmlConfigFile::ReadProjectConfigFile(const std::string& config_file_path, std::shared_ptr<RgProject>& project)
{
    bool ret = false;

    // Reset the output variable.
    project = nullptr;

    // Load the XML document.
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError    rc = doc.LoadFile(config_file_path.c_str());

    if (rc == tinyxml2::XML_SUCCESS)
    {
        // Get the XML declaration node.
        tinyxml2::XMLNode* node = doc.FirstChild();
        if (node != nullptr)
        {
            // Get the RGA Data Model version item.
            node = node->NextSibling();
            if (node != nullptr)
            {
                tinyxml2::XMLElement* elem = node->ToElement();
                if (elem != nullptr)
                {
                    // Get the data model version in order to verify
                    // that the project file is compatible.
                    const char* data_model_version = node->ToElement()->GetText();

                    // All v2.0 and v2.1 project files have the same format
                    // for the initial <Program> and <ProgramAPI> tags.
                    if (kRgaDataModel2_0.compare(data_model_version) == 0 || kRgaDataModel2_1.compare(data_model_version) == 0 ||
                        kRgaDataModel2_2.compare(data_model_version) == 0 || kRgaDataModel2_3.compare(data_model_version) == 0)
                    {
                        // Skip to the <Program> node.
                        node = node->NextSibling();
                        ret  = ReadProjectConfigFile_2_0(doc, data_model_version, node, project);
                    }
                    else
                    {
                        assert(!"RGA Data Model version is not supported");
                        ret = false;
                    }

                    if (ret)
                    {
                        // Set the project file path.
                        project->project_file_full_path = config_file_path;
                    }
                }
            }
        }
    }

    return ret;
}

// ***************************
// *** WRITER AREA - BEGIN ***
// ***************************

bool RgXmlConfigFileWriterImpl::WriteGeneralBuildSettings(const std::shared_ptr<RgBuildSettings> build_settings,
                                                          tinyxml2::XMLDocument&                 doc,
                                                          tinyxml2::XMLElement*                  build_settings_element)
{
    bool ret = false;

    assert(build_settings != nullptr);
    assert(build_settings_element != nullptr);
    if (build_settings != nullptr && build_settings_element != nullptr)
    {
        // Target devices.
        RgXMLUtils::AppendXMLElement(
            doc, build_settings_element, kXmlNodeTargetDevices, RgUtils::BuildSemicolonSeparatedStringList(build_settings->target_gpus).c_str());

        // Predefined Macros.
        RgXMLUtils::AppendXMLElement(
            doc, build_settings_element, kXmlNodePredefinedMacros, RgUtils::BuildSemicolonSeparatedStringList(build_settings->predefined_macros).c_str());

        // Additional include directories.
        RgXMLUtils::AppendXMLElement(doc,
                                     build_settings_element,
                                     kXmlNodeAdditionalIncludeDirectories,
                                     RgUtils::BuildSemicolonSeparatedStringList(build_settings->additional_include_directories).c_str());

        // Additional options.
        RgXMLUtils::AppendXMLElement(doc, build_settings_element, kXmlNodeAdditionalOptions, build_settings->additional_options.c_str());

        ret = true;
    }

    return ret;
}

void RgXmlConfigFileWriterImpl::AddConfigFileDeclaration(tinyxml2::XMLDocument& doc)
{
    tinyxml2::XMLNode* node = doc.NewDeclaration(kRgaXmlDeclaration);
    doc.InsertEndChild(node);

    // Create the Data Model Version element.
    tinyxml2::XMLElement* element = doc.NewElement(kXmlNodeDataModelVersion);
    element->SetText(kRgaDataModel.c_str());
    doc.LinkEndChild(element);
}

bool RgXmlGraphicsConfigFileReaderImpl::ReadPipeline(tinyxml2::XMLDocument& doc,
                                                     tinyxml2::XMLNode*     parent_clone,
                                                     bool                   is_backup_spv,
                                                     RgPipelineShaders&     pipeline) const
{
    bool ret = false;

    assert(parent_clone != nullptr);
    if (parent_clone != nullptr)
    {
        tinyxml2::XMLElement* pipeline_element = parent_clone->FirstChildElement(is_backup_spv ? kXmlNodeBackupSpvRoot : kXmlNodePipelineShadersRoot);
        ret                                    = (is_backup_spv || pipeline_element != nullptr);
        assert(ret);

        if (pipeline_element != nullptr)
        {
            tinyxml2::XMLNode* pipeline_type_node = pipeline_element->FirstChildElement(kXmlNodePipelineType);
            assert(pipeline_type_node != nullptr);
            if (pipeline_type_node != nullptr)
            {
                // Read the pipeline type node.
                std::string pipeline_type;
                ret = RgXMLUtils::ReadNodeTextString(pipeline_type_node, pipeline_type);

                // Lambda that implements reading a single pipeline file path element.
                auto ReadPipelineShaderFile =
                    [&](RgPipelineStage stage, const char* tag, std::function<bool(RgPipelineStage, const std::string&, RgPipelineShaders&)> f) {
                        std::string        shader_full_file_path;
                        tinyxml2::XMLNode* node = pipeline_type_node->NextSiblingElement(tag);
                        if (node != nullptr)
                        {
                            RgXMLUtils::ReadNodeTextString(node, shader_full_file_path);
                            f(stage, shader_full_file_path, pipeline);
                        }
                    };

                if (ret && (pipeline_type.compare(kXmlNodePipelineTypeGraphics) == 0))
                {
                    // Create a new graphics pipeline object.
                    pipeline.type = RgPipelineType::kGraphics;

                    // Read the paths to shader input files.
                    ReadPipelineShaderFile(RgPipelineStage::kVertex, kXmlNodePipelineVertexStage, RgUtils::SetStageShaderPath);
                    ReadPipelineShaderFile(RgPipelineStage::kTessellationControl, kXmlNodePipelineTessControlStage, RgUtils::SetStageShaderPath);
                    ReadPipelineShaderFile(RgPipelineStage::kTessellationEvaluation, kXmlNodePipelineTessEvalStage, RgUtils::SetStageShaderPath);
                    ReadPipelineShaderFile(RgPipelineStage::kGeometry, kXmlNodePipelineGeometryStage, RgUtils::SetStageShaderPath);
                    ReadPipelineShaderFile(RgPipelineStage::kFragment, kXmlNodePipelineFragmentStage, RgUtils::SetStageShaderPath);
                }
                else if (ret && (pipeline_type.compare(kXmlNodePipelineTypeCompute) == 0))
                {
                    // Create a new compute pipeline object.
                    pipeline.type = RgPipelineType::kCompute;

                    // Read the compute shader input file.
                    ReadPipelineShaderFile(RgPipelineStage::kCompute, kXmlNodePipelineComputeStage, RgUtils::SetStageShaderPath);
                }
                else
                {
                    assert(false);
                }
            }
        }
    }
    return ret;
}

bool RgXmlGraphicsConfigFileReaderImpl::ReadPipelineState(std::shared_ptr<RgGraphicsProjectClone> clone,
                                                          tinyxml2::XMLDocument&                  doc,
                                                          tinyxml2::XMLNode*                      pipeline_state_element) const
{
    bool ret = false;

    // Find the project clone's pipeline state root element.
    assert(pipeline_state_element != nullptr);
    if (pipeline_state_element != nullptr)
    {
        // Find the first pipeline state element.
        tinyxml2::XMLNode* pipeline_state_node = pipeline_state_element->FirstChildElement(kXmlNodePipelineState);

        if (pipeline_state_node != nullptr)
        {
            // Loop over each state node to read the pipeline properties.
            do
            {
                if (pipeline_state_node != nullptr)
                {
                    RgPipelineState state;

                    // Read the PSO name.
                    tinyxml2::XMLNode* pso_name_node = pipeline_state_node->FirstChildElement(kXmlNodePipelineName);
                    RgXMLUtils::ReadNodeTextString(pso_name_node, state.name);

                    // Read the "is active" flag.
                    tinyxml2::XMLNode* is_active_node = pipeline_state_node->FirstChildElement(kXmlNodePipelineIsActive);
                    RgXMLUtils::ReadNodeTextBool(is_active_node, state.is_active);

                    // Read the pipeline state file path.
                    tinyxml2::XMLNode* pipeline_state_file_path_node = pipeline_state_node->FirstChildElement(kXmlNodePipelineStateFilePath);
                    RgXMLUtils::ReadNodeTextString(pipeline_state_file_path_node, state.pipeline_state_file_path);

                    // Read the original pipeline state file path.
                    tinyxml2::XMLNode* original_pipeline_state_file_path_node = pipeline_state_node->FirstChildElement(kXmlNodeOriginalPipelineStateFilePath);
                    RgXMLUtils::ReadNodeTextString(original_pipeline_state_file_path_node, state.original_pipeline_state_file_path);

                    clone->pso_states.push_back(state);
                }

                // Try to move on to processing the next state node.
                pipeline_state_node = pipeline_state_node->NextSiblingElement(kXmlNodePipelineState);

            } while (pipeline_state_node != nullptr);
        }

        ret = true;
    }

    return ret;
}

bool RgXmlGraphicsConfigFileWriterImpl::WritePipeline(tinyxml2::XMLDocument&   doc,
                                                      tinyxml2::XMLElement*    parent_clone,
                                                      bool                     is_backup_spv,
                                                      const RgPipelineShaders& pipeline) const
{
    bool ret = false;

    assert(parent_clone != nullptr);
    if (parent_clone != nullptr)
    {
        // Create the project clone's pipeline root element.
        tinyxml2::XMLElement* pipeline_node = doc.NewElement(is_backup_spv ? kXmlNodeBackupSpvRoot : kXmlNodePipelineShadersRoot);

        assert(pipeline_node != nullptr);
        if (pipeline_node != nullptr)
        {
            bool        is_graphics_pipeline = pipeline.type == RgPipelineType::kGraphics;
            const char* pipeline_type        = is_graphics_pipeline ? kXmlNodePipelineTypeGraphics : kXmlNodePipelineTypeCompute;
            RgXMLUtils::AppendXMLElement(doc, pipeline_node, kXmlNodePipelineType, pipeline_type);

            // Lambda that implements writing a single pipeline file path element.
            auto AppendPipelineShaderFile =
                [&](RgPipelineStage stage, const char* tag, std::function<bool(const RgPipelineShaders&, RgPipelineStage, std::string&)> f) {
                    std::string shader_path;
                    if (f(pipeline, stage, shader_path))
                    {
                        RgXMLUtils::AppendXMLElement(doc, pipeline_node, tag, shader_path.c_str());
                    }
                };

            if (is_graphics_pipeline)
            {
                // Append each graphics pipeline shader stage's input file.
                AppendPipelineShaderFile(RgPipelineStage::kVertex, kXmlNodePipelineVertexStage, RgUtils::GetStageShaderPath);
                AppendPipelineShaderFile(RgPipelineStage::kTessellationControl, kXmlNodePipelineTessControlStage, RgUtils::GetStageShaderPath);
                AppendPipelineShaderFile(RgPipelineStage::kTessellationEvaluation, kXmlNodePipelineTessEvalStage, RgUtils::GetStageShaderPath);
                AppendPipelineShaderFile(RgPipelineStage::kGeometry, kXmlNodePipelineGeometryStage, RgUtils::GetStageShaderPath);
                AppendPipelineShaderFile(RgPipelineStage::kFragment, kXmlNodePipelineFragmentStage, RgUtils::GetStageShaderPath);
            }
            else
            {
                // Append the pipeline's compute shader input file.
                AppendPipelineShaderFile(RgPipelineStage::kCompute, kXmlNodePipelineComputeStage, RgUtils::GetStageShaderPath);
            }

            // Insert the pipeline element into the parent clone element.
            parent_clone->InsertEndChild(pipeline_node);
            ret = true;
        }
    }

    return ret;
}

bool RgXmlGraphicsConfigFileWriterImpl::WritePipelineState(const std::shared_ptr<RgGraphicsProjectClone> clone,
                                                           tinyxml2::XMLDocument&                        doc,
                                                           tinyxml2::XMLElement*                         parent_clone_element) const
{
    bool ret = false;

    assert(clone != nullptr);
    assert(parent_clone_element != nullptr);
    if (clone != nullptr && parent_clone_element != nullptr)
    {
        tinyxml2::XMLElement* pipeline_state_root = doc.NewElement(kXmlNodePipelineStateRoot);

        assert(pipeline_state_root != nullptr);
        if (pipeline_state_root != nullptr)
        {
            // Loop over each pipeline state in the clone.
            for (auto pipeline_state : clone->pso_states)
            {
                // Create a new element for writing each state info.
                tinyxml2::XMLElement* pipeline_state_element = doc.NewElement(kXmlNodePipelineState);

                // Append the state name, the active flag, and the pipeline state file path data.
                RgXMLUtils::AppendXMLElement(doc, pipeline_state_element, kXmlNodePipelineName, pipeline_state.name.c_str());
                RgXMLUtils::AppendXMLElement(doc, pipeline_state_element, kXmlNodePipelineIsActive, pipeline_state.is_active);
                RgXMLUtils::AppendXMLElement(doc, pipeline_state_element, kXmlNodePipelineStateFilePath, pipeline_state.pipeline_state_file_path.data());
                RgXMLUtils::AppendXMLElement(
                    doc, pipeline_state_element, kXmlNodeOriginalPipelineStateFilePath, pipeline_state.original_pipeline_state_file_path.data());

                // Insert the state into the list of pipeline states.
                pipeline_state_root->InsertEndChild(pipeline_state_element);
            }

            // Insert the pipeline state element in the project clone.
            parent_clone_element->InsertEndChild(pipeline_state_root);

            ret = true;
        }
    }

    return ret;
}

class ConfigFileWriterFactory
{
public:
    static std::shared_ptr<RgXmlConfigFileWriterImpl> CreateWriter(RgProjectAPI api)
    {
        std::shared_ptr<RgXmlConfigFileWriterImpl> ret = nullptr;
        switch (api)
        {
        case RgProjectAPI::kOpenCL:
            ret = std::make_shared<RgConfigFileWriterOpencl>();
            break;
        case RgProjectAPI::kVulkan:
            ret = std::make_shared<RgConfigFileWriterVulkan>();
            break;
        case RgProjectAPI::kUnknown:
        default:
            // If we got here, there's a problem because the API type is unrecognized.
            assert(false);
            break;
        }
        return ret;
    }
};

bool RgXmlConfigFile::WriteProjectConfigFile(const RgProject& project, const std::string& config_file_path)
{
    bool ret = false;

    std::shared_ptr<RgXmlConfigFileWriterImpl> writer = ConfigFileWriterFactory::CreateWriter(project.api);
    if (writer != nullptr)
    {
        ret = writer->WriteProjectConfigFile(project, config_file_path);
    }

    return ret;
}

// *************************
// *** WRITER AREA - END ***
// *************************

// **************************
// Global configuration file.
// **************************
bool RgXmlConfigFile::ReadGlobalSettings(const std::string& global_config_file_path, std::shared_ptr<RgGlobalSettings>& global_settings)
{
    bool ret = false;

    // Reset the output variable.
    global_settings = std::make_shared<RgGlobalSettings>();

    // Load the XML document.
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError    rc = doc.LoadFile(global_config_file_path.c_str());

    if (rc == tinyxml2::XML_SUCCESS)
    {
        // Get the XML declaration node.
        tinyxml2::XMLNode* node = doc.FirstChild();
        if (node != nullptr)
        {
            // Get the RGA Data Model version element.
            node = node->NextSibling();
            if (node != nullptr)
            {
                tinyxml2::XMLElement* elem = node->ToElement();
                if (elem != nullptr)
                {
                    // Determine which data model version to load.
                    const char* data_model_version = node->ToElement()->GetText();
                    RgConfigManager::Instance().SetConfigFileDataModelVersion(data_model_version);
                    try
                    {
                        if (kRgaDataModel2_0.compare(data_model_version) == 0)
                        {
                            // Get next sibling, which should be the GlobalSettings element.
                            tinyxml2::XMLNode* global_settings_node = node->NextSibling();
                            ret                                     = ExtractGlobalSettings_2_0(global_settings_node, global_settings);
                        }
                        else if (kRgaDataModel2_1.compare(data_model_version) == 0)
                        {
                            // Get Next sibling, which should be the GlobalSettings element.
                            tinyxml2::XMLNode* global_settings_node = node->NextSibling();
                            ret                                     = ExtractGlobalSettings_2_1(global_settings_node, global_settings);
                        }
                        else if (kRgaDataModel2_2.compare(data_model_version) == 0)
                        {
                            // Get Next sibling, which should be the GlobalSettings element.
                            tinyxml2::XMLNode* global_settings_node = node->NextSibling();
                            ret                                     = ExtractGlobalSettings_2_2(global_settings_node, global_settings);
                        }
                        else if (kRgaDataModel2_3.compare(data_model_version) == 0)
                        {
                            // Get Next sibling, which should be the GlobalSettings element.
                            tinyxml2::XMLNode* global_settings_node = node->NextSibling();
                            ret                                     = ExtractGlobalSettings_2_3(global_settings_node, global_settings);
                        }
                        else
                        {
                            // Data model version is not supported.
                            ret = false;
                        }
                    }
                    catch (...)
                    {
                        ret = false;
                    }
                }

                // Null xml element indicates a read failure at some point.
                if (elem == nullptr)
                {
                    ret = false;
                }
            }
        }
    }

    return ret;
}

bool RgXmlConfigFile::WriteGlobalSettings(std::shared_ptr<RgGlobalSettings> global_settings, const std::string& global_config_file_path)
{
    assert(global_settings != nullptr);

    bool ret = false;

    if (global_settings != nullptr)
    {
        // Create the XML declaration node.
        tinyxml2::XMLDocument doc;
        RgXmlConfigFileWriterImpl::AddConfigFileDeclaration(doc);

        // Create the Global Settings element.
        tinyxml2::XMLElement* global_settings_elem = doc.NewElement(kXmlNodeGlobalLogFileGlobalSettings);
        assert(global_settings_elem != nullptr);
        if (global_settings_elem != nullptr)
        {
            // Create the log file location element.
            RgXMLUtils::AppendXMLElement(doc, global_settings_elem, kXmlNodeGlobalLogFileLocation, global_settings->log_file_location.c_str());

            // Create the project file location element.
            RgXMLUtils::AppendXMLElement(doc, global_settings_elem, kXmlNodeGlobalProjectFileLocation, global_settings->project_file_location.c_str());

            // Create the last selected directory element.
            RgXMLUtils::AppendXMLElement(doc, global_settings_elem, kXmlNodeGlobalLastSelectedDirectory, global_settings->last_selected_directory.c_str());

            // Create a root node for the list of recent projects.
            tinyxml2::XMLElement* recent_projects_elem = doc.NewElement(kXmlNodeGlobalRecentProjectsRoot);
            assert(recent_projects_elem != nullptr);
            if (recent_projects_elem != nullptr)
            {
                if (global_settings->recent_projects.size() > 0)
                {
                    for (size_t project_index = 0; project_index < global_settings->recent_projects.size(); ++project_index)
                    {
                        tinyxml2::XMLElement* recent_project_elem = doc.NewElement(kXmlNodeGlobalRecentProjectRoot);
                        if (recent_project_elem != nullptr)
                        {
                            // Save the most recent files, starting at the end of the list.
                            assert(global_settings->recent_projects[project_index] != nullptr);
                            if (global_settings->recent_projects[project_index] != nullptr)
                            {
                                const std::string& project_path = global_settings->recent_projects[project_index]->project_path;

                                // Write the project path into the recent project element.
                                RgXMLUtils::AppendXMLElement(doc, recent_project_elem, kXmlNodeGlobalRecentProjectPath, project_path.c_str());

                                // Save the project api type.
                                std::string api_type;
                                bool        ok = RgUtils::ProjectAPIToString(global_settings->recent_projects[project_index]->api_type, api_type);
                                assert(ok);
                                if (ok)
                                {
                                    // Write the project api type into the recent project element.
                                    RgXMLUtils::AppendXMLElement(doc, recent_project_elem, kXmlNodeGlobalRecentProjectApi, api_type.c_str());
                                }
                                recent_projects_elem->InsertEndChild(recent_project_elem);
                            }
                        }
                    }
                }

                // Add the list of recent projects to the global settings node.
                global_settings_elem->InsertEndChild(recent_projects_elem);
            }

            // Create a root node for the font type and size.
            tinyxml2::XMLElement* font_family_element = doc.NewElement(kXmlNodeGlobalFontFamilyRoot);
            assert(font_family_element != nullptr);
            if (font_family_element != nullptr)
            {
                // Write the font family into the font element.
                RgXMLUtils::AppendXMLElement(doc, font_family_element, kXmlNodeGlobalFontFamilyType, global_settings->font_family.c_str());

                // Write the font size into the font element.
                RgXMLUtils::AppendXMLElement(doc, font_family_element, kXmlNodeGlobalFontSize, global_settings->font_size);

                // Add the font element to the global settings node.
                global_settings_elem->InsertEndChild(font_family_element);
            }

            // Write the include files viewer.
            RgXMLUtils::AppendXMLElement(doc, global_settings_elem, kXmlNodeGlobalIncludeFilesViewer, global_settings->include_files_viewer.c_str());

            // Create a comma-separated list from the column names that we have.
            std::string disassembly_column_str = RgUtils::BuildSemicolonSeparatedBoolList(global_settings->visible_disassembly_view_columns);
            RgXMLUtils::AppendXMLElement(doc, global_settings_elem, kXmlNodeGlobalDisassemblyColumns, disassembly_column_str.c_str());

            // An element used to determine if project names will be generated or provided by the user.
            RgXMLUtils::AppendXMLElement(doc, global_settings_elem, kXmlNodeUseGeneratedProjectNames, global_settings->use_default_project_name);

            RgXMLUtils::AppendXMLElement(doc, global_settings_elem, kXmlNodeGlobalDefaultApi, static_cast<int>(global_settings->default_api));

            RgXMLUtils::AppendXMLElement(doc, global_settings_elem, kXmlNodeGlobalPromptForApi, global_settings->should_prompt_for_api);

            RgXMLUtils::AppendXMLElement(doc, global_settings_elem, kXmlNodeGlobalInputFileExtGlsl, global_settings->input_file_ext_glsl.c_str());
            RgXMLUtils::AppendXMLElement(doc, global_settings_elem, kXmlNodeGlobalInputFileExtHlsl, global_settings->input_file_ext_hlsl.c_str());
            RgXMLUtils::AppendXMLElement(doc, global_settings_elem, kXmlNodeGlobalInputFileExtSpvTxt, global_settings->input_file_ext_spv_txt.c_str());
            RgXMLUtils::AppendXMLElement(doc, global_settings_elem, kXmlNodeGlobalInputFileExtSpvBin, global_settings->input_file_ext_spv_bin.c_str());

            RgXMLUtils::AppendXMLElement(doc, global_settings_elem, kXmlNodeGlobalDefaultSrcLang, (uint32_t)global_settings->default_lang);

            // Create "GUI" element.
            tinyxml2::XMLElement* gui_element = doc.NewElement(kXmlNodeGlobalGui);
            assert(gui_element != nullptr);
            if (gui_element != nullptr)
            {
                // Create "Layout" element.
                tinyxml2::XMLElement* layout_element = doc.NewElement(kXmlNodeGlobalGuiLayout);
                assert(layout_element != nullptr);
                if (layout_element != nullptr)
                {
                    if (global_settings->gui_layout_splitters.size() > 0)
                    {
                        for (RgSplitterConfig splitter_config : global_settings->gui_layout_splitters)
                        {
                            // Create "Splitter" element.
                            tinyxml2::XMLElement* splitter_element = doc.NewElement(kXmlNodeGlobalGuiSplitter);
                            assert(splitter_element != nullptr);
                            if (splitter_element != nullptr)
                            {
                                // Add "SplitterName" element.
                                RgXMLUtils::AppendXMLElement(doc, splitter_element, kXmlNodeGlobalGuiSplitterName, splitter_config.splitter_name.c_str());

                                // Add "SplitterValues" element.
                                std::string splitter_value_str = RgUtils::BuildSemicolonSeparatedIntList(splitter_config.splitter_values);
                                RgXMLUtils::AppendXMLElement(doc, splitter_element, kXmlNodeGlobalGuiSplitterValues, splitter_value_str.c_str());

                                // End of "Splitter" element.
                                layout_element->InsertEndChild(splitter_element);
                            }
                        }
                    }

                    // End of "Layout" element.
                    gui_element->InsertEndChild(layout_element);
                }

                // Create WindowSize element.
                tinyxml2::XMLElement* window_size_element = doc.NewElement(kXmlNodeGlobalGuiWindowGeometry);
                assert(window_size_element != nullptr);
                if (window_size_element != nullptr)
                {
                    // Add window X position element.
                    RgXMLUtils::AppendXMLElement(doc, window_size_element, kXmlNodeGlobalGuiWindowXPos, global_settings->gui_window_config.window_x_pos);

                    // Add window Y position element.
                    RgXMLUtils::AppendXMLElement(doc, window_size_element, kXmlNodeGlobalGuiWindowYPos, global_settings->gui_window_config.window_y_pos);

                    // Add window width element.
                    RgXMLUtils::AppendXMLElement(doc, window_size_element, kXmlNodeGlobalGuiWindowWidth, global_settings->gui_window_config.window_width);

                    // Add window height element.
                    RgXMLUtils::AppendXMLElement(doc, window_size_element, kXmlNodeGlobalGuiWindowHeight, global_settings->gui_window_config.window_height);

                    // Add window state.
                    RgXMLUtils::AppendXMLElement(doc, window_size_element, kXmlNodeGlobalGuiWindowState, global_settings->gui_window_config.window_state);

                    gui_element->InsertEndChild(window_size_element);
                }

                // End of "GUI" element.
                global_settings_elem->InsertEndChild(gui_element);
            }

            // Add the Global Settings element to the document.
            doc.InsertEndChild(global_settings_elem);

            // Create the Default Build Settings element.
            tinyxml2::XMLElement* default_build_settings = doc.NewElement(kXmlNodeGlobalDefaultBuildSettings);
            if (default_build_settings != nullptr)
            {
                // Loop through each API type, and use an API-specific rgConfigFileWriter to write the default settings.
                for (int api_index = static_cast<int>(RgProjectAPI::kOpenCL); api_index < static_cast<int>(RgProjectAPI::kApiCount); ++api_index)
                {
                    RgProjectAPI current_api = static_cast<RgProjectAPI>(api_index);

                    std::string api_string;
                    bool        is_ok = RgUtils::ProjectAPIToString(current_api, api_string);
                    assert(is_ok);
                    if (is_ok)
                    {
                        std::shared_ptr<RgXmlConfigFileWriterImpl> writer = ConfigFileWriterFactory::CreateWriter(current_api);
                        assert(writer != nullptr);
                        if (writer != nullptr)
                        {
                            // Add the new API element.
                            tinyxml2::XMLElement* api_default_settings = doc.NewElement(api_string.c_str());

                            // Create the Build Settings element for the API.
                            tinyxml2::XMLElement* build_settings_node = doc.NewElement(kXmlNodeBuildSettings);
                            assert(build_settings_node != nullptr);
                            if (build_settings_node != nullptr)
                            {
                                // Write the API-specific default build settings element.
                                std::shared_ptr<RgBuildSettings> default_api_build_settings = global_settings->default_build_settings[api_string];
                                assert(default_api_build_settings != nullptr);
                                if (default_api_build_settings != nullptr)
                                {
                                    ret = writer->WriteBuildSettingsElement(default_api_build_settings, doc, build_settings_node);
                                }

                                // Insert the build settings node into the API parent node.
                                api_default_settings->InsertEndChild(build_settings_node);
                            }

                            // Insert the OpenCL-specific default build settings element to the document.
                            default_build_settings->InsertEndChild(api_default_settings);
                        }
                    }
                }

                // Insert the Default Build Settings element to the document.
                doc.InsertEndChild(default_build_settings);

                // Save the configuration file.
                tinyxml2::XMLError rc = doc.SaveFile(global_config_file_path.c_str());
                if (rc == tinyxml2::XML_SUCCESS)
                {
                    ret = true;
                }
                assert(ret);
            }
        }
    }
    return ret;
}

static bool ExtractDefaultBuildSettings_2_0(tinyxml2::XMLNode* default_build_settings_node, std::shared_ptr<RgGlobalSettings>& global_settings)
{
    assert(default_build_settings_node != nullptr);
    assert(global_settings != nullptr);

    // Make sure this is actually the DefaultBuildSettings node.
    bool ret = ((global_settings != nullptr) && (default_build_settings_node != nullptr) &&
                (std::strcmp(default_build_settings_node->Value(), kXmlNodeGlobalDefaultBuildSettings) == 0));

    if (ret)
    {
        // Get the OpenCL default build settings element.
        tinyxml2::XMLNode* opencl_node = default_build_settings_node->FirstChildElement(kStrApiNameOpencl);
        if (opencl_node != nullptr)
        {
            tinyxml2::XMLElement* elem = opencl_node->FirstChildElement(kXmlNodeBuildSettings);
            assert(elem != nullptr);
            ret = elem != nullptr;
            if (ret)
            {
                // Use the API-specific factory to create an API-specific build settings object.
                std::shared_ptr<RgFactory> api_factory = RgFactory::CreateFactory(RgProjectAPI::kOpenCL);
                assert(api_factory != nullptr);
                ret = api_factory != nullptr;
                if (api_factory != nullptr)
                {
                    std::shared_ptr<RgBuildSettings> api_build_settings = api_factory->CreateBuildSettings(nullptr);
                    assert(api_build_settings != nullptr);
                    ret = api_build_settings != nullptr;
                    if (api_build_settings != nullptr)
                    {
                        // Create an API-specific build settings instance and parser object to read the settings.
                        std::shared_ptr<RgXmlConfigFileReaderImpl> reader = RgXMLConfigFileReaderImplFactory::CreateReader(kStrApiNameOpencl);
                        assert(reader != nullptr);
                        ret = reader != nullptr;
                        if (reader != nullptr)
                        {
                            tinyxml2::XMLNode* build_settings_node = opencl_node->FirstChildElement(kXmlNodeBuildSettings);
                            assert(build_settings_node != nullptr);
                            ret = build_settings_node != nullptr;
                            if (build_settings_node != nullptr)
                            {
                                // Extract the build settings common to all APIs.
                                ret = reader->ReadGeneralBuildSettings(build_settings_node, api_build_settings);
                                assert(ret);

                                // Extract the default build settings.
                                const std::string version = RgConfigManager::Instance().GetConfigFileDataModelVersion();
                                ret                       = reader->ReadApiBuildSettings(build_settings_node, api_build_settings, version);
                                assert(ret);

                                // Store the default OpenCL build settings in our data object.
                                global_settings->default_build_settings[kStrApiNameOpencl] = api_build_settings;
                            }
                        }
                    }
                }
            }
        }
    }

    return ret;
}

static bool ExtractDefaultBuildSettings_2_1(tinyxml2::XMLNode* default_build_settings_node, std::shared_ptr<RgGlobalSettings>& global_settings)
{
    assert(default_build_settings_node != nullptr);
    assert(global_settings != nullptr);

    // Make sure this is actually the DefaultBuildSettings node
    bool ret = ((global_settings != nullptr) && (default_build_settings_node != nullptr) &&
                (std::strcmp(default_build_settings_node->Value(), kXmlNodeGlobalDefaultBuildSettings) == 0));

    if (ret)
    {
        // Get the first default build settings element.
        tinyxml2::XMLNode* api_settings_node = default_build_settings_node->FirstChildElement();

        // Loop through each possible API type and read each API's default build settings.
        for (int api_index = static_cast<int>(RgProjectAPI::kOpenCL); api_index < static_cast<int>(RgProjectAPI::kApiCount); ++api_index)
        {
            if (api_settings_node == nullptr)
            {
                // If there are missing APIs from the config file, just break from the loop.
                break;
            }
            else
            {
                RgProjectAPI current_api = static_cast<RgProjectAPI>(api_index);

                std::string api_string;
                bool        is_ok = RgUtils::ProjectAPIToString(current_api, api_string);
                assert(is_ok);
                if (is_ok)
                {
                    // Use the API-specific factory to create an API-specific build settings object.
                    std::shared_ptr<RgFactory> api_factory = RgFactory::CreateFactory(current_api);
                    assert(api_factory != nullptr);
                    ret = api_factory != nullptr;
                    if (api_factory != nullptr)
                    {
                        std::shared_ptr<RgBuildSettings> api_build_settings = api_factory->CreateBuildSettings(nullptr);
                        assert(api_build_settings != nullptr);
                        ret = api_build_settings != nullptr;
                        if (api_build_settings != nullptr)
                        {
                            // Create an API-specific build settings instance and parser object to read the settings.
                            std::shared_ptr<RgXmlConfigFileReaderImpl> reader = RgXMLConfigFileReaderImplFactory::CreateReader(api_string.c_str());
                            assert(reader != nullptr);
                            ret = reader != nullptr;
                            if (reader != nullptr)
                            {
                                tinyxml2::XMLNode* build_settings_node = api_settings_node->FirstChildElement(kXmlNodeBuildSettings);
                                assert(build_settings_node != nullptr);
                                ret = build_settings_node != nullptr;
                                if (build_settings_node != nullptr && build_settings_node->FirstChild() != nullptr)
                                {
                                    // Extract the build settings common to all APIs.
                                    ret = reader->ReadGeneralBuildSettings(build_settings_node, api_build_settings);
                                    assert(ret);

                                    // Extract the API-specific default build settings.
                                    const std::string version                = RgConfigManager::Instance().GetConfigFileDataModelVersion();
                                    bool              has_api_build_settings = reader->ReadApiBuildSettings(build_settings_node, api_build_settings, version);

                                    // TEMP: we don't have any valid Vulkan settings, so this can't be a required read.
                                    if (api_index == static_cast<int>(RgProjectAPI::kVulkan))
                                    {
                                        // This is temporarily okay for VUlkan until we get some Vulkan-specific build settings.
                                    }
                                    else
                                    {
                                        // OpenCL, or possibly an error case.
                                        assert(has_api_build_settings);
                                        ret = has_api_build_settings;
                                    }

                                    // Store the default API-specific build settings in our data object.
                                    global_settings->default_build_settings[api_string] = api_build_settings;
                                }
                            }
                        }
                    }
                }

                // Advance to the next API-specific build settings node.
                api_settings_node = api_settings_node->NextSibling();
            }
        }
    }

    return ret;
}

static bool ExtractGlobalSettings_2_0(tinyxml2::XMLNode* global_settings_node, std::shared_ptr<RgGlobalSettings>& global_settings)
{
    assert(global_settings_node != nullptr);
    assert(global_settings != nullptr);

    // Make sure this is actually the GlobalSettings nodes.
    bool ret = ((global_settings != nullptr) && (global_settings_node != nullptr) &&
                (std::strcmp(global_settings_node->Value(), kXmlNodeGlobalLogFileGlobalSettings) == 0));

    if (ret)
    {
        // Read the global settings.
        tinyxml2::XMLElement* elem = global_settings_node->FirstChildElement(kXmlNodeGlobalLogFileLocation);
        if (ret && elem != nullptr)
        {
            // Read the log file location.
            ret = RgXMLUtils::ReadNodeTextString(elem, global_settings->log_file_location);

            elem = elem->NextSiblingElement(kXmlNodeGlobalLastSelectedDirectory);
            if (ret && elem != nullptr)
            {
                // Attempt to read the last selected directory. It might be empty, in which case the call returns false, and that's ok.
                RgXMLUtils::ReadNodeTextString(elem, global_settings->last_selected_directory);

                elem = elem->NextSiblingElement(kXmlNodeGlobalRecentProjectsRoot);
                if (ret && elem != nullptr)
                {
                    // Read the list of recently-accessed projects.
                    ret = ExtractRecentProjects(elem, global_settings->recent_projects);

                    elem = elem->NextSiblingElement(kXmlNodeGlobalDisassemblyColumns);
                    if (ret && elem != nullptr)
                    {
                        // Get the disassembly columns as a single string.
                        std::string tmp_disassembly_columns;
                        ret = RgXMLUtils::ReadNodeTextString(elem, tmp_disassembly_columns);

                        // Parse the disassembly columns string, and save the items in the data object.
                        ExtractDisassemblyColumns(tmp_disassembly_columns, global_settings->visible_disassembly_view_columns);

                        // Read the option that determines if the project names are generated or provided by the user.
                        elem = elem->NextSiblingElement(kXmlNodeUseGeneratedProjectNames);
                        if (ret && elem != nullptr)
                        {
                            ret = RgXMLUtils::ReadNodeTextBool(elem, global_settings->use_default_project_name);
                            assert(ret);

                            elem = elem->NextSiblingElement(kXmlNodeGlobalGui);
                            if (ret && elem != nullptr)
                            {
                                // Extract splitter config objects from the "GUI" node.
                                ret = ExtractSplitterConfigs(elem, global_settings->gui_layout_splitters);
                            }
                        }
                    }
                }
            }
        }

        // If the Global Settings could be read, then attempt to read the Default Build Settings.
        if (ret)
        {
            tinyxml2::XMLNode* default_build_settings_node = global_settings_node->NextSibling();
            ret                                            = ExtractDefaultBuildSettings_2_0(default_build_settings_node, global_settings);
        }
    }

    return ret;
}

static bool ExtractGlobalSettings_2_1(tinyxml2::XMLNode* global_settings_node, std::shared_ptr<RgGlobalSettings>& global_settings)
{
    // Make sure this is actually the GlobalSettings nodes.
    tinyxml2::XMLElement* elem = nullptr;
    bool                  ret  = (std::strcmp(global_settings_node->Value(), kXmlNodeGlobalLogFileGlobalSettings) == 0);

    if (ret)
    {
        // Read the log file location.
        elem = global_settings_node->FirstChildElement(kXmlNodeGlobalLogFileLocation);
        ret  = elem != nullptr;
        ret  = ret && RgXMLUtils::ReadNodeTextString(elem, global_settings->log_file_location);

        // If the saved log directory does not exist, fall back to using the default one.
        if (!RgUtils::IsDirExists(global_settings->log_file_location))
        {
            std::string app_data_dir;
            RgConfigManager::GetDefaultDataFolder(app_data_dir);
            global_settings->log_file_location = app_data_dir;
        }
    }
    if (ret)
    {
        // Attempt to read the last selected directory. It might be empty, in which case the call returns false, and that's ok.
        elem = elem->NextSiblingElement(kXmlNodeGlobalLastSelectedDirectory);
        ret &= (elem != nullptr);
        if (ret)
        {
            RgXMLUtils::ReadNodeTextString(elem, global_settings->last_selected_directory);
        }
    }
    if (ret)
    {
        // Read the list of recently-accessed projects.
        elem = elem->NextSiblingElement(kXmlNodeGlobalRecentProjectsRoot);
        ret &= (elem != nullptr);
        ret = ret && ExtractRecentProjects(elem, global_settings->recent_projects);
    }
    if (ret)
    {
        // Extract font family and size information.
        elem = elem->NextSiblingElement(kXmlNodeGlobalFontFamilyRoot);
        ret &= (elem != nullptr);
        ret = ret && ExtractFontInformation(elem, global_settings);
    }
    if (ret)
    {
        // Extract default include files viewer.
        // Before we move on keep the current node in case that we fail reading this node.
        auto elem_before_read = elem;
        elem                  = elem->NextSiblingElement(kXmlNodeGlobalIncludeFilesViewer);
        ret &= (elem != nullptr);
        ret = ret && RgXMLUtils::ReadNodeTextString(elem, global_settings->include_files_viewer);
        if (!ret)
        {
            // If we couldn't read it from the file, just use the default.
            global_settings->include_files_viewer = kStrGlobalSettingsSrcViewIncludeViewerDefault;

            // Roll back so that the next element read would be performed from the correct location.
            elem = elem_before_read;
            ret  = true;
        }
    }
    if (ret)
    {
        // Get the disassembly columns as a single string.
        elem = elem->NextSiblingElement(kXmlNodeGlobalDisassemblyColumns);
        ret &= (elem != nullptr);
        std::string tmp_disassembly_columns;
        ret = ret && RgXMLUtils::ReadNodeTextString(elem, tmp_disassembly_columns);

        // Parse the disassembly columns string, and save the items in the data object.
        if (ret)
        {
            ExtractDisassemblyColumns(tmp_disassembly_columns, global_settings->visible_disassembly_view_columns);
        }
    }
    if (ret)
    {
        // Read the option that determines if the project names are generated or provided by the user.
        elem = elem->NextSiblingElement(kXmlNodeUseGeneratedProjectNames);
        ret &= (elem != nullptr);
        ret = ret && RgXMLUtils::ReadNodeTextBool(elem, global_settings->use_default_project_name);
    }
    if (ret)
    {
        // Read the default API.
        elem = elem->NextSiblingElement(kXmlNodeGlobalDefaultApi);
        ret &= (elem != nullptr);
        ret = ret && RgXMLUtils::ReadNodeTextUnsigned(elem, (unsigned int&)global_settings->default_api);
    }
    if (ret)
    {
        // Read the "Should prompt for API" setting.
        elem = elem->NextSiblingElement(kXmlNodeGlobalPromptForApi);
        ret &= (elem != nullptr);
        ret = ret && RgXMLUtils::ReadNodeTextBool(elem, global_settings->should_prompt_for_api);
    }
    if (ret)
    {
        // Read the input file associations.
        elem = elem->NextSiblingElement(kXmlNodeGlobalInputFileExtGlsl);
        ret &= (elem != nullptr);
        ret = ret && RgXMLUtils::ReadNodeTextString(elem, global_settings->input_file_ext_glsl);
        if (ret)
        {
            elem = elem->NextSiblingElement(kXmlNodeGlobalInputFileExtHlsl);
            ret &= (elem != nullptr);
            ret = ret && RgXMLUtils::ReadNodeTextString(elem, global_settings->input_file_ext_hlsl);
        }
        if (ret)
        {
            elem = elem->NextSiblingElement(kXmlNodeGlobalInputFileExtSpvTxt);
            ret &= (elem != nullptr);
            ret = ret && RgXMLUtils::ReadNodeTextString(elem, global_settings->input_file_ext_spv_txt);
        }
        if (ret)
        {
            elem = elem->NextSiblingElement(kXmlNodeGlobalInputFileExtSpvBin);
            ret &= (elem != nullptr);
            ret = ret && RgXMLUtils::ReadNodeTextString(elem, global_settings->input_file_ext_spv_bin);
        }
    }
    if (ret)
    {
        // Read the default source language.
        elem = elem->NextSiblingElement(kXmlNodeGlobalDefaultSrcLang);
        ret &= (elem != nullptr);
        ret = ret && RgXMLUtils::ReadNodeTextUnsigned(elem, (uint32_t&)global_settings->default_lang);
    }
    if (ret)
    {
        // Extract splitter config objects from the "GUI" node.
        elem = elem->NextSiblingElement(kXmlNodeGlobalGui);
        ret &= (elem != nullptr);
        ret = ret && ExtractSplitterConfigs(elem, global_settings->gui_layout_splitters);
    }

    // If the Global Settings could be read, then attempt to read the Default Build Settings.
    if (ret)
    {
        tinyxml2::XMLNode* default_build_settings_node = global_settings_node->NextSibling();
        ret                                            = ExtractDefaultBuildSettings_2_1(default_build_settings_node, global_settings);
    }

    return ret;
}

static bool ExtractGlobalSettings_2_2(tinyxml2::XMLNode* global_settings_node, std::shared_ptr<RgGlobalSettings>& global_settings)
{
    // Make sure this is actually the GlobalSettings nodes.
    tinyxml2::XMLElement* elem = nullptr;
    bool                  ret  = (std::strcmp(global_settings_node->Value(), kXmlNodeGlobalLogFileGlobalSettings) == 0);

    if (ret)
    {
        // Read the log file location.
        elem = global_settings_node->FirstChildElement(kXmlNodeGlobalLogFileLocation);
        ret  = elem != nullptr;
        ret  = ret && RgXMLUtils::ReadNodeTextString(elem, global_settings->log_file_location);

        // If the saved log directory does not exist, fall back to using the default one.
        if (!RgUtils::IsDirExists(global_settings->log_file_location))
        {
            std::string app_data_dir;
            RgConfigManager::GetDefaultDataFolder(app_data_dir);
            global_settings->log_file_location = app_data_dir;
        }
    }
    if (ret)
    {
        // Attempt to read the last selected directory. It might be empty, in which case the call returns false, and that's ok.
        elem = elem->NextSiblingElement(kXmlNodeGlobalLastSelectedDirectory);
        ret &= (elem != nullptr);
        if (ret)
        {
            RgXMLUtils::ReadNodeTextString(elem, global_settings->last_selected_directory);
        }
    }
    if (ret)
    {
        // Read the list of recently-accessed projects.
        elem = elem->NextSiblingElement(kXmlNodeGlobalRecentProjectsRoot);
        ret &= (elem != nullptr);
        ret = ret && ExtractRecentProjects(elem, global_settings->recent_projects);
    }
    if (ret)
    {
        // Extract font family and size information.
        elem = elem->NextSiblingElement(kXmlNodeGlobalFontFamilyRoot);
        ret &= (elem != nullptr);
        ret = ret && ExtractFontInformation(elem, global_settings);
    }
    if (ret)
    {
        // Extract default include files viewer.
        // Before we move on keep the current node in case that we fail reading this node.
        auto elem_before_read = elem;
        elem                  = elem->NextSiblingElement(kXmlNodeGlobalIncludeFilesViewer);
        ret &= (elem != nullptr);
        ret = ret && RgXMLUtils::ReadNodeTextString(elem, global_settings->include_files_viewer);
        if (!ret)
        {
            // If we couldn't read it from the file, just use the default.
            global_settings->include_files_viewer = kStrGlobalSettingsSrcViewIncludeViewerDefault;

            // Roll back so that the next element read would be performed from the correct location.
            elem = elem_before_read;
            ret  = true;
        }
    }
    if (ret)
    {
        // Get the disassembly columns as a single string.
        elem = elem->NextSiblingElement(kXmlNodeGlobalDisassemblyColumns);
        ret &= (elem != nullptr);
        std::string tmp_disassembly_columns;
        ret = ret && RgXMLUtils::ReadNodeTextString(elem, tmp_disassembly_columns);

        // Parse the disassembly columns string, and save the items in the data object.
        if (ret)
        {
            ExtractDisassemblyColumns(tmp_disassembly_columns, global_settings->visible_disassembly_view_columns);

            // Update the columns to have the new VGPR pressure column as well if it is missing.
            if (global_settings->visible_disassembly_view_columns.size() == static_cast<int>(RgIsaDisassemblyTableColumns::kCount) - 1)
            {
                global_settings->visible_disassembly_view_columns.push_back("true");
            }
        }
    }
    if (ret)
    {
        // Read the option that determines if the project names are generated or provided by the user.
        elem = elem->NextSiblingElement(kXmlNodeUseGeneratedProjectNames);
        ret &= (elem != nullptr);
        ret = ret && RgXMLUtils::ReadNodeTextBool(elem, global_settings->use_default_project_name);
    }
    if (ret)
    {
        // Read the default API.
        elem = elem->NextSiblingElement(kXmlNodeGlobalDefaultApi);
        ret &= (elem != nullptr);
        ret = ret && RgXMLUtils::ReadNodeTextUnsigned(elem, (unsigned int&)global_settings->default_api);
    }
    if (ret)
    {
        // Read the "Should prompt for API" setting.
        elem = elem->NextSiblingElement(kXmlNodeGlobalPromptForApi);
        ret &= (elem != nullptr);
        ret = ret && RgXMLUtils::ReadNodeTextBool(elem, global_settings->should_prompt_for_api);
    }
    if (ret)
    {
        // Read the input file associations.
        elem = elem->NextSiblingElement(kXmlNodeGlobalInputFileExtGlsl);
        ret &= (elem != nullptr);
        ret = ret && RgXMLUtils::ReadNodeTextString(elem, global_settings->input_file_ext_glsl);
        if (ret)
        {
            elem = elem->NextSiblingElement(kXmlNodeGlobalInputFileExtHlsl);
            ret &= (elem != nullptr);
            ret = ret && RgXMLUtils::ReadNodeTextString(elem, global_settings->input_file_ext_hlsl);
        }
        if (ret)
        {
            elem = elem->NextSiblingElement(kXmlNodeGlobalInputFileExtSpvTxt);
            ret &= (elem != nullptr);
            ret = ret && RgXMLUtils::ReadNodeTextString(elem, global_settings->input_file_ext_spv_txt);
        }
        if (ret)
        {
            elem = elem->NextSiblingElement(kXmlNodeGlobalInputFileExtSpvBin);
            ret &= (elem != nullptr);
            ret = ret && RgXMLUtils::ReadNodeTextString(elem, global_settings->input_file_ext_spv_bin);
        }
    }
    if (ret)
    {
        // Read the default source language.
        elem = elem->NextSiblingElement(kXmlNodeGlobalDefaultSrcLang);
        ret &= (elem != nullptr);
        ret = ret && RgXMLUtils::ReadNodeTextUnsigned(elem, (uint32_t&)global_settings->default_lang);
    }
    if (ret)
    {
        // Extract splitter config objects from the "GUI" node.
        elem = elem->NextSiblingElement(kXmlNodeGlobalGui);
        ret &= (elem != nullptr);
        ret = ret && ExtractSplitterConfigs(elem, global_settings->gui_layout_splitters);
    }
    if (ret)
    {
        // Extract window size config objects from the "GUI" node.
        ret = ret && ExtractWindowGeometry(elem, global_settings->gui_window_config);
    }

    // If the Global Settings could be read, then attempt to read the Default Build Settings.
    if (ret)
    {
        tinyxml2::XMLNode* default_build_settings_node = global_settings_node->NextSibling();
        ret                                            = ExtractDefaultBuildSettings_2_1(default_build_settings_node, global_settings);
    }

    return ret;
}

static bool ExtractGlobalSettings_2_3(tinyxml2::XMLNode* global_settings_node, std::shared_ptr<RgGlobalSettings>& global_settings)
{
    // Make sure this is actually the GlobalSettings nodes.
    tinyxml2::XMLElement* elem = nullptr;
    bool                  ret  = (std::strcmp(global_settings_node->Value(), kXmlNodeGlobalLogFileGlobalSettings) == 0);

    if (ret)
    {
        // Read the log file location.
        elem = global_settings_node->FirstChildElement(kXmlNodeGlobalLogFileLocation);
        ret  = elem != nullptr;
        ret  = ret && RgXMLUtils::ReadNodeTextString(elem, global_settings->log_file_location);

        // If the saved log directory does not exist, fall back to using the default one.
        if (!RgUtils::IsDirExists(global_settings->log_file_location))
        {
            std::string app_data_dir;
            RgConfigManager::GetDefaultDataFolder(app_data_dir);
            global_settings->log_file_location = app_data_dir;
        }
    }
    if (ret)
    {
        // Read the project file location.
        elem = global_settings_node->FirstChildElement(kXmlNodeGlobalProjectFileLocation);
        ret  = elem != nullptr;
        ret  = ret && RgXMLUtils::ReadNodeTextString(elem, global_settings->project_file_location);

        // If the saved project directory does not exist, fall back to using the default one.
        if (!RgUtils::IsDirExists(global_settings->project_file_location))
        {
            std::string documents_dir;
            RgConfigManager::GetDefaultDocumentsFolder(documents_dir);
            global_settings->project_file_location = documents_dir;
        }
    }
    if (ret)
    {
        // Attempt to read the last selected directory. It might be empty, in which case the call returns false, and that's ok.
        elem = elem->NextSiblingElement(kXmlNodeGlobalLastSelectedDirectory);
        ret &= (elem != nullptr);
        if (ret)
        {
            RgXMLUtils::ReadNodeTextString(elem, global_settings->last_selected_directory);
        }
    }
    if (ret)
    {
        // Read the list of recently-accessed projects.
        elem = elem->NextSiblingElement(kXmlNodeGlobalRecentProjectsRoot);
        ret &= (elem != nullptr);
        ret = ret && ExtractRecentProjects(elem, global_settings->recent_projects);
    }
    if (ret)
    {
        // Extract font family and size information.
        elem = elem->NextSiblingElement(kXmlNodeGlobalFontFamilyRoot);
        ret &= (elem != nullptr);
        ret = ret && ExtractFontInformation(elem, global_settings);
    }
    if (ret)
    {
        // Extract default include files viewer.
        // Before we move on keep the current node in case that we fail reading this node.
        auto elem_before_read = elem;
        elem                  = elem->NextSiblingElement(kXmlNodeGlobalIncludeFilesViewer);
        ret &= (elem != nullptr);
        ret = ret && RgXMLUtils::ReadNodeTextString(elem, global_settings->include_files_viewer);
        if (!ret)
        {
            // If we couldn't read it from the file, just use the default.
            global_settings->include_files_viewer = kStrGlobalSettingsSrcViewIncludeViewerDefault;

            // Roll back so that the next element read would be performed from the correct location.
            elem = elem_before_read;
            ret  = true;
        }
    }
    if (ret)
    {
        // Get the disassembly columns as a single string.
        elem = elem->NextSiblingElement(kXmlNodeGlobalDisassemblyColumns);
        ret &= (elem != nullptr);
        std::string tmp_disassembly_columns;
        ret = ret && RgXMLUtils::ReadNodeTextString(elem, tmp_disassembly_columns);

        // Parse the disassembly columns string, and save the items in the data object.
        if (ret)
        {
            ExtractDisassemblyColumns(tmp_disassembly_columns, global_settings->visible_disassembly_view_columns);

            // Update the columns to have the new VGPR pressure column as well if it is missing.
            if (global_settings->visible_disassembly_view_columns.size() == static_cast<int>(RgIsaDisassemblyTableColumns::kCount) - 1)
            {
                global_settings->visible_disassembly_view_columns.push_back("true");
            }
        }
    }
    if (ret)
    {
        // Read the option that determines if the project names are generated or provided by the user.
        elem = elem->NextSiblingElement(kXmlNodeUseGeneratedProjectNames);
        ret &= (elem != nullptr);
        ret = ret && RgXMLUtils::ReadNodeTextBool(elem, global_settings->use_default_project_name);
    }
    if (ret)
    {
        // Read the default API.
        elem = elem->NextSiblingElement(kXmlNodeGlobalDefaultApi);
        ret &= (elem != nullptr);
        ret = ret && RgXMLUtils::ReadNodeTextUnsigned(elem, (unsigned int&)global_settings->default_api);
    }
    if (ret)
    {
        // Read the "Should prompt for API" setting.
        elem = elem->NextSiblingElement(kXmlNodeGlobalPromptForApi);
        ret &= (elem != nullptr);
        ret = ret && RgXMLUtils::ReadNodeTextBool(elem, global_settings->should_prompt_for_api);
    }
    if (ret)
    {
        // Read the input file associations.
        elem = elem->NextSiblingElement(kXmlNodeGlobalInputFileExtGlsl);
        ret &= (elem != nullptr);
        ret = ret && RgXMLUtils::ReadNodeTextString(elem, global_settings->input_file_ext_glsl);
        if (ret)
        {
            elem = elem->NextSiblingElement(kXmlNodeGlobalInputFileExtHlsl);
            ret &= (elem != nullptr);
            ret = ret && RgXMLUtils::ReadNodeTextString(elem, global_settings->input_file_ext_hlsl);
        }
        if (ret)
        {
            elem = elem->NextSiblingElement(kXmlNodeGlobalInputFileExtSpvTxt);
            ret &= (elem != nullptr);
            ret = ret && RgXMLUtils::ReadNodeTextString(elem, global_settings->input_file_ext_spv_txt);
        }
        if (ret)
        {
            elem = elem->NextSiblingElement(kXmlNodeGlobalInputFileExtSpvBin);
            ret &= (elem != nullptr);
            ret = ret && RgXMLUtils::ReadNodeTextString(elem, global_settings->input_file_ext_spv_bin);
        }
    }
    if (ret)
    {
        // Read the default source language.
        elem = elem->NextSiblingElement(kXmlNodeGlobalDefaultSrcLang);
        ret &= (elem != nullptr);
        ret = ret && RgXMLUtils::ReadNodeTextUnsigned(elem, (uint32_t&)global_settings->default_lang);
    }
    if (ret)
    {
        // Extract splitter config objects from the "GUI" node.
        elem = elem->NextSiblingElement(kXmlNodeGlobalGui);
        ret &= (elem != nullptr);
        ret = ret && ExtractSplitterConfigs(elem, global_settings->gui_layout_splitters);
    }
    if (ret)
    {
        // Extract window size config objects from the "GUI" node.
        ret = ret && ExtractWindowGeometry(elem, global_settings->gui_window_config);
    }

    // If the Global Settings could be read, then attempt to read the Default Build Settings.
    if (ret)
    {
        tinyxml2::XMLNode* default_build_settings_node = global_settings_node->NextSibling();
        ret                                            = ExtractDefaultBuildSettings_2_1(default_build_settings_node, global_settings);
    }

    return ret;
}
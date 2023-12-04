// C++>
#include <cassert>
#include <sstream>

// Infra.
#include <tinyxml2/Include/tinyxml2.h>

// Local.
#include "radeon_gpu_analyzer_gui/rg_config_file_definitions.h"
#include "radeon_gpu_analyzer_gui/rg_config_file_binary.h"
#include "radeon_gpu_analyzer_gui/rg_xml_utils.h"
#include "radeon_gpu_analyzer_gui/rg_utils.h"

bool RgConfigFileReaderBinary::ReadProjectConfigFile(tinyxml2::XMLDocument& doc, const char* file_data_model_version, std::shared_ptr<RgProject>& rga_project)
{
    // Version 2.2 requires processing of binary output file, which
   // is handled later.
    bool is_version_compatible =
        (kRgaDataModel2_0.compare(file_data_model_version) == 0) ||
        (kRgaDataModel2_1.compare(file_data_model_version) == 0) ||
        (kRgaDataModel2_2.compare(file_data_model_version) == 0) || 
        (kRgaDataModel2_3.compare(file_data_model_version) == 0) ||
        (kRgaDataModel2_4.compare(file_data_model_version) == 0);

    assert(is_version_compatible);

    bool ret = false;
    rga_project = nullptr;

    if (is_version_compatible)
    {
        // Find the project node.
        tinyxml2::XMLNode* node = doc.FirstChildElement(kXmlNodeProject);
        if (node != nullptr)
        {
            node = node->FirstChild();

            // Verify that this is an OpenCL API config file.
            std::string api_name;
            ret = RgXMLUtils::ReadNodeTextString(node, api_name);
            if (ret && (api_name.compare(kStrApiAbbreviationBinary) == 0) && node != nullptr)
            {
                // Go to the project name node.
                node = node->NextSibling();

                // Get the project name.
                std::string project_name;
                ret = RgXMLUtils::ReadNodeTextString(node, project_name);
                if (!project_name.empty() && node != nullptr)
                {
                    // Create the RGA project object.
                    std::shared_ptr<RgProjectBinary> binary_project = std::make_shared<RgProjectBinary>();
                    binary_project->project_name                    = project_name;
                    binary_project->project_data_model_version      = file_data_model_version;
                    rga_project                                     = binary_project;

                    // Iterate through the project's clones: get the first clone.
                    node = node->NextSibling();
                    tinyxml2::XMLNode* clone_root = node;

                    while (clone_root != nullptr)
                    {
                        // Get the clone ID.
                        node = clone_root->FirstChildElement(kXmlNodeCloneId);
                        std::shared_ptr<RgProjectClone> clone = std::make_shared<RgProjectClone>();
                        ret = RgXMLUtils::ReadNodeTextUnsigned(node, clone->clone_id);
                        if (ret && node != nullptr)
                        {
                            // Get the name of the clone.
                            node = node->NextSibling();

                            ret = RgXMLUtils::ReadNodeTextString(node, clone->clone_name);
                            if (ret)
                            {
                                //// Get the files in this clone.
                                //node = node->NextSibling();
                                //tinyxml2::XMLElement* file_node = node->FirstChildElement();
                                //while (file_node != nullptr)
                                //{
                                //    // Extract the full path of the current file.
                                //    std::string source_file_info_string;
                                //    ret = RgXMLUtils::ReadNodeTextString(file_node, source_file_info_string);
                                //    assert(ret);
                                //    if (ret && !source_file_info_string.empty())
                                //    {
                                //        RgSourceFileInfo new_source_file = {};
                                //        new_source_file.file_path = source_file_info_string;

                                //        // Add the file path to our clone object.
                                //        clone->source_files.push_back(new_source_file);
                                //    }

                                //    // Go to the next file.
                                //    file_node = file_node->NextSiblingElement();
                                //}

                                // Get the Binary build settings.
                                node = node->NextSiblingElement(kXmlNodeBuildSettings);
                                std::shared_ptr<RgBuildSettingsBinary> build_settings = std::make_shared<RgBuildSettingsBinary>();
                                assert(build_settings != nullptr);
                                if (build_settings != nullptr)
                                {
                                    // Read the general build settings that aren't specific to a single API.
                                    ret = ret && ReadGeneralBuildSettings(node, build_settings);
                                    assert(ret);
                                    if (ret)
                                    {
                                        // Read the build settings that apply only to API.
                                        ret = ret && ReadApiBuildSettings(node, build_settings, binary_project->project_data_model_version);
                                        assert(ret);
                                        if (ret)
                                        {
                                            // Add the build settings to the project clone.
                                            clone->build_settings = build_settings;

                                            // We are done, add this clone to the project object.
                                            binary_project->clones.push_back(clone);
                                        }
                                    }
                                }
                            }
                        }

                        // Go to the next clone element.
                        clone_root = clone_root->NextSibling();
                    }
                }
            }
        }
    }

    return ret;
}

bool RgConfigFileReaderBinary::ReadApiBuildSettings(tinyxml2::XMLNode* node, std::shared_ptr<RgBuildSettings> build_settings, const std::string& version)
{
    bool ret = false;

    const std::shared_ptr<RgBuildSettingsBinary> build_settings_binary = std::dynamic_pointer_cast<RgBuildSettingsBinary>(build_settings);

    if (build_settings_binary != nullptr)
    {
        ret = true;    
    }

    if (ret)
    {   
        // Binary file name.
        assert(node != nullptr);
        if (node != nullptr)
        {
            // Binary output file name.
            node = node->FirstChildElement(kXmlNodeGlobalBinaryInputFileName);
            ret  = (node != nullptr);
            assert(node != nullptr);
            std::string binary_file_name;
            bool        should_read = RgXMLUtils::ReadNodeTextString(node, binary_file_name);
            if (should_read)
            {
                build_settings_binary->binary_file_name = binary_file_name;
            }
        }
    }

    return ret;
}

bool RgConfigFileWriterBinary::WriteProjectConfigFile(const RgProject& project, const std::string& config_file_path)
{
    bool ret = false;

    // Create the XML declaration node.
    tinyxml2::XMLDocument doc;
    AddConfigFileDeclaration(doc);

    // Create the Project element.
    tinyxml2::XMLElement* project_ptr = doc.NewElement(kXmlNodeProject);
    tinyxml2::XMLElement* api = doc.NewElement(kXmlNodeApiName);
    std::string api_name;
    ret = RgUtils::ProjectAPIToString(project.api, api_name);
    if (ret)
    {
        // API name.
        api->SetText(api_name.c_str());
        project_ptr->InsertFirstChild(api);

        // Project name.
        tinyxml2::XMLElement* project_name = doc.NewElement(kXmlNodeProjectName);
        project_name->SetText(project.project_name.c_str());
        project_ptr->InsertEndChild(project_name);

        // Handle the project's clones.
        std::vector<tinyxml2::XMLElement*> clone_elems;
        const RgProjectBinary& bin_project = static_cast<const RgProjectBinary&>(project);
        WriteBinaryCloneElements(bin_project, doc, clone_elems);
        for (tinyxml2::XMLElement* clone_elem : clone_elems)
        {
            project_ptr->LinkEndChild(clone_elem);
        }

        // Add the project node.
        doc.InsertEndChild(project_ptr);

        // Save the file.
        tinyxml2::XMLError rc = doc.SaveFile(config_file_path.c_str());
        ret = (rc == tinyxml2::XML_SUCCESS);
        assert(ret);
    }

    return ret;
}

bool RgConfigFileWriterBinary::WriteBuildSettingsElement(const std::shared_ptr<RgBuildSettings> build_settings, tinyxml2::XMLDocument& doc, tinyxml2::XMLElement*& build_settings_elem)
{
    bool ret = false;

    assert(build_settings != nullptr);
    if (build_settings != nullptr)
    {
        const std::shared_ptr<RgBuildSettingsBinary> build_settings_binary = std::dynamic_pointer_cast<RgBuildSettingsBinary>(build_settings);
        assert(build_settings_binary != nullptr);

        if (build_settings_elem != nullptr && build_settings_binary != nullptr)
        {
            // Write API-agnostic build settings.
            WriteGeneralBuildSettings(build_settings_binary, doc, build_settings_elem);

            // Binary output file name.
            RgXMLUtils::AppendXMLElement(doc, build_settings_elem, kXmlNodeGlobalBinaryInputFileName, build_settings_binary->binary_file_name.c_str());

            // Add the Build Settings element its parent.
            doc.InsertEndChild(build_settings_elem);

            ret = true;
        }
    }

    return ret;
}

bool RgConfigFileWriterBinary::WriteBinaryCloneElements(const RgProjectBinary& project, tinyxml2::XMLDocument& doc, std::vector<tinyxml2::XMLElement*>& elems)
{
    bool ret = false;

    for (const std::shared_ptr<RgProjectClone>& clone : project.clones)
    {
        if (clone != nullptr)
        {
            // Project clone.
            tinyxml2::XMLElement* clone_element = doc.NewElement(kXmlNodeClone);

            // Clone ID.
            tinyxml2::XMLElement* clone_id = doc.NewElement(kXmlNodeCloneId);
            clone_id->SetText(clone->clone_id);
            clone_element->LinkEndChild(clone_id);

            // Clone name.
            tinyxml2::XMLElement* clone_name = doc.NewElement(kXmlNodeCloneName);
            clone_name->SetText(clone->clone_name.c_str());
            clone_element->LinkEndChild(clone_name);

            //// Source files.
            //tinyxml2::XMLElement* clone_source_files = doc.NewElement(kXmlNodeCloneSourceFiles);

            //// Go through each and every source file, and create its element.
            //for (const RgSourceFileInfo& source_file_info : clone->source_files)
            //{
            //    // Create the file element.
            //    tinyxml2::XMLElement* file_path = doc.NewElement(kXmlNodeFilePath);

            //    std::stringstream file_status_stream;
            //    file_status_stream << source_file_info.file_path;

            //    file_path->SetText(file_status_stream.str().c_str());

            //    // Attach the file element to the Source Files node.
            //    clone_source_files->LinkEndChild(file_path);
            //}


            //// Add the Source Files node to the Clone element.
            //clone_element->LinkEndChild(clone_source_files);

            // Build settings.
            tinyxml2::XMLElement* build_settings = doc.NewElement(kXmlNodeBuildSettings);
            ret = WriteBuildSettingsElement(clone->build_settings, doc, build_settings);

            if (ret)
            {
                clone_element->LinkEndChild(build_settings);
                elems.push_back(clone_element);
                ret = true;
            }
        }
    }

    return ret;
}

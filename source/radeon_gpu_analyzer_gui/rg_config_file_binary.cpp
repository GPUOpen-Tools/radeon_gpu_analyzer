//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for XML writer for Binary mode config files.
//=============================================================================
// C++>
#include <cassert>
#include <sstream>

// Infra.
#include "tinyxml2.h"

// Local.
#include "radeon_gpu_analyzer_gui/rg_config_file_definitions.h"
#include "radeon_gpu_analyzer_gui/rg_config_file_binary.h"
#include "radeon_gpu_analyzer_gui/rg_xml_utils.h"
#include "radeon_gpu_analyzer_gui/rg_utils.h"

bool RgConfigFileReaderBinary::ReadProjectConfigFile(tinyxml2::XMLDocument& doc, const char* file_data_model_version, std::shared_ptr<RgProject>& rga_project)
{
    // Version 2.2 requires processing of binary output file, which
    // is handled later.
    bool is_version_compatible = (kRgaDataModel2_0.compare(file_data_model_version) == 0) || (kRgaDataModel2_1.compare(file_data_model_version) == 0) ||
                                 (kRgaDataModel2_2.compare(file_data_model_version) == 0) || (kRgaDataModel2_3.compare(file_data_model_version) == 0) ||
                                 (kRgaDataModel2_4.compare(file_data_model_version) == 0) || (kRgaDataModel2_5.compare(file_data_model_version) == 0);

    assert(is_version_compatible);

    bool ret    = false;
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
                    node                          = node->NextSibling();
                    tinyxml2::XMLNode* clone_root = node;

                    while (clone_root != nullptr)
                    {
                        // Get the clone ID.
                        node                                  = clone_root->FirstChildElement(kXmlNodeCloneId);
                        std::shared_ptr<RgProjectClone> clone = std::make_shared<RgProjectClone>();
                        bool clone_ok                         = RgXMLUtils::ReadNodeTextUnsigned(node, clone->clone_id) && node != nullptr;
                        if (clone_ok)
                        {
                            // Get the name of the clone.
                            node = node->NextSibling();

                            clone_ok = RgXMLUtils::ReadNodeTextString(node, clone->clone_name);
                            if (clone_ok)
                            {
                                // Look up the clone's children by name: pre-2.5 projects have no BinaryInputFiles node.
                                tinyxml2::XMLElement* binary_files_root   = clone_root->FirstChildElement(kXmlNodeCloneBinaryFiles);
                                tinyxml2::XMLElement* build_settings_node = clone_root->FirstChildElement(kXmlNodeBuildSettings);

                                tinyxml2::XMLElement* binary_input_file_elem =
                                    (binary_files_root != nullptr) ? binary_files_root->FirstChildElement(kXmlNodeCloneBinaryFile) : nullptr;
                                while (binary_input_file_elem != nullptr)
                                {
                                    RgBinaryFileInfo new_binary_file = {};

                                    std::string file_path;
                                    if (auto* full_path_elem = binary_input_file_elem->FirstChildElement(kXmlNodeFilePath))
                                    {
                                        if (RgXMLUtils::ReadNodeTextString(full_path_elem, file_path) && !file_path.empty())
                                        {
                                            new_binary_file.file_path = file_path;
                                        }
                                    }

                                    if (auto* disasm_elem = binary_input_file_elem->FirstChildElement(kXmlNodeIsDisassemblyGenerated))
                                    {
                                        bool is_generated = false;
                                        if (RgXMLUtils::ReadNodeTextBool(disasm_elem, is_generated))
                                        {
                                            new_binary_file.is_disassembly_generated = is_generated;
                                        }
                                    }

                                    std::string target_gpu;
                                    if (auto* target_gpu_elem = binary_input_file_elem->FirstChildElement(kXmlNodeCloneBinaryTargetGpu))
                                    {
                                        if (RgXMLUtils::ReadNodeTextString(target_gpu_elem, target_gpu) && !target_gpu.empty())
                                        {
                                            new_binary_file.target_gpu = target_gpu;
                                        }
                                    }

                                    // Add to clone object.
                                    clone->binary_files.push_back(new_binary_file);

                                    // Move to the next BinaryInputFile.
                                    binary_input_file_elem = binary_input_file_elem->NextSiblingElement(kXmlNodeCloneBinaryFile);
                                }

                                // Data models 2.0-2.4 stored the code objects under BuildSettings. Migrate them.
                                if (clone->binary_files.empty() && build_settings_node != nullptr)
                                {
                                    for (tinyxml2::XMLElement* legacy_file_elem = build_settings_node->FirstChildElement(kXmlNodeLegacyBinaryInputFileName);
                                         legacy_file_elem != nullptr;
                                         legacy_file_elem = legacy_file_elem->NextSiblingElement(kXmlNodeLegacyBinaryInputFileName))
                                    {
                                        std::string legacy_file_path;
                                        if (RgXMLUtils::ReadNodeTextString(legacy_file_elem, legacy_file_path) && !legacy_file_path.empty())
                                        {
                                            RgBinaryFileInfo legacy_binary_file = {};
                                            legacy_binary_file.file_path        = legacy_file_path;
                                            clone->binary_files.push_back(legacy_binary_file);
                                        }
                                    }
                                }

                                // Get the Binary build settings.
                                node                                                  = build_settings_node;
                                std::shared_ptr<RgBuildSettingsBinary> build_settings = std::make_shared<RgBuildSettingsBinary>();
                                clone_ok                                              = (build_settings != nullptr);
                                assert(clone_ok);
                                if (clone_ok)
                                {
                                    // Read the general build settings that aren't specific to a single API.
                                    clone_ok = ReadGeneralBuildSettings(node, build_settings);
                                    assert(clone_ok);
                                    if (clone_ok)
                                    {
                                        // Read the build settings that apply only to API.
                                        clone_ok = ReadApiBuildSettings(node, build_settings, binary_project->project_data_model_version);
                                        assert(clone_ok);
                                        if (clone_ok)
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

                        // A single bad clone invalidates the whole project.
                        ret = ret && clone_ok;

                        // Go to the next clone element.
                        clone_root = clone_root->NextSibling();
                    }
                }
            }
        }
    }

    // Never hand back a partially-parsed project: callers only null-check it, and the GUI indexes clones[0].
    if (!ret || (rga_project != nullptr && rga_project->clones.empty()))
    {
        rga_project = nullptr;
        ret         = false;
    }

    return ret;
}

bool RgConfigFileReaderBinary::ReadApiBuildSettings(tinyxml2::XMLNode* node, std::shared_ptr<RgBuildSettings> build_settings, const std::string& version)
{
    bool ret = false;

    const std::shared_ptr<RgBuildSettingsBinary> build_settings_binary = std::dynamic_pointer_cast<RgBuildSettingsBinary>(build_settings);

    if (node != nullptr)
    {
        if (kRgaDataModel2_5.compare(version) == 0)
        {
            // Prompt to attach source (new in 2.5).
            node = node->FirstChildElement(kXmlNodeBinaryPromptToAttachSrc);
            ret  = (node != nullptr);
            assert(ret);
            if (node != nullptr)
            {
                ret = RgXMLUtils::ReadNodeTextBool(node, build_settings_binary->prompt_to_attach_source_dirs);
            }
        }
        else
        {
            // Projects saved before 2.5 don't have this setting; keep the default value.
            ret = true;
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
    ret = RgUtils::ProjectAPIToString(project.api, api_name, true);
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
        const RgProjectBinary&             bin_project = static_cast<const RgProjectBinary&>(project);
        WriteBinaryCloneElements(bin_project, doc, clone_elems);
        for (tinyxml2::XMLElement* clone_elem : clone_elems)
        {
            project_ptr->LinkEndChild(clone_elem);
        }

        // Add the project node.
        doc.InsertEndChild(project_ptr);

        // Save the file.
        tinyxml2::XMLError rc = doc.SaveFile(config_file_path.c_str());
        ret                   = (rc == tinyxml2::XML_SUCCESS);
        assert(ret);
    }

    return ret;
}

bool RgConfigFileWriterBinary::WriteBuildSettingsElement(const std::shared_ptr<RgBuildSettings> build_settings,
                                                         tinyxml2::XMLDocument&                 doc,
                                                         tinyxml2::XMLElement*&                 build_settings_elem)
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

            // Prompt to attach source.
            RgXMLUtils::AppendXMLElement(doc, build_settings_elem, kXmlNodeBinaryPromptToAttachSrc, build_settings_binary->prompt_to_attach_source_dirs);

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

            // Binary Code object files.
            tinyxml2::XMLElement* clone_binary_files = doc.NewElement(kXmlNodeCloneBinaryFiles);

            // Go through each and every source file, and create its element.
            for (const auto& source_file_info : clone->binary_files)
            {
                tinyxml2::XMLElement* clone_binary_file = doc.NewElement(kXmlNodeCloneBinaryFile);

                if (clone_binary_file != nullptr)
                {
                    // Create the file element.
                    tinyxml2::XMLElement* file_path = doc.NewElement(kXmlNodeFilePath);
                    std::stringstream file_status_stream;
                    file_status_stream << source_file_info.file_path;
                    file_path->SetText(file_status_stream.str().c_str());
                    // Attach the file element to the Binary File node.
                    clone_binary_file->LinkEndChild(file_path);

                    // Flag for is the current code object is analyzed.
                    tinyxml2::XMLElement* is_disassembly_generated = doc.NewElement(kXmlNodeIsDisassemblyGenerated);
                    is_disassembly_generated->SetText(source_file_info.is_disassembly_generated ? "true" : "false");
                    clone_binary_file->LinkEndChild(is_disassembly_generated);

                    // The current code object's target gpu.
                    tinyxml2::XMLElement* target_gpu = doc.NewElement(kXmlNodeCloneBinaryTargetGpu);
                    std::stringstream     target_gpu_stream;
                    target_gpu_stream << source_file_info.target_gpu;
                    target_gpu->SetText(target_gpu_stream.str().c_str());
                    clone_binary_file->LinkEndChild(target_gpu);

                    clone_binary_files->LinkEndChild(clone_binary_file);
                }
            }

            // Add the Coede object files Files node to the Clone element.
            clone_element->LinkEndChild(clone_binary_files);

            // Build settings.
            tinyxml2::XMLElement* build_settings = doc.NewElement(kXmlNodeBuildSettings);
            ret                                  = WriteBuildSettingsElement(clone->build_settings, doc, build_settings);

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

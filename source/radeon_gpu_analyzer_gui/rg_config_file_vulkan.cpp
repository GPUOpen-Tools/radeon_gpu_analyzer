// C++.
#include <cassert>
#include <memory>
#include <algorithm>

// Infra.
#include <tinyxml2/Include/tinyxml2.h>

// Local.
#include "radeon_gpu_analyzer_gui/rg_config_file_definitions.h"
#include "radeon_gpu_analyzer_gui/rg_config_file_vulkan.h"
#include "radeon_gpu_analyzer_gui/rg_xml_utils.h"
#include "radeon_gpu_analyzer_gui/rg_utils.h"

bool RgConfigFileReaderVulkan::ReadProjectClone(tinyxml2::XMLDocument& doc, tinyxml2::XMLNode* clones_root, std::shared_ptr<RgProjectVulkan>& vulkan_project)
{
    // Get the clone ID.
    tinyxml2::XMLNode* node = clones_root->FirstChildElement(kXmlNodeCloneId);
    std::shared_ptr<RgProjectCloneVulkan> clone = std::make_shared<RgProjectCloneVulkan>();
    bool ret = RgXMLUtils::ReadNodeTextUnsigned(node, clone->clone_id);
    if (ret && node != nullptr)
    {
        // Get the name of the clone.
        node = node->NextSibling();
        ret = RgXMLUtils::ReadNodeTextString(node, clone->clone_name);
    }

    // Read the pipeline object.
    ret = ret && ReadPipeline(doc, clones_root, false , clone->pipeline);
    assert(ret);

    // Read the backup SPIR-V binary paths (optional).
    ret = ret && ReadPipeline(doc, clones_root, true, clone->spv_backup_);
    assert(ret);

    if (ret)
    {
        // Create a Vulkan build settings object.
        std::shared_ptr<RgBuildSettingsVulkan> build_settings = std::make_shared<RgBuildSettingsVulkan>();
        assert(build_settings != nullptr);
        if ((ret = build_settings != nullptr) == true)
        {
            // Read the pipeline state.
            node = node->NextSiblingElement(kXmlNodePipelineStateRoot);
            assert(node != nullptr);
            ret = (node != nullptr);
        }

        ret = ret && ReadPipelineState(clone, doc, node);
        assert(ret);

        if (ret)
        {
            // Get the Vulkan build settings.
            node = node->NextSiblingElement(kXmlNodeBuildSettings);
            assert(node != nullptr);
            ret = node != nullptr;
        }

        // Read the general build settings that aren't specific to a single API.
        ret = ret && ReadGeneralBuildSettings(node, build_settings);
        assert(ret);

        // Read the build settings that apply only to Vulkan.
        ret = ret && ReadApiBuildSettings(node, build_settings, vulkan_project->project_data_model_version);
        assert(ret);

        if (ret)
        {
            // Add the build settings to the project clone.
            clone->build_settings = build_settings;

            // We are done, add this clone to the project object.
            vulkan_project->clones.push_back(clone);
        }
    }

    return ret;
}

bool RgConfigFileReaderVulkan::ReadProjectConfigFile(tinyxml2::XMLDocument& doc, const char* file_data_model_version, std::shared_ptr<RgProject>& rga_project)
{
    // The config files are the same format for v2.0 and v2.1.
    // Version 2.2 requires processing of binary output file, which
    // is handled later.
    bool is_version_compatible = (kRgaDataModel2_0.compare(file_data_model_version) == 0) || 
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

            // Verify that this is an Vulkan API config file.
            std::string api_name;
            ret = RgXMLUtils::ReadNodeTextString(node, api_name);
            if (ret && (api_name.compare(kStrApiNameVulkan) == 0) && node != nullptr)
            {
                // Go to the project name node.
                node = node->NextSibling();

                // Get the project name.
                std::string project_name;
                ret = RgXMLUtils::ReadNodeTextString(node, project_name);
                if (!project_name.empty() && node != nullptr)
                {
                    // Create the RGA project object.
                    std::shared_ptr<RgProjectVulkan> vulkan_project = std::make_shared<RgProjectVulkan>();
                    vulkan_project->project_name = project_name;
                    vulkan_project->project_data_model_version = file_data_model_version;
                    rga_project = vulkan_project;

                    // Iterate through the project's clones: get the first clone.
                    node = node->NextSibling();
                    tinyxml2::XMLNode* clone_root = node;

                    while (ret && clone_root != nullptr)
                    {
                        // Read the project clone data.
                        ret = ReadProjectClone(doc, clone_root, vulkan_project);

                        // Go to the next clone element.
                        clone_root = clone_root->NextSibling();
                    }
                }
            }
        }
    }

    return ret;
}

bool RgConfigFileReaderVulkan::ReadApiBuildSettings(tinyxml2::XMLNode* node, std::shared_ptr<RgBuildSettings> build_settings, const std::string& version)
{
    bool ret = false;

    assert(build_settings != nullptr);
    if (build_settings != nullptr)
    {
        const std::shared_ptr<RgBuildSettingsVulkan> build_settings_vulkan = std::dynamic_pointer_cast<RgBuildSettingsVulkan>(build_settings);
        assert(build_settings_vulkan != nullptr);
        if (build_settings_vulkan != nullptr)
        {
            if ((kRgaDataModel2_2.compare(version) == 0) || 
                (kRgaDataModel2_3.compare(version) == 0) || 
                (kRgaDataModel2_4.compare(version) == 0))
            {
                assert(node != nullptr);
                if (node != nullptr)
                {
                    // Binary output file name.
                    node = node->FirstChildElement(kXmlNodeGlobalBinaryOutputFileName);
                    ret = (node != nullptr);
                    assert(node != nullptr);
                    std::string binary_file_name;
                    bool should_read = RgXMLUtils::ReadNodeTextString(node, binary_file_name);
                    if (should_read)
                    {
                        build_settings_vulkan->binary_file_name = binary_file_name;
                    }
                }

                assert(node != nullptr);
                if (ret && node != nullptr)
                {
                    // Generate Debug Info.
                    node = node->NextSiblingElement(kXmlNodeVulkanGenerateDebugInfo);
                    ret = RgXMLUtils::ReadNodeTextBool(node, build_settings_vulkan->is_generate_debug_info_checked);
                }
            }
            else
            {
                // Use the default binary output file name here.
                build_settings_vulkan->binary_file_name = kStrBuildSettingsOutputBinaryFileName;

                assert(node != nullptr);
                if (node != nullptr)
                {
                    // Generate Debug Info.
                    node = node->FirstChildElement(kXmlNodeVulkanGenerateDebugInfo);
                    ret = RgXMLUtils::ReadNodeTextBool(node, build_settings_vulkan->is_generate_debug_info_checked);
                }
            }

            if (ret && node != nullptr)
            {
                // No Explicit Bindings.
                node = node->NextSiblingElement(kXmlNodeVulkanNoExplicitBindings);
                ret = RgXMLUtils::ReadNodeTextBool(node, build_settings_vulkan->is_no_explicit_bindings_checked);
            }

            if (ret && node != nullptr)
            {
                // Use HLSL Block Offsets.
                node = node->NextSiblingElement(kXmlNodeVulkanUseHlslBlockOffsets);
                ret = RgXMLUtils::ReadNodeTextBool(node, build_settings_vulkan->is_use_hlsl_block_offsets_checked);
            }

            if (ret && node != nullptr)
            {
                // Use HLSL IO Mapping.
                node = node->NextSiblingElement(kXmlNodeVulkanUseHlslIoMapping);
                ret = RgXMLUtils::ReadNodeTextBool(node, build_settings_vulkan->is_use_hlsl_io_mapping_checked);
            }

            if (ret && node != nullptr)
            {
                // Get ICD location.
                node = node->NextSiblingElement(kXmlNodeVulkanIcdLocation);
                ret = (node != nullptr);
                assert(node != nullptr);

                std::string icd_location;
                bool should_read = RgXMLUtils::ReadNodeTextString(node, icd_location);

                if (should_read)
                {
                    build_settings_vulkan->icd_location = icd_location;
                }
            }

            if (ret && node != nullptr)
            {
                // Use Enable validation layer.
                node = node->NextSiblingElement(kXmlNodeVulkanEnableValidationLayer);
                ret = RgXMLUtils::ReadNodeTextBool(node, build_settings_vulkan->is_enable_validation_layers_checked);
            }

            if (ret && node != nullptr)
            {
                // Remember where we were in case we fail reading.
                auto node_original = node;

                // Get glslang options.
                node = node->NextSiblingElement(kXmlNodeVulkanGlslangOptionsLocation);
                ret = (node != nullptr);
                if (node != nullptr)
                {
                    std::string glslang_options;
                    bool should_read = RgXMLUtils::ReadNodeTextString(node, glslang_options);

                    if (should_read)
                    {
                        build_settings_vulkan->glslang_options = glslang_options;
                    }
                }
                else
                {
                    // If the rga project file does not have the additional glslang options
                    // node, that's OK - just assume that they are empty.
                    ret = true;

                    // Roll back to the last node.
                    node = node_original;
                }
            }

            // Alternative compiler paths.
            if (ret && node != nullptr)
            {
                // Alternative compiler paths.
                if (node != nullptr)
                {
                    node = node->NextSiblingElement(kXmlNodeAlternativeCompilerBinDir);
                    RgXMLUtils::ReadNodeTextString(node, std::get<CompilerFolderType::kBin>(build_settings_vulkan->compiler_paths));
                }
                if (node != nullptr)
                {
                    node = node->NextSiblingElement(kXmlNodeAlternativeCompilerIncDir);
                    RgXMLUtils::ReadNodeTextString(node, std::get<CompilerFolderType::kInclude>(build_settings_vulkan->compiler_paths));
                }
                if (node != nullptr)
                {
                    node = node->NextSiblingElement(kXmlNodeAlternativeCompilerLibDir);
                    RgXMLUtils::ReadNodeTextString(node, std::get<CompilerFolderType::kLib>(build_settings_vulkan->compiler_paths));
                }
            }
        }
    }

    return ret;
}

bool RgConfigFileWriterVulkan::WriteProjectConfigFile(const RgProject& project, const std::string& config_file_path)
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
        const RgProjectVulkan& vulkan_project = static_cast<const RgProjectVulkan&>(project);

        WriteCloneElements(vulkan_project, doc, clone_elems);
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

bool RgConfigFileWriterVulkan::WriteBuildSettingsElement(const std::shared_ptr<RgBuildSettings> build_settings, tinyxml2::XMLDocument& doc, tinyxml2::XMLElement*& build_settings_elem)
{
    bool ret = false;

    assert(build_settings != nullptr);
    if (build_settings != nullptr)
    {
        const std::shared_ptr<RgBuildSettingsVulkan> build_settings_vulkan = std::dynamic_pointer_cast<RgBuildSettingsVulkan>(build_settings);
        assert(build_settings_vulkan != nullptr);
        if (build_settings_vulkan != nullptr)
        {
            // Write API-agnostic build settings.
            ret = WriteGeneralBuildSettings(build_settings, doc, build_settings_elem);

            // Write Vulkan-specific settings.

            // Binary output file name.
            RgXMLUtils::AppendXMLElement(doc, build_settings_elem, kXmlNodeGlobalBinaryOutputFileName, build_settings_vulkan->binary_file_name.c_str());

            // Generate debug information.
            RgXMLUtils::AppendXMLElement(doc, build_settings_elem, kXmlNodeVulkanGenerateDebugInfo, build_settings_vulkan->is_generate_debug_info_checked);

            // No Explicit Bindings.
            RgXMLUtils::AppendXMLElement(doc, build_settings_elem, kXmlNodeVulkanNoExplicitBindings, build_settings_vulkan->is_no_explicit_bindings_checked);

            // Use HLSL Block Offsets.
            RgXMLUtils::AppendXMLElement(doc, build_settings_elem, kXmlNodeVulkanUseHlslBlockOffsets, build_settings_vulkan->is_use_hlsl_block_offsets_checked);

            // Use HLSL IO Mapping.
            RgXMLUtils::AppendXMLElement(doc, build_settings_elem, kXmlNodeVulkanUseHlslIoMapping, build_settings_vulkan->is_use_hlsl_io_mapping_checked);

            // ICD location.
            RgXMLUtils::AppendXMLElement(doc, build_settings_elem, kXmlNodeVulkanIcdLocation, build_settings_vulkan->icd_location.c_str());

            // Enable validation layers.
            RgXMLUtils::AppendXMLElement(doc, build_settings_elem, kXmlNodeVulkanEnableValidationLayer, build_settings_vulkan->is_enable_validation_layers_checked);

            // Glslang additional options.
            RgXMLUtils::AppendXMLElement(doc, build_settings_elem, kXmlNodeVulkanGlslangOptionsLocation, build_settings_vulkan->glslang_options.c_str());

            // Alternative compiler paths.
            RgXMLUtils::AppendXMLElement(doc, build_settings_elem, kXmlNodeAlternativeCompilerBinDir, std::get<CompilerFolderType::kBin>(build_settings_vulkan->compiler_paths).c_str());
            RgXMLUtils::AppendXMLElement(doc, build_settings_elem, kXmlNodeAlternativeCompilerIncDir, std::get<CompilerFolderType::kInclude>(build_settings_vulkan->compiler_paths).c_str());
            RgXMLUtils::AppendXMLElement(doc, build_settings_elem, kXmlNodeAlternativeCompilerLibDir, std::get<CompilerFolderType::kLib>(build_settings_vulkan->compiler_paths).c_str());
        }
    }

    return ret;
}

bool RgConfigFileWriterVulkan::WriteCloneElements(const RgProjectVulkan& project, tinyxml2::XMLDocument& doc, std::vector<tinyxml2::XMLElement*>& elems)
{
    bool ret = false;

    for (const std::shared_ptr<RgProjectClone>& clone : project.clones)
    {
        std::shared_ptr<RgProjectCloneVulkan> vulkan_clone = std::dynamic_pointer_cast<RgProjectCloneVulkan>(clone);
        if (vulkan_clone != nullptr)
        {
            // Project clone.
            tinyxml2::XMLElement* clone_element = doc.NewElement(kXmlNodeClone);

            // Clone ID.
            tinyxml2::XMLElement* clone_id = doc.NewElement(kXmlNodeCloneId);
            clone_id->SetText(vulkan_clone->clone_id);
            clone_element->LinkEndChild(clone_id);

            // Clone name.
            tinyxml2::XMLElement* clone_name = doc.NewElement(kXmlNodeCloneName);
            clone_name->SetText(vulkan_clone->clone_name.c_str());
            clone_element->LinkEndChild(clone_name);

            // Write the pipeline input files.
            ret = WritePipeline(doc, clone_element, false, vulkan_clone->pipeline);

            // Write the backup spv files if there are any.
            const ShaderInputFileArray& spv_backup_files = vulkan_clone->spv_backup_.shader_stages;
            if (std::find_if(spv_backup_files.cbegin(), spv_backup_files.cend(), [&](const std::string& s) {return !s.empty(); }) != spv_backup_files.cend())
            {
                ret = WritePipeline(doc, clone_element, true, vulkan_clone->spv_backup_);
            }

            // Build settings.
            tinyxml2::XMLElement* build_settings = doc.NewElement(kXmlNodeBuildSettings);
            ret = ret && WriteBuildSettingsElement(clone->build_settings, doc, build_settings);

            // Write the pipeline state.
            ret = ret && WritePipelineState(vulkan_clone, doc, clone_element);

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

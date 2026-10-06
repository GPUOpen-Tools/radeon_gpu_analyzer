//=============================================================================
/// Copyright (c) 2020-2026 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for xml writer class.
//=============================================================================

// C++
#include <cassert>
#include <set>
#include <vector>

// Shared.
#include "common/rga_analysis_summary_xml_constants.h"
using namespace rga_xml;
#include "common/rga_entry_type.h"
#include "common/rga_shared_utils.h"
#include "common/rga_sorting_utils.h"
#include "common/rga_version_info.h"
#include "common/rga_xml_constants.h"

// Backend.
#include "radeon_gpu_analyzer_backend/be_analysis_summary.h"
#include "radeon_gpu_analyzer_backend/be_utils.h"

// Local.
#include "radeon_gpu_analyzer_cli/kc_cli_config_file.h"
#include "radeon_gpu_analyzer_cli/kc_xml_writer.h"

// Static constants.
static const char* kStrFopenModeAppend = "a";

// Creates an element that has value of any primitive type.
template <typename T>
static void AppendXMLElement(tinyxml2::XMLDocument& xml_doc, tinyxml2::XMLElement* parent, const char* elem_name, T elem_value)
{
    tinyxml2::XMLElement* elem = xml_doc.NewElement(elem_name);
    elem->SetText(elem_value);
    parent->InsertEndChild(elem);
}

// Extract the CAL (generation) and code name.
// Example of "deviceName" format: "Baffin (Graphics IP v8)"
// Returned value: {"Graphics IP v8", "Baffin"}
static std::pair<std::string, std::string> GetGenAndCodeNames(const std::string& device_name)
{
    size_t      code_name_offset = device_name.find('(');
    std::string code_name        = (code_name_offset != std::string::npos ? device_name.substr(0, code_name_offset - 1) : device_name);
    std::string gen_name = (code_name_offset != std::string::npos ? device_name.substr(code_name_offset + 1, device_name.size() - code_name_offset - 2) : "");
    return {gen_name, code_name};
}

static bool AddSupportedGPUInfo(tinyxml2::XMLDocument& doc, tinyxml2::XMLElement& parent, const std::set<std::string>& targets)
{
    const char* kFilterIndicator1 = ":";
    const char* kFilterIndicator2 = "Not Used";
    bool        ret               = false;

    tinyxml2::XMLElement* supported_gpus = doc.NewElement(kXmlNodeSupportedGpus);

    // Add supported GPUS info.
    KcUtils::DeviceNameMap cards_map;
    if ((ret = KcUtils::GetMarketingNameToCodenameMapping(cards_map)) == true)
    {
        struct Device : public std::pair<std::string, std::set<std::string>>
        {
            Device(const std::string& architecture_name, const std::set<std::string>& marketing_names)
                : std::pair<std::string, std::set<std::string>>(architecture_name, marketing_names)
            {
            }

            // Returns The name of the GPU HW architecture.
            std::string GetArchitectureName() const
            {
                return GetGenAndCodeNames(first).first;
            }

            // Comparision operator.
            bool operator<(const Device& other) const
            {
                return GetArchitectureName() < other.GetArchitectureName();
            }
        };

        std::vector<Device> devices;
        for (const auto& card : cards_map)
        {
            devices.emplace_back(card.first, card.second);
        }
        std::sort(devices.begin(), devices.end(), GpuComparator<Device>{});
        for (const auto& device : devices)
        {
            std::string device_name       = device.first, code_name, gen_name;
            std::tie(gen_name, code_name) = GetGenAndCodeNames(device_name);

            // Skip targets that are not supported in this mode.
            if (targets.count(RgaSharedUtils::ToLower(code_name)) == 0)
            {
                continue;
            }

            tinyxml2::XMLElement* gpu            = doc.NewElement(kXmlNodeGpu);
            tinyxml2::XMLElement* gen            = doc.NewElement(kXmlNodeGeneration);
            tinyxml2::XMLElement* code_name_elem = doc.NewElement(kXmlNodeCodename);
            gen->SetText(gen_name.c_str());
            code_name_elem->SetText(code_name.c_str());
            gpu->LinkEndChild(gen);
            gpu->LinkEndChild(code_name_elem);

            // Add the list of marketing names or a placeholder if the list is empty.
            std::stringstream marketing_names;
            bool              first = true;
            for (const std::string& mrkt_name : device.second)
            {
                if (mrkt_name.find(kFilterIndicator1) == std::string::npos && mrkt_name.find(kFilterIndicator2) == std::string::npos)
                {
                    marketing_names << (first ? "" : ", ") << mrkt_name;
                    first = false;
                }
            }

            if (marketing_names.str().empty())
            {
                marketing_names << kStrXmlNodeMktName;
            }

            tinyxml2::XMLElement* public_names = doc.NewElement(kXmlNodeProductNames);
            public_names->SetText(marketing_names.str().c_str());
            gpu->LinkEndChild(public_names);
            supported_gpus->LinkEndChild(gpu);
        }
    }

    if (ret)
    {
        parent.LinkEndChild(supported_gpus);
    }

    return ret;
}

//
// Write tinyxml2 document to the XML file specified by "fileName".
// The document will be appended to the existing content of file.
// If "fileName" is empty, the document will be dumped to stdout.
//
static bool WriteXMLDocToFile(tinyxml2::XMLDocument& doc, const std::string& filename)
{
    bool result = false;

    if (filename.empty())
    {
        doc.Print();
        result = true;
    }
    else
    {
        std::FILE* xml_file = std::fopen(filename.c_str(), kStrFopenModeAppend);
        assert(xml_file != nullptr);
        if (xml_file != nullptr)
        {
            tinyxml2::XMLPrinter printer(xml_file);
            doc.Print(&printer);
            std::fclose(xml_file);
            result = true;
        }
    }

    return result;
}

bool KcXmlWriter::AddVersionInfoGPUList(beKA::RgaMode mode, const std::set<std::string>& targets, const std::string& filename)
{
    std::string mode_str;
    bool        ret = false;

    switch (mode)
    {
    case beKA::RgaMode::kModeOpenclOffline:
        mode_str = kStrXmlNodeOpenclOffline;
        ret      = true;
        break;
    case beKA::RgaMode::kModeVulkan:
        mode_str = kStrXmlNodeVulkan;
        ret      = true;
        break;
    case beKA::RgaMode::kModeBinary:
        mode_str = kStrXmlNodeBinaryAnalysis;
        ret      = true;
        break;
    }

    if (ret)
    {
        tinyxml2::XMLDocument doc;
        tinyxml2::XMLElement *name_elem, *mode_elem = doc.NewElement(kStrXmlNodeMode);
        ret = ret && (mode_elem != nullptr);
        if (ret)
        {
            doc.LinkEndChild(mode_elem);
            name_elem = doc.NewElement(kStrXmlNodeName);
            if ((ret = (name_elem != nullptr)) == true)
            {
                mode_elem->LinkEndChild(name_elem);
                name_elem->SetText(mode_str.c_str());
            }
        }

        ret = ret && AddSupportedGPUInfo(doc, *mode_elem, targets);
        ret = ret && WriteXMLDocToFile(doc, filename);
    }

    return ret;
}

bool KcXmlWriter::AddVersionInfoSystemData(const std::vector<BeVkPhysAdapterInfo>& info, const std::string& filename)
{
    bool                  ret = false;
    tinyxml2::XMLDocument doc;

    tinyxml2::XMLElement* adapters_elem = nullptr;
    tinyxml2::XMLElement* system_elem   = doc.NewElement(kStrXmlNodeSystem);
    ret                                 = (system_elem != nullptr);
    if (ret)
    {
        doc.LinkEndChild(system_elem);
        adapters_elem = doc.NewElement(kStrXmlNodeAdapters);
        if ((ret = (adapters_elem != nullptr)) == true)
        {
            system_elem->LinkEndChild(adapters_elem);
        }
    }

    // Add data for all physical adapters.
    for (auto adapter_info = info.cbegin(); adapter_info != info.cend() && ret; adapter_info++)
    {
        // <DisplayAdapter>
        tinyxml2::XMLElement* display_adapter = doc.NewElement(kStrXmlNodeAdapter);
        if ((ret = (display_adapter != nullptr)) == true)
        {
            adapters_elem->LinkEndChild(display_adapter);
        }

        // <ID>
        tinyxml2::XMLElement* id_elem = (ret ? doc.NewElement(kStrXmlNodeId) : nullptr);
        if ((ret = (id_elem != nullptr)) == true)
        {
            id_elem->SetText(adapter_info->id);
            display_adapter->LinkEndChild(id_elem);
        }

        // <Name>
        tinyxml2::XMLElement* name_elem = (ret ? doc.NewElement(kStrXmlNodeName) : nullptr);
        if ((ret = (name_elem != nullptr)) == true)
        {
            name_elem->SetText(adapter_info->name.c_str());
            display_adapter->LinkEndChild(name_elem);
        }

        // <VulkanDriverVersion>
        tinyxml2::XMLElement* driver_version_elem = (ret ? doc.NewElement(kStrXmlNodeVkDriver) : nullptr);
        if ((ret = (driver_version_elem != nullptr)) == true)
        {
            driver_version_elem->SetText(adapter_info->vk_driver_version.c_str());
            display_adapter->LinkEndChild(driver_version_elem);
        }

        // <VulkanAPIVersion>
        tinyxml2::XMLElement* vulkan_api_version_elem = (ret ? doc.NewElement(kStrXmlNodeVkApi) : nullptr);
        if ((ret = (vulkan_api_version_elem != nullptr)) == true)
        {
            vulkan_api_version_elem->SetText(adapter_info->vk_api_version.c_str());
            display_adapter->LinkEndChild(vulkan_api_version_elem);
        }
    }

    ret = ret && WriteXMLDocToFile(doc, filename);

    return ret;
}

bool KcXmlWriter::AddVersionInfoHeader(const std::string& filename)
{
    bool                  ret = true;
    tinyxml2::XMLDocument doc;

    // Add the RGA CLI version.
    tinyxml2::XMLElement* version_elem = doc.NewElement(kXmlNodeVersion);
    std::stringstream     version_tag;
    version_tag << kStrRgaVersion << "." << kStrRgaBuildNum;
    version_elem->SetText(version_tag.str().c_str());
    doc.LinkEndChild(version_elem);

    // Add the RGA CLI build date.
    // First, reformat the Windows date string provided in format "Day dd/mm/yyyy" to format "yyyy-mm-dd".
    std::string date_string = kStrRgaBuildDate;

#ifdef WIN32
    ret = RgaSharedUtils::ConvertDateString(date_string);
#endif

    tinyxml2::XMLElement* build_date_elem = doc.NewElement(kStrXmlNodeBuildDate);
    build_date_elem->SetText(date_string.c_str());
    doc.LinkEndChild(build_date_elem);

    ret = ret && WriteXMLDocToFile(doc, filename);

    return ret;
}

static bool AddOutputFile(tinyxml2::XMLDocument& doc, tinyxml2::XMLElement& parent, const std::string& filename, const char* tag)
{
    bool ret = true;
    if (!filename.empty())
    {
        tinyxml2::XMLElement* element = doc.NewElement(tag);
        if (element != nullptr)
        {
            element->SetText(filename.c_str());
            ret = (parent.LinkEndChild(element) != nullptr);
        }
        else
        {
            ret = false;
        }
    }
    return ret;
}

static bool AddEntryType(tinyxml2::XMLDocument& doc, tinyxml2::XMLElement* pEntry, RgaEntryType rga_entry_type)
{
    bool                  ret        = (pEntry != nullptr);
    tinyxml2::XMLElement* entry_type = nullptr;
    if (ret)
    {
        entry_type = doc.NewElement(kStrXmlNodeType);
        ret        = ret && (entry_type != nullptr && pEntry->LinkEndChild(entry_type) != nullptr);
    }
    if (ret)
    {
        std::string entry_type_str = "";
        ret                        = RgaEntryTypeUtils::GetEntryTypeStr(rga_entry_type, entry_type_str);
        if (ret && entry_type)
        {
            entry_type->SetText(entry_type_str.c_str());
        }
        else
        {
            ret = false;
        }
    }

    return ret;
}

static bool AddExtremelyLongEntryName(tinyxml2::XMLDocument& doc, tinyxml2::XMLElement* pEntry, const std::string& extremely_long_kernel_name)
{
    bool                  ret        = (pEntry != nullptr);
    tinyxml2::XMLElement* entry_name = nullptr;
    if (ret)
    {
        entry_name = doc.NewElement(kStrXmlNodeExtremelyLongName);
        ret        = ret && (entry_name != nullptr && pEntry->LinkEndChild(entry_name) != nullptr);
    }
    if (ret && entry_name)
    {
        entry_name->SetText(extremely_long_kernel_name.c_str());
    }
    else
    {
        ret = false;
    }

    return ret;
}

static bool AddOutputFiles(tinyxml2::XMLDocument& doc, tinyxml2::XMLElement* entry, const std::string& target, const RgOutputFiles& out_files)
{
    bool                  ret    = false;
    tinyxml2::XMLElement* output = doc.NewElement(kStrXmlNodeOutput);
    if (entry != nullptr && output != nullptr && entry->LinkEndChild(output) != nullptr)
    {
        // Add target GPU.
        tinyxml2::XMLElement* target_elem = doc.NewElement(kStrXmlNodeTarget);
        ret                               = (target_elem != nullptr && output->LinkEndChild(target_elem) != nullptr);
        if (ret)
        {
            target_elem->SetText(target.c_str());
        }
        // Add output files.
        if (!out_files.is_isa_file_temp)
        {
            ret = ret && AddOutputFile(doc, *output, out_files.isa_file, kStrXmlNodeIsa);
        }
        ret = ret && AddOutputFile(doc, *output, out_files.isa_csv_file, kStrXmlNodeCsvIsa);
        ret = ret && AddOutputFile(doc, *output, out_files.stats_file, kStrXmlNodeResUsage);
        ret = ret && AddOutputFile(doc, *output, out_files.livereg_file, kStrXmlNodeLivereg);
        ret = ret && AddOutputFile(doc, *output, out_files.livereg_sgpr_file, kStrXmlNodeLiveregSgpr);
        ret = ret && AddOutputFile(doc, *output, out_files.cfg_file, kStrXmlNodeCfg);
    }
    return ret;
}

bool KcXmlWriter::GenerateClSessionMetadataFile(const std::string& filename, const RgFileEntryData& file_entry_data, const RgClOutputMetadata& out_files)
{
    if (!BeUtils::IsFilePresent(filename))
    {
        bool                  ret = true;
        tinyxml2::XMLDocument init;
        std::string           current_device = "";

        tinyxml2::XMLElement* data_model_elem = init.NewElement(kStrXmlNodeDataModel);
        if ((data_model_elem != nullptr && init.LinkEndChild(data_model_elem) != nullptr))
        {
            data_model_elem->SetText(kStrXmlNodeDataModel);
        }
        else
        {
            ret = false;
        }
        ret = ret && (init.SaveFile(filename.c_str()) == tinyxml2::XML_SUCCESS);
        if (!ret)
        {
            return ret;
        }
    }

    tinyxml2::XMLDocument doc;
    std::string           current_device = "";

    tinyxml2::XMLElement* metadata = doc.NewElement(kStrXmlNodeMetadata);
    bool                  ret      = (metadata != nullptr && doc.LinkEndChild(metadata) != nullptr);

    // Add binary name.
    if (!out_files.empty() && !out_files.begin()->second.is_bin_file_temp)
    {
        tinyxml2::XMLElement* binary_elem = doc.NewElement(kStrXmlNodeBinary);
        ret                               = ret && (binary_elem != nullptr && metadata->LinkEndChild(binary_elem) != nullptr);
        if (ret)
        {
            binary_elem->SetText((out_files.begin())->second.bin_file.c_str());
        }
    }

    if (ret)
    {
        // Map: kernel_name --> vector{pair{device, out_files}}.
        std::map<std::string, std::vector<std::pair<std::string, RgOutputFiles>>> out_files_map;

        // Map: input_file_name --> outFilesMap.
        std::map<std::string, decltype(out_files_map)> metadata_table;

        // Reorder the output file metadata in "kernel-first" order.
        for (const auto& out_file_set : out_files)
        {
            const std::string& device = out_file_set.first.first;
            const std::string& kernel = out_file_set.first.second;
            out_files_map[kernel].push_back({device, out_file_set.second});
        }

        // Now, try to find a source file for each entry in "outFilesMap" and fill the "metadataTable".
        // Split the "outFilesMap" into parts so that each part contains entries from the same source file.
        // If no source file is found for an entry, use "<Unknown>" source file name.
        for (auto& out_file_item : out_files_map)
        {
            const std::string& entry_name = out_file_item.first;
            std::string        src_file_name;

            // Try to find a source file corresponding to this entry name.
            auto input_file_info = std::find_if(file_entry_data.begin(), file_entry_data.end(), [&](RgFileEntryData::const_reference entry_info) {
                for (auto entry : entry_info.second)
                {
                    if (std::get<0>(entry) == entry_name)
                        return true;
                }
                return false;
            });

            if (input_file_info == file_entry_data.end())
            {
                src_file_name = kStrXmlNodeSourceFile;
            }
            else
            {
                src_file_name = input_file_info->first;
            }
            metadata_table[src_file_name].insert(out_file_item);
        }

        // Store the "metadataTable" structure to the session metadata file.
        for (auto& input_file_data : metadata_table)
        {
            if (ret)
            {
                // Add input file info.
                tinyxml2::XMLElement* input_file      = doc.NewElement(kStrXmlNodeInputFile);
                ret                                   = ret && (input_file != nullptr && metadata->LinkEndChild(input_file) != nullptr);
                tinyxml2::XMLElement* input_file_path = doc.NewElement(kStrXmlNodePath);
                ret                                   = ret && (input_file_path != nullptr && input_file->LinkEndChild(input_file_path) != nullptr);

                if (ret)
                {
                    input_file_path->SetText(input_file_data.first.c_str());

                    if (ret)
                    {
                        // Add entry points info.
                        for (auto& entry_data : input_file_data.second)
                        {
                            tinyxml2::XMLElement* entry = doc.NewElement(kStrXmlNodeEntry);
                            ret                         = (entry != nullptr && input_file->LinkEndChild(entry) != nullptr);
                            // Add entry name & type.
                            tinyxml2::XMLElement* name_elem = doc.NewElement(kStrXmlNodeName);
                            ret                             = ret && (name_elem != nullptr && entry->LinkEndChild(name_elem) != nullptr);
                            if (ret && entry_data.second.size() > 0)
                            {
                                RgOutputFiles outFileData  = entry_data.second[0].second;
                                std::string   entry_name   = entry_data.first;
                                bool          abbreviation = !outFileData.entry_abbreviation.empty();
                                if (abbreviation)
                                {
                                    entry_name = outFileData.entry_abbreviation;
                                }
                                name_elem->SetText(entry_name.c_str());
                                ret = ret && AddEntryType(doc, entry, outFileData.entry_type);
                                if (abbreviation)
                                {
                                    ret = ret && AddExtremelyLongEntryName(doc, entry, entry_data.first);
                                }
                            }
                            // Add "Output" nodes.
                            for (const std::pair<std::string, RgOutputFiles>& deviceAndOutFiles : entry_data.second)
                            {
                                ret = ret && AddOutputFiles(doc, entry, deviceAndOutFiles.first, deviceAndOutFiles.second);
                                if (!ret)
                                {
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    ret = ret && WriteXMLDocToFile(doc, filename);

    return ret;
}

// Add the per-stage session data to the pipeline data.
static bool AddVulkanPipelineStages(tinyxml2::XMLDocument& doc, tinyxml2::XMLElement* pipeline, const RgVkOutputMetadata& output_metadata)
{
    bool ret = false;

    // For each per-stage data.
    for (const RgOutputFiles& out_files : output_metadata)
    {
        if (!out_files.input_file.empty())
        {
            tinyxml2::XMLElement* stage = doc.NewElement(kStrXmlNodeStage);
            if ((ret = (stage != nullptr && pipeline->LinkEndChild(stage))) == true)
            {
                ret = AddEntryType(doc, stage, out_files.entry_type);
            }

            // Add the input file.
            tinyxml2::XMLElement* input_file      = doc.NewElement(kStrXmlNodeInputFile);
            ret                                   = ret && (input_file != nullptr && stage->LinkEndChild(input_file) != nullptr);
            tinyxml2::XMLElement* input_file_path = doc.NewElement(kStrXmlNodePath);
            ret                                   = ret && (input_file_path != nullptr && input_file->LinkEndChild(input_file_path) != nullptr);

            assert(ret);
            if (ret)
            {
                input_file_path->SetText(out_files.input_file.c_str());
            }

            // Add the output file data.
            ret = ret && AddOutputFiles(doc, stage, out_files.device, out_files);
        }
    }

    return ret;
}

bool KcXmlWriter::GenerateVulkanSessionMetadataFile(const std::string& filename, const std::map<std::string, RgVkOutputMetadata>& output_metadata)
{
    tinyxml2::XMLDocument doc;
    std::string           current_device = "";

    tinyxml2::XMLElement* data_model_elem = doc.NewElement(kStrXmlNodeDataModel);
    bool                  ret             = (data_model_elem != nullptr && doc.LinkEndChild(data_model_elem) != nullptr);
    if (ret)
    {
        data_model_elem->SetText(kStrXmlNodeDataModel);
    }

    tinyxml2::XMLElement* metadata_elem = doc.NewElement(kStrXmlNodeMetadataPipeline);
    ret                                 = ret && (metadata_elem != nullptr && doc.LinkEndChild(metadata_elem) != nullptr);

    if (ret)
    {
        // For each per-device data.
        for (const auto& output_metadata_for_device : output_metadata)
        {
            const RgVkOutputMetadata& out_files_for_device = output_metadata_for_device.second;
            if (out_files_for_device.empty())
            {
                continue;
            }

            // Add the "pipeline" tag and the pipeline type.
            tinyxml2::XMLElement* pipeline = doc.NewElement(kStrXmlNodePipeline);
            ret                            = ret && (pipeline != nullptr && metadata_elem->LinkEndChild(pipeline) != nullptr);
            if (ret)
            {
                tinyxml2::XMLElement* pipeline_type = doc.NewElement(kStrXmlNodeType);
                if ((ret = (pipeline_type != nullptr && pipeline->LinkEndChild(pipeline_type))) == true)
                {
                    bool is_compute = RgaEntryTypeUtils::IsComputeBitSet(out_files_for_device[0].entry_type);
                    pipeline_type->SetText(is_compute ? kStrXmlNodePipelineTypeCompute : kStrXmlNodePipelineTypeGraphics);
                }
            }

            // Add the pipeline stages.
            ret = ret && AddVulkanPipelineStages(doc, pipeline, out_files_for_device);
        }
    }

    ret = ret && (doc.SaveFile(filename.c_str()) == tinyxml2::XML_SUCCESS);

    return ret;
}

bool KcXmlWriter::GenerateBinaryAnalysisSessionMetadataFile(const std::string&        filename,
                                                            const std::string&        binary_codeobj_file,
                                                            const RgClOutputMetadata& out_files)
{
    if (!BeUtils::IsFilePresent(filename))
    {
        bool                  ret = true;
        tinyxml2::XMLDocument init;
        std::string           current_device = "";

        tinyxml2::XMLElement* data_model_elem = init.NewElement(kStrXmlNodeDataModel);
        if ((data_model_elem != nullptr && init.LinkEndChild(data_model_elem) != nullptr))
        {
            data_model_elem->SetText(kStrXmlNodeDataModel);
        }
        else
        {
            ret = false;
        }
        ret = ret && (init.SaveFile(filename.c_str()) == tinyxml2::XML_SUCCESS);
        if (!ret)
        {
            return ret;
        }
    }

    tinyxml2::XMLDocument doc;
    std::string           current_device = "";

    tinyxml2::XMLElement* metadata = doc.NewElement(kStrXmlNodeMetadata);
    bool                  ret      = (metadata != nullptr && doc.LinkEndChild(metadata) != nullptr);

    // Add binary name.
    if (!binary_codeobj_file.empty() && !out_files.empty())
    {
        tinyxml2::XMLElement* binary_elem = doc.NewElement(kStrXmlNodeBinary);
        ret                               = ret && (binary_elem != nullptr && metadata->LinkEndChild(binary_elem) != nullptr);
        if (ret)
        {
            binary_elem->SetText(binary_codeobj_file.c_str());
        }
    }

    if (ret)
    {
        // Map: kernel_name --> vector{pair{device, out_files}}.
        std::map<std::string, std::vector<std::pair<std::string, RgOutputFiles>>> out_files_map;

        // Reorder the output file metadata in "kernel-first" order.
        for (const auto& out_file_set : out_files)
        {
            const std::string& device = out_file_set.first.first;
            const std::string& kernel = out_file_set.first.second;
            out_files_map[kernel].push_back({device, out_file_set.second});
        }

        // Add input file info.
        tinyxml2::XMLElement* input_file      = doc.NewElement(kStrXmlNodeInputFile);
        ret                                   = ret && (input_file != nullptr && metadata->LinkEndChild(input_file) != nullptr);
        tinyxml2::XMLElement* input_file_path = doc.NewElement(kStrXmlNodePath);
        ret                                   = ret && (input_file_path != nullptr && input_file->LinkEndChild(input_file_path) != nullptr);

        if (ret)
        {
            input_file_path->SetText(kStrXmlNodeSourceFile);

            if (ret)
            {
                // Add entry points info.
                for (const auto& entry_data : out_files_map)
                {
                    tinyxml2::XMLElement* entry = doc.NewElement(kStrXmlNodeEntry);
                    ret                         = (entry != nullptr && input_file->LinkEndChild(entry) != nullptr);
                    // Add entry name & type.
                    tinyxml2::XMLElement* name_elem = doc.NewElement(kStrXmlNodeName);
                    ret                             = ret && (name_elem != nullptr && entry->LinkEndChild(name_elem) != nullptr);
                    if (ret && entry_data.second.size() > 0)
                    {
                        RgOutputFiles outFileData  = entry_data.second[0].second;
                        std::string   entry_name   = entry_data.first;
                        bool          abbreviation = !outFileData.entry_abbreviation.empty();
                        if (abbreviation)
                        {
                            entry_name = outFileData.entry_abbreviation;
                        }
                        name_elem->SetText(entry_name.c_str());
                        ret = ret && AddEntryType(doc, entry, outFileData.entry_type);
                        if (abbreviation)
                        {
                            ret = ret && AddExtremelyLongEntryName(doc, entry, entry_data.first);
                        }
                    }

                    // Add "Output" nodes.
                    for (const std::pair<std::string, RgOutputFiles>& deviceAndOutFiles : entry_data.second)
                    {
                        ret = ret && AddOutputFiles(doc, entry, deviceAndOutFiles.first, deviceAndOutFiles.second);
                        if (!ret)
                        {
                            break;
                        }
                    }
                }
            }
        }
    }

    ret = ret && WriteXMLDocToFile(doc, filename);

    return ret;
}

bool KcXmlWriter::WriteAnalysisSummaryToFile(const RgaAnalysisSummary& summary, const std::string& filename)
{
    if (filename.empty())
    {
        return false;
    }

    tinyxml2::XMLDocument doc;

    tinyxml2::XMLDeclaration* declaration = doc.NewDeclaration();
    if (!declaration)
    {
        return false;
    }
    doc.InsertFirstChild(declaration);

    tinyxml2::XMLElement* root = doc.NewElement(kRootElement);
    if (!root)
    {
        return false;
    }
    doc.InsertEndChild(root);

    WriteDocument(doc, root, summary.document_);
    WriteAnalysisResults(doc, root, summary.results_);

    tinyxml2::XMLError error = doc.SaveFile(filename.c_str());
    return error == tinyxml2::XML_SUCCESS;
}

void KcXmlWriter::WriteDocument(tinyxml2::XMLDocument& doc, tinyxml2::XMLElement* root, const RgaAnalysisSummary::Document& document)
{
    assert(root != nullptr);
    if (!root)
    {
        return;
    }

    tinyxml2::XMLElement* doc_element = doc.NewElement(kDocument);
    if (!doc_element)
    {
        return;
    }
    root->InsertEndChild(doc_element);

    tinyxml2::XMLElement* schema_element = CreateTextElement(doc, kSchemaVersion, document.schema_version_);
    if (schema_element)
    {
        doc_element->InsertEndChild(schema_element);
    }
}

void KcXmlWriter::WriteAnalysisResults(tinyxml2::XMLDocument& doc, tinyxml2::XMLElement* root, const std::vector<RgaAnalysisSummary::AnalysisResult>& results)
{
    assert(root != nullptr);
    if (!root)
    {
        return;
    }

    tinyxml2::XMLElement* results_element = doc.NewElement(kAnalysisResults);
    if (!results_element)
    {
        return;
    }
    root->InsertEndChild(results_element);

    for (const auto& result : results)
    {
        WriteAnalysisResult(doc, results_element, result);
    }
}

void KcXmlWriter::WriteAnalysisResult(tinyxml2::XMLDocument& doc, tinyxml2::XMLElement* parent, const RgaAnalysisSummary::AnalysisResult& result)
{
    assert(parent != nullptr);
    if (!parent)
    {
        return;
    }

    tinyxml2::XMLElement* result_element = doc.NewElement(kAnalysisResult);
    if (!result_element)
    {
        return;
    }
    parent->InsertEndChild(result_element);

    tinyxml2::XMLElement* target_arch_element = doc.NewElement(kTargetArchitecture);
    if (target_arch_element)
    {
        result_element->InsertEndChild(target_arch_element);

        if (!result.target_architecture_.empty())
        {
            tinyxml2::XMLElement* arch_name_element = CreateTextElement(doc, kTargetArchitectureName, result.target_architecture_);
            if (arch_name_element)
            {
                target_arch_element->InsertEndChild(arch_name_element);
            }
        }

        WriteTargetArchitectureMetadata(doc, target_arch_element, result.target_architecture_metadata_);
    }

    WriteInputs(doc, result_element, result.inputs_);

    WriteOutput(doc, result_element, result.output_);
}

void KcXmlWriter::WriteTargetArchitectureMetadata(tinyxml2::XMLDocument&                                doc,
                                                  tinyxml2::XMLElement*                                 parent,
                                                  const RgaAnalysisSummary::TargetArchitectureMetadata& metadata)
{
    assert(parent != nullptr);
    if (!parent)
    {
        return;
    }

    tinyxml2::XMLElement* metadata_element = doc.NewElement(kTargetArchitectureMetadata);
    if (!metadata_element)
    {
        return;
    }
    parent->InsertEndChild(metadata_element);

    WriteMetadataTypes(doc,
                       metadata_element,
                       kBranchInstructionTypes,
                       kBranchInstructionType,
                       kBranchInstructionTypeId,
                       kBranchInstructionTypeName,
                       nullptr,
                       metadata.branch_types_);

    WriteMetadataTypes(doc, metadata_element, kRegisterTypes, kRegisterType, kRegisterTypeId, kRegisterTypeName, nullptr, metadata.register_types_);

    WriteFunctionalGroupMetadata(doc, metadata_element, metadata.instruction_functional_groups_);
}

void KcXmlWriter::WriteMetadataTypes(tinyxml2::XMLDocument&                                                           doc,
                                     tinyxml2::XMLElement*                                                            parent,
                                     const char*                                                                      container_name,
                                     const char*                                                                      element_name,
                                     const char*                                                                      id_name,
                                     const char*                                                                      name_name,
                                     const char*                                                                      description_name,
                                     const std::vector<RgaAnalysisSummary::TargetArchitectureMetadata::MetadataType>& types)
{
    assert(parent != nullptr);
    assert(container_name != nullptr);
    assert(element_name != nullptr);
    assert(id_name != nullptr);
    assert(name_name != nullptr);

    if (!parent || !container_name || !element_name || !id_name || !name_name)
    {
        return;
    }

    tinyxml2::XMLElement* types_element = doc.NewElement(container_name);
    if (!types_element)
    {
        return;
    }
    parent->InsertEndChild(types_element);

    for (const auto& type : types)
    {
        tinyxml2::XMLElement* type_element = doc.NewElement(element_name);
        if (!type_element)
        {
            continue;
        }
        types_element->InsertEndChild(type_element);

        tinyxml2::XMLElement* id_element = CreateIntElement(doc, id_name, type.id_);
        if (id_element)
        {
            type_element->InsertEndChild(id_element);
        }

        tinyxml2::XMLElement* name_element = CreateTextElement(doc, name_name, type.name_);
        if (name_element)
        {
            type_element->InsertEndChild(name_element);
        }

        if (description_name && !type.description_.empty())
        {
            tinyxml2::XMLElement* desc_element = CreateTextElement(doc, description_name, type.description_);
            if (desc_element)
            {
                type_element->InsertEndChild(desc_element);
            }
        }
    }
}

void KcXmlWriter::WriteInputs(tinyxml2::XMLDocument& doc, tinyxml2::XMLElement* parent, const RgaAnalysisSummary::AnalysisResult::Inputs& inputs)
{
    assert(parent != nullptr);
    if (!parent)
    {
        return;
    }

    tinyxml2::XMLElement* inputs_element = doc.NewElement(kInputs);
    if (!inputs_element)
    {
        return;
    }
    parent->InsertEndChild(inputs_element);

    for (const auto& input_file : inputs.inputs_)
    {
        tinyxml2::XMLElement* input_element = CreateTextElement(doc, kInput, input_file);
        if (input_element)
        {
            inputs_element->InsertEndChild(input_element);
        }
    }
}

void KcXmlWriter::WriteOutput(tinyxml2::XMLDocument& doc, tinyxml2::XMLElement* parent, const RgaAnalysisSummary::AnalysisResult::Output& output)
{
    tinyxml2::XMLElement* output_element = doc.NewElement(kOutput);
    parent->InsertEndChild(output_element);

    if (!output.api_.empty())
    {
        output_element->InsertEndChild(CreateTextElement(doc, kApi, output.api_));
    }

    if (!output.kernels_.empty())
    {
        tinyxml2::XMLElement* kernels_element = doc.NewElement(kKernels);
        output_element->InsertEndChild(kernels_element);

        for (const auto& kernel : output.kernels_)
        {
            WriteKernel(doc, kernels_element, kernel);
        }
    }
}

void KcXmlWriter::WriteKernel(tinyxml2::XMLDocument& doc, tinyxml2::XMLElement* parent, const RgaAnalysisSummary::Kernel& kernel)
{
    assert(parent != nullptr);
    if (!parent)
    {
        return;
    }

    tinyxml2::XMLElement* kernel_element = doc.NewElement(kKernel);
    if (!kernel_element)
    {
        return;
    }
    parent->InsertEndChild(kernel_element);

    tinyxml2::XMLElement* id_element = CreateIntElement(doc, kKernelId, kernel.kernel_id_ != -1 ? kernel.kernel_id_ : 0);
    if (id_element)
    {
        kernel_element->InsertEndChild(id_element);
    }

    tinyxml2::XMLElement* name_element = CreateTextElement(doc, kKernelName, kernel.kernel_name_);
    if (name_element)
    {
        kernel_element->InsertEndChild(name_element);
    }

    tinyxml2::XMLElement* type_element = CreateTextElement(doc, kKernelType, kernel.kernel_type_);
    if (type_element)
    {
        kernel_element->InsertEndChild(type_element);
    }

    WriteISA(doc, kernel_element, kernel);

    WriteStatistics(doc, kernel_element, kernel.stats_);
}

void KcXmlWriter::WriteISA(tinyxml2::XMLDocument& doc, tinyxml2::XMLElement* parent, const RgaAnalysisSummary::Kernel& kernel)
{
    tinyxml2::XMLElement* isa_element = doc.NewElement(kIsa);
    parent->InsertEndChild(isa_element);

    isa_element->InsertEndChild(CreateTextElement(doc, kDisassembly, kernel.text_disassembly_));

    WriteInstructions(doc, isa_element, kernel.instructions_);
    WriteBlocks(doc, isa_element, kernel.cfg_.blocks_);

    WriteCFG(doc, isa_element, kernel.cfg_.branches_);
}

void KcXmlWriter::WriteBlocks(tinyxml2::XMLDocument& doc, tinyxml2::XMLElement* parent, const RgaAnalysisSummary::BasicBlocks& blocks)
{
    tinyxml2::XMLElement* blocks_element = doc.NewElement(kBlocks);
    parent->InsertEndChild(blocks_element);

    for (const auto& block : blocks)
    {
        if (block)
        {
            WriteBlock(doc, blocks_element, *block);
        }
    }
}

void KcXmlWriter::WriteBlock(tinyxml2::XMLDocument& doc, tinyxml2::XMLElement* parent, const RgaAnalysisSummary::BasicBlock& block)
{
    assert(parent != nullptr);
    if (!parent)
    {
        return;
    }

    tinyxml2::XMLElement* block_element = doc.NewElement(kBlock);
    if (!block_element)
    {
        return;
    }
    parent->InsertEndChild(block_element);

    if (block.block_id_ != -1)
    {
        tinyxml2::XMLElement* id_element = CreateIntElement(doc, kBlockId, block.block_id_);
        if (id_element)
        {
            block_element->InsertEndChild(id_element);
        }
    }

    if (!block.block_label_.empty())
    {
        block_element->InsertEndChild(CreateTextElement(doc, kBlockLabel, block.block_label_));
    }
    if (!block.display_label_.empty())
    {
        block_element->InsertEndChild(CreateTextElement(doc, kBlockDisplayLabel, block.display_label_));
    }

    WriteBlockMetadata(doc, block_element, block.metadata_);

    WriteBlockInstructions(doc, block_element, block.instruction_references_);
}

void KcXmlWriter::WriteBlockMetadata(tinyxml2::XMLDocument& doc, tinyxml2::XMLElement* parent, const RgaAnalysisSummary::BasicBlock::Metadata& metadata)
{
    tinyxml2::XMLElement* metadata_element = doc.NewElement(kBlockMetadata);
    parent->InsertEndChild(metadata_element);

    {
        tinyxml2::XMLElement* predecessors_element = doc.NewElement(kBlockPredecessorsIds);
        metadata_element->InsertEndChild(predecessors_element);

        std::stringstream predecessors;
        for (size_t i = 0; i < metadata.predecessors_ids_.size(); ++i)
        {
            if (i > 0)
            {
                predecessors << ",";
            }
            predecessors << std::to_string(metadata.predecessors_ids_[i]);
        }
        predecessors_element->SetText(predecessors.str().c_str());
    }

    if (!metadata.successors_ids_.empty())
    {
        tinyxml2::XMLElement* successors_element = doc.NewElement(kBlockSuccessorsIds);
        metadata_element->InsertEndChild(successors_element);

        std::stringstream successors;
        for (size_t i = 0; i < metadata.successors_ids_.size(); ++i)
        {
            if (i > 0)
            {
                successors << ",";
            }
            successors << std::to_string(metadata.successors_ids_[i]);
        }
        successors_element->SetText(successors.str().c_str());
    }

    if (!metadata.successors_branch_types_.empty())
    {
        tinyxml2::XMLElement* successors_types_element = doc.NewElement(kBlockSuccessorsBranchTypes);
        metadata_element->InsertEndChild(successors_types_element);

        std::stringstream successors_types;
        for (size_t i = 0; i < metadata.successors_branch_types_.size(); ++i)
        {
            if (i > 0)
            {
                successors_types << ",";
            }
            successors_types << metadata.successors_branch_types_[i];
        }

        successors_types_element->SetText(successors_types.str().c_str());
    }

    WriteLiveRegisters(doc, metadata_element, kLiveRegistersIn, metadata.live_registers_in_);
    WriteLiveRegisters(doc, metadata_element, kLiveRegistersOut, metadata.live_registers_out_);
}

void KcXmlWriter::WriteLiveRegisters(tinyxml2::XMLDocument&                               doc,
                                     tinyxml2::XMLElement*                                parent,
                                     std::string                                          name,
                                     const RgaAnalysisSummary::BasicBlock::LiveRegisters& live_registers)
{
    tinyxml2::XMLElement* live_regs_element = doc.NewElement(name.c_str());
    parent->InsertEndChild(live_regs_element);

    if (live_registers.vgpr_pressure_ > 0)
    {
        live_regs_element->InsertEndChild(CreateUInt64Element(doc, kVgprPressure, live_registers.vgpr_pressure_));
    }

    WriteRegisterAccess(doc, live_regs_element, beKA::BeIsaSpecExplorer::RegisterType::kVGPR, live_registers.vgprs_access_);

    if (live_registers.sgpr_pressure_ > 0)
    {
        live_regs_element->InsertEndChild(CreateUInt64Element(doc, kSgprPressure, live_registers.sgpr_pressure_));
    }

    WriteRegisterAccess(doc, live_regs_element, beKA::BeIsaSpecExplorer::RegisterType::kSGPR, live_registers.sgprs_access_);
}

void KcXmlWriter::WriteBlockInstructions(tinyxml2::XMLDocument&                                       doc,
                                         tinyxml2::XMLElement*                                        parent,
                                         const RgaAnalysisSummary::BasicBlock::InstructionReferences& instruction_references)
{
    if (instruction_references.empty())
    {
        return;
    }

    tinyxml2::XMLElement* instructions_element = doc.NewElement(kInstructions);
    parent->InsertEndChild(instructions_element);

    for (const auto& instruction_reference : instruction_references)
    {
        tinyxml2::XMLElement* instr_element = doc.NewElement(kInstructionReference);
        instructions_element->InsertEndChild(instr_element);

        if (instruction_reference.instruction_id_ != -1)
        {
            instr_element->InsertEndChild(CreateIntElement(doc, kInstructionId, instruction_reference.instruction_id_));
        }

        if (!instruction_reference.opcode_.empty())
        {
            instr_element->InsertEndChild(CreateTextElement(doc, kOpcode, instruction_reference.opcode_));
        }

        if (!instruction_reference.operands_.empty())
        {
            instr_element->InsertEndChild(CreateTextElement(doc, kOperands, instruction_reference.operands_));
        }
    }
}

void KcXmlWriter::WriteInstructions(tinyxml2::XMLDocument& doc, tinyxml2::XMLElement* parent, const RgaAnalysisSummary::Instructions& instructions)
{
    tinyxml2::XMLElement* instructions_element = doc.NewElement(kInstructions);
    parent->InsertEndChild(instructions_element);

    for (const auto& instruction : instructions)
    {
        if (instruction)
        {
            WriteInstruction(doc, instructions_element, *instruction);
        }
    }
}
void KcXmlWriter::WriteInstruction(tinyxml2::XMLDocument& doc, tinyxml2::XMLElement* parent, const RgaAnalysisSummary::Instruction& instruction)
{
    assert(parent != nullptr);
    if (!parent)
        return;

    tinyxml2::XMLElement* instr_element = doc.NewElement(kInstruction);
    parent->InsertEndChild(instr_element);

    if (instruction.instruction_id_ != -1)
    {
        instr_element->InsertEndChild(CreateIntElement(doc, kInstructionId, instruction.instruction_id_));
    }

    if (!instruction.offset_.empty())
    {
        instr_element->InsertEndChild(CreateTextElement(doc, kOffset, instruction.offset_));
    }

    if (!instruction.binary_representation_.empty())
    {
        instr_element->InsertEndChild(CreateTextElement(doc, kBinaryRepresentation, instruction.binary_representation_));
        instr_element->InsertEndChild(CreateUInt64Element(doc, kInstructionSize, instruction.instruction_size_));
    }

    if (!instruction.opcode_.empty())
    {
        instr_element->InsertEndChild(CreateTextElement(doc, kOpcode, instruction.opcode_));
    }

    WriteInstructionSemantics(doc, instr_element, instruction.semantics_);

    WriteInstructionDebugInfo(doc, instr_element, instruction.debug_info_);

    WriteOperands(doc, instr_element, instruction.operands_);

    if (!instruction.functional_group_.empty())
    {
        instr_element->InsertEndChild(CreateTextElement(doc, kFunctionalGroup, instruction.functional_group_));
    }

    if (!instruction.functional_group_.empty())
    {
        if (instruction.functional_sub_group_.empty())
        {
            instr_element->InsertEndChild(CreateTextElement(doc, kFunctionalSubGroups, "General"));
        }
        else
        {
            instr_element->InsertEndChild(CreateTextElement(doc, kFunctionalSubGroups, instruction.functional_sub_group_));
        }
    }

    if (!instruction.instruction_description_.empty())
    {
        instr_element->InsertEndChild(CreateTextElement(doc, kInstructionDescription, instruction.instruction_description_));
    }

    if (instruction.vgpr_pressure_ > 0)
    {
        instr_element->InsertEndChild(CreateUInt64Element(doc, kVgprPressure, instruction.vgpr_pressure_));
    }

    if (instruction.vgpr_allocation_ > 0)
    {
        instr_element->InsertEndChild(CreateUInt64Element(doc, kVgprAllocation, instruction.vgpr_allocation_));
    }

    WriteRegisterAccess(doc, instr_element, beKA::BeIsaSpecExplorer::RegisterType::kVGPR, instruction.vgprs_access_);

    if (instruction.sgpr_pressure_ > 0)
    {
        instr_element->InsertEndChild(CreateUInt64Element(doc, kSgprPressure, instruction.sgpr_pressure_));
    }

    if (instruction.sgpr_allocation_ > 0)
    {
        instr_element->InsertEndChild(CreateUInt64Element(doc, kSgprAllocation, instruction.sgpr_allocation_));
    }

    WriteRegisterAccess(doc, instr_element, beKA::BeIsaSpecExplorer::RegisterType::kSGPR, instruction.sgprs_access_);
}

void KcXmlWriter::WriteInstructionSemantics(tinyxml2::XMLDocument&                            doc,
                                            tinyxml2::XMLElement*                             parent,
                                            const RgaAnalysisSummary::Instruction::Semantics& semantics)
{
    tinyxml2::XMLElement* semantics_element = doc.NewElement(kInstructionSemantics);
    parent->InsertEndChild(semantics_element);

    semantics_element->InsertEndChild(CreateBoolElement(doc, kIsLabel, semantics.is_label_));

    WriteBranchInfo(doc, semantics_element, semantics.branch_info_);
}

void KcXmlWriter::WriteBranchInfo(tinyxml2::XMLDocument& doc, tinyxml2::XMLElement* parent, const RgaAnalysisSummary::Instruction::BranchInfo& branch_info)
{
    tinyxml2::XMLElement* branch_element = doc.NewElement(kBranchInfo);
    parent->InsertEndChild(branch_element);

    branch_element->InsertEndChild(CreateBoolElement(doc, kIsBranch, branch_info.is_branch_));

    if (!branch_info.branch_type_.empty())
    {
        branch_element->InsertEndChild(CreateTextElement(doc, kBranchType, branch_info.branch_type_));
    }

    if (!branch_info.branch_target_.empty())
    {
        branch_element->InsertEndChild(CreateTextElement(doc, kBranchTarget, branch_info.branch_target_));
    }
}

void KcXmlWriter::WriteInstructionDebugInfo(tinyxml2::XMLDocument&                            doc,
                                            tinyxml2::XMLElement*                             parent,
                                            const RgaAnalysisSummary::Instruction::DebugInfo& debug_info)
{
    tinyxml2::XMLElement* debug_element = doc.NewElement(kInstructionDebugInfo);
    parent->InsertEndChild(debug_element);

    WriteHighLevelSource(doc, debug_element, debug_info.high_level_source_);
}

void KcXmlWriter::WriteHighLevelSource(tinyxml2::XMLDocument& doc, tinyxml2::XMLElement* parent, const RgaAnalysisSummary::Instruction::HighLevelSource& source)
{
    tinyxml2::XMLElement* source_element = doc.NewElement(kHighLevelSource);
    parent->InsertEndChild(source_element);

    if (!source.file_name_.empty())
    {
        source_element->InsertEndChild(CreateTextElement(doc, kFileName, source.file_name_));
    }

    if (source.line_number_ >= 0)
    {
        source_element->InsertEndChild(CreateIntElement(doc, kLineNumber, source.line_number_));
    }
}

void KcXmlWriter::WriteOperands(tinyxml2::XMLDocument& doc, tinyxml2::XMLElement* parent, const RgaAnalysisSummary::Instruction::Operands& operands)
{
    tinyxml2::XMLElement* operands_element = doc.NewElement(kOperands);
    parent->InsertEndChild(operands_element);

    if (!operands.text_.empty())
    {
        operands_element->InsertEndChild(CreateTextElement(doc, kOperandsText, operands.text_));
    }

    for (const auto& operand : operands.items_)
    {
        WriteOperand(doc, operands_element, operand);
    }
}

void KcXmlWriter::WriteOperand(tinyxml2::XMLDocument& doc, tinyxml2::XMLElement* parent, const RgaAnalysisSummary::Instruction::Operand& operand)
{
    tinyxml2::XMLElement* operand_element = doc.NewElement(kOperand);
    parent->InsertEndChild(operand_element);
    operand_element->InsertEndChild(CreateIntElement(doc, kOperandOrder, operand.order_));

    if (!operand.access_.empty())
    {
        operand_element->InsertEndChild(CreateTextElement(doc, kAccess, operand.access_));
    }

    if (!operand.type_.empty())
    {
        operand_element->InsertEndChild(CreateTextElement(doc, kOperandType, operand.type_));
    }

    if (!operand.name_.empty())
    {
        operand_element->InsertEndChild(CreateTextElement(doc, kOperandName, operand.name_));
    }
}

void KcXmlWriter::WriteFunctionalGroupMetadata(
    tinyxml2::XMLDocument&                                                                           doc,
    tinyxml2::XMLElement*                                                                            parent,
    const std::vector<RgaAnalysisSummary::TargetArchitectureMetadata::FunctionalGroupMetadata>& groups)
{
    assert(parent != nullptr);
    if (!parent)
    {
        return;
    }

    tinyxml2::XMLElement* container = doc.NewElement(kInstructionFunctionalGroups);
    if (!container)
    {
        return;
    }
    parent->InsertEndChild(container);

    for (const auto& group : groups)
    {
        tinyxml2::XMLElement* element = doc.NewElement(kInstructionFunctionalGroup);
        if (!element)
        {
            continue;
        }
        container->InsertEndChild(element);

        tinyxml2::XMLElement* id_element = CreateIntElement(doc, kInstructionFunctionalGroupId, group.id_);
        if (id_element)
        {
            element->InsertEndChild(id_element);
        }

        tinyxml2::XMLElement* group_name_element = CreateTextElement(doc, kInstructionFunctionalGroupGroupName, group.group_name_);
        if (group_name_element)
        {
            element->InsertEndChild(group_name_element);
        }

        {
            tinyxml2::XMLElement* sub_group_name_element = nullptr;
            if (group.sub_group_name_.empty())
            {
                sub_group_name_element = CreateTextElement(doc, kInstructionFunctionalGroupSubGroupName, "General");
            }
            else
            {
                sub_group_name_element = CreateTextElement(doc, kInstructionFunctionalGroupSubGroupName, group.sub_group_name_);
            }
            if (sub_group_name_element)
            {
                element->InsertEndChild(sub_group_name_element);
            }
        }

        if (!group.description_.empty())
        {
            tinyxml2::XMLElement* description_element = CreateTextElement(doc, kInstructionFunctionalGroupDesc, group.description_);
            if (description_element)
            {
                element->InsertEndChild(description_element);
            }
        }
    }
}

void KcXmlWriter::WriteRegisterAccess(tinyxml2::XMLDocument&                                              doc,
                                      tinyxml2::XMLElement*                                               parent,
                                      beKA::BeIsaSpecExplorer::RegisterType                               reg_type,
                                      const std::vector<RgaAnalysisSummary::Instruction::RegisterAccess>& reg_access)
{
    if (reg_access.empty())
        return;

    tinyxml2::XMLElement* reg_element =
        (reg_type == beKA::BeIsaSpecExplorer::RegisterType::kSGPR) ? doc.NewElement(kSgprsAccess) : doc.NewElement(kVgprsAccess);
    parent->InsertEndChild(reg_element);

    for (const auto& ra : reg_access)
    {
        WriteRegisterAccess(doc, reg_element, reg_type, ra);
    }
}

void KcXmlWriter::WriteRegisterAccess(tinyxml2::XMLDocument&                                 doc,
                                      tinyxml2::XMLElement*                                  parent,
                                      beKA::BeIsaSpecExplorer::RegisterType                  reg_type,
                                      const RgaAnalysisSummary::Instruction::RegisterAccess& reg_access)
{
    tinyxml2::XMLElement* reg_element = (reg_type == beKA::BeIsaSpecExplorer::RegisterType::kSGPR) ? doc.NewElement(kSgpr) : doc.NewElement(kVgpr);
    parent->InsertEndChild(reg_element);

    if (reg_access.index_ >= 0)
    {
        reg_element->InsertEndChild(CreateIntElement(doc, kIndex, reg_access.index_));
    }

    if (!reg_access.name_.empty())
    {
        reg_element->InsertEndChild(CreateTextElement(doc, kName, reg_access.name_));
    }

    if (!reg_access.access_.empty())
    {
        reg_element->InsertEndChild(CreateTextElement(doc, kAccess, reg_access.access_));
    }
}

void KcXmlWriter::WriteCFG(tinyxml2::XMLDocument& doc, tinyxml2::XMLElement* parent, const std::vector<RgaAnalysisSummary::ControlFlowGraph::Branch>& branches)
{
    tinyxml2::XMLElement* cfg_element = doc.NewElement(kCfg);
    parent->InsertEndChild(cfg_element);

    for (const auto& branch : branches)
    {
        WriteBranch(doc, cfg_element, branch);
    }
}

void KcXmlWriter::WriteBranch(tinyxml2::XMLDocument& doc, tinyxml2::XMLElement* parent, const RgaAnalysisSummary::ControlFlowGraph::Branch& branch)
{
    tinyxml2::XMLElement* branch_element = doc.NewElement(kBranch);
    parent->InsertEndChild(branch_element);

    if (branch.source_block_id_ != -1)
    {
        branch_element->InsertEndChild(CreateIntElement(doc, kSourceBlockId, branch.source_block_id_));
    }

    if (branch.destination_block_id_ != -1)
    {
        branch_element->InsertEndChild(CreateIntElement(doc, kDestinationBlockId, branch.destination_block_id_));
    }
}

void KcXmlWriter::WriteStatistics(tinyxml2::XMLDocument& doc, tinyxml2::XMLElement* parent, const RgaAnalysisSummary::Statistics& stats)
{
    assert(parent != nullptr);
    if (!parent)
    {
        return;
    }

    tinyxml2::XMLElement* stats_element = doc.NewElement(kStats);
    if (!stats_element)
    {
        return;
    }
    parent->InsertEndChild(stats_element);

    if (!stats.api_shader_hash_.empty())
    {
        tinyxml2::XMLElement* elem = CreateTextElement(doc, kApiShaderHash, stats.api_shader_hash_);
        if (elem)
        {
            stats_element->InsertEndChild(elem);
        }
    }

    auto write_stat = [&](const char* name, uint64_t value) {
        stats_element->InsertEndChild(CreateUInt64Element(doc, name, value != beKA::kCalValue64Na ? value : 0));
    };

    write_stat(kVgprsAllocationGranularity, stats.vgprs_allocation_granularity_);
    write_stat(kVgprsAllocated,             stats.vgprs_allocated_);
    write_stat(kVgprsUsed,                  stats.vgprs_used_);
    write_stat(kVgprsSpilled,               stats.vgprs_spilled_);
    write_stat(kVgprsTotal,                 stats.vgprs_total_ != 0 ? stats.vgprs_total_ : 256);

    auto write_stat_or_na = [&](const char* name, uint64_t value) {
        tinyxml2::XMLElement* elem = CreateTextElement(doc, name, beKA::AnalysisData::na_or(value));
        if (elem)
        {
            stats_element->InsertEndChild(elem);
        }
    };
    write_stat_or_na(kAgprsUsed,  stats.agprs_used_);
    write_stat_or_na(kAgprsTotal, stats.agprs_total_);

    write_stat(kSgprsAllocated,             stats.sgprs_allocated_);
    write_stat(kSgprsUsed,                  stats.sgprs_used_);
    write_stat(kSgprsSpilled,               stats.sgprs_spilled_);
    write_stat(kScratchMemoryBytes,         stats.scratch_memory_bytes_);
    write_stat(kLdsBytesUsed,              stats.lds_bytes_used_);
}

tinyxml2::XMLElement* KcXmlWriter::CreateTextElement(tinyxml2::XMLDocument& doc, const char* name, const std::string& text)
{
    assert(name != nullptr);
    if (!name || !name[0])
    {
        return nullptr;
    }

    tinyxml2::XMLElement* element = doc.NewElement(name);
    if (element)
    {
        element->SetText(text.c_str());
    }
    return element;
}

tinyxml2::XMLElement* KcXmlWriter::CreateIntElement(tinyxml2::XMLDocument& doc, const char* name, int value)
{
    assert(name != nullptr);
    if (!name || !name[0])
    {
        return nullptr;
    }

    tinyxml2::XMLElement* element = doc.NewElement(name);
    if (element)
    {
        element->SetText(value);
    }
    return element;
}

tinyxml2::XMLElement* KcXmlWriter::CreateUInt64Element(tinyxml2::XMLDocument& doc, const char* name, uint64_t value)
{
    assert(name != nullptr);
    if (!name || !name[0])
    {
        return nullptr;
    }

    tinyxml2::XMLElement* element = doc.NewElement(name);
    if (element)
    {
        element->SetText(value);
    }
    return element;
}

tinyxml2::XMLElement* KcXmlWriter::CreateBoolElement(tinyxml2::XMLDocument& doc, const char* name, bool value)
{
    assert(name != nullptr);
    if (!name || !name[0])
    {
        return nullptr;
    }

    tinyxml2::XMLElement* element = doc.NewElement(name);
    if (element)
    {
        element->SetText(value ? "true" : "false");
    }
    return element;
}

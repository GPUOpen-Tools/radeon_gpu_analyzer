//=================================================================
// Copyright 2020 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

// C++
#include <cassert>

// Local.
#include "RadeonGPUAnalyzerCLI/Src/kc_xml_writer.h"
#include "Utils/Include/rgaXMLConstants.h"
#include "Utils/Include/rgaVersionInfo.h"
#include "Utils/Include/rgaSharedUtils.h"
#include "RadeonGPUAnalyzerBackend/Src/be_utils.h"
#include "kc_cli_config_file.h"

// Static constants.
static const char* kStrFopenModeAppend = "a";

// Creates an element that has value of any primitive type.
template <typename T>
static void AppendXMLElement(tinyxml2::XMLDocument &xml_doc, tinyxml2::XMLElement* parent, const char* elem_name, T elem_value)
{
    tinyxml2::XMLElement* elem = xml_doc.NewElement(elem_name);
    elem->SetText(elem_value);
    parent->InsertEndChild(elem);
}

// Extract the CAL (generation) and code name.
// Example of "deviceName" format: "Baffin (Graphics IP v8)"
// Returned value: {"Graphics IP v8", "Baffin"}
static std::pair<std::string, std::string>
GetGenAndCodeNames(const std::string& device_name)
{
    size_t  code_name_offset = device_name.find('(');
    std::string  code_name = (code_name_offset != std::string::npos ? device_name.substr(0, code_name_offset - 1) : device_name);
    std::string  gen_name = (code_name_offset != std::string::npos ? device_name.substr(code_name_offset + 1, device_name.size() - code_name_offset - 2) : "");
    return { gen_name, code_name };
}

static bool AddSupportedGPUInfo(tinyxml2::XMLDocument& doc, tinyxml2::XMLElement& parent, const std::set<std::string>& targets)
{
    const char* kFilterIndicator1 = ":";
    const char* kFilterIndicator2 = "Not Used";
    bool ret = false;

    tinyxml2::XMLElement* supported_gpus = doc.NewElement(kXmlNodeSupportedGpus);

    // Add supported GPUS info.
    KcUtils::DeviceNameMap  cards_map;
    if ((ret = KcUtils::GetMarketingNameToCodenameMapping(cards_map)) == true)
    {
        // Sort the devices by Generation name.
        std::vector<std::pair<std::string, std::set<std::string>>> devices(cards_map.begin(), cards_map.end());
        std::sort(devices.begin(), devices.end(),
            [](const std::pair<std::string, std::set<std::string>>& d1, const std::pair<std::string, std::set<std::string>> &d2)
        {
            bool is_less_than = true;
            const char * kStrIpv6Ipv7Ipv8Token = "Graphics";
            const char * kStrRdnaToken = "RDNA";
            const char * kStrRdna2Token = "RDNA2";
            const char * kStrVegaToken = "Vega";
            auto gen_device_pair1 = GetGenAndCodeNames(d1.first);
            auto gen_device_pair2 = GetGenAndCodeNames(d2.first);

            size_t gfx_token_pos1 = gen_device_pair1.first.find(kStrIpv6Ipv7Ipv8Token);
            size_t gfx_token_pos2 = gen_device_pair2.first.find(kStrIpv6Ipv7Ipv8Token);
            size_t vega_token_pos1 = gen_device_pair1.first.find(kStrVegaToken);
            size_t vega_token_pos2 = gen_device_pair2.first.find(kStrVegaToken);
            size_t rdna_token_pos1 = gen_device_pair1.first.find(kStrRdnaToken);
            size_t rdna_token_pos2 = gen_device_pair2.first.find(kStrRdnaToken);
            size_t rdna2_token_pos1 = gen_device_pair1.first.find(kStrRdna2Token);
            size_t rdna2_token_pos2 = gen_device_pair2.first.find(kStrRdna2Token);

            if (rdna2_token_pos1 != std::string::npos && rdna2_token_pos2 == std::string::npos)
            {
                is_less_than = false;
            }
            else if (vega_token_pos2 != std::string::npos && rdna_token_pos1 != std::string::npos)
            {
                is_less_than = false;
            }
            else if (gfx_token_pos1 != std::string::npos &&
                gfx_token_pos2 != std::string::npos)
            {
                is_less_than = gen_device_pair1.first < gen_device_pair2.first;
            }
            else if (gfx_token_pos1 == std::string::npos && gfx_token_pos2 != std::string::npos)
            {
                is_less_than = false;
            }
            else if (!(vega_token_pos1 != std::string::npos && rdna_token_pos2 != std::string::npos))
            {
                is_less_than = GetGenAndCodeNames(d1.first).first < GetGenAndCodeNames(d2.first).first;
            }

            return is_less_than;
        });

        for (const auto& device : devices)
        {
            std::string  device_name = device.first, code_name, gen_name;
            std::tie(gen_name, code_name) = GetGenAndCodeNames(device_name);

            // Skip targets that are not supported in this mode.
            if (targets.count(KcUtils::ToLower(code_name)) == 0)
            {
                continue;
            }

            tinyxml2::XMLElement*  gpu = doc.NewElement(kXmlNodeGpu);
            tinyxml2::XMLElement*  gen = doc.NewElement(kXmlNodeGeneration);
            tinyxml2::XMLElement*  code_name_elem = doc.NewElement(kXmlNodeCodename);
            gen->SetText(gen_name.c_str());
            code_name_elem->SetText(code_name.c_str());
            gpu->LinkEndChild(gen);
            gpu->LinkEndChild(code_name_elem);

            // Add the list of marketing names or a placeholder if the list is empty.
            std::stringstream  marketing_names;
            bool  first = true;
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
                marketing_names << XML_UNKNOWN_MKT_NAME;
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

bool KcXmlWriter::AddVersionInfoGPUList(RgaMode mode, const std::set<std::string>& targets, const std::string& filename)
{
    std::string  mode_str;
    bool  ret = false;

    switch (mode)
    {
    case RgaMode::kModeRocmOpencl:  mode_str = XML_MODE_ROCM_CL; ret = true; break;
    case RgaMode::kModeVulkan:       mode_str = XML_MODE_VULKAN;  ret = true; break;
    }

    if (ret)
    {
        tinyxml2::XMLDocument  doc;
        tinyxml2::XMLElement  *name_elem, *mode_elem = doc.NewElement(XML_NODE_MODE);
        ret = ret && (mode_elem != nullptr);
        if (ret)
        {
            doc.LinkEndChild(mode_elem);
            name_elem = doc.NewElement(XML_NODE_NAME);
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
    bool  ret = false;
    tinyxml2::XMLDocument  doc;

    tinyxml2::XMLElement *adapters_elem = nullptr;
    tinyxml2::XMLElement *system_elem = doc.NewElement(XML_NODE_SYSTEM);
    ret = (system_elem != nullptr);
    if (ret)
    {
        doc.LinkEndChild(system_elem);
        adapters_elem = doc.NewElement(XML_NODE_ADAPTERS);
        if ((ret = (adapters_elem != nullptr)) == true)
        {
            system_elem->LinkEndChild(adapters_elem);
        }
    }

    // Add data for all physical adapters.
    for (auto adapter_info = info.cbegin(); adapter_info != info.cend() && ret; adapter_info++)
    {
        // <DisplayAdapter>
        tinyxml2::XMLElement *display_adapter = doc.NewElement(XML_NODE_ADAPTER);
        if ((ret = (display_adapter != nullptr)) == true)
        {
            adapters_elem->LinkEndChild(display_adapter);
        }

        // <ID>
        tinyxml2::XMLElement *id_elem = (ret ? doc.NewElement(XML_NODE_ID) : nullptr);
        if ((ret = (id_elem != nullptr)) == true)
        {
            id_elem->SetText(adapter_info->id);
            display_adapter->LinkEndChild(id_elem);
        }

        // <Name>
        tinyxml2::XMLElement *name_elem = (ret ? doc.NewElement(XML_NODE_NAME) : nullptr);
        if ((ret = (name_elem != nullptr)) == true)
        {
            name_elem->SetText(adapter_info->name.c_str());
            display_adapter->LinkEndChild(name_elem);
        }

        // <VulkanDriverVersion>
        tinyxml2::XMLElement *driver_version_elem = (ret ? doc.NewElement(XML_NODE_VK_DRIVER) : nullptr);
        if ((ret = (driver_version_elem != nullptr)) == true)
        {
            driver_version_elem->SetText(adapter_info->vk_driver_version.c_str());
            display_adapter->LinkEndChild(driver_version_elem);
        }

        // <VulkanAPIVersion>
        tinyxml2::XMLElement *vulkan_api_version_elem = (ret ? doc.NewElement(XML_NODE_VK_API) : nullptr);
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
    bool ret = true;
    tinyxml2::XMLDocument doc;

    // Add the RGA CLI version.
    tinyxml2::XMLElement* version_elem = doc.NewElement(kXmlNodeVersion);
    std::stringstream version_tag;
    version_tag << STR_RGA_VERSION << "." << STR_RGA_BUILD_NUM;
    version_elem->SetText(version_tag.str().c_str());
    doc.LinkEndChild(version_elem);

    // Add the RGA CLI build date.
    // First, reformat the Windows date string provided in format "Day dd/mm/yyyy" to format "yyyy-mm-dd".
    std::string  date_string = STR_RGA_BUILD_DATE;

#ifdef WIN32
    ret = rgaSharedUtils::ConvertDateString(date_string);
#endif

    tinyxml2::XMLElement*  build_date_elem = doc.NewElement(XML_NODE_BUILD_DATE);
    build_date_elem->SetText(date_string.c_str());
    doc.LinkEndChild(build_date_elem);

    ret = ret && WriteXMLDocToFile(doc, filename);

    return ret;
}

static bool AddOutputFile(tinyxml2::XMLDocument& doc, tinyxml2::XMLElement& parent,
                          const std::string& filename, const char* tag)
{
    bool ret = true;
    if (!filename.empty())
    {
        tinyxml2::XMLElement*  element = doc.NewElement(tag);
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

static bool AddEntryType(tinyxml2::XMLDocument& doc, tinyxml2::XMLElement* pEntry, RgEntryType rga_entry_type)
{
    bool ret = (pEntry != nullptr);
    tinyxml2::XMLElement* entry_type;
    if (ret)
    {
        entry_type = doc.NewElement(XML_NODE_TYPE);
        ret = ret && (entry_type != nullptr && pEntry->LinkEndChild(entry_type) != nullptr);
    }
    if (ret)
    {
        std::string  entry_type_str = "";
        switch (rga_entry_type)
        {
        case RgEntryType::kOpenclKernel:
            entry_type_str = XML_SHADER_OPENCL;
            break;
        case RgEntryType::kDxVertex:
            entry_type_str = XML_SHADER_DX_VERTEX;
            break;
        case RgEntryType::kDxHull:
            entry_type_str = XML_SHADER_DX_HULL;
            break;
        case RgEntryType::kDxDomain:
            entry_type_str = XML_SHADER_DX_DOMAIN;
            break;
        case RgEntryType::kDxGeometry:
            entry_type_str = XML_SHADER_DX_GEOMETRY;
            break;
        case RgEntryType::kDxPixel:
            entry_type_str = XML_SHADER_DX_PIXEL;
            break;
        case RgEntryType::kDxCompute:
            entry_type_str = XML_SHADER_DX_COMPUTE;
            break;
        case RgEntryType::kGlVertex:
            entry_type_str = XML_SHADER_GL_VERTEX;
            break;
        case RgEntryType::kGlTessControl:
            entry_type_str = XML_SHADER_GL_TESS_CTRL;
            break;
        case RgEntryType::kGlTessEval:
            entry_type_str = XML_SHADER_GL_TESS_EVAL;
            break;
        case RgEntryType::kGlGeometry:
            entry_type_str = XML_SHADER_GL_GEOMETRY;
            break;
        case RgEntryType::kGlFragment:
            entry_type_str = XML_SHADER_GL_FRAGMENT;
            break;
        case RgEntryType::kGlCompute:
            entry_type_str = XML_SHADER_GL_COMPUTE;
            break;
        case RgEntryType::kUnknown:
            entry_type_str = XML_SHADER_UNKNOWN;
            break;
        default:
            ret = false;
            break;
        }

        if (ret)
        {
            entry_type->SetText(entry_type_str.c_str());
        }
    }

    return ret;
}

static bool AddOutputFiles(tinyxml2::XMLDocument& doc, tinyxml2::XMLElement* entry,
    const std::string& target, const RgOutputFiles& out_files)
{
    bool  ret = false;
    tinyxml2::XMLElement* output = doc.NewElement(XML_NODE_OUTPUT);
    if (entry != nullptr && output != nullptr && entry->LinkEndChild(output) != nullptr)
    {
        // Add target GPU.
        tinyxml2::XMLElement* target_elem = doc.NewElement(XML_NODE_TARGET);
        ret = (target_elem != nullptr && output->LinkEndChild(target_elem) != nullptr);
        if (ret)
        {
            target_elem->SetText(target.c_str());
        }
        // Add output files.
        if (!out_files.is_isa_file_temp)
        {
            ret = ret && AddOutputFile(doc, *output, out_files.isa_file, XML_NODE_ISA);
        }
        ret = ret && AddOutputFile(doc, *output, out_files.isa_csv_file, XML_NODE_CSV_ISA);
        ret = ret && AddOutputFile(doc, *output, out_files.stats_file, XML_NODE_RES_USAGE);
        ret = ret && AddOutputFile(doc, *output, out_files.livereg_file, XML_NODE_LIVEREG);
        ret = ret && AddOutputFile(doc, *output, out_files.cfg_file, XML_NODE_CFG);
    }
    return ret;
}

bool KcXmlWriter::GenerateClSessionMetadataFile(const std::string& filename, const RgFileEntryData& file_entry_data,
    const RgClOutputMetadata& out_files)
{
    tinyxml2::XMLDocument  doc;
    std::string  current_device = "";

    tinyxml2::XMLElement* data_model_elem = doc.NewElement(XML_NODE_DATA_MODEL);
    bool  ret = (data_model_elem != nullptr && doc.LinkEndChild(data_model_elem) != nullptr);
    if (ret)
    {
        data_model_elem->SetText(STR_RGA_OUTPUT_MD_DATA_MODEL);
    }
    tinyxml2::XMLElement* metadata = doc.NewElement(XML_NODE_METADATA);
    ret = ret && (metadata != nullptr && doc.LinkEndChild(metadata) != nullptr);

    // Add binary name.
    if (!out_files.empty() && !out_files.begin()->second.is_bin_file_temp)
    {
        tinyxml2::XMLElement* binary_elem = doc.NewElement(XML_NODE_BINARY);
        ret = ret && (binary_elem != nullptr && metadata->LinkEndChild(binary_elem) != nullptr);
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
            out_files_map[kernel].push_back({ device, out_file_set.second });
        }

        // Now, try to find a source file for each entry in "outFilesMap" and fill the "metadataTable".
        // Split the "outFilesMap" into parts so that each part contains entries from the same source file.
        // If no source file is found for an entry, use "<Unknown>" source file name.
        for (auto& out_file_item : out_files_map)
        {
            const std::string&  entry_name = out_file_item.first;
            std::string  src_file_name;

            // Try to find a source file corresponding to this entry name.
            auto input_file_info = std::find_if(file_entry_data.begin(), file_entry_data.end(),
                [&](RgFileEntryData::const_reference entry_info)
            { for (auto entry : entry_info.second) { if (std::get<0>(entry) == entry_name) return true; } return false; });

            src_file_name = (input_file_info == file_entry_data.end() ? XML_UNKNOWN_SOURCE_FILE : input_file_info->first);
            metadata_table[src_file_name].insert(out_file_item);
        }

        // Store the "metadataTable" structure to the session metadata file.
        for (auto& input_file_data : metadata_table)
        {
            if (ret)
            {
                // Add input file info.
                tinyxml2::XMLElement* input_file = doc.NewElement(XML_NODE_INPUT_FILE);
                ret = ret && (input_file != nullptr && metadata->LinkEndChild(input_file) != nullptr);
                tinyxml2::XMLElement* input_file_path = doc.NewElement(XML_NODE_PATH);
                ret = ret && (input_file_path != nullptr && input_file->LinkEndChild(input_file_path) != nullptr);

                if (ret)
                {
                    input_file_path->SetText(input_file_data.first.c_str());

                    // Add entry points info.
                    for (auto& entry_data : input_file_data.second)
                    {
                        tinyxml2::XMLElement* entry = doc.NewElement(XML_NODE_ENTRY);
                        ret = (entry != nullptr && input_file->LinkEndChild(entry) != nullptr);
                        // Add entry name & type.
                        tinyxml2::XMLElement* name_elem = doc.NewElement(XML_NODE_NAME);
                        ret = ret && (name_elem != nullptr && entry->LinkEndChild(name_elem) != nullptr);
                        if (ret && entry_data.second.size() > 0)
                        {
                            name_elem->SetText(entry_data.first.c_str());
                            RgOutputFiles  outFileData = entry_data.second[0].second;
                            ret = ret && AddEntryType(doc, entry, outFileData.entry_type);
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

    ret = ret && (doc.SaveFile(filename.c_str()) == tinyxml2::XML_SUCCESS);

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
            tinyxml2::XMLElement* stage = doc.NewElement(XML_NODE_STAGE);
            if ((ret = (stage != nullptr && pipeline->LinkEndChild(stage))) == true)
            {
                ret = AddEntryType(doc, stage, out_files.entry_type);
            }

            // Add the input file.
            tinyxml2::XMLElement* input_file = doc.NewElement(XML_NODE_INPUT_FILE);
            ret = ret && (input_file != nullptr && stage->LinkEndChild(input_file) != nullptr);
            tinyxml2::XMLElement* input_file_path = doc.NewElement(XML_NODE_PATH);
            ret = ret && (input_file_path != nullptr && input_file->LinkEndChild(input_file_path) != nullptr);

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

bool KcXmlWriter::GenerateVulkanSessionMetadataFile(const std::string& filename,
    const std::map<std::string, RgVkOutputMetadata>& output_metadata)
{
    tinyxml2::XMLDocument doc;
    std::string  current_device = "";

    tinyxml2::XMLElement* data_model_elem = doc.NewElement(XML_NODE_DATA_MODEL);
    bool  ret = (data_model_elem != nullptr && doc.LinkEndChild(data_model_elem) != nullptr);
    if (ret)
    {
        data_model_elem->SetText(STR_RGA_OUTPUT_MD_DATA_MODEL);
    }

    tinyxml2::XMLElement* metadata_elem = doc.NewElement(XML_NODE_METADATA_PIPELINE);
    ret = ret && (metadata_elem != nullptr && doc.LinkEndChild(metadata_elem) != nullptr);

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
            tinyxml2::XMLElement* pipeline = doc.NewElement(XML_NODE_PIPELINE);
            ret = ret && (pipeline != nullptr && metadata_elem->LinkEndChild(pipeline) != nullptr);
            if (ret)
            {
                tinyxml2::XMLElement* pipeline_type = doc.NewElement(XML_NODE_TYPE);
                if ((ret = (pipeline_type != nullptr && pipeline->LinkEndChild(pipeline_type))) == true)
                {
                    bool is_compute = out_files_for_device[0].entry_type == RgEntryType::kGlCompute;
                    pipeline_type->SetText(is_compute ? XML_PIPELINE_TYPE_COMPUTE : XML_PIPELINE_TYPE_GRAPHICS);
                }
            }

            // Add the pipeline stages.
            ret = ret && AddVulkanPipelineStages(doc, pipeline, out_files_for_device);
        }
    }

    ret = ret && (doc.SaveFile(filename.c_str()) == tinyxml2::XML_SUCCESS);

    return ret;
}

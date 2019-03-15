//=================================================================
// Copyright 2018 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

// C++
#include <cassert>

// Local
#include <RadeonGPUAnalyzerCLI/Src/kcXmlWriter.h>
#include <Utils/Include/rgaXMLConstants.h>
#include <Utils/Include/rgaVersionInfo.h>
#include <Utils/Include/rgaSharedUtils.h>

// Static constants.
static const char*     STR_FOPEN_MODE_APPEND = "a";

// Creates an element that has value of any primitive type.
template <typename T>
static void AppendXMLElement(tinyxml2::XMLDocument &xmlDoc, tinyxml2::XMLElement* pParent, const char* pElemName, T elemValue)
{
    tinyxml2::XMLElement* pElem = xmlDoc.NewElement(pElemName);
    pElem->SetText(elemValue);
    pParent->InsertEndChild(pElem);
}

// Extract the CAL (generation) and code name.
// Example of "deviceName" format: "Baffin (Graphics IP v8)"
// Returned value: {"Graphics IP v8", "Baffin"}
static std::pair<std::string, std::string>
GetGenAndCodeNames(const std::string& deviceName)
{
    size_t  codeNameOffset = deviceName.find('(');
    std::string  codeName = (codeNameOffset != std::string::npos ? deviceName.substr(0, codeNameOffset - 1) : deviceName);
    std::string  genName = (codeNameOffset != std::string::npos ? deviceName.substr(codeNameOffset + 1, deviceName.size() - codeNameOffset - 2) : "");
    return { genName, codeName };
}

static bool AddSupportedGPUInfo(tinyxml2::XMLDocument& doc, tinyxml2::XMLElement& parent, const std::set<std::string>& targets)
{
    const char* FILTER_INDICATOR_1 = ":";
    const char* FILTER_INDICATOR_2 = "Not Used";
    bool  ret;

    tinyxml2::XMLElement*  pSupportedGPUs = doc.NewElement(XML_NODE_SUPPORTED_GPUS);

    // Add supported GPUS info.
    kcUtils::DeviceNameMap  cardsMap;
    if ((ret = kcUtils::GetMarketingNameToCodenameMapping(cardsMap)) == true)
    {
        // Sort the devices by Generation name.
        std::vector<std::pair<std::string, std::set<std::string>>>  devices(cardsMap.begin(), cardsMap.end());
        std::sort(devices.begin(), devices.end(),
            [](const std::pair<std::string, std::set<std::string>>& d1, const std::pair<std::string, std::set<std::string>> &d2)
        { return (GetGenAndCodeNames(d1.first).first < GetGenAndCodeNames(d2.first).first); });

        for (const auto& device : devices)
        {
            std::string  deviceName = device.first, codeName, genName;
            std::tie(genName, codeName) = GetGenAndCodeNames(deviceName);

            // Skip targets that are not supported in this mode.
            if (targets.count(kcUtils::ToLower(codeName)) == 0)
            {
                continue;
            }

            tinyxml2::XMLElement*  pGPU = doc.NewElement(XML_NODE_GPU);
            tinyxml2::XMLElement*  pGen = doc.NewElement(XML_NODE_GENERATION);
            tinyxml2::XMLElement*  pCodeName = doc.NewElement(XML_NODE_CODENAME);
            pGen->SetText(genName.c_str());
            pCodeName->SetText(codeName.c_str());
            pGPU->LinkEndChild(pGen);
            pGPU->LinkEndChild(pCodeName);

            // Add the list of marketing names or a placeholder if the list is empty.
            std::stringstream  mktNames;
            bool  first = true;

            for (const std::string& mktName : device.second)
            {
                if (mktName.find(FILTER_INDICATOR_1) == std::string::npos && mktName.find(FILTER_INDICATOR_2) == std::string::npos)
                {
                    mktNames << (first ? "" : ", ") << mktName;
                    first = false;
                }
            }

            if (mktNames.str().empty())
            {
                mktNames << XML_UNKNOWN_MKT_NAME;
            }

            tinyxml2::XMLElement*  pPublicNames = doc.NewElement(XML_NODE_PRODUCT_NAMES);
            pPublicNames->SetText(mktNames.str().c_str());
            pGPU->LinkEndChild(pPublicNames);
            pSupportedGPUs->LinkEndChild(pGPU);
        }
    }

    if (ret)
    {
        parent.LinkEndChild(pSupportedGPUs);
    }

    return ret;
}

//
// Write tinyxml2 document to the XML file specified by "fileName".
// The document will be appended to the existing content of file.
// If "fileName" is empty, the document will be dumped to stdout.
//
static bool WriteXMLDocToFile(tinyxml2::XMLDocument& doc, const std::string& fileName)
{
    bool result = false;

    if (fileName.empty())
    {
        doc.Print();
        result = true;
    }
    else
    {
        std::FILE* xmlFile = std::fopen(fileName.c_str(), STR_FOPEN_MODE_APPEND);
        assert(xmlFile != nullptr);
        if (xmlFile != nullptr)
        {
            tinyxml2::XMLPrinter printer(xmlFile);
            doc.Print(&printer);
            std::fclose(xmlFile);
            result = true;
        }
    }

    return result;
}

bool kcXmlWriter::AddVersionInfoGPUList(RgaMode mode, const std::set<std::string>& targets, const std::string& fileName)
{
    std::string  modeStr;
    bool  ret = false;

    switch (mode)
    {
    case RgaMode::Mode_Rocm_OpenCL:  modeStr = XML_MODE_ROCM_CL; ret = true; break;
    case RgaMode::Mode_Vulkan:       modeStr = XML_MODE_VULKAN;  ret = true; break;
    }

    if (ret)
    {
        tinyxml2::XMLDocument  doc;

        tinyxml2::XMLElement  *pNameElem, *pModeElem = doc.NewElement(XML_NODE_MODE);
        ret = ret && (pModeElem != nullptr);
        if (ret)
        {
            doc.LinkEndChild(pModeElem);
            pNameElem = doc.NewElement(XML_NODE_NAME);
            if ((ret = (pNameElem != nullptr)) == true)
            {
                pModeElem->LinkEndChild(pNameElem);
                pNameElem->SetText(modeStr.c_str());
            }
        }

        ret = ret && AddSupportedGPUInfo(doc, *pModeElem, targets);
        ret = ret && WriteXMLDocToFile(doc, fileName);
    }

    return ret;
}

bool kcXmlWriter::AddVersionInfoSystemData(const std::vector<beVkPhysAdapterInfo>& info, const std::string& fileName)
{
    bool  ret = false;
    tinyxml2::XMLDocument  doc;

    tinyxml2::XMLElement *pAdapters = nullptr;
    tinyxml2::XMLElement *pSystem = doc.NewElement(XML_NODE_SYSTEM);
    ret = (pSystem != nullptr);
    if (ret)
    {
        doc.LinkEndChild(pSystem);
        pAdapters = doc.NewElement(XML_NODE_ADAPTERS);
        if ((ret = (pAdapters != nullptr)) == true)
        {
            pSystem->LinkEndChild(pAdapters);
        }
    }

    // Add data for all physical adapters.
    for (auto adapterInfo = info.cbegin(); adapterInfo != info.cend() && ret; adapterInfo++)
    {
        // <DisplayAdapter>
        tinyxml2::XMLElement *pAdapter = doc.NewElement(XML_NODE_ADAPTER);
        if ((ret = (pAdapter != nullptr)) == true)
        {
            pAdapters->LinkEndChild(pAdapter);
        }

        // <ID>
        tinyxml2::XMLElement *pID = (ret ? doc.NewElement(XML_NODE_ID) : nullptr);
        if ((ret = (pID != nullptr)) == true)
        {
            pID->SetText(adapterInfo->id);
            pAdapter->LinkEndChild(pID);
        }

        // <Name>
        tinyxml2::XMLElement *pName = (ret ? doc.NewElement(XML_NODE_NAME) : nullptr);
        if ((ret = (pName != nullptr)) == true)
        {
            pName->SetText(adapterInfo->name.c_str());
            pAdapter->LinkEndChild(pName);
        }

        // <VulkanDriverVersion>
        tinyxml2::XMLElement *pDriverVersion = (ret ? doc.NewElement(XML_NODE_VK_DRIVER) : nullptr);
        if ((ret = (pDriverVersion != nullptr)) == true)
        {
            pDriverVersion->SetText(adapterInfo->vkDriverVersion.c_str());
            pAdapter->LinkEndChild(pDriverVersion);
        }

        // <VulkanAPIVersion>
        tinyxml2::XMLElement *pVkAPIVersion = (ret ? doc.NewElement(XML_NODE_VK_API) : nullptr);
        if ((ret = (pVkAPIVersion != nullptr)) == true)
        {
            pVkAPIVersion->SetText(adapterInfo->vkAPIVersion.c_str());
            pAdapter->LinkEndChild(pVkAPIVersion);
        }
    }

    ret = ret && WriteXMLDocToFile(doc, fileName);

    return ret;
}

bool kcXmlWriter::AddVersionInfoHeader(const std::string& fileName)
{
    bool  ret = true;
    tinyxml2::XMLDocument  doc;

    // Add the RGA CLI version.
    tinyxml2::XMLElement*  pVersionElem = doc.NewElement(XML_NODE_VERSION);
    std::stringstream  versionTag;
    versionTag << STR_RGA_VERSION << "." << STR_RGA_BUILD_NUM;
    pVersionElem->SetText(versionTag.str().c_str());
    doc.LinkEndChild(pVersionElem);

    // Add the RGA CLI build date.
    // First, reformat the Windows date string provided in format "Day dd/mm/yyyy" to format "yyyy-mm-dd".
    std::string  dateString = STR_RGA_BUILD_DATE;

#ifdef WIN32
    ret = rgaSharedUtils::ConvertDateString(dateString);
#endif

    tinyxml2::XMLElement*  pBuildDateElem = doc.NewElement(XML_NODE_BUILD_DATE);
    pBuildDateElem->SetText(dateString.c_str());
    doc.LinkEndChild(pBuildDateElem);

    ret = ret && WriteXMLDocToFile(doc, fileName);

    return ret;
}

static bool AddOutputFile(tinyxml2::XMLDocument& doc, tinyxml2::XMLElement& parent,
                          const std::string& fileName, const char* tag)
{
    bool  ret = true;
    if (!fileName.empty())
    {
        tinyxml2::XMLElement*  pElement = doc.NewElement(tag);
        if (pElement != nullptr)
        {
            pElement->SetText(fileName.c_str());
            ret = (parent.LinkEndChild(pElement) != nullptr);
        }
        else
        {
            ret = false;
        }
    }
    return ret;
}

static bool AddEntryType(tinyxml2::XMLDocument& doc, tinyxml2::XMLElement* pEntry, rgEntryType entryType)
{
    bool  ret = (pEntry != nullptr);
    tinyxml2::XMLElement*  pEntryType;
    if (ret)
    {
        pEntryType = doc.NewElement(XML_NODE_TYPE);
        ret = ret && (pEntryType != nullptr && pEntry->LinkEndChild(pEntryType) != nullptr);
    }
    if (ret)
    {
        std::string  entryTypeStr = "";
        switch (entryType)
        {
        case rgEntryType::OpenCL_Kernel:
            entryTypeStr = XML_SHADER_OPENCL;
            break;
        case rgEntryType::DX_Vertex:
            entryTypeStr = XML_SHADER_DX_VERTEX;
            break;
        case rgEntryType::DX_Hull:
            entryTypeStr = XML_SHADER_DX_HULL;
            break;
        case rgEntryType::DX_Domain:
            entryTypeStr = XML_SHADER_DX_DOMAIN;
            break;
        case rgEntryType::DX_Geometry:
            entryTypeStr = XML_SHADER_DX_GEOMETRY;
            break;
        case rgEntryType::DX_Pixel:
            entryTypeStr = XML_SHADER_DX_PIXEL;
            break;
        case rgEntryType::DX_Compute:
            entryTypeStr = XML_SHADER_DX_COMPUTE;
            break;
        case rgEntryType::GL_Vertex:
            entryTypeStr = XML_SHADER_GL_VERTEX;
            break;
        case rgEntryType::GL_TessControl:
            entryTypeStr = XML_SHADER_GL_TESS_CTRL;
            break;
        case rgEntryType::GL_TessEval:
            entryTypeStr = XML_SHADER_GL_TESS_EVAL;
            break;
        case rgEntryType::GL_Geometry:
            entryTypeStr = XML_SHADER_GL_GEOMETRY;
            break;
        case rgEntryType::GL_Fragment:
            entryTypeStr = XML_SHADER_GL_FRAGMENT;
            break;
        case rgEntryType::GL_Compute:
            entryTypeStr = XML_SHADER_GL_COMPUTE;
            break;
        case rgEntryType::Unknown:
            entryTypeStr = XML_SHADER_UNKNOWN;
            break;
        default:
            ret = false;
            break;
        }

        if (ret)
        {
            pEntryType->SetText(entryTypeStr.c_str());
        }
    }

    return ret;
}

static bool AddOutputFiles(tinyxml2::XMLDocument& doc, tinyxml2::XMLElement* pEntry,
    const std::string& target, const rgOutputFiles& outFiles)
{
    bool  ret = false;
    tinyxml2::XMLElement* pOutput = doc.NewElement(XML_NODE_OUTPUT);
    if (pEntry != nullptr && pOutput != nullptr && pEntry->LinkEndChild(pOutput) != nullptr)
    {
        // Add target GPU.
        tinyxml2::XMLElement* pTarget = doc.NewElement(XML_NODE_TARGET);
        ret = (pTarget != nullptr && pOutput->LinkEndChild(pTarget) != nullptr);
        if (ret)
        {
            pTarget->SetText(target.c_str());
        }
        // Add output files.
        if (!outFiles.m_isIsaFileTemp)
        {
            ret = ret && AddOutputFile(doc, *pOutput, outFiles.m_isaFile, XML_NODE_ISA);
        }
        ret = ret && AddOutputFile(doc, *pOutput, outFiles.m_isaCsvFile, XML_NODE_CSV_ISA);
        ret = ret && AddOutputFile(doc, *pOutput, outFiles.m_statFile, XML_NODE_RES_USAGE);
        ret = ret && AddOutputFile(doc, *pOutput, outFiles.m_liveregFile, XML_NODE_LIVEREG);
        ret = ret && AddOutputFile(doc, *pOutput, outFiles.m_cfgFile, XML_NODE_CFG);
    }
    return ret;
}

bool kcXmlWriter::GenerateClSessionMetadataFile(const std::string& fileName, const rgFileEntryData& fileEntryData,
    const rgCLOutputMetadata& outFiles)
{
    tinyxml2::XMLDocument  doc;
    std::string  currentDevice = "";

    tinyxml2::XMLElement* pDataModelElem = doc.NewElement(XML_NODE_DATA_MODEL);
    bool  ret = (pDataModelElem != nullptr && doc.LinkEndChild(pDataModelElem) != nullptr);
    if (ret)
    {
        pDataModelElem->SetText(STR_RGA_OUTPUT_MD_DATA_MODEL);
    }
    tinyxml2::XMLElement* pMetadata = doc.NewElement(XML_NODE_METADATA);
    ret = ret && (pMetadata != nullptr && doc.LinkEndChild(pMetadata) != nullptr);

    // Add binary name.
    if (!outFiles.empty() && !outFiles.begin()->second.m_isBinFileTemp)
    {
        tinyxml2::XMLElement* pBinary = doc.NewElement(XML_NODE_BINARY);
        ret = ret && (pBinary != nullptr && pMetadata->LinkEndChild(pBinary) != nullptr);
        if (ret)
        {
            pBinary->SetText((outFiles.begin())->second.m_binFile.c_str());
        }
    }

    if (ret)
    {
        // Map: kernel_name --> vector{pair{device, out_files}}.
        std::map<std::string, std::vector<std::pair<std::string, rgOutputFiles>>>  outFilesMap;

        // Map: input_file_name --> outFilesMap.
        std::map<std::string, decltype(outFilesMap)>  metadataTable;

        // Reorder the output file metadata in "kernel-first" order.
        for (const auto& outFileSet : outFiles)
        {
            const std::string& device = outFileSet.first.first;
            const std::string& kernel = outFileSet.first.second;

            outFilesMap[kernel].push_back({ device, outFileSet.second });
        }

        // Now, try to find a source file for each entry in "outFilesMap" and fill the "metadataTable".
        // Split the "outFilesMap" into parts so that each part contains entries from the same source file.
        // If no source file is found for an entry, use "<Unknown>" source file name.
        for (auto& outFileItem : outFilesMap)
        {
            const std::string&  entryName = outFileItem.first;
            std::string  srcFileName;

            // Try to find a source file corresponding to this entry name.
            auto inputFileInfo = std::find_if(fileEntryData.begin(), fileEntryData.end(),
                [&](rgFileEntryData::const_reference entryInfo)
            { for (auto entry : entryInfo.second) { if (std::get<0>(entry) == entryName) return true; } return false; });

            srcFileName = (inputFileInfo == fileEntryData.end() ? XML_UNKNOWN_SOURCE_FILE : inputFileInfo->first);
            metadataTable[srcFileName].insert(outFileItem);
        }

        // Store the "metadataTable" structure to the session metadata file.
        for (auto& inputFileData : metadataTable)
        {
            if (ret)
            {
                // Add input file info.
                tinyxml2::XMLElement* pInputFile = doc.NewElement(XML_NODE_INPUT_FILE);
                ret = ret && (pInputFile != nullptr && pMetadata->LinkEndChild(pInputFile) != nullptr);
                tinyxml2::XMLElement* pInputFilePath = doc.NewElement(XML_NODE_PATH);
                ret = ret && (pInputFilePath != nullptr && pInputFile->LinkEndChild(pInputFilePath) != nullptr);

                if (ret)
                {
                    pInputFilePath->SetText(inputFileData.first.c_str());

                    // Add entry points info.
                    for (auto& entryData : inputFileData.second)
                    {
                        tinyxml2::XMLElement* pEntry = doc.NewElement(XML_NODE_ENTRY);
                        ret = (pEntry != nullptr && pInputFile->LinkEndChild(pEntry) != nullptr);
                        // Add entry name & type.
                        tinyxml2::XMLElement* pName = doc.NewElement(XML_NODE_NAME);
                        ret = ret && (pName != nullptr && pEntry->LinkEndChild(pName) != nullptr);
                        if (ret && entryData.second.size() > 0)
                        {
                            pName->SetText(entryData.first.c_str());
                            rgOutputFiles  outFileData = entryData.second[0].second;
                            ret = ret && AddEntryType(doc, pEntry, outFileData.m_entryType);
                        }
                        // Add "Output" nodes.
                        for (const std::pair<std::string, rgOutputFiles>& deviceAndOutFiles : entryData.second)
                        {
                            ret = ret && AddOutputFiles(doc, pEntry, deviceAndOutFiles.first, deviceAndOutFiles.second);
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

    ret = ret && (doc.SaveFile(fileName.c_str()) == tinyxml2::XML_SUCCESS);

    return ret;
}

// Add the per-stage session data to the pipeline data.
static bool AddVulkanPipelineStages(tinyxml2::XMLDocument& doc, tinyxml2::XMLElement* pPipeline, const rgVkOutputMetadata& outputMD)
{
    bool ret = false;

    // For each per-stage data.
    for (const rgOutputFiles& outFiles : outputMD)
    {
        if (!outFiles.m_inputFile.empty())
        {
            tinyxml2::XMLElement* pStage = doc.NewElement(XML_NODE_STAGE);
            if ((ret = (pStage != nullptr && pPipeline->LinkEndChild(pStage))) == true)
            {
                ret = AddEntryType(doc, pStage, outFiles.m_entryType);
            }

            // Add the input file.
            tinyxml2::XMLElement* pInputFile = doc.NewElement(XML_NODE_INPUT_FILE);
            ret = ret && (pInputFile != nullptr && pStage->LinkEndChild(pInputFile) != nullptr);
            tinyxml2::XMLElement* pInputFilePath = doc.NewElement(XML_NODE_PATH);
            ret = ret && (pInputFilePath != nullptr && pInputFile->LinkEndChild(pInputFilePath) != nullptr);

            assert(ret);
            if (ret)
            {
                pInputFilePath->SetText(outFiles.m_inputFile.c_str());
            }

            // Add the output file data.
            ret = ret && AddOutputFiles(doc, pStage, outFiles.m_device, outFiles);
        }
    }

    return ret;
}

bool kcXmlWriter::GenerateVulkanSessionMetadataFile(const std::string& fileName,
    const std::map<std::string, rgVkOutputMetadata>& outputMD)
{
    tinyxml2::XMLDocument  doc;
    std::string  currentDevice = "";

    tinyxml2::XMLElement* pDataModelElem = doc.NewElement(XML_NODE_DATA_MODEL);
    bool  ret = (pDataModelElem != nullptr && doc.LinkEndChild(pDataModelElem) != nullptr);
    if (ret)
    {
        pDataModelElem->SetText(STR_RGA_OUTPUT_MD_DATA_MODEL);
    }

    tinyxml2::XMLElement* pMetadata = doc.NewElement(XML_NODE_METADATA_PIPELINE);
    ret = ret && (pMetadata != nullptr && doc.LinkEndChild(pMetadata) != nullptr);

    if (ret)
    {
        // For each per-device data.
        for (const auto& outputMDForDevice : outputMD)
        {
            const rgVkOutputMetadata& outFilesForDevice = outputMDForDevice.second;
            if (outFilesForDevice.empty())
            {
                continue;
            }

            // Add the "pipeline" tag and the pipeline type.
            tinyxml2::XMLElement* pPipeline = doc.NewElement(XML_NODE_PIPELINE);
            ret = ret && (pPipeline != nullptr && pMetadata->LinkEndChild(pPipeline) != nullptr);
            if (ret)
            {
                tinyxml2::XMLElement* pPipelineType = doc.NewElement(XML_NODE_TYPE);
                if ((ret = (pPipelineType != nullptr && pPipeline->LinkEndChild(pPipelineType))) == true)
                {
                    bool isCompute = outFilesForDevice[0].m_entryType == rgEntryType::GL_Compute;
                    pPipelineType->SetText(isCompute ? XML_PIPELINE_TYPE_COMPUTE : XML_PIPELINE_TYPE_GRAPHICS);
                }
            }

            // Add the pipeline stages.
            ret = ret && AddVulkanPipelineStages(doc, pPipeline, outFilesForDevice);
        }
    }

    ret = ret && (doc.SaveFile(fileName.c_str()) == tinyxml2::XML_SUCCESS);

    return ret;
}

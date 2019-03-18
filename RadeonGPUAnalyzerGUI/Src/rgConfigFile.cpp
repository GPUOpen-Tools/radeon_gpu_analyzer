// C++.
#include <cassert>
#include <sstream>
#include <functional>

// XML.
#include <tinyxml2/Include/tinyxml2.h>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/rgConfigFileOpenCL.h>
#include <RadeonGPUAnalyzerGUI/Include/rgConfigFileVulkan.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDataTypesOpenCL.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDataTypesVulkan.h>
#include <RadeonGPUAnalyzerGUI/Include/rgFactory.h>
#include <RadeonGPUAnalyzerGUI/Include/rgStringConstants.h>
#include <RadeonGPUAnalyzerGUI/Include/rgConfigFile.h>
#include <RadeonGPUAnalyzerGUI/Include/rgUtils.h>
#include <RadeonGPUAnalyzerGUI/Include/rgConfigFileDefinitions.h>
#include <RadeonGPUAnalyzerGUI/Include/rgXMLUtils.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDataTypes.h>

// *** INTERNALLY-LINKED AUXILIARY FUNCTIONS - BEGIN ***

// Read v2.0 of the DefaultBuildSettings XML node from the config file.
static bool ExtractDefaultBuildSettings_2_0(tinyxml2::XMLNode* pDefaultBuildSettingsNode, std::shared_ptr<rgGlobalSettings>& pGlobalSettings);

// Read v2.1 of the DefaultBuildSettings XML node from the config file.
static bool ExtractDefaultBuildSettings_2_1(tinyxml2::XMLNode* pDefaultBuildSettingsNode, std::shared_ptr<rgGlobalSettings>& pGlobalSettings);

// Read v2.0 of the GlobalSettings XML node from the config file.
static bool ExtractGlobalSettings_2_0(tinyxml2::XMLNode* pGlobalSettingsNode, std::shared_ptr<rgGlobalSettings>& pGlobalSettings);

// Read v2.1 of the GlobalSettings XML node from the config file.
static bool ExtractGlobalSettings_2_1(tinyxml2::XMLNode* pGlobalSettingsNode, std::shared_ptr<rgGlobalSettings>& pGlobalSettings);

// Takes the comma-separated target devices list from the GUI, and returns the list of GPUs in it.
static void ExtractTargetGpus(const std::string& targetDevices, std::vector<std::string>& gpuList)
{
    rgUtils::splitString(targetDevices, rgConfigManager::RGA_LIST_DELIMITER, gpuList);
}

// Takes the comma-separated predefined macros list from the GUI, and returns the list of macros in it.
static void ExtractPredefinedMacros(const std::string& predefinedMacros, std::vector<std::string>& macroList)
{
    rgUtils::splitString(predefinedMacros, rgConfigManager::RGA_LIST_DELIMITER, macroList);
}

// Takes the comma-separated directory list from the GUI, and returns the list of directories in it.
static void ExtractAdditionalIncludeDirectories(const std::string& additionalIncludeDirectories, std::vector<std::string>& dirList)
{
    rgUtils::splitString(additionalIncludeDirectories, rgConfigManager::RGA_LIST_DELIMITER, dirList);
}

// Takes the comma-separated list of disassembly columns, and returns the list of the columns in it.
static void ExtractDisassemblyColumns(const std::string& disassemblyColumns, std::vector<bool>& columnIndices)
{
    std::vector<std::string> splitTokens;
    rgUtils::splitString(disassemblyColumns, rgConfigManager::RGA_LIST_DELIMITER, splitTokens);

    for (const std::string& token : splitTokens)
    {
        int index = std::stoi(token);
        columnIndices.push_back(index == 1);
    }
}

static bool ExtractRecentProjects(tinyxml2::XMLNode* pNode, std::vector<std::shared_ptr<rgRecentProject>>& recentProjects)
{
    bool ret = false;

    assert(pNode != nullptr);
    if (pNode != nullptr)
    {
        // Find the first project in the list of recent projects.
        pNode = pNode->FirstChildElement(XML_NODE_GLOBAL_RECENT_PROJECT_ROOT);

        // If there aren't any recent project paths, return early, as there's nothing to parse.
        if (pNode == nullptr)
        {
            ret = true;
        }
        else
        {
            // Step over each path to a recent project, and add it to the output list.
            while (pNode != nullptr)
            {
                tinyxml2::XMLNode* pChildNode = pNode->FirstChildElement(XML_NODE_GLOBAL_RECENT_PROJECT_PATH);
                assert(pChildNode != nullptr);
                if (pChildNode != nullptr)
                {
                    // Read the project path element.
                    std::string projectPath;
                    ret = rgXMLUtils::ReadNodeTextString(pChildNode, projectPath);

                    assert(ret);
                    if (ret)
                    {
                        auto pRecentProject = std::make_shared<rgRecentProject>();
                        pRecentProject->projectPath = projectPath;

                        // Get the project api element.
                        pChildNode = pChildNode->NextSiblingElement(XML_NODE_GLOBAL_RECENT_PROJECT_API);

                        assert(pChildNode != nullptr);
                        if (pChildNode != nullptr)
                        {
                            // Read the project api element.
                            std::string projectAPI;
                            ret = rgXMLUtils::ReadNodeTextString(pChildNode, projectAPI);

                            assert(ret);
                            if (ret)
                            {
                                pRecentProject->apiType = rgUtils::ProjectAPIToEnum(projectAPI);
                            }
                        }
                        recentProjects.push_back(pRecentProject);
                    }
                }

                pNode = pNode->NextSiblingElement(XML_NODE_GLOBAL_RECENT_PROJECT_ROOT);
            }
        }
    }
    return ret;
}

static bool ExtractFontInformation(tinyxml2::XMLNode* pNode, std::shared_ptr<rgGlobalSettings>& pGlobalSettings)
{
    bool ret = false;

    assert(pNode != nullptr);
    if (pNode != nullptr)
    {
        // Get the font family element.
        tinyxml2::XMLNode* pChildNode = pNode->FirstChildElement(XML_NODE_GLOBAL_FONT_FAMILY_TYPE);
        assert(pChildNode != nullptr);
        if (pChildNode != nullptr)
        {
            // Read font family element.
            assert(pGlobalSettings != nullptr);
            if (pGlobalSettings != nullptr)
            {
                ret = rgXMLUtils::ReadNodeTextString(pChildNode, pGlobalSettings->m_fontFamily);
                assert(ret);
                if (ret)
                {
                    // Get the font size element.
                    pChildNode = pChildNode->NextSiblingElement(XML_NODE_GLOBAL_FONT_SIZE);
                    assert(pChildNode != nullptr);
                    if (pChildNode != nullptr)
                    {
                        // Read the font size element.
                        unsigned fontSize;
                        ret = rgXMLUtils::ReadNodeTextUnsigned(pChildNode, fontSize);
                        assert(ret);
                        if (ret)
                        {
                            pGlobalSettings->m_fontSize = fontSize;
                        }
                    }
                }

                // Read the include files viewer.
                ret = rgXMLUtils::ReadNodeTextString(pChildNode, pGlobalSettings->m_includeFilesViewer);
                assert(ret);

                if (!ret)
                {
                    // If we could not read the include files viewer, that's OK. Use the system's default.
                    ret = true;
                    pGlobalSettings->m_includeFilesViewer = STR_GLOBAL_SETTINGS_SRC_VIEW_INCLUDE_VIEWER_DEFAULT;
                }
            }
        }
    }
    return ret;
}

static bool ExtractSplitterConfigs(tinyxml2::XMLNode* pNode, std::vector<rgSplitterConfig>& splitterConfigs)
{
    bool ret = false;

    assert(pNode != nullptr);
    if (pNode != nullptr)
    {
        // Get "Layout" node.
        pNode = pNode->FirstChildElement(XML_NODE_GLOBAL_GUI_LAYOUT);

        if (pNode != nullptr)
        {
            // Get "Splitter" node.
            pNode = pNode->FirstChildElement(XML_NODE_GLOBAL_GUI_SPLITTER);

            if (pNode == nullptr)
            {
                // this is a valid scenario if the config file was saved before the user opened a project, which means the splitter values may not have been defined yet.
                ret = true;
            }
            else
            {
                while (pNode != nullptr)
                {
                    std::string splitterName;
                    std::string splitterValues;

                    // Get splitter name.
                    tinyxml2::XMLNode* pSplitterNode = pNode->FirstChildElement(XML_NODE_GLOBAL_GUI_SPLITTER_NAME);
                    ret = rgXMLUtils::ReadNodeTextString(pSplitterNode, splitterName);

                    if (!ret)
                    {
                        break;
                    }

                    // Get splitter values.
                    pSplitterNode = pNode->FirstChildElement(XML_NODE_GLOBAL_GUI_SPLITTER_VALUES);
                    ret = rgXMLUtils::ReadNodeTextString(pSplitterNode, splitterValues);

                    if (!ret)
                    {
                        break;
                    }

                    std::vector<std::string> valueStringList;
                    std::vector<int> valueIntList;

                    // Create int list of splitter values from comma separated string.
                    rgUtils::splitString(splitterValues, rgConfigManager::RGA_LIST_DELIMITER, valueStringList);
                    for (std::string strValue : valueStringList)
                    {
                        valueIntList.push_back(stoi(strValue));
                    }

                    // Create splitter config object.
                    rgSplitterConfig splitterConfig;
                    splitterConfig.m_splitterName = splitterName;
                    splitterConfig.m_splitterValues = valueIntList;
                    splitterConfigs.push_back(splitterConfig);

                    // Get next "Splitter" node.
                    pNode = pNode->NextSiblingElement(XML_NODE_GLOBAL_GUI_SPLITTER);
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
bool rgXmlConfigFileReaderImpl::ReadGeneralBuildSettings(tinyxml2::XMLNode* pNode, std::shared_ptr<rgBuildSettings> pBuildSettings)
{
    bool ret = false;
    if (pNode != nullptr)
    {
        pNode = pNode->FirstChildElement(XML_NODE_TARGET_DEVICES);
        ret = (pNode != nullptr);
        assert(ret);
        if (ret)
        {
            std::string targetDevices;
            bool hasTargetDevices = rgXMLUtils::ReadNodeTextString(pNode, targetDevices);
            if (hasTargetDevices)
            {
                // Target GPUs.
                ExtractTargetGpus(targetDevices, pBuildSettings->m_targetGpus);
                ret = (!pBuildSettings->m_targetGpus.empty());
                assert(ret);
            }

            if (ret)
            {
                // Predefined macros.
                pNode = pNode->NextSiblingElement(XML_NODE_PREDEFINED_MACROS);
                ret = (pNode != nullptr);
                assert(ret);

                if (ret)
                {
                    std::string predefinedMacros;
                    bool shouldRead = rgXMLUtils::ReadNodeTextString(pNode, predefinedMacros);

                    if (shouldRead)
                    {
                        ExtractPredefinedMacros(predefinedMacros, pBuildSettings->m_predefinedMacros);
                    }

                    // Additional include directories.
                    pNode = pNode->NextSiblingElement(XML_NODE_ADDITIONAL_INCLUDE_DIRECTORIES);
                    ret = (pNode != nullptr);
                    assert(ret);

                    if (ret)
                    {
                        std::string additionalIncludeDirectories;
                        shouldRead = rgXMLUtils::ReadNodeTextString(pNode, additionalIncludeDirectories);
                        if (shouldRead)
                        {
                            ExtractAdditionalIncludeDirectories(additionalIncludeDirectories, pBuildSettings->m_additionalIncludeDirectories);
                        }

                        // Read additional build options element.
                        pNode = pNode->NextSiblingElement(XML_NODE_ADDITIONAL_OPTIONS);
                        ret = (pNode != nullptr);
                        assert(ret);
                        if (pNode != nullptr)
                        {
                            rgXMLUtils::ReadNodeTextString(pNode, pBuildSettings->m_additionalOptions);
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
class rgXMLConfigFileReaderImplFactory
{
public:
    static std::shared_ptr<rgXmlConfigFileReaderImpl> CreateReader(const std::string& apiName)
    {
        std::shared_ptr<rgXmlConfigFileReaderImpl> pRet = nullptr;
        if (apiName.compare(STR_API_NAME_OPENCL) == 0)
        {
            pRet = std::make_shared<rgConfigFileReaderOpenCL>();
        }
        else if (apiName.compare(STR_API_NAME_VULKAN) == 0)
        {
            pRet = std::make_shared<rgConfigFileReaderVulkan>();
        }
        return pRet;
    }
};

// Support for reading v2.0 of the project files
static bool ReadProjectConfigFile_2_0(tinyxml2::XMLDocument& doc, const char* pFileDataModelVersion, tinyxml2::XMLNode* pProjectNode, std::shared_ptr<rgProject>& pProject)
{
    bool ret = false;
    if (pProjectNode != nullptr)
    {
        bool isProjectNode = (std::string(XML_NODE_PROJECT).compare(pProjectNode->Value()) == 0);
        assert(isProjectNode);
        ret = isProjectNode;
        if (ret)
        {
            // Get the relevant API name.
            tinyxml2::XMLNode* pNode = pProjectNode->FirstChild();
            bool isProgramApiNode = (pNode != nullptr) && (std::string(XML_NODE_API_NAME).compare(pNode->Value()) == 0);
            assert(isProgramApiNode);
            ret = isProgramApiNode;
            if (ret)
            {
                std::string apiName;
                ret = rgXMLUtils::ReadNodeTextString(pNode, apiName);

                // Create the concrete config file reader for the relevant API.
                std::shared_ptr<rgXmlConfigFileReaderImpl> pReader = rgXMLConfigFileReaderImplFactory::CreateReader(apiName);
                if (pReader != nullptr)
                {
                    // Let the concrete parser handle the configuration.
                    ret = pReader->ReadProjectConfigFile(doc, pFileDataModelVersion, pProject);
                    assert(ret && pProject != nullptr);
                }
            }
        }
    }

    return ret;
}

bool rgXmlConfigFile::ReadProjectConfigFile(const std::string& configFilePath, std::shared_ptr<rgProject>& pProject)
{
    bool ret = false;

    // Reset the output variable.
    pProject = nullptr;

    // Load the XML document.
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError rc = doc.LoadFile(configFilePath.c_str());

    if (rc == tinyxml2::XML_SUCCESS)
    {
        // Get the XML declaration node.
        tinyxml2::XMLNode* pNode = doc.FirstChild();
        if (pNode != nullptr)
        {
            // Get the RGA Data Model version item.
            pNode = pNode->NextSibling();
            if (pNode != nullptr)
            {
                tinyxml2::XMLElement* pElem = pNode->ToElement();
                if (pElem != nullptr)
                {
                    // Get the data model version in order to verify
                    // that the project file is compatible.
                    const char* pDataModelVersion = pNode->ToElement()->GetText();

                    // All v2.0 and v2.1 project files have the same format
                    // for the initial <Program> and <ProgramAPI> tags.
                    if (RGA_DATA_MODEL_2_0.compare(pDataModelVersion) == 0 ||
                        RGA_DATA_MODEL_2_1.compare(pDataModelVersion) == 0)
                    {
                        // Skip to the <Program> node.
                        pNode = pNode->NextSibling();
                        ret = ReadProjectConfigFile_2_0(doc, pDataModelVersion, pNode, pProject);
                    }
                    else
                    {
                        assert(!"RGA Data Model version is not supported");
                        ret = false;
                    }

                    if (ret)
                    {
                        // Set the project file path.
                        pProject->m_projectFileFullPath = configFilePath;
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

bool rgXmlConfigFileWriterImpl::WriteGeneralBuildSettings(const std::shared_ptr<rgBuildSettings> pBuildSettings, tinyxml2::XMLDocument& doc, tinyxml2::XMLElement* pBuildSettingsElement)
{
    bool ret = false;

    assert(pBuildSettings != nullptr);
    assert(pBuildSettingsElement != nullptr);
    if (pBuildSettings != nullptr && pBuildSettingsElement != nullptr)
    {
        // Target devices.
        rgXMLUtils::AppendXMLElement(doc, pBuildSettingsElement, XML_NODE_TARGET_DEVICES, rgUtils::BuildSemicolonSeparatedStringList(pBuildSettings->m_targetGpus).c_str());

        // Predefined Macros.
        rgXMLUtils::AppendXMLElement(doc, pBuildSettingsElement, XML_NODE_PREDEFINED_MACROS, rgUtils::BuildSemicolonSeparatedStringList(pBuildSettings->m_predefinedMacros).c_str());

        // Additional include directories.
        rgXMLUtils::AppendXMLElement(doc, pBuildSettingsElement, XML_NODE_ADDITIONAL_INCLUDE_DIRECTORIES, rgUtils::BuildSemicolonSeparatedStringList(pBuildSettings->m_additionalIncludeDirectories).c_str());

        // Additional options.
        rgXMLUtils::AppendXMLElement(doc, pBuildSettingsElement, XML_NODE_ADDITIONAL_OPTIONS, pBuildSettings->m_additionalOptions.c_str());

        ret = true;
    }

    return ret;
}

void rgXmlConfigFileWriterImpl::AddConfigFileDeclaration(tinyxml2::XMLDocument& doc)
{
    tinyxml2::XMLNode* pNode = doc.NewDeclaration(RGA_XML_DECLARATION);
    doc.InsertEndChild(pNode);

    // Create the Data Model Version element.
    tinyxml2::XMLElement* pElement = doc.NewElement(XML_NODE_DATA_MODEL_VERSION);
    pElement->SetText(RGA_DATA_MODEL.c_str());
    doc.LinkEndChild(pElement);
}

bool rgXmlGraphicsConfigFileReaderImpl::ReadPipeline(tinyxml2::XMLDocument& doc, tinyxml2::XMLNode* pParentClone,
                                                     bool isBackupSpv, rgPipelineShaders& pipeline) const
{
    bool ret = false;

    assert(pParentClone != nullptr);
    if (pParentClone != nullptr)
    {
        tinyxml2::XMLElement* pPipelineElement = pParentClone->FirstChildElement(isBackupSpv ? XML_NODE_BACKUP_SPV_ROOT : XML_NODE_PIPELINE_SHADERS_ROOT);
        ret = (isBackupSpv || pPipelineElement != nullptr);
        assert(ret);

        if (pPipelineElement != nullptr)
        {
            tinyxml2::XMLNode* pPipelineTypeNode = pPipelineElement->FirstChildElement(XML_NODE_PIPELINE_TYPE);
            assert(pPipelineTypeNode != nullptr);
            if (pPipelineTypeNode != nullptr)
            {
                // Read the pipeline type node.
                std::string pipelineType;
                ret = rgXMLUtils::ReadNodeTextString(pPipelineTypeNode, pipelineType);

                // Lambda that implements reading a single pipeline file path element.
                auto ReadPipelineShaderFile = [&](rgPipelineStage stage, const char* tag, std::function<bool(rgPipelineStage, const std::string&, rgPipelineShaders&)> f)
                {
                    std::string shaderFullFilePath;
                    tinyxml2::XMLNode* pNode = pPipelineTypeNode->NextSiblingElement(tag);
                    if (pNode != nullptr)
                    {
                        rgXMLUtils::ReadNodeTextString(pNode, shaderFullFilePath);
                        f(stage, shaderFullFilePath, pipeline);
                    }
                };

                if (ret && (pipelineType.compare(XML_NODE_PIPELINE_TYPE_GRAPHICS) == 0))
                {
                    // Create a new graphics pipeline object.
                    pipeline.m_type = rgPipelineType::Graphics;

                    // Read the paths to shader input files.
                    ReadPipelineShaderFile(rgPipelineStage::Vertex,                 XML_NODE_PIPELINE_VERTEX_STAGE,       rgUtils::SetStageShaderPath);
                    ReadPipelineShaderFile(rgPipelineStage::TessellationControl,    XML_NODE_PIPELINE_TESS_CONTROL_STAGE, rgUtils::SetStageShaderPath);
                    ReadPipelineShaderFile(rgPipelineStage::TessellationEvaluation, XML_NODE_PIPELINE_TESS_EVAL_STAGE,    rgUtils::SetStageShaderPath);
                    ReadPipelineShaderFile(rgPipelineStage::Geometry,               XML_NODE_PIPELINE_GEOMETRY_STAGE,     rgUtils::SetStageShaderPath);
                    ReadPipelineShaderFile(rgPipelineStage::Fragment,               XML_NODE_PIPELINE_FRAGMENT_STAGE,     rgUtils::SetStageShaderPath);
                }
                else if (ret && (pipelineType.compare(XML_NODE_PIPELINE_TYPE_COMPUTE) == 0))
                {
                    // Create a new compute pipeline object.
                    pipeline.m_type = rgPipelineType::Compute;

                    // Read the compute shader input file.
                    ReadPipelineShaderFile(rgPipelineStage::Compute, XML_NODE_PIPELINE_COMPUTE_STAGE, rgUtils::SetStageShaderPath);
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

bool rgXmlGraphicsConfigFileReaderImpl::ReadPipelineState(std::shared_ptr<rgGraphicsProjectClone> pClone, tinyxml2::XMLDocument& doc, tinyxml2::XMLNode* pPipelineStateElement) const
{
    bool ret = false;

    // Find the project clone's pipeline state root element.
    assert(pPipelineStateElement != nullptr);
    if (pPipelineStateElement != nullptr)
    {
        // Find the first pipeline state element.
        tinyxml2::XMLNode* pPipelineStateNode = pPipelineStateElement->FirstChildElement(XML_NODE_PIPELINE_STATE);

        if (pPipelineStateNode != nullptr)
        {
            // Loop over each state node to read the pipeline properties.
            do
            {
                if (pPipelineStateNode != nullptr)
                {
                    rgPipelineState state;

                    // Read the PSO name.
                    tinyxml2::XMLNode* pPsoNameNode = pPipelineStateNode->FirstChildElement(XML_NODE_PIPELINE_NAME);
                    rgXMLUtils::ReadNodeTextString(pPsoNameNode, state.m_name);

                    // Read the "is active" flag.
                    tinyxml2::XMLNode* pIsActiveNode = pPipelineStateNode->FirstChildElement(XML_NODE_PIPELINE_IS_ACTIVE);
                    rgXMLUtils::ReadNodeTextBool(pIsActiveNode, state.m_isActive);

                    // Read the pipeline state file path.
                    tinyxml2::XMLNode* pPipelineStateFilePathNode = pPipelineStateNode->FirstChildElement(XML_NODE_PIPELINE_STATE_FILE_PATH);
                    rgXMLUtils::ReadNodeTextString(pPipelineStateFilePathNode, state.m_pipelineStateFilePath);

                    // Read the original pipeline state file path.
                    tinyxml2::XMLNode* pOrigPipelineStateFilePathNode = pPipelineStateNode->FirstChildElement(XML_NODE_ORIGINAL_PIPELINE_STATE_FILE_PATH);
                    rgXMLUtils::ReadNodeTextString(pOrigPipelineStateFilePathNode, state.m_originalPipelineStateFilePath);

                    pClone->m_psoStates.push_back(state);
                }

                // Try to move on to processing the next state node.
                pPipelineStateNode = pPipelineStateNode->NextSiblingElement(XML_NODE_PIPELINE_STATE);

            } while (pPipelineStateNode != nullptr);
        }

        ret = true;
    }

    return ret;
}

bool rgXmlGraphicsConfigFileWriterImpl::WritePipeline(tinyxml2::XMLDocument& doc, tinyxml2::XMLElement* pParentClone,
                                                      bool isBackupSpv, const rgPipelineShaders& pipeline) const
{
    bool ret = false;

    assert(pParentClone != nullptr);
    if (pParentClone != nullptr)
    {
        // Create the project clone's pipeline root element.
        tinyxml2::XMLElement* pPipelineNode = doc.NewElement(isBackupSpv ? XML_NODE_BACKUP_SPV_ROOT : XML_NODE_PIPELINE_SHADERS_ROOT);

        assert(pPipelineNode != nullptr);
        if (pPipelineNode != nullptr)
        {
            bool isGraphicsPipeline = pipeline.m_type == rgPipelineType::Graphics;
            const char* pPipelineType = isGraphicsPipeline ? XML_NODE_PIPELINE_TYPE_GRAPHICS : XML_NODE_PIPELINE_TYPE_COMPUTE;
            rgXMLUtils::AppendXMLElement(doc, pPipelineNode, XML_NODE_PIPELINE_TYPE, pPipelineType);

            // Lambda that implements writing a single pipeline file path element.
            auto AppendPipelineShaderFile = [&](rgPipelineStage stage, const char* tag, std::function<bool(const rgPipelineShaders&, rgPipelineStage, std::string&)> f)
            {
                std::string shaderPath;
                if (f(pipeline, stage, shaderPath))
                {
                    rgXMLUtils::AppendXMLElement(doc, pPipelineNode, tag, shaderPath.c_str());
                }
            };

            if (isGraphicsPipeline)
            {

                // Append each graphics pipeline shader stage's input file.
                AppendPipelineShaderFile(rgPipelineStage::Vertex,                 XML_NODE_PIPELINE_VERTEX_STAGE,       rgUtils::GetStageShaderPath);
                AppendPipelineShaderFile(rgPipelineStage::TessellationControl,    XML_NODE_PIPELINE_TESS_CONTROL_STAGE, rgUtils::GetStageShaderPath);
                AppendPipelineShaderFile(rgPipelineStage::TessellationEvaluation, XML_NODE_PIPELINE_TESS_EVAL_STAGE,    rgUtils::GetStageShaderPath);
                AppendPipelineShaderFile(rgPipelineStage::Geometry,               XML_NODE_PIPELINE_GEOMETRY_STAGE,     rgUtils::GetStageShaderPath);
                AppendPipelineShaderFile(rgPipelineStage::Fragment,               XML_NODE_PIPELINE_FRAGMENT_STAGE,     rgUtils::GetStageShaderPath);
            }
            else
            {
                // Append the pipeline's compute shader input file.
                AppendPipelineShaderFile(rgPipelineStage::Compute, XML_NODE_PIPELINE_COMPUTE_STAGE,            rgUtils::GetStageShaderPath);
            }

            // Insert the pipeline element into the parent clone element.
            pParentClone->InsertEndChild(pPipelineNode);
            ret = true;
        }
    }

    return ret;
}

bool rgXmlGraphicsConfigFileWriterImpl::WritePipelineState(const std::shared_ptr<rgGraphicsProjectClone> pClone, tinyxml2::XMLDocument& doc, tinyxml2::XMLElement* pParentCloneElement) const
{
    bool ret = false;

    assert(pClone != nullptr);
    assert(pParentCloneElement != nullptr);
    if (pClone != nullptr && pParentCloneElement != nullptr)
    {
        tinyxml2::XMLElement* pPipelineStateRoot = doc.NewElement(XML_NODE_PIPELINE_STATE_ROOT);

        assert(pPipelineStateRoot != nullptr);
        if (pPipelineStateRoot != nullptr)
        {
            // Loop over each pipeline state in the clone.
            for (auto pipelineState : pClone->m_psoStates)
            {
                // Create a new element for writing each state info.
                tinyxml2::XMLElement* pPipelineStateElement = doc.NewElement(XML_NODE_PIPELINE_STATE);

                // Append the state name, the active flag, and the pipeline state file path data.
                rgXMLUtils::AppendXMLElement(doc, pPipelineStateElement, XML_NODE_PIPELINE_NAME, pipelineState.m_name.c_str());
                rgXMLUtils::AppendXMLElement(doc, pPipelineStateElement, XML_NODE_PIPELINE_IS_ACTIVE, pipelineState.m_isActive);
                rgXMLUtils::AppendXMLElement(doc, pPipelineStateElement, XML_NODE_PIPELINE_STATE_FILE_PATH, pipelineState.m_pipelineStateFilePath.data());
                rgXMLUtils::AppendXMLElement(doc, pPipelineStateElement, XML_NODE_ORIGINAL_PIPELINE_STATE_FILE_PATH, pipelineState.m_originalPipelineStateFilePath.data());

                // Insert the state into the list of pipeline states.
                pPipelineStateRoot->InsertEndChild(pPipelineStateElement);
            }

            // Insert the pipeline state element in the project clone.
            pParentCloneElement->InsertEndChild(pPipelineStateRoot);

            ret = true;
        }
    }

    return ret;
}

class ConfigFileWriterFactory
{
public:
    static std::shared_ptr<rgXmlConfigFileWriterImpl> CreateWriter(rgProjectAPI api)
    {
        std::shared_ptr<rgXmlConfigFileWriterImpl> pRet = nullptr;
        switch (api)
        {
        case rgProjectAPI::OpenCL:
            pRet = std::make_shared<rgConfigFileWriterOpenCL>();
            break;
        case rgProjectAPI::Vulkan:
            pRet = std::make_shared<rgConfigFileWriterVulkan>();
            break;
        case rgProjectAPI::Unknown:
        default:
            // If we got here, there's a problem because the API type is unrecognized.
            assert(false);
            break;
        }
        return pRet;
    }
};

bool rgXmlConfigFile::WriteProjectConfigFile(const rgProject& project, const std::string& configFilePath)
{
    bool ret = false;

    std::shared_ptr<rgXmlConfigFileWriterImpl> pWriter = ConfigFileWriterFactory::CreateWriter(project.m_api);
    if (pWriter != nullptr)
    {
        ret = pWriter->WriteProjectConfigFile(project, configFilePath);
    }

    return ret;
}

// *************************
// *** WRITER AREA - END ***
// *************************

// **************************
// Global configuration file.
// **************************
bool rgXmlConfigFile::ReadGlobalSettings(const std::string& globalConfigFilePath, std::shared_ptr<rgGlobalSettings>& pGlobalSettings)
{
    bool ret = false;

    // Reset the output variable.
    pGlobalSettings = std::make_shared<rgGlobalSettings>();

    // Load the XML document.
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError rc = doc.LoadFile(globalConfigFilePath.c_str());

    if (rc == tinyxml2::XML_SUCCESS)
    {
        // Get the XML declaration node.
        tinyxml2::XMLNode* pNode = doc.FirstChild();
        if (pNode != nullptr)
        {
            // Get the RGA Data Model version element.
            pNode = pNode->NextSibling();
            if (pNode != nullptr)
            {
                tinyxml2::XMLElement* pElem = pNode->ToElement();
                if (pElem != nullptr)
                {
                    // Determine which data model version to load.
                    const char* pDataModelVersion = pNode->ToElement()->GetText();
                    if (RGA_DATA_MODEL_2_0.compare(pDataModelVersion) == 0)
                    {
                        // Get next sibling, which should be the GlobalSettings element
                        tinyxml2::XMLNode* pGlobalSettingsNode = pNode->NextSibling();
                        ret = ExtractGlobalSettings_2_0(pGlobalSettingsNode, pGlobalSettings);
                    }
                    else if (RGA_DATA_MODEL_2_1.compare(pDataModelVersion) == 0)
                    {
                        // Get Next sibling, which should be the GlobalSettings element
                        tinyxml2::XMLNode* pGlobalSettingsNode = pNode->NextSibling();
                        ret = ExtractGlobalSettings_2_1(pGlobalSettingsNode, pGlobalSettings);
                    }
                    else
                    {
                        // Data model version is not supported.
                        ret = false;
                    }
                }

                // Null xml element indicates a read failure at some point.
                if (pElem == nullptr)
                {
                    ret = false;
                }
            }
        }
    }

    return ret;
}

bool rgXmlConfigFile::WriteGlobalSettings(std::shared_ptr<rgGlobalSettings> pGlobalSettings, const std::string& globalConfigFilePath)
{
    assert(pGlobalSettings != nullptr);

    bool ret = false;

    if (pGlobalSettings != nullptr)
    {
        // Create the XML declaration node.
        tinyxml2::XMLDocument doc;
        rgXmlConfigFileWriterImpl::AddConfigFileDeclaration(doc);

        // Create the Global Settings element.
        tinyxml2::XMLElement* pGlobalSettingsElem = doc.NewElement(XML_NODE_GLOBAL_LOG_FILE_GLOBAL_SETTINGS);
        assert(pGlobalSettingsElem != nullptr);
        if (pGlobalSettingsElem != nullptr)
        {
            // Create the log file location element.
            rgXMLUtils::AppendXMLElement(doc, pGlobalSettingsElem, XML_NODE_GLOBAL_LOG_FILE_LOCATION, pGlobalSettings->m_logFileLocation.c_str());

            // Create the last selected directory element.
            rgXMLUtils::AppendXMLElement(doc, pGlobalSettingsElem, XML_NODE_GLOBAL_LAST_SELECTED_DIRECTORY, pGlobalSettings->m_lastSelectedDirectory.c_str());

            // Create a root node for the list of recent projects.
            tinyxml2::XMLElement* pRecentProjectsElem = doc.NewElement(XML_NODE_GLOBAL_RECENT_PROJECTS_ROOT);
            assert(pRecentProjectsElem != nullptr);
            if (pRecentProjectsElem != nullptr)
            {
                if (pGlobalSettings->m_recentProjects.size() > 0)
                {
                    for (size_t projectIndex = 0; projectIndex < pGlobalSettings->m_recentProjects.size(); ++projectIndex)
                    {
                        tinyxml2::XMLElement* pRecentProjectElem = doc.NewElement(XML_NODE_GLOBAL_RECENT_PROJECT_ROOT);
                        if (pRecentProjectElem != nullptr)
                        {
                            // Save the most recent files, starting at the end of the list.
                            assert(pGlobalSettings->m_recentProjects[projectIndex] != nullptr);
                            if (pGlobalSettings->m_recentProjects[projectIndex] != nullptr)
                            {
                                const std::string& projectPath = pGlobalSettings->m_recentProjects[projectIndex]->projectPath;

                                // Write the project path into the recent project element.
                                rgXMLUtils::AppendXMLElement(doc, pRecentProjectElem, XML_NODE_GLOBAL_RECENT_PROJECT_PATH, projectPath.c_str());

                                // Save the project api type.
                                std::string apiType;
                                bool ok = rgUtils::ProjectAPIToString(pGlobalSettings->m_recentProjects[projectIndex]->apiType, apiType);
                                assert(ok);
                                if (ok)
                                {
                                    // Write the project api type into the recent project element.
                                    rgXMLUtils::AppendXMLElement(doc, pRecentProjectElem, XML_NODE_GLOBAL_RECENT_PROJECT_API, apiType.c_str());
                                }
                                pRecentProjectsElem->InsertEndChild(pRecentProjectElem);
                            }
                        }
                    }
                }

                // Add the list of recent projects to the global settings node.
                pGlobalSettingsElem->InsertEndChild(pRecentProjectsElem);
            }

            // Create a root node for the font type and size.
            tinyxml2::XMLElement* pFontFamilyElement = doc.NewElement(XML_NODE_GLOBAL_FONT_FAMILY_ROOT);
            assert(pFontFamilyElement != nullptr);
            if (pFontFamilyElement != nullptr)
            {
                // Write the font family into the font element.
                rgXMLUtils::AppendXMLElement(doc, pFontFamilyElement, XML_NODE_GLOBAL_FONT_FAMILY_TYPE, pGlobalSettings->m_fontFamily.c_str());

                // Write the font size into the font element.
                rgXMLUtils::AppendXMLElement(doc, pFontFamilyElement, XML_NODE_GLOBAL_FONT_SIZE, pGlobalSettings->m_fontSize);

                // Add the font element to the global settings node.
                pGlobalSettingsElem->InsertEndChild(pFontFamilyElement);
            }

            // Write the include files viewer.
            rgXMLUtils::AppendXMLElement(doc, pGlobalSettingsElem, XML_NODE_GLOBAL_INCLUDE_FILES_VIEWER, pGlobalSettings->m_includeFilesViewer.c_str());

            // Create a comma-separated list from the column names that we have.
            std::string disassemblyColumnStr = rgUtils::BuildSemicolonSeparatedBoolList(pGlobalSettings->m_visibleDisassemblyViewColumns);
            rgXMLUtils::AppendXMLElement(doc, pGlobalSettingsElem, XML_NODE_GLOBAL_DISASSEMBLY_COLUMNS, disassemblyColumnStr.c_str());

            // An element used to determine if project names will be generated or provided by the user.
            rgXMLUtils::AppendXMLElement(doc, pGlobalSettingsElem, XML_NODE_USE_GENERATED_PROJECT_NAMES, pGlobalSettings->m_useDefaultProjectName);

            rgXMLUtils::AppendXMLElement(doc, pGlobalSettingsElem, XML_NODE_GLOBAL_DEFAULT_API, static_cast<int>(pGlobalSettings->m_defaultAPI));

            rgXMLUtils::AppendXMLElement(doc, pGlobalSettingsElem, XML_NODE_GLOBAL_PROMPT_FOR_API, pGlobalSettings->m_shouldPromptForAPI);

            rgXMLUtils::AppendXMLElement(doc, pGlobalSettingsElem, XML_NODE_GLOBAL_INPUT_FILE_EXT_GLSL, pGlobalSettings->m_inputFileExtGlsl.c_str());
            rgXMLUtils::AppendXMLElement(doc, pGlobalSettingsElem, XML_NODE_GLOBAL_INPUT_FILE_EXT_HLSL, pGlobalSettings->m_inputFileExtHlsl.c_str());
            rgXMLUtils::AppendXMLElement(doc, pGlobalSettingsElem, XML_NODE_GLOBAL_INPUT_FILE_EXT_SPV_TXT, pGlobalSettings->m_inputFileExtSpvTxt.c_str());
            rgXMLUtils::AppendXMLElement(doc, pGlobalSettingsElem, XML_NODE_GLOBAL_INPUT_FILE_EXT_SPV_BIN, pGlobalSettings->m_inputFileExtSpvBin.c_str());

            rgXMLUtils::AppendXMLElement(doc, pGlobalSettingsElem, XML_NODE_GLOBAL_DEFAULT_SRC_LANG, (uint32_t)pGlobalSettings->m_defaultLang);

            // Create "GUI" element.
            tinyxml2::XMLElement* pGuiElement = doc.NewElement(XML_NODE_GLOBAL_GUI);
            assert(pGuiElement != nullptr);
            if (pGuiElement != nullptr)
            {
                // Create "Layout" element.
                tinyxml2::XMLElement* pLayoutElement = doc.NewElement(XML_NODE_GLOBAL_GUI_LAYOUT);
                assert(pLayoutElement != nullptr);
                if (pLayoutElement != nullptr)
                {
                    if (pGlobalSettings->m_guiLayoutSplitters.size() > 0)
                    {
                        for (rgSplitterConfig splitterConfig : pGlobalSettings->m_guiLayoutSplitters)
                        {
                            // Create "Splitter" element.
                            tinyxml2::XMLElement* pSplitterElement = doc.NewElement(XML_NODE_GLOBAL_GUI_SPLITTER);
                            assert(pSplitterElement != nullptr);
                            if (pSplitterElement != nullptr)
                            {
                                // Add "SplitterName" element.
                                rgXMLUtils::AppendXMLElement(doc, pSplitterElement, XML_NODE_GLOBAL_GUI_SPLITTER_NAME, splitterConfig.m_splitterName.c_str());

                                // Add "SplitterValues" element.
                                std::string splitterValuesStr = rgUtils::BuildSemicolonSeparatedIntList(splitterConfig.m_splitterValues);
                                rgXMLUtils::AppendXMLElement(doc, pSplitterElement, XML_NODE_GLOBAL_GUI_SPLITTER_VALUES, splitterValuesStr.c_str());

                                // End of "Splitter" element.
                                pLayoutElement->InsertEndChild(pSplitterElement);
                            }
                        }
                    }

                    // End of "Layout" element.
                    pGuiElement->InsertEndChild(pLayoutElement);
                }

                // End of "GUI" element.
                pGlobalSettingsElem->InsertEndChild(pGuiElement);
            }

            // Add the Global Settings element to the document.
            doc.InsertEndChild(pGlobalSettingsElem);

            // Create the Default Build Settings element.
            tinyxml2::XMLElement* pDefaultBuildSettings = doc.NewElement(XML_NODE_GLOBAL_DEFAULT_BUILD_SETTINGS);
            if (pDefaultBuildSettings != nullptr)
            {
                // Loop through each API type, and use an API-specific rgConfigFileWriter to write the default settings.
                for (int apiIndex = static_cast<int>(rgProjectAPI::OpenCL); apiIndex < static_cast<int>(rgProjectAPI::ApiCount); ++apiIndex)
                {
                    rgProjectAPI currentApi = static_cast<rgProjectAPI>(apiIndex);

                    std::string apiString;
                    bool isOk = rgUtils::ProjectAPIToString(currentApi, apiString);
                    assert(isOk);
                    if (isOk)
                    {
                        std::shared_ptr<rgXmlConfigFileWriterImpl> pWriter = ConfigFileWriterFactory::CreateWriter(currentApi);
                        assert(pWriter != nullptr);
                        if (pWriter != nullptr)
                        {
                            // Add the new API element.
                            tinyxml2::XMLElement* pApiDefaultSettings = doc.NewElement(apiString.c_str());

                            // Create the Build Settings element for the API.
                            tinyxml2::XMLElement* pBuildSettingsNode = doc.NewElement(XML_NODE_BUILD_SETTINGS);
                            assert(pBuildSettingsNode != nullptr);
                            if (pBuildSettingsNode != nullptr)
                            {
                                // Write the API-specific default build settings element.
                                std::shared_ptr<rgBuildSettings> pDefaultApiBuildSettings = pGlobalSettings->m_pDefaultBuildSettings[apiString];
                                assert(pDefaultApiBuildSettings != nullptr);
                                if (pDefaultApiBuildSettings != nullptr)
                                {
                                    ret = pWriter->WriteBuildSettingsElement(pDefaultApiBuildSettings, doc, pBuildSettingsNode);
                                }

                                // Insert the build settings node into the API parent node.
                                pApiDefaultSettings->InsertEndChild(pBuildSettingsNode);
                            }

                            // Insert the OpenCL-specific default build settings element to the document.
                            pDefaultBuildSettings->InsertEndChild(pApiDefaultSettings);
                        }
                    }
                }

                // Insert the Default Build Settings element to the document.
                doc.InsertEndChild(pDefaultBuildSettings);

                // Save the configuration file.
                tinyxml2::XMLError rc = doc.SaveFile(globalConfigFilePath.c_str());
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

static bool ExtractDefaultBuildSettings_2_0(tinyxml2::XMLNode* pDefaultBuildSettingsNode, std::shared_ptr<rgGlobalSettings>& pGlobalSettings)
{
    assert(pDefaultBuildSettingsNode != nullptr);
    assert(pGlobalSettings != nullptr);

    // Make sure this is actually the DefaultBuildSettings node
    bool ret = ((pGlobalSettings != nullptr) &&
                (pDefaultBuildSettingsNode != nullptr) &&
                (std::strcmp(pDefaultBuildSettingsNode->Value(), XML_NODE_GLOBAL_DEFAULT_BUILD_SETTINGS) == 0));

    if (ret)
    {
        // Get the OpenCL default build settings element.
        tinyxml2::XMLNode* pOpenclNode = pDefaultBuildSettingsNode->FirstChildElement(STR_API_NAME_OPENCL);
        if (pOpenclNode != nullptr)
        {
            tinyxml2::XMLElement* pElem = pOpenclNode->FirstChildElement(XML_NODE_BUILD_SETTINGS);
            assert(pElem != nullptr);
            ret = pElem != nullptr;
            if (ret)
            {
                // Use the API-specific factory to create an API-specific build settings object.
                std::shared_ptr<rgFactory> pApiFactory = rgFactory::CreateFactory(rgProjectAPI::OpenCL);
                assert(pApiFactory != nullptr);
                ret = pApiFactory != nullptr;
                if (pApiFactory != nullptr)
                {
                    std::shared_ptr<rgBuildSettings> pApiBuildSettings = pApiFactory->CreateBuildSettings(nullptr);
                    assert(pApiBuildSettings != nullptr);
                    ret = pApiBuildSettings != nullptr;
                    if (pApiBuildSettings != nullptr)
                    {
                        // Create an API-specific build settings instance and parser object to read the settings.
                        std::shared_ptr<rgXmlConfigFileReaderImpl> pReader = rgXMLConfigFileReaderImplFactory::CreateReader(STR_API_NAME_OPENCL);
                        assert(pReader != nullptr);
                        ret = pReader != nullptr;
                        if (pReader != nullptr)
                        {
                            tinyxml2::XMLNode* pBuildSettingsNode = pOpenclNode->FirstChildElement(XML_NODE_BUILD_SETTINGS);
                            assert(pBuildSettingsNode != nullptr);
                            ret = pBuildSettingsNode != nullptr;
                            if (pBuildSettingsNode != nullptr)
                            {
                                // Extract the build settings common to all APIs.
                                ret = pReader->ReadGeneralBuildSettings(pBuildSettingsNode, pApiBuildSettings);
                                assert(ret);

                                // Extract the OpenCL default build settings.
                                ret = pReader->ReadApiBuildSettings(pBuildSettingsNode, pApiBuildSettings);
                                assert(ret);

                                // Store the default OpenCL build settings in our data object.
                                pGlobalSettings->m_pDefaultBuildSettings[STR_API_NAME_OPENCL] = pApiBuildSettings;
                            }
                        }
                    }
                }
            }
        }
    }

    return ret;
}

static bool ExtractDefaultBuildSettings_2_1(tinyxml2::XMLNode* pDefaultBuildSettingsNode, std::shared_ptr<rgGlobalSettings>& pGlobalSettings)
{
    assert(pDefaultBuildSettingsNode != nullptr);
    assert(pGlobalSettings != nullptr);

    // Make sure this is actually the DefaultBuildSettings node
    bool ret = ((pGlobalSettings != nullptr) &&
        (pDefaultBuildSettingsNode != nullptr) &&
        (std::strcmp(pDefaultBuildSettingsNode->Value(), XML_NODE_GLOBAL_DEFAULT_BUILD_SETTINGS) == 0));

    if (ret)
    {
        // Get the first default build settings element.
        tinyxml2::XMLNode* pApiSettingsNode = pDefaultBuildSettingsNode->FirstChildElement();

        // Loop through each possible API type and read each API's default build settings.
        for (int apiIndex = static_cast<int>(rgProjectAPI::OpenCL); apiIndex < static_cast<int>(rgProjectAPI::ApiCount); ++apiIndex)
        {
            if (pApiSettingsNode == nullptr)
            {
                // If there are missing APIs from the config file, just break from the loop.
                break;
            }
            else
            {
                rgProjectAPI currentApi = static_cast<rgProjectAPI>(apiIndex);

                std::string apiString;
                bool isOk = rgUtils::ProjectAPIToString(currentApi, apiString);
                assert(isOk);
                if (isOk)
                {
                    // Use the API-specific factory to create an API-specific build settings object.
                    std::shared_ptr<rgFactory> pApiFactory = rgFactory::CreateFactory(currentApi);
                    assert(pApiFactory != nullptr);
                    ret = pApiFactory != nullptr;
                    if (pApiFactory != nullptr)
                    {
                        std::shared_ptr<rgBuildSettings> pApiBuildSettings = pApiFactory->CreateBuildSettings(nullptr);
                        assert(pApiBuildSettings != nullptr);
                        ret = pApiBuildSettings != nullptr;
                        if (pApiBuildSettings != nullptr)
                        {
                            // Create an API-specific build settings instance and parser object to read the settings.
                            std::shared_ptr<rgXmlConfigFileReaderImpl> pReader = rgXMLConfigFileReaderImplFactory::CreateReader(apiString.c_str());
                            assert(pReader != nullptr);
                            ret = pReader != nullptr;
                            if (pReader != nullptr)
                            {
                                tinyxml2::XMLNode* pBuildSettingsNode = pApiSettingsNode->FirstChildElement(XML_NODE_BUILD_SETTINGS);
                                assert(pBuildSettingsNode != nullptr);
                                ret = pBuildSettingsNode != nullptr;
                                if (pBuildSettingsNode != nullptr && pBuildSettingsNode->FirstChild() != nullptr)
                                {
                                    // Extract the build settings common to all APIs.
                                    ret = pReader->ReadGeneralBuildSettings(pBuildSettingsNode, pApiBuildSettings);
                                    assert(ret);

                                    // Extract the API-specific default build settings.
                                    bool hasApiBuildSettings = pReader->ReadApiBuildSettings(pBuildSettingsNode, pApiBuildSettings);

                                    // TEMP: we don't have any valid Vulkan settings, so this can't be a required read.
                                    if (apiIndex == static_cast<int>(rgProjectAPI::Vulkan))
                                    {
                                        // This is temporarily okay for VUlkan until we get some Vulkan-specific build settings.
                                    }
                                    else
                                    {
                                        // OpenCL, or possibly an error case.
                                        assert(hasApiBuildSettings);
                                        ret = hasApiBuildSettings;
                                    }

                                    // Store the default API-specific build settings in our data object.
                                    pGlobalSettings->m_pDefaultBuildSettings[apiString] = pApiBuildSettings;
                                }
                            }
                        }
                    }
                }

                // Advance to the next API-specific build settings node.
                pApiSettingsNode = pApiSettingsNode->NextSibling();
            }
        }
    }

    return ret;
}

static bool ExtractGlobalSettings_2_0(tinyxml2::XMLNode* pGlobalSettingsNode, std::shared_ptr<rgGlobalSettings>& pGlobalSettings)
{
    assert(pGlobalSettingsNode != nullptr);
    assert(pGlobalSettings != nullptr);

    // Make sure this is actually the GlobalSettings nodes.
    bool ret = ((pGlobalSettings != nullptr) &&
                (pGlobalSettingsNode != nullptr) &&
                (std::strcmp(pGlobalSettingsNode->Value(), XML_NODE_GLOBAL_LOG_FILE_GLOBAL_SETTINGS) == 0));

    if (ret)
    {
        // Read the global settings.
        tinyxml2::XMLElement* pElem = pGlobalSettingsNode->FirstChildElement(XML_NODE_GLOBAL_LOG_FILE_LOCATION);
        if (ret && pElem != nullptr)
        {
            // Read the log file location.
            ret = rgXMLUtils::ReadNodeTextString(pElem, pGlobalSettings->m_logFileLocation);

            pElem = pElem->NextSiblingElement(XML_NODE_GLOBAL_LAST_SELECTED_DIRECTORY);
            if (ret && pElem != nullptr)
            {
                // Attempt to read the last selected directory. It might be empty, in which case the call returns false, and that's ok.
                rgXMLUtils::ReadNodeTextString(pElem, pGlobalSettings->m_lastSelectedDirectory);

                pElem = pElem->NextSiblingElement(XML_NODE_GLOBAL_RECENT_PROJECTS_ROOT);
                if (ret && pElem != nullptr)
                {
                    // Read the list of recently-accessed projects.
                    ret = ExtractRecentProjects(pElem, pGlobalSettings->m_recentProjects);

                    pElem = pElem->NextSiblingElement(XML_NODE_GLOBAL_DISASSEMBLY_COLUMNS);
                    if (ret && pElem != nullptr)
                    {
                        // Get the disassembly columns as a single string.
                        std::string tmpDisassemblyColumns;
                        ret = rgXMLUtils::ReadNodeTextString(pElem, tmpDisassemblyColumns);

                        // Parse the disassembly columns string, and save the items in the data object.
                        ExtractDisassemblyColumns(tmpDisassemblyColumns, pGlobalSettings->m_visibleDisassemblyViewColumns);

                        // Read the option that determines if the project names are generated or provided by the user.
                        pElem = pElem->NextSiblingElement(XML_NODE_USE_GENERATED_PROJECT_NAMES);
                        if (ret && pElem != nullptr)
                        {
                            ret = rgXMLUtils::ReadNodeTextBool(pElem, pGlobalSettings->m_useDefaultProjectName);
                            assert(ret);

                            pElem = pElem->NextSiblingElement(XML_NODE_GLOBAL_GUI);
                            if (ret && pElem != nullptr)
                            {
                                // Extract splitter config objects from the "GUI" node.
                                ret = ExtractSplitterConfigs(pElem, pGlobalSettings->m_guiLayoutSplitters);
                            }
                        }
                    }
                }
            }
        }

        // If the Global Settings could be read, then attempt to read the Default Build Settings.
        if (ret)
        {
            tinyxml2::XMLNode* pDefaultBuildSettingsNode = pGlobalSettingsNode->NextSibling();
            ret = ExtractDefaultBuildSettings_2_0(pDefaultBuildSettingsNode, pGlobalSettings);
        }
    }

    return ret;
}

static bool ExtractGlobalSettings_2_1(tinyxml2::XMLNode* pGlobalSettingsNode, std::shared_ptr<rgGlobalSettings>& pGlobalSettings)
{
    // Make sure this is actually the GlobalSettings nodes.
    tinyxml2::XMLElement* pElem = nullptr;
    bool ret = (std::strcmp(pGlobalSettingsNode->Value(), XML_NODE_GLOBAL_LOG_FILE_GLOBAL_SETTINGS) == 0);

    if (ret)
    {
        // Read the log file location.
        pElem = pGlobalSettingsNode->FirstChildElement(XML_NODE_GLOBAL_LOG_FILE_LOCATION);
        ret = pElem != nullptr;
        ret = ret && rgXMLUtils::ReadNodeTextString(pElem, pGlobalSettings->m_logFileLocation);

        // If the saved log directory does not exist, fall back to using the default one.
        if (!rgUtils::IsDirExists(pGlobalSettings->m_logFileLocation))
        {
            std::string appDataDir;
            rgConfigManager::GetDefaultDataFolder(appDataDir);
            pGlobalSettings->m_logFileLocation = appDataDir;
        }
    }
    if (ret)
    {
        // Attempt to read the last selected directory. It might be empty, in which case the call returns false, and that's ok.
        pElem = pElem->NextSiblingElement(XML_NODE_GLOBAL_LAST_SELECTED_DIRECTORY);
        ret &= (pElem != nullptr);
        if (ret)
        {
            rgXMLUtils::ReadNodeTextString(pElem, pGlobalSettings->m_lastSelectedDirectory);
        }
    }
    if (ret)
    {
        // Read the list of recently-accessed projects.
        pElem = pElem->NextSiblingElement(XML_NODE_GLOBAL_RECENT_PROJECTS_ROOT);
        ret &= (pElem != nullptr);
        ret = ret && ExtractRecentProjects(pElem, pGlobalSettings->m_recentProjects);
    }
    if (ret)
    {
        // Extract font family and size information.
        pElem = pElem->NextSiblingElement(XML_NODE_GLOBAL_FONT_FAMILY_ROOT);
        ret &= (pElem != nullptr);
        ret = ret && ExtractFontInformation(pElem, pGlobalSettings);
    }
    if (ret)
    {
        // Extract default include files viewer.
        // Before we move on keep the current node in case that we fail reading this node.
        auto pElemBeforeRead = pElem;
        pElem = pElem->NextSiblingElement(XML_NODE_GLOBAL_INCLUDE_FILES_VIEWER);
        ret &= (pElem != nullptr);
        ret = ret && rgXMLUtils::ReadNodeTextString(pElem, pGlobalSettings->m_includeFilesViewer);
        if (!ret)
        {
            // If we couldn't read it from the file, just use the default.
            pGlobalSettings->m_includeFilesViewer = STR_GLOBAL_SETTINGS_SRC_VIEW_INCLUDE_VIEWER_DEFAULT;

            // Roll back so that the next element read would be performed from the correct location.
            pElem = pElemBeforeRead;
            ret = true;
        }
    }
    if (ret)
    {
        // Get the disassembly columns as a single string.
        pElem = pElem->NextSiblingElement(XML_NODE_GLOBAL_DISASSEMBLY_COLUMNS);
        ret &= (pElem != nullptr);
        std::string tmpDisassemblyColumns;
        ret = ret && rgXMLUtils::ReadNodeTextString(pElem, tmpDisassemblyColumns);

        // Parse the disassembly columns string, and save the items in the data object.
        if (ret)
        {
            ExtractDisassemblyColumns(tmpDisassemblyColumns, pGlobalSettings->m_visibleDisassemblyViewColumns);
        }
    }
    if (ret)
    {
        // Read the option that determines if the project names are generated or provided by the user.
        pElem = pElem->NextSiblingElement(XML_NODE_USE_GENERATED_PROJECT_NAMES);
        ret &= (pElem != nullptr);
        ret = ret && rgXMLUtils::ReadNodeTextBool(pElem, pGlobalSettings->m_useDefaultProjectName);
    }
    if (ret)
    {
        // Read the default API.
        pElem = pElem->NextSiblingElement(XML_NODE_GLOBAL_DEFAULT_API);
        ret &= (pElem != nullptr);
        ret = ret && rgXMLUtils::ReadNodeTextUnsigned(pElem, (unsigned int&)pGlobalSettings->m_defaultAPI);
    }
    if (ret)
    {
        // Read the "Should prompt for API" setting.
        pElem = pElem->NextSiblingElement(XML_NODE_GLOBAL_PROMPT_FOR_API);
        ret &= (pElem != nullptr);
        ret = ret && rgXMLUtils::ReadNodeTextBool(pElem, pGlobalSettings->m_shouldPromptForAPI);
    }
    if (ret)
    {
        // Read the input file associations.
        pElem = pElem->NextSiblingElement(XML_NODE_GLOBAL_INPUT_FILE_EXT_GLSL);
        ret &= (pElem != nullptr);
        ret = ret && rgXMLUtils::ReadNodeTextString(pElem, pGlobalSettings->m_inputFileExtGlsl);
        if (ret)
        {
            pElem = pElem->NextSiblingElement(XML_NODE_GLOBAL_INPUT_FILE_EXT_HLSL);
            ret &= (pElem != nullptr);
            ret = ret && rgXMLUtils::ReadNodeTextString(pElem, pGlobalSettings->m_inputFileExtHlsl);
        }
        if (ret)
        {
            pElem = pElem->NextSiblingElement(XML_NODE_GLOBAL_INPUT_FILE_EXT_SPV_TXT);
            ret &= (pElem != nullptr);
            ret = ret && rgXMLUtils::ReadNodeTextString(pElem, pGlobalSettings->m_inputFileExtSpvTxt);
        }
        if (ret)
        {
            pElem = pElem->NextSiblingElement(XML_NODE_GLOBAL_INPUT_FILE_EXT_SPV_BIN);
            ret &= (pElem != nullptr);
            ret = ret && rgXMLUtils::ReadNodeTextString(pElem, pGlobalSettings->m_inputFileExtSpvBin);
        }
    }
    if (ret)
    {
        // Read the default source language.
        pElem = pElem->NextSiblingElement(XML_NODE_GLOBAL_DEFAULT_SRC_LANG);
        ret &= (pElem != nullptr);
        ret = ret && rgXMLUtils::ReadNodeTextUnsigned(pElem, (uint32_t&)pGlobalSettings->m_defaultLang);
    }
    if (ret)
    {
        // Extract splitter config objects from the "GUI" node.
        pElem = pElem->NextSiblingElement(XML_NODE_GLOBAL_GUI);
        ret &= (pElem != nullptr);
        ret = ret && ExtractSplitterConfigs(pElem, pGlobalSettings->m_guiLayoutSplitters);
    }

    // If the Global Settings could be read, then attempt to read the Default Build Settings.
    if (ret)
    {
        tinyxml2::XMLNode* pDefaultBuildSettingsNode = pGlobalSettingsNode->NextSibling();
        ret = ExtractDefaultBuildSettings_2_1(pDefaultBuildSettingsNode, pGlobalSettings);
    }

    return ret;
}

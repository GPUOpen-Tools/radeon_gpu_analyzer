// C++.
#include <cassert>
#include <sstream>

// XML.
#include <tinyxml2/Include/tinyxml2.h>

// Local.
#include <RadeonGPUAnalyzerGUI/include/rgStringConstants.h>
#include <RadeonGPUAnalyzerGUI/include/rgConfigFile.h>
#include <RadeonGPUAnalyzerGUI/include/rgUtils.h>
#include <RadeonGPUAnalyzerGUI/include/rgConfigFileDefinitions.h>
#include <RadeonGPUAnalyzerGUI/include/rgXMLUtils.h>

// *** INTERNALLY-LINKED AUXILIARY FUNCTIONS - BEGIN ***

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
static void ExtractAdditionalIncludeDirecotries(const std::string& additionalIncludeDirectories, std::vector<std::string>& dirList)
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

static bool ExtractRecentProjects(tinyxml2::XMLNode* pNode, std::vector<std::string>& recentProjects)
{
    bool ret = false;

    assert(pNode != nullptr);
    if (pNode != nullptr)
    {
        // Find the first project in the list of recent projects.
        pNode = pNode->FirstChildElement(XML_NODE_GLOBAL_RECENT_PROJECT_PATH);

        // If there aren't any recent project paths, return early, as there's nothing to parse..
        if (pNode == nullptr)
        {
            ret = true;
        }
        else
        {
            // Step over each path to a recent project, and add it to the output list.
            while (pNode != nullptr)
            {
                // Read the project path element.
                std::string projectPath;
                ret = rgXMLUtils::ReadNodeTextString(pNode, projectPath);
                assert(ret);
                if (ret)
                {
                    recentProjects.push_back(projectPath);
                }

                pNode = pNode->NextSiblingElement(XML_NODE_GLOBAL_RECENT_PROJECT_PATH);
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

            if (pNode != nullptr)
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
static bool ExtractGeneralBuildSettings(tinyxml2::XMLNode* pNode, rgBuildSettings& buildSettings)
{
    bool ret = false;
    if (pNode != nullptr)
    {
        pNode = pNode->FirstChildElement(XML_NODE_TARGET_DEVICES);
        std::string targetDevices;
        ret = rgXMLUtils::ReadNodeTextString(pNode, targetDevices);
        if (ret)
        {
            // Target GPUs.
            ExtractTargetGpus(targetDevices, buildSettings.m_targetGpus);
            ret = (!buildSettings.m_targetGpus.empty() && pNode != nullptr);
            assert(ret);

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
                        ExtractPredefinedMacros(predefinedMacros, buildSettings.m_predefinedMacros);
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
                            ExtractAdditionalIncludeDirecotries(additionalIncludeDirectories, buildSettings.m_additionalIncludeDirectories);
                        }
                    }
                }
            }
        }
    }
    return ret;
}

// Creates an element that has value of any primitive type.
template <typename T>
static void AppendXMLElement(tinyxml2::XMLDocument &xmlDoc,
    tinyxml2::XMLElement* pParentElement, const char* pElemName, T elemValue)
{
    tinyxml2::XMLElement* pElem = xmlDoc.NewElement(pElemName);
    pElem->SetText(elemValue);
    pParentElement->InsertEndChild(pElem);
}

// *** INTERNALLY-LINKED AUXILIARY FUNCTIONS - END ***


// An implantation for an API-specific config file reader.
class XMLConfigFileReaderImpl
{
public:

    // DTOR.
    virtual ~XMLConfigFileReaderImpl() = default;

    // Reads the configuration file from the given XML document, and instantiates a new RGA project.
    virtual bool ReadConfigFile(tinyxml2::XMLDocument& doc, std::shared_ptr<rgProject>& pProject) = 0;
};

// Reader for OpenCL config files.
class OpenCLConfigFileReader : public XMLConfigFileReaderImpl
{
public:

    bool ExtractOpenCLBuildSettings(tinyxml2::XMLNode* pNode, rgCLBuildSettings& buildSettings)
    {
        bool ret = false;

        assert(pNode != nullptr);
        if (pNode != nullptr)
        {
            // Extract the build settings.
            ret = ExtractGeneralBuildSettings(pNode, buildSettings);
        }

        if (ret && pNode != nullptr)
        {
            // OpenCL optimization level.
            pNode = pNode->FirstChildElement(XML_NODE_OPENCL_OPT_LEVEL);
            ret = rgXMLUtils::ReadNodeTextString(pNode, buildSettings.m_optimizationLevel);
        }

        if (ret && pNode != nullptr)
        {
            // Double as single.
            pNode = pNode->NextSiblingElement(XML_NODE_OPENCL_DOUBLE_AS_SINGLE);
            ret = rgXMLUtils::ReadNodeTextBool(pNode, buildSettings.m_isTreatDoubleAsSingle);
        }

        if (ret && pNode != nullptr)
        {
            // Denorms as zeros.
            pNode = pNode->NextSiblingElement(XML_NODE_OPENCL_DENORMS_AS_ZEROS);
            ret = rgXMLUtils::ReadNodeTextBool(pNode, buildSettings.m_isDenormsAsZeros);
        }

        if (ret && pNode != nullptr)
        {
            // Strict aliasing.
            pNode = pNode->NextSiblingElement(XML_NODE_OPENCL_STRICT_ALIASING);
            ret = rgXMLUtils::ReadNodeTextBool(pNode, buildSettings.m_isStrictAliasing);
        }

        if (ret && pNode != nullptr)
        {
            // Enable MAD.
            pNode = pNode->NextSiblingElement(XML_NODE_OPENCL_ENABLE_MAD);
            ret = rgXMLUtils::ReadNodeTextBool(pNode, buildSettings.m_isEnableMAD);
        }

        if (ret && pNode != nullptr)
        {
            // Ignore zero signedness.
            pNode = pNode->NextSiblingElement(XML_NODE_OPENCL_IGNORE_ZERO_SIGNEDNESS);
            ret = rgXMLUtils::ReadNodeTextBool(pNode, buildSettings.m_isIgnoreZeroSignedness);
        }

        if (ret && pNode != nullptr)
        {
            // Unsafe optimizations.
            pNode = pNode->NextSiblingElement(XML_NODE_OPENCL_UNSAFE_OPT);
            ret = rgXMLUtils::ReadNodeTextBool(pNode, buildSettings.m_isUnsafeOptimizations);
        }

        if (ret && pNode != nullptr)
        {
            // Non NaN nor Infinite.
            pNode = pNode->NextSiblingElement(XML_NODE_OPENCL_NO_NAN_NOR_INF);
            ret = rgXMLUtils::ReadNodeTextBool(pNode, buildSettings.m_isNoNanNorInfinite);
        }

        if (ret && pNode != nullptr)
        {
            // Aggressive math optimizations.
            pNode = pNode->NextSiblingElement(XML_NODE_OPENCL_AGGRESSIVE_MATH_OPT);
            ret = rgXMLUtils::ReadNodeTextBool(pNode, buildSettings.m_isAggressiveMathOptimizations);
        }

        if (ret && pNode != nullptr)
        {
            // Correctly round div / sqrt.
            pNode = pNode->NextSiblingElement(XML_NODE_OPENCL_CORRECT_ROUND_DIV_SQRT);
            ret = rgXMLUtils::ReadNodeTextBool(pNode, buildSettings.m_isCorrectlyRoundDivSqrt);
        }

        if (ret && pNode != nullptr)
        {
            // Additional options.
            pNode = pNode->NextSiblingElement(XML_NODE_ADDITIONAL_OPTIONS);
            rgXMLUtils::ReadNodeTextString(pNode, buildSettings.m_additionalOptions);
        }

        if (ret)
        {
            // Alternative compiler paths.
            if (pNode != nullptr)
            {
                pNode = pNode->NextSiblingElement(XML_NODE_ALTERNATIVE_COMPILER_BIN_DIR);
                rgXMLUtils::ReadNodeTextString(pNode, std::get<CompilerFolderType::Bin>(buildSettings.m_compilerPaths));
            }
            if (pNode != nullptr)
            {
                pNode = pNode->NextSiblingElement(XML_NODE_ALTERNATIVE_COMPILER_INC_DIR);
                rgXMLUtils::ReadNodeTextString(pNode, std::get<CompilerFolderType::Include>(buildSettings.m_compilerPaths));
            }
            if (pNode != nullptr)
            {
                pNode = pNode->NextSiblingElement(XML_NODE_ALTERNATIVE_COMPILER_LIB_DIR);
                rgXMLUtils::ReadNodeTextString(pNode, std::get<CompilerFolderType::Lib>(buildSettings.m_compilerPaths));
            }
        }

        return ret;
    }

    virtual bool ReadConfigFile(tinyxml2::XMLDocument& doc,
        std::shared_ptr<rgProject>& pRgaProject) override
    {
        bool ret = false;
        pRgaProject = nullptr;

        // Find the project node.
        tinyxml2::XMLNode* pNode = doc.FirstChildElement(XML_NODE_PROJECT);
        if (pNode != nullptr)
        {
            pNode = pNode->FirstChild();

            // Verify that this is an OpenCL API config file.
            std::string pApiName;
            ret = rgXMLUtils::ReadNodeTextString(pNode, pApiName);
            if (ret && (pApiName.compare(STR_API_NAME_OPENCL) == 0) && pNode != nullptr)
            {
                // Go to the project name node.
                pNode = pNode->NextSibling();

                // Get the project name.
                std::string projectName;
                ret = rgXMLUtils::ReadNodeTextString(pNode, projectName);
                if (!projectName.empty() && pNode != nullptr)
                {
                    // Create the RGA project object.
                    std::shared_ptr<rgCLProject> pOpenCLProject = std::make_shared<rgCLProject>();
                    pOpenCLProject->m_projectName = projectName;
                    pRgaProject = pOpenCLProject;

                    // Iterate through the project's clones: get the first clone.
                    pNode = pNode->NextSibling();
                    tinyxml2::XMLNode* pClonesRoot = pNode;

                    while (pClonesRoot != nullptr)
                    {
                        // Get the clone ID.
                        pNode = pClonesRoot->FirstChildElement(XML_NODE_CLONE_ID);
                        std::shared_ptr<rgProjectClone> pClone = std::make_shared<rgProjectClone>();
                        ret = rgXMLUtils::ReadNodeTextUnsigned(pNode, pClone->m_cloneId);
                        if (ret && pNode != nullptr)
                        {
                            // Get the name of the clone.
                            pNode = pNode->NextSibling();

                            ret = rgXMLUtils::ReadNodeTextString(pNode, pClone->m_cloneName);
                            if (ret)
                            {
                                // Get the files in this clone.
                                pNode = pNode->NextSibling();
                                tinyxml2::XMLElement* pFileNode = pNode->FirstChildElement();
                                while (pFileNode != nullptr)
                                {
                                    // Extract the full path of the current file.
                                    std::string sourceFileInfoString;
                                    ret = rgXMLUtils::ReadNodeTextString(pFileNode, sourceFileInfoString);
                                    assert(ret);
                                    if (ret && !sourceFileInfoString.empty())
                                    {
                                        rgSourceFileInfo newSourceFile = {};
                                        newSourceFile.m_filePath = sourceFileInfoString;

                                        // Add the file path to our clone object.
                                        pClone->m_sourceFiles.push_back(newSourceFile);
                                    }

                                    // Go to the next file.
                                    pFileNode = pFileNode->NextSiblingElement();
                                }

                                // Get the OpenCL build settings.
                                pNode = pNode->NextSiblingElement(XML_NODE_BUILD_SETTINGS);
                                std::shared_ptr<rgCLBuildSettings> pBuildSettings = std::make_shared< rgCLBuildSettings>();
                                ret = ExtractOpenCLBuildSettings(pNode, *pBuildSettings);
                                assert(ret);
                                if (ret)
                                {
                                    // Add the build settings to the project clone.
                                    pClone->m_pBuildSettings = pBuildSettings;

                                    // We are done, add this clone to the project object.
                                    pOpenCLProject->m_clones.push_back(pClone);
                                }
                            }
                        }

                        // Go to the next clone element.
                        pClonesRoot = pClonesRoot->NextSibling();
                    }
                }
            }

        }
        return ret;
    }
};

// Factory for creating the relevant config file reader in runtime.
class XMLConfigFileReaderImplFactory
{
public:
    static std::shared_ptr<XMLConfigFileReaderImpl> CreateReader(const std::string& apiName)
    {
        std::shared_ptr<XMLConfigFileReaderImpl> pRet = nullptr;
        if (apiName.compare(STR_API_NAME_OPENCL) == 0)
        {
            pRet = std::make_shared<OpenCLConfigFileReader>();
        }
        return pRet;
    }
};

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
                    // Verify that the data model is compatible.
                    const char* pDataModelVersion = pNode->ToElement()->GetText();
                    ret = (rgXMLUtils::GetRGADataModelVersion().compare(pDataModelVersion) == 0);
                    if (ret)
                    {
                        // Skip the project node.
                        pNode = pNode->NextSibling();

                        if (pNode != nullptr)
                        {
                            // Get the relevant API name.
                            pNode = pNode->FirstChild();
                            std::string apiName;
                            ret = rgXMLUtils::ReadNodeTextString(pNode, apiName);

                            // Create the concrete config file reader for the relevant API.
                            std::shared_ptr<XMLConfigFileReaderImpl> pReader = XMLConfigFileReaderImplFactory::CreateReader(apiName);
                            if (pReader != nullptr)
                            {
                                // Let the concrete parser handle the configuration.
                                ret = pReader->ReadConfigFile(doc, pProject);
                                assert(ret && pProject != nullptr);

                                // Set the project file path.
                                pProject->m_projectFileFullPath = configFilePath;
                            }
                        }
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

class XMLConfigWriterImpl
{
public:

    // DTOR.
    virtual ~XMLConfigWriterImpl() = default;

    // Writes the given project into a config file at the given location.
    virtual bool WriteConfigFile(const rgProject& project, const std::string& configFilePath) = 0;

    // Adds the opening RGA config file data to the given document: XML header, RGA data model version.
    static void AddConfigFileDeclaration(tinyxml2::XMLDocument& doc)
    {
        tinyxml2::XMLNode* pNode = doc.NewDeclaration(RGA_XML_DECLARATION);
        doc.InsertEndChild(pNode);

        // Create the Data Model Version element.
        tinyxml2::XMLElement* pElement = doc.NewElement(XML_NODE_DATA_MODEL_VERSION);
        pElement->SetText(RGA_DATA_MODEL.c_str());
        doc.LinkEndChild(pElement);
    }
};

class OpenCLConfigWriterImpl : public XMLConfigWriterImpl
{
public:

    virtual bool WriteConfigFile(const rgProject& project, const std::string& configFilePath) override
    {
        bool ret = false;

        // Create the XML declaration node.
        tinyxml2::XMLDocument doc;
        AddConfigFileDeclaration(doc);

        // Create the Project element.
        tinyxml2::XMLElement* pProject = doc.NewElement(XML_NODE_PROJECT);
        tinyxml2::XMLElement* pApi = doc.NewElement(XML_NODE_API_NAME);
        std::string apiName;
        ret = rgUtils::ProjectAPIToString(project.m_api, apiName);
        if (ret)
        {
            // API name.
            pApi->SetText(apiName.c_str());
            pProject->InsertFirstChild(pApi);

            // Project name.
            tinyxml2::XMLElement* pProjectName = doc.NewElement(XML_NODE_PROJECT_NAME);
            pProjectName->SetText(project.m_projectName.c_str());
            pProject->InsertEndChild(pProjectName);

            // Handle the project's clones.
            std::vector<tinyxml2::XMLElement*> cloneElems;
            const rgCLProject& clProject = static_cast<const rgCLProject&>(project);
            CreateOpenCLCloneElements(clProject, doc, cloneElems);
            for (tinyxml2::XMLElement* pCloneElem : cloneElems)
            {
                pProject->LinkEndChild(pCloneElem);
            }

            // Add the project node.
            doc.InsertEndChild(pProject);

            // Save the file.
            tinyxml2::XMLError rc = doc.SaveFile(configFilePath.c_str());
            ret = (rc == tinyxml2::XML_SUCCESS);
            assert(ret);
        }

        return ret;
    }

    static bool CreateOpenCLBuildSettingsElement(const rgCLBuildSettings& buildSettings, tinyxml2::XMLDocument& doc, tinyxml2::XMLElement*& pBuildSettingsElem)
    {
        bool ret = false;

        // Build Settings element.
        pBuildSettingsElem = doc.NewElement(XML_NODE_BUILD_SETTINGS);

        if (pBuildSettingsElem != nullptr)
        {
            // Target devices.
            AppendXMLElement(doc, pBuildSettingsElem, XML_NODE_TARGET_DEVICES, rgUtils::BuildSemicolonSeparatedStringList(buildSettings.m_targetGpus).c_str());

            // Predefined Macros.
            AppendXMLElement(doc, pBuildSettingsElem, XML_NODE_PREDEFINED_MACROS, rgUtils::BuildSemicolonSeparatedStringList(buildSettings.m_predefinedMacros).c_str());

            // Additional include directories.
            AppendXMLElement(doc, pBuildSettingsElem, XML_NODE_ADDITIONAL_INCLUDE_DIRECTORIES, rgUtils::BuildSemicolonSeparatedStringList(buildSettings.m_additionalIncludeDirectories).c_str());

            // Optimization Level.
            AppendXMLElement(doc, pBuildSettingsElem, XML_NODE_OPENCL_OPT_LEVEL, buildSettings.m_optimizationLevel.c_str());

            // Treat Double As Single.
            AppendXMLElement(doc, pBuildSettingsElem, XML_NODE_OPENCL_DOUBLE_AS_SINGLE, buildSettings.m_isTreatDoubleAsSingle);

            // Denorms As Zeros.
            AppendXMLElement(doc, pBuildSettingsElem, XML_NODE_OPENCL_DENORMS_AS_ZEROS, buildSettings.m_isDenormsAsZeros);

            // Strict Aliasing.
            AppendXMLElement(doc, pBuildSettingsElem, XML_NODE_OPENCL_STRICT_ALIASING, buildSettings.m_isStrictAliasing);

            // Enable MAD.
            AppendXMLElement(doc, pBuildSettingsElem, XML_NODE_OPENCL_ENABLE_MAD, buildSettings.m_isEnableMAD);

            // Ignore Zero Signedness.
            AppendXMLElement(doc, pBuildSettingsElem, XML_NODE_OPENCL_IGNORE_ZERO_SIGNEDNESS, buildSettings.m_isIgnoreZeroSignedness);

            // Unsafe Optimizations.
            AppendXMLElement(doc, pBuildSettingsElem, XML_NODE_OPENCL_UNSAFE_OPT, buildSettings.m_isUnsafeOptimizations);

            // No Nan Nor Infinite.
            AppendXMLElement(doc, pBuildSettingsElem, XML_NODE_OPENCL_NO_NAN_NOR_INF, buildSettings.m_isNoNanNorInfinite);

            // Aggressive Math Optimizations.
            AppendXMLElement(doc, pBuildSettingsElem, XML_NODE_OPENCL_AGGRESSIVE_MATH_OPT, buildSettings.m_isAggressiveMathOptimizations);

            // Correctly Round Div Sqrt.
            AppendXMLElement(doc, pBuildSettingsElem, XML_NODE_OPENCL_CORRECT_ROUND_DIV_SQRT, buildSettings.m_isCorrectlyRoundDivSqrt);

            // Additional options.
            AppendXMLElement(doc, pBuildSettingsElem, XML_NODE_ADDITIONAL_OPTIONS, buildSettings.m_additionalOptions.c_str());

            // Alternative compiler paths.
            AppendXMLElement(doc, pBuildSettingsElem, XML_NODE_ALTERNATIVE_COMPILER_BIN_DIR, std::get<CompilerFolderType::Bin>(buildSettings.m_compilerPaths).c_str());
            AppendXMLElement(doc, pBuildSettingsElem, XML_NODE_ALTERNATIVE_COMPILER_INC_DIR, std::get<CompilerFolderType::Include>(buildSettings.m_compilerPaths).c_str());
            AppendXMLElement(doc, pBuildSettingsElem, XML_NODE_ALTERNATIVE_COMPILER_LIB_DIR, std::get<CompilerFolderType::Lib>(buildSettings.m_compilerPaths).c_str());

            // Add the Build Settings element its parent.
            doc.InsertEndChild(pBuildSettingsElem);

            ret = true;
        }
        return ret;
    }

    bool CreateOpenCLCloneElements(const rgCLProject& project, tinyxml2::XMLDocument& doc, std::vector<tinyxml2::XMLElement*>& elems)
    {
        bool ret = false;

        for (const std::shared_ptr<rgProjectClone>& pClone : project.m_clones)
        {
            if (pClone != nullptr)
            {
                // Project clone.
                tinyxml2::XMLElement* pCloneElement = doc.NewElement(XML_NODE_CLONE);

                // Clone ID.
                tinyxml2::XMLElement* pCloneId = doc.NewElement(XML_NODE_CLONE_ID);
                pCloneId->SetText(pClone->m_cloneId);
                pCloneElement->LinkEndChild(pCloneId);

                // Clone name.
                tinyxml2::XMLElement* pCloneName = doc.NewElement(XML_NODE_CLONE_NAME);
                pCloneName->SetText(pClone->m_cloneName.c_str());
                pCloneElement->LinkEndChild(pCloneName);

                // Source files.
                tinyxml2::XMLElement* pCloneSourceFiles = doc.NewElement(XML_NODE_CLONE_SOURCE_FILES);

                // Go through each and every source file, and create its element.
                for (const rgSourceFileInfo& sourceFileInfo : pClone->m_sourceFiles)
                {
                    // Create the file element.
                    tinyxml2::XMLElement* pFilePath = doc.NewElement(XML_NODE_FILE_PATH);

                    std::stringstream fileStatusStream;
                    fileStatusStream << sourceFileInfo.m_filePath;

                    pFilePath->SetText(fileStatusStream.str().c_str());

                    // Attach the file element to the Source Files node.
                    pCloneSourceFiles->LinkEndChild(pFilePath);
                }

                // Add the Source Files node to the Clone element.
                pCloneElement->LinkEndChild(pCloneSourceFiles);

                // Build settings.
                tinyxml2::XMLElement* pBuildSettings = nullptr;
                std::shared_ptr<rgCLBuildSettings> pOpenCLBuildSettings =
                    std::static_pointer_cast<rgCLBuildSettings>(pClone->m_pBuildSettings);
                ret = CreateOpenCLBuildSettingsElement(*pOpenCLBuildSettings, doc, pBuildSettings);

                if (ret)
                {
                    pCloneElement->LinkEndChild(pBuildSettings);
                    elems.push_back(pCloneElement);
                    ret = true;
                }
            }
        }

        return ret;
    }
};

class ConfigFileWriterFactory
{
public:
    static std::shared_ptr<XMLConfigWriterImpl> CreateWriter(rgProjectAPI api)
    {
        std::shared_ptr<XMLConfigWriterImpl> pRet = nullptr;
        switch (api)
        {
        case Unknown:
            break;
        case OpenCL:
            pRet = std::make_shared<OpenCLConfigWriterImpl>();
            break;
        }
        return pRet;
    }
};

bool rgXmlConfigFile::WriteConfigFile(const rgProject& project, const std::string& configFilePath)
{
    bool ret = false;

    std::shared_ptr<XMLConfigWriterImpl> pWriter = ConfigFileWriterFactory::CreateWriter(project.m_api);
    if (pWriter != nullptr)
    {
        ret = pWriter->WriteConfigFile(project, configFilePath);
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
                    // Verify that the data model is compatible.
                    const char* pDataModelVersion = pNode->ToElement()->GetText();
                    ret = (rgXMLUtils::GetRGADataModelVersion().compare(pDataModelVersion) == 0);
                    if (ret)
                    {
                        // Go to the Global Settings element.
                        pNode = pNode->NextSibling();
                        if (ret && pNode != nullptr)
                        {
                            // Read the global settings.
                            pElem = pNode->FirstChildElement(XML_NODE_GLOBAL_LOG_FILE_LOCATION);
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

                                                    // Get the Default Build settings element.
                                                    pNode = pNode->NextSibling();
                                                    if (pNode != nullptr)
                                                    {
                                                        // Get the OpenCL default build settings element.
                                                        pNode = pNode->FirstChildElement(STR_API_NAME_OPENCL);
                                                        if (pNode != nullptr)
                                                        {
                                                            pElem = pNode->FirstChildElement(XML_NODE_BUILD_SETTINGS);

                                                            // Create the relevant reader: start with OpenCL.
                                                            std::shared_ptr<XMLConfigFileReaderImpl> pReader = XMLConfigFileReaderImplFactory::CreateReader(STR_API_NAME_OPENCL);
                                                            std::shared_ptr<OpenCLConfigFileReader> pOpenCLReader = std::dynamic_pointer_cast<OpenCLConfigFileReader>(pReader);

                                                            std::shared_ptr<rgCLBuildSettings> pOpenCLBuildSettings = std::make_shared<rgCLBuildSettings>();
                                                            if (pOpenCLReader != nullptr)
                                                            {
                                                                // Extract the OpenCL default build settings.
                                                                ret = pOpenCLReader->ExtractOpenCLBuildSettings(pElem, *pOpenCLBuildSettings);
                                                                assert(ret);

                                                                // Store the default OpenCL build settings in our data object.
                                                                pGlobalSettings->m_pDefaultBuildSettings[STR_API_NAME_OPENCL] = pOpenCLBuildSettings;
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
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
    bool ret = false;

    // Create the XML declaration node.
    tinyxml2::XMLDocument doc;
    XMLConfigWriterImpl::AddConfigFileDeclaration(doc);

    // Create the Global Settings element.
    tinyxml2::XMLElement* pGlobalSettingsElem = doc.NewElement(XML_NODE_GLOBAL_LOG_FILE_GLOBAL_SETTINGS);
    if (pGlobalSettingsElem != nullptr)
    {
        // Create the log file location element.
        AppendXMLElement(doc, pGlobalSettingsElem, XML_NODE_GLOBAL_LOG_FILE_LOCATION, pGlobalSettings->m_logFileLocation.c_str());

        // Create the last selected directory element.
        AppendXMLElement(doc, pGlobalSettingsElem, XML_NODE_GLOBAL_LAST_SELECTED_DIRECTORY, pGlobalSettings->m_lastSelectedDirectory.c_str());

        // Create a root node for the list of recent projects.
        tinyxml2::XMLElement* pRecentProjectsElem = doc.NewElement(XML_NODE_GLOBAL_RECENT_PROJECTS_ROOT);
        if (pRecentProjectsElem != nullptr)
        {
            if (pGlobalSettings->m_recentProjects.size() > 0)
            {
                for (size_t projectIndex = 0; projectIndex < pGlobalSettings->m_recentProjects.size(); ++projectIndex)
                {
                    // Save the most recent files, starting at the end of the list.
                    const std::string& projectPath = pGlobalSettings->m_recentProjects[projectIndex];

                    // Write the project path into the recent project element.
                    AppendXMLElement(doc, pRecentProjectsElem, XML_NODE_GLOBAL_RECENT_PROJECT_PATH, projectPath.c_str());
                }
            }

            // Add the list of recent projects to the global settings node.
            pGlobalSettingsElem->InsertEndChild(pRecentProjectsElem);
        }

        // Create a comma-separated list from the column names that we have.
        std::string disassemblyColumnStr = rgUtils::BuildSemicolonSeparatedBoolList(pGlobalSettings->m_visibleDisassemblyViewColumns);
        AppendXMLElement(doc, pGlobalSettingsElem, XML_NODE_GLOBAL_DISASSEMBLY_COLUMNS, disassemblyColumnStr.c_str());

        // An element used to determine if project names will be generated or provided by the user.
        AppendXMLElement(doc, pGlobalSettingsElem, XML_NODE_USE_GENERATED_PROJECT_NAMES, pGlobalSettings->m_useDefaultProjectName);

        // Create "GUI" element.
        tinyxml2::XMLElement* pGuiElement = doc.NewElement(XML_NODE_GLOBAL_GUI);
        if (pRecentProjectsElem != nullptr)
        {
            // Create "Layout" element.
            tinyxml2::XMLElement* pLayoutElement = doc.NewElement(XML_NODE_GLOBAL_GUI_LAYOUT);
            if (pLayoutElement != nullptr)
            {
                if (pGlobalSettings->m_guiLayoutSplitters.size() > 0)
                {
                    for (rgSplitterConfig splitterConfig : pGlobalSettings->m_guiLayoutSplitters)
                    {
                        // Create "Splitter" element.
                        tinyxml2::XMLElement* pSplitterElement = doc.NewElement(XML_NODE_GLOBAL_GUI_SPLITTER);
                        if (pSplitterElement != nullptr)
                        {
                            // Add "SplitterName" element.
                            AppendXMLElement(doc, pSplitterElement, XML_NODE_GLOBAL_GUI_SPLITTER_NAME, splitterConfig.m_splitterName.c_str());

                            // Add "SplitterValues" element.
                            std::string splitterValuesStr = rgUtils::BuildSemicolonSeparatedIntList(splitterConfig.m_splitterValues);
                            AppendXMLElement(doc, pSplitterElement, XML_NODE_GLOBAL_GUI_SPLITTER_VALUES, splitterValuesStr.c_str());

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
            // Add the OpenCL element.
            tinyxml2::XMLElement* pOpenCLSection = doc.NewElement(STR_API_NAME_OPENCL);

            // Create the OpenCL-specific default build settings element.
            std::shared_ptr<rgCLBuildSettings> pOpenCLBuildSettings =
                std::dynamic_pointer_cast<rgCLBuildSettings>(pGlobalSettings->m_pDefaultBuildSettings[STR_API_NAME_OPENCL]);

            // Create the OpenCL build settings element.
            tinyxml2::XMLElement* pOpenCLSpecificBuildSettingsElem;
            ret = OpenCLConfigWriterImpl::CreateOpenCLBuildSettingsElement(*pOpenCLBuildSettings, doc, pOpenCLSpecificBuildSettingsElem);

            if (ret)
            {
                // Add the Build Settings node under the OpenCL element.
                pOpenCLSection->LinkEndChild(pOpenCLSpecificBuildSettingsElem);
            }

            // Insert the OpenCL-specific default build settings element to the document.
            pDefaultBuildSettings->InsertEndChild(pOpenCLSection);

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

    return ret;
}


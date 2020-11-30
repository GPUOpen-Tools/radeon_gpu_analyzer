// C++>
#include <cassert>
#include <sstream>

// Infra.
#include <tinyxml2/Include/tinyxml2.h>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/rgConfigFileDefinitions.h>
#include <RadeonGPUAnalyzerGUI/Include/rgConfigFileOpenCL.h>
#include <RadeonGPUAnalyzerGUI/Include/rgXMLUtils.h>
#include <RadeonGPUAnalyzerGUI/Include/rgUtils.h>

bool rgConfigFileReaderOpenCL::ReadProjectConfigFile(tinyxml2::XMLDocument& doc, const char* pFileDataModelVersion, std::shared_ptr<rgProject>& pRgaProject)
{
    // Version 2.2 requires processing of binary output file, which
   // is handled later.
    bool isVersionCompatible =
        (RGA_DATA_MODEL_2_0.compare(pFileDataModelVersion) == 0) ||
        (RGA_DATA_MODEL_2_1.compare(pFileDataModelVersion) == 0) ||
        (RGA_DATA_MODEL_2_2.compare(pFileDataModelVersion) == 0);

    assert(isVersionCompatible);

    bool ret = false;
    pRgaProject = nullptr;

    if (isVersionCompatible)
    {
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
                    std::shared_ptr<rgProjectOpenCL> pOpenCLProject = std::make_shared<rgProjectOpenCL>();
                    pOpenCLProject->m_projectName = projectName;
                    pOpenCLProject->m_projectDataModelVersion = pFileDataModelVersion;
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
                                std::shared_ptr<rgBuildSettingsOpenCL> pBuildSettings = std::make_shared<rgBuildSettingsOpenCL>();
                                assert(pBuildSettings != nullptr);
                                if (pBuildSettings != nullptr)
                                {
                                    // Read the general build settings that aren't specific to a single API.
                                    ret = ret && ReadGeneralBuildSettings(pNode, pBuildSettings);
                                    assert(ret);
                                    if (ret)
                                    {
                                        // Read the build settings that apply only to API.
                                        ret = ret && ReadApiBuildSettings(pNode, pBuildSettings, pOpenCLProject->m_projectDataModelVersion);
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
                            }
                        }

                        // Go to the next clone element.
                        pClonesRoot = pClonesRoot->NextSibling();
                    }
                }
            }
        }
    }

    return ret;
}

bool rgConfigFileReaderOpenCL::ReadApiBuildSettings(tinyxml2::XMLNode* pNode, std::shared_ptr<rgBuildSettings> pBuildSettings, const std::string& version)
{
    bool ret = false;

    const std::shared_ptr<rgBuildSettingsOpenCL> pBuildSettingsOpenCL = std::dynamic_pointer_cast<rgBuildSettingsOpenCL>(pBuildSettings);

    if (pNode != nullptr)
    {
        // OpenCL optimization level.
        pNode = pNode->FirstChildElement(XML_NODE_OPENCL_OPT_LEVEL);
        ret = rgXMLUtils::ReadNodeTextString(pNode, pBuildSettingsOpenCL->m_optimizationLevel);
    }

    if (ret && pNode != nullptr)
    {
        // Double as single.
        pNode = pNode->NextSiblingElement(XML_NODE_OPENCL_DOUBLE_AS_SINGLE);
        ret = rgXMLUtils::ReadNodeTextBool(pNode, pBuildSettingsOpenCL->m_isTreatDoubleAsSingle);
    }

    if (ret && pNode != nullptr)
    {
        // Denorms as zeros.
        pNode = pNode->NextSiblingElement(XML_NODE_OPENCL_DENORMS_AS_ZEROS);
        ret = rgXMLUtils::ReadNodeTextBool(pNode, pBuildSettingsOpenCL->m_isDenormsAsZeros);
    }

    if (ret && pNode != nullptr)
    {
        // Strict aliasing.
        pNode = pNode->NextSiblingElement(XML_NODE_OPENCL_STRICT_ALIASING);
        ret = rgXMLUtils::ReadNodeTextBool(pNode, pBuildSettingsOpenCL->m_isStrictAliasing);
    }

    if (ret && pNode != nullptr)
    {
        // Enable MAD.
        pNode = pNode->NextSiblingElement(XML_NODE_OPENCL_ENABLE_MAD);
        ret = rgXMLUtils::ReadNodeTextBool(pNode, pBuildSettingsOpenCL->m_isEnableMAD);
    }

    if (ret && pNode != nullptr)
    {
        // Ignore zero signedness.
        pNode = pNode->NextSiblingElement(XML_NODE_OPENCL_IGNORE_ZERO_SIGNEDNESS);
        ret = rgXMLUtils::ReadNodeTextBool(pNode, pBuildSettingsOpenCL->m_isIgnoreZeroSignedness);
    }

    if (ret && pNode != nullptr)
    {
        // Unsafe optimizations.
        pNode = pNode->NextSiblingElement(XML_NODE_OPENCL_UNSAFE_OPT);
        ret = rgXMLUtils::ReadNodeTextBool(pNode, pBuildSettingsOpenCL->m_isUnsafeOptimizations);
    }

    if (ret && pNode != nullptr)
    {
        // Non NaN nor Infinite.
        pNode = pNode->NextSiblingElement(XML_NODE_OPENCL_NO_NAN_NOR_INF);
        ret = rgXMLUtils::ReadNodeTextBool(pNode, pBuildSettingsOpenCL->m_isNoNanNorInfinite);
    }

    if (ret && pNode != nullptr)
    {
        // Aggressive math optimizations.
        pNode = pNode->NextSiblingElement(XML_NODE_OPENCL_AGGRESSIVE_MATH_OPT);
        ret = rgXMLUtils::ReadNodeTextBool(pNode, pBuildSettingsOpenCL->m_isAggressiveMathOptimizations);
    }

    if (ret && pNode != nullptr)
    {
        // Correctly round div / sqrt.
        pNode = pNode->NextSiblingElement(XML_NODE_OPENCL_CORRECT_ROUND_DIV_SQRT);
        ret = rgXMLUtils::ReadNodeTextBool(pNode, pBuildSettingsOpenCL->m_isCorrectlyRoundDivSqrt);
    }

    if (ret)
    {
        // Alternative compiler paths.
        if (pNode != nullptr)
        {
            pNode = pNode->NextSiblingElement(XML_NODE_ALTERNATIVE_COMPILER_BIN_DIR);
            rgXMLUtils::ReadNodeTextString(pNode, std::get<CompilerFolderType::Bin>(pBuildSettingsOpenCL->m_compilerPaths));
        }
        if (pNode != nullptr)
        {
            pNode = pNode->NextSiblingElement(XML_NODE_ALTERNATIVE_COMPILER_INC_DIR);
            rgXMLUtils::ReadNodeTextString(pNode, std::get<CompilerFolderType::Include>(pBuildSettingsOpenCL->m_compilerPaths));
        }
        if (pNode != nullptr)
        {
            pNode = pNode->NextSiblingElement(XML_NODE_ALTERNATIVE_COMPILER_LIB_DIR);
            rgXMLUtils::ReadNodeTextString(pNode, std::get<CompilerFolderType::Lib>(pBuildSettingsOpenCL->m_compilerPaths));
        }
    }

    return ret;
}

bool rgConfigFileWriterOpenCL::WriteProjectConfigFile(const rgProject& project, const std::string& configFilePath)
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
        const rgProjectOpenCL& clProject = static_cast<const rgProjectOpenCL&>(project);
        WriteOpenCLCloneElements(clProject, doc, cloneElems);
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

bool rgConfigFileWriterOpenCL::WriteBuildSettingsElement(const std::shared_ptr<rgBuildSettings> pBuildSettings, tinyxml2::XMLDocument& doc, tinyxml2::XMLElement*& pBuildSettingsElem)
{
    bool ret = false;

    assert(pBuildSettings != nullptr);
    if (pBuildSettings != nullptr)
    {
        const std::shared_ptr<rgBuildSettingsOpenCL> pBuildSettingsOpenCL = std::dynamic_pointer_cast<rgBuildSettingsOpenCL>(pBuildSettings);
        assert(pBuildSettingsOpenCL != nullptr);

        if (pBuildSettingsElem != nullptr && pBuildSettingsOpenCL != nullptr)
        {
            // Write API-agnostic build settings.
            WriteGeneralBuildSettings(pBuildSettingsOpenCL, doc, pBuildSettingsElem);

            // Optimization Level.
            rgXMLUtils::AppendXMLElement(doc, pBuildSettingsElem, XML_NODE_OPENCL_OPT_LEVEL, pBuildSettingsOpenCL->m_optimizationLevel.c_str());

            // Treat Double As Single.
            rgXMLUtils::AppendXMLElement(doc, pBuildSettingsElem, XML_NODE_OPENCL_DOUBLE_AS_SINGLE, pBuildSettingsOpenCL->m_isTreatDoubleAsSingle);

            // Denorms As Zeros.
            rgXMLUtils::AppendXMLElement(doc, pBuildSettingsElem, XML_NODE_OPENCL_DENORMS_AS_ZEROS, pBuildSettingsOpenCL->m_isDenormsAsZeros);

            // Strict Aliasing.
            rgXMLUtils::AppendXMLElement(doc, pBuildSettingsElem, XML_NODE_OPENCL_STRICT_ALIASING, pBuildSettingsOpenCL->m_isStrictAliasing);

            // Enable MAD.
            rgXMLUtils::AppendXMLElement(doc, pBuildSettingsElem, XML_NODE_OPENCL_ENABLE_MAD, pBuildSettingsOpenCL->m_isEnableMAD);

            // Ignore Zero Signedness.
            rgXMLUtils::AppendXMLElement(doc, pBuildSettingsElem, XML_NODE_OPENCL_IGNORE_ZERO_SIGNEDNESS, pBuildSettingsOpenCL->m_isIgnoreZeroSignedness);

            // Unsafe Optimizations.
            rgXMLUtils::AppendXMLElement(doc, pBuildSettingsElem, XML_NODE_OPENCL_UNSAFE_OPT, pBuildSettingsOpenCL->m_isUnsafeOptimizations);

            // No Nan Nor Infinite.
            rgXMLUtils::AppendXMLElement(doc, pBuildSettingsElem, XML_NODE_OPENCL_NO_NAN_NOR_INF, pBuildSettingsOpenCL->m_isNoNanNorInfinite);

            // Aggressive Math Optimizations.
            rgXMLUtils::AppendXMLElement(doc, pBuildSettingsElem, XML_NODE_OPENCL_AGGRESSIVE_MATH_OPT, pBuildSettingsOpenCL->m_isAggressiveMathOptimizations);

            // Correctly Round Div Sqrt.
            rgXMLUtils::AppendXMLElement(doc, pBuildSettingsElem, XML_NODE_OPENCL_CORRECT_ROUND_DIV_SQRT, pBuildSettingsOpenCL->m_isCorrectlyRoundDivSqrt);

            // Alternative compiler paths.
            rgXMLUtils::AppendXMLElement(doc, pBuildSettingsElem, XML_NODE_ALTERNATIVE_COMPILER_BIN_DIR, std::get<CompilerFolderType::Bin>(pBuildSettingsOpenCL->m_compilerPaths).c_str());
            rgXMLUtils::AppendXMLElement(doc, pBuildSettingsElem, XML_NODE_ALTERNATIVE_COMPILER_INC_DIR, std::get<CompilerFolderType::Include>(pBuildSettingsOpenCL->m_compilerPaths).c_str());
            rgXMLUtils::AppendXMLElement(doc, pBuildSettingsElem, XML_NODE_ALTERNATIVE_COMPILER_LIB_DIR, std::get<CompilerFolderType::Lib>(pBuildSettingsOpenCL->m_compilerPaths).c_str());

            // Add the Build Settings element its parent.
            doc.InsertEndChild(pBuildSettingsElem);

            ret = true;
        }
    }

    return ret;
}

bool rgConfigFileWriterOpenCL::WriteOpenCLCloneElements(const rgProjectOpenCL& project, tinyxml2::XMLDocument& doc, std::vector<tinyxml2::XMLElement*>& elems)
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
            tinyxml2::XMLElement* pBuildSettings = doc.NewElement(XML_NODE_BUILD_SETTINGS);
            ret = WriteBuildSettingsElement(pClone->m_pBuildSettings, doc, pBuildSettings);

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
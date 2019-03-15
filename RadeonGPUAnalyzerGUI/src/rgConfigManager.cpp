// C++.
#include <algorithm>
#include <cassert>
#include <sstream>

// Qt.
#include <QStandardPaths>

// Infra.
#include <RadeonGPUAnalyzerGUI/../Utils/Include/rgaSharedUtils.h>
#include <RadeonGPUAnalyzerGUI/../Utils/Include/rgLog.h>
#include <Common/Src/AMDTOSWrappers/Include/osFilePath.h>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgIsaDisassemblyTableModel.h>
#include <RadeonGPUAnalyzerGUI/Include/rgConfigManager.h>
#include <RadeonGPUAnalyzerGUI/Include/rgConfigFile.h>
#include <RadeonGPUAnalyzerGUI/Include/rgCliLauncher.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDataTypes.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDataTypesOpenCL.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDataTypesVulkan.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDefinitions.h>
#include <RadeonGPUAnalyzerGUI/Include/rgStringConstants.h>
#include <RadeonGPUAnalyzerGUI/Include/rgXMLCliVersionInfo.h>
#include <RadeonGPUAnalyzerGUI/Include/rgUtils.h>

// The initial API that the application starts in.
static const rgProjectAPI INITIAL_API_TYPE = rgProjectAPI::OpenCL;

// The maximum number of recent files that the application is aware of.
static const int MAX_RECENT_FILES = 10;

// The default number of days for log files to be considered as "old".
static const int  DEFAULT_OLD_LOG_FILES_DAYS = 3;

// GUI log file name prefix.
static const char* GUI_LOG_FILE_PREFIX = "rga-gui.log";

// CLI log file name prefix.
static const char* CLI_LOG_FILE_PREFIX = "rga-cli.log";

struct ProjectPathSearcher
{
    ProjectPathSearcher(const std::string& projectPath) : m_projectPath(projectPath) {}

    // A predicate that will compare each project path with a target path to search for.
    bool operator()(const std::shared_ptr<rgRecentProject> pRecentProject) const
    {
        bool result = false;
        assert(pRecentProject != nullptr);
        if (pRecentProject != nullptr)
        {
            result =  pRecentProject->projectPath.compare(m_projectPath) == 0;
        }

        return result;
    }

    // The target project name to search for.
    const std::string& m_projectPath;
};

// This class creates default configuration-related objects.
class DefaultConfigFactory
{
public:
    // Creates the default build settings for the given API.
    static std::shared_ptr<rgBuildSettings> GetDefaultBuildSettings(rgProjectAPI api)
    {
        std::shared_ptr<rgBuildSettings> pRet = nullptr;
        switch (api)
        {
        case rgProjectAPI::OpenCL:
            pRet = CreateDefaultBuildSettingsOpenCL();
            break;
        case rgProjectAPI::Vulkan:
            pRet = CreateDefaultBuildSettingsVulkan();
            break;
        case rgProjectAPI::Unknown:
        default:
            break;
        }
        return pRet;
    }

private:

    // Add default target GPU hardware. Suitable default target GPUs are discovered by looking at
    // the list of supported devices and choosing the last item in the list. Items at the end of
    // the list are the most recently released products.
    static void AddDefaultTargetGpus(std::shared_ptr<rgBuildSettings> pBuildSettings)
    {
        // Use the version info's list of supported GPUs to determine the most recent
        // hardware to build for by default.
        rgConfigManager& configManager = rgConfigManager::Instance();
        std::shared_ptr<rgCliVersionInfo> pVersionInfo = configManager.GetVersionInfo();

        assert(pVersionInfo != nullptr);
        if (pVersionInfo != nullptr)
        {
            // Find the set of GPU architectures supported in the current mode.
            const std::string& apiMode = configManager.GetCurrentModeString();
            auto modeArchitecturesIter = pVersionInfo->m_gpuArchitectures.find(apiMode);
            if (modeArchitecturesIter != pVersionInfo->m_gpuArchitectures.end())
            {
                // Find the last supported architecture in the list.
                // The last entries are always the most recently released hardware.
                const std::vector<rgGpuArchitecture>& modeArchitectures = modeArchitecturesIter->second;
                auto lastArchitectureIter = modeArchitectures.rbegin();

                // Find the last family within the last architecture. This is the latest supported GPU family.
                const auto& architectureFamilies = lastArchitectureIter->m_gpuFamilies;
                const auto& latestFamily = architectureFamilies.rbegin();
                const std::string& latestFamilyName = latestFamily->m_familyName;

                // Add the latest supported GPU family as the default target GPU.
                pBuildSettings->m_targetGpus.push_back(latestFamilyName);
            }
        }
    }

    // Creates the default OpenCL build settings.
    static std::shared_ptr<rgBuildSettingsOpenCL> CreateDefaultBuildSettingsOpenCL()
    {
        std::shared_ptr<rgBuildSettingsOpenCL> pRet = std::make_shared<rgBuildSettingsOpenCL>();

        assert(pRet != nullptr);
        if (pRet != nullptr)
        {
            AddDefaultTargetGpus(pRet);
        }

        return pRet;
    }

    // Creates the default Vulkan build settings.
    static std::shared_ptr<rgBuildSettingsVulkan> CreateDefaultBuildSettingsVulkan()
    {
        std::shared_ptr<rgBuildSettingsVulkan> pRet = std::make_shared<rgBuildSettingsVulkan>();

        assert(pRet != nullptr);
        if (pRet != nullptr)
        {
            AddDefaultTargetGpus(pRet);
        }

        return pRet;
    }
};

// Generates the name of the config file.
static std::string GenConfigFileName()
{
    std::string configFileName = STR_GLOBAL_CONFIG_FILE_NAME_PREFIX;
    std::string versionSuffix = std::string("_") + std::to_string(RGA_VERSION_MAJOR) + "_" + std::to_string(RGA_VERSION_MINOR);
    configFileName += versionSuffix;
    configFileName += ".";
    configFileName += STR_GLOBAL_CONFIG_FILE_EXTENSION;

    return configFileName;
}

bool rgSourceFilePathSearcher::operator()(const rgSourceFileInfo& fileInfo) const
{
    return fileInfo.m_filePath.compare(m_targetFilePath) == 0;
}

bool rgConfigManager::Init()
{
    if (!m_isInitialized)
    {
        // Initialize the API used by the rgConfigManager.
        m_currentAPI = INITIAL_API_TYPE;

        // Get the path to the default configuration file.
        std::string appDataDir, configFileFullPath;
        rgConfigManager::GetDefaultDataFolder(appDataDir);

        // If the output directory does not exist - create it.
        if (!rgUtils::IsDirExists(appDataDir))
        {
            bool isDirCreated = rgUtils::CreateFolder(appDataDir);
            assert(isDirCreated);
        }

        // If necessary - create the Projects sub-directory.
        std::string projectSubDir;
        GetDefaultProjectsFolder(projectSubDir);
        if (!rgUtils::IsDirExists(projectSubDir))
        {
            bool isDirCreated = rgUtils::CreateFolder(projectSubDir);
            assert(isDirCreated);
        }

        // Attempt to load the version info file cached in the RGA data folder.
        std::string versionInfoFilePath;
        rgConfigManager::GetDefaultDataFolder(versionInfoFilePath);
        rgUtils::AppendFileNameToPath(versionInfoFilePath, STR_VERSION_INFO_FILE_NAME, versionInfoFilePath);

        // Ask the CLI to generate the version info file.
        bool isSuccessful = rgCliLauncher::GenerateVersionInfoFile(versionInfoFilePath);
        assert(isSuccessful);

        // Read the version info file.
        m_pVersionInfo = std::make_shared<rgCliVersionInfo>();
        bool isVersionInfoFileRead = rgXMLCliVersionInfo::ReadVersionInfo(versionInfoFilePath, m_pVersionInfo);
        assert(isVersionInfoFileRead);
        if (isVersionInfoFileRead)
        {
            // Append the global configuration file name to the path.
            std::string configFileName = GenConfigFileName();
            assert(!configFileName.empty());
            if (!configFileName.empty())
            {
                rgUtils::AppendFileNameToPath(appDataDir, configFileName, configFileFullPath);
            }

            // Check if the global configuration file exists. If not - create it.
            bool isConfigFileExists = rgUtils::IsFileExists(configFileFullPath);
            if (isConfigFileExists)
            {
                // Read the global configuration file and validate the operation's success.
                m_isInitialized = rgXmlConfigFile::ReadGlobalSettings(configFileFullPath, m_pGlobalSettings);

                if (!m_isInitialized)
                {
                    // Notify the user in case that we failed to read the config file.
                    std::stringstream msg;
                    msg << STR_ERR_FATAL_PREFIX << STR_ERR_CANNOT_READ_CONFIG_FILE_A << configFileFullPath <<
                        std::endl << STR_ERR_DELETE_FILE_AND_RERUN << std::endl << STR_ERR_RGA_WILL_NOW_EXIT;
                    m_fatalErrorMsg = msg.str();
                }

                m_isInitialized = m_isInitialized && (m_pGlobalSettings != nullptr);
            }

            // Write out a default configuration file when one either doesn't exist, or the read fails.
            if (!m_isInitialized && m_fatalErrorMsg.empty())
            {
                // Create the global configuration file.
                m_pGlobalSettings = std::make_shared<rgGlobalSettings>();

                // Initialize default values in the new global settings instance.
                assert(m_pGlobalSettings != nullptr);
                if (m_pGlobalSettings != nullptr)
                {
                    ResetToFactoryDefaults(*m_pGlobalSettings);
                }

                // Loop through each supported API and create + store the default build settings for that API.
                for (int apiIndex = static_cast<int>(rgProjectAPI::OpenCL); apiIndex < static_cast<int>(rgProjectAPI::ApiCount); ++apiIndex)
                {
                    rgProjectAPI currentApi = static_cast<rgProjectAPI>(apiIndex);

                    std::string apiString;
                    bool isOk = rgUtils::ProjectAPIToString(currentApi, apiString);
                    assert(isOk);
                    if (isOk)
                    {
                        std::shared_ptr<rgBuildSettings> pDefaultBuildSettings = GetDefaultBuildSettings(currentApi);
                        m_pGlobalSettings->m_pDefaultBuildSettings[apiString] = pDefaultBuildSettings;
                    }
                }

                // Write out the new global settings file.
                m_isInitialized = rgXmlConfigFile::WriteGlobalSettings(m_pGlobalSettings, configFileFullPath);
            }
        }
        else
        {
            std::stringstream msg;
            msg << STR_ERR_FATAL_PREFIX << STR_ERR_CANNOT_LOAD_VERSION_INFO_FILE << versionInfoFilePath << std::endl <<
                STR_ERR_DELETE_FILE_AND_RERUN << std::endl << STR_ERR_RGA_WILL_NOW_EXIT;
            m_fatalErrorMsg = msg.str();
            rgLog::file << STR_LOG_FAILED_LOAD_VERSION_INFO_FILE << versionInfoFilePath << std::endl;
        }

        if (m_isInitialized)
        {
            // Initialize GUI log file.
            std::string  logFileDir = (m_pGlobalSettings->m_logFileLocation.empty() ? appDataDir : m_pGlobalSettings->m_logFileLocation);
            bool isSuccessful = rgaSharedUtils::InitLogFile(logFileDir, GUI_LOG_FILE_PREFIX, DEFAULT_OLD_LOG_FILES_DAYS);
            assert(isSuccessful);
            if (isSuccessful)
            {
                rgLog::file << STR_LOG_RGA_GUI_STARTED << std::endl;
            }

            // Construct name for the CLI log file.
            std::string cliLogName = rgaSharedUtils::ConstructLogFileName(CLI_LOG_FILE_PREFIX);
            assert(!cliLogName.empty());
            if (!cliLogName.empty() && rgUtils::AppendFileNameToPath(appDataDir, cliLogName, cliLogName))
            {
                m_cliLogFilePath = cliLogName;
            }

            if (!isSuccessful || m_cliLogFilePath.empty())
            {
                rgUtils::ShowErrorMessageBox(STR_ERR_CANNOT_OPEN_LOG_FILE);
            }
        }
    }

    return m_isInitialized;
}

void rgConfigManager::ResetToFactoryDefaults(rgGlobalSettings& globalSettings)
{
    // Initialize the default log file location.
    rgConfigManager::GetDefaultDataFolder(globalSettings.m_logFileLocation);

    // Initialize the visible columns in the ISA disassembly table.
    // Only the Address, Opcode and Operands columns are visible by default.
    globalSettings.m_visibleDisassemblyViewColumns =
    {
        true,   // rgIsaDisassemblyTableColumns::Address
        true,   // rgIsaDisassemblyTableColumns::Opcode
        true,   // rgIsaDisassemblyTableColumns::Operands
        false,  // rgIsaDisassemblyTableColumns::FunctionalUnit
        false,  // rgIsaDisassemblyTableColumns::Cycles
        false,  // rgIsaDisassemblyTableColumns::BinaryEncoding
    };

    // Default to always asking the user for a name when creating a new project.
    globalSettings.m_useDefaultProjectName = false;

    // Default to always asking the user at startup which API to work in.
    globalSettings.m_shouldPromptForAPI = true;

    // This is an odd option, because Unknown isn't actually an option we support in the GUI
    // but it is theoretically okay, because by default the user is supposed to select it.
    globalSettings.m_defaultAPI = rgProjectAPI::Unknown;

    // Default font family.
    globalSettings.m_fontFamily = STR_BUILD_VIEW_FONT_FAMILY;

    // Default font size.
    globalSettings.m_fontSize = 10;

    // Default app to open include files.
    globalSettings.m_includeFilesViewer = STR_GLOBAL_SETTINGS_SRC_VIEW_INCLUDE_VIEWER_DEFAULT;

    globalSettings.m_inputFileExtGlsl = STR_GLOBAL_SETTINGS_FILE_EXT_GLSL;
    globalSettings.m_inputFileExtHlsl = STR_GLOBAL_SETTINGS_FILE_EXT_HLSL;
    globalSettings.m_inputFileExtSpvTxt = STR_GLOBAL_SETTINGS_FILE_EXT_SPVAS;
    globalSettings.m_inputFileExtSpvBin = STR_GLOBAL_SETTINGS_FILE_EXT_SPV;

    globalSettings.m_defaultLang = rgSrcLanguage::GLSL;
}

rgConfigManager& rgConfigManager::Instance()
{
    static rgConfigManager m_instance;
    return m_instance;
}

void rgConfigManager::AddSourceFileToProject(const std::string& sourceFilePath, std::shared_ptr<rgProject> pProject, int cloneIndex) const
{
    assert(pProject != nullptr);
    if (pProject != nullptr)
    {
        // Ensure that the incoming clone index is valid for the current project.
        bool isValidRange = (cloneIndex >= 0 && cloneIndex < pProject->m_clones.size());
        assert(isValidRange);

        if (isValidRange)
        {
            bool isPathEmpty = sourceFilePath.empty();
            assert(!isPathEmpty);
            if (!isPathEmpty)
            {
                std::shared_ptr<rgProjectClone> pClone = pProject->m_clones[cloneIndex];

                // Add a new source file info instance to the list of source files in the clone.
                rgSourceFileInfo newSourceFile = {};
                newSourceFile.m_filePath = sourceFilePath;
                pClone->m_sourceFiles.push_back(newSourceFile);
            }
        }
    }
}

void rgConfigManager::AddShaderStage(rgPipelineStage stage, const std::string& sourceFilePath, std::shared_ptr<rgProject> pProject, int cloneIndex) const
{
    assert(pProject != nullptr);
    if (pProject != nullptr)
    {
        // Ensure that the incoming clone index is valid for the current project.
        bool isValidRange = (cloneIndex >= 0 && cloneIndex < pProject->m_clones.size());
        assert(isValidRange);

        if (isValidRange)
        {
            bool isPathEmpty = sourceFilePath.empty();
            assert(!isPathEmpty);
            if (!isPathEmpty)
            {
                std::shared_ptr<rgProjectClone> pClone = pProject->m_clones[cloneIndex];

                std::shared_ptr<rgGraphicsProjectClone> pGraphicsClone =
                    std::dynamic_pointer_cast<rgGraphicsProjectClone>(pClone);

                // Add a new source file info instance to the list of source files in the clone.
                size_t stageIndex = static_cast<size_t>(stage);
                pGraphicsClone->m_pipeline.m_shaderStages[stageIndex] = sourceFilePath;
            }
        }
    }
}

bool rgConfigManager::GetShaderStageFilePath(rgPipelineStage stage, std::shared_ptr<rgGraphicsProjectClone> pGraphicsClone, std::string& fullFilePath) const
{
    // Find the input file path for given pipeline stage shader.
    size_t stageIndex = static_cast<size_t>(stage);
    const std::string& inputFilePath = pGraphicsClone->m_pipeline.m_shaderStages[stageIndex];

    bool stageInUse = !inputFilePath.empty();
    if (stageInUse)
    {
        fullFilePath = inputFilePath;
    }

    return stageInUse;
}

void rgConfigManager::RemoveShaderStage(rgPipelineStage stage, std::shared_ptr<rgGraphicsProjectClone> pGraphicsClone) const
{
    assert(pGraphicsClone != nullptr);
    if (pGraphicsClone != nullptr)
    {
        // Clear out the path to the input shader file for the given stage.
        size_t stageIndex = static_cast<size_t>(stage);
        pGraphicsClone->m_pipeline.m_shaderStages[stageIndex].clear();
    }
}

rgProjectAPI rgConfigManager::GetCurrentAPI() const
{
    return m_currentAPI;
}

void rgConfigManager::SetCurrentAPI(rgProjectAPI projectAPI)
{
    m_currentAPI = projectAPI;
}

void rgConfigManager::SetDefaultAPI(rgProjectAPI defaultAPI)
{
    m_pGlobalSettings->m_defaultAPI = defaultAPI;
}

void rgConfigManager::SetPromptForAPIAtStartup(bool shouldPrompt)
{
    m_pGlobalSettings->m_shouldPromptForAPI = shouldPrompt;
}

std::string rgConfigManager::GetCurrentModeString() const
{
    std::string modeString = "Invalid";

    switch (m_currentAPI)
    {
    case rgProjectAPI::OpenCL:
        modeString = STR_MODE_STRING_OPENCL;
        break;
    case rgProjectAPI::Vulkan:
        modeString = STR_MODE_STRING_VULKAN;
        break;
    default:
        assert(false);
        break;
    }

    return modeString;
}

// A predicate used to find a given GPU family name within an ASIC architecture.
struct GpuFamilyNameFinder
{
    GpuFamilyNameFinder(const std::string& targetName) : m_targetName(targetName) {}

    // A predicate that will compare a GPU family's name with a target name.
    bool operator()(const rgGpuFamily& gpuFamily) const
    {
        return gpuFamily.m_familyName.compare(m_targetName) == 0;
    }

    // The target family name to search for.
    std::string m_targetName;
};

bool rgConfigManager::IsGpuFamilySupported(const std::string& familyName) const
{
    bool isSupported = false;

    assert(m_pVersionInfo != nullptr);
    if (m_pVersionInfo != nullptr)
    {
        // Get the list of supported architectures for the current mode.
        std::string currentMode = GetCurrentModeString();
        auto gpuArchitectureIter = m_pVersionInfo->m_gpuArchitectures.find(currentMode);
        if (gpuArchitectureIter != m_pVersionInfo->m_gpuArchitectures.end())
        {
            // Step through each architecture's family list to try to find the given name.
            std::vector<rgGpuArchitecture> supportedArchitectures = gpuArchitectureIter->second;

            for (const rgGpuArchitecture& currentArchitecture : supportedArchitectures)
            {
                const std::vector<rgGpuFamily>& families = currentArchitecture.m_gpuFamilies;

                // Search the list of GPU families for the incoming name.
                GpuFamilyNameFinder familyNameSearcher(familyName);
                auto familySearchIter = std::find_if(families.begin(), families.end(), familyNameSearcher);
                if (familySearchIter != families.end())
                {
                    isSupported = true;
                    break;
                }
            }
        }
    }

    return isSupported;
}

void rgConfigManager::RemoveSourceFilePath(std::shared_ptr<rgProject> pProject, int cloneIndex, const std::string& sourceFilePath) const
{
    assert(pProject != nullptr);
    if (pProject != nullptr)
    {
        // Ensure that the incoming clone index is valid for the current project.
        bool isValidRange = (cloneIndex >= 0 && cloneIndex < pProject->m_clones.size());
        assert(isValidRange);

        if (isValidRange)
        {
            // Empty the list of source files for the selected clone.
            std::shared_ptr<rgProjectClone> pClone = pProject->m_clones[cloneIndex];

            bool isPathEmpty = sourceFilePath.empty();
            assert(!isPathEmpty);
            if (!isPathEmpty)
            {
                // Search the list of source files in the clone to obtain an iterator to it.
                rgSourceFilePathSearcher sourceFileSearcher(sourceFilePath);
                auto sourceFileIter = std::find_if(pClone->m_sourceFiles.begin(), pClone->m_sourceFiles.end(), sourceFileSearcher);
                bool isFileInList = (sourceFileIter != pClone->m_sourceFiles.end());
                assert(isFileInList);

                // If the path was found in the clone's source files, erase it.
                if (isFileInList)
                {
                    pClone->m_sourceFiles.erase(sourceFileIter);
                }
            }
        }
    }
}

void rgConfigManager::GetProjectSourceFilePaths(std::shared_ptr<rgProject> pProject, int cloneIndex, std::vector<std::string>& sourceFilePaths) const
{
    assert(pProject != nullptr);
    if (pProject != nullptr)
    {
        // Ensure that the incoming clone index is valid for the current project.
        bool isValidRange = (cloneIndex >= 0 && cloneIndex < pProject->m_clones.size());
        assert(isValidRange);

        if (isValidRange)
        {
            std::shared_ptr<rgProjectClone> pClone = pProject->m_clones[cloneIndex];
            for (const rgSourceFileInfo& fileInfo : pClone->m_sourceFiles)
            {
                sourceFilePaths.push_back(fileInfo.m_filePath);
            }
        }
    }
}

void rgConfigManager::UpdateSourceFilepath(const std::string& oldFilepath, const std::string& newFilepath, std::shared_ptr<rgProject> pProject, int cloneIndex)
{
    // Ensure that the incoming clone index is valid for the current project.
    bool isValidRange = (cloneIndex >= 0 && cloneIndex < pProject->m_clones.size());
    assert(isValidRange);
    if (isValidRange)
    {
        // Use the clone given by the incoming index, and make sure it's valid.
        std::shared_ptr<rgProjectClone> pClone = pProject->m_clones[cloneIndex];
        assert(pClone);

        // Attempt to find the old file path referenced within the clone.
        rgSourceFilePathSearcher sourceFileSearcher(oldFilepath);
        auto fileIter = std::find_if(pClone->m_sourceFiles.begin(), pClone->m_sourceFiles.end(), sourceFileSearcher);

        // Verify that the old file path is referenced within the clone.
        bool oldFilepathFound = fileIter != pClone->m_sourceFiles.end();
        assert(oldFilepathFound);

        // If the old file path exists in the clone, remove it.
        if (oldFilepathFound)
        {
            pClone->m_sourceFiles.erase(fileIter);
        }

        // Add the updated file path to the list of source files for the clone.
        rgSourceFileInfo updatedFilePath = {};
        updatedFilePath.m_filePath = newFilepath;
        pClone->m_sourceFiles.push_back(updatedFilePath);
    }
}

void rgConfigManager::UpdateShaderStageFilePath(const std::string& oldFilepath, const std::string& newFilepath, std::shared_ptr<rgProjectVulkan> pProject, int cloneIndex)
{
    // Ensure that the incoming clone index is valid for the current project.
    bool isValidRange = (cloneIndex >= 0 && cloneIndex < pProject->m_clones.size());
    assert(isValidRange);
    if (isValidRange)
    {
        // Use the clone given by the incoming index, and make sure it's valid.
        std::shared_ptr<rgProjectCloneVulkan> pClone = std::dynamic_pointer_cast<rgProjectCloneVulkan>(pProject->m_clones[cloneIndex]);
        assert(pClone != nullptr);
        if (pClone != nullptr)
        {
            ShaderInputFileArray& stageArray = pClone->m_pipeline.m_shaderStages;
            for (int stageIndex = 0; stageIndex < stageArray.size(); ++stageIndex)
            {
                const std::string& stageFilePath = pClone->m_pipeline.m_shaderStages[stageIndex];
                if (stageFilePath.compare(oldFilepath) == 0)
                {
                    // Update the file path for the shader stage.
                    pClone->m_pipeline.m_shaderStages[stageIndex] = newFilepath;
                }
            }
        }
    }
}

void rgConfigManager::GetProjectFolder(const std::string& projectName, std::string& projectFolder)
{
    // Start with the default directory where project files live.
    std::string projectsFolder;
    rgConfigManager::GetDefaultProjectsFolder(projectsFolder);

    // Append the project name as a subdirectory.
    rgUtils::AppendFolderToPath(projectsFolder, projectName, projectFolder);
}

void rgConfigManager::GenerateNewSourceFilepath(const std::string& projectName, int cloneIndex, const std::string& sourceFilename, const std::string& sourcefileExtension, std::string& generatedFileName, std::string& fullSourceFilePath)
{
    // Get the path to the folder for the given project.
    std::string projectFolder;
    GetProjectFolder(projectName, projectFolder);

    // Generate a folder name for where the new clone source file will be stored.
    std::string cloneFolderName = rgUtils::GenerateCloneName(cloneIndex);

    // Append the clone folder to the project folder.
    std::string cloneFolderPath;
    rgUtils::AppendFolderToPath(projectFolder, cloneFolderName, cloneFolderPath);

    // Append a suffix number onto the end of a filename to make it unique.
    unsigned int suffix = 0;

    // Loop to generate a new filename with suffix until it's a unique item in the file menu.
    do
    {
        std::stringstream filenameStream;
        filenameStream << sourceFilename;
        if (suffix > 0)
        {
            // Only insert the suffix if it is non-zero, so that generated filenames
            // follow the pattern: src, src1, src2, src3, etc.
            filenameStream << suffix;
        }
        generatedFileName = filenameStream.str();
        filenameStream << sourcefileExtension;
        suffix++;

        // Store the path in the final destination, it will get tested in the while statement.
        rgUtils::AppendFileNameToPath(cloneFolderPath, filenameStream.str(), fullSourceFilePath);

    } while (rgUtils::IsFileExists(fullSourceFilePath));

    // Make sure all path separators are the same.
    rgUtils::StandardizePathSeparator(fullSourceFilePath);
}

void rgConfigManager::GenerateNewPipelineFilepath(const std::string& projectName, int cloneIndex, const std::string& pipelineFilename, const std::string& pipelineFileExtension, std::string& fullPipelineFilePath)
{
    // Get the path to the folder for the given project.
    std::string projectFolder;
    GetProjectFolder(projectName, projectFolder);

    // Generate the project's filename.
    std::stringstream pipelineFileStream;
    pipelineFileStream << pipelineFilename;
    pipelineFileStream << pipelineFileExtension;

    // Generate a folder name for where the new clone source file will be stored.
    std::string cloneFolderName = rgUtils::GenerateCloneName(cloneIndex);

    // Append the clone folder to the project folder.
    std::string cloneFolderPath;
    rgUtils::AppendFolderToPath(projectFolder, cloneFolderName, cloneFolderPath);

    // Generate the full path to the project file.
    rgUtils::AppendFileNameToPath(cloneFolderPath, pipelineFileStream.str(), fullPipelineFilePath);

    // Make sure all path separators are the same.
    rgUtils::StandardizePathSeparator(fullPipelineFilePath);
}

std::shared_ptr<rgProject> rgConfigManager::LoadProjectFile(const std::string& projectFilePath)
{
    std::shared_ptr<rgProject> pProject = nullptr;
    bool isProjectLoaded = rgXmlConfigFile::ReadProjectConfigFile(projectFilePath, pProject);

    bool isOk = isProjectLoaded && pProject != nullptr;
    assert(isOk);

    if (isOk)
    {
        auto pRecentProject = std::make_shared<rgRecentProject>();
        pRecentProject->projectPath = projectFilePath;
        pRecentProject->apiType = pProject->m_api;

        // Add this path to the list of recently loaded projects.
        AddRecentProjectPath(pRecentProject);
    }

    return pProject;
}

bool rgConfigManager::SaveProjectFile(std::shared_ptr<rgProject> pProject)
{
    bool ret = false;
    if (pProject != nullptr)
    {
        // When saving a project, update the project's path in the list of recent projects.
        const std::string projectFileFullPath = pProject->m_projectFileFullPath;
        assert(!projectFileFullPath.empty());

        if (m_pGlobalSettings != nullptr)
        {
            // Add this project if it does not exist.
            ProjectPathSearcher searcher(projectFileFullPath);
            auto projectPathIter = std::find_if(m_pGlobalSettings->m_recentProjects.begin(), m_pGlobalSettings->m_recentProjects.end(), searcher);
            if (projectPathIter == m_pGlobalSettings->m_recentProjects.end())
            {
                auto pRecentProject = std::make_shared<rgRecentProject>();
                pRecentProject->projectPath = projectFileFullPath;
                pRecentProject->apiType = pProject->m_api;

                AddRecentProjectPath(pRecentProject);
            }
        }

        std::string projectDirectory;
        rgUtils::ExtractFileDirectory(projectFileFullPath, projectDirectory);

        if (!rgUtils::IsDirExists(projectDirectory))
        {
            bool isDirCreated = rgUtils::CreateFolder(projectDirectory);
            assert(isDirCreated);
        }

        // Save the file.
        rgXmlConfigFile::WriteProjectConfigFile(*pProject, projectFileFullPath);
        ret = true;
    }
    return ret;
}

void rgConfigManager::RevertToDefaultBuildSettings(rgProjectAPI api)
{
    std::string apiString;
    bool isOk = rgUtils::ProjectAPIToString(api, apiString);

    assert(isOk);
    if (isOk)
    {
        bool apiStringIsNotEmpty = !apiString.empty();
        assert(apiStringIsNotEmpty);

        if (apiStringIsNotEmpty)
        {
            // Find the settings for the API being reset.
            auto defaultSettingsIter = m_pGlobalSettings->m_pDefaultBuildSettings.find(apiString);
            if (defaultSettingsIter != m_pGlobalSettings->m_pDefaultBuildSettings.end())
            {
                std::shared_ptr<rgBuildSettings> pDefaultBuildSettings = rgConfigManager::GetDefaultBuildSettings(api);
                assert(pDefaultBuildSettings);

                // Assign the API's default settings as the global settings.
                if (pDefaultBuildSettings != nullptr)
                {
                    m_pGlobalSettings->m_pDefaultBuildSettings[apiString] = pDefaultBuildSettings;
                }
            }
        }
    }
}

bool rgConfigManager::SaveGlobalConfigFile() const
{
    bool ret = false;

    // Get the path to the default configuration file.
    std::string configFileFullPath;
    rgConfigManager::GetDefaultDataFolder(configFileFullPath);

    // Append the global configuration file name to the path.
    std::string configFileName = GenConfigFileName();
    assert(!configFileName.empty());
    if (!configFileName.empty())
    {
        if (rgUtils::AppendFileNameToPath(configFileFullPath, configFileName, configFileFullPath))
        {
            // Write out the settings.
            ret = rgXmlConfigFile::WriteGlobalSettings(m_pGlobalSettings, configFileFullPath);
        }
    }

    return ret;
}

void rgConfigManager::AddRecentProjectPath(std::shared_ptr<rgRecentProject> pRecentProject)
{
    const rgConfigManager& configManager = rgConfigManager::Instance();
    std::shared_ptr<rgGlobalSettings> pGlobalSettings = configManager.GetGlobalConfig();

    // Create a copy of the filepath, because it may be modified below.
    assert(pRecentProject != nullptr);
    if (pRecentProject != nullptr)
    {
        const std::string pathToAdd = pRecentProject->projectPath;

        // Strips a string from path separator characters ("\, /").
        auto stripString = [](std::string& str)
        {
            str.erase(std::remove(str.begin(), str.end(), '\\'), str.end());
            str.erase(std::remove(str.begin(), str.end(), '/'), str.end());
        };

        // Strip the path from path separator characters.
        std::string strippedPath = pathToAdd;
        stripString(strippedPath);

        // Is the project already in the list of recent projects?
        auto pathIter = std::find_if(pGlobalSettings->m_recentProjects.begin(), pGlobalSettings->m_recentProjects.end(),
            [&](std::shared_ptr<rgRecentProject> pRecentProject)
        {
            // Strip the current string and compare.
            bool status = false;
            std::string str = "";
            assert(pRecentProject != nullptr);
            if (pRecentProject != nullptr)
            {
                str = pRecentProject->projectPath;
                stripString(str);
                status = (0 == str.compare(strippedPath));
            }
            return status;
        });

        // If the project already exists, remove it.
        if (pathIter != pGlobalSettings->m_recentProjects.end())
        {
            pGlobalSettings->m_recentProjects.erase(pathIter);
        }

        // Add the project to the top of the list.
        pGlobalSettings->m_recentProjects.push_back(pRecentProject);

        // If there are now more than the maximum number of recent projects, remove the oldest entry.
        if (pGlobalSettings->m_recentProjects.size() > MAX_RECENT_FILES)
        {
            auto oldestRecentPath = (*pGlobalSettings->m_recentProjects.begin())->projectPath;

            // Remove the entry with the above file path.
            ProjectPathSearcher searcher(oldestRecentPath);
            auto projectPathIter = std::find_if(pGlobalSettings->m_recentProjects.begin(), pGlobalSettings->m_recentProjects.end(), searcher);
            if (projectPathIter != pGlobalSettings->m_recentProjects.end())
            {
                pGlobalSettings->m_recentProjects.erase(projectPathIter);
            }
        }

        // Save the configuration file after adding the new entry.
        configManager.SaveGlobalConfigFile();
    }
}

void rgConfigManager::UpdateRecentProjectPath(const std::string& oldFilePath, const std::string& newFilePath, rgProjectAPI api)
{
    const rgConfigManager& configManager = rgConfigManager::Instance();
    std::shared_ptr<rgGlobalSettings> pGlobalSettings = configManager.GetGlobalConfig();

    // Is the project already in the list of recent projects?
    ProjectPathSearcher searcher(oldFilePath);
    auto projectPathIter = std::find_if(pGlobalSettings->m_recentProjects.begin(), pGlobalSettings->m_recentProjects.end(), searcher);
    if (projectPathIter != pGlobalSettings->m_recentProjects.end())
    {
        // Compute the vector index of the path being updated.
        int pathIndex = (projectPathIter - pGlobalSettings->m_recentProjects.begin());

        // Verify that the index is valid.
        bool isValidIndex = (pathIndex >= 0 && pathIndex < pGlobalSettings->m_recentProjects.size());
        assert(isValidIndex);
        if (isValidIndex)
        {
            // If the path was already in the list, remove it and re-add it to the end of the list. It's now the most recent one.
            assert(pGlobalSettings->m_recentProjects[pathIndex]);
            if (pGlobalSettings->m_recentProjects[pathIndex] != nullptr)
            {
                pGlobalSettings->m_recentProjects[pathIndex]->projectPath = newFilePath;
            }
        }
    }

    // Add the new path, which will bump it to the top, and save the config file.
    auto pRecentProject = std::make_shared<rgRecentProject>();
    pRecentProject->projectPath = newFilePath;
    pRecentProject->apiType = api;

    AddRecentProjectPath(pRecentProject);
}

// Reset the current API Build settings with those supplied.
void rgConfigManager::SetApiBuildSettings(const std::string& apiName, rgBuildSettings* pBuildSettings)
{
    assert(m_pGlobalSettings != nullptr);
    if (m_pGlobalSettings != nullptr)
    {
        if (apiName.compare(STR_API_NAME_OPENCL) == 0)
        {
            std::shared_ptr<rgBuildSettingsOpenCL> pApiBuildSettings = std::dynamic_pointer_cast<rgBuildSettingsOpenCL>(m_pGlobalSettings->m_pDefaultBuildSettings[apiName]);
            assert(pApiBuildSettings != nullptr);
            if (pApiBuildSettings != nullptr)
            {
                *pApiBuildSettings = *dynamic_cast<rgBuildSettingsOpenCL*>(pBuildSettings);
            }
        }
        else if (apiName.compare(STR_API_NAME_VULKAN) == 0)
        {
            std::shared_ptr<rgBuildSettingsVulkan> pApiBuildSettings = std::dynamic_pointer_cast<rgBuildSettingsVulkan>(m_pGlobalSettings->m_pDefaultBuildSettings[apiName]);
            assert(pApiBuildSettings != nullptr);
            if (pApiBuildSettings != nullptr)
            {
                *pApiBuildSettings = *dynamic_cast<rgBuildSettingsVulkan*>(pBuildSettings);
            }
        }
        else
        {
            assert(!"Unsupported apiName in SetApiBuildSettings");
        }
    }
}

std::shared_ptr<rgGlobalSettings> rgConfigManager::GetGlobalConfig() const
{
    return m_pGlobalSettings;
}

std::shared_ptr<rgCliVersionInfo> rgConfigManager::GetVersionInfo() const
{
    return m_pVersionInfo;
}

void rgConfigManager::Close() const
{
    SaveGlobalConfigFile();
    rgLog::file << STR_LOG_CLOSING_RGA_GUI << std::endl;
    rgaSharedUtils::CloseLogFile();
}

std::shared_ptr<rgBuildSettings> rgConfigManager::GetDefaultBuildSettings(rgProjectAPI api)
{
    return DefaultConfigFactory::GetDefaultBuildSettings(api);
}

void rgConfigManager::GetDefaultDataFolder(std::string& defaultDataFolder)
{
    // Build the output path only once, then cache the result for subsequent calls.
    static std::string outputPath;
    if (outputPath.empty())
    {
        QString systemDefaultDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        rgUtils::AppendFolderToPath(systemDefaultDataPath.toStdString(), STR_APP_FOLDER_NAME, outputPath);
    }
    defaultDataFolder = outputPath;
}

void rgConfigManager::GetDefaultDocumentsFolder(std::string& defaultDocumentsFolder)
{
    // Build the output path only once, then cache the result for subsequent calls.
    static std::string outputPath;
    if (outputPath.empty())
    {
        QString systemDefaultDataPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
        rgUtils::AppendFolderToPath(systemDefaultDataPath.toStdString(), STR_APP_FOLDER_NAME, outputPath);
    }
    defaultDocumentsFolder = outputPath;
}

void rgConfigManager::GetDefaultProjectsFolder(std::string& defaultProjectsFolder)
{
    // Projects are saved under the system's "Documents" folder.
    rgConfigManager::GetDefaultDocumentsFolder(defaultProjectsFolder);
    rgUtils::AppendFolderToPath(defaultProjectsFolder, STR_PROJECTS_FOLDER_NAME, defaultProjectsFolder);
}

std::shared_ptr<rgBuildSettings> rgConfigManager::GetUserGlobalBuildSettings(rgProjectAPI api) const
{
    std::shared_ptr<rgBuildSettings> pRet = nullptr;
    if (m_pGlobalSettings != nullptr)
    {
        // Translate the project API to string.
        std::string apiName;
        bool isOk = rgUtils::ProjectAPIToString(api, apiName);
        if (isOk && !apiName.empty())
        {
            // Get the relevant build settings.
            auto iter = m_pGlobalSettings->m_pDefaultBuildSettings.find(apiName);
            if (iter != m_pGlobalSettings->m_pDefaultBuildSettings.cend())
            {
                pRet = iter->second;
            }
        }
    }

    assert(pRet != nullptr);
    return pRet;
}

std::string rgConfigManager::GetLastSelectedFolder() const
{
    static const char* DEFAULT_FOLDER = "./";
    std::string ret = DEFAULT_FOLDER;

    // Get global configuration file.
    std::shared_ptr<rgGlobalSettings> pGlobalConfig = rgConfigManager::Instance().GetGlobalConfig();

    // Only return if config file and string are valid, otherwise use default folder.
    if (pGlobalConfig != nullptr && pGlobalConfig->m_lastSelectedDirectory != "")
    {
        ret = pGlobalConfig->m_lastSelectedDirectory;
    }

    return ret;
}

std::vector<std::shared_ptr<rgRecentProject>> rgConfigManager::GetRecentProjects() const
{
    assert(m_pGlobalSettings != nullptr);
    bool isOk = (m_pGlobalSettings != nullptr);
    return (isOk ? m_pGlobalSettings->m_recentProjects : std::vector<std::shared_ptr<rgRecentProject>>());
}

void rgConfigManager::SetGlobalConfig(const rgGlobalSettings& globalSettings)
{
    assert(m_pGlobalSettings != nullptr);

    // Never allow reseting the pointer to the global config, there should be only one such pointer.
    // This function only makes sense when the object is initialized. In future, we need to make sure that this
    // function accepts a value, and not a pointer.
    if (m_pGlobalSettings != nullptr)
    {
        // Replace the global settings with the incoming instance.
        *m_pGlobalSettings = globalSettings;
    }
}

void rgConfigManager::SetLastSelectedDirectory(const std::string& lastSelectedDirectory)
{
    assert(m_pGlobalSettings != nullptr);
    if (m_pGlobalSettings != nullptr)
    {
        m_pGlobalSettings->m_lastSelectedDirectory = lastSelectedDirectory;
    }
}

std::string rgConfigManager::GenerateProjectFilepath(const std::string& projectName)
{
    // Get the path to the folder for the given project.
    std::string projectFolder;
    GetProjectFolder(projectName, projectFolder);

    // Generate the project's filename.
    std::stringstream projectFilename;
    projectFilename << projectName;
    projectFilename << STR_PROJECT_FILE_EXTENSION;

    // Generate the full path to the project file.
    std::string fullProjectPath;
    rgUtils::AppendFileNameToPath(projectFolder, projectFilename.str(), fullProjectPath);

    return fullProjectPath;
}

void rgConfigManager::SetSplitterValues(const std::string& splitterName, const std::vector<int>& splitterValues)
{
    assert(m_pGlobalSettings != nullptr);
    if (m_pGlobalSettings != nullptr)
    {
        bool isNewItem = true;

        // Find name match.
        for (rgSplitterConfig& splitterConfig : m_pGlobalSettings->m_guiLayoutSplitters)
        {
            // Name matches.
            if (splitterConfig.m_splitterName == splitterName)
            {
                splitterConfig.m_splitterValues = splitterValues;
                isNewItem = false;
                break;
            }
        }

        // Create a new config item if necessary.
        if (isNewItem)
        {
            // Construct new splitter config item.
            rgSplitterConfig newSplitterConfig;
            newSplitterConfig.m_splitterName = splitterName;
            newSplitterConfig.m_splitterValues = splitterValues;

            // Add the new config item.
            m_pGlobalSettings->m_guiLayoutSplitters.push_back(newSplitterConfig);
        }
    }
}

bool rgConfigManager::GetSplitterValues(const std::string& splitterName, std::vector<int>& splitterValues) const
{
    bool ret = false;
    assert(m_pGlobalSettings != nullptr);
    if (m_pGlobalSettings != nullptr)
    {
        // Find name match.
        for (rgSplitterConfig& splitterConfig : m_pGlobalSettings->m_guiLayoutSplitters)
        {
            // Name matches.
            if (splitterConfig.m_splitterName == splitterName)
            {
                splitterValues = splitterConfig.m_splitterValues;
                ret = true;
                break;
            }
        }
    }

    return ret;
}

void rgConfigManager::SetDisassemblyColumnVisibility(const std::vector<bool>& columnVisibility)
{
    assert(m_pGlobalSettings != nullptr);
    if (m_pGlobalSettings != nullptr)
    {
        assert(m_pGlobalSettings->m_visibleDisassemblyViewColumns.size() == columnVisibility.size());
        m_pGlobalSettings->m_visibleDisassemblyViewColumns = columnVisibility;
    }
}

std::string rgConfigManager::GetFatalErrorMessage() const
{
    return m_fatalErrorMsg;
}

void rgConfigManager::GetSupportedApis(std::vector<std::string>& supportedAPIs)
{
    for (auto const& buildSetting : m_pGlobalSettings->m_pDefaultBuildSettings)
    {
        supportedAPIs.push_back(buildSetting.first);
    }
}

const std::string & rgConfigManager::GetCLILogFilePath()
{
    return m_cliLogFilePath;
}

std::string rgConfigManager::GetIncludeFileViewer() const
{
    // By default, return the system default option.
    std::string ret = STR_GLOBAL_SETTINGS_SRC_VIEW_INCLUDE_VIEWER_DEFAULT;
    assert(m_pGlobalSettings != nullptr);
    if (m_pGlobalSettings != nullptr)
    {
        // Get the user's app of choice.
        ret = m_pGlobalSettings->m_includeFilesViewer;
    }
    return ret;
}

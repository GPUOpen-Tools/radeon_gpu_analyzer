// C++.
#include <algorithm>
#include <cassert>
#include <sstream>

// Qt.
#include <QStandardPaths>

// Infra.
#include <RadeonGPUAnalyzerGUI/../Utils/include/rgaSharedUtils.h>
#include <RadeonGPUAnalyzerGUI/../Utils/include/rgLog.h>

// Local.
#include <RadeonGPUAnalyzerGUI/include/qt/rgIsaDisassemblyTableModel.h>
#include <RadeonGPUAnalyzerGUI/include/rgConfigManager.h>
#include <RadeonGPUAnalyzerGUI/include/rgConfigFile.h>
#include <RadeonGPUAnalyzerGUI/include/rgCliLauncher.h>
#include <RadeonGPUAnalyzerGUI/include/rgDataTypes.h>
#include <RadeonGPUAnalyzerGUI/include/rgDefinitions.h>
#include <RadeonGPUAnalyzerGUI/include/rgStringConstants.h>
#include <RadeonGPUAnalyzerGUI/include/rgXMLCliVersionInfo.h>
#include <RadeonGPUAnalyzerGUI/include/rgUtils.h>

// The maximum number of recent files that the application is aware of.
static const int MAX_RECENT_FILES = 10;

// The API type that the rgConfigManager will be initialized with.
static const rgProjectAPI INITIAL_API_TYPE = rgProjectAPI::OpenCL;

// The default number of days for log files to be considered as "old".
static const int  DEFAULT_OLD_LOG_FILES_DAYS = 3;

// The mode used to compile the user's kernels.
static const char* INITIAL_MODE = "rocm-cl";

// GUI log file name prefix.
static const char* GUI_LOG_FILE_PREFIX = "rga-gui.log";

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
        case OpenCL:
            pRet = CreateCLDefaultBuildSettings();
            break;
        case Unknown:
        default:
            break;
        }
        return pRet;
    }

private:

    // Creates the default OpenCL build settings.
    static std::shared_ptr<rgCLBuildSettings> CreateCLDefaultBuildSettings()
    {
        std::shared_ptr<rgCLBuildSettings> pRet = std::make_shared<rgCLBuildSettings>();

        // Use the version info's list of supported GPUs to determine the most recent
        // hardware to build for by default.
        rgConfigManager& configManager = rgConfigManager::Instance();
        std::shared_ptr<rgCliVersionInfo> pVersionInfo = configManager.GetVersionInfo();

        assert(pVersionInfo != nullptr);
        if (pVersionInfo != nullptr)
        {
            // Find the set of GPU architectures supported in the current mode.
            const std::string& apiMode = configManager.GetCurrentMode();
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
                pRet->m_targetGpus.push_back(latestFamilyName);
            }
        }

        return pRet;
    }
};

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

        // Initialize the mode used to compile kernels.
        m_currentMode = INITIAL_MODE;

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
            configFileFullPath = appDataDir;
            rgUtils::AppendFileNameToPath(configFileFullPath, STR_GLOBAL_CONFIG_FILE_NAME, configFileFullPath);

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
                std::shared_ptr<rgBuildSettings> pDefaultBuildSettings = GetDefaultBuildSettings(INITIAL_API_TYPE);
                m_pGlobalSettings = std::make_shared<rgGlobalSettings>();

                // Initialize default values in the new global settings instance.
                InitializeDefaultGlobalConfig(m_pGlobalSettings);

                std::string defaultApiName;
                rgUtils::ProjectAPIToString(INITIAL_API_TYPE, defaultApiName);
                m_pGlobalSettings->m_pDefaultBuildSettings[defaultApiName] = pDefaultBuildSettings;

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
            // Initialize log file.
            std::string  logFileDir = (m_pGlobalSettings->m_logFileLocation.empty() ? appDataDir : m_pGlobalSettings->m_logFileLocation);
            bool isSuccessful = rgaSharedUtils::InitLogFile(logFileDir, GUI_LOG_FILE_PREFIX, DEFAULT_OLD_LOG_FILES_DAYS);
            assert(isSuccessful);
            if (isSuccessful)
            {
                rgLog::file << STR_LOG_RGA_GUI_STARTED << std::endl;
            }
        }
    }

    return m_isInitialized;
}

void rgConfigManager::InitializeDefaultGlobalConfig(std::shared_ptr<rgGlobalSettings> pGlobalSettings) const
{
    assert(pGlobalSettings != nullptr);
    if (pGlobalSettings != nullptr)
    {
        // Initialize the default log file location.
        rgConfigManager::Instance().GetDefaultDataFolder(pGlobalSettings->m_logFileLocation);

        // Initialize the visible columns in the ISA disassembly table.
        // Only the Address, Opcode and Operands columns are visible by default.
        pGlobalSettings->m_visibleDisassemblyViewColumns =
        {
            true,   // rgIsaDisassemblyTableColumns::Address
            true,   // rgIsaDisassemblyTableColumns::Opcode
            true,   // rgIsaDisassemblyTableColumns::Operands
            false,  // rgIsaDisassemblyTableColumns::FunctionalUnit
            false,  // rgIsaDisassemblyTableColumns::Cycles
            false,  // rgIsaDisassemblyTableColumns::BinaryEncoding
        };

        // Default to always asking the user for a name when creating a new project.
        pGlobalSettings->m_useDefaultProjectName = false;
    }
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

rgProjectAPI rgConfigManager::GetCurrentAPI() const
{
    return m_currentAPI;
}

const std::string& rgConfigManager::GetCurrentMode() const
{
    return m_currentMode;
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
        auto gpuArchitectureIter = m_pVersionInfo->m_gpuArchitectures.find(m_currentMode);
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

void rgConfigManager::UpdateSourceFilepath(const std::string& oldFilepath, const std::string& newFilepath, std::shared_ptr<rgProject> pProject, int cloneIndex) const
{
    // Ensure that the incoming clone index is valid for the current project.
    bool isValidRange = (cloneIndex >= 0 && cloneIndex < pProject->m_clones.size());
    assert(isValidRange);

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

void rgConfigManager::GetProjectFolder(const std::string& projectName, std::string& projectFolder)
{
    // Start with the default directory where project files live.
    std::string projectsFolder;
    rgConfigManager::GetDefaultProjectsFolder(projectsFolder);

    // Append the project name as a subdirectory.
    rgUtils::AppendFolderToPath(projectsFolder, projectName, projectFolder);
}

void rgConfigManager::GenerateNewSourceFilepath(const std::string& projectName, int cloneIndex, const std::string& sourceFilename, const std::string sourcefileExtension, std::string& fullSourcefilePath) const
{
    // Get the path to the folder for the given project.
    std::string projectFolder;
    GetProjectFolder(projectName, projectFolder);

    // Generate the project's filename.
    std::stringstream filename;
    filename << sourceFilename;
    filename << sourcefileExtension;

    // Generate a folder name for where the new clone source file will be stored.
    std::string cloneFolderName = rgUtils::GenerateCloneName(cloneIndex);

    // Append the clone folder to the project folder.
    std::string cloneFolderPath;
    rgUtils::AppendFolderToPath(projectFolder, cloneFolderName, cloneFolderPath);

    // Generate the full path to the project file.
    rgUtils::AppendFileNameToPath(cloneFolderPath, filename.str(), fullSourcefilePath);
}

std::shared_ptr<rgProject> rgConfigManager::LoadProjectFile(const std::string& projectFilePath)
{
    std::shared_ptr<rgProject> pProject = nullptr;
    bool isProjectLoaded = rgXmlConfigFile::ReadProjectConfigFile(projectFilePath, pProject);

    bool isOk = isProjectLoaded && pProject != nullptr;
    assert(isOk);

    if (isOk)
    {
        // Add this path to the list of recently loaded projects.
        AddRecentProjectPath(projectFilePath);
    }

    return pProject;
}

bool rgConfigManager::SaveProjectFile(std::shared_ptr<rgProject> pProject) const
{
    bool ret = false;
    if (pProject != nullptr)
    {
        // When saving a project, update the project's path in the list of recent project.
        const std::string projectFileFullPath = pProject->m_projectFileFullPath;
        assert(!projectFileFullPath.empty());

        std::shared_ptr<rgGlobalSettings> pGlobalSettings = rgConfigManager::Instance().GetGlobalConfig();
        if (pGlobalSettings != nullptr && std::find(pGlobalSettings->m_recentProjects.begin(), pGlobalSettings->m_recentProjects.end(),
            projectFileFullPath) == pGlobalSettings->m_recentProjects.end())
        {
            AddRecentProjectPath(projectFileFullPath);
        }

        std::string projectDirectory;
        rgUtils::ExtractFileDirectory(projectFileFullPath, projectDirectory);

        if (!rgUtils::IsDirExists(projectDirectory))
        {
            bool isDirCreated = rgUtils::CreateFolder(projectDirectory);
            assert(isDirCreated);
        }

        // Save the file.
        rgXmlConfigFile::WriteConfigFile(*pProject, projectFileFullPath);
        ret = true;
    }
    return ret;
}

void rgConfigManager::RevertToDefaultBuildSettings(rgProjectAPI api)
{
    std::string apiString;
    switch (api)
    {
    case OpenCL:
    {
        apiString = STR_API_NAME_OPENCL;
    }
    break;
    default:
        // Need to update the switch to handle more APIs.
        assert(false);
        break;
    }

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

bool rgConfigManager::SaveGlobalConfigFile() const
{
    // Get the path to the default configuration file.
    std::string configFileFullPath;
    rgConfigManager::GetDefaultDataFolder(configFileFullPath);

    // Append the global configuration file name to the path.
    rgUtils::AppendFileNameToPath(configFileFullPath, STR_GLOBAL_CONFIG_FILE_NAME, configFileFullPath);

    // Write out the settings.
    return rgXmlConfigFile::WriteGlobalSettings(m_pGlobalSettings, configFileFullPath);
}

void rgConfigManager::AddRecentProjectPath(const std::string& projectFilePath)
{
    const rgConfigManager& configManager = rgConfigManager::Instance();
    std::shared_ptr<rgGlobalSettings> pGlobalSettings = configManager.GetGlobalConfig();

    // Create a copy of the filepath, because it may be modified below.
    const std::string pathToAdd = projectFilePath;

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
        [&](std::string str)
    {
        // Strip the current string and compare.
        stripString(str);
        return (0 == str.compare(strippedPath));
    });

    // If the project already exists, remove it.
    if (pathIter != pGlobalSettings->m_recentProjects.end())
    {
        pGlobalSettings->m_recentProjects.erase(pathIter);
    }

    // Add the project to the top of the list.
    pGlobalSettings->m_recentProjects.push_back(pathToAdd);

    // If there are now more than the maximum number of recent project, remove the oldest entry.
    if (pGlobalSettings->m_recentProjects.size() > MAX_RECENT_FILES)
    {
        auto oldestRecentPath = pGlobalSettings->m_recentProjects.begin();
        pGlobalSettings->m_recentProjects.erase(oldestRecentPath);
    }

    // Save the configuration file after adding the new entry.
    configManager.SaveGlobalConfigFile();
}

void rgConfigManager::UpdateRecentProjectPath(const std::string& oldFilePath, const std::string& newFilePath)
{
    const rgConfigManager& configManager = rgConfigManager::Instance();
    std::shared_ptr<rgGlobalSettings> pGlobalSettings = configManager.GetGlobalConfig();

    // Is the project already in the list of recent projects?
    auto pathIter = std::find(pGlobalSettings->m_recentProjects.begin(), pGlobalSettings->m_recentProjects.end(), oldFilePath);
    if (pathIter != pGlobalSettings->m_recentProjects.end())
    {
        // Compute the vector index of the path being updated.
        int pathIndex = (pathIter - pGlobalSettings->m_recentProjects.begin());

        // Verify that the index is valid.
        bool isValidIndex = (pathIndex >= 0 && pathIndex < pGlobalSettings->m_recentProjects.size());
        assert(isValidIndex);
        if (isValidIndex)
        {
            // If the path was already in the list, remove it and re-add it to the end of the list. It's now the most recent.
            pGlobalSettings->m_recentProjects[pathIndex] = newFilePath;
        }
    }

    // Add the new path, which will bump it to the top, and save the config file.
    AddRecentProjectPath(newFilePath);
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

std::string rgConfigManager::GetLastSelectedFolder()
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

std::vector<std::string> rgConfigManager::GetRecentProjects()
{
    std::shared_ptr<rgGlobalSettings> pGlobalConfig = rgConfigManager::Instance().GetGlobalConfig();
    bool isOk = (pGlobalConfig != nullptr);
    return (isOk ? pGlobalConfig->m_recentProjects : std::vector<std::string>());
}

void rgConfigManager::SetGlobalConfig(std::shared_ptr<rgGlobalSettings> pGlobalSettings)
{
    assert(pGlobalSettings != nullptr);

    // Never allow reseting the pointer to the global config, there should be only one such pointer.
    // This function only makes sense when the object is initialized. In future, we need to make sure that this
    // function accepts a value, and not a pointer.
    assert(m_pGlobalSettings == nullptr || m_pGlobalSettings == pGlobalSettings);
    if (pGlobalSettings != nullptr)
    {
        // Replace the global settings with the incoming instance.
        m_pGlobalSettings = pGlobalSettings;
    }
}

void rgConfigManager::SetLastSelectedDirectory(const std::string& lastSelectedDirectory)
{
    std::shared_ptr<rgGlobalSettings> pGlobalConfig = rgConfigManager::Instance().GetGlobalConfig();
    pGlobalConfig->m_lastSelectedDirectory = lastSelectedDirectory;
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
    bool isNewItem = true;

    std::shared_ptr<rgGlobalSettings> pGlobalConfig = rgConfigManager::Instance().GetGlobalConfig();

    // Find name match.
    for (rgSplitterConfig& splitterConfig : pGlobalConfig->m_guiLayoutSplitters)
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
        pGlobalConfig->m_guiLayoutSplitters.push_back(newSplitterConfig);
    }
}

bool rgConfigManager::GetSplitterValues(const std::string& splitterName, std::vector<int>& splitterValues)
{
    bool ret = false;

    std::shared_ptr<rgGlobalSettings> pGlobalConfig = rgConfigManager::Instance().GetGlobalConfig();

    // Find name match.
    for (rgSplitterConfig& splitterConfig : pGlobalConfig->m_guiLayoutSplitters)
    {
        // Name matches.
        if (splitterConfig.m_splitterName == splitterName)
        {
            splitterValues = splitterConfig.m_splitterValues;
            ret = true;
            break;
        }
    }

    return ret;
}

std::string rgConfigManager::GetFatalErrorMessage() const
{
    return m_fatalErrorMsg;
}

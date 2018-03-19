#pragma once

// C++.
#include <vector>
#include <string>
#include <memory>

// Forward declarations.
struct rgProject;
struct rgBuildSettings;
struct rgGlobalSettings;
struct rgCliVersionInfo;
struct rgSourceFileInfo;
enum rgProjectAPI : char;

// A structure with a predicate used to search a rgSourceFileInfo vector for a specific file path.
struct rgSourceFilePathSearcher
{
    // Default constructor used to provide the path to search for.
    rgSourceFilePathSearcher(const std::string& targetFilePath) : m_targetFilePath(targetFilePath) {}

    // An overloaded function call operator used to compare file paths between rgSourceFileInfo instances.
    bool operator()(const rgSourceFileInfo& fileInfo) const;

    // The target file path to search for.
    std::string m_targetFilePath;
};

class rgConfigManager
{
public:
    // Initializes the configuration manager.
    // Call this function once during startup.
    bool Init();

    // Initialize the incoming global settings to the default values.
    void InitializeDefaultGlobalConfig(std::shared_ptr<rgGlobalSettings> pGlobalSettings) const;

    // Get the single instance of the configuration manager.
    static rgConfigManager& Instance();

    // Add the given source file path to the provided program.
    void AddSourceFileToProject(const std::string& sourceFilePath, std::shared_ptr<rgProject> pProgram, int cloneIndex) const;

    // Gets the current API being used
    rgProjectAPI GetCurrentAPI() const;

    // Gets the current mode used to build the user's kernels.
    const std::string& GetCurrentMode() const;

    // Check if the given GPU family is supported in the current mode.
    bool IsGpuFamilySupported(const std::string& familyName) const;

    // Remove a source file path from the given program clone.
    void RemoveSourceFilePath(std::shared_ptr<rgProject> pProgram, int cloneIndex, const std::string& sourceFilePath) const;

    // Retrieve a source file path from a program by clone index.
    void GetProjectSourceFilePaths(std::shared_ptr<rgProject> pProgram, int cloneIndex, std::vector<std::string>& sourceFilePaths) const;

    // Update the file path to a file that has already been added to a program clone.
    void UpdateSourceFilepath(const std::string& oldFilepath, const std::string& newFilepath, std::shared_ptr<rgProject> pProgram, int cloneIndex) const;

    // Generate a full path to a program's folder on disk.
    static void GetProjectFolder(const std::string& programName, std::string& programFolder);

    // Generate a full path to a new sourcefile for the given program.
    void GenerateNewSourceFilepath(const std::string& programName, int cloneIndex, const std::string& sourceFilename, const std::string sourcefileExtension, std::string& fullSourcefilePath) const;

    // Load an existing program.
    std::shared_ptr<rgProject> LoadProjectFile(const std::string& programFilePath);

    // Save the program File.
    bool SaveProjectFile(std::shared_ptr<rgProject>) const;

    // Generate a full path to a program file.
    static std::string GenerateProjectFilepath(const std::string& programName);

    // Revert the build settings to the default for the given API.
    void RevertToDefaultBuildSettings(rgProjectAPI api);

    // Get the global settings.
    std::shared_ptr<rgGlobalSettings> GetGlobalConfig() const;

    // Get the GPUs supported by RGA.
    std::shared_ptr<rgCliVersionInfo> GetVersionInfo() const;

    // Save and close configuration/log files.
    void Close() const;

    // Save the global configuration file.
    bool SaveGlobalConfigFile() const;

    // Add a recent program to the list of recently-accessed programs.
    static void AddRecentProjectPath(const std::string& programFilePath);

    // Update a recent program's file path.
    static void UpdateRecentProjectPath(const std::string& oldFilePath, const std::string& newFilePath);

    // Get the user's global build settings for the given API.
    // These are the default settings that the user would like to use for the API.
    std::shared_ptr<rgBuildSettings> GetUserGlobalBuildSettings(rgProjectAPI api) const;

    // Get a read-only version of the RGA default build settings for the given API.
    // Use this for performing a "factory reset", i.e. when the user wants to reset the settings to the default.
    static std::shared_ptr<rgBuildSettings> GetDefaultBuildSettings(rgProjectAPI api);

    // Gets the system's default folder where RGA should store data.
    static void GetDefaultDataFolder(std::string& defaultDataFolder);

    // Gets the system's default Documents folder where RGA should store project data.
    static void GetDefaultDocumentsFolder(std::string& defaultDocumentsFolder);

    // Gets the default folder where RGA programs should stored.
    static void GetDefaultProjectsFolder(std::string& defaultProgramsFolder);

    // Gets the last folder that the user selected in a File Dialog.
    static std::string GetLastSelectedFolder();

    // Retrieve the array of recently-accessed program files.
    static std::vector<std::string> GetRecentProjects();

    // Replace the global settings structure with the provided instance.
    void SetGlobalConfig(std::shared_ptr<rgGlobalSettings> pGlobalSettings);

    // Set the last directory navigated to using a file browser dialog.
    static void SetLastSelectedDirectory(const std::string& lastSelectedDirectory);

    // Set the splitter values for the given splitter name.
    static void SetSplitterValues(const std::string& splitterName, const std::vector<int>& splitterValues);

    // Get the splitter values for the given splitter name.
    static bool GetSplitterValues(const std::string& splitterName, std::vector<int>& splitterValues);

    // An error message for fatal scenarios that do not allow RGA to be initialized.
    std::string GetFatalErrorMessage() const;

    // The delimiter character which is used across the system to build and disassemble argument list.
    static const char RGA_LIST_DELIMITER = ';';

private:
    // Non-copyable.
    rgConfigManager() = default;
    ~rgConfigManager() = default;
    rgConfigManager(const rgConfigManager& other);
    const rgConfigManager& operator=(const rgConfigManager& other);

    // A pointer to the global settings.
    std::shared_ptr<rgGlobalSettings> m_pGlobalSettings = nullptr;

    // A pointer to the structure containing the CLI version info.
    std::shared_ptr<rgCliVersionInfo> m_pVersionInfo = nullptr;

    // The current API being used
    rgProjectAPI m_currentAPI;

    // The current build mode being used.
    std::string m_currentMode;

    // An error message for fatal scenarios that do not allow RGA to be initialized.
    std::string m_fatalErrorMsg;

    // A flag that indicates if this object has been initialized.
    bool m_isInitialized = false;
};
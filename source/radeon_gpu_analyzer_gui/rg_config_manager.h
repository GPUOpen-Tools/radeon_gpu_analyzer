#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_CONFIG_MANAGER_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_CONFIG_MANAGER_H_

// C++.
#include <vector>
#include <string>
#include <memory>

// Forward declarations.
struct RgProject;
struct RgBuildSettings;
struct RgGlobalSettings;
struct RgGraphicsProjectClone;
struct RgProjectVulkan;
struct RgCliVersionInfo;
struct RgSourceFileInfo;
struct RgRecentProject;
struct RgWindowConfig;
enum class RgProjectAPI : char;
enum RgPipelineStage : char;

// A structure with a predicate used to search a RgSourceFileInfo vector for a specific file path.
struct RgSourceFilePathSearcher
{
    // Default constructor used to provide the path to search for.
    RgSourceFilePathSearcher(const std::string& target_file_path) : target_file_path(target_file_path) {}

    // An overloaded function call operator used to compare file paths between RgSourceFileInfo instances.
    bool operator()(const RgSourceFileInfo& file_info) const;

    // The target file path to search for.
    std::string target_file_path;
};

class RgConfigManager
{
public:
    // Initializes the configuration manager.
    // Call this function once during startup.
    bool Init();

    // Reset the incoming global settings to the hardcoded default values.
    static void ResetToFactoryDefaults(RgGlobalSettings& global_settings);

    // Get the single instance of the configuration manager.
    static RgConfigManager& Instance();

    // Add the given source file path to the provided project.
    void AddSourceFileToProject(const std::string& source_file_path, std::shared_ptr<RgProject> program, int clone_index) const;

    // Add the given source file to a project's pipeline for the specified shader stage.
    void AddShaderStage(RgPipelineStage stage, const std::string& source_file_path, std::shared_ptr<RgProject> project, int clone_index) const;

    // Remove the shader stage from the given project's pipeline.
    bool GetShaderStageFilePath(RgPipelineStage stage, std::shared_ptr<RgGraphicsProjectClone> graphics_clone, std::string& full_file_path) const;

    // Remove the shader stage from the given project's pipeline.
    void RemoveShaderStage(RgPipelineStage stage, std::shared_ptr<RgGraphicsProjectClone> graphics_clone) const;

    // Gets the current API being used.
    RgProjectAPI GetCurrentAPI() const;

    // Sets the current API to be used.
    void SetCurrentAPI(RgProjectAPI project_api);

    // Sets whether or not the UI should prompt the user to select an API at startup.
    void SetPromptForAPIAtStartup(bool should_prompt);

    // Sets the default API to be used if not prompting for an API at startup.
    void SetDefaultAPI(RgProjectAPI default_api);

    // Gets the current mode used to build the user's kernels.
    std::string GetCurrentModeString() const;

    // Check if the given GPU family is supported in the current mode.
    bool IsGpuFamilySupported(const std::string& family_name) const;

    // Remove a source file path from the given program clone.
    void RemoveSourceFilePath(std::shared_ptr<RgProject> program, int clone_index, const std::string& source_file_path) const;

    // Retrieve a source file path from a program by clone index.
    void GetProjectSourceFilePaths(std::shared_ptr<RgProject> program, int clone_index, std::vector<std::string>& source_file_paths) const;

    // Update the file path to a file that has already been added to a program clone.
    static void UpdateSourceFilepath(const std::string& old_file_path, const std::string& new_file_path, std::shared_ptr<RgProject> program, int clone_index);

    // Update the file path for a shader stage source file that has been renamed.
    static void UpdateShaderStageFilePath(const std::string& old_file_path, const std::string& new_file_path, std::shared_ptr<RgProjectVulkan> project, int clone_index);

    // Generate a full path to a program's folder on disk.
    static void GetProjectFolder(const std::string& program_name, std::string& program_folder);

    // Generate a full path to a new source file for the given program.
    static void GenerateNewSourceFilepath(const std::string& program_name, int clone_index, const std::string& source_filename, const std::string& sourcefile_extension, std::string& generated_file_name, std::string& full_source_file_path);

    // Generate a full path to a new pipeline state file for the given program.
    static void GenerateNewPipelineFilepath(const std::string& project_name, int clone_index, const std::string& pipeline_filename, const std::string& pipeline_file_extension, std::string& full_pipeline_file_path);

    // Load an existing program.
    std::shared_ptr<RgProject> LoadProjectFile(const std::string& program_file_path);

    // Save the program File.
    bool SaveProjectFile(std::shared_ptr<RgProject>);

    // Generate a full path to a program file.
    static std::string GenerateProjectFilepath(const std::string& program_name);

    // Revert the build settings to the default for the given API.
    void RevertToDefaultBuildSettings(RgProjectAPI api);

    // Reset the current API Build settings with those supplied.
    void SetApiBuildSettings(const std::string& api_name, RgBuildSettings* build_settings);

    // Get the global settings.
    std::shared_ptr<RgGlobalSettings> GetGlobalConfig() const;

    // Get the GPUs supported by RGA.
    std::shared_ptr<RgCliVersionInfo> GetVersionInfo() const;

    // Save and close configuration/log files.
    void Close() const;

    // Save the global configuration file.
    bool SaveGlobalConfigFile() const;

    // Add a recent program to the list of recently-accessed programs.
    void AddRecentProjectPath(std::shared_ptr<RgRecentProject> recent_project);

    // Update a recent program's file path.
    void UpdateRecentProjectPath(const std::string& old_file_path, const std::string& new_file_path, RgProjectAPI api);

    // Get the user's global build settings for the given API.
    // These are the default settings that the user would like to use for the API.
    std::shared_ptr<RgBuildSettings> GetUserGlobalBuildSettings(RgProjectAPI api) const;

    // Get a read-only version of the RGA default build settings for the given API.
    // Use this for performing a "factory reset", i.e. when the user wants to reset the settings to the default.
    static std::shared_ptr<RgBuildSettings> GetDefaultBuildSettings(RgProjectAPI api);

    // Gets the system's default folder where RGA should store data.
    static void GetDefaultDataFolder(std::string& default_data_folder);

    // Gets the system's default Documents folder where RGA should store project data.
    static void GetDefaultDocumentsFolder(std::string& default_documents_folder);

    // Gets the default folder where RGA programs should stored.
    static void GetDefaultProjectsFolder(std::string& default_programs_folder);

    // Gets the last folder that the user selected in a File Dialog.
    std::string GetLastSelectedFolder() const;

    // Retrieve the array of recently-accessed program files.
    std::vector<std::shared_ptr<RgRecentProject>> GetRecentProjects() const;

    // Replace the global settings structure with the provided instance.
    void SetGlobalConfig(const RgGlobalSettings& global_settings);

    // Set the last directory navigated to using a file browser dialog.
    void SetLastSelectedDirectory(const std::string& last_selected_directory);

    // Set the splitter values for the given splitter name.
    void SetSplitterValues(const std::string& splitter_name, const std::vector<int>& splitter_values);

    // Get the splitter values for the given splitter name.
    bool GetSplitterValues(const std::string& splitter_name, std::vector<int>& splitter_values) const;

    // Set the GUI window geometry values.
    void SetWindowGeometry(int x_pos, int y_pos, int width, int height, int window_state);

    // Get the GUI window geometry values.
    void GetWindowGeometry(RgWindowConfig& window_values) const;

    // Set the column visibility based on the supplied vector.
    void SetDisassemblyColumnVisibility(const std::vector<bool>& column_visibility);

    // An error message for fatal scenarios that do not allow RGA to be initialized.
    std::string GetFatalErrorMessage() const;

    // Getter for the supported APIs.
    void GetSupportedApis(std::vector<std::string>& supported_apis);

    // Get path to CLI log file.
    const std::string& GetCLILogFilePath();

    // Get the default app for opening include files.
    std::string GetIncludeFileViewer() const;

    // Set the config file data model version.
    void SetConfigFileDataModelVersion(const std::string& data_model_version);

    // Get the config file data model version.
    std::string GetConfigFileDataModelVersion() const;

    // The delimiter character which is used across the system to build and disassemble argument list.
    static const char kRgaListDelimiter = ';';

private:
    // Non-copyable.
    RgConfigManager() = default;
    ~RgConfigManager() = default;
    RgConfigManager(const RgConfigManager& other);
    const RgConfigManager& operator=(const RgConfigManager& other);

    // A pointer to the global settings.
    std::shared_ptr<RgGlobalSettings> global_settings_ = nullptr;

    // A pointer to the structure containing the CLI version info.
    std::shared_ptr<RgCliVersionInfo> version_info_ = nullptr;

    // The current API being used
    RgProjectAPI current_api_;

    // An error message for fatal scenarios that do not allow RGA to be initialized.
    std::string fatal_error_msg_;

    // Path to the CLI log file.
    std::string cli_log_file_path_;

    // Current RGA config file data model version.
    std::string config_file_data_model_version_;

    // Current RGA project file data model version.
    std::string project_file_data_model_version_;

    // A flag that indicates if this object has been initialized.
    bool is_initialized_ = false;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_CONFIG_MANAGER_H_

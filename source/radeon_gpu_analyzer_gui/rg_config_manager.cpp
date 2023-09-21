// C++.
#include <algorithm>
#include <cassert>
#include <sstream>

// Qt.
#include <QStandardPaths>

// Infra.
#include "source/common/rga_shared_utils.h"
#include "source/common/rg_log.h"
#include "external/amdt_os_wrappers/Include/osFilePath.h"

// Local.
#include "radeon_gpu_analyzer_gui/rg_definitions.h"
#include "radeon_gpu_analyzer_gui/qt/rg_isa_disassembly_table_model.h"
#include "radeon_gpu_analyzer_gui/rg_config_manager.h"
#include "radeon_gpu_analyzer_gui/rg_config_file.h"
#include "radeon_gpu_analyzer_gui/rg_cli_launcher.h"
#include "radeon_gpu_analyzer_gui/rg_data_types.h"
#include "radeon_gpu_analyzer_gui/rg_data_types_opencl.h"
#include "radeon_gpu_analyzer_gui/rg_data_types_vulkan.h"
#include "radeon_gpu_analyzer_gui/rg_definitions.h"
#include "radeon_gpu_analyzer_gui/rg_string_constants.h"
#include "radeon_gpu_analyzer_gui/rg_xml_cli_version_info.h"
#include "radeon_gpu_analyzer_gui/rg_utils.h"

// The initial API that the application starts in.
static const RgProjectAPI kInitialApiType = RgProjectAPI::kOpenCL;

// The maximum number of recent files that the application is aware of.
static const int kMaxRecentFiles = 10;

// The default number of days for log files to be considered as "old".
static const int kDefaultOldLogFilesDays = 3;

// GUI log file name prefix.
static const char* kGuiLogFilePrefix = "rga-gui.log";

// CLI log file name prefix.
static const char* kCliLogFilePrefix = "rga-cli.log";

struct ProjectPathSearcher
{
    ProjectPathSearcher(const std::string& project_path)
        : project_path(project_path)
    {
    }

    // A predicate that will compare each project path with a target path to search for.
    bool operator()(const std::shared_ptr<RgRecentProject> recent_project) const
    {
        bool result = false;
        assert(recent_project != nullptr);
        if (recent_project != nullptr)
        {
            result = recent_project->project_path.compare(project_path) == 0;
        }

        return result;
    }

    // The target project name to search for.
    const std::string& project_path;
};

// This class creates default configuration-related objects.
class DefaultConfigFactory
{
public:
    // Creates the default build settings for the given API.
    static std::shared_ptr<RgBuildSettings> GetDefaultBuildSettings(RgProjectAPI api)
    {
        std::shared_ptr<RgBuildSettings> ret = nullptr;
        switch (api)
        {
        case RgProjectAPI::kOpenCL:
            ret = CreateDefaultBuildSettingsOpenCL();
            break;
        case RgProjectAPI::kVulkan:
            ret = CreateDefaultBuildSettingsVulkan();
            break;
        case RgProjectAPI::kUnknown:
        default:
            break;
        }
        return ret;
    }

private:
    // Add default target GPU hardware. Suitable default target GPUs are discovered by looking at
    // the list of supported devices and choosing the last item in the list. Items at the end of
    // the list are the most recently released products.
    static void AddDefaultTargetGpus(std::shared_ptr<RgBuildSettings> build_settings, const std::string& api_mode)
    {
        // Use the version info's list of supported GPUs to determine the most recent
        // hardware to build for by default.
        RgConfigManager&                  config_manager = RgConfigManager::Instance();
        std::shared_ptr<RgCliVersionInfo> version_info   = config_manager.GetVersionInfo();

        assert(version_info != nullptr);
        if (version_info != nullptr)
        {
            const char* kTokenRdna3 = "RDNA3";
            const char* kTokenRdna2 = "RDNA2";
            const char* kTokenRdna  = "RDNA";
            const char* kTokenVega  = "Vega";

            // Find the set of GPU architectures supported in the current mode.
            auto mode_architectures_iter = version_info->gpu_architectures.find(api_mode);
            if (mode_architectures_iter != version_info->gpu_architectures.end())
            {
                // Search for the latest target. Start with RDNA3, then RDNA2, then RDNA and Vega. As a fall back
                // just take the first element.
                const std::vector<RgGpuArchitecture>& mode_architectures = mode_architectures_iter->second;
                auto last_architecture_iter = std::find_if(mode_architectures.begin(), mode_architectures.end(), [&](const RgGpuArchitecture& arch) {
                    return arch.architecture_name.compare(kTokenRdna3) == 0;
                });

                if (last_architecture_iter == mode_architectures.end())
                {
                    last_architecture_iter = std::find_if(mode_architectures.begin(), mode_architectures.end(), [&](const RgGpuArchitecture& arch) {
                        return arch.architecture_name.compare(kTokenRdna2) == 0;
                    });

                    if (last_architecture_iter == mode_architectures.end())
                    {
                        // Search for the latest target. Start with RDNA, if not look for Vega. As a fall back
                        // just take the first element.
                        const std::vector<RgGpuArchitecture>& mode_architectures = mode_architectures_iter->second;
                        last_architecture_iter = std::find_if(mode_architectures.begin(), mode_architectures.end(), [&](const RgGpuArchitecture& arch) {
                            return arch.architecture_name.compare(kTokenRdna) == 0;
                        });

                        if (last_architecture_iter == mode_architectures.end())
                        {
                            last_architecture_iter = std::find_if(mode_architectures.begin(), mode_architectures.end(), [&](const RgGpuArchitecture& arch) {
                                return arch.architecture_name.compare(kTokenVega) == 0;
                            });

                            if (last_architecture_iter == mode_architectures.end())
                            {
                                // Take the first element.
                                last_architecture_iter = mode_architectures.begin();

                                // We shouldn't be getting here normally.
                                assert(false);
                            }
                        }
                    }
                }

                // Find the last family within the most recent architecture.
                std::vector<RgGpuFamily> architecture_families = last_architecture_iter->gpu_families;
                std::sort(architecture_families.begin(), architecture_families.end(), [&](const RgGpuFamily& family1, const RgGpuFamily& family2) {
                    return family1.family_name < family2.family_name;
                });

                const auto&        latest_family      = architecture_families.rbegin();
                const std::string& latest_family_name = latest_family->family_name;

                // Add the latest supported GPU family as the default target GPU.
                build_settings->target_gpus.push_back(latest_family_name);
            }
        }
    }

    // Creates the default OpenCL build settings.
    static std::shared_ptr<RgBuildSettingsOpencl> CreateDefaultBuildSettingsOpenCL()
    {
        std::shared_ptr<RgBuildSettingsOpencl> ret = std::make_shared<RgBuildSettingsOpencl>();

        assert(ret != nullptr);
        if (ret != nullptr)
        {
            AddDefaultTargetGpus(ret, kStrModeStringOpencl);
        }

        return ret;
    }

    // Creates the default Vulkan build settings.
    static std::shared_ptr<RgBuildSettingsVulkan> CreateDefaultBuildSettingsVulkan()
    {
        std::shared_ptr<RgBuildSettingsVulkan> ret = std::make_shared<RgBuildSettingsVulkan>();

        assert(ret != nullptr);
        if (ret != nullptr)
        {
            AddDefaultTargetGpus(ret, kStrModeStringVulkan);

            // Set the default output binary file name.
            ret->binary_file_name = kStrBuildSettingsOutputBinaryFileName;
        }

        return ret;
    }
};

// Generates the name of the config file.
static std::string GenConfigFileName()
{
    std::string config_file_name = kStrGlobalConfigFileNamePrefix;
    std::string versionSuffix    = std::string("_") + std::to_string(RGA_VERSION_MAJOR) + "_" + std::to_string(RGA_VERSION_MINOR);
    if (RGA_VERSION_UPDATE != 0)
    {
        versionSuffix += "_" + std::to_string(RGA_VERSION_UPDATE);
    }
    config_file_name += versionSuffix;
    config_file_name += ".";
    config_file_name += kStrGlobalConfigFileExtension;

    return config_file_name;
}

bool RgSourceFilePathSearcher::operator()(const RgSourceFileInfo& file_info) const
{
    return file_info.file_path.compare(target_file_path) == 0;
}

bool RgConfigManager::Init()
{
    if (!is_initialized_)
    {
        // Initialize the API used by the RgConfigManager.
        current_api_ = kInitialApiType;

        // Get the path to the default configuration file.
        std::string app_data_dir, config_file_full_path;
        RgConfigManager::GetDefaultDataFolder(app_data_dir);

        // If the output directory does not exist - create it.
        if (!RgUtils::IsDirExists(app_data_dir))
        {
            bool is_dir_created = RgUtils::CreateFolder(app_data_dir);
            assert(is_dir_created);
        }

        // Attempt to load the version info file cached in the RGA data folder.
        std::string version_info_file_path;
        RgConfigManager::GetDefaultDataFolder(version_info_file_path);
        RgUtils::AppendFileNameToPath(version_info_file_path, kStrVersionInfoFileName, version_info_file_path);

        // Ask the CLI to generate the version info file.
        bool is_successful = RgCliLauncher::GenerateVersionInfoFile(version_info_file_path);
        assert(is_successful);

        // Read the version info file.
        version_info_                  = std::make_shared<RgCliVersionInfo>();
        bool is_version_info_file_read = RgXMLCliVersionInfo::ReadVersionInfo(version_info_file_path, version_info_);
        assert(is_version_info_file_read);
        if (is_version_info_file_read)
        {
            // Append the global configuration file name to the path.
            std::string config_file_name = GenConfigFileName();
            assert(!config_file_name.empty());
            if (!config_file_name.empty())
            {
                RgUtils::AppendFileNameToPath(app_data_dir, config_file_name, config_file_full_path);
            }

            // Check if the global configuration file exists. If not - create it.
            bool is_config_file_exists = RgUtils::IsFileExists(config_file_full_path);
            if (is_config_file_exists)
            {
                // Read the global configuration file and validate the operation's success.
                is_initialized_ = RgXmlConfigFile::ReadGlobalSettings(config_file_full_path, global_settings_);

                if (!is_initialized_)
                {
                    // Notify the user in case that we failed to read the config file.
                    std::stringstream msg;
                    msg << kStrErrFatalPrefix << kStrErrCannotReadConfigFile << config_file_full_path << std::endl
                        << kStrErrDeleteFileAndRerun << std::endl
                        << kStrErrRgaWillNowExit;
                    fatal_error_msg_ = msg.str();
                }

                is_initialized_ = is_initialized_ && (global_settings_ != nullptr);
            }

            // Write out a default configuration file when one either doesn't exist, or the read fails.
            if (!is_initialized_ && fatal_error_msg_.empty())
            {
                // Create the global configuration file.
                global_settings_ = std::make_shared<RgGlobalSettings>();

                // Initialize default values in the new global settings instance.
                assert(global_settings_ != nullptr);
                if (global_settings_ != nullptr)
                {
                    ResetToFactoryDefaults(*global_settings_);
                }

                // Loop through each supported API and create + store the default build settings for that API.
                for (int api_index = static_cast<int>(RgProjectAPI::kOpenCL); api_index < static_cast<int>(RgProjectAPI::kApiCount); ++api_index)
                {
                    RgProjectAPI current_api = static_cast<RgProjectAPI>(api_index);

                    std::string api_string;
                    bool        is_ok = RgUtils::ProjectAPIToString(current_api, api_string);
                    assert(is_ok);
                    if (is_ok)
                    {
                        std::shared_ptr<RgBuildSettings> default_build_settings = GetDefaultBuildSettings(current_api);
                        global_settings_->default_build_settings[api_string]    = default_build_settings;
                    }
                }

                // Write out the new global settings file.
                is_initialized_ = RgXmlConfigFile::WriteGlobalSettings(global_settings_, config_file_full_path);
            }
        }
        else
        {
            std::stringstream msg;
            msg << kStrErrFatalPrefix << kStrErrCannotLoadVersionInfoFile << version_info_file_path << std::endl
                << kStrErrDeleteFileAndRerun << std::endl
                << kStrErrRgaWillNowExit;
            fatal_error_msg_ = msg.str();
            RgLog::file << kStrLogFailedLoadVersionInfoFile << version_info_file_path << std::endl;
        }

        if (is_initialized_)
        {
            // Initialize GUI log file.
            std::string log_file_dir  = (global_settings_->log_file_location.empty() ? app_data_dir : global_settings_->log_file_location);
            bool        is_successful = RgaSharedUtils::InitLogFile(log_file_dir, kGuiLogFilePrefix, kDefaultOldLogFilesDays);
            assert(is_successful);
            if (is_successful)
            {
                RgLog::file << kStrLogRgaGuiStarted << std::endl;
            }

            // Construct name for the CLI log file.
            std::string cli_log_name = RgaSharedUtils::ConstructLogFileName(kCliLogFilePrefix);
            assert(!cli_log_name.empty());
            if (!cli_log_name.empty() && RgUtils::AppendFileNameToPath(app_data_dir, cli_log_name, cli_log_name))
            {
                cli_log_file_path_ = cli_log_name;
            }

            if (!is_successful || cli_log_file_path_.empty())
            {
                RgUtils::ShowErrorMessageBox(kStrErrCannotOpenLogFile);
            }

            // Check to see if the user wants to use a custom projects folder.
            std::string project_sub_dir, custom_projects_folder;
            GetDefaultProjectsFolder(project_sub_dir);
            GetCustomProjectsLocation(custom_projects_folder);
            if (!custom_projects_folder.empty())
            {
                project_sub_dir = custom_projects_folder;
            }

            // If necessary - create the Projects sub-directory.
            if (!RgUtils::IsDirExists(project_sub_dir))
            {
                bool is_dir_created = RgUtils::CreateFolder(project_sub_dir);
                assert(is_dir_created);
            }
        }
    }

    return is_initialized_;
}

void RgConfigManager::ResetToFactoryDefaults(RgGlobalSettings& global_settings)
{
    // Initialize the default log file location.
    RgConfigManager::GetDefaultDataFolder(global_settings.log_file_location);

    // Initialize the default project file location.
    RgConfigManager::GetDefaultDocumentsFolder(global_settings.project_file_location);

    // Initialize the visible columns in the ISA disassembly table.
    // Only the Address, Opcode, Operands and Live VGPR columns are visible by default.
    global_settings.visible_disassembly_view_columns = {
        true,   // RgIsaDisassemblyTableColumns::kAddress
        true,   // RgIsaDisassemblyTableColumns::kOpcode
        true,   // RgIsaDisassemblyTableColumns::kOperands
        true,   // RgIsaDisassemblyTableColumns::kLiveVgprs
        false,  // RgIsaDisassemblyTableColumns::kFunctionalUnit
        false,  // RgIsaDisassemblyTableColumns::kCycles
        false,  // RgIsaDisassemblyTableColumns::kBinaryEncoding
    };

    // Default to always asking the user for a name when creating a new project.
    global_settings.use_default_project_name = false;

    // Default to always asking the user at startup which API to work in.
    global_settings.should_prompt_for_api = true;

    // This is an odd option, because Unknown isn't actually an option we support in the GUI
    // but it is theoretically okay, because by default the user is supposed to select it.
    global_settings.default_api = RgProjectAPI::kUnknown;

    // Default font family.
    global_settings.font_family = kStrBuildViewFontFamily;

    // Default font size.
    global_settings.font_size = 10;

    // Default app to open include files.
    global_settings.include_files_viewer = kStrGlobalSettingsSrcViewIncludeViewerDefault;

    global_settings.input_file_ext_glsl    = kStrGlobalSettingsFileExtGlsl;
    global_settings.input_file_ext_hlsl    = kStrGlobalSettingsFileExtHlsl;
    global_settings.input_file_ext_spv_txt = kStrGlobalSettingsFileExtSpvasm;
    global_settings.input_file_ext_spv_bin = kStrGlobalSettingsFileExtSpv;

    global_settings.default_lang = RgSrcLanguage::kGLSL;
}

RgConfigManager& RgConfigManager::Instance()
{
    static RgConfigManager instance;
    return instance;
}

void RgConfigManager::AddSourceFileToProject(const std::string& source_file_path, std::shared_ptr<RgProject> project, int clone_index) const
{
    assert(project != nullptr);
    if (project != nullptr)
    {
        // Ensure that the incoming clone index is valid for the current project.
        bool is_valid_range = (clone_index >= 0 && clone_index < project->clones.size());
        assert(is_valid_range);

        if (is_valid_range)
        {
            bool is_path_empty = source_file_path.empty();
            assert(!is_path_empty);
            if (!is_path_empty)
            {
                std::shared_ptr<RgProjectClone> clone = project->clones[clone_index];

                // Add a new source file info instance to the list of source files in the clone.
                RgSourceFileInfo new_source_file = {};
                new_source_file.file_path        = source_file_path;
                clone->source_files.push_back(new_source_file);
            }
        }
    }
}

void RgConfigManager::AddShaderStage(RgPipelineStage stage, const std::string& source_file_path, std::shared_ptr<RgProject> project, int clone_index) const
{
    assert(project != nullptr);
    if (project != nullptr)
    {
        // Ensure that the incoming clone index is valid for the current project.
        bool is_valid_range = (clone_index >= 0 && clone_index < project->clones.size());
        assert(is_valid_range);

        if (is_valid_range)
        {
            bool is_path_empty = source_file_path.empty();
            assert(!is_path_empty);
            if (!is_path_empty)
            {
                std::shared_ptr<RgProjectClone> clone = project->clones[clone_index];

                std::shared_ptr<RgGraphicsProjectClone> graphics_clone = std::dynamic_pointer_cast<RgGraphicsProjectClone>(clone);

                // Add a new source file info instance to the list of source files in the clone.
                size_t stage_index                                  = static_cast<size_t>(stage);
                graphics_clone->pipeline.shader_stages[stage_index] = source_file_path;
            }
        }
    }
}

bool RgConfigManager::GetShaderStageFilePath(RgPipelineStage stage, std::shared_ptr<RgGraphicsProjectClone> graphics_clone, std::string& full_file_path) const
{
    // Find the input file path for given pipeline stage shader.
    size_t             stage_index     = static_cast<size_t>(stage);
    const std::string& input_file_path = graphics_clone->pipeline.shader_stages[stage_index];

    bool stage_in_use = !input_file_path.empty();
    if (stage_in_use)
    {
        full_file_path = input_file_path;
    }

    return stage_in_use;
}

void RgConfigManager::RemoveShaderStage(RgPipelineStage stage, std::shared_ptr<RgGraphicsProjectClone> graphics_clone) const
{
    assert(graphics_clone != nullptr);
    if (graphics_clone != nullptr)
    {
        // Clear out the path to the input shader file for the given stage.
        size_t stage_index = static_cast<size_t>(stage);
        graphics_clone->pipeline.shader_stages[stage_index].clear();
    }
}

RgProjectAPI RgConfigManager::GetCurrentAPI() const
{
    return current_api_;
}

void RgConfigManager::SetCurrentAPI(RgProjectAPI project_api)
{
    current_api_ = project_api;
}

void RgConfigManager::SetDefaultAPI(RgProjectAPI default_api)
{
    global_settings_->default_api = default_api;
}

void RgConfigManager::SetPromptForAPIAtStartup(bool should_prompt)
{
    global_settings_->should_prompt_for_api = should_prompt;
}

std::string RgConfigManager::GetCurrentModeString() const
{
    std::string mode_string = "Invalid";

    switch (current_api_)
    {
    case RgProjectAPI::kOpenCL:
        mode_string = kStrModeStringOpencl;
        break;
    case RgProjectAPI::kVulkan:
        mode_string = kStrModeStringVulkan;
        break;
    default:
        assert(false);
        break;
    }

    return mode_string;
}

// A predicate used to find a given GPU family name within an ASIC architecture.
struct GpuFamilyNameFinder
{
    GpuFamilyNameFinder(const std::string& target_name)
        : target_name(target_name)
    {
    }

    // A predicate that will compare a GPU family's name with a target name.
    bool operator()(const RgGpuFamily& gpu_family) const
    {
        return gpu_family.family_name.compare(target_name) == 0;
    }

    // The target family name to search for.
    std::string target_name;
};

bool RgConfigManager::IsGpuFamilySupported(const std::string& family_name) const
{
    bool is_supported = false;

    assert(version_info_ != nullptr);
    if (version_info_ != nullptr)
    {
        // Get the list of supported architectures for the current mode.
        std::string current_mode          = GetCurrentModeString();
        auto        gpu_architecture_iter = version_info_->gpu_architectures.find(current_mode);
        if (gpu_architecture_iter != version_info_->gpu_architectures.end())
        {
            // Step through each architecture's family list to try to find the given name.
            std::vector<RgGpuArchitecture> supported_architectures = gpu_architecture_iter->second;

            for (const RgGpuArchitecture& current_architecture : supported_architectures)
            {
                const std::vector<RgGpuFamily>& families = current_architecture.gpu_families;

                // Search the list of GPU families for the incoming name.
                GpuFamilyNameFinder family_name_searcher(family_name);
                auto                family_search_iter = std::find_if(families.begin(), families.end(), family_name_searcher);
                if (family_search_iter != families.end())
                {
                    is_supported = true;
                    break;
                }
            }
        }
    }

    return is_supported;
}

void RgConfigManager::RemoveSourceFilePath(std::shared_ptr<RgProject> project, int clone_index, const std::string& source_file_path) const
{
    assert(project != nullptr);
    if (project != nullptr)
    {
        // Ensure that the incoming clone index is valid for the current project.
        bool is_valid_range = (clone_index >= 0 && clone_index < project->clones.size());
        assert(is_valid_range);

        if (is_valid_range)
        {
            // Empty the list of source files for the selected clone.
            std::shared_ptr<RgProjectClone> clone = project->clones[clone_index];

            bool is_path_empty = source_file_path.empty();
            assert(!is_path_empty);
            if (!is_path_empty)
            {
                // Search the list of source files in the clone to obtain an iterator to it.
                RgSourceFilePathSearcher source_file_searcher(source_file_path);
                auto                     sourceFileIter = std::find_if(clone->source_files.begin(), clone->source_files.end(), source_file_searcher);
                bool                     isFileInList   = (sourceFileIter != clone->source_files.end());
                assert(isFileInList);

                // If the path was found in the clone's source files, erase it.
                if (isFileInList)
                {
                    clone->source_files.erase(sourceFileIter);
                }
            }
        }
    }
}

void RgConfigManager::GetProjectSourceFilePaths(std::shared_ptr<RgProject> project, int clone_index, std::vector<std::string>& sourceFilePaths) const
{
    assert(project != nullptr);
    if (project != nullptr)
    {
        // Ensure that the incoming clone index is valid for the current project.
        bool is_valid_range = (clone_index >= 0 && clone_index < project->clones.size());
        assert(is_valid_range);

        if (is_valid_range)
        {
            std::shared_ptr<RgProjectClone> clone = project->clones[clone_index];
            for (const RgSourceFileInfo& file_info : clone->source_files)
            {
                sourceFilePaths.push_back(file_info.file_path);
            }
        }
    }
}

void RgConfigManager::UpdateSourceFilepath(const std::string&         old_file_path,
                                           const std::string&         new_file_path,
                                           std::shared_ptr<RgProject> project,
                                           int                        clone_index)
{
    // Ensure that the incoming clone index is valid for the current project.
    bool is_valid_range = (clone_index >= 0 && clone_index < project->clones.size());
    assert(is_valid_range);
    if (is_valid_range)
    {
        // Use the clone given by the incoming index, and make sure it's valid.
        std::shared_ptr<RgProjectClone> clone = project->clones[clone_index];
        assert(clone);

        // Attempt to find the old file path referenced within the clone.
        RgSourceFilePathSearcher source_file_searcher(old_file_path);
        auto                     file_iter = std::find_if(clone->source_files.begin(), clone->source_files.end(), source_file_searcher);

        // Verify that the old file path is referenced within the clone.
        bool old_file_path_found = file_iter != clone->source_files.end();
        assert(old_file_path_found);

        // If the old file path exists in the clone, remove it.
        if (old_file_path_found)
        {
            clone->source_files.erase(file_iter);
        }

        // Add the updated file path to the list of source files for the clone.
        RgSourceFileInfo updated_file_path = {};
        updated_file_path.file_path        = new_file_path;
        clone->source_files.push_back(updated_file_path);
    }
}

void RgConfigManager::UpdateShaderStageFilePath(const std::string&               old_file_path,
                                                const std::string&               new_file_path,
                                                std::shared_ptr<RgProjectVulkan> project,
                                                int                              clone_index)
{
    // Ensure that the incoming clone index is valid for the current project.
    bool is_valid_range = (clone_index >= 0 && clone_index < project->clones.size());
    assert(is_valid_range);
    if (is_valid_range)
    {
        // Use the clone given by the incoming index, and make sure it's valid.
        std::shared_ptr<RgProjectCloneVulkan> clone = std::dynamic_pointer_cast<RgProjectCloneVulkan>(project->clones[clone_index]);
        assert(clone != nullptr);
        if (clone != nullptr)
        {
            ShaderInputFileArray& stage_array = clone->pipeline.shader_stages;
            for (int stage_index = 0; stage_index < stage_array.size(); ++stage_index)
            {
                const std::string& stage_file_path = clone->pipeline.shader_stages[stage_index];
                if (stage_file_path.compare(old_file_path) == 0)
                {
                    // Update the file path for the shader stage.
                    clone->pipeline.shader_stages[stage_index] = new_file_path;
                }
            }
        }
    }
}

void RgConfigManager::GetProjectFolder(const std::string& project_name, std::string& project_folder)
{
    // Start with the default directory where project files live.
    std::string projects_folder, custom_folder;
    RgConfigManager::Instance().GetDefaultProjectsFolder(projects_folder);

    // Get the config manager.
    RgConfigManager& config_manager = RgConfigManager::Instance();

    // Update the recent project list to reference the new path.
    config_manager.GetCustomProjectFolder(custom_folder);

    // Check to see if the user wants to use a custom folder.
    if (!custom_folder.empty())
    {
        projects_folder = custom_folder;
    }

    // Append the project name as a subdirectory.
    RgUtils::AppendFolderToPath(projects_folder, project_name, project_folder);
}

void RgConfigManager::GenerateNewSourceFilepath(const std::string& project_name,
                                                int                clone_index,
                                                const std::string& source_filename,
                                                const std::string& sourcefile_extension,
                                                std::string&       generated_file_name,
                                                std::string&       full_source_file_path)
{
    // Get the path to the folder for the given project.
    std::string project_folder;
    GetProjectFolder(project_name, project_folder);

    // Generate a folder name for where the new clone source file will be stored.
    std::string clone_folder_name = RgUtils::GenerateCloneName(clone_index);

    // Append the clone folder to the project folder.
    std::string clone_folder_path;
    RgUtils::AppendFolderToPath(project_folder, clone_folder_name, clone_folder_path);

    // Append a suffix number onto the end of a filename to make it unique.
    unsigned int suffix = 0;

    // Loop to generate a new filename with suffix until it's a unique item in the file menu.
    do
    {
        std::stringstream filename_stream;
        filename_stream << source_filename;
        if (suffix > 0)
        {
            // Only insert the suffix if it is non-zero, so that generated filenames
            // follow the pattern: src, src1, src2, src3, etc.
            filename_stream << suffix;
        }
        generated_file_name = filename_stream.str();
        filename_stream << sourcefile_extension;
        suffix++;

        // Store the path in the final destination, it will get tested in the while statement.
        RgUtils::AppendFileNameToPath(clone_folder_path, filename_stream.str(), full_source_file_path);

    } while (RgUtils::IsFileExists(full_source_file_path));

    // Make sure all path separators are the same.
    RgUtils::StandardizePathSeparator(full_source_file_path);
}

void RgConfigManager::GenerateNewPipelineFilepath(const std::string& project_name,
                                                  int                clone_index,
                                                  const std::string& pipeline_filename,
                                                  const std::string& pipeline_file_extension,
                                                  std::string&       full_pipeline_file_path)
{
    // Get the path to the folder for the given project.
    std::string project_folder;
    GetProjectFolder(project_name, project_folder);

    // Generate the project's filename.
    std::stringstream pipeline_file_stream;
    pipeline_file_stream << pipeline_filename;
    pipeline_file_stream << pipeline_file_extension;

    // Generate a folder name for where the new clone source file will be stored.
    std::string clone_folder_name = RgUtils::GenerateCloneName(clone_index);

    // Append the clone folder to the project folder.
    std::string clone_folder_path;
    RgUtils::AppendFolderToPath(project_folder, clone_folder_name, clone_folder_path);

    // Generate the full path to the project file.
    RgUtils::AppendFileNameToPath(clone_folder_path, pipeline_file_stream.str(), full_pipeline_file_path);

    // Make sure all path separators are the same.
    RgUtils::StandardizePathSeparator(full_pipeline_file_path);
}

std::shared_ptr<RgProject> RgConfigManager::LoadProjectFile(const std::string& project_file_path)
{
    std::shared_ptr<RgProject> project           = nullptr;
    bool                       is_project_loaded = RgXmlConfigFile::ReadProjectConfigFile(project_file_path, project);

    bool is_ok = is_project_loaded && project != nullptr;
    assert(is_ok);

    if (is_ok)
    {
        auto recent_project          = std::make_shared<RgRecentProject>();
        recent_project->project_path = project_file_path;
        recent_project->api_type     = project->api;

        // Add this path to the list of recently loaded projects.
        AddRecentProjectPath(recent_project);
    }

    return project;
}

bool RgConfigManager::SaveProjectFile(std::shared_ptr<RgProject> project)
{
    bool ret = false;
    if (project != nullptr)
    {
        // When saving a project, update the project's path in the list of recent projects.
        const std::string project_file_full_path = project->project_file_full_path;
        assert(!project_file_full_path.empty());

        if (global_settings_ != nullptr)
        {
            // Add this project if it does not exist.
            ProjectPathSearcher searcher(project_file_full_path);
            auto                project_path_iter = std::find_if(global_settings_->recent_projects.begin(), global_settings_->recent_projects.end(), searcher);
            if (project_path_iter == global_settings_->recent_projects.end())
            {
                auto recent_project          = std::make_shared<RgRecentProject>();
                recent_project->project_path = project_file_full_path;
                recent_project->api_type     = project->api;

                AddRecentProjectPath(recent_project);
            }
        }

        std::string project_directory;
        RgUtils::ExtractFileDirectory(project_file_full_path, project_directory);

        if (!RgUtils::IsDirExists(project_directory))
        {
            bool is_dir_created = RgUtils::CreateFolder(project_directory);
            assert(is_dir_created);
        }

        // Save the file.
        RgXmlConfigFile::WriteProjectConfigFile(*project, project_file_full_path);
        ret = true;
    }
    return ret;
}

void RgConfigManager::RevertToDefaultBuildSettings(RgProjectAPI api)
{
    std::string api_string;
    bool        is_ok = RgUtils::ProjectAPIToString(api, api_string);

    assert(is_ok);
    if (is_ok)
    {
        bool api_string_is_not_empty = !api_string.empty();
        assert(api_string_is_not_empty);

        if (api_string_is_not_empty)
        {
            // Find the settings for the API being reset.
            auto default_settings_iter = global_settings_->default_build_settings.find(api_string);
            if (default_settings_iter != global_settings_->default_build_settings.end())
            {
                std::shared_ptr<RgBuildSettings> default_build_settings = RgConfigManager::GetDefaultBuildSettings(api);
                assert(default_build_settings);

                // Assign the API's default settings as the global settings.
                if (default_build_settings != nullptr)
                {
                    global_settings_->default_build_settings[api_string] = default_build_settings;
                }
            }
        }
    }
}

bool RgConfigManager::SaveGlobalConfigFile() const
{
    bool ret = false;

    // Get the path to the default configuration file.
    std::string config_file_full_path;
    RgConfigManager::GetDefaultDataFolder(config_file_full_path);

    // Append the global configuration file name to the path.
    std::string config_file_name = GenConfigFileName();
    assert(!config_file_name.empty());
    if (!config_file_name.empty())
    {
        if (RgUtils::AppendFileNameToPath(config_file_full_path, config_file_name, config_file_full_path))
        {
            // Write out the settings.
            ret = RgXmlConfigFile::WriteGlobalSettings(global_settings_, config_file_full_path);
        }
    }

    return ret;
}

void RgConfigManager::AddRecentProjectPath(std::shared_ptr<RgRecentProject> recent_project)
{
    const RgConfigManager&            config_manager  = RgConfigManager::Instance();
    std::shared_ptr<RgGlobalSettings> global_settings = config_manager.GetGlobalConfig();

    // Create a copy of the filepath, because it may be modified below.
    assert(recent_project != nullptr);
    if (recent_project != nullptr)
    {
        const std::string path_to_add = recent_project->project_path;

        // Strips a string from path separator characters ("\, /").
        auto strip_string = [](std::string& str) {
            str.erase(std::remove(str.begin(), str.end(), '\\'), str.end());
            str.erase(std::remove(str.begin(), str.end(), '/'), str.end());
        };

        // Strip the path from path separator characters.
        std::string stripped_path = path_to_add;
        strip_string(stripped_path);

        // Is the project already in the list of recent projects?
        auto path_iter = std::find_if(
            global_settings->recent_projects.begin(), global_settings->recent_projects.end(), [&](std::shared_ptr<RgRecentProject> recent_project) {
                // Strip the current string and compare.
                bool        status = false;
                std::string str    = "";
                assert(recent_project != nullptr);
                if (recent_project != nullptr)
                {
                    str = recent_project->project_path;
                    strip_string(str);
                    status = (0 == str.compare(stripped_path));
                }
                return status;
            });

        // If the project already exists, remove it.
        if (path_iter != global_settings->recent_projects.end())
        {
            global_settings->recent_projects.erase(path_iter);
        }

        // Add the project to the top of the list.
        global_settings->recent_projects.push_back(recent_project);

        // If there are now more than the maximum number of recent projects, remove the oldest entry.
        if (global_settings->recent_projects.size() > kMaxRecentFiles)
        {
            auto oldest_recent_path = (*global_settings->recent_projects.begin())->project_path;

            // Remove the entry with the above file path.
            ProjectPathSearcher searcher(oldest_recent_path);
            auto                projectPathIter = std::find_if(global_settings->recent_projects.begin(), global_settings->recent_projects.end(), searcher);
            if (projectPathIter != global_settings->recent_projects.end())
            {
                global_settings->recent_projects.erase(projectPathIter);
            }
        }

        // Save the configuration file after adding the new entry.
        config_manager.SaveGlobalConfigFile();
    }
}

void RgConfigManager::UpdateRecentProjectPath(const std::string& old_file_path, const std::string& new_file_path, RgProjectAPI api)
{
    const RgConfigManager&            config_manager  = RgConfigManager::Instance();
    std::shared_ptr<RgGlobalSettings> global_settings = config_manager.GetGlobalConfig();

    // Is the project already in the list of recent projects?
    ProjectPathSearcher searcher(old_file_path);
    auto                project_path_iter = std::find_if(global_settings->recent_projects.begin(), global_settings->recent_projects.end(), searcher);
    if (project_path_iter != global_settings->recent_projects.end())
    {
        // Compute the vector index of the path being updated.
        int path_index = (project_path_iter - global_settings->recent_projects.begin());

        // Verify that the index is valid.
        bool is_valid_index = (path_index >= 0 && path_index < global_settings->recent_projects.size());
        assert(is_valid_index);
        if (is_valid_index)
        {
            // If the path was already in the list, remove it and re-add it to the end of the list. It's now the most recent one.
            assert(global_settings->recent_projects[path_index]);
            if (global_settings->recent_projects[path_index] != nullptr)
            {
                global_settings->recent_projects[path_index]->project_path = new_file_path;
            }
        }
    }

    // Add the new path, which will bump it to the top, and save the config file.
    auto recent_project          = std::make_shared<RgRecentProject>();
    recent_project->project_path = new_file_path;
    recent_project->api_type     = api;

    AddRecentProjectPath(recent_project);
}

// Reset the current API Build settings with those supplied.
void RgConfigManager::SetApiBuildSettings(const std::string& api_name, RgBuildSettings* build_settings)
{
    assert(global_settings_ != nullptr);
    if (global_settings_ != nullptr)
    {
        if (api_name.compare(kStrApiNameOpencl) == 0)
        {
            std::shared_ptr<RgBuildSettingsOpencl> api_build_settings =
                std::dynamic_pointer_cast<RgBuildSettingsOpencl>(global_settings_->default_build_settings[api_name]);
            assert(api_build_settings != nullptr);
            if (api_build_settings != nullptr)
            {
                *api_build_settings = *dynamic_cast<RgBuildSettingsOpencl*>(build_settings);
            }
        }
        else if (api_name.compare(kStrApiNameVulkan) == 0)
        {
            std::shared_ptr<RgBuildSettingsVulkan> api_build_settings =
                std::dynamic_pointer_cast<RgBuildSettingsVulkan>(global_settings_->default_build_settings[api_name]);
            assert(api_build_settings != nullptr);
            if (api_build_settings != nullptr)
            {
                *api_build_settings = *dynamic_cast<RgBuildSettingsVulkan*>(build_settings);
            }
        }
        else
        {
            assert(!"Unsupported api_name in SetApiBuildSettings");
        }
    }
}

std::shared_ptr<RgGlobalSettings> RgConfigManager::GetGlobalConfig() const
{
    return global_settings_;
}

std::shared_ptr<RgCliVersionInfo> RgConfigManager::GetVersionInfo() const
{
    return version_info_;
}

void RgConfigManager::Close() const
{
    SaveGlobalConfigFile();
    RgLog::file << kStrLogClosingRgaGui << std::endl;
    RgaSharedUtils::CloseLogFile();
}

std::shared_ptr<RgBuildSettings> RgConfigManager::GetDefaultBuildSettings(RgProjectAPI api)
{
    return DefaultConfigFactory::GetDefaultBuildSettings(api);
}

void RgConfigManager::GetDefaultDataFolder(std::string& default_data_folder)
{
    // Build the output path only once, then cache the result for subsequent calls.
    static std::string output_path;
    if (output_path.empty())
    {
        QString systemDefaultDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        RgUtils::AppendFolderToPath(systemDefaultDataPath.toStdString(), kStrAppFolderName, output_path);
    }
    default_data_folder = output_path;
}

void RgConfigManager::GetDefaultDocumentsFolder(std::string& default_documents_folder)
{
    // Build the output path only once, then cache the result for subsequent calls.
    static std::string output_path;
    if (output_path.empty())
    {
        QString systemDefaultDataPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
        RgUtils::AppendFolderToPath(systemDefaultDataPath.toStdString(), kStrAppFolderName, output_path);
    }
    default_documents_folder = output_path;
}

void RgConfigManager::GetDefaultProjectsFolder(std::string& default_projects_folder)
{
    RgConfigManager::GetDefaultDocumentsFolder(default_projects_folder);
    RgUtils::AppendFolderToPath(default_projects_folder, kStrProjectsFolderName, default_projects_folder);
}

void RgConfigManager::GetCustomProjectsLocation(std::string& custom_project_location)
{
    if (global_settings_ != nullptr)
    {
        std::string temp_folder = global_settings_->project_file_location;

        // Append the "projects" subdirectory.
        RgUtils::AppendFolderToPath(temp_folder, kStrProjectsFolderName, custom_project_location);
    }
}

void RgConfigManager::GetCustomProjectFolder(std::string& custom_project_folder)
{
    if (global_settings_ != nullptr)
    {
        std::string temp_folder = global_settings_->project_file_location;

        // Append the "projects" subdirectory.
        RgUtils::AppendFolderToPath(temp_folder, kStrProjectsFolderName, custom_project_folder);
    }
}

std::shared_ptr<RgBuildSettings> RgConfigManager::GetUserGlobalBuildSettings(RgProjectAPI api) const
{
    std::shared_ptr<RgBuildSettings> ret = nullptr;
    if (global_settings_ != nullptr)
    {
        // Translate the project API to string.
        std::string api_name;
        bool        is_ok = RgUtils::ProjectAPIToString(api, api_name);
        if (is_ok && !api_name.empty())
        {
            // Get the relevant build settings.
            auto iter = global_settings_->default_build_settings.find(api_name);
            if (iter != global_settings_->default_build_settings.cend())
            {
                ret = iter->second;
            }
        }
    }

    assert(ret != nullptr);
    return ret;
}

std::string RgConfigManager::GetLastSelectedFolder() const
{
    static const char* kDefaultFolder = "./";
    std::string        ret            = kDefaultFolder;

    // Get global configuration file.
    std::shared_ptr<RgGlobalSettings> global_config = RgConfigManager::Instance().GetGlobalConfig();

    // Only return if config file and string are valid, otherwise use default folder.
    if (global_config != nullptr && global_config->last_selected_directory != "")
    {
        ret = global_config->last_selected_directory;
    }

    return ret;
}

std::vector<std::shared_ptr<RgRecentProject>> RgConfigManager::GetRecentProjects() const
{
    assert(global_settings_ != nullptr);
    bool is_ok = (global_settings_ != nullptr);
    return (is_ok ? global_settings_->recent_projects : std::vector<std::shared_ptr<RgRecentProject>>());
}

void RgConfigManager::SetGlobalConfig(const RgGlobalSettings& global_settings)
{
    assert(global_settings_ != nullptr);

    // Never allow reseting the pointer to the global config, there should be only one such pointer.
    // This function only makes sense when the object is initialized. In future, we need to make sure that this
    // function accepts a value, and not a pointer.
    if (global_settings_ != nullptr)
    {
        // Replace the global settings with the incoming instance.
        *global_settings_ = global_settings;
    }
}

void RgConfigManager::SetLastSelectedDirectory(const std::string& last_selected_directory)
{
    assert(global_settings_ != nullptr);
    if (global_settings_ != nullptr)
    {
        global_settings_->last_selected_directory = last_selected_directory;
    }
}

std::string RgConfigManager::GenerateProjectFilepath(const std::string& project_name)
{
    // Get the path to the folder for the given project.
    std::string project_folder;
    GetProjectFolder(project_name, project_folder);

    // Generate the project's filename.
    std::stringstream project_filename;
    project_filename << project_name;

    // Check if the filename already has the .rga extension.
    if (!QString::fromStdString(project_name).endsWith(kStrProjectFileExtension))
    {
        project_filename << kStrProjectFileExtension;
    }

    // Generate the full path to the project file.
    std::string file_project_path;
    RgUtils::AppendFileNameToPath(project_folder, project_filename.str(), file_project_path);

    return file_project_path;
}

void RgConfigManager::SetWindowGeometry(int x_pos, int y_pos, int width, int height, int window_state)
{
    assert(global_settings_ != nullptr);
    if (global_settings_ != nullptr)
    {
        RgWindowConfig window_config = global_settings_->gui_window_config;
        if (x_pos >= 0)
        {
            window_config.window_x_pos = x_pos;
        }
        else
        {
            window_config.window_x_pos = 0;
        }
        if (y_pos >= 0)
        {
            window_config.window_y_pos = y_pos;
        }
        else
        {
            window_config.window_y_pos = 0;
        }
        window_config.window_width  = width;
        window_config.window_height = height;

        // If the window is maximized, save the state.
        window_config.window_state = window_state;

        // Update the window configuration.
        global_settings_->gui_window_config = window_config;
    }
}

void RgConfigManager::GetWindowGeometry(RgWindowConfig& window_values) const
{
    assert(global_settings_ != nullptr);
    if (global_settings_ != nullptr)
    {
        window_values.window_x_pos  = global_settings_->gui_window_config.window_x_pos;
        window_values.window_y_pos  = global_settings_->gui_window_config.window_y_pos;
        window_values.window_height = global_settings_->gui_window_config.window_height;
        window_values.window_width  = global_settings_->gui_window_config.window_width;
        window_values.window_state  = global_settings_->gui_window_config.window_state;
    }
}

void RgConfigManager::SetSplitterValues(const std::string& splitter_name, const std::vector<int>& splitter_values)
{
    assert(global_settings_ != nullptr);
    if (global_settings_ != nullptr)
    {
        bool is_new_item = true;

        // Find name match.
        for (RgSplitterConfig& splitter_config : global_settings_->gui_layout_splitters)
        {
            // Name matches.
            if (splitter_config.splitter_name == splitter_name)
            {
                splitter_config.splitter_values = splitter_values;
                is_new_item                     = false;
                break;
            }
        }

        // Create a new config item if necessary.
        if (is_new_item)
        {
            // Construct new splitter config item.
            RgSplitterConfig new_splitter_config;
            new_splitter_config.splitter_name   = splitter_name;
            new_splitter_config.splitter_values = splitter_values;

            // Add the new config item.
            global_settings_->gui_layout_splitters.push_back(new_splitter_config);
        }
    }
}

bool RgConfigManager::GetSplitterValues(const std::string& splitter_name, std::vector<int>& splitter_values) const
{
    bool ret = false;
    assert(global_settings_ != nullptr);
    if (global_settings_ != nullptr)
    {
        // Find name match.
        for (RgSplitterConfig& splitterConfig : global_settings_->gui_layout_splitters)
        {
            // Name matches.
            if (splitterConfig.splitter_name == splitter_name)
            {
                splitter_values = splitterConfig.splitter_values;
                ret             = true;
                break;
            }
        }
    }

    return ret;
}

void RgConfigManager::SetDisassemblyColumnVisibility(const std::vector<bool>& column_visibility)
{
    assert(global_settings_ != nullptr);
    if (global_settings_ != nullptr)
    {
        assert(global_settings_->visible_disassembly_view_columns.size() == column_visibility.size());
        global_settings_->visible_disassembly_view_columns = column_visibility;
    }
}

std::string RgConfigManager::GetFatalErrorMessage() const
{
    return fatal_error_msg_;
}

void RgConfigManager::GetSupportedApis(std::vector<std::string>& supported_apis)
{
    for (auto const& build_setting : global_settings_->default_build_settings)
    {
        supported_apis.push_back(build_setting.first);
    }
}

const std::string& RgConfigManager::GetCLILogFilePath()
{
    return cli_log_file_path_;
}

std::string RgConfigManager::GetIncludeFileViewer() const
{
    // By default, return the system default option.
    std::string ret = kStrGlobalSettingsSrcViewIncludeViewerDefault;
    assert(global_settings_ != nullptr);
    if (global_settings_ != nullptr)
    {
        // Get the user's app of choice.
        ret = global_settings_->include_files_viewer;
    }
    return ret;
}

void RgConfigManager::SetConfigFileDataModelVersion(const std::string& data_model_version)
{
    config_file_data_model_version_ = data_model_version;
}

std::string RgConfigManager::GetConfigFileDataModelVersion() const
{
    return config_file_data_model_version_;
}

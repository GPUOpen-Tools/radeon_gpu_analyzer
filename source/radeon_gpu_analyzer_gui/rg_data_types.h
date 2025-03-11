#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_DATA_TYPES_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_DATA_TYPES_H_

// C++.
#include <array>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <algorithm>

// Qt.
#include <QMetaType>

// Infra.
#include "source/common/rga_shared_data_types.h"

// Local.
#include "radeon_gpu_analyzer_gui/rg_config_manager.h"
#include "radeon_gpu_analyzer_gui/rg_string_constants.h"

// The type of the project.
enum class RgProjectAPI : char
{
    // Unknown.
    kUnknown,

    // Binary.
    kBinary,

    // OpenCL.
    kOpenCL,

    // Vulkan.
    kVulkan,

    // The count of known APIs.
    kApiCount
};

// Supported textual source languages.
enum class RgSrcLanguage
{
    kOpenCL,
    kGLSL,
    kHLSL,
    kSPIRV_Text,

    Unknown
};

// Indices for disassembly view sub widgets.
enum class DisassemblyViewSubWidgets
{
    kTableView,
    kTargetGpuPushButton,
    kColumnPushButton,
    kOutputWindow,
    kSourceWindow,
    kNone,
    kCount
};

Q_DECLARE_METATYPE(DisassemblyViewSubWidgets);

// The type of pipeline.
enum class RgPipelineType : char
{
    // A multi-stage graphics pipeline.
    kGraphics,

    // A single-stage compute pipeline.
    kCompute
};

// An array of file paths for each pipeline stage. An empty string indicates an unused stage.
typedef std::array<std::string, RgPipelineStage::kCount> ShaderInputFileArray;

// The base class for pipeline objects.
struct RgPipelineShaders
{
    // The type of pipeline.
    RgPipelineType type;

    // An array of input source file paths per pipeline stage.
    ShaderInputFileArray shader_stages;
};

// The state for an individual pipeline object.
struct RgPipelineState
{
    // The pipeline name.
    std::string name;

    // The full path to the serialized pipeline state file.
    std::string pipeline_state_file_path;

    // The full path to the original pipeline state file loaded to the PSO editor.
    std::string original_pipeline_state_file_path;

    // A flag used to indicate if the pipeline is currently active.
    bool is_active = false;
};

// General RGA settings structure.
struct RgBuildSettings
{
    RgBuildSettings()          = default;
    virtual ~RgBuildSettings() = default;

    // A copy constructor used to initialize using another instance.
    RgBuildSettings(const RgBuildSettings& other)
        : target_gpus(other.target_gpus)
        , predefined_macros(other.predefined_macros)
        , additional_include_directories(other.additional_include_directories)
        , additional_options(other.additional_options)
        , compiler_paths(other.compiler_paths)
        , binary_file_name(other.binary_file_name)
    {
    }

    virtual bool HasSameSettings(const RgBuildSettings& other) const
    {
        bool isSame = (target_gpus == other.target_gpus) && (predefined_macros == other.predefined_macros) &&
                      (additional_include_directories == other.additional_include_directories) && (additional_options == other.additional_options) &&
                      (binary_file_name == other.binary_file_name) && (compiler_paths == other.compiler_paths);

        return isSame;
    }

    // General build settings.
    std::vector<std::string> target_gpus;
    std::vector<std::string> predefined_macros;
    std::vector<std::string> additional_include_directories;
    std::string              additional_options;
    std::string              binary_file_name;

    // Alternative compiler paths: {bin, include, lib}.
    std::tuple<std::string, std::string, std::string> compiler_paths;
};

// An info structure for each source file in a project.
struct RgSourceFileInfo
{
    // The full path to the source file.
    std::string file_path;

    // A flag indicating if the file is correlated with current build output.
    bool is_correlated = false;
};

// A base class for project clones.
struct RgProjectClone
{
    RgProjectClone()          = default;
    virtual ~RgProjectClone() = default;

    RgProjectClone(const std::string& clone_name, std::shared_ptr<RgBuildSettings> build_settings)
        : clone_name(clone_name)
        , build_settings(build_settings)
    {
    }

    // Returns "true" if the clone does not have any source files or "false" otherwise.
    virtual bool IsEmpty() const
    {
        return source_files.empty();
    }

    unsigned                         clone_id = 0;
    std::string                      clone_name;
    std::vector<RgSourceFileInfo>    source_files;
    std::shared_ptr<RgBuildSettings> build_settings = nullptr;
};

// A project clone containing graphics or compute pipeline state info.
struct RgGraphicsProjectClone : RgProjectClone
{
    RgGraphicsProjectClone()          = default;
    virtual ~RgGraphicsProjectClone() = default;

    // CTOR used to initialize with existing build settings.
    RgGraphicsProjectClone(const std::string& clone_name, std::shared_ptr<RgBuildSettings> build_settings)
        : RgProjectClone(clone_name, build_settings)
    {
    }

    // Returns "true" if the clone does not have any source files or "false" otherwise.
    virtual bool IsEmpty() const override
    {
        return std::all_of(pipeline.shader_stages.cbegin(), pipeline.shader_stages.cend(), [&](const std::string& file) { return file.empty(); });
    }

    // A list of pipeline states within the clone.
    std::vector<RgPipelineState> pso_states;

    // The pipeline object, which specifies the pipeline type, and
    // includes full paths to shader files associated with the pipeline's stages.
    RgPipelineShaders pipeline;
};

// This structure represents a family of GPU products.
struct RgGpuFamily
{
    // The GPU family name.
    std::string family_name;

    // An array of GPU product names.
    std::vector<std::string> product_names;
};

// This structure represents a GPU architecture.
struct RgGpuArchitecture
{
    // The name of the GPU HW architecture.
    std::string architecture_name;

    // An array of GPU families within this GPU hardware architecture.
    std::vector<RgGpuFamily> gpu_families;

    // Returns The name of the GPU HW architecture.
    std::string GetArchitectureName() const
    {
        return architecture_name;
    }

    // Comparision operator.
    bool operator<(const RgGpuArchitecture& other) const
    {
        return GetArchitectureName() < other.GetArchitectureName();
    }
};

// Structure containing the CLI version info.
struct RgCliVersionInfo
{
    // The CLI version string.
    std::string version;

    // The build date.
    std::string build_date;

    // A map of build mode to an array of all supported GPU architectures for the mode.
    std::map<std::string, std::vector<RgGpuArchitecture>> gpu_architectures;
};

// Base structure for projects.
struct RgProject
{
    // Default CTOR, DTOR.
    RgProject()          = default;
    virtual ~RgProject() = default;

    // CTOR #1.
    RgProject(const std::string& project_name, const std::string& project_file_full_path, RgProjectAPI api)
        : project_name(project_name)
        , project_file_full_path(project_file_full_path)
        , api(api)
    {
    }

    // CTOR #2.
    RgProject(const std::string&                                  project_name,
              const std::string&                                  project_file_full_path,
              RgProjectAPI                                        api,
              const std::vector<std::shared_ptr<RgProjectClone>>& clones)
        : project_name(project_name)
        , project_file_full_path(project_file_full_path)
        , api(api)
        , clones(clones)
    {
    }

    // Returns "true" if all clones of this project are empty.
    bool IsEmpty()
    {
        return (std::all_of(clones.cbegin(), clones.cend(), [](const std::shared_ptr<RgProjectClone>& clone) { return clone->IsEmpty(); }));
    }

    // Project name.
    std::string project_name;

    // The full path to the .rga file.
    std::string project_file_full_path;

    // Project API: for now, only OpenCL is supported.
    RgProjectAPI api;

    // Project clones.
    std::vector<std::shared_ptr<RgProjectClone>> clones;

    // Project data model version.
    std::string project_data_model_version;
};

// Splitter config structure for saving/restoring GUI layout.
struct RgSplitterConfig
{
    // Identifying name for the splitter.
    std::string splitter_name;

    // Splitter view sizes.
    std::vector<int> splitter_values;
};

// Window size config structure for saving/restoring GUI window size.
struct RgWindowConfig
{
    // Window position.
    int window_x_pos = 0;
    int window_y_pos = 0;

    // Window width.
    int window_width = 0;

    // Window height.
    int window_height = 0;

    // Window state.
    int window_state = 0;
};

// A structure used to hold all data parsed from a line of a resource usage CSV file.
struct RgResourceUsageData
{
    std::string device;
    int         scratch_memory;
    int         threads_per_workgroup;
    int         wavefront_size;
    int         available_lds_bytes;
    int         used_lds_bytes;
    int         available_sgprs;
    int         used_sgprs;
    int         sgpr_spills;
    int         available_vgprs;
    int         used_vgprs;
    int         vgpr_spills;
    int         cl_workgroup_x_dimension;
    int         cl_workgroup_y_dimension;
    int         cl_workgroup_z_dimension;
    int         isa_size;
};

// A structure used to hold data parsed from a livereg output file.
struct RgLiveregData
{
    int                              total_vgprs;
    int                              vgprs_granularity;
    int                              used;
    int                              allocated;
    int                              max_vgprs;
    int                              unmatched_count;
    std::vector<std::pair<int, int>> max_vgpr_line_numbers;
    int                              current_max_vgpr_line_numbers_index = -1;
};

// A structure used to hold project path and api type for each RGA project.
struct RgRecentProject
{
    std::string  project_path;
    RgProjectAPI api_type;
};

// ****************
// Global settings.
// ****************

struct RgGlobalSettings
{
    RgGlobalSettings() = default;

    // Initialize a copy of the incoming settings structure.
    RgGlobalSettings(const RgGlobalSettings& other)
        : log_file_location(other.log_file_location)
        , project_file_location(other.project_file_location)
        , visible_disassembly_view_columns(other.visible_disassembly_view_columns)
        , gui_layout_splitters(other.gui_layout_splitters)
        , default_build_settings(other.default_build_settings)
        , recent_projects(other.recent_projects)
        , last_selected_directory(other.last_selected_directory)
        , use_default_project_name(other.use_default_project_name)
        , should_prompt_for_api(other.should_prompt_for_api)
        , default_api(other.default_api)
        , font_family(other.font_family)
        , font_size(other.font_size)
        , color_theme(other.color_theme)
        , include_files_viewer(other.include_files_viewer)
        , input_file_ext_glsl(other.input_file_ext_glsl)
        , input_file_ext_hlsl(other.input_file_ext_hlsl)
        , input_file_ext_spv_txt(other.input_file_ext_spv_txt)
        , input_file_ext_spv_bin(other.input_file_ext_spv_bin)
        , default_lang(other.default_lang)
    {
    }

    bool HasSameSettings(const RgGlobalSettings& other) const
    {
        bool isSame = (log_file_location.compare(other.log_file_location) == 0) && (project_file_location.compare(other.project_file_location) == 0) &&
                      (visible_disassembly_view_columns == other.visible_disassembly_view_columns) &&
                      (use_default_project_name == other.use_default_project_name) && (should_prompt_for_api == other.should_prompt_for_api) &&
                      (default_api == other.default_api) && (font_family == other.font_family) && (font_size == other.font_size) &&
                      (color_theme == other.color_theme) && (include_files_viewer == other.include_files_viewer) &&
                      (input_file_ext_glsl == other.input_file_ext_glsl) && (input_file_ext_hlsl == other.input_file_ext_hlsl) &&
                      (input_file_ext_spv_txt == other.input_file_ext_spv_txt) && (input_file_ext_spv_bin == other.input_file_ext_spv_bin) &&
                      (default_lang == other.default_lang);

        return isSame;
    }

    // A full path to the location where RGA's log file will be saved.
    std::string log_file_location;

    // A full path to the location where RGA's project file will be saved.
    std::string project_file_location;

    // Visibility flags for each column within the disassembly table.
    std::vector<bool> visible_disassembly_view_columns;

    // Splitter configurations.
    std::vector<RgSplitterConfig> gui_layout_splitters;

    // Window geometry values.
    RgWindowConfig gui_window_config;

    // The default build settings, which can be customized by the user.
    // We keep a mapping, each API will have its entry pointing to its default settings.
    std::map<std::string, std::shared_ptr<RgBuildSettings>> default_build_settings;

    // A vector of recently-accessed projects with their api types.
    std::vector<std::shared_ptr<RgRecentProject>> recent_projects;

    // A full path to the most recently opened directory.
    std::string last_selected_directory;

    // If true, RGA will always use the auto-generated project name,
    // without prompting the user for renaming when creating a new project.
    bool use_default_project_name = true;

    // When RGA starts, should it prompt the user to select the API mode?
    // This can be modified by the user using the "Do not ask me again" option in the startup dialog.
    bool should_prompt_for_api = true;

    // The default API mode to use in the GUI if the user has selected an API
    // and then selected "Do not ask me again" in the startup dialog.
    RgProjectAPI default_api = RgProjectAPI::kUnknown;

    // The font family.
    std::string font_family;

    // The font size.
    int font_size = 0;

    // The application's default color theme.
    int color_theme = 2;

    // The app to use to open include files.
    std::string include_files_viewer = kStrGlobalSettingsSrcViewIncludeViewerDefault;

    // Extensions of input files: GLSL, HLSL, SPIR-V text, and SPIR-V binary.
    std::string input_file_ext_glsl;
    std::string input_file_ext_hlsl;
    std::string input_file_ext_spv_txt;
    std::string input_file_ext_spv_bin;

    // Default high-level language.
    RgSrcLanguage default_lang = RgSrcLanguage::kGLSL;
};

// The possible types of build output files.
enum RgCliOutputFileType
{
    // ISA disassembly in text format.
    kIsaDisassemblyText,

    // ISA disassembly in CSV format.
    kIsaDisassemblyCsv,

    // IL disassembly.
    kIlDisassembly,

    // Project binary.
    kBinary,

    // HW resource usage file.
    kHwResourceUsageFile,

    // Live register analysis report.
    kLiveRegisterAnalysisReport,

    // Control-flow graph.
    kControlFlowGraph
};

// This structure represents a single build output file.
struct RgOutputItem
{
    // The full path to the file.
    std::string file_path;

    // The target GPU that the output was generated for.
    std::string gpu_name;

    // The type of the file.
    RgCliOutputFileType file_type;
};

// This structure represents an input file's entry outputs.
struct RgEntryOutput
{
    // The full path to the input file used to build the output.
    std::string input_file_path;

    // The name of the kernel that was compiled to emit the outputs.
    std::string entrypoint_name;

    // The type of kernel that was compiled.
    std::string kernel_type;

    // The extremely long name of kernel that was compiled (HIP).
    std::string extremely_long_kernel_name;

    // A list of outputs generated for the entry.
    std::vector<RgOutputItem> outputs;
};

struct RgFileOutputs
{
    // The input source file path responsible for the entry.
    std::string input_file_path;

    // The list of entry outputs built from the specified input source file.
    std::vector<RgEntryOutput> outputs;
};

// A map of input file full path to a build output structure for the input.
typedef std::map<std::string, RgFileOutputs> InputFileToBuildOutputsMap;

struct RgCliBuildOutput
{
    virtual ~RgCliBuildOutput() = default;

    // The output as printed by the RGA command line.
    std::string cli_console_output;

    // Mapping between each input file and its outputs.
    InputFileToBuildOutputsMap per_file_output;
};

// This structure represents the output of the command line backend when building an OpenCL project.
struct RgCliBuildOutputOpencl : RgCliBuildOutput
{
    virtual ~RgCliBuildOutputOpencl() = default;

    // The full path to the project binary.
    std::string project_binary;
};

// This structure represents the output of the command line backend when building a pipeline object.
struct RgCliBuildOutputPipeline : RgCliBuildOutput
{
    virtual ~RgCliBuildOutputPipeline() = default;

    // The type of pipeline built by the CLI.
    RgPipelineType type;
};

// A map of GPU name to the project build outputs for the GPU.
typedef std::map<std::string, std::shared_ptr<RgCliBuildOutput>> RgBuildOutputsMap;

// A predicate used to find an RgOutputItem with a specific file type.
struct OutputFileTypeFinder
{
    OutputFileTypeFinder(RgCliOutputFileType file_type)
        : target_file_type(file_type)
    {
    }

    // A predicate that will compare each output item with a target file type to search for.
    bool operator()(const RgOutputItem& output_item) const
    {
        return (output_item.file_type == target_file_type);
    }

    // The target file type to search for.
    RgCliOutputFileType target_file_type;
};

// A map that associates an entry point name with the starting line number and ending line number.
typedef std::map<std::string, std::pair<int, int>> EntryToSourceLineRange;

// Enum that specifies the alternative compiler folder view: bin, include or lib.
enum CompilerFolderType
{
    kBin,
    kInclude,
    kLib
};

// This structure has style sheets for various widgets.
struct RgStylesheetPackage
{
    // The file menu stylesheet.
    std::string file_menu_stylesheet;

    // The API-specific file menu stylesheet.
    std::string file_menu_api_stylesheet;

    // The main window stylesheet.
    std::string main_window_stylesheet;

    // The application stylesheet.
    std::string application_stylesheet;

    // The main window api-specific stylesheet.
    std::string main_window_api_stylesheet;
};

// An enumeration used to classify different types of parsed disassembly lines.
enum class RgIsaLineType
{
    kInstruction,
    kLabel
};

// A simple data class used as a common base for storing lines parsed from a disassembly CSV file.
struct RgIsaLine
{
    // The type of line parsed from the disassembly file.
    RgIsaLineType type;

    // The number of live registers for this instruction.
    std::string num_live_registers;
};

// A class used to store a disassembled instruction line.
struct RgIsaLineInstruction : RgIsaLine
{
    // The instruction address within the disassembled binary.
    std::string address;

    // The instruction opcode.
    std::string opcode;

    // The instruction operands.
    std::string operands;

    // The hex representation of the instruction.
    std::string binary_encoding;
};

// A class used to store a disassembly line label.
struct RgIsaLineLabel : RgIsaLine
{
    // The name of the label, if applicable.
    std::string label_name;
};

#endif  // RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_DATA_TYPES_H_

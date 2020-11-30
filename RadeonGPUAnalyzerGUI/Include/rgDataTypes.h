#pragma once

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
#include <Utils/Include/rgaSharedDataTypes.h>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/rgConfigManager.h>
#include <RadeonGPUAnalyzerGUI/Include/rgStringConstants.h>

// The type of the project.
enum class rgProjectAPI : char
{
    // Unknown.
    Unknown,

    // OpenCL.
    OpenCL,

    // Vulkan.
    Vulkan,

    // The count of known APIs.
    ApiCount
};

// Supported textual source languages.
enum class rgSrcLanguage {
    OpenCL,
    GLSL,
    HLSL,
    SPIRV_Text,

    Unknown
};

// Indices for disassembly view sub widgets.
enum class DisassemblyViewSubWidgets
{
    TableView,
    TargetGpuPushButton,
    ColumnPushButton,
    OutputWindow,
    SourceWindow,
    None,
    Count
};

Q_DECLARE_METATYPE(DisassemblyViewSubWidgets);

// The type of pipeline.
enum class rgPipelineType : char
{
    // A multi-stage graphics pipeline.
    Graphics,

    // A single-stage compute pipeline.
    Compute
};

// An array of file paths for each pipeline stage. An empty string indicates an unused stage.
typedef std::array<std::string, rgPipelineStage::Count> ShaderInputFileArray;

// The base class for pipeline objects.
struct rgPipelineShaders
{
    // The type of pipeline.
    rgPipelineType m_type;

    // An array of input source file paths per pipeline stage.
    ShaderInputFileArray m_shaderStages;
};

// The state for an individual pipeline object.
struct rgPipelineState
{
    // The pipeline name.
    std::string m_name;

    // The full path to the serialized pipeline state file.
    std::string m_pipelineStateFilePath;

    // The full path to the original pipeline state file loaded to the PSO editor.
    std::string m_originalPipelineStateFilePath;

    // A flag used to indicate if the pipeline is currently active.
    bool m_isActive = false;
};

// General RGA settings structure.
struct rgBuildSettings
{
    rgBuildSettings() = default;
    virtual ~rgBuildSettings() = default;

    // A copy constructor used to initialize using another instance.
    rgBuildSettings(const rgBuildSettings& other) :
        m_targetGpus(other.m_targetGpus),
        m_predefinedMacros(other.m_predefinedMacros),
        m_additionalIncludeDirectories(other.m_additionalIncludeDirectories),
        m_additionalOptions(other.m_additionalOptions),
        m_compilerPaths(other.m_compilerPaths),
        m_binaryFileName(other.m_binaryFileName) {}

    virtual bool HasSameSettings(const rgBuildSettings& other) const
    {
        bool isSame =
            (m_targetGpus == other.m_targetGpus) &&
            (m_predefinedMacros == other.m_predefinedMacros) &&
            (m_additionalIncludeDirectories == other.m_additionalIncludeDirectories) &&
            (m_additionalOptions == other.m_additionalOptions) &&
            (m_binaryFileName == other.m_binaryFileName) &&
            (m_compilerPaths == other.m_compilerPaths);

        return isSame;
    }

    // General build settings.
    std::vector<std::string> m_targetGpus;
    std::vector<std::string> m_predefinedMacros;
    std::vector<std::string> m_additionalIncludeDirectories;
    std::string              m_additionalOptions;
    std::string              m_binaryFileName;

    // Alternative compiler paths: {bin, include, lib}.
    std::tuple<std::string, std::string, std::string>  m_compilerPaths;
};

// An info structure for each source file in a project.
struct rgSourceFileInfo
{
    // The full path to the source file.
    std::string m_filePath;

    // A flag indicating if the file is correlated with current build output.
    bool m_isCorrelated = false;
};

// A base class for project clones.
struct rgProjectClone
{
    rgProjectClone() = default;
    virtual ~rgProjectClone() = default;

    rgProjectClone(const std::string& cloneName, std::shared_ptr<rgBuildSettings> pBuildSettings) :
        m_cloneName(cloneName), m_pBuildSettings(pBuildSettings){}

    // Returns "true" if the clone does not have any source files or "false" otherwise.
    virtual bool IsEmpty() const { return m_sourceFiles.empty(); }

    unsigned m_cloneId = 0;
    std::string m_cloneName;
    std::vector<rgSourceFileInfo> m_sourceFiles;
    std::shared_ptr<rgBuildSettings> m_pBuildSettings = nullptr;
};

// A project clone containing graphics or compute pipeline state info.
struct rgGraphicsProjectClone : rgProjectClone
{
    rgGraphicsProjectClone() = default;
    virtual ~rgGraphicsProjectClone() = default;

    // CTOR used to initialize with existing build settings.
    rgGraphicsProjectClone(const std::string& cloneName, std::shared_ptr<rgBuildSettings> pBuildSettings) :
        rgProjectClone(cloneName, pBuildSettings) {}

    // Returns "true" if the clone does not have any source files or "false" otherwise.
    virtual bool IsEmpty() const override
    {
        return std::all_of(m_pipeline.m_shaderStages.cbegin(), m_pipeline.m_shaderStages.cend(),
                           [&](const std::string& file) { return file.empty(); });
    }

    // A list of pipeline states within the clone.
    std::vector<rgPipelineState> m_psoStates;

    // The pipeline object, which specifies the pipeline type, and
    // includes full paths to shader files associated with the pipeline's stages.
    rgPipelineShaders m_pipeline;
};

// This structure represents a family of GPU products.
struct rgGpuFamily
{
    // The GPU family name.
    std::string m_familyName;

    // An array of GPU product names.
    std::vector<std::string> m_productNames;
};

// This structure represents a GPU architecture.
struct rgGpuArchitecture
{
    // The name of the GPU HW architecture.
    std::string m_architectureName;

    // An array of GPU families within this GPU hardware architecture.
    std::vector<rgGpuFamily> m_gpuFamilies;
};

// Structure containing the CLI version info.
struct rgCliVersionInfo
{
    // The CLI version string.
    std::string m_version;

    // The build date.
    std::string m_buildDate;

    // A map of build mode to an array of all supported GPU architectures for the mode.
    std::map<std::string, std::vector<rgGpuArchitecture>> m_gpuArchitectures;
};

// Base structure for projects.
struct rgProject
{
    // Default CTOR, DTOR.
    rgProject() = default;
    virtual ~rgProject() = default;

    // CTOR #1.
    rgProject(const std::string& projectName,
        const std::string& projectFileFullPath, rgProjectAPI api) : m_projectName(projectName),
        m_projectFileFullPath(projectFileFullPath), m_api(api) {}

    // CTOR #2.
    rgProject(const std::string& projectName, const std::string& projectFileFullPath, rgProjectAPI api, const std::vector<std::shared_ptr<rgProjectClone>>& clones) :
        m_projectName(projectName), m_projectFileFullPath(projectFileFullPath), m_api(api),  m_clones(clones) {}

    // Returns "true" if all clones of this project are empty.
    bool IsEmpty()
    {
        return (std::all_of(m_clones.cbegin(), m_clones.cend(),
                            [](const std::shared_ptr<rgProjectClone>& pClone) { return pClone->IsEmpty(); }));
    }

    // Project name.
    std::string m_projectName;

    // The full path to the .rga file.
    std::string m_projectFileFullPath;

    // Project API: for now, only OpenCL is supported.
    rgProjectAPI m_api;

    // Project clones.
    std::vector<std::shared_ptr<rgProjectClone>> m_clones;

    // Project data model version.
    std::string m_projectDataModelVersion;
};

// Splitter config structure for saving/restoring GUI layout.
struct rgSplitterConfig
{
    // Identifying name for the splitter.
    std::string m_splitterName;

    // Splitter view sizes.
    std::vector<int> m_splitterValues;
};

// Window size config structure for saving/restoring GUI window size.
struct rgWindowConfig
{
    // Window position.
    int m_windowXPos = 0;
    int m_windowYPos = 0;

    // Window width.
    int m_windowWidth = 0;

    // Window height.
    int m_windowHeight = 0;

    // Window state.
    int m_windowState = 0;
};

// A structure used to hold all data parsed from a line of a resource usage CSV file.
struct rgResourceUsageData
{
    std::string m_device;
    int m_scratchMemory;
    int m_threadsPerWorkgroup;
    int m_wavefrontSize;
    int m_availableLdsBytes;
    int m_usedLdsBytes;
    int m_availableSgprs;
    int m_usedSgprs;
    int m_sgprSpills;
    int m_availableVgprs;
    int m_usedVgprs;
    int m_vgprSpills;
    int m_clWorkgroupXDimension;
    int m_clWorkgroupYDimension;
    int m_clWorkgroupZDimension;
    int m_isaSize;
};

// A structure used to hold project path and api type for each RGA project.
struct rgRecentProject
{
    std::string projectPath;
    rgProjectAPI apiType;
};

// ****************
// Global settings.
// ****************

struct rgGlobalSettings
{
    rgGlobalSettings() = default;

    // Initialize a copy of the incoming settings structure.
    rgGlobalSettings(const rgGlobalSettings& other) :
        m_logFileLocation(other.m_logFileLocation),
        m_visibleDisassemblyViewColumns(other.m_visibleDisassemblyViewColumns),
        m_guiLayoutSplitters(other.m_guiLayoutSplitters),
        m_pDefaultBuildSettings(other.m_pDefaultBuildSettings),
        m_recentProjects(other.m_recentProjects),
        m_lastSelectedDirectory(other.m_lastSelectedDirectory),
        m_useDefaultProjectName(other.m_useDefaultProjectName),
        m_shouldPromptForAPI(other.m_shouldPromptForAPI),
        m_defaultAPI(other.m_defaultAPI),
        m_fontFamily(other.m_fontFamily),
        m_fontSize(other.m_fontSize),
        m_includeFilesViewer(other.m_includeFilesViewer),
        m_inputFileExtGlsl(other.m_inputFileExtGlsl),
        m_inputFileExtHlsl(other.m_inputFileExtHlsl),
        m_inputFileExtSpvTxt(other.m_inputFileExtSpvTxt),
        m_inputFileExtSpvBin(other.m_inputFileExtSpvBin),
        m_defaultLang(other.m_defaultLang)
    {}

    bool HasSameSettings(const rgGlobalSettings& other) const
    {
        bool isSame = (m_logFileLocation.compare(other.m_logFileLocation) == 0) &&
                      (m_visibleDisassemblyViewColumns == other.m_visibleDisassemblyViewColumns) &&
                      (m_useDefaultProjectName == other.m_useDefaultProjectName) &&
                      (m_shouldPromptForAPI == other.m_shouldPromptForAPI) &&
                      (m_defaultAPI == other.m_defaultAPI) &&
                      (m_fontFamily == other.m_fontFamily) &&
                      (m_fontSize == other.m_fontSize) &&
                      (m_includeFilesViewer == other.m_includeFilesViewer) &&
                      (m_inputFileExtGlsl == other.m_inputFileExtGlsl) &&
                      (m_inputFileExtHlsl == other.m_inputFileExtHlsl) &&
                      (m_inputFileExtSpvTxt == other.m_inputFileExtSpvTxt) &&
                      (m_inputFileExtSpvBin == other.m_inputFileExtSpvBin) &&
                      (m_defaultLang == other.m_defaultLang);

        return isSame;
    }

    // A full path to the location where RGA's log file will be saved.
    std::string m_logFileLocation;

    // Visibility flags for each column within the disassembly table.
    std::vector<bool> m_visibleDisassemblyViewColumns;

    // Splitter configurations.
    std::vector<rgSplitterConfig> m_guiLayoutSplitters;

    // Window geometry values.
    rgWindowConfig m_guiWindowConfig;

    // The default build settings, which can be customized by the user.
    // We keep a mapping, each API will have its entry pointing to its default settings.
    std::map<std::string, std::shared_ptr<rgBuildSettings>> m_pDefaultBuildSettings;

    // A vector of recently-accessed projects with their api types.
    std::vector<std::shared_ptr<rgRecentProject>> m_recentProjects;

    // A full path to the most recently opened directory.
    std::string m_lastSelectedDirectory;

    // If true, RGA will always use the auto-generated project name,
    // without prompting the user for renaming when creating a new project.
    bool m_useDefaultProjectName = false;

    // When RGA starts, should it prompt the user to select the API mode?
    // This can be modified by the user using the "Do not ask me again" option in the startup dialog.
    bool m_shouldPromptForAPI = true;

    // The default API mode to use in the GUI if the user has selected an API
    // and then selected "Do not ask me again" in the startup dialog.
    rgProjectAPI m_defaultAPI = rgProjectAPI::Unknown;

    // The font family.
    std::string m_fontFamily;

    // The font size.
    int m_fontSize;

    // The app to use to open include files.
    std::string m_includeFilesViewer = STR_GLOBAL_SETTINGS_SRC_VIEW_INCLUDE_VIEWER_DEFAULT;

    // Extensions of input files: GLSL, HLSL, SPIR-V text, and SPIR-V binary.
    std::string m_inputFileExtGlsl;
    std::string m_inputFileExtHlsl;
    std::string m_inputFileExtSpvTxt;
    std::string m_inputFileExtSpvBin;

    // Default high-level language.
    rgSrcLanguage m_defaultLang = rgSrcLanguage::GLSL;
};

// The possible types of build output files.
enum rgCliOutputFileType
{
    // ISA disassembly in text format.
    IsaDisassemblyText,

    // ISA disassembly in CSV format.
    IsaDisassemblyCsv,

    // IL disassembly.
    IlDisassembly,

    // Project binary.
    Binary,

    // HW resource usage file.
    HwResourceUsageFile,

    // Live register analysis report.
    LiveRegisterAnalysisReport,

    // Control-flow graph.
    ControlFlowGraph
};

// This structure represents a single build output file.
struct rgOutputItem
{
    // The full path to the file.
    std::string m_filePath;

    // The target GPU that the output was generated for.
    std::string m_gpuName;

    // The type of the file.
    rgCliOutputFileType m_fileType;
};

// This structure represents an input file's entry outputs.
struct rgEntryOutput
{
    // The full path to the input file used to build the output.
    std::string m_inputFilePath;

    // The name of the kernel that was compiled to emit the outputs.
    std::string m_entrypointName;

    // The type of kernel that was compiled.
    std::string m_kernelType;

    // A list of outputs generated for the entry.
    std::vector<rgOutputItem> m_outputs;
};

struct rgFileOutputs
{
    // The input source file path responsible for the entry.
    std::string m_inputFilePath;

    // The list of entry outputs built from the specified input source file.
    std::vector<rgEntryOutput> m_outputs;
};

// A map of input file full path to a build output structure for the input.
typedef std::map<std::string, rgFileOutputs> InputFileToBuildOutputsMap;

struct rgCliBuildOutput
{
    virtual ~rgCliBuildOutput() = default;

    // The output as printed by the RGA command line.
    std::string m_cliConsoleOutput;

    // Mapping between each input file and its outputs.
    InputFileToBuildOutputsMap m_perFileOutput;
};

// This structure represents the output of the command line backend when building an OpenCL project.
struct rgCliBuildOutputOpenCL : rgCliBuildOutput
{
    virtual ~rgCliBuildOutputOpenCL() = default;

    // The full path to the project binary.
    std::string m_projectBinary;
};

// This structure represents the output of the command line backend when building a pipeline object.
struct rgCliBuildOutputPipeline : rgCliBuildOutput
{
    virtual ~rgCliBuildOutputPipeline() = default;

    // The type of pipeline built by the CLI.
    rgPipelineType m_type;
};

// A map of GPU name to the project build outputs for the GPU.
typedef std::map<std::string, std::shared_ptr<rgCliBuildOutput>> rgBuildOutputsMap;

// A predicate used to find an rgOutputItem with a specific file type.
struct OutputFileTypeFinder
{
    OutputFileTypeFinder(rgCliOutputFileType fileType) : m_targetFileType(fileType) {}

    // A predicate that will compare each output item with a target file type to search for.
    bool operator()(const rgOutputItem& outputItem) const
    {
        return (outputItem.m_fileType == m_targetFileType);
    }

    // The target file type to search for.
    rgCliOutputFileType m_targetFileType;
};

// A map that associates an entry point name with the starting line number and ending line number.
typedef std::map<std::string, std::pair<int, int>> EntryToSourceLineRange;

// Enum that specifies the alternative compiler folder view: bin, include or lib.
enum CompilerFolderType
{
    Bin,
    Include,
    Lib
};

// This structure has style sheets for various widgets.
struct rgStylesheetPackage
{
    // The file menu stylesheet.
    std::string m_fileMenuStylesheet;

    // The API-specific file menu stylesheet.
    std::string m_fileMenuApiStylesheet;

    // The main window stylesheet.
    std::string m_mainWindowStylesheet;

    // The application stylesheet.
    std::string m_applicationStylesheet;

    // The main window api-specific stylesheet.
    std::string m_mainWindowAPIStylesheet;
};

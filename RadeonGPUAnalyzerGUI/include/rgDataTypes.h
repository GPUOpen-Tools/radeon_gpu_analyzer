#pragma once

// C++.
#include <map>
#include <memory>
#include <string>
#include <vector>

// Local.
#include <RadeonGPUAnalyzerGUI/include/rgConfigManager.h>

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
        m_compilerPaths(other.m_compilerPaths) {}

    // General build settings.
    std::vector<std::string> m_targetGpus;
    std::vector<std::string> m_predefinedMacros;
    std::vector<std::string> m_additionalIncludeDirectories;
    std::string              m_additionalOptions;

    // Alternative compiler paths: {bin, include, lib}.
    std::tuple<std::string, std::string, std::string>  m_compilerPaths;
};

// OpenCL build settings.
struct rgCLBuildSettings : public rgBuildSettings
{
    rgCLBuildSettings() = default;
    virtual ~rgCLBuildSettings() = default;

    // Copy constructor used to initialize using another instance.
    rgCLBuildSettings(const rgCLBuildSettings& other) :
        rgBuildSettings(other),
        m_optimizationLevel(other.m_optimizationLevel),
        m_isTreatDoubleAsSingle(other.m_isTreatDoubleAsSingle),
        m_isDenormsAsZeros(other.m_isDenormsAsZeros),
        m_isStrictAliasing(other.m_isStrictAliasing),
        m_isEnableMAD(other.m_isEnableMAD),
        m_isIgnoreZeroSignedness(other.m_isIgnoreZeroSignedness),
        m_isUnsafeOptimizations(other.m_isUnsafeOptimizations),
        m_isNoNanNorInfinite(other.m_isNoNanNorInfinite),
        m_isAggressiveMathOptimizations(other.m_isAggressiveMathOptimizations),
        m_isCorrectlyRoundDivSqrt(other.m_isCorrectlyRoundDivSqrt) {}

    // Default values for specific settings.
    const std::string OPENCL_DEFAULT_OPT_LEVEL   = "Default";

    // OpenCL-specific build settings.
    std::string m_optimizationLevel = OPENCL_DEFAULT_OPT_LEVEL;
    bool m_isTreatDoubleAsSingle = false;
    bool m_isDenormsAsZeros= false;
    bool m_isStrictAliasing = false;
    bool m_isEnableMAD = false;
    bool m_isIgnoreZeroSignedness = false;
    bool m_isUnsafeOptimizations = false;
    bool m_isNoNanNorInfinite = false;
    bool m_isAggressiveMathOptimizations = false;
    bool m_isCorrectlyRoundDivSqrt = false;
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

    unsigned m_cloneId = 0;
    std::string m_cloneName;
    std::vector<rgSourceFileInfo> m_sourceFiles;
    std::shared_ptr<rgBuildSettings> m_pBuildSettings = std::make_shared<rgCLBuildSettings>();
};

// A clone of an OpenCL project.
struct rgCLProjectClone : public rgProjectClone
{
    rgCLProjectClone() = default;

    rgCLProjectClone(const std::string& cloneName, std::shared_ptr<rgCLBuildSettings> pBuildSettings) :
        rgProjectClone(cloneName, pBuildSettings){}
};

// The type of the project.
enum rgProjectAPI : char
{
    // Unknown.
    Unknown,

    // OpenCL.
    OpenCL
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

    // Project name.
    std::string m_projectName;

    // The full path to the .rga file.
    std::string m_projectFileFullPath;

    // Project API: for now, only OpenCL is supported.
    rgProjectAPI m_api;

    // Project clones.
    std::vector<std::shared_ptr<rgProjectClone>> m_clones;
};


// An OpenCL project.
struct rgCLProject : public rgProject
{
    rgCLProject() : rgProject("", "", rgProjectAPI::OpenCL){}

    // CTOR #1.
    rgCLProject(const std::string& projectName, const std::string& projectFileFullPath) : rgProject(projectName,
        projectFileFullPath, rgProjectAPI::OpenCL){}

    // CTOR #2.
    rgCLProject(const std::string& projectName, const std::string& projectFileFullPath,
        const std::vector<std::shared_ptr<rgProjectClone>>& clones) :
        rgProject(projectName, projectFileFullPath, rgProjectAPI::OpenCL, clones) {}
};

// Splitter config structure for saving/restoring GUI layout.
struct rgSplitterConfig
{
    // Identifying name for the splitter.
    std::string m_splitterName;

    // Splitter view sizes.
    std::vector<int> m_splitterValues;
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
        m_useDefaultProjectName(other.m_useDefaultProjectName) {}

    // A full path to the location where RGA's log file will be saved.
    std::string m_logFileLocation;

    // Visibility flags for each column within the disassembly table.
    std::vector<bool> m_visibleDisassemblyViewColumns;

    // Splitter configurations.
    std::vector<rgSplitterConfig> m_guiLayoutSplitters;

    // The default build settings, which can be customized by the user.
    // We keep a mapping, each API will have its entry pointing to its default settings.
    std::map<std::string, std::shared_ptr<rgBuildSettings>> m_pDefaultBuildSettings;

    // An array of recently-accessed projects.
    std::vector<std::string> m_recentProjects;

    // A full path to the most recently opened directory.
    std::string m_lastSelectedDirectory;

    // If true, RGA will always use the auto-generated project name,
    // without prompting the user for renaming when creating a new project.
    bool m_useDefaultProjectName = false;
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
    std::string m_kernelName;

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

// This structure represents the output of the command line backend.
struct rgCliBuildOutput
{
    // The output as printed by the RGA command line.
    std::string m_cliConsoleOutput;

    // The full path to the project binary.
    std::string m_projectBinary;

    // Mapping between each input file and its outputs.
    std::map<std::string, rgFileOutputs> m_perFileOutput;
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

// A map that associates an entrypoint name with the starting line number and ending line number.
typedef std::map<std::string, std::pair<int, int>> EntryToSourceLineRange;

// Enum that specifies the alternative compiler folder view: bin, include or lib.
enum CompilerFolderType
{
    Bin,
    Include,
    Lib
};

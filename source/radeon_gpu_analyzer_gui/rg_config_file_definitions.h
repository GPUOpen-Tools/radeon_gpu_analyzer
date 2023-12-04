#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_CONFIG_FILE_DEFINITIONS_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_CONFIG_FILE_DEFINITIONS_H_

// C++.
#include <string>

// Local.
#include "radeon_gpu_analyzer_gui/rg_string_constants.h"

// ***********************************************
// *** Configuration-file-related definitions. ***
// ***********************************************

// The data model version that RGA uses.
static const std::string kRgaDataModel2_0 = "2.0";
static const std::string kRgaDataModel2_1 = "2.1";
static const std::string kRgaDataModel2_2 = "2.2";
static const std::string kRgaDataModel2_3 = "2.3";
static const std::string kRgaDataModel2_4 = "2.4";
static const std::string kRgaDataModel    = kRgaDataModel2_4;

// The XML declaration string.
static const char* kRgaXmlDeclaration = "xml version = \"1.0\" encoding = \"UTF-8\"";

// Data model version node.
static const char* kXmlNodeDataModelVersion = "RGADataModelVersion";

// Program node.
static const char* kXmlNodeProject = "Program";

// API node.
static const char* kXmlNodeApiName = "ProgramAPI";

// Program name node.
static const char* kXmlNodeProjectName = "ProgramName";

// Program clone node.
static const char* kXmlNodeClone = "ProgramClone";

// Clone ID node.
static const char* kXmlNodeCloneId = "CloneID";

// Clone name node.
static const char* kXmlNodeCloneName = "CloneName";

// Clone source files.
static const char* kXmlNodeCloneSourceFiles = "SourceFiles";

// File path.
static const char* kXmlNodeFilePath = "FullPath";

// Build settings node.
static const char* kXmlNodeBuildSettings = "BuildSettings";

// Target devices settings node.
static const char* kXmlNodeTargetDevices = "TargetDevices";

// Predefined macros settings node.
static const char* kXmlNodePredefinedMacros = "PredefinedMacros";

// Additional include directories node.
static const char* kXmlNodeAdditionalIncludeDirectories = "AdditionalIncludeDirectories";

// Additional options provided by user.
static const char* kXmlNodeAdditionalOptions = "AdditionalOptions";

// A switch used to determine if project names will be generated or provided by the user.
static const char* kXmlNodeUseGeneratedProjectNames = "UseGeneratedProjectNames";

// Alternative compiler paths.
static const char* kXmlNodeAlternativeCompilerBinDir = "AlternativeCompilerBin";
static const char* kXmlNodeAlternativeCompilerIncDir = "AlternativeCompilerInc";
static const char* kXmlNodeAlternativeCompilerLibDir = "AlternativeCompilerLib";

// Binary input file.
static const char* kXmlNodeGlobalBinaryInputFileName = "BinaryInputFileName";

// *******************************
// *** OPENCL-SPECIFIC - BEGIN ***
// *******************************

// OpenCL optimization level.
static const char* kXmlNodeOpenclOptLevel = "OptimizationLevel";

// Treat double as single.
static const char* kXmlNodeOpenclDoubleAsSingle = "TreatDoubleAsSingle";

// Denorms as zeros.
static const char* kXmlNodeOpenclDenormsAsZeros = "DenormsAsZeros";

// Strict aliasing.
static const char* kXmlNodeOpenclStrictAliasing = "StrictAliasing";

// Enable MAD.
static const char* kXmlNodeOpenclEnableMad = "EnableMAD";

// Ignore zero signedness.
static const char* kXmlNodeOpenclIgnoreZeroSignedness = "IgnoreZeroSignedness";

// Allow unsafe optimizations.
static const char* kXmlNodeOpenclUnsafeOpt = "UnsafeOptimizations";

// No NaN nor Infinite.
static const char* kXmlNodeOpenclNoNanNorInf = "NoNanNorInfinite";

// Aggressive math optimizations.
static const char* kXmlNodeOpenclAggressiveMathOpt = "AggressiveMathOptimizations";

// Correctly round div / sqrt.
static const char* kXmlNodeOpenclCorrectRoundDivSqrt = "CorrectlyRoundDivSqrt";

// *****************************
// *** OPENCL-SPECIFIC - END ***
// *****************************

// *******************************
// *** GRAPHICS-SPECIFIC - BEGIN ***
// *******************************

// The root node for pipeline data.
static const char* kXmlNodePipelineShadersRoot = "Pipeline";

// The root node for the backup SPIR-V binary files.
static const char* kXmlNodeBackupSpvRoot = "BackupSpvFiles";

// The root node for the pipeline state data.
static const char* kXmlNodePipelineStateRoot = "PipelineState";

// The root node for an individual pipeline's state data.
static const char* kXmlNodePipelineState = "State";

// The element used to serialize the pipeline name.
static const char* kXmlNodePipelineName = "Name";

// The element used to serialize the active pipeline flag.
static const char* kXmlNodePipelineIsActive = "IsActive";

// The element used to serialize the pipeline state file path.
static const char* kXmlNodePipelineStateFilePath = "PipelineStateFilePath";

// The element used to serialize the original pipeline state file path.
static const char* kXmlNodeOriginalPipelineStateFilePath = "OriginalPipelineStateFilePath";

// The type of pipeline being serialized.
static const char* kXmlNodePipelineType = "Type";

// The graphics pipeline type string.
static const char* kXmlNodePipelineTypeGraphics = "Graphics";

// The compute pipeline type string.
static const char* kXmlNodePipelineTypeCompute = "Compute";

// Used to serialize the path to a shader input file.
static const char* kXmlNodePipelineVertexStage = "Vertex";

// Used to serialize the path to a tessellation control shader input file.
static const char* kXmlNodePipelineTessControlStage = "TessellationControl";

// Used to serialize the path to a tessellation evaluation shader input file.
static const char* kXmlNodePipelineTessEvalStage = "TessellationEvaluation";

// Used to serialize the path to a geometry shader input file.
static const char* kXmlNodePipelineGeometryStage = "Geometry";

// Used to serialize the path to a fragment shader input file.
static const char* kXmlNodePipelineFragmentStage = "Fragment";

// Used to serialize the path to a compute shader input file.
static const char* kXmlNodePipelineComputeStage = "Compute";

// *******************************
// *** GRAPHICS-SPECIFIC - END ***
// *******************************

// *******************************
// *** VULKAN-SPECIFIC - BEGIN ***
// *******************************

// Used to serialize the Generate Debug Info build setting.
static const char* kXmlNodeVulkanGenerateDebugInfo = "GenerateDebugInfo";

// Used to serialize the No Explicit Bindings build setting.
static const char* kXmlNodeVulkanNoExplicitBindings = "NoExplicitBindings";

// Used to serialize the Use HLSL Block Offsets build setting.
static const char* kXmlNodeVulkanUseHlslBlockOffsets = "UseHlslBlockOffsets";

// Used to serialize the Use HLSL IO Mapping build setting.
static const char* kXmlNodeVulkanUseHlslIoMapping = "UseHlslIoMapping";

// Used to serialize the ICD location build setting.
static const char* kXmlNodeVulkanIcdLocation = "ICDLocation";

// Used to serialize the glslang additional options build setting.
static const char* kXmlNodeVulkanGlslangOptionsLocation = "glslangOptions";

// Used to serialize the Enable validation layers build setting.
static const char* kXmlNodeVulkanEnableValidationLayer = "EnableValidationLayer";

// Binary output file association.
static const char* kXmlNodeGlobalBinaryOutputFileName = "BinaryOutputFileName";

// *******************************
// *** VULKAN-SPECIFIC - END ***
// *******************************

// *******************************
// *** GLOBAL SETTINGS - BEGIN ***
// *******************************

// Global settings root node.
static const char* kXmlNodeGlobalLogFileGlobalSettings = "GlobalSettings";

// Log file location.
static const char* kXmlNodeGlobalLogFileLocation = "LogFileLocation";

// Project file location.
static const char* kXmlNodeGlobalProjectFileLocation = "ProjectFileLocation";

// Output directory.
static const char* kXmlNodeGlobalOutputDir = "OutputDirectory";

// Last selected directory.
static const char* kXmlNodeGlobalLastSelectedDirectory = "LastSelectedDirectory";

// Default API mode (introduced in RGA Data Model 2.1)
static const char* kXmlNodeGlobalDefaultApi = "DefaultAPI";

// Indicates whether or not to (introduced in RGA Data Model 2.1)
static const char* kXmlNodeGlobalPromptForApi = "PromptForAPI";

// The recent programs root node.
static const char* kXmlNodeGlobalRecentProjectsRoot = "RecentPrograms";

// The recent program root node.
static const char* kXmlNodeGlobalRecentProjectRoot = "RecentProgram";

// The program path element.
static const char* kXmlNodeGlobalRecentProjectPath = "Path";

// The program api type element.
static const char* kXmlNodeGlobalRecentProjectApi = "API";

// The font root node.
static const char* kXmlNodeGlobalFontFamilyRoot = "Font";

// The font type element.
static const char* kXmlNodeGlobalFontFamilyType = "Type";

// The font size element.
static const char* kXmlNodeGlobalFontSize = "Size";

// The default include file viewer.
static const char* kXmlNodeGlobalIncludeFilesViewer = "IncludeFilesViewer";

// Disassembly view columns.
static const char* kXmlNodeGlobalDisassemblyColumns = "DisassemblyViewColumns";

// GUI elements.
static const char* kXmlNodeGlobalGui = "GUI";

// Gui layout.
static const char* kXmlNodeGlobalGuiLayout = "Layout";

// Gui splitter config.
static const char* kXmlNodeGlobalGuiSplitter = "Splitter";

// Splitter config name.
static const char* kXmlNodeGlobalGuiSplitterName = "SplitterName";

// Splitter config values.
static const char* kXmlNodeGlobalGuiSplitterValues = "SplitterValues";

// RGA window size values.
static const char* kXmlNodeGlobalGuiWindowGeometry = "WindowGeometry";

// RGA window width.
static const char* kXmlNodeGlobalGuiWindowWidth = "WindowWidth";

// RGA window height.
static const char* kXmlNodeGlobalGuiWindowHeight = "WindowHeight";

// RGA window state.
static const char* kXmlNodeGlobalGuiWindowState = "WindowState";

// RGA window X location.
static const char* kXmlNodeGlobalGuiWindowXPos = "WindowXPos";

// RGA window Y location.
static const char* kXmlNodeGlobalGuiWindowYPos = "WindowYPos";

// Default build settings.
static const char* kXmlNodeGlobalDefaultBuildSettings = "DefaultBuildSettings";

// Input file associations.
static const char* kXmlNodeGlobalInputFileExtGlsl   = "InputFileExtGLSL";
static const char* kXmlNodeGlobalInputFileExtHlsl   = "InputFileExtHLSL";
static const char* kXmlNodeGlobalInputFileExtSpvTxt = "InputFileExtSpvTxt";
static const char* kXmlNodeGlobalInputFileExtSpvBin = "InputFileExtSpvBin";

// Default source language.
static const char* kXmlNodeGlobalDefaultSrcLang = "DefaultSrcLanguage";

// *****************************
// *** GLOBAL SETTINGS - END ***
// *****************************
#endif  // RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_CONFIG_FILE_DEFINITIONS_H_
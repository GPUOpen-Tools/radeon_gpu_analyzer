#pragma once

// C++.
#include <string>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/rgStringConstants.h>

// ***********************************************
// *** Configuration-file-related definitions. ***
// ***********************************************

// The data model version that RGA uses.
static const std::string RGA_DATA_MODEL_2_0 = "2.0";
static const std::string RGA_DATA_MODEL_2_1 = "2.1";
static const std::string RGA_DATA_MODEL = RGA_DATA_MODEL_2_1;

// The XML declaration string.
static const char* RGA_XML_DECLARATION = "xml version = \"1.0\" encoding = \"UTF-8\"";

// Data model version node.
static const char* XML_NODE_DATA_MODEL_VERSION = "RGADataModelVersion";

// Program node.
static const char* XML_NODE_PROJECT = "Program";

// API node.
static const char* XML_NODE_API_NAME = "ProgramAPI";

// Program name node.
static const char* XML_NODE_PROJECT_NAME = "ProgramName";

// Program clone node.
static const char* XML_NODE_CLONE = "ProgramClone";

// Clone ID node.
static const char* XML_NODE_CLONE_ID = "CloneID";

// Clone name node.
static const char* XML_NODE_CLONE_NAME = "CloneName";

// Clone source files.
static const char* XML_NODE_CLONE_SOURCE_FILES = "SourceFiles";

// File path.
static const char* XML_NODE_FILE_PATH = "FullPath";

// Build settings node.
static const char* XML_NODE_BUILD_SETTINGS = "BuildSettings";

// Target devices settings node.
static const char* XML_NODE_TARGET_DEVICES = "TargetDevices";

// Predefined macros settings node.
static const char* XML_NODE_PREDEFINED_MACROS = "PredefinedMacros";

// Additional include directories node.
static const char* XML_NODE_ADDITIONAL_INCLUDE_DIRECTORIES = "AdditionalIncludeDirectories";

// Additional options provided by user.
static const char* XML_NODE_ADDITIONAL_OPTIONS = "AdditionalOptions";

// A switch used to determine if project names will be generated or provided by the user.
static const char* XML_NODE_USE_GENERATED_PROJECT_NAMES = "UseGeneratedProjectNames";

// Alternative compiler paths.
static const char* XML_NODE_ALTERNATIVE_COMPILER_BIN_DIR = "AlternativeCompilerBin";
static const char* XML_NODE_ALTERNATIVE_COMPILER_INC_DIR = "AlternativeCompilerInc";
static const char* XML_NODE_ALTERNATIVE_COMPILER_LIB_DIR = "AlternativeCompilerLib";

// *******************************
// *** OPENCL-SPECIFIC - BEGIN ***
// *******************************

// OpenCL optimization level.
static const char* XML_NODE_OPENCL_OPT_LEVEL = "OptimizationLevel";

// Treat double as single.
static const char* XML_NODE_OPENCL_DOUBLE_AS_SINGLE = "TreatDoubleAsSingle";

// Denorms as zeros.
static const char* XML_NODE_OPENCL_DENORMS_AS_ZEROS = "DenormsAsZeros";

// Strict aliasing.
static const char* XML_NODE_OPENCL_STRICT_ALIASING = "StrictAliasing";

// Enable MAD.
static const char* XML_NODE_OPENCL_ENABLE_MAD = "EnableMAD";

// Ignore zero signedness.
static const char* XML_NODE_OPENCL_IGNORE_ZERO_SIGNEDNESS = "IgnoreZeroSignedness";

// Allow unsafe optimizations.
static const char* XML_NODE_OPENCL_UNSAFE_OPT = "UnsafeOptimizations";

// No NaN nor Infinite.
static const char* XML_NODE_OPENCL_NO_NAN_NOR_INF = "NoNanNorInfinite";

// Aggressive math optimizations.
static const char* XML_NODE_OPENCL_AGGRESSIVE_MATH_OPT = "AggressiveMathOptimizations";

// Correctly round div / sqrt.
static const char* XML_NODE_OPENCL_CORRECT_ROUND_DIV_SQRT = "CorrectlyRoundDivSqrt";

// *****************************
// *** OPENCL-SPECIFIC - END ***
// *****************************

// *******************************
// *** GRAPHICS-SPECIFIC - BEGIN ***
// *******************************

// The root node for pipeline data.
static const char* XML_NODE_PIPELINE_SHADERS_ROOT = "Pipeline";

// The root node for the backup SPIR-V binary files.
static const char* XML_NODE_BACKUP_SPV_ROOT = "BackupSpvFiles";

// The root node for the pipeline state data.
static const char* XML_NODE_PIPELINE_STATE_ROOT = "PipelineState";

// The root node for an individual pipeline's state data.
static const char* XML_NODE_PIPELINE_STATE = "State";

// The element used to serialize the pipeline name.
static const char* XML_NODE_PIPELINE_NAME = "Name";

// The element used to serialize the active pipeline flag.
static const char* XML_NODE_PIPELINE_IS_ACTIVE = "IsActive";

// The element used to serialize the pipeline state file path.
static const char* XML_NODE_PIPELINE_STATE_FILE_PATH = "PipelineStateFilePath";

// The element used to serialize the original pipeline state file path.
static const char* XML_NODE_ORIGINAL_PIPELINE_STATE_FILE_PATH = "OriginalPipelineStateFilePath";

// The type of pipeline being serialized.
static const char* XML_NODE_PIPELINE_TYPE = "Type";

// The graphics pipeline type string.
static const char* XML_NODE_PIPELINE_TYPE_GRAPHICS = "Graphics";

// The compute pipeline type string.
static const char* XML_NODE_PIPELINE_TYPE_COMPUTE = "Compute";

// Used to serialize the path to a shader input file.
static const char* XML_NODE_PIPELINE_VERTEX_STAGE = "Vertex";

// Used to serialize the path to a tessellation control shader input file.
static const char* XML_NODE_PIPELINE_TESS_CONTROL_STAGE = "TessellationControl";

// Used to serialize the path to a tessellation evaluation shader input file.
static const char* XML_NODE_PIPELINE_TESS_EVAL_STAGE = "TessellationEvaluation";

// Used to serialize the path to a geometry shader input file.
static const char* XML_NODE_PIPELINE_GEOMETRY_STAGE = "Geometry";

// Used to serialize the path to a fragment shader input file.
static const char* XML_NODE_PIPELINE_FRAGMENT_STAGE = "Fragment";

// Used to serialize the path to a compute shader input file.
static const char* XML_NODE_PIPELINE_COMPUTE_STAGE = "Compute";

// *******************************
// *** GRAPHICS-SPECIFIC - END ***
// *******************************

// *******************************
// *** VULKAN-SPECIFIC - BEGIN ***
// *******************************

// Used to serialize the Generate Debug Info build setting.
static const char* XML_NODE_VULKAN_GENERATE_DEBUG_INFO = "GenerateDebugInfo";

// Used to serialize the No Explicit Bindings build setting.
static const char* XML_NODE_VULKAN_NO_EXPLICIT_BINDINGS = "NoExplicitBindings";

// Used to serialize the Use HLSL Block Offsets build setting.
static const char* XML_NODE_VULKAN_USE_HLSL_BLOCK_OFFSETS = "UseHlslBlockOffsets";

// Used to serialize the Use HLSL IO Mapping build setting.
static const char* XML_NODE_VULKAN_USE_HLSL_IO_MAPPING = "UseHlslIoMapping";

// Used to serialize the ICD location build setting.
static const char* XML_NODE_VULKAN_ICD_LOCATION = "ICDLocation";

// Used to serialize the glslang additional options build setting.
static const char* XML_NODE_VULKAN_GLSLANG_OPTIONS_LOCATION = "glslangOptions";

// Used to serialize the Enable validation layers build setting.
static const char* XML_NODE_VULKAN_ENABLE_VALIDATION_LAYER = "EnableValidationLayer";

// *******************************
// *** VULKAN-SPECIFIC - END ***
// *******************************

// *******************************
// *** GLOBAL SETTINGS - BEGIN ***
// *******************************

// Global settings root node.
static const char* XML_NODE_GLOBAL_LOG_FILE_GLOBAL_SETTINGS = "GlobalSettings";

// Log file location.
static const char* XML_NODE_GLOBAL_LOG_FILE_LOCATION = "LogFileLocation";

// Output directory.
static const char* XML_NODE_GLOBAL_OUTPUT_DIR = "OutputDirectory";

// Last selected directory.
static const char* XML_NODE_GLOBAL_LAST_SELECTED_DIRECTORY = "LastSelectedDirectory";

// Default API mode (introduced in RGA Data Model 2.1)
static const char* XML_NODE_GLOBAL_DEFAULT_API = "DefaultAPI";

// Indicates whether or not to (introduced in RGA Data Model 2.1)
static const char* XML_NODE_GLOBAL_PROMPT_FOR_API = "PromptForAPI";

// The recent programs root node.
static const char* XML_NODE_GLOBAL_RECENT_PROJECTS_ROOT = "RecentPrograms";

// The recent program root node.
static const char* XML_NODE_GLOBAL_RECENT_PROJECT_ROOT = "RecentProgram";

// The program path element.
static const char* XML_NODE_GLOBAL_RECENT_PROJECT_PATH = "Path";

// The program api type element.
static const char* XML_NODE_GLOBAL_RECENT_PROJECT_API = "API";

// The font root node.
static const char* XML_NODE_GLOBAL_FONT_FAMILY_ROOT = "Font";

// The font type element.
static const char* XML_NODE_GLOBAL_FONT_FAMILY_TYPE = "Type";

// The font size element.
static const char* XML_NODE_GLOBAL_FONT_SIZE = "Size";

// The default include file viewer.
static const char* XML_NODE_GLOBAL_INCLUDE_FILES_VIEWER = "IncludeFilesViewer";

// Disassembly view columns.
static const char* XML_NODE_GLOBAL_DISASSEMBLY_COLUMNS = "DisassemblyViewColumns";

// GUI elements.
static const char* XML_NODE_GLOBAL_GUI = "GUI";

// Gui layout.
static const char* XML_NODE_GLOBAL_GUI_LAYOUT = "Layout";

// Gui splitter config.
static const char* XML_NODE_GLOBAL_GUI_SPLITTER = "Splitter";

// Splitter config name.
static const char* XML_NODE_GLOBAL_GUI_SPLITTER_NAME = "SplitterName";

// Splitter config values.
static const char* XML_NODE_GLOBAL_GUI_SPLITTER_VALUES = "SplitterValues";

// Default build settings.
static const char* XML_NODE_GLOBAL_DEFAULT_BUILD_SETTINGS = "DefaultBuildSettings";

// Input file associations.
static const char* XML_NODE_GLOBAL_INPUT_FILE_EXT_GLSL = "InputFileExtGLSL";
static const char* XML_NODE_GLOBAL_INPUT_FILE_EXT_HLSL = "InputFileExtHLSL";
static const char* XML_NODE_GLOBAL_INPUT_FILE_EXT_SPV_TXT = "InputFileExtSpvTxt";
static const char* XML_NODE_GLOBAL_INPUT_FILE_EXT_SPV_BIN = "InputFileExtSpvBin";

// Default source language.
static const char* XML_NODE_GLOBAL_DEFAULT_SRC_LANG = "DefaultSrcLanguage";

// *****************************
// *** GLOBAL SETTINGS - END ***
// *****************************

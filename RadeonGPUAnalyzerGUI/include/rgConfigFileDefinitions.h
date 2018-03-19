#pragma once

// Local.
#include <RadeonGPUAnalyzerGUI/include/rgStringConstants.h>

// ***********************************************
// *** Configuration-file-related definitions. ***
// ***********************************************

// The data model version that RGA uses.
const std::string RGA_DATA_MODEL = "2.0";

// The XML declaration string.
const char* RGA_XML_DECLARATION = "xml version = \"1.0\" encoding = \"UTF-8\"";

// Data model version node.
const char* XML_NODE_DATA_MODEL_VERSION = "RGADataModelVersion";

// Program node.
const char* XML_NODE_PROJECT = "Program";

// API node.
const char* XML_NODE_API_NAME = "ProgramAPI";

// Program name node.
const char* XML_NODE_PROJECT_NAME = "ProgramName";

// Program clone node.
const char* XML_NODE_CLONE = "ProgramClone";

// Clone ID node.
const char* XML_NODE_CLONE_ID = "CloneID";

// Clone name node.
const char* XML_NODE_CLONE_NAME = "CloneName";

// Clone source files.
const char* XML_NODE_CLONE_SOURCE_FILES = "SourceFiles";

// File path.
const char* XML_NODE_FILE_PATH = "FullPath";

// Build settings node.
const char* XML_NODE_BUILD_SETTINGS = "BuildSettings";

// Target devices settings node.
const char* XML_NODE_TARGET_DEVICES = "TargetDevices";

// Predefined macros settings node.
const char* XML_NODE_PREDEFINED_MACROS = "PredefinedMacros";

// Additional include directories node.
const char* XML_NODE_ADDITIONAL_INCLUDE_DIRECTORIES = "AdditionalIncludeDirectories";

// Additional options provided by user.
const char* XML_NODE_ADDITIONAL_OPTIONS = "AdditionalOptions";

// A switch used to determine if project names will be generated or provided by the user.
const char* XML_NODE_USE_GENERATED_PROJECT_NAMES = "UseGeneratedProjectNames";

// Alternative compiler paths.
const char* XML_NODE_ALTERNATIVE_COMPILER_BIN_DIR = "AlternativeCompilerBin";
const char* XML_NODE_ALTERNATIVE_COMPILER_INC_DIR = "AlternativeCompilerInc";
const char* XML_NODE_ALTERNATIVE_COMPILER_LIB_DIR = "AlternativeCompilerLib";

// *******************************
// *** OPENCL-SPECIFIC - BEGIN ***
// *******************************

// OpenCL optimization level.
const char* XML_NODE_OPENCL_OPT_LEVEL = "OptimizationLevel";

// Treat double as single.
const char* XML_NODE_OPENCL_DOUBLE_AS_SINGLE = "TreatDoubleAsSingle";

// Denorms as zeros.
const char* XML_NODE_OPENCL_DENORMS_AS_ZEROS = "DenormsAsZeros";

// Strict aliasing.
const char* XML_NODE_OPENCL_STRICT_ALIASING = "StrictAliasing";

// Enable MAD.
const char* XML_NODE_OPENCL_ENABLE_MAD = "EnableMAD";

// Ignore zero signedness.
const char* XML_NODE_OPENCL_IGNORE_ZERO_SIGNEDNESS = "IgnoreZeroSignedness";

// Allow unsafe optimizations.
const char* XML_NODE_OPENCL_UNSAFE_OPT = "UnsafeOptimizations";

// No NaN nor Infinite.
const char* XML_NODE_OPENCL_NO_NAN_NOR_INF = "NoNanNorInfinite";

// Aggressive math optimizations.
const char* XML_NODE_OPENCL_AGGRESSIVE_MATH_OPT = "AggressiveMathOptimizations";

// Correctly round div / sqrt.
const char* XML_NODE_OPENCL_CORRECT_ROUND_DIV_SQRT = "CorrectlyRoundDivSqrt";

// *****************************
// *** OPENCL-SPECIFIC - END ***
// *****************************

// *******************************
// *** GLOBAL SETTINGS - BEGIN ***
// *******************************

// Global settings root node.
const char* XML_NODE_GLOBAL_LOG_FILE_GLOBAL_SETTINGS = "GlobalSettings";

// Log file location.
const char* XML_NODE_GLOBAL_LOG_FILE_LOCATION = "LogFileLocation";

// Output directory.
const char* XML_NODE_GLOBAL_OUTPUT_DIR = "OutputDirectory";

// Last selected directory.
const char* XML_NODE_GLOBAL_LAST_SELECTED_DIRECTORY = "LastSelectedDirectory";

// The recent programs root node.
const char* XML_NODE_GLOBAL_RECENT_PROJECTS_ROOT = "RecentPrograms";

// The program path element.
const char* XML_NODE_GLOBAL_RECENT_PROJECT_PATH = "Path";

// Disassembly view columns.
const char* XML_NODE_GLOBAL_DISASSEMBLY_COLUMNS = "DisassemblyViewColumns";

// GUI elements.
const char* XML_NODE_GLOBAL_GUI = "GUI";

// Gui layout.
const char* XML_NODE_GLOBAL_GUI_LAYOUT = "Layout";

// Gui splitter config.
const char* XML_NODE_GLOBAL_GUI_SPLITTER = "Splitter";

// Splitter config name.
const char* XML_NODE_GLOBAL_GUI_SPLITTER_NAME = "SplitterName";

// Splitter config values.
const char* XML_NODE_GLOBAL_GUI_SPLITTER_VALUES = "SplitterValues";

// Default build settings.
const char* XML_NODE_GLOBAL_DEFAULT_BUILD_SETTINGS = "DefaultBuildSettings";

// *****************************
// *** GLOBAL SETTINGS - END ***
// *****************************
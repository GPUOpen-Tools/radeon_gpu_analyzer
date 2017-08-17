#ifndef __kcCliStringConstantsh_h
    #define __kcCliStringConstantsh_h

    const char* const STR_RGA_VERSION = "1.2";
#ifdef RGA_BUILD_NUMBER
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
    const char* const STR_RGA_BUILD_NUM = STR(RGA_BUILD_NUMBER);
#else
    const char* const STR_RGA_BUILD_NUM = "0";
#endif
    const char* const STR_RGA_VERSION_PREFIX = "Radeon GPU Analyzer Version: ";
    const char* const STR_DRIVER_VERSION = "Driver version: ";
    const char* const STR_ERR_DRIVER_VERSION_EXTRACTION_FAILURE = "could not extract the driver version.";
    const char* const STR_ERR_INITIALIZATION_FAILURE = "Failed to initialize.";
    const char* const STR_ERR_OPENGL_VERSION_EXTRACTION_FAILURE = "Unable to extract the supported OpenGL version.\n";
    const char* const STR_TARGET_DETECTED = "Target GPU detected:";
    const char* const STR_FOUND_TARGETS = "Detected following target GPUs:";
    const char* const STR_FOUND_ADAPTERS = "Found the following supported display adapters installed on this system:";
    const char* const STR_DX_ADAPTERS_HELP_COMMON_TEXT = "This is only relevant if you have multiple display adapters installed on your system, and you would like RGA to use the driver which is associated with a non-primary display adapter.By default RGA will use the driver that is associated with the primary display adapter.";

    // Errors.
    const char* const STR_ERR_ERROR = "Error: ";
    const char* const STR_ERR_INVALID_GLSL_SHADER_TYPE = "Error: The Specified profile is invalid. Possible options: Vertex, Fragment, Compute, Geometry, TessControl, TessEval.";
    const char* const STR_ERR_UNSUPPORTED_GL_RT_VERSION = "Unsupported OpenGL runtime version. Please install Catalyst driver version 15.20 or above.";
    const char* const STR_ERR_NO_VALID_CMD_DETECTED = "Error: No valid command. Please run -h for available commands.";
    const char* const STR_ERR_COMMAND_NOT_SUPPORTED = "Error: the command is not supported for this mode.";
    const char* const STR_ERR_CANNOT_EXTRACT_SUPPORTED_DEVICE_LIST = "Error: Unable to extract the list of supported devices.";
    const char* const STR_ERR_MEMORY_ALLOC_FAILURE = "Error: memory allocation failure.";
    const char* const STR_ERR_RENDER_COMPUTE_MIX = "Error: cannot mix compute and non-compute shaders.";
    const char* const STR_ERR_CANNOT_FIND_SHADER_PREFIX = "Error: cannot find ";
    const char* const STR_ERR_CANNOT_FIND_SHADER_SUFFIX = " shader: ";
    const char* const STR_ERR_CANNOT_FIND_OUTPUT_DIR = "Error: output directory does not exist: ";
    const char* const STR_ERR_CANNOT_INVOKE_COMPILER = "Error: unable to invoke the compiler.";
    const char* const STR_ERR_CANNOT_GET_DEVICE_INFO = "Error: cannot get device info for: ";
    const char* const STR_ERR_CANNOT_READ_FILE = "Error: unable to read: ";
    const char* const STR_ERR_CANNOT_OPEN_FILE_FOR_WRITE_A = "Error: Unable to open ";
    const char* const STR_ERR_CANNOT_OPEN_FILE_FOR_WRITE_B =  " for write.";
    const char* const STR_ERR_CANNOT_LOCATE_LIVE_REG_ANALYZER = "Error: cannot locate the live register analyzer.";
    const char* const STR_ERR_CANNOT_LAUNCH_LIVE_REG_ANALYZER = "Error: cannot launch the live register analyzer.";
    const char* const STR_ERR_CANNOT_LAUNCH_CFG_ANALYZER = "Error: cannot launch the control graph generator.";
    const char* const STR_ERR_CANNOT_FIND_ISA_FILE = "Error: ISA file not found.";
    const char* const STR_ERR_CANNOT_PERFORM_LIVE_REG_ANALYSIS = "Error: failed to perform live register analysis.";
    const char* const STR_ERR_CANNOT_DISASSEMBLE_ISA = "Error: failed to disassemble binary output and produce textual ISA code";
    const char* const STR_ERR_CANNOT_DISASSEMBLE_AMD_IL = "Error: failed to disassemble binary output and produce textual IL code";
    const char* const STR_ERR_CANNOT_EXTRACT_BINARIES = "Error: failed to extract binaries";
    const char* const STR_ERR_CANNOT_EXTRACT_META_DATA = "Error: failed to extract meta-data";
    const char* const STR_ERR_CANNOT_EXTRACT_DEBUG_IL = "Error: failed to extract debug IL section";
    const char* const STR_ERR_GLSL_MODE_DEPRECATED = "Error: GLSL mode is no longer supported. Please use OpenGL mode to build OpenGL programs.";
    const char* const STR_ERR_CANNOT_EXTRACT_OPENGL_VERSION = "Error: unable to extract the OpenGL version.";
    const char* const STR_ERR_INVALID_INPUT_TYPE = "Error: invalid input type.";
    const char* const STR_ERR_TARGET_IS_NOT_SUPPORTED = " offline compilation for the detected target GPU is not supported: ";
    const char* const STR_ERR_COULD_NOT_DETECT_TARGET = "Error: could not detect target GPU -> ";
    const char* const STR_ERR_AMBIGUOUS_TARGET = "Error: ambiguous target GPU name -> ";
    const char* const STR_ERR_NO_KERNELS_FOR_ANALYSIS = "Error: No kernels provided for analysis.";
    const char* const STR_ERR_LIST_ADAPTERS_FAILED = "Error: failed to get the list of supported display adapters installed on this system.";
    const char* const STR_ERR_SET_ADAPTER_FAILED = "Error: failed to set display adapter with provided ID.";
    const char* const STR_ERR_PARSE_DX_SHADER_MODEL_FAILED = "Error: incorrect DX shader model provided.";
    const char* const STR_ERR_UNSUPPORTED_DX_SHADER_MODEL_1 = "Error: unsupported Shader Model detected: ";
    const char* const STR_ERR_UNSUPPORTED_DX_SHADER_MODEL_2 = "RGA supports Shader Model ";
    const char* const STR_ERR_UNSUPPORTED_DX_SHADER_MODEL_3 = "and below.";
    const char* const STR_ERR_FAILED_ADJUST_FILE_NAMES = "Error: failed to construct some of the output file names.";
    const char* const STR_ERR_NO_INPUT_FILE = "Error: no input file provided.";
    const char* const STR_ERR_MIXED_INPUT_FILES = "Error: cannot mix stage-specific input files (--vert, --tesc, --tese, --geom, --frag, --comp) with a stage-less SPIR-V input file.";
    const char* const STR_ERR_FAILED_OUTPUT_FILE_VERIFICATION = "Error: failed to generate one or more output files.";

    // Warnings.
    #define STR_WRN_DX_MIN_SUPPORTED_VERSION "Warning: AMD DirectX driver supports DX10 and above."
    #define STR_WRN_CL_SUPPRESS_WIHTOUT_BINARY "Warning: --suppress option is valid only with output binary."

    // Environment variables.
    #define STR_OCL_ENV_VAR_GPU_FORCE_64BIT_PTR_NAME  L"GPU_FORCE_64BIT_PTR"
    #define STR_OCL_ENV_VAR_GPU_FORCE_64BIT_PTR_VALUE L"1"

    // CSV file.
    #define STR_CSV_HEADER_DEVICE            "DEVICE"
    #define STR_CSV_HEADER_SCRATCH_REGS      "SCRATCH_REGS"
    #define STR_CSV_HEADER_THREADS_PER_WG    "THREADS_PER_WORKGROUP"
    #define STR_CSV_HEADER_WAVEFRONT_SIZE    "WAVEFRONT_SIZE"
    #define STR_CSV_HEADER_LDS_BYTES_MAX     "AVAILABLE_LDS_BYTES"
    #define STR_CSV_HEADER_LDS_BYTES_ACTUAL  "USED_LDS_BYTES"
    #define STR_CSV_HEADER_SGPR_AVAILABLE    "AVAILABLE_SGPRs"
    #define STR_CSV_HEADER_SGPR_USED         "USED_SGPRs"
    #define STR_CSV_HEADER_VGPR_AVAILABLE    "AVAILABLE_VGPRs"
    #define STR_CSV_HEADER_VGPR_USED         "USED_VGPRs"
    #define STR_CSV_HEADER_CL_WORKGROUP_DIM_X "CL_WORKGROUP_X_DIMENSION"
    #define STR_CSV_HEADER_CL_WORKGROUP_DIM_Y "CL_WORKGROUP_Y_DIMENSION"
    #define STR_CSV_HEADER_CL_WORKGROUP_DIM_Z "CL_WORKGROUP_Z_DIMENSION"
    #define STR_CSV_HEADER_ISA_SIZE_BYTES     "ISA_SIZE"

    // Build output.
    // Status strings.
    #define KA_CLI_STR_STATUS_SUCCESS "... succeeded."
    #define KA_CLI_STR_STATUS_FAILURE "... failed."
    #define KA_CLI_STR_ABORTING "Aborting."
    #define KA_CLI_STR_COMPILING "Building for "
    #define KA_CLI_STR_EXTRACTING_ISA "Extracting ISA for "
    #define KA_CLI_STR_EXTRACTING_BIN "Extracting Binary for "
    #define KA_CLI_STR_EXTRACTING_STATISTICS "Extracting statistics for "
    #define KA_CLI_STR_EXTRACTING_AMDIL "Extracting AMD IL code for "
    #define KA_CLI_STR_D3D_ASM_GENERATION_SUCCESS "DX ASM code generation succeeded."
    #define KA_CLI_STR_D3D_ASM_GENERATION_FAILURE "DX ASM code generation failed."
    #define KA_CLI_STR_ERR_GLSL_COMPILATION_NOT_SUPPORTED_A "Offline GLSL compilation for "
    #define KA_CLI_STR_ERR_GLSL_COMPILATION_NOT_SUPPORTED_B " is not supported."
    #define KA_CLI_STR_ERR_D3D_COMPILATION_NOT_SUPPORTED_FOR_DEVICE "D3D ASM offline compilation to ISA is not supported for "
    #define KA_CLI_STR_TMP_TXT_FILE_WILDCARD L"cxlTempFile_*.txt"
#endif // __kcCliStringConstantsh_h

// Shaders and pipeline stages.
#define KA_CLI_STR_VERTEX_SHADER "vertex"
#define KA_CLI_STR_TESS_CTRL_SHADER "tessellation control"
#define KA_CLI_STR_TESS_EVAL_SHADER "tessellation evaluation"
#define KA_CLI_STR_GEOMETRY_SHADER "geometry"
#define KA_CLI_STR_FRAGMENT_SHADER "fragment"
#define KA_CLI_STR_COMPUTE_SHADER "compute"

// Stage abbreviations for output file names.
#define KA_CLI_STR_VERTEX_ABBREVIATION "vert"
#define KA_CLI_STR_TESS_CTRL_ABBREVIATION "tesc"
#define KA_CLI_STR_TESS_EVAL_ABBREVIATION "tese"
#define KA_CLI_STR_GEOMETRY_ABBREVIATION "geom"
#define KA_CLI_STR_FRAGMENT_ABBREVIATION "frag"
#define KA_CLI_STR_COMPUTE_ABBREVIATION "comp"

// Default file extensions.
#define KC_STR_DEFAULT_ISA_SUFFIX "amdisa"
#define KC_STR_DEFAULT_AMD_IL_SUFFIX "amdiil"
#define KC_STR_DEFAULT_LIVE_REG_ANALYSIS_SUFFIX "txt"
#define KC_STR_DEFAULT_CFG_SUFFIX "dot"
#define KC_STR_DEFAULT_BIN_SUFFIX "bin"
#define KC_STR_DEFAULT_METADATA_SUFFIX "amdMetadata"
#define KC_STR_DEFAULT_DEBUG_IL_SUFFIX "amdDebugil"
#define KC_STR_DEFAULT_DXASM_SUFFIX "dxasm"
#define KC_STR_DEFAULT_STATISTICS_SUFFIX "csv"

// Default file names.
#define KC_STR_DEFAULT_LIVEREG_OUTPUT_FILE_NAME "livereg"

// Front-End strings
// Family names:
#define KA_STR_familyNameSICards " (HD7000 / HD8000 series)"
#define KA_STR_familyNameCICards " (HD8000 / Rx 200 / 300 series)"
#define KA_STR_familyNameVICards " (Rx 200 / 300 / Fury series)"
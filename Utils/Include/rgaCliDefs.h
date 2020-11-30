#pragma once

// *** OpenCL-specific - BEGIN ***

static const char* CLI_OPT_CL_AGGRESSIVE_OPTIMIZATIONS = "-cl-fast-relaxed-math";
static const char* CLI_OPT_CL_CORRECT_ROUND_DIV_SQRT = "-cl-fp32-correctly-rounded-divide-sqrt";
static const char* CLI_OPT_CL_IS_NAN_OR_INIFINITE = "-cl-finite-math-only";
static const char* CLI_OPT_CL_UNSAFE_OPTIMIZATIONS = "-cl-unsafe-math-optimizations";
static const char* CLI_OPT_CL_IGNORE_ZERO_SIGNEDNESS = "-cl-no-signed-zeros";
static const char* CLI_OPT_CL_ENABLE_MAD = "-cl-mad-enable";
static const char* CLI_OPT_CL_STRICT_ALIASING = "-cl-strict-aliasing";
static const char* CLI_OPT_CL_DENORMS_AS_ZEROES = "-cl-denorms-are-zero";
static const char* CLI_OPT_CL_TREAT_DOUBLE_AS_SINGLE = "-cl-single-precision-constant";
static const char* CLI_OPT_CL_TREAT_WARNING_AS_ERRORS = "-Werror";
static const char* CLI_OPT_CL_DISABLE_ALL_WARNING = "-w";
static const char* CLI_OPT_CL_OPTION = "--OpenCLoption";
static const char* CLI_OPT_CL_OPTMIZATION_LEVEL_0 = "--O0";
static const char* CLI_OPT_CL_OPTMIZATION_LEVEL_1 = "--O1";
static const char* CLI_OPT_CL_OPTMIZATION_LEVEL_2 = "--O2";
static const char* CLI_OPT_CL_OPTMIZATION_LEVEL_3 = "--O3";

// *** OpenCL-specific - END ***

// *** Vulkan-specific - BEGIN ***

static const char* CLI_OPT_VULKAN_OPTION = "--Vulkanoption";
static const char* CLI_OPT_VULKAN_GENERATE_DEBUG_INFORMATION = "-g";
static const char* CLI_OPT_VULKAN_NO_EXPLICIT_BINDINGS = "--auto-map-bindings";
static const char* CLI_OPT_VULKAN_HLSL_BLOCK_OFFSETS = "--hlsl-offsets";
static const char* CLI_OPT_VULKAN_HLSL_IOMAP = "--hlsl-iomap";
static const char* CLI_OPT_VULKAN_VALIDATION = "--validation";
static const char* CLI_OPT_VULKAN_ICD_LOCATION = "--icd";
static const char* CLI_OPT_VULKAN_GLSLANG_OPTIONS = "--glslang-opt";
static const char* CLI_OPT_ICD_DESCRIPTION = "Full path to a Vulkan ICD. If given, RGA would load the given ICD explicitly instead of the Vulkan loader.";
static const char* CLI_OPT_GLSLANG_OPT_DESCRIPTION_A = "Additional options to be passed to glslang for Vulkan front-end compilation (for example, \"--target-env vulkan1.1 --suppress-warnings\").";
static const char* CLI_OPT_GLSLANG_OPT_DESCRIPTION_B = "It is recommended to wrap the argument to this option with '@' characters in the case where glslang and rga options overlap, like -c or -w. For example, --glslang-opt \"@-c -w@\".";
static const char* CLI_OPT_VK_LOADER_DEBUG_DESCRIPTION = "Value for the VK_LOADER_DEBUG environment variable (all, error, info, warn, debug). Use this option to log Vulkan loader messages to the console.";
// A token to wrap --glslang-opt argument to avoid ambiguity between rga and glslang options.
static const char CLI_OPT_GLSLANG_TOKEN = '@';

// *** Vulkan-specific - END ***

// *** CLI GENERIC COMMANDS - BEGIN ***

static const char* CLI_OPT_LOG = "--log";
static const char* CLI_OPT_INPUT_TYPE = "-s";
static const char* CLI_OPT_VULKAN_SPV_DIS = "--disassemble-spv";
static const char* CLI_OPT_SESSION_METADATA = "--session-metadata";
static const char* CLI_OPT_ISA = "--isa";
static const char* CLI_OPT_PARSE_ISA = "--parse-isa";
static const char* CLI_OPT_LINE_NUMBERS = "--line-numbers";
static const char* CLI_OPT_STATISTICS = "--analysis";
static const char* CLI_OPT_BINARY = "-b";
static const char* CLI_OPT_ASIC = "--asic";
static const char* CLI_OPT_PSO = "--pso";
static const char* CLI_OPT_ADDITIONAL_INCLUDE_PATH = "-I";
static const char* CLI_OPT_PREPROCESSOR_DIRECTIVE = "-D";
static const char* CLI_OPT_LIST_KERNELS = "--list-kernels";
static const char* CLI_OPT_VERSION_INFO = "--version-info";
static const char* CLI_OPT_COMPILER_BIN_DIR = "--compiler-bin";
static const char* CLI_OPT_COMPILER_INC_DIR = "--compiler-inc";
static const char* CLI_OPT_COMPILER_LIB_DIR = "--compiler-lib";
static const char* CLI_NO_BINARY_FILE_EXTENSION_SWITCH = "--no-suffix-bin";
static const char* CLI_OPT_SPVAS_TEXT_FILE = "-spvas";

// *** CLI COMMANDS - END ***

// *** COMMAND DESCRIPTION STRINGS - BEGIN ***

static const char* CLI_DESC_ALTERNATIVE_ROCM_BIN_FOLDER = "Path to alternative compiler's binaries folder. The following executables are expected "
                                                          "to be in this folder: clang, lld, llvm-objdump, llvm-readobj.";
static const char* CLI_DESC_ALTERNATIVE_ROCM_INC_FOLDER = "Path to alternative compiler's headers folder. The specified folder is expected"
                                                          " to contain opencl-c.h header file.";
static const char* CLI_DESC_ALTERNATIVE_ROCM_LIB_FOLDER = "Path to alternative compiler's OpenCL libraries folder. The following bitcode files are expected to be in the specified folder: \n"
                                                          "irif.amdgcn.bc,  oclc_correctly_rounded_sqrt_off.amdgcn.bc, oclc_daz_opt_off.amdgcn.bc, \n"
                                                          "oclc_finite_only_off.amdgcn.bc, oclc_isa_version_900.amdgcn.bc, oclc_unsafe_math_on.amdgcn.bc, \n"
                                                          "opencl.amdgcn.bc, ockl.amdgcn.bc, oclc_correctly_rounded_sqrt_on.amdgcn.bc, oclc_daz_opt_on.amdgcn.bc, \n"
                                                          "oclc_finite_only_on.amdgcn.bc, oclc_unsafe_math_off.amdgcn.bc, ocml.amdgcn.bc";
static const char* CLI_DESC_ALTERNATIVE_VK_BIN_FOLDER = "Path to alternative compiler's binaries folder. The following executables are expected\n"
                                                        "to be in this folder: glslangValidator, spirv-as, spirv-dis. If given, this package would be\n"
                                                        "used instead of the glslang package that is bundled with RGA to compile GLSL to SPIR-V,\n"
                                                        "disassemble SPIR-V binaries, reassemble SPIR-V binaries, etc.";

// *** COMMAND DESCRIPTION STRINGS - END ***

// *** HW FUNCTIONAL UNIT NAMES - BEGIN ***

#define FUNC_UNIT_SALU                  "Scalar ALU"
#define FUNC_UNIT_SMEM                  "Scalar Memory"
#define FUNC_UNIT_VMEM                  "Vector Memory"
#define FUNC_UNIT_VALU                  "Vector ALU"
#define FUNC_UNIT_LDS                   "LDS"
#define FUNC_UNIT_GDS_EXPORT            "GDS/Export"
#define FUNC_UNIT_INTERNAL_FLOW         "Flow Control"
#define FUNC_UNIT_BRANCH                "Branch"
#define FUNC_UNIT_UNKNOWN               "Unknown"

// *** HW FUNCTIONAL UNIT NAMES - END ***

// Vulkan adapter list tags.
static const std::string CLI_VK_BACKEND_STR_ADAPTER         = "Adapter ";
static const std::string CLI_VK_BACKEND_STR_ADAPTER_OFFSET  = "    ";
static const std::string CLI_VK_BACKEND_STR_ADAPTER_NAME    = "Name: ";
static const std::string CLI_VK_BACKEND_STR_ADAPTER_DRIVER  = "Vulkan driver version: ";
static const std::string CLI_VK_BACKEND_STR_ADAPTER_VULKAN  = "Supported Vulkan API version: ";
static const std::string CLI_VK_BACKEND_STR_UNKNOWN_DRIVER  = "Unknown.";

// Other Vulkan live driver strings.
static const char* KC_STR_VK_VALIDATION_INFO_STDOUT = "<stdout>";

#pragma once

// *** OpenCL-specific - BEGIN ***

static const char* kStrCliOptClAggressiveOptimizations = "-cl-fast-relaxed-math";
static const char* kStrCliOptClCorrectRoundDivSqrt = "-cl-fp32-correctly-rounded-divide-sqrt";
static const char* kStrCliOptClIsNanOrInfinite = "-cl-finite-math-only";
static const char* kStrCliOptClUnsafeOptimizations = "-cl-unsafe-math-optimizations";
static const char* kStrCliOptClIgnoreZeroSignedness = "-cl-no-signed-zeros";
static const char* kStrCliOptClEnableMad = "-cl-mad-enable";
static const char* kStrCliOptClStrictAliasing = "-cl-strict-aliasing";
static const char* kStrCliOptClDenormsAsZeroes = "-cl-denorms-are-zero";
static const char* kStrCliOptClTreatDoubleAsSingle = "-cl-single-precision-constant";
static const char* kStrCliOptClTreatWarningsAsErrors = "-Werror";
static const char* kStrCliOptClDisableAllWarning = "-w";
static const char* kStrCliOptClOption = "--OpenCLoption";
static const char* kStrCliOptClOptimizationLevel0 = "--O0";
static const char* kStrCliOptClOptimizationLevel1 = "--O1";
static const char* kStrCliOptClOptimizationLevel2 = "--O2";
static const char* kStrCliOptClOptimizationLevel3 = "--O3";

// *** OpenCL-specific - END ***

// *** Vulkan-specific - BEGIN ***

static const char* kStrCliOptVulkanOption = "--Vulkanoption";
static const char* kStrCliOptVulkanGenerateDebugInformation = "-g";
static const char* kStrCliOptVulkanNoExplicitBindings = "--auto-map-bindings";
static const char* kStrCliOptVulkanHlslBlockOffsets = "--hlsl-offsets";
static const char* kStrCliOptVulkanHlslIomap = "--hlsl-iomap";
static const char* kStrCliOptVulkanValidation = "--validation";
static const char* kStrCliOptVulkanIcdLocation = "--icd";
static const char* kStrCliOptVulkanGlslangOptions = "--glslang-opt";
static const char* kStrCliOptIcdDescription = "Full path to a Vulkan ICD. If given, RGA would load the given ICD explicitly instead of the Vulkan loader.";
static const char* kStrCliOptGlslangOptDescriptionA = "Additional options to be passed to glslang for Vulkan front-end compilation (for example, \"--target-env vulkan1.1 --suppress-warnings\").";
static const char* kStrCliOptGlslangOptDescriptionB = "It is recommended to wrap the argument to this option with '@' characters in the case where glslang and rga options overlap, like -c or -w. For example, --glslang-opt \"@-c -w@\".";
static const char* kStrCliOptVkLoaderDebugDescription = "Value for the VK_LOADER_DEBUG environment variable (all, error, info, warn, debug). Use this option to log Vulkan loader messages to the console.";
// A token to wrap --glslang-opt argument to avoid ambiguity between rga and glslang options.
static const char kStrCliOptGlslangToken = '@';

// *** Vulkan-specific - END ***

// *** CLI GENERIC COMMANDS - BEGIN ***

static const char* kStrCliOptLog = "--log";
static const char* kStrCliOptInputType = "-s";
static const char* kStrCliOptVulkanSpvDis = "--disassemble-spv";
static const char* kStrCliOptSessionMetadata = "--session-metadata";
static const char* kStrCliOptIsa = "--isa";
static const char* kStrCliOptParseIsa = "--parse-isa";
static const char* kStrCliOptLivereg  = "--livereg";
static const char* kStrCliOptLineNumbers = "--line-numbers";
static const char* kStrCliOptStatistics = "--analysis";
static const char* kStrCliOptBinary = "-b";
static const char* kStrCliOptAsic = "--asic";
static const char* kStrCliOptPso = "--pso";
static const char* kStrCliOptAdditionalIncludePath = "-I";
static const char* kStrCliOptPreprocessorDirective = "-D";
static const char* kStrCliOptListKernels = "--list-kernels";
static const char* kStrCliOptVersionInfo = "--version-info";
static const char* kStrCliOptCompilerBinDir = "--compiler-bin";
static const char* kStrCliOptCompilerIncDir = "--compiler-inc";
static const char* kStrCliOptCompilerLibDir = "--compiler-lib";
static const char* kStrCliNoBinaryFileExtensionSwitch = "--no-suffix-bin";
static const char* kStrCliOptSpvasTextFile = "-spvas";

// *** CLI COMMANDS - END ***

// *** COMMAND DESCRIPTION STRINGS - BEGIN ***

static const char* kStrCliDescAlternativeLightningCompilerBinFolder = "Path to alternative compiler's binaries folder. The following executables are expected "
                                                          "to be in this folder: clang, lld, llvm-objdump, llvm-readobj.";
static const char* kStrCliDescAlternativeLightningCompilerIncFolder = "Path to alternative compiler's headers folder. The specified folder is expected"
                                                          " to contain opencl-c.h header file.";
static const char* kStrCliDescAlternativeLightningCompilerLibFolder =
    "Path to alternative compiler's OpenCL libraries folder. The following bitcode files are expected to be in the specified folder: "
    "hip.bc, "
    "ockl.bc, "
    "oclc_abi_version_400.bc, "
    "oclc_abi_version_500.bc, "
    "oclc_correctly_rounded_sqrt_off.bc, "
    "oclc_correctly_rounded_sqrt_on.bc, "
    "oclc_daz_opt_off.bc, "
    "oclc_daz_opt_on.bc, "
    "oclc_finite_only_off.bc, "
    "oclc_finite_only_on.bc, "
    "oclc_isa_version_1010.bc, "
    "oclc_isa_version_1100.bc, "
    "oclc_isa_version_1101.bc, "
    "oclc_isa_version_1102.bc, "
    "oclc_isa_version_1103.bc, "
    "oclc_isa_version_1011.bc, "
    "oclc_isa_version_1012.bc, "
    "oclc_isa_version_1030.bc, "
    "oclc_isa_version_1031.bc, "
    "oclc_isa_version_1032.bc, "
    "oclc_isa_version_900.bc, "
    "oclc_isa_version_902.bc, "
    "oclc_isa_version_904.bc, "
    "oclc_isa_version_906.bc, "
    "oclc_isa_version_908.bc, "
    "oclc_isa_version_909.bc, "
    "oclc_isa_version_90a.bc, "
    "oclc_isa_version_90c.bc, "
    "oclc_unsafe_math_off.bc, "
    "oclc_unsafe_math_on.bc, "
    "oclc_isa_version_1034.bc, "
    "oclc_wavefrontsize64_off.bc, "
    "oclc_wavefrontsize64_on.bc, "
    "ocml.bc, opencl.bc";
static const char* kStrCliDescAlternativeVkBinFolder = "Path to alternative compiler's binaries folder. The following executables are expected"
                                                        "to be in this folder: glslangValidator, spirv-as, spirv-dis. If given, this package would be"
                                                        "used instead of the glslang package that is bundled with RGA to compile GLSL to SPIR-V,"
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
static const std::string kStrCliVkBackendStrAdapter         = "Adapter ";
static const std::string kStrCliVkBackendStrAdapterOffset  = "    ";
static const std::string kStrCliVkBackendStrAdapterName    = "Name: ";
static const std::string kStrCliVkBackendStrAdapterDriver  = "Vulkan driver version: ";
static const std::string kStrCliVkBackendStrAdapterVulkan  = "Supported Vulkan API version: ";
static const std::string kStrCliVkBackendStrUnknownDriver  = "Unknown.";

// Other Vulkan live driver strings.
static const char* kStrCliVkValidationInfoStdout = "<stdout>";

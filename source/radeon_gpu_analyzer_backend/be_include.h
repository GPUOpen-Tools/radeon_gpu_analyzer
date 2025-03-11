//======================================================================
// Copyright 2020-2023 Advanced Micro Devices, Inc. All rights reserved.
//======================================================================

#ifndef RGA_RADEONGPUANALYZERBACKEND_SRC_BE_INCLUDE_H_
#define RGA_RADEONGPUANALYZERBACKEND_SRC_BE_INCLUDE_H_

#if defined(__linux__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wignored-qualifiers"
#endif
#if defined(__linux__)
    #pragma GCC diagnostic pop
#endif

#include <vector>
#include <algorithm>
#include <cctype>
#include <locale>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <set>
#include <map>
#include <cstdio>

#include "external/dx11/AsicReg/devices.h"

// Logging callback type.
typedef void(*LoggingCallBackFuncP)(const std::string&);

namespace beKA
{

static const uint64_t kCalValue64Na    = (uint64_t)-1;
static const uint64_t kCalValue64Error = (uint64_t)-2;

// Supported source language.
enum RgaMode
{
    kModeInvalid = -1,
    kModeNone = 0,
    kModeOpenclOffline,     // OpenCL offline mode. Supports OpenCL input files.
    kModeOpengl,            // OpenGL mode. Supports GLSL input files.
    kModeVulkan,            // Live-driver Vulkan mode (leverages AMD Graphics driver). Supports GLSL, HLSL and SPIR-V input files.
    kModeVkOffline,         // Offline Vulkan mode. Supports GLSL input files.
    kModeVkOfflineSpv,      // Offline Vulkan mode. Supports SPIR-V binary input files.
    kModeVkOfflineSpvTxt,   // Offline Vulkan mode. Supports SPIR-V text input files.
    kModeDx11,              // DirectX 11 mode.
    kModeDx12,              // DirectX 12 mode.
    kModeDxr,               // DXR mode.
    kModeAmdil,             // AMDIL mode. Supports AMDIL input files.
    kModeBinary             // Binary mode. Supports Code objects as input files.
};

enum BuiltProgramKind
{
    kProgramTypeInvalid = 0,
    kProgramTypeOpencl,
    kProgramTypeDx11,
};

enum beStatus
{
    kBeStatusInvalid = 0,
    kBeStatusSuccess,
    kBeStatusAmdxxModuleNotLoaded,
    kBeStatusAmdDx11GsaCompileShaderFailed,
    kBeStatusBackendNotInitialized,
    kBeStatusOpenclBuildProgramException,
    kBeStatusD3dCompilerModuleNotLoaded,
    kBeStatusD3dCompileFailed,
    kBeStatusNoBinaryForDevice,
    kBeStatusUnknownDevice,
    kBeStatusNoDeviceFound,
    kBeStatusNoIlForDevice,
    kBeStatusNoIsaForDevice,
    kBeStatusNoStatisticsForDevice,
    kBeStatusNoOpenclAmdPlatform,
    kBeStatusNoDebugIlForDevice,
    kBeStatusNoMetadataForDevice,
    kBeStatusOpenclModuleNotLoaded,
    kBeStatusOpenclModuleNotSupported,
    kBeStatusAclCompileFailed,
    kBeStatusAclBinaryFailed,
    kBeStatusOpenclCreateContextFromTypeFailed,
    kBeStatusOpenclCreateProgramWithSourceFailed,
    kBeStatusOpenclGetContextInfoFailed,
    kBeStatusOpenclGetDeviceInfoFailed,
    kBeStatusOpenclGetPlatformIDsFailed,
    kBeStatusOpenclGetPlatformInfoFailed,
    kBeStatusOpenclGetProgramInfoFailed,
    kBeStatusOpenclBuildProgramWrapperFailed,
    kBeStatusCreateBlobFromFileFailed,
    kBeStatusElfStatisticsSectionMissing,
    kBeStatusWrongKernelName,
    kBeStatusOpenglVirtualContextLaunchFailed,
    kBeStatusOpenglBuildError,
    kBeStatusOpenglUnknownHwFamily,
    kBeStatusVulkanAmdllpcLaunchFailure,
    kBeStatusGlcLaunchFailure,
    kBeStatusVulkanAmdllpcCompilationFailure,
    kBeStatusGlcCompilationFailure,
    kBeStatusVulkanNoInputFile,
    kBeStatusVulkanEmptyInputFile,
    kBeStatusVulkanGlslangLaunchFailed,
    kBeStatusVulkanFrontendCompileFailed,
    kBeStatusVulkanBackendLaunchFailed,
    kBeStatusVulkanAmdgpudisLaunchFailed,
    kBeStatusVulkanBackendCompileFailed,
    kBeStatusVulkanSetEnvVarFailed,
    kBeStatusVulkanSpvToolLaunchFailed,
    kBeStatusVulkanSpvAsmFailed,
    kBeStatusVulkanSpvDisasmFailed,
    kBeStatusVulkanParseStatsFailed,
    kBeStatusVulkanConstructOutFileNameFailed,
    kBeStatusVulkanFailedExtractValidationInfo,
    kBeStatusVulkanPreprocessFailed,
    KBeStatusVulkanMixedInputFiles,
    kBeStatusComputeCodeObjMetaDataSuccess,
    kBeStatusGraphicsCodeObjMetaDataSuccess,
    kBeStatusRayTracingCodeObjMetaDataSuccess,
    kBeStatusLightningCompilerLaunchFailed,
    kBeStatusLightningCompilerTimeOut,
    kBeStatusLightningCompilerGeneratedError,
    kBeStatusLightningObjDumpLaunchFailed,
    kBeStatusLightningDisassembleFailed,
    kBeStatusLightningBuilderLightningFailed,
    kBeStatusLightningParseCodeObjMDFailed,
    kBeStatusLightningGetISASizeFailed,
    kBeStatusLightningParseCompilerVersionFailed,
    kBeStatusLightningConstructISAFileNameFailed,
    kBeStatusLightningExtractMetadataFailed,
    kBeStatusLightningExtractCodePropsFailed,
    kBeStatusLightningStoreStatsFailed,
    kBeStatusLightningSplitIsaFailed,
    kBeStatusLightningExtractKernelNamesFailed,
    kBeStatusLightningGetKernelCodeSizeFailed,
    kBeStatusBinaryInvalidInput,
    kBeStatusCodeObjMdParsingFailed,
    kBeStatusWriteToFileFailed,
    kBeStatusFailedOutputVerification,
    kBeStatusShaeCannotLocateAnalyzer,
    kBeStatusShaeIsaFileNotFound,
    kBeStatusShaeFailedToLaunch,
    kBeStatusInferenceIsaFileNotFound,
    kBeStatusInferenceFailedToLaunch,
    kBeStatusdxDriverLaunchFailure,
    kBeStatusdx12BackendLaunchFailure,
    kBeStatusdx12CompileFailure,
    kBeStatusdx12OutputMetadataMissing,
    kBeStatusUnknownInputLang,
    kBeStatusUnknownObjDumpOperation,
    kBeStatusNoISAFileNameProvided,
    kBeStatusNoOutputFileGenerated,
    kBeStatusParseIsaToCsvFailed,
    kBeStatusConstructIsaFileNameFailed,
    kBeStatusConstructParsedIsaFileNameFailed,
    kBeStatusWriteParsedIsaFileFailed,
    kBeStatusDx12AlternativeDriverMissing,
    kBeStatusCannotParseDisassemblyShaderStage,
    kBeStatusCannotParseDisassemblyGeneral,
    kBeStatusDxcCannotAutoGenerateRootSignature,
    kBeStatusDxcCannotAutoGenerateVertexShader,
    kBeStatusDxcCannotAutoGeneratePixelShader,
    kBeStatusDxcCannotAutoGenerateGpso,
    kBeStatusDxcCheckHrFailed,
    kBeStatusGeneralFailed
};

// Options that make sense for any of OpenCL, DX, GL.
struct CompileOptions
{
    // Source language.
    RgaMode  mode;

    // Optimization level.
    // -1 : Use compiler default value (not specified by user).
    //  0 : Disable optimization.
    //  1 : Minimal optimization.
    //  2 : Optimize for speed (usually, compiler default level).
    //  3 : Maximum optimization (may significantly slow down compilation).
    int   optimization_level    = -1;

    // Indicates whether IL dump is required or not.
    bool  should_dump_il      = false;

    // Indicates whether line number info is required or not.
    bool  line_numbers = false;
};

// Object to customize binary output strings.
struct BinaryOptions
{
    // List of sections to suppress.
    std::vector<std::string> suppress_section;
};

// Data collected in analyzing a kernel/function.
struct AnalysisData
{
    AnalysisData() :
        scratch_memory_used(0),
        wavefront_count_per_simd(0),
        wavefront_size(0),
        num_gprs_available(0),
        num_gprs_used(0),
        lds_size_available(0),
        lds_size_used(0),
        stack_size_available(0),
        stack_size_used(0),
        num_sgprs_available(0),
        num_sgprs_used(0),
        num_sgpr_spills(0),
        num_vgprs_available(0),
        num_vgprs_used(0),
        num_vgpr_spills(0),
        num_threads_per_group_total(0),
        num_threads_per_group_x(0),
        num_threads_per_group_y(0),
        num_threads_per_group_z(0),
        num_alu_instructions(0),
        num_instructions_control_flow(0),
        num_instructions_fetch(0),
        isa_size(0)
    {
    }

    // Information directly from the compiled Kernel (passed back from SC).
    // We used to use the CAL interface to get this as a block.
    // We now use OpenCL extensions to get the same info.
    uint64_t scratch_memory_used;              // Size of statically allocated scratch memory
    uint64_t wavefront_count_per_simd;         // Number of wavefronts per SIMD             CL_KERNELINFO_WAVEFRONT_PER_SIMD
    uint64_t wavefront_size;                   // number of threads per wavefront.          CL_KERNELINFO_WAVEFRONT_SIZE
    uint64_t num_gprs_available;               // number of GPRs available to the program   CL_KERNELINFO_AVAILABLE_GPRS
    uint64_t num_gprs_used;                    // number of GPRs used by the program        CL_KERNELINFO_USED_GPRS
    uint64_t lds_size_available;               // LDS size available to the program         CL_KERNELINFO_AVAILABLE_LDS_SIZE
    uint64_t lds_size_used;                    // LDS size used by the program              CL_KERNELINFO_USED_LDS_SIZE
    uint64_t stack_size_available;             // stack size available to the program        CL_KERNELINFO_AVAILABLE_STACK_SIZE
    uint64_t stack_size_used;                  // stack size use by the program             CL_KERNELINFO_USED_STACK_SIZE
    uint64_t num_sgprs_available;               // number of SGPRs available to the program  CL_KERNELINFO_AVAILABLE_SGPRS
    uint64_t num_sgprs_used;                    // number of SGPRs used by the program       CL_KERNELINFO_USED_SGPRS
    uint64_t num_sgpr_spills;                  // number of spills/fills of SGPRs to/from memory
    uint64_t num_vgprs_available;               // number of VGPRs available to the program  CL_KERNELINFO_AVAILABLE_VGPRS
    uint64_t num_vgprs_used;                    // number of VGPRs used by the program       CL_KERNELINFO_USED_VGPRS
    uint64_t num_vgpr_spills;                  // number of spills/fills of VGPRs to/from memory

    uint64_t num_threads_per_group_total;      // Flattened Number of threads per group
    uint64_t num_threads_per_group_x;          // x dimension of numThreadPerGroup
    uint64_t num_threads_per_group_y;          // y dimension of numThreadPerGroup
    uint64_t num_threads_per_group_z;          // z dimension of numThreadPerGroup
    uint64_t num_alu_instructions;             // DX: Number of ALU instructions in the shader
    uint64_t num_instructions_control_flow;    // DX: Number of control flow instructions in the shader
    uint64_t num_instructions_fetch;           // DX: Number of HW TFETCHinstructions / Tx Units used
    uint64_t isa_size;                         // Size of ISA

public:
    // Lambda returning "N/A" string if value = -1 or string representation of value itself otherwise.
    static std::string na_or(uint64_t val)
    {
        return (val == static_cast<uint64_t>(-1) ? "N/A" : std::to_string(val));
    }
};

} // namespace beKA

#endif // RGA_RADEONGPUANALYZERBACKEND_SRC_BE_INCLUDE_H_

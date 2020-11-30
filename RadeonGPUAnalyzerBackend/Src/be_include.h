//=================================================================
// Copyright 2020 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

#ifndef RGA_RADEONGPUANALYZERBACKEND_SRC_BE_INCLUDE_H_
#define RGA_RADEONGPUANALYZERBACKEND_SRC_BE_INCLUDE_H_

#if defined(__linux__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wignored-qualifiers"
#endif
#include <CALModule.h>
#include <ACLModule.h>
#if defined(__linux__)
    #pragma GCC diagnostic pop
#endif

#include <algorithm>
#include <cctype>
#include <locale>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <set>
#include <map>
#include <cstdio>
#include <private/cal_private_ext.h>

#include "RadeonGPUAnalyzerBackend/Src/Common/AsicReg/devices.h"

// Logging callback type.
typedef void(*LoggingCallBackFuncP)(const std::string&);

namespace beKA
{
static const CALuint64 kCalValue64Na = (CALuint64) - 1;
static const CALuint64 kCalValue64Error = (CALuint64) - 2;

// Supported source language.
enum RgaMode
{
    kModeInvalid = -1,
    kModeNone = 0,
    kModeOpencl,            // Legacy OpenCL (live-driver) mode. Supports OpenCL input files.
    kModeRocmOpencl,        // ROCm OpenCL (offline) mode. Supports OpenCL input files.
    kModeOpengl,            // OpenGL mode. Supports GLSL input files.
    kModeVulkan,            // Live-driver Vulkan mode (leverages AMD Graphics driver). Supports GLSL, HLSL and SPIR-V input files.
    kModeVkOffline,         // Offline Vulkan mode. Supports GLSL input files.
    kModeVkOfflineSpv,      // Offline Vulkan mode. Supports SPIR-V binary input files.
    kModeVkOfflineSpvTxt,   // Offline Vulkan mode. Supports SPIR-V text input files.
    kModeDx11,              // DirectX 11 mode.
    kModeDx12,              // DirectX 12 mode.
    kModeDxr,               // DXR mode.
    kModeAmdil,             // AMDIL mode. Supports AMDIL input files.
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
    kBeStatusVulkanAmdspvLaunchFailure,
    kBeStatusVulkanAmdspvCompilationFailure,
    kBeStatusVulkanNoInputFile,
    kBeStatusVulkanEmptyInputFile,
    kBeStatusVulkanGlslangLaunchFailed,
    kBeStatusVulkanFrontendCompileFailed,
    kBeStatusVulkanBackendLaunchFailed,
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
    kBeStatusWriteToFileFailed,
    kBeStatusFailedOutputVerification,
    kBeStatusshaeCannotLocateAnalyzer,
    kBeStatusshaeIsaFileNotFound,
    kBeStatusshaeFailedToLaunch,
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
    kBeStatusGeneralFailed,
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
    CALuint64 scratch_memory_used;              // Size of statically allocated scratch memory
    CALuint64 wavefront_count_per_simd;         // Number of wavefronts per SIMD             CL_KERNELINFO_WAVEFRONT_PER_SIMD
    CALuint64 wavefront_size;                   // number of threads per wavefront.          CL_KERNELINFO_WAVEFRONT_SIZE
    CALuint64 num_gprs_available;               // number of GPRs available to the program   CL_KERNELINFO_AVAILABLE_GPRS
    CALuint64 num_gprs_used;                    // number of GPRs used by the program        CL_KERNELINFO_USED_GPRS
    CALuint64 lds_size_available;               // LDS size available to the program         CL_KERNELINFO_AVAILABLE_LDS_SIZE
    CALuint64 lds_size_used;                    // LDS size used by the program              CL_KERNELINFO_USED_LDS_SIZE
    CALuint64 stack_size_available;             // stack size available to the program        CL_KERNELINFO_AVAILABLE_STACK_SIZE
    CALuint64 stack_size_used;                  // stack size use by the program             CL_KERNELINFO_USED_STACK_SIZE
    CALuint64 num_sgprs_available;               // number of SGPRs available to the program  CL_KERNELINFO_AVAILABLE_SGPRS
    CALuint64 num_sgprs_used;                    // number of SGPRs used by the program       CL_KERNELINFO_USED_SGPRS
    CALuint64 num_sgpr_spills;                  // number of spills/fills of SGPRs to/from memory
    CALuint64 num_vgprs_available;               // number of VGPRs available to the program  CL_KERNELINFO_AVAILABLE_VGPRS
    CALuint64 num_vgprs_used;                    // number of VGPRs used by the program       CL_KERNELINFO_USED_VGPRS
    CALuint64 num_vgpr_spills;                  // number of spills/fills of VGPRs to/from memory

    CALuint64 num_threads_per_group_total;      // Flattened Number of threads per group
    CALuint64 num_threads_per_group_x;          // x dimension of numThreadPerGroup
    CALuint64 num_threads_per_group_y;          // y dimension of numThreadPerGroup
    CALuint64 num_threads_per_group_z;          // z dimension of numThreadPerGroup
    CALuint64 num_alu_instructions;             // DX: Number of ALU instructions in the shader
    CALuint64 num_instructions_control_flow;    // DX: Number of control flow instructions in the shader
    CALuint64 num_instructions_fetch;           // DX: Number of HW TFETCHinstructions / Tx Units used
    CALuint64 isa_size;                         // Size of ISA
};

} // namespace beKA

#endif // RGA_RADEONGPUANALYZERBACKEND_SRC_BE_INCLUDE_H_

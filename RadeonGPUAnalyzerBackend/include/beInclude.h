//=================================================================
// Copyright 2017 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

#ifndef _BEINCLUDE_H_
#define _BEINCLUDE_H_

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

// TODO Where should I get these?
// For now I've copied them from .../drivers/inc/asic_reg to KernelAnalyzer/Common/asic_reg.
// ../Common/Src is one plausible location.
#include "RadeonGPUAnalyzerBackend/include/beRGADllBuild.h"
#include <RadeonGPUAnalyzerBackend/include/Common/asic_reg/devices.h>

/// Logging callback type.
typedef void(*LoggingCallBackFuncP)(const std::string&);

namespace beKA
{
static const CALuint CAL_NA_Value = (CALuint) - 1;

static const CALuint64 CAL_NA_Value_64 = (CALuint64) - 1;

static const CALuint64 CAL_ERR_Value_64 = (CALuint64) - 2;

/// Supported source language
enum SourceLanguage
{
    SourceLanguage_None,
    SourceLanguage_Invalid,
    SourceLanguage_OpenCL,          // cl source of OpenCL kernels.
    SourceLanguage_GLSL,            // glsl input language for OpenGL (standalone, obsolete).
    SourceLanguage_GLSL_OpenGL,     // glsl input language for OpenGL Programs.
    SourceLanguage_GLSL_Vulkan,     // glsl input language for Vulkan Programs.
    SourceLanguage_SPIRV_Vulkan,    // Binary SPIR-V input for Vulkan Programs.
    SourceLanguage_SPIRVTXT_Vulkan, // Textual SPIR-V input for Vulkan Programs.
    SourceLanguage_HLSL,            // D3D/DX input language.
    SourceLanguage_AMDIL,           // AMDIL.
    SourceLanguage_DXasm,           // The other D3D/DX input language.
    SourceLanguage_DXasmT,          // D3D/DX Assembly as Text input language.
    SourceLanguage_Rocm_OpenCL      // ROCm OpenCL
};

enum BuiltProgramKind
{
    BuiltProgramKind_Invalid = 0,
    BuiltProgramKind_OpenCL,
    BuiltProgramKind_OpenGL,
    BuiltProgramKind_DX,
    BuiltProgramKind_Vulkan,
    BuiltProgramKind_LC
};

enum beStatus
{
    beStatus_Invalid = 0,
    beStatus_SUCCESS,
    beStatus_AMDDXX_MODULE_NOT_LOADED,
    beStatus_AmdDxGsaCompileShader_FAILED,
    beStatus_BACKEND_NOT_INITIALIZED,
    beStatus_BYTE_CODE_EXTRACT_FAILED,
    beStatus_CL_BUILD_PROGRAM_ICE,
    beStatus_D3DCompile_MODULE_NOT_LOADED,
    beStatus_D3DCompile_FAILED,
    beStatus_NO_BINARY_FOR_DEVICE,
    beStatus_NO_DEVICE_FOUND,
    beStatus_NO_IL_FOR_DEVICE,
    beStatus_NO_ISA_FOR_DEVICE,
    beStatus_NO_STATISTICS_FOR_DEVICE,
    beStatus_NO_OPENCL_AMD_PLATFORM,
    beStatus_NO_DEBUGIL_FOR_DEVICE,
    beStatus_NO_METADATA_FOR_DEVICE,
    beStatus_NO_SUCH_DEVICE,
    beStatus_OpenCL_MODULE_NOT_LOADED,
    beStatus_OpenCL_MODULE_TOO_OLD,
    beStatus_OpenCL_Compile_FAILED,
    beStatus_ACLCompile_FAILED,
    beStatus_ACLBinary_FAILED,
    beStatus_clCreateContextFromType_FAILED,
    beStatus_clCreateProgramWithSource_FAILED,
    beStatus_clGetContextInfo_FAILED,
    beStatus_clGetDeviceInfo_FAILED,
    beStatus_clGetPlatformIDs_FAILED,
    beStatus_clGetPlatformInfo_FAILED,
    beStatus_clGetProgramInfo_FAILED,
    beStatus_BuildOpenCLProgramWrapper_FAILED,
    beStatus_Create_Bolob_FromInput_Failed,
    beStatus_NoStatSectionInElfPossibleOldDxDriver,
    beStatus_WrongKernelName,
    beStatus_GLOpenGLVirtualContextFailedToLaunch,
    beStatus_GLOpenGLBuildError,
    beStatus_GLUnknownHardwareFamily,
    beStatus_VulkanAmdspvLaunchFailure,
    beStatus_VulkanAmdspvCompilationFailure,
    beStatus_VulkanNoInputFile,
    beStatus_FailedOutputVerification,
    beStatus_VulkanMixedInputFiles,
    beStatus_shaeCannotLocateAnalyzer,
    beStatus_shaeIsaFileNotFound,
    beStatus_shaeFailedToLaunch,
    beStatus_dxDriverLaunchFailure,
    beStatus_UnknownInputLang,
    beStatus_UnknownObjDumpOperation,
    beStatus_NoISAFileNameProvided,
    beStatus_NoOutputFileGenerated,
    beStatus_ParseIsaToCsvFailed,
    beStatus_ConstructParsedIsaFileNameFailed,
    beStatus_WriteParsedIsaFileFailed,
    beStatus_LC_CompilerLaunchFailed,
    beStatus_LC_CompilerTimeOut,
    beStatus_LC_CompilerGeneratedError,
    beStatus_LC_ObjDumpLaunchFailed,
    beStatus_LC_DisassembleFailed,
    beStatus_LC_BuilderLightningFailed,
    beStatus_LC_ParseCodeObjMDFailed,
    beStatus_LC_GetISASizeFailed,
    beStatus_LC_ParseCompilerVersionFailed,
    beStatus_LC_ConstructISAFileNameFailed,
    beStatus_LC_ExtractMetadataFailed,
    beStatus_LC_ExtractCodePropsFailed,
    beStatus_LC_StoreStatsFailed,
    beStatus_LC_SplitIsaFailed,
    beStatus_LC_ExtractKernelNamesFailed,
    beStatus_LC_GetKernelCodeSizeFailed,
    beStatus_WriteToFile_FAILED,
    beStatus_General_FAILED,
};

/// Selects which kind of text output to produce.
enum OutputKind
{
    OutputKind_Invalid = 0,
    OutputKind_IL,
    OutputKind_ISA,
    OutputKind_DebugIL,
    OutputKind_Metadata,
};

/// Selector for DeviceTable inquiries.
enum DeviceTableKind
{
    DeviceTableKind_Invalid = 0,
    DeviceTableKind_OpenCL,
    DeviceTableKind_OpenGL,
    DeviceTableKind_DX
};

/// Options that make sense for any of OpenCL, DX, GL.
struct CompileOptions
{
    // Source language
    SourceLanguage      m_SourceLanguage;

    // Optimization level.
    // -1 : Use compiler default value (not specified by user).
    //  0 : Disable optimization.
    //  1 : Minimal optimization.
    //  2 : Optimize for speed (usually, compiler default level).
    //  3 : Maximum optimization (may significantly slow down compilation).
    int                 m_optLevel    = -1;

    // Indicates whether IL dump is required or not.
    bool                m_dumpIL      = false;

    // Indicates whether line number info is required or not.
    bool                m_lineNumbers = false;
};

/// Object to customize binary output strings.
struct BinaryOptions
{
    /// List of sections to suppress.
    std::vector<std::string> m_SuppressSection;
};

/// Data collected in analyzing a kernel/function.
struct AnalysisData
{
    AnalysisData() :
        scratchMemoryUsed(0),
        numWavefrontPerSIMD(0),
        wavefrontSize(0),
        numGPRsAvailable(0),
        numGPRsUsed(0),
        LDSSizeAvailable(0),
        LDSSizeUsed(0),
        stackSizeAvailable(0),
        stackSizeUsed(0),
        numSGPRsAvailable(0),
        numSGPRsUsed(0),
        numSGPRSpills(0),
        numVGPRsAvailable(0),
        numVGPRsUsed(0),
        numVGPRSpills(0),
        numThreadPerGroup(0),
        numThreadPerGroupX(0),
        numThreadPerGroupY(0),
        numThreadPerGroupZ(0),
        totalNumThreadGroup(0),
        numAluInst(0),
        numControlFlowInst(0),
        numTfetchInst(0),
        ISASize(0)
    {
    }

    // Information directly from the compiled Kernel (passed back from SC).
    // We used to use the CAL interface to get this as a block.
    // We now use OpenCL extensions to get the same info.
    CALuint64 scratchMemoryUsed;       ///< Size of statically allocated scratch memory
    CALuint64 numWavefrontPerSIMD;     ///< Number of wavefronts per SIMD             CL_KERNELINFO_WAVEFRONT_PER_SIMD
    CALuint64 wavefrontSize;           ///< number of threads per wavefront.          CL_KERNELINFO_WAVEFRONT_SIZE
    CALuint64 numGPRsAvailable;        ///< number of GPRs available to the program   CL_KERNELINFO_AVAILABLE_GPRS
    CALuint64 numGPRsUsed;             ///< number of GPRs used by the program        CL_KERNELINFO_USED_GPRS
    CALuint64 LDSSizeAvailable;        ///< LDS size available to the program         CL_KERNELINFO_AVAILABLE_LDS_SIZE
    CALuint64 LDSSizeUsed;             ///< LDS size used by the program              CL_KERNELINFO_USED_LDS_SIZE
    CALuint64 stackSizeAvailable;      ///< stack size available to the program        CL_KERNELINFO_AVAILABLE_STACK_SIZE
    CALuint64 stackSizeUsed;           ///< stack size use by the program             CL_KERNELINFO_USED_STACK_SIZE
    CALuint64 numSGPRsAvailable;       ///< number of SGPRs available to the program  CL_KERNELINFO_AVAILABLE_SGPRS
    CALuint64 numSGPRsUsed;            ///< number of SGPRs used by the program       CL_KERNELINFO_USED_SGPRS
    CALuint64 numSGPRSpills;           ///< number of spills/fills of SGPRs to/from memory
    CALuint64 numVGPRsAvailable;       ///< number of VGPRs available to the program  CL_KERNELINFO_AVAILABLE_VGPRS
    CALuint64 numVGPRsUsed;            ///< number of VGPRs used by the program       CL_KERNELINFO_USED_VGPRS
    CALuint64 numVGPRSpills;           ///< number of spills/fills of VGPRs to/from memory

    CALuint64 numThreadPerGroup;       ///< flattened Number of threads per group
    CALuint64 numThreadPerGroupX;      ///< x dimension of numThreadPerGroup
    CALuint64 numThreadPerGroupY;      ///< y dimension of numThreadPerGroup
    CALuint64 numThreadPerGroupZ;      ///< z dimension of numThreadPerGroup
    CALuint64 totalNumThreadGroup;     ///< Total number of thread groups
    CALuint64 numAluInst;              ///< DX: Number of ALU instructions in the shader
    CALuint64 numControlFlowInst;      ///< DX: Number of control flow instructions in the shader
    CALuint64 numTfetchInst;           ///< DX: Number of HW TFETCHinstructions / Tx Units used
    CALuint64 ISASize;                 ///< Size of ISA
};

} // namespace beKA

#endif

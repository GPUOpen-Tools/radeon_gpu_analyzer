//======================================================================
// Copyright 2020-2022 Advanced Micro Devices, Inc. All rights reserved.
//======================================================================

#ifndef RGA_RADEONGPUANALYZERBACKEND_SRC_BE_STRING_CONSTANTS_H_
#define RGA_RADEONGPUANALYZERBACKEND_SRC_BE_STRING_CONSTANTS_H_

// LLVM Lightning Compiler.
#if defined(_WIN64) || defined(__linux)
static const wchar_t* kLcOpenclRootDir = L"utils/lc/opencl";
#elif defined(_WIN32)
static const wchar_t* kLcOpenclRootDir = L"x86/lc/opencl";
#endif

#if defined(_WIN32)
static const wchar_t* kLcCompilerExecutableExtension = L"exe";
#else
static const wchar_t* kLcCompilerExecutableExtension = L"";
#endif

// Vulkan
static const wchar_t* kGlslangRootDir = L"utils/vulkan";
static const wchar_t* kVulkanBackendRootDir = L"utils/vulkan";
static const wchar_t* kAmdgpudisRootDir = L"utils/lc/disassembler";

static const wchar_t* kGlslangExecutable = L"glslangValidator";
static const wchar_t* kSpirvAsExecutable = L"spirv-as";
static const wchar_t* kSpirvDisExecutable  = L"spirv-dis";
static const wchar_t* kSpirvLinkExecutable  = L"spirv-link";
static const wchar_t* kVulkanBackendExecutable  = L"vulkan_backend";
static const wchar_t* kAmdgpudisExecutable  = L"amdgpu-dis";

#if defined(_WIN32)
static const wchar_t* kGlslangExecutableExtension = L"exe";
static const wchar_t* kVulkanBackendExecutableExtension = L"exe";
static const wchar_t* kAmdgpudisExecutableExtension = L"exe";
#else
static const wchar_t* kAmdgpudisExecutableExtension     = L"";
static const wchar_t* kGlslangExecutableExtension = L"";
static const wchar_t* kVulkanBackendExecutableExtension = L"";
#endif

// Info messages.
static const char* kStrInfoVulkanMergedShadersGeometryVertex = "Info: geometry shader merged with vertex shader.";
static const char* kStrInfoVulkanMergedShadersGeometryTessellationEvaluation = "Info: geometry shader merged with tessellation evaluation shader.";
static const char* kStrInfoVulkanMergedShadersTessellationControlVertex = "Info: tessellation control shader merged with vertex shader.";

#endif // RGA_RADEONGPUANALYZERBACKEND_SRC_BE_STRING_CONSTANTS_H_

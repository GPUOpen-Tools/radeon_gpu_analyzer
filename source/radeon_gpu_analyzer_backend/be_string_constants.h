//=================================================================
// Copyright 2020 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

#ifndef RGA_RADEONGPUANALYZERBACKEND_SRC_BE_STRING_CONSTANTS_H_
#define RGA_RADEONGPUANALYZERBACKEND_SRC_BE_STRING_CONSTANTS_H_

// Device Names.
static const char* kDeviceNameCypress = "Cypress";
static const char* kDeviceNameCedar = "Cedar";
static const char* kDeviceNameTahiti = "Tahiti";
static const char* kDeviceNameHawaii = "Hawaii";
static const char* kDeviceNameKalindi = "Kalindi";
static const char* kDeviceNameGodavari = "Godavari";
static const char* kDeviceNameAmur = "Amur";
static const char* kDeviceNameNolan = "Nolan";
static const char* kDeviceNameStoney = "Stoney";
static const char* kDeviceNameGfx900 = "gfx900";
static const char* kDeviceNameGfx902 = "gfx902";
static const char* kDeviceNameGfx906 = "gfx906";
static const char* kDeviceNameGfx1010 = "gfx1010";
static const char* kDeviceNameGfx1011 = "gfx1011";
static const char* kDeviceNameGfx1012 = "gfx1012";
static const char* kDeviceNameGfx1030 = "gfx1030";
static const char* kDeviceNameGfx1031 = "gfx1031";
static const char* kDeviceNameGfx1032 = "gfx1032";
static const char* kDeviceNameGfx1034 = "gfx1034";

// LLVM Lightning Compiler.
#if defined(_WIN64) || defined(__linux)
static const wchar_t* kLcOpenclRootDir = L"utils/LC/OpenCL";
#elif defined(_WIN32)
static const wchar_t* kLcOpenclRootDir = L"x86/LC/OpenCL";
#endif

#if defined(_WIN32)
static const wchar_t* kLcCompilerExecutableExtension = L"exe";
#else
static const wchar_t* kLcCompilerExecutableExtension = L"";
#endif

// Vulkan
static const wchar_t* kGlslangRootDir = L"utils/Vulkan";
static const wchar_t* kVulkanBackendRootDir = L"utils/Vulkan";

static const wchar_t* kGlslangExecutable = L"glslangValidator";
static const wchar_t* kSpirvAsExecutable = L"spirv-as";
static const wchar_t* kSpirvDisExecutable  = L"spirv-dis";
static const wchar_t* kSpirvLinkExecutable  = L"spirv-link";
static const wchar_t* kVulkanBackendExecutable  = L"vulkan_backend";

#if defined(_WIN32)
static const wchar_t* kGlslangExecutableExtension = L"exe";
static const wchar_t* kVulkanBackendExecutableExtension = L"exe";
#else
static const wchar_t* kGlslangExecutableExtension = L"";
static const wchar_t* kVulkanBackendExecutableExtension = L"";
#endif

// Info messages.
static const char* kStrInfoVulkanMergedShadersGeometryVertex = "Info: geometry shader merged with vertex shader.";
static const char* kStrInfoVulkanMergedShadersGeometryTessellationEvaluation = "Info: geometry shader merged with tessellation evaluation shader.";
static const char* kStrInfoVulkanMergedShadersTessellationControlVertex = "Info: tessellation control shader merged with vertex shader.";

#endif // RGA_RADEONGPUANALYZERBACKEND_SRC_BE_STRING_CONSTANTS_H_

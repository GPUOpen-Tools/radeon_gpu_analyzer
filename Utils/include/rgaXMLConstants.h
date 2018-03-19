//=====================================================================
// Copyright 2017 Advanced Micro Devices, Inc. All rights reserved.

#pragma once

// CLI Version Info tags
static const char* XML_NODE_VERSION             = "Version";
static const char* XML_NODE_BUILD_DATE          = "BuildDate";

// Supported GPU tags
static const char* XML_NODE_SUPPORTED_GPUS      = "SupportedGPUs";
static const char* XML_NODE_GPU                 = "GPU";
static const char* XML_NODE_GENERATION          = "Generation";
static const char* XML_NODE_CODENAME            = "Codename";
static const char* XML_NODE_PRODUCT_NAMES       = "ProductNames";

static const char*  XML_NODE_MODE               = "Mode";

static const char*  XML_MODE_ROCM_CL            = "rocm-cl";
static const char*  XML_MODE_CL                 = "cl";
static const char*  XML_MODE_HLSL               = "hlsl";
static const char*  XML_MODE_OPENGL             = "opengl";
static const char*  XML_MODE_VULKAN             = "vulkan";

// CLI Output Metadata tags
static const char* XML_NODE_METADATA            = "ProgramOutputMetadata";
static const char* XML_NODE_DATA_MODEL          = "DataModel";
static const char* XML_NODE_INPUT_FILE          = "InputFile";
static const char* XML_NODE_PATH                = "Path";
static const char* XML_NODE_OUTPUT              = "Output";;
static const char* XML_NODE_TARGET              = "TargetGPU";
static const char* XML_NODE_ENTRY               = "Entry";
static const char* XML_NODE_NAME                = "Name";
static const char* XML_NODE_TYPE                = "Type";
static const char* XML_NODE_BINARY              = "Binary";
static const char* XML_NODE_ISA                 = "ISA";
static const char* XML_NODE_CSV_ISA             = "CSV_ISA";
static const char* XML_NODE_RES_USAGE           = "ResourceUsage";
static const char* XML_NODE_LIVEREG             = "LiveReg";
static const char* XML_NODE_CFG                 = "CFG";

// Shader types
static const char* XML_SHADER_OPENCL            = "OpenCL_Kernel";
static const char* XML_SHADER_DX_VERTEX         = "DX_Vertex";
static const char* XML_SHADER_DX_HULL           = "DX_Hull";
static const char* XML_SHADER_DX_DOMAIN         = "DX_Domain";
static const char* XML_SHADER_DX_GEOMETRY       = "DX_Geometry";
static const char* XML_SHADER_DX_PIXEL          = "DX_Pixel";
static const char* XML_SHADER_DX_COMPUTE        = "DX_Compute";
static const char* XML_SHADER_GL_VERTEX         = "GL_Vertex";
static const char* XML_SHADER_GL_TESS_CTRL      = "GL_TessellationControl";
static const char* XML_SHADER_GL_TESS_EVAL      = "GL_EvaluationControl";
static const char* XML_SHADER_GL_GEOMETRY       = "GL_Geometry";
static const char* XML_SHADER_GL_FRAGMENT       = "GL_Fragment";
static const char* XML_SHADER_GL_COMPUTE        = "GL_Compute";
static const char* XML_SHADER_UNKNOWN           = "Unknown";

// Other
static const char* XML_UNKNOWN_SOURCE_FILE      = "<Unknown>";

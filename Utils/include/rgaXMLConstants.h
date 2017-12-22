//=====================================================================
// Copyright 2017 Advanced Micro Devices, Inc. All rights reserved.

// CLI Version Info tags
const char*  XML_NODE_VERSION           = "Version";
const char*  XML_NODE_BUILD_DATE        = "BuildDate";
const char*  XML_NODE_SUPPORTED_GPUS    = "SupportedGPUs";
const char*  XML_NODE_GPU               = "GPU";
const char*  XML_NODE_GENERATION        = "Generation";
const char*  XML_NODE_CODENAME          = "Codename";
const char*  XML_NODE_PRODUCT_NAMES     = "ProductNames";

const char*  XML_NODE_MODE              = "Mode";

const char*  XML_MODE_ROCM_CL           = "rocm-cl";
const char*  XML_MODE_CL                = "cl";
const char*  XML_MODE_HLSL              = "hlsl";
const char*  XML_MODE_OPENGL            = "opengl";
const char*  XML_MODE_VULKAN            = "vulkan";

// CLI Output Metadata tags
const char*  XML_NODE_METADATA          = "ProgramOutputMetadata";
const char*  XML_NODE_DATA_MODEL        = "DataModel";
const char*  XML_NODE_INPUT_FILE        = "InputFile";
const char*  XML_NODE_PATH              = "Path";
const char*  XML_NODE_OUTPUT            = "Output";
const char*  XML_NODE_TARGET            = "TargetGPU";
const char*  XML_NODE_ENTRY             = "Entry";
const char*  XML_NODE_NAME              = "Name";
const char*  XML_NODE_TYPE              = "Type";
const char*  XML_NODE_BINARY            = "Binary";
const char*  XML_NODE_ISA               = "ISA";
const char*  XML_NODE_CSV_ISA           = "CSV_ISA";
const char*  XML_NODE_RES_USAGE         = "ResourceUsage";
const char*  XML_NODE_LIVEREG           = "LiveReg";
const char*  XML_NODE_CFG               = "CFG";

// Shader types
const char*  XML_SHADER_OPENCL          = "OpenCL_Kernel";
const char*  XML_SHADER_DX_VERTEX       = "DX_Vertex";
const char*  XML_SHADER_DX_HULL         = "DX_Hull";
const char*  XML_SHADER_DX_DOMAIN       = "DX_Domain";
const char*  XML_SHADER_DX_GEOMETRY     = "DX_Geometry";
const char*  XML_SHADER_DX_PIXEL        = "DX_Pixel";
const char*  XML_SHADER_DX_COMPUTE      = "DX_Compute";
const char*  XML_SHADER_GL_VERTEX       = "GL_Vertex";
const char*  XML_SHADER_GL_TESS_CTRL    = "GL_TessellationControl";
const char*  XML_SHADER_GL_TESS_EVAL    = "GL_EvaluationControl";
const char*  XML_SHADER_GL_GEOMETRY     = "GL_Geometry";
const char*  XML_SHADER_GL_FRAGMENT     = "GL_Fragment";
const char*  XML_SHADER_GL_COMPUTE      = "GL_Compute";
const char*  XML_SHADER_UNKNOWN         = "Unknown";

// Other
const char*  XML_UNKNOWN_SOURCE_FILE    = "<Unknown>";

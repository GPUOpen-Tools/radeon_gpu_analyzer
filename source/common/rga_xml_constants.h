//=====================================================================
// Copyright 2017 Advanced Micro Devices, Inc. All rights reserved.

#pragma once

// CLI Version Info tags
static const char* kStrXmlNodeVersion             = "Version";
static const char* kStrXmlNodeBuildDate          = "BuildDate";

// Supported GPU tags
static const char* kStrXmlNodeSupportedGpus      = "SupportedGPUs";
static const char* kStrXmlNodeGpu                 = "GPU";
static const char* kStrXmlNodeGeneration          = "Generation";
static const char* kStrXmlNodeCodename            = "Codename";
static const char* kStrXmlNodeProductNames       = "ProductNames";

static const char*  kStrXmlNodeMode               = "Mode";

static const char*  kStrXmlNodeOpenclOffline      = "opencl";
static const char*  kStrXmlNodeHlsl               = "hlsl";
static const char*  kStrXmlNodeOpengl             = "opengl";
static const char*  kStrXmlNodeVulkan             = "vulkan";

// System level Version Info tags
static const char*  kStrXmlNodeSystem             = "System";
static const char*  kStrXmlNodeAdapters           = "DisplayAdapters";
static const char*  kStrXmlNodeAdapter            = "DisplayAdapter";
static const char*  kStrXmlNodeId                 = "ID";
static const char*  kStrXmlNodeVkDriver          = "VulkanDriverVersion";
static const char*  kStrXmlNodeVkApi             = "VulkanAPIVersion";

// CLI Output Metadata tags
static const char* kStrXmlNodeMetadata            = "ProgramOutputMetadata";
static const char* kStrXmlNodeDataModel          = "DataModel";
static const char* kStrXmlNodeInputFile          = "InputFile";
static const char* kStrXmlNodePath                = "Path";
static const char* kStrXmlNodeOutput              = "Output";;
static const char* kStrXmlNodeTarget              = "TargetGPU";
static const char* kStrXmlNodeEntry               = "Entry";
static const char* kStrXmlNodeName                = "Name";
static const char* kStrXmlNodeType                = "Type";
static const char* kStrXmlNodeBinary              = "Binary";
static const char* kStrXmlNodeIsa                 = "ISA";
static const char* kStrXmlNodeCsvIsa             = "CSV_ISA";
static const char* kStrXmlNodeResUsage           = "ResourceUsage";
static const char* kStrXmlNodeLivereg             = "LiveReg";
static const char* kStrXmlNodeCfg                 = "CFG";
static const char* kStrXmlNodePipeline            = "Pipeline";
static const char* kStrXmlNodeStage               = "Stage";

// Pipeline types
static const char* kStrXmlNodeTypeGraphics   = "Graphics";
static const char* kStrXmlNodeTypeCompute    = "Compute";

// CLI Output Metadata tags for graphics and compute pipelines.
static const char* kStrXmlNodeMetadataPipeline           = "PipelineOutputMetadata";
static const char* kStrXmlNodePipelineTypeGraphics      = "Graphics";
static const char* kStrXmlNodePipelineTypeCompute       = "Compute";
static const char* kStrXmlNodePipelineStageVertex       = "Vert";
static const char* kStrXmlNodePipelineStageTessCtrl    = "Tesc";
static const char* kStrXmlNodePipelineStageTessEval    = "Tese";
static const char* kStrXmlNodePipelineStageGeometry     = "Geom";
static const char* kStrXmlNodePipelineStageFragment     = "Frag";
static const char* kStrXmlNodePipelineStageCompute      = "Comp";

// Shader types
static const char* kStrXmlNodeOpencl            = "OpenCL_Kernel";
static const char* kStrXmlNodeDxVertex         = "DX_Vertex";
static const char* kStrXmlNodeDxHull           = "DX_Hull";
static const char* kStrXmlNodeDxDomain         = "DX_Domain";
static const char* kStrXmlNodeDxGeometry       = "DX_Geometry";
static const char* kStrXmlNodeDxPixel          = "DX_Pixel";
static const char* kStrXmlNodeDxCompute        = "DX_Compute";
static const char* kStrXmlNodeGlVertex         = "GL_Vertex";
static const char* kStrXmlNodeGlTessCtrl      = "GL_TessellationControl";
static const char* kStrXmlNodeGlTessEval      = "GL_EvaluationControl";
static const char* kStrXmlNodeGlGeometry       = "GL_Geometry";
static const char* kStrXmlNodeGlFragment       = "GL_Fragment";
static const char* kStrXmlNodeGlCompute        = "GL_Compute";
static const char* kStrXmlNodeUnknown           = "Unknown";

// Other
static const char* kStrXmlNodeSourceFile      = "<Unknown>";
static const char* kStrXmlNodeMktName         = "<Unknown>";

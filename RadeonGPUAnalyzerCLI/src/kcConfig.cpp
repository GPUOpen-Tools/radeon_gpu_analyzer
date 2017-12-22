//=================================================================
// Copyright 2017 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

#if GSA_BUILD
    #include <sstream>
#endif

#include <RadeonGPUAnalyzerCLI/src/kcConfig.h>
#include <RadeonGPUAnalyzerCLI/src/kcCliStringConstants.h>

using namespace std;

std::string Config::sourceKindHLSL       = "HLSL";
std::string Config::sourceKindAMDIL      = "AMDIL";
std::string Config::sourceKindDXAsm      = "DXAsm";
std::string Config::sourceKindDXAsmT     = "DXAsmTxt";
std::string Config::sourceKindOpenCL     = "CL";
std::string Config::sourceKindGLSL       = "GLSL";
std::string Config::sourceKindOpenGL     = "OPENGL";
std::string Config::sourceKindGLSLVulkan = "VULKAN";
std::string Config::sourceKindSpirvBin   = "VULKAN-SPV";
std::string Config::sourceKindSpirvTxt   = "VULKAN-SPV-TXT";
std::string Config::sourceKindRocmOpenCL = "ROCM-CL";

Config::Config() :
    m_SourceLanguage(SourceLanguage_None),
    m_RequestedCommand(ccNone),
    m_AnalysisFile(),
    m_ILFile(),
    m_ISAFile(),
    m_LiveRegisterAnalysisFile(),
    m_BinaryOutputFile(),
    m_Function(),
    m_CSVSeparator(),
    m_MetadataFile(),
    m_ASICs(),
    m_SuppressSection(),
    m_OpenCLOptions(),
    m_Defines(),
    m_IncludePath(),
    m_isRetainUserBinaryPath(false),
    m_isParsedISARequired(false),
    m_isLineNumbersRequired(false),
    m_SourceKind(),
    m_Profile(),
    m_DXFlags(0),
    m_DXLocation(),
    m_DXAdapter(-1),
    m_FXC(),
    m_DumpMSIntermediate(),
    m_EnableShaderIntrinsics(false),
    m_UAVSlot(-1),
    m_optLevel(-1)
{
}

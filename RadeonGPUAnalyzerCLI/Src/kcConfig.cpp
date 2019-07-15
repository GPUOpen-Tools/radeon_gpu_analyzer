//=================================================================
// Copyright 2017 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

#if GSA_BUILD
    #include <sstream>
#endif

#include <RadeonGPUAnalyzerCLI/Src/kcConfig.h>
#include <RadeonGPUAnalyzerCLI/Src/kcCliStringConstants.h>

using namespace std;

std::string Config::sourceKindDx11                = "DX11";
std::string Config::sourceKindDx12                = "DX12";
std::string Config::sourceKindAMDIL               = "AMDIL";
std::string Config::sourceKindOpenCL              = "CL";
std::string Config::sourceKindOpenGL              = "OPENGL";
std::string Config::sourceKindGLSLVulkanOffline   = "VK-OFFLINE";
std::string Config::sourceKindSpirvBinOffline     = "VK-SPV-OFFLINE";
std::string Config::sourceKindSpirvTxtOffline     = "VK-SPV-TXT-OFFLINE";
std::string Config::sourceKindVulkan              = "VULKAN";
std::string Config::sourceKindRocmOpenCL          = "ROCM-CL";

Config::Config() :
    m_mode(Mode_None),
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
    m_isWarningsRequired(false),
    m_isHlslInput(false),
    m_isGlslInput(false),
    m_isSpvInput(false),
    m_isSpvTxtInput(false),
    m_vertShaderFileType(rgVulkanInputType::Unknown),
    m_tescShaderFileType(rgVulkanInputType::Unknown),
    m_teseShaderFileType(rgVulkanInputType::Unknown),
    m_fragShaderFileType(rgVulkanInputType::Unknown),
    m_geomShaderFileType(rgVulkanInputType::Unknown),
    m_compShaderFileType(rgVulkanInputType::Unknown),
    m_SourceKind(),
    m_Profile(),
    m_DXFlags(0),
    m_DXLocation(),
    m_DXAdapter(-1),
    m_FXC(),
    m_DumpMSIntermediate(),
    m_EnableShaderIntrinsics(false),
    m_UAVSlot(-1),
    m_optLevel(-1),
    m_printProcessCmdLines(false)
{
}

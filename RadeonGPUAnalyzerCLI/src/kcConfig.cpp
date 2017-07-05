//=================================================================
// Copyright 2017 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

#if GSA_BUILD
    #include <sstream>
#endif

#include <RadeonGPUAnalyzerCLI/src/kcConfig.h>

using namespace std;

std::string Config::sourceKindHLSL = "HLSL";
std::string Config::sourceKindAMDIL = "AMDIL";
std::string Config::sourceKindDXAsm = "DXAsm";
std::string Config::sourceKindDXAsmT = "DXAsmTxt";
std::string Config::sourceKindOpenCL = "CL";
std::string Config::sourceKindGLSL = "GLSL";
std::string Config::sourceKindOpenGL = "OPENGL";
std::string Config::sourceKindVulkan = "VULKAN";
std::string Config::sourceKindSpirvBin = "VULKAN-SPV";
std::string Config::sourceKindSpirvTxt = "VULKAN-SPV-TXT";

Config::Config() :
    m_SourceLanguage(SourceLanguage_Invalid),
    m_RequestedCommand(ccInvalid),
    m_InputFile(),
    m_AnalysisFile(),
    m_ILFile(),
    m_ISAFile(),
    m_LiveRegisterAnalysisFile(),
    m_BinaryOutputFile(),
    m_Function(),
    m_CSVSeparator(),
    m_DebugILFile(),
    m_MetadataFile(),
    m_ASICs(),
    m_SuppressSection(),
    m_OpenCLOptions(),
    m_Defines(),
    m_IncludePath(),
    m_isRetainUserBinaryPath(false),
    m_SourceKind(),
    m_Profile(),
    m_DXFlags(0),
    m_DXLocation(),
    m_FXC(),
    m_DumpMSIntermediate(),
    m_EnableShaderIntrinsics(false),
    m_ListGraphicsAdapters(false),
    m_UAVSlot(-1)
{
}

void
Config::dump(ostream& out) const
{
    out << "m_InputFile:            " << m_InputFile << endl;
    out << "m_AnalysisFile:         " << m_AnalysisFile << endl;
    out << "m_ILFile:               " << m_ILFile << endl;
    out << "m_ISAFile:              " << m_ISAFile << endl;
    out << "m_registerLivenessFile: " << m_LiveRegisterAnalysisFile << endl;
    out << "m_controlFlowGraphFile: " << m_ControlFlowGraphFile << endl;
    out << "m_BinaryOutputFile:     " << m_BinaryOutputFile << endl;
    out << "m_Function:             " << m_Function << endl;
    out << "m_CSVSeparator:         " << m_CSVSeparator << endl;
    out << "m_RequestedCommand:     " << m_RequestedCommand << endl;
    out << "m_SourceLanguage:       " << m_SourceLanguage << endl;
    out << "m_DebugILFile:          " << m_DebugILFile << endl;
    out << "m_MetadataFile:         " << m_MetadataFile << endl;
    out << "m_FXC:                  " << m_FXC << endl;
    out << "m_DumpMSIntermediate:   " << m_DumpMSIntermediate << endl;
    out << "m_EnableShaderIntrinsics:   " << m_EnableShaderIntrinsics << endl;
    out << "m_ListGraphicsAdapters:   " << m_ListGraphicsAdapters << endl;
    out << "m_UAVSlot:   " << m_UAVSlot << endl;

    out << "m_ASICs:               ";

    for (vector<string>::const_iterator it = m_ASICs.begin(); it < m_ASICs.end(); ++it)
    {
        out << " " << *it;
    }

    out << endl;

    out << "m_SuppressSection:     ";

    for (vector<string>::const_iterator it = m_SuppressSection.begin(); it < m_SuppressSection.end(); ++it)
    {
        out << " " << *it;
    }

    out << endl;

    out << "m_OpenCLOptions:       ";

    for (vector<string>::const_iterator it = m_OpenCLOptions.begin(); it < m_OpenCLOptions.end(); ++it)
    {
        out << " " << *it;
    }

    out << endl;


    out << "m_Defines:             ";

    for (vector<string>::const_iterator it = m_Defines.begin(); it < m_Defines.end(); ++it)
    {
        out << " " << *it;
    }

    out << "m_IncludePath:             ";

    for (vector<string>::const_iterator it = m_IncludePath.begin(); it < m_IncludePath.end(); ++it)
    {
        out << " " << *it;
    }

    out << endl;

    // DX/GL stuff
    out << "m_SourceKind:            " << m_SourceKind << endl;
    out << "m_Profile:               " << m_Profile << endl;
    out << "m_DXFlags:               " << m_DXFlags << endl;
    out << "m_DXLocation:            " << m_DXLocation << endl;
}

std::ostream& operator<<(std::ostream& ostr, const Config& config)
{
    config.dump(ostr);
    return ostr;
}

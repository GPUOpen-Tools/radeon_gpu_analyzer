//=================================================================
// Copyright 2017 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

#ifndef __beProgramBuilderOpenGL_h
#define __beProgramBuilderOpenGL_h

// C++.
#include <string>
#include <set>

// Local.
#include <RadeonGPUAnalyzerBackend/include/beProgramBuilder.h>
#include <RadeonGPUAnalyzerBackend/include/beDataTypes.h>
#include <RadeonGPUAnalyzerBackend/include/beInclude.h>

struct OpenGLOptions : public beKA::CompileOptions
{
    OpenGLOptions() : m_chipFamily(0), m_chipRevision(0), m_isAmdIsaBinariesRequired(false),
        m_isAmdIsaDisassemblyRequired(false), m_isScStatsRequired(false), m_isCfgRequired(false), m_isLiveRegisterAnalysisRequired(false)
    {
        CompileOptions::m_SourceLanguage = beKA::SourceLanguage_GLSL;
    }

    // The target device's chip family.
    size_t m_chipFamily;

    // The target device's chip revision.
    size_t m_chipRevision;

    // The input shader file names.
    beProgramPipeline m_pipelineShaders;

    // ISA disassembly output file names.
    beProgramPipeline m_isaDisassemblyOutputFiles;

    // Register liveness analysis output file names.
    beProgramPipeline m_liveRegisterAnalysisOutputFiles;

    // Register control flow graph output file names.
    beProgramPipeline m_controlFlowGraphOutputFiles;

    // SC statistics output file name.
    beProgramPipeline m_scStatisticsOutputFiles;

    // ISA binary output file name.
    gtString m_programBinaryFile;

    // True to generate AMD ISA binaries.
    bool m_isAmdIsaBinariesRequired;

    // True to generate AMD ISA disassembly.
    bool m_isAmdIsaDisassemblyRequired;

    // True to perform live register analysis.
    bool m_isLiveRegisterAnalysisRequired;

    // True to perform control flow graph.
    bool m_isCfgRequired;

    // True to generate shader compiler statistics.
    bool m_isScStatsRequired;
};

class RGA_BACKEND_DECLDIR beProgramBuilderOpenGL :
    public beProgramBuilder
{
public:
    beProgramBuilderOpenGL();
    virtual ~beProgramBuilderOpenGL();

    virtual beKA::beStatus GetBinary(const std::string& device, const beKA::BinaryOptions& binopts, std::vector<char>& binary) override;

    virtual beKA::beStatus GetKernelILText(const std::string& device, const std::string& kernel, std::string& il) override;

    virtual beKA::beStatus GetKernelISAText(const std::string& device, const std::string& kernel, std::string& isa) override;

    virtual beKA::beStatus GetStatistics(const std::string& device, const std::string& kernel, beKA::AnalysisData& analysis) override;

    virtual beKA::beStatus GetDeviceTable(std::vector<GDT_GfxCardInfo>& table) override;

    beKA::beStatus Compile(const OpenGLOptions& vulkanOptions, bool& cancelSignal, bool printCmd, gtString& compilerOutput);

    /// Extracts the OpenGL version of the installed runtime.
    bool GetOpenGLVersion(bool printCmd, gtString& glVersion);

    /// Retrieve the device ID and Revision ID from the OpenGL backend.
    bool GetDeviceGLInfo(const std::string& deviceName, size_t& deviceFamilyId, size_t& deviceRevision) const;

    /// Retrieves the list of supported devices.
    static bool GetSupportedDevices(std::set<std::string>& deviceList);

    /// Retrieves the list of disabled devices.
    static const std::set<std::string>& GetDisabledDevices();

private:
    std::set<std::string> m_publicDevices;
};

#endif // __beProgramBuilderOpenGL_h

//=================================================================
// Copyright 2017 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

#ifndef beProgramBuilderVkOffline_h__
#define beProgramBuilderVkOffline_h__

// C++.
#include <string>
#include <set>

// Local.
#include <RadeonGPUAnalyzerBackend/Include/beProgramBuilder.h>
#include <RadeonGPUAnalyzerBackend/Include/beInclude.h>
#include <RadeonGPUAnalyzerBackend/Include/beDataTypes.h>

struct VkOfflineOptions : public beKA::CompileOptions
{
    VkOfflineOptions(beKA::RgaMode mode) : m_isSpirvBinariesRequired(false), m_isAmdPalIlBinariesRequired(false),
        m_isAmdPalIlDisassemblyRequired(false), m_isAmdIsaBinariesRequired(false),
        m_isAmdIsaDisassemblyRequired(false), m_isScStatsRequired(false)
    {
        CompileOptions::m_mode = mode;
    }

    // The target devices.
    std::string m_targetDeviceName;

    // The generic input file name.
    std::string m_stagelessInputFile;

    // The input shader file names.
    beProgramPipeline m_pipelineShaders;

    // AMD PAL IL binary output file names.
    beProgramPipeline m_palIlBinaryOutputFiles;

    // AMD PAL IL disassembly output files.
    beProgramPipeline m_pailIlDisassemblyOutputFiles;

    // ISA disassembly output file names.
    beProgramPipeline m_isaDisassemblyOutputFiles;

    // Register liveness analysis output file names.
    beProgramPipeline m_liveRegisterAnalysisOutputFiles;

    // Control flow graph output file names.
    beProgramPipeline m_controlFlowGraphOutputFiles;

    // ISA binary output file name.
    beProgramPipeline m_isaBinaryFiles;

    // SC statistics output file name.
    beProgramPipeline m_scStatisticsOutputFiles;

    // True to generate SPIR-V binaries.
    bool m_isSpirvBinariesRequired;

    // True to generate AMD PAL IL binaries.
    bool m_isAmdPalIlBinariesRequired;

    // True to generate AMD PAL IL disassembly.
    bool m_isAmdPalIlDisassemblyRequired;

    // True to generate AMD ISA binaries.
    bool m_isAmdIsaBinariesRequired;

    // True to generate AMD ISA binaries.
    bool m_isAmdIsaDisassemblyRequired;

    // True to perform live register analysis.
    bool m_isLiveRegisterAnalysisRequired;

    // True to generate shader compiler statistics.
    bool m_isScStatsRequired;
};

class beProgramBuilderVkOffline : public beProgramBuilder
{
public:
    beProgramBuilderVkOffline() = default;
    ~beProgramBuilderVkOffline() = default;

    virtual beKA::beStatus GetBinary(const std::string& device, const beKA::BinaryOptions& binopts, std::vector<char>& binary) override;

    virtual beKA::beStatus GetKernelILText(const std::string& device, const std::string& kernel, std::string& il) override;

    virtual beKA::beStatus GetKernelISAText(const std::string& device, const std::string& kernel, std::string& isa) override;

    virtual beKA::beStatus GetStatistics(const std::string& device, const std::string& kernel, beKA::AnalysisData& analysis) override;

    virtual beKA::beStatus GetDeviceTable(std::vector<GDT_GfxCardInfo>& table) override;

    beKA::beStatus Compile(const VkOfflineOptions& vulkanOptions, bool& cancelSignal, bool printCmd, gtString& buildLog);

    /// Extracts the OpenGL version of the installed runtime.
    bool GetVulkanVersion(gtString& vkVersion);

    /// Retrieves the list of supported devices.
    static bool GetSupportedDevices(std::set<std::string>& deviceList);

private:

    std::set<std::string> m_publicDevices;
};

#endif // beProgramBuilderVkOffline_h__

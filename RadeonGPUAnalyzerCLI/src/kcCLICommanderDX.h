//=================================================================
// Copyright 2017 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

#pragma once

#ifdef _WIN32

// Local.
#include <RadeonGPUAnalyzerCLI/src/kcCLICommander.h>
#include <RadeonGPUAnalyzerCLI/src/kcDataTypes.h>

class kcCLICommanderDX : public kcCLICommander
{
public:
    kcCLICommanderDX(void);
    virtual ~kcCLICommanderDX(void);

    /// List the adapters installed on the system.
    void ListAdapters(Config& config, LoggingCallBackFunc_t callback) override;

    void RunCompileCommands(const Config& config, LoggingCallBackFunc_t callback);

private: //functions
    bool Init(const Config& config, LoggingCallBackFunc_t callback);
    bool ListGraphicsAdapters(const Config& config, LoggingCallBackFunc_t callback);
    void InitRequestedAsicListDX(const Config& config);
    bool Compile(const Config& config, const GDT_GfxCardInfo& gfxCardInfo, string sDevicenametoLog);
    bool WriteAnalysisDataForDX(const Config& config, const std::vector<AnalysisData>& AnalysisDataVec, const std::vector<string>& DeviceAnalysisDataVec,
                                const std::string& sAnalysisFile, std::stringstream& log);
    void ExtractISA(const std::string& deviceName, const Config& config, size_t& isaSizeInBytes,
                    std::string isaBuffer, bool& isIsaSizeDetected, bool& shouldDetectIsaSize);
    void ExtractIL(const std::string& deviceName, const Config& config);
    bool ExtractStats(const  std::string& deviceName, const Config& config, bool shouldDetectIsaSize, std::string isaBuffer,
                      bool isIsaSizeDetected, size_t isaSizeInBytes, vector<AnalysisData>& AnalysisDataVec, vector<string>& DeviceAnalysisDataVec);
    void ExtractBinary(const std::string& deviceName, const Config& config);

private: //members

    std::vector<GDT_GfxCardInfo> m_dxDefaultAsicsList;

    Backend* m_pBackEndHandler;

};

#endif

//=================================================================
// Copyright 2020 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================
#ifndef RGA_RADEONGPUANALYZERCLI_SRC_KC_CLI_COMMANDER_DX11_H_
#define RGA_RADEONGPUANALYZERCLI_SRC_KC_CLI_COMMANDER_DX11_H_

#ifdef _WIN32

// Local.
#include "radeon_gpu_analyzer_cli/kc_cli_commander.h"
#include "radeon_gpu_analyzer_cli/kc_data_types.h"

class KcCliCommanderDX : public KcCliCommander
{
public:
    KcCliCommanderDX(void);
    virtual ~KcCliCommanderDX(void);

    /// List the adapters installed on the system.
    void ListAdapters(Config& config, LoggingCallbackFunction callback) override;
    void RunCompileCommands(const Config& config, LoggingCallbackFunction callback) override;
    bool PrintAsicList(const Config& config) override;

private:
    // Functions.
    bool Init(const Config& config, LoggingCallbackFunction callback);
    void InitRequestedAsicListDX(const Config& config);
    bool Compile(const Config& config, const GDT_GfxCardInfo& gfx_card_info, const std::string& device_name_to_log);
    bool WriteAnalysisDataForDX(const Config& config, const std::vector<AnalysisData>& analysis_data, const std::vector<string>& device_analysis_data,
                                const std::string& analysis_file, std::stringstream& log);
    void ExtractISA(const std::string& device_name, const Config& config, size_t& isa_size_in_bytes,
                    std::string isa_buffer, bool& is_isa_size_detected, bool& should_detect_isa_size);
    void ExtractIL(const std::string& device_name, const Config& config);
    bool ExtractStats(const  std::string& device_name, const Config& config, bool should_detect_isa_size, std::string isa_buffer,
                      bool is_isa_size_detected, size_t isa_size_in_bytes, std::vector<AnalysisData>& analysis_data, std::vector<string>& device_analysis_data);
    void ExtractBinary(const std::string& device_name, const Config& config);

private:
    // Data members.
    std::vector<GDT_GfxCardInfo> dx_default_asics_list_;
    Backend* backend_handler_;
};

#endif

#endif // RGA_RADEONGPUANALYZERCLI_SRC_KC_CLI_COMMANDER_DX11_H_

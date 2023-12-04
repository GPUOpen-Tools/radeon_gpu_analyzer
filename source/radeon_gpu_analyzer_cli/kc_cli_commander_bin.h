//======================================================================
// Copyright 2023 Advanced Micro Devices, Inc. All rights reserved.
//======================================================================
#ifndef RGA_RADEONGPUANALYZERCLI_SRC_KC_CLI_COMMANDER_BIN_H_
#define RGA_RADEONGPUANALYZERCLI_SRC_KC_CLI_COMMANDER_BIN_H_

//C++.
#include <memory>

// Backend.
#include "radeon_gpu_analyzer_backend/be_program_builder_vulkan.h"

// Local.
#include "radeon_gpu_analyzer_cli/kc_cli_commander.h"
#include "radeon_gpu_analyzer_cli/kc_cli_commander_bin_util.h"

// Commander interface for binary code objects.
class KcCliCommanderBin : public KcCliCommander
{
public:
    KcCliCommanderBin()  = default;

    // RunCompileCommands function is called as such mostly for legacy reasons.
    // This function has no compiling step. It generates disassembly, and post-processing analyses.
    virtual void RunCompileCommands(const Config& config, LoggingCallbackFunction callback) override;

    // Perform post-compile actions.
    virtual bool RunPostCompileSteps(const Config& config) override;

    // Generates Binary Anaslysis "version info" data and writes it to the file specified by "fileName".
    // The data will be appended to the existing content of the file.
    static bool GenerateBinaryAnalysisVersionInfo(const std::string& fileName);

    // Get the list of names of supported targets in DeviceInfo format.
    static beKA::beStatus GetSupportedTargets(std::set<std::string>& targets);

private:

    // Validate .bin input file.
    beKA::beStatus IsBinaryInputValid(const Config& config) const;

    // Invoke the amdgpu-dis executable.
    beKA::beStatus InvokeAmdgpudis(const std::string& bin_file, bool should_print_cmd, std::string& out_text, std::string& error_txt) const;

    // Parses amdgpu-dis output and detects workflow type (graphics or compute).
    beKA::beStatus DetectAndSetWorkflowStrategy(const Config& config, const std::string& amdgpu_dis_output);

    // Extract target device from ISA disassembly.
    bool ExtractDeviceFromAmdgpudisOutput(const std::string& amdgpu_dis_output, std::string& device) const;

    // Identify the devices requested by user.
    beKA::beStatus InitRequestedAsicBinary(const Config& config, const std::string& amdgpu_dis_output);

    // Disassemble a code object binary file to ISA disassembly file(s).
    beKA::beStatus DisassembleBinary(const Config& config, const std::string& amdgpu_dis_output) const;

    // Parses amdgpu-dis output and extracts a table with
    // the amdgpu kernel name being the key and that shader stage's disassembly the value.
    beKA::beStatus ParseAmdgpudisKernels(const std::string&                  amdgpu_dis_output,
                                         std::map<std::string, std::string>& shader_to_disassembly,
                                         std::string&                        error_msg) const;

    // Write output file(s) to disk.
    beKA::beStatus WriteOutputFiles(const Config&                             config,
                                    const std::map<std::string, std::string>& kernel_to_disassembly,
                                    std::string&                              error_msg) const;

    // Perform post-processing actions.
    void RunPostProcessingSteps(const Config& config) const;

    // Target GPU extracted from dissassembly.
    std::string asic_;

    // Code object Metadata extracted from dissassembly.
    BeAmdPalMetaData::PipelineMetaData amdpal_pipeline_md_;

    // Pointer to call ParseAmdgpudisOutput methods - implementation differs for graphics or compute. 
    std::unique_ptr<ParseAmdgpudisOutputStrategy> amdgpudis_parser_strategy_ = nullptr;

    // Pointer to call Post Processing methods - implementation differs for graphics or compute.
    std::unique_ptr<BinaryWorkflowStrategy> workflow_strategy_ = nullptr;

};

#endif  // RGA_RADEONGPUANALYZERCLI_SRC_KC_CLI_COMMANDER_BIN_H_
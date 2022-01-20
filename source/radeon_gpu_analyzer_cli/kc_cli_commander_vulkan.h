//=================================================================
// Copyright 2020-2021 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================
#ifndef RGA_RADEONGPUANALYZERCLI_SRC_KC_CLI_COMMANDER_VULKAN_H_
#define RGA_RADEONGPUANALYZERCLI_SRC_KC_CLI_COMMANDER_VULKAN_H_

// Local.
#include "radeon_gpu_analyzer_cli/kc_cli_commander.h"

class KcCliCommanderVulkan : public KcCliCommander
{
public:
    KcCliCommanderVulkan()  = default;
    ~KcCliCommanderVulkan() = default;

    // Perform compilation.
    virtual void  RunCompileCommands(const Config& config, LoggingCallbackFunction callback) override;

    // Perform post-compile actions.
    virtual bool RunPostCompileSteps(const Config& config) override;

    // Print the list of supported Vulkan target GPUs.
    virtual bool PrintAsicList(const Config& config) override;

    // Parse the source file and extract list of shader functions.
    // Dump the extracted entry points to stdout.
    virtual bool ListEntries(const Config& config, LoggingCallbackFunction callback) override;

    // Get the list of supported targets for the given mode (source language).
    static bool GetSupportedTargets(const Config& config, std::set<std::string>& targets, bool print_cmd = false);

    // Generates the Vulkan version info and writes it to the file specified by "fileName".
    // The Vulkan version info will be appended to the existing content of the file.
    static bool GenerateVulkanVersionInfo(const Config& config, const std::string& filename, bool print_cmd = false);

    // Generates the system level version info and writes it to the file specified by "fileName".
    // The data will be appended to the existing content of the file.
    static bool GenerateSystemVersionInfo(const Config& config, const std::string& filename, bool print_cmd = false);

private:
    // Identify the devices requested by user.
    bool InitRequestedAsicListVulkan(const Config& config);

    // Compile GLSL and HLSL source file(s) to SPIR-V binary files.
    // Note: the sets of glsl and glsl files must not intersect, i.e. each stage may have
    // either a GLSL or an HLSL file but not both.
    // Names of output SPIR-V binary files are returned in "outSpvFiles".
    // (This function calls glslang compiler).
    bool CompileSourceToSpv(const Config& conf, const BeVkPipelineFiles& glsl_files,
                            const BeVkPipelineFiles& hlsl_files, BeVkPipelineFiles& out_spv_files);

    // Compile a SPIR-V binary file to ISA disassembly file(s) and shader statistics file(s).
    // (This function invokes VulkanBackend executable).
    void CompileSpvToIsa(const Config& conf, const BeVkPipelineFiles& spvFiles);

    // Compile a SPIR-V binary file to ISA disassembly file(s) and shader statistics file(s) for specified device.
    // (This function invokes VulkanBackend executable).
    void CompileSpvToIsaForDevice(const Config& config, const BeVkPipelineFiles& spv_files,
                                  const std::string& device, bool is_physical_adapter = false);

    // Assemble a SPIR-V text file to SPIR-V binary file.
    // (This function invokes spv-as from SPIR-V Tools.)
    static bool AssembleSpv(const Config& config);

    // Disassemble a SPIR-V binary file to SPIR-V text file.
    // (This function invokes spv-dis from SPIR-V Tools.)
    static bool DisassembleSpv(const Config& config);

    // Parse a SPIR-V binary file and print info to the specified output file or stdout.
    static bool ParseSpv(const Config& config);

    // Assemble input SPIR-V text files.
    // Names of output SPIR-V binary files are returned in "outSpvFiles".
    // (This function invokes SPIR-V Tools assembler).
    bool AssembleSpvTxtInputFiles(const Config& config, const BeVkPipelineFiles& spv_txt_files, BeVkPipelineFiles& out_spv_files);

    // Generate RGA CLI session metadata file.
    bool GenerateSessionMetadata(const Config& config) const;

    // Parse ISA files and generate separate files that contain parsed ISA in CSV format.
    bool ParseIsaFilesToCSV(bool add_line_numbers);

    // Perform the live registers analysis.
    bool PerformLiveRegAnalysis(const Config& config);

    // Predict shader performance.
    bool PredictShaderPerformance(const Config& config) const;

    // Perform stall analysis.
    bool PerformStallAnalysis(const Config& config) const;

    // Generate the per-block or per-instruction Control Flow Graph.
    bool ExtractCFG(const Config& config) const;

    // Store input file names to the output metadata.
    void StoreInputFilesToOutputMD(const BeVkPipelineFiles& input_files);

    // Store output file names to the output metadata.
    void StoreOutputFilesToOutputMD(const std::string& device, const BeVkPipelineFiles& spv_files,
                                    const BeVkPipelineFiles& isa_files, const BeVkPipelineFiles& stats_files);

    // Delete temporary files.
    void DeleteTempFiles();

    // ---- Data ----

    // Selected target GPUs.
    std::set<std::string> asics_;

    // Name of the first physical adapter installed on the system.
    std::string  physical_adapter_name_;

    // Per-device output metadata.
    std::map<std::string, RgVkOutputMetadata>  output_metadata_;

    // Temporary files.
    std::vector<std::string>  temp_files_;
};

#endif // RGA_RADEONGPUANALYZERCLI_SRC_KC_CLI_COMMANDER_VULKAN_H_

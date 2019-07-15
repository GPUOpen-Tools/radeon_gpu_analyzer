//=================================================================
// Copyright 2018 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

// Local.
#include <RadeonGPUAnalyzerCLI/Src/kcCLICommander.h>

class kcCLICommanderVulkan : public kcCLICommander
{
public:
    kcCLICommanderVulkan()  = default;
    ~kcCLICommanderVulkan() = default;

    // Perform compilation.
    virtual void  RunCompileCommands(const Config& config, LoggingCallBackFunc_t callback) override;

    // Perform post-compile actions.
    virtual bool RunPostCompileSteps(const Config& config) override;

    // Print the list of supported Vulkan target GPUs.
    virtual bool PrintAsicList(const Config& config) override;

    // Parse the source file and extract list of shader functions.
    // Dump the extracted entry points to stdout.
    virtual bool ListEntries(const Config& config, LoggingCallBackFunc_t callback) override;

    // Get the list of supported targets for the given mode (source language).
    static bool GetSupportedTargets(const Config& config, std::set<std::string>& targets, bool printCmd = false);

    // Generates the Vulkan version info and writes it to the file specified by "fileName".
    // The Vulkan version info will be appended to the existing content of the file.
    static bool GenerateVulkanVersionInfo(const Config& config, const std::string& fileName, bool printCmd = false);

    // Generates the system level version info and writes it to the file specified by "fileName".
    // The data will be appended to the existing content of the file.
    static bool GenerateSystemVersionInfo(const Config& config, const std::string& fileName, bool printCmd = false);

private:
    // Identify the devices requested by user.
    bool InitRequestedAsicListVulkan(const Config& config);

    // Compile GLSL and HLSL source file(s) to SPIR-V binary files.
    // Note: the sets of glsl and glsl files must not intersect, i.e. each stage may have
    // either a GLSL or an HLSL file but not both.
    // Names of output SPIR-V binary files are returned in "outSpvFiles".
    // (This function calls glslang compiler).
    bool CompileSourceToSpv(const Config& conf, const beVkPipelineFiles& glslFiles,
                            const beVkPipelineFiles& hlslFiles, beVkPipelineFiles& outSpvFiles);

    // Compile a SPIR-V binary file to ISA disassembly file(s) and shader statistics file(s).
    // (This function invokes VulkanBackend executable).
    void CompileSpvToIsa(const Config& conf, const beVkPipelineFiles& spvFiles);

    // Compile a SPIR-V binary file to ISA disassembly file(s) and shader statistics file(s) for specified device.
    // (This function invokes VulkanBackend executable).
    void CompileSpvToIsaForDevice(const Config& conf, const beVkPipelineFiles& spvFiles,
                                  const std::string& device, bool isPhysAdapter = false);

    // Assemble a SPIR-V text file to SPIR-V binary file.
    // (This function invokes spv-as from SPIR-V Tools.)
    static bool AssembleSpv(const Config& conf);

    // Disassemble a SPIR-V binary file to SPIR-V text file.
    // (This function invokes spv-dis from SPIR-V Tools.)
    static bool DisassembleSpv(const Config& conf);

    // Parse a SPIR-V binary file and print info to the specified output file or stdout.
    static bool ParseSpv(const Config& conf);

    // Assemble input SPIR-V text files.
    // Names of output SPIR-V binary files are returned in "outSpvFiles".
    // (This function invokes SPIR-V Tools assembler).
    bool AssembleSpvTxtInputFiles(const Config& conf, const beVkPipelineFiles& spvTxtFiles, beVkPipelineFiles& outSpvFiles);

    // Generate RGA CLI session metadata file.
    bool GenerateSessionMetadata(const Config& config) const;

    // Parse ISA files and generate separate files that contain parsed ISA in CSV format.
    bool ParseIsaFilesToCSV(bool addLineNumbers);

    // Perform the live registers analysis.
    bool PerformLiveRegAnalysis(const Config& conf) const;

    // Generate the per-block or per-instruction Control Flow Graph.
    bool ExtractCFG(const Config& conf) const;

    // Store input file names to the output metadata.
    void StoreInputFilesToOutputMD(const beVkPipelineFiles& inputFiles);

    // Store output file names to the output metadata.
    void StoreOutputFilesToOutputMD(const std::string& device, const beVkPipelineFiles& spvFiles,
                                    const beVkPipelineFiles& isaFiles, const beVkPipelineFiles& statsFiles);

    // Delete temporary files.
    void DeleteTempFiles();

    // ---- Data ----

    // Selected target GPUs.
    std::set<std::string> m_asics;

    // Name of the first physical adapter installed on the system.
    std::string  m_physAdapterName;

    // Per-device output metadata.
    std::map<std::string, rgVkOutputMetadata>  m_outputMD;

    // Temporary files.
    std::vector<std::string>  m_tempFiles;
};

#pragma once

// C++.
#include <string>
#include <set>
#include <memory>
#include <unordered_map>

// Local.
#include <RadeonGPUAnalyzerCLI/Src/kcCLICommander.h>
#include <RadeonGPUAnalyzerBackend/Include/beProgramBuilderLightning.h>

using namespace std;

// Commander interface for compiling with the Lightning Compiler (LC).
class kcCLICommanderLightning : public kcCLICommander
{
public:
    // CTOR/DTOR
    kcCLICommanderLightning() = default;
    ~kcCLICommanderLightning() = default;

    // Print the driver version.
    void Version(Config& config, LoggingCallBackFunc_t callback) override;

    // Print list of supported devices.
    virtual bool PrintAsicList(const Config&) override;

    // Parse the source file and extract list of entry points (for example, kernels for OpenCL).
    // Dump the extracted entry points to stdout.
    virtual bool ListEntries(const Config& config, LoggingCallBackFunc_t callback) override;

    // Actual implementation of "ListEntries" virtual function.
    // This function is also used by legacy cl mode.
    static bool ListEntriesRocmCL(const Config& config, LoggingCallBackFunc_t callback);

    // Perform compilation steps.
    void RunCompileCommands(const Config& config, LoggingCallBackFunc_t callback) override;

    // Perform post-compile actions.
    bool RunPostCompileSteps(const Config& config) override;

    // Initialize the Commander object.
    beStatus  Init(const Config& config, LoggingCallBackFunc_t logCallback);

    // Generates rocm-cl "version info" data and writes it to the file specified by "fileName".
    // The data will be appended to the existing content of the file.
    static bool GenerateRocmVersionInfo(const std::string& fileName);

    // Get the list of names of supported targets in DeviceInfo format.
    static bool GetSupportedTargets(std::set<std::string>& targets, bool printCmd = false);

private:
    // Map  device or kernel name <--> kernel ISA text.
    typedef std::unordered_map<std::string, std::string>  IsaMap;

    // Identify the devices requested by user.
    bool  InitRequestedAsicListLC(const Config& config);

    // Perform requested compilation.
    bool  Compile(const Config& config);

    // Perform OpenCL compilation.
    beStatus  CompileOpenCL(const Config& config, const OpenCLOptions& oclOptions);

    // Dissasemble binary file.
    // The disassembled ISA text are be divided into per-kernel parts and stored in separate files.
    // The names of ISA files are generated based on provided user ISA file name.
    beStatus DisassembleBinary(const std::string& binFileName, const std::string& userIsaFileName, const std::string& clangDevice, const std::string& rgaDevice,
                                const std::string& kernel, bool lineNumbers, std::string& errText);

    // Parse ISA files and generate separate files that contain parsed ISA in CSV format.
    bool  ParseIsaFilesToCSV(bool addLineNumbers);

    // Add the device name to the output file name provided in "outFileName" string.
    beStatus  AdjustBinaryFileName(const Config&       config,
                                   const std::string & device,
                                   std::string&        binFileName);

    // Perform the live VGPR analysis.
    bool  PerformLiveRegAnalysis(const Config& config);

    // Extract program Control Flow Graph.
    bool  ExtractCFG(const Config& config);

    // Get the AMD GPU metadata from the binary.
    beStatus  ExtractMetadata(const std::string& metadataFileName) const;

    // Extract Resource Usage (statistics) data.
    beStatus  ExtractStatistics(const Config& config);

    // Split ISA text into separate per-kernel ISA fragments and store them into separate files.
    // Puts the names of generated ISA files into output metadata (kcCLICommander::m_outputMetadata).
    bool  SplitISA(const std::string& binFile, const std::string& isaText,
                   const std::string& userIsaFileName, const std::string& device,
                   const std::string& kernel, const std::vector<std::string>& kernelNames);

    // Split ISA text into separate per-kernel ISA fragments. The fragments are returned in the
    // "kernelIsaTextMap" map.
    bool  SplitISAText(const std::string& isaText,
                       const std::vector<std::string>& kernelNames,
                       IsaMap& kernelIsaTextMap) const;

    // Remove unused code from the ISA disassembly.
    bool  ReduceISA(const std::string& binFile, IsaMap& kernelIsaTexts);

    // Generate RGA CLI session metadata file.
    bool  GenerateSessionMetadata(const Config& config) const;

    // Extract the list of entry points from the source file specified by "fileName".
    static bool  ExtractEntries(const std::string& fileName, const Config& config, rgEntryData& entryData);

    // Delete all temporary files created by RGA.
    void DeleteTempFiles() const;

    // ---- DATA ----

    // Output Metadata
    rgCLOutputMetadata  m_outputMetadata;

    // Alternative compiler paths specified by a user.
    CmplrPaths   m_cmplrPaths;

    // Chosen target GPUs.
    std::set<std::string>  m_targets;

    // Specifies whether the "-#" option (print commands) is enabled.
    bool  m_printCmds;
};

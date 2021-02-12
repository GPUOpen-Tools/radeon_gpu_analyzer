//=================================================================
// Copyright 2020 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================
#ifndef RGA_RADEONGPUANALYZERCLI_SRC_KC_UTILS_H_
#define RGA_RADEONGPUANALYZERCLI_SRC_KC_UTILS_H_

// C++.
#include <string>
#include <sstream>
#include <map>

// Infra.
#ifdef _WIN32
    #pragma warning(push)
    #pragma warning(disable:4309)
#endif
#include "AMDTBaseTools/Include/gtString.h"
#ifdef _WIN32
    #pragma warning(pop)
#endif

// Backend.
#include "RadeonGPUAnalyzerBackend/Src/be_data_types.h"
#include "RadeonGPUAnalyzerBackend/Src/be_include.h"

// Local.
#include "RadeonGPUAnalyzerCLI/Src/kc_data_types.h"
#include "RadeonGPUAnalyzerCLI/Src/kc_config.h"
#include "RadeonGPUAnalyzerCLI/Src/kc_statistics_parser.h"

// Constants.
static const unsigned long kProcessWaitInfinite = 0xFFFFFFFF;

class KcUtils
{
public:
    // Map "architecture name <--> list of marketing device names".
    typedef std::map<std::string, std::set<std::string>> DeviceNameMap;

    // Status of process launch.
    enum class ProcessStatus
    {
        kSuccess,
        kLaunchFailed,
        kTimeOut,
        kCreateTempFileFailed,
        kReadTempFileFailed
    };

    // Helper function to validate a shader's file name and generate the appropriate output message.
    // shader_type: the stage of the shader (vertex, tessellation control, tessellation evaluation, geometry, fragment or compute).
    // shader_filename: the file name to be validated.
    // log_msg: the stream to which the output should be directed.
    // Returns true if the file name is valid, and false otherwise.
    static bool ValidateShaderFileName(const char* shader_type, const std::string& shader_filename, std::stringstream& log_msg);

    // Helper function to validate an output file's directory and generate the appropriate output message.
    // output_filename: the file name to be validated.
    // log_msg: the stream to which the output should be directed.
    // Returns true if the directory is valid, and false otherwise.
    static bool ValidateShaderOutputDir(const std::string& output_filename, std::stringstream& log_msg);

    // Helper function to construct the output file name in Analyzer CLI's output format, which combines
    // the base output file name, the target device name and the rendering pipeline stage.
    // Returns true if succeeded and false in case of error.
    static bool AdjustRenderingPipelineOutputFileNames(const std::string& base_output_filename, const std::string& default_suffix,
                                                       const std::string& default_extension, const std::string& device, BeProgramPipeline& pipeline_files);

    // Creates the statistics file according to the user's configuration.
    // config: the user's configuration
    // analysis_data: a map that contains each device's statistics data
    // callback: the log callback
    static bool CreateStatisticsFile(const gtString& filename, const Config& config,
                                     const std::map<std::string, beKA::AnalysisData>& analysis_data, LoggingCallbackFunction callback);

    // Creates the statistics file according to the user's configuration.
    // filename: the target statistics file name
    // config: the user's configuration.
    // device: the target device
    // analysisData: the device's statistics data.
    // logCallback: the log callback
    static void CreateStatisticsFile(const gtString& filename, const Config& config,
                                     const std::string& device, const beKA::AnalysisData& analysis_data, LoggingCallbackFunction callback);

    // Generates the CLI statistics file header.
    // csv_separator - the character that is being used
    static std::string GetStatisticsCsvHeaderString(char csv_separator);

    // Returns the CLI statistics CSV separator, according to the user's configuration.
    static char GetCsvSeparator(const Config& config);

    // Converts the device statistics to a CSV string.
    static std::string DeviceStatisticsToCsvString(const Config& config, const std::string& device, const beKA::AnalysisData& statistics);

    // Deletes the a file from the file system.
    // file_full_path - the full path to the file to be deleted.
    static bool DeleteFile(const gtString& file_full_path);

    // Deletes the a file from the file system.
    // fileFullPath - the full path to the file to be deleted.
    static bool DeleteFile(const std::string& file_full_path);

    // Returns true if the given string represents a directory path, otherwise returns false.
    static bool IsDirectory(const std::string& dir_path);

    // Replaces a backend statistics file with a CLI statistics file.
    // statistics_file - full path to the file to be replaced
    // config - user configuration
    // device - the name of the device for which the statistics where generated
    // stats_parser - a parser to be used to parse the backend raw statistics file
    // callback - a log callback
    static void ReplaceStatisticsFile(const gtString& statistics_file, const Config& config, const std::string& device,
                                      IStatisticsParser& stats_parser, LoggingCallbackFunction callback);

    // Performs live register analysis for the ISA in the given file, and dumps
    // the output to the given output file name.
    // isa_filename - the disassembled ISA file name
    // output_filename - the output file name
    // callback - callback to log messages
    // print_cmd - print command line to stdout
    static bool PerformLiveRegisterAnalysis(const gtString& isa_filename, const gtString& output_filename,
                                            LoggingCallbackFunction callback, bool print_cmd);

    // Performs live register analysis for the ISA in the given file, and dumps
    // the output to the given output file name.
    // isa_filename - the disassembled ISA file name
    // output_filename - the output file name
    // callback - callback to log messages
    // print_cmd - print command line to stdout
    static bool PerformLiveRegisterAnalysis(const std::string& isa_filename, const std::string& output_filename,
        LoggingCallbackFunction callback, bool print_cmd);

    // Generates control flow graph for the given ISA.
    // isa_file_name - the disassembled ISA file name
    // outputFileName - the output file name
    // callback - callback to log messages
    // print_cmd - print command line to stdout
    static bool GenerateControlFlowGraph(const gtString& isa_file_name, const gtString& outputFileName,
                                         LoggingCallbackFunction callback, bool perInstCfg, bool print_cmd);

    // Generates control flow graph for the given ISA.
    // isa_file_name - the disassembled ISA file name
    // outputFileName - the output file name
    // callback - callback to log messages
    // print_cmd - print command line to stdout
    static bool GenerateControlFlowGraph(const std::string& isa_file_name, const std::string& outputFileName,
        LoggingCallbackFunction callback, bool perInstCfg, bool print_cmd);

    // Generates an output file name in the Analyzer CLI format.
    // base_output_filename - the base output file name as configured by the user's command
    // default_extension - default extension to use if user did not specify an extension for the output file
    // entry_point_name - the name of the entry point to which the output file refers (can be empty if not relevant)
    // device_name - the name of the target device to which the output file refers (can be empty if not relevant)
    // generated_filename - an output variable to hold the generated file name
    static void ConstructOutputFileName(const std::string& base_output_filename, const std::string& default_suffix,
                                        const std::string& default_extension, const std::string& entry_point_name,
                                        const std::string& device_name, gtString& generated_filename);

    // std::string version of ConstructOutputFileName().
    static void ConstructOutputFileName(const std::string& base_output_filename, const std::string& default_suffix,
                                        const std::string& default_extension, const std::string& entry_point_name,
                                        const std::string& device_name, std::string& generated_filename);

    // Construct output file name based on provided base name, device name and extension.
    // The file name is the full path.
    // If base name is empty, the (temp folder name + temp file name) will be used.
    static bool ConstructOutFileName(const std::string& baseFileName,
        const std::string& stage,
        const std::string& device,
        const std::string& ext,
        std::string& out_filename,
        bool should_append_suffix = true);

    // Append suffix to the provided file name.
    // If the file name has an extension, the suffix will be appended before the extension.
    static void AppendSuffix(std::string& filename, const std::string& suffix);

    // Generates a name for a temporary file in the OS temp directory. Makes sure the file with generated name
    // does not exist.
    // The generated name starts with the "prefix" and ends with the "ext".
    // Returns full path including the file name.
    static gtString ConstructTempFileName(const gtString& prefix, const gtString& ext);

    // std::string version of ConstructTempFileName().
    static std::string ConstructTempFileName(const std::string& prefix, const std::string& ext);

    // Get all available graphics cards public names, grouped by the internal code name.
    static bool GetMarketingNameToCodenameMapping(DeviceNameMap& cards_map);

    // Tries to find a GPU architecture name that corresponds to the device name provided by user ("device").
    // Returns "true" if corresponding arch is found, "false" otherwise.
    // Returns matched architecture in "matched_device" (if succeeded).
    // If "print_info" is true, the detected device or error message will be printed to stdout/stderr.
    // If "allow_unknown_device" is true, no error message will be printed if "device" is not found in the list of known devices.
    static bool FindGPUArchName(const std::string& device, std::string& matched_device, bool print_info, bool allow_unknown_device);

    // Check if file exists and not empty.
    // \param[in]  filename    name of file.
    // \returns                "true" if file exists and not empty, "false" otherwise.
    static bool FileNotEmpty(const std::string filename);

    // Read named file into a string.
    // \param[in]  input_file     name of file.
    // \param[out] program_source content of file.
    // \returns                  success.
    static bool ReadProgramSource(const std::string& input_file, std::string& program_source);

    // Write a binary file.
    // \param[in]  filename   the name of the file to be created
    // \param[in]  content    the contents
    // \param[in]  callback  callback for logging
    static bool WriteBinaryFile(const std::string& filename, const std::vector<char>& content, LoggingCallbackFunction callback);

    // Read from a text file.
    // \param[in]  filename   the name of the file to read from
    // \param[in]  content    the contents
    // \param[in]  callback  callback for logging
    static bool ReadTextFile(const std::string& filename, std::string& content, LoggingCallbackFunction callback);

    // Write a text file.
    // \param[in]  filename   the name of the file to be created
    // \param[in]  content    the contents
    // \param[in]  callback  callback for logging
    static bool WriteTextFile(const std::string& filename, const std::string& content, LoggingCallbackFunction callback);

    // Prints the list of required devices, or the names of the devices that appear
    // in the given set in case that the required devices set is empty.
    // Does not print the devices that are present in the "disabledDevices" set.
    static bool PrintAsicList(const std::set<std::string>& required_devices = std::set<std::string>(),
                              const std::set<std::string>& disabled_devices = std::set<std::string>());

    // Constructs full path for RGA log file.
    // Returns "true" if succeeded or "false" otherwise.
    static bool GetLogFileName(std::string& log_filename);

    // Constructs a name for parsed ISA based on provided ISA file name.
    // Returns "true" if succeeded or "false" otherwise.
    static bool GetParsedISAFileName(const std::string& isa_file_name, std::string& parsed_isa_file_name);

    // Quote the provided string if it contains spaces.
    static std::string Quote(const std::string& str);

    // Converts the given string to its lower-case version.
    static std::string ToLower(const std::string& str);

    // Delete the files specified in pipeline structure.
    static void DeletePipelineFiles(const BeProgramPipeline& files);

    // Returns the list of devices that are disabled for all modes.
    static const std::vector<std::string>  GetRgaDisabledDevices();

    // Prints the version of RGA backend (for all modes).
    static void  PrintRgaVersion();

    // Launch a process with provided executable name and command line arguments.
    // \param[in]  exec_path   the executable path
    // \param[in]  args        command line arguments
    // \param[in]  dir         working directory for the process. If empty string is provided, the current directory will be used.
    // \param[in]  time_out    process time out in milliseconds
    // \param[in]  print_cmd   print the command + arguments to be launched.
    // \param[out] std_out     the content of stdout stream dumped by launched process
    // \param[out] std_err     the content of stderr stream dumped by launched process
    // \param[out] exit_code   the exit code returned by launched process
    // Returns status of process launch.
    static ProcessStatus LaunchProcess(const std::string& exec_path, const std::string& args, const std::string& dir,
                                       unsigned long time_out, bool print_cmd, std::string& std_out, std::string& std_err, long& exit_code);

    // Open new CLI log file and delete old files (older than 1 week).
    static bool  InitCLILogFile(const Config& config);

    // Case-insensitive string compare.
    // Returns "true" if provided strings are equal or "false" otherwise.
    static bool  StrCmpNoCase(const std::string& s1, const std::string& s2);

    // Returns file extension.
    static std::string GetFileExtension(const std::string& file_path);

    // Set environment variable varName to varValue.
    // Returns true on success, false otherwise.
    static bool SetEnvrironmentVariable(const::std::string& var_name, const std::string var_value);

    // Checks for available updates.
    static void CheckForUpdates();

    // Returns true if the target is of the Navi generation and false otherwise.
    static bool IsNaviTarget(const std::string& target_name);

    // Returns true if the target is Navi21 or beyond and false otherwise.
    static bool IsNavi21AndBeyond(const std::string& target_name);

    // Returns true if the target is of the Vega generation and false otherwise.
    static bool IsVegaTarget(const std::string& target_name);

    // Returns true if the given disassembly is supported by the static analysis engine.
    static bool IsPostPorcessingSupported(const std::string& isa_file_path);

    // Returns true if the given disassembly has the LLPC tokens.
    static bool IsLlpcDisassembly(const std::string& isa_file_path);

    // Adjust base file name for binary output file.
    static std::string AdjustBaseFileNameBinary(const std::string& user_input_filename, const std::string& device);

    // Adjust base file name for ISA output file.
    static std::string AdjustBaseFileNameIsaDisassembly(const std::string& user_input_filename, const std::string& device);

    // Adjust base file name for IL disassembly output file.
    static std::string AdjustBaseFileNameIlDisassembly(const std::string& user_input_filename, const std::string& device);

    // Adjust base file name for statistics output file.
    static std::string AdjustBaseFileNameStats(const std::string& user_input_filename, const std::string& device);

    // Adjust base file name for live register analysis output file.
    static std::string AdjustBaseFileNameLivereg(const std::string& user_input_filename, const std::string& device);

    // Adjust base file name for cfg output file.
    static std::string AdjustBaseFileNameCfg(const std::string& user_input_filename, const std::string& device);

private:
    // This is a static class (no instances).
    KcUtils(const KcUtils& other);
    KcUtils()  = default;
    ~KcUtils() = default;
};

#endif // RGA_RADEONGPUANALYZERCLI_SRC_KC_UTILS_H_

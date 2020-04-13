//=================================================================
// Copyright 2017 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

#pragma once

// C++.
#include <string>
#include <sstream>
#include <map>

// Infra.
#ifdef _WIN32
    #pragma warning(push)
    #pragma warning(disable:4309)
#endif
#include <AMDTBaseTools/Include/gtString.h>
#ifdef _WIN32
    #pragma warning(pop)
#endif

// Backend.
#include <RadeonGPUAnalyzerBackend/Include/beDataTypes.h>
#include <RadeonGPUAnalyzerBackend/Include/beInclude.h>

// Local.
#include <RadeonGPUAnalyzerCLI/Src/kcDataTypes.h>
#include <RadeonGPUAnalyzerCLI/Src/kcConfig.h>
#include <RadeonGPUAnalyzerCLI/Src/kcIStatisticsParser.h>

// Constants.
static const  unsigned long  PROCESS_WAIT_INFINITE = 0xFFFFFFFF;

//
// kcUtils class.
//
class kcUtils
{
public:
    // Map "architecture name <--> list of marketing device names".
    typedef  std::map<std::string, std::set<std::string>>  DeviceNameMap;

    // Status of process launch.
    enum class ProcessStatus
    {
        Success,
        LaunchFailed,
        TimeOut,
        CreateTempFileFailed,
        ReadTempFileFailed
    };

    // Helper function to validate a shader's file name and generate the appropriate output message.
    // shaderType: the stage of the shader (vertex, tessellation control, tessellation evaluation, geometry, fragment or compute).
    // shaderFileName: the file name to be validated.
    // logMsg: the stream to which the output should be directed.
    // Returns true if the file name is valid, and false otherwise.
    static bool ValidateShaderFileName(const char* shaderType, const std::string& shaderFileName, std::stringstream& logMsg);

    // Helper function to validate an output file's directory and generate the appropriate output message.
    // outputFileName: the file name to be validated.
    // logMsg: the stream to which the output should be directed.
    // Returns true if the directory is valid, and false otherwise.
    static bool ValidateShaderOutputDir(const std::string& outputFileName, std::stringstream& logMsg);

    // Helper function to construct the output file name in Analyzer CLI's output format, which combines
    // the base output file name, the target device name and the rendering pipeline stage.
    // Returns true if succeeded and false in case of error.
    static bool AdjustRenderingPipelineOutputFileNames(const std::string& baseOutputFileName, const std::string& defaultSuffix,
                                                       const std::string& defaultExt, const std::string& device, beProgramPipeline& pipelineFiles);

    // Creates the statistics file according to the user's configuration.
    // config: the user's configuration
    // analysisData: a map that contains each device's statistics data
    // // logCallback: the log callback
    static bool CreateStatisticsFile(const gtString& fileName, const Config& config,
                                     const std::map<std::string, beKA::AnalysisData>& analysisData, LoggingCallBackFunc_t logCallback);

    // Creates the statistics file according to the user's configuration.
    // fileName: the target statistics file name
    // config: the user's configuration.
    // device: the target device
    // analysisData: the device's statistics data.
    // logCallback: the log callback
    static void CreateStatisticsFile(const gtString& fileName, const Config& config,
                                     const std::string& device, const beKA::AnalysisData& analysisData, LoggingCallBackFunc_t logCallback);

    // Generates the CLI statistics file header.
    // csvSeparator - the character that is being used
    static std::string GetStatisticsCsvHeaderString(char csvSeparator);

    // Returns the CLI statistics CSV separator, according to the user's configuration.
    static char GetCsvSeparator(const Config& config);

    // Converts the device statistics to a CSV string.
    static std::string DeviceStatisticsToCsvString(const Config& config, const std::string& device, const beKA::AnalysisData& statistics);

    // Deletes the a file from the file system.
    // fileFullPath - the full path to the file to be deleted.
    static bool DeleteFile(const gtString& fileFullPath);

    // Deletes the a file from the file system.
    // fileFullPath - the full path to the file to be deleted.
    static bool DeleteFile(const std::string& fileFullPath);

    // Replaces a backend statistics file with a CLI statistics file.
    // statisticsFile - full path to the file to be replaced
    // config - user configuration
    // device - the name of the device for which the statistics where generated
    // statsParser - a parser to be used to parse the backend raw statistics file
    // logCb - a log callback
    static void ReplaceStatisticsFile(const gtString& statisticsFile, const Config& config, const std::string& device,
                                      IStatisticsParser& statsParser, LoggingCallBackFunc_t logCb);

    // Performs live register analysis for the ISA in the given file, and dumps
    // the output to the given output file name.
    // isaFileName - the disassembled ISA file name
    // outputFileName - the output file name
    // pCallback - callback to log messages
    // printCmd - print command line to stdout
    static bool PerformLiveRegisterAnalysis(const gtString& isaFileName, const gtString& outputFileName,
                                            LoggingCallBackFunc_t pCallback, bool printCmd);

    // Performs live register analysis for the ISA in the given file, and dumps
    // the output to the given output file name.
    // isaFileName - the disassembled ISA file name
    // outputFileName - the output file name
    // pCallback - callback to log messages
    // printCmd - print command line to stdout
    static bool PerformLiveRegisterAnalysis(const std::string& isaFileName, const std::string& outputFileName,
        LoggingCallBackFunc_t pCallback, bool printCmd);

    // Generates control flow graph for the given ISA.
    // isaFileName - the disassembled ISA file name
    // outputFileName - the output file name
    // pCallback - callback to log messages
    // printCmd - print command line to stdout
    static bool GenerateControlFlowGraph(const gtString& isaFileName, const gtString& outputFileName,
                                         LoggingCallBackFunc_t pCallback, bool perInstCfg, bool printCmd);

    // Generates control flow graph for the given ISA.
    // isaFileName - the disassembled ISA file name
    // outputFileName - the output file name
    // pCallback - callback to log messages
    // printCmd - print command line to stdout
    static bool GenerateControlFlowGraph(const std::string& isaFileName, const std::string& outputFileName,
        LoggingCallBackFunc_t pCallback, bool perInstCfg, bool printCmd);

    // Generates an output file name in the Analyzer CLI format.
    // baseOutputFileName - the base output file name as configured by the user's command
    // defaultExtension - default extension to use if user did not specify an extension for the output file
    // entryPointName - the name of the entry point to which the output file refers (can be empty if not relevant)
    // deviceName - the name of the target device to which the output file refers (can be empty if not relevant)
    // generatedFileName - an output variable to hold the generated file name
    static void ConstructOutputFileName(const std::string& baseOutputFileName, const std::string& defaultSuffix,
                                        const std::string& defaultExtension, const std::string& entryPointName,
                                        const std::string& deviceName, gtString& generatedFileName);

    // std::string version of ConstructOutputFileName().
    static void ConstructOutputFileName(const std::string& baseOutputFileName, const std::string& defaultSuffix,
                                        const std::string& defaultExtension, const std::string& entryPointName,
                                        const std::string& deviceName, std::string& generatedFileName);

    // Construct output file name based on provided base name, device name and extension.
    // The file name is the full path.
    // If base name is empty, the (temp folder name + temp file name) will be used.
    static bool ConstructOutFileName(const std::string& baseFileName,
        const std::string& stage,
        const std::string& device,
        const std::string& ext,
        std::string&       outFileName);

    // Append suffix to the provided file name.
    // If the file name has an extension, the suffix will be appended before the extension.
    static void AppendSuffix(std::string& fileName, const std::string& suffix);

    // Generates a name for a temporary file in the OS temp directory. Makes sure the file with generated name
    // does not exist.
    // The generated name starts with the "prefix" and ends with the "ext".
    // Returns full path including the file name.
    static gtString ConstructTempFileName(const gtString& prefix, const gtString& ext);

    // std::string version of ConstructTempFileName().
    static std::string ConstructTempFileName(const std::string& prefix, const std::string& ext);

    // Get all available graphics cards public names, grouped by the internal code name.
    static bool GetMarketingNameToCodenameMapping(DeviceNameMap& cardsMap);

    // Tries to find a GPU architecture name that corresponds to the device name provided by user ("device").
    // Returns "true" if corresponding arch is found, "false" otherwise.
    // Returns matched architecture in "matchedDevice" (if succeeded).
    // If "printInfo" is true, the detected device or error message will be printed to stdout/stderr.
    // If "allowUnknownDevice" is true, no error message will be printed if "device" is not found in the list of known devices.
    static bool FindGPUArchName(const std::string& device, std::string& matchedDevice, bool printInfo, bool allowUnknownDevice);

    // Check if file exists and not empty.
    // \param[in]  fileName    name of file.
    // \returns                "true" if file exists and not empty, "false" otherwise.
    static bool FileNotEmpty(const std::string fileName);

    // Read named file into a string.
    // \param[in]  inputFile     name of file.
    // \param[out] programSource content of file.
    // \returns                  success.
    static bool ReadProgramSource(const std::string& inputFile, std::string& programSource);

    // Write a binary file.
    // \param[in]  fileName   the name of the file to be created
    // \param[in]  content    the contents
    // \param[in]  pCallback  callback for logging
    static bool WriteBinaryFile(const std::string& fileName, const std::vector<char>& content, LoggingCallBackFunc_t pCallback);

    // Read from a text file.
    // \param[in]  fileName   the name of the file to read from
    // \param[in]  content    the contents
    // \param[in]  pCallback  callback for logging
    static bool ReadTextFile(const std::string& fileName, std::string& content, LoggingCallBackFunc_t pCallback);

    // Write a text file.
    // \param[in]  fileName   the name of the file to be created
    // \param[in]  content    the contents
    // \param[in]  pCallback  callback for logging
    static bool WriteTextFile(const std::string& fileName, const std::string& content, LoggingCallBackFunc_t pCallback);

    // Prints the list of required devices, or the names of the devices that appear
    // in the given set in case that the required devices set is empty.
    // Does not print the devices that are present in the "disabledDevices" set.
    static bool PrintAsicList(const std::set<std::string>& requiredDevices = std::set<std::string>(),
                              const std::set<std::string>& disabledDevices = std::set<std::string>());

    // Constructs full path for RGA log file.
    // Returns "true" if succeeded or "false" otherwise.
    static bool GetLogFileName(std::string& logFileName);

    // Constructs a name for parsed ISA based on provided ISA file name.
    // Returns "true" if succeeded or "false" otherwise.
    static bool GetParsedISAFileName(const std::string& isaFileName, std::string& parsedIsaFileName);

    // Quote the provided string if it contains spaces.
    static std::string Quote(const std::string& str);

    // Converts the given string to its lower-case version.
    static std::string ToLower(const std::string& str);

    // Delete the files specified in pipeline structure.
    static void DeletePipelineFiles(const beProgramPipeline& files);

    // Returns the list of devices that are disabled for all modes.
    static const std::vector<std::string>  GetRgaDisabledDevices();

    // Prints the version of RGA backend (for all modes).
    static void  PrintRgaVersion();

    // Launch a process with provided executable name and command line arguments.
    // \param[in]  execPath    the executable path
    // \param[in]  args        command line arguments
    // \param[in]  dir         working directory for the process. If empty string is provided, the current directory will be used.
    // \param[in]  timeOut     process time out in milliseconds
    // \param[in]  printCmd    print the command + arguments to be launched.
    // \param[out] stdOut      the content of stdout stream dumped by launched process
    // \param[out] stdErr      the content of stderr stream dumped by launched process
    // \param[out] exitCode    the exit code returned by launched process
    // Returns status of process launch.
    static ProcessStatus LaunchProcess(const std::string& execPath, const std::string& args, const std::string& dir,
                                       unsigned long timeOut, bool printCmd, std::string& stdOut, std::string& stdErr, long& exitCode);

    // Open new CLI log file and delete old files (older than 1 week).
    static bool  InitCLILogFile(const Config& config);

    // Case-insensitive string compare.
    // Returns "true" if provided strings are equal or "false" otherwise.
    static bool  StrCmpNoCase(const std::string& s1, const std::string& s2);

    // Returns file extension.
    static std::string  GetFileExt(const std::string& filePath);

    // Set environment variable varName to varValue.
    // Returns true on success, false otherwise.
    static bool SetEnvrironmentVariable(const::std::string& varName, const std::string varValue);

    // Checks for available updates.
    static void CheckForUpdates();

    // Returns true if the target is of the Navi generation and false otherwise.
    static bool IsNaviTarget(const std::string& targetName);

    // Returns true if the target is of the Vega generation and false otherwise.
    static bool IsVegaTarget(const std::string& targetName);

    // Returns true if the given disassembly is supported by the static analysis engine.
    static bool IsPostPorcessingSupported(const std::string& isaFilePath);

    // Adjust base file name for binary output file.
    static std::string AdjustBaseFileNameBinary(const std::string& userInputFileName, const std::string& device);

    // Adjust base file name for ISA output file.
    static std::string AdjustBaseFileNameIsaDisassembly(const std::string& userInputFileName, const std::string& device);

    // Adjust base file name for IL disassembly output file.
    static std::string AdjustBaseFileNameIlDisassembly(const std::string& userInputFileName, const std::string& device);

    // Adjust base file name for statistics output file.
    static std::string AdjustBaseFileNameStats(const std::string& userInputFileName, const std::string& device);

    // Adjust base file name for live register analysis output file.
    static std::string AdjustBaseFileNameLivereg(const std::string& userInputFileName, const std::string& device);

    // Adjust base file name for cfg output file.
    static std::string AdjustBaseFileNameCfg(const std::string& userInputFileName, const std::string& device);

private:
    // This is a static class (no instances).
    kcUtils(const kcUtils& other);
    kcUtils()  = default;
    ~kcUtils() = default;
};

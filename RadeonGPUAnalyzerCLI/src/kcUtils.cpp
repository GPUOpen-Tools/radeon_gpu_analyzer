//=================================================================
// Copyright 2017 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

// XML.
#include <tinyxml2/Include/tinyxml2.h>

// Infra.
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osFile.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osProcess.h>

// Backend.
#include <RadeonGPUAnalyzerBackend/include/beStaticIsaAnalyzer.h>
#include <RadeonGPUAnalyzerBackend/include/beUtils.h>

// Local.
#include <RadeonGPUAnalyzerCLI/src/kcUtils.h>
#include <RadeonGPUAnalyzerCLI/src/kcCliStringConstants.h>
#include <RadeonGPUAnalyzerCLI/src/kcCLICommander.h>
#include <Utils/include/rgaXMLConstants.h>
#include <Utils/include/rgaSharedUtils.h>
#include <Utils/include/rgLog.h>


// Shared between CLI and GUI.
#include <RadeonGPUAnalyzerCLI/../Utils/include/rgaVersionInfo.h>

#ifndef _WIN32
#pragma GCC diagnostic ignored "-Wunused-variable"
#include <dirent.h>
#endif

using namespace beKA;

// Static constants.
static const gtString  RGA_CLI_LOG_FILE_NAME           = L"rga_cli";
static const gtString  RGA_CLI_LOG_FILE_EXT            = L"log";
static const gtString  RGA_CLI_PARSED_ISA_FILE_EXT     = L"csv";
static const gtString  RGA_CLI_TEMP_STDOUT_FILE_NAME   = L"rga_stdout";
static const gtString  RGA_CLI_TEMP_STDOUT_FILE_EXT    = L"txt";
static const gtString  RGA_CLI_TEMP_STDERR_FILE_NAME   = L"rga_stderr";
static const gtString  RGA_CLI_TEMP_STDERR_FILE_EXT    = L"txt";

#ifdef WIN32
const int  WINDOWS_DATE_STRING_LEN          = 14;
const int  WINDOWS_DATE_STRING_YEAR_OFFSET  = 10;
const int  WINDOWS_DATE_STRING_MONTH_OFFSET =  7;
const int  WINDOWS_DATE_STRING_DAY_OFFSET   =  4;
#endif

static const std::vector<std::string>  RGA_DISABLED_DEVICES = { };

static bool GetRGATempDir(osDirectory & dir);

bool kcUtils::ValidateShaderFileName(const char* shaderType, const std::string& shaderFileName, std::stringstream& logMsg)
{
    bool isShaderNameValid = true;
    gtString shaderFileNameAsGtStr;
    shaderFileNameAsGtStr << shaderFileName.c_str();
    osFilePath shaderFile(shaderFileNameAsGtStr);

    if (!shaderFile.exists())
    {
        isShaderNameValid = false;
        logMsg << STR_ERR_CANNOT_FIND_SHADER_PREFIX << shaderType << STR_ERR_CANNOT_FIND_SHADER_SUFFIX << shaderFileName << std::endl;
    }

    return isShaderNameValid;
}

bool kcUtils::ValidateShaderOutputDir(const std::string& outputFileName, std::stringstream& logMsg)
{
    bool isShaderOutputDirValid = true;
    gtString shaderFileNameAsGtStr;
    shaderFileNameAsGtStr << outputFileName.c_str();
    osFilePath shaderFile(shaderFileNameAsGtStr);
    osDirectory outputDir;
    shaderFile.getFileDirectory(outputDir);

    // If the directory is empty then we assume the output directory is the active directory which should exist.
    isShaderOutputDirValid = outputDir.asString().isEmpty() || outputDir.exists();

    if (!isShaderOutputDirValid)
    {
        logMsg << STR_ERR_CANNOT_FIND_OUTPUT_DIR << outputDir.directoryPath().asString().asASCIICharArray() << std::endl;
    }

    return isShaderOutputDirValid;
}


bool kcUtils::AdjustRenderingPipelineOutputFileNames(const std::string& baseOutputFileName, const std::string& defaultExt,
                                                     const std::string& device, beProgramPipeline& pipelineFiles)
{
    // Clear the existing pipeline.
    pipelineFiles.ClearAll();

    // Isolate the original file name.
    gtString outputFileAsGtStr;
    outputFileAsGtStr << baseOutputFileName.c_str();
    osFilePath outputFilePath(outputFileAsGtStr);

    if (!baseOutputFileName.empty() && outputFilePath.isDirectory())
    {
        osDirectory outputDir(outputFileAsGtStr);
        outputFilePath.setFileDirectory(outputDir);
        outputFilePath.clearFileName();
        outputFilePath.clearFileExtension();
    }

    osDirectory outputDir;
    bool  status = true;

    // Use system temp folder if no path is provided by a user.
    if (baseOutputFileName.empty())
    {
        status = GetRGATempDir(outputDir);
    }
    else
    {
        outputFilePath.getFileDirectory(outputDir);
    }

    if (status)
    {
        // File name.
        gtString originalFileName;
        outputFilePath.getFileName(originalFileName);

        // File extension.
        gtString originalFileExtension;
        outputFilePath.getFileExtension(originalFileExtension);

        // Make the adjustments.
        gtString fixedFileName;
        fixedFileName << outputDir.directoryPath().asString(true) << device.c_str() << "_";
        pipelineFiles.m_vertexShader << fixedFileName << KA_CLI_STR_VERTEX_ABBREVIATION;
        pipelineFiles.m_tessControlShader << fixedFileName << KA_CLI_STR_TESS_CTRL_ABBREVIATION;
        pipelineFiles.m_tessEvaluationShader << fixedFileName << KA_CLI_STR_TESS_EVAL_ABBREVIATION;
        pipelineFiles.m_geometryShader << fixedFileName << KA_CLI_STR_GEOMETRY_ABBREVIATION;
        pipelineFiles.m_fragmentShader << fixedFileName << KA_CLI_STR_FRAGMENT_ABBREVIATION;
        pipelineFiles.m_computeShader << fixedFileName << KA_CLI_STR_COMPUTE_ABBREVIATION;

        if (!originalFileName.isEmpty())
        {
            pipelineFiles.m_vertexShader << "_" << originalFileName.asASCIICharArray();
            pipelineFiles.m_tessControlShader << "_" << originalFileName.asASCIICharArray();
            pipelineFiles.m_tessEvaluationShader << "_" << originalFileName.asASCIICharArray();
            pipelineFiles.m_geometryShader << "_" << originalFileName.asASCIICharArray();
            pipelineFiles.m_fragmentShader << "_" << originalFileName.asASCIICharArray();
            pipelineFiles.m_computeShader << "_" << originalFileName.asASCIICharArray();
        }

        pipelineFiles.m_vertexShader << "." << (originalFileExtension.isEmpty() ? defaultExt.c_str() : originalFileExtension.asASCIICharArray());
        pipelineFiles.m_tessControlShader << "." << (originalFileExtension.isEmpty() ? defaultExt.c_str() : originalFileExtension.asASCIICharArray());
        pipelineFiles.m_tessEvaluationShader << "." << (originalFileExtension.isEmpty() ? defaultExt.c_str() : originalFileExtension.asASCIICharArray());
        pipelineFiles.m_geometryShader << "." << (originalFileExtension.isEmpty() ? defaultExt.c_str() : originalFileExtension.asASCIICharArray());
        pipelineFiles.m_fragmentShader << "." << (originalFileExtension.isEmpty() ? defaultExt.c_str() : originalFileExtension.asASCIICharArray());
        pipelineFiles.m_computeShader << "." << (originalFileExtension.isEmpty() ? defaultExt.c_str() : originalFileExtension.asASCIICharArray());
    }

    return status;
}

std::string kcUtils::DeviceStatisticsToCsvString(const Config& config, const std::string& device, const beKA::AnalysisData& statistics)
{
    std::stringstream output;

    // Device name.
    char csvSeparator = GetCsvSeparator(config);
    output << device << csvSeparator;

    // Scratch registers.
    output << statistics.scratchMemoryUsed << csvSeparator;

    // Work-items per work-group.
    output << statistics.numThreadPerGroup << csvSeparator;

    // Wavefront size.
    output << statistics.wavefrontSize << csvSeparator;

    // LDS available bytes.
    output << statistics.LDSSizeAvailable << csvSeparator;

    // LDS actual bytes.
    output << statistics.LDSSizeUsed << csvSeparator;

    // Available SGPRs.
    output << statistics.numSGPRsAvailable << csvSeparator;

    // Used SGPRs.
    output << statistics.numSGPRsUsed << csvSeparator;

    // Spills of SGPRs.
    output << statistics.numSGPRSpills << csvSeparator;

    // Available VGPRs.
    output << statistics.numVGPRsAvailable << csvSeparator;

    // Used VGPRs.
    output << statistics.numVGPRsUsed << csvSeparator;

    // Spills of VGPRs.
    output << statistics.numVGPRSpills << csvSeparator;

    // CL Work-group dimensions (for a unified format, to be revisited).
    output << statistics.numThreadPerGroupX << csvSeparator;
    output << statistics.numThreadPerGroupY << csvSeparator;
    output << statistics.numThreadPerGroupZ << csvSeparator;

    // ISA size.
    output << statistics.ISASize;

    output << std::endl;

    return output.str().c_str();
}

bool kcUtils::CreateStatisticsFile(const gtString& fileName, const Config& config,
                                   const std::map<std::string, beKA::AnalysisData>& analysisData, LoggingCallBackFunc_t pLogCallback)
{
    bool ret = false;

    // Get the separator for CSV list items.
    char csvSeparator = GetCsvSeparator(config);

    // Open output file.
    std::ofstream output;
    output.open(fileName.asASCIICharArray());

    if (output.is_open())
    {
        // Write the header.
        output << GetStatisticsCsvHeaderString(csvSeparator) << std::endl;

        // Write the device data.
        for (const auto& deviceStatsPair : analysisData)
        {
            // Write a line of CSV.
            output << DeviceStatisticsToCsvString(config, deviceStatsPair.first, deviceStatsPair.second);
        }

        output.close();
        ret = true;
    }
    else if (pLogCallback != nullptr)
    {
        std::stringstream s_Log;
        s_Log << STR_ERR_CANNOT_OPEN_FILE_FOR_WRITE_A << fileName.asASCIICharArray() <<
              STR_ERR_CANNOT_OPEN_FILE_FOR_WRITE_B << std::endl;
        pLogCallback(s_Log.str());
    }

    return ret;
}


std::string kcUtils::GetStatisticsCsvHeaderString(char csvSeparator)
{
    std::stringstream output;
    output << STR_CSV_HEADER_DEVICE << csvSeparator;
    output << STR_CSV_HEADER_SCRATCH_MEM << csvSeparator;
    output << STR_CSV_HEADER_THREADS_PER_WG << csvSeparator;
    output << STR_CSV_HEADER_WAVEFRONT_SIZE << csvSeparator;
    output << STR_CSV_HEADER_LDS_BYTES_MAX << csvSeparator;
    output << STR_CSV_HEADER_LDS_BYTES_ACTUAL << csvSeparator;
    output << STR_CSV_HEADER_SGPR_AVAILABLE << csvSeparator;
    output << STR_CSV_HEADER_SGPR_USED << csvSeparator;
    output << STR_CSV_HEADER_SGPR_SPILLS << csvSeparator;
    output << STR_CSV_HEADER_VGPR_AVAILABLE << csvSeparator;
    output << STR_CSV_HEADER_VGPR_USED << csvSeparator;
    output << STR_CSV_HEADER_VGPR_SPILLS << csvSeparator;
    output << STR_CSV_HEADER_CL_WORKGROUP_DIM_X << csvSeparator;
    output << STR_CSV_HEADER_CL_WORKGROUP_DIM_Y << csvSeparator;
    output << STR_CSV_HEADER_CL_WORKGROUP_DIM_Z << csvSeparator;
    output << STR_CSV_HEADER_ISA_SIZE_BYTES;
    return output.str().c_str();
}

void kcUtils::CreateStatisticsFile(const gtString& fileName, const Config& config, const std::string& device,
                                   const beKA::AnalysisData& deviceStatistics, LoggingCallBackFunc_t pLogCallback)
{
    // Create a temporary map and invoke the general routine.
    std::map<std::string, beKA::AnalysisData> tmpMap;
    tmpMap[device] = deviceStatistics;
    CreateStatisticsFile(fileName, config, tmpMap, pLogCallback);
}

char kcUtils::GetCsvSeparator(const Config& config)
{
    char csvSeparator;

    if (!config.m_CSVSeparator.empty())
    {
        csvSeparator = config.m_CSVSeparator[0];

        if (config.m_CSVSeparator[0] == '\\' && config.m_CSVSeparator.size() > 1)
        {
            switch (config.m_CSVSeparator[1])
            {
                case 'a': csvSeparator = '\a'; break;

                case 'b': csvSeparator = '\b'; break;

                case 'f': csvSeparator = '\f'; break;

                case 'n': csvSeparator = '\n'; break;

                case 'r': csvSeparator = '\r'; break;

                case 't': csvSeparator = '\t'; break;

                case 'v': csvSeparator = '\v'; break;

                default:
                    csvSeparator = config.m_CSVSeparator[1];
                    break;
            }
        }
    }
    else
    {
        // The default separator.
        csvSeparator = ',';
    }

    return csvSeparator;
}

bool kcUtils::DeleteFile(const gtString& fileFullPath)
{
    bool ret = false;
    osFilePath path(fileFullPath);

    if (path.exists())
    {
        osFile file(path);
        ret = file.deleteFile();
    }

    return ret;
}

void kcUtils::ReplaceStatisticsFile(const gtString& statisticsFile, const Config& config,
                                    const std::string& device, IStatisticsParser& statsParser, LoggingCallBackFunc_t logCb)
{
    // Parse the backend statistics.
    beKA::AnalysisData statistics;
    statsParser.ParseStatistics(statisticsFile, statistics);

    // Delete the older statistics file.
    kcUtils::DeleteFile(statisticsFile);

    // Create a new statistics file in the CLI format.
    kcUtils::CreateStatisticsFile(statisticsFile, config, device, statistics, logCb);
}

void kcUtils::PerformLiveRegisterAnalysis(const gtString& isaFileName, const gtString& outputFileName,
                                          LoggingCallBackFunc_t pCallback, bool printCmd)
{
    // Call the backend.
    beStatus rc = beStaticIsaAnalyzer::PerformLiveRegisterAnalysis(isaFileName, outputFileName, printCmd);

    if (rc != beStatus_SUCCESS && pCallback != nullptr)
    {
        // Inform the user in case of an error.
        std::stringstream msg;

        switch (rc)
        {
            case beKA::beStatus_shaeCannotLocateAnalyzer:
                // Failed to locate the ISA analyzer.
                msg << STR_ERR_CANNOT_LOCATE_LIVE_REG_ANALYZER << std::endl;
                break;

            case beKA::beStatus_shaeIsaFileNotFound:
                // ISA file not found.
                msg << STR_ERR_CANNOT_FIND_ISA_FILE << std::endl;
                break;

            case beKA::beStatus_shaeFailedToLaunch:
#ifndef __linux__
                // Failed to launch the ISA analyzer.
                // On Linux, there is an issue with this return code due to the
                // executable format that we use for the backend.
                msg << STR_ERR_CANNOT_LAUNCH_LIVE_REG_ANALYZER << std::endl;
#endif
                break;

            case beKA::beStatus_General_FAILED:
            default:
                // Generic error message.
                msg << STR_ERR_CANNOT_PERFORM_LIVE_REG_ANALYSIS << std::endl;
                break;
        }

        const std::string& errMsg = msg.str();

        if (!errMsg.empty() && pCallback != nullptr)
        {
            pCallback(errMsg);
        }
    }
}

void kcUtils::GenerateControlFlowGraph(const gtString& isaFileName, const gtString& outputFileName,
                                       LoggingCallBackFunc_t pCallback, bool perInstCfg, bool printCmd)
{
    // Call the backend.
    beStatus rc = beStaticIsaAnalyzer::GenerateControlFlowGraph(isaFileName, outputFileName, perInstCfg, printCmd);

    if (rc != beStatus_SUCCESS && pCallback != nullptr)
    {
        // Inform the user in case of an error.
        std::stringstream msg;

        switch (rc)
        {
            case beKA::beStatus_shaeCannotLocateAnalyzer:
                // Failed to locate the ISA analyzer.
                msg << STR_ERR_CANNOT_LOCATE_LIVE_REG_ANALYZER << std::endl;
                break;

            case beKA::beStatus_shaeIsaFileNotFound:
                // ISA file not found.
                msg << STR_ERR_CANNOT_FIND_ISA_FILE << std::endl;
                break;

            case beKA::beStatus_shaeFailedToLaunch:
#ifndef __linux__
                // Failed to launch the ISA analyzer.
                // On Linux, there is an issue with this return code due to the
                // executable format that we use for the backend.
                msg << STR_ERR_CANNOT_LAUNCH_CFG_ANALYZER << std::endl;
#endif
                break;

            case beKA::beStatus_General_FAILED:
            default:
                // Generic error message.
                msg << STR_ERR_CANNOT_PERFORM_LIVE_REG_ANALYSIS << std::endl;
                break;
        }

        const std::string& errMsg = msg.str();

        if (!errMsg.empty() && pCallback != nullptr)
        {
            pCallback(errMsg);
        }
    }

}

void kcUtils::ConstructOutputFileName(const std::string& baseOutputFileName, const std::string& defaultExtension,
                                      const std::string& kernelName, const std::string& deviceName, gtString& generatedFileName)
{
    // Convert the base output file name to gtString.
    gtString baseOutputFileNameAsGtStr;
    baseOutputFileNameAsGtStr << baseOutputFileName.c_str();
    osFilePath outputFilePath(baseOutputFileNameAsGtStr);

    // Extract the user's file name and extension.
    gtString fileName;
    if (!outputFilePath.isDirectory())
    {
        outputFilePath.getFileName(fileName);
    }
    else
    {
        osDirectory outputDir(baseOutputFileNameAsGtStr);
        outputFilePath.setFileDirectory(outputDir);
    }

    // Fix the user's file name to generate a unique output file name in the Analyzer CLI format.
    gtString fixedFileName;
    fixedFileName << deviceName.c_str();

    if (!kernelName.empty())
    {
        if (!fixedFileName.isEmpty())
        {
            fixedFileName << "_";
        }

        fixedFileName << kernelName.c_str();
    }

    if (!fileName.isEmpty())
    {
        if (!fixedFileName.isEmpty())
        {
            fixedFileName << "_";
        }

        fixedFileName << fileName;
    }

    outputFilePath.setFileName(fixedFileName);

    // Handle the default extension (unless the user specified an extension).
    gtString outputFileExtension;
    outputFilePath.getFileExtension(outputFileExtension);

    if (outputFileExtension.isEmpty())
    {
        outputFileExtension.fromASCIIString(defaultExtension.c_str());
        outputFilePath.setFileExtension(outputFileExtension);
    }

    // Set the output string.
    generatedFileName = outputFilePath.asString();
}

gtString kcUtils::ConstructTempFileName(const gtString& prefix, const gtString & ext)
{
    const unsigned int  MAX_ATTEMPTS = 1024;
    osDirectory  rgaTempDir;
    gtString  ret = L"";

    if (GetRGATempDir(rgaTempDir))
    {
        if (!rgaTempDir.exists())
        {
            rgaTempDir.create();
        }

        osFilePath tempFilePath;
        tempFilePath.setFileDirectory(rgaTempDir);

        gtString tempFileBaseName = prefix;
        tempFileBaseName.appendUnsignedIntNumber(osGetCurrentProcessId());
        gtString tempFileName = tempFileBaseName;
        tempFileName.append(L".");
        tempFileName.append(ext);
        tempFilePath.setFileName(tempFileName);

        uint32_t suffixNum = 0;
        while (tempFilePath.exists() && suffixNum < MAX_ATTEMPTS)
        {
            tempFileName = tempFileBaseName;
            tempFileName.appendUnsignedIntNumber(suffixNum++);
            tempFileName.append(L".");
            tempFileName.append(ext);
            tempFilePath.setFileName(tempFileName);
        }

        ret = suffixNum < MAX_ATTEMPTS ? tempFilePath.asString() : L"";
    }

    return ret;
}

bool kcUtils::GetMarketingNameToCodenameMapping(DeviceNameMap& cardsMap)
{
    bool ret = beUtils::GetMarketingNameToCodenameMapping(cardsMap);

    // Remove the disabled devices.
    for (const std::string& disabledDevice : RGA_DISABLED_DEVICES)
    {
        cardsMap.erase(disabledDevice);
    }

    return ret;
}

// Converts "srcName" string to lowercase and removes all spaces and '-'.
// Stores the result in "dstName".
static void ReduceDeviceName(std::string& name)
{
    std::transform(name.begin(), name.end(), name.begin(), [](const char& c) {return std::tolower(c);});
    name.erase(std::remove_if(name.begin(), name.end(), [](const char& c) {return (std::isspace(c) || c == '-');}), name.end());
}

// Helper function that interprets the matched devices found by the "kcUtils::FindGPUArchName()".
static bool ResolveMatchedDevices(const kcUtils::DeviceNameMap& matchedDevices, const std::string& device,
                                  bool printInfo, bool printUnknownDeviceError)
{
    bool  status = false;
    std::stringstream  outMsg, errMsg;
    // Routine printing the architecture name and all its device names to required stream.
    auto  PrintArchAndDevices = [&](const kcUtils::DeviceNameMap::value_type& arch, std::stringstream& s)
    {
        s << arch.first << std::endl;
        for (const std::string& marketingName : arch.second)
        {
            s << "\t" << marketingName << std::endl;
        }
    };
    if (matchedDevices.size() == 0)
    {
        if (printInfo && printUnknownDeviceError)
        {
            // No matching architectures found. Failure.
            errMsg << STR_ERR_COULD_NOT_DETECT_TARGET << device << std::endl;
        }
    }
    else if (matchedDevices.size() == 1)
    {
        // Found exactly one GPU architectire. Success.
        if (printInfo)
        {
            outMsg << STR_TARGET_DETECTED << std::endl << std::endl;
            PrintArchAndDevices(*(matchedDevices.begin()), outMsg);
            outMsg << std::endl;
        }
        status = true;
    }
    else if (printInfo)
    {
        // Found multiple GPU architectures. Failure.
        errMsg << STR_ERR_AMBIGUOUS_TARGET << device << std::endl << std::endl;
        for (auto& arch : matchedDevices)
        {
            PrintArchAndDevices(arch, errMsg);
        }
    }
    rgLog::stdOut << outMsg.str() << rgLog::flush;
    if (!errMsg.str().empty())
    {
        rgLog::stdErr << errMsg.str() << std::endl;
    }
    return status;
}

bool kcUtils::FindGPUArchName(const std::string& device, std::string& matchedDevice, bool printInfo, bool allowUnknownDevice)
{
    bool  status = false;
    const char* FILTER_INDICATOR_1 = ":";
    const char* FILTER_INDICATOR_2 = "Not Used";

    // Mappings  "architecture <--> marketing names"
    DeviceNameMap  matchedDevices, cardsMapping;

    // Transform the device name to lower case and remove spaces.
    std::string  reducedName = device;
    ReduceDeviceName(reducedName);

    bool rc = kcUtils::GetMarketingNameToCodenameMapping(cardsMapping);

    // Walk over all known architectures and devices.
    if (rc && !cardsMapping.empty())
    {
        for (const auto& pair : cardsMapping)
        {
            const std::string& archName = pair.first;
            std::string  reducedArchName = archName;
            ReduceDeviceName(reducedArchName);

            // If we found a match with an arch name -- add it to the list of matched archs and continue.
            // Otherwise, look at the marketing device names.
            if (reducedArchName.find(reducedName) != std::string::npos)
            {
                // Put the found arch with an all its devices into the "matchedArchs" map.
                // Filter out the devices with code names and unused names.
                std::set<std::string>& deviceList = matchedDevices[archName];
                for (const std::string& device : pair.second)
                {
                    if (device.find(FILTER_INDICATOR_1) == std::string::npos && device.find(FILTER_INDICATOR_2) == std::string::npos)
                    {
                        deviceList.emplace(device);
                    }
                }
            }
            else
            {
                bool  addedArch = false;

                for (const std::string& marketingName : pair.second)
                {
                    // We do not want to display names that contain these strings.
                    if (marketingName.find(FILTER_INDICATOR_1) == std::string::npos && marketingName.find(FILTER_INDICATOR_2) == std::string::npos)
                    {
                        std::string reducedMarketingName = marketingName;
                        ReduceDeviceName(reducedMarketingName);

                        if (reducedMarketingName.find(reducedName) != std::string::npos)
                        {
                            // Found a match with the marketing name -- add it to the device list corresponding to the "archName" architecture.
                            if (!addedArch)
                            {
                                matchedDevices[archName] = std::set<std::string>();
                                addedArch = true;
                            }
                            matchedDevices[archName].emplace(marketingName);
                        }
                    }
                }
            }
        }
    }

    // Interpret the matched names.
    if (ResolveMatchedDevices(matchedDevices, device, printInfo, !allowUnknownDevice))
    {
        matchedDevice = (*(matchedDevices.begin())).first;
        status = true;
    }

    return status;
}

// Returns a subdirectory of the OS temp directory where RGA keeps all temporary files.
// Creates the subdirectory if it does not exists.
bool GetRGATempDir(osDirectory & dir)
{
    const gtString AMD_RGA_TEMP_DIR_1 = L"GPUOpen";
    const gtString AMD_RGA_TEMP_DIR_2 = L"rga";
    bool  ret = true;

    osFilePath tempDirPath(osFilePath::OS_TEMP_DIRECTORY);
    tempDirPath.appendSubDirectory(AMD_RGA_TEMP_DIR_1);
    tempDirPath.getFileDirectory(dir);
    if (!dir.exists())
    {
        ret = dir.create();
    }

    if (ret)
    {
        tempDirPath.appendSubDirectory(AMD_RGA_TEMP_DIR_2);
        tempDirPath.getFileDirectory(dir);
        if (!dir.exists())
        {
            ret = dir.create();
        }
    }

    return ret;
}

bool kcUtils::PrintAsicList(std::ostream& log, const std::set<std::string>& disabledDevices, const std::set<std::string>& reqdDevices)
{
    // We do not want to display names that contain these strings.
    const char* FILTER_INDICATOR_1 = ":";
    const char* FILTER_INDICATOR_2 = "Not Used";

    bool  result = false;
    std::map<std::string, std::set<std::string>> cardsMapping;
    bool rc = kcUtils::GetMarketingNameToCodenameMapping(cardsMapping);
    if (rc && !cardsMapping.empty())
    {
        for (const auto& pair : cardsMapping)
        {
            // If "reqdDevices" is provided, print only devices from this set.
            // If "disdDevices" is provided, do not pring devices from this set.
            // The "reqdDevices" contains short arch names (like "gfx804"), while "cardsMapping" has extended names: "gfx804 (Graphics IP v8)".
            auto isInDeviceList = [](const std::set<std::string>& list, const std::string & device)
                                    { for (auto & d : list) { if (device.find(d) != std::string::npos) return true; } return false; };

            if ((reqdDevices.empty() || isInDeviceList(reqdDevices, pair.first)) && (disabledDevices.empty() || !isInDeviceList(disabledDevices, pair.first)))
            {
                log << pair.first << std::endl;
                for (const std::string& card : pair.second)
                {
                    // Filter out internal names.
                    if (card.find(FILTER_INDICATOR_1) == std::string::npos &&
                        card.find(FILTER_INDICATOR_2) == std::string::npos)
                    {
                        log << "\t" << card << std::endl;
                    }
                }
            }
        }
        result = true;
    }

    return result;
}

bool kcUtils::GetLogFileName(std::string& logFileName)
{
    bool  ret = false;
    osDirectory  tmpDir;
    osFilePath   logFile;
    if (GetRGATempDir(tmpDir))
    {
        logFile.setFileDirectory(tmpDir);
        logFile.setFileName(RGA_CLI_LOG_FILE_NAME);
        logFile.setFileExtension(RGA_CLI_LOG_FILE_EXT);
        logFileName = logFile.asString().asASCIICharArray();
        ret = true;
    }
    return ret;
}

bool kcUtils::GetParsedISAFileName(const std::string& isaFileName, std::string& parsedIsaFileName)
{
    gtString  gtFileName;
    gtFileName.fromASCIIString(isaFileName.c_str());
    osFilePath  filePath(gtFileName);
    filePath.setFileExtension(RGA_CLI_PARSED_ISA_FILE_EXT);
    parsedIsaFileName = filePath.asString().asASCIICharArray();

    return (!parsedIsaFileName.empty());
}

std::string kcUtils::Quote(const std::string& str)
{
    return (str.find(' ') == std::string::npos ? str : (std::string("\"") + str + '"'));
}

void kcUtils::DeletePipelineFiles(const beProgramPipeline & files)
{
    auto deleteFile = [](const gtString& fileName)
                      { if (!fileName.isEmpty() && FileNotEmpty(fileName.asASCIICharArray())) { kcUtils::DeleteFile(fileName); } };

    deleteFile(files.m_vertexShader);
    deleteFile(files.m_tessControlShader);
    deleteFile(files.m_tessEvaluationShader);
    deleteFile(files.m_geometryShader);
    deleteFile(files.m_fragmentShader);
    deleteFile(files.m_computeShader);
}

const std::vector<std::string> kcUtils::GetRgaDisabledDevices()
{
    return RGA_DISABLED_DEVICES;
}

void kcUtils::PrintRgaVersion()
{
    std::cout << STR_RGA_PRODUCT_NAME << " " << STR_RGA_VERSION_PREFIX << STR_RGA_VERSION << "." << STR_RGA_BUILD_NUM << std::endl;
}

kcUtils::ProcessStatus kcUtils::LaunchProcess(const std::string& execPath, const std::string& args, const std::string& dir,
                                              unsigned long timeOut, bool printCmd, std::string& stdOut, std::string& stdErr, long& exitCode)
{
    osProcessId      cmplrProcID;
    osProcessHandle  cmplrProcHandle;
    osThreadHandle   cmplrThreadHandle;
    ProcessStatus    status = ProcessStatus::Success;

    // Set working directory, executable and arguments.
    osFilePath  workDir;
    if (dir == "")
    {
        workDir.setPath(osFilePath::OS_CURRENT_DIRECTORY);
    }
    else
    {
        gtString  gtDir;
        gtDir << dir.c_str();
        workDir.setFileDirectory(gtDir);
    }

    gtString gtExecPath, gtArgs;
    gtExecPath << execPath.c_str();
    gtArgs << args.c_str();

    // Create temporary files for stdout/stderr.
    gtString  outFileName = kcUtils::ConstructTempFileName(RGA_CLI_TEMP_STDOUT_FILE_NAME, RGA_CLI_TEMP_STDOUT_FILE_EXT);
    gtString  errFileName = kcUtils::ConstructTempFileName(RGA_CLI_TEMP_STDERR_FILE_NAME, RGA_CLI_TEMP_STDERR_FILE_EXT);

    if (outFileName.isEmpty() || errFileName.isEmpty())
    {
        status = ProcessStatus::CreateTempFileFailed;
    }
    else
    {
        if (printCmd)
        {
            std::cout << std::endl << KC_STR_LAUNCH_EXTERNAL_PROCESS << gtExecPath.asASCIICharArray() << " " << gtArgs.asASCIICharArray() << std::endl;
        }

        gtArgs += L" >";
        gtArgs += outFileName;
        gtArgs += L" 2>";
        gtArgs += errFileName;

        // Launch a process.
        bool  procStatus = osLaunchSuspendedProcess(gtExecPath,
                                                    gtArgs,
                                                    workDir,
                                                    cmplrProcID,
                                                    cmplrProcHandle,
                                                    cmplrThreadHandle,
                                                    false,  // Don't create a window
                                                    true);  // Redirect stdout & stderr

        if (procStatus && osResumeSuspendedProcess(cmplrProcID, cmplrProcHandle, cmplrThreadHandle, false))
        {
            procStatus = osWaitForProcessToTerminate(cmplrProcID, timeOut, &exitCode);

            if (!procStatus)
            {
                osTerminateProcess(cmplrProcID);
                status = ProcessStatus::TimeOut;
            }
        }
        else
        {
            status = ProcessStatus::LaunchFailed;
        }
    }

    // Read the content of stderr and stdout files.
    stdOut = stdErr = "";
    if (status == ProcessStatus::Success)
    {
        if (FileNotEmpty(outFileName.asASCIICharArray()))
        {
            if (!kcUtils::ReadTextFile(outFileName.asASCIICharArray(), stdOut, nullptr))
            {
                status = ProcessStatus::ReadTempFileFailed;
            }
            std::remove(outFileName.asASCIICharArray());
        }

        if (FileNotEmpty(errFileName.asASCIICharArray()))
        {
            if (!kcUtils::ReadTextFile(errFileName.asASCIICharArray(), stdErr, nullptr))
            {
                status = ProcessStatus::ReadTempFileFailed;
            }
            std::remove(errFileName.asASCIICharArray());
        }
    }

    return status;
}

bool kcUtils::FileNotEmpty(const std::string fileName)
{
    bool  ret = false;
    if (!fileName.empty())
    {
        std::ifstream file(fileName);
        ret = (file.good() && file.peek() != std::ifstream::traits_type::eof());
    }
    return ret;
}

bool kcUtils::ReadProgramSource(const string& inputFile, string& programSource)
{
    std::ifstream input;

#ifndef _WIN32
    // test if the input file is a directory.
    // On some Linux machine ifstream open will be valid if it is a directory
    // but will not allow to read data which will cause a crash when trying to read the data
    DIR* pDir;

    pDir = opendir(inputFile.c_str());

    if (pDir != NULL)
    {
        (void)closedir(pDir);
        return false;
    }

#endif

    // Open (at e)nd of file.  We want the length below.
    // Open binary because it's faster & other bits of code deal OK with CRLF, LF etc.
    input.open(inputFile.c_str(), ios::ate | ios::binary);

    if (!input)
    {
        return false;
    }

    std::ifstream::pos_type fileSize = 0;

    fileSize = input.tellg();

    if (fileSize == static_cast<std::ifstream::pos_type>(0))
    {
        input.close();
        return false;
    }

    input.seekg(0, ios::beg);

    programSource.resize(size_t(fileSize));
    input.read(&programSource[0], fileSize);

    input.close();
    return true;
}


bool kcUtils::WriteBinaryFile(const std::string& fileName, const std::vector<char>& content, LoggingCallBackFunc_t pCallback)
{
    bool ret = false;
    ofstream output;
    output.open(fileName.c_str(), ios::binary);

    if (output.is_open() && !content.empty())
    {
        output.write(&content[0], content.size());
        output.close();
        ret = true;
    }
    else
    {
        std::stringstream log;
        log << "Error: Unable to open " << fileName << " for write.\n";

        if (pCallback != nullptr)
        {
            pCallback(log.str());
        }
    }

    return ret;
}

bool kcUtils::ReadTextFile(const std::string& fileName, std::string& content, LoggingCallBackFunc_t pCallback)
{
    bool ret = false;
    ifstream input;
    input.open(fileName.c_str());

    if (input.is_open())
    {
        std::stringstream  textStream;
        textStream << input.rdbuf();
        content = textStream.str();
        input.close();
        ret = true;
    }
    else
    {
        std::stringstream log;
        log << STR_ERR_CANNOT_READ_FILE << fileName << std::endl;

        if (pCallback != nullptr)
        {
            pCallback(log.str());
        }
    }

    return ret;
}

bool kcUtils::WriteTextFile(const std::string& fileName, const std::string& content, LoggingCallBackFunc_t pCallback)
{
    bool ret = false;
    ofstream output;
    output.open(fileName.c_str());

    if (output.is_open())
    {
        output << content << std::endl;
        output.close();
        ret = true;
    }
    else
    {
        std::stringstream log;
        log << STR_ERR_CANNOT_OPEN_FILE_FOR_WRITE_A << fileName << STR_ERR_CANNOT_OPEN_FILE_FOR_WRITE_B << std::endl;

        if (pCallback != nullptr)
        {
            pCallback(log.str());
        }
    }

    return ret;
}

// Creates an element that has value of any primitive type.
template <typename T>
static void AppendXMLElement(tinyxml2::XMLDocument &xmlDoc, tinyxml2::XMLElement* pParent, const char* pElemName, T elemValue)
{
    tinyxml2::XMLElement* pElem = xmlDoc.NewElement(pElemName);
    pElem->SetText(elemValue);
    pParent->InsertEndChild(pElem);
}

// Extract the CAL (generation) and code name.
// Example of "deviceName" format: "Baffin (Graphics IP v8)"
// Returned value: {"Graphics IP v8", "Baffin"}
static std::pair<std::string, std::string>
GetGenAndCodeNames(const std::string& deviceName)
{
    size_t  codeNameOffset = deviceName.find('(');
    std::string  codeName = (codeNameOffset != std::string::npos ? deviceName.substr(0, codeNameOffset - 1) : deviceName);
    std::string  genName  = (codeNameOffset != std::string::npos ? deviceName.substr(codeNameOffset + 1, deviceName.size() - codeNameOffset - 2) : "");
    return { genName, codeName };
}

static bool AddSupportedGPUInfo(tinyxml2::XMLDocument& doc, tinyxml2::XMLElement& parent, const std::set<std::string>& targets)
{
    const char* FILTER_INDICATOR_1 = ":";
    const char* FILTER_INDICATOR_2 = "Not Used";
    bool  ret;

    tinyxml2::XMLElement*  pSupportedGPUs = doc.NewElement(XML_NODE_SUPPORTED_GPUS);

    // Add supported GPUS info.
    kcUtils::DeviceNameMap  cardsMap;
    if ((ret = kcUtils::GetMarketingNameToCodenameMapping(cardsMap)) == true)
    {
        // Sort the devices by Generation name.
        std::vector<std::pair<std::string, std::set<std::string>>>  devices(cardsMap.begin(), cardsMap.end());
        std::sort(devices.begin(), devices.end(),
                  [](const std::pair<std::string, std::set<std::string>>& d1, const std::pair<std::string, std::set<std::string>> &d2)
                    { return (GetGenAndCodeNames(d1.first).first < GetGenAndCodeNames(d2.first).first); });

        for (const auto& device : devices)
        {
            std::string  deviceName = device.first, codeName, genName;
            std::tie(genName, codeName) = GetGenAndCodeNames(deviceName);
            // Skip targets that are not supported in this mode.
            if (targets.count(codeName) == 0)
            {
                continue;
            }
            tinyxml2::XMLElement*  pGPU = doc.NewElement(XML_NODE_GPU);
            tinyxml2::XMLElement*  pGen = doc.NewElement(XML_NODE_GENERATION);
            tinyxml2::XMLElement*  pCodeName = doc.NewElement(XML_NODE_CODENAME);
            pGen->SetText(genName.c_str());
            pCodeName->SetText(codeName.c_str());
            pGPU->LinkEndChild(pGen);
            pGPU->LinkEndChild(pCodeName);

            // Add the list of marketing names.
            std::stringstream  mktNames;
            bool  first = true;
            for (const std::string& mktName : device.second)
            {
                if (mktName.find(FILTER_INDICATOR_1) == std::string::npos && mktName.find(FILTER_INDICATOR_2) == std::string::npos)
                {
                    mktNames << (first ? "" : ", ") << mktName;
                    first = false;
                }
            }

            tinyxml2::XMLElement*  pPublicNames = doc.NewElement(XML_NODE_PRODUCT_NAMES);
            pPublicNames->SetText(mktNames.str().c_str());
            pGPU->LinkEndChild(pPublicNames);
            pSupportedGPUs->LinkEndChild(pGPU);
        }
    }

    if (ret)
    {
        parent.LinkEndChild(pSupportedGPUs);
    }

    return ret;
}

static bool  AddModeInfo(tinyxml2::XMLDocument& doc, SourceLanguage lang)
{
    std::set<std::string> targets;
    std::string  mode;
    bool  ret = true;

    switch (lang)
    {
        case SourceLanguage::SourceLanguage_Rocm_OpenCL:  mode = XML_MODE_ROCM_CL; break;
        case SourceLanguage::SourceLanguage_OpenCL:       mode = XML_MODE_CL;      break;
        case SourceLanguage::SourceLanguage_HLSL:         mode = XML_MODE_HLSL;    break;
        case SourceLanguage::SourceLanguage_GLSL_OpenGL:  mode = XML_MODE_OPENGL;  break;
        case SourceLanguage::SourceLanguage_GLSL_Vulkan:  mode = XML_MODE_VULKAN;  break;

        default: ret = false;
    }

    if (ret)
    {
        tinyxml2::XMLElement  *pNameElem, *pModeElem = doc.NewElement(XML_NODE_MODE);
        ret = ret && (pModeElem != nullptr);
        if (ret)
        {
            doc.LinkEndChild(pModeElem);
            pNameElem = doc.NewElement(XML_NODE_NAME);
            if ((ret = (pNameElem != nullptr)) == true)
            {
                pModeElem->LinkEndChild(pNameElem);
                pNameElem->SetText(mode.c_str());
            }
        }

        ret = ret && kcCLICommander::GetSupportedTargets(lang, targets);
        ret = ret && AddSupportedGPUInfo(doc, *pModeElem, targets);
    }

    return ret;
}

bool kcUtils::GenerateVersionInfoFile(const std::string & fileName)
{
    bool  ret = true;

    tinyxml2::XMLDocument  doc;

    // Add the RGA CLI version.
    tinyxml2::XMLElement*  pVersionElem = doc.NewElement(XML_NODE_VERSION);
    std::stringstream  versionTag;
    versionTag << STR_RGA_VERSION << "." << STR_RGA_BUILD_NUM;
    pVersionElem->SetText(versionTag.str().c_str());
    doc.LinkEndChild(pVersionElem);

    // Add the RGA CLI build date.
    // First, reformat the Windows date string provided in format "Day dd/mm/yyyy" to format "yyyy-mm-dd".
    std::string  dateString = STR_RGA_BUILD_DATE;

#ifdef WIN32
    ret = rgaSharedUtils::ConvertDateString(dateString);
#endif

    tinyxml2::XMLElement*  pBuildDateElem = doc.NewElement(XML_NODE_BUILD_DATE);
    pBuildDateElem->SetText(dateString.c_str());
    doc.LinkEndChild(pBuildDateElem);

    // Add supported devices for each mode.
    ret = ret && AddModeInfo(doc, SourceLanguage::SourceLanguage_Rocm_OpenCL);
    ret = ret && AddModeInfo(doc, SourceLanguage::SourceLanguage_OpenCL);
    ret = ret && AddModeInfo(doc, SourceLanguage::SourceLanguage_HLSL);
    ret = ret && AddModeInfo(doc, SourceLanguage::SourceLanguage_GLSL_OpenGL);
    ret = ret && AddModeInfo(doc, SourceLanguage::SourceLanguage_GLSL_Vulkan);

    if (ret)
    {
        if (fileName.empty())
        {
            doc.Print();
        }
        else
        {
            ret = (doc.SaveFile(fileName.c_str()) == tinyxml2::XML_SUCCESS);
        }
    }

    return ret;
}

static bool AddOutputFile(tinyxml2::XMLDocument& doc, tinyxml2::XMLElement& parent,
                          const std::string& fileName, const char* tag)
{
    bool  ret = true;
    if (!fileName.empty())
    {
        tinyxml2::XMLElement*  pElement = doc.NewElement(tag);
        if (pElement != nullptr)
        {
            pElement->SetText(fileName.c_str());
            ret = (parent.LinkEndChild(pElement) != nullptr);
        }
        else
        {
            ret = false;
        }
    }
    return ret;
}

static bool AddEntryType(tinyxml2::XMLDocument& doc, tinyxml2::XMLElement* pEntry, rgEntryType entryType)
{
    bool  ret = (pEntry != nullptr);
    tinyxml2::XMLElement*  pEntryType;
    if (ret)
    {
        pEntryType = doc.NewElement(XML_NODE_TYPE);
        ret = ret && (pEntryType != nullptr && pEntry->LinkEndChild(pEntryType) != nullptr);
    }
    if (ret)
    {
        std::string  entryTypeStr = "";
        switch (entryType)
        {
        case rgEntryType::OpenCL_Kernel:
            entryTypeStr = XML_SHADER_OPENCL;
            break;
        case rgEntryType::DX_Vertex:
            entryTypeStr = XML_SHADER_DX_VERTEX;
            break;
        case rgEntryType::DX_Hull:
            entryTypeStr = XML_SHADER_DX_HULL;
            break;
        case rgEntryType::DX_Domain:
            entryTypeStr = XML_SHADER_DX_DOMAIN;
            break;
        case rgEntryType::DX_Geometry:
            entryTypeStr = XML_SHADER_DX_GEOMETRY;
            break;
        case rgEntryType::DX_Pixel:
            entryTypeStr = XML_SHADER_DX_PIXEL;
            break;
        case rgEntryType::DX_Compute:
            entryTypeStr = XML_SHADER_DX_COMPUTE;
            break;
        case rgEntryType::GL_Vertex:
            entryTypeStr = XML_SHADER_GL_VERTEX;
            break;
        case rgEntryType::GL_TessControl:
            entryTypeStr = XML_SHADER_GL_TESS_CTRL;
            break;
        case rgEntryType::GL_TessEval:
            entryTypeStr = XML_SHADER_GL_TESS_EVAL;
            break;
        case rgEntryType::GL_Geometry:
            entryTypeStr = XML_SHADER_GL_GEOMETRY;
            break;
        case rgEntryType::GL_Fragment:
            entryTypeStr = XML_SHADER_GL_FRAGMENT;
            break;
        case rgEntryType::GL_Compute:
            entryTypeStr = XML_SHADER_GL_COMPUTE;
            break;
        case rgEntryType::Unknown:
            entryTypeStr = XML_SHADER_UNKNOWN;
            break;
        default:
            ret = false;
            break;
        }

        if (ret)
        {
            pEntryType->SetText(entryTypeStr.c_str());
        }
    }

    return ret;
}

static bool AddOutputFiles(tinyxml2::XMLDocument& doc, tinyxml2::XMLElement* pEntry,
                           const std::string& target, const rgOutputFiles& outFiles)
{
    bool  ret = false;
    tinyxml2::XMLElement* pOutput = doc.NewElement(XML_NODE_OUTPUT);
    if (pEntry != nullptr && pOutput != nullptr && pEntry->LinkEndChild(pOutput) != nullptr)
    {
        // Add target GPU.
        tinyxml2::XMLElement* pTarget = doc.NewElement(XML_NODE_TARGET);
        ret = (pTarget != nullptr && pOutput->LinkEndChild(pTarget) != nullptr);
        if (ret)
        {
            pTarget->SetText(target.c_str());
        }
        // Add output files.
        if (!outFiles.m_isIsaFileTemp)
        {
            ret = ret && AddOutputFile(doc, *pOutput, outFiles.m_isaFile, XML_NODE_ISA);
        }
        ret = ret && AddOutputFile(doc, *pOutput, outFiles.m_isaCsvFile,  XML_NODE_CSV_ISA);
        ret = ret && AddOutputFile(doc, *pOutput, outFiles.m_statFile,    XML_NODE_RES_USAGE);
        ret = ret && AddOutputFile(doc, *pOutput, outFiles.m_liveregFile, XML_NODE_LIVEREG);
        ret = ret && AddOutputFile(doc, *pOutput, outFiles.m_cfgFile,     XML_NODE_CFG);
    }
    return ret;
}

bool kcUtils::GenerateCliMetadataFile(const std::string& fileName, const rgFileEntryData& fileEntryData,
                                      const rgOutputMetadata& outFiles)
{
    tinyxml2::XMLDocument  doc;
    std::string  currentDevice = "";

    tinyxml2::XMLElement* pDataModelElem = doc.NewElement(XML_NODE_DATA_MODEL);
    bool  ret = (pDataModelElem != nullptr && doc.LinkEndChild(pDataModelElem) != nullptr);
    if (ret)
    {
        pDataModelElem->SetText(STR_RGA_OUTPUT_MD_DATA_MODEL);
    }
    tinyxml2::XMLElement* pMetadata = doc.NewElement(XML_NODE_METADATA);
    ret = ret && (pMetadata != nullptr && doc.LinkEndChild(pMetadata) != nullptr);

    // Add binary name.
    if (!outFiles.empty() && !outFiles.begin()->second.m_isBinFileTemp)
    {
        tinyxml2::XMLElement* pBinary = doc.NewElement(XML_NODE_BINARY);
        ret = ret && (pBinary != nullptr && pMetadata->LinkEndChild(pBinary) != nullptr);
        if (ret)
        {
            pBinary->SetText((outFiles.begin())->second.m_binFile.c_str());
        }
    }

    if (ret)
    {
        // Map: kernel_name --> vector{pair{device, out_files}}.
        std::map<std::string, std::vector<std::pair<std::string, rgOutputFiles>>>  outFilesMap;

        // Map: input_file_name --> outFilesMap.
        std::map<std::string, decltype(outFilesMap)>  metadataTable;

        // Reorder the output file metadata in "kernel-first" order.
        for (const auto& outFileSet : outFiles)
        {
            const std::string& device = outFileSet.first.first;
            const std::string& kernel = outFileSet.first.second;

            outFilesMap[kernel].push_back({ device, outFileSet.second });
        }

        // Now, try to find a source file for each entry in "outFilesMap" and fill the "metadataTable".
        // Split the "outFilesMap" into parts so that each part contains entries from the same source file.
        // If no source file is found for an entry, use "<Unknown>" source file name.
        for (auto& outFileItem : outFilesMap)
        {
            const std::string&  entryName = outFileItem.first;
            std::string  srcFileName;

            // Try to find a source file corresponding to this entry name.
            auto inputFileInfo = std::find_if(fileEntryData.begin(), fileEntryData.end(),
                                              [&](rgFileEntryData::const_reference entryInfo)
                                                 { for (auto entry : entryInfo.second) { if (std::get<0>(entry) == entryName) return true; } return false; });

            srcFileName = (inputFileInfo == fileEntryData.end() ? XML_UNKNOWN_SOURCE_FILE : inputFileInfo->first);
            metadataTable[srcFileName].insert(outFileItem);
        }

        // Store the "metadataTable" structure to the session metadata file.
        for (auto& inputFileData : metadataTable)
        {
            if (ret)
            {
                // Add input file info.
                tinyxml2::XMLElement* pInputFile = doc.NewElement(XML_NODE_INPUT_FILE);
                ret = ret && (pInputFile != nullptr && pMetadata->LinkEndChild(pInputFile) != nullptr);
                tinyxml2::XMLElement* pInputFilePath = doc.NewElement(XML_NODE_PATH);
                ret = ret && (pInputFilePath != nullptr && pInputFile->LinkEndChild(pInputFilePath) != nullptr);

                if (ret)
                {
                    pInputFilePath->SetText(inputFileData.first.c_str());

                    // Add entry points info.
                    for (auto& entryData : inputFileData.second)
                    {
                        tinyxml2::XMLElement* pEntry = doc.NewElement(XML_NODE_ENTRY);
                        ret = (pEntry != nullptr && pInputFile->LinkEndChild(pEntry) != nullptr);
                        // Add entry name & type.
                        tinyxml2::XMLElement* pName = doc.NewElement(XML_NODE_NAME);
                        ret = ret && (pName != nullptr && pEntry->LinkEndChild(pName) != nullptr);
                        if (ret && entryData.second.size() > 0)
                        {
                            pName->SetText(entryData.first.c_str());
                            rgOutputFiles  outFileData = entryData.second[0].second;
                            ret = ret && AddEntryType(doc, pEntry, outFileData.m_entryType);
                        }
                        // Add "Output" nodes.
                        for (const std::pair<std::string, rgOutputFiles>& deviceAndOutFiles : entryData.second)
                        {
                            ret = ret && AddOutputFiles(doc, pEntry, deviceAndOutFiles.first, deviceAndOutFiles.second);
                            if (!ret)
                            {
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    ret = ret && (doc.SaveFile(fileName.c_str()) == tinyxml2::XML_SUCCESS);

    return ret;
}

// Get current system time.
static bool  CurrentTime(struct tm& time)
{
    bool  ret = false;
    std::stringstream  suffix;
    time_t  currentTime = std::time(0);
#ifdef _WIN32
    struct tm* pTime = &time;
    ret = (localtime_s(pTime, &currentTime) == 0);
#else
    struct tm*  pTime = localtime(&currentTime);
    if (pTime != nullptr)
    {
        time = *pTime;
        ret = true;
    }
#endif
    return ret;
}

// Delete log files older than 1 week.
static bool  DeleteOldLogs()
{
    bool  ret = false;
    const double  oneWeekSeconds = static_cast<double>(7*24*60*60);
    osDirectory tmpDir;
    if ((ret = GetRGATempDir(tmpDir)) == true)
    {
        gtString  logFilePattern;
        gtList<osFilePath>  filePaths;
        logFilePattern << RGA_CLI_LOG_FILE_NAME.asASCIICharArray() << "*." << RGA_CLI_LOG_FILE_EXT.asASCIICharArray();
        if (tmpDir.getContainedFilePaths(logFilePattern, osDirectory::SORT_BY_DATE_ASCENDING, filePaths))
        {
            for (const osFilePath& path : filePaths)
            {
                osStatStructure  fileStat;
                if ((ret = (osWStat(path.asString(), fileStat) == 0)) == true)
                {
                    time_t  fileTime = fileStat.st_ctime;
                    struct tm  time;
                    if ((ret = CurrentTime(time)) == true)
                    {
                        time_t  curTime = std::mktime(&time);
                        if (std::difftime(curTime, fileTime) > oneWeekSeconds)
                        {
                            std::remove(path.asString().asASCIICharArray());
                        }
                    }
                }
            }
        }
    }
    return ret;
}

// Perform log file initialization.
bool  kcUtils::InitCLILogFile(const Config& config)
{
    struct tm   tt;

    bool  status = DeleteOldLogs();
    status = status && CurrentTime(tt);

    std::string  logFileName = config.m_logFile;
    if (logFileName.empty())
    {
        gtString gFileName = kcUtils::ConstructTempFileName(RGA_CLI_LOG_FILE_NAME, RGA_CLI_LOG_FILE_EXT);
        logFileName = gFileName.asASCIICharArray();
    }

    if (status)
    {
        auto ZeroExt = [](int n) {std::string nS = std::to_string(n); return (n < 10 ? std::string("0") + nS : nS);};

        // Append current date/time to the log file name.
        std::stringstream  suffix;
        suffix << "-" << std::to_string(tt.tm_year + 1900) << ZeroExt(tt.tm_mon + 1) << ZeroExt(tt.tm_mday) <<
                    "-" << ZeroExt(tt.tm_hour) << ZeroExt(tt.tm_min) << ZeroExt(tt.tm_sec);

        size_t  extOffset = logFileName.rfind('.');
        logFileName.insert((extOffset == std::string::npos ? logFileName.size() : extOffset), suffix.str());

        if ((status = rgLog::OpenLogFile(logFileName)) == false)
        {
            rgLog::stdErr << STR_ERR_FAILED_OPEN_LOG_FILE << logFileName << std::endl;
        }
    }

    return status;
}

bool kcUtils::StrCmpNoCase(const std::string & s1, const std::string & s2)
{
    std::string s1_u = s1, s2_u = s2;
    std::transform(s1_u.begin(), s1_u.end(), s1_u.begin(), [](unsigned char c) {return std::toupper(c); });
    std::transform(s2_u.begin(), s2_u.end(), s2_u.begin(), [](unsigned char c) {return std::toupper(c); });
    return (s1_u == s2_u);
}

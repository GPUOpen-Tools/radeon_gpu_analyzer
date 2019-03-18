//=================================================================
// Copyright 2017 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

// XML.
#include <tinyxml2/Include/tinyxml2.h>

// Infra.
#ifdef _WIN32
    #pragma warning(push)
    #pragma warning(disable:4309)
#endif
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osFile.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osEnvironmentVariable.h>
#include <UpdateCheckApi.h>
#ifdef _WIN32
    #pragma warning(pop)
#endif

// Backend.
#include <RadeonGPUAnalyzerBackend/Include/beStaticIsaAnalyzer.h>
#include <RadeonGPUAnalyzerBackend/Include/beUtils.h>

// Local.
#include <RadeonGPUAnalyzerCLI/Src/kcUtils.h>
#include <RadeonGPUAnalyzerCLI/Src/kcCliStringConstants.h>
#include <RadeonGPUAnalyzerCLI/Src/kcCLICommander.h>
#include <Utils/Include/rgaXMLConstants.h>
#include <Utils/Include/rgaSharedUtils.h>
#include <Utils/Include/rgLog.h>
#include <Utils/Include/rgaVersionInfo.h>

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

static const char*     STR_FOPEN_MODE_APPEND           = "a";

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


bool kcUtils::AdjustRenderingPipelineOutputFileNames(const std::string& baseOutputFileName, const std::string& defaultSuffix,
                                                     const std::string& defaultExt, const std::string& device,
                                                     beProgramPipeline& pipelineFiles)
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
        fixedFileName << outputDir.directoryPath().asString(true) << device.c_str();

        if (!originalFileName.isEmpty())
        {
            pipelineFiles.m_vertexShader << fixedFileName << "_" << originalFileName.asASCIICharArray();
            pipelineFiles.m_tessControlShader << fixedFileName << "_" << originalFileName.asASCIICharArray();
            pipelineFiles.m_tessEvaluationShader << fixedFileName << "_" << originalFileName.asASCIICharArray();
            pipelineFiles.m_geometryShader << fixedFileName << "_" << originalFileName.asASCIICharArray();
            pipelineFiles.m_fragmentShader << fixedFileName << "_" << originalFileName.asASCIICharArray();
            pipelineFiles.m_computeShader << fixedFileName << "_" << originalFileName.asASCIICharArray();
        }
        else if (!defaultSuffix.empty())
        {
            pipelineFiles.m_vertexShader << "_" << defaultSuffix.c_str();
            pipelineFiles.m_tessControlShader << "_" << defaultSuffix.c_str();
            pipelineFiles.m_tessEvaluationShader << "_" << defaultSuffix.c_str();
            pipelineFiles.m_geometryShader << "_" << defaultSuffix.c_str();
            pipelineFiles.m_fragmentShader << "_" << defaultSuffix.c_str();
            pipelineFiles.m_computeShader << "_" << defaultSuffix.c_str();
        }

        // Stage token.
        pipelineFiles.m_vertexShader << "_" << KA_CLI_STR_VERTEX_ABBREVIATION;
        pipelineFiles.m_tessControlShader << "_" << KA_CLI_STR_TESS_CTRL_ABBREVIATION;
        pipelineFiles.m_tessEvaluationShader << "_" << KA_CLI_STR_TESS_EVAL_ABBREVIATION;
        pipelineFiles.m_geometryShader << "_" << KA_CLI_STR_GEOMETRY_ABBREVIATION;
        pipelineFiles.m_fragmentShader << "_" << KA_CLI_STR_FRAGMENT_ABBREVIATION;
        pipelineFiles.m_computeShader << "_" << KA_CLI_STR_COMPUTE_ABBREVIATION;

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

bool kcUtils::DeleteFile(const std::string& fileFullPath)
{
    gtString gPath;
    gPath << fileFullPath.c_str();
    return DeleteFile(gPath);
}

void kcUtils::ReplaceStatisticsFile(const gtString& statisticsFile, const Config& config,
                                    const std::string& device, IStatisticsParser& statsParser, LoggingCallBackFunc_t logCb)
{
    // Parse the backend statistics.
    beKA::AnalysisData statistics;
    statsParser.ParseStatistics(device, statisticsFile, statistics);

    // Delete the older statistics file.
    kcUtils::DeleteFile(statisticsFile);

    // Create a new statistics file in the CLI format.
    kcUtils::CreateStatisticsFile(statisticsFile, config, device, statistics, logCb);
}

bool kcUtils::PerformLiveRegisterAnalysis(const gtString& isaFileName, const gtString& outputFileName,
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

    return (rc == beStatus_SUCCESS);
}

bool kcUtils::GenerateControlFlowGraph(const gtString& isaFileName, const gtString& outputFileName,
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

    return (rc == beStatus_SUCCESS);
}

void kcUtils::ConstructOutputFileName(const std::string& baseOutputFileName, const std::string& defaultSuffix,
                                      const std::string& defaultExtension, const std::string& kernelName,
                                      const std::string& deviceName, gtString& generatedFileName)
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
    gtString fixedFileName, suffix;
    fixedFileName << deviceName.c_str();
    suffix << defaultSuffix.c_str();

    if (!kernelName.empty())
    {
        if (!fixedFileName.isEmpty())
        {
            fixedFileName << "_";
        }

        fixedFileName << kernelName.c_str();
    }

    if (!suffix.isEmpty() || !fileName.isEmpty())
    {
        fixedFileName << (fixedFileName.isEmpty() ? "" : "_") << (fileName.isEmpty() ? suffix : fileName);
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

void kcUtils::ConstructOutputFileName(const std::string& baseOutputFileName, const std::string& defaultSuffix,
                                      const std::string& defaultExtension, const std::string& entryPointName,
                                      const std::string& deviceName, std::string& generatedFileName)
{
    gtString  gOutFileName;
    ConstructOutputFileName(baseOutputFileName, defaultSuffix, defaultExtension, entryPointName, deviceName, gOutFileName);
    generatedFileName = gOutFileName.asASCIICharArray();
}

void kcUtils::AppendSuffix(std::string& fileName, const std::string& suffix)
{
    if (!suffix.empty())
    {
        gtString gFileName, gSuffix;
        gFileName << fileName.c_str();
        gSuffix << suffix.c_str();
        osFilePath filePath(gFileName);
        gtString baseFileName;
        filePath.getFileName(baseFileName);
        baseFileName += L"_";
        baseFileName += gSuffix;
        filePath.setFileName(baseFileName);
        fileName = filePath.asString().asASCIICharArray();
    }
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

        // Ensure that the file name is unique for any invocation by the existing CLI process.
        static unsigned uniqueSuffix = 0;

        uint32_t suffixNum = 0;
        while (tempFilePath.exists() && suffixNum < MAX_ATTEMPTS)
        {
            tempFileName = tempFileBaseName;
            tempFileName.appendUnsignedIntNumber(suffixNum++);
            tempFileName.appendUnsignedIntNumber(uniqueSuffix++);
            tempFileName.append(L".");
            tempFileName.append(ext);
            tempFilePath.setFileName(tempFileName);
        }

        ret = suffixNum < MAX_ATTEMPTS ? tempFilePath.asString() : L"";
    }

    return ret;
}

std::string kcUtils::ConstructTempFileName(const std::string & prefix, const std::string & ext)
{
    gtString gPrefix, gExt, gFileName;
    gPrefix << prefix.c_str();
    gExt << ext.c_str();
    gFileName = ConstructTempFileName(gPrefix, gExt);
    return gFileName.asASCIICharArray();
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

bool kcUtils::PrintAsicList(const std::set<std::string>& requiredDevices, const std::set<std::string>& disabledDevices)
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
            {
                for (auto & d : list)
                    if (kcUtils::ToLower(device).find(kcUtils::ToLower(d)) != std::string::npos)
                        return true;
                return false;
            };

            if ((requiredDevices.empty() || isInDeviceList(requiredDevices, pair.first)) &&
                (disabledDevices.empty() || !isInDeviceList(disabledDevices, pair.first)))
            {
                rgLog::stdOut << rgLog::noflush << pair.first << std::endl;
                for (const std::string& card : pair.second)
                {
                    // Filter out internal names.
                    if (card.find(FILTER_INDICATOR_1) == std::string::npos &&
                        card.find(FILTER_INDICATOR_2) == std::string::npos)
                    {
                        rgLog::stdOut << "\t" << card << std::endl;
                    }
                }
                rgLog::stdOut << rgLog::flush;
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

        // Wrap the file name with double quotes in case that it
        // contains a whitespace.
        if (logFileName.find_first_not_of(' ') != std::string::npos)
        {
            std::stringstream wrappedFileName;
            wrappedFileName << "\"" << logFileName << "\"";
            logFileName = wrappedFileName.str();
        }

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

std::string kcUtils::ToLower(const std::string& str)
{
    std::string lstr = str;
    std::transform(lstr.begin(), lstr.end(), lstr.begin(), [](const char& c) {return std::tolower(c);});
    return lstr;
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

#ifdef _WIN32
kcUtils::ProcessStatus kcUtils::LaunchProcess(const std::string& execPath, const std::string& args, const std::string& dir,
    unsigned long timeOut, bool printCmd, std::string& stdOut, std::string& stdErr, long& exitCode)
{
    ProcessStatus status = ProcessStatus::Success;
    exitCode = 0;

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

    // Log the invocation event.
    std::stringstream msg;
    msg << KC_STR_LAUNCH_EXTERNAL_PROCESS << execPath.c_str()
        << " " << args.c_str();

    rgLog::file << msg.str() << std::endl;

    if (printCmd)
    {
        rgLog::stdOut << msg.str() << std::endl;
    }

    // Construct the invocation command.
    std::stringstream cmd;
    cmd << execPath.c_str() << " " << args.c_str();

    // Launch the process.
    bool shouldCancel = false;
    gtString workingDir = workDir.asString();
    gtString cmdOutput;
    gtString cmdOutputErr;
    bool isLaunchSuccess = osExecAndGrabOutputAndError(cmd.str().c_str(), shouldCancel,
        workingDir, cmdOutput, cmdOutputErr);

    // Read stdout and stderr.
    stdOut = cmdOutput.asASCIICharArray();
    stdErr = cmdOutputErr.asASCIICharArray();


    if (!isLaunchSuccess)
    {
        status = ProcessStatus::LaunchFailed;
    }

    return status;
}
#else
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
        std::stringstream msg;
        msg << KC_STR_LAUNCH_EXTERNAL_PROCESS << gtExecPath.asASCIICharArray()
            << " " << gtArgs.asASCIICharArray();

        rgLog::file << msg.str() << std::endl;

        if (printCmd)
        {
            rgLog::stdOut << msg.str() << std::endl;
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
#endif

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
                            ret = (std::remove(path.asString().asASCIICharArray()) == 0);
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
    bool  status = DeleteOldLogs();
    if (!status)
    {
        rgLog::stdErr << STR_WRN_FAILED_DELETE_LOG_FILES << std::endl;
    }

    std::string  logFileName = config.m_logFile;
    if (logFileName.empty())
    {
        gtString gFileName = kcUtils::ConstructTempFileName(RGA_CLI_LOG_FILE_NAME, RGA_CLI_LOG_FILE_EXT);
        logFileName = gFileName.asASCIICharArray();
    }

    if ((status = !logFileName.empty()) == true)
    {
        struct tm   tt;
        status = CurrentTime(tt);

        auto ZeroExt = [](int n) { std::string nS = std::to_string(n); return (n < 10 ? std::string("0") + nS : nS); };

        // Add time prefix to the log file name if file name is not specified by the "--log" option.
        if (config.m_logFile.empty())
        {
            // Append current date/time to the log file name.
            std::stringstream  suffix;
            suffix << "-" << std::to_string(tt.tm_year + 1900) << ZeroExt(tt.tm_mon + 1) << ZeroExt(tt.tm_mday) <<
                "-" << ZeroExt(tt.tm_hour) << ZeroExt(tt.tm_min) << ZeroExt(tt.tm_sec);

            size_t  extOffset = logFileName.rfind('.');
            logFileName.insert((extOffset == std::string::npos ? logFileName.size() : extOffset), suffix.str());
        }

        // Open log file.
        if ((status = rgLog::OpenLogFile(logFileName)) == true)
        {
            rgLog::file << KC_STR_CLI_LOG_HEADER << std::endl;
        }
        else
        {
            rgLog::stdErr << STR_ERR_FAILED_OPEN_LOG_FILE << logFileName << std::endl;
        }
    }

    return status;
}

bool kcUtils::StrCmpNoCase(const std::string & s1, const std::string & s2)
{
    std::string s1_u = s1, s2_u = s2;
    std::transform(s1_u.begin(), s1_u.end(), s1_u.begin(), [](unsigned char c) {return std::toupper(c);});
    std::transform(s2_u.begin(), s2_u.end(), s2_u.begin(), [](unsigned char c) {return std::toupper(c);});
    return (s1_u == s2_u);
}

std::string kcUtils::GetFileExt(const std::string & filePath)
{
    // Try deducing from the extension.
    gtString  gfilePath;
    gfilePath << filePath.c_str();
    osFilePath path(gfilePath);
    gtString gFileExt;
    return (path.getFileExtension(gFileExt) ? gFileExt.asASCIICharArray() : "");
}

bool kcUtils::SetEnvrironmentVariable(const::std::string& varName, const std::string varValue)
{
    // Convert to gtString.
    gtString varNameGtStr;
    gtString varValueGtStr;
    varNameGtStr << varName.c_str();
    varValueGtStr << varValue.c_str();

    // Set the environment variable.
    osEnvironmentVariable envVarVkLoaderDebug(varNameGtStr, varValueGtStr);
    bool ret = osSetCurrentProcessEnvVariable(envVarVkLoaderDebug);

    return ret;
}

void kcUtils::CheckForUpdates()
{
    UpdateCheck::VersionInfo rgaCliVersion;
    std::string buildDateString(STR_RGA_BUILD_DATE);

    if (buildDateString == STR_RGA_BUILD_DATE_DEV)
    {
        // Pretend a dev build has no version so that
        // all public versions are reported as being newer.
        rgaCliVersion.m_major = 0;
        rgaCliVersion.m_minor = 0;
        rgaCliVersion.m_patch = 0;
        rgaCliVersion.m_build = 0;
    }
    else
    {
        rgaCliVersion.m_major = RGA_VERSION_MAJOR;
        rgaCliVersion.m_minor = RGA_VERSION_MINOR;
        rgaCliVersion.m_patch = RGA_VERSION_UPDATE;
        rgaCliVersion.m_build = 0;
    }

    UpdateCheck::UpdateInfo updateInfo;
    string errorMessage;

    cout << "Checking for update... ";

    bool checkedForUpdate = UpdateCheck::CheckForUpdates(rgaCliVersion, STR_RGA_UPDATECHECK_URL, STR_RGA_UPDATECHECK_ASSET_NAME, updateInfo, errorMessage);
    if (!checkedForUpdate)
    {
        cout << "Unable to find update: " << errorMessage << endl;
    }
    else
    {
        if (!updateInfo.m_isUpdateAvailable)
        {
            cout << "No new updates available." << endl;
        }
        else
        {
            cout << "New version available!" << endl;

            for (std::vector<UpdateCheck::ReleaseInfo>::const_iterator releaseIter = updateInfo.m_releases.cbegin(); releaseIter != updateInfo.m_releases.cend(); ++releaseIter)
            {
                cout << "Description: " << releaseIter->m_title << endl;
                cout << "Version: " << releaseIter->m_version.ToString() << " (" << UpdateCheck::ReleaseTypeToString(releaseIter->m_type) << ")" << endl;
                cout << "Released: " << releaseIter->m_date << endl;

                if (!releaseIter->m_tags.empty())
                {
                    cout << "Tags: " << releaseIter->m_tags[0];

                    for (uint32_t i = 1; i < releaseIter->m_tags.size(); ++i)
                    {
                        cout << ", " << releaseIter->m_tags[i];
                    }

                    cout << endl;
                }

                if (releaseIter->m_downloadLinks.size() > 0)
                {
                    cout << "Download a package from:" << endl;

                    for (size_t packageIndex = 0; packageIndex < releaseIter->m_downloadLinks.size(); ++packageIndex)
                    {
                        cout << "   " << releaseIter->m_downloadLinks[packageIndex].m_url << endl;
                    }
                }

                if (releaseIter->m_infoLinks.size() > 0)
                {
                    cout << "For more information, visit:" << endl;

                    for (size_t infoLinkIndex = 0; infoLinkIndex < releaseIter->m_infoLinks.size(); ++infoLinkIndex)
                    {
                        cout << "   " << releaseIter->m_infoLinks[infoLinkIndex].m_url << endl;
                    }
                }

                // Add an extra line at the end in case there are mutiple versions available.
                cout << endl;
            }
        }
    }
}

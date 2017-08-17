//=================================================================
// Copyright 2017 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

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

using namespace beKA;

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


bool kcUtils::AdjustRenderingPipelineOutputFileNames(const std::string& baseOutputFileName,
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

        if (!originalFileExtension.isEmpty())
        {
            pipelineFiles.m_vertexShader << "." << originalFileExtension.asASCIICharArray();
            pipelineFiles.m_tessControlShader << "." << originalFileExtension.asASCIICharArray();
            pipelineFiles.m_tessEvaluationShader << "." << originalFileExtension.asASCIICharArray();
            pipelineFiles.m_geometryShader << "." << originalFileExtension.asASCIICharArray();
            pipelineFiles.m_fragmentShader << "." << originalFileExtension.asASCIICharArray();
            pipelineFiles.m_computeShader << "." << originalFileExtension.asASCIICharArray();
        }
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
    output << statistics.maxScratchRegsNeeded << csvSeparator;

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

    // Available VGPRs.
    output << statistics.numVGPRsAvailable << csvSeparator;

    // Used VGPRs.
    output << statistics.numVGPRsUsed << csvSeparator;

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
    output << STR_CSV_HEADER_SCRATCH_REGS << csvSeparator;
    output << STR_CSV_HEADER_THREADS_PER_WG << csvSeparator;
    output << STR_CSV_HEADER_WAVEFRONT_SIZE << csvSeparator;
    output << STR_CSV_HEADER_LDS_BYTES_MAX << csvSeparator;
    output << STR_CSV_HEADER_LDS_BYTES_ACTUAL << csvSeparator;
    output << STR_CSV_HEADER_SGPR_AVAILABLE << csvSeparator;
    output << STR_CSV_HEADER_SGPR_USED << csvSeparator;
    output << STR_CSV_HEADER_VGPR_AVAILABLE << csvSeparator;
    output << STR_CSV_HEADER_VGPR_USED << csvSeparator;
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

void kcUtils::PerformLiveRegisterAnalysis(const gtString& isaFileName,
                                          const gtString& outputFileName, LoggingCallBackFunc_t pCallback)
{
    // Call the backend.
    beStatus rc = beStaticIsaAnalyzer::PerformLiveRegisterAnalysis(isaFileName, outputFileName);

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

void kcUtils::GenerateControlFlowGraph(const gtString& isaFileName, const gtString& outputFileName, LoggingCallBackFunc_t pCallback)
{
    // Call the backend.
    beStatus rc = beStaticIsaAnalyzer::GenerateControlFlowGraph(isaFileName, outputFileName);

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
                // Failed to launch the ISA analyzer.
                msg << STR_ERR_CANNOT_LAUNCH_CFG_ANALYZER << std::endl;
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

void kcUtils::ConstructOutputFileName(const std::string& baseOutputFileName, const std::string& defaultExtension, const std::string& kernelName, const std::string& deviceName, gtString& generatedFileName)
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
        tempFileName.append(ext);
        tempFilePath.setFileName(tempFileName);

        uint32_t suffixNum = 0;
        while (tempFilePath.exists() && suffixNum < MAX_ATTEMPTS)
        {
            tempFileName = tempFileBaseName;
            tempFileName.appendUnsignedIntNumber(suffixNum++);
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
// Returns the message in "outMessage" string.
static bool ResolveMatchedDevices(const kcUtils::DeviceNameMap& matchedDevices, const std::string& device, std::string& outMessage)
{
    bool  status = false;
    std::stringstream  msg;

    // Routine printing the architecture name and all its device names.
    auto  PrintArchAndDevices = [&](const kcUtils::DeviceNameMap::value_type& arch)
    {
        msg << arch.first << std::endl;
        for (const std::string& marketingName : arch.second)
        {
            msg << "\t" << marketingName << std::endl;
        }
    };

    if (matchedDevices.size() == 0)
    {
        // No matching architectures found. Failure.
        msg << STR_ERR_COULD_NOT_DETECT_TARGET << device << std::endl;
    }
    else if (matchedDevices.size() == 1)
    {
        // Found exactly one GPU architectire. Success.
        msg << STR_TARGET_DETECTED << std::endl << std::endl;
        PrintArchAndDevices(*(matchedDevices.begin()));
        status = true;
    }
    else
    {
        // Found multiple GPU architectures. Failure.
        msg << STR_ERR_AMBIGUOUS_TARGET << device << std::endl << std::endl;
        for (auto& arch : matchedDevices)
        {
            PrintArchAndDevices(arch);
        }
    }
    msg << std::endl;
    outMessage = msg.str();

    return status;
}

bool kcUtils::FindGPUArchName(const std::string & deviceName, std::string & archName, std::string& msg)
{
    bool  status = false;
    const char* FILTER_INDICATOR_1 = ":";
    const char* FILTER_INDICATOR_2 = "Not Used";

    // Mappings  "architecture <--> marketing names"
    DeviceNameMap  matchedDevices, cardsMapping;

    // Transform the device name to lower case and remove spaces.
    std::string  reducedName = deviceName;
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
    if (ResolveMatchedDevices(matchedDevices, deviceName, msg))
    {
        archName = (*(matchedDevices.begin())).first;
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

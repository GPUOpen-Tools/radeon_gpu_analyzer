//=================================================================
// Copyright 2017 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

// C++.
#include <vector>
#include <map>
#include <utility>
#include <sstream>
#include <algorithm>

// Infra.
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTBaseTools/Include/gtAssert.h>

// Local.
#include <RadeonGPUAnalyzerCLI/src/kcCLICommanderCL.h>
#include <RadeonGPUAnalyzerCLI/src/kcCLICommanderLightning.h>
#include <RadeonGPUAnalyzerBackend/include/beProgramBuilderOpenCL.h>
#include <RadeonGPUAnalyzerCLI/src/kcCliStringConstants.h>
#include <RadeonGPUAnalyzerCLI/src/kcUtils.h>

// Analyzer
#include <VersionInfo/VersionInfo.h>

// Backend.
#include <DeviceInfoUtils.h>

// *****************************************
// *** INTERNALLY LINKED SYMBOLS - START ***
// *****************************************

static const std::string  STR_GFX804_TARGET_NAME = "gfx804";

static const std::set<std::string>  UnsupportedTargets = { STR_GFX804_TARGET_NAME };

// ***************************************
// *** INTERNALLY LINKED SYMBOLS - END ***
// ***************************************

kcCLICommanderCL::kcCLICommanderCL() : m_isAllKernels(false)
{

}

kcCLICommanderCL::~kcCLICommanderCL()
{
    // No need to call DeleteInstance. The base class singleton performs this.
}

// Obsolete: this routine is being used in the generation of the statistics CSV file.
// It should be removed after this mechanism is refactored.
template<class T> std::string doNAFormat(T ui, T sentinal, T err, char listSeparator, bool shouldAddSeparator /*= true*/)
{
    stringstream s;

    if (ui == sentinal)
    {
        s << "n/a";

        if (shouldAddSeparator)
        {
            s << listSeparator;
        }
    }
    else if (ui == err)
    {
        s << "err";

        if (shouldAddSeparator)
        {
            s << listSeparator;
        }
    }
    else
    {
        s << ui;

        if (shouldAddSeparator)
        {
            s << listSeparator;
        }
    }

    return s.str();
}

template<class T>
static string
doNAFormat(T ui, T sentinal, T err, char listSeparator)
{
    stringstream s;

    if (ui == sentinal)
    {
        s << "n/a" << listSeparator;
    }
    else if (ui == err)
    {
        s << "err" << listSeparator;
    }
    else
    {
        s << ui << listSeparator;
    }

    return s.str();
}



bool kcCLICommanderCL::Init(const Config& config, LoggingCallBackFunc_t callback)
{
    GT_UNREFERENCED_PARAMETER(config);
    m_LogCallback = callback;

    // Initialize the backend.
    be = Backend::Instance();
    beKA::beStatus beRet = be->Initialize(BuiltProgramKind_OpenCL, callback);
    bool ret = (beRet == beKA::beStatus_SUCCESS);

    if (ret)
    {
        // Initialize the devices list.
        std::set<std::string>  devices;
        beRet = be->theOpenCLBuilder()->GetDevices(devices);
        ret = (beRet == beKA::beStatus_SUCCESS);

        // Only external (non-placeholder) and based on CXL version devices should be used.
        if (ret)
        {
            beKA::beStatus beRetInner = be->theOpenCLBuilder()->GetDeviceTable(m_table);
            ret = (beRetInner == beKA::beStatus_SUCCESS);

            if (ret)
            {
                for (vector<GDT_GfxCardInfo>::const_iterator it = m_table.begin(); it != m_table.end(); ++it)
                {
                    if ((devices.find(it->m_szCALName) != devices.end()) && UnsupportedTargets.count(it->m_szCALName) == 0)
                    {
                        m_externalDevices.insert(it->m_szCALName);
                    }
                }
            }
        }
    }

    return ret;
}

bool kcCLICommanderCL::Compile(const Config& config)
{
    bool bRet = false;

    // Verify that an input file was specified
    if (config.m_InputFiles.size() != 1 || config.m_InputFiles[0].empty())
    {
        std::stringstream logStream;
        logStream << STR_ERR_ONE_INPUT_FILE_EXPECTED << std::endl;
        LogCallBack(logStream.str());
    }
    else
    {
        string sSource;
        bRet = kcUtils::ReadProgramSource(config.m_InputFiles[0], sSource);

        if (!bRet)
        {
            std::stringstream logStream;
            logStream << STR_ERR_CANNOT_READ_FILE << config.m_InputFiles[0] << endl;
            LogCallBack(logStream.str());
        }
        else
        {
            OpenCLOptions options;
            options.m_SourceLanguage = SourceLanguage_OpenCL;
            options.m_selectedDevices = m_asics;
            options.m_defines = config.m_Defines;
            options.m_openCLCompileOptions = config.m_OpenCLOptions;

            int numOfSuccessFulBuilds = 0;
            beKA::beStatus beRet;

            if (config.m_IncludePath.size() > 0)
            {
                beRet = be->theOpenCLBuilder()->Compile(sSource, options, config.m_InputFiles[0], &config.m_IncludePath, numOfSuccessFulBuilds);
            }
            else
            {
                beRet = be->theOpenCLBuilder()->Compile(sSource, options, config.m_InputFiles[0], NULL, numOfSuccessFulBuilds);
            }

            if (beRet == beKA::beStatus_SUCCESS)
            {
                bRet = true;
            }
            else
            {
                bRet = false;
            }
        }
    }

    return bRet;
}

void kcCLICommanderCL::RunCompileCommands(const Config& config, LoggingCallBackFunc_t callback)
{
    if (Init(config, callback))
    {
        if (InitRequestedAsicList(config, m_externalDevices, m_asics, false))
        {
            if (Compile(config))
            {
                InitRequiredKernels(config, m_asics, m_requiredKernels);

                GetBinary(config);

                GetISAText(config);

                GetILText(config);

                Analysis(config);

                GetMetadata(config);
            }
        }
    }
}

void kcCLICommanderCL::Analysis(const Config& config)
{
    if (config.m_AnalysisFile.size() == 0)
    {
        return;
    }

    if (m_requiredKernels.size() == 0)
    {
        std::stringstream s_Log;
        s_Log << STR_ERR_NO_KERNELS_FOR_ANALYSIS << endl;
        LogCallBack(s_Log.str());
    }

    if ((config.m_SuppressSection.size() > 0) && (config.m_BinaryOutputFile.size() == 0))
    {
        std::stringstream s_Log;
        s_Log << STR_WRN_CL_SUPPRESS_WIHTOUT_BINARY << std::endl;
        LogCallBack(s_Log.str());
    }

    // Get the separator for CSV list items.
    char csvSeparator = kcUtils::GetCsvSeparator(config);

    // Get analysis for devices.
    for (const std::string& deviceName : m_asics)
    {
        for (const std::string& kernelName : m_requiredKernels)
        {
            // Show the analysis only for external devices.
            if (m_externalDevices.find(deviceName) == m_externalDevices.end())
            {
                continue;
            }

            beStatus status;

            // Only do GPU devices.
            cl_device_type deviceType;
            status = be->theOpenCLBuilder()->GetDeviceType(deviceName, deviceType);

            if (status != beStatus_SUCCESS ||
                deviceType != CL_DEVICE_TYPE_GPU)
            {
                std::stringstream s_Log;
                s_Log << "Info: Skipping analysis of CPU device '" << deviceName << "'." << endl;
                LogCallBack(s_Log.str());
                continue;
            }

            AnalysisData analysis;
            (void)memset(&analysis, 0, sizeof(analysis));
            status = be->theOpenCLBuilder()->GetStatistics(deviceName, kernelName, analysis);

            if (status != beStatus_SUCCESS)
            {
                if (status == beStatus_WrongKernelName)
                {
                    std::stringstream s_Log;
                    s_Log << "Info: Skipping analysis, wrong kernel name provided: '" << config.m_Function << "'." << endl;
                    LogCallBack(s_Log.str());
                }

                continue;
            }

            // Create the output file.
            ofstream output;
            gtString statsOutputFileName;
            kcUtils::ConstructOutputFileName(config.m_AnalysisFile, KC_STR_DEFAULT_STATISTICS_SUFFIX, kernelName, deviceName, statsOutputFileName);
            osFilePath analysisOutputPath;
            analysisOutputPath.setFullPathFromString(statsOutputFileName);
            osDirectory targetDir;
            analysisOutputPath.getFileDirectory(targetDir);
            gtString targetDirAsStr = targetDir.directoryPath().asString(true);
            analysisOutputPath.setFileDirectory(targetDirAsStr);

            // Create the target directory if it does not exist.
            if (!targetDir.IsEmpty() && !targetDir.exists())
            {
                bool isTargetDirCreated(targetDir.create());

                if (!isTargetDirCreated)
                {
                    std::stringstream errMsg;
                    errMsg << STR_ERR_CANNOT_FIND_OUTPUT_DIR << std::endl;
                    LogCallBack(errMsg.str());
                }
            }

            output.open(analysisOutputPath.asString().asASCIICharArray());

            if (!output.is_open())
            {
                std::stringstream s_Log;
                s_Log << "Error: Unable to open " << config.m_AnalysisFile << " for write.\n";
                LogCallBack(s_Log.str());
            }
            else
            {
                // Write the headers.
                output << kcUtils::GetStatisticsCsvHeaderString(csvSeparator) << std::endl;

                // Write a line of CSV.
                output << deviceName << csvSeparator;
                output << analysis.scratchMemoryUsed << csvSeparator;
                output << analysis.numThreadPerGroup << csvSeparator;
                output << doNAFormat(analysis.wavefrontSize, CAL_NA_Value_64, CAL_ERR_Value_64, csvSeparator);
                output << analysis.LDSSizeAvailable << csvSeparator;
                output << analysis.LDSSizeUsed << csvSeparator;
                output << doNAFormat(analysis.numSGPRsAvailable, CAL_NA_Value_64, CAL_ERR_Value_64, csvSeparator);
                output << doNAFormat(analysis.numSGPRsUsed, CAL_NA_Value_64, CAL_ERR_Value_64, csvSeparator);
                output << doNAFormat(CAL_NA_Value_64, CAL_NA_Value_64, CAL_ERR_Value_64, csvSeparator);
                output << doNAFormat(analysis.numVGPRsAvailable, CAL_NA_Value_64, CAL_ERR_Value_64, csvSeparator);
                output << doNAFormat(analysis.numVGPRsUsed, CAL_NA_Value_64, CAL_ERR_Value_64, csvSeparator);
                output << doNAFormat(CAL_NA_Value_64, CAL_NA_Value_64, CAL_ERR_Value_64, csvSeparator);
                output << doNAFormat(analysis.numThreadPerGroupX, CAL_NA_Value_64, CAL_ERR_Value_64, csvSeparator);
                output << doNAFormat(analysis.numThreadPerGroupY, CAL_NA_Value_64, CAL_ERR_Value_64, csvSeparator);
                output << doNAFormat(analysis.numThreadPerGroupZ, CAL_NA_Value_64, CAL_ERR_Value_64, csvSeparator);
                output << doNAFormat(analysis.ISASize, (CALuint64)0, CAL_ERR_Value_64, csvSeparator, false);
                output << std::endl;

                // Close the output file.
                output.close();
            }
        }
    }
}

void kcCLICommanderCL::GetILText(const Config& config)
{
    if (!config.m_ILFile.empty())
    {
        if (!config.m_ILFile.empty() && !config.m_Function.empty() && be != nullptr)
        {
            beProgramBuilderOpenCL* pBuilder = be->theOpenCLBuilder();

            if (pBuilder != nullptr)
            {
                if ((config.m_SuppressSection.size() > 0) && (config.m_BinaryOutputFile.size() == 0))
                {
                    // Print the warning message.
                    std::stringstream msg;
                    msg << STR_WRN_CL_SUPPRESS_WIHTOUT_BINARY << std::endl;
                    LogCallBack(msg.str());
                }

                // This variable will hold the IL text.
                std::string ilTextBuffer;

                // Get IL text and make output files.
                for (const std::string& deviceName : m_asics)
                {
                    for (const std::string& kernelName : m_requiredKernels)
                    {
                        beKA::beStatus status = pBuilder->GetKernelILText(deviceName, kernelName, ilTextBuffer);

                        if (status == beStatus_SUCCESS)
                        {
                            gtString ilOutputFileName;
                            kcUtils::ConstructOutputFileName(config.m_ILFile,
                                                             KC_STR_DEFAULT_AMD_IL_SUFFIX, kernelName, deviceName, ilOutputFileName);
                            kcUtils::WriteTextFile(ilOutputFileName.asASCIICharArray(), ilTextBuffer, m_LogCallback);
                        }
                        else
                        {
                            // Inform the user.
                            std::stringstream msg;
                            msg << STR_ERR_CANNOT_DISASSEMBLE_AMD_IL << " for " << deviceName;

                            // If we have the kernel name - specify it in the error message.
                            if (!kernelName.empty())
                            {
                                msg << " (kernel: " << kernelName << ")";
                            }

                            // Print the message.
                            msg << "." << std::endl;
                            m_LogCallback(msg.str().c_str());
                        }

                        // Clear the output buffer.
                        ilTextBuffer.clear();
                    }
                }
            }
        }
    }
}

void kcCLICommanderCL::GetISAText(const Config& config)
{
    if (!config.m_ISAFile.empty() || !config.m_AnalysisFile.empty() ||
        !config.m_blockCFGFile.empty() || !config.m_instCFGFile.empty() || !config.m_LiveRegisterAnalysisFile.empty())
    {
        bool isIsaFileTemp = config.m_ISAFile.empty();

        if ((config.m_SuppressSection.size() > 0) && (config.m_BinaryOutputFile.size() == 0))
        {
            // Print the warning message.
            std::stringstream msg;
            msg << STR_WRN_CL_SUPPRESS_WIHTOUT_BINARY << std::endl;
            LogCallBack(msg.str());
        }

        beProgramBuilderOpenCL* pClBuilder = be->theOpenCLBuilder();

        if (pClBuilder != NULL)
        {
            std::string sISAIL;
            bool shouldCreateSubDirectories = m_isAllKernels && !m_requiredKernels.empty();
            GT_UNREFERENCED_PARAMETER(shouldCreateSubDirectories);

            // Get ISA text and make output files.
            for (const std::string& deviceName : m_asics)
            {
                for (const std::string& kernelName : m_requiredKernels)
                {
                    beKA::beStatus status = pClBuilder->GetKernelISAText(deviceName, kernelName, sISAIL);

                    if (status == beStatus_SUCCESS)
                    {
                        gtString isaOutputFileName;
                        if (isIsaFileTemp)
                        {
                            gtString  isaFileName, isaFileExt;
                            isaFileName << (std::string(KC_STR_DEFAULT_ISA_OUTPUT_FILE_NAME) + deviceName + kernelName).c_str();
                            isaFileExt << KC_STR_DEFAULT_ISA_SUFFIX;
                            isaOutputFileName = kcUtils::ConstructTempFileName(isaFileName, isaFileExt);
                        }
                        else
                        {
                            kcUtils::ConstructOutputFileName(config.m_ISAFile, KC_STR_DEFAULT_ISA_SUFFIX, kernelName, deviceName, isaOutputFileName);
                        }
                        kcUtils::WriteTextFile(isaOutputFileName.asASCIICharArray(), sISAIL, m_LogCallback);

                        // Perform live register analysis.
                        bool isRegLivenessRequired = !config.m_LiveRegisterAnalysisFile.empty();

                        if (isRegLivenessRequired)
                        {
                            gtString liveRegAnalysisOutputFileName;
                            kcUtils::ConstructOutputFileName(config.m_LiveRegisterAnalysisFile, KC_STR_DEFAULT_LIVE_REG_ANALYSIS_SUFFIX,
                                                             kernelName, deviceName, liveRegAnalysisOutputFileName);

                            // Call the kcUtils routine to analyze <generatedFileName> and write
                            // the analysis file.
                            kcUtils::PerformLiveRegisterAnalysis(isaOutputFileName, liveRegAnalysisOutputFileName,
                                                                 m_LogCallback, config.m_printProcessCmdLines);
                        }

                        // Generate control flow graph.
                        if (!config.m_blockCFGFile.empty() || !config.m_instCFGFile.empty())
                        {
                            gtString cfgOutputFileName;
                            std::string baseName = (!config.m_blockCFGFile.empty() ? config.m_blockCFGFile : config.m_instCFGFile);
                            kcUtils::ConstructOutputFileName(baseName, KC_STR_DEFAULT_CFG_EXT,
                                                             kernelName, deviceName, cfgOutputFileName);

                            // Call the kcUtils routine to analyze <generatedFileName> and write
                            // the analysis file.
                            kcUtils::GenerateControlFlowGraph(isaOutputFileName, cfgOutputFileName, m_LogCallback,
                                                              !config.m_instCFGFile.empty(), config.m_printProcessCmdLines);
                        }

                        // Delete temporary files.
                        if (config.m_ISAFile.empty())
                        {
                            if (kcUtils::FileNotEmpty(isaOutputFileName.asASCIICharArray()))
                            {
                                kcUtils::DeleteFile(isaOutputFileName);
                            }
                        }
                    }
                    else
                    {
                        // Inform the user.
                        std::stringstream msg;
                        msg << STR_ERR_CANNOT_DISASSEMBLE_ISA << " for " << deviceName;

                        // If we have the kernel name - specify it in the error message.
                        if (!kernelName.empty())
                        {
                            msg << " (kernel: " << kernelName << ")";
                        }

                        // Print the message.
                        msg << "." << std::endl;
                        m_LogCallback(msg.str().c_str());
                    }

                    // Clear the output buffer.
                    sISAIL.clear();
                }
            }
        }
    }
}

void kcCLICommanderCL::GetBinary(const Config& config)
{
    if (config.m_BinaryOutputFile.size() > 0 && be != nullptr)
    {
        beProgramBuilderOpenCL* pBuilder = be->theOpenCLBuilder();

        if (pBuilder != nullptr)
        {
            // Create binary output files.
            BinaryOptions binopts;
            binopts.m_SuppressSection = config.m_SuppressSection;

            std::vector<char> binary;

            for (const std::string& deviceName : m_asics)
            {
                beStatus status = pBuilder->GetBinary(deviceName, binopts, binary);

                if (status == beStatus_SUCCESS)
                {
                    gtString binOutputFileName;
                    kcUtils::ConstructOutputFileName(config.m_BinaryOutputFile,
                                                     KC_STR_DEFAULT_BIN_SUFFIX, "", deviceName, binOutputFileName);
                    kcUtils::WriteBinaryFile(binOutputFileName.asASCIICharArray(), binary, m_LogCallback);
                }
                else
                {
                    // Inform the user.
                    std::stringstream msg;
                    msg << STR_ERR_CANNOT_EXTRACT_BINARIES << " for " << deviceName << "." << std::endl;
                    m_LogCallback(msg.str().c_str());
                }

                // Clear the output buffer.
                binary.clear();
            }
        }
    }
}

void kcCLICommanderCL::GetMetadata(const Config& config)
{
    if (config.m_MetadataFile.size() > 0 && be != nullptr)
    {
        if ((config.m_SuppressSection.size() > 0) && (config.m_BinaryOutputFile.size() == 0))
        {
            // Print the warning message.
            std::stringstream msg;
            msg << STR_WRN_CL_SUPPRESS_WIHTOUT_BINARY << std::endl;
            LogCallBack(msg.str());
        }

        beProgramBuilderOpenCL* pBuilder = be->theOpenCLBuilder();

        if (pBuilder != nullptr)
        {
            // Create the meta-data output files.
            std::string metaDataText;

            for (const std::string& deviceName : m_asics)
            {
                for (const std::string& kernelName : m_requiredKernels)
                {
                    beStatus status = pBuilder->GetKernelMetaDataText(deviceName, kernelName, metaDataText);

                    if (status == beStatus_SUCCESS)
                    {
                        gtString metaDataOutputFileName;
                        kcUtils::ConstructOutputFileName(config.m_MetadataFile, KC_STR_DEFAULT_METADATA_SUFFIX,
                                                         kernelName, deviceName, metaDataOutputFileName);
                        kcUtils::WriteTextFile(metaDataOutputFileName.asASCIICharArray(), metaDataText, m_LogCallback);
                    }
                    else if (status == beStatus_NO_METADATA_FOR_DEVICE)
                    {
                        m_LogCallback(std::string(STR_WRN_CL_METADATA_NOT_SUPPORTED_1) + deviceName + STR_WRN_CL_METADATA_NOT_SUPPORTED_2 + "\n");
                        break;
                    }
                    else
                    {
                        // Inform the user.
                        std::stringstream msg;
                        msg << STR_ERR_CANNOT_EXTRACT_META_DATA << " for " << deviceName << "(kernel: " << kernelName << ")." << std::endl;
                        m_LogCallback(msg.str().c_str());
                    }

                    // Clear the output buffer.
                    metaDataText.clear();
                }
            }
        }
    }
}

void kcCLICommanderCL::InitRequiredKernels(const Config& config, const std::set<std::string>& requiredDevices, std::vector<std::string>& requiredKernels)
{
    requiredKernels.clear();

    if (!requiredDevices.empty())
    {
        // We only need a single device name.
        std::set<std::string>::const_iterator firstDevice = requiredDevices.begin();
        const std::string& deviceName = *firstDevice;
        std::string requestedKernel = config.m_Function;
        std::transform(requestedKernel.begin(), requestedKernel.end(), requestedKernel.begin(), ::tolower);
        // Process all kernels by default or if all kernels are explicitly requested.
        m_isAllKernels = (requestedKernel.compare("all") == 0 || requestedKernel.empty());
        beProgramBuilderOpenCL* pClBuilder = be->theOpenCLBuilder();

        if (pClBuilder != NULL)
        {
            if (m_isAllKernels)
            {
                pClBuilder->GetKernels(deviceName, requiredKernels);
            }
            else
            {
                requiredKernels.push_back(config.m_Function);
            }
        }
    }
}

//=================================================================
// Copyright 2017 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

// C++.
#include <vector>
#include <map>
#include <utility>
#include <sstream>
#include <algorithm>
#include <set>

// Infra.
#ifdef _WIN32
    #pragma warning(push)
    #pragma warning(disable:4309)
#endif
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#ifdef _WIN32
    #pragma warning(pop)
#endif

// Local.
#include <RadeonGPUAnalyzerCLI/Src/kcCLICommanderCL.h>
#include <RadeonGPUAnalyzerCLI/Src/kcCLICommanderLightning.h>
#include <RadeonGPUAnalyzerBackend/Include/beProgramBuilderOpenCL.h>
#include <RadeonGPUAnalyzerCLI/Src/kcCliStringConstants.h>
#include <RadeonGPUAnalyzerCLI/Src/kcUtils.h>

// Analyzer
#include <VersionInfo/VersionInfo.h>

// Backend.
#include <DeviceInfoUtils.h>
#include <RadeonGPUAnalyzerBackend/Include/beUtils.h>

// *****************************************
// *** INTERNALLY LINKED SYMBOLS - START ***
// *****************************************

static const std::string  STR_GFX804_TARGET_NAME = "gfx804";
static const char*  STR_WARNING_LIVEREG_NOT_SUPPORTED_FOR_ISA_A = "Warning: live register analysis cannot be performed on the generated ISA for ";
static const char*  STR_WARNING_LIVEREG_NOT_SUPPORTED_FOR_ISA_B = " - skipping.";
static const char*  STR_WARNING_CFG_NOT_SUPPORTED_FOR_ISA_A = "Warning: cfg cannot be generated from the ISA for ";
static const char*  STR_WARNING_CFG_NOT_SUPPORTED_FOR_ISA_B = " -skipping.";
static const char*  STR_WARNING_IL_NOT_SUPPORTED_FOR_RDNA_A = "Warning: IL disassembly extraction not supported for RDNA target ";
static const char*  STR_WARNING_IL_NOT_SUPPORTED_FOR_RDNA_B = " - skipping.";
static const char*  STR_ERROR_ISA_REQUIRED = "Error: --isa is required for post processing (live register analysis and cfg generation).";
static const char*  STR_STATS_NA = "n/a";
static const char*  STR_INFO_SUCCESS = "Success.";

static const std::set<std::string>  UnsupportedTargets = { STR_GFX804_TARGET_NAME };

static bool IsInputValid(const Config& config)
{
    bool ret = true;

    // ISA is required for post-processing.
    if (config.m_ISAFile.empty() && (!config.m_blockCFGFile.empty() || !config.m_instCFGFile.empty()
        || !config.m_LiveRegisterAnalysisFile.empty()))
    {
        std::cout << STR_ERROR_ISA_REQUIRED << std::endl;
        ret = false;
    }

    return ret;
}

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
        s << STR_STATS_NA << listSeparator;
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

        // Apply any filters to the targets here.
        std::set<std::string> filteredTargets;
        for (const std::string& target : devices)
        {
            filteredTargets.insert(target);
        }

        // Only external (non-placeholder) and based on CXL version devices should be used.
        if (ret)
        {
            beKA::beStatus beRetInner = be->theOpenCLBuilder()->GetDeviceTable(m_table);
            ret = (beRetInner == beKA::beStatus_SUCCESS);

            if (ret)
            {
                for (vector<GDT_GfxCardInfo>::const_iterator it = m_table.begin(); it != m_table.end(); ++it)
                {
                    if ((filteredTargets.find(it->m_szCALName) != filteredTargets.end()) &&
                        UnsupportedTargets.count(it->m_szCALName) == 0)
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
    bool ret = IsInputValid(config);
    if (ret)
    {
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
            ret = kcUtils::ReadProgramSource(config.m_InputFiles[0], sSource);

            if (!ret)
            {
                std::stringstream logStream;
                logStream << STR_ERR_CANNOT_READ_FILE << config.m_InputFiles[0] << endl;
                LogCallBack(logStream.str());
            }
            else
            {
                OpenCLOptions options;
                options.m_mode = Mode_OpenCL;
                options.m_selectedDevicesSorted = m_asicsSorted;
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
                    ret = true;
                }
                else
                {
                    ret = false;
                }
            }
        }
    }

    return ret;
}

bool kcCLICommanderCL::ListEntries(const Config& config, LoggingCallBackFunc_t callback)
{
    return kcCLICommanderLightning::ListEntriesRocmCL(config, callback);
}

void kcCLICommanderCL::RunCompileCommands(const Config& config, LoggingCallBackFunc_t callback)
{
    if (Init(config, callback))
    {
        if (InitRequestedAsicList(config.m_ASICs, config.m_mode, m_externalDevices, m_asics, false))
        {

            // Sort the targets.
            for (const std::string& target : m_asics)
            {
                m_asicsSorted.push_back(target);
            }
            std::sort(m_asicsSorted.begin(), m_asicsSorted.end(), &beUtils::DeviceNameLessThan);

            if (Compile(config))
            {
                InitRequiredKernels(config, m_asicsSorted, m_requiredKernels);

                GetBinary(config);

                GetISAText(config);

                GetILText(config);

                Analysis(config);

                GetMetadata(config);
            }
        }
    }
}

bool kcCLICommanderCL::PrintAsicList(const Config&)
{
    // We do not want to display names that contain these strings.
    const char* FILTER_INDICATOR_1 = ":";
    const char* FILTER_INDICATOR_2 = "Not Used";

    bool  result = false;
    std::map<std::string, std::set<std::string>> cardsMapping;
    bool rc = kcUtils::GetMarketingNameToCodenameMapping(cardsMapping);

    // Sort the mappings.
    std::map<std::string, std::set<std::string>,
        decltype(&beUtils::DeviceNameLessThan)> cardsMappingSorted(cardsMapping.begin(),
            cardsMapping.end(), &beUtils::DeviceNameLessThan);

    if (rc && !cardsMappingSorted.empty())
    {
        for (const auto& pair : cardsMappingSorted)
        {
            std::cout << pair.first << std::endl;
            for (const std::string& card : pair.second)
            {
                // Filter out internal names.
                if (card.find(FILTER_INDICATOR_1) == std::string::npos &&
                    card.find(FILTER_INDICATOR_2) == std::string::npos)
                {
                    std::cout << "\t" << card << std::endl;
                }
            }
        }
        result = true;
    }

    return result;
}

void kcCLICommanderCL::Analysis(const Config& config)
{
    if (!config.m_AnalysisFile.empty())
    {
        for (const std::string& requiredTarget : m_asicsSorted)
        {
            if (!be->theOpenCLBuilder()->HasCodeObjectBinary(requiredTarget))
            {
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

                for (const std::string& kernelName : m_requiredKernels)
                {
                    // Show the analysis only for external devices.
                    if (m_externalDevices.find(requiredTarget) == m_externalDevices.end())
                    {
                        continue;
                    }

                    AnalysisData analysis;
                    (void)memset(&analysis, 0, sizeof(analysis));
                    beStatus status = be->theOpenCLBuilder()->GetStatistics(requiredTarget, kernelName, analysis);
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

                    // Write the stats file.
                    WriteAnalysisFile(config, kernelName, requiredTarget, analysis);
                }
            }
            else
            {
                // Handle the CodeObject case.
                std::map<std::string, beKA::AnalysisData> statsMap;
                bool isStatsExtracted = be->theOpenCLBuilder()->ExtractStatisticsCodeObject(requiredTarget, statsMap);
                assert(isStatsExtracted);
                if (isStatsExtracted)
                {
                    for (auto iter = statsMap.begin(); iter != statsMap.end(); iter++)
                    {
                        // Write the stats file.
                        WriteAnalysisFile(config, iter->first, requiredTarget, iter->second);
                    }
                }
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
                    if (!kcUtils::IsNaviTarget(deviceName))
                    {
                        for (const std::string& kernelName : m_requiredKernels)
                        {
                            beKA::beStatus status = pBuilder->GetKernelILText(deviceName, kernelName, ilTextBuffer);

                            if (status == beStatus_SUCCESS)
                            {
                                gtString ilOutputFileName;
                                kcUtils::ConstructOutputFileName(config.m_ILFile, "", KC_STR_DEFAULT_AMD_IL_EXT,
                                    kernelName, deviceName, ilOutputFileName);
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
                    else
                    {
                        std::cout << STR_WARNING_IL_NOT_SUPPORTED_FOR_RDNA_A <<
                            deviceName << STR_WARNING_IL_NOT_SUPPORTED_FOR_RDNA_B << std::endl;
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

            // Perform live register analysis - blocked until analysis engine is improved to support
            // OpenCL disassembly with better stability.
            bool isPostProcessingEnabled = true;
            bool isLiveregRequired = !config.m_LiveRegisterAnalysisFile.empty();
            bool isCfgRequired = !config.m_blockCFGFile.empty() || !config.m_instCFGFile.empty();

            std::map<std::string, std::string> deviceToCodeObjectDisassemblyMapping;
            pClBuilder->GetDeviceToCodeObjectDisassemblyMapping(deviceToCodeObjectDisassemblyMapping);

            if (!config.m_ISAFile.empty() || !config.m_ILFile.empty())
            {
                // Get ISA text and make output files.
                for (const std::string& deviceName : m_asics)
                {
                    auto iter = deviceToCodeObjectDisassemblyMapping.find(deviceName);
                    if (iter != deviceToCodeObjectDisassemblyMapping.end())
                    {
                        // Write the ISA disassembly text file.
                        gtString isaOutputFileName;
                        kcUtils::ConstructOutputFileName(config.m_ISAFile, "", KC_STR_DEFAULT_ISA_EXT, "", deviceName, isaOutputFileName);
                        kcUtils::WriteTextFile(isaOutputFileName.asASCIICharArray(), iter->second, m_LogCallback);

                        // Perform post processing.
                        isPostProcessingEnabled = kcUtils::IsPostPorcessingSupported(isaOutputFileName.asASCIICharArray());
                        if (isLiveregRequired)
                        {
                            std::cout << STR_INFO_PERFORMING_LIVEREG_ANALYSIS_A << deviceName << "..." << std::endl;

                            if (isPostProcessingEnabled)
                            {
                                gtString liveRegAnalysisOutputFileName;
                                kcUtils::ConstructOutputFileName(config.m_LiveRegisterAnalysisFile, KC_STR_DEFAULT_LIVEREG_SUFFIX,
                                    KC_STR_DEFAULT_LIVEREG_EXT, "", deviceName, liveRegAnalysisOutputFileName);

                                // Call the kcUtils routine to analyze <generatedFileName> and write the analysis file.
                                kcUtils::PerformLiveRegisterAnalysis(isaOutputFileName, liveRegAnalysisOutputFileName,
                                    m_LogCallback, config.m_printProcessCmdLines);

                                if (kcUtils::FileNotEmpty(liveRegAnalysisOutputFileName.asASCIICharArray()))
                                {
                                    std::cout << STR_INFO_SUCCESS << std::endl;
                                }
                            }
                            else
                            {
                                std::cout << STR_WARNING_LIVEREG_NOT_SUPPORTED_DUE_TO_LLVM_DISASSEMBLY_A <<
                                    STR_WARNING_NOT_SUPPORTED_DUE_TO_LLVM_DISASSEMBLY_B <<
                                    STR_WARNING_SKIPPING << std::endl;
                            }
                        }

                        if (isCfgRequired)
                        {
                            bool isPerBlock = !config.m_blockCFGFile.empty();
                            std::cout << (isPerBlock ? STR_INFO_CONSTRUCTING_BLOCK_CFG_A : STR_INFO_CONSTRUCTING_INSTRUCTION_CFG_A) << deviceName << "..." << std::endl;
                            if (isPostProcessingEnabled)
                            {
                                if (!config.m_blockCFGFile.empty() || !config.m_instCFGFile.empty())
                                {
                                    gtString cfgOutputFileName;
                                    std::string baseName = (!config.m_blockCFGFile.empty() ? config.m_blockCFGFile : config.m_instCFGFile);
                                    kcUtils::ConstructOutputFileName(baseName, KC_STR_DEFAULT_CFG_SUFFIX, KC_STR_DEFAULT_CFG_EXT,
                                        "", deviceName, cfgOutputFileName);

                                    // Call the kcUtils routine to analyze <generatedFileName> and write
                                    // the analysis file.
                                    kcUtils::GenerateControlFlowGraph(isaOutputFileName, cfgOutputFileName, m_LogCallback,
                                        !config.m_instCFGFile.empty(), config.m_printProcessCmdLines);

                                    if (kcUtils::FileNotEmpty(cfgOutputFileName.asASCIICharArray()))
                                    {
                                        std::cout << STR_INFO_SUCCESS << std::endl;
                                    }
                                }
                            }
                            else
                            {
                                std::cout << STR_WARNING_CFG_NOT_SUPPORTED_DUE_TO_LLVM_DISASSEMBLY_A <<
                                    STR_WARNING_NOT_SUPPORTED_DUE_TO_LLVM_DISASSEMBLY_B <<
                                    STR_WARNING_SKIPPING << std::endl;
                            }
                        }
                    }
                    else
                    {
                        bool isIsaFileTemp = config.m_ISAFile.empty();
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
                                    isaFileExt << KC_STR_DEFAULT_ISA_EXT;
                                    isaOutputFileName = kcUtils::ConstructTempFileName(isaFileName, isaFileExt);
                                }
                                else
                                {
                                    kcUtils::ConstructOutputFileName(config.m_ISAFile, "", KC_STR_DEFAULT_ISA_EXT, kernelName, deviceName, isaOutputFileName);
                                }
                                kcUtils::WriteTextFile(isaOutputFileName.asASCIICharArray(), sISAIL, m_LogCallback);

                                // Perform live register analysis.
                                isPostProcessingEnabled = kcUtils::IsPostPorcessingSupported(isaOutputFileName.asASCIICharArray());

                                if (isLiveregRequired)
                                {
                                    std::cout << STR_INFO_PERFORMING_LIVEREG_ANALYSIS_A <<
                                        " kernel " << kernelName << " (" << deviceName << ")..." << std::endl;
                                    if (isPostProcessingEnabled)
                                    {
                                        gtString liveRegAnalysisOutputFileName;
                                        kcUtils::ConstructOutputFileName(config.m_LiveRegisterAnalysisFile, KC_STR_DEFAULT_LIVEREG_SUFFIX,
                                            KC_STR_DEFAULT_LIVEREG_EXT, kernelName, deviceName, liveRegAnalysisOutputFileName);

                                        // Call the kcUtils routine to analyze <generatedFileName> and write the analysis file.
                                        kcUtils::PerformLiveRegisterAnalysis(isaOutputFileName, liveRegAnalysisOutputFileName,
                                            m_LogCallback, config.m_printProcessCmdLines);

                                        if (kcUtils::FileNotEmpty(liveRegAnalysisOutputFileName.asASCIICharArray()))
                                        {
                                            std::cout << STR_INFO_SUCCESS << std::endl;
                                        }
                                    }
                                    else
                                    {
                                        std::cout << STR_WARNING_LIVEREG_NOT_SUPPORTED_DUE_TO_LLVM_DISASSEMBLY_A <<
                                            STR_WARNING_NOT_SUPPORTED_DUE_TO_LLVM_DISASSEMBLY_B <<
                                            STR_WARNING_SKIPPING << std::endl;
                                    }
                                }

                                // Generate control flow graph.
                                if (isCfgRequired)
                                {
                                    if (isPostProcessingEnabled)
                                    {
                                        bool isPerBlock = !config.m_blockCFGFile.empty();
                                        std::cout << (isPerBlock ? STR_INFO_CONSTRUCTING_BLOCK_CFG_A : STR_INFO_CONSTRUCTING_INSTRUCTION_CFG_A) <<
                                            "kernel " << kernelName << " (" << deviceName << ")..." << std::endl;

                                        gtString cfgOutputFileName;
                                        std::string baseName = (!config.m_blockCFGFile.empty() ? config.m_blockCFGFile : config.m_instCFGFile);
                                        kcUtils::ConstructOutputFileName(baseName, KC_STR_DEFAULT_CFG_SUFFIX, KC_STR_DEFAULT_CFG_EXT,
                                            kernelName, deviceName, cfgOutputFileName);

                                        // Call the kcUtils routine to analyze <generatedFileName> and write
                                        // the analysis file.
                                        kcUtils::GenerateControlFlowGraph(isaOutputFileName, cfgOutputFileName, m_LogCallback,
                                            !config.m_instCFGFile.empty(), config.m_printProcessCmdLines);

                                        if (kcUtils::FileNotEmpty(cfgOutputFileName.asASCIICharArray()))
                                        {
                                            std::cout << STR_INFO_SUCCESS << std::endl;
                                        }
                                    }
                                    else
                                    {
                                        std::cout << STR_WARNING_CFG_NOT_SUPPORTED_DUE_TO_LLVM_DISASSEMBLY_A <<
                                            STR_WARNING_NOT_SUPPORTED_DUE_TO_LLVM_DISASSEMBLY_B <<
                                            STR_WARNING_SKIPPING << std::endl;
                                    }
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
                    kcUtils::ConstructOutputFileName(config.m_BinaryOutputFile, "", KC_STR_DEFAULT_BIN_EXT,
                                                     "", deviceName, binOutputFileName);
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
                        kcUtils::ConstructOutputFileName(config.m_MetadataFile, "", KC_STR_DEFAULT_METADATA_EXT,
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

void kcCLICommanderCL::InitRequiredKernels(const Config& config, const std::vector<std::string>& requiredDevices, std::vector<std::string>& requiredKernels)
{
    requiredKernels.clear();

    if (!requiredDevices.empty())
    {
        // We only need a single device name.
        auto firstDevice = requiredDevices.begin();
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

void kcCLICommanderCL::WriteAnalysisFile(const Config& config, const std::string& kernelName,
    const std::string& deviceName, const beKA::AnalysisData& analysis)
{
    // Create the output file.
    ofstream output;
    gtString statsOutputFileName;
    kcUtils::ConstructOutputFileName(config.m_AnalysisFile, KC_STR_DEFAULT_STATS_SUFFIX,
        KC_STR_DEFAULT_STATS_EXT, kernelName, deviceName, statsOutputFileName);
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


    // Get the separator for CSV list items.
    char csvSeparator = kcUtils::GetCsvSeparator(config);

    // Generate the content for the file.
    std::stringstream fileContent;

    // Headers.
    fileContent << kcUtils::GetStatisticsCsvHeaderString(csvSeparator) << std::endl;

    // CSV line.
    fileContent << deviceName << csvSeparator;
    fileContent << analysis.scratchMemoryUsed << csvSeparator;
    fileContent << analysis.numThreadPerGroup << csvSeparator;

    // For Navi targets, the wave size is determined in runtime.
    bool isNaviTarget = kcUtils::IsNaviTarget(deviceName);
    fileContent << (!isNaviTarget ? doNAFormat(analysis.wavefrontSize,
        CAL_NA_Value_64, CAL_ERR_Value_64, csvSeparator) : STR_STATS_NA);
    if (isNaviTarget)
    {
        fileContent << csvSeparator;
    }

    fileContent << analysis.LDSSizeAvailable << csvSeparator;
    fileContent << analysis.LDSSizeUsed << csvSeparator;
    fileContent << doNAFormat(analysis.numSGPRsAvailable, CAL_NA_Value_64, CAL_ERR_Value_64, csvSeparator);
    fileContent << doNAFormat(analysis.numSGPRsUsed, CAL_NA_Value_64, CAL_ERR_Value_64, csvSeparator);
    fileContent << doNAFormat(analysis.numSGPRSpills, CAL_NA_Value_64, CAL_ERR_Value_64, csvSeparator);
    fileContent << doNAFormat(analysis.numVGPRsAvailable, CAL_NA_Value_64, CAL_ERR_Value_64, csvSeparator);
    fileContent << doNAFormat(analysis.numVGPRsUsed, CAL_NA_Value_64, CAL_ERR_Value_64, csvSeparator);
    fileContent << doNAFormat(analysis.numVGPRSpills, CAL_NA_Value_64, CAL_ERR_Value_64, csvSeparator);
    fileContent << doNAFormat(analysis.numThreadPerGroupX, CAL_NA_Value_64, CAL_ERR_Value_64, csvSeparator);
    fileContent << doNAFormat(analysis.numThreadPerGroupY, CAL_NA_Value_64, CAL_ERR_Value_64, csvSeparator);
    fileContent << doNAFormat(analysis.numThreadPerGroupZ, CAL_NA_Value_64, CAL_ERR_Value_64, csvSeparator);
    fileContent << doNAFormat(analysis.ISASize, (CALuint64)0, CAL_ERR_Value_64, csvSeparator, false);
    fileContent << std::endl;

    // Write the file.
    output.open(analysisOutputPath.asString().asASCIICharArray());
    if (!output.is_open())
    {
        std::stringstream s_Log;
        s_Log << "Error: Unable to open " << config.m_AnalysisFile << " for write.\n";
        LogCallBack(s_Log.str());
    }
    else
    {
        // Write the contents.
        output << fileContent.str();
        output.close();
    }
}

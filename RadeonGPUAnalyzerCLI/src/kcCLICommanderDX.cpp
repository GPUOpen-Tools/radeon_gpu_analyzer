//=================================================================
// Copyright 2017 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

#ifdef _WIN32
// Infra.
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtAssert.h>

// Boost.
#include <boost/algorithm/string/predicate.hpp>

// Backend.
#include <RadeonGPUAnalyzerBackend/include/beProgramBuilderDX.h>
#include <RadeonGPUAnalyzerBackend/include/beUtils.h>
#include  <CElf.h>
#include <DeviceInfoUtils.h>

// Local.
#include <RadeonGPUAnalyzerCLI/src/kcCLICommanderDX.h>
#include <RadeonGPUAnalyzerCLI/src/kcCliStringConstants.h>
#include <RadeonGPUAnalyzerCLI/src/kcUtils.h>

// Static constants.
const int  DX_MAX_SUPPORTED_SHADER_MODEL_MAJOR = 5;
const int  DX_MAX_SUPPORTED_SHADER_MODEL_MINOR = 0;

static const std::string  s_STR_NA_VALUE = "N/A";


kcCLICommanderDX::kcCLICommanderDX(void)
{
    m_pBackEndHandler = nullptr;
}


kcCLICommanderDX::~kcCLICommanderDX(void)
{
    // No need to call DeleteInstance. The base class singleton performs this.
}

void kcCLICommanderDX::ListAdapters(Config & config, LoggingCallBackFunc_t callback)
{
    std::vector<std::string> adapterNames;
    stringstream  msg;

    if (beProgramBuilderDX::GetSupportedDisplayAdapterNames(config.m_printProcessCmdLines, adapterNames))
    {
        msg << STR_FOUND_ADAPTERS << std::endl << std::endl;
        for (size_t  i = 0; i < adapterNames.size(); i++)
        {
            msg << " " << i << "\t" << adapterNames[i] << std::endl;
        }
    }
    else
    {
        msg << STR_ERR_LIST_ADAPTERS_FAILED << std::endl;
    }

    msg << std::endl;
    callback(msg.str());
}

void kcCLICommanderDX::InitRequestedAsicListDX(const Config& config)
{
    stringstream s_Log;

    // Get the default device list.
    if (!config.m_ASICs.empty())
    {
        m_dxDefaultAsicsList.clear();
        std::vector<GDT_GfxCardInfo> dxDeviceTable;
        std::set<std::string> supportedDevices;
        std::set<std::string> matchedTargets;

        if (beUtils::GetAllGraphicsCards(dxDeviceTable, supportedDevices))
        {
            if (InitRequestedAsicList(config, supportedDevices, matchedTargets, false))
            {
                for (const std::string& target : matchedTargets)
                {
                    for (const GDT_GfxCardInfo& dxDevice : dxDeviceTable)
                    {
                        if (boost::iequals(dxDevice.m_szCALName, target))
                        {
                            m_dxDefaultAsicsList.push_back(dxDevice);
                            break;
                        }
                    }
                }
            }
        }
    }
}


void kcCLICommanderDX::ExtractISA(const string& deviceName, const Config& config, size_t& isaSizeInBytes,
                                  string isaBuffer, bool& isIsaSizeDetected, bool& shouldDetectIsaSize)
{
    beProgramBuilderDX* pProgramBuilderDX =  m_pBackEndHandler != nullptr ? m_pBackEndHandler->theOpenDXBuilder() : nullptr;
    beStatus backendRet = beStatus_Invalid;
    GT_IF_WITH_ASSERT(pProgramBuilderDX != nullptr)
    {
        backendRet = pProgramBuilderDX->GetDxShaderISAText(deviceName, isaBuffer);
        string fileName = config.m_ISAFile;

        if (backendRet == beStatus_SUCCESS)
        {
            gtString isaOutputFileName;
            if (fileName.empty())
            {
                gtString  tempIsaFileName, isaFileExt;
                tempIsaFileName << (std::string(KC_STR_DEFAULT_ISA_OUTPUT_FILE_NAME) + deviceName + config.m_Function).c_str();
                isaFileExt << KC_STR_DEFAULT_ISA_SUFFIX;
                isaOutputFileName = kcUtils::ConstructTempFileName(tempIsaFileName, isaFileExt);
            }
            else
            {
                kcUtils::ConstructOutputFileName(config.m_ISAFile, KC_STR_DEFAULT_ISA_SUFFIX,
                                                 config.m_Function, deviceName, isaOutputFileName);
            }
            kcUtils::WriteTextFile(isaOutputFileName.asASCIICharArray(), isaBuffer, m_LogCallback);

            // Save parsed ISA to a CSV file if it's requested
            if (config.m_isParsedISARequired)
            {
                std::string  parsedIsa, parsedIsaFileName;
                backendRet = beProgramBuilder::ParseISAToCSV(isaBuffer, deviceName, parsedIsa);

                if (backendRet == beKA::beStatus_SUCCESS)
                {
                    backendRet = kcUtils::GetParsedISAFileName(isaOutputFileName.asASCIICharArray(), parsedIsaFileName) ?
                                     backendRet : beKA::beStatus_LC_ConstructISAFileNameFailed;
                }
                if (backendRet == beKA::beStatus_SUCCESS)
                {
                    kcUtils::WriteTextFile(parsedIsaFileName, parsedIsa, m_LogCallback);
                }
            }

            // Detect the ISA size.
            isIsaSizeDetected = pProgramBuilderDX->GetIsaSize(isaBuffer, isaSizeInBytes);

            // If we managed to detect the ISA size, don't do it again.
            shouldDetectIsaSize = !isIsaSizeDetected;

            if (!config.m_LiveRegisterAnalysisFile.empty())
            {
                gtString liveRegAnalysisOutputFileName;
                kcUtils::ConstructOutputFileName(config.m_LiveRegisterAnalysisFile, KC_STR_DEFAULT_LIVE_REG_ANALYSIS_SUFFIX,
                                                 config.m_Function, deviceName, liveRegAnalysisOutputFileName);

                // Call the kcUtils routine to analyze <generatedFileName> and write
                // the analysis file.
                kcUtils::PerformLiveRegisterAnalysis(isaOutputFileName, liveRegAnalysisOutputFileName,
                                                     m_LogCallback, config.m_printProcessCmdLines);
            }

            if (!config.m_instCFGFile.empty() || !config.m_blockCFGFile.empty())
            {
                gtString cfgOutputFileName;
                std::string baseName = (!config.m_instCFGFile.empty() ? config.m_instCFGFile : config.m_blockCFGFile);
                kcUtils::ConstructOutputFileName(baseName, KC_STR_DEFAULT_CFG_EXT,
                                                 config.m_Function, deviceName, cfgOutputFileName);

                kcUtils::GenerateControlFlowGraph(isaOutputFileName, cfgOutputFileName, m_LogCallback,
                                                  !config.m_instCFGFile.empty(),  config.m_printProcessCmdLines);
            }

            // Delete temporary ISA file.
            if (fileName.empty())
            {
                kcUtils::DeleteFile(isaOutputFileName);
            }
        }

        if (backendRet == beStatus_SUCCESS)
        {
            std::stringstream s_Log;
            s_Log << KA_CLI_STR_EXTRACTING_ISA << deviceName << "... " << KA_CLI_STR_STATUS_SUCCESS << std::endl;
            LogCallBack(s_Log.str());
        }
        else
        {
            std::stringstream s_Log;
            s_Log << KA_CLI_STR_EXTRACTING_ISA << deviceName << "... " << KA_CLI_STR_STATUS_FAILURE << std::endl;
            LogCallBack(s_Log.str());
        }
    }
}

void kcCLICommanderDX::ExtractIL(const std::string& deviceName, const Config& config)
{
    beProgramBuilderDX* pProgramBuilderDX = m_pBackEndHandler != nullptr ? m_pBackEndHandler->theOpenDXBuilder() : nullptr;
    beStatus backendRet = beStatus_Invalid;
    GT_IF_WITH_ASSERT(pProgramBuilderDX != nullptr)
    {
        std::string ilBuffer;
        backendRet = pProgramBuilderDX->GetDxShaderIL(deviceName, ilBuffer);

        if (backendRet == beStatus_SUCCESS)
        {
            gtString ilOutputFileName;
            kcUtils::ConstructOutputFileName(config.m_ILFile, KC_STR_DEFAULT_AMD_IL_SUFFIX,
                config.m_Function, deviceName, ilOutputFileName);
            kcUtils::WriteTextFile(ilOutputFileName.asASCIICharArray(), ilBuffer, m_LogCallback);
        }

        if (backendRet == beStatus_SUCCESS)
        {
            std::stringstream s_Log;
            s_Log << KA_CLI_STR_EXTRACTING_AMDIL << deviceName << "... " << KA_CLI_STR_STATUS_SUCCESS << std::endl;
            LogCallBack(s_Log.str());
        }
        else
        {
            std::stringstream s_Log;
            s_Log << KA_CLI_STR_EXTRACTING_AMDIL << deviceName << "... " << KA_CLI_STR_STATUS_FAILURE << std::endl;
            LogCallBack(s_Log.str());
        }
    }
}

bool kcCLICommanderDX::ExtractStats(const string& deviceName, const Config& config, bool shouldDetectIsaSize, string isaBuffer, bool isIsaSizeDetected,
                                    size_t isaSizeInBytes, vector<AnalysisData>& AnalysisDataVec, vector<string>& DeviceAnalysisDataVec)
{
    AnalysisData analysis;

    beStatus backendRet = m_pBackEndHandler->theOpenDXBuilder()->GetStatistics(deviceName, config.m_Function, analysis);

    if (backendRet == beStatus_SUCCESS)
    {
        if (shouldDetectIsaSize)
        {
            backendRet = m_pBackEndHandler->theOpenDXBuilder()->GetDxShaderISAText(deviceName, isaBuffer);


            if (backendRet == beStatus_SUCCESS)
            {
                // Detect the ISA size.
                isIsaSizeDetected = m_pBackEndHandler->theOpenDXBuilder()->GetIsaSize(isaBuffer, isaSizeInBytes);
            }
        }

        if (isIsaSizeDetected)
        {
            // assign IsaSize returned above
            analysis.ISASize = isaSizeInBytes;
        }
        else
        {
            // assign largest unsigned value, used as warning
            LogCallBack("Warning: ISA size not available.\n");
        }

        // Get WavefrontSize
        size_t nWavefrontSize = 0;

        if (m_pBackEndHandler->theOpenDXBuilder()->GetWavefrontSize(deviceName, nWavefrontSize))
        {
            analysis.wavefrontSize = nWavefrontSize;
        }
        else
        {
            LogCallBack("Warning: wavefrontSize size not available.\n");
        }

        AnalysisDataVec.push_back(analysis);
        DeviceAnalysisDataVec.push_back(deviceName);

        std::stringstream s_Log;
        s_Log << KA_CLI_STR_EXTRACTING_STATISTICS << "... " << KA_CLI_STR_STATUS_SUCCESS << std::endl;
        LogCallBack(s_Log.str());
    }
    else
    {
        std::stringstream s_Log;
        s_Log << KA_CLI_STR_EXTRACTING_STATISTICS << "... " << KA_CLI_STR_STATUS_FAILURE << std::endl;
        LogCallBack(s_Log.str());
    }                   return isIsaSizeDetected;
}


void kcCLICommanderDX::ExtractBinary(const std::string& deviceName, const Config& config)
{

    std::vector<char> binary;
    beKA::BinaryOptions binOptions;
    binOptions.m_SuppressSection = config.m_SuppressSection;

    if (beStatus_SUCCESS == m_pBackEndHandler->theOpenDXBuilder()->GetBinary(deviceName, binOptions, binary))
    {
        gtString binOutputFileName;
        kcUtils::ConstructOutputFileName(config.m_BinaryOutputFile, KC_STR_DEFAULT_BIN_SUFFIX, "", deviceName, binOutputFileName);
        kcUtils::WriteBinaryFile(binOutputFileName.asASCIICharArray(), binary, m_LogCallback);
        std::stringstream s_Log;
        s_Log << KA_CLI_STR_EXTRACTING_BIN << deviceName << "... " << KA_CLI_STR_STATUS_SUCCESS << std::endl;
        LogCallBack(s_Log.str());
    }
    else
    {
        // Inform the user.
        std::stringstream msg;
        msg << STR_ERR_CANNOT_EXTRACT_BINARIES << " for " << deviceName << "." << std::endl;
        m_LogCallback(msg.str().c_str());
    }
}

/// Output the ISA representative of the compilation
void kcCLICommanderDX::RunCompileCommands(const Config& config, LoggingCallBackFuncP callback)
{
    bool isInitSuccessful = Init(config, callback);

    if (isInitSuccessful)
    {
        const bool bISA = !config.m_ISAFile.empty();
        const bool bIL = !config.m_ILFile.empty();
        const bool bStatistics = !config.m_AnalysisFile.empty();
        const bool bBinaryOutput = !config.m_BinaryOutputFile.empty();
        const bool bLivereg = !config.m_LiveRegisterAnalysisFile.empty();
        const bool bCfg = (!config.m_instCFGFile.empty() || !config.m_blockCFGFile.empty());

        vector <AnalysisData> AnalysisDataVec;
        vector <string> DeviceAnalysisDataVec;

        // check for input correctness
        if ((config.m_FXC.length() > 0) && (config.m_SourceLanguage != SourceLanguage_DXasm))
        {
            std::stringstream s_Log;
            s_Log << "DXAsm must be specified when using FXC";
            LogCallBack(s_Log.str());
            return;
        }

        // check flags first
        if ((config.m_Profile.length() == 0) && (config.m_SourceLanguage == SourceLanguage_HLSL))
        {
            std::stringstream s_Log;
            s_Log << "-P Must be specified. Check compiler target: vs_5_0, ps_5_0 etc.";
            LogCallBack(s_Log.str());
            return;
        }

        if ((config.m_SourceLanguage != SourceLanguage_HLSL)    &&
            (config.m_SourceLanguage != SourceLanguage_DXasm)   &&
            (config.m_SourceLanguage != SourceLanguage_DXasmT)  &&
            (config.m_SourceLanguage != SourceLanguage_AMDIL))
        {
            std::stringstream s_Log;
            s_Log << "Source language is not supported. Please use ";
            LogCallBack(s_Log.str());
            return;
        }

        // Run FXC if required. It must be first because this is the input for the compilation. we cannot check for success.
        if (config.m_FXC.length() > 0)
        {
            std::string fixedCmd("\"");
            fixedCmd += config.m_FXC;
            fixedCmd += "\"";
            int iRet = ::system(fixedCmd.c_str());

            if (iRet != 0)
            {
                std::stringstream s_Log;
                s_Log << "FXC failed. Please check the arguments and path are correct. If path contains spaces, you need to put it in \\\"\\\" for example\n";
                s_Log << "-f  VsMain1 -s DXAsm -p vs_5_0 c:\\temp\\ClippingBlob.obj  --isa c:\\temp\\dxTest.isa -c tahiti --FXC \"\\\"C:\\Program Files (x86)\\Windows Kits\\8.1\\bin\\x86\\fxc.exe\\\" /E VsMain1 /T vs_5_0  /Fo c:/temp/ClippingBlob.obj c:/temp/Clipping.fx\" ";
                LogCallBack(s_Log.str());
                return;
            }
        }

        // see if the user asked for specific asics
        InitRequestedAsicListDX(config);

        // We need to iterate over the selected devices
        bool bCompileSucces = false;

        for (const GDT_GfxCardInfo& devceInfo : m_dxDefaultAsicsList)
        {
            // Get the device name.
            std::string deviceName = devceInfo.m_szCALName;

            if (Compile(config, devceInfo, deviceName))
            {
                if (bBinaryOutput)
                {
                    ExtractBinary(deviceName, config);
                }

                std::string isaBuffer;
                bCompileSucces = true;
                bool isIsaSizeDetected = false;
                bool shouldDetectIsaSize = true;
                size_t isaSizeInBytes(0);

                if (bISA || bStatistics || bLivereg || bCfg)
                {
                    ExtractISA(deviceName, config, isaSizeInBytes, isaBuffer, isIsaSizeDetected, shouldDetectIsaSize);
                }
                if (bIL)
                {
                    ExtractIL(deviceName, config);
                }
                if (bStatistics)
                {
                    isIsaSizeDetected = ExtractStats(deviceName, config, shouldDetectIsaSize, isaBuffer, isIsaSizeDetected,
                                                     isaSizeInBytes, AnalysisDataVec, DeviceAnalysisDataVec);
                }
            }

            std::stringstream s_Log;
            LogCallBack(s_Log.str());
        }

        if ((AnalysisDataVec.size() > 0) && bCompileSucces)
        {
            gtString analysisFileName;
            kcUtils::ConstructOutputFileName(config.m_AnalysisFile, KC_STR_DEFAULT_STATISTICS_SUFFIX, config.m_Function, "", analysisFileName);

            std::stringstream s_Log;
            WriteAnalysisDataForDX(config, AnalysisDataVec, DeviceAnalysisDataVec, analysisFileName.asASCIICharArray(), s_Log);
            LogCallBack(s_Log.str());
        }

        // We should do this only once because it is the same to all devices.
        if ((config.m_DumpMSIntermediate.size() > 0) && bCompileSucces)
        {
            string sDumpMSIntermediate;
            beStatus beRet = m_pBackEndHandler->theOpenDXBuilder()->GetIntermediateMSBlob(sDumpMSIntermediate);

            if (beRet == beStatus_SUCCESS)
            {
                std::stringstream ss;
                ss << KA_CLI_STR_D3D_ASM_GENERATION_SUCCESS << std::endl;
                LogCallBack(ss.str());

                gtString dxAsmOutputFileName;
                kcUtils::ConstructOutputFileName(config.m_DumpMSIntermediate, KC_STR_DEFAULT_DXASM_SUFFIX, config.m_Function, "", dxAsmOutputFileName);
                kcUtils::WriteTextFile(dxAsmOutputFileName.asASCIICharArray(), sDumpMSIntermediate, m_LogCallback);
            }
            else
            {
                std::stringstream ss;
                ss << KA_CLI_STR_D3D_ASM_GENERATION_FAILURE << std::endl;
                LogCallBack(ss.str());
            }
        }
    }
}

bool kcCLICommanderDX::WriteAnalysisDataForDX(const Config& config, const std::vector<AnalysisData>& AnalysisDataVec,
                                              const std::vector<string>& DeviceAnalysisDataVec, const std::string& sAnalysisFile, std::stringstream& log)
{
    // Get the separator for CSV list items.
    char csvSeparator = kcUtils::GetCsvSeparator(config);

    // Open output file.
    ofstream output;
    output.open(sAnalysisFile);

    if (! output.is_open())
    {
        log << "Error: Unable to open " << sAnalysisFile << " for write.\n";
    }
    else
    {
        // Write the headers.
        output << kcUtils::GetStatisticsCsvHeaderString(csvSeparator) << std::endl;

        // Write the statistics data.
        if (!AnalysisDataVec.empty())
        {
            std::vector<std::string>::const_iterator iter = DeviceAnalysisDataVec.begin();
            std::vector<AnalysisData>::const_iterator iterAd = AnalysisDataVec.begin();

            for (; iter < DeviceAnalysisDataVec.end(); ++iter, ++iterAd)
            {
                // Device name.
                output << *iter << csvSeparator;

                // Get the Analysis item.
                const AnalysisData& ad = *iterAd;

                // Scratch registers.
                output << ad.scratchMemoryUsed << csvSeparator;

                // Work-items per work-group.
                output << ad.numThreadPerGroup << csvSeparator;

                // Wavefront size.
                output << ad.wavefrontSize << csvSeparator;

                // LDS available bytes.
                output << ad.LDSSizeAvailable << csvSeparator;

                // LDS actual bytes.
                output << ad.LDSSizeUsed << csvSeparator;

                // Available SGPRs.
                output << ad.numSGPRsAvailable << csvSeparator;

                // Used SGPRs.
                output << ad.numSGPRsUsed << csvSeparator;

                // SGPR spills.
                output << s_STR_NA_VALUE << csvSeparator;

                // Available VGPRs.
                output << ad.numVGPRsAvailable << csvSeparator;

                // Used VGPRs.
                output << ad.numVGPRsUsed << csvSeparator;

                // VGPR spills.
                output << s_STR_NA_VALUE << csvSeparator;

                // CL Work-group dimensions (for a unified format, to be revisited).
                output << ad.numThreadPerGroupX << csvSeparator;
                output << ad.numThreadPerGroupY << csvSeparator;
                output << ad.numThreadPerGroupZ << csvSeparator;

                // ISA size.
                output << ad.ISASize;

                output << endl;
            }
        }

        output.close();
    }

    return true;
}

bool ParseProfileString(const std::string& profile, std::pair<int, int>& version)
{
    bool  result = false;
    // profile string format: XX_N_N
    if (!profile.empty())
    {
        size_t  minor, major = profile.find('_');
        if (major != std::string::npos)
        {
            if ((minor = profile.find('_', ++major)) != std::string::npos)
            {
                try
                {
                    version = { std::stoi(profile.substr(major, minor)), std::stoi(profile.substr(++minor)) };
                    result = true;
                }
                catch (...) {}
            }
        }
    }
    return result;
}

/// Output the ISA representative of the compilation
bool kcCLICommanderDX::Compile(const Config& config, const GDT_GfxCardInfo& gfxCardInfo, string sDevicenametoLog)
{
    bool ret = true;
    std::stringstream    s_Log;
    std::pair<int, int>  version;
    beProgramBuilderDX::DXOptions dxOptions;

    if (config.m_SourceLanguage != SourceLanguage_AMDIL &&
        !ParseProfileString(config.m_Profile, version))
    {
        s_Log << STR_ERR_PARSE_DX_SHADER_MODEL_FAILED << std::endl;
        ret = false;
    }

    // Check the provided shader profile version.
    if (ret)
    {
        if (version.first > DX_MAX_SUPPORTED_SHADER_MODEL_MAJOR ||
            (version.first == DX_MAX_SUPPORTED_SHADER_MODEL_MAJOR && version.second > DX_MAX_SUPPORTED_SHADER_MODEL_MINOR))
        {
            s_Log << STR_ERR_UNSUPPORTED_DX_SHADER_MODEL_1 << config.m_Profile << ". "
                << STR_ERR_UNSUPPORTED_DX_SHADER_MODEL_2 << DX_MAX_SUPPORTED_SHADER_MODEL_MAJOR << "." << DX_MAX_SUPPORTED_SHADER_MODEL_MINOR << " "
                << STR_ERR_UNSUPPORTED_DX_SHADER_MODEL_3 << std::endl;
            ret = false;
        }

        if ((config.m_Profile.find("_3_") != string::npos) || (config.m_Profile.find("_2_") != string::npos) || (config.m_Profile.find("_1_") != string::npos)
            || (config.m_Profile.find("level_9_") != string::npos))
        {
            s_Log << STR_WRN_DX_MIN_SUPPORTED_VERSION << std::endl;
        }
    }

    if (ret)
    {
        //// prepare the options
        dxOptions.m_Entrypoint = config.m_Function;
        dxOptions.m_Target = config.m_Profile;
        dxOptions.m_DXFlags.flagsAsInt = config.m_DXFlags;
        dxOptions.m_bDumpMSIntermediate = (config.m_DumpMSIntermediate.size() > 0 ? true : false);
        dxOptions.m_isShaderIntrinsicsEnabled = config.m_EnableShaderIntrinsics;
        dxOptions.m_UAVSlot = config.m_UAVSlot;

        // Process config.m_Defines
        // The interface for DX is different here.
        // It is set up for the GUI.
        // The CLI does some work here to translate.
        for (vector<string>::const_iterator it = config.m_Defines.begin();
            it != config.m_Defines.end();
            ++it)
        {
            size_t equal_pos = it->find('=');

            if (equal_pos == string::npos)
            {
                dxOptions.m_defines.push_back(make_pair(*it, string("")));
            }
            else
            {
                dxOptions.m_defines.push_back(make_pair(it->substr(0, equal_pos),
                    it->substr(equal_pos + 1, string::npos)));
            }
        }
    }


    // read the source
    string sSource;

    if (ret)
    {
        if (!config.m_InputFiles.empty())
        {
            if (config.m_InputFiles.size() > 1)
            {
                s_Log << STR_ERR_ONE_INPUT_FILE_EXPECTED << std::endl;
                ret = false;
            }
            else
            {
                ret = kcUtils::ReadProgramSource(config.m_InputFiles[0], sSource);
                if (!ret)
                {
                    s_Log << STR_ERR_CANNOT_READ_FILE << config.m_InputFiles[0] << std::endl;
                }
            }
        }
        else
        {
            s_Log << STR_ERR_NO_INPUT_FILE << std::endl;
            ret = false;
        }
    }

    if (ret)
    {
        // dx interface like the chip revision and family
        beStatus beRet = m_pBackEndHandler->GetDeviceChipFamilyRevision(gfxCardInfo, dxOptions.m_ChipFamily, dxOptions.m_ChipRevision);

        if (beRet != beStatus_SUCCESS)
        {
            // the use must have got the asics spelled wrong- let him know and continue
            s_Log << "Error: Couldn't find device named: " << sDevicenametoLog << ". Run \'-s HLSL -l to view available devices." << endl;
            ret = false;
        }
        else
        {
            // Set the source code file name.
            dxOptions.m_FileName = config.m_InputFiles[0];

            // Set the device file name.
            dxOptions.m_deviceName = sDevicenametoLog;

            // Fill the include files path.
            for (const std::string& includePath : config.m_IncludePath)
            {
                dxOptions.m_includeDirectories.push_back(includePath);
            }

            beRet = m_pBackEndHandler->theOpenDXBuilder()->Compile(config.m_SourceLanguage, sSource, dxOptions);

            if (beRet == beStatus_Create_Bolob_FromInput_Failed)
            {
                s_Log << "Error reading DX ASM file. ";
                ret = false;
            }

            if (beRet != beStatus_SUCCESS)
            {
                s_Log << KA_CLI_STR_COMPILING << sDevicenametoLog << "... " << KA_CLI_STR_STATUS_FAILURE << std::endl;
                ret = false;
            }
            else
            {
                s_Log << KA_CLI_STR_COMPILING << sDevicenametoLog << "... " << KA_CLI_STR_STATUS_SUCCESS << std::endl;
                ret = true;
            }
        }
    }

    LogCallBack(s_Log.str());
    return ret;
}

bool kcCLICommanderDX::Init(const Config& config, LoggingCallBackFuncP callback)
{
    bool bCont = true;

    m_LogCallback = callback;

    // initialize backend
    m_pBackEndHandler = Backend::Instance();

    if (!m_pBackEndHandler->Initialize(BuiltProgramKind_DX, callback))
    {
        bCont = false;
    }

    beProgramBuilderDX& dxBuilder = *(m_pBackEndHandler->theOpenDXBuilder());

    if (bCont)
    {
        // Initialize the DX Builder
        dxBuilder.SetLog(callback);

        // If a particular GPU adapter is requested, choose corresponding driver.
        std::string  adapterName = "", dxxModulePath;
        if (config.m_DXAdapter != -1)
        {
            bCont = beProgramBuilderDX::GetDXXModulePathForAdapter(config.m_DXAdapter, config.m_printProcessCmdLines, adapterName, dxxModulePath);
        }

        if (bCont)
        {
            bCont = (dxBuilder.Initialize(dxxModulePath, config.m_DXLocation) == beKA::beStatus_SUCCESS);
        }
        else
        {
            std::stringstream  errMsg;
            errMsg << STR_ERR_SET_ADAPTER_FAILED << std::endl;
            callback(errMsg.str());
        }
    }

    if (bCont)// init ASICs list
    {
        std::vector<GDT_GfxCardInfo> dxDeviceTable;
        beStatus bRet = dxBuilder.GetDeviceTable(dxDeviceTable);

        // Returns true if the device is disabled for all modes.
        auto isDisabled = [](const std::string& device)
                          { for (auto& d : kcUtils::GetRgaDisabledDevices()) { if (d.find(device) != std::string::npos) return true; } return false; };

        if (bRet == beStatus_SUCCESS)
        {
            string calName;

            for (vector<GDT_GfxCardInfo>::const_iterator it = dxDeviceTable.begin(); it != dxDeviceTable.end(); ++it)
            {
                if (calName != string(it->m_szCALName) && !isDisabled(it->m_szCALName))
                {
                    calName = it->m_szCALName;
                    m_dxDefaultAsicsList.push_back(*it);
                }
            }
        }
        else
        {
            bCont = false;
        }
    }

    return bCont;
}

#endif

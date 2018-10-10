//=================================================================
// Copyright 2017 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

// C++.
#include <iterator>

// Backend.
#include <DeviceInfo.h>
#include <RadeonGPUAnalyzerBackend/include/beBackend.h>

// Infra.
#include <AMDTBaseTools/Include/gtAssert.h>

// Local.
#include <RadeonGPUAnalyzerCLI/src/kcCLICommanderOpenGL.h>
#include <RadeonGPUAnalyzerCLI/src/kcCliStringConstants.h>
#include <RadeonGPUAnalyzerCLI/src/kcOpenGLStatisticsParser.h>
#include <RadeonGPUAnalyzerCLI/src/kcUtils.h>

struct kcCLICommanderOpenGL::OpenGLDeviceInfo
{
    OpenGLDeviceInfo() : m_deviceFamilyId(0), m_deviceId(0) {}

    OpenGLDeviceInfo(size_t chipFamily, size_t chipRevision) :
        m_deviceFamilyId(chipFamily), m_deviceId(chipRevision) {}

    static bool HwGenToFamilyId(GDT_HW_GENERATION hwGen, size_t& familyId)
    {
        bool ret = true;
        familyId = 0;

        switch (hwGen)
        {
            case GDT_HW_GENERATION_SOUTHERNISLAND:
                familyId = 110;
                break;

            case GDT_HW_GENERATION_SEAISLAND:
                familyId = 120;
                break;

            case GDT_HW_GENERATION_VOLCANICISLAND:
                familyId = 130;
                break;

            case GDT_HW_GENERATION_NONE:
            case GDT_HW_GENERATION_NVIDIA:
            case GDT_HW_GENERATION_LAST:
            default:
                ret = false;
                GT_ASSERT_EX(false, L"Unsupported HW generation.");
                break;
        }

        return ret;
    }

    // HW family id.
    size_t m_deviceFamilyId;

    // Chip id
    size_t m_deviceId;
};


kcCLICommanderOpenGL::kcCLICommanderOpenGL() : m_pOglBuilder(new beProgramBuilderOpenGL)
{
}


kcCLICommanderOpenGL::~kcCLICommanderOpenGL()
{
    delete m_pOglBuilder;
}

void kcCLICommanderOpenGL::Version(Config& config, LoggingCallBackFunc_t callback)
{
    GT_UNREFERENCED_PARAMETER(config);

    kcCLICommander::Version(config, callback);

    if (m_pOglBuilder != nullptr)
    {
        gtString glVersion;
        bool rc = m_pOglBuilder->GetOpenGLVersion(config.m_printProcessCmdLines, glVersion);

        callback(((rc && !glVersion.isEmpty()) ? std::string(glVersion.asASCIICharArray()) : STR_ERR_CANNOT_EXTRACT_OPENGL_VERSION) + "\n");
    }
}

bool kcCLICommanderOpenGL::PrintAsicList(std::ostream & log)
{
    return kcUtils::PrintAsicList(log, beProgramBuilderOpenGL::GetDisabledDevices());
}

// Helper function to remove unnecessary file paths.
static bool GenerateRenderingPipelineOutputPaths(const Config& config, const std::string& baseOutputFileName, const std::string& defaultExt,
                                                 const std::string& device, beProgramPipeline& pipelineToAdjust)
{
    // Generate the output file paths.
    bool  ret = kcUtils::AdjustRenderingPipelineOutputFileNames(baseOutputFileName, defaultExt, device, pipelineToAdjust);

    if (ret)
    {
        // Clear irrelevant paths.
        bool isVertexShaderPresent = (!config.m_VertexShader.empty());
        bool isTessControlShaderPresent = (!config.m_TessControlShader.empty());
        bool isTessEvaluationShaderPresent = (!config.m_TessEvaluationShader.empty());
        bool isGeometryexShaderPresent = (!config.m_GeometryShader.empty());
        bool isFragmentShaderPresent = (!config.m_FragmentShader.empty());
        bool isComputeShaderPresent = (!config.m_ComputeShader.empty());

        if (!isVertexShaderPresent)
        {
            pipelineToAdjust.m_vertexShader.makeEmpty();
        }

        if (!isTessControlShaderPresent)
        {
            pipelineToAdjust.m_tessControlShader.makeEmpty();
        }

        if (!isTessEvaluationShaderPresent)
        {
            pipelineToAdjust.m_tessEvaluationShader.makeEmpty();
        }

        if (!isGeometryexShaderPresent)
        {
            pipelineToAdjust.m_geometryShader.makeEmpty();
        }

        if (!isFragmentShaderPresent)
        {
            pipelineToAdjust.m_fragmentShader.makeEmpty();
        }

        if (!isComputeShaderPresent)
        {
            pipelineToAdjust.m_computeShader.makeEmpty();
        }
    }

    return ret;
}

void kcCLICommanderOpenGL::RunCompileCommands(const Config& config, LoggingCallBackFunc_t callback)
{
    // Output stream.
    std::stringstream logMsg;

    // Input validation.
    bool shouldAbort = false;
    bool status = true;
    bool isVertexShaderPresent = (!config.m_VertexShader.empty());
    bool isTessControlShaderPresent = (!config.m_TessControlShader.empty());
    bool isTessEvaluationShaderPresent = (!config.m_TessEvaluationShader.empty());
    bool isGeometryexShaderPresent = (!config.m_GeometryShader.empty());
    bool isFragmentShaderPresent = (!config.m_FragmentShader.empty());
    bool isComputeShaderPresent = (!config.m_ComputeShader.empty());
    bool isIsaRequired = (!config.m_ISAFile.empty() || !config.m_LiveRegisterAnalysisFile.empty() ||
                          !config.m_blockCFGFile.empty() || !config.m_instCFGFile.empty() || !config.m_AnalysisFile.empty());
    bool isLiveRegAnalysisRequired = (!config.m_LiveRegisterAnalysisFile.empty());
    bool isBlockCfgRequired = (!config.m_blockCFGFile.empty());
    bool isInstCfgRequired = (!config.m_instCFGFile.empty());
    bool isIsaBinary = (!config.m_BinaryOutputFile.empty());
    bool isStatisticsRequired = (!config.m_AnalysisFile.empty());

    // Fatal error. This should not happen unless we have an allocation problem.
    if (m_pOglBuilder == nullptr)
    {
        shouldAbort = true;
        logMsg << STR_ERR_MEMORY_ALLOC_FAILURE << std::endl;
    }

    // Cannot mix compute and non-compute shaders.
    if (isComputeShaderPresent && (isVertexShaderPresent || isTessControlShaderPresent ||
                                   isTessEvaluationShaderPresent || isGeometryexShaderPresent || isFragmentShaderPresent))
    {
        logMsg << STR_ERR_RENDER_COMPUTE_MIX << std::endl;
        shouldAbort = true;
    }

    // Options to be passed to the backend.
    OpenGLOptions glOptions;

    // Validate the input shaders.
    if (!shouldAbort && isVertexShaderPresent)
    {
        shouldAbort = !kcUtils::ValidateShaderFileName(KA_CLI_STR_VERTEX_SHADER, config.m_VertexShader, logMsg);
        glOptions.m_pipelineShaders.m_vertexShader << config.m_VertexShader.c_str();
    }

    if (!shouldAbort && isTessControlShaderPresent)
    {
        shouldAbort = !kcUtils::ValidateShaderFileName(KA_CLI_STR_TESS_CTRL_SHADER, config.m_TessControlShader, logMsg);
        glOptions.m_pipelineShaders.m_tessControlShader << config.m_TessControlShader.c_str();
    }

    if (!shouldAbort && isTessEvaluationShaderPresent)
    {
        shouldAbort = !kcUtils::ValidateShaderFileName(KA_CLI_STR_TESS_EVAL_SHADER, config.m_TessEvaluationShader, logMsg);
        glOptions.m_pipelineShaders.m_tessEvaluationShader << config.m_TessEvaluationShader.c_str();
    }

    if (!shouldAbort && isGeometryexShaderPresent)
    {
        shouldAbort = !kcUtils::ValidateShaderFileName(KA_CLI_STR_GEOMETRY_SHADER, config.m_GeometryShader, logMsg);
        glOptions.m_pipelineShaders.m_geometryShader << config.m_GeometryShader.c_str();
    }

    if (!shouldAbort && isFragmentShaderPresent)
    {
        shouldAbort = !kcUtils::ValidateShaderFileName(KA_CLI_STR_FRAGMENT_SHADER, config.m_FragmentShader, logMsg);
        glOptions.m_pipelineShaders.m_fragmentShader << config.m_FragmentShader.c_str();
    }

    if (!shouldAbort && isComputeShaderPresent)
    {
        shouldAbort = !kcUtils::ValidateShaderFileName(KA_CLI_STR_COMPUTE_SHADER, config.m_ComputeShader, logMsg);
        glOptions.m_pipelineShaders.m_computeShader << config.m_ComputeShader.c_str();
    }

    // Validate the output directories.
    if (!shouldAbort && isIsaRequired && !config.m_ISAFile.empty())
    {
        shouldAbort = !kcUtils::ValidateShaderOutputDir(config.m_ISAFile, logMsg);
    }

    if (!shouldAbort && isLiveRegAnalysisRequired)
    {
        shouldAbort = !kcUtils::ValidateShaderOutputDir(config.m_LiveRegisterAnalysisFile, logMsg);
    }

    if (!shouldAbort && isBlockCfgRequired)
    {
        shouldAbort = !kcUtils::ValidateShaderOutputDir(config.m_blockCFGFile, logMsg);
    }

    if (!shouldAbort && isInstCfgRequired)
    {
        shouldAbort = !kcUtils::ValidateShaderOutputDir(config.m_instCFGFile, logMsg);
    }

    if (!shouldAbort && isStatisticsRequired)
    {
        shouldAbort = !kcUtils::ValidateShaderOutputDir(config.m_AnalysisFile, logMsg);
    }

    if (!shouldAbort && isIsaBinary)
    {
        shouldAbort = !kcUtils::ValidateShaderOutputDir(config.m_BinaryOutputFile, logMsg);
    }

    if (!shouldAbort)
    {
        // Set the log callback for the backend.
        m_LogCallback = callback;
        m_pOglBuilder->SetLog(callback);

        std::set<std::string> targetDevices;

        if (m_supportedDevicesCache.empty())
        {
            // We need to populate the list of supported devices.
            bool isDeviceListExtracted = beProgramBuilderOpenGL::GetSupportedDevices(m_supportedDevicesCache);

            if (!isDeviceListExtracted)
            {
                std::stringstream errMsg;
                errMsg << STR_ERR_CANNOT_EXTRACT_SUPPORTED_DEVICE_LIST << std::endl;
                shouldAbort = true;
            }
        }

        if (!shouldAbort)
        {
            InitRequestedAsicList(config, m_supportedDevicesCache, targetDevices, false);

            for (const std::string& device : targetDevices)
            {
                if (!shouldAbort)
                {
                    // Generate the output message.
                    logMsg << KA_CLI_STR_COMPILING << device << "... ";

                    // Set the target device info for the backend.
                    bool isDeviceGlInfoExtracted = m_pOglBuilder->GetDeviceGLInfo(device, glOptions.m_chipFamily, glOptions.m_chipRevision);

                    if (!isDeviceGlInfoExtracted)
                    {
                        logMsg << STR_ERR_CANNOT_GET_DEVICE_INFO << device << std::endl;
                        continue;
                    }

                    // Adjust the output file names to the device and shader type.
                    if (isIsaRequired)
                    {
                        glOptions.m_isAmdIsaDisassemblyRequired = true;
                        status &= GenerateRenderingPipelineOutputPaths(config, config.m_ISAFile, KC_STR_DEFAULT_ISA_SUFFIX, device, glOptions.m_isaDisassemblyOutputFiles);
                    }

                    if (isLiveRegAnalysisRequired)
                    {
                        glOptions.m_isLiveRegisterAnalysisRequired = true;
                        status &= GenerateRenderingPipelineOutputPaths(config, config.m_LiveRegisterAnalysisFile, KC_STR_DEFAULT_LIVE_REG_ANALYSIS_SUFFIX,
                                                                       device, glOptions.m_liveRegisterAnalysisOutputFiles);
                    }

                    if (isBlockCfgRequired)
                    {
                        glOptions.m_isLiveRegisterAnalysisRequired = true;
                        status &= GenerateRenderingPipelineOutputPaths(config, config.m_blockCFGFile,
                                                                       KC_STR_DEFAULT_CFG_EXT, device, glOptions.m_controlFlowGraphOutputFiles);
                    }

                    if (isInstCfgRequired)
                    {
                        glOptions.m_isLiveRegisterAnalysisRequired = true;
                        status &= GenerateRenderingPipelineOutputPaths(config, config.m_instCFGFile,
                                                                       KC_STR_DEFAULT_CFG_EXT, device, glOptions.m_controlFlowGraphOutputFiles);
                    }

                    if (isStatisticsRequired)
                    {
                        glOptions.m_isScStatsRequired = true;
                        status &= GenerateRenderingPipelineOutputPaths(config, config.m_AnalysisFile, KC_STR_DEFAULT_STATISTICS_SUFFIX,
                                                                       device, glOptions.m_scStatisticsOutputFiles);
                    }

                    if (isIsaBinary)
                    {
                        glOptions.m_isAmdIsaBinariesRequired = true;
                        kcUtils::ConstructOutputFileName(config.m_BinaryOutputFile, KC_STR_DEFAULT_BIN_SUFFIX, "", device, glOptions.m_programBinaryFile);
                    }
                    else
                    {
                        // If binary file name is not provided, create a temp file.
                        glOptions.m_programBinaryFile = kcUtils::ConstructTempFileName(L"rgaTempFile", L"bin");
                        GT_ASSERT_EX(glOptions.m_programBinaryFile != L"", L"Cannot create a temp file.");
                    }

                    if (!status)
                    {
                        logMsg << STR_ERR_FAILED_ADJUST_FILE_NAMES << std::endl;
                        shouldAbort = true;
                        break;
                    }

                    // A handle for canceling the build. Currently not in use.
                    bool shouldCancel = false;

                    // Compile.
                    gtString vcOutput;
                    beKA::beStatus buildStatus = m_pOglBuilder->Compile(glOptions, shouldCancel, config.m_printProcessCmdLines, vcOutput);

                    if (buildStatus == beStatus_SUCCESS)
                    {
                        logMsg << KA_CLI_STR_STATUS_SUCCESS << std::endl;

                        // Parse and replace the statistics files.
                        beKA::AnalysisData statistics;
                        kcOpenGLStatisticsParser statsParser;

                        // Parse ISA and write it to a csv file if required.
                        if (isIsaRequired && config.m_isParsedISARequired)
                        {
                            bool  status;
                            std::string  isaText, parsedIsaText, parsedIsaFileName;
                            beProgramPipeline  isaFiles = glOptions.m_isaDisassemblyOutputFiles;
                            for (const gtString& isaFileName : { isaFiles.m_computeShader, isaFiles.m_fragmentShader, isaFiles.m_geometryShader,
                                                                isaFiles.m_tessControlShader, isaFiles.m_tessEvaluationShader, isaFiles.m_vertexShader })
                            {
                                if (!isaFileName.isEmpty())
                                {
                                    if ((status = kcUtils::ReadTextFile(isaFileName.asASCIICharArray(), isaText, m_LogCallback)) == true)
                                    {
                                        status = (beProgramBuilder::ParseISAToCSV(isaText, device, parsedIsaText) == beStatus::beStatus_SUCCESS);
                                        if (status)
                                        {
                                            status = kcUtils::GetParsedISAFileName(isaFileName.asASCIICharArray(), parsedIsaFileName);
                                        }
                                        if (status)
                                        {
                                            status = kcUtils::WriteTextFile(parsedIsaFileName, parsedIsaText, m_LogCallback);
                                        }
                                    }
                                    if (!status)
                                    {
                                        logMsg << STR_ERR_FAILED_PARSE_ISA << std::endl;
                                    }
                                }
                            }
                        }

                        if (isStatisticsRequired)
                        {
                            if (isVertexShaderPresent)
                            {
                                kcUtils::ReplaceStatisticsFile(glOptions.m_scStatisticsOutputFiles.m_vertexShader, config, device, statsParser, callback);
                            }

                            if (isTessControlShaderPresent)
                            {
                                kcUtils::ReplaceStatisticsFile(glOptions.m_scStatisticsOutputFiles.m_tessControlShader, config, device, statsParser, callback);
                            }

                            if (isTessEvaluationShaderPresent)
                            {
                                kcUtils::ReplaceStatisticsFile(glOptions.m_scStatisticsOutputFiles.m_tessEvaluationShader, config, device, statsParser, callback);
                            }

                            if (isGeometryexShaderPresent)
                            {
                                kcUtils::ReplaceStatisticsFile(glOptions.m_scStatisticsOutputFiles.m_geometryShader, config, device, statsParser, callback);
                            }

                            if (isFragmentShaderPresent)
                            {
                                kcUtils::ReplaceStatisticsFile(glOptions.m_scStatisticsOutputFiles.m_fragmentShader, config, device, statsParser, callback);
                            }

                            if (isComputeShaderPresent)
                            {
                                kcUtils::ReplaceStatisticsFile(glOptions.m_scStatisticsOutputFiles.m_computeShader, config, device, statsParser, callback);
                            }
                        }

                        // Perform live register analysis if required.
                        if (isLiveRegAnalysisRequired)
                        {
                            if (isVertexShaderPresent)
                            {
                                kcUtils::PerformLiveRegisterAnalysis(glOptions.m_isaDisassemblyOutputFiles.m_vertexShader,
                                                                     glOptions.m_liveRegisterAnalysisOutputFiles.m_vertexShader, callback, config.m_printProcessCmdLines);
                            }

                            if (isTessControlShaderPresent)
                            {
                                kcUtils::PerformLiveRegisterAnalysis(glOptions.m_isaDisassemblyOutputFiles.m_tessControlShader,
                                                                     glOptions.m_liveRegisterAnalysisOutputFiles.m_tessControlShader, callback, config.m_printProcessCmdLines);
                            }

                            if (isTessEvaluationShaderPresent)
                            {
                                kcUtils::PerformLiveRegisterAnalysis(glOptions.m_isaDisassemblyOutputFiles.m_tessEvaluationShader,
                                                                     glOptions.m_liveRegisterAnalysisOutputFiles.m_tessEvaluationShader, callback, config.m_printProcessCmdLines);
                            }

                            if (isGeometryexShaderPresent)
                            {
                                kcUtils::PerformLiveRegisterAnalysis(glOptions.m_isaDisassemblyOutputFiles.m_geometryShader,
                                                                     glOptions.m_liveRegisterAnalysisOutputFiles.m_geometryShader, callback, config.m_printProcessCmdLines);
                            }

                            if (isFragmentShaderPresent)
                            {
                                kcUtils::PerformLiveRegisterAnalysis(glOptions.m_isaDisassemblyOutputFiles.m_fragmentShader,
                                                                     glOptions.m_liveRegisterAnalysisOutputFiles.m_fragmentShader, callback, config.m_printProcessCmdLines);
                            }

                            if (isComputeShaderPresent)
                            {
                                kcUtils::PerformLiveRegisterAnalysis(glOptions.m_isaDisassemblyOutputFiles.m_computeShader,
                                                                     glOptions.m_liveRegisterAnalysisOutputFiles.m_computeShader, callback, config.m_printProcessCmdLines);
                            }
                        }

                        // Generate control flow graph if required.
                        if (isBlockCfgRequired || isInstCfgRequired)
                        {
                            if (isVertexShaderPresent)
                            {
                                kcUtils::GenerateControlFlowGraph(glOptions.m_isaDisassemblyOutputFiles.m_vertexShader,
                                                                  glOptions.m_controlFlowGraphOutputFiles.m_vertexShader, callback, isInstCfgRequired, config.m_printProcessCmdLines);
                            }

                            if (isTessControlShaderPresent)
                            {
                                kcUtils::GenerateControlFlowGraph(glOptions.m_isaDisassemblyOutputFiles.m_tessControlShader,
                                                                  glOptions.m_controlFlowGraphOutputFiles.m_tessControlShader, callback, isInstCfgRequired, config.m_printProcessCmdLines);
                            }

                            if (isTessControlShaderPresent)
                            {
                                kcUtils::GenerateControlFlowGraph(glOptions.m_isaDisassemblyOutputFiles.m_tessEvaluationShader,
                                                                  glOptions.m_controlFlowGraphOutputFiles.m_tessEvaluationShader, callback, isInstCfgRequired, config.m_printProcessCmdLines);
                            }

                            if (isGeometryexShaderPresent)
                            {
                                kcUtils::GenerateControlFlowGraph(glOptions.m_isaDisassemblyOutputFiles.m_geometryShader,
                                                                  glOptions.m_controlFlowGraphOutputFiles.m_geometryShader, callback, isInstCfgRequired, config.m_printProcessCmdLines);
                            }

                            if (isFragmentShaderPresent)
                            {
                                kcUtils::GenerateControlFlowGraph(glOptions.m_isaDisassemblyOutputFiles.m_fragmentShader,
                                                                  glOptions.m_controlFlowGraphOutputFiles.m_fragmentShader, callback, isInstCfgRequired, config.m_printProcessCmdLines);
                            }

                            if (isComputeShaderPresent)
                            {
                                kcUtils::GenerateControlFlowGraph(glOptions.m_isaDisassemblyOutputFiles.m_computeShader,
                                                                  glOptions.m_controlFlowGraphOutputFiles.m_computeShader, callback, isInstCfgRequired, config.m_printProcessCmdLines);
                            }
                        }
                    }
                    else
                    {
                        logMsg << KA_CLI_STR_STATUS_FAILURE << std::endl;

                        if (buildStatus == beKA::beStatus_GLOpenGLVirtualContextFailedToLaunch)
                        {
                            logMsg << STR_ERR_CANNOT_INVOKE_COMPILER << std::endl;
                        }
                        else if (buildStatus == beKA::beStatus_FailedOutputVerification)
                        {
                            logMsg << STR_ERR_FAILED_OUTPUT_FILE_VERIFICATION << std::endl;
                        }
                    }

                    // Delete temporary files
                    if (isIsaRequired && config.m_ISAFile.empty())
                    {
                        kcUtils::DeletePipelineFiles(glOptions.m_isaDisassemblyOutputFiles);
                    }

                    // Notify the user about build errors if any.
                    if (!vcOutput.isEmpty())
                    {
                        logMsg << vcOutput.asASCIICharArray() << std::endl;
                    }

                    // Print the message for the current device.
                    callback(logMsg.str());

                    // Clear the output stream for the next iteration.
                    logMsg.str("");
                }
            }
        }
    }
    else
    {
        logMsg << KA_CLI_STR_ABORTING << std::endl;
    }

    // Print the output message.
    if (callback != nullptr)
    {
        callback(logMsg.str());
    }

}


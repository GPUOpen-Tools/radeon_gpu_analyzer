//=================================================================
// Copyright 2017 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

// C++.
#include <iterator>

// Local.
#include <RadeonGPUAnalyzerCLI/src/kcCLICommanderVulkan.h>
#include <RadeonGPUAnalyzerCLI/src/kcCliStringConstants.h>
#include <RadeonGPUAnalyzerCLI/src/kcUtils.h>
#include <RadeonGPUAnalyzerCLI/src/kcVulkanStatisticsParser.h>

// Backend.
#include <RadeonGPUAnalyzerBackend/include/beProgramBuilderVulkan.h>
#include <RadeonGPUAnalyzerBackend/include/beBackend.h>

// Shared between CLI and GUI.
#include <RadeonGPUAnalyzerCLI/../Utils/include/rgaVersionInfo.h>

kcCLICommanderVulkan::kcCLICommanderVulkan() : m_pVulkanBuilder(new beProgramBuilderVulkan)
{
}

kcCLICommanderVulkan::~kcCLICommanderVulkan()
{
    delete m_pVulkanBuilder;
}

bool kcCLICommanderVulkan::GetSupportedDevices()
{
    bool ret = !m_supportedDevicesCache.empty();
    if (!ret)
    {
        ret = beProgramBuilderVulkan::GetSupportedDevices(m_supportedDevicesCache);
    }
    return (ret && !m_supportedDevicesCache.empty());
}

void kcCLICommanderVulkan::Version(Config& config, LoggingCallBackFunc_t callback)
{
    GT_UNREFERENCED_PARAMETER(config);

    std::stringstream logMsg;
    logMsg << STR_RGA_PRODUCT_NAME << " " << STR_RGA_VERSION_PREFIX << STR_RGA_VERSION << "." << STR_RGA_BUILD_NUM << std::endl;

    if (m_pVulkanBuilder != nullptr)
    {
        gtString vkVersion;
        bool rc = m_pVulkanBuilder->GetVulkanVersion(vkVersion);

        if (rc && !vkVersion.isEmpty())
        {
            vkVersion << L"\n";
            logMsg << vkVersion.asASCIICharArray();
        }
        else
        {
            logMsg << STR_ERR_CANNOT_EXTRACT_OPENGL_VERSION << std::endl;
        }
    }

    callback(logMsg.str());
}


void kcCLICommanderVulkan::RunCompileCommands(const Config& config, LoggingCallBackFunc_t callback)
{
    m_LogCallback = callback;
    bool  status = true;

    // Output stream.
    std::stringstream logMsg, wrnMsg;

    // Input validation.
    bool shouldAbort = false;
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
    bool isIlRequired = (!config.m_ILFile.empty());
    bool isStatisticsRequired = (!config.m_AnalysisFile.empty());


    // Fatal error. This should not happen unless we have an allocation problem.
    if (m_pVulkanBuilder == nullptr)
    {
        shouldAbort = true;
        logMsg << STR_ERR_MEMORY_ALLOC_FAILURE << std::endl;
    }

    // Cannot mix compute and non-compute shaders in Vulkan.
    if (isComputeShaderPresent && (isVertexShaderPresent || isTessControlShaderPresent ||
                                   isTessEvaluationShaderPresent || isGeometryexShaderPresent || isFragmentShaderPresent))
    {
        logMsg << STR_ERR_RENDER_COMPUTE_MIX << std::endl;
        shouldAbort = true;
    }

    // Make sure that the input type is valid for Vulkan mode.
    if (config.m_SourceLanguage != SourceLanguage_GLSL_Vulkan &&
        config.m_SourceLanguage != SourceLanguage_SPIRV_Vulkan &&
        config.m_SourceLanguage != SourceLanguage_SPIRVTXT_Vulkan)
    {
        logMsg << STR_ERR_INVALID_INPUT_TYPE << std::endl;
        shouldAbort = true;
    }

    // Options to be passed to the backend.
    VulkanOptions vulkanOptions(config.m_SourceLanguage);

    if (!shouldAbort && !config.m_InputFiles.empty() && !config.m_InputFiles[0].empty())
    {
        shouldAbort = !kcUtils::ValidateShaderFileName("", config.m_InputFiles[0], logMsg);
        vulkanOptions.m_stagelessInputFile = config.m_InputFiles[0];
    }

    // Validate the input shaders.
    if (!shouldAbort && isVertexShaderPresent)
    {
        shouldAbort = !kcUtils::ValidateShaderFileName(KA_CLI_STR_VERTEX_SHADER, config.m_VertexShader, logMsg);
        vulkanOptions.m_pipelineShaders.m_vertexShader << config.m_VertexShader.c_str();
    }

    if (!shouldAbort && isTessControlShaderPresent)
    {
        shouldAbort = !kcUtils::ValidateShaderFileName(KA_CLI_STR_TESS_CTRL_SHADER, config.m_TessControlShader, logMsg);
        vulkanOptions.m_pipelineShaders.m_tessControlShader << config.m_TessControlShader.c_str();
    }

    if (!shouldAbort && isTessEvaluationShaderPresent)
    {
        shouldAbort = !kcUtils::ValidateShaderFileName(KA_CLI_STR_TESS_EVAL_SHADER, config.m_TessEvaluationShader, logMsg);
        vulkanOptions.m_pipelineShaders.m_tessEvaluationShader << config.m_TessEvaluationShader.c_str();
    }

    if (!shouldAbort && isGeometryexShaderPresent)
    {
        shouldAbort = !kcUtils::ValidateShaderFileName(KA_CLI_STR_GEOMETRY_SHADER, config.m_GeometryShader, logMsg);
        vulkanOptions.m_pipelineShaders.m_geometryShader << config.m_GeometryShader.c_str();
    }

    if (!shouldAbort && isFragmentShaderPresent)
    {
        shouldAbort = !kcUtils::ValidateShaderFileName(KA_CLI_STR_FRAGMENT_SHADER, config.m_FragmentShader, logMsg);
        vulkanOptions.m_pipelineShaders.m_fragmentShader << config.m_FragmentShader.c_str();
    }

    if (!shouldAbort && isComputeShaderPresent)
    {
        shouldAbort = !kcUtils::ValidateShaderFileName(KA_CLI_STR_COMPUTE_SHADER, config.m_ComputeShader, logMsg);
        vulkanOptions.m_pipelineShaders.m_computeShader << config.m_ComputeShader.c_str();
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

    if (!shouldAbort && isIlRequired)
    {
        shouldAbort = !kcUtils::ValidateShaderOutputDir(config.m_ILFile, logMsg);
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
        m_pVulkanBuilder->SetLog(callback);

        // If the user did not specify any device, we should use all supported devices.
        std::set<std::string> targetDevices;

        if (m_supportedDevicesCache.empty())
        {
            // We need to populate the list of supported devices.
            bool isDeviceListExtracted = GetSupportedDevices();

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
                // Generate the output message.
                logMsg << KA_CLI_STR_COMPILING << device << "... ";

                // Set the target device for the backend.
                vulkanOptions.m_targetDeviceName = device;

                // Set the optimization level.
                if (config.m_optLevel == -1 || config.m_optLevel == 0 || config.m_optLevel == 1)
                {
                    vulkanOptions.m_optLevel = config.m_optLevel;
                }
                else
                {
                    vulkanOptions.m_optLevel = -1;
                    wrnMsg << STR_WRN_INCORRECT_OPT_LEVEL << std::endl;
                }

                // Adjust the output file names to the device and shader type.
                if (isIsaRequired || isIsaBinary)
                {
                    // We must generate the ISA binaries (we will delete them in the end of the process).
                    vulkanOptions.m_isAmdIsaBinariesRequired = true;
                    status &= kcUtils::AdjustRenderingPipelineOutputFileNames(config.m_BinaryOutputFile, KC_STR_DEFAULT_BIN_SUFFIX, device, vulkanOptions.m_isaBinaryFiles);

                    if (isIsaRequired)
                    {
                        vulkanOptions.m_isAmdIsaDisassemblyRequired = true;
                        status &= kcUtils::AdjustRenderingPipelineOutputFileNames(config.m_ISAFile, KC_STR_DEFAULT_ISA_SUFFIX, device, vulkanOptions.m_isaDisassemblyOutputFiles);
                    }
                }

                if (isLiveRegAnalysisRequired)
                {
                    vulkanOptions.m_isLiveRegisterAnalysisRequired = true;
                    status &= kcUtils::AdjustRenderingPipelineOutputFileNames(config.m_LiveRegisterAnalysisFile, KC_STR_DEFAULT_LIVE_REG_ANALYSIS_SUFFIX,
                                                                              device, vulkanOptions.m_liveRegisterAnalysisOutputFiles);
                }

                if (isBlockCfgRequired)
                {
                    status &= kcUtils::AdjustRenderingPipelineOutputFileNames(config.m_blockCFGFile, KC_STR_DEFAULT_CFG_EXT,
                                                                              device, vulkanOptions.m_controlFlowGraphOutputFiles);
                }

                if (isInstCfgRequired)
                {
                    status &= kcUtils::AdjustRenderingPipelineOutputFileNames(config.m_instCFGFile, KC_STR_DEFAULT_CFG_EXT,
                                                                              device, vulkanOptions.m_controlFlowGraphOutputFiles);
                }

                if (isIlRequired)
                {
                    vulkanOptions.m_isAmdPalIlDisassemblyRequired = true;
                    status &= kcUtils::AdjustRenderingPipelineOutputFileNames(config.m_ILFile, KC_STR_DEFAULT_AMD_IL_SUFFIX, device, vulkanOptions.m_pailIlDisassemblyOutputFiles);
                }

                if (isStatisticsRequired)
                {
                    vulkanOptions.m_isScStatsRequired = true;
                    status &= kcUtils::AdjustRenderingPipelineOutputFileNames(config.m_AnalysisFile, KC_STR_DEFAULT_STATISTICS_SUFFIX, device, vulkanOptions.m_scStatisticsOutputFiles);
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
                gtString buildErrorLog;
                beKA::beStatus compilationStatus = m_pVulkanBuilder->Compile(vulkanOptions, shouldCancel, config.m_printProcessCmdLines, buildErrorLog);

                if (compilationStatus == beStatus_SUCCESS)
                {
                    logMsg << KA_CLI_STR_STATUS_SUCCESS << std::endl;

                    // Parse ISA and write it to a csv file if required.
                    if (isIsaRequired && config.m_isParsedISARequired)
                    {
                        bool  status;
                        std::string  isaText, parsedIsaText, parsedIsaFileName;
                        beProgramPipeline  isaFiles = vulkanOptions.m_isaDisassemblyOutputFiles;
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

                    // Parse the statistics file if required.
                    if (isStatisticsRequired)
                    {
                        beKA::AnalysisData statistics;
                        kcVulkanStatisticsParser statsParser;

                        if (isVertexShaderPresent)
                        {
                            kcUtils::ReplaceStatisticsFile(vulkanOptions.m_scStatisticsOutputFiles.m_vertexShader, config, device, statsParser, callback);
                        }

                        if (isTessControlShaderPresent)
                        {
                            kcUtils::ReplaceStatisticsFile(vulkanOptions.m_scStatisticsOutputFiles.m_tessControlShader, config, device, statsParser, callback);
                        }

                        if (isTessEvaluationShaderPresent)
                        {
                            kcUtils::ReplaceStatisticsFile(vulkanOptions.m_scStatisticsOutputFiles.m_tessEvaluationShader, config, device, statsParser, callback);
                        }

                        if (isGeometryexShaderPresent)
                        {
                            kcUtils::ReplaceStatisticsFile(vulkanOptions.m_scStatisticsOutputFiles.m_geometryShader, config, device, statsParser, callback);
                        }

                        if (isFragmentShaderPresent)
                        {
                            kcUtils::ReplaceStatisticsFile(vulkanOptions.m_scStatisticsOutputFiles.m_fragmentShader, config, device, statsParser, callback);
                        }

                        if (isComputeShaderPresent)
                        {
                            kcUtils::ReplaceStatisticsFile(vulkanOptions.m_scStatisticsOutputFiles.m_computeShader, config, device, statsParser, callback);
                        }
                    }

                    // Perform live register analysis if required.
                    if (isLiveRegAnalysisRequired)
                    {
                        if (isVertexShaderPresent)
                        {
                            kcUtils::PerformLiveRegisterAnalysis(vulkanOptions.m_isaDisassemblyOutputFiles.m_vertexShader,
                                                                 vulkanOptions.m_liveRegisterAnalysisOutputFiles.m_vertexShader, callback, config.m_printProcessCmdLines);
                        }

                        if (isTessControlShaderPresent)
                        {
                            kcUtils::PerformLiveRegisterAnalysis(vulkanOptions.m_isaDisassemblyOutputFiles.m_tessControlShader,
                                                                 vulkanOptions.m_liveRegisterAnalysisOutputFiles.m_tessControlShader, callback, config.m_printProcessCmdLines);
                        }

                        if (isTessEvaluationShaderPresent)
                        {
                            kcUtils::PerformLiveRegisterAnalysis(vulkanOptions.m_isaDisassemblyOutputFiles.m_tessEvaluationShader,
                                                                 vulkanOptions.m_liveRegisterAnalysisOutputFiles.m_tessEvaluationShader, callback, config.m_printProcessCmdLines);
                        }

                        if (isGeometryexShaderPresent)
                        {
                            kcUtils::PerformLiveRegisterAnalysis(vulkanOptions.m_isaDisassemblyOutputFiles.m_geometryShader,
                                                                 vulkanOptions.m_liveRegisterAnalysisOutputFiles.m_geometryShader, callback, config.m_printProcessCmdLines);
                        }

                        if (isFragmentShaderPresent)
                        {
                            kcUtils::PerformLiveRegisterAnalysis(vulkanOptions.m_isaDisassemblyOutputFiles.m_fragmentShader,
                                                                 vulkanOptions.m_liveRegisterAnalysisOutputFiles.m_fragmentShader, callback, config.m_printProcessCmdLines);
                        }

                        if (isComputeShaderPresent)
                        {
                            kcUtils::PerformLiveRegisterAnalysis(vulkanOptions.m_isaDisassemblyOutputFiles.m_computeShader,
                                                                 vulkanOptions.m_liveRegisterAnalysisOutputFiles.m_computeShader, callback, config.m_printProcessCmdLines);
                        }

                        // Process stageless files.
                        if (!vulkanOptions.m_stagelessInputFile.empty())
                        {
                            for (const auto& isaAndLiveReg : {
                                   std::pair<gtString, gtString>{vulkanOptions.m_isaDisassemblyOutputFiles.m_vertexShader, vulkanOptions.m_liveRegisterAnalysisOutputFiles.m_vertexShader},
                                   std::pair<gtString, gtString>{vulkanOptions.m_isaDisassemblyOutputFiles.m_tessControlShader, vulkanOptions.m_liveRegisterAnalysisOutputFiles.m_tessControlShader},
                                   std::pair<gtString, gtString>{vulkanOptions.m_isaDisassemblyOutputFiles.m_tessEvaluationShader, vulkanOptions.m_liveRegisterAnalysisOutputFiles.m_tessEvaluationShader},
                                   std::pair<gtString, gtString>{vulkanOptions.m_isaDisassemblyOutputFiles.m_geometryShader, vulkanOptions.m_liveRegisterAnalysisOutputFiles.m_geometryShader},
                                   std::pair<gtString, gtString>{vulkanOptions.m_isaDisassemblyOutputFiles.m_fragmentShader, vulkanOptions.m_liveRegisterAnalysisOutputFiles.m_fragmentShader},
                                   std::pair<gtString, gtString>{vulkanOptions.m_isaDisassemblyOutputFiles.m_computeShader, vulkanOptions.m_liveRegisterAnalysisOutputFiles.m_computeShader} } )
                            {
                                if (kcUtils::FileNotEmpty(isaAndLiveReg.first.asASCIICharArray()))
                                {
                                    kcUtils::PerformLiveRegisterAnalysis(isaAndLiveReg.first, isaAndLiveReg.second, callback, config.m_printProcessCmdLines);
                                }
                            }
                        }
                    }

                    // Generate control flow graph if required.
                    if (isInstCfgRequired || isBlockCfgRequired)
                    {
                        if (isVertexShaderPresent)
                        {
                            kcUtils::GenerateControlFlowGraph(vulkanOptions.m_isaDisassemblyOutputFiles.m_vertexShader,
                                                              vulkanOptions.m_controlFlowGraphOutputFiles.m_vertexShader, callback, isInstCfgRequired, config.m_printProcessCmdLines);
                        }

                        if (isTessControlShaderPresent)
                        {
                            kcUtils::GenerateControlFlowGraph(vulkanOptions.m_isaDisassemblyOutputFiles.m_tessControlShader,
                                                              vulkanOptions.m_controlFlowGraphOutputFiles.m_tessControlShader, callback, isInstCfgRequired, config.m_printProcessCmdLines);
                        }

                        if (isTessEvaluationShaderPresent)
                        {
                            kcUtils::GenerateControlFlowGraph(vulkanOptions.m_isaDisassemblyOutputFiles.m_tessEvaluationShader,
                                                              vulkanOptions.m_controlFlowGraphOutputFiles.m_tessEvaluationShader, callback, isInstCfgRequired, config.m_printProcessCmdLines);
                        }

                        if (isGeometryexShaderPresent)
                        {
                            kcUtils::GenerateControlFlowGraph(vulkanOptions.m_isaDisassemblyOutputFiles.m_geometryShader,
                                                              vulkanOptions.m_controlFlowGraphOutputFiles.m_geometryShader, callback, isInstCfgRequired, config.m_printProcessCmdLines);
                        }

                        if (isFragmentShaderPresent)
                        {
                            kcUtils::GenerateControlFlowGraph(vulkanOptions.m_isaDisassemblyOutputFiles.m_fragmentShader,
                                                              vulkanOptions.m_controlFlowGraphOutputFiles.m_fragmentShader, callback, isInstCfgRequired, config.m_printProcessCmdLines);
                        }

                        if (isComputeShaderPresent)
                        {
                            kcUtils::GenerateControlFlowGraph(vulkanOptions.m_isaDisassemblyOutputFiles.m_computeShader,
                                                              vulkanOptions.m_controlFlowGraphOutputFiles.m_computeShader, callback, isInstCfgRequired, config.m_printProcessCmdLines);
                        }

                        // Process stageless files.
                        if (!vulkanOptions.m_stagelessInputFile.empty())
                        {
                            for (const auto& isaAndCfg : {
                                std::pair<gtString, gtString>{vulkanOptions.m_isaDisassemblyOutputFiles.m_vertexShader, vulkanOptions.m_controlFlowGraphOutputFiles.m_vertexShader},
                                std::pair<gtString, gtString>{vulkanOptions.m_isaDisassemblyOutputFiles.m_tessControlShader, vulkanOptions.m_controlFlowGraphOutputFiles.m_tessControlShader},
                                std::pair<gtString, gtString>{vulkanOptions.m_isaDisassemblyOutputFiles.m_tessEvaluationShader, vulkanOptions.m_controlFlowGraphOutputFiles.m_tessEvaluationShader},
                                std::pair<gtString, gtString>{vulkanOptions.m_isaDisassemblyOutputFiles.m_geometryShader, vulkanOptions.m_controlFlowGraphOutputFiles.m_geometryShader},
                                std::pair<gtString, gtString>{vulkanOptions.m_isaDisassemblyOutputFiles.m_fragmentShader, vulkanOptions.m_controlFlowGraphOutputFiles.m_fragmentShader},
                                std::pair<gtString, gtString>{vulkanOptions.m_isaDisassemblyOutputFiles.m_computeShader, vulkanOptions.m_controlFlowGraphOutputFiles.m_computeShader} })
                            {
                                if (kcUtils::FileNotEmpty(isaAndCfg.first.asASCIICharArray()))
                                {
                                    kcUtils::GenerateControlFlowGraph(isaAndCfg.first, isaAndCfg.second, callback, isInstCfgRequired, config.m_printProcessCmdLines);
                                }
                            }
                        }
                    }
                }
                else
                {
                    logMsg << KA_CLI_STR_STATUS_FAILURE << std::endl;

                    switch (compilationStatus)
                    {
                    case beStatus_VulkanAmdspvCompilationFailure:
                        logMsg << buildErrorLog.asASCIICharArray();
                        break;
                    case beStatus_VulkanAmdspvLaunchFailure:
                        logMsg << STR_ERR_CANNOT_INVOKE_COMPILER << std::endl;
                        break;
                    case beStatus_VulkanNoInputFile:
                        logMsg << STR_ERR_NO_INPUT_FILE << std::endl;
                        break;
                    case beStatus_VulkanMixedInputFiles:
                        logMsg << STR_ERR_MIXED_INPUT_FILES << std::endl;
                        break;
                    case beStatus_FailedOutputVerification:
                        logMsg << STR_ERR_FAILED_OUTPUT_FILE_VERIFICATION << std::endl;
                        break;
                    }
                }

                // Delete temporary files
                if (isIsaRequired && config.m_ISAFile.empty())
                {
                    kcUtils::DeletePipelineFiles(vulkanOptions.m_isaDisassemblyOutputFiles);
                }
                if (isIsaRequired && config.m_BinaryOutputFile.empty())
                {
                    kcUtils::DeletePipelineFiles(vulkanOptions.m_isaBinaryFiles);
                }

                // Print the message for the current device.
                callback(logMsg.str());

                // Clear the output stream for the next iteration.
                logMsg.str("");
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
        callback(wrnMsg.str());
    }
}

//=================================================================
// Copyright 2019 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

#ifdef _WIN32

// C++.
#include <cassert>

// Backend.
#include <RadeonGPUAnalyzerBackend/Include/beUtils.h>

// Local.
#include <RadeonGPUAnalyzerCLI/Src/kcCLICommanderDX12.h>

// Device info.
#include <DeviceInfoUtils.h>

// *****************************************
// *** INTERNALLY LINKED SYMBOLS - START ***
// *****************************************

// Constants - error messages.
static const char* STR_ERROR_DX12_NO_TARGET_PROVIDED = "Error: no target device provided.";
static const char* STR_ERROR_DX12_TWO_INPUT_FILES_PER_STAGE_A = "Error: both HLSL and DXBC input files specified for ";
static const char* STR_ERROR_DX12_TWO_INPUT_FILES_PER_STAGE_B = "shader. Only a single input file can be passed per stage.";
static const char* STR_ERROR_DX12_NO_SHADER_MODEL_PROVIDED_A = "Error: no shader model provided for ";
static const char* STR_ERROR_DX12_NO_SHADER_MODEL_PROVIDED_B = " shader.";
static const char* STR_ERROR_DX12_NO_ENTRY_POINT_PROVIDED_A = "Error: no entry point provided for ";
static const char* STR_ERROR_DX12_NO_ENTRY_POINT_PROVIDED_B = " shader.";
static const char* STR_ERROR_DX12_SHADER_NOT_FOUND_OR_EMPTY_A = "Error: ";
static const char* STR_ERROR_DX12_SHADER_NOT_FOUND_OR_EMPTY_B = " shader does not exist or file is empty: ";
static const char* STR_ERROR_DX12_ISA_NOT_GENERATED_A = "Error: failed to generated ISA disassembly for ";
static const char* STR_ERROR_DX12_ISA_NOT_GENERATED_B = " shader";
static const char* STR_ERROR_DX12_STATS_NOT_GENERATED_A = "Error: failed to generated resource usage statistics for ";
static const char* STR_ERROR_DX12_STATS_NOT_GENERATED_B = " shader";

// Constants - warnings messages.
static const char* STR_WARNING_DX12_BINARY_OPTION_NOT_SUPPORTED = "Warning: extracting pipeline binary option (-b) is currently "
"not supported for DX12 mode.";
static const char* STR_WARNING_DX12_AMDIL_OPTION_NOT_SUPPORTED = "Warning: extracting AMD IL disassembly is currently not "
"supported for DX12 mode. You can extract DXIL disassembly (for shader model 5.1 and above) using the <stage>-dxil-dis option. "
"See -h for more details.";

// Validates the input arguments for a specific stage, and prints appropriate error messages to the console.
static bool IsShaderInputsValid(const std::string& shaderHlsl, const std::string& shaderModel,
    const std::string& shaderEntryPoint, const std::string& stageName)
{
    bool ret = true;
    if (!shaderHlsl.empty())
    {
        if (!kcUtils::FileNotEmpty(shaderHlsl))
        {
            std::cout << STR_ERROR_DX12_SHADER_NOT_FOUND_OR_EMPTY_A <<
                stageName << STR_ERROR_DX12_SHADER_NOT_FOUND_OR_EMPTY_B << shaderHlsl << std::endl;
            ret = false;
        }

        // Validate inputs for shader front-end compilation.
        if (shaderModel.empty())
        {
            std::cout << STR_ERROR_DX12_NO_SHADER_MODEL_PROVIDED_A << stageName <<
                STR_ERROR_DX12_NO_SHADER_MODEL_PROVIDED_B << std::endl;
            ret = false;
        }
        else if (shaderEntryPoint.empty())
        {
            std::cout << STR_ERROR_DX12_NO_ENTRY_POINT_PROVIDED_A << stageName <<
                STR_ERROR_DX12_NO_ENTRY_POINT_PROVIDED_B << std::endl;
            ret = false;
        }
    }
    return ret;
}

// Validates that a DXBC input file is valid.
static bool IsDxbcInputValid(const std::string& dxbcInputFile, const std::string& stage)
{
    bool ret = true;
    if (!dxbcInputFile.empty() && !kcUtils::FileNotEmpty(dxbcInputFile))
    {
        std::cout << STR_ERROR_DX12_SHADER_NOT_FOUND_OR_EMPTY_A <<
            stage << STR_ERROR_DX12_SHADER_NOT_FOUND_OR_EMPTY_B << dxbcInputFile << std::endl;
        ret = false;
    }
    return ret;
}

// Validates the input and prints appropriate error messages to the console.
static bool IsInputValid(const Config& config)
{
    bool ret = true;

    if (!config.m_csHlsl.empty() && !config.m_csDxbc.empty())
    {
        std::cout << STR_ERROR_DX12_TWO_INPUT_FILES_PER_STAGE_A << "compute " <<
            STR_ERROR_DX12_TWO_INPUT_FILES_PER_STAGE_B << std::endl;
        ret = false;
    }
    else if (!config.m_vsHlsl.empty() && !config.m_vsDxbc.empty())
    {
        std::cout << STR_ERROR_DX12_TWO_INPUT_FILES_PER_STAGE_A << "vertex " <<
            STR_ERROR_DX12_TWO_INPUT_FILES_PER_STAGE_B << std::endl;
        ret = false;
    }
    else if (!config.m_hsHlsl.empty() && !config.m_hsDxbc.empty())
    {
        std::cout << STR_ERROR_DX12_TWO_INPUT_FILES_PER_STAGE_A << "hull " <<
            STR_ERROR_DX12_TWO_INPUT_FILES_PER_STAGE_B << std::endl;
        ret = false;
    }
    else if (!config.m_dsHlsl.empty() && !config.m_dsDxbc.empty())
    {
        std::cout << STR_ERROR_DX12_TWO_INPUT_FILES_PER_STAGE_A << "domain " <<
            STR_ERROR_DX12_TWO_INPUT_FILES_PER_STAGE_B << std::endl;
        ret = false;
    }
    else if (!config.m_gsHlsl.empty() && !config.m_gsDxbc.empty())
    {
        std::cout << STR_ERROR_DX12_TWO_INPUT_FILES_PER_STAGE_A << "geometry " <<
            STR_ERROR_DX12_TWO_INPUT_FILES_PER_STAGE_B << std::endl;
        ret = false;
    }
    else if (!config.m_psHlsl.empty() && !config.m_psDxbc.empty())
    {
        std::cout << STR_ERROR_DX12_TWO_INPUT_FILES_PER_STAGE_A << "pixel " <<
            STR_ERROR_DX12_TWO_INPUT_FILES_PER_STAGE_B << std::endl;
        ret = false;
    }

    if (ret)
    {
        if (!config.m_csHlsl.empty())
        {
            // Compute.
            if (!IsShaderInputsValid(config.m_csHlsl, config.m_csModel,
                config.m_csEntryPoint, STR_DX12_STAGE_NAMES[bePipelineStage::Compute]))
            {
                ret = false;
            }
        }
        else
        {
            // Vertex.
            if (!IsShaderInputsValid(config.m_vsHlsl, config.m_vsModel,
                config.m_vsEntryPoint, STR_DX12_STAGE_NAMES[bePipelineStage::Vertex]))
            {
                ret = false;
            }

            // Hull.
            if (!IsShaderInputsValid(config.m_hsHlsl, config.m_hsModel,
                config.m_hsEntryPoint, STR_DX12_STAGE_NAMES[bePipelineStage::TessellationControl]))
            {
                ret = false;
            }

            // Domain.
            if (!IsShaderInputsValid(config.m_dsHlsl, config.m_dsModel,
                config.m_dsEntryPoint, STR_DX12_STAGE_NAMES[bePipelineStage::TessellationEvaluation]))
            {
                ret = false;
            }

            // Geometry.
            if (!IsShaderInputsValid(config.m_gsHlsl, config.m_gsModel,
                config.m_gsEntryPoint, STR_DX12_STAGE_NAMES[bePipelineStage::Geometry]))
            {
                ret = false;
            }

            // Pixel.
            if (!IsShaderInputsValid(config.m_psHlsl, config.m_psModel,
                config.m_psEntryPoint, STR_DX12_STAGE_NAMES[bePipelineStage::Fragment]))
            {
                ret = false;
            }
        }
    }

    // Verify DXBC input files, if relevant.
    if (ret)
    {
        if (!IsDxbcInputValid(config.m_vsDxbc, STR_DX12_STAGE_NAMES[bePipelineStage::Vertex]))
        {
            ret = false;
        }
        if (!IsDxbcInputValid(config.m_hsDxbc, STR_DX12_STAGE_NAMES[bePipelineStage::TessellationControl]))
        {
            ret = false;
        }
        if (!IsDxbcInputValid(config.m_dsDxbc, STR_DX12_STAGE_NAMES[bePipelineStage::TessellationEvaluation]))
        {
            ret = false;
        }
        if (!IsDxbcInputValid(config.m_gsDxbc, STR_DX12_STAGE_NAMES[bePipelineStage::Geometry]))
        {
            ret = false;
        }
        if (!IsDxbcInputValid(config.m_psDxbc, STR_DX12_STAGE_NAMES[bePipelineStage::Fragment]))
        {
            ret = false;
        }
        if (!IsDxbcInputValid(config.m_csDxbc, STR_DX12_STAGE_NAMES[bePipelineStage::Compute]))
        {
            ret = false;
        }
    }

    return ret;
}

// ****************************************
// *** INTERNALLY LINKED SYMBOLS - END ***
// ****************************************

void kcCLICommanderDX12::ListAdapters(Config& config, LoggingCallBackFunc_t callback)
{
    std::vector<std::string> supportedGpus;
    std::map<std::string, int> driverMapping;
    m_dx12Backend.GetSupportGpus(config, supportedGpus, driverMapping);
}

void kcCLICommanderDX12::RunCompileCommands(const Config& config, LoggingCallBackFunc_t callback)
{
    bool isOk = false;

    // Container for all targets.
    std::vector<std::string> targetDevices;

    // Targets that have been covered.
    std::vector<std::string> completedTargets;

    if (IsInputValid(config))
    {
        if (config.m_ASICs.empty())
        {
            std::vector<GDT_GfxCardInfo> tmpCardList;
            std::set<std::string> deviceList;
            isOk = beUtils::GetAllGraphicsCards(tmpCardList, deviceList);
            assert(isOk);
            assert(!deviceList.empty());
            if (isOk && !deviceList.empty())
            {
                // Sort and choose the latest target.
                std::set<std::string, decltype(&beUtils::DeviceNameLessThan)> sortUniqueNames(deviceList.begin(),
                    deviceList.end(), beUtils::DeviceNameLessThan);
                targetDevices.push_back(*sortUniqueNames.rbegin());
            }
        }
        else
        {
            targetDevices = config.m_ASICs;
        }
        assert(!targetDevices.empty());
        if (!targetDevices.empty())
        {
            for (const std::string& target : targetDevices)
            {
                // Track the devices that we covered so that we do not compile twice.
                if (std::find(completedTargets.begin(),
                    completedTargets.end(), target) == completedTargets.end())
                {
                    // Mark as covered.
                    completedTargets.push_back(target);

                    std::string outText;
                    std::string errorMsg;
                    beVkPipelineFiles isaFiles;
                    beVkPipelineFiles statsFiles;
                    std::cout << KA_CLI_STR_COMPILING << target << "..." << std::endl;
                    beStatus rc = m_dx12Backend.Compile(config, target, outText, errorMsg, isaFiles, statsFiles);
                    isOk = (rc == beStatus_SUCCESS);
                    if (!outText.empty())
                    {
                        std::cout << outText << std::endl;
                    }
                    if (isOk)
                    {
                        if (!errorMsg.empty())
                        {
                            std::cout << errorMsg << std::endl;
                        }

                        // Warnings.
                        if (!config.m_BinaryOutputFile.empty())
                        {
                            std::cout << STR_WARNING_DX12_BINARY_OPTION_NOT_SUPPORTED << std::endl;
                        }
                        if (!config.m_ILFile.empty())
                        {
                            std::cout << STR_WARNING_DX12_AMDIL_OPTION_NOT_SUPPORTED << std::endl;
                        }

                        bool isSuccess = true;
                        for (int stage = 0; stage < bePipelineStage::Count; stage++)
                        {
                            if (!isaFiles[stage].empty() && !kcUtils::FileNotEmpty(isaFiles[stage]))
                            {
                                std::cout << STR_ERROR_DX12_ISA_NOT_GENERATED_A <<
                                    STR_DX12_STAGE_NAMES[stage] << STR_ERROR_DX12_ISA_NOT_GENERATED_B << std::endl;
                                isSuccess = false;
                            }
                            if (!statsFiles[stage].empty() && !kcUtils::FileNotEmpty(statsFiles[stage]))
                            {
                                std::cout << STR_ERROR_DX12_STATS_NOT_GENERATED_A
                                    << STR_DX12_STAGE_NAMES[stage] << STR_ERROR_DX12_STATS_NOT_GENERATED_B << std::endl;
                                isSuccess = false;
                            }
                        }

                        if (isSuccess)
                        {
                            std::cout << KA_CLI_STR_STATUS_SUCCESS << std::endl;

                            if (!config.m_LiveRegisterAnalysisFile.empty() ||
                                !config.m_instCFGFile.empty() || !config.m_blockCFGFile.empty())
                            {
                                if (!kcUtils::IsNaviTarget(target))
                                {
                                    // Post-processing.
                                    std::cout << STR_INFO_POST_PROCESSING_SEPARATOR << std::endl;
                                    std::cout << STR_INFO_POST_PROCESSING << std::endl;

                                    if (!config.m_LiveRegisterAnalysisFile.empty())
                                    {
                                        // Live register analysis files.
                                        for (int stage = 0; stage < bePipelineStage::Count; stage++)
                                        {
                                            if (!isaFiles[stage].empty())
                                            {
                                                std::cout << STR_INFO_PERFORMING_LIVEREG_ANALYSIS_A <<
                                                    STR_DX12_STAGE_NAMES[stage] << STR_INFO_PERFORMING_LIVEREG_ANALYSIS_B << std::endl;

                                                // Construct a name for the output file.
                                                std::string outputFileName;
                                                isOk = kcUtils::ConstructOutFileName(config.m_LiveRegisterAnalysisFile, STR_DX12_STAGE_NAMES[stage],
                                                    target, KC_STR_DEFAULT_LIVEREG_EXT, outputFileName);

                                                if (isOk)
                                                {
                                                    // Delete that file if it already exists.
                                                    if (beUtils::IsFilePresent(outputFileName))
                                                    {
                                                        beUtils::DeleteFileFromDisk(outputFileName.c_str());
                                                    }

                                                    isOk = kcUtils::PerformLiveRegisterAnalysis(isaFiles[stage], outputFileName, NULL,
                                                        config.m_printProcessCmdLines);
                                                    if (isOk)
                                                    {
                                                        if (kcUtils::FileNotEmpty(outputFileName))
                                                        {
                                                            std::cout << KA_CLI_STR_STATUS_SUCCESS << std::endl;
                                                        }
                                                        else
                                                        {
                                                            std::cout << KA_CLI_STR_STATUS_FAILURE << std::endl;
                                                        }
                                                    }
                                                }
                                                else
                                                {
                                                    std::cout << STR_ERR_FAILED_TO_CONSTRUCT_LIVEREG_OUTPUT_FILE << std::endl;
                                                }
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    std::cout << STR_WARNING_LIVEREG_NOT_SUPPORTED_TO_NAVI << std::endl;
                                }

                                if (!config.m_blockCFGFile.empty())
                                {
                                    if (!kcUtils::IsNaviTarget(target))
                                    {
                                        // Per-block control-flow graphs.
                                        for (int stage = 0; stage < bePipelineStage::Count; stage++)
                                        {
                                            if (!isaFiles[stage].empty())
                                            {
                                                std::cout << STR_INFO_CONSTRUCTING_BLOCK_CFG_A <<
                                                    STR_DX12_STAGE_NAMES[stage] << STR_INFO_CONSTRUCTING_BLOCK_CFG_B << std::endl;

                                                // Construct a name for the output file.
                                                std::string outputFileName;
                                                isOk = kcUtils::ConstructOutFileName(config.m_blockCFGFile, STR_DX12_STAGE_NAMES[stage],
                                                    target, KC_STR_DEFAULT_CFG_EXT, outputFileName);

                                                if (isOk)
                                                {
                                                    // Delete that file if it already exists.
                                                    if (beUtils::IsFilePresent(outputFileName))
                                                    {
                                                        beUtils::DeleteFileFromDisk(outputFileName);
                                                    }

                                                    isOk = kcUtils::GenerateControlFlowGraph(isaFiles[stage], outputFileName, NULL,
                                                        false, config.m_printProcessCmdLines);
                                                    if (isOk)
                                                    {
                                                        if (kcUtils::FileNotEmpty(outputFileName))
                                                        {
                                                            std::cout << KA_CLI_STR_STATUS_SUCCESS << std::endl;
                                                        }
                                                        else
                                                        {
                                                            std::cout << KA_CLI_STR_STATUS_FAILURE << std::endl;
                                                        }
                                                    }
                                                }
                                                else
                                                {
                                                    std::cout << STR_ERR_FAILED_TO_CONSTRUCT_CFG_OUTPUT_FILE << std::endl;
                                                }
                                            }
                                        }
                                    }
                                    else
                                    {
                                        std::cout << STR_WARNING_CFG_NOT_SUPPORTED_TO_NAVI << std::endl;
                                    }
                                }

                                if (!config.m_instCFGFile.empty())
                                {
                                    if(!kcUtils::IsNaviTarget(target))
                                    {
                                        // Per-instruction control-flow graphs.
                                        for (int stage = 0; stage < bePipelineStage::Count; stage++)
                                        {
                                            if (!isaFiles[stage].empty())
                                            {
                                                std::cout << STR_INFO_CONSTRUCTING_INSTRUCTION_CFG_A <<
                                                    STR_DX12_STAGE_NAMES[stage] << STR_INFO_CONSTRUCTING_INSTRUCTION_CFG_B << std::endl;

                                                // Construct a name for the output file.
                                                std::string outputFileName;
                                                isOk = kcUtils::ConstructOutFileName(config.m_instCFGFile, STR_DX12_STAGE_NAMES[stage],
                                                    target, KC_STR_DEFAULT_CFG_EXT, outputFileName);

                                                if (isOk)
                                                {
                                                    // Delete that file if it already exists.
                                                    if (beUtils::IsFilePresent(outputFileName))
                                                    {
                                                        beUtils::DeleteFileFromDisk(outputFileName);
                                                    }

                                                    isOk = kcUtils::GenerateControlFlowGraph(isaFiles[stage], outputFileName, NULL,
                                                        true, config.m_printProcessCmdLines);
                                                    if (isOk)
                                                    {
                                                        if (kcUtils::FileNotEmpty(outputFileName))
                                                        {
                                                            std::cout << KA_CLI_STR_STATUS_SUCCESS << std::endl;
                                                        }
                                                        else
                                                        {
                                                            std::cout << KA_CLI_STR_STATUS_FAILURE << std::endl;
                                                        }
                                                    }
                                                }
                                                else
                                                {
                                                    std::cout << STR_ERR_FAILED_TO_CONSTRUCT_CFG_OUTPUT_FILE << std::endl;
                                                }
                                            }
                                        }
                                    }
                                    else
                                    {
                                        std::cout << STR_WARNING_CFG_NOT_SUPPORTED_TO_NAVI << std::endl;
                                    }
                                }
                            }
                        }
                    }
                    else if (!errorMsg.empty())
                    {
                        std::cout << errorMsg << std::endl;
                    }
                    else
                    {
                        std::cout << KA_CLI_STR_STATUS_FAILURE << std::endl;
                    }

                    if (targetDevices.size() > 1)
                    {
                        // In case that we are compiling for multiple targets,
                        // print a line of space between the different devices.
                        std::cout << std::endl;
                    }
                }
            }
        }
        else
        {
            std::cout << STR_ERROR_DX12_NO_TARGET_PROVIDED << std::endl;
        }
    }
}


bool kcCLICommanderDX12::PrintAsicList(const Config& config)
{
    std::vector<std::string> supportedGpus;
    std::map<std::string, int> deviceIdMapping;

    // Retrieve the list of supported targets from the DX12 backend.
    beStatus rc = m_dx12Backend.GetSupportGpus(config, supportedGpus, deviceIdMapping);
    bool result = (rc == beStatus_SUCCESS);
    assert(result);

    // Filter duplicates and call the shared print routine.
    std::set<std::string> supportedGpusUnique = std::set<std::string>(supportedGpus.begin(), supportedGpus.end());
    result = result && kcUtils::PrintAsicList(supportedGpusUnique);

    return result;
}

#endif

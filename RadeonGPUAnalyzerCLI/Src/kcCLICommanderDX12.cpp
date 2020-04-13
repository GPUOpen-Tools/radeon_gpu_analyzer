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
static const char* STR_ERROR_DX12_ISA_NOT_GENERATED_A = "Error: failed to generate ISA disassembly for ";
static const char* STR_ERROR_DX12_ISA_NOT_GENERATED_B = " shader";
static const char* STR_ERROR_DX12_STATS_NOT_GENERATED_A = "Error: failed to generate resource usage statistics for ";
static const char* STR_ERROR_DX12_STATS_NOT_GENERATED_B = " shader";
static const char* STR_ERROR_NO_GPSO_FILE = "Error: .gpso file must be provided for graphics pipelines (use the --pso option).";
static const char* STR_ERROR_NO_GPSO_FILE_HINT_A = "The format of a .gpso file is : ";
static const char* STR_ERROR_NO_GPSO_FILE_HINT_B = "See --gpso-template option for more info.";
static const char* STR_ERROR_MIX_COMPUTE_GRAPHICS_INPUT_A = "Error: cannot mix compute and graphics for ";
static const char* STR_ERROR_MIX_COMPUTE_GRAPHICS_INPUT_SHADER = "shader input files.";
static const char* STR_ERROR_MIX_COMPUTE_GRAPHICS_INPUT_DXIL_DISASSEMBLY = "DXIL/DXBC disassembly output file.";
static const char* STR_ERROR_MIX_COMPUTE_GRAPHICS_INPUT_SHADER_MODEL = "shader model.";
static const char* STR_ERROR_MIX_COMPUTE_GRAPHICS_INPUT_ENTRY_POINT = "entry point.";
static const char* STR_ERROR_NO_ISA_NO_STATS_OPTION = "Error: one of --isa or -a/--analysis or -b/--binary options must be provided.";
static const char* STR_ERROR_NO_ROOT_SIGNATURE_HLSL_FILE_DEFINED = "Error: use --rs-hlsl option together with --rs-macro to specify the HLSL file where the macro is defined.";
static const char* STR_WARNING_AUTO_DEDUCING_RS_HLSL = "Warning: --rs-hlsl option not provided, assuming that root signature macro is defined in ";

// Constants - warnings messages.
static const char* STR_WARNING_DX12_AMDIL_OPTION_NOT_SUPPORTED = "Warning: extracting AMD IL disassembly is currently not "
"supported for DX12 mode. You can extract DXIL disassembly (for shader model 5.1 and above) using the <stage>-dxil-dis option. "
"See -h for more details.";

// Constants - info messages.
static const char* STR_INFO_TEMPLATE_GPSO_FILE_GENERATED = "Template .gpso file created successfully.";
// Constants other.
static const char* TEMPLATE_GPSO_FILE_CONTENT = "# schemaVersion\n1.0\n\n# InputLayoutNumElements (the number of "
"D3D12_INPUT_ELEMENT_DESC elements in the D3D12_INPUT_LAYOUT_DESC structure - must match the following \"InputLayout\" "
"section)\n2\n\n# InputLayout ( {SemanticName, SemanticIndex, Format, InputSlot, AlignedByteOffset, InputSlotClass, "
"InstanceDataStepRate } )\n{ \"POSITION\", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,"
"0 },\n{ \"COLOR\", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }\n\n#"
"PrimitiveTopologyType (the D3D12_PRIMITIVE_TOPOLOGY_TYPE value to be used when creating the PSO)\n"
"D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE\n\n# NumRenderTargets (the number of formats in the upcoming RTVFormats section)"
"\n1\n\n# RTVFormats (an array of DXGI_FORMAT-typed values for the render target formats - the number of items in the array "
"should match the above NumRenderTargets section)\n{ DXGI_FORMAT_R8G8B8A8_UNORM }";

// Validates the input arguments for a specific stage, and prints appropriate error messages to the console.
static bool IsShaderInputsValid(const Config& config, const std::string& shaderHlsl, const std::string& shaderModel,
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
        if (shaderModel.empty() && config.m_allModel.empty())
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

    if (config.m_ISAFile.empty() && config.m_AnalysisFile.empty() && config.m_BinaryOutputFile.empty())
    {
        std::cout << STR_ERROR_NO_ISA_NO_STATS_OPTION << std::endl;
        ret = false;
    }

    if (ret)
    {
        if ((!config.m_csDxbc.empty() || !config.m_csHlsl.empty()) && ((!config.m_vsDxbc.empty() || !config.m_hsDxbc.empty() || !config.m_dsDxbc.empty()
            || !config.m_gsDxbc.empty() || !config.m_psDxbc.empty()) || (!config.m_vsHlsl.empty() || !config.m_hsHlsl.empty()
                || !config.m_dsHlsl.empty() || !config.m_gsHlsl.empty() || !config.m_psHlsl.empty())))
        {
            ret = false;
            std::cout << STR_ERROR_MIX_COMPUTE_GRAPHICS_INPUT_A << STR_ERROR_MIX_COMPUTE_GRAPHICS_INPUT_SHADER << std::endl;
        }
        else if (!config.m_csDxilDisassembly.empty() && (!config.m_vsDxilDisassembly.empty() || !config.m_hsDxilDisassembly.empty() ||
            !config.m_dsDxilDisassembly.empty() || !config.m_gsDxilDisassembly.empty() || !config.m_psDxilDisassembly.empty()))
        {
            ret = false;
            std::cout << STR_ERROR_MIX_COMPUTE_GRAPHICS_INPUT_A << STR_ERROR_MIX_COMPUTE_GRAPHICS_INPUT_DXIL_DISASSEMBLY << std::endl;
        }
        else if (!config.m_csModel.empty() && (!config.m_vsModel.empty() || !config.m_hsModel.empty() ||
            !config.m_dsModel.empty() || !config.m_gsModel.empty() || !config.m_psModel.empty()))
        {
            ret = false;
            std::cout << STR_ERROR_MIX_COMPUTE_GRAPHICS_INPUT_A << STR_ERROR_MIX_COMPUTE_GRAPHICS_INPUT_SHADER_MODEL << std::endl;
        }
        else if (!config.m_csEntryPoint.empty() && (!config.m_vsEntryPoint.empty() || !config.m_hsEntryPoint.empty() ||
            !config.m_dsEntryPoint.empty() || !config.m_gsEntryPoint.empty() || !config.m_psEntryPoint.empty()))
        {
            ret = false;
            std::cout << STR_ERROR_MIX_COMPUTE_GRAPHICS_INPUT_A << STR_ERROR_MIX_COMPUTE_GRAPHICS_INPUT_ENTRY_POINT << std::endl;
        }
    }

    if (ret)
    {
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
    }

    if (ret)
    {
        if (!config.m_csHlsl.empty())
        {
            // Compute.
            if (!IsShaderInputsValid(config, config.m_csHlsl,
                config.m_csModel, config.m_csEntryPoint, STR_DX12_STAGE_NAMES[bePipelineStage::Compute]))
            {
                ret = false;
            }
        }
        else
        {
            // Vertex.
            if (!IsShaderInputsValid(config, config.m_vsHlsl,
                config.m_vsModel, config.m_vsEntryPoint, STR_DX12_STAGE_NAMES[bePipelineStage::Vertex]))
            {
                ret = false;
            }

            // Hull.
            if (!IsShaderInputsValid(config, config.m_hsHlsl,
                config.m_hsModel, config.m_hsEntryPoint, STR_DX12_STAGE_NAMES[bePipelineStage::TessellationControl]))
            {
                ret = false;
            }

            // Domain.
            if (!IsShaderInputsValid(config, config.m_dsHlsl,
                config.m_dsModel, config.m_dsEntryPoint, STR_DX12_STAGE_NAMES[bePipelineStage::TessellationEvaluation]))
            {
                ret = false;
            }

            // Geometry.
            if (!IsShaderInputsValid(config, config.m_gsHlsl,
                config.m_gsModel, config.m_gsEntryPoint, STR_DX12_STAGE_NAMES[bePipelineStage::Geometry]))
            {
                ret = false;
            }

            // Pixel.
            if (!IsShaderInputsValid(config, config.m_psHlsl,
                config.m_psModel, config.m_psEntryPoint, STR_DX12_STAGE_NAMES[bePipelineStage::Fragment]))
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

    // Verify that we have a valid PSO file.
    if (ret)
    {
        if (config.m_csHlsl.empty() && config.m_csDxbc.empty() &&
            config.m_psoDx12.empty() &&
            (!config.m_vsHlsl.empty() || !config.m_vsDxbc.empty() ||
                !config.m_hsHlsl.empty() || !config.m_hsDxbc.empty() ||
                !config.m_dsHlsl.empty() || !config.m_dsDxbc.empty() ||
                !config.m_gsHlsl.empty() || !config.m_gsDxbc.empty() ||
                !config.m_psHlsl.empty() || !config.m_psDxbc.empty()))
        {
            std::cout << STR_ERROR_NO_GPSO_FILE << std::endl;
            std::cout << STR_ERROR_NO_GPSO_FILE_HINT_A << std::endl << TEMPLATE_GPSO_FILE_CONTENT << std::endl
                << STR_ERROR_NO_GPSO_FILE_HINT_B << std::endl;
        }
    }

    if (ret)
    {
        // For graphics pipelines, if --rs-macro is used, make sure that either --rs-hlsl or --all-hlsl is used
        // so that we know at which HLSL file to search for the macro definition.
        if (!config.m_rsMacro.empty() && config.m_csHlsl.empty() && config.m_allHlsl.empty() && config.m_rsHlsl.empty())
        {
            std::cout << STR_ERROR_NO_ROOT_SIGNATURE_HLSL_FILE_DEFINED << std::endl;
            ret = false;
        }
    }

    return ret;
}

// Update the user provided configuration if necessary.
static void UpdateConfig(const Config& userInput, Config& updatedConfig)
{
    updatedConfig = userInput;
    if (!updatedConfig.m_allHlsl.empty())
    {
        if (!updatedConfig.m_vsEntryPoint.empty() && updatedConfig.m_vsHlsl.empty() && updatedConfig.m_vsDxbc.empty())
        {
            updatedConfig.m_vsHlsl = updatedConfig.m_allHlsl;
        }
        if (!updatedConfig.m_hsEntryPoint.empty() && updatedConfig.m_hsHlsl.empty() && updatedConfig.m_hsDxbc.empty())
        {
            updatedConfig.m_hsHlsl = updatedConfig.m_allHlsl;
        }
        if (!updatedConfig.m_dsEntryPoint.empty() && updatedConfig.m_dsHlsl.empty() && updatedConfig.m_dsDxbc.empty())
        {
            updatedConfig.m_dsHlsl = updatedConfig.m_allHlsl;
        }
        if (!updatedConfig.m_gsEntryPoint.empty() && updatedConfig.m_gsHlsl.empty() && updatedConfig.m_gsDxbc.empty())
        {
            updatedConfig.m_gsHlsl = updatedConfig.m_allHlsl;
        }
        if (!updatedConfig.m_psEntryPoint.empty() && updatedConfig.m_psHlsl.empty() && updatedConfig.m_psDxbc.empty())
        {
            updatedConfig.m_psHlsl = updatedConfig.m_allHlsl;
        }
        if (!updatedConfig.m_csEntryPoint.empty() && updatedConfig.m_csHlsl.empty() && updatedConfig.m_csDxbc.empty())
        {
            updatedConfig.m_csHlsl = updatedConfig.m_allHlsl;
        }
    }

    if (!userInput.m_rsMacro.empty() && userInput.m_csHlsl.empty() && userInput.m_rsHlsl.empty() && userInput.m_allHlsl.empty())
    {
        // If in a graphics pipeline --rs-macro is used without --rs-hlsl, check if all stages point to the same file.
        // If this is the case, just use that file as if it was the input to --rs-hlsl.
        std::vector<std::string> presentStages;
        if (!userInput.m_vsHlsl.empty())
        {
            presentStages.push_back(userInput.m_vsHlsl);
        }
        if (!userInput.m_hsHlsl.empty())
        {
            presentStages.push_back(userInput.m_hsHlsl);
        }
        if (!userInput.m_dsHlsl.empty())
        {
            presentStages.push_back(userInput.m_dsHlsl);
        }
        if (!userInput.m_gsHlsl.empty())
        {
            presentStages.push_back(userInput.m_gsHlsl);
        }
        if (!userInput.m_psHlsl.empty())
        {
            presentStages.push_back(userInput.m_psHlsl);
        }

        // If we have a single HLSL file for all stages - use that file for --rs-hlsl.
        if (!presentStages.empty() && (presentStages.size() == 1 || std::adjacent_find(presentStages.begin(),
            presentStages.end(), std::not_equal_to<>()) == presentStages.end()))
        {
            updatedConfig.m_rsHlsl = presentStages[0];
            std::cout << STR_WARNING_AUTO_DEDUCING_RS_HLSL << updatedConfig.m_rsHlsl << std::endl;
        }
    }
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
    bool shouldAbort = false;

    // Container for all targets.
    std::vector<std::string> targetDevices;

    // Targets that have been covered.
    std::vector<std::string> completedTargets;

    // Input validation - commands.
    if (config.m_ISAFile.empty())
    {
        if (!config.m_LiveRegisterAnalysisFile.empty())
        {
            std::cout << STR_ERR_LIVEREG_WITHOUT_ISA << std::endl;
            shouldAbort = true;
        }
        else if (!config.m_blockCFGFile.empty() ||
            !config.m_instCFGFile.empty())
        {
            std::cout << STR_ERR_CFG_WITHOUT_ISA << std::endl;
            shouldAbort = true;
        }
    }

    if (!shouldAbort)
    {
        if (config.m_psoDx12Template.empty())
        {
            // Update the user provided config if necessary.
            Config configUpdated;
            UpdateConfig(config, configUpdated);

            if (IsInputValid(configUpdated))
            {
                if (configUpdated.m_ASICs.empty())
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
                            beStatus rc = m_dx12Backend.Compile(configUpdated, target, outText, errorMsg, isaFiles, statsFiles);
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
                                                    isOk = kcUtils::ConstructOutFileName(configUpdated.m_LiveRegisterAnalysisFile, STR_DX12_STAGE_NAMES[stage],
                                                        target, KC_STR_DEFAULT_LIVEREG_EXT, outputFileName);

                                                    if (isOk)
                                                    {
                                                        // Delete that file if it already exists.
                                                        if (beUtils::IsFilePresent(outputFileName))
                                                        {
                                                            kcUtils::DeleteFile(outputFileName.c_str());
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
                                                                kcUtils::DeleteFile(outputFileName);
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

                                        if (!configUpdated.m_blockCFGFile.empty())
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
                                                    isOk = kcUtils::ConstructOutFileName(configUpdated.m_blockCFGFile, STR_DX12_STAGE_NAMES[stage],
                                                        target, KC_STR_DEFAULT_CFG_EXT, outputFileName);

                                                    if (isOk)
                                                    {
                                                        // Delete that file if it already exists.
                                                        if (beUtils::IsFilePresent(outputFileName))
                                                        {
                                                            beUtils::DeleteFileFromDisk(outputFileName);
                                                        }

                                                        isOk = kcUtils::GenerateControlFlowGraph(isaFiles[stage], outputFileName, NULL,
                                                            false, configUpdated.m_printProcessCmdLines);
                                                        if (isOk)
                                                        {
                                                            if (kcUtils::FileNotEmpty(outputFileName))
                                                            {
                                                                std::cout << KA_CLI_STR_STATUS_SUCCESS << std::endl;
                                                            }
                                                            else
                                                            {
                                                                std::cout << KA_CLI_STR_STATUS_FAILURE << std::endl;
                                                                kcUtils::DeleteFile(outputFileName);
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

                                        if (!configUpdated.m_instCFGFile.empty())
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
                                                    isOk = kcUtils::ConstructOutFileName(configUpdated.m_instCFGFile, STR_DX12_STAGE_NAMES[stage],
                                                        target, KC_STR_DEFAULT_CFG_EXT, outputFileName);

                                                    if (isOk)
                                                    {
                                                        // Delete that file if it already exists.
                                                        if (beUtils::IsFilePresent(outputFileName))
                                                        {
                                                            beUtils::DeleteFileFromDisk(outputFileName);
                                                        }

                                                        isOk = kcUtils::GenerateControlFlowGraph(isaFiles[stage], outputFileName, NULL,
                                                            true, configUpdated.m_printProcessCmdLines);
                                                        if (isOk)
                                                        {
                                                            if (kcUtils::FileNotEmpty(outputFileName))
                                                            {
                                                                std::cout << KA_CLI_STR_STATUS_SUCCESS << std::endl;
                                                            }
                                                            else
                                                            {
                                                                std::cout << KA_CLI_STR_STATUS_FAILURE << std::endl;
                                                                kcUtils::DeleteFile(outputFileName);
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
        else
        {
            bool isFileWritten = kcUtils::WriteTextFile(config.m_psoDx12Template, TEMPLATE_GPSO_FILE_CONTENT, nullptr);
            assert(isFileWritten);
            if (isFileWritten)
            {
                std::cout << STR_INFO_TEMPLATE_GPSO_FILE_GENERATED << std::endl;
            }
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

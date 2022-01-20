//======================================================================
// Copyright 2019-2021 Advanced Micro Devices, Inc. All rights reserved.
//======================================================================

#ifdef _WIN32

// C++.
#include <cassert>

// Infra.
#include "external/amdt_base_tools/Include/gtString.h"
#include "external/amdt_base_tools/Include/gtList.h"
#include "external/amdt_os_wrappers/Include/osDirectory.h"
#include "external/amdt_os_wrappers/Include/osFilePath.h"

// Backend.
#include "radeon_gpu_analyzer_backend/be_utils.h"
#include "radeon_gpu_analyzer_backend/be_data_types.h"

// Local.
#include "radeon_gpu_analyzer_cli/kc_cli_commander_dx12.h"
#include "radeon_gpu_analyzer_cli/kc_cli_string_constants.h"

// Device info.
#include "DeviceInfoUtils.h"

using namespace rga;

// *****************************************
// *** INTERNALLY LINKED SYMBOLS - START ***
// *****************************************

// Constants - error messages.
static const char* kStrErrorDx12NoTargetProvided = "Error: no supported target device provided.";
static const char* kStrErrorDx12TwoInputFilesPerStageA = "Error: both HLSL and DXBC input files specified for ";
static const char* kStrErrorDx12TwoInputFilesPerStageB = "shader. Only a single input file can be passed per stage.";
static const char* kStrErrorDx12NoShaderModelProvidedA = "Error: no shader model provided for ";
static const char* kStrErrorDx12NoShaderModelProvidedB = " shader.";
static const char* kStrErrorDx12NoEntryPointProvidedA = "Error: no entry point provided for ";
static const char* kStrErrorDx12NoEntryPointProvidedB = " shader.";
static const char* kStrErrorDx12ShaderNotFoundOrEmptyA = "Error: ";
static const char* kStrErrorDx12ShaderNotFoundOrEmptyB = " shader does not exist or file is empty: ";
static const char* kStrErrorDx12IsaNotGeneratedA = "Error: failed to generate ISA disassembly for ";
static const char* kStrErrorDx12AmdilNotGeneratedA = "Error: failed to generate AMDIL disassembly for ";
static const char* kStrErrorDx12OutputNotGeneratedB = " shader";
static const char* kStrErrorDx12CannotGenerateElfDisassemblyWithoutPipelineBinary = "Error: cannot generate ELF disassembly if pipeline binary is not "
"generated (must use -b option together with --elf-dis option).";
static const char* kStrErrorDx12StatsNotGeneratedA = "Error: failed to generate resource usage statistics for ";
static const char* kStrErrorDx12StatsNotGeneratedB = " shader";
static const char* kStrErrorDx12BinaryNotGeneratedA = "Error: failed to extract pipeline binary for ";
static const char* kStrErrorNoGpsoFile = "Error: .gpso file must be provided for graphics pipelines (use the --pso option).";
static const char* kStrErrorNoGpsoFileHintA = "The format of a .gpso file is : ";
static const char* kStrErrorNoGpsoFileHintB = "See --gpso-template option for more info.";
static const char* kStrErrorMixComputeGraphicsInputA = "Error: cannot mix compute and graphics for ";
static const char* kStrErrorMixComputeGraphicsInputShader = "shader input files.";
static const char* kStrErrorMixComputeGraphicsInputDxilDisassembly = "DXIL/DXBC disassembly output file.";
static const char* kStrErrorMixComputeGraphicsInputShaderModel = "shader model.";
static const char* kStrErrorMixComputeGraphicsInputEntryPoint = "entry point.";
static const char* kStrErrorNoIsaNoStatsOption = "Error: one of --isa or -a/--analysis or -b/--binary options must be provided.";
static const char* kStrErrorFailedToConstructLiveregOutputFilename = "Error: failed to construct live register analysis output file name.";
static const char* kStrErrorFailedToConstructCfgOutputFilename = "Error: failed to construct control-flow graph output file name.";
static const char* kStrErrorInvalidDxcOptionArgument = "Error: argument to --dxc option should be path to the folder where DXC is located, not a full path to a file.";

// DXR-specific error messages.
static const char* kStrErrorDxrIsaNotGeneratedB = " export.";
static const char* kStrErrorDxrIsaNotGeneratedBPipeline = " pipeline.";
static const char* kStrErrorDxrHlslInputFileMissing = "Error: no HLSL input source file provided (use --hlsl option).";
static const char* kStrErrorDxrStateDescAndHlslFileMix = "Error: cannot mix --state-desc and --hlsl options.If --hlsl option is used, it must contain the entire state definition in the HLSL source code.";
static const char* kStrErrorDxrStateDescFileNotFoundOrEmpty = "Error: DXR state JSON file not found or empty: ";
static const char* kStrErrorDxrHlslFileNotFoundOrEmpty = "Error: HLSL input file not found or empty: ";
static const char* kStrErrorDxrModeNotProvided = "Error: no DXR mode provided (use --mode with either 'pipeline' or 'shader' as the argument).";
static const char* kStrErrorDxrModeNotRecognizedA = "Error: unrecognized DXR mode given: ";
static const char* kStrErrorDxrModeNotRecognizedB = ". Expected: pipeline or shader.";
static const char* kStrErrorDxrExportNotProvided = "Error: DXR export not provided (use --export option).";
static const char* kStrErrorDxrRootSignatureHlslFileDefined = "Error: use --rs-hlsl option together with --rs-macro to specify the HLSL file where the macro is defined.";
static const char* kStrErrorDxrNoSupportedTargetsFound = "Error: non of the targets which are supported by the driver is gfx1030 or beyond. Aborting.";
static const char* kStrErrorDxrExportNotSupportedInPipelineMode = "Error: --export option not supported in pipeline mode.";
static const char* kStrErrorOfflineTargetDeviceMustBeSpecifed   = "Error: target device must be specified in offline mode (use -c option).";

// Constants - warnings messages.
static const char* kStrWarningDx12AutoDeducingRootSignatureAsHlsl = "Warning: --rs-hlsl option not provided, assuming that root signature macro is defined in ";

// DXR-specific warning messages.
static const char* kStrWarningDxrBinaryExtractionNotSupportedInShaderMode = "Warning: pipeline binary extraction (-b option) "
"is not supported in Shader mode.";
static const char* kStrWarningDxrSkippingUnsupportedTarget = "Warning: DXR mode only supports gfx1030 and beyond as a target. Skipping ";

// Constants - info messages.
static const char* kStrInfoTemplateGpsoFileGenerated = "Template .gpso file created successfully.";
static const char* kStrInfoDx12PostProcessingSeparator = "-=-=-=-=-=-=-";
static const char* kStrInfoDx12PostProcessing = "Post-processing...";
static const char* kStrInfoDxrPerformingPostProcessing = "...";
static const char* kStrInfoDxrAssumingShaderMode = "Info: no --mode argument detected, assuming '--mode shader' by default.";
static const char* kStrInfoDxrOutputPipelineNumber = "pipeline #";
static const char* kStrInfoDxrOutputPipelineName = "pipeline associated with raygeneration shader ";
static const char* kStrInfoDxrOutputShader = "shader ";
static const char* kStrInfoDxrUsingDefaultShaderModel = "Info: using user-provided shader model instead of the default model (";

// Constants other.
static const char* kStrTemplateGpsoFileContent = "# schemaVersion\n1.0\n\n# InputLayoutNumElements (the number of "
"D3D12_INPUT_ELEMENT_DESC elements in the D3D12_INPUT_LAYOUT_DESC structure - must match the following \"InputLayout\" "
"section)\n2\n\n# InputLayout ( {SemanticName, SemanticIndex, Format, InputSlot, AlignedByteOffset, InputSlotClass, "
"InstanceDataStepRate } )\n{ \"POSITION\", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,"
"0 },\n{ \"COLOR\", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }\n\n#"
"PrimitiveTopologyType (the D3D12_PRIMITIVE_TOPOLOGY_TYPE value to be used when creating the PSO)\n"
"D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE\n\n# NumRenderTargets (the number of formats in the upcoming RTVFormats section)"
"\n1\n\n# RTVFormats (an array of DXGI_FORMAT-typed values for the render target formats - the number of items in the array "
"should match the above NumRenderTargets section)\n{ DXGI_FORMAT_R8G8B8A8_UNORM }";

// Constants - other.
static const char* kStrDxrModePipeline = "pipeline";
static const char* kStrDxrModeShader = "shader";
static const char* kStrDxrUnifiedSuffix = "_unified";
const char kStrFileNmaeTokenIndirect = '*';
const char* kStrDxrNullPipelineName = "null";
const char* kStrDefaultDxrShaderModel = "lib_6_3";

// Validates the input arguments for a specific stage, and prints appropriate error messages to the console.
static bool IsShaderInputsValid(const Config& config, const std::string& shader_hlsl, const std::string& shader_model,
    const std::string& shader_entry_point, const std::string& stage_name)
{
    bool ret = true;
    if (!shader_hlsl.empty())
    {
        if (!KcUtils::FileNotEmpty(shader_hlsl))
        {
            std::cout << kStrErrorDx12ShaderNotFoundOrEmptyA <<
                stage_name << kStrErrorDx12ShaderNotFoundOrEmptyB << shader_hlsl << std::endl;
            ret = false;
        }

        // Validate inputs for shader front-end compilation.
        if (shader_model.empty() && config.all_model.empty())
        {
            std::cout << kStrErrorDx12NoShaderModelProvidedA << stage_name <<
                kStrErrorDx12NoShaderModelProvidedB << std::endl;
            ret = false;
        }
        else if (shader_entry_point.empty())
        {
            std::cout << kStrErrorDx12NoEntryPointProvidedA << stage_name <<
                kStrErrorDx12NoEntryPointProvidedB << std::endl;
            ret = false;
        }
    }
    return ret;
}

// Validates that a DXBC input file is valid.
static bool IsDxbcInputValid(const std::string& dxbc_input_file, const std::string& stage)
{
    bool ret = true;
    if (!dxbc_input_file.empty() && !KcUtils::FileNotEmpty(dxbc_input_file))
    {
        std::cout << kStrErrorDx12ShaderNotFoundOrEmptyA <<
            stage << kStrErrorDx12ShaderNotFoundOrEmptyB << dxbc_input_file << std::endl;
        ret = false;
    }
    return ret;
}

// Returns true if this is DXR Shader mode and false otherwise.
static bool IsDxrShaderMode(const Config& config)
{
    std::string dxr_mode = KcUtils::ToLower(config.dxr_mode);
    return (dxr_mode.compare(kStrDxrModeShader) == 0);
}

// Validates the input and prints appropriate error messages to the console - for DXR sessions.
static bool IsInputValidDxr(const Config& config)
{
    bool ret = true;
    if (config.dxr_hlsl.empty() && config.dxr_state_desc.empty())
    {
        // HLSL file not provided.
        std::cout << kStrErrorDxrHlslInputFileMissing << std::endl;
        ret = false;
    }
    else if (!config.dxr_state_desc.empty() && !config.dxr_hlsl.empty())
    {
        // Mix of state description file and HLSL input.
        std::cout << kStrErrorDxrStateDescAndHlslFileMix << std::endl;
        ret = false;
    }
    else if (!config.dxr_state_desc.empty() && !KcUtils::FileNotEmpty(config.dxr_state_desc))
    {
        // Empty state description file.
        std::cout << kStrErrorDxrStateDescFileNotFoundOrEmpty << config.dxr_state_desc << std::endl;
        ret = false;
    }
    else if (!config.dxr_hlsl.empty() && !KcUtils::FileNotEmpty(config.dxr_hlsl))
    {
        // Empty HLSL input file.
        std::cout << kStrErrorDxrHlslFileNotFoundOrEmpty << config.dxr_hlsl << std::endl;
        ret = false;
    }
    else if (config.dxr_mode.empty())
    {
        // Mode not provided.
        std::cout << kStrErrorDxrModeNotProvided << std::endl;
        ret = false;
    }
    else if (IsDxrShaderMode(config) && config.dxr_exports.empty())
    {
        // Export not provided in Shader mode.
        std::cout << kStrErrorDxrExportNotProvided << std::endl;
        ret = false;
    }
    else if (!IsDxrShaderMode(config) && !config.dxr_exports.empty())
    {
        // Export not supported in Pipeline mode.
        std::cout << kStrErrorDxrExportNotSupportedInPipelineMode << std::endl;
        ret = false;
    }
    else
    {
        std::string dxr_mode = KcUtils::ToLower(config.dxr_mode);;
        if (dxr_mode.compare(kStrDxrModePipeline) != 0 && dxr_mode.compare(kStrDxrModeShader) != 0)
        {
            // Unrecognized mode.
            std::cout << kStrErrorDxrModeNotRecognizedA <<
                dxr_mode << kStrErrorDxrModeNotRecognizedB << std::endl;
            ret = false;
        }
        if (ret && dxr_mode.compare(kStrDxrModeShader) == 0 && !config.dxr_exports.empty() &&
            std::find_if(config.dxr_exports.begin(), config.dxr_exports.end(), [&](const std::string currExp)
            {
                std::string curr_export_lower = KcUtils::ToLower(currExp);
                return (curr_export_lower.compare("all") == 0);
            }) != config.dxr_exports.end())
        {
            std::cout << "Error: 'all' is not a valid argument for --export in Shader mode (only valid in Pipeline mode). Please specify a shader name." << std::endl;
            ret = false;
        }
        if (ret && dxr_mode.compare(kStrDxrModeShader) == 0 && !config.binary_output_file.empty())
        {
            std::cout << kStrWarningDxrBinaryExtractionNotSupportedInShaderMode << std::endl;
        }
    }

    // In Offline mode, the target device must be specified.
    if ((config.dx12_offline_session || !config.alternative_amdxc.empty()) && config.asics.empty())
    {
        std::cout << kStrErrorOfflineTargetDeviceMustBeSpecifed << std::endl;
        ret = false;
    }

    return ret;
}

// Validates the input and prints appropriate error messages to the console.
static bool IsInputValid(const Config& config)
{
    bool ret = true;
    if (config.isa_file.empty() && config.analysis_file.empty() && config.binary_output_file.empty())
    {
        std::cout << kStrErrorNoIsaNoStatsOption << std::endl;
        ret = false;
    }

    if (ret)
    {
        if ((!config.cs_dxbc.empty() || !config.cs_hlsl.empty()) && ((!config.vs_dxbc.empty() || !config.hs_dxbc.empty() || !config.ds_dxbc.empty()
            || !config.gs_dxbc.empty() || !config.ps_dxbc.empty()) || (!config.vs_hlsl.empty() || !config.hs_hlsl.empty()
                || !config.ds_hlsl.empty() || !config.gs_hlsl.empty() || !config.ps_hlsl.empty())))
        {
            ret = false;
            std::cout << kStrErrorMixComputeGraphicsInputA << kStrErrorMixComputeGraphicsInputShader << std::endl;
        }
        else if (!config.cs_dxil_disassembly.empty() && (!config.vs_dxil_disassembly.empty() || !config.hs_dxil_disassembly.empty() ||
            !config.ds_dxil_disassembly.empty() || !config.gs_dxil_disassembly.empty() || !config.ps_dxil_disassembly.empty()))
        {
            ret = false;
            std::cout << kStrErrorMixComputeGraphicsInputA << kStrErrorMixComputeGraphicsInputDxilDisassembly << std::endl;
        }
        else if (!config.cs_model.empty() && (!config.vs_model.empty() || !config.hs_model.empty() ||
            !config.ds_model.empty() || !config.gs_model.empty() || !config.ps_model.empty()))
        {
            ret = false;
            std::cout << kStrErrorMixComputeGraphicsInputA << kStrErrorMixComputeGraphicsInputShaderModel << std::endl;
        }
        else if (!config.cs_entry_point.empty() && (!config.vs_entry_point.empty() || !config.hs_entry_point.empty() ||
            !config.ds_entry_point.empty() || !config.gs_entry_point.empty() || !config.ps_entry_point.empty()))
        {
            ret = false;
            std::cout << kStrErrorMixComputeGraphicsInputA << kStrErrorMixComputeGraphicsInputEntryPoint << std::endl;
        }
    }

    if (ret)
    {
        if (!config.cs_hlsl.empty() && !config.cs_dxbc.empty())
        {
            std::cout << kStrErrorDx12TwoInputFilesPerStageA << "compute " <<
                kStrErrorDx12TwoInputFilesPerStageB << std::endl;
            ret = false;
        }
        else if (!config.vs_hlsl.empty() && !config.vs_dxbc.empty())
        {
            std::cout << kStrErrorDx12TwoInputFilesPerStageA << "vertex " <<
                kStrErrorDx12TwoInputFilesPerStageB << std::endl;
            ret = false;
        }
        else if (!config.hs_hlsl.empty() && !config.hs_dxbc.empty())
        {
            std::cout << kStrErrorDx12TwoInputFilesPerStageA << "hull " <<
                kStrErrorDx12TwoInputFilesPerStageB << std::endl;
            ret = false;
        }
        else if (!config.ds_hlsl.empty() && !config.ds_dxbc.empty())
        {
            std::cout << kStrErrorDx12TwoInputFilesPerStageA << "domain " <<
                kStrErrorDx12TwoInputFilesPerStageB << std::endl;
            ret = false;
        }
        else if (!config.gs_hlsl.empty() && !config.gs_dxbc.empty())
        {
            std::cout << kStrErrorDx12TwoInputFilesPerStageA << "geometry " <<
                kStrErrorDx12TwoInputFilesPerStageB << std::endl;
            ret = false;
        }
        else if (!config.ps_hlsl.empty() && !config.ps_dxbc.empty())
        {
            std::cout << kStrErrorDx12TwoInputFilesPerStageA << "pixel " <<
                kStrErrorDx12TwoInputFilesPerStageB << std::endl;
            ret = false;
        }
    }

    if (ret)
    {
        if (!config.cs_hlsl.empty())
        {
            // Compute.
            if (!IsShaderInputsValid(config, config.cs_hlsl,
                config.cs_model, config.cs_entry_point, kStrDx12StageNames[BePipelineStage::kCompute]))
            {
                ret = false;
            }
        }
        else
        {
            // Vertex.
            if (!IsShaderInputsValid(config, config.vs_hlsl,
                config.vs_model, config.vs_entry_point, kStrDx12StageNames[BePipelineStage::kVertex]))
            {
                ret = false;
            }

            // Hull.
            if (!IsShaderInputsValid(config, config.hs_hlsl,
                config.hs_model, config.hs_entry_point, kStrDx12StageNames[BePipelineStage::kTessellationControl]))
            {
                ret = false;
            }

            // Domain.
            if (!IsShaderInputsValid(config, config.ds_hlsl,
                config.ds_model, config.ds_entry_point, kStrDx12StageNames[BePipelineStage::kTessellationEvaluation]))
            {
                ret = false;
            }

            // Geometry.
            if (!IsShaderInputsValid(config, config.gs_hlsl,
                config.gs_model, config.gs_entry_point, kStrDx12StageNames[BePipelineStage::kGeometry]))
            {
                ret = false;
            }

            // Pixel.
            if (!IsShaderInputsValid(config, config.ps_hlsl,
                config.ps_model, config.ps_entry_point, kStrDx12StageNames[BePipelineStage::kFragment]))
            {
                ret = false;
            }
        }
    }

    // Verify DXBC input files, if relevant.
    if (ret)
    {
        if (!IsDxbcInputValid(config.vs_dxbc, kStrDx12StageNames[BePipelineStage::kVertex]))
        {
            ret = false;
        }
        if (!IsDxbcInputValid(config.hs_dxbc, kStrDx12StageNames[BePipelineStage::kTessellationControl]))
        {
            ret = false;
        }
        if (!IsDxbcInputValid(config.ds_dxbc, kStrDx12StageNames[BePipelineStage::kTessellationEvaluation]))
        {
            ret = false;
        }
        if (!IsDxbcInputValid(config.gs_dxbc, kStrDx12StageNames[BePipelineStage::kGeometry]))
        {
            ret = false;
        }
        if (!IsDxbcInputValid(config.ps_dxbc, kStrDx12StageNames[BePipelineStage::kFragment]))
        {
            ret = false;
        }
        if (!IsDxbcInputValid(config.cs_dxbc, kStrDx12StageNames[BePipelineStage::kCompute]))
        {
            ret = false;
        }
    }

    // Verify that we have a valid PSO file.
    if (ret)
    {
        if (config.cs_hlsl.empty() && config.cs_dxbc.empty() &&
            config.pso_dx12.empty() &&
            (!config.vs_hlsl.empty() || !config.vs_dxbc.empty() ||
                !config.hs_hlsl.empty() || !config.hs_dxbc.empty() ||
                !config.ds_hlsl.empty() || !config.ds_dxbc.empty() ||
                !config.gs_hlsl.empty() || !config.gs_dxbc.empty() ||
                !config.ps_hlsl.empty() || !config.ps_dxbc.empty()))
        {
            std::cout << kStrErrorNoGpsoFile << std::endl;
            std::cout << kStrErrorNoGpsoFileHintA << std::endl << kStrTemplateGpsoFileContent << std::endl
                << kStrErrorNoGpsoFileHintB << std::endl;
        }
    }

    if (ret)
    {
        // For graphics pipelines, if --rs-macro is used, make sure that either --rs-hlsl or --all-hlsl is used
        // so that we know at which HLSL file to search for the macro definition.
        if (!config.rs_macro.empty() && config.cs_hlsl.empty() && config.all_hlsl.empty() && config.rs_hlsl.empty())
        {
            std::cout << kStrErrorDxrRootSignatureHlslFileDefined << std::endl;
            ret = false;
        }
    }

    if (ret)
    {
        // Cannot generate ELF disassembly if pipeline binary is not generated.
        if (!config.elf_dis.empty() && config.binary_output_file.empty())
        {
            std::cout << kStrErrorDx12CannotGenerateElfDisassemblyWithoutPipelineBinary << std::endl;
            ret = false;
        }
    }

    // In Offline mode, the target device must be specified.
    if ((config.dx12_offline_session || !config.alternative_amdxc.empty()) && config.asics.empty())
    {
        std::cout << kStrErrorOfflineTargetDeviceMustBeSpecifed << std::endl;
        ret = false;
    }

    return ret;
}

// Update the user provided configuration if necessary.
static void UpdateConfig(const Config& user_input, Config& updated_config)
{
    updated_config = user_input;
    bool is_dxr = (user_input.mode == RgaMode::kModeDxr);
    if (!is_dxr)
    {
        if (!updated_config.all_hlsl.empty())
        {
            if (!updated_config.vs_entry_point.empty() && updated_config.vs_hlsl.empty() && updated_config.vs_dxbc.empty())
            {
                updated_config.vs_hlsl = updated_config.all_hlsl;
            }
            if (!updated_config.hs_entry_point.empty() && updated_config.hs_hlsl.empty() && updated_config.hs_dxbc.empty())
            {
                updated_config.hs_hlsl = updated_config.all_hlsl;
            }
            if (!updated_config.ds_entry_point.empty() && updated_config.ds_hlsl.empty() && updated_config.ds_dxbc.empty())
            {
                updated_config.ds_hlsl = updated_config.all_hlsl;
            }
            if (!updated_config.gs_entry_point.empty() && updated_config.gs_hlsl.empty() && updated_config.gs_dxbc.empty())
            {
                updated_config.gs_hlsl = updated_config.all_hlsl;
            }
            if (!updated_config.ps_entry_point.empty() && updated_config.ps_hlsl.empty() && updated_config.ps_dxbc.empty())
            {
                updated_config.ps_hlsl = updated_config.all_hlsl;
            }
            if (!updated_config.cs_entry_point.empty() && updated_config.cs_hlsl.empty() && updated_config.cs_dxbc.empty())
            {
                updated_config.cs_hlsl = updated_config.all_hlsl;
            }
        }

        if (!user_input.rs_macro.empty() && user_input.cs_hlsl.empty() && user_input.rs_hlsl.empty() && user_input.all_hlsl.empty())
        {
            // If in a graphics pipeline --rs-macro is used without --rs-hlsl, check if all stages point to the same file.
            // If this is the case, just use that file as if it was the input to --rs-hlsl.
            std::vector<std::string> present_stages;
            if (!user_input.vs_hlsl.empty())
            {
                present_stages.push_back(user_input.vs_hlsl);
            }
            if (!user_input.hs_hlsl.empty())
            {
                present_stages.push_back(user_input.hs_hlsl);
            }
            if (!user_input.ds_hlsl.empty())
            {
                present_stages.push_back(user_input.ds_hlsl);
            }
            if (!user_input.gs_hlsl.empty())
            {
                present_stages.push_back(user_input.gs_hlsl);
            }
            if (!user_input.ps_hlsl.empty())
            {
                present_stages.push_back(user_input.ps_hlsl);
            }

            // If we have a single HLSL file for all stages - use that file for --rs-hlsl.
            if (!present_stages.empty() &&
                (present_stages.size() == 1 || std::adjacent_find(present_stages.begin(), present_stages.end(), std::not_equal_to<>()) == present_stages.end()))
            {
                updated_config.rs_hlsl = present_stages[0];
                std::cout << kStrWarningDx12AutoDeducingRootSignatureAsHlsl << updated_config.rs_hlsl << std::endl;
            }
        }
    }
    else
    {
        // Use shader mode by default.
        if (user_input.dxr_mode.compare(kStrDxrModeShader) != 0 &&
            user_input.dxr_mode.compare(kStrDxrModePipeline) != 0)
        {
            std::cout << kStrInfoDxrAssumingShaderMode << std::endl;
            updated_config.dxr_mode = kStrDxrModeShader;
        }

        if (user_input.dxr_shader_model.empty())
        {
            // Use the default shader model unless specified otherwise by the user.
            updated_config.dxr_shader_model = kStrDefaultDxrShaderModel;
        }
        else
        {
            std::cout << kStrInfoDxrUsingDefaultShaderModel <<
                kStrDefaultDxrShaderModel << "): " << user_input.dxr_shader_model << std::endl;
        }
    }
}

// Accepts a DXR pipeline name as reported in the DXR output metadata file and
// returns true if this is a "NULL" pipeline (empty pipeline generated as a container in Shader mode).
static bool IsDxrNullPipeline(const std::string& pipeline_name)
{
    return (pipeline_name.empty() || pipeline_name.compare(kStrDxrNullPipelineName) == 0);
}

static bool PerformLiveRegisterAnalysisDxr(const Config& config_updated, const std::string& isa_file, const std::string& target,
    const std::string& output_filename, const std::string& stage_name, const RgDxrShaderResults& shader_results)
{
    bool is_ok = false;
    if (!isa_file.empty())
    {
        std::cout << kStrInfoPerformingLiveregAnalysis1;
        std::cout << kStrInfoDxrOutputShader << shader_results.export_name << kStrInfoDxrPerformingPostProcessing << std::endl;

        // Delete the file if it already exists.
        if (BeUtils::IsFilePresent(output_filename))
        {
            KcUtils::DeleteFile(output_filename.c_str());
        }

        is_ok = KcUtils::PerformLiveRegisterAnalysis(isa_file, target, output_filename, NULL,
            config_updated.print_process_cmd_line);

        if (is_ok)
        {
            if (KcUtils::FileNotEmpty(output_filename))
            {
                std::cout << kStrInfoSuccess << std::endl;
            }
            else
            {
                std::cout << kStrInfoFailed << std::endl;
                KcUtils::DeleteFile(output_filename);
            }
        }
    }

    return is_ok;
}

static void PerformLiveRegisterAnalysis(const std::string& isa_file,
    const std::string& stage_name,
    const std::string& target,
    const Config& config_updated,
    bool& is_ok)
{
    if (!isa_file.empty())
    {
        std::cout << kStrInfoPerformingLiveregAnalysis1;
        std::cout << stage_name << kStrInfoPerformingAnalysis2 << std::endl;

        // Construct a name for the output file.
        std::string output_filename;
        std::string file_suffix = stage_name;

        is_ok = KcUtils::ConstructOutFileName(config_updated.livereg_analysis_file, file_suffix,
                                              target,
                                              kStrDefaultExtensionLivereg,
                                              output_filename,
                                              !KcUtils::IsDirectory(config_updated.livereg_analysis_file));

        if (is_ok)
        {
            // Delete that file if it already exists.
            if (BeUtils::IsFilePresent(output_filename))
            {
                KcUtils::DeleteFile(output_filename.c_str());
            }

            is_ok = KcUtils::PerformLiveRegisterAnalysis(isa_file, target, output_filename, NULL,
                config_updated.print_process_cmd_line);
            if (is_ok)
            {
                if (KcUtils::FileNotEmpty(output_filename))
                {
                    std::cout << kStrInfoSuccess << std::endl;
                }
                else
                {
                    std::cout << kStrInfoFailed << std::endl;
                    KcUtils::DeleteFile(output_filename);
                }
            }
        }
        else
        {
            std::cout << kStrErrorFailedToConstructLiveregOutputFilename << std::endl;
        }
    }
}

static bool GeneratePerBlockCfgDxr(const Config& config_updated, const std::string& isa_file, const std::string& target,
    const std::string& output_filename, const std::string& stage_name, const RgDxrShaderResults& shader_results)
{
    bool is_ok = false;
    if (!isa_file.empty())
    {
        std::cout << kStrInfoContructingPerBlockCfg1;
        std::cout << kStrInfoDxrOutputShader << shader_results.export_name << kStrInfoDxrPerformingPostProcessing << std::endl;

        // Delete the file if it already exists.
        if (BeUtils::IsFilePresent(output_filename))
        {
            KcUtils::DeleteFile(output_filename.c_str());
        }

        is_ok = KcUtils::GenerateControlFlowGraph(isa_file, target, output_filename, NULL,
            false, config_updated.print_process_cmd_line);

        if (is_ok)
        {
            if (KcUtils::FileNotEmpty(output_filename))
            {
                std::cout << kStrInfoSuccess << std::endl;
            }
            else
            {
                std::cout << kStrInfoFailed << std::endl;
                KcUtils::DeleteFile(output_filename);
            }
        }
    }

    return is_ok;
}

static void GeneratePerBlockCfg(const std::string& isa_file, const std::string& pipeline_name_dxr, const std::string& stage_name, const std::string& target,
    const Config& config_updated, bool is_dxr, bool& is_ok)
{
    if (!isa_file.empty())
    {
        std::cout << kStrInfoContructingPerBlockCfg1;
        if (is_dxr)
        {
            std::cout << kStrInfoDxrOutputShader << stage_name << kStrInfoDxrPerformingPostProcessing << std::endl;
        }
        else
        {
            std::cout << stage_name << kStrInfoContructingPerBlockCfg2 << std::endl;
        }

        // Construct a name for the output file.
        std::string output_filename;
        std::string file_suffix = stage_name;
        if (is_dxr && !IsDxrNullPipeline(pipeline_name_dxr))
        {
            std::stringstream suffix_updated;
            suffix_updated << pipeline_name_dxr << "_" << stage_name;
            file_suffix = suffix_updated.str();
        }

        is_ok = KcUtils::ConstructOutFileName(config_updated.block_cfg_file, file_suffix,
            target, kStrDefaultExtensionDot, output_filename, !KcUtils::IsDirectory(config_updated.block_cfg_file));

        if (is_ok)
        {
            // Delete that file if it already exists.
            if (BeUtils::IsFilePresent(output_filename))
            {
                BeUtils::DeleteFileFromDisk(output_filename);
            }

            is_ok = KcUtils::GenerateControlFlowGraph(isa_file, target, output_filename, NULL,
                false, config_updated.print_process_cmd_line);
            if (is_ok)
            {
                if (KcUtils::FileNotEmpty(output_filename))
                {
                    std::cout << kStrInfoSuccess << std::endl;
                }
                else
                {
                    std::cout << kStrInfoFailed << std::endl;
                    KcUtils::DeleteFile(output_filename);
                }
            }
        }
        else
        {
            std::cout << kStrErrorFailedToConstructCfgOutputFilename << std::endl;
        }
    }
}

static bool GeneratePerInstructionCfgDxr(const Config& config_updated, const std::string& isa_file, const std::string& target,
    const std::string& output_filename, const std::string& stage_name, const RgDxrShaderResults& shader_results)
{
    bool is_ok = false;
    if (!isa_file.empty())
    {
        std::cout << kStrInfoContructingPerInstructionCfg1;
        std::cout << kStrInfoDxrOutputShader << shader_results.export_name << kStrInfoDxrPerformingPostProcessing << std::endl;

        // Delete the file if it already exists.
        if (BeUtils::IsFilePresent(output_filename))
        {
            KcUtils::DeleteFile(output_filename.c_str());
        }

        is_ok = KcUtils::GenerateControlFlowGraph(isa_file, target, output_filename, NULL,
            true, config_updated.print_process_cmd_line);

        if (is_ok)
        {
            if (KcUtils::FileNotEmpty(output_filename))
            {
                std::cout << kStrInfoSuccess << std::endl;
            }
            else
            {
                std::cout << kStrInfoFailed << std::endl;
                KcUtils::DeleteFile(output_filename);
            }
        }
    }

    return is_ok;
}

static void GeneratePerInstructionCfg(const std::string& isa_file, const std::string& pipeline_name_dxr, const std::string& stage_name, const std::string& target,
    const Config& config_updated, bool is_dxr, bool& is_ok)
{
    if (!isa_file.empty())
    {
        std::cout << kStrInfoContructingPerInstructionCfg1;
        if (is_dxr && !pipeline_name_dxr.empty())
        {
            std::cout << kStrInfoDxrOutputShader << stage_name << kStrInfoDxrPerformingPostProcessing << std::endl;
        }
        else
        {
            std::cout << stage_name << kStrInfoContructingPerInstructionCfg2 << std::endl;
        }

        // Construct a name for the output file.
        std::string output_filename;
        std::string file_suffix = stage_name;
        if (is_dxr && !IsDxrNullPipeline(pipeline_name_dxr))
        {
            std::stringstream suffix_updated;
            suffix_updated << pipeline_name_dxr << "_" << stage_name;
            file_suffix = suffix_updated.str();
        }
        is_ok = KcUtils::ConstructOutFileName(config_updated.inst_cfg_file, file_suffix,
            target, kStrDefaultExtensionDot, output_filename, !KcUtils::IsDirectory(config_updated.inst_cfg_file));

        if (is_ok)
        {
            // Delete that file if it already exists.
            if (BeUtils::IsFilePresent(output_filename))
            {
                BeUtils::DeleteFileFromDisk(output_filename);
            }

            is_ok = KcUtils::GenerateControlFlowGraph(isa_file, target, output_filename, NULL,
                true, config_updated.print_process_cmd_line);
            if (is_ok)
            {
                if (KcUtils::FileNotEmpty(output_filename))
                {
                    std::cout << kStrInfoSuccess << std::endl;
                }
                else
                {
                    std::cout << kStrInfoFailed << std::endl;
                    KcUtils::DeleteFile(output_filename);
                }
            }
        }
        else
        {
            std::cout << kStrErrorFailedToConstructCfgOutputFilename << std::endl;
        }
    }
}

// Disassembles ELF container and saves it to the requested output file according to the given Config.
// pipelineBinary is the full path to the pipeline binary ELF container file.
static void DisassembleElfBinary(const std::string& target, const std::string& pipeline_binary, const Config& config, const std::string& output_filename, std::string error_msg)
{
    const char* kStrInfoDisassemblingBinaryElfContainer = "Disassembling pipeline binary ELF container ";
    const char* kStrInfoDisassemblingBinaryElfContainerSuccess = "Pipeline binary ELF container disassembled successfully.";
    const char* kStrInfoDisassemblingBinaryElfFailure = "failure.";
    const char* kStrInfoDisassemblingBinaryElfContainerOnlyVegaRdna = "Disassembling pipeline binary ELF container (--elf-dis) is only supported for binaries generated for Vega, RDNA and beyond. Skipping for ";
    std::cout << kStrInfoDisassemblingBinaryElfContainer << pipeline_binary << "... " << std::endl;
    std::string elf_disassembly;
    std::string elf_disassembly_dot_text;

    if (KcUtils::IsNaviTarget(target) || KcUtils::IsVegaTarget(target))
    {
        const std::string quoted_binary_path = KcUtils::Quote(pipeline_binary);
        bool is_elf_disassembled = BeUtils::DisassembleCodeObject(quoted_binary_path,
            config.print_process_cmd_line, elf_disassembly, elf_disassembly_dot_text, error_msg);
        assert(is_elf_disassembled);
        if (is_elf_disassembled && !elf_disassembly.empty())
        {
            std::cout << kStrInfoDisassemblingBinaryElfContainerSuccess << std::endl;
            bool isElfDisassemblySaved = KcUtils::WriteTextFile(output_filename, elf_disassembly, nullptr);
            assert(isElfDisassemblySaved);
        }
        else
        {
            std::cout << kStrInfoDisassemblingBinaryElfFailure << std::endl;
        }
    }
    else
    {
        std::cout << kStrInfoDisassemblingBinaryElfContainerOnlyVegaRdna << target << "." << std::endl;
    }
}

// ****************************************
// *** INTERNALLY LINKED SYMBOLS - END ***
// ****************************************

void KcCliCommanderDX12::ListAdapters(Config& config, LoggingCallbackFunction callback)
{
    std::vector<std::string> supported_gpus;
    std::map<std::string, int> driver_mapping;
    dx12_backend_.GetSupportGpus(config, supported_gpus, driver_mapping);
}

void KcCliCommanderDX12::RunCompileCommands(const Config& config, LoggingCallbackFunction callback)
{
    bool is_ok = false;
    bool should_abort = false;

    // Container for all targets.
    std::vector<std::string> target_devices;

    // Targets that have been covered.
    std::vector<std::string> completed_targets;

    // Input validation - commands.
    if (config.isa_file.empty())
    {
        if (!config.livereg_analysis_file.empty())
        {
            std::cout << kStrErrorLiveregWithoutIsa << std::endl;
            should_abort = true;
        }
        else if (!config.block_cfg_file.empty() ||
            !config.inst_cfg_file.empty())
        {
            std::cout << kStrErrorCfgWithoutIsa << std::endl;
            should_abort = true;
        }
    }

    if (!should_abort)
    {
        if (config.pso_dx12_template.empty())
        {
            // Update the user provided config if necessary.
            Config config_updated;
            UpdateConfig(config, config_updated);

            // Validate the --dxc option argument.
            if (!config_updated.dxc_path.empty())
            {
                if (BeUtils::IsFilePresent(config_updated.dxc_path))
                {
                    std::cout << kStrErrorInvalidDxcOptionArgument << std::endl;
                    should_abort = true;
                }
            }

            if (!should_abort)
            {
                // Validate the input.
                bool is_dxr = (config_updated.mode == RgaMode::kModeDxr);
                bool is_input_valid = (is_dxr ? IsInputValidDxr(config_updated) : IsInputValid(config_updated));

                // In DXR pipeline mode, always assume "all" as the export.
                if (is_dxr && !IsDxrShaderMode(config_updated) && config_updated.dxr_exports.empty())
                {
                    config_updated.dxr_exports.push_back("all");
                }

                bool was_asic_list_auto_generated = false;
                if (is_input_valid)
                {
                    if (config_updated.asics.empty())
                    {
                        was_asic_list_auto_generated = true;
                        std::vector<GDT_GfxCardInfo> tmp_card_list;
                        std::set<std::string> device_list;
                        is_ok = BeUtils::GetAllGraphicsCards(tmp_card_list, device_list);
                        assert(is_ok);
                        assert(!device_list.empty());
                        if (is_ok && !device_list.empty())
                        {
                            // Sort and choose the latest target.
                            std::set<std::string, decltype(&BeUtils::DeviceNameLessThan)> sort_unique_names(device_list.begin(),
                                device_list.end(), BeUtils::DeviceNameLessThan);
                            target_devices.push_back(*sort_unique_names.rbegin());
                        }
                    }
                    else
                    {
                        target_devices = config_updated.asics;
                    }

                    if (is_dxr)
                    {
                        // DXR mode only supports gfx1030 and beyond.
                        // Filter unsupported targets with an appropriate message.
                        auto target_devices_tmp = target_devices;
                        target_devices.clear();
                        for (const std::string& target : target_devices_tmp)
                        {
                            // Convert to lower case.
                            const std::string target_lower = KcUtils::ToLower(target);

                            if (KcUtils::IsNavi21AndBeyond(target_lower))
                            {
                                target_devices.push_back(target_lower);
                            }
                            else
                            {
                                std::cout << kStrWarningDxrSkippingUnsupportedTarget << target << "." << std::endl;
                            }
                        }

                        assert(!target_devices.empty());
                        if (target_devices.empty() && was_asic_list_auto_generated)
                        {
                            std::cout << kStrErrorDxrNoSupportedTargetsFound << std::endl;
                        }
                    }

                    assert(!target_devices.empty());
                    if (!target_devices.empty())
                    {
                        // DX12 graphics or compute.
                        for (const std::string& target : target_devices)
                        {
                            // Track the devices that we covered so that we do not compile twice.
                            if (std::find(completed_targets.begin(),
                                completed_targets.end(), target) == completed_targets.end())
                            {
                                // Mark as covered.
                                completed_targets.push_back(target);

                                std::string out_text;
                                std::string error_msg;
                                beStatus rc = beStatus::kBeStatusInvalid;
                                std::cout << kStrInfoCompiling << target << "..." << std::endl;

                                if (is_dxr)
                                {
                                    bool is_all_mode = (config_updated.dxr_exports.size() == 1 && config_updated.dxr_exports[0].compare("all") == 0);
                                    std::vector<RgDxrPipelineResults> output_mapping;
                                    rc = dx12_backend_.CompileDXRPipeline(config_updated, target, out_text, output_mapping, error_msg);
                                    is_ok = (rc == kBeStatusSuccess);
                                    assert(is_ok);
                                    if (!out_text.empty())
                                    {
                                        std::cout << out_text << std::endl;
                                    }
                                    if (!error_msg.empty())
                                    {
                                        std::cout << error_msg << std::endl;
                                    }

                                    bool is_success = is_ok;
                                    if (is_success)
                                    {
                                        for (const RgDxrPipelineResults& curr_pipeline_results : output_mapping)
                                        {
                                            for (const RgDxrShaderResults& curr_shader_results : curr_pipeline_results.results)
                                            {
                                                // The key to the map is the export name.
                                                const std::string& currExport = curr_shader_results.export_name;

                                                // Verify ISA files created.
                                                if (!curr_shader_results.isa_disassembly.empty() && !KcUtils::FileNotEmpty(curr_shader_results.isa_disassembly))
                                                {
                                                    std::cout << kStrErrorDx12IsaNotGeneratedA << currExport
                                                        << kStrErrorDxrIsaNotGeneratedB << std::endl;
                                                    is_success = false;
                                                }

                                                if (!curr_shader_results.stats.empty() && !KcUtils::FileNotEmpty(curr_shader_results.stats))
                                                {
                                                    std::cout << kStrErrorDx12StatsNotGeneratedA << currExport
                                                        << kStrErrorDxrIsaNotGeneratedB << std::endl;
                                                    is_success = false;
                                                }
                                            }

                                            // Verify binary files created.
                                            bool should_extract_pipeline_binaries =
                                                config_updated.dxr_mode.compare(kStrDxrModeShader) != 0 && !curr_pipeline_results.pipeline_binary.empty();
                                            if (should_extract_pipeline_binaries)
                                            {
                                                if (!curr_pipeline_results.pipeline_binary.empty() && !KcUtils::FileNotEmpty(curr_pipeline_results.pipeline_binary))
                                                {
                                                    std::cout << kStrErrorDx12BinaryNotGeneratedA << curr_pipeline_results.pipeline_name
                                                        << kStrErrorDxrIsaNotGeneratedBPipeline << std::endl;
                                                    is_success = false;
                                                }
                                                else
                                                {
                                                    // Generate ELF disassembly if needed.
                                                    if (!config_updated.elf_dis.empty())
                                                    {
                                                        // Construct a name for the output file.
                                                        std::string output_filename;
                                                        is_ok = KcUtils::ConstructOutFileName(config_updated.elf_dis, "",
                                                            target, kStrDefaultExtensionText, output_filename);

                                                        // Disassemble the pipeline binary.
                                                        DisassembleElfBinary(target, curr_pipeline_results.pipeline_binary,
                                                            config_updated, output_filename, error_msg);
                                                    }
                                                }
                                            }
                                        }
                                    }

                                    if (is_success)
                                    {
                                        const bool is_shader_mode = IsDxrShaderMode(config_updated);
                                        std::cout << kStrInfoSuccess << std::endl;
                                        if (!config_updated.livereg_analysis_file.empty() ||
                                            !config_updated.inference_analysis_file.empty() ||
                                            !config_updated.inst_cfg_file.empty() ||
                                            !config_updated.block_cfg_file.empty())
                                        {
                                            // Post-processing.
                                            std::cout << kStrInfoDx12PostProcessingSeparator << std::endl;
                                            std::cout << kStrInfoDx12PostProcessing << std::endl;

                                            // Live register analysis files.
                                            if (!config_updated.livereg_analysis_file.empty())
                                            {
                                                for (const RgDxrPipelineResults& curr_pipeline_results : output_mapping)
                                                {
                                                    if (!IsDxrNullPipeline(curr_pipeline_results.pipeline_name))
                                                    {
                                                        // Announce the pipeline name in pipeline mode.
                                                        std::cout << kStrInfoPerformingLiveregAnalysis1 <<
                                                            (curr_pipeline_results.isUnified ? kStrInfoDxrOutputPipelineName : kStrInfoDxrOutputPipelineNumber) <<
                                                            curr_pipeline_results.pipeline_name << "..." << std::endl;
                                                    }

                                                    for (const RgDxrShaderResults& curr_shader_results : curr_pipeline_results.results)
                                                    {
                                                        std::stringstream filename_suffix_stream;
                                                        if (!IsDxrNullPipeline(curr_pipeline_results.pipeline_name) &&
                                                            !curr_pipeline_results.isUnified)
                                                        {
                                                            filename_suffix_stream << curr_pipeline_results.pipeline_name << "_";
                                                        }
                                                        filename_suffix_stream << curr_shader_results.export_name;
                                                        if (!is_shader_mode && curr_pipeline_results.isUnified)
                                                        {
                                                            filename_suffix_stream << kStrDxrUnifiedSuffix;
                                                        }
                                                        std::string filename_suffix = filename_suffix_stream.str();

                                                        // Do not append a suffix in case that the file name is empty,
                                                        // to prevent a situation where we have the shader name appearing twice.
                                                        bool should_append_suffix = !KcUtils::IsDirectory(config.livereg_analysis_file);

                                                        std::string output_filename;
                                                        KcUtils::ConstructOutFileName(config.livereg_analysis_file, filename_suffix,
                                                            target, kStrDefaultExtensionLivereg, output_filename, should_append_suffix);

                                                        PerformLiveRegisterAnalysisDxr(config_updated, curr_shader_results.isa_disassembly, target,
                                                            output_filename, filename_suffix, curr_shader_results);
                                                    }
                                                }
                                            }

                                            // Per-block control-flow graphs.
                                            if (!config_updated.block_cfg_file.empty())
                                            {
                                                for (const RgDxrPipelineResults& curr_pipeline_results : output_mapping)
                                                {
                                                    if (!IsDxrNullPipeline(curr_pipeline_results.pipeline_name))
                                                    {
                                                        // Announce the pipeline name in pipeline mode.
                                                        std::cout << kStrInfoContructingPerBlockCfg1 <<
                                                            (curr_pipeline_results.isUnified ? kStrInfoDxrOutputPipelineName : kStrInfoDxrOutputPipelineNumber) <<
                                                            curr_pipeline_results.pipeline_name << "..." << std::endl;
                                                    }

                                                    for (const RgDxrShaderResults& curr_shader_results : curr_pipeline_results.results)
                                                    {
                                                        std::stringstream filename_suffix_stream;
                                                        if (!IsDxrNullPipeline(curr_pipeline_results.pipeline_name) &&
                                                            !curr_pipeline_results.isUnified)
                                                        {
                                                            filename_suffix_stream << curr_pipeline_results.pipeline_name << "_";
                                                        }
                                                        filename_suffix_stream << curr_shader_results.export_name;
                                                        if (!is_shader_mode && curr_pipeline_results.isUnified)
                                                        {
                                                            filename_suffix_stream << kStrDxrUnifiedSuffix;
                                                        }
                                                        std::string filename_suffix = filename_suffix_stream.str();

                                                        // Do not append a suffix in case that the file name is empty,
                                                        // to prevent a situation where we have the shader name appearing twice.
                                                        bool should_append_suffix = !KcUtils::IsDirectory(config.block_cfg_file);

                                                        std::string output_filename;
                                                        KcUtils::ConstructOutFileName(config.block_cfg_file, filename_suffix,
                                                            target, kStrDefaultExtensionDot, output_filename, should_append_suffix);

                                                        GeneratePerBlockCfgDxr(config_updated, curr_shader_results.isa_disassembly, target,
                                                            output_filename, filename_suffix, curr_shader_results);
                                                    }
                                                }
                                            }

                                            // Per-instruction control-flow graphs.
                                            if (!config_updated.inst_cfg_file.empty())
                                            {
                                                for (const RgDxrPipelineResults& curr_pipeline_results : output_mapping)
                                                {
                                                    if (!IsDxrNullPipeline(curr_pipeline_results.pipeline_name))
                                                    {
                                                        // Announce the pipeline name in pipeline mode.
                                                        std::cout << kStrInfoContructingPerInstructionCfg1 <<
                                                            (curr_pipeline_results.isUnified ? kStrInfoDxrOutputPipelineName : kStrInfoDxrOutputPipelineNumber) <<
                                                            curr_pipeline_results.pipeline_name << "..." << std::endl;
                                                    }

                                                    for (const RgDxrShaderResults& curr_shader_results : curr_pipeline_results.results)
                                                    {
                                                        std::stringstream filename_suffix_stream;
                                                        if (!IsDxrNullPipeline(curr_pipeline_results.pipeline_name) &&
                                                            !curr_pipeline_results.isUnified)
                                                        {
                                                            filename_suffix_stream << curr_pipeline_results.pipeline_name << "_";
                                                        }
                                                        filename_suffix_stream << curr_shader_results.export_name;
                                                        if (!is_shader_mode && curr_pipeline_results.isUnified)
                                                        {
                                                            filename_suffix_stream << kStrDxrUnifiedSuffix;
                                                        }
                                                        std::string filename_suffix = filename_suffix_stream.str();

                                                        // Do not append a suffix in case that the file name is empty,
                                                        // to prevent a situation where we have the shader name appearing twice.
                                                        bool should_append_suffix = !KcUtils::IsDirectory(config.inst_cfg_file);

                                                        std::string output_filename;
                                                        KcUtils::ConstructOutFileName(config.inst_cfg_file, filename_suffix,
                                                            target, kStrDefaultExtensionDot, output_filename, should_append_suffix);

                                                        GeneratePerInstructionCfgDxr(config_updated, curr_shader_results.isa_disassembly, target,
                                                            output_filename, filename_suffix, curr_shader_results);
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    BeVkPipelineFiles isa_files;
                                    BeVkPipelineFiles amdil_files;
                                    BeVkPipelineFiles stats_files;
                                    std::string binary_file;
                                    rc = dx12_backend_.Compile(config_updated, target, out_text, error_msg,
                                        isa_files, amdil_files, stats_files, binary_file);
                                    is_ok = (rc == kBeStatusSuccess);
                                    if (!out_text.empty())
                                    {
                                        std::cout << out_text << std::endl;
                                    }
                                    if (is_ok)
                                    {
                                        if (!error_msg.empty())
                                        {
                                            std::cout << error_msg << std::endl;
                                        }

                                        bool is_success = true;
                                        for (int stage = 0; stage < BePipelineStage::kCount; stage++)
                                        {
                                            if (!isa_files[stage].empty() && !KcUtils::FileNotEmpty(isa_files[stage]))
                                            {
                                                std::cout << kStrErrorDx12IsaNotGeneratedA <<
                                                    kStrDx12StageNames[stage] << kStrErrorDx12OutputNotGeneratedB << std::endl;
                                                is_success = false;
                                            }
                                            if (!amdil_files[stage].empty() && !KcUtils::FileNotEmpty(amdil_files[stage]))
                                            {
                                                std::cout << kStrErrorDx12AmdilNotGeneratedA <<
                                                    kStrDx12StageNames[stage] << kStrErrorDx12OutputNotGeneratedB << std::endl;
                                                is_success = false;
                                            }
                                            if (!stats_files[stage].empty() && !KcUtils::FileNotEmpty(stats_files[stage]))
                                            {
                                                std::cout << kStrErrorDx12StatsNotGeneratedA
                                                    << kStrDx12StageNames[stage] << kStrErrorDx12StatsNotGeneratedB << std::endl;
                                                is_success = false;
                                            }
                                        }

                                        if (is_success)
                                        {
                                            if (!binary_file.empty() && !KcUtils::FileNotEmpty(binary_file))
                                            {
                                                std::cout << kStrErrorDx12BinaryNotGeneratedA << target << std::endl;
                                                is_success = false;
                                            }
                                            else if (!config_updated.elf_dis.empty())
                                            {
                                                // Construct a name for the output file.
                                                std::string output_filename;
                                                is_ok = KcUtils::ConstructOutFileName(config_updated.elf_dis, "",
                                                    target, kStrDefaultExtensionText, output_filename);

                                                // Disassemble the pipeline binary.
                                                DisassembleElfBinary(target, binary_file, config_updated, output_filename, error_msg);

                                            }
                                        }

                                        if (is_success)
                                        {
                                            std::cout << kStrInfoSuccess << std::endl;
                                            if (!config_updated.livereg_analysis_file.empty() || !config_updated.inst_cfg_file.empty() ||
                                                !config_updated.block_cfg_file.empty() || !config_updated.inference_analysis_file.empty())
                                            {
                                                // Post-processing.
                                                std::cout << kStrInfoDx12PostProcessingSeparator << std::endl;
                                                std::cout << kStrInfoDx12PostProcessing << std::endl;

                                                if (!config_updated.livereg_analysis_file.empty())
                                                {
                                                    // Live register analysis files.
                                                    for (int stage = 0; stage < BePipelineStage::kCount; stage++)
                                                    {
                                                        PerformLiveRegisterAnalysis(isa_files[stage], kStrDx12StageNames[stage], target, config_updated, is_ok);
                                                    }
                                                }

                                                if (!config_updated.block_cfg_file.empty())
                                                {
                                                    // Per-block control-flow graphs.
                                                    for (int stage = 0; stage < BePipelineStage::kCount; stage++)
                                                    {
                                                        GeneratePerBlockCfg(isa_files[stage], "", kStrDx12StageNames[stage], target, config_updated, is_dxr, is_ok);
                                                    }
                                                }

                                                if (!config_updated.inst_cfg_file.empty())
                                                {
                                                    // Per-instruction control-flow graphs.
                                                    for (int stage = 0; stage < BePipelineStage::kCount; stage++)
                                                    {
                                                        GeneratePerInstructionCfg(isa_files[stage], "", kStrDx12StageNames[stage], target, config_updated, is_dxr, is_ok);
                                                    }
                                                }
                                            }
                                        }
                                    }
                                    else if (!error_msg.empty())
                                    {
                                        std::cout << error_msg << std::endl;
                                    }
                                    else
                                    {
                                        std::cout << kStrInfoFailed << std::endl;
                                    }

                                    if (target_devices.size() > 1)
                                    {
                                        // In case that we are compiling for multiple targets,
                                        // print a line of space between the different devices.
                                        std::cout << std::endl;
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        std::cout << kStrErrorDx12NoTargetProvided << std::endl;
                    }
                }
            }
        }
        else
        {
            bool is_file_written = KcUtils::WriteTextFile(config.pso_dx12_template, kStrTemplateGpsoFileContent, nullptr);
            assert(is_file_written);
            if (is_file_written)
            {
                std::cout << kStrInfoTemplateGpsoFileGenerated << std::endl;
            }
        }
    }
}

bool KcCliCommanderDX12::PrintAsicList(const Config& config)
{
    std::vector<std::string> supported_gpus;
    std::vector<std::string> supported_gpus_filtered;
    std::map<std::string, int> device_id_mapping;

    // Retrieve the list of supported targets from the DX12 backend.
    beStatus rc = dx12_backend_.GetSupportGpus(config, supported_gpus, device_id_mapping);
    bool result = (rc == kBeStatusSuccess);
    assert(result);

    // DXR mode only supports Navi21 and beyond: filter unsupported targets.
    if (config.mode != RgaMode::kModeDxr)
    {
        supported_gpus_filtered = supported_gpus;
    }
    else
    {
        for (const std::string& targetName : supported_gpus)
        {
            if (KcUtils::IsNavi21AndBeyond(targetName))
            {
                supported_gpus_filtered.push_back(targetName);
            }
        }
    }

    // Filter duplicates and call the shared print routine.
    std::set<std::string> supported_gpus_unique = std::set<std::string>(supported_gpus_filtered.begin(), supported_gpus_filtered.end());
    result = result && KcUtils::PrintAsicList(supported_gpus_unique);

    return result;
}

#endif

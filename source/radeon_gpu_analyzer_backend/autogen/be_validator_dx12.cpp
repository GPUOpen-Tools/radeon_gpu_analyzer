//=============================================================================
/// Copyright (c) 2024-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for dx12 pipeline auto generator class.
//=============================================================================
// Dx12 Backend.
#include "utils/dx12/backend/rg_dx12_utils.h"

// Shared.
#include "common/rga_shared_utils.h"

// Local.
#include "radeon_gpu_analyzer_backend/autogen/be_validator_dx12.h"
#include "radeon_gpu_analyzer_backend/autogen/be_generator_dx12.h"
#include "radeon_gpu_analyzer_backend/be_string_constants.h"
#include "radeon_gpu_analyzer_backend/be_utils.h"
#include "radeon_gpu_analyzer_backend/be_program_builder_dx12.h"
#include "radeon_gpu_analyzer_backend/autogen/be_utils_dx12.h"

// CLI.
#include "source/radeon_gpu_analyzer_cli/kc_utils.h"

// DXR.
#include "utils/dx12/backend/rg_dxr_state_desc_reader.h"
#include "utils/dx12/backend/rg_dxr_output_metadata.h"
#include "source/radeon_gpu_analyzer_cli/kc_cli_string_constants.h"

// Constant error strings.
static const char* kStrErrorNoGpsoFileA = "Error: .gpso file must be provided for graphics pipelines (use the --pso option).";
static const char* kStrErrorNoGpsoFileB = ".gpso file could be auto-generated for pipelines with vertex, mesh, pixel, or compute shaders.";
static const char* kStrErrorNoGpsoFileHintA = "The format of a .gpso file is : ";
static const char* kStrErrorNoGpsoFileHintB = "See --gpso-template option for more info.";
static const char* kStrErrorDxrRootSignatureHlslFileDefinedA =
    "Error: use --rs-hlsl option together with --rs-macro to specify the HLSL file where the macro is defined.";
static const char* kStrErrorDxrRootSignatureHlslFileDefinedB       = "root signature file could be auto-generated for pipelines with vertex, mesh, pixel, or compute shaders.";
static const char* kStrErrorNoIsaNoStatsOption                     = "Error: one of --isa or -a/--analysis or -b/--binary options must be provided.";
static const char* kStrErrorMixComputeGraphicsInputA               = "Error: cannot mix compute and graphics for ";
static const char* kStrErrorMixComputeGraphicsInputShader          = "shader input files.";
static const char* kStrErrorMixComputeGraphicsInputDxilDisassembly = "DXIL/DXBC disassembly output file.";
static const char* kStrErrorMixComputeGraphicsInputShaderModel     = "shader model.";
static const char* kStrErrorMixComputeGraphicsInputEntryPoint      = "entry point.";
static const char* kStrErrorDx12MixMeshTraditional                 = "Error: cannot mix mesh/amplification shaders with vertex, hull, domain, or geometry shaders.";
static const char* kStrErrorDx12AmplificationWithoutMesh           = "Error: amplification shader requires a mesh shader. Please provide a mesh shader using --ms or --ms-blob.";
static const char* kStrErrorDx12TwoInputFilesPerStageA             = "Error: both HLSL and DXBC input files specified for ";
static const char* kStrErrorDx12TwoInputFilesPerStageB             = "shader. Only a single input file can be passed per stage.";
static const char* kStrErrorDx12NoShaderModelProvidedA             = "Error: no shader model provided for ";
static const char* kStrErrorDx12NoShaderModelProvidedB             = " shader.";
static const char* kStrErrorDx12NoEntryPointProvidedA              = "Error: no entry point provided for ";
static const char* kStrErrorDx12NoEntryPointProvidedB              = " shader.";
static const char* kStrErrorDx12ShaderNotFoundOrEmptyA             = "Error: ";
static const char* kStrErrorDx12ShaderNotFoundOrEmptyB             = " shader does not exist or file is empty: ";
static const char* kStrErrorDx12CannotGenerateElfDisassemblyWithoutPipelineBinary =
    "Error: cannot generate ELF disassembly if pipeline binary is not "
    "generated (must use -b option together with --elf-dis option).";
static const char* kStrErrorOfflineTargetDeviceMustBeSpecifed = "Error: target device must be specified in offline mode (use -c option).";
static const char* kStrErrorDxrHlslInputFileMissing           = "Error: no HLSL input source file provided (use --hlsl option).";
static const char* kStrErrorDxrStateDescAndHlslFileMix =
    "Error: cannot mix --state-desc and --hlsl options.If --hlsl option is used, it must contain the entire state definition in the HLSL source code.";
static const char* kStrErrorDxrStateDescFileNotFoundOrEmpty = "Error: DXR state JSON file not found or empty: ";
static const char* kStrErrorDxrHlslFileNotFoundOrEmpty      = "Error: HLSL input file not found or empty: ";
static const char* kStrWarningDxrBinaryExtractionNotSupportedInShaderMode =
    "Warning: pipeline binary extraction (-b option) "
    "is not supported in Shader mode.";
static const char* kStrInfoInvalidPipelineOptionAutogen =
    "Info: Shader/state autogeneration only supported for graphics pipelines that include a VS or MS, PS or a combination thereof.";
static const char* kStrInfoInvalidAutogenDirectory = "Error: Invalid folder path provided for autogenerating the missing component of D3D12 pipeline.";
static const char* kStrErrorInvalidShaderModelA = "Error: Invalid shader model for ";
static const char* kStrErrorInvalidShaderModelB = " stage. Expected model is 6_5 or higher.";

// Validates the input arguments for a specific stage, and prints appropriate error messages to the console.
static bool IsShaderInputsValid(const Config&      config,
                                const std::string& shader_hlsl,
                                const std::string& shader_model,
                                const std::string& shader_entry_point,
                                const std::string& shader_dxbc,
                                const std::string& stage_name)
{
    bool ret = true;
    if (!shader_hlsl.empty())
    {
        if (!KcUtils::FileNotEmpty(shader_hlsl))
        {
            std::cout << kStrErrorDx12ShaderNotFoundOrEmptyA << stage_name << kStrErrorDx12ShaderNotFoundOrEmptyB << shader_hlsl << std::endl;
            ret = false;
        }

        // Validate inputs for shader front-end compilation.
        if (shader_model.empty() && config.all_model.empty())
        {
            std::cout << kStrErrorDx12NoShaderModelProvidedA << stage_name << kStrErrorDx12NoShaderModelProvidedB << std::endl;
            ret = false;
        }
        else if (shader_entry_point.empty())
        {
            std::cout << kStrErrorDx12NoEntryPointProvidedA << stage_name << kStrErrorDx12NoEntryPointProvidedB << std::endl;
            ret = false;
        }
    }

    if (!shader_entry_point.empty())
    {
        if (shader_hlsl.empty() && config.all_hlsl.empty() && shader_dxbc.empty())
        {
            std::cout << kStrErrorDx12ShaderNotFoundOrEmptyA << stage_name << kStrErrorDx12ShaderNotFoundOrEmptyB << std::endl;
            ret = false;
        }
    }

    if (!shader_model.empty() &&
        (stage_name == kStrDx12StageNames[BePipelineStage::kTask] ||
         stage_name == kStrDx12StageNames[BePipelineStage::kMesh]))
    {
        // Expected model is 6_5 or higher
        int version_major = std::atoi(shader_model.substr(3, 1).c_str());
        int version_minor = std::atoi(shader_model.substr(5, 1).c_str());

        if (version_major < 6 || (version_major == 6 && version_minor < 5))
        {
            std::cout << kStrErrorInvalidShaderModelA << stage_name << kStrErrorInvalidShaderModelB << std::endl;
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
        std::cout << kStrErrorDx12ShaderNotFoundOrEmptyA << stage << kStrErrorDx12ShaderNotFoundOrEmptyB << dxbc_input_file << std::endl;
        ret = false;
    }
    return ret;
}

static bool IsAutogeneratePipelinePossible(const Config& config)
{
    bool ret = true;
    if (!config.hs_dxbc.empty() || !config.ds_dxbc.empty() || !config.gs_dxbc.empty() || !config.hs_hlsl.empty() || !config.ds_hlsl.empty() ||
        !config.gs_hlsl.empty() || !config.hs_dxil_disassembly.empty() || !config.ds_dxil_disassembly.empty() || !config.gs_dxil_disassembly.empty() ||
        !config.hs_model.empty() || !config.ds_model.empty() || !config.gs_model.empty() || !config.hs_entry_point.empty() || !config.ds_entry_point.empty() ||
        !config.gs_entry_point.empty())
    {
        bool has_vs = (!config.vs_hlsl.empty() || !config.vs_dxbc.empty());
        bool has_ms = (!config.ms_hlsl.empty() || !config.ms_dxbc.empty());
        bool has_ps = (!config.ps_hlsl.empty() || !config.ps_dxbc.empty());
        bool is_complete_graphics_pipeline = (has_vs || has_ms) && has_ps;
        if (!is_complete_graphics_pipeline)
        {
            std::cout << kStrInfoInvalidPipelineOptionAutogen << "\n";
        }
        ret = false;
    }
    return ret;
}

// Validates the input pso and rs and prints appropriate error messages to the console.
static bool IsInputValidPSOAndRS(const Config& config, const BeDx12AutoGenPipelineInfo& info)
{
    bool ret = true;

    if (info.gpso_file.status == BeDx12AutoGenStatus::kSuccess)
    {
        ; // rga managed to auto-generate gpso.
    }
    else
    {
        // Verify that we have a valid PSO file.
        if (config.cs_hlsl.empty() && config.cs_dxbc.empty() && config.pso_dx12.empty() &&
            (!config.vs_hlsl.empty() || !config.vs_dxbc.empty() || !config.ms_hlsl.empty() || !config.ms_dxbc.empty() ||
             !config.hs_hlsl.empty() || !config.hs_dxbc.empty() || !config.ds_hlsl.empty() || !config.ds_dxbc.empty() ||
             !config.gs_hlsl.empty() || !config.gs_dxbc.empty() || !config.ps_hlsl.empty() || !config.ps_dxbc.empty()))
        {
            std::cout << kStrErrorNoGpsoFileA << "\n";
            std::cout << kStrErrorNoGpsoFileB << "\n";
            std::cout << kStrErrorNoGpsoFileHintA << "\n" << kStrTemplateGpsoFileContent << "\n" << kStrErrorNoGpsoFileHintB << "\n";
            ret = false;
        }
    }

    if (info.root_signature.status == BeDx12AutoGenStatus::kSuccess)
    {
        ;  // rga managed to auto-generate root signature.
    }
    else
    {
        // For graphics pipelines, if --rs-macro is used, make sure that either --rs-hlsl or --all-hlsl is used
        // so that we know at which HLSL file to search for the macro definition.
        if (!config.rs_macro.empty() && config.cs_hlsl.empty() && config.all_hlsl.empty() && config.rs_hlsl.empty())
        {
            std::cout << kStrErrorDxrRootSignatureHlslFileDefinedA << "\n";
            std::cout << kStrErrorDxrRootSignatureHlslFileDefinedB << "\n";
            ret = false;
        }
    }

    return ret;
}

// Validates the input and prints appropriate error messages to the console.
static bool IsInputValidDX12(const Config& config)
{
    bool ret = true;
    if (config.isa_file.empty() && config.analysis_file.empty() && config.binary_output_file.empty())
    {
        std::cout << kStrErrorNoIsaNoStatsOption << "\n";
        ret = false;
    }

    if (ret)
    {
        if ((!config.cs_dxbc.empty() || !config.cs_hlsl.empty()) &&
            ((!config.vs_dxbc.empty() || !config.ms_dxbc.empty() || !config.as_dxbc.empty() || !config.hs_dxbc.empty() || !config.ds_dxbc.empty() || !config.gs_dxbc.empty() || !config.ps_dxbc.empty()) ||
             (!config.vs_hlsl.empty() || !config.ms_hlsl.empty() || !config.as_hlsl.empty() || !config.hs_hlsl.empty() || !config.ds_hlsl.empty() || !config.gs_hlsl.empty() || !config.ps_hlsl.empty())))
        {
            ret = false;
            std::cout << kStrErrorMixComputeGraphicsInputA << kStrErrorMixComputeGraphicsInputShader << "\n";
        }
        else if (!config.cs_dxil_disassembly.empty() &&
                 (!config.vs_dxil_disassembly.empty() || !config.ms_dxil_disassembly.empty() || !config.as_dxil_disassembly.empty() || !config.hs_dxil_disassembly.empty() ||
                  !config.ds_dxil_disassembly.empty() || !config.gs_dxil_disassembly.empty() || !config.ps_dxil_disassembly.empty()))
        {
            ret = false;
            std::cout << kStrErrorMixComputeGraphicsInputA << kStrErrorMixComputeGraphicsInputDxilDisassembly << "\n";
        }
        else if (!config.cs_model.empty() &&
                 (!config.vs_model.empty() || !config.ms_model.empty() || !config.as_model.empty() || !config.hs_model.empty() || !config.ds_model.empty() || !config.gs_model.empty() || !config.ps_model.empty()))
        {
            ret = false;
            std::cout << kStrErrorMixComputeGraphicsInputA << kStrErrorMixComputeGraphicsInputShaderModel << "\n";
        }
        else if (!config.cs_entry_point.empty() && (!config.vs_entry_point.empty() || !config.ms_entry_point.empty() || !config.as_entry_point.empty() ||
                                                    !config.hs_entry_point.empty() || !config.ds_entry_point.empty() ||
                                                    !config.gs_entry_point.empty() || !config.ps_entry_point.empty()))
        {
            ret = false;
            std::cout << kStrErrorMixComputeGraphicsInputA << kStrErrorMixComputeGraphicsInputEntryPoint << "\n";
        }
    }

    if (ret)
    {
        if (!config.cs_hlsl.empty() && !config.cs_dxbc.empty())
        {
            std::cout << kStrErrorDx12TwoInputFilesPerStageA << "compute " << kStrErrorDx12TwoInputFilesPerStageB << "\n";
            ret = false;
        }
        else if (!config.vs_hlsl.empty() && !config.vs_dxbc.empty())
        {
            std::cout << kStrErrorDx12TwoInputFilesPerStageA << "vertex " << kStrErrorDx12TwoInputFilesPerStageB << "\n";
            ret = false;
        }
        else if (!config.ms_hlsl.empty() && !config.ms_dxbc.empty())
        {
            std::cout << kStrErrorDx12TwoInputFilesPerStageA << "mesh " << kStrErrorDx12TwoInputFilesPerStageB << "\n";
            ret = false;
        }
        else if (!config.as_hlsl.empty() && !config.as_dxbc.empty())
        {
            std::cout << kStrErrorDx12TwoInputFilesPerStageA << "amplification " << kStrErrorDx12TwoInputFilesPerStageB << "\n";
            ret = false;
        }
        else if (!config.hs_hlsl.empty() && !config.hs_dxbc.empty())
        {
            std::cout << kStrErrorDx12TwoInputFilesPerStageA << "hull " << kStrErrorDx12TwoInputFilesPerStageB << "\n";
            ret = false;
        }
        else if (!config.ds_hlsl.empty() && !config.ds_dxbc.empty())
        {
            std::cout << kStrErrorDx12TwoInputFilesPerStageA << "domain " << kStrErrorDx12TwoInputFilesPerStageB << "\n";
            ret = false;
        }
        else if (!config.gs_hlsl.empty() && !config.gs_dxbc.empty())
        {
            std::cout << kStrErrorDx12TwoInputFilesPerStageA << "geometry " << kStrErrorDx12TwoInputFilesPerStageB << "\n";
            ret = false;
        }
        else if (!config.ps_hlsl.empty() && !config.ps_dxbc.empty())
        {
            std::cout << kStrErrorDx12TwoInputFilesPerStageA << "pixel " << kStrErrorDx12TwoInputFilesPerStageB << "\n";
            ret = false;
        }
    }

    // Validate that mesh/amplification shaders are not mixed with vertex/hull/domain/geometry shaders.
    if (ret)
    {
        bool has_mesh_pipeline = !config.ms_hlsl.empty() || !config.ms_dxbc.empty() || !config.as_hlsl.empty() || !config.as_dxbc.empty();
        bool has_traditional   = !config.vs_hlsl.empty() || !config.vs_dxbc.empty() || !config.hs_hlsl.empty() || !config.hs_dxbc.empty() ||
                                 !config.ds_hlsl.empty() || !config.ds_dxbc.empty() || !config.gs_hlsl.empty() || !config.gs_dxbc.empty();
        if (has_mesh_pipeline && has_traditional)
        {
            std::cout << kStrErrorDx12MixMeshTraditional << "\n";
            ret = false;
        }
    }

    // Validate that amplification shaders are paired with mesh shaders.
    if (ret)
    {
        bool has_as_pipeline = !config.as_hlsl.empty() || !config.as_dxbc.empty();
        bool has_ms_pipeline = !config.ms_hlsl.empty() || !config.ms_dxbc.empty();
        if (has_as_pipeline && !has_ms_pipeline)
        {
            std::cout << kStrErrorDx12AmplificationWithoutMesh << "\n";
            ret = false;
        }
    }

    if (ret)
    {
        if (!config.cs_hlsl.empty())
        {
            // Compute.
            if (!IsShaderInputsValid(
                    config, config.cs_hlsl, config.cs_model, config.cs_entry_point, config.cs_dxbc, kStrDx12StageNames[BePipelineStage::kCompute]))
            {
                ret = false;
            }
        }
        else
        {
            // Vertex.
            if (!IsShaderInputsValid(
                    config, config.vs_hlsl, config.vs_model, config.vs_entry_point, config.vs_dxbc, kStrDx12StageNames[BePipelineStage::kVertex]))
            {
                ret = false;
            }

            // Hull.
            if (!IsShaderInputsValid(
                    config, config.hs_hlsl, config.hs_model, config.hs_entry_point, config.hs_dxbc, kStrDx12StageNames[BePipelineStage::kTessellationControl]))
            {
                ret = false;
            }

            // Domain.
            if (!IsShaderInputsValid(
                    config, config.ds_hlsl, config.ds_model, config.ds_entry_point, config.ds_dxbc, kStrDx12StageNames[BePipelineStage::kTessellationEvaluation]))
            {
                ret = false;
            }

            // Geometry.
            if (!IsShaderInputsValid(
                    config, config.gs_hlsl, config.gs_model, config.gs_entry_point, config.gs_dxbc, kStrDx12StageNames[BePipelineStage::kGeometry]))
            {
                ret = false;
            }

            // Pixel.
            if (!IsShaderInputsValid(
                    config, config.ps_hlsl, config.ps_model, config.ps_entry_point, config.ps_dxbc, kStrDx12StageNames[BePipelineStage::kFragment]))
            {
                ret = false;
            }

            // Mesh.
            if (!IsShaderInputsValid(
                    config, config.ms_hlsl, config.ms_model, config.ms_entry_point, config.ms_dxbc, kStrDx12StageNames[BePipelineStage::kMesh]))
            {
                ret = false;
            }

            // Amplification.
            if (!IsShaderInputsValid(
                    config, config.as_hlsl, config.as_model, config.as_entry_point, config.as_dxbc, kStrDx12StageNames[BePipelineStage::kTask]))
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
        if (!IsDxbcInputValid(config.ms_dxbc, kStrDx12StageNames[BePipelineStage::kMesh]))
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
        if (!IsDxbcInputValid(config.as_dxbc, kStrDx12StageNames[BePipelineStage::kTask]))
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

    if (ret)
    {
        // Cannot generate ELF disassembly if pipeline binary is not generated.
        if (!config.elf_dis.empty() && config.binary_output_file.empty())
        {
            std::cout << kStrErrorDx12CannotGenerateElfDisassemblyWithoutPipelineBinary << "\n";
            ret = false;
        }
    }

    // In Offline mode, the target device must be specified.
    if ((config.dx12_offline_session || !config.alternative_amdxc.empty()) && config.asics.empty())
    {
        std::cout << kStrErrorOfflineTargetDeviceMustBeSpecifed << "\n";
        ret = false;
    }

    return ret;
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

    // In Offline mode, the target device must be specified.
    if ((config.dx12_offline_session || !config.alternative_amdxc.empty()) && config.asics.empty())
    {
        std::cout << kStrErrorOfflineTargetDeviceMustBeSpecifed << std::endl;
        ret = false;
    }

    return ret;
}

bool BeDx12PipelineValidator::ValidateAndGenerateDx12Pipeline(const Config& config, bool is_dxr_pipeline, BeDx12AutoGenPipelineInfo& info)
{
    bool is_input_valid = false;
    if (is_dxr_pipeline)
    {
        is_input_valid = IsInputValidDxr(config);
    }
    else
    {
        is_input_valid = IsInputValidDX12(config);
        if (is_input_valid)
        {
            if (IsAutogeneratePipelinePossible(config))
            {
                AutoGenerateMissingPipeline(config, info);
            }
            is_input_valid = IsInputValidPSOAndRS(config, info);
        }
    }
    return is_input_valid;
}

void BeDx12PipelineValidator::AutoGenerateMissingPipeline(const Config& config, BeDx12AutoGenPipelineInfo& info)
{
    BeDx12AutoGenerator generator;
    info.autogen_status = generator.PopulateAutoGenInputs(config, info.source_code, info.dxc_out);
    if (info.autogen_status == BeDx12AutoGenStatus::kRequired)
    {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC parsed_gpso = {};
        if (info.source_code.is_gpso_specified)
        {
            if (RgDx12Utils::ParseGpsoFile(BeDx12Utils::GetAbsoluteFileName(KcUtils::Quote(config.pso_dx12)), parsed_gpso))
            {
                info.source_code.parsed_gpso_file = &parsed_gpso;
            }
        }

        if (!config.dx12_autogen_dir.empty())
        {
            info.autogen_dir = BeDx12Utils::GetAbsoluteFileName(config.dx12_autogen_dir);
            if (KcUtils::IsDirectory(info.autogen_dir))
            {
                info.should_retain_temp_files = true;
            }
            else
            {
                info.autogen_status = BeDx12AutoGenStatus::kFailed;
                std::cout << kStrInfoInvalidAutogenDirectory << "\n\n";
            }
        }

        if (config.should_retain_temp_files)
        {
            info.should_retain_temp_files = true;
        }

        if (info.autogen_status == BeDx12AutoGenStatus::kRequired)
        {
            beKA::beStatus rc = generator.GenerateFiles(
                config, info.source_code, info.autogen_dir, info.root_signature, info.gpso_file, info.vertex_shader, info.mesh_shader, info.pixel_shader, info.dxc_out);
            if (rc == beKA::beStatus::kBeStatusSuccess)
            {
                info.autogen_status = BeDx12AutoGenStatus::kSuccess;
            }
        }
    }
}

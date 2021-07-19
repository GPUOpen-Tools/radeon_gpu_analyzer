// D3D12.
#include <d3d12.h>

// C++.
#include <cassert>
#include <string>
#include <vector>

// cxxopts.
#include "cxxopts/include/cxxopts.hpp"

// Local.
#include "rg_dx12_frontend.h"
#include "rg_dx12_data_types.h"
#include "rg_dx12_utils.h"

using namespace Microsoft::WRL;

// *** CONSTANTS - START ***

static const char* kStrErrorMultipleInputFilesPerStage1 = "Error: both HLSL and DXBC input file provided for ";
static const char* kStrErrorMultipleInputFilesPerStage2 = " stage. Only a single input file is valid per stage.";
static const char* kStrErrorNoInputFileProvided1 = "Error: no input file was provided for .";
static const char* kStrErrorNoInputFileProvided2 = " stage.";
static const char* kStrErrorMixedRootSignatureInput = "Error: cannot mix root signature inputs. The command instructs to both use serialized root "
"signature from file and extract root signature from hlsl code. Only one option is valid at a time.";
static const char* kStrErrorTargetListFetchFailure = "Error: failed to retrieve the list of supported targets.";
static const char* kStrErrorDriverInitFailure = "Error: driver initialization failed.";
static const char* kStrErrorMixComputeAndGraphics = "Cannot mix and match compute shaders with non-compute shaders.";
static const char* kStrErrorNoDxrStateDescriptionFile = "Error: no DXR state description file provided.";
static const char* kStrInfoCompilingComputePipeline = "Compiling compute pipeline...";
static const char* kStrInfoCompilingGraphicsPipeline = "Compiling graphics pipeline...";
static const char* kStrInfoCompilingDxrPipeline = "Compiling ray tracing pipeline...";
static const char* kStrInfoCompilingDxrShader = "Compiling ray tracing shader...";

// *** CONSTANTS - END ***


// *** INTERNALLY LINKED UTILITIES - START ***

// Returns true if the user specified a DXR configuration.
static bool IsDxrConfig(const rga::RgDx12Config &config, bool& is_pipeline_mode, bool& is_shader_mode)
{
    const char* kStrModePipeline = "pipeline";
    const char* kStrModeShader = "shader";
    std::string mode_lower = rga::RgDx12Utils::ToLower(config.dxr_mode);
    is_pipeline_mode = mode_lower.compare(kStrModePipeline) == 0;
    is_shader_mode = !is_pipeline_mode && mode_lower.compare(kStrModeShader) == 0;
    return (is_pipeline_mode || is_shader_mode);
}

// Validates the inputs, prints error messages as
// necessary and also adjusts the configuration as necessary.
// Returns true if valid, otherwise returns false.
static bool IsInputValid(rga::RgDx12Config& config)
{
    bool ret = true;
    if (!config.should_list_targets)
    {
        bool is_compute_pipeline = !config.comp.hlsl.empty() || !config.comp.dxbc.empty();
        if (is_compute_pipeline)
        {
            // Only a single input file is valid.
            if (!config.comp.hlsl.empty() && !config.comp.dxbc.empty())
            {
                std::cout << kStrErrorMultipleInputFilesPerStage1 << "compute" <<
                    kStrErrorMultipleInputFilesPerStage2 << std::endl;
                ret = false;
            }
            if (ret && config.comp.hlsl.empty() && config.comp.dxbc.empty())
            {
                std::cout << kStrErrorNoInputFileProvided1 << "compute" <<
                    kStrErrorNoInputFileProvided2 << std::endl;
                ret = false;
            }
        }
        else
        {
            bool is_dxr_pipeline = false;
            bool is_dxr_shader = false;
            bool is_dxr = IsDxrConfig(config, is_dxr_pipeline, is_dxr_shader);
            if (is_dxr)
            {
                if (config.dxr_state_file.empty() && config.dxr_hlsl_input.empty())
                {
                    std::cout << kStrErrorNoDxrStateDescriptionFile << std::endl;
                    ret = false;
                }
            }
            else
            {
                // Vertex shader must be present.
                if (config.vert.hlsl.empty() && config.vert.dxbc.empty())
                {
                    std::cout << kStrErrorNoInputFileProvided1 << "vertex" <<
                        kStrErrorNoInputFileProvided2 << std::endl;
                    ret = false;
                }

                // Only a single input file is valid.
                if (ret && !config.vert.hlsl.empty() && !config.vert.dxbc.empty())
                {
                    std::cout << kStrErrorMultipleInputFilesPerStage1 << "vertex" <<
                        kStrErrorMultipleInputFilesPerStage2 << std::endl;
                    ret = false;
                }
                if (ret && !config.hull.hlsl.empty() && !config.hull.dxbc.empty())
                {
                    std::cout << kStrErrorMultipleInputFilesPerStage1 << "hull" <<
                        kStrErrorMultipleInputFilesPerStage2 << std::endl;
                    ret = false;
                }
                if (ret && !config.domain.hlsl.empty() && !config.domain.dxbc.empty())
                {
                    std::cout << kStrErrorMultipleInputFilesPerStage1 << "domain" <<
                        kStrErrorMultipleInputFilesPerStage2 << std::endl;
                    ret = false;
                }
                if (ret && !config.geom.hlsl.empty() && !config.geom.dxbc.empty())
                {
                    std::cout << kStrErrorMultipleInputFilesPerStage1 << "geometry" <<
                        kStrErrorMultipleInputFilesPerStage2 << std::endl;
                    ret = false;
                }
                if (ret && !config.pixel.hlsl.empty() && !config.pixel.dxbc.empty())
                {
                    std::cout << kStrErrorMultipleInputFilesPerStage1 << "pixel" <<
                        kStrErrorMultipleInputFilesPerStage2 << std::endl;
                    ret = false;
                }
            }
        }

        // Do not mix and match compute and non-compute shaders.
        if ((!config.comp.hlsl.empty() || !config.comp.dxbc.empty()) &&
            ((!config.vert.hlsl.empty() || !config.vert.dxbc.empty())    ||
            (!config.hull.hlsl.empty() || !config.hull.dxbc.empty())     ||
                (!config.domain.hlsl.empty() || !config.domain.dxbc.empty()) ||
                (!config.geom.hlsl.empty() || !config.geom.dxbc.empty()) ||
                (!config.pixel.hlsl.empty() || !config.pixel.dxbc.empty())))
        {
            std::cout << kStrErrorMixComputeAndGraphics << std::endl;
            ret = false;
        }

        // Do not mix root signature inputs.
        if (ret && !config.rs_macro.empty() && !config.rs_serialized.empty())
        {
            std::cout << kStrErrorMixedRootSignatureInput << std::endl;
            ret = false;
        }

        if (ret)
        {
            // Set the root signature file if there is only a single input file.
            if (is_compute_pipeline)
            {
                if (config.rs_macro_file.empty() && !config.comp.hlsl.empty())
                {
                    config.rs_macro_file = config.comp.hlsl;
                }
            }
            else
            {
                // Vertex shader must be present. Check if there is no different shader in the pipeline.
                bool is_single_hlsl_file =
                    (config.hull.hlsl.empty() || config.vert.hlsl.compare(config.hull.hlsl) == 0) &&
                    (config.domain.hlsl.empty() || config.vert.hlsl.compare(config.domain.hlsl) == 0) &&
                    (config.geom.hlsl.empty() || config.vert.hlsl.compare(config.geom.hlsl) == 0) &&
                    (config.pixel.hlsl.empty() || config.vert.hlsl.compare(config.pixel.hlsl) == 0);

                // If there is only a single hlsl input file, and the user did not explicitly provide
                // a file to read the root signature macro definition from, use that single hlsl input
                // file as the source for compiling the root signature.
                if (is_single_hlsl_file && config.rs_macro_file.empty())
                {
                    config.rs_macro_file = config.vert.hlsl;
                }
            }
        }
    }
    return ret;
}

// *** INTERNALLY LINKED UTILITIES - END ***


int main(int argc, char* argv[])
{
    assert(argv != nullptr);
    if (argv != nullptr)
    {
        // Flag for tracking failure.
        bool is_ok = true;

        try
        {
            // Keep user inputs in this structure.
            rga::RgDx12Config config;

            cxxopts::Options opts(argv[0]);
            opts.add_options()
                ("h,help", "Print help.")
                ("l, list", "List the names and IDs of the supported targets by the driver.", cxxopts::value<bool>(config.should_list_targets))
                ("c, asic", "The target GPU ID for the pipeline compilation. If not specified, the pipeline would be compiled "
                    "for the physically installed GPU by default.", cxxopts::value<int>(config.target_gpu))

                // Pipeline binary.
                ("b", "Full path to the output file for the pipeline binary.", cxxopts::value<std::string>(config.pipeline_binary))

                // Compute.
                ("comp", "Full path to the HLSL file where the compute shader is defined.", cxxopts::value<std::string>(config.comp.hlsl))
                ("comp-dxbc", "Full path to the compute shader's compiled DXBC binary.", cxxopts::value<std::string>(config.comp.dxbc))
                ("comp-isa", "Full path to ISA disassembly output file.", cxxopts::value<std::string>(config.comp.isa))
                ("comp-stats", "Full path to resource usage output file.", cxxopts::value<std::string>(config.comp.stats))
                ("comp-entry", "Name of the entry point.", cxxopts::value<std::string>(config.comp.entry_point))
                ("comp-dxbc-dump", "Full path to output file for compiled compute shader DXBC binary.",
                    cxxopts::value<std::string>(config.comp.dxbcOut))
                ("comp-dxbc-dis", "Full path to output file for compiled compute shader DXBC disassembly.",
                    cxxopts::value<std::string>(config.comp.dxbc_disassembly))
                ("comp-target", "Shader model for compute shader (e.g. \"cs_5_0\" or \"cs_5_1\".", cxxopts::value<std::string>(config.comp.shader_model))

                // Vertex.
                ("vert", "Full path to the HLSL file where the vertex shader is defined.", cxxopts::value<std::string>(config.vert.hlsl))
                ("vert-dxbc", "Full path to the vertex shader's compiled DXBC binary.", cxxopts::value<std::string>(config.vert.dxbc))
                ("vert-isa", "Full path to ISA disassembly output file.", cxxopts::value<std::string>(config.vert.isa))
                ("vert-stats", "Full path to resource usage output file.", cxxopts::value<std::string>(config.vert.stats))
                ("vert-entry", "Name of the entry point.", cxxopts::value<std::string>(config.vert.entry_point))
                ("vert-dxbc-dump", "Full path to output file for compiled vertex shader DXBC binary.",
                    cxxopts::value<std::string>(config.vert.dxbcOut))
                ("vert-dxbc-dis", "Full path to output file for compiled vertex shader DXBC disassembly.",
                    cxxopts::value<std::string>(config.vert.dxbc_disassembly))
                ("vert-target", "Shader model for vertex shader (e.g. \"vs_5_0\" or \"vs_5_1\".", cxxopts::value<std::string>(config.vert.shader_model))

                // Hull.
                ("hull", "Full path to the HLSL file where the hull shader is defined.", cxxopts::value<std::string>(config.hull.hlsl))
                ("hull-dxbc", "Full path to the hull shader's compiled DXBC binary.", cxxopts::value<std::string>(config.hull.dxbc))
                ("hull-isa", "Full path to ISA disassembly output file.", cxxopts::value<std::string>(config.hull.isa))
                ("hull-stats", "Full path to resource usage output file.", cxxopts::value<std::string>(config.hull.stats))
                ("hull-entry", "Name of the entry point.", cxxopts::value<std::string>(config.hull.entry_point))
                ("hull-dxbc-dump", "Full path to output file for compiled hull shader DXBC binary.",
                    cxxopts::value<std::string>(config.hull.dxbcOut))
                ("hull-dxbc-dis", "Full path to output file for compiled hull shader DXBC disassembly.",
                    cxxopts::value<std::string>(config.hull.dxbc_disassembly))
                ("hull-target", "Shader model for hull shader (e.g. \"hs_5_0\" or \"hs_5_1\".", cxxopts::value<std::string>(config.hull.shader_model))

                // Domain.
                ("domain", "Full path to the HLSL file where the domain shader is defined.", cxxopts::value<std::string>(config.domain.hlsl))
                ("domain-dxbc", "Full path to the domain shader's compiled DXBC binary.", cxxopts::value<std::string>(config.domain.dxbc))
                ("domain-isa", "Full path to ISA disassembly output file.", cxxopts::value<std::string>(config.domain.isa))
                ("domain-stats", "Full path to resource usage output file.", cxxopts::value<std::string>(config.domain.stats))
                ("domain-entry", "Name of the entry point.", cxxopts::value<std::string>(config.domain.entry_point))
                ("domain-dxbc-dump", "Full path to output file for compiled domain shader DXBC binary.",
                    cxxopts::value<std::string>(config.domain.dxbcOut))
                ("domain-dxbc-dis", "Full path to output file for compiled domain shader DXBC disassembly.",
                    cxxopts::value<std::string>(config.domain.dxbc_disassembly))
                ("domain-target", "Shader model for domain shader (e.g. \"ds_5_0\" or \"ds_5_1\".", cxxopts::value<std::string>(config.domain.shader_model))

                // Geometry.
                ("geom", "Full path to the HLSL file where the geometry shader is defined.", cxxopts::value<std::string>(config.geom.hlsl))
                ("geom-dxbc", "Full path to the geometry shader's compiled DXBC binary.", cxxopts::value<std::string>(config.geom.dxbc))
                ("geom-isa", "Full path to ISA disassembly output file.", cxxopts::value<std::string>(config.geom.isa))
                ("geom-stats", "Full path to resource usage output file.", cxxopts::value<std::string>(config.geom.stats))
                ("geom-entry", "Name of the entry point.", cxxopts::value<std::string>(config.geom.entry_point))
                ("geom-dxbc-dump", "Full path to output file for compiled geometry shader DXBC binary.",
                    cxxopts::value<std::string>(config.geom.dxbcOut))
                ("geom-dxbc-dis", "Full path to output file for compiled geometry shader DXBC disassembly.",
                    cxxopts::value<std::string>(config.geom.dxbc_disassembly))
                ("geom-target", "Shader model for geometry shader (e.g. \"gs_5_0\" or \"gs_5_1\".", cxxopts::value<std::string>(config.geom.shader_model))

                // Pixel.
                ("pixel", "Full path to the HLSL file where the pixel shader is defined.", cxxopts::value<std::string>(config.pixel.hlsl))
                ("pixel-dxbc", "Full path to the pixel shader's compiled DXBC binary.", cxxopts::value<std::string>(config.pixel.dxbc))
                ("pixel-isa", "Full path to ISA disassembly output file.", cxxopts::value<std::string>(config.pixel.isa))
                ("pixel-stats", "Full path to resource usage output file.", cxxopts::value<std::string>(config.pixel.stats))
                ("pixel-entry", "Name of the entry point.", cxxopts::value<std::string>(config.pixel.entry_point))
                ("pixel-dxbc-dump", "Full path to output file for compiled pixel shader DXBC binary.",
                    cxxopts::value<std::string>(config.pixel.dxbcOut))
                ("pixel-dxbc-dis", "Full path to output file for compiled pixel shader DXBC disassembly.",
                        cxxopts::value<std::string>(config.pixel.dxbc_disassembly))
                ("pixel-target", "Shader model for pixel shader (e.g. \"ps_5_0\" or \"ps_5_1\".", cxxopts::value<std::string>(config.pixel.shader_model))

                // DX12 graphics or compute.
                ("rs-macro", "The name of the RootSignature macro in the HLSL code. If specified, the root signature "
                    "would be compiled from the HLSL source. Use this if your shader does not include the "
                    "[RootSignature()] attribute but has a root signature macro defined in the source code.",
                    cxxopts::value<std::string>(config.rs_macro))
                ("rs-macro-version", "The version of the RootSignature macro specified through the rs-macro option. "
                 "By default, 'rootsig_1_1' would be assumed.",
                    cxxopts::value<std::string>(config.rs_version))
                ("rs-hlsl", "Full path to the HLSL file where the RootSignature macro is defined in. If there is only "
                 "a single hlsl input file, this option is not required.",
                    cxxopts::value<std::string>(config.rs_macro_file))
                ("rs-bin", "The full path to the serialized root signature to be used in the compilation process.",
                    cxxopts::value<std::string>(config.rs_serialized))
                ("pso", "The full path to the text file that describes the pipeline state.",
                    cxxopts::value<std::string>(config.rs_pso))

                // DXR options.
                ("mode", "Compilation mode (Pipeline or Shader).",
                        cxxopts::value<std::string>(config.dxr_mode))
                ("state-desc", "Full path to the DXR state description JSON file.",
                        cxxopts::value<std::string>(config.dxr_state_file))
                ("export", "The name of the export to target, could be a raygeneration shader "
                    "name in Pipeline mode or a shader name in Shader mode.",
                        cxxopts::value<std::string>(config.dxrExport))
                ("dxr-isa", "Full path to DXR ISA disassembly output file.",
                        cxxopts::value<std::string>(config.dxr_isa_output))
                ("dxr-stats", "Full path to DXR hardware usage statistics output file.",
                        cxxopts::value<std::string>(config.dxr_statistics_output))
                ("dxr-bin", "Full path to DXR pipeline binary output file.",
                        cxxopts::value<std::string>(config.dxr_binary_output))
                ("dxr-hlsl-mapping", "HLSL->DXIL mapping (optional).",
                        cxxopts::value<std::string>(config.dxr_hlsl_mapping))
                 ("output-metadata", "Full path to output metadata file.",
                        cxxopts::value<std::string>(config.output_metadata))
                 ("hlsl", "Full path to input HLSL file.",
                        cxxopts::value<std::string>(config.dxr_hlsl_input))

                // General options.
                ("include", "Additional include directories for the front-end compiler.",
                    cxxopts::value<std::vector<std::string>>(config.include_dirs))
                ("define", "Preprocessor definitions and macros to pass to the front-end compiler.",
                    cxxopts::value<std::vector<std::string>>(config.defines))
                ("debug-layer", "Enable the D3D12 debug layer.",
                        cxxopts::value<bool>(config.should_enable_debug_layer))
                ;

            // Parse command line.
            auto result = opts.parse(argc, argv);

            // Show the help menu and exit if needed.
            if (result.count("help"))
            {
                std::cout << opts.help() << std::endl;
                exit(0);
            }

            // Validate the input.
            bool is_input_valid = IsInputValid(config);
            assert(is_input_valid);
            if (is_input_valid)
            {
                // Check if this is a DXR or standard graphics or compute pipeline.
                bool is_dxr_pipeline = false;
                bool is_dxr_shader = false;
                bool is_dxr = IsDxrConfig(config, is_dxr_pipeline, is_dxr_shader);

                if (config.should_enable_debug_layer)
                {
                    // Enable debug layer.
                    const char* kStrInfoEnablingDebugLayer = "Enabling the D3D12 debug layer... ";
                    const char* kStrInfoEnablingDebugLayerSuccess = "success.";
                    const char* kStrInfoEnablingDebugLayerFailure = "\n Warning: failed to enable the D3D12 debug layer.";
                    std::cout << kStrInfoEnablingDebugLayer;
                    ComPtr<ID3D12Debug> debug_controller;
                    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug_controller))))
                    {
                        std::cout << kStrInfoEnablingDebugLayerSuccess << std::endl;
                        debug_controller->EnableDebugLayer();
                    }
                    else
                    {
                        std::cout << kStrInfoEnablingDebugLayerFailure << std::endl;
                    }
                }

                // Initialize the backend.
                // If the user wants to target a virtual target, convert to upper case.
                // Here we assume that the CLI passed the correct string. Input validation
                // should be performed at the CLI level.
                D3D_FEATURE_LEVEL feature_level = is_dxr ? D3D_FEATURE_LEVEL_12_0 : D3D_FEATURE_LEVEL_11_0;
                rga::rgDx12Frontend rgFrontend(feature_level);
                is_ok = rgFrontend.Init(is_dxr);
                assert(is_ok);
                if (is_ok)
                {
                    if (result.count("list"))
                    {
                        // List of supported target GPUs, as reported by the driver.
                        std::vector<std::string> virtual_gpus;

                        // Mapping to of virtual GPU to ID.
                        std::map<std::string, unsigned> vgpu_to_id;

                        // Get the supported targets.
                        is_ok = rgFrontend.GetSupportedTargets(virtual_gpus, vgpu_to_id);

                        if (!virtual_gpus.empty())
                        {
                            // Print the list of targets and exit if needed.
                            for (const std::string& target : virtual_gpus)
                            {
                                std::cout << target << "->" << vgpu_to_id[target] << std::endl;
                            }
                        }
                        else
                        {
                            // We failed to extract the list of targets and must abort.
                            std::cerr << kStrErrorTargetListFetchFailure << std::endl;
                        }
                    }
                    else
                    {
                        // Perform the compilation.
                        std::string error_msg;
                        bool is_compute_pipeline = !config.comp.hlsl.empty() || !config.comp.dxbc.empty();
                        if (is_compute_pipeline)
                        {
                            std::cout << kStrInfoCompilingComputePipeline << std::endl;
                            is_ok = rgFrontend.CompileComputePipeline(config, error_msg);
                        }
                        else
                        {
                            if (is_dxr)
                            {
#ifdef RGA_DXR_ENABLED
                                // DXR pipeline or shader.
                                if (is_dxr_pipeline)
                                {
                                    std::cout << kStrInfoCompilingDxrPipeline << std::endl;
                                }
                                else
                                {
                                    std::cout << kStrInfoCompilingDxrShader << std::endl;
                                }
                                is_ok = rgFrontend.CompileRayTracingPipeline(config, error_msg);
#endif
                            }
                            else
                            {
                                // Graphics pipeline.
                                std::cout << kStrInfoCompilingGraphicsPipeline << std::endl;
                                is_ok = rgFrontend.CompileGraphicsPipeline(config, error_msg);
                            }
                        }
                        assert(is_ok);

                        if (!error_msg.empty())
                        {
                            std::cerr << error_msg << std::endl;
                        }
                    }
                }
                else
                {
                    // Failed to initialize the backend.
                    std::cerr << kStrErrorDriverInitFailure << std::endl;
                }
            }
        }
        catch (const cxxopts::OptionException& e)
        {
            static const char* kStrErrorParsingOptions = "Error parsing options: ";
            std::cerr << kStrErrorParsingOptions << e.what() << std::endl;
            is_ok = false;
        }

        return (is_ok ? 0 : -1);
    }
}
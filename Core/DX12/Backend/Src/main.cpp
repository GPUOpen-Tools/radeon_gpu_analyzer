// D3D12.
#include <d3d12.h>

// C++.
#include <cassert>
#include <string>
#include <vector>

// cxxopts.
#include <cxxopts/include/cxxopts.hpp>

// Local.
#include <rgDx12Frontend.h>
#include <rgDx12StringConstants.h>

// Local.
#include <rgDx12DataTypes.h>

using namespace Microsoft::WRL;

// *** CONSTANTS - START ***

const char* STR_ERROR_MULTIPLE_INPUT_FILES_PER_STAGE_A = "Error: both HLSL and DXBC input file provided for ";
const char* STR_ERROR_MULTIPLE_INPUT_FILES_PER_STAGE_B = " stage. Only a single input file is valid per stage.";
const char* STR_ERROR_NO_INPUT_FILE_PROVIDED_A = "Error: no input file was provided for .";
const char* STR_ERROR_NO_INPUT_FILE_PROVIDED_B = " stage.";
const char* STR_ERROR_MIXED_ROOT_SIGNATURE_INPUT = "Error: cannot mix root signature inputs. The command instructs to both use serialized root "
"signature from file and extract root signature from hlsl code. Only one option is valid at a time.";
const char* STR_ERROR_TARGET_LIST_FETCH_FAILURE = "Error: failed to retrieve the list of supported targets.";
const char* STR_ERROR_DRIVER_INIT_FAILURE = "Error: driver initialization failed.";
const char* STR_ERROR_MIX_COMPUTE_AND_GRAPHICS = "Cannot mix and match compute shaders with non-compute shaders.";
const char* STR_INFO_COMPILING_COMPUTE_PIPELINE = "Compiling compute pipeline...";
const char* STR_INFO_COMPILING_GRAPHICS_PIPELINE = "Compiling graphics pipeline...";

// *** CONSTANTS - END ***

// Validates the inputs, prints error messages as
// necessary and also adjusts the configuration as necessary.
// Returns true if valid, otherwise returns false.
static bool IsInputValid(rga::rgDx12Config& config)
{
    bool ret = true;
    if (!config.shouldListTargets)
    {
        bool isComputePipeline = !config.comp.hlsl.empty() || !config.comp.dxbc.empty();
        if (isComputePipeline)
        {
            // Only a single input file is valid.
            if (!config.comp.hlsl.empty() && !config.comp.dxbc.empty())
            {
                std::cout << STR_ERROR_MULTIPLE_INPUT_FILES_PER_STAGE_A << "compute" <<
                    STR_ERROR_MULTIPLE_INPUT_FILES_PER_STAGE_B << std::endl;
                ret = false;
            }
            if (ret && config.comp.hlsl.empty() && config.comp.dxbc.empty())
            {
                std::cout << STR_ERROR_NO_INPUT_FILE_PROVIDED_A << "compute" <<
                    STR_ERROR_NO_INPUT_FILE_PROVIDED_B << std::endl;
                ret = false;
            }
        }
        else
        {
            // Vertex shader must be present.
            if (config.vert.hlsl.empty() && config.vert.dxbc.empty())
            {
                std::cout << STR_ERROR_NO_INPUT_FILE_PROVIDED_A << "vertex" <<
                    STR_ERROR_NO_INPUT_FILE_PROVIDED_B << std::endl;
                ret = false;
            }

            // Only a single input file is valid.
            if (ret && !config.vert.hlsl.empty() && !config.vert.dxbc.empty())
            {
                std::cout << STR_ERROR_MULTIPLE_INPUT_FILES_PER_STAGE_A << "vertex" <<
                    STR_ERROR_MULTIPLE_INPUT_FILES_PER_STAGE_B << std::endl;
                ret = false;
            }
            if (ret && !config.hull.hlsl.empty() && !config.hull.dxbc.empty())
            {
                std::cout << STR_ERROR_MULTIPLE_INPUT_FILES_PER_STAGE_A << "hull" <<
                    STR_ERROR_MULTIPLE_INPUT_FILES_PER_STAGE_B << std::endl;
                ret = false;
            }
            if (ret && !config.domain.hlsl.empty() && !config.domain.dxbc.empty())
            {
                std::cout << STR_ERROR_MULTIPLE_INPUT_FILES_PER_STAGE_A << "domain" <<
                    STR_ERROR_MULTIPLE_INPUT_FILES_PER_STAGE_B << std::endl;
                ret = false;
            }
            if (ret && !config.geom.hlsl.empty() && !config.geom.dxbc.empty())
            {
                std::cout << STR_ERROR_MULTIPLE_INPUT_FILES_PER_STAGE_A << "geometry" <<
                    STR_ERROR_MULTIPLE_INPUT_FILES_PER_STAGE_B << std::endl;
                ret = false;
            }
            if (ret && !config.pixel.hlsl.empty() && !config.pixel.dxbc.empty())
            {
                std::cout << STR_ERROR_MULTIPLE_INPUT_FILES_PER_STAGE_A << "pixel" <<
                    STR_ERROR_MULTIPLE_INPUT_FILES_PER_STAGE_B << std::endl;
                ret = false;
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
            std::cout << STR_ERROR_MIX_COMPUTE_AND_GRAPHICS << std::endl;
            ret = false;
        }

        // Do not mix root signature inputs.
        if (ret && !config.rsMacro.empty() && !config.rsSerialized.empty())
        {
            std::cout << STR_ERROR_MIXED_ROOT_SIGNATURE_INPUT << std::endl;
            ret = false;
        }

        if (ret)
        {
            // Set the root signature file if there is only a single input file.
            if (isComputePipeline)
            {
                if (config.rsMacroFile.empty() && !config.comp.hlsl.empty())
                {
                    config.rsMacroFile = config.comp.hlsl;
                }
            }
            else
            {
                // Vertex shader must be present. Check if there is no different shader in the pipeline.
                bool isSingleHlslFile =
                    (config.hull.hlsl.empty() || config.vert.hlsl.compare(config.hull.hlsl) == 0) &&
                    (config.domain.hlsl.empty() || config.vert.hlsl.compare(config.domain.hlsl) == 0) &&
                    (config.geom.hlsl.empty() || config.vert.hlsl.compare(config.geom.hlsl) == 0) &&
                    (config.pixel.hlsl.empty() || config.vert.hlsl.compare(config.pixel.hlsl) == 0);

                // If there is only a single hlsl input file, and the user did not explicitly provide
                // a file to read the root signature macro definition from, use that single hlsl input
                // file as the source for compiling the root signature.
                if (isSingleHlslFile && config.rsMacroFile.empty())
                {
                    config.rsMacroFile = config.vert.hlsl;
                }
            }
        }
    }
    return ret;
}

int main(int argc, char* argv[])
{
    assert(argv != nullptr);
    if (argv != nullptr)
    {
        try
        {
            // Keep user inputs in this structure.
            rga::rgDx12Config config;

            cxxopts::Options opts(argv[0]);
            opts.add_options()
                ("h,help", "Print help.")
                ("l, list", "List the names and IDs of the supported targets by the driver.", cxxopts::value<bool>(config.shouldListTargets))
                ("c, asic", "The target GPU ID for the pipeline compilation. If not specified, the pipeline would be compiled "
                    "for the physically installed GPU by default.", cxxopts::value<int>(config.targetGpu))

                // Pipeline binary.
                ("b", "Full path to the output file for the pipeline binary.", cxxopts::value<std::string>(config.pipelineBinary))

                // Compute.
                ("comp", "Full path to the HLSL file where the compute shader is defined.", cxxopts::value<std::string>(config.comp.hlsl))
                ("comp-dxbc", "Full path to the compute shader's compiled DXBC binary.", cxxopts::value<std::string>(config.comp.dxbc))
                ("comp-isa", "Full path to ISA disassembly output file.", cxxopts::value<std::string>(config.comp.isa))
                ("comp-stats", "Full path to resource usage output file.", cxxopts::value<std::string>(config.comp.stats))
                ("comp-entry", "Name of the entry point.", cxxopts::value<std::string>(config.comp.entryPoint))
                ("comp-dxbc-dump", "Full path to output file for compiled compute shader DXBC binary.",
                    cxxopts::value<std::string>(config.comp.dxbcOut))
                ("comp-dxbc-dis", "Full path to output file for compiled compute shader DXBC disassembly.",
                    cxxopts::value<std::string>(config.comp.dxbcDisassembly))
                ("comp-target", "Shader model for compute shader (e.g. \"cs_5_0\" or \"cs_5_1\".", cxxopts::value<std::string>(config.comp.shaderModel))

                // Vertex.
                ("vert", "Full path to the HLSL file where the vertex shader is defined.", cxxopts::value<std::string>(config.vert.hlsl))
                ("vert-dxbc", "Full path to the vertex shader's compiled DXBC binary.", cxxopts::value<std::string>(config.vert.dxbc))
                ("vert-isa", "Full path to ISA disassembly output file.", cxxopts::value<std::string>(config.vert.isa))
                ("vert-stats", "Full path to resource usage output file.", cxxopts::value<std::string>(config.vert.stats))
                ("vert-entry", "Name of the entry point.", cxxopts::value<std::string>(config.vert.entryPoint))
                ("vert-dxbc-dump", "Full path to output file for compiled vertex shader DXBC binary.",
                    cxxopts::value<std::string>(config.vert.dxbcOut))
                ("vert-dxbc-dis", "Full path to output file for compiled vertex shader DXBC disassembly.",
                    cxxopts::value<std::string>(config.vert.dxbcDisassembly))
                ("vert-target", "Shader model for vertex shader (e.g. \"vs_5_0\" or \"vs_5_1\".", cxxopts::value<std::string>(config.vert.shaderModel))

                // Hull.
                ("hull", "Full path to the HLSL file where the hull shader is defined.", cxxopts::value<std::string>(config.hull.hlsl))
                ("hull-dxbc", "Full path to the hull shader's compiled DXBC binary.", cxxopts::value<std::string>(config.hull.dxbc))
                ("hull-isa", "Full path to ISA disassembly output file.", cxxopts::value<std::string>(config.hull.isa))
                ("hull-stats", "Full path to resource usage output file.", cxxopts::value<std::string>(config.hull.stats))
                ("hull-entry", "Name of the entry point.", cxxopts::value<std::string>(config.hull.entryPoint))
                ("hull-dxbc-dump", "Full path to output file for compiled hull shader DXBC binary.",
                    cxxopts::value<std::string>(config.hull.dxbcOut))
                ("hull-dxbc-dis", "Full path to output file for compiled hull shader DXBC disassembly.",
                    cxxopts::value<std::string>(config.hull.dxbcDisassembly))
                ("hull-target", "Shader model for hull shader (e.g. \"hs_5_0\" or \"hs_5_1\".", cxxopts::value<std::string>(config.hull.shaderModel))

                // Domain.
                ("domain", "Full path to the HLSL file where the domain shader is defined.", cxxopts::value<std::string>(config.domain.hlsl))
                ("domain-dxbc", "Full path to the domain shader's compiled DXBC binary.", cxxopts::value<std::string>(config.domain.dxbc))
                ("domain-isa", "Full path to ISA disassembly output file.", cxxopts::value<std::string>(config.domain.isa))
                ("domain-stats", "Full path to resource usage output file.", cxxopts::value<std::string>(config.domain.stats))
                ("domain-entry", "Name of the entry point.", cxxopts::value<std::string>(config.domain.entryPoint))
                ("domain-dxbc-dump", "Full path to output file for compiled domain shader DXBC binary.",
                    cxxopts::value<std::string>(config.domain.dxbcOut))
                ("domain-dxbc-dis", "Full path to output file for compiled domain shader DXBC disassembly.",
                    cxxopts::value<std::string>(config.domain.dxbcDisassembly))
                ("domain-target", "Shader model for domain shader (e.g. \"ds_5_0\" or \"ds_5_1\".", cxxopts::value<std::string>(config.domain.shaderModel))

                // Geometry.
                ("geom", "Full path to the HLSL file where the geometry shader is defined.", cxxopts::value<std::string>(config.geom.hlsl))
                ("geom-dxbc", "Full path to the geometry shader's compiled DXBC binary.", cxxopts::value<std::string>(config.geom.dxbc))
                ("geom-isa", "Full path to ISA disassembly output file.", cxxopts::value<std::string>(config.geom.isa))
                ("geom-stats", "Full path to resource usage output file.", cxxopts::value<std::string>(config.geom.stats))
                ("geom-entry", "Name of the entry point.", cxxopts::value<std::string>(config.geom.entryPoint))
                ("geom-dxbc-dump", "Full path to output file for compiled geometry shader DXBC binary.",
                    cxxopts::value<std::string>(config.geom.dxbcOut))
                ("geom-dxbc-dis", "Full path to output file for compiled geometry shader DXBC disassembly.",
                    cxxopts::value<std::string>(config.geom.dxbcDisassembly))
                ("geom-target", "Shader model for geometry shader (e.g. \"gs_5_0\" or \"gs_5_1\".", cxxopts::value<std::string>(config.geom.shaderModel))

                // Pixel.
                ("pixel", "Full path to the HLSL file where the pixel shader is defined.", cxxopts::value<std::string>(config.pixel.hlsl))
                ("pixel-dxbc", "Full path to the pixel shader's compiled DXBC binary.", cxxopts::value<std::string>(config.pixel.dxbc))
                ("pixel-isa", "Full path to ISA disassembly output file.", cxxopts::value<std::string>(config.pixel.isa))
                ("pixel-stats", "Full path to resource usage output file.", cxxopts::value<std::string>(config.pixel.stats))
                ("pixel-entry", "Name of the entry point.", cxxopts::value<std::string>(config.pixel.entryPoint))
                ("pixel-dxbc-dump", "Full path to output file for compiled pixel shader DXBC binary.",
                    cxxopts::value<std::string>(config.pixel.dxbcOut))
                ("pixel-dxbc-dis", "Full path to output file for compiled pixel shader DXBC disassembly.",
                        cxxopts::value<std::string>(config.pixel.dxbcDisassembly))
                ("pixel-target", "Shader model for pixel shader (e.g. \"ps_5_0\" or \"ps_5_1\".", cxxopts::value<std::string>(config.pixel.shaderModel))

                ("include", "Additional include directories for the front-end compiler.",
                    cxxopts::value<std::vector<std::string>>(config.includeDirs))
                ("define", "Preprocessor definitions and macros to pass to the front-end compiler.",
                    cxxopts::value<std::vector<std::string>>(config.defines))

                ("rs-macro", "The name of the RootSignature macro in the HLSL code. If specified, the root signature "
                    "would be compiled from the HLSL source. Use this if your shader does not include the "
                    "[RootSignature()] attribute but has a root signature macro defined in the source code.",
                    cxxopts::value<std::string>(config.rsMacro))

                ("rs-macro-version", "The version of the RootSignature macro specified through the rs-macro option. "
                    "By default, 'rootsig_1_1' would be assumed.",
                    cxxopts::value<std::string>(config.rsVersion))

                ("rs-hlsl", "Full path to the HLSL file where the RootSignature macro is defined in. If there is only "
                    "a single hlsl input file, this option is not required.",
                    cxxopts::value<std::string>(config.rsMacroFile))

                ("rs-bin", "The full path to the serialized root signature to be used in the compilation process.",
                    cxxopts::value<std::string>(config.rsSerialized))

                ("pso", "The full path to the text file that describes the pipeline state.",
                        cxxopts::value<std::string>(config.rsPso))
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
            bool isInputValid = IsInputValid(config);
            assert(isInputValid);
            if (isInputValid)
            {
                // Initialize the backend.
                // If the user wants to target a virtual target, convert to upper case.
                // Here we assume that the CLI passed the correct string. Input validation
                // should be performed at the CLI level.
                rga::rgDx12Frontend rgFrontend;
                bool isOk = rgFrontend.Init();
                assert(isOk);
                if (isOk)
                {
                    if (isOk)
                    {
                        if (result.count("list"))
                        {
                            // List of supported target GPUs, as reported by the driver.
                            std::vector<std::string> virtualGpus;

                            // Mapping to of virtual GPU to ID.
                            std::map<std::string, unsigned> vgpuToId;

                            // Get the supported targets.
                            isOk = rgFrontend.GetSupportedTargets(virtualGpus, vgpuToId);

                            // Print the list of targets and exit if needed.
                            for (const std::string& target : virtualGpus)
                            {
                                std::cout << target << "->" << vgpuToId[target] << std::endl;
                            }
                        }
                        else
                        {
                            // Perform the compilation.
                            std::string errorMsg;

                            bool isComputePipeline = !config.comp.hlsl.empty() || !config.comp.dxbc.empty();
                            if (isComputePipeline)
                            {
                                std::cout << STR_INFO_COMPILING_COMPUTE_PIPELINE << std::endl;
                                isOk = rgFrontend.CompileComputePipeline(config, errorMsg);
                            }
                            else
                            {
                                std::cout << STR_INFO_COMPILING_GRAPHICS_PIPELINE << std::endl;
                                isOk = rgFrontend.CompileGraphicsPipeline(config, errorMsg);
                            }
                            assert(isOk);

                            if (!errorMsg.empty())
                            {
                                std::cerr << errorMsg << std::endl;
                            }
                        }
                    }
                    else
                    {
                        // We failed to extract the list of targets and must abort.
                        std::cerr << STR_ERROR_TARGET_LIST_FETCH_FAILURE << std::endl;
                    }
                }
                else
                {
                    // Failed to initialize the backend.
                    std::cerr << STR_ERROR_DRIVER_INIT_FAILURE << std::endl;
                }
            }
        }
        catch (const cxxopts::OptionException& e)
        {
            std::cerr << STR_ERR_PARSING_OPTIONS << e.what() << std::endl;
        }
        return 0;
    }
}
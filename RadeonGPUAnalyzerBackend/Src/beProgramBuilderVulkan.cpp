//=================================================================
// Copyright 2018 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

// C++
#include <cassert>
#include <stdlib.h>

// Infra.
#ifdef _WIN32
    #pragma warning(push)
    #pragma warning(disable:4309)
#endif
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#ifdef _WIN32
    #pragma warning(pop)
#endif

// Local.
#include <RadeonGPUAnalyzerBackend/Include/beProgramBuilderVulkan.h>
#include <RadeonGPUAnalyzerBackend/Include/beStringConstants.h>
#include <RadeonGPUAnalyzerBackend/Include/beDataTypes.h>
#include <RadeonGPUAnalyzerBackend/Include/beUtils.h>
#include <RadeonGPUAnalyzerCLI/Src/kcUtils.h>
#include <Utils/Include/rgLog.h>
#include <Utils/Include/rgaCliDefs.h>

using namespace beKA;

// Constants.

// Glslang option: output file.
static const std::string  STR_GLSLANG_OPT_OUTPUT       = "-o";

// Glslang option: SPIR-V binary output file.
static const std::string  STR_GLSLANG_OPT_SPIRV_OUTPUT = "-V";

// Glslang option: Input file is an HLSL shader.
static const std::string  STR_GLSLANG_OPT_HLSL_INPUT   = "-D";

// Glslang option: Macro (yes, it is the same as HLSL shader).
// For macros, the usage a different as an argument is being
// appended to the command line switch of glslang.
static const std::string  STR_GLSLANG_OPT_MACRO_DEFINE = "-D";

// Glslang option: explicitly specify shader stage.
static const std::string  STR_GLSLANG_OPT_SHADER_STAGE = "-S";

// Glslang option: Include directory.
static const std::string  STR_GLSLANG_OPT_INCLUDE_DIR = "-I";

// Glslang option: Preprocess input GLSL/HLSL file to stdout.
static const std::string  STR_GLSLANG_OPT_PREPROCESS   = "-E";

// Glslang option: Pipeline stage.
static const std::string  STR_GLSLANG_OPT_STAGE        = "-S";

// Glslang options: Pipeline stage names.
static const std::string  STR_GLSLANG_OPT_STAGE_VERT   = "vert";
static const std::string  STR_GLSLANG_OPT_STAGE_TESC   = "tesc";
static const std::string  STR_GLSLANG_OPT_STAGE_TESE   = "tese";
static const std::string  STR_GLSLANG_OPT_STAGE_GEOM   = "geom";
static const std::string  STR_GLSLANG_OPT_STAGE_FRAG   = "frag";
static const std::string  STR_GLSLANG_OPT_STAGE_COMP   = "comp";

// Info messages.
static const char* STR_LOADER_DEBUG_INFO_BEGIN = "*** Loader debug info - BEGIN ***";
static const char* STR_LOADER_DEBUG_INFO_END = "*** Loader debug info - END ***";

// A container for all valid glslang extensions for GLSL automatic stage detection.
static const vector<std::string> VALID_GLSLANG_GLSL_EXTENSIONS =
{
    STR_GLSLANG_OPT_STAGE_VERT,
    STR_GLSLANG_OPT_STAGE_TESC,
    STR_GLSLANG_OPT_STAGE_TESE,
    STR_GLSLANG_OPT_STAGE_GEOM,
    STR_GLSLANG_OPT_STAGE_FRAG,
    STR_GLSLANG_OPT_STAGE_COMP
};

// SPIR-V disassembler option: output file.
static const std::string  STR_SPV_DIS_OPT_OUTPUT       = "-o";

// VulkanBackend options: input spv files.
static const std::array<std::string, bePipelineStage::Count>
STR_VULKAN_BACKEND_OPT_STAGE_INPUT_FILE =
{
    "--vert",
    "--tesc",
    "--tese",
    "--geom",
    "--frag",
    "--comp"
};

// VulkanBackend options: output ISA disassembly files.
static const std::array<std::string, bePipelineStage::Count>
STR_VULKAN_BACKEND_OPT_STAGE_ISA_FILE =
{
    "--vert-isa",
    "--tesc-isa",
    "--tese-isa",
    "--geom-isa",
    "--frag-isa",
    "--comp-isa"
};

// VulkanBackend options: output statistics files.
static const std::array<std::string, bePipelineStage::Count>
STR_VULKAN_BACKEND_OPT_STAGE_STATS_FILE =
{
    "--vert-stats",
    "--tesc-stats",
    "--tese-stats",
    "--geom-stats",
    "--frag-stats",
    "--comp-stats"
};

// VulkanBackend options: output binary file.
static const std::string  STR_VULKAN_BACKEND_OPT_BIN_FILE = "--bin";

// VulkanBackend options: input pipeline object file.
static const std::string  STR_VULKAN_BACKEND_OPT_PSO_FILE = "--pso";

// VulkanBackend options: input alternative ICD path.
static const std::string  STR_VULKAN_BACKEND_OPT_ICD_PATH = "--icd";

// VulkanBackend options: value for VK_LOADER_DEBUG environment variable.
static const std::string  STR_VULKAN_BACKEND_OPT_VK_LOADER_DEBUG = "--loader-debug";

// VulkanBackend options: list target GPUs.
static const std::string  STR_VULKAN_BACKEND_OPT_LIST_TARGETS = "--list-targets";

// VulkanBackend options: list physical GPU adapters.
static const std::string  STR_VULKAN_BACKEND_OPT_LIST_ADAPTERS = "--list-adapters";

// VulkanBackend options: enable Vulkan validation layers.
static const std::string  STR_VULKAN_BACKEND_OPT_ENABLE_LAYERS = "--enable-layers";

// VulkanBackend options: path to the validation output text file.
static const std::string  STR_VULKAN_BACKEND_OPT_LAYERS_FILE = "--layers-file";

// VulkanBackend options: target GPU.
static const std::string  STR_VULKAN_BACKEND_OPT_TARGET = "--target";

// Copy the Vulkan Validation layers info from temp file ("tempInfoFile") to the Log file and user-provided validation info file ("outputFile").
// Delete the temp info file after copying its content.
static void CopyValidatioInfo(const std::string& tempInfoFile, const std::string& outputFile)
{
    static const char* STR_WRN_VULKAN_FAILED_EXTRACT_VALIDATION_INFO = "<stdout>";

    bool result = false;
    std::string info;
    if ((result = kcUtils::ReadTextFile(tempInfoFile, info, nullptr)) == true)
    {
        rgLog::file << info << std::endl;
        if (outputFile == KC_STR_VK_VALIDATION_INFO_STDOUT)
        {
            rgLog::stdOut << std::endl << info << std::endl;
        }
        else
        {
            result = kcUtils::WriteTextFile(outputFile, info, nullptr);
        }
    }

    if (!kcUtils::DeleteFile(tempInfoFile))
    {
        result = false;
    }

    if (!result)
    {
        rgLog::stdErr << STR_WRN_VULKAN_FAILED_EXTRACT_VALIDATION_INFO << std::endl;
    }
}

// Construct command line options for Vulkan Backend.
static std::string ConstructVulkanBackendOptions(const std::string& loaderDebug,
                                                 const beVkPipelineFiles& spvFiles,
                                                 const beVkPipelineFiles& isaFiles,
                                                 const beVkPipelineFiles& statsFiles,
                                                 const std::string& binFile,
                                                 const std::string& psoFile,
                                                 const std::string& icdFile,
                                                 const std::string& validationOutput,
                                                 const std::string& device)
{
    std::stringstream opts;

    // Add target option.
    if (!device.empty())
    {
        opts << STR_VULKAN_BACKEND_OPT_TARGET << " " << device;
    }

    // Add per-stage input & output files names.
    for (int stage = 0; stage < bePipelineStage::Count; stage++)
    {
        if (!spvFiles[stage].empty())
        {
            assert(!isaFiles[stage].empty() && !statsFiles[stage].empty());
            if (!isaFiles[stage].empty() && !statsFiles[stage].empty())
            {
                opts << " " << STR_VULKAN_BACKEND_OPT_STAGE_INPUT_FILE[stage] << " " << kcUtils::Quote(spvFiles[stage]) << " "
                            << STR_VULKAN_BACKEND_OPT_STAGE_ISA_FILE[stage]   << " " << kcUtils::Quote(isaFiles[stage]) << " "
                            << STR_VULKAN_BACKEND_OPT_STAGE_STATS_FILE[stage] << " " << kcUtils::Quote(statsFiles[stage]) << " ";
            }
        }
    }

    // Output binary file name.
    if (!binFile.empty())
    {
        opts << " " << STR_VULKAN_BACKEND_OPT_BIN_FILE << " " << kcUtils::Quote(binFile);
    }

    // Pipeline state file.
    if (!psoFile.empty())
    {
        opts << " " << STR_VULKAN_BACKEND_OPT_PSO_FILE << " " << kcUtils::Quote(psoFile);
    }

    // Alternative ICD full path.
    if (!icdFile.empty())
    {
        opts << " " << STR_VULKAN_BACKEND_OPT_ICD_PATH << " " << kcUtils::Quote(icdFile);
    }

    // Value for VK_LOADER_DEBUG.
    if (!loaderDebug.empty())
    {
        opts << " " << STR_VULKAN_BACKEND_OPT_VK_LOADER_DEBUG << " " << kcUtils::Quote(loaderDebug);
    }

    // Add valdiation enabling option if required.
    if (!validationOutput.empty())
    {
        opts << " " << STR_VULKAN_BACKEND_OPT_ENABLE_LAYERS;
        opts << " " << STR_VULKAN_BACKEND_OPT_LAYERS_FILE << " " << validationOutput;
    }

    return opts.str();
}

// Construct command line options for SPIR-V disassembler.
static std::string ConstructSpvDisOptions(const std::string& spvFileName, const std::string& spvDisFileName)
{
    std::stringstream opts;

    if (!spvFileName.empty())
    {
        if (!spvDisFileName.empty())
        {
            opts << STR_SPV_DIS_OPT_OUTPUT << " " << kcUtils::Quote(spvDisFileName);
        }
        opts << " " << kcUtils::Quote(spvFileName);
    }

    return opts.str();
}

// Construct command line options for SPIR-V assembler.
static std::string ConstructSpvAsmOptions(const std::string& spvTxtFileName, const std::string& spvFileName)
{
    std::stringstream opts;

    if (!spvTxtFileName.empty())
    {
        if (!spvFileName.empty())
        {
            opts << STR_SPV_DIS_OPT_OUTPUT << " " << kcUtils::Quote(spvFileName);
        }
        opts << " " << kcUtils::Quote(spvTxtFileName);
    }

    return opts.str();
}

// Construct command line options for Glslang compiler.
static std::string ConstructGlslangOptions(const Config& config, const std::string& srcFileName,
    const std::string& spvFileName, size_t stage, bool isHlsl, bool isPreprocess = false)
{
    std::stringstream opts;

    // Append any additional options from the user.
    if (!config.m_glslangOpt.empty())
    {
        // Unwrap the argument from the token.
        std::string fixedOptions = config.m_glslangOpt;
        fixedOptions.erase(std::remove(fixedOptions.begin(),
            fixedOptions.end(), CLI_OPT_GLSLANG_TOKEN), fixedOptions.end());
        opts << fixedOptions;
    }

    if (isHlsl)
    {
        opts << " " << STR_GLSLANG_OPT_HLSL_INPUT;
    }

    if (isPreprocess)
    {
        opts << " " << STR_GLSLANG_OPT_PREPROCESS;

        // Glslang preprocessor requires specifying the pipeline stage for some reason.
        // Always use "vert" since preprocessor is stage-agnostic.
        opts << " " << STR_GLSLANG_OPT_STAGE << " " << STR_GLSLANG_OPT_STAGE_VERT;
    }
    else
    {
        // Add the switches for any given include paths.
        if (!config.m_IncludePath.empty())
        {
            for (const std::string& includePath : config.m_IncludePath)
            {
                opts << " " << STR_GLSLANG_OPT_INCLUDE_DIR << includePath << " ";
            }
        }

        // Add the switches for any given macro/define directives..
        if (!config.m_Defines.empty())
        {
            for (const std::string& givenMacro : config.m_Defines)
            {
                opts << " " << STR_GLSLANG_OPT_MACRO_DEFINE << givenMacro << " ";
            }
        }

        // Get the file extension.
        std::string ext = beUtils::GetFileExtension(srcFileName);

        // If the file extension is not one of the "default" file extensions:
        // vert, tesc, tese, geom, frag, comp, we need to explicitly specify for glslang the stage.
        if (std::find(VALID_GLSLANG_GLSL_EXTENSIONS.cbegin(),
            VALID_GLSLANG_GLSL_EXTENSIONS.cend(), ext) == VALID_GLSLANG_GLSL_EXTENSIONS.cend())
        {
            assert(stage < VALID_GLSLANG_GLSL_EXTENSIONS.size());
            if(stage < VALID_GLSLANG_GLSL_EXTENSIONS.size())
            {
                opts << " " << STR_GLSLANG_OPT_SHADER_STAGE << " " <<
                    VALID_GLSLANG_GLSL_EXTENSIONS[stage] << " ";
            }
        }

        opts << " " << STR_GLSLANG_OPT_SPIRV_OUTPUT;
        opts << " " << STR_GLSLANG_OPT_OUTPUT << " " << kcUtils::Quote(spvFileName);
    }

    opts << " " << kcUtils::Quote(srcFileName);

    return opts.str();
}

// Check if ISA disassembly and statistics files are not empty for corresponding input spv files.
static bool VerifyOutputFiles(const beVkPipelineFiles& spvFiles,
                              const beVkPipelineFiles& isaFiles,
                              const beVkPipelineFiles& statsFiles)
{
    bool result = true;

    for (int stage = 0; stage < bePipelineStage::Count; stage++)
    {
        if (!spvFiles[stage].empty())
        {
            if (!kcUtils::FileNotEmpty(isaFiles[stage]) || !kcUtils::FileNotEmpty(statsFiles[stage]))
            {
                result = false;
                break;
            }
        }
    }

    return result;
}

beKA::beStatus beProgramBuilderVulkan::GetVulkanDriverTargetGPUs(const std::string& loaderDebug, const std::string& icdFile, std::set<std::string>& targetGPUs,
                                                           bool printCmd, std::string& errText)
{
    std::string stdOutText, stdErrText;
    std::string opts = STR_VULKAN_BACKEND_OPT_LIST_TARGETS;

    // Alternative ICD library path.
    if (!icdFile.empty())
    {
        opts += (" " + STR_VULKAN_BACKEND_OPT_ICD_PATH + " " + kcUtils::Quote(icdFile));
    }

    // Launch the VulkanBacked and get the list of devices.
    beStatus status = InvokeVulkanBackend(opts, printCmd, stdOutText, stdErrText);

    // If the VulkanBackend provided any warning/errors, display them.
    if (!stdErrText.empty())
    {
        rgLog::stdOut << stdErrText << std::endl;
    }

    // Process the target list.
    // The format of device list returned by VulkanBackend:
    // BONAIRE:gfx700
    // CARRIZO:gfx801
    // FIJI:gfx803:gfx804
    // ...

    if (status == beStatus_SUCCESS)
    {
        size_t start = 0, end = 0;
        std::string devicesStr = kcUtils::ToLower(stdOutText);

        while (start < devicesStr.size() && (end = devicesStr.find_first_of(":\n", start)) != std::string::npos)
        {
            targetGPUs.insert(devicesStr.substr(start, end - start));
            start = end + 1;
        }
    }

    return status;
}

beStatus beProgramBuilderVulkan::GetPhysicalGPUs(const std::string& icdFile, std::vector<beVkPhysAdapterInfo>& gpuInfo,
                                                 bool printCmd, std::string& errText)
{
    // The format of physical adapter list returned by VulkanBackend:
    //
    // Adapter 0:
    //     Name: Radeon(TM) RX 480 Graphics
    //     Vulkan driver version : 2.0.0
    //     Supported Vulkan API version : 1.1.77
    // ...

    std::string stdOutText, stdErrText;
    std::string opts = STR_VULKAN_BACKEND_OPT_LIST_ADAPTERS;

    // Alternative ICD library path.
    if (!icdFile.empty())
    {
        opts += (" " + STR_VULKAN_BACKEND_OPT_ICD_PATH + " " + kcUtils::Quote(icdFile));
    }

    // Invoke "VulkanBackend --list-adapters" to get the list of physical adapters installed on the system.
    beStatus status = InvokeVulkanBackend(opts, printCmd, stdOutText, stdErrText);

    if (status == beStatus_SUCCESS)
    {
        errText = stdErrText;
        std::stringstream outStream(stdOutText);
        std::string line;
        uint32_t id = 0;
        size_t offset = 0;

        // Parse the VulkanBackend output.
        while (std::getline(outStream, line))
        {
            beVkPhysAdapterInfo info;

            if (line.find(CLI_VK_BACKEND_STR_ADAPTER) == 0)
            {
                info.id = id++;
                if (std::getline(outStream, line) && (offset = line.find(CLI_VK_BACKEND_STR_ADAPTER_NAME)) != std::string::npos)
                {
                    info.name = line.substr(offset + CLI_VK_BACKEND_STR_ADAPTER_NAME.size());
                }
                if (std::getline(outStream, line) && (offset = line.find(CLI_VK_BACKEND_STR_ADAPTER_DRIVER)) != std::string::npos)
                {
                    info.vkDriverVersion = line.substr(offset + CLI_VK_BACKEND_STR_ADAPTER_DRIVER.size());
                }
                if (std::getline(outStream, line) && (offset = line.find(CLI_VK_BACKEND_STR_ADAPTER_VULKAN)) != std::string::npos)
                {
                    info.vkAPIVersion = line.substr(offset + CLI_VK_BACKEND_STR_ADAPTER_VULKAN.size());
                }
                gpuInfo.push_back(info);
            }
        }
    }

    return status;
}

beKA::beStatus beProgramBuilderVulkan::CompileSrcToSpirvBinary(const Config& config,
                                                         const std::string& srcFile,
                                                         const std::string& spvFile,
                                                         bePipelineStage stage,
                                                         bool isHlsl,
                                                         std::string& errText)
{
    beStatus status = beStatus_Vulkan_EmptyInputFile;
    std::string outText;

    if (!srcFile.empty())
    {
        std::string glslangOpts = ConstructGlslangOptions(config, srcFile, spvFile, stage, isHlsl);
        assert(!glslangOpts.empty());
        if (!glslangOpts.empty())
        {
            status = InvokeGlslang(config.m_cmplrBinPath, glslangOpts,
                config.m_printProcessCmdLines, outText, errText);
        }
    }

    return status;
}

beStatus beProgramBuilderVulkan::InvokeGlslang(const std::string& glslangBinDir, const std::string& cmdLineOptions,
                                               bool printCmd, std::string& outText, std::string& errText)
{
    osFilePath  glslangExec;
    long        exitCode = 0;

    // Use the glslang folder provided by user if it's not empty.
    // Otherwise, use the default location.
    if (!glslangBinDir.empty())
    {
        gtString  binFolder;
        binFolder << glslangBinDir.c_str();
        glslangExec.setFileDirectory(binFolder);
    }
    else
    {
        osGetCurrentApplicationPath(glslangExec, false);
        glslangExec.appendSubDirectory(GLSLANG_ROOT_DIR);
        glslangExec.appendSubDirectory(GLSLANG_BIN_DIR);
    }
    glslangExec.setFileName(GLSLANG_EXEC);
    glslangExec.setFileExtension(GLSLANG_EXEC_EXT);

    // Clear the error message buffer.
    errText.clear();

    kcUtils::ProcessStatus  status = kcUtils::LaunchProcess(glslangExec.asString().asASCIICharArray(),
                                                            cmdLineOptions,
                                                            "",
                                                            PROCESS_WAIT_INFINITE,
                                                            printCmd,
                                                            outText,
                                                            errText,
                                                            exitCode);

    // If the output was streamed to stdout, grab it from there.
    if (errText.empty() && !outText.empty())
    {
        errText = outText;
    }

    return (status == kcUtils::ProcessStatus::Success ? beStatus_SUCCESS : beStatus_Vulkan_GlslangLaunchFailed);
}

beStatus beProgramBuilderVulkan::InvokeSpvTool(beSpvTool tool, const std::string& spvToolsBinDir, const std::string& cmdLineOptions,
                                               bool printCmd, std::string& outMsg, std::string& errMsg)
{
    osFilePath  spvDisExec;
    long        exitCode = 0;

    if (!spvToolsBinDir.empty())
    {
        gtString  binFolder;
        binFolder << spvToolsBinDir.c_str();
        spvDisExec.setFileDirectory(binFolder);
    }
    else
    {
        osGetCurrentApplicationPath(spvDisExec, false);
        spvDisExec.appendSubDirectory(GLSLANG_ROOT_DIR);
        spvDisExec.appendSubDirectory(GLSLANG_BIN_DIR);
    }

    const gtString spvToolExecName = ( tool == beSpvTool::Assembler ? SPIRV_AS_EXEC :
                                       tool == beSpvTool::Disassembler ? SPIRV_DIS_EXEC :
                                       L"");

    spvDisExec.setFileName(spvToolExecName);
    spvDisExec.setFileExtension(VULKAN_BACKEND_EXEC_EXT);

    kcUtils::ProcessStatus  status = kcUtils::LaunchProcess(spvDisExec.asString().asASCIICharArray(),
                                                            cmdLineOptions,
                                                            "",
                                                            PROCESS_WAIT_INFINITE,
                                                            printCmd,
                                                            outMsg,
                                                            errMsg,
                                                            exitCode);

    return (status == kcUtils::ProcessStatus::Success ? beStatus_SUCCESS : beStatus_Vulkan_SpvToolLaunchFailed);
}

beStatus beProgramBuilderVulkan::InvokeVulkanBackend(const std::string& cmdLineOptions, bool printCmd,
                                                     std::string& outText, std::string& errText)
{
    osFilePath  vkBackendExec;
    long        exitCode = 0;

    // Construct the path to the VulkanBackend executable.
    osGetCurrentApplicationPath(vkBackendExec, false);
    vkBackendExec.appendSubDirectory(VULKAN_BACKEND_ROOT_DIR);
    vkBackendExec.appendSubDirectory(VULKAN_BACKEND_BIN_DIR);
    vkBackendExec.setFileName(VULKAN_BACKEND_EXEC);
    vkBackendExec.setFileExtension(VULKAN_BACKEND_EXEC_EXT);

    kcUtils::ProcessStatus  status = kcUtils::LaunchProcess(vkBackendExec.asString().asASCIICharArray(),
                                                            cmdLineOptions,
                                                            "",
                                                            PROCESS_WAIT_INFINITE,
                                                            printCmd,
                                                            outText,
                                                            errText,
                                                            exitCode);

    return (status == kcUtils::ProcessStatus::Success ? beStatus_SUCCESS : beStatus_Vulkan_BackendLaunchFailed);
}

beKA::beStatus beProgramBuilderVulkan::CompileSpirv(const std::string& loaderDebug,
                                                    const beVkPipelineFiles& spirvFiles,
                                                    const beVkPipelineFiles& isaFiles,
                                                    const beVkPipelineFiles& statsFiles,
                                                    const std::string& binFile,
                                                    const std::string& psoFile,
                                                    const std::string& icdFile,
                                                    const std::string& validationOutput,
                                                    const std::string& validationOutputRedirection,
                                                    const std::string& device,
                                                    bool printCmd,
                                                    std::string& errMsg)
{
    beStatus  status = beStatus_Vulkan_BackendLaunchFailed;
    std::string stdOutText, stdErrText;

    // Construct the command for invoking the Vulkan backend.
    std::string opts = ConstructVulkanBackendOptions(loaderDebug, spirvFiles, isaFiles, statsFiles, binFile, psoFile, icdFile, validationOutput, device);
    assert(!opts.empty());
    if (!opts.empty())
    {
        status = InvokeVulkanBackend(opts, printCmd, stdOutText, stdErrText);

        // Check if some output files have not been generated for some reason.
        if (status == beStatus_SUCCESS && !VerifyOutputFiles(spirvFiles, isaFiles, statsFiles))
        {
            status = beStatus_Vulkan_BackendCompileFailed;
            errMsg = stdErrText;
        }
        else if (!loaderDebug.empty())
        {
            rgLog::stdOut << std::endl << STR_LOADER_DEBUG_INFO_BEGIN << std::endl <<
                std::endl << stdErrText << std::endl << STR_LOADER_DEBUG_INFO_END << std::endl;
        }

        // Print the Vulkan backend's output.
        if (!stdOutText.empty())
        {
            rgLog::stdOut << std::endl << stdOutText << std::endl;
        }

        // Dump the Vulkan validation info to the output file/stdout and log file.
        if (!validationOutputRedirection.empty())
        {
            CopyValidatioInfo(validationOutput, validationOutputRedirection);
        }
    }

    return status;
}

beStatus beProgramBuilderVulkan::DisassembleSpv(const std::string& spvToolsBinDir, const std::string& spvFilePath,
                                                const std::string& spvDisFilePath, bool printCmd, std::string& errMsg)
{
    beStatus status = beStatus_Vulkan_SpvDisasmFailed;
    const std::string& opts = ConstructSpvDisOptions(spvFilePath, spvDisFilePath);
    if (!opts.empty())
    {
        std::string outMsg;
        status = InvokeSpvTool(beSpvTool::Disassembler, spvToolsBinDir, opts, printCmd, outMsg, errMsg);

        // Check if the spv-dis has generated expected output file.
        if (status == beStatus_SUCCESS)
        {
            // Dump disassembly output to the stdout if no output file name is specified.
            if (spvDisFilePath.empty())
            {
                rgLog::stdOut << outMsg << std::endl;
            }
            else if (!kcUtils::FileNotEmpty(spvDisFilePath))
            {
                status = beStatus_Vulkan_SpvDisasmFailed;
            }
        }
    }

    return status;
}

beStatus beProgramBuilderVulkan::AssembleSpv(const std::string& spvToolsBinDir, const std::string& spvTxtFilePath,
                                             const std::string& spvFilePath, bool printCmd, std::string& errMsg)
{
    beStatus status = beStatus_Vulkan_SpvAsmFailed;
    const std::string& opts = ConstructSpvAsmOptions(spvTxtFilePath, spvFilePath);
    if (!opts.empty())
    {
        std::string outMsg;
        status = InvokeSpvTool(beSpvTool::Assembler, spvToolsBinDir, opts, printCmd, outMsg, errMsg);

        // Check if the assembler has generated expected output file.
        if (status == beStatus_SUCCESS && !kcUtils::FileNotEmpty(spvFilePath))
        {
            status = beStatus_Vulkan_SpvAsmFailed;
        }
    }

    return status;
}

beKA::beStatus beProgramBuilderVulkan::PreprocessSource(const Config& config, const std::string& glslangBinDir, const std::string& inputFile,
                                                  bool isHlsl, bool printCmd, std::string& output, std::string& errMsg)
{
    beStatus status = beStatus_Vulkan_PreprocessFailed;
    const std::string& opts = ConstructGlslangOptions(config, inputFile, "", bePipelineStage::Count, isHlsl, true);
    assert(!opts.empty());
    if (!opts.empty())
    {
        status = InvokeGlslang(glslangBinDir, opts, printCmd, output, errMsg);
    }

    return status;
}

// C++.
#include <sstream>
#include <cassert>

// Infra.
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osProcess.h>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/rgCliLauncher.h>
#include <RadeonGPUAnalyzerGUI/Include/rgCliUtils.h>
#include <RadeonGPUAnalyzerGUI/Include/rgStringConstants.h>
#include <RadeonGPUAnalyzerGUI/Include/rgUtils.h>
#include <RadeonGPUAnalyzerGUI/Include/rgXMLSessionConfig.h>

// OpenCL includes.
#include <RadeonGPUAnalyzerGUI/Include/rgCliKernelListParser.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDataTypesOpenCL.h>

// Vulkan includes.
#include <RadeonGPUAnalyzerGUI/Include/rgDataTypesVulkan.h>
#include <RadeonGPUAnalyzerGUI/Include/rgUtilsVulkan.h>

// Common between the CLI and the GUI.
#include <RadeonGPUAnalyzerGUI/../Utils/Include/rgLog.h>
#include <RadeonGPUAnalyzerGUI/../Utils/Include/rgaCliDefs.h>

void BuildRgaExecutableCommandString(std::stringstream& commandStream)
{
    // Add the RGA executable name to invoke, and a space.
    commandStream << STR_EXECUTABLE_NAME;
    commandStream << " ";
}

// A helper function responsible for building a base command string for CLI invocation.
void BuildCompileProjectCommandString(std::stringstream& commandStream, const std::string& outputPath)
{
    BuildRgaExecutableCommandString(commandStream);

    // Add the current build mode to the command.
    const std::string& currentMode = rgConfigManager::Instance().GetCurrentModeString();
    commandStream << CLI_OPT_INPUT_TYPE << " " << currentMode << " ";

    // ISA disassembly in text and CSV formats.
    commandStream << CLI_OPT_ISA << " \"" << outputPath << "disassem.txt\" " << CLI_OPT_PARSE_ISA << " ";

    // Include line numbers in the CSV file.
    commandStream << CLI_OPT_LINE_NUMBERS << " ";

    // Add the output path for the resource usage analysis file.
    commandStream << CLI_OPT_STATISTICS << " \"" << outputPath << STR_RESOURCE_USAGE_CSV_FILENAME << "\" ";

    // Binary.
    commandStream << CLI_OPT_BINARY << " \"" << outputPath << "codeobj.bin\" ";

    // CLI log file path.
    const std::string& cliLogFilePath = rgConfigManager::Instance().GetCLILogFilePath();
    if (!cliLogFilePath.empty())
    {
        commandStream << CLI_OPT_LOG << " \"" << cliLogFilePath << "\" ";
    }
}

void BuildOutputViewCommandHeader(std::shared_ptr<rgProject> pProject, const std::string& targetGpu, std::string& invocationText)
{
    std::stringstream textStream;

    // Build an output line for each CLI invocation. Print the command string that's about to be invoked.
    textStream << STR_OUTPUT_WINDOW_BUILDING_PROJECT_HEADER_A;

    // Include the project's API in the build header line.
    std::string projectApiString;
    bool isOk = rgUtils::ProjectAPIToString(pProject->m_api, projectApiString);
    assert(isOk);
    if (isOk)
    {
        textStream << projectApiString;
    }

    // Also include the project name and target GPU.
    textStream << STR_OUTPUT_WINDOW_BUILDING_PROJECT_HEADER_B;
    textStream << pProject->m_projectName;
    textStream << STR_OUTPUT_WINDOW_BUILDING_PROJECT_HEADER_C;
    textStream << targetGpu;

    // Insert a separator line above and below the build output line.
    std::string buildHeaderString = textStream.str();
    int numDashesToInsert = static_cast<int>(buildHeaderString.length());
    std::string dashedLines = std::string(numDashesToInsert, '-');

    // Surround the project build header text with dashed line separators.
    std::stringstream cmdLineOutputStream;
    cmdLineOutputStream << dashedLines << std::endl;
    cmdLineOutputStream << buildHeaderString << std::endl;
    cmdLineOutputStream << dashedLines << std::endl;

    invocationText = cmdLineOutputStream.str();
}

bool rgCliLauncher::BuildProjectCloneOpenCL(std::shared_ptr<rgProject> pProject, int cloneIndex, const std::string& outputPath,
    std::function<void(const std::string&)> cliOutputHandlingCallback, std::vector<std::string>& gpusBuilt, bool& cancelSignal)
{
    bool ret = false;
    if (pProject != nullptr)
    {
        rgLog::file << STR_LOG_BUILDING_PROJECT_CLONE_1 << pProject->m_projectName << STR_LOG_BUILDING_PROJECT_CLONE_2 << cloneIndex << std::endl;

        // A stream of all text collected from CLI invocation.
        std::stringstream fullCliOutput;

        // Verify that the clone index is valid for the given project.
        int numClones = static_cast<int>(pProject->m_clones.size());
        bool isCloneIndexValid = (cloneIndex >= 0 && cloneIndex < numClones);
        assert(isCloneIndexValid);
        if (isCloneIndexValid)
        {
            std::shared_ptr<rgProjectClone> pTargetClone = pProject->m_clones[cloneIndex];

            // Generate the command line invocation command.
            std::stringstream cmd;

            // Build the compile project command string.
            BuildCompileProjectCommandString(cmd, outputPath);

            // Build settings.
            std::string buildSettings;
            std::shared_ptr<rgBuildSettingsOpenCL> pClBuildSettings =
                std::static_pointer_cast<rgBuildSettingsOpenCL>(pTargetClone->m_pBuildSettings);
            assert(pClBuildSettings != nullptr);
            ret = rgCliUtils::GenerateOpenClBuildSettingsString(*pClBuildSettings, buildSettings);
            assert(ret);
            if (!buildSettings.empty())
            {
                cmd << buildSettings << " ";
            }

            // Execute the CLI for each target GPU, appending the GPU to the command.
            for (const std::string& targetGpu : pTargetClone->m_pBuildSettings->m_targetGpus)
            {
                if (!cancelSignal)
                {
                    // Print the command string that's about to be used to invoke the RGA CLI build process.
                    std::string cliInvocationCommandString;
                    BuildOutputViewCommandHeader(pProject, targetGpu, cliInvocationCommandString);

                    // Append the command string header text to the output string.
                    std::stringstream cmdLineOutputStream;
                    cmdLineOutputStream << cliInvocationCommandString;

                    // Construct the full CLI command string including the current target GPU.
                    std::stringstream fullCmdWithGpu;
                    fullCmdWithGpu << cmd.str();

                    // Specify the Metadata file path.
                    fullCmdWithGpu << CLI_OPT_SESSION_METADATA << " \"" << outputPath << targetGpu << "_" << STR_SESSION_METADATA_FILENAME << "\" ";

                    // Specify which GPU to build outputs for.
                    fullCmdWithGpu << CLI_OPT_ASIC << " " << targetGpu << " ";

                    // Append each input file to the end of the CLI command.
                    for (const rgSourceFileInfo& fileInfo : pTargetClone->m_sourceFiles)
                    {
                        // Surround the path to the input file with quotes to prevent breaking the CLI parser.
                        fullCmdWithGpu << "\"";
                        fullCmdWithGpu << fileInfo.m_filePath;
                        fullCmdWithGpu << "\" ";
                    }

                    // Add the full CLI execution string to the output window's log.
                    cmdLineOutputStream << fullCmdWithGpu.str();

                    // Send the new output text to the output window.
                    if (cliOutputHandlingCallback != nullptr)
                    {
                        cliOutputHandlingCallback(cmdLineOutputStream.str());
                    }

                    // Execute the command and grab the output.
                    rgLog::file << STR_LOG_LAUNCHING_CLI << rgLog::noflush << std::endl << fullCmdWithGpu.str() << std::endl << rgLog::flush;
                    gtString cmdLineOutputAsGtStr;
                    ret = osExecAndGrabOutput(fullCmdWithGpu.str().c_str(), cancelSignal, cmdLineOutputAsGtStr);

                    assert(ret);
                    if (ret)
                    {
                        // Add the GPU to the output list if it was built successfully.
                        gpusBuilt.push_back(targetGpu);
                    }

                    // Append the CLI's output to the string containing the entire execution output.
                    fullCliOutput << cmdLineOutputAsGtStr.asASCIICharArray();

                    // Invoke the callback used to send new CLI output to the GUI.
                    if (cliOutputHandlingCallback != nullptr)
                    {
                        cliOutputHandlingCallback(cmdLineOutputAsGtStr.asASCIICharArray());
                    }
                }
                else
                {
                    // Stop the process if build is canceled.
                    break;
                }
            }
        }
        else
        {
            // Invoke the callback used to send new CLI output to the GUI.
            if (cliOutputHandlingCallback != nullptr)
            {
                std::stringstream errorStream;
                errorStream << STR_OUTPUT_WINDOW_BUILDING_PROJECT_FAILED_TEXT;
                errorStream << STR_OUTPUT_WINDOW_BUILDING_PROJECT_FAILED_INVALID_CLONE;
                cliOutputHandlingCallback(errorStream.str());
            }
        }
    }

    return ret;
}

bool rgCliLauncher::BuildProjectCloneVulkan(std::shared_ptr<rgProject> pProject, int cloneIndex, const std::string& outputPath,
    std::function<void(const std::string&)> cliOutputHandlingCallback, std::vector<std::string>& gpusBuilt, bool& cancelSignal)
{
    bool ret = false;
    if (pProject != nullptr)
    {
        rgLog::file << STR_LOG_BUILDING_PROJECT_CLONE_1 << pProject->m_projectName << STR_LOG_BUILDING_PROJECT_CLONE_2 << cloneIndex << std::endl;

        // A stream of all text collected from CLI invocation.
        std::stringstream fullCliOutput;

        // Verify that the clone index is valid for the given project.
        int numClones = static_cast<int>(pProject->m_clones.size());
        bool isCloneIndexValid = (cloneIndex >= 0 && cloneIndex < numClones);
        assert(isCloneIndexValid);
        if (isCloneIndexValid)
        {
            std::shared_ptr<rgProjectClone> pTargetClone = pProject->m_clones[cloneIndex];
            assert(pTargetClone != nullptr);
            if (pTargetClone != nullptr)
            {
                std::shared_ptr<rgProjectCloneVulkan> pVulkanClone = std::dynamic_pointer_cast<rgProjectCloneVulkan>(pTargetClone);
                assert(pVulkanClone != nullptr);
                if (pVulkanClone != nullptr)
                {
                    // Generate the command line invocation command.
                    std::stringstream cmd;

                    // Build the compile project command string.
                    BuildCompileProjectCommandString(cmd, outputPath);

                    // Build settings.
                    std::string buildSettings;
                    std::shared_ptr<rgBuildSettingsVulkan> pBuildSettingsVulkan =
                        std::static_pointer_cast<rgBuildSettingsVulkan>(pTargetClone->m_pBuildSettings);
                    assert(pBuildSettingsVulkan != nullptr);
                    ret = rgCliUtils::GenerateVulkanBuildSettingsString(*pBuildSettingsVulkan, buildSettings);
                    assert(ret);
                    if (!buildSettings.empty())
                    {
                        cmd << buildSettings << " ";
                    }

                    std::shared_ptr<rgUtilsVulkan> pVulkanUtil = std::dynamic_pointer_cast<rgUtilsVulkan>(rgUtilsGraphics::CreateUtility(rgProjectAPI::Vulkan));
                    assert(pVulkanUtil != nullptr);
                    if (pVulkanUtil != nullptr)
                    {
                        // Execute the CLI for each target GPU, appending the GPU to the command.
                        for (const std::string& targetGpu : pTargetClone->m_pBuildSettings->m_targetGpus)
                        {
                            if (!cancelSignal)
                            {
                                // Print the command string that's about to be used to invoke the RGA CLI build process.
                                std::string cliInvocationCommandString;
                                BuildOutputViewCommandHeader(pProject, targetGpu, cliInvocationCommandString);

                                // Append the command string header text to the output string.
                                std::stringstream cmdLineOutputStream;
                                cmdLineOutputStream << cliInvocationCommandString;

                                // Construct the full CLI command string including the current target GPU.
                                std::stringstream fullCmdWithGpu;
                                fullCmdWithGpu << cmd.str();

                                // Specify the Metadata file path.
                                fullCmdWithGpu << CLI_OPT_SESSION_METADATA << " \"" << outputPath << targetGpu << "_" << STR_SESSION_METADATA_FILENAME << "\" ";

                                // Specify which GPU to build outputs for.
                                fullCmdWithGpu << CLI_OPT_ASIC << " " << targetGpu << " ";

                                // Provide the pipeline state object configuration file.
                                for (auto psoStateFile : pVulkanClone->m_psoStates)
                                {
                                    // Only append the path for the active PSO config.
                                    if (psoStateFile.m_isActive)
                                    {
                                        // Append the pipeline's state file path.
                                        fullCmdWithGpu << CLI_OPT_PSO << " \"" << psoStateFile.m_pipelineStateFilePath << "\" ";
                                        break;
                                    }
                                }

                                // Append each active pipeline stage's input file to the command line.
                                if (pVulkanClone->m_pipeline.m_type == rgPipelineType::Graphics)
                                {
                                    size_t firstStage = static_cast<size_t>(rgPipelineStage::Vertex);
                                    size_t lastStage = static_cast<size_t>(rgPipelineStage::Fragment);

                                    // Step through each stage in a graphics pipeline. If the project's
                                    // stage is not empty, append the stage's shader file to the cmdline.
                                    for (size_t stageIndex = firstStage; stageIndex <= lastStage; ++stageIndex)
                                    {
                                        rgPipelineStage currentStage = static_cast<rgPipelineStage>(stageIndex);

                                        // Try to find the given stage within the pipeline's stage map.
                                        const auto& stageInputFile = pVulkanClone->m_pipeline.m_shaderStages[stageIndex];
                                        if (!stageInputFile.empty())
                                        {
                                            // A source file exists in this stage. Append it to the command line.
                                            std::string stageAbbreviation = pVulkanUtil->PipelineStageToAbbreviation(currentStage);

                                            // Append the stage type and shader file path to the command line.
                                            fullCmdWithGpu << "--" << stageAbbreviation << " \"" << stageInputFile << "\" ";
                                        }
                                    }
                                }
                                else if (pVulkanClone->m_pipeline.m_type == rgPipelineType::Compute)
                                {
                                    rgPipelineStage currentStage = rgPipelineStage::Compute;

                                    // Does the pipeline have a compute shader source file?
                                    const auto& computeShaderInputSourceFilePath = pVulkanClone->m_pipeline.m_shaderStages[static_cast<size_t>(currentStage)];
                                    if (!computeShaderInputSourceFilePath.empty())
                                    {
                                        // A source file exists in this stage. Append it to the command line.
                                        std::string stageAbbreviation = pVulkanUtil->PipelineStageToAbbreviation(currentStage);

                                        // Append the stage type and shader file path to the command line.
                                        fullCmdWithGpu << "--" << stageAbbreviation << " \"" << computeShaderInputSourceFilePath << "\" ";
                                    }
                                }
                                else
                                {
                                    // The pipeline type can only be graphics or compute.
                                    // If we get here something is wrong with the project clone.
                                    assert(false);
                                    ret = false;
                                }

                                // Verify that all operations up to this point were successful.
                                assert(ret);
                                if (ret)
                                {
                                    // Add the full CLI execution string to the output window's log.
                                    cmdLineOutputStream << fullCmdWithGpu.str();

                                    // Send the new output text to the output window.
                                    if (cliOutputHandlingCallback != nullptr)
                                    {
                                        cliOutputHandlingCallback(cmdLineOutputStream.str());
                                    }

                                    // Execute the command and grab the output.
                                    rgLog::file << STR_LOG_LAUNCHING_CLI << rgLog::noflush << std::endl << fullCmdWithGpu.str() << std::endl << rgLog::flush;
                                    gtString cmdLineOutputAsGtStr;
                                    ret = osExecAndGrabOutput(fullCmdWithGpu.str().c_str(), cancelSignal, cmdLineOutputAsGtStr);

                                    assert(ret);
                                    if (ret)
                                    {
                                        // Add the GPU to the output list if it was built successfully.
                                        gpusBuilt.push_back(targetGpu);
                                    }

                                    // Append the CLI's output to the string containing the entire execution output.
                                    fullCliOutput << cmdLineOutputAsGtStr.asASCIICharArray();

                                    // Invoke the callback used to send new CLI output to the GUI.
                                    if (cliOutputHandlingCallback != nullptr)
                                    {
                                        cliOutputHandlingCallback(cmdLineOutputAsGtStr.asASCIICharArray());
                                    }
                                }
                                else
                                {
                                    // Send a failure error message to the output window.
                                    if (cliOutputHandlingCallback != nullptr)
                                    {
                                        cliOutputHandlingCallback(STR_ERR_FAILED_TO_GENERATE_BUILD_COMMAND);
                                    }
                                }
                            }
                            else
                            {
                                // Stop the process if build is canceled.
                                break;
                            }
                        }
                    }
                }
            }
        }
        else
        {
            // Invoke the callback used to send new CLI output to the GUI.
            if (cliOutputHandlingCallback != nullptr)
            {
                std::stringstream errorStream;
                errorStream << STR_OUTPUT_WINDOW_BUILDING_PROJECT_FAILED_TEXT;
                errorStream << STR_OUTPUT_WINDOW_BUILDING_PROJECT_FAILED_INVALID_CLONE;
                cliOutputHandlingCallback(errorStream.str());
            }
        }
    }

    return ret;
}

bool rgCliLauncher::DisassembleSpvToText(const std::string& compilerBinFolder, const std::string& spvFullFilePath,
                                         const std::string& outputFilePath, std::string& cliOutput)
{
    bool ret = false;

    // Generate the command line backend invocation command.
    std::stringstream commandStream;
    BuildRgaExecutableCommandString(commandStream);

    // Add the current build mode to the command.
    const std::string& currentMode = rgConfigManager::Instance().GetCurrentModeString();
    commandStream << CLI_OPT_INPUT_TYPE << " " << currentMode << " ";

    // Append the alternative compiler path if it's not empty.
    if (!compilerBinFolder.empty())
    {
        commandStream << CLI_OPT_COMPILER_BIN_DIR << " \"" << compilerBinFolder << "\" ";
    }

    // Append the ISA text output path as well as the target SPIR-V to disassemble.
    commandStream << CLI_OPT_VULKAN_SPV_DIS << " \"" << outputFilePath << "\" \"" << spvFullFilePath << "\"";

    // Launch the command line backend to generate the version info file.
    bool cancelSignal = false;
    gtString cliOutputAsGtStr;
    bool isLaunchSuccessful = osExecAndGrabOutput(commandStream.str().c_str(), cancelSignal, cliOutputAsGtStr);
    assert(isLaunchSuccessful);

    if (isLaunchSuccessful)
    {
        cliOutput = cliOutputAsGtStr.asASCIICharArray();

        // Verify that the file was indeed created.
        bool isOutputFileCreated = rgUtils::IsFileExists(outputFilePath);
        assert(isOutputFileCreated);
        ret = isOutputFileCreated;
    }

    return ret;
}

bool rgCliLauncher::GenerateVersionInfoFile(const std::string& fullPath)
{
    bool ret = false;

    // Generate the command line backend invocation command.
    std::stringstream cmd;

    // Add the executable name.
    BuildRgaExecutableCommandString(cmd);

    // Add the version-info option to the command.
    cmd << CLI_OPT_VERSION_INFO << " \"" << fullPath << "\"";

    // Launch the command line backend to generate the version info file.
    bool cancelSignal = false;
    gtString cliOutputAsGtStr;
    bool isLaunchSuccessful = osExecAndGrabOutput(cmd.str().c_str(), cancelSignal, cliOutputAsGtStr);
    assert(isLaunchSuccessful);

    if (isLaunchSuccessful)
    {
        // Verify that the file was indeed created.
        bool isOutputFileCreated = rgUtils::IsFileExists(fullPath);
        assert(isOutputFileCreated);
        ret = isOutputFileCreated;
    }

    return ret;
}

bool rgCliLauncher::ListKernels(std::shared_ptr<rgProject> pProject, int cloneIndex, std::map<std::string, EntryToSourceLineRange>& entrypointLineNumbers)
{
    bool isParsingFailed = false;

    assert(pProject != nullptr);
    if (pProject != nullptr)
    {
        bool isValidIndex = cloneIndex >= 0 && cloneIndex < pProject->m_clones.size();
        assert(isValidIndex);
        if (isValidIndex && !pProject->m_clones.empty() && pProject->m_clones[cloneIndex] != nullptr)
        {
            // Append each input file to the end of the CLI command.
            for (const rgSourceFileInfo& fileInfo : pProject->m_clones[cloneIndex]->m_sourceFiles)
            {
                // Generate the command line backend invocation command.
                std::stringstream cmd;

                // Add the executable name.
                BuildRgaExecutableCommandString(cmd);

                // Add the current build mode to the command.
                const std::string& currentMode = rgConfigManager::Instance().GetCurrentModeString();
                cmd << CLI_OPT_INPUT_TYPE << " " << currentMode << " ";

                std::stringstream fullCmdWithGpu;
                fullCmdWithGpu << cmd.str();

                const std::string& sourceFilePath = fileInfo.m_filePath;

                // Add the version-info option to the command.
                cmd << CLI_OPT_LIST_KERNELS << " " << "\"" << sourceFilePath << "\"";

                // Launch the command line backend to generate the version info file.
                bool cancelSignal = false;
                gtString cliOutputAsGtStr;
                bool isLaunchSuccessful = osExecAndGrabOutput(cmd.str().c_str(), cancelSignal, cliOutputAsGtStr);
                assert(isLaunchSuccessful);
                if (isLaunchSuccessful)
                {
                    // Parse the output and dump it into entrypointLineNumbers.
                    isParsingFailed = !rgCliKernelListParser::ReadListKernelsOutput(cliOutputAsGtStr.asASCIICharArray(),
                                                                                    entrypointLineNumbers[sourceFilePath]);
                }
                else
                {
                    isParsingFailed = true;
                }
            }
        }
    }

    isParsingFailed = entrypointLineNumbers.empty();

    return !isParsingFailed;
}
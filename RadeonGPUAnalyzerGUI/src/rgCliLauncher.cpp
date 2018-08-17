// C++.
#include <sstream>
#include <cassert>

// Infra.
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <RadeonGPUAnalyzerGUI/../Utils/include/rgLog.h>

// Local.
#include <RadeonGPUAnalyzerGUI/include/rgCliLauncher.h>
#include <RadeonGPUAnalyzerGUI/include/rgDataTypes.h>
#include <RadeonGPUAnalyzerGUI/include/rgCliKernelListParser.h>
#include <RadeonGPUAnalyzerGUI/include/rgStringConstants.h>
#include <RadeonGPUAnalyzerGUI/include/rgUtils.h>
#include <RadeonGPUAnalyzerGUI/include/rgCliUtils.h>
#include <RadeonGPUAnalyzerGUI/include/rgXMLSessionConfig.h>

// Common between the CLI and the GUI.
#include <RadeonGPUAnalyzerGUI/../Utils/include/rgaCliDefs.h>

// A helper function responsible for building a base command string for CLI invocation.
void BuildRgaExecutableCommand(std::stringstream& commandStream)
{
    // Add the RGA executable name to invoke, and a space.
    commandStream << STR_EXECUTABLE_NAME;
    commandStream << " ";
}

bool rgCliLauncher::BuildProjectClone(std::shared_ptr<rgProject> pProject, int cloneIndex, const std::string& outputPath,
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
            std::shared_ptr<rgProjectClone> targetClone = pProject->m_clones[cloneIndex];

            // Generate the command line invocation command.
            std::stringstream cmd;

            // Add the executable name.
            BuildRgaExecutableCommand(cmd);

            // Add the current build mode to the command.
            const std::string& currentMode = rgConfigManager::Instance().GetCurrentMode();
            cmd << CLI_OPT_INPUT_TYPE << " " << currentMode << " ";

            // ISA disassembly in text and CSV formats.
            cmd << CLI_OPT_ISA << " \"" << outputPath << "disassem.txt\" " << CLI_OPT_PARSE_ISA << " ";

            // Include line numbers in the CSV file.
            cmd << CLI_OPT_LINE_NUMBERS << " ";

            // Add the output path for the resource usage analysis file.
            cmd << CLI_OPT_STATISTICS << " \"" << outputPath << STR_RESOURCE_USAGE_CSV_FILENAME << "\" ";

            // Binary.
            cmd << CLI_OPT_BINARY << " \"" << outputPath << "codeobj.bin\" ";

            // Build settings.
            std::string buildSettings;
            std::shared_ptr<rgCLBuildSettings> pClBuildSettings =
                std::static_pointer_cast<rgCLBuildSettings>(targetClone->m_pBuildSettings);
            ret = rgCliUtils::GenerateBuildSettingsString(pClBuildSettings, buildSettings);
            assert(ret);
            if (!buildSettings.empty())
            {
                cmd << buildSettings << " ";
            }

            // Execute the CLI for each target GPU, appending the GPU to the command.
            for (const std::string& targetGpu : targetClone->m_pBuildSettings->m_targetGpus)
            {
                if (!cancelSignal)
                {
                    std::stringstream fullCmdWithGpu;
                    fullCmdWithGpu << cmd.str();

                    // Metadata file.
                    fullCmdWithGpu << CLI_OPT_SESSION_METADATA << " \"" << outputPath << targetGpu << "_" << STR_SESSION_METADATA_FILENAME << "\" ";

                    // Specify which GPU to build outputs for.
                    fullCmdWithGpu << CLI_OPT_ASIC << " " << targetGpu << " ";

                    // Append each input file to the end of the CLI command.
                    for (const rgSourceFileInfo& fileInfo : targetClone->m_sourceFiles)
                    {
                        // Surround the path to the input file with quotes to prevent breaking the CLI parser.
                        fullCmdWithGpu << "\"";
                        fullCmdWithGpu << fileInfo.m_filePath;
                        fullCmdWithGpu << "\" ";
                    }

                    // Build an output line for each CLI invocation. Print the command string that's about to be invoked.
                    std::stringstream buildOutputHeaderStream;
                    buildOutputHeaderStream << STR_OUTPUT_WINDOW_BUILDING_PROJECT_HEADER_A;

                    // Include the project's API in the build header line.
                    std::string projectApiString;
                    bool isOk = rgUtils::ProjectAPIToString(pProject->m_api, projectApiString);
                    assert(isOk);
                    if (isOk)
                    {
                        buildOutputHeaderStream << projectApiString;
                    }

                    // Also include the project name and target GPU.
                    buildOutputHeaderStream << STR_OUTPUT_WINDOW_BUILDING_PROJECT_HEADER_B;
                    buildOutputHeaderStream << pProject->m_projectName;
                    buildOutputHeaderStream << STR_OUTPUT_WINDOW_BUILDING_PROJECT_HEADER_C;
                    buildOutputHeaderStream << targetGpu;

                    // Insert a separator line above and below the build output line.
                    std::string buildHeaderString = buildOutputHeaderStream.str();
                    int numDashesToInsert = static_cast<int>(buildHeaderString.length());
                    std::string dashedLines = std::string(numDashesToInsert, '-');

                    // Append the build project header line into the output window's log.
                    std::stringstream cmdLineOutputStream;
                    cmdLineOutputStream << dashedLines << std::endl;
                    cmdLineOutputStream << buildHeaderString << std::endl;
                    cmdLineOutputStream << dashedLines << std::endl;

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

bool rgCliLauncher::GenerateVersionInfoFile(const std::string& fullPath)
{
    bool ret = false;

    // Generate the command line backend invocation command.
    std::stringstream cmd;

    // Add the executable name.
    BuildRgaExecutableCommand(cmd);

    // Add the current build mode to the command.
    const std::string& currentMode = rgConfigManager::Instance().GetCurrentMode();
    cmd << CLI_OPT_INPUT_TYPE << " " << currentMode << " ";

    // Add the version-info option to the command.
    cmd << CLI_OPT_VERSION_INFO << " " << fullPath;

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
        std::shared_ptr<rgProjectClone> pClone = pProject->m_clones[cloneIndex];
        if (isValidIndex && !pProject->m_clones.empty() && pClone != nullptr)
        {
            // Append each input file to the end of the CLI command.
            for (const rgSourceFileInfo& fileInfo : pClone->m_sourceFiles)
            {
                // Generate the command line backend invocation command.
                std::stringstream cmd;

                // Add the executable name.
                BuildRgaExecutableCommand(cmd);

                // Add the current build mode to the command.
                const std::string& currentMode = rgConfigManager::Instance().GetCurrentMode();
                cmd << CLI_OPT_INPUT_TYPE << " " << currentMode << " ";

                std::stringstream fullCmdWithGpu;
                fullCmdWithGpu << cmd.str();

                // Append extra command line tokens for the build settings.
                std::string buildSettings;
                std::shared_ptr<rgCLBuildSettings> pClBuildSettings =
                    std::static_pointer_cast<rgCLBuildSettings>(pClone->m_pBuildSettings);
                isParsingFailed = rgCliUtils::GenerateBuildSettingsString(pClBuildSettings, buildSettings);

                assert(isParsingFailed);
                if (!buildSettings.empty())
                {
                    fullCmdWithGpu << buildSettings << " ";
                }

                const std::string& sourceFilePath = fileInfo.m_filePath;

                // Add the version-info option to the command.
                fullCmdWithGpu << CLI_OPT_LIST_KERNELS << " " << "\"" << sourceFilePath << "\"";

                // Launch the command line backend to generate the version info file.
                bool cancelSignal = false;
                gtString cliOutputAsGtStr;
                bool isLaunchSuccessful = osExecAndGrabOutput(fullCmdWithGpu.str().c_str(), cancelSignal, cliOutputAsGtStr);
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
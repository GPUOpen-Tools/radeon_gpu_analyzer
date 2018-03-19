#pragma once

// C++.
#include <memory>
#include <string>
#include <vector>
#include <functional>

// Local.
#include <RadeonGPUAnalyzerGUI/include/rgDataTypes.h>

// Forward declarations.
struct rgCLProjectClone;
struct rgCliBuildOutput;
struct rgProject;

class rgCliLauncher
{
public:
    // Runs RGA to compile the given ROCm-OpenCL project clone.
    // pProject is the project containing the clone to be built.
    // cloneIndex is the index of the clone to be built.
    // outputPath is where the output files will be generated.
    // cliOutputHandlingCallback is a callback used to send CLI output text to the GUI.
    // cancelSignal can be used to terminate the operation.
    // Returns true for success, false otherwise.
    static bool BuildProjectClone(std::shared_ptr<rgProject> pProject, int cloneIndex, const std::string& outputPath,
        std::function<void(const std::string&)> cliOutputHandlingCallback, std::vector<std::string>& gpusBuilt, bool& cancelSignal);

    // Runs RGA to generate a version info file for the CLI.
    // fullPath is the path to where the version info XML file will be saved to.
    // Returns true for success, false otherwise.
    static bool GenerateVersionInfoFile(const std::string& fullPath);

    // Runs RGA to retrieve the start line number for each kernel.
    // pProject is a pointer to the project being built.
    // cloneIndex is the index of the clone to access.
    // entrypointLineNumbers A map that gets filled up with the start line numbers for each input file's entrypoints.
    static bool ListKernels(std::shared_ptr<rgProject> pProject, int cloneIndex, std::map<std::string, EntryToSourceLineRange>& entrypointLineNumbers);
private:
    rgCliLauncher() = default;
    ~rgCliLauncher() = default;
};
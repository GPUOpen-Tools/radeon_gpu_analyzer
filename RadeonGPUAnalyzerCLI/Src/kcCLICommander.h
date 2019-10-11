#pragma once

// C++.
#include <string>
#include <set>
#include <functional>

// Local.
#include <RadeonGPUAnalyzerCLI/Src/kcConfig.h>
#include <RadeonGPUAnalyzerCLI/Src/kcDataTypes.h>
#include <RadeonGPUAnalyzerCLI/Src/kcCliStringConstants.h>
#include <RadeonGPUAnalyzerCLI/Src/kcUtils.h>

using namespace std;

// This is the Commander interface
class kcCLICommander
{
public:

    virtual ~kcCLICommander() {};

    // List the adapters installed on the system.
    virtual void ListAdapters(Config& config, LoggingCallBackFunc_t callback);

    // Print the driver version.
    virtual void Version(Config& config, LoggingCallBackFunc_t callback);

    // Print list of supported devices.
    virtual bool PrintAsicList(const Config&)
    {
        return kcUtils::PrintAsicList();
    }

    // Perform compilation steps.
    virtual void RunCompileCommands(const Config& config, LoggingCallBackFunc_t callback) = 0;

    // Perform post-compile actions.
    virtual bool RunPostCompileSteps(const Config& config);

    // Generates the RGA CLI version info file with the provided name.
    static bool GenerateVersionInfoFile(const Config& config);

    // Parse the source file and extract list of entry points (for example, kernels for OpenCL).
    // Dump the extracted entry points to stdout.
    virtual bool ListEntries(const Config& config, LoggingCallBackFunc_t callback);

protected:
    // -- Functions --

    // Convert ISA text to CSV form with additional data.
    bool GetParsedIsaCSVText(const std::string& isaText, const std::string& device, bool addLineNumbers, std::string& csvText);

    // Initialize the list of GPU targets based on device(s) specified in "devices".
    // "supportedDevices" is the list of supported devices for the given mode. Only targets from this list will be considered.
    // The matched devices are returned in "matchedDevices" set.
    // If "allowUnknownDevice" is true, no error message will be printed if no matched devices are found.
    static bool InitRequestedAsicList(const std::vector<std::string>& devices, RgaMode mode,
                                      const std::set<std::string>& supportedDevices,
                                      std::set<std::string>& matchedDevices, bool allowUnknownDevice);

    // Store ISA text in the file.
    beStatus WriteISAToFile(const std::string& fileName, const std::string& isaText);

    // Logging callback type.
    bool LogCallBack(const std::string& str) const;

    // -- Data --

    // Log callback function
    LoggingCallBackFunc_t   m_LogCallback;
};

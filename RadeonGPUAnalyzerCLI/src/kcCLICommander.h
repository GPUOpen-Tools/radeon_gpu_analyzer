#pragma once

// C++.
#include <string>
#include <set>
#include <functional>

// Local.
#include <RadeonGPUAnalyzerCLI/src/kcConfig.h>
#include <RadeonGPUAnalyzerCLI/src/kcDataTypes.h>
#include <RadeonGPUAnalyzerCLI/src/kcCliStringConstants.h>
#include <RadeonGPUAnalyzerCLI/src/kcUtils.h>

using namespace std;

// This is the Commander interface
class kcCLICommander
{
public:

    virtual ~kcCLICommander() {};

    // List the adapters installed on the system.
    virtual void ListAdapters(Config& config, LoggingCallBackFunc_t callback);

    // list the driver version
    virtual void Version(Config& config, LoggingCallBackFunc_t callback);

    // Print list of known devices
    virtual bool PrintAsicList(std::ostream& log)
    {
        return kcUtils::PrintAsicList(log);
    }

    // Output multiple commands for all commands that requires compilation:
    // GetBinary, GetIL, GetISA, GetAnlysis, GetMetadata, GetDebugIL, ListKernels
    virtual void RunCompileCommands(const Config& config, LoggingCallBackFunc_t callback) = 0;

    // Perform post-compile actions.
    virtual bool RunPostCompileSteps(const Config& config) const;

    // Delete all temporary files created by RGA.
    void DeleteTempFiles() const;

    // Get the list of supported targets for the given mode (source language).
    static bool  GetSupportedTargets(SourceLanguage lang, std::set<std::string>& targets);

protected:
    // -- Functions --

    // Initialize the list of GPU targets based on device(s) specified in the config.
    // "supportedDevices" is the list of supported devices. Only targets from this list will be considered.
    // The matched devices are returned in "matchedDevices" set.
    // If "allowUnknownDevice" is true, no error message will be printed if no matched devices are found.
    static bool InitRequestedAsicList(const Config& config, const std::set<std::string>& supportedDevices,
                                      std::set<std::string>& matchedDevices, bool allowUnknownDevice);

    // Generate RGA CLI session metadata file.
    virtual bool  GenerateSessionMetadata(const Config& config, const rgOutputMetadata& outMetadata) const { return true; }

    // Logging callback type.
    bool LogCallBack(const std::string& theString) const
    {
        bool bRet = false;

        if (m_LogCallback)
        {
            m_LogCallback(theString);
            bRet = true;
        }

        return bRet;
    }

    // -- Data --

    // Log callback function
    LoggingCallBackFunc_t   m_LogCallback;
    // Output Metadata
    rgOutputMetadata        m_outputMetadata;
};

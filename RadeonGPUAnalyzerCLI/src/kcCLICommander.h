#ifndef _KCCLICOMMANDER_H_
#define _KCCLICOMMANDER_H_

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
    bool RunPostCompileSteps(const Config& config) const;

    // Delete all temporary files created by RGA.
    void DeleteTempFiles() const;

    // Parse the source file and extract list of entry points (for example, kernels for OpenCL).
    // Dump the extracted entry points to stdout.
    static bool  ListEntries(const Config& config, LoggingCallBackFunc_t callback);

    // Get the list of supported targets for the given mode (source language).
    static bool  GetSupportedTargets(SourceLanguage lang, std::set<std::string>& targets);

protected:
    // -- Functions --

    // Initialize the list of GPU targets.
    static bool InitRequestedAsicList(const Config& config, const std::set<std::string>& supportedDevices,
                                      std::set<std::string>& targets, std::function<void(const std::string&)> logCallback);

    // Generate RGA CLI session metadata file.
    bool  GenerateSessionMetadata(const Config& config, const rgOutputMetadata& outMetadata) const;

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



#endif



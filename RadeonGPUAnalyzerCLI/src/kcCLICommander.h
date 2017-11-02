#ifndef _KCCLICOMMANDER_H_
#define _KCCLICOMMANDER_H_

// C++.
#include <string>
#include <set>

// Local.
#include <RadeonGPUAnalyzerCLI/src/kcConfig.h>
#include <RadeonGPUAnalyzerCLI/src/kcDataTypes.h>
#include <RadeonGPUAnalyzerCLI/src/kcCliStringConstants.h>

using namespace std;

/// This is the Commander interface
class kcCLICommander
{
public:

    virtual ~kcCLICommander() {};

    /// List the asics as got from device
    virtual void ListAsics(Config& config, LoggingCallBackFunc_t callback) = 0;

    /// List the adapters installed on the system.
    virtual void ListAdapters(Config& config, LoggingCallBackFunc_t callback);

    /// list the driver version
    virtual void Version(Config& config, LoggingCallBackFunc_t callback);

    /// Output multiple commands for all commands that requires compilation: GetBinary, GetIL, GetISA, GetAnlysis, GetMetadata, GetDebugIL,ListKernels
    virtual void RunCompileCommands(const Config& config, LoggingCallBackFunc_t callback) = 0;


protected: // functions

    /// Initialize the list of GPU targets.
    bool InitRequestedAsicList(const Config& config, const std::set<std::string>& supportedDevices, std::set<std::string>& targets);

    LoggingCallBackFunc_t m_LogCallback;

    /// Logging callback type.
    bool LogCallBack(const std::string& theString)
    {
        bool bRet = false;

        if (m_LogCallback)
        {
            m_LogCallback(theString);
            bRet = true;
        }

        return bRet;
    }
};



#endif



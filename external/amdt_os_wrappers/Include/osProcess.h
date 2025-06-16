//=====================================================================
// Copyright 2016-2025 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osProcess.h
///
//=====================================================================

//------------------------------ osProcess.h ------------------------------

#ifndef __OSPROCESS
#define __OSPROCESS

// Forward decelerations:
struct osEnvironmentVariable;

// Infra:
#include <amdt_base_tools/Include/gtString.h>
#include <amdt_base_tools/Include/gtVector.h>

// Local:
#include <amdt_os_wrappers/Include/osOSWrappersDLLBuild.h>
#include <amdt_os_wrappers/Include/osOSDefinitions.h>
#include <amdt_os_wrappers/Include/osFilePath.h>
#include <amdt_os_wrappers/Include/osModuleArchitecture.h>

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)
    #include <TlHelp32.h>
#endif


enum osRuntimePlatform
{
    OS_NATIVE_PLATFORM,
    OS_JAVA_PLATFORM,
    OS_DOT_NET_PLATFORM,
    OS_UNKNOWN_PLATFORM
};

/// This structure can be implemented for any OS implementation to add platform-specific parameters for process creation
struct AdditionalProcessParams
{
    /// Virtual Destructor
    virtual ~AdditionalProcessParams() {}
};

/// Structure for the additional parameters for the Window Process
struct WindowsProcessAdditionalParameter : AdditionalProcessParams
{
    bool m_bCreateNewProcessWithGroup;   ///< this flag corresponds to CREATE_NEW_PROCESS_GROUP for windows process creation

    /// Constructor
    WindowsProcessAdditionalParameter():
        m_bCreateNewProcessWithGroup(true)
    {}
};

OS_API bool osSetCurrentProcessEnvVariable(const osEnvironmentVariable& envVariable);
OS_API bool osRemoveCurrentProcessEnvVariable(const gtString& envVariableName);
OS_API bool osGetCurrentProcessEnvVariableValue(const gtString& envVariableName, gtString& envVariableValue);
OS_API bool osTokenizeCurrentProcessEnvVariableValue(const gtString& envVariableName, const gtString& delimiters, gtVector<gtString>& envVariableTokens);
OS_API bool osExpandEnvironmentStrings(const gtString& envVariableValue, gtString& expandedEnvVariableValue);
OS_API osProcessId osGetCurrentProcessId();
OS_API void osExitCurrentProcess(int exitCode);

OS_API bool osLaunchSuspendedProcess(const osFilePath& executablePath,
                                     const gtString& arguments,
                                     const osFilePath& workDirectory,
                                     osProcessId& processId,
                                     osProcessHandle& processHandle,
                                     osThreadHandle& processThreadHandle,
                                     bool createWindow = true,
                                     bool redirectFiles = false,
                                     bool removeCodeXLPaths = true,
                                     const AdditionalProcessParams* pAdditionalParams = nullptr);
OS_API bool osResumeSuspendedProcess(const osProcessId& processId, const osProcessHandle& processHandle,
                                     const osThreadHandle& processThreadHandle, bool closeHandles);

OS_API bool osWaitForProcessToTerminate(osProcessId processId, unsigned long timeoutMsec, long* pExitCode = NULL, bool child = true);
OS_API bool osIsProcessAlive(osProcessId processId, bool& isAliveBuffer);
OS_API bool osIsProcessAlive(const gtString& processName);
OS_API bool osTerminateProcess(osProcessId processId, long exitCode = 0, bool isTerminateChildren = true, bool isGracefulShutdownRequired = false);
OS_API void osTerminateProcessesByName(const gtVector<gtString>& processNames, const osProcessId parentProcessId = 0, const bool exactMatch = true, bool isGracefulShutdownRequired = false);
OS_API void osCloseProcessRedirectionFiles();
OS_API bool osIsParent(osProcessId parentProcessId, osProcessId processId);

OS_API bool osSetProcessAffinityMask(osProcessId processId, const osProcessHandle processHandle, gtUInt64 affinityMask);

OS_API bool osTerminateChildren(osProcessId processId, bool isGracefulShutdownRequired = false);

OS_API bool osGetProcessType(osProcessId processId, osModuleArchitecture& arch, osRuntimePlatform& platform, bool setPrivilege = true);

OS_API bool osGetProcessLaunchInfo(osProcessId processId,
                                   osModuleArchitecture& arch, osRuntimePlatform& platform,
                                   gtString& executablePath, gtString& commandLine, gtString& workDirectory,
                                   bool setPrivilege = true);

OS_API bool osIsProcessAttachable(osProcessId processId);

OS_API bool osExecAndGrabOutput(const char* cmd, const bool& cancelSignal, gtString& cmdOutput);

#ifdef _WIN32
OS_API bool osExecAndGrabOutputAndError(const char* cmd, const bool& cancelSignal,
    const gtString& workingDir, gtString& cmdOutput, gtString& cmdErrOutput);

OS_API bool osExecAndGrabOutputAndErrorDebug(const char*     cmd, 
                                             const bool&     cancel_signal,
                                             const gtString& working_dir,
                                             gtString&       cmd_output,
                                             gtString&       cmd_er_output,
                                             gtString&       cmd_debug_output);
#endif

OS_API bool osGetMemoryUsage(const unsigned int processID,
                             unsigned int& PageFaultCount,
                             size_t& WorkingSetSize,
                             size_t& PeakWorkingSetSize,
                             size_t& QuotaPeakPagedPoolUsage,
                             size_t& QuotaPagedPoolUsage,
                             size_t& QuotaPeakNonPagedPoolUsage,
                             size_t& QuotaNonPagedPoolUsage,
                             size_t& PagefileUsage,
                             size_t& PeakPagefileUsage,
                             size_t& PrivateUsage);

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)
    OS_API bool osGetMainThreadId(osProcessId processId, osThreadId& mainThreadId);
#endif

class OS_API osProcessesEnumerator
{
public:
    osProcessesEnumerator();
    ~osProcessesEnumerator();

    bool initialize();
    void deinitialize();

    bool next(osProcessId& processId, gtString* pName = NULL);

private:
    void* m_pEnumHandler;

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    PROCESSENTRY32 m_pe32;
#endif
};

// Mac OS X only:
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
    OS_API bool osShowOrHideProcess(osProcessId processId, bool showProcess);
    OS_API bool osTransformProcessToForegroundApplication(osProcessId processId);
#endif

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)
OS_API bool osGetRegistryKeyValue(HKEY hKey, const gtString& keyPath, const gtString& keyName, gtString& keyValue);
#else
OS_API bool osGetProcessIdentificationInfo(osProcessId& processId,
                                           osProcessId* pParentProcessId = NULL,
                                           osProcessId* pGroupId = NULL,
                                           char* pName = NULL,
                                           gtSize_t* pNameLen = NULL);
#endif
#endif  // __OSPROCESS


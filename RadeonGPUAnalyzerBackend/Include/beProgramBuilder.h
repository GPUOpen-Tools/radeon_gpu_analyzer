//=================================================================
// Copyright 2017 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

#ifndef _BEPROGRAMBUILDER_H_
#define _BEPROGRAMBUILDER_H_

// Disable warning:
#ifdef _WIN32
    #pragma warning(push)
    #pragma warning( disable : 4996 )
    #pragma warning( disable : 4251 )
#endif

#include "RadeonGPUAnalyzerBackend/Include/beInclude.h"
#include <DeviceInfo.h>
class backend;


class beProgramBuilder
{
public:
    virtual ~beProgramBuilder() {};

    /// Get a binary version of the program.
    /// \param[in]  program Handle to the built program.
    /// \param[in]  device  The name of the device to choose.
    /// \param[in]  binopts Options to customize the output object.
    ///                     If NULL, a complete object is returned.
    /// \param[out] binary  A place to return a reference to the binary.
    /// \returns            a status.
    /// If a Log stream is available, some failures may be diagnosed.
    virtual beKA::beStatus GetBinary(const std::string& device, const beKA::BinaryOptions& binopts, std::vector<char>& binary) = 0;

    /// Get a string for a kernel IL.
    /// \param[in]  device     The name of the device.
    /// \param[in]  kernel     The name of the kernel.
    /// \param[out] s          The output as a string.
    /// \returns               a status.
    /// If a Log stream is available, some failures may be diagnosed.
    virtual beKA::beStatus GetKernelILText(const std::string& device, const std::string& kernel, std::string& il) = 0;

    /// Get a string for a kernel ISA.
    /// \param[in]  device     The name of the device.
    /// \param[in]  kernel     The name of the kernel.
    /// \param[out] s          The output as a string.
    /// \returns               a status.
    /// If a Log stream is available, some failures may be diagnosed.
    virtual beKA::beStatus GetKernelISAText(const std::string& device, const std::string& kernel, std::string& isa) = 0;

    /// Return the statistics for a specific kernel on a certain device.
    /// \param[in]  device     The name of the device.
    /// \param[in]  kernel     The name of the kernel.
    /// \param[out] analysis   The output as a analysis.
    /// \returns               a status.
    /// If a Log stream is available, some failures may be diagnosed.
    virtual beKA::beStatus GetStatistics(const std::string& device, const std::string& kernel, beKA::AnalysisData& analysis) = 0;

    /// retrieve all devices as got from the loaded module
    virtual beKA::beStatus GetDeviceTable(std::vector<GDT_GfxCardInfo>& table) = 0;

    // Parse ISA text and convert it to CSV format with additional data added (Functional Unit, Cycles, etc.)
    // if "isHeaderRequired" is true, adds standard disassembly header to the ISA text before parsing it.
    static beKA::beStatus ParseISAToCSV(const std::string& isaText, const std::string& device,
                                        std::string& parsedIsaText, bool addLineNumbers = false, bool isHeaderRequired = false);

    /// Set callback function for diagnostic output.
    /// \param[in] callback A pointer to callback function. Use nullptr to avoid output generation.
    void SetLog(LoggingCallBackFuncP callback)
    {
        m_LogCallback = callback;
    }

protected:
    /// Stream for diagnostic output. set externally.
    LoggingCallBackFuncP m_LogCallback;

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

    /// Force the string to use native line endings.
    /// Linux will get LF.  Windows gets CR LF.
    /// This makes notepad and other Windows tools happier.
    virtual void UsePlatformNativeLineEndings(std::string& text);

    std::string m_DriverVersion;
};

#ifdef _WIN32
    #pragma warning(pop)
#endif

#endif

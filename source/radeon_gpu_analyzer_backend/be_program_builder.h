//=================================================================
// Copyright 2020 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

#ifndef RGA_RADEONGPUANALYZERBACKEND_SRC_BE_PROGRAM_BUILDER_H_
#define RGA_RADEONGPUANALYZERBACKEND_SRC_BE_PROGRAM_BUILDER_H_

// Disable warning:
#ifdef _WIN32
    #pragma warning(push)
    #pragma warning( disable : 4996 )
    #pragma warning( disable : 4251 )
#endif

#include "radeon_gpu_analyzer_backend/be_include.h"
#include "DeviceInfo.h"

class BeProgramBuilder
{
public:
    BeProgramBuilder() = default;
    virtual ~BeProgramBuilder() = default;

    // Get a string for a kernel's IL disassembly for a given device..
    virtual beKA::beStatus GetKernelIlText(const std::string& device, const std::string& kernel, std::string& il) = 0;

    // Get a string for a kernel's ISA disassembly for a given device..
    virtual beKA::beStatus GetKernelIsaText(const std::string& device, const std::string& kernel, std::string& isa) = 0;

    // Return the statistics for a specific kernel on a certain device.
    virtual beKA::beStatus GetStatistics(const std::string& device, const std::string& kernel, beKA::AnalysisData& analysis) = 0;

    // Retrieve all devices as got from the loaded module
    virtual beKA::beStatus GetDeviceTable(std::vector<GDT_GfxCardInfo>& table) = 0;

    // Parse ISA text and convert it to CSV format with additional data added (Functional Unit, Cycles, etc.)
    // if "is_header_required" is true, adds standard disassembly header to the ISA text before parsing it.
    static beKA::beStatus ParseIsaToCsv(const std::string& isa_text, const std::string& device,
                                        std::string& parsed_isa_text, bool should_add_line_numbers = false, bool is_header_required = false);

    // Set callback function for diagnostic output.
    void SetLog(LoggingCallBackFuncP callback);

protected:
    // Logging callback.
    bool LogCallback(const std::string& str);

    // Stream for diagnostic output. set externally.
    LoggingCallBackFuncP log_callback_ = nullptr;
};

#ifdef _WIN32
    #pragma warning(pop)
#endif

#endif // RGA_RADEONGPUANALYZERBACKEND_SRC_BE_PROGRAM_BUILDER_H_

//=============================================================================
/// Copyright (c) 2024-2026 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Class for Binary Mode mode utility functions for ISA post-processing.
//=============================================================================

#ifndef RGA_RADEONGPUANALYZERCLI_SRC_KC_UTILS_BINARY_H_
#define RGA_RADEONGPUANALYZERCLI_SRC_KC_UTILS_BINARY_H_

// C++.
#include <string>

// Local.
#include "radeon_gpu_analyzer_cli/kc_utils.h"

// Class for Binary Mode mode utility functions for ISA post-processing.
class KcUtilsBinary
{
public:
    static void LogPreStep(const std::string& msg, const std::string& device = "");

    static void LogResult(bool result);

    static void LogErrorStatus(beKA::beStatus status, const std::string& errMsg);

    // Extract target device from ISA disassembly (amdgpu-dis output: looks for -mcpu= token).
    static bool ExtractDeviceFromIsaDisassembly(const std::string& isa_dsassembly, std::string& device);

    // Extract target device from llvm-readobj --file-header output (looks for EF_AMDGPU_MACH_AMDGCN_ token).
    static bool ExtractDeviceFromFileHeader(const std::string& file_header_text, std::string& device);

    // Write text disassembly to disk.
    static beKA::beStatus WriteDisassemblyText(const Config&      config,
                                               const std::string& binary_codeobj_file,
                                               const std::string& device,
                                               const std::string& text_dsassembly,
                                               std::string&       filename_on_disk);
};

#endif  // RGA_RADEONGPUANALYZERCLI_SRC_KC_UTILS_BINARY_H_
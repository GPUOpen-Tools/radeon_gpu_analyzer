//=============================================================================
/// Copyright (c) 2024-2026 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for Binary Mode mode utility functions for ISA post-processing.
//=============================================================================

// C++
#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstring>
#include <string>

// Shared.
#include "common/rga_cli_defs.h"
#include "common/rg_log.h"

// Local.
#include "radeon_gpu_analyzer_cli/kc_cli_string_constants.h"
#include "radeon_gpu_analyzer_cli/kc_utils_binary.h"

void KcUtilsBinary::LogPreStep(const std::string& msg, const std::string& device)
{
    RgLog::stdOut << msg << device << "... " << std::flush;
}

void KcUtilsBinary::LogResult(bool result)
{
    RgLog::stdOut << (result ? kStrInfoSuccess : kStrInfoFailed) << std::endl;
}

void KcUtilsBinary::LogErrorStatus(beKA::beStatus status, const std::string& errMsg)
{
    switch (status)
    {
    case beKA::beStatus::kBeStatusSuccess:
        break;
    case beKA::beStatus::kBeStatusNoDeviceFound:
        RgLog::stdOut << kStrErrorNoDeviceFound << errMsg << std::endl;
        break;
    case beKA::beStatus::kBeStatusUnknownDevice:
        RgLog::stdOut << kStrErrorUnknownDevice << errMsg << std::endl;
        break;
    case beKA::beStatus::kBeStatusBinaryInvalidInput:
        RgLog::stdOut << kStrErrorCannotReadFile << errMsg << std::endl;
        break;
    case beKA::beStatus::kBeStatusVulkanAmdgpudisLaunchFailed:
        RgLog::stdOut << kStrErrorFailedAmdGpuDisStatus << errMsg << std::endl;
        break;
    case beKA::beStatus::kBeStatusWriteToFileFailed:
    case beKA::beStatus::kBeStatusWriteParsedIsaFileFailed:
        RgLog::stdOut << kErrCannotWriteDisassemblyFile << errMsg << std::endl;
        break;
    default:
        RgLog::stdOut << std::endl << (errMsg.empty() ? kStrErrorUnknownAmdGpuDisStatus : errMsg) << std::endl;
        break;
    }
}

bool KcUtilsBinary::ExtractDeviceFromIsaDisassembly(const std::string& isa_dsassembly, std::string& device)
{
    bool ret = false;
    assert(!isa_dsassembly.empty());
    if (!isa_dsassembly.empty())
    {
        const char* kDeviceTextToken = "-mcpu=";
        // Get to the -mcpu section.
        size_t curr_pos = isa_dsassembly.find(kDeviceTextToken);
        assert(curr_pos != std::string::npos);
        if (curr_pos != std::string::npos)
        {
            const size_t device_offset_end   = isa_dsassembly.find(" ", curr_pos + strlen(kDeviceTextToken));
            const size_t device_offset_begin = curr_pos + strlen(kDeviceTextToken);
            device                           = isa_dsassembly.substr(device_offset_begin, device_offset_end - device_offset_begin);
            ret                              = true;
        }
    }
    return ret;
}

bool KcUtilsBinary::ExtractDeviceFromFileHeader(const std::string& file_header_text, std::string& device)
{
    // llvm-readobj --file-header outputs lines like:
    //   EF_AMDGPU_MACH_AMDGCN_GFX1102 (0x47)
    // We search for the EF_AMDGPU_MACH_AMDGCN_ prefix and extract the gfx name after it.
    bool               ret   = false;
    const std::string  token = "EF_AMDGPU_MACH_AMDGCN_";
    const size_t       pos   = file_header_text.find(token);
    if (pos != std::string::npos)
    {
        size_t name_begin = pos + token.size();
        size_t name_end   = file_header_text.find_first_of(" \t\n\r(", name_begin);
        if (name_end == std::string::npos)
        {
            name_end = file_header_text.size();
        }
        std::string raw_name = file_header_text.substr(name_begin, name_end - name_begin);
        // Convert to lowercase (ELF flags use uppercase like GFX1102 -> gfx1102).
        std::transform(raw_name.begin(), raw_name.end(), raw_name.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        if (!raw_name.empty())
        {
            device = raw_name;
            ret    = true;
        }
    }
    return ret;
}

beKA::beStatus KcUtilsBinary::WriteDisassemblyText(const Config&      config,
                                                   const std::string& binary_codeobj_file,
                                                   const std::string& device,
                                                   const std::string& text_dsassembly,
                                                   std::string&       filename_on_disk)
{
    beKA::beStatus status = beKA::beStatus::kBeStatusSuccess;
    if (!text_dsassembly.empty() && !config.binary_text_disassembly.empty())
    {
        // Construct a name for the output disassembly file.
        std::string output_disassembly_filename;
        bool        is_ok = KcUtils::ConstructOutFileName(
            config.binary_text_disassembly, kStrDefaultExtensionRawDisassembly, device, kStrDefaultExtensionText, output_disassembly_filename, false);
        if (is_ok && KcUtils::WriteTextFile(output_disassembly_filename, text_dsassembly, nullptr))
        {
            filename_on_disk = output_disassembly_filename;
        }
        else
        {
            status = beKA::beStatus::kBeStatusWriteToFileFailed;
        }
        LogErrorStatus(status, binary_codeobj_file);
    }
    return status;
}

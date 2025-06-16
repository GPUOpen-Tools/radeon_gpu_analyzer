//=============================================================================
/// Copyright (c) 2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for parsing binary code object disassembly.
//=============================================================================
#ifndef RGA_RADEONGPUANALYZERCLI_SRC_KC_UTILS_BINARY_PARSER_H_
#define RGA_RADEONGPUANALYZERCLI_SRC_KC_UTILS_BINARY_PARSER_H_

// C++.
#include <string>

// Local.
#include "radeon_gpu_analyzer_cli/kc_config.h"

// AmdgpudisOutput Parser interface.
class ParseAmdgpudisOutputStrategy
{
public:

    // Defaulted Virtual Destructor.
    virtual ~ParseAmdgpudisOutputStrategy() = default;

    // Parses amdgpu-dis output and extracts a table with
    // the amdgpu kernel name being the key and that shader stage's disassembly the value.
    virtual beKA::beStatus ParseAmdgpudisKernels(const std::string&                  amdgpu_dis_output,
                                                 std::map<std::string, std::string>& kernel_to_disassembly,
                                                 std::string&                        error_msg) const = 0;
};

// amdgpu-dis Output Parser for graphics workflows.
class ParseAmdgpudisOutputGraphicStrategy : public ParseAmdgpudisOutputStrategy
{
public:
    // Parses amdgpu-dis output and extracts a table with
    // the amdgpu shader name being the key and that shader stage's disassembly the value.
    beKA::beStatus ParseAmdgpudisKernels(const std::string&                  amdgpu_dis_output,
                                         std::map<std::string, std::string>& kernel_to_disassembly,
                                         std::string&                        error_msg) const override;
};

// amdgpu-dis output Parser for compute workflows.
class ParseAmdgpudisOutputComputeStrategy : public ParseAmdgpudisOutputStrategy
{
public:
    // Parses amdgpu-dis output and extracts a table with
    // the amdgpu shader name being the key and that shader stage's disassembly the value.
    beKA::beStatus ParseAmdgpudisKernels(const std::string&                  amdgpu_dis_output,
                                         std::map<std::string, std::string>& kernel_to_disassembly,
                                         std::string&                        error_msg) const override;
};

#endif  // RGA_RADEONGPUANALYZERCLI_SRC_KC_UTILS_BINARY_PARSER_H_

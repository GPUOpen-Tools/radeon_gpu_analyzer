//=============================================================================
// Copyright (c) 2022-2026 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Declaration for an isa parser utility.
//=============================================================================

#ifndef RGA_RADEONGPUANALYZERBACKEND_SRC_BE_ISA_PARSER_H_
#define RGA_RADEONGPUANALYZERBACKEND_SRC_BE_ISA_PARSER_H_

#include <memory>
#include <string>
#include <set>
#include <vector>

// Decoder.
#include "amdisa/isa_decoder.h"

// IsaParser is an utility for parsing isa instructions.
class IsaParser
{
public:
    // Initialize the decoder for the architectures.
    static bool InitializeDecoder(const std::set<std::string>& xml_files, std::string& decoder_status_error);

    // Specify an architecture to be used by the decoder.
    static bool SetArchitecture(const std::string& target_gpu);

    // Parse a single instruction's binary representation into an InstructionInfoBundle.
    static bool ParseInstruction(std::string binary_representation, amdisa::InstructionInfoBundle& instruction_info, uint64_t& instruction_size_in_bytes, std::string& err_message);

private:
    IsaParser()                            = delete;
    ~IsaParser()                           = default;
    IsaParser(const IsaParser&)            = delete;
    IsaParser& operator=(const IsaParser&) = delete;

    // Helper to get decoder for a given architecture.
    static bool SetArchitecture(amdisa::GpuArchitecture architecture);
};

#endif  // RGA_RADEONGPUANALYZERBACKEND_SRC_BE_ISA_PARSER_H_

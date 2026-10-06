//=============================================================================
/// Copyright (c) 2026 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for isa parser utility.
//=============================================================================

// C++.
#include <algorithm>
#include <cassert>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <unordered_map>

// Infra.
#include "external/amdt_os_wrappers/Include/osFilePath.h"
#include "external/amdt_os_wrappers/Include/osDirectory.h"
#include "external/amdt_os_wrappers/Include/osApplication.h"

// Shared.
#include "common/rga_shared_utils.h"

// Local.
#include "radeon_gpu_analyzer_backend/be_isa_parser.h"

namespace
{
    // The active decoder for the active architecture.
    std::shared_ptr<amdisa::IsaDecoder> isa_decoder = nullptr;

    // The manager of all the architectures.
    amdisa::DecodeManager decode_manager;

    // Isa decoder initialization status.
    bool is_decoder_initialized = false;

    // Instructions shader disassembly text delimiter
    static const char* kCodeCommentToken = "//";
    static const char  kColumnToken      = ':';
    static const char  kLlvmCommentToken = ';';

    // Constants used for comma separated values string.
    static const char* COMMA_SEPARATOR = ",";
    static const char* SPACE_SEPARATOR = " ";

    // Errors
    static const char* kStringWarnShaderTextInvalidDwordByte = "Warning: Invalid isa instruction: ";
    static const char* kStringWarnShaderTextDecodedDwordByte = "Warning: Failed to decode isa instruction: ";

}  // namespace

bool IsaParser::SetArchitecture(amdisa::GpuArchitecture architecture)
{
    if (!is_decoder_initialized)
    {
        return false;
    }

    isa_decoder = decode_manager.GetDecoder(architecture);

    return isa_decoder != nullptr;
}

bool IsaParser::InitializeDecoder(const std::set<std::string>& xml_files, std::string& decoder_status_error)
{
    bool       ret = true;
    osFilePath application_dir;
    osGetCurrentApplicationPath(application_dir, false);

    std::filesystem::path isa_spec_dir_path(application_dir.fileDirectoryAsString().asASCIICharArray());
    isa_spec_dir_path /= "utils";
    isa_spec_dir_path /= "isa_spec";
    isa_spec_dir_path.make_preferred();

    std::vector<std::string> xml_file_paths;

    for (const auto& isa_spec_name : xml_files)
    {
        std::filesystem::path isa_spec_path(isa_spec_dir_path);

        isa_spec_path /= isa_spec_name;

        isa_spec_path.make_preferred();

        xml_file_paths.push_back(isa_spec_path.string());
    }

    if (!xml_file_paths.empty())
    {
        is_decoder_initialized = decode_manager.Initialize(xml_file_paths, decoder_status_error);

        if (!is_decoder_initialized)
        {
            ret = false;
        }
    }

    return ret;
}

bool IsaParser::SetArchitecture(const std::string& target_gpu)
{
    amdisa::GpuArchitecture architecture = amdisa::GpuArchitecture::kUnknown;
    bool                    success      = RgaSharedUtils::GetGpuArchitectureFromTarget(target_gpu, architecture);
    return SetArchitecture(architecture) && success;
}

bool IsaParser::ParseInstruction(std::string                    binary_representation,
                                 amdisa::InstructionInfoBundle& instruction_info,
                                 uint64_t&                      instruction_size_in_bytes,
                                 std::string&                   err_message)
{
    // Decode dword/s
    std::stringstream stream;
    stream << binary_representation;

    static const uint8_t kDwordSize     = 32;
    static const uint8_t kBytesPerDword = 4;
    bool                 is_decoded     = false;
    if (stream.str().length() == (kDwordSize / kBytesPerDword))
    {
        // Single machine code 64-bit
        std::uint64_t machine_code_64     = 0;
        instruction_size_in_bytes         = kBytesPerDword;

        // Convert instruction DWORD from hex to uint64_t
        stream >> std::hex >> machine_code_64;
        is_decoded = (isa_decoder != nullptr) && isa_decoder->DecodeInstruction(machine_code_64, instruction_info, err_message);
        if (!is_decoded || instruction_info.bundle.empty())
        {
            err_message = kStringWarnShaderTextDecodedDwordByte;
            err_message.append(stream.str());
        }
    }
    else if (stream.str().length() > (kDwordSize / kBytesPerDword))
    {
        std::vector<amdisa::InstructionInfoBundle> instruction_info_stream;

        // For 32-bit Instructions
        std::uint32_t              machine_code_32  = 0;
        std::vector<std::uint32_t> machine_codes_32 = {};

        // Pack DWORDs in an instruction stream for decoding
        while (stream >> std::hex >> machine_code_32)
        {
            machine_codes_32.push_back(machine_code_32);
            machine_code_32 = 0;
        }

        instruction_size_in_bytes = machine_codes_32.size() * kBytesPerDword;

        is_decoded = (isa_decoder != nullptr) && isa_decoder->DecodeInstructionStream(machine_codes_32, instruction_info_stream, err_message);

        if (!is_decoded || instruction_info_stream.size() != 1)
        {
            err_message = kStringWarnShaderTextDecodedDwordByte;
            err_message.append(stream.str());
        }
        else
        {
            instruction_info = instruction_info_stream.front();
        }

        machine_codes_32.clear();
    }
    else
    {
        err_message = kStringWarnShaderTextInvalidDwordByte;
        err_message.append(stream.str());
    }

    return is_decoded;
}
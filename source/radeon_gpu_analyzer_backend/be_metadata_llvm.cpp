//=============================================================================
/// Copyright (c) 2024-2026 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for class representing metadata for reconstructing .cl src code from llvm text disassembly.
//=============================================================================

// C++.
#include <cassert>
#include <filesystem>

// Common.
#include "common/rga_shared_utils.h"

// Local.
#include "radeon_gpu_analyzer_backend/be_metadata_llvm.h"

// CLI.
#include "radeon_gpu_analyzer_cli/kc_utils.h"

bool BeLlvmMetaData::PopulateSourceFileInfo(const std::string& isa_file_name, SrcFileMap& src_fileinfo_map)
{
    if (!isa_file_name.empty())
    {
        std::string content;
        bool        is_file_read = KcUtils::ReadTextFile(isa_file_name, content, nullptr);
        assert(is_file_read);
        if (is_file_read && !content.empty())
        {
            std::string           src_file_name;
            uint32_t              src_line_number = 0;
            std::istringstream    isa_stream(content);
            std::string           isa_line, prev_line, src_line;
            std::vector<uint32_t> line_numbers;
            while (getline(isa_stream, isa_line))
            {
                if (RgaSharedUtils::GetSourceLineInfo(isa_line, prev_line, src_file_name, src_line, src_line_number))
                {
                    line_numbers.push_back(src_line_number);
                    src_fileinfo_map[src_file_name][src_line_number] = src_line;
                    continue;
                }
                prev_line = isa_line;
            }
        }
    }

    return !src_fileinfo_map.empty();
}

//=============================================================================
/// Copyright (c) 2026 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Class representing metadata for reconstructing .cl src code from llvm text disassembly.
//=============================================================================

#ifndef RGA_RADEONGPUANALYZERBACKEND_SRC_BE_METADATA_LLVM_H_
#define RGA_RADEONGPUANALYZERBACKEND_SRC_BE_METADATA_LLVM_H_

// C++.
#include <map>
#include <string>

// Metadata for reconstructing .cl src code from llvm text disassembly.
class BeLlvmMetaData
{
private:
    // Map line_number -> src_line(s).
    using SrcLineMap = std::map<uint32_t, std::string>;

public:
    // Map filename -> SrcLineMap.
    using SrcFileMap = std::map<std::string, SrcLineMap>;

    // Extract the source file associated with "isa".
    // We implicitly assume each isa entry correlates with a sincle src file.
    static bool PopulateSourceFileInfo(const std::string& isa_file_name, SrcFileMap& src_fileinfo_map);
};

#endif  // RGA_RADEONGPUANALYZERBACKEND_SRC_BE_METADATA_LLVM_H_

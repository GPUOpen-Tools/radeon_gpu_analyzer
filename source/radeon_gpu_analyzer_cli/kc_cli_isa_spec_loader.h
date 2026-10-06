//=============================================================================
// Copyright (c) 2022-2026 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Declaration for ISA spec loader utility.
//=============================================================================

#ifndef RGA_RADEONGPUANALYZERCLI_SRC_KC_CLI_ISA_SPEC_LOADER_H_
#define RGA_RADEONGPUANALYZERCLI_SRC_KC_CLI_ISA_SPEC_LOADER_H_

// C++.
#include <string>
#include <vector>

// KcCliIsaSpecLoader is a utility for loading and parsing ISA spec XML data.
class KcCliIsaSpecLoader
{
public:
    // Utility for parsing isa xml specs.
    static bool LoadIsaSpecsFromXML(bool is_parsed_isa_required, bool is_isa_spec_metadata_required, const std::vector<std::string>& target_gpus);
};

#endif  // RGA_RADEONGPUANALYZERCLI_SRC_KC_CLI_ISA_SPEC_LOADER_H_

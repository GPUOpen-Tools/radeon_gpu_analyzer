//=============================================================================
// Copyright (c) 2022-2026 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Declaration for an isa spec explorer utility.
//=============================================================================

#ifndef RGA_RADEONGPUANALYZERBACKEND_SRC_BE_ISA_SPEC_METADATA_H_
#define RGA_RADEONGPUANALYZERBACKEND_SRC_BE_ISA_SPEC_METADATA_H_

// C++.
#include <set>
#include <string>

// Common.
#include "common/rga_analysis_summary.h"

// Decoder.
#include <amdisa/isa_decoder.h>
#include <amdisa/isa_explorer.h>

namespace beKA
{
    // Explores ISA specifications for different target architectures.
    class BeIsaSpecExplorer
    {
    public:
        // Initialize the decoder for the architectures.
        static bool InitializeExplorer(const std::map<amdisa::GpuArchitecture, std::string>& xml_files, std::string& explorer_status_error);

        // Branch instruction types.
        enum class BranchType
        {
            kFallThrough = 0,
            kDirectBranch,
            kIndirectBranch,
            kCount
        };
            
        // Register types.
        enum class RegisterType
        {
            kVGPR = 0,
            kSGPR,
            kAGPR,
            kCount
        };

        // Populate ISA spec metadata for a given target GPU.
        static bool PopulateFromSpec(const std::string& target_gpu, RgaAnalysisSummary::TargetArchitectureMetadata& metadata);

        // Helper to populate branch type metadata.
        static std::string GetBranchTypeStr(BranchType branch_type);

    private:
        BeIsaSpecExplorer()                    = delete;
        ~BeIsaSpecExplorer()                   = default;
        BeIsaSpecExplorer(const BeIsaSpecExplorer&) = delete;
        BeIsaSpecExplorer& operator=(const BeIsaSpecExplorer&) = delete;

        // Specify an architecture to be used by the explorer.
        static bool SetArchitecture(const std::string& target_gpu);

        // Helper to get decoder for a given architecture.
        static bool SetArchitecture(amdisa::GpuArchitecture architecture);
    };

}  // namespace beKA

#endif  // RGA_RADEONGPUANALYZERBACKEND_SRC_BE_ISA_SPEC_METADATA_H_

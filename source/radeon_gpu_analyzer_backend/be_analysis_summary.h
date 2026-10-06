//=============================================================================
// Copyright (c) 2022-2026 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Contains the declaration of data structures representing the analysis summary.
//=============================================================================

#ifndef RGA_RADEONGPUANALYZERBACKEND_SRC_BE_ANALYSIS_SUMMARY_H_
#define RGA_RADEONGPUANALYZERBACKEND_SRC_BE_ANALYSIS_SUMMARY_H_

#include "common/rga_analysis_summary.h"
#include "radeon_gpu_analyzer_backend/be_include.h"
#include "radeon_gpu_analyzer_backend/be_isa_spec_metadata.h"

namespace beKA
{
    // Internal parsing utilities for RgaAnalysisSummary.
    struct BeAnalysisSummaryUtils
    {
        // Map from block label to block ID.
        std::unordered_map<std::string, int> block_label_to_id_map_;

        // Reads the isa text file, and gets its contents.
        static bool ReadKernelIsaDisassembly(const std::string& isa_filename, std::string& isa_text, LoggingCallBackFuncP callback);

        // Reads the isa text, and parses contents into instructions and basic blocks.
        static bool ParseCsvIsaDisassembly(const std::string& device,
                                           const std::string& kernel_name,
                                           const std::string& csv_filename,
                                           RgaAnalysisSummary::Instructions& instructions,
                                           bool               create_blocks,
                                           RgaAnalysisSummary::BasicBlocks& blocks,
                                           bool               verbose);

        // Reads the vgpr/sgpr file text, and parses contents.
        static bool ParseKernelRegisterFile(BeIsaSpecExplorer::RegisterType reg_type,
                                            const std::string&              filename,
                                            RgaAnalysisSummary::Instructions& instructions,
                                            RgaAnalysisSummary::Statistics&   stats);

        // Reads the cfg file text, and parses contents.
        static bool ParseCfgFile(const std::string&                      cfg_filename,
                                 const RgaAnalysisSummary::Instructions& instructions,
                                 RgaAnalysisSummary::ControlFlowGraph&   cfg);

        // Reads the stats file, and gets stats.
        static bool ParseStatsFile(const std::string& stats_filename, RgaAnalysisSummary::Statistics& stats);

        // Build a single comma-separated subgroup string from user-friendly name strings.
        static std::string BuildFunctionalSubgroupStringFromNames(const std::vector<std::string>& subgroup_names);
    };
}  // namespace beKA

#endif  // RGA_RADEONGPUANALYZERBACKEND_SRC_BE_ANALYSIS_SUMMARY_H_

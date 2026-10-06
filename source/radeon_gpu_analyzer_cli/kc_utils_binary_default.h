//=============================================================================
/// Copyright (c) 2026 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for binary analysis strategy interface.
//=============================================================================
#ifndef RGA_RADEONGPUANALYZERCLI_SRC_KC_UTILS_BINARY_DEFAULT_H_
#define RGA_RADEONGPUANALYZERCLI_SRC_KC_UTILS_BINARY_DEFAULT_H_

// C++.
#include <vector>

// Backend
#include "radeon_gpu_analyzer_backend/be_analysis_summary.h"
#include "radeon_gpu_analyzer_backend/be_metadata_parser.h"

// Local.
#include "radeon_gpu_analyzer_cli/kc_config.h"

// Post-processing workflow functions interface for binary mode.
class KcCliBinaryAnalysisStrategy
{
public:
    // Defaulted Virtual Destructor.
    virtual ~KcCliBinaryAnalysisStrategy() = default;

    // Write Isa file(s) to disk.
    virtual beKA::beStatus WriteOutputFiles(const Config&                             config,
                                            const std::string&                        asic,
                                            const std::map<std::string, std::string>& kernel_to_disassembly,
                                            std::string&                              error_msg) = 0;

    // Perform post-processing actions.
    virtual void RunPostProcessingSteps(const Config& config) = 0;

    // Generates the metadata for the binary.
    virtual bool GenerateSessionMetadataFile(const Config& config) = 0;

    // Generates the analysis output for the binary.
    virtual bool GeneratCompilationSummary(const Config& config, const std::string& asic, RgaAnalysisSummary::AnalysisResult& result) = 0;
};

#endif  // RGA_RADEONGPUANALYZERCLI_SRC_KC_UTILS_BINARY_DEFAULT_H_

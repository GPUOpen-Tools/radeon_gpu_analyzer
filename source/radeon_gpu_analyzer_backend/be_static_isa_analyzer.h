//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for rga backend isa static analyzer class.
//=============================================================================

#ifndef RGA_RADEONGPUANALYZERBACKEND_SRC_BE_STATIC_ISA_ANALYZER_H_
#define RGA_RADEONGPUANALYZERBACKEND_SRC_BE_STATIC_ISA_ANALYZER_H_

// Infra.
#include "external/amdt_base_tools/Include/gtString.h"

// Local.
#include "radeon_gpu_analyzer_backend/be_data_types.h"
#include "radeon_gpu_analyzer_backend/be_include.h"

namespace beKA
{
    class BeStaticIsaAnalyzer
    {
    public:
    // Pre-processes the ISA disassembly to allow it to be parsed by the static analysis engine.
    static beStatus PreprocessIsaFile(const std::string& isa_filename, const std::string& output_filename);

    // Perform live register analysis on the ISA disassembly contained in the given file,
    // and dump the analysis output to another file.
    static beStatus PerformLiveRegisterAnalysis(const gtString& isa_filename, const gtString& target,
        const gtString& output_filename, beWaveSize wave_size, bool should_print_cmd, bool is_reg_type_sgpr);

    // Generate control flow graph for the ISA disassembly contained in the given file, and dump the analysis output to another file.
    static beStatus GenerateControlFlowGraph(const gtString& isa_filename, const gtString& target, const gtString& output_filename, bool is_per_instruction, bool should_print_cmd);

private:
    // No instances.
    BeStaticIsaAnalyzer(const BeStaticIsaAnalyzer& other);
    BeStaticIsaAnalyzer() = default;
    ~BeStaticIsaAnalyzer() = default;
};
}

#endif // RGA_RADEONGPUANALYZERBACKEND_SRC_BE_STATIC_ISA_ANALYZER_H_

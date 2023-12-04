//=================================================================
// Copyright 2020 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

#ifndef RGA_RADEONGPUANALYZERBACKEND_SRC_BE_STATIC_ISA_ANALYZER_H_
#define RGA_RADEONGPUANALYZERBACKEND_SRC_BE_STATIC_ISA_ANALYZER_H_

// Infra.
#ifdef _WIN32
    #pragma warning(push)
    #pragma warning(disable:4309)
#endif
#include "external/amdt_base_tools/Include/gtString.h"
#ifdef _WIN32
    #pragma warning(pop)
#endif

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

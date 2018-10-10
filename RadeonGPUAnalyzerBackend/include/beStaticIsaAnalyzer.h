//=================================================================
// Copyright 2017 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

#ifndef beStaticIsaAnalyzer_h__
#define beStaticIsaAnalyzer_h__

// Infra.
#include <AMDTBaseTools/Include/gtString.h>

// Local.
#include <RadeonGPUAnalyzerBackend/include/beInclude.h>

namespace beKA
{
class RGA_BACKEND_DECLDIR beStaticIsaAnalyzer
{
public:

    /// Perform live register analysis on the ISA contained in the given file,
    /// and dump the analysis output to another file.
    /// Params:
    ///     isaFileName: the file name that contains the ISA to be analyzed.
    ///     outputFileName: the output file name (will contain the analysis output).
    ///     printCmd: print the command line.
    /// Returns: true if the operation succeeded, false otherwise.
    static beStatus PerformLiveRegisterAnalysis(const gtString& isaFileName, const gtString& outputFileName, bool printCmd);

    /// Generate control flow graph for the given ISA code.
    /// Params:
    ///     isaFileName: the file name that contains the ISA to be analyzed.
    ///     outputFileName: the output file name (will contain the graph representation in dot format).
    //      perInst:  generate per-instruction CFG.
    ///     printCmd: print the command line.
    /// Returns: true if the operation succeeded, false otherwise.
    static beStatus GenerateControlFlowGraph(const gtString& isaFileName, const gtString& outputFileName, bool perInst, bool printCmd);

private:
    // No instances.
    beStaticIsaAnalyzer(const beStaticIsaAnalyzer& other);
    beStaticIsaAnalyzer();
    ~beStaticIsaAnalyzer();
};
}
#endif // beStaticIsaAnalyzer_h__
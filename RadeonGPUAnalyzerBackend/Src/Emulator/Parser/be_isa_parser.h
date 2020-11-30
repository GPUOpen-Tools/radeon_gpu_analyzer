//=============================================================
// Copyright (c) 2013 Advanced Micro Devices, Inc.
//=============================================================

#ifndef RGA_RADEONGPUANALYZERBACKEND_SRC_EMULATOR_PARSER_BE_ISA_PARSER_H_
#define RGA_RADEONGPUANALYZERBACKEND_SRC_EMULATOR_PARSER_BE_ISA_PARSER_H_

#include <iostream>
#include <vector>
#include <map>
#include <sstream>
#include <set>

#ifdef _WIN32
    #pragma warning(push)
    #pragma warning(disable:4309)
#endif
#include "be_parser_si.h"
#include "be_isa_program_graph.h"
#include "RadeonGPUAnalyzerBackend/Src/be_include.h"
#ifdef _WIN32
    #pragma warning(pop)
#endif

// Constants.
static const char* kStrHsailDisassemblyTokenStart = "Disassembly for ";
static const char* kStrHsailDisassemblyTokenEnd = "\nend\n";

class ParserIsa
{
public:
    explicit ParserIsa(ParserSi::LoggingCallBackFuncP log_function = nullptr);

    // dtor
    ~ParserIsa();

    // Parse the 32 instruction.
    bool Parse(const std::string& isa_line, GDT_HW_GENERATION asic_generation, Instruction::Instruction32Bit instruction_hex,
               const std::string& src_line, int srcLineNum, bool is_literal_32b = false, uint32_t literal_32b = 0,
               int label = kNoLabel, int goto_label = kNoLabel, int line_count = 0);

    // Parse the 64 instruction.
    bool Parse(const std::string& isa_line, GDT_HW_GENERATION asic_generation, Instruction::Instruction64Bit instruction_hex,
               const std::string& src_line, int srcLineNum, int label = kNoLabel,
               int goto_label = kNoLabel, int line_count = 0);

    // Parse the ISA.
    bool Parse(const std::string& isa);

    // Parse the ISA and retrieve its size.
    bool ParseForSize(const std::string& isa);

    // Splits the given isa source code line to a set of strings:
    // instruction_opcode - string representation of the instruction's opcode.
    // params - string representation of the instruction's parameters.
    // binary_representation - string representation of the instruction's binary representation.
    // offset - string representation of the instruction's offset within the program.
    bool SplitIsaLine(const std::string& isa_source_code_line, std::string& instruction_opcode,
        std::string& params, std::string& binary_representation, std::string& offset) const;

    // Extracts the statistics from ISA that was produced from HSAIL-path compilation.
    // hsailIsa - the disassembled ISA (which was produced from HSAIL-path compilation).
    static bool ParseHsailStatistics(const std::string& hsailIsa, beKA::AnalysisData& stats);

    // Get all ISA instructions for the program.
    const std::vector<Instruction*>& GetInstructions() const { return instructions_;}

    unsigned int GetCodeLength() const { return code_len_; }

private:
    // Reset all instruction counters.
    void ResetInstsCounters();

    // Get an ISA line and return the label if any.
    int GetLabel(const std::string& isa_line);

    // Accepts an ISA disassembly line and returns the goto label (if this is a branch instruction).
    int GetGotoLabel(const std::string& isa_line);

    // Parse the ISA disassembly line by line, and store the instructions internally.
    bool ParseToVector(const std::string& isa);

    // Extracts a numeric value from an ISA string that was produced from HSAIL path.
    static void ExtractHsailIsaNumericValue(const std::string& hsail_isa,
        const std::string value_token, CALuint64& value_buffer);

    unsigned int sgprs_ = 0;
    unsigned int vgprs_ = 0;
    unsigned int code_len_ = 0;

    // all instructions generated for the ISA
    std::vector<Instruction*> instructions_;

    // The map between Parser`s instruction kind identifier and the parser
    std::map<Instruction::InstructionSet, ParserSi*> parser_si_;

    // the ISA program graph
    ISAProgramGraph isa_graph_;
};

#endif //RGA_RADEONGPUANALYZERBACKEND_SRC_EMULATOR_PARSER_BE_ISA_PARSER_H_

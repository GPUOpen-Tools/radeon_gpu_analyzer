//=================================================================
// Copyright 2020 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

#ifndef RGA_RADEONGPUANALYZERBACKEND_SRC_EMULATOR_PARSER_BE_PARSER_SI_VOP_H_
#define RGA_RADEONGPUANALYZERBACKEND_SRC_EMULATOR_PARSER_BE_PARSER_SI_VOP_H_

#include "be_parser_si.h"

// Parser for the Southern Island [SI] VOP instructions.
class ParserSiVop : public ParserSi
{
public:
    ParserSiVop() = default;
    ~ParserSiVop() = default;

    virtual ParserSi::kaStatus Parse(GDT_HW_GENERATION hw_generation, Instruction::Instruction32Bit hex_instruction,
        Instruction*& instruction, bool is_literal_32b, uint32_t literal_32b, int label = kNoLabel, int goto_label = kNoLabel) override;

    virtual ParserSi::kaStatus Parse(GDT_HW_GENERATION hw_generation, Instruction::Instruction64Bit hex_instruction,
        Instruction*& instruction, int label = kNoLabel, int goto_label = kNoLabel) override;

private:
    // Get encoding for 32-bit VOP instruction.
    static VOPInstruction::Encoding GetInstructionType(Instruction::Instruction32Bit hex_instruction);

    // Get encoding for 64-bit VOP instruction.
    static VOPInstruction::Encoding GetInstructionType(Instruction::Instruction64Bit hex_instruction);
};

#endif //RGA_RADEONGPUANALYZERBACKEND_SRC_EMULATOR_PARSER_BE_PARSER_SI_VOP_H_

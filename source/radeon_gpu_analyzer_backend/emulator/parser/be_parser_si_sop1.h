//=================================================================
// Copyright 2020 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

#ifndef RGA_RADEONGPUANALYZERBACKEND_SRC_EMULATOR_PARSER_BE_PARSER_SI_SOP1_H_
#define RGA_RADEONGPUANALYZERBACKEND_SRC_EMULATOR_PARSER_BE_PARSER_SI_SOP1_H_

#include "be_parser_si.h"

// Parser for the Southern Island [SI] SOP1 instructions.
class ParserSiSop1 : public ParserSi
{
public:
    ParserSiSop1() = default;
    ~ParserSiSop1() = default;

    virtual ParserSi::kaStatus Parse(GDT_HW_GENERATION hw_generation, Instruction::Instruction32Bit hex_instruction,
        Instruction * &instruction, bool is_literal_32b, uint32_t literal_32b, int label, int goto_label) override;

    virtual ParserSi::kaStatus Parse(GDT_HW_GENERATION hw_generation, Instruction::Instruction64Bit hex_instruction,
        Instruction * &instruction, int label, int goto_label) override;

private:
    // SOP1 Instruction`s fields masks
    // Note regarding name format of enum value names:
    // There is legacy code that heavily uses macros which makes assumption on this specific name format.
    // Therefore, we did not convert these enumeration values to the kAbcDef format.
    enum SOP1Mask
    {
        //    SSRC0 [7:0]
        SOP1Mask_SSRC = 0x000000FF,
        //    OP     [15:8]
        SOP1Mask_OP   =   0x000000FF << 8,
        //    SDST [22:16]
        SOP1Mask_SDST = 0x0000007F << 16,
    };

    // Get SOP1 instruction`s SSRC0 field.
    static SOP1Instruction::SSRC GetSSRC0(Instruction::Instruction32Bit hex_instruction, unsigned int& ridx);

    // Get SOPC instruction`s OP field.
    static SISOP1Instruction::OP GetSISOP1Op(Instruction::Instruction32Bit hex_instruction);

    // Get SOPC instruction`s OP field.
    static VISOP1Instruction::OP GetVISOP1Op(Instruction::Instruction32Bit hex_instruction);

    // Get SOPC instruction`s OP field.
    static G9SOP1Instruction::OP GetG9SOP1Op(Instruction::Instruction32Bit hex_instruction);

    // Get SOP1 instruction`s SDST field.
    static SOP1Instruction::SDST GetSdst(Instruction::Instruction32Bit hex_instruction, unsigned int& ridx);
};

#endif //RGA_RADEONGPUANALYZERBACKEND_SRC_EMULATOR_PARSER_BE_PARSER_SI_SOP1_H_


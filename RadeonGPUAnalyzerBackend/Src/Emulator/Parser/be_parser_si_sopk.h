//=================================================================
// Copyright 2020 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

#ifndef RGA_RADEONGPUANALYZERBACKEND_SRC_EMULATOR_PARSER_BE_PARSER_SI_SOPK_H_
#define RGA_RADEONGPUANALYZERBACKEND_SRC_EMULATOR_PARSER_BE_PARSER_SI_SOPK_H_

#include "be_parser_si.h"

// Parser for the Southern Island [SI] SOPK instructions.
class ParserSiSopk : public ParserSi
{
public:
    ParserSiSopk() = default;
    ~ParserSiSopk() = default;

    virtual ParserSi::kaStatus Parse(GDT_HW_GENERATION hw_generation, Instruction::Instruction32Bit hex_instruction,
        Instruction * &instruction, bool is_literal_32b, uint32_t literal_32b, int label = kNoLabel, int goto_label = kNoLabel) override;

    virtual ParserSi::kaStatus Parse(GDT_HW_GENERATION hw_generation, Instruction::Instruction64Bit hex_instruction,
        Instruction * &instruction, int label = kNoLabel, int goto_label = kNoLabel) override;

private:
    // SOPK Instruction`s fields masks.
    // Note regarding name format of enum value names:
    // There is legacy code that heavily uses macros which makes assumption on this specific name format.
    // Therefore, we did not convert these enumeration values to the kAbcDef format.
    enum SOPKMask
    {
        //    SIMM16 [15:0]
        SOPKMask_SIMM16 = 0x0000FFFF,
        //    SDST [22:16]
        SOPKMask_SDST   =   0x0000007F << 16,
        //    OP [27:23]
        SOPKMask_OP = 0x0000001F << 23,
    };

    // Get SOPK instruction SIMM16 field.
    static SOPKInstruction::SIMM16 GetSiMm16(Instruction::Instruction32Bit hex_instruction, unsigned int& ridx);

    // Get SOPK instruction OP field for SI.
    static SISOPKInstruction::OP GetSiSopkOp(Instruction::Instruction32Bit hex_instruction);

    // Get SOPK instruction OP field for VI.
    static VISOPKInstruction::OP GetViSopkOp(Instruction::Instruction32Bit hex_instruction);

    // Get SOPK instruction OP field for Vega.
    static G9SOPKInstruction::OP GetVegaSopkOp(Instruction::Instruction32Bit hex_instruction);

    // Get SOPK instruction SDST field.
    static SOPKInstruction::SDST GetSdst(Instruction::Instruction32Bit hex_instruction, unsigned int& ridx);
};

#endif //RGA_RADEONGPUANALYZERBACKEND_SRC_EMULATOR_PARSER_BE_PARSER_SI_SOPK_H_


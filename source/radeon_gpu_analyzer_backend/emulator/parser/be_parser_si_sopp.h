//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header of parser for the Southern Island [SI] SOPP instructions.
//=============================================================================

#ifndef RGA_RADEONGPUANALYZERBACKEND_SRC_EMULATOR_PARSER_BE_PARSER_SI_SOPP_H_
#define RGA_RADEONGPUANALYZERBACKEND_SRC_EMULATOR_PARSER_BE_PARSER_SI_SOPP_H_

#include "be_parser_si.h"

// Parser for the Southern Island [SI] SOPP instructions.
class ParserSiSopp : public ParserSi
{
public:

    ParserSiSopp() = default;
    ~ParserSiSopp() = default;

    virtual ParserSi::kaStatus Parse(GDT_HW_GENERATION hw_generation, Instruction::Instruction32Bit hex_instruction,
        Instruction * &instruction, bool is_literal_32bit, uint32_t literal_32bit, int label = kNoLabel, int goto_label = kNoLabel) override;

    virtual ParserSi::kaStatus Parse(GDT_HW_GENERATION hw_generation, Instruction::Instruction64Bit hex_instruction,
        Instruction * &instruction, int label = kNoLabel, int goto_label = kNoLabel) override;

private:
    // SOPP Instruction`s fields masks.
    // Note regarding name format of enum value names:
    // There is legacy code that heavily uses macros which makes assumption on this specific name format.
    // Therefore, we did not convert these enumeration values to the kAbcDef format.
    enum SOPPMask
    {
        //    SIMM [15:0]
        SOPPMask_SIMM16 = 0x0000FFFF,
        //    OP     [26:22]
        SOPPMask_OP   =   0x0000007F << 16
    };

    // Get SOPP instruction SIMM16 field.
    static SOPPInstruction::SIMM16 GetSimm16(Instruction::Instruction32Bit hex_instruction);

    // Get SOPP instruction OP field for SI.
    static SISOPPInstruction::OP GetSiSoppOp(Instruction::Instruction32Bit hex_instruction);

    // Get SOPP instruction OP field for VI.
    static VISOPPInstruction::OP GetViSoppOp(Instruction::Instruction32Bit hex_instruction);
};

#endif //RGA_RADEONGPUANALYZERBACKEND_SRC_EMULATOR_PARSER_BE_PARSER_SI_SOPP_H_


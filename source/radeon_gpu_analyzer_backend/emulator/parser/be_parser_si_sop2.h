//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header of parser for the Southern Island [SI] SOP2 instructions.
//=============================================================================

#ifndef RGA_RADEONGPUANALYZERBACKEND_SRC_EMULATOR_PARSER_BE_PARSER_SI_SOP2_H_
#define RGA_RADEONGPUANALYZERBACKEND_SRC_EMULATOR_PARSER_BE_PARSER_SI_SOP2_H_

#include "be_parser_si.h"

// Parser for the Southern Island [SI] SOP2 instructions.
class ParserSiSop2 : public ParserSi
{
public:
    ParserSiSop2() = default;
    ~ParserSiSop2() = default;

    virtual ParserSi::kaStatus Parse(GDT_HW_GENERATION hw_generation, Instruction::Instruction32Bit hex_instruction, Instruction*& instruction,
        bool is_literal_32b, uint32_t literal_32b, int label = -1, int goto_label = -1) override;
    virtual ParserSi::kaStatus Parse(GDT_HW_GENERATION hw_generation, Instruction::Instruction64Bit hex_instruction, Instruction*& instruction,
        int label = -1, int goto_label = -1) override;

private:

    // SOP2 Instruction`s fields masks
    // Note regarding name format of enum value names:
    // There is legacy code that heavily uses macros which makes assumption on this specific name format.
    // Therefore, we did not convert these enumeration values to the kAbcDef format.
    enum SOPCMask
    {
        //    SSRC0 [7:0]
        SOP2Mask_SSRC0 = 0x000000FF,
        //    SSRC1 [15:8]
        SOP2Mask_SSRC1 = 0x000000FF << 8,
        //    SDST [22:16]
        SOP2Mask_SDST = 0x0000007F << 16,
        //    OP     [22:16]
        SOP2Mask_OP   =   0x0000007F << 23
    };

    // Get SOP2 instruction`s SSRC field.
    static SOP2Instruction::SSRC GetSsrc(Instruction::Instruction32Bit hex_instruction, unsigned int& ridx, unsigned int idx_ssrc);

    // Get SOP2 instruction`s SDST field.
    static SOP2Instruction::SDST GetSdst(Instruction::Instruction32Bit hex_instruction, unsigned int& ridx);

    // Get SOP2 instruction`s OP field.
    static SISOP2Instruction::OP GetSiSop2Op(Instruction::Instruction32Bit hex_instruction);

    // Get SOP2 instruction`s OP field.
    static VISOP2Instruction::OP GetViSop2Op(Instruction::Instruction32Bit hex_instruction);

    // Get SOP2 instruction`s OP field.
    static G9SOP2Instruction::OP GetG9Sop2Op(Instruction::Instruction32Bit hex_instruction);
};

#endif //RGA_RADEONGPUANALYZERBACKEND_SRC_EMULATOR_PARSER_BE_PARSER_SI_SOP2_H_


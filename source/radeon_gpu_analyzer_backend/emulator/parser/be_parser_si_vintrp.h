//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header of parser for the Southern Island [SI] VINTRP instructions.
//=============================================================================

#ifndef RGA_RADEONGPUANALYZERBACKEND_SRC_EMULATOR_PARSER_BE_PARSER_SI_VINTRP_H_
#define RGA_RADEONGPUANALYZERBACKEND_SRC_EMULATOR_PARSER_BE_PARSER_SI_VINTRP_H_

#include "be_parser_si.h"

// Parser for the Southern Island [SI] VINTRP instructions.
class ParserSiVintrp : public ParserSi
{
public:
    ParserSiVintrp() = default;
    ~ParserSiVintrp() = default;

    virtual ParserSi::kaStatus Parse(GDT_HW_GENERATION hw_generation, Instruction::Instruction32Bit hex_instruction,
        Instruction*& instruction, bool is_literal_32b, uint32_t literal_32b, int label = kNoLabel, int goto_label = kNoLabel) override;

    virtual ParserSi::kaStatus Parse(GDT_HW_GENERATION hw_generation, Instruction::Instruction64Bit hex_instruction,
        Instruction*& instruction, int label = kNoLabel, int goto_label = kNoLabel) override;
private:
    // VINTRP Instruction`s fields masks.
    // Note regarding name format of enum value names:
    // There is legacy code that heavily uses macros which makes assumption on this specific name format.
    // Therefore, we did not convert these enumeration values to the kAbcDef format.
    enum VINTRPMask
    {
        //    VSRC [7:0]
        VINTRPMask_VSRC = 0x00000FFF,
        //    ATTRCHAN [9:8]
        VINTRPMask_ATTRCHAN = 0x00000003 << 8,
        //    ATTR [15:10]
        VINTRPMask_ATTR = 0x0000003F << 10,
        //    OP [27:23]
        VINTRPMask_OP = 0x00000003 << 16,
        //    VDST [25:18]
        VINTRPMask_VDST = 0x000000FF << 18,
    };

    // Get VINTRP instruction`s VSRC field.
    static VINTRPInstruction::VSRC GetVsrc(Instruction::Instruction32Bit hex_instruction);

    // Get VINTRP instruction`s ATTRCHAN field.
    static VINTRPInstruction::ATTRCHAN GetATTRCHAN(Instruction::Instruction32Bit hex_instruction);

    // Get VINTRP instruction`s SIMM16 field.
    static VINTRPInstruction::ATTR GetATTR(Instruction::Instruction32Bit hex_instruction);

    // Get VINTRP instruction`s OP field.
    static SIVINTRPInstruction::OP GetSIVINTRPOp(Instruction::Instruction32Bit hex_instruction);

    // Get VINTRP instruction`s OP field.
    static VIVINTRPInstruction::OP GetVIVINTRPOp(Instruction::Instruction32Bit hex_instruction);

    // Get VINTRP instruction`s VDST field.
    static VINTRPInstruction::VDST GetVDST(Instruction::Instruction32Bit hex_instruction);
};

#endif //RGA_RADEONGPUANALYZERBACKEND_SRC_EMULATOR_PARSER_BE_PARSER_SI_VINTRP_H_


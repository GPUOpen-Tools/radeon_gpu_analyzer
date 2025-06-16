//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header of parser for the Southern Island [SI] SMRD instructions.
//=============================================================================

#ifndef RGA_RADEONGPUANALYZERBACKEND_SRC_EMULATOR_PARSER_BE_PARSER_SI_SMRD_H_
#define RGA_RADEONGPUANALYZERBACKEND_SRC_EMULATOR_PARSER_BE_PARSER_SI_SMRD_H_

#include "be_parser_si.h"

// Parser for the Southern Island [SI] SMRD instructions.
class ParserSiSmrd : public ParserSi
{
public:
    ParserSiSmrd() = default;
    ~ParserSiSmrd() = default;

    virtual ParserSi::kaStatus Parse(GDT_HW_GENERATION hw_generation, Instruction::Instruction32Bit hex_instruction,
        Instruction*& instruction, bool is_literal_32b, uint32_t literal_32b, int label, int goto_label) override;

    virtual ParserSi::kaStatus Parse(GDT_HW_GENERATION hw_generation, Instruction::Instruction64Bit hex_instruction,
        Instruction*& instruction, int label, int goto_label);

private:

    // SMRD Instruction`s fields masks.
    // Note regarding name format of enum value names:
    // There is legacy code that heavily uses macros which makes assumption on this specific name format.
    // Therefore, we did not convert these enumeration values to the kAbcDef format.
    enum SMRDMask
    {
        //    OFFSET [7:0]
        SMRDMask_OFFSET = 0x000000FF,
        //    IMM    [8:8]
        SMRDMask_IIM    = 0x00000001 << 8,
        //    SBASE  [14:9]
        SMRDMask_SBASE =  0x0000003F << 9,
        //    SDST   [21:15]
        SMRDMask_SDST =   0x0000007F << 15,
        //    OP     [26:22]
        SMRDMask_OP   =   0x0000001F << 22,
        //    OP     [25:18]
        SMEMMask_OP   =   0x000000FF << 18,
    };

    // Get SMRD instruction`s OFFSET field.
    static SMRDInstruction::OFFSET GetOffset(Instruction::Instruction32Bit hex_instruction);

    // Get SMRD instruction`s IMM field.
    static SMRDInstruction::IMM GetImm(Instruction::Instruction32Bit hex_instruction);

    // Get SMRD instruction`s SBASE field.
    static SMRDInstruction::SBASE GetSBase(Instruction::Instruction32Bit hex_instruction);

    // Get SMRD instruction`s SDST field.
    static SMRDInstruction::SDST GetSdst(Instruction::Instruction32Bit hex_instruction, unsigned int& ridx);

    // Get SMRD instruction`s OP field.
    static SISMRDInstruction::OP GetSiSmrdOp(Instruction::Instruction32Bit hex_instruction);

    // Get SMRD instruction`s OP field.
    static VISMEMInstruction::OP GetViSmrdOp(Instruction::Instruction64Bit hex_instruction);
};

#endif //RGA_RADEONGPUANALYZERBACKEND_SRC_EMULATOR_PARSER_BE_PARSER_SI_SMRD_H_

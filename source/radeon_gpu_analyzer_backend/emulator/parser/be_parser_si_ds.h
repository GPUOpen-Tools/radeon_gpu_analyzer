//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header of parser for the Southern Island [SI] DS instructions.
//=============================================================================

#ifndef RGA_RADEONGPUANALYZERBACKEND_SRC_EMULATOR_PARSER_BE_PARSER_SI_DS_H_
#define RGA_RADEONGPUANALYZERBACKEND_SRC_EMULATOR_PARSER_BE_PARSER_SI_DS_H_

#include "be_parser_si.h"

// Parser for the Southern Island [SI] DS instructions.
class ParserSiDs : public ParserSi
{
public:
    ParserSiDs() = default;
    ~ParserSiDs() = default;

    virtual ParserSi::kaStatus Parse(GDT_HW_GENERATION hw_generation, Instruction::Instruction32Bit hex_instruction,
        Instruction*& instruction, bool is_literal_32b, uint32_t literal_32b, int label = kNoLabel , int goto_label = kNoLabel) override;

    virtual ParserSi::kaStatus Parse(GDT_HW_GENERATION hwGen, Instruction::Instruction64Bit hexInstruction,
        Instruction*& instruction, int label_ = kNoLabel , int iGotoLabel = kNoLabel) override;
private:

    // DS Instruction fields masks.
    // Note regarding name format of enum values:
    // There is legacy code that heavily uses macros which makes assumption on this specific name format.
    // Therefore, we did not convert these enumeration values to the kAbcDef format.
    enum DSMask
    {
        // Low 32 bits
        //    OFFSET0 [7:0]
        DSMask_OFFSET0 = 0x000000FF,
        //    OFFSET1 [15:8]
        DSMask_OFFSET1 = 0x000000FF << 8,
        //    GDS [17:17]
        DSMask_GDS = 0x00000001 << 17,
        //    OP [25:18]
        DSMask_OP = 0x000000FF << 18,

        // High 32 bits
        //    ADDR [39:32]
        DSMask_ADDR   =   0x000000FF,
        //    DATA0 [47:40]
        DSMask_DATA0   =   0x000000FF << 8,
        //    DATA1 [55:48]
        DSMask_DATA1   =   0x000000FF << 16,
        //    VDST [63:56]
        DSMask_VDST   =   0x000000FF << 24,
    };

    // Get the OFFSET0 [7:0] / OFFSET1  for the given 64 bit hexadecimal instruction.
    static DSInstruction::OFFSET GetOffset(Instruction::Instruction64Bit, const unsigned char offset_index);

    // Get the GDS [17:17] for the given 64 bit hexadecimal instruction.
    static DSInstruction::GDS GetGDS(Instruction::Instruction64Bit);

    // Get the OP [25:18] for the given 64 bit hexadecimal instruction.
    static SIDSInstruction::OP GetSIDSOp(Instruction::Instruction64Bit);

    // Get the OP [25:18] for the given 64 bit hexadecimal instruction.
    static VIDSInstruction::OP GetVIDSOp(Instruction::Instruction64Bit);

    // Get the OP [25:18] for the given 64 bit hexadecimal instruction.
    static G9DSInstruction::OP GetG9DSOp(Instruction::Instruction64Bit);

    // Get the ADDR [39:32] for the given 64 bit hexadecimal instruction.
    static DSInstruction::ADDR GetADDR(Instruction::Instruction64Bit);

    // Get the DATA for the given 64 bit hexadecimal instruction using the given index.
    static DSInstruction::DATA GetDATA(Instruction::Instruction64Bit, const unsigned int data_index);

    // Get the VDST [63:56] for the given 64 bit hexadecimal instruction.
    static DSInstruction::VDST GetVDST(Instruction::Instruction64Bit);
};

#endif //RGA_RADEONGPUANALYZERBACKEND_SRC_EMULATOR_PARSER_BE_PARSER_SI_DS_H_


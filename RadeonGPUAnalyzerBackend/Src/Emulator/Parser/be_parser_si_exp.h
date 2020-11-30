//=================================================================
// Copyright 2020 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================
#ifndef RGA_RADEONGPUANALYZERBACKEND_SRC_EMULATOR_PARSER_BE_PARSER_SI_EXP_H_
#define RGA_RADEONGPUANALYZERBACKEND_SRC_EMULATOR_PARSER_BE_PARSER_SI_EXP_H_

#include "be_parser_si.h"

// Parser for the Southern Island[SI] EXP instructions.
class ParserSiExp : public ParserSi
{
public:
    ParserSiExp() = default;
    ~ParserSiExp() = default;

    virtual ParserSi::kaStatus Parse(GDT_HW_GENERATION hw_generation, Instruction::Instruction64Bit hex_instruction,
        Instruction*& instruction, int label = kNoLabel, int goto_label = kNoLabel) override;

    virtual ParserSi::kaStatus Parse(GDT_HW_GENERATION hw_generation, Instruction::Instruction32Bit
        hex_instruction, Instruction*& instruction, bool is_literal_32b, uint32_t literal_32b, int label = kNoLabel, int goto_label = kNoLabel) override;

private:
    // EXP Instruction`s fields masks.
    enum EXTMask
    {
        // Low 32 bits.
        //        EN [3:0]
        kExpMaskEn = 0x0000000F,
        //        TGT [9:4]
        kExpMaskTgt = 0x0000003F << 4,
        //        COMPR [10:10]
        kExpMaskCompr = 0x00000001 << 10,
        //        DONE [11:11]
        kExpMaskDone = 0x00000001 << 11,
        //        VM [12:12]
        kExpMaskVm = 0x00000001 << 12,

        // High 32 bits.
        //        VSRC0 [39:32]
        kExpMaskVsrc0 = 0x000000FF ,
        //        VSRC1 [47:40]
        kExpMaskVsrc1 = 0x000000FF << 8,
        //        VSRC2 [55:48]
        kExpMaskVsrc2 = 0x000000FF << 16,
        //        VSRC3 [63:56]
        kExpMaskVsrc3 = 0x000000FF << 24
    };

    // Get the EN [3:0] for the given 64 bit hexadecimal instruction.
    static EXPInstruction::EN GetEn(Instruction::Instruction64Bit);

    // Get the TGT [9:4] for the given 64 bit hexadecimal instruction.
    static EXPInstruction::TGT GetTgt(Instruction::Instruction64Bit);

    // Get the COMPR [10:10] for the given 64 bit hexadecimal instruction.
    static EXPInstruction::COMPR GetCompr(Instruction::Instruction64Bit);

    // Get the DONE [11:11] for the given 64 bit hexadecimal instruction.
    static EXPInstruction::DONE GetDone(Instruction::Instruction64Bit);

    // Get the VM [12:12] for the given 64 bit hexadecimal instruction.
    static EXPInstruction::VM GetVm(Instruction::Instruction64Bit);

    // Get the VSRC for the given 64 bit hexadecimal instruction.
    static EXPInstruction::VSRC GetVsrc(Instruction::Instruction64Bit, const unsigned int vsrc_index);
};

#endif //RGA_RADEONGPUANALYZERBACKEND_SRC_EMULATOR_PARSER_BE_PARSER_SI_EXP_H_

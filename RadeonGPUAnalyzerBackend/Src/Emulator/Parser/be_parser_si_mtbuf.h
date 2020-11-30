//=================================================================
// Copyright 2020 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

#ifndef RGA_RADEONGPUANALYZERBACKEND_SRC_EMULATOR_PARSER_BE_PARSER_SI_MTBUF_H_
#define RGA_RADEONGPUANALYZERBACKEND_SRC_EMULATOR_PARSER_BE_PARSER_SI_MTBUF_H_

#include "be_parser_si.h"

// Parser for the Southern Island [SI] MTBUF instructions.
class ParserSiMtbuf : public ParserSi
{
public:
    ParserSiMtbuf() = default;
    ~ParserSiMtbuf() = default;

    virtual ParserSi::kaStatus Parse(GDT_HW_GENERATION hw_generation, Instruction::Instruction32Bit hex_instruction,
        Instruction*& instruction, bool is_literal_32b, uint32_t literal_32b, int label = kNoLabel, int goto_label = kNoLabel) override;

    virtual ParserSi::kaStatus Parse(GDT_HW_GENERATION hw_generation, Instruction::Instruction64Bit hex_instruction,
        Instruction*& instruction, int label = kNoLabel, int goto_label = kNoLabel) override;

private:
    // MTBUF Instruction`s fields masks.
    // Note regarding name format of enum value names:
    // There is legacy code that heavily uses macros which makes assumption on this specific name format.
    // Therefore, we did not convert these enumeration values to the kAbcDef format.
    enum MTBUFMask
    {
        // Low 32 bits
        //    OFFSET [11:0]
        MTBUFMask_OFFSET = 0x00000FFF,
        //    OFFEN  [12:12]
        MTBUFMask_OFFEN = 0x00000001 << 12,
        //    IDXEN  [13:13]
        MTBUFMask_IDXEN = 0x00000001 << 13,
        //    GLC    [14:14]
        MTBUFMask_GLC = 0x00000001 << 14,
        //    ADDR64 [15:15]
        MTBUFMask_ADDR64 = 0x00000001 << 15,
        //    OP     [18:16]
        MTBUFMask_OP = 0x00000007 << 16,
        //    DFMT   [22:19]
        MTBUFMask_DFMT = 0x0000000F << 19,
        //    NFMT   [25:23]
        MTBUFMask_NFMT = 0x00000007 << 23,

        // High 32 bits
        //    VADDR  [39:32]
        MTBUFMask_VADDR = 0x000000FF,
        //    VDATA  [47:40]
        MTBUFMask_VDATA = 0x000000FF << 8,
        //    SRSRC  [52:48]
        MTBUFMask_SRSRC = 0x0000001F << 16,
        //    SLC    [54:54]
        MTBUFMask_SLC = 0x00000001 << 22,
        //    TFE    [55:55]
        MTBUFMask_TFE = 0x00000001 << 23,
        //    SOFFSET [63:56]
        MTBUFMask_SOFFSET = 0x000000FF << 24
    };

    // Get the OFFSET [11:0] for the given 64 bit hexadecimal instruction.
    static MTBUFInstruction::OFFSET GetOffset(Instruction::Instruction64Bit hex_instruction);

    // Get the OFFEN  [12:12] for the given 64 bit hexadecimal instruction.
    static MTBUFInstruction::OFFEN GetOffen(Instruction::Instruction64Bit hex_instruction);

    // Get the IDXEN  [13:13] for the given 64 bit hexadecimal instruction.
    static MTBUFInstruction::IDXEN GetIdxen(Instruction::Instruction64Bit hex_instruction);

    // Get the GLC    [14:14] for the given 64 bit hexadecimal instruction.
    static MTBUFInstruction::GLC GetGlc(Instruction::Instruction64Bit hex_instruction);

    // Get the  ADDR64 [15:15] for the given 64 bit hexadecimal instruction.
    static MTBUFInstruction::ADDR64 GetAddr64(Instruction::Instruction64Bit hex_instruction);

    // Get the OP     [18:16] for the given 64 bit hexadecimal instruction.
    static SIMTBUFInstruction::OP GetSiOpMtbuf(Instruction::Instruction64Bit hex_instruction, Instruction::InstructionCategory& instruction_kind);

    // Get the OP     [18:16] for the given 64 bit hexadecimal instruction.
    static VIMTBUFInstruction::OP GetViOpMtbuf(Instruction::Instruction64Bit hex_instruction, Instruction::InstructionCategory& instruction_kind);

    // Get the DFMT   [22:19] for the given 64 bit hexadecimal instruction.
    static MTBUFInstruction::DFMT GetDfmt(Instruction::Instruction64Bit hex_instruction);

    // Get the NFMT   [25:23] for the given 64 bit hexadecimal instruction.
    static MTBUFInstruction::NFMT GetNfmt(Instruction::Instruction64Bit hex_instruction);

    // Get the VADDR  [39:32] for the given 64 bit hexadecimal instruction.
    static MTBUFInstruction::VADDR GetVaddr(Instruction::Instruction64Bit hex_instruction);

    // Get the VDATA  [47:40] for the given 64 bit hexadecimal instruction.
    static MTBUFInstruction::VDATA GetVdata(Instruction::Instruction64Bit hex_instruction);

    // Get the SRSRC  [52:48] for the given 64 bit hexadecimal instruction.
    static MTBUFInstruction::SRSRC GetSrsrc(Instruction::Instruction64Bit hex_instruction);

    // Get the SLC    [54:54] for the given 64 bit hexadecimal instruction.
    static MTBUFInstruction::SLC GetSlc(Instruction::Instruction64Bit hex_instruction);

    // Get the TFE    [55:55] for the given 64 bit hexadecimal instruction.
    static MTBUFInstruction::TFE GetTfe(Instruction::Instruction64Bit hex_instruction);

    // Get the SOFFSET [63:56] for the given 64 bit hexadecimal instruction.
    static MTBUFInstruction::SOFFSET GetSOFFSET(Instruction::Instruction64Bit hex_instruction, unsigned int& ridx);
};

#endif //RGA_RADEONGPUANALYZERBACKEND_SRC_EMULATOR_PARSER_BE_PARSER_SI_MTBUF_H_

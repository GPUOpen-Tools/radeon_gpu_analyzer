//=================================================================
// Copyright 2020 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

#ifndef RGA_RADEONGPUANALYZERBACKEND_SRC_EMULATOR_PARSER_BE_PARSER_SI_MUBUF_H_
#define RGA_RADEONGPUANALYZERBACKEND_SRC_EMULATOR_PARSER_BE_PARSER_SI_MUBUF_H_

#include "be_parser_si.h"

// Parser for the Southern Island [SI] MUBUF instructions.
class ParserSiMubuf : public ParserSi
{
public:
    ParserSiMubuf() = default;
    ~ParserSiMubuf() = default;

    virtual ParserSi::kaStatus Parse(GDT_HW_GENERATION hwGen, Instruction::Instruction32Bit hex_instruction,
        Instruction*& instruction, bool is_literal_32b, uint32_t literal_32b, int label = kNoLabel, int goto_label = kNoLabel) override;

    virtual ParserSi::kaStatus Parse(GDT_HW_GENERATION hwGen, Instruction::Instruction64Bit hex_instruction,
        Instruction*& instruction, int label = kNoLabel, int goto_label = kNoLabel) override;

private:
    // MUBUF Instruction`s fields masks
    // Note regarding name format of enum value names:
    // There is legacy code that heavily uses macros which makes assumption on this specific name format.
    // Therefore, we did not convert these enumeration values to the kAbcDef format.
    enum MUBUFMask
    {
        // Low 32 bits
        //    OFFSET [11:0]
        MUBUFMask_OFFSET = 0x00000FFF,
        //    OFFEN  [12:12]
        MUBUFMask_OFFEN = 0x00000001 << 12,
        //    IDXEN  [13:13]
        MUBUFMask_IDXEN = 0x00000001 << 13,
        //    GLC    [14:14]
        MUBUFMask_GLC = 0x00000001 << 14,
        //    ADDR64 [15:15]
        MUBUFMask_ADDR64 = 0x00000001 << 15,
        //    LDS [16:16]
        MUBUFMask_LDS = 0x00000001 << 16,
        //    OP     [24:18]
        MUBUFMask_OP = 0x0000003F << 18,

        // High 32 bits
        //    VADDR  [39:32]
        MUBUFMask_VADDR = 0x000000FF,
        //    VDATA  [47:40]
        MUBUFMask_VDATA = 0x000000FF << 8,
        //    SRSRC  [52:48]
        MUBUFMask_SRSRC = 0x0000001F << 16,
        //    SLC    [54:54]
        MUBUFMask_SLC = 0x00000001 << 22,
        //    TFE    [55:55]
        MUBUFMask_TFE = 0x00000001 << 23,
        //    SOFFSET [63:56]
        MUBUFMask_SOFFSET = 0x000000FF << 24
    };

    // Get the OFFSET [11:0] for the given 64 bit hexadecimal instruction.
    static MUBUFInstruction::OFFSET GetOffset(Instruction::Instruction64Bit hex_instruction);

    // Get the OFFEN  [12:12] for the given 64 bit hexadecimal instruction.
    static MUBUFInstruction::OFFEN GetOffen(Instruction::Instruction64Bit hex_instruction);

    // Get the IDXEN  [13:13] for the given 64 bit hexadecimal instruction.
    static MUBUFInstruction::IDXEN GetIdxen(Instruction::Instruction64Bit hex_instruction);

    // Get the GLC    [14:14] for the given 64 bit hexadecimal instruction.
    static MUBUFInstruction::GLC GetGlc(Instruction::Instruction64Bit hex_instruction);

    // Get the  ADDR64 [15:15] for the given 64 bit hexadecimal instruction.
    static MUBUFInstruction::ADDR64 GetAddr64(Instruction::Instruction64Bit hex_instruction);

    // Get the LDS   [16:16] for the given 64 bit hexadecimal instruction.
    static MUBUFInstruction::LDS GetLds(Instruction::Instruction64Bit hex_instruction);

    // Get the OP     [18:16] for the given 64 bit hexadecimal instruction.
    static SIMUBUFInstruction::OP GetSiOpMubuf(Instruction::Instruction64Bit hex_instruction, Instruction::InstructionCategory& instruction_kind);

    // Get the OP     [18:16] for the given 64 bit hexadecimal instruction.
    static VIMUBUFInstruction::OP GetViOpMubuf(Instruction::Instruction64Bit hex_instruction, Instruction::InstructionCategory& instruction_kind);

    // Get the OP     [18:16] for the given 64 bit hexadecimal instruction.
    static G9MUBUFInstruction::OP GetVegaOpMubuf(Instruction::Instruction64Bit hex_instruction, Instruction::InstructionCategory& instruction_kind);

    // Get the VADDR  [39:32] for the given 64 bit hexadecimal instruction.
    static MUBUFInstruction::VADDR GetVaddr(Instruction::Instruction64Bit hex_instruction);

    // Get the VDATA  [47:40] for the given 64 bit hexadecimal instruction.
    static MUBUFInstruction::VDATA GetVdata(Instruction::Instruction64Bit hex_instruction);

    // Get the SRSRC  [52:48] for the given 64 bit hexadecimal instruction.
    static MUBUFInstruction::SRSRC GetSrsrc(Instruction::Instruction64Bit hex_instruction);

    // Get the SLC    [54:54] for the given 64 bit hexadecimal instruction.
    static MUBUFInstruction::SLC GetSlc(Instruction::Instruction64Bit hex_instruction);

    // Get the TFE    [55:55] for the given 64 bit hexadecimal instruction.
    static MUBUFInstruction::TFE GetTfe(Instruction::Instruction64Bit hex_instruction);

    // Get the SOFFSET [63:56] for the given 64 bit hexadecimal instruction.
    static MUBUFInstruction::SOFFSET GetSOFFSET(Instruction::Instruction64Bit hex_instruction, unsigned int& ridx);
};

#endif //RGA_RADEONGPUANALYZERBACKEND_SRC_EMULATOR_PARSER_BE_PARSER_SI_MUBUF_H_


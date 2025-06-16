//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header of parser for the Southern Island [SI] MIMG instructions.
//=============================================================================

#ifndef RGA_RADEONGPUANALYZERBACKEND_SRC_EMULATOR_PARSER_BE_PARSER_SI_MIMG_H_
#define RGA_RADEONGPUANALYZERBACKEND_SRC_EMULATOR_PARSER_BE_PARSER_SI_MIMG_H_

#include "be_parser_si.h"

// Parser for the Southern Island [SI] MIMG instructions.
class ParserSiMimg : public ParserSi
{
public:
    ParserSiMimg() = default;
    ~ParserSiMimg() = default;

    virtual ParserSi::kaStatus Parse(GDT_HW_GENERATION hw_generation, Instruction::Instruction32Bit hex_instruction,
        Instruction*& instruction, bool is_literal_32b, uint32_t literal_32b, int label = kNoLabel, int goto_label = kNoLabel) override;

    virtual ParserSi::kaStatus Parse(GDT_HW_GENERATION hw_generation, Instruction::Instruction64Bit hex_instruction,
        Instruction*& instruction, int label = kNoLabel, int goto_label = kNoLabel) override;
private:
    // MUBUF Instruction`s fields masks.
    // Note regarding name format of enum values:
    // There is legacy code that heavily uses macros which makes assumption on this specific name format.
    // Therefore, we did not convert these enumeration values to the kAbcDef format.
    enum MIMGMask
    {
        // Low 32 bits
        //    DMASK  [11:8]
        MIMGMask_DMASK = 0x0000000F << 8,
        //    UNORM  [12:12]
        MIMGMask_UNORM = 0x00000001 << 12,
        //    GLC    [13:13]
        MIMGMask_GLC = 0x00000001 << 13,
        //    DA     [14:14]
        MIMGMask_DA = 0x00000001 << 14,
        //    R128   [15:15]
        MIMGMask_R128 = 0x00000001 << 15,
        //    TFE    [16:16]
        MIMGMask_TFE = 0x00000001 << 16,
        //    LWE    [17:17]
        MIMGMask_LWE = 0x00000001 << 17,
        //    OP     [24:18]
        MIMGMask_OP = 0x000000FF << 18,
        //    SLC    [25:25]
        MIMGMask_SLC = 0x00000001 << 25,

        // High 32 bits
        //    VADDR  [39:32]
        MIMGMask_VADDR = 0x000000FF,
        //    VDATA  [47:40]
        MIMGMask_VDATA = 0x000000FF << 8,
        //    SRSRC  [52:48]
        MIMGMask_SRSRC = 0x0000001F << 16,
        //    SSAMP  [57:53]
        MIMGMask_SSAMP = 0x0000001F << 21
    };

    // Get the DMASK  [11:8] for the given 64 bit hexadecimal instruction.
    static MIMGInstruction::DMASK GetDmask(Instruction::Instruction64Bit hex_instruction);

    // Get the UNORM  [12:12] for the given 64 bit hexadecimal instruction.
    static MIMGInstruction::UNORM GetUnorm(Instruction::Instruction64Bit hex_instruction);

    // Get the GLC    [14:14] for the given 64 bit hexadecimal instruction.
    static MIMGInstruction::GLC GetGlc(Instruction::Instruction64Bit hex_instruction);

    // Get the  DA     [14:14] for the given 64 bit hexadecimal instruction.
    static MIMGInstruction::DA GetDa(Instruction::Instruction64Bit hex_instruction);

    // Get the R128   [15:15] for the given 64 bit hexadecimal instruction.
    static MIMGInstruction::R128 GetR128(Instruction::Instruction64Bit hex_instruction);

    // Get the TFE    [16:16] for the given 64 bit hexadecimal instruction.
    static MIMGInstruction::TFE GetTfe(Instruction::Instruction64Bit hex_instruction);

    // Get the LWE    [17:17] for the given 64 bit hexadecimal instruction.
    static MIMGInstruction::LWE GetLwe(Instruction::Instruction64Bit hex_instruction);

    // Get the     OP [24:18] for the given 64 bit hexadecimal instruction.
    static SIMIMGInstruction::OP GetOpSimimg(Instruction::Instruction64Bit hex_instruction, Instruction::InstructionCategory& instruction_kind);

    // Get the     OP [24:18] for the given 64 bit hexadecimal instruction.
    static VIMIMGInstruction::OP GetOpVimimg(Instruction::Instruction64Bit hex_instruction, Instruction::InstructionCategory& instruction_kind);

    // Get the SLC    [25:25] for the given 64 bit hexadecimal instruction.
    static MIMGInstruction::SLC GetSlc(Instruction::Instruction64Bit hex_instruction);

    // Get the VADDR  [39:32] for the given 64 bit hexadecimal instruction.
    static MIMGInstruction::VADDR GetVaddr(Instruction::Instruction64Bit hex_instruction);

    // Get the VDATA  [47:40] for the given 64 bit hexadecimal instruction.
    static MIMGInstruction::VDATA GetVdata(Instruction::Instruction64Bit hex_instruction);

    // Get the SRSRC  [52:48] for the given 64 bit hexadecimal instruction.
    static MIMGInstruction::SRSRC GetSrsrc(Instruction::Instruction64Bit hex_instruction);

    // Get the SSAMP  [57:53] for the given 64 bit hexadecimal instruction.
    static MIMGInstruction::SSAMP GetSsamp(Instruction::Instruction64Bit hex_instruction);
};

#endif //RGA_RADEONGPUANALYZERBACKEND_SRC_EMULATOR_PARSER_BE_PARSER_SI_MIMG_H_


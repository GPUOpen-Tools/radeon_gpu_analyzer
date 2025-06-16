//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implemetation of parser for the Southern Island [SI] VINTRP instructions.
//=============================================================================

// Local:
#include "be_parser_si_vintrp.h"

VINTRPInstruction::VSRC ParserSiVintrp::GetVsrc(Instruction::Instruction32Bit hex_instruction)
{
    EXTRACT_INSTRUCTION32_FIELD(hex_instruction, SI, VINTRP, vsrc, VSRC, 0);
    RETURN_EXTRACT_INSTRUCTION(vsrc);
}

VINTRPInstruction::ATTRCHAN ParserSiVintrp::GetATTRCHAN(Instruction::Instruction32Bit hex_instruction)
{
    EXTRACT_INSTRUCTION32_FIELD(hex_instruction, SI, VINTRP, attrchan, ATTRCHAN, 8);
    RETURN_EXTRACT_INSTRUCTION(attrchan);
}

VINTRPInstruction::ATTR ParserSiVintrp::GetATTR(Instruction::Instruction32Bit hex_instruction)
{
    EXTRACT_INSTRUCTION32_FIELD(hex_instruction, SI, VINTRP, attr, ATTR, 10);
    RETURN_EXTRACT_INSTRUCTION(attr);
}

SIVINTRPInstruction::OP ParserSiVintrp::GetSIVINTRPOp(Instruction::Instruction32Bit hex_instruction)
{
    EXTRACT_INSTRUCTION32_FIELD(hex_instruction, SI, VINTRP, op, OP, 16);
    RETURN_EXTRACT_INSTRUCTION(op);
}

VIVINTRPInstruction::OP ParserSiVintrp::GetVIVINTRPOp(Instruction::Instruction32Bit hex_instruction)
{
    EXTRACT_INSTRUCTION32_FIELD(hex_instruction, VI, VINTRP, op, OP, 16);
    RETURN_EXTRACT_INSTRUCTION(op);
}

VINTRPInstruction::VDST ParserSiVintrp::GetVDST(Instruction::Instruction32Bit hex_instruction)
{
    EXTRACT_INSTRUCTION32_FIELD(hex_instruction, SI, VINTRP, vdst, VDST, 18);
    RETURN_EXTRACT_INSTRUCTION(vdst);
}

ParserSi::kaStatus ParserSiVintrp::Parse(GDT_HW_GENERATION hw_generation, Instruction::Instruction32Bit hex_instruction, Instruction*& instruction,
    bool, uint32_t, int label /*=kNoLabel*/ , int goto_label /*=kNoLabel*/)
{
    VINTRPInstruction::VSRC vsrc = GetVsrc(hex_instruction);
    VINTRPInstruction::ATTRCHAN attrchan = GetATTRCHAN(hex_instruction);
    VINTRPInstruction::ATTR attr = GetATTR(hex_instruction);
    VINTRPInstruction::VDST vdst = GetVDST(hex_instruction);

    if ((hw_generation == GDT_HW_GENERATION_SEAISLAND) || (hw_generation == GDT_HW_GENERATION_SOUTHERNISLAND))
    {
        SIVINTRPInstruction::OP op = GetSIVINTRPOp(hex_instruction);
        instruction = new SIVINTRPInstruction(vsrc, attrchan, attr, op, vdst, label, goto_label);
    }
    else
    {
        VIVINTRPInstruction::OP op = GetVIVINTRPOp(hex_instruction);
        instruction = new VIVINTRPInstruction(vsrc, attrchan, attr, op, vdst, label, goto_label);
    }

    return ParserSi::kStatusSuccess;
}

ParserSi::kaStatus ParserSiVintrp::Parse(GDT_HW_GENERATION, Instruction::Instruction64Bit, Instruction*&, int /*label =kNoLabel*/ , int /*goto_label =kNoLabel*/)
{
    return ParserSi::kStatus64BitInstructionNotSupported;
}

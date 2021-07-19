//=================================================================
// Copyright 2020 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

// Local:
#include "be_parser_si_sopp.h"

SOPPInstruction::SIMM16 ParserSiSopp::GetSimm16(Instruction::Instruction32Bit hex_instruction)
{
    EXTRACT_INSTRUCTION32_FIELD(hex_instruction, SI, SOPP, simm16, SIMM16, 0);
    RETURN_EXTRACT_INSTRUCTION(simm16);
}

SISOPPInstruction::OP ParserSiSopp::GetSiSoppOp(Instruction::Instruction32Bit hex_instruction)
{
    EXTRACT_INSTRUCTION32_FIELD(hex_instruction, SI, SOPP, op, OP, 16);

    if ((op > SISOPPInstruction::kBranch && op < SISOPPInstruction::kCBranchScc0))
    {
        return SISOPPInstruction::kReserved;
    }
    else if ((op > SISOPPInstruction::kBarrier && op < SISOPPInstruction::kWaitcnt) || (op > SISOPPInstruction::kTtracedata))
    {
        return SISOPPInstruction::kIllegal;
    }
    else
    {
        return op;
    }
}

VISOPPInstruction::OP ParserSiSopp::GetViSoppOp(Instruction::Instruction32Bit hex_instruction)
{
    EXTRACT_INSTRUCTION32_FIELD(hex_instruction, VI, SOPP, op, OP, 16);

    if (op > VISOPPInstruction::kIllegal)
    {
        return VISOPPInstruction::kIllegal;
    }
    else
    {
        return op;
    }
}

ParserSi::kaStatus ParserSiSopp::Parse(GDT_HW_GENERATION hw_generation, Instruction::Instruction32Bit hex_instruction,
    Instruction*& instruction, bool, uint32_t, int label /*=kNoLabel*/ , int goto_label /*=kNoLabel*/)
{
    SOPPInstruction::SIMM16 simm16 = GetSimm16(hex_instruction);

    if ((hw_generation == GDT_HW_GENERATION_SEAISLAND) || (hw_generation == GDT_HW_GENERATION_SOUTHERNISLAND))
    {
        SISOPPInstruction::OP op = GetSiSoppOp(hex_instruction);
        instruction = new SISOPPInstruction(simm16, op, label, goto_label);
    }
    else
    {
        VISOPPInstruction::OP op = GetViSoppOp(hex_instruction);
        instruction = new VISOPPInstruction(simm16, op, label, goto_label);
    }

    return ParserSi::kStatusSuccess;
}

ParserSi::kaStatus ParserSiSopp::Parse(GDT_HW_GENERATION, Instruction::Instruction64Bit, Instruction*&, int /*label =kNoLabel*/ , int /*goto_label =kNoLabel*/)
{
    return ParserSi::kStatus64BitInstructionNotSupported;
}

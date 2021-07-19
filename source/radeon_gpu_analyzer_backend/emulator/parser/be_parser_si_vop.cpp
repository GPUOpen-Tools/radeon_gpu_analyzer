//=================================================================
// Copyright 2020 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

#include "be_parser_si_vop.h"

ParserSi::kaStatus ParserSiVop::Parse(GDT_HW_GENERATION hw_generation, Instruction::Instruction32Bit hex_instruction, Instruction*& instruction, bool, uint32_t, int label /*=kNoLabel*/ , int goto_label /*=kNoLabel*/)
{
    kaStatus ret = ParserSi::kStatus32BitInstructionNotSupported;
    VOPInstruction::Encoding encoding = GetInstructionType(hex_instruction);

    if ((hw_generation == GDT_HW_GENERATION_SEAISLAND) || (hw_generation == GDT_HW_GENERATION_SOUTHERNISLAND))
    {
        if (VOPInstruction::kEncodingVop1 == encoding)
        {
            uint64_t hex_instruction_temp = hex_instruction << 15;
            hex_instruction_temp = hex_instruction_temp >> 24;
            SIVOP1Instruction::Vop1Op op1 = static_cast<SIVOP1Instruction::Vop1Op>(hex_instruction_temp);
            instruction = new SIVOP1Instruction(32, encoding, op1, label, goto_label);
            ret = ParserSi::kStatusSuccess;
        }
        else if (VOPInstruction::kEncodingVop2 == encoding)
        {
            uint64_t hex_instruction_temp = hex_instruction << 15;
            hex_instruction_temp = hex_instruction_temp >> 24;
            SIVOP2Instruction::Vop2Op op2 = static_cast<SIVOP2Instruction::Vop2Op>(hex_instruction_temp);
            instruction = new SIVOP2Instruction(32, encoding, op2, label, goto_label);
            ret = ParserSi::kStatusSuccess;
        }

        else if (VOPInstruction::kEncodingVopC == encoding)
        {
            uint64_t hex_instruction_temp = hex_instruction << 15;
            hex_instruction_temp = hex_instruction_temp >> 24;
            SIVOPCInstruction::VopcOp opc = static_cast<SIVOPCInstruction::VopcOp>(hex_instruction_temp);
            instruction = new SIVOPCInstruction(32, encoding, opc, label, goto_label);
            ret = ParserSi::kStatusSuccess;
        }
    }
    else if (hw_generation == GDT_HW_GENERATION_VOLCANICISLAND)
    {
        if (VOPInstruction::kEncodingVop1 == encoding)
        {
            uint64_t hex_instruction_temp = hex_instruction << 15;
            hex_instruction_temp = hex_instruction_temp >> 24;
            VIVOP1Instruction::Vop1Op op1 = static_cast<VIVOP1Instruction::Vop1Op>(hex_instruction_temp);
            instruction = new VIVOP1Instruction(32, encoding, op1, label, goto_label);
            ret = ParserSi::kStatusSuccess;
        }
        else if (VOPInstruction::kEncodingVop2 == encoding)
        {
            uint64_t hex_instruction_temp = hex_instruction << 15;
            hex_instruction_temp = hex_instruction_temp >> 24;
            VIVOP2Instruction::Vop2Op op2 = static_cast<VIVOP2Instruction::Vop2Op>(hex_instruction_temp);
            instruction = new VIVOP2Instruction(32, encoding, op2, label, goto_label);
            ret = ParserSi::kStatusSuccess;
        }

        else if (VOPInstruction::kEncodingVopC == encoding)
        {
            uint64_t hex_instruction_temp = hex_instruction << 15;
            hex_instruction_temp = hex_instruction_temp >> 24;
            VIVOPCInstruction::VopcOp opc = static_cast<VIVOPCInstruction::VopcOp>(hex_instruction_temp);
            instruction = new VIVOPCInstruction(32, encoding, opc, label, goto_label);
            ret = ParserSi::kStatusSuccess;
        }
    }
    else if (hw_generation == GDT_HW_GENERATION_GFX9)
    {
        if (VOPInstruction::kEncodingVop1 == encoding)
        {
            uint64_t hex_instruction_temp = hex_instruction << 15;
            hex_instruction_temp = hex_instruction_temp >> 24;
            G9VOP1Instruction::Vop1Op op1 = static_cast<G9VOP1Instruction::Vop1Op>(hex_instruction_temp);
            instruction = new G9VOP1Instruction(32, encoding, op1, label, goto_label);
            ret = ParserSi::kStatusSuccess;
        }
        else if (VOPInstruction::kEncodingVop2 == encoding)
        {
            uint64_t hex_instruction_temp = hex_instruction << 15;
            hex_instruction_temp = hex_instruction_temp >> 24;
            G9VOP2Instruction::Vop2Op op2 = static_cast<G9VOP2Instruction::Vop2Op>(hex_instruction_temp);
            instruction = new G9VOP2Instruction(32, encoding, op2, label, goto_label);
            ret = ParserSi::kStatusSuccess;
        }

        else if (VOPInstruction::kEncodingVopC == encoding)
        {
            uint64_t hex_instruction_temp = hex_instruction << 15;
            hex_instruction_temp = hex_instruction_temp >> 24;
            VIVOPCInstruction::VopcOp opc = static_cast<VIVOPCInstruction::VopcOp>(hex_instruction_temp);
            instruction = new VIVOPCInstruction(32, encoding, opc, label, goto_label);
            ret = ParserSi::kStatusSuccess;
        }
    }
    else
    {
        ret = ParserSi::kStatusUnexpectedHwGeneration;
    }

    return ret;
}

ParserSi::kaStatus ParserSiVop::Parse(GDT_HW_GENERATION hw_generation, Instruction::Instruction64Bit hex_instruction, Instruction*& instruction, int label /*=kNoLabel*/ , int goto_label /*=kNoLabel*/)
{
    kaStatus ret =   ParserSi::kStatus64BitInstructionNotSupported;
    VOPInstruction::Encoding encoding = GetInstructionType(hex_instruction);

    if (hw_generation == GDT_HW_GENERATION_SEAISLAND || hw_generation == GDT_HW_GENERATION_SOUTHERNISLAND || hw_generation == GDT_HW_GENERATION_VOLCANICISLAND)
    {
        if (VOPInstruction::kEncodingVop3 == encoding)
        {
            uint64_t hex_instruction_temp = hex_instruction << 15;
            hex_instruction_temp = hex_instruction_temp >> 24;
            SIVOP3Instruction::Vop3Op op3 = static_cast<SIVOP3Instruction::Vop3Op>(hex_instruction_temp);
            instruction = new SIVOP3Instruction(64, encoding, op3, label, goto_label);
            ret =  ParserSi::kStatusSuccess;
        }
    }
    else if (hw_generation == GDT_HW_GENERATION_GFX9)
    {
        if (VOPInstruction::kEncodingVop3p == encoding)
        {
            uint64_t hex_instruction_temp = (hex_instruction >> 16) & 0x7F;
            G9VOP3Instruction::Vop3Op op3 = static_cast<G9VOP3Instruction::Vop3Op>(hex_instruction_temp);
            instruction = new G9VOP3Instruction(64, encoding, op3, label, goto_label);
            ret =  ParserSi::kStatusSuccess;
        }
        else if (VOPInstruction::kEncodingVop3 == encoding)
        {
            uint64_t hex_instruction_temp = hex_instruction << 15;
            hex_instruction_temp = hex_instruction_temp >> 24;
            G9VOP3Instruction::Vop3Op op3 = static_cast<G9VOP3Instruction::Vop3Op>(hex_instruction_temp);
            instruction = new G9VOP3Instruction(64, encoding, op3, label, goto_label);
            ret =  ParserSi::kStatusSuccess;
        }
    }
    else
    {
        ret = ParserSi::kStatusUnexpectedHwGeneration;
    }

    return ret;
}

VOPInstruction::Encoding ParserSiVop::GetInstructionType(Instruction::Instruction32Bit hex_instruction)
{
    uint64_t hex_instruction_temp = hex_instruction >> 25;

    if (hex_instruction_temp == VOPInstruction::kEncodingVop1)
    {
        return VOPInstruction::kEncodingVop1;
    }
    else if (hex_instruction_temp == VOPInstruction::kMaskVopc)
    {
        return VOPInstruction::kEncodingVopC;
    }

    hex_instruction_temp = hex_instruction  >> 31;

    if (hex_instruction_temp == VOPInstruction::kMaskVop2)
    {
        return VOPInstruction::kEncodingVop2;
    }

    return VOPInstruction::kEncodingIllegal;
}

VOPInstruction::Encoding ParserSiVop::GetInstructionType(Instruction::Instruction64Bit hex_instruction)
{
    if ((hex_instruction && VOPInstruction::kMaskVop3) >> 26 == VOPInstruction::kEncodingVop3)
    {
        return VOPInstruction::kEncodingVop3;
    }

    return VOPInstruction::kEncodingIllegal;
}

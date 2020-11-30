//=================================================================
// Copyright 2020 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

#include "be_parser_si_exp.h"

EXPInstruction::EN ParserSiExp::GetEn(Instruction::Instruction64Bit hex_instruction)
{
    EXPInstruction::EN en = static_cast<EXPInstruction::EN>(hex_instruction & kExpMaskEn);
    return en;
}

EXPInstruction::TGT ParserSiExp::GetTgt(Instruction::Instruction64Bit hex_instruction)
{
    EXPInstruction::TGT tgt = static_cast<EXPInstruction::TGT>((hex_instruction & kExpMaskTgt) >> 4);

    // Target 0 - 63 has not reserved values.
    if (tgt >= EXPInstruction::TGTExpReserved)
    {
        return EXPInstruction::TGTExpReserved;
    }

    return tgt;
}

EXPInstruction::COMPR ParserSiExp::GetCompr(Instruction::Instruction64Bit hex_instruction)
{
    EXPInstruction::COMPR compr = static_cast<EXPInstruction::COMPR>((hex_instruction & kExpMaskCompr) >> 10);
    return compr;
}

EXPInstruction::DONE ParserSiExp::GetDone(Instruction::Instruction64Bit hex_instruction)
{
    EXPInstruction::DONE done = static_cast<EXPInstruction::DONE>((hex_instruction & kExpMaskDone) >> 11);
    return done;
}


EXPInstruction::VM ParserSiExp::GetVm(Instruction::Instruction64Bit hex_instruction)
{
    EXPInstruction::VM vm = static_cast<EXPInstruction::VM>((hex_instruction & kExpMaskVm) >> 12);
    return vm;
}

EXPInstruction::VSRC ParserSiExp::GetVsrc(Instruction::Instruction64Bit hex_instruction, const unsigned int vsrcIdx)
{
    EXPInstruction::VSRC vsrc = 0;
    Instruction::Instruction64Bit exp_mask_vsrc;

    switch (vsrcIdx)
    {
        case 0:
            exp_mask_vsrc = (static_cast<Instruction::Instruction64Bit>(kExpMaskVsrc0)) << 32;
            vsrc = static_cast<EXPInstruction::VM>((hex_instruction & exp_mask_vsrc) >> 32);
            break;

        case 1:
            exp_mask_vsrc = (static_cast<Instruction::Instruction64Bit>(kExpMaskVsrc1)) << 32;
            vsrc = static_cast<EXPInstruction::VM>((hex_instruction & exp_mask_vsrc) >> 40);
            break;

        case 2:
            exp_mask_vsrc = (static_cast<Instruction::Instruction64Bit>(kExpMaskVsrc2)) << 32;
            vsrc = static_cast<EXPInstruction::VM>((hex_instruction & exp_mask_vsrc) >> 48);
            break;

        case 3:
            exp_mask_vsrc = (static_cast<Instruction::Instruction64Bit>(kExpMaskVsrc3)) << 32;
            vsrc = static_cast<EXPInstruction::VM>((hex_instruction & exp_mask_vsrc) >> 56);
            break;
    }

    return vsrc;
}

ParserSi::kaStatus ParserSiExp::Parse(GDT_HW_GENERATION, Instruction::Instruction64Bit hex_instruction, Instruction*& instruction, int label /*=kNoLabel*/ , int goto_label /*=kNoLabel*/)
{
    EXPInstruction::VSRC vsrc[4];
    EXPInstruction::EN en = GetEn(hex_instruction);
    EXPInstruction::TGT target = GetTgt(hex_instruction);
    EXPInstruction::COMPR compr = GetCompr(hex_instruction);
    EXPInstruction::DONE done = GetDone(hex_instruction);
    EXPInstruction::VM vm = GetVm(hex_instruction);
    vsrc[0] = GetVsrc(hex_instruction, 0);
    vsrc[1] = GetVsrc(hex_instruction, 1);
    vsrc[2] = GetVsrc(hex_instruction, 2);
    vsrc[3] = GetVsrc(hex_instruction, 3);

    instruction = new EXPInstruction(en, target, compr, done, vm, vsrc[0], vsrc[1], vsrc[2], vsrc[3], label, goto_label);
    return ParserSi::kStatusSuccess;
}

ParserSi::kaStatus ParserSiExp::Parse(GDT_HW_GENERATION, Instruction::Instruction32Bit, Instruction*&, bool , uint32_t , int  /*label=kNoLabel*/ , int /*goto_label =kNoLabel*/)
{
    return ParserSi::kStatus32BitInstructionNotSupported;
}


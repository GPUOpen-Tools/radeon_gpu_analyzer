//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implemetation of parser for the Southern Island [SI] DS instructions.
//=============================================================================

// Local.
#include "be_parser_si_ds.h"

DSInstruction::OFFSET
ParserSiDs::GetOffset(Instruction::Instruction64Bit hexInstruction, const unsigned char offset_index)
{
    DSInstruction::OFFSET offset;
    if (offset_index)
    {
        offset = static_cast<DSInstruction::OFFSET>((hexInstruction & DSMask_OFFSET1) >> 8);
    }
    else
    {
        offset = static_cast<DSInstruction::OFFSET>(hexInstruction & DSMask_OFFSET0);
    }
    return offset;
}

DSInstruction::GDS ParserSiDs::GetGDS(Instruction::Instruction64Bit hex_instruction)
{
    DSInstruction::GDS gds;
    gds  = static_cast<DSInstruction::GDS>(((hex_instruction & DSMask_GDS) >> 17) != 0);
    return gds;
}

SIDSInstruction::OP ParserSiDs::GetSIDSOp(Instruction::Instruction64Bit hex_instruction)
{
    EXTRACT_INSTRUCTION32_FIELD(hex_instruction, SI, DS, op, OP, 18);
    if ((op >= SIDSInstruction::kDsReserved))
    {
        return SIDSInstruction::kDsReserved;
    }
    else
    {
        return op;
    }
}

VIDSInstruction::OP ParserSiDs::GetVIDSOp(Instruction::Instruction64Bit hex_instruction)
{
    EXTRACT_INSTRUCTION32_FIELD(hex_instruction, VI, DS, op, OP, 18);
    if ((op >= VIDSInstruction::kDsIlegal))
    {
        return VIDSInstruction::kDsIlegal;
    }
    else
    {
        return op;
    }
}

G9DSInstruction::OP ParserSiDs::GetG9DSOp(Instruction::Instruction64Bit hex_instruction)
{
    EXTRACT_INSTRUCTION32_FIELD(hex_instruction, G9, DS, op, OP, 18);
    return (op < G9DSInstruction::kDsIlegal ? op : G9DSInstruction::kDsIlegal);
}

DSInstruction::ADDR ParserSiDs::GetADDR(Instruction::Instruction64Bit hex_instruction)
{
    EXTRACT_INSTRUCTION64_FIELD(hex_instruction, DS, addr, ADDR, 32);
    RETURN_EXTRACT_INSTRUCTION(addr);
}

DSInstruction::DATA
ParserSiDs::GetDATA(Instruction::Instruction64Bit hex_instruction, const unsigned int data_index)
{
    DSInstruction::DATA data;
    Instruction::Instruction64Bit ds_mask_data;
    if (data_index)
    {
        ds_mask_data = (static_cast<Instruction::Instruction64Bit>(DSMask_DATA1)) << 32;
        data = static_cast<DSInstruction::DATA>((hex_instruction & ds_mask_data) >> 48);
    }
    else
    {
        ds_mask_data = (static_cast<Instruction::Instruction64Bit>(DSMask_DATA0)) << 32;
        data = static_cast<DSInstruction::DATA>((hex_instruction & ds_mask_data) >> 40);
    }

    return data;
}

DSInstruction::VDST ParserSiDs::GetVDST(Instruction::Instruction64Bit hex_instruction)
{
    EXTRACT_INSTRUCTION64_FIELD(hex_instruction, DS, vdst, VDST, 56);
    RETURN_EXTRACT_INSTRUCTION(vdst);
}

ParserSi::kaStatus ParserSiDs::Parse(GDT_HW_GENERATION hw_generation, Instruction::Instruction64Bit hex_instruction,
    Instruction*& instruction, int label /*=kNoLabel*/ , int goto_label /*=kNoLabel*/)
{
    ParserSi::kaStatus status = ParserSi::kStatusSuccess;
    DSInstruction::OFFSET offset0 = GetOffset(hex_instruction, 0);
    DSInstruction::OFFSET offset1 = GetOffset(hex_instruction, 1);
    DSInstruction::GDS gds = GetGDS(hex_instruction);
    DSInstruction::ADDR addr = GetADDR(hex_instruction);
    DSInstruction::DATA data0 = GetDATA(hex_instruction, 0);
    DSInstruction::DATA data1 = GetDATA(hex_instruction, 1);
    DSInstruction::VDST vdst = GetVDST(hex_instruction);

    if ((hw_generation == GDT_HW_GENERATION_SEAISLAND) || (hw_generation == GDT_HW_GENERATION_SOUTHERNISLAND))
    {
        SIDSInstruction::OP op = GetSIDSOp(hex_instruction);
        instruction = new SIDSInstruction(offset0, offset1, gds, op, addr, data0, data1, vdst, label, goto_label);
    }
    else if (hw_generation == GDT_HW_GENERATION_VOLCANICISLAND)
    {
        VIDSInstruction::OP op = GetVIDSOp(hex_instruction);
        instruction = new VIDSInstruction(offset0, offset1, gds, op, addr, data0, data1, vdst, label, goto_label);
    }
    else if (hw_generation == GDT_HW_GENERATION_GFX9)
    {
        G9DSInstruction::OP op = GetG9DSOp(hex_instruction);
        instruction = new G9DSInstruction(offset0, offset1, gds, op, addr, data0, data1, vdst, label, goto_label);
    }
    else
    {
        status = ParserSi::kStatusUnexpectedHwGeneration;
    }

    return ParserSi::kStatusSuccess;
}

ParserSi::kaStatus ParserSiDs::Parse(GDT_HW_GENERATION, Instruction::Instruction32Bit, Instruction*&, bool , uint32_t, int /*label =kNoLabel*/ , int /*goto_label =kNoLabel*/)
{
    return ParserSi::kStatus32BitInstructionNotSupported;
}

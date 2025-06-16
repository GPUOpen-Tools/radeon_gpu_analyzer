//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation of parser for the Southern Island [SI] MUBUF instructions.
//=============================================================================
// Local.
#include "be_parser_si_mubuf.h"

MUBUFInstruction::OFFSET
ParserSiMubuf::GetOffset(Instruction::Instruction64Bit hex_instruction)
{
    EXTRACT_INSTRUCTION32_FIELD(hex_instruction, SI, MUBUF, offset, OFFSET, 0);
    RETURN_EXTRACT_INSTRUCTION(offset);
}

MUBUFInstruction::OFFEN
ParserSiMubuf::GetOffen(Instruction::Instruction64Bit hex_instruction)
{
    EXTRACT_INSTRUCTION32_FIELD(hex_instruction, SI, MUBUF, offen, OFFEN, 12);
    RETURN_EXTRACT_INSTRUCTION(offen);
}

MUBUFInstruction::IDXEN
ParserSiMubuf::GetIdxen(Instruction::Instruction64Bit hex_instruction)
{
    EXTRACT_INSTRUCTION32_FIELD(hex_instruction, SI, MUBUF, idxen, IDXEN, 13);
    RETURN_EXTRACT_INSTRUCTION(idxen);
}

MUBUFInstruction::GLC
ParserSiMubuf::GetGlc(Instruction::Instruction64Bit hex_instruction)
{
    EXTRACT_INSTRUCTION32_FIELD(hex_instruction, SI, MUBUF, glc, GLC, 14);
    RETURN_EXTRACT_INSTRUCTION(glc);
}

MUBUFInstruction::ADDR64
ParserSiMubuf::GetLds(Instruction::Instruction64Bit hex_instruction)
{
    EXTRACT_INSTRUCTION32_FIELD(hex_instruction, SI, MUBUF, lds, LDS, 16);
    RETURN_EXTRACT_INSTRUCTION(lds);
}

MUBUFInstruction::LDS
ParserSiMubuf::GetAddr64(Instruction::Instruction64Bit hex_instruction)
{
    EXTRACT_INSTRUCTION32_FIELD(hex_instruction, SI, MUBUF, addr64, ADDR64, 15);
    RETURN_EXTRACT_INSTRUCTION(addr64);
}

SIMUBUFInstruction::OP
ParserSiMubuf::GetSiOpMubuf(Instruction::Instruction64Bit hex_instruction, Instruction::InstructionCategory& instruction_kind)
{
    EXTRACT_INSTRUCTION32_FIELD(hex_instruction, SI, MUBUF, op, OP, 18);

    if ((op >= SIMUBUFInstruction::kBufferLoadFormatX && op <= SIMUBUFInstruction::kBufferLoadFormatXyzw) ||
        (op >= SIMUBUFInstruction::kBufferLoadUByte && op <= SIMUBUFInstruction::kBufferLoadDwordX4))
    {
        instruction_kind = Instruction::kVectorMemoryRead;
    }
    else if ((op >= SIMUBUFInstruction::kBufferStoreFormatX && op <= SIMUBUFInstruction::kBufferStoreFormatXyzw) ||
             (op >= SIMUBUFInstruction::kBufferStoreByte && op <= SIMUBUFInstruction::kBufferStoreDwordX4) ||
             (op >= SIMUBUFInstruction::kBufferWbinVl1Sc && op <= SIMUBUFInstruction::kBufferWBinVl1))
    {
        instruction_kind = Instruction::kVectorMemoryWrite;
    }
    else if (op >= SIMUBUFInstruction::kBufferAtomicSwap && op <= SIMUBUFInstruction::kBufferAtomicFmaxX2)
    {
        instruction_kind = Instruction::kAtomics;
    }

    if ((op > SIMUBUFInstruction::kBufferLoadDwordX4    && op < SIMUBUFInstruction::kBufferStoreByte)
        || (op > SIMUBUFInstruction::kBufferStoreByte  && op < SIMUBUFInstruction::kBufferStoreShort)
        || (op > SIMUBUFInstruction::kBufferStoreShort && op < SIMUBUFInstruction::kBufferStoreDword)
        || (op > SIMUBUFInstruction::kBufferStoreDwordX4 && op < SIMUBUFInstruction::kBufferAtomicSwap)
        || (op > SIMUBUFInstruction::kBufferAtomicFmax && op < SIMUBUFInstruction::kBufferAtomicSwapX2)
        || (op > SIMUBUFInstruction::kBufferAtomicFmaxX2 && op < SIMUBUFInstruction::kBufferWbinVl1Sc)
        || (op >= SIMUBUFInstruction::kBufferReserved))
    {
        return SIMUBUFInstruction::kBufferReserved;
    }
    else
    {
        return op;
    }
}

VIMUBUFInstruction::OP
ParserSiMubuf::GetViOpMubuf(Instruction::Instruction64Bit hex_instruction, Instruction::InstructionCategory& instruction_kind)
{
    EXTRACT_INSTRUCTION32_FIELD(hex_instruction, VI, MUBUF, op, OP, 18);

    if ((op >= VIMUBUFInstruction::kBufferLoadFormatX && op <= VIMUBUFInstruction::kBufferLoadFormatXyzw) ||
        (op >= VIMUBUFInstruction::kBufferLoadFormatD16X && op <= VIMUBUFInstruction::kBufferLoadFormatD16Xyzw) ||
        (op >= VIMUBUFInstruction::kBufferLoadUbyte && op <= VIMUBUFInstruction::kBufferLoadDwordx4))
    {
        instruction_kind = Instruction::kVectorMemoryRead;
    }
    else if ((op >= VIMUBUFInstruction::kBufferStoreFormatX && op <= VIMUBUFInstruction::kBufferStoreFormatXyzw) ||
             (op >= VIMUBUFInstruction::kBufferStoreFormatD16X && op <= VIMUBUFInstruction::kBufferStoreFormatD16Xyzw) ||
             (op >= VIMUBUFInstruction::kBufferStoreByte && op <= VIMUBUFInstruction::kBufferWbinvl1Vol))
    {
        instruction_kind = Instruction::kVectorMemoryWrite;
    }
    else if (op >= VIMUBUFInstruction::kBufferAtomicSwap && op <= VIMUBUFInstruction::kBufferAtomicDecX2)
    {
        instruction_kind = Instruction::kAtomics;
    }

    if (op > VIMUBUFInstruction::kBufferIlegal)
    {
        return VIMUBUFInstruction::kBufferIlegal;
    }
    else
    {
        return op;
    }
}

G9MUBUFInstruction::OP
ParserSiMubuf::GetVegaOpMubuf(Instruction::Instruction64Bit hex_instruction, Instruction::InstructionCategory& instruction_kind)
{
    EXTRACT_INSTRUCTION32_FIELD(hex_instruction, G9, MUBUF, op, OP, 18);

    if ((op >= G9MUBUFInstruction::kBufferLoadFormatX && op <= G9MUBUFInstruction::kBufferLoadFormatXyzw) ||
        (op >= G9MUBUFInstruction::kBufferLoadFormatD16X && op <= G9MUBUFInstruction::kBufferLoadFormatD16Xyzw) ||
        (op >= G9MUBUFInstruction::kBufferLoadUbyte && op <= G9MUBUFInstruction::kBufferLoadDwordx4) ||
        (op >= G9MUBUFInstruction:: kBufferLoadUbyteD16 && op <= G9MUBUFInstruction::kBufferLoadFormatD16HiX))
    {
        instruction_kind = Instruction::kVectorMemoryRead;
    }
    else if ((op >= G9MUBUFInstruction::kBufferStoreFormatX && op <= G9MUBUFInstruction::kBufferStoreFormatXyzw) ||
             (op >= G9MUBUFInstruction::kBufferStoreFormatD16X && op <= G9MUBUFInstruction::kBufferStoreFormatD16Xyzw) ||
             (op >= G9MUBUFInstruction::kBufferStoreByte && op <= G9MUBUFInstruction::kBufferStoreDwordx4) ||
             (op >= G9MUBUFInstruction::kBufferStoreFormatD16HiX && op <= G9MUBUFInstruction::kBufferWbinvl1Vol))

    {
        instruction_kind = Instruction::kVectorMemoryWrite;
    }
    else if (op >= G9MUBUFInstruction::kBufferAtomicSwap && op <= G9MUBUFInstruction::kBufferAtomicDecX2)
    {
        instruction_kind = Instruction::kAtomics;
    }

    return (op < G9MUBUFInstruction::kBufferIllegal ? op : G9MUBUFInstruction::kBufferIllegal);
}

MUBUFInstruction::VADDR
ParserSiMubuf::GetVaddr(Instruction::Instruction64Bit hex_instruction)
{
    EXTRACT_INSTRUCTION64_FIELD(hex_instruction, MUBUF, vaddr, VADDR, 32);
    RETURN_EXTRACT_INSTRUCTION(vaddr);
}

MUBUFInstruction::VDATA
ParserSiMubuf::GetVdata(Instruction::Instruction64Bit hex_instruction)
{
    EXTRACT_INSTRUCTION64_FIELD(hex_instruction, MUBUF, vdata, VDATA, 40);
    RETURN_EXTRACT_INSTRUCTION(vdata);
}

MUBUFInstruction::SRSRC
ParserSiMubuf::GetSrsrc(Instruction::Instruction64Bit hex_instruction)
{
    EXTRACT_INSTRUCTION64_FIELD(hex_instruction, MUBUF, srsrc, SRSRC, 48);
    RETURN_EXTRACT_INSTRUCTION(srsrc);
}

MUBUFInstruction::SLC
ParserSiMubuf::GetSlc(Instruction::Instruction64Bit hex_instruction)
{
    EXTRACT_INSTRUCTION64_FIELD(hex_instruction, MUBUF, slc, SLC, 54);
    RETURN_EXTRACT_INSTRUCTION(slc);
}

MUBUFInstruction::TFE
ParserSiMubuf::GetTfe(Instruction::Instruction64Bit hex_instruction)
{
    EXTRACT_INSTRUCTION64_FIELD(hex_instruction, MUBUF, tfe, TFE, 55);
    RETURN_EXTRACT_INSTRUCTION(tfe);
}


MUBUFInstruction::SOFFSET
ParserSiMubuf::GetSOFFSET(Instruction::Instruction64Bit hex_instruction, unsigned int&)
{
    EXTRACT_INSTRUCTION64_FIELD(hex_instruction, MUBUF, soffset, SOFFSET, 56);
    RETURN_EXTRACT_INSTRUCTION(soffset);

#define X_RANGE(FIELD_MIN,FIELD_MAX,FIELD,IN,VAL)\
    if ((IN >= MUBUFInstruction::SOFFSET##FIELD_MIN) && (IN <= MUBUFInstruction::SOFFSET##FIELD_MAX)) \
    { \
        VAL = IN; \
        return MUBUFInstruction::SOFFSET##FIELD; \
    }
#define X(FIELD,IN) \
    if (IN == MUBUFInstruction::SOFFSET##FIELD) \
    { \
        return MUBUFInstruction::SOFFSET##FIELD; \
    }
    //GENERIC_INSTRUCTION_FIELDS_1(soffset, ridx);
#undef X
#undef X_RANGE

#define X_RANGE(FIELD_MIN,FIELD_MAX,FIELD,IN)\
    if ((IN >= MUBUFInstruction::SOFFSET##FIELD_MIN) && (IN <= MUBUFInstruction::SOFFSET##FIELD_MAX)) \
    { \
        return MUBUFInstruction::SOFFSET##FIELD; \
    }
#define X(FIELD,IN) \
    if (IN == MUBUFInstruction::SOFFSET##FIELD) \
    { \
        return MUBUFInstruction::SOFFSET##FIELD; \
    }
    //SCALAR_INSTRUCTION_FIELDS(soffset);
#undef X
#undef X_RANGE
    //return MUBUFInstruction::SOFFSETIllegal;
}

ParserSi::kaStatus ParserSiMubuf::Parse(GDT_HW_GENERATION hwGen, Instruction::Instruction64Bit hex_instruction, Instruction*& instruction, int label /*=kNoLabel*/ , int goto_label /*=kNoLabel*/)
{
    ParserSi::kaStatus status = ParserSi::kStatusSuccess;

    Instruction::InstructionCategory instruction_kind = Instruction::kVectorMemoryRead;
    unsigned int ridx = 0;
    MUBUFInstruction::OFFSET offset = GetOffset(hex_instruction);
    MUBUFInstruction::OFFEN offen = GetOffen(hex_instruction);
    MUBUFInstruction::IDXEN idxen = GetIdxen(hex_instruction);
    MUBUFInstruction::GLC glc = GetGlc(hex_instruction);
    MUBUFInstruction::ADDR64 addr64 = GetAddr64(hex_instruction);
    MUBUFInstruction::LDS lds = GetLds(hex_instruction);
    MUBUFInstruction::VADDR vaddr = GetVaddr(hex_instruction);
    MUBUFInstruction::VDATA vdata = GetVdata(hex_instruction);
    MUBUFInstruction::SRSRC srsrc = GetSrsrc(hex_instruction);
    MUBUFInstruction::SLC slc = GetSlc(hex_instruction);
    MUBUFInstruction::TFE tfe = GetTfe(hex_instruction);
    MUBUFInstruction::SOFFSET soffset = GetSOFFSET(hex_instruction, ridx);

    if ((hwGen == GDT_HW_GENERATION_SEAISLAND) || (hwGen == GDT_HW_GENERATION_SOUTHERNISLAND))
    {
        SIMUBUFInstruction::OP op = GetSiOpMubuf(hex_instruction, instruction_kind);
        instruction = new SIMUBUFInstruction(offset, offen, idxen, glc, addr64, lds, op, vaddr, vdata, srsrc, slc,
                                             tfe, soffset, ridx, instruction_kind, label, goto_label);
    }
    else if (hwGen == GDT_HW_GENERATION_VOLCANICISLAND)
    {
        VIMUBUFInstruction::OP op = GetViOpMubuf(hex_instruction, instruction_kind);
        instruction = new VIMUBUFInstruction(offset, offen, idxen, glc, addr64, lds, op, vaddr, vdata, srsrc, slc,
                                             tfe, soffset, ridx, instruction_kind, label, goto_label);
    }
    else if (hwGen == GDT_HW_GENERATION_GFX9)
    {
        G9MUBUFInstruction::OP op = GetVegaOpMubuf(hex_instruction, instruction_kind);
        instruction = new G9MUBUFInstruction(offset, offen, idxen, glc, addr64, lds, op, vaddr, vdata, srsrc, slc,
                                             tfe, soffset, ridx, instruction_kind, label, goto_label);
    }
    else
    {
        status = ParserSi::kStatusUnexpectedHwGeneration;
    }

    return status;
}

ParserSi::kaStatus
ParserSiMubuf::Parse(GDT_HW_GENERATION, Instruction::Instruction32Bit, Instruction*&, bool , uint32_t, int /*label =kNoLabel*/ , int /*goto_label =kNoLabel*/)
{
    return ParserSi::kStatus32BitInstructionNotSupported;
}

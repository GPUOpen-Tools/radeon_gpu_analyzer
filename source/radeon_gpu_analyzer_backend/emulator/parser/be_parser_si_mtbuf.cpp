//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation of parser for the Southern Island [SI] MTBUF instructions.
//=============================================================================

// Local.
#include "be_parser_si_mtbuf.h"

MTBUFInstruction::OFFSET ParserSiMtbuf::GetOffset(Instruction::Instruction64Bit hex_instruction)
{
    EXTRACT_INSTRUCTION32_FIELD(hex_instruction, SI, MTBUF, offset, OFFSET, 0);
    RETURN_EXTRACT_INSTRUCTION(offset);
}

MTBUFInstruction::OFFEN ParserSiMtbuf::GetOffen(Instruction::Instruction64Bit hex_instruction)
{
    EXTRACT_INSTRUCTION32_FIELD(hex_instruction, SI, MTBUF, offen, OFFEN, 12);
    RETURN_EXTRACT_INSTRUCTION(offen);
}

MTBUFInstruction::IDXEN ParserSiMtbuf::GetIdxen(Instruction::Instruction64Bit hex_instruction)
{
    EXTRACT_INSTRUCTION32_FIELD(hex_instruction, SI, MTBUF, idxen, IDXEN, 13);
    RETURN_EXTRACT_INSTRUCTION(idxen);
}

MTBUFInstruction::GLC ParserSiMtbuf::GetGlc(Instruction::Instruction64Bit hex_instruction)
{
    EXTRACT_INSTRUCTION32_FIELD(hex_instruction, SI, MTBUF, glc, GLC, 14);
    RETURN_EXTRACT_INSTRUCTION(glc);
}

MTBUFInstruction::ADDR64
ParserSiMtbuf::GetAddr64(Instruction::Instruction64Bit hex_instruction)
{
    EXTRACT_INSTRUCTION32_FIELD(hex_instruction, SI, MTBUF, addr64, ADDR64, 15);
    RETURN_EXTRACT_INSTRUCTION(addr64);
}

SIMTBUFInstruction::OP ParserSiMtbuf::GetSiOpMtbuf(Instruction::Instruction64Bit hex_instruction, Instruction::InstructionCategory& instruction_kind)
{
    EXTRACT_INSTRUCTION32_FIELD(hex_instruction, SI, MTBUF, op, OP, 16);

    if (op <= SIMTBUFInstruction::kTbufferLoadFormatXyzw)
    {
        instruction_kind = Instruction::kVectorMemoryRead;
    }
    else
    {
        instruction_kind = Instruction::kVectorMemoryWrite;
    }

    if ((op >= SIMTBUFInstruction::kTbufferRESERVED))
    {
        return SIMTBUFInstruction::kTbufferRESERVED;
    }
    else
    {
        return op;
    }
}

VIMTBUFInstruction::OP ParserSiMtbuf::GetViOpMtbuf(Instruction::Instruction64Bit hex_instruction, Instruction::InstructionCategory& instruction_kind)
{
    EXTRACT_INSTRUCTION32_FIELD(hex_instruction, VI, MTBUF, op, OP, 16);

    if (((op <= VIMTBUFInstruction::kTbufferLoadFormatXyzw) || (op >= VIMTBUFInstruction::kTbufferLoadFormatD16Xyzw)) && (op <= VIMTBUFInstruction::kTbufferLoadFormatD16X))
    {
        instruction_kind = Instruction::kVectorMemoryRead;
    }
    else
    {
        instruction_kind = Instruction::kVectorMemoryWrite;
    }

    if ((op >= VIMTBUFInstruction::kTbufferIllegal))
    {
        return VIMTBUFInstruction::kTbufferIllegal;
    }
    else
    {
        return op;
    }
}

MTBUFInstruction::DFMT ParserSiMtbuf::GetDfmt(Instruction::Instruction64Bit hex_instruction)
{
    EXTRACT_INSTRUCTION32_FIELD(hex_instruction, SI, MTBUF, dmft, ADDR64, 19);
    RETURN_EXTRACT_INSTRUCTION(dmft);
}

MTBUFInstruction::NFMT ParserSiMtbuf::GetNfmt(Instruction::Instruction64Bit hex_instruction)
{
    EXTRACT_INSTRUCTION32_FIELD(hex_instruction, SI, MTBUF, nmft, ADDR64, 15);
    RETURN_EXTRACT_INSTRUCTION(nmft);
}

MTBUFInstruction::VADDR ParserSiMtbuf::GetVaddr(Instruction::Instruction64Bit hex_instruction)
{
    EXTRACT_INSTRUCTION64_FIELD(hex_instruction, MTBUF, vaddr, VADDR, 32);
    RETURN_EXTRACT_INSTRUCTION(vaddr);
}

MTBUFInstruction::VDATA ParserSiMtbuf::GetVdata(Instruction::Instruction64Bit hex_instruction)
{
    EXTRACT_INSTRUCTION64_FIELD(hex_instruction, MTBUF, vdata, VDATA, 40);
    RETURN_EXTRACT_INSTRUCTION(vdata);
}

MTBUFInstruction::SRSRC ParserSiMtbuf::GetSrsrc(Instruction::Instruction64Bit hex_instruction)
{
    EXTRACT_INSTRUCTION64_FIELD(hex_instruction, MTBUF, srsrc, SRSRC, 48);
    RETURN_EXTRACT_INSTRUCTION(srsrc);
}

MTBUFInstruction::SLC ParserSiMtbuf::GetSlc(Instruction::Instruction64Bit hex_instruction)
{
    EXTRACT_INSTRUCTION64_FIELD(hex_instruction, MTBUF, slc, SLC, 54);
    RETURN_EXTRACT_INSTRUCTION(slc);
}

MTBUFInstruction::TFE ParserSiMtbuf::GetTfe(Instruction::Instruction64Bit hex_instruction)
{
    EXTRACT_INSTRUCTION64_FIELD(hex_instruction, MTBUF, tfe, TFE, 55);
    RETURN_EXTRACT_INSTRUCTION(tfe);
}

MTBUFInstruction::SOFFSET ParserSiMtbuf::GetSOFFSET(Instruction::Instruction64Bit hex_instruction, unsigned int&)
{
    EXTRACT_INSTRUCTION64_FIELD(hex_instruction, MTBUF, soffset, SOFFSET, 56);
    RETURN_EXTRACT_INSTRUCTION(soffset);

#define X_RANGE(FIELD_MIN,FIELD_MAX,FIELD,IN,VAL)\
    if ((IN >= MTBUFInstruction::SOFFSET##FIELD_MIN) && (IN <= MTBUFInstruction::SOFFSET##FIELD_MAX)) \
    { \
        VAL = IN; \
        return MTBUFInstruction::SOFFSET##FIELD; \
    }
#define X(FIELD,IN) \
    if (IN == MTBUFInstruction::SOFFSET##FIELD) \
    { \
        return MTBUFInstruction::SOFFSET##FIELD; \
    }
    //GENERIC_INSTRUCTION_FIELDS_1(soffset, ridx);
#undef X
#undef X_RANGE

#define X_RANGE(FIELD_MIN,FIELD_MAX,FIELD,IN)\
    if ((IN >= MTBUFInstruction::SOFFSET##FIELD_MIN) && (IN <= MTBUFInstruction::SOFFSET##FIELD_MAX)) \
    { \
        return MTBUFInstruction::SOFFSET##FIELD; \
    }
#define X(FIELD,IN) \
    if (IN == MTBUFInstruction::SOFFSET##FIELD) \
    { \
        return MTBUFInstruction::SOFFSET##FIELD; \
    }
    //SCALAR_INSTRUCTION_FIELDS(soffset);
#undef X
#undef X_RANGE
    //return MTBUFInstruction::SOFFSETIllegal;
}

ParserSi::kaStatus ParserSiMtbuf::Parse(GDT_HW_GENERATION hw_generation, Instruction::Instruction64Bit hex_instruction,
    Instruction*& instruction, int label /*=kNoLabel*/ , int goto_label /*=kNoLabel*/)
{
    Instruction::InstructionCategory instruction_kind = Instruction::kVectorMemoryRead;
    unsigned int ridx = 0;
    MTBUFInstruction::OFFSET offset = GetOffset(hex_instruction);
    MTBUFInstruction::OFFEN offen = GetOffen(hex_instruction);
    MTBUFInstruction::IDXEN idxen = GetIdxen(hex_instruction);
    MTBUFInstruction::GLC glc = GetGlc(hex_instruction);
    MTBUFInstruction::ADDR64 addr64 = GetAddr64(hex_instruction);
    MTBUFInstruction::DFMT dfmt = GetDfmt(hex_instruction);
    MTBUFInstruction::NFMT nmft = GetNfmt(hex_instruction);
    MTBUFInstruction::VADDR vaddr = GetVaddr(hex_instruction);
    MTBUFInstruction::VDATA vdata = GetVdata(hex_instruction);
    MTBUFInstruction::SRSRC srsrc = GetSrsrc(hex_instruction);
    MTBUFInstruction::SLC slc = GetSlc(hex_instruction);
    MTBUFInstruction::TFE tfe = GetTfe(hex_instruction);
    MTBUFInstruction::SOFFSET soffset = GetSOFFSET(hex_instruction, ridx);

    if ((hw_generation == GDT_HW_GENERATION_SEAISLAND) || (hw_generation == GDT_HW_GENERATION_SOUTHERNISLAND))
    {
        SIMTBUFInstruction::OP op = GetSiOpMtbuf(hex_instruction, instruction_kind);
        instruction = new SIMTBUFInstruction(offset, offen, idxen, glc, addr64, op, dfmt, nmft, vaddr, vdata, srsrc, slc,
                                             tfe, soffset, ridx, instruction_kind, label, goto_label);
    }
    else
    {
        VIMTBUFInstruction::OP op = GetViOpMtbuf(hex_instruction, instruction_kind);
        instruction = new VIMTBUFInstruction(offset, offen, idxen, glc, addr64, op, dfmt, nmft, vaddr, vdata, srsrc, slc,
                                             tfe, soffset, ridx, instruction_kind, label, goto_label);
    }

    return ParserSi::kStatusSuccess;
}

ParserSi::kaStatus ParserSiMtbuf::Parse(GDT_HW_GENERATION, Instruction::Instruction32Bit,
    Instruction*&, bool , uint32_t, int /*label =kNoLabel*/ , int /*goto_label =kNoLabel*/)
{
    return ParserSi::kStatus32BitInstructionNotSupported;
}

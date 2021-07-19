//=================================================================
// Copyright 2020 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

// Local.
#include "be_parser_si_sop2.h"

SOP2Instruction::SSRC ParserSiSop2::GetSsrc(Instruction::Instruction32Bit hex_instruction, unsigned int&, unsigned int index_ssrc)
{
    SOP2Instruction::SSRC ssrc = (SOP2Instruction::SSRC)0;

    switch (index_ssrc)
    {
        case 0 :
            ssrc = static_cast<SOP2Instruction::SSRC>(hex_instruction & static_cast<Instruction::Instruction32Bit>(SOP2Mask_SSRC0));
            break;

        case 1:
            ssrc = static_cast<SOP2Instruction::SSRC>((hex_instruction & static_cast<Instruction::Instruction32Bit>(SOP2Mask_SSRC1)) >> 8);
            break;
    }

#define X_RANGE(FIELD_MIN,FIELD_MAX,FIELD,IN,VAL)\
    if ((IN >= SOP2Instruction::SSRC##FIELD_MIN) && (IN <= SOP2Instruction::SSRC##FIELD_MAX)) \
    { \
        VAL = IN; \
        return SOP2Instruction::SSRC##FIELD; \
    }
#define X(FIELD,IN) \
    if (IN == SOP2Instruction::SSRC##FIELD) \
    { \
        return SOP2Instruction::SSRC##FIELD; \
    }
    //GENERIC_INSTRUCTION_FIELDS_1(ssrc, ridx);
#undef X
#undef X_RANGE

#define X_RANGE(FIELD_MIN,FIELD_MAX,FIELD,IN)\
    if ((IN >= SOP2Instruction::SSRC##FIELD_MIN) && (IN <= SOP2Instruction::SSRC##FIELD_MAX)) \
    { \
        return SOP2Instruction::SSRC##FIELD; \
    }
#define X(FIELD,IN) \
    if (IN == SOP2Instruction::SSRC##FIELD) \
    { \
        return SOP2Instruction::SSRC##FIELD; \
    }
    //SCALAR_INSTRUCTION_FIELDS(ssrc);
    GENERIC_INSTRUCTION_FIELDS_2(ssrc);
#undef X
#undef X_RANGE
    return SOP2Instruction::SSRCIllegal;
}

SISOP2Instruction::OP ParserSiSop2::GetSiSop2Op(Instruction::Instruction32Bit hex_instruction)
{
    EXTRACT_INSTRUCTION32_FIELD(hex_instruction, SI, SOP2, op, OP, 23);

    if ((op > SISOP2Instruction::kCselectB64 && op < SISOP2Instruction::kAndB32)
        || (op >= SISOP2Instruction::kReserved))
    {
        return SISOP2Instruction::kReserved;
    }
    else
    {
        return op;
    }
}

VISOP2Instruction::OP ParserSiSop2::GetViSop2Op(Instruction::Instruction32Bit hex_instruction)
{
    EXTRACT_INSTRUCTION32_FIELD(hex_instruction, VI, SOP2, op, OP, 23);

    if (op > VISOP2Instruction::S_ILLEGAL)
    {
        return VISOP2Instruction::S_ILLEGAL;
    }
    else
    {
        return op;
    }
}

G9SOP2Instruction::OP ParserSiSop2::GetG9Sop2Op(Instruction::Instruction32Bit hex_instruction)
{
    EXTRACT_INSTRUCTION32_FIELD(hex_instruction, G9, SOP2, op, OP, 23);

    if (op > G9SOP2Instruction::kIllegal)
    {
        return G9SOP2Instruction::kIllegal;
    }
    else
    {
        return op;
    }
}

SOP2Instruction::SDST ParserSiSop2::GetSdst(Instruction::Instruction32Bit hex_instruction, unsigned int& ridx)
{
    EXTRACT_INSTRUCTION32_FIELD(hex_instruction, SI, SOP2, sdst, SDST, 16);
#define X_RANGE(FIELD_MIN,FIELD_MAX,FIELD,IN,VAL)\
    if ((IN >= SOP2Instruction::SDST##FIELD_MIN) && (IN <= SOP2Instruction::SDST##FIELD_MAX)) \
    { \
        VAL = IN; \
        return SOP2Instruction::SDST##FIELD; \
    }
#define X(FIELD,IN) \
    if (IN == SOP2Instruction::SDST##FIELD) \
    { \
        return SOP2Instruction::SDST##FIELD; \
    }
    GENERIC_INSTRUCTION_FIELDS_1(sdst, ridx);
#undef X
#undef X_RANGE
    return SOP2Instruction::SDSTIllegal;
}

ParserSi::kaStatus ParserSiSop2::Parse(GDT_HW_GENERATION hw_generation, Instruction::Instruction32Bit hex_instruction,
    Instruction*& instruction, bool is_literal_32b, uint32_t literal_32b, int label, int goto_label)
{
    ParserSi::kaStatus status = ParserSi::kStatusSuccess;
    unsigned int ridx0 = 0, ridx1 = 0, sdstRidx = 0;
    SOP2Instruction::SSRC ssrc0 = GetSsrc(hex_instruction, ridx0, 0);
    SOP2Instruction::SSRC ssrc1 = GetSsrc(hex_instruction, ridx1, 1);
    SOP2Instruction::SDST sdst = GetSdst(hex_instruction, sdstRidx);

    if ((hw_generation == GDT_HW_GENERATION_SEAISLAND) || (hw_generation == GDT_HW_GENERATION_SOUTHERNISLAND))
    {
        SISOP2Instruction::OP op = GetSiSop2Op(hex_instruction);
        instruction = new SISOP2Instruction(ssrc0, ssrc1, sdst, op, ridx0, ridx1, sdstRidx, is_literal_32b, literal_32b, label, goto_label);
    }
    else if (hw_generation == GDT_HW_GENERATION_VOLCANICISLAND)
    {
        VISOP2Instruction::OP op = GetViSop2Op(hex_instruction);
        instruction = new VISOP2Instruction(ssrc0, ssrc1, sdst, op, ridx0, ridx1, sdstRidx, is_literal_32b, literal_32b, label, goto_label);
    }
    else if (hw_generation == GDT_HW_GENERATION_GFX9)
    {
        G9SOP2Instruction::OP op = GetG9Sop2Op(hex_instruction);
        instruction = new G9SOP2Instruction(ssrc0, ssrc1, sdst, op, ridx0, ridx1, sdstRidx, is_literal_32b, literal_32b, label, goto_label);
    }
    else
    {
        status = ParserSi::kStatusUnexpectedHwGeneration;
    }

    return status;
}

ParserSi::kaStatus ParserSiSop2::Parse(GDT_HW_GENERATION, Instruction::Instruction64Bit, Instruction*&, int , int)
{
    return ParserSi::kStatus64BitInstructionNotSupported;
}


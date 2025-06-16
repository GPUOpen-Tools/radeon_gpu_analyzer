//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation of parser for the Southern Island [SI] SOP1 instructions.
//=============================================================================

// Local.
#include "be_parser_si_sop1.h"

SOP1Instruction::SSRC ParserSiSop1::GetSSRC0(Instruction::Instruction32Bit hex_instruction, unsigned int& ridx)
{
    EXTRACT_INSTRUCTION32_FIELD(hex_instruction, SI, SOP1, ssrc, SSRC, 0);
#define X_RANGE(FIELD_MIN,FIELD_MAX,FIELD,IN,VAL)\
    if ((IN >= SOP1Instruction::SSRC##FIELD_MIN) && (IN <= SOP1Instruction::SSRC##FIELD_MAX)) \
    { \
        VAL = IN; \
        return SOP1Instruction::SSRC##FIELD; \
    }
#define X(FIELD,IN) \
    if (IN == SOP1Instruction::SSRC##FIELD) \
    { \
        return SOP1Instruction::SSRC##FIELD; \
    }
    GENERIC_INSTRUCTION_FIELDS_1(ssrc, ridx);
#undef X
#undef X_RANGE

#define X_RANGE(FIELD_MIN,FIELD_MAX,FIELD,IN)\
    if ((IN >= SOP1Instruction::SSRC##FIELD_MIN) && (IN <= SOP1Instruction::SSRC##FIELD_MAX)) \
    { \
        return SOP1Instruction::SSRC##FIELD; \
    }
#define X(FIELD,IN) \
    if (IN == SOP1Instruction::SSRC##FIELD) \
    { \
        return SOP1Instruction::SSRC##FIELD; \
    }
    SCALAR_INSTRUCTION_FIELDS(ssrc);
    GENERIC_INSTRUCTION_FIELDS_2(ssrc);
#undef X
#undef X_RANGE
    return SOP1Instruction::SSRCIllegal;
    //return ssrc;
}

SISOP1Instruction::OP ParserSiSop1::GetSISOP1Op(Instruction::Instruction32Bit hex_instruction)
{
    EXTRACT_INSTRUCTION32_FIELD(hex_instruction, SI, SOP1, op, OP, 8);

    if ((op < SISOP1Instruction::kMovB32)
        || (op > SISOP1Instruction::kRfeB64 && op < SISOP1Instruction::kAndSaveexecB64)
        || (op > SISOP1Instruction::kCbranchJoin && op < SISOP1Instruction::kAbsI32)
        || (op >= SISOP1Instruction::kReserved))
    {
        return SISOP1Instruction::kReserved;
    }
    else
    {
        return op;
    }
}

VISOP1Instruction::OP ParserSiSop1::GetVISOP1Op(Instruction::Instruction32Bit hex_instruction)
{
    EXTRACT_INSTRUCTION32_FIELD(hex_instruction, VI, SOP1, op, OP, 8);

    if (op >= VISOP1Instruction::kIllegal)
    {
        return VISOP1Instruction::kIllegal;
    }
    else
    {
        return op;
    }
}

G9SOP1Instruction::OP ParserSiSop1::GetG9SOP1Op(Instruction::Instruction32Bit hex_instruction)
{
    EXTRACT_INSTRUCTION32_FIELD(hex_instruction, G9, SOP1, op, OP, 8);

    return (op < G9SOP1Instruction::kIllegal ? op : G9SOP1Instruction::kIllegal);
}

SOP1Instruction::SDST ParserSiSop1::GetSdst(Instruction::Instruction32Bit hex_instruction, unsigned int& ridx)
{
    EXTRACT_INSTRUCTION32_FIELD(hex_instruction, SI, SOP1, sdst, SDST, 16);
#define X_RANGE(FIELD_MIN,FIELD_MAX,FIELD,IN,VAL)\
    if ((IN >= SOP1Instruction::SDST##FIELD_MIN) && (IN <= SOP1Instruction::SDST##FIELD_MAX)) \
    { \
        VAL = IN; \
        return SOP1Instruction::SDST##FIELD; \
    }
#define X(FIELD,IN) \
    if (IN == SOP1Instruction::SDST##FIELD) \
    { \
        return SOP1Instruction::SDST##FIELD; \
    }
    GENERIC_INSTRUCTION_FIELDS_1(sdst, ridx);
#undef X
#undef X_RANGE
    return SOP1Instruction::SDSTIllegal;
}

ParserSi::kaStatus ParserSiSop1::Parse(GDT_HW_GENERATION hw_generation, Instruction::Instruction32Bit hex_instruction,
    Instruction*& instruction, bool, uint32_t, int label /*=kNoLabel*/ , int goto_label /*=kNoLabel*/)
{
    ParserSi::kaStatus status = ParserSi::kStatusSuccess;
    unsigned int ridx0 = 0, sdstRidx1 = 0;
    SOP1Instruction::SSRC ssrc0 = GetSSRC0(hex_instruction, ridx0);
    SOP1Instruction::SDST sdst = GetSdst(hex_instruction, sdstRidx1);

    if ((hw_generation == GDT_HW_GENERATION_SEAISLAND) || (hw_generation == GDT_HW_GENERATION_SOUTHERNISLAND))
    {
        SISOP1Instruction::OP op = GetSISOP1Op(hex_instruction);
        instruction = new SISOP1Instruction(ssrc0, op, sdst, ridx0, sdstRidx1, label, goto_label);
    }
    else if (hw_generation == GDT_HW_GENERATION_VOLCANICISLAND)
    {
        VISOP1Instruction::OP op = GetVISOP1Op(hex_instruction);
        instruction = new VISOP1Instruction(ssrc0, op, sdst, ridx0, sdstRidx1, label, goto_label);
    }
    else if (hw_generation == GDT_HW_GENERATION_GFX9)
    {
        G9SOP1Instruction::OP op = GetG9SOP1Op(hex_instruction);
        instruction = new G9SOP1Instruction(ssrc0, op, sdst, ridx0, sdstRidx1, label, goto_label);
    }
    else
    {
        status = ParserSi::kStatusUnexpectedHwGeneration;
    }

    return status;
}

ParserSi::kaStatus ParserSiSop1::Parse(GDT_HW_GENERATION, Instruction::Instruction64Bit, Instruction*&, int/* label =kNoLabel*/ , int /*goto_label =kNoLabel*/)
{
    return ParserSi::kStatus64BitInstructionNotSupported;
}

//=================================================================
// Copyright 2020 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

// Local.
#include "be_parser_si_sopc.h"

SOPCInstruction::SSRC ParserSiSopC::GetSsrc(Instruction::Instruction32Bit hex_instruction, unsigned int&, unsigned int idxSSRC)
{
    SOPCInstruction::SSRC ssrc = (SOPCInstruction::SSRC)0;

    switch (idxSSRC)
    {
        case 0 :
            ssrc = static_cast<SOPCInstruction::SSRC>(hex_instruction & SOPCMask_SSRC0);
            break;

        case 1:
            ssrc = static_cast<SOPCInstruction::SSRC>((hex_instruction & SOPCMask_SSRC1) >> 8);
            break;
    }

#define X_RANGE(FIELD_MIN,FIELD_MAX,FIELD,IN,VAL)\
    if ((IN >= SOPCInstruction::SSRC##FIELD_MIN) && (IN <= SOPCInstruction::SSRC##FIELD_MAX)) \
    { \
        VAL = IN; \
        return SOPCInstruction::SSRC##FIELD; \
    }
#define X(FIELD,IN) \
    if (IN == SOPCInstruction::SSRC##FIELD) \
    { \
        return SOPCInstruction::SSRC##FIELD; \
    }
    //GENERIC_INSTRUCTION_FIELDS_1(ssrc, ridx);
#undef X
#undef X_RANGE

#define X_RANGE(FIELD_MIN,FIELD_MAX,FIELD,IN)\
    if ((IN >= SOPCInstruction::SSRC##FIELD_MIN) && (IN <= SOPCInstruction::SSRC##FIELD_MAX)) \
    { \
        return SOPCInstruction::SSRC##FIELD; \
    }
#define X(FIELD,IN) \
    if (IN == SOPCInstruction::SSRC##FIELD) \
    { \
        return SOPCInstruction::SSRC##FIELD; \
    }
    SCALAR_INSTRUCTION_FIELDS(ssrc);
    GENERIC_INSTRUCTION_FIELDS_2(ssrc);
#undef X
#undef X_RANGE
    return SOPCInstruction::SSRCIllegal;
}

SISOPCInstruction::OP ParserSiSopC::GetSiSopcOp(Instruction::Instruction32Bit hex_instruction)
{
    EXTRACT_INSTRUCTION32_FIELD(hex_instruction, SI, SOPC, op, OP, 16);

    if ((op >= SISOPCInstruction::kIllegal))
    {
        return SISOPCInstruction::kIllegal;
    }
    else
    {
        return op;
    }
}

VISOPCInstruction::OP ParserSiSopC::GetViSopcOp(Instruction::Instruction32Bit hex_instruction)
{
    EXTRACT_INSTRUCTION32_FIELD(hex_instruction, VI, SOPC, op, OP, 16);

    if ((op >= VISOPCInstruction::kIllegal))
    {
        return VISOPCInstruction::kIllegal;
    }
    else
    {
        return op;
    }
}

ParserSi::kaStatus ParserSiSopC::Parse(GDT_HW_GENERATION hw_generation, Instruction::Instruction32Bit hex_instruction, Instruction*& instruction, bool , uint32_t , int label /*=kNoLabel*/ , int goto_label /*=kNoLabel*/)
{
    unsigned int ridx0 = 0, ridx1 = 0;
    SOPCInstruction::SSRC ssrc0 = GetSsrc(hex_instruction, ridx0, 0);
    SOPCInstruction::SSRC ssrc1 = GetSsrc(hex_instruction, ridx1, 1);

    if ((hw_generation == GDT_HW_GENERATION_SEAISLAND) || (hw_generation == GDT_HW_GENERATION_SOUTHERNISLAND))
    {
        SISOPCInstruction::OP op = GetSiSopcOp(hex_instruction);
        instruction = new SISOPCInstruction(ssrc0, ssrc1, op, ridx0, ridx1, label, goto_label);
    }
    else
    {
        VISOPCInstruction::OP op = GetViSopcOp(hex_instruction);
        instruction = new VISOPCInstruction(ssrc0, ssrc1, op, ridx0, ridx1, label, goto_label);
    }

    return ParserSi::kStatusSuccess;
}

ParserSi::kaStatus ParserSiSopC::Parse(GDT_HW_GENERATION, Instruction::Instruction64Bit, Instruction*&, int /*iLabel =kNoLabel*/ , int/* goto_label =kNoLabel*/)
{
    return ParserSi::kStatus64BitInstructionNotSupported;
}

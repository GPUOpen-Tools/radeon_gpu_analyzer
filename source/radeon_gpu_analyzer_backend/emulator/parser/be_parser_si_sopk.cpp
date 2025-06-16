//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation of parser for the Southern Island [SI] SOPK instructions.
//=============================================================================

// Local:
#include "be_parser_si_sopk.h"

SOPKInstruction::SIMM16 ParserSiSopk::GetSiMm16(Instruction::Instruction32Bit hex_instruction, unsigned int& ridx)
{
    EXTRACT_INSTRUCTION32_FIELD(hex_instruction, SI, SOPK, simm16, SIMM16, 0);
#define X_RANGE(FIELD_MIN,FIELD_MAX,FIELD,IN,VAL)\
    if ((IN >= SOPKInstruction::SIMM16##FIELD_MIN) && (IN <= SOPKInstruction::SIMM16##FIELD_MAX)) \
    { \
        VAL = IN; \
        return SOPKInstruction::SIMM16##FIELD; \
    }
#define X(FIELD,IN) \
    if (IN == SOPKInstruction::SIMM16##FIELD) \
    { \
        return SOPKInstruction::SIMM16##FIELD; \
    }
    GENERIC_INSTRUCTION_FIELDS_1(simm16, ridx);
#undef X
#undef X_RANGE

#define X_RANGE(FIELD_MIN,FIELD_MAX,FIELD,IN)\
    if ((IN >= SOPKInstruction::SIMM16##FIELD_MIN) && (IN <= SOPKInstruction::SIMM16##FIELD_MAX)) \
    { \
        return SOPKInstruction::SIMM16##FIELD; \
    }
#define X(FIELD,IN) \
    if (IN == SOPKInstruction::SIMM16##FIELD) \
    { \
        return SOPKInstruction::SIMM16##FIELD; \
    }
    SCALAR_INSTRUCTION_FIELDS(simm16);
    GENERIC_INSTRUCTION_FIELDS_2(simm16);
#undef X
#undef X_RANGE
    return SOPKInstruction::SIMM16Illegal;
}

SISOPKInstruction::OP ParserSiSopk::GetSiSopkOp(Instruction::Instruction32Bit hex_instruction)
{
    EXTRACT_INSTRUCTION32_FIELD(hex_instruction, SI, SOPK, op, OP, 23);

    if ((op > SISOPKInstruction::kMovkI32 && op < SISOPKInstruction::kCMovkI32)
        || (op >= SISOPKInstruction::kReserved))
    {
        return SISOPKInstruction::kReserved;
    }
    else
    {
        return op;
    }
}

VISOPKInstruction::OP ParserSiSopk::GetViSopkOp(Instruction::Instruction32Bit hex_instruction)
{
    EXTRACT_INSTRUCTION32_FIELD(hex_instruction, VI, SOPK, op, OP, 23);

    if (op > VISOPKInstruction::kIllegal)
    {
        return VISOPKInstruction::kIllegal;
    }
    else
    {
        return op;
    }
}

G9SOPKInstruction::OP ParserSiSopk::GetVegaSopkOp(Instruction::Instruction32Bit hex_instruction)
{
    EXTRACT_INSTRUCTION32_FIELD(hex_instruction, G9, SOPK, op, OP, 23);

    return (op < G9SOPKInstruction::kIllegal ? op : G9SOPKInstruction::kIllegal);
}

SOPKInstruction::SDST ParserSiSopk::GetSdst(Instruction::Instruction32Bit hex_instruction, unsigned int& ridx)
{
    EXTRACT_INSTRUCTION32_FIELD(hex_instruction, SI, SOPK, sdst, SDST, 16);
#define X_RANGE(FIELD_MIN,FIELD_MAX,FIELD,IN,VAL)\
    if ((IN >= SOPKInstruction::SDST##FIELD_MIN) && (IN <= SOPKInstruction::SDST##FIELD_MAX)) \
    { \
        VAL = IN; \
        return SOPKInstruction::SDST##FIELD; \
    }
#define X(FIELD,IN) \
    if (IN == SOPKInstruction::SDST##FIELD) \
    { \
        return SOPKInstruction::SDST##FIELD; \
    }
    GENERIC_INSTRUCTION_FIELDS_1(sdst, ridx);
#undef X
#undef X_RANGE
    return SOPKInstruction::SDSTIllegal;
}

ParserSi::kaStatus ParserSiSopk::Parse(GDT_HW_GENERATION hw_generation, Instruction::Instruction32Bit hex_instruction,
    Instruction*& instruction, bool , uint32_t, int label /*=kNoLabel*/ , int goto_label /*=kNoLabel*/)
{
    kaStatus status = kStatusSuccess;
    unsigned int simm16_ridx = 0, sdstRidx = 0;
    SOPKInstruction::SIMM16 simm16 = GetSiMm16(hex_instruction, simm16_ridx);
    SOPKInstruction::SDST sdst = GetSdst(hex_instruction, sdstRidx);

    switch (hw_generation)
    {
        case GDT_HW_GENERATION_SEAISLAND:
        case GDT_HW_GENERATION_SOUTHERNISLAND:
        {
            SISOPKInstruction::OP op = GetSiSopkOp(hex_instruction);
            instruction = new SISOPKInstruction(simm16, op, sdst, simm16_ridx, sdstRidx, label, goto_label);
            break;
        }
        case GDT_HW_GENERATION_VOLCANICISLAND:
        {
            VISOPKInstruction::OP op = GetViSopkOp(hex_instruction);
            instruction = new VISOPKInstruction(simm16, op, sdst, simm16_ridx, sdstRidx, label, goto_label);
            break;
        }
        case GDT_HW_GENERATION_GFX9:
        {
            G9SOPKInstruction::OP op = GetVegaSopkOp(hex_instruction);
            instruction = new G9SOPKInstruction(simm16, op, sdst, simm16_ridx, sdstRidx, label, goto_label);
            break;
        }
        default:
            status = kStatusUnexpectedHwGeneration;
    }

    return status;
}

ParserSi::kaStatus ParserSiSopk::Parse(GDT_HW_GENERATION, Instruction::Instruction64Bit, Instruction*&, int /*label =kNoLabel*/ , int /*goto_label =kNoLabel*/)
{
    return ParserSi::kStatus64BitInstructionNotSupported;
}


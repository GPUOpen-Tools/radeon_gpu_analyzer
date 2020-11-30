//=================================================================
// Copyright 2020 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

#include "be_parser_si_smrd.h"

char ParserSiSmrd::GetOffset(Instruction::Instruction32Bit hex_instruction)
{
    EXTRACT_INSTRUCTION32_FIELD(hex_instruction, SI, SMRD, offset, OFFSET, 0);
    RETURN_EXTRACT_INSTRUCTION(offset);
}

SMRDInstruction::IMM ParserSiSmrd::GetImm(Instruction::Instruction32Bit hex_instruction)
{
    Instruction::Instruction32Bit imm = (hex_instruction & SMRDMask_IIM) >> 8;
    return static_cast<SMRDInstruction::IMM>(imm != 0);
}

SMRDInstruction::SBASE ParserSiSmrd::GetSBase(Instruction::Instruction32Bit hex_instruction)
{
    EXTRACT_INSTRUCTION32_FIELD(hex_instruction, SI, SMRD, sbase, SBASE, 8);
    RETURN_EXTRACT_INSTRUCTION(sbase);
}

SMRDInstruction::SDST ParserSiSmrd::GetSdst(Instruction::Instruction32Bit hex_instruction, unsigned int& ridx)
{
    SMRDInstruction::SDST sdst = static_cast<SMRDInstruction::SDST>((hex_instruction & SMRDMask_SDST) >> 15);
#define X_RANGE(FIELD_MIN,FIELD_MAX,FIELD,IN,VAL)\
    if ((IN >= SMRDInstruction::SDST##FIELD_MIN) && (IN <= SMRDInstruction::SDST##FIELD_MAX)) \
    { \
        VAL = IN; \
        return SMRDInstruction::SDST##FIELD; \
    }
#define X(FIELD,IN) \
    if (IN == SMRDInstruction::SDST##FIELD) \
    { \
        return SMRDInstruction::SDST##FIELD; \
    }
    GENERIC_INSTRUCTION_FIELDS_1(sdst, ridx);
#undef X
#undef X_RANGE
    return SMRDInstruction::SDSTIllegal;
}

SISMRDInstruction::OP ParserSiSmrd::GetSiSmrdOp(Instruction::Instruction32Bit hex_instruction)
{
    EXTRACT_INSTRUCTION32_FIELD(hex_instruction, SI, SMRD, op, OP, 22);

    if ((op > SISMRDInstruction::kLoadDwordX16 && op < SISMRDInstruction::kBufferLoadDword) ||
        (op > SISMRDInstruction::kBufferLoadDwordX16 && op < SISMRDInstruction::kMemtime) ||
        (op >= SISMRDInstruction::kReserved))
    {
        return SISMRDInstruction::kReserved;
    }
    else
    {
        return op;
    }
}

VISMEMInstruction::OP ParserSiSmrd::GetViSmrdOp(Instruction::Instruction64Bit hex_instruction)
{
    VISMEMInstruction::OP theOp;

    // we want bits[26:18] out of the 64bit.
    Instruction::Instruction64Bit the32 = static_cast<Instruction::Instruction64Bit>(hex_instruction << 38);
    Instruction::Instruction64Bit the321 = static_cast<Instruction::Instruction64Bit>(the32 >> 56);
    theOp    = static_cast<VISMEMInstruction::OP>(the321);

    if ((theOp > VISMEMInstruction::kStoreDwordx4 && theOp < VISMEMInstruction::kBufferStoreDword) ||
        (theOp > VISMEMInstruction::kBufferStoreDwordx4 && theOp < VISMEMInstruction::kDcacheInv) ||
        (theOp >= VISMEMInstruction::kIllegal))
    {
        return VISMEMInstruction::kIllegal;
    }
    else
    {
        return theOp;
    }
}

ParserSi::kaStatus ParserSiSmrd::Parse(GDT_HW_GENERATION hw_generation, Instruction::Instruction32Bit hex_instruction, Instruction*& instruction, bool , uint32_t, int label, int goto_label)
{
    if (hw_generation == GDT_HW_GENERATION_VOLCANICISLAND)
    {
        return ParserSi::kStatus32BitInstructionNotSupported;
    }

    unsigned int ridx = 0;

    // todo offset: [32:52]- it is not a char
    SMRDInstruction::OFFSET offset = GetOffset(hex_instruction);
    SMRDInstruction::IMM imm    = GetImm(hex_instruction);
    SMRDInstruction::SBASE sbase  = GetSBase(hex_instruction);
    SMRDInstruction::SDST sdst = GetSdst(hex_instruction, ridx);
    SISMRDInstruction::OP op = GetSiSmrdOp(hex_instruction);
    instruction = new SISMRDInstruction(offset, imm, sbase, sdst, ridx, op, label, goto_label);

    return ParserSi::kStatusSuccess;
}

ParserSi::kaStatus ParserSiSmrd::Parse(GDT_HW_GENERATION hw_generation, Instruction::Instruction64Bit hex_instruction, Instruction*& instruction, int label, int goto_label)
{
    if ((hw_generation == GDT_HW_GENERATION_SEAISLAND) || (hw_generation == GDT_HW_GENERATION_SOUTHERNISLAND))
    {
        return ParserSi::kStatus64BitInstructionNotSupported;
    }

    unsigned int ridx = 0;
    SMRDInstruction::OFFSET offset = GetOffset(Instruction::Instruction32Bit(hex_instruction & 0xffff));
    SMRDInstruction::IMM imm = GetImm(Instruction::Instruction32Bit(hex_instruction & 0xffff));
    SMRDInstruction::SBASE sbase = GetSBase(Instruction::Instruction32Bit(hex_instruction & 0xffff));
    SMRDInstruction::SDST sdst = GetSdst(Instruction::Instruction32Bit(hex_instruction & 0xffff), ridx);
    VISMEMInstruction::OP op = GetViSmrdOp(hex_instruction);
    instruction = new VISMEMInstruction(offset, imm, sbase, sdst, ridx, op, label, goto_label);

    return ParserSi::kStatusSuccess;
}

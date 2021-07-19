//=================================================================
// Copyright 2020 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

// Local.
#include "be_parser_si.h"

ParserSi::InstructionEncoding ParserSi::GetInstructionEncoding(Instruction::Instruction32Bit hex_instruction)
{
    // Try all instructions encoding masks from longest to shortest until
    // legal InstructionEncoding is found.
    Instruction::Instruction32Bit instruction_encoding;

    // kInstructionEncodingMask9bit
    instruction_encoding = (hex_instruction & kInstructionEncodingMask9bit);

    switch (instruction_encoding)
    {
        case kInstructionEncodingSop1:
        case kInstructionEncodingSopp:
        case kInstructionEncodingSopc:
            return static_cast<InstructionEncoding>(instruction_encoding);

        default:
            break;
    }

    // kInstructionEncodingMask7bit
    instruction_encoding = (hex_instruction & kInstructionEncodingMask7bit);

    switch (instruction_encoding)
    {
        case kInstructionEncodingVop1:
        case kInstructionEncodingVopc:
            return static_cast<InstructionEncoding>(instruction_encoding);

        default:
            break;
    }

    // kInstructionEncodingMask6bit
    instruction_encoding = (hex_instruction & kInstructionEncodingMask6bit);

    switch (instruction_encoding)
    {
        case kInstructionEncodingVop3:
        case InstructionEncodingExp:
        case kInstructionEncodingVintrp:
        case kInstructionEncodingDs:
        case kInstructionEncodingMubuf:
        case kInstructionEncodingMtbuf:
        case kInstructionEncodingMimg:
        case kViInstructionEncodingSmem:
        case kViInstructionEncodingVintrp:
        case kViInstructionEncodingFlat:
            return static_cast<InstructionEncoding>(instruction_encoding);

        default:
            break;
    }

    // kInstructionEncodingMask5bit
    instruction_encoding = (hex_instruction & kInstructionEncodingMask5bit);

    switch (instruction_encoding)
    {
        case kInstructionEncodingSmrd:
            return static_cast<InstructionEncoding>(instruction_encoding);

        default:
            break;
    }

    // kInstructionEncodingMask4bit
    instruction_encoding = (hex_instruction & kInstructionEncodingMask4bit);

    switch (instruction_encoding)
    {
        case kInstructionEncodingSopk:
            return static_cast<InstructionEncoding>(instruction_encoding);

        default:
            break;
    }

    // kInstructionEncodingMask2bit
    instruction_encoding = (hex_instruction & kInstructionEncodingMask2bit);

    switch (instruction_encoding)
    {
        case kInstructionEncodingSop2:
            return static_cast<InstructionEncoding>(instruction_encoding);

        default:
            break;
    }

    // kInstructionEncodingMask1bit
    instruction_encoding = (hex_instruction & kInstructionEncodingMask1bit);

    switch (instruction_encoding)
    {
        case kInstructionEncodingVop2:
            return static_cast<InstructionEncoding>(instruction_encoding);

        default:
            break;
    }

    // If no legal InstructionEncoding found return kInstructionEncodingIllegal
    return kInstructionEncodingIllegal;
}

void ParserSi::SetLog(ParserSi::LoggingCallBackFuncP callback)
{
    log_callback_ = callback;
}

ParserSi::LoggingCallBackFuncP ParserSi::log_callback_;

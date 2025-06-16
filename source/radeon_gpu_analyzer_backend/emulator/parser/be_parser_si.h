//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header of parser for the Southern Island [SI] instructions.
//=============================================================================

#ifndef RGA_RADEONGPUANALYZERBACKEND_SRC_EMULATOR_PARSER_BE_PARSER_SI_H_
#define RGA_RADEONGPUANALYZERBACKEND_SRC_EMULATOR_PARSER_BE_PARSER_SI_H_

#include <string>
#include "be_instruction.h"
#include "be_instruction_smrd.h"
#include "be_instruction_sopp.h"
#include "be_instruction_sopc.h"
#include "be_instruction_sop1.h"
#include "be_instruction_sopk.h"
#include "be_instruction_sop2.h"
#include "be_instruction_vintrp.h"
#include "be_instruction_ds.h"
#include "be_instruction_mubuf.h"
#include "be_instruction_mtbuf.h"
#include "be_instruction_mimg.h"
#include "be_instruction_exp.h"
#include "be_instruction_vop.h"

// Interface a parser for the Southern Island [SI] instructions.
class ParserSi
{
public:
    // Return Codes
    // Anything other than SUCCESS is a problem.
    enum kaStatus
    {
        kStatusSuccess,
        kStatus32BitInstructionNotSupported,
        kStatus64BitInstructionNotSupported,
        kStatusUnexpectedHwGeneration
    };

    // SI instruction`s encoding mask
    enum InstructionEncodingMask
    {
        kInstructionEncodingMask9bit = 0x000001FF << 23,
        kInstructionEncodingMask7bit = 0x0000007F << 25,
        kInstructionEncodingMask6bit = 0x0000003F << 26,
        kInstructionEncodingMask5bit = 0x0000001F << 27,
        kInstructionEncodingMask4bit = 0x0000000F << 28,
        kInstructionEncodingMask2bit = 0x00000003 << 30,
        kInstructionEncodingMask1bit = 0x00000001 << 31
    };

    // SI instruction`s encoding
    // InstructionEncoding has uniform representation : [encoding] << [encoding start bit]
    enum InstructionEncoding
    {
        // kInstructionEncodingMask9bit.
        // bits [31:23] - (1 0 1 1 1 1 1 0 1).
        kInstructionEncodingSop1 = 0x0000017D << 23,

        // bits [31:23] - (1 0 1 1 1 1 1 1 1).
        kInstructionEncodingSopp = 0x0000017F << 23,

        // bits [31:23] - (1 0 1 1 1 1 1 1 0).
        kInstructionEncodingSopc = 0x0000017E << 23,

        // kInstructionEncodingMask7bit.
        // bits [31:25] - (0 1 1 1 1 1 1).
        kInstructionEncodingVop1 = 0x0000003F << 25,

        // bits [31:25] - (0 1 1 1 1 1 0).
        kInstructionEncodingVopc = 0x0000003E << 25,

        // kInstructionEncodingMask6bit.
        // bits [31:26] - (1 1 0 1 0 0).
        kInstructionEncodingVop3 = 0x00000034 << 26,

        // bits [31:26] - (1 1 1 1 1 0).
        InstructionEncodingExp = 0x0000003E << 26,

        // bits [31:26] - (1 1 0 0 1 0).
        kInstructionEncodingVintrp = 0x00000032 << 26,

        // bits [31:26] - (1 1 0 1 1 0).
        kInstructionEncodingDs =  0x00000036 << 26,

        // bits [31:26] - (1 1 1 0 0 0).
        kInstructionEncodingMubuf =  0x00000038 << 26,

        // bits [31:26] - (1 1 1 0 1 0).
        kInstructionEncodingMtbuf =  0x0000003A << 26,

        // bits [31:26] - (1 1 1 1 0 0).
        kInstructionEncodingMimg =  0x0000003C << 26,

        // kInstructionEncodingMask5bit.
        // bits [31:27] - (1 1 0 0 0).
        kInstructionEncodingSmrd = 0x00000018 << 27,

        // kInstructionEncodingMask4bit.
        // bits [31:28] - (1 0 1 1).
        kInstructionEncodingSopk = 0x0000000B << 28,

        // kInstructionEncodingMask2bit.
        // bits [31:30] - (1 0).
        kInstructionEncodingSop2 = 0x00000002 << 30,

        // kInstructionEncodingMask1bit.
        // bits [31:31] - (0).
        kInstructionEncodingVop2 = 0x00000000 << 31,

        // bits [31:26] - (1 1 0 0 0 0).
        kViInstructionEncodingSmem = 0x00000030 << 26,

        // bits [31:26] - (1 1 0 1 0 1).
        kViInstructionEncodingVintrp = 0x00000035 << 26,

        // bits [31:26] - (1 1 0 1 1 1).
        kViInstructionEncodingFlat = 0x00000037 << 26,

        kInstructionEncodingIllegal
    };

    ParserSi() = default;
    virtual ~ParserSi() = default;

    // Parse a 32-bit instruction.
    virtual ParserSi::kaStatus Parse(GDT_HW_GENERATION hwGen, Instruction::Instruction32Bit hex_instruction, Instruction*& instruction, bool is_literal_32b = false, uint32_t literal_32b = 0, int label = kNoLabel, int goto_label = kNoLabel) = 0;

    // Parse a 64-bit instruction.
    virtual ParserSi::kaStatus Parse(GDT_HW_GENERATION hwGen, Instruction::Instruction64Bit hexInstruction, Instruction*& instruction, int label_ = kNoLabel , int iGotoLabel = kNoLabel) = 0;

    // Get instruction encoding.
    static InstructionEncoding GetInstructionEncoding(Instruction::Instruction32Bit hexInstruction);

    // Logging callback type.
    typedef void (*LoggingCallBackFuncP)(const std::string&);

    // Set callback function for diagnostic output.
    // If NULL, no output is generated.
    // Used to report :
    // 1)Unrecognized Instruction (encoding)
    // 2)Unrecognized Instruction fields
    // 3)Memory allocation failures
    // 4)Report of start/end of parsing
    // \param[in] callback a pointer to callback function.
    static void SetLog(LoggingCallBackFuncP callback);

private:
    // Stream for diagnostic output.
    static LoggingCallBackFuncP log_callback_;
};

#endif //RGA_RADEONGPUANALYZERBACKEND_SRC_EMULATOR_PARSER_BE_PARSER_SI_H_


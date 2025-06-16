//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header of parser for the Southern Island [SI] SOPC instructions.
//=============================================================================

#ifndef RGA_RADEONGPUANALYZERBACKEND_SRC_EMULATOR_PARSER_BE_PARSER_SI_SOPC_H_
#define RGA_RADEONGPUANALYZERBACKEND_SRC_EMULATOR_PARSER_BE_PARSER_SI_SOPC_H_

#include "be_parser_si.h"

// Parser for the Southern Island [SI] SOPC instructions.
class ParserSiSopC : public ParserSi
{
public:
    ParserSiSopC() = default;
    ~ParserSiSopC() = default;

    virtual ParserSi::kaStatus Parse(GDT_HW_GENERATION hw_generation, Instruction::Instruction32Bit hex_instruction,
        Instruction * &instruction, bool is_literal_32b, uint32_t literal_32b, int label = kNoLabel, int goto_label = kNoLabel) override;

    virtual ParserSi::kaStatus Parse(GDT_HW_GENERATION hw_generation, Instruction::Instruction64Bit hex_instruction,
        Instruction * &instruction, int label = kNoLabel, int goto_label = kNoLabel) override;

private:
    // SOPC Instruction`s fields masks
    enum SOPCMask
    {
        //    SSRC0 [7:0]
        SOPCMask_SSRC0 = 0x000000FF,
        //    SSRC1 [15:8]
        SOPCMask_SSRC1 = 0x000000FF << 8,
        //    OP     [22:16]
        SOPCMask_OP   =   0x0000007F << 16
    };

    // Get SOPC instruction`s SSRC field.
    static SOPCInstruction::SSRC GetSsrc(Instruction::Instruction32Bit hex_instruction, unsigned int& ridx, unsigned int idx_ssrc);

    // Get SOPC instruction`s OP field for SI.
    static SISOPCInstruction::OP GetSiSopcOp(Instruction::Instruction32Bit hex_instruction);

    // Get SOPC instruction`s OP field for VI.
    static VISOPCInstruction::OP GetViSopcOp(Instruction::Instruction32Bit hex_instruction);
};

#endif //RGA_RADEONGPUANALYZERBACKEND_SRC_EMULATOR_PARSER_BE_PARSER_SI_SOPC_H_


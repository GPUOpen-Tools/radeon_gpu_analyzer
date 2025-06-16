//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for class representing SOPC Instruction in shader isa.
//=============================================================================

#ifndef __SOPCINSTRUCTION_H
#define __SOPCINSTRUCTION_H

#include "be_instruction.h"

// Scalar instruction taking two inputs and producing a comparison result.
// Can be followed by a 32-bit literal constant.
// Opcode :
//        SSRC0 [7:0]
//        SSRC1 [15:8]
//        OP [22:16]

class SOPCInstruction : public Instruction
{
public:

#define X(SYM) SSRC##SYM
#define X_INIT(SYM,VAL) SSRC##SYM = VAL
    // Destination for scalar memory read instruction
    enum SSRC
    {
#include "be_instruction_fields_generic1.h"
#include "be_instruction_fields_scalar.h"
#include "be_instruction_fields_generic2.h"
        X(Illegal)
    };
#undef X_INIT
#undef X

private:
    // Source 0. First operand for the instruction.
    SSRC ssrc0_;

    // Source 1. Second operand for instruction.
    SSRC ssrc1_;

    // Registers index (ssrc0_).
    // Note : Relevant only if ssrc0_ == ScalarGPR or ssrc0_ == ScalarTtmp
    unsigned int sridx0_;

    // Registers index (ssrc1_).
    // Note : Relevant only if ssrc1_ == ScalarGPR or ssrc1_ == ScalarTtmp
    unsigned int sridx1_;

    // SOPP Instruction Width in bits
    static const unsigned int sopc_instruction_width = 32;
public:

    SOPCInstruction(SSRC ssrc0, SSRC ssrc1, unsigned int sridx0, unsigned int sridx1, int label, int goto_label) :
        Instruction(sopc_instruction_width, InstructionCategory::kScalarAlu, kInstructionSetSopc, label, goto_label),
        ssrc0_(ssrc0), ssrc1_(ssrc1), sridx0_(sridx0), sridx1_(sridx1) {}

    ~SOPCInstruction() = default;

    // Get the SSRC0 [7:0]
    SSRC GetSsrc0() const { return ssrc0_; }

    // Get the SSRC1 [15:8]
    SSRC GetSsrc1() const { return ssrc0_; }

    // Get the (scalar) register`s index.
    // Note : Relevant only if m_ssrc == ScalarGPR or m_ssrc == ScalarTtmp
    unsigned int GetSridx0() const { return sridx0_; }

    // Get the (scalar) register`s index.
    // Note : Relevant only if m_ssrc == ScalarGPR or m_ssrc == ScalarTtmp
    unsigned int GetSridx1() const { return sridx1_; }
};

class SISOPCInstruction : public SOPCInstruction
{
public:
    // Selector for the SOPC Instruction
    enum OP
    {
        //  SCC = (S0.i == S1.i).
        kCmpEqi32,
        //  SCC = (S0.i != S1.i).
        kCmpLgi32,
        //  SCC = (S0.i > S1.i).
        kCmpGti32,
        //  SCC = (S0.i >= S1.i).
        kCmpGei32,
        //  SCC = (S0.i < S1.i).
        kCmpLti32,
        //  SCC = (S0.i <= S1.i).
        kCmpLei32,
        //  SCC = (S0.u == S1.u).
        kCmpEqu32,
        //  SCC = (S0.u != S1.u).
        kCmpLgu32,
        //  SCC = (S0.u > S1.u).
        kCmpGtu32,
        //  SCC = (S0.u >= S1.u).
        kCmpGeu32,
        //  SCC = (S0.u < S1.u).
        kCmpLtu32,
        //  SCC = (S0.u <= S1.u).
        kCmpLeu32,
        //  SCC = (S0.u[S1.u[4:0]] == 0).
        kBitCmp0B32,
        //  SCC = (S0.u[S1.u[4:0]] == 1).
        kBitCmp1B32,
        //  SCC = (S0.u[S1.u[5:0]] == 0).
        kBITCmp0B64,
        //  SCC = (S0.u[S1.u[5:0]] == 1).
        kBitCmp1B64,
        //  VSKIP = S0.u[S1.u[4:0]].
        kSetvskip,
        // ILLEGAL
        kIllegal
    };

    // Get the OP [22:16]
    OP GetOp() const { return op_; }

    // ctor
    SISOPCInstruction(SSRC ssrc0, SSRC ssrc1, OP op, unsigned int sridx0, unsigned int sridx1, int label_, int iGotoLabel) : SOPCInstruction(ssrc0, ssrc1, sridx0, sridx1, label_, iGotoLabel), op_(op) {}


private:
    // SOPC operation.
    OP op_;


};

class VISOPCInstruction : public SOPCInstruction
{
public:
    // Selector for the SOPC Instruction
    enum OP
    {
        kCmpEqI32 = 0,
        kCmpLgI32 = 1,
        kCmpGtI32 = 2,
        kCmpGeI32 = 3,
        kCmpLtI32 = 4,
        kCmpLeI32 = 5,
        kCmpEqU32 = 6,
        kCmpLgU32 = 7,
        kCmpGtU32 = 8,
        kCmpGeU32 = 9,
        kCmpLtU32 = 10,
        kCmpLeU32 = 11,
        kBitcmp0B32 = 12,
        kBitcmp1B32 = 13,
        kBitcmp0B64 = 14,
        kBitcmp1B64 = 15,
        kSetvskip = 16,
        kSetGprIdxOn = 17,
        kCmpEqU64 = 18,
        kCmpLgU64 = 19,
        kIllegal = 20,
    };

    // Get the OP [22:16]
    OP GetOp() const { return op_; }

    VISOPCInstruction(SSRC ssrc0, SSRC ssrc1, OP op, unsigned int sridx0, unsigned int sridx1,
        int label, int goto_label) : SOPCInstruction(ssrc0, ssrc1, sridx0, sridx1, label, goto_label), op_(op)
    {
        hw_gen_ = GDT_HW_GENERATION_VOLCANICISLAND;
    }

private:
    // SOPC operation.
    OP op_;
};


#endif //__SOPCINSTRUCTION_H


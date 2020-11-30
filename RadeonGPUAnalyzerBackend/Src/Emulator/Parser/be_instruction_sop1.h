//=================================================================
// Copyright 2020 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

#ifndef __SOP1INSTRUCTION_H
#define __SOP1INSTRUCTION_H

#include "be_instruction.h"

// This is a scalar instruction with one input and one output.
// Can be followed by a 32-bit literal constant.
// Opcode :
//        SSRC0 [7:0]
//        OP [15:8]
//        SDST [22:16]

class SOP1Instruction : public Instruction
{
public:

#define X(SYM) SSRC##SYM
#define X_INIT(SYM,VAL) SSRC##SYM = VAL
    // Destination for SSRC
    enum SSRC
    {
#include "be_instruction_fields_generic1.h"
#include "be_instruction_fields_scalar.h"
#include "be_instruction_fields_generic2.h"
        X(Illegal)
    };
#undef X_INIT
#undef X

#define X(SYM) SDST##SYM
#define X_INIT(SYM,VAL) SDST##SYM = VAL
    // Destination for SDST
    enum SDST
    {
#include "be_instruction_fields_generic1.h"
        X(Illegal)
    };
#undef X_INIT
#undef X

private:
    // Source 0. First operand for the instruction.
    SSRC ssrc0_;

    // Registers index (ssrc0_).
    // Note : Relevant only if dst_ == ScalarGPR or dst_ == ScalarTtmp
    unsigned int sridx0_;

    // Scalar destination for instruction..
    SDST sdst_;

    // Registers index (dst_).
    // Note : Relevant only if dst_ == ScalarGPR or dst_ == ScalarTtmp
    unsigned int sdst_ridx_;

    // SOP1 Instruction Width in bits
    static const unsigned int sop1_instruction_width_ = 32;
public:
    SOP1Instruction(SSRC ssrc0, SDST sdst, unsigned int sridx0, unsigned int sdstRidx, int label, int goto_label) :
        Instruction(sop1_instruction_width_, InstructionCategory::kScalarAlu, kInstructionSetSop1, label, goto_label),
        ssrc0_(ssrc0), sridx0_(sridx0), sdst_(sdst), sdst_ridx_(sdstRidx) { }

    ~SOP1Instruction() = default;

    // Get the SSRC0 [7:0]
    SSRC GetSSRC0() const { return ssrc0_; }

    // Get the SDST [22:16]
    SDST GetSdst() const { return sdst_; }

    // Get the (scalar) register`s index.
    // Note : Relevant only if m_ssrc == ScalarGPR or m_ssrc == ScalarTtmp
    unsigned int GetSRidx() const { return sridx0_; }

    // Get the (scalar) register`s index.
    // Note : Relevant only if dst_ == ScalarGPR or dst_ == ScalarTtmp
    unsigned int GetSDSTRidx() const { return sdst_ridx_; }
};

class SISOP1Instruction : public SOP1Instruction
{
public:
    // Selector for the SOP1 Instruction
    enum OP
    {
        //  D.u = S0.u.
        kMovB32 = 3,
        //  D/u = S0.u.
        kMovB64,
        //  if(SCC) D.u = S0.u; else NOP.
        kCMovB32,
        //  if(SCC) D.u = S0.u; else NOP.
        kCMovB64,
        //  D.u = ~S0.u SCC = 1 if result non-zero.
        kNotB32,
        //  D.u = ~S0.u SCC = 1 if result non-zero.
        kNotB64,
        //  D.u = WholeQuadMode(S0.u). SCC = 1 if result is non-zero.
        kWqmB32,
        //  D.u = WholeQuadMode(S0.u). SCC = 1 if result is non-zero.
        kWqmB64,
        //  D.u = S0.u[0:31] (reverse bits).
        kBrevB32,
        //  D.u = S0.u[0:63] (reverse bits).
        kBrevB64,
        //  D.i = CountZeroBits(S0.u). SCC = 1 if result is non-zero.
        kBcnt0I32B32,
        //  D.i = CountZeroBits(S0.u). SCC = 1 if result is non-zero.
        kBcnt0I32B64,
        //  D.i = CountOneBits(S0.u). SCC = 1 if result is non-zero.
        kBcnt1I32B32,
        //  D.i = CountOneBits(S0.u). SCC = 1 if result is non-zero.
        kBcnt1I32B64,
        //  D.i = FindFirstZero(S0.u) from LSB; if no zeros, return -1.
        kFf0I32B32,
        //  D.i = FindFirstZero(S0.u) from LSB; if no zeros, return -1.
        kFf0I32B64,
        //  D.i = FindFirstOne(S0.u) from LSB; if no ones, return -1.
        kFf1I32B32,
        //  D.i = FindFirstOne(S0.u) from LSB; if no ones, return -1.
        kFf1I32B64,
        //  D.i = FindFirstOne(S0.u) from MSB; if no ones, return -1.
        kFlbitI32B32,
        //  D.i = FindFirstOne(S0.u) from MSB; if no ones, return -1.
        kFlbitI32B64,
        //  D.i = Find first bit opposite of sign bit from MSB. If S0 == -1,
        // return -1.
        kFlbitI32,
        //  D.i = Find first bit opposite of sign bit from MSB. If
        // S0 == -1, return -1.
        kFlbitI32_I64,
        //  D.i = signext(S0.i[7:0]).
        kSEXT_I32_I8,
        //  D.i = signext(S0.i[15:0]).
        kSEXT_I32_I16,
        //  D.u[S0.u[4:0]] = 0.
        kBitset0B32,
        //  D.u[S0.u[5:0]] = 0.
        kBitset0B64,
        //  D.u[S0.u[4:0]] = 1.
        kBitset1B32,
        //  D.u[S0.u[5:0]] = 1.
        kBitset1B64,
        //  D.u = PC + 4; destination receives the byte address of the next
        // instruction.
        kGetpcB64,
        //  PC = S0.u; S0.u is a byte address of the instruction to jump to.
        kSetpcB64,
        //  D.u = PC + 4; PC = S0.u.
        kSwappcB64,
        //  Return from Exception; PC = TTMP1,0.
        kRfeB64,
        //  D.u = EXEC, EXEC = S0.u & EXEC. SCC = 1 if the
        // new value of EXEC is non-zero.
        kAndSaveexecB64 = 36,
        //  D.u = EXEC, EXEC = S0.u | EXEC. SCC = 1 if the new
        // value of EXEC is non-zero.
        kORSaveexecB64,
        //  D.u = EXEC, EXEC = S0.u ^ EXEC. SCC = 1 if the
        // new value of EXEC is non-zero.
        kXorSaveexecB64,
        //  D.u = EXEC, EXEC = S0.u & ~EXEC. SCC = 1
        //if the new value of EXEC is non-zero.
        kAndn2SaveexecB64,
        //  D.u = EXEC, EXEC = S0.u | ~EXEC. SCC = 1 if
        // the new value of EXEC is non-zero.
        kOrn2SaveexecB64,
        //  D.u = EXEC, EXEC = ~(S0.u & EXEC). SCC = 1
        //if the new value of EXEC is non-zero.
        kNAndSaveexecB64,
        //  D.u = EXEC, EXEC = ~(S0.u | EXEC). SCC = 1 if the
        // new value of EXEC is non-zero.
        kNorSaveexecB64,
        //  D.u = EXEC, EXEC = ~(S0.u ^ EXEC). SCC = 1 if the
        // new value of EXEC is non-zero.
        kXnorSaveexecB64,
        //  D.u = QuadMask(S0.u). D[0] = OR(S0[3:0]),
        // D[1] = OR(S0[7:4]) .... SCC = 1 if result is non-zero.
        kQuadmaskB32,
        //  D.u = QuadMask(S0.u). D[0] = OR(S0[3:0]),
        // D[1] = OR(S0[7:4]) .... SCC = 1 if result is non-zero
        kQuadmaskB64,
        //  SGPR[D.u] = SGPR[S0.u + M0.u].
        kMovrelsB32,
        //  SGPR[D.u] = SGPR[S0.u + M0.u].
        kMovrelsB64,
        //  SGPR[D.u + M0.u] = SGPR[S0.u].
        kMovreldB32,
        //  SGPR[D.u + M0.u] = SGPR[S0.u].
        kMovreldB64,
        //  Conditional branch join point. Arg0 = saved CSP value. No
        // dest.
        kCbranchJoin,
        //  D.i = abs(S0.i). SCC=1 if result is non-zero.
        kAbsI32 = 52,
        //  D.u = S0.u, introduce edc double error upon write to dest
        // sgpr.
        // All other values are reserved.
        kMovFedB32,
        // Reserved
        kReserved
    };

    // Get the OP [15:8].
    OP GetOp() const { return op_; }

    SISOP1Instruction(SSRC ssrc0, OP op, SDST sdst, unsigned int sridx0, unsigned int sdst_ridx,
        int label, int goto_label) : SOP1Instruction(ssrc0, sdst, sridx0, sdst_ridx, label, goto_label), op_(op) { }

private:
    // SOP1 operation.
    OP op_;
};

class VISOP1Instruction : public SOP1Instruction
{
public:
    // Selector for the SOP1 Instruction
    enum OP
    {
        kMovB32 = 0,
        kMovB64 = 1,
        kCmovB32 = 2,
        kCmovB64 = 3,
        kNotB32 = 4,
        kNotB64 = 5,
        kWqmB32 = 6,
        kWqmB64 = 7,
        kBrevB32 = 8,
        kBrevB64 = 9,
        kBcnt0I32B32 = 10,
        kBcnt0I32B64 = 11,
        kBcnt1I32B32 = 12,
        kBcnt1I32B64 = 13,
        kFf0I32B32 = 14,
        kFf0I32B64 = 15,
        kFf1I32B32 = 16,
        kFf1I32B64 = 17,
        kFlbitI32B32 = 18,
        kFlbitI32B64 = 19,
        kFlbitI32 = 20,
        kFlbitI32i64 = 21,
        kSextI32I8 = 22,
        kSextI32I16 = 23,
        kBitset0B32 = 24,
        kBitset0B64 = 25,
        kBitset1B32 = 26,
        kBitset1B64 = 27,
        kGetpcB64 = 28,
        kSetpcB64 = 29,
        kSwappcB64 = 30,
        kRfeB64 = 31,
        kAndSaveexecB64 = 32,
        kOrSaveexecB64 = 33,
        kXorSaveexecB64 = 34,
        kAndn2SaveexecB64 = 35,
        kOrn2SaveexecB64 = 36,
        kNandSaveexecB64 = 37,
        kNorSaveexecB64 = 38,
        kXnorSaveexecB64 = 39,
        kQuadmaskB32 = 40,
        kQuadmaskB64 = 41,
        kMovrelsB32 = 42,
        kMovrelsB64 = 43,
        kMovreldB32 = 44,
        kMovreldB64 = 45,
        kCbranchJoin = 46,
        kAbsI32 = 48,
        kMovFedB32 = 49,
        kSetGprIdx = 50,
        kIllegal = 51,
    };

    // Get the OP [15:8].
    OP GetOp() const { return op_; }

    // ctor
    VISOP1Instruction(SSRC ssrc0, OP op, SDST sdst, unsigned int sridx0, unsigned int sdst_ridx,
        int label, int goto_label) : SOP1Instruction(ssrc0, sdst, sridx0, sdst_ridx, label, goto_label), op_(op)
    {
        hw_gen_ = GDT_HW_GENERATION_VOLCANICISLAND;
    }
private:
    // SOP1 operation.
    OP op_;

};

// SOP1 instructions for VEGA (GFX9)
class G9SOP1Instruction : public SOP1Instruction
{
public:
    // Selector for the SOP1 Instruction
    enum OP
    {
        kMovB32 = 0,
        kMovB64 = 1,
        kCmovB32 = 2,
        kCmovB64 = 3,
        kNotB32 = 4,
        kNotB64 = 5,
        kWqmB32 = 6,
        kWqmB64 = 7,
        kBrevB32 = 8,
        kBrevB64 = 9,
        kBcnt0I32B32 = 10,
        kBcnt0I32B64 = 11,
        kBcnt1I32B32 = 12,
        kBcnt1I32B64 = 13,
        kFf0I32B32 = 14,
        kFf0I32B64 = 15,
        kFf1I32B32 = 16,
        kFf1I32B64 = 17,
        kFlbitI32B32 = 18,
        kFlbitI32B64 = 19,
        kFlbitI32 = 20,
        kFlbitI32I64 = 21,
        kSextI32I8 = 22,
        kSextI32I16 = 23,
        kBitset0B32 = 24,
        kBitset0B64 = 25,
        kBitset1B32 = 26,
        kBitset1B64 = 27,
        kGetpcB64 = 28,
        kSetpcB64 = 29,
        kSwappcB64 = 30,
        kRfeB64 = 31,
        kAndSaveexecB64 = 32,
        kOrSaveexecB64 = 33,
        kXorSaveexecB64 = 34,
        kAndn2SaveexecB64 = 35,
        kOrn2SaveexecB64 = 36,
        kNandSaveexecB64 = 37,
        kNorSaveexecB64 = 38,
        kXnorSaveexecB64 = 39,
        kQuadmaskB32 = 40,
        kQuadmaskB64 = 41,
        kMovrelsB32 = 42,
        kMovrelsB64 = 43,
        kMovreldB32 = 44,
        kMovreldB64 = 45,
        kCbranchJoin = 46,
        kAbsI32 = 48,
        kMovFedB32 = 49,
        kSetGprIdxIdx = 50,
        kAndn1SaveexecB64 = 51,
        kOrn1SaveexecB64 = 52,
        kAndn1_wrexecB64 = 53,
        kAndn2_wrexecB64 = 54,
        kBitreplicateB64B32 = 55,
        kIllegal = 56,
    };

    // Get the OP [15:8]
    OP GetOp() const { return op_; }

    // ctor
    G9SOP1Instruction(SSRC ssrc0, OP op, SDST sdst, unsigned int sridx0, unsigned int sdstRidx, int label_, int iGotoLabel) : SOP1Instruction(ssrc0, sdst, sridx0, sdstRidx, label_, iGotoLabel), op_(op)
    {
        hw_gen_ = GDT_HW_GENERATION_GFX9;
    }
private:
    // SOP1 operation.
    OP op_;
};

#endif //__SOP1INSTRUCTION_H


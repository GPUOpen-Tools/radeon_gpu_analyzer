//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for class representing SOP2 Instruction in shader isa.
//=============================================================================

#ifndef __SOP2INSTRUCTION_H
#define __SOP2INSTRUCTION_H

#include "be_instruction.h"

// This is a scalar instruction with two inputs and one output.
// Can be followed by a 32-bit literal constant.
// Opcode :
//        SSRC0 [7:0]
//        SSRC1 [15:8]
//        SDST [22:16]
//        OP [29:23]

class SOP2Instruction : public Instruction
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

    // Source 1. Second operand for instruction.
    SSRC ssrc1_;

    // Registers index (ssrc0_).
    // Note : Relevant only if ssrc0_ == ScalarGPR or ssrc0_ == ScalarTtmp
    unsigned int sridx0_;

    // Registers index (ssrc1_).
    // Note : Relevant only if ssrc1_ == ScalarGPR or ssrc1_ == ScalarTtmp
    unsigned int sridx1_;

    // Scalar destination for instruction..
    SDST sdst_;

    // Registers index (dst_).
    // Note : Relevant only if dst_ == ScalarGPR or dst_ == ScalarTtmp
    unsigned int sdst_ridx_;

    // true if instruction is followed by 32-bit literal
    bool is_literal_32bit_;

    // 32-bit literal
    uint32_t literal_32bit_;

    // SOPP Instruction Width in bits
    static const unsigned int sopc_instruction_width = 32;
public:

    SOP2Instruction(SSRC ssrc0, SSRC ssrc1, SDST sdst, unsigned int sridx0, unsigned int sridx1, unsigned int sdst_ridx, bool is_literal_32bit = false,
        uint32_t literal_32bit = 0, int label = kNoLabel, int goto_label = kNoLabel) : Instruction(sopc_instruction_width, InstructionCategory::kScalarAlu, InstructionSet::kInstructionSetSop2, label, goto_label),
        ssrc0_(ssrc0), ssrc1_(ssrc1), sridx0_(sridx0), sridx1_(sridx1), sdst_(sdst), sdst_ridx_(sdst_ridx), is_literal_32bit_(is_literal_32bit), literal_32bit_(literal_32bit) {}

    ~SOP2Instruction() = default;

    // Get the SSRC0 [7:0].
    SSRC GetSsrc0() const { return ssrc0_; }

    // Get the SSRC1 [15:8].
    SSRC GetSrc1() const { return ssrc0_; }

    // Get the SDST [22:16].
    SDST GetSdst() const { return sdst_; }

    // Get the (scalar) register`s index.
    // Note : Relevant only if ssrc0_ == ScalarGPR or ssrc0_ == ScalarTtmp
    unsigned int GetSRidx0() const { return sridx0_; }

    // Get the (scalar) register`s index.
    // Note : Relevant only if ssrc1_ == ScalarGPR or ssrc1_ == ScalarTtmp
    unsigned int GetSRidx1() const { return sridx1_; }

    // Get the (scalar) register`s index.
    // Note : Relevant only if dst_ == ScalarGPR or dst_ == ScalarTtmp
    unsigned int GetSdstRidx() const { return sdst_ridx_; }
};

class SISOP2Instruction : public SOP2Instruction
{
public:
    // Selector for the SOP2 Instruction
    enum OP
    {
        //  D.u = S0.u + S1.u. SCC = carry out.
        kAddU32,
        //  D.u = S0.u - S1.u. SCC = carry out.
        kSUBU32,
        //  D.u = S0.i + S1.i. SCC = overflow.
        kAddI32,
        //  D.u = S0.i - S1.i. SCC = overflow.
        kSUBI32,
        //  D.u = S0.u + S1.u + SCC. SCC = carry-out.
        kAddcU32,
        //  D.u = S0.u - S1.u - SCC. SCC = carry-out.
        kSubbU32,
        //  D.i = (S0.i < S1.i) ? S0.i : S1.i. SCC = 1 if S0 is min.
        kMinI32,
        //  D.u = (S0.u < S1.u) ? S0.u : S1.u. SCC = 1 if S0 is min.
        kMinU32,
        //  D.i = (S0.i > S1.i) ? S0.i : S1.i. SCC = 1 if S0 is max.
        kMaxI32,
        //  D.u = (S0.u > S1.u) ? S0.u : S1.u. SCC = 1 if S0 is max.
        kMaxU32,
        //  D.u = SCC ? S0.u : S1.u.
        kCselectB32,
        //  D.u = SCC ? S0.u : S1.u.
        kCselectB64 = 11,
        //  D.u = S0.u & S1.u. SCC = 1 if result is non-zero.
        kAndB32 = 14,
        //  D.u = S0.u & S1.u. SCC = 1 if result is non-zero.
        kAndB64,
        //  D.u = S0.u | S1.u. SCC = 1 if result is non-zero.
        kOrB32,
        //  D.u = S0.u | S1.u. SCC = 1 if result is non-zero.
        kOrB64,
        //  D.u = S0.u ^ S1.u. SCC = 1 if result is non-zero.
        kXorB32,
        //  D.u = S0.u ^ S1.u. SCC = 1 if result is non-zero.
        kXorB64,
        //  D.u = S0.u & ~S1.u. SCC = 1 if result is non-zero.
        kAndn2B32,
        //  D.u = S0.u & ~S1.u. SCC = 1 if result is non-zero.
        kAndn2B64,
        //  D.u = S0.u | ~S1.u. SCC = 1 if result is non-zero.
        kOrn2B32,
        //  D.u = S0.u | ~S1.u. SCC = 1 if result is non-zero.
        kOrn2B64,
        //  D.u = ~(S0.u & S1.u). SCC = 1 if result is non-zero.
        kNandB32,
        //  D.u = ~(S0.u & S1.u). SCC = 1 if result is non-zero.
        kNandB64,
        //  D.u = ~(S0.u | S1.u). SCC = 1 if result is non-zero.
        kNorB32,
        //  D.u = ~(S0.u | S1.u). SCC = 1 if result is non-zero.
        kNorB64,
        //  D.u = ~(S0.u ^ S1.u). SCC = 1 if result is non-zero.
        kXNorB32,
        //  D.u = ~(S0.u ^ S1.u). SCC = 1 if result is non-zero.
        kXNorB64,
        //  D.u = S0.u << S1.u[4:0]. SCC = 1 if result is non-zero.
        kLshlB32,
        //  D.u = S0.u << S1.u[5:0]. SCC = 1 if result is non-zero.
        kLshlB64,
        //  D.u = S0.u >> S1.u[4:0]. SCC = 1 if result is non-zero.
        kLshrB32,
        //  D.u = S0.u >> S1.u[5:0]. SCC = 1 if result is non-zero.
        kLshrB64,
        //  D.i = signtext(S0.i) >> S1.i[4:0]. SCC = 1 if result is non-zero.
        kAshrI32,
        //  D.i = signtext(S0.i) >> S1.i[5:0]. SCC = 1 if result is non-zero.
        kAshrI64,
        //  D.u = ((1 << S0.u[4:0]) - 1) << S1.u[4:0]; bitfield mask.
        kBfmB32,
        //  D.u = ((1 << S0.u[5:0]) - 1) << S1.u[5:0]; bitfield mask.
        kBfmB64,
        //  D.i = S0.i * S1.i.
        kMulI32,
        //  Bit field extract. S0 is data, S1[4:0] is field offset, S1[22:16] is field
        // width. D.u = (S0.u >> S1.u[4:0]) & ((1 << S1.u[22:16]) - 1). SCC = 1 if result
        // is non-zero.
        kBfeU32,
        //  Bit field extract. S0 is data, S1[4:0] is field offset, S1[22:16] is field
        // width. D.i = (S0.u >> S1.u[4:0]) & ((1 << S1.u[22:16]) - 1). SCC = 1 if result is
        // non-zero. Test sign-extended result.
        kBfeI32,
        //  Bit field extract. S0 is data, S1[4:0] is field offset, S1[22:16] is field
        // width. D.u = (S0.u >> S1.u[5:0]) & ((1 << S1.u[22:16]) - 1). SCC = 1 if result
        // is non-zero.
        kBfeU64,
        //  Bit field extract. S0 is data, S1[5:0] is field offset, S1[22:16] is field
        // width. D.i = (S0.u >> S1.u[5:0]) & ((1 << S1.u[22:16]) - 1). SCC = 1 if result is
        // non-zero. Test sign-extended result.
        kBfeI64,
        //  Conditional branch using branch stack. Arg0 = compare
        // mask (VCC or any SGPR), Arg1 = 64-bit byte address of target instruction.
        kCbranchGFork,
        //  D.i = abs(S0.i >> S1.i). SCC = 1 if result is non-zero.
        // All other values are reserved.
        kAbsdiffI32,
        // Reserved
        kReserved
    };

    // Get the OP [29:23].
    OP GetOp() const { return op_; }

    // ctor
    SISOP2Instruction(SSRC ssrc0, SSRC ssrc1, SDST sdst, OP op, unsigned int sridx0,
        unsigned int sridx1, unsigned int sdst_ridx, bool is_literal_32bit = false, uint32_t literal_32bit = 0,
        int label = kNoLabel, int goto_label = kNoLabel) : SOP2Instruction(ssrc0, ssrc1, sdst, sridx0, sridx1,
            sdst_ridx, is_literal_32bit, literal_32bit, label, goto_label), op_(op) {}

private:
    // SOPC operation.
    OP op_;
};

class VISOP2Instruction : public SOP2Instruction
{
public:
    // Selector for the SOP2 Instruction
    enum OP
    {
        kAddU32 = 0,
        kSubU32 = 1,
        kAddI32 = 2,
        kSubI32 = 3,
        kAddcU32 = 4,
        kSubbU32 = 5,
        kMinI32 = 6,
        kMinU32 = 7,
        kMaxI32 = 8,
        kMaxU32 = 9,
        kCselectB32 = 10,
        kCselectB64 = 11,
        kAndB32 = 12,
        kAndB64 = 13,
        kOrB32 = 14,
        kOrB64 = 15,
        kXorB32 = 16,
        kXorB64 = 17,
        kAndn2B32 = 18,
        kAndn2B64 = 19,
        kOrn2B32 = 20,
        kOrn2B64 = 21,
        kNandB32 = 22,
        kNandB64 = 23,
        kNorB32 = 24,
        kNorB64 = 25,
        kXnorB32 = 26,
        kXnorB64 = 27,
        kLshlB32 = 28,
        kLshlB64 = 29,
        kLshrB32 = 30,
        kLshrB64 = 31,
        kAshrI32 = 32,
        kAshrI64 = 33,
        kBfmB32 = 34,
        kBfmB64 = 35,
        kMulI32 = 36,
        kBfeU32 = 37,
        kBfeI32 = 38,
        kBfeU64 = 39,
        kBfeI64 = 40,
        kCbranchGFork = 41,
        kAbsdiffI32 = 42,
        kRfeRestoreB64 = 43,
        // Illegal
        S_ILLEGAL = 44,
    };

    // Get the OP [29:23]
    OP GetOp() const { return op_; }

    // ctor
    VISOP2Instruction(SSRC ssrc0, SSRC ssrc1, SDST sdst, OP op, unsigned int sridx0, unsigned int sridx1, unsigned int sdstRidx, bool isLiteral32bit = false, uint32_t literal32bit = 0, int label_ = kNoLabel, int iGotoLabel = kNoLabel) : SOP2Instruction(ssrc0, ssrc1, sdst, sridx0, sridx1, sdstRidx, isLiteral32bit, literal32bit, label_, iGotoLabel), op_(op)
    {
        hw_gen_ = GDT_HW_GENERATION_VOLCANICISLAND;
    }


private:
    // SOPC operation.
    OP op_;

};


// SOP2 instructions for VEGA (GFX9)
class G9SOP2Instruction : public SOP2Instruction
{
public:
    // Selector for the SOP2 Instruction
    enum OP
    {
        KAddU32 = 0,
        KSubU32 = 1,
        KAddI32 = 2,
        KSubI32 = 3,
        KAddcU32 = 4,
        KSubbU32 = 5,
        KMinI32 = 6,
        KMinU32 = 7,
        KMaxI32 = 8,
        KMaxU32 = 9,
        KCselectB32 = 10,
        KCselectB64 = 11,
        KAndB32 = 12,
        KAndB64 = 13,
        KOrB32 = 14,
        KOrB64 = 15,
        KXorB32 = 16,
        KXorB64 = 17,
        KAndn2B32 = 18,
        KAndn2B64 = 19,
        KOrn2B32 = 20,
        KOrn2B64 = 21,
        KNandB32 = 22,
        KNandB64 = 23,
        KNorB32 = 24,
        KNorB64 = 25,
        KXnorB32 = 26,
        KXnorB64 = 27,
        KLshlB32 = 28,
        KLshlB64 = 29,
        KLshrB32 = 30,
        KLshrB64 = 31,
        KAshrI32 = 32,
        KAshrI64 = 33,
        KBfmB32 = 34,
        KBfmB64 = 35,
        KMulI32 = 36,
        KBfeU32 = 37,
        KBfeI32 = 38,
        KBfeU64 = 39,
        KBfeI64 = 40,
        KCbranchGFork = 41,
        KAbsdiffI32 = 42,
        KRfeRestoreB64 = 43,
        KMulHiU32 = 44,
        KMulHiI32 = 45,
        KLshl1AddU32 = 46,
        KLshl2AddU32 = 47,
        KLshl3AddU32 = 48,
        KLshl4AddU32 = 49,
        KPackLlB32B16 = 50,
        KPackLhB32B16 = 51,
        KPackHhB32B16 = 52,
        // Illegal
        kIllegal = 53,
    };

    // Get the OP [29:23]
    OP GetOp() const { return op_; }

    // ctor
    G9SOP2Instruction(SSRC ssrc0, SSRC ssrc1, SDST sdst, OP op, unsigned int sridx0, unsigned int sridx1,
        unsigned int sdst_ridx, bool is_literal_32bit = false, uint32_t literal_32bit = 0, int label = kNoLabel,
        int goto_label = kNoLabel) : SOP2Instruction(ssrc0, ssrc1, sdst, sridx0, sridx1, sdst_ridx, is_literal_32bit, literal_32bit, label, goto_label), op_(op)
    {
        hw_gen_ = GDT_HW_GENERATION_GFX9;
    }

private:
    // SOPC operation.
    OP op_;
};

#endif //__SOP2INSTRUCTION_H


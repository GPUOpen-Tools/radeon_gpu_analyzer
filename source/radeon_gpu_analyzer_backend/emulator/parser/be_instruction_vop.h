//=============================================================================
/// Copyright (c) 2013-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for class representing Vector ALU general Instruction in shader isa.
//=============================================================================

#ifndef __VOPINSTRUCTION_H
#define __VOPINSTRUCTION_H

#include "be_instruction.h"

// Vector ALU general Instruction ,which represents the following instruction :
// VOP2,VOP1,VOPC,VOP3 (3 input, one output),VOP3 (3 input, two output).
class VOPInstruction : public Instruction
{
public:
    // Selector for the Instruction Type.
    enum Encoding
    {
        kEncodingVop1 = 0x0000003F, // [31:25] enum(7) Must be 0 1 1 1 1 1 1.
        kEncodingVop2 = 0x00000000, // [31]    enum(1) Must be 0.
        kEncodingVop3 = 0x00000034, // [31:26] enum(6) Must be 1 1 0 1 0 0. (VOP3a)
        kEncodingVopC = 0x0000003E, // [31:25] enum(7) 0 1 1 1 1 1 0. // Single Vector Compare Operations
        kEncodingVop3p = 0x000001A7, // [31:23] Must be  1 1 0 1 0 0 1 1 1.
        kEncodingIllegal
    };

    // VOP Instructions encoding masks.
    enum VopMask
    {
        kMaskVop1 = 0x0000003F << 25, // [31:25] enum(7) Must be 0 1 1 1 1 1 1.
        kMaskVop2 = 0x00000000 << 31, // [31]    enum(1) Must be 0.
        kMaskVop3 = 0x00000034 << 26, // [31:26] enum(6) Must be 1 1 0 1 0 0.
        kMaskVopc = 0x0000003E << 25, // [31:25] enum(7) 0 1 1 1 1 1 0.
        kMaskVop3p = 0x000001A7 << 23,
        kMaskUndefined,
    };

    VOPInstruction(unsigned int instruction_width, InstructionSet instruction_encoding,
        int label, int goto_label) : Instruction(instruction_width,
            InstructionCategory::kVectorAlu, instruction_encoding, label, goto_label) { }

    ~VOPInstruction() = default;
};

class SIVOP1Instruction : public VOPInstruction
{
public:
    enum Vop1Op
    {
        kNOP = 0,  //  do nothing.
        kMovB32 = 1,  //  D.u = S0.u.
        kReadfirstlaneB32 = 2,     //  copy one VGPR value to one SGPR. Dst = SGPRdest, Src0 = Source Data (VGPR# or M0(lds-direct)), Lane# = FindFirst1fromLSB(exec) (lane = 0 if exec is zero). Ignores exec mask.
        kCvtI32F64 = 3,  //  D.i = (int)S0.d.
        kCvtF64I32 = 4,  //  D.f = (float)S0.i.
        kCvtF32I32 = 5,  //  D.f = (float)S0.i.
        kCvtF32U32 = 6,  //  D.f = (float)S0.u.
        kCvtU32F32 = 7,  //  D.u = (unsigned)S0.f.
        kCvtI32F32 = 8,  //  D.i = (int)S0.f.
        kMovFedB32 = 9,  //  D.u = S0.u, introduce edc double error upon write to dest vgpr without causing an exception.
        kCvtF16F32 = 10,    //  D.f16 = flt32_to_flt16(S0.f).
        kCvtF32F16 = 11,    //  D.f = flt16_to_flt32(S0.f16).
        kCvtRpiI32F32 = 12, //  D.i = (int)floor(S0.f + 0.5).
        kCvtFlrI32F32 = 13, //  D.i = (int)floor(S0.f).
        kCvtOffF32I4 = 14, //  4-bit signed int to 32-bit float. For interpolation in shader.
        kCvtF32F64 = 15, //  D.f = (float)S0.d.
        kCvtF64F32 = 16, //  D.d = (double)S0.f.
        kCvtF32Ubyte0 = 17, //  D.f = UINT2FLT(S0.u[7           // 0]).
        kCvtF32Ubyte1 = 18, //  D.f = UINT2FLT(S0.u[15           // 8]).
        kCvtF32Ubyte2 = 19, //  D.f = UINT2FLT(S0.u[23           // 16]).
        kCvtF32Ubyte3 = 20, //  D.f = UINT2FLT(S0.u[31           // 24]).
        kCvtU32F64 = 21, //  D.u = (uint)S0.d.
        kCvtF64U32 = 22, //  D.d = (double)S0.u.
        // 23 - 31 reserved.
        kFractF32 = 32, //  D.f = S0.f - floor(S0.f).
        kTruncF32 = 33, //  D.f = trunc(S0.f), return integer part of S0.
        kCeilF32 = 34, //  D.f = ceil(S0.f). Implemented as           //  D.f = trunc(S0.f); if (S0 > 0.0 && S0 != D), D += 1.0.
        kRndneF32 = 35, //  D.f = round_nearest_even(S0.f).
        kFloorF32 = 36, //  D.f = trunc(S0); if ((S0 < 0.0) && (S0 != D)) D += -1.0.
        kExpF32 = 37, //  D.f = pow(2.0, S0.f).
        kLogClampF32 = 38, //  D.f = log2(S0.f), clamp -infinity to -max_float.
        kLogF32 = 39, //  D.f = log2(S0.f).
        kRcpClampF32 = 40, //  D.f = 1.0 / S0.f, result clamped to +-max_float.
        kRcpLegacyF32 = 41, //  D.f = 1.0 / S0.f, +-infinity result clamped to +-0.0.
        kRcpF32 = 42, //  D.f = 1.0 / S0.f.
        kRcpIFLAGF32 = 43, //  D.f = 1.0 / S0.f, only integer div_by_zero flag can be raised.
        kRsqClampF32 = 44, //  D.f = 1.0 / sqrt(S0.f), result clamped to +-max_float.
        kRsqLegacyF32 = 45, //  D.f = 1.0 / sqrt(S0.f).
        kRsqF32 = 46, //  D.f = 1.0 / sqrt(S0.f).
        kRcpF64 = 47, //  D.d = 1.0 / (S0.d).
        kRcpClampF64 = 48, //  D.f = 1.0 / (S0.f), result clamped to +-max_float.
        kRsqF64 = 49, //  D.f = 1.0 / sqrt(S0.f).
        kRsqClampF64 = 50, //  D.d = 1.0 / sqrt(S0.d), result clamped to +-max_float.
        kSqrtF32 = 51, //  D.f = sqrt(S0.f).
        kSqrtF64 = 52, //  D.d = sqrt(S0.d).
        kSinF32 = 53, //  D.f = sin(S0.f).
        kCosF32 = 54, //  D.f = cos(S0.f).
        kNotB32 = 55, //  D.u = ~S0.u.
        kBfrevB32 = 56, //  D.u[31           // 0] = S0.u[0           // 31], bitfield reverse.
        kFfbhU32 = 57, //  D.u = position of first 1 in S0 from MSB; D=0xFFFFFFFF if S0==0.
        kFFBLB32 = 58, //  D.u = position of first 1 in S0 from LSB; D=0xFFFFFFFF if S0==0.
        kFfbhI32 = 59, //  D.u = position of first bit different from sign bit in S0 from MSB; D=0xFFFFFFFF if S0==0 or 0xFFFFFFFF.
        kFrExpExpI32F64 = 60, //  See V_FrExpExpI32F32.
        kFrExpMantF64 = 61,   //  See V_FrExpMantF32.
        kFractF64 = 62,   //  S0.d - floor(S0.d).
        kFrExpExpI32F32 = 63,   //  If (S0.f == INF || S0.f == NAN), then D.i = 0; else D.i = TwosComplement(Exponent(S0.f) - 127 + 1). Returns exponent of single precision float input, such that S0.f = significand * (2 ** exponent). See also FrExpMantF32, which returns the significand.
        kFrExpMantF32 = 64,   //  if (S0.f == INF || S0.f == NAN) then D.f = S0.f; else D.f = Mantissa(S0.f). Result range is in (-1.0,-0.5][0.5,1.0) in normal cases. Returns binary significand of single precision float input, such that S0.f = significand * (2 ** exponent). See also FrExpExpI32F32, which returns integer exponent.
        kClrexp = 65,   //  Clear wave's exception state in SIMD(SP).
        kMovereldB32 = 66,   //  VGPR[D.u + M0.u] = VGPR[S0.u].
        kMoverelsB32 = 67,   //  VGPR[D.u] = VGPR[S0.u + M0.u].
        kMoverelsdB32 = 68,   //  VGPR[D.u + M0.u] = VGPR[S0.u + M0.u].
        kUndefined = 999,
    };

    SIVOP1Instruction(unsigned int instruction_width, Encoding instruction_encoding, Vop1Op op,
        int label, int goto_label) : VOPInstruction(instruction_width, InstructionSet_VOP1, label, goto_label), op_(op), instruction_encoding_(instruction_encoding) { }

    ~SIVOP1Instruction() = default;

    // Get the OP
    Vop1Op GetOp() const { return op_; }

    // return the instruction encoding
    Encoding GetInstructionType() const { return instruction_encoding_; }

private:

    // VOP operation.
    Vop1Op op_;

    //Instruction Type
    Encoding instruction_encoding_;

    int GetInstructionClockCount()
    {
        int ret = 4;

        switch (op_)
        {
        case    Vop1Op::kNOP:
        case    Vop1Op::kMovB32:
        case    Vop1Op::kReadfirstlaneB32:
        case    Vop1Op::kCvtF32I32:
        case    Vop1Op::kCvtF32U32:
        case    Vop1Op::kCvtU32F32:
        case    Vop1Op::kCvtI32F32:
        case    Vop1Op::kMovFedB32:
        case    Vop1Op::kCvtF16F32:
        case    Vop1Op::kCvtF32F16:
        case    Vop1Op::kCvtRpiI32F32:
        case    Vop1Op::kCvtFlrI32F32:
        case    Vop1Op::kCvtOffF32I4:
        case    Vop1Op::kCvtF32Ubyte0:
        case    Vop1Op::kCvtF32Ubyte1:
        case    Vop1Op::kCvtF32Ubyte2:
        case    Vop1Op::kCvtF32Ubyte3:
        case    Vop1Op::kFractF32:
        case    Vop1Op::kTruncF32:
        case    Vop1Op::kCeilF32:
        case    Vop1Op::kRndneF32:
        case    Vop1Op::kFloorF32:
        case    Vop1Op::kRcpIFLAGF32:
        case    Vop1Op::kSqrtF32:
        case    Vop1Op::kNotB32:
        case    Vop1Op::kBfrevB32:
        case    Vop1Op::kFfbhU32:
        case    Vop1Op::kFFBLB32:
        case    Vop1Op::kFfbhI32:
        case    Vop1Op::kFrExpExpI32F32:
        case    Vop1Op::kFrExpMantF32:
        case    Vop1Op::kClrexp:
        case    Vop1Op::kMovereldB32:
        case    Vop1Op::kMoverelsB32:
        case    Vop1Op::kMoverelsdB32:
            ret = 4;
            break;

        case    Vop1Op::kCvtI32F64:
        case    Vop1Op::kCvtF64I32:
        case    Vop1Op::kCvtF32F64:
        case    Vop1Op::kCvtF64F32:
        case    Vop1Op::kCvtU32F64:
        case    Vop1Op::kCvtF64U32:
        case    Vop1Op::kFractF64:
            ret = 8;
            break;

        case    Vop1Op::kExpF32:
        case    Vop1Op::kLogClampF32:
        case    Vop1Op::kLogF32:
        case    Vop1Op::kRcpClampF32:
        case    Vop1Op::kRcpLegacyF32:
        case    Vop1Op::kRcpF32:
        case    Vop1Op::kRsqClampF32:
        case    Vop1Op::kRsqLegacyF32:
        case    Vop1Op::kRsqF32:
        case    Vop1Op::kRcpF64:
        case    Vop1Op::kRcpClampF64:
        case    Vop1Op::kRsqF64:
        case    Vop1Op::kRsqClampF64:
        case    Vop1Op::kSqrtF64:
        case    Vop1Op::kSinF32:
        case    Vop1Op::kCosF32:
        case    Vop1Op::kFrExpExpI32F64:
        case    Vop1Op::kFrExpMantF64:
            ret = 16;
            break;
        default:
            break;
        }

        return ret;
    }
};

class SIVOP2Instruction : public VOPInstruction
{
public:
    enum Vop2Op
    {
        kCndmaskB32 = 0,     //  D.u = VCC[i] ? S1.u : S0.u (i = threadID in wave).
        kReadlaneB32 = 1,      //  copy one VGPR value to one SGPR. Dst = SGPR-dest, Src0 = Source Data (VGPR# or M0(lds-direct)), Src1 = Lane Select (SGPRor M0). Ignores exec mask.
        kWritelaneB32 = 2,       //  Write value into one VGPR one one lane. Dst = VGPRdest,       Src0 = Source Data (sgpr, m0, exec or constants), Src1 = Lane Select (SGPR or M0). Ignores exec mask.
        kAddF32 = 3,  //  D.f = S0.f + S1.f.
        kSubF32 = 4,  //  D.f = S0.f - S1.f.
        kSubrevF32 = 5,    //  D.f = S1.f - S0.f.
        kMacLegacyF32 = 6,        //  D.f = S0.F * S1.f + D.f.
        kMulLegacyF32 = 7,        //  D.f = S0.f * S1.f (DX9 rules, 0.0*x = 0.0).
        kMulF32 = 8,  //  D.f = S0.f * S1.f.
        kMulI32I24 = 9,     //  D.i = S0.i[23              // 0] * S1.i[23:0].
        kMulHiI32I24 = 10,       //  D.i = (S0.i[23:0] * S1.i[23:0])>>32.
        kMulU32U24 = 11,    //  D.u = S0.u[23:0] * S1.u[23:0].
        kMulHiU32U24 = 12,       //  D.i = (S0.u[23:0] * S1.u[23:0])>>32.
        kMinLegacyF32 = 13,       //  D.f = min(S0.f, S1.f) (DX9 rules for NaN).
        kMaxLegacyF32 = 14,       //  D.f = max(S0.f, S1.f) (DX9 rules for NaN).
        kMinF32 = 15, //  D.f = min(S0.f, S1.f).
        kMaxF32 = 16, //  D.f = max(S0.f, S1.f).
        kMinI32 = 17, //  D.i = min(S0.i, S1.i).
        kMaxI32 = 18, //  D.i = max(S0.i, S1.i).
        kMinU32 = 19, //  D.u = min(S0.u, S1.u).
        kMaxU32 = 20, //  D.u = max(S0.u, S1.u).
        kLshrB32 = 21, //  D.u = S0.u >> S1.u[4:0].
        kLshrrevB32 = 22,    //  D.u = S1.u >> S0.u[4:0].
        kAshrI32 = 23, //  D.i = S0.i >> S1.i[4:0].
        kAshrrevI32 = 24,    //  D.i = S1.i >> S0.i[4:0].
        kLshlB32 = 25, //  D.u = S0.u << S1.u[4:0].
        kLshlrevB32 = 26,   //  D.u = S1.u << S0.u[4:0].
        kAndB32 = 27, //  D.u = S0.u & S1.u.
        kOrB32 = 28, //  D.u = S0.u | S1.u.
        kXorB32 = 29, //  D.u = S0.u ^ S1.u.
        kBfmB32 = 30, //  D.u = ((1<<S0.u[4:0])-1) << S1.u[4:0]; S0=bitfield_width, S1=bitfield_offset.
        kMacF32 = 31, //  D.f = S0.f * S1.f + D.f.
        kMadmkF32 = 32, //  D.f = S0.f * K + S1.f; K is a 32-bit inline constant.
        kMadakF32 = 33,  //  D.f = S0.f * S1.f + K; K is a 32-bit inline constant.
        kBcntU32B32 = 34,     //  D.u = CountOneBits(S0.u) + S1.u. Bit count.
        kMBcntLOU32B32 = 35,         //  ThreadMask = (1 << ThreadPosition) - 1; D.u = CountOneBits(S0.u & ThreadMask[31:0]) + S1.u. Masked bit count, ThreadPosition is the position of this thread in the wavefront (in 0 63).
        kMBcntHiU32B32 = 36,         //  ThreadMask = (1 << ThreadPosition) - 1; D.u = CountOneBits(S0.u & ThreadMask[63:32]) + S1.u. Masked bit count, ThreadPosition is the position of this thread in the wavefront (in 0..63).
        kAddI32 = 37, //  D.u = S0.u + S1.u; VCC=carry-out (VOP3:sgpr=carry-out).
        kSubI32 = 38, //  D.u = S0.u - S1.u; VCC=carry-out (VOP3:sgpr=carry-out).
        kSubrevI32 = 39,   //  D.u = S1.u - S0.u; VCC=carry-out (VOP3:sgpr=carry-out).
        kAddcU32 = 40, //  D.u = S0.u + S1.u + VCC; VCC=carry-out (VOP3:sgpr=carryout, S2.u=carry-in).
        kSubbu32 = 41, //  D.u = S0.u - S1.u - VCC; VCC=carry-out (VOP3:sgpr=carry-out, S2.u=carry-in).
        kSubbrevU32 = 42,    //  D.u = S1.u - S0.u - VCC; VCC=carry-out (VOP3:sgpr=carryout, S2.u=carry-in).
        kLdexpF32 = 43,  //  D.d = pow(S0.f, S1.i).
        kCvtPkaccumU8F32 = 44, // f32->u8(s0.f), pack into byte(s1.u), of dst.
        kCvtPknormI16F32 = 45,           //  D = {(snorm)S1.f, (snorm)S0.f}.
        kCvtPknormU16F32 = 46,           //  D = {(unorm)S1.f, (unorm)S0.f}.
        kCvtPkrtzF16F32 = 47,          //  D = {flt32_to_flt16(S1.f),flt32_to_flt16(S0.f)}, with round-toward-zero.
        kCvtPkU16U32 = 48,       //  D = {(u32->u16)S1.u, (u32->u16)S0.u}.
        kCvtPkI16I32 = 49,       //  D = {(i32->i16)S1.i, (i32->i16)S0.i}.
        kUndefined = 999,
    };

    SIVOP2Instruction(unsigned int instruction_width, Encoding instruction_encoding,
        Vop2Op op, int label, int goto_label) : VOPInstruction(instruction_width, InstructionSet_VOP2, label, goto_label), op_(op), instruction_encoding_(instruction_encoding) { }

    ~SIVOP2Instruction() = default;

    Vop2Op GetOp() const { return op_; }

    // Return the instruction encoding.
    Encoding GetInstructionType() const { return instruction_encoding_; }

    int GetInstructionClockCount()
    {
        int ret = 4;
        return ret;
    }

private:

    // VOP operation.
    Vop2Op op_;

    // Instruction Type.
    Encoding instruction_encoding_;
};

class SIVOP3Instruction : public VOPInstruction
{
public:

    enum Vop3Op
    {
        kV3CmpF32_0 = 0,  //  Signal on sNaN input only.
        kV3CmpF32_1 = 1,
        kV3CmpF32_2 = 2,
        kV3CmpF32_3 = 3,
        kV3CmpF32_4 = 4,
        kV3CmpF32_5 = 5,
        kV3CmpF32_6 = 6,
        kV3CmpF32_7 = 7,
        kV3CmpF32_8 = 8,
        kV3CmpF32_9 = 9,
        kV3CmpF32_10 = 10,
        kV3CmpF32_11 = 11,
        kV3CmpF32_12 = 12,
        kV3CmpF32_13 = 13,
        kV3CmpF32_14 = 14,
        kV3CmpF32_15 = 15,

        kV3CmpxF32_16 = 16,  // Signal on sNaN input only. Also write EXEC.
        kV3CmpxF32_17 = 17,
        kV3CmpxF32_18 = 18,
        kV3CmpxF32_19 = 19,
        kV3CmpxF32_20 = 20,
        kV3CmpxF32_21 = 21,
        kV3CmpxF32_22 = 22,
        kV3CmpxF32_23 = 23,
        kV3CmpxF32_24 = 24,
        kV3CmpxF32_25 = 25,
        kV3CmpxF32_26 = 26,
        kV3CmpxF32_27 = 27,
        kV3CmpxF32_28 = 28,
        kV3CmpxF32_29 = 29,
        kV3CmpxF32_30 = 30,
        kV3CmpxF32_31 = 31,

        kV3CmpF64_32 = 32, //  Signal on sNaN input only.
        kV3CmpF64_33 = 33,
        kV3CmpF64_34 = 34,
        kV3CmpF64_35 = 35,
        kV3CmpF64_36 = 36,
        kV3CmpF64_37 = 37,
        kV3CmpF64_38 = 38,
        kV3CmpF64_39 = 39,
        kV3CmpF64_40 = 40,
        kV3CmpF64_41 = 41,
        kV3CmpF64_42 = 42,
        kV3CmpF64_43 = 43,
        kV3CmpF64_44 = 44,
        kV3CmpF64_45 = 45,
        kV3CmpF64_46 = 46,
        kV3CmpF64_47 = 47,

        kV3CmpxF64_48 = 48, //  Signal on sNaN input only. Also write EXEC.
        kV3CmpxF64_49 = 49,
        kV3CmpxF64_50 = 50,
        kV3CmpxF64_51 = 51,
        kV3CmpxF64_52 = 52,
        kV3CmpxF64_53 = 53,
        kV3CmpxF64_54 = 54,
        kV3CmpxF64_55 = 55,
        kV3CmpxF64_56 = 56,
        kV3CmpxF64_57 = 57,
        kV3CmpxF64_58 = 58,
        kV3CmpxF64_59 = 59,
        kV3CmpxF64_60 = 60,
        kV3CmpxF64_61 = 61,
        kV3CmpxF64_62 = 62,
        kV3CmpxF64_63 = 63,

        kV3CmpsF32_64 = 64,  //  Signal on any NaN.
        kV3CmpsF32_65 = 65,
        kV3CmpsF32_66 = 66,
        kV3CmpsF32_67 = 67,
        kV3CmpsF32_68 = 68,
        kV3CmpsF32_69 = 69,
        kV3CmpsF32_70 = 70,
        kV3CmpsF32_71 = 71,
        kV3CmpsF32_72 = 72,
        kV3CmpsF32_73 = 73,
        kV3CmpsF32_74 = 74,
        kV3CmpsF32_75 = 75,
        kV3CmpsF32_76 = 76,
        kV3CmpsF32_77 = 77,
        kV3CmpsF32_78 = 78,
        kV3CmpsF32_79 = 79,

        kV3CmpsxF32_80 = 80,  //  Signal on any NaN. Also write EXEC.
        kV3CmpsxF32_81 = 81,
        kV3CmpsxF32_82 = 82,
        kV3CmpsxF32_83 = 83,
        kV3CmpsxF32_84 = 84,
        kV3CmpsxF32_85 = 85,
        kV3CmpsxF32_86 = 86,
        kV3CmpsxF32_87 = 87,
        kV3CmpsxF32_88 = 88,
        kV3CmpsxF32_89 = 89,
        kV3CmpsxF32_90 = 90,
        kV3CmpsxF32_91 = 91,
        kV3CmpsxF32_92 = 92,
        kV3CmpsxF32_93 = 93,
        kV3CmpsxF32_94 = 94,
        kV3CmpsxF32_95 = 95,

        kV3CmpsF64_96 = 96,  //  Signal on any NaN.
        kV3CmpsF64_97 = 97,
        kV3CmpsF64_98 = 98,
        kV3CmpsF64_99 = 99,
        kV3CmpsF64_100 = 100,
        kV3CmpsF64_101 = 101,
        kV3CmpsF64_102 = 102,
        kV3CmpsF64_103 = 103,
        kV3CmpsF64_104 = 104,
        kV3CmpsF64_105 = 105,
        kV3CmpsF64_106 = 106,
        kV3CmpsF64_107 = 107,
        kV3CmpsF64_108 = 108,
        kV3CmpsF64_109 = 109,
        kV3CmpsF64_110 = 110,
        kV3CmpsF64_111 = 111,

        kV3CmpsxF64_112 = 112,    //  Signal on any NaN. Also write EXEC.
        kV3CmpsxF64_113 = 113,
        kV3CmpsxF64_114 = 114,
        kV3CmpsxF64_115 = 115,
        kV3CmpsxF64_116 = 116,
        kV3CmpsxF64_117 = 117,
        kV3CmpsxF64_118 = 118,
        kV3CmpsxF64_119 = 119,
        kV3CmpsxF64_120 = 120,
        kV3CmpsxF64_121 = 121,
        kV3CmpsxF64_122 = 122,
        kV3CmpsxF64_123 = 123,
        kV3CmpsxF64_124 = 124,
        kV3CmpsxF64_125 = 125,
        kV3CmpsxF64_126 = 126,
        kV3CmpsxF64_127 = 127,

        kV3CmpI32_128 = 128,  //  On 32-bit integers.
        kV3CmpI32_129 = 129,
        kV3CmpI32_130 = 130,
        kV3CmpI32_131 = 131,
        kV3CmpI32_132 = 132,
        kV3CmpI32_133 = 133,
        kV3CmpI32_134 = 134,
        kV3CmpI32_135 = 135,

        kV3CmpClassF32 = 136, // D = IEEE numeric class function specified in S1.u, performed on S0.f.

        kV3CmpxI32_144 = 144,    //  Also write EXEC.
        kV3CmpxI32_145 = 145,
        kV3CmpxI32_146 = 146,
        kV3CmpxI32_147 = 147,
        kV3CmpxI32_148 = 148,
        kV3CmpxI32_149 = 149,
        kV3CmpxI32_150 = 150,
        kV3CmpxI32_151 = 151,

        kV3CmpxClassF32 = 152,    //  D = IEEE numeric class function specified in S1.u, performed on S0.f. Also write EXEC.

        kV3CmpI64_160 = 160,  //  On 64-bit integers.
        kV3CmpI64_161 = 161,
        kV3CmpI64_162 = 162,
        kV3CmpI64_163 = 163,
        kV3CmpI64_164 = 164,
        kV3CmpI64_165 = 165,
        kV3CmpI64_166 = 166,
        kV3CmpI64_167 = 167,

        kV3CmpClassF64 = 168,    //  D = IEEE numeric class function specified in S1.u, performed on S0.d.

        kV3CmpxI64_176 = 176,  //  Also write EXEC.
        kV3CmpxI64_177 = 177,
        kV3CmpxI64_178 = 178,
        kV3CmpxI64_179 = 179,
        kV3CmpxI64_180 = 180,
        kV3CmpxI64_181 = 181,
        kV3CmpxI64_182 = 182,
        kV3CmpxI64_183 = 183,

        kV3CmpxClassF64 = 184,    //  D = IEEE numeric class function specified in S1.u, performed on S0.d. Also write EXEC.

        kV3CmpU32_192 = 192,    //  On unsigned 32-bit intergers.
        kV3CmpU32_193 = 193,
        kV3CmpU32_194 = 194,
        kV3CmpU32_195 = 195,
        kV3CmpU32_196 = 196,
        kV3CmpU32_197 = 197,
        kV3CmpU32_198 = 198,
        kV3CmpU32_199 = 199,

        kV3CmpxU32_208 = 208,  //  Also write EXEC.
        kV3CmpxU32_209 = 209,
        kV3CmpxU32_210 = 210,
        kV3CmpxU32_211 = 211,
        kV3CmpxU32_212 = 212,
        kV3CmpxU32_213 = 213,
        kV3CmpxU32_214 = 214,
        kV3CmpxU32_215 = 215,

        kV3CmpU64_224 = 224,    //  On unsigned 64-bit integers.
        kV3CmpU64_225 = 225,
        kV3CmpU64_226 = 226,
        kV3CmpU64_227 = 227,
        kV3CmpU64_228 = 228,
        kV3CmpU64_229 = 229,
        kV3CmpU64_230 = 230,
        kV3CmpU64_231 = 231,

        kV3CmpxU64_240 = 240,  //  Also write EXEC.
        kV3CmpxU64_241 = 241,
        kV3CmpxU64_242 = 242,
        kV3CmpxU64_243 = 243,
        kV3CmpxU64_244 = 244,
        kV3CmpxU64_245 = 245,
        kV3CmpxU64_246 = 246,
        kV3CmpxU64_247 = 247,

        kV3CndmaskB32 = 256,       //  D.u = S2[i] ? S1.u : S0.u (i = threadID in wave).
        kV3ReadlaneB32 = 257,       //  copy one VGPR value to one SGPR. Dst = SGPR-dest, Src0 = Source Data (VGPR# or M0(lds-direct)), Src1 = Lane Select (SGPR or M0). Ignores exec mask.
        kV3WritelaneB32 = 258,       //  Write value into one VGPR one one lane. Dst = VGPRdest,  Src0 = Source Data (sgpr, m0, exec or constants), Src1 = Lane Select (SGPR or M0). Ignores exec mask.
        kV3AddF32 = 259,      //  D.f = S0.f + S1.f.
        kV3SubF32 = 260,      //  D.f = S0.f - S1.f.
        kV3SubrevF32 = 261,      //  D.f = S1.f - S0.f.
        kV3MacLegacyF32 = 262,       //  D.f = S0.F * S1.f + D.f.
        kV3MulLegacyF32 = 263,       //  D.f = S0.f * S1.f (DX9 rules, 0.0*x = 0.0).
        kV3MulF32 = 264,      //  D.f = S0.f * S1.f.
        kV3MulI32I24 = 265,       //  D.i = S0.i[23           // 0] * S1.i[23:0].
        kV3MulHiI32I24 = 266,       //  D.i = (S0.i[23:0] * S1.i[23:0])>>32.
        kV3MulU32U24 = 267,       //  D.u = S0.u[23:0] * S1.u[23:0].
        kV3MulHiU32U24 = 268,       //  D.i = (S0.u[23:0] * S1.u[23:0])>>32.
        kV3MinLegacyF32 = 269,       //  D.f = min(S0.f, S1.f) (DX9 rules for NaN).
        kV3MaxLegacyF32 = 270,       //  D.f = max(S0.f, S1.f) (DX9 rules for NaN).
        kV3MinF32 = 271,      //  D.f = min(S0.f, S1.f).
        kV3MaxF32 = 272,      //  D.f = max(S0.f, S1.f).
        kV3MinI32 = 273,      //  D.i = min(S0.i, S1.i).

        kV3MaxI32 = 274,     //  D.i = max(S0.i, S1.i).
        kV3MinU32 = 275,     //  D.u = min(S0.u, S1.u).
        kV3MaxU32 = 276,     //  D.u = max(S0.u, S1.u).
        kV3LshrB32 = 277,     //  D.u = S0.u >> S1.u[4:0].
        kV3LshrrevB32 = 278,     //  D.u = S1.u >> S0.u[4:0].
        kV3AshrI32 = 279,     //  D.i = S0.i >> S1.i[4:0].
        kV3AshrrevI32 = 280,     //  D.i = S1.i >> S0.i[4:0].
        kV3LshlB32 = 281,     //  D.u = S0.u << S1.u[4:0].
        kV3LshlREVB32 = 282,     //  D.u = S1.u << S0.u[4:0].
        kV3AndB32 = 283,     //  D.u = S0.u & S1.u.
        kV3ORB32 = 284,    //  D.u = S0.u | S1.u.
        kV3XORB32 = 285,     //  D.u = S0.u ^ S1.u.
        kV3BFMB32 = 286,     //  D.u = ((1<<S0.u[4:0])-1) << S1.u[4:0]; S0=bitfield_width, S1=bitfield_offset.
        kV3MacF32 = 287,     //  D.f = S0.f * S1.f + D.f.
        kV3MadmkF32 = 288,     //  D.f = S0.f * K + S1.f; K is a 32-bit inline constant.
        kV3MadakF32 = 289,     //  D.f = S0.f * S1.f + K; K is a 32-bit inline constant.
        kV3BcntU32B32 = 290,     //  D.u = CountOneBits(S0.u) + S1.u. Bit count.
        kV3MBcntLOU32B32 = 291,    //  ThreadMask = (1 << ThreadPosition) - 1; D.u = CountOneBits(S0.u & ThreadMask[31              // 0]) + S1.u. Masked bit count, ThreadPosition is the position of this thread in the wavefront (in 0..63).
        kV3MBcntHiU32B32 = 292,    //  ThreadMask = (1 << ThreadPosition) - 1; D.u = CountOneBits(S0.u & ThreadMask[63:32]) + S1.u. Masked bit count, ThreadPosition is the position of this thread in the wavefront (in 0..63).

        // 293 � 298 See corresponding opcode numbers in VOP3b (3 in, 2 out).
        kV3AddI32 = 293,         //  D.u = S0.u + S1.u; VCC=carry-out (VOP3:sgpr=carry-out).
        kV3SubI32 = 294,        //  D.u = S0.u - S1.u; VCC=carry-out (VOP3:sgpr=carry-out).
        kV3SubrevI32 = 295,         //  D.u = S1.u - S0.u; VCC=carry-out (VOP3:sgpr=carry-out).
        kV3AddcU32 = 296,         //  D.u = S0.u + S1.u + VCC; VCC=carry-out (VOP3:sgpr=carryout,      S2.u=carry-in).
        kV3SubbU32 = 297,         //  D.u = S0.u - S1.u - VCC; VCC=carry-out (VOP3:sgpr=carry-out, S2.u=carry-in).
        kV3SubbrevU32 = 298,         //  D.u = S1.u - S0.u - VCC; VCC=carry-out (VOP3:sgpr=carryout, S2.u=carry-in).

        kV3LdexpF32 = 299,   //  D.d = pow(S0.f, S1.i).
        kV3CvtPkaccumU8F32 = 300,            //  f32->u8(s0.f), pack into byte(s1.u), of dst.
        kV3CvtPknormI16F32 = 301,            //  D = {(snorm)S1.f, (snorm)S0.f}.
        kV3CvtPknormU16F32 = 302,            //  D = {(unorm)S1.f, (unorm)S0.f}.
        kV3CvtPkrtzF16F32 = 303,           //  D = {flt32_to_flt16(S1.f),flt32_to_flt16(S0.f)}, with round-toward-zero.
        kV3CvtPkU16U32 = 304,        //  D = {(u32->u16)S1.u, (u32->u16)S0.u}.
        kV3CvtPkI16I32 = 305,        //  D = {(i32->i16)S1.i, (i32->i16)S0.i}.
        //318 - 319 Do not use (maps to VOP1 and VOPC).
        //320 - 372 Are VOP3a-only opcodes.
        kV3MadLegacyF32 = 320,         // D.f = S0.f * S1.f + S2.f (DX9 rules, 0.0*x = 0.0).
        kV3MadF32 = 321,         // D.f = S0.f * S1.f + S2.f.
        kV3MadI32I24 = 322,         // D.i = S0.i * S1.i + S2.iD.i = S0.i * S1.i + S2.i.
        kV3MadU32U24 = 323,         // D.u = S0.u * S1.u + S2.u.
        kV3CubeidF32 = 324,         // Rm.w <- Rn,x, Rn,y, Rn.z.
        kV3CubescF32 = 325,         // Rm.y <- Rn,x, Rn,y, Rn.z.
        kV3CubetcF32 = 326,         // Rm.x <- Rn,x, Rn,y, Rn.z.
        kV3CubemaF32 = 327,         // Rm.z <- Rn,x, Rn,y, Rn.z
        kV3BfeU32 = 328,       // D.u = (S0.u>>S1.u[4:0]) & ((1<<S2.u[4:0])-1); bitfield extract, S0=data, S1=field_offset, S2=field_width.
        kV3BfeI32 = 329,         // D.i = (S0.i>>S1.u[4:0]) & ((1<<S2.u[4:0])-1); bitfield extract, S0=data, S1=field_offset, S2=field_width.
        kV3BfiB32 = 330,         // D.u = (S0.u & S1.u) | (~S0.u & S2.u); bitfield insert.
        kV3FmaF32 = 331,         // D.f = S0.f * S1.f + S2.f
        kV3FmaF64 = 332,         // D.d = S0.d * S1.d + S2.d.

        kV3LerpU8 = 333,         // D.u = ((S0.u[31:24] + S1.u[31:24] + S2.u[24]) >> 1) << 24 +
        kV3AlignbitB32 = 334,         // D.u = ({S0,S1} >> S2.u[4:0]) & 0xFFFFFFFF.
        kV3AlignbyteB32 = 335,         // D.u = ({S0,S1} >> (8*S2.u[4:0])) & 0xFFFFFFFF.
        kV3MullitF32 = 336,         // D.f = S0.f * S1.f, replicate result into 4 components (0.0 * x = 0.0; special INF, NaN, overflow rules).
        kV3Min3F32 = 337,         //D.f = min(S0.f, S1.f, S2.f).
        kV3Min3I32 = 338,         //D.i = min(S0.i, S1.i, S2.i).
        kV3Min3U32 = 339,         //0x153 D.u = min(S0.u, S1.u, S2.u).
        kV3Max3F32 = 340,         //D.f = max(S0.f, S1.f, S2.f).
        kV3Max3I32 = 341,         //D.i = max(S0.i, S1.i, S2.i).
        kV3Max3U32 = 342,         //D.u = max(S0.u, S1.u, S2.u).
        kV3Med3F32 = 343,         //D.f = median(S0.f, S1.f, S2.f).
        kV3Med3I32 = 344,         //D.i = median(S0.i, S1.i, S2.i).
        kV3Med3U32 = 345,         // D.u = median(S0.u, S1.u, S2.u).
        kV3SadU8 = 346,         //D.u = Byte SAD with accum_lo(S0.u, S1.u, S2.u).
        kV3SadHiU8 = 347,         //D.u = Byte SAD with accum_hi(S0.u, S1.u, S2.u).
        kV3SadU16 = 348,         //D.u = Word SAD with accum(S0.u, S1.u, S2.u).
        kV3SadU32 = 349,         //D.u = Dword SAD with accum(S0.u, S1.u, S2.u).
        kV3CvtPkU8F32 = 350,         //f32->u8(s0.f), pack into byte(s1.u), of dword(s2).
        kV3DivFixupF32 = 351,         //D.f = Special case divide fixup and flags(s0.f = Quotient, s1.f = Denominator, s2.f = Numerator).
        kV3DivFixupF64 = 352,         //D.d = Special case divide fixup and flags(s0.d = Quotient, s1.d = Denominator, s2.d = Numerator).
        kV3LshlB64 = 353,         //D = S0.u << S1.u[4:0].
        kV3LshrB64 = 354,       //D = S0.u >> S1.u[4:0].
        kV3AshrI64 = 355,         //D = S0.u >> S1.u[4:0].
        kV3AddF64 = 356,         //D.d = S0.d + S1.d.
        kV3MulF64 = 357,         //D.d = S0.d * S1.d.
        kV3MinF64 = 358,         //D.d = min(S0.d, S1.d).
        kV3MaxF64 = 359,         //D.d = max(S0.d, S1.d).
        kV3LdexpF64 = 360,         //D.d = pow(S0.d, S1.i[31:0]).
        kV3MulLOU32 = 361,         //D.u = S0.u * S1.u.
        kV3MulHiU32 = 362,         //D.u = (S0.u * S1.u)>>32.
        kV3MulLOI32 = 363,         //D.i = S0.i * S1.i.
        kV3MulHiI32 = 364,         //D.i = (S0.i * S1.i)>>32.
        //365 - 366 See corresponding opcode numbers in VOP3 (3 in, 2 out), (VOP3b).
        kV3DivSCALEF32 = 365, //D.f = Special case divide preop and flags(s0.f = Quotient, s1.f = Denominator, s2.f = Numerator) s0 must equal s1 or s2.
        kV3DivSCALEF64 = 366, //D.d = Special case divide preop and flags(s0.d = Quotient, s1.d = Denominator, s2.d = Numerator) s0 must equal s1 or s2.

        kV3DivFmasF32 = 367,            //D.f = Special case divide FMA with scale and flags(s0.f = Quotient, s1.f = Denominator, s2.f = Numerator).
        kV3DivFmasF64 = 368,            //D.d = Special case divide FMA with scale and flags(s0.d = Quotient, s1.d = Denominator, s2.d = Numerator).
        kV3MSadU8 = 369,            //D.u = Masked Byte SAD with accum_lo(S0.u, S1.u, S2.u).
        kV3QSadU8 = 370,            //D.u = Quad-Byte SAD with accum_lo/hiu(S0.u[63:0], S1.u[31:0], S2.u[63:0]).
        kV3MQSadU8 = 371,            //D.u = Masked Quad-Byte SAD with accum_lo/hi(S0.u[63:0], S1.u[31:0], S2.u[63:0]).
        kV3TrigPreopF64 = 372,            //D.d = Look Up 2/PI (S0.d) with segment select S1.u[4:0].

        // 384 - 452 Are the VOP1 opcodes when VOP3 encoding is required. For example, V_OP1_OffSET + V_MOVB32 generates the VOP3 version of MOV.
        kV3NOP = 384,   //  do nothing.
        kV3MOVB32 = 385,   //  D.u = S0.u.
        kV3ReadfirstlaneB32 = 386,             //  copy one VGPR value to one SGPR. Dst = SGPRdest, Src0 = Source Data (VGPR# or M0(lds-direct)), Lane# = FindFirst1fromLSB(exec) (lane = 0 if exec is zero). Ignores exec mask.
        kV3CvtI32F64 = 387,       //  D.i = (int)S0.d.
        kV3CvtF64I32 = 388,       //  D.f = (float)S0.i.
        kV3CvtF32I32 = 389,       //  D.f = (float)S0.i.
        kV3CvtF32U32 = 390,       //  D.f = (float)S0.u.
        kV3CvtU32F32 = 391,       //  D.u = (unsigned)S0.f.
        kV3CvtI32F32 = 392,       //  D.i = (int)S0.f.
        kV3MOVFedB32 = 393,       //  D.u = S0.u, introduce edc double error upon write to dest vgpr without causing an exception.
        kV3CvtF16F32 = 394,       //  D.f16 = flt32_to_flt16(S0.f).
        kV3CvtF32_F16 = 395,       //  D.f = flt16_to_flt32(S0.f16).
        kV3CvtRpiI32F32 = 396,           //  D.i = (int)floor(S0.f + 0.5).
        kV3CvtFlrI32F32 = 397,           //  D.i = (int)floor(S0.f).
        kV3CvtOffF32I4 = 398,          //  4-bit signed int to 32-bit float. For interpolation in shader.
        kV3CvtF32F64 = 399,       //  D.f = (float)S0.d.
        kV3CvtF64F32 = 400,       //  D.d = (double)S0.f.
        kV3CvtF32Ubyte0 = 401,          //  D.f = UINT2FLT(S0.u[7              // 0]).
        kV3CvtF32Ubyte1 = 402,          //  D.f = UINT2FLT(S0.u[15:8]).
        kV3CvtF32Ubyte2 = 403,          //  D.f = UINT2FLT(S0.u[23:16]).
        kV3CvtF32Ubyte3 = 404,          //  D.f = UINT2FLT(S0.u[31:24]).
        kV3CvtU32F64 = 405,      //  D.u = (uint)S0.d.
        kV3CvtF64U32 = 406,      //  D.d = (double)S0.u.
        //407 � 415 reserved.
        kV3FractF32 = 416,     //  D.f = S0.f - floor(S0.f).
        kV3TruncF32 = 417,     //  D.f = trunc(S0.f), return integer part of S0.
        kV3CeilF32 = 418,    //  D.f = ceil(S0.f). Implemented as: D.f = trunc(S0.f); if (S0 > 0.0 && S0 != D), D += 1.0.
        kV3RndneF32 = 419,     //  D.f = round_nearest_even(S0.f).
        kV3FLOORF32 = 420,     //  D.f = trunc(S0); if ((S0 < 0.0) && (S0 != D)) D += -1.0.
        kV3ExpF32 = 421,   //  D.f = pow(2.0, S0.f).
        kV3LogClampF32 = 422,         //  D.f = log2(S0.f), clamp -infinity to -max_float.
        kV3LogF32 = 423,   //  D.f = log2(S0.f).
        kV3RcpClampF32 = 424,         //  D.f = 1.0 / S0.f, result clamped to +-max_float.
        kV3RcpLegacyF32 = 425,          //  D.f = 1.0 / S0.f, +-infinity result clamped to +-0.0.
        kV3RcpF32 = 426,   //  D.f = 1.0 / S0.f.
        kV3RcpIFLAGF32 = 427,         //  D.f = 1.0 / S0.f, only integer div_by_zero flag can be           raised.
        kV3RsqClampF32 = 428,         //  D.f = 1.0 / sqrt(S0.f), result clamped to +-max_float.
        kV3RsqLegacyF32 = 429,          //  D.f = 1.0 / sqrt(S0.f).
        kV3RsqF32 = 430,   //  D.f = 1.0 / sqrt(S0.f).
        kV3RcpF64 = 431,   //  D.d = 1.0 / (S0.d).
        kV3RcpClampF64 = 432,         //  D.f = 1.0 / (S0.f), result clamped to +-max_float.
        kV3RsqF64 = 433,   //  D.f = 1.0 / sqrt(S0.f).
        kV3RsqClampF64 = 434,         //  D.d = 1.0 / sqrt(S0.d), result clamped to +-max_float.
        kV3SqrtF32 = 435,    //  D.f = sqrt(S0.f).
        kV3SqrtF64 = 436,    //  D.d = sqrt(S0.d).
        kV3SinF32 = 437,   //  D.f = sin(S0.f).
        kV3CosF32 = 438,   //  D.f = cos(S0.f).
        kV3NotB32 = 439,   //  D.u = ~S0.u.
        kV3BfrevB32 = 440,     //  D.u[31:0] = S0.u[0:31], bitfield reverse.
        kV3FfbhU32 = 441,    //  D.u = position of first 1 in S0 from MSB; D=0xFFFFFFFF if             S0==0.
        kV3FFBLB32 = 442,    //  D.u = position of first 1 in S0 from LSB; D=0xFFFFFFFF if             S0==0.
        kV3FfbhI32 = 443,    //  D.u = position of first bit different from sign bit in S0 from MSB;       D=0xFFFFFFFF if S0==0 or 0xFFFFFFFF.
        kV3FrExpExpI32F64 = 444,             //  See V_FrExpExpI32F32.
        kV3FrExpMantF64 = 445,          //  See V_FrExpMantF32.
        kV3FractF64 = 446,     //  S0.d - floor(S0.d).
        kV3FrExpExpI32F32 = 447,             //  If (S0.f == INF || S0.f == NAN), then D.i = 0; else D.i = TwosComplement(Exponent(S0.f) - 127 + 1). Returns exponent of single precision float input, such that S0.f = significand * (2 ** exponent). See also FrExpMantF32, which returns the significand.
        kV3FrExpMantF32 = 448,          //  if (S0.f == INF || S0.f == NAN) then D.f = S0.f; else D.f = Mantissa(S0.f). Result range is in (-1.0,-0.5][0.5,1.0) in normal cases.
        kV3Clrexp = 449,   //  Clear wave's exception state in shader processor SIMD.
        kV3MovereldB32 = 450,       //  VGPR[D.u + M0.u] = VGPR[S0.u].
        kV3MoverelsB32 = 451,       //  VGPR[D.u] = VGPR[S0.u + M0.u].
        kV3MoverelsdB32 = 452,        //  VGPR[D.u + M0.u] = VGPR[S0.u + M0.u].

        VOP3_UNDEFINE = 999,
    };// end VOP3

    SIVOP3Instruction(unsigned int instructionWidth, Encoding instructionEncoding, Vop3Op op, int label_, int iGotoLabel) : VOPInstruction(instructionWidth, InstructionSet_VOP3, label_, iGotoLabel), op_(op), instruction_encoding_(instructionEncoding) { }

    ~SIVOP3Instruction() {}

    // Get the OP
    Vop3Op GetOp() const { return op_; }

    // return the instruction encoding
    Encoding GetInstructionType() const { return instruction_encoding_; }

private:

    // VOP operation.
    Vop3Op op_;

    //Instruction Type
    Encoding instruction_encoding_;

    int GetInstructionClockCount()
    {
        int iRet = 4;

        switch (op_)
        {
        case    kV3CmpF32_0:
        case    kV3CmpF32_1:
        case    kV3CmpF32_2:
        case    kV3CmpF32_3:
        case    kV3CmpF32_4:
        case    kV3CmpF32_5:
        case    kV3CmpF32_6:
        case    kV3CmpF32_7:
        case    kV3CmpF32_8:
        case    kV3CmpF32_9:
        case    kV3CmpF32_10:
        case    kV3CmpF32_11:
        case    kV3CmpF32_12:
        case    kV3CmpF32_13:
        case    kV3CmpF32_14:
        case    kV3CmpF32_15:
        case    kV3CmpxF32_16:
        case    kV3CmpxF32_17:
        case    kV3CmpxF32_18:
        case    kV3CmpxF32_19:
        case    kV3CmpxF32_20:
        case    kV3CmpxF32_21:
        case    kV3CmpxF32_22:
        case    kV3CmpxF32_23:
        case    kV3CmpxF32_24:
        case    kV3CmpxF32_25:
        case    kV3CmpxF32_26:
        case    kV3CmpxF32_27:
        case    kV3CmpxF32_28:
        case    kV3CmpxF32_29:
        case    kV3CmpxF32_30:
        case    kV3CmpxF32_31:
        case    kV3CmpsF32_64:
        case    kV3CmpsF32_65:
        case    kV3CmpsF32_66:
        case    kV3CmpsF32_67:
        case    kV3CmpsF32_68:
        case    kV3CmpsF32_69:
        case    kV3CmpsF32_70:
        case    kV3CmpsF32_71:
        case    kV3CmpsF32_72:
        case    kV3CmpsF32_73:
        case    kV3CmpsF32_74:
        case    kV3CmpsF32_75:
        case    kV3CmpsF32_76:
        case    kV3CmpsF32_77:
        case    kV3CmpsF32_78:
        case    kV3CmpsF32_79:
        case    kV3CmpsxF32_80:
        case    kV3CmpsxF32_81:
        case    kV3CmpsxF32_82:
        case    kV3CmpsxF32_83:
        case    kV3CmpsxF32_84:
        case    kV3CmpsxF32_85:
        case    kV3CmpsxF32_86:
        case    kV3CmpsxF32_87:
        case    kV3CmpsxF32_88:
        case    kV3CmpsxF32_89:
        case    kV3CmpsxF32_90:
        case    kV3CmpsxF32_91:
        case    kV3CmpsxF32_92:
        case    kV3CmpsxF32_93:
        case    kV3CmpsxF32_94:
        case    kV3CmpsxF32_95:
        case    kV3CmpI32_128:
        case    kV3CmpI32_129:
        case    kV3CmpI32_130:
        case    kV3CmpI32_131:
        case    kV3CmpI32_132:
        case    kV3CmpI32_133:
        case    kV3CmpI32_134:
        case    kV3CmpI32_135:
        case    kV3CmpClassF32:
        case    kV3CmpxI32_144:
        case    kV3CmpxI32_145:
        case    kV3CmpxI32_146:
        case    kV3CmpxI32_147:
        case    kV3CmpxI32_148:
        case    kV3CmpxI32_149:
        case    kV3CmpxI32_150:
        case    kV3CmpxI32_151:
        case    kV3CmpxClassF32:
        case    kV3CmpU32_192:
        case    kV3CmpU32_193:
        case    kV3CmpU32_194:
        case    kV3CmpU32_195:
        case    kV3CmpU32_196:
        case    kV3CmpU32_197:
        case    kV3CmpU32_198:
        case    kV3CmpU32_199:
        case    kV3CmpxU32_208:
        case    kV3CmpxU32_209:
        case    kV3CmpxU32_210:
        case    kV3CmpxU32_211:
        case    kV3CmpxU32_212:
        case    kV3CmpxU32_213:
        case    kV3CmpxU32_214:
        case    kV3CmpxU32_215:
        case    kV3CndmaskB32:
        case    kV3ReadlaneB32:
        case    kV3WritelaneB32:
        case    kV3AddF32:
        case    kV3SubF32:
        case    kV3SubrevF32:
        case    kV3MacLegacyF32:
        case    kV3MulLegacyF32:
        case    kV3MulF32:
        case    kV3MulI32I24:
        case    kV3MulHiI32I24:
        case    kV3MulU32U24:
        case    kV3MulHiU32U24:
        case    kV3MinLegacyF32:
        case    kV3MaxLegacyF32:
        case    kV3MinF32:
        case    kV3MaxF32:
        case    kV3MinI32:
        case    kV3AddI32:
        case    kV3SubI32:
        case    kV3SubrevI32:
        case    kV3AddcU32:
        case    kV3SubbU32:
        case    kV3SubbrevU32:
        case    kV3LdexpF32:
        case    kV3CvtPkaccumU8F32:
        case    kV3CvtPknormI16F32:
        case    kV3CvtPknormU16F32:
        case    kV3CvtPkrtzF16F32:
        case    kV3CvtPkU16U32:
        case    kV3CvtPkI16I32:
        case    kV3MadLegacyF32:
        case    kV3MadF32:
        case    kV3MadI32I24:
        case    kV3MadU32U24:
        case    kV3CubeidF32:
        case    kV3CubescF32:
        case    kV3CubetcF32:
        case    kV3CubemaF32:
        case    kV3BfeU32:
        case    kV3BfeI32:
        case    kV3BfiB32:
        case    kV3FmaF32:
        case    kV3LerpU8:
        case    kV3AlignbitB32:
        case    kV3AlignbyteB32:
        case    kV3MullitF32:
        case    kV3Min3F32:
        case    kV3Min3I32:
        case    kV3Min3U32:
        case    kV3Max3F32:
        case    kV3Max3I32:
        case    kV3Max3U32:
        case    kV3Med3F32:
        case    kV3Med3I32:
        case    kV3Med3U32:
        case    kV3SadU8:
        case    kV3SadHiU8:
        case    kV3SadU16:
        case    kV3SadU32:
        case    kV3CvtPkU8F32:
        case    kV3DivFixupF32:
        case    kV3MulLOU32:
        case    kV3MulHiU32:
        case    kV3MulLOI32:
        case    kV3MulHiI32:
        case    kV3DivSCALEF32:
        case    kV3DivFmasF32:
        case    kV3MSadU8:
        case    kV3QSadU8:
        case    kV3MQSadU8:
        case    kV3NOP:
        case    kV3MOVB32:
        case    kV3ReadfirstlaneB32:
        case    kV3CvtF64I32:
        case    kV3CvtF32I32:
        case    kV3CvtF32U32:
        case    kV3CvtU32F32:
        case    kV3CvtI32F32:
        case    kV3MOVFedB32:
        case    kV3CvtF16F32:
        case    kV3CvtF32_F16:
        case    kV3CvtRpiI32F32:
        case    kV3CvtFlrI32F32:
        case    kV3CvtOffF32I4:
        case    kV3CvtF32Ubyte0:
        case    kV3CvtF32Ubyte1:
        case    kV3CvtF32Ubyte2:
        case    kV3CvtF32Ubyte3:
        case    kV3FractF32:
        case    kV3TruncF32:
        case    kV3CeilF32:
        case    kV3RndneF32:
        case    kV3FLOORF32:
        case    kV3ExpF32:
        case    kV3LogClampF32:
        case    kV3LogF32:
        case    kV3RcpClampF32:
        case    kV3RcpLegacyF32:
        case    kV3RcpF32:
        case    kV3RcpIFLAGF32:
        case    kV3RsqClampF32:
        case    kV3RsqLegacyF32:
        case    kV3RsqF32:
        case    kV3SqrtF32:
        case    kV3SinF32:
        case    kV3CosF32:
        case    kV3NotB32:
        case    kV3BfrevB32:
        case    kV3FfbhU32:
        case    kV3FFBLB32:
        case    kV3FfbhI32:
        case    kV3FrExpExpI32F32:
        case    kV3FrExpMantF32:
        case    kV3Clrexp:
        case    kV3MovereldB32:
        case    kV3MoverelsB32:
        case    kV3MoverelsdB32:
            iRet = 4;
            break;

        case    kV3CmpF64_32:
        case    kV3CmpF64_33:
        case    kV3CmpF64_34:
        case    kV3CmpF64_35:
        case    kV3CmpF64_36:
        case    kV3CmpF64_37:
        case    kV3CmpF64_38:
        case    kV3CmpF64_39:
        case    kV3CmpF64_40:
        case    kV3CmpF64_41:
        case    kV3CmpF64_42:
        case    kV3CmpF64_43:
        case    kV3CmpF64_44:
        case    kV3CmpF64_45:
        case    kV3CmpF64_46:
        case    kV3CmpF64_47:
        case    kV3CmpxF64_48:
        case    kV3CmpxF64_49:
        case    kV3CmpxF64_50:
        case    kV3CmpxF64_51:
        case    kV3CmpxF64_52:
        case    kV3CmpxF64_53:
        case    kV3CmpxF64_54:
        case    kV3CmpxF64_55:
        case    kV3CmpxF64_56:
        case    kV3CmpxF64_57:
        case    kV3CmpxF64_58:
        case    kV3CmpxF64_59:
        case    kV3CmpxF64_60:
        case    kV3CmpxF64_61:
        case    kV3CmpxF64_62:
        case    kV3CmpxF64_63:
        case    kV3CmpsF64_96:
        case    kV3CmpsF64_97:
        case    kV3CmpsF64_98:
        case    kV3CmpsF64_99:
        case    kV3CmpsF64_100:
        case    kV3CmpsF64_101:
        case    kV3CmpsF64_102:
        case    kV3CmpsF64_103:
        case    kV3CmpsF64_104:
        case    kV3CmpsF64_105:
        case    kV3CmpsF64_106:
        case    kV3CmpsF64_107:
        case    kV3CmpsF64_108:
        case    kV3CmpsF64_109:
        case    kV3CmpsF64_110:
        case    kV3CmpsF64_111:
        case    kV3CmpsxF64_112:
        case    kV3CmpsxF64_113:
        case    kV3CmpsxF64_114:
        case    kV3CmpsxF64_115:
        case    kV3CmpsxF64_116:
        case    kV3CmpsxF64_117:
        case    kV3CmpsxF64_118:
        case    kV3CmpsxF64_119:
        case    kV3CmpsxF64_120:
        case    kV3CmpsxF64_121:
        case    kV3CmpsxF64_122:
        case    kV3CmpsxF64_123:
        case    kV3CmpsxF64_124:
        case    kV3CmpsxF64_125:
        case    kV3CmpsxF64_126:
        case    kV3CmpsxF64_127:
        case    kV3CmpI64_160:
        case    kV3CmpI64_161:
        case    kV3CmpI64_162:
        case    kV3CmpI64_163:
        case    kV3CmpI64_164:
        case    kV3CmpI64_165:
        case    kV3CmpI64_166:
        case    kV3CmpI64_167:
        case    kV3CmpClassF64:
        case    kV3CmpxI64_176:
        case    kV3CmpxI64_177:
        case    kV3CmpxI64_178:
        case    kV3CmpxI64_179:
        case    kV3CmpxI64_180:
        case    kV3CmpxI64_181:
        case    kV3CmpxI64_182:
        case    kV3CmpxI64_183:
        case    kV3CmpxClassF64:
        case    kV3CmpU64_224:
        case    kV3CmpU64_225:
        case    kV3CmpU64_226:
        case    kV3CmpU64_227:
        case    kV3CmpU64_228:
        case    kV3CmpU64_229:
        case    kV3CmpU64_230:
        case    kV3CmpU64_231:
        case    kV3CmpxU64_240:
        case    kV3CmpxU64_241:
        case    kV3CmpxU64_242:
        case    kV3CmpxU64_243:
        case    kV3CmpxU64_244:
        case    kV3CmpxU64_245:
        case    kV3CmpxU64_246:
        case    kV3CmpxU64_247:
        case    kV3FmaF64:
        case    kV3DivFixupF64:
        case    kV3LshlB64:
        case    kV3LshrB64:
        case    kV3AshrI64:
        case    kV3AddF64:
        case    kV3MulF64:
        case    kV3MinF64:
        case    kV3MaxF64:
        case    kV3LdexpF64:
        case    kV3DivFmasF64:
        case    kV3TrigPreopF64:
        case    kV3CvtI32F64:
        case    kV3CvtF32F64:
        case    kV3CvtF64F32:
        case    kV3CvtU32F64:
        case    kV3CvtF64U32:
        case    kV3RcpF64:
        case    kV3RcpClampF64:
        case    kV3RsqF64:
        case    kV3RsqClampF64:
        case    kV3SqrtF64:
        case    kV3FrExpExpI32F64:
        case    kV3FrExpMantF64:
        case    kV3FractF64:
            iRet = 8;
            break;

        default:
            break;
        }

        return iRet;
    }
};

class SIVOPCInstruction : public VOPInstruction
{
public:

    enum VopcOp // Single Vector Compare Operations
    {
        kCmpF32_0 = 0,  // Signal on sNaN input only.
        kCmpF32_1 = 1,
        kCmpF32_2 = 2,
        kCmpF32_3 = 3,
        kCmpF32_4 = 4,
        kCmpF32_5 = 5,
        kCmpF32_6 = 6,
        kCmpF32_7 = 7,
        kCmpF32_8 = 8,
        kCmpF32_9 = 9,
        kCmpF32_10 = 10,
        kCmpF32_11 = 11,
        kCmpF32_12 = 12,
        kCmpF32_13 = 13,
        kCmpF32_14 = 14,
        kCmpF32_15 = 15,

        kCmpxF32_16 = 16, // Signal on sNaN input only. Also write EXEC.
        kCmpxF32_17 = 17,
        kCmpxF32_18 = 18,
        kCmpxF32_19 = 19,
        kCmpxF32_20 = 20,
        kCmpxF32_21 = 21,
        kCmpxF32_22 = 22,
        kCmpxF32_23 = 23,
        kCmpxF32_24 = 24,
        kCmpxF32_25 = 25,
        kCmpxF32_26 = 26,
        kCmpxF32_27 = 27,
        kCmpxF32_28 = 28,
        kCmpxF32_29 = 29,
        kCmpxF32_30 = 30,
        kCmpxF32_31 = 31,

        kCmpF64_32 = 32, // Signal on sNaN input only.
        kCmpF64_33 = 33,
        kCmpF64_34 = 34,
        kCmpF64_35 = 35,
        kCmpF64_36 = 36,
        kCmpF64_37 = 37,
        kCmpF64_38 = 38,
        kCmpF64_39 = 39,
        kCmpF64_40 = 40,
        kCmpF64_41 = 41,
        kCmpF64_42 = 42,
        kCmpF64_43 = 43,
        kCmpF64_44 = 44,
        kCmpF64_45 = 45,
        kCmpF64_46 = 46,
        kCmpF64_47 = 47,

        kCmpxF64_48 = 48, // Signal on sNaN input only. Also write EXEC.
        kCmpxF64_49 = 49,
        kCmpxF64_50 = 50,
        kCmpxF64_51 = 51,
        kCmpxF64_52 = 52,
        kCmpxF64_53 = 53,
        kCmpxF64_54 = 54,
        kCmpxF64_55 = 55,
        kCmpxF64_56 = 56,
        kCmpxF64_57 = 57,
        kCmpxF64_58 = 58,
        kCmpxF64_59 = 59,
        kCmpxF64_60 = 60,
        kCmpxF64_61 = 61,
        kCmpxF64_62 = 62,
        kCmpxF64_63 = 63,

        kCmpsF32_64 = 64, // Signal on any NaN.
        kCmpsF32_65 = 65,
        kCmpsF32_66 = 66,
        kCmpsF32_67 = 67,
        kCmpsF32_68 = 68,
        kCmpsF32_69 = 69,
        kCmpsF32_70 = 70,
        kCmpsF32_71 = 71,
        kCmpsF32_72 = 72,
        kCmpsF32_73 = 73,
        kCmpsF32_74 = 74,
        kCmpsF32_75 = 75,
        kCmpsF32_76 = 76,
        kCmpsF32_77 = 77,
        kCmpsF32_78 = 78,
        kCmpsF32_79 = 79,

        kCmpsxF32_80 = 80, //Signal on any NaN. Also write EXEC.
        kCmpsxF32_81 = 81,
        kCmpsxF32_82 = 82,
        kCmpsxF32_83 = 83,
        kCmpsxF32_84 = 84,
        kCmpsxF32_85 = 85,
        kCmpsxF32_86 = 86,
        kCmpsxF32_87 = 87,
        kCmpsxF32_88 = 88,
        kCmpsxF32_89 = 89,
        kCmpsxF32_90 = 90,
        kCmpsxF32_91 = 91,
        kCmpsxF32_92 = 92,
        kCmpsxF32_93 = 93,
        kCmpsxF32_94 = 94,
        kCmpsxF32_95 = 95,

        kCmpsF64_96 = 96, //ignal on any NaN.
        kCmpsF64_97 = 97,
        kCmpsF64_98 = 98,
        kCmpsF64_99 = 99,
        kCmpsF64_100 = 100,
        kCmpsF64_101 = 101,
        kCmpsF64_102 = 102,
        kCmpsF64_103 = 103,
        kCmpsF64_104 = 104,
        kCmpsF64_105 = 105,
        kCmpsF64_106 = 106,
        kCmpsF64_107 = 107,
        kCmpsF64_108 = 108,
        kCmpsF64_109 = 109,
        kCmpsF64_110 = 110,
        kCmpsF64_111 = 111,

        kCmpsxF64_112 = 112, //Signal on any NaN. Also write EXEC.
        kCmpsxF64_113 = 113,
        kCmpsxF64_114 = 114,
        kCmpsxF64_115 = 115,
        kCmpsxF64_116 = 116,
        kCmpsxF64_117 = 117,
        kCmpsxF64_118 = 118,
        kCmpsxF64_119 = 119,
        kCmpsxF64_120 = 120,
        kCmpsxF64_121 = 121,
        kCmpsxF64_122 = 122,
        kCmpsxF64_123 = 123,
        kCmpsxF64_124 = 124,
        kCmpsxF64_125 = 125,
        kCmpsxF64_126 = 126,
        kCmpsxF64_127 = 127,

        kCmpI32_128 = 128,  // On 32-bit integers.
        kCmpI32_129 = 129,
        kCmpI32_130 = 130,
        kCmpI32_131 = 131,
        kCmpI32_132 = 132,
        kCmpI32_133 = 133,
        kCmpI32_134 = 134,
        kCmpI32_135 = 135,
        kCmpI32_136 = 136,
        kCmpI32_137 = 137,
        kCmpI32_138 = 138,
        kCmpI32_139 = 139,
        kCmpI32_140 = 140,
        kCmpI32_141 = 141,
        kCmpI32_142 = 142,
        kCmpI32_143 = 143,

        kCmpxI32_144 = 144, // Also write EXEC.
        kCmpxI32_145 = 145,
        kCmpxI32_146 = 146,
        kCmpxI32_147 = 147,
        kCmpxI32_148 = 148,
        kCmpxI32_149 = 149,
        kCmpxI32_150 = 150,
        kCmpxI32_151 = 151,
        kCmpxI32_152 = 152,
        kCmpxI32_153 = 153,
        kCmpxI32_154 = 154,
        kCmpxI32_155 = 155,
        kCmpxI32_156 = 156,
        kCmpxI32_157 = 157,
        kCmpxI32_158 = 158,
        kCmpxI32_159 = 159,

        kCmpI64_160 = 160, // On 64-bit integers.
        kCmpI64_161 = 161,
        kCmpI64_162 = 162,
        kCmpI64_163 = 163,
        kCmpI64_164 = 164,
        kCmpI64_165 = 165,
        kCmpI64_166 = 166,
        kCmpI64_167 = 167,
        kCmpI64_168 = 168,
        kCmpI64_169 = 169,
        kCmpI64_170 = 170,
        kCmpI64_171 = 171,
        kCmpI64_172 = 172,
        kCmpI64_173 = 173,
        kCmpI64_174 = 174,
        kCmpI64_175 = 175,

        kCmpxI64_176 = 176, // Also write EXEC.
        kCmpxI64_177 = 177,
        kCmpxI64_178 = 178,
        kCmpxI64_179 = 179,
        kCmpxI64_180 = 180,
        kCmpxI64_181 = 181,
        kCmpxI64_182 = 182,
        kCmpxI64_183 = 183,
        kCmpxI64_184 = 184,
        kCmpxI64_185 = 185,
        kCmpxI64_186 = 186,
        kCmpxI64_187 = 187,
        kCmpxI64_188 = 188,
        kCmpxI64_189 = 189,
        kCmpxI64_190 = 190,
        kCmpxI64_191 = 191,

        kCmpU32_192 = 192, // On unsigned 32-bit intergers.
        kCmpU32_193 = 193,
        kCmpU32_194 = 194,
        kCmpU32_195 = 195,
        kCmpU32_196 = 196,
        kCmpU32_197 = 197,
        kCmpU32_198 = 198,
        kCmpU32_199 = 199,
        kCmpU32_200 = 200,
        kCmpU32_201 = 201,
        kCmpU32_202 = 202,
        kCmpU32_203 = 203,
        kCmpU32_204 = 204,
        kCmpU32_205 = 205,
        kCmpU32_206 = 206,
        kCmpU32_207 = 207,

        kCmpxU32_208 = 208, // Also write EXEC.
        kCmpxU32_209 = 209,
        kCmpxU32_210 = 210,
        kCmpxU32_211 = 211,
        kCmpxU32_212 = 212,
        kCmpxU32_213 = 213,
        kCmpxU32_214 = 214,
        kCmpxU32_215 = 215,
        kCmpxU32_216 = 216,
        kCmpxU32_217 = 217,
        kCmpxU32_218 = 218,
        kCmpxU32_219 = 219,
        kCmpxU32_220 = 220,
        kCmpxU32_221 = 221,
        kCmpxU32_222 = 222,
        kCmpxU32_223 = 223,

        kCmpU64_224 = 224, // On unsigned 64-bit integers.
        kCmpU64_225 = 225,
        kCmpU64_226 = 226,
        kCmpU64_227 = 227,
        kCmpU64_228 = 228,
        kCmpU64_229 = 229,
        kCmpU64_230 = 230,
        kCmpU64_231 = 231,
        kCmpU64_232 = 232,
        kCmpU64_233 = 233,
        kCmpU64_234 = 234,
        kCmpU64_235 = 235,
        kCmpU64_236 = 236,
        kCmpU64_237 = 237,
        kCmpU64_238 = 238,
        kCmpU64_239 = 239,

        kCmpxU64_240 = 240, // Also write EXEC.
        kCmpxU64_241 = 241,
        kCmpxU64_242 = 242,
        kCmpxU64_243 = 243,
        kCmpxU64_244 = 244,
        kCmpxU64_245 = 245,
        kCmpxU64_246 = 246,
        kCmpxU64_247 = 247,
        kCmpxU64_248 = 248,
        kCmpxU64_249 = 249,
        kCmpxU64_250 = 250,
        kCmpxU64_251 = 251,
        kCmpxU64_252 = 252,
        kCmpxU64_253 = 253,
        kCmpxU64_254 = 254,
        kCmpxU64_255 = 255,

        kCmpClassF32 = 136, // D = IEEE numeric class function specified in S1.u, performed on S0.f.
        kCmpxClassF32 = 153, // D = IEEE numeric class function specified in S1.u, performed on S0.f. Also write EXEC.
        kCmpClassF64 = 168, // D = IEEE numeric class function specified in S1.u, performed on S0.d.
        kCmpxClassF64 = 184, // D = IEEE numeric class function specified in S1.u, performed on S0.d. Also write EXEC.
        kIllegal = 999,
    };

    SIVOPCInstruction(unsigned int instruction_width, Encoding instruction_encoding, VopcOp op,
        int label, int goto_label) : VOPInstruction(instruction_width, InstructionSet_VOPC, label, goto_label), op_(op), instruction_encoding_(instruction_encoding) { }

    ~SIVOPCInstruction() = default;

    // Get the OP.
    VopcOp GetOp() const { return op_; }

    // Return the instruction encoding.
    Encoding GetInstructionType() const { return instruction_encoding_; }

    int GetInstructionClockCount()
    {
        int ret = 4;

        switch (op_)
        {
        case    VopcOp::kCmpF32_0:
        case    VopcOp::kCmpF32_1:
        case    VopcOp::kCmpF32_2:
        case    VopcOp::kCmpF32_3:
        case    VopcOp::kCmpF32_4:
        case    VopcOp::kCmpF32_5:
        case    VopcOp::kCmpF32_6:
        case    VopcOp::kCmpF32_7:
        case    VopcOp::kCmpF32_8:
        case    VopcOp::kCmpF32_9:
        case    VopcOp::kCmpF32_10:
        case    VopcOp::kCmpF32_11:
        case    VopcOp::kCmpF32_12:
        case    VopcOp::kCmpF32_13:
        case    VopcOp::kCmpF32_14:
        case    VopcOp::kCmpF32_15:
        case    VopcOp::kCmpxF32_16:
        case    VopcOp::kCmpxF32_17:
        case    VopcOp::kCmpxF32_18:
        case    VopcOp::kCmpxF32_19:
        case    VopcOp::kCmpxF32_20:
        case    VopcOp::kCmpxF32_21:
        case    VopcOp::kCmpxF32_22:
        case    VopcOp::kCmpxF32_23:
        case    VopcOp::kCmpxF32_24:
        case    VopcOp::kCmpxF32_25:
        case    VopcOp::kCmpxF32_26:
        case    VopcOp::kCmpxF32_27:
        case    VopcOp::kCmpxF32_28:
        case    VopcOp::kCmpxF32_29:
        case    VopcOp::kCmpxF32_30:
        case    VopcOp::kCmpxF32_31:
        case    VopcOp::kCmpsF32_64:
        case    VopcOp::kCmpsF32_65:
        case    VopcOp::kCmpsF32_66:
        case    VopcOp::kCmpsF32_67:
        case    VopcOp::kCmpsF32_68:
        case    VopcOp::kCmpsF32_69:
        case    VopcOp::kCmpsF32_70:
        case    VopcOp::kCmpsF32_71:
        case    VopcOp::kCmpsF32_72:
        case    VopcOp::kCmpsF32_73:
        case    VopcOp::kCmpsF32_74:
        case    VopcOp::kCmpsF32_75:
        case    VopcOp::kCmpsF32_76:
        case    VopcOp::kCmpsF32_77:
        case    VopcOp::kCmpsF32_78:
        case    VopcOp::kCmpsF32_79:
        case    VopcOp::kCmpsxF32_80:
        case    VopcOp::kCmpsxF32_81:
        case    VopcOp::kCmpsxF32_82:
        case    VopcOp::kCmpsxF32_83:
        case    VopcOp::kCmpsxF32_84:
        case    VopcOp::kCmpsxF32_85:
        case    VopcOp::kCmpsxF32_86:
        case    VopcOp::kCmpsxF32_87:
        case    VopcOp::kCmpsxF32_88:
        case    VopcOp::kCmpsxF32_89:
        case    VopcOp::kCmpsxF32_90:
        case    VopcOp::kCmpsxF32_91:
        case    VopcOp::kCmpsxF32_92:
        case    VopcOp::kCmpsxF32_93:
        case    VopcOp::kCmpsxF32_94:
        case    VopcOp::kCmpsxF32_95:
        case    VopcOp::kCmpI32_128:
        case    VopcOp::kCmpI32_129:
        case    VopcOp::kCmpI32_130:
        case    VopcOp::kCmpI32_131:
        case    VopcOp::kCmpI32_132:
        case    VopcOp::kCmpI32_133:
        case    VopcOp::kCmpI32_134:
        case    VopcOp::kCmpI32_135:
        case    VopcOp::kCmpI32_136:
        case    VopcOp::kCmpI32_137:
        case    VopcOp::kCmpI32_138:
        case    VopcOp::kCmpI32_139:
        case    VopcOp::kCmpI32_140:
        case    VopcOp::kCmpI32_141:
        case    VopcOp::kCmpI32_142:
        case    VopcOp::kCmpI32_143:
        case    VopcOp::kCmpxI32_144:
        case    VopcOp::kCmpxI32_145:
        case    VopcOp::kCmpxI32_146:
        case    VopcOp::kCmpxI32_147:
        case    VopcOp::kCmpxI32_148:
        case    VopcOp::kCmpxI32_149:
        case    VopcOp::kCmpxI32_150:
        case    VopcOp::kCmpxI32_151:
        case    VopcOp::kCmpxI32_152:
        case    VopcOp::kCmpxI32_153:
        case    VopcOp::kCmpxI32_154:
        case    VopcOp::kCmpxI32_155:
        case    VopcOp::kCmpxI32_156:
        case    VopcOp::kCmpxI32_157:
        case    VopcOp::kCmpxI32_158:
        case    VopcOp::kCmpxI32_159:
        case    VopcOp::kCmpU32_192:
        case    VopcOp::kCmpU32_193:
        case    VopcOp::kCmpU32_194:
        case    VopcOp::kCmpU32_195:
        case    VopcOp::kCmpU32_196:
        case    VopcOp::kCmpU32_197:
        case    VopcOp::kCmpU32_198:
        case    VopcOp::kCmpU32_199:
        case    VopcOp::kCmpU32_200:
        case    VopcOp::kCmpU32_201:
        case    VopcOp::kCmpU32_202:
        case    VopcOp::kCmpU32_203:
        case    VopcOp::kCmpU32_204:
        case    VopcOp::kCmpU32_205:
        case    VopcOp::kCmpU32_206:
        case    VopcOp::kCmpU32_207:
        case    VopcOp::kCmpxU32_208:
        case    VopcOp::kCmpxU32_209:
        case    VopcOp::kCmpxU32_210:
        case    VopcOp::kCmpxU32_211:
        case    VopcOp::kCmpxU32_212:
        case    VopcOp::kCmpxU32_213:
        case    VopcOp::kCmpxU32_214:
        case    VopcOp::kCmpxU32_215:
        case    VopcOp::kCmpxU32_216:
        case    VopcOp::kCmpxU32_217:
        case    VopcOp::kCmpxU32_218:
        case    VopcOp::kCmpxU32_219:
        case    VopcOp::kCmpxU32_220:
        case    VopcOp::kCmpxU32_221:
        case    VopcOp::kCmpxU32_222:
        case    VopcOp::kCmpxU32_223:
            ret = 4;
            break;

        case    VopcOp::kCmpF64_32:
        case    VopcOp::kCmpF64_33:
        case    VopcOp::kCmpF64_34:
        case    VopcOp::kCmpF64_35:
        case    VopcOp::kCmpF64_36:
        case    VopcOp::kCmpF64_37:
        case    VopcOp::kCmpF64_38:
        case    VopcOp::kCmpF64_39:
        case    VopcOp::kCmpF64_40:
        case    VopcOp::kCmpF64_41:
        case    VopcOp::kCmpF64_42:
        case    VopcOp::kCmpF64_43:
        case    VopcOp::kCmpF64_44:
        case    VopcOp::kCmpF64_45:
        case    VopcOp::kCmpF64_46:
        case    VopcOp::kCmpF64_47:
        case    VopcOp::kCmpxF64_48:
        case    VopcOp::kCmpxF64_49:
        case    VopcOp::kCmpxF64_50:
        case    VopcOp::kCmpxF64_51:
        case    VopcOp::kCmpxF64_52:
        case    VopcOp::kCmpxF64_53:
        case    VopcOp::kCmpxF64_54:
        case    VopcOp::kCmpxF64_55:
        case    VopcOp::kCmpxF64_56:
        case    VopcOp::kCmpxF64_57:
        case    VopcOp::kCmpxF64_58:
        case    VopcOp::kCmpxF64_59:
        case    VopcOp::kCmpxF64_60:
        case    VopcOp::kCmpxF64_61:
        case    VopcOp::kCmpxF64_62:
        case    VopcOp::kCmpxF64_63:
        case    VopcOp::kCmpsF64_96:
        case    VopcOp::kCmpsF64_97:
        case    VopcOp::kCmpsF64_98:
        case    VopcOp::kCmpsF64_99:
        case    VopcOp::kCmpsF64_100:
        case    VopcOp::kCmpsF64_101:
        case    VopcOp::kCmpsF64_102:
        case    VopcOp::kCmpsF64_103:
        case    VopcOp::kCmpsF64_104:
        case    VopcOp::kCmpsF64_105:
        case    VopcOp::kCmpsF64_106:
        case    VopcOp::kCmpsF64_107:
        case    VopcOp::kCmpsF64_108:
        case    VopcOp::kCmpsF64_109:
        case    VopcOp::kCmpsF64_110:
        case    VopcOp::kCmpsF64_111:
        case    VopcOp::kCmpsxF64_112:
        case    VopcOp::kCmpsxF64_113:
        case    VopcOp::kCmpsxF64_114:
        case    VopcOp::kCmpsxF64_115:
        case    VopcOp::kCmpsxF64_116:
        case    VopcOp::kCmpsxF64_117:
        case    VopcOp::kCmpsxF64_118:
        case    VopcOp::kCmpsxF64_119:
        case    VopcOp::kCmpsxF64_120:
        case    VopcOp::kCmpsxF64_121:
        case    VopcOp::kCmpsxF64_122:
        case    VopcOp::kCmpsxF64_123:
        case    VopcOp::kCmpsxF64_124:
        case    VopcOp::kCmpsxF64_125:
        case    VopcOp::kCmpsxF64_126:
        case    VopcOp::kCmpsxF64_127:
        case    VopcOp::kCmpI64_160:
        case    VopcOp::kCmpI64_161:
        case    VopcOp::kCmpI64_162:
        case    VopcOp::kCmpI64_163:
        case    VopcOp::kCmpI64_164:
        case    VopcOp::kCmpI64_165:
        case    VopcOp::kCmpI64_166:
        case    VopcOp::kCmpI64_167:
        case    VopcOp::kCmpI64_168:
        case    VopcOp::kCmpI64_169:
        case    VopcOp::kCmpI64_170:
        case    VopcOp::kCmpI64_171:
        case    VopcOp::kCmpI64_172:
        case    VopcOp::kCmpI64_173:
        case    VopcOp::kCmpI64_174:
        case    VopcOp::kCmpI64_175:
        case    VopcOp::kCmpxI64_176:
        case    VopcOp::kCmpxI64_177:
        case    VopcOp::kCmpxI64_178:
        case    VopcOp::kCmpxI64_179:
        case    VopcOp::kCmpxI64_180:
        case    VopcOp::kCmpxI64_181:
        case    VopcOp::kCmpxI64_182:
        case    VopcOp::kCmpxI64_183:
        case    VopcOp::kCmpxI64_184:
        case    VopcOp::kCmpxI64_185:
        case    VopcOp::kCmpxI64_186:
        case    VopcOp::kCmpxI64_187:
        case    VopcOp::kCmpxI64_188:
        case    VopcOp::kCmpxI64_189:
        case    VopcOp::kCmpxI64_190:
        case    VopcOp::kCmpxI64_191:
        case    VopcOp::kCmpU64_224:
        case    VopcOp::kCmpU64_225:
        case    VopcOp::kCmpU64_226:
        case    VopcOp::kCmpU64_227:
        case    VopcOp::kCmpU64_228:
        case    VopcOp::kCmpU64_229:
        case    VopcOp::kCmpU64_230:
        case    VopcOp::kCmpU64_231:
        case    VopcOp::kCmpU64_232:
        case    VopcOp::kCmpU64_233:
        case    VopcOp::kCmpU64_234:
        case    VopcOp::kCmpU64_235:
        case    VopcOp::kCmpU64_236:
        case    VopcOp::kCmpU64_237:
        case    VopcOp::kCmpU64_238:
        case    VopcOp::kCmpU64_239:
        case    VopcOp::kCmpxU64_240:
        case    VopcOp::kCmpxU64_241:
        case    VopcOp::kCmpxU64_242:
        case    VopcOp::kCmpxU64_243:
        case    VopcOp::kCmpxU64_244:
        case    VopcOp::kCmpxU64_245:
        case    VopcOp::kCmpxU64_246:
        case    VopcOp::kCmpxU64_247:
        case    VopcOp::kCmpxU64_248:
        case    VopcOp::kCmpxU64_249:
        case    VopcOp::kCmpxU64_250:
        case    VopcOp::kCmpxU64_251:
        case    VopcOp::kCmpxU64_252:
        case    VopcOp::kCmpxU64_253:
        case    VopcOp::kCmpxU64_254:
        case    VopcOp::kCmpxU64_255:
            ret = 8;
            break;

        default:
            break;
        }

        return ret;
    }

private:

    // VOP operation.
    VopcOp op_;

    // Instruction Type.
    Encoding instruction_encoding_;
};

class VIVOP1Instruction : public VOPInstruction
{
public:

    enum Vop1Op
    {
        kNop = 0,
        kMovB32 = 1,
        kReadfirstlaneB32 = 2,
        kCvtI32F64 = 3,
        kCvtF64I32 = 4,
        kCvtF32I32 = 5,
        kCvtF32U32 = 6,
        kCvtU32F32 = 7,
        kCvtI32F32 = 8,
        kMovFedB32 = 9,
        kCvtF16F32 = 10,
        kCvtF32F16 = 11,
        kCvtRpiI32F32 = 12,
        kCvtFlrI32F32 = 13,
        kCvtOffF32I4 = 14,
        kCvtF32F64 = 15,
        kCvtF64F32 = 16,
        kCvtF32Ubyte0 = 17,
        kCvtF32Ubyte1 = 18,
        kCvtF32Ubyte2 = 19,
        kCvtF32Ubyte3 = 20,
        kCvtU32F64 = 21,
        kCvtF64U32 = 22,
        kTruncF64 = 23,
        kCeilF64 = 24,
        kRndneF64 = 25,
        kFloorF64 = 26,
        kFractF32 = 27,
        kTruncF32 = 28,
        kCeilF32 = 29,
        kRndneF32 = 30,
        kFloorF32 = 31,
        kExpF32 = 32,
        kLogF32 = 33,
        kRcpF32 = 34,
        kRcpIflagF32 = 35,
        kRsqF32 = 36,
        kRcpF64 = 37,
        kRsqF64 = 38,
        kSqrtF32 = 39,
        kSqrtF64 = 40,
        kSinF32 = 41,
        kCosF32 = 42,
        kNotB32 = 43,
        kBfrevB32 = 44,
        kFfbhU32 = 45,
        kFfblB32 = 46,
        kFfbhI32 = 47,
        kFrexpExpI32F64 = 48,
        kFrexpMantF64 = 49,
        kFractF64 = 50,
        kFrexpExpI32F32 = 51,
        kFrexpMantF32 = 52,
        kClrexcp = 53,
        kCvtF16U16 = 57,
        kCvtF16I16 = 58,
        kCvtU16F16 = 59,
        kCvtI16F16 = 60,
        kRcpF16 = 61,
        kSqrtF16 = 62,
        kRsqF16 = 63,
        kLogF16 = 64,
        kExpF16 = 65,
        kFrexpMantF16 = 66,
        kFrexpExpI16F16 = 67,
        kFloorF16 = 68,
        kCeilF16 = 69,
        kTruncF16 = 70,
        kRndneF16 = 71,
        kFractF16 = 72,
        kSinF16 = 73,
        kCosF16 = 74,
        kExpLegacyF32 = 75,
        kLogLegacyF32 = 76,
        // Illegal
        kIllegal = 77,
    };

    VIVOP1Instruction(unsigned int instruction_width, Encoding instruction_encoding, Vop1Op op, int label,
        int goto_label) : VOPInstruction(instruction_width, InstructionSet_VOP1, label, goto_label), op_(op), instruction_encoding_(instruction_encoding) { }

    ~VIVOP1Instruction() = default;

    // Get the OP
    Vop1Op GetOp() const { return op_; }

    // Return the instruction encoding.
    Encoding GetInstructionType() const { return instruction_encoding_; }

private:

    // VOP operation.
    Vop1Op op_;

    // Instruction Type.
    Encoding instruction_encoding_;

    int GetInstructionClockCount()
    {
        int ret = 4;
        return ret;
    }
};

class VIVOP2Instruction : public VOPInstruction
{
public:
    enum Vop2Op
    {
        kCndmaskB32 = 0,
        kAddF32 = 1,
        kSubF32 = 2,
        kSubrevF32 = 3,
        kMulLegacyF32 = 4,
        kMulF32 = 5,
        kMulI32I24 = 6,
        kMulHiI32I24 = 7,
        kMulU32U24 = 8,
        kMulHiU32U24 = 9,
        kMinF32 = 10,
        kMaxF32 = 11,
        kMinI32 = 12,
        kMaxI32 = 13,
        kMinU32 = 14,
        kMaxU32 = 15,
        kLshrrevB32 = 16,
        kAshrrevI32 = 17,
        kLshlrevB32 = 18,
        kAndB32 = 19,
        kOrB32 = 20,
        kXorB32 = 21,
        kMacF32 = 22,
        kMadmkF32 = 23,
        kMadakF32 = 24,
        kAddU32 = 25,
        kSubU32 = 26,
        kSubrevU32 = 27,
        kAddcU32 = 28,
        kSubbU32 = 29,
        kSubbrevU32 = 30,
        kAddF16 = 31,
        kSubF16 = 32,
        kSubrevF16 = 33,
        kMulF16 = 34,
        kMacF16 = 35,
        kMadmkF16 = 36,
        kMadakF16 = 37,
        kAddU16 = 38,
        kSubU16 = 39,
        kSubrevU16 = 40,
        kMulLoU16 = 41,
        kLshlrevB16 = 42,
        kLshrrevB16 = 43,
        kAshrrevI16 = 44,
        kMaxF16 = 45,
        kMinF16 = 46,
        kMaxU16 = 47,
        kMaxI16 = 48,
        kMinU16 = 49,
        kMinI16 = 50,
        kLdexpF16 = 51,

        // Illegal.
        kIllegal = 52,
    };

    VIVOP2Instruction(unsigned int instruciton_width, Encoding instruction_encoding, Vop2Op op,
        int label, int goto_label) : VOPInstruction(instruciton_width, InstructionSet_VOP2, label, goto_label), op_(op), instruction_encoding_(instruction_encoding)
    {
        hw_gen_ = GDT_HW_GENERATION_VOLCANICISLAND;
    }

    ~VIVOP2Instruction() = default;

    // Get the OP
    Vop2Op GetOp() const { return op_; }

    // return the instruction encoding
    Encoding GetInstructionType() const { return instruction_encoding_; }

    int GetInstructionClockCount()
    {
        int ret = 4;
        return ret;
    }

private:

    // VOP operation.
    Vop2Op op_;

    // Instruction Type.
    Encoding instruction_encoding_;
};

class VIVOP3Instruction : public VOPInstruction
{
public:

    enum Vop3Op
    {
        kMadLegacyF32 = 448,
        kMadF32 = 449,
        kMadI32I24 = 450,
        kMadU32U24 = 451,
        kCubeidF32 = 452,
        kCubescF32 = 453,
        kCubetcF32 = 454,
        kCubemaF32 = 455,
        kBfeU32 = 456,
        kBfeI32 = 457,
        kBfiB32 = 458,
        kFmaF32 = 459,
        kFmaF64 = 460,
        kLerpU8 = 461,
        kAlignbitB32 = 462,
        kAlignbyteB32 = 463,
        kMin3F32 = 464,
        kMin3I32 = 465,
        kMin3U32 = 466,
        kMax3F32 = 467,
        kMax3I32 = 468,
        kMax3U32 = 469,
        kMed3F32 = 470,
        kMed3I32 = 471,
        kMed3U32 = 472,
        kSadU8 = 473,
        kSadHiU8 = 474,
        kSadU16 = 475,
        kSadU32 = 476,
        kCvtPkU8F32 = 477,
        kDivFixupF32 = 478,
        kDivFixupF64 = 479,
        kDivScaleF32 = 480,
        kDivScaleF64 = 481,
        kDivFmasF32 = 482,
        kDivFmasF64 = 483,
        kMsadU8 = 484,
        kQsadPkU16U8 = 485,
        kMqsadPkU16U8 = 486,
        kMqsadU32U8 = 487,
        kMadU64U32 = 488,
        kMadI64I32 = 489,
        kMadF16 = 490,
        kMadU16 = 491,
        kMadI16 = 492,
        kPermB32 = 493,
        kFmaF16 = 494,
        kDivFixupF16 = 495,
        kCvtPkaccumU8F32 = 496,
        kInterpP1llF16 = 628,
        kInterpP1lvF16 = 629,
        kInterpP2F16 = 630,
        kAddF64 = 640,
        kMulF64 = 641,
        kMinF64 = 642,
        kMaxF64 = 643,
        kLdexpF64 = 644,
        kMulLoU32 = 645,
        kMulHiU32 = 646,
        kMulHiI32 = 647,
        kLdexpF32 = 648,
        kReadlaneB32 = 649,
        kWritelaneB32 = 650,
        kBcntU32B32 = 651,
        kMbcntLoU32B32 = 652,
        kMbcntHiU32B32 = 653,
        kLshlrevB64 = 655,
        kLshrrevB64 = 656,
        kAshrrevI64 = 657,
        kTrigPreopF64 = 658,
        kBfmB32 = 659,
        kCvtPknormI16F32 = 660,
        kCvtPknormU16F32 = 661,
        kCvtPkrtzF16F32 = 662,
        kCvtPkU16U32 = 663,
        kCvtPkI16I32 = 664,

        // Illegal.
        kIllegal = 665,
    };// end VOP3

    VIVOP3Instruction(unsigned int instruciton_width, Encoding instruction_encoding, Vop3Op op, int label,
        int goto_label) : VOPInstruction(instruciton_width, InstructionSet_VOP3, label, goto_label), op_(op), instruction_encoding_(instruction_encoding)
    {
        hw_gen_ = GDT_HW_GENERATION_VOLCANICISLAND;
    }

    ~VIVOP3Instruction() = default;

    Vop3Op GetOp() const { return op_; }

    // Return the instruction encoding.
    Encoding GetInstructionType() const { return instruction_encoding_; }

private:

    // VOP operation.
    Vop3Op op_;

    // Instruction Type.
    Encoding instruction_encoding_;

    int GetInstructionClockCount()
    {
        int ret = 4;
        return ret;
    }
};

class VIVOPCInstruction : public VOPInstruction
{
public:
    // Single Vector Compare Operations.
    enum VopcOp
    {
        kCmpClassF32 = 16,
        kCmpxClassF32 = 17,
        kCmpClassF64 = 18,
        kCmpxClassF64 = 19,
        kCmpClassF16 = 20,
        kCmpxClassF16 = 21,
        kCmpFF16 = 32,
        kCmpLtF16 = 33,
        kCmpEqF16 = 34,
        kCmpLeF16 = 35,
        kCmpGtF16 = 36,
        kCmpLgF16 = 37,
        kCmpGeF16 = 38,
        kCmpOF16 = 39,
        kCmpUF16 = 40,
        kCmpNgeF16 = 41,
        kCmpNlgF16 = 42,
        kCmpNgtF16 = 43,
        kCmpNleF16 = 44,
        kCmpNeqF16 = 45,
        kCmpNltF16 = 46,
        kCmpTruF16 = 47,
        kCmpxFF16 = 48,
        kCmpxLtF16 = 49,
        kCmpxEqF16 = 50,
        kCmpxLeF16 = 51,
        kCmpxGtF16 = 52,
        kCmpxLgF16 = 53,
        kCmpxGeF16 = 54,
        kCmpxOF16 = 55,
        kCmpxUF16 = 56,
        kCmpxNgeF16 = 57,
        kCmpxNlgF16 = 58,
        kCmpxNgtF16 = 59,
        kCmpxNleF16 = 60,
        kCmpxNeqF16 = 61,
        kCmpxNltF16 = 62,
        kCmpxTruF16 = 63,
        kCmpFF32 = 64,
        kCmpLtF32 = 65,
        kCmpEqF32 = 66,
        kCmpLeF32 = 67,
        kCmpGtF32 = 68,
        kCmpLgF32 = 69,
        kCmpGeF32 = 70,
        kCmpOF32 = 71,
        kCmpUF32 = 72,
        kCmpNgeF32 = 73,
        kCmpNlgF32 = 74,
        kCmpNgtF32 = 75,
        kCmpNleF32 = 76,
        kCmpNeqF32 = 77,
        kCmpNltF32 = 78,
        kCmpTruF32 = 79,
        kCmpxFF32 = 80,
        kCmpxLtF32 = 81,
        kCmpxEqF32 = 82,
        kCmpxLeF32 = 83,
        kCmpxGtF32 = 84,
        kCmpxLgF32 = 85,
        kCmpxGeF32 = 86,
        kCmpxOF32 = 87,
        kCmpxUF32 = 88,
        kCmpxNgeF32 = 89,
        kCmpxNlgF32 = 90,
        kCmpxNgtF32 = 91,
        kCmpxNleF32 = 92,
        kCmpxNeqF32 = 93,
        kCmpxNltF32 = 94,
        kCmpxTruF32 = 95,
        kCmpFF64 = 96,
        kCmpLtF64 = 97,
        kCmpEqF64 = 98,
        kCmpLeF64 = 99,
        kCmpGtF64 = 100,
        kCmpLgF64 = 101,
        kCmpGeF64 = 102,
        kCmpOF64 = 103,
        kCmpUF64 = 104,
        kCmpNgeF64 = 105,
        kCmpNlgF64 = 106,
        kCmpNgtF64 = 107,
        kCmpNleF64 = 108,
        kCmpNeqF64 = 109,
        kCmpNltF64 = 110,
        kCmpTruF64 = 111,
        kCmpxFF64 = 112,
        kCmpxLtF64 = 113,
        kCmpxEqF64 = 114,
        kCmpxLeF64 = 115,
        kCmpxGtF64 = 116,
        kCmpxLgF64 = 117,
        kCmpxGeF64 = 118,
        kCmpxOF64 = 119,
        kCmpxUF64 = 120,
        kCmpxNgeF64 = 121,
        kCmpxNlgF64 = 122,
        kCmpxNgtF64 = 123,
        kCmpxNleF64 = 124,
        kCmpxNeqF64 = 125,
        kCmpxNltF64 = 126,
        kCmpxTruF64 = 127,
        kCmpFI16 = 160,
        kCmpLtI16 = 161,
        kCmpEqI16 = 162,
        kCmpLeI16 = 163,
        kCmpGtI16 = 164,
        kCmpNeI16 = 165,
        kCmpGeI16 = 166,
        kCmpTI16 = 167,
        kCmpFU16 = 168,
        kCmpLtU16 = 169,
        kCmpEqU16 = 170,
        kCmpLeU16 = 171,
        kCmpGtU16 = 172,
        kCmpNeU16 = 173,
        kCmpGeU16 = 174,
        kCmpTU16 = 175,
        kCmpxFI16 = 176,
        kCmpxLtI16 = 177,
        kCmpxEqI16 = 178,
        kCmpxLeI16 = 179,
        kCmpxGtI16 = 180,
        kCmpxNeI16 = 181,
        kCmpxGeI16 = 182,
        kCmpxTI16 = 183,
        kCmpxFU16 = 184,
        kCmpxLtU16 = 185,
        kCmpxEqU16 = 186,
        kCmpxLeU16 = 187,
        kCmpxGtU16 = 188,
        kCmpxNeU16 = 189,
        kCmpxGeU16 = 190,
        kCmpxTU16 = 191,
        kCmpFI32 = 192,
        kCmpLtI32 = 193,
        kCmpEqI32 = 194,
        kCmpLeI32 = 195,
        kCmpGtI32 = 196,
        kCmpNeI32 = 197,
        kCmpGeI32 = 198,
        kCmpTI32 = 199,
        kCmpFU32 = 200,
        kCmpLtU32 = 201,
        kCmpEqU32 = 202,
        kCmpLeU32 = 203,
        kCmpGtU32 = 204,
        kCmpNeU32 = 205,
        kCmpGeU32 = 206,
        kCmpTU32 = 207,
        kCmpxFI32 = 208,
        kCmpxLtI32 = 209,
        kCmpxEqI32 = 210,
        kCmpxLeI32 = 211,
        kCmpxGtI32 = 212,
        kCmpxNeI32 = 213,
        kCmpxGeI32 = 214,
        kCmpxTI32 = 215,
        kCmpxFU32 = 216,
        kCmpxLtU32 = 217,
        kCmpxEqU32 = 218,
        kCmpxLeU32 = 219,
        kCmpxGtU32 = 220,
        kCmpxNeU32 = 221,
        kCmpxGeU32 = 222,
        kCmpxTU32 = 223,
        kCmpFI64 = 224,
        kCmpLtI64 = 225,
        kCmpEqI64 = 226,
        kCmpLeI64 = 227,
        kCmpGtI64 = 228,
        kCmpNeI64 = 229,
        kCmpGeI64 = 230,
        kCmpTI64 = 231,
        kCmpFU64 = 232,
        kCmpLtU64 = 233,
        kCmpEqU64 = 234,
        kCmpLeU64 = 235,
        kCmpGtU64 = 236,
        kCmpNeU64 = 237,
        kCmpGeU64 = 238,
        kCmpTU64 = 239,
        kCmpxFI64 = 240,
        kCmpxLtI64 = 241,
        kCmpxEqI64 = 242,
        kCmpxLeI64 = 243,
        kCmpxGtI64 = 244,
        kCmpxNeI64 = 245,
        kCmpxGeI64 = 246,
        kCmpxTI64 = 247,
        kCmpxFU64 = 248,
        kCmpxLtU64 = 249,
        kCmpxEqU64 = 250,
        kCmpxLeU64 = 251,
        kCmpxGtU64 = 252,
        kCmpxNeU64 = 253,
        kCmpxGeU64 = 254,
        kCmpxTU64 = 255,

        // Illegal.
        kIllegal = 256,
    };

    VIVOPCInstruction(unsigned int instruciton_width, Encoding instruction_encoding, VopcOp op,
        int label_, int iGotoLabel) : VOPInstruction(instruciton_width, InstructionSet_VOPC, label_, iGotoLabel), op_(op), instruction_encoding_(instruction_encoding)
    {
        hw_gen_ = GDT_HW_GENERATION_VOLCANICISLAND;
    }

    ~VIVOPCInstruction() = default;

    // Get the OP
    VopcOp GetOp() const { return op_; }

    // return the instruction encoding
    Encoding GetInstructionType() const { return instruction_encoding_; }

    int GetInstructionClockCount()
    {
        int ret = 4;
        return ret;
    }

private:
    // VOP operation.
    VopcOp op_;

    // Instruction Type.
    Encoding instruction_encoding_;
};


// VOP1 instructions for VEGA (GFX9)
class G9VOP1Instruction : public VOPInstruction
{
public:

    enum Vop1Op
    {
        kNop = 0,
        kMovB32 = 1,
        kReadfirstlaneB32 = 2,
        kCvtI32F64 = 3,
        kCvtF64I32 = 4,
        kCvtF32I32 = 5,
        kCvtF32U32 = 6,
        kCvtU32F32 = 7,
        kCvtI32F32 = 8,
        kMovFedB32 = 9,
        kCvtF16F32 = 10,
        kCvtF32F16 = 11,
        kCvtRpiI32F32 = 12,
        kCvtFlrI32F32 = 13,
        kCvtOffF32I4 = 14,
        kCvtF32F64 = 15,
        kCvtF64F32 = 16,
        kCvtF32Ubyte0 = 17,
        kCvtF32Ubyte1 = 18,
        kCvtF32Ubyte2 = 19,
        kCvtF32Ubyte3 = 20,
        kCvtU32F64 = 21,
        kCvtF64U32 = 22,
        kTruncF64 = 23,
        kCeilF64 = 24,
        kRndneF64 = 25,
        kFloorF64 = 26,
        kFractF32 = 27,
        kTruncF32 = 28,
        kCeilF32 = 29,
        kRndneF32 = 30,
        kFloorF32 = 31,
        kExpF32 = 32,
        kLogF32 = 33,
        kRcpF32 = 34,
        kRcpIflagF32 = 35,
        kRsqF32 = 36,
        kRcpF64 = 37,
        kRsqF64 = 38,
        kSqrtF32 = 39,
        kSqrtF64 = 40,
        kSinF32 = 41,
        kCosF32 = 42,
        kNotB32 = 43,
        kBfrevB32 = 44,
        kFfbhU32 = 45,
        kFfblB32 = 46,
        kFfbhI32 = 47,
        kFrexpExpI32F64 = 48,
        kFrexpMantF64 = 49,
        kFractF64 = 50,
        kFrexpExpI32F32 = 51,
        kFrexpMantF32 = 52,
        kClrexcp = 53,
        kScreenPartition4seB32 = 55,
        kCvtF16U16 = 57,
        kCvtF16I16 = 58,
        kCvtU16F16 = 59,
        kCvtI16F16 = 60,
        kRcpF16 = 61,
        kSqrtF16 = 62,
        kRsqF16 = 63,
        kLogF16 = 64,
        kExpF16 = 65,
        kFrexpMantF16 = 66,
        kFrexpExpI16F16 = 67,
        kFloorF16 = 68,
        kCeilF16 = 69,
        kTruncF16 = 70,
        kRndneF16 = 71,
        kFractF16 = 72,
        kSinF16 = 73,
        kCosF16 = 74,
        kExpLegacyF32 = 75,
        kLogLegacyF32 = 76,
        kCvtNormI16F16 = 77,
        kCvtNormU16F16 = 78,
        kSatPkU8I16 = 79,
        kSwapB32 = 81,
        // Illegal
        kIllegal = 82,
    };

    G9VOP1Instruction(unsigned int instruction_width, Encoding instruction_encoding, Vop1Op op, int label, int goto_label)
        : VOPInstruction(instruction_width, InstructionSet_VOP1, label, goto_label), op_(op), instruction_encoding_(instruction_encoding) { }

    ~G9VOP1Instruction() = default;

    Vop1Op GetOp() const { return op_; }

    // Return the instruction encoding.
    Encoding GetInstructionType() const { return instruction_encoding_; }

private:
    // VOP operation.
    Vop1Op op_;

    // Instruction Type.
    Encoding instruction_encoding_;
};

// VOP2 instructions for VEGA (GFX9).
class G9VOP2Instruction : public VOPInstruction
{
public:
    enum Vop2Op
    {
        kCndmaskb32 = 0,
        kAddF32 = 1,
        kSubF32 = 2,
        kSubrevF32 = 3,
        kMulLegacyF32 = 4,
        kMulF32 = 5,
        kMulI32i24 = 6,
        kMulHiI32I24 = 7,
        kMulU32u24 = 8,
        kMulHiu32U24 = 9,
        kMinF32 = 10,
        kMaxF32 = 11,
        kMinI32 = 12,
        kMaxI32 = 13,
        kMinU32 = 14,
        kMaxU32 = 15,
        kLshrrevB32 = 16,
        kAshrrevI32 = 17,
        kLshlrevB32 = 18,
        kAndB32 = 19,
        kOrB32 = 20,
        kXorB32 = 21,
        kMacF32 = 22,
        kMadmkF32 = 23,
        kMadakF32 = 24,
        kAddCoU32 = 25,
        kSubCoU32 = 26,
        kSubrevCoU32 = 27,
        kAddcCoU32 = 28,
        kSubbCoU32 = 29,
        kSubbrevCoU32 = 30,
        kAddF16 = 31,
        kSubF16 = 32,
        kSubrevF16 = 33,
        kMulF16 = 34,
        kMacF16 = 35,
        kMadmkF16 = 36,
        kMadakF16 = 37,
        kAddU16 = 38,
        kSubU16 = 39,
        kSubrevU16 = 40,
        kMulLoU16 = 41,
        kLshlrevB16 = 42,
        kLshrrevB16 = 43,
        kAshrrevI16 = 44,
        kMaxF16 = 45,
        kMinF16 = 46,
        kMaxU16 = 47,
        kMaxI16 = 48,
        kMinU16 = 49,
        kMinI16 = 50,
        kLdexpF16 = 51,
        kAddU32 = 52,
        kSubU32 = 53,
        kSubrevU32 = 54,
        kIllegal = 55
    };

    G9VOP2Instruction(unsigned int instruciton_width, Encoding instruction_encoding, Vop2Op op, int label_, int goto_label)
        : VOPInstruction(instruciton_width, InstructionSet_VOP2, label_, goto_label), op_(op), instruction_encoding_(instruction_encoding)
    {
        hw_gen_ = GDT_HW_GENERATION_GFX9;
    }

    ~G9VOP2Instruction() = default;

    Vop2Op GetOp() const { return op_; }

    // Return the instruction encoding.
    Encoding GetInstructionType() const { return instruction_encoding_; }

    int GetInstructionClockCount()
    {
        int ret = 4;
        return ret;
    }

private:

    // VOP operation.
    Vop2Op op_;

    // Instruction Type.
    Encoding instruction_encoding_;
};

// VOP3 for VEGA (GFX9)
class G9VOP3Instruction : public VOPInstruction
{
public:
    enum Vop3Op
    {
        kMadLegacyF32 = 448,
        kMadF32 = 449,
        kMadI32I24 = 450,
        kMadU32U24 = 451,
        kCubeidF32 = 452,
        kCubescF32 = 453,
        kCubetcF32 = 454,
        kCubemaF32 = 455,
        kBfeU32 = 456,
        kBfeI32 = 457,
        kBfiB32 = 458,
        kFmaF32 = 459,
        kFmaF64 = 460,
        kLerpU8 = 461,
        kAlignbitB32 = 462,
        kAlignbyteB32 = 463,
        kMin3F32 = 464,
        kMin3I32 = 465,
        kMin3U32 = 466,
        kMax3F32 = 467,
        kMax3I32 = 468,
        kMax3U32 = 469,
        kMed3F32 = 470,
        kMed3I32 = 471,
        kMed3U32 = 472,
        kSadU8 = 473,
        kSadHiU8 = 474,
        kSadU16 = 475,
        kSadU32 = 476,
        kCvtPkU8F32 = 477,
        kDivFixupF32 = 478,
        kDivFixupF64 = 479,
        kDivScaleF32 = 480,
        kDivScaleF64 = 481,
        kDivFmasF32 = 482,
        kDivFmasF64 = 483,
        kMsadU8 = 484,
        kQsadPkU16U8 = 485,
        kMqsadPkU16U8 = 486,
        kMqsadU32U8 = 487,
        kMadU64U32 = 488,
        kMadI64I32 = 489,
        kMadLegacyF16 = 490,
        kMadLegacyU16 = 491,
        kMadLegacyI16 = 492,
        kPermB32 = 493,
        kFmaLegacyF16 = 494,
        kDivFixupLegacyF16 = 495,
        kCvtPkaccumU8F32 = 496,
        kMadU32U16 = 497,
        kMadI32I16 = 498,
        kXadU32 = 499,
        kMin3F16 = 500,
        kMin3I16 = 501,
        kMin3U16 = 502,
        kMax3F16 = 503,
        kMax3I16 = 504,
        kMax3U16 = 505,
        kMed3F16 = 506,
        kMed3I16 = 507,
        kMed3U16 = 508,
        kLshl_addB32 = 509,
        kAdd_lshlB32 = 510,
        kAdd3U32 = 511,
        kLshl_orB32 = 512,
        kAnd_orB32 = 513,
        kOr3B32 = 514,
        kMadF16 = 515,
        kMadU16 = 516,
        kMadI16 = 517,
        kFmaF16 = 518,
        kDivFixupF16 = 519,
        kInterpP1F32 = 624,
        kInterpP2F32 = 625,
        kInterpMovF32 = 626,
        kInterpP1llF16 = 628,
        kInterpP1lvF16 = 629,
        kInterpP2LegacyF16 = 630,
        kInterpP2F16 = 631,
        kAddF64 = 640,
        kMulF64 = 641,
        kMinF64 = 642,
        kMaxF64 = 643,
        kLdexpF64 = 644,
        kMulLoU32 = 645,
        kMulHiU32 = 646,
        kMulHiI32 = 647,
        kLdexpF32 = 648,
        kReadlaneB32 = 649,
        kWritelaneB32 = 650,
        kBcntU32B32 = 651,
        kMbcntLoU32B32 = 652,
        kMbcntHiU32B32 = 653,
        kLshlrevB64 = 655,
        kLshrrevB64 = 656,
        kAshrrevI64 = 657,
        kTrigPreopF64 = 658,
        kBfmB32 = 659,
        kCvtPknormI16F32 = 660,
        kCvtPknormU16F32 = 661,
        kCvtPkrtzF16F32 = 662,
        kCvtPkU16U32 = 663,
        kCvtPkI16I32 = 664,
        kCvtPknormI16F16 = 665,
        kCvtPknormU16F16 = 666,
        kReadlane_regrdB32 = 667,
        kAddI32 = 668,
        kSubI32 = 669,
        kAddI16 = 670,
        kSubI16 = 671,
        kPackB32F16 = 672,

        // Illegal.
        kIllegal = 673
    };// end VOP3

    G9VOP3Instruction(unsigned int instruciton_width, Encoding instruction_encoding, Vop3Op op, int label, int goto_label)
        : VOPInstruction(instruciton_width, InstructionSet_VOP3, label, goto_label), op_(op), instruction_encoding_(instruction_encoding)
    {
        hw_gen_ = GDT_HW_GENERATION_GFX9;
    }

    ~G9VOP3Instruction() = default;

    // Get the OP
    Vop3Op GetOp() const { return op_; }

    // Return the instruction encoding.
    Encoding GetInstructionType() const { return instruction_encoding_; }

private:
    // VOP operation.
    Vop3Op op_;

    // Instruction Type.
    Encoding instruction_encoding_;
};

// VOP3 for VEGA (GFX9).
class G9VOP3PInstruction : public VOPInstruction
{
public:

    enum Vop3pOp
    {
        kPkMadI16 = 0,
        kPkMulLoU16 = 1,
        kPkAddI16 = 2,
        kPkSubI16 = 3,
        kPkLshlrevB16I16 = 4,
        kPkLshrrevB16I16 = 5,
        kPkAshrrevB16I16 = 6,
        kPkMaxI16 = 7,
        kPkMinI16 = 8,
        kPkMadU16 = 9,
        kPkAddU16 = 10,
        kPkSubU16 = 11,
        kPkMaxU16 = 12,
        kPkMinU16 = 13,
        kPkFmaF16 = 14,
        kPkAddF16 = 15,
        kPkMulF16 = 16,
        kPkMinF16 = 17,
        kPkMaxF16 = 18,
        kMadMixF32 = 32,
        kMadMixloF16 = 33,
        kMadMixhiF16 = 34,

        // Illegal.
        kIllegal = 35
    };// end VOP3P

    G9VOP3PInstruction(unsigned int instruciton_width, Encoding instruction_encoding, Vop3pOp op, int label, int goto_label)
        : VOPInstruction(instruciton_width, InstructionSet_VOP3, label, goto_label), op_(op), instruction_encoding_(instruction_encoding)
    {
        hw_gen_ = GDT_HW_GENERATION_GFX9;
    }

    ~G9VOP3PInstruction() = default;

    // Get the OP.
    Vop3pOp GetOp() const { return op_; }

    // Return the instruction encoding.
    Encoding GetInstructionType() const { return instruction_encoding_; }

private:
    // VOP operation.
    Vop3pOp op_;

    // Instruction Type.
    Encoding instruction_encoding_;
};

#endif //__VOPINSTRUCTION_H


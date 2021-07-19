//=================================================================
// Copyright 2020 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

#ifndef __DSINSTRUCTION_H
#define __DSINSTRUCTION_H

#include "be_instruction.h"

// Local and global data share instructions.
// Opcode :
//        OFFSET0 [7:0]
//        OFFSET1 [15:8]
//        GDS     [17:17] - 0 = LDS; 1 = GDS
//        OP      [25:18]
//        ADDR    [39:32]
//        DATA0 [47:40]
//        DATA1 [55:48]
//        VDST [63:56]

// In terms of type and variable naming in this class,
// due to legacy code that relies on Macros that use the variable name
// we will not be able to use the standard naming and would have to stick
// to capital letters separated by '_' characters.
class DSInstruction : public Instruction
{
public:
    typedef char OFFSET;
    typedef bool GDS;
    typedef char ADDR;
    typedef char DATA;
    typedef char VDST;

protected:
    // Unsigned byte offset added to the address supplied by the ADDR VGPR.
    OFFSET offset0_;

    // Unsigned byte offset added to the address supplied by the ADDR VGPR.
    OFFSET offset1_;

    // 0(false) = LDS; 1(true) = GDS.
    GDS is_gds_;

    // Source LDS address VGPR 0 - 255.
    ADDR addr_;

    // Source data0 VGPR 0 - 255.
    DATA data0_;

    // Source data1 VGPR 0 - 255.
    DATA data1_;

    // Destination VGPR 0 - 255.
    VDST vdst_;

    // DS Instruction Width in bits.
    static const unsigned int ds_instruction_width_ = 64;

public:
    DSInstruction(OFFSET offset0, OFFSET offset1, GDS is_gds, ADDR addr, DATA data0, DATA data1, VDST vdst, int label, int goto_label) :
        Instruction(ds_instruction_width_, is_gds ? InstructionCategory::kGds : InstructionCategory::kLds, kInstructionSetDs, label, goto_label),
        offset0_(offset0), offset1_(offset1), is_gds_(is_gds), addr_(addr), data0_(data0), data1_(data1), vdst_(vdst) { }

    virtual ~DSInstruction() = default;

    // Get the OFFSET0 [7:0] / OFFSET1 [15:8]
    // offset_index is the offset`s index (0 or 1).
    OFFSET GetOffset(const unsigned char offset_index) const
    {
        if (offset_index)
        {
            return offset1_;
        }
        return offset0_;
    }

    // Get the GDS [17:17].
    GDS GetGds() const { return is_gds_; }

    // Get the ADDR [39:32].
    ADDR GetAddr() const { return addr_; }

    // Get the DATA0 [47:40] / DATA1 [55:48]
    // data_index is the data`s index (0 or 1).
    DATA GetDATA(const unsigned char data_index) const
    {
        if (data_index)
        {
            return data1_;
        }
        return data0_;
    }

    // Get the VDST [63:56].
    VDST GetVdst() const { return vdst_; }
};

class SIDSInstruction : public DSInstruction
{
public:
    // Selector for the DS Instruction
    enum OP
    {
        // DS[A] = DS[A] + D0; uint add.
        kDsAddU32,
        // DS[A] = DS[A] - D0; uint subtract.
        kDsSubU32,
        // DS[A] = D0 - DS[A]; uint reverse subtract.
        kDsRSubU32,
        // DS[A] = (DS[A] >= D0 ? 0 : DS[A] + 1); uint increment.
        kDsIncU32,
        // DS[A] = (DS[A] == 0 || DS[A] > D0 ? D0 : DS[A] -
        // 1); uint decrement.
        kDsDecU32,
        // DS[A] = min(DS[A], D0); int min.
        kDsMinI32,
        // DS[A] = max(DS[A], D0); int max.
        kDsMaxI32,
        // DS[A] = min(DS[A], D0); uint min.
        kDsMinU32,
        // DS[A] = max(DS[A], D0); uint max.
        kDsMaxU32,
        // DS[A] = DS[A] & D0; Dword AND.
        kDsAndB32,
        // DS[A] = DS[A] | D0; Dword OR.
        kDsOrB32,
        // DS[A] = DS[A] ^ D0; Dword XOR.
        kDsXorB32,
        // DS[A] = (DS[A] ^ ~D0) | D1; masked Dword OR.
        kDsMskorB32,
        // DS[A] = D0; write a Dword.
        kDsWriteB32,
        // DS[ADDR+offset0*4] = D0;
        // DS[ADDR+offset1*4] = D1; write 2 Dwords.
        kDsWrite2B32,
        // DS[ADDR+offset0*4*64] = D0;
        // DS[ADDR+offset1*4*64] = D1; write 2 Dwords.
        kDsWRITE2St64B32,
        // DS[A] = (DS[A] == D0 ? D1 : DS[A]); compare
        // store.
        kDsCmpstB32,
        // DS[A] = (DS[A] == D0 ? D1 : DS[A]); compare
        // store with float rules.
        kDsCmpstF32,
        // DS[A] = (DS[A] < D1) ? D0 : DS[A]; float compare
        // swap (handles NaN/INF/denorm).
        kDsMinF32,
        // DS[A] = (D0 > DS[A]) ? D0 : DS[A]; float, handles
        // NaN/INF/denorm.
        kDsMaxF32 = 19,

        kDsNOP = 20, // CI Specific
        kDsGwsSemaReleaseAll = 24, //CI Specific

        // GDS only.
        kDsGwsINIT = 25,
        // GDS only.
        kDsGwsSemaV,
        // GDS only.
        kDsGwsSemaBR,
        // GDS only.
        kDsGwsSemaP,
        // GDS only.
        kDsGwsBARRIER,
        // DS[A] = D0[7:0]; byte write.
        kDsWriteB8,
        // DS[A] = D0[15:0]; short write.
        kDsWriteB16,
        // Uint add.
        kDsAddRtnU32,
        // Uint subtract.
        kDsSubRtnU32,
        // Uint reverse subtract.
        kDsRSubRtnU32,
        // Uint increment.
        kDsIncRtnU32,
        // Uint decrement.
        kDsDecRtnU32,
        // Int min.
        kDsMinRtnI32,
        // Int max.
        kDsMaxRtnI32,
        // Uint min.
        kDsMinRtnU32,
        // Uint max.
        kDsMaxRtnU32,
        // Dword AND.
        kDsAndRtnB32,
        // Dword OR.
        kDsOrRtnB32,
        // Dword XOR.
        kDsXorRtnB32,
        // Masked Dword OR.
        kDsMskorRtnB32,
        // Write exchange. Offset = {offset1,offset0}.
        // A = ADDR+offset. D=DS[Addr].
        // DS[Addr]=D0.
        kDsWrxchgRtnB32,
        // Write exchange 2 separate Dwords.
        kDsWrxchg2RtnB32,
        // Write echange 2 Dwords, stride 64.
        kDsWRXCHG2St64RtnB32,
        // Compare store.
        kDsCmpstRtnB32,
        // Compare store with float rules.
        kDsCmpstRtnF32,
        // DS[A] = (DS[A] < D1) ? D0 : DS[A]; float compare
        // swap (handles NaN/INF/denorm).
        kDsMinRtnF32,
        // DS[A] = (D0 > DS[A]) ? D0 : DS[A]; float, handles
        // NaN/INF/denorm .
        kDsMaxRtnF32 = 51,

        kDsWRAP_RtnB32 = 52, //CI Specific

        // R = swizzle(Data(vgpr), offset1:offset0).
        // Dword swizzle. no data is written to
        // LDS. see ds_opcodes.docx for details.
        kDsSwizzleB32 = 53,
        // R = DS[A]; Dword read.
        kDsReadB32,
        // R = DS[ADDR+offset0*4], R+1 =
        // DS[ADDR+offset1*4]. Read 2 Dwords.
        kDsRead2B32,
        // R = DS[ADDR+offset0*4*64], R+1 =
        // DS[ADDR+offset1*4*64]. Read 2
        // Dwords.
        kDsREAD2St64B32,
        // R = signext(DS[A][7:0]}; signed byte read.
        kDsReadI8,
        // R = {24�h0,DS[A][7:0]}; unsigned byte read.
        kDsReadU8,
        // R = signext(DS[A][15:0]}; signed short read.
        kDsReadI16,
        // R = {16�h0,DS[A][15:0]}; unsigned short read.
        kDsReadU16,
        // Consume entries from a buffer.
        kDsCONSUME,
        // Append one or more entries to a buffer.
        kDsAPPEND,
        // Increment an append counter. Operation is
        // done in order of wavefront creation.
        kDsORDERED_COUNT,
        // Uint add.
        kDsAddU64,
        // Uint subtract.
        kDsSubU64,
        // Uint reverse subtract.
        kDsRSubU64,
        // Uint increment.
        kDsIncU64,
        // Uint decrement.
        kDsDecU64,
        // Int min.
        kDsMinI64,
        // Int max.
        kDsMaxI64,
        // Uint min.
        kDsMinU64,
        // Uint max.
        kDsMaxU64,
        // Dword AND.
        kDsAndB64,
        // Dword OR.
        kDsOrB64,
        // Dword XOR.
        kDsXorB64,
        // Masked Dword XOR.
        kDsMskorB64,
        // Write.
        kDsWriteB64,
        // DS[ADDR+offset0*8] = D0;
        // DS[ADDR+offset1*8] = D1; write 2 Dwords.
        kDsWrite2B64,
        // DS[ADDR+offset0*8*64] = D0;
        // DS[ADDR+offset1*8*64] = D1; write 2 Dwords.
        kDsWRITE2St64B64,
        // Compare store.
        kDsCmpstB64,
        // Compare store with float rules.
        kDsCmpstF64,
        // DS[A] = (D0 < DS[A]) ? D0 : DS[A]; float, handles
        // NaN/INF/denorm.
        kDsMinF64,
        // DS[A] = (D0 > DS[A]) ? D0 : DS[A]; float, handles
        // NaN/INF/denorm.
        kDsMaxF64 = 83,
        // Uint add.
        kDsAddRtnU64 = 96,
        // Uint subtract.
        kDsSubRtnU64,
        // Uint reverse subtract.
        kDsRSubRtnU64,
        // Uint increment.
        kDsIncRtnU64,
        // Uint decrement.
        kDsDecRtnU64,
        // Int min.
        kDsMinRtnI64,
        // Int max.
        kDsMaxRtnI64,
        // Uint min.
        kDsMinRtnU64,
        // Uint max.
        kDsMaxRtnU64,
        // Dword AND.
        kDsAndRtnB64,
        // Dword OR.
        kDsOrRtnB64,
        // Dword XOR.
        kDsXorRtnB64,
        // Masked Dword XOR.
        kDsMskorRtnB64,
        // Write exchange.
        kDsWrxchgRtnB64,
        // Write exchange relative.
        kDsWrxchg2RtnB64,
        // Write echange 2 Dwords.
        kDsWRXCHG2St64RtnB64,
        // Compare store.
        kDsCmpstRtnB64,
        // Compare store with float rules.
        kDsCmpstRtnF64,
        // DS[A] = (D0 < DS[A]) ? D0 : DS[A]; float, handles
        // NaN/INF/denorm.
        kDsMinRtnF64,
        // DS[A] = (D0 > DS[A]) ? D0 : DS[A]; float, handles
        // NaN/INF/denorm.
        kDsMaxRtnF64 = 115,
        // Dword read.
        kDsReadB64 = 118,
        // R = DS[ADDR+offset0*8], R+1 =
        // DS[ADDR+offset1*8]. Read 2 Dwords
        kDsRead2B64 = 119,
        // R = DS[ADDR+offset0*8*64], R+1 =
        // DS[ADDR+offset1*8*64]. Read 2 Dwords.
        kDsREAD2St64B64 = 120,

        kDsCONDXCHG32_RtnB64 = 126, //CI Specific

        // B = A + 4*(offset1[7] ? {A[31],A[31:17]} :
        // {offset1[6],offset1[6:0],offset0}). DS[A] = DS[A]
        // + DS[B]; uint add.
        kDsAddSrc2U32 = 128,
        // B = A + 4*(offset1[7] ? {A[31],A[31:17]} :
        // {offset1[6],offset1[6:0],offset0}). DS[A] = DS[A]
        // - DS[B]; uint subtract.
        kDsSubSrc2U32 = 129,
        // B = A + 4*(offset1[7] ? {A[31],A[31:17]} :
        // {offset1[6],offset1[6:0],offset0}). DS[A] = DS[B]
        // - DS[A]; uint reverse subtract
        kDsRSubSrc2U32 = 130,
        // B = A + 4*(offset1[7] ? {A[31],A[31:17]} :
        // {offset1[6],offset1[6:0],offset0}). DS[A] = (DS[A]
        // >= DS[B] ? 0 : DS[A] + 1); uint increment.
        kDsINCSrc2U32 = 131,
        // B = A + 4*(offset1[7] ? {A[31],A[31:17]} :
        // {offset1[6],offset1[6:0],offset0}). DS[A] = (DS[A]
        // == 0 || DS[A] > DS[B] ? DS[B] : DS[A] - 1); uint
        // decrement.
        kDsDECSrc2U32 = 132,
        // B = A + 4*(offset1[7] ? {A[31],A[31:17]} :
        // {offset1[6],offset1[6:0],offset0}). DS[A] =
        // min(DS[A], DS[B]); int min.
        kDsMinSrc2I32 = 133,
        // B = A + 4*(offset1[7] ? {A[31],A[31:17]} :
        // {offset1[6],offset1[6:0],offset0}). DS[A] =
        // max(DS[A], DS[B]); int max.
        kDsMaxSrc2I32 = 134,
        // B = A + 4*(offset1[7] ? {A[31],A[31:17]} :
        // {offset1[6],offset1[6:0],offset0}). DS[A] =
        // min(DS[A], DS[B]); uint min.
        kDsMinSrc2U32 = 135,
        // B = A + 4*(offset1[7] ? {A[31],A[31:17]} :
        // {offset1[6],offset1[6:0],offset0}). DS[A] = max(DS[A], DS[B]); uint maxw
        kDsMaxSrc2U32 = 136,
        // B = A + 4*(offset1[7] ? {A[31],A[31:17]} :
        // {offset1[6],offset1[6:0],offset0}). DS[A] =
        // max(DS[A], DS[B]); uint maxw137
        // kDsANDSrc2B32B = A + 4*(offset1[7] ?
        kDsANDSrc2B32 = 137,
        // B = A + 4*(offset1[7] ? {A[31],A[31:17]} :
        // {offset1[6],offset1[6:0],offset0}). DS[A] = DS[A] |
        // DS[B]; Dword OR.
        kDsORSrc2B32 = 138,
        // B = A + 4*(offset1[7] ? {A[31],A[31:17]} :
        // {offset1[6],offset1[6:0],offset0}). DS[A] = DS[A] ^
        // DS[B]; Dword XOR.
        kDsXORSrc2B32 = 139,
        // B = A + 4*(offset1[7] ? {A[31],A[31:17]} :
        // {offset1[6],offset1[6:0],offset0}). DS[A] = DS[B];
        // write Dword.
        kDsWriteSrc2B32 = 140,
        // B = A + 4*(offset1[7] ? {A[31],A[31:17]} :
        // {offset1[6],offset1[6:0],offset0}). DS[A] = (DS[B]
        // < DS[A]) ? DS[B] : DS[A]; float, handles
        // NaN/INF/denorm.
        kDsMinSrc2F32 = 146,
        // B = A + 4*(offset1[7] ? {A[31],A[31:17]} :
        // {offset1[6],offset1[6:0],offset0}). DS[A] = (DS[B]
        // > DS[A]) ? DS[B] : DS[A]; float, handles
        // NaN/INF/denorm.
        kDsMaxSrc2F32 = 147,
        // Uint add.
        kDsAddSrc2U64 = 192,
        // Uint subtract.
        kDsSubSrc2U64,
        // Uint reverse subtract.
        kDsRSubSrc2U64,
        // Uint increment.
        kDsINCSrc2U64,
        // Uint decrement.
        kDsDECSrc2U64,
        // Int min.
        kDsMinSrc2I64,
        // Int max.
        kDsMaxSrc2I64,
        // Uint min.
        kDsMinSrc2U64,
        // Uint max.
        kDsMaxSrc2U64 = 200,
        // Dword AND.
        kDsANDSrc2B64,
        // Dword OR.
        kDsORSrc2B64,
        // Dword XOR.
        kDsXORSrc2B64,
        // B = A + 4*(offset1[7] ? {A[31],A[31:17]} :
        // {offset1[6],offset1[6:0],offset0}). DS[A] =
        // DS[B]; write Qword.
        kDsWriteSrc2B64 = 204,
        // B = A + 4*(offset1[7] ? {A[31],A[31:17]} :
        // {offset1[6],offset1[6:0],offset0}). [A] = (D0 <
        // DS[A]) ? D0 : DS[A]; float, handles
        // NaN/INF/denorm.
        kDsMinSrc2F64 = 210,
        // B = A + 4*(offset1[7] ? {A[31],A[31:17]} :
        // {offset1[6],offset1[6:0],offset0}). [A] = (D0 >
        // DS[A]) ? D0 : DS[A]; float, handles
        // NaN/INF/denorm.
        kDsMaxSrc2F64 = 211,

        kDsWriteB96 = 222, // CI Specific
        kDsWriteB128 = 223, // CI Specific
        kDsCONDXCHG32_RtnB128 = 253, // CI Specific
        kDsReadB96 = 254, // CI Specific
        kDsReadB128 = 255, // CI Specific
        // Reserved
        kDsReserved
    };

    int GetInstructionClockCount()
    {
        int iRet = 1;

        switch (op_)
        {
        case    kDsMinI32:
        case    kDsMaxI32:
        case    kDsMinU32:
        case    kDsMaxU32:
        case    kDsAndB32:
        case    kDsOrB32:
        case    kDsXorB32:
        case    kDsMskorB32:
        case    kDsWriteB32:
        case    kDsWrite2B32:
        case    kDsWRITE2St64B32:
        case    kDsCmpstB32:
        case    kDsCmpstF32:
        case    kDsMinF32:
        case    kDsMaxF32:
        case    kDsWriteB8:
        case    kDsWriteB16:
        case    kDsAddRtnU32:
        case    kDsSubRtnU32:
        case    kDsRSubRtnU32:
        case    kDsIncRtnU32:
        case    kDsDecRtnU32:
        case    kDsMinRtnI32:
        case    kDsMaxRtnI32:
        case    kDsMinRtnU32:
        case    kDsMaxRtnU32:
        case    kDsAndRtnB32:
        case    kDsOrRtnB32:
        case    kDsXorRtnB32:
        case    kDsMskorRtnB32:
        case    kDsWrxchgRtnB32:
        case    kDsWrxchg2RtnB32:
        case    kDsWRXCHG2St64RtnB32:
        case    kDsCmpstRtnB32:
        case    kDsCmpstRtnF32:
        case    kDsMinRtnF32:
        case    kDsMaxRtnF32:
        case    kDsNOP: // hunch
            iRet = 4;
            break;

        case    kDsSwizzleB32:
        case    kDsReadB32:
        case    kDsRead2B32:
        case    kDsREAD2St64B32:
        case    kDsReadI8:
        case    kDsReadU8:
        case    kDsReadI16:
        case    kDsReadU16:
        case    kDsCONSUME:
        case    kDsAPPEND:
        case    kDsAddU64:
        case    kDsSubU64:
        case    kDsRSubU64:
        case    kDsIncU64:
        case    kDsDecU64:
        case    kDsMinI64:
        case    kDsMaxI64:
        case    kDsMinU64:
        case    kDsMaxU64:
        case    kDsAndB64:
        case    kDsOrB64:
        case    kDsXorB64:
        case    kDsMskorB64:
        case    kDsWriteB64:
        case    kDsWrite2B64:
        case    kDsWRITE2St64B64:
        case    kDsCmpstB64:
        case    kDsCmpstF64:
        case    kDsMinF64:
        case    kDsMaxF64:
            iRet = 8;
            break;

        case    kDsAddRtnU64:
        case    kDsSubRtnU64:
        case    kDsRSubRtnU64:
        case    kDsIncRtnU64:
        case    kDsDecRtnU64:
        case    kDsMinRtnI64:
        case    kDsMaxRtnI64:
        case    kDsMinRtnU64:
        case    kDsMaxRtnU64:
        case    kDsAndRtnB64:
        case    kDsOrRtnB64:
        case    kDsXorRtnB64:
        case    kDsMskorRtnB64:
        case    kDsWrxchgRtnB64:
        case    kDsWrxchg2RtnB64:
        case    kDsWRXCHG2St64RtnB64:
        case    kDsCmpstRtnB64:
        case    kDsCmpstRtnF64:
        case    kDsMinRtnF64:
        case    kDsMaxRtnF64:
        case    kDsReadB64:
        case    kDsRead2B64:
        case    kDsREAD2St64B64:
        case    kDsAddSrc2U32:
        case    kDsSubSrc2U32:
        case    kDsRSubSrc2U32:
        case    kDsINCSrc2U32:
        case    kDsDECSrc2U32:
        case    kDsMinSrc2I32:
        case    kDsMaxSrc2I32:
        case    kDsMinSrc2U32:
        case    kDsMaxSrc2U32:
        case    kDsANDSrc2B32:
        case    kDsORSrc2B32:
        case    kDsXORSrc2B32:
        case    kDsWriteSrc2B32:
        case    kDsMinSrc2F32:
        case    kDsMaxSrc2F32:
            iRet = 12;
            break;

        case    kDsAddSrc2U64:
        case    kDsSubSrc2U64:
        case    kDsRSubSrc2U64:
        case    kDsINCSrc2U64:
        case    kDsDECSrc2U64:
        case    kDsMinSrc2I64:
        case    kDsMaxSrc2I64:
        case    kDsMinSrc2U64:
        case    kDsMaxSrc2U64:
        case    kDsANDSrc2B64:
        case    kDsORSrc2B64:
        case    kDsXORSrc2B64:
        case    kDsWriteSrc2B64:
        case    kDsMinSrc2F64:
        case    kDsMaxSrc2F64:
            iRet = 20;
            break;

        default:
            break;
        }

        return iRet;
    }

    SIDSInstruction(OFFSET offset0, OFFSET offset1, GDS is_gds, OP op, ADDR addr, DATA data0,
        DATA data1, VDST vdst, int label, int goto_label) : DSInstruction(offset0, offset1, is_gds, addr, data0, data1, vdst, label, goto_label), op_(op)
    { }

    // Get the OP [27:23]
    OP GetOp() const { return op_; }

private:
    // DS operation.
    OP op_;
};

class VIDSInstruction : public DSInstruction
{
public:
    // Selector for the DS Instruction.
    enum OP
    {
        kDsAddU32 = 0,
        kDsSubU32 = 1,
        kDsRsubU32 = 2,
        kDsIncU32 = 3,
        kDsDecU32 = 4,
        kMinI32 = 5,
        kDsMaxI32 = 6,
        kDsMinI32 = 7,
        kDsMaxU32 = 8,
        kDsAndB32 = 9,
        kDsOrB32 = 10,
        kDsXorB32 = 11,
        kDsMskorB32 = 12,
        kDsWriteB32 = 13,
        kDsWrite2B32 = 14,
        kDsWrite2st64B32 = 15,
        kDsCmpstB32 = 16,
        kDsCmpstF32 = 17,
        kDsMinF32 = 18,
        kDsMaxF32 = 19,
        kDsNop = 20,
        kDsAddF32 = 21,
        kDsWriteB8 = 30,
        kDsWriteB16 = 31,
        kDsAddRtnU32 = 32,
        kDsSubRtnU32 = 33,
        kDsRsubRtnU32 = 34,
        kDsIncRtnU32 = 35,
        kDsDecRtnU32 = 36,
        kDsMinRtnI32 = 37,
        kDsMaxRtnI32 = 38,
        kDsMinRtnU32 = 39,
        kDsMaxRtnU32 = 40,
        kDsAndRtnB32 = 41,
        kDsOrRtnB32 = 42,
        kDsXorRtnB32 = 43,
        kDsMskorRtnB32 = 44,
        kDsWrxchgRtnB32 = 45,
        kDsWrxchg2RtnB32 = 46,
        kDsWrxchg2st64RtnB32 = 47,
        kDsCmpstRtnB32 = 48,
        kDsCmpstRtnF32 = 49,
        kDsMinRtnF32 = 50,
        kDsMaxRtnF32 = 51,
        kDsWrapRtnB32 = 52,
        kDsAddRtnF32 = 53,
        kDsReadB32 = 54,
        kDsRead2B32 = 55,
        kDsRead2st64B32 = 56,
        kDsReadI8 = 57,
        kDsReadU8 = 58,
        kDsReadI16 = 59,
        kDsReadU16 = 60,
        kDsSwizzleB32 = 61,
        kDsPermuteB32 = 62,
        kDsBpermuteB32 = 63,
        kDsAddU64 = 64,
        kDsSubU64 = 65,
        kDsRsubU64 = 66,
        kDsIncU64 = 67,
        kDsDecU64 = 68,
        kDsMinI64 = 69,
        kDsMaxI64 = 70,
        kDsMinU64 = 71,
        kDsMaxU64 = 72,
        kDsAndB64 = 73,
        kDsOrB64 = 74,
        kDsXorB64 = 75,
        kDsMskorB64 = 76,
        kDsWriteB64 = 77,
        kDsWrite2B64 = 78,
        kDsWrite2st64B64 = 79,
        kDsCmpstB64 = 80,
        kDsCmpstF64 = 81,
        kDsMinF64 = 82,
        kDsMaxF64 = 83,
        kDsAddRtnU64 = 96,
        kDsSubRtnU64 = 97,
        kDsRsubRtnU64 = 98,
        kDsIncRtnU64 = 99,
        kDsDecRtnU64 = 100,
        kDsMinRtnI64 = 101,
        kDsMaxRtnI64 = 102,
        kDsMinRtnU64 = 103,
        kDsMaxRtnU64 = 104,
        kDsAndRtnB64 = 105,
        kDsOrRtnB64 = 106,
        kDsXorRtnB64 = 107,
        kDsMskorRtnB64 = 108,
        kDsWrxchgRtnB64 = 109,
        kDsWrxchg2RtnB64 = 110,
        kDsWrxchg2st64RtnB64 = 111,
        kDsCmpstRtnB64 = 112,
        kDsCmpstRtnF64 = 113,
        kDsMinRtnF64 = 114,
        kDsMaxRtnF64 = 115,
        kDsReadB64 = 118,
        kDsRead2B64 = 119,
        kDsRead2st64B64 = 120,
        kDsCondxchg32RtnB64 = 126,
        kDsAddSrc2U32 = 128,
        kDsSubSrc2U32 = 129,
        kDsRsubSrc2U32 = 130,
        kDsIncSrc2U32 = 131,
        kDsDecSrc2U32 = 132,
        kDsMinSrc2I32 = 133,
        kDsMaxSrc2I32 = 134,
        kDsMinSrc2U32 = 135,
        kDsMaxSrc2U32 = 136,
        kDsAndSrc2B32 = 137,
        kDsOrSrc2B32 = 138,
        kDsXorSrc2B32 = 139,
        kDsWriteSrc2B32 = 141,
        kDsMinSrc2F32 = 146,
        kDsMaxSrc2F32 = 147,
        kDsAddSrc2F32 = 149,
        kDsGwsSemaReleaseAll = 152,
        kDsGwsInit = 153,
        kDsGwsSemaV = 154,
        kDsGwsSemaBr = 155,
        kDsGwsSemaP = 156,
        kDsGwsBarrier = 157,
        kDsConsume = 189,
        kDsAppend = 190,
        kDsOrderedCount = 191,
        kDsAddSrc2U64 = 192,
        kDsSubSrc2U64 = 193,
        kDsRsubSrc2U64 = 194,
        kDsIncSrc2U64 = 195,
        kDsDecSrc2U64 = 196,
        kDsMinSrc2I64 = 197,
        kDsMaxSrc2I64 = 198,
        kDsMinSrc2U64 = 199,
        kDsMaxSrc2U64 = 200,
        kDsAndSrc2B64 = 201,
        kDsOrSrc2B64 = 202,
        kDsXorSrc2B64 = 203,
        kDsWriteSrc2B64 = 205,
        kDsMinSrc2F64 = 210,
        kDsMaxSrc2F64 = 211,
        kDsWriteB96 = 222,
        kDsWriteB128 = 223,
        kDsReadB96 = 254,
        kDsReadB128 = 255,
        kDsIlegal = 256,
    };

    int GetInstructionClockCount()
    {
        int iRet = 4;

        return iRet;
    }

    VIDSInstruction(OFFSET offset0, OFFSET offset1, GDS isGDS, OP op, ADDR addr, DATA data0,
        DATA data1, VDST vdst, int label_, int iGotoLabel) : DSInstruction(offset0, offset1, isGDS, addr, data0, data1, vdst, label_, iGotoLabel), op_(op)
    {
        hw_gen_ = GDT_HW_GENERATION_VOLCANICISLAND;
    }

    // Get the OP [27:23]
    OP GetOp() const { return op_; }

private:
    // DS operation.
    OP op_;
};

// DS instruction for VEGA (GFX9).
class G9DSInstruction : public DSInstruction
{
public:
    // Selector for the DS Instruction
    enum OP
    {
        kDsAddU32 = 0,
        kDsSubU32 = 1,
        kDsRsubU32 = 2,
        kDsIncU32 = 3,
        kDsDecU32 = 4,
        kMinI32 = 5,
        kDsMaxI32 = 6,
        kDsMinU32 = 7,
        kDsMaxU32 = 8,
        kDsAndB32 = 9,
        kDsOrB32 = 10,
        kDsXorB32 = 11,
        kDsMskorB32 = 12,
        kDsWriteB32 = 13,
        kDsWrite2B32 = 14,
        kDsWrite2st64B32 = 15,
        kDsCmpstB32 = 16,
        kDsCmpstF32 = 17,
        kDsMinF32 = 18,
        kDsMaxF32 = 19,
        kDsNop = 20,
        kDsAddF32 = 21,
        kDsWriteB8 = 30,
        kDsWriteB16 = 31,
        kDsAddRtnU32 = 32,
        kDsSubRtnU32 = 33,
        kDsRsubRtnU32 = 34,
        kDsIncRtnU32 = 35,
        kDsDecRtnU32 = 36,
        kDsMinRtnI32 = 37,
        kDsMaxRtnI32 = 38,
        kDsMinRtnU32 = 39,
        kDsMaxRtnU32 = 40,
        kDsAndRtnB32 = 41,
        kDsOrRtnB32 = 42,
        kDsXorRtnB32 = 43,
        kDsMskorRtnB32 = 44,
        kDsWrxchgRtnB32 = 45,
        kDsWrxchg2RtnB32 = 46,
        kDsWrxchg2st64RtnB32 = 47,
        kDsCmpstRtnB32 = 48,
        kDsCmpstRtnF32 = 49,
        kDsMinRtnF32 = 50,
        kDsMaxRtnF32 = 51,
        kDsWrapRtnB32 = 52,
        kDsAddRtnF32 = 53,
        kDsReadB32 = 54,
        kDsRead2B32 = 55,
        kDsRead2st64B32 = 56,
        kDsReadI8 = 57,
        kDsReadU8 = 58,
        kDsReadI16 = 59,
        kDsReadU16 = 60,
        kDsSwizzleB32 = 61,
        kDsPermuteB32 = 62,
        kDsBpermuteB32 = 63,
        kDsAddU64 = 64,
        kDsSubU64 = 65,
        kDsRsubU64 = 66,
        kDsIncU64 = 67,
        kDsDecU64 = 68,
        kDsMinI64 = 69,
        kDsMaxI64 = 70,
        kDsMinU64 = 71,
        kDsMaxU64 = 72,
        kDsAndB64 = 73,
        kDsOrB64 = 74,
        kDsXorB64 = 75,
        kDsMskorB64 = 76,
        kDsWriteB64 = 77,
        kDsWrite2B64 = 78,
        kDsWrite2st64B64 = 79,
        kDsCmpstB64 = 80,
        kDsCmpstF64 = 81,
        kDsMinF64 = 82,
        kDsMaxF64 = 83,
        kDsWriteB8D16Hi = 84,
        kDsWriteB16D16Hi = 85,
        kDsReadU8D16 = 86,
        kDsReadU8D16Hi = 87,
        kDsReadI8D16 = 88,
        kDsReadI8D16Hi = 89,
        kDsReadU16D16 = 90,
        kDsReadU16D16Hi = 91,
        kDsAddRtnU64 = 96,
        kDsSubRtnU64 = 97,
        kDsRsubRtnU64 = 98,
        kDsIncRtnU64 = 99,
        kDsDecRtnU64 = 100,
        kDsMinRtnI64 = 101,
        kDsMaxRtnI64 = 102,
        kDsMinRtnU64 = 103,
        kDsMaxRtnU64 = 104,
        kDsAndRtnB64 = 105,
        kDsOrRtnB64 = 106,
        kDsXorRtnB64 = 107,
        kDsMskorRtnB64 = 108,
        kDsWrxchgRtnB64 = 109,
        kDsWrxchg2RtnB64 = 110,
        kDsWrxchg2st64RtnB64 = 111,
        kDsCmpstRtnB64 = 112,
        kDsCmpstRtnF64 = 113,
        kDsMinRtnF64 = 114,
        kDsMaxRtnF64 = 115,
        kDsReadB64 = 118,
        kDsRead2B64 = 119,
        kDsRead2st64B64 = 120,
        kDsCondxchg32RtnB64 = 126,
        kDsAddSrc2U32 = 128,
        kDsSubSrc2U32 = 129,
        kDsRsubSrc2U32 = 130,
        kDsIncSrc2U32 = 131,
        kDsDecSrc2U32 = 132,
        kDsMinSrc2I32 = 133,
        kDsMaxSrc2I32 = 134,
        kDsMinSrc2U32 = 135,
        kDsMaxSrc2U32 = 136,
        kDsAndSrc2B32 = 137,
        kDsOrSrc2B32 = 138,
        kDsXorSrc2B32 = 139,
        kDsWriteSrc2B32 = 141,
        kDsMinSrc2F32 = 146,
        kDsMaxSrc2F32 = 147,
        kDsAddSrc2F32 = 149,
        kDsGwsSemaReleaseAll = 152,
        kDsGwsInit = 153,
        kDsGwsSemaV = 154,
        kDsGwsSemaBr = 155,
        kDsGwsSemaP = 156,
        kDsGwsBarrier = 157,
        kDsReadAddtidB32 = 182,
        kDsConsume = 189,
        kDsAppend = 190,
        kDsOrderedCount = 191,
        kDsAddSrc2U64 = 192,
        kDsSubSrc2U64 = 193,
        kDsRsubSrc2U64 = 194,
        kDsIncSrc2U64 = 195,
        kDsDecSrc2U64 = 196,
        kDsMinSrc2I64 = 197,
        kDsMaxSrc2I64 = 198,
        kDsMinSrc2U64 = 199,
        kDsMaxSrc2U64 = 200,
        kDsAndSrc2B64 = 201,
        kDsOrSrc2B64 = 202,
        kDsXorSrc2B64 = 203,
        kDsWriteSrc2B64 = 205,
        kDsMinSrc2F64 = 210,
        kDsMaxSrc2F64 = 211,
        kDsWriteB96 = 222,
        kDsWriteB128 = 223,
        kDsReadB96 = 254,
        kDsReadB128 = 255,
        kDsIlegal = 256,
    };

    G9DSInstruction(OFFSET offset0, OFFSET offset1, GDS is_gds, OP op, ADDR addr, DATA data0, DATA data1,
        VDST vdst, int label, int goto_label) : DSInstruction(offset0, offset1, is_gds, addr, data0, data1, vdst, label, goto_label), op_(op)
    {
        hw_gen_ = GDT_HW_GENERATION_GFX9;
    }

    // Get the OP [27:23].
    OP GetOp() const { return op_; }

private:
    // DS operation.
    OP op_;
};

#endif //__DSINSTRUCTION_H


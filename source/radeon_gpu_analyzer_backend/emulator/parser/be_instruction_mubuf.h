//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for class representing MUBUF Instruction in shader isa.
//=============================================================================

#ifndef __MUBUFINSTRUCTION_H
#define __MUBUFINSTRUCTION_H

#include "be_instruction.h"

// Untyped memory buffer operation. First word with LDS, second word non-LDS.
// Opcode :
//        OFFSET [11:0]
//        OFFEN  [12:12]
//        IDXEN  [13:13]
//        GLC    [14:14]
//        ADDR64 [15:15]
//        LDS    [16:16]
//        OP     [24:18]
//        VADDR  [39:32]
//        VDATA  [47:40]
//        SRSRC  [52:48]
//        SLC    [54:54]
//        TFE    [55:55]
//        SOFFSET [63:56]

class MUBUFInstruction : public Instruction
{
public:

#define X(SYM) SOFFSET##SYM
#define X_INIT(SYM,VAL) SOFFSET##SYM = VAL
    // Byte offset added to the memory address. Scalar or constant GPR containing the
    // base offset. This is always sent.
    enum SOFFSET
    {
#include "be_instruction_fields_generic1.h"
#include "be_instruction_fields_scalar.h"
        X(Illegal)
    };
#undef X_INIT
#undef X

    typedef short OFFSET;
    typedef char OFFEN;
    typedef char IDXEN;
    typedef char GLC;
    typedef char ADDR64;
    typedef char LDS;
    typedef char VADDR;
    typedef char VDATA;
    typedef char SRSRC;
    typedef char SLC;
    typedef char TFE;


private:
    // Unsigned byte offset. Only used when OFFEN = 0.
    OFFSET offset_;

    // If set, send VADDR as an offset. If clear, send the instruction offset stored in
    //OFFSET. Only one of these offsets can be sent.
    OFFEN offen_;

    // If set, send VADDR as an index. If clear, treat the index as zero.
    IDXEN idxen_;

    // If set, operation is globally coherent.
    GLC glc_;

    // If set, buffer address is 64-bits (base and size in resource is ignored).
    ADDR64 addr64_;

    // If set, data is read from/written to LDS memory. If unset, data is read from/written
    // to a VGPR.
    LDS lds_;

    // VGPR address source. Can carry an offset or an index.
    VADDR vaddr_;

    // Vector GPR to read/write result to.
    VDATA vdata_;

    // Scalar GPR that specifies the resource constant, in units of four SGPRs.
    SRSRC srsrc_;

    // System Level Coherent.
    SLC slc_;

    // Texture Fail Enable (for partially resident textures).
    TFE tfe_;

    // Byte offset added to the memory address. Scalar or constant GPR containing the
    //base offset. This is always sent.
    SOFFSET soffset_;

    // Registers index (soffset_).
    // Note : Relevant only if soffset_ == ScalarGPR or soffset_ == ScalarTtmp
    unsigned int ridx_;

    // MTBUF Instruction Width in bits
    static const unsigned int mubuf_instruction_width = 64;
public:

    MUBUFInstruction(OFFSET offset, OFFEN offen, IDXEN idxen, GLC glc, ADDR64 addr64, LDS lds, VADDR vaddr, VDATA vdata, SRSRC srsrc,
        SLC slc, TFE tfe, SOFFSET soffset, unsigned int ridx, Instruction::InstructionCategory inst, int label,
        int goto_label) : Instruction(mubuf_instruction_width, inst, kInstructionSetMubuf, label, goto_label),
        offset_(offset), offen_(offen), idxen_(idxen), glc_(glc), addr64_(addr64), lds_(lds),
        vaddr_(vaddr), vdata_(vdata), srsrc_(srsrc), slc_(slc), tfe_(tfe), soffset_(soffset), ridx_(ridx) {}

    ~MUBUFInstruction() = default;

    // Get the OFFSET [11:0].
    OFFSET GetOffset() const { return offset_; }

    // Get the OFFEN  [12:12].
    OFFEN GetOffen() const { return offen_; }

    // Get the IDXEN  [13:13]
    IDXEN GetIdxen() const { return idxen_; }

    // Get the GLC    [14:14].
    GLC GetGlc() const { return glc_; }

    // Get the  ADDR64 [15:15].
    ADDR64 GetADDR64() const { return addr64_; }

    // Get the LDS   [16:16].
    LDS GetLds() const { return lds_; }

    // Get the VADDR  [39:32].
    VADDR GetVaddr() const { return vaddr_; }

    // Get the VDATA  [47:40].
    VDATA GetVdata() const { return vdata_; }

    // Get the SRSRC  [52:48].
    SRSRC GetSrsrc() const { return srsrc_; }

    // Get the SLC    [54:54].
    SLC GetSlc() const { return slc_; }

    // Get the TFE    [55:55].
    TFE GetTfe() const { return tfe_; }

    // Get the SOFFSET [63:56].
    SOFFSET GetSOFFSET() const { return soffset_; }
};

class SIMUBUFInstruction : public MUBUFInstruction
{
public:
    // Selector for the MTBUF Instruction
    enum OP
    {
        //  Untyped buffer load 1 Dword with format conversion.
        kBufferLoadFormatX,
        //  Untyped buffer load 2 Dwords with format conversion.
        kBufferLoadFormatXy,
        //  Untyped buffer load 3 Dwords with format conversion.
        kBufferLoadFormatXyz,
        //  Untyped buffer load 4 Dwords with format conversion.
        kBufferLoadFormatXyzw,
        //  Untyped buffer store 1 Dword with format conversion.
        kBufferStoreFormatX,
        //  Untyped buffer store 2 Dwords with format conversion.
        kBufferStoreFormatXy,
        //  Untyped buffer store 3 Dwords with format conversion.
        kBufferStoreFormatXyz,
        //  Untyped buffer store 4 Dwords with format conversion.
        kBufferStoreFormatXyzw,
        //  Untyped buffer load unsigned byte.
        kBufferLoadUByte,
        //  Untyped buffer load signed byte.
        kBufferLoadSByte,
        //  Untyped buffer load unsigned short.
        kBufferLoadUShort,
        //  Untyped buffer load signed short.
        kBufferLoadSShort,
        //  Untyped buffer load Dword.
        kBufferLoadDword,
        //  Untyped buffer load 2 Dwords.
        kBufferLoadDwordX2,
        //  Untyped buffer load 4 Dwords.
        kBufferLoadDwordX4 = 14,

        kBufferLoadDwordX3 = 15, // CI Specific

        //  Untyped buffer store byte.
        kBufferStoreByte = 24,
        //  Untyped buffer store short.
        kBufferStoreShort = 26,
        //  Untyped buffer store Dword.
        kBufferStoreDword = 28,
        //  Untyped buffer store 2 Dwords.
        kBufferStoreDwordX2,
        //  Untyped buffer store 4 Dwords.
        kBufferStoreDwordX4 = 30,

        kBufferStoreDwordX3 = 31, // CI Specific

        //  32b. dst=src, returns previous value if glc==1.
        kBufferAtomicSwap = 48,
        //  32b, dst = (dst==cmp) ? src : dst. Returns previous
        // value if glc==1. src comes from the first data-vgpr, cmp from the second.
        kBufferAtomicCmpSwap,
        //  32b, dst += src. Returns previous value if glc==1.
        kBufferAtomicAdd,
        //  32b, dst -= src. Returns previous value if glc==1.
        kBufferAtomicSub,
        //  32b, dst = src-dst. Returns previous value if glc==1.
        kBufferAtomicRSub,
        //  32b, dst = (src < dst) ? src : dst (signed). Returns previous
        // value if glc==1.
        kBufferAtomicSmin,
        //  32b, dst = (src < dst) ? src : dst (unsigned). Returns
        // previous value if glc==1.
        kBufferAtomicUmin,
        //  32b, dst = (src > dst) ? src : dst (signed). Returns previous
        // value if glc==1.
        kBufferAtomicSmax,
        //  32b, dst = (src > dst) ? src : dst (unsigned). Returns
        // previous value if glc==1.
        kBufferAtomicUmax,
        //  32b, dst &= src. Returns previous value if glc==1.
        kBufferAtomicAnd,
        //  32b, dst |= src. Returns previous value if glc==1.
        kBufferAtomicOr,
        //  32b, dst ^= src. Returns previous value if glc==1.
        kBufferAtomicXor,
        //  32b, dst = (dst >= src) ? 0 : dst+1. Returns previous
        // value if glc==1.
        kBufferAtomicInc,
        //  32b, dst = ((dst==0 || (dst > src)) ? src : dst-1. Returns
        // previous value if glc==1.
        kBufferAtomicDec,
        //  32b , dst = (dst == cmp) ? src : dst, returns previous
        // value if glc==1. Float compare swap (handles NaN/INF/denorm). src
        // comes from the first data-vgpr; cmp from the second.
        kBufferAtomicFcmpSwap,
        //  32b , dst = (src < dst) ? src : dst,. Returns previous
        // value if glc==1. float, handles NaN/INF/denorm.
        kBufferAtomicFmin,
        //  32b , dst = (src > dst) ? src : dst, returns previous value
        // if glc==1. float, handles NaN/INF/denorm.
        kBufferAtomicFmax,
        //  64b. dst=src, returns previous value if glc==1.
        kBufferAtomicSwapX2 = 80,
        //  64b, dst = (dst==cmp) ? src : dst. Returns previous
        // value if glc==1. src comes from the first two data-vgprs, cmp from the
        // second two.
        kBufferAtomicCMPSwapX2,
        //  64b, dst += src. Returns previous value if glc==1.
        kBufferAtomicAddX2,
        //  64b, dst -= src. Returns previous value if glc==1.
        kBufferAtomicSubX2,
        //  64b, dst = src-dst. Returns previous value if
        // glc==1.
        kBufferAtomicRSubX2,
        //  64b, dst = (src < dst) ? src : dst (signed). Returns
        // previous value if glc==1.
        kBufferAtomicSminX2,
        //  64b, dst = (src < dst) ? src : dst (unsigned).
        // Returns previous value if glc==1.
        kBufferAtomicUminX2,
        //  64b, dst = (src > dst) ? src : dst (signed). Returns
        // previous value if glc==1.
        kBufferAtomicSmaxX2,
        //  64b, dst = (src > dst) ? src : dst (unsigned).
        // Returns previous value if glc==1.
        kBufferAtomicUmaxX2,
        //  64b, dst &= src. Returns previous value if glc==1.
        kBufferAtomicAndX2,
        //  64b, dst |= src. Returns previous value if glc==1.
        kBufferAtomicOrX2,
        //  64b, dst ^= src. Returns previous value if glc==1.
        kBufferAtomicXorX2,
        //  64b, dst = (dst >= src) ? 0 : dst+1. Returns previous
        // value if glc==1.
        kBufferAtomicIncX2,
        //  64b, dst = ((dst==0 || (dst > src)) ? src : dst-1.
        // Returns previous value if glc==1.
        kBufferAtomicDecX2,
        //  64b , dst = (dst == cmp) ? src : dst, returns
        // previous value if glc==1. Double compare swap (handles NaN/INF/denorm).
        // src comes from the first two data-vgprs, cmp from the second two.
        kBufferAtomicFcmpSwapX2,
        //  64b , dst = (src < dst) ? src : dst, returns previous
        // value if glc==1. Double, handles NaN/INF/denorm.
        kBufferAtomicFminX2,
        //  64b , dst = (src > dst) ? src : dst, returns previous
        // value if glc==1. Double, handles NaN/INF/denorm.
        kBufferAtomicFmaxX2,
        //  write back and invalidate the shader L1 only for lines of
        // MTYPE SC and GC. Always returns ACK to shader.
        kBufferWbinVl1Sc = 112, // SI Only

        kBufferWbinVl1Vol = 112, // CI Only

        //  write back and invalidate the shader L1. Always returns
        // ACK to shader.
        // All other values are reserved.
        kBufferWBinVl1,
        // Reserved
        kBufferReserved
    };

    // Get the OP     OP [24:18]
    OP GetOp() const { return op_; }

    SIMUBUFInstruction(OFFSET offset, OFFEN offen, IDXEN idxen, GLC glc, ADDR64 addr64, LDS lds, OP op, VADDR vaddr, VDATA vdata, SRSRC srsrc,
        SLC slc, TFE tfe, SOFFSET soffset, unsigned int ridx, Instruction::InstructionCategory inst, int label,
        int goto_label) : MUBUFInstruction(offset, offen, idxen, glc, addr64, lds, vaddr, vdata, srsrc,
            slc, tfe, soffset, ridx, inst, label, goto_label), op_(op) {}

private:
    // MTBUF operation.
    OP op_;
};

class VIMUBUFInstruction : public MUBUFInstruction
{

public:
    // Selector for the MTBUF Instruction
    enum OP
    {
        kBufferLoadFormatX = 0,
        kBufferLoadFormatXy = 1,
        kBufferLoadFormatXyz = 2,
        kBufferLoadFormatXyzw = 3,
        kBufferStoreFormatX = 4,
        kBufferStoreFormatXy = 5,
        kBufferStoreFormatXyz = 6,
        kBufferStoreFormatXyzw = 7,
        kBufferLoadFormatD16X = 8,
        kBufferLoadFormatD16Xy = 9,
        kBufferLoadFormatD16Xyz = 10,
        kBufferLoadFormatD16Xyzw = 11,
        kBufferStoreFormatD16X = 12,
        kBufferStoreFormatD16Xy = 13,
        kBufferStoreFormatD16Xyz = 14,
        kBufferStoreFormatD16Xyzw = 15,
        kBufferLoadUbyte = 16,
        kBufferLoadSbyte = 17,
        kBufferLoadUshort = 18,
        kBufferLoadSshort = 19,
        kBufferLoadDword = 20,
        kBufferLoadDwordx2 = 21,
        kBufferLoadDwordx3 = 22,
        kBufferLoadDwordx4 = 23,
        kBufferStoreByte = 24,
        kBufferStoreShort = 26,
        kBufferStoreDword = 28,
        kBufferStoreDwordx2 = 29,
        kBufferStoreDwordx3 = 30,
        kBufferStoreDwordx4 = 31,
        kBufferStoreLdsDword = 61,
        kBufferWbinvl1 = 62,
        kBufferWbinvl1Vol = 63,
        kBufferAtomicSwap = 64,
        kBufferAtomicCmpswap = 65,
        kBufferAtomicAdd = 66,
        kBufferAtomicSub = 67,
        kBufferAtomicSmin = 68,
        kBufferAtomicUmin = 69,
        kBufferAtomicSmax = 70,
        kBufferAtomicUmax = 71,
        kBufferAtomicAnd = 72,
        kBufferAtomicOr = 73,
        kBufferAtomicXor = 74,
        kBufferAtomicInc = 75,
        kBufferAtomicDec = 76,
        kBufferAtomicSwapX2 = 96,
        kBufferAtomicCmpswapX2 = 97,
        kBufferAtomicAddX2 = 98,
        kBufferAtomicSubX2 = 99,
        kBufferAtomicSminX2 = 100,
        kBufferAtomicUminX2 = 101,
        kBufferAtomicSmaxX2 = 102,
        kBufferAtomicUmaxX2 = 103,
        kBufferAtomicAndX2 = 104,
        kBufferAtomicOrX2 = 105,
        kBufferAtomicXorX2 = 106,
        kBufferAtomicIncX2 = 107,
        kBufferAtomicDecX2 = 108,
        kBufferIlegal = 109,
    };

    // Get the OP     OP [24:18]
    OP GetOp() const { return op_; }

    // ctor
    VIMUBUFInstruction(OFFSET offset, OFFEN offen, IDXEN idxen, GLC glc, ADDR64 addr64, LDS lds, OP op, VADDR vaddr, VDATA vdata, SRSRC srsrc,
        SLC slc, TFE tfe, SOFFSET soffset, unsigned int ridx, Instruction::InstructionCategory inst, int label_, int iGotoLabel) : MUBUFInstruction(offset, offen, idxen, glc, addr64, lds, vaddr, vdata, srsrc,
            slc, tfe, soffset, ridx, inst, label_, iGotoLabel), op_(op)
    {
        hw_gen_ = GDT_HW_GENERATION_VOLCANICISLAND;
    }

private:

    // MTBUF operation.
    OP op_;

};


// MUBUF instructions for VEGA (GFX9)
class G9MUBUFInstruction : public MUBUFInstruction
{

public:
    // Selector for the MTBUF Instruction
    enum OP
    {
        kBufferLoadFormatX = 0,
        kBufferLoadFormatXy = 1,
        kBufferLoadFormatXyz = 2,
        kBufferLoadFormatXyzw = 3,
        kBufferStoreFormatX = 4,
        kBufferStoreFormatXy = 5,
        kBufferStoreFormatXyz = 6,
        kBufferStoreFormatXyzw = 7,
        kBufferLoadFormatD16X = 8,
        kBufferLoadFormatD16Xy = 9,
        kBufferLoadFormatD16Xyz = 10,
        kBufferLoadFormatD16Xyzw = 11,
        kBufferStoreFormatD16X = 12,
        kBufferStoreFormatD16Xy = 13,
        kBufferStoreFormatD16Xyz = 14,
        kBufferStoreFormatD16Xyzw = 15,
        kBufferLoadUbyte = 16,
        kBufferLoadSbyte = 17,
        kBufferLoadUshort = 18,
        kBufferLoadSshort = 19,
        kBufferLoadDword = 20,
        kBufferLoadDwordx2 = 21,
        kBufferLoadDwordx3 = 22,
        kBufferLoadDwordx4 = 23,
        kBufferStoreByte = 24,
        kBufferStoreShort = 26,
        kBufferStoreDword = 28,
        kBufferStoreDwordx2 = 29,
        kBufferStoreDwordx3 = 30,
        kBufferStoreDwordx4 = 31,
        kBufferLoadUbyteD16 = 32,
        kBufferLoadUbyteD16Hi = 33,
        kBufferLoadSbyteD16 = 34,
        kBufferLoadSbyteD16Hi = 35,
        kBufferLoadShortD16 = 36,
        kBufferLoadShortD16Hi = 37,
        kBufferLoadFormatD16HiX = 38,
        kBufferStoreFormatD16HiX = 39,
        kBufferStoreLdsDword = 61,
        kBufferWbinvl1 = 62,
        kBufferWbinvl1Vol = 63,
        kBufferAtomicSwap = 64,
        kBufferAtomicCmpswap = 65,
        kBufferAtomicAdd = 66,
        kBufferAtomicSub = 67,
        kBufferAtomicSmin = 68,
        kBufferAtomicUmin = 69,
        kBufferAtomicSmax = 70,
        kBufferAtomicUmax = 71,
        kBufferAtomicAnd = 72,
        kBufferAtomicOr = 73,
        kBufferAtomicXor = 74,
        kBufferAtomicInc = 75,
        kBufferAtomicDec = 76,
        kBufferAtomicSwapX2 = 96,
        kBufferAtomicCmpswapX2 = 97,
        kBufferAtomicAddX2 = 98,
        kBufferAtomicSubX2 = 99,
        kBufferAtomicSminX2 = 100,
        kBufferAtomicUminX2 = 101,
        kBufferAtomicSmaxX2 = 102,
        kBufferAtomicUmaxX2 = 103,
        kBufferAtomicAndX2 = 104,
        kBufferAtomicOrX2 = 105,
        kBufferAtomicXorX2 = 106,
        kBufferAtomicIncX2 = 107,
        kBufferAtomicDecX2 = 108,
        kBufferIllegal = 109
    };

    // Get the OP     OP [24:18]
    OP GetOp() const { return op_; }

    // ctor
    G9MUBUFInstruction(OFFSET offset, OFFEN offen, IDXEN idxen, GLC glc, ADDR64 addr64, LDS lds, OP op, VADDR vaddr, VDATA vdata, SRSRC srsrc,
        SLC slc, TFE tfe, SOFFSET soffset, unsigned int ridx, Instruction::InstructionCategory inst, int label_, int iGotoLabel) : MUBUFInstruction(offset, offen, idxen, glc, addr64, lds, vaddr, vdata, srsrc,
            slc, tfe, soffset, ridx, inst, label_, iGotoLabel), op_(op)
    {
        hw_gen_ = GDT_HW_GENERATION_GFX9;
    }

private:

    // MTBUF operation.
    OP op_;
};


#endif //__MUBUFINSTRUCTION_H


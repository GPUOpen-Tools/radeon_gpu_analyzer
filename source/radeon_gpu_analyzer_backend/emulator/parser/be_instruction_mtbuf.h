//=================================================================
// Copyright 2020 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

#ifndef __MTBUFINSTRUCTION_H
#define __MTBUFINSTRUCTION_H

#include "be_instruction.h"

// Typed memory buffer operation. Two words
// Opcode :
//        OFFSET [11:0]
//        OFFEN  [12:12]
//        IDXEN  [13:13]
//        GLC    [14:14]
//        ADDR64 [15:15]
//        OP     [18:16]
//        DFMT   [22:19]
//        NFMT   [25:23]
//        VADDR  [39:32]
//        VDATA  [47:40]
//        SRSRC  [52:48]
//        SLC    [54:54]
//        TFE    [55:55]
//        SOFFSET [63:56]

class MTBUFInstruction : public Instruction
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
    typedef char DFMT;
    typedef char NFMT;
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

    // Data format for typed buffer.
    DFMT dmft_;

    // Number format for typed buffer.
    NFMT nmft_;

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

    // MTBUF Instruction Width in bits.
    static const unsigned int mtbuf_instruction_width = 64;
public:
    MTBUFInstruction(OFFSET offset, OFFEN offen, IDXEN idxen, GLC glc, ADDR64 addr64, DFMT dmft, NFMT nmft, VADDR vaddr, VDATA vdata, SRSRC srsrc,
                     SLC slc, TFE tfe, SOFFSET soffset, unsigned int ridx, Instruction::InstructionCategory inst, int label, int goto_label): Instruction(mtbuf_instruction_width, inst, kInstructionSetMtbuf, label, goto_label),
        offset_(offset), offen_(offen), idxen_(idxen), glc_(glc), addr64_(addr64), dmft_(dmft), nmft_(nmft),
        vaddr_(vaddr), vdata_(vdata), srsrc_(srsrc), slc_(slc), tfe_(tfe), soffset_(soffset), ridx_(ridx) {}

    ~MTBUFInstruction() = default;

    // Get the OFFSET [11:0].
    OFFSET GetOffset() const { return offset_; }

    // Get the OFFEN  [12:12].
    OFFEN GetOffen() const { return offen_; }

    // Get the IDXEN  [13:13].
    IDXEN GetIdxen() const { return idxen_; }

    // Get the GLC    [14:14].
    GLC GetGlc() const { return glc_; }

    // Get the  ADDR64 [15:15].
    ADDR64 GetADDR64() const { return addr64_; }

    // Get the DFMT   [22:19].
    DFMT GetDfmt() const { return dmft_; }

    // Get the NFMT   [25:23].
    NFMT GetNfmt() const { return nmft_; }

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

class SIMTBUFInstruction : public MTBUFInstruction
{
public:
    // Selector for the MTBUF Instruction
    enum OP
    {
        //  Untyped buffer load 1 Dword with format conversion.
        kTbufferLoadFormatX,
        //  Untyped buffer load 2 Dwords with format conversion.
        kTbufferLoadFormatXy,
        //  Untyped buffer load 3 Dwords with format conversion.
        kTbufferLoadFormatXyz,
        //  Untyped buffer load 4 Dwords with format conversion.
        kTbufferLoadFormatXyzw,
        //  Untyped buffer store 1 Dword with format conversion.
        kTbufferStoreFormatX,
        //  Untyped buffer store 2 Dwords with format conversion.
        kTbufferStoreFormatXy,
        //  Untyped buffer store 3 Dwords with format conversion.
        kTbufferStoreFormatXyz,
        //  Untyped buffer store 4 Dwords with format conversion.
        // All other values are reserved.
        kTbufferStoreFormatXyzw,
        // Reserved
        kTbufferRESERVED
    };

    // Get the OP     [18:16]
    OP GetOp() const { return op_; }

    // ctor
    SIMTBUFInstruction(OFFSET offset, OFFEN offen, IDXEN idxen, GLC glc, ADDR64 addr64, OP op, DFMT dmft, NFMT nmft, VADDR vaddr, VDATA vdata, SRSRC srsrc,
        SLC slc, TFE tfe, SOFFSET soffset, unsigned int ridx, Instruction::InstructionCategory inst, int label,
        int goto_label) : MTBUFInstruction(offset, offen, idxen, glc, addr64, dmft, nmft, vaddr, vdata, srsrc,
            slc, tfe, soffset, ridx, inst, label, goto_label), op_(op) {}

private:
    // MTBUF operation.
    OP op_;
};

class VIMTBUFInstruction : public MTBUFInstruction
{
public:
    // Selector for the MTBUF Instruction
    enum OP
    {
        kTbufferLoadFormatX = 0,
        kTbufferLoadFormatXy = 1,
        kTbufferLoadFormatXyz = 2,
        kTbufferLoadFormatXyzw = 3,
        kTbufferStoreFormatX = 4,
        kTbufferStoreFormatXy = 5,
        kTbufferStoreFormatXyz = 6,
        kTbufferStoreFormatXyzw = 7,
        kTbufferLoadFormatD16X = 8,
        kTbufferLoadFormatD16Xy = 9,
        kTbufferLoadFormatD16Xyz = 10,
        kTbufferLoadFormatD16Xyzw = 11,
        kTbufferStoreFormatD16X = 12,
        kTbufferStoreFormatD16Xy = 13,
        kTbufferStoreFormatD16Xyz = 14,
        kTbufferStoreFormatD16Xyzw = 15,
        kTbufferIllegal = 16,
    };

    // Get the OP     [18:16].
    OP GetOp() const { return op_; }

    VIMTBUFInstruction(OFFSET offset, OFFEN offen, IDXEN idxen, GLC glc, ADDR64 addr64, OP op, DFMT dmft, NFMT nmft, VADDR vaddr, VDATA vdata, SRSRC srsrc,
        SLC slc, TFE tfe, SOFFSET soffset, unsigned int ridx, Instruction::InstructionCategory inst,
        int label, int goto_label) : MTBUFInstruction(offset, offen, idxen, glc, addr64, dmft, nmft, vaddr, vdata, srsrc,
            slc, tfe, soffset, ridx, inst, label, goto_label), op_(op)
    {
        hw_gen_ = GDT_HW_GENERATION_VOLCANICISLAND;
    }

private:
    // MTBUF operation.
    OP op_;
};
#endif //__MTBUFINSTRUCTION_H


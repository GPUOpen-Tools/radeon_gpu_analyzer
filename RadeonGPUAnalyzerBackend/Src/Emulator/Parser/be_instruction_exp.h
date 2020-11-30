//=================================================================
// Copyright 2020 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

#ifndef __EXPINSTRUCTION_H
#define __EXPINSTRUCTION_H

#include "be_instruction.h"
// Export (output) pixel color, pixel depth, vertex position, or vertex parameter data. Two words.
// Opcode :
//        EN [3:0]
//        TGT [9:4]
//        COMPR [10:10]
//        DONE [11:11]
//        VM [12:12]
//        reserved [25:13]
//        VSRC0 [39:32]
//        VSRC1 [47:40]
//        VSRC2 [55:48]
//        VSRC3 [63:56]

class EXPInstruction : public Instruction
{
public:

#define X(SYM) TGT##SYM
#define X_INIT(SYM,VAL) TGT##SYM = VAL
    // Export target based on the enumeration below.
    enum TGT
    {
        // Output to color MRT 0. Increment from here for additional MRTs.
        //There are EXP_NUM_MRT MRTs in total.
        X_INIT(ExpMRTMin, 0),
        X(ExpMRT),
        X_INIT(ExpMRTMax, 7),
        // Output to Z.
        X(ExpMRTZ),
        // Output to NULL.
        X(ExpNULL),
        // Output to position 0. Increment from here for additional positions.
        // There are EXP_NUM_POS positions in total.
        X_INIT(ExpPosMin, 12),
        X(ExpPos),
        X_INIT(ExpPosMax, 15),
        // Output to parameter 0. Increment from here for additional parameters.
        // There are EXP_NUM_PARAM parameters in total.
        X_INIT(ExpParamMin, 32),
        X(ExpParam),
        X_INIT(ExpParamMax, 63),
        // All other values are reserved.
        X(ExpReserved)
    };
#undef X_INIT
#undef X

    // This bitmask determines which VSRC registers export data.
    // When COMPR is 0: VSRC0 only exports data when en[0] is set to 1; VSRC1 when en[1], VSRC2 when en[2], and VSRC3 when en[3].
    // When COMPR is 1: VSRC0 contains two 16-bit data and only exports when en[0] is set to 1;
    // VSRC1 only exports when en[2] is set to 1; en[1] and en[3] are ignored when COMPR is 1.
    typedef char EN;

    // Boolean. If true, data is exported in float16 format; if false, data is 32 bit.
    typedef char COMPR;

    // If set, this is the last export of a given type. If this is set for a colour export (PS only),
    // then the valid mask must be present in the EXEC register.
    typedef char DONE;

    // Mask contains valid-mask when set; otherwise, mask is just write-mask. Used
    // only for pixel(mrt) exports.
    typedef char VM;

    // VGPR of the data source to export.
    typedef char VSRC;
private:
    // This bitmask determines which VSRC registers export data.
    EN en_;

    // Export target based on the enumeration below.
    TGT tgt_;

    // Boolean. If true, data is exported in float16 format; if false, data is 32 bit.
    COMPR compr_;

    // If set, this is the last export of a given type. If this is set for a colour export (PS only),
    // then the valid mask must be present in the EXEC register.
    DONE done_;

    // Mask contains valid-mask when set; otherwise, mask is just write-mask. Used
    // only for pixel(mrt) exports.
    VM vm_;

    // VGPR of the first/second/third/fourth data to export.
    VSRC vsrc_[4];

    // EXP Instruction Width in bits
    static const unsigned int exp_instruction_width_ = 64;
public:
    EXPInstruction(EN en, TGT tgt, COMPR compr, DONE done, VM vm, VSRC vsrc0, VSRC vsrc1, VSRC vsrc2, VSRC vsrc3,
        int label, int goto_label): Instruction(exp_instruction_width_, InstructionCategory::kExport, kInstructionSetExp, label, goto_label),
        en_(en), tgt_(tgt), compr_(compr), done_(done), vm_(vm)
    {
        vsrc_[0] = vsrc0;
        vsrc_[1] = vsrc1;
        vsrc_[2] = vsrc2;
        vsrc_[3] = vsrc3;
    }

    ~EXPInstruction() = default;

    // Get the EN [3:0].
    EN GetEn() const { return en_; }

    // Get the TGT [9:4].
    TGT GetTgt() const { return tgt_; }

    // Get the COMPR [10:10].
    COMPR GetCompr() const { return compr_; }

    // Get the DONE [11:11].
    DONE GetDone() const { return done_; }

    // Get the VM [12:12].
    VM GetVm() const { return vm_; }

    // Get the VSRC based on the given index.
    VSRC GetVsrc(const unsigned int vsrc_index) const { return vsrc_[vsrc_index];}
};
#endif //__EXPINSTRUCTION_H


//=================================================================
// Copyright 2020 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

#ifndef __VINTRPINSTRUCTION_H
#define __VINTRPINSTRUCTION_H

#include "be_instruction.h"
// Interpolate data for the pixel shader.
// Opcode :
//        VSRC [7:0]
//        ATTRCHAN [9:8]
//        ATTR [15:10]
//        OP [17:16]
//        VDST [25:18]

class VINTRPInstruction : public Instruction
{
public:


    typedef char VSRC;
    typedef char ATTRCHAN;
    typedef char ATTR;
    typedef char VDST;

private:
    // Vector General-Purpose Registers (VGPR) containing the i/j coordinate by which
    //to multiply one of the parameter components.
    VSRC vsrc_;

    // Attribute component to interpolate.
    ATTRCHAN attrchan_;

    // Attribute to interpolate.
    ATTR attr_;

    // Vector General-Purpose Registers VGPR [255:0] to which results are written, and,
    //optionally, from which they are read when accumulating results.
    VDST vdst_;

    // VINTRP Instruction Width in bits
    static const unsigned int vsrc_instruction_width_ = 32;
public:

    VINTRPInstruction(VSRC vsrc, ATTRCHAN attrchan, ATTR attr, VDST vdst, int label,
        int goto_label): Instruction(vsrc_instruction_width_, InstructionCategory::kVectorAlu, kInstructionSetVintrp, label, goto_label),
        vsrc_(vsrc), attrchan_(attrchan), attr_(attr), vdst_(vdst) {}

    ~VINTRPInstruction() = default;

    // Get the VSRC [7:0]
    VSRC GetVsrc() const { return vsrc_; }

    // Get the ATTRCHAN [9:8]
    ATTRCHAN GetAttrchan() const { return attrchan_; }

    // Get the ATTR [15:10]
    ATTR GetAttr() const { return attr_; }

    // Get the VDST
    VDST GetVDST() const { return vdst_; }
};

class SIVINTRPInstruction : public VINTRPInstruction
{
public:
    // Selector for the VINTRP Instruction
    enum OP
    {
        //  D = P10 * S + P0; parameter interpolation.
        KInterpP1F32,
        //  D = P20 * S + D; parameter interpolation.
        kVinterpP2F32,
        //  D = {P10,P20,P0}[S]; parameter load.
        kVInterpMovF32,
        // Reserved
        kVinterpReserved
    };

    // Get the OP [17:16]
    OP GetOp() const { return op_; }

    SIVINTRPInstruction(VSRC vsrc, ATTRCHAN attrchan, ATTR attr, OP op, VDST vdst,
        int label, int goto_label): VINTRPInstruction(vsrc, attrchan, attr, vdst, label, goto_label), op_(op) {}

private:
    // VINTRP operation.
    OP op_;
};

class VIVINTRPInstruction : public VINTRPInstruction
{
public:
    // Selector for the VINTRP Instruction
    enum OP
    {
        //  D = P10 * S + P0; parameter interpolation.
        kVinterpP1F32,
        //  D = P20 * S + D; parameter interpolation.
        kVinterpP2F32,
        //  D = {P10,P20,P0}[S]; parameter load.
        kVinterpMovF32,
        // Illegal
        kVIllegal,
    };

    // Get the OP [17:16].
    OP GetOp() const { return op_; }

    VIVINTRPInstruction(VSRC vsrc, ATTRCHAN attrchan, ATTR attr, OP op, VDST vdst,
        int label, int goto_label): VINTRPInstruction(vsrc, attrchan, attr, vdst, label, goto_label), op_(op)
    {
        hw_gen_ = GDT_HW_GENERATION_VOLCANICISLAND;
    }

private:
    // VINTRP operation.
    OP op_;
};

#endif //__VINTRPINSTRUCTION_H


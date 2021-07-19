//=================================================================
// Copyright 2020 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

#ifndef __SOPKINSTRUCTION_H
#define __SOPKINSTRUCTION_H

#include "be_instruction.h"

// This is a scalar instruction with one inline constant input and one output.
// Opcode :
//        SIMM16 [15:0]
//        SDST [22:16]
//        OP [27:23]

class SOPKInstruction : public Instruction
{
public:

#define X(SYM) SIMM16##SYM
#define X_INIT(SYM,VAL) SIMM16##SYM = VAL
    // Destination for SSRC
    enum SIMM16
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
    // 16-bit integer input for opcode. Signedness is determined by opcode..
    SIMM16 simm16_;

    // Registers index (simm16_).
    // Note : Relevant only if simm16_ == ScalarGPR or simm16_ == ScalarTtmp
    unsigned int simm16_ridx_;

    // Scalar destination for instruction..
    SDST m_sdst;

    // Registers index (dst_).
    // Note : Relevant only if dst_ == ScalarGPR or dst_ == ScalarTtmp
    unsigned int sdst_ridx_;

    // SOP1 Instruction Width in bits
    static const unsigned int sopk_instruction_width = 32;
public:
    SOPKInstruction(SIMM16 simm16, SDST sdst, unsigned int simm16_ridx, unsigned int sdst_ridx, int label, int goto_label) :
        Instruction(sopk_instruction_width, InstructionCategory::kScalarAlu, kInstructionSetSopk, label, goto_label),
        simm16_(simm16), simm16_ridx_(simm16_ridx), m_sdst(sdst), sdst_ridx_(sdst_ridx) { }

    ~SOPKInstruction() = default;

    // Get the SIMM16 [15:0]
    SIMM16 GetSiMm16() const { return simm16_; }

    // Get the SDST [22:16]
    SDST GetSdst() const { return m_sdst; }

    // Get the (scalar) register`s index.
    // Note : Relevant only if simm16_ridx_ == ScalarGPR or simm16_ridx_ == ScalarTtmp
    unsigned int GetSimm16Ridx() const { return simm16_ridx_; }

    // Get the (scalar) register`s index.
    // Note : Relevant only if dst_ == ScalarGPR or dst_ == ScalarTtmp
    unsigned int GetSdstRidx() const { return sdst_ridx_; }
};

class SISOPKInstruction : public SOPKInstruction
{
public:
    // Selector for the SOPK Instruction
    enum OP
    {
        //D.i = signext(SIMM16).
        kMovkI32,
        //  if (SCC) D.i = signext(SIMM16); else NOP.
        kCMovkI32 = 2,
        //  SCC = (D.i == signext(SIMM16).
        kCmpkEQI32,
        //  SCC = (D.i != signext(SIMM16).
        kCmpkLGI32,
        //  SCC = (D.i != signext(SIMM16)).
        kCmpkGTI32,
        //  SCC = (D.i >= signext(SIMM16)).
        kCmpkGEI32,
        //  SCC = (D.i < signext(SIMM16)).
        kCmpkLTI32,
        //  SCC = (D.i <= signext(SIMM16)).
        kCmpkLEI32,
        //  SCC = (D.u == SIMM16).
        kCmpkEQ_U32,
        //  SCC = (D.u != SIMM16).
        kCmpkLG_U32,
        //  SCC = (D.u > SIMM16).
        kCmpkGT_U32,
        //  SCC = (D.u >= SIMM16).
        kCmpkGE_U32,
        //  SCC = (D.u < SIMM16).
        kCmpkLT_U32,
        //  D.u = SCC = (D.u <= SIMM16).
        kCmpkLE_U32,
        //  D.i = D.i + signext(SIMM16). SCC = overflow.
        kAddkI32,
        //  D.i = D.i * signext(SIMM16). SCC = overflow.
        kMulkI32,
        //  Conditional branch using branch-stack.
        // Arg0(sdst) = compare mask (VCC or any SGPR), SIMM16 = signed DWORD
        // branch offset relative to next instruction.
        kCbranchiFork,
        //  D.u = hardware register. Read some or all of a hardware register
        // into the LSBs of D. SIMM16 = {size[4:0], offset[4:0], hwRegId[5:0]}; offset
        // is 0�31, size is 1�32.
        kGetregB32,
        //  hardware register = D.u. Write some or all of the LSBs of D
        // into a hardware register (note that D is a source SGPR).
        // SIMM16 = {size[4:0], offset[4:0], hwRegId[5:0]}; offset is 0�31, size is 1�32.
        kSetregB32,
        //  This instruction uses a 32-bit literal constant. Write
        // some or all of the LSBs of IMM32 into a hardware register.
        // SIMM16 = {size[4:0], offset[4:0], hwRegId[5:0]}; offset is 0�31, size is 1�32.
        // All other values are reserved.
        kSetregimm32B32 = 21,
        // Reserved
        kReserved
    };

    // Get the OP [27:23].
    OP GetOp() const { return op_; }

    SISOPKInstruction(SIMM16 simm16, OP op, SDST sdst, unsigned int simm16_ridx, unsigned int sdst_ridx, int label,
        int goto_label) : SOPKInstruction(simm16, sdst, simm16_ridx, sdst_ridx, label, goto_label), op_(op) { }

private:
    // SOPK operation.
    OP op_;
};

class VISOPKInstruction : public SOPKInstruction
{
public:

    // Selector for the SOPK Instruction
    enum OP
    {
        kMovkI32 = 0,
        kCmovkI32 = 1,
        kCmpkEqI32 = 2,
        kCmpkLgI32 = 3,
        kCmpkGtI32 = 4,
        kCmpkGeI32 = 5,
        kCmpkLtI32 = 6,
        kCmpkLeI32 = 7,
        kCmpkEqU32 = 8,
        kCmpkLgU32 = 9,
        kCmpkGtU32 = 10,
        kCmpkGeU32 = 11,
        kCmpkLtU32 = 12,
        kCmpkLeU32 = 13,
        kAddkI32 = 14,
        kMulkI32 = 15,
        kCbranchIFork = 16,
        kGetregB32 = 17,
        kSetregB32 = 18,
        kSetregImm32B32 = 20,
        // Illegal
        kIllegal = 21,
    };

    // Get the OP [27:23]
    OP GetOp() const { return op_; }

    // ctor
    VISOPKInstruction(SIMM16 simm16, OP op, SDST sdst, unsigned int simm16Ridx, unsigned int sdstRidx, int label_, int iGotoLabel) : SOPKInstruction(simm16, sdst, simm16Ridx, sdstRidx, label_, iGotoLabel), op_(op)
    {
        hw_gen_ = GDT_HW_GENERATION_VOLCANICISLAND;
    }

private:
    // SOPK operation.
    OP op_;


};

// VEGA (GFX9)
class G9SOPKInstruction : public SOPKInstruction
{
public:
    enum OP
    {
        kMovkI32 = 0,
        kCmovkI32 = 1,
        kCmpkEqI32 = 2,
        kCmpkLgI32 = 3,
        kCmpkGtI32 = 4,
        kCmpkGeI32 = 5,
        kCmpkLtI32 = 6,
        kCmpkLeI32 = 7,
        kCmpkEqU32 = 8,
        kCmpkLgU32 = 9,
        kCmpkGtU32 = 10,
        kCmpkGeU32 = 11,
        kCmpkLtU32 = 12,
        kCmpkLeU32 = 13,
        kAddkI32 = 14,
        kMulkI32 = 15,
        kCbranchIFork = 16,
        kGetregB32 = 17,
        kSetregB32 = 18,
        kGetregRegrd_b32 = 19,
        kSetregImm32_b32 = 20,
        kCallB64 = 21,
        // Illegal
        kIllegal = 22
    };

    // Get the OP [27:23]
    OP GetOp() const { return op_; }

    // ctor
    G9SOPKInstruction(SIMM16 simm16, OP op, SDST sdst, unsigned int simm16Ridx, unsigned int sdstRidx, int label_, int iGotoLabel)
        : SOPKInstruction(simm16, sdst, simm16Ridx, sdstRidx, label_, iGotoLabel), op_(op)
    {
        hw_gen_ = GDT_HW_GENERATION_GFX9;
    }

private:
    // SOPK operation.
    OP op_;
};

#endif //__SOPKINSTRUCTION_H


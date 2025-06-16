//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for class representing SMRD Instruction in shader isa.
//=============================================================================

#ifndef __SMRDINSTRUCTION_H
#define __SMRDINSTRUCTION_H

#include "be_instruction.h"
// Scalar instruction performing a memory read from L1 (constant) memory.
// Opcode :
//        OFFSET [7:0]
//        IMM    [8:8]
//        SBASE  [14:9]
//        SDST   [21:15]
//        OP     [26:22]
class SMRDInstruction : public Instruction
{
public:

#define X(SYM) SDST##SYM
#define X_INIT(SYM,VAL) SDST##SYM = VAL
    // Destination for scalar memory read instruction
    enum SDST
    {
#include "be_instruction_fields_generic1.h"
        X(Illegal)
    };
#undef X_INIT
#undef X

    typedef char OFFSET;
    typedef bool IMM;
    typedef unsigned char SBASE;
private:
    //Unsigned eight-bit Dword offset to the address specified in SBASE.
    OFFSET offset_;

    // IMM = 0 (false): Specifies an SGPR address that supplies a Dword offset for the
    // memory operation
    // IMM = 1 (true): Specifies an 8-bit unsigned Dword offset.
    IMM imm_;

    // Bits [6:1] of an aligned pair of SGPRs specifying {size[16], base[48]}, where base
    // and size are in Dword units. The low-order bits are in the first SGPR.
    SBASE sbase_;

    // Destination for instruction.
    SDST dst_;

    // Registers index (dst_).
    // Note : Relevant only if dst_ == ScalarGPR or dst_ == ScalarTtmp
    unsigned int sridx_;

    // SMRD Instruction Width in bits
    static const unsigned int smrd_instruction_width = 32;

public:
    SMRDInstruction(OFFSET offset, IMM imm, SBASE sbase, SDST sdst, unsigned int sridx, int label, int goto_label) :
        Instruction(smrd_instruction_width, InstructionCategory::kScalarMemoryRead, kInstructionSetSmrd, label, goto_label), offset_(offset),
        imm_(imm), sbase_(sbase), dst_(sdst), sridx_(sridx) {}

    ~SMRDInstruction() = default;

    // Get the OFFSET [7:0].
    OFFSET GetOffset() const { return offset_; }

    // Get the IMM    [8:8].
    IMM GetImm() const { return imm_; }

    // Get the SBASE  [14:9]
    SBASE GetSbase() const { return sbase_;}

    // Get the SDST   [21:15]
    SDST GetSdst() const { return dst_; }

    // Get the (scalar) register`s index.
    // Note : Relevant only if dst_ == ScalarGPR or dst_ == ScalarTtmp
    unsigned int GetSRidx() const { return sridx_; }
};

class SISMRDInstruction : public SMRDInstruction
{
public:
    // Selector for the scalar memory read instruction.
    enum OP
    {
        //  Read from read-only constant memory.
        kLoadDword = 0,
        //  Read from read-only constant memory.
        kLoadDwordX2,
        //  Read from read-only constant memory.
        kLoadDwordX4,
        //  Read from read-only constant memory.
        kLoadDwordX8,
        //  Read from read-only constant memory.
        kLoadDwordX16 = 4,
        //  Read from read-only constant memory.
        kBufferLoadDword = 8,
        //  Read from read-only constant memory.
        kBufferLoadDwordX2,
        //  Read from read-only constant memory.
        kBufferLoadDwordX4,
        //  Read from read-only constant memory.
        kBufferLoadDwordX8,
        //  Read from read-only constant memory.
        kBufferLoadDwordX16 = 12,

        kDcacheInvVol = 29, // CI Specific

        //  Return current 64-bit timestamp.
        kMemtime = 30,
        //  Invalidate entire L1 K cache.
        // All other values are reserved.
        kDcacheInv = 31,
        // Reserved
        kReserved
    };

    // Get the OP     [26:22]
    OP GetOp() const { return op_; }

    SISMRDInstruction(OFFSET offset, IMM imm, SBASE sbase, SDST sdst, unsigned int sridx, OP op,
        int label, int goto_label) : SMRDInstruction(offset, imm, sbase, sdst, sridx, label, goto_label), op_(op) {}

private:
    // SMRD operation.
    OP op_;
};

class VISMEMInstruction : public SMRDInstruction
{
public:
    // Selector for the scalar memory read instruction.
    enum OP
    {
        kLoadDword = 0,
        kLoadDwordx2 = 1,
        kLoadDwordx4 = 2,
        kLoadDwordx8 = 3,
        kLoadDwordx16 = 4,
        kBufferLoadDword = 8,
        kBufferLoadDwordx2 = 9,
        kBufferLoadDwordx4 = 10,
        kBufferLoadDwordx8 = 11,
        kBufferLoadDwordx16 = 12,
        kStoreDword = 16,
        kStoreDwordx2 = 17,
        kStoreDwordx4 = 18,
        kBufferStoreDword = 24,
        kBufferStoreDwordx2 = 25,
        kBufferStoreDwordx4 = 26,
        kDcacheInv = 32,
        kDcacheWb = 33,
        kDcacheInvVol = 34,
        kDcacheWbVol = 35,
        kMemtime = 36,
        kMemrealtime = 37,
        kAtcProbe = 38,
        kAtcProbe_buffer = 39,
        // Illegal
        kIllegal = 40,
    };

    // Get the OP     [26:22].
    OP GetOp() const { return op_; }

    VISMEMInstruction(OFFSET offset, IMM imm, SBASE sbase, SDST sdst, unsigned int sridx,
        OP op, int label, int goto_label) : SMRDInstruction(offset, imm, sbase, sdst, sridx, label, goto_label), op_(op)
    {
        hw_gen_ = GDT_HW_GENERATION_VOLCANICISLAND;
    }

private:
    // SMRD operation.
    OP op_;
};

#endif //__SMRDINSTRUCTION_H


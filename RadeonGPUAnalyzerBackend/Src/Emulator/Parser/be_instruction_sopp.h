//=================================================================
// Copyright 2020 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

#ifndef __SOPPINSTRUCTION_H
#define __SOPPINSTRUCTION_H

#include "be_instruction.h"
// Scalar instruction taking one inline constant input and performing a special operation
// (for example: jump).
// Opcode :
//        SIMM16 [15:0]
//        OP [22:16]

class SOPPInstruction : public Instruction
{
public:

    typedef short SIMM16;

private:
    // 16-bit integer input for opcode. Signedness is determined by opcode.
    SIMM16 simm16_;

    // SOPP Instruction Width in bits
    static const unsigned int sopp_instruction_width_ = 32;
public:
    SOPPInstruction(SIMM16 simm16, int label, int goto_label) : Instruction(sopp_instruction_width_, InstructionCategory::kScalarAlu, kInstructionSetSopp, label, goto_label), simm16_(simm16) {}

    // dtor
    ~SOPPInstruction() = default;

    // Get the SIMM16 [15:0]
    SIMM16 GetSimm16() const { return simm16_; }
};

class SISOPPInstruction : public SOPPInstruction
{
public:
    // Selector for the SOPP Instruction
    enum OP
    {
        //  do nothing. Repeat NOP 1..8 times based on SIMM16[2:0]. 0 = 1 time,= 8 times.,
        kNop,
        //  end of program; terminate wavefront.
        kEndpgm,
        //  PC = PC + signext(SIMM16 * 4) + 4.
        kBranch,
        //  if(SCC == 0) then PC = PC + signext(SIMM16 * 4) + 4; else
        // nop.
        kCBranchScc0 = 4,
        //  if(SCC == 1) then PC = PC + signext(SIMM16 * 4) + 4; else
        // nop.
        kCBranchScc1,
        //  if(VCC == 0) then PC = PC + signext(SIMM16 * 4) + 4; else
        // nop.
        kCBranchVccz,
        //  if(VCC != 0) then PC = PC + signext(SIMM16 * 4) + 4; else
        // nop.
        kCBranchVccnz,
        //  if(EXEC == 0) then PC = PC + signext(SIMM16 * 4) + 4;
        // else nop.
        kCBranchExecz,
        //  if(EXEC != 0) then PC = PC + signext(SIMM16 * 4) + 4;
        // else nop.
        kCBranchExecnz,
        //  Sync waves within a thread group.
        kBarrier,
        //  Wait for count of outstanding lds, vector-memory and
        // export/vmem-write-data to be at or below the specified levels. simm16[3:0] =
        // vmcount, simm16[6:4] = export/mem-write-data count, simm16[12:8] =
        // LGKM_cnt (scalar-mem/GDS/LDS count).
        kWaitcnt = 12,
        //  set HALT bit to value of SIMM16[0]. 1=halt, 0=resume. Halt is
        // ignored while priv=1.
        kSethalt,
        //  Cause a wave to sleep for approximately 64*SIMM16[2:0] clocks.
        kSleep,
        //  User settable wave priority. 0 = lowest, 3 = highest.
        kSetprio,
        //  Send a message.
        kSendmsg,
        //  Send a message and then HALT.
        kSendmsghalt,
        //  Enter the trap handler. TrapID = SIMM16[7:0]. Wait for all instructions
        // to complete, save {pc_rewind,trapID,pc} into ttmp0,1; load TBA into PC,
        // set PRIV=1 and continue.
        kTrap,
        //  Invalidate entire L1 I cache.
        kIcacheInv,
        //  Increment performance counter specified in SIMM16[3:0]
        // by 1.
        kIncperflevel,
        //  Decrement performance counter specified in SIMM16[3:0]
        // by 1.
        kDecperflevel,
        //  Send M0 as user data to thread-trace.
        kTtracedata,

        kCBranchCdbgsys = 23, // CI Specific

        kCBranchCdbguser = 24, // CI Specific

        kCBranchCdbgsysOrUser = 25, // CI Specific

        kCBranchCdbgsysAndUser = 26, // CI Specific

        // Reserved
        kReserved,
        // ILLEGAL
        kIllegal
    };

    // Get the OP     [22:16].
    OP GetOp() const { return op_; }

    SISOPPInstruction(SIMM16 simm16, OP op, int label, int goto_label) : SOPPInstruction(simm16, label, goto_label), op_(op) {}

private:
    // SOPP operation.
    OP op_;
};

class VISOPPInstruction : public SOPPInstruction
{
public:
    // Selector for the SOPP Instruction
    enum OP
    {
        kNop = 0,
        kEndpgm = 1,
        kBranch = 2,
        kWakeup = 3,
        kCbranch_scc0 = 4,
        kCbranch_scc1 = 5,
        kCbranch_vccz = 6,
        kCbranch_vccnz = 7,
        kCbranch_execz = 8,
        kCbranch_execnz = 9,
        kBarrier = 10,
        kSetkill = 11,
        kWaitcnt = 12,
        kSethalt = 13,
        kSleep = 14,
        kSetprio = 15,
        kSendmsg = 16,
        kSendmsghalt = 17,
        kTrap = 18,
        kIcache_inv = 19,
        kIncperflevel = 20,
        kDecperflevel = 21,
        kTtracedata = 22,
        kCbranchCdbgsys = 23,
        kCbranchCdbguser = 24,
        kCbranchCdbgsysOrUser = 25,
        kCbranchCdbgsysAndUser = 26,
        kEndpgm_saved = 27,
        kSetGprIdxOff = 28,
        kSetGprIdxMode = 29,
        // ILLEGAL
        kIllegal = 30,
    };

    // Get the OP     [22:16].
    OP GetOp() const { return op_; }

    VISOPPInstruction(SIMM16 simm16, OP op, int label, int goto_label) : SOPPInstruction(simm16, label, goto_label), op_(op)
    {
        hw_gen_ = GDT_HW_GENERATION_VOLCANICISLAND;
    }

private:
    // SOPP operation.
    OP op_;
};

#endif //__SOPPINSTRUCTION_H


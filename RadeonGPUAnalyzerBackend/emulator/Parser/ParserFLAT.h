//=============================================================
// Copyright (c) 2017 Advanced Micro Devices, Inc.
//=============================================================

#pragma once

#ifndef __FLATINSTRUCTION_H
#define __FLATINSTRUCTION_H

#include "Instruction.h"
#include "ParserSI.h"

/// Opcode :
///        reserved [15:0]
///        GLC      [16:16]
///        SLC      [17:17]
///        OP       [24:18]
///        ENCODING [31:26]  // 1 1 0 1 1 1
///        ADDR     [39:32]
///        DATA     [47:40]
///        reserved [54:48]
///        TFE      [55:55]
///        VDST     [63:56]

class FLATInstruction : public Instruction
{
public:
    FLATInstruction(bool GLC, bool SLC, uint8_t OP, uint8_t ADDR, uint8_t DATA, bool TFE, uint8_t VDST, int iLabel, int iGotoLabel)
        : Instruction(64, GetInstKind(OP), InstructionSet::InstructionSet_FLAT, iLabel, iGotoLabel),
          m_GLC(GLC), m_SLC(SLC), m_OP(OP), m_ADDR(ADDR), m_DATA(DATA), m_TFE(TFE), m_VDST(VDST) {}

    ~FLATInstruction() = default;

    bool      GetGLC()  const  { return m_GLC;  }
    bool      GetSLC()  const  { return m_SLC;  }
    uint8_t   GetOP()   const  { return m_OP;   }
    uint8_t   GetADDR() const  { return m_ADDR; }
    uint8_t   GetDATA() const  { return m_DATA; }
    bool      GetTFE()  const  { return m_TFE;  }
    uint8_t   GetVDST() const  { return m_VDST; }


private:
    // Returns the category of FLAT instruction (Load, Store or Atomic).
    static InstructionCategory  GetInstKind(uint8_t OP)
    {
        return (OP <= 15 ? InstructionCategory::VectorMemoryRead : 
               (OP <= 31 ? InstructionCategory::VectorMemoryWrite : InstructionCategory::Atomics));
    }

    bool      m_GLC;
    bool      m_SLC;
    uint8_t   m_OP;
    uint8_t   m_ADDR;
    uint8_t   m_DATA;
    bool      m_TFE;
    uint8_t   m_VDST;
};

class ParserFLAT : public ParserSI
{
public:
    ParserFLAT() = default;
    ~ParserFLAT() = default;

    // See description in "PerserSI" class.
    virtual ParserSI::kaStatus Parse(GDT_HW_GENERATION, Instruction::instruction32bit, Instruction*&, bool, uint32_t, int, int) override
    {
        return ParserSI::Status_32BitInstructionNotSupported;
    }

    // See description in "PerserSI" class.
    virtual ParserSI::kaStatus Parse(GDT_HW_GENERATION, Instruction::instruction64bit hexInstruction, Instruction*& instruction,
                                     int iLabel = NO_LABEL, int iGotoLabel = NO_LABEL) override
    {
        instruction = new FLATInstruction(    0 != INSTRUCTION_FIELD(hexInstruction, FLAT, GLC,  16),
                                              0 != INSTRUCTION_FIELD(hexInstruction, FLAT, SLC,  17),
                                          (uint8_t)INSTRUCTION_FIELD(hexInstruction, FLAT, OP,   18),
                                          (uint8_t)INSTRUCTION_FIELD(hexInstruction, FLAT, ADDR, 32),
                                          (uint8_t)INSTRUCTION_FIELD(hexInstruction, FLAT, DATA, 40),
                                              0 != INSTRUCTION_FIELD(hexInstruction, FLAT, TFE,  55),
                                          (uint8_t)INSTRUCTION_FIELD(hexInstruction, FLAT, VDST, 56),
                                          iLabel, iGotoLabel);
        return ParserSI::Status_SUCCESS;
    }

private:
    enum FLATMask : uint64_t
    {
        FLATMask_GLC  = 0x0000000000000001ULL << 16, // [16:16]
        FLATMask_SLC  = 0x0000000000000001ULL << 17, // [17:17]
        FLATMask_OP   = 0x000000000000007FULL << 18, // [24:18]
        FLATMask_ENC  = 0x000000000000003FULL << 26, // [31:26]
        FLATMask_ADDR = 0x00000000000000FFULL << 32, // [39:32]
        FLATMask_DATA = 0x00000000000000FFULL << 40, // [47:40]
        FLATMask_TFE  = 0x0000000000000001ULL << 55, // [55:55]
        FLATMask_VDST = 0x00000000000000FFULL << 56  // [63:56]
    };
};

#endif // __FLATINSTRUCTION_H

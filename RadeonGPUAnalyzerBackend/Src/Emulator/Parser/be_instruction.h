//=============================================================
// Copyright (c) 2020 Advanced Micro Devices, Inc.
//=============================================================

#ifndef RGA_RADEONGPUANALYZERBACKEND_SRC_EMULATOR_PARSER_BE_INSTRUCTION_H_
#define RGA_RADEONGPUANALYZERBACKEND_SRC_EMULATOR_PARSER_BE_INSTRUCTION_H_

#ifndef _WIN32
#include <stdint.h>
#endif

// C++.
#include <math.h>
#include <string>
#include <unordered_map>

// Infra.
#include "DeviceInfo.h"
#include "AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h"
#include "AMDTBaseTools/Include/gtAssert.h"
#include "RadeonGPUAnalyzerBackend/Src/be_string_constants.h"

// This value means no label in an instruction.
const int kNoLabel = -1;

#define GENERIC_INSTRUCTION_FIELDS_1(in,val) \
    X_RANGE(ScalarGPRMin,ScalarGPRMax,ScalarGPR,in,val) \
    X_RANGE(Reserved104,Reserved105,Reserved,in,val) \
    X(VccLo,in) \
    X(VccHi,in) \
    X(TbaLo,in) \
    X(TbaHi,in) \
    X(TmaLo,in) \
    X(TmaHi,in) \
    X_RANGE(TtmpMin,TtmpMax,Ttmp,in,val) \
    X(M0,in) \
    X_RANGE(Reserved125,Reserved125,Reserved,in,val) \
    X(ExecLo,in) \
    X(ExecHi,in) \

#define GENERIC_INSTRUCTION_FIELDS_2(in) \
    X_RANGE(Reserved209,Reserved239,Reserved,in) \
    X_RANGE(Reserved248,Reserved250,Reserved,in) \
    X(VCCZ,in)\
    X(EXECZ,in)\
    X(SCC,in) \
    X_RANGE(Reserved254,Reserved254,Reserved,in) \

#define SCALAR_INSTRUCTION_FIELDS(in) \
    X(ConstZero,in) \
    X_RANGE(SignedConstIntPosMin,SignedConstIntPosMax,SignedConstIntPos,in) \
    X_RANGE(SignedConstIntNegMin,SignedConstIntNegMax,SignedConstIntNeg,in) \
    X(ConstFloatPos_0_5,in) \
    X(ConstFloatNeg_0_5,in) \
    X(ConstFloatPos_1_0,in) \
    X(ConstFloatNeg_1_0,in) \
    X(ConstFloatPos_2_0,in) \
    X(ConstFloatNeg_2_0,in) \
    X(ConstFloatPos_4_0,in) \
    X(ConstFloatNeg_4_0,in) \
    X(LiteralConst,in)

#define INSTRUCTION_FIELD(hexInstruction, insName, fieldName, fieldOffset) \
            ((hexInstruction & insName##Mask_##fieldName) >> fieldOffset)

#define INSTRUCTION32_FIELD(hexInstruction, insName, fieldName, fieldOffset) \
            ((hexInstruction & static_cast<Instruction::Instruction32Bit>(insName##Mask_##fieldName)) >> fieldOffset)

#define INSTRUCTION64_FIELD(hexInstruction, insName, fieldName, fieldOffset) \
            ((hexInstruction & (static_cast<Instruction::Instruction64Bit>(insName##Mask_##fieldName) << 32)) >> fieldOffset)

#define EXTRACT_INSTRUCTION32_FIELD(hexInstruction,genName,insName,fieldVar,fieldName,fieldOffset) \
    genName##insName##Instruction::fieldName fieldVar = static_cast<genName##insName##Instruction::fieldName>(INSTRUCTION32_FIELD(hexInstruction, insName, fieldName, fieldOffset));

#define EXTRACT_INSTRUCTION64_FIELD(hexInstruction,insName,fieldVar,fieldName,fieldOffset) \
    insName##Instruction::fieldName fieldVar = static_cast<insName##Instruction::fieldName>(INSTRUCTION64_FIELD(hexInstruction, insName, fieldName, fieldOffset));

#define RETURN_EXTRACT_INSTRUCTION(fieldVar) \
    return fieldVar

class Instruction
{
public:

    // SI instruction microcode formats.
    enum InstructionSet
    {
        kInstructionSetSop2,
        kInstructionSetSopk,
        kInstructionSetSop1,
        kInstructionSetSopc,
        kInstructionSetSopp,
        kInstructionSetVop,
        InstructionSet_VOP2,
        InstructionSet_VOP1,
        InstructionSet_VOPC,
        InstructionSet_VOP3,
        kInstructionSetSmrd, //SI+CI Only
        kInstructionSetVintrp,
        kInstructionSetDs,
        kInstructionSetMubuf,
        kInstructionSetMtbuf,
        kInstructionSetMimg,
        kInstructionSetExp,
        InstructionSet_SMEM, // VI Only, was SMRD
        kInstructionSetFlat
    };

    // Instruction format types.
    enum InstructionCategory
    {
        // Scalar Instruction Memory Read.
        kScalarMemoryRead = 0,

        // Scalar Instruction Memory Write.
        // Note : No scalar memory write until Volcanic Island [VI].
        kScalarMemoryWrite,

        // Scalar ALU Operation
        kScalarAlu,

        // Vector Instruction Memory Read.
        kVectorMemoryRead,

        // Vector Instruction Memory Write.
        kVectorMemoryWrite,

        // Vector ALU Operation.
        kVectorAlu,

        // LDS.
        kLds,

        // GDS.
        kGds,

        // Export.
        kExport,

        // Atomics.
        kAtomics,

        // Flow-Control ([Internal] functional unit).
        kInternal,

        // Branch.
        kBranch,

        // Amount of type.
        kInstructionsCategoryCount
    };

    // Translates Instruction`s functional unit to user friendly std::string.
    static std::string GetFunctionalUnitAsString(InstructionCategory category);

    // 32 bit instructions.
    typedef uint32_t Instruction32Bit;

    // 64 bit instruction.
    typedef uint64_t Instruction64Bit;

    Instruction(unsigned int instructionWidth, InstructionCategory instructionFormatKind, InstructionSet instructionFormat, int label_ = kNoLabel, int iGotoLabel = kNoLabel);

    // ctor for label instruction
    explicit Instruction(const std::string& labelString);

    virtual ~Instruction() = default;

    // Get an instruction`s width in bits.
    // Returns the instruction`s width in bits.
    unsigned int GetInstructionWidth() const { return m_instructionWidth; }

    InstructionCategory GetInstructionCategory() const { return Instruction_category_; }

    InstructionSet GetInstructionFormat() const { return instruction_format_; }

    int GetLabel() const { return label_; }

    // Get instruction line number in disassembly.
    int GetLineNumber() const { return line_number_; }

    // Set instruction line number in disassembly.
    void SetLineNumber(int iLineNumber) { line_number_ = iLineNumber; }

    // Get the source line (and its number) that was translated to this instructions.
    std::pair<int, std::string>  GetSrcLineInfo() const { return { src_line_number_, src_line_ }; }

    // Set the source line (and its number) that was translated to this instructions.
    void SetSrcLineInfo(int lineNum, const std::string& line) { src_line_number_ = lineNum; src_line_ = line; }

    // Get the label if any where instruction is a branch.
    int GetGotoLabel() const { return goto_label_; }

    // Set the label if any where instruction is a branch.
    void SetGotoLabel(int iGotoLabel) { goto_label_ = iGotoLabel; }

    // Get instruction cycle count for a given target.
    int GetInstructionClockCount(const std::string& deviceName) const;

    // The Instruction Asic HW generation. default is SI
    GDT_HW_GENERATION GetHwGen() const { return hw_gen_; }
    void SetHwGen(GDT_HW_GENERATION HwGen) { hw_gen_ = HwGen; }

    // String representation of the instruction's opcode.
    const std::string& GetInstructionOpCode() const { return instruction_opcode_; }

    // String representation of the instruction's parameters.
    const std::string& GetInstructionParameters() const { return parameters_; }

    // String representation of the instruction's binary representation.
    const std::string& GetInstructionBinaryRep() const { return binary_representation_; }

    // String representation of the instruction's offset within the program.
    const std::string& GetInstructionOffset() const { return offset_in_bytes_; }

    // Sets the string representation of the instruction's opcode.
    void SetInstructionOpCode(const std::string& opCode) { instruction_opcode_ = opCode; }

    // Sets the string representation of the instruction's parameters.
    void SetInstructionParameters(const std::string& params) { parameters_ = params; }

    // Sets the string representation of the instruction's binary representation.
    void SetInstructionBinaryRep(const std::string& binaryRep) { binary_representation_ = binaryRep; }

    // Sets the string representation of the instruction's offset within the program.
    void SetInstructionOffset(const std::string& offset) { offset_in_bytes_ = offset; }

    // Sets the string representation of the instruction: opcode, parameters, binary representation and offset within the program.
    void SetInstructionStringRepresentation(const std::string& opCode,
        const std::string& params, const std::string& binaryRep, const std::string& offset);

    // Returns pointing label string
    const std::string& GetPointingLabelString() const { return pointing_label_string_; }

    // Generates a comma separated string representation of the instruction.
    void GetCsvString(const std::string& deviceName, bool srcLineInfo, std::string& commaSeparatedString)const;

protected:

    // Instruction format kind.
    InstructionCategory Instruction_category_;

    // Instruction format.
    InstructionSet instruction_format_;

    // Hardware generation.
    GDT_HW_GENERATION hw_gen_;

    // Instruction width in bits.
    unsigned int m_instructionWidth;

    // Indicates whether this is before the instruction.
    int label_;

    // Indicates whether this is a branch instruction.
    int goto_label_;

    // Line number in the ISA were the instruction came from (used by application).
    int line_number_;

    // String representation of the instruction's opcode.
    std::string instruction_opcode_;

private:

    // Initializes the performance tables.
    static void SetUpPerfTables();

    // Initializes the hybrid architecture performance tables.
    static void SetUpHybridPerfTables();

    // Initializes the scalar performance tables.
    static void SetUpScalarPerfTables();

    // Initializes quarter devices performance tables.
    static void SetUpQuarterDevicesPerfTables();

    // Initializes half devices performance tables.
    static void SetUpHalfDevicesPerfTables();

    // String representation of the parameters.
    std::string parameters_;

    // String of the binary representation of the instruction (e.g. 0xC2078914).
    std::string binary_representation_;

    // String representation of the offset in bytes of the current instruction
    // from the beginning of the program.
    std::string offset_in_bytes_;

    // If this instruction is being pointed by a label, this member will hold the label.
    std::string pointing_label_string_;

    // Corresponding source line.
    std::string src_line_;

    // Corresponding source line number.
    int src_line_number_;

    // Indicates whether the performance tables were initialized or not.
    static bool is_perf_tables_initialized_;

    // Holds the cycles per instruction for the 1/2 device architecture.
    static std::unordered_map<std::string, int> half_device_perf_table_;

    // Holds the cycles per instruction for the 1/4 device architecture.
    static std::unordered_map<std::string, int> quarter_device_perf_table_;

    // Holds the cycles per instruction for the 1/16 device architecture.
    static std::unordered_map<std::string, int> hybrid_device_perf_table_;

    // Holds the cycles per instruction for the scalar instructions.
    static std::unordered_map<std::string, int> scalar_device_perf_table_;
};

#endif //RGA_RADEONGPUANALYZERBACKEND_SRC_EMULATOR_PARSER_BE_INSTRUCTION_H_

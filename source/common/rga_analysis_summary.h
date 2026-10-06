//=============================================================================
/// Copyright (c) 2026 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Public header: data structures for the RGA XML analysis summary.
///
/// This is a standalone header distributed as part of the RGA public release.
/// It defines the RgaAnalysisSummary data model produced by rga cli's --session-summary option.
///
/// Dependencies:
///   - Standard C++ only (<cstdint>, <memory>, <string>, <unordered_map>, <vector>)
//=============================================================================

#ifndef RGA_COMMON_RGA_ANALYSIS_SUMMARY_H_
#define RGA_COMMON_RGA_ANALYSIS_SUMMARY_H_

// C++.
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

// Analysis summary containing all compilation results and metadata.
struct RgaAnalysisSummary
{
    // Schema version for XML serialization.
    static constexpr const char* kSchemaVersion = "1.0.9";

    // Target architecture metadata.
    struct TargetArchitectureMetadata
    {
        // Generic metadata type with ID, name, and description.
        struct MetadataType
        {
            // Unique identifier.
            int id_ = -1;
            // Name.
            std::string name_;
            // Description.
            std::string description_;
        };

        // Instruction functional group metadata with separate group and subgroup names.
        struct FunctionalGroupMetadata
        {
            // Unique identifier.
            int id_ = -1;
            // Functional group name (e.g., "Vector Memory", "Scalar ALU").
            std::string group_name_;
            // Comma-separated subgroup classification (e.g., "Buffer, Load"). Empty if N/A.
            std::string sub_group_name_;
            // Human-readable description of the functional group.
            std::string description_;
        };

        // Branch instruction type definitions.
        std::vector<MetadataType> branch_types_;
        // Register type definitions (VGPR, SGPR, etc.).
        std::vector<MetadataType> register_types_;
        // Instruction functional group definitions.
        std::vector<FunctionalGroupMetadata> instruction_functional_groups_;
    };

    // Instruction is a convenience class meant to represent 1 line of instruction, as a child row.
    struct Instruction
    {
        Instruction(int line, std::string address, std::string binary_representation, std::string block_label)
            : instruction_id_(line)
            , offset_(address)
            , binary_representation_(binary_representation)
            , block_label_(block_label)
        {
        }

        Instruction()                              = delete;
        Instruction(const Instruction&)            = delete;
        Instruction(Instruction&&)                 = delete;
        Instruction& operator=(const Instruction&) = delete;
        Instruction& operator=(Instruction&&)      = delete;
        ~Instruction()                             = default;

        // High-level source code correlation.
        struct HighLevelSource
        {
            // Source file name.
            std::string file_name_;
            // Line number in source file.
            int line_number_ = -1;
        };

        // Branch instruction metadata.
        struct BranchInfo
        {
            // Whether this instruction is a branch.
            bool is_branch_ = false;
            // Branch type (Indirect or Direct).
            std::string branch_type_;
            // Branch target label or address.
            std::string branch_target_;
        };

        // Instruction semantic properties.
        struct Semantics
        {
            // Whether this instruction is a label.
            bool is_label_ = false;
            // Branch information if applicable.
            BranchInfo branch_info_;
        };

        // Debug information for instruction.
        struct DebugInfo
        {
            // High-level source correlation.
            HighLevelSource high_level_source_;
        };

        // Instruction operand.
        struct Operand
        {
            // Operand position (0-based).
            int order_ = 0;
            // Access pattern.
            std::string access_;
            // Operand type.
            std::string type_;
            // Operand name.
            std::string name_;
        };

        struct Operands
        {
            // Raw text of the operands.
            std::string text_;
            // Collection of operand items.
            std::vector<Operand> items_;
        };

        // Register access tracking.
        struct RegisterAccess
        {
            // Register index.
            int index_ = -1;
            // Register name.
            std::string name_;
            // Access pattern (Read, Write, ReadWrite).
            std::string access_;
        };

        // Unique instruction identifier.
        int instruction_id_ = -1;
        // Byte offset in disassembly.
        std::string offset_;
        // Binary representation (hex).
        std::string binary_representation_;
        // Instruction size in bytes. (default is 4 bytes for 32-bit instructions).
        uint64_t instruction_size_ = 4;
        // Instruction opcode.
        std::string opcode_;
        // Semantic properties.
        Semantics semantics_;
        // Debug information.
        DebugInfo debug_info_;
        // Instruction operands.
        Operands operands_;
        // Functional group classification.
        std::string functional_group_;
        // Functional sub-group classification (comma-separated string, e.g., "Buffer, Load").
        std::string functional_sub_group_;
        // Human-readable instruction description.
        std::string instruction_description_;
        // VGPR pressure at this instruction.
        uint64_t vgpr_pressure_ = 0;
        // VGPR allocation at this instruction.
        uint64_t vgpr_allocation_ = 0;
        // Detailed VGPR access information.
        std::vector<RegisterAccess> vgprs_access_;
        // SGPR pressure at this instruction.
        uint64_t sgpr_pressure_ = 0;
        // SGPR allocation at this instruction.
        uint64_t sgpr_allocation_ = 0;
        // Detailed SGPR access information.
        std::vector<RegisterAccess> sgprs_access_;
        // Block label if applicable.
        std::string block_label_;
    };

    // Shared pointer to an Instruction.
    using InstructionPtr = std::shared_ptr<Instruction>;

    // Structure representing a collection of Instructions.
    using Instructions = std::vector<InstructionPtr>;

    // Basic block in control flow graph.
    struct BasicBlock
    {
        // Instruction reference.
        struct InstructionReference
        {
            // Instruction ID reference.
            int instruction_id_ = -1;
            // Instruction opcode.
            std::string opcode_;
            // Operands as text.
            std::string operands_;
        };

        // Live register counts for block.
        struct LiveRegisters
        {
            // VGPR pressure.
            uint64_t vgpr_pressure_ = 0;
            // Detailed VGPR access information.
            std::vector<Instruction::RegisterAccess> vgprs_access_;
            // SGPR pressure.
            uint64_t sgpr_pressure_ = 0;
            // Detailed SGPR access information.
            std::vector<Instruction::RegisterAccess> sgprs_access_;
        };

        // Basic block metadata.
        struct Metadata
        {
            // Predecessor block IDs.
            std::vector<int> predecessors_ids_;
            // Successor block IDs.
            std::vector<int> successors_ids_;
            // Successor branch types.
            std::vector<std::string> successors_branch_types_;
            // Live register information.
            LiveRegisters live_registers_in_;
            // Live register information.
            LiveRegisters live_registers_out_;
        };

        explicit BasicBlock(int block_id, std::string label)
            : block_id_(block_id)
            , block_label_(std::move(label))
        {
        }

        BasicBlock()                             = delete;
        BasicBlock(const BasicBlock&)            = delete;
        BasicBlock(BasicBlock&&)                 = delete;
        BasicBlock& operator=(const BasicBlock&) = delete;
        BasicBlock& operator=(BasicBlock&&)      = delete;
        ~BasicBlock()                            = default;

        bool IsEntry() const
        {
            return block_label_ == kEntryLabel;
        }
        bool IsExit() const
        {
            return block_label_ == kExitLabel;
        }
        bool IsJumpTarget() const
        {
            return !IsEntry() && !IsExit() && block_label_.find(kBasicBlockLabel) == std::string::npos;
        }

        // Unique block identifier.
        int block_id_ = -1;
        // Block label for display.
        std::string block_label_;
        // Human-readable display label (e.g. "label_0A40"). Empty by default.
        std::string display_label_;
        // Block metadata.
        Metadata metadata_;
        // Structure representing a collection of Isa Instructions.
        using InstructionReferences = std::vector<InstructionReference>;
        // Instruction references in this block.
        InstructionReferences instruction_references_;

    private:
        // Block label strings used by IsEntry / IsExit / IsJumpTarget.
        static constexpr const char* kEntryLabel      = "entry";
        static constexpr const char* kExitLabel       = "exit";
        static constexpr const char* kBasicBlockLabel = "basic_block_";
    };

    // Shared pointer to a Basic Block.
    using BasicBlockPtr = std::shared_ptr<BasicBlock>;

    // Collection of basic blocks.
    using BasicBlocks = std::vector<BasicBlockPtr>;

    // Control flow graph with blocks and branches.
    struct ControlFlowGraph
    {
        // Branch edge between blocks.
        struct Branch
        {
            // Source block ID.
            int source_block_id_;
            // Destination block ID.
            int destination_block_id_;
        };

        // All basic blocks in the CFG.
        BasicBlocks blocks_;
        // All branch edges in the CFG.
        std::vector<Branch> branches_;
    };

    // Kernel compilation statistics.
    struct Statistics
    {
        // Serialized as "N/A".
        static constexpr uint64_t kValueNa = UINT64_MAX;

        // API Shader Hash.
        std::string api_shader_hash_;
        // VGPR allocation granularity.
        uint64_t vgprs_allocation_granularity_ = 0;
        // Number of VGPRs allocated.
        uint64_t vgprs_allocated_ = 0;
        // Number of VGPRs used.
        uint64_t vgprs_used_ = 0;
        // Number of VGPRs spilled.
        uint64_t vgprs_spilled_ = 0;
        // Total Number of VGPRs.
        uint64_t vgprs_total_ = 0;
        // Number of AGPRs (accumulation VGPRs) used.
        uint64_t agprs_used_ = kValueNa;
        // Total Number of AGPRs available.
        uint64_t agprs_total_ = kValueNa;
        // SGPR allocation granularity.
        uint64_t sgprs_allocation_granularity_ = 1;
        // Number of SGPRs allocated.
        uint64_t sgprs_allocated_ = 0;
        // Number of SGPRs used.
        uint64_t sgprs_used_ = 0;
        // Number of SGPRs spilled.
        uint64_t sgprs_spilled_ = 0;
        // Total Number of SGPRs.
        uint64_t sgprs_total_ = 0;
        // Scratch memory usage in bytes.
        uint64_t scratch_memory_bytes_ = 0;
        // LDS memory usage in bytes.
        uint64_t lds_bytes_used_ = 0;
    };

    // Kernel with ISA and statistics.
    struct Kernel
    {
        // Unique kernel identifier.
        int kernel_id_ = -1;
        // Kernel name.
        std::string kernel_name_;
        // Kernel type.
        std::string kernel_type_;
        // Full text disassembly.
        std::string text_disassembly_;
        // Full instruction set in the ISA.
        Instructions instructions_;
        // Control flow graph.
        ControlFlowGraph cfg_;
        // Compilation statistics.
        Statistics stats_;
    };

    // Root document metadata.
    struct Document
    {
        // Schema version string.
        std::string schema_version_ = kSchemaVersion;
    };

    // Analysis result for a single compilation.
    struct AnalysisResult
    {
        // Input files and options.
        struct Inputs
        {
            // Input file paths.
            std::vector<std::string> inputs_;
        };

        // Analysis output.
        struct Output
        {
            // API name.
            std::string api_;
            // Compiled kernels.
            std::vector<Kernel> kernels_;
        };

        // Target architecture name.
        std::string target_architecture_;
        // Target architecture metadata.
        TargetArchitectureMetadata target_architecture_metadata_;
        // Analysis inputs.
        Inputs inputs_;
        // Analysis output.
        Output output_;
    };

    // Document metadata.
    Document document_;
    // Analysis results.
    std::vector<AnalysisResult> results_;
};

#endif  // RGA_COMMON_RGA_ANALYSIS_SUMMARY_H_

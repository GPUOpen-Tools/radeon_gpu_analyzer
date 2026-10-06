//=============================================================================
/// Copyright (c) 2026 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief XML element name constants for the RGA analysis summary schema.
//=============================================================================

#ifndef RGA_COMMON_RGA_ANALYSIS_SUMMARY_XML_CONSTANTS_H_
#define RGA_COMMON_RGA_ANALYSIS_SUMMARY_XML_CONSTANTS_H_

namespace rga_xml
{

// Root and document elements.
constexpr const char* kRootElement   = "RgaAnalysisOutput";
constexpr const char* kDocument      = "Document";
constexpr const char* kSchemaVersion = "SchemaVersion";

// Analysis results.
constexpr const char* kAnalysisResults            = "AnalysisResults";
constexpr const char* kAnalysisResult             = "AnalysisResult";
constexpr const char* kTargetArchitecture         = "TargetArchitecture";
constexpr const char* kTargetArchitectureName     = "TargetArchitectureName";
constexpr const char* kTargetArchitectureMetadata = "TargetArchitectureMetadata";

// Metadata types.
constexpr const char* kBranchInstructionTypes         = "BranchInstructionTypes";
constexpr const char* kBranchInstructionType          = "BranchInstructionType";
constexpr const char* kBranchInstructionTypeId        = "BranchInstructionTypeId";
constexpr const char* kBranchInstructionTypeName      = "BranchInstructionTypeName";
constexpr const char* kRegisterTypes                  = "RegisterTypes";
constexpr const char* kRegisterType                   = "RegisterType";
constexpr const char* kRegisterTypeId                 = "RegisterTypeId";
constexpr const char* kRegisterTypeName               = "RegisterTypeName";
constexpr const char* kInstructionFunctionalGroups            = "InstructionFunctionalGroups";
constexpr const char* kInstructionFunctionalGroup             = "InstructionFunctionalGroup";
constexpr const char* kInstructionFunctionalGroupId           = "InstructionFunctionalGroupId";
constexpr const char* kInstructionFunctionalGroupGroupName    = "InstructionFunctionalGroupGroupName";
constexpr const char* kInstructionFunctionalGroupSubGroupName = "InstructionFunctionalGroupSubGroupName";
constexpr const char* kInstructionFunctionalGroupDesc         = "InstructionFunctionalGroupDescription";

// Inputs and Outputs.
constexpr const char* kInputs = "Inputs";
constexpr const char* kInput  = "Input";
constexpr const char* kOutput = "Output";
constexpr const char* kApi    = "API";

// Kernels.
constexpr const char* kKernels        = "Kernels";
constexpr const char* kKernel         = "Kernel";
constexpr const char* kKernelId       = "KernelId";
constexpr const char* kKernelName     = "KernelName";
constexpr const char* kKernelType     = "KernelType";

// ISA.
constexpr const char* kIsa         = "ISA";
constexpr const char* kDisassembly = "Disassembly";

// Blocks.
constexpr const char* kBlocks                     = "Blocks";
constexpr const char* kBlock                      = "Block";
constexpr const char* kBlockId                    = "BlockId";
constexpr const char* kBlockLabel                 = "BlockLabel";
constexpr const char* kBlockDisplayLabel          = "BlockDisplayLabel";
constexpr const char* kBlockMetadata              = "BlockMetadata";
constexpr const char* kBlockPredecessorsIds       = "BlockPredecessorsIds";
constexpr const char* kBlockSuccessorsIds         = "BlockSuccessorsIds";
constexpr const char* kBlockSuccessorsBranchTypes = "BlockSuccessorsBranchTypes";

// Live registers.
constexpr const char* kLiveRegistersIn  = "LiveRegistersIn";
constexpr const char* kLiveRegistersOut = "LiveRegistersOut";

// Instructions.
constexpr const char* kInstructions          = "Instructions";
constexpr const char* kInstruction           = "Instruction";
constexpr const char* kInstructionReference  = "InstructionReference";
constexpr const char* kInstructionId        = "InstructionId";
constexpr const char* kOffset               = "Offset";
constexpr const char* kBinaryRepresentation = "BinaryRepresentation";
constexpr const char* kInstructionSize      = "InstructionSizeInBytes";
constexpr const char* kOpcode               = "Opcode";

// Instruction semantics.
constexpr const char* kInstructionSemantics = "InstructionSemantics";
constexpr const char* kIsLabel              = "IsLabel";
constexpr const char* kBranchInfo           = "BranchInfo";
constexpr const char* kIsBranch             = "IsBranch";
constexpr const char* kBranchType           = "BranchType";
constexpr const char* kBranchTarget         = "BranchTarget";

// Debug info.
constexpr const char* kInstructionDebugInfo = "InstructionDebugInfo";
constexpr const char* kHighLevelSource      = "HighLevelSource";
constexpr const char* kFileName             = "FileName";
constexpr const char* kLineNumber           = "LineNumber";

// Operands.
constexpr const char* kOperands     = "Operands";
constexpr const char* kOperandsText = "Text";
constexpr const char* kOperand      = "Operand";
constexpr const char* kOperandOrder = "OperandOrder";
constexpr const char* kAccess       = "Access";
constexpr const char* kOperandType  = "OperandType";
constexpr const char* kOperandName  = "OperandName";

// Functional groups.
constexpr const char* kFunctionalGroup     = "FunctionalGroup";
constexpr const char* kFunctionalSubGroups = "FunctionalSubGroups";

// Instruction description.
constexpr const char* kInstructionDescription = "InstructionDescription";

// VGPR info.
constexpr const char* kVgprPressure   = "VgprPressure";
constexpr const char* kVgprAllocation = "VgprAllocation";
constexpr const char* kVgprsAccess    = "VgprsAccess";
constexpr const char* kVgpr           = "Vgpr";
constexpr const char* kIndex          = "Index";
constexpr const char* kName           = "Name";

// SGPR info.
constexpr const char* kSgprPressure   = "SgprPressure";
constexpr const char* kSgprAllocation = "SgprAllocation";
constexpr const char* kSgprsAccess    = "SgprsAccess";
constexpr const char* kSgpr           = "Sgpr";

// CFG.
constexpr const char* kCfg                = "CFG";
constexpr const char* kBranch             = "Branch";
constexpr const char* kSourceBlockId      = "SourceBlockId";
constexpr const char* kDestinationBlockId = "DestinationBlockId";

// Statistics.
constexpr const char* kStats                      = "Stats";
constexpr const char* kApiShaderHash              = "ApiShaderHash";
constexpr const char* kVgprsAllocationGranularity = "VgprsAllocationGranularity";
constexpr const char* kVgprsAllocated             = "VgprsAllocated";
constexpr const char* kVgprsUsed                  = "VgprsUsed";
constexpr const char* kVgprsSpilled               = "VgprsSpilled";
constexpr const char* kVgprsTotal                 = "VgprsTotal";
constexpr const char* kAgprsUsed                  = "AgprsUsed";
constexpr const char* kAgprsTotal                 = "AgprsTotal";
constexpr const char* kSgprsAllocated             = "SgprsAllocated";
constexpr const char* kSgprsUsed                  = "SgprsUsed";
constexpr const char* kSgprsSpilled               = "SgprsSpilled";
constexpr const char* kScratchMemoryBytes         = "ScratchMemoryBytes";
constexpr const char* kLdsBytesUsed               = "LdsBytesUsed";

}  // namespace rga_xml

#endif  // RGA_COMMON_RGA_ANALYSIS_SUMMARY_XML_CONSTANTS_H_

//=============================================================================
/// Copyright (c) 2026 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header-only public reader for RGA analysis summary XML files.
///
/// Consumers must provide tinyxml2 (link against tinyxml2) and include its
/// header before or after this file (the include order does not matter as
/// long as tinyxml2.h is on the include path).
//=============================================================================
#ifndef RGA_COMMON_RGA_ANALYSIS_SUMMARY_READER_H_
#define RGA_COMMON_RGA_ANALYSIS_SUMMARY_READER_H_

// C++.
#include <cassert>
#include <sstream>
#include <string>
#include <vector>

// XML.
#include "tinyxml2.h"

// Public data structures.
#include "common/rga_analysis_summary.h"

// XML tag constants.
#include "common/rga_analysis_summary_xml_constants.h"

// Public header-only XML reader for RGA analysis summary files.
// Deserializes the XML produced by rga --session-summary into an RgaAnalysisSummary struct.
//
// Usage:
//   RgaAnalysisSummary summary;
//   bool ok = RgaAnalysisSummaryReader::ReadAnalysisSummaryFromFile("output.xml", summary);
class RgaAnalysisSummaryReader
{
public:
    // Read analysis summary from an XML file.
    // Returns true on success, false on failure.
    static bool ReadAnalysisSummaryFromFile(const std::string& filename, RgaAnalysisSummary& summary);

private:
    // Read document metadata element.
    static bool ReadDocument(const tinyxml2::XMLElement* root, RgaAnalysisSummary::Document& document);

    // Read target architecture metadata element.
    static bool ReadTargetArchitectureMetadata(const tinyxml2::XMLElement* element, RgaAnalysisSummary::TargetArchitectureMetadata& metadata);

    // Read metadata types.
    static bool ReadMetadataTypes(const tinyxml2::XMLElement*                                                      element,
                                  const char*                                                                      container_name,
                                  const char*                                                                      element_name,
                                  const char*                                                                      id_name,
                                  const char*                                                                      name_name,
                                  const char*                                                                      description_name,
                                  std::vector<RgaAnalysisSummary::TargetArchitectureMetadata::MetadataType>& types);

    // Read inputs section.
    static bool ReadInputs(const tinyxml2::XMLElement* element, RgaAnalysisSummary::AnalysisResult::Inputs& inputs);

    // Read output section.
    static bool ReadOutput(const tinyxml2::XMLElement* element, RgaAnalysisSummary::AnalysisResult::Output& output);

    // Read kernel element.
    static bool ReadKernel(const tinyxml2::XMLElement* element, RgaAnalysisSummary::Kernel& kernel);

    // Read ISA section.
    static bool ReadISA(const tinyxml2::XMLElement* element, RgaAnalysisSummary::Kernel& kernel);

    // Read blocks section.
    static bool ReadBlocks(const tinyxml2::XMLElement* element, RgaAnalysisSummary::BasicBlocks& blocks);

    // Read basic block element.
    static bool ReadBlock(const tinyxml2::XMLElement* element, RgaAnalysisSummary::BasicBlockPtr& block);

    // Read block metadata element.
    static bool ReadBlockMetadata(const tinyxml2::XMLElement* element, RgaAnalysisSummary::BasicBlock::Metadata& metadata);

    // Read block instruction references.
    static bool ReadBlockInstructions(const tinyxml2::XMLElement* element, RgaAnalysisSummary::BasicBlock::InstructionReferences& instruction_refs);

    // Read live registers element.
    static bool ReadLiveRegisters(const tinyxml2::XMLElement* element, RgaAnalysisSummary::BasicBlock::LiveRegisters& live_registers);

    // Read instructions section.
    static bool ReadInstructions(const tinyxml2::XMLElement* element, RgaAnalysisSummary::Instructions& instructions);

    // Read instruction element.
    static bool ReadInstruction(const tinyxml2::XMLElement* element, RgaAnalysisSummary::InstructionPtr& instruction);

    // Read instruction semantics element.
    static bool ReadInstructionSemantics(const tinyxml2::XMLElement* element, RgaAnalysisSummary::Instruction::Semantics& semantics);

    // Read branch info element.
    static bool ReadBranchInfo(const tinyxml2::XMLElement* element, RgaAnalysisSummary::Instruction::BranchInfo& branch_info);

    // Read instruction debug info element.
    static bool ReadInstructionDebugInfo(const tinyxml2::XMLElement* element, RgaAnalysisSummary::Instruction::DebugInfo& debug_info);

    // Read high-level source element.
    static bool ReadHighLevelSource(const tinyxml2::XMLElement* element, RgaAnalysisSummary::Instruction::HighLevelSource& source);

    // Read operands section.
    static bool ReadOperands(const tinyxml2::XMLElement* element, RgaAnalysisSummary::Instruction::Operands& operands);

    // Read operand element.
    static bool ReadOperand(const tinyxml2::XMLElement* element, RgaAnalysisSummary::Instruction::Operand& operand);

    // Read functional group metadata from target architecture metadata section.
    static bool ReadFunctionalGroupMetadata(
        const tinyxml2::XMLElement*                                                           element,
        std::vector<RgaAnalysisSummary::TargetArchitectureMetadata::FunctionalGroupMetadata>& groups);

    // Read register access section.
    // reg_tag: XML child element tag, pass rga_xml::kVgpr or rga_xml::kSgpr.
    static bool ReadRegisterAccess(const tinyxml2::XMLElement*                                         element,
                                   const char*                                                         reg_tag,
                                   std::vector<RgaAnalysisSummary::Instruction::RegisterAccess>& reg_access);

    // Read register access element.
    static bool ReadRegisterAccess(const tinyxml2::XMLElement* element, RgaAnalysisSummary::Instruction::RegisterAccess& reg_access);

    // Read CFG section.
    static bool ReadCFG(const tinyxml2::XMLElement* element, std::vector<RgaAnalysisSummary::ControlFlowGraph::Branch>& branches);

    // Read branch element.
    static bool ReadBranch(const tinyxml2::XMLElement* element, RgaAnalysisSummary::ControlFlowGraph::Branch& branch);

    // Read statistics section.
    static bool ReadStatistics(const tinyxml2::XMLElement* element, RgaAnalysisSummary::Statistics& stats);

    // Read analysis results section.
    static bool ReadAnalysisResults(const tinyxml2::XMLElement* root, std::vector<RgaAnalysisSummary::AnalysisResult>& results);

    // Read analysis result element.
    static bool ReadAnalysisResult(const tinyxml2::XMLElement* element, RgaAnalysisSummary::AnalysisResult& result);

    // Get text content from element.
    static std::string GetTextContent(const tinyxml2::XMLElement* element);

    // Get integer content from element.
    static int GetIntContent(const tinyxml2::XMLElement* element, int default_value = -1);

    // Get uint64_t content from element.
    static uint64_t GetUInt64Content(const tinyxml2::XMLElement* element, uint64_t default_value = 0);

    // Get boolean content from element.
    static bool GetBoolContent(const tinyxml2::XMLElement* element, bool default_value = false);

    // Find child element by name.
    static const tinyxml2::XMLElement* FindChildElement(const tinyxml2::XMLElement* parent, const char* name);
};

// ---- Inline method definitions ----

inline bool RgaAnalysisSummaryReader::ReadAnalysisSummaryFromFile(const std::string& filename, RgaAnalysisSummary& summary)
{
    if (filename.empty())
    {
        return false;
    }

    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError    error = doc.LoadFile(filename.c_str());
    if (error != tinyxml2::XML_SUCCESS)
    {
        return false;
    }

    const tinyxml2::XMLElement* root = doc.FirstChildElement(rga_xml::kRootElement);
    if (!root)
    {
        return false;
    }

    if (!ReadDocument(root, summary.document_))
    {
        return false;
    }

    if (!ReadAnalysisResults(root, summary.results_))
    {
        return false;
    }

    return true;
}

inline bool RgaAnalysisSummaryReader::ReadDocument(const tinyxml2::XMLElement* root, RgaAnalysisSummary::Document& document)
{
    assert(root != nullptr);
    if (!root)
    {
        return false;
    }

    const tinyxml2::XMLElement* doc_element = FindChildElement(root, rga_xml::kDocument);
    if (!doc_element)
    {
        return false;
    }

    const tinyxml2::XMLElement* version_element = FindChildElement(doc_element, rga_xml::kSchemaVersion);
    if (version_element)
    {
        document.schema_version_ = GetTextContent(version_element);
    }

    return true;
}

inline bool RgaAnalysisSummaryReader::ReadTargetArchitectureMetadata(const tinyxml2::XMLElement*                         element,
                                                         RgaAnalysisSummary::TargetArchitectureMetadata& metadata)
{
    assert(element != nullptr);
    if (!element)
    {
        return false;
    }

    ReadMetadataTypes(element,
                      rga_xml::kBranchInstructionTypes,
                      rga_xml::kBranchInstructionType,
                      rga_xml::kBranchInstructionTypeId,
                      rga_xml::kBranchInstructionTypeName,
                      nullptr,
                      metadata.branch_types_);

    ReadMetadataTypes(element, rga_xml::kRegisterTypes, rga_xml::kRegisterType, rga_xml::kRegisterTypeId, rga_xml::kRegisterTypeName, nullptr, metadata.register_types_);

    ReadFunctionalGroupMetadata(element, metadata.instruction_functional_groups_);

    return true;
}

inline bool RgaAnalysisSummaryReader::ReadMetadataTypes(const tinyxml2::XMLElement*                                                          element,
                                            const char*                                                                          container_name,
                                            const char*                                                                          element_name,
                                            const char*                                                                          id_name,
                                            const char*                                                                          name_name,
                                            const char*                                                                          description_name,
                                            std::vector<RgaAnalysisSummary::TargetArchitectureMetadata::MetadataType>& types)
{
    assert(element != nullptr);
    assert(container_name != nullptr);
    assert(element_name != nullptr);
    assert(id_name != nullptr);
    assert(name_name != nullptr);
    if (!element || !container_name || !element_name || !id_name || !name_name)
    {
        return false;
    }

    const tinyxml2::XMLElement* container = FindChildElement(element, container_name);
    if (!container)
    {
        return true;
    }

    for (const tinyxml2::XMLElement* type_element = container->FirstChildElement(element_name); type_element != nullptr;
         type_element                                 = type_element->NextSiblingElement(element_name))
    {
        RgaAnalysisSummary::TargetArchitectureMetadata::MetadataType type;

        const tinyxml2::XMLElement* id_element = FindChildElement(type_element, id_name);
        if (id_element)
        {
            type.id_ = GetIntContent(id_element);
        }

        const tinyxml2::XMLElement* name_element = FindChildElement(type_element, name_name);
        if (name_element)
        {
            type.name_ = GetTextContent(name_element);
        }

        if (description_name != nullptr)
        {
            const tinyxml2::XMLElement* desc_element = FindChildElement(type_element, description_name);
            if (desc_element)
            {
                type.description_ = GetTextContent(desc_element);
            }
        }

        types.push_back(type);
    }

    return true;
}

inline bool RgaAnalysisSummaryReader::ReadInputs(const tinyxml2::XMLElement* element, RgaAnalysisSummary::AnalysisResult::Inputs& inputs)
{
    assert(element != nullptr);
    if (!element)
    {
        return false;
    }

    const tinyxml2::XMLElement* inputs_element = FindChildElement(element, rga_xml::kInputs);
    if (!inputs_element)
    {
        return true;
    }

    for (const tinyxml2::XMLElement* input_element = inputs_element->FirstChildElement(rga_xml::kInput); input_element != nullptr;
         input_element                                 = input_element->NextSiblingElement(rga_xml::kInput))
    {
        std::string input = GetTextContent(input_element);
        if (!input.empty())
        {
            inputs.inputs_.push_back(input);
        }
    }

    return true;
}

inline bool RgaAnalysisSummaryReader::ReadOutput(const tinyxml2::XMLElement* element, RgaAnalysisSummary::AnalysisResult::Output& output)
{
    assert(element != nullptr);
    if (!element)
    {
        return false;
    }

    const tinyxml2::XMLElement* output_element = FindChildElement(element, rga_xml::kOutput);
    if (!output_element)
    {
        return false;
    }

    const tinyxml2::XMLElement* api_element = FindChildElement(output_element, rga_xml::kApi);
    if (api_element)
    {
        output.api_ = GetTextContent(api_element);
    }

    const tinyxml2::XMLElement* kernels_element = FindChildElement(output_element, rga_xml::kKernels);
    if (kernels_element)
    {
        for (const tinyxml2::XMLElement* kernel_element = kernels_element->FirstChildElement(rga_xml::kKernel); kernel_element != nullptr;
             kernel_element                                 = kernel_element->NextSiblingElement(rga_xml::kKernel))
        {
            RgaAnalysisSummary::Kernel kernel;
            if (ReadKernel(kernel_element, kernel))
            {
                output.kernels_.push_back(kernel);
            }
        }
    }

    return true;
}

inline bool RgaAnalysisSummaryReader::ReadKernel(const tinyxml2::XMLElement* element, RgaAnalysisSummary::Kernel& kernel)
{
    assert(element != nullptr);
    if (!element)
    {
        return false;
    }

    const tinyxml2::XMLElement* kernel_id_element = FindChildElement(element, rga_xml::kKernelId);
    if (kernel_id_element)
    {
        kernel.kernel_id_ = GetIntContent(kernel_id_element);
    }

    const tinyxml2::XMLElement* kernel_name_element = FindChildElement(element, rga_xml::kKernelName);
    if (kernel_name_element)
    {
        kernel.kernel_name_ = GetTextContent(kernel_name_element);
    }

    const tinyxml2::XMLElement* kernel_type_element = FindChildElement(element, rga_xml::kKernelType);
    if (kernel_type_element)
    {
        kernel.kernel_type_ = GetTextContent(kernel_type_element);
    }

    const tinyxml2::XMLElement* isa_element = FindChildElement(element, rga_xml::kIsa);
    if (isa_element)
    {
        ReadISA(isa_element, kernel);
    }

    const tinyxml2::XMLElement* stats_element = FindChildElement(element, rga_xml::kStats);
    if (stats_element)
    {
        ReadStatistics(stats_element, kernel.stats_);
    }

    return true;
}

inline bool RgaAnalysisSummaryReader::ReadISA(const tinyxml2::XMLElement* element, RgaAnalysisSummary::Kernel& kernel)
{
    assert(element != nullptr);
    if (!element)
    {
        return false;
    }

    const tinyxml2::XMLElement* disassembly_element = FindChildElement(element, rga_xml::kDisassembly);
    if (disassembly_element)
    {
        kernel.text_disassembly_ = GetTextContent(disassembly_element);
    }

    const tinyxml2::XMLElement* instructions_element = FindChildElement(element, rga_xml::kInstructions);
    if (instructions_element)
    {
        ReadInstructions(instructions_element, kernel.instructions_);
    }

    const tinyxml2::XMLElement* blocks_element = FindChildElement(element, rga_xml::kBlocks);
    if (blocks_element)
    {
        ReadBlocks(blocks_element, kernel.cfg_.blocks_);
    }

    const tinyxml2::XMLElement* cfg_element = FindChildElement(element, rga_xml::kCfg);
    if (cfg_element)
    {
        ReadCFG(cfg_element, kernel.cfg_.branches_);
    }

    return true;
}

inline bool RgaAnalysisSummaryReader::ReadBlocks(const tinyxml2::XMLElement* element, RgaAnalysisSummary::BasicBlocks& blocks)
{
    assert(element != nullptr);
    if (!element)
    {
        return false;
    }

    for (const tinyxml2::XMLElement* block_element = element->FirstChildElement(rga_xml::kBlock); block_element != nullptr;
         block_element                                 = block_element->NextSiblingElement(rga_xml::kBlock))
    {
        RgaAnalysisSummary::BasicBlockPtr block;
        if (ReadBlock(block_element, block) && block)
        {
            blocks.push_back(block);
        }
    }

    return true;
}

inline bool RgaAnalysisSummaryReader::ReadBlock(const tinyxml2::XMLElement* element, RgaAnalysisSummary::BasicBlockPtr& block)
{
    assert(element != nullptr);
    if (!element)
    {
        return false;
    }

    int         block_id    = -1;
    std::string block_label = "";

    const tinyxml2::XMLElement* block_id_element = FindChildElement(element, rga_xml::kBlockId);
    if (block_id_element)
    {
        block_id = GetIntContent(block_id_element);
    }

    const tinyxml2::XMLElement* block_label_element = FindChildElement(element, rga_xml::kBlockLabel);
    if (block_label_element)
    {
        block_label = GetTextContent(block_label_element);
    }

    block = std::make_shared<RgaAnalysisSummary::BasicBlock>(block_id, block_label);

    const tinyxml2::XMLElement* display_label_element = FindChildElement(element, rga_xml::kBlockDisplayLabel);
    if (display_label_element)
    {
        block->display_label_ = GetTextContent(display_label_element);
    }

    const tinyxml2::XMLElement* metadata_element = FindChildElement(element, rga_xml::kBlockMetadata);
    if (metadata_element)
    {
        ReadBlockMetadata(metadata_element, block->metadata_);
    }

    const tinyxml2::XMLElement* instructions_element = FindChildElement(element, rga_xml::kInstructions);
    if (instructions_element)
    {
        ReadBlockInstructions(instructions_element, block->instruction_references_);
    }

    return true;
}

inline bool RgaAnalysisSummaryReader::ReadBlockMetadata(const tinyxml2::XMLElement* element, RgaAnalysisSummary::BasicBlock::Metadata& metadata)
{
    assert(element != nullptr);
    if (!element)
    {
        return false;
    }

    const tinyxml2::XMLElement* predecessors_element = FindChildElement(element, rga_xml::kBlockPredecessorsIds);
    if (predecessors_element)
    {
        std::string text = GetTextContent(predecessors_element);
        if (!text.empty())
        {
            std::istringstream iss(text);
            int                id;
            while (iss >> id)
            {
                metadata.predecessors_ids_.push_back(id);
            }
        }
    }

    const tinyxml2::XMLElement* successors_element = FindChildElement(element, rga_xml::kBlockSuccessorsIds);
    if (successors_element)
    {
        std::string text = GetTextContent(successors_element);
        if (!text.empty())
        {
            std::istringstream iss(text);
            int                id;
            while (iss >> id)
            {
                metadata.successors_ids_.push_back(id);
            }
        }
    }

    const tinyxml2::XMLElement* successors_types_element = FindChildElement(element, rga_xml::kBlockSuccessorsBranchTypes);
    if (successors_types_element)
    {
        std::string text = GetTextContent(successors_types_element);
        if (!text.empty())
        {
            std::istringstream iss(text);
            std::string        branch_type;
            while (iss >> branch_type)
            {
                metadata.successors_branch_types_.push_back(branch_type);
            }
        }
    }

    const tinyxml2::XMLElement* live_registers_in_element = FindChildElement(element, rga_xml::kLiveRegistersIn);
    if (live_registers_in_element)
    {
        ReadLiveRegisters(live_registers_in_element, metadata.live_registers_in_);
    }

    const tinyxml2::XMLElement* live_registers_out_element = FindChildElement(element, rga_xml::kLiveRegistersOut);
    if (live_registers_out_element)
    {
        ReadLiveRegisters(live_registers_out_element, metadata.live_registers_out_);
    }

    return true;
}

inline bool RgaAnalysisSummaryReader::ReadBlockInstructions(const tinyxml2::XMLElement*                                    element,
                                                RgaAnalysisSummary::BasicBlock::InstructionReferences& instruction_refs)
{
    assert(element != nullptr);
    if (!element)
    {
        return false;
    }

    for (const tinyxml2::XMLElement* instruction_element = element->FirstChildElement(rga_xml::kInstructionReference); instruction_element != nullptr;
         instruction_element                                 = instruction_element->NextSiblingElement(rga_xml::kInstructionReference))
    {
        RgaAnalysisSummary::BasicBlock::InstructionReference instruction_ref;

        const tinyxml2::XMLElement* id_element = FindChildElement(instruction_element, rga_xml::kInstructionId);
        if (id_element)
        {
            instruction_ref.instruction_id_ = GetIntContent(id_element);
        }

        const tinyxml2::XMLElement* opcode_element = FindChildElement(instruction_element, rga_xml::kOpcode);
        if (opcode_element)
        {
            instruction_ref.opcode_ = GetTextContent(opcode_element);
        }

        const tinyxml2::XMLElement* operands_element = FindChildElement(instruction_element, rga_xml::kOperands);
        if (operands_element)
        {
            instruction_ref.operands_ = GetTextContent(operands_element);
        }

        instruction_refs.push_back(instruction_ref);
    }

    return true;
}

inline bool RgaAnalysisSummaryReader::ReadLiveRegisters(const tinyxml2::XMLElement* element, RgaAnalysisSummary::BasicBlock::LiveRegisters& live_registers)
{
    assert(element != nullptr);
    if (!element)
    {
        return false;
    }

    const tinyxml2::XMLElement* vgpr_pressure_element = FindChildElement(element, rga_xml::kVgprPressure);
    if (vgpr_pressure_element)
    {
        live_registers.vgpr_pressure_ = GetUInt64Content(vgpr_pressure_element);
    }

    const tinyxml2::XMLElement* vgprs_access_element = FindChildElement(element, rga_xml::kVgprsAccess);
    if (vgprs_access_element)
    {
        ReadRegisterAccess(vgprs_access_element, rga_xml::kVgpr, live_registers.vgprs_access_);
    }

    const tinyxml2::XMLElement* sgpr_pressure_element = FindChildElement(element, rga_xml::kSgprPressure);
    if (sgpr_pressure_element)
    {
        live_registers.sgpr_pressure_ = GetUInt64Content(sgpr_pressure_element);
    }

    const tinyxml2::XMLElement* sgprs_access_element = FindChildElement(element, rga_xml::kSgprsAccess);
    if (sgprs_access_element)
    {
        ReadRegisterAccess(sgprs_access_element, rga_xml::kSgpr, live_registers.sgprs_access_);
    }

    return true;
}

inline bool RgaAnalysisSummaryReader::ReadInstructions(const tinyxml2::XMLElement* element, RgaAnalysisSummary::Instructions& instructions)
{
    assert(element != nullptr);
    if (!element)
    {
        return false;
    }

    for (const tinyxml2::XMLElement* instruction_element = element->FirstChildElement(rga_xml::kInstruction); instruction_element != nullptr;
         instruction_element                                 = instruction_element->NextSiblingElement(rga_xml::kInstruction))
    {
        RgaAnalysisSummary::InstructionPtr instruction;
        if (ReadInstruction(instruction_element, instruction) && instruction)
        {
            instructions.push_back(instruction);
        }
    }

    return true;
}

inline bool RgaAnalysisSummaryReader::ReadInstruction(const tinyxml2::XMLElement* element, RgaAnalysisSummary::InstructionPtr& instruction)
{
    assert(element != nullptr);
    if (!element)
    {
        return false;
    }

    int         line                  = -1;
    std::string address               = "";
    std::string binary_representation = "";
    std::string block_label           = "";

    const tinyxml2::XMLElement* id_element = FindChildElement(element, rga_xml::kInstructionId);
    if (id_element)
    {
        line = GetIntContent(id_element);
    }

    const tinyxml2::XMLElement* offset_element = FindChildElement(element, rga_xml::kOffset);
    if (offset_element)
    {
        address = GetTextContent(offset_element);
    }

    const tinyxml2::XMLElement* binary_element = FindChildElement(element, rga_xml::kBinaryRepresentation);
    if (binary_element)
    {
        binary_representation = GetTextContent(binary_element);
    }

    instruction = std::make_shared<RgaAnalysisSummary::Instruction>(line, address, binary_representation, block_label);

    const tinyxml2::XMLElement* instruction_size_element = FindChildElement(element, rga_xml::kInstructionSize);
    if (instruction_size_element)
    {
        instruction->instruction_size_ = GetUInt64Content(instruction_size_element);
    }

    const tinyxml2::XMLElement* opcode_element = FindChildElement(element, rga_xml::kOpcode);
    if (opcode_element)
    {
        instruction->opcode_ = GetTextContent(opcode_element);
    }

    const tinyxml2::XMLElement* semantics_element = FindChildElement(element, rga_xml::kInstructionSemantics);
    if (semantics_element)
    {
        ReadInstructionSemantics(semantics_element, instruction->semantics_);
    }

    const tinyxml2::XMLElement* debug_info_element = FindChildElement(element, rga_xml::kInstructionDebugInfo);
    if (debug_info_element)
    {
        ReadInstructionDebugInfo(debug_info_element, instruction->debug_info_);
    }

    const tinyxml2::XMLElement* operands_element = FindChildElement(element, rga_xml::kOperands);
    if (operands_element)
    {
        ReadOperands(operands_element, instruction->operands_);
    }

    const tinyxml2::XMLElement* functional_group_element = FindChildElement(element, rga_xml::kFunctionalGroup);
    if (functional_group_element)
    {
        instruction->functional_group_ = GetTextContent(functional_group_element);
    }

    const tinyxml2::XMLElement* functional_sub_group_element = FindChildElement(element, rga_xml::kFunctionalSubGroups);
    if (functional_sub_group_element)
    {
        instruction->functional_sub_group_ = GetTextContent(functional_sub_group_element);
    }

    const tinyxml2::XMLElement* instruction_description_element = FindChildElement(element, rga_xml::kInstructionDescription);
    if (instruction_description_element)
    {
        instruction->instruction_description_ = GetTextContent(instruction_description_element);
    }

    const tinyxml2::XMLElement* vgpr_pressure_element = FindChildElement(element, rga_xml::kVgprPressure);
    if (vgpr_pressure_element)
    {
        instruction->vgpr_pressure_ = GetUInt64Content(vgpr_pressure_element);
    }

    const tinyxml2::XMLElement* vgpr_allocation_element = FindChildElement(element, rga_xml::kVgprAllocation);
    if (vgpr_allocation_element)
    {
        instruction->vgpr_allocation_ = GetUInt64Content(vgpr_allocation_element);
    }

    const tinyxml2::XMLElement* vgprs_access_element = FindChildElement(element, rga_xml::kVgprsAccess);
    if (vgprs_access_element)
    {
        ReadRegisterAccess(vgprs_access_element, rga_xml::kVgpr, instruction->vgprs_access_);
    }

    const tinyxml2::XMLElement* sgpr_pressure_element = FindChildElement(element, rga_xml::kSgprPressure);
    if (sgpr_pressure_element)
    {
        instruction->sgpr_pressure_ = GetUInt64Content(sgpr_pressure_element);
    }

    const tinyxml2::XMLElement* sgpr_allocation_element = FindChildElement(element, rga_xml::kSgprAllocation);
    if (sgpr_allocation_element)
    {
        instruction->sgpr_allocation_ = GetUInt64Content(sgpr_allocation_element);
    }

    const tinyxml2::XMLElement* sgprs_access_element = FindChildElement(element, rga_xml::kSgprsAccess);
    if (sgprs_access_element)
    {
        ReadRegisterAccess(sgprs_access_element, rga_xml::kSgpr, instruction->sgprs_access_);
    }

    return true;
}

inline bool RgaAnalysisSummaryReader::ReadInstructionSemantics(const tinyxml2::XMLElement*                       element,
                                                   RgaAnalysisSummary::Instruction::Semantics& semantics)
{
    assert(element != nullptr);
    if (!element)
    {
        return false;
    }

    const tinyxml2::XMLElement* is_label_element = FindChildElement(element, rga_xml::kIsLabel);
    if (is_label_element)
    {
        semantics.is_label_ = GetBoolContent(is_label_element);
    }

    const tinyxml2::XMLElement* branch_info_element = FindChildElement(element, rga_xml::kBranchInfo);
    if (branch_info_element)
    {
        ReadBranchInfo(branch_info_element, semantics.branch_info_);
    }

    return true;
}

inline bool RgaAnalysisSummaryReader::ReadBranchInfo(const tinyxml2::XMLElement* element, RgaAnalysisSummary::Instruction::BranchInfo& branch_info)
{
    assert(element != nullptr);
    if (!element)
    {
        return false;
    }

    const tinyxml2::XMLElement* is_branch_element = FindChildElement(element, rga_xml::kIsBranch);
    if (is_branch_element)
    {
        branch_info.is_branch_ = GetBoolContent(is_branch_element);
    }

    const tinyxml2::XMLElement* branch_type_element = FindChildElement(element, rga_xml::kBranchType);
    if (branch_type_element)
    {
        branch_info.branch_type_ = GetTextContent(branch_type_element);
    }

    const tinyxml2::XMLElement* branch_target_element = FindChildElement(element, rga_xml::kBranchTarget);
    if (branch_target_element)
    {
        branch_info.branch_target_ = GetTextContent(branch_target_element);
    }

    return true;
}

inline bool RgaAnalysisSummaryReader::ReadInstructionDebugInfo(const tinyxml2::XMLElement*                       element,
                                                   RgaAnalysisSummary::Instruction::DebugInfo& debug_info)
{
    assert(element != nullptr);
    if (!element)
    {
        return false;
    }

    const tinyxml2::XMLElement* high_level_source_element = FindChildElement(element, rga_xml::kHighLevelSource);
    if (high_level_source_element)
    {
        ReadHighLevelSource(high_level_source_element, debug_info.high_level_source_);
    }

    return true;
}

inline bool RgaAnalysisSummaryReader::ReadHighLevelSource(const tinyxml2::XMLElement*                             element,
                                              RgaAnalysisSummary::Instruction::HighLevelSource& source)
{
    assert(element != nullptr);
    if (!element)
    {
        return false;
    }

    const tinyxml2::XMLElement* file_name_element = FindChildElement(element, rga_xml::kFileName);
    if (file_name_element)
    {
        source.file_name_ = GetTextContent(file_name_element);
    }

    const tinyxml2::XMLElement* line_number_element = FindChildElement(element, rga_xml::kLineNumber);
    if (line_number_element)
    {
        source.line_number_ = GetIntContent(line_number_element);
    }

    return true;
}

inline bool RgaAnalysisSummaryReader::ReadOperands(const tinyxml2::XMLElement* element, RgaAnalysisSummary::Instruction::Operands& operands)
{
    assert(element != nullptr);
    if (!element)
    {
        return false;
    }

    const tinyxml2::XMLElement* text_element = FindChildElement(element, rga_xml::kOperandsText);
    if (text_element)
    {
        operands.text_ = GetTextContent(text_element);
    }

    for (const tinyxml2::XMLElement* operand_element = element->FirstChildElement(rga_xml::kOperand); operand_element != nullptr;
         operand_element                                 = operand_element->NextSiblingElement(rga_xml::kOperand))
    {
        RgaAnalysisSummary::Instruction::Operand operand;
        if (ReadOperand(operand_element, operand))
        {
            operands.items_.push_back(operand);
        }
    }

    return true;
}

inline bool RgaAnalysisSummaryReader::ReadOperand(const tinyxml2::XMLElement* element, RgaAnalysisSummary::Instruction::Operand& operand)
{
    assert(element != nullptr);
    if (!element)
    {
        return false;
    }

    const tinyxml2::XMLElement* order_element = FindChildElement(element, rga_xml::kOperandOrder);
    if (order_element)
    {
        operand.order_ = GetIntContent(order_element);
    }

    const tinyxml2::XMLElement* access_element = FindChildElement(element, rga_xml::kAccess);
    if (access_element)
    {
        operand.access_ = GetTextContent(access_element);
    }

    const tinyxml2::XMLElement* type_element = FindChildElement(element, rga_xml::kOperandType);
    if (type_element)
    {
        operand.type_ = GetTextContent(type_element);
    }

    const tinyxml2::XMLElement* name_element = FindChildElement(element, rga_xml::kOperandName);
    if (name_element)
    {
        operand.name_ = GetTextContent(name_element);
    }

    return true;
}

inline bool RgaAnalysisSummaryReader::ReadFunctionalGroupMetadata(
    const tinyxml2::XMLElement*                                                           element,
    std::vector<RgaAnalysisSummary::TargetArchitectureMetadata::FunctionalGroupMetadata>& groups)
{
    assert(element != nullptr);
    if (!element)
    {
        return false;
    }

    const tinyxml2::XMLElement* container = FindChildElement(element, rga_xml::kInstructionFunctionalGroups);
    if (!container)
    {
        return true;
    }

    for (const tinyxml2::XMLElement* fg_element = container->FirstChildElement(rga_xml::kInstructionFunctionalGroup);
         fg_element != nullptr;
         fg_element = fg_element->NextSiblingElement(rga_xml::kInstructionFunctionalGroup))
    {
        RgaAnalysisSummary::TargetArchitectureMetadata::FunctionalGroupMetadata meta;

        const tinyxml2::XMLElement* id_element = FindChildElement(fg_element, rga_xml::kInstructionFunctionalGroupId);
        if (id_element)
        {
            meta.id_ = GetIntContent(id_element);
        }

        const tinyxml2::XMLElement* group_name_element = FindChildElement(fg_element, rga_xml::kInstructionFunctionalGroupGroupName);
        if (group_name_element)
        {
            meta.group_name_ = GetTextContent(group_name_element);
        }

        const tinyxml2::XMLElement* sub_group_name_element = FindChildElement(fg_element, rga_xml::kInstructionFunctionalGroupSubGroupName);
        if (sub_group_name_element)
        {
            meta.sub_group_name_ = GetTextContent(sub_group_name_element);
        }

        const tinyxml2::XMLElement* desc_element = FindChildElement(fg_element, rga_xml::kInstructionFunctionalGroupDesc);
        if (desc_element)
        {
            meta.description_ = GetTextContent(desc_element);
        }

        groups.push_back(meta);
    }

    return true;
}

inline bool RgaAnalysisSummaryReader::ReadRegisterAccess(const tinyxml2::XMLElement*                                          element,
                                             const char*                                                          reg_tag,
                                             std::vector<RgaAnalysisSummary::Instruction::RegisterAccess>& reg_access)
{
    assert(element != nullptr);
    if (!element)
    {
        return false;
    }

    for (const tinyxml2::XMLElement* reg_element = element->FirstChildElement(reg_tag); reg_element != nullptr;
         reg_element                             = reg_element->NextSiblingElement(reg_tag))
    {
        RgaAnalysisSummary::Instruction::RegisterAccess reg_access_entry;
        if (ReadRegisterAccess(reg_element, reg_access_entry))
        {
            reg_access.push_back(reg_access_entry);
        }
    }

    return true;
}

inline bool RgaAnalysisSummaryReader::ReadRegisterAccess(const tinyxml2::XMLElement* element, RgaAnalysisSummary::Instruction::RegisterAccess& reg_access)
{
    assert(element != nullptr);
    if (!element)
    {
        return false;
    }

    const tinyxml2::XMLElement* index_element = FindChildElement(element, rga_xml::kIndex);
    if (index_element)
    {
        reg_access.index_ = GetIntContent(index_element);
    }

    const tinyxml2::XMLElement* name_element = FindChildElement(element, rga_xml::kName);
    if (name_element)
    {
        reg_access.name_ = GetTextContent(name_element);
    }

    const tinyxml2::XMLElement* access_element = FindChildElement(element, rga_xml::kAccess);
    if (access_element)
    {
        reg_access.access_ = GetTextContent(access_element);
    }

    return true;
}

inline bool RgaAnalysisSummaryReader::ReadCFG(const tinyxml2::XMLElement*                                      element,
                                  std::vector<RgaAnalysisSummary::ControlFlowGraph::Branch>& branches)
{
    assert(element != nullptr);
    if (!element)
    {
        return false;
    }

    for (const tinyxml2::XMLElement* branch_element = element->FirstChildElement(rga_xml::kBranch); branch_element != nullptr;
         branch_element                                 = branch_element->NextSiblingElement(rga_xml::kBranch))
    {
        RgaAnalysisSummary::ControlFlowGraph::Branch branch;
        if (ReadBranch(branch_element, branch))
        {
            branches.push_back(branch);
        }
    }

    return true;
}

inline bool RgaAnalysisSummaryReader::ReadBranch(const tinyxml2::XMLElement* element, RgaAnalysisSummary::ControlFlowGraph::Branch& branch)
{
    assert(element != nullptr);
    if (!element)
    {
        return false;
    }

    const tinyxml2::XMLElement* source_element = FindChildElement(element, rga_xml::kSourceBlockId);
    if (source_element)
    {
        branch.source_block_id_ = GetIntContent(source_element);
    }

    const tinyxml2::XMLElement* destination_element = FindChildElement(element, rga_xml::kDestinationBlockId);
    if (destination_element)
    {
        branch.destination_block_id_ = GetIntContent(destination_element);
    }

    return true;
}

inline bool RgaAnalysisSummaryReader::ReadStatistics(const tinyxml2::XMLElement* element, RgaAnalysisSummary::Statistics& stats)
{
    assert(element != nullptr);
    if (!element)
    {
        return false;
    }

    const tinyxml2::XMLElement* api_shader_hash_element = FindChildElement(element, rga_xml::kApiShaderHash);
    if (api_shader_hash_element)
    {
        stats.api_shader_hash_ = GetTextContent(api_shader_hash_element);
    }

    const tinyxml2::XMLElement* vgprs_alloc_gran_element = FindChildElement(element, rga_xml::kVgprsAllocationGranularity);
    if (vgprs_alloc_gran_element)
    {
        stats.vgprs_allocation_granularity_ = GetUInt64Content(vgprs_alloc_gran_element);
    }

    const tinyxml2::XMLElement* vgprs_allocated_element = FindChildElement(element, rga_xml::kVgprsAllocated);
    if (vgprs_allocated_element)
    {
        stats.vgprs_allocated_ = GetUInt64Content(vgprs_allocated_element);
    }

    const tinyxml2::XMLElement* vgprs_used_element = FindChildElement(element, rga_xml::kVgprsUsed);
    if (vgprs_used_element)
    {
        stats.vgprs_used_ = GetUInt64Content(vgprs_used_element);
    }

    const tinyxml2::XMLElement* vgprs_spilled_element = FindChildElement(element, rga_xml::kVgprsSpilled);
    if (vgprs_spilled_element)
    {
        stats.vgprs_spilled_ = GetUInt64Content(vgprs_spilled_element);
    }

    const tinyxml2::XMLElement* vgprs_total_element = FindChildElement(element, rga_xml::kVgprsTotal);
    if (vgprs_total_element)
    {
        stats.vgprs_total_ = GetUInt64Content(vgprs_total_element);
    }

    const tinyxml2::XMLElement* agprs_used_element = FindChildElement(element, rga_xml::kAgprsUsed);
    if (agprs_used_element)
    {
        stats.agprs_used_ = GetUInt64Content(agprs_used_element, RgaAnalysisSummary::Statistics::kValueNa);
    }

    const tinyxml2::XMLElement* agprs_total_element = FindChildElement(element, rga_xml::kAgprsTotal);
    if (agprs_total_element)
    {
        stats.agprs_total_ = GetUInt64Content(agprs_total_element, RgaAnalysisSummary::Statistics::kValueNa);
    }

    const tinyxml2::XMLElement* sgprs_allocated_element = FindChildElement(element, rga_xml::kSgprsAllocated);
    if (sgprs_allocated_element)
    {
        stats.sgprs_allocated_ = GetUInt64Content(sgprs_allocated_element);
    }

    const tinyxml2::XMLElement* sgprs_used_element = FindChildElement(element, rga_xml::kSgprsUsed);
    if (sgprs_used_element)
    {
        stats.sgprs_used_ = GetUInt64Content(sgprs_used_element);
    }

    const tinyxml2::XMLElement* sgprs_spilled_element = FindChildElement(element, rga_xml::kSgprsSpilled);
    if (sgprs_spilled_element)
    {
        stats.sgprs_spilled_ = GetUInt64Content(sgprs_spilled_element);
    }

    const tinyxml2::XMLElement* scratch_memory_element = FindChildElement(element, rga_xml::kScratchMemoryBytes);
    if (scratch_memory_element)
    {
        stats.scratch_memory_bytes_ = GetUInt64Content(scratch_memory_element);
    }

    const tinyxml2::XMLElement* lds_bytes_element = FindChildElement(element, rga_xml::kLdsBytesUsed);
    if (lds_bytes_element)
    {
        stats.lds_bytes_used_ = GetUInt64Content(lds_bytes_element);
    }

    return true;
}

inline bool RgaAnalysisSummaryReader::ReadAnalysisResults(const tinyxml2::XMLElement*                            root,
                                              std::vector<RgaAnalysisSummary::AnalysisResult>& results)
{
    assert(root != nullptr);
    if (!root)
    {
        return false;
    }

    const tinyxml2::XMLElement* results_element = root->FirstChildElement(rga_xml::kAnalysisResults);
    if (!results_element)
    {
        return false;
    }

    for (const tinyxml2::XMLElement* result_element = results_element->FirstChildElement(rga_xml::kAnalysisResult); result_element != nullptr;
         result_element                             = result_element->NextSiblingElement(rga_xml::kAnalysisResult))
    {
        RgaAnalysisSummary::AnalysisResult result;
        if (ReadAnalysisResult(result_element, result))
        {
            results.push_back(result);
        }
    }

    return true;
}

inline bool RgaAnalysisSummaryReader::ReadAnalysisResult(const tinyxml2::XMLElement* element, RgaAnalysisSummary::AnalysisResult& result)
{
    assert(element != nullptr);
    if (!element)
    {
        return false;
    }

    const tinyxml2::XMLElement* target_arch_element = FindChildElement(element, rga_xml::kTargetArchitecture);
    if (target_arch_element)
    {
        const tinyxml2::XMLElement* target_arch_name_element = FindChildElement(target_arch_element, rga_xml::kTargetArchitectureName);
        if (target_arch_name_element)
        {
            result.target_architecture_ = GetTextContent(target_arch_name_element);
        }

        const tinyxml2::XMLElement* target_arch_metadata_element = FindChildElement(target_arch_element, rga_xml::kTargetArchitectureMetadata);
        if (target_arch_metadata_element)
        {
            ReadTargetArchitectureMetadata(target_arch_metadata_element, result.target_architecture_metadata_);
        }
    }

    ReadInputs(element, result.inputs_);
    ReadOutput(element, result.output_);

    return true;
}

inline std::string RgaAnalysisSummaryReader::GetTextContent(const tinyxml2::XMLElement* element)
{
    if (!element)
    {
        return "";
    }

    const char* text = element->GetText();
    return text ? text : "";
}

inline int RgaAnalysisSummaryReader::GetIntContent(const tinyxml2::XMLElement* element, int default_value)
{
    if (!element)
    {
        return default_value;
    }

    int value = default_value;
    element->QueryIntText(&value);
    return value;
}

inline uint64_t RgaAnalysisSummaryReader::GetUInt64Content(const tinyxml2::XMLElement* element, uint64_t default_value)
{
    if (!element)
    {
        return default_value;
    }

    uint64_t value = default_value;
    element->QueryUnsigned64Text(&value);
    return value;
}

inline bool RgaAnalysisSummaryReader::GetBoolContent(const tinyxml2::XMLElement* element, bool default_value)
{
    if (!element)
    {
        return default_value;
    }

    bool value = default_value;
    element->QueryBoolText(&value);
    return value;
}

inline const tinyxml2::XMLElement* RgaAnalysisSummaryReader::FindChildElement(const tinyxml2::XMLElement* parent, const char* name)
{
    if (!parent || !name)
    {
        return nullptr;
    }

    return parent->FirstChildElement(name);
}

#endif  // RGA_COMMON_RGA_ANALYSIS_SUMMARY_READER_H_

//=============================================================================
/// Copyright (c) 2026 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for rga analysis summary struct.
//=============================================================================

// C++.
#include <algorithm>
#include <cassert>
#include <cstring>
#include <string>
#include <unordered_map>

// Common.
#include "common/rga_cli_defs.h"
#include "common/rga_shared_data_types.h"
#include "common/rga_shared_utils.h"

// Local.
#include "radeon_gpu_analyzer_backend/be_analysis_summary.h"
#include "radeon_gpu_analyzer_backend/be_isa_parser.h"
#include "radeon_gpu_analyzer_backend/be_utils.h"

// CLI.
#include "radeon_gpu_analyzer_cli/kc_utils.h"

namespace beKA
{
    // Type aliases.
    using Instruction      = RgaAnalysisSummary::Instruction;
    using InstructionPtr   = RgaAnalysisSummary::InstructionPtr;
    using Instructions     = RgaAnalysisSummary::Instructions;
    using BasicBlock       = RgaAnalysisSummary::BasicBlock;
    using BasicBlockPtr    = RgaAnalysisSummary::BasicBlockPtr;
    using BasicBlocks      = RgaAnalysisSummary::BasicBlocks;
    using Statistics       = RgaAnalysisSummary::Statistics;
    using ControlFlowGraph = RgaAnalysisSummary::ControlFlowGraph;

    // String constants.
    static const char* kStrWhitespaceChars          = " \t\n\r\f\v";
    static const char  kStrColonDelimiter           = ':';
    static const char* kStrVgprPrefix               = "v";
    static const char* kStrSgprPrefix               = "s";
    static const char* kShaeDelimiter               = "|";
    static const char* kShaeLabel                   = "label_";
    static const char* kShaeLabel2                  = "label__";
    static const char* kStrSuffixE32                = "_e32";
    static const char* kStrSuffixE64                = "_e64";
    static const char* kStrSuffixSdwa               = "_sdwa";
    static const char* kStrSuffixDpp                = "_dpp";
    static const char* kStrSuffixE64Dpp             = "_e64_dpp";
    static const char* kStrVgprsTotal               = "VGPRs total:";
    static const char* kStrSgprsTotal               = "SGPRs total:";
    static const char* kStrVgprsGranularity         = "VGPR allocation granularity:";
    static const char* kStrSgprsGranularity         = "SGPR allocation granularity:";
    static const char* kStrRegUsedAndAllocated      = "Maximum";
    static const char* kStrRegUsed                  = "used";
    static const char* kStrRegAllocatedByHw         = "allocated by HW:";
    static const char* kStrRegAllocated             = "allocated :";
    static const char* kStrNoVgprsUsed              = "No VGPRs used";
    static const char* kStrNoSgprsUsed              = "No SGPRs used";
    static const char  kShaeReadWriteAccess         = 'x';
    static const char  kShaeReadAccess              = 'v';
    static const char  kShaeWriteAccess             = '^';
    static const char  kShaeLiveAccess              = ':';
    static const char  kShaeEmpty                   = ' ';
    static const char* kStrReadWriteAccess          = "ReadWrite";
    static const char* kStrReadAccess               = "Read";
    static const char* kStrWriteAccess              = "Write";
    static const char* kStrLiveAccess               = "Live";
    static const char* kShaeStrDigraph              = "digraph {";
    static const char* kShaeStrNodeShape            = "node [shape=box]";
    static const char* kShaeStrBeginNode            = "\"n_";
    static const char* kShaeStrBracketLabel         = "\" [ label= \"";
    static const char* kShaeStrEndNode              = "\" ]";
    static const char* kShaeStrBeginEdge            = "\"n_";
    static const char* kShaeStrDirectedEdge         = "\":s -> \"n_";
    static const char* kShaeStrEndEdge              = "\";";
    static const char* kShaeStrInstructionSeparator = "\\l";

    // Returns the canonical display name for a FunctionalGroups enum value from the ISA decoder.
    static const char* FunctionalGroupName(amdisa::FunctionalGroups g)
    {
        return amdisa::FunctionalGroupNames[static_cast<int>(g)];
    }

    // Maps CSV "Functional Unit" column values to ISA spec FunctionalGroupNames.
    // The CSV uses hardware execution unit labels that don't always match the ISA
    // spec semantic categories, so explicit translation is required.
    static const std::unordered_map<std::string, std::string> kCsvFunctionalUnitToIsaGroup = {
        {FUNC_UNIT_SALU,          FunctionalGroupName(amdisa::FunctionalGroups::kFunctionalGroupSalu)},
        {FUNC_UNIT_SMEM,          FunctionalGroupName(amdisa::FunctionalGroups::kFunctionalGroupSmem)},
        {FUNC_UNIT_VALU,          FunctionalGroupName(amdisa::FunctionalGroups::kFunctionalGroupValu)},
        {FUNC_UNIT_VMEM,          FunctionalGroupName(amdisa::FunctionalGroups::kFunctionalGroupVmem)},
        {FUNC_UNIT_LDS,           FunctionalGroupName(amdisa::FunctionalGroups::kFunctionalGroupVmem)},
        {FUNC_UNIT_GDS_EXPORT,    FunctionalGroupName(amdisa::FunctionalGroups::kFunctionalGroupExport)},
        {FUNC_UNIT_INTERNAL_FLOW, FunctionalGroupName(amdisa::FunctionalGroups::kFunctionalGroupWaveControl)},
        {FUNC_UNIT_BRANCH,        FunctionalGroupName(amdisa::FunctionalGroups::kFunctionalGroupBranch)},
    };

    enum FunctionalSubgroupTier
    {
        kSubgroupTierUnknown,
        kSubgroupTierModifier,
        kSubgroupTierDirection,
        kSubgroupTierPrimaryAccess,
    };

    struct FunctionalSubgroupOrderEntry
    {
        FunctionalSubgroupTier tier;
        const char*            name;
    };

    static const char* FunctionalSubgroupName(amdisa::FunctionalSubgroups sg)
    {
        return amdisa::FunctionalSubgroupNames[static_cast<int>(sg)];
    }

    struct FunctionalSubgroupsHash
    {
        size_t operator()(amdisa::FunctionalSubgroups sg) const noexcept
        {
            return std::hash<int>{}(static_cast<int>(sg));
        }
    };

    static const std::unordered_map<amdisa::FunctionalSubgroups, FunctionalSubgroupOrderEntry, FunctionalSubgroupsHash> kFunctionalSubgroupEnumOrder = {
        {amdisa::FunctionalSubgroups::kFunctionalSubgroupBvh,            {kSubgroupTierPrimaryAccess, FunctionalSubgroupName(amdisa::FunctionalSubgroups::kFunctionalSubgroupBvh)}},
        {amdisa::FunctionalSubgroups::kFunctionalSubgroupBuffer,         {kSubgroupTierPrimaryAccess, FunctionalSubgroupName(amdisa::FunctionalSubgroups::kFunctionalSubgroupBuffer)}},
        {amdisa::FunctionalSubgroups::kFunctionalSubgroupDataShare,      {kSubgroupTierPrimaryAccess, FunctionalSubgroupName(amdisa::FunctionalSubgroups::kFunctionalSubgroupDataShare)}},
        {amdisa::FunctionalSubgroups::kFunctionalSubgroupFlat,           {kSubgroupTierPrimaryAccess, FunctionalSubgroupName(amdisa::FunctionalSubgroups::kFunctionalSubgroupFlat)}},
        {amdisa::FunctionalSubgroups::kFunctionalSubgroupSample,         {kSubgroupTierPrimaryAccess, FunctionalSubgroupName(amdisa::FunctionalSubgroups::kFunctionalSubgroupSample)}},
        {amdisa::FunctionalSubgroups::kFunctionalSubgroupTexture,        {kSubgroupTierPrimaryAccess, FunctionalSubgroupName(amdisa::FunctionalSubgroups::kFunctionalSubgroupTexture)}},
        {amdisa::FunctionalSubgroups::kFunctionalSubgroupLoad,           {kSubgroupTierDirection,     FunctionalSubgroupName(amdisa::FunctionalSubgroups::kFunctionalSubgroupLoad)}},
        {amdisa::FunctionalSubgroups::kFunctionalSubgroupStore,          {kSubgroupTierDirection,     FunctionalSubgroupName(amdisa::FunctionalSubgroups::kFunctionalSubgroupStore)}},
        {amdisa::FunctionalSubgroups::kFunctionalSubgroupAtomic,         {kSubgroupTierModifier,      FunctionalSubgroupName(amdisa::FunctionalSubgroups::kFunctionalSubgroupAtomic)}},
        {amdisa::FunctionalSubgroups::kFunctionalSubgroupFloatingPoint,  {kSubgroupTierUnknown,    FunctionalSubgroupName(amdisa::FunctionalSubgroups::kFunctionalSubgroupFloatingPoint)}},
        {amdisa::FunctionalSubgroups::kFunctionalSubgroupMFMA,           {kSubgroupTierUnknown,    FunctionalSubgroupName(amdisa::FunctionalSubgroups::kFunctionalSubgroupMFMA)}},
        {amdisa::FunctionalSubgroups::kFunctionalSubgroupStatic,         {kSubgroupTierUnknown,    FunctionalSubgroupName(amdisa::FunctionalSubgroups::kFunctionalSubgroupStatic)}},
        {amdisa::FunctionalSubgroups::kFunctionalSubgroupTranscendental, {kSubgroupTierUnknown,    FunctionalSubgroupName(amdisa::FunctionalSubgroups::kFunctionalSubgroupTranscendental)}},
        {amdisa::FunctionalSubgroups::kFunctionalSubgroupWMMA,           {kSubgroupTierUnknown,    FunctionalSubgroupName(amdisa::FunctionalSubgroups::kFunctionalSubgroupWMMA)}},
    };

    static const std::unordered_map<std::string, FunctionalSubgroupOrderEntry> kFunctionalSubgroupNameOrder = {
        {FunctionalSubgroupName(amdisa::FunctionalSubgroups::kFunctionalSubgroupBvh),            {kSubgroupTierPrimaryAccess, FunctionalSubgroupName(amdisa::FunctionalSubgroups::kFunctionalSubgroupBvh)}},
        {FunctionalSubgroupName(amdisa::FunctionalSubgroups::kFunctionalSubgroupBuffer),         {kSubgroupTierPrimaryAccess, FunctionalSubgroupName(amdisa::FunctionalSubgroups::kFunctionalSubgroupBuffer)}},
        {FunctionalSubgroupName(amdisa::FunctionalSubgroups::kFunctionalSubgroupDataShare),      {kSubgroupTierPrimaryAccess, FunctionalSubgroupName(amdisa::FunctionalSubgroups::kFunctionalSubgroupDataShare)}},
        {FunctionalSubgroupName(amdisa::FunctionalSubgroups::kFunctionalSubgroupFlat),           {kSubgroupTierPrimaryAccess, FunctionalSubgroupName(amdisa::FunctionalSubgroups::kFunctionalSubgroupFlat)}},
        {FunctionalSubgroupName(amdisa::FunctionalSubgroups::kFunctionalSubgroupSample),         {kSubgroupTierPrimaryAccess, FunctionalSubgroupName(amdisa::FunctionalSubgroups::kFunctionalSubgroupSample)}},
        {FunctionalSubgroupName(amdisa::FunctionalSubgroups::kFunctionalSubgroupTexture),        {kSubgroupTierPrimaryAccess, FunctionalSubgroupName(amdisa::FunctionalSubgroups::kFunctionalSubgroupTexture)}},
        {FunctionalSubgroupName(amdisa::FunctionalSubgroups::kFunctionalSubgroupLoad),           {kSubgroupTierDirection,     FunctionalSubgroupName(amdisa::FunctionalSubgroups::kFunctionalSubgroupLoad)}},
        {FunctionalSubgroupName(amdisa::FunctionalSubgroups::kFunctionalSubgroupStore),          {kSubgroupTierDirection,     FunctionalSubgroupName(amdisa::FunctionalSubgroups::kFunctionalSubgroupStore)}},
        {FunctionalSubgroupName(amdisa::FunctionalSubgroups::kFunctionalSubgroupAtomic),         {kSubgroupTierModifier,      FunctionalSubgroupName(amdisa::FunctionalSubgroups::kFunctionalSubgroupAtomic)}},
        {FunctionalSubgroupName(amdisa::FunctionalSubgroups::kFunctionalSubgroupFloatingPoint),  {kSubgroupTierUnknown,    FunctionalSubgroupName(amdisa::FunctionalSubgroups::kFunctionalSubgroupFloatingPoint)}},
        {FunctionalSubgroupName(amdisa::FunctionalSubgroups::kFunctionalSubgroupMFMA),           {kSubgroupTierUnknown,    FunctionalSubgroupName(amdisa::FunctionalSubgroups::kFunctionalSubgroupMFMA)}},
        {FunctionalSubgroupName(amdisa::FunctionalSubgroups::kFunctionalSubgroupStatic),         {kSubgroupTierUnknown,    FunctionalSubgroupName(amdisa::FunctionalSubgroups::kFunctionalSubgroupStatic)}},
        {FunctionalSubgroupName(amdisa::FunctionalSubgroups::kFunctionalSubgroupTranscendental), {kSubgroupTierUnknown,    FunctionalSubgroupName(amdisa::FunctionalSubgroups::kFunctionalSubgroupTranscendental)}},
        {FunctionalSubgroupName(amdisa::FunctionalSubgroups::kFunctionalSubgroupWMMA),           {kSubgroupTierUnknown,    FunctionalSubgroupName(amdisa::FunctionalSubgroups::kFunctionalSubgroupWMMA)}},
    };

    static std::string BuildFunctionalSubgroupString(const std::vector<amdisa::FunctionalSubgroups>& subgroups)
    {
        if (subgroups.empty())
        {
            return {};
        }

        std::vector<FunctionalSubgroupOrderEntry> entries;
        for (const auto& sg : subgroups)
        {
            if (sg == amdisa::FunctionalSubgroups::kFunctionalSubgroupUnknown)
            {
                continue;
            }
            auto it = kFunctionalSubgroupEnumOrder.find(sg);
            if (it != kFunctionalSubgroupEnumOrder.end())
            {
                entries.push_back(it->second);
            }
        }

        std::sort(entries.begin(), entries.end(), [](const FunctionalSubgroupOrderEntry& a, const FunctionalSubgroupOrderEntry& b) {
            return (a.tier != b.tier) ? a.tier > b.tier : std::strcmp(a.name, b.name) < 0;
        });

        std::string result;
        for (size_t i = 0; i < entries.size(); ++i)
        {
            if (i > 0)
            {
                result += ", ";
            }
            result += entries[i].name;
        }
        return result;
    }

    // Build a single comma-separated subgroup string from user-friendly name strings.
    // Filters out "-" (NOT_ASSIGNED) entries.
    // Note: Also exposed as BeAnalysisSummaryUtils::BuildFunctionalSubgroupStringFromNames().
    static std::string BuildFunctionalSubgroupStringFromNamesImpl(const std::vector<std::string>& subgroup_names)
    {
        if (subgroup_names.empty())
        {
            return {};
        }

        std::vector<FunctionalSubgroupOrderEntry> entries;
        for (const auto& name : subgroup_names)
        {
            if (name == "-" || name.empty())
            {
                continue;
            }
            auto it = kFunctionalSubgroupNameOrder.find(name);
            if (it != kFunctionalSubgroupNameOrder.end())
            {
                entries.push_back(it->second);
            }
        }

        std::sort(entries.begin(), entries.end(), [](const FunctionalSubgroupOrderEntry& a, const FunctionalSubgroupOrderEntry& b) {
            return (a.tier != b.tier) ? a.tier > b.tier : std::strcmp(a.name, b.name) < 0;
        });

        std::string result;
        for (size_t i = 0; i < entries.size(); ++i)
        {
            if (i > 0)
            {
                result += ", ";
            }
            result += entries[i].name;
        }
        return result;
    }

    bool BeAnalysisSummaryUtils::ReadKernelIsaDisassembly(const std::string& isa_filename, std::string& isa_text, LoggingCallBackFuncP callback)
    {
        return KcUtils::ReadTextFile(isa_filename, isa_text, callback);
    }

    bool BeAnalysisSummaryUtils::ParseCsvIsaDisassembly(const std::string& device,
                                                        const std::string& kernel_name,
                                                        const std::string& csv_filename,
                                                        Instructions&      instructions,
                                                        bool               create_blocks,
                                                        BasicBlocks&       blocks,
                                                        bool               verbose)
    {
        std::ifstream file(csv_filename);
        if (!file.is_open())
        {
            std::cerr << "Error: Cannot open file: " << csv_filename << std::endl;
            return false;
        }

        bool is_decoder_available = IsaParser::SetArchitecture(device);

        int         line_number         = 0;
        std::string code_block_label    = kernel_name;
        int         code_block_position = 0;

        std::string isa_line;
        // Read the header line to detect which CSV format was used.
        // When the driver produces line correlation data, the writer emits an 8-column header with
        // "Source Line Number" and "Source Line Path". When no line correlation data is available,
        // a 6-column header without those columns is emitted instead.
        std::getline(file, isa_line);
        const bool has_line_correlation = (isa_line.find(kStrCsvColumnSourceLineNumber) != std::string::npos);

        BasicBlockPtr current_code_block = nullptr;
        if (create_blocks)
        {
            current_code_block = std::make_shared<BasicBlock>(code_block_position++, code_block_label);
            blocks.emplace_back(current_code_block);
        }

        // Parse rest of the csv file.
        while (std::getline(file, isa_line))
        {
            if (isa_line.empty())
            {
                continue;
            }

            std::vector<std::string> line_tokens;
            std::vector<std::string> operands;

            RgaSharedUtils::ParseCsvLine(isa_line, line_tokens, operands);
            int       num_columns     = static_cast<int>(line_tokens.size());
            const int num_csv_columns = static_cast<int>(RgCsvFileColumns::kCount);
            // When the CSV has no line correlation columns, each data row has 6 tokens instead of 8.
            // The column offset shifts all indices after kAddress by -1 for the no-line-correlation format.
            const int col_offset = has_line_correlation ? 0 : -1;

            // When line correlation is enabled but the trailing Source Line Path field is empty,
            // the CSV line ends with a comma and std::getline produces num_csv_columns - 1 tokens.
            // Treat this the same as a full 8-column row by appending the missing empty token.
            if (has_line_correlation && num_columns == num_csv_columns - 1)
            {
                line_tokens.push_back("");
                num_columns = static_cast<int>(line_tokens.size());
            }

            switch (num_columns)
            {
            case 1:
            {
                code_block_label = line_tokens[0];
                if (code_block_label.find(":", code_block_label.size() - 1))
                {
                    code_block_label = code_block_label.erase(code_block_label.size() - 1);
                }

                if (create_blocks)
                {
                    current_code_block = std::make_shared<BasicBlock>(code_block_position++, code_block_label);
                    blocks.emplace_back(current_code_block);
                }
            }
            break;
            case num_csv_columns:
            case num_csv_columns - 2:
            {
                std::string address         = line_tokens[static_cast<int>(RgCsvFileColumns::kAddress)];
                std::string binary_encoding = line_tokens[static_cast<int>(RgCsvFileColumns::kBinaryEncoding) + col_offset];

                auto instruction = std::make_shared<Instruction>(line_number++, address, binary_encoding, code_block_label);
                if (instruction != nullptr)
                {
                    bool is_decode_successful = false;
                    // The decoded instruction info bundle.
                    amdisa::InstructionInfoBundle instruction_info_bundle;
                    std::string                   decode_error;
                    if (is_decoder_available)
                    {
                        try
                        {
                            if (IsaParser::ParseInstruction(binary_encoding, instruction_info_bundle, instruction->instruction_size_, decode_error) &&
                                !instruction_info_bundle.bundle.empty())
                            {
                                is_decode_successful = true;
                            }
                        }
                        catch (const std::exception& e)
                        {
                            std::stringstream err;
                            err << "Warning:" << e.what();
                            decode_error = err.str();
                        }
                    }

                    if (is_decode_successful)
                    {
                        const amdisa::InstructionInfo& instruction_info = instruction_info_bundle.bundle.front();

                        instruction->operands_.text_ = line_tokens[static_cast<int>(RgCsvFileColumns::kOperands) + col_offset];
                        instruction->opcode_         = line_tokens[static_cast<int>(RgCsvFileColumns::kOpcode) + col_offset];

                        instruction->semantics_.is_label_               = !instruction->block_label_.empty();
                        instruction->semantics_.branch_info_.is_branch_ = instruction_info.instruction_semantic_info.branch_info.is_branch;
                        if (instruction->semantics_.branch_info_.is_branch_)
                        {
                            if (instruction_info.instruction_semantic_info.branch_info.is_indirect)
                            {
                                instruction->semantics_.branch_info_.branch_type_ =
                                    beKA::BeIsaSpecExplorer::GetBranchTypeStr(beKA::BeIsaSpecExplorer::BranchType::kIndirectBranch);
                            }
                            else if (instruction_info.instruction_semantic_info.branch_info.IsDirect())
                            {
                                instruction->semantics_.branch_info_.branch_type_ =
                                    beKA::BeIsaSpecExplorer::GetBranchTypeStr(beKA::BeIsaSpecExplorer::BranchType::kDirectBranch);
                            }

                            if (!operands.empty())
                            {
                                std::vector<std::string> tokens;
                                BeUtils::SplitString(operands.front(), std::string(" "), tokens, true);
                                if (!tokens.empty())
                                {
                                    instruction->semantics_.branch_info_.branch_target_ = tokens[0];
                                }
                            }

                            if (!instruction_info.instruction_operands.empty())
                            {
                                Instruction::Operand operand;
                                operand.order_ = 0;

                                bool is_read    = instruction_info.instruction_operands.front().is_input;
                                bool is_written = instruction_info.instruction_operands.front().is_output;
                                if (is_read && is_written)
                                {
                                    operand.access_ = kStrReadWriteAccess;
                                }
                                else if (is_read)
                                {
                                    operand.access_ = kStrReadAccess;
                                }
                                else if (is_written)
                                {
                                    operand.access_ = kStrWriteAccess;
                                }

                                operand.type_ = instruction_info.instruction_operands.front().data_format;
                                operand.name_ = instruction->semantics_.branch_info_.branch_target_;

                                instruction->operands_.items_.emplace_back(operand);
                            }
                        }
                        else
                        {
                            int order = 0;
                            for (const auto& operand_info : instruction_info.instruction_operands)
                            {
                                Instruction::Operand operand;
                                operand.order_ = order++;

                                bool is_read    = operand_info.is_input;
                                bool is_written = operand_info.is_output;
                                if (is_read && is_written)
                                {
                                    operand.access_ = kStrReadWriteAccess;
                                }
                                else if (is_read)
                                {
                                    operand.access_ = kStrReadAccess;
                                }
                                else if (is_written)
                                {
                                    operand.access_ = kStrWriteAccess;
                                }

                                operand.type_ = operand_info.data_format;
                                operand.name_ = operand_info.operand_name;

                                instruction->operands_.items_.emplace_back(operand);
                            }
                        }

                        if (has_line_correlation)
                        {
                            const std::string& source_line_str = line_tokens[static_cast<int>(RgCsvFileColumns::kSourceLineNumber)];
                            const std::string& source_filepath = line_tokens[static_cast<int>(RgCsvFileColumns::kSourcePath)];
                            if (!source_filepath.empty() && !source_line_str.empty())
                            {
                                instruction->debug_info_.high_level_source_.file_name_   = source_filepath;
                                instruction->debug_info_.high_level_source_.line_number_ = std::stoi(source_line_str);
                            }
                        }

                        const auto functional_group    = instruction_info.functional_group_subgroup_info.isa_functional_group;
                        instruction->functional_group_ = FunctionalGroupName(functional_group);
                        instruction->functional_sub_group_ = BuildFunctionalSubgroupString(
                            instruction_info.functional_group_subgroup_info.isa_functional_subgroups);

                        instruction->instruction_description_ = instruction_info.instruction_description;
                    }
                    else
                    {
                        // Fallback when instruction decoding fails.
                        if (is_decoder_available)
                        {
                            std::cerr << decode_error << "\n";
                            if (verbose)
                            {
                                std::cerr << isa_line << "\n";
                            }
                        }

                        instruction->opcode_         = line_tokens[static_cast<int>(RgCsvFileColumns::kOpcode) + col_offset];
                        instruction->operands_.text_ = line_tokens[static_cast<int>(RgCsvFileColumns::kOperands) + col_offset];

                        // Map the CSV "Functional Unit" value to the ISA spec functional group name.
                        // The CSV uses hardware execution unit labels ("Vector ALU", "Flow Control", etc.)
                        // which differ from the ISA spec semantic categories ("Vector ALU", "Wave Control", etc.).
                        const std::string csv_functional_unit = line_tokens[static_cast<int>(RgCsvFileColumns::kFunctionalUnit) + col_offset];
                        const auto        it                  = kCsvFunctionalUnitToIsaGroup.find(csv_functional_unit);
                        if (it != kCsvFunctionalUnitToIsaGroup.end())
                        {
                            instruction->functional_group_ = it->second;
                        }

                        uint32_t dword_count = 1 + static_cast<uint32_t>(std::count(binary_encoding.begin(), binary_encoding.end(), ' '));
                        instruction->instruction_size_ = dword_count * sizeof(uint32_t);

                        instruction->semantics_.is_label_               = !instruction->block_label_.empty();
                        instruction->semantics_.branch_info_.is_branch_ = csv_functional_unit == FUNC_UNIT_BRANCH;
                        if (instruction->semantics_.branch_info_.is_branch_)
                        {
                            for (const auto& operand : operands)
                            {
                                std::vector<std::string> tokens;
                                BeUtils::SplitString(operand, std::string(" "), tokens, true);
                                if (!tokens.empty())
                                {
                                    instruction->semantics_.branch_info_.branch_type_ = tokens[0];
                                    break;
                                }
                            }
                        }

                        if (has_line_correlation)
                        {
                            const std::string& source_line_str = line_tokens[static_cast<int>(RgCsvFileColumns::kSourceLineNumber)];
                            const std::string& source_filepath = line_tokens[static_cast<int>(RgCsvFileColumns::kSourcePath)];
                            if (!source_filepath.empty() && !source_line_str.empty())
                            {
                                instruction->debug_info_.high_level_source_.file_name_   = source_filepath;
                                instruction->debug_info_.high_level_source_.line_number_ = std::stoi(source_line_str);
                            }
                        }
                    }

                    if (create_blocks && current_code_block != nullptr)
                    {
                        BasicBlock::InstructionReference reference;
                        reference.instruction_id_ = instruction->instruction_id_;
                        reference.opcode_         = instruction->opcode_;
                        reference.operands_       = instruction->operands_.text_;
                        current_code_block->instruction_references_.emplace_back(reference);
                    }

                    instructions.emplace_back(instruction);
                }

                code_block_label.clear();
            }
            break;
            default:
                // Catch cases where the type of line format is unhandled.
                assert(false);
                break;
            }
        }

        if (file.bad())
        {
            std::cerr << "Error: Failed during file read: " << csv_filename << std::endl;
            return false;
        }

        return true;
    }

    bool IsValidVgprLine(const std::string& line, std::vector<std::string>& parts)
    {
        bool ret = false;
        BeUtils::SplitString(line, std::string(kShaeDelimiter), parts, false);

        if (parts.size() >= 4 && BeUtils::IsNumericValue(BeUtils::TrimLeadingAndTrailingWhitespace(parts[0])) && BeUtils::IsNumericValue(BeUtils::TrimLeadingAndTrailingWhitespace(parts[1])))
        {
            ret = true;
        }

        return ret;
    }

    bool IsValidVgprLabel(const std::vector<std::string>& parts)
    {
        const auto& after = BeUtils::TrimLeadingAndTrailingWhitespace(parts[3]);
        // Must start with "label_" and end with ':'
        if (after.size() < 7)
        {
            return false;
        }
        if (after.compare(0, 6, kShaeLabel) != 0)
        {
            return false;
        }
        return true;
    }

    bool ReadFile(const std::string& filepath, std::vector<std::string>& lines)
    {
        lines.clear();

        std::ifstream file(filepath);
        if (!file.is_open())
            return false;

        std::string line;
        while (std::getline(file, line))
        {
            lines.push_back(line);
        }

        return true;
    }

    static bool AreOpcodesEqual(const std::string& blocks_op_code, const std::string& vgpr_op_code)
    {
        return (vgpr_op_code.compare(blocks_op_code) == 0) || (blocks_op_code.compare(vgpr_op_code + kStrSuffixE32) == 0) ||
               (blocks_op_code.compare(vgpr_op_code + kStrSuffixE64) == 0) || (blocks_op_code.compare(vgpr_op_code + kStrSuffixSdwa) == 0) ||
               (blocks_op_code.compare(vgpr_op_code + kStrSuffixDpp) == 0) || (blocks_op_code.compare(vgpr_op_code + kStrSuffixE64Dpp) == 0);
    }

    static bool GetVgprArchInformation(RgaAnalysisSummary::Statistics& stats, const std::vector<std::string>& reg_file_lines)
    {
        bool is_vgprs_total              = false;
        bool is_vgprs_granularity        = false;
        bool is_vgprs_used_and_allocated = false;
        bool is_no_vgprs_used            = false;

        for (const auto& line : reg_file_lines)
        {
            std::string trimmed = BeUtils::TrimLeadingAndTrailingWhitespace(line);
            if (trimmed.rfind(kStrVgprsTotal, 0) == 0)
            {
                size_t pos = trimmed.find(':', std::strlen(kStrVgprsTotal) - 1);
                if (pos != std::string::npos)
                {
                    stats.vgprs_total_ = std::stoull(trimmed.substr(pos + 1));
                    is_vgprs_total     = true;
                }
            }
            else if (trimmed.rfind(kStrVgprsGranularity, 0) == 0)
            {
                size_t pos = trimmed.find(':', std::strlen(kStrVgprsGranularity) - 1);
                if (pos != std::string::npos)
                {
                    stats.vgprs_allocation_granularity_ = std::stoull(trimmed.substr(pos + 1));
                    is_vgprs_granularity                = true;
                }
            }
            else if (trimmed.rfind(kStrRegUsedAndAllocated, 0) == 0)
            {
                size_t used_pos  = trimmed.find(kStrRegUsed);
                size_t comma_pos = trimmed.find(',');
                size_t      alloc_pos    = trimmed.find(kStrRegAllocatedByHw);
                size_t      alloc_token_len = std::strlen(kStrRegAllocatedByHw);
                if (alloc_pos == std::string::npos)
                {
                    alloc_pos       = trimmed.find(kStrRegAllocated);
                    alloc_token_len = std::strlen(kStrRegAllocated);
                }

                if (used_pos != std::string::npos && alloc_pos != std::string::npos)
                {
                    std::string used_str  = trimmed.substr(used_pos + std::strlen(kStrRegUsed), comma_pos - (used_pos + std::strlen(kStrRegUsed)));
                    std::string alloc_str = trimmed.substr(alloc_pos + alloc_token_len);

                    stats.vgprs_used_           = std::stoull(used_str);
                    stats.vgprs_allocated_      = std::stoull(alloc_str);
                    is_vgprs_used_and_allocated = true;
                }
            }
            else if (trimmed.find(kStrNoVgprsUsed) != std::string::npos)
            {
                is_no_vgprs_used = true;
            }
        }

        return (is_vgprs_total && is_vgprs_granularity && is_vgprs_used_and_allocated) || is_no_vgprs_used;
    }

    static bool GetSgprArchInformation(RgaAnalysisSummary::Statistics& stats, const std::vector<std::string>& reg_file_lines)
    {
        bool is_sgprs_total   = false;
        bool is_no_sgprs_used = false;

        for (const auto& line : reg_file_lines)
        {
            std::string trimmed = BeUtils::TrimLeadingAndTrailingWhitespace(line);
            if (trimmed.rfind(kStrSgprsTotal, 0) == 0)
            {
                size_t pos = trimmed.find(':', std::strlen(kStrSgprsTotal) - 1);
                if (pos != std::string::npos)
                {
                    stats.sgprs_total_ = std::stoull(trimmed.substr(pos + 1));
                    is_sgprs_total     = true;
                }
            }
            else if (trimmed.rfind(kStrSgprsGranularity, 0) == 0)
            {
                size_t pos = trimmed.find(':', std::strlen(kStrSgprsGranularity) - 1);
                if (pos != std::string::npos)
                {
                    stats.sgprs_allocation_granularity_ = std::stoull(trimmed.substr(pos + 1));
                }
            }
            else if (trimmed.rfind(kStrRegUsedAndAllocated, 0) == 0)
            {
                size_t used_pos  = trimmed.find(kStrRegUsed);
                size_t comma_pos = trimmed.find(',');
                size_t alloc_pos = trimmed.find(kStrRegAllocatedByHw);

                if (used_pos != std::string::npos && alloc_pos != std::string::npos)
                {
                    std::string used_str  = trimmed.substr(used_pos + std::strlen(kStrRegUsed), comma_pos - (used_pos + std::strlen(kStrRegUsed)));
                    std::string alloc_str = trimmed.substr(alloc_pos + std::strlen(kStrRegAllocatedByHw));

                    stats.sgprs_used_      = std::stoull(used_str);
                    stats.sgprs_allocated_ = std::stoull(alloc_str);
                }
            }
            else if (trimmed.find(kStrNoSgprsUsed) != std::string::npos)
            {
                is_no_sgprs_used = true;
            }
        }

        return is_sgprs_total || is_no_sgprs_used;
    }

    std::pair<uint64_t, uint64_t> GetRegisterPressureAndAllocation(uint64_t used, uint64_t granularity)
    {
        uint64_t allocated = 0;
        if (used % granularity == 0)
        {
            allocated = used;
        }
        else
        {
            allocated = ((used / granularity) + 1) * granularity;
        }
        return {used, allocated};
    }

    static void ParseRegisterAccessString(const std::string&                    reg_access_raw_str,
                                          beKA::BeIsaSpecExplorer::RegisterType reg_type,
                                          RgaAnalysisSummary::InstructionPtr    instruction)
    {
        if (instruction != nullptr)
        {
            auto&      register_access = (reg_type == beKA::BeIsaSpecExplorer::RegisterType::kSGPR) ? instruction->sgprs_access_ : instruction->vgprs_access_;
            const auto kStrRegisterPrefix = (reg_type == beKA::BeIsaSpecExplorer::RegisterType::kSGPR) ? kStrSgprPrefix : kStrVgprPrefix;

            for (int j = 0; j < static_cast<int>(reg_access_raw_str.size()); ++j)
            {
                int  reg_index = j - 1;
                char reg_char  = reg_access_raw_str[j];

                RgaAnalysisSummary::Instruction::RegisterAccess reg_access;
                reg_access.index_ = reg_index;
                reg_access.name_  = kStrRegisterPrefix + std::to_string(reg_index);

                switch (reg_char)
                {
                case kShaeReadWriteAccess:
                    reg_access.access_ = kStrReadWriteAccess;
                    register_access.push_back(reg_access);
                    break;
                case kShaeReadAccess:
                    reg_access.access_ = kStrReadAccess;
                    register_access.push_back(reg_access);
                    break;
                case kShaeWriteAccess:
                    reg_access.access_ = kStrWriteAccess;
                    register_access.push_back(reg_access);
                    break;
                case kShaeLiveAccess:
                    reg_access.access_ = kStrLiveAccess;
                    register_access.push_back(reg_access);
                    break;
                case kShaeEmpty:
                    break;
                default:
                    assert(false);
                    break;
                }
            }
        }
    }

    bool BeAnalysisSummaryUtils::ParseKernelRegisterFile(beKA::BeIsaSpecExplorer::RegisterType reg_type,
                                                         const std::string&                    filename,
                                                         Instructions&                         instructions,
                                                         Statistics&                           stats)
    {
        std::vector<std::string> reg_file_lines;
        bool                     ret = ReadFile(filename, reg_file_lines);

        ret = ret && (reg_type == beKA::BeIsaSpecExplorer::RegisterType::kSGPR) ? GetSgprArchInformation(stats, reg_file_lines)
                                                                                : GetVgprArchInformation(stats, reg_file_lines);

        if (ret)
        {
            // The index in the parsed reg file lines list, without any of the comment lines in the isa.
            int file_lines_index = 0;

            for (auto& instruction : instructions)
            {
                if (instruction != nullptr)
                {
                    if (file_lines_index < reg_file_lines.size())
                    {
                        std::vector<std::string> parts;
                        auto                     live_reg_line = reg_file_lines.at(file_lines_index);
                        while (!IsValidVgprLine(live_reg_line, parts))
                        {
                            file_lines_index++;
                            if (file_lines_index >= reg_file_lines.size())
                            {
                                break;
                            }
                            live_reg_line = reg_file_lines.at(file_lines_index);
                        }
                        if (file_lines_index >= reg_file_lines.size())
                        {
                            break;
                        }

                        if (IsValidVgprLine(live_reg_line, parts))
                        {
                            std::string num_live_registers   = BeUtils::TrimLeadingAndTrailingWhitespace(parts[1]);
                            std::string reg_file_instruction = BeUtils::TrimLeadingAndTrailingWhitespace(parts[3]);
                            std::string opcode               = BeUtils::SubstringBeforeFirst(reg_file_instruction, kStrWhitespaceChars);
                            if (AreOpcodesEqual(RgaSharedUtils::ToLower(instruction->opcode_), opcode))
                            {
                                const auto& reg = GetRegisterPressureAndAllocation(std::stoull(num_live_registers),
                                                                                   (reg_type == beKA::BeIsaSpecExplorer::RegisterType::kSGPR)
                                                                                       ? stats.sgprs_allocation_granularity_
                                                                                       : stats.vgprs_allocation_granularity_);
                                if (reg_type == beKA::BeIsaSpecExplorer::RegisterType::kSGPR)
                                {
                                    instruction->sgpr_pressure_   = reg.first;
                                    instruction->sgpr_allocation_ = reg.second;
                                    ParseRegisterAccessString(parts[2], reg_type, instruction);
                                }
                                else
                                {
                                    instruction->vgpr_pressure_   = reg.first;
                                    instruction->vgpr_allocation_ = reg.second;
                                    ParseRegisterAccessString(parts[2], reg_type, instruction);
                                }
                            }
                            else
                            {
                                bool is_label = IsValidVgprLabel(parts);
                                if (is_label)
                                {
                                    const std::string kShaeLabelString  = kShaeLabel2;
                                    std::string       remove_label_text = reg_file_instruction;
                                    remove_label_text                   = remove_label_text.erase(0, kShaeLabelString.size());
                                    remove_label_text                   = BeUtils::TrimLeadingAndTrailingWhitespace(BeUtils::SubstringAfterFirst(remove_label_text, kStrColonDelimiter));
                                    opcode                              = BeUtils::SubstringBeforeFirst(remove_label_text, kStrWhitespaceChars);
                                    if (AreOpcodesEqual(RgaSharedUtils::ToLower(instruction->opcode_), opcode))
                                    {
                                        const auto& reg = GetRegisterPressureAndAllocation(std::stoull(num_live_registers),
                                                                                           (reg_type == beKA::BeIsaSpecExplorer::RegisterType::kSGPR)
                                                                                               ? stats.sgprs_allocation_granularity_
                                                                                               : stats.vgprs_allocation_granularity_);
                                        if (reg_type == beKA::BeIsaSpecExplorer::RegisterType::kSGPR)
                                        {
                                            instruction->sgpr_pressure_   = reg.first;
                                            instruction->sgpr_allocation_ = reg.second;
                                            ParseRegisterAccessString(parts[2], reg_type, instruction);
                                        }
                                        else
                                        {
                                            instruction->vgpr_pressure_   = reg.first;
                                            instruction->vgpr_allocation_ = reg.second;
                                            ParseRegisterAccessString(parts[2], reg_type, instruction);
                                        }
                                    }
                                    else
                                    {
                                        continue;
                                    }
                                }
                                else
                                {
                                    continue;
                                }
                            }
                        }

                        file_lines_index++;
                    }
                }
            }
        }

        return ret;
    }

    // Cfg Dot File Parser.
    class CfgDotFileParser
    {
        // Strips Colon from the end of the string.
        static std::string StripColon(std::string s)
        {
            std::size_t colon = s.find(':');
            if (colon != std::string::npos)
            {
                s = s.substr(0, colon);
            }
            return s;
        }

    public:
        // Structure representing the control flow graph.
        struct Graph
        {
            // Cfg node.
            struct Node
            {
                std::string id;
                std::string label;
                std::string instructions;
            };

            // Cfg edge.
            struct Edge
            {
                std::string src;
                std::string dst;
            };

            // Collection of edges.
            using Edges = std::vector<Edge>;

            // Data.
            std::vector<Node> nodes_;
            Edges             edges_;

            // Create a new cfg node, or update an existing node.
            void CreateNode(const std::string& dot_file_id, const std::string& label = "", const std::string& instr = "")
            {
                nodes_.push_back({dot_file_id, label.empty() ? dot_file_id : label, instr});
            }

            // Create an cfg edge.
            void CreateEdge(const std::string& src, const std::string& dst)
            {
                edges_.push_back({src, dst});
            }
        };

        // Function to parse the cfg dot file.
        static bool Parse(const std::string& path, Graph& cfg)
        {
            std::ifstream ifs(path);
            if (!ifs.is_open())
            {
                return false;
            }

            std::string line;
            while (std::getline(ifs, line))
            {
                line = BeUtils::TrimLeadingAndTrailingWhitespace(line);
                if (line.empty() || line[0] == '/' || line[0] == '}')
                {
                    continue;
                }
                if (line.find(kShaeStrDigraph) == 0)
                {
                    continue;
                }
                if (line.find(kShaeStrNodeShape) == 0)
                {
                    continue;
                }

                // Node: "n_id" [ label ="..." ]
                std::size_t begin  = line.find(kShaeStrBeginNode);
                std::size_t middle = line.find(kShaeStrBracketLabel);
                if (begin != std::string::npos && middle != std::string::npos)
                {
                    std::size_t beg_length  = std::strlen(kShaeStrBeginNode);
                    std::string dot_file_id = line.substr(begin + beg_length, middle - (begin + beg_length));

                    std::size_t mid_length = std::strlen(kShaeStrBracketLabel);
                    std::string label      = line.substr(middle + mid_length);

                    std::string instructions;
                    if (std::getline(ifs, instructions))
                    {
                        std::size_t end_length = std::strlen(kShaeStrEndNode);
                        if (instructions.size() >= end_length && instructions.compare(instructions.size() - end_length, end_length, kShaeStrEndNode) == 0)
                        {
                            instructions.erase(instructions.size() - end_length);
                        }
                    }

                    cfg.CreateNode(dot_file_id, label, instructions);
                    continue;
                }

                // Edge: "n_a" : s-> "n_b";
                std::size_t begin_edge  = line.find(kShaeStrBeginEdge);
                std::size_t middle_edge = line.find(kShaeStrDirectedEdge);
                std::size_t end_edge    = line.find(kShaeStrEndEdge);
                if (begin_edge != std::string::npos && middle_edge != std::string::npos && end_edge != std::string::npos)
                {
                    std::size_t beg_edge_length = std::strlen(kShaeStrBeginEdge);
                    std::string left            = line.substr(begin_edge + beg_edge_length, middle_edge - (begin_edge + beg_edge_length));

                    std::size_t mid_edge_length = std::strlen(kShaeStrDirectedEdge);
                    std::string right           = line.substr(middle_edge + mid_edge_length, end_edge - (middle_edge + mid_edge_length));

                    cfg.CreateEdge(left, right);
                    continue;
                }

                return false;
            }

            return true;
        }
    };

    bool BeAnalysisSummaryUtils::ParseCfgFile(const std::string& cfg_filename, const Instructions& instructions, ControlFlowGraph& control_flow_graph)
    {
        BeAnalysisSummaryUtils  utils;
        CfgDotFileParser::Graph cfg;
        bool                    ret = CfgDotFileParser::Parse(cfg_filename, cfg);
        if (ret)
        {
            control_flow_graph                             = {};
            int                         block_number       = 0;
            int                         instruction_index  = 0;
            std::shared_ptr<BasicBlock> current_code_block = nullptr;

            for (const auto& cfg_node : cfg.nodes_)
            {
                std::vector<std::string> cfg_node_instructions;
                BeUtils::SplitString(cfg_node.instructions, std::string(kShaeStrInstructionSeparator), cfg_node_instructions, true);

                auto code_block = std::make_shared<BasicBlock>(block_number, cfg_node.label);
                if (code_block != nullptr)
                {
                    block_number++;
                    current_code_block                                             = code_block;
                    utils.block_label_to_id_map_[current_code_block->block_label_] = current_code_block->block_id_;
                    control_flow_graph.blocks_.emplace_back(current_code_block);

                    if (current_code_block->IsEntry() || current_code_block->IsExit())
                    {
                        continue;
                    }

                    for (auto cfg_node_instruction : cfg_node_instructions)
                    {
                        if (instruction_index >= instructions.size())
                        {
                            continue;
                        }

                        auto instruction = instructions.at(instruction_index);
                        while (instruction == nullptr)
                        {
                            instruction_index++;
                            if (instruction_index >= instructions.size())
                            {
                                break;
                            }
                            instruction = instructions.at(instruction_index);
                        }
                        if (instruction == nullptr || instruction_index >= instructions.size())
                        {
                            continue;
                        }

                        // Compare instruction opcodes.
                        const std::string opcode = BeUtils::SubstringBeforeFirst(cfg_node_instruction, kStrWhitespaceChars);

                        // Skip padding instructions in the flat ISA list that are absent from the DOT CFG
                        // (e.g. .long / s_nop filler bytes emitted by amdgpu-dis between basic blocks).
                        while (!AreOpcodesEqual(RgaSharedUtils::ToLower(instruction->opcode_), opcode) &&
                               instruction_index + 1 < static_cast<int>(instructions.size()))
                        {
                            instruction = instructions.at(++instruction_index);
                            while (instruction == nullptr && instruction_index + 1 < static_cast<int>(instructions.size()))
                            {
                                instruction = instructions.at(++instruction_index);
                            }
                        }

                        if (instruction != nullptr && AreOpcodesEqual(RgaSharedUtils::ToLower(instruction->opcode_), opcode))
                        {
                            BasicBlock::InstructionReference reference;
                            reference.instruction_id_ = instruction->instruction_id_;
                            reference.opcode_         = instruction->opcode_;
                            reference.operands_       = instruction->operands_.text_;
                            current_code_block->instruction_references_.emplace_back(reference);
                        }

                        instruction_index++;
                    }
                }
            }

            for (const auto& edge : cfg.edges_)
            {
                auto src_it = utils.block_label_to_id_map_.find(edge.src);
                auto dst_it = utils.block_label_to_id_map_.find(edge.dst);
                if (src_it != utils.block_label_to_id_map_.end() && dst_it != utils.block_label_to_id_map_.end())
                {
                    ControlFlowGraph::Branch branch;
                    branch.source_block_id_      = src_it->second;
                    branch.destination_block_id_ = dst_it->second;

                    // Update predecessors and successors.
                    auto src_block_it = control_flow_graph.blocks_[branch.source_block_id_];
                    auto dst_block_it = control_flow_graph.blocks_[branch.destination_block_id_];
                    if (src_block_it != nullptr && dst_block_it != nullptr)
                    {
                        src_block_it->metadata_.successors_ids_.emplace_back(branch.destination_block_id_);
                        if (!src_block_it->instruction_references_.empty())
                        {
                            auto        last_instruction_ref = src_block_it->instruction_references_.back();
                            const auto& last_instruction     = instructions.at(last_instruction_ref.instruction_id_);
                            if (last_instruction != nullptr)
                            {
                                const auto& branch_info = last_instruction->semantics_.branch_info_;
                                if (branch_info.is_branch_)
                                {
                                    src_block_it->metadata_.successors_branch_types_.push_back(branch_info.branch_type_);
                                    if (dst_block_it->IsJumpTarget())
                                    {
                                        // Set the display label only if the branch operand refers to this destination block.
                                        // This prevents self-loop branches from incorrectly labeling the fall-through successor.
                                        const std::string& operand_text = last_instruction->operands_.text_;
                                        if (operand_text.find(dst_block_it->block_label_) != std::string::npos)
                                        {
                                            dst_block_it->display_label_ = operand_text;
                                        }
                                    }
                                }
                                else
                                {
                                    src_block_it->metadata_.successors_branch_types_.push_back(
                                        beKA::BeIsaSpecExplorer::GetBranchTypeStr(beKA::BeIsaSpecExplorer::BranchType::kFallThrough));
                                }
                            }
                        }
                        else
                        {
                            src_block_it->metadata_.successors_branch_types_.push_back(
                                beKA::BeIsaSpecExplorer::GetBranchTypeStr(beKA::BeIsaSpecExplorer::BranchType::kFallThrough));
                        }

                        dst_block_it->metadata_.predecessors_ids_.emplace_back(branch.source_block_id_);
                    }

                    control_flow_graph.branches_.emplace_back(std::move(branch));
                }
            }

            // Per block register pressure.
            for (auto& block : control_flow_graph.blocks_)
            {
                if (block != nullptr && !block->instruction_references_.empty())
                {
                    // Pressure at first instruction in the block.
                    auto        first_instruction_ref = block->instruction_references_.front();
                    const auto& first_instruction     = instructions.at(first_instruction_ref.instruction_id_);
                    if (first_instruction != nullptr)
                    {
                        block->metadata_.live_registers_in_.vgpr_pressure_ = first_instruction->vgpr_pressure_;
                        block->metadata_.live_registers_in_.vgprs_access_  = first_instruction->vgprs_access_;
                        block->metadata_.live_registers_in_.sgpr_pressure_ = first_instruction->sgpr_pressure_;
                        block->metadata_.live_registers_in_.sgprs_access_  = first_instruction->sgprs_access_;
                    }

                    // Pressure at last instruction in the block.
                    auto        last_instruction_ref = block->instruction_references_.back();
                    const auto& last_instruction     = instructions.at(last_instruction_ref.instruction_id_);
                    if (last_instruction != nullptr)
                    {
                        block->metadata_.live_registers_out_.vgpr_pressure_ = last_instruction->vgpr_pressure_;
                        block->metadata_.live_registers_out_.vgprs_access_  = last_instruction->vgprs_access_;
                        block->metadata_.live_registers_out_.sgpr_pressure_ = last_instruction->sgpr_pressure_;
                        block->metadata_.live_registers_out_.sgprs_access_  = last_instruction->sgprs_access_;
                    }
                }
            }
        }

        return ret;
    }

    bool BeAnalysisSummaryUtils::ParseStatsFile(const std::string& stats_filename, Statistics& stats)
    {
        beKA::AnalysisData statistics;
        bool               ret = KcUtils::ReadStatisticsFile(stats_filename, statistics);
        if (ret)
        {
            // Max of used vgpr registers per the vgpr file and the stats file.
            stats.vgprs_used_    = std::max(stats.vgprs_used_, statistics.num_vgprs_used);
            stats.vgprs_spilled_ = statistics.num_vgpr_spills;

            // AGPR counts from the stats file.
            static_assert(beKA::kCalValue64Na == RgaAnalysisSummary::Statistics::kValueNa,
                          "beKA::kCalValue64Na must match RgaAnalysisSummary::Statistics::kValueNa");
            stats.agprs_used_  = statistics.num_agprs_used;
            stats.agprs_total_ = statistics.num_agprs_available;

            // Max of used sgpr registers per the sgpr file and the stats file.
            stats.sgprs_used_    = std::max(stats.sgprs_used_, statistics.num_sgprs_used);
            stats.sgprs_spilled_ = statistics.num_sgpr_spills;

            stats.scratch_memory_bytes_ = statistics.scratch_memory_used;
            stats.lds_bytes_used_       = statistics.lds_size_used;
        }
        return ret;
    }

    std::string BeAnalysisSummaryUtils::BuildFunctionalSubgroupStringFromNames(const std::vector<std::string>& subgroup_names)
    {
        return BuildFunctionalSubgroupStringFromNamesImpl(subgroup_names);
    }

}  // namespace beKA

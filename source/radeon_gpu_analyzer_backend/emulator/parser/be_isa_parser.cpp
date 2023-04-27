//=================================================================
// Copyright 2020 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================
#define _HAS_AUTO_PTR_ETC 1

// C++.
#include <fstream>
#include <string>

// Boost.
#include <boost/regex.hpp>

#ifdef _WIN32
    #pragma warning(push)
    #pragma warning(disable:4309)
#endif
#include "amdt_os_wrappers/Include/osDebugLog.h"
#ifdef _WIN32
    #pragma warning(pop)
#endif

// Local.
#include "be_string_constants.h"
#include "be_isa_parser.h"
#include "be_parser_si_sop2.h"
#include "be_parser_si_sopk.h"
#include "be_parser_si_sop1.h"
#include "be_parser_si_sopp.h"
#include "be_parser_si_sopc.h"
#include "be_parser_si_smrd.h"
#include "be_parser_si_vintrp.h"
#include "be_parser_si_ds.h"
#include "be_parser_si_mubuf.h"
#include "be_parser_si_mtbuf.h"
#include "be_parser_si_mimg.h"
#include "be_parser_si_exp.h"
#include "be_parser_si_vop.h"
#include "be_parser_flat.h"

// *****************************************
// *** INTERNALLY LINKED SYMBOLS - START ***
// *****************************************

static const std::string kIsaLabelToken1 = "label_";
static const std::string kIsaLabelToken2 = "BB";
static const std::string kIsaBranchToken  = "branch";
static const std::string kIsaCallToken    = "call";

static bool ExtractRuntimeChangedNumOfGprs(const std::string& isa_line, unsigned int& num_gprs)
{
    bool ret = false;
    const std::string kChangedByRuntimeToken = "modified by runtime to be ";
    size_t pos_begin = isa_line.find(kChangedByRuntimeToken);
    if (pos_begin != std::string::npos)
    {
        // Handles the case where the number of SGPRs was changed by the runtime.
        pos_begin += kChangedByRuntimeToken.size();
        size_t pos_end = isa_line.find(";", pos_begin);
        if (pos_end > pos_begin)
        {
            const std::string kNumAsText = isa_line.substr(pos_begin, pos_end - pos_begin);
            num_gprs = std::stoul(kNumAsText);
            ret = true;
        }
    }

    return ret;
}

// Trim from the start.
static std::string& trimStart(std::string& str_to_trim)
{
    str_to_trim.erase(str_to_trim.begin(), std::find_if(str_to_trim.begin(), str_to_trim.end(), [](int c) { return !std::isspace(c); }));
    return str_to_trim;
}

// Trim from the end.
static std::string& trimEnd(std::string& str_to_trim)
{
    str_to_trim.erase(std::find_if(str_to_trim.rbegin(), str_to_trim.rend(), [](int c) { return !std::isspace(c); }).base(), str_to_trim.end());
    return str_to_trim;
}

// Trim from both sides.
static std::string trimStr(const std::string& str_to_trim)
{
    std::string ret = str_to_trim;
    return trimStart(trimEnd(ret));
}

// Split the given instruction into its building blocks: opcode, operands, binary representation and offset.
static bool ExtractBuildingBlocks(const std::string& isa_instruction, std::string& instruction_opcode,
    std::string& params, std::string& binary_representation, std::string& offset)
{
    bool ret = false;

    // Clear the white spaces.
    std::string trimmed_instruction = trimStr(isa_instruction);

    // Find the first white space.
    size_t index_start = trimmed_instruction.find(' ');
    if (index_start != std::string::npos)
    {
        // Extract the instruction.
        instruction_opcode = trimmed_instruction.substr(0, index_start);

        // Extract the parameters.
        bool is_llpc_disassembly = false;
        size_t index_end_llpc = trimmed_instruction.find(";");
        size_t index_end = trimmed_instruction.find("//");
        if (index_end == std::string::npos)
        {
            index_end = index_end_llpc;
            is_llpc_disassembly = true;
        }
        if (index_end != std::string::npos)
        {
            size_t substr_length = index_end - index_start;
            if (substr_length > 0)
            {
                params = trimmed_instruction.substr(index_start, substr_length);

                // Clear white spaces.
                trimStart(trimEnd(params));

                if (!is_llpc_disassembly)
                {
                    // Extract the offset.
                    index_start = index_end + 3;
                    index_end = trimmed_instruction.find(':', index_end);

                    if (index_start != std::string::npos)
                    {
                        substr_length = index_end - index_start;
                        if (substr_length > 0)
                        {
                            offset = trimmed_instruction.substr(index_start, substr_length);

                            // Extract the binary representation.
                            binary_representation = trimmed_instruction.substr(index_end + 2);

                            // We are done.
                            ret = true;
                        }
                    }
                }
                else
                {
                    size_t binary_representation_start = index_end_llpc + 2;
                    assert(binary_representation_start < trimmed_instruction.size());
                    if (binary_representation_start < trimmed_instruction.size())
                    {
                        binary_representation = trimmed_instruction.substr(binary_representation_start);
                        ret = true;
                    }
                }
            }
        }
    }

    return ret;
}

// ***************************************
// *** INTERNALLY LINKED SYMBOLS - END ***
// ***************************************

ParserIsa::ParserIsa(ParserSi::LoggingCallBackFuncP log_function)
{
    ParserSi::SetLog(log_function);
    parser_si_[Instruction::kInstructionSetSop2] = new ParserSiSop2();
    parser_si_[Instruction::kInstructionSetSopk] = new ParserSiSopk();
    parser_si_[Instruction::kInstructionSetSop1] = new ParserSiSop1();
    parser_si_[Instruction::kInstructionSetSopc] = new ParserSiSopC();
    parser_si_[Instruction::kInstructionSetSopp] = new ParserSiSopp();
    parser_si_[Instruction::kInstructionSetSmrd] = new ParserSiSmrd();
    parser_si_[Instruction::kInstructionSetVintrp] = new ParserSiVintrp();
    parser_si_[Instruction::kInstructionSetDs] = new ParserSiDs();
    parser_si_[Instruction::kInstructionSetMubuf] = new ParserSiMubuf();
    parser_si_[Instruction::kInstructionSetMtbuf] = new ParserSiMtbuf();
    parser_si_[Instruction::kInstructionSetMimg] = new ParserSiMimg();
    parser_si_[Instruction::kInstructionSetExp] = new ParserSiExp();
    parser_si_[Instruction::kInstructionSetVop] = new ParserSiVop();
    parser_si_[Instruction::kInstructionSetFlat] = new ParserFLAT();
}

ParserIsa::~ParserIsa()
{
    ParserSi::SetLog(NULL);
    delete parser_si_[Instruction::kInstructionSetSop2];
    delete parser_si_[Instruction::kInstructionSetSopk];
    delete parser_si_[Instruction::kInstructionSetSop1];
    delete parser_si_[Instruction::kInstructionSetSopc];
    delete parser_si_[Instruction::kInstructionSetSopp];
    delete parser_si_[Instruction::kInstructionSetSmrd];
    delete parser_si_[Instruction::kInstructionSetVintrp];
    delete parser_si_[Instruction::kInstructionSetDs];
    delete parser_si_[Instruction::kInstructionSetMubuf];
    delete parser_si_[Instruction::kInstructionSetMtbuf];
    delete parser_si_[Instruction::kInstructionSetMimg];
    delete parser_si_[Instruction::kInstructionSetExp];
    delete parser_si_[Instruction::kInstructionSetVop];
    delete parser_si_[Instruction::kInstructionSetFlat];
}

bool ParserIsa::Parse(const std::string& isa_line, GDT_HW_GENERATION asic_generation, Instruction::Instruction32Bit hex_instruction,
    const std::string& src_line, int src_line_number, bool is_literal_32b, uint32_t literal_32b,
    int label /*=kNoLabel*/, int goto_global /*=kNoLabel*/, int line_count/* = 0*/)
{
    bool ret = false;
    Instruction* instruction = NULL;
    ParserSi::InstructionEncoding instruction_encoding = ParserSi::GetInstructionEncoding(hex_instruction);

    if (instruction_encoding == ParserSi::kInstructionEncodingSop2)
    {
        parser_si_[Instruction::kInstructionSetSop2]->Parse(asic_generation, hex_instruction, instruction, is_literal_32b, literal_32b, label, goto_global);
    }
    else if (instruction_encoding == ParserSi::kInstructionEncodingSopk)
    {
        parser_si_[Instruction::kInstructionSetSopk]->Parse(asic_generation, hex_instruction, instruction, is_literal_32b, literal_32b, label, goto_global);
    }
    else if (instruction_encoding == ParserSi::kInstructionEncodingSop1)
    {
        parser_si_[Instruction::kInstructionSetSop1]->Parse(asic_generation, hex_instruction, instruction, is_literal_32b, literal_32b, label, goto_global);
    }
    else if (instruction_encoding == ParserSi::kInstructionEncodingSopc)
    {
        parser_si_[Instruction::kInstructionSetSopc]->Parse(asic_generation, hex_instruction, instruction, is_literal_32b, literal_32b, label, goto_global);
    }
    else if (instruction_encoding == ParserSi::kInstructionEncodingSopp)
    {
        parser_si_[Instruction::kInstructionSetSopp]->Parse(asic_generation, hex_instruction, instruction, is_literal_32b, literal_32b, label, goto_global);
    }
    else if ((instruction_encoding == ParserSi::kInstructionEncodingSmrd) || (instruction_encoding == ParserSi::kViInstructionEncodingSmem))
    {
        parser_si_[Instruction::kInstructionSetSmrd]->Parse(asic_generation, hex_instruction, instruction, is_literal_32b, literal_32b, label, goto_global);
    }
    else if ((instruction_encoding == ParserSi::kInstructionEncodingVop2) ||
             (instruction_encoding == ParserSi::kInstructionEncodingVop1) ||
             (instruction_encoding == ParserSi::kInstructionEncodingVopc))
    {
        parser_si_[Instruction::kInstructionSetVop]->Parse(asic_generation, hex_instruction, instruction, is_literal_32b, literal_32b, label, goto_global);
    }
    else if ((instruction_encoding == ParserSi::kInstructionEncodingVintrp) ||
             (instruction_encoding == ParserSi::kViInstructionEncodingVintrp))
    {
        parser_si_[Instruction::kInstructionSetVintrp]->Parse(asic_generation, hex_instruction, instruction, is_literal_32b, literal_32b, label, goto_global);
    }

    if (nullptr == instruction)
    {
        // Push an instruction of an arbitrary type into the collection so the ISA view can display the text of this instruction.
        // The textual part that is displayed in the ISA view is added in the next if block below.
        instruction = new SIVOP1Instruction(32, VOPInstruction::kEncodingVop1, SIVOP1Instruction::kNOP, kNoLabel, kNoLabel);
    }

    if (instruction != NULL)
    {
        instruction->SetLineNumber(line_count);
        instruction->SetSrcLineInfo(src_line_number, src_line);
        instructions_.push_back(instruction);

        std::string opcode;
        std::string params;
        std::string binary_representation;
        std::string offset;

        ret = ExtractBuildingBlocks(isa_line, opcode, params, binary_representation, offset);

        if (ret)
        {
            // Set the ISA instruction's string representation.
            instruction->SetInstructionStringRepresentation(opcode, params, binary_representation, offset);
        }
    }

    return ret;
}

bool ParserIsa::Parse(const std::string& isa_line, GDT_HW_GENERATION asic_generation, Instruction::Instruction64Bit hex_instruction,
    const std::string& src_line, int src_line_number, int label /*=kNoLabel*/,
    int goto_label /*=kNoLabel*/, int line_count /*= 0*/)
{
    Instruction* instruction = nullptr;

    // Instruction encoding appears only in low 32 bits of instruction
    ParserSi::InstructionEncoding instruction_encoding = ParserSi::GetInstructionEncoding(static_cast<Instruction::Instruction32Bit>(hex_instruction));

    switch (instruction_encoding)
    {
        case ParserSi::kInstructionEncodingDs:
            parser_si_[Instruction::kInstructionSetDs]->Parse(asic_generation, hex_instruction, instruction, label, goto_label);
            break;

        case ParserSi::kInstructionEncodingMubuf:
            parser_si_[Instruction::kInstructionSetMubuf]->Parse(asic_generation, hex_instruction, instruction, label, goto_label);
            break;

        case ParserSi::kInstructionEncodingMimg:
            parser_si_[Instruction::kInstructionSetMimg]->Parse(asic_generation, hex_instruction, instruction, label, goto_label);
            break;

        case ParserSi::kInstructionEncodingMtbuf:
            parser_si_[Instruction::kInstructionSetMtbuf]->Parse(asic_generation, hex_instruction, instruction, label, goto_label);
            break;

        case ParserSi::InstructionEncodingExp:
            parser_si_[Instruction::kInstructionSetExp]->Parse(asic_generation, hex_instruction, instruction, label, goto_label);
            break;

        case ParserSi::kInstructionEncodingVop3:
            parser_si_[Instruction::kInstructionSetVop]->Parse(asic_generation, hex_instruction, instruction, label, goto_label);
            break;

        case ParserSi::kViInstructionEncodingSmem:
            parser_si_[Instruction::kInstructionSetSmrd]->Parse(asic_generation, hex_instruction, instruction, label, goto_label);
            break;

        case ParserSi::kViInstructionEncodingFlat:
            parser_si_[Instruction::kInstructionSetFlat]->Parse(asic_generation, hex_instruction, instruction, label, goto_label);
            break;

        default:
            break;
    }

    if (nullptr == instruction)
    {
        // Push an instruction of an arbitrary type into the collection so the ISA view can display the text of this instruction.
        // The textual part that is displayed in the ISA view is added in the next if block below.
        instruction = new SIVOP1Instruction(32, VOPInstruction::kEncodingVop1, SIVOP1Instruction::kNOP, kNoLabel, kNoLabel);
    }

    if (instruction != NULL)
    {
        if (kNoLabel == instruction->GetLabel())
        {
            instruction->SetLineNumber(line_count);
            instruction->SetSrcLineInfo(src_line_number, src_line);
            instructions_.push_back(instruction);

            // Set the ISA instruction's string representation.
            std::string opcode;
            std::string params;
            std::string binary_representation;
            std::string offset;
            bool ret = ExtractBuildingBlocks(isa_line, opcode, params, binary_representation, offset);

            if (ret)
            {
                instruction->SetInstructionStringRepresentation(opcode, params, binary_representation, offset);
            }
        }
    }

    return true;
}

bool ParserIsa::Parse(const std::string& isa)
{
    ResetInstsCounters();

    bool ret = ParseToVector(isa);
    if (ret)
    {
        ret = isa_graph_.BuildISAProgramStructure(instructions_);
    }
    return ret;
}

bool ParserIsa::ParseForSize(const std::string& isa)
{
    bool ret = false;
    ResetInstsCounters();
    boost::regex code_len_byte_ex("([[:blank:]]*codeLenInByte[[:blank:]]*=[[:blank:]]*)([[:digit:]]*)");
    boost::regex code_len_byte_ni("([[:blank:]]*CodeLen[[:blank:]]*=[[:blank:]]*)([[:digit:]]*)");

    // SCPC disassembly format:
    // v_add_u32     v1, s[2:3], v0, s0    // 000000000130: D1190201 00000100   <--- 64-bit instruction
    // v_mov_b32     v0, 0                 // 000000000138: 7E000280            <--- 32-bit instruction
    boost::regex instruction_annotation_scpc("//[[:blank:]]*[[:xdigit:]]{12}:[[:blank:]]*([[:xdigit:]]{8})([[:blank:]]+[[:xdigit:]]{8}){0,1}");

    // LLPC disassembly format:
    // v_add_u32_e32 v0, s2, v0; 68000002                 <--- 64-bit instruction
    // exp pos0 v1, v0, v3, v2 done; C40008CF 02030001    <--- 32-bit instruction
    boost::regex instruction_annotation_llpc(";[[:blank:]]*([[:xdigit:]]{8})([[:blank:]]+[[:xdigit:]]{8}){0,1}");

    std::istringstream isa_stream(isa);
    boost::smatch match_instruction;
    std::string isa_line;
    int  isa_size = 0;

    while (getline(isa_stream, isa_line))
    {
        if (boost::regex_search(isa_line, match_instruction, code_len_byte_ex))
        {
            std::string code_len_text(match_instruction[2].first, match_instruction[2].second);
            code_len_ = atoi(code_len_text.c_str());
            ret = true;
            break;
        }
        else if (boost::regex_search(isa_line, match_instruction, code_len_byte_ni))
        {
            std::string code_len_text(match_instruction[2].first, match_instruction[2].second);
            code_len_ = atoi(code_len_text.c_str());
            ret = true;
            break;
        }
        else if (boost::regex_search(isa_line, match_instruction, instruction_annotation_scpc) ||
            boost::regex_search(isa_line, match_instruction, instruction_annotation_llpc))
        {
            // Count size of instructions "manually" if ISA size is not provided by disassembler.
            int instruction_size = match_instruction[(int) match_instruction.size() - 1].matched ? 8 : 4;
            isa_size += instruction_size;
            ret = true;
        }
    }

    if (ret && isa_size != 0)
    {
        code_len_ = isa_size;
    }

    return ret;
}

static bool  GetSourceLineInfo(const std::string& isa_line, const std::string& prev_isa_line, std::string& src_line, int& src_line_number)
{
    bool ret = false;

    // Source line info has the following format:
    // ; C:\DEV\work\line_numbers\test.cl:4    <-- prevIsaLine
    // ; A[0] = 0.0f;                          <-- isaLine
    const size_t src_line_offset = 2;
    size_t colon_offset = 0;
    if (prev_isa_line.find(';') == 0 && isa_line.find(';') == 0 && ((colon_offset = prev_isa_line.rfind(':')) != std::string::npos))
    {
        src_line = isa_line.substr(src_line_offset);
        src_line_number = std::atoi(prev_isa_line.substr(colon_offset + 1).c_str());
        ret = true;
    }

    return ret;
}

bool ParserIsa::ParseToVector(const std::string& isa)
{
    int line_count = 0, src_line_number = 0;
    Instruction::Instruction32Bit inst32;
    Instruction::Instruction64Bit inst64;

    std::istringstream isa_stream(isa);
    std::string isa_line, src_line;
    bool isa_code_proc = false, parse_ok = true, gpr_proc = false, is_vgpr_found = false, is_sgpr_found = false, is_code_len_found = false;
    int label = kNoLabel, goto_label = kNoLabel;

    boost::smatch match_instruction;

    boost::regex regex_inst32_48("([[:print:]]*// [[:print:]]{12}: )([[:print:]]{8})");
    boost::regex regex_inst32_48_llpc("([[:print:]]*; )([[:print:]]{8})");

    boost::regex regex_inst64_48("([[:print:]]*// [[:print:]]{12}: )([[:print:]]{8})( )([[:print:]]{8})");
    boost::regex regex_inst64_48_llpc("([[:print:]]*; )([[:print:]]{8})( )([[:print:]]{8})");

    boost::regex regex_inst32("([[:print:]]*// [[:print:]]{8}: )([[:print:]]{8})");
    boost::regex regex_inst_64("([[:print:]]*// [[:print:]]{8}: )([[:print:]]{8})( )([[:print:]]{8})");
    boost::regex regex_vgpr("([[:blank:]]*NumVgprs[[:blank:]]*=[[:blank:]]*)([[:digit:]]*)");
    boost::regex regex_sgprs("([[:blank:]]*NumSgprs[[:blank:]]*=[[:blank:]]*)([[:digit:]]*)");
    boost::regex regex_code_length_bytes("([[:blank:]]*codeLenInByte[[:blank:]]*=[[:blank:]]*)([[:digit:]]*)");
    boost::regex regex_code_length_bytes_ni("([[:blank:]]*CodeLen[[:blank:]]*=[[:blank:]]*)([[:digit:]]*)");

    std::string isa_start;
    std::string isa_end;

    bool is_llpc_disassembly = false;
    if (isa.rfind("Disassembly for _amdgpu_", 0) == 0)
    {
        is_llpc_disassembly = true;
    }

    if (isa.find("Disassembly --------------------") != std::string::npos)
    {
        // OCL or non DX
        /// ISA SI + "starts" from "; -------- Disassembly --------------------"
        /// ISA NI "starts" from "; --------  Disassembly --------------------" notice extra space
        isa_start = "Disassembly --------------------";

        /// ISA "ends" with "; ----------------- CS Data ------------------------"
        isa_end = "; ----------------- CS Data ------------------------";

        // For Vulkan, we don't have the CS Data section.
        if (isa.find(isa_end) == std::string::npos)
        {
            isa_end = "end";
        }
    }
    else if (isa.find(kStrHsailDisassemblyTokenStart) != std::string::npos)
    {
        // Shader entry point in HSAIL disassembly.
        isa_start = kStrHsailDisassemblyTokenStart;

        // End of shader token in HSAIL disassembly.
        isa_end = kStrHsailDisassemblyTokenEnd;
    }
    else
    {
        // beginning of shader
        isa_start = "shader ";

        // parse to the end
        isa_end = "end";
    }

    GDT_HW_GENERATION asicGen = GDT_HW_GENERATION_NONE;
    // Asic generation is in "asic(".
    const std::string kAsicGenStr("asic(");
    std::string  prev_line = "";

    while (getline(isa_stream, isa_line))
    {
        line_count++;

        if (!isa_code_proc && !gpr_proc && strstr(isa_line.c_str(), isa_start.c_str()) == NULL ||
            GetSourceLineInfo(isa_line, prev_line, src_line, src_line_number))
        {
            continue;
        }
        else if (!isa_code_proc && !gpr_proc)
        {
            isa_code_proc = true;
        }
        else if (isa_code_proc && strstr(isa_line.c_str(), isa_end.c_str()) != NULL
                 && isa_line.find("//") == std::string::npos)
        {
            // at least one line of valid code detected
            gpr_proc = true;
            isa_code_proc = false;
        }
        else if (isa_code_proc)
        {
            /// check generation first
            size_t pos = isa_line.find(kAsicGenStr);
            if (pos != std::string::npos)
            {
                std::string tmp = isa_line.substr(pos + 5, 2);

                if (tmp == "SI")
                {
                    asicGen = GDT_HW_GENERATION_SOUTHERNISLAND;
                }
                else if (tmp == "CI")
                {
                    asicGen = GDT_HW_GENERATION_SEAISLAND;    // we consider SI and CI to be the same when parsing
                }
                else if (tmp == "VI")
                {
                    asicGen = GDT_HW_GENERATION_VOLCANICISLAND;
                }
            }

            std::stringstream instruction_stream;
            std::string::const_iterator isa_line_start = isa_line.begin();
            std::string::const_iterator isa_line_end = isa_line.end();
            bool is_instruction_parsed = true;

            if (label == kNoLabel)
            {
                label = GetLabel(isa_line);
            }

            goto_label = GetGotoLabel(isa_line);

            if ((boost::regex_search(isa_line_start, isa_line_end, match_instruction, regex_inst_64)) ||
                (boost::regex_search(isa_line_start, isa_line_end, match_instruction, regex_inst64_48)) ||
                (is_llpc_disassembly && boost::regex_search(isa_line_start, isa_line_end, match_instruction, regex_inst64_48_llpc)))
            {
                // This is either inst64Ex or inst64_48Ex.
                std::string inst32_text_lower_case(match_instruction[2].first, match_instruction[2].second);
                std::string inst32_text_upper_case(match_instruction[4].first, match_instruction[4].second);
                instruction_stream << std::hex << inst32_text_upper_case << inst32_text_lower_case;
                instruction_stream >> inst64;
                is_instruction_parsed = Parse(isa_line, asicGen, inst64, src_line, src_line_number, label, goto_label, line_count);
                label = goto_label = kNoLabel;

                if (!is_instruction_parsed)
                {
                    uint32_t literal_32b = 0;
                    instruction_stream.clear();
                    instruction_stream << std::hex << inst32_text_lower_case;
                    instruction_stream >> inst32;

                    instruction_stream.clear();
                    instruction_stream << std::hex << inst32_text_upper_case;
                    instruction_stream >> literal_32b;

                    is_instruction_parsed = Parse(isa_line, asicGen, inst32, src_line, src_line_number, true, literal_32b, label, goto_label, line_count);
                    label = goto_label = kNoLabel;

                }
            }
            else if (is_instruction_parsed && (boost::regex_search(isa_line_start, isa_line_end, match_instruction, regex_inst32) ||
                boost::regex_search(isa_line_start, isa_line_end, match_instruction, regex_inst32_48) ||
                (is_llpc_disassembly && boost::regex_search(isa_line_start, isa_line_end, match_instruction, regex_inst32_48_llpc))))
            {
                // This is either inst32Ex or inst32_48Ex.
                std::string inst32_ext(match_instruction[2].first, match_instruction[2].second);

                instruction_stream << std::hex << inst32_ext;
                instruction_stream >> inst32;
                is_instruction_parsed = Parse(isa_line, asicGen, inst32, src_line, src_line_number, false, 0, label, goto_label, line_count);
                label = goto_label = kNoLabel;
            }
            else if (label != kNoLabel)
            {
                Instruction* instruction = nullptr;
                std::string trimmed_isa_line = trimStr(isa_line);
                instruction = new Instruction(trimmed_isa_line);
                instructions_.push_back(instruction);
                label = goto_label = kNoLabel;
            }

            parse_ok &= is_instruction_parsed;
        }
        else if (is_sgpr_found && is_vgpr_found && is_code_len_found)
        {
            break;
        }
        else if (gpr_proc)
        {
            std::string::const_iterator isa_line_start = isa_line.begin();
            std::string::const_iterator isa_line_end   = isa_line.end();

            if (boost::regex_search(isa_line_start, isa_line_end, match_instruction, regex_vgpr))
            {
                // Mark the VGPR section as found.
                is_vgpr_found = true;
                vgprs_ = 0;

                // Check if the number of VGPRs was changed by the runtime.
                bool is_changed_by_runtime = ExtractRuntimeChangedNumOfGprs(isa_line, vgprs_);
                if (!is_changed_by_runtime)
                {
                    // If the value was not changed, extract the original value.
                    std::string vgprs_text(match_instruction[2].first, match_instruction[2].second);
                    vgprs_ = atoi(vgprs_text.c_str());
                }
            }
            else if (boost::regex_search(isa_line_start, isa_line_end, match_instruction, regex_sgprs))
            {
                // Mark the SGPR section as found.
                is_sgpr_found = true;
                sgprs_ = 0;

                // Check if the number of SGPRs was changed by the runtime.
                bool is_changed_by_runtime = ExtractRuntimeChangedNumOfGprs(isa_line, sgprs_);
                if (!is_changed_by_runtime)
                {
                    // If the value was not changed, extract the original value.
                    std::string sgprs_text(match_instruction[2].first, match_instruction[2].second);
                    sgprs_ = atoi(sgprs_text.c_str());
                }
            }
            else if (boost::regex_search(isa_line_start, isa_line_end, match_instruction, regex_code_length_bytes))
            {
                is_code_len_found = true;
                std::string code_length_text(match_instruction[2].first, match_instruction[2].second);
                code_len_ = atoi(code_length_text.c_str());
            }
            else if (boost::regex_search(isa_line_start, isa_line_end, match_instruction, regex_code_length_bytes_ni))
            {
                is_code_len_found = true;
                std::string code_length_text(match_instruction[2].first, match_instruction[2].second);
                code_len_ = atoi(code_length_text.c_str());
            }
        }
        prev_line = isa_line;
    }

    return parse_ok;
}

void ParserIsa::ResetInstsCounters()
{
    std::vector<Instruction*>::const_iterator instruction_begin = instructions_.begin();
    for (; instruction_begin != instructions_.end(); ++instruction_begin)
    {
        if (NULL != (*instruction_begin))
        {
            delete *instruction_begin;
        }
    }

    instructions_.clear();
    sgprs_ = 0;
    vgprs_ = 0;

    isa_graph_.DestroyISAProgramStructure();
}

int ParserIsa::GetLabel(const std::string& isa_line)
{
    const int kHsailIsaOffset = 2;
    int ret = kNoLabel;
    size_t  offset = 0;
    std::stringstream  stream;

    if ((offset = isa_line.find(kIsaLabelToken1)) == 0 || offset == kHsailIsaOffset)
    {
        size_t  labelNumLen = isa_line.size() - offset - kIsaLabelToken1.size() - 1;
        stream << std::hex << isa_line.substr(offset + kIsaLabelToken1.size(), labelNumLen);
        stream >> ret;
    }
    else if ((offset = isa_line.find(kIsaLabelToken2)) == 0)
    {
        if ((offset = isa_line.find('_')) != std::string::npos)
        {
            stream << isa_line.substr(kIsaLabelToken2.size(), offset - kIsaLabelToken2.size()) <<
                      isa_line.substr(offset + 1, isa_line.size() - offset - 2);
            stream >> ret;
        }
    }
    else
    {
        // Clang-15 LC generated labels have the format "L<number>:" or "_L<number>:".
        size_t       labelNumLen     = 0;
        const size_t kLabelStartSize = 1;
        const size_t kLabelEndSize   = 1;
        size_t       labelStartMatch = isa_line.find("_L");
        if (labelStartMatch == std::string::npos)
        {
            labelStartMatch = isa_line.find("L");
        }
        const size_t kLabelEndMatch = isa_line.find(":");
        if (labelStartMatch != std::string::npos)
        {
            if (labelStartMatch == 0 && kLabelEndMatch != std::string::npos)
            {
                labelNumLen = isa_line.size() - (kLabelStartSize + kLabelEndSize);
            }
            if (labelNumLen > 0)
            {
                stream << isa_line.substr(kLabelStartSize, labelNumLen);
                stream >> ret;
            }
        }
    }

    return ret;
}

int ParserIsa::GetGotoLabel(const std::string& sISALine)
{
    int ret = kNoLabel;
    size_t  offset = 0;
    std::stringstream  stream;

    if ((offset = sISALine.find(kIsaBranchToken)) != std::string::npos ||
        (offset = sISALine.find(kIsaCallToken)) != std::string::npos)
    {
        if ((offset = sISALine.find(kIsaLabelToken1)) != std::string::npos)
        {
            size_t  labelNumLen = sISALine.find_first_of(' ', offset) - offset - kIsaLabelToken1.size();
            stream << std::hex << sISALine.substr(offset + kIsaLabelToken1.size(), labelNumLen);
            stream >> ret;
        }
        else if ((offset = sISALine.find(kIsaLabelToken2)) != std::string::npos)
        {
            size_t  labelNumOffset = offset + kIsaLabelToken2.size();
            if ((offset = sISALine.find('_', offset)) != std::string::npos)
            {
                stream << sISALine.substr(labelNumOffset, offset - labelNumOffset) <<
                          sISALine.substr(offset + 1, sISALine.find_first_of(' ', offset) - offset - 1);
                stream >> ret;
            }
        }
        else
        {
            // Clang-15 LC generated labels have the format "L<number>:".
            size_t                    labelNumLen = 0;
            const size_t              kLabelStartSize  = 1;
            const size_t              kLabelEndSize    = 1;
            const size_t              kLabelStartMatch = sISALine.find("L");
            const size_t              kLabelEndMatch   = sISALine.find(":");
            if (kLabelStartMatch != std::string::npos)
            {
                if (kLabelStartMatch == 0 && kLabelEndMatch != std::string::npos)
                {
                    labelNumLen = sISALine.size() - (kLabelStartSize + kLabelEndSize);
                }
                if (labelNumLen > 0)
                {
                    stream << sISALine.substr(kLabelStartSize, labelNumLen);
                    stream >> ret;
                }
            }
        }
    }

    return ret;
}

bool ParserIsa::SplitIsaLine(const std::string& isaInstruction, std::string& instrOpCode,
                             std::string& params, std::string& binaryRepresentation, std::string& offset) const
{
    return ExtractBuildingBlocks(isaInstruction, instrOpCode, params, binaryRepresentation, offset);
}

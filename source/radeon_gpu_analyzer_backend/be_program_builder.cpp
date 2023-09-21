//=================================================================
// Copyright 2020 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

// Local.
#include "radeon_gpu_analyzer_backend/be_program_builder.h"
#include "emulator/parser/be_isa_parser.h"

bool BeProgramBuilder::LogCallback(const std::string& str)
{
    bool ret = false;
    if (log_callback_)
    {
        log_callback_(str);
        ret = true;
    }
    return ret;
}

beKA::beStatus BeProgramBuilder::ParseIsaToCsv(const std::string& isa_text, const std::string& device,
    std::string& parsed_isa_text, bool add_line_numbers, bool is_header_required)
{
    beKA::beStatus     status = beKA::kBeStatusParseIsaToCsvFailed;
    std::stringstream  parsed_isa;
    ParserIsa          parser;
    std::string        input_isa;

    if (is_header_required)
    {
        // Add ISA starting and ending tokens so that Parser can recognize it.
        std::stringstream  input_isa_stream;
        input_isa_stream << kStrHsailDisassemblyTokenStart << isa_text << kStrHsailDisassemblyTokenEnd << std::endl;
        input_isa = input_isa_stream.str();
    }
    else
    {
        input_isa = isa_text;
    }

    if (parser.Parse(input_isa))
    {
        // Padding instruction to be ignored.
        const char* kCodeEndPadding = "s_code_end";
        for (const Instruction* instruction : parser.GetInstructions())
        {
            std::string instruction_str;
            instruction->GetCsvString(device, add_line_numbers, instruction_str);
            if (instruction->GetInstructionOpCode().find(kCodeEndPadding) == std::string::npos)
            {
                parsed_isa << instruction_str;
            }
        }
        parsed_isa_text = parsed_isa.str();
        status = beKA::kBeStatusSuccess;
    }
    return status;
}

void BeProgramBuilder::SetLog(LoggingCallBackFuncP callback)
{
    log_callback_ = callback;
}

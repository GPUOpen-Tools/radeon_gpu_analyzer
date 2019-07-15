//=================================================================
// Copyright 2017 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

// Local.
#include <Include/beProgramBuilder.h>
#include <RadeonGPUAnalyzerBackend/Emulator/Parser/ISAParser.h>

using namespace std;

void beProgramBuilder::UsePlatformNativeLineEndings(std::string& text)
{
    // If text is empty
    if (text.length() <= 0)
    {
        return;
    }

    // Remove all carriage returns.
    // This will put us into Linux format (from either Mac or Windows).
    // [With the AMD OpenCL stack as of April 2012, this does nothing.]
    text.erase(std::remove(text.begin(), text.end(), '\r'), text.end());

    // Add a linefeed at the end if there's not one there already.
    if (text[text.length() - 1] != '\n')
    {
        text += '\n';
    }

#ifdef _WIN32
    // Now convert all of the \n to \r\n.
    size_t pos = 0;

    while ((pos = text.find('\n', pos)) != string::npos)
    {
        text.replace(pos, 1, "\r\n");
        pos += 2;
    }

#endif
}

beKA::beStatus beProgramBuilder::ParseISAToCSV(const std::string& isaText, const std::string& device,
                                               std::string& parsedIsaText, bool addLineNumbers, bool isHeaderRequired)
{
    beKA::beStatus     status = beKA::beStatus_ParseIsaToCsvFailed;
    std::stringstream  parsedIsa;
    ParserISA          parser;
    std::string        inputIsa;

    if (isHeaderRequired)
    {
        std::stringstream  inputIsaStream;
        // Add ISA starting and ending tokens so that Parser can recognize it.
        inputIsaStream << STR_HSAIL_DISASM_START_TOKEN << isaText << STR_HSAIL_DISASM_END_TOKEN << std::endl;
        inputIsa = inputIsaStream.str();
    }
    else
    {
        inputIsa = isaText;
    }

    if (parser.Parse(inputIsa))
    {
        // Padding instruction to be ignored.
        const char* CODE_END_PADDING = "s_code_end";
        for (const Instruction* pInst : parser.GetInstructions())
        {
            std::string instStr;
            pInst->GetCSVString(device, addLineNumbers, instStr);
            if (pInst->GetInstructionOpCode().find(CODE_END_PADDING) == std::string::npos)
            {
                parsedIsa << instStr;
            }
        }
        parsedIsaText = parsedIsa.str();
        status = beKA::beStatus_SUCCESS;
    }
    return status;
}

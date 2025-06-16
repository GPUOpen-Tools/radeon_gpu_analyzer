//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation of parser for the Southern Island [SI] MIMG instructions.
//=============================================================================

// Local.
#include "be_parser_si_mimg.h"

MIMGInstruction::DMASK ParserSiMimg::GetDmask(Instruction::Instruction64Bit hex_instructions)
{
    EXTRACT_INSTRUCTION32_FIELD(hex_instructions, SI, MIMG, dmask, DMASK, 8);
    RETURN_EXTRACT_INSTRUCTION(dmask);
}

MIMGInstruction::UNORM ParserSiMimg::GetUnorm(Instruction::Instruction64Bit hex_instructions)
{
    EXTRACT_INSTRUCTION32_FIELD(hex_instructions, SI, MIMG, unorm, UNORM, 12);
    RETURN_EXTRACT_INSTRUCTION(unorm);
}

MIMGInstruction::GLC ParserSiMimg::GetGlc(Instruction::Instruction64Bit hex_instructions)
{
    EXTRACT_INSTRUCTION32_FIELD(hex_instructions, SI, MIMG, glc, GLC, 13);
    RETURN_EXTRACT_INSTRUCTION(glc);
}

MIMGInstruction::DA ParserSiMimg::GetDa(Instruction::Instruction64Bit hexInstruction)
{
    EXTRACT_INSTRUCTION32_FIELD(hexInstruction, SI, MIMG, da, DA, 14);
    RETURN_EXTRACT_INSTRUCTION(da);
}

MIMGInstruction::R128 ParserSiMimg::GetR128(Instruction::Instruction64Bit hex_instructions)
{
    EXTRACT_INSTRUCTION32_FIELD(hex_instructions, SI, MIMG, r128, R128, 15);
    RETURN_EXTRACT_INSTRUCTION(r128);
}

MIMGInstruction::TFE ParserSiMimg::GetTfe(Instruction::Instruction64Bit hex_instructions)
{
    EXTRACT_INSTRUCTION32_FIELD(hex_instructions, SI, MIMG, tfe, TFE, 16);
    RETURN_EXTRACT_INSTRUCTION(tfe);
}

MIMGInstruction::LWE ParserSiMimg::GetLwe(Instruction::Instruction64Bit hex_instructions)
{
    EXTRACT_INSTRUCTION32_FIELD(hex_instructions, SI, MIMG, lwe, LWE, 17);
    RETURN_EXTRACT_INSTRUCTION(lwe);
}

SIMIMGInstruction::OP ParserSiMimg::GetOpSimimg(Instruction::Instruction64Bit hex_instructions, Instruction::InstructionCategory& instruction_kind)
{
    EXTRACT_INSTRUCTION32_FIELD(hex_instructions, SI, MIMG, op, OP, 18);

    if ((op >= SIMIMGInstruction::kImageLoad && op <= SIMIMGInstruction::kImageLoadMipPckSgn) ||
        (op == SIMIMGInstruction::kImageGetResinfo) ||
        (op >= SIMIMGInstruction::kImageSample && op <= SIMIMGInstruction::kImageSampleCCdClO))
    {
        instruction_kind = Instruction::kVectorMemoryRead;
    }
    else if (op >= SIMIMGInstruction::kImageStore && op <= SIMIMGInstruction::kImageStoreMipPck)
    {
        instruction_kind = Instruction::kVectorMemoryWrite;
    }
    else if (op >= SIMIMGInstruction::kImageAtomic_SWAP && op <= SIMIMGInstruction::kImageAtomicFmax)
    {
        instruction_kind = Instruction::kAtomics;
    }

    if ((op > SIMIMGInstruction::kImageLoadMipPckSgn && op < SIMIMGInstruction::kImageStore)
        || (op > SIMIMGInstruction::kImageStoreMipPck && op < SIMIMGInstruction::kImageGetResinfo)
        || (op > SIMIMGInstruction::kImageGather4CCl && op < SIMIMGInstruction::kImageGather4CL)
        || (op > SIMIMGInstruction::kImageGather4ClO && op < SIMIMGInstruction::kImageGather4LO)
        || (op > SIMIMGInstruction::kImageGather4CClO && op < SIMIMGInstruction::kImageGather4CLO)
        || (op > SIMIMGInstruction::kImageGetLod && op < SIMIMGInstruction::kImageSampleCd)
        || (op >= SIMIMGInstruction::kImageReserved))
    {
        return SIMIMGInstruction::kImageReserved;
    }
    else
    {
        return op;
    }
}

VIMIMGInstruction::OP ParserSiMimg::GetOpVimimg(Instruction::Instruction64Bit hex_instructions, Instruction::InstructionCategory& instruction_kind)
{
    EXTRACT_INSTRUCTION32_FIELD(hex_instructions, VI, MIMG, op, OP, 18);

    if ((op >= VIMIMGInstruction::kImageLoad && op <= VIMIMGInstruction::kImageLoadMipPckSgn) ||
        (op == VIMIMGInstruction::kImageGetResinfo) ||
        (op >= VIMIMGInstruction::kImageSample && op <= VIMIMGInstruction::kImageSampleCCdClO))
    {
        instruction_kind = Instruction::kVectorMemoryRead;
    }
    else if (op >= VIMIMGInstruction::kImageStore && op <= VIMIMGInstruction::kImageStoreMipPck)
    {
        instruction_kind = Instruction::kVectorMemoryWrite;
    }
    else if (op >= VIMIMGInstruction::kImageAtomicSwap && op <= VIMIMGInstruction::kImageAtomicDec)
    {
        instruction_kind = Instruction::kAtomics;
    }

    if (op > VIMIMGInstruction::kImageSampleCCdClO)
    {
        return VIMIMGInstruction::kImageIlegal;
    }
    else
    {
        return op;
    }
}

MIMGInstruction::SLC ParserSiMimg::GetSlc(Instruction::Instruction64Bit hex_instructions)
{
    EXTRACT_INSTRUCTION32_FIELD(hex_instructions, SI, MIMG, slc, SLC, 25);
    RETURN_EXTRACT_INSTRUCTION(slc);
}

MIMGInstruction::VADDR ParserSiMimg::GetVaddr(Instruction::Instruction64Bit hex_instructions)
{
    EXTRACT_INSTRUCTION64_FIELD(hex_instructions, MIMG, vaddr, VADDR, 32);
    RETURN_EXTRACT_INSTRUCTION(vaddr);
}

MIMGInstruction::VDATA ParserSiMimg::GetVdata(Instruction::Instruction64Bit hex_instructions)
{
    EXTRACT_INSTRUCTION64_FIELD(hex_instructions, MIMG, vdata, VDATA, 40);
    RETURN_EXTRACT_INSTRUCTION(vdata);
}

MIMGInstruction::SRSRC ParserSiMimg::GetSrsrc(Instruction::Instruction64Bit hex_instructions)
{
    EXTRACT_INSTRUCTION64_FIELD(hex_instructions, MIMG, srsrc, SRSRC, 48);
    RETURN_EXTRACT_INSTRUCTION(srsrc);
}

MIMGInstruction::SSAMP ParserSiMimg::GetSsamp(Instruction::Instruction64Bit hex_instructions)
{
    EXTRACT_INSTRUCTION64_FIELD(hex_instructions, MIMG, ssamp, SSAMP, 53);
    RETURN_EXTRACT_INSTRUCTION(ssamp);
}

ParserSi::kaStatus ParserSiMimg::Parse(GDT_HW_GENERATION hw_generation, Instruction::Instruction64Bit hex_instructions, Instruction*& instruction, int label, int goto_label)
{
    Instruction::InstructionCategory instruction_kind = Instruction::kScalarMemoryRead;
    MIMGInstruction::DMASK dmask = GetDmask(hex_instructions);
    MIMGInstruction::UNORM unorm = GetUnorm(hex_instructions);
    MIMGInstruction::GLC glc = GetGlc(hex_instructions);
    MIMGInstruction::DA da = GetDa(hex_instructions);
    MIMGInstruction::R128 r128 = GetR128(hex_instructions);
    MIMGInstruction::TFE tfe = GetTfe(hex_instructions);
    MIMGInstruction::LWE lwe = GetLwe(hex_instructions);
    MIMGInstruction::SLC slc = GetSlc(hex_instructions);
    MIMGInstruction::VADDR vaddr = GetVaddr(hex_instructions);
    MIMGInstruction::VDATA vdata = GetVdata(hex_instructions);
    MIMGInstruction::SRSRC srsrc = GetSrsrc(hex_instructions);
    MIMGInstruction::SSAMP ssamp = GetSsamp(hex_instructions);

    if ((hw_generation == GDT_HW_GENERATION_SEAISLAND) || (hw_generation == GDT_HW_GENERATION_SOUTHERNISLAND))
    {
        SIMIMGInstruction::OP op = GetOpSimimg(hex_instructions, instruction_kind);
        instruction = new SIMIMGInstruction(dmask, unorm, glc, da, r128, tfe, lwe, op, vaddr, vdata, srsrc, slc,
                                            ssamp, instruction_kind, label, goto_label);
    }
    else
    {
        VIMIMGInstruction::OP op = GetOpVimimg(hex_instructions, instruction_kind);
        instruction = new VIMIMGInstruction(dmask, unorm, glc, da, r128, tfe, lwe, op, vaddr, vdata, srsrc, slc,
                                            ssamp, instruction_kind, label, goto_label);
    }

    return ParserSi::kStatusSuccess;
}

ParserSi::kaStatus ParserSiMimg::Parse(GDT_HW_GENERATION, Instruction::Instruction32Bit, Instruction*&, bool, uint32_t, int, int)
{
    return ParserSi::kStatus32BitInstructionNotSupported;
}

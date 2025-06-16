//=============================================================================
/// Copyright (c) 2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for parsing binary code object disassembly.
//=============================================================================

// C++.
#include <cassert>

// Local.
#include "radeon_gpu_analyzer_cli/kc_utils_binary_parser.h"
#include "radeon_gpu_analyzer_cli/kc_utils_lightning.h"

// Backend.
#include "radeon_gpu_analyzer_backend/be_program_builder_binary.h"
#include "radeon_gpu_analyzer_backend/be_utils.h"

static const char* kStrErrorCannotParseDisassembly          = "Error: failed to parse LLVM disassembly.";
static const char* kStrErrorNoShaderFoundInDisassembly      = "Error: LLVM disassembly does not contatin valid kernels.";
static const char* kAmdgpuDisDotTextToken                   = ".text";
static const char* kAmdgpuDisShaderEndToken                 = "_symend:";
static const char* kAmdgpuDisDotSizeToken                   = ".size";
static const char* kAmdgpuDisKernelNameStartToken           = "_symend-";
static const char* kAmdgpuDisKernelNameStartTokenWithQuotes = "_symend\"-";
static const char* kAmdgpuDisShaderEndTokenNoColon          = "_symend";
static const char* kContinuationKernelRedundant             = "$local";

beKA::beStatus ParseAmdgpudisOutputGraphicStrategy::ParseAmdgpudisKernels(const std::string&                  amdgpu_dis_output,
                                                                          std::map<std::string, std::string>& kernel_to_disassembly,
                                                                          std::string&                        error_msg) const
{
    beKA::beStatus status = beKA::beStatus::kBeStatusGeneralFailed;
    status                = beProgramBuilderVulkan::ParseAmdgpudisOutput(amdgpu_dis_output, kernel_to_disassembly, error_msg);
    if (status != beKA::beStatus::kBeStatusSuccess)
    {
        std::map<std::string, std::string>  dxr_kernel_to_disassembly;
        std::string                         dxr_error_str;
        if (ParseAmdgpudisOutputComputeStrategy{}.ParseAmdgpudisKernels(amdgpu_dis_output, dxr_kernel_to_disassembly, dxr_error_str) ==
            beKA::beStatus::kBeStatusSuccess)
        {
            status                = beKA::beStatus::kBeStatusSuccess;
            kernel_to_disassembly = std::move(dxr_kernel_to_disassembly);
            error_msg             = std::move(dxr_error_str);
        }
    }
    return status;
}

beKA::beStatus ParseAmdgpudisOutputComputeStrategy::ParseAmdgpudisKernels(const std::string&                  amdgpu_dis_output,
                                                                          std::map<std::string, std::string>& kernel_to_disassembly,
                                                                          std::string&                        error_msg) const
{
    beKA::beStatus status = beKA::beStatus::kBeStatusGeneralFailed;
    assert(!amdgpu_dis_output.empty());
    if (!amdgpu_dis_output.empty())
    {
        // Get to the .text section.
        size_t curr_pos = amdgpu_dis_output.find(kAmdgpuDisDotTextToken);
        assert(curr_pos != std::string::npos);
        if (curr_pos != std::string::npos)
        {
            // These will be used to extract the disassembly for each shader.
            size_t kernel_offset_begin = 0;
            size_t kernel_offset_end   = 0;

            // Parse the .text section. Identify each shader's area by its ".size" token.
            curr_pos = amdgpu_dis_output.find(kAmdgpuDisDotSizeToken);
            if (curr_pos == std::string::npos)
            {
                error_msg = kStrErrorNoShaderFoundInDisassembly;
            }

            while (curr_pos != std::string::npos)
            {
                bool is_kernel_name_in_quotes = false;

                // Find the name of the kernel.
                std::string kernel_name;
                size_t      end_of_line = amdgpu_dis_output.find("\n", curr_pos);
                std::string line        = amdgpu_dis_output.substr(curr_pos, end_of_line - curr_pos);
                auto        found       = line.find(kAmdgpuDisKernelNameStartToken);
                if (found != std::string::npos)
                {
                    size_t start = found + strlen(kAmdgpuDisKernelNameStartToken);
                    kernel_name  = line.substr(start, end_of_line - start);
                    curr_pos     = end_of_line;

                    if (kernel_name.find(kContinuationKernelRedundant) != std::string::npos)
                    {
                        // Skip kernels with $local in the kernel names.
                        kernel_offset_end = end_of_line;
                        curr_pos          = amdgpu_dis_output.find(kAmdgpuDisDotSizeToken, kernel_offset_end);
                        continue;
                    }
                }
                else
                {
                    found = line.find(kAmdgpuDisKernelNameStartTokenWithQuotes);
                    if (found != std::string::npos)
                    {
                        is_kernel_name_in_quotes = true;
                        size_t start             = found + strlen(kAmdgpuDisKernelNameStartTokenWithQuotes);
                        kernel_name              = line.substr(start, end_of_line - start);
                        kernel_name              = BeMangledKernelUtils::UnQuote(kernel_name);
                        curr_pos                 = end_of_line;
                    }
                }

                if (!kernel_name.empty() && curr_pos != std::string::npos)
                {
                    // Construct the shader end token "kernel_name_symend:".
                    std::stringstream kernel_end_token_stream;
                    if (is_kernel_name_in_quotes)
                    {
                        kernel_end_token_stream << BeMangledKernelUtils::Quote(kernel_name + kAmdgpuDisShaderEndTokenNoColon) << ":";
                    }
                    else
                    {
                        kernel_end_token_stream << kernel_name << kAmdgpuDisShaderEndToken;
                    }
                    std::string kernel_token_end = kernel_end_token_stream.str();

                    // Extract the kernel disassembly.
                    kernel_offset_begin            = curr_pos;
                    kernel_offset_end              = amdgpu_dis_output.find(kernel_token_end, kernel_offset_begin);
                    std::string kernel_disassembly = amdgpu_dis_output.substr(curr_pos, kernel_offset_end - curr_pos);
                    BeUtils::TrimLeadingAndTrailingWhitespace(kernel_disassembly, kernel_disassembly);
                    kernel_name                        = BeMangledKernelUtils::DemangleShaderName(kernel_name);
                    kernel_to_disassembly[kernel_name] = KcUtilsLightning::PrefixWithISAHeader(kernel_name, kernel_disassembly);
                }
                else
                {
                    error_msg         = kStrErrorCannotParseDisassembly;
                    status            = beKA::beStatus::kBeStatusCannotParseDisassemblyGeneral;
                    kernel_offset_end = found;
                }

                // Look for the next shader.
                curr_pos = amdgpu_dis_output.find(kAmdgpuDisDotSizeToken, kernel_offset_end);
            }
        }
        else
        {
            error_msg = kStrErrorCannotParseDisassembly;
        }
    }

    if (!kernel_to_disassembly.empty())
    {
        status = beKA::beStatus::kBeStatusSuccess;
    }
    return status;
}

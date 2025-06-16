//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for parsing listed kernels in RGA CLI output.
//=============================================================================

#ifndef RGA_RADEONGPUANALYZERGUI_SOURCE_RG_CLI_KERNEL_LIST_PARSER_H_
#define RGA_RADEONGPUANALYZERGUI_SOURCE_RG_CLI_KERNEL_LIST_PARSER_H_

// C++.
#include <cassert>
#include <sstream>

// Local.
#include "radeon_gpu_analyzer_gui/rg_data_types.h"
#include "radeon_gpu_analyzer_gui/rg_data_types_opencl.h"
#include "radeon_gpu_analyzer_gui/rg_utils.h"

class RgCliKernelListParser
{
public:
    static bool ReadListKernelsOutput(const std::string& list_kernels_output, EntryToSourceLineRange& file_line_numbers)
    {
        bool is_parsing_failed = false;

        // Process the CLI output and return it through an output parameter.
        std::stringstream entrypoint_lines_stream;
        entrypoint_lines_stream.str(list_kernels_output);

        // Read each line of the list-kernels output.
        // Example:
        //   foo: 12-50
        std::string line_string;
        while (std::getline(entrypoint_lines_stream, line_string))
        {
            // Skip empty lines.
            if (!line_string.empty())
            {
                // Split each token using the location of the colon.
                std::vector<std::string> name_and_lines, lines;
                RgUtils::splitString(line_string, ':', name_and_lines);

                // Verify that there are only two tokens per line.
                bool is_token_count_correct = (name_and_lines.size() == 2);
                assert(is_token_count_correct);
                if (is_token_count_correct)
                {
                    // The first token is the entry point name.
                    const std::string& entrypoint_name = name_and_lines[0];

                    // The second token is the starting and ending line numbers separated by '-' symbol.
                    RgUtils::splitString(name_and_lines[1], '-', lines);
                    is_token_count_correct = (lines.size() == 2);
                    assert(is_token_count_correct);
                    if (is_token_count_correct)
                    {
                        file_line_numbers[entrypoint_name] = { std::atoi(lines[0].c_str()), std::atoi(lines[1].c_str()) };
                    }
                }
                else
                {
                    is_parsing_failed = true;
                }
            }
        }

        return !is_parsing_failed;
    }
};
#endif // RGA_RADEONGPUANALYZERGUI_SOURCE_RG_CLI_KERNEL_LIST_PARSER_H_

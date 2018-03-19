#pragma once

// C++.
#include <cassert>
#include <sstream>

// Local.
#include <RadeonGPUAnalyzerGUI/include/rgDataTypes.h>
#include <RadeonGPUAnalyzerGUI/include/rgUtils.h>

class rgCliKernelListParser
{
public:
    static bool ReadListKernelsOutput(const std::string& listKernelsOutput, EntryToSourceLineRange& fileLineNumbers)
    {
        bool isParsingFailed = false;

        // Process the CLI output and return it through an output parameter.
        std::stringstream entrypointLinesStream;
        entrypointLinesStream.str(listKernelsOutput);

        // Read each line of the lsit-kernels output.
        // Example:
        //   foo: 12-50
        std::string lineString;
        while (std::getline(entrypointLinesStream, lineString))
        {
            // Skip empty lines.
            if (!lineString.empty())
            {
                // Split each token using the location of the colon.
                std::vector<std::string> nameAndLines, lines;
                rgUtils::splitString(lineString, ':', nameAndLines);

                // Verify that there are only two tokens per line.
                bool isTokenCountCorrect = (nameAndLines.size() == 2);
                assert(isTokenCountCorrect);
                if (isTokenCountCorrect)
                {
                    // The first token is the entrypoint name.
                    const std::string& entrypointName = nameAndLines[0];

                    // The second token is the starting and ending line numbers separated by '-' symbol.
                    rgUtils::splitString(nameAndLines[1], '-', lines);
                    isTokenCountCorrect = (lines.size() == 2);
                    assert(isTokenCountCorrect);
                    if (isTokenCountCorrect)
                    {
                        fileLineNumbers[entrypointName] = { std::atoi(lines[0].c_str()), std::atoi(lines[1].c_str()) };
                    }
                }
                else
                {
                    isParsingFailed = true;
                }
            }
        }

        return !isParsingFailed;
    }
};
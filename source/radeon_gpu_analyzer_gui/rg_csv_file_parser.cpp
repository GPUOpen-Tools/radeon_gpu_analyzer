// C++.
#include <cassert>
#include <memory>
#include <sstream>

// Qt.
#include <QFile>
#include <QTextStream>

// Local.
#include "radeon_gpu_analyzer_gui/rg_csv_file_parser.h"
#include "radeon_gpu_analyzer_gui/rg_string_constants.h"

bool RgCsvFileParser::Parse(std::string& error_string)
{
    bool is_parsing_failed = false;

    QFile csv_file(csv_file_path_.c_str());
    QTextStream file_stream(&csv_file);

    // Attempt to open the CSV file and parse each line.
    bool is_file_opened = csv_file.open(QFile::ReadOnly | QFile::Text);
    assert(is_file_opened);
    if (is_file_opened)
    {
        // Read the first line and move on, as it's just column labels.
        QString csv_line = file_stream.readLine();

        int line_index = 2;
        // Parse each new line in the CSV file.
        do
        {
            // Read the next line in the file.
            csv_line = file_stream.readLine();
            if (!csv_line.isEmpty())
            {
                std::vector<std::string> line_tokens;
                ParseLine(csv_line.toStdString(), line_tokens);

                bool line_parsed_successfully = false;
                if (!line_tokens.empty())
                {
                    line_parsed_successfully = ProcessLineTokens(line_tokens);
                    assert(line_parsed_successfully);
                }

                // Was the line processed correctly?
                if (!line_parsed_successfully)
                {
                    // Build an error message indicating that parsing the file failed.
                    std::stringstream error_stream;
                    error_stream << kStrErrCsvParsingFailedA;
                    error_stream << csv_file_path_.c_str();
                    error_stream << kStrErrCsvParsingFailedB;
                    error_stream << line_index;

                    // Return the error string.
                    error_string = error_stream.str();
                    is_parsing_failed = true;
                }
            }

            // Increment the line index currently being parsed.
            line_index++;
        } while (!file_stream.atEnd());
    }
    else
    {
        // Failed to load the CSV file correctly.
        std::stringstream error_stream;
        error_stream << kStrErrCsvParsingFailedA;
        error_stream << csv_file_path_.c_str();

        // Return the error string.
        error_string = error_stream.str();
        is_parsing_failed = true;
    }

    return !is_parsing_failed;
}

void RgCsvFileParser::ParseLine(const std::string& csv_line, std::vector<std::string>& line_tokens)
{
    std::stringstream line_stream;
    line_stream.str(csv_line);
    std::string substr;

    // Step through the entire line of text, and split into tokens based on comma position.
    while (std::getline(line_stream, substr, ','))
    {
        // Are there any quotation marks within the token? If so, parsing is handled differently.
        size_t num_quotes_in_token = std::count(substr.begin(), substr.end(), '\"');
        switch (num_quotes_in_token)
        {
        case 0:
            {
                // If there are no quotes, just add the token to the line tokens list.
                line_tokens.push_back(substr);
            }
            break;
        case 1:
            {
                // Found a start quote. Keep reading new tokens to find the matching end quote.
                std::stringstream token_stream;
                do
                {
                    // Add the token to the quoted column string.
                    token_stream << substr << ',';
                    std::getline(line_stream, substr, ',');
                } while (!(substr.find('"') != substr.npos));

                // Add the final portion of the token to the stream.
                token_stream << substr;

                // Remove the quotation marks from the final token string.
                std::string quoted_token = token_stream.str();
                quoted_token.erase(std::remove(quoted_token.begin(), quoted_token.end(), '\"'), quoted_token.end());

                // Add the token to the line tokens list.
                line_tokens.push_back(quoted_token);
            }
            break;
        case 2:
            {
                // There's a single token surrounded with 2 quotes. Just remove the quotes and add the token to the lines.
                substr.erase(std::remove(substr.begin(), substr.end(), '\"'), substr.end());
                line_tokens.push_back(substr);
            }
            break;
        default:
            // If this happens, the format of the ISA line likely wasn't handled correctly.
            assert(false);
            break;
        }
    }
}

#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_CSV_FILE_PARSER_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_CSV_FILE_PARSER_H_

// C++.
#include <memory>
#include <string>

// The base class for a generic CSV file parser object.
class RgCsvFileParser
{
public:
    // A constructor that accepts a full path to the CSV file to be parsed.
    RgCsvFileParser(const std::string& csv_file_path) : csv_file_path_(csv_file_path) {}
    virtual ~RgCsvFileParser() = default;

    // Parse all lines in the given input file.
    bool Parse(std::string& error_string);

protected:
    // Process the tokens parsed from a line of the CSV file.
    virtual bool ProcessLineTokens(const std::vector<std::string>& tokens) = 0;

private:
    // Parse a line from the CSV file into a series of tokens.
    void ParseLine(const std::string& csv_line, std::vector<std::string>& line_tokens);

    // The full path to the CSV file being parsed.
    std::string csv_file_path_;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_CSV_FILE_PARSER_H_

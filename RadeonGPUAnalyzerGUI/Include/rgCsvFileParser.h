#pragma once

// C++.
#include <memory>
#include <string>

// The base class for a generic CSV file parser object.
class rgCsvFileParser
{
public:
    // A constructor that accepts a full path to the CSV file to be parsed.
    rgCsvFileParser(const std::string& csvFilePath) : m_csvFilePath(csvFilePath) {}
    virtual ~rgCsvFileParser() = default;

    // Parse all lines in the given input file.
    bool Parse(std::string& errorString);

protected:
    // Process the tokens parsed from a line of the CSV file.
    virtual bool ProcessLineTokens(const std::vector<std::string>& tokens) = 0;

private:
    // Parse a line from the CSV file into a series of tokens.
    void ParseLine(const std::string& csvLine, std::vector<std::string>& lineTokens);

    // The full path to the CSV file being parsed.
    std::string m_csvFilePath;
};
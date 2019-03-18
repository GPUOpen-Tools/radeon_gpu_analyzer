// C++.
#include <cassert>
#include <memory>
#include <sstream>

// Qt.
#include <QFile>
#include <QTextStream>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/rgCsvFileParser.h>
#include <RadeonGPUAnalyzerGUI/Include/rgStringConstants.h>

bool rgCsvFileParser::Parse(std::string& errorString)
{
    bool isParsingFailed = false;

    QFile csvFile(m_csvFilePath.c_str());
    QTextStream fileStream(&csvFile);

    // Attempt to open the CSV file and parse each line.
    bool isFileOpened = csvFile.open(QFile::ReadOnly | QFile::Text);
    assert(isFileOpened);
    if (isFileOpened)
    {
        // Read the first line and move on, as it's just column labels.
        QString csvLine = fileStream.readLine();

        int lineIndex = 2;
        // Parse each new line in the CSV file.
        do
        {
            // Read the next line in the file.
            csvLine = fileStream.readLine();
            if (!csvLine.isEmpty())
            {
                std::vector<std::string> lineTokens;
                ParseLine(csvLine.toStdString(), lineTokens);

                bool lineParsedSuccessfully = false;
                if (!lineTokens.empty())
                {
                    lineParsedSuccessfully = ProcessLineTokens(lineTokens);
                    assert(lineParsedSuccessfully);
                }

                // Was the line processed correctly?
                if (!lineParsedSuccessfully)
                {
                    // Build an error message indicating that parsing the file failed.
                    std::stringstream errorStream;
                    errorStream << STR_ERR_CSV_PARSING_FAILED_A;
                    errorStream << m_csvFilePath.c_str();
                    errorStream << STR_ERR_CSV_PARSING_FAILED_B;
                    errorStream << lineIndex;

                    // Return the error string.
                    errorString = errorStream.str();
                    isParsingFailed = true;
                }
            }

            // Increment the line index currently being parsed.
            lineIndex++;
        } while (!fileStream.atEnd());
    }
    else
    {
        // Failed to load the CSV file correctly.
        std::stringstream errorStream;
        errorStream << STR_ERR_CSV_PARSING_FAILED_A;
        errorStream << m_csvFilePath.c_str();

        // Return the error string.
        errorString = errorStream.str();
        isParsingFailed = true;
    }

    return !isParsingFailed;
}

void rgCsvFileParser::ParseLine(const std::string& csvLine, std::vector<std::string>& lineTokens)
{
    std::stringstream lineStream;
    lineStream.str(csvLine);
    std::string substr;

    // Step through the entire line of text, and split into tokens based on comma position.
    while (std::getline(lineStream, substr, ','))
    {
        // Are there any quotation marks within the token? If so, parsing is handled differently.
        size_t numQuotesInToken = std::count(substr.begin(), substr.end(), '\"');
        switch (numQuotesInToken)
        {
        case 0:
            {
                // If there are no quotes, just add the token to the line tokens list.
                lineTokens.push_back(substr);
            }
            break;
        case 1:
            {
                // Found a start quote. Keep reading new tokens to find the matching end quote.
                std::stringstream tokenStream;
                do
                {
                    // Add the token to the quoted column string.
                    tokenStream << substr << ',';
                    std::getline(lineStream, substr, ',');
                } while (!(substr.find('"') != substr.npos));

                // Add the final portion of the token to the stream.
                tokenStream << substr;

                // Remove the quotation marks from the final token string.
                std::string quotedToken = tokenStream.str();
                quotedToken.erase(std::remove(quotedToken.begin(), quotedToken.end(), '\"'), quotedToken.end());

                // Add the token to the line tokens list.
                lineTokens.push_back(quotedToken);
            }
            break;
        case 2:
            {
                // There's a single token surrounded with 2 quotes. Just remove the quotes and add the token to the lines.
                substr.erase(std::remove(substr.begin(), substr.end(), '\"'), substr.end());
                lineTokens.push_back(substr);
            }
            break;
        default:
            // If this happens, the format of the ISA line likely wasn't handled correctly.
            assert(false);
            break;
        }
    }
}
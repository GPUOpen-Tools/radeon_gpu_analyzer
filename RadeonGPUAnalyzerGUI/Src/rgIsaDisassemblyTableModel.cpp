// C++.
#include <algorithm>
#include <cassert>
#include <sstream>

// Qt.
#include <QClipboard>
#include <QFile>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QTextStream>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgIsaDisassemblyTableModel.h>
#include <RadeonGPUAnalyzerGUI/Include/rgConfigManager.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDefinitions.h>
#include <RadeonGPUAnalyzerGUI/Include/rgStringConstants.h>
#include <RadeonGPUAnalyzerGUI/Include/rgUtils.h>
#include <ui_rgIsaDisassemblyTableView.h>

// Shared with CLI.
#include <RadeonGPUAnalyzerGUI/../Utils/Include/rgaCliDefs.h>

// The color to use for label backgrounds within the table.
static const QColor kBranchLabelInstructionColor = QColor(37, 36, 225);
static const QColor kBranchLabelBackgroundColor = QColor("lightGray").lighter(113);


rgIsaDisassemblyTableModel::rgIsaDisassemblyTableModel(uint32_t modelCount, QWidget* pParent) :
    ModelViewMapper(modelCount)
{
    // Create the table model.
    m_pIsaTableModel = new QStandardItemModel(0, GetTableColumnIndex(rgIsaDisassemblyTableColumns::Count), pParent);

    // Add column headers to the table.
    m_pIsaTableModel->setHorizontalHeaderItem(GetTableColumnIndex(rgIsaDisassemblyTableColumns::Address),        new QStandardItem(STR_DISASSEMBLY_TABLE_COLUMN_ADDRESS));
    m_pIsaTableModel->setHorizontalHeaderItem(GetTableColumnIndex(rgIsaDisassemblyTableColumns::Opcode),         new QStandardItem(STR_DISASSEMBLY_TABLE_COLUMN_OPCODE));
    m_pIsaTableModel->setHorizontalHeaderItem(GetTableColumnIndex(rgIsaDisassemblyTableColumns::Operands),       new QStandardItem(STR_DISASSEMBLY_TABLE_COLUMN_OPERANDS));
    m_pIsaTableModel->setHorizontalHeaderItem(GetTableColumnIndex(rgIsaDisassemblyTableColumns::FunctionalUnit), new QStandardItem(STR_DISASSEMBLY_TABLE_COLUMN_FUNCTIONAL_UNIT));
    m_pIsaTableModel->setHorizontalHeaderItem(GetTableColumnIndex(rgIsaDisassemblyTableColumns::Cycles),         new QStandardItem(STR_DISASSEMBLY_TABLE_COLUMN_CYCLES));
    m_pIsaTableModel->setHorizontalHeaderItem(GetTableColumnIndex(rgIsaDisassemblyTableColumns::BinaryEncoding), new QStandardItem(STR_DISASSEMBLY_TABLE_COLUMN_BINARY_ENCODING));
}

const std::vector<int>& rgIsaDisassemblyTableModel::GetCorrelatedLineIndices() const
{
    return m_correlatedIsaLineIndices;
}

QStandardItemModel* rgIsaDisassemblyTableModel::GetTableModel() const
{
    return m_pIsaTableModel;
}

bool rgIsaDisassemblyTableModel::GetInputSourceLineNumberFromInstructionRow(int instructionLineIndex, int& inputSourceLineNumber)
{
    bool ret = false;

    auto inputSourceLineIter = m_instructionLineNumberToInputSourceLineNumber.find(instructionLineIndex);
    if (inputSourceLineIter != m_instructionLineNumberToInputSourceLineNumber.end())
    {
        // Return the input source file's line number that corresponds with the given disassembly line number.
        inputSourceLineNumber = inputSourceLineIter->second;
        ret = true;
    }

    return ret;
}

void rgIsaDisassemblyTableModel::GetLabelNameToLineIndexMap(std::map<std::string, int>& linkLabels) const
{
    int numLines = static_cast<int>(m_disassembledIsaLines.size());
    for (int lineIndex = 0; lineIndex < numLines; ++lineIndex)
    {
        std::shared_ptr<rgIsaLine> pCurrentLine = m_disassembledIsaLines[lineIndex];
        if (pCurrentLine->m_type == rgIsaLineType::Label)
        {
            // Cast the line into a label line.
            std::shared_ptr<rgIsaLineLabel> pLabelLine = std::static_pointer_cast<rgIsaLineLabel>(pCurrentLine);

            // Extract the label name and insert into the output map.
            std::string labelName = pLabelLine->m_labelName;
            linkLabels[labelName] = lineIndex;
        }
    }
}

void rgIsaDisassemblyTableModel::GetLineIndexToLabelNameMap(std::map<int, std::string>& linkLabels) const
{
    // Step through each disassembled line and collect all label line numbers, and their destination operand.
    int numLines = static_cast<int>(m_disassembledIsaLines.size());
    for (int lineIndex = 0; lineIndex < numLines; ++lineIndex)
    {
        std::shared_ptr<rgIsaLine> pCurrentLine = m_disassembledIsaLines[lineIndex];
        if (pCurrentLine->m_type == rgIsaLineType::Instruction)
        {
            // Look for branch instructions, and extract the destination from the operands text.
            std::shared_ptr<rgIsaLineInstruction> pInstructionLine = std::static_pointer_cast<rgIsaLineInstruction>(pCurrentLine);
            if (pInstructionLine->m_functionalUnit.compare("Branch") == 0)
            {
                std::string jumpDestination = pInstructionLine->m_operands;
                linkLabels[lineIndex] = jumpDestination;
            }
        }
    }
}

void rgIsaDisassemblyTableModel::InsertLabelRows()
{
    // Try to insert labels into the start column if possible.
    int startColumn = GetTableColumnIndex(rgIsaDisassemblyTableColumns::Address);
    int endColumn = GetTableColumnIndex(rgIsaDisassemblyTableColumns::Count);

    // The column index to insert the labels into.
    m_labelColumnIndex = startColumn;

    // Check the global settings to determine which disassembly table columns are visible.
    std::shared_ptr<rgGlobalSettings> pGlobalSettings = rgConfigManager::Instance().GetGlobalConfig();
    assert(pGlobalSettings != nullptr);
    if (pGlobalSettings != nullptr)
    {
        // Determine which column is the left-most visible column in the table.
        for (int columnIndex = startColumn; columnIndex < endColumn; ++columnIndex)
        {
            bool isValidColumnIndex = (columnIndex >= startColumn) && (columnIndex < endColumn);
            bool isVisible = pGlobalSettings->m_visibleDisassemblyViewColumns[columnIndex];
            if (isVisible)
            {
                m_labelColumnIndex = columnIndex;
                break;
            }
        }
    }

    // Step through each ISA line and update the model data if it's a label row.
    int rowCount = static_cast<int>(m_disassembledIsaLines.size());
    for (int rowIndex = 0; rowIndex < rowCount; ++rowIndex)
    {
        std::shared_ptr<rgIsaLine> pIsaLine = m_disassembledIsaLines[rowIndex];

        if (pIsaLine->m_type == rgIsaLineType::Label)
        {
            // Cast into a rgIsaLineLabel to extract the label name.
            std::shared_ptr<rgIsaLineLabel> pLabelLine = std::static_pointer_cast<rgIsaLineLabel>(pIsaLine);

            // Add the label text to the model.
            SetTableModelText(pLabelLine->m_labelName, rowIndex, m_labelColumnIndex);

            // Set the background color to yellow to make the label stand out more.
            SetTableModelBackgroundColor(kBranchLabelBackgroundColor, rowIndex, m_labelColumnIndex);
        }
    }
}

int rgIsaDisassemblyTableModel::GetTableColumnIndex(rgIsaDisassemblyTableColumns column)
{
    return static_cast<int>(column);
}

bool rgIsaDisassemblyTableModel::IsBranchOperandItem(const QModelIndex& modelIndex)
{
    bool isLabelLinkItem = false;

    // Find the disassembly row for the item.
    int rowIndex = modelIndex.row();
    std::shared_ptr<rgIsaLine> pIsaLine = m_disassembledIsaLines[rowIndex];

    // Determine what kind of instruction is represented in this row.
    if (pIsaLine->m_type == rgIsaLineType::Instruction)
    {
        // Cast the line to access the instruction data.
        std::shared_ptr<rgIsaLineInstruction> pInstructionLine = std::static_pointer_cast<rgIsaLineInstruction>(pIsaLine);

        // Is this a branch instruction?
        if (pInstructionLine->m_functionalUnit.compare("Branch") == 0)
        {
            // Is the given index in the Operands column? That's the column a label needs to be painted in.
            int columnIndex = modelIndex.column();
            bool isOperandsColumn = columnIndex == GetTableColumnIndex(rgIsaDisassemblyTableColumns::Operands);
            if (isOperandsColumn)
            {
                isLabelLinkItem = true;
            }
        }
    }

    return isLabelLinkItem;
}

bool rgIsaDisassemblyTableModel::IsColumnVisible(int columnIndex) const
{
    bool isVisible = false;

    // Check the global config settings to see if the column should be visible.
    rgConfigManager& configManager = rgConfigManager::Instance();
    std::shared_ptr<rgGlobalSettings> pGlobalSettings = configManager.GetGlobalConfig();
    assert(pGlobalSettings != nullptr);
    if (pGlobalSettings != nullptr)
    {
        // Verify that the incoming column index is valid.
        bool isValidIndex = (columnIndex >= 0) && (columnIndex < pGlobalSettings->m_visibleDisassemblyViewColumns.size());
        assert(isValidIndex);
        if (isValidIndex)
        {
            isVisible = pGlobalSettings->m_visibleDisassemblyViewColumns[columnIndex];
        }
    }

    return isVisible;
}

bool rgIsaDisassemblyTableModel::IsIsaLineCorrelated(int lineIndex) const
{
    bool ret = false;

    // Check if the given line index exists in the current cache of correlated line indices.
    auto lineIter = std::find(m_correlatedIsaLineIndices.begin(), m_correlatedIsaLineIndices.end(), lineIndex);
    if (lineIter != m_correlatedIsaLineIndices.end())
    {
        ret = true;
    }

    return ret;
}

bool rgIsaDisassemblyTableModel::IsSourceLineCorrelated(int lineIndex) const
{
    return (m_inputSourceLineIndexToInstructionLineIndices.find(lineIndex) != m_inputSourceLineIndexToInstructionLineIndices.end());
}

bool rgIsaDisassemblyTableModel::IsSourceLineInEntrypoint(int lineIndex) const
{
    bool ret = false;

    // Does the given line fall within the line number bounds for this entrypoint?
    if ((lineIndex >= m_sourceFileEntrypointStartLine) && (lineIndex <= m_sourceFileEntrypointEndLine))
    {
        ret = true;
    }

    return ret;
}

bool rgIsaDisassemblyTableModel::PopulateFromCsvFile(const std::string& csvFileFullPath)
{
    // Attempt to load the CSV file, and then update the model's data.
    bool isDataLoaded = LoadCsvData(csvFileFullPath);
    assert(isDataLoaded);
    if (isDataLoaded)
    {
        // Initialize the model using the CSV data that was just loaded.
        InitializeModelData();
    }

    return isDataLoaded;
}

bool rgIsaDisassemblyTableModel::SetCorrelatedSourceLineIndex(int lineIndex)
{
    bool isCorrelated = false;

    // Determine the set of ISA instructions that should become highlighted.
    std::vector<int> isaLineIndices;
    isCorrelated = GetDisassemblyLineIndicesFromInputSourceLine(lineIndex, isaLineIndices);

    if (isCorrelated)
    {
        // Assign the list of highlighted lines to paint.
        m_correlatedIsaLineIndices = isaLineIndices;
    }
    else
    {
        // Clear the list and highlight nothing.
        m_correlatedIsaLineIndices.clear();
    }

    return isCorrelated;
}

bool rgIsaDisassemblyTableModel::GetDisassemblyLineIndicesFromInputSourceLine(int inputSourceLine, std::vector<int>& disassemblyLines)
{
    bool ret = false;

    auto disassemblyInstructionLinesIter = m_inputSourceLineIndexToInstructionLineIndices.find(inputSourceLine);
    if (disassemblyInstructionLinesIter != m_inputSourceLineIndexToInstructionLineIndices.end())
    {
        // Return the list of disassembly line indices.
        disassemblyLines = disassemblyInstructionLinesIter->second;
        ret = true;
    }

    return ret;
}

int rgIsaDisassemblyTableModel::GetCsvColumnIndex(rgIsaDisassemblyCsvFileColumns column) const
{
    return static_cast<uint>(column);
}

QColor rgIsaDisassemblyTableModel::GetFunctionalUnitColor(const std::string& functionalUnit) const
{
    QColor color;

    static const std::map<std::string, QColor> kFunctionalUnitColors =
    {
        { FUNC_UNIT_SALU, QColor(28, 124, 84) },
        { FUNC_UNIT_VALU, QColor(75, 124, 140) },
        { FUNC_UNIT_SMEM, QColor(227, 29, 63).darker(110) },
        { FUNC_UNIT_VMEM, QColor(227, 29, 63).darker(90) },
        { FUNC_UNIT_INTERNAL_FLOW, QColor("black").lighter(150) },
        { FUNC_UNIT_BRANCH, kBranchLabelInstructionColor },
    };

    auto unit = kFunctionalUnitColors.find(functionalUnit);
    if (unit != kFunctionalUnitColors.end())
    {
        color = unit->second;
    }

    return color;
}

bool rgIsaDisassemblyTableModel::LoadCsvData(const std::string& csvFileFullPath)
{
    bool ret = true;

    // Clear out existing data.
    m_disassembledIsaLines.clear();

    QFile csvFile(csvFileFullPath.c_str());
    QTextStream fileStream(&csvFile);

    // Attempt to open the file to read each instruction line.
    bool isFileOpened = csvFile.open(QFile::ReadOnly | QFile::Text);
    assert(isFileOpened);
    if (isFileOpened)
    {
        // Read the first ISA instruction line and just move on, as it's just column labels.
        QString isaLine = fileStream.readLine();

        // Parse each new line in the ISA disassembly file.
        do
        {
            // Read the next line in the file.
            isaLine = fileStream.readLine();
            if (!isaLine.isEmpty())
            {
                // Parse the line that was just read into a new rgIsaLine.
                std::shared_ptr<rgIsaLine> pNewLine = nullptr;
                int inputSourceLineIndex = kInvalidCorrelationLineIndex;
                bool isSplitSuccessful = ParseCsvIsaLine(isaLine.toStdString(), pNewLine, inputSourceLineIndex);

                // Was the line processed correctly?
                assert(isSplitSuccessful);
                if (isSplitSuccessful)
                {
                    assert(pNewLine != nullptr);
                    if (pNewLine != nullptr)
                    {
                        // Does the parsed ISA line have an associated input source line?
                        if (inputSourceLineIndex != kInvalidCorrelationLineIndex)
                        {
                            // Use a map to associate the current line of disassembly with the input source line index.
                            int disassemblyLineIndex = static_cast<int>(m_disassembledIsaLines.size());
                            m_instructionLineNumberToInputSourceLineNumber[disassemblyLineIndex] = inputSourceLineIndex;

                            // Find the minimum line number of the entry point in the input file.
                            if (inputSourceLineIndex < m_sourceFileEntrypointStartLine)
                            {
                                m_sourceFileEntrypointStartLine = inputSourceLineIndex;
                            }

                            // Find the maximum line number of the entry point in the input file.
                            if (inputSourceLineIndex > m_sourceFileEntrypointEndLine)
                            {
                                m_sourceFileEntrypointEndLine = inputSourceLineIndex;
                            }

                            // Also create a mapping of input source code line index to a list of all associated disassembly instruction line indices.
                            std::vector<int>& disassemblyLineIndices = m_inputSourceLineIndexToInstructionLineIndices[inputSourceLineIndex];
                            disassemblyLineIndices.push_back(disassemblyLineIndex);
                        }

                        // Add the ISA line's structure to the list of instruction lines.
                        m_disassembledIsaLines.push_back(pNewLine);
                    }
                }
                else
                {
                    ret = false;
                }
            }
        } while (!fileStream.atEnd());
    }
    else
    {
        // Failed to open the disassembly file.
        ret = false;
    }

    return ret;
}

void rgIsaDisassemblyTableModel::SetTableModelText(const std::string& modelText, uint rowIndex, uint columnIndex)
{
    // Set the cell's text, and make it left-aligned.
    m_pIsaTableModel->setData(m_pIsaTableModel->index(rowIndex, columnIndex), QString(modelText.c_str()));
    m_pIsaTableModel->setData(m_pIsaTableModel->index(rowIndex, columnIndex), Qt::AlignLeft, Qt::TextAlignmentRole);
}

void rgIsaDisassemblyTableModel::SetTableModelTextColor(const QColor& modelColor, uint rowIndex)
{
    // Set the table's model color for each column in the given row.
    for (int columnIndex = 0; columnIndex < GetTableColumnIndex(rgIsaDisassemblyTableColumns::Count); ++columnIndex)
    {
        // Set the foreground color role in the model.
        m_pIsaTableModel->setData(m_pIsaTableModel->index(rowIndex, columnIndex), modelColor, Qt::ForegroundRole);
    }
}

void rgIsaDisassemblyTableModel::SetTableModelBackgroundColor(const QColor& modelColor, uint rowIndex, uint columnIndex)
{
    // Set the background color role in the model.
    m_pIsaTableModel->setData(m_pIsaTableModel->index(rowIndex, columnIndex), modelColor, Qt::BackgroundColorRole);
}

bool rgIsaDisassemblyTableModel::ParseCsvIsaLine(const std::string& disassembledLine, std::shared_ptr<rgIsaLine>& parsedLine, int& inputSourceLineIndex)
{
    bool ret = false;

    // Some parsed CSV lines don't have an input sourcecode line number associated with them. In these cases, return -1.
    inputSourceLineIndex = kInvalidCorrelationLineIndex;

    std::vector<std::string> lineTokens;
    std::stringstream lineStream;
    lineStream.str(disassembledLine);
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
        }
    }

    int numColumns = static_cast<int>(lineTokens.size());
    const int numCsvColumns = static_cast<int>(rgIsaDisassemblyCsvFileColumns::Count);
    switch (numColumns)
    {
    case 1:
        {
            // This line is a label line that indicates a new section of instructions.
            std::shared_ptr<rgIsaLineLabel> pLabelLine = std::make_shared<rgIsaLineLabel>();
            pLabelLine->m_labelName = lineTokens[0];

            // The line was parsed successfully.
            parsedLine = std::static_pointer_cast<rgIsaLine>(pLabelLine);
            ret = true;
        }
        break;
    case numCsvColumns:
        {
            // Add each token to the output structure.
            std::shared_ptr<rgIsaLineInstruction> pInstructionLine = std::make_shared<rgIsaLineInstruction>();

            // Assign the values into the instruction line object.
            pInstructionLine->m_address             = lineTokens[GetCsvColumnIndex(rgIsaDisassemblyCsvFileColumns::Address)];
            pInstructionLine->m_opcode              = lineTokens[GetCsvColumnIndex(rgIsaDisassemblyCsvFileColumns::Opcode)];
            pInstructionLine->m_operands            = lineTokens[GetCsvColumnIndex(rgIsaDisassemblyCsvFileColumns::Operands)];
            pInstructionLine->m_functionalUnit      = lineTokens[GetCsvColumnIndex(rgIsaDisassemblyCsvFileColumns::FunctionalUnit)];
            pInstructionLine->m_cycles              = lineTokens[GetCsvColumnIndex(rgIsaDisassemblyCsvFileColumns::Cycles)];
            pInstructionLine->m_binaryEncoding      = lineTokens[GetCsvColumnIndex(rgIsaDisassemblyCsvFileColumns::BinaryEncoding)];

            // Extract the source line number as an integer.
            inputSourceLineIndex = std::stoi(lineTokens[GetCsvColumnIndex(rgIsaDisassemblyCsvFileColumns::SourceLineNumber)].c_str());

            // The line was parsed successfully, so assign it to the output instance.
            parsedLine = std::static_pointer_cast<rgIsaLineInstruction>(pInstructionLine);
            ret = true;
        }
        break;
    default:
        // Catch cases where the type of line format is unhandled.
        assert(false);
        break;
    }

    return ret;
}

void rgIsaDisassemblyTableModel::ClearLabelLines()
{
    // Empty out the contents of all lines with labels.
    int rowCount = static_cast<int>(m_disassembledIsaLines.size());
    for (int rowIndex = 0; rowIndex < rowCount; ++rowIndex)
    {
        std::shared_ptr<rgIsaLine> pIsaLine = m_disassembledIsaLines[rowIndex];
        if (pIsaLine->m_type == rgIsaLineType::Label)
        {
            // Add the label text to the model.
            SetTableModelText("", rowIndex, m_labelColumnIndex);

            // Set the background color to white, because the data was erased.
            SetTableModelBackgroundColor(QColor(Qt::white), rowIndex, m_labelColumnIndex);
        }
    }
}

void rgIsaDisassemblyTableModel::GetColumnMaxWidths(const QVector<int>& selectedRowNumbers, std::vector<int>& widths) const
{
    const auto& pGlobalSettings = rgConfigManager::Instance().GetGlobalConfig();
    for (int col = 0, numColumns = m_pIsaTableModel->columnCount(); col < numColumns; ++col)
    {
        if (pGlobalSettings->m_visibleDisassemblyViewColumns[col])
        {
            int maxWidth = 0;
            if (selectedRowNumbers.at(selectedRowNumbers.size() - 1) < m_disassembledIsaLines.size())
            {
                foreach (auto row, selectedRowNumbers)
                {
                    if (m_disassembledIsaLines[row]->m_type == rgIsaLineType::Instruction)
                    {
                        QVariant  cellText = m_pIsaTableModel->data(m_pIsaTableModel->index(row, col));
                        maxWidth = std::max(maxWidth, cellText.toString().size());
                    }
                }
            }
            widths.push_back(maxWidth);
        }
    }
}

void rgIsaDisassemblyTableModel::CopyRowsToClipboard(const QVector<int>& selectedRowNumbers)
{
    const int MIN_COLUMN_OFFSET = 4;
    std::shared_ptr<rgGlobalSettings> pGlobalSettings = rgConfigManager::Instance().GetGlobalConfig();

    // Calculate maximum width for each column in the selected ISA region.
    std::vector<int>  maxColumnWidths;
    GetColumnMaxWidths(selectedRowNumbers, maxColumnWidths);

    // Add a tab as a delimiter between each column.
    const char COLUMN_DELIMITER_SYMBOL = ' ';

    // Build a string including each line in the selected range.
    std::stringstream clipboardText;
    foreach(auto rowIndex, selectedRowNumbers)
    {
        // Is the current row an instruction or a label?
        std::shared_ptr<rgIsaLine> pIsaLine = m_disassembledIsaLines[rowIndex];
        if (pIsaLine->m_type == rgIsaLineType::Instruction)
        {
            bool isFirstTokenInLine = true;
            std::string  prevColumnText = "";

            // Add the contents of visible columns to the clipboard text.
            for (int columnIndex = 0, prevVisibleColumnIndex = 0; columnIndex < pGlobalSettings->m_visibleDisassemblyViewColumns.size(); ++columnIndex)
            {
                // Is the current column visible?
                bool isColumnVisible = pGlobalSettings->m_visibleDisassemblyViewColumns[columnIndex];
                if (isColumnVisible)
                {
                    if (isFirstTokenInLine)
                    {
                        // We don't need to append the delimiter before the first token in the line.
                        isFirstTokenInLine = false;
                    }
                    else
                    {
                        // Append sufficient number of spaces so that the columns are aligned.
                        // The number of needed spaces is based on the width of the text in the previous column and its maximum width.
                        assert(prevVisibleColumnIndex < maxColumnWidths.size());
                        std::string  delimiter = "";
                        for (size_t i = 0, numSpaces = MIN_COLUMN_OFFSET + maxColumnWidths[prevVisibleColumnIndex++] - prevColumnText.size();
                            i < numSpaces; ++i)
                        {
                            delimiter += COLUMN_DELIMITER_SYMBOL;
                        }
                        clipboardText << delimiter;
                    }

                    // Get the cell data.
                    QModelIndex cellIndex = m_pIsaTableModel->index(rowIndex, columnIndex);
                    QVariant cellText = m_pIsaTableModel->data(cellIndex);

                    // If the data is invalid, get it from somewhere else.
                    if (!cellText.isValid())
                    {
                        // Check if the functional unit is set to "Branch".
                        // If it is, get the data from Operands column from disassembled ISA Lines instead.
                        QString cellTextString = m_pIsaTableModel->data(m_pIsaTableModel->index(rowIndex, static_cast<int>(rgIsaDisassemblyTableColumns::FunctionalUnit))).toString();
                        if (cellTextString.compare("Branch") == 0)
                        {
                            std::shared_ptr<rgIsaLine> pIsaLine = m_disassembledIsaLines[rowIndex];
                            assert(pIsaLine != nullptr);
                            if (pIsaLine != nullptr)
                            {
                                std::shared_ptr<rgIsaLineInstruction> pInstructionLine = std::static_pointer_cast<rgIsaLineInstruction>(pIsaLine);
                                assert(pInstructionLine != nullptr);
                                if (pInstructionLine != nullptr)
                                {
                                    cellText = QString::fromStdString(pInstructionLine->m_operands);
                                }
                            }
                        }
                    }

                    // Add the data to the clipboard string stream.
                    clipboardText << (prevColumnText = cellText.toString().toStdString());
                }
            }
        }
        else if (pIsaLine->m_type == rgIsaLineType::Label)
        {
            // Extract the label text from whatever column the label is in.
            QModelIndex cellIndex = m_pIsaTableModel->index(rowIndex, m_labelColumnIndex);
            QVariant cellText = m_pIsaTableModel->data(cellIndex);

            // Append the label name to the clipboard text.
            clipboardText << cellText.toString().toStdString();
        }

        // Add a newline to the end of each row of data.
        clipboardText << std::endl;
    }

    // Add the text into the clipboard.
    QClipboard* pClipboard = QApplication::clipboard();
    pClipboard->setText(clipboardText.str().c_str());
}

void rgIsaDisassemblyTableModel::InitializeModelData()
{
    // Update the number of rows in the table.
    int lineCount = static_cast<int>(m_disassembledIsaLines.size());
    m_pIsaTableModel->setRowCount(lineCount);

    // Step through each ISA line and update the row item's data.
    for (int lineIndex = 0; lineIndex < lineCount; ++lineIndex)
    {
        std::shared_ptr<rgIsaLine> pIsaLine = m_disassembledIsaLines[lineIndex];

        if (pIsaLine->m_type == rgIsaLineType::Instruction)
        {
            std::shared_ptr<rgIsaLineInstruction> pInstructionLine = std::static_pointer_cast<rgIsaLineInstruction>(pIsaLine);

            // Update the model cells with data from each disassembled ISA instruction line.
            SetTableModelText(pInstructionLine->m_address,          lineIndex, GetTableColumnIndex(rgIsaDisassemblyTableColumns::Address));
            SetTableModelText(pInstructionLine->m_opcode,           lineIndex, GetTableColumnIndex(rgIsaDisassemblyTableColumns::Opcode));
            SetTableModelText(pInstructionLine->m_functionalUnit,   lineIndex, GetTableColumnIndex(rgIsaDisassemblyTableColumns::FunctionalUnit));
            SetTableModelText(pInstructionLine->m_cycles,           lineIndex, GetTableColumnIndex(rgIsaDisassemblyTableColumns::Cycles));
            SetTableModelText(pInstructionLine->m_binaryEncoding,   lineIndex, GetTableColumnIndex(rgIsaDisassemblyTableColumns::BinaryEncoding));

            // Branch instructions are rendered with a separate delegate, so we don't need add them to the table model.
            if (pInstructionLine->m_functionalUnit.compare("Branch") != 0)
            {
                SetTableModelText(pInstructionLine->m_operands, lineIndex, GetTableColumnIndex(rgIsaDisassemblyTableColumns::Operands));
            }

            // The functional unit column's text color is based on value.
            QColor textColor = GetFunctionalUnitColor(pInstructionLine->m_functionalUnit);
            SetTableModelTextColor(textColor, lineIndex);
        }
    }

    InsertLabelRows();
}
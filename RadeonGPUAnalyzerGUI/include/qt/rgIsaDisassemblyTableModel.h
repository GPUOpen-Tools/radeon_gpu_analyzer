#pragma once

// Qt.
#include <QWidget>

// Infra.
#include <QtCommon/Util/ModelViewMapper.h>

// Forward declarations.
class QStandardItemModel;

// An enumeration specifying each column loaded into in the ISA disassembly table.
enum class rgIsaDisassemblyTableColumns
{
    Address,
    Opcode,
    Operands,
    FunctionalUnit,
    Cycles,
    BinaryEncoding,
    Count
};

// A model responsible for housing a table of disassembled ISA instruction data.
class rgIsaDisassemblyTableModel : public ModelViewMapper
{
    Q_OBJECT

public:
    explicit rgIsaDisassemblyTableModel(uint32_t modelCount, QWidget* pParent = nullptr);
    virtual ~rgIsaDisassemblyTableModel() = default;

    // Remove all label lines from the table.
    void ClearLabelLines();

    // Copy the given range of data in the table to the clipboard.
    void CopyRangeToClipboard(int startRow, int endRow);

    // Get the enum value of the incoming ISA table column.
    static int GetTableColumnIndex(rgIsaDisassemblyTableColumns column);

    // Retrieve the current set of correlated line indices. When empty, no line is highlighted in the input source file.
    const std::vector<int>& GetCorrelatedLineIndices() const;

    // Retrieve the table model.
    QStandardItemModel* GetTableModel() const;

    // Get the input source file's line number associated with the instruction given disassembly line index.
    bool GetInputSourceLineNumberFromInstructionRow(int instructionLineIndex, int& inputSourceLineNumber);

    // Retrieve a map of label names to the disassembly line index of the label.
    void GetLabelNameToLineIndexMap(std::map<std::string, int>& linkLabels) const;

    // Retrieve a map of disassembly table line indices to the label name.
    void GetLineIndexToLabelNameMap(std::map<int, std::string>& linkLabels) const;

    // Insert label rows into the disassembly table.
    void InsertLabelRows();

    // Determine if the given index contains a branch instruction's operand jump target.
    bool IsBranchOperandItem(const QModelIndex& modelIndex);

    // Determine if the column with a given index is visible.
    bool IsColumnVisible(int columnIndex) const;

    // Is the given ISA line correlated with a line in the input file?
    bool IsIsaLineCorrelated(int lineIndex) const;

    // Is the given source line correlated with some ISA line(s)?
    bool IsSourceLineCorrelated(int lineIndex) const;

    // Is the given source file line number within the bounds of the entrypoint code?
    bool IsSourceLineInEntrypoint(int lineIndex) const;

    // Populate the model by loading a disassembly CSV file.
    bool PopulateFromCsvFile(const std::string& csvFileFullPath);

    // Set the correlated input source line index to highlight.
    bool SetCorrelatedSourceLineIndex(int lineIndex);

private:
    // An enumeration specifying each column we expect to find in an ISA CSV file.
    enum class rgIsaDisassemblyCsvFileColumns
    {
        Address,
        SourceLineNumber,
        Opcode,
        Operands,
        FunctionalUnit,
        Cycles,
        BinaryEncoding,
        Count
    };

    // An enumeration used to classify different types of parsed disassembly lines.
    enum class rgIsaLineType
    {
        Instruction,
        Label
    };

    // A simple data class used as a common base for storing lines parsed from a disassembly CSV file.
    class rgIsaLine
    {
    public:
        // A constructor used to initialize the type of disassembled instruction.
        rgIsaLine(rgIsaLineType lineType) : m_type(lineType) {}

        // The type of line parsed from the disassembly file.
        rgIsaLineType m_type;
    };

    // A class used to store a disassembled instruction line.
    class rgIsaLineInstruction : public rgIsaLine
    {
    public:
        // A constructor used to initialize the line type.
        rgIsaLineInstruction() : rgIsaLine(rgIsaLineType::Instruction) {}

        // The instruction address within the disassembled binary.
        std::string m_address;

        // The instruction opcode.
        std::string m_opcode;

        // The instruction operands.
        std::string m_operands;

        // The cycle count of the instruction.
        std::string m_cycles;

        // The functional unit responsible for execution.
        std::string m_functionalUnit;

        // The hex representation of the instruction.
        std::string m_binaryEncoding;
    };

    // A class used to store a disassembly line label.
    class rgIsaLineLabel : public rgIsaLine
    {
    public:
        // A constructor used to initialize the line type.
        rgIsaLineLabel() : rgIsaLine(rgIsaLineType::Label) {}

        // The name of the label, if applicable.
        std::string m_labelName;
    };

    // Retrieve the list of disassembled instruction lines associated with the given input source line number.
    bool GetDisassemblyLineIndicesFromInputSourceLine(int inputSourceLine, std::vector<int>& disassemblyLines);

    // Get the enum value of the incoming CSV table column.
    int GetCsvColumnIndex(rgIsaDisassemblyCsvFileColumns column) const;

    // Look up the color to use for the given functional unit.
    QColor GetFunctionalUnitColor(const std::string& functionalUnit) const;

    // Load the data in the given CSV file into the model.
    bool LoadCsvData(const std::string& csvFileFullPath);

    // Set the model's cell text.
    void SetTableModelText(const std::string& modelText, uint rowIndex, uint columnIndex);

    // Set the model's cell text foreground color.
    void SetTableModelTextColor(const QColor& modelColor, uint rowIndex);

    // Set the model's cell text background color.
    void SetTableModelBackgroundColor(const QColor& modelColor, uint rowIndex, uint columnIndex);

    // Parse an instruction line from an ISA disassembly CSV file.
    bool ParseCsvIsaLine(const std::string& disassembledLine, std::shared_ptr<rgIsaLine>& parsedLine, int& inputSourceLineIndex);

    // Initialize the model's data with the disassembled instruction lines.
    void InitializeModelData();

    // Get the maximum text length for each column in the ISA region from startRow to EndRow.
    // The values of maximum width for all visible columns are added to the "widths" vector.
    void GetColumnMaxWigths(const int startRow, const int endRow, std::vector<int>& widths) const;

    // A vector of all disassembled instructions loaded from file.
    std::vector<std::shared_ptr<rgIsaLine>> m_disassembledIsaLines;

    // A map of instruction index to high level source line index.
    std::map<int, int> m_instructionLineNumberToInputSourceLineNumber;

    // A map of input source file line index to a list of assembly instruction line indices.
    std::map<int, std::vector<int>> m_inputSourceLineIndexToInstructionLineIndices;

    // The entrypoint's start line number in the input source file.
    int m_sourceFileEntrypointStartLine = INT_MAX;

    // The entrypoint's end line number in the input source file.
    int m_sourceFileEntrypointEndLine = INT_MIN;

    // The column that labels are currently inserted into.
    int m_labelColumnIndex = -1;

    // The cache of correlated line indices used for highlights in the ISA table.
    std::vector<int> m_correlatedIsaLineIndices;

    // The model containing the data used to populate the ISA disassembly table.
    QStandardItemModel* m_pIsaTableModel = nullptr;
};
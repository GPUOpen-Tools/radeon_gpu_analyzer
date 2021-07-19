#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_ISA_DISASSEMBLY_TABLE_MODEL_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_ISA_DISASSEMBLY_TABLE_MODEL_H_

// Qt.
#include <QWidget>

// Infra.
#include "QtCommon/Util/ModelViewMapper.h"

// Forward declarations.
class QStandardItemModel;

// An enumeration specifying each column loaded into in the ISA disassembly table.
enum class RgIsaDisassemblyTableColumns
{
    kAddress,
    kOpcode,
    kOperands,
    kFunctionalUnit,
    kCycles,
    kBinaryEncoding,
    kCount
};

// A model responsible for housing a table of disassembled ISA instruction data.
class RgIsaDisassemblyTableModel : public ModelViewMapper
{
    Q_OBJECT

public:
    explicit RgIsaDisassemblyTableModel(uint32_t model_count, QWidget* parent = nullptr);
    virtual ~RgIsaDisassemblyTableModel() = default;

    // Remove all label lines from the table.
    void ClearLabelLines();

    // Copy the given range of data in the table to the clipboard.
    void CopyRowsToClipboard(const QVector<int>& selected_row_numbers);

    // Get the enum value of the incoming ISA table column.
    static int GetTableColumnIndex(RgIsaDisassemblyTableColumns column);

    // Retrieve the current set of correlated line indices. When empty, no line is highlighted in the input source file.
    const std::vector<int>& GetCorrelatedLineIndices() const;

    // Retrieve the table model.
    QStandardItemModel* GetTableModel() const;

    // Get the input source file's line number associated with the instruction given disassembly line index.
    bool GetInputSourceLineNumberFromInstructionRow(int instruction_line_index, int& input_source_line_number);

    // Retrieve a map of label names to the disassembly line index of the label.
    void GetLabelNameToLineIndexMap(std::map<std::string, int>& link_labels) const;

    // Retrieve a map of disassembly table line indices to the label name.
    void GetLineIndexToLabelNameMap(std::map<int, std::string>& link_labels) const;

    // Insert label rows into the disassembly table.
    void InsertLabelRows();

    // Determine if the given index contains a branch instruction's operand jump target.
    bool IsBranchOperandItem(const QModelIndex& model_index);

    // Determine if the column with a given index is visible.
    bool IsColumnVisible(int column_index) const;

    // Is the given ISA line correlated with a line in the input file?
    bool IsIsaLineCorrelated(int line_index) const;

    // Is the given source line correlated with some ISA line(s)?
    bool IsSourceLineCorrelated(int line_index) const;

    // Is the given source file line number within the bounds of the entry point code?
    bool IsSourceLineInEntrypoint(int line_index) const;

    // Populate the model by loading a disassembly CSV file.
    bool PopulateFromCsvFile(const std::string& csv_file_full_path);

    // Set the correlated input source line index to highlight.
    bool SetCorrelatedSourceLineIndex(int line_index);

protected:
    // An enumeration specifying each column we expect to find in an ISA CSV file.
    enum class RgIsaDisassemblyCsvFileColumns
    {
        kAddress,
        kSourceLineNumber,
        kOpcode,
        kOperands,
        kFunctionalUnit,
        kCycles,
        kBinaryEncoding,
        kCount
    };

    // An enumeration used to classify different types of parsed disassembly lines.
    enum class RgIsaLineType
    {
        kInstruction,
        kLabel
    };

    // A simple data class used as a common base for storing lines parsed from a disassembly CSV file.
    class RgIsaLine
    {
    public:
        // A constructor used to initialize the type of disassembled instruction.
        RgIsaLine(RgIsaLineType line_type) : type_(line_type) {}

        // The type of line parsed from the disassembly file.
        RgIsaLineType type_;
    };

    // A class used to store a disassembled instruction line.
    class RgIsaLineInstruction : public RgIsaLine
    {
    public:
        // A constructor used to initialize the line type.
        RgIsaLineInstruction() : RgIsaLine(RgIsaLineType::kInstruction) {}

        // The instruction address within the disassembled binary.
        std::string address_;

        // The instruction opcode.
        std::string opcode_;

        // The instruction operands.
        std::string operands_;

        // The cycle count of the instruction.
        std::string cycles_;

        // The functional unit responsible for execution.
        std::string functional_unit_;

        // The hex representation of the instruction.
        std::string binary_encoding_;
    };

    // A class used to store a disassembly line label.
    class RgIsaLineLabel : public RgIsaLine
    {
    public:
        // A constructor used to initialize the line type.
        RgIsaLineLabel() : RgIsaLine(RgIsaLineType::kLabel) {}

        // The name of the label, if applicable.
        std::string label_name_;
    };

    // Retrieve the list of disassembled instruction lines associated with the given input source line number.
    bool GetDisassemblyLineIndicesFromInputSourceLine(int input_source_line, std::vector<int>& disassembly_lines);

    // Get the enum value of the incoming CSV table column.
    int GetCsvColumnIndex(RgIsaDisassemblyCsvFileColumns column) const;

    // Look up the color to use for the given functional unit.
    QColor GetFunctionalUnitColor(const std::string& functional_unit) const;

    // Load the data in the given CSV file into the model.
    bool LoadCsvData(const std::string& csv_file_full_path);

    // Set the model's cell text.
    void SetTableModelText(const std::string& model_text, uint row_index, uint column_index);

    // Set the model's cell text foreground color.
    void SetTableModelTextColor(const QColor& model_color, uint row_index);

    // Set the model's cell text background color.
    void SetTableModelBackgroundColor(const QColor& model_color, uint row_index, uint column_index);

    // Parse an instruction line from an ISA disassembly CSV file.
    bool ParseCsvIsaLine(const std::string& disassembled_line, std::shared_ptr<RgIsaLine>& parsed_line, int& input_source_line_index);

    // Initialize the model's data with the disassembled instruction lines.
    void InitializeModelData();

    // Get the maximum text length for each column in the ISA region from start_row to EndRow.
    // The values of maximum width for all visible columns are added to the "widths" vector.
    void GetColumnMaxWidths(const QVector<int>& selected_row_numbers, std::vector<int>& widths) const;

    // A vector of all disassembled instructions loaded from file.
    std::vector<std::shared_ptr<RgIsaLine>> disassembled_isa_lines_;

    // A map of instruction index to high level source line index.
    std::map<int, int> instruction_line_number_to_input_source_line_number_;

    // A map of input source file line index to a list of assembly instruction line indices.
    std::map<int, std::vector<int>> input_source_line_index_to_instruction_line_indices_;

    // The entrypoint's start line number in the input source file.
    int source_file_entrypoint_start_line_ = INT_MAX;

    // The entrypoint's end line number in the input source file.
    int source_file_entrypoint_end_line_ = INT_MIN;

    // The column that labels are currently inserted into.
    int label_column_index_ = -1;

    // The cache of correlated line indices used for highlights in the ISA table.
    std::vector<int> correlated_isa_line_indices_;

    // The model containing the data used to populate the ISA disassembly table.
    QStandardItemModel* isa_table_model_ = nullptr;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_ISA_DISASSEMBLY_TABLE_MODEL_H_

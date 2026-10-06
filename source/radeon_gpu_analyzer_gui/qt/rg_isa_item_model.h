//=============================================================================
/// Copyright (c) 2020-2026 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for shader ISA Disassembly view item model.
//=============================================================================

#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_ISA_ITEM_MODEL_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_ISA_ITEM_MODEL_H_

// C++.
#include <set>

// Qt.
#include <QWidget>

// Infra.
#include "qt_isa_gui/widgets/isa_item_model.h"

// Local.
#include "radeon_gpu_analyzer_gui/rg_data_types.h"
#include "radeon_gpu_analyzer_gui/rg_definitions.h"

// A widget used to present disassembled instructions for multiple entrypoints in a kernel.
// Within this view all disassembled instructions target the same ASIC.
class RgIsaItemModel final : public IsaItemModel
{
    Q_OBJECT

public:
    enum Columns
    {
        // The VGPR pressure.
        kIsaColumnVgprPressure = IsaItemModel::Columns::kColumnCount,
        // The function unit.
        kIsaColumnFunctionalUnit,
        // Total RGA isa item model columns.
        kColumnCount
    };

    // Predefined column headers.
    static const std::array<std::string, kColumnCount - IsaItemModel::kColumnCount> kColumnNames;

    enum UserRoles
    {
        // The line with VGPR pressure.
        kMaxVgprLineRole = IsaItemModel::UserRoles::kUserRolesCount,
        // (Boolean) if the current isa line correlated with the currently selected line in the input file?
        kSrcLineToIsaRowRole,
        // (Integer) Line in the input src correlated with the current line in the isa.
        kIsaRowToSrcLineRole,
        // Total User role count.
        kUserRolesCount
    };

    // Source code info corresponding the instruction row.
    struct SrcLineInfo
    {
        // Source file path.
        std::string input_source_file_path;
        // Source line number.
        int input_source_line_index = kInvalidCorrelationLineIndex;
    };

    typedef struct EntryData
    {
        // Target GPU asic.
        std::string target_gpu;
        // Path to current entry's isa file.
        std::string isa_file_path;
        // Path to current entry's isa file.
        std::string vgpr_file_path;
        // Current entry's highlighted input src line number.
        SrcLineInfo source_info;

        enum class Operation
        {
            // Read isa and livereg file data into cache.
            kLoadData,
            // Evict isa and livereg file data from cache.
            kEvictData,
            // Highlight next index with max vgpr.
            kGoToNextMaxVgpr,
            // Highlight prev index with max vgpr.
            kGoToPrevMaxVgpr,
            // Highlight isa lines input_source_line_index
            kUpdateLineCorrelation,
            // Total operations count.
            kOperationCount
        } operation = Operation::kLoadData;

    } EntryData;

    typedef struct RgIndexData
    {
        // Number of live registers units for the row.
        std::string num_live_registers;
        // Opcode for the row.
        std::string opcode;
        // Flag for max vgpr row.
        bool is_max_vgpr_row = false;
        // Flag for if the the row is correlated to the higlighted src row.
        bool is_active_correlation = false;
        // Input src line sc path for the row.
        SrcLineInfo source_info;
        // Tooltip for the vgpr column for the row.
        std::string vgpr_tooltip;
    } RgIndexData;

    explicit RgIsaItemModel(QObject* parent = nullptr);

    virtual ~RgIsaItemModel() = default;

    // Override column count to add columns specific to RGA.
    int columnCount(const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;

    // Override headerData to add columns specific to RGA.
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

    // Override data to add columns specific to RGA.
    QVariant data(const QModelIndex& index, int role) const Q_DECL_OVERRIDE;

    // Override and implement UpdateData(...) to update this model's internal state for a new RGA event.
    void UpdateData(void* data) override;

    // Get all the source indices of the instruction(s) with the highest vgpr pressure.
    bool GetMaxVgprPressureIndices(std::vector<QModelIndex>& source_indices) const;

    // Get the first isa row index for the given input source line number.
    // When input_source_file_path is non-empty, only rows whose source path matches are considered.
    QModelIndex GetFirstLineCorrelatedIndex(int input_source_line_index, const std::string& input_source_file_path = "") const;

    // Get the isa row index for the current max vgpr.
    QModelIndex GetMaxVgprIndex() const;

    // Get the bit-shift used to map live VGPR counts to color range indices for the current target.
    int GetVgprBitShift() const { return vgpr_bit_shift_; }

    // Set target gpu asic for the model.
    bool SetArchitecture(const std::string target_gpu);

    // Cache the sizes of columns based on the isa text that is currently in this model.
    void CacheSizeHints() override;

    // Get the cached column size for the requested column index and tree.
    QSize ColumnSizeHint(int column_index, IsaTreeView* tree) const override;

protected:
    // Columns in the Parsed CSV file.
    enum CsvFileColumns
    {
        kAddress,
        kSourceLineNumber,
        kOpcode,
        kOperands,
        kFunctionalUnit,
        kCycles,
        kBinaryEncoding,
        kSourcePath,
        kCount,
    };

    // ResetModelObject is an RAII convenience class to safely wrap resetting the RgpIsaItemModel.
    // When constructed and destructed this class will invalidate all model indices for any attached views.
    class ResetModelObject final
    {
    public:
        // Constructor; begin resetting the provided RgpIsaItemModel.
        explicit ResetModelObject(RgIsaItemModel* isa_item_model)
            : isa_item_model_(isa_item_model)
        {
            isa_item_model_->beginResetModel();
        }

        // Destructor; end resetting the attached RgpIsaItemModel.
        ~ResetModelObject()
        {
            isa_item_model_->endResetModel();
        }

    private:
        // The RgpIsaItemModel to reset.
        RgIsaItemModel* isa_item_model_;
    };

    // Cached column widths.
    std::array<uint32_t, kColumnCount - IsaItemModel::kColumnCount> column_widths_ = {0, 0};

    // Index data for the entry.
    std::vector<std::vector<RgIndexData>> current_index_data_;

    // Livereg analysis data for the entry.
    RgLiveregData current_livereg_data_;

    // Default bit-shift for mapping live VGPR count into one of 8 color range indices (kVgprRangeColors.size()).
    // Derived from: log2(256 VGPRs / 8 ranges) = log2(32) = 5.
    static constexpr int kDefaultVgprBitShift = 5;

    // Bit-shift used for the current target; overridden in SetArchitecture() for non-default targets.
    int vgpr_bit_shift_ = kDefaultVgprBitShift;

    // Parses a single line of isa in the form of string into the list of strings, one for each column and a list of operands.
    // isa_line           A single line of isa in the form of a std::string that needs to be parsed.
    // line_tokens        Will contain each column separated out into its own string. The operands are all contained in a single string.
    // operand_tokens_str Will contain the operands parsed out into their own string. Does not parse operands into 2 dimensional vector.
    void ParseCsvLine(QString isa_line, std::vector<std::string>& line_tokens, std::vector<std::string>& operand_tokens_str);

    // Reads the csv file at the given path and uses its contents to populate a vector of SharedIsaItemModel blocks.
    // csv_file_full_path The full path and file name of the csv file containng the isa.
    // blocks             Will contain the shared isa blocks require to populate the blocks_ structure and populate the tree.
    void ReadIsaCsvFile(std::string                                        csv_file_full_path,
                        std::vector<std::shared_ptr<IsaItemModel::Block>>& blocks,
                        std::vector<std::vector<RgIndexData>>&             index_data);

    // Reads the csv file at the given path and uses its contents to populate a vector of SharedIsaItemModel blocks.
    bool ParseLiveVgprsData(const std::string&                                 live_vgpr_file_full_path,
                            std::vector<std::shared_ptr<IsaItemModel::Block>>& blocks,
                            std::vector<std::vector<RgIndexData>>&             index_data,
                            RgLiveregData&                                     livereg_data);

    // Calculate the maximum number of VGPRs for the entry.
    int CalculateMaxVgprs(std::vector<std::vector<RgIndexData>>&             index_data,
                          std::vector<std::pair<int, int>>&                  max_line_numbers,
                          std::vector<std::shared_ptr<IsaItemModel::Block>>& blocks);

    // Set the next/prev max vgpr line update.
    bool SetCurrentMaxVgprLine(EntryData::Operation op);

    // Updates highlighted isa rows for the input source line number.
    // When input_source_file_path is non-empty, only rows whose source path matches are highlighted.
    void SetLineCorrelatedIndices(int input_source_line_index, const std::string& input_source_file_path = "");

    // Helper function to create vgpr column tooltip.
    void CreateVgprTooltip(std::string& tooltip, const std::string& num_live_registers) const;

    // Cached Parsed csv data for all entries.
    std::unordered_map<std::string, std::vector<std::shared_ptr<IsaItemModel::Block>>> cached_isa_;

    // Cached index data for all entries.
    std::unordered_map<std::string, std::vector<std::vector<RgIndexData>>> cached_index_data_;

    // Cached livereg data for all entries.
    std::unordered_map<std::string, RgLiveregData> cached_livereg_data_;
};

Q_DECLARE_METATYPE(RgIsaItemModel::SrcLineInfo);

#endif  // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_ISA_ITEM_MODEL_H_
